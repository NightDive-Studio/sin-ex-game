//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/securityturret.h                 $
// $Revision:: 11                                                             $
//   $Author:: Aldie                                                          $
//     $Date:: 7/07/98 4:12p                                                  $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// 

#ifndef __SECURITY_TURRET_H__
#define __SECURITY_TURRET_H__

#include "turret.h"

class EXPORT_FROM_DLL TurretTop : public Turret
{
private:
   void        SetupBase();
public:
   CLASS_PROTOTYPE(TurretTop);

   TurretTop();
};

class EXPORT_FROM_DLL TurretBase : public Entity
{
public:
   CLASS_PROTOTYPE(TurretBase);

   TurretBase();
   void        GoUp(Event *ev);
   void        GoDown(Event *ev);
};

#endif /* Turret.h */

// EOF

