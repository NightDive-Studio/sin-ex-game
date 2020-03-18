//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/reactiveshields.cpp              $
// $Revision:: 12                                                             $
//   $Author:: Aldie                                                          $
//     $Date:: 3/24/99 4:30p                                                  $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Reactive Shields

#include "inventoryitem.h" 

Event EV_Item_ReactiveShields_PowerDown("shields_powerdown");

class EXPORT_FROM_DLL ReactiveShields : public InventoryItem
{
public:
   CLASS_PROTOTYPE(ReactiveShields);
   ReactiveShields();
   void        Use(Event *ev);
   void        PowerDown(Event *ev);
};

CLASS_DECLARATION(InventoryItem, ReactiveShields, "powerup_shield")

ResponseDef ReactiveShields::Responses[] =
{
   { &EV_InventoryItem_Use,				(Response)&ReactiveShields::Use },
   { &EV_Item_ReactiveShields_PowerDown,(Response)&ReactiveShields::PowerDown },
   { nullptr, nullptr }
};

ReactiveShields::ReactiveShields() : InventoryItem()
{
   setModel("shield.def");
   Set(1);
   setRespawnTime(180 + G_Random(60));
}

void ReactiveShields::Use(Event *ev)
{
   Event *event;
   str realname;

   if(!owner)
   {
      CancelPendingEvents();
      PostEvent(EV_Remove, 0);
      return;
   }

   if(owner->PowerupActive())
   {
      return;
   }

   // Make sure there is a reactive shield available
   assert(amount);

   amount--;
   if(amount <= 0)
   {
      owner->RemoveItem(this);
   }

   owner->flags |= FL_SHIELDS;
   PostEvent(EV_Item_ReactiveShields_PowerDown, 30);

   realname = GetRandomAlias("snd_activate");
   if(realname.length())
      owner->sound(realname, 1, CHAN_ITEM, ATTN_NORM);
   owner->edict->s.renderfx |= RF_DLIGHT;
   owner->edict->s.color_r = 0;
   owner->edict->s.color_g = 0;
   owner->edict->s.color_b = 1;
   owner->edict->s.radius  = 120;
   //### remove the player's heat signature
   owner->edict->s.effects &= ~EF_WARM;
   //###

   event = new Event("poweruptimer");
   event->AddInteger(30);
   event->AddInteger(P_SHIELDS);
   owner->ProcessEvent(event);
}

void ReactiveShields::PowerDown(Event *ev)
{
   if(!owner)
   {
      CancelPendingEvents();
      PostEvent(EV_Remove, 0);
      return;
   }

   if(owner->flags & FL_SHIELDS)
   {
      str realname;

      owner->flags &= ~FL_SHIELDS;
      realname = GetRandomAlias("snd_deactivate");
      if(realname.length())
         owner->sound(realname, 1, CHAN_ITEM, ATTN_NORM);
      owner->edict->s.renderfx &= ~RF_DLIGHT;
      owner->edict->s.color_r = 0;
      owner->edict->s.color_g = 0;
      owner->edict->s.color_b = 0;
      owner->edict->s.radius  = 0;
      //### replace owners heat signature if alive
      if(!owner->deadflag)
         owner->edict->s.effects |= EF_WARM;
      //###
   }
   CancelPendingEvents();
   PostEvent(EV_Remove, 0);
}

// EOF

