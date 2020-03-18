/*
================================================================
BOB MONSTER
================================================================

Copyright (C) 2020 by Night Dive Studios, Inc.
All rights reserved.

See the license.txt file for conditions and terms of use for this code.
*/

#ifndef __BOB_H__
#define __BOB_H__

#include "g_local.h"
#include "actor.h"
#include "trigger.h"
#include "concussion.h"
#include "crossbow.h"

// variant of the Plasma Bow that's the secondary fire only
class EXPORT_FROM_DLL BobBow : public Weapon
{
public:
   CLASS_PROTOTYPE(BobBow);

   BobBow();
   virtual void Shoot(Event *ev);
};

extern Event EV_Bob_Concussion;

class EXPORT_FROM_DLL Bob :public Actor
{
private:
   float  concussiontime;

   int    blastcount;  // fire counter
   Vector lastfirepos; // where it last fired from
   Vector lastfiredir; // direction it last fired in

public:
   CLASS_PROTOTYPE(Bob);

   Bob();
   virtual void Prethink() override;
   void         Killed(Event *ev);
   qboolean     InConcussionFOV(Vector pos);
   void         FireConcussion(Event *ev);
   void         ConcussionEffect(Event *ev);

   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void Bob::Archive(Archiver &arc)
{
   Actor::Archive(arc);

   arc.WriteFloat(concussiontime);
   arc.WriteInteger(blastcount);
   arc.WriteVector(lastfirepos);
   arc.WriteVector(lastfiredir);
}

inline EXPORT_FROM_DLL void Bob::Unarchive(Archiver &arc)
{
   Actor::Unarchive(arc);

   arc.ReadFloat(&concussiontime);
   arc.ReadInteger(&blastcount);
   arc.ReadVector(&lastfirepos);
   arc.ReadVector(&lastfiredir);
}

#endif

// EOF

