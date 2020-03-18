//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/behavior.cpp                     $
// $Revision:: 117                                                            $
//   $Author:: Markd                                                          $
//     $Date:: 11/20/98 7:17p                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Behaviors used by the AI.
//

#include "g_local.h"
#include "behavior.h"
#include "actor.h"
#include "doors.h"
#include "object.h"

Event EV_Behavior_Args("args");
Event EV_Behavior_AnimDone("animdone");

/****************************************************************************

  Behavior Class Definition

****************************************************************************/

CLASS_DECLARATION(Listener, Behavior, NULL);

ResponseDef Behavior::Responses[] =
{
   { NULL, NULL }
};

void Behavior::ShowInfo(Actor &self)
{
   if(movegoal)
   {
      gi.printf("movegoal: ( %f, %f, %f ) - '%s'\n",
                movegoal->worldorigin.x, movegoal->worldorigin.y, movegoal->worldorigin.z, movegoal->targetname.c_str());
   }
   else
   {
      gi.printf("movegoal: NULL\n");
   }
}

void Behavior::Begin(Actor &self)
{
}

qboolean	Behavior::Evaluate(Actor &self)
{
   return false;
}

void Behavior::End(Actor &self)
{
}

/****************************************************************************

  Idle Class Definition

****************************************************************************/

CLASS_DECLARATION(Behavior, Idle, NULL);

ResponseDef Idle::Responses[] =
{
   { &EV_Behavior_Args,			(Response)&Idle::SetArgs },
   { NULL, NULL }
};

void Idle::SetArgs(Event *ev)
{
   anim = ev->GetString(2);
}

void Idle::ShowInfo(Actor &self)
{
   Behavior::ShowInfo(self);

   gi.printf("\nnexttwitch : %f\n", nexttwitch);
   gi.printf("anim : %s\n", anim.c_str());
}

void Idle::Begin(Actor &self)
{
   self.currentEnemy = NULL;
   self.seenEnemy = false;
   nexttwitch = level.time + 10 + G_Random(30);

   if(anim.length())
   {
      self.SetAnim(anim);
   }
}

qboolean	Idle::Evaluate(Actor &self)
{
   if(self.currentEnemy)
   {
      if(self.DoAction("sightenemy"))
      {
         self.seenEnemy = true;
         self.Chatter("snd_sightenemy", 5);
      }
      else
      {
         self.currentEnemy = NULL;
      }
      return true;
   }

   if(nexttwitch < level.time)
   {
      self.chattime += 10;
      self.DoAction("twitch");
      return true;
   }
   else
   {
      self.Chatter("snd_idle", 1);
   }

   return true;
}

void Idle::End(Actor &self)
{
}

/****************************************************************************

  Aim Class Definition

****************************************************************************/

CLASS_DECLARATION(Behavior, Aim, NULL);

ResponseDef Aim::Responses[] =
{
   { NULL, NULL }
};

void Aim::SetTarget(Entity *ent)
{
   target = ent;
}

void Aim::ShowInfo(Actor &self)
{
   Behavior::ShowInfo(self);

   gi.printf("\nseek:\n");
   seek.ShowInfo(self);
   if(target)
   {
      gi.printf("\ntarget : #%d '%s'\n", target->entnum, target->targetname.c_str());
   }
   else
   {
      gi.printf("\ntarget : NULL\n");
   }
}

void Aim::Begin(Actor &self)
{
   seek.Begin(self);
}

qboolean Aim::Evaluate(Actor &self)
{
   Vector dir;
   Vector ang;
   Vector pos;

   if(!target)
   {
      return false;
   }

   //
   // get my gun pos
   //
   pos = self.GunPosition();

   ang = self.MyGunAngles(pos, false);

   // 
   // invert PITCH
   //
   ang[PITCH] = -ang[PITCH];

   ang.AngleVectors(&dir, NULL, NULL);

   seek.SetTargetPosition(target->centroid);
   seek.SetTargetVelocity(target->velocity);
   seek.SetPosition(self.centroid);
   seek.SetDir(dir);
   seek.SetMaxSpeed(1400 + skill->value * 600);
   seek.Evaluate(self);
   if((fabs(seek.steeringforce.y) > 5) && (self.enemyRange > RANGE_MELEE))
   {
      seek.steeringforce.y *= 2;
   }

   self.Accelerate(seek.steeringforce);
   if(seek.steeringforce.y < 0.25f)
   {
      // dead on
      return false;
   }

   return true;
}

void Aim::End(Actor &self)
{
   seek.End(self);
}

/****************************************************************************

  FireOnSight Class Definition

****************************************************************************/

CLASS_DECLARATION(Behavior, FireOnSight, NULL);

ResponseDef FireOnSight::Responses[] =
{
   { &EV_Behavior_Args,			(Response)&FireOnSight::SetArgs },
   { NULL, NULL }
};

void FireOnSight::SetArgs (Event *ev)
{
   if(ev->NumArgs() > 1)
   {
      anim = ev->GetString(1);
   }
}

void FireOnSight::ShowInfo (Actor &self)
{
   Behavior::ShowInfo(self);

   gi.printf("\nchase:\n");
   chase.ShowInfo(self);

   gi.printf("\naim:\n");
   aim.ShowInfo(self);
   gi.printf("\nmode : %d\n", mode);
   gi.printf("anim : %s\n", anim.c_str());
}

void FireOnSight::Begin (Actor&self)
{
   mode = 0;
   if(!anim.length())
   {
      anim = "run";
   }
}

qboolean FireOnSight::Evaluate(Actor &self)
{
   if(!self.currentEnemy || self.currentEnemy->deadflag || self.currentEnemy->health <= 0)
   {
      return false;
   }

   switch(mode)
   {
   case 0:
      // Start chasing
      self.SetAnim(anim);
      chase.Begin(self);
      mode = 1;

   case 1:
      // Chasing
      if(self.WeaponReady() && self.CanShoot(self.currentEnemy, false))
      {
         chase.End(self);
         self.SetAnim("readyfire");
         aim.Begin(self);
         mode = 2;
         break;
      }
      else
      {
         self.Chatter("snd_pursuit", 1);
      }

      chase.SetTarget(self.currentEnemy);
      chase.Evaluate(self);
      break;

   case 2:
      // Aiming
      aim.SetTarget(self.currentEnemy);
      aim.Evaluate(self);

      if(self.WeaponReady() && self.CanShoot(self.currentEnemy, true))
      {
         self.Chatter("snd_inmysights", 5);
         self.SetAnim("fire");
         mode = 3;
      }
      else if(!self.WeaponReady() || !self.CanShoot(self.currentEnemy, false))
      {
         aim.End(self);
         mode = 0;
         break;
      }
      break;

   case 3:
      // Fire
      aim.SetTarget(self.currentEnemy);
      aim.Evaluate(self);
      if(!self.CanShoot(self.currentEnemy, true))
      {
         self.SetAnim("aim");
         mode = 2;
      }
      else
      {
         self.Chatter("snd_attacktaunt", 4);
      }
      break;
   }

   return true;
}

void FireOnSight::End(Actor &self)
{
   chase.End(self);
   aim.End(self);
}

/****************************************************************************

  TurnTo Class Definition

****************************************************************************/

CLASS_DECLARATION(Behavior, TurnTo, NULL);

ResponseDef TurnTo::Responses[] =
{
   { NULL, NULL }
};

TurnTo::TurnTo() : Behavior()
{
   dir  = Vector(1, 0, 0);
   mode = 0;
   ent  = NULL;
   yaw  = 0;
   tolerance = 1; //###
}

void TurnTo::SetDirection(float yaw)
{
   Vector ang;

   ang = Vector(0, yaw, 0);
   this->yaw = anglemod(yaw);
   ang.AngleVectors(&dir, NULL, NULL);
   mode = 1;
}

//###
void TurnTo::SetTolerance(float tol)
{
   tolerance = tol;
   if(tolerance < 1)
      tolerance = 1;
   else if(tolerance > 360)
      tolerance = 360;
}
//###

void TurnTo::SetTarget(Entity *ent)
{
   this->ent = ent;
   mode = 2;
}

void TurnTo::ShowInfo(Actor &self)
{
   Behavior::ShowInfo(self);

   gi.printf("\nseek:\n");
   seek.ShowInfo(self);

   if(ent)
   {
      gi.printf("\nent: #%d '%s'\n", ent->entnum, ent->targetname.c_str());
   }
   else
   {
      gi.printf("\nent: NULL\n");
   }

   gi.printf("dir: ( %f, %f, %f )\n", dir.x, dir.y, dir.z);
   gi.printf("yaw: %f\n", yaw);
   gi.printf("mode: %d\n", mode);
   gi.printf("tolerance: %i\n", tolerance); //###
}

void TurnTo::Begin(Actor &self)
{
   seek.Begin(self);
}

qboolean TurnTo::Evaluate(Actor &self)
{
   Vector delta;
   float ang;

   switch(mode)
   {
   case 1:
      ang = angledist(yaw - self.angles.yaw());
      if(fabs(ang) < tolerance) //### added facing angle tolerance
      {
         self.Accelerate(Vector(0, ang, 0));
         return false;
      }

      seek.SetTargetPosition(self.worldorigin + dir);
      seek.SetTargetVelocity(vec_zero);
      break;

   case 2:
      if(!ent)
      {
         return false;
      }

      delta = ent->worldorigin - self.worldorigin;
      yaw = delta.toYaw();
      //if ( self.angles.yaw() == yaw )
      //	{
      //	return false;
      //	}

      seek.SetTargetPosition(ent->worldorigin);
      seek.SetTargetVelocity(vec_zero);
      break;

   default:
      return false;
   }

   seek.SetPosition(self.worldorigin);
   seek.SetDir(self.movedir);
   seek.SetMaxSpeed(self.movespeed);
   seek.Evaluate(self);
   //seek.DrawForces();

   self.Accelerate(seek.steeringforce);

   return true;
}

void TurnTo::End(Actor &self)
{
   seek.End(self);
}

/****************************************************************************

  GotoPathNode Class Definition

****************************************************************************/

CLASS_DECLARATION(Behavior, GotoPathNode, NULL);

ResponseDef GotoPathNode::Responses[] =
{
   { &EV_Behavior_Args,			(Response)&GotoPathNode::SetArgs },
   { NULL, NULL }
};

void GotoPathNode::SetArgs(Event *ev)
{
   anim = ev->GetString(2);

   if(ev->IsVectorAt(3))
   {
      goal = ev->GetVector(3);
      usevec = true;
   }
   else
   {
      usevec = false;
      movegoal = AI_FindNode(ev->GetString(3));
      if(!movegoal)
      {
         goalent = ev->GetEntity(3);
      }
   }
}

void GotoPathNode::SetGoal(PathNode *node)
{
   usevec = false;
   movegoal = node;
}

void GotoPathNode::ShowInfo(Actor &self)
{
   Behavior::ShowInfo(self);

   gi.printf("\nturnto:\n");
   turnto.ShowInfo(self);

   gi.printf("\nchase:\n");
   chase.ShowInfo(self);

   gi.printf("\nstate: %d\n", state);
   gi.printf("usevec: %d\n", usevec);
   gi.printf("time: %f\n", time);
   gi.printf("anim: %s\n", anim.c_str());

   if(goalent)
   {
      gi.printf("\ngoalent: #%d '%s'\n", goalent->entnum, goalent->targetname.c_str());
   }
   else
   {
      gi.printf("\ngoalent: NULL\n");
   }

   gi.printf("goal: ( %f, %f, %f )\n", goal.x, goal.y, goal.z);
}

void GotoPathNode::Begin(Actor &self)
{
   state = 0;
   chase.Begin(self);
   turnto.Begin(self);
   if(goalent)
   {
      chase.SetTarget(goalent);
   }
   else if(movegoal)
   {
      chase.SetGoal(movegoal);
   }
   else
   {
      chase.SetGoalPos(goal);
   }

   // don't check for new paths as often
   chase.SetPathRate(4);

   if(anim.length())
   {
      self.SetAnim(anim);
   }
}

qboolean	GotoPathNode::Evaluate(Actor &self)
{
   float yaw;

   if(!usevec && !goalent && !movegoal)
   {
      return false;
   }

   switch(state)
   {
   case 0:
      if(chase.Evaluate(self))
      {
         break;
      }

      state = 1;
      self.SetAnim("idle");

      // cascade down to case 1
   case 1:
      if(!movegoal)
      {
         return false;
      }

      if(movegoal->setangles)
      {
         yaw = movegoal->worldangles.yaw();
         turnto.SetDirection(yaw);
         if(turnto.Evaluate(self))
         {
            break;
         }
      }

      if(movegoal->animname == "")
      {
         self.SetAnim("idle");
         return false;
      }

      self.SetAnim(movegoal->animname, EV_Actor_FinishedBehavior);
      state = 2;
      break;

   case 2:
      break;
   }

   return true;
}

void GotoPathNode::End(Actor &self)
{
   chase.End(self);
}

/****************************************************************************

  Investigate Class Definition

****************************************************************************/

CLASS_DECLARATION(Behavior, Investigate, NULL);

ResponseDef Investigate::Responses[] =
{
   { &EV_Behavior_Args,			(Response)&Investigate::SetArgs },
   { NULL, NULL }
};

void Investigate::SetArgs(Event *ev)
{
   //Entity *ent;
   //ent = ev->GetEntity(1);

   anim = ev->GetString(2);
   goal = ev->GetVector(3);
}

void Investigate::ShowInfo(Actor &self)
{
   Behavior::ShowInfo(self);

   gi.printf("\nchase:\n");
   chase.ShowInfo(self);
   gi.printf("\nanim: %s\n", anim.c_str());
   gi.printf("curioustime: %f\n", curioustime);
   gi.printf("goal: ( %f, %f, %f )\n", goal.x, goal.y, goal.z);
}

void Investigate::Begin(Actor &self)
{
   //
   // we are only interested for about 10 seconds, if we can't get there, lets go back to what we were doing
   //
   curioustime = level.time + 10;
   self.Chatter("snd_investigate", 10);
   chase.Begin(self);
   chase.SetGoalPos(goal);

   // Don't allow guys to change their anim if we're already close enough to the goal
   if(!Done(self) && anim.length())
   {
      self.SetAnim(anim);
   }
}

qboolean Investigate::Done(Actor &self)
{
   Vector delta;
   float xydist;

   if(curioustime < level.time)
   {
      return true;
   }

   if(self.CanSeeEnemyFrom(self.worldorigin))
   {
      return true;
   }

   if(self.lastmove == STEPMOVE_STUCK)
   {
      return true;
   }
   delta = goal - self.worldorigin;
   // 
   // get rid of Z variance
   //
   delta[2] = 0;
   xydist = delta.length();
   if(xydist < 100)
   {
      return true;
   }

   return false;
}

qboolean	Investigate::Evaluate(Actor &self)
{
   if(Done(self) || !chase.Evaluate(self))
   {
      return false;
   }

   return true;
}

void Investigate::End(Actor &self)
{
   chase.End(self);
}

/****************************************************************************

  Flee Class Definition

****************************************************************************/

CLASS_DECLARATION(Behavior, Flee, NULL);

ResponseDef Flee::Responses[] =
{
   { &EV_Behavior_Args,			(Response)&Flee::SetArgs },
   { NULL, NULL }
};

void Flee::SetArgs(Event *ev)
{
   anim = ev->GetString(2);
}

void Flee::ShowInfo(Actor &self)
{
   Behavior::ShowInfo(self);

   gi.printf("\nfollow:\n");
   follow.ShowInfo(self);

   if(path)
   {
      gi.printf("\npath : ( %f, %f, %f ) to ( %f, %f, %f )\n",
                path->Start()->worldorigin.x, path->Start()->worldorigin.y, path->Start()->worldorigin.z,
                path->End()->worldorigin.x, path->End()->worldorigin.y, path->End()->worldorigin.z);
   }
   else
   {
      gi.printf("\npath : NULL\n");
   }

   gi.printf("\navoid:\n");
   avoid.ShowInfo(self);

   gi.printf("\navoidtime: %f\n", avoidtime);
   gi.printf("anim: %s\n", anim.c_str());
}

void Flee::Begin(Actor &self)
{
   follow.Begin(self);
   avoid.AvoidWalls(false);
   avoid.Begin(self);
   avoidtime = 0;

   path = NULL;

   if(anim.length())
   {
      self.SetAnim(anim);
   }
}

qboolean Flee::Evaluate(Actor &self)
{
   PathNode *node;
   int i;

   self.Chatter("snd_panic", 3);

   if(path && follow.DoneWithPath(self))
   {
      path = NULL;

      if(!self.currentEnemy || !self.CanSee(self.currentEnemy))
      {
         return false;
      }
   }

   if(!path)
   {
      for(i = 0; i < 5; i++)
      {
         node = AI_GetNode((int)G_Random(ai_maxnode + 1));
         if(node)
         {
            break;
         }
      }

      if(node)
      {
         path = follow.SetPath(self, self.worldorigin, node->worldorigin);
      }
      else
      {
         return false;
      }
   }

   follow.SetPosition(self.worldorigin);
   follow.SetDir(self.movedir);
   follow.SetMaxSpeed(self.movespeed);
   follow.Evaluate(self);

   if(avoidtime < level.time)
   {
      avoidtime = level.time + 0.4;

      avoid.SetMaxSpeed(self.movespeed);
      avoid.SetPosition(self.worldorigin);
      avoid.SetDir(self.movedir);
      avoid.Evaluate(self);

      follow.steeringforce += avoid.steeringforce;
   }

   self.Accelerate(follow.steeringforce);

   return true;
}

void Flee::End(Actor &self)
{
   avoid.End(self);
   follow.End(self);
   path = NULL;
}

/****************************************************************************

  OpenDoor Class Definition

****************************************************************************/

CLASS_DECLARATION(Behavior, OpenDoor, NULL);

ResponseDef OpenDoor::Responses[] =
{
   { NULL, NULL }
};

void OpenDoor::SetArgs(Event *ev)
{
   if(ev->NumArgs() > 1)
   {
      dir = ev->GetVector(2);
      //usedir = true;
   }
}

void OpenDoor::ShowInfo(Actor &self)
{
   Behavior::ShowInfo(self);

   gi.printf("\ntime: %f\n", time);
   gi.printf("endtime: %f\n", endtime);
   gi.printf("usedir: %d\n", usedir);
   gi.printf("dir: ( %f, %f, %f )\n", dir.x, dir.y, dir.z);
}

void OpenDoor::Begin(Actor &self)
{
   Event		*e;
   trace_t	trace;
   Entity	*ent;
   Vector	pos;
   Vector	end;

   endtime = 0;

   pos = self.worldorigin + self.eyeposition;
   if(usedir)
   {
      end = pos + dir;
   }
   else
   {
      end = pos + Vector(self.orientation[0]) * 64;
   }

   trace = G_Trace(pos, vec_zero, vec_zero, end, &self, self.edict->clipmask, "OpenDoor 1");

   ent = trace.ent->entity;
   if(ent && ent->isSubclassOf<Door>())
   {
      self.SetAnim("idle");

      time = level.time + 0.1;
      endtime = time + 1;

      e = new Event(EV_Use);
      e->AddEntity(&self);
      ent->ProcessEvent(e);
   }
}

qboolean OpenDoor::Evaluate(Actor &self)
{
   trace_t	trace;
   Vector	pos;

   if(level.time > endtime)
   {
      return false;
   }

   if(time < level.time)
   {
      //pos = self.worldorigin + self.eyeposition;
      //trace = G_Trace( pos, self.mins, self.maxs, pos + Vector( self.orientation[ 0 ] ) * 32, &self, self.edict->clipmask, "OpenDoor 2" );
      //### used same test for door as in OpenDoor::Begin...
      // the previous behavior failed under many conditions

      //pos = self.worldorigin;
      //trace = G_Trace( pos, self.mins + Vector(0, 0, STEPSIZE), self.maxs, pos + Vector( self.orientation[ 0 ] ) * 32, &self, self.edict->clipmask, "OpenDoor 2" );

      //gi.dprintf("org: %i %i %i   mins: %i %i %i   maxs: %i %i %i\n", (int)self.worldorigin.x, (int)self.worldorigin.y, (int)self.worldorigin.z, (int)self.mins.x, (int)self.mins.y, (int)self.mins.z, (int)self.maxs.x, (int)self.maxs.y, (int)self.maxs.z);
      pos = self.worldorigin + Vector(0, 0, 3);
      trace = G_Trace(pos, self.mins + Vector(2, 2, -1), self.maxs + Vector(-2, -2, -5), pos + Vector(self.orientation[0]) * 64, &self, self.edict->clipmask, "OpenDoor 2");

      if(trace.startsolid)
      {
         return true;
      }
      //###

      if(trace.fraction == 1)
      {
         return false;
      }
   }

   return true;
}

void OpenDoor::End(Actor &self)
{
}

/****************************************************************************

  PlayAnim Class Definition

****************************************************************************/

CLASS_DECLARATION(Behavior, PlayAnim, NULL);

ResponseDef PlayAnim::Responses[] =
{
   { &EV_Behavior_Args,			(Response)&PlayAnim::SetArgs },
   { NULL, NULL }
};

void PlayAnim::SetArgs(Event *ev)
{
   anim = ev->GetString(2);
}

void PlayAnim::ShowInfo(Actor &self)
{
   Behavior::ShowInfo(self);

   gi.printf("\nanim: %s\n", anim.c_str());
}

void PlayAnim::Begin(Actor &self)
{
   if(anim.length())
   {
      if(!self.SetAnim(anim, EV_Actor_FinishedBehavior))
      {
         //warning( "Begin", "%s does not exist for %s.", anim.c_str(), self.targetname.c_str() );
         self.PostEvent(EV_Actor_FinishedBehavior, 0);
      }
   }
}

qboolean PlayAnim::Evaluate(Actor &self)
{
   return true;
}

void PlayAnim::End(Actor &self)
{
}

/****************************************************************************

  Wander Class Definition

****************************************************************************/
/*
CLASS_DECLARATION( Behavior, Wander, NULL );

ResponseDef Wander::Responses[] =
   {
      { &EV_Behavior_Args,			( Response )Wander::SetArgs },
      { NULL, NULL }
   };

void Wander::SetArgs
   (
   Event *ev
   )

   {
   anim = ev->GetString( 2 );
   maxdistance = ev->GetFloat( 3 );
   maxdistance *= maxdistance;
   }

PathNode *Wander::FindWanderNode
   (
   Actor &self
   )

   {
   int i;
   PathNode	*bestnode;
   PathNode *node;
   FindCoverPath find;
   Path		*path;
   Vector	delta;
   float		dist;
   Vector	pos;
   int      count = 0;

   pos = self.worldorigin;
   bestnode = NULL;

   for ( i = 0; i < 5; i++ )
      {
      node = AI_GetNode( G_Random( ai_maxnode + 1 ) );

      if ( !node )
         continue;

      delta = node->worldorigin - pos;
      dist = delta * delta;
      if ( ( dist > 1024 ) && ( dist < maxdistance ) )
         {
         bestnode = node;
         break;
         }
      }

   if ( bestnode )
      {
      find.heuristic.self = &self;
      find.heuristic.setSize( self.size );
      find.heuristic.entnum = self.entnum;

      path = find.FindPath( self.worldorigin, bestnode->worldorigin );
      if ( path )
         {
         node = path->End();
         // Mark node as occupied for a short time
         node->occupiedTime = level.time + 1.5;
         node->entnum = self.entnum;
         chase.SetGoal( node );
         chase.SetPath( path );
         return node;
         }
      }
   return NULL;
   }

void Wander::ShowInfo
   (
   Actor &self
   )

   {
   Behavior::ShowInfo( self );

   gi.printf( "\nchase:\n" );
   chase.ShowInfo( self );

   gi.printf( "\nanim: %s\n", anim.c_str() );
   gi.printf( "state: %d\n", state );
   gi.printf( "maxdistance: %f\n", maxdistance );
   }

void Wander::Begin
   (
   Actor &self
   )

   {
   if ( !anim.length() )
      {
      anim = "walk";
      }

   movegoal = NULL;
   state = 0;
   }

qboolean	Wander::Evaluate
   (
   Actor &self
   )

   {
   if ( !movegoal )
      {
      state = 0;
      }

   switch( state )
      {
      case 0 :
         chase.Begin( self );
         movegoal = FindWanderNode( self );
         if ( !movegoal )
            {
            return false;
            }
         if ( anim.length() && ( anim != self.animname ) )
            {
            self.SetAnim( anim );
            }

         state = 1;

      case 1 :
         if ( chase.Evaluate( self ) )
            {
            return true;
            }

         // Look for another wander node
         state = 0;
         chase.End( self );
         return false;
         break;
      }
   return true;
   }

void Wander::End
   (
   Actor &self
   )

   {
   chase.End( self );
   }
*/

/****************************************************************************

  FindCover Class Definition

****************************************************************************/

CLASS_DECLARATION(Behavior, FindCover, NULL);

ResponseDef FindCover::Responses[] =
{
   { &EV_Behavior_Args,			(Response)&FindCover::SetArgs },
   { NULL, NULL }
};

void FindCover::SetArgs(Event *ev)
{
   anim = ev->GetString(2);
}

PathNode *FindCover::FindCoverNode(Actor &self)
{
   int i;
   PathNode	*bestnode;
   float		bestdist;
   PathNode	*desperatebestnode;
   float		desperatebestdist;
   PathNode *node;
   FindCoverPath find;
   Path		*path;
   Vector	delta;
   float		dist;
   Vector	pos;

   pos = self.worldorigin;

   bestnode = NULL;
   bestdist = 999999999; // greater than ( 8192 * sqr(2) ) ^ 2 -- maximum squared distance
   desperatebestnode = NULL;
   desperatebestdist = 999999999; // greater than ( 8192 * sqr(2) ) ^ 2 -- maximum squared distance
   for(i = 0; i <= ai_maxnode; i++)
   {
      node = AI_GetNode(i);
      if(node && (node->nodeflags & (AI_DUCK | AI_COVER)) &&
         ((node->occupiedTime <= level.time) || (node->entnum == self.entnum)))
      {
         // get the distance squared (faster than getting real distance)
         delta = node->worldorigin - pos;
         dist = delta * delta;
         if((dist < bestdist) && (!self.CanSeeEnemyFrom(node->worldorigin) ||//) )//||
            ((node->nodeflags & AI_DUCK) && !self.CanSeeEnemyFrom(node->worldorigin - Vector(0, 0, 32)))))
         {
            bestnode = node;
            bestdist = dist;
         }
         else if((dist < desperatebestdist) && (desperatebestdist > (64 * 64)))
         {
            desperatebestnode = node;
            desperatebestdist = dist;
         }
      }
   }

   if(!bestnode)
   {
      bestnode = desperatebestnode;
   }

   if(bestnode)
   {
      find.heuristic.self = &self;
      find.heuristic.setSize(self.size);
      find.heuristic.entnum = self.entnum;

      path = find.FindPath(self.worldorigin, bestnode->worldorigin);
      if(path)
      {
         node = path->End();

         // Mark node as occupied for a short time
         node->occupiedTime = level.time + 1.5;
         node->entnum = self.entnum;

         chase.SetGoal(node);
         chase.SetPath(path);

         return node;
      }
   }

   return NULL;
}

void FindCover::ShowInfo(Actor &self)
{
   Behavior::ShowInfo(self);

   gi.printf("\nchase:\n");
   chase.ShowInfo(self);

   gi.printf("\nstate: %d\n", state);
   gi.printf("anim: %s\n", anim.c_str());
   gi.printf("nextsearch: %f\n", nextsearch);
}

void FindCover::Begin(Actor &self)
{
   if(!anim.length())
   {
      anim = "run";
   }

   movegoal = NULL;
   state = 0;
}

qboolean FindCover::Evaluate(Actor &self)
{
   if(!movegoal)
   {
      state = 0;
   }

   switch(state)
   {
   case 0:
      // Checking for cover
      chase.Begin(self);
      movegoal = FindCoverNode(self);
      if(!movegoal)
      {
         // Couldn't find any!
         return false;
      }

      // Found cover, going to it
      if(anim.length() && (anim != self.animname))
      {
         self.SetAnim(anim);
      }

      state = 1;
      nextsearch = level.time + 1;

   case 1:
      if(chase.Evaluate(self))
      {
         if(nextsearch < level.time)
         {
            state = 0;
         }
         return true;
      }

      // Reached cover
      if(self.CanSeeEnemyFrom(self.worldorigin))
      {
         state = 0;
      }

      if(movegoal->nodeflags & AI_DUCK)
      {
         // ducking
         self.SetAnim("crouch_down");
      }
      else
      {
         // standing
         self.SetAnim("idle");
      }

      chase.End(self);
      return false;
      break;
   }

   return true;
}

void FindCover::End(Actor &self)
{
   chase.End(self);
}

/****************************************************************************

  FindFlee Class Definition

****************************************************************************/

CLASS_DECLARATION(Behavior, FindFlee, NULL);

ResponseDef FindFlee::Responses[] =
{
   { &EV_Behavior_Args,			(Response)&FindFlee::SetArgs },
   { NULL, NULL }
};

void FindFlee::SetArgs(Event *ev)
{
   anim = ev->GetString(2);
}

PathNode *FindFlee::FindFleeNode(Actor &self)
{
   int i;
   PathNode	*bestnode;
   float		bestdist;
   PathNode	*desperatebestnode;
   float		desperatebestdist;
   PathNode *node;
   FindFleePath find;
   Path		*path;
   Vector	delta;
   float		dist;
   Vector	pos;

   pos = self.worldorigin;

   bestnode = NULL;
   bestdist = 999999999; // greater than ( 8192 * sqr(2) ) ^ 2 -- maximum squared distance
   desperatebestnode = NULL;
   desperatebestdist = 999999999; // greater than ( 8192 * sqr(2) ) ^ 2 -- maximum squared distance
   for(i = 0; i <= ai_maxnode; i++)
   {
      node = AI_GetNode(i);
      if(node && (node->nodeflags & AI_FLEE) &&
         ((node->occupiedTime <= level.time) || (node->entnum == self.entnum)))
      {
         // get the distance squared (faster than getting real distance)
         delta = node->worldorigin - pos;
         dist = delta * delta;
         if((dist < bestdist) && !self.CanSeeEnemyFrom(node->worldorigin))
         {
            bestnode = node;
            bestdist = dist;
         }
         else if((dist < desperatebestdist) && (desperatebestdist > (64 * 64)))
         {
            desperatebestnode = node;
            desperatebestdist = dist;
         }
      }
   }

   if(!bestnode)
   {
      bestnode = desperatebestnode;
   }

   if(bestnode)
   {
      find.heuristic.self = &self;
      find.heuristic.setSize(self.size);
      find.heuristic.entnum = self.entnum;

      path = find.FindPath(self.worldorigin, bestnode->worldorigin);
      if(path)
      {
         node = path->End();

         // Mark node as occupied for a short time
         node->occupiedTime = level.time + 1.5;
         node->entnum = self.entnum;

         chase.SetGoal(node);
         chase.SetPath(path);

         return node;
      }
   }

   return NULL;
}

void FindFlee::ShowInfo(Actor &self)
{
   Behavior::ShowInfo(self);

   gi.printf("\nchase:\n");
   chase.ShowInfo(self);

   gi.printf("\nstate: %d\n", state);
   gi.printf("anim: %s\n", anim.c_str());
   gi.printf("nextsearch: %f\n", nextsearch);
}

void FindFlee::Begin(Actor &self)
{
   if(!anim.length())
   {
      anim = "run";
   }

   movegoal = NULL;
   state = 0;
}

qboolean FindFlee::Evaluate(Actor &self)
{
   if(!movegoal)
   {
      state = 0;
   }

   switch(state)
   {
   case 0:
      // Checking for flee node
      chase.Begin(self);
      movegoal = FindFleeNode(self);
      if(!movegoal)
      {
         // Couldn't find any!
         return false;
      }

      // Found flee node, going to it
      if(anim.length() && (anim != self.animname))
      {
         self.SetAnim(anim);
      }

      state = 1;
      nextsearch = level.time + 1;

   case 1:
      if(chase.Evaluate(self))
      {
         if(nextsearch < level.time)
         {
            state = 0;
         }
         return true;
      }

      // Reached cover
      if(self.CanSeeEnemyFrom(self.worldorigin))
      {
         state = 0;
      }
      else
      {
         // standing
         self.SetAnim("idle");
         chase.End(self);
         return false;
      }
      break;
   }

   return true;
}

void FindFlee::End(Actor &self)
{
   chase.End(self);
}

/****************************************************************************

  FindEnemy Class Definition

****************************************************************************/

CLASS_DECLARATION(Behavior, FindEnemy, NULL);

ResponseDef FindEnemy::Responses[] =
{
   { &EV_Behavior_Args,			(Response)&FindEnemy::SetArgs },
   { NULL, NULL }
};

void FindEnemy::SetArgs(Event *ev)
{
   anim = ev->GetString(2);
}

void FindEnemy::ShowInfo(Actor &self)
{
   Behavior::ShowInfo(self);

   gi.printf("\nchase:\n");
   chase.ShowInfo(self);

   gi.printf("\nstate: %d\n", state);
   gi.printf("nextsearch: %f\n", nextsearch);
   gi.printf("anim: %s\n", anim.c_str());
}

void FindEnemy::Begin(Actor &self)
{
   if(!anim.length())
   {
      anim = "run";
   }

   movegoal = NULL;
   lastSearchNode = NULL;
   state = 0;
}

PathNode *FindEnemy::FindClosestSightNode(Actor &self)
{
   int i;
   PathNode	*bestnode;
   float		bestdist;
   PathNode *node;
   Vector	delta;
   float		dist;
   Vector	pos;

   if(self.currentEnemy)
   {
      pos = self.currentEnemy->worldorigin;
   }
   else
   {
      pos = self.worldorigin;
   }

   bestnode = NULL;
   bestdist = 999999999; // greater than ( 8192 * sqr(2) ) ^ 2 -- maximum squared distance
   for(i = 0; i <= ai_maxnode; i++)
   {
      node = AI_GetNode(i);
      if(node && ((node->occupiedTime <= level.time) || (node->entnum != self.entnum)))
      {
         // get the distance squared (faster than getting real distance)
         delta = node->worldorigin - pos;
         dist = delta * delta;
         if((dist < bestdist) && self.CanSeeFrom(node->worldorigin, self.currentEnemy))
         {
            bestnode = node;
            bestdist = dist;
         }
      }
   }

   return bestnode;
}

qboolean FindEnemy::Evaluate(Actor &self)
{
   if(!self.currentEnemy)
   {
      return false;
   }

   if(nextsearch < level.time)
   {
      // check if we should search for the first time
      if(!lastSearchNode)
      {
         //gi.dprintf( "%d: %s(%d) first time\n", level.framenum, self.targetname.c_str(), self.entnum );
         state = 0;
      }
      else
      {
         // search less often if we're far away
         nextsearch = self.DistanceTo(self.currentEnemy) * (1.0 / 512.0);
         if(nextsearch < 1)
         {
            nextsearch = 1;
         }
         nextsearch += level.time;

         // don't search again if our enemy hasn't moved very far
         if(!self.currentEnemy->WithinDistance(lastSearchPos, 256))
         {
            //gi.dprintf( "%d: %s(%d) searching again\n", level.framenum, self.targetname.c_str(), self.entnum );
            state = 0;
         }
      }
   }

   switch(state)
   {
   case 0:
      // Searching for enemy
      chase.Begin(self);
      lastSearchPos = self.currentEnemy->worldorigin;
      movegoal = PathManager.NearestNode(lastSearchPos, &self);
      if(!movegoal)
      {
         movegoal = PathManager.NearestNode(lastSearchPos, &self, false);
      }
      lastSearchNode = movegoal;
      if(movegoal)
      {
         Path *path;
         FindEnemyPath find;
         PathNode *from;

         find.heuristic.self = &self;
         find.heuristic.setSize(self.size);
         find.heuristic.entnum = self.entnum;

         from = PathManager.NearestNode(self.worldorigin, &self);
         if((from == movegoal) && (self.DistanceTo(from->worldorigin) < 8))
         {
            movegoal = NULL;
            from = NULL;
         }

         if(from)
         {
            path = find.FindPath(from, movegoal);
            if(path)
            {
               chase.SetGoal(movegoal);
               chase.SetPath(path);
            }
            else
            {
               movegoal = NULL;
            }
         }
      }

      if(!movegoal)
      {
         if(self.CanSee(self.currentEnemy) || (!self.currentEnemy->groundentity && !self.waterlevel))
         {
            chase.SetGoalPos(self.currentEnemy->worldorigin);
         }
         else
         {
            // Couldn't find enemy
            // since we can't reach em
            // clear out enemy state
            //
            self.ClearEnemies();
            return false;
         }
      }

      // Found enemy, going to it
      if(anim.length() && (anim != self.animname))
      {
         self.SetAnim(anim);
      }

      state = 1;

   case 1:
      if(self.CanShoot(self.currentEnemy, false))
      {
         // Reached enemy
         chase.End(self);
         return false;
      }

      if(!chase.Evaluate(self))
      {
         state = 0;
         nextsearch = 0;
      }
      break;
   }

   return true;
}

void FindEnemy::End(Actor &self)
{
   chase.End(self);
}

/****************************************************************************

  Hide Class Definition

****************************************************************************/

CLASS_DECLARATION(Behavior, Hide, NULL);

ResponseDef Hide::Responses[] =
{
   { &EV_Behavior_Args,			(Response)&Hide::SetArgs },
   { NULL, NULL }
};

void Hide::SetArgs(Event *ev)
{
   anim = ev->GetString(2);
}

void Hide::ShowInfo(Actor &self)
{
   Behavior::ShowInfo(self);

   gi.printf("\nhide:\n");
   hide.ShowInfo(self);

   gi.printf("\nanim: %s\n", anim.c_str());
   gi.printf("state: %d\n", state);
   gi.printf("checktime: %f\n", checktime);
}

void Hide::Begin(Actor &self)
{
   if(!anim.length())
   {
      anim = "run";
   }

   state = 0;
}

qboolean Hide::Evaluate(Actor &self)
{
   switch(state)
   {
      // init run for cover
   case 0:
      state = 1;
      hide.Begin(self);
      if(hide.Evaluate(self) && anim.length())
      {
         self.SetAnim(anim);
      }
      else
      {
         hide.End(self);
         self.SetAnim("crouch_down");
         state = 2;
         checktime = level.time + 1;
      }
      break;


      // running for cover
   case 1:
      if(!hide.Evaluate(self))
      {
         hide.End(self);
         self.SetAnim("crouch_down");
         state = 2;
         checktime = level.time + 1;
      }
      break;

      // wait for a bit
   case 2:
      if(checktime < level.time)
      {
         checktime = level.time + 0.5f;
         if(self.CanSeeEnemyFrom(self.worldorigin))
         {
            hide.Begin(self);
            if(hide.Evaluate(self) && anim.length())
            {
               self.SetAnim(anim);
               state = 1;
            }
            else
            {
               hide.End(self);
               checktime = level.time + 2;
            }
         }
      }
      break;
   }

   return true;
}

void Hide::End(Actor &self)
{
   hide.End(self);
   self.SetAnim("crouch_down");
}

/****************************************************************************

  FleeAndRemove Class Definition

****************************************************************************/

CLASS_DECLARATION(Behavior, FleeAndRemove, NULL);

ResponseDef FleeAndRemove::Responses[] =
{
   { &EV_Behavior_Args,			(Response)&FleeAndRemove::SetArgs },
   { NULL, NULL }
};

void FleeAndRemove::SetArgs(Event *ev)
{
   anim = ev->GetString(2);
}

void FleeAndRemove::ShowInfo(Actor &self)
{
   Behavior::ShowInfo(self);

   gi.printf("\nfindflee:\n");
   flee.ShowInfo(self);

   gi.printf("\nanim: %s\n", anim.c_str());
   gi.printf("state: %d\n", state);
   gi.printf("checktime: %f\n", checktime);
}

void FleeAndRemove::Begin(Actor &self)
{
   if(!anim.length())
   {
      anim = "run";
   }

   state = 0;
}

qboolean FleeAndRemove::Evaluate(Actor &self)
{
   if(!self.currentEnemy)
   {
      return false;
   }

   switch(state)
   {
      // init run for cover
   case 0:
      state = 1;

      if(anim.length())
      {
         self.SetAnim(anim);
      }

      flee.Begin(self);

      // if we fail on the first try, probably can't flee.
      if(!flee.Evaluate(self))
      {
         flee.End(self);
         return false;
      }
      return true;

      // running for cover
   case 1:
      if(!flee.Evaluate(self))
      {
         flee.End(self);
         state = 2;
         checktime = 0;
         self.SetAnim("crouch_down");
      }
      break;

      // wait for a bit
   case 2:
      if(checktime < level.time)
      {
         checktime = level.time + 1;
         if(self.CanSeeEnemyFrom(self.worldorigin))
         {
            state = 0;
         }
         else
         {
            self.PostEvent(EV_Remove, 0);
         }
      }
      break;
   }

   return true;
}

void FleeAndRemove::End(Actor &self)
{
   flee.End(self);
   self.SetAnim("crouch_down");
}

/****************************************************************************

  AimAndShoot Class Definition

****************************************************************************/

CLASS_DECLARATION(Behavior, AimAndShoot, NULL);

ResponseDef AimAndShoot::Responses[] =
{
   { &EV_Behavior_Args,			(Response)&AimAndShoot::SetArgs },
   { &EV_Behavior_AnimDone,	(Response)&AimAndShoot::AnimDone },
   { NULL, NULL }
};

void AimAndShoot::ShowInfo(Actor &self)
{
   Behavior::ShowInfo(self);

   gi.printf("\naim:\n");
   aim.ShowInfo(self);

   gi.printf("\nmode: %d\n", mode);
   gi.printf("maxshots: %d\n", maxshots);
   gi.printf("numshots: %d\n", numshots);
   gi.printf("animdone: %d\n", animdone);
}

void AimAndShoot::Begin(Actor &self)
{
   enemy_health = 0;
   mode = 0;
   animdone = false;

   readyfireanim = animprefix + "readyfire";
   if(!self.HasAnim(readyfireanim.c_str()))
   {
      readyfireanim = "";
   }

   aimanim = animprefix + "aim";
   if(!self.HasAnim(aimanim.c_str()))
   {
      aimanim = "";
   }

   fireanim = animprefix + "fire";
   if(!self.HasAnim(fireanim.c_str()))
   {
      fireanim = "";
   }
}

void AimAndShoot::SetMaxShots(int num)
{
   maxshots = (num >> 1) + G_Random(num);
}

void AimAndShoot::SetArgs(Event *ev)
{
   SetMaxShots(ev->GetInteger(2));
   if(ev->NumArgs() > 2)
   {
      animprefix = ev->GetString(3);
   }
}

void AimAndShoot::AnimDone(Event *ev)
{
   animdone = true;
}

qboolean AimAndShoot::Evaluate(Actor &self)
{
   //### hack to prevent weirdness when a crawler
   // is shot down from the ceiling while attacking
   if((!strcmp(animprefix.c_str(), "ceiling_")) && (self.movetype != MOVETYPE_CEILINGSTEP))
   {
      animprefix = "";

      readyfireanim = "readyfire";
      if(!self.HasAnim(readyfireanim.c_str()))
      {
         readyfireanim = "";
      }

      aimanim = "aim";
      if(!self.HasAnim(aimanim.c_str()))
      {
         aimanim = "";
      }

      fireanim = "fire";
      if(!self.HasAnim(fireanim.c_str()))
      {
         fireanim = "";
      }
   }
   //###

   switch(mode)
   {
   case 0:
      if(!self.currentEnemy)
      {
         return false;
      }

      if(!self.CanShoot(self.currentEnemy, false))
      {
         return false;
      }

      if(readyfireanim.length())
      {
         animdone = false;
         self.SetAnim(readyfireanim.c_str(), EV_Actor_NotifyBehavior);
         aim.Begin(self);
         mode = 1;
      }
      else
      {
         // skip the ready fire animation
         animdone = true;
      }

   case 1:
      // readying gun
      if(!animdone)
      {
         aim.SetTarget(self.currentEnemy);
         aim.Evaluate(self);
         break;
      }
      // start Aiming
      animdone = false;
      if(aimanim.length())
      {
         self.SetAnim(aimanim.c_str());
      }
      // 
      // save off time, in case we aim for too long
      //
      aim_time = level.time + 1;
      mode = 2;

   case 2:
      // Aiming
      if(!self.currentEnemy)
      {
         return false;
      }
      //
      // see if we aimed for too long
      //
      if(aim_time < level.time)
      {
         return false;
      }

      aim.SetTarget(self.currentEnemy);
      aim.Evaluate(self);

      // don't go into our firing animation until our weapon is ready, and we are on target
      if(self.WeaponReady() && self.currentEnemy && self.CanShoot(self.currentEnemy, true))
      {
         animdone = false;
         self.Chatter("snd_inmysights", 5);
         self.SetAnim(fireanim.c_str(), EV_Actor_NotifyBehavior);
         enemy_health = self.currentEnemy->health;
         mode = 3;
      }
      else if(!self.currentEnemy || self.currentEnemy->deadflag ||
         (self.currentEnemy->health <= 0) || !self.CanShoot(self.currentEnemy, false))
      {
         // either our enemy is dead, or we can't shoot the enemy from here
         if(self.CurrentWeapon())
         {
            self.CurrentWeapon()->ForceReload();
         }
         return false;
      }
      break;

   case 3:
      // Fire
      aim.SetTarget(self.currentEnemy);
      aim.Evaluate(self);
      if(animdone)
      {
         if(!self.currentEnemy || (self.currentEnemy->health < enemy_health))
            self.Chatter("snd_attacktaunt", 4);
         else
            self.Chatter("snd_missed", 7);

         animdone = false;
         numshots++;

         if((numshots >= maxshots) || !self.currentEnemy || self.currentEnemy->deadflag ||
            (self.currentEnemy->health <= 0) || !self.CanShoot(self.currentEnemy, false))
         {
            // either we're out of shots, our enemy is dead, or we can't shoot the enemy from here
            if(self.CurrentWeapon())
            {
               self.CurrentWeapon()->ForceReload();
            }
            return false;
         }
         else if(!self.WeaponReady() || !self.CanShoot(self.currentEnemy, false))
         {
            // weapon not ready or not aimed at enemy, so just keep trying to get enemy in our sights
            if(aimanim.length())
            {
               self.SetAnim(aimanim.c_str());
            }
            // 
            // save off time, in case we aim for too long
            //
            aim_time = level.time + 1;
            mode = 2;
         }
         else
         {
            // keep firing
            self.SetAnim(fireanim.c_str(), EV_Actor_NotifyBehavior);
            enemy_health = self.currentEnemy->health;
         }
      }
      break;
   }

   return true;
}

void AimAndShoot::End(Actor &self)
{
   aim.End(self);

   //### added anim prefix support to the idle animation resetting
   //self.SetAnim( "idle" );
   fireanim = animprefix;
   fireanim += "idle";
   if(!self.HasAnim(fireanim.c_str()))
   {
      fireanim = "idle";
   }
   self.SetAnim(fireanim);
   //###
}

/****************************************************************************

  AimAndMelee Class Definition

****************************************************************************/

CLASS_DECLARATION(Behavior, AimAndMelee, NULL);

ResponseDef AimAndMelee::Responses[] =
{
   { &EV_Behavior_Args,			(Response)&AimAndMelee::SetArgs },
   { &EV_Behavior_AnimDone,	(Response)&AimAndMelee::AnimDone },
   { NULL, NULL }
};

void AimAndMelee::ShowInfo(Actor &self)
{
   Behavior::ShowInfo(self);

   gi.printf("\naim:\n");
   aim.ShowInfo(self);

   gi.printf("\nmode: %d\n", mode);
   gi.printf("maxshots: %d\n", maxshots);
   gi.printf("numshots: %d\n", numshots);
   gi.printf("animdone: %d\n", animdone);
}

void AimAndMelee::Begin(Actor &self)
{
   mode = 0;
   numshots = 0;
   maxshots = 2 + G_Random(4);
   animdone = false;
}

void AimAndMelee::SetArgs(Event *ev)
{
   int num;

   num = ev->GetInteger(2);
   maxshots = (num >> 1) + G_Random(num);
}

void AimAndMelee::AnimDone(Event *ev)
{
   animdone = true;
}

qboolean AimAndMelee::Evaluate(Actor &self)
{
   float r;
   Vector delta;

   switch(mode)
   {
   case 0:
      if(!self.has_melee || !self.currentEnemy)
      {
         return false;
      }

      delta = self.centroid - self.currentEnemy->centroid;
      r = delta.length();
      if(r > self.melee_range)
      {
         return false;
      }

      aim.SetTarget(self.currentEnemy);
      if(aim.Evaluate(self))
      {
         break;
      }
      numshots++;
      animdone = false;
      // melee
      self.SetAnim("melee", EV_Actor_NotifyBehavior);
      self.Chatter("snd_attacktaunt", 4);

      mode = 1;

   case 1:
      // finish up the attack
      if(animdone)
      {
         if(numshots < maxshots)
         {
            mode = 0;
         }
         else
         {
            return false;
         }
      }
      break;
   }

   return true;
}

void AimAndMelee::End(Actor &self)
{
   aim.End(self);
   self.SetAnim("idle");
}

/****************************************************************************

  Melee Class Definition

****************************************************************************/

CLASS_DECLARATION(Behavior, Melee, NULL);

ResponseDef Melee::Responses[] =
{
   { &EV_Behavior_Args,			(Response)&Melee::SetArgs },
   { &EV_Behavior_AnimDone,	(Response)&Melee::AnimDone },
   { NULL, NULL }
};

void Melee::ShowInfo(Actor &self)
{
   Behavior::ShowInfo(self);

   gi.printf("\nmode: %d\n", mode);
   gi.printf("animdone: %d\n", animdone);
}

void Melee::Begin(Actor &self)
{
   mode = 0;
   animdone = false;
}

void Melee::SetArgs(Event *ev)
{
}

void Melee::AnimDone(Event *ev)
{
   animdone = true;
}

qboolean Melee::Evaluate(Actor &self)
{
   float r;
   Vector delta;
   Vector ang;

   switch(mode)
   {
   case 0:
      if(!self.has_melee || !self.currentEnemy)
      {
         return false;
      }

      delta = self.currentEnemy->worldorigin - self.worldorigin;
      r = delta.length();
      if(r > self.melee_range)
      {
         return false;
      }
      animdone = false;
      // melee
      ang = delta.toAngles();
      ang[PITCH] = -ang[PITCH];
      self.setAngles(ang);
      self.SetAnim("melee", EV_Actor_NotifyBehavior);
      self.Chatter("snd_attacktaunt", 4);

      mode = 1;

   case 1:
      // finsh up the attack
      if(animdone)
      {
         return false;
      }
      break;
   }

   return true;
}

void Melee::End(Actor &self)
{
   self.SetAnim("idle");
}

/****************************************************************************

  Repel Class Definition

****************************************************************************/

CLASS_DECLARATION(Behavior, Repel, NULL);

ResponseDef Repel::Responses[] =
{
   { &EV_Behavior_Args,			(Response)&Repel::SetArgs },
   { NULL, NULL }
};

void Repel::SetArgs(Event *ev)
{
   anim = ev->GetString(2);
   movegoal = AI_FindNode(ev->GetString(3));
   if(movegoal)
   {
      goal = movegoal->worldorigin;
   }
   speed = ev->GetFloat(4);
}

void Repel::ShowInfo(Actor &self)
{
   Behavior::ShowInfo(self);

   gi.printf("\nanim: %s\n", anim.c_str());

   gi.printf("dist: %f\n", dist);
   gi.printf("len: %f\n", len);
   gi.printf("speed: %f\n", speed);
   gi.printf("goal: ( %f, %f, %f )\n", goal.x, goal.y, goal.z);
   gi.printf("start: ( %f, %f, %f )\n", start.x, start.y, start.z);
   gi.printf("dir: ( %f, %f, %f )\n", dir.x, dir.y, dir.z);
}

void Repel::Begin(Actor &self)
{
   start.x = goal.x;
   start.y = goal.y;
   start.z = self.worldorigin.z;
   dir = goal - start;
   len = dir.length();
   dir *= 1 / len;

   if(speed <= 0)
   {
      speed = 32;
   }

   if(anim.length())
   {
      self.SetAnim(anim);
   }
   dist = -1;
}

qboolean Repel::Evaluate(Actor &self)
{
   Vector pos;
   qboolean done;
   float sp;
   trace_t trace;

   done = false;

   // this is silly, but it works
   if(dist < 0)
   {
      sp = 0;
   }
   else if(dist < 32)
   {
      sp = dist * speed / 32;
   }
   else if((len - dist) < 32)
   {
      sp = (len - dist) * speed / 32;
   }
   else
   {
      sp = speed;
   }

   pos = start + dir * (dist + sp);
   dist = (pos - start) * dir;
   if(dist >= (len - 1))
   {
      pos = goal;
      done = true;
   }
   else if(dist < 1)
   {
      dist = 1;
   }

   trace = G_Trace(self.worldorigin, self.mins, self.maxs, pos, &self, self.edict->clipmask, "Repel");
   self.setOrigin(trace.endpos);

   return !done;
}

void Repel::End(Actor &self)
{
   self.SetAnim("idle");
}

/****************************************************************************

  Pickup Behavior Class Definition

****************************************************************************/

CLASS_DECLARATION(Behavior, PickupAndThrow, NULL);

Event EV_PickupAndThrow_Pickup("pickup");
Event EV_PickupAndThrow_Throw("throw");

ResponseDef PickupAndThrow::Responses[] =
{
   { &EV_Behavior_Args,         (Response)&PickupAndThrow::SetArgs },
   { &EV_Behavior_AnimDone,     (Response)&PickupAndThrow::AnimDone },
   { &EV_PickupAndThrow_Pickup, (Response)&PickupAndThrow::Pickup },
   { &EV_PickupAndThrow_Throw,  (Response)&PickupAndThrow::Throw },
   { NULL, NULL }
};

void PickupAndThrow::ShowInfo(Actor &self)
{
   Behavior::ShowInfo(self);

   gi.printf("\naim:\n");
   aim.ShowInfo(self);

   gi.printf("\nmode: %d\n", mode);
   gi.printf("animdone: %d\n", animdone);

   if(pickup_target)
   {
      gi.printf("\npickup_target: #%d '%s'\n", pickup_target->entnum, pickup_target->targetname.c_str());
   }
   else
   {
      gi.printf("\npickup_target: NULL\n");
   }
}

void PickupAndThrow::Begin(Actor &self)
{
   mode = 0;
   animdone = false;
}

void PickupAndThrow::SetArgs(Event *ev)
{
   pickup_target = ev->GetEntity(2);
}

void PickupAndThrow::AnimDone(Event *ev)
{
   animdone = true;
}

void PickupAndThrow::Pickup(Event *ev)
{
   Entity * ent;
   Event * e;

   ent = ev->GetEntity(1);
   if(pickup_target)
   {
      e = new Event(EV_ThrowObject_Pickup);
      e->AddEntity(ent);
      e->AddString(ev->GetString(2));
      pickup_target->ProcessEvent(e);
   }
}

void PickupAndThrow::Throw(Event *ev)
{
   Actor * act;
   Event * e;

   act = (Actor *)ev->GetEntity(1);
   if(pickup_target)
   {
      if(!act->currentEnemy)
         return;
      e = new Event(EV_ThrowObject_Throw);
      e->AddEntity(act);
      e->AddFloat(500);
      e->AddEntity(act->currentEnemy);
      e->AddFloat(1);
      pickup_target->ProcessEvent(e);
   }
}

qboolean PickupAndThrow::Evaluate(Actor &self)
{
   Event * ev;

   if(!self.currentEnemy || !pickup_target)
      return false;

   switch(mode)
   {
   case 0:
      if(self.HasAnim("pickup"))
      {
         animdone = false;
         self.SetAnim("pickup", EV_Actor_NotifyBehavior);
         mode = 1;
      }
      else
      {
         // skip the pickup animation
         ev = new Event(EV_PickupAndThrow_Pickup);
         ev->AddEntity(&self);
         ev->AddString("gun");
         ProcessEvent(ev);
         animdone = true;
      }

   case 1:
      if(!animdone)
         break;
      aim.Begin(self);
      mode = 2;
      if(self.HasAnim("throw_aim"))
      {
         self.SetAnim("throw_aim", EV_Actor_NotifyBehavior);
      }
      else
      {
         self.SetAnim("idle");
      }

   case 2:
      // aim towards our target
      aim.SetTarget(self.currentEnemy);
      if(aim.Evaluate(self))
      {
         break;
      }
      mode = 3;

   case 3:
      // Throwing
      mode = 4;
      if(self.HasAnim("throw"))
      {
         animdone = false;
         self.SetAnim("throw", EV_Actor_NotifyBehavior);
         mode = 4;
      }
      else
      {
         // skip the pickup animation
         ev = new Event(EV_PickupAndThrow_Throw);
         ev->AddEntity(&self);
         ProcessEvent(ev);
         animdone = true;
      }
      break;

   case 4:
      if(!animdone)
         break;
      return false;
      break;
   }

   return true;
}

void PickupAndThrow::End(Actor &self)
{
   aim.End(self);
   self.SetAnim("idle");
}

/****************************************************************************

  Jump Class Definition

****************************************************************************/

CLASS_DECLARATION(Behavior, Jump, NULL);

ResponseDef Jump::Responses[] =
{
   { &EV_Behavior_Args,			(Response)&Jump::SetArgs },
   { &EV_Behavior_AnimDone,	(Response)&Jump::AnimDone },
   { NULL, NULL }
};

Jump::Jump() : Behavior()
{
   endtime = 0;
   speed = 200;
   state = 0;
}

void Jump::AnimDone(Event *ev)
{
   animdone = true;
}

void Jump::SetArgs(Event *ev)
{
   anim = ev->GetString(2);

   //
   // see if it is an entity first 
   //
   movegoal = AI_FindNode(ev->GetString(3));
   if(movegoal)
   {
      goal = movegoal->worldorigin;
   }
   else
   {
      Entity *ent;

      ent = ev->GetEntity(3);
      if(ent)
      {
         goal = ent->worldorigin;
      }
      else
      {
         gi.dprintf("Jump::SetArgs invalid target %s", ev->GetString(3));
      }
   }

   if(ev->NumArgs() >= 4)
      speed = ev->GetFloat(4);
   else
      speed = 0;
}

void Jump::ShowInfo(Actor &self)
{
   Behavior::ShowInfo(self);

   gi.printf("\nendtime: %f\n", endtime);
   gi.printf("speed: %f\n", speed);
   gi.printf("state: %d\n", state);
   gi.printf("animdone: %d\n", animdone);
   gi.printf("anim: %s\n", anim.c_str());
}

void Jump::Begin(Actor &self)
{
   float traveltime;

   if(anim.length())
   {
      self.SetAnim(anim);
   }
   traveltime = self.JumpTo(goal, speed);
   endtime = traveltime + level.time;

   self.last_jump_time = endtime;

   state = 0;
}

qboolean Jump::Evaluate(Actor &self)
{
   switch(state)
   {
   case 0:
      state = 1;
      // this is here so that we at least hit this function at least once
      // this gaves the character the chance to leave the ground, nulling out
      // self.groundentity
      break;
   case 1:
      //
      // wait for the character to hit the ground
      //
      if(self.groundentity)
      {
         state = 2;
         //
         // if we have an anim, we go to state 3
         //
         if(self.HasAnim("land"))
         {
            animdone = false;
            self.SetAnim("land", EV_Actor_NotifyBehavior);
            state = 3;
         }
         else
         {
            return false;
         }
      }
      break;
   case 2:
      // 
      // we are on the ground and waiting to timeout
      //
      if(level.time > endtime)
         return false;
      break;
   case 3:
      //
      // we are on the ground and waiting for our landing animation to finish
      //
      if(animdone)
      {
         return false;
      }
      break;
   }

   return true;
}

void Jump::End(Actor &self)
{
   //turn.End( self );
   self.SetAnim("idle");
}

/****************************************************************************

  StrafeAttack Class Definition

****************************************************************************/

CLASS_DECLARATION(Behavior, StrafeAttack, NULL);

ResponseDef StrafeAttack::Responses[] =
{
   { NULL, NULL }
};

void StrafeAttack::ShowInfo(Actor &self)
{
   Behavior::ShowInfo(self);

   gi.printf("\nturn:\n");
   turn.ShowInfo(self);

   gi.printf("\nstate: %d\n", state);
}

void StrafeAttack::Begin(Actor &self)
{
   state = 0;
}

qboolean StrafeAttack::Evaluate(Actor &self)
{
   int num;
   Vector delta;
   Vector left;
   Vector pos;

   if(!self.currentEnemy)
   {
      return false;
   }

   switch(state)
   {
   case 0:
      delta = self.currentEnemy->worldorigin - self.worldorigin;
      turn.SetDirection(delta.toYaw());
      turn.Begin(self);
      state = 1;
      break;

   case 1:
      if(turn.Evaluate(self))
      {
         return true;
      }

      turn.End(self);
      state = 2;

   case 2:
      delta = self.currentEnemy->worldorigin - self.worldorigin;
      left.x = -delta.y;
      left.y = delta.x;
      left.normalize();

      if(G_Random(10) < 5)
      {
         num = gi.Anim_Random(self.edict->s.modelindex, "step_left");
         if(num != -1)
         {
            gi.Anim_Delta(self.edict->s.modelindex, num, delta.vec3());
            delta *= self.edict->s.scale;
            pos = self.worldorigin + left * delta.length();
            if(self.CanMoveTo(pos) && self.CanShootFrom(pos, self.currentEnemy, false))
            {
               self.SetAnim("step_left", EV_Actor_FinishedBehavior);
               state = 3;
               return true;
            }
         }

         num = gi.Anim_Random(self.edict->s.modelindex, "step_right");
         if(num != -1)
         {
            gi.Anim_Delta(self.edict->s.modelindex, num, delta.vec3());
            delta *= self.edict->s.scale;
            pos = self.worldorigin - left * delta.length();
            if(self.CanMoveTo(pos) && self.CanShootFrom(pos, self.currentEnemy, false))
            {
               self.SetAnim("step_right", EV_Actor_FinishedBehavior);
               state = 3;
               return true;
            }
         }
      }
      else
      {
         num = gi.Anim_Random(self.edict->s.modelindex, "step_right");
         if(num != -1)
         {
            gi.Anim_Delta(self.edict->s.modelindex, num, delta.vec3());
            delta *= self.edict->s.scale;
            pos = self.worldorigin - left * delta.length();
            if(self.CanMoveTo(pos) && self.CanShootFrom(pos, self.currentEnemy, false))
            {
               self.SetAnim("step_right", EV_Actor_FinishedBehavior);
               state = 3;
               return true;
            }
         }

         num = gi.Anim_Random(self.edict->s.modelindex, "step_left");
         if(num != -1)
         {
            gi.Anim_Delta(self.edict->s.modelindex, num, delta.vec3());
            delta *= self.edict->s.scale;
            pos = self.worldorigin + left * delta.length();
            if(self.CanMoveTo(pos) && self.CanShootFrom(pos, self.currentEnemy, false))
            {
               self.SetAnim("step_left", EV_Actor_FinishedBehavior);
               state = 3;
               return true;
            }
         }

      }

      return false;
      break;
   }

   return true;
}

void StrafeAttack::End(Actor &self)
{
   turn.End(self);
   self.SetAnim("idle");
}

/****************************************************************************

  StrafeTo Class Definition

****************************************************************************/

CLASS_DECLARATION(Behavior, StrafeTo, NULL);

ResponseDef StrafeTo::Responses[] =
{
   { &EV_Behavior_Args,			(Response)&StrafeTo::SetArgs },
   { NULL, NULL }
};

void StrafeTo::ShowInfo(Actor &self)
{
   Behavior::ShowInfo(self);

   gi.printf("goal: ( %f, %f, %f )\n", goal.x, goal.y, goal.z);
   gi.printf("fail: %d\n", fail);

   gi.printf("\nseek:\n");
   seek.ShowInfo(self);
}

void StrafeTo::SetArgs(Event *ev)
{
   Entity *ent;

   if(ev->IsVectorAt(2))
   {
      goal = ev->GetVector(2);
   }
   else
   {
      movegoal = AI_FindNode(ev->GetString(2));
      if(movegoal)
      {
         goal = movegoal->worldorigin;
      }
      else
      {
         ent = ev->GetEntity(2);
         if(ent)
         {
            goal = ent->worldorigin;
         }
      }
   }
}

void StrafeTo::Begin(Actor &self)
{
   Vector delta;
   float dot;

   seek.Begin(self);

   delta = goal - self.worldorigin;
   dot = delta * self.orientation[1];

   if(dot < 0)
   {
      self.SetAnim("step_right");
   }
   else
   {
      self.SetAnim("step_left");
   }

   fail = 0;
}

qboolean StrafeTo::Evaluate(Actor &self)
{
   seek.SetMaxSpeed(self.movespeed);
   seek.SetPosition(self.worldorigin);
   seek.SetDir(self.movedir);
   seek.SetTargetPosition(goal);

   if(!seek.Evaluate(self))
   {
      return false;
   }

   self.Accelerate(seek.steeringforce);

   // prevent him from trying to strafing forever if he's stuck
   if(self.lastmove != STEPMOVE_OK)
   {
      if(fail)
      {
         return false;
      }

      fail++;
   }
   else
   {
      fail = 0;
   }

   return true;
}

void StrafeTo::End(Actor &self)
{
   seek.End(self);
   self.SetAnim("idle");
}

/****************************************************************************

  Random Direction Utility function

****************************************************************************/

Vector ChooseRandomDirection(Actor &self, Vector previousdir, float *time, int allowcontentsmask, int disallowcontentsmask, qboolean usepitch)
{
   static float x[9] = { 0, 22, -22, 45, -45, 0, 22, -22, 45 };
   Vector dir;
   Vector ang;
   Vector bestdir;
   Vector newdir;
   Vector s;
   float bestfraction;
   trace_t trace;
   int i;
   int j;
   int k;
   int t;
   int u;
   int contents;
   qboolean checkmask;
   Vector centroid;

   centroid = self.centroid - self.worldorigin;

   checkmask = allowcontentsmask || disallowcontentsmask;

   s = Vector(0, 0, STEPSIZE);
   bestfraction = -1;
   bestdir = self.worldorigin;
   for(i = 0; i <= 4; i++)
   {
      t = i * 45;
      if(rand() < 0.5)
      {
         // sometimes we choose left first, other times right.
         t = -t;
      }
      for(j = -1; j < 2; j += 2)
      {
         ang.y = self.worldangles.y + (t * j);

         if(usepitch)
         {
            u = (int)G_Random(5);
            for(k = 0; k < 5; k++)
            {
               ang.x = x[k + u];
               ang.AngleVectors(&dir, NULL, NULL);

               dir *= self.movespeed * (*time);
               dir += self.worldorigin;
               trace = G_Trace(self.worldorigin, self.mins, self.maxs, dir, &self,
                               self.edict->clipmask, "ChooseRandomDirection 1");

               if(!trace.startsolid && !trace.allsolid)
               {
                  newdir = Vector(trace.endpos);
                  if(checkmask)
                  {
                     contents = gi.pointcontents((newdir + centroid).vec3());
                     if(
                        (!allowcontentsmask || (contents & allowcontentsmask)) &&
                        (!disallowcontentsmask || !(contents & disallowcontentsmask)) &&
                        (trace.fraction > bestfraction) &&
                        (newdir != bestdir) &&
                        (newdir != previousdir)
                        )

                     {
                        bestdir = newdir;
                        bestfraction = trace.fraction;
                     }
                  }
                  else
                  {
                     if(
                        (trace.fraction > bestfraction) &&
                        (newdir != bestdir) &&
                        (newdir != previousdir)
                        )
                     {
                        bestdir = newdir;
                        bestfraction = trace.fraction;
                     }
                  }
               }
            }
         }
         else
         {
            ang.x = 0;
            ang.AngleVectors(&dir, NULL, NULL);

            dir *= self.movespeed * (*time);
            dir += self.worldorigin;
            dir += s;
            trace = G_Trace(self.worldorigin + s, self.mins, self.maxs, dir, &self,
                            self.edict->clipmask, "ChooseRandomDirection 2");

            if(!trace.startsolid && !trace.allsolid)
            {
               newdir = Vector(trace.endpos);
               if(checkmask)
               {
                  contents = gi.pointcontents((newdir + centroid).vec3());
                  if(
                     (!allowcontentsmask || (contents & allowcontentsmask)) &&
                     (!disallowcontentsmask || !(contents & disallowcontentsmask)) &&
                     (trace.fraction > bestfraction) &&
                     (newdir != bestdir) &&
                     (newdir != previousdir)
                     )
                  {
                     bestdir = newdir;
                     bestfraction = trace.fraction;
                  }
               }
               else
               {
                  if(
                     (trace.fraction > bestfraction) &&
                     (newdir != bestdir) &&
                     (newdir != previousdir)
                     )
                  {
                     bestdir = newdir;
                     bestfraction = trace.fraction;
                  }
               }
            }
         }


         if((i == 4) || (i == 0))
         {
            break;
         }
      }
   }
   if(bestfraction > 0)
   {
      *time *= bestfraction;
   }
   else
   {
      *time = 0;
   }

   return bestdir;
}

/****************************************************************************

  Swim Class Definition

****************************************************************************/

CLASS_DECLARATION(Behavior, Swim, NULL);

ResponseDef Swim::Responses[] =
{
   { &EV_Behavior_Args,			(Response)&Swim::SetArgs },
   { NULL, NULL }
};

void Swim::SetArgs(Event *ev)
{
   if(ev->NumArgs() > 1)
      anim = ev->GetString(2);
}

void Swim::ShowInfo(Actor &self)
{
   Behavior::ShowInfo(self);

   gi.printf("\nseek:\n");
   seek.ShowInfo(self);

   gi.printf("\navoid:\n");
   avoid.ShowInfo(self);
}

void Swim::Begin(Actor &self)
{
   avoidtime = 0;
   avoidvec = vec_zero;

   avoid.Begin(self);
   seek.Begin(self);
   seek.SetTargetVelocity(vec_zero);
   if(anim.length())
   {
      self.SetAnim(anim);
   }
}

qboolean Swim::Evaluate(Actor &self)
{
   if((self.lastmove != STEPMOVE_OK) || (avoidtime <= level.time))
   {
      Vector dir;
      Vector ang;
      float time;

      time = 3;
      avoidvec = ChooseRandomDirection(self, avoidvec, &time, MASK_WATER, 0, true);
      avoidtime = level.time + time;
      dir = avoidvec - self.worldorigin;
      ang = dir.toAngles();
      ang[PITCH] = -ang[PITCH];
      self.setAngles(ang);
   }

   seek.SetMaxSpeed(self.movespeed);
   seek.SetPosition(self.worldorigin);
   seek.SetDir(self.movedir);
   seek.SetTargetPosition(avoidvec);
   // if we reached the goal, re-evaluate
   if(!seek.Evaluate(self))
   {
      avoidtime = 0;
   }

   avoid.SetMaxSpeed(self.movespeed * 2);
   avoid.SetPosition(self.worldorigin);
   avoid.SetDir(self.movedir);
   avoid.Evaluate(self);

   self.Accelerate(seek.steeringforce + avoid.steeringforce);

   return true;
}

void Swim::End(Actor &self)
{
   avoid.End(self);
   seek.End(self);
}

/****************************************************************************

  SwimCloseAttack Class Definition

****************************************************************************/


CLASS_DECLARATION(Behavior, SwimCloseAttack, NULL);

ResponseDef SwimCloseAttack::Responses[] =
{
   { &EV_Behavior_Args,			(Response)&SwimCloseAttack::SetArgs },
   { NULL, NULL }
};

void SwimCloseAttack::SetArgs(Event *ev)
{
   anim = ev->GetString(2);
}

void SwimCloseAttack::ShowInfo(Actor &self)
{
   Behavior::ShowInfo(self);

   gi.printf("\nseek:\n");
   seek.ShowInfo(self);

   gi.printf("\navoid:\n");
   avoid.ShowInfo(self);

   gi.printf("\navoidtime: %.2f\n", avoidtime);
}

void SwimCloseAttack::Begin(Actor &self)
{
   avoidtime = 0;
   avoiding = false;
   avoidvec = vec_zero;

   avoid.Begin(self);

   seek.Begin(self);
   seek.SetTargetVelocity(vec_zero);
   if(anim.length())
   {
      self.SetAnim(anim);
   }
}

qboolean SwimCloseAttack::Evaluate(Actor &self)
{
   trace_t trace;
   Vector dir;
   Vector ang;
   Vector goal;

   if(!self.currentEnemy)
   {
      return false;
   }

   //
   // close enough
   //
   if(self.CanShoot(self.currentEnemy, false))
   {
      if(self.WeaponReady())
         return false;
      else
         return true;
   }

   if((self.lastmove == STEPMOVE_STUCK) || (avoidtime <= level.time))
   {
      //
      // see if we can get to the player
      //
      dir = self.currentEnemy->worldorigin - self.worldorigin;
      dir.normalize();
      goal = self.currentEnemy->worldorigin;
      trace = G_Trace(self.worldorigin, self.mins, self.maxs, goal, &self, self.edict->clipmask, "SwimCloseAttack::Evaluate 2");
      if(trace.fraction < 0.5f)
      {
         float time;
         time = 2.5f;
         goal = ChooseRandomDirection(self, goal, &time, MASK_WATER, 0, true);
         avoidtime = level.time + time;
      }
      else
      {
         goal = trace.endpos;
         avoidtime = level.time + 2.5f;
      }
      dir = goal - self.worldorigin;
      dir.normalize();
      //self.movedir = dir;
      ang = dir.toAngles();
      ang[PITCH] = -ang[PITCH];
      self.setAngles(ang);
      seek.SetTargetPosition(goal);
   }

   seek.SetMaxSpeed(self.movespeed);
   seek.SetPosition(self.worldorigin);
   seek.SetDir(self.movedir);
   // if we reached the goal, re-evaluate
   if(!seek.Evaluate(self))
   {
      avoidtime = 0;
   }

   avoid.SetMaxSpeed(self.movespeed * 2);
   avoid.SetPosition(self.worldorigin);
   avoid.SetDir(self.movedir);
   avoid.Evaluate(self);

   self.Accelerate(seek.steeringforce + avoid.steeringforce);

   return true;
}

void SwimCloseAttack::End(Actor &self)
{
   seek.End(self);
}

/****************************************************************************

  Fly Class Definition

****************************************************************************/

CLASS_DECLARATION(Behavior, Fly, NULL);

ResponseDef Fly::Responses[] =
{
   { &EV_Behavior_Args,			(Response)&Fly::SetArgs },
   { NULL, NULL }
};

void Fly::SetArgs(Event *ev)
{
   if(ev->NumArgs() > 1)
      anim = ev->GetString(2);
}

void Fly::ShowInfo(Actor &self)
{
   Behavior::ShowInfo(self);

   gi.printf("\nseek:\n");
   seek.ShowInfo(self);

   gi.printf("\navoid:\n");
   avoid.ShowInfo(self);
}

void Fly::Begin(Actor &self)
{
   avoidtime = 0;
   avoidvec = vec_zero;

   avoid.Begin(self);
   seek.Begin(self);
   seek.SetTargetVelocity(vec_zero);
   if(anim.length())
   {
      self.SetAnim(anim);
   }
}


qboolean Fly::Evaluate(Actor &self)
{
   if((self.lastmove != STEPMOVE_OK) || (avoidtime <= level.time))
   {
      Vector dir;
      Vector ang;
      float time;

      time = 3;
      avoidvec = ChooseRandomDirection(self, avoidvec, &time, 0, MASK_WATER, true);
      avoidtime = level.time + time;
      dir = avoidvec - self.worldorigin;
      ang = dir.toAngles();
      ang[PITCH] = -ang[PITCH];
      self.setAngles(ang);
   }

   seek.SetMaxSpeed(self.movespeed);
   seek.SetPosition(self.worldorigin);
   seek.SetDir(self.movedir);
   seek.SetTargetPosition(avoidvec);
   // if we reached the goal, re-evaluate
   if(!seek.Evaluate(self))
   {
      avoidtime = 0;
   }

   avoid.SetMaxSpeed(self.movespeed * 2);
   avoid.SetPosition(self.worldorigin);
   avoid.SetDir(self.movedir);
   avoid.Evaluate(self);

   self.Accelerate(seek.steeringforce + avoid.steeringforce);

   return true;
}

void Fly::End(Actor &self)
{
   avoid.End(self);
   seek.End(self);
}

/****************************************************************************

  FlyCloseAttack Class Definition

****************************************************************************/


CLASS_DECLARATION(Behavior, FlyCloseAttack, NULL);

ResponseDef FlyCloseAttack::Responses[] =
{
   { &EV_Behavior_Args,			(Response)&FlyCloseAttack::SetArgs },
   { NULL, NULL }
};

void FlyCloseAttack::SetArgs(Event *ev)
{
   anim = ev->GetString(2);
}

void FlyCloseAttack::ShowInfo(Actor &self)
{
   Behavior::ShowInfo(self);

   gi.printf("\nseek:\n");
   seek.ShowInfo(self);

   gi.printf("\navoid:\n");
   avoid.ShowInfo(self);

   gi.printf("\navoidtime: %.2f\n", avoidtime);
}

void FlyCloseAttack::Begin(Actor &self)
{
   avoidtime = 0;
   avoiding = false;
   avoidvec = vec_zero;

   avoid.Begin(self);

   seek.Begin(self);
   seek.SetTargetVelocity(vec_zero);
   if(anim.length())
   {
      self.SetAnim(anim);
   }
}

qboolean FlyCloseAttack::Evaluate(Actor &self)
{
   trace_t trace;
   Vector dir;
   Vector ang;
   float minrange;
   Vector goal;
   Sentient *sent;

   if(!self.currentEnemy)
   {
      return false;
   }

   if(!self.CurrentWeapon() && self.currentEnemy->isSubclassOf<Sentient>())
   {
      sent = (Sentient *)(Entity *)self.currentEnemy;
      if(sent->waterlevel > 1)
      {
         // can no longer get to enemy
         self.ClearEnemies();
         return false;
      }
   }

   minrange = self.MinimumAttackRange();
   //
   // close enough
   //
   if(self.CanShoot(self.currentEnemy, false))
   {
      if(self.WeaponReady())
         return false;
      else
         return true;
   }

   if((self.lastmove == STEPMOVE_STUCK) || (avoidtime <= level.time))
   {
      //
      // see if we can get to the player
      //
      goal = self.currentEnemy->centroid;
      trace = G_Trace(self.worldorigin, self.mins, self.maxs, goal, &self, self.edict->clipmask, "FlyCloseAttack::Evaluate 2");
      if(trace.fraction < 0.5f)
      {
         float time;
         time = 2.5f;
         goal = ChooseRandomDirection(self, goal, &time, 0, MASK_WATER, true);
         avoidtime = level.time + time;
      }
      else
      {
         goal = trace.endpos;
         avoidtime = level.time + 2.5f;
      }
      dir = goal - self.worldorigin;
      dir.normalize();
      //self.movedir = dir;
      ang = dir.toAngles();
      ang[PITCH] = -ang[PITCH];
      self.setAngles(ang);
      seek.SetTargetPosition(goal);
   }

   seek.SetMaxSpeed(self.movespeed);
   seek.SetPosition(self.worldorigin);
   seek.SetDir(self.movedir);
   // if we reached the goal, re-evaluate
   if(!seek.Evaluate(self))
   {
      avoidtime = 0;
   }

   avoid.SetMaxSpeed(self.movespeed * 2);
   avoid.SetPosition(self.worldorigin);
   avoid.SetDir(self.movedir);
   avoid.Evaluate(self);

   self.Accelerate(seek.steeringforce + avoid.steeringforce);

   return true;
}

void FlyCloseAttack::End(Actor &self)
{
   seek.End(self);
}

/****************************************************************************

  Wander Class Definition

****************************************************************************/

CLASS_DECLARATION(Behavior, Wander, NULL);

ResponseDef Wander::Responses[] =
{
   { &EV_Behavior_Args,			(Response)&Wander::SetArgs },
   { NULL, NULL }
};

void Wander::SetArgs(Event *ev)
{
   if(ev->NumArgs() > 1)
      anim = ev->GetString(2);
}

void Wander::ShowInfo(Actor &self)
{
   Behavior::ShowInfo(self);

   gi.printf("\nseek:\n");
   seek.ShowInfo(self);

   gi.printf("\navoid:\n");
   avoid.ShowInfo(self);
}

void Wander::Begin(Actor &self)
{
   avoidtime = 0;
   avoidvec = vec_zero;

   avoid.Begin(self);
   seek.Begin(self);
   seek.SetTargetVelocity(vec_zero);
   if(anim.length())
   {
      self.SetAnim(anim);
   }
}

qboolean Wander::Evaluate(Actor &self)
{
   if((self.lastmove != STEPMOVE_OK) || (avoidtime <= level.time))
   {
      Vector dir;
      Vector ang;
      float time;

      time = 5;
      self.Chatter("snd_idle", 4);
      avoidvec = ChooseRandomDirection(self, avoidvec, &time, 0, 0, false);
      avoidtime = level.time + time;
      dir = avoidvec - self.worldorigin;
      ang = dir.toAngles();
      self.angles[YAW] = ang[YAW];
      self.setAngles(self.angles);
   }

   seek.SetMaxSpeed(self.movespeed);
   seek.SetPosition(self.worldorigin);
   seek.SetDir(self.movedir);
   seek.SetTargetPosition(avoidvec);
   // if we reached the goal, re-evaluate
   if(!seek.Evaluate(self))
   {
      avoidtime = 0;
   }

   avoid.SetMaxSpeed(self.movespeed * 2);
   avoid.SetPosition(self.worldorigin);
   avoid.SetDir(self.movedir);
   avoid.Evaluate(self);

   self.Accelerate(seek.steeringforce + avoid.steeringforce);

   return true;
}

void Wander::End(Actor &self)
{
   avoid.End(self);
   seek.End(self);
}

/****************************************************************************

  WanderCloseAttack Class Definition

****************************************************************************/

CLASS_DECLARATION(Behavior, WanderCloseAttack, NULL);

ResponseDef WanderCloseAttack::Responses[] =
{
   { &EV_Behavior_Args,			(Response)&WanderCloseAttack::SetArgs },
   { NULL, NULL }
};

void WanderCloseAttack::SetArgs(Event *ev)
{
   anim = ev->GetString(2);
}

void WanderCloseAttack::ShowInfo(Actor &self)
{
   Behavior::ShowInfo(self);

   gi.printf("\nseek:\n");
   seek.ShowInfo(self);

   gi.printf("\navoid:\n");
   avoid.ShowInfo(self);

   gi.printf("\navoidtime: %.2f\n", avoidtime);
}

void WanderCloseAttack::Begin(Actor &self)
{
   avoidtime = 0;
   avoiding = false;
   avoidvec = vec_zero;

   avoid.Begin(self);

   seek.Begin(self);
   seek.SetTargetVelocity(vec_zero);
   if(anim.length())
   {
      self.SetAnim(anim);
   }
}

qboolean WanderCloseAttack::Evaluate(Actor &self)
{
   trace_t trace;
   Vector dir;
   Vector ang;
   float minrange;
   Vector goal;

   if(!self.currentEnemy)
   {
      return false;
   }

   minrange = self.MinimumAttackRange();
   //
   // close enough
   //
   if(self.CanShoot(self.currentEnemy, false))
   {
      if(self.WeaponReady())
         return false;
      else
         return true;
   }

   if((self.lastmove == STEPMOVE_STUCK) || (avoidtime <= level.time))
   {
      //
      // see if we can get to the player
      //
      dir = self.currentEnemy->worldorigin - self.worldorigin;
      dir.normalize();
      goal = self.currentEnemy->worldorigin;
      trace = G_Trace(self.worldorigin, self.mins, self.maxs, goal, &self, self.edict->clipmask, "WanderCloseAttack::Evaluate 2");
      if(trace.fraction < 0.5f)
      {
         float time;
         time = 2.5f;
         goal = ChooseRandomDirection(self, goal, &time, 0, 0, false);
         avoidtime = level.time + time;
      }
      else
      {
         goal = trace.endpos;
         avoidtime = level.time + 2.5f;
      }
     
      dir = goal - self.worldorigin;

      //### The following altered to fix funky face-down behaviour
      // It would fail when ChooseRandomDirection returned self.worldorigin
      if(dir != vec_zero)
      {
         dir.normalize();
         //self.movedir = dir;
         ang = dir.toAngles();
         ang[PITCH] = -ang[PITCH];
         self.setAngles(ang);
         seek.SetTargetPosition(goal);
      }
      //### that's all
   }

   seek.SetMaxSpeed(self.movespeed);
   seek.SetPosition(self.worldorigin);
   seek.SetDir(self.movedir);
   // if we reached the goal, re-evaluate
   if(!seek.Evaluate(self))
   {
      avoidtime = 0;
   }

   avoid.SetMaxSpeed(self.movespeed * 2);
   avoid.SetPosition(self.worldorigin);
   avoid.SetDir(self.movedir);
   avoid.Evaluate(self);

   self.Accelerate(seek.steeringforce + avoid.steeringforce);

   return true;
}

void WanderCloseAttack::End(Actor &self)
{
   seek.End(self);
}

/****************************************************************************

  GetCloseToEnemy Class Definition

****************************************************************************/

CLASS_DECLARATION(Behavior, GetCloseToEnemy, NULL);

ResponseDef GetCloseToEnemy::Responses[] =
{
   { &EV_Behavior_Args,			(Response)&GetCloseToEnemy::SetArgs },
   { NULL, NULL }
};

void GetCloseToEnemy::SetArgs(Event *ev)
{
   int num;

   num = 2;
   if(!ev->IsNumericAt(num))
   {
      anim = ev->GetString(num);
      num++;
   }

   howclose = 32;
   if(ev->IsNumericAt(num))
   {
      howclose = ev->GetFloat(num);
      num++;
   }
}

void GetCloseToEnemy::ShowInfo(Actor &self)
{
   Behavior::ShowInfo(self);

   gi.printf("\nchase:\n");
   chase.ShowInfo(self);

   gi.printf("\nstate: %d\n", state);
   gi.printf("howclose: %f\n", howclose);
   gi.printf("nextsearch: %f\n", nextsearch);
   gi.printf("anim: %s\n", anim.c_str());
}

void GetCloseToEnemy::Begin(Actor &self)
{
   if(!anim.length())
   {
      anim = "run";
   }

   movegoal = NULL;
   state = 0;
}

qboolean GetCloseToEnemy::Evaluate(Actor &self)
{
   if(!self.currentEnemy)
   {
      return false;
   }

   if(nextsearch < level.time)
   {
      state = 0;
   }

   switch(state)
   {
   case 0:
      chase.Begin(self);
      movegoal = PathManager.NearestNode(self.currentEnemy->worldorigin, &self, false);
      if(movegoal)
      {
         Path *path;
         FindEnemyPath find;

         find.heuristic.self = &self;
         find.heuristic.setSize(self.size);
         find.heuristic.entnum = self.entnum;

         path = find.FindPath(self.worldorigin, movegoal->worldorigin);
         movegoal = NULL;
         if(path)
         {
            float dist;
            float movedist;

            //
            // check range 
            // if distance is less then total_delta, than lets go straight for him
            //
            dist = path->DistanceAlongPath(self.worldorigin);
            movedist = self.total_delta.length();
            if(dist >= movedist)
            {
               movegoal = path->End();

               chase.SetGoal(movegoal);
               chase.SetPath(path);
            }
         }
      }

      if(!movegoal)
      {
         if(self.CanSee(self.currentEnemy))
         {
            chase.SetGoalPos(self.currentEnemy->worldorigin);
         }
         else
         {
            //
            // since we can't reach em
            // clear out enemy state
            //
            self.ClearEnemies();
            return false;
         }
      }

      if(anim.length() && (anim != self.animname))
      {
         self.SetAnim(anim);
      }

      state = 1;
      nextsearch = level.time + 3;

   case 1:
      if(self.WithinDistance(self.currentEnemy, howclose))
      {
         chase.End(self);
         return false;
      }

      if(!chase.Evaluate(self) || (nextsearch < level.time))
      {
         state = 0;
      }
      break;
   }

   return true;
}

void GetCloseToEnemy::End(Actor &self)
{
   chase.End(self);
}

/****************************************************************************

  PlayAnimSeekEnemy Class Definition

****************************************************************************/

CLASS_DECLARATION(Behavior, PlayAnimSeekEnemy, NULL);

ResponseDef PlayAnimSeekEnemy::Responses[] =
{
   { &EV_Behavior_Args,			(Response)&PlayAnimSeekEnemy::SetArgs },
   { &EV_Behavior_AnimDone,	(Response)&PlayAnimSeekEnemy::AnimDone },
   { NULL, NULL }
};

void PlayAnimSeekEnemy::ShowInfo(Actor &self)
{
   Behavior::ShowInfo(self);

   gi.printf("\naim:\n");
   aim.ShowInfo(self);

   gi.printf("\nmode: %d\n", mode);
   gi.printf("anim: %s\n", anim.c_str());
   gi.printf("animdone: %d\n", animdone);
}

void PlayAnimSeekEnemy::Begin(Actor &self)
{
   oldanim = self.animname;
   mode = 0;
   animdone = false;
}

void PlayAnimSeekEnemy::SetArgs(Event *ev)
{
   anim = ev->GetString(2);
}

void PlayAnimSeekEnemy::AnimDone(Event *ev)
{
   animdone = true;
}

qboolean PlayAnimSeekEnemy::Evaluate(Actor &self)
{
   switch(mode)
   {
   case 0:
      if(!anim.length())
      {
      }

      if(!self.currentEnemy)
      {
         return false;
      }

      animdone = false;
      self.SetAnim(anim.c_str(), EV_Actor_NotifyBehavior);

      mode = 1;

   case 1:
      aim.SetTarget(self.currentEnemy);
      aim.Evaluate(self);

      // finish up the attack
      if(animdone)
      {
         return false;
      }
      break;
   }

   return true;
}

void PlayAnimSeekEnemy::End(Actor &self)
{
   aim.End(self);
   if(oldanim.length())
   {
      self.SetAnim(oldanim.c_str());
   }
}

//### 2015 added behaiviors

/****************************************************************************

GetCloseToObject Class Definition

Used to get reasonably close to an object before throwing it.
Has smoother ans less regid results than using GotoPathNode.

****************************************************************************/

CLASS_DECLARATION(Behavior, GetCloseToObject, NULL);

ResponseDef GetCloseToObject::Responses[] =
{
   { &EV_Behavior_Args, (Response)&GetCloseToObject::SetArgs },
   { NULL, NULL }
};

GetCloseToObject::GetCloseToObject() : Behavior()
{
   howclose = 32;
}

void GetCloseToObject::SetArgs(Event *ev)
{
   int num;

   num = 2;

   targetobject = ev->GetEntity(num);
   num++;

   howclose = 32;
   if(ev->IsNumericAt(num))
   {
      howclose = ev->GetFloat(num);
      num++;
   }

   if(!ev->IsNumericAt(num))
   {
      anim = ev->GetString(num);
      num++;
   }
}

void GetCloseToObject::ShowInfo(Actor &self)
{
   Behavior::ShowInfo(self);

   gi.printf("\nchase:\n");
   chase.ShowInfo(self);

   gi.printf("\nstate: %d\n",    state);
   gi.printf("howclose: %f\n",   howclose);
   gi.printf("nextsearch: %f\n", nextsearch);
   gi.printf("anim: %s\n",       anim.c_str());
   if(targetobject)
   {
      gi.printf("\ntargetobject: #%d '%s'\n", targetobject->entnum, targetobject->targetname.c_str());
   }
   else
   {
      gi.printf("\ntargetobject: NULL\n");
   }
}

void GetCloseToObject::Begin(Actor &self)
{
   if(!anim.length())
   {
      anim = "run";
   }

   movegoal = NULL;
   state = 0;
}

qboolean GetCloseToObject::Evaluate(Actor &self)
{
   //	if ( !self.currentEnemy )
   if(!targetobject)
   {
      return false;
   }

   if(nextsearch < level.time)
   {
      state = 0;
   }

   switch(state)
   {
   case 0:
      chase.Begin(self);
      movegoal = PathManager.NearestNode(targetobject->worldorigin, &self, false);
      if(movegoal)
      {
         Path *path;
         FindEnemyPath find;

         find.heuristic.self = &self;
         find.heuristic.setSize(self.size);
         find.heuristic.entnum = self.entnum;

         path = find.FindPath(self.worldorigin, movegoal->worldorigin);
         movegoal = NULL;
         if(path)
         {
            float dist;
            float movedist;

            //
            // check range 
            // if distance is less then total_delta, than lets go straight for him
            //
            dist = path->DistanceAlongPath(self.worldorigin);
            movedist = self.total_delta.length();
            if(dist >= movedist)
            {
               movegoal = path->End();

               chase.SetGoal(movegoal);
               chase.SetPath(path);
            }
         }
      }

      if(!movegoal)
      {
         //if ( self.CanSee( targetobject ) )
         //{
         chase.SetGoalPos(targetobject->worldorigin);
         //}
         //else
         //{
         //
         // we can't reach it
         //
         //   return FALSE;
         //}
      }

      if(anim.length() && (anim != self.animname))
      {
         self.SetAnim(anim.c_str());
      }

      state = 1;
      nextsearch = level.time + 3;

   case 1 :
      if(self.WithinDistance(targetobject, howclose))
      {
         chase.End(self);
         return false;
      }

      if(!chase.Evaluate(self) || (nextsearch < level.time))
      {
         state = 0;
      }
      break;
   }

   return true;
}

void GetCloseToObject::End(Actor &self)
{
   chase.End(self);
   self.SetAnim("idle");
}

//###

// EOF

