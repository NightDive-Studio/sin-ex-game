//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/ctf_turret.cpp                    $
// $Revision:: 14                                                             $
//   $Author:: Aldie                                                          $
//     $Date:: 3/19/99 5:50p                                                  $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// CTF turret, usable turret
// 

#include "g_local.h"
#include "vehicle.h"
#include "ctf_turret.h"
#include "heligun.h"
#include "player.h"


CLASS_DECLARATION(DrivableVehicle, CTFTurret, "ctf_turret");

ResponseDef CTFTurret::Responses[] =
{
   { &EV_Use, (Response)&CTFTurret::DriverUse },
   { NULL, NULL }
};

CTFTurret::CTFTurret() : DrivableVehicle()
{
   takedamage = DAMAGE_YES;
   setModel("ctf_turret.def");
   modelIndex("ctf_turretgun.def");
   modelIndex("view_ctf_turretgun.def");
   setOrigin(origin - Vector(0, 0, 30));
   drivable = false;
   setMoveType(MOVETYPE_NONE);
   setSize({ -24, -24, -64 }, { 24, 24, 0 });
   edict->s.renderfx |= RF_MINLIGHT;
}

void CTFTurret::DriverUse(Event *ev)
{
   Entity * old_driver;

   old_driver = driver;
   DrivableVehicle::DriverUse(ev);
   if(old_driver != driver)
   {
      if(driver)
      {
         setSolidType(SOLID_NOT);
         takedamage = DAMAGE_NO;
         hideModel();
      }
      else
      {
         setSolidType(SOLID_BBOX);
         takedamage = DAMAGE_YES;
         showModel();
      }
   }
   setMoveType(MOVETYPE_NONE);
}

#define MAX_PITCH 45
float CTFTurret::SetDriverPitch(float pitch)
{
   if(pitch > 180)
   {
      if(pitch < 360 - MAX_PITCH)
      {
         pitch = 360 - MAX_PITCH;
      }
   }
   else
   {
      if(pitch > MAX_PITCH)
      {
         pitch = MAX_PITCH;
      }
      if(pitch < -MAX_PITCH)
      {
         pitch = -MAX_PITCH;
      }
   }
   return pitch;
}

CLASS_DECLARATION(HeliGun, CTFTurretGun, "ctf_weapon_turretgun");

ResponseDef CTFTurretGun::Responses[] =
{
   { NULL, NULL }
};

CTFTurretGun::CTFTurretGun() : HeliGun()
{
   SetModels("ctf_turretgun.def", "view_ctf_turretgun.def");
}

void CTFTurretGun::Shoot(Event *ev)
{
   FireBullets(1, { 20, 20, 20 }, 48, 64, DAMAGE_BULLET, MOD_CTFTURRET, false);
   NextAttack(0);
}

qboolean CTFTurretGun::IsDroppable(void)
{
   return false;
}

//
// CTF turret which is lagged but has angle limits set on it.
//
CLASS_DECLARATION( DrivableVehicle, CTFDrivableTurret, "ctf_fixedturret" );

ResponseDef CTFDrivableTurret::Responses[] =
{
   { &EV_Use, (Response)&CTFDrivableTurret::DriverUse },
   { NULL, NULL }
};

CTFDrivableTurret::CTFDrivableTurret() : DrivableVehicle()
{
   const ClassDef *cls;

   takedamage = DAMAGE_YES;
   setModel("ctf_fixedturret.def");
   setOrigin(origin - Vector(0, 0, 30));
   baseangles = angles;

   shotdamage = G_GetFloatArg("shotdamage", 48);
   maxpitch = G_GetFloatArg("maxpitch", 30);
   maxyaw = G_GetFloatArg("maxyaw", 75);

   cls = getClass("Sentient");
   if(cls)
   {
      gunholder = (Sentient *)cls->newInstance();
      gunholder->hideModel();
   }
   cls = getClass("BulletWeapon");
   if(cls)
   {
      gun = (BulletWeapon *)cls->newInstance();
      gun->SetModels("ctf_fixedturret.def", "ctf_fixedturret.def");
      gun->SetOwner(gunholder);
      gun->hideModel();
   }

   setSize({ -24, -24, -64 }, { 24, 24, 0 });
   lastbutton = -1;
   entertime = -1;
   exittime = -1;
   setMoveType(MOVETYPE_NONE);
   edict->s.renderfx |= RF_MINLIGHT;
}

void CTFDrivableTurret::Postthink(void)
{
   Vector temp;
   Vector diff;

   // make sure we don't move
   velocity = vec_zero;

   if(cmd_angles[0] > maxturnrate)
   {
      cmd_angles[0] = maxturnrate;
   }
   else if(cmd_angles[0] < -maxturnrate)
   {
      cmd_angles[0] = -maxturnrate;
   }
   if(cmd_angles[1] > maxturnrate)
   {
      cmd_angles[1] = maxturnrate;
   }
   else if(cmd_angles[1] < -maxturnrate)
   {
      cmd_angles[1] = -maxturnrate;
   }

   temp = angles + cmd_angles;

   diff = temp - baseangles;
   diff.x = angledist(diff.x);
   diff.y = angledist(diff.y);
   gi.dprintf("diff x%5.2f y%5.2f\n", diff.x, diff.y);

   if(diff.x > maxpitch)
   {
      temp.x = baseangles.x + maxpitch;
   }
   else if(diff.x < -maxpitch)
   {
      temp.x = baseangles.x - maxpitch;
   }

   if(maxyaw < 180)
   {
      if(diff.y > maxyaw)
      {
         temp.y = baseangles.y + maxyaw;
      }
      else if(diff.y < -maxyaw)
      {
         temp.y = baseangles.y - maxyaw;
      }
   }

   setAngles(temp);

   if(driver)
   {
      Vector i, j, k;
      Player * player;

      i = Vector(orientation[0]);
      j = Vector(orientation[1]);
      k = Vector(orientation[2]);

      player = (Player *)(Sentient *)driver;
      player->setOrigin(worldorigin + i * driveroffset[0] + j * driveroffset[1] + k * driveroffset[2]);
      if(drivable)
      {
         player->velocity = vec_zero;
         player->setAngles(angles);
         player->SetViewAngles(angles);
         player->SetDeltaAngles();
      }
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
   if(driver)
   {
      if(buttons & BUTTON_ATTACK)
      {
         if(!lastbutton)
         {
            lastbutton = 1;
            RandomAnimate("fire", NULL);
         }
         Fire();
      }
      else if(buttons & BUTTON_USE)
      {
         Event * event;

         if(level.time > (entertime + 0.5f))
         {
            event = new Event(EV_Use);
            event->AddEntity(driver);
            ProcessEvent(event);
         }
      }
      else
      {
         if(lastbutton)
         {
            lastbutton = 0;
            RandomAnimate("idle", NULL);
         }
      }
   }
}

void CTFDrivableTurret::Fire(void)
{
   setSolidType(SOLID_NOT);
   gunholder->setOrigin(origin);
   gunholder->setAngles(angles);
   gun->FireBullets(1, { 20, 20, 20 }, shotdamage, shotdamage * 1.5f, DAMAGE_BULLET, MOD_CTFTURRET, true);
   setSolidType(SOLID_BBOX);
}

float CTFDrivableTurret::SetDriverPitch(float pitch)
{
   return pitch;
}

qboolean CTFDrivableTurret::Drive(usercmd_t *ucmd)
{
   if(!driver || !driver->isClient())
   {
      return false;
   }
   else
   {
      DrivableVehicle::Drive(ucmd);
      ucmd->buttons &= ~BUTTON_ATTACK;
      ucmd->buttons &= ~BUTTON_USE;
      return true;
   }
}

void CTFDrivableTurret::DriverUse(Event *ev)
{
   Entity * old_driver;

   if(!driver && (level.time < (exittime + 0.5f)))
      return;

   cmd_angles[0] = 0;
   cmd_angles[1] = 0;
   cmd_angles[2] = 0;
   buttons = 0;
   lastbutton = -1;
   setMoveType(MOVETYPE_NONE);
   RandomAnimate("idle", NULL);

   old_driver = driver;

   DrivableVehicle::DriverUse(ev);

   if(old_driver != driver)
   {
      if(driver)
      {
         entertime = level.time;
      }
      else
      {
         exittime = level.time;
      }
   }
}

// EOF

