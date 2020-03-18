//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/archive.h                        $
// $Revision:: 9                                                              $
//   $Author:: Jimdose                                                        $
//     $Date:: 11/12/98 2:31a                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// $Log:: /Quake 2 Engine/Sin/code/game/archive.h                             $
// 
// DESCRIPTION:
// Class for archiving objects
// 

#ifndef __ARCHIVE_H__
#define __ARCHIVE_H__

#include "g_local.h"
#include "str.h"

#define ARCHIVE_NULL_POINTER ( -654321 )
#define ARCHIVE_POINTER_VALID ( 0 )
#define ARCHIVE_POINTER_NULL ( ARCHIVE_NULL_POINTER )
#define ARCHIVE_POINTER_SELF_REFERENTIAL ( -123456 )

enum
{
   pointer_fixup_normal,
   pointer_fixup_safe
};

typedef struct
{
   void **ptr;
   int  index;
   int  type;
} pointer_fixup_t;

#ifdef EXPORT_TEMPLATE
template class EXPORT_FROM_DLL Container<Class *>;
template class EXPORT_FROM_DLL Container<pointer_fixup_t *>;
#endif

class EXPORT_FROM_DLL ReadFile : public Class
{
protected:
   str            filename;
   size_t         length;
   byte           *buffer;
   byte           *pos;

public:
   CLASS_PROTOTYPE(ReadFile);

   ReadFile();
   ~ReadFile();
   void           Close(void);
   const char     *Filename(void);
   size_t         Length(void);
   size_t         Pos(void);
   qboolean       Seek(size_t newpos);
   qboolean       Open(const char *name);
   qboolean       Read(void *dest, size_t size);
};

class EXPORT_FROM_DLL Archiver : public Class
{
private:
   Container<Class *>           classpointerList;
   Container<pointer_fixup_t *> fixupList;

protected:
   str            filename;
   qboolean       fileerror;
   FILE           *file;
   ReadFile       readfile;
   int            archivemode;
   int            numclassespos;

   void           CheckRead(void);
   void           CheckType(int type);
   int            ReadType(void);
   size_t         ReadSize(void);
   void           CheckSize(int type, size_t size);
   void           ReadData(int type, void *data, size_t size);

   void           CheckWrite(void);
   void           WriteType(int type);
   void           WriteSize(size_t size);
   void           WriteData(int type, const void *data, size_t size);

public:
   CLASS_PROTOTYPE(Archiver);

   Archiver();
   ~Archiver();
   void           FileError(const char *fmt, ...);
   void           Close(void);

   void           Read(str &name);
   void           Read(const char *name);

   //
   // return methods
   //
   Vector         ReadVector(void);
   Quat           ReadQuat(void);
   int            ReadInteger(void);
   unsigned       ReadUnsigned(void);
   byte           ReadByte(void);
   char           ReadChar(void);
   short          ReadShort(void);
   unsigned short ReadUnsignedShort(void);
   float          ReadFloat(void);
   double         ReadDouble(void);
   qboolean       ReadBoolean(void);
   str            ReadString(void);
   Event          ReadEvent(void);
   //
   // ptr methods
   //
   void           ReadVector(Vector * vec);
   void           ReadQuat(Quat * quat);
   void           ReadInteger(int * num);
   void           ReadUnsigned(unsigned * unum);
   void           ReadByte(byte * num);
   void           ReadChar(char * ch);
   void           ReadShort(short * num);
   void           ReadUnsignedShort(unsigned short * num);
   void           ReadFloat(float * num);
   void           ReadDouble(double * num);
   void           ReadBoolean(qboolean * boolValue);
   void           ReadString(str * string);
   void           ReadObjectPointer(Class ** ptr);
   void           ReadSafePointer(SafePtrBase * ptr);
   void           ReadEvent(Event * ev);

   void           ReadRaw(void *data, size_t size);
   Class          *ReadObject(void);
   Class          *ReadObject(Class *obj);

   void           Create(str &name);
   void           Create(const char *name);
   void           WriteVector(Vector &v);
   void           WriteQuat(Quat &quat);
   void           WriteInteger(int v);
   void           WriteUnsigned(unsigned v);
   void           WriteByte(byte v);
   void           WriteChar(char v);
   void           WriteShort(short v);
   void           WriteUnsignedShort(unsigned short v);
   void           WriteFloat(float v);
   void           WriteDouble(double v);
   void           WriteBoolean(qboolean v);
   void           WriteRaw(const void *data, size_t size);
   void           WriteString(str &string);
   void           WriteObject(Class *obj);
   void           WriteObjectPointer(Class * ptr);
   void           WriteSafePointer(Class * ptr);
   void           WriteEvent(Event &ev);
};

inline EXPORT_FROM_DLL void Archiver::Read(str &name)
{
   Read(name.c_str());
}

inline EXPORT_FROM_DLL void Archiver::Create(str &name)
{
   Create(name.c_str());
}

#endif /* archive.h */

// EOF

