/*
================================================================
FLASHLIGHT
================================================================

Copyright (C) 2020 by Night Dive Studios, Inc.
All rights reserved.

See the license.txt file for conditions and terms of use for this code.
*/

#ifndef __FLASHLIGHT_H__
#define __FLASHLIGHT_H__

#include "g_local.h"
#include "inventoryitem.h"

class EXPORT_FROM_DLL Flashlight : public InventoryItem
{
private:
   EntityPtr lightent  = nullptr;
   EntityPtr lightent2 = nullptr;

public:
   int lighton = 0;

   CLASS_PROTOTYPE(Flashlight);

   Flashlight();
   ~Flashlight();
   virtual void      Use(Event *ev)            override;
   void              EmitLight(Event *ev);
   virtual qboolean  Pickupable(Entity *other) override;
   virtual Item     *ItemPickup(Entity *other) override;
   virtual void      Pickup(Event *ev)         override;

   virtual void      Archive(Archiver &arc)    override;
   virtual void      Unarchive(Archiver &arc)  override;
};

inline EXPORT_FROM_DLL void Flashlight::Archive(Archiver &arc)
{
   InventoryItem::Archive(arc);

   arc.WriteSafePointer(lightent);
   arc.WriteSafePointer(lightent2);
   arc.WriteInteger(lighton);
}

inline EXPORT_FROM_DLL void Flashlight::Unarchive(Archiver &arc)
{
   InventoryItem::Unarchive(arc);

   arc.ReadSafePointer(&lightent);
   arc.ReadSafePointer(&lightent2);
   arc.ReadInteger(&lighton);
}

#endif

// EOF

