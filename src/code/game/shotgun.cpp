//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/shotgun.cpp                      $
// $Revision:: 25                                                             $
//   $Author:: Markd                                                          $
//     $Date:: 11/13/98 3:30p                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Shotgun
// 

#include "g_local.h"
#include "bullet.h"
#include "shotgun.h"

CLASS_DECLARATION(BulletWeapon, Shotgun, "weapon_shotgun");

ResponseDef Shotgun::Responses[] =
{
   { &EV_Weapon_Shoot, (Response)&Shotgun::Shoot },
   { NULL, NULL }
};

Shotgun::Shotgun() : BulletWeapon()
{
   SetModels("shotgun.def", "view_shotgun.def");
   SetAmmo("ShotgunClip", 1, 10);
   SetRank(30, 30);
   SetType(WEAPON_2HANDED_HI);
   modelIndex("shotgunclip.def");
}

void Shotgun::Shoot(Event *ev)
{
   // Non clients use a toned down version of the shotgun
   if(owner->isClient())
      FireBullets(10, { 375, 375, 375 }, 8, 16, DAMAGE_BULLET, MOD_SHOTGUN, false);
   else
      FireBullets(5, { 100, 100, 100 }, 5, 10, DAMAGE_BULLET, MOD_SHOTGUN, false);

   NextAttack(0.95f);
}

// EOF

