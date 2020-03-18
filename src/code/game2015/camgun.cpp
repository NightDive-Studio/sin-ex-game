//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/camgun.cpp                       $
// $Revision:: 18                                                             $
//   $Author:: Markd                                                          $
//     $Date:: 10/15/98 3:39p                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Generic turret

#include "turret.h"
#include "weapon.h"

class EXPORT_FROM_DLL Camgun : public Turret
{
private:
   Vector         zeroangle;
   float          yawrange;
   float          pitchrange;
   float          maxpitch;
   float          maxyaw;
   Vector         new_orientation;

public:
   CLASS_PROTOTYPE(Camgun);

   Camgun();
   virtual void   Seek(Event *ev)          override;
   virtual void   Turn(Event *ev)          override;
   virtual void   Down(Event *ev)          override;
   virtual void   ClampOrientation();
   virtual void   Archive(Archiver &arc)   override;
   virtual void   Unarchive(Archiver &arc) override;
};

EXPORT_FROM_DLL void Camgun::Archive(Archiver &arc)
{
   Turret::Archive(arc);

   arc.WriteVector(zeroangle);
   arc.WriteFloat(yawrange);
   arc.WriteFloat(pitchrange);
   arc.WriteFloat(maxpitch);
   arc.WriteFloat(maxyaw);
   arc.WriteVector(new_orientation);
}

EXPORT_FROM_DLL void Camgun::Unarchive(Archiver &arc)
{
   Turret::Unarchive(arc);

   arc.ReadVector(&zeroangle);
   arc.ReadFloat(&yawrange);
   arc.ReadFloat(&pitchrange);
   arc.ReadFloat(&maxpitch);
   arc.ReadFloat(&maxyaw);
   arc.ReadVector(&new_orientation);
}

CLASS_DECLARATION(Turret, Camgun, "trap_camgun");

ResponseDef Camgun::Responses[] =
{
   { nullptr, nullptr }
};

Camgun::Camgun(void)
{
   setModel("camgun2.def");
   RandomAnimate("down_idle", nullptr);
   setMoveType(MOVETYPE_NONE);
   setSolidType(SOLID_BBOX);

   gunoffset      = { 0, 0, 0 };
   neworientation = angles.yaw();
   flags          |= FL_SPARKS;

   zeroangle = Vector(0, G_GetFloatArg("angle", 0), 0);

   if(zeroangle[YAW] > 180)
      zeroangle[YAW] -= 360;
   if(zeroangle[YAW] < -180)
      zeroangle[YAW] += 360;

   yawrange    = G_GetFloatArg("yawrange", 180);
   maxyaw      = yawrange;
   pitchrange  = G_GetFloatArg("pitchrange", 180);
   maxpitch    = pitchrange;

   wakeupdistance = G_GetFloatArg("wakeupdistance", 750);
   firingdistance = G_GetFloatArg("firingdistance", 800);
}

void Camgun::ClampOrientation()
{
   Vector delta;

   delta = new_orientation - zeroangle;

   if(delta[PITCH] > 180)
      delta[PITCH] -= 360;
   if(delta[PITCH] < -180)
      delta[PITCH] += 360;

   if(delta[PITCH] > maxpitch)
      delta[PITCH] = maxpitch;
   if(delta[PITCH] < -maxpitch)
      delta[PITCH] = -maxpitch;

   if(delta[YAW] > 180)
      delta[YAW] -= 360;
   if(delta[YAW] < -180)
      delta[YAW] += 360;

   if(delta[YAW] > maxyaw)
      delta[YAW] = maxyaw;
   if(delta[YAW] < -maxyaw)
      delta[YAW] = -maxyaw;

   new_orientation[PITCH] = zeroangle[PITCH] + delta[PITCH];
   new_orientation[YAW]   = zeroangle[YAW]   + delta[YAW];
}

void Camgun::Seek(Event *ev)
{
   Entity	*ent;
   Vector	v;
   int		range;

   active = true;
   ent = nullptr;
   if(enemy)
   {
      ent = G_GetEntity(enemy);
      if((!ent) || (ent->health <= 0) || (ent->flags & FL_NOTARGET) || (ent == this))
      {
         enemy = 0;
         ent = nullptr;
      }
      else
      {
         range = Range(Distance(ent));
      }
   }

   if((lastSightTime) && ((lastSightTime + patience) < level.time))
   {
      ProcessEvent(EV_Turret_GoDown);
   }

   if(!enemy)
   {
      FindTarget();
      PostEvent(EV_Turret_Seek, FRAMETIME * 2);
      return;
   }

   if((range != TURRET_OUTOFRANGE) && ent && CanSee(ent))
   {
      lastSightTime = level.time;
      v = ent->centroid - worldorigin;
      new_orientation = v.toAngles();
      ClampOrientation();
   }

   if((angles[YAW] != new_orientation[YAW]) && !turning)
   {
      Event *event;
      event = new Event(EV_Turret_Turn);
      event->AddVector(new_orientation);
      ProcessEvent(event);
   }

   if(range == TURRET_FIRERANGE && !attacking)
   {
      // Allow some freetime to let player get somewhere before turret shoots
      if(level.time < firetime)
      {
         PostEvent(EV_Turret_Seek, FRAMETIME);
         return;
      }
      PostEvent(EV_Turret_Attack, 0.1);
   }

   PostEvent(EV_Turret_Seek, FRAMETIME);
}

void Camgun::Turn(Event *ev)
{
   Vector new_angle = ev->GetVector(1);

   if(angles[YAW] != new_angle[YAW])
   {
      turntime = level.time + 0.2;
      turning = true;
   }
   else if(turntime < level.time)
   {
      turning = false;
      angles[PITCH] = -new_angle[PITCH];
      return;
   }

   angles[PITCH] = -new_angle[PITCH];
   angles[YAW]   =  new_angle[YAW];//AdjustAngle( 12, angles[ YAW ], new_angle[ YAW ] );	
   setAngles(angles);
   PostEvent(ev, FRAMETIME);
}

void Camgun::Down(Event *ev)
{
}

// EOF

