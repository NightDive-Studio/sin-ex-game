//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/weapon.cpp                       $
// $Revision:: 147                                                            $
//   $Author:: Markd                                                          $
//     $Date:: 7/24/99 5:45p                                                  $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Source file for Weapon class.  The weapon class is the base class for
// all weapons in Sin.  Any entity created from a class derived from the weapon
// class will be usable by any Sentient (players and monsters) as a weapon.
// 

#include "g_local.h"
#include "entity.h"
#include "item.h"
#include "weapon.h"
#include "scriptmaster.h"
#include "sentient.h"
#include "misc.h"
#include "specialfx.h"
#include "chaingun.h"
#include "assaultrifle.h"
//###
#include "player.h" // added for guided missile
#include "rocketlauncher.h"
#include "guidedmissile.h"
#include "informergun.h"
#include "magnum.h"
#include "dualmagnum.h"
#include "actor.h"
//###

#ifdef SIN_ARCADE
static ScriptVariablePtr sv_infinitebullets;
static ScriptVariablePtr sv_infiniterockets;
static ScriptVariablePtr sv_infiniteplasma;
static ScriptVariablePtr sv_infinitespears;
static ScriptVariablePtr sv_infinitesniper;
#endif

CLASS_DECLARATION(Item, Weapon, nullptr);

Event EV_Weapon_Shoot("shoot");
Event EV_Weapon_FinishAttack("attack_finished");
Event EV_Weapon_DoneLowering("putaway");
Event EV_Weapon_DoneRaising("ready");
Event EV_Weapon_DoneFiring("donefiring");
Event EV_Weapon_Idle("idle");
Event EV_Weapon_MuzzleFlash("muzzleflash");
Event EV_Weapon_SecondaryUse("secondaryuse");
Event EV_Weapon_DoneReloading("donereloading");
Event EV_Weapon_SetAmmoClipSize("ammoclipsize");
Event EV_Weapon_ProcessModelCommands("process_mdl_cmds");
Event EV_Weapon_SetMaxRange("maxrange");
Event EV_Weapon_SetMinRange("minrange");
Event EV_Weapon_SetProjectileSpeed("projectilespeed");
Event EV_Weapon_SetKick("kick");
Event EV_Weapon_PrimaryMode("primarymode");
Event EV_Weapon_SecondaryMode("secondarymode");
Event EV_Weapon_ActionIncrement("actionincrement");
Event EV_Weapon_PutAwayAndRaise("putawaythenraise");
Event EV_Weapon_Raise("raise");
Event EV_Weapon_NotDroppable("notdroppable");
Event EV_Weapon_SetAimAnim("setaimanim");
//### added ability for weapons to add view jitter to their owners
Event EV_Weapon_SetAngleJitter("anglejitter");
Event EV_Weapon_SetOffsetJitter("offsetjitter");
//###

ResponseDef Weapon::Responses[] =
{
   { &EV_Item_Pickup,                  (Response)&Weapon::PickupWeapon },
   { &EV_Weapon_FinishAttack,          (Response)&Weapon::FinishAttack },
   { &EV_Weapon_DoneLowering,          (Response)&Weapon::DoneLowering },
   { &EV_Weapon_DoneRaising,           (Response)&Weapon::DoneRaising },
   { &EV_Weapon_DoneFiring,            (Response)&Weapon::DoneFiring },
   { &EV_Weapon_Idle,                  (Response)&Weapon::Idle },
   { &EV_Weapon_MuzzleFlash,           (Response)&Weapon::EventMuzzleFlash },
   { &EV_WeaponSound,                  (Response)&Weapon::WeaponSound },
   { &EV_Weapon_DoneReloading,         (Response)&Weapon::DoneReloading },
   { &EV_Weapon_SetAmmoClipSize,       (Response)&Weapon::SetAmmoClipSize },
   { &EV_Weapon_ProcessModelCommands,  (Response)&Weapon::ProcessWeaponCommandsEvent },
   { &EV_Weapon_SetMaxRange,           (Response)&Weapon::SetMaxRangeEvent },
   { &EV_Weapon_SetMinRange,           (Response)&Weapon::SetMinRangeEvent },
   { &EV_Weapon_SetProjectileSpeed,    (Response)&Weapon::SetProjectileSpeedEvent },
   { &EV_Weapon_SetKick,               (Response)&Weapon::SetKick },
   { &EV_Weapon_SecondaryUse,          (Response)&Weapon::SecondaryUse },
   { &EV_Weapon_PrimaryMode,           (Response)&Weapon::PrimaryMode },
   { &EV_Weapon_SecondaryMode,         (Response)&Weapon::SecondaryMode },
   { &EV_Weapon_ActionIncrement,       (Response)&Weapon::SetActionLevelIncrement },
   { &EV_Weapon_PutAwayAndRaise,       (Response)&Weapon::PutAwayAndRaise },
   { &EV_Weapon_Raise,                 (Response)&Weapon::Raise },
   { &EV_Weapon_NotDroppable,          (Response)&Weapon::NotDroppableEvent },
   { &EV_Weapon_SetAimAnim,            (Response)&Weapon::SetAimAnim },
   //### added ability for weapons to add view jitter to their owners
   { &EV_Weapon_SetAngleJitter,        (Response)&Weapon::AngleJitterEvent },
   { &EV_Weapon_SetOffsetJitter,       (Response)&Weapon::OffsetJitterEvent },
   //###

   { nullptr, nullptr }
};

static int numBulletHoles = 0;
static Queue queueBulletHoles;

void ResetBulletHoles()
{
   numBulletHoles = 0;
   queueBulletHoles.Clear();
}

Weapon::Weapon() : Item()
{
   owner                  = nullptr;
   ammo_in_clip           = G_GetIntArg("ammo_in_clip", 0);
   last_attack_time       = level.time - 10;
   weaponmode             = (weaponmode_t)G_GetIntArg("weaponmode", PRIMARY);
   silenced               = G_GetIntArg("silenced", false);

   // handles most cases
   weapontype = WEAPON_2HANDED_HI;

   // start off unattached
   attached = false;

   // maximum effective firing distance (for AI)
   maxrange = 8192 * 2; // FIXME grr... magic number...

   // minimum safe firing distance (for AI)
   minrange = 0;

   // speed of the projectile (0 == infinite speed)
   projectilespeed = 0;

   // default action_level_increment
   action_level_increment = 2;

   // default weapons don't have alt fire
   alternate_fire = false;
}

Weapon::~Weapon()
{
   DetachGun();
}

void Weapon::CreateSpawnArgs()
{
   Item::CreateSpawnArgs();
   G_SetIntArg("ammo_in_clip", ammo_in_clip);
   G_SetIntArg("silenced", silenced);
   G_SetIntArg("weaponmode", weaponmode);
}

int Weapon::Rank()
{
   return rank;
}

int Weapon::Order()
{
   return order;
}

void Weapon::SetRank(int order, int rank)
{
   this->order = order;
   this->rank = rank;
}

void Weapon::SetType(weapontype_t type)
{
   weapontype = type;
}

weapontype_t Weapon::GetType()
{
   return weapontype;
}

void Weapon::SetAmmo(const char *type, int amount, int startamount)
{
   if(type)
   {
      ammotype = type;
      primary_ammo_type = type;
   }
   else
   {
      ammotype = "";
      primary_ammo_type = "";
   }

   ammorequired = amount;
   startammo = startamount;
}

void Weapon::SetSecondaryAmmo(const char *type, int amount, int startamount)
{
   if(type)
   {
      secondary_ammo_type = type;
   }
   else
   {
      secondary_ammo_type = "";
   }

   secondary_ammorequired = amount;
}

void Weapon::SetAmmoAmount(int amount)
{
   if(ammo_clip_size)
      ammo_in_clip = amount;
}

void Weapon::UseAmmo(int amount)
{
   if(ammo_clip_size)
   {
      ammo_in_clip -= amount;
      if(ammo_in_clip < 0)
      {
         warning("UseAmmo", "Used more ammo than in clip.\n");
         ammo_in_clip = 0;
      }
   }
   else
   {
      Ammo * ammo;
      assert(owner);
      if(owner && owner->isClient() && !UnlimitedAmmo())
      {
         if(ammotype.length())
         {
            ammo = static_cast<Ammo *>(owner->FindItem(ammotype.c_str()));

            //### it should use the amount of ammo passed in
            if(!ammo || !ammo->Use(amount))
            {
               SetAmmoAmount(0);
               return;
            }
            //###

            SetAmmoAmount(ammo->Amount());
         }
      }
   }
}

Vector Weapon::MuzzleOffset()
{
   vec3_t	trans[3];
   vec3_t   orient;
   int		groupindex;
   int		tri_num;
   Vector	offset = vec_zero;
   int      useanim;
   int      useframe;

   // get the bone information
   if((!edict->s.gunmodelindex) || (owner && owner->isClient()))
   {
      if(gi.GetBoneInfo(edict->s.modelindex, "barrel", &groupindex, &tri_num, orient))
      {
         if(aimanim == -1)
         {
            useanim = edict->s.anim;
            useframe = edict->s.frame;
         }
         else
         {
            useanim = aimanim;
            useframe = aimframe;
         }
         if(gi.GetBoneTransform(edict->s.modelindex, groupindex, tri_num, orient, useanim, useframe,
                                edict->s.scale, trans, offset.vec3()))
         {
            //
            // we scale the pos by 0.3 because of RF_DEPTHHACK 
            //
            offset *= 0.3f;
            if(owner && owner->isClient())
            {
               switch(owner->client->pers.hand)
               {
               case LEFT_HANDED:
                  offset[1] *= -1.0f;
                  break;
               case CENTER_HANDED:
                  offset[1] = 0;
                  break;
               }
            }
         }
      }
   }
   //
   // if it is a non-client, than get the information from the world model of the gun
   //
   else if(gi.GetBoneInfo(edict->s.gunmodelindex, "barrel", &groupindex, &tri_num, orient))
   {
      gi.GetBoneTransform(edict->s.gunmodelindex, groupindex, tri_num, orient, 0, 0,
                          edict->s.scale, trans, offset.vec3());
   }
   // Gun doesn't have a barrel, so search the owner for a barrel bone
   else if(owner && 
           gi.GetBoneInfo(owner->edict->s.modelindex, "barrel", &groupindex, &tri_num, orient))
   {
      gi.GetBoneTransform(
         owner->edict->s.modelindex,
         groupindex,
         tri_num,
         orient,
         owner->edict->s.anim,
         owner->edict->s.frame,
         owner->edict->s.scale,
         trans,
         offset.vec3());
   }

   return offset;
}

void Weapon::GetMuzzlePosition(Vector *position, Vector *forward, Vector *right, Vector *up)
{
   Vector	offset;
   Vector	f, r, u;
   Vector	pos;
   Vector	end;
   Vector   dir;
   Vector	gunpos;
   trace_t	trace;

   assert(owner);

   // technically, we should never not have an owner when firing.
   if(!owner)
   {
      return;
   }

   //
   // get the position of the owners gun bone
   //
   gunpos = owner->GunPosition();
   pos = gunpos;

   owner->GetGunOrientation(pos, &f, &r, &u);

   // get the bone information
   offset = MuzzleOffset();

   pos += f * offset[0];
   pos -= r * offset[1];
   pos += u * offset[2];

   // prevent the creature from firing through walls
   trace = G_Trace(gunpos, vec_zero, vec_zero, pos, owner, MASK_PROJECTILE, "Weapon::GetMuzzlePosition");
   if((trace.fraction < 1) || (trace.startsolid) || (trace.allsolid))
   {
      pos = gunpos;
      pos -= r * offset[1];
      pos += u * offset[2];
   }

   //
   // calculate where this projectile is going to hit
   //
   end = gunpos + f * 2048;
   trace = G_FullTrace(gunpos, vec_zero, vec_zero, end, 10, owner, MASK_SHOT, "Weapon::GetMuzzlePosition");
   dir = trace.endpos - pos;
   dir.normalize();
   if(dir*f < 0.707)
      dir = f;

   if(position)
   {
      *position = pos;
   }

   if(forward)
   {
      *forward = dir;
   }

   if(right)
   {
      *right = r;
   }

   if(up)
   {
      *up = u;
   }
}

void Weapon::SetAmmoClipSize(Event * ev)
{
   ammo_clip_size = ev->GetInteger(1);
}

void Weapon::SetModels(const char *world, const char *view)
{
   Event *ev;
   assert(view);

   viewmodel = view;
   modelIndex(view);

   if(world)
   {
      worldmodel = world;
      modelIndex(world);
   }
   else
   {
      worldmodel = "";
   }

   if(owner)
   {
      setModel(viewmodel);
   }
   else if(worldmodel.length())
   {
      setModel(worldmodel);
   }
   else
   {
      setModel(viewmodel);
   }

   if(worldmodel.length())
   {
      ev = new Event(EV_Weapon_ProcessModelCommands);
      ev->AddInteger(modelIndex(worldmodel.c_str()));
      PostEvent(ev, 0);
   }
   ev = new Event(EV_Weapon_ProcessModelCommands);
   ev->AddInteger(modelIndex(viewmodel.c_str()));
   PostEvent(ev, 0);
}

void Weapon::SetAimAnim(Event *ev)
{
   str anim;

   anim = ev->GetString(1);
   aimanim = gi.Anim_NumForName(edict->s.modelindex, anim.c_str());
   aimframe = ev->GetInteger(2);
}

void Weapon::SetOwner(Sentient *ent)
{
   assert(ent);
   if(!ent)
   {
      // return to avoid any buggy behaviour
      return;
   }

   Item::SetOwner(ent);

   setOrigin(vec_zero);
   setAngles(vec_zero);

   if(!viewmodel.length())
   {
      error("setOwner", "Weapon without viewmodel set");
   }

   setModel(viewmodel);

   if(ent->isClient() && ammotype.length() && startammo && !G_GetSpawnArg("savegame"))
   {
      ent->giveItem(ammotype.c_str(), startammo);
   }
}

void Weapon::TakeAllAmmo()
{
   Ammo *ammo;

   if(owner)
   {
      if(ammotype.length())
      {
         ammo = static_cast<Ammo *>(owner->FindItem(ammotype.c_str()));
         if(ammo)
         {
            owner->takeItem(ammotype.c_str(), ammo->Amount());
         }
      }
   }
}

int Weapon::AmmoAvailable()
{
   Ammo *ammo;

   if(owner)
   {
      if(ammotype.length())
      {
         ammo = static_cast<Ammo *>(owner->FindItem(ammotype.c_str()));
         if(ammo)
         {
            return ammo->Amount();
         }
      }
   }

   return 0;
}

qboolean Weapon::UnlimitedAmmo()
{
   if(!owner)
   {
      return false;
   }

   if(!owner->isClient() || (owner->flags & FL_GODMODE) || DM_FLAG(DF_INFINITE_AMMO))
   {
      return true;
   }

#ifdef SIN_ARCADE
   if(!sv_infinitebullets || !sv_infiniterockets || !sv_infiniteplasma || !sv_infinitespears || !sv_infinitesniper)
   {
      sv_infinitebullets = gameVars.CreateVariable("infinitebullets", 0);
      sv_infiniterockets = gameVars.CreateVariable("infiniterockets", 0);
      sv_infiniteplasma  = gameVars.CreateVariable("infiniteplasma", 0);
      sv_infinitespears  = gameVars.CreateVariable("infinitespears", 0);
      sv_infinitesniper  = gameVars.CreateVariable("infinitesniper", 0);
   }

   if(sv_infinitebullets->intValue() && ((!Q_strncasecmp(ammotype.c_str(), "bullet", 6) && (ammotype != "BulletPulse") &&
      (ammotype != "BulletSniper")) || (ammotype == "ShotgunClip")))
   {
      return true;
   }

   if(sv_infiniterockets->intValue() && (ammotype == "Rockets"))
   {
      return true;
   }

   if(sv_infiniteplasma->intValue() && (ammotype == "BulletPulse"))
   {
      return true;
   }

   if(sv_infinitesniper->intValue() && (ammotype == "BulletSniper"))
   {
      return true;
   }

   if(sv_infinitespears->intValue() && (ammotype == "Spears"))
   {
      return true;
   }
#endif

   return false;
}

qboolean Weapon::HasAmmo()
{
   if(!owner)
   {
      return false;
   }

   if(UnlimitedAmmo())
   {
      return true;
   }

   if(weaponmode == PRIMARY)
   {
      if((ammo_clip_size && ammo_in_clip >= ammorequired) || AmmoAvailable() >= ammorequired)
      {
         return true;
      }
   }
   else
   {
      if((ammo_clip_size && ammo_in_clip >= secondary_ammorequired) || AmmoAvailable() >= secondary_ammorequired)
      {
         return true;
      }
   }

   return false;
}

qboolean Weapon::HasAmmoInClip()
{
   if(ammo_clip_size)
   {
      if(weaponmode == PRIMARY)
      {
         if(ammo_in_clip >= ammorequired)
         {
            return true;
         }
      }
      else
      {
         if(ammo_in_clip >= secondary_ammorequired)
         {
            return true;
         }
      }
   }
   else
   {
      return HasAmmo();
   }

   return false;
}

void Weapon::ForceState(weaponstate_t state)
{
   weaponstate = state;
}

qboolean Weapon::AttackDone()
{
   //### added check for player controlling a missile
   if(owner->isSubclassOf<Player>())
      return (attack_finished <= level.time) && (static_cast<Player *>(owner.ptr)->ViewMode() != MISSILE_VIEW);
   else
      return (attack_finished <= level.time);
   //###
}

qboolean Weapon::ReadyToFire()
{
   return (weaponstate == WEAPON_READY) && AttackDone();
}

qboolean Weapon::ReadyToChange()
{
   //###
   // can never change from the informer gun
   if((deathmatch->value == DEATHMATCH_MFD || deathmatch->value == DEATHMATCH_MOB) &&
      this->isSubclassOf<InformerGun>())
   {
      return false;
   }
   //###
   return (weaponstate == WEAPON_READY);
}

qboolean Weapon::ReadyToUse()
{
   return true;
}

qboolean Weapon::ChangingWeapons()
{
   return (weaponstate == WEAPON_LOWERING) || (weaponstate == WEAPON_RAISING);
}

qboolean Weapon::WeaponRaising()
{
   return (weaponstate == WEAPON_RAISING);
}

qboolean Weapon::WeaponPuttingAway()
{
   return (weaponstate == WEAPON_LOWERING);
}

qboolean Weapon::Reloading()
{
   return (weaponstate == WEAPON_RELOADING);
}

void Weapon::PutAway()
{
   if(weaponstate != WEAPON_READY)
   {
      return;
   }

   weaponstate = WEAPON_LOWERING;

   if(!HasAnim("putaway") || (deathmatch->value && ((int)dmflags->value & DF_FAST_WEAPONS)))
   {
      ProcessEvent(EV_Weapon_DoneLowering);
      return;
   }

   if(dualmode)
   {
      if(weaponmode == PRIMARY)
         RandomAnimate("primaryputaway", EV_Weapon_DoneLowering);
      else
         RandomAnimate("secondaryputaway", EV_Weapon_DoneLowering);
   }
   else if((owner) && (owner->flags & FL_SILENCER) && (silenced))
      RandomAnimate("silputaway", EV_Weapon_DoneLowering);
   else
      RandomAnimate("putaway", EV_Weapon_DoneLowering);

   //weaponmode = PRIMARY;
}

void Weapon::ReadyWeapon()
{
   str   animname;

   if(weaponstate != WEAPON_HOLSTERED)
   {
      return;
   }

   weaponstate = WEAPON_RAISING;

   AttachGun();

   if((owner) && (owner->flags & FL_SILENCER) && (silenced))
      animname = "silready";
   else
   {
      if(weaponmode == SECONDARY)
         animname = "secondaryready";
      else
         animname = "ready";

   }

   if(!HasAnim(animname.c_str()) || (deathmatch->value && ((int)dmflags->value & DF_FAST_WEAPONS)))
   {
      ProcessEvent(EV_Weapon_DoneRaising);
      return;
   }

   RandomAnimate(animname.c_str(), EV_Weapon_DoneRaising);
}

void Weapon::DetachFromOwner()
{
   StopAnimating();
   DetachGun();
   weaponstate = WEAPON_HOLSTERED;
}

void Weapon::AttachToOwner()
{
   AttachGun();
   ForceIdle();
}

qboolean Weapon::Drop()
{
   float radius;
   Vector temp;

   if(!owner)
   {
      return false;
   }

   if(!IsDroppable())
   {
      return false;
   }

   DetachGun();

   const gravityaxis_t &grav = gravity_axis[gravaxis];

   temp[grav.z] = 40 * grav.sign;
   if(owner)
   {
      setOrigin(owner->worldorigin + temp);
   }
   else
   {
      setOrigin(worldorigin + temp);
   }
   setModel(worldmodel);

   // hack to fix the bounds when the gun is dropped

   //
   // once dropped reset the rotated bounds
   //
   flags |= FL_ROTATEDBOUNDS;

   gi.CalculateBounds(edict->s.modelindex, edict->s.scale, mins.vec3(), maxs.vec3());
   radius = (mins - maxs).length() * 0.25f;
   mins.x = mins.y = -radius;
   maxs.x = maxs.y = radius;
   setSize(mins, maxs);

   StopAnimating();
   edict->s.frame = 0;
   edict->s.anim = 0;

   // drop the weapon
   PlaceItem();
   if(owner)
   {
      temp[grav.x] = G_CRandom(50);
      temp[grav.y] = G_CRandom(50);
      temp[grav.z] = 100 * grav.sign;
      velocity = owner->velocity * 0.5 + temp;
      setAngles(owner->angles);
   }

   avelocity = Vector(0, G_CRandom(360), 0);

   if(owner && owner->isClient())
   {
      spawnflags |= DROPPED_PLAYER_ITEM;
      if(ammo_clip_size)
         startammo = ammo_in_clip;
      else
         startammo = 0;

      // If owner is dead, put all his ammo of that type in the gun.
      if(owner->deadflag)
      {
         startammo = AmmoAvailable();
      }
   }
   else
   {
      spawnflags |= DROPPED_ITEM;
      if(ammo_clip_size && ammo_in_clip)
         startammo = ammo_in_clip;
      else
         startammo >>= 2;

      if(startammo == 0)
      {
         startammo = 1;
      }
   }

   // Wait some time before the last owner can pickup this weapon
   last_owner = owner;
   last_owner_trigger_time = level.time + 2.5f;

   // Cancel reloading events
   CancelEventsOfType(EV_Weapon_DoneReloading);

   // Remove this from the owner's item list
   if(owner)
   {
      owner->RemoveItem(this);
   }

   owner = nullptr;

   // Fade out dropped weapons, to keep down the clutter
   PostEvent(EV_FadeOut, 30);
   return true;
}

void Weapon::Fire()
{
   qboolean skipframefix;

   if(!ReadyToFire())
   {
      return;
   }

   if(!HasAmmoInClip())
   {
      CheckReload();
      return;
   }

   if(weaponmode == PRIMARY)
      UseAmmo(ammorequired);
   else
      UseAmmo(secondary_ammorequired);

   weaponstate = WEAPON_FIRING;

   CancelEventsOfType(EV_Weapon_DoneFiring);
   // this is just a precaution that we can re-trigger
   NextAttack(5);

   skipframefix = false;
   if(owner && owner->isClient() && !isSubclassOf<ChainGun>() && !isSubclassOf<AssaultRifle>())
   {
      skipframefix = true;
      StopAnimating();
   }

   if(dualmode)
   {
      if(weaponmode == PRIMARY)
      {
         RandomAnimate("primaryfire", EV_Weapon_DoneFiring);
      }
      else
      {
         RandomAnimate("secondaryfire", EV_Weapon_DoneFiring);
      }
   }
   else
   {
      if((owner) && (owner->flags & FL_SILENCER) && (silenced))
         RandomAnimate("silfire", EV_Weapon_DoneFiring);
      else
         RandomAnimate("fire", EV_Weapon_DoneFiring);
   }

   if(skipframefix)
   {
      last_animation_time = (level.framenum + 1) * FRAMETIME;
   }

   last_attack_time = level.time;
}

//**********************************************************************************/
//
// Non-public Weapon functions
//
//**********************************************************************************/

void Weapon::DetachGun()
{
   if(attached)
   {
      RandomGlobalSound("null_sound", 1, CHAN_WEAPONIDLE);
      attached = false;
      detach();
      hideModel();
      edict->s.gunmodelindex = 0;
      edict->s.gunanim = 0;
      edict->s.gunframe = 0;
      edict->s.effects &= ~EF_SMOOTHANGLES;
      edict->s.effects &= ~EF_WARM; //### turn off its heat signature
   }
}

//
// attach and detach the gun from the owner
//

void Weapon::AttachGun()
{
   int groupindex;
   int tri_num;
   Vector orient;

   if(!owner)
   {
      return;
   }

   if(attached)
      DetachGun();

   if(gi.GetBoneInfo(owner->edict->s.modelindex, "gun", &groupindex, &tri_num, orient.vec3()))
   {
      attached = true;
      attach(owner->entnum, groupindex, tri_num, orient);
      showModel();
      setOrigin(vec_zero);
      edict->s.gunmodelindex = modelIndex(worldmodel.c_str());
      if(edict->s.gunmodelindex)
      {
         edict->s.gunanim = gi.Anim_Random(edict->s.gunmodelindex, "idle");
         if(edict->s.gunanim < 0)
            edict->s.gunanim = 0;
         edict->s.gunframe = 0;
      }
      else
      {
         edict->s.gunanim = 0;
         edict->s.gunframe = 0;
      }
      edict->s.effects |= EF_SMOOTHANGLES;
      edict->s.effects |= EF_WARM; //### enable heat signature
   }
   else
   {
      gi.dprintf("attach failed\n");
   }
}

//### changed so that weapon does not
// change if player already has the weapon
// also doesn't change the player's weapon if on a hoverbike.
// Added support for togglable auto weapon switching
void Weapon::PickupWeapon(Event *ev)
{
   Sentient *sen;
   Entity   *other;
   Weapon   *weapon;
   Weapon   *current;
   qboolean  hasweapon;
   qboolean  giveammo;
   Ammo     *ammo;

   other = ev->GetEntity(1);

   assert(other);

   if(!other->isSubclassOf<Sentient>())
   {
      return;
   }

   sen = static_cast<Sentient *>(other);

   // If this is the last owner, check to see if he can pick it up
   if((sen == last_owner) && (level.time < last_owner_trigger_time))
   {
      return;
   }

   hasweapon = sen->HasItem(getClassname());
   giveammo = (sen->isClient() && ammotype.length() && startammo);

   // if he already has the weapon, don't pick it up if he doesn't need the ammo
   if(hasweapon)
   {
      //### added for dual magnum
      if(isSubclassOf<Magnum>() && !sen->HasItem("DualMagnum"))
      {
         // give a dual magnum
         weapon = sen->giveWeapon("DualMagnum");

         current = sen->CurrentWeapon();
         if(!hasweapon && current && (current != weapon) && (current->AutoChange()) &&
            ((current->Rank() < weapon->Rank()) || (!current->HasAmmo() && weapon->HasAmmo())))
         {
            if(sen->IsOnBike() && sen->isClient())
            {
               if(sen->autoweaponswitch || Q_strcasecmp(current->getClassname(), "Magnum"))
               {
                  static_cast<Player *>(sen)->GetHoverbike()->oldweapon = weapon;
               }
            }
            else if(sen->autoweaponswitch)
            {
               sen->ChangeWeapon(weapon);
            }
            else
            {
               sen->NonAutoChangeWeapon(weapon);
            }
         }
      }
      //###

      if(!giveammo)
         return;

      // check if he needs the ammo
      ammo = static_cast<Ammo *>(sen->FindItem(ammotype.c_str()));
      if(ammo && (ammo->Amount() >= ammo->MaxAmount()))
      {
         // doesn't need the ammo or the weapon, so return.
         return;
      }
   }

   weapon = static_cast<Weapon *>(ItemPickup(other));
   if(!weapon)
   {
      // Item Pickup failed, so don't give ammo either.
      return;
   }

   //
   // once picked up we don't want rotated bounds
   //
   flags &= ~FL_ROTATEDBOUNDS;

   //FIXME
   // Sentient should probably handle this when itempickup is called
   // Check if we should switch his weapon
   current = sen->CurrentWeapon();
   //###if(!DM_FLAG(DF_NO_WEAPON_CHANGE))
   //{
   if(!hasweapon && current && (current != weapon) && (current->AutoChange()) &&
      ((current->Rank() < weapon->Rank()) || (!current->HasAmmo() && weapon->HasAmmo())))
   {
      //### added hoverbike condition stuff
      if(sen->IsOnBike() && sen->isClient()) // haleyjd 20170605: added isClient() check
      {
         if(sen->autoweaponswitch)
         {
            static_cast<Player *>(sen)->GetHoverbike()->oldweapon = weapon;
         }
      }
      else if(sen->autoweaponswitch)
      {
         sen->ChangeWeapon(weapon);
      }
      else
      {
         sen->NonAutoChangeWeapon(weapon);
      }
      //###
   }
   //}###

   //### added give stuff for rocket & missile launchers in deathmatch
   if(deathmatch->value)
   {
      if(isSubclassOf<RocketLauncher>())
      {
         sen->giveWeapon("MissileLauncher");
      }
      else if(isSubclassOf<MissileLauncher>())
      {
         sen->giveWeapon("RocketLauncher");
      }
   }
   //###

   // check if we should give him ammo
   if(giveammo)
      sen->giveItem(ammotype.c_str(), startammo);
}

void Weapon::ForceIdle()
{
   weaponstate = WEAPON_READY;

   if(dualmode)
   {
      if(weaponmode == PRIMARY)
         RandomAnimate("primaryidle", EV_Weapon_Idle);
      else
         RandomAnimate("secondaryidle", EV_Weapon_Idle);
   }
   else
   {
      if(owner && (owner->flags & FL_SILENCER) && (silenced))
         RandomAnimate("silidle", EV_Weapon_Idle);
      else
         RandomAnimate("idle", EV_Weapon_Idle);
   }
}

void Weapon::DoneLowering(Event *ev)
{
   weaponstate = WEAPON_HOLSTERED;

   DetachGun();

   if(owner)
   {
      owner->ProcessEvent(EV_Sentient_WeaponPutAway);
   }
   StopAnimating();
}

void Weapon::DoneRaising(Event *ev)
{
   weaponstate = WEAPON_READY;

   if(!ForceReload())
   {
      ForceIdle();
   }

   if(!owner)
   {
      PostEvent(EV_Remove, 0);
      return;
   }

   // Set the ammo type
   if(weaponmode == PRIMARY)
   {
      ammotype = primary_ammo_type;
   }
   else if(weaponmode == SECONDARY)
   {
      ammotype = secondary_ammo_type;
   }

   if(owner)
      owner->ProcessEvent(EV_Sentient_WeaponReady);
}

void Weapon::DoneFiring(Event *ev)
{
   if(!CheckReload())
   {
      ForceIdle();
   }

   if(owner)
   {
      owner->ProcessEvent(EV_Sentient_WeaponDoneFiring);
   }
}

void Weapon::DoneReloading(Event *ev)
{
   Ammo *ammo;
   int amount;

   amount = ammo_clip_size - ammo_in_clip;
   assert(owner);
   if(owner && owner->isClient() && !UnlimitedAmmo())
   {
      if(ammotype.length())
      {
         ammo = static_cast<Ammo *>(owner->FindItem(ammotype.c_str()));
         if(ammo)
         {
            if(ammo->Amount() < amount)
               amount = ammo->Amount();
            if(!ammo->Use(amount))
            {
               amount = 0;
            }
         }
         SetAmmoAmount(amount + ammo_in_clip);
      }
   }
   else
   {
      SetAmmoAmount(ammo_clip_size);
   }

   ForceIdle();

   if(owner)
   {
      owner->ProcessEvent(EV_Sentient_WeaponReady);
   }
}

qboolean Weapon::ForceReload()
{
   if(weaponstate != WEAPON_READY)
      return false;

   if(level.cinematic && owner && owner->isClient())
   {
      return false;
   }

   // do a reload if the ammo in clip is not full, 
   // and there is some ammo available out of clip
   if(
      (ammo_clip_size != ammo_in_clip) &&
      (AmmoAvailable() > 0)
      )
   {
      weaponstate = WEAPON_RELOADING;
      if((owner) && (owner->flags & FL_SILENCER) && (silenced))
         RandomAnimate("silreload", EV_Weapon_DoneReloading);
      else
         RandomAnimate("reload", EV_Weapon_DoneReloading);
      return true;
   }
   return false;
}

qboolean Weapon::CheckReload()
{
   if(weaponstate != WEAPON_READY)
      return false;

   if(level.cinematic && owner && owner->isClient())
   {
      return false;
   }

   if(ammo_clip_size && !ammo_in_clip && HasAmmo())
   {
      weaponstate = WEAPON_RELOADING;
      if((owner) && (owner->flags & FL_SILENCER) && (silenced))
         RandomAnimate("silreload", EV_Weapon_DoneReloading);
      else
         RandomAnimate("reload", EV_Weapon_DoneReloading);
      return true;
   }
   return false;
}

void Weapon::Idle(Event *ev)
{
   if(ammo_clip_size && (!ammo_in_clip || level.time > last_attack_time + 2))
   {
      if(CheckReload())
         return;
   }

   ForceIdle();
}

void Weapon::FinishAttack(Event *ev)
{
   attack_finished = level.time;
}

void Weapon::NextAttack(double rate)
{
   attack_finished = level.time + (float)rate;
}

float Weapon::GetMaxRange()
{
   return maxrange;
}

float Weapon::GetMinRange()
{
   return minrange;
}

float Weapon::GetProjectileSpeed()
{
   return projectilespeed;
}

void Weapon::SetMaxRangeEvent(Event *ev)
{
   maxrange = ev->GetFloat(1);
}

void Weapon::SetMinRangeEvent(Event *ev)
{
   minrange = ev->GetFloat(1);
}

void Weapon::SetProjectileSpeedEvent(Event *ev)
{
   projectilespeed = ev->GetFloat(1);
}

void Weapon::NotDroppableEvent(Event *ev)
{
   notdroppable = true;
}

void Weapon::SetMaxRange(float val)
{
   maxrange = val;
}

void Weapon::SetMinRange(float val)
{
   minrange = val;
}

void Weapon::SetProjectileSpeed(float val)
{
   projectilespeed = val;
}

void Weapon::EventMuzzleFlash(Event *ev)
{
   if(!owner)
   {
      return;
   }

   SpawnTempDlight
   (
      owner->worldorigin,
      ev->GetFloat(1),
      ev->GetFloat(2),
      ev->GetFloat(3),
      ev->GetFloat(4),
      ev->GetFloat(5),
      ev->GetFloat(6)
   );
}

void Weapon::MuzzleFlash(float r, float g, float b, float radius, float decay, float life)
{
   if(!owner)
   {
      return;
   }

   SpawnTempDlight
   (
      owner->worldorigin,
      r,
      g,
      b,
      radius,
      decay,
      life
   );
}

void Weapon::BulletHole(trace_t *trace)
{
   Entity *hole;
   Vector norm, norm2;
   Entity *hit;

   hit = trace->ent->entity;

   if(!hit)
   {
      return;
   }

   if(trace->surface &&
      (trace->surface->flags & SURF_NODRAW))
      return;

   if(hit->hidden())
      return;

   // FIXME: Make Bullet holes client side.
   if(deathmatch->value)
      return;

   hole = new Entity();
   hole->setMoveType(MOVETYPE_PUSH);
   hole->setSolidType(SOLID_NOT);
   hole->setModel("sprites/bullethole.spr");
   hole->setSize({ 0, 0, 0 }, { 0, 0, 0 });

   norm = trace->plane.normal;
   norm2.x = -norm.x;
   norm2.y = -norm.y;
   norm2.z = norm.z;

   hole->angles = norm2.toAngles();
   hole->setAngles(hole->angles);
   hole->setOrigin(Vector(trace->endpos) + (norm * 0.2));

   if((trace->ent->solid == SOLID_BSP) && (hit != world))
   {
      hole->bind(hit);
   }

   queueBulletHoles.Enqueue(hole);
   numBulletHoles++;

   if(numBulletHoles > sv_maxbulletholes->value)
   {
      // Fade one out of the list.
      static_cast<Entity *>(queueBulletHoles.Dequeue())->ProcessEvent(EV_Remove);
      numBulletHoles--;
   }
}

EXPORT_FROM_DLL void Weapon::WeaponSound(Event *ev)
{
   Event *e;

   // Broadcasting a sound can be time consuming.  Only do it once in a while on really fast guns.
   if(nextweaponsoundtime > level.time)
   {
      if(owner)
      {
         owner->BroadcastSound(ev, CHAN_WEAPON, NullEvent, 0);
      }
      else
      {
         BroadcastSound(ev, CHAN_WEAPON, EV_HeardWeapon, 0);
      }
      return;
   }

   if(owner)
   {
      e = new Event(ev);
      owner->ProcessEvent(e);
   }
   else
   {
      Item::WeaponSound(ev);
   }

   // give us some breathing room
   nextweaponsoundtime = level.time + 0.4;
}

qboolean Weapon::Removable()
{
   if(
      ((int)(dmflags->value) & DF_WEAPONS_STAY) &&
      !(spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM))
      )
      return false;
   else
      return true;
}

qboolean Weapon::Pickupable(Entity *other)
{
   Sentient *sen;

   if(!other->isSubclassOf<Sentient>())
   {
      return false;
   }
   else if(!other->isClient())
   {
      return false;
   }

   sen = (Sentient *)other;

   //FIXME
   // This should be in player

   // Mutants can't pickup weapons
   if(other->flags & (FL_MUTANT | FL_SP_MUTANT))
   {
      return false;
   }

   // If we have the weapon and weapons stay, then don't pick it up
   if(((int)(dmflags->value) & DF_WEAPONS_STAY) && !(spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM)))
   {
      auto weapon = static_cast<Weapon *>(sen->FindItem(getClassname()));

      if(weapon)
         return false;
   }

   return true;
}

qboolean Weapon::AutoChange()
{
   return true;
}

int Weapon::ClipAmmo()
{
   if(ammo_clip_size)
      return ammo_in_clip;
   else
      return -1;
}

void Weapon::ProcessWeaponCommandsEvent(Event *ev)
{
   int index;

   index = ev->GetInteger(1);
   ProcessInitCommands(index);
}

void Weapon::SetKick(Event *ev)
{
   kick = ev->GetInteger(1);
}

void Weapon::SecondaryUse(Event *ev)
{
   if(!dualmode)
      return;

   // Switch to the secondary mode of the weapon

   if(weaponstate != WEAPON_READY)
      return;

   weaponstate = WEAPON_CHANGING;

   if(weaponmode == PRIMARY)
   {
      RandomAnimate("primary2secondary", EV_Weapon_SecondaryMode);
      weaponmode = SECONDARY;
   }
   else
   {
      RandomAnimate("secondary2primary", EV_Weapon_PrimaryMode);
      weaponmode = PRIMARY;
   }
}

void Weapon::PrimaryMode(Event *ev)
{
   RandomAnimate("primaryidle", EV_Weapon_Idle);
   weaponstate = WEAPON_READY;
   ammotype = primary_ammo_type;
}

void Weapon::SecondaryMode(Event *ev)
{
   RandomAnimate("secondaryidle", EV_Weapon_Idle);
   weaponstate = WEAPON_READY;
   ammotype = secondary_ammo_type;
}

void Weapon::SetActionLevelIncrement(Event *ev)
{
   action_level_increment = ev->GetInteger(1);
}

int Weapon::ActionLevelIncrement()
{
   return action_level_increment;
}

qboolean Weapon::IsDroppable()
{
   if(notdroppable || !worldmodel.length())
   {
      return false;
   }
   else
   {
      return true;
   }
}

qboolean Weapon::IsSilenced()
{
   return silenced;
}

void Weapon::Raise(Event *ev)
{
   weaponstate = WEAPON_HOLSTERED;
   ReadyWeapon();
}

void Weapon::PutAwayAndRaise(Event *ev)
{
   weaponstate = WEAPON_LOWERING;

   if(deathmatch->value && ((int)dmflags->value & DF_FAST_WEAPONS))
   {
      ProcessEvent(EV_Weapon_DoneRaising);
      return;
   }

   RandomAnimate("putaway", EV_Weapon_Raise);
}

void Weapon::SetPrimaryMode()
{
   weaponmode = PRIMARY;
   ammotype = primary_ammo_type;
}

void Weapon::SetSecondaryMode()
{
   weaponmode = SECONDARY;
   ammotype = secondary_ammo_type;
}

//### 2015 added stuffs

void Weapon::AngleJitterEvent(Event *ev)
{
   if(!owner)
      return;

   if(!owner->isClient())
      return;

   auto e = new Event(EV_Player_SetAngleJitter);
   e->AddFloat(ev->GetFloat(1));
   e->AddFloat(ev->GetFloat(2));
   e->AddFloat(ev->GetFloat(3));
   owner->ProcessEvent(e);
}

void Weapon::OffsetJitterEvent(Event *ev)
{
   if(!owner)
      return;

   if(!owner->isClient())
      return;

   auto e = new Event(EV_Player_SetOffsetJitter);
   e->AddFloat(ev->GetFloat(1));
   e->AddFloat(ev->GetFloat(2));
   e->AddFloat(ev->GetFloat(3));
   owner->ProcessEvent(e);
}

//###

// EOF
