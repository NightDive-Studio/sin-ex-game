//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/g_phys.cpp                       $
// $Revision:: 61                                                             $
//   $Author:: Markd                                                          $
//     $Date:: 11/20/98 7:17p                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// 

#include "g_local.h"
#include "sentient.h"
#include "actor.h"

extern cvar_t *sv_fatrockets;

/*

pushmove objects do not obey gravity, and do not interact with each other or 
trigger fields, but block normal movement and push normal objects when they 
move.

onground is set for toss objects when they come to a complete rest.  it is 
set for steping or walking objects 

doors, plats, etc are SOLID_BSP, and MOVETYPE_PUSH
bonus items are SOLID_TRIGGER touch, and MOVETYPE_TOSS
corpses are SOLID_NOT and MOVETYPE_TOSS
crates are SOLID_BBOX and MOVETYPE_TOSS
walking monsters are SOLID_SLIDEBOX and MOVETYPE_STEP
flying/floating monsters are SOLID_SLIDEBOX and MOVETYPE_FLY

solid_edge items only clip against bsp models.

*/

typedef struct
{
   Entity	*ent;
   Vector	origin;
   Vector	worldorigin;
   Vector	angles;
   Vector	worldangles;
   float		deltayaw;
} pushed_t;

pushed_t	pushed[ MAX_EDICTS ];
pushed_t *pushed_p;

Entity *obstacle;

/*
============
G_TestEntityPosition

============
*/
Entity *G_TestEntityPosition(Entity *ent)
{
   int mask;
   trace_t trace;

   mask = ent->edict->clipmask;
   if(!mask)
      mask = MASK_SOLID;

   trace = G_Trace(ent->worldorigin, ent->mins, ent->maxs, ent->worldorigin, ent, mask, "G_TestEntityPosition");

   if(trace.startsolid)
   {
      //return g_edicts->entity;
      assert(trace.ent->entity);
      return trace.ent->entity;
   }

   return NULL;
}

/*
================
G_CheckVelocity
================
*/
void G_CheckVelocity(Entity *ent)
{
   int i;

   //
   // bound velocity
   //
   for(i = 0; i < 3; i++)
   {
      if(ent->velocity[i] > sv_maxvelocity->value)
      {
         ent->velocity[i] = sv_maxvelocity->value;
      }
      else if(ent->velocity[i] < -sv_maxvelocity->value)
      {
         ent->velocity[i] = -sv_maxvelocity->value;
      }
   }
}

/*
==================
G_Impact

Two entities have touched, so run their touch functions
==================
*/
void G_Impact(Entity *e1, trace_t *trace)
{
   edict_t    *e2;
   Event      *ev;
   qboolean    doImpactDamage = false;
   //### added for hit damage check
   Vector      tmpvec, vecdiff;
   int         hitdmg;
   //###

   e2 = trace->ent;

   if((e1->movetype == MOVETYPE_HURL) || (e2->entity->movetype == MOVETYPE_HURL))
   {
      doImpactDamage = true;
   }

   //FIXME - this should be passed in the event.
   level.impact_trace = *trace;

   //### do check for really hard impact
   if(e1->concussioner || e2->entity->concussioner)
   {
      if(e2)
         vecdiff = e2->entity->velocity - e1->velocity;
      else
         vecdiff = e1->velocity;

      if(vecdiff.length() > 900) // fast enough to hurt
      {
         hitdmg = (vecdiff.length() - 800)*0.06 + 0.5;
      }
      else 
         hitdmg = 0;
      tmpvec = e1->velocity;
   }
   //###

   if(e1->edict->solid != SOLID_NOT)
   {
      ev = new Event(EV_Touch);
      ev->AddEntity(e2->entity);
      e1->ProcessEvent(ev);

      if(doImpactDamage)
      {
         ev = new Event(EV_Sentient_ImpactDamage);
         e1->ProcessEvent(ev);
      }
   }

   if(e2->entity && e2->solid != SOLID_NOT)
   {
      ev = new Event(EV_Touch);
      ev->AddEntity(e1);
      e2->entity->ProcessEvent(ev);

      if(doImpactDamage)
      {
         ev = new Event(EV_Sentient_ImpactDamage);
         e2->entity->ProcessEvent(ev);
      }
   }

   //### now apply hit damage to each
   if(e1->concussioner || e2->entity->concussioner)
   {
      vecdiff = trace->plane.normal;
      tmpvec.normalize();
      if(hitdmg && fabs(DotProduct(vecdiff.vec3(), tmpvec.vec3())) > 0.35)
      {
         tmpvec = e1->origin + (e1->mins + e1->maxs)*0.5;
         gi.positioned_sound(tmpvec.vec3(), g_edicts, CHAN_AUTO,
                             gi.soundindex("weapons/concussion/sml1.wav"), 1, 1, 0, 1, 0, 0);

         if(e1->edict->inuse && e1->concussioner)
         {
            if(!e1->deadflag && e1->takedamage)
            {
               e1->Damage(e1->concussioner, e1->concussioner, hitdmg, e1->origin, vec_zero, vec_zero, 0, 0, MOD_CONCUSSION, -1, -1, 1.0f);

               if(e1->isSubclassOf<Sentient>())
               {
                  vecdiff = vec_zero - vecdiff;
                  ((Sentient *)e1)->SprayBlood(e1->centroid, vecdiff, hitdmg);
               }
            }
         }

         if(e2->inuse && e1->concussioner)
         {
            if(!e2->entity->deadflag && e2->entity->takedamage)
            {
               e2->entity->Damage(e1->concussioner, e1->concussioner, hitdmg, e2->entity->origin, vec_zero, vec_zero, 0, 0, MOD_CONCUSSION, -1, -1, 1.0f);

               if(e2->entity->isSubclassOf<Sentient>())
               {
                  vecdiff = vec_zero - vecdiff;
                  ((Sentient *)e2->entity)->SprayBlood(e2->entity->centroid, vecdiff, hitdmg);
               }
            }
         }
      }
   }
   //###

   memset(&level.impact_trace, 0, sizeof(level.impact_trace));
}

/*
=============
G_AddCurrents
=============
*/
void G_AddCurrents(Entity *ent, Vector *basevel)
{
   float			speed;
   csurface_t *surface;
   float			angle;
   Vector		vel;

   vel = vec_zero;

   //
   // add water currents
   //
   if(ent->watertype & MASK_CURRENT)
   {
      speed = sv_waterspeed->value;
      if((ent->waterlevel == 1) && (ent->groundentity))
      {
         speed /= 2;
      }

      if(ent->watertype & CONTENTS_CURRENT_0)
      {
         vel[0] += speed;
      }
      if(ent->watertype & CONTENTS_CURRENT_90)
      {
         vel[1] += speed;
      }
      if(ent->watertype & CONTENTS_CURRENT_180)
      {
         vel[0] -= speed;
      }
      if(ent->watertype & CONTENTS_CURRENT_270)
      {
         vel[1] -= speed;
      }
      if(ent->watertype & CONTENTS_CURRENT_UP)
      {
         vel[2] += speed;
      }
      if(ent->watertype & CONTENTS_CURRENT_DOWN)
      {
         vel[2] -= speed;
      }
   }

   //
   // add conveyor belt velocities
   //
   if(ent->groundentity && ent->groundsurface)
   {
      surface = ent->groundsurface;
      if((surface->flags & SURF_CONVEYOR) && (surface->flags & SURF_TRANSLATE))
      {
         angle = surface->trans_angle * (3.14159 / 180.0);

         vel[0] += -cos(angle) * surface->trans_mag;
         vel[1] += sin(angle) * surface->trans_mag;
      }
   }

   *basevel = vel;
}

/*
==================
G_ClipVelocity

Slide off of the impacting object
returns the blocked flags (1 = floor, 2 = step / wall)
==================
*/
#define STOP_EPSILON 0.1

int G_ClipVelocity(Vector &in, Vector &normal, Vector &out, float overbounce, int gravaxis)
{
   int	i;
   int	blocked;
   float	backoff;

   blocked = 0;
   if((normal[gravity_axis[gravaxis].z] * gravity_axis[gravaxis].sign) > 0)
   {
      // floor
      blocked |= 1;
   }
   if(!normal[gravity_axis[gravaxis].z])
   {
      // step
      blocked |= 2;
   }

   backoff = (in * normal) * overbounce;

   out = in - normal * backoff;
   for(i = 0; i < 3; i++)
   {
      if(out[i] > -STOP_EPSILON && out[i] < STOP_EPSILON)
      {
         out[i] = 0;
      }
   }

   return blocked;
}


/*
============
G_FlyMove

The basic solid body movement clip that slides along multiple planes
Returns the clipflags if the velocity was modified (hit something solid)
1 = floor
2 = wall / step
4 = dead stop
============
*/
#define MAX_CLIP_PLANES 5

int G_FlyMove(Entity *ent, Vector basevel, float time, int mask)
{
   Entity	*hit;
   edict_t	*edict;
   int		bumpcount, numbumps;
   Vector	dir;
   float		d;
   int		numplanes;
   vec3_t	planes[MAX_CLIP_PLANES];
   Vector	primal_velocity, original_velocity, new_velocity;
   int		i, j;
   trace_t	trace;
   Vector	end;
   float		time_left;
   int		blocked;
#if 0
   Vector   move;
   Vector	v;
#endif

   edict = ent->edict;

   numbumps = 4;

   blocked = 0;
   original_velocity = ent->velocity;
   primal_velocity = ent->velocity;
   numplanes = 0;

#if 1
   time_left = time;
#else
   time_left = 1.0;//time;

   v = ent->total_delta;
   v[1] = -v[1];  // sigh...
   MatrixTransformVector(v.vec3(), ent->orientation, move.vec3());
   move += ent->velocity * time;
   ent->total_delta = vec_zero;
#endif

   ent->groundentity = NULL;
   for(bumpcount = 0; bumpcount < numbumps; bumpcount++)
   {
#if 1
      end = ent->worldorigin + time_left * (ent->velocity + basevel);
#else
      end = ent->worldorigin + time_left * move;
#endif

      trace = G_Trace(ent->worldorigin, ent->mins, ent->maxs, end, ent, mask, "G_FlyMove");

      if(
         (trace.allsolid) ||
         (
         (trace.startsolid) &&
            (ent->movetype == MOVETYPE_VEHICLE)
            )
         )
      {
         // entity is trapped in another solid
         ent->velocity = vec_zero;
         return 3;
      }

      if(trace.fraction > 0)
      {
         // actually covered some distance
         ent->setOrigin(trace.endpos);
         original_velocity = ent->velocity;
         numplanes = 0;
      }

      if(trace.fraction == 1)
      {
         // moved the entire distance
         break;
      }

      hit = trace.ent->entity;

      if(trace.plane.normal[2] > 0.7)
      {
         // floor
         blocked |= 1;
         if(hit->getSolidType() == SOLID_BSP)
         {
            ent->groundentity = hit->edict;
            ent->groundentity_linkcount = hit->edict->linkcount;
            ent->groundplane = trace.plane;
            ent->groundsurface = trace.surface;
            ent->groundcontents = trace.contents;
         }
      }

      if(!trace.plane.normal[2])
      {
         // step
         blocked |= 2;
      }

      //
      // run the impact function
      //
      G_Impact(ent, &trace);
      if(!edict->inuse)
      {
         break;		// removed by the impact function
      }

      time_left -= time_left * trace.fraction;

      // clipped to another plane
      if(numplanes >= MAX_CLIP_PLANES)
      {
         // this shouldn't really happen
         ent->velocity = vec_zero;
         return 3;
      }

      VectorCopy(trace.plane.normal, planes[numplanes]);
      numplanes++;

      //
      // modify original_velocity so it parallels all of the clip planes
      //
      for(i = 0; i < numplanes; i++)
      {
         G_ClipVelocity(original_velocity, Vector(planes[i]), new_velocity, 1.01, ent->gravaxis);
         for(j = 0; j < numplanes; j++)
         {
            if(j != i)
            {
               if((new_velocity * planes[j]) < 0)
               {
                  // not ok
                  break;
               }
            }
         }

         if(j == numplanes)
         {
            break;
         }
      }

      if(i != numplanes)
      {
         // go along this plane
         ent->velocity = new_velocity;
      }
      else
      {
         // go along the crease
         if(numplanes != 2)
         {
            ent->velocity = vec_zero;
            return 7;
         }
         CrossProduct(planes[0], planes[1], dir.vec3());
         d = dir * ent->velocity;
         ent->velocity = dir * d;
      }

      //
      // if original velocity is against the original velocity, stop dead
      // to avoid tiny occilations in sloping corners
      //
      if((ent->velocity * primal_velocity) <= 0)
      {
         ent->velocity = vec_zero;
         return blocked;
      }
   }

   return blocked;
}

/*
============
G_AddGravity

============
*/
void G_AddGravity(Entity *ent)
{
   float grav;

   if(ent->waterlevel > 2)
   {
      grav = ent->gravity * 60 * FRAMETIME * gravity_axis[ent->gravaxis].sign;
   }
   else
   {
      grav = ent->gravity * sv_gravity->value * FRAMETIME * gravity_axis[ent->gravaxis].sign;
   }

   ent->velocity[gravity_axis[ent->gravaxis].z] -= grav;
}

/*
===============================================================================

PUSHMOVE

===============================================================================
*/

/*
============
G_PushEntity

Does not change the entities velocity at all
============
*/
trace_t G_PushEntity(Entity *ent, Vector push)
{
   trace_t	trace;
   Vector	start;
   Vector	end;
   int		mask;
   edict_t	*edict;

   start = ent->worldorigin;
   end = start + push;

retry:
   if(ent->edict->clipmask)
   {
      mask = ent->edict->clipmask;
   }
   else
   {
      mask = MASK_SOLID;
   }

   if(!sv_fatrockets->value && (ent->flags & FL_FATPROJECTILE))
   {
      trace = G_FullTrace(start, ent->mins, ent->maxs, end, ent->maxs.y, ent, mask, "G_PushEntity");
   }
   else
   {
      trace = G_Trace(start, ent->mins, ent->maxs, end, ent, mask, "G_PushEntity");
   }

   edict = ent->edict;

   ent->setOrigin(trace.endpos);

   if(trace.fraction != 1.0)
   {
      G_Impact(ent, &trace);

      // if the pushed entity went away and the pusher is still there
      if(!trace.ent->inuse && edict->inuse)
      {
         // move the pusher back and try again
         ent->setOrigin(start);
         goto retry;
      }
   }

   if(edict && (edict != ent->edict))
   {
      G_TouchTriggers(ent);
   }

   return trace;
}

/*
============
G_SlideEntity
============
*/
trace_t G_SlideEntity(Entity *ent, Vector push)
{
   trace_t	trace;
   Vector	start;
   Vector	end;
   int		mask;
   edict_t	*edict;

   start = ent->worldorigin;
   end = start + push;

   if(ent->edict->clipmask)
   {
      mask = ent->edict->clipmask;
   }
   else
   {
      mask = MASK_SOLID;
   }

   trace = G_Trace(start, ent->mins, ent->maxs, end, ent, mask, "G_SlideEntity");

   edict = ent->edict;

   ent->setOrigin(trace.endpos);

   return trace;
}


/*
================
G_SnapPosition

================
*/
/*
qboolean G_SnapPosition ( Entity *ent )
{
   int    x, y, z;
   Vector offset( 0, -1, 1 );
   Vector base;

   base = ent->worldorigin;
   for ( z = 0; z < 3; z++ )
   {
      ent->worldorigin.z = base.z + offset[ z ];
      for ( y = 0; y < 3; y++ )
      {
         ent->worldorigin.y = base.y + offset[ y ];
         for ( x = 0; x < 3; x++ )
         {
            ent->worldorigin.x = base.x + offset[ x ];
            if ( G_TestEntityPosition( ent ) )
            {
               ent->origin.x += offset[ x ];
               ent->origin.y += offset[ y ];
               ent->origin.z += offset[ z ];
               ent->setOrigin( ent->origin );
               return true;
            }
         }
      }
   }

   // can't find a good position, so put him back.
   ent->worldorigin = base;

   return false;
}
*/

/*
============
G_Push

Objects need to be moved back on a failed push,
otherwise riders would continue to slide.
============
*/
qboolean G_Push(Entity *pusher, Vector pushermove, Vector pusheramove)
{
   Entity		*check, *block;
   edict_t		*edict, *next;
   Vector		move, amove;
   Vector		mins, maxs;
   Vector		save;
   pushed_t		*p;
   Vector		org, org2, move2;
   float			mat[3][3];
   pushed_t		*pusher_p;

   // save the pusher's original position
   pusher_p = pushed_p;
   pushed_p->ent				= pusher;
   pushed_p->origin			= pusher->origin;
   pushed_p->worldorigin	= pusher->worldorigin;
   pushed_p->angles			= pusher->angles;
   pushed_p->worldangles	= pusher->worldangles;
   if(pusher->client)
   {
      pushed_p->deltayaw = pusher->client->ps.pmove.delta_angles[YAW];
   }
   pushed_p++;
   if(pushed_p >= &pushed[MAX_EDICTS])
   {
      gi.error(ERR_FATAL, "Pushed too many entities.");
   }

   // move the pusher to it's final position
   pusher->setAngles(pusher->angles + pusheramove);
   pusher->setOrigin(pusher->origin + pushermove);

   if(pusher->edict->solid == SOLID_NOT)
   {
      // Doesn't push anything
      return true;
   }

   // change the move to worldspace
   move = pusher->worldorigin - pusher_p->worldorigin;
   amove = pusher->worldangles - pusher_p->worldangles;

   // we need this for pushing things later
   AnglesToMat(amove.vec3(), mat);

   // find the bounding box
   mins = pusher->absmin;
   maxs = pusher->absmax;

   // see if any solid entities are inside the final position
   for(edict = g_edicts->next; edict != &active_edicts; edict = next)
   {
      assert(edict);
      assert(edict->inuse);
      assert(edict->entity);

      next = edict->next;
      check = edict->entity;

      if(check->movetype == MOVETYPE_PUSH ||
         check->movetype == MOVETYPE_STOP ||
         check->movetype == MOVETYPE_NONE ||
         check->movetype == MOVETYPE_NOCLIP)
      {
         continue;
      }

      // if the entity is standing on the pusher, it will definitely be moved
      if(check->groundentity != pusher->edict)
      {
         // Only move triggers and non-solid objects if they're sitting on a moving object
         if(check->edict->solid == SOLID_TRIGGER ||  check->edict->solid == SOLID_NOT)
         {
            continue;
         }

         // see if the ent needs to be tested
         if(check->absmin[0] >= maxs[0] ||
            check->absmin[1] >= maxs[1] ||
            check->absmin[2] >= maxs[2] ||
            check->absmax[0] <= mins[0] ||
            check->absmax[1] <= mins[1] ||
            check->absmax[2] <= mins[2])
         {
            continue;
         }

         // see if the ent's bbox is inside the pusher's final position
         if(!G_TestEntityPosition(check))
         {
            continue;
         }
      }

      if((pusher->movetype == MOVETYPE_PUSH) || (check->groundentity == pusher->edict))
      {
         // move this entity
         pushed_p->ent				= check;
         pushed_p->origin			= check->origin;
         pushed_p->worldorigin	= check->worldorigin;
         pushed_p->angles			= check->angles;
         pushed_p->worldangles	= check->worldangles;
         pushed_p++;
         if(pushed_p >= &pushed[MAX_EDICTS])
         {
            gi.error(ERR_FATAL, "Pushed too many entities.");
         }

         // save off the origin
         save = check->origin;

         // try moving the contacted entity
         move2 = move;

#if 0
         // The way id had this was wrong (delta_angles are shorts, not floats)
         // This is probably what it should be, but it's not interpolated by the client, so it jitters.
         if(check->client)
         {
            // FIXME: doesn't rotate monsters?
            //check->client->ps.pmove.delta_angles[ YAW ] += amove[ YAW ];

            check->client->ps.pmove.delta_angles[YAW] =
               ANGLE2SHORT(
                  SHORT2ANGLE(check->client->ps.pmove.delta_angles[YAW]) + amove[YAW]);
         }
#endif

         // figure movement due to the pusher's amove
         org = check->worldorigin - pusher->worldorigin;

         MatrixTransformVector(org.vec3(), mat, org2.vec3());
         move2 += org2 - org;

         //FIXME
         // We should probably do a flymove here so that we slide against other objects
         check->setOrigin(check->origin + check->getParentVector(move2));

         // may have pushed them off an edge
         if(check->groundentity != pusher->edict)
         {
            check->groundentity = NULL;
         }

         block = G_TestEntityPosition(check);
         if(!block)
         {
            // pushed ok
            check->link();

            // impact?
            continue;
         }

         // try to snap it to a good position
         /*
         if ( G_SnapPosition( check ) )
            {
            // snapped ok.  we don't have to link since G_SnapPosition does it for us.
            continue;
            }
         */

         // if it is ok to leave in the old position, do it
         // this is only relevent for riding entities, not pushed
         // FIXME: this doesn't acount for rotation
         check->setOrigin(save);
         block = G_TestEntityPosition(check);
         if(!block)
         {
            pushed_p--;
            continue;
         }
      }

      // save off the obstacle so we can call the block function
      obstacle = check;

      // move back any entities we already moved
      // go backwards, so if the same entity was pushed
      // twice, it goes back to the original position
      for(p = pushed_p - 1; p >= pushed; p--)
      {
         p->ent->angles = p->angles;
         p->ent->origin = p->origin;
         if(p->ent->client)
         {
            p->ent->client->ps.pmove.delta_angles[YAW] = p->deltayaw;
         }
      }

      // Only "really" move it in order so that the bound coordinate system is correct
      for(p = pushed; p < pushed_p; p++)
      {
         p->ent->setAngles(p->ent->angles);
         p->ent->setOrigin(p->ent->origin);
      }

      return false;
   }

   //FIXME: is there a better way to handle this?
   // see if anything we moved has touched a trigger
   for(p = pushed_p - 1; p >= pushed; p--)
   {
      G_TouchTriggers(p->ent);
   }

   return true;
}

/*
================
G_PushMove
================
*/
qboolean G_PushMove(Entity *ent, Vector move, Vector amove)
{
   Entity *part;
   Vector m, a;
   Event	 *ev;

   m = move;
   a = amove;

   pushed_p = pushed;
   for(part = ent; part; part = part->teamchain)
   {
      if(!G_Push(part, m, a))
      {
         // move was blocked
         // call the pusher's "blocked" function
         // otherwise, just stay in place until the obstacle is gone
         ev = new Event(EV_Blocked);
         ev->AddEntity(obstacle);
         part->ProcessEvent(ev);
         return false;
      }

      m = vec_zero;
      a = vec_zero;
   }

   return true;
}

/*
================
G_Physics_Pusher

Bmodel objects don't interact with each other, but
push all box objects
================
*/
void G_Physics_Pusher(Entity *ent)
{
   Vector	move, amove;
   Entity	*part, *mv;
   Event		*ev;

   // team slaves are only moved by their captains
   if(ent->flags & FL_TEAMSLAVE)
   {
      return;
   }

   // Check if anyone on the team is moving
   for(part = ent; part; part = part->teamchain)
   {
      if(part->velocity != vec_zero || part->avelocity != vec_zero)
      {
         break;
      }
   }

   // make sure all team slaves can move before commiting
   // any moves or calling any think functions
   // if the move is blocked, all moved objects will be backed out
   pushed_p = pushed;
   while(part)
   {
      move = part->velocity * FRAMETIME;
      amove = part->avelocity * FRAMETIME;
      if(!G_Push(part, move, amove))
      {
         // move was blocked
         break;
      }

      part = part->teamchain;
   }

   if(part)
   {
      // the move failed, bump all movedone times
      for(mv = ent; mv; mv = mv->teamchain)
      {
         mv->PostponeEvent(EV_MoveDone, FRAMETIME);
      }

      // if the pusher has a "blocked" function, call it
      // otherwise, just stay in place until the obstacle is gone
      ev = new Event(EV_Blocked);
      ev->AddEntity(obstacle);
      part->ProcessEvent(ev);
   }
}

//==================================================================

/*
=============
G_Physics_Noclip

A moving object that doesn't obey physics
=============
*/
void G_Physics_Noclip(Entity *ent)
{
   ent->angles += ent->avelocity * FRAMETIME;
   ent->origin += ent->velocity * FRAMETIME;
   ent->link();
}

/*
==============================================================================

TOSS / BOUNCE

==============================================================================
*/

/*
=============
G_Physics_Toss

Toss, bounce, and fly movement.  When onground, do nothing.
=============
*/
void G_Physics_Toss(Entity *ent)
{
   trace_t	trace;
   Vector	move;
   float		backoff;
   Entity	*slave;
   qboolean	wasinwater;
   qboolean	isinwater;
   Vector	old_origin;
   Vector	basevel;
   edict_t	*edict;
   qboolean onconveyor;
   const gravityaxis_t &grav = gravity_axis[ent->gravaxis];

   // if not a team captain, so movement will be handled elsewhere
   if(ent->flags & FL_TEAMSLAVE)
   {
      return;
   }

   if((ent->velocity[grav.z] * grav.sign) > 0)
   {
      ent->groundentity = NULL;
   }

   // check for the groundentity going away
   if(ent->groundentity && !ent->groundentity->inuse)
   {
      ent->groundentity = NULL;
   }

   //### check for ground falling out from underneath
   if(ent->groundentity)
   {
      Entity *ground = ent->groundentity->entity;

      if((ground->velocity[grav.z]*grav.sign) < 0)
      {
         ent->groundentity = NULL;
      }
   }
   //###

   G_AddCurrents(ent, &basevel);
   onconveyor = (basevel != vec_zero);

   // if onground, return without moving
   if(ent->groundentity && !onconveyor && (ent->movetype != MOVETYPE_VEHICLE))
   {
      if(ent->avelocity.length())
      {
         // move angles
         ent->setAngles(ent->angles + ent->avelocity * FRAMETIME);
      }
      ent->velocity = vec_zero;
      return;
   }

   old_origin = ent->origin;

   G_CheckVelocity(ent);

   // add gravity
   if(!onconveyor && ent->movetype != MOVETYPE_FLY && ent->movetype != MOVETYPE_FLYMISSILE)
   {
      G_AddGravity(ent);
   }

   // move angles
   ent->setAngles(ent->angles + ent->avelocity * FRAMETIME);

   // move origin
   move = (ent->velocity + basevel) * FRAMETIME;

   edict = ent->edict;
   if(ent->movetype == MOVETYPE_VEHICLE)
   {
      int mask;

      if(ent->edict->clipmask)
      {
         mask = ent->edict->clipmask;
      }
      else
      {
         mask = MASK_MONSTERSOLID;
      }
      G_FlyMove(ent, basevel, FRAMETIME, mask);
      G_TouchTriggers(ent);
      return;
   }
   else
   {
      trace = G_PushEntity(ent, move);
   }

   if((trace.fraction == 0) && (ent->movetype == MOVETYPE_SLIDE))
   {
      // Check for slide by removing the downward velocity
      Vector slide;

      slide[grav.x] = move[grav.x] * 0.7f;
      slide[grav.y] = move[grav.y] * 0.7f;
      slide[grav.z] = 0;
      G_PushEntity(ent, slide);
   }

   if(!edict->inuse)
   {
      return;
   }

   if(trace.fraction < 1)
   {
      if(ent->movetype == MOVETYPE_BOUNCE)
      {
         backoff = 1.5;
      }
      else
      {
         backoff = 1;
      }

      G_ClipVelocity(ent->velocity, Vector(trace.plane.normal), ent->velocity, backoff, ent->gravaxis);

      // stop if on ground
      if((trace.plane.normal[grav.z] * grav.sign) > 0.7)
      {
         if(((ent->velocity[grav.z] * grav.sign) < 60 || ent->movetype != MOVETYPE_BOUNCE) &&
            (ent->movetype != MOVETYPE_SLIDE))
         {
            ent->groundentity = trace.ent;
            ent->groundentity_linkcount = trace.ent->linkcount;
            ent->groundplane = trace.plane;
            ent->groundsurface = trace.surface;
            ent->groundcontents = trace.contents;
            ent->velocity = vec_zero;
            ent->avelocity = vec_zero;
         }
      }
   }

   if((move[grav.z] == 0) && onconveyor)
   {
      // Check if we still have a ground
      ent->CheckGround();
   }

   // check for water transition
   wasinwater = (ent->watertype & MASK_WATER);
   ent->watertype = gi.pointcontents(ent->worldorigin.vec3());
   isinwater = ent->watertype & MASK_WATER;

   if(isinwater)
   {
      ent->waterlevel = 1;
   }
   else
   {
      ent->waterlevel = 0;
   }

   if((edict->spawntime < (level.time - FRAMETIME)) && (ent->mass > 0))
   {
      if(!wasinwater && isinwater)
      {
         ent->RandomPositionedSound(old_origin.vec3(), "impact_watersplash");
      }
      else if(wasinwater && !isinwater)
      {
         ent->RandomPositionedSound(old_origin.vec3(), "impact_watersplash");
      }
   }

   // move teamslaves
   for(slave = ent->teamchain; slave; slave = slave->teamchain)
   {
      slave->origin = ent->origin;
      slave->link();
   }

   G_TouchTriggers(ent);
}

/*
===============================================================================

STEPPING MOVEMENT

===============================================================================
*/

void G_AddRotationalFriction(Entity *ent)
{
   int	n;
   float	adjustment;

   ent->angles += FRAMETIME * ent->avelocity;
   adjustment = FRAMETIME * sv_stopspeed->value * sv_friction->value;
   for(n = 0; n < 3; n++)
   {
      if(ent->avelocity[n] > 0)
      {
         ent->avelocity[n] -= adjustment;
         if(ent->avelocity[n] < 0)
         {
            ent->avelocity[n] = 0;
         }
      }
      else
      {
         ent->avelocity[n] += adjustment;
         if(ent->avelocity[n] > 0)
         {
            ent->avelocity[n] = 0;
         }
      }
   }
}

/*
=============
G_CheckWater

=============
*/

void G_CheckWater(Entity *ent)
{
   if(ent->isSubclassOf<Actor>())
   {
      static_cast<Actor *>(ent)->CheckWater();
   }
   else
   {
      ent->watertype = gi.pointcontents(ent->worldorigin.vec3());
      if(ent->watertype & MASK_WATER)
      {
         ent->waterlevel = 1;
      }
      else
      {
         ent->waterlevel = 0;
      }
   }
}

/*
=============
G_Physics_Step

Monsters freefall when they don't have a ground entity, otherwise
all movement is done with discrete steps.

This is also used for objects that have become still on the ground, but
will fall if the floor is pulled out from under them.
FIXME: is this true?
=============
*/

void G_Physics_Step(Entity *ent)
{
   qboolean	wasonground;
   qboolean	hitsound = false;
   Vector	vel;
   float		speed, newspeed, control;
   float		friction;
   int		mask;
   Vector	basevel;

   // airborn monsters should always check for ground
   if(!ent->groundentity)
   {
      ent->CheckGround();
   }

   if(ent->groundentity)
   {
      wasonground = true;
      ent->concussioner = NULL; //###
   }
   else
   {
      wasonground = false;
   }

   G_CheckVelocity(ent);

   if(ent->avelocity != vec_zero)
   {
      G_AddRotationalFriction(ent);
   }

   // add gravity except:
   //   flying monsters
   //   swimming monsters who are in the water
   if(!wasonground)
   {
      if(!(ent->flags & FL_FLY))
      {
         if(!((ent->flags & FL_SWIM) && (ent->waterlevel > 2)))
         {
            if(ent->velocity[gravity_axis[ent->gravaxis].z] < sv_gravity->value * ent->gravity * -0.1 *
               gravity_axis[ent->gravaxis].sign)
            {
               hitsound = true;
            }

            // Testing water gravity.  If this doesn't work, just restore the uncommented lines
            //if ( ent->waterlevel == 0 )
            //{
            G_AddGravity(ent);
            //}
         }
      }
   }

   // friction for flying monsters that have been given vertical velocity
   if((ent->flags & FL_FLY) && (ent->velocity.z != 0))
   {
      speed = fabs(ent->velocity.z);
      control = speed < sv_stopspeed->value ? sv_stopspeed->value : speed;
      friction = sv_friction->value / 3;
      newspeed = speed - (FRAMETIME * control * friction);
      if(newspeed < 0)
      {
         newspeed = 0;
      }
      newspeed /= speed;
      ent->velocity.z *= newspeed;
   }

   // friction for flying monsters that have been given vertical velocity
   if((ent->flags & FL_SWIM) && (ent->velocity.z != 0))
   {
      speed = fabs(ent->velocity.z);
      control = speed < sv_stopspeed->value ? sv_stopspeed->value : speed;
      newspeed = speed - (FRAMETIME * control * sv_waterfriction->value * ent->waterlevel);
      if(newspeed < 0)
      {
         newspeed = 0;
      }
      newspeed /= speed;
      ent->velocity.z *= newspeed;
   }

   if(ent->velocity != vec_zero)
   {
      // apply friction
      // let dead monsters who aren't completely onground slide
      if((wasonground) || (ent->flags & (FL_SWIM | FL_FLY)))
      {
         if(!(ent->health <= 0.0 && !M_CheckBottom(ent)))
         {
            vel = ent->velocity;
            vel.z = 0;
            speed = vel.length();
            if(speed)
            {
               friction = sv_friction->value;

               control = speed < sv_stopspeed->value ? sv_stopspeed->value : speed;
               newspeed = speed - FRAMETIME * control * friction;

               if(newspeed < 0)
               {
                  newspeed = 0;
               }

               newspeed /= speed;

               ent->velocity.x *= newspeed;
               ent->velocity.y *= newspeed;
            }
         }
      }
   }

   G_AddCurrents(ent, &basevel);

   if((basevel != vec_zero) || (ent->velocity != vec_zero) || (ent->total_delta != vec_zero))
   {
      if(ent->edict->svflags & SVF_MONSTER)
      {
         mask = MASK_MONSTERSOLID;
      }
      else
      {
         mask = MASK_SOLID;
      }

      G_FlyMove(ent, basevel, FRAMETIME, mask);

      ent->link();

      G_CheckWater(ent);
      G_TouchTriggers(ent);

      if(ent->groundentity && !wasonground && hitsound)
      {
         ent->RandomGlobalSound("impact_softland", 0.5f, CHAN_BODY, 1);
      }
   }
}

//============================================================================
//###
#define MAX_CLIP_PLANES 5
/*
============
G_FlyCeilingMove

The basic solid body movement clip that slides along multiple planes
Returns the clipflags if the velocity was modified (hit something solid)
1 = floor
2 = wall / step
4 = dead stop
============
*/
static int G_FlyCeilingMove (Entity *ent, Vector basevel,float time, int mask)	
{
   Entity  *hit;
   edict_t *edict;
   int      bumpcount, numbumps;
   Vector   dir;
   float    d;
   int      numplanes;
   vec3_t   planes[ MAX_CLIP_PLANES ];
   Vector   primal_velocity, original_velocity, new_velocity;
   int      i, j;
   trace_t  trace;
   Vector   end;
   float    time_left;
   int      blocked;

   edict = ent->edict;

   numbumps = 4;

   blocked = 0;
   original_velocity = ent->velocity;
   primal_velocity   = ent->velocity;
   numplanes = 0;

   time_left = time;

   ent->groundentity = NULL;
   for(bumpcount = 0; bumpcount < numbumps; bumpcount++)
   {
      end = ent->worldorigin + time_left * (ent->velocity + basevel);

      trace = G_Trace(ent->worldorigin, ent->mins, ent->maxs, end, ent, mask, "G_FlyCeilingMove");

      if((trace.allsolid) ||
         ((trace.startsolid) &&
          (ent->movetype == MOVETYPE_VEHICLE)))
      {
         // entity is trapped in another solid
         ent->velocity = vec_zero;
         return 3;
      }

      if(trace.fraction > 0)
      {
         // actually covered some distance
         ent->setOrigin(trace.endpos);
         original_velocity = ent->velocity;
         numplanes = 0;
      }

      if(trace.fraction == 1)
      {
         // moved the entire distance
         break;
      }

      hit = trace.ent->entity;

      if(trace.plane.normal[2] < -0.7)
      {
         // floor
         blocked |= 1;
         if(hit->getSolidType() == SOLID_BSP)
         {
            ent->groundentity = hit->edict;
            ent->groundentity_linkcount = hit->edict->linkcount;
            ent->groundplane = trace.plane;
            ent->groundsurface = trace.surface;
            ent->groundcontents = trace.contents;
         }
      }

      if(!trace.plane.normal[2])
      {
         // step
         blocked |= 2;
      }

      //
      // run the impact function
      //
      G_Impact(ent, &trace);
      if(!edict->inuse)
      {
         break; // removed by the impact function
      }

      time_left -= time_left * trace.fraction;

      // cliped to another plane
      if(numplanes >= MAX_CLIP_PLANES)
      {
         // this shouldn't really happen
         ent->velocity = vec_zero;
         return 3;
      }

      VectorCopy(trace.plane.normal, planes[numplanes]);
      numplanes++;

      //
      // modify original_velocity so it parallels all of the clip planes
      //
      for(i = 0; i < numplanes; i++)
      {
         Vector FooVec3(planes[i]);
         G_ClipVelocity(original_velocity, FooVec3, new_velocity, 1.01, ent->gravaxis);
         for(j = 0; j < numplanes; j++)
         {
            if(j != i)
            {
               if((new_velocity * planes[j]) < 0)
               {
                  // not ok
                  break;
               }
            }
         }

         if(j == numplanes)
         {
            break;
         }
      }

      if(i != numplanes)
      {
         // go along this plane
         ent->velocity = new_velocity;
      }
      else
      {
         // go along the crease
         if(numplanes != 2)
         {
            ent->velocity = vec_zero;
            return 7;
         }
         CrossProduct(planes[0], planes[1], dir.vec3());
         d = dir * ent->velocity;
         ent->velocity = dir * d;
      }

      //
      // if original velocity is against the original velocity, stop dead
      // to avoid tiny occilations in sloping corners
      //
      if((ent->velocity * primal_velocity) <= 0)
      {
         ent->velocity = vec_zero;
         return blocked;
      }
   }

   return blocked;
}

/*
=============
G_Physics_CeilingStep

Monsters freefall when they don't have a ground entity, otherwise
all movement is done with discrete steps.

This is also used for objects that have become still on the ground, but
will fall if the floor is pulled out from under them.
FIXME: is this TRUE?
=============
*/

static void G_Physics_CeilingStep (Entity *ent)	
{
   qboolean wasonground;
   qboolean hitsound = false;
   Vector   vel;
   float    speed, newspeed, control;
   float    friction;
   int      mask;
   Vector   basevel;

   // airborn monsters should always check for ground
   if(!ent->groundentity)
   {
      ent->CheckCeilingGround();
   }

   if(ent->groundentity)
   {
      wasonground = true;
      ent->concussioner = NULL; //###
   }
   else
   {
      wasonground = false;
   }

   G_CheckVelocity(ent);

   if(ent->avelocity != vec_zero)
   {
      G_AddRotationalFriction(ent);
   }

   // add gravity except:
   //   flying monsters
   //   swimming monsters who are in the water
   if(!wasonground)
   {
      if(!(ent->flags & FL_FLY))
      {
         if(!((ent->flags & FL_SWIM) && (ent->waterlevel > 2)))
         {
            float grav;

            //if(ent->velocity[ gravity_axis[ ent->gravaxis ].z ] < sv_gravity->value * ent->gravity * -0.1 *
            //   gravity_axis[ ent->gravaxis ].sign )
            if(ent->velocity[gravity_axis[ent->gravaxis].z] > sv_gravity->value * ent->gravity * -0.1 *
               gravity_axis[ent->gravaxis].sign)
            {
               hitsound = true;
            }

            // Testing water gravity.  If this doesn't work, just restore the uncommented lines
            //if( ent->waterlevel == 0 )
            //{
            //   G_AddGravity( ent );
            //}

            // don't add gravity if already moving towards the ceiling fast enough
            if(ent->velocity.z < 300)
            {
               grav = ent->gravity * sv_gravity->value * FRAMETIME * gravity_axis[ent->gravaxis].sign;

               // reverse gravity while on the ceiling to prevent weird
               // bobbing movements
               //ent->velocity[ gravity_axis[ ent->gravaxis ].z ] -= grav;
               ent->velocity.z += grav;
            }
         }
      }
   }

   // there won't be any flying or swiming stuff on the ceiling
#if 0
   // friction for flying monsters that have been given vertical velocity
   if((ent->flags & FL_FLY) && (ent->velocity.z != 0))
   {
      speed = Burger::FPMath::Abs(ent->velocity.z);
      control = speed < sv_stopspeed->value ? sv_stopspeed->value : speed;
      friction = sv_friction->value / 3;
      newspeed = speed - (FRAMETIME * control * friction);
      if(newspeed < 0)
      {
         newspeed = 0;
      }
      newspeed /= speed;
      ent->velocity.z *= newspeed;
   }

   // friction for flying monsters that have been given vertical velocity
   if((ent->flags & FL_SWIM) && (ent->velocity.z != 0))
   {
      speed = Burger::FPMath::Abs(ent->velocity.z);
      control = speed < sv_stopspeed->value ? sv_stopspeed->value : speed;
      newspeed = speed - (FRAMETIME * control * sv_waterfriction->value * ent->waterlevel);
      if(newspeed < 0)
      {
         newspeed = 0;
      }
      newspeed /= speed;
      ent->velocity.z *= newspeed;
   }
#endif

   if(ent->velocity != vec_zero)
   {
      // apply friction
      // let dead monsters who aren't completely onground slide
      if((wasonground) || (ent->flags & (FL_SWIM | FL_FLY)))
      {
         if(!(ent->health <= 0.0 && !M_CeilingCheckBottom(ent)))
         {
            vel = ent->velocity;
            vel.z = 0;
            speed = vel.length();
            if(speed)
            {
               friction = sv_friction->value;

               control = speed < sv_stopspeed->value ? sv_stopspeed->value : speed;
               newspeed = speed - FRAMETIME * control * friction;

               if(newspeed < 0)
               {
                  newspeed = 0;
               }

               newspeed /= speed;

               ent->velocity.x *= newspeed;
               ent->velocity.y *= newspeed;
            }
         }
      }
   }

   G_AddCurrents(ent, &basevel);

   if((basevel != vec_zero) || (ent->velocity != vec_zero) || (ent->total_delta != vec_zero))
   {
      if(ent->edict->svflags & SVF_MONSTER)
      {
         mask = MASK_MONSTERSOLID;
      }
      else
      {
         mask = MASK_SOLID;
      }

      G_FlyCeilingMove(ent, basevel, FRAMETIME, mask);

      ent->link();

      G_CheckWater(ent);
      G_TouchTriggers(ent);

      if(ent->groundentity && !wasonground && hitsound)
      {
         ent->RandomGlobalSound("impact_softland", 0.5f, CHAN_BODY, 1);
      }
   }
}
//###
//============================================================================

//### added physics type functions
class RopePiece;
class Hoverbike;
void G_Physics_Rope(RopePiece *ent);
int  G_Physics_Hoverbike(Hoverbike *ent); // haleyjd 20170605: return type is int, not void
//###

/*
================
G_RunEntity

================
*/
void G_RunEntity(Entity *ent)
{
   edict_t *edict;

   edict = ent->edict;

   if(ent->animating && !level.intermissiontime)
   {
      ent->AnimateFrame();
   }

   if(edict->inuse && ent->flags & FL_PRETHINK)
   {
      ent->Prethink();
   }

   if(edict->inuse)
   {
      switch((int)ent->movetype)
      {
      case MOVETYPE_PUSH:
      case MOVETYPE_STOP:
         G_Physics_Pusher(ent);
         break;
      case MOVETYPE_NONE:
      case MOVETYPE_WALK:
         break;
      case MOVETYPE_NOCLIP:
         G_Physics_Noclip(ent);
         break;
      case MOVETYPE_STEP:
      case MOVETYPE_HURL:
         G_Physics_Step(ent);
         break;
      case MOVETYPE_TOSS:
      case MOVETYPE_BOUNCE:
      case MOVETYPE_FLY:
      case MOVETYPE_FLYMISSILE:
      case MOVETYPE_SLIDE:
      case MOVETYPE_VEHICLE:
         G_Physics_Toss(ent);
         break;
      //###
      case MOVETYPE_ROPE:
         // added for ropes
         // rope movement physics are contained in rope.cpp
         G_Physics_Rope((RopePiece *)ent);
         break;
      case MOVETYPE_HOVERBIKE:
         // riderless hoverbike movement
         G_Physics_Hoverbike((Hoverbike *)ent);
         break;
      case MOVETYPE_CEILINGSTEP:
         // for walking on the ceiling
         G_Physics_CeilingStep(ent);
         break;
      //###
      default:
         gi.error("G_Physics: bad movetype %i", (int)ent->movetype);
      }
   }

   if((edict->inuse) && (ent->flags & FL_POSTTHINK))
   {
      ent->Postthink();
   }
}

// EOF

