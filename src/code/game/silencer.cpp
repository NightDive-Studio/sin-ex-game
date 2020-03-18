//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/silencer.cpp                     $
// $Revision:: 10                                                             $
//   $Author:: Jimdose                                                        $
//     $Date:: 10/19/98 12:07a                                                $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Silencer Powerup

#include "silencer.h"
#include "sentient.h"

CLASS_DECLARATION(InventoryItem, Silencer, "powerups_silencer")

ResponseDef Silencer::Responses[] =
{
   { &EV_Trigger_Effect,					(Response)&Silencer::PickupSilencer },
   { NULL, NULL }
};

Silencer::Silencer() : InventoryItem()
{
   setModel("silencer.def");
   icon_name = "i_silencer";
   icon_index = gi.imageindex(icon_name.c_str());
   Set(1);
}

void Silencer::PickupSilencer(Event *ev)
{
   Sentient    *sen;
   Entity      *other;

   other = ev->GetEntity(1);
   if(ItemPickup(other))
   {
      if(other->isSubclassOf<Sentient>())
      {
         sen = (Sentient *)other;
         // Make the user lower and raise the weapon so that the silencer will attach
         sen->UpdateSilencedWeapons();

         sen->flags |= FL_SILENCER;
      }
   }
}

// EOF

