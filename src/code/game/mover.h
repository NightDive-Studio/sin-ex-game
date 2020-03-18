//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/mover.h                          $
// $Revision:: 10                                                             $
//   $Author:: Markd                                                          $
//     $Date:: 9/28/98 9:12p                                                  $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Base class for any object that needs to move to specific locations over a
// period of time.  This class is kept separate from most entities to keep
// class size down for objects that don't need such behavior.
// 

#ifndef __MOVER_H__
#define __MOVER_H__

#include "g_local.h"
#include "entity.h"
#include "trigger.h"

class EXPORT_FROM_DLL Mover : public Trigger
{
private:
   Vector            finaldest;
   Vector            angledest;
   Event             endevent;
   int               moveflags;

public:
   CLASS_PROTOTYPE(Mover);

   void              MoveDone(Event *ev);
   void              MoveTo(Vector tdest, Vector angdest, float tspeed, Event &event);
   void              LinearInterpolate(Vector tdest, Vector angdest, float time, Event &event);
   virtual void      Archive(Archiver &arc)   override;
   virtual void      Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void Mover::Archive(Archiver &arc)
{
   Trigger::Archive(arc);

   arc.WriteVector(finaldest);
   arc.WriteVector(angledest);
   arc.WriteEvent(endevent);
   arc.WriteInteger(moveflags);
}

inline EXPORT_FROM_DLL void Mover::Unarchive(Archiver &arc)
{
   Trigger::Unarchive(arc);

   arc.ReadVector(&finaldest);
   arc.ReadVector(&angledest);
   arc.ReadEvent(&endevent);
   arc.ReadInteger(&moveflags);
}

#endif

// EOF

