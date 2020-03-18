//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/hammer.cpp                          $
// $Revision:: 4                                                              $
//   $Author:: Markd                                                          $
//     $Date:: 10/04/98 10:25p                                                $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Hammer Melee weapon
// 

#include "g_local.h"
#include "item.h"
#include "weapon.h"
#include "fists.h"

class EXPORT_FROM_DLL Hammer : public Fists
{
public:
   CLASS_PROTOTYPE(Hammer);

   Hammer();
   virtual qboolean IsDroppable() override;
};

CLASS_DECLARATION(Fists, Hammer, nullptr);

ResponseDef Hammer::Responses[] =
{
   { nullptr, nullptr }
};

Hammer::Hammer() : Fists()
{
#ifdef SIN_DEMO
   PostEvent(EV_Remove, 0);
   return;
#endif
   SetModels("sledgeham.def", "view_sledge.def");
   SetAmmo(nullptr, 0, 0);
   SetRank(11, 11);
   strike_reach = 48;
   strike_damage = 55;
   SetMaxRange(strike_reach);
   SetType(WEAPON_MELEE);
   kick = 25;
}

qboolean Hammer::IsDroppable()
{
   return false;
}

// EOF

