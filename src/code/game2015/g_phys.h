//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/g_phys.h                         $
// $Revision:: 5                                                              $
//   $Author:: Jimdose                                                        $
//     $Date:: 11/13/98 10:04p                                                $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Global header file for g_phys.cpp
// 

#ifndef __G_PHYS_H__
#define __G_PHYS_H__

#include "g_local.h"

typedef enum
{
   STEPMOVE_OK,
   STEPMOVE_BLOCKED_BY_ENTITY,
   STEPMOVE_BLOCKED_BY_WORLD,
   STEPMOVE_BLOCKED_BY_WATER,
   STEPMOVE_BLOCKED_BY_FALL,
   STEPMOVE_STUCK
} stepmoveresult_t;

#define	STEPSIZE	18

// movetype values
typedef enum
{
   MOVETYPE_NONE,       // never moves
   MOVETYPE_NOCLIP,     // origin and angles change with no interaction
   MOVETYPE_PUSH,       // no clip to world, push on box contact
   MOVETYPE_STOP,       // no clip to world, stops on box contact
   MOVETYPE_WALK,       // gravity
   MOVETYPE_STEP,       // gravity, special edge handling
   MOVETYPE_FLY,
   MOVETYPE_TOSS,       // gravity
   MOVETYPE_FLYMISSILE, // extra size to monsters
   MOVETYPE_BOUNCE,
   MOVETYPE_SLIDE,
   MOVETYPE_VEHICLE,
   MOVETYPE_HURL,
   //###
   MOVETYPE_ROPE,       // physics type for ropes
   MOVETYPE_HOVERBIKE,  // movement for hoverbikes
   MOVETYPE_CEILINGSTEP // ceiling movement type for the crawler
   //###
} movetype_t;

void     G_RunEntity(Entity *ent);
void     G_Impact(Entity *e1, trace_t *trace);
qboolean G_PushMove(Entity *pusher, Vector move, Vector amove);
void     G_CheckWater(Entity *ent);
//###
void     G_AddGravity(Entity *ent);
void     G_AddCurrents(Entity *ent,Vector *basevel);
void     G_CheckVelocity(Entity *ent);
int      G_ClipVelocity(Vector &in, Vector &normal, Vector &out, float overbounce, int gravaxis);
//###

#endif /* g_phys.h */

// EOF

