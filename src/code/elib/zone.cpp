/*
  ELib
  
  Safe memory allocation
  
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

#include "elib.h"

void *E_Malloc(size_t size)
{
   void *ret;

   if(!(ret = std::malloc(size)))
      E_Error("E_Malloc: failed on allocation of %lu bytes", size);

   return ret;
}

void *E_Calloc(size_t count, size_t size)
{
   void *ret;

   if(!(ret = std::calloc(count, size)))
      E_Error("E_Calloc: failed on allocation of %lu bytes", count*size);

   return ret;
}

void *E_Realloc(void *ptr, size_t size)
{
   void *ret;

   if(!(ret = std::realloc(ptr, size)))
      E_Error("E_Realloc: failed on allocation of %lu bytes", size);

   return ret;
}

char *E_Strdup(const char *str)
{
   char *ret = nullptr;

   if((ret = static_cast<char *>(E_Calloc(1, std::strlen(str) + 1))))
      return std::strcpy(ret, str);

   return ret;
}

void E_Free(void *ptr)
{
   if(!ptr)
      E_Error("E_Free: attempt to free null pointer");

   std::free(ptr);
}

// EOF

