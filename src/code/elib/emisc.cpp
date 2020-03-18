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

#include "elib.h"
#include "misc.h"

#ifdef _WIN32
#if _MSC_VER < 1400 /* not needed for Visual Studio 2008 */
#define vsnprintf _vsnprintf
#endif
#endif

//
// Portable vsnprintf
//
size_t pvsnprintf(char *buf, size_t buf_len, const char *s, va_list args)
{
   if(buf_len < 1)
   {
      return 0;
   }
   
   // Windows (and other OSes?) has a vsnprintf() that doesn't always
   // append a trailing \0. So we must do it, and write into a buffer
   // that is one byte shorter; otherwise this function is unsafe.
   int    sresult = vsnprintf(buf, buf_len, s, args);
   size_t result;
   
   // If truncated, change the final char in the buffer to a \0.
   // A negative result indicates a truncated buffer on Windows.
   if(sresult < 0 || static_cast<size_t>(sresult) >= buf_len)
   {
      buf[buf_len - 1] = '\0';
      result = buf_len - 1;
   }
   else
      result = static_cast<size_t>(sresult);

   return result;
}

//
// Portable snprintf
//
size_t psnprintf(char *buf, size_t buf_len, const char *s, ...)
{
   va_list args;
   size_t result;

   va_start(args, s);
   result = pvsnprintf(buf, buf_len, s, args);
   va_end(args);

   return result;
}

//=============================================================================
//
// Basic File IO Utils
//

//
// Write a file from a data source
//
int M_WriteFile(const char *filename, const void *source, size_t length)
{
   FILE *fp;
   int result;

   errno = 0;

   if(!(fp = std::fopen(filename, "wb")))
      return 0;

   result = (std::fwrite(source, 1, length, fp) == length);
   std::fclose(fp);

   if(!result)
      std::remove(filename);

   return result;
}

//
// Get the length of a file
//
long M_FileLength(FILE *f)
{
   long curpos, len;
   curpos = std::ftell(f);
   std::fseek(f, 0, SEEK_END);
   len = std::ftell(f);
   std::fseek(f, 0, SEEK_SET);

   return len;
}

//
// Read in a file
//
size_t M_ReadFile(const char *name, uint8_t **buffer)
{
   FILE *fp;

   errno = 0;

   if((fp = std::fopen(name, "rb")))
   {
      size_t length = static_cast<size_t>(M_FileLength(fp));

      *buffer = ecalloc(uint8_t, 1, length);

      if(std::fread(*buffer, 1, length, fp) == length)
      {
         std::fclose(fp);
         return length;
      }
      std::fclose(fp);
   }

   return 0;
}

//
// Assume the contents of a file are a string, null-terminating the buffer
// when reading it in.
//
char *M_LoadStringFromFile(const char *filename)
{
   FILE   *f   = nullptr;
   char   *buf = nullptr;
   size_t  len = 0;

   if(!(f = std::fopen(filename, "rb")))
      return nullptr;

   // allocate at length+1 for null termination
   len = static_cast<size_t>(M_FileLength(f));
   buf = ecalloc(char, 1, len + 1);
   if(std::fread(buf, 1, len, f) != len)
      E_Printf("Warning: short read of file %s\n", filename);
   std::fclose(f);

   return buf;
}

//=============================================================================
//
// Polyfills for non-standard C functions
//

// Convert a string to uppercase in-place.
char *M_Strupr(char *string)
{
   char *s = string;

   while(*s)
   {
      int c = ectype::toUpper(*s);
      *s++ = c;
   }

   return string;
}

// Convert a string to lowercase in-place.
char *M_Strlwr(char *string)
{
   char *s = string;

   while(*s)
   {
      int c = ectype::toLower(*s);
      *s++ = c;
   }

   return string;
}

// Itoa - integer to string
char *M_Itoa(int value, char *string, int radix)
{
   int i;
   char temp[33];
   char *sp, *rover = temp;
   unsigned int v;

   // check for valid base
   if(radix <= 1 || radix > 36)
      return 0;

   int sign = (radix == 10 && value < 0);

   if(sign)
      v = -value;
   else
      v = static_cast<unsigned int>(value);

   while(v || rover == temp)
   {
      i = v % radix;
      v = v / radix;

      if(i < 10)
         *rover++ = i + '0';
      else
         *rover++ = i + 'a' - 10;
   }

   if(string == nullptr)
      string = ecalloc(char, (rover - temp) + sign + 1, 1);
   sp = string;

   if(sign)
      *sp++ = '-';

   while(rover > temp)
   {
      *sp = *--rover;
      ++sp;
   }
   *sp = '\0';

   return string;
}

//=============================================================================
//
// Filename and Path Routines
//

void M_NormalizeSlashes(char *str)
{
   char *p;
   char useSlash     = '/';  // slash type to use for normalization
   char replaceSlash = '\\'; // type of slash to replace
   bool isUNC        = false;

   if(std::strlen(str) > 2 &&
      ((str[0] == '\\' || str[0] == '/') && str[0] == str[1]))
   {
      useSlash     = '\\';
      replaceSlash = '/';
      isUNC        = true;
   }

   // convert all replaceSlash to useSlash
   for(p = str; *p; p++)
   {
      if(*p == replaceSlash)
         *p = useSlash;
   }

   // remove trailing slashes
   while(p > str && *--p == useSlash)
      *p = '\0';

   // collapse multiple slashes
   for(p = str + (isUNC ? 2 : 0); (*str++ = *p); )
   {
      if(*p++ == useSlash)
      {
         while(*p == useSlash)
            ++p;
      }
   }
}

// EOF

