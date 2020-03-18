//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/gibs.h                           $
// $Revision:: 8                                                              $
//   $Author:: Aldie                                                          $
//     $Date:: 11/08/98 8:30p                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Gibs - nuff said

#ifndef __GIBS_H__
#define __GIBS_H__

#include "g_local.h"

class EXPORT_FROM_DLL Gib : public Entity
{
private:
   qboolean sprayed;

public:
   CLASS_PROTOTYPE(Gib);

   qboolean    fadesplat;
   Gib();
   Gib(const char *name, qboolean blood_trail = true);

   void     SetVelocity(float health);
   void     SprayBlood(Vector start, Vector end, int damage);
   void     Throw(Event *ev);
   void     Splat(Event *ev);
   void     ClipGibVelocity();
   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void Gib::Archive(Archiver &arc)
{
   Entity::Archive(arc);

   arc.WriteBoolean(sprayed);
   arc.WriteBoolean(fadesplat);
}

inline EXPORT_FROM_DLL void Gib::Unarchive(Archiver &arc)
{
   Entity::Unarchive(arc);

   arc.ReadBoolean(&sprayed);
   arc.ReadBoolean(&fadesplat);
}

void CreateGibs
(
   Entity * ent,
   float damage = -50,
   float scale = 1.0f,
   int num = 1,
   const char * modelname = nullptr
);

extern Event EV_ThrowGib;

#endif // gibs.h

// EOF

