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

#ifndef ZONE_H__
#define ZONE_H__

#ifdef __cplusplus
extern "C" {
#endif

void *E_Malloc(size_t size);
void *E_Calloc(size_t count, size_t size);
void *E_Realloc(void *ptr, size_t size);
char *E_Strdup(const char *str);
void  E_Free(void *ptr);

#ifdef __cplusplus
}
#endif

#define ecalloc(type, count, size) (type *)(E_Calloc(count, size))
#define emalloc(type, size)        (type *)(E_Malloc(size))
#define erealloc(type, ptr, size)  (type *)(E_Realloc(ptr, size))
#define estructalloc(type, num)    (type *)(E_Calloc(num, sizeof(type)))
#define estrdup(str)               E_Strdup(str)
#define efree(ptr)                 E_Free(ptr)

#endif

// EOF

