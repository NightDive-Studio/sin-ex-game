//-----------------------------------------------------------------------------
//
// Splitter header file by Boon, created 25-11-98
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.//
//
// DESCRIPTION:
// Splitter
// 

#ifndef __SPLITTER_H__
#define __SPLITTER_H__

#include "g_local.h"
#include "actor.h"

class EXPORT_FROM_DLL Splitter : public Actor
{
public:
   CLASS_PROTOTYPE(Splitter);

   Splitter();
   virtual void      KilledEvent(Event *ev);
   virtual void      SpawnBugEvent(Event *ev);
   virtual void      Archive(Archiver &arc)   override;
   virtual void      Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void Splitter::Archive(Archiver &arc)
{
   Actor::Archive(arc);

   // FIXME add saving of state so splitters who are dying stay dying through save games
   // arc.WriteFloat( splitterstate_or_something );
}

inline EXPORT_FROM_DLL void Splitter::Unarchive(Archiver &arc)
{
   Actor::Unarchive(arc);

   // FIXME add loading of state so splitters who are dying stay dying through save games
   // splitterstate_or_something = arc.ReadFloat();
}

//### for bug
class EXPORT_FROM_DLL Jump2 : public Jump
{
public:
   CLASS_PROTOTYPE(Jump2);
   virtual void Begin(Actor &self) override;
};
//###

#endif /* splitter.h */

// EOF

