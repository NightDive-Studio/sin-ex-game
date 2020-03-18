//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/ctf_turret.h                     $
// $Revision:: 6                                                              $
//   $Author:: Aldie                                                          $
//     $Date:: 3/19/99 5:13p                                                  $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Fires a spear.  Used by Seabonites.
// 

#ifndef __CTF_TURRET_H__
#define __CTF_TURRET_H__

#include "g_local.h"
#include "vehicle.h"
#include "heligun.h"
#include "bullet.h"
#include "sentient.h"


class EXPORT_FROM_DLL CTFTurret : public DrivableVehicle
{
public:
   CLASS_PROTOTYPE(CTFTurret);

   CTFTurret();
   virtual void      DriverUse(Event *ev);
   virtual float     SetDriverPitch(float pitch);
};


class EXPORT_FROM_DLL CTFTurretGun : public HeliGun
{
public:
   CLASS_PROTOTYPE(CTFTurretGun);

   CTFTurretGun();
   virtual void      Shoot(Event *ev) override;
   virtual qboolean  IsDroppable()    override;

};

class EXPORT_FROM_DLL CTFDrivableTurret : public DrivableVehicle
{
private:
   Vector            baseangles;
   float             maxpitch;
   float             maxyaw;
   float             entertime;
   float             exittime;
   int               lastbutton;
   float             shotdamage;
   Sentient          *gunholder;
   BulletWeapon      *gun;

public:
   CLASS_PROTOTYPE(CTFDrivableTurret);

   CTFDrivableTurret();
   virtual qboolean  Drive(usercmd_t *ucmd)      override;
   virtual void      Postthink()                 override;
   virtual void      Fire();
   virtual float     SetDriverPitch(float pitch) override;
   virtual void      DriverUse(Event *ev)        override;
};

#endif /* ctfturret.h */

// EOF

