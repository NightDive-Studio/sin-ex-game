//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/skeet.cpp                        $
// $Revision:: 24                                                             $
//   $Author:: Markd                                                          $
//     $Date:: 10/24/98 12:42a                                                $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION: Skeet Entity
// 

#include "g_local.h"
#include "item.h"
#include "rocketlauncher.h"
#include "worldspawn.h"
#include "skeet.h"
#include "scriptmaster.h"

CLASS_DECLARATION(Object, Pigeon, "pigeon");

Event EV_Pigeon_Remove("pigeonremove");

ResponseDef Pigeon::Responses[] =
{
   { &EV_Touch,	         			(Response)&Pigeon::RemovePigeon },
   { &EV_Killed,					      (Response)&Pigeon::Killed },
   { nullptr, nullptr }
};

void Pigeon::RemovePigeon(Event *ev)
{
   setSolidType(SOLID_NOT);
   hideModel();
   PostEvent(EV_Remove, 0);
}

void Pigeon::Killed(Event *ev)
{
   Entity * attacker;
   Vector dir;

   takedamage = DAMAGE_NO;
   setSolidType(SOLID_NOT);
   hideModel();

   attacker = ev->GetEntity(1);
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
   RandomSound("pig_death", 1);

   Director.DeathMessage(name.c_str());
   PostEvent(EV_Remove, 0);
}

void Pigeon::Setup(Entity *owner, Vector pos, Vector dir, float pspeed, float pgrav, float pduration, const char *pigeon_name)
{
   Event *ev;

   this->owner = owner->entnum;
   edict->owner = owner->edict;

   name = str(pigeon_name);
   setMoveType(MOVETYPE_BOUNCE);
   setSolidType(SOLID_BBOX);
   edict->clipmask = MASK_SHOT;

   maxspeed = 500;
   acceleration = 150;
   speed = pspeed;
   velocity = dir * speed;
   health = 1;
   gravity = pgrav;
   takedamage = DAMAGE_YES;

   ev = new Event(EV_Remove);
   ev->AddEntity(world);
   PostEvent(ev, pduration);

   setModel("skeet.def");
   setSize(mins, maxs);
   setOrigin(pos);
   worldorigin.copyTo(edict->s.old_origin);
}

CLASS_DECLARATION(Entity, SkeetLauncher, "skeetlauncher");

Event EV_SkeetLauncher_Launch("launch");
Event EV_SkeetLauncher_SetXdir("setXdir");
Event EV_SkeetLauncher_SetYdir("setYdir");
Event EV_SkeetLauncher_SetZdir("setZdir");
Event EV_SkeetLauncher_SetSpeed("setSpeed");
Event EV_SkeetLauncher_SetGravity("setGravity");
Event EV_SkeetLauncher_SetDuration("setDuration");
Event EV_SkeetLauncher_SetSpeedVar("setSpeedVar");
Event EV_SkeetLauncher_SetXvar("setXvar");
Event EV_SkeetLauncher_SetYvar("setYvar");
Event EV_SkeetLauncher_SetZvar("setZvar");

ResponseDef SkeetLauncher::Responses[] =
{
   { &EV_SkeetLauncher_Launch,				(Response)&SkeetLauncher::Launch },
   { &EV_SkeetLauncher_SetXdir,				(Response)&SkeetLauncher::SetXdir },
   { &EV_SkeetLauncher_SetYdir,				(Response)&SkeetLauncher::SetYdir },
   { &EV_SkeetLauncher_SetZdir,				(Response)&SkeetLauncher::SetZdir },
   { &EV_SkeetLauncher_SetSpeed,				(Response)&SkeetLauncher::SetSpeed },
   { &EV_SkeetLauncher_SetGravity,	   	(Response)&SkeetLauncher::SetGravity },
   { &EV_SkeetLauncher_SetDuration,			(Response)&SkeetLauncher::SetDuration },
   { &EV_SkeetLauncher_SetSpeedVar,       (Response)&SkeetLauncher::SetSpeedVar },
   { &EV_SkeetLauncher_SetXvar,           (Response)&SkeetLauncher::SetXvar },
   { &EV_SkeetLauncher_SetYvar,           (Response)&SkeetLauncher::SetYvar },
   { &EV_SkeetLauncher_SetZvar,           (Response)&SkeetLauncher::SetZvar },
   { nullptr, nullptr }
};

SkeetLauncher::SkeetLauncher() : Entity()
{
   showModel();
   setSolidType(SOLID_BSP);
   speed_var = 0;
   x_var = 0;
   y_var = 0;
   z_var = 0;
}

void SkeetLauncher::SetXdir(Event *ev)
{
   launchAngle.x = ev->GetFloat(1);
}

void SkeetLauncher::SetYdir(Event *ev)
{
   launchAngle.y = ev->GetFloat(1);
}

void SkeetLauncher::SetZdir(Event *ev)
{
   launchAngle.z = ev->GetFloat(1);
}

void SkeetLauncher::SetSpeed(Event *ev)
{
   pspeed = ev->GetFloat(1);
}

void SkeetLauncher::SetGravity(Event *ev)
{
   pgravity = ev->GetFloat(1);
}

void SkeetLauncher::SetDuration(Event *ev)
{
   pduration = ev->GetFloat(1);
}

void SkeetLauncher::SetSpeedVar(Event *ev)
{
   speed_var = ev->GetFloat(1);
}

void SkeetLauncher::SetXvar(Event *ev)
{
   x_var = ev->GetFloat(1);
}

void SkeetLauncher::SetYvar(Event *ev)
{
   y_var = ev->GetFloat(1);
}

void SkeetLauncher::SetZvar(Event *ev)
{
   z_var = ev->GetFloat(1);
}

void SkeetLauncher::Launch(Event *ev)
{
   Pigeon		*pigeon;
   float			sp;
   Vector		ang;
   const char	*name;

   pigeon = new Pigeon();

   name = ev->GetString(1);
   ang.x = launchAngle.x + G_CRandom(x_var);
   ang.y = launchAngle.y + G_CRandom(y_var);
   ang.z = launchAngle.z + G_CRandom(z_var);
   sp = pspeed + G_CRandom(speed_var);
   pigeon->Setup(this, worldorigin, ang.normalize(), sp, pgravity, pduration, name);
}

// EOF

