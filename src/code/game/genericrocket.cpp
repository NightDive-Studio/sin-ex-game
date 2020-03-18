//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/genericrocket.cpp                $
// $Revision:: 3                                                              $
//   $Author:: Markd                                                          $
//     $Date:: 10/04/98 10:23p                                                $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Generic Rocket Launcher - to be used on monsters that have rocket launchers
// shown in their models

#include "g_local.h"
#include "genericrocket.h"
#include "rocketlauncher.h"

CLASS_DECLARATION(RocketLauncher, GenericRocket, "weapon_genericrocket");

ResponseDef GenericRocket::Responses[] =
{
   { &EV_Weapon_Shoot,  (Response)&GenericRocket::Shoot },
   { NULL, NULL }
};

GenericRocket::GenericRocket() : RocketLauncher()
{
   SetModels(NULL, "view_genrocket.def");
   SetAmmo("Rockets", 1, 5);
}

void GenericRocket::Shoot(Event *ev)
{
   Rocket	*rocket;
   Vector	pos;
   Vector	dir;

   assert(owner);
   if(!owner)
   {
      return;
   }

   GetMuzzlePosition(&pos, &dir);

   rocket = new Rocket();
   rocket->Setup(owner, pos, dir);
}

// EOF

