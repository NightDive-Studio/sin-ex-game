//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/misc.cpp                         $
// $Revision:: 152                                                            $
//   $Author:: Jimdose                                                        $
//     $Date:: 11/19/98 9:29p                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Basically the big stew pot of the DLLs, or maybe a garbage bin, whichever
// metaphore you prefer.  This really should be cleaned up.  Anyway, this
// should contain utility functions that could be used by any entity.
// Right now it contains everything from entities that could be in their
// own file to my mother's pot roast recipes.
// 

#include "g_local.h"
#include "entity.h"
#include "trigger.h"
#include "explosion.h"
#include "areaportal.h"
#include "misc.h"
#include "navigate.h"
#include "deadbody.h"
#include "specialfx.h"
#include "player.h"

/*
================
SendDialog
================
*/
void SendDialog(const char *icon_name, const char *dialog_text)
{
   char imageindex;
   char temp[1024];

   imageindex = gi.imageindex(icon_name);
   Com_sprintf(temp, sizeof(temp), "dia %d \"%s\"", imageindex, dialog_text);
   gi.WriteByte(svc_console_command);
   gi.WriteString(temp);
   gi.multicast(vec3_origin, MULTICAST_ALL);
}

/*
================
SendOverlay
================
*/
void SendOverlay(Entity *ent, str overlayname)
{
   if(ent)
   {
      gi.WriteByte(svc_console_command);
      gi.WriteString(va("lo %s", overlayname.c_str()));
      gi.unicast(ent->edict, true);
   }
   else
   {
      gi.WriteByte(svc_console_command);
      gi.WriteString(va("lo %s", overlayname.c_str()));
      gi.multicast(NULL, MULTICAST_ALL);
   }
}

/*
================
SendIntermission
================
*/
void SendIntermission(Entity *ent, str intermissionname)
{
   if(ent)
   {
      gi.WriteByte(svc_console_command);
      gi.WriteString(va("imf %s", intermissionname.c_str()));
      gi.unicast(ent->edict, true);
   }
   else
   {
      gi.WriteByte(svc_console_command);
      gi.WriteString(va("imf %s", intermissionname.c_str()));
      gi.multicast(NULL, MULTICAST_ALL);
   }
}

/*****************************************************************************/
/*SINED func_group (0 0 0) ?

Used to group brushes together just for editor convenience.

/*****************************************************************************/

/*****************************************************************************/
/*SINED func_remove (0 0.5 0) ?

Used for lighting and such

/*****************************************************************************/

CLASS_DECLARATION(Entity, FuncRemove, "func_remove");

ResponseDef FuncRemove::Responses[] =
{
   { NULL, NULL }
};

FuncRemove::FuncRemove() : Entity()
{
   ProcessEvent(EV_Remove);
}

/*****************************************************************************/
/*SINED info_null (0 0.5 0) (-4 -4 -4) (4 4 4)

Used as a positional target for spotlights, etc.

/*****************************************************************************/

CLASS_DECLARATION(Entity, InfoNull, "info_null");

ResponseDef InfoNull::Responses[] =
{
   { NULL, NULL }
};

InfoNull::InfoNull() : Entity()
{
   ProcessEvent(EV_Remove);
}

/*****************************************************************************/
/*SINED info_notnull (0 0.5 0) (-4 -4 -4) (4 4 4)

Used as a positional target for lightning.

/*****************************************************************************/

CLASS_DECLARATION(Entity, InfoNotNull, "info_notnull");

ResponseDef InfoNotNull::Responses[] =
{
   { NULL, NULL }
};

/*****************************************************************************/
/*SINED func_electrocute (0 .5 .8) ?
"radius" - range of the effect (Default is 500)
"key"    The item needed to activate this. (default nothing)
  Electrocutes everything it can see if it is in the water
/*****************************************************************************/

CLASS_DECLARATION(Trigger, Electrocute, "func_electrocute");

ResponseDef Electrocute::Responses[] =
{
   { &EV_Trigger_Effect,				(Response)&Electrocute::KillSight },
   { NULL, NULL }
};

Electrocute::Electrocute() : Trigger()
{
   setOrigin(origin);
   setSolidType(SOLID_NOT);
   setMoveType(MOVETYPE_NONE);

   radius = G_GetFloatArg("radius", 500);
}

void Electrocute::KillSight(Event *ev)
{
   Entity *other = ev->GetEntity(1);
   Entity *ent;

   ent = findradius(NULL, worldorigin, radius);
   while(ent)
   {
      if((ent != this) && (!ent->deadflag))
      {
         if(ent->waterlevel)
         {
            ent->Damage(this, other, ent->health, ent->worldorigin, vec_zero, vec_zero, 0, DAMAGE_NO_ARMOR, MOD_ELECTRIC, -1, -1, 1.0f);
         }
      }
      ent = findradius(ent, worldorigin, radius);
   }
}

/*****************************************************************************/
/*SINED func_spawn(0 .5 .8) (-8 -8 -8) (8 8 8)
"modelname" The name of the .def file you wish to spawn. (Required)
"spawntargetname" This will be the targetname of the spawned model. (default is null)
"key"       The item needed to activate this. (default nothing)
"attackmode" Attacking mode of the spawned actor (default 0)
/*****************************************************************************/

CLASS_DECLARATION(Entity, Spawn, "func_spawn");

ResponseDef Spawn::Responses[] =
{
   { &EV_Activate,         (Response)&Spawn::DoSpawn },
   { NULL, NULL }
};

Spawn::Spawn() : Entity()
{
   setSolidType(SOLID_NOT);
   setMoveType(MOVETYPE_NONE);
   hideModel();
   modelname = G_GetStringArg("modelname", NULL);
   angles = Vector(va("0 %f 0", G_GetFloatArg("angle", 0)));

   if(!modelname.length())
      warning("Spawn", "modelname not set");

   spawntargetname = G_GetStringArg("spawntargetname", NULL);
   attackmode = G_GetIntArg("attackmode", 0);
}

void Spawn::DoSpawn(Event *ev)
{
   char         temp[128];

   // Clear the spawn args
   G_InitSpawnArguments();

   snprintf(temp, sizeof(temp), "%f %f %f", worldorigin[0], worldorigin[1], worldorigin[2]);
   G_SetSpawnArg("origin", temp);
   snprintf(temp, sizeof(temp), "%f", angles[1]);
   G_SetSpawnArg("angle", temp);
   G_SetSpawnArg("model", modelname.c_str());
   G_SetSpawnArg("targetname", spawntargetname.c_str());
   G_SetSpawnArg("attackmode", va("%i", attackmode));
   G_SetSpawnArg("gravityaxis", va("%i", gravaxis)); //###

   G_CallSpawn();
   // Clear the spawn args
   G_InitSpawnArguments();
}

/*****************************************************************************/
/*SINED func_respawn(0 .5 .8) (-8 -8 -8) (8 8 8) 
When the thing that is spawned is killed, this func_respawn will get 
triggered.
"modelname"   The name of the .def file you wish to spawn. (Required)
"spawntargetname" This will be the targetname of the spawned model. (default is null)
"key"         The item needed to activate this. (default nothing)
/*****************************************************************************/

CLASS_DECLARATION(Spawn, ReSpawn, "func_respawn");

ResponseDef ReSpawn::Responses[] =
{
   { NULL, NULL }
};

void ReSpawn::DoSpawn(Event *ev)
{
   char         temp[128];

   // Clear the spawn args
   G_InitSpawnArguments();

   snprintf(temp, sizeof(temp), "%f %f %f", worldorigin[0], worldorigin[1], worldorigin[2]);
   G_SetSpawnArg("origin", temp);
   snprintf(temp, sizeof(temp), "%f", angles[1]);
   G_SetSpawnArg("angle", temp);
   G_SetSpawnArg("model", modelname.c_str());

   // This will trigger the func_respawn when the thing dies
   G_SetSpawnArg("targetname", TargetName());
   G_SetSpawnArg("target", TargetName());

   G_CallSpawn();
   // Clear the spawn args
   G_InitSpawnArguments();
}

/*****************************************************************************/
/*SINED func_spawnoutofsight(0 .5 .8) (-8 -8 -8) (8 8 8) 
Will only spawn something out of sight of its targets.
"modelname"   The name of the .def file you wish to spawn. (Required)
"spawntargetname" This will be the targetname of the spawned model. (default is null)
"key"         The item needed to activate this. (default nothing)
/*****************************************************************************/

CLASS_DECLARATION(Spawn, SpawnOutOfSight, "func_spawnoutofsight");

ResponseDef SpawnOutOfSight::Responses[] =
{
   { NULL, NULL }
};

void SpawnOutOfSight::DoSpawn(Event *ev)
{
   char     temp[128];
   int      i;
   Entity	*ent;
   edict_t	*ed;
   trace_t  trace;
   qboolean seen = false;

   // Check to see if I can see any players before spawning
   for(i = 0; i < game.maxclients; i++)
   {
      ed = &g_edicts[1 + i];
      if(!ed->inuse || !ed->entity)
      {
         continue;
      }

      ent = ed->entity;
      if((ent->health < 0) || (ent->flags & FL_NOTARGET))
      {
         continue;
      }

      trace = G_Trace(worldorigin, vec_zero, vec_zero, ent->centroid, this, MASK_OPAQUE, "SpawnOutOfSight::DoSpawn");
      if(trace.fraction == 1.0)
      {
         seen = true;
         break;
      }
   }

   if(seen)
      return;

   // Clear the spawn args
   G_InitSpawnArguments();

   snprintf(temp, sizeof(temp), "%f %f %f", worldorigin[0], worldorigin[1], worldorigin[2]);
   G_SetSpawnArg("origin", temp);
   snprintf(temp, sizeof(temp), "%f", angles[1]);
   G_SetSpawnArg("angle", temp);
   G_SetSpawnArg("model", modelname.c_str());
   G_SetSpawnArg("targetname", spawntargetname.c_str());

   G_CallSpawn();
   // Clear the spawn args
   G_InitSpawnArguments();
}

/*****************************************************************************/
/*SINED func_spawnchain(0 .5 .8) (-8 -8 -8) (8 8 8) 
Tries to spawn something out of the sight of players.  If it fails, it will
trigger its targets. 
"modelname"   The name of the .def file you wish to spawn. (Required)
"spawntargetname" This will be the targetname of the spawned model. (default is null)
"key"         The item needed to activate this. (default nothing)
/*****************************************************************************/

CLASS_DECLARATION(Spawn, SpawnChain, "func_spawnchain");

ResponseDef SpawnChain::Responses[] =
{
   { NULL, NULL }
};

void SpawnChain::DoSpawn(Event *ev)
{
   char        temp[128];
   int         i, num;
   Entity	   *ent;
   edict_t	   *ed;
   trace_t     trace;
   qboolean    seen = false;
   const char  *name;
   Event       *event;

   // Check to see if this can see any players before spawning
   for(i = 0; i < game.maxclients; i++)
   {
      ed = &g_edicts[1 + i];
      if(!ed->inuse || !ed->entity)
      {
         continue;
      }

      ent = ed->entity;
      if((ent->health < 0) || (ent->flags & FL_NOTARGET))
      {
         continue;
      }

      trace = G_Trace(worldorigin, vec_zero, vec_zero, ent->centroid, this, MASK_OPAQUE, "SpawnChain::DoSpawn");
      if(trace.fraction == 1.0)
      {
         seen = true;
         break;
      }
   }

   // Couldn't spawn anything, so activate targets
   if(seen)
   {
      name = Target();
      if(name && strcmp(name, ""))
      {
         num = 0;
         do
         {
            num = G_FindTarget(num, name);
            if(!num)
            {
               break;
            }
            ent = G_GetEntity(num);

            event = new Event(EV_Activate);
            event->AddEntity(world);
            ent->PostEvent(event, 0);
         }
         while(1);
      }
      return;
   }

   // Can't see the player, so do the spawn
   G_InitSpawnArguments();
   snprintf(temp, sizeof(temp), "%f %f %f", worldorigin[0], worldorigin[1], worldorigin[2]);
   G_SetSpawnArg("origin", temp);
   snprintf(temp, sizeof(temp), "%f", angles[1]);
   G_SetSpawnArg("angle", temp);
   G_SetSpawnArg("model", modelname.c_str());
   G_SetSpawnArg("targetname", spawntargetname.c_str());
   G_CallSpawn();
   G_InitSpawnArguments();
}

/*****************************************************************************/
/*SINED func_wall (0 .5 .8) ?

This is just a solid wall if not inhibitted

/*****************************************************************************/

CLASS_DECLARATION(Entity, Wall, "func_wall");

ResponseDef Wall::Responses[] =
{
   { NULL, NULL }
};

Wall::Wall() : Entity()
{
   setOrigin(origin);
   setSolidType(SOLID_BSP);
   setMoveType(MOVETYPE_PUSH);
}

/*****************************************************************************/
/*SINED func_illusionary (0 .5 .8) ?

A simple entity that looks solid but lets you walk through it.

/*****************************************************************************/

CLASS_DECLARATION(Entity, IllusionaryWall, "func_illusionary");

ResponseDef IllusionaryWall::Responses[] =
{
   { NULL, NULL }
};

IllusionaryWall::IllusionaryWall() : Entity()
{
   setSolidType(SOLID_NOT);
   setMoveType(MOVETYPE_NONE);
}

/*****************************************************************************/
/*SINED func_breakawaywall (0 .5 .8) ? x x NOT_PLAYERS MONSTERS PROJECTILES

Special walltype that removes itself when triggered.  Will also trigger
any func_areaportals that it targets.

"key"          The item needed to activate this. (default nothing)

If NOT_PLAYERS is set, the trigger does not respond to players
If MONSTERS is set, the trigger will respond to monsters
If PROJECTILES is set, the trigger will respond to projectiles (rockets, grenades, etc.)

/*****************************************************************************/

CLASS_DECLARATION(TriggerOnce, BreakawayWall, "func_breakawaywall");

Event EV_BreakawayWall_Setup("BreakawayWall_Setup");

ResponseDef BreakawayWall::Responses[] =
{
   { &EV_Touch,							NULL },
   { &EV_Trigger_Effect,				(Response)&BreakawayWall::BreakWall },
   { &EV_BreakawayWall_Setup,			(Response)&BreakawayWall::Setup },
   { NULL, NULL }
};

void BreakawayWall::BreakWall(Event *ev)
{
   SetAreaPortals(Target(), true);
   ActivateTargets(ev);
}

void BreakawayWall::Setup(Event *ev)
{
   SetAreaPortals(Target(), false);
}

BreakawayWall::BreakawayWall() : TriggerOnce()
{
   showModel();
   setMoveType(MOVETYPE_PUSH);
   setSolidType(SOLID_BSP);
   PostEvent(EV_BreakawayWall_Setup, 0.1);
   respondto = spawnflags ^ TRIGGER_PLAYERS;
};

//### added explosion scale control
/*****************************************************************************/
/*SINED func_explodingwall (0 .5 .8) ? RANDOMANGLES LANDSHATTER NOT_PLAYERS MONSTERS PROJECTILES INVISIBLE ACCUMALATIVE TWOSTAGE

Blows up on activation or when attacked

"explosions"   number of explosions to spawn ( default 1 )
"land_angles"  The angles you want this piece to\
               orient to when it lands on the ground
"land_radius"  The distance of the ground the piece\
               should be when on the ground ( default 0 )
"anglespeed"   Speed at which pieces rotate ( default 100 ) \
               if RANDOMANGLES ( default is 600 )
"key"          The item needed to activate this. (default nothing)
"scale" is the scale of the explosions to make. Default is 1.

IF RANDOMANGLES is set, object randomly spins while in the air.
IF LANDSHATTER is set, object shatters when it hits the ground.
IF TWOSTAGE is set, object can be shattered once it lands on the ground.
IF ACCUMALATIVE is set, damage is accumlative not threshold
IF INVISIBLE is set, these are invisible and not solid until triggered
If NOT_PLAYERS is set, the trigger does not respond to players
If MONSTERS is set, the trigger will respond to monsters
If PROJECTILES is set, the trigger will respond to projectiles (rockets, grenades, etc.)

/*****************************************************************************/
#define RANDOMANGLES ( 1 << 0 )
#define LANDSHATTER  ( 1 << 1 )
#define INVISIBLE    ( 1 << 5 )
#define ACCUMULATIVE ( 1 << 6 )
#define TWOSTAGE     ( 1 << 7 )

CLASS_DECLARATION(Trigger, ExplodingWall, "func_explodingwall");

Event EV_ExplodingWall_StopRotating("stoprotating");
Event EV_ExplodingWall_OnGround("checkonground");

ResponseDef ExplodingWall::Responses[] =
{
   { &EV_Trigger_Effect,             (Response)&ExplodingWall::Explode },
   { &EV_Damage,                     (Response)&ExplodingWall::DamageEvent },
   { &EV_Touch,                      (Response)&ExplodingWall::TouchFunc },
   { &EV_ExplodingWall_StopRotating, (Response)&ExplodingWall::StopRotating },
   { &EV_ExplodingWall_OnGround,     (Response)&ExplodingWall::CheckOnGround },
   { NULL, NULL }
};

void ExplodingWall::Explode(Event *ev)
{
   Entity		*other;
   Vector		pos;
   Vector      mins, maxs;
   int			i;

   if(spawnflags & INVISIBLE)
   {
      showModel();
      setSolidType(SOLID_BSP);
      takedamage = DAMAGE_YES;
   }

   if(takedamage == DAMAGE_NO)
   {
      return;
   }

   other = ev->GetEntity(1);

   health = 0;
   takedamage = DAMAGE_NO;

   // Create explosions
   for(i = 0; i < explosions; i++)
   {
      pos[0] = absmin[0] + G_Random(size[0]);
      pos[1] = absmin[1] + G_Random(size[1]);
      pos[2] = absmin[2] + G_Random(size[2]);

      //###
      //CreateExplosion( pos, dmg, 1.0f, TRUE, this, other, this );
      CreateExplosion(pos, dmg, explosionscale, true, this, other, this);
      //###
   }

   // throw itself
   state = 1;
   on_ground = false;
   PostEvent(EV_ExplodingWall_OnGround, 0.1f);
   velocity[0] = G_CRandom(70);
   velocity[1] = G_CRandom(70);
   velocity[2] = 140 + G_Random(70);
   setMoveType(MOVETYPE_BOUNCE);
   setSolidType(SOLID_BBOX);
   if(spawnflags & RANDOMANGLES)
   {
      avelocity[0] = G_Random(angle_speed);
      avelocity[1] = G_Random(angle_speed);
      avelocity[2] = G_Random(angle_speed);
   }
   else
   {
      Vector delta;
      float most;
      float time;
      int   t;

      delta = land_angles - worldangles;
      if(delta[0] > 180)
         delta[0] -= 360;
      if(delta[0] < -180)
         delta[0] += 360;
      if(delta[1] > 180)
         delta[1] -= 360;
      if(delta[1] < -180)
         delta[1] += 360;
      if(delta[2] > 180)
         delta[2] -= 360;
      if(delta[2] < -180)
         delta[2] += 360;
      most = MaxValue(delta);
      if(!angle_speed)
         angle_speed = 1;
      t = 10 * most / angle_speed;
      time = (float)t / 10;
      delta = delta * (1.0 / time);
      avelocity = delta;
      PostEvent(EV_ExplodingWall_StopRotating, time);
      state = 2;
   }

   ActivateTargets(ev);

   if(land_radius > 0)
   {
      mins[0] = mins[1] = mins[2] = -land_radius;
      maxs[0] = maxs[1] = maxs[2] = land_radius;
      setSize(mins, maxs);
   }

   attack_finished = 0;
}

void ExplodingWall::DamageEvent(Event *ev)
{
   Event			*event;
   Entity		*inflictor;
   Entity		*attacker;
   int			damage;

   if(takedamage == DAMAGE_NO)
   {
      return;
   }

   if(on_ground)
   {
      GroundDamage(ev);
      return;
   }

   damage = ev->GetInteger(1);
   inflictor = ev->GetEntity(2);
   attacker = ev->GetEntity(3);

   if(spawnflags & ACCUMULATIVE)
   {
      health -= damage;
      if(health > 0)
         return;
   }
   else
   {
      if(damage < health)
      {
         return;
      }
   }

   event = new Event(EV_Activate);
   event->AddEntity(attacker);
   ProcessEvent(event);
}

void ExplodingWall::GroundDamage(Event *ev)
{
   Vector      dir;
   Entity		*inflictor;
   Entity		*attacker;
   Vector		pos;
   int			damage;

   if(takedamage == DAMAGE_NO)
   {
      return;
   }

   damage = ev->GetInteger(1);
   inflictor = ev->GetEntity(2);
   attacker = ev->GetEntity(3);

   if(spawnflags & ACCUMULATIVE)
   {
      health -= damage;
      if(health > 0)
         return;
   }
   else
   {
      if(damage < health)
      {
         return;
      }
   }

   if(explosions)
   {
      pos[0] = absmin[0] + G_Random(size[0]);
      pos[1] = absmin[1] + G_Random(size[1]);
      pos[2] = absmin[2] + G_Random(size[2]);

      //###
      //CreateExplosion( pos, damage, 1.0f, true, this, attacker, this );
      CreateExplosion(pos, damage, explosionscale, true, this, attacker, this);
      //###
   }
   takedamage = DAMAGE_NO;
   hideModel();
   dir = worldorigin - attacker->worldorigin;
   TesselateModel
   (
      this,
      tess_min_size,
      tess_max_size,
      dir,
      damage,
      tess_percentage,
      tess_thickness,
      vec3_origin
   );
   ProcessEvent(EV_BreakingSound);
   PostEvent(EV_Remove, 0);
}

void ExplodingWall::SetupSecondStage(void)
{
   health = max_health;
   takedamage = DAMAGE_YES;
}

void ExplodingWall::StopRotating(Event *ev)
{
   avelocity = vec_zero;
   setAngles(land_angles);
   if(spawnflags & TWOSTAGE)
      SetupSecondStage();
}

void ExplodingWall::CheckOnGround(Event *ev)
{
   if((velocity == vec_zero) && groundentity)
   {
      Vector delta;
      float most;
      float time;
      int   t;

      delta = land_angles - worldangles;
      if(delta.length() > 1)
      {
         if(delta[0] > 180)
            delta[0] -= 360;
         if(delta[0] < -180)
            delta[0] += 360;
         if(delta[1] > 180)
            delta[1] -= 360;
         if(delta[1] < -180)
            delta[1] += 360;
         if(delta[2] > 180)
            delta[2] -= 360;
         if(delta[2] < -180)
            delta[2] += 360;
         most = MaxValue(delta);
         if(angle_speed > 3)
            t = 10.0f * most / (angle_speed / 3);
         else
            t = 10.0f * most;
         time = (float)t / 10;
         delta = delta * (1.0 / time);
         avelocity = delta;
         PostEvent(EV_ExplodingWall_StopRotating, time);
      }
      state = 2;
      setSize(orig_mins, orig_maxs);
      on_ground = true;
   }
   else
      PostEvent(ev, 0.1f);
}

void ExplodingWall::TouchFunc(Event *ev)
{
   Entity *other;

   if((velocity == vec_zero) || (level.time < attack_finished))
   {
      return;
   }

   other = ev->GetEntity(1);

   if((spawnflags & LANDSHATTER) && (other == world))
   {
      Vector pos;

      takedamage = DAMAGE_NO;

      if(explosions)
      {
         pos[0] = absmin[0] + G_Random(size[0]);
         pos[1] = absmin[1] + G_Random(size[1]);
         pos[2] = absmin[2] + G_Random(size[2]);

         //###
         //CreateExplosion( pos, dmg, 1.0f, true, this, other, this );
         CreateExplosion(pos, dmg, explosionscale, true, this, other, this);
         //###
      }
      hideModel();
      TesselateModel
      (
         this,
         tess_min_size,
         tess_max_size,
         vec_zero,
         100,
         tess_percentage,
         tess_thickness,
         vec3_origin
      );
      ProcessEvent(EV_BreakingSound);
      PostEvent(EV_Remove, 0);
      return;
   }

   if(other->takedamage)
   {
      other->Damage(this, activator, dmg, worldorigin, vec_zero, vec_zero, 20, 0, MOD_EXPLODEWALL, -1, -1, 1.0f);
      RandomGlobalSound("debris_generic", 1, CHAN_WEAPON, ATTN_NORM);
      attack_finished = level.time + 0.1;
   }
}

ExplodingWall::ExplodingWall() : Trigger()
{
   if(spawnflags & INVISIBLE)
   {
      if(Targeted())
         takedamage = DAMAGE_YES;
      else
         takedamage = DAMAGE_NO;
      hideModel();
      setSolidType(SOLID_NOT);
   }
   else
   {
      showModel();
      setSolidType(SOLID_BSP);
      takedamage = DAMAGE_YES;
   }
   setMoveType(MOVETYPE_PUSH);
   setOrigin(origin);

   health = G_GetFloatArg("health", 60);
   max_health = health;
   on_ground = false;

   state = 0;
   if(spawnflags & RANDOMANGLES)
      angle_speed = G_GetFloatArg("anglespeed", 600);
   else
      angle_speed = G_GetFloatArg("anglespeed", 100);

   land_radius = G_GetFloatArg("land_radius", 0);
   land_angles = G_GetVectorArg("land_angles");
   dmg = G_GetIntArg("dmg", 10);
   explosions = G_GetIntArg("explosions", 1);

   explosionscale = G_GetFloatArg("scale", 1); //###

   orig_mins = mins;
   orig_maxs = maxs;

   respondto = spawnflags ^ TRIGGER_PLAYERS;
}

/*****************************************************************************/
/*SINED detail (0.5 0 1.0) ?

Used to fake details before the Quake 2 merge.

/*****************************************************************************/

CLASS_DECLARATION(Entity, Detail, "detail");

ResponseDef Detail::Responses[] =
{
   { NULL, NULL }
};

Detail::Detail() : Wall()
{
   // Be an asshole to the level designers so that they make the change asap.
   gi.dprintf("Detail brushes are no longer needed.  Use Surface attributes.\n");

   if(!G_GetSpawnArg("model"))
   {
      gi.dprintf("Detail brush with NULL model removed!!!\n");
      ProcessEvent(EV_Remove);
   }
   else
   {
      ProcessEvent(EV_Remove);
   }
}

/*****************************************************************************/
/*SINED misc_oxygen (1 0 0) ? VISIBLE

Touching this entity will reset the drowning time - only
responds to players.

"key" The item needed to activate this. (default nothing)
/*****************************************************************************/

CLASS_DECLARATION(Trigger, Oxygenator, "misc_oxygen");

ResponseDef Oxygenator::Responses[] =
{
   { &EV_Trigger_Effect,	(Response)&Oxygenator::Oxygenate },
   { NULL, NULL }
};

Oxygenator::Oxygenator() : Trigger()
{
   if(spawnflags & 1)
   {
      showModel();
   }

   respondto = TRIGGER_PLAYERS;
}

EXPORT_FROM_DLL void Oxygenator::Oxygenate(Event *ev)
{
   Entity *other;
   Player *player;

   other = ev->GetEntity(1);

   if(!other)
      return;

   player = (Player *)(Sentient *)other;
   player->GiveOxygen(time);
}

//### added stuff for hoverbikes
#define NOEFFECTS 64
#define OFFSET_MOVE 128
/*****************************************************************************/
/*SINED misc_teleporter (1 0 0) ? VISIBLE x NOT_PLAYERS NOT_MONSTERS NOT_PROJECTILES x NOEFFECTS OFFSET_MOVE

Touching this entity will teleport players to the targeted object.

"key" The item needed to activate this. (default nothing)

If NOT_PLAYERS is set, the teleporter does not teleport players
If NOT_MONSTERS is set, the teleporter does not teleport monsters
If NOT_PROJECTILES is set, the teleporter does not teleport projectiles (rockets, grenades, etc.)
If NOEFFECTS is set, it will not make a teleporting sound or particles.
If OFFSET_MOVE is set, it will "offset" the player to the destination, not actually teleport him.

"gravityaxis" now sets player's axis of gravity when teleported. Valid values are 0 to 5. Here's list of what orientation goes with which value.
0: upright
1: South is down
2: East is down
3: upsidedown
4: North is down
5: West is down

/*****************************************************************************/

CLASS_DECLARATION(Trigger, Teleporter, "misc_teleporter");

ResponseDef Teleporter::Responses[] =
{
   { &EV_Trigger_Effect,	(Response)&Teleporter::Teleport },
   { NULL, NULL }
};

EXPORT_FROM_DLL void Teleporter::Teleport(Event *ev)
{
   gclient_t	*client;
   Entity		*dest;
   int			num;
   int			i;
   Entity		*other;
   Vector		mid;

   other = ev->GetEntity(1);

   if(!other || (other == world))
      return;

   num = G_FindTarget(0, Target());
   if(!num)
   {
      warning("Teleport", "Couldn't find destination\n");
      return;
   }

   dest = G_GetEntity(num);
   assert(dest);

   //### spawnflag that disables the teleporting effects
   if(!(spawnflags & NOEFFECTS))
   {
      other->RandomGlobalSound("snd_teleport");
   }
   //###

   // unlink to make sure it can't possibly interfere with KillBox
   other->unlink();

   //### added OFFSET_MOVE type of teleporting
   if(spawnflags & OFFSET_MOVE)
   {
      mid = (absmax + absmin) * 0.5;
      mid[2] = absmin[2];
      other->origin -= mid;
      other->origin += dest->origin;
   }
   else if(other->isSubclassOf<Sentient>())       //###
   {
      PathManager.Teleport(other, other->worldorigin, dest->worldorigin);
      other->worldorigin = dest->worldorigin + Vector(0, 0, 1);
      other->velocity = vec_zero;
      //### raise player up a bit if on a hoverbike
      if(((Sentient *)other)->IsOnBike())
      {
         other->origin[gravity_axis[gravaxis].z] += 24 * gravity_axis[gravaxis].sign;
      }
      //###
   }
   else
   {
      mid = (absmax - absmin) * 0.5;
      other->worldorigin = dest->worldorigin + Vector(0, 0, 1);
      other->origin += mid;
   }

   // draw the teleport splash at the destination
   //other->edict->s.event = EV_PLAYER_TELEPORT;

   // set angles
   //### added OFFSET_MOVE type of teleporting
   if(!(spawnflags & OFFSET_MOVE))
   {
      // set angles
      other->setAngles( dest->angles );
   }
   //###

   // set their gravity axis
   other->SetGravityAxis(gravaxis);

   if(other->client)
   {
      Event * ev;
      client = other->client;

      if(!gravaxis)
      {
         // clear the velocity and hold them in place briefly
         client->ps.pmove.pm_time = 100;
         client->ps.pmove.pm_flags |= PMF_TIME_TELEPORT;

         //### spawnflag that disables the teleporting effects
         if(!(spawnflags & NOEFFECTS))
         {
            ev = new Event(EV_Player_SaveFov);
            other->ProcessEvent(ev);

            ev = new Event(EV_Player_Fov);
            ev->AddFloat(180);
            other->ProcessEvent(ev);
         }
         //###
      }

      /*
      if ( gravaxis )
         {
         ev = new Event( EV_Player_RestoreFov );
         other->PostEvent( ev, 0.1f );
         }
      */

      //### added OFFSET_MOVE type of teleporting
      if(!(spawnflags & OFFSET_MOVE))
      {
         for(i = 0; i < 3; i++)
         {
            client->ps.pmove.delta_angles[i] = ANGLE2SHORT(dest->angles[i] - client->resp.cmd_angles[i]);
         }

         VectorCopy(angles.vec3(), client->ps.viewangles);
      }
      //###
   }

   if(dest->isSubclassOf<TeleporterDestination>() && !gravaxis)
   {
      //### added OFFSET_MOVE type of teleporting
      if(!(spawnflags & OFFSET_MOVE))
      {
         float len;

         len = other->velocity.length();
         //
         // give them a bit of a push
         //
         if(len < 400)
            len = 400;
         //### corrected for different gravityaxies
         //other->velocity = ( ( TeleporterDestination * )dest )->movedir * len;
         mid = ((TeleporterDestination *)dest)->movedir * len;
         other->velocity[0] = mid[gravity_axis[other->gravaxis].x];
         other->velocity[1] = mid[gravity_axis[other->gravaxis].y] * gravity_axis[other->gravaxis].sign;
         other->velocity[2] = mid[gravity_axis[other->gravaxis].z] * gravity_axis[other->gravaxis].sign;
      }
      //###
   }

   // kill anything at the destination
   KillBox(other);

   other->setOrigin(other->worldorigin);
   other->worldorigin.copyTo(other->edict->s.old_origin);

   //### check if teleporting a player on a hoverbike
   if(other->isClient())
   {
      Hoverbike *bike;
      Player *rider;

      rider = (Player *)other;

      // he's on a bike, so move the bike too
      if((bike = rider->GetHoverbike()))
      {
         bike->SetGravityAxis(gravaxis);

         bike->setOrigin(other->origin);
         bike->worldorigin.copyTo(bike->edict->s.old_origin);

         if(dest->isSubclassOf<TeleporterDestination>())
         {
            if(!(spawnflags & OFFSET_MOVE))
            {
               float len = bike->velocity.length();
               // rider velocity has already been adjusted
               bike->velocity = rider->velocity;
            }
         }
      }
   }
   //###
}

Teleporter::Teleporter() : Trigger()
{
   if(!Target())
   {
      gi.dprintf("teleporter without a target.\n");
      ProcessEvent(EV_Remove);
      return;
   }

   if(spawnflags & 1)
   {
      showModel();
   }

   //### for hoverbikes
   //respondto = spawnflags ^ ( TRIGGER_PLAYERS | TRIGGER_MONSTERS | TRIGGER_PROJECTILES );
   respondto = spawnflags ^ (TRIGGER_PLAYERS | TRIGGER_MONSTERS | TRIGGER_PROJECTILES | TRIGGER_HOVERBIKES);
}

/*****************************************************************************/
/*SINED misc_teleporter_dest (1 0 0) (-32 -32 0) (32 32 8)

Point teleporters at these.

/*****************************************************************************/

CLASS_DECLARATION(Entity, TeleporterDestination, "misc_teleporter_dest");

ResponseDef TeleporterDestination::Responses[] =
{
   { NULL, NULL }
};

TeleporterDestination::TeleporterDestination() : Entity()
{
   movedir = G_GetMovedir();
   setAngles(movedir.toAngles());
}

/*****************************************************************************/
/*SINED waypoint (0 0.5 0) (-8 -8 -8) (8 8 8)

Used as a positioning device for objects

/*****************************************************************************/

CLASS_DECLARATION(Entity, Waypoint, "waypoint");

ResponseDef Waypoint::Responses[] =
{
   { NULL, NULL }
};

/*****************************************************************************/
/*SINED func_shatter (0 .5 .8) ? x x NOT_PLAYERS NOT_MONSTERS NOT_PROJECTILES HURT_SHATTER THRESHOLD

For shattering objects.  Triggers only when a threshold of damage is exceeded.
Will also trigger any targeted func_areaportals when not invisible.

"health" specifies how much damage must occur before trigger fires.  Default is 20.
"percentage" specifies how much of the thing to shatter. Default is 50
"minsize" specifies minsize for tesselation, default based off size
"maxsize" specifies maxsize for tesselation, default based off size
"thickness" specifies thickness for tesselation, default same as minsize
"key"     The item needed to activate this. (default nothing)
"noise" sound to play when shattered, defaults to nothing

HURT_SHATTER - when the thing gets hurt, spawn pieces of itself
THRESHOLD - damage threshold behavior

If NOT_PLAYERS is set, the trigger does not respond to damage caused by players
If NOT_MONSTERS is set, the trigger does not respond to damage caused by monsters
If NOT_PROJECTILES is set, the trigger does not respond to damage caused by projectiles

/*****************************************************************************/

CLASS_DECLARATION(Trigger, Shatter, "func_shatter");

ResponseDef Shatter::Responses[] =
{
   { &EV_Trigger_Effect,			(Response)&Shatter::DoShatter },
   { &EV_Damage,						(Response)&Shatter::DamageEvent },
   { NULL, NULL }
};

void Shatter::DamageEvent(Event *ev)
{
   Event			*event;
   Entity		*inflictor;
   Entity		*attacker;
   int			damage;

   if(takedamage == DAMAGE_NO)
   {
      return;
   }

   damage = ev->GetInteger(1);
   inflictor = ev->GetEntity(2);
   attacker = ev->GetEntity(3);

   if(threshold && damage < health)
   {
      return;
   }
   else if(!threshold)
   {
      health -= damage;
      if(health > 0)
      {
         damage_taken += damage;
         return;
      }
   }

   damage_taken += damage;

   if(attacker)
   {
      event = new Event(EV_Activate);
      event->AddEntity(attacker);
      ProcessEvent(event);
   }
   else
   {
      warning("Damage", "Attacker is null\n");
   }
}

void Shatter::DoShatter(Event *ev)
{
   Entity	*other;
   Event		*event;
   Vector   dir;

   if(takedamage == DAMAGE_NO)
   {
      return;
   }

   if(noise.length() > 1)
   {
      sound(noise.c_str(), 1, CHAN_VOICE + CHAN_NO_PHS_ADD);
   }

   other = ev->GetEntity(1);

   takedamage = DAMAGE_NO;

   dir = worldorigin - other->worldorigin;

   TesselateModel
   (
      this,
      tess_min_size,
      tess_max_size,
      dir,
      damage_taken,
      tess_percentage,
      tess_thickness,
      vec3_origin
   );

   SetAreaPortals(Target(), true);

   event = new Event(EV_Trigger_ActivateTargets);
   event->AddEntity(other);
   ProcessEvent(event);

   ProcessEvent(EV_BreakingSound);

   PostEvent(EV_Remove, 0);
}

Shatter::Shatter() : DamageThreshold()
{
   //
   // Can only be used once
   //
   count = -1;

   // Since we're a subclass of DamageThreshold, override the invisible behaviour
   showModel();
   PostEvent(Event("setup"), 0);

   tess_percentage = G_GetFloatArg("percentage", tess_percentage * 100) / 100.0f;
   tess_min_size   = G_GetIntArg("minsize",   tess_min_size);
   tess_max_size   = G_GetIntArg("maxsize",   tess_max_size);
   tess_thickness  = G_GetIntArg("thickness", tess_thickness);

   health = G_GetFloatArg("health", 20);
   max_health = health;

   noise = str(G_GetSpawnArg("noise", ""));

   if(spawnflags & (1 << 5))
      flags |= FL_TESSELATE;

   if(spawnflags & (1 << 6))
      threshold = true;

   tess_thickness = 10;

   respondto = spawnflags ^ (TRIGGER_PLAYERS | TRIGGER_MONSTERS | TRIGGER_PROJECTILES);
}

/*****************************************************************************/
/*SINED func_glass (0 .5 .8) ? x x NOT_PLAYERS NOT_MONSTERS NOT_PROJECTILES HURT_SHATTER THRESHOLD

For glass objects.  Shatters when the accumulated damage is exceeded, or when activated

"health" specifies how much damage must occur before the glass shatters.  Default is 60.
"percentage" specifies how much of the thing to shatter. Default is 50
"minsize" specifies minsize for tesselation, default based off size
"maxsize" specifies maxsize for tesselation, default based off size
"thickness" specifies thickness for tesselation, default same as minsize
"key"     The item needed to activate this. (default nothing)
"noise" sound to play when shattered, defaults to glass breaking

If NOT_PLAYERS is set, the trigger does not respond to events caused by players
If NOT_MONSTERS is set, the trigger does not respond to events caused by monsters
If NOT_PROJECTILES is set, the trigger does not respond to events caused by projectiles

/*****************************************************************************/

CLASS_DECLARATION(Shatter, Glass, "func_glass");

ResponseDef Glass::Responses[] =
{
   { NULL, NULL }
};

Glass::Glass() : Shatter()
{
   if(!noise.length())
   {
      const char * realname;

      if(max_health <= 60)
      {
         realname = gi.GlobalAlias_FindRandom("impact_smlglass");
         if(realname)
            noise = str(realname);
      }
      else
      {
         realname = gi.GlobalAlias_FindRandom("impact_lrgglass");
         if(realname)
            noise = str(realname);
      }
   }
   gi.soundindex(noise.c_str());
}

//
// MadeBreakingSound
//
// Entity-less notifier for AI
//
void MadeBreakingSound(Vector pos, Entity *activator)
{
   Entity	*ent;
   Event		*ev;

   //
   // make sure activator is valid
   //

   if(!activator)
      activator = world;

   ent = NULL;
   while((ent = findradius(ent, pos.vec3(), SOUND_BREAKING_RADIUS)))
   {
      if(!ent->deadflag && ent->isSubclassOf<Sentient>() && (ent != activator) &&
         gi.inPHS(pos.vec3(), ent->centroid.vec3())
         )
      {
         ev = new Event(EV_HeardBreaking);
         ev->AddEntity(activator);
         ev->AddVector(pos);
         ent->PostEvent(ev, 0);
      }
   }
}

CLASS_DECLARATION( Entity, BloodSplat, NULL );

ResponseDef BloodSplat::Responses[] =
{
   { NULL, NULL }
};

int BloodSplat::numBloodSplats = 0;
Queue BloodSplat::queueBloodSplats;

BloodSplat::BloodSplat(Vector pos, Vector ang, float scale) : Entity()
{
   BloodSplat *fadesplat;

   if(numBloodSplats > sv_maxbloodsplats->value)
   {
      // Fade one out of the list.
      fadesplat = (BloodSplat *)queueBloodSplats.Dequeue();
      fadesplat->ProcessEvent(EV_FadeOut);
      numBloodSplats--;

      // Don't spawn one until we others have faded
      PostEvent(EV_Remove, 0);
      return;
   }

   setMoveType(MOVETYPE_NONE);
   setSolidType(SOLID_NOT);
   setModel("sprites/bloodsplat.spr");
   edict->s.frame = G_Random(4);
   setSize({ 0, 0, 0 }, { 0, 0, 0 });

   queueBloodSplats.Enqueue( this );
   numBloodSplats++;

   edict->s.scale = scale * edict->s.scale;
	setAngles( ang );
   setOrigin( pos );
	}

BloodSplat::~BloodSplat()
{
   if(queueBloodSplats.Inqueue(this))
   {
      queueBloodSplats.Remove(this);
      numBloodSplats--;
   }
}

/*****************************************************************************/
/*SINED func_clipbox (0 .5 .8) (-16 -16 -16) (16 16 16)

Invisible bounding box used like a clip brush.  This is mainly used for blocking
off areas or improving clipping without having to recompile the map.  Because of
this, it will most likely only be spawned via script.
age is exceeded, or when activated

"mins" min point of the clip.
"maxs" max point of the clip.
"type" -
0 Monster and Player clip
1 Monster clip
2 Player clip

/*****************************************************************************/

CLASS_DECLARATION(Entity, ClipBox, "func_clipbox");

ResponseDef ClipBox::Responses[] =
{
   { NULL, NULL }
};

ClipBox::ClipBox() : Entity()
{
   int type;

   setMoveType(MOVETYPE_NONE);
   setSolidType(SOLID_BBOX);
   hideModel();

   type = G_GetIntArg("type");

   edict->clipmask = MASK_SOLID;
   switch(type)
   {
   case 1:
      edict->svflags |= SVF_MONSTERCLIP;
      break;

   case 2:
      edict->svflags |= SVF_PLAYERCLIP;
      break;

   default:
      edict->svflags |= SVF_PLAYERCLIP | SVF_MONSTERCLIP;
         break;
   }

   mins = G_GetVectorArg("mins");
   maxs = G_GetVectorArg("maxs");

   origin = (mins + maxs) * 0.5;

   setSize(mins - origin, maxs - origin);
   setOrigin(origin);
}

// EOF

