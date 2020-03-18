/*
================================================================
STINGER PACK WEAPON
================================================================

Copyright (C) 2020 by Night Dive Studios, Inc.
All rights reserved.

See the license.txt file for conditions and terms of use for this code.
*/

#ifndef __ROCKETPACK_H__
#define __ROCKETPACK_H__

#include "g_local.h"
#include "item.h"
#include "weapon.h"
#include "specialfx.h"

class EXPORT_FROM_DLL StingerRocket : public Projectile
{
private:
   float  speed;
   Vector movedir; // primary direction of travel
   Vector targpos; // where it should me going
   Vector destpos; // where it is going
   Vector offsets; // +/- offsets for swirving

public:
   CLASS_PROTOTYPE(StingerRocket);

   void Turn(Event *ev);
   void Explode(Event *ev);
   void Setup(Entity *owner, Vector pos, Vector firedest, Vector dir, Vector firedir);

   virtual void Archive(Archiver &arc);
   virtual void Unarchive(Archiver &arc);
};

inline EXPORT_FROM_DLL void StingerRocket::Archive(Archiver &arc)
{
   Projectile::Archive(arc);

   arc.WriteFloat(speed);
   arc.WriteVector(movedir);
   arc.WriteVector(targpos);
   arc.WriteVector(destpos);
   arc.WriteVector(offsets);
}

inline EXPORT_FROM_DLL void StingerRocket::Unarchive(Archiver &arc)
{
   Projectile::Unarchive(arc);

   arc.ReadFloat(&speed);
   arc.ReadVector(&movedir);
   arc.ReadVector(&targpos);
   arc.ReadVector(&destpos);
   arc.ReadVector(&offsets);
}

class EXPORT_FROM_DLL StingerPack : public Weapon
{
private:
   qboolean attached;
   int rocketnum; // for tracking firing positions

protected:
   virtual void AttachGun(void);
   virtual void DetachGun(void);

public:
   CLASS_PROTOTYPE(StingerPack);

   StingerPack();
   virtual void Shoot(Event *ev);
   virtual void SecondaryUse(Event *ev);

   virtual void Archive(Archiver &arc);
   virtual void Unarchive(Archiver &arc);
};

inline EXPORT_FROM_DLL void StingerPack::Archive(Archiver &arc)
{
   Weapon::Archive(arc);

   arc.WriteBoolean(attached);
   arc.WriteInteger(rocketnum);
}

inline EXPORT_FROM_DLL void StingerPack::Unarchive(Archiver &arc)
{
   Weapon::Unarchive(arc);

   arc.ReadBoolean(&attached);
   arc.ReadInteger(&rocketnum);
}

#endif /* rocketpack.h */

// EOF

