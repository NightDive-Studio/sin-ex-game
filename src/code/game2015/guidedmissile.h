/*
================================================================
GUIDED MISSILE LAUNCHER
================================================================

Copyright (C) 2020 by Night Dive Studios, Inc.
All rights reserved.

See the license.txt file for conditions and terms of use for this code.
*/

#ifndef __GUIDEDMISSILE_H__
#define __GUIDEDMISSILE_H__

#include "g_local.h"
#include "item.h"
#include "weapon.h"
#include "misc.h"
#include "player.h"

#define MISSILE_SPEED 450

class EXPORT_FROM_DLL MissileView : public Entity
{
private:
   EntityPtr  missile;
   EntityPtr  owner;
   viewmode_t oldviewmode;
   float      removetime = 0.0f;
   float      starttime;

public:
   CLASS_PROTOTYPE(MissileView);

   ~MissileView();

   virtual void Setup(Entity *missile, int owner, viewmode_t oldvmode);
   virtual void SetupMissile();

   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void MissileView::Archive(Archiver &arc)
{
   Entity::Archive(arc);

   arc.WriteSafePointer(missile);
   arc.WriteSafePointer(owner);
   arc.WriteInteger(oldviewmode);
   arc.WriteFloat(removetime);
   arc.WriteFloat(starttime);
}

inline EXPORT_FROM_DLL void MissileView::Unarchive(Archiver &arc)
{
   int temp;

   Entity::Unarchive(arc);

   arc.ReadSafePointer(&missile);
   arc.ReadSafePointer(&owner);
   arc.ReadInteger(&temp);
   oldviewmode = (viewmode_t)temp;
   arc.ReadFloat(&removetime);
   arc.ReadFloat(&starttime);
}

template class EXPORT_FROM_DLL SafePtr<MissileView>;
typedef SafePtr<MissileView> MissileViewPtr;

class EXPORT_FROM_DLL Missile : public Projectile
{
private:
   float          speed;
   MissileViewPtr missileview;

public:
   CLASS_PROTOTYPE(Missile);

   void         Explode(Event *ev);
   virtual void Setup(Entity *owner, Vector pos, Vector dir) override;
   void         StartOpen(Event *ev);
   void         FinishOpen(Event *ev);

   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void Missile::Archive(Archiver &arc)
{
   Projectile::Archive(arc);

   arc.WriteFloat(speed);
   arc.WriteSafePointer(missileview);
}

inline EXPORT_FROM_DLL void Missile::Unarchive(Archiver &arc)
{
   Projectile::Unarchive(arc);

   arc.ReadFloat(&speed);
   arc.ReadSafePointer(&missileview);
}

class EXPORT_FROM_DLL MissileLauncher : public Weapon
{
public:
   CLASS_PROTOTYPE(MissileLauncher);

   MissileLauncher();
   virtual void Shoot(Event *ev);
   virtual void SecondaryUse(Event *ev) override;
};

#endif /* guidedmissile.h */

// EOF

