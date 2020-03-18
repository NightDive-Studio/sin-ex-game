/*
  ELib
  
  Inline comparison templates
  
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

#ifndef M_COMPARE_H__
#define M_COMPARE_H__

//
// emin
//
// Return a reference to the lesser of two objects.
//
template<typename T> inline const T &emin(const T &a, const T &b)
{
   return ((b < a) ? b : a);
}

//
// emax
//
// Return a reference to the greater of two objects.
//
template<typename T> inline const T &emax(const T &a, const T &b)
{
   return ((a < b) ? b : a);
}

//
// eclamp
//
// Clamp a value to a given range, inclusive.
//
template<typename T> inline const T &eclamp(const T &a, const T &min, const T &max)
{
   return (a < min ? min : (max < a ? max : a));
}

#endif
