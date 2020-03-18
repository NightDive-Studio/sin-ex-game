//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/PlayerStart.h                    $
// $Revision:: 10                                                             $
//   $Author:: Jimdose                                                        $
//     $Date:: 12/10/98 1:45p                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Player start location entity declarations
// 

#ifndef __PLAYERSTART_H__
#define __PLAYERSTART_H__

#include "g_local.h"
#include "entity.h"
#include "camera.h"

class EXPORT_FROM_DLL PlayerStart : public Entity
{
public:
   CLASS_PROTOTYPE(PlayerStart);

   PlayerStart();
};

class EXPORT_FROM_DLL ProgressiveStart : public PlayerStart
{
public:
   CLASS_PROTOTYPE(ProgressiveStart);

   ProgressiveStart();
   void  SetSpawnpoint(Event *ev);
};

class EXPORT_FROM_DLL TestPlayerStart : public PlayerStart
{
public:
   CLASS_PROTOTYPE(TestPlayerStart);
};

class EXPORT_FROM_DLL PlayerDeathmatchStart : public PlayerStart
{
public:
   CLASS_PROTOTYPE(PlayerDeathmatchStart);
};

class EXPORT_FROM_DLL PlayerCoopStart : public PlayerStart
{
public:
   CLASS_PROTOTYPE(PlayerCoopStart);
};

class EXPORT_FROM_DLL PlayerIntermission : public Camera
{
public:
   CLASS_PROTOTYPE(PlayerIntermission);
   PlayerIntermission();
};

#endif /* PlayerStart.h */

// EOF

