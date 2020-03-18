//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/str.cpp                          $
// $Revision:: 4                                                              $
//   $Author:: Jimdose                                                        $
//     $Date:: 5/13/98 4:56p                                                  $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
// 
// DESCRIPTION:
// Simple, DLL portable string class
// 

#include "str.h"

/*
=================
TestStringClass

This is a fairly rigorous test of the str class's functionality.
Because of the fairly global and subtle ramifications of a bug occuring
in this class, it should be run after any changes to the class.
Add more tests as functionality is changed.  Tests should include
any possible bounds violation and NULL data tests.
=================
*/
void TestStringClass()
{
   char	ch;							// ch == ?
   str	*t;							// t == ?
   str	a;								// a.len == 0, a.data == "\0"
   str	b;								// b.len == 0, b.data == "\0"
   str	c("test");				// c.len == 4, c.data == "test\0"
   str	d(c);						// d.len == 0, d.data == "test\0"
   str	e(nullptr);					// e.len == 0, e.data == "\0"					ASSERT!
   int	i;								// i == ?

   i = a.length();					// i == 0
   i = c.length();					// i == 4

   const char *s1 = a.c_str();	// s1 == "\0"
   const char *s2 = c.c_str();	// s2 == "test\0"

   t = new str();						// t->len == 0, t->data == "\0"
   delete t;							// t == ?

   b = "test";							// b.len == 4, b.data == "test\0"
   t = new str("test");			// t->len == 4, t->data == "test\0"
   delete t;							// t == ?

   a = c;								// a.len == 4, a.data == "test\0"
   a = nullptr;							// a.len == 0, a.data == "\0"					ASSERT!
   a = c + d;							// a.len == 8, a.data == "testtest\0"
   a = c + "wow";						// a.len == 7, a.data == "testwow\0"
   a = c + nullptr;						// a.len == 4, a.data == "test\0"			ASSERT!
   a = "this" + d;					// a.len == 8, a.data == "thistest\0"
   a = nullptr + d;						// a.len == 4, a.data == "test\0"			ASSERT!
   a += c;								// a.len == 8, a.data == "testtest\0"
   a += "wow";							// a.len == 11, a.data == "testtestwow\0"
   a += nullptr;							// a.len == 11, a.data == "testtestwow\0"	ASSERT!

   a = "test";							// a.len == 4, a.data == "test\0"
   ch = a[0];						// ch == 't'
   ch = a[-1];						// ch == 0											ASSERT!
   ch = a[1000];					// ch == 0											ASSERT!
   ch = a[0];						// ch == 't'
   ch = a[1];						// ch == 'e'
   ch = a[2];						// ch == 's'
   ch = a[3];						// ch == 't'
   ch = a[4];						// ch == '\0'										ASSERT!
   ch = a[5];						// ch == '\0'										ASSERT!

   a[1] = 'b';						// a.len == 4, a.data == "tbst\0"
   a[-1] = 'b';						// a.len == 4, a.data == "tbst\0"			ASSERT!
   a[0] = '0';						// a.len == 4, a.data == "0bst\0"
   a[1] = '1';						// a.len == 4, a.data == "01st\0"
   a[2] = '2';						// a.len == 4, a.data == "012t\0"
   a[3] = '3';						// a.len == 4, a.data == "0123\0"
   a[4] = '4';						// a.len == 4, a.data == "0123\0"			ASSERT!
   a[5] = '5';						// a.len == 4, a.data == "0123\0"			ASSERT!
   a[7] = '7';						// a.len == 4, a.data == "0123\0"			ASSERT!

   a = "test";							// a.len == 4, a.data == "test\0"
   b = "no";							// b.len == 2, b.data == "no\0"

   i = (a == b);					// i == 0
   i = (a == c);					// i == 1

   i = (a == "blow");				// i == 0
   i = (a == "test");				// i == 1
   i = (a == nullptr);				// i == 0											ASSERT!

   i = ("test" == b);				// i == 0
   i = ("test" == a);				// i == 1
   i = (nullptr == a);				// i == 0											ASSERT!

   i = (a != b);					// i == 1
   i = (a != c);					// i == 0

   i = (a != "blow");				// i == 1
   i = (a != "test");				// i == 0
   i = (a != nullptr);				// i == 1											ASSERT!

   i = ("test" != b);				// i == 1
   i = ("test" != a);				// i == 0
   i = (nullptr != a);				// i == 1											ASSERT!
}

// EOF

