//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/eonandpeon.h                     $
// $Revision:: 5                                                              $
//   $Author:: Jimdose                                                        $
//     $Date:: 11/08/98 10:51p                                                $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
// 
// DESCRIPTION:
// Eon and Peon
// 

#ifndef __EONANDPEON_H__
#define __EONANDPEON_H__

#include "g_local.h"
#include "actor.h"
#include "peon.h"

class EXPORT_FROM_DLL EonAndPeon : public Peon
{
private:
   EntityPtr         eon;

public:
   CLASS_PROTOTYPE(EonAndPeon);

   EonAndPeon();
   virtual void Postthink() override;
   void         Killed(Event *ev);
   virtual void Chatter(const char *sound, float chance = 10, float volume = 1.0f, int channel = CHAN_VOICE) override;
   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void EonAndPeon::Archive(Archiver &arc)
{
   Peon::Archive(arc);

   arc.WriteSafePointer(eon);
}

inline EXPORT_FROM_DLL void EonAndPeon::Unarchive(Archiver &arc)
{
   Peon::Unarchive(arc);

   arc.ReadSafePointer(&eon);
}

#endif /* eonandpeon.h */

// EOF

