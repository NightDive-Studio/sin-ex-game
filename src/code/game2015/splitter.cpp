/*
================================================================
SPLITTER
================================================================

Copyright (C) 2020 by Night Dive Studios, Inc.
All rights reserved.

See the license.txt file for conditions and terms of use for this code.
*/

#include "g_local.h"
#include "actor.h"
#include "gibs.h"
#include "splitter.h"
#include "flamethrower.h"

CLASS_DECLARATION(Actor, Splitter, "monster_splitter");

Event	EV_Splitter_SpawnBug("spawnbug");

ResponseDef Splitter::Responses[] =
{
   { &EV_Killed,            (Response)&Splitter::KilledEvent   },
   { &EV_Splitter_SpawnBug, (Response)&Splitter::SpawnBugEvent },
   { nullptr, nullptr }
};

////////////////////////////////////////////////
//
// SpawnBugEvent
//
////////////////////////////////////////////////

// Spawns a bug actor and throws it from the splitter
void Splitter::SpawnBugEvent(Event *ev)
{
   Vector pos;
   char temp[128];

   // Make the splitter not-solid, so the bug doesn't get caught in him
   setSolidType(SOLID_NOT);

   // Find the appropriate bone in the splitter for spawning the bug
   // at (currently disabled since there is not bone)
   pos = worldorigin;
   pos.z += 20;

   // Spawn lots of blood and shit
   // Well, not shit - not literally, anyhow.

   //Creategibs arguments:
   //1 - entity where they are spawned from
   //2 - damage done which translates to speed to throw them at
   //3 - maximum size of gibs to be created
   //4 - number of gibs
   //5 - optional model to use
   //From another spot in the code:  CreateGibs( this, health, gibsize, numgibs );
   CreateGibs(this, -50, 0.8, 5, "gib1.def");
   CreateGibs(this, -200, 0.3, 5, "gib2.def");

   // Clear the spawn args
   G_InitSpawnArguments();

   snprintf(temp, sizeof(temp), "%f %f %f", (float)pos.x, (float)pos.y, (float)pos.z);
   G_SetSpawnArg("origin", temp);
   snprintf(temp, sizeof(temp), "%f", (float)angles[YAW]);
   G_SetSpawnArg("angle", temp);
   G_SetSpawnArg("model", "bug.def");
   G_SetSpawnArg("targetname", TargetName()); // Gives the splitter's targetname to the bug
   G_SetSpawnArg("attackmode", "2");

   G_CallSpawn();
   // Clear the spawn args
   G_InitSpawnArguments();
}

////////////////////////////////////////////////
// KilledEvent
////////////////////////////////////////////////

// Based on Actor::Killed
// Since WarT had already altered Actor::Killed, I've put my alterations to that
// in #### markings.
void Splitter::KilledEvent(Event *ev)
{
   const char *name;
   Entity     *ent;
   int         num;
   Entity     *attacker;
   Entity     *inflictor;
   Vector      dir;
   Event      *event;
   int         i;
   str         dname;
   int         meansofdeath;

   CheckWater();
   StopAnimating();
   CancelPendingEvents();

   // don't allow them to fly, think, or swim anymore
   flags &= ~(FL_PRETHINK | FL_SWIM | FL_FLY);

   deadflag     = DEAD_DYING;
   takedamage   = DAMAGE_YES;
   groundentity = nullptr;

   attacker     = ev->GetEntity(1);
   inflictor    = ev->GetEntity(3);
   meansofdeath = ev->GetInteger(5);

   // Double all the armor
   DoubleArmor();

   SetVariable("other", ev->GetEntity(1));
   if(!DoAction("killed") && actorthread)
      actorthread->ProcessEvent(EV_ScriptThread_End);

   // Turn off dlight and shadow
   edict->s.renderfx &= ~(RF_DLIGHT|RF_XFLIP);

   //
   // kill the killtargets
   //
   // added extended targeting stuff
   name = KillTarget();
   for(int j = 0; j < 2; j++)
   {
      switch(j)
      {
      case 0:
         name = KillTarget();
         break;
      case 1:
         name = KillTarget2();
         break;
      }

      if(name && strcmp(name, ""))
      {
         num = 0;
         do
         {
            num = G_FindTarget(num, name);
            if(!num)
               break;

            ent = G_GetEntity(num);
            ent->PostEvent(EV_Remove, 0);
         }
         while(1);
      }
   }

   //
   // fire targets
   //
   // added extended targeting stuff
   for(int j = 0; j < 4; j++)
   {
      switch(j)
      {
      case 0:
         name = Target();
         break;
      case 1:
         name = Target2();
         break;
      case 2:
         name = Target3();
         break;
      case 3:
         name = Target4();
         break;
      }

      if(name && strcmp(name, ""))
      {
         num = 0;
         do
         {
            num = G_FindTarget(num, name);
            if(!num)
               break;

            ent = G_GetEntity(num);

            event = new Event(EV_Activate);
            event->AddEntity(attacker);
            ent->ProcessEvent(event);
         }
         while(1);
      }
   }

   //
   // see if we have a kill_thread
   //
   if(kill_thread.length() > 1)
   {
      //
      // create the thread, but don't start it yet
      //
      ScriptThread *thread = ExecuteThread(kill_thread, false);
      if(thread)
         ProcessScript(thread, NULL);
      else
         warning("Killed", "could not process kill_thread");
   }

   if(DoGib(meansofdeath, inflictor))
      deathgib = true;

   // skin darkening for death from flames
   if(inflictor->isSubclassOf<ThrowerFlame>())
   {
      edict->s.renderfx |= RF_LIGHTOFFSET;
      edict->s.lightofs = -127;

      CancelEventsOfType(EV_Sentient_HurtFlame);
      edict->s.effects  |= EF_FLAMES;
   }
   else
   {
      // turn off the actor's heat signature
      edict->s.effects &= ~EF_WARM;
   }

   if(currentWeapon)
      DropWeapon(currentWeapon);

   animOverride = false;

   //
   // determine death animation
   //
   if(!strncmp(animname.c_str(), "crouch", 6))
   {
      dname = "crouch_";
   }

   if(!strncmp(animname.c_str(), "live_split", 10))
   {
      dname = "live_split_";
   }
   else if(!strncmp(animname.c_str(), "dead", 4))
   {
      dname = "dead_split_";
   }
   else if(deathgib)
   {
      const char *location;

      location = ev->GetString(4);

      // Check for location first otherwise randomize
      if(!strcmp(location, "torso_upper"))
         dname += "gibdeath_upper";
      else if(!strcmp(location, "torso_lower"))
         dname += "gibdeath_lower";
      else if(strstr(location, "leg"))
         dname += "gibdeath_lower";
      else if(strstr(location, "arm"))
         dname += "gibdeath_upper";
      else if(strstr(location, "head"))
         dname += "gibdeath_upper";
      else if(G_Random() > 0.5)
         dname += "gibdeath_upper";
      else
         dname += "gibdeath_lower";
   }
   else
   {
      dname += "death_";
      dname += ev->GetString(4);
   }

   i = gi.Anim_Random(edict->s.modelindex, dname.c_str());

   if((i == -1) && !strncmp(animname.c_str(), "crouch", 6))
   {
      dname = "crouch_death";
      i = gi.Anim_Random(edict->s.modelindex, dname.c_str());
   }

   if((i == -1) && !strncmp(animname.c_str(), "live_split", 10))
   {
      dname = "live_split_death";
      i = gi.Anim_Random(edict->s.modelindex, dname.c_str());
   }
   if((i == -1) && !strncmp(animname.c_str(), "dead", 4))
   {
      dname = "dead_split_death";
      i = gi.Anim_Random(edict->s.modelindex, dname.c_str());
   }

   if(i == -1)
   {
      dname = "death";
   }

   if((i != -1) &&  (!strncmp(dname.c_str(), "gibdeath", 7)))
   {
      auto ev1 = new Event(EV_Gib);
      ev1->AddInteger(1);
      ProcessEvent(ev1);
   }

   if(attacker)
   {
      str location;
      float damage;

      damage   = ev->GetFloat(2);
      location = ev->GetString(4);

      event = new Event(EV_GotKill);
      event->AddEntity(this);
      event->AddInteger(damage);
      event->AddEntity(inflictor);
      event->AddString(location);
      event->AddInteger(meansofdeath);
      event->AddInteger(deathgib);
      attacker->ProcessEvent(event);
   }

   SetAnim(dname.c_str(), EV_Actor_Dead);

   // Call changeanim immediatly since we're no longer calling prethink
   ChangeAnim();

   //
   // moved this here so guys would not be solid right away
   //
   edict->svflags |= SVF_DEADMONSTER;
   edict->clipmask = MASK_DEADSOLID;

   angles.x = 0;
   angles.z = 0;
   setAngles(angles);
}

Splitter::Splitter() : Actor()
{
   setModel("splitter.def");
   modelIndex("bug.def");
}

/****************************************************************************
// for bug
  Jump2 Class Definition

****************************************************************************/

CLASS_DECLARATION(Jump, Jump2, nullptr);

ResponseDef Jump2::Responses[] =
{
   { nullptr, nullptr }
};

void Jump2::Begin(Actor &self)
{
   float  traveltime;
   float  dist;
   Vector goaldir, newgoal;

   if(anim.length())
      self.SetAnim(anim.c_str());

   // making it jump no more than 500 units
   goaldir = goal - self.worldorigin;
   dist    = goaldir.length();
   if(dist > 500)
   {
      goaldir.normalize();
      goaldir *= 500;
   }
   newgoal = goaldir + self.worldorigin;

   traveltime = self.JumpTo(newgoal, speed);
   endtime = traveltime + level.time;

   self.last_jump_time = endtime;

   state = 0;
}

// EOF

