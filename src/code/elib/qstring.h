/*
  ELib
  
  Quick secure string library
  
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

#ifndef QSTRING_H__
#define QSTRING_H__

#include "elib.h"

//
// Quasar's robust, secure string class.
//
class qstring
{
private:
   char    local[16];
   char   *buffer;
   size_t  index;
   size_t  size;
   
   bool isLocal() const { return (buffer == local); }
   void unLocalize(size_t pSize);

public:
   static const size_t npos;
   static const size_t basesize;

   // Constructors / Destructor
   qstring(size_t startSize = 0) 
      : index(0), size(16)
   {
      buffer = local;
      std::memset(local, 0, sizeof(local));
      if(startSize)
         initCreateSize(startSize);
   }

   qstring(const qstring &other) 
      : index(0), size(16)
   {
      buffer = local;
      std::memset(local, 0, sizeof(local));
      copy(other);
   }

   explicit qstring(const char *cstr)
      : index(0), size(16)
   {
      buffer = local;
      std::memset(local, 0, sizeof(local));
      copy(cstr);
   }

   qstring(qstring &&other) noexcept;

   ~qstring() { freeBuffer(); }

   // Basic Property Getters

   //
   // qstring::getBuffer
   //
   // Retrieves a pointer to the internal buffer. This pointer shouldn't be 
   // cached, and is not meant for writing into (although it is safe to do so, it
   // circumvents the encapsulation and security of this structure).
   //
   char *getBuffer() { return buffer; }

   //
   // qstring::constPtr
   //
   // Like qstring::getBuffer, but casts to const to enforce safety.
   //
   const char *constPtr() const { return buffer; }

   //
   // qstring::length
   //
   // Because the validity of "index" is maintained by all insertion and editing
   // functions, we can bypass calling strlen.
   //
   size_t length() const { return index; }

   //
   // qstring::empty
   //
   // For C++ std::string compatibility.
   //
   bool empty() const { return (index == 0); }

   //
   // qstring::getSize
   //
   // Returns the amount of size allocated for this qstring (will be >= strlen).
   // You are allowed to index into the qstring up to size - 1, although any bytes
   // beyond the strlen will be zero.
   //
   size_t getSize() const { return size; }

   // Initialization and Resizing
   qstring &initCreate();
   qstring &initCreateSize(size_t size);
   qstring &createSize(size_t size);
   qstring &create();
   qstring &grow(size_t len);
   qstring &clear();
   qstring &clearOrCreate(size_t size);
   void     freeBuffer();

   // Indexing operations
   char  charAt(size_t idx) const;
   char *bufferAt(size_t idx);

   unsigned char ucharAt(size_t idx) const { return (unsigned char)charAt(idx); }

   // Concatenation and insertion/deletion
   qstring &push(char ch);
   qstring &pop();
   qstring &concat(const char *str);
   qstring &concat(const qstring &src);
   qstring &insert(const char *insertstr, size_t pos);

   // Comparisons: C and C++ style
   int  strCmp(const char *str) const;
   int  strNCmp(const char *str, size_t maxcount) const;
   int  strCaseCmp(const char *str) const;
   int  strNCaseCmp(const char *str, size_t maxcount) const;
   bool compare(const char *str) const;
   bool compare(const qstring &other) const;

   bool operator < (const qstring &other) const { return strCmp(other.constPtr()) < 0; }
   bool operator > (const qstring &other) const { return strCmp(other.constPtr()) > 0; }

   // Hashing
   static unsigned int HashCodeStatic(const char *str);
   static unsigned int HashCodeCaseStatic(const char *str);

   unsigned int hashCode() const;      // case-insensitive
   unsigned int hashCodeCase() const;  // case-considering

   struct hash
   {
      size_t operator ()(const qstring &qstr) const { return qstr.hashCode(); }
   };

   // Copying and Swapping
   qstring &copy(const char *str);
   qstring &copy(const char *str, size_t count);
   qstring &copy(const qstring &src);
   char    *copyInto(char *dest, size_t size) const;
   qstring &copyInto(qstring &dest) const;
   void     swapWith(qstring &str2);

   // In-Place Case Conversions
   qstring &toUpper();
   qstring &toLower();

   // Substring Replacements
   size_t replace(const char *filter, char repl);
   size_t replaceNotOf(const char *filter, char repl);
   
   // File Path Utilities
   qstring &normalizeSlashes();
   qstring &pathConcatenate(const char *addend);
   qstring &pathConcatenate(const qstring &other);
   qstring &addDefaultExtension(const char *ext);
   qstring &removeFileSpec();
   void     extractFileBase(qstring &dest);

   // strdup wrappers
   char *duplicate() const;

   // Numeric Conversions
   int    toInt() const;
   long   toLong(char **endptr, int radix) const;
   double toDouble(char **endptr) const;

   // Searching/Substring Finding Routines
   const char *strChr(char c) const;
   const char *strRChr(char c) const;
   size_t findFirstOf(char c) const;
   size_t findFirstNotOf(char c) const;
   size_t findLastOf(char c) const;
   const char *findSubStr(const char *substr) const;
   const char *findSubStrNoCase(const char *substr) const;
   size_t find(const char *s, size_t pos = 0) const;

   // Stripping and Truncation
   qstring &lstrip(char c);
   qstring &rstrip(char c);
   qstring &truncate(size_t pos);
   qstring &erase(size_t pos, size_t n = npos);

   // Special Formatting 
   qstring &makeQuoted();
   size_t   printf(size_t maxlen, const char *fmt, ...);

   // Operators
   bool     operator == (const qstring &other) const;
   bool     operator == (const char    *other) const;
   bool     operator != (const qstring &other) const;
   bool     operator != (const char    *other) const;
   qstring &operator  = (const qstring &other);
   qstring &operator  = (const char    *other);
   qstring &operator += (const qstring &other);
   qstring &operator += (const char    *other);
   qstring &operator += (char  ch);
   qstring &operator << (const qstring &other);
   qstring &operator << (const char    *other);
   qstring &operator << (char   ch);
   qstring &operator << (int    i);
   qstring &operator << (double d);
   
   char       &operator [] (size_t idx);
   const char &operator [] (size_t idx) const;
};

#endif

// EOF

