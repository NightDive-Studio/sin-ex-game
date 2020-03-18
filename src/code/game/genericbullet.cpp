//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/genericbullet.cpp                $
// $Revision:: 16                                                             $
//   $Author:: Aldie                                                          $
//     $Date:: 10/27/98 3:44a                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Generic Bullet Weapon.
// 

#include "g_local.h"
#include "genericbullet.h"

CLASS_DECLARATION( BulletWeapon, GenericBullet, "weapon_genericbullet" );

ResponseDef GenericBullet::Responses[] =
{
   { &EV_Weapon_Shoot, (Response)&GenericBullet::Shoot },
   { NULL, NULL }
};

GenericBullet::GenericBullet() : BulletWeapon()
{
   SetModels(NULL, "view_genbullet.def");
   SetAmmo("Bullet10mm", 1, 100);
}

void GenericBullet::Shoot(Event *ev)
{
   FireTracer();
   FireBullets(1, { 10, 10, 10 }, 2, 3, DAMAGE_BULLET, MOD_GENBULLET, true);
   NextAttack(0);
}

CLASS_DECLARATION(GenericBullet, ReconahGun, "weapon_reconahgun");

ResponseDef ReconahGun::Responses[] =
{
   { &EV_Weapon_Shoot, (Response)&ReconahGun::Shoot },
   { NULL, NULL }
};

void ReconahGun::Shoot(Event *ev)
{
   if((level.framenum % 3) == (entnum % 3))
   {
      FireTracer();
   }

   FireBullets(1, { 10, 10, 10 }, 14, 26, DAMAGE_BULLET, MOD_GENBULLET, true);
}

CLASS_DECLARATION(GenericBullet, BeeGun, "weapon_beegun");

ResponseDef BeeGun::Responses[] =
{
   { &EV_Weapon_Shoot, (Response)&BeeGun::Shoot },
   { NULL, NULL }
};

void BeeGun::Shoot(Event *ev)
{
   FireBullets(1, { 10, 10, 10 }, 4, 8, DAMAGE_BULLET, MOD_GENBULLET, true);
}

// EOF

