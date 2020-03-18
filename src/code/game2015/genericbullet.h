//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/genericbullet.h                  $
// $Revision:: 6                                                              $
//   $Author:: Aldie                                                          $
//     $Date:: 10/23/98 5:09a                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Generic Bullet Weapon.
// 

#ifndef __GENERIC_BULLET_H__
#define __GENERIC_BULLET_H__

#include "g_local.h"
#include "item.h"
#include "weapon.h"
#include "bullet.h"

class EXPORT_FROM_DLL GenericBullet : public BulletWeapon
{
public:
   CLASS_PROTOTYPE(GenericBullet);

   GenericBullet();
   virtual void Shoot(Event *ev);
};

class EXPORT_FROM_DLL ReconahGun : public GenericBullet
{
public:
   CLASS_PROTOTYPE(ReconahGun);

   virtual void Shoot(Event *ev);
};

class EXPORT_FROM_DLL BeeGun : public GenericBullet
{
public:
   CLASS_PROTOTYPE(BeeGun);

   virtual void Shoot(Event *ev);
};

#endif /* generic bullet.h */

// EOF

