/*
================================================================
MOVEMENT CAPTURING ENTITY
================================================================

Copyright (C) 2020 by Night Dive Studios, Inc.
All rights reserved.

See the license.txt file for conditions and terms of use for this code.
*/

#ifndef __MOVECAPTURE_H__
#define __MOVECAPTURE_H__

#include "g_local.h"
#include "trigger.h"

class EXPORT_FROM_DLL MoveCapture :public TriggerUse
{
private:
   str   outputvar;
   int   lastvalue;   // used to minimize the number of times the output is updated
   float usedebounce; // keep player from popping right back out
public:
   CLASS_PROTOTYPE(MoveCapture);
   MoveCapture();

   EntityPtr usingplayer; // pointer to current user

   virtual void Use(Event *ev);
   virtual void Activate(Event *ev);
   virtual void CaptureMovement(usercmd_t *ucmd);
   virtual void Deactivate(void);
   virtual void SetMCVariable(int newvalue);

   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void MoveCapture::Archive(Archiver &arc)
{
   TriggerUse::Archive(arc);

   arc.WriteString(outputvar);
   arc.WriteInteger(lastvalue);
   arc.WriteFloat(usedebounce);
   arc.WriteSafePointer(usingplayer);
}

inline EXPORT_FROM_DLL void MoveCapture::Unarchive(Archiver &arc)
{
   TriggerUse::Unarchive(arc);

   arc.ReadString(&outputvar);
   arc.ReadInteger(&lastvalue);
   arc.ReadFloat(&usedebounce);
   arc.ReadSafePointer(&usingplayer);
}

template class EXPORT_FROM_DLL	SafePtr<MoveCapture>;
typedef SafePtr<MoveCapture> MoveCapturePtr;

#endif

// EOF

