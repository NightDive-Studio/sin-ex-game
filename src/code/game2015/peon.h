//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/peon.h                           $
// $Revision:: 3                                                              $
//   $Author:: Jimdose                                                        $
//     $Date:: 11/08/98 10:52p                                                $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
// 
// DESCRIPTION:
// Eon and Peon
// 

#ifndef __PEON_H__
#define __PEON_H__

#include "g_local.h"
#include "actor.h"

class EXPORT_FROM_DLL Peon : public Actor
{
private:
   float             gootime = 0.0f;

public:
   CLASS_PROTOTYPE(Peon);

   Peon();
   virtual void      Prethink()               override;
   virtual void      SpawnGoo(Event *ev);
   virtual void      Archive(Archiver &arc)   override;
   virtual void      Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void Peon::Archive(Archiver &arc)
{
   Actor::Archive(arc);

   arc.WriteFloat(gootime);
}

inline EXPORT_FROM_DLL void Peon::Unarchive(Archiver &arc)
{
   Actor::Unarchive(arc);

   arc.ReadFloat(&gootime);
}

#endif /* peon.h */

// EOF

