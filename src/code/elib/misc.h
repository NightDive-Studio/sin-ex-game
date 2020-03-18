/*
  ELib
  
  Miscellaneous utilities
  
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

#ifndef MISC_H__
#define MISC_H__

#ifdef __cplusplus
extern "C" {
#endif

//
// Portable snprintf
//

size_t pvsnprintf(char *buf, size_t buf_len, const char *s, va_list args);
size_t psnprintf(char *buf, size_t buf_len, const char *s, ...);

//
// File IO Utils
//

int     M_WriteFile(const char *filename, const void *source, size_t length);
long    M_FileLength(FILE *f);
size_t  M_ReadFile(const char *name, uint8_t **buffer);
char   *M_LoadStringFromFile(const char *filename);

//
// C library polyfills
//

char *M_Strupr(char *string);
char *M_Strlwr(char *string);
char *M_Itoa(int value, char *string, int radix);

//
// Filename and Path Utils
//

void M_NormalizeSlashes(char *str);

#ifdef __cplusplus
}
#endif

#endif

// EOF

