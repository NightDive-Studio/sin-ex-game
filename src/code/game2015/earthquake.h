//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/earthquake.h                     $
// $Revision:: 8                                                              $
//   $Author:: Jimdose                                                        $
//     $Date:: 11/08/98 10:47p                                                $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Earthquake trigger causes a localized earthquake when triggered.
// The earthquake effect is visible to the user as the shaking of his screen.
// 

#ifndef __EARTHQUAKE_H__
#define __EARTHQUAKE_H__

#include "g_local.h"
#include "trigger.h"

#define EARTHQUAKE_STRENGTH 100

class EXPORT_FROM_DLL Earthquake : public Trigger
{
protected:
   qboolean quakeactive;
   float    duration;

public:
   CLASS_PROTOTYPE(Earthquake)
   Earthquake();
   
   void         Activate(Event *ev);
   void         Deactivate(Event *ev);
   qboolean     EarthquakeActive() const { return quakeactive; };
   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void Earthquake::Archive(Archiver &arc)
{
   Trigger::Archive(arc);

   arc.WriteBoolean(quakeactive);
   arc.WriteFloat(duration);
}

inline EXPORT_FROM_DLL void Earthquake::Unarchive(Archiver &arc)
{
   Trigger::Unarchive(arc);

   arc.ReadBoolean(&quakeactive);
   arc.ReadFloat(&duration);
}

#endif /* Earthquake.h */

// EOF

