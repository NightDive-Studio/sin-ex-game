/*
================================================================
HOVERBIKE WEAPONS
================================================================

Copyright (C) 2020 by Night Dive Studios, Inc.
All rights reserved.

See the license.txt file for conditions and terms of use for this code.

serves as both the view model and the world model
for when a bike has a rider. Also does all the weapon
related stuff for hoverbikes.
*/

#ifndef __HOVERWEAP_H__
#define __HOVERWEAP_H__

#include "g_local.h"
#include "entity.h"
#include "item.h"
#include "weapon.h"
#include "specialfx.h"
#include "hoverbike.h"
#include "sentient.h"

class EXPORT_FROM_DLL HoverWeap : public Weapon
{
private:
   qboolean     damagedtarget;
   qboolean     attached;
   HoverbikePtr bike;

protected:
   virtual void AttachGun() override;
   virtual void DetachGun() override;

public:
   int side    = 1; // for alternating the firing side
   int rockets;     // rocket ammo count
   int bullets;     // bullet ammo count
   int mines;       // mine ammo count

   CLASS_PROTOTYPE(HoverWeap);

   HoverWeap();
   virtual void Fire() override;
   virtual void Shoot(Event *ev);
   virtual qboolean Drop() override;

   virtual void TraceAttack(const Vector &start, const Vector &end, int damage, trace_t *trace, int numricochets, 
                            int kick, int dflags, int meansofdeath, qboolean server_effects);
   virtual void FireBullets(Vector src, Vector dir, int numbullets, Vector spread, int mindamage, int maxdamage, 
                            int dflags, int meansofdeath, qboolean server_effects);

   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void HoverWeap::Archive(Archiver &arc)
{
   Weapon::Archive(arc);

   arc.WriteBoolean(damagedtarget);
   arc.WriteBoolean(attached);
   arc.WriteSafePointer(bike);

   arc.WriteInteger(side);
   arc.WriteInteger(rockets);
   arc.WriteInteger(bullets);
   arc.WriteInteger(mines);
}

inline EXPORT_FROM_DLL void HoverWeap::Unarchive(Archiver &arc)
{
   Weapon::Unarchive(arc);

   arc.ReadBoolean(&damagedtarget);
   arc.ReadBoolean(&attached);
   arc.ReadSafePointer(&bike);

   arc.ReadInteger(&side);
   arc.ReadInteger(&rockets);
   arc.ReadInteger(&bullets);
   arc.ReadInteger(&mines);
}

class EXPORT_FROM_DLL HBRocket : public Projectile
{
private:
   float speed;
   int   owner;
   int   bike;
   int   frontbox;
   int   backbox;

public:
   CLASS_PROTOTYPE(HBRocket);

   void         Explode(Event *ev);
   virtual void Setup(Entity *owner, Vector pos, Vector dir) override;

   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void HBRocket::Archive(Archiver &arc)
{
   Projectile::Archive(arc);

   arc.WriteFloat(speed);
   arc.WriteInteger(owner);
   arc.WriteInteger(bike);
   arc.WriteInteger(frontbox);
   arc.WriteInteger(backbox);
}

inline EXPORT_FROM_DLL void HBRocket::Unarchive(Archiver &arc)
{
   Projectile::Unarchive(arc);

   arc.ReadFloat(&speed);
   arc.ReadInteger(&owner);
   arc.ReadInteger(&bike);
   arc.ReadInteger(&frontbox);
   arc.ReadInteger(&backbox);
}

class EXPORT_FROM_DLL HBMine : public Projectile
{
private:
   int owner;
   float detonate_time;

public:
   CLASS_PROTOTYPE(HBMine);

   void         Explode(Event *ev);
   void         Detect(Event *ev);
   virtual void Setup(Entity *owner, Vector pos, Vector dir) override;

   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void HBMine::Archive(Archiver &arc)
{
   Projectile::Archive(arc);

   arc.WriteInteger(owner);
   arc.WriteFloat(detonate_time);
}

inline EXPORT_FROM_DLL void HBMine::Unarchive(Archiver &arc)
{
   Projectile::Unarchive(arc);

   arc.ReadInteger(&owner);
   arc.ReadFloat(&detonate_time);
}

#endif /* hoverweap.h */

// EOF
