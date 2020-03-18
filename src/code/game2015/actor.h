//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/actor.h                          $
// $Revision:: 90                                                             $
//   $Author:: Jimdose                                                        $
//     $Date:: 1/29/99 7:02p                                                  $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
// 
// DESCRIPTION:
// Base class for character AI.
//

#ifndef __ACTOR_H__
#define __ACTOR_H__

#include "g_local.h"
#include "weapon.h"
#include "sentient.h"
#include "container.h"
#include "stack.h"
#include "navigate.h"
#include "behavior.h"
#include "scriptmaster.h"
#include "prioritystack.h"

#include <float.h>

extern Event EV_Actor_Start;
extern Event EV_Actor_Dead;
extern Event EV_Actor_PopAnim;

extern Event EV_Actor_LookAt;
extern Event EV_Actor_TurnTo;
extern Event EV_Actor_FinishedBehavior;
extern Event EV_Actor_NotifyBehavior;
extern Event EV_Actor_FinishedMove;
extern Event EV_Actor_FinishedAnim;
extern Event EV_Actor_WalkTo;
extern Event EV_Actor_RunTo;
extern Event EV_Actor_Anim;
extern Event EV_Actor_Attack;
extern Event EV_Actor_InPain;
extern Event EV_Actor_Gib;
extern Event EV_Actor_ForwardToBehavior;
extern Event EV_Actor_Aim;
extern Event EV_Actor_MeleeRange;
extern Event EV_Actor_MeleeDamage;
extern Event EV_Actor_AttackFinished;
extern Event EV_Actor_Attack;
extern Event EV_Actor_AttackPlayer;

typedef enum
{
   RANGE_MELEE,
   RANGE_NEAR,
   RANGE_MID,
   RANGE_FAR
} range_t;

enum
{
   ON_SIGHT,
   ON_ATTACK
};

typedef enum
{
   IGNORE,
   ATTACK,
   FLEE,
} reaction_t;

typedef enum
{
   LIKES,         // Will not fire at, even if attacked
   TOLERATES,     // Will attack when attacked
   HATES,         // Will attack on sight
   WARY,          // Will flee when attacked
   FEARS,         // Will flee on sight
   NUM_DISPOSITIONS
} disposition_t;

typedef enum
{
   IS_INANIMATE,
   IS_MONSTER,
   IS_ENEMY,
   IS_CIVILIAN,
   IS_FRIEND,
   IS_ANIMAL,
   NUM_ACTORTYPES
} actortype_t;

#define AI_CANWALK   0x00000001
#define AI_CANSWIM   0x00000002
#define AI_CANFLY    0x00000004

#define WEAK_WEIGHT     0.5
#define WEAK_HEALTH     20
#define STRONGER_WEIGHT 0.9
#define VISIBLE_WEIGHT  0.5
#define NEWENEMY_WEIGHT 0.75  // if he's a new enemy

//#define DAMAGE_WEIGHT 0.5   // If he's done a lot of damage to you
//#define WEAPON_WEIGHT 1.5   // How much the weapon influences you

class EXPORT_FROM_DLL StateInfo : public Class
{
public:
   CLASS_PROTOTYPE(StateInfo);

   str      action;
   str      response;
   qboolean ignore = true;

   virtual void         Archive(Archiver &arc)   override;
   virtual void         Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void StateInfo::Archive(Archiver &arc)
{
   Class::Archive(arc);

   arc.WriteString(action);
   arc.WriteString(response);
   arc.WriteBoolean(ignore);
}

inline EXPORT_FROM_DLL void StateInfo::Unarchive(Archiver &arc)
{
   Class::Unarchive(arc);

   arc.ReadString(&action);
   arc.ReadString(&response);
   arc.ReadBoolean(&ignore);
}

#ifdef EXPORT_TEMPLATE
template class EXPORT_FROM_DLL Container<StateInfo *>;
#endif

class EXPORT_FROM_DLL ActorState : public Class
{
public:
   CLASS_PROTOTYPE(ActorState);

   str                     name;
   str                     anim;
   Event                  *animDoneEvent = nullptr;
   Behavior               *behavior      = nullptr;
   PathPtr                 path;
   int                     thread;
   ThreadMarker            marker;
   Container<StateInfo *>  actionList;
   virtual void            Archive(Archiver &arc)   override;
   virtual void            Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void ActorState::Archive(Archiver &arc)
{
   int i, num;

   Class::Archive(arc);

   arc.WriteString(name);
   arc.WriteString(anim);
   arc.WriteEvent(*animDoneEvent);
   //FIXME
   arc.WriteBoolean(behavior != NULL);
   if(behavior)
   {
      arc.WriteObject(behavior);
   }
   arc.WriteSafePointer(path);
   arc.WriteInteger(thread);
   arc.WriteObject(&marker);
   num = actionList.NumObjects();
   arc.WriteInteger(num);
   for(i = 1; i <= num; i++)
   {
      arc.WriteObject(actionList.ObjectAt(i));
   }
}

inline EXPORT_FROM_DLL void ActorState::Unarchive(Archiver &arc)
{
   Event * ev;
   int i, num;

   Class::Unarchive(arc);

   arc.ReadString(&name);
   arc.ReadString(&anim);
   ev = new Event();
   arc.ReadEvent(ev);
   animDoneEvent = ev;

   //FIXME
   behavior = NULL;
   if(arc.ReadBoolean())
   {
      behavior = (Behavior *)arc.ReadObject();
   }

   arc.ReadSafePointer(&path);
   arc.ReadInteger(&thread);
   arc.ReadObject(&marker);
   actionList.FreeObjectList();
   arc.ReadInteger(&num);
   for(i = 1; i <= num; i++)
   {
      StateInfo * info;

      info = new StateInfo();
      arc.ReadObject(info);
      actionList.AddObject(info);
   }
}

//
// Exported templated classes must be explicitly instantiated
//
#ifdef EXPORT_TEMPLATE
template class EXPORT_FROM_DLL Container<EntityPtr>;
template class EXPORT_FROM_DLL Stack<ActorState *>;
#endif

class EXPORT_FROM_DLL Actor : public Sentient
{
public:
   str                        newanim;
   Event                     *newanimevent;
   int                        newanimnum;

   str                        spawngroup;

   int                        movement;
   stepmoveresult_t           lastmove;
   float                      forwardspeed;

   actortype_t                actortype;
   int                        attackmode;

   Vector                     standsize_min;
   Vector                     standsize_max;
   Vector                     crouchsize_min;
   Vector                     crouchsize_max;

   str                        state;
   str                        animname;
   Container<StateInfo *>     actionList;
   int                        numonstack;
   Stack<ActorState *>        stateStack;

   BehaviorPtr                behavior;
   str                        currentBehavior;

   PathPtr                    path;

   Container<EntityPtr>       targetList;
   Container<EntityPtr>       nearbyList;
   Container<EntityPtr>       enemyList;
   EntityPtr                  currentEnemy;
   qboolean                   seenEnemy;
   range_t                    enemyRange;
   EntityPtr                  lastEnemy;

   float                      fov;
   float                      fovdot;
   float                      vision_distance;

   Vector                     startpos;

   Vector                     move;
   Vector                     movedir;
   float                      movespeed;
   Vector                     movevelocity;
   float                      totallen;
   float                      turnspeed;
   Vector                     animdir;

   float                      chattime;
   float                      nextsoundtime;

   qboolean                   hasalert;

   PathNodePtr                movegoal;
   PathNodePtr                soundnode;
   ThreadPtr                  thread;

   ThreadPtr                  actorthread;
   str                        actorscript;
   str                        actorstart;
   TouchFieldPtr              trig;

   qboolean                   has_melee;
   float                      melee_damage;
   float                      melee_range;
   float                      aim;
   float                      pain_threshold;

   qboolean                   checkStrafe;

   float                      next_drown_time;
   float                      air_finished;
   float                      attack_range;
   float                      shots_per_attack;
   qboolean                   deathgib;

   str                        kill_thread;
   Vector                     eyeoffset;
   float                      last_jump_time;
   qboolean                   nodeathfade;
   qboolean                   nochatter;

   CLASS_PROTOTYPE(Actor);

   // Initialization functions
   Actor();
   ~Actor();
   void                       Start(Event *ev);

   // Vision functions
   range_t                    Range(Entity *ent);
   qboolean                   InFOV(Vector pos);
   qboolean                   InFOV(Entity *ent);
   qboolean                   CanSeeFOV(Entity *ent);
   qboolean                   CanSeeFrom(Vector pos, Entity *ent);
   qboolean                   CanSee(Entity *ent);
   int                        EnemyCanSeeMeFrom(Vector pos);
   qboolean                   CanSeeEnemyFrom(Vector pos);

   // Weapon functions
   qboolean                   WeaponReady(void);
   void                       Attack(Event *ev);
   virtual Vector             GunPosition(void);
   virtual Vector             MyGunAngles(Vector muzzlepos, qboolean firing);
   virtual void               GetGunOrientation(Vector muzzlepos, Vector *forward, Vector *right, Vector *up);
   virtual qboolean           CanShootFrom(Vector pos, Entity *ent, qboolean usecurrentangles);
   virtual qboolean           CanShoot(Entity *ent, qboolean usecurrentangles);

   // Actor type script commands
   void                       FriendEvent(Event *ev);
   void                       CivilianEvent(Event *ev);
   void                       EnemyEvent(Event *ev);
   void                       InanimateEvent(Event *ev);
   void                       MonsterEvent(Event *ev);
   void                       AnimalEvent(Event *ev);

   // Enemy management
   qboolean                   HasEnemies(void);
   qboolean                   IsEnemy(Entity *ent);
   void                       MakeEnemy(Entity *ent, qboolean force = false);
   qboolean                   Likes(Entity *ent);
   qboolean                   Hates(Entity *ent);

   // Targeting functions
   qboolean                   GetVisibleTargets(void);
   void                       TargetEnemies(Event *ev);
   Entity                     *BestTarget(void);
   Sentient                   *NearFriend(void);
   qboolean                   CloseToEnemy(Vector pos, float howclose);
   float                      AttackRange(void);
   float                      MinimumAttackRange(void);

   // State control functions
   void                       EnableState(str action);
   void                       DisableState(str action);
   StateInfo                  *SetResponse(str action, str response, qboolean ignore = false);
   const char                 *GetResponse(str action, qboolean force = false);
   StateInfo                  *GetState(str action);

   // State stack management
   void                       ClearStateStack(void);
   qboolean                   PopState(void);
   void                       PushState(const char *newstate, ScriptThread *newthread = NULL, ThreadMarker *marker = NULL);

   // State control script commands
   void                       DefineStateEvent(Event *ev);
   void                       CopyStateEvent(Event *ev);
   void                       IgnoreAllEvent(Event *ev);
   void                       IgnoreEvent(Event *ev);
   void                       RespondToAllEvent(Event *ev);
   void                       RespondToEvent(Event *ev);
   void                       ClearStateEvent(Event *ev);
   void                       StateDoneEvent(Event *ev);
   void                       SetStateEvent(Event *ev);

   // Thread management
   void                       SetupThread(void);
   qboolean                   DoAction(str name, qboolean force = false);
   qboolean                   ForceAction(str name);
   void                       ProcessScript(ScriptThread *thread, Event *ev = NULL);
   void                       StartMove(Event *ev);
   ScriptVariable             *SetVariable(const char *name, float value);
   ScriptVariable             *SetVariable(const char *name, int value);
   ScriptVariable             *SetVariable(const char *name, const char *text);
   ScriptVariable             *SetVariable(const char *name, str &text);
   ScriptVariable             *SetVariable(const char *name, Entity *ent);
   ScriptVariable             *SetVariable(const char *name, Vector &vec);

   // Thread based script commands
   void                       SetScript(Event *ev);
   void                       SetThread(Event *ev);

   // Behavior management
   void                       EndBehavior(void);
   void                       SetBehaviorEvent(Event *ev);
   void                       SetBehavior(Behavior *newbehavior, Event *argevent = NULL, ScriptThread *thread = NULL);
   void                       FinishedBehavior(Event *ev);
   void                       NotifyBehavior(Event *ev);
   void                       ForwardBehaviorEvent(Event *ev);

   // Path and node management
   void                       SetPath(Path *newpath);
   void                       ReserveNodeEvent(Event *ev);
   void                       ReleaseNodeEvent(Event *ev);

   // Animation control functions
   void                       ChangeAnim(void);
   qboolean                   SetAnim(str anim, Event *ev = NULL);
   qboolean                   SetAnim(str anim, Event &ev);
   void                       Anim(Event *ev);

   // Script commands
   void                       CrouchSize(Event *ev);
   void                       SetFov(Event *ev);
   void                       SetVisionDistance(Event *ev);
   void                       LookAt(Event *ev);
   void                       TurnToEvent(Event *ev);
   void                       IdleEvent(Event *ev);
   void                       WalkTo(Event *ev);
   void                       RunTo(Event *ev);
   void                       AttackEntity(Event *ev);
   void                       AttackPlayer(Event *ev);
   void                       JumpToEvent(Event *ev);
   void                       GotoEvent(Event *ev);

   // Script conditionals
   void                       IfEnemyVisibleEvent(Event *ev);
   void                       IfNearEvent(Event *ev);
   void                       IfCanHideAtEvent(Event *ev);
   void                       IfCanStrafeAttackEvent(Event *ev);
   void                       IfCanMeleeAttackEvent(Event *ev);
   void                       IfCanShootEvent(Event *ev);
   void                       IfEnemyWithinEvent(Event *ev);

   // Sound reaction functions
   void                       IgnoreSoundsEvent(Event *ev);
   void                       RespondToSoundsEvent(Event *ev);
   void                       InvestigateWeaponSound(Event *ev);
   void                       InvestigateMovementSound(Event *ev);
   void                       InvestigatePainSound(Event *ev);
   void                       InvestigateDeathSound(Event *ev);
   void                       InvestigateBreakingSound(Event *ev);
   void                       InvestigateDoorSound(Event *ev);
   void                       InvestigateMutantSound(Event *ev);
   void                       InvestigateVoiceSound(Event *ev);
   void                       InvestigateMachineSound(Event *ev);
   void                       InvestigateRadioSound(Event *ev);

   // Pain and death related functions
   void                       Pain(Event *ev);
   void                       Dead(Event *ev);
   void                       Killed(Event *ev);
   void                       GibEvent(Event *ev);
   void                       RemoveUselessBody(Event *ev);

   // Movement functions
   void                       ForwardSpeedEvent(Event *ev);
   void                       SwimEvent(Event *ev);
   void                       FlyEvent(Event *ev);
   void                       NotLandEvent(Event *ev);
   qboolean                   CanMoveTo(Vector pos);
   void                       Accelerate(Vector steering);
   void                       CalcMove(void);
   void                       setAngles(const Vector &ang);
   stepmoveresult_t           WaterMove(void);
   stepmoveresult_t           AirMove(void);
   stepmoveresult_t           TryMove(void);
   void                       CheckWater(void);
   virtual void               setSize(Vector min, Vector max);

   // Debug functions
   void                       ShowInfo(void);

   // General functions
   virtual void               Chatter(const char *sound, float chance = 10, float volume = 1.0f, int channel = CHAN_VOICE);
   void                       ActivateEvent(Event *ev);
   void                       UseEvent(Event *ev);
   virtual void               Prethink() override;

   virtual void               Archive(Archiver &arc);
   virtual void               Unarchive(Archiver &arc);

   void                       SetAim(Event *ev);
   void                       SetMeleeRange(Event *ev);
   void                       SetMeleeDamage(Event *ev);
   void                       MeleeEvent(Event *ev);
   void                       AttackFinishedEvent(Event *ev);

   void                       CanStrafeEvent(Event *ev);
   void                       NoStrafeEvent(Event *ev);
   void                       SetPainThresholdEvent(Event *ev);
   void                       SetKillThreadEvent(Event *ev);
   void                       AttackRangeEvent(Event *ev);
   void                       AttackModeEvent(Event *ev);
   void                       ShotsPerAttackEvent(Event *ev);
   void                       ClearEnemyEvent(Event *ev);
   void                       SetHealth(Event *ev);
   void                       ClearEnemies(void);
   void                       EyeOffset(Event *ev);
   void                       NoDeathFadeEvent(Event *ev);
   void                       NoChatterEvent(Event *ev);
   void                       TurnSpeedEvent(Event *ev);
   void                       GotoObjectEvent(Event *ev); //###

   qboolean                   HasWeapon(void);
   float                      JumpTo(PathNode * goal, float speed);
   float                      JumpTo(Entity * goal, float speed);
   float                      JumpTo(Vector targ, float speed);
};

inline EXPORT_FROM_DLL void Actor::Archive(Archiver &arc)
{
   int i, num;
   Stack<ActorState *> tempStack;
   ActorState *s;

   Sentient::Archive(arc);

   arc.WriteString(newanim);
   arc.WriteEvent(*newanimevent);
   arc.WriteInteger(newanimnum);

   arc.WriteString(spawngroup);

   arc.WriteInteger(movement);
   // cast as stepmoveresult_t on read
   arc.WriteInteger((int)lastmove);
   arc.WriteFloat(forwardspeed);

   // cast as actortype_t on read
   arc.WriteInteger((int)actortype);
   arc.WriteInteger(attackmode);

   arc.WriteVector(standsize_min);
   arc.WriteVector(standsize_max);
   arc.WriteVector(crouchsize_min);
   arc.WriteVector(crouchsize_max);

   arc.WriteString(state);
   arc.WriteString(animname);

   num = actionList.NumObjects();
   arc.WriteInteger(num);
   for(i = 1; i <= num; i++)
   {
      arc.WriteObject(actionList.ObjectAt(i));
   }

   arc.WriteInteger(numonstack);

   // We have to push all the states onto another stack in order to
   // save them out in the proper order and to restore them after
   // saving the game.
   for(i = 1; i <= numonstack; i++)
   {
      tempStack.Push(stateStack.Pop());
   }

   // put them back on the stateStack as we write them out.
   for(i = 1; i <= numonstack; i++)
   {
      s = tempStack.Pop();
      stateStack.Push(s);
      arc.WriteObject(s);
   }

   // FIXME
   arc.WriteBoolean(behavior != NULL);
   if(behavior)
   {
      arc.WriteObject(behavior);
   }

   arc.WriteString(currentBehavior);

   arc.WriteSafePointer(path);

   num = targetList.NumObjects();
   arc.WriteInteger(num);
   for(i = 1; i <= num; i++)
   {
      arc.WriteSafePointer(targetList.ObjectAt(i));
   }

   num = nearbyList.NumObjects();
   arc.WriteInteger(num);
   for(i = 1; i <= num; i++)
   {
      arc.WriteSafePointer(nearbyList.ObjectAt(i));
   }

   num = enemyList.NumObjects();
   arc.WriteInteger(num);
   for(i = 1; i <= num; i++)
   {
      arc.WriteSafePointer(enemyList.ObjectAt(i));
   }

   arc.WriteSafePointer(currentEnemy);
   arc.WriteBoolean(seenEnemy);
   // cast as range_t on read
   arc.WriteInteger((int)enemyRange);
   arc.WriteSafePointer(lastEnemy);

   arc.WriteFloat(fov);
   arc.WriteFloat(fovdot);
   arc.WriteFloat(vision_distance);

   arc.WriteVector(startpos);

   arc.WriteVector(move);
   arc.WriteVector(movedir);
   arc.WriteFloat(movespeed);
   arc.WriteVector(movevelocity);
   arc.WriteFloat(totallen);
   arc.WriteFloat(turnspeed);
   arc.WriteVector(animdir);

   arc.WriteFloat(chattime);
   arc.WriteFloat(nextsoundtime);

   arc.WriteBoolean(hasalert);

   arc.WriteSafePointer(movegoal);
   arc.WriteSafePointer(soundnode);
   arc.WriteSafePointer(thread);

   arc.WriteSafePointer(actorthread);
   arc.WriteString(actorscript);
   arc.WriteString(actorstart);
   arc.WriteSafePointer(trig);

   arc.WriteBoolean(has_melee);
   arc.WriteFloat(melee_damage);
   arc.WriteFloat(melee_range);
   arc.WriteFloat(aim);
   arc.WriteFloat(pain_threshold);

   arc.WriteBoolean(checkStrafe);

   arc.WriteFloat(next_drown_time);
   arc.WriteFloat(air_finished);
   arc.WriteFloat(attack_range);
   arc.WriteFloat(shots_per_attack);
   arc.WriteBoolean(deathgib);

   arc.WriteString(kill_thread);
   arc.WriteVector(eyeoffset);
   arc.WriteFloat(last_jump_time);
   arc.WriteBoolean(nodeathfade);
   arc.WriteBoolean(nochatter);
}

inline EXPORT_FROM_DLL void Actor::Unarchive(Archiver &arc)
{
   Event * ev;
   int i, num, temp;

   Sentient::Unarchive(arc);

   arc.ReadString(&newanim);
   ev = new Event();
   arc.ReadEvent(ev);
   newanimevent = ev;
   arc.ReadInteger(&newanimnum);

   arc.ReadString(&spawngroup);

   arc.ReadInteger(&movement);
   // cast as stepmoveresult_t on read
   temp = arc.ReadInteger();
   lastmove = (stepmoveresult_t)temp;
   arc.ReadFloat(&forwardspeed);

   // cast as actortype_t on read
   temp = arc.ReadInteger();
   actortype = (actortype_t)temp;
   arc.ReadInteger(&attackmode);

   arc.ReadVector(&standsize_min);
   arc.ReadVector(&standsize_max);
   arc.ReadVector(&crouchsize_min);
   arc.ReadVector(&crouchsize_max);

   arc.ReadString(&state);
   arc.ReadString(&animname);

   arc.ReadInteger(&num);
   for(i = 1; i <= num; i++)
   {
      StateInfo * info;

      info = new StateInfo();
      arc.ReadObject(info);
      actionList.AddObject(info);
   }

   arc.ReadInteger(&numonstack);

   for(i = 1; i <= numonstack; i++)
   {
      ActorState * state;

      state = new ActorState();
      arc.ReadObject(state);
      stateStack.Push(state);
   }

   behavior = NULL;
   if(arc.ReadBoolean())
   {
      behavior = (Behavior *)arc.ReadObject();
   }

   arc.ReadString(&currentBehavior);

   arc.ReadSafePointer(&path);

   arc.ReadInteger(&num);
   targetList.FreeObjectList();
   targetList.Resize(num);
   for(i = 1; i <= num; i++)
   {
      EntityPtr tmp, *ptr;

      targetList.AddObject(tmp);
      ptr = targetList.AddressOfObjectAt(i);
      arc.ReadSafePointer(ptr);
   }

   arc.ReadInteger(&num);
   nearbyList.FreeObjectList();
   nearbyList.Resize(num);
   for(i = 1; i <= num; i++)
   {
      EntityPtr tmp, *ptr;

      nearbyList.AddObject(tmp);
      ptr = nearbyList.AddressOfObjectAt(i);
      arc.ReadSafePointer(ptr);
   }

   arc.ReadInteger(&num);
   enemyList.FreeObjectList();
   enemyList.Resize(num);
   for(i = 1; i <= num; i++)
   {
      EntityPtr tmp, *ptr;

      enemyList.AddObject(tmp);
      ptr = enemyList.AddressOfObjectAt(i);
      arc.ReadSafePointer(ptr);
   }

   arc.ReadSafePointer(&currentEnemy);
   arc.ReadBoolean(&seenEnemy);
   // cast as range_t on read
   temp = arc.ReadInteger();
   enemyRange = (range_t)temp;
   arc.ReadSafePointer(&lastEnemy);

   arc.ReadFloat(&fov);
   arc.ReadFloat(&fovdot);
   arc.ReadFloat(&vision_distance);

   arc.ReadVector(&startpos);

   arc.ReadVector(&move);
   arc.ReadVector(&movedir);
   arc.ReadFloat(&movespeed);
   arc.ReadVector(&movevelocity);
   arc.ReadFloat(&totallen);
   arc.ReadFloat(&turnspeed);
   arc.ReadVector(&animdir);

   arc.ReadFloat(&chattime);
   arc.ReadFloat(&nextsoundtime);

   arc.ReadBoolean(&hasalert);

   arc.ReadSafePointer(&movegoal);
   arc.ReadSafePointer(&soundnode);
   arc.ReadSafePointer(&thread);

   arc.ReadSafePointer(&actorthread);
   arc.ReadString(&actorscript);
   arc.ReadString(&actorstart);
   arc.ReadSafePointer(&trig);

   arc.ReadBoolean(&has_melee);
   arc.ReadFloat(&melee_damage);
   arc.ReadFloat(&melee_range);
   arc.ReadFloat(&aim);
   arc.ReadFloat(&pain_threshold);

   arc.ReadBoolean(&checkStrafe);

   arc.ReadFloat(&next_drown_time);
   arc.ReadFloat(&air_finished);
   arc.ReadFloat(&attack_range);
   arc.ReadFloat(&shots_per_attack);
   arc.ReadBoolean(&deathgib);

   arc.ReadString(&kill_thread);
   arc.ReadVector(&eyeoffset);
   arc.ReadFloat(&last_jump_time);
   arc.ReadBoolean(&nodeathfade);
   arc.ReadBoolean(&nochatter);

   // check some vectors
   if(_isnan(move.x) || _isnan(move.y) || _isnan(move.z))
      move = vec_zero;
   if(_isnan(movedir.x) || _isnan(movedir.y) || _isnan(movedir.z))
      movedir = vec_zero;
   if(_isnan(movevelocity.x) || _isnan(movevelocity.y) || _isnan(movevelocity.z))
      movevelocity = vec_zero;
}


class EXPORT_FROM_DLL MonsterStart : public PathNode
{
private:
   str                        spawngroup;
   MonsterStart               *groupchain;
   MonsterStart               *leader;
   static MonsterStart        *GetGroupLeader(str& groupname);

public:
   CLASS_PROTOTYPE(MonsterStart);

   MonsterStart();
   static MonsterStart        *GetRandomSpot(str& groupname);
   virtual void               Archive(Archiver &arc);
   virtual void               Unarchive(Archiver &arc);
};

inline EXPORT_FROM_DLL void MonsterStart::Archive(Archiver &arc)
{
   PathNode::Archive(arc);

   arc.WriteString(spawngroup);
   arc.WriteObjectPointer(groupchain);
   arc.WriteObjectPointer(leader);
}

inline EXPORT_FROM_DLL void MonsterStart::Unarchive(Archiver &arc)
{
   PathNode::Unarchive(arc);

   arc.ReadString(&spawngroup);
   arc.ReadObjectPointer((Class **)&groupchain);
   arc.ReadObjectPointer((Class **)&leader);
}

// Set destination to node with duck or cover set.  Class will find a path to that node, or a closer one.
class EXPORT_FROM_DLL FindCoverMovement : public StandardMovement
{
public:
   Actor           *self;

   inline qboolean validpath(PathNode *node, int i)
   {
      pathway_t *path;
      PathNode *n;

      path = &node->Child[i];

      if(!StandardMovement::validpath(node, i))
      {
         return false;
      }

      n = AI_GetNode(path->node);
      if(!n || self->CloseToEnemy(n->worldorigin, 128))
      {
         return false;
      }

      return true;
   }

   inline qboolean done(PathNode *node, PathNode *end)
   {
      if(node == end)
      {
         return true;
      }

      //if ( node->reject )
      if(node->reject || !(node->nodeflags & (AI_DUCK | AI_COVER)))
      {
         return false;
      }

      if(self)
      {
         node->reject = self->CanSeeEnemyFrom(node->worldorigin);
         return !node->reject;
      }

      return false;
   }
};

// Set destination to node with flee set.  Class will find a path to that node, or a closer one.
class EXPORT_FROM_DLL FindFleeMovement : public StandardMovement
{
public:
   Actor *self;

   inline qboolean validpath(PathNode *node, int i)
   {
      pathway_t *path;
      PathNode *n;

      path = &node->Child[i];

      if(!StandardMovement::validpath(node, i))
      {
         return false;
      }

      n = AI_GetNode(path->node);
      if(!n || self->CloseToEnemy(n->worldorigin, 128))
      {
         return false;
      }

      return true;
   }

   inline qboolean done(PathNode *node, PathNode *end)
   {
      if(node == end)
      {
         return true;
      }

      //if ( node->reject )
      if(node->reject || !(node->nodeflags & AI_FLEE))
      {
         return false;
      }

      if(self)
      {
         node->reject = self->CanSeeEnemyFrom(node->worldorigin);
         return !node->reject;
      }

      return false;
   }
};

class EXPORT_FROM_DLL FindEnemyMovement : public StandardMovement
{
public:
   Actor *self;

   inline qboolean done(PathNode *node, PathNode *end)
   {
      if(node == end)
      {
         return true;
      }

      if(node->reject)
      {
         return false;
      }

      if(self)
      {
         if(self->currentEnemy)
         {
            node->reject = !self->CanShootFrom(node->worldorigin, self->currentEnemy, false);
         }
         else
         {
            node->reject = false;
         }

         return !node->reject;
      }

      return false;
   }
};

#ifdef EXPORT_TEMPLATE
template class EXPORT_FROM_DLL PathFinder<FindCoverMovement>;
template class EXPORT_FROM_DLL PathFinder<FindFleeMovement>;
template class EXPORT_FROM_DLL PathFinder<FindEnemyMovement>;
#endif

typedef PathFinder<FindCoverMovement> FindCoverPath;
typedef PathFinder<FindFleeMovement>  FindFleePath;
typedef PathFinder<FindEnemyMovement> FindEnemyPath;

#endif /* actor.h */

// EOF

