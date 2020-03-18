//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/camera.cpp                       $
// $Revision:: 52                                                             $
//   $Author:: Markd                                                          $
//     $Date:: 11/02/99 12:09p                                                $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Camera.  Duh.
// 

#include "g_local.h"
#include "entity.h"
#include "trigger.h"
#include "camera.h"
#include "bspline.h"
#include "player.h"
#include "camera.h"

#define CAMERA_PATHFILE_VERSION 1

CameraManager CameraMan;

cvar_t *sv_showcameras;

Event EV_Camera_FollowingPath( "followingpath" );
Event EV_Camera_StartMoving( "start" );
Event EV_Camera_Pause( "pause" );
Event EV_Camera_Continue( "continue" );
Event EV_Camera_StopMoving( "stop" );
Event EV_Camera_SetSpeed( "speed" );
Event EV_Camera_SetDistance( "distance" );
Event EV_Camera_SetHeight( "height" );
Event EV_Camera_SetYaw( "yaw" );
Event EV_Camera_FixedYaw( "fixedyaw" );
Event EV_Camera_RelativeYaw( "relativeyaw" );
Event EV_Camera_SetFOV( "fov" );
Event EV_Camera_Orbit( "orbit" );
Event EV_Camera_Follow( "follow" );
Event EV_Camera_Watch( "watch" );
Event EV_Camera_LookAt( "lookat" );
Event EV_Camera_TurnTo( "turnto" );
Event EV_Camera_MoveToEntity( "moveto" );
Event EV_Camera_MoveToPos( "movetopos" );
Event EV_Camera_NoWatch( "nowatch" );
Event EV_Camera_IgnoreAngles( "ignoreangles" );
Event EV_Camera_UseAngles( "useangles" );
Event EV_Camera_SplineAngles( "splineangles" );
Event EV_Camera_NormalAngles( "normalangles" );
Event EV_Camera_FixedPosition( "fixedposition" );
Event EV_Camera_NoFixedPosition( "nofixedposition" );
Event EV_Camera_JumpTime( "jumptime" );
Event EV_Camera_JumpCut( "jumpcut" );
Event EV_Camera_Pan( "pan" );
Event EV_Camera_StopPan( "stoppan" );
Event EV_Camera_PanSpeed( "panspeed" );
Event EV_Camera_PanMax( "panmax" );
Event EV_Camera_SetPanAngles( "setpanangles" );
Event EV_Camera_SetNextCamera( "nextcamera" );
Event EV_Camera_SetOverlay( "setoverlay" );
Event EV_Camera_SetThread( "setthread" );

/*****************************************************************************/
/*SINED func_camera (1 0 0) ? ORBIT START_ON PAN

Camera used for cinematic sequences.  Start

"target" points to the target to orbit or follow.  If it points to a path, the camera will follow the path.
"distance" the distance to follow or orbit if the target is not a path. (default 128).
"speed" specifies how fast to move on the path or orbit.  (default 1).
"fov" specifies fov of camera, default 90.
"yaw" specifies yaw of camera, default 0.
"height" specifies height of camera from origin, default 128.
"panspeed" speed at which to pan ( 7 degrees per second )
"panmax" maximum angle offset for panning ( 45 degrees )
"nextcamera" a link to the next camera in a chain of cameras
"overlay" an overlay to use while looking through the camera
"thread" a thread label that will be fired when the camera is looked through

ORBIT tells the camera to create a circular path around the object it points to.  It the camera points to a path, it will loop when it gets to the end of the path.
START_ON causes the camera to be moving as soon as it is spawned.
PAN camera should pan from right to left

/*****************************************************************************/

CLASS_DECLARATION( Entity, Camera, "func_camera" );

ResponseDef Camera::Responses[] =
{
   { &EV_Camera_FollowingPath,	(Response)&Camera::FollowingPath },
   { &EV_Activate,					(Response)&Camera::StartMoving },
   { &EV_Camera_StartMoving,		(Response)&Camera::StartMoving },
   { &EV_Camera_StopMoving,		(Response)&Camera::StopMoving },
   { &EV_Camera_Pause,				(Response)&Camera::Pause },
   { &EV_Camera_Continue,			(Response)&Camera::Continue },
   { &EV_Camera_SetSpeed,			(Response)&Camera::SetSpeed },
   { &EV_Camera_SetDistance,		(Response)&Camera::SetDistance },
   { &EV_Camera_SetHeight,			(Response)&Camera::SetHeight },
   { &EV_Camera_SetYaw,				(Response)&Camera::SetYaw },
   { &EV_Camera_FixedYaw,			(Response)&Camera::FixedYaw },
   { &EV_Camera_RelativeYaw,		(Response)&Camera::RelativeYaw },
   { &EV_Camera_SetFOV,				(Response)&Camera::SetFOV },
   { &EV_Camera_Orbit,				(Response)&Camera::OrbitEvent },
   { &EV_Camera_Follow,				(Response)&Camera::FollowEvent },
   { &EV_Camera_Watch,				(Response)&Camera::WatchEvent },
   { &EV_Camera_NoWatch,			(Response)&Camera::NoWatchEvent },
   { &EV_Camera_LookAt,				(Response)&Camera::LookAt },
   { &EV_Camera_TurnTo,				(Response)&Camera::TurnTo },
   { &EV_Camera_MoveToEntity,  	(Response)&Camera::MoveToEntity },
   { &EV_Camera_MoveToPos,			(Response)&Camera::MoveToPos },
   { &EV_Camera_IgnoreAngles,		(Response)&Camera::IgnoreAngles },
   { &EV_Camera_UseAngles,		   (Response)&Camera::UseAngles },
   { &EV_Camera_SplineAngles,		(Response)&Camera::SplineAngles },
   { &EV_Camera_NormalAngles,		(Response)&Camera::NormalAngles },
   { &EV_Camera_FixedPosition,	(Response)&Camera::FixedPosition },
   { &EV_Camera_NoFixedPosition,	(Response)&Camera::NoFixedPosition },
   { &EV_Camera_JumpCut,	      (Response)&Camera::JumpCut },
   { &EV_Camera_JumpTime,	      (Response)&Camera::JumpTime },
   { &EV_Camera_Pan,    	      (Response)&Camera::PanEvent },
   { &EV_Camera_StopPan,    	   (Response)&Camera::StopPanEvent },
   { &EV_Camera_PanSpeed,    	   (Response)&Camera::PanSpeedEvent },
   { &EV_Camera_PanMax,    	   (Response)&Camera::PanMaxEvent },
   { &EV_Camera_SetPanAngles,    (Response)&Camera::SetPanAngles },
   { &EV_Camera_SetNextCamera,   (Response)&Camera::SetNextCamera },
   { &EV_Camera_SetOverlay,      (Response)&Camera::SetOverlay },
   { &EV_Camera_SetThread,       (Response)&Camera::SetThread },

   { NULL, NULL }
};

Camera::Camera() : Entity()
{
   Vector ang;

   default_fov = G_GetFloatArg("fov", 90);
   if(default_fov <= 0)
      default_fov = 90;
   default_yaw = G_GetFloatArg("yaw", 0);

   default_follow_dist = G_GetFloatArg("distance", 128);
   default_height = G_GetFloatArg("height", 128);
   default_speed = G_GetFloatArg("speed", 1);

   default_pan_speed = G_GetFloatArg("panspeed", 7);
   default_pan_max = G_GetFloatArg("panmax", 45);

   nextCamera = G_GetStringArg("nextcamera");
   overlay = G_GetStringArg("overlay");
   thread = G_GetStringArg("thread");

   watchTime = 0;
   followTime = 0;

   targetEnt = NULL;
   targetWatchEnt = NULL;

   fov = default_fov;

   jumpTime = 2.0f;

   setSolidType(SOLID_NOT);
   setMoveType(MOVETYPE_NONE);

   ang = G_GetVectorArg("angles", vec_zero);
   setAngles(ang);
   default_angles = ang;

   InitializeState(currentstate);
   InitializeState(newstate);

   sv_showcameras = gi.cvar("sv_showcameras", "0", 0);
   showcamera = sv_showcameras->value;
   if(showcamera)
   {
      setModel("xyz.def");
      showModel();
   }
   else
   {
      hideModel();
   }

   if(spawnflags & START_ON)
   {
      PostEvent(EV_Activate, FRAMETIME);
   }
}

void Camera::InitializeMoveState(CameraMoveState &movestate)
{
   movestate.pos = worldorigin;
   movestate.followEnt = NULL;
   movestate.orbitEnt = NULL;

   movestate.followingpath = false;
   movestate.cameraTime = 0;
   movestate.cameraPath.Clear();

   movestate.fov = default_fov;
   movestate.fixed_position = false;

   movestate.follow_dist = default_follow_dist;
   movestate.follow_mask = MASK_SOLID;
   movestate.height = default_height;
   movestate.speed = default_speed;
}

void Camera::InitializeWatchState(CameraWatchState &watchstate)
{
   worldangles.AngleVectors(&watchstate.dir, NULL, NULL);
   watchstate.watchEnt = NULL;

   watchstate.ignoreangles = false;
   watchstate.splineangles = true;
   watchstate.panning = false;

   watchstate.pan_offset = 0;
   watchstate.pan_dir = 1;
   watchstate.pan_max = default_pan_max;
   watchstate.pan_speed = default_pan_speed;
   watchstate.pan_angles = default_angles;

   watchstate.yaw = default_yaw;
   watchstate.fixedyaw = false;
}

void Camera::InitializeState(CameraState &state)
{
   InitializeMoveState(state.move);
   InitializeWatchState(state.watch);
}

#define DELTA 1e-6

void Camera::EvaluatePosition(CameraState &state)
{
   Vector oldpos, olddir;
   float speed_multiplier;
   Vector prevpos;

   prevpos = state.move.pos;

   olddir = state.watch.dir;

   //
   // evaluate position
   //
   if(state.move.followingpath)
   {
      speed_multiplier = state.move.cameraPath.Eval(state.move.cameraTime, oldpos, olddir);

      state.move.cameraTime += FRAMETIME * state.move.speed * speed_multiplier;

      if(state.move.orbitEnt)
      {
         oldpos += state.move.orbitEnt->worldorigin;
      }
   }
   else
   {
      if(!state.move.followEnt)
      {
         oldpos = state.move.pos;
      }
      else
      {
         trace_t trace;
         Vector start, end, ang, back, temp;
         const gravityaxis_t &grav = gravity_axis[state.move.followEnt->gravaxis];

         start = state.move.followEnt->worldorigin;
         start[grav.z] += state.move.followEnt->maxs[2];

         if(state.watch.fixedyaw)
         {
            ang = vec_zero;
         }
         else
         {
            if(state.move.followEnt->isSubclassOf<Player>())
            {
               Entity * ent;
               ent = state.move.followEnt;
               ang = ((Player *)ent)->v_angle;
            }
            else
            {
               ang = state.move.followEnt->worldangles;
            }
         }
         ang.y += state.watch.yaw;
         ang.AngleVectors(&temp, NULL, NULL);
         back[grav.x] = temp[0];
         back[grav.y] = temp[1] * grav.sign;
         back[grav.z] = temp[2] * grav.sign;

         end = start - back * state.move.follow_dist;
         end[2] += 24;

         trace = G_Trace(start, vec_zero, vec_zero, end, state.move.followEnt, state.move.follow_mask, "Camera::EvaluatePosition");
         //dir = start - trace.endpos;
         //dir.normalize();

         end = trace.endpos;
         oldpos = end + back * 16;
      }
   }

   //
   // evaluate old orientation
   //
   if(state.watch.watchEnt)
   {
      Vector watchPos;

      watchPos.x = state.watch.watchEnt->worldorigin.x;
      watchPos.y = state.watch.watchEnt->worldorigin.y;
      watchPos.z = state.watch.watchEnt->absmax.z;
      if(state.move.followEnt == state.watch.watchEnt)
      {
         olddir = watchPos - oldpos;
      }
      else
      {
         olddir = watchPos - worldorigin;
      }
   }
   else
   {
      if(state.watch.ignoreangles)
      {
         olddir = state.watch.dir;
      }
      else if(state.move.followingpath)
      {
         if(!state.watch.splineangles)
         {
            olddir = oldpos - prevpos;
         }
         else
         {
            Vector dir;

            dir = olddir;
            dir.AngleVectors(&olddir, NULL, NULL);
         }
      }
      else if(state.move.followEnt)
      {
         Vector start;

         start = state.move.followEnt->worldorigin;
         start[2] += state.move.followEnt->maxs[2];
         olddir = oldpos - start;
      }
      else if(state.watch.panning)
      {
         Vector ang;
         state.watch.pan_offset += FRAMETIME * state.watch.pan_speed * state.watch.pan_dir;
         if(state.watch.pan_offset > state.watch.pan_max)
         {
            state.watch.pan_offset = state.watch.pan_max;
            state.watch.pan_dir = -state.watch.pan_dir;
         }
         else if(state.watch.pan_offset < -state.watch.pan_max)
         {
            state.watch.pan_offset = -state.watch.pan_max;
            state.watch.pan_dir = -state.watch.pan_dir;
         }

         ang = state.watch.pan_angles;
         ang[YAW] += state.watch.pan_offset;
         ang.AngleVectors(&olddir, NULL, NULL);
      }
   }
   olddir.normalize();

   if(!state.move.fixed_position)
      state.move.pos = oldpos;
   state.watch.dir = olddir;
}

void Camera::FollowingPath(Event *ev)
{
   Vector   pos;
   Vector   dir;
   float    len;

   //
   // evaluate position
   //
   if(followTime || watchTime)
   {
      int i;
      float t;

      EvaluatePosition(currentstate);
      EvaluatePosition(newstate);

      if(followTime)
      {
         t = followTime - level.time;
         if(t < 0)
         {
            t = 0;
            currentstate.move = newstate.move;
            InitializeMoveState(newstate.move);
            followTime = 0;
         }
         //
         // we want the transition to happen over 2 seconds
         //
         if(jumpTime == 0.0f) // haleyjd 20170614:  must tolerate jumpTime of 0
         {
            pos = newstate.move.pos;
            fov = newstate.move.fov;
         }
         else
         {
            t = (jumpTime - t) / jumpTime;

            for(i = 0; i < 3; i++)
            {
               pos[i] = currentstate.move.pos[i] + (t * (newstate.move.pos[i] - currentstate.move.pos[i]));
            }
            fov = currentstate.move.fov + (t * (newstate.move.fov - currentstate.move.fov));
         }
      }
      else
      {
         fov = currentstate.move.fov;
         pos = currentstate.move.pos;
      }

      if(watchTime)
      {
         t = watchTime - level.time;
         if(t < 0)
         {
            t = 0;
            currentstate.watch = newstate.watch;
            InitializeWatchState(newstate.watch);
            watchTime = 0;
         }
         //
         // we want the transition to happen over 2 seconds
         //
         t = (jumpTime - t) / jumpTime;

         dir = LerpVector(currentstate.watch.dir, newstate.watch.dir, t);
      }
      else
      {
         dir = currentstate.watch.dir;
      }
   }
   else
   {
      EvaluatePosition(currentstate);
      fov = currentstate.move.fov;
      pos = currentstate.move.pos;
      dir = currentstate.watch.dir;
      //warning("FollowingPath","%p pos x%.2f y%.2f z%2.f time %.2f", this, pos.x, pos.y, pos.z, level.time );
   }

   setOrigin(pos);

   len = dir.length();
   if(len > 0.05)
   {
      dir *= (1 / len);
      angles = dir.toAngles();
      angles[PITCH] = -angles[PITCH];
      setAngles(angles);
      //warning("FollowingPath","%p angles x%.2f y%.2f z%2.f time %.2f", this, angles.x, angles.y, angles.z, level.time );
   }


   if(sv_showcameras->value != showcamera)
   {
      showcamera = sv_showcameras->value;
      if(showcamera)
      {
         setModel("xyz.def");
         showModel();
      }
      else
      {
         hideModel();
      }
   }

   if(sv_showcameras->value != showcamera)
   {
      showcamera = sv_showcameras->value;
      if(showcamera)
      {
         setModel("xyz.def");
         showModel();
      }
      else
      {
         hideModel();
      }
   }
   if(showcamera && currentstate.move.followingpath)
   {
      G_Color3f(1, 1, 0);
      if(currentstate.watch.watchEnt)
      {
         currentstate.move.cameraPath.DrawCurve(currentstate.watch.watchEnt->worldorigin, 10);
      }
      else
      {
         currentstate.move.cameraPath.DrawCurve(10);
      }
   }


   PostEvent(EV_Camera_FollowingPath, FRAMETIME);
}

void Camera::LookAt(Event *ev)
{
   Vector pos, delta;
   float len;
   Entity * ent;

   ent = ev->GetEntity(1);

   if(!ent)
      return;

   pos.x = ent->worldorigin.x;
   pos.y = ent->worldorigin.y;
   pos.z = ent->absmax.z;
   delta = pos - worldorigin;
   delta.normalize();
   currentstate.watch.dir = delta;

   len = currentstate.watch.dir.length();
   if(len > 0.05)
   {
      angles = currentstate.watch.dir.toAngles();
      angles[PITCH] = -angles[PITCH];
      setAngles(angles);
   }
}

void Camera::TurnTo(Event *ev)
{
   Vector ang;

   ang = ev->GetVector(1);
   ang.AngleVectors(&currentstate.watch.dir, NULL, NULL);
   setAngles(ang);
}

void Camera::MoveToEntity(Event *ev)
{
   Entity * ent;

   ent = ev->GetEntity(1);
   if(ent)
      currentstate.move.pos = ent->worldorigin;
   setOrigin(currentstate.move.pos);
}

void Camera::MoveToPos(Event *ev)
{
   currentstate.move.pos = ev->GetVector(1);
   setOrigin(currentstate.move.pos);
}

void Camera::Stop(void)
{
   if(followTime)
   {
      currentstate.move = newstate.move;
      InitializeMoveState(newstate.move);
   }
   if(watchTime)
   {
      currentstate.watch = newstate.watch;
      InitializeWatchState(newstate.watch);
   }
   CancelEventsOfType(moveevent);
   //	moveevent = NullEvent;
   watchTime = 0;
   followTime = 0;
}

void Camera::CreateOribit(Vector pos, float radius)
{
   newstate.move.cameraPath.Clear();
   newstate.move.cameraPath.SetType(SPLINE_LOOP);

   newstate.move.cameraPath.AppendControlPoint(pos + Vector(radius, 0, 0));
   newstate.move.cameraPath.AppendControlPoint(pos + Vector(0, radius, 0));
   newstate.move.cameraPath.AppendControlPoint(pos + Vector(-radius, 0, 0));
   newstate.move.cameraPath.AppendControlPoint(pos + Vector(0, -radius, 0));
}

void Camera::CreatePath(SplinePath *path, splinetype_t type)
{
   SplinePath	*node;
   SplinePath	*loop;

   newstate.move.cameraPath.Clear();
   newstate.move.cameraPath.SetType(type);

   node = path;
   while(node != NULL)
   {
      newstate.move.cameraPath.AppendControlPoint(node->worldorigin, node->angles, node->speed);
      loop = node->GetLoop();
      if(loop)
      {
         newstate.move.cameraPath.SetLoopPoint(loop->worldorigin);
      }
      node = node->GetNext();

      if(node == path)
      {
         break;
      }
   }
}

void Camera::FollowPath(SplinePath *path, qboolean loop, Entity * watch)
{
   Stop();
   if(loop)
   {
      CreatePath(path, SPLINE_LOOP);
   }
   else
   {
      CreatePath(path, SPLINE_CLAMP);
   }

   newstate.move.cameraTime = -2;
   newstate.move.followingpath = true;
   followTime = level.time + jumpTime;
   watchTime = level.time + jumpTime;

   if(watch)
   {
      newstate.watch.watchEnt = watch;
   }
   else
   {
      newstate.watch.watchEnt = NULL;
   }

   moveevent = EV_Camera_FollowingPath;
   PostEvent(EV_Camera_FollowingPath, FRAMETIME);
}

void Camera::Orbit(Entity *ent, float dist, Entity *watch)
{
   Stop();
   CreateOribit(Vector(0, 0, newstate.move.height), dist);
   newstate.move.cameraTime = -2;
   newstate.move.orbitEnt = ent;
   newstate.move.followingpath = true;
   followTime = level.time + jumpTime;

   watchTime = level.time + jumpTime;
   if(watch)
   {
      newstate.watch.watchEnt = watch;
   }
   else
   {
      newstate.watch.watchEnt = ent;
   }

   moveevent = EV_Camera_FollowingPath;
   PostEvent(EV_Camera_FollowingPath, FRAMETIME);
}

void Camera::FollowEntity(Entity *ent, float dist, int mask, Entity *watch)
{
   assert(ent);

   Stop();

   if(ent)
   {
      newstate.move.followEnt = ent;
      newstate.move.followingpath = false;
      followTime = level.time + jumpTime;
      watchTime = level.time + jumpTime;
      if(watch)
      {
         newstate.watch.watchEnt = watch;
      }
      else
      {
         newstate.watch.watchEnt = ent;
      }
      newstate.move.follow_dist = dist;
      newstate.move.follow_mask = mask;
      moveevent = EV_Camera_FollowingPath;
      PostEvent(EV_Camera_FollowingPath, 0);
   }
}

void Camera::StartMoving(Event *ev)
{
   Entity *ent;
   SplinePath *path;
   int num;

   if(!targetEnt)
   {
      num = G_FindTarget(0, Target());
      ent = G_GetEntity(num);
      if(!num || !ent)
      {
         if(spawnflags & PANNING)
         {
            currentstate.watch.panning = true;
            moveevent = EV_Camera_FollowingPath;
            PostEvent(EV_Camera_FollowingPath, FRAMETIME);
            return;
         }
         //
         // we took this out just because of too many warnings, oh well
         //
         //warning("StartMoving", "Can't find target for camera\n" );
         return;
      }
   }
   else
   {
      ent = targetEnt;
   }

   if(ent->isSubclassOf<SplinePath>())
   {
      path = static_cast<SplinePath *>(ent);
      FollowPath(path, spawnflags & ORBIT, targetWatchEnt);
   }
   else
   {
      if(spawnflags & ORBIT)
      {
         Orbit(ent, newstate.move.follow_dist, targetWatchEnt);
      }
      else
      {
         FollowEntity(ent, newstate.move.follow_dist, newstate.move.follow_mask, targetWatchEnt);
      }
   }
}

void Camera::StopMoving(Event *ev)
{
   Stop();
}

void Camera::Pause(Event *ev)
{
   CancelEventsOfType(moveevent);
}

void Camera::Continue(Event *ev)
{
   if((int)moveevent != (int)NullEvent)
   {
      CancelEventsOfType(moveevent);
      PostEvent(moveevent, 0);
   }
}

void Camera::SetSpeed(Event *ev)
{
   newstate.move.speed = ev->GetFloat(1);
}

void Camera::SetDistance(Event *ev)
{
   newstate.move.follow_dist = ev->GetFloat(1);
}

void Camera::SetCurrentDistance
(float dist)
{
   currentstate.move.follow_dist = dist;
}

void Camera::SetHeight(Event *ev)
{
   newstate.move.height = ev->GetFloat(1);
}

void Camera::SetYaw(Event *ev)
{
   newstate.watch.yaw = ev->GetFloat(1);
}

void Camera::FixedYaw(Event *ev)
{
   newstate.watch.fixedyaw = true;
}

void Camera::RelativeYaw(Event *ev)
{
   newstate.watch.fixedyaw = false;
}

void Camera::IgnoreAngles(Event *ev)
{
   newstate.watch.ignoreangles = true;
}

void Camera::UseAngles(Event *ev)
{
   newstate.watch.ignoreangles = false;
}

void Camera::SplineAngles(Event *ev)
{
   newstate.watch.splineangles = true;
}

void Camera::NormalAngles(Event *ev)
{
   newstate.watch.splineangles = false;
}

void Camera::FixedPosition(Event *ev)
{
   newstate.move.fixed_position = true;
}

void Camera::NoFixedPosition(Event *ev)
{
   newstate.move.fixed_position = false;
}

void Camera::PanEvent(Event *ev)
{
   currentstate.watch.panning = true;
}

void Camera::StopPanEvent(Event *ev)
{
   currentstate.watch.panning = false;
}

void Camera::PanSpeedEvent(Event *ev)
{
   currentstate.watch.pan_speed = ev->GetFloat(1);
}

void Camera::PanMaxEvent(Event *ev)
{
   currentstate.watch.pan_max = ev->GetFloat(1);
}

void Camera::SetPanAngles(Event *ev)
{
   if(ev->NumArgs() > 0)
   {
      currentstate.watch.pan_angles = ev->GetVector(1);
   }
   else
   {
      currentstate.watch.pan_angles = worldangles;
   }
}

void Camera::SetNextCamera(Event *ev)
{
   nextCamera = ev->GetString(1);
}

void Camera::SetOverlay(Event *ev)
{
   overlay = ev->GetString(1);
}

void Camera::JumpCut(Event *ev)
{
   if(followTime)
   {
      currentstate.move = newstate.move;
      InitializeMoveState(newstate.move);
      followTime = 0;
   }
   if(watchTime)
   {
      currentstate.watch = newstate.watch;
      InitializeWatchState(newstate.watch);
      watchTime = 0;
   }
   if(static_cast<int>(moveevent)) // haleyjd 20170606: explicit cast
   {
      CancelEventsOfType(moveevent);
      ProcessEvent(Event(moveevent));
   }
}

void Camera::JumpTime(Event *ev)
{
   float t;
   float newjumptime;

   newjumptime = ev->GetFloat(1);
   if(followTime)
   {
      t = (jumpTime - (level.time - followTime)) / jumpTime;
      followTime = level.time + (t * newjumptime);
   }
   if(watchTime)
   {
      t = (jumpTime - (level.time - watchTime)) / jumpTime;
      watchTime = level.time + (t * newjumptime);
   }
   jumpTime = newjumptime;
}

void Camera::OrbitEvent(Event *ev)
{
   Entity *ent;

   spawnflags |= ORBIT;
   ent = ev->GetEntity(1);
   if(ent)
   {
      targetEnt = ent;
      targetWatchEnt = NULL;
      if(ev->NumArgs() > 1)
         targetWatchEnt = ev->GetEntity(2);
      if(static_cast<int>(moveevent)) // haleyjd 20170606: explicit cast
      {
         Stop();
      }
      ProcessEvent(EV_Activate);
   }
}

void Camera::FollowEvent(Event *ev)
{
   Entity *ent;

   spawnflags &= ~ORBIT;
   ent = ev->GetEntity(1);
   if(ent)
   {
      targetEnt = ent;
      targetWatchEnt = NULL;
      if(ev->NumArgs() > 1)
         targetWatchEnt = ev->GetEntity(2);
      if(static_cast<int>(moveevent)) // haleyjd 20170606: explicit cast
      {
         Stop();
      }
      ProcessEvent(EV_Activate);
   }
}

void Camera::SetFOV(Event *ev)
{
   currentstate.move.fov = ev->GetFloat(1);
}

void Camera::WatchEvent(Event *ev)
{
   watchTime = level.time + jumpTime;
   newstate.watch.watchEnt = ev->GetEntity(1);
}

void Camera::NoWatchEvent(Event *ev)
{
   watchTime = level.time + jumpTime;
   newstate.watch.watchEnt = NULL;
}

void SetCamera(Entity *ent)
{
   int j;
   edict_t		*other;

   for(j = 1; j <= game.maxclients; j++)
   {
      other = &g_edicts[j];
      if(other->inuse && other->client)
      {
         Player * client;
         client = (Player *)other->entity;
         client->SetCamera(ent);
      }
   }
}

str &Camera::NextCamera(void)
{
   return nextCamera;
}

str &Camera::Overlay(void)
{
   return overlay;
}

void Camera::SetThread(Event *ev)
{
   thread = ev->GetString(1);
}

str &Camera::Thread(void)
{
   return thread;
}

void Camera::Reset(Vector org, Vector ang)
{
   setOrigin(org);
   setAngles(ang);
   InitializeState(currentstate);
   InitializeState(newstate);
}

CLASS_DECLARATION( Camera, SecurityCamera, "func_securitycamera" );

ResponseDef SecurityCamera::Responses[] =
{
   { NULL, NULL }
};

SecurityCamera::SecurityCamera()
{
   setModel("camera.def");
   showModel();
}

/******************************************************************************

  Camera Manager

******************************************************************************/

Event EV_CameraManager_NewPath( "new", EV_CONSOLE | EV_CHEAT );
Event EV_CameraManager_SetPath( "setpath", EV_CONSOLE | EV_CHEAT );
Event EV_CameraManager_SetTargetName( "settargetname", EV_CONSOLE | EV_CHEAT );
Event EV_CameraManager_SetTarget( "settarget", EV_CONSOLE | EV_CHEAT );
Event EV_CameraManager_AddPoint( "add", EV_CONSOLE | EV_CHEAT );
Event EV_CameraManager_DeletePoint( "delete", EV_CONSOLE | EV_CHEAT );
Event EV_CameraManager_MovePlayer( "moveplayer", EV_CONSOLE | EV_CHEAT );
Event EV_CameraManager_ReplacePoint( "replace", EV_CONSOLE | EV_CHEAT );
Event EV_CameraManager_NextPoint( "next", EV_CONSOLE | EV_CHEAT );
Event EV_CameraManager_PreviousPoint( "prev", EV_CONSOLE | EV_CHEAT );
Event EV_CameraManager_ShowPath( "show" );
Event EV_CameraManager_HidePath( "hide" );
Event EV_CameraManager_PlayPath( "play", EV_CONSOLE | EV_CHEAT );
Event EV_CameraManager_LoopPath( "loop", EV_CONSOLE );
Event EV_CameraManager_StopPlayback( "stop" );
Event EV_CameraManager_Watch( "watch" );
Event EV_CameraManager_NoWatch( "nowatch" );
Event EV_CameraManager_Speed( "setspeed", EV_CONSOLE | EV_CHEAT );
Event EV_CameraManager_Save( "save", EV_CONSOLE | EV_CHEAT );
Event EV_CameraManager_Load( "load", EV_CONSOLE | EV_CHEAT );
Event EV_CameraManager_SaveMap( "savemap", EV_CONSOLE | EV_CHEAT );

CLASS_DECLARATION( Listener, CameraManager, NULL );

ResponseDef CameraManager::Responses[] =
{
   { &EV_CameraManager_NewPath,         (Response)&CameraManager::NewPath },
   { &EV_CameraManager_SetPath,         (Response)&CameraManager::SetPath },
   { &EV_CameraManager_SetTargetName,   (Response)&CameraManager::SetTargetName },
   { &EV_CameraManager_SetTarget,       (Response)&CameraManager::SetTarget },
   { &EV_CameraManager_SetPath,         (Response)&CameraManager::SetPath },
   { &EV_CameraManager_AddPoint,        (Response)&CameraManager::AddPoint },
   { &EV_CameraManager_ReplacePoint,    (Response)&CameraManager::ReplacePoint },
   { &EV_CameraManager_DeletePoint,     (Response)&CameraManager::DeletePoint },
   { &EV_CameraManager_MovePlayer,      (Response)&CameraManager::MovePlayer },
   { &EV_CameraManager_NextPoint,       (Response)&CameraManager::NextPoint },
   { &EV_CameraManager_PreviousPoint,   (Response)&CameraManager::PreviousPoint },
   { &EV_CameraManager_ShowPath,        (Response)&CameraManager::ShowPath },
   { &EV_CameraManager_HidePath,        (Response)&CameraManager::HidePath },
   { &EV_CameraManager_PlayPath,        (Response)&CameraManager::PlayPath },
   { &EV_CameraManager_LoopPath,        (Response)&CameraManager::LoopPath },
   { &EV_CameraManager_StopPlayback,    (Response)&CameraManager::StopPlayback },
   { &EV_CameraManager_Watch,           (Response)&CameraManager::Watch },
   { &EV_CameraManager_NoWatch,         (Response)&CameraManager::NoWatch },
   { &EV_CameraManager_Speed,           (Response)&CameraManager::Speed },
   { &EV_CameraManager_Save,            (Response)&CameraManager::Save },
   { &EV_CameraManager_Load,            (Response)&CameraManager::Load },
   { &EV_CameraManager_SaveMap,         (Response)&CameraManager::SaveMap },
   { NULL, NULL }
};

Player *CameraManager_GetPlayer(void)
{
   assert(g_edicts[1].entity && g_edicts[1].entity->isSubclassOf<Player>());
   if(!g_edicts[1].entity || !(g_edicts[1].entity->isSubclassOf<Player>()))
   {
      gi.printf("No player found.\n");
      return NULL;
   }

   return (Player *)g_edicts[1].entity;
}

void CameraManager::NewPath(Event *ev)
{
   if(path)
   {
      path = NULL;
      current = NULL;
   }

   ProcessEvent(EV_CameraManager_ShowPath);
}

void CameraManager::SetPath(SplinePath *node)
{
   path = node;
   current = node;
}

void CameraManager::SetPath(Event *ev)
{
   Entity *ent;

   if(!ev->NumArgs())
   {
      ev->Error("Usage: cam setpath [pathname]");
      return;
   }

   ent = ev->GetEntity(1);
   if(!ent)
   {
      ev->Error("Could not find path named '%s'.", ev->GetString(1));
      return;
   }

   if(!ent->isSubclassOf<SplinePath>())
   {
      ev->Error("'%s' is not a camera path.", ev->GetString(1));
      return;
   }

   SetPath(static_cast<SplinePath *>(ent));
}

void CameraManager::SetTargetName(Event *ev)
{
   if(ev->NumArgs() != 1)
   {
      ev->Error("Usage: cam targetname [name]");
      return;
   }

   if(!path)
   {
      ev->Error("Camera path not set.");
      return;
   }

   path->SetTargetName(ev->GetString(1));
}

void CameraManager::SetTarget(Event *ev)
{
   if(ev->NumArgs() != 1)
   {
      ev->Error("Usage: cam target [name]");
      return;
   }

   if(!current)
   {
      ev->Error("Camera path not set.");
      return;
   }

   current->SetTarget(ev->GetString(1));
}

void CameraManager::AddPoint(Event *ev)
{
   Player *player;
   SplinePath *prev;
   SplinePath *next;
   Vector ang;
   Vector pos;

   player = CameraManager_GetPlayer();
   if(player)
   {
      prev = current;
      if(current)
      {
         next = current->GetNext();
      }
      else
      {
         next = NULL;
      }

      player->GetPlayerView(&pos, &ang);

      current = new SplinePath();
      current->setOrigin(pos);
      current->setAngles(ang);
      current->speed = speed;
      current->SetPrev(prev);
      current->SetNext(next);

      if(!path)
      {
         path = current;
      }

      CancelEventsOfType(EV_CameraManager_ShowPath);
      ProcessEvent(EV_CameraManager_ShowPath);
   }
}

void CameraManager::ReplacePoint(Event *ev)
{
   Player *player;
   Vector ang;
   Vector pos;

   player = CameraManager_GetPlayer();
   if(current && player)
   {
      player->GetPlayerView(&pos, &ang);

      current->setOrigin(pos);
      current->setAngles(ang);
      current->speed = speed;
   }
}

void CameraManager::DeletePoint(Event *ev)
{
   SplinePath *node;

   if(current)
   {
      node = current->GetNext();
      if(!node)
      {
         node = current->GetPrev();
      }

      if(path == current)
      {
         path = current->GetNext();
      }

      delete current;
      current = node;
   }
}

void CameraManager::MovePlayer(Event *ev)
{
   Player *player;
   Vector pos;

   player = CameraManager_GetPlayer();
   if(current && player)
   {
      player->GetPlayerView(&pos, NULL);

      player->setOrigin(current->origin - pos + player->origin);
      player->setAngles(current->angles);
   }
}

void CameraManager::NextPoint(Event *ev)
{
   SplinePath *next;

   if(current)
   {
      next = current->GetNext();
      if(next)
      {
         current = next;
      }
   }
}

void CameraManager::PreviousPoint(Event *ev)
{
   SplinePath *prev;

   if(current)
   {
      prev = current->GetPrev();
      if(prev)
      {
         current = prev;
      }
   }
}

void CameraManager::ShowPath(Event *ev)
{
   SplinePath *node;
   SplinePath *prev;
   Vector mins(-8, -8, -8);
   Vector maxs(8, 8, 8);

   if(ev->NumArgs())
   {
      Entity *ent;

      ent = ev->GetEntity(1);
      if(!ent)
      {
         ev->Error("Could not find path named '%s'.", ev->GetString(1));
         return;
      }

      if(!ent->isSubclassOf<SplinePath>())
      {
         ev->Error("'%s' is not a camera path.", ev->GetString(1));
         return;
      }

      SetPath(static_cast<SplinePath *>(ent));
   }

   prev = NULL;
   for(node = path; node != NULL; node = node->GetNext())
   {
      if(prev)
      {
         //G_LineStipple( 4, ( unsigned short )( 0xF0F0F0F0 >> ( 7 - ( level.framenum & 7 ) ) ) );
         G_DebugLine(prev->origin, node->origin, 1, 1, 1, 1);
         //G_LineStipple( 1, 0xffff );
      }

      if(current == node)
      {
         G_DrawDebugNumber(node->origin + Vector(0, 0, 20), node->speed, 0.5, 0, 1, 0, 1);
         G_DebugBBox(node->origin, mins, maxs, 1, 1, 0, 1);
      }
      else
      {
         G_DebugBBox(node->origin, mins, maxs, 1, 0, 0, 1);
      }

      //G_LineWidth( 3 );
      G_DebugLine(node->origin, node->origin + Vector(node->orientation[0]) * 16, 1, 0, 0, 1);
      G_DebugLine(node->origin, node->origin + Vector(node->orientation[1]) * 16, 0, 1, 0, 1);
      G_DebugLine(node->origin, node->origin + Vector(node->orientation[2]) * 16, 0, 0, 1, 1);
      //G_LineWidth( 1 );

      prev = node;
   }
   PostEvent(EV_CameraManager_ShowPath, FRAMETIME);
}

void CameraManager::HidePath(Event *ev)
{
   CancelEventsOfType(EV_CameraManager_ShowPath);
}

void CameraManager::StopPlayback(Event *ev)
{
   if(cam)
   {
      cam->Stop();
      SetCamera(NULL);
   }
}

void CameraManager::PlayPath(Event *ev)
{
   if(cam)
   {
      SetCamera(NULL);
   }

   if(ev->NumArgs())
   {
      Entity *ent;

      ent = ev->GetEntity(1);
      if(!ent)
      {
         ev->Error("Could not find path named '%s'.", ev->GetString(1));
         return;
      }

      if(!ent->isSubclassOf<SplinePath>())
      {
         ev->Error("'%s' is not a camera path.", ev->GetString(1));
         return;
      }

      SetPath(static_cast<SplinePath *>(ent));
   }

   if(path)
   {
      if(!cam)
      {
         cam = new Camera();
         cam->SetTargetName("_loadedcamera");
      }

      cam->Reset(path->origin, path->angles);
      cam->FollowPath(path, false, NULL);
      SetCamera(cam);
   }
}

void CameraManager::LoopPath(Event *ev)
{
   if(cam)
   {
      SetCamera(nullptr);
   }

   if(ev->NumArgs())
   {
      Entity *ent;

      ent = ev->GetEntity(1);
      if(!ent)
      {
         ev->Error("Could not find path named '%s'.", ev->GetString(1));
         return;
      }

      if(!ent->isSubclassOf<SplinePath>())
      {
         ev->Error("'%s' is not a camera path.", ev->GetString(1));
         return;
      }

      SetPath(static_cast<SplinePath *>(ent));
   }

   if(path)
   {
      if(!cam)
      {
         cam = new Camera();
         cam->SetTargetName("_loadedcamera");
      }

      cam->Reset(path->origin, path->angles);
      cam->FollowPath(path, true, nullptr);
      SetCamera(cam);
   }
}

void CameraManager::Watch(Event *ev)
{
   if(current)
   {
      current->SetWatch(ev->GetString(1));
   }
}

void CameraManager::NoWatch(Event *ev)
{
   if(current)
   {
      current->NoWatch();
   }
}

void CameraManager::Speed(Event *ev)
{
   speed = ev->GetFloat(1);
   if(current)
   {
      current->speed = speed;
   }
}

void CameraManager::Save(Event *ev)
{
   SplinePath *node;
   Archiver arc;
   str filename;
   int num;

   if(ev->NumArgs() != 1)
   {
      ev->Error("Usage: cam save [filename]");
      return;
   }

   num = 0;
   for(node = path; node != nullptr; node = node->GetNext())
   {
      num++;
   }

   if(num == 0)
   {
      ev->Error("Can't save.  No points in path.");
      return;
   }

   filename = gi.GameDir();
   filename += "/cams/";
   filename += ev->GetString(1);
   filename += ".cam";

   if(!path->targetname.length())
   {
      path->SetTargetName(ev->GetString(1));
      gi.printf("Targetname set to '%s'\n", path->targetname.c_str());
   }

   gi.printf("Saving camera path to '%s'...\n", filename.c_str());

   arc.Create(filename);
   arc.WriteInteger(CAMERA_PATHFILE_VERSION);

   arc.WriteInteger(num);
   arc.WriteSafePointer(path);

   for(node = path; node != nullptr; node = node->GetNext())
   {
      arc.WriteObject(node);
   }

   arc.Close();

   gi.printf("done.\n");
}

void CameraManager::Load(Event *ev)
{
   Archiver arc;
   str		filename;
   int		version;
   int      i;
   int      num;

   if(ev->NumArgs() != 1)
   {
      ev->Error("Usage: cam load [filename]");
      return;
   }

   filename = "cams/";
   filename += ev->GetString(1);
   filename += ".cam";

   gi.printf("Loading camera path from '%s'...\n", filename.c_str());

   arc.Read(filename);
   version = arc.ReadInteger();
   if(version == CAMERA_PATHFILE_VERSION)
   {
      arc.ReadInteger(&num);

      arc.ReadSafePointer(&path);

      for(i = 0; i < num; i++)
      {
         arc.ReadObject();
      }

      arc.Close();
      current = path;

      gi.printf("done.\n");
   }
   else
   {
      arc.Close();
      ev->Error("Expecting version %d path file.  Camera path file is version %d.", CAMERA_PATHFILE_VERSION, version);
   }
}

void CameraManager::SaveMap(Event *ev)
{
   SplinePath *node;
   str         buf;
   char        tempbuf[512];
   str         filename;
   int         num;
   int         index;
   FILE       *file;

   if(ev->NumArgs() != 1)
   {
      ev->Error("Usage: cam savemap [filename]");
      return;
   }

   num = 0;
   for(node = path; node != NULL; node = node->GetNext())
   {
      num++;
   }

   if(num == 0)
   {
      ev->Error("Can't save.  No points in path.");
      return;
   }

   filename = "cams/";
   filename += ev->GetString(1);
   filename += ".map";

   if(!path->targetname.length())
   {
      path->SetTargetName(ev->GetString(1));
      gi.printf("Targetname set to '%s'\n", path->targetname.c_str());
   }

   gi.printf("Saving camera path to map '%s'...\n", filename.c_str());

   buf = "";
   index = 0;
   for(node = path; node != NULL; node = node->GetNext())
   {
      snprintf(tempbuf, sizeof(tempbuf), "// pathnode %d\n", index);
      buf += str(tempbuf);
      buf += "{\n";
      snprintf(tempbuf, sizeof(tempbuf), "\"classname\" \"info_splinepath\"\n");
      buf += str(tempbuf);
      if(index < (num - 1))
      {
         snprintf(tempbuf, sizeof(tempbuf), "\"target\" \"camnode_%s_%d\"\n", ev->GetString(1), index + 1);
         buf += str(tempbuf);
      }
      if(!index)
      {
         snprintf(tempbuf, sizeof(tempbuf), "\"targetname\" \"%s\"\n", ev->GetString(1));
      }
      else
      {
         snprintf(tempbuf, sizeof(tempbuf), "\"targetname\" \"camnode_%s_%d\"\n", ev->GetString(1), index);
      }
      buf += str(tempbuf);
      snprintf(tempbuf, sizeof(tempbuf), "\"origin\" \"%d %d %d\"\n", (int)node->origin.x, (int)node->origin.y, (int)node->origin.z);
      buf += str(tempbuf);
      snprintf(tempbuf, sizeof(tempbuf), "\"angles\" \"%d %d %d\"\n", (int)anglemod(node->angles.x), (int)anglemod(node->angles.y), (int)anglemod(node->angles.z));
      buf += str(tempbuf);
      snprintf(tempbuf, sizeof(tempbuf), "\"speed\" \"%.3f\"\n", node->speed);
      buf += str(tempbuf);
      buf += "}\n";
      index++;
   }

   gi.CreatePath(filename.c_str());
   file = fopen(filename.c_str(), "wt");
   fwrite(buf.c_str(), 1, buf.length() + 1, file);
   fclose(file);

   gi.printf("done.\n");
}

// EOF

