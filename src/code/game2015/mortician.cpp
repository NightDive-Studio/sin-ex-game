/*
================================================================
MORTICIAN
================================================================

Copyright (C) 2020 by Night Dive Studios, Inc.
All rights reserved.

See the license.txt file for conditions and terms of use for this code.
*/

#include "g_local.h"
#include "actor.h"
#include "mortician.h"
#include "behavior.h"
#include "path.h"
#include "explosion.h"

CLASS_DECLARATION(Actor, Mortician, "monster_mortician");

Event EV_Mortician_MJumpTo("mjumpto");
Event EV_Mortician_Flash("doflash");

ResponseDef Mortician::Responses[] =
{
   { &EV_Mortician_MJumpTo, (Response)&Mortician::JumpToEvent },
   { &EV_Mortician_Flash,   (Response)&Mortician::FlashEvent  },
   { nullptr, nullptr }
};

void Mortician::FlashEvent(Event *ev)
{
   FlashPlayers(worldorigin, 1, 1, 1, 0.5, 300);
}

void Mortician::JumpToEvent(Event *ev)
{
   if((deadflag) && (actortype != IS_INANIMATE))
   {
      return;
   }

   auto e = new Event(EV_Behavior_Args);
   e->SetSource(EV_FROM_SCRIPT);
   e->SetThread(ev->GetThread());
   e->SetLineNumber(ev->GetLineNumber());

   e->AddEntity(this);

   int n = ev->NumArgs();

   e->AddString("mjump");
   for(int i = 1; i <= n; i++)
      e->AddToken(ev->GetToken(i));

   SetBehavior(new MJump(), e, ev->GetThread());
}


// This takes a target vector, and then sets
// the entity's horizontal and vertical velocity so that 
// it will jump gracefully to its target, and returns 
// the estimated travel time

// It is a copy of Actor::JumpTo, but I had to rewrite it because jumpTo
// spins the actor around to face its target, which is not something I 
// want the mortician to do.
float Mortician::JumpTo(const Vector &targ)
{
   float vertical_speed;
   CheckWater();

   //
   // if we got a jump, go into that mode
   //
   float  traveltime = 0;
   Vector target(targ);
   Vector dir(target - worldorigin);
   Vector xydir(dir);
   xydir.z = 0;

   float speed = movespeed * 1.7 * gravity;
   speed += (xydir.length()/ 2.5) * gravity;

   //setAngles( xydir.toAngles() );
   traveltime = xydir.length() / speed;

   //
   // we add 16 to allow for a little bit higher
   //
   if(waterlevel > 2)
      vertical_speed = ((dir.z + 16) / traveltime) + (0.5f * gravity * 60 * traveltime);
   else
      vertical_speed = ((dir.z + 16) / traveltime) + (0.5f * gravity * sv_gravity->value * traveltime);

   xydir.normalize();

   velocity = speed * xydir;
   velocity.z = vertical_speed;

   return traveltime;
}

/****************************************************************************

  MJump Class Definition

   Based of jump behaviour, used for the mortician, so he can look better
   when he jumps.
****************************************************************************/

CLASS_DECLARATION(Behavior, MJump, NULL);

ResponseDef MJump::Responses[] =
{
   { &EV_Behavior_Args, (Response)&MJump::SetArgs },
   { &EV_Behavior_AnimDone, (Response)&MJump::AnimDone },
   { NULL, NULL }
};

void MJump::AnimDone(Event *ev)
{
   animdone = true;
}

void MJump::SetArgs(Event *ev)
{
   //
   // see if it is an entity first 
   //
   movegoal = AI_FindNode(ev->GetString(2));
   if(movegoal)
   {
      goal = movegoal->worldorigin;
   }
   else
   {
      Entity *ent = ev->GetEntity(2);
      if(ent)
      {
         goal = ent->worldorigin;
         target = ent;				//We need to set target seperate because goal may be changed soon
      }
   }

   if(ev->NumArgs() >= 3)
      accurate = ev->GetInteger(3);
   else
      accurate = false;
}

void MJump::ShowInfo(Actor &self)
{
   Behavior::ShowInfo(self);
}

///////////////////////
// Find close sight node to
///////////////////////
// This function returns a pathnode which is near the target, visible from the target and from
// self's current position, and is as high or higher than the target (since that usually conveys
// a sense of tactical advantage).  The node must also be more than a minimum distance from
// self's current position
//
Vector MJump::FindCloseSightNodeTo(Actor &self, const Vector &pos, float maxdist)
{
   PathNode	*bestnode;
   float		bestdist;
   PathNode	*node;
   Vector		delta;
   float		dist;
   trace_t		trace1;
   trace_t		trace2;
   Vector		selfpos;
   Vector		temppos;
   const float minjumpdist = 150*150; // Absolute minimum distance he will jump
   // How do I declare these as constants?
   float		maxjumpdist = 900*900;    // Maximum distance he is capable of jumping (squared)
   float		jumpdist = 300*300;       // Distance he likes to keep from his enemy

   // reduce maxjumpdist if we need to
   if((maxdist*maxdist) < maxjumpdist)
      maxjumpdist = maxdist*maxdist;

   // Raise our endpoints up for a variety of reasons
   selfpos    = self.worldorigin;
   selfpos.z += 100;
   temppos    = pos;
   temppos.z += 100;
   bestnode   = NULL;
   bestdist   = 999999999; // greater than ( 8192 * sqr(2) ) ^ 2 -- maximum squared distance

   delta = selfpos - temppos;
   dist = delta * delta;
   if(dist > jumpdist)	// We are a long way away so jump jump near
   {
      for(int i = rand()%4; i <= ai_maxnode; i+=4)	//Check every fourth node since we don't want do be too accurate
      {
         node = AI_GetNode(i);
         if(node && (node->occupiedTime <= level.time))
         {
            delta = node->worldorigin - self.worldorigin;
            dist = delta * delta;
            if((node->worldorigin.z > (pos.z-20)) && (dist > minjumpdist) && (dist < maxjumpdist))
            {
               // get the distance squared (faster than getting real distance)
               delta = node->worldorigin - pos;
               dist = delta * delta;
               trace1 = G_Trace(temppos, Vector(0, 0, 0), Vector(0, 0, 0), node->worldorigin, &self, MASK_SHOT, "MJump::FindCloseSightNodeTo");
               trace2 = G_Trace(selfpos, Vector(0, 0, 0), Vector(0, 0, 0), node->worldorigin, &self, MASK_SHOT, "MJump::FindCloseSightNodeTo");

               // Now if this one can be seen from both ends, is closest, and is also higher than the target, we want it.
               if((trace1.fraction >= 1) && (trace2.fraction >= 1) && (dist < bestdist))
               {
                  bestnode = node;
                  bestdist = dist;
               }
            }
         }
      }
   }
   else // We are close so try to jump over our enemy
   {
      for(int i = rand()%4; i <= ai_maxnode; i+=4)	//Check every fourth node since we don't want do be too accurate
      {
         node = AI_GetNode(i);
         if(node && (node->occupiedTime <= level.time))
         {
            delta = node->worldorigin - self.worldorigin;
            // get the distance squared (faster than getting real distance)
            dist = delta * delta;

            delta = node->worldorigin - pos;
            jumpdist = delta * delta;		// Jumpdist is now the distance from the test node to the target

            if((node->worldorigin.z > (pos.z-10)) && (dist > minjumpdist) && (dist > jumpdist) && (dist < maxjumpdist)) //This is the line that makes him try to jump over
            {
               dist = jumpdist;
               trace1 = G_Trace(temppos, Vector(0, 0, 0), Vector(0, 0, 0), node->worldorigin, &self, MASK_SHOT, "MJump::FindCloseSightNodeTo");
               trace2 = G_Trace(selfpos, Vector(0, 0, 0), Vector(0, 0, 0), node->worldorigin, &self, MASK_SHOT, "MJump::FindCloseSightNodeTo");
               // Now if this one can be seen from both ends, is closest, and is also higher than the target, we want it.
               if((trace1.fraction >= 1) && (trace2.fraction >= 1) && (dist < bestdist))
               {
                  bestnode = node;
                  bestdist = dist;
               }
            }
         }
      }
   } //end else
   
   // Oh, shit, we didn't find anywhere we can jump to, let's look harder this time
   if(bestnode == NULL)
   {
      for(int i = 0; (i <= ai_maxnode && bestnode == NULL); i++)	//Check every node this time since we're getting desperate
      {
         node = AI_GetNode(i);
         // if we don't have a node value, carry on
         if(!node)
            continue;

         delta = node->worldorigin - self.worldorigin;	//Make sure it isn't too far away
         dist = delta * delta;
         if(node && (node->occupiedTime <= level.time) && (dist > minjumpdist) && (dist < maxjumpdist) && (node->worldorigin.z > pos.z-500))
         {
            // get the distance squared (faster than getting real distance)
            delta = node->worldorigin - pos;
            dist = delta * delta;
            trace1 = G_Trace(temppos, Vector(0, 0, 0), Vector(0, 0, 0), node->worldorigin, &self, MASK_SHOT, "MJump::FindCloseSightNodeTo");
            trace2 = G_Trace(selfpos, Vector(0, 0, 0), Vector(0, 0, 0), node->worldorigin, &self, MASK_SHOT, "MJump::FindCloseSightNodeTo");
            if((trace1.fraction >= 1) && (trace2.fraction >= 1) && (dist < bestdist))
            {
               bestnode = node;
               bestdist = dist;
            }
         }
      }
   }

   if(bestnode == NULL) // Revert back to original target if we didn't find anywhere near it
      return pos;
   else
      return bestnode->worldorigin;
}


void MJump::Begin(Actor &self)
{
   trace_t trace;
   float   direction;              //Used for testing of direction of movement relative to orientation
   float   maxjumpdistance = 1000; // Clips the max jump distance if we have a low ceiling

   // This bit aborts the jump if the ceiling is too low, and culls it if it is a little too low

   Vector upcheck(self.worldorigin);
   upcheck.z += heightneeded;
   trace = G_Trace(self.worldorigin, Vector(-22, -22, 0), Vector(22, 22, 0), upcheck, &self, MASK_MONSTERSOLID, "MJumpTo::Begin");
   if(trace.fraction >= 1)
   {

      state = 0; // I shouldn't have to do this here, but I tried it anyhow and it worked...

      upcheck = self.worldorigin;
      upcheck.z += heightwanted;
      trace = G_Trace(self.worldorigin, Vector(-22, -22, 0), Vector(22, 22, 0), upcheck, &self, MASK_MONSTERSOLID, "MJumpTo::Begin");
      if(trace.fraction < 1)
         // In this piece, we assume that we can jump thrice as far as the roof is high, 
         // which is a reasonable (if not entirely accurate) assumption, I think.
         maxjumpdistance = heightwanted * trace.fraction * 3;

      if(!accurate)
         goal = FindCloseSightNodeTo(self, goal, maxjumpdistance);

      direction = Vector(self.orientation[0]).toYaw() - (goal-self.worldorigin).toYaw();
      direction = fmod(direction, 360.0f);
      if(direction < 0)
         direction += 360;
      if((direction >= 45) && (direction < 135))
         jumpdir = "right";
      else if((direction >= 135) && (direction < 225))
         jumpdir = "back";
      else if((direction >= 225) && (direction < 315))
         jumpdir = "left";
      else
         jumpdir = "forward";

      anim = "jump_";
      anim += jumpdir;

      if(anim.length())
      {
         animdone = false;
         self.SetAnim(anim.c_str(), EV_Actor_NotifyBehavior);
      }
   } //end if (ceiling is high enough)
   else
   {
      jumpok = false;
   }
}

qboolean MJump::Evaluate(Actor &self)
{
   Vector		landpos;
   trace_t		trace;
   float		direction;	//Used for testing of direction of movement relative to orientation
   Vector		targetdir;

   // Do a quick check so we don't cause major problems if we accidentally pass
   // a non-mortician to this function.
   if(!self.isSubclassOf<Mortician>())
      return false;

   // Do a check to see if we have any ceiling height
   if(!jumpok)
      return false;

   if(!target)
      target = world;

   // OK, now get on with it
   Mortician *mself = (Mortician *)(&self);

   upspeed = (self.worldorigin.z - oldworldheight)/FRAMETIME;

   float traveltime = 0;
   switch(state)
   {
   case 0:
      if(animdone)
      {
         animdone = false;
         anim  = "jump_";
         anim += jumpdir;
         anim += "_inair";
         self.SetAnim(anim, EV_Actor_NotifyBehavior);
         // Use mself here instead of self so we call the correct jumping function
         traveltime = mself->JumpTo(goal);
         endtime = traveltime + level.time;

         self.last_jump_time = endtime;

         state = 1;
      }
      break;

   case 1:
      // Now, watch the player as we move upwards
      if(animdone)
      {
         Vector tempv;

         ///////////////
         // Turn to face the player
         if(target == NULL)
         {
            tempv = (goal - self.worldorigin);
            targetdir = tempv.toAngles();
            targetdir.setPitch(0);
         }
         else
         {
            tempv = (target->worldorigin - self.worldorigin);
            targetdir = tempv.toAngles();
            // Invert pitch since setAngles is screwy, and divide by 2
            if(targetdir.pitch() > 180)
               targetdir.setPitch(targetdir.pitch() - 360);
            targetdir.setPitch(-targetdir.pitch()/2);
         }
         self.setAngles(targetdir);

         /////////////////
         // Check to see if we will be moving downwards next frame
         if(upspeed <= 0)
         {
            state = 2;
         }
         else
         {
            // Take a shot from in the air

            if(((endtime-level.time)>0.5) &&
               animdone &&
               self.currentEnemy &&
               self.CanSee(self.currentEnemy)) // make sure can actually hit
            {
               animdone = false;
               anim = "air_shoot";
               self.SetAnim(anim.c_str(), EV_Actor_NotifyBehavior);
            }
            else if(animdone)
            {
               self.SetAnim("air_idle");
            }
         }
      }
      break;

   case 2:
      //
      // Moving downwards, waiting to hit the ground...
      //

      if(animdone)
         self.SetAnim("air_idle");

      ///////////////
      // Turn to face the player
      if(target == NULL)
      {
         Vector tempr = (goal - self.worldorigin);
         targetdir = tempr.toAngles();
         targetdir.setPitch(0);
      }
      else
      {
         Vector tempr(target->worldorigin - self.worldorigin);
         targetdir = tempr.toAngles();
         // Invert pitch since setAngles is screwy, and divide by 2
         if(targetdir.pitch() > 180)
            targetdir.setPitch(targetdir.pitch() - 360);
         targetdir.setPitch(-targetdir.pitch()/2);
      }
      self.setAngles(targetdir);
      landpos = self.worldorigin + (((upspeed - self.gravity*sv_gravity->value)*Vector(0, 0, 1)) * FRAMETIME);
      trace   = G_Trace(self.worldorigin, Vector(-22, -22, 0), Vector(22, 22, 0), landpos, &self, MASK_MONSTERSOLID, "MJumpTo::Evaluate");

      if((trace.fraction < 1) || (self.groundentity))
      {
         state = 3;

         // Make sure we're not tipped over any more
         //
         Vector tempp(target->worldorigin - self.worldorigin);
         targetdir = tempp.toAngles();
         targetdir.setPitch(0);
         self.setAngles(targetdir);

         //
         // Set landing animation depending on velocity relative to orientation
         //

         // We have to use toYaw here as opposed to yaw because 
         // this is a velocity vector rather than an angle vector
         direction = Vector(self.orientation[0]).toYaw() - self.velocity.toYaw();
         direction = fmod(direction, 360.0f);
         if(direction < 0)
            direction += 360;
         temp_vel = self.velocity;
         temp_vel.z = 0;
         if((temp_vel.length()/(-upspeed) < 0.4) || (temp_vel.length() < 60))
            jumpdir = "down";
         else if((direction >= 45) && (direction < 135))
            jumpdir = "right";
         else if((direction >= 135) && (direction < 225))
            jumpdir = "back";
         else if((direction >= 225) && (direction < 315))
            jumpdir = "left";
         else
            jumpdir = "forward";

         anim = "land_";
         anim += jumpdir;

         //
         // if we have an anim, we go to state 4
         //
         if(self.HasAnim(anim.c_str()))
         {
            animdone = false;
            self.SetAnim(anim.c_str(), EV_Actor_NotifyBehavior);
            state = 4;
         }
         else
         {
            self.SetAnim("jump_idle", EV_Actor_NotifyBehavior);
            state = 3;
         }
      }
      // Record downward velocity for comparison with horizontal velocity
      // later, to determine which landing animation should be played.

      break;

   case 3:
      // 
      // we are on the ground and waiting to timeout
      //
      if((level.time > endtime) && (self.groundentity))
         return false;
      break;
   case 4:
      //
      // we are on the ground and waiting for our landing animation to finish
      //
      if(animdone)
      {
         if(self.groundentity)
            return false;
         else
            self.SetAnim("idle");
      }
      break;
   }

   //Save off origin for velocity calcs
   oldworldheight = self.worldorigin.z;

   return true;
}

void MJump::End(Actor &self)
{
   Vector targetdir =  (target->worldorigin - self.worldorigin);
   targetdir = targetdir.toAngles();
   targetdir.setPitch(0);
   self.setAngles(targetdir);

   self.SetAnim("idle");
}

void MJump::Archive(Archiver &arc)
{
   Behavior::Archive(arc);

   arc.WriteFloat(endtime);
   arc.WriteString(anim);
   arc.WriteString(jumpdir);
   arc.WriteInteger(state);
   arc.WriteBoolean(animdone);
   arc.WriteVector(goal);
   arc.WriteVector(temp_vel);
   arc.WriteBoolean(accurate);
   arc.WriteSafePointer(target);
   arc.WriteFloat(heightwanted);
   arc.WriteFloat(heightneeded);
   arc.WriteBoolean(jumpok);
   arc.WriteFloat(oldworldheight);
   arc.WriteFloat(upspeed);
   arc.WriteBoolean(jumpbegun);
}

void MJump::Unarchive(Archiver &arc)
{
   Behavior::Unarchive(arc);

   arc.ReadFloat(&endtime);
   arc.ReadString(&anim);
   arc.ReadString(&jumpdir);
   arc.ReadInteger(&state);
   arc.ReadBoolean(&animdone);
   arc.ReadVector(&goal);
   arc.ReadVector(&temp_vel);
   arc.ReadBoolean(&accurate);
   arc.ReadSafePointer(&target);
   arc.ReadFloat(&heightwanted);
   arc.ReadFloat(&heightneeded);
   arc.ReadBoolean(&jumpok);
   arc.ReadFloat(&oldworldheight);
   arc.ReadFloat(&upspeed);
   arc.ReadBoolean(&jumpbegun);
}

// EOF

