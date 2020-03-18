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
// DESCRIPTION:
// Explodable box that falls when boxes below it are destroyed.
// 

#ifndef __BOX_H__
#define __BOX_H__

#include "g_local.h"
#include "entity.h"

//###
extern Event EV_Box_StartThread;
extern Event EV_Box_SetThread;
//###

class EXPORT_FROM_DLL Box : public Entity
{
private:
   float          movetime = 0.0f;
   str            items;
   qboolean       setangles;
   str            thread;    //###

public:
   CLASS_PROTOTYPE(Box);

   Box();
   void           StartFalling(void);
   void           Falling(Event *ev);
   void           TellNeighborsToFall(void);
   virtual void   Killed(Event *ev);
   //###
   void           StartThread(Event *ev);
   void           EventSetThread(Event *ev);
   //###
   virtual void   Archive(Archiver &arc)   override;
   virtual void   Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void Box::Archive(Archiver &arc)
{
   Entity::Archive(arc);

   arc.WriteFloat(movetime);
   arc.WriteString(items);
   arc.WriteBoolean(setangles);
   arc.WriteString(thread); //###
}

inline EXPORT_FROM_DLL void Box::Unarchive(Archiver &arc)
{
   Entity::Unarchive(arc);

   arc.ReadFloat(&movetime);
   arc.ReadString(&items);
   arc.ReadBoolean(&setangles);
   arc.ReadString(&thread);
}

#endif /* box.h */

// EOF

