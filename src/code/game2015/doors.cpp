//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/doors.cpp                        $
// $Revision:: 71                                                             $
//   $Author:: Markd                                                          $
//     $Date:: 11/16/98 9:48p                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Doors are environment objects that rotate open when activated by triggers
// or when used by the player.
// 

#include "g_local.h"
#include "entity.h"
#include "trigger.h"
#include "mover.h"
#include "doors.h"
#include "sentient.h"
#include "areaportal.h"
#include "scriptmaster.h"
#include "item.h"
#include "actor.h"
#include "player.h"

Event EV_Door_TriggerFieldTouched("door_triggerfield");
Event EV_Door_TryOpen("tryToOpen");
Event EV_Door_Close("close");
Event EV_Door_Open("open");
Event EV_Door_DoClose("doclose");
Event EV_Door_DoOpen("doopen");
Event EV_Door_CloseEnd("doorclosed");
Event EV_Door_OpenEnd("dooropened");
Event EV_Door_Fire("toggledoor");
Event EV_Door_Link("linkdoor");
Event EV_Door_SetTime("time");
Event EV_Door_Lock("lock");
Event EV_Door_Unlock("unlock");
Event EV_Door_RespondTo("respondto"); //### 8 player, 16 player, 24 both

CLASS_DECLARATION(ScriptSlave, Door, "NormalDoor");

#define DOOR_START_OPEN			1
#define DOOR_OPEN_DIRECTION	2
#define DOOR_DONT_LINK			4
#define DOOR_TOGGLE				32
#define DOOR_AUTO_OPEN			64
#define DOOR_TARGETED 			128

#define STATE_OPEN		1
#define STATE_OPENING	2
#define STATE_CLOSING   3
#define STATE_CLOSED		4

/*

Doors are similar to buttons, but can spawn a fat trigger field around them
to open without a touch, and they link together to form simultanious
double/quad doors.
 
Door.master is the master door.  If there is only one door, it points to itself.
If multiple doors, all will point to a single one.

Door.enemy chains from the master door through all doors linked in the chain.

*/

ResponseDef Door::Responses[] =
{
   { &EV_Door_TriggerFieldTouched,  ( Response )&Door::FieldTouched },
   { &EV_Trigger_Effect,			   ( Response )&Door::TryOpen },
   { &EV_Activate,			         ( Response )&Door::TryOpen },
   { &EV_Door_TryOpen,	            ( Response )&Door::TryOpen },
   { &EV_Door_Close,		            ( Response )&Door::Close },
   { &EV_Door_Open,		            ( Response )&Door::Open },
   { &EV_Door_CloseEnd,	            ( Response )&Door::CloseEnd },
   { &EV_Door_OpenEnd,	            ( Response )&Door::OpenEnd },
   { &EV_Door_Fire,		            ( Response )&Door::DoorFire },
   { &EV_Door_Link,		            ( Response )&Door::LinkDoors },
   { &EV_Door_SetTime,	            ( Response )&Door::SetTime },
   { &EV_Use,							   ( Response )&Door::DoorUse },
   { &EV_PreciseUse,					   ( Response )&Door::DoorUse }, //###
   { &EV_Killed,						   ( Response )&Door::DoorFire },
   { &EV_Blocked,						   ( Response )&Door::DoorBlocked },
   { &EV_Door_Lock,		            ( Response )&Door::LockDoor },
   { &EV_Door_Unlock,		         ( Response )&Door::UnlockDoor },
   { &EV_Touch,						   NULL },
   { &EV_Door_RespondTo,		      ( Response )&Door::RespondTo },
   { NULL, NULL }
};

Door::Door() : ScriptSlave()
{
   float t;
   str   tmpstr; //###

   master = this;

   showModel();

   sound_stop    = G_GetStringArg("sound_stop", gi.GlobalAlias_FindRandom("door_stop"));
   sound_move    = G_GetStringArg("sound_move", gi.GlobalAlias_FindRandom("door_moving"));
   sound_message = G_GetStringArg("sound_message");
   sound_locked  = G_GetStringArg("sound_locked");
   //###
   tmpstr = G_GetStringArg("sound_close_stop");
   if(tmpstr.length() > 1)
      sound_close_stop = tmpstr;
   else
      sound_close_stop = sound_stop;

   tmpstr = G_GetStringArg("sound_close_move");
   if(tmpstr.length() > 1)
      sound_close_move = tmpstr;
   else
      sound_close_move = sound_move;

   gi.soundindex(sound_close_stop.c_str());
   gi.soundindex(sound_close_move.c_str());
   //###

   gi.soundindex(sound_stop.c_str());
   gi.soundindex(sound_move.c_str());
   if(sound_message.length() > 1)
      gi.soundindex(sound_message.c_str());

   //###
   t = G_GetIntArg("preciseuse", 0);
   if(t)
      preciseopen = true;
   else
      preciseopen = false;

   trigdist = G_GetFloatArg("triggerdist", 60);
   // put a min dist cap on it
   if(trigdist < 1)
      trigdist = 1;
   //###

   health		= G_GetFloatArg("health");
   max_health	= health;

   traveltime = G_GetFloatArg("time", 0.3);

   if(traveltime < FRAMETIME)
   {
      traveltime = FRAMETIME;
   }

   speed = 1.0f / traveltime;
   wait	= G_GetFloatArg("wait", 3);

   if((spawnflags & DOOR_TOGGLE) && (wait == 3))
      wait = 0;

   dmg = G_GetIntArg("dmg", 0);

   setSolidType(SOLID_BSP);
   setMoveType(MOVETYPE_PUSH);

   setSize(mins, maxs);

   dir = G_GetMovedir();
   t = dir[0];
   dir[0] = -dir[1];
   dir[1] = t;

   setOrigin(origin);
   doormin = absmin;
   doormax = absmax;

   // DOOR_START_OPEN is to allow an entity to be lighted in the closed position
   // but spawn in the open position
   if(spawnflags & DOOR_START_OPEN)
   {
      state = STATE_OPEN;
      PostEvent(EV_Door_Open, 0.05f);
   }
   else
   {
      state = STATE_CLOSED;
   }
   previous_state = state;

   if(health)
   {
      takedamage = DAMAGE_YES;
   }

   // LinkDoors can't be done until all of the doors have been spawned, so
   // the sizes can be detected properly.
   nextdoor = 0;
   PostEvent(EV_Door_Link, 0);

   // Default to work with monsters and players 
   respondto = TRIGGER_PLAYERS | TRIGGER_MONSTERS;
   if(spawnflags & 8)
   {
      respondto &= ~TRIGGER_PLAYERS;
   }
   if(spawnflags & 16)
   {
      respondto &= ~TRIGGER_MONSTERS;
   }
}

qboolean Door::isOpen() const
{
   return (state == STATE_OPEN);
}

void Door::OpenEnd(Event *ev)
{
   //	if ( master == this )
   {
      if(sound_stop.length() > 1)
      {
         ProcessEvent(EV_DoorSound);
         sound(sound_stop, 1, CHAN_VOICE + CHAN_NO_PHS_ADD, ATTN_NORM);
      }
      else
      {
         RandomGlobalSound("null_sound", 1, CHAN_VOICE + CHAN_NO_PHS_ADD, ATTN_NORM);
      }
   }
   previous_state = state;
   state = STATE_OPEN;
   if(spawnflags & DOOR_TOGGLE)
   {
      // don't close automatically
      return;
   }

   if((wait > 0) && (master == this))
   {
      PostEvent(EV_Door_Close, wait);
   }
}

void Door::CloseEnd(Event *ev)
{
   //	if ( master == this )
   {
      //###
      // if ( sound_stop.length() > 1 )
      if(sound_close_stop.length() > 1)
      {
         ProcessEvent(EV_DoorSound);
         //sound( sound_stop, 1, CHAN_VOICE + CHAN_NO_PHS_ADD, ATTN_NORM );
         sound(sound_close_stop, 1, CHAN_VOICE + CHAN_NO_PHS_ADD, ATTN_NORM);
      }
      //###
      else
      {
         RandomGlobalSound("null_sound", 1, CHAN_VOICE + CHAN_NO_PHS_ADD, ATTN_NORM);
      }
   }

   SetAreaPortals(Target(), false);

   previous_state = state;
   state = STATE_CLOSED;
}

void Door::Close(Event *ev)
{
   Door *door;

   CancelEventsOfType(EV_Door_Close);

   previous_state = state;
   state = STATE_CLOSING;

   ProcessEvent(EV_Door_DoClose);

   //###
   //if ( sound_move.length() > 1 )
   if(sound_close_move.length() > 1)
   {
      ProcessEvent(EV_DoorSound);
      //sound( sound_move, 1, CHAN_VOICE + CHAN_NO_PHS_ADD, ATTN_NORM );
      sound(sound_close_move, 1, CHAN_VOICE + CHAN_NO_PHS_ADD, ATTN_NORM);
   }
   //###
   if(master == this)
   {
      if(max_health)
      {
         takedamage	= DAMAGE_YES;
         health		= max_health;
      }

      // trigger all paired doors
      door = (Door *)G_GetEntity(nextdoor);
      assert(door->isSubclassOf<Door>());
      while(door && (door != this))
      {
         door->ProcessEvent(EV_Door_Close);
         door = (Door *)G_GetEntity(door->nextdoor);
         assert(door->isSubclassOf<Door>());
      }
   }
}

void Door::Open(Event *ev)
{
   Door *door;
   Event *e;
   Entity *other;

   if(ev->NumArgs() < 1)
   {
      ev->Error("No entity specified to open door.  Door may open the wrong way.");
      other = world;
   }
   else
   {
      other = ev->GetEntity(1);
   }

   if(state == STATE_OPENING)
   {
      // already going up
      return;
   }

   if(state == STATE_OPEN)
   {
      // reset top wait time
      if(wait > 0)
      {
         CancelEventsOfType(EV_Door_Close);
         PostEvent(EV_Door_Close, wait);
      }
      return;
   }

   previous_state = state;
   state = STATE_OPENING;

   e = new Event(EV_Door_DoOpen);
   e->AddEntity(other);
   ProcessEvent(e);

   if(sound_move.length() > 1)
   {
      ProcessEvent(EV_DoorSound);
      sound(sound_move, 1, CHAN_VOICE + CHAN_NO_PHS_ADD, ATTN_NORM);
   }
   if(master == this)
   {
      // trigger all paired doors
      door = (Door *)G_GetEntity(nextdoor);
      assert(door->isSubclassOf<Door>());
      while(door && (door != this))
      {
         e = new Event(EV_Door_Open);
         e->AddEntity(other);
         door->ProcessEvent(e);
         door = (Door *)G_GetEntity(door->nextdoor);
         assert(door->isSubclassOf<Door>());
      }

      SetAreaPortals(Target(), true);
   }
}

void Door::DoorUse(Event *ev)
{
   Entity *other;
   qboolean respond;
   Event *e;

   //###
   // added precise use opening to doors
   if(preciseopen)
   {
      str eventname(ev->getName());
      if(!Q_strcasecmp(eventname.c_str(), "doUse"))
         return;
   }
   //###

   other = ev->GetEntity(1);

   respond = (((respondto & TRIGGER_PLAYERS) && other->isClient()) ||
              ((respondto & TRIGGER_MONSTERS) && other->isSubclassOf<Actor>()));

   if(!respond)
      return;

   // only allow use when not triggerd by other events
   if(health || (spawnflags & (DOOR_AUTO_OPEN | DOOR_TARGETED)))
   {
      if(other->isSubclassOf<Sentient>() && (state == STATE_CLOSED))
      {
         if(health)
         {
            gi.centerprintf(other->edict, "jcx yv 20 string \"This door is jammed.\"");
         }
         else if(spawnflags & DOOR_TARGETED)
         {
            RandomGlobalSound("door_triggered", 1, CHAN_VOICE + CHAN_NO_PHS_ADD, ATTN_NORM);
            //gi.centerprintf ( other->edict, "jcx yv 20 string \"This door opens elsewhere.\"" );
         }
      }
      return;
   }

   assert(master);
   if(!master)
   {
      // bulletproofing
      master = this;
   }

   if(master->state == STATE_CLOSED)
   {
      e = new Event(EV_Door_TryOpen);
      e->AddEntity(other);
      master->ProcessEvent(e);
   }
   else if(master->state == STATE_OPEN)
   {
      e = new Event(EV_Door_Close);
      e->AddEntity(other);
      master->ProcessEvent(e);
   }
}

void Door::DoorFire(Event *ev)
{
   Event *e;
   Entity *other;

   other = ev->GetEntity(1);

   assert(master == this);
   if(master != this)
   {
      gi.error("DoorFire: master != self");
   }

   // no more messages
   SetMessage(NULL);

   // reset health in case we were damage triggered
   health = max_health;

   // will be reset upon return
   takedamage = DAMAGE_NO;

   if((spawnflags & (DOOR_TOGGLE | DOOR_START_OPEN)) && (state == STATE_OPENING || state == STATE_OPEN))
   {
      spawnflags &= ~DOOR_START_OPEN;
      ProcessEvent(EV_Door_Close);
   }
   else
   {
      e = new Event(EV_Door_Open);
      e->AddEntity(other);
      ProcessEvent(e);
   }
}

void Door::DoorBlocked(Event *ev)
{
   Event *e;
   Entity *other;

   assert(master);
   if((master) && (master != this))
   {
      master->ProcessEvent(ev);
      return;
   }

   if(lastblocktime > level.time)
   {
      return;
   }

   lastblocktime = level.time + 0.3;

   other = ev->GetEntity(1);

   if(dmg)
   {
      other->Damage(this, this, (int)dmg, worldorigin, vec_zero, vec_zero, 0, 0, MOD_CRUSH, -1, -1, 1.0f);
   }

   // 
   // if we killed him, lets keep on going
   //
   if(other->health <= 0)
   {
      return;
   }

   if(state == STATE_OPENING || state == STATE_OPEN)
   {
      spawnflags &= ~DOOR_START_OPEN;
      ProcessEvent(EV_Door_Close);
   }
   else
   {
      e = new Event(EV_Door_Open);
      e->AddEntity(other);
      ProcessEvent(e);
   }
}

void Door::FieldTouched(Event *ev)
{
   Entity *other;

   other = ev->GetEntity(1);

#ifdef SIN_DEMO
   if((state != STATE_OPEN) && !(spawnflags & DOOR_AUTO_OPEN) &&
      (!other || !other->isSubclassOf(Actor)))
#else
   if((state != STATE_OPEN) && !(spawnflags & DOOR_AUTO_OPEN))
#endif
   {
      return;
   }

   TryOpen(ev);
}

qboolean Door::CanBeOpenedBy(Entity *ent) const
{
   assert(master);
   if((master) && (master != this))
   {
      return master->CanBeOpenedBy(ent);
   }

   if(!locked && !key.length())
   {
      return true;
   }

   if(ent && ent->isSubclassOf<Sentient>() && static_cast<Sentient *>(ent)->HasItem(key.c_str()))
   {
      return true;
   }

   return false;
}

void Door::TryOpen(Event *ev)
{
   Entity *other;
   Event *event;

   //FIXME
   // hack so that doors aren't triggered by guys when game starts.
   // have to fix delay that guys go through before setting up their threads
   if(level.time < 0.4)
   {
      return;
   }

   other = ev->GetEntity(1);

   assert(master);
   if(master && (this != master))
   {
      event = new Event(EV_Door_TryOpen);
      event->AddEntity(other);
      master->ProcessEvent(event);
      return;
   }

   if(!other || (other->health <= 0))
   {
      return;
   }

   if(locked)
   {
      if(sound_locked.length() > 1)
      {
         other->sound(sound_locked.c_str(), 1, CHAN_VOICE);
      }
      else if(other->isSubclassOf<Player>())
      {
         other->RandomSound("snd_locked", 1, CHAN_VOICE);
         //gi.centerprintf ( other->edict, "jcx yv 20 string \"This door is locked.\"" );
      }

      // locked doors don't open for anyone
      return;
   }

   if(!CanBeOpenedBy(other))
   {
      Item           *item;
      const ClassDef *cls;

      if(other->isClient())
      {
         cls = getClass(key.c_str());
         if(!cls)
         {
            gi.dprintf("No item named '%s'\n", key.c_str());
            return;
         }
         item = (Item *)cls->newInstance();
         item->CancelEventsOfType(EV_Item_DropToFloor);
         item->CancelEventsOfType(EV_Remove);
         item->ProcessPendingEvents();
         gi.centerprintf(other->edict, "jcx yv 20 string \"You need this item:\" jcx yv -20 icon %d", item->GetIconIndex());
         delete item;
      }
      return;
   }

   // once we're opened by an item, we no longer need that item to open the door
   key = "";

   if(Message().length())
   {
      gi.centerprintf(other->edict, "jcx jcy string \"%s\"", Message().c_str());
      sound(sound_message, 1, CHAN_VOICE + CHAN_NO_PHS_ADD, ATTN_NORM);
   }

   event = new Event(EV_Door_Fire);
   event->AddEntity(other);
   ProcessEvent(event);
}

void Door::SpawnTriggerField(Vector fmins, Vector fmaxs)
{
   TouchField *trig;
   Vector min;
   Vector max;

   //### added variable trigger distances
   //min = fmins - "60 60 8";
   //max = fmaxs + "60 60 8";
   min = fmins - Vector(trigdist, trigdist, trigdist*0.1333);
   max = fmaxs + Vector(trigdist, trigdist, trigdist*0.1333);
   //###

   trig = new TouchField();
   trig->Setup(this, EV_Door_TriggerFieldTouched, min, max, respondto);

   trigger = trig->entnum;
}

EXPORT_FROM_DLL qboolean Door::DoorTouches(Door *e1)
{
   if(e1->doormin.x > doormax.x)
   {
      return false;
   }
   if(e1->doormin.y > doormax.y)
   {
      return false;
   }
   if(e1->doormin.z > doormax.z)
   {
      return false;
   }
   if(e1->doormax.x < doormin.x)
   {
      return false;
   }
   if(e1->doormax.y < doormin.y)
   {
      return false;
   }
   if(e1->doormax.z < doormin.z)
   {
      return false;
   }

   return true;
}

void Door::LinkDoors(Event *ev)
{
   Door	*ent;
   Door	*next;
   Vector			cmins;
   Vector			cmaxs;
   int				t;
   int				i;

   if(nextdoor)
   {
      // already linked by another door
      return;
   }

   // master doors own themselves
   master = this;

   if(spawnflags & DOOR_DONT_LINK)
   {
      // don't want to link this door
      nextdoor = entnum;
      return;
   }

   cmins = absmin;
   cmaxs = absmax;

   ent = this;
   for(t = entnum; t != 0; t = G_FindClass(t, getClassID()))
   {
      next = (Door *)G_GetEntity(t);
      if(!ent->DoorTouches(next))
      {
         continue;
      }

      if(next->nextdoor)
      {
         error("cross connected doors.  Targetname = %s entity %d", TargetName(), entnum);
      }

      ent->nextdoor = next->entnum;
      ent = next;

      for(i = 0; i < 3; i++)
      {
         if(ent->absmin[i] < cmins[i])
         {
            cmins[i] = ent->absmin[i];
         }
         if(ent->absmax[i] > cmaxs[i])
         {
            cmaxs[i] = ent->absmax[i];
         }
      }

      // set master door
      ent->master = this;

      if(ent->health)
      {
         health = ent->health;
      }

      if(ent->Targeted())
      {
         if(!Targeted())
         {
            SetTargetName(ent->TargetName());
         }
         else if(strcmp(TargetName(), ent->TargetName()))
         {
            // not a critical error, but let them know about it.
            gi.dprintf("cross connected doors");

            ent->SetTargetName(TargetName());
         }
      }

      if(ent->Message().length())
      {
         if(Message().length() && !strcmp(Message().c_str(), ent->Message().c_str()))
         {
            // not a critical error, but let them know about it.
            gi.dprintf("Different messages on linked doors.  Targetname = %s", TargetName());
         }

         // only master should have a message
         SetMessage(ent->Message().c_str());
         ent->SetMessage(NULL);
      }
   }

   // make the chain a loop
   ent->nextdoor = entnum;

   // open up any portals we control
   SetAreaPortals(Target(), (spawnflags & DOOR_START_OPEN) ? true : false);

   // shootable or targeted doors don't need a trigger
   if(health || (spawnflags & DOOR_TARGETED))
   {
      // Don't let the player trigger the door
      return;
   }

   // Don't spawn trigger field when set to toggle
   if(!(spawnflags & DOOR_TOGGLE))
   {
      SpawnTriggerField(cmins, cmaxs);
   }
}

void Door::SetTime(Event *ev)
{
   traveltime = ev->GetFloat(1);
   if(traveltime < FRAMETIME)
   {
      traveltime = FRAMETIME;
   }

   speed = 1.0f / traveltime;
}

void Door::LockDoor(Event *ev)
{
   locked = true;
}

void Door::UnlockDoor(Event *ev)
{
   locked = false;
}

//### added for claw door in underground
void Door::RespondTo(Event *ev)
{
   if(ev->NumArgs() < 1)
      return;
   respondto = ev->GetInteger(1);
}
//###

//### added one way opening thing & precise opening & closing sounds
/*****************************************************************************/
/*SINED func_rotatingdoor (0 .5 .8) ? START_OPEN OPEN_DIRECTION DOOR_DONT_LINK NOT_PLAYERS NOT_MONSTERS TOGGLE AUTO_OPEN TARGETED
if two doors touch, they are assumed to be connected and operate as a unit.

TOGGLE causes the door to wait in both the start and end states for a trigger event.
DOOR_DONT_LINK is for when you have two doors that are touching but you want to operate independently.

START_OPEN causes the door to move to its destination when spawned, and operate in reverse.  It is used to temporarily or permanently close off an area when triggered (not usefull for touch or takedamage doors).
OPEN_DIRECTION indicates which direction to open when START_OPEN is set.
AUTO_OPEN causes the door to open when a player is near instead of waiting for the player to use the door.
TARGETED door is only operational from triggers or script

"message"		is printed when the door is touched if it is a trigger door and it hasn't been fired yet
"openangle"    how wide to open the door
"angle"			determines the opening direction.  point toward the middle of the door (away from the hinge)
"targetname"	if set, no touch field will be spawned and a remote button or trigger field activates the door.
"health"			if set, door must be shot open
"time"			move time (0.3 default)
"wait"			wait before returning (3 default, -1 = never return)
"dmg"				damage to inflict when blocked (0 default)
"key"          The item needed to open this door (default nothing)
"onewayangles" the angles to rotate through when opening. Setting this makes the door only open in one direction.
"preciseuse" make the door only open on a precise use when set to a non-zero value.
"triggerdist" sets the distance out from the door that the trigger field will be made. Default is 60.

"sound_stop"		Specify the sound that plays when the door stops moving (default global door_stop)
"sound_move"		Specify the sound that plays when the door opens or closes (default global door_moving)
"sound_message"	Specify the sound that plays when the door displays a message
"sound_locked"	   Specify the sound that plays when the door is locked
"sound_close_stop"	Specify the sound that plays when the door stops closing (default global door_stop)
"sound_close_move"	Specify the sound that plays when the door closes (default global door_moving)

/*****************************************************************************/
CLASS_DECLARATION(Door, RotatingDoor, "func_rotatingdoor");

ResponseDef RotatingDoor::Responses[] =
{
   { &EV_Door_DoClose, (Response)&RotatingDoor::DoClose },
   { &EV_Door_DoOpen,  (Response)&RotatingDoor::DoOpen },
   { NULL, NULL }
};

void RotatingDoor::DoOpen(Event *ev)
{
   Vector ang;

   if(previous_state == STATE_CLOSED)
   {
      if(ev->NumArgs() > 0)
      {
         Entity *other;
         Vector p;

         other = ev->GetEntity(1);
         p = other->worldorigin - worldorigin;
         p.z = 0;
         diropened = dir * p;
      }
      else
      {
         diropened = 0 - init_door_direction;
      }
   }

   //###
   if(onewayangles != vec_zero)
   {
      ang = startangle + onewayangles;
   }
   else if(diropened < 0)
   //###
   {
      ang = startangle + Vector(0, angle, 0);
   }
   else
   {
      ang = startangle - Vector(0, angle, 0);
   }

   MoveTo(worldorigin, ang, fabs(speed*angle), EV_Door_OpenEnd);
}

void RotatingDoor::DoClose(Event *ev)
{
   MoveTo(worldorigin, startangle, fabs(speed*angle), EV_Door_CloseEnd);
}

RotatingDoor::RotatingDoor() : Door()
{
   startangle = angles;

   angle = G_GetFloatArg("openangle", 90);

   onewayangles = G_GetVectorArg("onewayangles", vec_zero); //###

   init_door_direction = (spawnflags & DOOR_OPEN_DIRECTION);
}

//### added precise opening & closing sounds
/*
/*****************************************************************************/
/*SINED func_door (0 .5 .8) ? START_OPEN x DOOR_DONT_LINK NOT_PLAYERS NOT_MONSTERS TOGGLE AUTO_OPEN TARGETED
if two doors touch, they are assumed to be connected and operate as a unit.

TOGGLE causes the door to wait in both the start and end states for a trigger event.
DOOR_DONT_LINK is for when you have two doors that are touching but you want to operate independently.

START_OPEN causes the door to move to its destination when spawned, and operate in reverse.  It is used to temporarily or permanently close off an area when triggered (not usefull for touch or takedamage doors).
OPEN_DIRECTION indicates which direction to open when START_OPEN is set.
AUTO_OPEN causes the door to open when a player is near instead of waiting for the player to use the door.
TARGETED door is only operational from triggers or script

"message"		is printed when the door is touched if it is a trigger door and it hasn't been fired yet
"angle"			determines the opening direction.  point toward the middle of the door (away from the hinge)
"targetname"	if set, no touch field will be spawned and a remote button or trigger field activates the door.
"health"			if set, door must be shot open
"speed"			move speed (100 default)
"time"			move time (1/speed default, overides speed)
"wait"			wait before returning (3 default, -1 = never return)
"lip"				lip remaining at end of move (8 default)
"dmg"				damage to inflict when blocked (0 default)
"key"          The item needed to open this door (default nothing)
"preciseuse" make the door only open on a precise use when set to a non-zero value.
"triggerdist" sets the distance out from the door that the trigger field will be made. Default is 60.

"sound_stop"		Specify the sound that plays when the door stops moving (default global door_stop)
"sound_move"		Specify the sound that plays when the door opens or closes (default global door_moving)
"sound_message"	Specify the sound that plays when the door displays a message
"sound_locked"	   Specify the sound that plays when the door is locked
"sound_close_stop"	Specify the sound that plays when the door stops closing (default global door_stop)
"sound_close_move"	Specify the sound that plays when the door closes (default global door_moving)

/*****************************************************************************/
CLASS_DECLARATION(Door, SlidingDoor, "func_door");

ResponseDef SlidingDoor::Responses[] =
{
   { &EV_Door_DoClose, (Response)&SlidingDoor::DoClose },
   { &EV_Door_DoOpen,  (Response)&SlidingDoor::DoOpen },
   { NULL, NULL }
};

void SlidingDoor::DoOpen(Event *ev)
{
   MoveTo(pos2, angles, speed*totalmove, EV_Door_OpenEnd);
}

void SlidingDoor::DoClose(Event *ev)
{
   MoveTo(pos1, angles, speed*totalmove, EV_Door_CloseEnd);
}

SlidingDoor::SlidingDoor() : Door()
{
   Vector movedir;
   float sp;

   lip = G_GetFloatArg("lip", 8);

   movedir = G_GetMovedir();
   totalmove = fabs(movedir * size) - lip;
   pos1 = worldorigin;
   pos2 = pos1 + movedir * totalmove;
   setOrigin(pos1);

   sp = G_GetFloatArg("speed", 0);
   if(sp)
   {
      speed = sp / totalmove;
   }
}

//### added precise opening & closing sounds
/*
/*****************************************************************************/
/*SINED func_scriptdoor (0 .5 .8) ? START_OPEN x DOOR_DONT_LINK NOT_PLAYERS NOT_MONSTERS TOGGLE AUTO_OPEN TARGETED
if two doors touch, they are assumed to be connected and operate as a unit.

TOGGLE causes the door to wait in both the start and end states for a trigger event.
DOOR_DONT_LINK is for when you have two doors that are touching but you want to operate independently.

START_OPEN causes the door to move to its destination when spawned, and operate in reverse.  It is used to temporarily or permanently close off an area when triggered (not usefull for touch or takedamage doors).
OPEN_DIRECTION indicates which direction to open when START_OPEN is set.
AUTO_OPEN causes the door to open when a player is near instead of waiting for the player to use the door.
TARGETED door is only operational from triggers or script

"message"		is printed when the door is touched if it is a trigger door and it hasn't been fired yet
"angle"			determines the opening direction.  point toward the middle of the door (away from the hinge)
"targetname"	if set, no touch field will be spawned and a remote button or trigger field activates the door.
"health"			if set, door must be shot open
"speed"			move speed (100 default)
"time"			move time (1/speed default, overides speed)
"wait"			wait before returning (3 default, -1 = never return)
"dmg"				damage to inflict when blocked (0 default)
"key"          The item needed to open this door (default nothing)
"initthread"   code to execute to setup the door (optional)
"openthread"   code to execute when opening the door (required)
"closethread"  code to execute when closing the door (required)
"preciseuse" make the door only open on a precise use when set to a non-zero value.
"triggerdist" sets the distance out from the door that the trigger field will be made. Default is 60.

"sound_stop"		Specify the sound that plays when the door stops moving (default global door_stop)
"sound_move"		Specify the sound that plays when the door opens or closes (default global door_moving)
"sound_message"	Specify the sound that plays when the door displays a message
"sound_locked"	   Specify the sound that plays when the door is locked
"sound_close_stop"	Specify the sound that plays when the door stops closing (default global door_stop)
"sound_close_move"	Specify the sound that plays when the door closes (default global door_moving)

/*****************************************************************************/
CLASS_DECLARATION(Door, ScriptDoor, "func_scriptdoor");

Event EV_ScriptDoor_DoInit("doinit");
Event EV_ScriptDoor_SetOpenThread("openthread");
Event EV_ScriptDoor_SetCloseThread("closethread");

ResponseDef ScriptDoor::Responses[] =
{
   { &EV_ScriptDoor_DoInit,         ( Response )&ScriptDoor::DoInit },
   { &EV_Door_DoClose,              ( Response )&ScriptDoor::DoClose },
   { &EV_Door_DoOpen,               ( Response )&ScriptDoor::DoOpen },
   { &EV_ScriptDoor_SetOpenThread,  ( Response )&ScriptDoor::SetOpenThread },
   { &EV_ScriptDoor_SetCloseThread, ( Response )&ScriptDoor::SetCloseThread },
   { NULL, NULL }
};

void ScriptDoor::SetOpenThread(Event *ev)
{
   openthreadname = ev->GetString(1);
}

void ScriptDoor::SetCloseThread(Event *ev)
{
   closethreadname = ev->GetString(1);
}

void ScriptDoor::DoInit(Event *ev)
{
   const char * label = NULL;
   GameScript * s;
   const char * tname;

   s = ScriptLib.GetScript(ScriptLib.GetGameScript());

   if(!s)
   {
      warning("DoInit", "Null game script");
      return;
   }

   if(initthreadname.length())
      label = initthreadname.c_str();

   doorthread = Director.CreateThread(s, label, MODEL_SCRIPT);
   if(!doorthread)
   {
      warning("DoInit", "Could not allocate thread.");
      return;
   }
   doorthread->Vars()->SetVariable("self", this);
   tname = TargetName();
   if(tname && tname[0])
   {
      str name;
      name = "$" + str(tname);
      doorthread->Vars()->SetVariable("targetname", name.c_str());
   }
   doorthread->Vars()->SetVariable("startorigin", startorigin);
   doorthread->Vars()->SetVariable("startangles", startangle);
   doorthread->Vars()->SetVariable("movedir", movedir);
   doorthread->Vars()->SetVariable("doorsize", doorsize);
   if(initthreadname.length())
   {
      // start right away
      doorthread->Start(-1);
   }
}

void ScriptDoor::DoOpen(Event *ev)
{
   if(!doorthread)
   {
      warning("DoOpen", "No Thread allocated.");
      return;
   }
   else
   {
      if(!doorthread->Goto(openthreadname.c_str()))
      {
         warning("DoOpen", "Could not goto %s", openthreadname.c_str());
         return;
      }
   }

   if(previous_state == STATE_CLOSED)
   {
      diropened = 0;
      if(ev->NumArgs() > 0)
      {
         Entity *other;
         Vector p;

         other = ev->GetEntity(1);
         p = other->worldorigin - worldorigin;
         p.z = 0;
         diropened = dir * p;
      }
   }
   doorthread->Vars()->SetVariable("origin", worldorigin);
   doorthread->Vars()->SetVariable("opendot", diropened);
   doorthread->Start(0);
}

void ScriptDoor::DoClose(Event *ev)
{
   if(!doorthread)
   {
      warning("DoClose", "No Thread allocated.");
      return;
   }
   else
   {
      if(!doorthread->Goto(closethreadname.c_str()))
      {
         warning("DoOpen", "Could not goto %s", closethreadname.c_str());
      }
   }
   doorthread->Vars()->SetVariable("origin", worldorigin);
   doorthread->Start(0);
}

ScriptDoor::ScriptDoor() : Door()
{
   const char * text;

   startangle = angles;
   //
   // see if we have an initthread
   //
   text = G_GetSpawnArg("initthread");
   if(text)
      initthreadname = text;

   //
   // see if we have an openthread
   //
   text = G_GetSpawnArg("openthread");
   if(text)
      openthreadname = text;
   else
      warning("ScriptDoor", "No openthread defined for door at %.2f %.2f %.2f", origin[0], origin[1], origin[2]);

   //
   // see if we have an closethread
   //
   text = G_GetSpawnArg("closethread");
   if(text)
      closethreadname = text;
   else
      warning("ScriptDoor", "No closethread defined for door at %.2f %.2f %.2f", origin[0], origin[1], origin[2]);
   //
   // clear out the sounds if necessary
   // scripted doors typically have their own sounds
   //
   text = G_GetSpawnArg("sound_stop");
   if(!text)
      sound_stop = "";
   text = G_GetSpawnArg("sound_move");
   if(!text)
      sound_move = "";

   movedir = G_GetMovedir();
   startorigin = worldorigin;
   doorsize = fabs(movedir * size);
   PostEvent(EV_ScriptDoor_DoInit, 0);
}

// EOF

