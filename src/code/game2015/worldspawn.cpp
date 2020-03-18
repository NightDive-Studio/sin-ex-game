//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/worldspawn.cpp                   $
// $Revision:: 97                                                             $
//   $Author:: Jimdose                                                        $
//     $Date:: 11/08/98 10:49p                                                $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Base class for worldspawn objects.  This should be subclassed whenever
// a DLL has new game behaviour that needs to be initialized before any other
// entities are created, or before any entity thinks each frame.  Also controls
// spawning of clients.
//

#include "g_local.h"
#include "entity.h"
#include "scriptmaster.h"
#include "worldspawn.h"
#include "surface.h"
#include "console.h"
#include "deadbody.h"
#include "gravpath.h"
#include "earthquake.h"
//###
#include "spritegun.h"   // added for spritegun
#include "checkpoints.h"
//###

extern void CreateMissionComputer();

WorldPtr  world;

//### added some options to the worldspawn
/*****************************************************************************/
/*SINED worldspawn (0 0 0) ? CINEMATIC x BIKESONLY BODIESFADE

Only used for the world.
"sky"			environment map name
"skyaxis"	vector axis for rotating sky
"skyrotate"	speed of rotation in degrees/second
"soundtrack" the music file to use
"cdtrack"	music cd track number
"gravity"	800 is default gravity
"message"	text to print at user logon
"skipthread" thread that is activated to skip this level (if cinematic)

"watercolor" specifies the view blend color while in water.
"wateralpha" specifies the alpha of the view blend while in water.
"lavacolor" specifies the view blend color while in lava.
"lavaalpha" specifies the alpha of the view blend while in lava.
"lightcolor" specifies the view blend color while in light.
"lightalpha" specifies the alpha of the view blend while in light.

"lavadamage" sets the amount of damage laval should do per waterheight level. Default is 10.

"cpsounds" sets the script file to get the hoverbike checkpoint sounds from. Default is "global/default_cp_sounds.scr"

BIKESONLY  all players spawn in hoverbikes and can't get off. If the bike dies, they die.
BODIESFADE  makes dead player bodies fade shortly after dying. Good for big open maps.
/*****************************************************************************/

#define CINEMATIC  1
//###
#define BIKESONLY  4
#define BODIESFADE 8
//###

CLASS_DECLARATION(Entity, World, "worldspawn");

ResponseDef World::Responses[] =
{
   { nullptr, nullptr }
};

World::World() : Entity()
{
   const char *text;
   str         mapname;
   int         i;
   Vector      water_color;
   Vector      lightvolume_color;
   Vector      lava_color;
   Event      *ev; //### added for spritegun

   world = this;

   setMoveType(MOVETYPE_NONE);
   setSolidType(SOLID_BSP);

   // world model is always index 1
   edict->s.modelindex = 1;
   model = "*1";

   // Anything that modifies configstrings, or spawns things is ignored when loading savegames
   if(LoadingSavegame)
   {
      return;
   }

   // inform the client that this is deathmatch, and we should
   // draw deathmatch stats.  This goes around what the CS_STATUSBAR
   // used to be used for since we moved all HUDS to the client.
   if(deathmatch->value)
   {
      gi.configstring(CS_STATUSBAR, "DEATHMATCH");
   }
   else
   {
      gi.configstring(CS_STATUSBAR, "SINGLE_PLAYER");
   }

   //
   // see if the level has a soundtrack associated withit
   //
   text = G_GetSpawnArg("soundtrack");
   if(text)
   {
      gi.configstring(CS_SOUNDTRACK, text);
   }

   text = G_GetSpawnArg("sky");
   if(text)
   {
      gi.configstring(CS_SKY, text);
   }
   else
   {
      gi.configstring(CS_SKY, "sky_");
   }

   text = G_GetSpawnArg("skyrotate");
   gi.configstring(CS_SKYROTATE, text ? text : "0");

   text = G_GetSpawnArg("skyaxis");
   gi.configstring(CS_SKYAXIS, text ? text : "0 0 0");

   text = G_GetSpawnArg("cdtrack");
   gi.configstring(CS_CDTRACK, text ? text : "0");

   gi.configstring(CS_MAXCLIENTS, va("%i", (int)(maxclients->value)));

   text = G_GetSpawnArg("gravity");
   if(!text)
   {
      gi.cvar_set("sv_gravity", "800");
   }
   else
   {
      gi.cvar_set("sv_gravity", text);
   }

   // get skipthread
   skipthread = G_GetStringArg("skipthread");

   // the world takes blast marks and sparks by default
   flags |= FL_BLASTMARK;
   flags |= FL_SPARKS;

   // Reserve some space for dead bodies
   InitializeBodyQueue();

   //
   // see if this is a cinematic level
   //
   if(spawnflags & CINEMATIC)
   {
      level.cinematic = true;
   }
   else
   {
      level.cinematic = false;
   }

   level.nextmap = G_GetStringArg("nextmap");

   // make some data visible to the server
   text = G_GetSpawnArg("message");
   if(text)
   {
      gi.configstring(CS_NAME, text);
      level.level_name = text;
   }
   else
   {
      level.level_name = level.mapname;
   }

   // Set up script
   text = G_GetSpawnArg("script");
   if(!text)
   {
      // No script specified.  Try using the mapname as the script name
      mapname = "maps/";
      mapname += level.mapname;
      for(i = mapname.length() - 1; i >= 0; i--)
      {
         if(mapname[i] == '.')
         {
            mapname[i] = 0;
            break;
         }
      }

      mapname += ".scr";

      text = &mapname[5];

      // If there isn't a script with the same name as the map, then don't try to load script
      if(gi.LoadFile(mapname.c_str(), nullptr, 0) == -1)
      {
         text = nullptr;
      }
   }

   if(text)
   {
      gi.dprintf("Adding script: '%s'\n", text);
      mapname = va("maps/%s", text);

      // just set the script, we will start it in G_Spawn
      ScriptLib.SetGameScript(mapname.c_str());
   }

   //###
   // read in sprite file after everything's spawned
   ev = new Event("spg_load");
   Spritecontrol.PostEvent(ev, 0.1);

   // setup checkpoints sounds
   level.cp_sounds_script = G_GetStringArg("cpsounds", "global/default_cp_sounds.scr");

   //set this here for when there's no checkpoints
   gi.configstring(CS_CHECKPOINTS, "NONE");

   // init the checkpoint counter
   level.cp_anyorder = false;
   level.cp_num = 0;

   if(spawnflags & BIKESONLY)
      level.bikesonly = true;
   else
      level.bikesonly = false;

   if(spawnflags & BODIESFADE)
      level.bodiesfade = true;
   else
      level.bodiesfade = false;

   level.lavadamage = G_GetIntArg("lavadamage", 10);

   // remove any time limit offset
   reset_timeofs = 0;

   // clear client ghost data since we're starting a new map
   memset(game.clientghosts, 0, game.maxclients * sizeof(game.clientghosts[0]));
   //###

   // Set the color for the blends.
   water_color = G_GetVectorArg("watercolor", Vector(0, 0, 1));
   level.water_alpha = G_GetFloatArg("wateralpha", 0.1);

   lightvolume_color = G_GetVectorArg("lightcolor", Vector(1, 1, 1));
   level.lightvolume_alpha = G_GetFloatArg("lightalpha", 0.5);

   lava_color = G_GetVectorArg("lavacolor", Vector(1.0, 0.3, 0));
   level.lava_alpha = G_GetFloatArg("lavaalpha", 0.6);

   level.water_color = water_color;
   level.lightvolume_color = lightvolume_color;
   level.lava_color = lava_color;

   //
   // reset the earthquake
   //
   level.earthquake = 0;
}

TargetList *World::GetTargetList(str &targetname)
{
   TargetList *targetlist;

   for(int i = 1; i <= targetList.NumObjects(); i++)
   {
      targetlist = targetList.ObjectAt(i);
      if(targetname == targetlist->targetname)
         return targetlist;
   }
   targetlist = new TargetList(targetname);
   targetList.AddObject(targetlist);
   return targetlist;
}

void World::AddTargetEntity(str &targetname, Entity * ent)
{
   TargetList * targetlist;

   targetlist = GetTargetList(targetname);
   targetlist->AddEntity(ent);
}

void World::RemoveTargetEntity(str &targetname, Entity * ent)
{
   TargetList * targetlist;

   targetlist = GetTargetList(targetname);
   targetlist->RemoveEntity(ent);
}

Entity * World::GetNextEntity(str &targetname, Entity * ent)
{
   TargetList * targetlist;

   targetlist = GetTargetList(targetname);
   return targetlist->GetNextEntity(ent);
}

//### added so I can trigger stuff easily from code
void World::ActivateTarget(str &targetname)
{
   TargetList *targetlist = GetTargetList(targetname);
   
   int num = targetlist->list.NumObjects();
   for(int i = 1; i <= num; i++)
   {
      Entity *ent = targetlist->list.ObjectAt(i);

      assert(ent);

      auto event = new Event(EV_Activate);
      event->AddEntity(world);
      ent->ProcessEvent(event);
   }
}
//###

World::~World()
{
   FreeTargetList();
}

void World::FreeTargetList()
{
   int i;
   int num;

   num = targetList.NumObjects();
   for(i = 1; i <= num; i++)
   {
      delete targetList.ObjectAt(i);
   }

   targetList.FreeObjectList();
}

//
// List stuff for targets
//

CLASS_DECLARATION(Class, TargetList, nullptr);

ResponseDef TargetList::Responses[] =
{
   { nullptr, nullptr }
};

TargetList::TargetList(str &tname)
{
   targetname = tname;
}

TargetList::~TargetList()
{
}

void TargetList::AddEntity(Entity * ent)
{
   if(!list.ObjectInList(ent))
   {
      list.AddObject(ent);
   }
}

void TargetList::RemoveEntity(Entity * ent)
{
   if(list.ObjectInList(ent))
   {
      list.RemoveObject(ent);
   }
}

Entity *TargetList::GetNextEntity(Entity * ent)
{
   int index;

   index = 0;
   if(ent)
      index = list.IndexOfObject(ent);
   index++;
   if(index > list.NumObjects())
      return nullptr;
   else
      return list.ObjectAt(index);
}

// EOF

