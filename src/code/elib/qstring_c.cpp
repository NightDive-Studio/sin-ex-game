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

#include "qstring.h"
#include "qstring_c.h"

const size_t qstring_npos     = qstring::npos;
const size_t qstring_basesize = qstring::basesize;

qstring_t qstring_new(const char *cstr)
{
   return new qstring(cstr);
}

qstring_t qstring_newsize(size_t startSize)
{
   return new qstring(startSize);
}

qstring_t qstring_newcopy(cqstring_t other)
{
   return new qstring(*other);
}

qstring_t qstring_move(qstring_t other)
{
   return new qstring(std::move(*other));
}

void qstring_free(qstring_t qstr)
{
   delete qstr;
}

#define QS  qstring_t
#define CQS cqstring_t
#define CH  char
#define CC  const char
#define UC  unsigned char
#define SZ  size_t
#define I   int
#define UI  unsigned int
#define L   long
#define DB  double
#define V   void
#define RET return

CH *qstring_getbuffer     (QS  this_               ) { RET this_->getBuffer();         }
CC *qstring_constptr      (CQS this_               ) { RET this_->constPtr();          }
SZ  qstring_length        (CQS this_               ) { RET this_->length();            }
I   qstring_empty         (CQS this_               ) { RET this_->empty();             }
SZ  qstring_getsize       (CQS this_               ) { RET this_->getSize();           }
CH  qstring_charat        (CQS this_, SZ idx       ) { RET this_->charAt(idx);         }
CH *qstring_bufferat      (QS  this_, SZ idx       ) { RET this_->bufferAt(idx);       }
UC  qstring_ucharat       (CQS this_, SZ idx       ) { RET this_->ucharAt(idx);        }
QS  qstring_clear         (QS  this_               ) { RET &this_->clear();            }
QS  qstring_push          (QS  this_, CH c         ) { RET &this_->push(c);            }
QS  qstring_pop           (QS  this_               ) { RET &this_->pop();              }
QS  qstring_concat        (QS  this_, CC *str      ) { RET &this_->concat(str);        }
QS  qstring_qconcat       (QS  this_, CQS src      ) { RET &this_->concat(*src);       }
QS  qstring_insert        (QS  this_, CC *s,  SZ i ) { RET &this_->insert(s, i);       }
I   qstring_strcmp        (CQS this_, CC *s        ) { RET this_->strCmp(s);           }
I   qstring_strncmp       (CQS this_, CC *s,  SZ i ) { RET this_->strNCmp(s, i);       }
I   qstring_strcasecmp    (CQS this_, CC *s        ) { RET this_->strCaseCmp(s);       }
I   qstring_strncasecmp   (CQS this_, CC *s,  SZ i ) { RET this_->strNCaseCmp(s, i);   }
I   qstring_compare       (CQS this_, CC *str      ) { RET this_->compare(str);        }
I   qstring_qcompare      (CQS this_, CQS other    ) { RET this_->compare(*other);     }
I   qstring_less          (CQS this_, CQS other    ) { RET this_->operator < (*other); }
I   qstring_greater       (CQS this_, CQS other    ) { RET this_->operator > (*other); }
UI  qstring_hashcode      (CQS this_               ) { RET this_->hashCode();          }
UI  qstring_hashcodecase  (CQS this_               ) { RET this_->hashCodeCase();      }
QS  qstring_copy          (QS  this_, CQS other    ) { RET &this_->copy(*other);       }
QS  qstring_copycstr      (QS  this_, CC *other    ) { RET &this_->copy(other);        }
QS  qstring_copycstrlen   (QS  this_, CC *o,  SZ i ) { RET &this_->copy(o, i);         }
QS  qstring_copyinto      (CQS this_, QS other     ) { RET &this_->copyInto(*other);   }
CH *qstring_copyintocstr  (CQS this_, CH *d,  SZ i ) { RET this_->copyInto(d, i);      }
V   qstring_swapwith      (QS  this_, QS other     ) { this_->swapWith(*other);        }
QS  qstring_toupper       (QS  this_               ) { RET &this_->toUpper();          }
QS  qstring_tolower       (QS  this_               ) { RET &this_->toLower();          }
SZ  qstring_replace       (QS  this_, CC *f, CH rp ) { RET this_->replace(f, rp);      }
SZ  qstring_replacenotof  (QS  this_, CC *f, CH rp ) { RET this_->replaceNotOf(f, rp); }
CH *qstring_duplicate     (CQS this_               ) { RET this_->duplicate();         }
I   qstring_toint         (CQS this_               ) { RET this_->toInt();             }
L   qstring_tolong        (CQS this_, CH **e, I r  ) { RET this_->toLong(e, r);        }
DB  qstring_todouble      (CQS this_, CH **e       ) { RET this_->toDouble(e);         }
CC *qstring_strchr        (CQS this_, CH c         ) { RET this_->strChr(c);           }
CC *qstring_strrchr       (CQS this_, CH c         ) { RET this_->strRChr(c);          }
SZ  qstring_findfirstof   (CQS this_, CH c         ) { RET this_->findFirstOf(c);      }
SZ  qstring_findfirstnotof(CQS this_, CH c         ) { RET this_->findFirstNotOf(c);   }
SZ  qstring_findlastof    (CQS this_, CH c         ) { RET this_->findLastOf(c);       }
CC *qstring_findsubstr    (CQS this_, CC *sub      ) { RET this_->findSubStr(sub);     }
SZ  qstring_find          (CQS this_, CC *s, SZ p  ) { RET this_->find(s, p);          }
QS  qstring_lstrip        (QS  this_, CH c         ) { RET &this_->lstrip(c);          }
QS  qstring_rstrip        (QS  this_, CH c         ) { RET &this_->rstrip(c);          }
QS  qstring_truncate      (QS  this_, SZ pos       ) { RET &this_->truncate(pos);      }
QS  qstring_erase         (QS  this_, SZ p, SZ n   ) { RET &this_->erase(p, n);        }
QS  qstring_makequoted    (QS  this_               ) { RET &this_->makeQuoted();       }

// Static Hash Functions
UI  qstring_hashstatic    (CC *s) { RET qstring::HashCodeStatic(s);     }
UI  qstring_hashcasestatic(CC *s) { RET qstring::HashCodeCaseStatic(s); }

// File Path Utilities
QS  qstring_normalizeslashes   (QS this_       ) { RET &this_->normalizeSlashes();     }
QS  qstring_pathconcatenate    (QS this_, CQS o) { RET &this_->pathConcatenate(*o);    }
QS  qstring_pathconcatenatecstr(QS this_, CC *a) { RET &this_->pathConcatenate(a);     }
QS  qstring_adddefaultextension(QS this_, CC *x) { RET &this_->addDefaultExtension(x); }
QS  qstring_removefilespec     (QS this_       ) { RET &this_->removeFileSpec();       }
V   qstring_extractfilebase    (QS this_, QS d ) { this_->extractFileBase(*d);         }

// Operators
I   qstring_equal       (CQS this_, CQS other) { RET this_->operator == (*other);  }
I   qstring_equalcstr   (CQS this_, CC *other) { RET this_->operator == (other);   }
I   qstring_notequal    (CQS this_, CQS other) { RET this_->operator != (*other);  }
I   qstring_notequalcstr(CQS this_, CC *other) { RET this_->operator != (other);   }
QS  qstring_assign      (QS  this_, CQS other) { RET &this_->operator = (*other);  }
QS  qstring_assigncstr  (QS  this_, CC *other) { RET &this_->operator = (other);   }
QS  qstring_append      (QS  this_, CQS other) { RET &this_->operator += (*other); }
QS  qstring_appendcstr  (QS  this_, CC *other) { RET &this_->operator += (other);  }
QS  qstring_appendchar  (QS  this_, CH c     ) { RET &this_->operator += (c);      }
QS  qstring_appendint   (QS  this_, I  i     ) { RET &this_->operator << (i);      }
QS  qstring_appenddouble(QS  this_, DB d     ) { RET &this_->operator << (d);      }

// EOF

