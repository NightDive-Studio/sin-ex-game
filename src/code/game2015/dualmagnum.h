/*
================================================================
DUAL MAGNUMS
================================================================

Copyright (C) 2020 by Night Dive Studios, Inc.
All rights reserved.

See the license.txt file for conditions and terms of use for this code.

This weapon is given to the player when he picks up a second magnum
*/

#ifndef __DUALMAGNUM_H__
#define __DUALMAGNUM_H__

#include "g_local.h"
#include "item.h"
#include "weapon.h"
#include "bullet.h"

class EXPORT_FROM_DLL DualMagnum : public BulletWeapon
{
public:
   CLASS_PROTOTYPE(DualMagnum);

   qboolean ammosynced = false; // for syncing ammo with a single magnum

   DualMagnum();
   virtual void     ReadyWeapon()            override;
   virtual void     Shoot(Event *ev);
   virtual void     SecondaryUse(Event *ev)  override;
   virtual qboolean Drop()                   override;

   virtual void     Archive(Archiver &arc)   override;
   virtual void     Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void DualMagnum::Archive(Archiver &arc)
{
   BulletWeapon::Archive(arc);

   arc.WriteBoolean(ammosynced);
}

inline EXPORT_FROM_DLL void DualMagnum::Unarchive(Archiver &arc)
{
   BulletWeapon::Unarchive(arc);

   arc.ReadBoolean(&ammosynced);
}

#endif /* dualmagnum.h */

// EOF

