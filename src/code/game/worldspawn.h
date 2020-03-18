//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/worldspawn.h                     $
// $Revision:: 21                                                             $
//   $Author:: Jimdose                                                        $
//     $Date:: 10/27/98 5:43a                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Base class for worldspawn objects.  This should be subclassed whenever
// a DLL has new game behaviour that needs to be initialized before any other
// entities are created, or before any entity thinks each frame.  Also controls
// spawning of clients.
//

#ifndef __WORLDSPAWN_H__
#define __WORLDSPAWN_H__

#include "entity.h"

#ifdef EXPORT_TEMPLATE
template class EXPORT_FROM_DLL Container<Entity *>;
#endif

class EXPORT_FROM_DLL TargetList : public Class
{
public:
   CLASS_PROTOTYPE(TargetList);
   Container<Entity *>  list;
   str                  targetname;

   TargetList() = default;
   TargetList(str &tname);
   ~TargetList();
   void      AddEntity(Entity * ent);
   void      RemoveEntity(Entity * ent);
   Entity   *GetNextEntity(Entity * ent);
};

#ifdef EXPORT_TEMPLATE
template class EXPORT_FROM_DLL Container<TargetList *>;
#endif

class EXPORT_FROM_DLL World : public Entity
{
private:
   Container<TargetList *> targetList;
public:
   CLASS_PROTOTYPE(World);

   World();
   ~World();

   str         skipthread;

   void        FreeTargetList();
   TargetList *GetTargetList(str &targetname);
   void        AddTargetEntity(str &targetname, Entity * ent);
   void        RemoveTargetEntity(str &targetname, Entity * ent);
   Entity     *GetNextEntity(str &targetname, Entity * ent);

   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void World::Archive(Archiver &arc)
{
   Entity::Archive(arc);

   arc.WriteString(skipthread);
}

inline EXPORT_FROM_DLL void World::Unarchive(Archiver &arc)
{
   FreeTargetList();

   Entity::Unarchive(arc);

   skipthread = arc.ReadString();
}

#ifdef EXPORT_TEMPLATE
template class EXPORT_FROM_DLL SafePtr<World>;
#endif

typedef SafePtr<World> WorldPtr;
extern WorldPtr world;

#endif /* worldspawn.h */

// EOF

