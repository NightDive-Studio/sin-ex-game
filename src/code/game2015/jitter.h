/*
================================================================
VIEW JITTER STUFF
================================================================

Copyright (C) 2020 by Night Dive Studios, Inc.
All rights reserved.

See the license.txt file for conditions and terms of use for this code.
*/

#ifndef __JITTER_H__
#define __JITTER_H__

#include "g_local.h"
#include "trigger.h"

class EXPORT_FROM_DLL BaseJitter : public Trigger
{
protected:
   qboolean angleactive     = false;
   float    angleduration;
   float    anglemagnitude;
   float    anglefalloff;
   qboolean offsetactive    = false;
   float    offsetduration;
   float    offsetmagnitude;
   float    offsetfalloff;

public:
   CLASS_PROTOTYPE(BaseJitter);

   qboolean AngleActive()  const { return angleactive;  }
   qboolean OffsetActive() const { return offsetactive; }

   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void BaseJitter::Archive(Archiver &arc)
{
   Trigger::Archive(arc);

   arc.WriteBoolean(angleactive);
   arc.WriteFloat(angleduration);
   arc.WriteFloat(anglemagnitude);
   arc.WriteFloat(anglefalloff);
   arc.WriteBoolean(offsetactive);
   arc.WriteFloat(offsetduration);
   arc.WriteFloat(offsetmagnitude);
   arc.WriteFloat(offsetfalloff);
}

inline EXPORT_FROM_DLL void BaseJitter::Unarchive(Archiver &arc)
{
   Trigger::Unarchive(arc);

   arc.ReadBoolean(&angleactive);
   arc.ReadFloat(&angleduration);
   arc.ReadFloat(&anglemagnitude);
   arc.ReadFloat(&anglefalloff);
   arc.ReadBoolean(&offsetactive);
   arc.ReadFloat(&offsetduration);
   arc.ReadFloat(&offsetmagnitude);
   arc.ReadFloat(&offsetfalloff);
}

extern float anglejitter_time;       // timmer for angle jitter
extern float anglejitter_magnitude;  // angle jitter magnitude
extern float anglejitter_falloff;    // falloffrate for angle jitter
extern float offsetjitter_time;      // timmer for offset jitter
extern float offsetjitter_magnitude; // offset jitter magnitude
extern float offsetjitter_falloff;   // falloffrate for offset jitter

class EXPORT_FROM_DLL GlobalJitter : public BaseJitter
{
public:
   CLASS_PROTOTYPE(GlobalJitter);
   GlobalJitter();

   void     Activate(Event *ev);
   void     DeactivateAngle(Event *ev);
   void     DeactivateOffset(Event *ev);
   qboolean AngleActive()  const { return angleactive;  }
   qboolean OffsetActive() const { return offsetactive; }
};

class EXPORT_FROM_DLL RadiusJitter : public BaseJitter
{
protected:
   float jitterradius;
   float radiusfalloff;
   float anglecurrent;
   float angletime;
   float offsetcurrent;
   float offsettime;

public:
   CLASS_PROTOTYPE(RadiusJitter);
   RadiusJitter();

   void     Activate(Event *ev);
   void     ApplyJitter(Event *ev);
   void     DeactivateAngle(Event *ev);
   void     DeactivateOffset(Event *ev);
   qboolean AngleActive()  const { return angleactive;  }
   qboolean OffsetActive() const { return offsetactive; }
   void     Setup(const Vector &org, float rad, float outer,
                  float angdur, float angmag, float angfall,
                  float ofsdur, float ofsmag, float ofsfall);
   void     SetupSmall(const Vector &org); // sets it up as a small explosion
   void     SetupLarge(const Vector &org); // sets it up as a large explosion

   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void RadiusJitter::Archive(Archiver &arc)
{
   BaseJitter::Archive(arc);

   arc.WriteFloat(jitterradius);
   arc.WriteFloat(radiusfalloff);
   arc.WriteFloat(anglecurrent);
   arc.WriteFloat(angletime);
   arc.WriteFloat(offsetcurrent);
   arc.WriteFloat(offsettime);
}

inline EXPORT_FROM_DLL void RadiusJitter::Unarchive(Archiver &arc)
{
   BaseJitter::Unarchive(arc);

   arc.ReadFloat(&jitterradius);
   arc.ReadFloat(&radiusfalloff);
   arc.ReadFloat(&anglecurrent);
   arc.ReadFloat(&angletime);
   arc.ReadFloat(&offsetcurrent);
   arc.ReadFloat(&offsettime);
}

#endif /* jitter.h */

// EOF
