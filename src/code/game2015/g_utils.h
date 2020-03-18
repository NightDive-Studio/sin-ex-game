//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/g_utils.h                        $
// $Revision:: 17                                                             $
//   $Author:: Markd                                                          $
//     $Date:: 5/19/99 4:57p                                                  $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// 

#ifndef __G_UTILS_H__
#define __G_UTILS_H__

class Archiver;

EXPORT_FROM_DLL void       G_ArchiveEdict(Archiver &arc, edict_t *edict);
EXPORT_FROM_DLL void       G_UnarchiveEdict(Archiver &arc, edict_t *edict);

#include "entity.h"

EXPORT_FROM_DLL void       G_InitEdict(edict_t *e);
EXPORT_FROM_DLL edict_t   *G_Spawn(void);
EXPORT_FROM_DLL void       G_FreeEdict(edict_t *e);

EXPORT_FROM_DLL void       G_TouchTriggers(Entity *ent);
EXPORT_FROM_DLL void       G_TouchSolids(Entity *ent);

EXPORT_FROM_DLL char      *G_CopyString(const char *in);

EXPORT_FROM_DLL int        G_FindClass(int entnum, const char *classname);
EXPORT_FROM_DLL Entity    *G_NextEntity(Entity *ent);

EXPORT_FROM_DLL void       G_CalcBoundsOfMove(Vector &start, Vector &end, Vector &mins, Vector &maxs, Vector *minbounds, Vector *maxbounds);

EXPORT_FROM_DLL void       G_ShowTrace(trace_t *trace, edict_t *passent, const char *reason);
EXPORT_FROM_DLL trace_t    G_Trace(Vector &start, Vector &mins, Vector &maxs, Vector &end, Entity *passent, int contentmask, const char *reason);
EXPORT_FROM_DLL trace_t    G_Trace(vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, edict_t *passent, int contentmask, const char *reason);
EXPORT_FROM_DLL trace_t    G_FullTrace(Vector &start, Vector &mins, Vector &maxs, Vector &end, float radius, Entity *passent, int contentmask, const char *reason);
EXPORT_FROM_DLL trace_t    G_FullTrace(vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, float radius, edict_t *passent, int contentmask, const char *reason);

//###
//EXPORT_FROM_DLL void     SelectSpawnPoint( Vector &origin, Vector &angles, int *gravaxis = NULL );
EXPORT_FROM_DLL void       SelectSpawnPoint(Vector &origin, Vector &angles, edict_t *edict, int *gravaxis = NULL, int *startonbike = NULL);

EXPORT_FROM_DLL int        G_FindTarget(int entnum, const char *name);
EXPORT_FROM_DLL Entity    *G_NextEntity(Entity *ent);

EXPORT_FROM_DLL qboolean   M_CheckBottom(Entity *ent);
EXPORT_FROM_DLL qboolean   M_CeilingCheckBottom(Entity *ent); //###

EXPORT_FROM_DLL Vector     G_GetMovedir(void);
EXPORT_FROM_DLL qboolean   KillBox(Entity *ent);
EXPORT_FROM_DLL qboolean   IsNumeric(const char *str);

EXPORT_FROM_DLL Entity     *findradius(Entity *startent, Vector org, float rad);
EXPORT_FROM_DLL Entity     *findclientsinradius(Entity *startent, Vector org, float rad);
EXPORT_FROM_DLL const char *G_GetNameForSurface(csurface_t *s);

EXPORT_FROM_DLL Vector     G_CalculateImpulse(Vector start, Vector end, float speed, float gravity);
EXPORT_FROM_DLL Vector     G_PredictPosition(Vector start, Vector target, Vector targetvelocity, float speed);

EXPORT_FROM_DLL void G_InitDebugLines(void);
EXPORT_FROM_DLL void G_DebugLine(Vector start, Vector end, float r, float g, float b, float alpha);
EXPORT_FROM_DLL void G_Color3f(float r, float g, float b);
EXPORT_FROM_DLL void G_Color3v(Vector color);
EXPORT_FROM_DLL void G_Color4f(float r, float g, float b, float alpha);
EXPORT_FROM_DLL void G_Color3vf(Vector color, float alpha);
EXPORT_FROM_DLL void G_BeginLine(void);
EXPORT_FROM_DLL void G_Vertex(Vector v);
EXPORT_FROM_DLL void G_EndLine(void);
EXPORT_FROM_DLL void G_DebugBBox(Vector origin, Vector mins, Vector maxs, float r, float g, float b, float alpha);
EXPORT_FROM_DLL void G_DrawDebugNumber(Vector org, float number, float scale, float r, float g, float b, int precision = 0);

class ScriptThread;

EXPORT_FROM_DLL void G_LoadAndExecScript(const char *filename, const char *label = NULL);
EXPORT_FROM_DLL ScriptThread *ExecuteThread(str thread_name, qboolean start = true);

EXPORT_FROM_DLL int  G_Milliseconds(void);
EXPORT_FROM_DLL void G_DebugPrintf(const char *fmt, ...);

//==================================================================
//
// Inline functions
//
//==================================================================

inline EXPORT_FROM_DLL float angmod(float v)
{
   int b;

   b = (int)v;

   b = b - (b % 360);
   if(b < 0)
   {
      b -= 360;
   }

   return v - (float)b;
}

//
// Takes an index to an entity and returns pointer to it.
//
inline EXPORT_FROM_DLL Entity *G_GetEntity(int entnum)
{
   if((entnum < 0) || (entnum >= globals.max_edicts))
   {
      gi.error("G_GetEntity: %d out of valid range.", entnum);
   }

   return (Entity *)g_edicts[entnum].entity;
}

//
// Returns a number from 0<= num < 1
//
inline EXPORT_FROM_DLL float G_Random(void)
{
   return ((float)(rand() & 0x7fff)) / ((float)0x8000);
}

//
// Returns a number from 0 <= num < n
//
inline EXPORT_FROM_DLL float G_Random(float n)
{
   return G_Random() * n;
}

//
// Returns a number from -1 <= num < 1
//
inline EXPORT_FROM_DLL float G_CRandom(void)
{
   return G_Random(2) - 1;
}

//
// Returns a number from -n <= num < n
//
inline EXPORT_FROM_DLL float G_CRandom(float n)
{
   return G_CRandom() * n;
}

//
// Converts all backslashes in a string to forward slashes.
// Used to make filenames consistant.
//
inline EXPORT_FROM_DLL str G_FixSlashes(const char *filename)
{
   int i;
   int len;
   str text;

   if(filename)
   {
      // Convert all forward slashes to back slashes
      text = filename;
      len = text.length();
      for(i = 0; i < len; i++)
      {
         if(text[i] == '\\')
         {
            text[i] = '/';
         }
      }
   }

   return text;
}

#endif /* g_utils.h */

// EOF

