//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/bouncingbetty.cpp                $
// $Revision:: 24                                                             $
//   $Author:: Jimdose                                                        $
//     $Date:: 11/12/98 11:30p                                                $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// 

#include "g_local.h"
#include "bouncingbetty.h"
#include "explosion.h"
#include "specialfx.h"

Event EV_Betty_CheckVicinity( "checkvicinity" );
Event EV_Betty_Launch( "launch" );
Event EV_Betty_AttackFinished( "attack_finished" );
Event EV_Betty_Fire( "fire" );
Event EV_Betty_Detonate( "detonate" );
Event EV_Betty_Explode( "explode" );
Event EV_BettySpike_Bubbles( "bubble" );

CLASS_DECLARATION( Entity, BettyLauncher, "trap_bouncingbetty" );

ResponseDef BettyLauncher::Responses[] =
{
   { &EV_Betty_CheckVicinity,  (Response)&BettyLauncher::CheckVicinity },
   { &EV_Activate,             (Response)&BettyLauncher::Launch },
   { &EV_Betty_Launch,         (Response)&BettyLauncher::Launch },
   { &EV_Betty_AttackFinished, (Response)&BettyLauncher::AttackFinished },
   { &EV_Betty_Fire,           (Response)&BettyLauncher::ReleaseBetty },
   { &EV_Killed,               (Response)&BettyLauncher::Killed },
   { NULL, NULL }
};

BettyLauncher::BettyLauncher() : Entity()
{
   setModel("iris.def");
   modelIndex("betty.def");
   modelIndex("bettyspike.def");
   RandomAnimate("idle", NULL);

   setSize({ -16, -16, 0 }, { 16, 16, 2 });

   setMoveType(MOVETYPE_NONE);
   setSolidType(SOLID_BBOX);
   health = G_GetFloatArg("health", 200);

   takedamage = DAMAGE_YES;
   flags |= FL_SPARKS;

   firing = false;
   activator = 0;

   if(!Targeted())
   {
      PostEvent(EV_Betty_CheckVicinity, 0.3f);
   }
}

void BettyLauncher::Killed(Event *ev)
{
   takedamage = DAMAGE_NO;
   CreateExplosion(worldorigin, 30, 0.5, true, this, this, this);
   PostEvent(EV_Remove, 0);
}

qboolean BettyLauncher::inRange(Entity *ent)
{
   Vector delta;
   float dist;

   delta = worldorigin - ent->worldorigin;
   dist = delta.length();

   if(dist > BOUNCINGBETTY_RANGE)
   {
      return false;
   }
   return true;
}

void BettyLauncher::CheckVicinity(Event *ev)
{
   Entity	*ent;
   edict_t	*ed;
   int		i;
   Event		*e;

   if(firing)
   {
      return;
   }

   for(i = 0; i < game.maxclients; i++)
   {
      ed = &g_edicts[1 + i];
      if(!ed->inuse || !ed->entity)
      {
         continue;
      }

      ent = ed->entity;
      if((ent->health < 0) || (ent->flags & FL_NOTARGET))
      {
         continue;
      }

      if(inRange(ent))
      {
         e = new Event(EV_Betty_Launch);
         e->AddEntity(ent);
         ProcessEvent(e);

         // wait a couple seconds before checking again
         PostEvent(EV_Betty_CheckVicinity, 2);
         return;
      }
   }

   PostEvent(EV_Betty_CheckVicinity, 0.3f);
}

void BettyLauncher::Launch(Event *ev)
{
   if(!firing)
   {
      firing = true;
      activator = (int)ev->GetEntity(1)->entnum;
      RandomAnimate("open", NULL);
   }
}

void BettyLauncher::AttackFinished(Event *ev)
{
   firing = false;
}

void BettyLauncher::ReleaseBetty (    Event *ev ) 
{
   BouncingBetty *betty;

   betty = new BouncingBetty();
   betty->Launch(worldorigin, activator);
   activator = 0;
}

CLASS_DECLARATION( Entity, BouncingBetty, NULL );

ResponseDef BouncingBetty::Responses[] =
{
   {&EV_Betty_Detonate, (Response)&BouncingBetty::Detonate},
   {&EV_Betty_Explode,  (Response)&BouncingBetty::Explode},
   {NULL, NULL}
};

BouncingBetty::BouncingBetty() : Entity()
{
   setModel("betty.def");
   setSize({ -4, -4, 0 }, { 4, 4, 8 });

   setMoveType(MOVETYPE_BOUNCE);
   setSolidType(SOLID_BBOX);
}

void BouncingBetty::Launch(Vector pos, int activatorEnt)
{
   activator = activatorEnt;

   setOrigin(pos);
   worldorigin.copyTo(edict->s.old_origin);
   velocity  = { 0, 0, 400 };
   avelocity = { 0, 180, 0 };
   watertype = gi.pointcontents(worldorigin.vec3());

   RandomAnimate("detonate", NULL);
}

void BouncingBetty::Detonate(Event *ev)
{
   RandomAnimate("detonate", NULL);
}

void BouncingBetty::Explode(Event *ev)
{
   BettySpike	*spike;
   Entity		*ent;
   Vector		vec;
   Vector		v;
   int			i;
   float			r;

   ent = G_GetEntity(activator);

   CreateExplosion(worldorigin, 150, 1.0f, true, this, ent, this);

   vec = Vector(25, 0, 0);

   r = G_Random(360);
   for(i = 0; i < 12; i++)
   {
      vec[1] = r;
      vec.AngleVectors(&v, NULL, NULL);

      spike = new BettySpike();
      spike->Setup(worldorigin + v * 8, v);

      r += 360.0 / 12.0;
   }

   PostEvent(EV_Remove, 0);
}

CLASS_DECLARATION(Entity, BettySpike, NULL);

ResponseDef BettySpike::Responses[] =
{
   {&EV_Touch, (Response)&BettySpike::SpikeTouch},
   {NULL, NULL}
};

EXPORT_FROM_DLL void BettySpike::SpikeTouch(Event *ev)
{
   Entity *other;
   int kick = 10;

   if(HitSky())
   {
      PostEvent(EV_Remove, 0);
      return;
   }

   other = ev->GetEntity(1);
   assert(other);

   if(other->health)
   {
      other->Damage(this, this, 10, worldorigin, velocity, level.impact_trace.plane.normal, kick, 0, MOD_BETTYSPIKE, -1, -1, 1.0f);
      RandomGlobalSound("impact_goryimpact", 1);
   }
   else
   {
      RandomGlobalSound("impact_bulletcase", 0.3);
   }

   PostEvent(EV_Remove, 0);
}

EXPORT_FROM_DLL void BettySpike::Setup(Vector pos, Vector dir)
{
   setModel("bettyspike.def");
   setMoveType(MOVETYPE_FLYMISSILE);
   setSolidType(SOLID_BBOX);
   setSize({ 0, 0, 0 }, { 0, 0, 0 });
   setOrigin(pos);
   worldorigin.copyTo(edict->s.old_origin);
   watertype = gi.pointcontents(worldorigin.vec3());

   // set missile speed
   velocity = dir;
   velocity.normalize();
   velocity *= 500 + G_CRandom(100);

   angles = dir.toAngles();
   angles[PITCH] = -angles[PITCH];
   setAngles(angles);

   PostEvent(EV_Remove, 5);
}

// EOF

