//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/viewthing.h                     $
// $Revision:: 20                                                             $
//   $Author:: Jimdose                                                        $
//     $Date:: 10/08/98 12:35a                                                $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Actor code for the viewthing. 
//

#ifndef __VIEWTHING_H__
#define __VIEWTHING_H__

#include "entity.h"

class EXPORT_FROM_DLL ViewMaster : public Listener
{
public:
   CLASS_PROTOTYPE(ViewMaster)

   int current_viewthing = 0;

   void Next(Event *ev);
   void Prev(Event *ev);
   void DeleteAll(Event *ev);
   void Spawn(Event *ev);
   void SetModelEvent(Event *ev);
   void PassEvent(Event *ev);

   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void ViewMaster::Archive(Archiver &arc)
{
   Listener::Archive(arc);

   arc.WriteInteger(current_viewthing);
}

inline EXPORT_FROM_DLL void ViewMaster::Unarchive(Archiver &arc)
{
   Listener::Unarchive(arc);

   arc.ReadInteger(&current_viewthing);
}

extern ViewMaster Viewmodel;

class EXPORT_FROM_DLL Viewthing : public Entity
{
public:
   CLASS_PROTOTYPE(Viewthing)

   int      animstate;
   Vector   baseorigin;

   Viewthing();
   void              Think(Event *ev);
   void              LastFrameEvent(Event *ev);
   void              ToggleAnimateEvent(Event *ev);
   void              SetModelEvent(Event *ev);
   void              NextFrameEvent(Event *ev);
   void              PrevFrameEvent(Event *ev);
   void              NextAnimEvent(Event *ev);
   void              PrevAnimEvent(Event *ev);
   void              ScaleUpEvent(Event *ev);
   void              ScaleDownEvent(Event *ev);
   void              SetScaleEvent(Event *ev);
   void              SetYawEvent(Event *ev);
   void              SetPitchEvent(Event *ev);
   void              SetRollEvent(Event *ev);
   void              SetAnglesEvent(Event *ev);
   void              AttachModel(Event *ev);
   void              Delete(Event *ev);
   void              DetachAll(Event *ev);
   void              BoneGroup(Event *ev);
   void              BoneNum(Event *ev);
   void              ChangeOrigin(Event *ev);
   void              ChangeBoneAngles(Event *ev);
   void              NextSkinEvent(Event *ev);
   void              PrevSkinEvent(Event *ev);
   void              AutoAnimateEvent(Event *ev);

   virtual void      Archive(Archiver &arc)   override;
   virtual void      Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void Viewthing::Archive(Archiver &arc)
{
   Entity::Archive(arc);

   arc.WriteInteger(animstate);
   arc.WriteVector(baseorigin);
}

inline EXPORT_FROM_DLL void Viewthing::Unarchive(Archiver &arc)
{
   Entity::Unarchive(arc);

   arc.ReadInteger(&animstate);
   arc.ReadVector(&baseorigin);
}

#endif /* viewthing.h */

// EOF

