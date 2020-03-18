//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/scriptslave.h                    $
// $Revision:: 32                                                             $
//   $Author:: Markd                                                          $
//     $Date:: 10/28/98 8:37p                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Standard scripted objects.  Controlled by scriptmaster.  These objects
// are bmodel objects created in the editor and controlled by an external
// text based script.  Commands are interpretted on by one and executed
// upon a signal from the script master.  The base script object can
// perform several different relative and specific rotations and translations
// and can cause other parts of the script to be executed when touched, damaged,
// touched, or used.
// 

#ifndef __SCRIPTSLAVE_H__
#define __SCRIPTSLAVE_H__

#include "g_local.h"
#include "entity.h"
#include "trigger.h"
#include "mover.h"
#include "script.h"
#include "scriptmaster.h"
#include "misc.h"
#include "bspline.h"

class EXPORT_FROM_DLL ScriptSlave : public Mover
{
protected:
   ThreadPtr         touchthread;
   ThreadPtr         blockthread;
   ThreadPtr         triggerthread;
   ThreadPtr         usethread;
   ThreadPtr         damagethread;
   ThreadPtr         movethread;

   str               touchlabel;
   str               uselabel;
   str               preciseuselabel; //### precise use
   str               blocklabel;
   str               triggerlabel;
   str               damagelabel;

   float             attack_finished;
   int               dmg;

public:
   qboolean	         commandswaiting;
   Vector            TotalRotation;
   Vector            NewAngles;
   Vector            NewPos;
   Vector            ForwardDir;
   float             speed;
   Waypoint         *waypoint;
   float             traveltime;
   BSpline          *splinePath;
   float             splineTime;
   qboolean          splineangles;
   qboolean          ignoreangles;

   CLASS_PROTOTYPE(ScriptSlave);

   ScriptSlave();
   ~ScriptSlave();

   void              NewOrders(Event *ev);
   void              BindEvent(Event *ev);
   void              EventUnbind(Event *ev);
   void              DoMove(Event *ev);
   void              MoveEnd(Event *ev);
   void              SetAnglesEvent(Event *ev);
   void              TriggerEvent(Event *ev);
   void              GotoNextWaypoint(Event *ev);
   void              JumpTo(Event *ev);
   void              MoveToEvent(Event *ev);
   void              SetSpeed(Event *ev);
   void              SetTime(Event *ev);
   void              MoveUp(Event *ev);
   void              MoveDown(Event *ev);
   void              MoveNorth(Event *ev);
   void              MoveSouth(Event *ev);
   void              MoveEast(Event *ev);
   void              MoveWest(Event *ev);
   void              MoveForward(Event *ev);
   void              MoveBackward(Event *ev);
   void              MoveLeft(Event *ev);
   void              MoveRight(Event *ev);
   void              RotateXdownto(Event *ev);
   void              RotateYdownto(Event *ev);
   void              RotateZdownto(Event *ev);
   void              RotateAxisdownto(Event *ev);
   void              RotateXupto(Event *ev);
   void              RotateYupto(Event *ev);
   void              RotateZupto(Event *ev);
   void              RotateAxisupto(Event *ev);
   void              Rotateupto(Event *ev);
   void              Rotatedownto(Event *ev);
   void              Rotateto(Event *ev);
   void              RotateXdown(Event *ev);
   void              RotateYdown(Event *ev);
   void              RotateZdown(Event *ev);
   void              RotateAxisdown(Event *ev);
   void              RotateXup(Event *ev);
   void              RotateYup(Event *ev);
   void              RotateZup(Event *ev);
   void              RotateAxisup(Event *ev);
   void              RotateX(Event *ev);
   void              RotateY(Event *ev);
   void              RotateZ(Event *ev);
   void              RotateAxis(Event *ev);
   void              OnTouch(Event *ev);
   void              NoTouch(Event *ev);
   void              TouchFunc(Event *ev);
   void              OnBlock(Event *ev);
   void              NoBlock(Event *ev);
   void              BlockFunc(Event *ev);
   void              OnTrigger(Event *ev);
   void              NoTrigger(Event *ev);
   void              TriggerFunc(Event *ev);
   void              OnUse(Event *ev);
   void              OnPreciseUse(Event *ev); //### precise use
   void              NoUse(Event *ev);
   void              UseFunc(Event *ev);
   void              UsePreciseFunc(Event *ev); //### precise use
   void              OnDamage(Event *ev);
   void              NoDamage(Event *ev);
   void              DamageFunc(Event *ev);
   void              SetDamage(Event *ev);
   void              FollowPath(Event *ev);
   void              EndPath(Event *ev);
   void              FollowingPath(Event *ev);
   void              CreatePath(SplinePath *path, splinetype_t type);
   void              Explode(Event *ev);
   void              NotShootable(Event *ev);
   virtual void      Archive(Archiver &arc)   override;
   virtual void      Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void ScriptSlave::Archive(Archiver &arc)
{
   Mover::Archive(arc);

   arc.WriteSafePointer(touchthread);
   arc.WriteSafePointer(blockthread);
   arc.WriteSafePointer(triggerthread);
   arc.WriteSafePointer(usethread);
   arc.WriteSafePointer(damagethread);
   arc.WriteSafePointer(movethread);

   arc.WriteString(touchlabel);
   arc.WriteString(uselabel);
   arc.WriteString(blocklabel);
   arc.WriteString(triggerlabel);
   arc.WriteString(damagelabel);

   arc.WriteFloat(attack_finished);
   arc.WriteInteger(dmg);

   arc.WriteBoolean(commandswaiting);
   arc.WriteVector(TotalRotation);
   arc.WriteVector(NewAngles);
   arc.WriteVector(NewPos);
   arc.WriteVector(ForwardDir);
   arc.WriteFloat(speed);
   arc.WriteObjectPointer(waypoint);
   arc.WriteFloat(traveltime);
   arc.WriteFloat(splineTime);
   arc.WriteBoolean(splineangles);
   arc.WriteBoolean(ignoreangles);

   // if it exists, archive it, otherwise place a special NULL ptr tag
   if(splinePath)
   {
      arc.WriteInteger(ARCHIVE_POINTER_VALID);
      splinePath->Archive(arc);
   }
   else
   {
      arc.WriteInteger(ARCHIVE_POINTER_NULL);
   }

   arc.WriteString(preciseuselabel); //###
}

inline EXPORT_FROM_DLL void ScriptSlave::Unarchive(Archiver &arc)
{
   int i;

   Mover::Unarchive(arc);

   arc.ReadSafePointer(&touchthread);
   arc.ReadSafePointer(&blockthread);
   arc.ReadSafePointer(&triggerthread);
   arc.ReadSafePointer(&usethread);
   arc.ReadSafePointer(&damagethread);
   arc.ReadSafePointer(&movethread);

   arc.ReadString(&touchlabel);
   arc.ReadString(&uselabel);
   arc.ReadString(&blocklabel);
   arc.ReadString(&triggerlabel);
   arc.ReadString(&damagelabel);

   arc.ReadFloat(&attack_finished);
   arc.ReadInteger(&dmg);

   arc.ReadBoolean(&commandswaiting);
   arc.ReadVector(&TotalRotation);
   arc.ReadVector(&NewAngles);
   arc.ReadVector(&NewPos);
   arc.ReadVector(&ForwardDir);
   arc.ReadFloat(&speed);
   arc.ReadObjectPointer((Class **)&waypoint);
   arc.ReadFloat(&traveltime);
   arc.ReadFloat(&splineTime);
   arc.ReadBoolean(&splineangles);
   arc.ReadBoolean(&ignoreangles);

   i = arc.ReadInteger();

   if(i == ARCHIVE_POINTER_VALID)
   {
      splinePath = new BSpline();
      splinePath->Unarchive(arc);
   }
   else if(i == ARCHIVE_POINTER_NULL)
   {
      splinePath = nullptr;
   }
   else
   {
      warning("Unarchive", "unable to determine archive type for splinePath");
   }

   arc.ReadString(&preciseuselabel); //###
}

class EXPORT_FROM_DLL ScriptModel : public ScriptSlave
{
private:
   void              GibEvent(Event *ev);
public:
   CLASS_PROTOTYPE(ScriptModel);
   ScriptModel();
};

class EXPORT_FROM_DLL ScriptOrigin : public ScriptSlave
{
public:
   CLASS_PROTOTYPE(ScriptOrigin);
   ScriptOrigin();
};

class EXPORT_FROM_DLL ScriptVolumetric : public ScriptSlave
{
public:
   CLASS_PROTOTYPE(ScriptVolumetric);
   ScriptVolumetric();
};

#endif /* scriptslave.h */

// EOF

