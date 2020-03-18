//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/ammo.h                           $
// $Revision:: 19                                                             $
//   $Author:: Aldie                                                          $
//     $Date:: 10/24/98 2:09p                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Base class for all ammunition for entities derived from the Weapon class.
// 

#ifndef __AMMO_H__
#define __AMMO_H__

#include "g_local.h"
#include "item.h"
#include "sentient.h"
#include "item.h"

class EXPORT_FROM_DLL Ammo : public Item
{
protected:
   virtual void Setup(const char *model);

public:
   CLASS_PROTOTYPE(Ammo);

   Ammo();
};

class EXPORT_FROM_DLL Bullet10mm : public Ammo
{
public:
   CLASS_PROTOTYPE(Bullet10mm);

   Bullet10mm();
};

class EXPORT_FROM_DLL Bullet50mm : public Ammo
{
public:
   CLASS_PROTOTYPE(Bullet50mm);

   Bullet50mm();
};

class EXPORT_FROM_DLL BulletPulse : public Ammo
{
public:
   CLASS_PROTOTYPE(BulletPulse);

   BulletPulse();
};

class EXPORT_FROM_DLL BulletSniper : public Ammo
{
public:
   CLASS_PROTOTYPE(BulletSniper);

   BulletSniper();
};

class EXPORT_FROM_DLL Rockets : public Ammo
{
public:
   CLASS_PROTOTYPE(Rockets);

   Rockets();
};

class EXPORT_FROM_DLL Spears : public Ammo
{
public:
   CLASS_PROTOTYPE(Spears);

   Spears();
};

class EXPORT_FROM_DLL ShotgunClip : public Ammo
{
public:
   CLASS_PROTOTYPE(ShotgunClip);

   ShotgunClip();
};

class EXPORT_FROM_DLL SpiderMines : public Ammo
{
public:
   CLASS_PROTOTYPE(SpiderMines);

   SpiderMines();
};

#endif /* ammo.h */

// EOF

