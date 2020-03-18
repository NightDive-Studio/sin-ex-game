//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/item.cpp                         $
// $Revision:: 51                                                             $
//   $Author:: Markd                                                          $
//     $Date:: 11/12/98 9:20p                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Base class for respawnable, carryable objects.
// 

#include "g_local.h"
#include "entity.h"
#include "trigger.h"
#include "item.h"
#include "inventoryitem.h"
#include "scriptmaster.h"
#include "health.h"

Event EV_Item_Pickup("item_pickup");
Event EV_Item_DropToFloor("droptofloor");
Event EV_Item_Respawn("respawn");
Event EV_Item_SetAmount("amount");
Event EV_Item_SetMaxAmount("maxamount");
Event EV_Item_SetIconName("iconname");
Event EV_Item_SetItemName("itemname");
Event EV_Item_RespawnSound("respawnsound");
Event EV_Item_DialogNeeded("dialogneeded");

CLASS_DECLARATION(Trigger, Item, nullptr);

ResponseDef Item::Responses[] =
{
   { &EV_Trigger_Effect,	   (Response)&Item::ItemTouch },
   { &EV_Item_DropToFloor,	   (Response)&Item::DropToFloor },
   { &EV_Item_Respawn,		   (Response)&Item::Respawn },
   { &EV_Item_SetAmount,      (Response)&Item::SetAmount },
   { &EV_Item_SetMaxAmount,   (Response)&Item::SetMaxAmount },
   { &EV_Item_SetIconName,    (Response)&Item::SetIconName },
   { &EV_Item_SetItemName,    (Response)&Item::SetItemName },
   { &EV_Item_Pickup,			(Response)&Item::Pickup },
   { &EV_Use,		            (Response)&Item::TriggerStuff },
   { &EV_Item_RespawnSound,   (Response)&Item::RespawnSound },
   { &EV_Item_DialogNeeded,   (Response)&Item::DialogNeeded },
   { nullptr, nullptr }
};

Item::Item() : Trigger()
{
   str fullname;
   Vector defangles;

   setRespawnTime(20);
   setRespawn(false);

   setSolidType(SOLID_NOT);

   // Set default respawn behavior
   // Derived classes should use setRespawn
   // if they want to override the default behavior
   if(deathmatch->value)
   {
      setRespawn(true);
   }
   else
   {
      setRespawn(false);
   }

   edict->s.renderfx |= RF_GLOW;

   // angles
   defangles = Vector(0, G_GetFloatArg("angle", 0), 0);
   if(defangles.y == -1)
   {
      defangles = Vector(-90, 0, 0);
   }
   else if(defangles.y == -2)
   {
      defangles = Vector(90, 0, 0);
   }
   angles = G_GetVectorArg("angles", defangles);
   setAngles(angles);

   //
   // we want the bounds of this model auto-rotated
   //
   flags |= FL_ROTATEDBOUNDS;

   //
   // rotate the mins and maxs for the model
   //
   if(size.length() < 10)
      setSize({ -10, -10, 0 }, { 10, 10, 20 });

   //
   // reset the mins and maxs to pickup the FL_ROTATEDBOUNDS flag
   //
   setSize(mins, maxs);

   if(!LoadingSavegame)
   {
      // Items can't be immediately dropped to floor, because they might
      // be on an entity that hasn't spawned yet.
      PostEvent(EV_Item_DropToFloor, 0);
   }

   //### so players can pick up stuff while on a hoverbike
   //respondto = TRIGGER_PLAYERS;
   respondto = TRIGGER_PLAYERS | TRIGGER_HOVERBIKES;

   icon_name = str("");
   icon_index = 0;
   item_index = 0;
   maximum_amount = 1;
   playrespawn = false;

   amount_override = false;
   amount = 1;

   // FIXME
   // If the targetname is set by the spawn args, then this item
   // will have a targetname.  If we try to remove the owner of this
   // item, then we will remove the owner, then try to remove the item
   // which will already have been removed by the previous event.
   // This doesn't allow any items to have a targetname.
   SetTargetName("");

   // Using itemname as a temporary fix to this problem
   itemname = G_GetSpawnArg("itemname", "");

   if(G_GetSpawnArg("amount"))
   {
      amount = G_GetIntArg("amount");
      if(amount >= MaxAmount())
      {
         SetMax(amount);
      }
      amount_override = true;
   }
}

Item::~Item()
{
   if(owner)
   {
      owner->RemoveItem(this);
      owner = nullptr;
   }
}

void Item::CreateSpawnArgs(void)
{
   G_SetIntArg("amount", amount);
   G_SetSpawnArg("model", model.c_str());
}

/*
============
PlaceItem

Puts an item back in the world
============
*/
void Item::PlaceItem(void)
{
   setSolidType(SOLID_TRIGGER);
   setMoveType(MOVETYPE_TOSS);
   showModel();

   edict->s.renderfx |= RF_GLOW;
   groundentity = NULL;
}

/*
============
DropToFloor

plants the object on the floor
============
*/
void Item::DropToFloor(Event *ev)
{
   str fullname;
   Vector save;

   PlaceItem();

   setOrigin(origin + Vector(0, 0, 1));
   save = origin;
   if(!droptofloor(8192))
   {
      gi.dprintf("%s fell out of level at '%5.1f %5.1f %5.1f'\n",
                 getClassID(), origin.x, origin.y, origin.z);
      PostEvent(EV_Remove, 0);
      return;
   }
   //
   // if the our global variable doesn't exist, lets zero it out
   //
   fullname = str("playeritem_") + getClassname();
   if(!gameVars.VariableExists(fullname.c_str()))
   {
      gameVars.SetVariable(fullname.c_str(), 0);
   }

   if(!levelVars.VariableExists(fullname.c_str()))
   {
      levelVars.SetVariable(fullname.c_str(), 0);
   }

   setOrigin(save);
   groundentity = NULL;
}

qboolean Item::Drop(void)
{
   if(!owner)
   {
      return false;
   }

   setOrigin(owner->worldorigin + Vector(0, 0, 40));

   // drop the item
   PlaceItem();
   velocity = owner->velocity * 0.5 + Vector(G_CRandom(50), G_CRandom(50), 100);
   setAngles(owner->angles);
   avelocity = Vector(0, G_CRandom(360), 0);

   trigger_time = level.time + 1;

   if(owner->isClient())
   {
      spawnflags |= DROPPED_PLAYER_ITEM;
   }
   else
   {
      spawnflags |= DROPPED_ITEM;
   }

   // Remove this from the owner's item list
   owner->RemoveItem(this);
   owner = NULL;

   return true;
}

void Item::ItemTouch(Event *ev)
{
   Entity	*other;
   Event		*e;

   if(owner)
   {
      // Don't respond to trigger events after item is picked up.
      gi.dprintf("%s with targetname of %s was triggered unexpectedly.\n", getClassID(), TargetName());
      return;
   }

   other = ev->GetEntity(1);

   e = new Event(EV_Item_Pickup);
   e->AddEntity(other);
   ProcessEvent(e);
}

void Item::SetOwner(Sentient *ent)
{
   assert(ent);
   if(!ent)
   {
      // return to avoid any buggy behaviour
      return;
   }

   owner = ent;
   setRespawn(false);

   edict->s.renderfx &= ~RF_GLOW;
   setSolidType(SOLID_NOT);
   hideModel();
   CancelEventsOfType(EV_Item_DropToFloor);
   CancelEventsOfType(EV_Remove);
   //	ItemPickup( ent );
}

Item *Item::ItemPickup(Entity *other)
{
   Sentient * sent;
   Item * item;
   str realname;

   if(!Pickupable(other))
   {
      return NULL;
   }

   sent = (Sentient *)other;

   item = sent->giveItem(getClassname(), Amount(), icon_index);

   if(!item)
      return NULL;

   realname = GetRandomAlias("snd_pickup");
   if(realname.length() > 1)
      sent->sound(realname, 1, CHAN_ITEM, ATTN_NORM);

   if(!Removable())
   {
      // leave the item for others to pickup
      return item;
   }

   CancelEventsOfType(EV_Item_DropToFloor);
   CancelEventsOfType(EV_Item_Respawn);
   CancelEventsOfType(EV_FadeOut);

   setSolidType(SOLID_NOT);
   hideModel();


   if(Respawnable())
   {
      PostEvent(EV_Item_Respawn, RespawnTime());
   }
   else
   {
      PostEvent(EV_Remove, 0.1);
   }

   if(DM_FLAG(DF_INSTANT_ITEMS))
   {
      Event *ev;

      ev = new Event(EV_InventoryItem_Use);
      ev->AddEntity(other);

      item->ProcessEvent(ev);
   }

   return item;
}

void Item::Respawn(Event *ev)
{
   showModel();

   // allow it to be touched again
   setSolidType(SOLID_TRIGGER);

   // play respawn sound
   if(playrespawn)
   {
      RandomGlobalSound("snd_itemspawn");
   }

   setOrigin(origin);
}

void Item::setRespawn(qboolean flag)
{
   respawnable = flag;
}

qboolean Item::Respawnable(void)
{
   return respawnable;
}

void Item::setRespawnTime(float time)
{
   respawntime = time;
}

float Item::RespawnTime(void)
{
   return respawntime;
}

int Item::Amount(void)
{
   return amount;
}

int Item::MaxAmount(void)
{
   return maximum_amount;
}

qboolean Item::Pickupable(Entity *other)
{
   if(!other->isSubclassOf<Sentient>())
   {
      return false;
   }
   else
   {
      Sentient * sent;
      Item * item;

      sent = (Sentient *)other;
      item = sent->FindItem(getClassname());

      if(item && (item->Amount() >= item->MaxAmount()))
      {
         return false;
      }

      // Mutants can't pick up anything but health
      if(other->flags & (FL_MUTANT | FL_SP_MUTANT) && !(this->isSubclassOf<Health>()))
      {
         return false;
      }

      // If deathmatch and already in a powerup, don't pickup anymore when DF_INSTANT_ITEMS is on
      if(DM_FLAG(DF_INSTANT_ITEMS) &&
         this->isSubclassOf<InventoryItem>() &&
         sent->PowerupActive()
         )
      {
         return false;
      }
   }
   return true;
}

void Item::Pickup(Event * ev)
{
   ItemPickup(ev->GetEntity(1));
}

void Item::setIcon(const char *i)
{
   icon_name = i;
   icon_index = gi.imageindex(i);
}

void Item::setName(const char *i)
{
   item_name = i;
   item_index = gi.itemindex(i);
}

int Item::Icon(void)
{
   if(icon_name.length())
      return icon_index;
   else
      return -1;
}

void Item::Set(int startamount)
{
   if(!amount_override)
   {
      amount = startamount;
      if(amount >= MaxAmount())
         SetMax(amount);
   }
}

void Item::SetMax(int maxamount)
{
   maximum_amount = maxamount;
}

void Item::SetAmount(Event *ev)
{
   Set(ev->GetInteger(1));
}

void Item::SetMaxAmount(Event *ev)
{
   SetMax(ev->GetInteger(1));
}

void Item::SetIconName(Event *ev)
{
   setIcon(ev->GetString(1));
}

void Item::SetItemName(Event *ev)
{
   setName(ev->GetString(1));
}

void Item::Add(int num)
{
   amount += num;
   if(amount >= MaxAmount())
      amount = MaxAmount();
}

//###
void Item::DoubleMaxAdd(int num)
{
   amount += num;
   if(amount >= MaxAmount() * 2)
      amount = MaxAmount() * 2;
}
//###

void Item::Remove(int num)
{
   amount -= num;
   if(amount < 0)
      amount = 0;
}

qboolean Item::Use(int num)
{
   if(num > amount)
   {
      return false;
   }

   amount -= num;
   return true;
}

qboolean Item::Removable(void)
{
   return true;
}

void Item::RespawnSound(Event *ev)
{
   playrespawn = true;
}

void Item::DialogNeeded(Event *ev)
{
   //
   // if this item is needed for a trigger, play this dialog
   //
   dialog_needed = ev->GetString(1);
}

str Item::GetDialogNeeded(void)
{
   return dialog_needed;
}

// EOF

