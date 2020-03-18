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
   virtual qboolean  Drop()                   override;
   //### for dual magnum
   virtual void      SecondaryUse(Event *ev)  override;
   virtual void      ReadyWeapon()            override;

   qboolean          ammosynced = false; // for syncing ammo with dual magnums

   virtual void      Archive(Archiver &arc)   override;
   virtual void      Unarchive(Archiver &arc) override;
   //### 
};

//###
inline EXPORT_FROM_DLL void Magnum::Archive(Archiver &arc)
{
   BulletWeapon::Archive(arc);

   arc.WriteBoolean(ammosynced);
}

inline EXPORT_FROM_DLL void Magnum::Unarchive(Archiver &arc)
{
   BulletWeapon::Unarchive(arc);

   arc.ReadBoolean(&ammosynced);
}
//###

#endif /* magnum.h */

// EOF

