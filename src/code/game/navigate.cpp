//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/navigate.cpp                     $
// $Revision:: 68                                                             $
//   $Author:: Markd                                                          $
//     $Date:: 11/18/98 7:47p                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// C++ implementation of the A* search algorithm.
// 

#include "g_local.h"
#include "navigate.h"
#include "path.h"
#include "misc.h"
#include "doors.h"

#define PATHFILE_VERSION 4

Event EV_AI_SavePaths("ai_savepaths", EV_CHEAT);
Event EV_AI_SaveNodes("ai_save", EV_CHEAT);
Event EV_AI_LoadNodes("ai_load", EV_CHEAT);
Event EV_AI_ClearNodes("ai_clearnodes", EV_CHEAT);
Event EV_AI_RecalcPaths("ai_recalcpaths", EV_CHEAT);
Event EV_AI_CalcPath("ai_calcpath", EV_CHEAT);
Event EV_AI_DisconnectPath("ai_disconnectpath", EV_CHEAT);
Event EV_AI_SetNodeFlags("ai_setflags", EV_CHEAT);

cvar_t	*ai_createnodes = NULL;
cvar_t	*ai_showpath;
cvar_t	*ai_debugpath;
cvar_t	*ai_debuginfo;
cvar_t	*ai_showroutes;
cvar_t   *ai_shownodenums;
cvar_t   *ai_timepaths;

static Entity	*IgnoreObjects[MAX_EDICTS];
static int		NumIgnoreObjects;

static PathNode *pathnodes[MAX_PATHNODES];
static qboolean pathnodesinitialized = false;
static qboolean loadingarchive = false;
int ai_maxnode;

#define	MASK_PATHSOLID		(CONTENTS_SOLID|CONTENTS_MONSTERCLIP|CONTENTS_WINDOW|CONTENTS_FENCE)

PathSearch PathManager;

int path_checksthisframe;

PathNode *AI_FindNode(const char *name)
{
   int i;

   if(!name)
   {
      return NULL;
   }

   if(name[0] == '!')
   {
      name++;
      return AI_GetNode(atof(name));
   }

   if(name[0] == '$')
   {
      name++;
   }

   for(i = 0; i <= ai_maxnode; i++)
   {
      if(pathnodes[i] && (pathnodes[i]->TargetName() == name))
      {
         return pathnodes[i];
      }
   }

   return NULL;
}

PathNode *AI_GetNode(int num)
{
   if((num < 0) || (num > MAX_PATHNODES))
   {
      assert(false);
      return NULL;
   }

   return pathnodes[num];
}

void AI_AddNode(PathNode *node)
{
   int i;

   assert(node);

   for(i = 0; i < MAX_PATHNODES; i++)
   {
      if(pathnodes[i] == NULL)
      {
         break;
      }
   }

   if(i < MAX_PATHNODES)
   {
      if(i > ai_maxnode)
      {
         ai_maxnode = i;
      }
      pathnodes[i] = node;
      node->nodenum = i;
      return;
   }

   gi.error("Exceeded MAX_PATHNODES!\n");
}

void AI_RemoveNode(PathNode *node)
{
   assert(node);
   if((node->nodenum < 0) || (node->nodenum > ai_maxnode))
   {
      assert(false);
      gi.error("Corrupt pathnode!\n");
   }

   // If the nodenum is 0, the node was probably removed during initialization
   // otherwise, it's a bug.
   assert((pathnodes[node->nodenum] == node) || (node->nodenum == 0));
   if(pathnodes[node->nodenum] == node)
   {
      pathnodes[node->nodenum] = NULL;
   }
}

void AI_ResetNodes(void)
{
   int i;

   if(!pathnodesinitialized)
   {
      memset(pathnodes, 0, sizeof(pathnodes));
      pathnodesinitialized = true;
   }
   else
   {
      for(i = 0; i < MAX_PATHNODES; i++)
      {
         if(pathnodes[i])
         {
            delete pathnodes[i];
         }
      }
   }

   ai_maxnode = 0;
}

/*****************************************************************************/
/*SINED info_pathnode (1 0 0) (-24 -24 0) (24 24 32) FLEE DUCK COVER DOOR JUMP LADDER

FLEE marks the node as a safe place to flee to.  Actor will be removed when it reaches a flee node and is not visible to a player.

DUCK marks the node as a good place to duck behind during weapon fire.

COVER marks the node as a good place to hide behind during weapon fire.

DOOR marks the node as a door node.  If an adjacent node has DOOR marked as well, the actor will only use the path if the door in between them is unlocked.

JUMP marks the node as one to jump from when going to the node specified by target.
"target" the pathnode to jump to.

/*****************************************************************************/

CLASS_DECLARATION(Listener, PathNode, "info_pathnode");

Event EV_Path_FindChildren("findchildren");
Event EV_Path_FindEntities("findentities");

ResponseDef PathNode::Responses[] =
{
   { &EV_Path_FindChildren,      (Response)&PathNode::FindChildren },
   { &EV_Path_FindEntities,      (Response)&PathNode::FindEntities },
   { NULL, NULL }
};

static int			numNodes = 0;
static PathNode	*NodeList = NULL;

PathNode::PathNode() : Listener()
{
   chain = NULL;
   gridX = gridY = 0;
   drawtime = 0;
   entnum = 0;
   nodenum = 0;
   if(!loadingarchive)
   {
      // our archive function will take care of this stuff
      AI_AddNode(this);
      PostEvent(EV_Path_FindChildren, 0);
   }

   occupiedTime = 0;

   nodeflags = G_GetIntArg("spawnflags");

   setOrigin(Vector(G_GetSpawnArg("origin")));

   setangles = (G_GetSpawnArg("angle") || G_GetSpawnArg("angles"));
   if(setangles)
   {
      float angle;

      angle = G_GetFloatArg("angle", 0);
      setAngles(G_GetVectorArg("angles", Vector(0, angle, 0)));
   }

   animname = G_GetStringArg("anim");
   targetname = G_GetStringArg("targetname");
   target = G_GetStringArg("target");

   // crouch height
   setSize({ -24, -24, 0 }, { 24, 24, 40 });

   f = 0;
   h = 0;
   g = 0;

   inlist = NOT_IN_LIST;

   // reject is used to indicate that a node is unfit for ending on during a search
   reject = false;

   numChildren = 0;

   Parent = NULL;
   NextNode = NULL;
}

PathNode::~PathNode()
{
   PathManager.RemoveNode(this);

   AI_RemoveNode(this);
}

str &PathNode::TargetName(void)
{
   return targetname;
}

void PathNode::setAngles(const Vector &ang)
{
   worldangles = ang;
}

void PathNode::setOrigin(Vector org)
{
   worldorigin = org;
   contents = gi.pointcontents(worldorigin.vec3());
}

void PathNode::setSize(Vector min, Vector max)
{
   mins = min;
   maxs = max;
}

EXPORT_FROM_DLL void PathNode::Setup(Vector pos)
{
   CancelEventsOfType(EV_Path_FindChildren);

   setOrigin(pos);

   ProcessEvent(EV_Path_FindChildren);
}

EXPORT_FROM_DLL void PathNode::Archive(Archiver &arc)
{
   int i;

   arc.WriteInteger(nodenum);
   arc.WriteInteger(nodeflags);
   arc.WriteVector(worldorigin);
   arc.WriteVector(worldangles);
   arc.WriteBoolean(setangles);
   arc.WriteString(target);
   arc.WriteString(targetname);
   arc.WriteString(animname);

   arc.WriteFloat(occupiedTime);
   arc.WriteInteger(entnum);

   arc.WriteInteger(numChildren);
   for(i = 0; i < numChildren; i++)
   {
      arc.WriteShort(Child[i].node);
      arc.WriteShort(Child[i].moveCost);
      arc.WriteRaw(Child[i].maxheight, sizeof(Child[i].maxheight));
      arc.WriteInteger(Child[i].door);
   }
}

EXPORT_FROM_DLL void PathNode::FindEntities(Event *ev)
{
   int i;
   Door *door;
   PathNode *node;

   for(i = 0; i < numChildren; i++)
   {
      if(Child[i].door)
      {
         node = AI_GetNode(Child[i].node);

         assert(node);

         door = CheckDoor(node->worldorigin);
         if(door)
         {
            Child[i].door = door->entnum;
         }
         else
         {
            Child[i].door = 0;
         }
      }
   }
}

EXPORT_FROM_DLL void PathNode::Unarchive(Archiver &arc)
{
   int i;

   nodenum = arc.ReadInteger();
   assert(nodenum <= MAX_PATHNODES);
   if(nodenum > MAX_PATHNODES)
   {
      arc.FileError("Node exceeds max path nodes");
   }

   nodeflags = arc.ReadInteger();

   setOrigin(arc.ReadVector());
   setAngles(arc.ReadVector());

   setangles = arc.ReadBoolean();
   target = arc.ReadString();
   targetname = arc.ReadString();
   animname = arc.ReadString();

   occupiedTime = arc.ReadFloat();
   entnum = arc.ReadInteger();
   if(!LoadingSavegame)
   {
      occupiedTime = 0;
      entnum = 0;
   }

   numChildren = arc.ReadInteger();
   assert(numChildren <= NUM_PATHSPERNODE);
   if(numChildren > NUM_PATHSPERNODE)
   {
      arc.FileError("Exceeded num paths per node");
   }

   for(i = 0; i < numChildren; i++)
   {
      Child[i].node = arc.ReadShort();
      Child[i].moveCost = arc.ReadShort();
      arc.ReadRaw(Child[i].maxheight, sizeof(Child[i].maxheight));
      Child[i].door = arc.ReadInteger();
   }

   if(!LoadingSavegame)
   {
      // Fixup the doors
      PostEvent(EV_Path_FindEntities, 0);
   }

   pathnodes[nodenum] = this;
   if(ai_maxnode < nodenum)
   {
      ai_maxnode = nodenum;
   }

   PathManager.AddNode(this);
}

EXPORT_FROM_DLL void RestoreEnts(void)
{
   int i;

   for(i = 0; i < NumIgnoreObjects; i++)
   {
      IgnoreObjects[i]->link();
   }
}

EXPORT_FROM_DLL qboolean PathNode::TestMove(Entity *ent, Vector start, Vector end, Vector &min, Vector &max, qboolean allowdoors, qboolean fulltest)
{
   // NOTE: TestMove may allow wide paths to succeed when they shouldn't since it 
   // may place the lower node above obstacles that actors can't step over.
   // Since any path that's wide enough for large boxes must also allow
   // thinner boxes to go through, you must ignore the results of TestMove
   // when thinner checks have already failed.
   trace_t  trace;
   Vector   end_trace;
   Vector   pos;
   Vector   dir;
   float    t;
   float    dist;

   // By requiring that paths have STEPSIZE headroom above the path, we simplify the test
   // to see if an actor can move to a node down to a simple trace.  By stepping up the start
   // and end points, we account for the actor's ability to step up any geometry lower than
   // STEPSIZE in height.
   start.z += STEPSIZE;
   end.z += STEPSIZE;

   // Check the move
   trace = G_Trace(start, min, max, end, ent, MASK_PATHSOLID, "PathNode::TestMove 1");
   if(trace.startsolid || (trace.fraction != 1))
   {
      // No direct path available.  The actor will most likely not be able to move through here.
      return false;
   }

   if(!fulltest)
   {
      // Since we're not doing a full test (full tests are only done when we're connecting nodes to save time),
      // we test to see if the midpoint of the move would only cause a change in height of STEPSIZE
      // from the predicted height.  This prevents most cases where a dropoff lies between a actor and a node.
      Vector pos;

      // Since we start and end are already STEPSIZE above the ground, we have to check twice STEPSIZE below
      // the midpoint to see if the midpoint is on the ground.
      dir = end - start;
      pos = start + dir * 0.5;
      end_trace = pos;
      end_trace.z -= STEPSIZE * 2;

      // Check that the midpos is onground.  This may fail on ok moves, but a true test would be too slow 
      // to do in real time.  Also, we may miss cases where a dropoff exists before or after the midpoint.
      // Once the actor is close enough to the drop off, it will discover the fall and hopefully try
      // another route.
      trace = G_Trace(pos, min, max, end_trace, ent, MASK_PATHSOLID, "PathNode::TestMove 2");
      if(trace.startsolid || (trace.fraction == 1))
      {
         // We're not on the ground, so there's a good posibility that we can't make this move without falling.
         return false;
      }
   }
   else if(!(contents & MASK_WATER))
   {
      // When we're creating the paths during load time, we do a much more exhaustive test to see if the
      // path is valid.  This test takes a bounding box and moves it 8 units at a time closer to the goal,
      // testing the ground after each move.  The test involves checking whether we will fall more than
      // STEPSIZE to the ground (since we've raised the start and end points STEPSIZE above the ground,
      // we must actually test 2 * STEPSIZE down to see if we're on the ground).  After each test, we set
      // the new height of the box to be STEPSIZE above the ground.  Each move closer to the goal is only
      // done horizontally to simulate how the actors normally move.  This method ensures that any actor
      // wider than 8 units in X and Y will be able to move from start to end.
      //
      // NOTE: This may allow wide paths to succeed when they shouldn't since it 
      // may place the lower node above obstacles that actors can't step over.
      // Since any path that's wide enough for large boxes must also allow
      // thinner boxes to go through, you must ignore the results of TestMove
      // when thinner checks have already failed.

      dir = end - start;
      dir.z = 0;
      dist = dir.length();
      dir *= 1 / dist;

      // check the entire move
      pos = start;
      for(t = 0; t < dist; t += 8)
      {
         // Move the box to our position along the path and make our downward trace vector
         end_trace.x = pos.x = start.x + t * dir.x;
         end_trace.y = pos.y = start.y + t * dir.y;
         end_trace.z = pos.z - STEPSIZE * 2;

         // check the ground
         trace = G_Trace(pos, min, max, end_trace, ent, MASK_PATHSOLID, "PathNode::TestMove 3");
         if(trace.startsolid || (trace.fraction == 1))
         {
            // Either we're stuck in something solid, or we would fall farther than STEPSIZE to the ground,
            // so the path is not acceptable.
            return false;
         }

         // move the box to STEPSIZE above the ground.
         pos.z = trace.endpos[2] + STEPSIZE;
      }
   }

   return true;
}

EXPORT_FROM_DLL qboolean PathNode::CheckMove(Entity *ent, Vector pos, Vector &min, Vector &max, qboolean allowdoors, qboolean fulltest)
{
   // Since we need to support actors of variable widths, we need to do some special checks when a potential
   // path goes up or down stairs.  Placed pathnodes are only 16x16 in width, so when they are dropped to the
   // ground, they may end in a position where a larger box would not fit.  Making the pathnodes larger
   // would make it hard to place paths where smaller actors could go, and making paths of various sizes would
   // be overkill (more work for the level designer, or a lot of redundant data).  The solution is to treat
   // paths with verticle movement differently than paths that are purely horizontal.  For horizontal moves,
   // a simple trace STEPSIZE above the ground will be sufficient to prove that we can travel from one node
   // to another, in either direction.  For moves that have some change in height, we can check that we have
   // a clear path by tracing horizontally from the higher node to a point where larger bounding box actors
   // could then move at a slope downward to the lower node.  This fixes the problem where path points that
   // are larger than the depth of a step would have to intersect with the step in order to get the center
   // of the box on solid ground.  If you're still confused, well, tough. :)  Think about the problem of
   // larger bounding boxes going up stairs for a bit and you should see the problem.  You can also read
   // section 8.4, "Translating a Convex Polygon", from Computational Geometry in C (O'Rourke) (a f'ing
   // great book, BTW) for information on similar problems (which is also a good explanation of how
   // Quake's collision detection works).
   trace_t trace;
   int height;

   height = (int)fabs(pos.z - worldorigin.z);
   // Check if the path is strictly horizontal
   if(!height)
   {
      // We do two traces for the strictly horizontal test.  One normal, and one STEPSIZE
      // above.  The normal trace is needed because most doors in the game aren't tall enough
      // to allow actors to trace STEPSIZE above the ground.  This means that failed horizontal
      // tests require two traces.  Annoying.
      trace = G_Trace(worldorigin, min, max, pos, ent, MASK_PATHSOLID, "PathNode::CheckMove 1");
      if(!trace.startsolid && (trace.fraction == 1))
      {
         return true;
      }

      // Do the step test
      return TestMove(ent, pos, worldorigin, min, max, allowdoors, fulltest);
   }

   Vector size;
   float  width;

   size = max - min;
   width = max(size.x, size.y);

   // if our bounding box is smaller than that of the pathnode, we can do the standard trace.
   if(width <= 32)
   {
      return TestMove(ent, pos, worldorigin, min, max, allowdoors, fulltest);
   }

   Vector start;
   Vector end;
   Vector delta;
   float  radius;
   float  len;

   // We calculate the radius of the smallest cylinder that would contain the bounding box.
   // In some cases, this would make the first horizontal move longer than it needs to be, but
   // that shouldn't be a problem.

   // multiply the width by 1/2 the square root of 2 to get radius
   radius = width * 1.415 * 0.5;

   // Make sure that our starting position is the higher node since it doesn't matter which
   // direction the move is in.
   if(pos.z < worldorigin.z)
   {
      start = worldorigin;
      end = pos;
   }
   else
   {
      start = pos;
      end = worldorigin;
   }

   // If the distance between the two points is less than the radius of the bounding box,
   // then we only have to do the horizontal test since larger bounding boxes would not fall.
   delta = end - start;
   len = delta.length();
   if(len <= radius)
   {
      end.z = start.z;
      return TestMove(ent, start, end, min, max, allowdoors, fulltest);
   }

   Vector mid;

   // normalize delta and multiply by radius (saving a few multiplies by combining into one).
   delta *= radius / len;

   mid = start;
   mid.x += delta.x;
   mid.y += delta.y;

   // Check the horizontal move
   if(!TestMove(ent, start, mid, min, max, allowdoors, fulltest))
   {
      return false;
   }

   // Calculate our new endpoint
   end.z -= delta.z;

   // Check our new sloping move
   return TestMove(ent, mid, end, min, max, allowdoors, fulltest);
}

EXPORT_FROM_DLL Door *PathNode::CheckDoor(Vector pos)
{
   trace_t	trace;
   Entity	*ent;

   trace = G_Trace(worldorigin, vec_zero, vec_zero, pos, NULL, MASK_PATHSOLID, "PathNode::CheckDoor");

   ent = trace.ent->entity;
   if(ent && ent->isSubclassOf<Door>())
   {
      return static_cast<Door *>(ent);
   }

   return NULL;
}

EXPORT_FROM_DLL qboolean PathNode::CheckMove(Vector pos, Vector min, Vector max)
{
   return true;
}

EXPORT_FROM_DLL qboolean PathNode::CheckPath(PathNode *node, Vector min, Vector max, qboolean fulltest)
{
   Vector	delta;
   qboolean allowdoors;
   qboolean result;

   delta = node->worldorigin - worldorigin;
   delta[2] = 0;
   if(delta.length() >= PATHMAP_CELLSIZE)
   {
      return false;
   }

   allowdoors = (nodeflags & AI_DOOR) && (node->nodeflags & AI_DOOR);

   result = CheckMove(NULL, node->worldorigin, min, max, allowdoors, fulltest);
   RestoreEnts();

   return result;
}

EXPORT_FROM_DLL qboolean PathNode::ClearPathTo(PathNode *node, byte maxheight[NUM_WIDTH_VALUES], qboolean fulltest)
{
   int      i;
   int      width;
   int      height;
   int      bottom;
   int      top;
   Vector   min;
   Vector   max;
   Vector   bmin;
   Vector   bmax;
   qboolean path;
   edict_t	*touch[MAX_EDICTS];
   Entity   *ent;
   int      num;

   path = false;
   for(i = 0; i < NUM_WIDTH_VALUES; i++)
   {
      maxheight[i] = 0;
   }

   width = NUM_WIDTH_VALUES * WIDTH_STEP * 0.5;
   min = Vector(-width, -width, 0);
   max = Vector(width, width, MAX_HEIGHT);
   G_CalcBoundsOfMove(worldorigin, node->worldorigin, min, max, &bmin, &bmax);

   num = gi.BoxEdicts(bmin.vec3(), bmax.vec3(), touch, MAX_EDICTS, AREA_SOLID);
   for(i = 0; i < num; i++)
   {
      ent = touch[i]->entity;
      if(ent && ent->isSubclassOf<Door>())
      {
         ent->unlink();
      }
   }

   for(i = 0; i < NUM_WIDTH_VALUES; i++)
   {
      width = (i + 1) * WIDTH_STEP * 0.5;

      min.x = min.y = -width;
      max.x = max.y = width;

      // Perform a binary search to find the height of the path.  Neat, eh? :)
      bottom = 0;
      top = MAX_HEIGHT;
      while(top >= bottom)
      {
         height = ((bottom + top + 3) >> 1) & ~0x3;
         if(!height)
         {
            break;
         }

         max.z = (float)height;
         if(!CheckPath(node, min, max, fulltest))
         {
            top = height - 4;
         }
         else
         {
            bottom = height + 4;
            maxheight[i] = height;
         }
      }

      if(!maxheight[i])
      {
         // If no paths were available at this width, don't allow any wider paths.
         // CheckPath uses TestMove which may allow wide paths to succeed when they
         // shouldn't since it may place the lower node above obstacles that actors
         // can't step over.  Since any path that's wide enough for large boxes must
         // also allow thinner boxes to go through, this check avoids the hole in
         // TestMove's functioning.
         break;
      }

      path = true;
   }

   // Restore the doors
   for(i = 0; i < num; i++)
   {
      ent = touch[i]->entity;
      if(ent && ent->isSubclassOf<Door>())
      {
         ent->link();
      }
   }

   return path;
}

EXPORT_FROM_DLL qboolean PathNode::LadderTo(PathNode *node, byte maxheight[NUM_WIDTH_VALUES])
{
   int i;
   int j;
   int m;
   int width;
   Vector min;
   Vector max;
   qboolean path;

   trace_t trace;

   if(!(nodeflags & AI_LADDER) || !(node->nodeflags & AI_LADDER))
   {
      return false;
   }

   if((worldorigin.x != node->worldorigin.x) || (worldorigin.y != node->worldorigin.y))
   {
      return false;
   }

   path = false;

   for(i = 0; i < NUM_WIDTH_VALUES; i++)
   {
      width = (i + 1) * WIDTH_STEP * 0.5;
      min = Vector(-width, -width, 12);
      max = Vector(width, width, 40);

      trace = G_Trace(worldorigin, min, max, node->worldorigin, NULL, MASK_PATHSOLID, "PathNode::LadderTo 1");
      if((trace.fraction != 1) || trace.startsolid)
      {
         maxheight[i] = 0;
         continue;
      }

      path = true;

      m = 40;
      for(j = 48; j < MAX_HEIGHT; j += 8)
      {
         max.z = j;
         trace = G_Trace(worldorigin, min, max, node->worldorigin, NULL, MASK_PATHSOLID, "PathNode::LadderTo 2");
         if((trace.fraction != 1) || trace.startsolid)
         {
            break;
         }

         m = j;
      }

      maxheight[i] = m;
   }

   return path;
}

EXPORT_FROM_DLL qboolean PathNode::ConnectedTo(PathNode *node)
{
   int i;

   for(i = 0; i < numChildren; i++)
   {
      if(Child[i].node == node->nodenum)
      {
         return true;
      }
   }

   return false;
}

EXPORT_FROM_DLL void PathNode::ConnectTo(PathNode *node, byte maxheight[NUM_WIDTH_VALUES], float cost, Door *door)
{
   int i;

   if((numChildren < NUM_PATHSPERNODE) && (node != this))
   {
      Child[numChildren].node = node->nodenum;
      for(i = 0; i < NUM_WIDTH_VALUES; i++)
      {
         Child[numChildren].maxheight[i] = maxheight[i];
      }
      Child[numChildren].moveCost = (int)cost;
      Child[numChildren].door = door ? door->entnum : 0;
      numChildren++;
   }
   else
   {
      warning("ConnectTo", "Child overflow");
   }
}

EXPORT_FROM_DLL void PathNode::ConnectTo(PathNode *node, byte maxheight[NUM_WIDTH_VALUES])
{
   Vector delta;
   Door *door;

   door = CheckDoor(node->worldorigin);
   delta = node->worldorigin - worldorigin;
   ConnectTo(node, maxheight, delta.length(), door);
}

EXPORT_FROM_DLL void PathNode::Disconnect(PathNode *node)
{
   int i;

   for(i = 0; i < numChildren; i++)
   {
      if(Child[i].node == node->nodenum)
      {
         break;
      }
   }

   // Should never happen, but let it slide after release
   assert(i != numChildren);
   if(i == numChildren)
   {
      return;
   }

   numChildren--;

   // Since we're not worried about the order of the nodes, just
   // move the last node into the slot occupied by the removed node.
   Child[i] = Child[numChildren];
   Child[numChildren].node = 0;
   Child[numChildren].moveCost = 0;
}

EXPORT_FROM_DLL void PathNode::FindChildren(Event *ev)
{
   trace_t	trace;
   Vector	end;
   Vector	start;

   worldorigin.x = ((int)(worldorigin.x * 0.125)) * 8;
   worldorigin.y = ((int)(worldorigin.y * 0.125)) * 8;
   setOrigin(worldorigin);

   if(!(contents & MASK_WATER))
   {
      start = worldorigin + Vector(0, 0, 1);
      end = worldorigin;
      end[2] -= 256;

      trace = G_Trace(start, mins, maxs, end, NULL, MASK_PATHSOLID, "PathNode::FindChildren");
      if(trace.fraction != 1 && !trace.allsolid)
      {
         setOrigin(trace.endpos);
      }
   }

   PathManager.AddNode(this);
}

EXPORT_FROM_DLL void PathNode::DrawConnections(void)
{
   int i;
   pathway_t *path;
   PathNode *node;

   for(i = 0; i < numChildren; i++)
   {
      path = &Child[i];
      node = AI_GetNode(path->node);

      G_DebugLine(worldorigin + Vector(0, 0, 24), node->worldorigin + Vector(0, 0, 24), 0.7, 0.7, 0, 1);
   }
}

EXPORT_FROM_DLL void DrawAllConnections(void)
{
   int         i;
   pathway_t   *path;
   PathNode    *node;
   PathNode    *n;
   Vector      down;
   Vector      up;
   Vector      dir;
   Vector      p1;
   Vector      p2;
   Vector      p3;
   Vector      playerorigin;
   qboolean    showroutes;
   qboolean    shownums;
   qboolean    draw;
   int         maxheight;
   int         pathnum;

   showroutes = (ai_showroutes->value != 0);
   shownums = (ai_shownodenums->value != 0);

   if(ai_showroutes->value == 1)
   {
      pathnum = (32 / WIDTH_STEP) - 1;
   }
   else
   {
      pathnum = (((int)ai_showroutes->value) / WIDTH_STEP) - 1;
   }

   if((pathnum < 0) || (pathnum >= MAX_WIDTH))
   {
      gi.printf("ai_showroutes: Value out of range\n");
      gi.cvar_set("ai_showroutes", "0");
      return;
   }

   // Figure out where the camera is
   assert(g_edicts[1].client);
   playerorigin.x = g_edicts[1].client->ps.pmove.origin[0];
   playerorigin.y = g_edicts[1].client->ps.pmove.origin[1];
   playerorigin.z = g_edicts[1].client->ps.pmove.origin[2];
   playerorigin *= 0.125;
   playerorigin += g_edicts[1].client->ps.viewoffset;

   for(node = NodeList; node != NULL; node = node->chain)
   {
      if(!gi.inPVS(playerorigin.vec3(), node->worldorigin.vec3()))
      {
         continue;
      }

      if(shownums)
      {
         G_DrawDebugNumber(node->worldorigin + Vector(0, 0, 14), node->nodenum, 1.5, 1, 1, 0);
      }

      draw = false;
      for(i = 0; i < node->numChildren; i++)
      {
         path = &node->Child[i];
         n = AI_GetNode(path->node);

         if(!showroutes)
         {
            continue;
         }

         maxheight = path->maxheight[pathnum];
         if(maxheight == 0)
         {
            continue;
         }

         draw = true;

         // don't draw the path if it's already been drawn by the destination node
         if(n->drawtime < level.time)
         {
            down.z = 2;
            up.z = maxheight;
            G_DebugLine(node->worldorigin + down, n->worldorigin + down, 0, 1, 0, 1);
            G_DebugLine(n->worldorigin + down, n->worldorigin + up, 0, 1, 0, 1);
            G_DebugLine(node->worldorigin + up, n->worldorigin + up, 0, 1, 0, 1);
            G_DebugLine(node->worldorigin + up, node->worldorigin + down, 0, 1, 0, 1);
         }

         // draw an arrow for the direction
         dir.x = n->worldorigin.x - node->worldorigin.x;
         dir.y = n->worldorigin.y - node->worldorigin.y;
         dir.normalize();

         p1 = node->worldorigin;
         p1.z += maxheight * 0.5;
         p2 = dir * 8;
         p3 = p1 + p2 * 2;

         G_DebugLine(p1, p3, 0, 1, 0, 1);

         p2.z += 8;
         G_DebugLine(p3, p3 - p2, 0, 1, 0, 1);

         p2.z -= 16;
         G_DebugLine(p3, p3 - p2, 0, 1, 0, 1);
      }

      if(!draw)
      {
         // Put a little X where the node is to show that it had no connections
         p1 = node->worldorigin;
         p1.z += 2;

         p2 = Vector(12, 12, 0);
         G_DebugLine(p1 - p2, p1 + p2, 1, 0, 0, 1);

         p2.x = -12;
         G_DebugLine(p1 - p2, p1 + p2, 1, 0, 0, 1);
      }

      node->drawtime = level.time;
   }
}

MapCell::MapCell()
{
   Init();
}

MapCell::~MapCell()
{
   Init();
}

EXPORT_FROM_DLL void MapCell::Init(void)
{
   numnodes = 0;
   memset(nodes, 0, sizeof(nodes));
}

EXPORT_FROM_DLL qboolean MapCell::AddNode(PathNode *node)
{
   if(numnodes >= PATHMAP_NODES)
   {
      return false;
   }

   nodes[numnodes] = (short)node->nodenum;
   numnodes++;

   return true;
}

EXPORT_FROM_DLL qboolean MapCell::RemoveNode(PathNode *node)
{
   int i;
   int num;

   num = node->nodenum;
   for(i = 0; i < numnodes; i++)
   {
      if(num == (int)nodes[i])
      {
         break;
      }
   }

   if(i >= numnodes)
   {
      return false;
   }

   numnodes--;

   // Since we're not worried about the order of the nodes, just
   // move the last node into the slot occupied by the removed node.
   nodes[i] = nodes[numnodes];
   nodes[numnodes] = 0;

   return true;
}

EXPORT_FROM_DLL PathNode *MapCell::GetNode(int index)
{
   assert(index >= 0);
   assert(index < numnodes);
   if(index >= numnodes)
   {
      return NULL;
   }

   return AI_GetNode(nodes[index]);
}

EXPORT_FROM_DLL int MapCell::NumNodes(void)
{
   return numnodes;
}

/*                         All
                     work and no play
                 makes Jim a dull boy. All
               work and no  play makes Jim a  
             dull boy. All  work and no  play 
           makes Jim a dull boy. All work and no  
         play makes Jim a dull  boy. All work and 
        no play makes Jim a dull boy. All work and  
       no play makes Jim a dull boy. All work and no  
      play makes Jim a dull boy. All work and no play  
     makes Jim a dull boy. All work and no play makes  
    Jim a dull boy.  All work and no  play makes Jim a  
   dull boy. All work and no play makes Jim a dull boy.  
   All work and no play makes  Jim a dull boy. All work  
  and no play makes Jim a dull boy. All work and no play  
  makes Jim a dull boy. All work and no play makes Jim a  
 dull boy. All work and no play makes Jim a dull boy. All  
 work and no play makes  Jim a dull boy. All  work and no 
 play makes Jim a dull boy. All work and no play makes Jim  
 a dull boy. All work  and no play makes Jim  a dull boy. 
 All work and no play makes Jim  a dull boy. All work and 
 no play makes Jim a dull boy. All work and no play makes  
 Jim a dull boy.  All work and no  play makes Jim a  dull 
 boy. All work and no play makes Jim a dull boy. All work  
 and no play makes Jim  a dull boy. All work  and no play 
 makes Jim a dull boy.  All work and no play  makes Jim a 
 dull boy. All work and no play makes Jim a dull boy. All  
  work and no play makes Jim a dull boy. All work and no  
  play makes Jim a dull boy.  All work and no play makes  
   Jim a dull boy. All work and no play makes Jim a dull  
   boy. All work and no play  makes Jim a dull boy. All  
    work and no play makes Jim a dull boy. All work and  
     no play makes  Jim a dull  boy. All work  and no 
      play makes Jim a dull boy. All work and no play  
       makes Jim a dull  boy. All work and  no play 
        makes Jim a dull boy. All work and no play  
         makes Jim a  dull boy. All  work and no  
           play makes Jim a  dull boy. All work  
             and no play makes Jim a dull boy.  
               All work  and no  play makes  
                 Jim a dull boy. All work  
                     and no play makes  
                          Jim  a  
*/

CLASS_DECLARATION(Class, PathSearch, NULL);

ResponseDef PathSearch::Responses[] =
{
   { &EV_AI_SavePaths,				(Response)&PathSearch::SavePathsEvent },
   { &EV_AI_LoadNodes,				(Response)&PathSearch::LoadNodes },
   { &EV_AI_SaveNodes,				(Response)&PathSearch::SaveNodes },
   { &EV_AI_ClearNodes,				(Response)&PathSearch::ClearNodes },
   { &EV_AI_SetNodeFlags,        (Response)&PathSearch::SetNodeFlagsEvent },
   { &EV_AI_RecalcPaths,         (Response)&PathSearch::RecalcPathsEvent },
   { &EV_AI_CalcPath,            (Response)&PathSearch::CalcPathEvent },
   { &EV_AI_DisconnectPath,      (Response)&PathSearch::DisconnectPathEvent },

   { NULL, NULL }
};

void PathSearch::AddToGrid(PathNode *node, int x, int y)
{
   PathNode *node2;
   MapCell *cell;
   int numnodes;
   int i;
   int j;
   byte maxheight[NUM_WIDTH_VALUES];

   cell = GetNodesInCell(x, y);
   if(!cell)
   {
      return;
   }

   if(!cell->AddNode(node))
   {
      warning("AddToGrid", "Node overflow at ( %d, %d )\n", x, y);
      return;
   }

   if(!loadingarchive)
   {
      //
      // explicitly link up the targets and their destinations
      //
      if(node->nodeflags & AI_JUMP)
      {
         if(node->target.length() > 1)
         {
            node2 = AI_FindNode(node->target.c_str());
            if(node2)
            {
               for(j = 0; j < NUM_WIDTH_VALUES; j++)
               {
                  maxheight[j] = MAX_HEIGHT;
               }
               node->ConnectTo(node2, maxheight);
            }
         }
      }

      // Connect the node to its neighbors
      numnodes = cell->NumNodes();
      for(i = 0; i < numnodes; i++)
      {
         node2 = (PathNode *)cell->GetNode(i);
         if(node2 == node)
         {
            continue;
         }

         if((node->numChildren < NUM_PATHSPERNODE) && !node->ConnectedTo(node2))
         {
            if(node->ClearPathTo(node2, maxheight) || node->LadderTo(node2, maxheight))
            {
               node->ConnectTo(node2, maxheight);
            }
            else if((node->nodeflags & AI_JUMP) && (node->target == node2->targetname))
            {
               //FIXME
               // don't hardcode size
               for(j = 0; j < NUM_WIDTH_VALUES; j++)
               {
                  maxheight[j] = MAX_HEIGHT;
               }
               node->ConnectTo(node2, maxheight);
            }
         }

         if((node2->numChildren < NUM_PATHSPERNODE) && !node2->ConnectedTo(node))
         {
            if(node2->ClearPathTo(node, maxheight) || node2->LadderTo(node, maxheight))
            {
               node2->ConnectTo(node, maxheight);
            }
            else if((node2->nodeflags & AI_JUMP) && (node2->target == node->targetname))
            {
               //FIXME
               // don't hardcode size
               for(j = 0; j < NUM_WIDTH_VALUES; j++)
               {
                  maxheight[j] = MAX_HEIGHT;
               }
               node2->ConnectTo(node, maxheight);
            }
         }
      }
   }
}

qboolean PathSearch::RemoveFromGrid(PathNode *node, int x, int y)
{
   MapCell	*cell;
   PathNode *node2;
   int		numnodes;
   int		i;

   cell = GetNodesInCell(x, y);
   if(!cell || !cell->RemoveNode(node))
   {
      return false;
   }

   // Disconnect the node from all nodes in the cell
   numnodes = cell->NumNodes();
   for(i = 0; i < numnodes; i++)
   {
      node2 = (PathNode *)cell->GetNode(i);
      if(node2->ConnectedTo(node))
      {
         node2->Disconnect(node);
      }
   }

   return true;
}

int PathSearch::NodeCoordinate(float coord)
{
   return ((int)coord + 4096 - (PATHMAP_CELLSIZE / 2)) / PATHMAP_CELLSIZE;
}

int PathSearch::GridCoordinate(float coord)
{
   return ((int)coord + 4096) / PATHMAP_CELLSIZE;
}

void PathSearch::AddNode(PathNode *node)
{
   int x;
   int y;

   assert(node);

   numNodes++;

   if(NodeList == NULL)
   {
      NodeList = node;
      node->chain = NULL;
   }
   else
   {
      node->chain = NodeList;
      NodeList = node;
   }

   x = NodeCoordinate(node->worldorigin[0]);
   y = NodeCoordinate(node->worldorigin[1]);

   AddToGrid(node, x, y);
   AddToGrid(node, x + 1, y);
   AddToGrid(node, x, y + 1);
   AddToGrid(node, x + 1, y + 1);

   node->gridX = x;
   node->gridY = y;
}

void PathSearch::RemoveNode(PathNode *node)
{
   int x;
   int y;
   PathNode *n;
   PathNode *p;

   assert(node);

   x = node->gridX;
   y = node->gridY;

   RemoveFromGrid(node, x, y);
   RemoveFromGrid(node, x + 1, y);
   RemoveFromGrid(node, x, y + 1);
   RemoveFromGrid(node, x + 1, y + 1);

   p = NULL;
   for(n = NodeList; n != node; p = n, n = n->chain)
   {
      if(!n)
      {
         // Not in list.
         return;
      }
   }

   if(p)
   {
      p->chain = n->chain;
   }
   else
   {
      NodeList = n->chain;
   }

   n->chain = NULL;
   numNodes--;
}

void PathSearch::UpdateNode(PathNode *node)
{
   int x;
   int y;
   int mx;
   int my;

   assert(node);

   x = NodeCoordinate(node->worldorigin[0]);
   y = NodeCoordinate(node->worldorigin[1]);

   mx = node->gridX;
   my = node->gridY;

   RemoveFromGrid(node, mx, my);
   RemoveFromGrid(node, mx + 1, my);
   RemoveFromGrid(node, mx, my + 1);
   RemoveFromGrid(node, mx + 1, my + 1);

   node->numChildren = 0;

   AddToGrid(node, x, y);
   AddToGrid(node, x + 1, y);
   AddToGrid(node, x, y + 1);
   AddToGrid(node, x + 1, y + 1);

   node->gridX = x;
   node->gridY = y;
}

MapCell *PathSearch::GetNodesInCell(int x, int y)
{
   if((x < 0) || (x >= PATHMAP_GRIDSIZE) || (y < 0) || (y >= PATHMAP_GRIDSIZE))
   {
      return NULL;
   }

   return &PathMap[x][y];
}

MapCell *PathSearch::GetNodesInCell(Vector pos)
{
   int x;
   int y;

   x = GridCoordinate(pos[0]);
   y = GridCoordinate(pos[1]);

   return GetNodesInCell(x, y);
}

EXPORT_FROM_DLL PathNode *PathSearch::NearestNode(Vector pos, Entity *ent, qboolean usebbox)
{
   Vector	delta;
   PathNode	*node;
   PathNode	*bestnode;
   float		bestdist;
   float		dist;
   int		n;
   int		i;
   MapCell	*cell;
   Vector	min;
   Vector	max;

   cell = GetNodesInCell(pos);
   if(!cell)
   {
      return NULL;
   }

   if(ent && usebbox)
   {
      min = ent->mins;
      max = ent->maxs;
   }
   else
   {
      min = Vector(-16, -16, 12);
      max = Vector(16, 16, 40);
   }

   n = cell->NumNodes();

   if(ai_debugpath->value)
   {
      gi.dprintf("NearestNode: Checking %d nodes\n", n);
   }

   bestnode = NULL;
   bestdist = 999999999; // greater than ( 8192 * sqr(2) ) ^ 2 -- maximum squared distance
   for(i = 0; i < n; i++)
   {
      node = (PathNode *)cell->GetNode(i);
      if(!node)
      {
         continue;
      }

      delta = node->worldorigin - pos;

      // get the distance squared (faster than getting real distance)
      dist = delta * delta;
      if((dist < bestdist) && node->CheckMove(ent, pos, min, max, false, false))
      {
         bestnode = node;
         bestdist = dist;

         // if we're close enough, early exit
         if(dist < 16)
         {
            break;
         }
      }
   }

   return bestnode;
}

EXPORT_FROM_DLL void PathSearch::Teleport(Entity *teleportee, Vector from, Vector to)
{
   PathNode	*node1;
   PathNode	*node2;
   byte maxheight[NUM_WIDTH_VALUES];
   int j;

   if(ai_createnodes->value)
   {
      node1 = new PathNode();
      node1->Setup(from);

      node2 = new PathNode();
      node2->Setup(to);

      // FIXME
      // shouldn't hard-code width and height
      for(j = 0; j < NUM_WIDTH_VALUES; j++)
      {
         maxheight[j] = 72;
      }

      // connect with 0 cost
      node1->ConnectTo(node2, maxheight, 0);
   }
}

EXPORT_FROM_DLL void PathSearch::ShowNodes(void)
{
   if(ai_showroutes->value || ai_shownodenums->value)
   {
      DrawAllConnections();
   }
}

EXPORT_FROM_DLL int PathSearch::NumNodes(void)
{
   return numNodes;
}

EXPORT_FROM_DLL void PathSearch::Archive(Archiver &arc)
{
   PathNode *node;
   int i;
   int num;

   num = 0;
   for(i = 0; i < MAX_PATHNODES; i++)
   {
      node = AI_GetNode(i);
      if(node)
      {
         num++;
      }
   }

   arc.WriteInteger(num);
   for(i = 0; i < MAX_PATHNODES; i++)
   {
      node = AI_GetNode(i);
      if(node)
      {
         arc.WriteObject(node);
      }
   }

   if(ai_debuginfo->value)
   {
      gi.dprintf("Wrote %d path nodes\n", num);
   }
}

EXPORT_FROM_DLL void PathSearch::ClearNodes(Event *ev)
{
   PathNode *node;
   int i;
   int num;

   num = 0;
   for(i = 0; i < MAX_PATHNODES; i++)
   {
      node = AI_GetNode(i);
      if(node)
      {
         node->PostEvent(EV_Remove, 0);
         num++;
      }
   }

   if(ai_debuginfo->value)
   {
      gi.dprintf("Deleted %d path nodes\n", num);
   }
}

EXPORT_FROM_DLL void PathSearch::Unarchive(Archiver &arc)
{
   int num;
   int i;
   int x;
   int y;

   numNodes = 0;
   NodeList = NULL;
   loadingarchive = true;

   // Get rid of the nodes that were spawned by the map
   AI_ResetNodes();

   // Init the grid
   for(x = 0; x < PATHMAP_GRIDSIZE; x++)
   {
      for(y = 0; y < PATHMAP_GRIDSIZE; y++)
      {
         PathMap[x][y].Init();
      }
   }

   num = arc.ReadInteger();

   assert(num <= MAX_PATHNODES);
   if(num > MAX_PATHNODES)
   {
      arc.FileError("Exceeded max path nodes");
   }

   for(i = 0; i < num; i++)
   {
      arc.ReadObject();
   }

   if(ai_debuginfo->value)
   {
      gi.dprintf("Path nodes loaded: %d\n", NumNodes());
   }

   loadingarchive = false;
}

EXPORT_FROM_DLL void PathSearch::SetNodeFlagsEvent(Event *ev)
{
   const char * token;
   int i, argnum, flags;
   int mask;
   int action;
   int nodenum;
   PathNode *node;

#define FLAG_IGNORE  0
#define FLAG_SET     1
#define FLAG_CLEAR   2
#define FLAG_ADD     3

   nodenum = ev->GetInteger(1);
   node = AI_GetNode(nodenum);

   if(!node)
   {
      ev->Error("Node not found.");
      return;
   }

   flags = 0;
   argnum = 2;
   for(i = argnum; i <= ev->NumArgs(); i++)
   {
      token = ev->GetString(i);
      action = 0;
      switch(token[0])
      {
      case '+':
         action = FLAG_ADD;
         token++;
         break;
      case '-':
         action = FLAG_CLEAR;
         token++;
         break;
      default:
         action = FLAG_SET;
         break;
      }

      if(!strcmpi(token, "flee"))
      {
         mask = AI_FLEE;
      }
      else if(!strcmpi(token, "duck"))
      {
         mask = AI_DUCK;
      }
      else if(!strcmpi(token, "cover"))
      {
         mask = AI_COVER;
      }
      else if(!strcmpi(token, "door"))
      {
         mask = AI_DOOR;
      }
      else if(!strcmpi(token, "jump"))
      {
         mask = AI_JUMP;
      }
      else if(!strcmpi(token, "ladder"))
      {
         mask = AI_LADDER;
      }
      else if(!strcmpi(token, "action"))
      {
         mask = AI_ACTION;
      }
      else
      {
         action = FLAG_IGNORE;
         ev->Error("Unknown token %s.", token);
      }

      switch(action)
      {
      case FLAG_SET:
         node->nodeflags = 0;

      case FLAG_ADD:
         node->nodeflags |= mask;
         break;

      case FLAG_CLEAR:
         node->nodeflags &= ~mask;
         break;

      case FLAG_IGNORE:
         break;
      }
   }
}

EXPORT_FROM_DLL void PathSearch::CalcPathEvent(Event *ev)
{
   int nodenum;
   PathNode *node;
   PathNode *node2;
   int j;
   byte maxheight[NUM_WIDTH_VALUES];

   nodenum = ev->GetInteger(1);
   node = AI_GetNode(nodenum);

   nodenum = ev->GetInteger(2);
   node2 = AI_GetNode(nodenum);

   if(!node || !node2)
   {
      ev->Error("Node not found.");
      return;
   }

   if((node->numChildren < NUM_PATHSPERNODE) && !node->ConnectedTo(node2))
   {
      if(node->ClearPathTo(node2, maxheight, false) || node->LadderTo(node2, maxheight))
      {
         node->ConnectTo(node2, maxheight);
      }
      else if((node->nodeflags & AI_JUMP) && (node->target == node2->targetname))
      {
         //FIXME
         // don't hardcode size
         for(j = 0; j < NUM_WIDTH_VALUES; j++)
         {
            maxheight[j] = MAX_HEIGHT;
         }
         node->ConnectTo(node2, maxheight);
      }
   }

   if((node2->numChildren < NUM_PATHSPERNODE) && !node2->ConnectedTo(node))
   {
      if(node2->ClearPathTo(node, maxheight, false) || node2->LadderTo(node, maxheight))
      {
         node2->ConnectTo(node, maxheight);
      }
      else if((node2->nodeflags & AI_JUMP) && (node2->target == node->targetname))
      {
         //FIXME
         // don't hardcode size
         for(j = 0; j < NUM_WIDTH_VALUES; j++)
         {
            maxheight[j] = MAX_HEIGHT;
         }
         node2->ConnectTo(node, maxheight);
      }
   }
}

EXPORT_FROM_DLL void PathSearch::DisconnectPathEvent(Event *ev)
{
   int nodenum;
   PathNode *node;
   PathNode *node2;

   nodenum = ev->GetInteger(1);
   node = AI_GetNode(nodenum);

   nodenum = ev->GetInteger(2);
   node2 = AI_GetNode(nodenum);

   if(!node || !node2)
   {
      ev->Error("Node not found.");
      return;
   }

   if(node->ConnectedTo(node2))
   {
      node->Disconnect(node2);
   }

   if(node2->ConnectedTo(node))
   {
      node2->Disconnect(node);
   }
}

EXPORT_FROM_DLL void PathSearch::RecalcPathsEvent(Event *ev)
{
   int nodenum;
   PathNode *node;

   nodenum = ev->GetInteger(1);
   node = AI_GetNode(nodenum);
   if(node)
   {
      UpdateNode(node);
   }
   else
   {
      ev->Error("Node not found.");
   }
}

EXPORT_FROM_DLL void PathSearch::SaveNodes(Event *ev)
{
   Archiver arc;
   str name;

   if(ev->NumArgs() != 1)
   {
      gi.printf("Usage: ai_save [filename]\n");
      return;
   }

   name = ev->GetString(1);

   gi.printf("Archiving\n");

   arc.Create(name);
   arc.WriteInteger(PATHFILE_VERSION);
   arc.WriteObject(this);
   arc.Close();

   gi.printf("done.\n");
}

EXPORT_FROM_DLL void PathSearch::LoadNodes(Event *ev)
{
   Archiver arc;
   str		name;
   int		version;

   if(ev->NumArgs() != 1)
   {
      gi.printf("Usage: ai_load [filename]\n");
      return;
   }

   gi.printf("Loading nodes...\n");

   name = ev->GetString(1);

   arc.Read(name);
   version = arc.ReadInteger();
   if(version == PATHFILE_VERSION)
   {
      arc.ReadObject(this);
      arc.Close();

      gi.printf("done.\n");
   }
   else
   {
      arc.Close();

      gi.printf("Expecting version %d path file.  Path file is version %d.", PATHFILE_VERSION, version);

      // Only replace the file if this event was called from our init function (as opposed to the user
      // calling us from the console) and the version number is older than our current version.
      if((ev->GetSource() == EV_FROM_CODE) && (version < PATHFILE_VERSION))
      {
         gi.printf("Replacing file.\n\n");

         // At this point, the nodes are still scheduled to find their neighbors, because we posted this event
         // before we the nodes were spawned.  Post the event with 0 delay so that it gets processed after all 
         // the nodes find their neighbors.
         PostEvent(EV_AI_SavePaths, 0);
      }
      else
      {
         // otherwise, just let them know that the path file needs to be replaced.
         gi.printf("Type 'ai_savepaths' at the console to replace the current path file.\n");
      }

      // Print out something fairly obvious
      gi.dprintf("***********************************\n"
                 "***********************************\n"
                 "\n"
                 "Creating paths...\n"
                 "\n"
                 "***********************************\n"
                 "***********************************\n");
   }
}

EXPORT_FROM_DLL void PathSearch::SavePaths(void)
{
   str filename;
   Event *ev;

   if(loadingarchive)
   {
      // force it to zero since we probably had an error
      gi.cvar_set("ai_createnodes", "0");
   }

   if(!loadingarchive && ai_createnodes && ai_createnodes->value)
   {
      filename = gi.GameDir();
      filename += "/maps/";
      filename += level.mapname;
      filename += ".pth";

      gi.dprintf("Saving path nodes to '%s'\n", filename.c_str());

      ev = new Event(EV_AI_SaveNodes);
      ev->AddString(filename);
      ProcessEvent(ev);
   }
}

EXPORT_FROM_DLL void PathSearch::SavePathsEvent(Event *ev)
{
   str temp;

   temp = ai_createnodes->string;
   gi.cvar_set("ai_createnodes", "1");

   SavePaths();

   gi.cvar_set("ai_createnodes", temp.c_str());
}

EXPORT_FROM_DLL void PathSearch::Init(const char *mapname)
{
   int x;
   int y;
   str filename;
   Event *ev;

   ai_createnodes  = gi.cvar("ai_createnodes", "0", 0);
   ai_showpath     = gi.cvar("ai_showpath", "0", 0);
   ai_debugpath    = gi.cvar("ai_debugpath", "0", 0);
   ai_debuginfo    = gi.cvar("ai_debuginfo", "0", 0);
   ai_showroutes   = gi.cvar("ai_showroutes", "0", 0);
   ai_shownodenums = gi.cvar("ai_shownodenums", "0", 0);
   ai_timepaths    = gi.cvar("ai_timepaths", "0", 0);

   numNodes = 0;
   NodeList = NULL;
   loadingarchive = false;

   // Get rid of the nodes that were spawned by the map
   AI_ResetNodes();

   // Init the grid
   for(x = 0; x < PATHMAP_GRIDSIZE; x++)
   {
      for(y = 0; y < PATHMAP_GRIDSIZE; y++)
      {
         PathMap[x][y].Init();
      }
   }

   if(mapname)
   {
      filename = "maps/";
      filename += mapname;
      filename += ".pth";
      if(gi.LoadFile(filename.c_str(), NULL, 0) != -1)
      {
         ev = new Event(EV_AI_LoadNodes);
         ev->AddString(filename);

         // This can't happen until the world is spawned
         PostEvent(ev, 0);
      }
      else
      {
         // Print out something fairly obvious
         gi.dprintf("***********************************\n"
                    "***********************************\n"
                    "\n"
                    "No paths found.  Creating paths...\n"
                    "\n"
                    "***********************************\n"
                    "***********************************\n");
      }
   }
}

// EOF

