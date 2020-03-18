//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/archive.cpp                      $
// $Revision:: 19                                                             $
//   $Author:: Aldie                                                          $
//     $Date:: 11/12/98 3:46p                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Class for archiving objects
// 

#include "g_local.h"
#include "archive.h"

#define ARCHIVE_WRITE 0
#define ARCHIVE_READ  1

enum arctype_e
{
   ARC_NULL, ARC_Vector, ARC_Integer, ARC_Unsigned, ARC_Byte, ARC_Char, ARC_Short, ARC_UnsignedShort,
   ARC_Float, ARC_Double, ARC_Boolean, ARC_String, ARC_Raw, ARC_Object, ARC_ObjectPointer,
   ARC_SafePointer, ARC_Event, ARC_Quat, ARC_Entity,
   ARC_NUMTYPES
};

static const char *typenames[ARC_NUMTYPES] =
{
   "NULL", "vector", "int", "unsigned", "byte", "char", "short", "unsigned short",
   "float", "double", "qboolean", "string", "raw data", "object", "objectpointer",
   "safepointer", "event", "quaternion", "entity"
};

#define ArchiveHeader   ( *( int * )"SIN\0" )
#define ArchiveVersion  2                       // This must be changed any time the format changes!
#define ArchiveInfo     "Sin Archive Version 2" // This must be changed any time the format changes!

CLASS_DECLARATION( Class, ReadFile, NULL );

ResponseDef ReadFile::Responses[] =
{
   { NULL, NULL }
};

ReadFile::ReadFile()
{
   length = 0;
   buffer = NULL;
   pos = 0;
}

ReadFile::~ReadFile()
{
   Close();
}

void ReadFile::Close(void)
{
   if(buffer)
   {
      gi.TagFree((void *)buffer);
      buffer = NULL;
   }

   filename = "";
   length = 0;
   pos = 0;
}

const char *ReadFile::Filename(void)
{
   return filename.c_str();
}

size_t ReadFile::Length(void)
{
   return length;
}

size_t ReadFile::Pos(void)
{
   return pos - buffer;
}

qboolean ReadFile::Seek(size_t newpos)
{
   if(!buffer)
   {
      return false;
   }

   if(newpos < 0)
   {
      return false;
   }

   if(newpos > length)
   {
      return false;
   }

   pos = buffer + newpos;

   return true;
}

qboolean ReadFile::Open(const char *name)
{
   assert(name);

   assert(!buffer);
   Close();

   if(!name)
   {
      return false;
   }

   length = gi.LoadFile(name, (void **)&buffer, 0);
   if(length == (size_t)(-1))
   {
      return false;
   }

   filename = name;
   pos = buffer;

   return true;
}

qboolean ReadFile::Read(void *dest, size_t size)
{
   assert(dest);
   assert(buffer);
   assert(pos);

   if(!dest)
   {
      return false;
   }

   if(size <= 0)
   {
      return false;
   }

   if((pos + size) > (buffer + length))
   {
      return false;
   }

   memcpy(dest, pos, size);
   pos += size;

   return true;
}

CLASS_DECLARATION( Class, Archiver, NULL );

ResponseDef Archiver::Responses[] =
{
   { NULL, NULL }
};

static_assert((sizeof(typenames) / sizeof(typenames[0])) == ARC_NUMTYPES, "ARC_NUMTYPES is incorrect");

Archiver::Archiver()
{
   file = NULL;
   fileerror = false;
}

Archiver::~Archiver()
{
   if(file)
   {
      Close();
   }

   readfile.Close();
}

void Archiver::FileError(const char *fmt, ...)
{
   va_list  argptr;
   char     text[1024];

   va_start(argptr, fmt);
   vsnprintf(text, sizeof(text), fmt, argptr);
   va_end(argptr);

   fileerror = true;
   Close();
   if(archivemode == ARCHIVE_READ)
   {
      gi.error("Error while loading %s : %s\n", filename.c_str(), text);
   }
   else
   {
      gi.error("Error while writing to %s : %s\n", filename.c_str(), text);
   }
}

void Archiver::Close(void)
{
   if(file)
   {
      if(archivemode == ARCHIVE_WRITE)
      {
         // write out the number of classpointers
         fseek(file, numclassespos, SEEK_SET);
         numclassespos = ftell(file);
         WriteInteger(classpointerList.NumObjects());
      }

      fclose(file);
      file = NULL;
   }

   readfile.Close();

   if(archivemode == ARCHIVE_READ)
   {
      int i, num;
      Class * classptr;
      pointer_fixup_t *fixup;

      num = fixupList.NumObjects();
      for(i = 1; i <= num; i++)
      {
         fixup = fixupList.ObjectAt(i);
         classptr = classpointerList.ObjectAt(fixup->index);
         if(fixup->type == pointer_fixup_normal)
         {
            Class ** fixupptr;
            fixupptr = (Class **)fixup->ptr;
            *fixupptr = classptr;
         }
         else if(fixup->type == pointer_fixup_safe)
         {
            SafePtrBase * fixupptr;
            fixupptr = (SafePtrBase *)fixup->ptr;
            fixupptr->InitSafePtr(classptr);
         }
         delete fixup;
      }
      fixupList.FreeObjectList();
      classpointerList.FreeObjectList();
   }
}

/****************************************************************************************

  File Read functions

*****************************************************************************************/

void Archiver::Read(const char *name)
{
   unsigned header;
   unsigned version;
   str      info;
   int      num;
   int      i;
   Class    *null;

   assert(name);
   if(!name)
   {
      gi.error("NULL pointer for filename in Archiver::Read.\n");
   }

   fileerror = false;

   archivemode = ARCHIVE_READ;

   filename = name;

   if(!readfile.Open(filename.c_str()))
   {
      FileError("Couldn't open file.");
   }

   header = ReadUnsigned();
   if(header != ArchiveHeader)
   {
      readfile.Close();
      FileError("Not a valid Sin archive.");
   }

   version = ReadUnsigned();
   if(version > ArchiveVersion)
   {
      readfile.Close();
      // SINEX_FIXME: Ritual URL
      FileError("Archive is from version %.2f.  Check http://www.ritual.com for an update.", version);
   }

   if(version < ArchiveVersion)
   {
      readfile.Close();
      FileError("Archive is out of date.");
   }

   info = ReadString();
   gi.dprintf("%s\n", info.c_str());

   // setup out class pointers
   num = ReadInteger();
   classpointerList.Resize(num);
   null = NULL;
   for(i = 1; i <= num; i++)
   {
      classpointerList.AddObject(null);
   }
}

inline void Archiver::CheckRead(void)
{
   assert(archivemode == ARCHIVE_READ);
   if(!fileerror && (archivemode != ARCHIVE_READ))
   {
      FileError("File read during a write operation.");
   }
}

inline int Archiver::ReadType(void)
{
   int t;

   if(!fileerror)
   {
      readfile.Read(&t, sizeof(t));

      return t;
   }

   return ARC_NULL;
}

inline void Archiver::CheckType(int type)
{
   int t;

   assert((type >= 0) && (type < ARC_NUMTYPES));

   if(!fileerror)
   {
      t = ReadType();
      if(t != type)
      {
         FileError("Expecting %s", typenames[type]);
      }
   }
}

inline size_t Archiver::ReadSize(void)
{
   size_t s;

   s = 0;
   if(!fileerror)
   {
      readfile.Read(&s, sizeof(s));
   }

   return s;
}

inline void Archiver::CheckSize(int type, size_t size)
{
   size_t s;

   if(!fileerror)
   {
      s = ReadSize();

      if(size != s)
      {
         FileError("Invalid data size of %d on %s.", s, typenames[type]);
      }
   }
}

inline void Archiver::ReadData(int type, void *data, size_t size)
{
   CheckRead();
   CheckType(type);
   CheckSize(type, size);

   if(!fileerror && size)
   {
      readfile.Read(data, size);
   }
}

// SINEX_FIXME: not strictly portable
#define READ( func, type ) \
   type Archiver::Read ## func (void)           \
   {                                            \
      type v;                                   \
      ReadData(ARC_ ## func, &v, sizeof(type)); \
      return v;                                 \
   }

READ(Vector, Vector);
READ(Integer, int);
READ(Unsigned, unsigned);
READ(Byte, byte);
READ(Char, char);
READ(Short, short);
READ(UnsignedShort, unsigned short);
READ(Float, float);
READ(Double, double);
READ(Boolean, qboolean);
READ(Quat, Quat);

// SINEX_FIXME: not strictly portable
#define READPTR( func, type )                  \
   void Archiver::Read ## func (type *v)       \
   {                                           \
      ReadData(ARC_ ## func, v, sizeof(type)); \
   }

READPTR(Vector, Vector);
READPTR(Integer, int);
READPTR(Unsigned, unsigned);
READPTR(Byte, byte);
READPTR(Char, char);
READPTR(Short, short);
READPTR(UnsignedShort, unsigned short);
READPTR(Float, float);
READPTR(Double, double);
READPTR(Boolean, qboolean);
READPTR(Quat, Quat);

void Archiver::ReadObjectPointer(Class ** ptr)
{
   int index;
   pointer_fixup_t *fixup;

   ReadData(ARC_ObjectPointer, &index, sizeof(index));

   // Check for a NULL pointer
   assert(ptr);
   if(!ptr)
   {
      FileError("NULL pointer in ReadObjectPointer.");
   }

   //
   // see if the variable was NULL
   //
   if(index == ARCHIVE_NULL_POINTER)
   {
      *ptr = NULL;
   }
   else
   {
      // init the pointer with NULL until we can fix it
      *ptr = NULL;

      fixup = new pointer_fixup_t();
      fixup->ptr = (void **)ptr;
      fixup->index = index;
      fixup->type = pointer_fixup_normal;
      fixupList.AddObject(fixup);
   }
}

void Archiver::ReadSafePointer(SafePtrBase * ptr)
{
   int index;
   pointer_fixup_t *fixup;

   ReadData(ARC_SafePointer, &index, sizeof(&index));

   // Check for a NULL pointer
   assert(ptr);
   if(!ptr)
   {
      FileError("NULL pointer in ReadSafePointer.");
   }

   //
   // see if the variable was NULL
   //
   if(index == ARCHIVE_NULL_POINTER)
   {
      ptr->InitSafePtr(NULL);
   }
   else
   {
      // init the pointer with NULL until we can fix it
      ptr->InitSafePtr(NULL);

      // Add new fixup
      fixup = new pointer_fixup_t();
      fixup->ptr = (void **)ptr;
      fixup->index = index;
      fixup->type = pointer_fixup_safe;
      fixupList.AddObject(fixup);
   }
}

Event Archiver::ReadEvent(void)
{
   Event ev;

   CheckRead();
   CheckType(ARC_Event);

   if(!fileerror)
   {
      ev.Unarchive(*this);
   }

   return ev;
}

void Archiver::ReadEvent(Event * ev)
{
   CheckRead();
   CheckType(ARC_Event);

   if(!fileerror)
   {
      ev->Unarchive(*this);
   }
}

void Archiver::ReadRaw(void *data, size_t size)
{
   ReadData(ARC_Raw, data, size);
}

str Archiver::ReadString(void)
{
   size_t	s;
   char		*data;
   str		string;

   CheckRead();
   CheckType(ARC_String);

   if(!fileerror)
   {
      s = ReadSize();
      if(!fileerror)
      {
         data = new char[s + 1];
         if(s)
         {
            readfile.Read(data, s);
         }
         data[s] = 0;

         string = data;

         delete[] data;
      }
   }

   return string;
}

void Archiver::ReadString(str * string)
{
   *string = ReadString();
}

Class *Archiver::ReadObject(void)
{
   const ClassDef *cls;
   Class          *obj;
   str		classname;
   long		objstart;
   long		endpos;
   int      index;
   size_t	size;
   qboolean isent;
   int      type;

   CheckRead();

   type = ReadType();
   if((type != ARC_Object) && (type != ARC_Entity))
   {
      FileError("Expecting %s or %s", typenames[ARC_Object], typenames[ARC_Entity]);
   }

   size = ReadSize();
   classname = ReadString();

   cls = getClass(classname.c_str());
   if(!cls)
   {
      FileError("Invalid class %s.", classname.c_str());
   }

   isent = checkInheritance(&Entity::ClassInfo, cls);
   if(type == ARC_Entity)
   {
      if(!isent)
      {
         FileError("Non-Entity class object '%s' saved as an Entity based object.", classname.c_str());
      }

      game.force_entnum = true;
      game.spawn_entnum = ReadInteger();
   }
   else if(isent)
   {
      FileError("Entity class object '%s' saved as non-Entity based object.", classname.c_str());
   }

   index = ReadInteger();
   objstart = readfile.Pos();

   obj = (Class *)cls->newInstance();
   if(!obj)
   {
      FileError("Failed to on new instance of class %s.", classname.c_str());
   }
   else
   {
      obj->Unarchive(*this);
   }

   if(isent)
   {
      game.force_entnum = false;
   }

   if(!fileerror)
   {
      endpos = readfile.Pos();
      if((endpos - objstart) > size)
      {
         FileError("Object read past end of object's data");
      }
      else if((endpos - objstart) < size)
      {
         FileError("Object didn't read entire data from file");
      }
   }

   //
   // register this pointer with our list
   //
   classpointerList.AddObjectAt(index, obj);

   return obj;
}

Class *Archiver::ReadObject(Class *obj)
{
   const ClassDef *cls;
   str		classname;
   long		objstart;
   long		endpos;
   int      index;
   size_t	size;
   int      type;
   qboolean isent;

   CheckRead();
   type = ReadType();
   if((type != ARC_Object) && (type != ARC_Entity))
   {
      FileError("Expecting %s or %s", typenames[ARC_Object], typenames[ARC_Entity]);
   }

   size = ReadSize();
   classname = ReadString();

   cls = getClass(classname.c_str());
   if(!cls)
   {
      FileError("Invalid class %s.", classname.c_str());
   }

   if(obj->classinfo() != cls)
   {
      FileError("Archive has a '%s' object, but was expecting a '%s' object.", classname.c_str(), obj->getClassname());
   }

   isent = obj->isSubclassOf<Entity>();
   if(type == ARC_Entity)
   {
      if(!isent)
      {
         FileError("Non-Entity class object '%s' saved as an Entity based object.", classname.c_str());
      }

      static_cast<Entity *>(obj)->SetEntNum(ReadInteger());
   }
   else if(isent)
   {
      FileError("Entity class object '%s' saved as non-Entity based object.", classname.c_str());
   }

   index = ReadInteger();
   objstart = readfile.Pos();

   obj->Unarchive(*this);

   if(!fileerror)
   {
      endpos = readfile.Pos();
      if((endpos - objstart) > size)
      {
         FileError("Object read past end of object's data");
      }
      else if((endpos - objstart) < size)
      {
         FileError("Object didn't read entire data from file");
      }
   }

   //
   // register this pointer with our list
   //
   classpointerList.AddObjectAt(index, obj);

   return obj;
}

/****************************************************************************************

  File Write functions

*****************************************************************************************/

void Archiver::Create(const char *name)
{
   assert(name);
   if(!name)
   {
      gi.error("NULL pointer for filename in Archiver::Create.\n");
   }

   fileerror = false;

   archivemode = ARCHIVE_WRITE;

   filename = name;

   gi.CreatePath(filename.c_str());
   file = fopen(filename.c_str(), "wb");
   if(!file)
   {
      FileError("Couldn't open file.");
   }

   WriteUnsigned(ArchiveHeader);
   WriteUnsigned(ArchiveVersion);
   WriteString(str(ArchiveInfo));

   numclassespos = ftell(file);
   WriteInteger(0);
}

inline void Archiver::CheckWrite(void)
{
   assert(archivemode == ARCHIVE_WRITE);
   if(!fileerror && (archivemode != ARCHIVE_WRITE))
   {
      FileError("File write during a read operation.");
   }
}

inline void Archiver::WriteType(int type)
{
   fwrite(&type, sizeof(type), 1, file);
}

inline void Archiver::WriteSize(size_t size)
{
   fwrite(&size, sizeof(size), 1, file);
}

inline void Archiver::WriteData(int type, const void *data, size_t size)
{
   CheckWrite();
   WriteType(type);
   WriteSize(size);

   if(!fileerror && size)
   {
      fwrite(data, size, 1, file);
   }
}

// SINEX_FIXME: not strictly portable
#define WRITE( func, type )                      \
   void Archiver::Write ## func(type v)          \
   {                                             \
      WriteData(ARC_ ## func, &v, sizeof(type)); \
   }

WRITE(Vector, Vector &);
WRITE(Quat, Quat &);
WRITE(Integer, int);
WRITE(Unsigned, unsigned);
WRITE(Byte, byte);
WRITE(Char, char);
WRITE(Short, short);
WRITE(UnsignedShort, unsigned short);
WRITE(Float, float);
WRITE(Double, double);
WRITE(Boolean, qboolean);

void Archiver::WriteRaw
(
   const void *data,
   size_t size
)
{
   WriteData(ARC_Raw, data, size);
}

void Archiver::WriteString(str &string)
{
   WriteData(ARC_String, string.c_str(), string.length());
}

void Archiver::WriteObject(Class *obj)
{
   str      classname;
   long     sizepos;
   long     objstart;
   long     endpos;
   int      index;
   size_t   size;
   qboolean isent;

   assert(obj);
   if(!obj)
   {
      FileError("NULL object in WriteObject");
   }

   isent = obj->isSubclassOf<Entity>();

   CheckWrite();
   if(isent)
   {
      WriteType(ARC_Entity);
   }
   else
   {
      WriteType(ARC_Object);
   }

   sizepos = ftell(file);
   size = 0;
   WriteSize(size);

   classname = obj->getClassname();
   WriteString(classname);

   if(isent)
   {
      // Write out the entity number
      WriteInteger(static_cast<Entity *>(obj)->entnum);
   }

   // write out pointer index for this class pointer
   index = classpointerList.AddUniqueObject(obj);
   WriteInteger(index);

   if(!fileerror)
   {
      objstart = ftell(file);
      obj->Archive(*this);
   }

   if(!fileerror)
   {
      endpos = ftell(file);
      size = endpos - objstart;
      fseek(file, sizepos, SEEK_SET);
      WriteSize(size);

      if(!fileerror)
      {
         fseek(file, endpos, SEEK_SET);
      }
   }
}

void Archiver::WriteObjectPointer(Class *ptr)
{
   int index;

   if(ptr)
   {
      index = classpointerList.AddUniqueObject(ptr);
   }
   else
   {
      index = ARCHIVE_NULL_POINTER;
   }
   WriteData(ARC_ObjectPointer, &index, sizeof(index));
}

void Archiver::WriteSafePointer(Class *ptr)
{
   int index;

   if(ptr)
   {
      index = classpointerList.AddUniqueObject(ptr);
   }
   else
   {
      index = ARCHIVE_NULL_POINTER;
   }
   WriteData(ARC_SafePointer, &index, sizeof(index));
}

void Archiver::WriteEvent(Event &ev)
{
   CheckWrite();
   WriteType(ARC_Event);

   //FIXME!!!! Make this handle null events
   if(&ev == NULL)
   {
      NullEvent.Archive(*this);
   }
   else
   {
      ev.Archive(*this);
   }
}

// EOF

