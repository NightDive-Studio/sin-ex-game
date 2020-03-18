//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/vehicle.h                        $
// $Revision:: 31                                                             $
//   $Author:: Markd                                                          $
//     $Date:: 3/05/99 5:51p                                                  $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Script controlled vehicles.
// 

#ifndef __VEHICLE_H__
#define __VEHICLE_H__

#include "g_local.h"
#include "entity.h"
#include "sentient.h"
#include "scriptslave.h"

extern Event EV_Vehicle_Enter;
extern Event EV_Vehicle_Exit;
extern Event EV_Vehicle_Drivable;
extern Event EV_Vehicle_UnDrivable;
extern Event EV_Vehicle_Lock;
extern Event EV_Vehicle_UnLock;
extern Event EV_Vehicle_SeatAnglesOffset;
extern Event EV_Vehicle_SeatOffset;
extern Event EV_Vehicle_DriverAnimation;
extern Event EV_Vehicle_SetWeapon;
extern Event EV_Vehicle_ShowWeapon;
extern Event EV_Vehicle_SetSpeed;
extern Event EV_Vehicle_SetTurnRate;

class EXPORT_FROM_DLL VehicleBase : public ScriptModel
{
public:
   VehicleBase *vlink  = nullptr;
   Vector       offset;

   CLASS_PROTOTYPE(VehicleBase);

   VehicleBase();
   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void VehicleBase::Archive(Archiver &arc)
{
   ScriptModel::Archive(arc);
   arc.WriteObjectPointer(vlink);
   arc.WriteVector(offset);
}

inline EXPORT_FROM_DLL void VehicleBase::Unarchive(Archiver &arc)
{
   ScriptModel::Unarchive(arc);
   arc.ReadObjectPointer((Class **)&vlink);
   arc.ReadVector(&offset);
}

/*QUAKED script_wheelsback (0 .5 .8) ?
*/
class EXPORT_FROM_DLL BackWheels : public VehicleBase
{
public:
   CLASS_PROTOTYPE(BackWheels);
};

/*QUAKED script_wheelsfront (0 .5 .8) ?
*/
class EXPORT_FROM_DLL FrontWheels : public VehicleBase
{
public:
   CLASS_PROTOTYPE(FrontWheels)
};

class EXPORT_FROM_DLL Vehicle : public VehicleBase
{
protected:
   SentientPtr       driver       = nullptr;
   SentientPtr       lastdriver   = nullptr;
   float             maxturnrate;
   float             currentspeed = 0.0f;
   float             turnangle    = 0.0f;
   float             turnimpulse  = 0.0f;
   float             moveimpulse  = 0.0f;
   float             jumpimpulse  = 0.0f;
   float             speed;
   float             conesize     = 75.0f;
   float             maxtracedist;
   int               buttons;
   Vector            cmd_angles;
   str               weaponName;
   str               driveranim;
   Vector            last_origin;
   Vector            seatangles;
   Vector            seatoffset;
   Vector            driveroffset;
   Vector            Corners[4];

   qboolean          drivable     = false;
   qboolean          locked       = false;
   qboolean          hasweapon    = false;
   qboolean          showweapon   = false;
   qboolean          steerinplace = false;
   qboolean          jumpable     = false;

   // CTF only stuff
   Vector            startorigin;

   virtual void      WorldEffects();
   virtual void      CheckWater();
   virtual void      DriverUse(Event *ev);
   virtual void      VehicleStart(Event *ev);
   virtual void      VehicleTouched(Event *ev);
   virtual void      VehicleBlocked(Event *ev);
   virtual void      Postthink() override;
   virtual void      Drivable(Event *ev);
   virtual void      UnDrivable(Event *ev);
   virtual void      Jumpable(Event *ev);
   virtual void      SeatAnglesOffset(Event *ev);
   virtual void      SeatOffset(Event *ev);
   virtual void      SetDriverAngles(Vector angles);
   virtual void      Lock(Event *ev);
   virtual void      UnLock(Event *ev);
   virtual void      SetWeapon(Event *ev);
   virtual void      ShowWeaponEvent(Event *ev);
   virtual void      DriverAnimation(Event *ev);
   virtual void      SetSpeed(Event *ev);
   virtual void      SetTurnRate(Event *ev);
   virtual void      SteerInPlace(Event *ev);

public:

   CLASS_PROTOTYPE(Vehicle);

   Vehicle();

   virtual qboolean   Drive(usercmd_t *ucmd);
   virtual qboolean   HasWeapon();
   virtual qboolean   ShowWeapon();
   Sentient          *Driver();
   virtual qboolean   IsDrivable();
   virtual void       Archive(Archiver &arc)   override;
   virtual void       Unarchive(Archiver &arc) override;
   virtual float      SetDriverPitch(float pitch);
};

inline EXPORT_FROM_DLL void Vehicle::Archive(Archiver &arc)
{
   VehicleBase::Archive(arc);

   arc.WriteSafePointer(driver);
   arc.WriteSafePointer(lastdriver);
   arc.WriteFloat(maxturnrate);
   arc.WriteFloat(currentspeed);
   arc.WriteFloat(turnangle);
   arc.WriteFloat(turnimpulse);
   arc.WriteFloat(moveimpulse);
   arc.WriteFloat(jumpimpulse);
   arc.WriteFloat(speed);
   arc.WriteFloat(conesize);
   arc.WriteFloat(maxtracedist);
   arc.WriteString(weaponName);
   arc.WriteString(driveranim);
   arc.WriteVector(last_origin);
   arc.WriteVector(seatangles);
   arc.WriteVector(seatoffset);
   arc.WriteVector(driveroffset);

   arc.WriteVector(Corners[0]);
   arc.WriteVector(Corners[1]);
   arc.WriteVector(Corners[2]);
   arc.WriteVector(Corners[3]);

   arc.WriteBoolean(drivable);
   arc.WriteBoolean(locked);
   arc.WriteBoolean(hasweapon);
   arc.WriteBoolean(showweapon);
   arc.WriteBoolean(steerinplace);
   arc.WriteBoolean(jumpable);
}

inline EXPORT_FROM_DLL void Vehicle::Unarchive(Archiver &arc)
{
   VehicleBase::Unarchive(arc);
   arc.ReadSafePointer(&driver);
   arc.ReadSafePointer(&lastdriver);
   arc.ReadFloat(&maxturnrate);
   arc.ReadFloat(&currentspeed);
   arc.ReadFloat(&turnangle);
   arc.ReadFloat(&turnimpulse);
   arc.ReadFloat(&moveimpulse);
   arc.ReadFloat(&jumpimpulse);
   arc.ReadFloat(&speed);
   arc.ReadFloat(&conesize);
   arc.ReadFloat(&maxtracedist);
   arc.ReadString(&weaponName);
   arc.ReadString(&driveranim);
   arc.ReadVector(&last_origin);
   arc.ReadVector(&seatangles);
   arc.ReadVector(&seatoffset);
   arc.ReadVector(&driveroffset);

   arc.ReadVector(&Corners[0]);
   arc.ReadVector(&Corners[1]);
   arc.ReadVector(&Corners[2]);
   arc.ReadVector(&Corners[3]);

   arc.ReadBoolean(&drivable);
   arc.ReadBoolean(&locked);
   arc.ReadBoolean(&hasweapon);
   arc.ReadBoolean(&showweapon);
   arc.ReadBoolean(&steerinplace);
   arc.ReadBoolean(&jumpable);
}

class EXPORT_FROM_DLL DrivableVehicle : public Vehicle
{
public:
   float respawntime = 0.0f;

   CLASS_PROTOTYPE(DrivableVehicle);

   DrivableVehicle();

   virtual void Killed(Event *ev);
   virtual void Respawn(Event *ev);
};

#ifdef EXPORT_TEMPLATE
template class EXPORT_FROM_DLL   SafePtr<Vehicle>;
#endif
typedef SafePtr<Vehicle>         VehiclePtr;

#endif /* vehicle.h */

// EOF

