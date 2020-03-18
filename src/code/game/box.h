//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/box.h                            $
// $Revision:: 6                                                              $
//   $Author:: Markd                                                          $
//     $Date:: 9/29/98 5:58p                                                  $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// $Log:: /Quake 2 Engine/Sin/code/game/box.h                                 $
// 
// 6     9/29/98 5:58p Markd
// put in archive and unarchive
// 
// 5     8/09/98 6:11p Aldie
// New box behavior
// 
// 4     4/08/98 4:19p Jimdose
// Converted to Q2
// 
// 2     10/30/97 7:42p Jimdose
// Created file
//
// DESCRIPTION:
// Explodable box that falls when boxes below it are destroyed.
// 

#ifndef __BOX_H__
#define __BOX_H__

#include "g_local.h"
#include "entity.h"

class EXPORT_FROM_DLL Box : public Entity
{
private:
   float    movetime = 0.0f;
   str      items;
   qboolean setangles;

public:
   CLASS_PROTOTYPE(Box);

   Box();
   void           StartFalling(void);
   void           Falling(Event *ev);
   void           TellNeighborsToFall(void);
   virtual void   Killed(Event *ev);
   virtual void   Archive(Archiver &arc)   override;
   virtual void   Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void Box::Archive(Archiver &arc)
{
   Entity::Archive(arc);

   arc.WriteFloat(movetime);
   arc.WriteString(items);
   arc.WriteBoolean(setangles);
}

inline EXPORT_FROM_DLL void Box::Unarchive(Archiver &arc)
{
   Entity::Unarchive(arc);

   arc.ReadFloat(&movetime);
   arc.ReadString(&items);
   arc.ReadBoolean(&setangles);
}

#endif /* box.h */

// EOF

