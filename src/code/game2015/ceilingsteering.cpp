/*
================================================================
CEILING STEERING FOR CRAWLER
================================================================

Copyright (C) 2020 by Night Dive Studios, Inc.
All rights reserved.

See the license.txt file for conditions and terms of use for this code.
*/

#include "g_local.h"
#include "ceilingsteering.h"
#include "actor.h"

/****************************************************************************

  CeilingObstacleAvoidance Class Definition

****************************************************************************/

CLASS_DECLARATION(Steering, CeilingObstacleAvoidance, NULL);

ResponseDef CeilingObstacleAvoidance::Responses[] =
{
   { NULL, NULL }
};

void CeilingObstacleAvoidance::AvoidWalls(qboolean avoid)
{
   avoidwalls = avoid;
}

void CeilingObstacleAvoidance::ShowInfo(Actor &self)
{
   Steering::ShowInfo(self);

   gi.printf("\navoidwalls: %d\n", avoidwalls);
}

qboolean CeilingObstacleAvoidance::Evaluate(Actor &self)
{
   Vector   predictedposition;
   Vector   normal;
   Vector   angles;
   Vector   dir;
   Vector   right;
   Vector   delta;
   float    urgency;
   float    dot;
   trace_t  tracef;
   Entity  *ent;

   ResetForces();

   angles = self.movedir.toAngles();
   angles.AngleVectors(NULL, &right, NULL);

   origin = self.worldorigin;
   origin.z -= 1;
   predictedposition = origin + self.movedir * self.movespeed;//maxspeed;

   tracef = G_Trace(origin, self.mins, self.maxs, predictedposition, &self, self.edict->clipmask, "CeilingObstacleAvoidance forward");
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
            return true;

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

  CeilingObstacleAvoidance2 Class Definition

****************************************************************************/

CLASS_DECLARATION(Steering, CeilingObstacleAvoidance2, NULL);

ResponseDef CeilingObstacleAvoidance2::Responses[] =
{
   { NULL, NULL }
};

void CeilingObstacleAvoidance2::AvoidWalls(qboolean avoid)
{
   avoidwalls = avoid;
}

void CeilingObstacleAvoidance2::ShowInfo(Actor &self)
{
   Steering::ShowInfo(self);

   gi.printf("\navoidwalls: %d\n", avoidwalls);
}

qboolean CeilingObstacleAvoidance2::Evaluate(Actor &self)
{
   Vector   predictedposition;
   Vector   normal;
   Vector   angles;
   Vector   dir;
   Vector   right;
   Vector   delta;
   float    urgency;
   float    dot;
   trace_t  tracef;
   Entity  *ent;

   ResetForces();

   angles = self.movedir.toAngles();
   angles.AngleVectors(NULL, &right, NULL);

   origin = self.worldorigin;
   origin.z -= 1;
   predictedposition = origin + self.movedir * self.movespeed;//maxspeed;

   tracef = G_Trace(origin, self.mins, self.maxs, predictedposition, &self, self.edict->clipmask, "ObstacleAvoidance2 forward");
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
            return true;

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

  CeilingChase Class Definition

****************************************************************************/

CLASS_DECLARATION(Steering, CeilingChase, NULL);

ResponseDef CeilingChase::Responses[] =
{
   { NULL, NULL }
};

void CeilingChase::SetGoalPos(Vector goalpos)
{
   goal    = goalpos;
   usegoal = true;
   goalent = NULL;
}

void CeilingChase::SetTarget(Entity *ent)
{
   goalent = ent;
   usegoal = false;
}

void CeilingChase::ShowInfo(Actor &self)
{
   Steering::ShowInfo(self);

   gi.printf("\nseek:\n");
   seek.ShowInfo(self);

   gi.printf("goal: ( %f, %f, %f )\n", goal.x, goal.y, goal.z);

   if(goalent)
      gi.printf("\ngoalent: #%d '%s'\n", goalent->entnum, goalent->targetname.c_str());
   else
      gi.printf("\ngoalent: NULL\n");

   gi.printf("avoid:\n");
   avoid.ShowInfo(self);

   gi.printf("\ntime: %f\n", avoidtime);

   gi.printf("usegoal: %d\n", usegoal);
   gi.printf("wander: %d\n", wander);
   gi.printf("stuck: %d\n", stuck);
   gi.printf("avoidvec : ( %f, %f, %f )\n", avoidvec.x, avoidvec.y, avoidvec.z);
}

void CeilingChase::Begin(Actor &self)
{
   seek.Begin(self);
   avoid.AvoidWalls(false);
   avoid.Begin(self);
   turnto.Begin(self);
   anim   = self.animname;
   stuck  = 0;
   wander = 0;
}

Vector CeilingChase::ChooseRandomDirection(Actor &self)
{
   Vector  ang;
   trace_t trace;
   trace_t groundtrace;
   Vector  end;
   Vector  groundend;

   Vector s(0, 0, -STEPSIZE);
   Vector start(self.worldorigin + s);

   // quantize to nearest 45 degree
   int   u = ((int)(self.worldangles.y * (1 / 45) + 22.5)) * 45;
   float bestfraction = -1;
   
   //
   // in case we don't find anything!
   //
   Vector bestdir(self.worldorigin - (Vector(self.orientation[0]) * 100));

   for(int i = 0; i <= 180; i += 20)
   {
      if(rand() < 0.3)
      {
         i += 20;
      }

      int t = i;

      if(rand() < 0.5)
      {
         // sometimes we choose left first, other times right.
         t = -t;
      }

      for(int j = -1; j < 2; j += 2)
      {
         if((j == 1) && (i == 180))
         {
            ang.y = self.worldangles.y + (t * j);
         }
         else
         {
            ang.y = u + t * j;
         }

         Vector dir;
         ang.AngleVectors(&dir, NULL, NULL);

         end = self.worldorigin + dir * 140 + s;
         trace = G_Trace(start, self.mins, self.maxs, end, &self,
                         self.edict->clipmask, "CeilingChase::ChooseRandomDirection 1");
         if((trace.fraction > bestfraction) && (!trace.startsolid) && !(trace.allsolid))
         {
            if(trace.endpos != avoidvec)
            {
               // check if we're near the ground 
               end = self.worldorigin + dir * 32 + s;
               groundend = end;
               groundend.z += STEPSIZE * 2;
               groundtrace = G_Trace(end, self.mins, self.maxs, groundend, &self,
                                     self.edict->clipmask, "CeilingChase::ChooseRandomDirection 2");
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

qboolean CeilingChase::Evaluate(Actor &self)
{
   qboolean result;

   if(!usegoal && (!goalent || goalent->deadflag))
      return false;

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
      break;

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

      wander = 0;
      break;
   }

   if(goalent && (goalent->edict->solid != SOLID_NOT) && (goalent->edict->solid != SOLID_TRIGGER))
   {
      trace_t trace = G_Trace(self.worldorigin, self.mins, self.maxs, self.worldorigin +
                              Vector(self.orientation[0]) * self.movespeed * 0.1, &self, self.edict->clipmask, "CeilingChase");
      if(trace.ent->entity == goalent)
         return false;
   }

   if(goalent)
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

   steeringforce = seek.steeringforce;

   if(!result)
      return false;

   if(avoidtime < level.time)
   {
      avoid.SetMaxSpeed(self.movespeed);
      avoid.SetPosition(self.worldorigin);
      avoid.SetDir(self.movedir);
      avoid.Evaluate(self);

      if(avoid.steeringforce == vec_zero)
         avoidtime = level.time + 0.1;
      else
         steeringforce += avoid.steeringforce;
   }

   self.Accelerate(steeringforce);

   return true;
}

void CeilingChase::End(Actor &self)
{
   seek.End(self);
   avoid.End(self);
   turnto.End(self);
}

//EOF

