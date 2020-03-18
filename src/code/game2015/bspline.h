//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/bspline.h                        $
// $Revision:: 20                                                             $
//   $Author:: Markd                                                          $
//     $Date:: 5/19/99 11:30a                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Uniform non-rational bspline class.
// 

#ifndef __BSPLINE_H__
#define __BSPLINE_H__

#include "g_local.h"
#include "vector.h"

typedef enum
{
   SPLINE_NORMAL,
   SPLINE_LOOP,
   SPLINE_CLAMP
} splinetype_t;

class EXPORT_FROM_DLL BSplineControlPoint
{
private:
   float          roll        = 0.0f;
   Vector         position;
   Vector         orientation;
   float          speed       = 1.0f;

public:
   BSplineControlPoint() = default;
   BSplineControlPoint(Vector pos, Vector orient, float speed);
   BSplineControlPoint(Vector pos);
   void           Clear(void);
   void           Set(Vector pos);
   void           Set(Vector pos, float speed);
   void           Set(Vector pos, Vector orient, float speed);
   void           Get(Vector& pos, Vector& orient, float& speed);
   void           Get(Vector& pos);
   Vector         *GetPosition(void);
   Vector         *GetOrientation(void);
   float          *GetRoll(void);
   float          *GetSpeed(void);
   void           operator = (BSplineControlPoint &point);
   virtual void   Archive(Archiver &arc);
   virtual void   Unarchive(Archiver &arc);
};

inline EXPORT_FROM_DLL void BSplineControlPoint::Archive(Archiver &arc)
{
   arc.WriteVector(position);
   arc.WriteVector(orientation);
   arc.WriteFloat(speed);
   arc.WriteFloat(roll);
}

inline EXPORT_FROM_DLL void BSplineControlPoint::Unarchive(Archiver &arc)
{
   arc.ReadVector(&position);
   arc.ReadVector(&orientation);
   arc.ReadFloat(&speed);
   arc.ReadFloat(&roll);
}

inline void BSplineControlPoint::operator = (BSplineControlPoint &point)
{
   position    = point.position;
   orientation = point.orientation;
   speed       = point.speed;
   roll        = point.roll;
}

inline BSplineControlPoint::BSplineControlPoint(Vector pos) 
   : roll(0), orientation()
{
   speed = 1;
   position = pos;
}

inline BSplineControlPoint::BSplineControlPoint(Vector pos, Vector orient, float speed)
{
   position = pos;
   orient[PITCH] = -orient[PITCH];
   orient.AngleVectors(&orientation, NULL, NULL);
   roll = orient[ROLL];
   if(roll > 180)
   {
      roll -= 360;
   }
   if(roll < -180)
   {
      roll += 360;
   }
   this->speed = speed;
}

inline void BSplineControlPoint::Clear(void)
{
   roll = 0;
   position = { 0, 0, 0 };
   vec_zero.AngleVectors(&orientation, NULL, NULL);
   speed = 1.0f;
}

inline void BSplineControlPoint::Set(Vector pos)
{
   speed = 1;
   position = pos;
}

inline void BSplineControlPoint::Set(Vector pos, float pointspeed)
{
   speed = pointspeed;
   position = pos;
}

inline void BSplineControlPoint::Set(Vector pos, Vector orient, float speed)
{
   position = pos;
   orient[PITCH] = -orient[PITCH];
   orient.AngleVectors(&orientation, NULL, NULL);
   roll = orient[ROLL];
   if(roll > 180)
   {
      roll -= 360;
   }
   if(roll < -180)
   {
      roll += 360;
   }
   this->speed = speed;
}

inline void BSplineControlPoint::Get(Vector &pos)
{
   pos = position;
}

inline Vector *BSplineControlPoint::GetPosition(void)
{
   return &position;
}

inline void BSplineControlPoint::Get(Vector &pos, Vector &orient, float &speed)
{
   pos    = position;
   orient = orientation;
   speed  = this->speed;
}

inline Vector *BSplineControlPoint::GetOrientation(void)
{
   return &orientation;
}

inline float *BSplineControlPoint::GetRoll(void)
{
   return &roll;
}

inline float *BSplineControlPoint::GetSpeed(void)
{
   return &speed;
}

class EXPORT_FROM_DLL BSpline
{
private:
   BSplineControlPoint  *control_points;
   int                  num_control_points;
   int                  loop_control_point;
   splinetype_t         curvetype;
   qboolean             has_orientation;

   float          EvalNormal(float u, Vector &pos, Vector& orient);
   float          EvalLoop(float u, Vector &pos, Vector& orient);
   float          EvalClamp(float u, Vector &pos, Vector& orient);

public:
   BSpline();
   BSpline(const BSpline &other);
   ~BSpline();
   BSpline(Vector *control_points_, int num_control_points_, splinetype_t type);
   BSpline(Vector *control_points_, Vector *control_orients_, float *control_speeds_, int num_control_points_, splinetype_t type);

   void           operator = (const BSpline &spline);
   void           SetType(splinetype_t type);
   int            GetType(void);
   void           Clear(void);
   void           Set(Vector *control_points_, int num_control_points_, splinetype_t type);
   void           Set(Vector *control_points_, Vector *control_orients_, float *control_speeds_, int num_control_points_, splinetype_t type);
   void           AppendControlPoint(const Vector &new_control_point);
   void           AppendControlPoint(const Vector &new_control_point, const float &speed);
   void           AppendControlPoint(const Vector &new_control_point, const Vector &new_control_orient, const float &speed);
   Vector         Eval(float u);
   float          Eval(float u, Vector& pos, Vector& orient);

   void           DrawControlSegments(void);
   void           DrawCurve(int num_subdivisions);
   void           DrawCurve(Vector offset, int num_subdivisions);

   void           SetLoopPoint(const Vector& pos);

   float          EndPoint(void);

   // return the index of the control point picked or -1 if none.
   int            PickControlPoint(const Vector& window_point, float pick_size);

   Vector         *GetControlPoint(int id);
   void           GetControlPoint(int id, Vector& pos, Vector& orient, float& speed);
   void           SetControlPoint(int id, const Vector& new_control_point);
   void           SetControlPoint(int id, const Vector& new_control_point, const Vector& new_control_orient, const float& speed);
   virtual void   Archive(Archiver &arc);
   virtual void   Unarchive(Archiver &arc);
};

inline BSpline::BSpline()
{
   has_orientation    = false;
   control_points     = NULL;
   num_control_points = 0;
   loop_control_point = 0;
   curvetype          = SPLINE_NORMAL;
}

inline BSpline::BSpline(const BSpline &other)
{
   num_control_points = other.num_control_points;
   loop_control_point = other.loop_control_point;
   curvetype          = other.curvetype;
   has_orientation    = other.has_orientation;

   if(num_control_points)
   {
      control_points = new BSplineControlPoint[num_control_points];
      assert(control_points);
      for(int i = 0; i < num_control_points; i++)
         control_points[i] = other.control_points[i];
   }
   else
      control_points = nullptr;
}

inline BSpline::~BSpline()
{
   if(control_points)
   {
      delete [] control_points;
      control_points = NULL;
   }
}

inline BSpline::BSpline(Vector *control_points_, int num_control_points_, splinetype_t type)
{
   has_orientation    = false;
   control_points     = NULL;
   num_control_points = 0;
   loop_control_point = 0;
   curvetype          = SPLINE_NORMAL;

   Set(control_points_, num_control_points_, type);
}

inline BSpline::BSpline(Vector *control_points_, Vector *control_orients_, float  *control_speeds_, 
                        int num_control_points_, splinetype_t type)
{
   has_orientation    = false;
   control_points     = NULL;
   num_control_points = 0;
   loop_control_point = 0;
   curvetype          = SPLINE_NORMAL;

   Set(control_points_, control_orients_, control_speeds_, num_control_points_, type);
}

inline void BSpline::operator = (const BSpline &spline)
{
   int i;

   if(this == &spline)
      return;

   Clear();
   num_control_points = spline.num_control_points;
   loop_control_point = spline.loop_control_point;
   curvetype          = spline.curvetype;
   has_orientation    = spline.has_orientation;

   control_points = new BSplineControlPoint[num_control_points];
   assert(control_points);
   for(i = 0; i < num_control_points; i++)
      control_points[i] = spline.control_points[i];
}

inline void BSpline::SetType(splinetype_t type)
{
   curvetype = type;
}

inline int BSpline::GetType(void)
{
   return curvetype;
}

inline float BSpline::EndPoint(void)
{
   return num_control_points;
}

inline Vector *BSpline::GetControlPoint(int id)
{
   assert(id >= 0);
   assert(id < num_control_points);
   if((id < 0) && (id >= num_control_points))
   {
      // probably wrong, but if we're in release mode we have no recourse
      id = 0;
   }

   return control_points[id].GetPosition();
}

inline void BSpline::GetControlPoint(int id, Vector &pos, Vector &orient, float &speed)
{
   assert(id >= 0);
   assert(id < num_control_points);
   if((id >= 0) && (id < num_control_points))
   {
      control_points[id].Get(pos, orient, speed);
   }
}

inline void BSpline::SetControlPoint(int id, const Vector &new_control_point)
{
   assert(id >= 0);
   assert(id < num_control_points);
   if((id >= 0) && (id < num_control_points))
   {
      control_points[id].Set(new_control_point);
   }
}

inline void BSpline::SetControlPoint(int id, const Vector &new_control_point, const Vector &new_control_orient, const float &speed)
{
   assert(id >= 0);
   assert(id < num_control_points);
   if((id >= 0) && (id < num_control_points))
   {
      control_points[id].Set(new_control_point, new_control_orient, speed);
   }
}

inline EXPORT_FROM_DLL void BSpline::Archive(Archiver &arc)
{
   int i;

   arc.WriteInteger(num_control_points);
   arc.WriteInteger(loop_control_point);
   arc.WriteInteger(curvetype);
   arc.WriteBoolean(has_orientation);
   for(i = 0; i < num_control_points; i++)
   {
      control_points[i].Archive(arc);
   }
}

inline EXPORT_FROM_DLL void BSpline::Unarchive(Archiver &arc)
{
   int i;

   arc.ReadInteger(&num_control_points);
   arc.ReadInteger(&loop_control_point);
   arc.ReadInteger(&i);
   curvetype = (splinetype_t)i;
   arc.ReadBoolean(&has_orientation);
   control_points = new BSplineControlPoint[num_control_points];
   for(i = 0; i < num_control_points; i++)
   {
      control_points[i].Unarchive(arc);
   }
}

class SplinePath;
#ifdef EXPORT_TEMPLATE
template class EXPORT_FROM_DLL SafePtr<SplinePath>;
#endif
typedef SafePtr<SplinePath> SplinePathPtr;

extern Event EV_SplinePath_Create;
extern Event EV_SplinePath_Loop;
extern Event EV_SplinePath_Speed;

class EXPORT_FROM_DLL SplinePath : public Entity
{
protected:
   SplinePathPtr     owner;
   SplinePathPtr     next;
   SplinePathPtr     loop;
   str               loop_name;

   void              CreatePath(Event *ev);
   void              SetLoop(Event *ev);
   void              SetSpeed(Event *ev);

public:
   float             speed;
   qboolean          doWatch;
   str               watchEnt;

   CLASS_PROTOTYPE(SplinePath);

   SplinePath();
   ~SplinePath();
   SplinePath        *GetNext();
   SplinePath        *GetPrev();
   SplinePath        *GetLoop();
   void              SetWatch(const char *name);
   void              NoWatch();
   Entity            *GetWatch();
   void              SetNext(SplinePath *node);
   void              SetPrev(SplinePath *node);
   virtual void      Archive(Archiver &arc)   override;
   virtual void      Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void SplinePath::Archive(Archiver &arc)
{
   Entity::Archive(arc);

   arc.WriteSafePointer(owner);
   arc.WriteSafePointer(next);
   arc.WriteSafePointer(loop);
   arc.WriteString(loop_name);
   arc.WriteFloat(speed);
   arc.WriteBoolean(doWatch);
   arc.WriteString(watchEnt);
}

inline EXPORT_FROM_DLL void SplinePath::Unarchive(Archiver &arc)
{
   Entity::Unarchive(arc);

   arc.ReadSafePointer(&owner);
   arc.ReadSafePointer(&next);
   arc.ReadSafePointer(&loop);
   arc.ReadString(&loop_name);
   arc.ReadFloat(&speed);
   arc.ReadBoolean(&doWatch);
   arc.ReadString(&watchEnt);

   CancelEventsOfType(EV_SplinePath_Create);
}

#endif /* __BSPLINE_H__ */

// EOF

