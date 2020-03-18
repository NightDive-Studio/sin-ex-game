/*
================================================================
FLAMETHROWER
================================================================

Copyright (C) 2020 by Night Dive Studios, Inc.
All rights reserved.

See the license.txt file for conditions and terms of use for this code.
*/

#ifndef __FLAMETHROWER_H__
#define __FLAMETHROWER_H__

#include "g_local.h"
#include "item.h"
#include "weapon.h"
#include "misc.h"

class EXPORT_FROM_DLL ThrowerFlame : public Entity
{
private:
   int    owner;
   Vector end;
   int    counter;
   float  length;
   Vector dir;
   Vector hitpos;

public:
   CLASS_PROTOTYPE(ThrowerFlame);

   void             Setup(Entity *owner, Vector pos, Vector streamend);
   virtual qboolean CanToast(Entity *target, float arc);
   void             Burn(Event *ev);

   virtual void     Archive(Archiver &arc)   override;
   virtual void     Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void ThrowerFlame::Archive(Archiver &arc)
{
   Entity::Archive(arc);

   arc.WriteInteger(owner);
   arc.WriteVector(end);
   arc.WriteInteger(counter);
   arc.WriteFloat(length);
   arc.WriteVector(dir);
   arc.WriteVector(hitpos);
}

inline EXPORT_FROM_DLL void ThrowerFlame::Unarchive(Archiver &arc)
{
   Entity::Unarchive(arc);

   arc.ReadInteger(&owner);
   arc.ReadVector(&end);
   arc.ReadInteger(&counter);
   arc.ReadFloat(&length);
   arc.ReadVector(&dir);
   arc.ReadVector(&hitpos);
}

template class EXPORT_FROM_DLL SafePtr<ThrowerFlame>;
typedef SafePtr<ThrowerFlame> ThrowerFlamePtr;

class EXPORT_FROM_DLL FlameThrower : public Weapon
{
   //private:
protected:
   float  blastcounter = 0.0f; // used to make sure the flamethrower fires for a min amount of time
   float  flamelength  = 0.0f; // keeps track of how long the flamethrower flame is.
   float  lastfiretime = 0.0f; // last time that the flamethrower was fired
   float  lastanimtime = 0.0f; // last time firing animation was played
   Vector lastpos;             // last place fired from
   Vector lastdest;            // last place fired to

public:
   CLASS_PROTOTYPE(FlameThrower);

   ThrowerFlamePtr mainflame; // points to main damage causing entity

   FlameThrower();
   virtual void Fire()                   override;
   virtual void Shoot(Event *ev);
   virtual void BlastTimer(Event *ev);
   virtual void SecondaryUse(Event *ev)  override;

   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void FlameThrower::Archive(Archiver &arc)
{
   Weapon::Archive(arc);

   arc.WriteFloat(blastcounter);
   arc.WriteFloat(flamelength);
   arc.WriteFloat(lastfiretime);
   arc.WriteFloat(lastanimtime);
   arc.WriteVector(lastpos);
   arc.WriteVector(lastdest);
   arc.WriteSafePointer(mainflame);
}

inline EXPORT_FROM_DLL void FlameThrower::Unarchive(Archiver &arc)
{
   Weapon::Unarchive(arc);

   arc.ReadFloat(&blastcounter);
   arc.ReadFloat(&flamelength );
   arc.ReadFloat(&lastfiretime);
   arc.ReadFloat(&lastanimtime);
   arc.ReadVector(&lastpos);
   arc.ReadVector(&lastdest);
   arc.ReadSafePointer(&mainflame);
}

#endif /* flamethrower.h */

// EOF

