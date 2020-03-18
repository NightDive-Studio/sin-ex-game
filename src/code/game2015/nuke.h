/*
================================================================
NUKE LAUNCHER aka IP36
================================================================

Copyright (C) 2020 by Night Dive Studios, Inc.
All rights reserved.

See the license.txt file for conditions and terms of use for this code.
*/

#ifndef __NUKE_H__
#define __NUKE_H__

#include "g_local.h"
#include "item.h"
#include "weapon.h"
#include "misc.h"
#include "player.h"

// nukeball state flags
#define NBS_EVIL       -1
#define NBS_FLYING      0
#define NBS_COLLAPSING  1
#define NBS_EXPLODING   2
#define NBS_RING1       3
#define NBS_RING2       4

// takes care of applying the flash to players
class EXPORT_FROM_DLL NukeFlash : public Entity
{
private:
   Vector fblend;
   float falpha;

public:
   CLASS_PROTOTYPE(NukeFlash);

   virtual void Flash(Event *ev);
   virtual void Setup(Vector pos, float delay);

   virtual void Archive(Archiver &arc);
   virtual void Unarchive(Archiver &arc);
};

inline EXPORT_FROM_DLL void NukeFlash::Archive(Archiver &arc)
{
   Entity::Archive(arc);

   arc.WriteVector(fblend);
   arc.WriteFloat(falpha);
}

inline EXPORT_FROM_DLL void NukeFlash::Unarchive(Archiver &arc)
{
   Entity::Unarchive(arc);

   arc.ReadVector(&fblend);
   arc.ReadFloat(&falpha);
}

// the visual fireballs that the player sees
//class EXPORT_FROM_DLL NukeFireball : public Entity
class EXPORT_FROM_DLL NukeFireball : public Projectile
{
private:
   int owner;

public:
   CLASS_PROTOTYPE(NukeFireball);

   virtual void Animate(Event *ev);
   virtual void DoDamage(Event *ev);
   virtual void Setup(Entity *owner, Vector pos, float life, int type);
   virtual void Fade(Event *ev);

   virtual void Archive(Archiver &arc);
   virtual void Unarchive(Archiver &arc);
};

inline EXPORT_FROM_DLL void NukeFireball::Archive(Archiver &arc)
{
   Projectile::Archive(arc);

   arc.WriteInteger(owner);
}

inline EXPORT_FROM_DLL void NukeFireball::Unarchive(Archiver &arc)
{
   Projectile::Unarchive(arc);

   arc.ReadInteger(&owner);
}

// this is the ball fired our of the gun
class EXPORT_FROM_DLL NukeBall : public Projectile
{
private:
   float speed;
   int ballstate;

public:
   CLASS_PROTOTYPE(NukeBall);

   void         Animate(Event *ev);
   void         Collapse(Event *ev);
   void         Explode(Event *ev);
   void         ThrowRing(Event *ev);
   void         ExplosionBall(Event *ev);
   void         Setup(Entity *owner, Vector pos, Vector dir);
   void         EvilSetup(Entity *owner, Vector pos, Vector dir); // for devious purposes...

   virtual void Archive(Archiver &arc);
   virtual void Unarchive(Archiver &arc);
};

inline EXPORT_FROM_DLL void NukeBall::Archive(Archiver &arc)
{
   Projectile::Archive(arc);

   arc.WriteFloat(speed);
   arc.WriteInteger(ballstate);
}

inline EXPORT_FROM_DLL void NukeBall::Unarchive(Archiver &arc)
{
   Projectile::Unarchive(arc);

   arc.ReadFloat(&speed);
   arc.ReadInteger(&ballstate);
}

class EXPORT_FROM_DLL IP36 : public Weapon
{
public:
   CLASS_PROTOTYPE(IP36);

   IP36();
   virtual void Shoot(Event *ev);  // starts the actual firing event
   virtual void Launch(Event *ev); // does the actual firing
};

#endif /* nuke.h */

// EOF

