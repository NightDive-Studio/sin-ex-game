//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/armor.cpp                        $
// $Revision:: 21                                                             $
//   $Author:: Aldie                                                          $
//     $Date:: 10/24/98 2:07p                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Standard armor that prevents a percentage of damage per hit.
// 

#include "g_local.h"
#include "armor.h"

/*
========
ARMOR
========
*/

CLASS_DECLARATION(Item, Armor, NULL)

ResponseDef Armor::Responses[] =
{
   { NULL, NULL }
};

Armor::Armor()
{
   if(DM_FLAG(DF_NO_ARMOR))
   {
      PostEvent(EV_Remove, 0);
      return;
   }

   Set(0);
}

void Armor::Setup(const char *model, int amount)
{
   assert(model);
   setModel(model);
   Set(amount);
}

void Armor::Add(int num)
{
   // Armor never adds, it only replaces
   amount = num;
   if(amount >= MaxAmount())
      amount = MaxAmount();
}

qboolean Armor::Pickupable(Entity *other)
{
   if(!other->isSubclassOf<Sentient>())
   {
      return false;
   }
   else
   {
      Sentient * sent;
      Item * item;

      sent = static_cast<Sentient *>(other);

      // Mutants don't get armor
      if(sent->flags & (FL_MUTANT | FL_SP_MUTANT))
      {
         return false;
      }

      item = sent->FindItem(getClassname());
      // If our armor is > than our current armor or armor has no value, then leave it alone.
      if(item)
      {
         if((item->Amount() >= this->Amount()) || !this->Amount())
         {
            return false;
         }
      }
   }
   return true;
}

CLASS_DECLARATION(Armor, RiotHelmet, "armor_riothelmet");

ResponseDef RiotHelmet::Responses[] =
{
   { NULL, NULL }
};

RiotHelmet::RiotHelmet()
{
   Setup("riothelm.def", MAX_ARMOR);
}

CLASS_DECLARATION(Armor, FlakJacket, "armor_flakjacket");

ResponseDef FlakJacket::Responses[] =
{
   { NULL, NULL }
};

FlakJacket::FlakJacket()
{
   Setup("flakjack.def", MAX_ARMOR);
}

CLASS_DECLARATION(Armor, FlakPants, "armor_flakpants");

ResponseDef FlakPants::Responses[] =
{
   { NULL, NULL }
};

FlakPants::FlakPants()
{
   Setup("flakpants.def", MAX_ARMOR);
}

// EOF

