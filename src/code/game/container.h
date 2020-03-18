//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/container.h                      $
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
// Base class for a dynamic array.  Allows adding, removing, index of,
// and finding of entries with specified value.  Originally created for 
// cataloging entities, but pointers to objects that may be removed at
// any time are bad to keep around, so only entity numbers should be
// used in the future.
// 

#ifndef __CONTAINER_H__
#define __CONTAINER_H__

#include "g_local.h"
#include <stdlib.h>

template< class Type >
class EXPORT_FROM_DLL Container
{
private:
   Type  *objlist;
   int   numobjects;
   int   maxobjects;

public:
   Container();
   ~Container();

   void     FreeObjectList(void);
   void     ClearObjectList(void);
   int      NumObjects(void) const;
   void     Resize(int maxelements);
   void     SetObjectAt(int index, Type& obj);
   int      AddObject(Type& obj);
   int      AddUniqueObject(Type& obj);
   void     AddObjectAt(int index, Type& obj);
   int      IndexOfObject(const Type & obj);
   qboolean ObjectInList(Type& obj);
   Type     &ObjectAt(int index);
   Type     *AddressOfObjectAt(int index);
   void     RemoveObjectAt(int index);
   void     RemoveObject(Type const & obj);
   void     Sort(int(__cdecl *compare)(const void *elem1, const void *elem2));
   //      virtual void Archive(	Archiver &arc );
   //      virtual void Unarchive( Archiver &arc );

   // haleyjd 20170608: C++ iterator support for this container
   typedef Type       *iterator;
   typedef Type const *const_iterator;
   iterator       begin()  const { return objlist; }
   iterator       end()    const { return objlist + numobjects; }
   const_iterator cbegin() const { return objlist; }
   const_iterator cend()   const { return objlist + numobjects; }
};

/*
template< class Type >
EXPORT_FROM_DLL void Container<Type>::Archive
   (
   Archiver &arc
   )
   {
   int i;

   arc.WriteInteger( maxobjects );
   arc.WriteInteger( numobjects );

   for ( i = 0; i < numobjects; i++ )
      {
      arc.WriteRaw( objlist[ i ], sizeof( Type ) );
      }
   }

template< class Type >
EXPORT_FROM_DLL void Container<Type>::Unarchive
   (
   Archiver &arc
   )

   {
   int i;

   FreeObjectList();

   maxobjects = arc.ReadInteger();
   numobjects = arc.ReadInteger();

   objlist = new Type[ maxobjects ];
   for ( i = 0; i < numobjects; i++ )
      {
      arc.ReadRaw( &objlist[ i ], sizeof( Type ) );
      }
   }
*/

template<class Type>
Container<Type>::Container()
{
   objlist = nullptr;
   FreeObjectList();
}

template<class Type>
Container<Type>::~Container()
{
   FreeObjectList();
}

template<class Type>
EXPORT_FROM_DLL void Container<Type>::FreeObjectList(void)
{
   if(objlist)
   {
      delete [] objlist;
   }
   objlist = nullptr;
   numobjects = 0;
   maxobjects = 0;
}

template<class Type>
EXPORT_FROM_DLL void Container<Type>::ClearObjectList(void)
{
   // only delete the list if we have objects in it 
   if(objlist && numobjects)
   {
      delete [] objlist;
      objlist = new Type[maxobjects];
      numobjects = 0;
   }
}

template<class Type>
EXPORT_FROM_DLL int Container<Type>::NumObjects(void) const
{
   return numobjects;
}

template<class Type>
EXPORT_FROM_DLL void Container<Type>::Resize(int maxelements)
{
   Type *temp;
   int i;

   assert(maxelements >= 0);

   if(maxelements <= 0)
   {
      FreeObjectList();
      return;
   }

   if(!objlist)
   {
      maxobjects = maxelements;
      objlist = new Type [maxobjects];
   }
   else
   {
      temp = objlist;
      maxobjects = maxelements;
      if(maxobjects < numobjects)
      {
         maxobjects = numobjects;
      }

      objlist = new Type [maxobjects];
      for(i = 0; i < numobjects; i++)
      {
         objlist[i] = temp[i];
      }
      delete [] temp;
   }
}

template<class Type>
EXPORT_FROM_DLL void Container<Type>::SetObjectAt(int index, Type &obj)
{
   if((index <= 0) || (index > numobjects))
   {
      gi.error("Container::SetObjectAt : index out of range");
   }

   objlist[index - 1] = obj;
}

template<class Type>
EXPORT_FROM_DLL int Container<Type>::AddObject(Type &obj)
{
   if(!objlist)
   {
      Resize(10);
   }

   if(numobjects == maxobjects)
   {
      Resize(maxobjects * 2);
   }

   objlist[numobjects] = obj;
   numobjects++;

   return numobjects;
}

template<class Type>
EXPORT_FROM_DLL int Container<Type>::AddUniqueObject(Type &obj)
{
   int index;

   index = IndexOfObject(obj);
   if(!index)
      index = AddObject(obj);
   return index;
}

template<class Type>
EXPORT_FROM_DLL void Container<Type>::AddObjectAt(int index, Type &obj)
{
   //
   // this should only be used when reconstructing a list that has to be identical to the original
   //
   if(index > maxobjects)
   {
      Resize(index);
   }
   if(index > numobjects)
   {
      numobjects = index;
   }
   SetObjectAt(index, obj);
}

template<class Type>
EXPORT_FROM_DLL int Container<Type>::IndexOfObject(const Type &obj)
{
   int i;

   for(i = 0; i < numobjects; i++)
   {
      if(objlist[i] == obj)
      {
         return i + 1;
      }
   }

   return 0;
}

template<class Type>
EXPORT_FROM_DLL qboolean Container<Type>::ObjectInList(Type &obj)
{
   if(!IndexOfObject(obj))
   {
      return false;
   }

   return true;
}

template<class Type>
EXPORT_FROM_DLL Type &Container<Type>::ObjectAt(int index)
{
   if((index <= 0) || (index > numobjects))
   {
      gi.error("Container::ObjectAt : index out of range");
   }

   return objlist[index - 1];
}

template< class Type >
EXPORT_FROM_DLL Type *Container<Type>::AddressOfObjectAt(int index)
{
   //
   // this should only be used when reconstructing a list that has to be identical to the original
   //
   if(index > maxobjects)
   {
      gi.error("Container::AddressOfObjectAt : index is greater than maxobjects");
   }
   if(index > numobjects)
   {
      numobjects = index;
   }
   return &objlist[index - 1];
}

template<class Type>
EXPORT_FROM_DLL void Container<Type>::RemoveObjectAt(int index)
{
   int i;

   if(!objlist)
   {
      gi.dprintf("Container::RemoveObjectAt : Empty list\n");
      return;
   }

   if((index <= 0) || (index > numobjects))
   {
      gi.error("Container::RemoveObjectAt : index out of range");
      return;
   }

   i = index - 1;
   numobjects--;
   for(i = index - 1; i < numobjects; i++)
   {
      objlist[i] = objlist[i + 1];
   }
}

template<class Type>
EXPORT_FROM_DLL void Container<Type>::RemoveObject(const Type &obj)
{
   int index;

   index = IndexOfObject(obj);
   if(!index)
   {
      gi.dprintf("Container::RemoveObject : Object not in list\n");
      return;
   }

   RemoveObjectAt(index);
}

template<class Type>
EXPORT_FROM_DLL void Container<Type>::Sort(int (__cdecl *compare)(const void *elem1, const void *elem2))
{
   if(!objlist)
   {
      gi.dprintf("Container::RemoveObjectAt : Empty list\n");
      return;
   }

   qsort((void *)objlist, (size_t)numobjects, sizeof(Type), compare);
}

//
// Exported templated classes must be explicitly instantiated
//
#ifdef EXPORT_TEMPLATE
template class EXPORT_FROM_DLL Container<int>;
#endif

#endif /* container.h */

// EOF

