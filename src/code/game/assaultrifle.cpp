//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/assaultrifle.cpp                 $
// $Revision:: 33                                                             $
//   $Author:: Markd                                                          $
//     $Date:: 11/15/98 9:12p                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Assault rifle
// 

#include "g_local.h"
#include "assaultrifle.h"

CLASS_DECLARATION(BulletWeapon, AssaultRifle, "weapon_assaultrifle");

ResponseDef AssaultRifle::Responses[] =
{
   { &EV_Weapon_Shoot, (Response)&AssaultRifle::Shoot },
   { NULL, NULL }
};

AssaultRifle::AssaultRifle()
{
   SetModels("asrifle.def", "view_asrifle.def");
   SetAmmo("Bullet10mm", 1, 30);
   SetRank(40, 40);
   SetType(WEAPON_2HANDED_LO);
   modelIndex("10mm.def");
   modelIndex("sprites/gunblast.spr");
   modelIndex("shell.def");
   silenced = true;
}

void AssaultRifle::Shoot(Event *ev)
{
   FireBullets(1, { 120, 120, 120 }, 8, 14, DAMAGE_BULLET, MOD_ASSRIFLE, false);
   NextAttack(0);
}

// EOF

