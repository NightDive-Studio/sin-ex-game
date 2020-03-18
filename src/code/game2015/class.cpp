//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/class.cpp                        $
// $Revision:: 20                                                             $
//   $Author:: Jimdose                                                        $
//     $Date:: 10/19/98 12:07a                                                $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Base class that all classes that are used in conjunction with Sin should
// be based off of.  Class gives run-time type information about any class
// derived from it.  This is really handy when you have a pointer to an object 
// that you need to know if it supports certain behaviour.
// 

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "g_local.h"
#include "class.h"
#include "linklist.h"

int totalmemallocated = 0;
int numclassesallocated = 0;

static ClassDef *classlist = nullptr;

ClassDef::ClassDef()
{
   this->prev = this;
   this->next = this;
}

ClassDef::ClassDef(const char *classname, const char *classID, const char *superclass, ResponseDef *responses, 
                   void *(*newInstance)(), int classSize)
{
   if(classlist == nullptr)
   {
      classlist = new ClassDef();
   }

   this->classname      = classname;
   this->classID        = classID;
   this->superclass     = superclass;
   this->responses      = responses;
   this->numEvents      = 0;
   this->responseLookup = nullptr;
   this->newInstance    = newInstance;
   this->classSize      = classSize;
   this->super          = getClass(superclass);

   // It's not uncommon for classes to not have a class id, so just set it 
   // to an empty string so that we're not checking for it all the time.
   if(!classID)
   {
      this->classID = "";
   }

   // Check if any subclasses were initialized before their superclass
   for(ClassDef *node = classlist->next; node != classlist; node = node->next)
   {
      if((node->super == nullptr) && (!Q_stricmp(node->superclass, this->classname)) &&
         (Q_stricmp(node->classname, "Class")))
      {
         node->super = this;
      }
   }

   // Add to front of list
   LL_Add(classlist, this, prev, next);
}

ClassDef::~ClassDef()
{
   ClassDef *node;

   if(classlist != this)
   {
      LL_Remove(this, prev, next);

      // Check if any subclasses were initialized before their superclass
      for(node = classlist->next; node != classlist; node = node->next)
      {
         if(node->super == this)
         {
            node->super = nullptr;
         }
      }
   }
   else
   {
      // If the head of the list is deleted before the list is cleared, then we may have problems
      assert(this->next == this->prev);
   }

   if(responseLookup)
   {
      delete [] responseLookup;
      responseLookup = nullptr;
   }
}

EXPORT_FROM_DLL void ClassDef::BuildResponseList()
{
   const ClassDef *c;
   ResponseDef    *r;
   int             ev;
   int             i;
   qboolean       *set;
   int             num;

   if(responseLookup)
   {
      delete [] responseLookup;
      responseLookup = nullptr;
   }

   num = Event::NumEventCommands();
   responseLookup = reinterpret_cast<Response **>(new char[sizeof(Response *) * num]);
   memset(responseLookup, 0, sizeof(Response *) * num);

   set = new qboolean [num];
   memset(set, 0, sizeof(qboolean) * num);

   this->numEvents = num;

   for(c = this; c != nullptr; c = c->super)
   {
      r = c->responses;
      if(r)
      {
         for(i = 0; r[i].event != nullptr; i++)
         {
            ev = (int)*r[i].event;
            if(!set[ev])
            {
               set[ev] = true;
               if(r[i].response)
               {
                  responseLookup[ev] = &r[i].response;
               }
               else
               {
                  responseLookup[ev] = nullptr;
               }
            }
         }
      }
   }

   delete [] set;
}

EXPORT_FROM_DLL void BuildEventResponses()
{
   ClassDef *c;
   int amount;
   int numclasses;

   amount = 0;
   numclasses = 0;
   for(c = classlist->next; c != classlist; c = c->next)
   {
      c->BuildResponseList();

      amount += c->numEvents * sizeof(Response *);
      numclasses++;
   }

   gi.dprintf("\n------------------\n"
              "Event system initialized:\n"
              "%d classes\n%d events\n%d total memory in response list\n\n",
              numclasses, Event::NumEventCommands(), amount);
}

EXPORT_FROM_DLL const ClassDef *getClassForID(const char *name)
{
   for(const ClassDef *c = classlist->next; c != classlist; c = c->next)
   {
      if(c->classID && !Q_stricmp(c->classID, name))
      {
         return c;
      }
   }

   return nullptr;
}

EXPORT_FROM_DLL const ClassDef *getClass(const char *name)
{
   for(const ClassDef *c = classlist->next; c != classlist; c = c->next)
   {
      if(!Q_stricmp(c->classname, name))
      {
         return c;
      }
   }

   return nullptr;
}

EXPORT_FROM_DLL const ClassDef *getClassList()
{
   return classlist;
}

EXPORT_FROM_DLL void listAllClasses()
{
   for(const ClassDef *c = classlist->next; c != classlist; c = c->next)
   {
      gi.dprintf("%s\n", c->classname);
   }
}

EXPORT_FROM_DLL void listInheritanceOrder(const char *classname)
{
   const ClassDef *cls = getClass(classname);
   if(!cls)
   {
      gi.dprintf("Unknown class: %s\n", classname);
      return;
   }
   for(const ClassDef *c = cls; c != nullptr; c = c->super)
   {
      gi.dprintf("%s\n", c->classname);
   }
}

EXPORT_FROM_DLL qboolean checkInheritance(const ClassDef *superclass, const ClassDef *subclass)
{
   for(const ClassDef *c = subclass; c != nullptr; c = c->super)
   {
      if(c == superclass)
      {
         return true;
      }
   }
   return false;
}

EXPORT_FROM_DLL qboolean checkInheritance(const ClassDef *superclass, const char *subclass)
{
   const ClassDef *c = getClass(subclass);
   if(c == nullptr)
   {
      gi.dprintf("Unknown class: %s\n", subclass);
      return false;
   }
   return checkInheritance(superclass, c);
}

EXPORT_FROM_DLL qboolean checkInheritance(const char *superclass, const char *subclass)
{
   const ClassDef *c1 = getClass(superclass);
   const ClassDef *c2 = getClass(subclass);

   if(c1 == nullptr)
   {
      gi.dprintf("Unknown class: %s\n", superclass);
      return false;
   }
   if(c2 == nullptr)
   {
      gi.dprintf("Unknown class: %s\n", subclass);
      return false;
   }
   return checkInheritance(c1, c2);
}

CLASS_DECLARATION( nullptr, Class, nullptr );

ResponseDef Class::Responses[] =
{
   { nullptr, nullptr }
};

#ifdef NDEBUG

EXPORT_FROM_DLL void *Class::operator new (size_t s)
{
   int *p;

   s += sizeof(int);
   p = reinterpret_cast<int *>(::new char [s]);
   memset(p, 0, s);
   *p = s;
   totalmemallocated += s;
   numclassesallocated++;
   return p + 1;
}

EXPORT_FROM_DLL void Class::operator delete (void *ptr)
{
   int *p;

   p = (reinterpret_cast<int *>(ptr)) - 1;
   totalmemallocated -= *p;
   numclassesallocated--;
   ::delete [] (p);
}

#else

EXPORT_FROM_DLL void * Class::operator new (size_t s)
{
   int *p;

   s += sizeof(int) * 3;
   p = reinterpret_cast<int *>(::new char [s]);
   memset(p, 0, s);
   p[0] = 0x12348765;
   *reinterpret_cast<int *>((reinterpret_cast<byte *>(p)) + s - sizeof(int)) = 0x56784321;
   p[1] = s;
   totalmemallocated += s;
   numclassesallocated++;
   return p + 2;
}

EXPORT_FROM_DLL void Class::operator delete (void *ptr)
{
   int *p;

   p = (reinterpret_cast<int *>(ptr)) - 2;

   assert(p[0] == 0x12348765);
   assert(*reinterpret_cast<int *>((reinterpret_cast<byte *>(p)) + p[1] - sizeof(int)) == 0x56784321);

   totalmemallocated -= p[1];
   numclassesallocated--;
   ::delete [] (p);
}

#endif

EXPORT_FROM_DLL void DisplayMemoryUsage()
{
   gi.printf("Classes %-5d Class memory used: %d\n", numclassesallocated, totalmemallocated);
}

Class::~Class()
{
   while(SafePtrList != nullptr)
   {
      SafePtrList->Clear();
   }
}

EXPORT_FROM_DLL void Class::Archive(Archiver &arc)
{
}

EXPORT_FROM_DLL void Class::Unarchive(Archiver &arc)
{
}

EXPORT_FROM_DLL void Class::warning(const char *function, const char *fmt, ...) const
{
   va_list  argptr;
   char     text[1024];

   va_start(argptr, fmt);
   vsnprintf(text, sizeof(text), fmt, argptr);
   va_end(argptr);

   if(getClassID())
   {
      gi.dprintf("%s::%s : %s\n", getClassID(), function, text);
   }
   else
   {
      gi.dprintf("%s::%s : %s\n", getClassname(), function, text);
   }
}

EXPORT_FROM_DLL void Class::error(const char *function, const char *fmt, ...) const
{
   va_list  argptr;
   char     text[1024];

   va_start(argptr, fmt);
   vsnprintf(text, sizeof(text), fmt, argptr);
   va_end(argptr);

   if(getClassID())
   {
      gi.error("%s::%s : %s\n", getClassID(), function, text);
   }
   else
   {
      gi.error("%s::%s : %s\n", getClassname(), function, text);
   }
}

EXPORT_FROM_DLL qboolean Class::inheritsFrom(const char *name) const
{
   const ClassDef *c = getClass(name);
   if(c == nullptr)
   {
      gi.dprintf("Unknown class: %s\n", name);
      return false;
   }
   return checkInheritance(c, classinfo());
}

EXPORT_FROM_DLL qboolean Class::isInheritedBy(const char *name) const
{
   const ClassDef *c = getClass(name);
   if(c == nullptr)
   {
      gi.dprintf("Unknown class: %s\n", name);
      return false;
   }
   return checkInheritance(classinfo(), c);
}

EXPORT_FROM_DLL const char *Class::getClassname() const
{
   return classinfo()->classname;
}

EXPORT_FROM_DLL const char *Class::getClassID() const
{
   return classinfo()->classID;
}

EXPORT_FROM_DLL const char *Class::getSuperclass() const
{
   return classinfo()->superclass;
}

EXPORT_FROM_DLL void *Class::newInstance() const
{
   return classinfo()->newInstance();
}

// EOF

