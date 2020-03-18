//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/mutanthands.h                       $
// $Revision:: 5                                                              $
//   $Author:: Markd                                                          $
//     $Date:: 9/22/98 12:49p                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Mutant Hands
// 

#ifndef __MUTANTHANDS_H__
#define __MUTANTHANDS_H__

#include "g_local.h"
#include "item.h"
#include "weapon.h"
#include "fists.h"

class EXPORT_FROM_DLL MutantHands : public Fists
{
public:
   CLASS_PROTOTYPE(MutantHands);

   MutantHands();
};

#endif /* MutantHands.h */
