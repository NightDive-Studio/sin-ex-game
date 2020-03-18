//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/item.h                           $
// $Revision:: 24                                                             $
//   $Author:: Aldie                                                          $
//     $Date:: 3/02/99 9:16p                                                  $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Base class for respawnable, carryable objects.
// 

#ifndef __ITEM_H__
#define __ITEM_H__

#include "g_local.h"
#include "entity.h"
#include "trigger.h"
#include "sentient.h"

extern Event EV_Item_Pickup;
extern Event EV_Item_DropToFloor;
extern Event EV_Item_Respawn;
extern Event EV_Item_SetAmount;
extern Event EV_Item_SetMaxAmount;
extern Event EV_Item_SetIconName;
extern Event EV_Item_RespawnSound;
extern Event EV_Item_DialogNeeded;

#define DROPPED_ITEM         0x00008000
#define DROPPED_PLAYER_ITEM  0x00010000

class EXPORT_FROM_DLL Item : public Trigger
{
protected:
   SentientPtr       owner;
   qboolean          respawnable;
   qboolean          playrespawn;
   float             respawntime;
   str               icon_name;
   str               dialog_needed;
   str               item_name;
   int               icon_index;
   int               item_index;
   int               maximum_amount;
   int               amount;
   qboolean          amount_override;


   void              ItemTouch(Event *ev);

public:
   str               itemname;

   CLASS_PROTOTYPE(Item);

   Item();
   ~Item();
   virtual void      CreateSpawnArgs();
   virtual void      PlaceItem();
   virtual void      SetOwner(Sentient *ent);
   virtual void      DropToFloor(Event *ev);
   virtual Item      *ItemPickup(Entity *other);
   virtual void      Respawn(Event *ev);
   virtual void      setRespawn(qboolean flag);
   virtual qboolean  Respawnable(void);
   virtual void      setRespawnTime(float time);
   virtual float     RespawnTime();
   virtual int       GetIconIndex() { return icon_index; };
   virtual int       GetItemIndex() { return item_index; };
   virtual int       Amount();
   virtual int       MaxAmount();
   virtual int       Icon();
   virtual qboolean  Pickupable(Entity *other);
   virtual void      setIcon(const char *i);
   virtual void      setName(const char *i);
   virtual void      SetAmount(Event *ev);
   virtual void      SetMaxAmount(Event *ev);
   virtual void      SetIconName(Event *ev);
   virtual void      SetItemName(Event *ev);
   virtual void      Set(int startamount);
   virtual void      SetMax(int maxamount);
   virtual void      Add(int num);
   virtual void      Remove(int num);
   virtual qboolean  Use(int amount);
   virtual qboolean  Removable();
   virtual void      Pickup(Event *ev);
   virtual qboolean  Drop();
   virtual void      RespawnSound(Event *ev);
   virtual void      DialogNeeded(Event *ev);
   virtual str       GetDialogNeeded();
   virtual void      ClearOwner() { owner = nullptr; }
   virtual void      Archive(Archiver &arc)   override;
   virtual void      Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void Item::Archive(Archiver &arc)
{
   Trigger::Archive(arc);

   arc.WriteSafePointer(owner);
   arc.WriteBoolean(respawnable);
   arc.WriteBoolean(playrespawn);
   arc.WriteFloat(respawntime);
   arc.WriteString(icon_name);
   arc.WriteString(dialog_needed);
   arc.WriteString(item_name);
   arc.WriteInteger(maximum_amount);
   arc.WriteInteger(amount);
   arc.WriteBoolean(amount_override);
   arc.WriteString(itemname);
}

inline EXPORT_FROM_DLL void Item::Unarchive(Archiver &arc)
{
   Trigger::Unarchive(arc);

   arc.ReadSafePointer(&owner);
   arc.ReadBoolean(&respawnable);
   arc.ReadBoolean(&playrespawn);
   arc.ReadFloat(&respawntime);

   arc.ReadString(&icon_name);
   icon_index = gi.imageindex(icon_name.c_str());

   arc.ReadString(&dialog_needed);

   arc.ReadString(&item_name);
   item_index = gi.itemindex(item_name.c_str());

   arc.ReadInteger(&maximum_amount);
   arc.ReadInteger(&amount);
   arc.ReadBoolean(&amount_override);
   arc.ReadString(&itemname);
}

#endif /* item.h */

// EOF

