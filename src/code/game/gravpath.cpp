//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/gravpath.cpp                     $
// $Revision:: 18                                                             $
//   $Author:: Markd                                                          $
//     $Date:: 10/24/98 12:42a                                                $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Gravity path - Used for underwater currents and wells.

#include "g_local.h"
#include "entity.h"
#include "gravpath.h"
#include "container.h"
#include "navigate.h"
#include "misc.h"
#include "player.h"

GravPathManager gravPathManager;

CLASS_DECLARATION(Class, GravPathManager, NULL);

ResponseDef GravPathManager::Responses[] =
{
   { NULL,NULL }
};

GravPathManager::~GravPathManager()
{
   Reset();
}

void GravPathManager::Reset(void)
{
   while(pathList.NumObjects() > 0)
   {
      delete (GravPath *)pathList.ObjectAt(1);
   }

   pathList.FreeObjectList();
}

void GravPathManager::AddPath(GravPath *p)
{
   int num;
   num = pathList.AddObject(p);
   pathList.Resize(pathList.NumObjects());
}

void GravPathManager::RemovePath(GravPath *p)
{
   pathList.RemoveObject(p);
   pathList.Resize(pathList.NumObjects());
}

Vector GravPathManager::CalculateGravityPull(Entity &ent, Vector pos, qboolean *force)
{
   int            i, num;
   GravPath       *p;
   GravPathNode   *node;
   Vector         point;
   Vector         newpoint;
   Vector         dir;
   float          bestdist = 99999;
   float          dist;
   float          speed;
   float          radius;
   Vector         velocity{};
   int            bestpath = 0;
   int            entity_contents, grav_contents;

   num = pathList.NumObjects();

   entity_contents = gi.pointcontents(ent.worldorigin.vec3());

   for(i = 1; i <= num; i++)
   {
      p = (GravPath *)pathList.ObjectAt(i);

      if(!p)
         continue;

      // Check to see if path is active
      node = p->GetNode(1);
      if(!node || !node->active)
         continue;

      // Check to see if the contents are the same
      grav_contents = gi.pointcontents(node->worldorigin.vec3());

      // If grav node is in water, make sure ent is too.
      if((grav_contents & CONTENTS_WATER) && !(entity_contents & CONTENTS_WATER))
         continue;

      // Test to see if we are in this path's bounding box
      if((pos.x < p->maxs.x) && (pos.y < p->maxs.y) && (pos.z < p->maxs.z) &&
         (pos.x > p->mins.x) && (pos.y > p->mins.y) && (pos.z > p->mins.z))
      {
         point = p->ClosestPointOnPath(pos, ent, &dist, &speed, &radius);

         // If the closest distance on the path is greater than the radius, then 
         // do not consider this path.

         if(dist > radius)
         {
            continue;
         }
         else if(dist < bestdist)
         {
            bestpath = i;
            bestdist = dist;
         }
      }
   }

   if(!bestpath)
   {
      return vec_zero;
   }

   p = (GravPath *)pathList.ObjectAt(bestpath);
   if(!p)
      return velocity;
   *force = p->force;
   dist = p->DistanceAlongPath(pos, &speed);
   newpoint = p->PointAtDistance(dist + speed);
   dir = newpoint - pos;
   dir.normalize();
   velocity = dir * speed;
   return velocity;
}

/*****************************************************************************/
/*SINED info_grav_pathnode (0 0 .5) (-16 -16 0) (16 16 32) HEADNODE FORCE
 "radius" Radius of the effect of the pull (Default is 256)
 "speed"  Speed of the pull (Use negative for a repulsion) (Default is 100)

  Set HEADNODE to signify the head of the path.
  Set FORCE if you want un-fightable gravity ( i.e. can't go backwards )
/*****************************************************************************/

CLASS_DECLARATION( Entity, GravPathNode, "info_grav_pathnode" );

Event EV_GravPath_Create( "gravpath_create" );
Event EV_GravPath_Activate( "activate" );
Event EV_GravPath_Deactivate( "deactivate" );

ResponseDef GravPathNode::Responses[] =
{
   { &EV_GravPath_Create,           (Response)&GravPathNode::CreatePath },
   { &EV_GravPath_Activate,         (Response)&GravPathNode::Activate },
   { &EV_GravPath_Deactivate,       (Response)&GravPathNode::Deactivate },
   { NULL, NULL }
};

GravPathNode::GravPathNode() : Entity()
{
   setMoveType(MOVETYPE_NONE);
   setSolidType(SOLID_NOT);
   hideModel();

   speed = G_GetFloatArg("speed", 100.0f);
   radius = G_GetFloatArg("radius", 256.0f);
   headnode = spawnflags & 1;
   active = true;

   // This is the head of a new path, post an event to create the path
   if(headnode)
   {
      PostEvent(EV_GravPath_Create, 0);
   }
}

float GravPathNode::Speed() const
{
   if(active)
      return speed;
   else
      return 0;
};

void GravPathNode::Activate(Event *ev)
{
   GravPathNode   *node;
   int            num;
   const char     *target;

   active = true;
   node = this;
   // Go through the entire path and activate it
   target = node->Target();
   while(target[0])
   {
      if(num = G_FindTarget(0, target))
      {
         node = (GravPathNode *)G_GetEntity(num);
         assert(node);
         node->active = true;
      }
      else
      {
         gi.error("GravPathNode::CreatePath: target %s not found\n", target);
      }
      target = node->Target();
   }
}

void GravPathNode::Deactivate(Event *ev)
{
   GravPathNode   *node;
   int            num;
   const char     *target;

   active = false;
   node = this;
   // Go through the entire path and activate it
   target = node->Target();
   while(target[0])
   {
      if(num = G_FindTarget(0, target))
      {
         node = (GravPathNode *)G_GetEntity(num);
         assert(node);
         node->active = false;
      }
      else
      {
         gi.error("GravPathNode::CreatePath: target %s not found\n", target);
      }
      target = node->Target();
   }
}

void GravPathNode::CreatePath(Event *ev)
{
   const char     *target;
   GravPath       *path = new GravPath();
   GravPathNode   *node;
   int            num;

   ClearBounds(path->mins.vec3(), path->maxs.vec3());

   // This node is the head of a path, create a new path in the path manager.
   // and add it in, then add all of it's children in the path.
   node = this;
   path->AddNode(node);
   path->force = spawnflags & 2;

   // Make the path from the targetlist.
   target = node->Target();
   while(target[0])
   {
      if(num = G_FindTarget(0, target))
      {
         node = (GravPathNode *)G_GetEntity(num);
         assert(node);
         path->AddNode(node);
      }
      else
      {
         gi.error("GravPathNode::CreatePath: target %s not found\n", target);
      }
      target = node->Target();
   }

   // Set the origin.
   path->origin = path->mins + path->maxs;
   path->origin *= 0.5f;
}

CLASS_DECLARATION(Listener, GravPath, NULL);

Event EV_DrawGravPath("drawpath");

ResponseDef GravPath::Responses[] =
{
   { &EV_DrawGravPath, (Response)&GravPath::DrawPath },
   { NULL, NULL }
};

GravPath::GravPath() : Listener()
{
   // Event *event;

   if(!LoadingSavegame)
   {
      gravPathManager.AddPath(this);
   }

   // event = new Event(EV_DrawGravPath);
   // event->AddFloat(1);
   // event->AddFloat(0);
   // event->AddFloat(0);
   // PostEvent(event,0.1f);
}

GravPath::~GravPath()
{
   pathlength = 0;
   from = NULL;
   to = NULL;
   nextnode = 1;
   gravPathManager.RemovePath(this);
}

void GravPath::Clear(void)
{
   nextnode = 1;
   pathlength = 0;
   from = NULL;
   to = NULL;
   pathlist.FreeObjectList();
}

void GravPath::Reset(void)
{
   nextnode = 1;
}

GravPathNode *GravPath::Start(void)
{
   return from;
}

GravPathNode *GravPath::End(void)
{
   return to;
}

void GravPath::AddNode(GravPathNode *node)
{
   int num;
   Vector r, addp;

   if(!from)
   {
      from = node;
   }

   to = node;
   pathlist.AddObject(GravPathNodePtr(node));

   num = NumNodes();
   if(num > 1)
   {
      pathlength += (node->worldorigin - GetNode(num)->worldorigin).length();
   }

   r.setXYZ(node->Radius(), node->Radius(), node->Radius());
   addp = node->worldorigin + r;
   AddPointToBounds(addp.vec3(), mins.vec3(), maxs.vec3());
   addp = node->worldorigin - r;
   AddPointToBounds(addp.vec3(), mins.vec3(), maxs.vec3());
}

GravPathNode *GravPath::GetNode(int num)
{
   return pathlist.ObjectAt(num);
}

GravPathNode *GravPath::NextNode(void)
{
   if(nextnode <= NumNodes())
   {
      return pathlist.ObjectAt(nextnode++);
   }
   return NULL;
}

Vector GravPath::ClosestPointOnPath(Vector pos, Entity &ent, float *ret_dist, float *speed, float *radius)
{
   GravPathNode	*s;
   GravPathNode	*e;
   int	      	num;
   int		      i;
   float		      bestdist;
   Vector	      bestpoint;
   float		      dist;
   float		      segmentlength;
   Vector	      delta;
   Vector	      p1;
   Vector      	p2;
   Vector	      p3;
   float		      t;
   trace_t	      trace;

   num = NumNodes();
   s = GetNode(1);
   trace = G_Trace(pos, ent.mins, ent.maxs, s->worldorigin, &ent, MASK_PLAYERSOLID, "GravPath::ClosestPointOnPath 1");
   bestpoint = s->worldorigin;
   delta = bestpoint - pos;
   bestdist = delta.length();
   *speed = s->Speed();
   *radius = s->Radius();

   for(i = 2; i <= num; i++)
   {
      e = GetNode(i);

      // check if we're closest to the endpoint
      delta = e->worldorigin - pos;
      dist = delta.length();

      if(dist < bestdist)
      {
         trace = G_Trace(pos, ent.mins, ent.maxs, e->worldorigin, &ent, MASK_PLAYERSOLID, "GravPath::ClosestPointOnPath 2");
         bestdist = dist;
         bestpoint = e->worldorigin;
         *speed = e->Speed();
         *radius = e->Radius();
      }

      // check if we're closest to the segment
      p1 = e->worldorigin - s->worldorigin;
      segmentlength = p1.length();
      p1 *= 1 / segmentlength;
      p2 = pos - s->worldorigin;

      t = p1 * p2;
      if((t > 0) && (t < segmentlength))
      {
         p3 = (p1 * t) + s->worldorigin;

         delta = p3 - pos;
         dist = delta.length();
         if(dist < bestdist)
         {
            trace = G_Trace(pos, ent.mins, ent.maxs, p3, &ent, MASK_PLAYERSOLID, "GravPath::ClosestPointOnPath 3");
            bestdist = dist;
            bestpoint = p3;
            *speed = (e->Speed() * t) + (s->Speed() * (1.0f - t));
            *radius = (e->Radius() * t) + (s->Radius() * (1.0f - t));
         }
      }

      s = e;
   }
   *ret_dist = bestdist;
   return bestpoint;
}

float GravPath::DistanceAlongPath(Vector pos, float *speed)
{
   GravPathNode	*s;
   GravPathNode	*e;
   int	      	num;
   int		      i;
   float		      bestdist;
   float		      dist;
   float		      segmentlength;
   Vector	      delta;
   Vector	      segment;
   Vector      	p1;
   Vector	      p2;
   Vector	      p3;
   float		      t;
   float		      pathdist;
   float		      bestdistalongpath;
   float          oosl;
   pathdist = 0;

   num = NumNodes();
   s = GetNode(1);
   delta = s->worldorigin - pos;
   bestdist = delta.length();
   bestdistalongpath = 0;
   *speed = s->Speed();

   for(i = 2; i <= num; i++)
   {
      e = GetNode(i);

      segment = e->worldorigin - s->worldorigin;
      segmentlength = segment.length();

      // check if we're closest to the endpoint
      delta = e->worldorigin - pos;
      dist = delta.length();

      if(dist < bestdist)
      {
         bestdist = dist;
         bestdistalongpath = pathdist + segmentlength;
         *speed = e->Speed();
      }

      // check if we're closest to the segment
      oosl = (1 / segmentlength);
      p1 = segment * oosl;
      p1.normalize();
      p2 = pos - s->worldorigin;

      t = p1 * p2;
      if((t > 0) && (t < segmentlength))
      {
         p3 = (p1 * t) + s->worldorigin;

         delta = p3 - pos;
         dist = delta.length();
         if(dist < bestdist)
         {
            bestdist = dist;
            bestdistalongpath = pathdist + t;

            t *= oosl;
            *speed = (e->Speed() * t) + (s->Speed() * (1.0f - t));
         }
      }

      s = e;
      pathdist += segmentlength;
   }

   return bestdistalongpath;
}

Vector GravPath::PointAtDistance(float dist)
{
   GravPathNode	*s;
   GravPathNode	*e;
   int      		num;
   int		      i;
   Vector	      delta;
   Vector	      p1;
   float	      	t;
   float		      pathdist;
   float		      segmentlength;

   num = NumNodes();
   s = GetNode(1);
   pathdist = 0;

   for(i = 2; i <= num; i++)
   {
      e = GetNode(i);

      delta = e->worldorigin - s->worldorigin;
      segmentlength = delta.length();

      if((pathdist + segmentlength) > dist)
      {
         t = dist - pathdist;

         p1 = delta * (t / segmentlength);
         return p1 + s->worldorigin;
      }

      s = e;
      pathdist += segmentlength;
   }

   // cap it off at start or end of path
   return s->worldorigin;
}

void GravPath::DrawPath(Event *ev)
{
   Vector	      s;
   Vector	      e;
   Vector	      offset;
   GravPathNode	*node;
   int		      num;
   int		      i;
   float          r = ev->GetFloat(1);
   float          g = ev->GetFloat(2);
   float          b = ev->GetFloat(3);
   Event          *event;

   num = NumNodes();
   node = GetNode(1);
   s = node->worldorigin;
   offset = Vector(r, g, b) * 4 + Vector(0, 0, 0);
   offset = Vector(0, 0, 0);

   for(i = 2; i <= num; i++)
   {
      node = GetNode(i);
      e = node->worldorigin;

      G_DebugLine(s + offset, e + offset, r, g, b, 1);
      s = e;
   }

   G_DebugBBox(origin, mins - origin, maxs - origin, 1, 0, 0, 1);

   event = new Event(EV_DrawGravPath);
   event->AddFloat(r);
   event->AddFloat(g);
   event->AddFloat(b);
   PostEvent(event, 0.1f);
}

int GravPath::NumNodes() const
{
   return pathlist.NumObjects();
}

float GravPath::Length() const
{
   return pathlength;
}

// EOF

