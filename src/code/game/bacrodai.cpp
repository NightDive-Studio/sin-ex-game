//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/bacrodai.cpp                     $
// $Revision:: 3                                                              $
//   $Author:: Markd                                                          $
//     $Date:: 10/22/98 7:57p                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
// 
// DESCRIPTION:
// Bacrodai
// 

#include "g_local.h"
#include "actor.h"
#include "bacrodai.h"

CLASS_DECLARATION(Actor, Bacrodai, "monster_bachrodai");

Event EV_Bacrodai_SpawnBat("spawnbat");

ResponseDef Bacrodai::Responses[] =
{
   { &EV_Bacrodai_SpawnBat, (Response)&Bacrodai::SpawnBat },
   { NULL, NULL }
};

Bacrodai::Bacrodai()
{
   setModel("bacrodai.def");
   modelIndex("bat.def");
}

void Bacrodai::SpawnBat(Event *ev)
{
   Actor * ent;
   str text;
   Vector pos;

   if(!currentEnemy)
      return;

   pos = centroid - (Vector(orientation[0]) * 48);

   // create a new entity
   G_InitSpawnArguments();

   G_SetSpawnArg("model", "bat.def");

   text = va("%f %f %f", pos[0], pos[1], pos[2]);
   G_SetSpawnArg("origin", text.c_str());

   ent = new Actor;

   G_InitSpawnArguments();

   ent->MakeEnemy(currentEnemy, true);
}

// EOF

