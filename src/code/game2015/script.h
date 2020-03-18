//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/script.h                         $
// $Revision:: 12                                                             $
//   $Author:: Jimdose                                                        $
//     $Date:: 10/07/98 11:59p                                                $
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

#ifndef __SCRIPT_H__
#define __SCRIPT_H__

#include "class.h"
#include "vector.h"
#include "str.h"

#define	MAXTOKEN	256

typedef struct
{
   qboolean    tokenready;
   int         offset;
   int         line;
   char        token[MAXTOKEN];
} scriptmarker_t;

class EXPORT_FROM_DLL Script : public Class
{
protected:
   qboolean    tokenready = false;

   str         filename;
   const char *script_p = nullptr;
   const char *end_p    = nullptr;

   int         line     = 0;
   char        token[MAXTOKEN];

   qboolean    releaseBuffer = false;

   qboolean    AtComment(void);
   void        CheckOverflow(void);

public:
   const char *buffer = nullptr;
   int         length;

   CLASS_PROTOTYPE(Script);

   Script();
   ~Script();
   
   void         Close();
   const char  *Filename() const;
   int          GetLineNumber();
   void         Reset();
   void         MarkPosition(scriptmarker_t *mark);
   void         RestorePosition(scriptmarker_t *mark);
   qboolean     SkipToEOL();
   void         SkipWhiteSpace(qboolean crossline);
   void         SkipNonToken(qboolean crossline);
   qboolean     TokenAvailable(qboolean crossline);
   qboolean     CommentAvailable(qboolean crossline);
   void         UnGetToken();
   qboolean     AtString(qboolean crossline);
   const char  *GetToken(qboolean crossline);
   const char  *GetLine(qboolean crossline);
   const char  *GetRaw();
   const char  *GetString(qboolean crossline);
   qboolean     GetSpecific(const char *string);
   int          GetInteger(qboolean crossline);
   double       GetDouble(qboolean crossline);
   float        GetFloat(qboolean crossline);
   Vector       GetVector(qboolean crossline);
   int          LinesInFile();
   void         Parse(const char *data, int length, const char *name);
   void         LoadFile(const char *name);
   const char  *Token();
   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void Script::Archive(Archiver &arc)
{
   Class::Archive(arc);

   arc.WriteString(filename);
   arc.WriteBoolean(tokenready);
   //
   // save out current pointer as an offset
   //
   arc.WriteInteger(script_p - buffer);
   arc.WriteInteger(line);
   arc.WriteRaw(token, sizeof(token));
}

inline EXPORT_FROM_DLL void Script::Unarchive(Archiver &arc)
{
   int i;

   Class::Unarchive(arc);

   arc.ReadString(&filename);
   //
   // load the file in
   //
   LoadFile(filename.c_str());

   arc.ReadBoolean(&tokenready);
   arc.ReadInteger(&i);
   //
   // restore the script pointer
   //
   script_p = buffer + i;
   arc.ReadInteger(&line);
   arc.ReadRaw(token, sizeof(token));
}

#endif

// EOF

