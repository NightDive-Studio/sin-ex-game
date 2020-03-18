/*
================================================================
CHECKPOINTS
================================================================

Copyright (C) 2020 by Night Dive Studios, Inc.
All rights reserved.

See the license.txt file for conditions and terms of use for this code.
*/

#include "checkpoints.h"
#include "actor.h"

// defines for ammo maxes
#define ROCKET_MAX       100
#define BULLET_MAX       250
#define MINE_MAX         10
// hoverbike health
#define HOVER_MAX_HEALTH 250
// checkpoint spawnflags
#define ANYORDER         1
#define RANDREWARD       2
#define TURBOFILL        4
#define ALWAYSREWARD     8

/*****************************************************************************/
/*SINED func_checkpoint (0 .5 .8) ? ANYORDER RANDREWARD TURBOFILL ALWAYSREWARD
Each time a player passes through a checkpoint, he recieves a number of
frag points, as well as some hoverbike ammo. (rockets, bullets, or mines, randomly)
Each of these have default values, but may all be changed. For ammo, enter a value
of -1 for the player to never recieve that kind of ammo. Ammo amounts recieved are
doubled for going through a goal line.

ANYORDER is only settable on the goal line. When set, it allows players to hit the
checkpoints in the level in any order. They must still hit them all

RANDREWARD specifies that the checkpoint will give out only one type of ammo (choosen randomly)
when touched. Otherwise, a player will recieve some of each type of ammo.

TURBOFILL makes this checkpoint refill a hoverbike's turbo meter when passed.

ALWAYSREWARD always gives a player the ammo and health reward even if he's already hit it.

"id" is the id number of the checkpoint. Set to 0 for the goal line and 1 or greater for checkpoints.
This needs to be unique to each checkpoint. Max allowed value is 255.
"previous_id" is the id number of the checkpoint that comes before this one.
This does not need to be set if the ANYORDER flag is used.
"points" is the number of frag points recieved for passing this checkpoint.
   Default is 0 for checkpoints and 1 for goal lines.
"fastestpoints" is the number of points rewarded for making the fastest lap. Default is 1.
"rockets" is the amount of rocket ammo that the player may recieve. Default is 10.
"bullets" is the amount of bullet ammo that the player may recieve. Default is 25.
"mines" is the number of mines that the player may recieve. Default is 3.
"bikehealth" is the amount of health a hoverbike may recieve. Default is -1(don't give any).
"riderhealth" is the amount of health a hoverbike may recieve. Default is -1(don't give any).
"disabletime" sets how long after someone successfully hits this checkpoint that it will be disabled.
  No one can successfully hit the checkpoint while it's disabled. Default is 0, don't disable.
  If ALWAYSREWARD is set, default is 5.
"target" is activated when the checkpoint is hit successfully.
"thread" is the script thread that is called when the checkpoint is hit successfully.
/*****************************************************************************/

CLASS_DECLARATION(Trigger, CheckPoint, "func_checkpoint");

ResponseDef CheckPoint::Responses[] =
{
   { &EV_Trigger_Effect, (Response)&CheckPoint::CPTouch      },
   { &EV_Touch,          (Response)&CheckPoint::TriggerStuff },
   { &EV_Killed,         (Response)&CheckPoint::TriggerStuff },
   { &EV_Activate,       (Response)&CheckPoint::TriggerStuff },
   { NULL, NULL }
};

CheckPoint::CheckPoint() : Trigger()
{
   rockets     = G_GetIntArg("rockets", 10);
   bullets     = G_GetIntArg("bullets", 25);
   mines       = G_GetIntArg("mines", 3);
   bikehealth  = G_GetIntArg("bikehealth", -1);
   riderhealth = G_GetIntArg("riderhealth", -1);
   if(spawnflags & ALWAYSREWARD)
      disabletime = G_GetFloatArg("disabletime", 5);
   else
      disabletime = G_GetFloatArg("disabletime", 0);

   id = G_GetIntArg("id", 0);
   if(id < 0)
      id = 0;
   else if(id > 255)
      id = 255;

   if(id == 0) // it's a goal line
   {
      points = G_GetIntArg("points", 1);
      id = -1; // goal lines are 0 to the LD's, but -1 to the code
      fastestpoints = G_GetIntArg("fastestpoints", 1);

      // double the ammo rewards
      rockets *= 2;
      bullets *= 2;
      mines   *= 2;

      // init checkpoint system stuff
      level.fastest_lap = 5999.9; // init fastest lap to 99:59.9

      // this is used by client to display the fastest lap time
      gi.configstring(CS_CHECKPOINTS, "99:59.9");

      // set any order boolean if needed
      if(spawnflags & ANYORDER)
         level.cp_anyorder = true;

      // make sure that CP stuff is properly setup
      // for players spawned before this
      for(int i = 1; i <= maxclients->value; i++)
      {
         if(!g_edicts[i].inuse || !g_edicts[i].entity)
            continue;

         Player *player = (Player *)g_edicts[i].entity;
         player->last_cp_id = -1;
         player->last_goal_time = level.time;
      }
   }
   else
   {
      points = G_GetIntArg("points", 0);

      // add it to the level's checkpoint total
      level.cp_num++;
   }

   previous_id = G_GetIntArg("previous_id", 0);
   if(previous_id == 0)
      previous_id = -1;

   respondto = TRIGGER_PLAYERS | TRIGGER_HOVERBIKES;

   // get alias and cache the checkpoint sounds
   G_LoadAndExecScript(level.cp_sounds_script.c_str());
}

void CheckPoint::CPTouch(Event *ev)
{
   Entity    *ent;
   Player    *other;
   qboolean   successfullhit, givereward;
   float      awardmult;

   successfullhit = true;
   awardmult = 1;

   ent = ev->GetEntity(1);
   assert(ent);
   if(!ent->isSubclassOf<Player>())
      return;
   other = (Player *)ent;

   if(other->checkpoint_debounce > level.time)
   {
      if((other->last_cp_id == id) ||
         ((other->last_cp_id == 0) && (id != -1)))
      {
         return;
      }
   }

   // completely different touching checks done for anyorder levels
   if(level.cp_anyorder)
   {
      other->last_cp_id = id;

      if(id == -1)
      {
         // check if player has touched all the checkpoints
         if(other->CPTouchedAll())
         {
            // check if player got the fastest lap time
            float laptime = level.time - other->last_goal_time;

            if(other->last_goal_time > 0 && laptime < level.fastest_lap)
            {
               int flm;
               float fls;
               char opstring[10];

               other->RandomGlobalSound("fastestlap_sound", 1.0, CHAN_VOICE);
               level.fastest_lap = laptime;
               other->client->resp.score += fastestpoints;

               // update config string with new fastest lap time
               flm = floor(level.fastest_lap / 60);
               fls = level.fastest_lap - (float)flm*60.0;
               opstring[0] = 0;
               snprintf(opstring, sizeof(opstring), "%2i:%04.1f", flm, fls);
               gi.configstring(CS_CHECKPOINTS, opstring);
            }
            else // just play regular passing sound
            {
               other->RandomGlobalSound("gl_sound", 1.0, CHAN_VOICE);
            }

            other->last_goal_time = level.time;
            other->client->resp.last_lap_time = laptime;
            other->client->resp.score += points;
            other->CPListClear();
         }
         else
         {
            if(other->checkpoint_debounce < level.time)
               other->RandomGlobalSound("bad_cp_sound", 1.0, CHAN_VOICE);
            other->checkpoint_debounce = level.time + 5.0;
            successfullhit = false;
         }
      }
      else // touched a regular checkpoint
      {
         if(other->CPTouched(id))
         {
            //already touched this checkpoint this lap
            other->checkpoint_debounce = level.time + 5.0;
            successfullhit = false;
         }
         else
         {
            other->RandomGlobalSound("cp_sound", 1.0, CHAN_VOICE);
            // add this checkpoint to the player's list
            other->CPListAdd(id);
            other->client->resp.score += points;
         }
      }

      other->checkpoint_debounce = level.time + 5.0;
   }
   else
   {
      // check for player start his first lap
      if(other->last_cp_id == 0)
      {
         // start up the player if passing through the goal line
         if(id == -1)
         {
            other->RandomGlobalSound("startlap_sound", 1.0, CHAN_VOICE);
            other->last_cp_id = -1;
            other->last_goal_time = level.time;
         }
         else
         {
            if(other->checkpoint_debounce < level.time)
               other->RandomGlobalSound("bad_cp_sound", 1.0, CHAN_VOICE);
         }

         other->checkpoint_debounce = level.time + 1.0;
         successfullhit = false;
      }
      // make sure player touched correct previous checkpoint
      else if(other->last_cp_id != previous_id)
      {
         if(other->checkpoint_debounce < level.time)
            other->RandomGlobalSound("bad_cp_sound", 1.0, CHAN_VOICE);
         other->checkpoint_debounce = level.time + 1.0;
         successfullhit = false;
      }

      if(successfullhit)
      {
         other->client->resp.score += points;

         if(id == -1)
         {
            float laptime;

            // check if player got the fastest lap time
            laptime = level.time - other->last_goal_time;

            if(other->last_goal_time > 0 && laptime < level.fastest_lap)
            {
               int flm;
               float fls;
               char opstring[10];

               other->RandomGlobalSound("fastestlap_sound", 1.0, CHAN_VOICE);
               level.fastest_lap = laptime;
               other->client->resp.score += fastestpoints;

               // update config string with new fastest lap time
               flm = floor(level.fastest_lap / 60);
               fls = level.fastest_lap - (float)flm*60.0;
               opstring[0] = 0;
               snprintf(opstring, sizeof(opstring), "%2i:%04.1f", flm, fls);
               gi.configstring(CS_CHECKPOINTS, opstring);
            }
            else // just play regular passing sound
            {
               other->RandomGlobalSound("gl_sound", 1.0, CHAN_VOICE);
            }

            other->last_goal_time = level.time;
            other->client->resp.last_lap_time = laptime;

            other->CPListClear(); // reset CP counter
         }
         else
         {
            other->RandomGlobalSound("cp_sound", 1.0, CHAN_VOICE);
            other->CPListAdd(id); // decrement the CP counter
         }

         other->last_cp_id = id;
         other->client->resp.score += points;
         other->checkpoint_debounce = level.time + 5.0;
      }
   }

   // check reward disabler
   if(disabletime && offtimmer > level.time)
      givereward = false;
   else
      givereward = true;

   // check for doing a sound correction
   if(!successfullhit && (spawnflags & ALWAYSREWARD) && givereward)
      other->RandomGlobalSound("cp_sound", 1.0, CHAN_VOICE);

   // cut reward in half if not a successfull CP hit
   if(!successfullhit)
      awardmult = 0.5;

   // give hoverbike ammo reward
   if(givereward &&
      (successfullhit || (spawnflags & ALWAYSREWARD)) &&
      (rockets >= 0 ||
       bullets >= 0 ||
       mines >= 0 ||
       bikehealth >= 0 ||
       riderhealth >= 0 ||
       (spawnflags & TURBOFILL)))
   {
      if(other->GetHoverbike())
      {
         if(spawnflags & RANDREWARD)
         {
            while(1) // keep going till we give the player a reward
            {
               float tmpflt = G_Random();

               if(tmpflt < 0.1666)
               {
                  if(rockets >= 0)
                  {
                     other->GetHoverbike()->rockets += rockets*awardmult;
                     if(other->GetHoverbike()->rockets > ROCKET_MAX)
                        other->GetHoverbike()->rockets = ROCKET_MAX;
                     break;
                  }
               }
               else if(tmpflt < 0.3333)
               {
                  if(bullets >= 0)
                  {
                     other->GetHoverbike()->bullets += bullets*awardmult;
                     if(other->GetHoverbike()->bullets > BULLET_MAX)
                        other->GetHoverbike()->bullets = BULLET_MAX;
                     break;
                  }
               }
               else if(tmpflt < 0.4999)
               {
                  if(mines >= 0)
                  {
                     other->GetHoverbike()->mines += mines*awardmult;
                     if(other->GetHoverbike()->mines > MINE_MAX)
                        other->GetHoverbike()->mines = MINE_MAX;
                     break;
                  }
               }
               else if(tmpflt < 0.6666)
               {
                  if(bikehealth >= 0)
                  {
                     other->GetHoverbike()->health += bikehealth*awardmult;
                     if(other->GetHoverbike()->health > HOVER_MAX_HEALTH)
                        other->GetHoverbike()->health = HOVER_MAX_HEALTH;
                     break;
                  }
               }
               else if(tmpflt < 0.8333)
               {
                  if(riderhealth > 0)
                  {
                     Event *e;
                     float newhealth;

                     if(other->health < 100)
                     {
                        newhealth = other->health + riderhealth*awardmult;
                        if(newhealth > 100)
                           newhealth = 100;
                        e = new Event(EV_Sentient_GiveHealth);
                        e->AddFloat(newhealth);
                        other->ProcessEvent(e);;
                     }
                     break;
                  }
               }
               else
               {
                  if(spawnflags & TURBOFILL)
                  {
                     other->GetHoverbike()->turbo = 100 * awardmult;
                  }
               }
            }
         }
         else
         {
            if(rockets > 0)
            {
               other->GetHoverbike()->rockets += rockets*awardmult;
               if(other->GetHoverbike()->rockets > ROCKET_MAX)
                  other->GetHoverbike()->rockets = ROCKET_MAX;
            }

            if(bullets > 0)
            {
               other->GetHoverbike()->bullets += bullets*awardmult;
               if(other->GetHoverbike()->bullets > BULLET_MAX)
                  other->GetHoverbike()->bullets = BULLET_MAX;
            }

            if(mines > 0)
            {
               other->GetHoverbike()->mines += mines*awardmult;
               if(other->GetHoverbike()->mines > MINE_MAX)
                  other->GetHoverbike()->mines = MINE_MAX;
            }

            if(bikehealth > 0)
            {
               other->GetHoverbike()->health += bikehealth*awardmult;
               if(other->GetHoverbike()->health > HOVER_MAX_HEALTH)
                  other->GetHoverbike()->health = HOVER_MAX_HEALTH;
            }

            if(riderhealth > 0)
            {
               if(other->health < 100)
               {
                  float newhealth = other->health + riderhealth*awardmult;
                  if(newhealth > 100)
                     newhealth = 100;
                  auto e = new Event(EV_Sentient_GiveHealth);
                  e->AddFloat(newhealth);
                  other->ProcessEvent(e);;
               }
            }

            if(spawnflags & TURBOFILL)
               other->GetHoverbike()->turbo = 100 * awardmult;
         }
      }
      else if(riderhealth > 0)
      {
         if(other->health < 100)
         {
            float newhealth = other->health + riderhealth*awardmult;
            if(newhealth > 100)
               newhealth = 100;
            auto e = new Event(EV_Sentient_GiveHealth);
            e->AddFloat(newhealth);
            other->ProcessEvent(e);;
         }
      }
   }

   // check if we have to disable this checkpoint for a while
   // nover disable the goal line though.
   if(disabletime && id != -1)
      offtimmer = level.time + disabletime;
}

void CheckPoint::TriggerStuff(Event *ev)
{
   Entity   *other;
   qboolean  respond;

   // Don't bother with testing anything if we can't trigger yet
   if((level.time < trigger_time) || (trigger_time == -1))
      return;

   health = max_health;
   if(health && ((int)*ev != (int)EV_Killed) && ((int)*ev != (int)EV_Activate))
   {
      // if health is set, we only respond to killed and activate messages
      return;
   }

   other = ev->GetEntity(1);

   assert(other != this);

   if((respondto & TRIGGER_PLAYERS) && other->isClient())
   {
      // player has a hoverbike, but this trigger doesn't respond to them
      if(!(respondto & TRIGGER_HOVERBIKES) && ((Player *)other)->GetHoverbike())
         respond = 0;
      else
         respond = 1;
   }
   else
   {
      respond = (((respondto & TRIGGER_MONSTERS)    && other->isSubclassOf<Actor>()) ||
                 ((respondto & TRIGGER_PROJECTILES) && other->isSubclassOf<Projectile>()));
   }

   // Always respond to activate messages from the world since they're probably from 
   // the "trigger" command
   if(!respond && !((other == world) && ((int)*ev == (int)EV_Activate)))
      return;

   if(key.length())
   {
      if(!other->isSubclassOf<Sentient>() || !(((Sentient *)other)->HasItem(key.c_str())))
      {
         const ClassDef *cls = getClass(key.c_str());
         if(!cls)
         {
            gi.dprintf("No item named '%s'\n", key.c_str());
            return;
         }

         Item *item = (Item *)cls->newInstance();
         item->CancelEventsOfType(EV_Item_DropToFloor);
         item->CancelEventsOfType(EV_Remove);
         item->ProcessPendingEvents();

         str dialog(item->GetDialogNeeded());
         if(dialog.length() > 1)
         {
            if(!ExecuteThread(dialog))
               warning("TriggerStuff", "Null game script");
         }
         else
         {
            gi.centerprintf(other->edict, "jcx yv 20 string \"You need this item:\" jcx yv -20 icon %d", item->GetIconIndex());
         }

         delete item;
         return;
      }
   }

   trigger_time = level.time + wait;

   auto event = new Event(EV_Trigger_Effect);
   event->AddEntity(other);
   PostEvent(event, delay);
}

// EOF

