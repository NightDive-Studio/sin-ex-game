//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/bspline.cpp                      $
// $Revision:: 14                                                             $
//   $Author:: Markd                                                          $
//     $Date:: 8/07/99 1:53p                                                  $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Uniform non-rational bspline class.
// 

#include "g_local.h"
#include "bspline.h"

void BSpline::Set(Vector *control_points_, int num_control_points_, splinetype_t type)
{
   int i;

   SetType(type);

   has_orientation = false;

   if(control_points)
   {
      delete[] control_points;
      control_points = NULL;
   }

   num_control_points = num_control_points_;
   if(num_control_points)
   {
      control_points = new BSplineControlPoint[num_control_points];
      assert(control_points);

      for(i = 0; i < num_control_points; i++)
      {
         control_points[i].Set(control_points_[i]);
      }
   }
}

void BSpline::Set(Vector *control_points_, Vector *control_orients_, float  *control_speeds_, int num_control_points_, splinetype_t type)
{
   int i;

   SetType(type);

   has_orientation = true;

   if(control_points)
   {
      delete[] control_points;
      control_points = NULL;
   }

   num_control_points = num_control_points_;
   if(num_control_points)
   {
      control_points = new BSplineControlPoint[num_control_points];
      assert(control_points);

      for(i = 0; i < num_control_points; i++)
      {
         control_points[i].Set(control_points_[i], control_orients_[i], control_speeds_[i]);
      }
   }
}

void BSpline::Clear(void)
{
   if(control_points)
   {
      delete[] control_points;
      control_points = NULL;
   }
   num_control_points = 0;
   has_orientation = false;
}

inline float BSpline::EvalNormal(float u, Vector& pos, Vector& orient)
{
   int		segment_id;
   float		B[4];
   float		tmp;
   float		u_2;
   float		u_3;
   Vector   ang;
   float    roll;
   float    speed;

   segment_id = (int)u;
   if(segment_id < 0)
   {
      segment_id = 0;
   }
   if(segment_id > num_control_points - 4)
   {
      segment_id = num_control_points - 4;
   }
   u -= (float)segment_id;

   u_2 = u * u;
   u_3 = u * u_2;

   tmp = 1 - u;
   B[0] = (tmp * tmp * tmp) * (1.0f / 6.0f);
   B[1] = (3.0f * u_3 - 6.0f * u_2 + 4.0f) * (1.0f / 6.0f);
   B[2] = (-3.0f * u_3 + 3.0f * u_2 + 3.0f * u + 1) * (1.0f / 6.0f);
   B[3] = u_3 * (1.0f / 6.0f);

   pos =
      *control_points[0 + segment_id].GetPosition() * B[0] +
      *control_points[1 + segment_id].GetPosition() * B[1] +
      *control_points[2 + segment_id].GetPosition() * B[2] +
      *control_points[3 + segment_id].GetPosition() * B[3];

   ang =
      *control_points[0 + segment_id].GetOrientation() * B[0] +
      *control_points[1 + segment_id].GetOrientation() * B[1] +
      *control_points[2 + segment_id].GetOrientation() * B[2] +
      *control_points[3 + segment_id].GetOrientation() * B[3];

   roll =
      *control_points[0 + segment_id].GetRoll() * B[0] +
      *control_points[1 + segment_id].GetRoll() * B[1] +
      *control_points[2 + segment_id].GetRoll() * B[2] +
      *control_points[3 + segment_id].GetRoll() * B[3];

   speed =
      *control_points[0 + segment_id].GetSpeed() * B[0] +
      *control_points[1 + segment_id].GetSpeed() * B[1] +
      *control_points[2 + segment_id].GetSpeed() * B[2] +
      *control_points[3 + segment_id].GetSpeed() * B[3];

   orient = ang.toAngles();
   orient[ROLL] = roll;

   return speed;
}

inline float BSpline::EvalLoop(float t, Vector& pos, Vector& orient)
{
   Vector	retval;
   Vector   ang;
   float    speed;
   float    roll;
   int		segment_id;
   int      next_id;
   float		B[4];
   float		tmp;
   float		u;
   float		u_2;
   float		u_3;
   int		i;
   int		j;

   segment_id = (int)floor(t);
   u = t - floor(t);

   segment_id %= num_control_points;
   if(segment_id < 0)
   {
      segment_id += num_control_points;
   }

   u_2 = u * u;
   u_3 = u * u_2;

   tmp = 1 - u;
   B[0] = (tmp * tmp * tmp) * (1.0f / 6.0f);
   B[1] = (3.0f * u_3 - 6.0f * u_2 + 4.0f) * (1.0f / 6.0f);
   B[2] = (-3.0f * u_3 + 3.0f * u_2 + 3.0f * u + 1) * (1.0f / 6.0f);
   B[3] = u_3 * (1.0f / 6.0f);

   speed = 0;
   roll = 0;

   for(i = 0, j = segment_id; i < 4; i++, j++)
   {
      if(j >= num_control_points)
      {
         j -= (num_control_points - loop_control_point);
      }

      retval += *control_points[j].GetPosition() * B[i];
      ang += *control_points[j].GetOrientation() * B[i];
      speed += *control_points[j].GetSpeed() * B[i];
      roll += *control_points[j].GetRoll() * B[i];
   }

   pos = retval;

   next_id = segment_id + 1;
   if(next_id >= num_control_points)
   {
      next_id -= (num_control_points - loop_control_point);
   }
   orient = ang.toAngles();
   orient[ROLL] = roll;

   return speed;
}

inline float BSpline::EvalClamp(float t, Vector& pos, Vector& orient)
{
   Vector	retval;
   Vector   ang;
   int		segment_id;
   int		next_id;
   float		B[4];
   float		tmp;
   float		u;
   float		u_2;
   float		u_3;
   int		i;
   int		j;
   float    speed;
   float    roll;

   segment_id = (int)floor(t);
   u = t - floor(t);

   u_2 = u * u;
   u_3 = u * u_2;

   tmp = 1 - u;
   B[0] = (tmp * tmp * tmp) * (1.0f / 6.0f);
   B[1] = (3.0f * u_3 - 6.0f * u_2 + 4.0f) * (1.0f / 6.0f);
   B[2] = (-3.0f * u_3 + 3.0f * u_2 + 3.0f * u + 1) * (1.0f / 6.0f);
   B[3] = u_3 * (1.0f / 6.0f);

   speed = 0;
   roll = 0;
   for(i = 0; i < 4; i++, segment_id++)
   {
      j = segment_id;
      if(j < 0)
      {
         j = 0;
      }
      else if(j >= num_control_points)
      {
         j = num_control_points - 1;
      }

      retval += *control_points[j].GetPosition() * B[i];
      ang += *control_points[j].GetOrientation() * B[i];
      speed += *control_points[j].GetSpeed() * B[i];
      roll += *control_points[j].GetRoll() * B[i];
   }

   pos = retval;

   next_id = segment_id + 1;
   if(segment_id < 0)
   {
      segment_id = 0;
   }
   if(segment_id >= num_control_points)
   {
      segment_id = num_control_points - 1;
   }
   if(next_id < 0)
   {
      next_id = 0;
   }
   if(next_id >= num_control_points)
   {
      next_id = num_control_points - 1;
   }
   orient = ang.toAngles();
   orient[ROLL] = roll;

   return speed;
}

Vector BSpline::Eval(float u)
{
   Vector pos;
   Vector orient;

   switch(curvetype)
   {
   default:
   case SPLINE_NORMAL:
      EvalNormal(u, pos, orient);
      break;

   case SPLINE_CLAMP:
      EvalClamp(u, pos, orient);
      break;

   case SPLINE_LOOP:
      EvalLoop(u, pos, orient);
      break;
   }
   return pos;
}

float BSpline::Eval(float u, Vector &pos, Vector &orient)
{
   switch(curvetype)
   {
   default:
   case SPLINE_NORMAL:
      return EvalNormal(u, pos, orient);
      break;

   case SPLINE_CLAMP:
      return EvalClamp(u, pos, orient);
      break;

   case SPLINE_LOOP:
      return EvalLoop(u, pos, orient);
      break;
   }
}

void BSpline::DrawControlSegments(void)
{
   int i;

   G_BeginLine();
   for(i = 0; i < num_control_points; i++)
   {
      G_Vertex(*control_points[i].GetPosition());
   }
   G_EndLine();
}

void BSpline::DrawCurve(int num_subdivisions)
{
   float u;
   float du;

   du = 1.0f / (float)num_subdivisions;

   G_BeginLine();
   for(u = 0.0f; u <= (float)num_control_points; u += du)
   {
      G_Vertex((Vector)Eval(u));
   }
   G_EndLine();
}

void BSpline::DrawCurve(Vector offset, int num_subdivisions)
{
   float u;
   float du;

   du = 1.0f / (float)num_subdivisions;

   G_BeginLine();
   for(u = 0.0f; u <= (float)num_control_points; u += du)
   {
      G_Vertex(offset + (Vector)Eval(u));
   }
   G_EndLine();
}

void BSpline::AppendControlPoint(const Vector& new_control_point)
{
   BSplineControlPoint *old_control_points;
   int i;

   old_control_points = control_points;
   num_control_points++;

   control_points = new BSplineControlPoint[num_control_points];
   assert(control_points);

   if(old_control_points)
   {
      for(i = 0; i < num_control_points - 1; i++)
      {
         control_points[i] = old_control_points[i];
      }
      delete[] old_control_points;
   }

   control_points[num_control_points - 1].Set(new_control_point);
}

void BSpline::AppendControlPoint(const Vector &new_control_point, const float &speed)
{
   BSplineControlPoint *old_control_points;
   int i;

   old_control_points = control_points;
   num_control_points++;

   control_points = new BSplineControlPoint[num_control_points];
   assert(control_points);

   if(old_control_points)
   {
      for(i = 0; i < num_control_points - 1; i++)
      {
         control_points[i] = old_control_points[i];
      }
      delete[] old_control_points;
   }

   control_points[num_control_points - 1].Set(new_control_point, speed);
}

void BSpline::AppendControlPoint(const Vector& new_control_point, const Vector& new_control_orient, const float& new_control_speed)
{
   BSplineControlPoint *old_control_points;
   int i;

   has_orientation = true;

   old_control_points = control_points;
   num_control_points++;

   control_points = new BSplineControlPoint[num_control_points];
   assert(control_points);

   if(old_control_points)
   {
      for(i = 0; i < num_control_points - 1; i++)
      {
         control_points[i] = old_control_points[i];
      }
      delete[] old_control_points;
   }

   control_points[num_control_points - 1].Set(new_control_point, new_control_orient, new_control_speed);
}

void BSpline::SetLoopPoint(const Vector& pos)
{
   int i;

   for(i = 0; i < num_control_points; i++)
   {
      if(pos == *control_points[i].GetPosition())
      {
         loop_control_point = i;
         break;
      }
   }
}

int BSpline::PickControlPoint(const Vector& window_point, float pick_size)
{
   int	i;
   float closest_dist_2;
   int	closest_index;
   float dist_2;
   Vector delta;

   closest_index = -1;
   closest_dist_2 = 1000000.0f;
   for(i = 0; i < num_control_points; i++)
   {
      delta = window_point - *control_points[i].GetPosition();
      dist_2 = delta * delta;
      if(dist_2 < closest_dist_2)
      {
         closest_dist_2 = dist_2;
         closest_index = i;
      }
   }

   if(pick_size * pick_size >= closest_dist_2)
   {
      return closest_index;
   }
   else
   {
      return -1;
   }
}

CLASS_DECLARATION(Entity, SplinePath, "info_splinepath");

Event EV_SplinePath_Create("SplinePath_create");
Event EV_SplinePath_Loop("loop", EV_CONSOLE);
Event EV_SplinePath_Speed("speed");

ResponseDef SplinePath::Responses[] =
{
   {&EV_SplinePath_Create, (Response)&SplinePath::CreatePath},
   {&EV_SplinePath_Loop, (Response)&SplinePath::SetLoop},
   {&EV_SplinePath_Speed, (Response)&SplinePath::SetSpeed},
   {NULL, NULL}
};

SplinePath::SplinePath() : Entity()
{
   owner = this;
   next = NULL;
   loop = NULL;
   doWatch = false;
   watchEnt = "";

   loop_name = G_GetStringArg("loop");
   angles = G_GetVectorArg("angles");
   speed = G_GetFloatArg("speed", 1);
   setMoveType(MOVETYPE_NONE);
   setSolidType(SOLID_NOT);
   hideModel();

   if(!LoadingSavegame)
   {
      PostEvent(EV_SplinePath_Create, 0);
   }
}

SplinePath::~SplinePath()
{
   // disconnect from the chain
   if(owner != this)
   {
      owner->SetNext(next);
      owner = this;
   }
   else if(next)
   {
      next->SetPrev(NULL);
      next = NULL;
   }

   assert(owner == this);
   //assert( next == NULL );
}


void SplinePath::CreatePath(Event *ev)
{
   const char  *target;
   int         num;

   // Make the path from the targetlist.
   target = Target();
   if(target[0])
   {
      if((num = G_FindTarget(0, target)))
      {
         next = (SplinePath *)G_GetEntity(num);
         next->owner = this;
      }
      else
      {
         gi.error("SplinePath::CreatePath: target %s not found\n", target);
      }
   }
   if(loop_name.length())
   {
      if((num = G_FindTarget(0, loop_name.c_str())))
      {
         loop = (SplinePath *)G_GetEntity(num);
      }
   }
}

SplinePath *SplinePath::GetNext(void)
{
   return next;
}

SplinePath *SplinePath::GetLoop(void)
{
   return loop;
}

void SplinePath::SetLoop(Event *ev)
{
   loop_name = ev->GetString(1);
}

void SplinePath::SetSpeed(Event *ev)
{
   speed = ev->GetFloat(1);
}

SplinePath *SplinePath::GetPrev(void)
{
   if(owner == this)
   {
      return NULL;
   }

   return owner;
}

void SplinePath::SetNext(SplinePath *node)
{
   if(next)
   {
      // remove ourselves from the chain
      next->owner = next;
   }

   next = node;
   if(next)
   {
      // disconnect next from it's previous node
      if(next->owner != next)
      {
         next->owner->next = NULL;
      }
      next->owner = this;
   }
}

void SplinePath::SetPrev(SplinePath *node)
{
   if(owner != this)
   {
      owner->next = NULL;
   }

   if(node && (node != this))
   {
      // safely remove the node from its chain
      if(node->next)
      {
         node->next->owner = node->next;
      }
      node->next = this;
      owner = node;
   }
   else
   {
      owner = this;
   }
}

void SplinePath::SetWatch(const char *name)
{
   doWatch = true;
   watchEnt = name;
}

void SplinePath::NoWatch(void)
{
   doWatch = true;
   watchEnt = "";
}

Entity *SplinePath::GetWatch(void)
{
   const char *name;
   int t;

   name = watchEnt.c_str();
   if(name[0] == '$')
   {
      t = G_FindTarget(0, &name[1]);
      if(!t)
      {
         warning("GetWatch", "Entity with targetname of '%s' not found", &name[1]);
         return NULL;
      }
   }
   else
   {
      if(name[0] != '*')
      {
         warning("GetWatch", "Expecting a '*'-prefixed entity number but found '%s'.", name);
         return NULL;
      }

      if(!IsNumeric(&name[1]))
      {
         warning("GetWatch", "Expecting a numeric value but found '%s'.", &name[1]);
         return NULL;
      }
      else
      {
         t = atoi(&name[1]);
      }
   }

   if((t < 0) || (t > game.maxentities))
   {
      warning("GetWatch", "%d out of valid range for entity.", t);
      return NULL;
   }

   return G_GetEntity(t);
}

// EOF

