/*
================================================================
CHECKPOINTS
================================================================

Copyright (C) 2020 by Night Dive Studios, Inc.
All rights reserved.

See the license.txt file for conditions and terms of use for this code.
*/

#ifndef __CHECKPOINTS_H__
#define __CHECKPOINTS_H__

#include "g_local.h"
#include "entity.h"
#include "trigger.h"
#include "player.h"

class EXPORT_FROM_DLL CheckPoint : public Trigger
{
protected:
   int   id;            // id number for this checkpoint
   int   previous_id;   // id number for previous checkpoint
   int   points;        // number of frags awarded when passed
   int   fastestpoints; // number of frags for fastest lap
   int   rockets;
   int   bullets;
   int   mines;
   int   bikehealth;
   int   riderhealth;
   float disabletime;
   float offtimmer;

public:
   CLASS_PROTOTYPE(CheckPoint);

   CheckPoint();
   virtual void CPTouch(Event *ev);
   void         TriggerStuff(Event *ev) ;

   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void CheckPoint::Archive(Archiver &arc)
{
   Trigger::Archive(arc);

   arc.WriteInteger(id);
   arc.WriteInteger(previous_id);
   arc.WriteInteger(points);
   arc.WriteInteger(fastestpoints);
   arc.WriteInteger(rockets);
   arc.WriteInteger(bullets);
   arc.WriteInteger(mines);
   arc.WriteInteger(bikehealth);
   arc.WriteInteger(riderhealth);
   arc.WriteFloat(disabletime);
   arc.WriteFloat(offtimmer);
}

inline EXPORT_FROM_DLL void CheckPoint::Unarchive(Archiver &arc)
{
   Trigger::Unarchive(arc);

   arc.ReadInteger(&id);
   arc.ReadInteger(&previous_id);
   arc.ReadInteger(&points);
   arc.ReadInteger(&fastestpoints);
   arc.ReadInteger(&rockets);
   arc.ReadInteger(&bullets);
   arc.ReadInteger(&mines);
   arc.ReadInteger(&bikehealth);
   arc.ReadInteger(&riderhealth);
   arc.ReadFloat(&disabletime);
   arc.ReadFloat(&offtimmer);
}

#endif /* checkpoints.h */

// EOF

