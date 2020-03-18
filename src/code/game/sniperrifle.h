//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/sniperrifle.h                    $
// $Revision:: 11                                                             $
//   $Author:: Aldie                                                          $
//     $Date:: 3/02/99 9:16p                                                  $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Sniper rifle
// 

#ifndef __SNIPERRIFLE_H__
#define __SNIPERRIFLE_H__

#include "g_local.h"
#include "item.h"
#include "weapon.h"

class EXPORT_FROM_DLL SniperRifle : public BulletWeapon
{
public:
   CLASS_PROTOTYPE(SniperRifle);

   SniperRifle();
   virtual void      Shoot(Event *ev);
   virtual void      DoneRaising(Event *ev)  override;
   virtual void      Open(Event *ev);
   virtual void      SecondaryUse(Event *ev) override;
   virtual void      DoneLowering(Event *ev) override;
   virtual qboolean  AutoChange()            override;
   virtual void      ReleaseFire(float time) override;
};

#endif /* sniperrifle.h */

// EOF

