//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/inventoryitem.h                  $
// $Revision:: 5                                                              $
//   $Author:: Aldie                                                          $
//     $Date:: 10/09/98 2:07a                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Items that are visible in the player's inventory


#ifndef __INVITEM_H__
#define __INVITEM_H__

#include "item.h"

class EXPORT_FROM_DLL InventoryItem : public Item
{
public:
   CLASS_PROTOTYPE(InventoryItem);

   InventoryItem();
   virtual void   Use(Event *ev);
};

extern Event EV_InventoryItem_Use;

#endif /* inventoryitem.h */

// EOF

