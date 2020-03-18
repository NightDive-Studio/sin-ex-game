//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/heligun.h                           $
// $Revision:: 2                                                              $
//   $Author:: Markd                                                          $
//     $Date:: 7/08/98 11:55p                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// $Log:: /Quake 2 Engine/Sin/code/game/heligun.h                                $
// 
// 2     7/08/98 11:55p Markd
// First time
// 
// 1     7/08/98 11:33p Markd
// 
// DESCRIPTION:
// Helicopter gun
// 

#ifndef __HELIGUN_H__
#define __HELIGUN_H__

#include "g_local.h"
#include "item.h"
#include "weapon.h"
#include "bullet.h"
#include "misc.h"

class EXPORT_FROM_DLL HeliGun : public BulletWeapon
{
public:
   CLASS_PROTOTYPE(HeliGun);

   HeliGun();
   virtual void Shoot(Event *ev);
};

#endif /* HeliGun.h */

// EOF

