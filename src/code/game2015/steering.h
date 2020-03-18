//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/steering.h                       $
// $Revision:: 7                                                              $
//   $Author:: Jimdose                                                        $
//     $Date:: 10/25/98 5:00a                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Steering behaviors for AI.
// 

#ifndef __STEERING_H__
#define __STEERING_H__

#include "g_local.h"
#include "entity.h"
#include "path.h"

class Actor;

class EXPORT_FROM_DLL Steering : public Listener
{
public:
   Vector               steeringforce;
   Vector               origin;
   Vector               movedir;
   float                maxspeed = 320.0f;

   CLASS_PROTOTYPE(Steering);

   virtual void         ShowInfo(Actor &self);
   virtual void         Begin(Actor &self);
   virtual qboolean     Evaluate(Actor &self);
   virtual void         End(Actor &self);

   void                 ResetForces();
   void                 SetPosition(Vector pos);
   void                 SetDir(Vector dir);
   void                 SetMaxSpeed(float speed);

   virtual void         DrawForces();
   virtual void         Archive(Archiver &arc)   override;
   virtual void         Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void Steering::Archive(Archiver &arc)
{
   Listener::Archive(arc);

   arc.WriteVector(steeringforce);
   arc.WriteVector(origin);
   arc.WriteVector(movedir);
   arc.WriteFloat(maxspeed);
}

inline EXPORT_FROM_DLL void Steering::Unarchive(Archiver &arc)
{
   Listener::Unarchive(arc);

   arc.ReadVector(&steeringforce);
   arc.ReadVector(&origin);
   arc.ReadVector(&movedir);
   arc.ReadFloat(&maxspeed);
}

class EXPORT_FROM_DLL Seek : public Steering
{
protected:
   Vector               targetposition;
   Vector               targetvelocity;

public:
   CLASS_PROTOTYPE(Seek);

   void                 SetTargetPosition(Vector pos);
   void                 SetTargetVelocity(Vector vel);
   virtual void         ShowInfo(Actor &self)    override;
   virtual qboolean     Evaluate(Actor &self)    override;
   virtual void         Archive(Archiver &arc)   override;
   virtual void         Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void Seek::Archive(Archiver &arc)
{
   Steering::Archive(arc);

   arc.WriteVector(targetposition);
   arc.WriteVector(targetvelocity);
}

inline EXPORT_FROM_DLL void Seek::Unarchive(Archiver &arc)
{
   Steering::Unarchive(arc);

   arc.ReadVector(&targetposition);
   arc.ReadVector(&targetvelocity);
}

class EXPORT_FROM_DLL ObstacleAvoidance : public Steering
{
protected:
   qboolean avoidwalls = true;

public:
   CLASS_PROTOTYPE(ObstacleAvoidance);

   void                 AvoidWalls(qboolean);
   virtual void         ShowInfo(Actor &self)    override;
   virtual qboolean     Evaluate(Actor &self)    override;
   virtual void         Archive(Archiver &arc)   override;
   virtual void         Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void ObstacleAvoidance::Archive(Archiver &arc)
{
   Steering::Archive(arc);

   arc.WriteBoolean(avoidwalls);
}

inline EXPORT_FROM_DLL void ObstacleAvoidance::Unarchive(Archiver &arc)
{
   Steering::Unarchive(arc);

   arc.ReadBoolean(&avoidwalls);
}

class EXPORT_FROM_DLL ObstacleAvoidance2 : public Steering
{
protected:
   qboolean avoidwalls = true;

public:
   CLASS_PROTOTYPE(ObstacleAvoidance2);

   void                 AvoidWalls(qboolean);
   virtual void         ShowInfo(Actor &self)    override;
   virtual qboolean	    Evaluate(Actor &self)    override;
   virtual void         Archive(Archiver &arc)   override;
   virtual void         Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void ObstacleAvoidance2::Archive(Archiver &arc)
{
   Steering::Archive(arc);

   arc.WriteBoolean(avoidwalls);
}

inline EXPORT_FROM_DLL void ObstacleAvoidance2::Unarchive(Archiver &arc)
{
   Steering::Unarchive(arc);

   arc.ReadBoolean(&avoidwalls);
}

class EXPORT_FROM_DLL FollowPath : public Steering
{
protected:
   PathPtr              path        = nullptr;
   Seek                 seek;
   PathNodePtr          currentNode = nullptr;

   void                 FindCurrentNode(Actor &self);

public:
   CLASS_PROTOTYPE(FollowPath);

   ~FollowPath();
   void                 SetPath(Path *newpath);
   Path                *SetPath(Actor &self, Vector from, Vector to);
   qboolean             DoneWithPath(Actor &self);
   virtual void         ShowInfo(Actor &self)    override;
   virtual void         Begin(Actor &self)       override;
   virtual qboolean     Evaluate(Actor &self)    override;
   virtual void         End(Actor &self)         override;
   virtual void         DrawForces(void)         override;
   virtual void         Archive(Archiver &arc)   override;
   virtual void         Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void FollowPath::Archive(Archiver &arc)
{
   Steering::Archive(arc);

   arc.WriteSafePointer(path);
   arc.WriteObject(&seek);
   arc.WriteSafePointer(currentNode);
}

inline EXPORT_FROM_DLL void FollowPath::Unarchive(Archiver &arc)
{
   Steering::Unarchive(arc);

   arc.ReadSafePointer(&path);
   arc.ReadObject(&seek);
   arc.ReadSafePointer(&currentNode);
}

class EXPORT_FROM_DLL Turn : public Steering
{
private:
   Seek      seek;
   EntityPtr ent  = nullptr;
   Vector    dir  = { 1.0f, 0.0f, 0.0f };
   float     yaw;
   int       mode = 0;

public:
   CLASS_PROTOTYPE(Turn);

   void                 SetDirection(float yaw);
   void                 SetTarget(Entity *ent);
   virtual void         ShowInfo(Actor &self)    override;
   virtual void         Begin(Actor &self)       override;
   virtual qboolean     Evaluate(Actor &self)    override;
   virtual void         End(Actor &self)         override;
   virtual void         Archive(Archiver &arc)   override;
   virtual void         Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void Turn::Archive(Archiver &arc)
{
   Steering::Archive(arc);

   arc.WriteObject(&seek);
   arc.WriteSafePointer(ent);
   arc.WriteVector(dir);
   arc.WriteFloat(yaw);
   arc.WriteInteger(mode);
}

inline EXPORT_FROM_DLL void Turn::Unarchive(Archiver &arc)
{
   Steering::Unarchive(arc);

   arc.ReadObject(&seek);
   arc.ReadSafePointer(&ent);
   arc.ReadVector(&dir);
   arc.ReadFloat(&yaw);
   arc.ReadInteger(&mode);
}

class EXPORT_FROM_DLL Chase : public Steering
{
private:
   Seek                 seek;
   FollowPath           follow;
   float                nextpathtime;
   PathPtr              path;
   Vector               goal;
   EntityPtr            goalent      = nullptr;
   PathNodePtr          goalnode     = nullptr;
   ObstacleAvoidance    avoid;
   float                avoidtime;
   qboolean             usegoal      = false;
   float                newpathrate  = 2.0f;

   Vector               wanderstart;
   int                  wander;
   float                wandertime;
   Turn                 turnto;
   str                  anim;
   int                  stuck;
   Vector               avoidvec;

public:
   CLASS_PROTOTYPE(Chase);

   void                 SetPath(Path *newpath);
   void                 SetGoalPos(Vector pos);
   void                 SetGoal(PathNode *node);
   void                 SetTarget(Entity *ent);
   void                 SetPathRate(float rate);
   virtual void         ShowInfo(Actor &self)    override;
   Vector               ChooseRandomDirection(Actor &self);
   virtual void         Begin(Actor &self)       override;
   virtual qboolean     Evaluate(Actor &self)    override;
   virtual void         End(Actor &self)         override;
   virtual void         Archive(Archiver &arc)   override;
   virtual void         Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void Chase::Archive(Archiver &arc)
{
   Steering::Archive(arc);

   arc.WriteObject(&seek);
   arc.WriteObject(&follow);
   arc.WriteFloat(nextpathtime);
   arc.WriteSafePointer(path);
   arc.WriteVector(goal);
   arc.WriteSafePointer(goalent);
   arc.WriteSafePointer(goalnode);
   arc.WriteObject(&avoid);
   arc.WriteFloat(avoidtime);
   arc.WriteBoolean(usegoal);
   arc.WriteFloat(newpathrate);
   arc.WriteVector(wanderstart);
   arc.WriteInteger(wander);
   arc.WriteFloat(wandertime);
   arc.WriteObject(&turnto);
   arc.WriteString(anim);
   arc.WriteInteger(stuck);
   arc.WriteVector(avoidvec);
}

inline EXPORT_FROM_DLL void Chase::Unarchive(Archiver &arc)
{
   Steering::Unarchive(arc);

   arc.ReadObject(&seek);
   arc.ReadObject(&follow);
   arc.ReadFloat(&nextpathtime);
   arc.ReadSafePointer(&path);
   arc.ReadVector(&goal);
   arc.ReadSafePointer(&goalent);
   arc.ReadSafePointer(&goalnode);
   arc.ReadObject(&avoid);
   arc.ReadFloat(&avoidtime);
   arc.ReadBoolean(&usegoal);
   arc.ReadFloat(&newpathrate);
   arc.ReadVector(&wanderstart);
   arc.ReadInteger(&wander);
   arc.ReadFloat(&wandertime);
   arc.ReadObject(&turnto);
   arc.ReadString(&anim);
   arc.ReadInteger(&stuck);
   arc.ReadVector(&avoidvec);
}

#endif /* steering.h */

// EOF

