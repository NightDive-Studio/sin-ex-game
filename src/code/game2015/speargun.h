//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/speargun.h                       $
// $Revision:: 8                                                              $
//   $Author:: Aldie                                                          $
//     $Date:: 10/06/98 10:52p                                                $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Fires a spear.  Used by Seabonites.
// 

#ifndef __SPEARGUN_H__
#define __SPEARGUN_H__

#include "g_local.h"
#include "item.h"
#include "weapon.h"
#include "specialfx.h"

class EXPORT_FROM_DLL Spear : public Projectile
{
public:
   CLASS_PROTOTYPE(Spear);

   void Hit(Event *ev);
   virtual void Setup(Entity *owner, Vector pos, Vector dir) override;
};


class EXPORT_FROM_DLL SpearGun : public Weapon
{
public:
   CLASS_PROTOTYPE(SpearGun);

   SpearGun();
   virtual void Shoot(Event *ev);
};

#endif /* speargun.h */

// EOF

