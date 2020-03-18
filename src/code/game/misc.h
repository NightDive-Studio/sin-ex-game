//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/misc.h                           $
// $Revision:: 71                                                             $
//   $Author:: Jimdose                                                        $
//     $Date:: 11/11/98 10:02p                                                $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Basically the big stew pot of the DLLs, or maybe a garbage bin, whichever
// metaphore you prefer.  This really should be cleaned up.  Anyway, this
// should contain utility functions that could be used by any entity.
// Right now it contains everything from entities that could be in their
// own file to my mother pot roast recipes.
// 

#ifndef __MISC_H__
#define __MISC_H__

#include "g_local.h"
#include "entity.h"
#include "trigger.h"
#include "queue.h"

void SendOverlay(Entity *ent, str overlayname);

void SendIntermission(Entity *ent, str intermissionname);

void SendDialog(const char *icon_name, const char *dialog_text);

const char *ExpandLocation(const char *location);

void MadeBreakingSound(Vector pos, Entity * activator);

qboolean OnSameTeam(Entity *ent1, Entity *ent2);

class EXPORT_FROM_DLL InfoNull : public Entity
{
public:
   CLASS_PROTOTYPE(InfoNull);

   InfoNull();
};

class EXPORT_FROM_DLL FuncRemove : public Entity
{
public:
   CLASS_PROTOTYPE(FuncRemove);

   FuncRemove();
};

class EXPORT_FROM_DLL InfoNotNull : public Entity
{
public:
   CLASS_PROTOTYPE(InfoNotNull);
};

class EXPORT_FROM_DLL Wall : public Entity
{
private:

public:
   CLASS_PROTOTYPE(Wall);

   Wall();
};

class EXPORT_FROM_DLL IllusionaryWall : public Entity
{
private:

public:
   CLASS_PROTOTYPE(IllusionaryWall);
   IllusionaryWall();
};

class EXPORT_FROM_DLL BreakawayWall : public TriggerOnce
{
private:

public:
   CLASS_PROTOTYPE(BreakawayWall);

   void  BreakWall(Event *ev);
   void  Setup(Event *ev);
   BreakawayWall();
};

class EXPORT_FROM_DLL ExplodingWall : public Trigger
{
protected:
   int      dmg;
   int      explosions;
   float    attack_finished;
   Vector   land_angles;
   float    land_radius;
   float    angle_speed;
   int      state;
   Vector   orig_mins, orig_maxs;
   qboolean on_ground;

public:
   CLASS_PROTOTYPE(ExplodingWall);

   ExplodingWall();
   virtual void   SetupSecondStage();
   virtual void   Explode(Event *ev);
   virtual void   DamageEvent(Event *ev);
   virtual void   GroundDamage(Event *ev);
   virtual void   TouchFunc(Event *ev);
   virtual void   StopRotating(Event *ev);
   virtual void   CheckOnGround(Event *ev);
   virtual void   Archive(Archiver &arc)   override;
   virtual void   Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void ExplodingWall::Archive(Archiver &arc)
{
   Trigger::Archive(arc);

   arc.WriteInteger(dmg);
   arc.WriteInteger(explosions);
   arc.WriteFloat(attack_finished);
   arc.WriteVector(land_angles);
   arc.WriteFloat(land_radius);
   arc.WriteFloat(angle_speed);
   arc.WriteInteger(state);
   arc.WriteVector(orig_mins);
   arc.WriteVector(orig_maxs);
   arc.WriteBoolean(on_ground);
}

inline EXPORT_FROM_DLL void ExplodingWall::Unarchive(Archiver &arc)
{
   Trigger::Unarchive(arc);

   arc.ReadInteger(&dmg);
   arc.ReadInteger(&explosions);
   arc.ReadFloat(&attack_finished);
   arc.ReadVector(&land_angles);
   arc.ReadFloat(&land_radius);
   arc.ReadFloat(&angle_speed);
   arc.ReadInteger(&state);
   arc.ReadVector(&orig_mins);
   arc.ReadVector(&orig_maxs);
   arc.ReadBoolean(&on_ground);
}

class EXPORT_FROM_DLL Electrocute : public Trigger
{
private:
   float radius;

public:
   CLASS_PROTOTYPE(Electrocute);

   Electrocute();
   virtual void   KillSight(Event *ev);
   virtual void   Archive(Archiver &arc)   override;
   virtual void   Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void Electrocute::Archive(Archiver &arc)
{
   Trigger::Archive(arc);

   arc.WriteFloat(radius);
}

inline EXPORT_FROM_DLL void Electrocute::Unarchive(Archiver &arc)
{
   Trigger::Unarchive(arc);

   arc.ReadFloat(&radius);
}

class EXPORT_FROM_DLL Detail : public Wall
{
public:
   CLASS_PROTOTYPE(Detail);

   Detail();
};

class EXPORT_FROM_DLL Teleporter : public Trigger
{
public:
   CLASS_PROTOTYPE(Teleporter);

   Teleporter();
   virtual void Teleport(Event *ev);
};

class EXPORT_FROM_DLL TeleporterDestination : public Entity
{
public:
   Vector movedir;

   CLASS_PROTOTYPE(TeleporterDestination);

   TeleporterDestination();
   virtual void      Archive(Archiver &arc)   override;
   virtual void      Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void TeleporterDestination::Archive(Archiver &arc)
{
   Entity::Archive(arc);

   arc.WriteVector(movedir);
}

inline EXPORT_FROM_DLL void TeleporterDestination::Unarchive(Archiver &arc)
{
   Entity::Unarchive(arc);

   arc.ReadVector(&movedir);
}

class EXPORT_FROM_DLL Waypoint : public Entity
{
public:
   CLASS_PROTOTYPE(Waypoint);
};

class EXPORT_FROM_DLL Shatter : public DamageThreshold
{
protected:
   qboolean       threshold = false;
   str            noise;
public:
   CLASS_PROTOTYPE(Shatter);
   Shatter();

   virtual void   DamageEvent(Event *ev);
   virtual void   DoShatter(Event *ev);
   virtual void   Archive(Archiver &arc)   override;
   virtual void   Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void Shatter::Archive(Archiver &arc)
{
   DamageThreshold::Archive(arc);

   arc.WriteBoolean(threshold);
   arc.WriteString(noise);
}

inline EXPORT_FROM_DLL void Shatter::Unarchive(Archiver &arc)
{
   DamageThreshold::Unarchive(arc);

   arc.ReadBoolean(&threshold);
   arc.ReadString(&noise);
}

class EXPORT_FROM_DLL Glass : public Shatter
{
public:
   CLASS_PROTOTYPE(Glass);
   Glass();
};

class EXPORT_FROM_DLL Spawn : public Entity
{
protected:
   str      modelname;
   str      spawntargetname;
   int      attackmode;

public:
   CLASS_PROTOTYPE(Spawn);
   Spawn();

   virtual void   DoSpawn(Event *ev);
   virtual void   Archive(Archiver &arc)   override;
   virtual void   Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void Spawn::Archive(Archiver &arc)
{
   Entity::Archive(arc);

   arc.WriteString(modelname);
   arc.WriteString(spawntargetname);
   arc.WriteInteger(attackmode);
}

inline EXPORT_FROM_DLL void Spawn::Unarchive(Archiver &arc)
{
   Entity::Unarchive(arc);

   arc.ReadString(&modelname);
   arc.ReadString(&spawntargetname);
   arc.ReadInteger(&attackmode);
}

class EXPORT_FROM_DLL ReSpawn : public Spawn
{
public:
   CLASS_PROTOTYPE(ReSpawn);

   virtual void DoSpawn(Event *ev);
};

class EXPORT_FROM_DLL SpawnOutOfSight : public Spawn
{
public:
   CLASS_PROTOTYPE(SpawnOutOfSight);

   virtual void DoSpawn(Event *ev);
};

class EXPORT_FROM_DLL SpawnChain : public Spawn
{
public:
   CLASS_PROTOTYPE(SpawnChain);

   virtual void DoSpawn(Event *ev);
};

class EXPORT_FROM_DLL Oxygenator : public Trigger
{
private:
   float time = 20.0f;

public:
   CLASS_PROTOTYPE(Oxygenator);

   Oxygenator();
   virtual void   Oxygenate(Event *ev);
   virtual void   Archive(Archiver &arc)   override;
   virtual void   Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void Oxygenator::Archive(Archiver &arc)
{
   Trigger::Archive(arc);

   arc.WriteFloat(time);
}

inline EXPORT_FROM_DLL void Oxygenator::Unarchive(Archiver &arc)
{
   Trigger::Unarchive(arc);

   arc.ReadFloat(&time);
}

class EXPORT_FROM_DLL BloodSplat : public Entity
{
private:
   // No archive function is declared since the class automaintains itself.
   // Just spawning and freeing maintains the queue.

   static int     numBloodSplats;
   static Queue   queueBloodSplats;

public:
   CLASS_PROTOTYPE(BloodSplat);

   BloodSplat(Vector pos = { 0, 0, 0 }, Vector ang = { 0, 0, 0 }, float scale = 1);
   ~BloodSplat();
};

class EXPORT_FROM_DLL ClipBox : public Entity
{
public:
   CLASS_PROTOTYPE(ClipBox);

   ClipBox();
};

#endif /* misc.h */

// EOF

