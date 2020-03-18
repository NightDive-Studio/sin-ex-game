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
   virtual void Setup(Entity *owner, Vector pos, Vector dir);
   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;
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

#endif /* specialfx.h */

// EOF

