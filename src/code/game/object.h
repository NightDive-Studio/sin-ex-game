//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/object.h                         $
// $Revision:: 7                                                              $
//   $Author:: Markd                                                          $
//     $Date:: 9/22/98 12:49p                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
// 
// DESCRIPTION:
// Object class
// 

#ifndef __OBJECT_H__
#define __OBJECT_H__

#include "g_local.h"
#include "entity.h"
#include "specialfx.h"

class EXPORT_FROM_DLL Object : public Entity
{
public:
   CLASS_PROTOTYPE(Object);
   Object();
   void              Killed(Event *ev);
};

extern Event EV_ThrowObject_Pickup;
extern Event EV_ThrowObject_Throw;

class EXPORT_FROM_DLL ThrowObject : public Object
{
private:
   int               owner;
   Vector            pickup_offset;
   str               throw_sound;

public:
   CLASS_PROTOTYPE(ThrowObject);
   ThrowObject() = default;

   void              Touch(Event *ev);
   void              Throw(Event * ev);
   void              Pickup(Event * ev);
   void              PickupOffset(Event * ev);
   void              ThrowSound(Event * ev);
   virtual void      Archive(Archiver &arc)   override;
   virtual void      Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void ThrowObject::Archive(Archiver &arc)
{
   Object::Archive(arc);

   arc.WriteInteger(owner);
   arc.WriteVector(pickup_offset);
   arc.WriteString(throw_sound);
}

inline EXPORT_FROM_DLL void ThrowObject::Unarchive(Archiver &arc)
{
   Object::Unarchive(arc);

   arc.ReadInteger(&owner);
   arc.ReadVector(&pickup_offset);
   arc.ReadString(&throw_sound);
}


class EXPORT_FROM_DLL FireBarrel : public Object
{
private:
   FireSprite        *fire;
public:
   CLASS_PROTOTYPE(FireBarrel);
   FireBarrel();
   ~FireBarrel();
   virtual void      Archive(Archiver &arc)   override;
   virtual void      Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void FireBarrel::Archive(Archiver &arc)
{
   Object::Archive(arc);

   arc.WriteObjectPointer(fire);
}

inline EXPORT_FROM_DLL void FireBarrel::Unarchive(Archiver &arc)
{
   Object::Unarchive(arc);

   arc.ReadObjectPointer((Class **)&fire);
}

#endif /* object.h */

// EOF

