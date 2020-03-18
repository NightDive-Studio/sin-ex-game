//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/quantumd.h                       $
// $Revision:: 17                                                             $
//   $Author:: Jimdose                                                        $
//     $Date:: 11/08/98 10:54p                                                $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Quantum Destabilizer

#ifndef __QUANTUMD_H__
#define __QUANTUMD_H__

#include "g_local.h"
#include "item.h"
#include "weapon.h"
#include "bullet.h"

class EXPORT_FROM_DLL QuantumDestabilizer : public BulletWeapon
{
private:
   float             power;
   SentientPtr       trapped_sent;

public:
   CLASS_PROTOTYPE(QuantumDestabilizer);

   QuantumDestabilizer();
   virtual void      Shoot(Event *ev);
   virtual void      ReleaseFire(float holdfiretime) override;
   void              EatAmmo(Event *ev);
   void              Destruct(Event *ev);
   void              StartSelfDestruct(Event *ev);
   void              SuckSentient(Vector pos, Vector org);
   qboolean          ShootSentient(Vector pos, Vector dir);
   void              SentientOverload(Event *ev);
   void              TraceAttack(Vector start,
                                 Vector end,
                                 int damage,
                                 trace_t	*trace,
                                 int numricochets,
                                 int kick,
                                 int dflags);
   virtual qboolean  Drop()                   override;
   virtual void      Archive(Archiver &arc)   override;
   virtual void      Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void QuantumDestabilizer::Archive(Archiver &arc)
{
   BulletWeapon::Archive(arc);

   arc.WriteFloat(power);
   arc.WriteSafePointer(trapped_sent);
}

inline EXPORT_FROM_DLL void QuantumDestabilizer::Unarchive(Archiver &arc)
{
   BulletWeapon::Unarchive(arc);

   arc.ReadFloat(&power);
   arc.ReadSafePointer(&trapped_sent);
}

#endif /* quantumd.h */

// EOF

