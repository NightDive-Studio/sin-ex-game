//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/health.h                         $
// $Revision:: 6                                                              $
//   $Author:: Aldie                                                          $
//     $Date:: 10/27/98 5:18p                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Health powerup
// 

#ifndef __HEALTH_H__
#define __HEALTH_H__

#include "g_local.h"
#include "item.h"
#include "sentient.h"
#include "item.h"

class EXPORT_FROM_DLL Health : public Item
{
public:
   CLASS_PROTOTYPE(Health);

   Health();
   virtual void PickupHealth(Event *ev);
};

class EXPORT_FROM_DLL SmallHealth : public Health
{
public:
   CLASS_PROTOTYPE(SmallHealth);
   SmallHealth();
};

class EXPORT_FROM_DLL LargeHealth : public Health
{
public:
   CLASS_PROTOTYPE(LargeHealth);
   LargeHealth();
};

class EXPORT_FROM_DLL MegaHealth : public Health
{
public:
   CLASS_PROTOTYPE(MegaHealth);
   MegaHealth();
};

class EXPORT_FROM_DLL Apple : public Health
{
public:
   CLASS_PROTOTYPE(Apple);
   Apple();
};

class EXPORT_FROM_DLL Banana : public Health
{
public:
   CLASS_PROTOTYPE(Banana);
   Banana();
};

class EXPORT_FROM_DLL Sandwich : public Health
{
public:
   CLASS_PROTOTYPE(Sandwich);
   Sandwich();
};

class EXPORT_FROM_DLL Soda : public Health
{
public:
   CLASS_PROTOTYPE(Soda);
   Soda();
};

//###
class EXPORT_FROM_DLL Poofs : public Health
{
public:
   CLASS_PROTOTYPE(Poofs);
   Poofs();
   virtual void PickupPoofs(Event *ev);
};
//###

#endif /* health.h */

// EOF

