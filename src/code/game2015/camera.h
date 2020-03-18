//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/camera.h                         $
// $Revision:: 27                                                             $
//   $Author:: Markd                                                          $
//     $Date:: 5/19/99 11:30a                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Camera.  Duh.
// 

#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "g_local.h"
#include "entity.h"
#include "bspline.h"

#define ORBIT     1
#define START_ON  2
#define PANNING   4

extern Event EV_Camera_FollowingPath;
extern Event EV_Camera_StartMoving;
extern Event EV_Camera_Pause;
extern Event EV_Camera_Continue;
extern Event EV_Camera_StopMoving;
extern Event EV_Camera_SetSpeed;
extern Event EV_Camera_SetDistance;
extern Event EV_Camera_SetHeight;
extern Event EV_Camera_SetYaw;
extern Event EV_Camera_FixedYaw;
extern Event EV_Camera_RelativeYaw;
extern Event EV_Camera_SetFOV;
extern Event EV_Camera_Orbit;
extern Event EV_Camera_Follow;
extern Event EV_Camera_Watch;
extern Event EV_Camera_LookAt;
extern Event EV_Camera_TurnTo;
extern Event EV_Camera_MoveToEntity;
extern Event EV_Camera_MoveToPos;
extern Event EV_Camera_NoWatch;
extern Event EV_Camera_IgnoreAngles;
extern Event EV_Camera_UseAngles;
extern Event EV_Camera_SplineAngles;
extern Event EV_Camera_NormalAngles;
extern Event EV_Camera_FixedPosition;
extern Event EV_Camera_NoFixedPosition;
extern Event EV_Camera_JumpTime;
extern Event EV_Camera_JumpCut;
extern Event EV_Camera_Pan;
extern Event EV_Camera_StopPan;
extern Event EV_Camera_PanSpeed;
extern Event EV_Camera_PanMax;
extern Event EV_Camera_SetPanAngles;


class EXPORT_FROM_DLL CameraMoveState
{
public:
   Vector            pos;

   BSpline           cameraPath;
   float             cameraTime = 0.0f;

   EntityPtr         followEnt  = nullptr;
   EntityPtr         orbitEnt   = nullptr;

   qboolean          followingpath  = false;
   float             speed          = 0.0f;
   qboolean          fixed_position = false;
   float             fov            = 90.0f;
   float             height         = 0.0f;
   float             follow_dist    = 0.0f;
   int               follow_mask    = 0;

   CameraMoveState() = default;
   CameraMoveState(const CameraMoveState &other) = default;

   CameraMoveState  &operator = (const CameraMoveState &newstate) = default;
   virtual void      Archive(Archiver &arc);
   virtual void      Unarchive(Archiver &arc);
};

inline EXPORT_FROM_DLL void CameraMoveState::Archive(Archiver &arc)
{
   arc.WriteVector(pos);

   cameraPath.Archive(arc);

   arc.WriteFloat(cameraTime);

   arc.WriteSafePointer(followEnt);
   arc.WriteSafePointer(orbitEnt);

   arc.WriteBoolean(followingpath);
   arc.WriteFloat(speed);
   arc.WriteBoolean(fixed_position);

   arc.WriteFloat(fov);
   arc.WriteFloat(height);
   arc.WriteFloat(follow_dist);
   arc.WriteInteger(follow_mask);
}

inline EXPORT_FROM_DLL void CameraMoveState::Unarchive(Archiver &arc)
{
   arc.ReadVector(&pos);

   cameraPath.Unarchive(arc);

   arc.ReadFloat(&cameraTime);

   arc.ReadSafePointer(&followEnt);
   arc.ReadSafePointer(&orbitEnt);

   arc.ReadBoolean(&followingpath);
   arc.ReadFloat(&speed);
   arc.ReadBoolean(&fixed_position);

   arc.ReadFloat(&fov);
   arc.ReadFloat(&height);
   arc.ReadFloat(&follow_dist);
   arc.ReadInteger(&follow_mask);
}

class EXPORT_FROM_DLL CameraWatchState
{
public:
   Vector            dir;

   EntityPtr         watchEnt     = nullptr;

   qboolean          ignoreangles = false;
   qboolean          splineangles = false;
   qboolean          panning      = false;

   float             pan_offset   = 0.0f;
   float             pan_dir      = 0.0f;
   float             pan_max      = 0.0f;
   float             pan_speed    = 0.0f;
   Vector            pan_angles;

   float             yaw          = 0.0f;
   qboolean          fixedyaw     = false;

   CameraWatchState() = default;
   CameraWatchState(const CameraWatchState &other) = default;
   CameraWatchState &operator = (const CameraWatchState &other) = default;

   virtual void Archive(Archiver &arc);
   virtual void Unarchive(Archiver &arc);
};

inline EXPORT_FROM_DLL void CameraWatchState::Archive(Archiver &arc)
{
   arc.WriteVector(dir);
   arc.WriteSafePointer(watchEnt);

   arc.WriteBoolean(ignoreangles);
   arc.WriteBoolean(splineangles);
   arc.WriteBoolean(panning);

   arc.WriteFloat(pan_offset);
   arc.WriteFloat(pan_dir);
   arc.WriteFloat(pan_max);
   arc.WriteFloat(pan_speed);
   arc.WriteVector(pan_angles);

   arc.WriteFloat(yaw);
   arc.WriteBoolean(fixedyaw);
}

inline EXPORT_FROM_DLL void CameraWatchState::Unarchive(Archiver &arc)
{
   arc.ReadVector(&dir);
   arc.ReadSafePointer(&watchEnt);

   arc.ReadBoolean(&ignoreangles);
   arc.ReadBoolean(&splineangles);
   arc.ReadBoolean(&panning);

   arc.ReadFloat(&pan_offset);
   arc.ReadFloat(&pan_dir);
   arc.ReadFloat(&pan_max);
   arc.ReadFloat(&pan_speed);
   arc.ReadVector(&pan_angles);

   arc.ReadFloat(&yaw);
   arc.ReadBoolean(&fixedyaw);
}

class EXPORT_FROM_DLL CameraState
{
public:
   CameraMoveState   move;
   CameraWatchState  watch;

   CameraState() = default;
   CameraState(const CameraState &other) = default;
   CameraState &operator = (const CameraState &other) = default;

   virtual void Archive(Archiver &arc);
   virtual void Unarchive(Archiver &arc);
};

inline EXPORT_FROM_DLL void CameraState::Archive(Archiver &arc)
{
   move.Archive(arc);
   watch.Archive(arc);
}

inline EXPORT_FROM_DLL void CameraState::Unarchive(Archiver &arc)
{
   move.Unarchive(arc);
   watch.Unarchive(arc);
}

class EXPORT_FROM_DLL Camera : public Entity
{
private:
   float default_fov;
   float default_yaw;
   float default_follow_dist;
   float default_height;
   float default_speed;
   float default_pan_max;
   float default_pan_speed;

   Vector default_angles;

   Camera(const Camera &)              = delete;
   Camera &operator = (const Camera &) = delete;

protected:
   CameraState       currentstate;
   CameraState       newstate;

   float             watchTime;
   float             followTime;
   float             jumpTime;

   EntityPtr         targetEnt;
   EntityPtr         targetWatchEnt;

   str               nextCamera;
   str               overlay;
   str               thread;

   qboolean          showcamera;

   Event             moveevent;

   void              FollowingPath(Event *ev);
   void              CreateOribit(Vector pos, float radius);
   void              CreatePath(SplinePath *path, splinetype_t type);
   void              InitializeMoveState(CameraMoveState &movestate);
   void              InitializeWatchState(CameraWatchState &watchstate);
   void              InitializeState(CameraState &state);

public:
   CLASS_PROTOTYPE(Camera);

   float             fov;

   Camera();
   void              Stop();
   void              FollowPath(SplinePath *path, qboolean loop, Entity *watch);
   void              Orbit(Entity *ent, float dist, Entity *watch);
   void              FollowEntity(Entity *ent, float dist, int mask, Entity *watch = NULL);
   void              StartMoving(Event *ev);
   void              StopMoving(Event *ev);
   void              Pause(Event *ev);
   void              Continue(Event *ev);
   void              SetSpeed(Event *ev);
   void              SetDistance(Event *ev);
   void              SetHeight(Event *ev);
   void              SetYaw(Event *ev);
   void              FixedYaw(Event *ev);
   void              RelativeYaw(Event *ev);
   void              SetFOV(Event *ev);
   void              OrbitEvent(Event *ev);
   void              FollowEvent(Event *ev);
   void              WatchEvent(Event *ev);
   void              NoWatchEvent(Event *ev);
   void              LookAt(Event *ev);
   void              MoveToEntity(Event *ev);
   void              MoveToPos(Event *ev);
   void              IgnoreAngles(Event *ev);
   void              UseAngles(Event *ev);
   void              SplineAngles(Event *ev);
   void              NormalAngles(Event *ev);
   void              FixedPosition(Event *ev);
   void              NoFixedPosition(Event *ev);
   void              JumpCut(Event *ev);
   void              JumpTime(Event *ev);
   void              TurnTo(Event *ev);
   void              EvaluatePosition(CameraState &state);
   void              PanEvent(Event *ev);
   void              StopPanEvent(Event *ev);
   void              PanSpeedEvent(Event *ev);
   void              PanMaxEvent(Event *ev);
   void              SetPanAngles(Event *ev);
   void              SetNextCamera(Event *ev);
   void              SetOverlay(Event *ev);
   void              SetThread(Event *ev);

   str               &NextCamera();
   str               &Thread();
   str               &Overlay();
   void              SetCurrentDistance(float dist);

   void              Reset(Vector org, Vector ang);

   virtual void      Archive(Archiver &arc)   override;
   virtual void      Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void Camera::Archive(Archiver &arc)
{
   Entity::Archive(arc);

   arc.WriteFloat(default_fov);
   arc.WriteFloat(default_yaw);
   arc.WriteFloat(default_follow_dist);
   arc.WriteFloat(default_height);
   arc.WriteFloat(default_speed);
   arc.WriteFloat(default_pan_max);
   arc.WriteFloat(default_pan_speed);
   arc.WriteVector(default_angles);

   // currentstate
   currentstate.Archive(arc);
   // newstate
   newstate.Archive(arc);

   arc.WriteFloat(watchTime);
   arc.WriteFloat(followTime);
   arc.WriteFloat(jumpTime);

   arc.WriteSafePointer(targetEnt);
   arc.WriteSafePointer(targetWatchEnt);

   arc.WriteString(nextCamera);
   arc.WriteString(overlay);
   arc.WriteString(thread);

   arc.WriteBoolean(showcamera);
   arc.WriteEvent(moveevent);
   arc.WriteFloat(fov);
}

inline EXPORT_FROM_DLL void Camera::Unarchive(Archiver &arc)
{
   Entity::Unarchive(arc);

   arc.ReadFloat(&default_fov);
   arc.ReadFloat(&default_yaw);
   arc.ReadFloat(&default_follow_dist);
   arc.ReadFloat(&default_height);
   arc.ReadFloat(&default_speed);
   arc.ReadFloat(&default_pan_max);
   arc.ReadFloat(&default_pan_speed);
   arc.ReadVector(&default_angles);

   // currentstate
   currentstate.Unarchive(arc);
   // newstate
   newstate.Unarchive(arc);

   arc.ReadFloat(&watchTime);
   arc.ReadFloat(&followTime);
   arc.ReadFloat(&jumpTime);

   arc.ReadSafePointer(&targetEnt);
   arc.ReadSafePointer(&targetWatchEnt);

   arc.ReadString(&nextCamera);
   arc.ReadString(&overlay);
   arc.ReadString(&thread);

   arc.ReadBoolean(&showcamera);
   arc.ReadEvent(&moveevent);
   arc.ReadFloat(&fov);
}

void SetCamera(Entity *ent);

#ifdef EXPORT_TEMPLATE
template class EXPORT_FROM_DLL SafePtr<Camera>;
#endif
typedef SafePtr<Camera> CameraPtr;

class EXPORT_FROM_DLL SecurityCamera : public Camera
{
public:
   CLASS_PROTOTYPE(SecurityCamera);

   SecurityCamera();
};


class EXPORT_FROM_DLL CameraManager : public Listener
{
protected:
   SplinePathPtr     path    = nullptr;
   SplinePathPtr     current = nullptr;
   float             speed   = 1.0f;
   int               watch   = 0;
   CameraPtr         cam     = nullptr;

   void              NewPath(Event *ev);
   void              SetPath(Event *ev);
   void              SetTargetName(Event *ev);
   void              SetTarget(Event *ev);
   void              AddPoint(Event *ev);
   void              ReplacePoint(Event *ev);
   void              DeletePoint(Event *ev);
   void              MovePlayer(Event *ev);
   void              NextPoint(Event *ev);
   void              PreviousPoint(Event *ev);
   void              ShowPath(Event *ev);
   void              HidePath(Event *ev);
   void              StopPlayback(Event *ev);
   void              PlayPath(Event *ev);
   void              LoopPath(Event *ev);
   void              Watch(Event *ev);
   void              NoWatch(Event *ev);
   void              Speed(Event *ev);
   void              Save(Event *ev);
   void              Load(Event *ev);
   void              SaveMap(Event *ev);

public:
   CLASS_PROTOTYPE(CameraManager);

   CameraManager() = default;
   void              SetPath(SplinePath *node);
   virtual void      Archive(Archiver &arc)   override;
   virtual void      Unarchive(Archiver &arc) override;
};

inline void CameraManager::Archive(Archiver &arc)
{
   Listener::Archive(arc);

   arc.WriteSafePointer(path);
   arc.WriteSafePointer(current);
   arc.WriteFloat(speed);
   arc.WriteInteger(watch);
   arc.WriteSafePointer(cam);
}

inline void CameraManager::Unarchive(Archiver &arc)
{
   Listener::Unarchive(arc);

   arc.ReadSafePointer(&path);
   arc.ReadSafePointer(&current);
   arc.ReadFloat(&speed);
   arc.ReadInteger(&watch);
   arc.ReadSafePointer(&cam);
}

extern CameraManager CameraMan;

#endif /* camera.h */

// EOF

