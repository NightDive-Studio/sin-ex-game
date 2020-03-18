/*
================================================================
GUIDED MISSILE LAUNCHER
================================================================

Copyright (C) 2020 by Night Dive Studios, Inc.
All rights reserved.

See the license.txt file for conditions and terms of use for this code.
*/

#include "guidedmissile.h"
#include "explosion.h"
#include "surface.h"
#include "misc.h"

#define ROCKET_RADIUS	150
#define MISSILE_TIME	40
#define MISSILE_AFTER_TIME 0.5

Event EV_Missile_Explode("explode");
Event EV_Missile_StartOpen("startopen");
Event EV_Missile_FinishOpen("finishopen");

//=================================================================

CLASS_DECLARATION(Entity, MissileView, nullptr);

ResponseDef MissileView::Responses[] =
{
   { nullptr, nullptr }
};

MissileView::~MissileView()
{
   if(!owner)
      return;

   // return player's view back to normal
   if(owner->isSubclassOf<Player>())
   {
      static_cast<Player *>(owner.ptr)->SetViewMode(oldviewmode, nullptr);
      // turn on the missile overlay
      static_cast<Player *>(owner.ptr)->MissileOverlayOff();
   }
}

void MissileView::Setup(Entity *missile, int owner, viewmode_t oldvmode)
{
   setSolidType(SOLID_NOT);
   setMoveType(MOVETYPE_FLY);

   setModel("sprites/null.spr");
   hideModel();
   StopAnimating();

   this->owner = G_GetEntity(owner);
   this->missile = missile;
   oldviewmode = oldvmode;

   starttime = level.time + 1;

   // set the player as this model's owner
   edict->owner = this->owner->edict;
   edict->svflags |= SVF_ONLYPARENT;
}

void MissileView::SetupMissile(void)
{
   if(!missile)
   {
      angles.AngleVectors(&velocity, nullptr, nullptr);
      velocity *= -24;

      // check remove timmer
      if(!removetime)
      {
         removetime = level.time + MISSILE_AFTER_TIME;
         setOrigin(origin + velocity*4);
      }
      else if(removetime < level.time)
      {
         if(!owner)
            return;

         // return player's view back to normal
         if(owner->isSubclassOf<Player>())
         {
            static_cast<Player *>(owner.ptr)->SetViewMode(oldviewmode, nullptr);
            // turn on the missile overlay
            static_cast<Player *>(owner.ptr)->MissileOverlayOff();
         }

         owner = nullptr;
         PostEvent(EV_Remove, 0.1);

         return;
      }

      return;
   }

   missile->velocity = velocity;
   missile->setAngles(angles);
   missile->setOrigin(origin);

   // the player pressed the attack button again, blow the missile
   if((starttime < level.time) && (((Player *)owner.ptr)->Buttons() & BUTTON_ATTACK))
   {
      Event *ev;

      ev = new Event(EV_Missile_Explode);
      ev->AddEntity(world);
      missile->ProcessEvent(ev);
      return;
   }
}

//=================================================================
// guided missile

CLASS_DECLARATION(Projectile, Missile, nullptr);

ResponseDef Missile::Responses[] =
{
   { &EV_Touch, (Response)&Missile::Explode },
   { &EV_Killed, (Response)&Missile::Explode },
   { &EV_Missile_Explode, (Response)&Missile::Explode },
   { &EV_Missile_StartOpen, (Response)&Missile::StartOpen },
   { &EV_Missile_FinishOpen, (Response)&Missile::FinishOpen },
   { nullptr, nullptr }
};

void Missile::Explode (Event *ev)
{
   int         damg;
   Vector      v;
   Entity     *other;
   Vector      norm;
   Entity     *owner;

   other = ev->GetEntity(1);
   assert(other);

   if(other->isSubclassOf<Teleporter>())
      return;

   if(other->entnum == this->owner)
      return;

   owner = G_GetEntity(this->owner);

   stopsound(CHAN_VOICE);

   setSolidType(SOLID_NOT);
   hideModel();
   if(HitSky())
   {
      PostEvent(EV_Remove, 0);
      return;
   }

   setSolidType(SOLID_NOT);
   takedamage = DAMAGE_NO;

   damg = 130 + (int)G_Random(20);

   if(health <= 0) // it was shot down
   {
      // other is actually the one that shot it down, so don't just damage him, hehe
      other = this;
   }
   else // it hit something
   {
      if(other->takedamage)
         other->Damage(this, owner, damg, origin, velocity, level.impact_trace.plane.normal, 0, 0, MOD_MISSILE, -1, -1, 1.0f);

      SpawnBlastDamage(&level.impact_trace, damg, owner);
   }

   v = velocity;
   v.normalize();

   // don't do radius damage to the other, because all the damage
   // was done in the impact
   v = origin - v * 24;
   CreateExplosion(v, damg, 1.0f, true, this, owner, other, MOD_MISSILESPLASH, 0.75);

   PostEvent(EV_Remove, 0.1);
}

void Missile::StartOpen(Event *ev)
{
   RandomAnimate("ready", EV_Missile_FinishOpen);

   // make missile damagable now that some time has passed since firing
   takedamage = DAMAGE_YES;
}

void Missile::FinishOpen(Event *ev)
{
   RandomAnimate("idle", nullptr);
}

void Missile::Setup (Entity *owner, Vector pos, Vector dir)
{
   Event *ev;

   this->owner  = owner->entnum;
   edict->owner = owner->edict;

   setMoveType(MOVETYPE_FLYMISSILE);
   setSolidType(SOLID_BBOX);
   edict->clipmask = MASK_PROJECTILE;

   angles = dir.toAngles();
   angles[PITCH] = -angles[PITCH];
   setAngles(angles);

   speed = MISSILE_SPEED;
   velocity = dir * MISSILE_SPEED;

   // set missile duration
   ev = new Event(EV_Missile_Explode);
   ev->AddEntity(world);
   PostEvent(ev, MISSILE_TIME);

   // will become damagable later
   takedamage = DAMAGE_NO;
   health = 10;

   setModel("missile.def");

   RandomAnimate("ready", nullptr);
   StopAnimating();
   PostEvent(EV_Missile_StartOpen, 0.5);

   edict->s.renderfx |= RF_DLIGHT;
   edict->s.effects  |= EF_ROCKET;
   gravity = 0;
   edict->s.color_r = 0.8;
   edict->s.color_g = 0.4;
   edict->s.color_b = 0;
   edict->s.radius  = 200;

   // give it a heat signature
   edict->s.effects |= EF_WARM;

   // pick a skin
   if(G_Random() < 0.75)
      edict->s.skinnum = 0;
   else
      edict->s.skinnum = 1;

   // setup ambient thrust
   ev = new Event(EV_RandomEntitySound);
   ev->AddString("thrust");
   ProcessEvent(ev);

   setSize({ -6, -6, -6 }, { 6, 6, 6 });
   setOrigin(pos);
   worldorigin.copyTo(edict->s.old_origin);

   // till/if I can fix the (insert swear word here) PVS thing
   missileview = new MissileView();
   missileview->Setup(this, owner->entnum, ((Player *)owner)->ViewMode());
   missileview->setOrigin(origin);
   missileview->setAngles(angles);

   // set player's view to follow the missile
   static_cast<Player *>(owner)->SetViewMode(MISSILE_VIEW, missileview);
   // turn on the missile overlay
   static_cast<Player *>(owner)->MissileOverlayOn();
}

//=================================================================
// guided missile launcher
CLASS_DECLARATION(Weapon, MissileLauncher, "weapon_missilelauncher");

ResponseDef MissileLauncher::Responses[] =
{
   { &EV_Weapon_Shoot,        (Response)&MissileLauncher::Shoot },
   { &EV_Weapon_SecondaryUse, (Response)&MissileLauncher::SecondaryUse },
   { nullptr, nullptr }
};

MissileLauncher::MissileLauncher() : Weapon()
{
   SetModels("missile_w.def", "view_missile.def");
   modelIndex("missile.def");
   modelIndex("sprites/null.spr");
   SetAmmo("Missiles", 1, 0);
   SetRank(71, 0);
   SetType(WEAPON_2HANDED_LO);

   SetMinRange(ROCKET_RADIUS);
   SetProjectileSpeed(MISSILE_SPEED);
}

void MissileLauncher::SecondaryUse(Event *ev)
{
   if(weaponstate != WEAPON_READY)
      return;

   // make sure he has it
   if(!owner->HasItem("RocketLauncher"))
      owner->giveWeapon("RocketLauncher");

   owner->useWeapon("RocketLauncher");
}

void MissileLauncher::Shoot(Event *ev)
{
   Missile *missile;
   Vector pos, dir;

   assert(owner);
   if(!owner)
      return;

   GetMuzzlePosition(&pos, &dir);
   missile = new Missile();
   missile->Setup(owner, pos, dir);

   NextAttack(2);
}

// EOF
