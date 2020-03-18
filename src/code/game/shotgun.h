//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/shotgun.h                        $
// $Revision:: 4                                                              $
//   $Author:: Jimdose                                                        $
//     $Date:: 4/04/98 6:12p                                                  $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Shotgun
// 

#ifndef __SHOTGUN_H__
#define __SHOTGUN_H__

#include "g_local.h"
#include "item.h"
#include "weapon.h"
#include "bullet.h"

class EXPORT_FROM_DLL Shotgun : public BulletWeapon
{
public:
   CLASS_PROTOTYPE(Shotgun);

   Shotgun();
   virtual void Shoot(Event *ev);
};

#endif /* shotgun.h */

// EOF

