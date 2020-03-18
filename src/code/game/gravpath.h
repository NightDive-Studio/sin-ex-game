//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/gravpath.h                       $
// $Revision:: 11                                                             $
//   $Author:: Jimdose                                                        $
//     $Date:: 10/25/98 11:53p                                                $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Gravity path - Used for underwater currents and wells.

#ifndef __GRAVPATH_H__
#define __GRAVPATH_H__

#include "g_local.h"
#include "class.h"
#include "container.h"

class EXPORT_FROM_DLL GravPathNode : public Entity
{
private:
   float    speed;
   float    radius;
   qboolean headnode;

public:
   qboolean       active;

   CLASS_PROTOTYPE(GravPathNode);
   GravPathNode();
   void           CreatePath(Event *ev);
   void           Activate(Event *ev);
   void           Deactivate(Event *ev);
   float          Speed() const;
   float          Radius() const { return radius; };
   virtual void   Archive(Archiver &arc)   override;
   virtual void   Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void GravPathNode::Archive(Archiver &arc)
{
   Entity::Archive(arc);

   arc.WriteFloat(speed);
   arc.WriteFloat(radius);
   arc.WriteBoolean(headnode);
   arc.WriteBoolean(active);
}

inline EXPORT_FROM_DLL void GravPathNode::Unarchive(Archiver &arc)
{
   Entity::Unarchive(arc);

   arc.ReadFloat(&speed);
   arc.ReadFloat(&radius);
   arc.ReadBoolean(&headnode);
   arc.ReadBoolean(&active);
}

typedef SafePtr<GravPathNode> GravPathNodePtr;

//
// Exported templated classes must be explicitly instantiated
//
#ifdef EXPORT_TEMPLATE
template class EXPORT_FROM_DLL SafePtr<GravPathNode>;
template class EXPORT_FROM_DLL Container<GravPathNodePtr>;
#endif

class EXPORT_FROM_DLL GravPath : public Listener
{
private:
   Container<GravPathNodePtr> pathlist;
   float                      pathlength = 0;

   GravPathNodePtr            from       = nullptr;
   GravPathNodePtr            to         = nullptr;
   int                        nextnode   = 1;

public:
   CLASS_PROTOTYPE(GravPath);

   GravPath();
   ~GravPath();
   void               Clear();
   void               Reset();
   void               AddNode(GravPathNode *node);
   GravPathNode      *GetNode(int num);
   GravPathNode      *NextNode(void);
   Vector             ClosestPointOnPath(Vector pos, Entity &ent, float *bestdist, float *speed, float *radius);
   float              DistanceAlongPath(Vector pos, float *speed);
   Vector             PointAtDistance(float dist);
   void               DrawPath(Event *ev);
   int                NumNodes() const;
   float              Length() const;
   GravPathNode      *Start();
   GravPathNode      *End();
   virtual void       Archive(Archiver &arc)   override;
   virtual void       Unarchive(Archiver &arc) override;

   Vector                     mins;
   Vector                     maxs;
   Vector                     origin;
   qboolean                   force;
};

inline EXPORT_FROM_DLL void GravPath::Archive(Archiver &arc)
{
   int i, num;

   Listener::Archive(arc);

   num = pathlist.NumObjects();
   arc.WriteInteger(num);
   for(i = 1; i <= num; i++)
   {
      arc.WriteSafePointer(pathlist.ObjectAt(i));
   }

   arc.WriteFloat(pathlength);
   arc.WriteSafePointer(from);
   arc.WriteSafePointer(to);
   arc.WriteInteger(nextnode);
   arc.WriteVector(mins);
   arc.WriteVector(maxs);
   arc.WriteVector(origin);
   arc.WriteBoolean(force);
}

inline EXPORT_FROM_DLL void GravPath::Unarchive(Archiver &arc)
{
   int i, num;

   Reset();

   Listener::Unarchive(arc);

   arc.ReadInteger(&num);
   pathlist.Resize(num);
   for(i = 1; i <= num; i++)
   {
      arc.ReadSafePointer(pathlist.AddressOfObjectAt(i));
   }

   arc.ReadFloat(&pathlength);
   arc.ReadSafePointer(&from);
   arc.ReadSafePointer(&to);
   arc.ReadInteger(&nextnode);
   arc.ReadVector(&mins);
   arc.ReadVector(&maxs);
   arc.ReadVector(&origin);
   arc.ReadBoolean(&force);
}

#ifdef EXPORT_TEMPLATE
template class EXPORT_FROM_DLL Container<GravPath *>;
#endif

class EXPORT_FROM_DLL GravPathManager : public Class
{
private:
   Container<GravPath *> pathList;

public:
   CLASS_PROTOTYPE(GravPathManager);
   ~GravPathManager();
   void     Reset();
   void     AddPath(GravPath *p);
   void     RemovePath(GravPath *p);
   Vector   CalculateGravityPull(Entity &ent, Vector position, qboolean *force);
   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void GravPathManager::Archive(Archiver &arc)
{
   int i, num;

   Class::Archive(arc);

   num = pathList.NumObjects();
   arc.WriteInteger(num);
   for(i = 1; i <= num; i++)
   {
      arc.WriteObject(pathList.ObjectAt(i));
   }
}

inline EXPORT_FROM_DLL void GravPathManager::Unarchive(Archiver &arc)
{
   int i, num;

   Reset();

   Class::Unarchive(arc);

   arc.ReadInteger(&num);
   for(i = 1; i <= num; i++)
   {
      GravPath * ptr;

      ptr = new GravPath();
      arc.ReadObject(ptr);
      pathList.AddObject(ptr);
   }
}

extern GravPathManager gravPathManager;

extern Event EV_DrawGravPath;

#endif /* gravpath.h */

// EOF

