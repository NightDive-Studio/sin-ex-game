//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/secgun.h                            $
// $Revision:: 2                                                              $
//   $Author:: Markd                                                          $
//     $Date:: 10/21/98 12:05a                                                $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Securton Gun
// 

#ifndef __SECGUN_H__
#define __SECGUN_H__

#include "g_local.h"
#include "item.h"
#include "weapon.h"
#include "genericbullet.h"

class EXPORT_FROM_DLL Secgun : public GenericBullet
{
public:
   CLASS_PROTOTYPE(Secgun);

   Secgun();
   virtual void Shoot(Event *ev);
};

#endif /* ChainGun.h */

// EOF

