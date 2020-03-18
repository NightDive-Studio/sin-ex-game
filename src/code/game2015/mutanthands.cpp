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

CLASS_DECLARATION(Fists, MutantHands, nullptr);

ResponseDef MutantHands::Responses[] =
{
   { nullptr, nullptr }
};

MutantHands::MutantHands() : Fists()
{
#ifdef SIN_DEMO
   PostEvent(EV_Remove, 0);
   return;
#endif
   SetModels(nullptr, "view_mutanthands.def");
   SetAmmo(nullptr, 0, 0);
   SetRank(11, 11);
   strike_reach = 75;
   strike_damage = 100;
   SetMaxRange(strike_reach);
   SetType(WEAPON_MELEE);
   kick = 25;
   meansofdeath = MOD_MUTANTHANDS;
}

// EOF

