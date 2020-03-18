/*
================================================================
CONCUSSION GUN
================================================================

Copyright (C) 2020 by Night Dive Studios, Inc.
All rights reserved.

See the license.txt file for conditions and terms of use for this code.
*/

#ifndef __CONCUSSION_H__
#define __CONCUSSION_H__

#include "g_local.h"
#include "item.h"
#include "weapon.h"
#include "misc.h"

// number of times to do concussion effect per fire
#define CONCUSSION_COUNT 3
// effective firing arc
#define CONCUSSION_ARC 0.4
// max distance that concussion gun is effective
#define CONCUSSION_DIST 450
// max amout of push on effect
//#define CONCUSSION_PUSH 800
#define CONCUSSION_PUSH 600
// push falloff rate
//#define CONCUSSION_PUSH_DECAY 1.1
#define CONCUSSION_PUSH_DECAY 0.64
// max amount of damage done on effect
#define CONCUSSION_DAMG 8
// damage falloff rate
#define CONCUSSION_DAMG_DECAY 0.006
// firing kick back amount
#define CONCUSSION_KICKBACK 800

extern Event EV_ConcussionRing_Animate2;
extern Event EV_ConcussionRing_Animate3;

class EXPORT_FROM_DLL ConcussionRing :public Entity
{
public:
   CLASS_PROTOTYPE(ConcussionRing);

   virtual void Animate2(Event *ev);
   virtual void Animate3(Event *ev);
   virtual void Setup(Vector pos, Vector dir);
};

extern Event EV_Concussion_Effect;

class EXPORT_FROM_DLL ConcussionGun : public Weapon
{
private:
   int    blastcount;  // fire counter
   Vector lastfirepos; // where it last fired from
   Vector lastfiredir; // direction it last fired in

public:
   CLASS_PROTOTYPE(ConcussionGun);

   ConcussionGun();
   virtual void     Shoot(Event *ev);
   virtual void     BlastEffect(Event *ev);
   virtual void     BatteryRegen(Event *ev);
   virtual void     Fire()                   override;
   virtual qboolean HasAmmo()                override;
   virtual void     SecondaryUse(Event *ev)  override;

   virtual void     Archive(Archiver &arc)   override;
   virtual void     Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void ConcussionGun::Archive(Archiver &arc)
{
   Weapon::Archive(arc);

   arc.WriteInteger(blastcount);
   arc.WriteVector(lastfirepos);
   arc.WriteVector(lastfiredir);
}

inline EXPORT_FROM_DLL void ConcussionGun::Unarchive(Archiver &arc)
{
   Weapon::Unarchive(arc);

   arc.ReadInteger(&blastcount);
   arc.ReadVector(&lastfirepos);
   arc.ReadVector(&lastfiredir);
}

#endif /* concussion.h */

//EOF

