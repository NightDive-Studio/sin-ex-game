//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/bouncingbetty.h                  $
// $Revision:: 10                                                             $
//   $Author:: Aldie                                                          $
//     $Date:: 10/24/98 12:52a                                                $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// 

#ifndef __BOUNCINGBETTY_H__
#define __BOUNCINGBETTY_H__

#include "g_local.h"
#include "entity.h"

#define BOUNCINGBETTY_RANGE 192

class EXPORT_FROM_DLL BettyLauncher : public Entity
{
protected:
   qboolean firing;
   int      activator;

public:
   CLASS_PROTOTYPE(BettyLauncher);

   BettyLauncher();
   qboolean     inRange(Entity *ent);
   void         CheckVicinity(Event *ev);
   void         Launch(Event *ev);
   void         AttackFinished(Event *ev);
   void         ReleaseBetty(Event *ev);
   void         Killed(Event *ev);
   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void BettyLauncher::Archive(Archiver &arc)
{
   Entity::Archive(arc);

   arc.WriteBoolean(firing);
   arc.WriteInteger(activator);
}

inline EXPORT_FROM_DLL void BettyLauncher::Unarchive(Archiver &arc)
{
   Entity::Unarchive(arc);

   arc.ReadBoolean(&firing);
   arc.ReadInteger(&activator);
}

class EXPORT_FROM_DLL BouncingBetty : public Entity
{
protected:
   int activator = 0;

public:
   CLASS_PROTOTYPE(BouncingBetty);

   BouncingBetty();
   void         Launch(Vector pos, int activatorEnt);
   void         Detonate(Event *ev);
   void         Explode(Event *ev);
   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void BouncingBetty::Archive(Archiver &arc)
{
   Entity::Archive(arc);

   arc.WriteInteger(activator);
}

inline EXPORT_FROM_DLL void BouncingBetty::Unarchive(Archiver &arc)
{
   Entity::Unarchive(arc);

   arc.ReadInteger(&activator);
}

class EXPORT_FROM_DLL BettySpike : public Entity
{
protected:
   int activator;

public:
   CLASS_PROTOTYPE(BettySpike);

   void         SpikeTouch(Event *ev);
   void         Setup(Vector pos, Vector dir);
   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void BettySpike::Archive(Archiver &arc)
{
   Entity::Archive(arc);

   arc.WriteInteger(activator);
}

inline EXPORT_FROM_DLL void BettySpike::Unarchive(Archiver &arc)
{
   Entity::Unarchive(arc);

   arc.ReadInteger(&activator);
}

#endif /* bouncingbetty.h */

// EOF

