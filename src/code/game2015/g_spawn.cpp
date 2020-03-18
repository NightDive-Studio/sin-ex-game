//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/g_spawn.cpp                      $
// $Revision:: 68                                                             $
//   $Author:: Jimdose                                                        $
//     $Date:: 1/29/99 6:14p                                                  $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// 

#include "g_local.h"
#include "class.h"
#include "Entity.h"
#include "g_spawn.h"
#include "navigate.h"
#include <setjmp.h>
#include "player.h"
#include "gravpath.h"
#include "surface.h"
#include "console.h"
#include "object.h"

void G_ExitWithError( void );
extern jmp_buf	G_AbortGame;

SpawnArgsForEntity PersistantData;

typedef struct
{
   char key[64];
   char value[256];
} spawnargs_t;

#define NUM_SPAWN_ARGS 32 

int			numSpawnArgs = 0;
spawnargs_t spawnArgs[ NUM_SPAWN_ARGS ];

/****************************************************************************

  SpawnArg Class Definition

****************************************************************************/

CLASS_DECLARATION(Class, SpawnArg, NULL);

ResponseDef SpawnArg::Responses[] =
{
   { NULL, NULL }
};

SpawnArg::SpawnArg() : Class()
{
   memset(key, 0, sizeof(key));
   memset(value, 0, sizeof(value));
}

SpawnArg::SpawnArg(SpawnArg &arg) 
{
   strcpy(key, arg.key);
   strcpy(value, arg.value);
}

void SpawnArg::Archive(Archiver &arc)
{
   Class::Archive(arc);

   arc.WriteRaw(key, sizeof(key));
   arc.WriteRaw(value, sizeof(value));
}

void SpawnArg::Unarchive(Archiver &arc)
{
   Class::Unarchive(arc);

   arc.ReadRaw(key, sizeof(key));
   arc.ReadRaw(value, sizeof(value));
}

/****************************************************************************

  SpawnArgs Class Definition

****************************************************************************/

CLASS_DECLARATION(Class, SpawnArgs, NULL);

ResponseDef SpawnArgs::Responses[] =
{
   { NULL, NULL }
};

SpawnArgs::SpawnArgs(SpawnArgs &otherlist)
{
   SpawnArg arg1;
   SpawnArg *arg2;
   int num;
   int i;

   num = otherlist.NumArgs();
   for(i = 1; i <= num; i++)
   {
      arg2 = otherlist.argList.AddressOfObjectAt(i);

      strcpy(arg1.key, arg2->key);
      strcpy(arg1.value, arg2->value);

      argList.AddObject(arg1);
   }
   argList.Resize(num);
}

void SpawnArgs::operator = (SpawnArgs &otherlist)
{
   SpawnArg arg1;
   SpawnArg *arg2;
   int num;
   int i;

   argList.ClearObjectList();

   num = otherlist.NumArgs();
   for(i = 1; i <= num; i++)
   {
      arg2 = otherlist.argList.AddressOfObjectAt(i);

      strcpy(arg1.key, arg2->key);
      strcpy(arg1.value, arg2->value);

      argList.AddObject(arg1);
   }
   argList.Resize(num);
}

int SpawnArgs::NumArgs(void)
{
   return argList.NumObjects();
}

void SpawnArgs::SetArgs(void)
{
   SpawnArg arg;
   int i;

   for(i = 0; i < numSpawnArgs; i++)
   {
      strcpy(arg.key, spawnArgs[i].key);
      strcpy(arg.value, spawnArgs[i].value);
      argList.AddObject(arg);
   }
   argList.Resize(numSpawnArgs);
}

void SpawnArgs::RestoreArgs(void)
{
   SpawnArg *arg;
   int i;

   numSpawnArgs = argList.NumObjects();
   for(i = 0; i < numSpawnArgs; i++)
   {
      arg = &argList.ObjectAt(i + 1);
      strcpy(spawnArgs[i].key, arg->key);
      strcpy(spawnArgs[i].value, arg->value);
   }
}

void SpawnArgs::Archive(Archiver &arc)
{
   int i;
   int num;

   Class::Archive(arc);

   num = argList.NumObjects();
   arc.WriteInteger(num);
   for(i = 1; i <= num; i++)
   {
      arc.WriteObject(argList.AddressOfObjectAt(i));
   }
}

void SpawnArgs::Unarchive(Archiver &arc)
{
   int i;
   int num;

   Class::Unarchive(arc);

   argList.FreeObjectList();

   num = arc.ReadInteger();
   argList.Resize(num);
   for(i = 1; i <= num; i++)
   {
      arc.ReadObject(argList.AddressOfObjectAt(i));
   }
}

/****************************************************************************

  SpawnArgGroup Class Definition

****************************************************************************/

CLASS_DECLARATION(Class, SpawnArgGroup, NULL);

ResponseDef SpawnArgGroup::Responses[] =
{
   { NULL, NULL }
};

SpawnArgGroup::SpawnArgGroup(SpawnArgGroup &group)
{
   SpawnArgs *arg;
   int num;
   int i;

   num = group.spawnArgList.NumObjects();
   for(i = 1; i <= num; i++)
   {
      arg = group.spawnArgList.AddressOfObjectAt(i);
      spawnArgList.AddObject(*arg);
   }
   spawnArgList.Resize(num);
}

void SpawnArgGroup::operator = (SpawnArgGroup &group)
{
   SpawnArgs *arg;
   int num;
   int i;

   num = group.spawnArgList.NumObjects();
   for(i = 1; i <= num; i++)
   {
      arg = group.spawnArgList.AddressOfObjectAt(i);
      spawnArgList.AddObject(*arg);
   }
   spawnArgList.Resize(num);
}

int SpawnArgGroup::NumInGroup(void)
{
   return spawnArgList.NumObjects();
}

void SpawnArgGroup::AddArgs(void)
{
   SpawnArgs args;

   args.SetArgs();
   spawnArgList.AddObject(args);
   spawnArgList.Resize(NumInGroup());
}

void SpawnArgGroup::RestoreArgs(int i)
{
   assert((i > 0) && (i <= NumInGroup()));
   if((i <= 0) || (i > NumInGroup()))
   {
      G_InitSpawnArguments();
      return;
   }

   spawnArgList.ObjectAt(i).RestoreArgs();
}

void SpawnArgGroup::Archive(Archiver &arc)
{
   int i;
   int num;

   Class::Archive(arc);

   num = spawnArgList.NumObjects();
   arc.WriteInteger(num);
   for(i = 1; i <= num; i++)
   {
      arc.WriteObject(spawnArgList.AddressOfObjectAt(i));
   }
}

void SpawnArgGroup::Unarchive(Archiver &arc)
{
   int i;
   int num;

   Class::Unarchive(arc);

   spawnArgList.FreeObjectList();

   num = arc.ReadInteger();
   spawnArgList.Resize(num);
   for(i = 1; i <= num; i++)
   {
      arc.ReadObject(spawnArgList.AddressOfObjectAt(i));
   }
}

/****************************************************************************

  SpawnArgsForEntity Class Definition

****************************************************************************/

CLASS_DECLARATION(Class, SpawnArgsForEntity, NULL);

ResponseDef SpawnArgsForEntity::Responses[] =
{
   { NULL, NULL }
};

void SpawnArgsForEntity::Reset(void)
{
   groupList.FreeObjectList();
   entnumList.FreeObjectList();
}

void SpawnArgsForEntity::AddEnt(Entity *ent)
{
   int num;
   SpawnArgGroup group;

   if(ent && ent->isSubclassOf<Sentient>())
   {
      G_InitSpawnArguments();

      entnumList.AddObject(ent->entnum);
      groupList.AddObject(group);
      num = groupList.NumObjects();
      groupList.Resize(num);
      entnumList.Resize(num);
      ((Sentient *)ent)->WritePersistantData(groupList.ObjectAt(num));
   }
}

qboolean SpawnArgsForEntity::RestoreEnt(Entity *ent)
{
   int num;
   SpawnArgGroup *group;

   num = entnumList.IndexOfObject(ent->entnum);
   if(num && ent->isSubclassOf<Sentient>())
   {
      group = groupList.AddressOfObjectAt(num);
      ((Sentient *)ent)->RestorePersistantData(*group);

      G_InitSpawnArguments();

      return true;
   }

   return false;
}

void SpawnArgsForEntity::RestoreEnts(void)
{
   int num;
   int i;
   int entnum;
   edict_t *ent;
   SpawnArgGroup *group;

   num = groupList.NumObjects();
   for(i = 1; i <= num; i++)
   {
      entnum = entnumList.ObjectAt(i);
      ent = &g_edicts[entnum];

      group = groupList.AddressOfObjectAt(i);

      group->RestoreArgs(1);

      game.force_entnum = true;
      game.spawn_entnum = ent->s.number;
      G_CallSpawn();
      game.force_entnum = false;

      if(ent->entity && ent->entity->isSubclassOf<Sentient>())
      {
         static_cast<Sentient *>(ent->entity)->RestorePersistantData(*group);
      }
   }

   Reset();
}

void SpawnArgsForEntity::Archive(Archiver &arc)
{
   int i;
   int num;

   Class::Archive(arc);

   num = groupList.NumObjects();
   arc.WriteInteger(num);
   for(i = 1; i <= num; i++)
   {
      arc.WriteInteger(entnumList.ObjectAt(i));
      arc.WriteObject(groupList.AddressOfObjectAt(i));
   }
}

void SpawnArgsForEntity::Unarchive(Archiver &arc)
{
   int i;
   int num;

   Reset();

   Class::Unarchive(arc);

   num = arc.ReadInteger();
   entnumList.Resize(num);
   groupList.Resize(num);
   for(i = 1; i <= num; i++)
   {
      arc.ReadInteger(entnumList.AddressOfObjectAt(i));
      arc.ReadObject(groupList.AddressOfObjectAt(i));
   }
}

/****************************************************************************

  spawn arg management

****************************************************************************/

void G_SetFloatArg(const char *key, double value)
{
   char text[20];

   snprintf(text, sizeof(text), "%f", value);
   G_SetSpawnArg(key, text);
}

void G_SetIntArg(const char *key, int value)
{
   char text[20];

   snprintf(text, sizeof(text), "%d", value);
   G_SetSpawnArg(key, text);
}

void G_DefaultArg(const char *key, const char *defaultvalue)
{
   if(!G_GetSpawnArg(key))
   {
      G_SetSpawnArg(key, defaultvalue);
   }
}

void G_DefaultFloatArg(const char *key, double defaultvalue)
{
   if(!G_GetSpawnArg(key))
   {
      G_SetFloatArg(key, defaultvalue);
   }
}

void G_DefaultIntArg(const char *key, int defaultvalue)
{
   if(!G_GetSpawnArg(key))
   {
      G_SetIntArg(key, defaultvalue);
   }
}

Vector G_GetVectorArg(const char *key, Vector defaultvalue)
{
   const char *text;

   text = G_GetSpawnArg(key);
   if(text)
   {
      return Vector(text);
   }
   return defaultvalue;
}

float G_GetFloatArg(const char *key, double defaultvalue)
{
   const char *text;

   text = G_GetSpawnArg(key);
   if(text)
   {
      return (float)atof(text);
   }
   return (float)defaultvalue;
}

int G_GetIntArg(const char *key, int defaultvalue)
{
   const char *text;

   text = G_GetSpawnArg(key);
   if(text)
   {
      return atoi(text);
   }
   return defaultvalue;
}

str G_GetStringArg(const char *key, const char *defaultvalue)
{
   const char	*text;
   str			ret;

   text = G_GetSpawnArg(key);
   if(!text)
   {
      text = defaultvalue;
   }

   if(text)
   {
      return text;
   }

   return "";
}

void G_InitSpawnArguments(void)
{
   int i;

   numSpawnArgs = 0;
   for(i = 0; i < NUM_SPAWN_ARGS; i++)
   {
      memset(spawnArgs[i].key, 0, sizeof(spawnArgs[i].key));
      memset(spawnArgs[i].value, 0, sizeof(spawnArgs[i].value));
   }
}

qboolean G_SetSpawnArg(const char *keyname, const char *value)
{
   int i;

   for(i = 0; i < numSpawnArgs; i++)
   {
      if(!strcmp(keyname, spawnArgs[i].key))
      {
         break;
      }
   }

   if(i >= NUM_SPAWN_ARGS)
   {
      return false;
   }

   if(i == numSpawnArgs)
   {
      Q_strlcpy(spawnArgs[i].key, keyname, sizeof(spawnArgs[0].key));
      numSpawnArgs++;
   }

   Q_strlcpy(spawnArgs[i].value, value, sizeof(spawnArgs[0].value));

   return true;
}

const char *G_GetSpawnArg(const char *key, const char *defaultvalue)
{
   int i;

   for(i = 0; i < numSpawnArgs; i++)
   {
      if(!strcmp(key, spawnArgs[i].key))
      {
         return spawnArgs[i].value;
      }
   }

   return defaultvalue;
}

int G_GetNumSpawnArgs(void)
{
   return numSpawnArgs;
}

/*
===============
G_GetClassFromArgs

Finds the spawn function for the entity and returns ClassDef *
===============
*/
const ClassDef *G_GetClassFromArgs()
{
   const char     *classname;
   const ClassDef *cls = nullptr;

   classname = G_GetSpawnArg("classname");

   //
   // check normal spawn functions
   // see if the class name is stored within the model
   //
   if(classname)
   {
      cls = getClassForID(classname);
      if(!cls)
      {
         cls = getClass(classname);
      }
   }

   if(!cls)
   {
      const char *model;

      //
      // get Object in case we cannot find an alternative
      //
      cls = &Object::ClassInfo;
      model = G_GetSpawnArg("model");
      if(model)
      {
         sinmdl_cmd_t *cmds;
         int modelindex;
         int i;

         //
         // get handle to def file
         //
         if((strlen(model) >= 3) && (!strcmpi(&model[strlen(model) - 3], "def")))
         {
            if(!strchr(model, '\\') && !strchr(model, '/'))
            {
               char str[128];
               strcpy(str, "models/");
               strcat(str, model);
               modelindex = gi.modelindex(str);
            }
            else
               modelindex = gi.modelindex(model);
            if(gi.IsModel(modelindex))
            {
               cmds = gi.InitCommands(modelindex);
               if(cmds)
               {
                  for(i=0; i<cmds->num_cmds; i++)
                  {
                     if(!strcmpi(cmds->cmds[i].args[0], "classname"))
                     {
                        cls = getClass(cmds->cmds[i].args[1]);
                        break;
                     }
                  }
                  if(i == cmds->num_cmds)
                     gi.dprintf("Classname %s used, but 'classname' was not found in Initialization commands, using Object.\n", classname);
               }
               else
                  gi.dprintf("Classname %s used, but SINMDL had no Initialization commands, using Object.\n", classname);
            }
            else
               gi.dprintf("Classname %s used, but SINMDL was not valid, using Object.\n", classname);
         }
         else
            gi.dprintf("Classname %s used, but model was not a SINMDL, using Object.\n", classname);
      }
      else
      {
         gi.dprintf("Classname %s' used, but no model was set, using Object.\n", classname);
      }
   }

   return cls;
}

/*
===============
G_CallSpawn

Finds the spawn function for the entity and calls it.
Returns pointer to Entity
===============
*/
Entity *G_CallSpawn()
{
   str             classname;
   const ClassDef *cls;
   Entity         *obj;

   classname = G_GetStringArg("classname");
   cls = G_GetClassFromArgs();
   if(!cls)
   {
      gi.dprintf("%s doesn't have a spawn function\n", classname.c_str());
      G_InitSpawnArguments();
      return NULL;
   }

   obj = (Entity *)cls->newInstance();
   G_InitSpawnArguments();
   if(!obj)
   {
      gi.dprintf("%s failed on newInstance\n", classname.c_str());
      return NULL;
   }

   return obj;
}

/*
====================
G_ParseEdict

Parses an edict out of the given string, returning the new position
ed should be a properly initialized empty edict.
====================
*/
const char *G_ParseEdict(const char *data)
{
   qboolean		init;
   char			keyname[256];
   const char	*com_token;

   init = false;

   G_InitSpawnArguments();

   // go through all the dictionary pairs
   while(1)
   {
      // parse key
      com_token = COM_Parse(&data);
      if(com_token[0] == '}')
      {
         break;
      }

      if(!data)
      {
         gi.error("G_ParseEntity: EOF without closing brace");
      }

      Q_strlcpy(keyname, com_token, sizeof(keyname));

      // parse value	
      com_token = COM_Parse(&data);
      if(!data)
      {
         gi.error("G_ParseEntity: EOF without closing brace");
      }

      if(com_token[0] == '}')
      {
         gi.error("G_ParseEntity: closing brace without data");
      }

      init = true;

      // keynames with a leading underscore are used for utility comments,
      // and are immediately discarded by quake
      if(keyname[0] == '_')
      {
         continue;
      }

      G_SetSpawnArg(keyname, com_token);
   }

   return data;
}

/*
================
G_FindTeams

Chain together all entities with a matching team field.

All but the first will have the FL_TEAMSLAVE flag set.
All but the last will have the teamchain field set to the next one
================
*/
void G_FindTeams(void)
{
   edict_t	*e;
   edict_t	*e2;
   edict_t	*next;
   edict_t	*next2;
   Entity	*chain;
   Entity	*ent;
   Entity	*ent2;
   int		c;
   int		c2;

   c = 0;
   c2 = 0;

   for(e = active_edicts.next; e != &active_edicts; e = next)
   {
      assert(e);
      assert(e->inuse);
      assert(e->entity);

      next = e->next;

      if(e == g_edicts)
      {
         continue;
      }

      ent = e->entity;
      if(!ent->moveteam.length())
      {
         continue;
      }

      if(ent->flags & FL_TEAMSLAVE)
      {
         continue;
      }

      chain = ent;
      ent->teammaster = ent;
      c++;
      c2++;
      for(e2 = next; e2 != &active_edicts; e2 = next2)
      {
         assert(e2);
         assert(e2->inuse);
         assert(e2->entity);

         next2 = e2->next;

         ent2 = e2->entity;
         if(!ent2->moveteam.length())
         {
            continue;
         }

         if(ent2->flags & FL_TEAMSLAVE)
         {
            continue;
         }

         if(ent->moveteam == ent2->moveteam)
         {
            c2++;
            chain->teamchain = ent2;
            ent2->teammaster = ent;
            chain = ent2;
            ent2->flags |= FL_TEAMSLAVE;
         }
      }
   }

   gi.dprintf("%i teams with %i entities\n", c, c2);
}

/*
==============
G_LevelShutdown

Get rid of anything left over from the last level
==============
*/
void G_LevelShutdown(void)
{
   PathManager.SavePaths();

   assert(active_edicts.next);
   assert(active_edicts.next->prev = &active_edicts);
   assert(active_edicts.prev);
   assert(active_edicts.prev->next = &active_edicts);
   assert(free_edicts.next);
   assert(free_edicts.next->prev == &free_edicts);
   assert(free_edicts.prev);
   assert(free_edicts.prev->next == &free_edicts);

   while(active_edicts.next != &active_edicts)
   {
      assert(active_edicts.next != &free_edicts);
      assert(active_edicts.prev != &free_edicts);

      assert(active_edicts.next);
      assert(active_edicts.next->prev = &active_edicts);
      assert(active_edicts.prev);
      assert(active_edicts.prev->next = &active_edicts);
      assert(free_edicts.next);
      assert(free_edicts.next->prev == &free_edicts);
      assert(free_edicts.prev);
      assert(free_edicts.prev->next == &free_edicts);

      if(active_edicts.next->entity)
      {
         delete active_edicts.next->entity;
      }
      else
      {
         G_FreeEdict(active_edicts.next);
      }
   }

   globals.num_edicts = game.maxclients + 1;

   // Reset the gravity paths
   gravPathManager.Reset();

   // close all the scripts
   Director.CloseScript();

   // invalidate player readiness
   Director.PlayerNotReady();

   // clearout any waiting events
   G_ClearEventList();

   gi.FreeTags(TAG_LEVEL);
}

/*
==============
G_ResetEdicts
==============
*/
void G_ResetEdicts(void)
{
   int i;

   memset(g_edicts, 0, game.maxentities * sizeof(g_edicts[0]));

   // Add all the edicts to the free list
   LL_Reset(&free_edicts, next, prev);
   LL_Reset(&active_edicts, next, prev);
   for(i = 0; i < game.maxentities; i++)
   {
      LL_Add(&free_edicts, &g_edicts[i], next, prev);
   }

   for(i=0; i<game.maxclients; i++)
   {
      // set client fields on player ents
      g_edicts[i + 1].client = game.clients + i;
      G_InitClientResp(&game.clients[i]);
   }

   globals.num_edicts = game.maxclients + 1;
}

/*
==============
G_MapInit

Set up for a new map.  This is called for loading savegames, so anything done here should
be compatible with G_ReadGame.
==============
*/
void G_MapInit(const char *mapname)
{
   G_ClearEventList();
   PathManager.Init(mapname);

   // init level and console script variables
   consoleVars.ClearList();
   levelVars.ClearList();
}

/*
==============
G_LevelStart

Does all post-spawning setup.  This is NOT called for savegames.
==============
*/
void G_LevelStart(void)
{
   ScriptThread *gamescript;
   const char   *scriptname;

   // initialize secrets
   levelVars.SetVariable("total_secrets", level.total_secrets);
   levelVars.SetVariable("found_secrets", level.found_secrets);

   G_FindTeams();

   // Create the mission computer
   consoleManager.CreateMissionComputer();

   // call the precache scripts
   G_Precache();

   //
   // start executing the game script
   //
   scriptname = ScriptLib.GetGameScript();
   if(scriptname && scriptname[0])
   {
      gamescript = Director.CreateThread(scriptname, LEVEL_SCRIPT);
      if(gamescript)
      {
         // start at the end of this frame
         gamescript->Start(0);
      }
   }
}

/*
==============
G_Precache

Calls precache scripts
==============
*/
void G_Precache(void)
{
   const char *scriptname;
   int i;

   //
   // load in global0-9.scr
   // 
   for(i = 0; i < 10; i++)
   {
      G_LoadAndExecScript(va("global/global%i.scr", i));
   }

   //
   // load in precache0-9.scr
   // 
   if(precache->value)
   {
      for(i = 0; i < 10; i++)
      {
         G_LoadAndExecScript(va("global/precache%i.scr", i));
      }
   }

   //
   // load in players0-9.scr
   //
   for(i = 0; i< 10; i++)
   {
      G_LoadAndExecScript(va("global/players%i.scr", i));
   }

   //###
   //
   // load in hoverbikes0-9.scr
   //
   for(i = 0; i < 10; i++)
   {
      G_LoadAndExecScript(va("global/hoverbikes%i.scr", i));
   }
   //###

   //
   // load in universal_script.scr
   //
   G_LoadAndExecScript("global/universal_script.scr", "precache:");

   //
   // precache for the game script
   //
   scriptname = ScriptLib.GetGameScript();
   if(scriptname && scriptname[0])
   {
      G_LoadAndExecScript(scriptname, "precache:");
   }
}

/*
==============
G_SpawnEntities

Creates a server's entity / program execution context by
parsing textual entity definitions out of an ent file.
==============
*/
void G_SpawnEntities(const char *mapname, const char *entities, const char *spawnpoint)
{
   int			inhibit;
   const char	*com_token;
   float			skill_level;
   const char	*value;
   int			spawnflags;
   qboolean		world_spawned;
   cvar_t		*lowdetail;
   int         i=0;
#if 0
   Class       *obj;
   Entity      *ent;
#endif

   // If we get an error, call the server's error function
   if(setjmp(G_AbortGame))
   {
      G_ExitWithError();
   }

   lowdetail = gi.cvar("r_lowdetail", "0", CVAR_ARCHIVE);

   // Init the level variables
   level = level_locals_t();
   level.mapname = mapname;
   game.spawnpoint = spawnpoint;

   if(!LoadingServer)
   {
      // Get rid of anything left over from the last level
      G_LevelShutdown();

      G_ResetEdicts();

      // Set up for a new map
      G_MapInit(mapname);
   }

   // Init surface manager & consoles
   surfaceManager.Reset();
   globals.num_surfaces = 0;
   memset(g_surfaces, 0, game.maxsurfaces * sizeof(g_surfaces[0]));

   globals.num_consoles = 0;
   memset(g_consoles, 0, game.maxconsoles * sizeof(g_consoles[0]));

   skill_level = floor(skill->value);
   skill_level = bound(skill_level, 0, 3);
   if(skill->value != skill_level)
   {
      gi.cvar_forceset("skill", va("%f", skill_level));
   }

   gameVars.SetVariable("skill", skill_level);

   // reset out count of the number of game traces
   sv_numtraces = 0;

   level.playerfrozen = false;

   inhibit = 0;
   world_spawned = false;

   // parse ents
   while(1)
   {
      // parse the opening brace	
      com_token = COM_Parse(&entities);
      if(!entities)
      {
         break;
      }
      if(com_token[0] != '{')
      {
         gi.error("G_LoadFromFile: found %s when expecting {", com_token);
      }

      i++;
      if(!(i % 20))
         gi.IncrementStatusCount(20);

      entities = G_ParseEdict(entities);

      // remove things (except the world) from different skill levels or deathmatch
      value = G_GetSpawnArg("spawnflags");
      if(world_spawned && value)
      {
         spawnflags = atoi(value);
         if(deathmatch->value)
         {
            if(spawnflags & SPAWNFLAG_NOT_DEATHMATCH)
            {
               inhibit++;
               continue;
            }
         }
         else
         {
            if(
               ((skill->value == 0) && (spawnflags & SPAWNFLAG_NOT_EASY)) ||
               ((skill->value == 1) && (spawnflags & SPAWNFLAG_NOT_MEDIUM)) ||
               (((skill->value == 2) || (skill->value == 3)) && (spawnflags & SPAWNFLAG_NOT_HARD) ||
               (coop->value && (spawnflags & SPAWNFLAG_NOT_COOP))) ||
                (!developer->value && (spawnflags & SPAWNFLAG_DEVELOPMENT)) ||
               (lowdetail->value && (spawnflags & SPAWNFLAG_DETAIL))
               )
            {
               inhibit++;
               continue;
            }
         }
      }

      game.force_entnum = !world_spawned;
      game.spawn_entnum = 0;
      G_CallSpawn();
      world_spawned = true;

#if 0
      // have to fix G_CallSpawn so that freed entities are accounted for
      if(obj && obj->isSubclassOf(Entity))
      {
         ent = (Entity *)obj;

         // Sanity check to see if we're expecting a B-Model
         assert(!((ent->edict->solid == SOLID_BSP) && !ent->edict->s.modelindex));
         if((ent->edict->solid == SOLID_BSP) && !ent->edict->s.modelindex)
         {
            if(ent->edict->s.number == 0)
            {
               gi.error("No model for worldspawn!");
            }
            else
            {
               gi.dprintf("Deleting %s with SOLID_BSP and no model\n", ent->getClassID());
               delete ent;
            }
         }
      }
#endif
   }

   game.force_entnum = false;
   gi.dprintf("%i entities inhibited\n", inhibit);

   G_InitSpawnArguments();

   if(!LoadingServer || game.autosaved)
   {
      G_LevelStart();
   }
}

/*
=================
G_Spawn

Either finds a free edict, or allocates a new one.
Try to avoid reusing an entity that was recently freed, because it
can cause the client to think the entity morphed into something else
instead of being removed and recreated, which can cause interpolated
angles and bad trails.
=================
*/
edict_t *G_Spawn(void)
{
   int		i;
   edict_t	*e;

   e = &g_edicts[(int)maxclients->value + 1];
   for(i = maxclients->value + 1; i < globals.num_edicts; i++, e++)
   {
      // the first couple seconds of server time can involve a lot of
      // freeing and allocating, so relax the replacement policy
      if(!e->inuse && (e->freetime < 2 || level.time - e->freetime > 0.5))
      {
         assert(e->next);
         assert(e->prev);
         LL_Remove(e, next, prev);
         G_InitEdict(e);
         assert(active_edicts.next);
         assert(active_edicts.prev);
         LL_Add(&active_edicts, e, next, prev);
         assert(e->next);
         assert(e->prev);
         return e;
      }
   }

   if(i == game.maxentities)
   {
      gi.error("G_Spawn: no free edicts");
   }

   globals.num_edicts++;
   assert(e->next);
   assert(e->prev);
   LL_Remove(e, next, prev);
   G_InitEdict(e);
   assert(active_edicts.next);
   assert(active_edicts.prev);
   LL_Add(&active_edicts, e, next, prev);
   assert(e->next);
   assert(e->prev);

   assert(e->next != &free_edicts);
   assert(e->prev != &free_edicts);

   return e;
}

/*
=================
G_FreeEdict

Marks the edict as free
=================
*/
void G_FreeEdict(edict_t *ed)
{
   gclient_t *client;

   assert(ed != &free_edicts);

   // unlink from world
   gi.unlinkentity(ed);

   assert(ed->next);
   assert(ed->prev);

   if(level.next_edict == ed)
   {
      level.next_edict = ed->next;
   }

   LL_Remove(ed, next, prev);

   assert(ed->next == ed);
   assert(ed->prev == ed);
   assert(free_edicts.next);
   assert(free_edicts.prev);

   client = ed->client;
   memset(ed, 0, sizeof(*ed));
   ed->client = client;
   ed->freetime = level.time;
   ed->inuse = false;
   ed->s.number = ed - g_edicts;

   assert(free_edicts.next);
   assert(free_edicts.prev);

   LL_Add(&free_edicts, ed, next, prev);

   assert(ed->next);
   assert(ed->prev);
}

/*
==============
G_InitClientPersistant

This is only called when the game first initializes in single player,
but is called after each death and level change in deathmatch
==============
*/
void G_InitClientPersistant(gclient_t *client)
{
   memset(&client->pers, 0, sizeof(client->pers));

   client->pers.health		= 100;
   client->pers.max_health	= 100;
}

void G_InitClientResp(gclient_t *client)
{
   memset(&client->resp, 0, sizeof(client->resp));
   client->resp.enterframe = level.framenum;
}

// EOF

