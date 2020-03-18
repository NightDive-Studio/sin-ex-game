//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/light.h                          $
// $Revision:: 12                                                             $
//   $Author:: Aldie                                                          $
//     $Date:: 10/09/98 9:01p                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Classes for creating and controlling lights.
// 

#ifndef __LIGHT_H__
#define __LIGHT_H__

#include "g_local.h"
#include "entity.h"
#include "trigger.h"

extern Event EV_Light_TurnOn;
extern Event EV_Light_TurnOff;
extern Event EV_Light_SetLightStyle;

class EXPORT_FROM_DLL BaseLight : public Trigger
{
protected:
   int      style;
   str      lightstyle;
   str      on_style;
   str      off_style;

public:
   CLASS_PROTOTYPE(BaseLight);

   BaseLight();
   void              SetLightStyle(const char *stylestring);
   void              EventSetLightStyle(Event *ev);
   int               GetStyle(void);
   void              TurnOn(Event *ev);
   void              TurnOff(Event *ev);
   virtual void      Archive(Archiver &arc)   override;
   virtual void      Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void BaseLight::Archive(Archiver &arc)
{
   Trigger::Archive(arc);

   arc.WriteInteger(style);
   arc.WriteString(lightstyle);
   arc.WriteString(on_style);
   arc.WriteString(off_style);
}

inline EXPORT_FROM_DLL void BaseLight::Unarchive(Archiver &arc)
{
   Trigger::Unarchive(arc);

   arc.ReadInteger(&style);
   arc.ReadString(&lightstyle);
   arc.ReadString(&on_style);
   arc.ReadString(&off_style);
}

class EXPORT_FROM_DLL LightRamp : public BaseLight
{
protected:
   float minlevel;
   float maxlevel;
   float currentlevel;
   float rate;

public:
   CLASS_PROTOTYPE(LightRamp);

   LightRamp();
   void           RampLight(Event *ev);
   void           StartRamp(Event *ev);
   virtual void   Archive(Archiver &arc)   override;
   virtual void   Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void LightRamp::Archive(Archiver &arc)
{
   BaseLight::Archive(arc);

   arc.WriteFloat(minlevel);
   arc.WriteFloat(maxlevel);
   arc.WriteFloat(currentlevel);
   arc.WriteFloat(rate);
}

inline EXPORT_FROM_DLL void LightRamp::Unarchive(Archiver &arc)
{
   BaseLight::Unarchive(arc);

   arc.ReadFloat(&minlevel);
   arc.ReadFloat(&maxlevel);
   arc.ReadFloat(&currentlevel);
   arc.ReadFloat(&rate);
}

class EXPORT_FROM_DLL TriggerLightRamp : public LightRamp
{
public:
   CLASS_PROTOTYPE(TriggerLightRamp);
};

class EXPORT_FROM_DLL Light : public BaseLight
{
public:
   CLASS_PROTOTYPE(Light);
   
   Light();
   void ToggleLight(Event *ev);
};

class EXPORT_FROM_DLL TriggerLightStyle : public Light
{
public:
   CLASS_PROTOTYPE(TriggerLightStyle);
};

#endif /* light.h */

// EOF

