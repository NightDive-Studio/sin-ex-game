//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/doors.h                          $
// $Revision:: 26                                                             $
//   $Author:: Jimdose                                                        $
//     $Date:: 11/08/98 10:50p                                                $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Doors are environment objects that slide open when activated by triggers
// or when used by the player.
// 

#ifndef __DOORS_H__
#define __DOORS_H__

#include "g_local.h"
#include "entity.h"
#include "trigger.h"
#include "scriptslave.h"

extern Event EV_Door_TryOpen;
extern Event EV_Door_GoDown;
extern Event EV_Door_GoUp;
extern Event EV_Door_HitBottom;
extern Event EV_Door_HitTop;
extern Event EV_Door_Fire;
extern Event EV_Door_Link;
extern Event EV_Door_SetSpeed;
extern Event EV_Door_Lock;
extern Event EV_Door_Unlock;

class Door;
#ifdef EXPORT_TEMPLATE
template class EXPORT_FROM_DLL SafePtr<Door>;
#endif
typedef SafePtr<Door> DoorPtr;

class EXPORT_FROM_DLL Door : public ScriptSlave
{
protected:
   str         sound_stop;
   str         sound_move;
   str         sound_message;
   str         sound_locked;
   float       lastblocktime = 0.0f;
   float       angle;
   Vector      dir;
   Vector      doormin;
   Vector      doormax;
   float       diropened = 0.0f;
   int         state;
   int         previous_state;
   int         trigger  = 0;
   int         nextdoor = 0;
   DoorPtr     master;

   void        OpenEnd(Event *ev);
   void        CloseEnd(Event *ev);
   void        Close(Event *ev);
   void        Open(Event *ev);
   void        DoorUse(Event *ev);
   void        DoorFire(Event *ev);
   void        DoorBlocked(Event *ev);
   void        FieldTouched(Event *ev);
   void        TryOpen(Event *ev);
   void        SpawnTriggerField(Vector fmins, Vector fmaxs);
   qboolean    DoorTouches(Door *e1);
   void        LinkDoors(Event *ev);
   void        SetTime(Event *ev);
   void        LockDoor(Event *ev);
   void        UnlockDoor(Event *ev);

public:
   CLASS_PROTOTYPE(Door);

   qboolean    locked = false;

   Door();
   qboolean     isOpen() const;
   qboolean     CanBeOpenedBy(Entity *ent) const;
   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void Door::Archive(Archiver &arc)
{
   ScriptSlave::Archive(arc);

   arc.WriteString(sound_stop);
   arc.WriteString(sound_move);
   arc.WriteString(sound_message);
   arc.WriteString(sound_locked);
   arc.WriteFloat(lastblocktime);
   arc.WriteFloat(angle);
   arc.WriteVector(dir);
   arc.WriteVector(doormin);
   arc.WriteVector(doormax);
   arc.WriteFloat(diropened);
   arc.WriteInteger(state);
   arc.WriteInteger(previous_state);
   arc.WriteInteger(trigger);
   arc.WriteInteger(nextdoor);
   arc.WriteSafePointer(master);
   arc.WriteBoolean(locked);
}

inline EXPORT_FROM_DLL void Door::Unarchive(Archiver &arc)
{
   ScriptSlave::Unarchive(arc);

   arc.ReadString(&sound_stop);
   arc.ReadString(&sound_move);
   arc.ReadString(&sound_message);
   arc.ReadString(&sound_locked);
   arc.ReadFloat(&lastblocktime);
   arc.ReadFloat(&angle);
   arc.ReadVector(&dir);
   arc.ReadVector(&doormin);
   arc.ReadVector(&doormax);
   arc.ReadFloat(&diropened);
   arc.ReadInteger(&state);
   arc.ReadInteger(&previous_state);
   arc.ReadInteger(&trigger);
   arc.ReadInteger(&nextdoor);
   arc.ReadSafePointer(&master);
   arc.ReadBoolean(&locked);
}


class EXPORT_FROM_DLL SlidingDoor : public Door
{
protected:
   float    totalmove;
   float    lip;
   Vector   pos1;
   Vector   pos2;

public:
   CLASS_PROTOTYPE(SlidingDoor);

   void     DoOpen(Event *ev);
   void     DoClose(Event *ev);
   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;

   SlidingDoor();
};

inline EXPORT_FROM_DLL void SlidingDoor::Archive(Archiver &arc)
{
   Door::Archive(arc);

   arc.WriteFloat(totalmove);
   arc.WriteFloat(lip);
   arc.WriteVector(pos1);
   arc.WriteVector(pos2);
}

inline EXPORT_FROM_DLL void SlidingDoor::Unarchive(Archiver &arc)
{
   Door::Unarchive(arc);

   arc.ReadFloat(&totalmove);
   arc.ReadFloat(&lip);
   arc.ReadVector(&pos1);
   arc.ReadVector(&pos2);
}

class EXPORT_FROM_DLL RotatingDoor : public Door
{
protected:
   float    angle;
   Vector   startangle;
   int      init_door_direction;

public:
   CLASS_PROTOTYPE(RotatingDoor);

   void     DoOpen(Event *ev);
   void     DoClose(Event *ev);
   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;

   RotatingDoor();
};

inline EXPORT_FROM_DLL void RotatingDoor::Archive(Archiver &arc)
{
   Door::Archive(arc);

   arc.WriteFloat(angle);
   arc.WriteVector(startangle);
   arc.WriteInteger(init_door_direction);
}

inline EXPORT_FROM_DLL void RotatingDoor::Unarchive(Archiver &arc)
{
   Door::Unarchive(arc);

   arc.ReadFloat(&angle);
   arc.ReadVector(&startangle);
   arc.ReadInteger(&init_door_direction);
}

class EXPORT_FROM_DLL ScriptDoor : public Door
{
protected:
   ThreadPtr doorthread;
   str       initthreadname;
   str       openthreadname;
   str       closethreadname;
   float     doorsize;
   Vector    startangle;
   Vector    startorigin;
   Vector    movedir;

public:
   CLASS_PROTOTYPE(ScriptDoor);
   ScriptDoor();
   
   void     DoInit(Event *ev);
   void     DoOpen(Event *ev);
   void     DoClose(Event *ev);
   void     SetOpenThread(Event *ev);
   void     SetCloseThread(Event *ev);
   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void ScriptDoor::Archive(Archiver &arc)
{
   Door::Archive(arc);

   arc.WriteSafePointer(doorthread);
   arc.WriteString(initthreadname);
   arc.WriteString(openthreadname);
   arc.WriteString(closethreadname);
   arc.WriteFloat(doorsize);
   arc.WriteVector(startangle);
   arc.WriteVector(startorigin);
   arc.WriteVector(movedir);
}

inline EXPORT_FROM_DLL void ScriptDoor::Unarchive(Archiver &arc)
{
   Door::Unarchive(arc);

   arc.ReadSafePointer(&doorthread);
   arc.ReadString(&initthreadname);
   arc.ReadString(&openthreadname);
   arc.ReadString(&closethreadname);
   arc.ReadFloat(&doorsize);
   arc.ReadVector(&startangle);
   arc.ReadVector(&startorigin);
   arc.ReadVector(&movedir);
}

#endif /* doors.h */

// EOF

