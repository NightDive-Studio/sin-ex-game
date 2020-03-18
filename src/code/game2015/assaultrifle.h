//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/assaultrifle.h                   $
// $Revision:: 7                                                              $
//   $Author:: Aldie                                                          $
//     $Date:: 10/05/98 10:38p                                                $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Assault rifle
// 

#ifndef __ASSAULTRIFLE_H__
#define __ASSAULTRIFLE_H__

#include "g_local.h"
#include "item.h"
#include "weapon.h"
#include "bullet.h"

class EXPORT_FROM_DLL AssaultRifle : public BulletWeapon
{
public:
   CLASS_PROTOTYPE(AssaultRifle);

   AssaultRifle();
   virtual void Shoot(Event *ev);
   virtual void SecondaryUse(Event *ev) override; //###
};

#endif /* assaultrifle.h */

// EOF

