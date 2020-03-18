//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/lensflare.h                      $
// $Revision:: 9                                                              $
//   $Author:: Aldie                                                          $
//     $Date:: 10/09/98 9:02p                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:LensFlare effect
// 

#ifndef __LENSFLARE_H__
#define __LENSFLARE_H__

#include "g_local.h"
#include "entity.h"

class EXPORT_FROM_DLL LensFlare : public Entity
{
public:
   CLASS_PROTOTYPE(LensFlare);
   LensFlare();

   void        Activate(Event *ev);
   void        Deactivate(Event *ev);
   void        Lightstyle(Event *ev);
   void        SetLightstyle(Event *ev);
};

#endif /* lensflare.h */

// EOF

