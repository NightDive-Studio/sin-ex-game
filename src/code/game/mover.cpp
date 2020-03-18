//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/mover.cpp                        $
// $Revision:: 18                                                             $
//   $Author:: Jimdose                                                        $
//     $Date:: 10/24/98 8:30p                                                 $
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

#include "g_local.h"
#include "entity.h"
#include "trigger.h"
#include "mover.h"

#define MOVE_ANGLES 1
#define MOVE_ORIGIN 2

CLASS_DECLARATION(Trigger, Mover, "mover");

ResponseDef Mover::Responses[] =
{
   { &EV_MoveDone, (Response)&Mover::MoveDone },
   { NULL, NULL }
};

EXPORT_FROM_DLL void Mover::MoveDone(Event *ev)
{
   Vector move;
   Vector amove;

   // zero out the movement
   if(moveflags & MOVE_ANGLES)
   {
      avelocity = vec_zero;
      amove = angledest - angles;
   }
   else
   {
      amove = vec_zero;
   }

   if(moveflags & MOVE_ORIGIN)
   {
      velocity = vec_zero;
      move = finaldest - origin;
   }
   else
   {
      move = vec_zero;
   }

   if(!G_PushMove(this, move, amove))
   {
      // Delay finish till we can move into the final position
      PostEvent(EV_MoveDone, FRAMETIME);
      return;
   }

   //
   // After moving, set origin to exact final destination
   //
   if(moveflags & MOVE_ORIGIN)
   {
      setOrigin(finaldest);
   }

   if(moveflags & MOVE_ANGLES)
   {
      angles = angledest;

      if((angles.x >= 360) || (angles.x < 0))
      {
         angles.x -= ((int)angles.x / 360) * 360;
      }
      if((angles.y >= 360) || (angles.y < 0))
      {
         angles.y -= ((int)angles.y / 360) * 360;
      }
      if((angles.z >= 360) || (angles.z < 0))
      {
         angles.z -= ((int)angles.z / 360) * 360;
      }
   }

   ProcessEvent(endevent);
}

/*
=============
MoveTo

calculate self.velocity and self.nextthink to reach dest from
self.origin traveling at speed
===============
*/
EXPORT_FROM_DLL void Mover::MoveTo(Vector tdest, Vector angdest, float tspeed, Event &event)
{
   Vector vdestdelta;
   Vector angdestdelta;
   float  len;
   float  traveltime;

   assert(tspeed >= 0);

   if(!tspeed)
   {
      error("MoveTo", "No speed is defined!");
   }

   if(tspeed < 0)
   {
      error("MoveTo", "Speed is negative!");
   }

   // Cancel previous moves
   CancelEventsOfType(EV_MoveDone);

   moveflags = 0;

   endevent = event;
   finaldest = tdest;
   angledest = angdest;

   if(finaldest != origin)
   {
      moveflags |= MOVE_ORIGIN;
   }
   if(angledest != angles)
   {
      moveflags |= MOVE_ANGLES;
   }

   if(!moveflags)
   {
      // stop the object from moving
      velocity = vec_zero;
      avelocity = vec_zero;

      // post the event so we don't wait forever
      PostEvent(EV_MoveDone, FRAMETIME);
      return;
   }

   // set destdelta to the vector needed to move
   vdestdelta = tdest - origin;
   angdestdelta = angdest - angles;

   if(tdest == origin)
   {
      // calculate length of vector based on angles
      len = angdestdelta.length();
   }
   else
   {
      // calculate length of vector based on distance
      len = vdestdelta.length();
   }

   // divide by speed to get time to reach dest
   traveltime = len / tspeed;

   // Quantize to FRAMETIME
   traveltime *= (1 / FRAMETIME);
   traveltime = (float)((int)traveltime) * FRAMETIME;
   if(traveltime < FRAMETIME)
   {
      traveltime = FRAMETIME;
      vdestdelta = vec_zero;
      angdestdelta = vec_zero;
   }

   // scale the destdelta vector by the time spent traveling to get velocity
   if(moveflags & MOVE_ORIGIN)
   {
      velocity = vdestdelta * (1 / traveltime);
   }

   if(moveflags & MOVE_ANGLES)
   {
      avelocity = angdestdelta * (1 / traveltime);
   }

   PostEvent(EV_MoveDone, traveltime);
}

/*
=============
LinearInterpolate
===============
*/
EXPORT_FROM_DLL void Mover::LinearInterpolate(Vector tdest, Vector angdest, float time, Event &event)
{
   Vector vdestdelta;
   Vector angdestdelta;
   float t;

   endevent = event;
   finaldest = tdest;
   angledest = angdest;

   // Cancel previous moves
   CancelEventsOfType(EV_MoveDone);

   // Quantize to FRAMETIME
   time *= (1 / FRAMETIME);
   time = (float)((int)time) * FRAMETIME;
   if(time < FRAMETIME)
   {
      time = FRAMETIME;
   }

   moveflags = 0;
   t = 1 / time;
   // scale the destdelta vector by the time spent traveling to get velocity
   if(finaldest != origin)
   {
      vdestdelta = tdest - origin;
      velocity = vdestdelta * t;
      moveflags |= MOVE_ORIGIN;
   }

   if(angledest != angles)
   {
      angdestdelta = angdest - angles;
      avelocity = angdestdelta * t;
      moveflags |= MOVE_ANGLES;
   }

   PostEvent(EV_MoveDone, time);
}

// EOF

