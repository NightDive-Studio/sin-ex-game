//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/securityturret.cpp               $
// $Revision:: 25                                                             $
//   $Author:: Aldie                                                          $
//     $Date:: 9/19/98 6:13p                                                  $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Security turret.  Senses when player is near and raises up, searches for player,
// and fires.  When player is dead or not near, it lowers down and goes back to
// sleep.
// 

#include "securityturret.h"

CLASS_DECLARATION(Turret, TurretTop, "trap_securityturret")

ResponseDef TurretTop::Responses[] =
{
   { nullptr, nullptr }
};

TurretTop::TurretTop() : Turret()
{
   setModel("turtop.def");
   setSize({ -16, -16, 0 }, { 16, 16, 26 });
   RandomAnimate("down_idle", nullptr);

   gunoffset = { 0, 0, 0 };

   neworientation = angles.yaw();

   setMoveType(MOVETYPE_NONE);
   setSolidType(SOLID_BBOX);

   health = G_GetFloatArg("health", 100);
   takedamage = DAMAGE_YES;
   flags |= FL_SPARKS;

   wakeupdistance = G_GetFloatArg("wakeupdistance", 750);
   firingdistance = G_GetFloatArg("firingdistance", 800);

   SetupBase();
}

void TurretTop::SetupBase()
{
   TurretBase *baseptr;

   baseptr = new TurretBase();
   baseptr->setOrigin(worldorigin);

   base = baseptr->entnum;
}

CLASS_DECLARATION(Entity, TurretBase, nullptr)

ResponseDef TurretBase::Responses[] =
{
   { &EV_Turret_GoUp,	(Response)&TurretBase::GoUp },
   { &EV_Turret_GoDown,	(Response)&TurretBase::GoDown },
   { nullptr, nullptr }
};

TurretBase::TurretBase() : Entity()
{
   setModel("turbase.def");
   setSize({ -16, -16, 0 }, { 16, 16, 1 });
   setMoveType(MOVETYPE_NONE);
   setSolidType(SOLID_BBOX);
   RandomAnimate("down_idle", NULL);
   takedamage = DAMAGE_NO;
}

void TurretBase::GoUp(Event *ev)
{
   RandomAnimate("raise", nullptr);
}

void TurretBase::GoDown(Event *ev)
{
   RandomAnimate("lower", nullptr);
}

// EOF

