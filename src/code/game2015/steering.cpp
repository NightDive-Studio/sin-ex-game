//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/steering.cpp                     $
// $Revision:: 21                                                             $
//   $Author:: Markd                                                          $
//     $Date:: 11/18/98 8:53p                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Steering behaviors for AI.
// 

#include "g_local.h"
#include "steering.h"
#include "actor.h"

/****************************************************************************

  Steering Class Definition

****************************************************************************/

CLASS_DECLARATION(Listener, Steering, nullptr);

ResponseDef Steering::Responses[] =
{
   { nullptr, nullptr }
};

void Steering::ShowInfo(Actor &self)
{
   gi.printf("steeringforce: ( %f, %f, %f )\n", steeringforce.x, steeringforce.y, steeringforce.z);
   gi.printf("origin: ( %f, %f, %f )\n", origin.x, origin.y, origin.z);
   gi.printf("movedir: ( %f, %f, %f )\n", movedir.x, movedir.y, movedir.z);
   gi.printf("maxspeed: %f\n", maxspeed);
}

void Steering::Begin(Actor &self)
{
}

qboolean Steering::Evaluate(Actor &self)
{
   return false;
}

void Steering::End(Actor &self)
{
}

void Steering::DrawForces()
{
   G_Color3f(0.3, 0.5, 1);
   G_BeginLine();
   G_Vertex(origin);
   G_Vertex(origin + steeringforce * FRAMETIME);
   G_EndLine();

   G_Color3f(1, 0, 1);
   G_BeginLine();
   G_Vertex(origin);
   G_Vertex(origin + movedir * maxspeed * FRAMETIME);
   G_EndLine();
}

void Steering::SetPosition(Vector pos)
{
   origin = pos;
}

void Steering::SetDir(Vector dir)
{
   movedir = dir;
}

void Steering::SetMaxSpeed(float speed)
{
   maxspeed = speed;
}

void Steering::ResetForces(void)
{
   steeringforce = vec_zero;
}

/****************************************************************************

  Seek Class Definition

****************************************************************************/

CLASS_DECLARATION(Steering, Seek, nullptr);

ResponseDef Seek::Responses[] =
{
   { nullptr, nullptr }
};

void Seek::SetTargetPosition(Vector pos)
{
   targetposition = pos;
}

void Seek::SetTargetVelocity(Vector vel)
{
   targetvelocity = vel;
}

void Seek::ShowInfo(Actor &self)
{
   Steering::ShowInfo(self);

   gi.printf("\ntargetposition: ( %f, %f, %f )\n", targetposition.x, targetposition.y, targetposition.z);
   gi.printf("targetvelocity: ( %f, %f, %f )\n", targetvelocity.x, targetvelocity.y, targetvelocity.z);
}

qboolean Seek::Evaluate(Actor &self)
{
   Vector	predictedposition;
   Vector	dir;
   Vector	delta;
   Vector	ang1;
   Vector	ang2;
   float		dist;
   float    xydist;
   float		l;

   ResetForces();

   delta = targetposition - origin;
   dist = delta.length();
   //
   // null out z component
   //
   delta.z = 0;
   xydist = delta.length();

   if(maxspeed != 0.0f)
      predictedposition = targetposition + (targetvelocity * (dist / maxspeed));
   else
      predictedposition = targetposition;

   dir = predictedposition - origin;
   dir.normalize();

   ang1 = dir.toAngles();
   ang2 = movedir.toAngles();

   steeringforce.x = ang1.x - ang2.x;
   if(steeringforce.x <= -180)
   {
      steeringforce.x += 360;
   }
   if(steeringforce.x >= 180)
   {
      steeringforce.x -= 360;
   }

   steeringforce.y = ang1.y - ang2.y;
   if(steeringforce.y <= -180)
   {
      steeringforce.y += 360;
   }
   if(steeringforce.y >= 180)
   {
      steeringforce.y -= 360;
   }

   // if we're nearly there, turn directly toward our goal
   if(xydist > self.movespeed)
   {
      if(fabs(steeringforce.x) > 1)
      {
         steeringforce.x *= 0.4;
      }

      if(fabs(steeringforce.y) > 1)
      {
         steeringforce.y *= 0.4;
      }
   }
   else
   {
      l = self.total_delta.length();
      if(xydist <= l)
      {
         //steeringforce = vec_zero;
         self.total_delta = self.animdir * xydist;
         return false;
      }
   }

   steeringforce.z = 0;

   return true;
}

/****************************************************************************

  ObstacleAvoidance Class Definition

****************************************************************************/

CLASS_DECLARATION(Steering, ObstacleAvoidance, nullptr);

ResponseDef ObstacleAvoidance::Responses[] =
{
   { nullptr, nullptr }
};

void ObstacleAvoidance::AvoidWalls(qboolean avoid)
{
   avoidwalls = avoid;
}

void ObstacleAvoidance::ShowInfo(Actor &self)
{
   Steering::ShowInfo(self);

   gi.printf("\navoidwalls: %d\n", avoidwalls);
}

qboolean ObstacleAvoidance::Evaluate(Actor &self)
{
   Vector	predictedposition;
   Vector	normal;
   Vector	angles;
   Vector	right;
   float		urgency;
   float		dot;
   trace_t	tracef;
#if 0
   trace_t	tracel;
   trace_t	tracer;
   Vector	leftposition;
   Vector	rightposition;
#endif
   Entity	*ent;

   ResetForces();

   angles = self.movedir.toAngles();
   angles.AngleVectors(NULL, &right, NULL);

   origin = self.worldorigin;
   origin.z += 1;
   predictedposition = origin + self.movedir * self.movespeed;//maxspeed;
#if 0
   leftposition = origin - right * 8;
   rightposition = origin + right * 8;
#endif

#if 0
   G_Color3f(1, 1, 1);
   G_BeginLine();
   G_Vertex(origin);
   G_Vertex(predictedposition);
   G_Vertex(origin);
   G_Vertex(leftposition);
   G_Vertex(origin);
   G_Vertex(rightposition);
   G_EndLine();
#endif

   tracef = G_Trace(origin, self.mins, self.maxs, predictedposition, &self, self.edict->clipmask, "ObstacleAvoidance forward");
#if 0
   tracel = G_Trace(origin, self.mins, self.maxs, leftposition, &self, MASK_PLAYERSOLID, "ObstacleAvoidance left");
   tracer = G_Trace(origin, self.mins, self.maxs, rightposition, &self, MASK_PLAYERSOLID, "ObstacleAvoidance right");
   if(tracel.fraction < 1)
   {
      urgency = 1.1 - tracel.fraction;
      normal = tracel.plane.normal;
      ent = tracel.ent->entity;
      steeringforce = Vector(0, -90, 0);;
   }
   else if(tracer.fraction < 1)
   {
      urgency = 1.1 - tracer.fraction;
      normal = tracer.plane.normal;
      ent = tracer.ent->entity;
      steeringforce = Vector(0, 90, 0);;
   }
   else
#endif
   if(tracef.fraction < 1)
   {
      urgency = 1.0 - tracef.fraction;
      normal = tracef.plane.normal;
      ent = tracef.ent->entity;
      if(ent->getSolidType() != SOLID_BSP)
      {
         dot = -(right * (ent->worldorigin - self.worldorigin));
      }
      else
      {
         if(!avoidwalls)
         {
            return true;
         }

         dot = right * normal;
      }

      if(dot < 0)
      {
         // turn left
         steeringforce = Vector(0, 90, 0);;
      }
      else
      {
         // turn right
         steeringforce = Vector(0, -90, 0);;
      }
   }
   else
   {
      return true;
   }

   steeringforce *= urgency;

   return true;
}

/****************************************************************************

  ObstacleAvoidance2 Class Definition

****************************************************************************/

CLASS_DECLARATION(Steering, ObstacleAvoidance2, nullptr);

ResponseDef ObstacleAvoidance2::Responses[] =
{
   { nullptr, nullptr }
};

void ObstacleAvoidance2::AvoidWalls(qboolean avoid)
{
   avoidwalls = avoid;
}

void ObstacleAvoidance2::ShowInfo(Actor &self)
{
   Steering::ShowInfo(self);

   gi.printf("\navoidwalls: %d\n", avoidwalls);
}

qboolean ObstacleAvoidance2::Evaluate(Actor &self)
{
   Vector	predictedposition;
   Vector	normal;
   Vector	angles;
   Vector	right;
   float		urgency;
   float		dot;
   trace_t	tracef;
#if 0
   trace_t	tracel;
   trace_t	tracer;
   Vector	leftposition;
   Vector	rightposition;
#endif
   Entity	*ent;

   ResetForces();

   angles = self.movedir.toAngles();
   angles.AngleVectors(NULL, &right, NULL);

   origin = self.worldorigin;
   origin.z += 1;
   predictedposition = origin + self.movedir * self.movespeed;//maxspeed;
#if 0
   leftposition = origin - right * 8;
   rightposition = origin + right * 8;
#endif

#if 0
   G_Color3f(1, 1, 1);
   G_BeginLine();
   G_Vertex(origin);
   G_Vertex(predictedposition);
   G_Vertex(origin);
   G_Vertex(leftposition);
   G_Vertex(origin);
   G_Vertex(rightposition);
   G_EndLine();
#endif

   tracef = G_Trace(origin, self.mins, self.maxs, predictedposition, &self, self.edict->clipmask, "ObstacleAvoidance2 forward");
#if 0
   tracel = G_Trace(origin, self.mins, self.maxs, leftposition, &self, MASK_PLAYERSOLID, "ObstacleAvoidance2 left");
   tracer = G_Trace(origin, self.mins, self.maxs, rightposition, &self, MASK_PLAYERSOLID, "ObstacleAvoidance2 right");
   if(tracel.fraction < 1)
   {
      urgency = 1.1 - tracel.fraction;
      normal = tracel.plane.normal;
      ent = tracel.ent->entity;
      steeringforce = Vector(0, -90, 0);;
   }
   else if(tracer.fraction < 1)
   {
      urgency = 1.1 - tracer.fraction;
      normal = tracer.plane.normal;
      ent = tracer.ent->entity;
      steeringforce = Vector(0, 90, 0);;
   }
   else
#endif
   if(tracef.fraction < 1)
   {
      urgency = 1.0 - tracef.fraction;
      normal = tracef.plane.normal;
      ent = tracef.ent->entity;
      if(ent->getSolidType() != SOLID_BSP)
      {
         dot = -(right * (ent->worldorigin - self.worldorigin));
      }
      else
      {
         if(!avoidwalls)
         {
            return true;
         }

         dot = right * normal;
      }

      if(dot < 0)
      {
         // turn left
         steeringforce = Vector(0, 22, 0) * urgency;
      }
      else
      {
         // turn right
         steeringforce = Vector(0, 22, 0) * urgency;
      }
   }
   else
   {
      return true;
   }

   steeringforce *= urgency;

   return true;
}

/****************************************************************************

  FollowPath Class Definition

****************************************************************************/

CLASS_DECLARATION(Steering, FollowPath, nullptr);

ResponseDef FollowPath::Responses[] =
{
   { nullptr, nullptr }
};

FollowPath::~FollowPath()
{
   currentNode = nullptr;
   if(path)
   {
      delete path;
   }
}

void FollowPath::FindCurrentNode(Actor &self)
{
   // Sometimes the second node on the path is the proper node to start from.
   // This happens because instead of creating the shortest path from the actor,
   // we create the shortest path from his nearest node.  Often, this creates a
   // path where he may already be further along the path than the first node,
   // causing him to "go back" along the path.  By checking if we can get to the
   // second node, we get rid of the backtracking.
   PathNode *node;

   if(!path)
   {
      currentNode = nullptr;
      return;
   }

   currentNode = path->NextNode();
   if(path->NumNodes() < 2)
   {
      return;
   }

   node = path->GetNode(2);
   if(self.CanMoveTo(node->worldorigin))
   {
      currentNode = path->NextNode();
   }
}

void FollowPath::SetPath(Path *newpath)
{
   if(path)
   {
      delete path;
   }

   currentNode = nullptr;

   path = newpath;
}

Path *FollowPath::SetPath(Actor &self, Vector from, Vector to)
{
   PathNode *goal;
   PathNode *node;
   StandardMovePath find;

   if(path)
   {
      delete path;
      path = nullptr;
   }

   currentNode = nullptr;

   goal = PathManager.NearestNode(to, &self);
   if(!goal)
   {
      return nullptr;
   }

   node = PathManager.NearestNode(from, &self);
   if(!node || (goal == node))
   {
      return nullptr;
   }

   find.heuristic.setSize(self.size);
   find.heuristic.entnum = self.entnum;

   path = find.FindPath(node, goal);

   return path;
}

void FollowPath::DrawForces()
{
   seek.DrawForces();
}

qboolean FollowPath::DoneWithPath(Actor &self)
{
   if(!path)
   {
      return true;
   }

   return (currentNode == nullptr);
}

void FollowPath::ShowInfo(Actor &self)
{
   Steering::ShowInfo(self);

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

   gi.printf("seek:\n");
   seek.ShowInfo(self);

   if(currentNode)
   {
      gi.printf("currentNode: ( %f, %f, %f )\n",
                currentNode->worldorigin.x, currentNode->worldorigin.y, currentNode->worldorigin.z);
   }
   else
   {
      gi.printf("currentNode: NULL\n");
   }
}

void FollowPath::Begin(Actor &self)
{
   seek.Begin(self);
}

qboolean FollowPath::Evaluate(Actor &self)
{
   PathNode *lastnode;
   Vector delta;
   Vector targetpos;

   ResetForces();

   if(!path)
   {
      return false;
   }

   // the first time we come through here with a path, currentNode is NULL.
   if(!currentNode)
   {
      FindCurrentNode(self);
      if(!currentNode)
      {
         delete path;
         path = nullptr;
         return false;
      }
   }

   targetpos = currentNode->worldorigin;

   // check if the remaining distance is less than the
   // distance we'll travel this frame.
   delta = targetpos - self.worldorigin;

   // check if the squared distance remaining is less than 
   // the squared distance we'll travel
   if(delta * delta <= self.frame_delta * self.frame_delta)
   {
      lastnode = currentNode;
      currentNode = path->NextNode();

      // check if we should jump to our next node
      if(currentNode && (lastnode->nodeflags & AI_JUMP) && (currentNode->targetname == lastnode->target))
      {
         if(self.last_jump_time < level.time)
         {
            self.SetVariable("jumptarget", lastnode->target.c_str());
            self.ForceAction("jump");
         }

         return true;
      }

      // if we're not done with the path, steer toward the next node
      if(currentNode)
      {
         targetpos = currentNode->worldorigin;
      }
      else
      {
         delete path;
         path = nullptr;

         return false;
      }
   }

   // steer toward our next path node
   seek.SetTargetPosition(targetpos);
   seek.SetTargetVelocity(vec_zero);
   seek.SetMaxSpeed(self.movespeed);
   seek.SetPosition(origin);
   seek.SetDir(self.movedir);
   seek.Evaluate(self);

   steeringforce = seek.steeringforce;

   return (currentNode != nullptr);
}

void FollowPath::End(Actor &self)
{
   seek.End(self);
}

/****************************************************************************

  Turn Class Definition

****************************************************************************/

CLASS_DECLARATION(Steering, Turn, nullptr);

ResponseDef Turn::Responses[] =
{
   { nullptr, nullptr }
};

void Turn::SetDirection(float yaw)
{
   Vector ang;

   ang = Vector(0, yaw, 0);
   this->yaw = anglemod(yaw);
   ang.AngleVectors(&dir, nullptr, nullptr);
   mode = 1;
}

void Turn::SetTarget(Entity *ent)
{
   this->ent = ent;
   mode = 2;
}

void Turn::ShowInfo(Actor &self)
{
   Steering::ShowInfo(self);

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
}

void Turn::Begin(Actor &self)
{
   seek.Begin(self);
}

extern float angledist(float ang);

qboolean Turn::Evaluate(Actor &self)
{
   Vector delta;
   float ang;

   switch(mode)
   {
   case 1:
      ang = angledist(yaw - self.angles.yaw());
      if(fabs(ang) < 1)
      {
         steeringforce = Vector(0, ang, 0);
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

   steeringforce = seek.steeringforce;

   return true;
}

void Turn::End(Actor &self)
{
   seek.End(self);
}

/****************************************************************************

  Chase Class Definition

****************************************************************************/

CLASS_DECLARATION(Steering, Chase, nullptr);

ResponseDef Chase::Responses[] =
{
   { nullptr, nullptr }
};

void Chase::SetPath(Path *newpath)
{
   follow.SetPath(newpath);
   path = newpath;
}

void Chase::SetGoalPos(Vector goalpos)
{
   goal = goalpos;
   usegoal = true;
   goalent = nullptr;
   goalnode = nullptr;
}

void Chase::SetGoal(PathNode *node)
{
   goalnode = node;
   usegoal = false;
   goalent = nullptr;
}

void Chase::SetTarget(Entity *ent)
{
   goalent = ent;
   goalnode = nullptr;
   usegoal = false;
}

void Chase::SetPathRate(float rate)
{
   newpathrate = rate;
}

void Chase::ShowInfo(Actor &self)
{
   Steering::ShowInfo(self);

   gi.printf("\nseek:\n");
   seek.ShowInfo(self);

   gi.printf("\nfollow:\n");
   follow.ShowInfo(self);

   gi.printf("\nnextpathtime: %f\n", nextpathtime);

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

   gi.printf("goal: ( %f, %f, %f )\n", goal.x, goal.y, goal.z);

   if(goalent)
   {
      gi.printf("\ngoalent: #%d '%s'\n", goalent->entnum, goalent->targetname.c_str());
   }
   else
   {
      gi.printf("\ngoalent: NULL\n");
   }

   if(goalnode)
   {
      gi.printf("\ngoalnode: #%d '%s' ( %f, %f, %f )\n", goalnode->nodenum, goalnode->targetname.c_str(),
                goalnode->worldorigin.x, goalnode->worldorigin.y, goalnode->worldorigin.z);
   }
   else
   {
      gi.printf("\ngoalnode: NULL\n");
   }

   gi.printf("avoid:\n");
   avoid.ShowInfo(self);

   gi.printf("\ntime: %f\n", avoidtime);

   gi.printf("usegoal: %d\n", usegoal);
   gi.printf("newpathrate: %f\n", newpathrate);
   gi.printf("wander: %d\n", wander);
   gi.printf("stuck: %d\n", stuck);
   gi.printf("avoidvec : ( %f, %f, %f )\n", avoidvec.x, avoidvec.y, avoidvec.z);
}

void Chase::Begin(Actor &self)
{
   nextpathtime = 0;
   path = nullptr;
   seek.Begin(self);
   follow.Begin(self);
   avoid.AvoidWalls(false);
   avoid.Begin(self);
   turnto.Begin(self);
   anim = self.animname;
   stuck = 0;
   wander = 0;
}

Vector Chase::ChooseRandomDirection(Actor &self)
{
   Vector dir;
   Vector ang;
   Vector bestdir;
   float bestfraction;
   trace_t trace;
   trace_t groundtrace;
   int i;
   int j;
   int t;
   int u;
   Vector s;
   Vector start;
   Vector end;
   Vector groundend;

   s = Vector(0, 0, STEPSIZE);
   start = self.worldorigin + s;

   // quantize to nearest 45 degree
   u = ((int)(self.worldangles.y * (1 / 45) + 22.5)) * 45;
   bestfraction = -1;
   //
   // in case we don't find anything!
   //
   bestdir = self.worldorigin - (Vector(self.orientation[0]) * 100);

   for(i = 0; i <= 180; i += 20)
   {
      if(rand() < 0.3)
      {
         i += 20;
      }
      t = i;
      if(rand() < 0.5)
      {
         // sometimes we choose left first, other times right.
         t = -t;
      }
      for(j = -1; j < 2; j += 2)
      {
         if((j == 1) && (i == 180))
         {
            ang.y = self.worldangles.y + (t * j);
         }
         else
         {
            ang.y = u + t * j;
         }

         ang.AngleVectors(&dir, nullptr, nullptr);

         end = self.worldorigin + dir * 140 + s;
         trace = G_Trace(start, self.mins, self.maxs, end, &self,
                         self.edict->clipmask, "Chase::ChooseRandomDirection 1");
         if((trace.fraction > bestfraction) && (!trace.startsolid) && !(trace.allsolid))
         {
            if(trace.endpos != avoidvec)
            {
               // check if we're near the ground 
               end = self.worldorigin + dir * 32 + s;
               groundend = end;
               groundend.z -= STEPSIZE * 2;
               groundtrace = G_Trace(end, self.mins, self.maxs, groundend, &self,
                                     self.edict->clipmask, "Chase::ChooseRandomDirection 2");
               if(groundtrace.fraction != 1)
               {
                  bestdir = trace.endpos;
                  bestfraction = trace.fraction;
               }
            }
         }

         if(i == 0)
         {
            break;
         }
      }
   }

   return bestdir;
}

qboolean Chase::Evaluate(Actor &self)
{
   qboolean result;
   trace_t trace;

   if(!usegoal && !goalnode && (!goalent || goalent->deadflag))
   {
      return false;
   }

   ResetForces();

   if(!wander)
   {
      if(self.lastmove == STEPMOVE_OK)
      {
         stuck = 0;
      }
      else
      {
         stuck++;
         if(stuck >= 2)
         {
            stuck = 3;
            wander = 1;
         }
      }
   }

   switch(wander)
   {
   case 1:
      stuck--;
      if(!stuck)
      {
         wander = 0;
         nextpathtime = 0;
         path = NULL;
         break;
      }
      wanderstart = self.worldorigin;
      avoidvec = ChooseRandomDirection(self);
      wandertime = level.time + 1;
      wander = 2;

   case 2:
      seek.SetTargetPosition(avoidvec);
      seek.SetTargetVelocity(vec_zero);
      seek.SetPosition(self.worldorigin);
      seek.SetDir(self.movedir);
      seek.SetMaxSpeed(self.movespeed);
      result = seek.Evaluate(self);
      if(result)
      {
         if((level.time > wandertime) && (self.lastmove != STEPMOVE_OK))
         {
            wander = 0;
            stuck = 0;
         }
         self.Accelerate(seek.steeringforce);
         return true;
      }
      wander = 0;
      nextpathtime = 0;
      path = nullptr;
      break;

      //self.SetAnim( "idle" );
      turnto.SetDirection((wanderstart - self.worldorigin).toYaw());
      wander = 3;
      wandertime = level.time + 1;

   case 3:
      if(level.time < wandertime)
      {
         turnto.Evaluate(self);
         self.Accelerate(turnto.steeringforce);
         return true;
      }

      //self.SetAnim( anim );
      wander = 0;
      nextpathtime = 0;
      path = nullptr;
      break;
   }

   if(path && follow.DoneWithPath(self))
   {
      path = nullptr;
      nextpathtime = 0;
   }

   if(goalent && (goalent->edict->solid != SOLID_NOT) && (goalent->edict->solid != SOLID_TRIGGER))
   {
      trace = G_Trace(self.worldorigin, self.mins, self.maxs, self.worldorigin +
                      Vector(self.orientation[0]) * self.movespeed * 0.1, &self, self.edict->clipmask, "Chase");
      if(trace.ent->entity == goalent)
      {
         return false;
      }
   }

   if(nextpathtime < level.time)
   {
      nextpathtime = level.time + newpathrate;
      if(goalnode)
      {
         path = follow.SetPath(self, self.worldorigin, goalnode->worldorigin);
      }
      else if(goalent)
      {
         path = follow.SetPath(self, self.worldorigin, goalent->worldorigin);
      }
      else
      {
         path = follow.SetPath(self, self.worldorigin, goal);
      }
   }

   if(!path)
   {
      if(goalnode)
      {
         seek.SetTargetPosition(goalnode->worldorigin);
         seek.SetTargetVelocity(vec_zero);
      }
      else if(goalent)
      {
         seek.SetTargetPosition(goalent->worldorigin);
         seek.SetTargetVelocity(goalent->velocity);
      }
      else
      {
         seek.SetTargetPosition(goal);
         seek.SetTargetVelocity(vec_zero);
      }

      seek.SetPosition(self.worldorigin);
      seek.SetDir(self.movedir);
      seek.SetMaxSpeed(self.movespeed);
      result = seek.Evaluate(self);
      //seek.DrawForces();

      steeringforce = seek.steeringforce;

      if(!result)
      {
         return false;
      }
   }
   else
   {
      follow.SetPosition(self.worldorigin);
      follow.SetDir(self.movedir);
      follow.SetMaxSpeed(self.movespeed);
      if(!follow.Evaluate(self))
      {
         nextpathtime = 0;
         if(goalnode)
         {
            self.frame_delta = goalnode->worldorigin - self.worldorigin;
            return false;
         }
      }
      //follow.DrawForces();
      steeringforce = follow.steeringforce;
   }

   if(avoidtime < level.time)
   {
      avoid.SetMaxSpeed(self.movespeed);
      avoid.SetPosition(self.worldorigin);
      avoid.SetDir(self.movedir);
      avoid.Evaluate(self);

      if(avoid.steeringforce == vec_zero)
      {
         avoidtime = level.time + 0.1;
      }
      else
      {
         steeringforce += avoid.steeringforce;
      }
   }

   self.Accelerate(steeringforce);

   return true;
}

void Chase::End(Actor &self)
{
   //if ( wander && ( self.newanimnum != -1 ) )
      //{
      //self.SetAnim( anim );
      //}
   seek.End(self);
   follow.End(self);
   avoid.End(self);
   path = nullptr;
   turnto.End(self);
}

// EOF

