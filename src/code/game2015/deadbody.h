//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/deadbody.h                       $
// $Revision:: 5                                                              $
//   $Author:: Jimdose                                                        $
//     $Date:: 11/18/98 9:18p                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Dead body

#ifndef __DEADBODY_H__
#define __DEADBODY_H__

#include "g_local.h"
#include "sentient.h"

#define BODY_QUEUE_SIZE 4

EXPORT_FROM_DLL void InitializeBodyQueue(void);
EXPORT_FROM_DLL void CopyToBodyQueue(edict_t *ent);

class EXPORT_FROM_DLL Deadbody : public Sentient
{
public:
   CLASS_PROTOTYPE(Deadbody);

   virtual void   GibEvent(Event *ev);
   //###
   virtual void   StopBurning(Event *ev);
   virtual void   BodyFade(Event *ev);
   //###
};

#endif /* deadbody.h */

// EOF

