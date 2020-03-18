//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/heligun.cpp                  $
// $Revision:: 11                                                             $
//   $Author:: Aldie                                                          $
//     $Date:: 8/06/98 10:53p                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Helicopter gun
// 

#include "g_local.h"
#include "bullet.h"
#include "heligun.h"
#include "rocketlauncher.h"
#include "explosion.h"

CLASS_DECLARATION(BulletWeapon, HeliGun, "weapon_heligun");

ResponseDef HeliGun::Responses[] =
{
   { &EV_Weapon_Shoot,           (Response)&HeliGun::Shoot },
   { NULL, NULL }
};

HeliGun::HeliGun() : BulletWeapon()
{
   SetModels(nullptr, "view_heligun.def");
   SetAmmo("Bullet50mm", 0, 0);
   SetRank(50, 50);
}

void HeliGun::Shoot(Event *ev)
{
   FireBullets(1, { 20, 20, 20 }, 16, 28, DAMAGE_BULLET, MOD_HELIGUN, false);
   NextAttack(0);
}

// EOF

