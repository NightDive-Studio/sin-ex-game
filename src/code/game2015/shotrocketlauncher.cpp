//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/shotrocketlauncher.cpp           $
// $Revision:: 30                                                             $
//   $Author:: Markd                                                          $
//     $Date:: 11/13/98 3:30p                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// ShotRocketLauncher - Shoots slow moving shootable rockets

#include "g_local.h"
#include "explosion.h"
#include "shotrocketlauncher.h"
#include "rocket_turret.h"
#include "misc.h"
#include "surface.h"

CLASS_DECLARATION(Projectile, ShotRocket, nullptr);

Event EV_ShotRocket_Explode("explode");
Event EV_ShotRocket_HeatSeek("heatseek");

ResponseDef ShotRocket::Responses[] =
{
   { &EV_Touch,				      (Response)&ShotRocket::Explode },
   { &EV_Damage,                 (Response)&ShotRocket::DamageEvent },
   { &EV_ShotRocket_Explode,	   (Response)&ShotRocket::Explode },
   { &EV_ShotRocket_HeatSeek,	   (Response)&ShotRocket::HeatSeek },
   { nullptr, nullptr }
};

EXPORT_FROM_DLL void ShotRocket::DamageEvent(Event *ev)
{
   Entity *other;
   Event *event;
   int damage;

   damage = ev->GetInteger(1);

   if(damage <= 0)
      return;

   other = ev->GetEntity(2);

   event = new Event(EV_ShotRocket_Explode);
   event->AddEntity(other);
   ProcessEvent(event);
}

EXPORT_FROM_DLL void ShotRocket::Explode(Event *ev)
{
   int			damg;
   Vector		v;
   Entity		*other;
   Entity      *owner;

   other = ev->GetEntity(1);
   assert(other);

   owner = G_GetEntity(this->owner);

   if(!owner)
      owner = world;

   if(other->isSubclassOf<Teleporter>())
   {
      return;
   }

   if(other == owner)
   {
      return;
   }

   stopsound(CHAN_VOICE);
   setSolidType(SOLID_NOT);
   hideModel();

   if(HitSky())
   {
      PostEvent(EV_Remove, 0);
      return;
   }


   damg = 100 + (int)G_Random(20);

   takedamage = DAMAGE_NO;

   if(other->takedamage)
      other->Damage(this, owner, damg, worldorigin, velocity, level.impact_trace.plane.normal, 32, 0, MOD_SHOTROCKET, -1, -1, 1.0f);

   SpawnBlastDamage(&level.impact_trace, damg, owner);

   v = velocity;
   v.normalize();

   // don't do radius damage to the other, because all the damage
   // was done in the impact
   v = worldorigin - v * 36;
   CreateExplosion(v, damg, 1.0f, true, this, owner, other);
   PostEvent(EV_Remove, 0.1);
}

float ShotRocket::Distance(Entity *targ)
{
   Vector delta;

   delta = worldorigin - targ->centroid;
   return delta.length();
}

qboolean ShotRocket::CanSee(Entity *ent)
{
   Vector start;
   Vector end;
   trace_t trace;

   start = worldorigin;
   end = ent->centroid;

   // Check if he's visible
   trace = G_Trace(start, vec_zero, vec_zero, end, this, MASK_OPAQUE, "ShotRocket::CanSee");
   if(trace.fraction == 1.0 || trace.ent == ent->edict)
   {
      return true;
   }

   return false;
}

void ShotRocket::HeatSeek(Event *ev)
{
   // Seek out the closest client

   Entity	*ent;
   edict_t	*ed;
   int		i;
   float		dist;
   float		bestdist = 99999;
   Entity	*bestent;
   Vector   delta;

   bestent = nullptr;

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

      dist = Distance(ent);
      if((dist < bestdist) && CanSee(ent))
      {
         bestent = ent;
         bestdist = dist;
      }
   }

   if(!bestent)
   {
      PostEvent(ev, FRAMETIME);
      return;
   }
   if(bestdist < 16)
   {
      Event * ev;
      ev = new Event(EV_ShotRocket_Explode);
      ev->AddEntity(bestent);
      PostEvent(ev, 0);
   }

   delta = bestent->centroid - worldorigin;
   delta.normalize();
   velocity = delta * speed;
   angles = delta.toAngles();
   angles[PITCH] = -angles[PITCH];
   setAngles(angles);
   PostEvent(ev, FRAMETIME);
}

void ShotRocket::Setup(Entity *owner, Vector pos, Vector dir)
{
   Event *ev;
   RocketTurret *turret;

   this->owner = owner->entnum;
   edict->owner = owner->edict;

   if(owner->isSubclassOf<RocketTurret>())
   {
      turret = (RocketTurret *)owner;
      speed = turret->speed;
   }
   else
   {
      turret = nullptr;
      speed = 500;
   }

   setMoveType(MOVETYPE_FLYMISSILE);
   setSolidType(SOLID_BBOX);
   edict->clipmask = MASK_PROJECTILE;

   angles = dir.toAngles();
   angles[PITCH] = -angles[PITCH];
   setAngles(angles);

   velocity = dir * speed;

   // set missile duration
   ev = new Event(EV_ShotRocket_Explode);
   ev->AddEntity(world);
   PostEvent(ev, 15);

   takedamage = DAMAGE_YES;
   health = 1;

   setModel("rocket.def");
   edict->s.effects |= EF_ROCKET;
   edict->s.effects |= EF_EVERYFRAME;
   edict->s.angles[ROLL] = rand() % 360;
   avelocity = { 0, 0, 90 };
   gravity   = 0;
   edict->svflags |= SVF_USEBBOX;

   //### give it a heat signature
   edict->s.effects |= EF_WARM;
   //###

   // setup ambient thrust
   ev = new Event(EV_RandomEntitySound);
   ev->AddString("thrust");
   ProcessEvent(ev);

   setSize({ -8, -8, -8 }, { 8, 8, 8 });
   setOrigin(pos);
   worldorigin.copyTo(edict->s.old_origin);

   ev = new Event(EV_ShotRocket_HeatSeek);
   PostEvent(ev, FRAMETIME);
}

CLASS_DECLARATION(Weapon, ShotRocketLauncher, "weapon_shotrocketlauncher");

ResponseDef ShotRocketLauncher::Responses[] =
{
   { &EV_Weapon_Shoot,	(Response)&ShotRocketLauncher::Shoot },
   { nullptr, nullptr }
};

ShotRocketLauncher::ShotRocketLauncher() : Weapon()
{
   SetModels("rlaunch.def", "view_rlaunch.def");
   modelIndex("rockets.def");
   modelIndex("rocket.def");
   modelIndex("sprites/blastmark.spr");
   SetAmmo("Rockets", 1, 5);
   SetRank(70, 50);
}

void ShotRocketLauncher::Shoot(Event *ev)
{
   ShotRocket	      *rocket;
   Vector	         pos;
   Vector	         dir;
   RocketTurret      *turret;

   assert(owner);
   if(!owner)
   {
      return;
   }

   turret = (RocketTurret *)(Entity *)owner;

   GetMuzzlePosition(&pos, &dir);

   rocket = new ShotRocket();
   rocket->Setup(owner, pos, dir);

   MuzzleFlash(1.0, 0.1, 0, 400, 0.6, 0.2);
   NextAttack(turret->rate);
}

// EOF

