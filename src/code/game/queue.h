//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/queue.h                          $
// $Revision:: 5                                                              $
//   $Author:: Jimdose                                                        $
//     $Date:: 10/07/98 11:59p                                                $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Generic Queue object
// 

#ifndef __QUEUE_H__
#define __QUEUE_H__

#include "class.h"

class QueueNode
{
public:
   void      *data = nullptr;
   QueueNode *next = nullptr;
};

class EXPORT_FROM_DLL Queue
{
private:
   QueueNode *head = nullptr;
   QueueNode *tail = nullptr;

public:
   ~Queue();
   void     Clear();
   qboolean Empty();
   void     Enqueue(void *data);
   void     *Dequeue();
   void     Remove(void *data);
   qboolean Inqueue(void *data);
};

inline Queue::~Queue()
{
   Clear();
}

inline void Queue::Clear()
{
   while(!Empty())
   {
      Dequeue();
   }
}

inline qboolean Queue::Empty()
{
   if(head == nullptr)
   {
      assert(!tail);
      return true;
   }

   assert(tail);
   return false;
}

inline void Queue::Enqueue(void *data)
{
   QueueNode *tmp;

   tmp = new QueueNode();
   if(!tmp)
   {
      assert(0);
      gi.error("Queue::Enqueue : Out of memory");
   }

   tmp->data = data;

   assert(!tmp->next);
   if(!head)
   {
      assert(!tail);
      head = tmp;
   }
   else
   {
      assert(tail);
      tail->next = tmp;
   }
   tail = tmp;
}

inline void *Queue::Dequeue()
{
   void *ptr;
   QueueNode *node;

   if(!head)
   {
      assert(!tail);
      return nullptr;
   }

   node = head;
   ptr = node->data;

   head = node->next;
   if(head == nullptr)
   {
      assert(tail == node);
      tail = nullptr;
   }

   delete node;

   return ptr;
}

inline void Queue::Remove(void *data)
{
   QueueNode *node;
   QueueNode *prev;

   if(!head)
   {
      assert(!tail);

      gi.dprintf("Queue::Remove : Data not found in queue\n");
      return;
   }

   for(prev = NULL, node = head; node != NULL; prev = node, node = node->next)
   {
      if(node->data == data)
      {
         break;
      }
   }

   if(!node)
   {
      gi.dprintf("Queue::Remove : Data not found in queue\n");
   }
   else
   {
      if(!prev)
      {
         // at head
         assert(node == head);
         head = node->next;
         if(head == nullptr)
         {
            assert(tail == node);
            tail = nullptr;
         }
      }
      else
      {
         prev->next = node->next;
         if(prev->next == nullptr)
         {
            // at tail
            assert(tail == node);
            tail = prev;
         }
      }

      delete node;
   }
}

inline qboolean Queue::Inqueue(void *data)
{
   for(QueueNode *node = head; node != nullptr; node = node->next)
   {
      if(node->data == data)
      {
         return true;
      }
   }

   return false;
}

#endif /* queue.h */

// EOF

