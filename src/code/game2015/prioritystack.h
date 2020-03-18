//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/prioritystack.h                  $
// $Revision:: 2                                                              $
//   $Author:: Jimdose                                                        $
//     $Date:: 6/09/98 4:26p                                                  $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
// 
// DESCRIPTION:
// Stack based object that pushes and pops objects in a priority based manner.
// 

#ifndef __PRIORITYSTACK_H__
#define __PRIORITYSTACK_H__

#include "g_local.h"
#include "class.h"

template<class Type>
class PriorityStackNode
{
public:
   int  priority;
   Type data;
   PriorityStackNode *next;

   PriorityStackNode(Type d, int p);
};

template<class Type>
inline PriorityStackNode<Type>::PriorityStackNode(Type d, int p) 
    : data(d), priority(p), next(nullptr)
{
}

template<class Type>
class EXPORT_FROM_DLL PriorityStack
{
private:
   PriorityStackNode<Type> *head = nullptr;

public:
   ~PriorityStack();
   void     Clear();
   qboolean Empty();
   void     Push(Type data, int priority);
   Type     Pop();
};

template<class Type>
inline PriorityStack<Type>::~PriorityStack()
{
   Clear();
}

template<class Type>
inline void EXPORT_FROM_DLL PriorityStack<Type>::Clear()
{
   while(!Empty())
   {
      Pop();
   }
}

template<class Type>
inline qboolean EXPORT_FROM_DLL PriorityStack<Type>::Empty()
{
   if(head == nullptr)
   {
      return true;
   }
   return false;
}

template <class Type>
inline void EXPORT_FROM_DLL PriorityStack<Type>::Push(Type data, int priority)
{
   PriorityStackNode<Type> *tmp;
   PriorityStackNode<Type> *next;
   PriorityStackNode<Type> *prev;

   tmp = new PriorityStackNode<Type>(data, priority);
   if(!tmp)
   {
      assert(0);
      gi.error("PriorityStack::Push : Out of memory");
   }

   if(!head || (priority >= head->priority))
   {
      tmp->next = head;
      head = tmp;
   }
   else
   {
      for(prev = head, next = head->next; next; prev = next, next = next->next)
      {
         if(priority >= next->priority)
         {
            break;
         }
      }

      tmp->next = prev->next;
      prev->next = tmp;
   }
}

template<class Type>
inline Type EXPORT_FROM_DLL PriorityStack<Type>::Pop()
{
   Type ret;
   PriorityStackNode<Type> *node;

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

#endif /* prioritystack.h */

// EOF

