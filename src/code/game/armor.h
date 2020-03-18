//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/armor.h                          $
// $Revision:: 11                                                             $
//   $Author:: Aldie                                                          $
//     $Date:: 7/24/98 5:03p                                                  $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Standard armor that prevents a percentage of damage per hit.
// 

#ifndef __ARMOR_H__
#define __ARMOR_H__

#include "item.h" 

#define MAX_ARMOR 100

class EXPORT_FROM_DLL Armor : public Item
{
protected:

   virtual void      Setup(const char *model, int amount);
   virtual void      Add(int amount) override;
public:
   CLASS_PROTOTYPE(Armor);
   Armor();
   virtual qboolean  Pickupable(Entity *other) override;
};

class EXPORT_FROM_DLL RiotHelmet : public Armor
{
public:
   CLASS_PROTOTYPE(RiotHelmet);
   RiotHelmet();
};

class EXPORT_FROM_DLL FlakJacket : public Armor
{
public:
   CLASS_PROTOTYPE(FlakJacket);
   FlakJacket();
};

class EXPORT_FROM_DLL FlakPants : public Armor
{
public:
   CLASS_PROTOTYPE(FlakPants);
   FlakPants();
};

#endif /* armor.h */
