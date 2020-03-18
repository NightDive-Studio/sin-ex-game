//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/rocket_turret.cpp                $
// $Revision:: 14                                                             $
//   $Author:: Aldie                                                          $
//     $Date:: 9/22/98 4:56p                                                  $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Rocket Turret - Shoots slow moving shootable rockets at the player

#include "turret.h"
#include "rocket_turret.h"
#include "shotrocketlauncher.h"

CLASS_DECLARATION(Turret, RocketTurret, "trap_rocketturret");

Event EV_RocketTurret_Rate("rocketrate");
Event EV_RocketTurret_Speed("rocketspeed");

ResponseDef RocketTurret::Responses[] =
{
   { &EV_RocketTurret_Rate,         (Response)&RocketTurret::SetRate },
   { &EV_RocketTurret_Speed,        (Response)&RocketTurret::SetSpeed },
   { nullptr, nullptr }
};

RocketTurret::RocketTurret() : Turret()
{
   Entity *base;

   setModel("launcher_top.def");
   RandomAnimate("down_idle", NULL);
   neworientation = angles.yaw();
   setMoveType(MOVETYPE_NONE);
   setSolidType(SOLID_BBOX);
   flags |= FL_SPARKS;
   enemy_oldorigin = vec_zero;
   edict->svflags |= SVF_USEBBOX;

   base = new Entity();
   base->setModel("launcher_base.def");
   base->setMoveType(MOVETYPE_NONE);
   base->setSolidType(SOLID_BBOX);
   base->setOrigin(worldorigin);

   health = 150;
   takedamage = DAMAGE_YES;
   wakeupdistance = G_GetFloatArg("wakeupdistance", 1950);
   firingdistance = G_GetFloatArg("firingdistance", 2000);
   rate = 8.0;
   speed = 300;
}

void RocketTurret::Seek(Event *ev)
{
   Entity	*ent;
   Vector	v;
   int		range;
   Vector   enemy_velocity;

   active = true;
   ent = nullptr;
   if(enemy)
   {
      ent = G_GetEntity(enemy);
      if((!ent) || (ent->health <= 0) || (ent->flags & FL_NOTARGET) || (ent == this))
      {
         enemy = 0;
         enemy_oldorigin = vec_zero;
         ent = nullptr;
      }
      else
      {
         float delta_t;

         // Have an enemy locked on, get the range and calculate velocity
         // based on the old origin and the time difference since last locked
         // onto him.
         range = Range(Distance(ent));

         delta_t = level.time - lastSightTime;

         if(delta_t)
            enemy_velocity = (ent->worldorigin - enemy_oldorigin) * (1.0f / delta_t);
         else
            enemy_velocity = vec_zero;
         // Update the enemy's origin and the time we last saw him
         enemy_oldorigin = ent->worldorigin;
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
      Vector targ, src, d;

      float dist;

      lastSightTime = level.time;

      // Get distance and orientation to target
      targ = ent->centroid;
      currentWeapon->GetMuzzlePosition(&src);
      d = targ - src;
      dist = d.length();

      targ += (enemy_velocity * (dist / sv_rocketspeed->value));

      v = targ - src;
      v.normalize();

      new_orientation = v.toAngles();
   }

   if((angles[YAW] != new_orientation[YAW]) && !turning)
   {
      Event *event;
      event = new Event(EV_Turret_Turn);
      event->AddVector(new_orientation);
      ProcessEvent(event);
   }

   if(range == TURRET_FIRERANGE)
   {
      // Allow some freetime to let player get somewhere before turret fires on him
      if(level.time < firetime)
      {
         PostEvent(EV_Turret_Seek, FRAMETIME);
         return;
      }
      ProcessEvent(EV_Turret_Attack);
   }

   PostEvent(EV_Turret_Seek, FRAMETIME);
}

void RocketTurret::Turn(Event *ev)
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
   angles[YAW] = AdjustAngle(32, angles[YAW], new_angle[YAW]);
   setAngles(angles);
   PostEvent(ev, FRAMETIME);
}

void RocketTurret::SetRate(Event *ev)
{
   rate = ev->GetFloat(1);
}

void RocketTurret::SetSpeed(Event *ev)
{
   speed = ev->GetFloat(1);
}

// EOF

