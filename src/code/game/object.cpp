//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/object.cpp                       $
// $Revision:: 48                                                             $
//   $Author:: Markd                                                          $
//     $Date:: 11/13/98 10:00p                                                $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION: Skeet Entity
// 

#include "g_local.h"
#include "object.h"
#include "misc.h"
#include "explosion.h"
#include "gibs.h"
#include "specialfx.h"

CLASS_DECLARATION(Entity, Object, "object");

ResponseDef Object::Responses[] =
{
   { &EV_Killed,					      (Response)&Object::Killed },
   { NULL, NULL }
};

Object::Object() : Entity()
{
   const char * animname;
   const char * skinname;
   Vector defangles;

   SetKillTarget(G_GetSpawnArg("killtarget"));
   setSolidType(SOLID_BBOX);
   // if it isn't solid, lets still make it shootable
   if(spawnflags & 1)
   {
      if(!(spawnflags & 2))
      {
         edict->svflags |= SVF_SHOOTABLE;
         setOrigin(origin);
      }
      else
         setSolidType(SOLID_NOT);
   }
   if(!health)
   {
      health = (maxs - mins).length();
      max_health = health;
   }
   takedamage = DAMAGE_YES;
   if(spawnflags & 2)
   {
      takedamage = DAMAGE_NO;
   }

   // angles
   defangles = Vector(0, G_GetFloatArg("angle", 0), 0);
   if(defangles.y == -1)
   {
      defangles = Vector(-90, 0, 0);
   }
   else if(defangles.y == -2)
   {
      defangles = Vector(90, 0, 0);
   }
   angles = G_GetVectorArg("angles", defangles);
   setAngles(angles);
   setOrigin(origin);

   //
   // we want the bounds of this model auto-rotated
   //
   flags |= FL_ROTATEDBOUNDS;

   //
   // rotate the mins and maxs for the model
   //
   setSize(mins, maxs);

   animname = G_GetSpawnArg("anim");
   if(animname && strlen(animname) && gi.IsModel(edict->s.modelindex))
   {
      int animnum;

      animnum = gi.Anim_NumForName(edict->s.modelindex, animname);
      if(animnum >= 0)
         NextAnim(animnum);

      // Sets up the edict
      AnimateFrame();
      StopAnimating();
      // 
      // we only want to start animating if it was explicitly defined in the def file
      //
      //StartAnimating();
   }
   skinname = G_GetSpawnArg("skin");
   if(skinname && strlen(skinname) && gi.IsModel(edict->s.modelindex))
   {
      int skinnum;

      skinnum = gi.Skin_NumForName(edict->s.modelindex, skinname);
      if(skinnum >= 0)
         edict->s.skinnum = skinnum;
   }
   if(parentmode->value)
   {
      flags &= ~FL_BLOOD;
      flags &= ~FL_DIE_GIBS;
   }

   if(!(flags & (FL_BLOOD | FL_SPARKS | FL_TESSELATE)))
   {
      flags |= FL_DARKEN;
      flags |= FL_TESSELATE;
      flags |= FL_DIE_TESSELATE;
   }
}

void Object::Killed(Event *ev)
{
   Entity * ent;
   Entity * attacker;
   Vector dir;
   Event * event;
   const char * name;
   int num;

   takedamage = DAMAGE_NO;
   setSolidType(SOLID_NOT);
   hideModel();

   attacker = ev->GetEntity(1);

   if(flags & FL_DIE_TESSELATE)
   {
      dir = worldorigin - attacker->worldorigin;
      TesselateModel
      (
         this,
         tess_min_size,
         tess_max_size,
         dir,
         ev->GetInteger(2),
         tess_percentage,
         tess_thickness,
         vec3_origin
      );
      ProcessEvent(EV_BreakingSound);
   }

   if(flags & FL_DIE_EXPLODE)
   {
      CreateExplosion(worldorigin, 50, 0.5f, true, this, this, this);
   }

   if(flags & FL_DIE_GIBS)
   {
      setSolidType(SOLID_NOT);
      hideModel();

      CreateGibs(this, -150, edict->s.scale, 3);
   }

   //
   // kill the killtargets
   //
   name = KillTarget();
   if(name && strcmp(name, ""))
   {
      num = 0;
      do
      {
         num = G_FindTarget(num, name);
         if(!num)
         {
            break;
         }

         ent = G_GetEntity(num);
         ent->PostEvent(EV_Remove, 0);
      }
      while(1);
   }

   //
   // fire targets
   //
   name = Target();
   if(name && strcmp(name, ""))
   {
      num = 0;
      do
      {
         num = G_FindTarget(num, name);
         if(!num)
         {
            break;
         }

         ent = G_GetEntity(num);

         event = new Event(EV_Activate);
         event->AddEntity(attacker);
         ent->ProcessEvent(event);
      }
      while(1);
   }

   PostEvent(EV_Remove, 0);
}

/*****************************************************************************/
/*SINED func_throwobject (0 .5 .8) (0 0 0) (0 0 0)

This is an object you can pickup and throw at people
/*****************************************************************************/
CLASS_DECLARATION(Object, ThrowObject, "func_throwobject");

Event EV_ThrowObject_Pickup("pickup");
Event EV_ThrowObject_Throw("throw");
Event EV_ThrowObject_PickupOffset("pickupoffset");
Event EV_ThrowObject_ThrowSound("throwsound");

ResponseDef ThrowObject::Responses[] =
{
   { &EV_Touch,					      (Response)&ThrowObject::Touch },
   { &EV_ThrowObject_Pickup,	      (Response)&ThrowObject::Pickup },
   { &EV_ThrowObject_Throw,	      (Response)&ThrowObject::Throw },
   { &EV_ThrowObject_PickupOffset,	(Response)&ThrowObject::PickupOffset },
   { &EV_ThrowObject_ThrowSound,	   (Response)&ThrowObject::ThrowSound },
   { NULL, NULL }
};

void ThrowObject::PickupOffset(Event *ev)
{
   pickup_offset = edict->s.scale * ev->GetVector(1);
}

void ThrowObject::ThrowSound(Event *ev)
{
   throw_sound = ev->GetString(1);
}

void ThrowObject::Touch(Event *ev)
{
   Event * e;
   Entity * other;

   if(movetype != MOVETYPE_BOUNCE)
      return;

   other = ev->GetEntity(1);
   assert(other);

   if(other->isSubclassOf<Teleporter>())
   {
      return;
   }

   if(other->entnum == owner)
   {
      return;
   }

   if(throw_sound.length())
   {
      e = new Event(EV_StopEntitySound);
      ProcessEvent(e);
   }

   if(other->takedamage)
   {
      other->Damage(this, G_GetEntity(owner), size.length()*velocity.length() / 400, worldorigin, velocity, level.impact_trace.plane.normal, 32, 0, 
                    MOD_THROWNOBJECT, -1, -1, 1.0f);
   }
   Damage(this, this, max_health, worldorigin, velocity, level.impact_trace.plane.normal, 32, 0, MOD_THROWNOBJECT, -1, -1, 1);
}

void ThrowObject::Throw(Event *ev)
{
   Entity *owner;
   Entity *targetent;
   float  speed;
   float traveltime;
   float vertical_speed;
   Vector target, dir;
   float  grav;
   Vector xydir;
   Event * e;

   owner = ev->GetEntity(1);
   assert(owner);
   if(!owner)
      return;
   speed = ev->GetFloat(2);
   if(!speed)
      speed = 1;
   targetent = ev->GetEntity(3);
   assert(targetent);
   if(!targetent)
      return;
   if(ev->NumArgs() == 4)
      grav = ev->GetFloat(4);
   else
      grav = 1;

   e = new Event(EV_Detach);
   ProcessEvent(e);

   this->owner = owner->entnum;
   edict->owner = owner->edict;

   gravity = grav;

   target = targetent->worldorigin;
   target.z += targetent->viewheight;
   setMoveType(MOVETYPE_BOUNCE);
   setSolidType(SOLID_BBOX);
   edict->clipmask = MASK_PROJECTILE;

   dir = target - worldorigin;
   xydir = dir;
   xydir.z = 0;
   traveltime = xydir.length() / speed;
   vertical_speed = (dir.z / traveltime) + (0.5f * gravity * sv_gravity->value * traveltime);
   xydir.normalize();

   // setup ambient flying sound
   if(throw_sound.length())
   {
      e = new Event(EV_RandomEntitySound);
      e->AddString(throw_sound);
      ProcessEvent(e);
   }

   velocity = speed * xydir;
   velocity.z = vertical_speed;

   angles = velocity.toAngles();
   angles[PITCH] = -angles[PITCH];
   setAngles(angles);

   avelocity.x = crandom() * 200;
   avelocity.y = crandom() * 200;
   takedamage = DAMAGE_YES;

}

void ThrowObject::Pickup(Event *ev)
{
   Entity * ent;
   Event * e;
   str bone;

   ent = ev->GetEntity(1);
   assert(ent);
   if(!ent)
      return;
   bone = ev->GetString(2);

   e = new Event(EV_Attach);
   e->AddEntity(ent);
   e->AddString(bone);
   setOrigin(pickup_offset);
   ProcessEvent(e);
   edict->s.renderfx &= ~RF_FRAMELERP;
}

//
// Barrel with fire coming out of it
//
CLASS_DECLARATION(Object, FireBarrel, "world_firebarrel");

ResponseDef FireBarrel::Responses[] =
{
   { NULL, NULL }
};

FireBarrel::FireBarrel() : Object()
{
   Vector offset;

   fire = new FireSprite();
   // put the fire 3/4 of the way up the barrel
   offset[2] = (3 * size[2]) / 4;
   fire->setOrigin(worldorigin + offset);
   fire->setScale(edict->s.scale);
}

FireBarrel::~FireBarrel()
{
   fire->PostEvent(EV_Remove, 0);
   fire = NULL;
}

// EOF

