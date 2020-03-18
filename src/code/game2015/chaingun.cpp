//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/chaingun.cpp                 $
// $Revision:: 45                                                             $
//   $Author:: Aldie                                                          $
//     $Date:: 3/19/99 3:43p                                                  $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// High velocity chain gun
// 

#include "g_local.h"
#include "bullet.h"
#include "chaingun.h"
#include "rocketlauncher.h"
#include "explosion.h"
#include "specialfx.h"
#include "misc.h"
#include "surface.h"

#define BULLET_MODE 1

CLASS_DECLARATION(Projectile, Grenade, "grenade");

Event EV_Grenade_Explode("grenade_explode");

ResponseDef Grenade::Responses[] =
{
   { &EV_Touch,           (Response)&Grenade::Grenade_Touch },
   { &EV_Grenade_Explode, (Response)&Grenade::Explode       },
   { NULL, NULL }
};

EXPORT_FROM_DLL void Grenade::Grenade_Touch(Event *ev)
{
   Entity *other;

   other = ev->GetEntity(1);
   assert(other);

   if(other->entnum == owner)
      return;

   if(HitSky())
   {
      PostEvent(EV_Remove, 0);
      return;
   }

   if(!other->takedamage || other->health <= 0)
   {
      // Play a bouncy sound
      RandomSound("grenade_bounce", 1);
      return;
   }

   Explode(ev);
}

EXPORT_FROM_DLL void Grenade::Explode(Event *ev)
{
   int			damg;
   Vector		v;
   Entity		*other;
   Entity      *owner;

   other = ev->GetEntity(1);
   assert(other);

   if(other->isSubclassOf<Teleporter>())
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

   owner = G_GetEntity(this->owner);

   if(!owner)
      owner = world;

   damg = 75 + (int)G_Random(25);

   if(!deathmatch->value && owner->isClient())
      damg *= 1.5;

   if(other->takedamage)
      other->Damage(this, owner, damg, worldorigin, velocity, level.impact_trace.plane.normal, 30, 0, MOD_GRENADE, -1, -1, 1.0f);

   SpawnBlastDamage(&level.impact_trace, damg, owner);

   v = velocity;
   v.normalize();

   // don't do radius damage to the other, because all the damage
   // was done in the impact
   v = worldorigin - v * 24;
   CreateExplosion(v, damg, 1.0f, true, this, owner, other);
   PostEvent(EV_Remove, 0.1);
}

EXPORT_FROM_DLL void Grenade::Setup(Entity *owner, Vector pos, Vector forward, Vector right, Vector up)
{
   Event *ev;

   setModel("grenade.def");

   this->owner = owner->entnum;
   edict->owner = owner->edict;
   setMoveType(MOVETYPE_BOUNCE);
   setSolidType(SOLID_BBOX);
   takedamage = DAMAGE_YES;
   edict->clipmask = MASK_PROJECTILE;
   health = 10;
   SetGravityAxis(owner->gravaxis);

   velocity = forward * (500 + G_Random(200));
   velocity += up * (200 + crandom() * 10.0);
   velocity += right * (crandom() * 10.0);

   avelocity ={ 575, 0, 0 };

   ev = new Event(EV_Grenade_Explode);
   ev->AddEntity(world);

   if(ctf->value)
      PostEvent(ev, 0.9);
   else
      PostEvent(ev, 2.5 + G_Random(1.0));

   edict->s.effects |= EF_ROCKET;
   setOrigin(pos);
   worldorigin.copyTo(edict->s.old_origin);
   setSize({ -4, -4, -4 }, { 4, 4, 4 });

   edict->s.effects  |= EF_EVERYFRAME;
   RandomAnimate("smoke", NULL);
   
   //### give it a heat signature
   edict->s.effects |= EF_WARM;
   //###
}

CLASS_DECLARATION(BulletWeapon, ChainGun, "weapon_highvelocitygun");

ResponseDef ChainGun::Responses[] =
{
   { &EV_Weapon_Shoot, (Response)&ChainGun::Shoot },
   { NULL, NULL }
};

ChainGun::ChainGun() : BulletWeapon()
{
#ifdef SIN_DEMO
   PostEvent(EV_Remove, 0);
   return;
#endif
   SetModels("hvgun.def", "view_hvgun.def");
   SetAmmo("Bullet50mm", 1, 30);
   SetSecondaryAmmo("Rockets", 1, 5);
   SetRank(50, 50);
   SetType(WEAPON_2HANDED_LO);
   dualmode = true;
   modelIndex("grenade.def");
   modelIndex("rocket.def");
   modelIndex("rockets.def");
   modelIndex("50mm.def");
   modelIndex("sprites/blastmark.spr");
   modelIndex("sprites/hvblast.spr");
   modelIndex("sprites/tracer.spr");
   modelIndex("hvshell.def");

   if(ctf->value)
      alternate_fire = true;
}

void ChainGun::Shoot(Event *ev)
{
   if(weaponmode == PRIMARY)
   {
      if(deathmatch->value)
         FireBullets(1, { 300, 300, 300 }, 24, 32, DAMAGE_BULLET, MOD_CHAINGUN, false);
      else
         FireBullets(1, { 300, 300, 300 }, 16, 24, DAMAGE_BULLET, MOD_CHAINGUN, false);
      NextAttack(0);
   }
   else
   {
      Grenade *grenade;
      Vector pos;
      Vector forward;
      Vector up;
      Vector right;

      GetMuzzlePosition(&pos, &forward, &up, &right);
      grenade = new Grenade();
      grenade->Setup(owner, pos, forward, up, right);
      NextAttack(0.8);

      if(ctf->value)
         SetPrimaryMode();
   }
}

qboolean ChainGun::HasAmmo(Event *ev)
{
   if(!owner)
   {
      return false;
   }

   if(UnlimitedAmmo())
   {
      return true;
   }

   if((ammo_clip_size && ammo_in_clip >= ammorequired) || AmmoAvailable() >= ammorequired)
   {
      return true;
   }
   else if((ammo_clip_size && ammo_in_clip >= secondary_ammorequired) || AmmoAvailable() >= secondary_ammorequired)
   {
      return true;
   }

   return false;
}

// EOF

