//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/fists.h                             $
// $Revision:: 7                                                              $
//   $Author:: Aldie                                                          $
//     $Date:: 10/11/98 5:34p                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Mutant Hands
// 

#ifndef __FISTS_H__
#define __FISTS_H__

#include "g_local.h"
#include "item.h"
#include "weapon.h"

class EXPORT_FROM_DLL Fists : public Weapon
{
public:
   float             strike_reach;
   float             strike_damage;
   int               meansofdeath;

   CLASS_PROTOTYPE(Fists);

   Fists();
   virtual void      Shoot(Event *ev);
   virtual void      SecondaryUse(Event *ev)  override; //###
   virtual void      Archive(Archiver &arc)   override;
   virtual void      Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void Fists::Archive(Archiver &arc)
{
   Weapon::Archive(arc);

   arc.WriteFloat(strike_reach);
   arc.WriteFloat(strike_damage);
   arc.WriteInteger(meansofdeath);
}

inline EXPORT_FROM_DLL void Fists::Unarchive(Archiver &arc)
{
   Weapon::Unarchive(arc);

   arc.ReadFloat(&strike_reach);
   arc.ReadFloat(&strike_damage);
   arc.ReadInteger(&meansofdeath);
}

#endif /* Fists.h */

// EOF

