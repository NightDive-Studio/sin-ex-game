//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/bacrodai.h                       $
// $Revision:: 2                                                              $
//   $Author:: Markd                                                          $
//     $Date:: 10/20/98 3:25a                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// $Log:: /Quake 2 Engine/Sin/code/game/bacrodai.h                            $
// 
// 2     10/20/98 3:25a Markd
// first time
// 
// 1     10/20/98 3:10a Markd
// 
// DESCRIPTION:
// Bacrodai
// 

#ifndef __BACRODAI_H__
#define __BACRODAI_H__

#include "g_local.h"
#include "actor.h"

class EXPORT_FROM_DLL Bacrodai : public Actor
{
public:
   CLASS_PROTOTYPE(Bacrodai);

   Bacrodai();
   virtual void SpawnBat(Event *ev);
};

#endif /* assaultrifle.h */

// EOF

