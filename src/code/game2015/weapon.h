//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/weapon.h                         $
// $Revision:: 69                                                             $
//   $Author:: Aldie                                                          $
//     $Date:: 4/16/99 5:03p                                                  $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Header file for Weapon class.  The weapon class is the base class for
// all weapons in Sin.  Any entity created from a class derived from the weapon
// class will be usable by any Sentient (players and monsters) as a weapon.
// 

#ifndef __WEAPON_H__
#define __WEAPON_H__

#include "g_local.h"
#include "item.h"
#include "sentient.h"
#include "ammo.h"
#include "queue.h"

extern Event EV_Weapon_Shoot;
extern Event EV_Weapon_FinishAttack;
extern Event EV_Weapon_DoneLowering;
extern Event EV_Weapon_DoneRaising;
extern Event EV_Weapon_DoneFiring;
extern Event EV_Weapon_Idle;
extern Event EV_Weapon_MuzzleFlash;
extern Event EV_Weapon_SecondaryUse;
extern Event EV_Weapon_DoneReloading;
extern Event EV_Weapon_SetMaxRange;
extern Event EV_Weapon_SetMinRange;
extern Event EV_Weapon_SetProjectileSpeed;
extern Event EV_Weapon_ActionIncrement;
extern Event EV_Weapon_PutAwayAndRaise;
extern Event EV_Weapon_NotDroppable;
//### added ability for weapons to add view jitter to their owners
extern Event EV_Weapon_SetAngleJitter;
extern Event EV_Weapon_SetOffsetJitter;
//###

typedef enum
{
   WEAPON_READY,
   WEAPON_FIRING,
   WEAPON_LOWERING,
   WEAPON_RAISING,
   WEAPON_HOLSTERED,
   WEAPON_RELOADING,
   WEAPON_CHANGING,
   WEAPON_CHARGING //### for plasma bow
} weaponstate_t;

typedef enum
{
   WEAPON_MELEE,              // For hands only weapons
   WEAPON_1HANDED,            // For short, lightweight weapons fired high (Magnum, pistols, etc.)
   WEAPON_2HANDED_HI,         // For long, aimed weapons fired high (sniper rifle, shotgun, etc.)
   WEAPON_2HANDED_LO          // For long, heavy weapons fired low (rocket launcher)
} weapontype_t;

typedef enum
{
   PRIMARY,
   SECONDARY
} weaponmode_t;

class EXPORT_FROM_DLL Weapon : public Item
{
private:
   qboolean             attached;
   float                nextweaponsoundtime     = 0.0f;

protected:
   float                maxrange;            // maximum effective firing distance (for AI)
   float                minrange;            // minimum safe firing distance (for AI)
   float                projectilespeed;     // speed of the projectile (0 == infinite speed)

   float                attack_finished         = 0.0f;
   float                flashtime;
   float                flashdecay;
   weaponstate_t        weaponstate             = WEAPON_HOLSTERED;
   weaponmode_t         weaponmode;
   qboolean             dualmode                = false;
   str                  primary_ammo_type;
   str                  secondary_ammo_type;
   str                  viewmodel;
   str                  worldmodel;
   str                  ammotype;
   int                  ammorequired            = 0;
   int                  secondary_ammorequired  = 0;
   int                  startammo               = 0;
   int                  rank                    = 0;
   int                  order;
   int                  ammo_clip_size          = 0;
   int                  ammo_in_clip;
   float                last_attack_time;
   weapontype_t         weapontype;
   SentientPtr          last_owner;
   float                last_owner_trigger_time;
   int                  kick                    = 0;
   int                  action_level_increment;
   qboolean             silenced;
   qboolean             notdroppable            = false;
   int                  aimanim                 = -1;
   int                  aimframe                = 0;

   // CTF
   qboolean             alternate_fire;

   void                 SetMaxRangeEvent(Event *ev);
   void                 SetMinRangeEvent(Event *ev);
   void                 SetProjectileSpeedEvent(Event *ev);
   void                 SetSecondaryAmmo(const char *type, int amount, int startamount);

   virtual void         DetachGun();
   virtual void         AttachGun();

   virtual void         PickupWeapon(Event *ev);
   virtual void         DoneLowering(Event *ev);
   virtual void         DoneRaising(Event *ev);
   virtual void         DoneFiring(Event *ev);
   virtual void         Idle(Event *ev);
   virtual qboolean     CheckReload();
   virtual void         DoneReloading(Event *ev);
   virtual void         FinishAttack(Event *ev);
   virtual void         EventMuzzleFlash(Event *ev);
   virtual void         MuzzleFlash(float r, float g, float b, float radius, float decay, float life);
   virtual void         BulletHole(trace_t *trace);
   virtual void         SetAimAnim(Event *ev);

public:

   CLASS_PROTOTYPE(Weapon);

   Weapon();
   virtual ~Weapon();

   virtual void         CreateSpawnArgs() override;

   virtual int          Rank();
   virtual int          Order();
   virtual void         SetRank(int order, int rank);

   virtual void         SetType(weapontype_t type);
   virtual weapontype_t GetType();

   float                GetMaxRange();
   float                GetMinRange();
   float                GetProjectileSpeed();

   void                 SetMaxRange(float val);
   void                 SetMinRange(float val);
   void                 SetProjectileSpeed(float val);

   virtual void         ForceIdle();
   virtual void         NextAttack(double rate);

   virtual void         SetAmmo(const char *type, int amount, int startamount);
   virtual void         SetAmmoAmount(int amount);
   virtual void         UseAmmo(int amount);
   virtual void         SetAmmoClipSize(Event * ev);
   virtual void         SetModels(const char *world, const char *view);

   virtual void         SetOwner(Sentient *ent) override;

   virtual int          AmmoAvailable();
   virtual qboolean     UnlimitedAmmo();
   virtual qboolean     HasAmmo();
   virtual qboolean     HasAmmoInClip();
   virtual qboolean     AttackDone();
   virtual qboolean     ReadyToFire();
   virtual qboolean     ReadyToChange();
   virtual qboolean     ReadyToUse();
   virtual qboolean     ChangingWeapons();
   virtual qboolean     Reloading();
   virtual qboolean     WeaponRaising();
   virtual qboolean     WeaponPuttingAway();

   virtual void         PutAway();
   virtual void         ReadyWeapon();
   virtual qboolean     Drop() override;
   virtual void         Fire();
   virtual void         ReleaseFire(float holdfiretime) { /* No action for base class */ };

   virtual qboolean     Removable() override;
   virtual qboolean     Pickupable(Entity *other) override;
   virtual void         DetachFromOwner();
   virtual void         AttachToOwner();

   virtual void         WeaponSound(Event *ev);

   virtual Vector       MuzzleOffset();
   virtual void         GetMuzzlePosition(Vector *position, Vector *forward = nullptr, Vector *right = nullptr, Vector *up = nullptr);

   virtual qboolean     AutoChange();
   virtual int          ClipAmmo();
   virtual qboolean     ForceReload();

   virtual void         ProcessWeaponCommandsEvent(Event *ev);
   virtual void         SetKick(Event *ev);
   virtual void         SecondaryUse(Event *ev);
   virtual void         PrimaryMode(Event *ev);
   virtual void         SecondaryMode(Event *ev);

   virtual qboolean     IsDroppable();
   virtual int          ActionLevelIncrement();
   virtual void         SetActionLevelIncrement(Event *ev);
   virtual void         PutAwayAndRaise(Event *ev);
   virtual void         Raise(Event *ev);
   virtual qboolean     IsSilenced();
   virtual void         ForceState(weaponstate_t state);
   virtual void         NotDroppableEvent(Event *ev);

   //### added ability for weapons to add view jitter to their owners
   virtual void         AngleJitterEvent(Event *ev);
   virtual void         OffsetJitterEvent(Event *ev);
   //###

   virtual void         Archive(Archiver &arc)   override;
   virtual void         Unarchive(Archiver &arc) override;

   // CTF
   void                 SetPrimaryMode();
   void                 SetSecondaryMode();
   virtual void         TakeAllAmmo();
   qboolean             AlternateFire() { return alternate_fire;   }
   const char          *AmmoType()      { return ammotype.c_str(); } //###
};


inline EXPORT_FROM_DLL void Weapon::Archive(Archiver &arc)
{
   Item::Archive(arc);
   arc.WriteBoolean(attached);
   arc.WriteFloat(nextweaponsoundtime);
   arc.WriteFloat(maxrange);
   arc.WriteFloat(minrange);
   arc.WriteFloat(projectilespeed);
   arc.WriteFloat(attack_finished);
   arc.WriteFloat(flashtime);
   arc.WriteFloat(flashdecay);
   arc.WriteInteger(weaponstate);
   arc.WriteInteger(weaponmode);
   arc.WriteBoolean(dualmode);
   arc.WriteString(primary_ammo_type);
   arc.WriteString(secondary_ammo_type);
   arc.WriteString(viewmodel);
   arc.WriteString(worldmodel);
   arc.WriteString(ammotype);
   arc.WriteInteger(ammorequired);
   arc.WriteInteger(secondary_ammorequired);
   arc.WriteInteger(startammo);
   arc.WriteInteger(rank);
   arc.WriteInteger(order);
   arc.WriteInteger(ammo_clip_size);
   arc.WriteInteger(ammo_in_clip);
   arc.WriteFloat(last_attack_time);
   arc.WriteInteger(weapontype);
   arc.WriteSafePointer(last_owner);
   arc.WriteFloat(last_owner_trigger_time);
   arc.WriteInteger(kick);
   arc.WriteInteger(action_level_increment);
   arc.WriteBoolean(silenced);
   arc.WriteBoolean(notdroppable);
   arc.WriteInteger(aimanim);
   arc.WriteInteger(aimframe);
}

inline EXPORT_FROM_DLL void Weapon::Unarchive(Archiver &arc)
{
   int temp;

   Item::Unarchive(arc);
   arc.ReadBoolean(&attached);
   arc.ReadFloat(&nextweaponsoundtime);
   arc.ReadFloat(&maxrange);
   arc.ReadFloat(&minrange);
   arc.ReadFloat(&projectilespeed);
   arc.ReadFloat(&attack_finished);
   arc.ReadFloat(&flashtime);
   arc.ReadFloat(&flashdecay);

   arc.ReadInteger(&temp);
   weaponstate = (weaponstate_t)temp;
   arc.ReadInteger(&temp);
   weaponmode = (weaponmode_t)temp;

   arc.ReadBoolean(&dualmode);
   arc.ReadString(&primary_ammo_type);
   arc.ReadString(&secondary_ammo_type);
   arc.ReadString(&viewmodel);
   arc.ReadString(&worldmodel);
   arc.ReadString(&ammotype);
   arc.ReadInteger(&ammorequired);
   arc.ReadInteger(&secondary_ammorequired);
   arc.ReadInteger(&startammo);
   arc.ReadInteger(&rank);
   arc.ReadInteger(&order);
   arc.ReadInteger(&ammo_clip_size);
   arc.ReadInteger(&ammo_in_clip);
   arc.ReadFloat(&last_attack_time);

   arc.ReadInteger(&temp);
   weapontype = (weapontype_t)temp;

   arc.ReadSafePointer(&last_owner);
   arc.ReadFloat(&last_owner_trigger_time);
   arc.ReadInteger(&kick);
   arc.ReadInteger(&action_level_increment);
   arc.ReadBoolean(&silenced);
   arc.ReadBoolean(&notdroppable);
   arc.ReadInteger(&aimanim);
   arc.ReadInteger(&aimframe);
}

#ifdef EXPORT_TEMPLATE
template class EXPORT_FROM_DLL SafePtr<Weapon>;
#endif
typedef SafePtr<Weapon> WeaponPtr;

EXPORT_FROM_DLL void ResetBulletHoles(void);

#endif /* weapon.h */

// EOF

