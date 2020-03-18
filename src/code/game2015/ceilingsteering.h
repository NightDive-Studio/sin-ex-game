/*
================================================================
CEILING STEERING FOR CRAWLER
================================================================

Copyright (C) 2020 by Night Dive Studios, Inc.
All rights reserved.

See the license.txt file for conditions and terms of use for this code.
*/

#ifndef __CEILINGSTEERING_H__
#define __CEILINGSTEERING_H__

#include "g_local.h"
#include "entity.h"
#include "steering.h"

class Actor;

class EXPORT_FROM_DLL CeilingObstacleAvoidance : public Steering
{
protected:
   qboolean avoidwalls = true;

public:
   CLASS_PROTOTYPE(CeilingObstacleAvoidance);

   void             AvoidWalls(qboolean);
   virtual void     ShowInfo(Actor &self)    override;
   virtual qboolean Evaluate(Actor &self)    override;
   virtual void     Archive(Archiver &arc)   override;
   virtual void     Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void CeilingObstacleAvoidance::Archive(Archiver &arc)
{
   Steering::Archive(arc);

   arc.WriteBoolean(avoidwalls);
}

inline EXPORT_FROM_DLL void CeilingObstacleAvoidance::Unarchive(Archiver &arc)
{
   Steering::Unarchive(arc);

   avoidwalls = arc.ReadBoolean();
}

class EXPORT_FROM_DLL CeilingObstacleAvoidance2 : public Steering
{
protected:
   qboolean avoidwalls = true;

public:
   CLASS_PROTOTYPE(CeilingObstacleAvoidance2);

   void             AvoidWalls(qboolean);
   virtual void     ShowInfo(Actor &self)    override;
   virtual qboolean Evaluate(Actor &self)    override;
   virtual void     Archive(Archiver &arc)   override;
   virtual void     Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void CeilingObstacleAvoidance2::Archive(Archiver &arc)
{
   Steering::Archive(arc);

   arc.WriteBoolean(avoidwalls);
}

inline EXPORT_FROM_DLL void CeilingObstacleAvoidance2::Unarchive(Archiver &arc)
{
   Steering::Unarchive(arc);

   avoidwalls = arc.ReadBoolean();
}

class EXPORT_FROM_DLL CeilingChase : public Steering
{
private:
   Seek                     seek;
   Vector                   goal;
   EntityPtr                goalent     = nullptr;
   CeilingObstacleAvoidance avoid;
   float                    avoidtime;
   qboolean                 usegoal     = false;
                            
   Vector                   wanderstart;
   int                      wander;
   float                    wandertime;
   Turn                     turnto;
   str                      anim;
   int                      stuck;
   Vector                   avoidvec;

public:
   CLASS_PROTOTYPE(CeilingChase);

   void             SetGoalPos(Vector pos);
   void             SetTarget(Entity *ent);
   virtual void     ShowInfo(Actor &self)    override;
   Vector           ChooseRandomDirection(Actor &self);
   virtual void     Begin(Actor &self)       override;
   virtual qboolean Evaluate(Actor &self)    override;
   virtual void     End(Actor &self)         override;
   virtual void     Archive(Archiver &arc)   override;
   virtual void     Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void CeilingChase::Archive(Archiver &arc)
{
   Steering::Archive(arc);

   arc.WriteObject(&seek);
   arc.WriteVector(goal);
   arc.WriteSafePointer(goalent);
   arc.WriteObject(&avoid);
   arc.WriteFloat(avoidtime);
   arc.WriteBoolean(usegoal);
   arc.WriteVector(wanderstart);
   arc.WriteInteger(wander);
   arc.WriteFloat(wandertime);
   arc.WriteObject(&turnto);
   arc.WriteString(anim);
   arc.WriteInteger(stuck);
   arc.WriteVector(avoidvec);
}

inline EXPORT_FROM_DLL void CeilingChase::Unarchive(Archiver &arc)
{
   Steering::Unarchive(arc);

   arc.ReadObject(&seek);
   arc.ReadVector(&goal);
   arc.ReadSafePointer(&goalent);
   arc.ReadObject(&avoid);
   arc.ReadFloat(&avoidtime);
   arc.ReadBoolean(&usegoal);
   arc.ReadVector(&wanderstart);
   arc.ReadInteger(&wander);
   arc.ReadFloat(&wandertime);
   arc.ReadObject(&turnto);
   arc.ReadString(&anim);
   arc.ReadInteger(&stuck);
   arc.ReadVector(&avoidvec);
}

#endif /* ceilingsteering.h */

// EOF

