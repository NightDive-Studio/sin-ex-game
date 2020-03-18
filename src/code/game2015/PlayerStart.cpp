//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/PlayerStart.cpp                  $
// $Revision:: 14                                                             $
//   $Author:: Jimdose                                                        $
//     $Date:: 12/15/98 6:17p                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Player start location entity declarations
// 

#include "g_local.h"
#include "entity.h"
#include "trigger.h"
#include "PlayerStart.h"

//### added start on bike flag
/*****************************************************************************/
/*SINED info_player_start (1 0 0) (-16 -16 0) (16 16 64) ONBIKE

The normal starting point for a level.

/*****************************************************************************/

CLASS_DECLARATION(Entity, PlayerStart, "info_player_start");

ResponseDef PlayerStart::Responses[] =
{
   { nullptr, nullptr }
};

PlayerStart::PlayerStart() : Entity()
{
   float angle;

   angle = G_GetFloatArg("angle", 0);
   angles = Vector(0, angle, 0);
}

//### added start on bike flag
/*****************************************************************************/
/*SINED info_player_progressivestart (1 0 0) (-16 -16 0) (16 16 64) ONBIKE

Starting point for a level.  When triggered, sets next spawnpoint to itself.
Used for respawn point in Sin Arcade.  This must have a targetname for it to work.

"starthere" set to 1 to make this the default spawnpoint.  (Note: multiple progressive
starts with starthere set will override each other).

/*****************************************************************************/

CLASS_DECLARATION(PlayerStart, ProgressiveStart, "info_player_progressivestart");

ResponseDef ProgressiveStart::Responses[] =
{
   { &EV_Activate,		   (Response)&ProgressiveStart::SetSpawnpoint },
   { nullptr, nullptr }
};

ProgressiveStart::ProgressiveStart() : PlayerStart()
{
   if(!targetname.length())
   {
      gi.error("ProgressiveStart without targetname at (%f,%f,%f)\n", origin.x, origin.y, origin.z);
   }

   if(G_GetIntArg("starthere"))
   {
      game.spawnpoint = targetname;
   }
}

void ProgressiveStart::SetSpawnpoint(Event *ev)
{
   game.spawnpoint = targetname;
}

/*****************************************************************************/
/*  saved out by quaked in region mode

/*****************************************************************************/

CLASS_DECLARATION(PlayerStart, TestPlayerStart, "testplayerstart");

ResponseDef TestPlayerStart::Responses[] =
{
   { nullptr, nullptr }
};

//### added start on bike flag
/*****************************************************************************/
/*SINED info_player_deathmatch (1 0 1) (-16 -16 0) (16 16 64) ONBIKE

potential spawning position for deathmatch games

/*****************************************************************************/

CLASS_DECLARATION(PlayerStart, PlayerDeathmatchStart, "info_player_deathmatch");

ResponseDef PlayerDeathmatchStart::Responses[] =
{
   { nullptr, nullptr }
};

//### added start on bike flag
/*****************************************************************************/
/*SINED info_player_coop (1 0 1) (-16 -16 0) (16 16 64) ONBIKE)

potential spawning position for coop games

/*****************************************************************************/

CLASS_DECLARATION(PlayerStart, PlayerCoopStart, "info_player_coop");

ResponseDef PlayerCoopStart::Responses[] =
{
   { nullptr, nullptr }
};

/*****************************************************************************/
/*SINED info_player_intermission (1 0 1) (-16 -16 0) (16 16 64)

viewing point in between deathmatch levels

/*****************************************************************************/

CLASS_DECLARATION(Camera, PlayerIntermission, "info_player_intermission");

ResponseDef PlayerIntermission::Responses[] =
{
   { nullptr, nullptr }
};

PlayerIntermission::PlayerIntermission() : Camera()
{
   currentstate.watch.splineangles = false;
   newstate.watch.splineangles = false;
}

// EOF

