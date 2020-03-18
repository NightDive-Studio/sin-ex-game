//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/testweapon.cpp                   $
// $Revision:: 5                                                              $
//   $Author:: Jimdose                                                        $
//     $Date:: 6/19/98 9:30p                                                  $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Weapon for testing view models
// 

#include "g_local.h"
#include "testweapon.h"

cvar_t *gun_model;
cvar_t *gun_wmodel;

CLASS_DECLARATION(Weapon, TestWeapon, nullptr);

ResponseDef TestWeapon::Responses[] =
{
   { &EV_Weapon_Shoot,			(Response)&TestWeapon::Shoot },
   { &EV_Weapon_DoneFiring,	(Response)&TestWeapon::Done },
   { nullptr, nullptr }
};

TestWeapon::TestWeapon() : Weapon()
{
   char *wmodel;
   char *model;

   gun_model = gi.cvar("w_model", "magnum.def", 0);
   gun_wmodel = gi.cvar("ww_model", "magnum_w.def", 0);

   model = gun_model->string;
   wmodel = gun_wmodel->string;

   SetModels(wmodel, model);
   SetAmmo("Bullet357", 0, 0);
   SetRank(0, 0);

   flags |= FL_PRETHINK;
}

void TestWeapon::Prethink()
{
   char *wmodel;
   char *model;

   if(stricmp(gun_model->string, viewmodel.c_str()) ||
      stricmp(gun_wmodel->string, worldmodel.c_str()))
   {
      model = gun_model->string;
      wmodel = gun_wmodel->string;

      SetModels(wmodel, model);
   }
}

void TestWeapon::Shoot(Event *ev)
{
   assert(owner);
   if(!owner)
   {
      return;
   }

   // Long attack since the animation will control it
   NextAttack(1);
}

void TestWeapon::Done(Event *ev)
{
   NextAttack(0);
   Weapon::DoneFiring(ev);
}

// EOF

