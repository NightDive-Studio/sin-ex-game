//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/magnum.cpp                       $
// $Revision:: 46                                                             $
//   $Author:: Markd                                                          $
//     $Date:: 11/13/98 3:30p                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Magnum.
// 

#include "g_local.h"
#include "magnum.h"

CLASS_DECLARATION(BulletWeapon, Magnum, "weapon_magnum");

ResponseDef Magnum::Responses[] =
{
   { &EV_Weapon_Shoot, (Response)&Magnum::Shoot },
   { NULL, NULL }
};

Magnum::Magnum() : BulletWeapon()
{
   SetModels("magnum.def", "view_magnum.def");
   SetAmmo("Bullet10mm", 1, 100);
   SetRank(20, 20);
   SetType(WEAPON_1HANDED);
   modelIndex("10mm.def");
   silenced = true;
}

void Magnum::Shoot(Event *ev)
{
   NextAttack(0.20);
   FireBullets(1, { 10, 10, 10 }, 12, 24, DAMAGE_BULLET, MOD_MAGNUM, false);
}

qboolean Magnum::Drop(void)
{
   // Don't leave magnums around
   if(owner && owner->deadflag && deathmatch->value)
   {
      return false;
   }

   return BulletWeapon::Drop();
}

// EOF

