//-----------------------------------------------------------------------------
//
// Thug header file by Boon, created 11-11-98
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Contains the code which causes enemies to be able to shoot around walls
// and stuff when they are ducked
// 

#ifndef __THUG_H__
#define __THUG_H__

#include "g_local.h"
#include "actor.h"

extern Event EV_Behavior_AnimDone;

///////////////////////////////////////////
// DuckAttack stuff by Boon
///////////////////////////////////////////

class EXPORT_FROM_DLL DuckAttack : public Behavior
{
private:
   int      state;
   TurnTo   turn;
   qboolean animdone;

public:
   CLASS_PROTOTYPE(DuckAttack);

   void         ShowInfo(Actor &self);
   void         AnimDone(Event *ev);
   void         Begin(Actor &self);
   qboolean     Evaluate(Actor &self);
   qboolean     FireLeft(Actor &self);
   void         End(Actor &self);
   virtual void Archive(Archiver &arc);
   virtual void Unarchive(Archiver &arc);
};

inline EXPORT_FROM_DLL void DuckAttack::Archive(Archiver &arc)
{
   Behavior::Archive(arc);

   arc.WriteInteger(state);
   arc.WriteObject(&turn);
   arc.WriteBoolean(animdone);
}

inline EXPORT_FROM_DLL void DuckAttack::Unarchive(Archiver &arc)
{
   Behavior::Unarchive(arc);

   arc.ReadInteger(&state);
   arc.ReadObject(&turn);
   arc.ReadBoolean(&animdone);
}

class EXPORT_FROM_DLL Lean_AimAndShoot : public AimAndShoot
{
private:
   Aim      aim;
   int      mode;
   int      maxshots;
   int      numshots;
   qboolean animdone;
   float    enemy_health;
   float    aim_time;
   str      animprefix;
   str      moveanim;
   str      readyfireanim;
   str      aimanim;
   str      fireanim;

public:
   CLASS_PROTOTYPE(Lean_AimAndShoot);

   Lean_AimAndShoot();
   void         SetMaxShots(int num);
   void         SetArgs(Event *ev);
   void         AnimDone(Event *ev);
   void         Begin(Actor &self);
   qboolean     Evaluate(Actor &self);
   void         End(Actor &self);
   virtual void Archive(Archiver &arc);
   virtual void Unarchive(Archiver &arc);
};

inline EXPORT_FROM_DLL void Lean_AimAndShoot::Archive(Archiver &arc)
{
   AimAndShoot::Archive(arc);

   arc.WriteObject(&aim);
   arc.WriteInteger(mode);
   arc.WriteInteger(maxshots);
   arc.WriteInteger(numshots);
   arc.WriteBoolean(animdone);
   arc.WriteFloat(enemy_health);
   arc.WriteFloat(aim_time);
   arc.WriteString(animprefix);
   arc.WriteString(moveanim);
   arc.WriteString(readyfireanim);
   arc.WriteString(aimanim);
   arc.WriteString(fireanim);
}

inline EXPORT_FROM_DLL void Lean_AimAndShoot::Unarchive(Archiver &arc)
{
   AimAndShoot::Unarchive(arc);

   arc.ReadObject(&aim);
   arc.ReadInteger(&mode);
   arc.ReadInteger(&maxshots);
   arc.ReadInteger(&numshots);
   arc.ReadBoolean(&animdone);
   arc.ReadFloat(&enemy_health);
   arc.ReadFloat(&aim_time);
   arc.ReadString(&animprefix);
   arc.ReadString(&moveanim);
   arc.ReadString(&readyfireanim);
   arc.ReadString(&aimanim);
   arc.ReadString(&fireanim);
}

////////////////////////
// Slim's concussion gun
////////////////////////

#include "item.h"
#include "weapon.h"
#include "concussion.h"

class EXPORT_FROM_DLL Slimconcussion : public ConcussionGun
{
public:
   CLASS_PROTOTYPE(Slimconcussion);

   Slimconcussion();
};

#endif

// EOF

