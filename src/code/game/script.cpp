//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/script.cpp                       $
// $Revision:: 13                                                             $
//   $Author:: Jimdose                                                        $
//     $Date:: 10/07/98 11:58p                                                $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// C++ implementaion of tokenizing text interpretation.  Class accepts filename
// to load or pointer to preloaded text data.  Standard tokenizing operations
// such as skip white-space, get string, get integer, get float, get token,
// and skip line are implemented.  
//
// Note: all '//', '#', and ';' are treated as comments.  Probably should
// make this behaviour toggleable.
// 

#include "g_local.h"
#include "script.h"

#define TOKENCOMMENT      (';')
#define TOKENCOMMENT2     ('#')
#define TOKENEOL          ('\n')
#define TOKENNULL         ('\0')
#define TOKENSPACE        (' ')

CLASS_DECLARATION( Class, Script, NULL );

ResponseDef Script::Responses[] =
{
   { NULL, NULL }
};

Script::~Script()
{
   Close();
}

Script::Script() : Class()
{
   memset(token, 0, sizeof(token));
}

void Script::Close(void)
{
   if(releaseBuffer && buffer)
   {
      gi.TagFree((void *)buffer);
   }

   buffer        = NULL;
   script_p      = NULL;
   end_p         = NULL;
   line          = 0;
   releaseBuffer = false;
   tokenready    = false;
   memset(token, 0, sizeof(token));
}

/*
==============
=
= Filename
=
==============
*/
const char *Script::Filename(void) const
{
   return filename.c_str();
}

/*
==============
=
= GetLineNumber
=
==============
*/
int Script::GetLineNumber(void)
{
   return line;
}

/*
==============
=
= Reset
=
==============
*/
void Script::Reset(void)
{
   script_p = buffer;
   line = 1;
   tokenready = false;
}

/*
==============
=
= MarkPosition
=
==============
*/
void Script::MarkPosition(scriptmarker_t *mark)
{
   assert(mark);

   mark->tokenready = tokenready;
   mark->offset = script_p - buffer;
   mark->line = line;
   strcpy(mark->token, token);
}

/*
==============
=
= RestorePosition
=
==============
*/
void Script::RestorePosition(scriptmarker_t *mark)
{
   assert(mark);

   tokenready = mark->tokenready;
   script_p = buffer + mark->offset;
   line = mark->line;
   strcpy(token, mark->token);

   assert(script_p <= end_p);
   if(script_p > end_p)
   {
      script_p = end_p;
   }
}

/*
==============
=
= SkipToEOL
=
==============
*/
qboolean Script::SkipToEOL(void)
{
   if(script_p >= end_p)
   {
      return true;
   }

   while(*script_p != TOKENEOL)
   {
      if(script_p >= end_p)
      {
         return true;
      }
      script_p++;
   }
   return false;
}

/*
==============
=
= CheckOverflow
=
==============
*/
void Script::CheckOverflow(void)
{
   if(script_p >= end_p)
   {
      gi.error("End of token file reached prematurely reading %s\n", filename.c_str());
   }
}

/*
==============
=
= SkipWhiteSpace
=
==============
*/
void Script::SkipWhiteSpace(qboolean crossline)
{
   //
   // skip space
   //
   CheckOverflow();

   while(*script_p <= TOKENSPACE)
   {
      if(*script_p++ == TOKENEOL)
      {
         if(!crossline)
         {
            gi.error("Line %i is incomplete in file %s\n", line, filename.c_str());
         }

         line++;
      }
      CheckOverflow();
   }
}

qboolean Script::AtComment(void)
{
   if(script_p >= end_p)
   {
      return false;
   }

   if(*script_p == TOKENCOMMENT)
   {
      return true;
   }

   if(*script_p == TOKENCOMMENT2)
   {
      return true;
   }

   // Two or more character comment specifiers
   if((script_p + 1) >= end_p)
   {
      return false;
   }

   if((*script_p == '/') && (*(script_p + 1) == '/'))
   {
      return true;
   }

   return false;
}

/*
==============
=
= SkipNonToken
=
==============
*/
void Script::SkipNonToken(qboolean crossline)
{
   //
   // skip space and comments
   //
   SkipWhiteSpace(crossline);
   while(AtComment())
   {
      SkipToEOL();
      SkipWhiteSpace(crossline);
   }
}

/*
=============================================================================
=
= Token section
=
=============================================================================
*/

/*
==============
=
= TokenAvailable
=
==============
*/
qboolean Script::TokenAvailable(qboolean crossline)
{
   if(script_p >= end_p)
   {
      return false;
   }

   while(1)
   {
      while(*script_p <= TOKENSPACE)
      {
         if(*script_p == TOKENEOL)
         {
            if(crossline == false)
            {
               return(false);
            }
            line++;
         }

         script_p++;
         if(script_p >= end_p)
         {
            return false;
         }
      }

      if(AtComment())
      {
         qboolean done;

         done = SkipToEOL();
         if(done)
         {
            return false;
         }
      }
      else
      {
         break;
      }
   }

   return true;
}

/*
==============
=
= CommentAvailable
=
==============
*/
qboolean Script::CommentAvailable(qboolean crossline)
{
   const char *searchptr;

   searchptr = script_p;

   if(searchptr >= end_p)
   {
      return false;
   }

   while(*searchptr <= TOKENSPACE)
   {
      if((*searchptr == TOKENEOL) && (!crossline))
      {
         return false;
      }
      searchptr++;
      if(searchptr >= end_p)
      {
         return false;
      }
   }

   return true;
}

/*
==============
=
= UnGet
=
= Signals that the current token was not used, and should be reported
= for the next GetToken.  Note that

GetToken (true);
UnGetToken ();
GetToken (false);

= could cross a line boundary.
=
==============
*/
void Script::UnGetToken(void)
{
   tokenready = true;
}

/*
==============
=
= Get
=
==============
*/
qboolean Script::AtString(qboolean crossline)
{
   //
   // skip space
   //
   SkipNonToken(crossline);

   return (*script_p == '"');
}

/*
==============
=
= Get
=
==============
*/
const char *Script::GetToken(qboolean crossline)
{
   char *token_p;

   // is a token already waiting?
   if(tokenready)
   {
      tokenready = false;
      return token;
   }

   //
   // skip space
   //
   SkipNonToken(crossline);

   //
   // copy token
   //

   if(*script_p == '"')
   {
      return GetString(crossline);
   }

   token_p = token;
   while(*script_p > TOKENSPACE && !AtComment())
   {
      if((*script_p == '\\') && (script_p < end_p - 1))
      {
         script_p++;
         switch(*script_p)
         {
         case 'n':	*token_p++ = '\n'; break;
         case 'r':	*token_p++ = '\n'; break;
         case '\'': *token_p++ = '\''; break;
         case '\"': *token_p++ = '\"'; break;
         case '\\': *token_p++ = '\\'; break;
         default:		*token_p++ = *script_p; break;
         }
         script_p++;
      }
      else
      {
         *token_p++ = *script_p++;
      }

      if(token_p == &token[MAXTOKEN])
      {
         gi.error("Token too large on line %i in file %s\n", line, filename.c_str());
      }

      if(script_p == end_p)
      {
         break;
      }
   }

   *token_p = 0;

   return token;
}

/*
==============
=
= GetLine
=
==============
*/
const char *Script::GetLine(qboolean crossline)
{
   const char	*start;
   int			size;

   // is a token already waiting?
   if(tokenready)
   {
      tokenready = false;
      return token;
   }

   //
   // skip space
   //
   SkipNonToken(crossline);

   //
   // copy token
   //
   start = script_p;
   SkipToEOL();
   size = script_p - start;
   if(size < MAXTOKEN - 1)
   {
      memcpy(token, start, size);
      token[size] = '\0';
   }
   else
   {
      gi.error("Token too large on line %i in file %s\n", line, filename.c_str());
   }

   return token;
}

/*
==============
=
= GetRaw
=
==============
*/
const char *Script::GetRaw(void)
{
   const char	*start;
   int			size;

   //
   // skip white space
   //
   SkipWhiteSpace(true);

   //
   // copy token
   //
   start = script_p;
   SkipToEOL();
   size = script_p - start;
   if(size < MAXTOKEN - 1)
   {
      memset(token, 0, sizeof(token));
      memcpy(token, start, size);
   }
   else
   {
      gi.error("Token too large on line %i in file %s\n", line, filename.c_str());
   }

   return token;
}

/*
==============
=
= GetString
=
==============
*/
const char *Script::GetString(qboolean crossline)
{
   int startline;
   char *token_p;

   // is a token already waiting?
   if(tokenready)
   {
      tokenready = false;
      return token;
   }

   //
   // skip space
   //
   SkipNonToken(crossline);

   if(*script_p != '"')
   {
      gi.error("Expecting string on line %i in file %s\n", line, filename.c_str());
   }

   script_p++;

   startline = line;
   token_p = token;
   while(*script_p != '"')
   {
      if(*script_p == TOKENEOL)
      {
         gi.error("Line %i is incomplete while reading string in file %s\n", line, filename.c_str());
      }

      if((*script_p == '\\') && (script_p < end_p - 1))
      {
         script_p++;
         switch(*script_p)
         {
         case 'n':	*token_p++ = '\n'; break;
         case 'r':	*token_p++ = '\n'; break;
         case '\'': *token_p++ = '\''; break;
         case '\"': *token_p++ = '\"'; break;
         case '\\': *token_p++ = '\\'; break;
         default:		*token_p++ = *script_p; break;
         }
         script_p++;
      }
      else
      {
         *token_p++ = *script_p++;
      }

      if(script_p >= end_p)
      {
         gi.error("End of token file reached prematurely while reading string on\n"
                  "line %d in file %s\n", startline, filename.c_str());
      }

      if(token_p == &token[MAXTOKEN])
      {
         gi.error("String too large on line %i in file %s\n", line, filename.c_str());
      }
   }

   *token_p = 0;

   // skip last quote
   script_p++;

   return token;
}

/*
==============
=
= GetSpecific
=
==============
*/
qboolean Script::GetSpecific(const char *string)
{
   do
   {
      if(!TokenAvailable(true))
      {
         return false;
      }
      GetToken(true);
   }
   while(strcmp(token, string));

   return true;
}

/*
==============
=
= GetInteger
=
==============
*/
int Script::GetInteger(qboolean crossline)
{
   GetToken(crossline);
   return atoi(token);
}

/*
==============
=
= GetDouble
=
==============
*/
double Script::GetDouble(qboolean crossline)
{
   GetToken(crossline);
   return atof(token);
}

/*
==============
=
= GetFloat
=
==============
*/
float Script::GetFloat(qboolean crossline)
{
   return (float)GetDouble(crossline);
}

/*
==============
=
= GetVector
=
==============
*/
Vector Script::GetVector(qboolean crossline)
{
   return Vector(GetFloat(crossline), GetFloat(crossline), GetFloat(crossline));
}

/*
===================
=
= LinesInFile
=
===================
*/
int Script::LinesInFile(void)
{
   qboolean		temp_tokenready;
   const char	*temp_script_p;
   int			temp_line;
   char			temp_token[MAXTOKEN];
   int			numentries;

   temp_tokenready = tokenready;
   temp_script_p = script_p;
   temp_line = line;
   strcpy(temp_token, token);

   numentries = 0;

   Reset();
   while(TokenAvailable(true))
   {
      GetLine(true);
      numentries++;
   }

   tokenready = temp_tokenready;
   script_p = temp_script_p;
   line = temp_line;
   strcpy(token, temp_token);

   return numentries;
}

/*
==============
=
= Parse
=
==============
*/
void Script::Parse(const char *data, int length, const char *name)
{
   Close();

   buffer = data;
   Reset();
   end_p = script_p + length;
   this->length = length;
   filename = name;
}

/*
==============
=
= Load
=
==============
*/
void Script::LoadFile(const char *name)
{
   int			length;
   const char	*buffer;

   Close();

   length = gi.LoadFile(name, (void **)&buffer, TAG_GAME);
   if(length < 0)
   {
      error("LoadFile", "Couldn't load %s\n", name);
   }
   Parse(buffer, length, name);
   releaseBuffer = true;
}

const char *Script::Token(void)
{
   return token;
}

// EOF

