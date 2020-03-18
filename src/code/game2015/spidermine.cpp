//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/spidermine.cpp                      $
// $Revision:: 50                                                             $
//   $Author:: Markd                                                          $
//     $Date:: 6/07/99 2:29p                                                  $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Spider Mine
// 

#include "g_local.h"
#include "spidermine.h"
#include "explosion.h"
#include "player.h"
#include "surface.h"

CLASS_DECLARATION(Projectile, Mine, "Mine");

Event EV_Mine_Explode("mine_explode");
Event EV_Mine_CheckForTargets("mine_targets");
Event EV_Mine_Run("mine_run");

ResponseDef Mine::Responses[] =
{
   { &EV_Touch,				         (Response)&Mine::SlideOrStick },
   { &EV_Killed,				         (Response)&Mine::Explode },
   { &EV_Mine_Explode,	            (Response)&Mine::Explode },
   { &EV_Trigger_Effect,				(Response)&Mine::SlideOrStick },
   { &EV_Mine_CheckForTargets,   	(Response)&Mine::CheckForTargets },
   { &EV_Mine_Run,                	(Response)&Mine::Run },
   { nullptr, nullptr }
};

void Mine::Run(Event *ev)
{
   RandomAnimate("run", nullptr);
}

void Mine::CheckForTargets(Event *ev)
{
   Entity      *ent;
   Event       *event;
   trace_t     trace;

   // Playtest
   return;

   if(detonate)
      return;

   ent = findradius(nullptr, worldorigin, 150);
   while(ent)
   {
      if((ent != this) &&
         (!ent->deadflag) &&
         (ent->entnum != owner) &&
         (ent->takedamage) &&
         (!(ent->flags & FL_NOTARGET)) &&
         (strcmp(ent->getClassname(), "Mine")))
      {
         detonate = true;
         trace = G_Trace(worldorigin, vec_zero, vec_zero, ent->worldorigin, ent, MASK_PROJECTILE, "Mine::CheckForTargets");
         if(trace.fraction != 1.0f)
            detonate = false;
         else
            break;
      }
      ent = findradius(ent, worldorigin, 150);
   }

   if(detonate)
   {
      RandomGlobalSound("spider_arm", 1);
      event = new Event(EV_Mine_Explode);
      PostEvent(event, 0.5f);
   }
   else
   {
      event = new Event(EV_Mine_CheckForTargets);
      PostEvent(event, 0.5f);
   }
}

void Mine::SlideOrStick(Event *ev)
{
   Entity   *other;
   Event    *event;
   Vector   norm;

   if(detonate)
      return;

   other = ev->GetEntity(1);
   assert(other);

   if(other->takedamage)
   {
      setMoveType(MOVETYPE_BOUNCE);
      return;
   }

   // Check to see if we hit the ground, if so then slide along,
   // otherwise stick to the wall.
   if(((level.impact_trace.plane.normal[gravity_axis[gravaxis].z] * gravity_axis[gravaxis].sign) > 0.7) &&
      !sticky)
   {
      setMoveType(MOVETYPE_SLIDE);
      event = new Event(EV_Mine_CheckForTargets);
      PostEvent(event, 2.5);
   }
   else
   {
      // So that we can shoot our own mines
      edict->owner = world->edict;

      CancelEventsOfType(EV_Mine_Run);
      setMoveType(MOVETYPE_NONE);
      RandomAnimate("stick", nullptr);
      norm = level.impact_trace.plane.normal;
      angles = norm.toAngles();
      angles.x = -angles.x;
      setAngles(angles);
      velocity = { 0, 0, 0 };

      // Check to see if we are on the floor and adjust origin
      if(!sticky)
      {
         CheckGround();
         if(groundentity)
            setOrigin(origin + Vector(0, 0, 12));
      }

      setOrigin(origin);
      event = new Event(EV_Mine_CheckForTargets);
      PostEvent(event, 0.7f);
   }
}

void Mine::Explode(Event *ev)
{
   int			damg;
   Vector		v;
   Player      *client;
   Camera      *cam;
   Entity      *owner;
   
   detonate = true;
   takedamage = DAMAGE_NO;

   stopsound(CHAN_VOICE);
   setSolidType(SOLID_NOT);

   if(HitSky())
   {
      PostEvent(EV_Remove, 0);
      return;
   }

   owner = G_GetEntity(this->owner);

   if(!owner)
      owner = world;

   damg = 100;

   if(!deathmatch->value && owner->isClient())
      damg *= 1.5;

   surfaceManager.DamageSurface(&level.impact_trace, damg, owner);

   v = velocity;
   v.normalize();
   v = worldorigin - v * 24;

   RadiusDamage(this, owner, damg, this, MOD_SPIDERSPLASH);

   if(owner->isSubclassOf<Player>())
   {
      client = static_cast<Player *>(owner);
      cam = client->CurrentCamera();

      // If we are in the camera, get out of it
      if(cam == (Camera *)this)
         client->SetCamera(nullptr);
   }

   // Remove the mine from the detonator
   if(detonator)
      detonator->RemoveMine(this);

   RandomAnimate("explode", nullptr);
   PostEvent(EV_Remove, 0.2);
}

void Mine::SetDetonator(Detonator *det)
{
   detonator = det;
}

qboolean Mine::IsOwner(Sentient *sent)
{
   return (sent == G_GetEntity(this->owner));
}

void Mine::Setup(Entity *owner, Vector pos, Vector dir)
{
   Event *ev;
   Vector t[3];
   Vector forward;
   Vector right;
   Vector up;

   this->owner = owner->entnum;

   edict->owner = owner->edict;

   setModel("spidermine.def");

   showModel();
   setMoveType(MOVETYPE_SLIDE);
   setSolidType(SOLID_BBOX);
   edict->clipmask = MASK_PROJECTILE;
   RandomAnimate("open", EV_Mine_Run);

   SetGravityAxis(owner->gravaxis);
   velocity = dir * 600;
   sticky = false;

   const gravityaxis_t &grav = gravity_axis[gravaxis];

   setAngles(Vector(0, dir.toYaw(), 0));

   angles.AngleVectors(&t[0], &t[1], &t[2]);
   forward[grav.x] = t[0][0];
   forward[grav.y] = t[0][1] * grav.sign;
   forward[grav.z] = t[0][2] * grav.sign;
   right[grav.x] = t[1][0];
   right[grav.y] = t[1][1] * grav.sign;
   right[grav.z] = t[1][2] * grav.sign;
   up[grav.x] = t[2][0];
   up[grav.y] = t[2][1] * grav.sign;
   up[grav.z] = t[2][2] * grav.sign;
   VectorsToEulerAngles(forward.vec3(), right.vec3(), up.vec3(), angles.vec3());
   setAngles(angles);

   ev = new Event(EV_Mine_Explode);
   ev->AddEntity(world);
   PostEvent(ev, 180);

   takedamage = DAMAGE_YES;
   health = 150;
   edict->svflags |= (SVF_SHOOTABLE);
   setSize({ -4, -4, -4 }, { 4, 4, 4 });

   setOrigin(pos);
   worldorigin.copyTo(edict->s.old_origin);
   detonate = false;

   fov = 120;

   if(owner->isClient() && owner->client->ps.pmove.pm_flags & PMF_DUCKED)
   {
      setMoveType(MOVETYPE_TOSS);
      sticky = true;
   }

   //### give it a heat signature
   edict->s.effects |= EF_WARM;
   //###
}

CLASS_DECLARATION(Weapon, SpiderMine, nullptr);

ResponseDef SpiderMine::Responses[] =
{
   { &EV_Weapon_Shoot,              (Response)&SpiderMine::Shoot },
   { &EV_Weapon_DoneFiring,         (Response)&SpiderMine::DoneFiring },
   { &EV_Weapon_SecondaryUse,       (Response)&SpiderMine::ChangeToDetonator },
   { nullptr, nullptr }
};

SpiderMine::SpiderMine() : Weapon()
{
#ifdef SIN_DEMO
   PostEvent(EV_Remove, 0);
   return;
#endif
   SetModels("mines.def", "view_mines.def");
   SetAmmo("SpiderMines", 1, 0);
   SetRank(60, 5);
   SetType(WEAPON_1HANDED);
   modelIndex("spidermine.def");
   modelIndex("mine.def");

   currentMine = 0;
}

qboolean SpiderMine::IsDroppable()
{
   return false;
}

void SpiderMine::SetOwner(Sentient *sent)
{
   Detonator   *detonator;
   int         num;

   assert(sent);
   if(!sent)
   {
      // return to avoid any buggy behaviour
      return;
   }

   Item::SetOwner(sent);

   setOrigin(vec_zero);
   setAngles(vec_zero);

   if(!viewmodel.length())
   {
      error("setOwner", "Weapon without viewmodel set");
   }

   setModel(viewmodel);

   // Give the owner a detonator and setup the link

   detonator = (Detonator *)sent->FindItem("Detonator");

   if(!detonator)
   {
      detonator = (Detonator *)sent->giveWeapon("Detonator");
   }

   SetDetonator(detonator);

   // Check the world for any spidermines in existence and 
   // if the player owns them, add them to the minelist.
   num = 0;
   while((num = G_FindClass(num, "Mine")) != 0)
   {
      Mine *mine = static_cast<Mine *>(G_GetEntity(num));

      if(mine->IsOwner(sent))
      {
         detonator->AddMine(mine);
         mine->SetDetonator(detonator);
      }
   }
}

void SpiderMine::DoneFiring(Event *ev)
{
   assert(owner);

   weaponstate = WEAPON_HOLSTERED;
   DetachGun();
   StopAnimating();

   if(owner)
   {
      owner->SetCurrentWeapon(detonator);
   }
}

void SpiderMine::ChangeToDetonator(Event *ev)
{
   if(owner && detonator->mineList.NumObjects())
      owner->ChangeWeapon(detonator);
}

qboolean SpiderMine::ReadyToUse()
{
   Event *ev;

   if(HasAmmo())
   {
      return true;
   }
   else
   {
      if(owner->isClient())
      {
         // Check to see if detonator has mines active
         if(detonator && detonator->mineList.NumObjects())
         {
            ev = new Event("use");
            ev->AddString("Detonator");
            owner->PostEvent(ev, 0);
         }
      }
      return false;
   }
}

void SpiderMine::SetDetonator(Detonator *det)
{
   detonator = det;
}

void SpiderMine::Shoot(Event *ev)
{
   Vector	pos;
   Vector	dir;
   Mine     *mine;
   Event    *event;

   assert(owner);
   if(!owner)
   {
      return;
   }

   GetMuzzlePosition(&pos, &dir);

   if(currentMine)
   {
      event = new Event(EV_Mine_Explode);
      currentMine->ProcessEvent(event);
   }
   else
   {
      mine = new Mine();
      mine->Setup(owner, pos, dir);
      mine->SetDetonator(detonator);
      detonator->AddMine(mine);
   }

   NextAttack(1.8f);
}

CLASS_DECLARATION(Weapon, Detonator, nullptr);

ResponseDef Detonator::Responses[] =
{
   { &EV_Weapon_Shoot,              (Response)&Detonator::Shoot },
   { &EV_Weapon_DoneFiring,         (Response)&Detonator::DoneFiring },
   { &EV_Weapon_SecondaryUse,       (Response)&Detonator::CycleCamera },
   { nullptr, nullptr }
};

Detonator::Detonator() : Weapon()
{
#ifdef SIN_DEMO
   PostEvent(EV_Remove, 0);
   return;
#endif
   SetModels(nullptr, "view_detonator.def");
   SetType(WEAPON_1HANDED);
   currentMine = 0;
}

void Detonator::RemoveMine(Mine *mine)
{
   mineList.RemoveObject(MinePtr(mine));
}

void Detonator::AddMine(Mine *mine)
{
   mineList.AddObject(MinePtr(mine));
}

void Detonator::CycleCamera(Event *ev)
{
   Player      *client;
   int         numMines;
   Mine        *mine;

   assert(owner);
   if(!owner)
   {
      return;
   }

   numMines = mineList.NumObjects();

   if(!numMines)
      return;

   currentMine++;

   mine = mineList.ObjectAt((currentMine % numMines) + 1);
   if(!mine)
   {
      mineList.RemoveObjectAt((currentMine % numMines) + 1);
      return;
   }

   client = (Player *)(Entity *)owner;
   if(client->edict->areanum == mine->edict->areanum)
      client->SetCamera(mine);
   else
   {
      gi.cprintf(client->edict, PRINT_HIGH, "Spidermine %d out of range\n", (currentMine % numMines) + 1);
   }
}

void Detonator::DoneFiring(Event *ev)
{
   Weapon *weapon;

   weaponstate = WEAPON_READY;

   if(owner)
   {
      owner->ProcessEvent(EV_Sentient_WeaponDoneFiring);
   }

   // Change back to spidermines if there is ammo, otherwise change weapons.

   weapon = static_cast<Weapon *>(owner->FindItem("SpiderMine"));

   if(!weapon || !weapon->HasAmmo())
   {
      weapon = owner->BestWeapon();
   }

   owner->ChangeWeapon(weapon);
}

void Detonator::Shoot(Event *ev)
{
   int      num, i;
   Player   *player;
   Entity   *cameraMine;
   Event    *event;

   // If the owner is in a camera, the only detonate that mine, 
   // otherwise detonate them all

   NextAttack(0.1);

   if(owner->isClient())
   {
      player = static_cast<Player *>(static_cast<Entity *>(owner));

      cameraMine = player->CurrentCamera();

      if(cameraMine && cameraMine->isSubclassOf<Mine>())
      {
         event = new Event(EV_Mine_Explode);
         cameraMine->PostEvent(event, 0);
         return;
      }
   }

   // Go through all the mines and detonate them 

   num = mineList.NumObjects();

   for(i = num; i >= 1; i--)
   {
      Event *explodeEvent;
      Mine  *mine;

      mine = mineList.ObjectAt(i);
      explodeEvent = new Event(EV_Mine_Explode);
      mine->PostEvent(explodeEvent, 0);
   }
}

qboolean Detonator::IsDroppable()
{
   return false;
}

qboolean Detonator::AutoChange()
{
   return false;
}

// EOF

