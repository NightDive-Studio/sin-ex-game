//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/specialfx.h                      $
// $Revision:: 24                                                             $
//   $Author:: Markd                                                          $
//     $Date:: 11/15/98 11:34p                                                $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// $Log:: /Quake 2 Engine/Sin/code/game/specialfx.h                           $
// 
// DESCRIPTION:
// special effects
// 

#ifndef __SPECIAL_FX_H__
#define __SPECIAL_FX_H__

#include "g_local.h"
#include "entity.h"
#include "trigger.h"
#include "light.h"
#include "scriptslave.h"

void SpawnBlastDamage( trace_t *trace, int damage, Entity *attacker );
void Particles( Vector org, Vector norm, int count, int lightstyle, int flags );
void SpawnBlood( Vector org, Vector vel, int damage );
void SpawnSparks( Vector org,	Vector norm, int count );
void BurnWall( Vector org, Vector end, int amount );
void SpawnRocketExplosion( Vector org );
void SpawnScaledExplosion(	Vector org, float scale	);
void SpawnTeleportEffect( Vector org, int lightstyle );
void SpawnBeam(Vector start, Vector end, int parent_entnum, int modelindex, float alpha, float life, int flags);
void SpawnTempDlight(Vector org, float r, float g,  float b, float radius,  float decay, float life);

void TempModel(Entity * parent, Vector origin, Vector angles, const char *modelname, int anim,
               float scale, float alpha,  int flags, float life);

void TesselateModel(Entity * ent, int min_size, int max_size, Vector dir, float power, float percentage,
                    int thickness, Vector origin, int type=TESS_DEFAULT_TYPE, int lightstyle=TESS_DEFAULT_LIGHTSTYLE);

void ChangeMusic(const char *current, const char *fallback, qboolean force);

void ChangeSoundtrack(const char * soundtrack);

class EXPORT_FROM_DLL Bubble : public Entity
{
public:
   CLASS_PROTOTYPE(Bubble);

   virtual void Think(Event *ev);
   virtual void Touch(Event *ev);
   virtual void Setup(Vector pos);
};

class EXPORT_FROM_DLL Beam : public Entity
{
private:
   Vector      start;
   Vector      end;

public:
   CLASS_PROTOTYPE(Beam);

   Beam();
   void         setBeam(Vector start, Vector end, int diameter, float r, float g, float b, float alpha, float lifespan);
   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void Beam::Archive(Archiver &arc)
{
   Entity::Archive(arc);
   arc.WriteVector(start);
   arc.WriteVector(end);
}

inline EXPORT_FROM_DLL void Beam::Unarchive(Archiver &arc)
{
   Entity::Unarchive(arc);
   arc.ReadVector(&start);
   arc.ReadVector(&end);
}

class EXPORT_FROM_DLL Projectile : public Entity
{
protected:
   int      owner;

public:
   CLASS_PROTOTYPE(Projectile);

   float    fov;
   
   Projectile();
   Entity       *Owner(); //###
   virtual void  Setup(Entity *owner, Vector pos, Vector dir);
   virtual void  Archive(Archiver &arc)   override;
   virtual void  Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void Projectile::Archive(Archiver &arc)
{
   Entity::Archive(arc);
   arc.WriteInteger(owner);
   arc.WriteFloat(fov);
}

inline EXPORT_FROM_DLL void Projectile::Unarchive(Archiver &arc)
{
   Entity::Unarchive(arc);
   arc.ReadInteger(&owner);
   arc.ReadFloat(&fov);
}

class EXPORT_FROM_DLL FireSprite : public Light
{
public:
   CLASS_PROTOTYPE(FireSprite);

   FireSprite();
   ~FireSprite();
};

class EXPORT_FROM_DLL FuncBeam : public ScriptSlave
{
protected:
   Entity     *end;
   float       damage;
   float       life;

public:
   CLASS_PROTOTYPE(FuncBeam);

   FuncBeam();
   void         Activate(Event *ev);
   void         Deactivate(Event *ev);
   void         SetDiameter(Event *ev);
   void         SetLightstyle(Event *ev);
   void         SetMaxoffset(Event *ev);
   void         SetMinoffset(Event *ev);
   void         SetColor(Event *ev);
   void         SetTarget(Event *ev);
   void         Shoot(Vector start, Vector end, int diameter);
   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void FuncBeam::Archive(Archiver &arc)
{
   ScriptSlave::Archive(arc);
   arc.WriteObjectPointer(end);
   arc.WriteFloat(damage);
   arc.WriteFloat(life);
}

inline EXPORT_FROM_DLL void FuncBeam::Unarchive(Archiver &arc)
{
   ScriptSlave::Unarchive(arc);
   arc.ReadObjectPointer((Class **)&end);
   arc.ReadFloat(&damage);
   arc.ReadFloat(&life);
}

class EXPORT_FROM_DLL Sprite : public Trigger
{
public:
   CLASS_PROTOTYPE(Sprite);
   Sprite();

   void        Activate(Event *ev);
   void        Deactivate(Event *ev);
};

//
//### 2015 specialfx
//

// for flamethrower
void SpawnThrowerFlame(Vector org, Vector dest); 
void SpawnThrowerFlameRow(Vector org, Vector dest, Vector org2, Vector dest2);
void SpawnThrowerFlameHit(Vector org, Vector dir);
// for misc flames
void SpawnFlame(Vector org, Vector dira, Vector dest, Vector dirb, int count, int style, int size);
// for hoverbike vertical booster
void SpawnHoverBoost(Vector org, int yaw);
// for nuke
void SpawnNukeExplosion(Vector org,	int size, int lightstyle); 
// for plasma bow
void SpawnBowExplosion(Vector org,	int size); 
// variable sized particles
void SizedParticles(Vector org, Vector norm, int count, int lightstyle, int flags, int size, int growrate); 
// fully controllable particles
void FullParticles(Vector org, Vector norm, int count, int lightstyle, int flags, int size, int growrate, int speed, int spread, Vector accel);

class EXPORT_FROM_DLL FireHurtField : public TriggerUse
{
protected:
   float     damage;
   qboolean  fireon;
   EntityPtr parentfire;

   void      Hurt(Event *ev);
   void      SetHurt(Event *ev);

public:
   CLASS_PROTOTYPE(FireHurtField);

   FireHurtField();
   void         Setup(Vector minpos, Vector maxpos, int firedamage);
   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void FireHurtField::Archive (Archiver &arc)
{
   TriggerUse::Archive( arc );

   arc.WriteFloat(damage);
   arc.WriteBoolean(fireon);
   arc.WriteSafePointer(parentfire);
}

inline EXPORT_FROM_DLL void FireHurtField::Unarchive (Archiver &arc)
{
   TriggerUse::Unarchive( arc );

   arc.ReadFloat(&damage);
   arc.ReadBoolean(&fireon);
   arc.ReadSafePointer(&parentfire);
}

template class SafePtr<FireHurtField>;
typedef SafePtr<FireHurtField> FireHurtFieldPtr;

class EXPORT_FROM_DLL FuncFire : public Entity
{
private:
   int    flaming;
   int    flamecount;
   Vector flameend;
   Vector flamevela;
   Vector flamevelb;
   float  flamedelay;
   float  flamerandom;
   int    flametype;
   int    flamesize;
   int    flamedamage;
   FireHurtFieldPtr hurtfield;

public:
   CLASS_PROTOTYPE(FuncFire);

   FuncFire();
   virtual void Animate(Event *ev);
   virtual void ToggleFlame(Event *ev);

   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void FuncFire::Archive (Archiver &arc)
{
   Entity::Archive(arc);

   arc.WriteInteger(flaming);
   arc.WriteInteger(flamecount);
   arc.WriteVector(flameend);
   arc.WriteVector(flamevela);
   arc.WriteVector(flamevelb);
   arc.WriteFloat(flamedelay);
   arc.WriteFloat(flamerandom);
   arc.WriteInteger(flametype);
   arc.WriteInteger(flamesize);
   arc.WriteSafePointer(hurtfield);
   arc.WriteInteger(flamedamage);
}

inline EXPORT_FROM_DLL void FuncFire::Unarchive (Archiver &arc)
{
   Entity::Unarchive( arc );

   arc.ReadInteger(&flaming);
   arc.ReadInteger(&flamecount);
   arc.ReadVector(&flameend);
   arc.ReadVector(&flamevela);
   arc.ReadVector(&flamevelb);
   arc.ReadFloat(&flamedelay);
   arc.ReadFloat(&flamerandom);
   arc.ReadInteger(&flametype);
   arc.ReadInteger(&flamesize);
   arc.ReadSafePointer(&hurtfield);
   arc.ReadInteger(&flamedamage);
}

void SpawnCrawlerSpitDamage(trace_t *trace, int damage, Entity *attacker);
//###

#endif /* specialfx.h */

// EOF

