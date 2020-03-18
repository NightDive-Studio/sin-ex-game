/*
================================================================
THUG & ADDITIONAL AI
================================================================

Copyright (C) 2020 by Night Dive Studios, Inc.
All rights reserved.

See the license.txt file for conditions and terms of use for this code.
*/

#include "g_local.h"
#include "actor.h"
#include "thug.h"
#include "behavior.h"
#include "path.h"

/****************************************************************************

  DuckAttack Class Definition

  Added by Boon, who doesn't know C++
  I've made this class a descendant of Behavior, and based the code on
  that of strafeattack, and commented out the methods I don't want to change.
  It should really be called LeanAttack, since it lines the player up so he is
  ready to lean out and fire.  That is, except for the fact that it looks for
  crouching animations when seeing if it can do stuff.

****************************************************************************/

CLASS_DECLARATION(Behavior, DuckAttack, nullptr);

ResponseDef DuckAttack::Responses[] =
{
   { &EV_Behavior_AnimDone, (Response)&DuckAttack::AnimDone },
   { nullptr, nullptr }
};

void DuckAttack::ShowInfo(Actor &self)
{
   Behavior::ShowInfo(self);

   turn.ShowInfo(self);
}

void DuckAttack::AnimDone(Event *ev)
{
   animdone = true;
}

void DuckAttack::Begin(Actor &self)
{
   state = 0;
   animdone = true;
}

qboolean DuckAttack::Evaluate(Actor &self)
{
   int num;
   Vector delta;
   Vector left;
   Vector pos;

   if(!self.currentEnemy)
      return false;

   if(!animdone)
      return true;

   switch(state)
   {
   case 0:
      // Start turning towards the enemy
      delta = self.currentEnemy->worldorigin - self.worldorigin;
      delta.z = 0; //KDT fix to prevent tipping over :)
      {
         Vector tempv;
         tempv = delta.toAngles();
         self.setAngles(tempv);
      }

      state = 2;
      break;

   case 1:
      state = 2;
      // No need to return here as we really just want to go onto the next case now

   case 2:
      // This bit finds the direction of the enemy, so we can strafe around him
      delta = self.currentEnemy->worldorigin - self.worldorigin;
      left.x = -delta.y;
      left.y = delta.x;
      left.normalize();

      //first, check to see if we can shoot him from where we are
      if(self.CanShootFrom(self.worldorigin, self.currentEnemy, false))
      {
         return false;
      }

      //If we couldn't hit the enemy from where we are, see if we could hit him by
      //leaning.  If we can, let Lean_AimAndShoot take care of it
      num = gi.Anim_Random(self.edict->s.modelindex, "crouch_strafe_left");
      if(num != -1)
      {
         gi.Anim_Delta(self.edict->s.modelindex, num, delta.vec3());
         delta *= self.edict->s.scale *0.8;
         pos = self.worldorigin + left * delta.length();
         if(self.CanMoveTo(pos) && self.CanShootFrom(pos, self.currentEnemy, false))
            return false;
      }

      num = gi.Anim_Random(self.edict->s.modelindex, "crouch_strafe_right");
      if(num != -1)
      {
         gi.Anim_Delta(self.edict->s.modelindex, num, delta.vec3());
         delta *= self.edict->s.scale *0.8;
         pos = self.worldorigin - left * delta.length();
         if(self.CanMoveTo(pos) && self.CanShootFrom(pos, self.currentEnemy, false))
            return false;
      }

      //If we couldn't hit the enemy from where we are, see if we could hit him by
      //strafing first and then leaning.  If we can, do the strafing bit now
      num = gi.Anim_Random(self.edict->s.modelindex, "crouch_strafe_left");
      if(num != -1)
      {
         gi.Anim_Delta(self.edict->s.modelindex, num, delta.vec3());
         delta *= self.edict->s.scale *1.8;	/* times 1.8 because we have 1 for the strafe and 0.8 for the lean */
         pos = self.worldorigin + left * delta.length();
         if(self.CanMoveTo(pos) && self.CanShootFrom(pos, self.currentEnemy, false))
         {
            // We set the flag Actor_FinishedBehavior so that DuckAttack will terminate
            // when the animation finishes.  We return TRUE so that it won't terminate
            // immediately.  We set state to 3 so that it won't do anything else while 
            // it is waiting to terminate.
            self.SetAnim("crouch_strafe_left", EV_Actor_FinishedBehavior);
            state = 3;
            return true;
         }
      }

      num = gi.Anim_Random(self.edict->s.modelindex, "crouch_strafe_right");
      if(num != -1)
      {
         gi.Anim_Delta(self.edict->s.modelindex, num, delta.vec3());
         delta *= self.edict->s.scale *1.8;
         pos = self.worldorigin - left * delta.length();
         if(self.CanMoveTo(pos) && self.CanShootFrom(pos, self.currentEnemy, false))
         {
            self.SetAnim("crouch_strafe_right", EV_Actor_FinishedBehavior);
            state = 3;
            return true;
         }
      }

      // Now see if we could do it by strafing even further...
      num = gi.Anim_Random(self.edict->s.modelindex, "crouch_strafe_left");
      if(num != -1)
      {
         gi.Anim_Delta(self.edict->s.modelindex, num, delta.vec3());
         delta *= self.edict->s.scale *3.8;	// times 4 = 3 for strafes and 0.8 for a lean 
         pos = self.worldorigin + left * delta.length();
         if(self.CanMoveTo(pos) && self.CanShootFrom(pos, self.currentEnemy, false))
         {
            // Set the anim to go left, and let this case run through again to take
            // care of the second (and possibly third) strafes left
            // Note that I wanted to set EV_NotifyBehavior here so this would run recursively,
            // but it didn't work for some reason, so I've had to set EV_Finished which means it 
            // doesn't work quite as well as I wanted it to.
            animdone = false;
            self.SetAnim("crouch_strafe_left", EV_Actor_FinishedBehavior);
            state = 3;//2;
            return true;
         }
      }

      num = gi.Anim_Random(self.edict->s.modelindex, "crouch_strafe_right");
      if(num != -1)
      {
         gi.Anim_Delta(self.edict->s.modelindex, num, delta.vec3());
         delta *= self.edict->s.scale *3.8;
         pos = self.worldorigin - left * delta.length();
         if(self.CanMoveTo(pos) && self.CanShootFrom(pos, self.currentEnemy, false))
         {
            animdone = false;
            self.SetAnim("crouch_strafe_right", EV_Actor_FinishedBehavior);
            state = 3;//2;
            return true;
         }
      }

      //We couldn't hit him by moving a fair distance to either side, so give up on 
      //shooting him from here.
      return false;
   }

   return true;
}

void DuckAttack::End(Actor &self)
{
   turn.End(self);
   self.SetAnim("crouch_idle");
}

/****************************************************************************

  Lean_AimAndShoot Class Definition

  Added by Boon to allow the Thug to lean left and right to shoot around corners

****************************************************************************/

CLASS_DECLARATION(AimAndShoot, Lean_AimAndShoot, nullptr);

ResponseDef Lean_AimAndShoot::Responses[] =
{
   { &EV_Behavior_Args,     (Response)&Lean_AimAndShoot::SetArgs  },
   { &EV_Behavior_AnimDone, (Response)&Lean_AimAndShoot::AnimDone },
   { nullptr, nullptr }
};

Lean_AimAndShoot::Lean_AimAndShoot()
{
   maxshots = 1;
   numshots = 0;
}

void Lean_AimAndShoot::Begin(Actor &self)
{
   //Vector forward;
   Vector   left;
   Vector   delta;
   Vector   pos;
   int      num;
   qboolean canshoot = true; // haleyjd 20170606: was uninitialized

   // Before we do anything, check that we have an enemy
   if(!self.currentEnemy)
   {
      canshoot = false; // Setting this causes the evaluate section to drop out as well
      return;
   }

   enemy_health = 0;
   mode         = 0;
   animdone     = false;

   delta = self.currentEnemy->worldorigin - self.worldorigin;
   left.x = -delta.y;
   left.y = delta.x;
   left.normalize();

   // Find out if we can hit the player from where we are, and if we can't,
   // try moving around

   if(!self.CanShootFrom(self.worldorigin, self.currentEnemy, false))
   {
      // If we couldn't hit him without moving, try moving left and then right,
      // and if that works, don't actually move, but lean
      moveanim = animprefix;
      moveanim += "strafe_left";
      num = gi.Anim_Random(self.edict->s.modelindex, moveanim.c_str());
      if(num != -1)
      {
         gi.Anim_Delta(self.edict->s.modelindex, num, delta.vec3());
         delta *= self.edict->s.scale *0.8;
         pos = self.worldorigin + left * delta.length();
         if(self.CanShootFrom(pos, self.currentEnemy, false))
         {
            animprefix+= "left_";
         }
         else //ie self.can't shoot from
         {
            moveanim = animprefix;
            moveanim += "strafe_right";
            num = gi.Anim_Random(self.edict->s.modelindex, moveanim.c_str());
            if(num != -1)
            {
               gi.Anim_Delta(self.edict->s.modelindex, num, delta.vec3());
               delta *= self.edict->s.scale *0.8;
               pos = self.worldorigin - left * delta.length();
               if(self.CanShootFrom(pos, self.currentEnemy, false))
               {
                  animprefix += "right_";
               }
               else
               {
                  //we couldn't hit him
                  canshoot = false;
               }
            }
         }
      }
   }

   if(canshoot)
   {
      readyfireanim = animprefix;
      readyfireanim += "readyfire";
      if(!self.HasAnim(readyfireanim.c_str()))
         readyfireanim = "";

      aimanim = animprefix;
      aimanim += "aim";
      if(!self.HasAnim(aimanim.c_str()))
         aimanim = "";

      fireanim = animprefix;
      fireanim += "fire";
      if(!self.HasAnim(fireanim.c_str()))
         fireanim = "";
   }
   else
   {
      readyfireanim = "";
      aimanim = "";
      fireanim = "";
   }
}

void Lean_AimAndShoot::SetMaxShots(int num)
{
   maxshots = (num>>1) + G_Random(num);
}

void Lean_AimAndShoot::SetArgs(Event *ev)
{
   SetMaxShots(ev->GetInteger(2));
   if(ev->NumArgs() > 2)
   {
      animprefix = ev->GetString(3);
   }
}

void Lean_AimAndShoot::AnimDone(Event *ev)
{
   animdone = true;
}

qboolean Lean_AimAndShoot::Evaluate(Actor &self)
{
   // Check that we were given a fire animation by the begin bit
   if(!fireanim.length())
      return false;

   switch(mode)
   {
   case 0:
      if(!self.currentEnemy)
         return false;

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
      if(aimanim.length())
      {
         animdone = false;
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
         return false;

      //
      // see if we aimed for too long
      //
      if(aim_time < level.time)
         return false;

      aim.SetTarget(self.currentEnemy);
      aim.Evaluate(self);

      // Don't go into our firing animation until our weapon is ready, and we are on target
      //Small piece of trivia:  the TRUE or FALSE on the end of CanShoot tells the function whether
      //to worry about which direction we are facing or not
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
            self.CurrentWeapon()->ForceReload();

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
               self.CurrentWeapon()->ForceReload();

            return false;
         }
         else if(!self.WeaponReady() || !self.CanShoot(self.currentEnemy, true))
         {
            // weapon not ready or not aimed at enemy, so just keep trying to get enemy in our sights
            if(aimanim.length())
               self.SetAnim(aimanim.c_str());

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

void Lean_AimAndShoot::End(Actor &self)
{
   aim.End(self);
   moveanim = animprefix;
   moveanim += "idle";
   if(moveanim.length())
      self.SetAnim(moveanim.c_str(), EV_Actor_NotifyBehavior);
}

/****************************************************************************

  Slim's concussion gun

  This is a concussion gun with a different world model

*****************************************************************************
*/

CLASS_DECLARATION(ConcussionGun, Slimconcussion, "weapon_slimconcussion");

ResponseDef Slimconcussion::Responses[] =
{
   { nullptr, nullptr }
};

Slimconcussion::Slimconcussion() : ConcussionGun()
{
   SetModels("concussion_slim.def", "concussion_slim.def");
}

// EOF

