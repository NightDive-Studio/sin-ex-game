//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/secgun.cpp                          $
// $Revision:: 2                                                              $
//   $Author:: Markd                                                          $
//     $Date:: 10/21/98 12:05a                                                $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Securton mounted gun
// 

#include "g_local.h"
#include "item.h"
#include "weapon.h"
#include "genericbullet.h"
#include "secgun.h"


CLASS_DECLARATION(GenericBullet, Secgun, "weapon_secgun");

ResponseDef Secgun::Responses[] =
{
   { &EV_Weapon_Shoot,           (Response)&Secgun::Shoot },
   { nullptr, nullptr }
};

Secgun::Secgun() : GenericBullet()
{
   SetModels("secgun.def", "view_secgun.def");
   SetAmmo(nullptr, 0, 0);
   SetType(WEAPON_2HANDED_LO);
   notdroppable = true;
}

void Secgun::Shoot(Event *ev)
{
   FireBullets(1, { 300, 300, 300 }, 28, 36, DAMAGE_BULLET, MOD_CHAINGUN, false);
   NextAttack(0);
}

// EOF

