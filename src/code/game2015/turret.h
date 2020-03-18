//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/turret.h                         $
// $Revision:: 14                                                             $
//   $Author:: Markd                                                          $
//     $Date:: 10/15/98 3:37p                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Script controlled turret.
// 

#ifndef __TURRET_H__
#define __TURRET_H__

#include "sentient.h"

#define TURRET_OUTOFRANGE     0
#define TURRET_WAKEUPRANGE    1
#define TURRET_FIRERANGE      2

class EXPORT_FROM_DLL Turret : public Sentient
{
protected:
   int            base           = 0;
   int            enemy          = 0;
   float          lastSightTime  = 0;
   qboolean       active         = false;        
   qboolean       attacking      = false;
   qboolean       turning        = false;
   float          neworientation;
   float          turntime       = 0.0f;
   float          lagtime;
   float          firetime       = 0.0f;
   float          patience;
   int            wakeupdistance;
   int            firingdistance;
   qboolean       activated;
   str            thread;
   str            sight_target;

public:
   CLASS_PROTOTYPE(Turret);

   Turret();

   virtual qboolean        CanSee(Entity *ent);
   virtual int             Range(float dist);
   virtual float           Distance(Entity *targ);
   virtual void            Pain(Event *ev);
   virtual void            Killed(Event *ev);
   virtual qboolean        FindTarget();
   virtual float           AdjustAngle(float maxadjust, float currangle, float targetangle);
   virtual void            Seek(Event *ev);
   virtual void            Fire(Event *ev);
   virtual void            Turn(Event *ev);
   virtual void            EndSeek(Event *ev);
   virtual void            CheckVicinity(Event *ev);
   virtual void            GoUp(Event *ev);
   virtual void            GoDown(Event *ev);
   virtual void            Down(Event *ev);
   virtual void            AttackStart(Event *ev);
   virtual void            AttackFinished(Event *ev);
   virtual void            Activate(Event *ev);
   virtual void            Deactivate(Event *ev);
   virtual void            Lagtime(Event *ev);
   virtual void            SetSightTime(Event *ev);
   virtual void            Archive(Archiver &arc)   override;
   virtual void            Unarchive(Archiver &arc) override;
};

extern Event EV_Turret_GoUp;
extern Event EV_Turret_GoDown;
extern Event EV_Turret_Turn;
extern Event EV_Turret_Seek;
extern Event EV_Turret_Attack;

inline EXPORT_FROM_DLL void Turret::Archive(Archiver &arc)
{
   Sentient::Archive(arc);
   arc.WriteInteger(base);
   arc.WriteInteger(enemy);
   arc.WriteFloat(lastSightTime);
   arc.WriteBoolean(active);
   arc.WriteBoolean(attacking);
   arc.WriteBoolean(turning);
   arc.WriteFloat(neworientation);
   arc.WriteFloat(turntime);
   arc.WriteFloat(lagtime);
   arc.WriteFloat(firetime);
   arc.WriteFloat(patience);
   arc.WriteInteger(wakeupdistance);
   arc.WriteInteger(firingdistance);
   arc.WriteBoolean(activated);
   arc.WriteString(thread);
   arc.WriteString(sight_target);
}

inline EXPORT_FROM_DLL void Turret::Unarchive(Archiver &arc)
{
   Sentient::Unarchive(arc);
   arc.ReadInteger(&base);
   arc.ReadInteger(&enemy);
   arc.ReadFloat(&lastSightTime);
   arc.ReadBoolean(&active);
   arc.ReadBoolean(&attacking);
   arc.ReadBoolean(&turning);
   arc.ReadFloat(&neworientation);
   arc.ReadFloat(&turntime);
   arc.ReadFloat(&lagtime);
   arc.ReadFloat(&firetime);
   arc.ReadFloat(&patience);
   arc.ReadInteger(&wakeupdistance);
   arc.ReadInteger(&firingdistance);
   arc.ReadBoolean(&activated);
   arc.ReadString(&thread);
   arc.ReadString(&sight_target);
}

#endif /* turret.h */

// EOF

