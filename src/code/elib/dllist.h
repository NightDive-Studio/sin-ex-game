/*
  CALICO

  Generic linked list, useful for hash chaining

  The MIT License (MIT)

  Copyright (C) 2016 James Haley

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#pragma once

// 
// DLListItem
//
// This template class is an evolution of the original mdllistitem_t.
// However rather than using an is-a relationship, this functions best
// in a has-a relationship (which is the same role it could already
// play via use of the object member pointer).
//
// This class is intentionally a POD and will most likely remain that way
// for speed and efficiency concerns.
//
template<typename T> 
class DLListItem
{
public:
   DLListItem<T>  *dllNext   = nullptr;
   DLListItem<T> **dllPrev   = nullptr;
   T              *dllObject = nullptr; // 08/02/09: pointer back to object
   unsigned int    dllData;             // 02/07/10: arbitrary data cached at node

   void insert(T *parentObject, DLListItem<T> **head)
   {
      DLListItem<T> *next = *head;

      if((dllNext = next))
         next->dllPrev = &dllNext;
      dllPrev = head;
      *head = this;

      dllObject = parentObject; // set to object, which is generally distinct
   }

   void remove()
   {
      DLListItem<T> **prev = dllPrev;
      DLListItem<T>  *next = dllNext;

      // haleyjd 05/07/13: safety #1: only if prev is non-null
      if(prev && (*prev = next))
         next->dllPrev = prev;

      // haleyjd 05/07/13: safety #2: clear links.
      dllPrev = nullptr;
      dllNext = nullptr;
   }
};

//
// DLList
//
// haleyjd 05/07/13: Added a list type which makes use of DLListItem more
// regulated. Use is strictly optional. Provide the type and a member to
// pointer to the DLListItem field in the class the list will use for links.
//
template<typename T, DLListItem<T> T::* link> 
class DLList
{
public:
   DLListItem<T> *head = nullptr;

   void insert(T *object) { (object->*link).insert(object, &head); }
   void remove(T *object) { (object->*link).remove();              }
   void insert(T &object) { insert(&object);                       }
   void remove(T &object) { remove(&object);                       }

   bool empty() const { return head == nullptr; }
};

// EOF
