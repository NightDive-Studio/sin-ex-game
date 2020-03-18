//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/stack.h                          $
// $Revision:: 6                                                              $
//   $Author:: Jimdose                                                        $
//     $Date:: 3/02/98 5:27p                                                  $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Generic Stack object.
// 

#ifndef __STACK_H__
#define __STACK_H__

#include "g_local.h"
#include "class.h"

template<class Type>
class StackNode
{
public:
   Type       data;
   StackNode *next = nullptr;

   StackNode(Type d);
};

template<class Type>
inline StackNode<Type>::StackNode(Type d) : data(d)
{
}

template<class Type>
class EXPORT_FROM_DLL Stack
{
private:
   StackNode<Type> *head = nullptr;

public:
   ~Stack();
   void     Clear();
   qboolean Empty();
   void     Push(Type data);
   Type     Pop();
};

template<class Type>
inline Stack<Type>::~Stack()
{
   Clear();
}

template<class Type>
inline void EXPORT_FROM_DLL Stack<Type>::Clear()
{
   while(!Empty())
   {
      Pop();
   }
}

template<class Type>
inline qboolean EXPORT_FROM_DLL Stack<Type>::Empty()
{
   if(head == NULL)
   {
      return true;
   }
   return false;
}

template<class Type>
inline void EXPORT_FROM_DLL Stack<Type>::Push(Type data)
{
   StackNode<Type> *tmp;

   tmp = new StackNode<Type>(data);
   if(!tmp)
   {
      assert(0);
      gi.error("Stack::Push : Out of memory");
   }

   tmp->next = head;
   head = tmp;
}

template<class Type>
inline Type EXPORT_FROM_DLL Stack<Type>::Pop()
{
   Type ret;
   StackNode<Type> *node;

   if(!head)
   {
      return nullptr;
   }

   node = head;
   ret = node->data;
   head = node->next;

   delete node;

   return ret;
}

#endif /* stack.h */

// EOF

