/*
  ELib
  
  The MIT License (MIT)
  
  Copyright (c) 2016 James Haley
  
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

#ifndef ELIB_QSTRINGMAP_H__
#define ELIB_QSTRINGMAP_H__

#include <unordered_map>
#include "qstring.h"

template<typename T, typename C>
class qstringmap
{
public:
   C m_hashkeyfunc;

   using TMap = std::unordered_map<qstring, T, qstring::hash>;
   TMap tmap;

   qstringmap(C hashkeyfunc) : m_hashkeyfunc(hashkeyfunc), tmap() {}

   void insert(T obj)
   {
      tmap.emplace(qstring(m_hashkeyfunc(obj)), obj);
   }

   bool contains(T obj) const
   {
      return tmap.find(qstring(m_hashkeyfunc(obj))) != tmap.cend();
   }

   bool contains(const char *name) const
   {
      return tmap.find(qstring(name)) != tmap.cend();
   }

   T find(const char *name) const
   {
      TMap::const_iterator itr = tmap.find(qstring(name));
      return (itr != tmap.cend()) ? itr->second : nullptr;
   }

   void clear()
   {
      tmap.clear();
   }
};

#endif

// EOF

