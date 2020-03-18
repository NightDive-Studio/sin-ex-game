//-----------------------------------------------------------------------------
//
// Mortician header file by Boon, created 11-11-98
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.//
//
// DESCRIPTION:
// Mortician
// 

#ifndef __MORTICIAN_H__
#define __MORTICIAN_H__

#include "g_local.h"
#include "actor.h"

class EXPORT_FROM_DLL Mortician : public Actor 
{
public:
   CLASS_PROTOTYPE(Mortician);

   virtual void JumpToEvent(Event *ev);
   virtual void FlashEvent(Event *ev);
   float        JumpTo(const Vector &targ); //Subtle alteration of regular jumpto
};

class EXPORT_FROM_DLL MJump : public Behavior 
{
private:
   float     endtime        = 0.0f;
   str       anim;                     //Temp variable for assembling animation names in
   str       jumpdir;                  //Used for the direction part of the animation name to be played
   int       state          = 0;
   qboolean  animdone       = true;
   Vector    goal;          
   Vector    temp_vel;                 //Used for destructive testing of velocity
   qboolean  accurate;                 //Do we need to land exactly there, or only near there?
   EntityPtr target         = nullptr; //Used to tell him where to face if he's jumping around the player
   float     heightwanted   = 400.0f;  // Height of ceiling which allows unrestricted jumping
   float     heightneeded   = 140.0f;  // Minimum height of ceiling for reasonable jumping
   qboolean  jumpok         = true;    // Set to false if there is not enough ceiling height
   float     oldworldheight;           //Since self.velocity doesn't work real reliably due to animation vectors, I'm making my own
   float     upspeed;                  // Keep a flag on vertical velocity
   qboolean  jumpbegun      = false;   //Flag to note that we have taken off

   Vector    FindCloseSightNodeTo(Actor &self, const Vector &pos, float maxjumpdistance);

public:
   CLASS_PROTOTYPE(MJump);

   void             SetArgs(Event *ev);
   virtual void     ShowInfo(Actor &self)    override;
   virtual void     Begin(Actor &self)       override;
   virtual qboolean Evaluate(Actor &self)    override;
   virtual void     End(Actor &self)         override;
   void             AnimDone(Event *ev);
   virtual void     Archive(Archiver &arc)   override;
   virtual void     Unarchive(Archiver &arc) override;
};


#endif /* mortician.h */

// EOF

