//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/class.h                          $
// $Revision:: 17                                                             $
//   $Author:: Jimdose                                                        $
//     $Date:: 10/25/98 11:53p                                                $
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

#ifndef __CLASS_H__
#define __CLASS_H__

#include "g_local.h"

class Class;
class Event;
class Archiver;

typedef void (Class::*Response)(Event *event);
typedef struct
{
   Event		*event;
   Response	response;
} ResponseDef;

/***********************************************************************

  ClassDef

***********************************************************************/

class EXPORT_FROM_DLL ClassDef final
{
public:
   const char      *classname        = nullptr;
   const char      *classID          = nullptr;
   const char      *superclass       = nullptr;
   void            *(*newInstance)() = nullptr;
   int              classSize        = 0;
   ResponseDef     *responses        = nullptr;
   int              numEvents        = 0;
   Response       **responseLookup   = nullptr;
   const ClassDef  *super            = nullptr;
   ClassDef        *next;
   ClassDef        *prev;

   ClassDef();
   ~ClassDef();
   ClassDef(const char *classname, const char *classID, const char *superclass,
            ResponseDef *responses, void *(*newInstance)(), int classSize);
   void BuildResponseList();
};

/***********************************************************************

  SafePtr

***********************************************************************/

class SafePtrBase;

class Class;

class EXPORT_FROM_DLL SafePtrBase
{
private:
   void AddReference(Class *ptr)    noexcept;
   void RemoveReference(Class *ptr) noexcept;

public: 
   // ksh--Changed from protected to remove need for operator friend functions in class SafePtr
   SafePtrBase *prevSafePtr = nullptr;
   SafePtrBase *nextSafePtr = nullptr;
   Class       *ptr         = nullptr;

public:
   virtual     ~SafePtrBase();
   void        InitSafePtr(Class *newptr) noexcept;
   void        Clear();
};

/***********************************************************************

  Class

***********************************************************************/

#define CLASS_DECLARATION( nameofsuperclass, nameofclass, classid )  \
   ClassDef nameofclass::ClassInfo                                   \
   (                                                                 \
      #nameofclass, classid, #nameofsuperclass,                      \
      nameofclass::Responses, nameofclass::_newInstance,             \
      sizeof( nameofclass )                                          \
   );                                                                \
   EXPORT_FROM_DLL void *nameofclass::_newInstance()                 \
   {                                                                 \
      return new nameofclass();                                      \
   }                                                                 \
   const ClassDef *nameofclass::classinfo() const                    \
   {                                                                 \
      return &( nameofclass::ClassInfo );                            \
   }

#define CLASS_PROTOTYPE_BASE( nameofclass )    \
   public:                                     \
   static   ClassDef       ClassInfo;          \
   static   void           *_newInstance();    \
   virtual  const ClassDef *classinfo() const; \
   static   ResponseDef    Responses[];

#define CLASS_PROTOTYPE( nameofclass )                  \
   public:                                              \
   static   ClassDef       ClassInfo;                   \
   static   void           *_newInstance();             \
   virtual  const ClassDef *classinfo() const override; \
   static   ResponseDef    Responses[];

class EXPORT_FROM_DLL Class
{
private:
   SafePtrBase	 *SafePtrList = nullptr;
   friend class SafePtrBase;

public:
   CLASS_PROTOTYPE_BASE(Class);
   void *operator    new (size_t);
   void  operator    delete (void *);

   virtual           ~Class();
   virtual void      Archive(Archiver &arc);
   virtual void      Unarchive(Archiver &arc);
   void              warning(const char *function, const char *fmt, ...) const;
   void              error(const char *function, const char *fmt, ...)   const;
   qboolean          inheritsFrom(const ClassDef *c)  const;
   qboolean          inheritsFrom(const char *name)   const;
   qboolean          isInheritedBy(const char *name)  const;
   qboolean          isInheritedBy(const ClassDef *c) const;
   const char        *getClassname()  const;
   const char        *getClassID()    const;
   const char        *getSuperclass() const;
   void              *newInstance()   const;

   template<typename T>
   qboolean isSubclassOf() const { return inheritsFrom(&T::ClassInfo); }

   template<typename T>
   qboolean isSuperclassOf() const { return isInheritedBy(&T::ClassInfo); }
};

EXPORT_FROM_DLL void       BuildEventResponses();
EXPORT_FROM_DLL const ClassDef *getClassForID(const char *name);
EXPORT_FROM_DLL const ClassDef *getClass(const char *name);
EXPORT_FROM_DLL const ClassDef *getClassList();
EXPORT_FROM_DLL void       listAllClasses();
EXPORT_FROM_DLL void       listInheritanceOrder(const char *classname);
EXPORT_FROM_DLL qboolean   checkInheritance(const ClassDef *superclass, const ClassDef *subclass);
EXPORT_FROM_DLL qboolean   checkInheritance(const ClassDef *superclass, const char *subclass);
EXPORT_FROM_DLL qboolean   checkInheritance(const char *superclass, const char *subclass);
EXPORT_FROM_DLL void       DisplayMemoryUsage();

inline EXPORT_FROM_DLL qboolean Class::inheritsFrom(const ClassDef *c) const
{
   return checkInheritance(c, classinfo());
}

inline EXPORT_FROM_DLL qboolean Class::isInheritedBy(const ClassDef *c) const
{
   return checkInheritance(classinfo(), c);
}

/***********************************************************************

  SafePtr

***********************************************************************/

inline EXPORT_FROM_DLL SafePtrBase::~SafePtrBase()
{
   Clear();
}

inline EXPORT_FROM_DLL void SafePtrBase::Clear(void)
{
   if(ptr)
   {
      RemoveReference(ptr);
      ptr = nullptr;
   }
}

inline EXPORT_FROM_DLL void SafePtrBase::InitSafePtr(Class *newptr) noexcept
{
   if(ptr != newptr)
   {
      if(ptr)
      {
         RemoveReference(ptr);
      }

      ptr = newptr;
      if(ptr == nullptr)
      {
         return;
      }

      AddReference(ptr);
   }
}

inline EXPORT_FROM_DLL void SafePtrBase::AddReference(Class *ptr) noexcept
{
   if(!ptr->SafePtrList)
   {
      ptr->SafePtrList = this;
      LL_Reset(this, nextSafePtr, prevSafePtr);
   }
   else
   {
      LL_Add(ptr->SafePtrList, this, nextSafePtr, prevSafePtr);
   }
}

inline EXPORT_FROM_DLL void SafePtrBase::RemoveReference(Class *ptr) noexcept
{
   if(ptr->SafePtrList == this)
   {
      if(ptr->SafePtrList->nextSafePtr == this)
      {
         ptr->SafePtrList = nullptr;
      }
      else
      {
         ptr->SafePtrList = nextSafePtr;
         LL_Remove(this, nextSafePtr, prevSafePtr);
      }
   }
   else
   {
      LL_Remove(this, nextSafePtr, prevSafePtr);
   }
}

template<class T>
class EXPORT_FROM_DLL SafePtr : public SafePtrBase
{
public:
   SafePtr(T *objptr = nullptr) noexcept
   {
       InitSafePtr(objptr);
   }

   SafePtr(const SafePtr &obj) noexcept
   {
       InitSafePtr(obj.ptr);
   }

   SafePtr &operator = (const SafePtr &obj) noexcept
   {
       InitSafePtr(obj.ptr);
       return *this;
   }

   SafePtr &operator = (T *const obj) noexcept
   {
       InitSafePtr(obj);
       return *this;
   }

   // ksh -- Removed these, as they were screwing up the linking under .NET.
//		friend EXPORT_FROM_DLL int __cdecl operator==( SafePtr<T> a, T *b );
//		friend EXPORT_FROM_DLL int __cdecl operator!=( SafePtr<T> a, T *b );
//		friend EXPORT_FROM_DLL int __cdecl operator==( T *a, SafePtr<T> b );
//		friend EXPORT_FROM_DLL int __cdecl operator!=( T *a, SafePtr<T> b );
//		friend EXPORT_FROM_DLL int __cdecl operator==( SafePtr<T> a, SafePtr<T> b ); // ksh -- added to fix compilation bugs under .NET
//		friend EXPORT_FROM_DLL int __cdecl operator!=( SafePtr<T> a, SafePtr<T> b ); // ksh -- added to fix compilation bugs under .NET

   operator T *() const noexcept
   {
       return (T *)ptr;
   }

   T *operator -> () const noexcept
   {
       return (T *)ptr;
   }

   T &operator * () const noexcept
   {
       return *(T *)ptr;
   }
};

template<class T>
inline EXPORT_FROM_DLL constexpr int __cdecl operator == (const SafePtr<T> &a, T *b) noexcept
{
   return a.ptr == b;
}

template<class T>
inline EXPORT_FROM_DLL constexpr int __cdecl operator != (const SafePtr<T> &a, T *b) noexcept
{
   return a.ptr != b;
}

template<class T>
inline EXPORT_FROM_DLL constexpr int __cdecl operator == (T *a, const SafePtr<T> &b) noexcept
{
   return a == b.ptr;
}

template<class T>
inline EXPORT_FROM_DLL constexpr int __cdecl operator != (T *a, const SafePtr<T> &b) noexcept
{
   return a != b.ptr;
}

template<class T>
inline EXPORT_FROM_DLL constexpr int __cdecl operator == (const SafePtr<T> &a, const SafePtr<T> &b) noexcept
{
   return a.ptr == b.ptr;
}

template<class T>
inline EXPORT_FROM_DLL constexpr int __cdecl operator != (const SafePtr<T> &a, const SafePtr<T> &b) noexcept
{
   return a.ptr != b.ptr;
}

#ifdef EXPORT_TEMPLATE
template class EXPORT_FROM_DLL SafePtr<Class>;
#endif
typedef SafePtr<Class> ClassPtr;

#include "archive.h"

#endif /* class.h */

// EOF

