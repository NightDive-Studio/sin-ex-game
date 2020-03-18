/*
  ELib
  
  Quick secure string library - C Adapter functions
  
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

#ifndef ELIB_QSTRING_C_H__
#define ELIB_QSTRING_C_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
// To the C++ code, these mean the same thing
typedef class qstring *qstring_t;
typedef const class qstring *cqstring_t;
#else
// Present an opaque typedef to consuming C code
typedef struct qstring *qstring_t;
typedef const struct qstring *cqstring_t;
#endif

extern const size_t qstring_npos;
extern const size_t qstring_basesize;

qstring_t      qstring_new                (const char *cstr);
qstring_t      qstring_newsize            (size_t startSize);
qstring_t      qstring_newcopy            (cqstring_t other);
qstring_t      qstring_move               (qstring_t other);
void           qstring_free               (qstring_t qstr);
char          *qstring_getbuffer          (qstring_t  this_);
const char    *qstring_constptr           (cqstring_t this_);
size_t         qstring_length             (cqstring_t this_);
int            qstring_empty              (cqstring_t this_);
size_t         qstring_getsize            (cqstring_t this_);
char           qstring_charat             (cqstring_t this_, size_t idx);
char          *qstring_bufferat           (qstring_t  this_, size_t idx);
unsigned char  qstring_ucharat            (cqstring_t this_, size_t idx);
qstring_t      qstring_clear              (qstring_t  this_);
qstring_t      qstring_push               (qstring_t  this_, char c);
qstring_t      qstring_pop                (qstring_t  this_);
qstring_t      qstring_concat             (qstring_t  this_, const char *str);
qstring_t      qstring_qconcat            (qstring_t  this_, cqstring_t src);
qstring_t      qstring_insert             (qstring_t  this_, const char *str, size_t maxlen);
int            qstring_strcmp             (cqstring_t this_, const char *str);
int            qstring_strncmp            (cqstring_t this_, const char *str, size_t maxlen);
int            qstring_strcasecmp         (cqstring_t this_, const char *str);
int            qstring_strncasecmp        (cqstring_t this_, const char *str, size_t maxlen );
int            qstring_compare            (cqstring_t this_, const char *str);
int            qstring_qcompare           (cqstring_t this_, cqstring_t other);
int            qstring_less               (cqstring_t this_, cqstring_t other);
int            qstring_greater            (cqstring_t this_, cqstring_t other);
unsigned int   qstring_hashstatic         (const char *str);
unsigned int   qstring_hashcasestatic     (const char *str);
unsigned int   qstring_hashcode           (cqstring_t this_);
unsigned int   qstring_hashcodecase       (cqstring_t this_);
qstring_t      qstring_copy               (qstring_t  this_, cqstring_t other);
qstring_t      qstring_copycstr           (qstring_t  this_, const char *other);
qstring_t      qstring_copycstrlen        (qstring_t  this_, const char *other,  size_t maxlen);
qstring_t      qstring_copyinto           (cqstring_t this_, qstring_t other);
char          *qstring_copyintocstr       (cqstring_t this_, char *dest,  size_t len);
void           qstring_swapwith           (qstring_t  this_, qstring_t other);
qstring_t      qstring_toupper            (qstring_t  this_);
qstring_t      qstring_tolower            (qstring_t  this_);
size_t         qstring_replace            (qstring_t  this_, const char *filter, char repl);
size_t         qstring_replacenotof       (qstring_t  this_, const char *filter, char repl);
char          *qstring_duplicate          (cqstring_t this_);
int            qstring_toint              (cqstring_t this_);
long           qstring_tolong             (cqstring_t this_, char **endptr, int radix);
double         qstring_todouble           (cqstring_t this_, char **endptr);
const char    *qstring_strchr             (cqstring_t this_, char c);
const char    *qstring_strrchr            (cqstring_t this_, char c);
size_t         qstring_findfirstof        (cqstring_t this_, char c);
size_t         qstring_findfirstnotof     (cqstring_t this_, char c);
size_t         qstring_findlastof         (cqstring_t this_, char c);
const char    *qstring_findsubstr         (cqstring_t this_, const char *substr);
size_t         qstring_find               (cqstring_t this_, const char *str, size_t pos);
qstring_t      qstring_lstrip             (qstring_t  this_, char c);
qstring_t      qstring_rstrip             (qstring_t  this_, char c);
qstring_t      qstring_truncate           (qstring_t  this_, size_t pos);
qstring_t      qstring_erase              (qstring_t  this_, size_t pos, size_t n);
qstring_t      qstring_makequoted         (qstring_t  this_);
qstring_t      qstring_normalizeslashes   (qstring_t  this_);
qstring_t      qstring_pathconcatenate    (qstring_t  this_, cqstring_t other);
qstring_t      qstring_pathconcatenatecstr(qstring_t  this_, const char *other);
qstring_t      qstring_adddefaultextension(qstring_t  this_, const char *ext);
qstring_t      qstring_removefilespec     (qstring_t  this_);
void           qstring_extractfilebase    (qstring_t  this_, qstring_t d );
int            qstring_equal              (cqstring_t this_, cqstring_t other);
int            qstring_equalcstr          (cqstring_t this_, const char *other);
int            qstring_notequal           (cqstring_t this_, cqstring_t other);
int            qstring_notequalcstr       (cqstring_t this_, const char *other);
qstring_t      qstring_assign             (qstring_t  this_, cqstring_t other);
qstring_t      qstring_assigncstr         (qstring_t  this_, const char *other);
qstring_t      qstring_append             (qstring_t  this_, cqstring_t other);
qstring_t      qstring_appendcstr         (qstring_t  this_, const char *other);
qstring_t      qstring_appendchar         (qstring_t  this_, char c);
qstring_t      qstring_appendint          (qstring_t  this_, int  i);
qstring_t      qstring_appenddouble       (qstring_t  this_, double d);

#ifdef __cplusplus
}
#endif

#endif

// EOF

