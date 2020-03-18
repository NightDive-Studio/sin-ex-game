//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/behavior.h                       $
// $Revision:: 55                                                             $
//   $Author:: Jimdose                                                        $
//     $Date:: 10/27/98 8:02p                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Standard class for creating AI behaviors
//

#ifndef __BEHAVIOR_H__
#define __BEHAVIOR_H__

#include "g_local.h"
#include "entity.h"
#include "path.h"
#include "steering.h"

extern Event EV_Behavior_Args;
extern Event EV_Behavior_AnimDone;

class Actor;

class EXPORT_FROM_DLL Behavior : public Listener
{
protected:
   PathNodePtr          movegoal;

public:
   CLASS_PROTOTYPE(Behavior);

   virtual void         ShowInfo(Actor &self);
   virtual void         Begin(Actor &self);
   virtual qboolean     Evaluate(Actor &self);
   virtual void         End(Actor &self);
   virtual void         Archive(Archiver &arc)   override;
   virtual void         Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void Behavior::Archive(Archiver &arc)
{
   Listener::Archive(arc);

   arc.WriteSafePointer(movegoal);
}

inline EXPORT_FROM_DLL void Behavior::Unarchive(Archiver &arc)
{
   Listener::Unarchive(arc);

   arc.ReadSafePointer(&movegoal);
}

#ifdef EXPORT_TEMPLATE
template class EXPORT_FROM_DLL SafePtr<Behavior>;
#endif
typedef SafePtr<Behavior> BehaviorPtr;

class EXPORT_FROM_DLL Idle : public Behavior
{
private:
   float						nexttwitch;
   str						anim;

public:
   CLASS_PROTOTYPE(Idle);

   void             SetArgs(Event *ev);
   virtual void     ShowInfo(Actor &self)    override;
   virtual void     Begin(Actor &self)       override;
   virtual qboolean Evaluate(Actor &self)    override;
   virtual void     End(Actor &self)         override;
   virtual void     Archive(Archiver &arc)   override;
   virtual void     Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void Idle::Archive(Archiver &arc)
{
   Behavior::Archive(arc);

   arc.WriteFloat(nexttwitch);
   arc.WriteString(anim);
}

inline EXPORT_FROM_DLL void Idle::Unarchive(Archiver &arc)
{
   Behavior::Unarchive(arc);

   arc.ReadFloat(&nexttwitch);
   arc.ReadString(&anim);
}

class EXPORT_FROM_DLL Aim : public Behavior
{
private:
   Seek                 seek;
   EntityPtr            target;

public:
   CLASS_PROTOTYPE(Aim);

   void             SetTarget(Entity *ent);
   virtual void     ShowInfo(Actor &self)    override;
   virtual void     Begin(Actor &self)       override;
   virtual qboolean Evaluate(Actor &self)    override;
   virtual void     End(Actor &self)         override;
   virtual void     Archive(Archiver &arc)   override;
   virtual void     Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void Aim::Archive(Archiver &arc)
{
   Behavior::Archive(arc);

   arc.WriteObject(&seek);
   arc.WriteSafePointer(target);
}

inline EXPORT_FROM_DLL void Aim::Unarchive(Archiver &arc)
{
   Behavior::Unarchive(arc);

   arc.ReadObject(&seek);
   arc.ReadSafePointer(&target);
}

class EXPORT_FROM_DLL FireOnSight : public Behavior
{
private:
   Chase                chase;
   Aim                  aim;
   int                  mode;
   str                  anim;

public:
   CLASS_PROTOTYPE(FireOnSight);

   void                 SetArgs(Event *ev);
   virtual void         ShowInfo(Actor &self)    override;
   virtual void         Begin(Actor &self)       override;
   virtual qboolean     Evaluate(Actor &self)    override;
   virtual void         End(Actor &self)         override;
   virtual void         Archive(Archiver &arc)   override;
   virtual void         Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void FireOnSight::Archive(Archiver &arc)
{
   Behavior::Archive(arc);

   arc.WriteObject(&chase);
   arc.WriteObject(&aim);
   arc.WriteInteger(mode);
   arc.WriteString(anim);
}

inline EXPORT_FROM_DLL void FireOnSight::Unarchive(Archiver &arc)
{
   Behavior::Unarchive(arc);

   arc.ReadObject(&chase);
   arc.ReadObject(&aim);
   arc.ReadInteger(&mode);
   arc.ReadString(&anim);
}

class EXPORT_FROM_DLL TurnTo : public Behavior
{
private:
   Seek                 seek;
   EntityPtr            ent;
   Vector               dir;
   float                yaw;
   int                  mode;

public:
   CLASS_PROTOTYPE(TurnTo);

   TurnTo();
   void                 SetDirection(float yaw);
   void                 SetTarget(Entity *ent);
   virtual void         ShowInfo(Actor &self)    override;
   virtual void         Begin(Actor &self)       override;
   virtual qboolean     Evaluate(Actor &self)    override;
   virtual void         End(Actor &self)         override;
   virtual void         Archive(Archiver &arc)   override;
   virtual void         Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void TurnTo::Archive(Archiver &arc)
{
   Behavior::Archive(arc);

   arc.WriteObject(&seek);
   arc.WriteSafePointer(ent);
   arc.WriteVector(dir);
   arc.WriteFloat(yaw);
   arc.WriteInteger(mode);
}

inline EXPORT_FROM_DLL void TurnTo::Unarchive(Archiver &arc)
{
   Behavior::Unarchive(arc);

   arc.ReadObject(&seek);
   arc.ReadSafePointer(&ent);
   arc.ReadVector(&dir);
   arc.ReadFloat(&yaw);
   arc.ReadInteger(&mode);
}

class EXPORT_FROM_DLL Jump : public Behavior
{
private:
   float                endtime;
   float                speed;
   str                  anim;
   int                  state;
   qboolean             animdone;
   Vector               goal;

public:
   CLASS_PROTOTYPE(Jump);

   Jump();
   void                 SetArgs(Event *ev);
   virtual void         ShowInfo(Actor &self)    override;
   virtual void         Begin(Actor &self)       override;
   virtual qboolean     Evaluate(Actor &self)    override;
   virtual void         End(Actor &self)         override;
   void                 AnimDone(Event *ev);
   virtual void         Archive(Archiver &arc)   override;
   virtual void         Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void Jump::Archive(Archiver &arc)
{
   Behavior::Archive(arc);

   arc.WriteFloat(endtime);
   arc.WriteFloat(speed);
   arc.WriteString(anim);
   arc.WriteInteger(state);
   arc.WriteBoolean(animdone);
   arc.WriteVector(goal);
}

inline EXPORT_FROM_DLL void Jump::Unarchive(Archiver &arc)
{
   Behavior::Unarchive(arc);

   arc.ReadFloat(&endtime);
   arc.ReadFloat(&speed);
   arc.ReadString(&anim);
   arc.ReadInteger(&state);
   arc.ReadBoolean(&animdone);
   arc.ReadVector(&goal);
}


class EXPORT_FROM_DLL GotoPathNode : public Behavior
{
private:
   TurnTo               turnto;
   Chase                chase;
   int                  state  = 0;
   qboolean             usevec = false;
   float                time   = 0;
   str                  anim;
   EntityPtr            goalent;
   Vector               goal;

public:
   CLASS_PROTOTYPE(GotoPathNode);

   void                 SetArgs(Event *ev);
   void                 SetGoal(PathNode *node);
   virtual void         ShowInfo(Actor &self)    override;
   virtual void         Begin(Actor &self)       override;
   virtual qboolean     Evaluate(Actor &self)    override;
   virtual void         End(Actor &self)         override;
   virtual void         Archive(Archiver &arc)   override;
   virtual void         Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void GotoPathNode::Archive(Archiver &arc)
{
   Behavior::Archive(arc);

   arc.WriteObject(&turnto);
   arc.WriteObject(&chase);
   arc.WriteInteger(state);
   arc.WriteBoolean(usevec);
   arc.WriteFloat(time);
   arc.WriteString(anim);
   arc.WriteSafePointer(goalent);
   arc.WriteVector(goal);
}

inline EXPORT_FROM_DLL void GotoPathNode::Unarchive(Archiver &arc)
{
   Behavior::Unarchive(arc);

   arc.ReadObject(&turnto);
   arc.ReadObject(&chase);
   arc.ReadInteger(&state);
   arc.ReadBoolean(&usevec);
   arc.ReadFloat(&time);
   arc.ReadString(&anim);
   arc.ReadSafePointer(&goalent);
   arc.ReadVector(&goal);
}

class EXPORT_FROM_DLL Investigate : public Behavior
{
private:
   Chase                chase;
   str                  anim;
   Vector               goal;
   float                curioustime;

   qboolean             Done(Actor &self);

public:
   CLASS_PROTOTYPE(Investigate);

   void                 SetArgs(Event *ev);
   virtual void         ShowInfo(Actor &self)    override;
   virtual void         Begin(Actor &self)       override;
   virtual qboolean     Evaluate(Actor &self)    override;
   virtual void         End(Actor &self)         override;
   virtual void         Archive(Archiver &arc)   override;
   virtual void         Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void Investigate::Archive(Archiver &arc)
{
   Behavior::Archive(arc);

   arc.WriteObject(&chase);
   arc.WriteString(anim);
   arc.WriteVector(goal);
   arc.WriteFloat(curioustime);
}

inline EXPORT_FROM_DLL void Investigate::Unarchive(Archiver &arc)
{
   Behavior::Unarchive(arc);

   arc.ReadObject(&chase);
   arc.ReadString(&anim);
   arc.ReadVector(&goal);
   arc.ReadFloat(&curioustime);
}

class EXPORT_FROM_DLL Flee : public Behavior
{
private:
   FollowPath           follow;
   PathPtr              path;
   ObstacleAvoidance    avoid;
   float                avoidtime;
   str                  anim;

public:
   CLASS_PROTOTYPE(Flee);

   void                 SetArgs(Event *ev);
   virtual void         ShowInfo(Actor &self)    override;
   virtual void         Begin(Actor &self)       override;
   virtual qboolean     Evaluate(Actor &self)    override;
   virtual void         End(Actor &self)         override;
   virtual void         Archive(Archiver &arc)   override;
   virtual void         Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void Flee::Archive(Archiver &arc)
{
   Behavior::Archive(arc);

   arc.WriteObject(&follow);
   arc.WriteSafePointer(path);
   arc.WriteObject(&avoid);
   arc.WriteFloat(avoidtime);
   arc.WriteString(anim);
}

inline EXPORT_FROM_DLL void Flee::Unarchive
(
   Archiver &arc
)
{
   Behavior::Unarchive(arc);

   arc.ReadObject(&follow);
   arc.ReadSafePointer(&path);
   arc.ReadObject(&avoid);
   arc.ReadFloat(&avoidtime);
   arc.ReadString(&anim);
}

class EXPORT_FROM_DLL OpenDoor : public Behavior
{
private:
   float                time;
   float                endtime;
   qboolean             usedir = false;
   Vector               dir;

public:
   CLASS_PROTOTYPE(OpenDoor);

   void                 SetArgs(Event *ev);
   virtual void         ShowInfo(Actor &self)    override;
   virtual void         Begin(Actor &self)       override;
   virtual qboolean     Evaluate(Actor &self)    override;
   virtual void         End(Actor &self)         override;
   virtual void         Archive(Archiver &arc)   override;
   virtual void         Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void OpenDoor::Archive(Archiver &arc)
{
   Behavior::Archive(arc);

   arc.WriteFloat(time);
   arc.WriteFloat(endtime);
   arc.WriteBoolean(usedir);
   arc.WriteVector(dir);
}

inline EXPORT_FROM_DLL void OpenDoor::Unarchive(Archiver &arc)
{
   Behavior::Unarchive(arc);

   arc.ReadFloat(&time);
   arc.ReadFloat(&endtime);
   arc.ReadBoolean(&usedir);
   arc.ReadVector(&dir);
}

class EXPORT_FROM_DLL PlayAnim : public Behavior
{
private:
   str                  anim;

public:
   CLASS_PROTOTYPE(PlayAnim);

   void                 SetArgs(Event *ev);
   virtual void         ShowInfo(Actor &self)    override;
   virtual void         Begin(Actor &self)       override;
   virtual qboolean     Evaluate(Actor &self)    override;
   virtual void         End(Actor &self)         override;
   virtual void         Archive(Archiver &arc)   override;
   virtual void         Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void PlayAnim::Archive(Archiver &arc)
{
   Behavior::Archive(arc);

   arc.WriteString(anim);
}

inline EXPORT_FROM_DLL void PlayAnim::Unarchive(Archiver &arc)
{
   Behavior::Unarchive(arc);

   arc.ReadString(&anim);
}

class EXPORT_FROM_DLL FindCover : public Behavior
{
private:
   str	 anim;
   Chase chase;
   int	 state;
   float nextsearch;

public:
   CLASS_PROTOTYPE(FindCover);

   void              SetArgs(Event *ev);
   PathNode         *FindCoverNode(Actor &self);
   virtual void      ShowInfo(Actor &self)    override;
   virtual void      Begin(Actor &self)       override;
   virtual qboolean  Evaluate(Actor &self)    override;
   virtual void      End(Actor &self)         override;
   virtual void      Archive(Archiver &arc)   override;
   virtual void      Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void FindCover::Archive(Archiver &arc)
{
   Behavior::Archive(arc);

   arc.WriteString(anim);
   arc.WriteObject(&chase);
   arc.WriteInteger(state);
   arc.WriteFloat(nextsearch);
}

inline EXPORT_FROM_DLL void FindCover::Unarchive(Archiver &arc)
{
   Behavior::Unarchive(arc);

   arc.ReadString(&anim);
   arc.ReadObject(&chase);
   arc.ReadInteger(&state);
   arc.ReadFloat(&nextsearch);
}

class EXPORT_FROM_DLL FindFlee : public Behavior
{
private:
   str						anim;
   Chase						chase;
   int						state;
   float						nextsearch;

public:
   CLASS_PROTOTYPE(FindFlee);

   void              SetArgs(Event *ev);
   PathNode         *FindFleeNode(Actor &self);
   virtual void      ShowInfo(Actor &self)    override;
   virtual void      Begin(Actor &self)       override;
   virtual qboolean  Evaluate(Actor &self)    override;
   virtual void      End(Actor &self)         override;
   virtual void      Archive(Archiver &arc)   override;
   virtual void      Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void FindFlee::Archive(Archiver &arc)
{
   Behavior::Archive(arc);

   arc.WriteString(anim);
   arc.WriteObject(&chase);
   arc.WriteInteger(state);
   arc.WriteFloat(nextsearch);
}

inline EXPORT_FROM_DLL void FindFlee::Unarchive(Archiver &arc)
{
   Behavior::Unarchive(arc);

   arc.ReadString(&anim);
   arc.ReadObject(&chase);
   arc.ReadInteger(&state);
   arc.ReadFloat(&nextsearch);
}

class EXPORT_FROM_DLL FindEnemy : public Behavior
{
private:
   str						anim;
   Chase						chase;
   int						state;
   float						nextsearch;
   PathNodePtr          lastSearchNode;
   Vector               lastSearchPos;

public:
   CLASS_PROTOTYPE(FindEnemy);

   PathNode         *FindClosestSightNode(Actor &self);
   void              SetArgs(Event *ev);
   virtual void      ShowInfo(Actor &self)    override;
   virtual void      Begin(Actor &self)       override;
   virtual qboolean  Evaluate(Actor &self)    override;
   virtual void      End(Actor &self)         override;
   virtual void      Archive(Archiver &arc)   override;
   virtual void      Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void FindEnemy::Archive(Archiver &arc)
{
   Behavior::Archive(arc);

   arc.WriteString(anim);
   arc.WriteObject(&chase);
   arc.WriteInteger(state);
   arc.WriteFloat(nextsearch);
   arc.WriteSafePointer(lastSearchNode);
   arc.WriteVector(lastSearchPos);
}

inline EXPORT_FROM_DLL void FindEnemy::Unarchive(Archiver &arc)
{
   Behavior::Unarchive(arc);

   arc.ReadString(&anim);
   arc.ReadObject(&chase);
   arc.ReadInteger(&state);
   arc.ReadFloat(&nextsearch);
   arc.ReadSafePointer(&lastSearchNode);
   arc.ReadVector(&lastSearchPos);
}

class EXPORT_FROM_DLL Hide : public Behavior
{
private:
   FindCover				hide;
   str						anim;
   int						state;
   float						checktime;

public:
   CLASS_PROTOTYPE(Hide);

   void             SetArgs(Event *ev);
   virtual void     ShowInfo(Actor &self)    override;
   virtual void     Begin(Actor &self)       override;
   virtual qboolean Evaluate(Actor &self)    override;
   virtual void     End(Actor &self)         override;
   virtual void     Archive(Archiver &arc)   override;
   virtual void     Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void Hide::Archive(Archiver &arc)
{
   Behavior::Archive(arc);

   arc.WriteObject(&hide);
   arc.WriteString(anim);
   arc.WriteInteger(state);
   arc.WriteFloat(checktime);
}

inline EXPORT_FROM_DLL void Hide::Unarchive(Archiver &arc)
{
   Behavior::Unarchive(arc);

   arc.ReadObject(&hide);
   arc.ReadString(&anim);
   arc.ReadInteger(&state);
   arc.ReadFloat(&checktime);
}


class EXPORT_FROM_DLL FleeAndRemove : public Behavior
{
private:
   FindFlee				   flee;
   str						anim;
   int						state;
   float						checktime;

public:
   CLASS_PROTOTYPE(FleeAndRemove);

   void						SetArgs(Event *ev);
   void                 ShowInfo(Actor &self);
   void						Begin(Actor &self);
   qboolean					Evaluate(Actor &self);
   void						End(Actor &self);
   virtual void         Archive(Archiver &arc);
   virtual void         Unarchive(Archiver &arc);
};

inline EXPORT_FROM_DLL void FleeAndRemove::Archive(Archiver &arc)
{
   Behavior::Archive(arc);

   arc.WriteObject(&flee);
   arc.WriteString(anim);
   arc.WriteInteger(state);
   arc.WriteFloat(checktime);
}

inline EXPORT_FROM_DLL void FleeAndRemove::Unarchive(Archiver &arc)
{
   Behavior::Unarchive(arc);

   arc.ReadObject(&flee);
   arc.ReadString(&anim);
   arc.ReadInteger(&state);
   arc.ReadFloat(&checktime);
}


class EXPORT_FROM_DLL AimAndShoot : public Behavior
{
private:
   Aim                  aim;
   int                  mode;
   int                  maxshots = 1;
   int                  numshots = 0;
   qboolean             animdone;
   float                enemy_health;
   float                aim_time;
   str                  animprefix;
   str                  readyfireanim;
   str                  aimanim;
   str                  fireanim;

public:
   CLASS_PROTOTYPE(AimAndShoot);

   void             SetMaxShots(int num);
   void             SetArgs(Event *ev);
   void             AnimDone(Event *ev);
   virtual void     ShowInfo(Actor &self)    override;
   virtual void     Begin(Actor &self)       override;
   virtual qboolean Evaluate(Actor &self)    override;
   virtual void     End(Actor &self)         override;
   virtual void     Archive(Archiver &arc)   override;
   virtual void     Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void AimAndShoot::Archive(Archiver &arc)
{
   Behavior::Archive(arc);

   arc.WriteObject(&aim);
   arc.WriteInteger(mode);
   arc.WriteInteger(maxshots);
   arc.WriteInteger(numshots);
   arc.WriteBoolean(animdone);
   arc.WriteFloat(enemy_health);
   arc.WriteFloat(aim_time);
   arc.WriteString(animprefix);
   arc.WriteString(readyfireanim);
   arc.WriteString(aimanim);
   arc.WriteString(fireanim);
}

inline EXPORT_FROM_DLL void AimAndShoot::Unarchive(Archiver &arc)
{
   Behavior::Unarchive(arc);

   arc.ReadObject(&aim);
   arc.ReadInteger(&mode);
   arc.ReadInteger(&maxshots);
   arc.ReadInteger(&numshots);
   arc.ReadBoolean(&animdone);
   arc.ReadFloat(&enemy_health);
   arc.ReadFloat(&aim_time);
   arc.ReadString(&animprefix);
   arc.ReadString(&readyfireanim);
   arc.ReadString(&aimanim);
   arc.ReadString(&fireanim);
}

class EXPORT_FROM_DLL AimAndMelee : public Behavior
{
private:
   Aim						aim;
   int						mode;
   int						maxshots;
   int						numshots;
   qboolean					animdone;

public:
   CLASS_PROTOTYPE(AimAndMelee);

   void             SetArgs(Event *ev);
   void             AnimDone(Event *ev);
   virtual void     ShowInfo(Actor &self)    override;
   virtual void     Begin(Actor &self)       override;
   virtual qboolean Evaluate(Actor &self)    override;
   virtual void     End(Actor &self)         override;
   virtual void     Archive(Archiver &arc)   override;
   virtual void     Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void AimAndMelee::Archive(Archiver &arc)
{
   Behavior::Archive(arc);

   arc.WriteObject(&aim);
   arc.WriteInteger(mode);
   arc.WriteInteger(maxshots);
   arc.WriteInteger(numshots);
   arc.WriteBoolean(animdone);
}

inline EXPORT_FROM_DLL void AimAndMelee::Unarchive(Archiver &arc)
{
   Behavior::Unarchive(arc);

   arc.ReadObject(&aim);
   arc.ReadInteger(&mode);
   arc.ReadInteger(&maxshots);
   arc.ReadInteger(&numshots);
   arc.ReadBoolean(&animdone);
}


class EXPORT_FROM_DLL Melee : public Behavior
{
private:
   int						mode;
   qboolean					animdone;

public:
   CLASS_PROTOTYPE(Melee);

   void             SetArgs(Event *ev);
   void             AnimDone(Event *ev);
   virtual void     ShowInfo(Actor &self)    override;
   virtual void     Begin(Actor &self)       override;
   virtual qboolean Evaluate(Actor &self)    override;
   virtual void     End(Actor &self)         override;
   virtual void     Archive(Archiver &arc)   override;
   virtual void     Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void Melee::Archive(Archiver &arc)
{
   Behavior::Archive(arc);

   arc.WriteInteger(mode);
   arc.WriteBoolean(animdone);
}

inline EXPORT_FROM_DLL void Melee::Unarchive(Archiver &arc)
{
   Behavior::Unarchive(arc);

   arc.ReadInteger(&mode);
   arc.ReadBoolean(&animdone);
}

class EXPORT_FROM_DLL Repel : public Behavior
{
private:
   str                  anim;
   float                dist;
   float                len;
   float                speed;
   Vector               goal;
   Vector               start;
   Vector               dir;

public:
   CLASS_PROTOTYPE(Repel);

   void             SetArgs(Event *ev);
   virtual void     ShowInfo(Actor &self)    override;
   virtual void     Begin(Actor &self)       override;
   virtual qboolean Evaluate(Actor &self)    override;
   virtual void     End(Actor &self)         override;
   virtual void     Archive(Archiver &arc)   override;
   virtual void     Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void Repel::Archive(Archiver &arc)
{
   Behavior::Archive(arc);

   arc.WriteString(anim);
   arc.WriteFloat(dist);
   arc.WriteFloat(len);
   arc.WriteFloat(speed);
   arc.WriteVector(goal);
   arc.WriteVector(start);
   arc.WriteVector(dir);
}

inline EXPORT_FROM_DLL void Repel::Unarchive(Archiver &arc)
{
   Behavior::Unarchive(arc);

   arc.ReadString(&anim);
   arc.ReadFloat(&dist);
   arc.ReadFloat(&len);
   arc.ReadFloat(&speed);
   arc.ReadVector(&goal);
   arc.ReadVector(&start);
   arc.ReadVector(&dir);
}


class EXPORT_FROM_DLL PickupAndThrow : public Behavior
{
private:
   Aim						aim;
   int						mode;
   qboolean					animdone;
   EntityPtr            pickup_target;


public:
   CLASS_PROTOTYPE(PickupAndThrow);

   void             SetArgs(Event *ev);
   void             AnimDone(Event *ev);
   void             Pickup(Event *ev);
   void             Throw(Event *ev);
   virtual void     ShowInfo(Actor &self)    override;
   virtual void     Begin(Actor &self)       override;
   virtual qboolean Evaluate(Actor &self)    override;
   virtual void     End(Actor &self)         override;
   virtual void     Archive(Archiver &arc)   override;
   virtual void     Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void PickupAndThrow::Archive(Archiver &arc)
{
   Behavior::Archive(arc);

   arc.WriteObject(&aim);
   arc.WriteInteger(mode);
   arc.WriteBoolean(animdone);
   arc.WriteSafePointer(pickup_target);
}

inline EXPORT_FROM_DLL void PickupAndThrow::Unarchive(Archiver &arc)
{
   Behavior::Unarchive(arc);

   arc.ReadObject(&aim);
   arc.ReadInteger(&mode);
   arc.ReadBoolean(&animdone);
   arc.ReadSafePointer(&pickup_target);
}

class EXPORT_FROM_DLL StrafeAttack : public Behavior
{
private:
   int                  state;
   TurnTo               turn;

public:
   CLASS_PROTOTYPE(StrafeAttack);

   void         ShowInfo(Actor &self)    override;
   void         Begin(Actor &self)       override;
   qboolean     Evaluate(Actor &self)    override;
   void         End(Actor &self)         override;
   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void StrafeAttack::Archive(Archiver &arc)
{
   Behavior::Archive(arc);

   arc.WriteInteger(state);
   arc.WriteObject(&turn);
}

inline EXPORT_FROM_DLL void StrafeAttack::Unarchive(Archiver &arc)
{
   Behavior::Unarchive(arc);

   arc.ReadInteger(&state);
   arc.ReadObject(&turn);
}

class EXPORT_FROM_DLL StrafeTo : public Behavior
{
private:
   Vector               goal;
   Seek                 seek;
   int                  fail;

public:
   CLASS_PROTOTYPE(StrafeTo);

   virtual void      ShowInfo(Actor &self)    override;
   void              SetArgs(Event *ev);
   virtual void      Begin(Actor &self)       override;
   virtual qboolean  Evaluate(Actor &self)    override;
   virtual void      End(Actor &self)         override;
   virtual void      Archive(Archiver &arc)   override;
   virtual void      Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void StrafeTo::Archive(Archiver &arc)
{
   Behavior::Archive(arc);

   arc.WriteVector(goal);
   arc.WriteObject(&seek);
   arc.WriteInteger(fail);
}

inline EXPORT_FROM_DLL void StrafeTo::Unarchive(Archiver &arc)
{
   Behavior::Unarchive(arc);

   arc.ReadVector(&goal);
   arc.ReadObject(&seek);
   arc.ReadInteger(&fail);
}

class EXPORT_FROM_DLL Swim : public Behavior
{
private:
   Seek                 seek;
   ObstacleAvoidance2   avoid;
   str                  anim;
   float                avoidtime;
   Vector               avoidvec;

public:
   CLASS_PROTOTYPE(Swim);

   void             SetArgs(Event *ev);
   virtual void     ShowInfo(Actor &self)    override;
   virtual void     Begin(Actor &self)       override;
   virtual qboolean Evaluate(Actor &self)    override;
   virtual void     End(Actor &self)         override;
   virtual void     Archive(Archiver &arc)   override;
   virtual void     Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void Swim::Archive(Archiver &arc)
{
   Behavior::Archive(arc);

   arc.WriteObject(&seek);
   arc.WriteObject(&avoid);
   arc.WriteString(anim);
   arc.WriteFloat(avoidtime);
   arc.WriteVector(avoidvec);
}

inline EXPORT_FROM_DLL void Swim::Unarchive(Archiver &arc)
{
   Behavior::Unarchive(arc);

   arc.ReadObject(&seek);
   arc.ReadObject(&avoid);
   arc.ReadString(&anim);
   arc.ReadFloat(&avoidtime);
   arc.ReadVector(&avoidvec);
}

class EXPORT_FROM_DLL SwimCloseAttack : public Behavior
{
private:
   Seek                 seek;
   ObstacleAvoidance2   avoid;
   qboolean             avoiding;
   str                  anim;
   float                avoidtime;
   Vector               avoidvec;

public:
   CLASS_PROTOTYPE(SwimCloseAttack);

   void             SetArgs(Event *ev);
   virtual void     ShowInfo(Actor &self)    override;
   virtual void     Begin(Actor &self)       override;
   virtual qboolean Evaluate(Actor &self)    override;
   virtual void     End(Actor &self)         override;
   virtual void     Archive(Archiver &arc)   override;
   virtual void     Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void SwimCloseAttack::Archive(Archiver &arc)
{
   Behavior::Archive(arc);

   arc.WriteObject(&seek);
   arc.WriteObject(&avoid);
   arc.WriteBoolean(avoiding);
   arc.WriteString(anim);
   arc.WriteFloat(avoidtime);
   arc.WriteVector(avoidvec);
}

inline EXPORT_FROM_DLL void SwimCloseAttack::Unarchive(Archiver &arc)
{
   Behavior::Unarchive(arc);

   arc.ReadObject(&seek);
   arc.ReadObject(&avoid);
   arc.ReadBoolean(&avoiding);
   arc.ReadString(&anim);
   arc.ReadFloat(&avoidtime);
   arc.ReadVector(&avoidvec);
}

class EXPORT_FROM_DLL Fly : public Behavior
{
private:
   Seek                 seek;
   ObstacleAvoidance2   avoid;
   str                  anim;
   float                avoidtime;
   Vector               avoidvec;

public:
   CLASS_PROTOTYPE(Fly);

   void             SetArgs(Event *ev);
   virtual void     ShowInfo(Actor &self)    override;
   virtual void     Begin(Actor &self)       override;
   virtual qboolean Evaluate(Actor &self)    override;
   virtual void     End(Actor &self)         override;
   virtual void     Archive(Archiver &arc)   override;
   virtual void     Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void Fly::Archive(Archiver &arc)
{
   Behavior::Archive(arc);

   arc.WriteObject(&seek);
   arc.WriteObject(&avoid);
   arc.WriteString(anim);
   arc.WriteFloat(avoidtime);
   arc.WriteVector(avoidvec);
}

inline EXPORT_FROM_DLL void Fly::Unarchive(Archiver &arc)
{
   Behavior::Unarchive(arc);

   arc.ReadObject(&seek);
   arc.ReadObject(&avoid);
   arc.ReadString(&anim);
   arc.ReadFloat(&avoidtime);
   arc.ReadVector(&avoidvec);
}

class EXPORT_FROM_DLL FlyCloseAttack : public Behavior
{
private:
   Seek                 seek;
   ObstacleAvoidance2   avoid;
   qboolean             avoiding;
   str                  anim;
   float                avoidtime;
   Vector               avoidvec;

public:
   CLASS_PROTOTYPE(FlyCloseAttack);

   void             SetArgs(Event *ev);
   virtual void     ShowInfo(Actor &self)    override;
   virtual void     Begin(Actor &self)       override;
   virtual qboolean Evaluate(Actor &self)    override;
   virtual void     End(Actor &self)         override;
   virtual void     Archive(Archiver &arc)   override;
   virtual void     Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void FlyCloseAttack::Archive(Archiver &arc)
{
   Behavior::Archive(arc);

   arc.WriteObject(&seek);
   arc.WriteObject(&avoid);
   arc.WriteBoolean(avoiding);
   arc.WriteString(anim);
   arc.WriteFloat(avoidtime);
   arc.WriteVector(avoidvec);
}

inline EXPORT_FROM_DLL void FlyCloseAttack::Unarchive(Archiver &arc)
{
   Behavior::Unarchive(arc);

   arc.ReadObject(&seek);
   arc.ReadObject(&avoid);
   arc.ReadBoolean(&avoiding);
   arc.ReadString(&anim);
   arc.ReadFloat(&avoidtime);
   arc.ReadVector(&avoidvec);
}

class EXPORT_FROM_DLL Wander : public Behavior
{
private:
   Seek                 seek;
   ObstacleAvoidance2   avoid;
   str                  anim;
   float                avoidtime;
   Vector               avoidvec;

public:
   CLASS_PROTOTYPE(Wander);

   void             SetArgs(Event *ev);
   virtual void     ShowInfo(Actor &self)    override;
   virtual void     Begin(Actor &self)       override;
   virtual qboolean Evaluate(Actor &self)    override;
   virtual void     End(Actor &self)         override;
   virtual void     Archive(Archiver &arc)   override;
   virtual void     Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void Wander::Archive(Archiver &arc)
{
   Behavior::Archive(arc);

   arc.WriteObject(&seek);
   arc.WriteObject(&avoid);
   arc.WriteString(anim);
   arc.WriteFloat(avoidtime);
   arc.WriteVector(avoidvec);
}

inline EXPORT_FROM_DLL void Wander::Unarchive(Archiver &arc)
{
   Behavior::Unarchive(arc);

   arc.ReadObject(&seek);
   arc.ReadObject(&avoid);
   arc.ReadString(&anim);
   arc.ReadFloat(&avoidtime);
   arc.ReadVector(&avoidvec);
}


class EXPORT_FROM_DLL WanderCloseAttack : public Behavior
{
private:
   Seek                 seek;
   ObstacleAvoidance2   avoid;
   qboolean             avoiding;
   str                  anim;
   float                avoidtime;
   Vector               avoidvec;

public:
   CLASS_PROTOTYPE(WanderCloseAttack);

   void             SetArgs(Event *ev);
   virtual void     ShowInfo(Actor &self)    override;
   virtual void     Begin(Actor &self)       override;
   virtual qboolean Evaluate(Actor &self)    override;
   virtual void     End(Actor &self)         override;
   virtual void     Archive(Archiver &arc)   override;
   virtual void     Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void WanderCloseAttack::Archive
(
   Archiver &arc
)
{
   Behavior::Archive(arc);

   arc.WriteObject(&seek);
   arc.WriteObject(&avoid);
   arc.WriteBoolean(avoiding);
   arc.WriteString(anim);
   arc.WriteFloat(avoidtime);
   arc.WriteVector(avoidvec);
}

inline EXPORT_FROM_DLL void WanderCloseAttack::Unarchive(Archiver &arc)
{
   Behavior::Unarchive(arc);

   arc.ReadObject(&seek);
   arc.ReadObject(&avoid);
   arc.ReadBoolean(&avoiding);
   arc.ReadString(&anim);
   arc.ReadFloat(&avoidtime);
   arc.ReadVector(&avoidvec);
}

class EXPORT_FROM_DLL GetCloseToEnemy : public Behavior
{
private:
   float                howclose = 32.0f;
   str                  anim;
   Chase                chase;
   int                  state;
   float                nextsearch;

public:
   CLASS_PROTOTYPE(GetCloseToEnemy);

   void             SetArgs(Event *ev);
   virtual void     ShowInfo(Actor &self)    override;
   virtual void     Begin(Actor &self)       override;
   virtual qboolean Evaluate(Actor &self)    override;
   virtual void     End(Actor &self)         override;
   virtual void     Archive(Archiver &arc)   override;
   virtual void     Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void GetCloseToEnemy::Archive(Archiver &arc)
{
   Behavior::Archive(arc);

   arc.WriteFloat(howclose);
   arc.WriteString(anim);
   arc.WriteObject(&chase);
   arc.WriteInteger(state);
   arc.WriteFloat(nextsearch);
}

inline EXPORT_FROM_DLL void GetCloseToEnemy::Unarchive(Archiver &arc)
{
   Behavior::Unarchive(arc);

   arc.ReadFloat(&howclose);
   arc.ReadString(&anim);
   arc.ReadObject(&chase);
   arc.ReadInteger(&state);
   arc.ReadFloat(&nextsearch);
}

class EXPORT_FROM_DLL PlayAnimSeekEnemy : public Behavior
{
private:
   Aim      aim;
   int      mode;
   qboolean animdone;
   str      anim;
   str      oldanim;

public:
   CLASS_PROTOTYPE(PlayAnimSeekEnemy);

   void             SetArgs(Event *ev);
   void             AnimDone(Event *ev);
   virtual void     ShowInfo(Actor &self)    override;
   virtual void     Begin(Actor &self)       override;
   virtual qboolean Evaluate(Actor &self)    override;
   virtual void     End(Actor &self)         override;
   virtual void     Archive(Archiver &arc)   override;
   virtual void     Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void PlayAnimSeekEnemy::Archive(Archiver &arc)
{
   Behavior::Archive(arc);

   arc.WriteObject(&aim);
   arc.WriteInteger(mode);
   arc.WriteBoolean(animdone);
   arc.WriteString(anim);
   arc.WriteString(oldanim);
}

inline EXPORT_FROM_DLL void PlayAnimSeekEnemy::Unarchive(Archiver &arc)
{
   Behavior::Unarchive(arc);

   arc.ReadObject(&aim);
   arc.ReadInteger(&mode);
   arc.ReadBoolean(&animdone);
   arc.ReadString(&anim);
   arc.ReadString(&oldanim);
}

#endif /* behavior.h */

// EOF

