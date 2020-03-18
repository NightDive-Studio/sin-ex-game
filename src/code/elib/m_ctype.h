/*
  ELib
  
  ctype.h replacement functions
  
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

#ifndef M_CTYPE_H__
#define M_CTYPE_H__

//
// Namespace for Eternity Engine character type functions.
//
namespace ectype
{
   // Return 1 if c is a control character, 0 otherwise.
   inline int isCntrl(int c)
   {
      return (c >= 0x00 && c <= 0x1f);
   }

   // Return 1 if c is a blank character, 0 otherwise.
   inline int isBlank(int c)
   {
      return (c == '\t' || c == ' ');
   }

   // Return 1 if c is a whitespace character, 0 otherwise.
   inline int isSpace(int c)
   {
      return ((c >= '\t' && c <= '\r') || c == ' ');
   }

   // Return 1 if c is an uppercase alphabetic character, 0 otherwise.
   inline int isUpper(int c)
   {
      return (c >= 'A' && c <= 'Z');
   }

   // Return 1 if c is a lowercase alphabetic character, 0 otherwise.
   inline int isLower(int c)
   {
      return (c >= 'a' && c <= 'z');
   }

   // Return 1 if c is an alphabetic character, 0 otherwise.
   inline int isAlpha(int c)
   {
      return (isUpper(c) || isLower(c));
   }

   // Return 1 if c is a base 10 numeral, 0 otherwise.
   inline int isDigit(int c)
   {
      return (c >= '0' && c <= '9');
   }

   // Return 1 if c is a base 16 numeral, 0 otherwise.
   inline int isXDigit(int c)
   {
      return (isDigit(c) || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'));
   }

   // Return 1 if c is either a base 10 digit or an alphabetic character.
   // Return 0 otherwise.
   inline int isAlnum(int c)
   {
      return (isAlpha(c) || isDigit(c));
   }

   // Return 1 if c is a punctuation mark, 0 otherwise.
   inline int isPunct(int c)
   {
      return ((c >= '!' && c <= '/') || (c >= ':' && c <= '@') || 
              (c >= '[' && c <= '`') || (c >= '{' && c <= '~'));
   }

   // Return 1 if c is a character with a graphical glyph.
   // Return 0 otherwise.
   inline int isGraph(int c)
   {
      return (c >= '!' && c <= '~');
   }

   // Return 1 if c is a printable character, 0 otherwise.
   inline int isPrint(int c)
   {
      return (c >= ' ' && c <= '~');
   }

   // Eternity-specific; return 1 if c is an extended ASCII character, 
   // and 0 otherwise.
   inline int isExtended(int c)
   {
      return (c < 0 || c > 0x7f);
   }

   // Convert lowercase alphabetics to uppercase; leave other inputs alone.
   inline int toUpper(int c)
   {
      return (isLower(c) ? c - 'a' + 'A' : c);
   }

   // Convert uppercase alphabetics to lowercase; leave other inputs alone.
   inline int toLower(int c)
   {
      return (isUpper(c) ? c - 'A' + 'a' : c);
   }
}

#endif

// EOF

