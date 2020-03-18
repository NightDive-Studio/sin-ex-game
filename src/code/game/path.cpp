//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/path.cpp                         $
// $Revision:: 33                                                             $
//   $Author:: Jimdose                                                        $
//     $Date:: 10/16/98 8:25p                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// 

#include "g_local.h"
#include "entity.h"
#include "path.h"
#include "container.h"
#include "navigate.h"
#include "misc.h"

CLASS_DECLARATION( Class, Path, nullptr );

ResponseDef Path::Responses[] =
{
   { nullptr, nullptr }
};

Path::Path(int numnodes) : Path()
{
   pathlist.Resize(numnodes);
   dirToNextNode.Resize(numnodes);
   distanceToNextNode.Resize(numnodes);
}

void Path::Clear()
{
   nextnode = 1;
   pathlength = 0;
   from = nullptr;
   to = nullptr;
   pathlist.FreeObjectList();
   dirToNextNode.FreeObjectList();
   distanceToNextNode.FreeObjectList();
}

void Path::Reset()
{
   nextnode = 1;
}

PathNode *Path::Start()
{
   return from;
}

PathNode *Path::End()
{
   return to;
}

void Path::AddNode(PathNode *node)
{
   Vector dir;
   float len;
   int num;

   if(!from)
   {
      from = node;
   }

   to = node;
   pathlist.AddObject(PathNodePtr(node));

   len = 0;
   distanceToNextNode.AddObject(len);
   dirToNextNode.AddObject(vec_zero);

   num = NumNodes();
   if(num > 1)
   {
      dir = node->worldorigin - GetNode(num - 1)->worldorigin;
      len = dir.length();
      dir *= 1 / len;

      distanceToNextNode.SetObjectAt(num - 1, len);
      dirToNextNode.SetObjectAt(num - 1, dir);

      pathlength += len;
   }
}

PathNode *Path::GetNode(int num)
{
   PathNode *node;

   node = pathlist.ObjectAt(num);
   assert(node != nullptr);
   if(node == nullptr)
   {
      error("GetNode", "Null pointer in node list\n");
   }

   return node;
}

PathNode *Path::NextNode()
{
   if(nextnode <= NumNodes())
   {
      return pathlist.ObjectAt(nextnode++);
   }
   return nullptr;
}

PathNode *Path::NextNode(PathNode *node)
{
   int i;
   int num;
   PathNode *n;

   num = NumNodes();

   // NOTE: We specifically DON'T check the last object (hence the i < num instead
   // of the usual i <= num, so don't go doing something stupid like trying to fix
   // this without keeping this in mind!! :)
   for(i = 1; i < num; i++)
   {
      n = pathlist.ObjectAt(i);
      if(n == node)
      {
         // Since we only check up to num - 1, it's ok to do this.
         // We do this since the last node in the list has no next node (duh!).
         return pathlist.ObjectAt(i + 1);
      }
   }

   return nullptr;
}

Vector Path::ClosestPointOnPath(Vector pos)
{
   PathNode	*s;
   PathNode	*e;
   int		num;
   int		i;
   float		bestdist;
   Vector	bestpoint;
   float		dist;
   float		segmentlength;
   Vector	delta;
   Vector	p1;
   Vector	p2;
   Vector	p3;
   float		t;

   num = NumNodes();
   s = GetNode(1);

   bestpoint = s->worldorigin;
   delta = bestpoint - pos;
   bestdist = delta * delta;

   for(i = 2; i <= num; i++)
   {
      e = GetNode(i);

      // check if we're closest to the endpoint
      delta = e->worldorigin - pos;
      dist = delta * delta;

      if(dist < bestdist)
      {
         bestdist = dist;
         bestpoint = e->worldorigin;
      }

      // check if we're closest to the segment
      segmentlength = distanceToNextNode.ObjectAt(i - 1);
      p1 = dirToNextNode.ObjectAt(i - 1);
      p2 = pos - s->worldorigin;

      t = p1 * p2;
      if((t > 0) && (t < segmentlength))
      {
         p3 = (p1 * t) + s->worldorigin;

         delta = p3 - pos;
         dist = delta * delta;
         if(dist < bestdist)
         {
            bestdist = dist;
            bestpoint = p3;
         }
      }

      s = e;
   }

   return bestpoint;
}

float Path::DistanceAlongPath(Vector pos)
{
   PathNode	*s;
   PathNode	*e;
   int		num;
   int		i;
   float		bestdist;
   float		dist;
   float		segmentlength;
   Vector	delta;
   Vector	p1;
   Vector	p2;
   Vector	p3;
   float		t;
   float		pathdist;
   float		bestdistalongpath;

   pathdist = 0;

   num = NumNodes();
   s = GetNode(1);
   delta = s->worldorigin - pos;
   bestdist = delta * delta;
   bestdistalongpath = 0;

   for(i = 2; i <= num; i++)
   {
      e = GetNode(i);

      segmentlength = distanceToNextNode.ObjectAt(i - 1);

      // check if we're closest to the endpoint
      delta = e->worldorigin - pos;
      dist = delta * delta;

      if(dist < bestdist)
      {
         bestdist = dist;
         bestdistalongpath = pathdist + segmentlength;
      }

      // check if we're closest to the segment
      p1 = dirToNextNode.ObjectAt(i - 1);
      p2 = pos - s->worldorigin;

      t = p1 * p2;
      if((t > 0) && (t < segmentlength))
      {
         p3 = (p1 * t) + s->worldorigin;

         delta = p3 - pos;
         dist = delta * delta;
         if(dist < bestdist)
         {
            bestdist = dist;
            bestdistalongpath = pathdist + t;
         }
      }

      s = e;

      pathdist += segmentlength;
   }

   return bestdistalongpath;
}

Vector Path::PointAtDistance(float dist)
{
   PathNode	*s;
   PathNode	*e;
   int		num;
   int		i;
   float		t;
   float		pathdist;
   float		segmentlength;

   num = NumNodes();
   s = GetNode(1);
   pathdist = 0;

   for(i = 2; i <= num; i++)
   {
      e = GetNode(i);

      segmentlength = distanceToNextNode.ObjectAt(i - 1);
      if((pathdist + segmentlength) > dist)
      {
         t = dist - pathdist;
         return s->worldorigin + dirToNextNode.ObjectAt(i - 1) * t;
      }

      s = e;
      pathdist += segmentlength;
   }

   // cap it off at start or end of path
   return s->worldorigin;
}

PathNode *Path::NextNode(float dist)
{
   PathNode	*s;
   PathNode	*e;
   int		num;
   int		i;
   float		pathdist;
   float		segmentlength;

   num = NumNodes();
   s = GetNode(1);
   pathdist = 0;

   for(i = 2; i <= num; i++)
   {
      e = GetNode(i);

      segmentlength = distanceToNextNode.ObjectAt(i - 1);
      if((pathdist + segmentlength) > dist)
      {
         return e;
      }

      s = e;
      pathdist += segmentlength;
   }

   // cap it off at start or end of path
   return s;
}

void Path::DrawPath(float r, float g, float b, float time)
{
   Vector	s;
   Vector	e;
   Vector	offset;
   PathNode	*node;
   int		num;
   int		i;

   num = NumNodes();

   if(ai_debugpath->value)
   {
      gi.dprintf("numnodes %d, len %d, nodes %d :", PathManager.NumNodes(), (int)Length(), num);
      for(i = 1; i <= num; i++)
      {
         node = GetNode(i);
         gi.dprintf(" %d", node->nodenum);
      }

      gi.dprintf("\n");
   }

   node = GetNode(1);
   s = node->worldorigin;

   offset = Vector(r, g, b) * 4 + Vector(0, 0, 20);
   for(i = 2; i <= num; i++)
   {
      node = GetNode(i);
      e = node->worldorigin;

      G_DebugLine(s + offset, e + offset, r, g, b, 1);
      s = e;
   }
}

int Path::NumNodes()
{
   return pathlist.NumObjects();
}

float Path::Length()
{
   return pathlength;
}

// EOF

