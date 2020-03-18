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

#include "elib.h"
//#include "../hal/hal_platform.h"
#include "misc.h"
#include "qstring.h"

const size_t qstring::npos = ((size_t) -1);
const size_t qstring::basesize = 16;

//=============================================================================
//
// Constructors, Destructors, and Reinitializers
//

void qstring::unLocalize(size_t pSize)
{
   if(isLocal())
   {
      buffer = ecalloc(char, 1, pSize);
      size   = pSize;
      strcpy(buffer, local);

      memset(local, 0, basesize);
   }
}

//
// Creates a qstring with a given initial size, which helps prevent
// unnecessary initial reallocations. Resets insertion point to zero.
// This is safe to call on an existing qstring to reinitialize it.
//
qstring &qstring::createSize(size_t pSize)
{
   // Can remain local?
   if(isLocal() && pSize <= basesize)
      clear();
   else
   {
      unLocalize(pSize);

      // Don't realloc if not needed
      if(size < pSize)
      {
         buffer = erealloc(char, buffer, pSize);
         size   = pSize;
      }
      clear();
   }

   return *this;
}

//
// Gives the qstring a buffer of the default size and initializes
// it to zero. Resets insertion point to zero. This is safe to call
// on an existing qstring to reinitialize it.
//
qstring &qstring::create()
{
   return createSize(basesize);
}

//
// Initializes a qstring struct to all zeroes, then calls
// qstring::create. This is for safety the first time a qstring
// is created (if qstr::buffer is uninitialized, realloc will
// crash).
//
qstring &qstring::initCreate()
{
   buffer = local;
   size   = basesize;
   clear();

   return create();
}

//
// Initialization and creation with size specification.
//
qstring &qstring::initCreateSize(size_t pSize)
{
   buffer = local;
   size   = basesize;
   clear();

   return createSize(pSize);
}

//
// Frees the qstring object's buffer, if it is not localized. The qstring
// returns to being localized at that point, and will be empty.
//
void qstring::freeBuffer()
{
   if(buffer && !isLocal())
      efree(buffer);

   // return to being local
   buffer = local;
   size   = basesize;
   clear();
}

//
// haleyjd 05/22/2013: Enable C++11 move semantics for qstring instances.
// Required for efficiency when using qstring with Collection<T>.
//
qstring::qstring(qstring &&other) noexcept
   : index(0), size(16)
{
   // When other is not localized, take direct ownership of its buffer
   if(!other.isLocal())
   {
      buffer = other.buffer;
      index  = other.index;
      size   = other.size;
      std::memset(local, 0, sizeof(local));

      // leave the other object in a usable state, it's not necessarily dead.
      other.buffer = nullptr;
      other.freeBuffer(); // returns to being localized
   }
   else
   {
      // Copy the local buffer
      std::memcpy(local, other.local, sizeof(local));
      buffer = local;
      index  = other.index;
   }
}

//=============================================================================
//
// Basic Properties
//

//
// Indexing function to access a character in a qstring. This is slower but 
// more secure than using qstring::getBuffer with array indexing.
//
char qstring::charAt(size_t idx) const
{
   if(idx >= size)
      E_Error("qstring::charAt: index out of range");

   return buffer[idx];
}

//
// Gets a pointer into the buffer at the given position, if that position would
// be valid. Returns NULL otherwise. The same caveats apply as with qstring::getBuffer.
//
char *qstring::bufferAt(size_t idx)
{
   return idx < size ? buffer + idx : NULL;
}

//
// Read-write variant.
//
char &qstring::operator [] (size_t idx)
{
   if(idx >= size)
      E_Error("qstring::operator []: index out of range");

   return buffer[idx];
}

//
// Read-only variant.
//
const char &qstring::operator [] (size_t idx) const
{
   if(idx >= size)
      E_Error("qstring::operator []: index out of range");

   return buffer[idx];
}


//=============================================================================
//
// Basic String Ops
//

//
// Sets the entire qstring buffer to zero, and resets the insertion index. 
// Does not reallocate the buffer.
//
qstring &qstring::clear()
{
   std::memset(buffer, 0, size);
   index = 0;

   return *this;
}

//
// Creates a qstring, or clears it if it is already valid.
//
qstring &qstring::clearOrCreate(size_t pSize)
{
   return createSize(pSize);
}

//
// Grows the qstring's buffer by the indicated amount. This is automatically
// called by other qstring methods, so there is generally no need to call it 
// yourself.
//
qstring &qstring::grow(size_t len)
{   
   if(len > 0)
   {
      size_t newsize = size + len;

      if(isLocal()) // are we local?
      {
         if(newsize > basesize) // can we stay local?
            unLocalize(newsize);
      }
      else
      {
         buffer = erealloc(char, buffer, newsize);
         std::memset(buffer + size, 0, len);
         size += len;
      }
   }

   return *this;
}

//=============================================================================
// 
// Concatenation and Insertion/Deletion/Copying Functions
//

//
// Adds a character to the end of the qstring, reallocating via buffer doubling
// if necessary.
//
qstring &qstring::push(char ch)
{
   if(index >= size - 1) // leave room for \0
      grow(size);        // double buffer size

   buffer[index++] = ch;

   return *this;
}

//
// Overloaded += for characters
//
qstring &qstring::operator += (char ch)
{
   return push(ch);
}

//
// Deletes a character from the end of the qstring.
//
qstring &qstring::pop()
{
   if(index > 0)
   {
      index--;
      buffer[index] = '\0';
   }

   return *this;
}

//
// Concatenates a C string onto the end of a qstring, expanding the buffer if
// necessary.
//
qstring &qstring::concat(const char *str)
{
   size_t cursize = size;
   size_t newsize = index + std::strlen(str) + 1;

   if(newsize > cursize)
      grow(newsize - cursize);

   std::strcat(buffer, str);

   index = std::strlen(buffer);

   return *this;
}

//
// Concatenates a qstring to a qstring. Convenience routine.
//
qstring &qstring::concat(const qstring &src)
{
   return concat(src.buffer);
}

//
// Overloaded += for const char *
//
qstring &qstring::operator += (const char *other)
{
   return concat(other);
}

//
// Overloaded += for qstring &
//
qstring &qstring::operator += (const qstring &other)
{
   return concat(other);
}

//
// Inserts a string at a given position in the qstring. If the
// position is outside the range of the string, an error will occur.
//
qstring &qstring::insert(const char *insertstr, size_t pos)
{
   char *insertpoint;
   size_t charstomove;
   size_t insertstrlen = std::strlen(insertstr);
   size_t totalsize    = index + insertstrlen + 1; 
   
   // pos must be between 0 and dest->index - 1
   if(pos >= index)
      E_Error("qstring::insert: position out of range");

   // grow the buffer to hold the resulting string if necessary
   if(totalsize > size)
      grow(totalsize - size);

   insertpoint = buffer + pos;
   charstomove = index  - pos;

   // use memmove for absolute safety
   std::memmove(insertpoint + insertstrlen, insertpoint, charstomove);
   std::memmove(insertpoint, insertstr, insertstrlen);

   index = std::strlen(buffer);

   return *this;
}

//
// Copies a C string into the qstring. The qstring is cleared first,
// and then set to the contents of *str.
//
qstring &qstring::copy(const char *str)
{
   if(index > 0)
      clear();
   
   return concat(str);
}

//
// Copies at most count bytes from the C string into the qstring.
//
qstring &qstring::copy(const char *str, size_t count)
{
   if(index > 0)
      clear();

   size_t newsize = count + 1;

   if(newsize > size)
      grow(newsize - size);

   std::strncpy(buffer, str, count);

   index = std::strlen(buffer);

   return *this;
}

//
// Overloaded for qstring &
//
qstring &qstring::copy(const qstring &src)
{
   if(index > 0)
      clear();

   return concat(src);
}

//
// Assignment from a qstring &
//
qstring &qstring::operator = (const qstring &other)
{
   return copy(other);
}

//
// Assignment from a const char *
//
qstring &qstring::operator = (const char *other)
{
   return copy(other);
}

//
// Copies the qstring into a C string buffer.
//
char *qstring::copyInto(char *dest, size_t pSize) const
{
   return std::strncpy(dest, buffer, pSize);
}

//
// Copies one qstring into another.
//
qstring &qstring::copyInto(qstring &dest) const
{
   if(dest.index > 0)
      dest.clear();
   
   return dest.concat(*this);
}

//
// Exchanges the contents of two qstrings.
//
void qstring::swapWith(qstring &str2)
{
   char   *tmpbuffer;
   size_t tmpsize;
   size_t tmpindex;

   // Both must be unlocalized.
   unLocalize(size);
   str2.unLocalize(str2.size);

   tmpbuffer = this->buffer; // make a shallow copy
   tmpsize   = this->size;
   tmpindex  = this->index;

   this->buffer = str2.buffer;
   this->size   = str2.size;
   this->index  = str2.index;

   str2.buffer = tmpbuffer;
   str2.size   = tmpsize;
   str2.index  = tmpindex;
}

//
// Removes occurrences of a specified character at the beginning of a qstring.
//
qstring &qstring::lstrip(char c)
{
   size_t i   = 0;
   size_t len = index;

   while(buffer[i] != '\0' && buffer[i] == c)
      ++i;

   if(i)
   {
      if((len -= i) == 0)
         clear();
      else
      {
         std::memmove(buffer, buffer + i, len);
         std::memset(buffer + len, 0, size - len);
         index -= i;
      }
   }

   return *this;
}

//
// Removes occurrences of a specified character at the end of a qstring.
//
qstring &qstring::rstrip(char c)
{
   while(index && buffer[index - 1] == c)
      pop();

   return *this;
}

//
// Truncates the qstring to the indicated position.
//
qstring &qstring::truncate(size_t pos)
{
   // pos must be between 0 and qstr->index - 1
   if(pos >= index)
      E_Error("qstring::truncate: position out of range");

   std::memset(buffer + pos, 0, index - pos);
   index = pos;

   return *this;
}

//
// std::string-compatible erasure function.
//
qstring &qstring::erase(size_t pos, size_t n)
{
   // truncate handles the case of n == qstring::npos
   if(!n)
      return *this;
   else if(n == npos)
      return truncate(pos);

   // pos must be between 0 and qstr->index - 1
   if(pos >= index)
      E_Error("qstring::erase: position out of range");

   size_t endPos = pos + n;
   if(endPos > index)
      endPos = index;

   char *to   = buffer + pos;
   char *from = buffer + endPos;
   char *end  = buffer + index;

   while(to != end)
   {
      *to = *from;
      ++to;
      if(from != end)
         ++from;
   }

   index -= (endPos - pos);
   return *this;
}


//=============================================================================
//
// Stream Insertion Operators
//

qstring &qstring::operator << (const qstring &other)
{
   return concat(other);
}

qstring &qstring::operator << (const char *other)
{
   return concat(other);
}

qstring &qstring::operator << (char ch)
{
   return push(ch);
}

qstring &qstring::operator << (int i)
{
   char buf[33];
   M_Itoa(i, buf, 10);
   return concat(buf);
}

qstring &qstring::operator << (double d)
{
   char buf[1079]; // srsly...
   psnprintf(buf, sizeof(buf), "%f", d);
   return concat(buf);
}

//=============================================================================
//
// Comparison Functions
//

//
// C-style string comparison/collation ordering.
//
int qstring::strCmp(const char *str) const
{
   return std::strcmp(buffer, str);
}

//
// C-style string compare with length limitation.
//
int qstring::strNCmp(const char *str, size_t maxcount) const
{
   return std::strncmp(buffer, str, maxcount);
}

//
// Case-insensitive C-style string compare.
//
int qstring::strCaseCmp(const char *str) const
{
   return strcasecmp(buffer, str);
}

//
// Case-insensitive C-style compare with length limitation.
//
int qstring::strNCaseCmp(const char *str, size_t maxcount) const
{
   return strncasecmp(buffer, str, maxcount);
}

// 
// C++ style comparison. True return value means it is equal to the argument.
//
bool qstring::compare(const char *str) const
{
   return !std::strcmp(buffer, str);
}

// 
// Overload for qstrings.
//
bool qstring::compare(const qstring &other) const
{
   return !std::strcmp(buffer, other.buffer);
}

//
// Overloaded comparison operator for const char *
//
bool qstring::operator == (const char *other) const
{
   return !std::strcmp(buffer, other);
}

//
// Overloaded comparison operator for qstring &
//
bool qstring::operator == (const qstring &other) const
{
   return !strcmp(buffer, other.buffer);
}

//
// Overloaded comparison operator for const char *
//
bool qstring::operator != (const char *other) const
{
   return std::strcmp(buffer, other) != 0;
}

//
// Overloaded comparison operator for qstring &
//
bool qstring::operator != (const qstring &other) const
{
   return std::strcmp(buffer, other.buffer) != 0;
}

//=============================================================================
//
// Hash Code Functions
//
// These are just convenience wrappers.
//

//
// Static version, for convenience and so that the convention of hashing a
// null pointer to 0 hash code is enforceable without special checks, even
// if the thing being hashed isn't a qstring instance.
//
unsigned int qstring::HashCodeStatic(const char *str)
{
   auto ustr = reinterpret_cast<const unsigned char *>(str);
   int c;
   unsigned int h = 0;

   while((c = *ustr++))
      h = ectype::toUpper(c) + (h << 6) + (h << 16) - h;

   return h;
}

//
// As above, but with case sensitivity.
//
unsigned int qstring::HashCodeCaseStatic(const char *str)
{
   auto ustr = reinterpret_cast<const unsigned char *>(str);
   int c;
   unsigned int h = 0;

   while((c = *ustr++))
      h = c + (h << 6) + (h << 16) - h;

   return h;
}

//
// Calls the standard D_HashTableKey that is used for the vast majority of
// string hash code computations in Eternity.
//
unsigned int qstring::hashCode() const
{
   return HashCodeStatic(buffer);
}

//
// Returns a hash code computed with the case of characters being treated as
// relevant to the computation.
//
unsigned int qstring::hashCodeCase() const
{
   return HashCodeCaseStatic(buffer);
}

//=============================================================================
//
// Search Functions
//

//
// Calls strchr on the qstring ("find first of", C-style).
//
const char *qstring::strChr(char c) const
{
   return std::strchr(buffer, c);
}

//
// Calls strrchr on the qstring ("find last of", C-style)
//
const char *qstring::strRChr(char c) const
{
   return std::strrchr(buffer, c);
}

//
// Finds the first occurance of a character in the qstring and returns its 
// position. Returns qstring_npos if not found.
//
size_t qstring::findFirstOf(char c) const
{
   const char *rover = buffer;
   bool found = false;
   
   while(*rover)
   {
      if(*rover == c)
      {
         found = true;
         break;
      }
      ++rover;
   }

   return found ? rover - buffer : npos;
}

//
// Finds the first occurance of a character in the qstring which does not
// match the provided character. Returns qstring_npos if not found.
//
size_t qstring::findFirstNotOf(char c) const
{
   const char *rover = buffer;
   bool found = false;

   while(*rover)
   {
      if(*rover != c)
      {
         found = true;
         break;
      }
      ++rover;
   }

   return found ? rover - buffer : npos;
}

//
// Find the last occurrance of a character in the qstring which matches
// the provided character. Returns qstring::npos if not found.
//
size_t qstring::findLastOf(char c) const
{
   const char *rover;
   bool found = false;
   
   if(!index)
      return npos;
   
   rover = buffer + index - 1;
   do
   {
      if(*rover == c)
      {
         found = true;
         break;
      }
   }
   while((rover == buffer) ? false : (--rover, true));

   return found ? rover - buffer : npos;
}

//
// Calls strstr on the qstring. If the passed-in string is found, then the
// return value points to the location of the first instance of that substring.
//
const char *qstring::findSubStr(const char *substr) const
{
   return std::strstr(buffer, substr);
}

//
// haleyjd 10/28/11: call strcasestr on the qstring, courtesy of implementation
// of the non-standard routine adapted from GNUlib.
//
const char *qstring::findSubStrNoCase(const char *substr) const
{
   // WARNING: unimplemented (no closed-source-compatible implementation available)
   //return M_StrCaseStr(buffer, substr);
   return "";
}

//
// std::string-compatible find function.
//
size_t qstring::find(const char *s, size_t pos) const
{
   // pos must be between 0 and index - 1
   if(pos >= index)
      E_Error("qstring::find: position out of range");

   char *base   = buffer + pos;
   char *substr = std::strstr(base, s);
   
   return substr ? substr - buffer : npos;
}

//=============================================================================
//
// Conversion Functions
//

//
// Integers
//

//
// Returns the qstring converted to an integer via atoi.
//
int qstring::toInt() const
{
   return std::atoi(buffer);
}

//
// Returns the qstring converted to a long integer via strtol.
//
long qstring::toLong(char **endptr, int radix) const
{
   return std::strtol(buffer, endptr, radix);
}

//
// Floating Point
//

//
// Calls strtod on the qstring.
//
double qstring::toDouble(char **endptr) const
{
   return std::strtod(buffer, endptr);
}

//
// Other String Types
//

//
// Creates a C string duplicate of a qstring's contents.
//
char *qstring::duplicate() const
{
   return estrdup(buffer);
}

//=============================================================================
//
// Case Handling
//

//
// qstring::toLower
//
// Converts the string to lowercase.
//
qstring &qstring::toLower()
{
   M_Strlwr(buffer);
   return *this;
}

//
// qstring::toUpper
//
// Converts the string to uppercase.
//
qstring &qstring::toUpper()
{
   M_Strupr(buffer);
   return *this;
}

//=============================================================================
//
// Replacement Operations
//

typedef uint8_t byte;

static byte qstr_repltable[256];

//
// Static routine for replacement functions.
//
static size_t QStrReplaceInternal(qstring *qstr, char repl)
{
   size_t repcount = 0;
   byte *rptr = (byte *)(qstr->getBuffer());

   // now scan through the qstring buffer and replace any characters that
   // match characters in the filter table.
   while(*rptr)
   {
      if(qstr_repltable[*rptr])
      {
         *rptr = (byte)repl;
         ++repcount; // count characters replaced
      }
      ++rptr;
   }

   return repcount;
}

//
// Replaces characters in the qstring that match any character in the filter
// string with the character specified by the final parameter.
//
size_t qstring::replace(const char *filter, char repl)
{
   const unsigned char *fptr = (unsigned char *)filter;

   std::memset(qstr_repltable, 0, sizeof(qstr_repltable));

   // first scan the filter string and build the replacement filter table
   while(*fptr)
      qstr_repltable[*fptr++] = 1;

   return QStrReplaceInternal(this, repl);
}

//
// As above, but replaces all characters NOT in the filter string.
//
size_t qstring::replaceNotOf(const char *filter, char repl)
{
   const unsigned char *fptr = (unsigned char *)filter;
   
   std::memset(qstr_repltable, 1, sizeof(qstr_repltable));

   // first scan the filter string and build the replacement filter table
   while(*fptr)
      qstr_repltable[*fptr++] = 0;

   return QStrReplaceInternal(this, repl);
}

//=============================================================================
//
// File Path Specific Routines
//

//
// Calls M_NormalizeSlashes on a qstring, which replaces \ characters with /
// and eliminates any duplicate slashes. This isn't simply a convenience
// method, as the qstring structure requires a fix-up after this function is
// used on it, in order to keep the string length correct.
//
qstring &qstring::normalizeSlashes()
{
   M_NormalizeSlashes(buffer);
   index = std::strlen(buffer);

   return *this;
}

//
// Concatenate a C string assuming the qstring's current contents are a file
// path. Slashes will be normalized.
//
qstring &qstring::pathConcatenate(const char *addend)
{
   // Only add a slash if this is not the initial path component.
   if(index > 0)
      *this += '/';

   *this += addend;
   normalizeSlashes();

   return *this;
}

//
// Convenience overload for qstring
//
qstring &qstring::pathConcatenate(const qstring &other)
{
   return pathConcatenate(other.constPtr());
}

//
// Similar to M_AddDefaultExtension, but for qstrings.
// Note: an empty string will not be modified.
//
qstring &qstring::addDefaultExtension(const char *ext)
{
   char *p = buffer;

   if(p && index > 0)
   {
      p = p + index - 1;  // no need to seek for \0 here
      while(p-- > buffer && *p != '/' && *p != '\\')
      {
         if(*p == '.')
            return *this; // has an extension already.
      }
      if(*ext != '.') // need a dot?
         *this += '.';
      *this += ext;   // add the extension
   }

   return *this;
}

//
// Removes a filespec from a path.
// If called on a path without a file, the last path component is removed.
//
qstring &qstring::removeFileSpec()
{
   size_t lastSlash;

   lastSlash = findLastOf('/');
   if(lastSlash == npos)
      lastSlash = findLastOf('\\');
   if(lastSlash != npos)
      truncate(lastSlash);

   return *this;
}

//
// Similar to M_ExtractFileBase, but for qstrings.
// This one is not limited to 8 character file names, and will include any
// file extension, however, so it is not strictly equivalent.
//
void qstring::extractFileBase(qstring &dest)
{
   const char *src = buffer + index - 1;
   dest = "";

   // back up until a \ or the start
   while(src != buffer && 
      *(src - 1) != ':' &&
      *(src - 1) != '\\' &&
      *(src - 1) != '/')
   {
      --src;
   }

   dest = src;
}

//=============================================================================
// 
// Formatting
//

// 
// Adds quotation marks to the qstring.
//
qstring &qstring::makeQuoted()
{
   // if the string is empty, make it "", else add quotes around the contents
   if(index == 0)
      return concat("\"\"");
   else
   {
      insert("\"", 0);
      return push('\"');
   }
}

//
// Performs formatted printing into a qstring. If maxlen is > 0, the qstring
// will be reallocated to a minimum of that size for the formatted printing.
// Otherwise, the qstring will be allocated to a worst-case size for the given
// format string, and in this case, the format string MAY NOT contain any
// padding directives, as they will be ignored, and the resulting output may
// then be truncated to qstr->size - 1.
//
size_t qstring::printf(size_t maxlen, const char *fmt, ...)
{
   va_list va2;
   size_t returnval;
   size_t fmtsize = strlen(fmt) + 1;

   if(maxlen)
   {
      // If format string is longer than max specified, use format string len
      if(fmtsize > maxlen)
         maxlen = fmtsize;

      // maxlen is specified. Check it against the qstring's current size.
      if(maxlen > size)
         createSize(maxlen);
      else
         clear();
   }
   else
   {
      // determine a worst-case size by parsing the fmt string
      va_list va1;                // args
      char c;                     // current character
      const char *s = fmt;        // pointer into format string
      bool pctstate = false;      // seen a percentage?
      const char *dummystr;
      size_t charcount = fmtsize; // start at strlen of format string

      va_start(va1, fmt);
      while((c = *s++))
      {
         if(pctstate)
         {
            switch(c)
            {
            case 'x': // Integer formats
            case 'X':
               charcount += 2; // for 0x
               // fall-through
            case 'd':
            case 'i':
            case 'o':
            case 'u':
               // highest 32-bit octal is 11, plus 1 for possible sign
               (void)(va_arg(va1, int));
               charcount += 12; 
               pctstate = false;
               break;
            case 'p': // Pointer
               (void)(va_arg(va1, void *));
               charcount += 8 * sizeof(void *) / 4 + 2;
               pctstate = false;
               break;
            case 'e': // Float formats
            case 'E':
            case 'f':
            case 'g':
            case 'G':
               // extremely excessive, but it's possible 
               (void)(va_arg(va1, double));
               charcount += 1078; 
               pctstate = false;
               break;
            case 'c': // Character
               c = va_arg(va1, int);
               pctstate = false;
               break;
            case 's': // String
               dummystr = va_arg(va1, const char *);
               charcount += strlen(dummystr);
               pctstate = false;
               break;
            case '%': // Just a percent sign
               pctstate = false;
               break;
            default:
               // subtract width specifiers, signs, etc.
               charcount -= 1;
               break;
            }
         }
         else if(c == '%')
         {
            pctstate = true;
            charcount -= 1;
         }
      }
      va_end(va1);

      if(charcount > size)
         createSize(charcount);
      else
         clear();
   }

   va_start(va2, fmt);
   returnval = pvsnprintf(buffer, size, fmt, va2);
   va_end(va2);

   index = strlen(buffer);

   return returnval;
}

// EOF

