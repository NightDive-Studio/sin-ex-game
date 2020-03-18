/*
================================================================
MANERO DA BOSS MAN
================================================================

Copyright (C) 2020 by Night Dive Studios, Inc.
All rights reserved.

See the license.txt file for conditions and terms of use for this code.
*/

#ifndef __MANEROBOSS_H__
#define __MANEROBOSS_H__

#include "g_local.h"
#include "actor.h"
#include "weapon.h"
#include "rocketpack.h"
#include "nuke.h"

class EXPORT_FROM_DLL ManeroCopter :public ScriptModel
{
private:
   float     lastrockettime  = 0.0f;
   float     lastguntime     = 0.0f;
   float     lastgrenadetime = 0.0f;
   Vector    lastfiredir;
   EntityPtr maneromodel;
   EntityPtr manerogun;

public:
   CLASS_PROTOTYPE(ManeroCopter);

   ManeroCopter();

   virtual void Postthink() override;
   void         StartAttacking(Event *ev);
   void         StopAttacking(Event *ev);
   void         RemoveManero(Event *ev);
   void         FireRockets(Event *ev);
   void         FireGrenade();
   void         FireGun();
   void         TraceAttack(Vector start, Vector end, int damage, trace_t *trace, int numricochets, int kick);
   void         FireBullets(Vector spread, int mindamage, int maxdamage, Vector src, Vector dir);
   void         FireTracer(Vector src, Vector end);
   Vector       GunPosition();

   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void ManeroCopter::Archive(Archiver &arc)
{
   ScriptModel::Archive(arc);

   arc.WriteFloat(lastrockettime);
   arc.WriteFloat(lastguntime);
   arc.WriteFloat(lastgrenadetime);
   arc.WriteVector(lastfiredir);
   arc.WriteSafePointer(maneromodel);
   arc.WriteSafePointer(manerogun);
}

inline EXPORT_FROM_DLL void ManeroCopter::Unarchive(Archiver &arc)
{
   ScriptModel::Unarchive(arc);

   arc.ReadFloat(&lastrockettime);
   arc.ReadFloat(&lastguntime);
   arc.ReadFloat(&lastgrenadetime);
   arc.ReadVector(&lastfiredir);
   arc.ReadSafePointer(&maneromodel);
   arc.ReadSafePointer(&manerogun);
}

class EXPORT_FROM_DLL ManeroCopterTracer :public Entity
{
public:
   CLASS_PROTOTYPE(ManeroCopterTracer);

   ManeroCopterTracer();

   void RemoveTracer(Event *ev);
};

class EXPORT_FROM_DLL ManeroBoss : public Actor
{
private:
   WeaponPtr secondaryWeapon;

public:
   CLASS_PROTOTYPE(ManeroBoss);

   ManeroBoss();
   virtual void Postthink()              override;
   void         ShieldsOn(Event *ev);
   void         ShieldsOff(Event *ev);
   void         CloakOn(Event *ev);
   void         CloakOff(Event *ev);
   void         FireNuke(Event *ev);
   virtual void ArmorDamage(Event *ev)   override;
   void         Pain(Event *ev);
   void         Dead(Event *ev);
   void         Killed(Event *ev);

   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void ManeroBoss::Archive(Archiver &arc)
{
   Actor::Archive(arc);

   arc.WriteSafePointer(secondaryWeapon);
}

inline EXPORT_FROM_DLL void ManeroBoss::Unarchive(Archiver &arc)
{
   Actor::Unarchive(arc);

   arc.ReadSafePointer(&secondaryWeapon);
}

class EXPORT_FROM_DLL ManeroStingerPack : public StingerPack
{
public:
   CLASS_PROTOTYPE(ManeroStingerPack);

   ManeroStingerPack();
   virtual void Shoot(Event *ev);
};

class EXPORT_FROM_DLL ManeroIP36 : public IP36
{
public:
   CLASS_PROTOTYPE(ManeroIP36);

   virtual void Shoot(Event *ev);
};

#endif

// EOF

