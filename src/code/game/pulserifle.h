//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/pulserifle.h                     $
// $Revision:: 14                                                             $
//   $Author:: Markd                                                          $
//     $Date:: 10/09/98 11:56p                                                $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Pulse rifle
// 

#ifndef __PULSERIFLE_H__
#define __PULSERIFLE_H__

#include "g_local.h"
#include "item.h"
#include "weapon.h"
#include "bullet.h"
#include "specialfx.h"

class EXPORT_FROM_DLL PulseRifle : public BulletWeapon
{
   int      beam_modelindex;

public:
   CLASS_PROTOTYPE(PulseRifle);

   PulseRifle();
   virtual void      Shoot(Event *ev);
   void              TraceAttack(Vector start, Vector end, int damage, trace_t *trace, int numricochets,
                                 int kick, int dflags);
   void              PulseExplosion(trace_t *trace);
   virtual void      Archive(Archiver &arc)   override;
   virtual void      Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void PulseRifle::Archive(Archiver &arc)
{
   BulletWeapon::Archive(arc);

   arc.WriteInteger(beam_modelindex);
}

inline EXPORT_FROM_DLL void PulseRifle::Unarchive(Archiver &arc)
{
   BulletWeapon::Unarchive(arc);

   arc.ReadInteger(&beam_modelindex);
}

class EXPORT_FROM_DLL GenericPulseRifle : public PulseRifle
{
public:
   CLASS_PROTOTYPE(GenericPulseRifle);

   GenericPulseRifle();
};

#endif /* pulserifle.h */

// EOF

