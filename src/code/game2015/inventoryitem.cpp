//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/inventoryitem.cpp                $
// $Revision:: 7                                                              $
//   $Author:: Aldie                                                          $
//     $Date:: 10/09/98 2:06a                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Inventory items

#include "inventoryitem.h"

CLASS_DECLARATION(Item, InventoryItem, nullptr);

Event EV_InventoryItem_Use("useinvitem");

ResponseDef InventoryItem::Responses[] =
{
   { &EV_InventoryItem_Use,   (Response)&InventoryItem::Use },
   { nullptr, nullptr }
};

InventoryItem::InventoryItem() : Item()
{
   // All powerups are inventory items
   if(DM_FLAG(DF_NO_POWERUPS))
   {
      PostEvent(EV_Remove, 0);
      return;
   }
}

void InventoryItem::Use(Event *ev)
{
}

// EOF

