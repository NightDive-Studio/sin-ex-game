//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/g_spawn.h                        $
// $Revision:: 18                                                             $
//   $Author:: Jimdose                                                        $
//     $Date:: 11/07/98 10:01p                                                $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// 

#ifndef __G_SPAWN_H__
#define __G_SPAWN_H__

#ifdef __cplusplus

#include "entity.h"

// spawnflags
// these are set with checkboxes on each entity in the map editor
#define SPAWNFLAG_NOT_EASY       0x00000100
#define SPAWNFLAG_NOT_MEDIUM     0x00000200
#define SPAWNFLAG_NOT_HARD       0x00000400
#define SPAWNFLAG_NOT_DEATHMATCH 0x00000800
#define SPAWNFLAG_NOT_COOP       0x00001000
#define SPAWNFLAG_DEVELOPMENT    0x00002000
#define SPAWNFLAG_DETAIL         0x00004000

class EXPORT_FROM_DLL SpawnArg : public Class
{
public:
   char           key[64];
   char           value[256];

   CLASS_PROTOTYPE(SpawnArg);

   SpawnArg();
   SpawnArg(SpawnArg &arg);
   friend  int    operator == (SpawnArg a, SpawnArg b);
   virtual void   Archive(Archiver &arc)   override;
   virtual void   Unarchive(Archiver &arc) override;
};

inline int operator == (SpawnArg a, SpawnArg b)
{
   int i;

   i = strcmp(a.key, b.key);
   if(i)
   {
      return i;
   }

   return strcmp(a.value, b.value);
}

#ifdef EXPORT_TEMPLATE
template class EXPORT_FROM_DLL Container<SpawnArg>;
#endif

class EXPORT_FROM_DLL SpawnArgs : public Class
{
public:
   Container<SpawnArg> argList;

   CLASS_PROTOTYPE(SpawnArgs);

   SpawnArgs() = default;
   SpawnArgs(SpawnArgs &arglist);
   int            NumArgs();
   void           SetArgs();
   void           RestoreArgs();
   void           operator = (SpawnArgs &a);
   friend  int    operator == (SpawnArgs a, SpawnArgs b);
   virtual void   Archive(Archiver &arc)   override;
   virtual void   Unarchive(Archiver &arc) override;
};

inline int operator == (SpawnArgs a, SpawnArgs b)
{
   return -1;
}

#ifdef EXPORT_TEMPLATE
template class EXPORT_FROM_DLL Container<SpawnArgs>;
#endif

class EXPORT_FROM_DLL SpawnArgGroup : public Class
{
public:
   Container<SpawnArgs> spawnArgList;

   CLASS_PROTOTYPE(SpawnArgGroup);

   SpawnArgGroup() = default;
   SpawnArgGroup(SpawnArgGroup &group);
   int            NumInGroup();
   void           AddArgs();
   void           RestoreArgs(int i);
   void           operator = (SpawnArgGroup &a);
   friend  int    operator == (SpawnArgGroup a, SpawnArgGroup b);
   virtual void   Archive(Archiver &arc)   override;
   virtual void   Unarchive(Archiver &arc) override;
};

inline int operator == (SpawnArgGroup a, SpawnArgGroup b)
{
   return -1;
}

#ifdef EXPORT_TEMPLATE
template class EXPORT_FROM_DLL Container<SpawnArgGroup>;
#endif

class EXPORT_FROM_DLL SpawnArgsForEntity : public Class
{
public:
   Container<SpawnArgGroup>   groupList;
   Container<int>             entnumList;

   CLASS_PROTOTYPE(SpawnArgsForEntity);

   void           Reset(void);
   void           AddEnt(Entity *ent);
   qboolean       RestoreEnt(Entity *ent);
   void           RestoreEnts(void);
   virtual void   Archive(Archiver &arc)   override;
   virtual void   Unarchive(Archiver &arc) override;
};

extern SpawnArgsForEntity PersistantData;

void        G_SetFloatArg(const char *key, double value);
void        G_SetIntArg(const char *key, int value);
qboolean    G_SetSpawnArg(const char *keyname, const char *value);

void        G_DefaultArg(const char *key, const char *defaultvalue);
void        G_DefaultFloatArg(const char *key, double defaultvalue);
void        G_DefaultIntArg(const char *key, int defaultvalue);

float       G_GetFloatArg(const char *key, double defaultvalue = 0);
Vector      G_GetVectorArg(const char *key, Vector defaultvalue = Vector(0, 0, 0));
int         G_GetIntArg(const char *key, int defaultvalue = 0);
str         G_GetStringArg(const char *key, const char *defaultvalue = NULL);
const char *G_GetSpawnArg(const char *key, const char *defaultvalue = NULL);

void        G_InitSpawnArguments(void);
int         G_GetNumSpawnArgs(void);

void        G_InitClientPersistant(gclient_t *client);
void        G_InitClientResp(gclient_t *client);

const ClassDef   *G_GetClassFromArgs(void);
Entity     *G_CallSpawn(void);
const char *G_ParseEdict(const char *data, edict_t *ent);
void        G_FindTeams(void);

void        G_LevelShutdown(void);
void        G_ResetEdicts(void);
void        G_MapInit(const char *mapname);
void        G_LevelStart(void);
void        G_Precache(void);

#endif

#ifdef __cplusplus
extern "C" {
#endif

EXPORT_FROM_DLL void G_SpawnEntities(const char *mapname, const char *entities, const char *spawnpoint);

#ifdef __cplusplus
}
#endif

#endif

// EOF

