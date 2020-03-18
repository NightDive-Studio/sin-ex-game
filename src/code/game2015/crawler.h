/*
================================================================
CRAWLER MONSTER
================================================================

Copyright (C) 2020 by Night Dive Studios, Inc.
All rights reserved.

See the license.txt file for conditions and terms of use for this code.
*/

#ifndef __CRAWLER_H__
#define __CRAWLER_H__

#include "g_local.h"
#include "actor.h"
#include "specialfx.h"
#include "weapon.h"
#include "ceilingsteering.h"

//===============================================================
// crawler weapon

class EXPORT_FROM_DLL CrawlerGoo : public Projectile
{
public:
   CLASS_PROTOTYPE(CrawlerGoo);

   virtual void Setup(Entity *owner, Vector pos, Vector vel) override;
   void FadeOut(Event *ev);
   void GooTouch(Event *ev);
};

class EXPORT_FROM_DLL CrawlerWeapon : public Weapon
{
public:
   CLASS_PROTOTYPE(CrawlerWeapon);

   CrawlerWeapon();
   virtual void Shoot(Event *ev);
};

//===============================================================
// da crawler

class EXPORT_FROM_DLL Crawler : public Actor
{
public:
   qboolean onceiling     = false;
   qboolean likesceiling  = true;  // true if he'd rather be on the ceiling
   int      ceilingheight;

   void JumpToCeilingEvent(Event *ev);
   void OrientToCeilingEvent(Event *ev);

   void LikesCeilingEvent(Event *ev);
   void LikesFloorEvent(Event *ev);
   void CeilingHeightEvent(Event *ev);

   void IfOnCeilingEvent(Event *ev);
   void IfOnFloorEvent(Event *ev);
   void IfCeilingOKEvent(Event *ev);

   CLASS_PROTOTYPE(Crawler);

   Crawler();
   void             IdleEvent(Event *ev);
   void             Pain(Event *ev);
   void             Dead(Event *ev);
   void             Killed(Event *ev);
   qboolean         CanMoveTo(Vector pos);
   virtual qboolean CanShootFrom(Vector pos, Entity *ent, qboolean usecurrentangles) override;

   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void Crawler::Archive(Archiver &arc)
{
   Actor::Archive(arc);

   arc.WriteBoolean(onceiling);
   arc.WriteBoolean(likesceiling);
   arc.WriteInteger(ceilingheight);
}

inline EXPORT_FROM_DLL void Crawler::Unarchive(Archiver &arc)
{
   Actor::Unarchive(arc);

    arc.ReadBoolean(&onceiling);
    arc.ReadBoolean(&likesceiling);
    arc.ReadInteger(&ceilingheight);
}

//===============================================================
// crawler behaiviors

class EXPORT_FROM_DLL JumpToCeiling : public Behavior
{
private:
   float jumpchance = 1.0f;

public:
   CLASS_PROTOTYPE(JumpToCeiling);

   void             SetArgs(Event *ev);
   virtual void     ShowInfo(Actor &self)    override;
   virtual void     Begin(Actor &self)       override;
   virtual qboolean Evaluate(Actor &self)    override;
   virtual void     End(Actor &self)         override;
   virtual void     Archive(Archiver &arc)   override;
   virtual void     Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void JumpToCeiling::Archive(Archiver &arc)
{
   Behavior::Archive(arc);

   arc.WriteFloat(jumpchance);
}

inline EXPORT_FROM_DLL void JumpToCeiling::Unarchive(Archiver &arc)
{
   Behavior::Unarchive(arc);

   jumpchance = arc.ReadFloat();
}

class EXPORT_FROM_DLL CrawlerFindEnemy : public Behavior
{
private:
   str          anim;
   str          ceilinganim;
   Chase        chase;
   CeilingChase ceilingchase;
   int          state;
   float        nextsearch;
   PathNodePtr  lastSearchNode;
   Vector       lastSearchPos;
   qboolean     lastceilingstate;

public:
   CLASS_PROTOTYPE(CrawlerFindEnemy);

   PathNode         *FindClosestSightNode(Actor &self);
   void              SetArgs(Event *ev);
   virtual void      ShowInfo(Actor &self)    override;
   virtual void      Begin(Actor &self)       override;
   virtual qboolean  Evaluate(Actor &self)    override;
   virtual void      End(Actor &self)         override;
   virtual void      Archive(Archiver &arc)   override;
   virtual void      Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void CrawlerFindEnemy::Archive(Archiver &arc)
{
   Behavior::Archive(arc);

   arc.WriteString(anim);
   arc.WriteString(ceilinganim);
   arc.WriteObject(&chase);
   arc.WriteObject(&ceilingchase);
   arc.WriteInteger(state);
   arc.WriteFloat(nextsearch);
   arc.WriteSafePointer(lastSearchNode);
   arc.WriteVector(lastSearchPos);
   arc.WriteBoolean(lastceilingstate);
}
inline EXPORT_FROM_DLL void CrawlerFindEnemy::Unarchive(Archiver &arc)
{
   Behavior::Unarchive(arc);

   arc.ReadString(&anim);
   arc.ReadString(&ceilinganim);
   arc.ReadObject(&chase);
   arc.ReadObject(&ceilingchase);
   arc.ReadInteger(&state);
   arc.ReadFloat(&nextsearch);
   arc.ReadSafePointer(&lastSearchNode);
   arc.ReadVector(&lastSearchPos);
   arc.ReadBoolean(&lastceilingstate);
}
class EXPORT_FROM_DLL CrawlerIdle : public Behavior
{
private:
   float    nexttwitch;
   str      anim;
   str      ceilinganim;
   qboolean lastceilingstate;

public:
   CLASS_PROTOTYPE(CrawlerIdle);

   void             SetArgs(Event *ev);
   virtual void     ShowInfo(Actor &self)    override;
   virtual void     Begin(Actor &self)       override;
   virtual qboolean Evaluate(Actor &self)    override;
   virtual void     End(Actor &self)         override;
   virtual void     Archive(Archiver &arc)   override;
   virtual void     Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void CrawlerIdle::Archive(Archiver &arc)
{
   Behavior::Archive(arc);

   arc.WriteFloat(nexttwitch);
   arc.WriteString(anim);
   arc.WriteString(ceilinganim);
   arc.WriteBoolean(lastceilingstate);
}

inline EXPORT_FROM_DLL void CrawlerIdle::Unarchive(Archiver &arc)
{
   Behavior::Unarchive(arc);

   arc.ReadFloat(&nexttwitch);
   arc.ReadString(&anim);
   arc.ReadString(&ceilinganim);
   arc.ReadBoolean(&lastceilingstate);
}

class EXPORT_FROM_DLL CrawlerStrafeTo : public Behavior
{
private:
   Vector   goal;
   Seek     seek;
   int      fail;
   qboolean lastceilingstate;

public:
   CLASS_PROTOTYPE(CrawlerStrafeTo);

   virtual void     ShowInfo(Actor &self)    override;
   void             SetArgs(Event *ev);
   virtual void     Begin(Actor &self)       override;
   virtual qboolean Evaluate(Actor &self)    override;
   virtual void     End(Actor &self)         override;
   virtual void     Archive(Archiver &arc)   override;
   virtual void     Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void CrawlerStrafeTo::Archive(Archiver &arc)
{
   Behavior::Archive(arc);

   arc.WriteVector(goal);
   arc.WriteObject(&seek);
   arc.WriteInteger(fail);
   arc.WriteBoolean(lastceilingstate);
}

inline EXPORT_FROM_DLL void CrawlerStrafeTo::Unarchive(Archiver &arc)
{
   Behavior::Unarchive(arc);

   arc.ReadVector(&goal);
   arc.ReadObject(&seek);
   arc.ReadInteger(&fail);
   arc.ReadBoolean(&lastceilingstate);
}
class EXPORT_FROM_DLL CeilingStrafeAttack : public Behavior
{
private:
   int    state;
   TurnTo turn;

public:
   CLASS_PROTOTYPE(CeilingStrafeAttack);

   virtual void     ShowInfo(Actor &self)    override;
   virtual void     Begin(Actor &self)       override;
   virtual qboolean Evaluate(Actor &self)    override;
   virtual void     End(Actor &self)         override;
   virtual void     Archive(Archiver &arc)   override;
   virtual void     Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void CeilingStrafeAttack::Archive(Archiver &arc)
{
   Behavior::Archive(arc);

   arc.WriteInteger(state);
   arc.WriteObject(&turn);
}

inline EXPORT_FROM_DLL void CeilingStrafeAttack::Unarchive(Archiver &arc)
{
   Behavior::Unarchive(arc);

   arc.ReadInteger(&state);
   arc.ReadObject(&turn);
}

#endif /* crawler.h */

// EOF