//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/g_utils.cpp                      $
// $Revision:: 71                                                             $
//   $Author:: Markd                                                          $
//     $Date:: 5/20/99 6:41p                                                  $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// 

#include "g_local.h"
#include "g_utils.h"
#include "ctype.h"
#include "worldspawn.h"
#include "scriptmaster.h"
#include "windows.h"
#include "ctf.h"

cvar_t *g_numdebuglines;

debugline_t *DebugLines = NULL;
Vector		currentVertex( 0, 0, 0 );
Vector		vertColor( 1, 1, 1 );
float			vertAlpha = 1;
float			vertexIndex = 0;

/*
============
G_TouchTriggers

============
*/
void G_TouchTriggers(Entity *ent)
{
   int		i;
   int		num;
   edict_t	*touch[MAX_EDICTS];
   edict_t	*hit;
   Event		*ev;

   // dead things don't activate triggers!
   if((ent->client || (ent->edict->svflags & SVF_MONSTER)) && (ent->health <= 0))
   {
      return;
   }

   num = gi.BoxEdicts(ent->absmin.vec3(), ent->absmax.vec3(), touch, MAX_EDICTS, AREA_TRIGGERS);

   // be careful, it is possible to have an entity in this
   // list removed before we get to it (killtriggered)
   for(i = 0; i < num; i++)
   {
      hit = touch[i];
      if(!hit->inuse || (hit->entity == ent))
      {
         continue;
      }

      assert(hit->entity);

      // FIXME
      // should we post the events on the list with zero time
      ev = new Event(EV_Touch);
      ev->AddEntity(ent);
      hit->entity->ProcessEvent(ev);
   }
}

/*
============
G_TouchSolids

Call after linking a new trigger in during gameplay
to force all entities it covers to immediately touch it
============
*/
void G_TouchSolids(Entity *ent)
{
   int		i;
   int		num;
   edict_t	*touch[MAX_EDICTS];
   edict_t	*hit;
   Event		*ev;

   num = gi.BoxEdicts(ent->absmin.vec3(), ent->absmax.vec3(), touch, MAX_EDICTS, AREA_SOLID);

   // be careful, it is possible to have an entity in this
   // list removed before we get to it (killtriggered)
   for(i = 0; i < num; i++)
   {
      hit = touch[i];
      if(!hit->inuse)
      {
         continue;
      }

      assert(hit->entity);

      //FIXME
      // should we post the events so that we don't have to worry about any entities going away
      ev = new Event(EV_Touch);
      ev->AddEntity(ent);
      hit->entity->ProcessEvent(ev);
   }
}

EXPORT_FROM_DLL void G_ShowTrace(trace_t *trace, edict_t *passent, const char *reason)
{
   str text;
   str pass;
   str hit;

   assert(reason);
   assert(trace);

   if(passent)
   {
      pass = va("'%s'(%d)", passent->entname, passent->s.number);
   }
   else
   {
      pass = "NULL";
   }

   if(trace->ent)
   {
      hit = va("'%s'(%d)", trace->ent->entname, trace->ent->s.number);
   }
   else
   {
      hit = "NULL";
   }

   text = va("%0.1f : Pass %s Frac %f Hit %s : '%s'\n",
             level.time, pass.c_str(), trace->fraction, hit.c_str(), reason ? reason : "");

   if(sv_traceinfo->value == 3)
   {
      G_DebugPrintf(text.c_str());
   }
   else
   {
      gi.dprintf("%s", text.c_str());
   }
}

EXPORT_FROM_DLL void G_CalcBoundsOfMove(Vector &start, Vector &end, Vector &mins, Vector &maxs, Vector *minbounds, Vector *maxbounds)
{
   Vector bmin;
   Vector bmax;

   ClearBounds(bmin.vec3(), bmax.vec3());
   AddPointToBounds(start.vec3(), bmin.vec3(), bmax.vec3());
   AddPointToBounds(end.vec3(), bmin.vec3(), bmax.vec3());
   bmin += mins;
   bmax += maxs;

   if(minbounds)
   {
      *minbounds = bmin;
   }

   if(maxbounds)
   {
      *maxbounds = bmax;
   }
}

EXPORT_FROM_DLL trace_t G_Trace(vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, edict_t *passent, int contentmask, const char *reason)
{
   trace_t trace;

   trace = gi.trace(start, mins, maxs, end, passent, contentmask);
   assert(!trace.ent || trace.ent->entity);

   if(sv_traceinfo->value > 1)
   {
      G_ShowTrace(&trace, passent, reason);
   }
   sv_numtraces++;

   if(sv_drawtrace->value)
   {
      G_DebugLine(Vector(start), Vector(end), 1, 1, 0, 1);
   }

   return trace;
}

EXPORT_FROM_DLL trace_t G_Trace(Vector &start, Vector &mins, Vector &maxs, Vector &end, Entity *passent, int contentmask, const char *reason)
{
   edict_t *ent;
   trace_t trace;

   assert(reason);

   if(passent == NULL)
   {
      ent = NULL;
   }
   else
   {
      ent = passent->edict;
   }

   trace = gi.trace(start.vec3(), mins.vec3(), maxs.vec3(), end.vec3(), ent, contentmask);

   assert(!trace.ent || trace.ent->entity);

   if(sv_traceinfo->value > 1)
   {
      G_ShowTrace(&trace, ent, reason);
   }
   sv_numtraces++;

   if(sv_drawtrace->value)
   {
      G_DebugLine(start, end, 1, 1, 0, 1);
   }

   return trace;
}

EXPORT_FROM_DLL trace_t G_FullTrace(Vector &start, Vector &mins, Vector &maxs, Vector &end, float  radius, Entity *passent, int contentmask, const char *reason)
{
   edict_t *ent;
   trace_t trace;

   if(passent == NULL)
   {
      ent = NULL;
   }
   else
   {
      ent = passent->edict;
   }

   trace = gi.fulltrace(start.vec3(), mins.vec3(), maxs.vec3(), end.vec3(), radius, ent, contentmask);
   assert(!trace.ent || trace.ent->entity);

   if(sv_traceinfo->value > 1)
   {
      G_ShowTrace(&trace, ent, reason);
   }
   sv_numtraces++;

   if(sv_drawtrace->value)
   {
      G_DebugLine(start, end, 0, 1, 1, 1);
   }

   return trace;
}

EXPORT_FROM_DLL trace_t G_FullTrace(vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, float  radius, edict_t *passent, int contentmask, const char *reason)
{
   trace_t trace;

   trace = gi.fulltrace(start, mins, maxs, end, radius, passent, contentmask);
   assert(!trace.ent || trace.ent->entity);

   if(sv_traceinfo->value > 1)
   {
      G_ShowTrace(&trace, passent, reason);
   }
   sv_numtraces++;

   if(sv_drawtrace->value)
   {
      G_DebugLine(Vector(start), Vector(end), 0, 1, 1, 1);
   }

   return trace;
}

/*
=======================================================================

  SelectSpawnPoint

=======================================================================
*/

/*
================
PlayersRangeFromSpot

Returns the distance to the nearest player from the given spot
================
*/
float PlayersRangeFromSpot(Entity *spot)
{
   Entity	*player;
   float		bestplayerdistance;
   Vector	v;
   int		n;
   float		playerdistance;

   bestplayerdistance = 9999999;
   for(n = 1; n <= maxclients->value; n++)
   {
      if(!g_edicts[n].inuse || !g_edicts[n].entity)
      {
         continue;
      }

      player = g_edicts[n].entity;
      if(player->health <= 0)
      {
         continue;
      }

      v = spot->worldorigin - player->worldorigin;
      playerdistance = v.length();

      if(playerdistance < bestplayerdistance)
      {
         bestplayerdistance = playerdistance;
      }
   }

   return bestplayerdistance;
}

/*
================
SelectRandomDeathmatchSpawnPoint

go to a random point, but NOT the two points closest
to other players
================
*/
Entity *SelectRandomDeathmatchSpawnPoint(void)
{
   Entity	*spot, *spot1, *spot2;
   int		count = 0;
   int		selection;
   int		num;
   float		range, range1, range2;

   spot = NULL;
   range1 = range2 = 99999;
   spot1 = spot2 = NULL;

   num = 0;
   while((num = G_FindClass(num, "info_player_deathmatch")) != 0)
   {
      spot = G_GetEntity(num);
      count++;
      range = PlayersRangeFromSpot(spot);
      if(range < range1)
      {
         range1 = range;
         spot1 = spot;
      }
      else if(range < range2)
      {
         range2 = range;
         spot2 = spot;
      }
   }

   if(!count)
   {
      return NULL;
   }

   if(count <= 2)
   {
      spot1 = spot2 = NULL;
   }
   else
   {
      count -= 2;
   }

   selection = rand() % count;

   spot = NULL;
   num = 0;
   do
   {
      num = G_FindClass(num, "info_player_deathmatch");
      spot = G_GetEntity(num);
      if(spot == spot1 || spot == spot2)
      {
         selection++;
      }
   }
   while(selection--);

   return spot;
}

/*
================
SelectFarthestDeathmatchSpawnPoint

================
*/
Entity *SelectFarthestDeathmatchSpawnPoint(void)
{
   Entity	*bestspot;
   float		bestdistance;
   float		bestplayerdistance;
   Entity	*spot;
   int		num;

   spot = NULL;
   bestspot = NULL;
   bestdistance = 0;
   num = 0;
   while((num = G_FindClass(num, "info_player_deathmatch")) != NULL)
   {
      spot = G_GetEntity(num);

      bestplayerdistance = PlayersRangeFromSpot(spot);
      if(bestplayerdistance > bestdistance)
      {
         bestspot = spot;
         bestdistance = bestplayerdistance;
      }
   }

   if(bestspot)
   {
      return bestspot;
   }

   // if there is a player just spawned on each and every start spot
   // we have no choice to turn one into a telefrag meltdown
   num = G_FindClass(0, "info_player_deathmatch");
   spot = G_GetEntity(num);

   return spot;
}

Entity *SelectDeathmatchSpawnPoint(void)
{
   if(DM_FLAG(DF_SPAWN_FARTHEST))
   {
      return SelectFarthestDeathmatchSpawnPoint();
   }
   else
   {
      return SelectRandomDeathmatchSpawnPoint();
   }
}

Entity *SelectCoopSpawnPoint(void)
{
   const char *tname;
   Entity *spot = NULL;
   int num;

   num = 0;
   while((num = G_FindClass(num, "info_player_coop")) != 0)
   {
      spot = G_GetEntity(num);
      tname = spot->TargetName();
      if(!game.spawnpoint.length() || !tname || !tname[0])
      {
         break;
      }

      if(Q_stricmp(game.spawnpoint.c_str(), spot->TargetName()) == 0)
      {
         break;
      }
   }

   return spot;
}

/*
===========
SelectSpawnPoint

Chooses a player start, deathmatch start, coop start, etc
============
*/
void SelectSpawnPoint(Vector &origin, Vector &angles, edict_t *edict, int *gravaxis)
{
   const char * tname;
   Entity *spot = NULL;
   Entity *spot2 = NULL;
   int num;

   if((level.training == 2) && game.spawnpoint.length())
   {
      num = 0;
      while((num = G_FindClass(num, "info_player_progressivestart")) != 0)
      {
         spot2 = G_GetEntity(num);
         tname = spot2->TargetName();
         if(!tname || !tname[0])
         {
            break;
         }

         if(Q_stricmp(game.spawnpoint.c_str(), spot2->TargetName()) == 0)
         {
            spot = spot2;
            break;
         }
      }
   }
   else if(ctf->value)
   {
      spot = SelectCTFSpawnPoint(edict);
   }
   else if(deathmatch->value || level.training == 1)
   {
      spot = SelectDeathmatchSpawnPoint();
   }
   else if(coop->value)
   {
      spot = SelectCoopSpawnPoint();
   }

   // find a single player start spot
   if(!spot)
   {
      num = 0;
      while((num = G_FindClass(num, "info_player_start")) != 0)
      {
         spot = G_GetEntity(num);
         tname = spot->TargetName();
         if(!game.spawnpoint.length() || !tname || !tname[0])
         {
            break;
         }

         if(Q_stricmp(game.spawnpoint.c_str(), spot->TargetName()) == 0)
         {
            break;
         }
      }

      if(!spot)
      {
         if(!game.spawnpoint.length())
         {
            // there wasn't a spawnpoint without a target, so use any
            num = G_FindClass(0, "info_player_start");
            spot = G_GetEntity(num);
         }

         if(!spot || !spot->entnum)
         {
            gi.error("Couldn't find spawn point %s\n", game.spawnpoint.c_str());
         }
      }
   }

   origin = spot->worldorigin + Vector(0, 0, 9);
   angles = spot->angles;
   if(gravaxis)
   {
      *gravaxis = spot->gravaxis;
   }
}

/*
=============
M_CheckBottom

Returns false if any part of the bottom of the entity is off an edge that
is not a staircase.

=============
*/
int c_yes, c_no;

qboolean M_CheckBottom(Entity *ent)
{
   Vector	mins, maxs, start, stop;
   trace_t	trace;
   int		x, y;
   float		mid, bottom;

   mins = ent->worldorigin + ent->mins * 0.5;
   maxs = ent->worldorigin + ent->maxs * 0.5;

   // if all of the points under the corners are solid world, don't bother
   // with the tougher checks
   // the corners must be within 16 of the midpoint
   start[2] = mins[2] - 1;

   for(x = 0; x <= 1; x++)
   {
      for(y = 0; y <= 1; y++)
      {
         start[0] = x ? maxs[0] : mins[0];
         start[1] = y ? maxs[1] : mins[1];
         if(gi.pointcontents(start.vec3()) != CONTENTS_SOLID)
         {
            goto realcheck;
         }
      }
   }

   c_yes++;
   return true;		// we got out easy

realcheck:

   c_no++;

   //
   // check it for real...
   //
   start[2] = mins[2];

   // the midpoint must be within 16 of the bottom
   start[0] = stop[0] = (mins[0] + maxs[0]) * 0.5;
   start[1] = stop[1] = (mins[1] + maxs[1]) * 0.5;
   stop[2] = start[2] - 3 * STEPSIZE;//2 * STEPSIZE;

   trace = G_Trace(start, vec_zero, vec_zero, stop, ent, MASK_MONSTERSOLID, "M_CheckBottom 1");

   if(trace.fraction == 1.0)
   {
      return false;
   }

   mid = bottom = trace.endpos[2];

   // the corners must be within 16 of the midpoint	
   for(x = 0; x <= 1; x++)
   {
      for(y = 0; y <= 1; y++)
      {
         start[0] = stop[0] = x ? maxs[0] : mins[0];
         start[1] = stop[1] = y ? maxs[1] : mins[1];

         trace = G_Trace(start, vec_zero, vec_zero, stop, ent, MASK_MONSTERSOLID, "M_CheckBottom 2");

         if(trace.fraction != 1.0 && trace.endpos[2] > bottom)
         {
            bottom = trace.endpos[2];
         }

         if(trace.fraction == 1.0 || mid - trace.endpos[2] > STEPSIZE)
         {
            return false;
         }
      }
   }

   c_yes++;
   return true;
}

char *G_CopyString(const char *in)
{
   char	*newb;
   char	*new_p;
   int	i, l;

   assert(in);

   l = strlen(in) + 1;

   newb = (char *)gi.TagMalloc(l, TAG_LEVEL);
   new_p = newb;

   for(i = 0; i < l; i++)
   {
      if((in[i] == '\\') && (i < l - 1))
      {
         i++;
         if(in[i] == 'n')
         {
            *new_p++ = '\n';
         }
         else
         {
            *new_p++ = '\\';
         }
      }
      else
      {
         *new_p++ = in[i];
      }
   }

   return newb;
}

int G_FindClass(int entnum, const char *classname)
{
   edict_t *from;

   for(from = &g_edicts[entnum + 1]; from < &g_edicts[globals.num_edicts]; from++)
   {
      if(!from->inuse)
      {
         continue;
      }
      if(!Q_stricmp(from->entity->getClassID(), classname))
      {
         return from->s.number;
      }
   }

   return 0;
}

int G_FindTarget(int entnum, const char *name)
{
   edict_t  *from;
   Entity   *next;

   if(name && name[0])
   {
      from = &g_edicts[entnum];
      next = world->GetNextEntity(str(name), from->entity);
      if(next)
      {
         return next->entnum;
      }
   }

   return 0;
}

EXPORT_FROM_DLL Entity *G_NextEntity(Entity *ent)
{
   edict_t *from;

   if(!g_edicts)
   {
      return NULL;
   }

   if(!ent)
   {
      ent = world;
   }

   if(!ent)
   {
      return NULL;
   }

   for(from = ent->edict + 1; from < &g_edicts[globals.num_edicts]; from++)
   {
      if(!from->inuse || !from->entity)
      {
         continue;
      }

      return from->entity;
   }

   return NULL;
}

//
// QuakeEd only writes a single float for angles (bad idea), so up and down are
// just constant angles.
//
Vector G_GetMovedir(void)
{
   float	angle;

   angle = G_GetFloatArg("angle");
   if(angle == -1)
   {
      return Vector(0, 0, 1);
   }
   else if(angle == -2)
   {
      return Vector(0, 0, -1);
   }

   angle *= (M_PI * 2 / 360);
   return Vector(cos(angle), sin(angle), 0);
}

/*
=================
KillBox

Kills all entities that would touch the proposed new positioning
of ent.  Ent should be unlinked before calling this!
=================
*/
qboolean KillBox(Entity *ent)
{
   int		i;
   int		num;
   edict_t	*touch[MAX_EDICTS];
   edict_t	*hit;
   Vector   min;
   Vector   max;
   int      fail;

   fail = 0;

   min = ent->worldorigin + ent->mins;
   max = ent->worldorigin + ent->maxs;
   num = gi.BoxEdicts(min.vec3(), max.vec3(), touch, MAX_EDICTS, AREA_SOLID);
   for(i = 0; i < num; i++)
   {
      hit = touch[i];

      if(!hit->inuse || (hit->entity == ent) || !hit->entity || (hit->entity == world))
      {
         continue;
      }

      hit->entity->Damage(ent, ent, hit->entity->health + 100000, ent->worldorigin, vec_zero, vec_zero,
                          0, DAMAGE_NO_PROTECTION, MOD_TELEFRAG, -1, -1, 1.0f);

      //
      // if we didn't kill it, fail
      //
      if(hit->entity->getSolidType() != SOLID_NOT)
      {
         fail++;
      }
   }

   //
   // all clear
   //
   return !fail;
}

qboolean IsNumeric(const char *str)
{
   int len;
   int i;
   qboolean dot;

   if(*str == '-')
   {
      str++;
   }

   dot = false;
   len = strlen(str);
   for(i = 0; i < len; i++)
   {
      if(!isdigit(str[i]))
      {
         if((str[i] == '.') && !dot)
         {
            dot = true;
            continue;
         }
         return false;
      }
   }

   return true;
}

void G_InitEdict(edict_t *e)
{
   e->inuse = true;
   e->s.number = e - g_edicts;

   // make sure a default scale gets set
   e->s.scale = 1.0f;
   e->s.renderfx |= RF_FRAMELERP;
   e->spawntime = level.time;
   e->s.frame = 0;
   e->s.prevframe = -1;
}

/*
=================
findradius

Returns entities that have origins within a spherical area

findradius (origin, radius)
=================
*/
Entity *findradius(Entity *startent, Vector org, float rad)
{
   Vector	eorg;
   edict_t	*from;
   float		r2;

   if(!startent)
   {
      startent = world;
   }

   if(!startent)
   {
      return NULL;
   }

   // square the radius so that we don't have to do a square root
   r2 = rad * rad;

   assert(startent->edict);
   for(from = startent->edict + 1; from < &g_edicts[globals.num_edicts]; from++)
   {
      if(!from->inuse)
      {
         continue;
      }

      assert(from->entity);

      eorg = org - from->entity->centroid;

      // dot product returns length squared
      if((eorg * eorg) <= r2)
      {
         return from->entity;
      }
   }

   return NULL;
}

/*
=================
findclientinradius

Returns clients that have origins within a spherical area

findclientinradius (origin, radius)
=================
*/
Entity *findclientsinradius(Entity *startent, Vector org, float rad)
{
   Vector	eorg;
   edict_t	*ed;
   float		r2;
   int      i;

   // square the radius so that we don't have to do a square root
   r2 = rad * rad;

   for(i = startent->entnum; i < game.maxclients; i++)
   {
      ed = &g_edicts[1 + i];

      if(!ed->inuse || !ed->entity)
      {
         continue;
      }

      eorg = org - ed->entity->centroid;

      // dot product returns length squared
      if((eorg * eorg) <= r2)
      {
         return ed->entity;
      }
   }

   return NULL;
}

const char *G_GetNameForSurface(csurface_t *s)
{
   switch(s->flags & MASK_SURF_TYPE)
   {
   case SURF_TYPE_WOOD:
      return "wood";

   case SURF_TYPE_METAL:
      return "metal";

   case SURF_TYPE_STONE:
      return "stone";

   case SURF_TYPE_CONCRETE:
      return "concrete";

   case SURF_TYPE_DIRT:
      return "dirt";

   case SURF_TYPE_FLESH:
      return "flesh";

   case SURF_TYPE_GRILL:
      return "grill";

   case SURF_TYPE_GLASS:
      return "glass";

   case SURF_TYPE_FABRIC:
      return "fabric";

   case SURF_TYPE_MONITOR:
      return "monitor";

   case SURF_TYPE_GRAVEL:
      return "gravel";

   case SURF_TYPE_VEGETATION:
      return "vegetation";

   case SURF_TYPE_PAPER:
      return "paper";

   case SURF_TYPE_DUCT:
      return "duct";

   case SURF_TYPE_WATER:
      return "water";
   }

   return "";
}

void G_InitDebugLines(void)
{
   *gi.DebugLines = DebugLines;
   *gi.numDebugLines = 0;

   currentVertex = vec_zero;
   vertColor = Vector(1, 1, 1);
   vertAlpha = 1;
   vertexIndex = 0;
}

void G_AllocDebugLines(void)
{
   g_numdebuglines = gi.cvar("g_numdebuglines", "4096", CVAR_LATCH);

   DebugLines = (debugline_t *)gi.TagMalloc((int)g_numdebuglines->value * sizeof(debugline_t), TAG_GAME);

   G_InitDebugLines();
}

void G_DebugLine(Vector start, Vector end, float r, float g, float b, float alpha)
{
   debugline_t *line;

   if(!g_numdebuglines)
   {
      return;
   }

   if(*gi.numDebugLines >= g_numdebuglines->value)
   {
      gi.dprintf("G_DebugLine: Exceeded MAX_DEBUG_LINES\n");
      return;
   }

   line = &DebugLines[*gi.numDebugLines];
   (*gi.numDebugLines)++;

   VectorCopy(start.vec3(), line->start);
   VectorCopy(end.vec3(), line->end);
   VectorSet(line->color, r, g, b);
   line->alpha = alpha;
}

void G_Color3f(float r, float g, float b)
{
   vertColor = Vector(r, g, b);
}

void G_Color3v(Vector color)
{
   vertColor = color;
}

void G_Color4f(float r, float g, float b, float alpha)
{
   vertColor = Vector(r, g, b);
   vertAlpha = alpha;
}

void G_Color3vf(Vector color, float alpha)
{
   vertColor = color;
   vertAlpha = alpha;
}

void G_BeginLine(void)
{
   currentVertex = vec_zero;
   vertexIndex = 0;
}

void G_Vertex(Vector v)
{
   vertexIndex++;
   if(vertexIndex > 1)
   {
      G_DebugLine(currentVertex, v, vertColor[0], vertColor[1], vertColor[2], vertAlpha);
   }
   currentVertex = v;
}

void G_EndLine(void)
{
   currentVertex = vec_zero;
   vertexIndex = 0;
}

void G_DebugBBox(Vector origin, Vector mins, Vector maxs, float r, float g, float b, float alpha)
{
   int i;
   Vector points[8];

   /*
   ** compute a full bounding box
   */
   for(i = 0; i < 8; i++)
   {
      Vector   tmp;

      if(i & 1)
         tmp[0] = origin[0] + mins[0];
      else
         tmp[0] = origin[0] + maxs[0];

      if(i & 2)
         tmp[1] = origin[1] + mins[1];
      else
         tmp[1] = origin[1] + maxs[1];

      if(i & 4)
         tmp[2] = origin[2] + mins[2];
      else
         tmp[2] = origin[2] + maxs[2];

      points[i] = tmp;
   }

   G_Color4f(r, g, b, alpha);

   G_BeginLine();
   G_Vertex(points[0]);
   G_Vertex(points[1]);
   G_Vertex(points[3]);
   G_Vertex(points[2]);
   G_Vertex(points[0]);
   G_EndLine();

   G_BeginLine();
   G_Vertex(points[4]);
   G_Vertex(points[5]);
   G_Vertex(points[7]);
   G_Vertex(points[6]);
   G_Vertex(points[4]);
   G_EndLine();

   for(i = 0; i < 4; i++)
   {
      G_BeginLine();
      G_Vertex(points[i]);
      G_Vertex(points[4 + i]);
      G_EndLine();
   }
}

//
// LED style digits
//
// ****1***
// *      *		8 == /
// 6     *4
// *    * *
// ****2***
// *  *   *
// 7 *--8 5     9
// **     *    **10
// ****3***  12**
//             11

static int Numbers[12][8] =
{
   { 1, 3, 4, 5, 6, 7, 0, 0 }, // 0
   { 4, 5, 0, 0, 0, 0, 0, 0 }, // 1
   { 1, 4, 2, 7, 3, 0, 0, 0 }, // 2
   { 1, 4, 2, 5, 3, 0, 0, 0 }, // 3
   { 6, 4, 2, 5, 0, 0, 0, 0 }, // 4
   { 1, 6, 2, 5, 3, 0, 0, 0 }, // 5
   { 1, 6, 2, 5, 7, 3, 0, 0 }, // 6
   { 1, 8, 0, 0, 0, 0, 0, 0 }, // 7
   { 1, 2, 3, 4, 5, 6, 7, 0 }, // 8
   { 1, 6, 4, 2, 5, 3, 0, 0 }, // 9
   { 9, 10, 11, 12, 0, 0, 0, 0 }, // .
   { 2, 0, 0, 0, 0, 0, 0, 0 }, // -
};

static float Lines[13][4] =
{
   { 0, 0, 0, 0 },		// Unused
   { -4, 8, 4, 8 },		// 1
   { -4, 4, 4, 4 },		// 2
   { -4, 0, 4, 0 },		// 3
   { 4, 8, 4, 4 },		// 4
   { 4, 4, 4, 0 },		// 5
   { -4, 8, -4, 4 },		// 6
   { -4, 4, -4, 0 },		// 7
   { 4, 8, -4, 0 },		// 8

   { -1, 2, 1, 2 },		// 9
   { 1, 2, 1, 0 },		// 10
   { -1, 0, 1, 0 },		// 11
   { -1, 0, -1, 2 },		// 12
};

void G_DrawDebugNumber(Vector org, float number, float scale, float r, float g, float b, int precision)
{
   int    i, j, l, num;
   Vector up, right, pos, start, ang, delta;
   str    text;
   char   format[20];

   // only draw entity numbers within a certain radius
   delta = Vector(g_edicts[1].s.origin) - org;
   if((delta * delta) > (1000 * 1000))
   {
      return;
   }

   G_Color4f(r, g, b, 1.0);

   ang = game.clients[0].ps.viewangles;
   ang.AngleVectors(NULL, &right, &up);

   up *= scale;
   right *= scale;

   if(precision > 0)
   {
      snprintf(format, sizeof(format), "%%.%df", precision);
      text = va(format, number);
   }
   else
   {
      text = va("%d", (int)number);
   }

   start = org - (text.length() - 1) * 5 * right;

   for(i = 0; i < text.length(); i++)
   {
      if(text[i] == '.')
      {
         num = 10;
      }
      else if(text[i] == '-')
      {
         num = 11;
      }
      else
      {
         num = text[i] - '0';
      }

      for(j = 0; j < 8; j++)
      {
         l = Numbers[num][j];
         if(l == 0)
         {
            break;
         }

         G_BeginLine();

         pos = start + Lines[l][0] * right + Lines[l][1] * up;
         G_Vertex(pos);

         pos = start + Lines[l][2] * right + Lines[l][3] * up;
         G_Vertex(pos);

         G_EndLine();
      }

      start += 10 * right;
   }
}

#if 0
//
// LED style digits
//
// ****1***
// *      *		8 == /
// 6     *4
// *    * *
// ****2***
// *  *   *
// 7 *--8 5
// **     *
// ****3***
//

static int Numbers[10][8] =
{
   { 1, 3, 4, 5, 6, 7, 0, 0 }, // 0
   { 4, 5, 0, 0, 0, 0, 0, 0 }, // 1
   { 1, 4, 2, 7, 3, 0, 0, 0 }, // 2
   { 1, 4, 2, 5, 3, 0, 0, 0 }, // 3
   { 6, 4, 2, 5, 0, 0, 0, 0 }, // 4
   { 1, 6, 2, 5, 3, 0, 0, 0 }, // 5
   { 1, 6, 2, 5, 7, 3, 0, 0 }, // 6
   { 1, 8, 0, 0, 0, 0, 0, 0 }, // 7
   { 1, 2, 3, 4, 5, 6, 7, 0 }, // 8
   { 1, 6, 4, 2, 5, 3, 0, 0 }, // 9
};

static float Lines[9][4] =
{
   { 0, 0, 0, 0 },		// Unused
   { -4, 8, 4, 8 },		// 1
   { -4, 4, 4, 4 },		// 2
   { -4, 0, 4, 0 },		// 3
   { 4, 8, 4, 4 },		// 4
   { 4, 4, 4, 0 },		// 5
   { -4, 8, -4, 4 },		// 6
   { -4, 4, -4, 0 },		// 7
   { 4, 8, -4, 0 },		// 8
};

void G_DrawDebugNumber(Vector origin, float number, float scale, float r, float g, float b, int precision)
{
   int i;
   int j;
   int l;
   int num;
   Vector up;
   Vector right;
   Vector pos;
   Vector start;
   Vector ang;
   str text;
   Vector delta;
   char format[20];

   // only draw entity numbers within a certain radius
   delta = Vector(g_edicts[1].s.origin) - origin;
   if((delta * delta) > (1000 * 1000))
   {
      return;
   }

   G_Color4f(r, g, b, 1.0);

   ang = game.clients[0].ps.viewangles;
   ang.AngleVectors(NULL, &right, &up);

   up *= scale;
   right *= scale;

   if(precision > 0)
   {
      snprintf(format, sizeof(format), "%%.%df", precision);
      text = va(format, number);
   }
   else
   {
      text = va("%d", (int)number);
   }

   start = origin - (text.length() - 1) * 5 * right;

   for(i = 0; i < text.length(); i++)
   {
      num = text[i] - '0';
      for(j = 0; j < 8; j++)
      {
         l = Numbers[num][j];
         if(l == 0)
         {
            break;
         }
         if((l < 0) || (l > 8))
         {
            break;
         }

         G_BeginLine();

         pos = start + Lines[l][0] * right + Lines[l][1] * up;
         G_Vertex(pos);

         pos = start + Lines[l][2] * right + Lines[l][3] * up;
         G_Vertex(pos);

         G_EndLine();
      }

      start += 10 * right;
   }
}
#endif

Vector G_CalculateImpulse(Vector start, Vector end, float speed, float gravity)
{
   float traveltime, vertical_speed;
   Vector dir, xydir, velocity;

   dir = end - start;
   xydir = dir;
   xydir.z = 0;
   traveltime = xydir.length() / speed;
   vertical_speed = (dir.z / traveltime) + (0.5f * gravity * sv_gravity->value * traveltime);
   xydir.normalize();

   velocity = speed * xydir;
   velocity.z = vertical_speed;
   return velocity;
}

Vector G_PredictPosition(Vector start, Vector target, Vector targetvelocity, float  speed)
{
   Vector projected;
   float traveltime;
   Vector dir, xydir;

   dir = target - start;
   xydir = dir;
   xydir.z = 0;
   traveltime = xydir.length() / speed;
   projected = target + (targetvelocity * traveltime);

   return projected;
}

const char *ExpandLocation(const char *location)
{
   if(!strcmpi(location, "all"))
      return NULL;

   if(!strnicmp(location, "torso", 5))
   {
      if(location[6] == 'u')
      {
         return ("upper chest");
      }
      else if(location[6] == 'l')
      {
         return ("lower chest");
      }
      else
      {
         return ("chest");
      }
   }
   else if(!strnicmp(location, "leg", 3))
   {
      if(location[9] == 'u')
      {
         return ("upper leg");
      }
      else if(location[9] == 'l')
      {
         return ("lower leg");
      }
      else
      {
         return ("leg");
      }
   }
   else if(!strnicmp(location, "arm", 3))
   {
      return ("arm");
   }
   else if(!strnicmp(location, "head", 4))
   {
      return ("head");
   }

   return NULL;
}

char *ClientTeam(edict_t *ent)
{
   static char	value[512];

   value[0] = 0;

   if(!ent->client)
      return value;

   if(DM_FLAG(DF_MODELTEAMS))
      COM_StripExtension(Info_ValueForKey(ent->client->pers.userinfo, "model"), value);
   else if(DM_FLAG(DF_SKINTEAMS))
      COM_StripExtension(Info_ValueForKey(ent->client->pers.userinfo, "skin"), value);

   return(value);
}

qboolean OnSameTeam(Entity *ent1, Entity *ent2)
{
   char	ent1Team[512];
   char	ent2Team[512];

   // CTF check for same team
   if(ctf->value)
   {
      if(!ent1->client || !ent2->client)
      {
         return false;
      }

      if(ent1->client->resp.ctf_team == ent2->client->resp.ctf_team)
      {
         return true;
      }
      else
      {
         return false;
      }
   }

   if(!DM_FLAG(DF_MODELTEAMS | DF_SKINTEAMS))
      return false;

   strcpy(ent1Team, ClientTeam(ent1->edict));
   strcpy(ent2Team, ClientTeam(ent2->edict));

   if(!strcmp(ent1Team, ent2Team))
      return true;

   return false;
}

/*
==============
G_LoadAndExecScript

Like the man says...
==============
*/
void G_LoadAndExecScript(const char *filename, const char *label)
{
   ScriptThread *pThread;

   if(gi.LoadFile(filename, NULL, 0) != -1)
   {
      pThread = Director.CreateThread(filename, LEVEL_SCRIPT, label);
      if(pThread)
      {
         // start right away
         pThread->Start(-1);
      }
      else
      {
         gi.dprintf("G_LoadAndExecScript : %s could not create thread.", filename);
      }
   }
}

ScriptThread *ExecuteThread(str thread_name, qboolean start)
{
   GameScript * s;

   if(thread_name.length())
   {
      ScriptThread * pThread;

      s = ScriptLib.GetScript(ScriptLib.GetGameScript());
      if(!s)
      {
         gi.dprintf("StartThread::Null game script\n");
         return false;
      }
      pThread = Director.CreateThread(s, thread_name.c_str());
      if(pThread)
      {
         if(start)
         {
            // start right away
            pThread->Start(-1);
         }
      }
      else
      {
         gi.dprintf("StartThread::unable to go to %s\n", thread_name.c_str());
         return NULL;
      }
      return pThread;
   }
   return NULL;
}

/*
==============
G_ArchiveEdict
==============
*/
void G_ArchiveEdict(Archiver &arc, edict_t *edict)
{
   assert(edict);

   if(edict->client)
   {
      arc.WriteRaw(edict->client, sizeof(*edict->client));
   }

   arc.WriteVector(Vector(edict->s.origin));
   arc.WriteVector(Vector(edict->s.angles));

   arc.WriteQuat(Quat(edict->s.quat));
   arc.WriteQuat(Quat(edict->s.mat));

   arc.WriteVector(Vector(edict->s.old_origin));
   arc.WriteInteger(edict->s.modelindex);
   arc.WriteInteger(edict->s.frame);
   arc.WriteInteger(edict->s.prevframe);

   arc.WriteVector(Vector(edict->s.vieworigin));
   arc.WriteVector(Vector(edict->s.viewangles));

   arc.WriteInteger(edict->s.anim);
   arc.WriteFloat(edict->s.scale);
   arc.WriteFloat(edict->s.alpha);
   arc.WriteFloat(edict->s.color_r);
   arc.WriteFloat(edict->s.color_g);
   arc.WriteFloat(edict->s.color_b);
   arc.WriteInteger(edict->s.radius);
   arc.WriteRaw(&edict->s.bone, sizeof(edict->s.bone));
   arc.WriteInteger(edict->s.parent);
   arc.WriteInteger(edict->s.numgroups);
   arc.WriteRaw(&edict->s.groups, sizeof(edict->s.groups));
   arc.WriteInteger(edict->s.gunanim);
   arc.WriteInteger(edict->s.gunframe);
   // index into configstrings
   arc.WriteInteger(edict->s.gunmodelindex);
   arc.WriteInteger(edict->s.lightofs);
   arc.WriteInteger(edict->s.skinnum);
   arc.WriteInteger(edict->s.effects);
   arc.WriteInteger(edict->s.renderfx);
   arc.WriteInteger(edict->s.solid);
   // index into configstrings
   arc.WriteInteger(edict->s.sound);
   //   arc.WriteInteger( edict->s.event );

   arc.WriteInteger(edict->svflags);
   arc.WriteVector(Vector(edict->mins));
   arc.WriteVector(Vector(edict->maxs));
   arc.WriteVector(Vector(edict->absmin));
   arc.WriteVector(Vector(edict->absmax));
   arc.WriteVector(Vector(edict->size));
   arc.WriteVector(Vector(edict->fullmins));
   arc.WriteVector(Vector(edict->fullmaxs));
   arc.WriteFloat(edict->fullradius);
   arc.WriteVector(Vector(edict->centroid));
   arc.WriteInteger((int)edict->solid);
   arc.WriteInteger(edict->clipmask);
   if(edict->owner)
   {
      // s.number may be cleared out, so write the actual number
      arc.WriteInteger(edict->owner - g_edicts);
   }
   else
   {
      arc.WriteInteger(-1);
   }

   arc.WriteFloat(edict->freetime);
   arc.WriteFloat(edict->spawntime);
   arc.WriteString(str(edict->entname));
}

/*
==============
G_UnarchiveEdict
==============
*/
void G_UnarchiveEdict(Archiver &arc, edict_t *edict)
{
   Vector tempvec;
   str tempstr;
   Quat q;
   int temp;

   assert(edict);

   // 
   // edict will already be setup as far as entnum is concerned
   //
   if(edict->client)
   {
      arc.ReadRaw(edict->client, sizeof(*edict->client));
   }

   tempvec = arc.ReadVector();
   tempvec.copyTo(edict->s.origin);
   tempvec = arc.ReadVector();
   tempvec.copyTo(edict->s.angles);

   q = arc.ReadQuat();
   edict->s.quat[0] = q.x;
   edict->s.quat[1] = q.y;
   edict->s.quat[2] = q.z;
   edict->s.quat[3] = q.w;

   q = arc.ReadQuat();
   QuatToMat(q.vec4(), edict->s.mat);

   tempvec = arc.ReadVector();
   tempvec.copyTo(edict->s.old_origin);
   arc.ReadInteger(&edict->s.modelindex);
   arc.ReadInteger(&edict->s.frame);
   arc.ReadInteger(&edict->s.prevframe);

   tempvec = arc.ReadVector();
   tempvec.copyTo(edict->s.vieworigin);
   tempvec = arc.ReadVector();
   tempvec.copyTo(edict->s.viewangles);

   arc.ReadInteger(&edict->s.anim);
   arc.ReadFloat(&edict->s.scale);
   arc.ReadFloat(&edict->s.alpha);
   arc.ReadFloat(&edict->s.color_r);
   arc.ReadFloat(&edict->s.color_g);
   arc.ReadFloat(&edict->s.color_b);
   arc.ReadInteger(&edict->s.radius);
   arc.ReadRaw(&edict->s.bone, sizeof(edict->s.bone));
   arc.ReadInteger(&edict->s.parent);
   arc.ReadInteger(&edict->s.numgroups);
   arc.ReadRaw(&edict->s.groups, sizeof(edict->s.groups));
   arc.ReadInteger(&edict->s.gunanim);
   arc.ReadInteger(&edict->s.gunframe);
   // index into configstrings
   arc.ReadInteger(&edict->s.gunmodelindex);
   arc.ReadInteger(&edict->s.lightofs);
   arc.ReadInteger(&edict->s.skinnum);
   arc.ReadInteger(&edict->s.effects);
   arc.ReadInteger(&edict->s.renderfx);
   arc.ReadInteger(&edict->s.solid);
   // index into configstrings
   arc.ReadInteger(&edict->s.sound);
   //   arc.ReadInteger( &edict->s.event );

   arc.ReadInteger(&edict->svflags);

   tempvec = arc.ReadVector();
   tempvec.copyTo(edict->mins);
   tempvec = arc.ReadVector();
   tempvec.copyTo(edict->maxs);
   tempvec = arc.ReadVector();
   tempvec.copyTo(edict->absmin);
   tempvec = arc.ReadVector();
   tempvec.copyTo(edict->absmax);
   tempvec = arc.ReadVector();
   tempvec.copyTo(edict->size);
   tempvec = arc.ReadVector();
   tempvec.copyTo(edict->fullmins);
   tempvec = arc.ReadVector();
   tempvec.copyTo(edict->fullmaxs);
   arc.ReadFloat(&edict->fullradius);
   tempvec = arc.ReadVector();
   tempvec.copyTo(edict->centroid);

   edict->solid = (solid_t)arc.ReadInteger();
   arc.ReadInteger(&edict->clipmask);

   temp = arc.ReadInteger();
   if(temp < 0)
   {
      edict->owner = NULL;
   }
   else
   {
      edict->owner = &g_edicts[temp];
   }

   arc.ReadFloat(&edict->freetime);
   arc.ReadFloat(&edict->spawntime);
   tempstr = arc.ReadString();
   strcpy(edict->entname, tempstr.c_str());

   gi.linkentity(edict);
}

/*
================
G_Milliseconds
================
*/
int G_Milliseconds(void)
{
#ifdef _WIN32
   static int        base;
   static qboolean   initialized = false;

   if(!initialized)
   {
      // let base retain 16 bits of effectively random data
      base = timeGetTime() & 0xffff0000;
      initialized = true;
   }

   return timeGetTime() - base;
#else
   //FIXME
   return 0;
#endif
}

/*
===============
G_DebugPrintf

Outputs a string to the debug window
===============
*/
void G_DebugPrintf(const char *fmt, ...)
{
   va_list  argptr;
   char     message[1024];

   va_start(argptr, fmt);
   vsnprintf(message, sizeof(message), fmt, argptr);
   va_end(argptr);

#ifdef _WIN32
   OutputDebugString(message);
#else
   gi.dprintf(message);
#endif
}

// EOF

