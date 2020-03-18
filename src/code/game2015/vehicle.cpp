//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/vehicle.cpp                      $
// $Revision:: 56                                                             $
//   $Author:: Aldie                                                          $
//     $Date:: 3/17/99 4:00p                                                  $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Script controlled Vehicles.
// 

#include "g_local.h"
#include "scriptslave.h"
#include "vehicle.h"
#include "player.h"
#include "specialfx.h"
#include "explosion.h"
#include "earthquake.h"
#include "gibs.h"
#include "item.h"

Event EV_Vehicle_Start("start");
Event EV_Vehicle_Enter("enter");
Event EV_Vehicle_Exit("exit");
Event EV_Vehicle_Drivable("drivable");
Event EV_Vehicle_UnDrivable("undrivable");
Event EV_Vehicle_Jumpable("canjump");
Event EV_Vehicle_Lock("lock");
Event EV_Vehicle_UnLock("unlock");
Event EV_Vehicle_SeatAnglesOffset("seatanglesoffset");
Event EV_Vehicle_SeatOffset("seatoffset");
Event EV_Vehicle_DriverAnimation("driveranim");
Event EV_Vehicle_SetWeapon("setweapon");
Event EV_Vehicle_SetSpeed("vehiclespeed");
Event EV_Vehicle_SetTurnRate("turnrate");
Event EV_Vehicle_SteerInPlace("steerinplace");
Event EV_Vehicle_ShowWeapon("showweapon");

CLASS_DECLARATION(ScriptModel, VehicleBase, NULL);

ResponseDef VehicleBase::Responses[] =
{
   { NULL, NULL }
};

VehicleBase::VehicleBase() : ScriptModel()
{
   takedamage = DAMAGE_NO;
   showModel();
   setSolidType(SOLID_NOT);
   setMoveType(MOVETYPE_NONE);
   setOrigin(origin + Vector(0, 0, 30));

   //
   // we want the bounds of this model auto-rotated
   //
   flags |= FL_ROTATEDBOUNDS;

   //
   // rotate the mins and maxs for the model
   //
   setSize(mins, maxs);
}

CLASS_DECLARATION( VehicleBase, BackWheels, "script_wheelsback" );

ResponseDef BackWheels::Responses[] =
{
   { NULL, NULL }
};

CLASS_DECLARATION(VehicleBase, FrontWheels, "script_wheelsfront");

ResponseDef FrontWheels::Responses[] =
{
   { NULL, NULL }
};

CLASS_DECLARATION( VehicleBase, Vehicle, "script_vehicle" );

ResponseDef Vehicle::Responses[] =
{
   { &EV_Blocked,                   (Response)&Vehicle::VehicleBlocked },
   { &EV_Touch,                     (Response)&Vehicle::VehicleTouched },
   { &EV_Use,                       (Response)&Vehicle::DriverUse },
   { &EV_Vehicle_Start,             (Response)&Vehicle::VehicleStart },
   { &EV_Vehicle_Drivable,          (Response)&Vehicle::Drivable },
   { &EV_Vehicle_UnDrivable,        (Response)&Vehicle::UnDrivable },
   { &EV_Vehicle_Jumpable,          (Response)&Vehicle::Jumpable },
   { &EV_Vehicle_SeatAnglesOffset,  (Response)&Vehicle::SeatAnglesOffset },
   { &EV_Vehicle_SeatOffset,        (Response)&Vehicle::SeatOffset },
   { &EV_Vehicle_Lock,              (Response)&Vehicle::Lock },
   { &EV_Vehicle_UnLock,            (Response)&Vehicle::UnLock },
   { &EV_Vehicle_SetWeapon,         (Response)&Vehicle::SetWeapon },
   { &EV_Vehicle_DriverAnimation,   (Response)&Vehicle::DriverAnimation },
   { &EV_Vehicle_SetSpeed,          (Response)&Vehicle::SetSpeed },
   { &EV_Vehicle_SetTurnRate,       (Response)&Vehicle::SetTurnRate },
   { &EV_Vehicle_SteerInPlace,      (Response)&Vehicle::SteerInPlace },
   { &EV_Vehicle_ShowWeapon,        (Response)&Vehicle::ShowWeaponEvent },
   { NULL, NULL }
};

Vehicle::Vehicle() : VehicleBase()
{
   takedamage   = DAMAGE_YES;
   setSolidType(SOLID_BBOX);
   flags |= FL_SPARKS | FL_DIE_TESSELATE | FL_DIE_EXPLODE | FL_DARKEN;
   gravity = 1;
   mass = size.length() * 10;

   // 
   // we use this to signify the init state of the angles
   //
   health = G_GetFloatArg("health", 1000);
   speed = G_GetFloatArg("speed", 600);
   maxturnrate = G_GetFloatArg("maxturnrate", 40.0f);
   PostEvent(EV_Vehicle_Start, 0.2f);
}

void Vehicle::VehicleStart(Event *ev)
{
   Entity      *ent;
   VehicleBase *last;
   vec3_t       trans[3];
   vec3_t       orient;
   Vector       drivemins, drivemaxs;
   int          groupindex;
   int          tri_num;
   float        max;
   float        width, height;

   last = this;

   for(ent = G_NextEntity(world); ent != NULL; ent = G_NextEntity(ent))
   {
      if(ent != this && ent->isSubclassOf<VehicleBase>())
      {
         if((ent->absmax.x >= absmin.x) && (ent->absmax.y >= absmin.y) && (ent->absmax.z >= absmin.z) &&
            (ent->absmin.x <= absmax.x) && (ent->absmin.y <= absmax.y) && (ent->absmin.z <= absmax.z))
         {
            last->vlink = (VehicleBase *)ent;
            last = (VehicleBase *)ent;
            last->offset = last->worldorigin - worldorigin;
            last->offset = getLocalVector(last->offset);
            last->edict->s.scale *= edict->s.scale;
         }
      }
   }

   last->vlink = NULL;

   //
   // get the seat offset
   //
   if(gi.GetBoneInfo(edict->s.modelindex, "seat", &groupindex, &tri_num, orient))
   {
      gi.GetBoneTransform(edict->s.modelindex, groupindex, tri_num, orient, edict->s.anim, edict->s.frame,
                          edict->s.scale, trans, driveroffset.vec3());
   }
   driveroffset += seatoffset * edict->s.scale;
   SetDriverAngles(worldangles + seatangles);

   max_health = health;

   //
   // calculate drive mins and maxs
   //
   max = 0;
   if(fabs(mins[0]) > max)
      max = fabs(mins[0]);
   if(fabs(maxs[0]) > max)
      max = fabs(maxs[0]);
   if(fabs(mins[1]) > max)
      max = fabs(mins[1]);
   if(fabs(maxs[1]) > max)
      max = fabs(maxs[1]);
   drivemins = Vector(-max, -max, mins[2]) * edict->s.scale;
   drivemaxs = Vector( max,  max, maxs[2]) * edict->s.scale;

   width  = maxs[1] - mins[1];
   height = maxs[0] - mins[0];

   maxtracedist = height;

   Corners[0][0] = -(width / 4);
   Corners[0][1] = (height / 4);
   Corners[0][2] = 0;

   Corners[1][0] = (width / 4);
   Corners[1][1] = (height / 4);
   Corners[1][2] = 0;

   Corners[2][0] = -(width / 4);
   Corners[2][1] = -(height / 4);
   Corners[2][2] = 0;

   Corners[3][0] = (width / 4);
   Corners[3][1] = -(height / 4);
   Corners[3][2] = 0;

   if(drivable)
   {
      // drop everything back to the floor
      droptofloor(64);
      Postthink();
   }
   last_origin = worldorigin;
   setSize(drivemins, drivemaxs);
   startorigin = worldorigin;
}

void Vehicle::Drivable(Event *ev)
{
   setMoveType(MOVETYPE_NONE);
   drivable = true;
}

void Vehicle::UnDrivable(Event *ev)
{
   setMoveType(MOVETYPE_PUSH);
   drivable = false;
}

void Vehicle::Jumpable(Event *ev)
{
   jumpable = true;
}

void Vehicle::Lock(Event *ev)
{
   locked = true;
}

void Vehicle::UnLock(Event *ev)
{
   locked = false;
}

void Vehicle::SteerInPlace(Event *ev)
{
   steerinplace = true;
}

void Vehicle::SeatAnglesOffset(Event *ev)
{
   seatangles = ev->GetVector(1);
}

void Vehicle::SeatOffset(Event *ev)
{
   seatoffset = ev->GetVector(1);
}

void Vehicle::SetWeapon(Event *ev)
{
   showweapon = true;
   hasweapon  = true;
   weaponName = ev->GetString(1);
}

void Vehicle::ShowWeaponEvent(Event *ev)
{
   showweapon = true;
}

void Vehicle::DriverAnimation(Event *ev)
{
   driveranim = ev->GetString(1);
}

qboolean Vehicle::HasWeapon(void)
{
   return hasweapon;
}

qboolean Vehicle::ShowWeapon(void)
{
   return showweapon;
}

void Vehicle::SetDriverAngles(Vector angles)
{
   int i;

   if(!driver)
      return;

   for(i = 0; i < 3; i++)
   {
      driver->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(angles[i] - driver->client->resp.cmd_angles[i]);
   }
}

/*
=============
CheckWater
=============
*/
void Vehicle::CheckWater(void)
{
   Vector  point;
   int	  cont;
   int	  sample1;
   int	  sample2;
   VehicleBase *v;
   const gravityaxis_t &grav = gravity_axis[gravaxis];

   unlink();
   v = this;
   while(v->vlink)
   {
      v = v->vlink;
      v->unlink();
   }

   if(driver)
   {
      driver->unlink();
   }

   //
   // get waterlevel
   //
   waterlevel = 0;
   watertype = 0;

   sample2 = maxs[grav.z] - mins[grav.z];
   sample1 = sample2 / 2;

   point[grav.x] = worldorigin[grav.x];
   point[grav.y] = worldorigin[grav.y];
   point[grav.z] = worldorigin[grav.z] + mins[grav.z] + grav.sign;
   cont = gi.pointcontents(point.vec3());

   if(cont & MASK_WATER)
   {
      watertype = cont;
      waterlevel = 1;
      point[grav.z] = worldorigin[grav.z] + mins[grav.z] + sample1;
      cont = gi.pointcontents(point.vec3());
      if(cont & MASK_WATER)
      {
         waterlevel = 2;
         point[grav.z] = worldorigin[grav.z] + mins[grav.z] + sample2;
         cont = gi.pointcontents(point.vec3());
         if(cont & MASK_WATER)
         {
            waterlevel = 3;
         }
      }
   }

   link();
   v = this;
   while(v->vlink)
   {
      v = v->vlink;
      v->link();
   }

   if(driver)
   {
      driver->link();
      driver->waterlevel = waterlevel;
      driver->watertype = watertype;
   }
}

/*
=============
WorldEffects
=============
*/
void Vehicle::WorldEffects(void)
{
   //
   // Check for earthquakes
   //
   if(groundentity && (level.earthquake > level.time))
   {
      velocity += Vector(EARTHQUAKE_STRENGTH*G_CRandom(), EARTHQUAKE_STRENGTH*G_CRandom(), fabs(50 * G_CRandom()));
   }

   //
   // check for lava
   //
   if(watertype & CONTENTS_LAVA)
   {
      Damage(world, world, 20 * waterlevel, worldorigin, vec_zero, vec_zero, 0, DAMAGE_NO_ARMOR, MOD_LAVA, -1, -1, 1.0f);
   }
}

void Vehicle::DriverUse(Event *ev)
{
   Event *event;
   Entity *other;
   Sentient *sent;

   other = ev->GetEntity(1);
   if(!other || !other->isSubclassOf<Sentient>())
   {
      return;
   }

   if(other->gravaxis != gravaxis)
   {
      return;
   }

   sent = (Sentient *)other;
   if(driver)
   {
      int height;
      int ang;
      Vector angles;
      Vector forward;
      Vector pos;
      float ofs;
      trace_t trace;

      if(other != driver)
      {
         return;
      }

      if(locked)
         return;

      //
      // place the driver on the ground
      //
      ofs = size.length() * 0.5f;
      for(height = 0; height < 100; height += 16)
      {
         for(ang = 0; ang < 360; ang += 30)
         {
            angles[1] = driver->worldangles[1] + ang + 90;
            angles.AngleVectors(&forward, NULL, NULL);
            pos = worldorigin + (forward * ofs);
            pos[2] += height;
            trace = G_Trace(pos, driver->mins, driver->maxs, pos, NULL, MASK_PLAYERSOLID, "Vehicle::DriverUse 1");
            if(!trace.allsolid)
            {
               Vector end;

               end = pos;
               end[2] -= 128;
               trace = G_Trace(pos, driver->mins, driver->maxs, end, NULL, MASK_PLAYERSOLID, "Vehicle::DriverUse 2");
               if(trace.fraction < 1.0f)
               {
                  driver->setOrigin(pos);
                  goto foundpos;
               }
            }
         }
      }

      // if we are in CTF choose a place by default
      if(ctf->value)
      {
         pos = worldorigin;
         pos[2] += 192;
         driver->setOrigin(pos);
         goto foundpos;
      }
      return;

foundpos:
      turnimpulse = 0;
      moveimpulse = 0;
      jumpimpulse = 0;

      //driver->unbind();
      event = new Event(EV_Vehicle_Exit);
      event->AddEntity(this);
      driver->ProcessEvent(event);
      if(hasweapon)
      {
         //###
         if(oldweapon)
         {
            driver->ForceChangeWeapon(oldweapon);
            oldweapon = NULL;
         }
         //###
         driver->takeWeapon(weaponName.c_str());
      }
      if(drivable)
      {
         edict->s.sound = 0;
         RandomSound("snd_dooropen", 1, CHAN_BODY);
         RandomSound("snd_stop", 1, CHAN_VOICE);
         driver->setSolidType(SOLID_BBOX);
      }

      driver = NULL;
      //if ( drivable )
      //   setMoveType( MOVETYPE_NONE );
   }
   else
   {
      driver = (Sentient *)other;
      lastdriver = driver;

      if(drivable)
         setMoveType(MOVETYPE_VEHICLE);
      if(hasweapon)
      {
         Weapon *weapon = driver->giveWeapon(weaponName.c_str());

         if(weapon)
         {
            oldweapon = driver->CurrentWeapon(); //###
            driver->ForceChangeWeapon(weapon);
         }
         else
         {
            return;
         }
      }
      if(drivable)
      {
         RandomSound("snd_doorclose", 1, CHAN_BODY);
         RandomSound("snd_start", 1, CHAN_VOICE);
         driver->setSolidType(SOLID_NOT);
      }

      event = new Event(EV_Vehicle_Enter);
      event->AddEntity(this);
      if(driveranim.length())
         event->AddString(driveranim);
      driver->ProcessEvent(event);

      //driver->bind( this );
      //offset = other->origin;
      offset = other->worldorigin - worldorigin;

      flags |= FL_POSTTHINK;
      SetDriverAngles(worldangles + seatangles);
   }
}

qboolean Vehicle::Drive(usercmd_t *ucmd)
{
   if(!driver || !driver->isClient())
   {
      return false;
   }

   if(!drivable)
   {
      driver->client->ps.pmove.pm_type = PM_INVEHICLE;
      ucmd->forwardmove = 0;
      ucmd->sidemove = 0;
      return false;
   }

   driver->client->ps.pmove.pm_type = PM_LOCKVIEW;
   driver->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;

   moveimpulse = ((float)ucmd->forwardmove) * 3;
   turnimpulse = ((float)-ucmd->sidemove) * 0.5;
   jumpimpulse = ((float)ucmd->upmove * gravity) / 350;
   if((jumpimpulse < 0) || (!jumpable))
      jumpimpulse = 0;

   buttons = ucmd->buttons;

   turnimpulse += 2 * angledist(SHORT2ANGLE(ucmd->angles[1]) - driver->client->resp.cmd_angles[1]);

   cmd_angles.x = 2 * angledist(SHORT2ANGLE(ucmd->angles[0]) - driver->client->resp.cmd_angles[0]);
   cmd_angles.y = 2 * angledist(SHORT2ANGLE(ucmd->angles[1]) - driver->client->resp.cmd_angles[1]);
   cmd_angles.z = 0;

   return true;
}

void Vehicle::Postthink(void)
{
   float       turn;
   Vector      i, j, k;
   int         index;
   trace_t     trace;
   Vector      normalsum;
   int         numnormals;
   Vector      temp;
   Vector      pitch;
   VehicleBase *v;
   VehicleBase *last;
   float       drivespeed;

   if(drivable)
   {
      currentspeed = moveimpulse / 10;

      turnangle = turnangle * 0.25f + turnimpulse;
      if(turnangle > maxturnrate)
      {
         turnangle = maxturnrate;
      }
      else if(turnangle < -maxturnrate)
      {
         turnangle = -maxturnrate;
      }
      else if(fabs(turnangle) < 2)
      {
         turnangle = 0;
      }
      temp[PITCH] = 0;
      temp[YAW  ] = angles[YAW];
      temp[ROLL ] = 0;
      temp.AngleVectors(&i, &j, &k);
      j = vec_zero - j;

      //
      // figure out what our orientation is
      //
      numnormals = 0;
      for(index = 0; index < 4; index++)
      {
         Vector start, end;
         Vector boxoffset;

         boxoffset = Corners[index];
         start = worldorigin + i * boxoffset[0] + j * boxoffset[1] + k * boxoffset[2];
         end = start;
         end[2] -= maxtracedist;
         trace = G_Trace(start, vec_zero, vec_zero, end, NULL, MASK_SOLID, "Vehicle::PostThink Corners");
         if(trace.fraction != 1.0f && !trace.startsolid)
         {
            normalsum += Vector(trace.plane.normal);
            numnormals++;
         }
      }
      if(numnormals > 1)
      {
         temp = normalsum * (1.0f / numnormals);
         temp.normalize();
         i = temp.CrossProduct(temp, j);
         pitch = i;
         // determine pitch
         angles[0] = (pitch.toPitch());
      }

      turn = turnangle * (1.0f / 200.0f);

      if(groundentity)
      {
         float dot;
         Vector newvel;
         Vector flatvel;

         velocity[0] *= 0.925f;
         velocity[1] *= 0.925f;
         flatvel = Vector(orientation[0]);
         velocity += flatvel * currentspeed;
         flatvel[2] = 0;
         dot = velocity * flatvel;
         if(dot > speed)
         {
            dot = speed;
         }
         else if(dot < -speed)
         {
            dot = -speed;
         }
         else if(fabs(dot) < 20.0f)
         {
            dot = 0;
         }
         newvel = flatvel * dot;
         velocity[0] = newvel[0];
         velocity[1] = newvel[1];
         velocity[2] += dot * jumpimpulse;

         avelocity *= 0.05;
         if(steerinplace)
         {
            if(dot < 350)
               dot = 350;
            avelocity.y += turn * dot;
         }
         else
         {
            avelocity.y += turn * dot;
         }
      }
      else
      {
         avelocity *= 0.1;
      }
      angles += avelocity * FRAMETIME;
      setAngles(angles);
   }
   drivespeed = velocity * Vector(orientation[0]);

   if(drivable && driver)
   {
      Event * event;

      event = new Event(EV_RandomEntitySound);
      if(currentspeed > 0)
         event->AddString("snd_forward");
      else if(currentspeed < 0)
         event->AddString("snd_backward");
      else
         event->AddString("snd_idle");
      ProcessEvent(event);
   }

   i = Vector(orientation[0]);
   j = Vector(orientation[1]);
   k = Vector(orientation[2]);

   if(driver)
   {
      Player * player;

      player = (Player *)(Sentient *)driver;
      player->setOrigin(worldorigin + i * driveroffset[0] + j * driveroffset[1] + k * driveroffset[2]);
      if(drivable)
      {
         player->velocity = vec_zero;
         player->setAngles(angles);
         player->v_angle = angles;
         player->v_angle[PITCH] = player->v_angle[PITCH];
      }
   }

   last = this;
   while(last->vlink)
   {
      v = last->vlink;
      v->setOrigin(worldorigin + i * v->offset.x + j * v->offset.y + k * v->offset.z);
      v->avelocity = avelocity;
      v->velocity = velocity;
      v->angles[ROLL ] = angles[ROLL];
      v->angles[YAW  ] = angles[YAW];
      v->angles[PITCH] = (int)(v->angles[PITCH] + (drivespeed / 4)) % 360;

      if(v->isSubclassOf<FrontWheels>())
      {
         v->angles += Vector(0, turnangle, 0);
      }
      v->setAngles(v->angles);

      last = v;
   }

   CheckWater();
   WorldEffects();

   // save off last origin
   last_origin = worldorigin;

   if(!driver && !velocity.length() && groundentity && !(watertype & CONTENTS_LAVA))
   {
      flags &= ~FL_POSTTHINK;
      if(drivable)
         setMoveType(MOVETYPE_NONE);
   }
}

void Vehicle::VehicleTouched(Event *ev)
{
   Entity  *other;
   float    speed;
   Vector   delta;
   Vector   dir;

   other = ev->GetEntity(1);
   if(other == driver)
   {
      return;
   }

   if(other == world)
   {
      return;
   }

   if(drivable && !driver)
   {
      return;
   }

   delta = worldorigin - last_origin;
   speed = delta.length();
   if(speed > 2)
   {
      RandomGlobalSound("vehicle_crash");
      dir = delta * (1 / speed);
      other->Damage(this, lastdriver, speed * 8, worldorigin, dir, vec_zero, speed * 15, 0, MOD_VEHICLE, -1, -1, 1.0f);
   }
}

void Vehicle::VehicleBlocked(Event *ev)
{
   Entity  *other;
   float    speed;
   float    damage;
   Vector   delta;
   Vector   newvel;
   Vector   dir;

   return;
   if(!velocity[0] && !velocity[1])
      return;

   other = ev->GetEntity(1);
   if(other == driver)
   {
      return;
   }
   if(other->isSubclassOf<VehicleBase>())
   {
      delta = other->worldorigin - worldorigin;
      delta.normalize();

      newvel = vec_zero - (velocity)+(other->velocity * 0.25);
      if(newvel * delta < 0)
      {
         velocity = newvel;
         delta = velocity - other->velocity;
         damage = delta.length() / 4;
      }
      else
      {
         return;
      }
   }
   else if((velocity.length() < 350))
   {
      other->velocity += velocity*1.25f;
      other->velocity[2] += 100;
      damage = velocity.length() / 4;
   }
   else
   {
      damage = other->health + 1000;
   }

   // Gib 'em outright
   speed = fabs(velocity.length());
   dir = velocity * (1 / speed);
   other->Damage(this, lastdriver, damage, worldorigin, dir, vec_zero, speed, 0, MOD_VEHICLE, -1, -1, 1.0f);
}

Sentient *Vehicle::Driver(void)
{
   return driver;
}

qboolean Vehicle::IsDrivable(void)
{
   return drivable;
}

void Vehicle::SetSpeed(Event *ev)
{
   speed = ev->GetFloat(1);
}

void Vehicle::SetTurnRate(Event *ev)
{
   maxturnrate = ev->GetFloat(1);
}

float Vehicle::SetDriverPitch(float pitch)
{
   return pitch;
}

CLASS_DECLARATION(Vehicle, DrivableVehicle, "script_drivablevehicle");

ResponseDef DrivableVehicle::Responses[] =
{
   { &EV_Damage,           (Response)&Entity::DamageEvent      },
   { &EV_Killed,           (Response)&DrivableVehicle::Killed  },
   { &EV_Item_Respawn,     (Response)&DrivableVehicle::Respawn },
   { NULL, NULL }
};

DrivableVehicle::DrivableVehicle() : Vehicle()
{
   drivable = true;
   flags |= FL_SPARKS | FL_DIE_TESSELATE | FL_DIE_EXPLODE | FL_DARKEN;
   if(ctf->value)
      respawntime = G_GetFloatArg("respawntime", 60);
}

void DrivableVehicle::Killed(Event *ev)
{
   Entity * ent;
   Entity * attacker;
   Vector dir;
   Event * event;
   const char * name;
   int num;
   VehicleBase *last;

   takedamage = DAMAGE_NO;
   setSolidType(SOLID_NOT);
   hideModel();

   attacker = ev->GetEntity(1);

   //
   // kill the driver
   //
   if(driver)
   {
      Vector dir;
      SentientPtr sent;
      Event * event;

      velocity = vec_zero;
      sent = driver;
      event = new Event(EV_Use);
      event->AddEntity(sent);
      ProcessEvent(event);
      dir = sent->worldorigin - worldorigin;
      dir[2] += 64;
      dir.normalize();
      sent->Damage(this, this, sent->health * 2, worldorigin, dir, vec_zero, 50, 0, MOD_VEHICLE, -1, -1, 1.0f);
   }

   if(flags & FL_DIE_TESSELATE)
   {
      dir = worldorigin - attacker->worldorigin;
      TesselateModel
      (
         this,
         tess_min_size,
         tess_max_size,
         dir,
         ev->GetInteger(2),
         tess_percentage,
         tess_thickness,
         vec3_origin
      );
      ProcessEvent(EV_BreakingSound);
   }

   if(flags & FL_DIE_EXPLODE)
   {
      CreateExplosion(centroid, 150 * edict->s.scale, edict->s.scale * 2, true, this, this, this);
   }

   if(flags & FL_DIE_GIBS)
   {
      setSolidType(SOLID_NOT);
      hideModel();

      CreateGibs(this, -150, edict->s.scale, 3);
   }

   //
   // kill all my wheels
   //
   if(ctf->value)
   {
      last = this;
      while(last->vlink)
      {
         last->vlink->hideModel();
         last = last->vlink;
      }
   }
   else
   {
      last = this;
      while(last->vlink)
      {
         last->vlink->PostEvent(EV_Remove, 0);
         last = last->vlink;
      }
      vlink = NULL;
   }

   //
   // kill the killtargets
   //
   //### added extended targeting stuff
   for(int i = 0; i < 2; i++)
   {
      switch(i)
      {
      case 0:
         name = KillTarget();
         break;
      case 1:
         name = KillTarget2();
         break;
      }

      if(name && strcmp(name, ""))
      {
         num = 0;
         do
         {
            num = G_FindTarget(num, name);
            if(!num)
               break;

            ent = G_GetEntity(num);
            ent->PostEvent(EV_Remove, 0);
         }
         while(1);
      }
   }
   //###

   //
   // fire targets
   //
   //### added extended targeting stuff
   for(int i = 0; i < 4; i++)
   {
      switch(i)
      {
      case 0:
         name = Target();
         break;
      case 1:
         name = Target2();
         break;
      case 2:
         name = Target3();
         break;
      case 3:
         name = Target4();
         break;
      }

      if(name && strcmp(name, ""))
      {
         num = 0;
         do
         {
            num = G_FindTarget(num, name);
            if(!num)
            {
               break;
            }

            ent = G_GetEntity(num);

            event = new Event(EV_Activate);
            event->AddEntity(attacker);
            ent->ProcessEvent(event);
         }
         while(1);
      }
   }
   //###

   if(ctf->value)
   {
      hideModel();
      // cancel events of type remove
      CancelEventsOfType(EV_Remove);
      PostEvent(EV_Item_Respawn, respawntime);
   }
   else
   {
      PostEvent(EV_Remove, 0);
   }
}

void DrivableVehicle::Respawn(Event *ev)
{
   VehicleBase *last;

   health = max_health;
   edict->s.lightofs = 0;

   showModel();

   last = this;
   while(last->vlink)
   {
      last->vlink->showModel();
      last = last->vlink;
   }

   // allow it to be touched again
   setSolidType(SOLID_BBOX);
   takedamage = DAMAGE_YES;

   // play respawn sound
   RandomGlobalSound("snd_itemspawn");

   setOrigin(startorigin);
   KillBox(this);
   Postthink();
}

// EOF

