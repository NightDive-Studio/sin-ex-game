/*
================================================================
PLASMA BOW WEAPON
================================================================

Copyright (C) 2020 by Night Dive Studios, Inc.
All rights reserved.

See the license.txt file for conditions and terms of use for this code.
*/

#ifndef __CROSSBOW_H__
#define __CROSSBOW_H__

#include "g_local.h"
#include "item.h"
#include "weapon.h"
#include "misc.h"
#include "specialfx.h"

// time it takes to fully charge plasma bow (in seconds)
#define CHARGE_TIME 1.6
// number of times it will cycle through the full charge
// animation before automatically firing off. Will give
// warning sound on last cycle. Each cycle is 2 seconds long
#define MAX_FULL_CYCLES 4

class EXPORT_FROM_DLL PBolt : public Projectile
{
private:
   float speed;

public:
   CLASS_PROTOTYPE(PBolt);

   int charge; // stored here as a percentage of full charge

   void EnergyTrail(Event *ev);
   void BoltTouch(Event *ev);
   void Explode(Event *ev);
   void Setup(Entity *owner, Vector pos, Vector dir, int firecharge);

   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void PBolt::Archive(Archiver &arc)
{
   Projectile::Archive(arc);

   arc.WriteFloat(speed);
   arc.WriteInteger(charge);
}

inline EXPORT_FROM_DLL void PBolt::Unarchive(Archiver &arc)
{
   Projectile::Unarchive(arc);

   speed = arc.ReadFloat();
   charge = arc.ReadInteger();
}

class EXPORT_FROM_DLL PlasmaBow : public Weapon
{
public:
   CLASS_PROTOTYPE(PlasmaBow);

   int   fullcycles; // number of cycles for full charge animation
   float chargetime; // starting time of charging
   int   drainammo;  // amount of ammo to drain at that time;

   PlasmaBow();
   virtual void SecondaryUse(Event *ev)  override;
   virtual void Fire()                   override;
   virtual void FullRetain(Event *ev);
   virtual void Shoot(Event *ev);
   virtual void ReleaseCheck(Event *ev);

   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void PlasmaBow::Archive(Archiver &arc)
{
   Weapon::Archive(arc);

   arc.WriteInteger(fullcycles);
   arc.WriteFloat(chargetime);
   arc.WriteInteger(drainammo);
}

inline EXPORT_FROM_DLL void PlasmaBow::Unarchive(Archiver &arc)
{
   Weapon::Unarchive(arc);

    arc.ReadInteger(&fullcycles);
    arc.ReadFloat(&chargetime);
    arc.ReadInteger(&drainammo);
}

#endif /* crossbow.h */

// EOF

