/*
================================================================
INFORMER GUN
================================================================

Copyright (C) 2020 by Night Dive Studios, Inc.
All rights reserved.

See the license.txt file for conditions and terms of use for this code.
*/

#ifndef __INFORMERGUN_H__
#define __INFORMERGUN_H__

#include "g_local.h"
#include "item.h"
#include "weapon.h"

class EXPORT_FROM_DLL InformerBeam : public Entity
{
private:
   Vector start;
   Vector end;

public:
   int beamstate;

   CLASS_PROTOTYPE(InformerBeam);

   InformerBeam();
   void         setBeam(Vector start, Vector end, int diameter, float r, float g, float b, float alpha, float lifespan);
   void         FadeBeam(Event *ev);
   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void InformerBeam::Archive(Archiver &arc)
{
   Entity::Archive(arc);
   arc.WriteVector(start);
   arc.WriteVector(end);
   arc.WriteInteger(beamstate);
}

inline EXPORT_FROM_DLL void InformerBeam::Unarchive(Archiver &arc)
{
   Entity::Unarchive(arc);
   arc.ReadVector(&start);
   arc.ReadVector(&end);
   arc.ReadInteger(&beamstate);
}

class EXPORT_FROM_DLL InformerGun : public Weapon
{
private:
   EntityPtr fadingbeam;
   int       beamstate;
   Vector    beamstart;
   Vector    beamvec;

public:
   CLASS_PROTOTYPE(InformerGun);

   InformerGun();
   virtual void     Shoot(Event *ev);
   virtual void     Informify(Entity *ent);
   virtual qboolean AutoChange()             override;
   virtual qboolean Drop()                   override;
   virtual qboolean ReadyToChange()          override;

   virtual void     Archive(Archiver &arc)   override;
   virtual void     Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void InformerGun::Archive(Archiver &arc)
{
   Weapon::Archive(arc);

   arc.WriteSafePointer(fadingbeam);
   arc.WriteInteger(beamstate);
   arc.WriteVector(beamstart);
   arc.WriteVector(beamvec);
}

inline EXPORT_FROM_DLL void InformerGun::Unarchive(Archiver &arc)
{
   Weapon::Unarchive(arc);

   arc.ReadSafePointer(&fadingbeam);
   arc.ReadInteger(&beamstate);
   arc.ReadVector(&beamstart);
   arc.ReadVector(&beamvec);
}

#endif /* informergun.h */

// EOF
