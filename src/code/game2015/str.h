//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/str.h                            $
// $Revision:: 6                                                              $
//   $Author:: Jimdose                                                        $
//     $Date:: 10/25/98 11:58p                                                $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Simple, DLL portable string class
// 

#ifndef __STR_H__
#define __STR_H__

#include <assert.h>
#include <string.h>

class str
{
protected:
   int               len;
   char             *data;

public:
   str();
   str(const char *text);
   str(const str &text);
   str(str &&text);
   str(const str &string, int start, int end);
   ~str();

   int         length() const { return len; }

   const char *c_str() const;

   void     append(const char *text);
   void     append(const str& text);

   char     operator [] (int index) const;
   char    &operator [] (int index);

   str     &operator = (const str &text);
   str     &operator = (str &&text);
   str     &operator = (const char *text);

   friend str operator + (const str &a, const str &b);
   friend str operator + (const str &a, const char *b);
   friend str operator + (const char *a, const str &b);

   str &operator += (const str &a);
   str &operator += (const char *a);

   friend int operator == (const str &a, const str &b);
   friend int operator == (const str &a, const char *b);
   friend int operator == (const char *a, const str &b);

   friend int operator != (const str &a, const str &b);
   friend int operator != (const str &a, const char *b);
   friend int operator != (const char *a, const str &b);
};

inline str::~str()
{
   if(data)
   {
      delete [] data;
      data = nullptr;
   }
}

inline str::str()
{
   data = new char [1]; // SINEX_FIXME: efficiency - allocate a default chunk size
   data[0] = '\0';
   len = 0;
}

inline str::str(const char *text)
{
   assert(text);

   if(text)
   {
      len = strlen(text);
      data = new char [len + 1];
      strcpy(data, text);
   }
   else
   {
      data = new char [1]; // SINEX_FIXME: efficiency - allocate a default chunk size
      data[0] = 0;
      len = 0;
   }
}

inline str::str(const str &text)
{
   len = text.len;
   data = new char [len + 1];
   strcpy(data, text.c_str());
}

inline str::str(str &&text)
{
    len = text.len;
    data = text.data;
    text.data = nullptr;
}

inline str::str(const str &text, int start, int end)
{
   int i;

   if(end > text.len)
   {
      end = text.len;
   }

   if(start > text.len)
   {
      start = text.len;
   }

   len = end - start;
   if(len < 0)
   {
      len = 0;
   }

   data = new char [len + 1];
   for(i = 0; i < len; i++)
   {
      data[i] = text[start + i];
   }

   data[len] = 0;
}

inline const char *str::c_str() const
{
   assert(data);
   return data;
}

inline void str::append(const char *text)
{
   char *olddata;

   assert(text);
   assert(data);

   if(text)
   {
      olddata = data;

      len = strlen(data) + strlen(text);
      data = new char [len + 1];

      strcpy(data, olddata);
      strcat(data, text);

      if(olddata)
      {
         delete [] olddata;
         olddata = nullptr;
      }
   }
}

inline void str::append(const str &text)
{
   assert(data);
   append(text.c_str());
}

inline char str::operator [] (int index) const
{
   assert(data);

   // don't include the '/0' in the test, because technically, it's out of bounds
   assert((index >= 0) && (index < len));

   // In release mode, give them a null character
   // don't include the '/0' in the test, because technically, it's out of bounds
   if((index < 0) || (index >= len))
   {
      return 0;
   }

   return data[index];
}

inline char &str::operator [] (int index)
{
   // Used for result for invalid indices
   static char dummy = 0;

   assert(data);

   // don't include the '/0' in the test, because technically, it's out of bounds
   assert((index >= 0) && (index < len));

   // In release mode, let them change a safe variable
   // don't include the '/0' in the test, because technically, it's out of bounds
   if((index < 0) || (index >= len))
   {
      return dummy;
   }

   return data[index];
}

inline str &str::operator = (const str &text)
{
   char *temp;

   assert(data);

   len = text.len;
   temp = new char [len + 1];
   strcpy(temp, text.c_str());

   // SINEX_FIXME: efficiency - bad reallocation policy for strings with sufficient length

   // we don't destroy the old data until we've allocated the new one
   // in case we are assigning the string from data inside the old string.
   if(data)
   {
      delete [] data;
   }

   data = temp;
   return *this;
}

inline str &str::operator = (str &&text)
{
    len  = text.len;
    data = text.data;
    text.data = nullptr;
    return *this;
}

inline str &str::operator = (const char *text)
{
   char *temp;

   assert(data);
   assert(text);

   if(!text)
   {
      // safe behaviour if NULL
      len = 0;
      temp = new char[1];
      temp[0] = 0;
   }
   else
   {
      len = strlen(text);
      temp = new char[len + 1];
      strcpy(temp, text);
   }

   // SINEX_FIXME: efficiency - bad reallocation policy for strings with sufficient length

   // we don't destroy the old data until we've allocated the new one
   // in case we are assigning the string from data inside the old string.
   if(data)
   {
      delete [] data;
   }

   data = temp;
   return *this;
}

inline str operator + (const str &a, const str &b)
{
   str result { a };

   result.append(b);

   return result;
}

inline str operator + (const str &a, const char *b)
{
   str result { a };

   result.append(b);

   return result;
}

inline str operator + (const char *a, const str &b)
{
   str result { a };

   result.append(b);

   return result;
}

inline str &str::operator += (const str &a)
{
   assert(data);
   append(a);
   return *this;
}

inline str &str::operator += (const char *a)
{
   assert(data);
   append(a);
   return *this;
}

inline int operator == (const str &a, const str &b)
{
   return (!strcmp(a.c_str(), b.c_str()));
}

inline int operator == (const str &a, const char *b)
{
   assert(b);
   if(!b)
   {
      return false;
   }
   return (!strcmp(a.c_str(), b));
}

inline int operator == (const char *a, const str &b)
{
   assert(a);
   if(!a)
   {
      return false;
   }
   return (!strcmp(a, b.c_str()));
}

inline int operator != (const str &a, const str &b)
{
   return !(a == b);
}

inline int operator != (const str &a, const char *b)
{
   return !(a == b);
}

inline int operator != (const char *a, const str &b)
{
   return !(a == b);
}

// Test function
void TestStringClass();

#endif

// EOF

