//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/mutanthands.cpp                     $
// $Revision:: 11                                                             $
//   $Author:: Aldie                                                          $
//     $Date:: 10/11/98 5:32p                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Mutant Hands
// 

#include "g_local.h"
#include "mutanthands.h"

CLASS_DECLARATION(Fists, MutantHands, NULL);

ResponseDef MutantHands::Responses[] =
{
   { NULL, NULL }
};

MutantHands::MutantHands() : Fists()
{
#ifdef SIN_DEMO
   PostEvent(EV_Remove, 0);
   return;
#endif
   SetModels(NULL, "view_mutanthands.def");
   SetAmmo(NULL, 0, 0);
   SetRank(11, 11);
   strike_reach = 75;
   strike_damage = 100;
   SetMaxRange(strike_reach);
   SetType(WEAPON_MELEE);
   kick = 25;
   meansofdeath = MOD_MUTANTHANDS;
}

// EOF

