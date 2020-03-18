//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/datamap.h                        $
// $Revision:: 2                                                              $
//   $Author:: Jimdose                                                        $
//     $Date:: 8/31/98 5:40p                                                  $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Template class for mapping values of one data type to another.
// 

#ifndef __DATAMAP_H__
#define __DATAMAP_H__

#include "g_local.h"
#include <stdlib.h>

template<class Key, class Value>
class EXPORT_FROM_DLL DataMap
{
private:
   Key   **keyarray   = nullptr;
   Value **valuearray = nullptr;
   int     numobjects = 0;
   int     maxobjects = 0;

public:
   DataMap() noexcept = default;
   DataMap(DataMap<Key, Value> &map);
   ~DataMap();

   void     FreeObjectList();
   void     ClearObjectList();
   int      NumObjects() const;
   void     Resize(int maxelements);
   Value   &operator [] (Key key);
   int      SetValue(Key key, Value obj);
   void     SetValueAt(int index, Value &obj);
   int      AddKey(Key& key);
   int      AddKeyPair(Key &key, Value &value);
   int      FindKey(Key &key) const;
   void     KeyInList(Key &key) const;
   Value   &ValueAt(int index);
   Key     &KeyAt(int index);
   void     RemoveKeyAt(int index);
   void     RemoveKey(Key &key);
};

template<class Key, class Value>
DataMap<Key, Value>::DataMap(DataMap<Key, Value> &map) : DataMap()
{
   Resize(map.maxobjects);
   for(int i = 0; i < map.numobjects; i++)
   {
      AddKeyPair(map.KeyAt(i), map.ValueAt(i));
   }
}

template<class Key, class Value>
DataMap<Key, Value>::~DataMap()
{
   FreeObjectList();
}

template<class Key, class Value>
EXPORT_FROM_DLL void DataMap<Key, Value>::FreeObjectList()
{
   if(keyarray)
   {
      ClearObjectList();

      delete [] keyarray;
      delete [] valuearray;
   }

   keyarray   = nullptr;
   valuearray = nullptr;
   numobjects = 0;
   maxobjects = 0;
}

template<class Key, class Value>
EXPORT_FROM_DLL void DataMap<Key, Value>::ClearObjectList()
{
   // only delete the list if we have objects in it 
   if(keyarray && numobjects)
   {
      for(i = 0; i < numobjects; i++)
      {
         delete keyarray[i];
         delete valuearray[i];
      }

      memset(keyarray,   0, maxobjects * sizeof(Key *));
      memset(valuearray, 0, maxobjects * sizeof(Value *));

      numobjects = 0;
   }
}

template<class Key, class Value>
EXPORT_FROM_DLL int DataMap<Key, Value>::NumObjects() const
{
   return numobjects;
}

template<class Key, class Value>
EXPORT_FROM_DLL void DataMap<Key, Value>::Resize(int maxelements)
{
   Key   *keytemp;
   Value *valuetemp;

   assert(maxelements > 0);

   if(maxelements <= 0)
   {
      FreeObjectList();
      return;
   }

   if(!keyarray)
   {
      maxobjects = maxelements;

      keyarray = new Key[maxobjects];
      memset(keyarray, 0, maxobjects * sizeof(Key *));

      valuearray = new Value[maxobjects];
      memset(valuearray, 0, maxobjects * sizeof(Value *));
   }
   else
   {
      keytemp = keyarray;
      valuetemp = valuearray;

      maxobjects = maxelements;
      if(maxobjects < numobjects)
      {
         maxobjects = numobjects;
      }

      keyarray   = new Key [maxobjects];
      valuearray = new Value [maxobjects];

      memcpy(keyarray,   keytemp,   sizeof(Key   *) * maxobjects);
      memcpy(valuearray, valuetemp, sizeof(Value *) * maxobjects);

      delete [] keytemp;
      delete [] valuetemp;
   }
}

template< class Key, class Value>
EXPORT_FROM_DLL inline Value& DataMap<Key, Value>::operator [] (Key key)
{
   int index;

   index = FindKey(key);
   if(index == -1)
   {
      index = AddKey(key);
   }

   assert((index >= 0) && (index < numobjects));
   assert(valuearray);

   return *valuearray[index];
}

template<class Key, class Value>
EXPORT_FROM_DLL inline int DataMap<Key, Value>::SetValue(Key key, Value value)
{
   int index;

   index = FindKey(key);
   if(index == -1)
   {
      index = AddKey(key);
   }

   assert((index >= 0) && (index < numobjects));
   assert(valuearray);

   *valuearray[index] = value;

   return index;
}

template< class Key, class Value>
EXPORT_FROM_DLL void DataMap<Key, Value>::SetValueAt(int index, Value &obj)
{
   if((index < 0) || (index >= numobjects))
   {
      gi.error("DataMap::SetValueAt : index out of range");
   }

   assert(valuearray);

   *valuearray[index] = obj;
}

template< class Key, class Value>
EXPORT_FROM_DLL int DataMap<Key, Value>::AddKey(Key &key)
{
   int index;

   if(!keyarray)
   {
      Resize(10);
   }

   if(numobjects == maxobjects)
   {
      Resize(maxobjects * 2);
   }

   index = numobjects;
   numobjects++;

   keyarray[index] = new Key();
   valuearray[index] = new Value();
   *keyarray[index] = key;

   return index;
}

template<class Key, class Value>
EXPORT_FROM_DLL int DataMap<Key, Value>::AddKeyPair(Key &key, Value &value)
{
   int index;

   if(!keyarray)
   {
      Resize(10);
   }

   if(numobjects == maxobjects)
   {
      Resize(maxobjects * 2);
   }

   index = numobjects;
   numobjects++;

   keyarray[index]    = new Key();
   valuearray[index]  = new Value();
   *keyarray[index]   = key;
   *valuearray[index] = value;

   return index;
}

template<class Key, class Value>
EXPORT_FROM_DLL int DataMap<Key, Value>::FindKey(Key &key) const
{
   int i;

   for(i = 0; i < numobjects; i++)
   {
      if(*keyarray[i] == key)
      {
         return i;
      }
   }

   return -1;
}

template<class Key, class Value>
EXPORT_FROM_DLL void DataMap<Key, Value>::KeyInList(Key &key) const
{
   if(FindKey(key) == -1)
   {
      return false;
   }

   return true;
}

template<class Key, class Value>
EXPORT_FROM_DLL Value &DataMap<Key, Value>::ValueAt(int index)
{
   if((index < 0) || (index >= numobjects))
   {
      gi.error("DataMap::ValueAt : index out of range");
   }

   return *valuearray[index];
}

template<class Key, class Value>
EXPORT_FROM_DLL Key &DataMap<Key, Value>::KeyAt(int index)
{
   if((index < 0) || (index >= numobjects))
   {
      gi.error("DataMap::KeyAt : index out of range");
   }

   return *keyarray[index];
}

template<class Key, class Value>
EXPORT_FROM_DLL void DataMap<Key, Value>::RemoveKeyAt(int index)
{
   int i;

   if(!keyarray)
   {
      gi.dprintf("DataMap::RemoveKeyAt : Empty list");
      return;
   }

   if((index < 0) || (index >= numobjects))
   {
      gi.error("DataMap::RemoveKeyAt : index out of range");
      return;
   }

   delete keyarray[index];
   delete valuearray[index];

   for(i = index; i < numobjects; i++)
   {
      keyarray[i] = keyarray[i + 1];
      valuearray[i] = valuearray[i + 1];
   }

   numobjects--;
   keyarray[numobjects]   = nullptr;
   valuearray[numobjects] = nullptr;
}

template<class Key, class Value>
EXPORT_FROM_DLL void DataMap<Key, Value>::RemoveKey(Key &key)
{
   int index;

   index = FindKey(key);
   if(index == -1)
   {
      gi.dprintf("DataMap::RemoveKey : Object not in list");
      return;
   }

   RemoveKeyAt(index);
}

#endif /* datamap.h */

// EOF

