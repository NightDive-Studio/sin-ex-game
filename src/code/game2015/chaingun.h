//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/chaingun.h                          $
// $Revision:: 10                                                             $
//   $Author:: Aldie                                                          $
//     $Date:: 3/19/99 3:43p                                                  $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
// 
// DESCRIPTION:
// High Velocity Gun
// 

#ifndef __CHAINGUN_H__
#define __CHAINGUN_H__

#include "g_local.h"
#include "item.h"
#include "weapon.h"
#include "bullet.h"
#include "specialfx.h"

class EXPORT_FROM_DLL Grenade : public Projectile
{
public:
   CLASS_PROTOTYPE(Grenade);

   virtual void      Explode(Event *ev);
   virtual void      Grenade_Touch(Event *ev);
   void              Setup(Entity *owner, Vector pos, Vector forward, Vector up, Vector right);

};

class EXPORT_FROM_DLL ChainGun : public BulletWeapon
{
public:
   CLASS_PROTOTYPE(ChainGun);

   ChainGun();
   virtual void      Shoot(Event *ev);
   virtual qboolean  HasAmmo(Event *ev);
};

#endif /* ChainGun.h */

// EOF

