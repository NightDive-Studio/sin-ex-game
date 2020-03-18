//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/rocketlauncher.h                 $
// $Revision:: 20                                                             $
//   $Author:: Markd                                                          $
//     $Date:: 9/22/98 12:49p                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Standard rocketlauncher, similar to the Quake and Doom rocketlaunchers.
// 

#ifndef __ROCKETLAUNCHER_H__
#define __ROCKETLAUNCHER_H__

#include "g_local.h"
#include "item.h"
#include "weapon.h"
#include "specialfx.h"

class EXPORT_FROM_DLL Rocket : public Projectile
{
private:
   float             speed;

public:
   CLASS_PROTOTYPE(Rocket);

   void              Explode(Event *ev);
   void              Setup(Entity *owner, Vector pos, Vector dir);
   virtual void      Archive(Archiver &arc)   override;
   virtual void      Unarchive(Archiver &arc) override;
};


inline EXPORT_FROM_DLL void Rocket::Archive(Archiver &arc)
{
   Projectile::Archive(arc);

   arc.WriteFloat(speed);
}

inline EXPORT_FROM_DLL void Rocket::Unarchive(Archiver &arc)
{
   Projectile::Unarchive(arc);

   arc.ReadFloat(&speed);
}

class EXPORT_FROM_DLL RocketLauncher : public Weapon
{
public:
   CLASS_PROTOTYPE(RocketLauncher);

   RocketLauncher();
   virtual void Shoot(Event *ev);
   virtual void SecondaryUse(Event *ev) override; //### for guided missile
};

#endif /* rocketlauncher.h */

// EOF

