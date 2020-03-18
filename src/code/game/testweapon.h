//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/testweapon.h                     $
// $Revision:: 3                                                              $
//   $Author:: Jimdose                                                        $
//     $Date:: 5/25/98 6:47p                                                  $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Weapon for testing view models
// 

#ifndef __TESTWEAPON_H__
#define __TESTWEAPON_H__

#include "g_local.h"
#include "item.h"
#include "weapon.h"

class EXPORT_FROM_DLL TestWeapon : public Weapon
{
public:
   CLASS_PROTOTYPE(TestWeapon);

   TestWeapon();
   virtual void Prethink() override;
   virtual void Shoot(Event *ev);
   virtual void Done(Event *ev);
};

#endif /* testweapon.h */

// EOF

