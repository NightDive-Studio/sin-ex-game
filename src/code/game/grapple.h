//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/grapple.h                        $
// $Revision:: 1                                                              $
//   $Author:: Aldie                                                          $
//     $Date:: 3/02/99 9:24p                                                  $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Grappling Hook

#ifndef __GRAPPLE_H__
#define __GRAPPLE_H__

#include "g_local.h"
#include "item.h"
#include "weapon.h"
#include "specialfx.h"

class EXPORT_FROM_DLL Hook : public Projectile
{
private:
   float					speed;

public:
   CLASS_PROTOTYPE(Hook);

   virtual void      Setup(Entity *owner, Vector pos, Vector dir) override;
   virtual void      Archive(Archiver &arc)   override;
   virtual void      Unarchive(Archiver &arc) override;
   virtual void      Touch(Event *ev);
};

inline EXPORT_FROM_DLL void Hook::Archive(Archiver &arc)
{
   Projectile::Archive(arc);

   arc.WriteFloat(speed);
}

inline EXPORT_FROM_DLL void Hook::Unarchive(Archiver &arc)
{
   Projectile::Unarchive(arc);

   arc.ReadFloat(&speed);
}

typedef SafePtr<Hook> HookPtr;
typedef SafePtr<Beam> BeamPtr;

class EXPORT_FROM_DLL Grapple : public Weapon
{
private:
   HookPtr           hook;
   BeamPtr           beam;

public:
   CLASS_PROTOTYPE(Grapple);

   Grapple();
   ~Grapple();
   virtual void      Shoot(Event *ev);
   virtual qboolean  HasAmmo()            override;
   virtual void      ReleaseFire(float t) override;
   virtual void      UpdateBeam(Event *ev);
};

#endif /* grapple.h */

// EOF

