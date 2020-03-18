//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/rocket_turret.h                  $
// $Revision:: 5                                                              $
//   $Author:: Markd                                                          $
//     $Date:: 9/22/98 12:49p                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Rocket based turret

#ifndef __ROCKET_TURRET_H__
#define __ROCKET_TURRET_H__

#include "turret.h"

class EXPORT_FROM_DLL RocketTurret : public Turret
{
protected:
   Vector   new_orientation;
   Vector   enemy_oldorigin;

public:
   CLASS_PROTOTYPE(RocketTurret);

   float          speed;
   float          rate;

   RocketTurret();
   virtual void   Seek(Event *ev);
   virtual void   Turn(Event *ev);
   virtual void   SetSpeed(Event *ev);
   virtual void   SetRate(Event *ev);
   virtual void   Archive(Archiver &arc)   override;
   virtual void   Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void RocketTurret::Archive(Archiver &arc)
{
   Turret::Archive(arc);

   arc.WriteVector(new_orientation);
   arc.WriteVector(enemy_oldorigin);
   arc.WriteFloat(speed);
   arc.WriteFloat(rate);
}

inline EXPORT_FROM_DLL void RocketTurret::Unarchive(Archiver &arc)
{
   Turret::Unarchive(arc);

   arc.ReadVector(&new_orientation);
   arc.ReadVector(&enemy_oldorigin);
   arc.ReadFloat(&speed);
   arc.ReadFloat(&rate);
}

#endif // rocket_turret

// EOF

