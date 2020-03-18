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

Ammo::Ammo() : Item()
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

//###
qboolean Ammo::Pickupable(Entity *other)
{
   if(!other->isSubclassOf<Sentient>())
   {
      return false;
   }
   else
   {
      Sentient *sent;
      Item     *item;

      sent = (Sentient *)other;
      item = sent->FindItem(getClassname());

      if(ctf->value && sent->HasItem("CTF_Tech_AmmoVortex"))
      {
         // CTF_Tech_AmmoVortex allows carrying twice as much ammo
         if(item && (item->Amount() >= (item->MaxAmount() * 2)))
         {
            return false;
         }
      }
      else if(item && (item->Amount() >= item->MaxAmount()))
      {
         return false;
      }

      // Mutants can't pick up ammo
      if(other->flags & (FL_MUTANT | FL_SP_MUTANT))
      {
         return false;
      }
   }
   return true;
}
//###

CLASS_DECLARATION(Ammo, Bullet10mm, "ammo_10mm");

ResponseDef Bullet10mm::Responses[] =
{
   { NULL, NULL }
};

Bullet10mm::Bullet10mm() : Ammo()
{
   Setup("357.def");
}

CLASS_DECLARATION(Ammo, Bullet50mm, "ammo_50mm");

ResponseDef Bullet50mm::Responses[] =
{
   { NULL, NULL }
};

Bullet50mm::Bullet50mm() : Ammo()
{
   Setup("50mm.def");
}

CLASS_DECLARATION(Ammo, BulletPulse, "ammo_pulserifle");

ResponseDef BulletPulse::Responses[] =
{
   { NULL, NULL }
};

BulletPulse::BulletPulse() : Ammo()
{
   Setup("pulse_ammo.def");
}

CLASS_DECLARATION(Ammo, BulletSniper, "ammo_sniperrifle");

ResponseDef BulletSniper::Responses[] =
{
   { NULL, NULL }
};

BulletSniper::BulletSniper() : Ammo()
{
   Setup("sniper_ammo.def");
}

CLASS_DECLARATION(Ammo, Rockets, "ammo_rockets");

ResponseDef Rockets::Responses[] =
{
   { NULL, NULL }
};

Rockets::Rockets() : Ammo()
{
   Setup("rockets.def");
}

CLASS_DECLARATION(Ammo, Spears, "ammo_speargun");

ResponseDef Spears::Responses[] =
{
   { NULL, NULL }
};

Spears::Spears() : Ammo()
{
   Setup("spear_ammo.def");
}

CLASS_DECLARATION(Ammo, ShotgunClip, "ammo_shotgunclip");

ResponseDef ShotgunClip::Responses[] =
{
   { NULL, NULL }
};

ShotgunClip::ShotgunClip() : Ammo()
{
   Setup("shotgunclip.def");
}

CLASS_DECLARATION(Ammo, SpiderMines, "ammo_spidermines");

ResponseDef SpiderMines::Responses[] =
{
   { NULL, NULL }
};

SpiderMines::SpiderMines() : Ammo()
{
   Setup("mine.def");
}

//### new ammo types
CLASS_DECLARATION(Ammo, Missiles, "ammo_missiles");

ResponseDef Missiles::Responses[] =
{
   { NULL, NULL }
};

Missiles::Missiles() : Ammo()
{
   Setup("ammo_missiles.def");
}

CLASS_DECLARATION(Ammo, IlludiumModules, "ammo_illudium");

ResponseDef IlludiumModules::Responses[] =
{
   { NULL, NULL }
};

IlludiumModules::IlludiumModules() : Ammo()
{
   Setup("ammo_illudium.def");
}

CLASS_DECLARATION(Ammo, ConcussionBattery, "dummy_concussionbattery");

ResponseDef ConcussionBattery::Responses[] =
{
   { NULL, NULL }
};

ConcussionBattery::ConcussionBattery() : Ammo()
{
   Setup("concussionbattery.def");
}

CLASS_DECLARATION(Ammo, FlameFuel, "dummy_flamethrowerfuel");

ResponseDef FlameFuel::Responses[] =
{
   { NULL, NULL }
};

FlameFuel::FlameFuel() : Ammo()
{
   Setup("flamethrowerfuel.def");
}
//###

// EOF

