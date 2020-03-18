//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/magnum.h                         $
// $Revision:: 13                                                             $
//   $Author:: Jimdose                                                        $
//     $Date:: 10/19/98 12:06a                                                $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Magnum.
// 

#ifndef __MAGNUM_H__
#define __MAGNUM_H__

#include "g_local.h"
#include "item.h"
#include "weapon.h"
#include "bullet.h"

class EXPORT_FROM_DLL Magnum : public BulletWeapon
{
public:
   CLASS_PROTOTYPE(Magnum);

   Magnum();
   virtual void      Shoot(Event *ev);
   virtual qboolean  Drop() override;
};

#endif /* magnum.h */

// EOF

