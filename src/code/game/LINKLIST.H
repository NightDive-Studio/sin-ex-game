//-----------------------------------------------------------------------------
//
//  $Logfile:: /Sin/LINKLIST.H                                                $
// $Revision:: 2                                                              $
//   $Author:: Jimdose                                                        $
//     $Date:: 9/26/97 7:05p                                                  $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// 

#ifndef __linklist_h
#define __linklist_h
#ifdef __cplusplus
extern "C" {
#endif


#define NewNode(type)  ((type *)Z_Malloc(sizeof(type)))

#define LL_New(rootnode,type,next,prev) \
   {                                    \
      (rootnode) = NewNode(type);       \
      (rootnode)->prev = (rootnode);    \
      (rootnode)->next = (rootnode);    \
   }

#define LL_Add(rootnode, newnode, next, prev) \
   {                                          \
      (newnode)->next = (rootnode);           \
      (newnode)->prev = (rootnode)->prev;     \
      (rootnode)->prev->next = (newnode);     \
      (rootnode)->prev = (newnode);           \
   }

//MED
#define LL_AddFirst(rootnode, newnode, next, prev) \
   {                                               \
      (newnode)->prev = (rootnode);                \
      (newnode)->next = (rootnode)->next;          \
      (rootnode)->next->prev = (newnode);          \
      (rootnode)->next = (newnode);                \
   }

#define LL_Transfer(oldroot,newroot,next,prev) \
   {                                           \
      if(oldroot->prev != oldroot)             \
      {                                        \
         oldroot->prev->next = newroot;        \
         oldroot->next->prev = newroot->prev;  \
         newroot->prev->next = oldroot->next;  \
         newroot->prev = oldroot->prev;        \
         oldroot->next = oldroot;              \
         oldroot->prev = oldroot;              \
      }                                        \
   }

#define LL_Reverse(root,type,next,prev)                    \
   {                                                       \
      type *newend,*trav,*tprev;                           \
                                                           \
      newend = root->next;                                 \
      for(trav = root->prev; trav != newend; trav = tprev) \
      {                                                    \
         tprev = trav->prev;                               \
         LL_Move(trav,newend,next,prev);                   \
      }                                                    \
   }

#define LL_Remove(node,next,prev)    \
   {                                 \
      node->prev->next = node->next; \
      node->next->prev = node->prev; \
      node->next = node;             \
      node->prev = node;             \
   }

#define LL_SortedInsertion(rootnode,insertnode,next,prev,type,sortparm)    \
   {                                                                       \
      type *hoya;                                                          \
                                                                           \
      hoya = rootnode->next;                                               \
      while((hoya != rootnode) && (insertnode->sortparm > hoya->sortparm)) \
      {                                                                    \
         hoya = hoya->next;                                                \
      }                                                                    \
      LL_Add(hoya,insertnode,next,prev);                                   \
   }

#define LL_Move(node,newroot,next,prev) \
   {                                    \
      LL_Remove(node,next,prev);        \
      LL_Add(newroot,node,next,prev);   \
   }

#define LL_Empty(list,next,prev)   \
   (                               \
      ((list)->next == (list)) &&  \
      ((list)->prev == (list))     \
   )

#define LL_Free(list)            Z_Free(list)
#define LL_Reset(list,next,prev) (list)->next = (list)->prev = (list)

#ifdef __cplusplus
}
#endif

#endif

// EOF

