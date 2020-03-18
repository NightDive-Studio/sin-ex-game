//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/sniperrifle.cpp                  $
// $Revision:: 28                                                             $
//   $Author:: Jimdose                                                        $
//     $Date:: 3/12/99 8:15p                                                  $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Sniper rifle
// 

#include "g_local.h"
#include "bullet.h"
#include "sniperrifle.h"
#include "player.h"

CLASS_DECLARATION(BulletWeapon, SniperRifle, "weapon_sniperrifle");

Event EV_SniperRifle_Ready("sniperrifle_ready");
Event EV_SniperRifle_PositionDot("positiondot"); //###

ResponseDef SniperRifle::Responses[] =
{
   { &EV_Weapon_SecondaryUse,     (Response)&SniperRifle::SecondaryUse },
   { &EV_SniperRifle_Ready,       (Response)&SniperRifle::DoneRaising },
   { &EV_Weapon_DoneRaising,      (Response)&SniperRifle::Open },
   { &EV_Weapon_Shoot,            (Response)&SniperRifle::Shoot },
   { &EV_SniperRifle_PositionDot, (Response)&SniperRifle::PositionDot }, //###
   { nullptr, nullptr }
};

SniperRifle::SniperRifle() : BulletWeapon()
{
   //###
   if(ctf->value)
      SetModels( "sniperrifle.def", "view_srifle_ctf.def" );
   else
   //###
      SetModels("sniperrifle.def", "view_srifle.def");
   SetAmmo("BulletSniper", 1, 5);
   modelIndex("sniper_ammo.def");
   SetRank(100, 0);
   SetType(WEAPON_2HANDED_HI);
}

// CTF - Fire upon release of the weapon
void SniperRifle::ReleaseFire(float holdfiretime)
{
   RemoveDot(); //###
   StopAnimating();
   RandomAnimate("releasefire", EV_Weapon_DoneFiring);
   last_animation_time = (level.framenum + 1) * FRAMETIME;
}

void SniperRifle::Shoot(Event *ev)
{
   assert(owner);
   if(!owner)
   {
      return;
   }

   if(ctf->value)
   {
      // CTF - kill the target
      FireBullets(1, vec_zero, 10000, 10000, DAMAGE_BULLET | DAMAGE_NO_ARMOR, MOD_SNIPER, false);
   }
   else if(deathmatch->value)
   {
      FireBullets(1, vec_zero, 105, 135, DAMAGE_BULLET | DAMAGE_NO_ARMOR, MOD_SNIPER, false);
   }
   else
   {
      FireBullets(1, vec_zero, 105, 135, DAMAGE_BULLET, MOD_SNIPER, false);
   }

   NextAttack(1.5);
}

void SniperRifle::DoneRaising(Event *ev)
{
   BulletWeapon::DoneRaising(ev);
}

void SniperRifle::Open(Event *ev)
{
   RandomAnimate("open", EV_SniperRifle_Ready);
}

void SniperRifle::SecondaryUse(Event *ev)
{
   Entity *ent;
   Event *event;

   event = new Event(EV_Player_ToggleZoomMode);
   ent = ev->GetEntity(1);
   ent->ProcessEvent(event);
}

void SniperRifle::DoneLowering(Event *ev)
{
   Event *event;

   assert(owner);
   if(!owner)
   {
      return;
   }

   event = new Event(EV_Player_ZoomOut);
   owner->ProcessEvent(event);

   Weapon::DoneLowering(ev);
}

qboolean SniperRifle::AutoChange()
{
   return false;
}

//### ==========================================================================
// added a laser dot to the sniper rifle to help balance out it's one-shotedness

void SniperRifle::RemoveDot()
{
   if(LaserDot)
   {
      delete LaserDot;
      LaserDot = nullptr;
   }
}

void SniperRifle::PositionDot(Event *ev)
{
   Vector src, dir, pos;
   trace_t trace;
   int mask;

   if(!LaserDot)
   {
      LaserDot = new Entity();
      LaserDot->setModel("sprites/laserdot.spr");
      LaserDot->setMoveType(MOVETYPE_NONE);
      LaserDot->setSolidType(SOLID_NOT);
   }

   GetMuzzlePosition(&src, &dir);

   mask  = MASK_SHOT | MASK_WATER;
   pos   = src + dir * 8192;
   trace = G_Trace(src, vec_zero, vec_zero, pos, owner, mask, "SniperRifle::PositionDot");

   pos = Vector(trace.endpos) - dir * 8;

   LaserDot->setOrigin(pos);
}

void SniperRifle::DetachGun()
{
   RemoveDot();

   Weapon::DetachGun();
}

SniperRifle::~SniperRifle()
{
   RemoveDot();
}

//
//### ==========================================================================

// EOF

