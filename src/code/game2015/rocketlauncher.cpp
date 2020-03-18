//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/rocketlauncher.cpp               $
// $Revision:: 84                                                             $
//   $Author:: Aldie                                                          $
//     $Date:: 3/02/99 9:14p                                                  $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Standard rocketlauncher, similar to the Quake and Doom rocketlaunchers.
// 

#include "g_local.h"
#include "explosion.h"
#include "rocketlauncher.h"
#include "worldspawn.h"
#include "specialfx.h"
#include "misc.h"
#include "surface.h"
#include "thrall.h"
#include "ctf.h"

#define ROCKET_SPEED    1000
#define ROCKET_RADIUS   150    // FIXME: max damage (90 +rand(20)) + 40 == 150... hard coded.  gotta pass this in

CLASS_DECLARATION(Projectile, Rocket, nullptr);

Event EV_Rocket_Explode("explode");

ResponseDef Rocket::Responses[] =
{
   { &EV_Touch,            (Response)&Rocket::Explode },
   { &EV_Rocket_Explode,   (Response)&Rocket::Explode },
   { nullptr, nullptr }
};

EXPORT_FROM_DLL void Rocket::Explode(Event *ev)
{
   int	 damg;
   Vector v;
   Entity *other;
   Entity *owner;

   other = ev->GetEntity(1);
   assert(other);

   if(other->isSubclassOf<Teleporter>())
   {
      return;
   }

   if(other->entnum == this->owner)
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

   damg = 90 + (int)G_Random(25);

   owner = G_GetEntity(this->owner);

   if(!owner)
      owner = world;

   // Single player packs a bigger punch
   if(!deathmatch->value && owner->isClient())
      damg *= 1.5;

   if(other->takedamage)
      other->Damage(this, owner, damg, worldorigin, velocity, level.impact_trace.plane.normal, 200, 0, MOD_ROCKET, -1, -1, 1.0f);

   SpawnBlastDamage(&level.impact_trace, damg, owner);

   v = velocity;
   v.normalize();

   // don't do radius damage to the other, because all the damage
   // was done in the impact
   v = worldorigin - v * 36;
   CreateExplosion(v, damg, 1.0f, true, this, owner, other);
   PostEvent(EV_Remove, 0.1);
}

EXPORT_FROM_DLL void Rocket::Setup(Entity *owner, Vector pos, Vector dir)
{
   Event *ev;

   this->owner = owner->entnum;
   edict->owner = owner->edict;

   setMoveType(MOVETYPE_FLYMISSILE);
   setSolidType(SOLID_BBOX);
   edict->clipmask = MASK_PROJECTILE;

   angles = dir.toAngles();
   angles[PITCH] = -angles[PITCH];
   setAngles(angles);

   speed = ROCKET_SPEED;
   velocity = dir * ROCKET_SPEED;

   // set missile duration
   ev = new Event(EV_Remove);
   ev->AddEntity(world);
   PostEvent(ev, 20);

   takedamage = DAMAGE_YES;
   health = 10;

   setModel("rocket.def");
   edict->s.renderfx |= RF_DLIGHT;
   edict->s.effects |= EF_ROCKET;
   edict->s.effects |= EF_EVERYFRAME;
   edict->s.angles[ROLL] = rand() % 360;
   avelocity = { 0, 0, 90 };
   gravity = 0;
   edict->s.color_r = 0.8;
   edict->s.color_g = 0.4;
   edict->s.color_b = 0;
   edict->s.radius  = 200;

   //### give it a heat signature
   edict->s.effects |= EF_WARM;
   //###

   // setup ambient thrust
   ev = new Event(EV_RandomEntitySound);
   ev->AddString("thrust");
   ProcessEvent(ev);

   setSize({ -1, -1, -1 }, { 1, 1, 1 });
   setOrigin(pos);
   worldorigin.copyTo(edict->s.old_origin);
}

CLASS_DECLARATION(Weapon, RocketLauncher, "weapon_rocketlauncher");

ResponseDef RocketLauncher::Responses[] =
{
   { &EV_Weapon_Shoot,	(Response)&RocketLauncher::Shoot },
   { nullptr, nullptr }
};

RocketLauncher::RocketLauncher() : Weapon()
{
   if(ctf->value) //###
      SetModels("rlaunch.def", "view_rlaunch_ctf.def");
   else
      SetModels("rlaunch.def", "view_rlaunch.def");

   modelIndex("rocket.def");
   modelIndex("rockets.def");
   modelIndex("sprites/blastmark.spr");
   gi.soundindex("weapons/rlaunch/stmmchn.wav");
   SetAmmo("Rockets", 1, 5);
   SetRank(70, 70);
   SetType(WEAPON_2HANDED_LO);

   SetMinRange(ROCKET_RADIUS);
   SetProjectileSpeed(ROCKET_SPEED);

   //### precaches for the guided missile
   if(deathmatch->value)
   {
      modelIndex("missile_w.def");
      modelIndex("view_missile.def");
      modelIndex("missile.def");
   }
   //###

   if(ctf->value)
   {
      // CTF rocketlauncher has alternate fire mode
      modelIndex("sprites/thrallpulse.spr");
      SetSecondaryAmmo("Rockets", 10, 0); //### 15 -> 10
      dualmode = true;
      alternate_fire = true;
   }
}

void RocketLauncher::Shoot(Event *ev)
{
   Rocket	      *rocket;
   ThrallPulse    *pulse;
   Vector	      pos;
   Vector	      dir;

   assert(owner);
   if(!owner)
   {
      return;
   }

   GetMuzzlePosition(&pos, &dir);

   if(weaponmode == PRIMARY)
   {
      rocket = new Rocket();
      rocket->Setup(owner, pos, dir);
      NextAttack(1.0);
   }
   else
   {
      pulse = new ThrallPulse();
      pulse->Setup(owner, pos, dir);
      NextAttack(1.0); //### 3.0 -> 1.0
      SetPrimaryMode();
   }
}

//### for guided missile
void RocketLauncher::SecondaryUse(Event *ev)
{
   // make sure he has it, but only in deathmatch
   if(!owner->HasItem("MissileLauncher") && deathmatch->value)
      owner->giveWeapon("MissileLauncher");

   owner->useWeapon("MissileLauncher");
}
//###

// EOF

