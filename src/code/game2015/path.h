//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/path.h                           $
// $Revision:: 22                                                             $
//   $Author:: Jimdose                                                        $
//     $Date:: 10/25/98 11:53p                                                $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// 

#pragma once 

#include "g_local.h"
#include "class.h"
#include "container.h"
#include "navigate.h"

//
// Exported templated classes must be explicitly instantiated
//
#ifdef EXPORT_TEMPLATE
template class EXPORT_FROM_DLL Container<PathNodePtr>;
template class EXPORT_FROM_DLL Container<float>;
template class EXPORT_FROM_DLL Container<Vector>;
#endif

class EXPORT_FROM_DLL Path : public Class
{
private:
   Container<PathNodePtr>  pathlist;
   Container<float>        distanceToNextNode;
   Container<Vector>       dirToNextNode;
   float                   pathlength = 0.0f;
   PathNodePtr             from       = nullptr;
   PathNodePtr             to         = nullptr;
   int                     nextnode   = 1;

public:
   CLASS_PROTOTYPE(Path);

   Path() = default;
   Path(int numnodes);
   void         Clear();
   void         Reset();
   void         AddNode(PathNode *node);
   PathNode    *GetNode(int num);
   PathNode    *NextNode();
   PathNode    *NextNode(PathNode *node);
   Vector       ClosestPointOnPath(Vector pos);
   float        DistanceAlongPath(Vector pos);
   Vector       PointAtDistance(float dist);
   PathNode    *NextNode(float dist);
   void         DrawPath(float r, float g, float b, float time);
   int          NumNodes();
   float        Length();
   PathNode    *Start();
   PathNode    *End();
   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void Path::Archive(Archiver &arc)
{
   PathNodePtr ptr;
   int i, num;

   Class::Archive(arc);

   num = pathlist.NumObjects();
   arc.WriteInteger(num);
   for(i = 1; i <= num; i++)
   {
      ptr = pathlist.ObjectAt(i);
      arc.WriteSafePointer(ptr);
   }

   arc.WriteFloat(pathlength);
   arc.WriteSafePointer(from);
   arc.WriteSafePointer(to);
   arc.WriteInteger(nextnode);
}

inline EXPORT_FROM_DLL void Path::Unarchive(Archiver &arc)
{
   PathNodePtr *ptr;
   PathNodePtr node;
   float len;
   Vector dir;
   int i, num;

   Class::Unarchive(arc);

   pathlist.FreeObjectList();
   distanceToNextNode.FreeObjectList();
   dirToNextNode.FreeObjectList();

   arc.ReadInteger(&num);
   for(i = 1; i <= num; i++)
   {
      pathlist.AddObject(node);
      ptr = pathlist.AddressOfObjectAt(i);
      arc.ReadSafePointer(ptr);
   }

   // Recalculate the path distances and directions
   // only go up to the node before the last node.
   for(i = 1; i < num; i++)
   {
      dir = pathlist.ObjectAt(i + 1)->worldorigin - pathlist.ObjectAt(i)->worldorigin;
      len = dir.length();
      dir *= 1 / len;

      distanceToNextNode.SetObjectAt(i, len);
      dirToNextNode.SetObjectAt(i, dir);
   }

   if(num)
   {
      // special case for last node
      len = 0;
      distanceToNextNode.AddObject(len);
      dirToNextNode.AddObject(vec_zero);
   }

   arc.ReadFloat(&pathlength);
   arc.ReadSafePointer(&from);
   arc.ReadSafePointer(&to);
   arc.ReadInteger(&nextnode);
}


#ifdef EXPORT_TEMPLATE
template class EXPORT_FROM_DLL SafePtr<Path>;
#endif
typedef SafePtr<Path> PathPtr;

// EOF
