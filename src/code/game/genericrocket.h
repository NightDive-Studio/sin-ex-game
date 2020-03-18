//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/genericrocket.h                  $
// $Revision:: 3                                                              $
//   $Author:: Markd                                                          $
//     $Date:: 10/04/98 10:26p                                                $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Generic Rocket Launcher - to be used on monsters that have rocket launchers
// shown in their models

#ifndef __GENERIC_ROCKET_H__
#define __GENERIC_ROCKET_H__

#include "g_local.h"
#include "item.h"
#include "weapon.h"
#include "rocketlauncher.h"

class EXPORT_FROM_DLL GenericRocket : public RocketLauncher
{
public:
   CLASS_PROTOTYPE(GenericRocket);

   GenericRocket();
   virtual void Shoot(Event *ev);
};

#endif /* genericrocket.h */

// EOF

