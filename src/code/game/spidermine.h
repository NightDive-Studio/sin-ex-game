//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/spidermine.h                        $
// $Revision:: 15                                                             $
//   $Author:: Aldie                                                          $
//     $Date:: 11/11/98 5:46p                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Spider Mines
// 

#ifndef __SPIDERMINE_H__
#define __SPIDERMINE_H__

#include "g_local.h"
#include "item.h"
#include "weapon.h"
#include "specialfx.h"

class Detonator;

typedef SafePtr<Detonator> DetonatorPtr;
#ifdef EXPORT_TEMPLATE
template class EXPORT_FROM_DLL SafePtr<Detonator>;
#endif

class EXPORT_FROM_DLL Mine : public Projectile
{
private:
   DetonatorPtr      detonator;
   qboolean          detonate;
   qboolean          sticky;

public:
   CLASS_PROTOTYPE(Mine);

   void              Explode(Event *ev);
   virtual void      Setup(Entity *owner, Vector pos, Vector dir) override;
   void              SlideOrStick(Event *ev);
   void              CheckForTargets(Event *ev);
   void              Run(Event *ev);
   void              SetDetonator(Detonator *det);
   qboolean          IsOwner(Sentient *sent);
   virtual void      Archive(Archiver &arc)   override;
   virtual void      Unarchive(Archiver &arc) override;
};

#ifdef EXPORT_TEMPLATE
template class EXPORT_FROM_DLL SafePtr<Mine>;
#endif

typedef SafePtr<Mine> MinePtr;

#ifdef EXPORT_TEMPLATE
template class EXPORT_FROM_DLL Container<MinePtr>;
#endif

class EXPORT_FROM_DLL Detonator : public Weapon
{
private:
public:
   Container<MinePtr>    mineList;
   int                   currentMine;

   CLASS_PROTOTYPE(Detonator);

   Detonator();
   virtual void          Shoot(Event *ev);
   virtual void          DoneFiring(Event *ev);
   void                  AddMine(Mine *mine);
   void                  RemoveMine(Mine *mine);
   void                  CycleCamera(Event *ev);
   virtual qboolean      AutoChange()             override;
   virtual qboolean      IsDroppable()            override;
   virtual void          Archive(Archiver &arc)   override;
   virtual void          Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void Mine::Archive(Archiver &arc)
{
   Projectile::Archive(arc);
   arc.WriteSafePointer(detonator);
   arc.WriteBoolean(detonate);
   arc.WriteBoolean(sticky);
}

inline EXPORT_FROM_DLL void Mine::Unarchive(Archiver &arc)
{
   Projectile::Unarchive(arc);
   arc.ReadSafePointer(&detonator);
   arc.ReadBoolean(&detonate);
   arc.ReadBoolean(&sticky);
}

inline EXPORT_FROM_DLL void Detonator::Archive(Archiver &arc)
{
   int i;
   int num;

   Weapon::Archive(arc);

   arc.WriteInteger(currentMine);

   num = mineList.NumObjects();
   arc.WriteInteger(num);
   for(i = 1; i <= num; i++)
   {
      arc.WriteSafePointer(mineList.ObjectAt(i));
   }
}

inline EXPORT_FROM_DLL void Detonator::Unarchive(Archiver &arc)
{
   int i;
   int num;

   mineList.FreeObjectList();

   Weapon::Unarchive(arc);

   arc.ReadInteger(&currentMine);

   arc.ReadInteger(&num);
   mineList.Resize(num);
   for(i = 1; i <= num; i++)
   {
      arc.ReadSafePointer(mineList.AddressOfObjectAt(i));
   }
}

class EXPORT_FROM_DLL SpiderMine : public Weapon
{
private:
   MinePtr           currentMine;
   DetonatorPtr      detonator;

public:
   CLASS_PROTOTYPE(SpiderMine);

   SpiderMine();

   virtual void      Shoot(Event *ev);
   virtual void      DoneFiring(Event *ev)    override;
   virtual qboolean  ReadyToUse()             override;
   void              ChangeToDetonator(Event *ev);
   void              SetDetonator(Detonator *det);
   virtual void      SetOwner(Sentient *sent) override;
   virtual qboolean  IsDroppable()            override;
   virtual void      Archive(Archiver &arc)   override;
   virtual void      Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void SpiderMine::Archive(Archiver &arc)
{
   Weapon::Archive(arc);
   arc.WriteSafePointer(currentMine);
   arc.WriteSafePointer(detonator);
}

inline EXPORT_FROM_DLL void SpiderMine::Unarchive(Archiver &arc)
{
   Weapon::Unarchive(arc);
   arc.ReadSafePointer(&currentMine);
   arc.ReadSafePointer(&detonator);
}

#endif /* SpiderMine.h */

// EOF

