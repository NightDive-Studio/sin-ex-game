//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/ammo.cpp                         $
// $Revision:: 28                                                             $
//   $Author:: Aldie                                                          $
//     $Date:: 10/24/98 2:07p                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Base class for all ammunition for entities derived from the Weapon class.
// 

#include "g_local.h"
#include "item.h"
#include "ammo.h"

CLASS_DECLARATION(Item, Ammo, NULL);

ResponseDef Ammo::Responses[] =
{
   { NULL, NULL }
};

Ammo::Ammo()
{
   Set(0);
}

void Ammo::Setup(const char *model)
{
   const char * m;
   m = G_GetSpawnArg("model");
   if(m)
   {
      setModel(m);
   }
   else
   {
      assert(model);
      setModel(model);
   }
}

CLASS_DECLARATION(Ammo, Bullet10mm, "ammo_10mm");

ResponseDef Bullet10mm::Responses[] =
{
   { NULL, NULL }
};

Bullet10mm::Bullet10mm()
{
   Setup("357.def");
}

CLASS_DECLARATION(Ammo, Bullet50mm, "ammo_50mm");

ResponseDef Bullet50mm::Responses[] =
{
   { NULL, NULL }
};

Bullet50mm::Bullet50mm()
{
   Setup("50mm.def");
}

CLASS_DECLARATION(Ammo, BulletPulse, "ammo_pulserifle");

ResponseDef BulletPulse::Responses[] =
{
   { NULL, NULL }
};

BulletPulse::BulletPulse()
{
   Setup("pulse_ammo.def");
}

CLASS_DECLARATION(Ammo, BulletSniper, "ammo_sniperrifle");

ResponseDef BulletSniper::Responses[] =
{
   { NULL, NULL }
};

BulletSniper::BulletSniper()
{
   Setup("sniper_ammo.def");
}

CLASS_DECLARATION(Ammo, Rockets, "ammo_rockets");

ResponseDef Rockets::Responses[] =
{
   { NULL, NULL }
};

Rockets::Rockets()
{
   Setup("rockets.def");
}

CLASS_DECLARATION(Ammo, Spears, "ammo_speargun");

ResponseDef Spears::Responses[] =
{
   { NULL, NULL }
};

Spears::Spears()
{
   Setup("spear_ammo.def");
}

CLASS_DECLARATION(Ammo, ShotgunClip, "ammo_shotgunclip");

ResponseDef ShotgunClip::Responses[] =
{
   { NULL, NULL }
};

ShotgunClip::ShotgunClip()
{
   Setup("shotgunclip.def");
}

CLASS_DECLARATION(Ammo, SpiderMines, "ammo_spidermines");

ResponseDef SpiderMines::Responses[] =
{
   { NULL, NULL }
};

SpiderMines::SpiderMines()
{
   Setup("mine.def");
}

// EOF

