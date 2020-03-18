/*
================================================================
GOLIATH
================================================================

Copyright (C) 2020 by Night Dive Studios, Inc.
All rights reserved.

See the license.txt file for conditions and terms of use for this code.
*/

#ifndef __GOLIATH_H__
#define __GOLIATH_H__

#include "g_local.h"
#include "actor.h"

extern Event EV_Goliath_InitThrowTime;
extern Event EV_Goliath_MeleeForce;
extern Event EV_Goliath_SetLeftHand;
extern Event EV_Goliath_SetRightHand;
extern Event EV_Goliath_SetBothHands;
extern Event EV_Goliath_SetAttackStage;

extern Event EV_Goliath_IfThrowHigh;
extern Event EV_Goliath_IfThrowLow;

class EXPORT_FROM_DLL Goliath :public Actor
{
public:
   float    randomthrowtime;
   float    rubbletime;
   qboolean throwhigh;

   float    melee_force;
   float    rightdamage;
   float    rightforce;
   float    leftdamage;
   float    leftforce;
   qboolean hitsentient; // set when a melee attack hits a sentient
   Vector   lastrightpos;
   Vector   lastleftpos;

   int      attackstage;

   CLASS_PROTOTYPE(Goliath);

   Goliath();
   virtual void Prethink(void);
   virtual void Postthink(void);

   void         Pain(Event *ev);
   void         InitThrowTime(Event *ev);
   void         SetMeleeForce(Event *ev);
   void         SetLeftHand(Event *ev);
   void         SetRightHand(Event *ev);
   void         SetBothHands(Event *ev);
   void         SetAttackStage(Event *ev);
   void         DoHandHits(Vector pos, Vector dir, float handdamage, float handforce, edict_t *touch[MAX_EDICTS], int num);

   Entity      *CheckObjectsAbove(Entity *ent);

   void         IfThrowHighEvent(Event *ev);
   void         IfThrowLowEvent(Event *ev);

   virtual void Archive(Archiver &arc);
   virtual void Unarchive(Archiver &arc);
};

inline EXPORT_FROM_DLL void Goliath::Archive(Archiver &arc)
{
   Actor::Archive(arc);

   arc.WriteFloat(randomthrowtime);
   arc.WriteFloat(rubbletime);
   arc.WriteBoolean(throwhigh);
   arc.WriteFloat(melee_force);
   arc.WriteFloat(rightdamage);
   arc.WriteFloat(rightforce);
   arc.WriteFloat(leftdamage);
   arc.WriteFloat(leftforce);
   arc.WriteBoolean(hitsentient);
   arc.WriteVector(lastrightpos);
   arc.WriteVector(lastleftpos);
   arc.WriteInteger(attackstage);
}

inline EXPORT_FROM_DLL void Goliath::Unarchive(Archiver &arc)
{
   Actor::Unarchive(arc);

   arc.ReadFloat(&randomthrowtime);
   arc.ReadFloat(&rubbletime);
   arc.ReadBoolean(&throwhigh);
   arc.ReadFloat(&melee_force);
   arc.ReadFloat(&rightdamage);
   arc.ReadFloat(&rightforce);
   arc.ReadFloat(&leftdamage);
   arc.ReadFloat(&leftforce);
   arc.ReadBoolean(&hitsentient);
   arc.ReadVector(&lastrightpos);
   arc.ReadVector(&lastleftpos);
   arc.ReadInteger(&attackstage);
}

//=====================================================================
// Goliath behaiviors


class EXPORT_FROM_DLL RumbleAttack : public Behavior
{
private:
   str      anim;
   qboolean animdone;
   str      rubbletarget;
   int      rubblenums;

public:
   CLASS_PROTOTYPE(RumbleAttack);

   void         SetArgs(Event *ev);
   void         AnimDone(Event *ev);
   void         DoRumble(Event *ev);
   void         ShowInfo(Actor &self);
   void         Begin(Actor &self);
   qboolean     Evaluate(Actor &self);
   void         End(Actor &self);
   virtual void Archive(Archiver &arc);
   virtual void Unarchive(Archiver &arc);
};

inline EXPORT_FROM_DLL void RumbleAttack::Archive(Archiver &arc)
{
   Behavior::Archive(arc);

   arc.WriteString(anim);
   arc.WriteBoolean(animdone);
   arc.WriteString(rubbletarget);
   arc.WriteInteger(rubblenums);
}

inline EXPORT_FROM_DLL void RumbleAttack::Unarchive(Archiver &arc)
{
   Behavior::Unarchive(arc);

   arc.ReadString(&anim);
   arc.ReadBoolean(&animdone);
   arc.ReadString(&rubbletarget);
   arc.ReadInteger(&rubblenums);
}

class EXPORT_FROM_DLL GoliathMelee : public Behavior
{
private:
   TurnTo   turnto;
   int      mode;
   qboolean animdone;

public:
   CLASS_PROTOTYPE(GoliathMelee);

   GoliathMelee();
   void         SetArgs(Event *ev);
   void         AnimDone(Event *ev);
   void         ShowInfo(Actor &self);
   void         Begin(Actor &self);
   qboolean     Evaluate(Actor &self);
   void         End(Actor &self);
   virtual void Archive(Archiver &arc);
   virtual void Unarchive(Archiver &arc);
};

inline EXPORT_FROM_DLL void GoliathMelee::Archive(Archiver &arc)
{
   Behavior::Archive(arc);

   arc.WriteObject(&turnto);
   arc.WriteInteger(mode);
   arc.WriteBoolean(animdone);
}

inline EXPORT_FROM_DLL void GoliathMelee::Unarchive(Archiver &arc)
{
   Behavior::Unarchive(arc);

   arc.ReadObject(&turnto);
   arc.ReadInteger(&mode);
   arc.ReadBoolean(&animdone);
}

class EXPORT_FROM_DLL GoliathPickupAndThrow : public Behavior
{
private:
   Aim       aim;
   int       mode;
   qboolean  animdone;
   EntityPtr pickup_target;
   str       anim; // added different pickup animations support
   TurnTo    turnto; // added to make the AI look at what they're picking up

public:
   CLASS_PROTOTYPE(GoliathPickupAndThrow);

   GoliathPickupAndThrow();
   void                 SetArgs(Event *ev);
   void                 AnimDone(Event *ev);
   void                 Pickup(Event *ev);
   void                 Throw(Event *ev);
   void                 ShowInfo(Actor &self);
   void                 Begin(Actor &self);
   qboolean             Evaluate(Actor &self);
   void                 End(Actor &self);
   virtual void         Archive(Archiver &arc);
   virtual void         Unarchive(Archiver &arc);
};

inline EXPORT_FROM_DLL void GoliathPickupAndThrow::Archive(Archiver &arc)
{
   Behavior::Archive(arc);

   arc.WriteObject(&aim);
   arc.WriteInteger(mode);
   arc.WriteBoolean(animdone);
   arc.WriteSafePointer(pickup_target);
   arc.WriteString(anim);
   arc.WriteObject(&turnto);
}

inline EXPORT_FROM_DLL void GoliathPickupAndThrow::Unarchive(Archiver &arc)
{
   Behavior::Unarchive(arc);

   arc.ReadObject(&aim);
   arc.ReadInteger(&mode);
   arc.ReadBoolean(&animdone);
   arc.ReadSafePointer(&pickup_target);
   arc.ReadString(&anim);
   arc.ReadObject(&turnto);
}

#endif

// EOF

