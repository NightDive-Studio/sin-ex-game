//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/silencer.h                       $
// $Revision:: 5                                                              $
//   $Author:: Markd                                                          $
//     $Date:: 9/21/98 5:01p                                                  $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Silencer Powerup

#ifndef __SILENCER_H__
#define __SILENCER_H__

#include "inventoryitem.h" 

class EXPORT_FROM_DLL Silencer : public InventoryItem
{
public:
   CLASS_PROTOTYPE(Silencer);
   Silencer();
   void PickupSilencer(Event *ev);
};

#endif /* armor.h */

// EOF

