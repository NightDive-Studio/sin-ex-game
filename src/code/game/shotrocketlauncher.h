//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/shotrocketlauncher.h             $
// $Revision:: 11                                                             $
//   $Author:: Jimdose                                                        $
//     $Date:: 10/17/98 8:12p                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Shootable rockets and a launcher

#ifndef __SHOTROCKETLAUNCHER_H__
#define __SHOTROCKETLAUNCHER_H__

#include "g_local.h"
#include "item.h"
#include "weapon.h"
#include "specialfx.h"

class EXPORT_FROM_DLL ShotRocket : public Projectile
{
private:
   float             speed;

public:
   CLASS_PROTOTYPE(ShotRocket);

   void              DamageEvent(Event *ev);
   void              Explode(Event *ev);
   void              Setup(Entity *owner, Vector pos, Vector dir);
   void              HeatSeek(Event *ev);
   float             Distance(Entity *targ);
   qboolean          CanSee(Entity *ent);
   virtual void      Archive(Archiver &arc)   override;
   virtual void      Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void ShotRocket::Archive(Archiver &arc)
{
   Projectile::Archive(arc);
   arc.WriteFloat(speed);
}

inline EXPORT_FROM_DLL void ShotRocket::Unarchive(Archiver &arc)
{
   Projectile::Unarchive(arc);
   arc.ReadFloat(&speed);
}

class EXPORT_FROM_DLL ShotRocketLauncher : public Weapon
{
public:
   CLASS_PROTOTYPE(ShotRocketLauncher);

   ShotRocketLauncher();
   virtual void Shoot(Event *ev);
};

#endif // shotrocketlauncher.h

// EOF

