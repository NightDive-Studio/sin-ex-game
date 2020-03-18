//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/thrall.h                         $
// $Revision:: 6                                                              $
//   $Author:: Aldie                                                          $
//     $Date:: 11/08/98 8:30p                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// ThrallMaster
// 

#ifndef __THRALLMASTER_H__
#define __THRALLMASTER_H__

#include "g_local.h"
#include "actor.h"
#include "specialfx.h"

class EXPORT_FROM_DLL ThrallMaster : public Actor
{
protected:
   weaponmode_t      weaponmode;
   str               gunbone;

public:
   CLASS_PROTOTYPE(ThrallMaster);

   ThrallMaster();
   virtual Vector    GunPosition() override;
   virtual qboolean  CanShootFrom(Vector pos, Entity *ent, qboolean usecurrentangles) override;
   virtual void      Chatter(const char *sound, float chance = 10, float volume = 1.0f, int channel = CHAN_VOICE) override;
   virtual void      WeaponUse(Event *ev) override;
   virtual void      FirePulse(Event *ev);
   virtual void      FireRockets(Event *ev);
   virtual void      GibFest(Event *ev);
   virtual void      Archive(Archiver &arc)   override;
   virtual void      Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void ThrallMaster::Archive(Archiver &arc)
{
   Actor::Archive(arc);

   arc.WriteInteger(weaponmode);
   arc.WriteString(gunbone);
}


inline EXPORT_FROM_DLL void ThrallMaster::Unarchive(Archiver &arc)
{
   Actor::Unarchive(arc);

   weaponmode = (weaponmode_t)arc.ReadInteger();
   arc.ReadString(&gunbone);
}

class EXPORT_FROM_DLL ThrallGun : public Weapon
{
public:
   CLASS_PROTOTYPE(ThrallGun);

   ThrallGun();
   virtual void      Shoot(Event *ev);
   virtual void      SecondaryUse(Event *ev) override;
};

class EXPORT_FROM_DLL DrunkMissile : public Projectile
{
private:
   EntityPtr target;

public:
   CLASS_PROTOTYPE(DrunkMissile);

   float          AdjustAngle(float maxadjust, float currangle, float targetangle);
   void           Explode(Event *ev);
   float          ResolveMinimumDistance(Entity *potential_target, float currmin);
   void           HeatSeek(Event *ev);
   virtual void   Prethink() override;
   virtual void   Setup(Entity *owner, Vector pos, Vector dir) override;
   virtual void   Archive(Archiver &arc)   override;
   virtual void   Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void DrunkMissile::Archive(Archiver &arc)
{
   Projectile::Archive(arc);

   arc.WriteSafePointer(target);
}

inline EXPORT_FROM_DLL void DrunkMissile::Unarchive(Archiver &arc)
{
   Projectile::Unarchive(arc);

   arc.ReadSafePointer(&target);
}

class EXPORT_FROM_DLL ThrallPulse : public Projectile
{
public:
   CLASS_PROTOTYPE(ThrallPulse);

   void         Explode(Event *ev);
   virtual void Setup(Entity *owner, Vector pos, Vector dir) override;
};

class EXPORT_FROM_DLL ThrallPulseDebris : public Projectile
{
private:
   float    size;
   float    spawntime;
   float    nexttouch;

public:
   CLASS_PROTOTYPE(ThrallPulseDebris);

   void         Touch(Event *ev);
   virtual void Prethink() override;
   void         Setup(Entity *owner, Vector pos, float size);
   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void ThrallPulseDebris::Archive(Archiver &arc)
{
   Projectile::Archive(arc);

   arc.WriteFloat(size);
   arc.WriteFloat(spawntime);
   arc.WriteFloat(nexttouch);
}

inline EXPORT_FROM_DLL void ThrallPulseDebris::Unarchive(Archiver &arc)
{
   Projectile::Unarchive(arc);

   arc.ReadFloat(&size);
   arc.ReadFloat(&spawntime);
   arc.ReadFloat(&nexttouch);
}

#endif /* thrall.h */

// EOF

