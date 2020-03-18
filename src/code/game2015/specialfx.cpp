//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/specialfx.cpp                    $
// $Revision:: 40                                                             $
//   $Author:: Markd                                                          $
//     $Date:: 11/15/98 11:33p                                                $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Special fx
// 

#include "g_local.h"
#include "entity.h"
#include "trigger.h"
#include "explosion.h"
#include "misc.h"
#include "light.h"
#include "specialfx.h"
#include "player.h"
#include "g_utils.h"
#include "surface.h"

/*
================
SpawnBlastDamage
================
*/
void SpawnBlastDamage(trace_t *trace, int damage, Entity *attacker)
{
   assert(trace);
   assert(attacker);
   if(!trace)
   {
      return;
   }

   if(!attacker)
   {
      attacker = world;
   }

   surfaceManager.DamageSurface(trace, damage, attacker);
   //### don't make the blast marks in DM to save on net bandwidth
   //if(trace->ent && trace->ent->entity && trace->ent->entity->flags & FL_BLASTMARK)
   if(!deathmatch->value && trace->ent && trace->ent->entity && trace->ent->entity->flags & FL_BLASTMARK)
   {
      auto blastmark = new Entity();
      blastmark->setMoveType(MOVETYPE_NONE);
      blastmark->setSolidType(SOLID_NOT);
      blastmark->setModel("sprites/blastmark.spr");
      blastmark->setSize({ 0, 0, 0 }, { 0, 0, 0 });
      blastmark->edict->s.scale = G_Random() + 0.7;

      Vector norm = trace->plane.normal;
      norm.x = -norm.x;
      norm.y = -norm.y;
      blastmark->angles = norm.toAngles();
      blastmark->angles.z = G_Random(360);
      blastmark->setAngles(blastmark->angles);
      blastmark->setOrigin(Vector(trace->endpos) + (Vector(trace->plane.normal) * 0.2));

      blastmark->PostEvent(EV_FadeOut, 5);
   }
}

/*
================
Particles
================
*/
void Particles(Vector org, Vector norm, int count, int lightstyle, int flags)
{
   if(count > 255)
      count = 255;
   
   gi.WriteByte(svc_temp_entity);
   gi.WriteByte(TE_PARTICLES);
   gi.WritePosition(org.vec3());
   gi.WriteDir(norm.vec3());
   gi.WriteByte(lightstyle);
   gi.WriteByte(count);
   gi.WriteByte(flags);
   gi.multicast(org.vec3(), MULTICAST_PVS);
}

/*
================
SpawnBlood
================
*/
void SpawnBlood(Vector org, Vector norm, int damage)
{
   if(damage > 150)
      damage = 150;
   
   Particles(org, norm, damage, 121, 0);
}

/*
================
SpawnSparks
================
*/
void SpawnSparks(Vector org, Vector norm, int count)
{
   Particles(org, norm, count, 122, PARTICLE_OVERBRIGHT);
}

/*
==============
SpawnPulseBeam
==============
*/
void SpawnBeam(Vector start, Vector end, int parent_entnum, int modelindex, float alpha, float life, int flags)
{
   byte sendflags;
   sendflags = 0;

   if(alpha != 1.0f)
      sendflags |= BM_ALPHA;
   if(life != 30)
      sendflags |= BM_LIFE;
   if(flags)
      sendflags |= BM_FLAGS;

   gi.WriteByte(svc_temp_entity);
   gi.WriteByte(TE_BEAM);
   gi.WriteByte(sendflags);
   gi.WritePosition(start.vec3());
   gi.WritePosition(end.vec3());
   gi.WriteShort(modelindex);

   if(sendflags & BM_ALPHA)
      gi.WriteByte(alpha * 255);
   if(sendflags & BM_FLAGS)
      gi.WriteByte(flags);
   if(sendflags & BM_LIFE)
      gi.WriteShort(life);

   gi.multicast(start.vec3(), MULTICAST_PVS);
}

/*
================
SpawnRocketExplosion
================
*/
void SpawnRocketExplosion(Vector org)
{
   gi.WriteByte(svc_temp_entity);
   gi.WriteByte(TE_ROCKET_EXPLOSION);
   gi.WritePosition(org.vec3());
   gi.multicast(org.vec3(), MULTICAST_PVS);
}

/*
================
SpawnScaledExplosion
================
*/
void SpawnScaledExplosion(Vector org, float scale)
{
   scale *= 64;
   if(scale > 255)
      scale = 255;
   else if(scale < 0)
      return;
   gi.WriteByte(svc_temp_entity);
   gi.WriteByte(TE_SCALED_EXPLOSION);
   gi.WritePosition(org.vec3());
   gi.WriteByte(scale);
   gi.multicast(org.vec3(), MULTICAST_PVS);
}

/*
================
SpawnTempDlight
================
*/
void SpawnTempDlight(Vector org, float r, float g, float b, float radius, float decay, float life)
{
   gi.WriteByte(svc_temp_entity);
   gi.WriteByte(TE_DLIGHT);
   gi.WritePosition(org.vec3());
   gi.WriteByte(r*255.0f);
   gi.WriteByte(g*255.0f);
   gi.WriteByte(b*255.0f);
   gi.WriteByte(radius / 4);
   gi.WriteByte(decay*255.0f);
   gi.WriteByte(life*25.5f);
   gi.multicast(org.vec3(), MULTICAST_PVS);
}

/*
===================
SpawnTeleportEffect
===================
*/
void SpawnTeleportEffect(Vector org, int lightstyle)
{
   gi.WriteByte(svc_temp_entity);
   gi.WriteByte(TE_TELEPORT_EFFECT);
   gi.WritePosition(org.vec3());
   gi.WriteByte(lightstyle);
   gi.multicast(org.vec3(), MULTICAST_PVS);
}

/*
================
BurnWall
================
*/
void BurnWall(Vector org, Vector end, int amount)
{
   gi.WriteByte(svc_temp_entity);
   gi.WriteByte(TE_BURNWALL);
   gi.WritePosition(org.vec3());
   gi.WritePosition(end.vec3());
   gi.WriteByte(amount);
   gi.multicast(org.vec3(), MULTICAST_ALL);
}

/*
================
TempModel
================
*/
void TempModel(Entity * parent, Vector origin, Vector angles, const char * modelname, int anim, float scale, float alpha, int flags, float life)
{
   byte sendflags;
   int  modelindex;
   sendflags = 0;

   // encode sendflags
   if(origin[0] || origin[1] || origin[2])
      sendflags |= TM_ORIGIN;
   if(angles[0] || angles[1] || angles[2])
      sendflags |= TM_ANGLES;
   if(anim)
      sendflags |= TM_ANIM;
   if(scale != 1.0f)
      sendflags |= TM_SCALE;
   if(flags != 0)
      sendflags |= TM_FLAGS;
   if(life != 0.1f)
      sendflags |= TM_LIFE;
   if(parent)
      sendflags |= TM_OWNER;
   if(alpha != 1.0f)
      sendflags |= TM_ALPHA;

   modelindex = gi.modelindex(modelname);
   //
   // send the stuff out
   //
   gi.WriteByte(svc_temp_entity);
   gi.WriteByte(TE_TEMPMODEL);
   gi.WriteByte(sendflags);
   if(sendflags & TM_ORIGIN)
      gi.WritePosition(origin.vec3());
   if(sendflags & TM_ANGLES)
      gi.WritePosition(angles.vec3());
   if(sendflags & TM_ANIM)
      gi.WriteByte(anim);
   if(sendflags & TM_SCALE)
      gi.WriteShort(scale * 256);
   if(sendflags & TM_ALPHA)
      gi.WriteByte(alpha * 255);
   if(sendflags & TM_FLAGS)
      gi.WriteByte(flags);
   if(sendflags & TM_OWNER)
      gi.WriteShort(parent->entnum);
   if(sendflags & TM_LIFE)
      gi.WriteShort(life*1000.0f);
   gi.WriteShort(modelindex);
   if(parent)
   {
      gi.multicast(parent->worldorigin.vec3(), MULTICAST_ALL);
   }
   else
   {
      gi.multicast(origin.vec3(), MULTICAST_ALL);
   }
}

/*
================
TesselateModel
================
*/
void TesselateModel(Entity * ent, int min_size, int max_size, Vector dir, float power, float percentage, 
                    int thickness, Vector origin, int type, int lightstyle)
{
   short sendflags;

   sendflags = 0;

   if(power > 255) power = 255;
   if(power < 0) power = 0;
   if(thickness > 255) thickness = 255;
   if(thickness < 0) thickness = 0;
   if(percentage < 0) percentage = 0;
   if(percentage > 1) percentage = 1;
   if(min_size > 255) min_size = 255;
   if(min_size < 0) min_size = 0;
   if(max_size > 255) max_size = 255;
   if(max_size < 0) max_size = 0;

   // encode sendflags
   if(origin[0] || origin[1] || origin[2])
      sendflags |= TESS_ORIGIN;
   if(dir[0] || dir[1] || dir[2])
      sendflags |= TESS_DIR;
   if(ent && ent->edict)
      sendflags |= TESS_ENTNUM;
   if(min_size != TESS_DEFAULT_MIN_SIZE)
      sendflags |= TESS_MINSIZE;
   if(max_size != TESS_DEFAULT_MAX_SIZE)
      sendflags |= TESS_MAXSIZE;
   if(power != TESS_DEFAULT_POWER)
      sendflags |= TESS_POWER;
   if(percentage != TESS_DEFAULT_PERCENT)
      sendflags |= TESS_PERCENT;
   if(thickness != min_size)
      sendflags |= TESS_THICK;
   if(lightstyle != TESS_DEFAULT_LIGHTSTYLE)
      sendflags |= TESS_LIGHTSTYLE;
   if(type != TESS_DEFAULT_TYPE)
      sendflags |= TESS_TYPE;
   
   //
   // send the stuff out
   //
   gi.WriteByte(svc_temp_entity);
   gi.WriteByte(TE_TESSELATE);
   gi.WriteShort(sendflags);
   if(sendflags & TESS_ORIGIN)
      gi.WritePosition(origin.vec3());
   if(sendflags & TESS_DIR)
      gi.WriteDir(dir.vec3());
   if(sendflags & TESS_ENTNUM)
      gi.WriteShort(ent->entnum);
   if(sendflags & TESS_MINSIZE)
      gi.WriteByte(min_size);
   if(sendflags & TESS_MAXSIZE)
      gi.WriteByte(max_size);
   if(sendflags & TESS_POWER)
      gi.WriteByte(power);
   if(sendflags & TESS_PERCENT)
      gi.WriteByte(percentage * 255.0f);
   if(sendflags & TESS_THICK)
      gi.WriteByte(thickness);
   if(sendflags & TESS_LIGHTSTYLE)
      gi.WriteByte(lightstyle);
   if(sendflags & TESS_TYPE)
      gi.WriteByte(type);

   if(ent)
   {
      if(gi.IsModel(ent->edict->s.modelindex))
         gi.multicast(((ent->absmax + ent->absmin)*0.5).vec3(), MULTICAST_PVS);
      else
         gi.multicast(ent->worldorigin.vec3(), MULTICAST_ALL);
   }
   else
   {
      gi.multicast(origin.vec3(), MULTICAST_ALL);
   }
}

CLASS_DECLARATION(Entity, Bubble, nullptr)

Event EV_Bubble_Think("think");

ResponseDef Bubble::Responses[] =
{
   { &EV_Bubble_Think,	(Response)&Bubble::Think },
   //{ &EV_Touch,			( Response )&Bubble::Touch },
   { nullptr, nullptr }
};

EXPORT_FROM_DLL void Bubble::Touch(Event *ev)
{
   edict->s.scale *= 0.8f;
   velocity[0] = G_CRandom(30);
   velocity[1] = G_CRandom(30);
   velocity[2] = -G_Random(30);
}

EXPORT_FROM_DLL void Bubble::Think(Event *ev)
{
   velocity[0] *= 0.8f;
   velocity[1] *= 0.8f;
   velocity[2] += 10;

   if((edict->s.scale > 0.2f) || (edict->s.scale < 0.02f) || !waterlevel)
   {
      PostEvent(EV_Remove, 0);
      return;
   }

   PostEvent(EV_Bubble_Think, 0.1);
}

void Bubble::Setup(Vector pos)
{
   setSolidType(SOLID_NOT);
   setMoveType(MOVETYPE_FLY);

   setModel("sprites/bubble.spr");
   setOrigin(pos);
   worldorigin.copyTo(edict->s.old_origin);
   mass = 0;

   watertype = gi.pointcontents(worldorigin.vec3());

   PostEvent(EV_Bubble_Think, 0.1);

   velocity[0] = G_CRandom(30);
   velocity[1] = G_CRandom(30);
   velocity[2] = -G_Random(30);

   edict->s.scale = 0.05f + G_CRandom(0.04f);
}

CLASS_DECLARATION(ScriptSlave, FuncBeam, "func_beam");

Event EV_FuncBeam_Activate("activate");
Event EV_FuncBeam_Deactivate("deactivate");
Event EV_FuncBeam_Diameter("diameter");
Event EV_FuncBeam_Lightstyle("beamstyle");
Event EV_FuncBeam_Maxoffset("maxoffset");
Event EV_FuncBeam_Minoffset("minoffset");
Event EV_FuncBeam_Color("color");
Event EV_FuncBeam_SetTarget("settarget");

ResponseDef FuncBeam::Responses[] =
{
   { &EV_Activate,               (Response)&FuncBeam::Activate },
   { &EV_FuncBeam_Activate,      (Response)&FuncBeam::Activate },
   { &EV_FuncBeam_Deactivate,    (Response)&FuncBeam::Deactivate },
   { &EV_FuncBeam_Diameter,      (Response)&FuncBeam::SetDiameter },
   { &EV_FuncBeam_Lightstyle,    (Response)&FuncBeam::SetLightstyle },
   { &EV_FuncBeam_Maxoffset,     (Response)&FuncBeam::SetMaxoffset },
   { &EV_FuncBeam_Minoffset,     (Response)&FuncBeam::SetMinoffset },
   { &EV_FuncBeam_Color,         (Response)&FuncBeam::SetColor },
   { &EV_FuncBeam_SetTarget,     (Response)&FuncBeam::SetTarget },
   { nullptr, nullptr }
};

void FuncBeam::SetColor(Event *ev)
{
   Vector color = ev->GetVector(1);
   edict->s.color_r = color[0];
   edict->s.color_g = color[1];
   edict->s.color_b = color[2];
}

void FuncBeam::SetMaxoffset(Event *ev)
{
   edict->s.frame = ev->GetFloat(1);
}

void FuncBeam::SetMinoffset(Event *ev)
{
   edict->s.radius = ev->GetFloat(1);
}

void FuncBeam::SetDiameter(Event *ev)
{
   edict->s.frame = ev->GetFloat(1);
}

void FuncBeam::SetLightstyle(Event *ev)
{
   edict->s.skinnum = ev->GetInteger(1);
}

void FuncBeam::Deactivate(Event *ev)
{
   hideModel();

   // Cancel all of the events to activate
   CancelEventsOfType(EV_FuncBeam_Activate);
   CancelEventsOfType(EV_Activate);
}

void FuncBeam::Shoot(Vector start, Vector end, int radius)
{
   trace_t  trace;
   Vector   dir;
   Vector   b1 = vec_zero, b2 = vec_zero;

   dir = end - start;

   if(edict->s.effects & EF_BEAMEFFECT)
   {
      b1 = Vector(-radius, -radius, -radius);
      b2 = Vector(radius, radius, radius);
   }

   trace = G_FullTrace(start, b1, b2, end, radius, this, MASK_SHOT, "FuncBeam::Activate");

   if(trace.ent->entity && trace.ent->entity->takedamage)
   {
      if(trace.intersect.valid)
      {
         // We hit a valid group so send in location based damage
         trace.ent->entity->Damage(
            this,
            this,
            damage,
            trace.endpos,
            dir,
            trace.plane.normal,
            0,
            0,
            MOD_BEAM,
            trace.intersect.parentgroup,
            -1,
            trace.intersect.damage_multiplier);
      }
      else
      {
         // We didn't hit any groups, so send in generic damage
         trace.ent->entity->Damage(
            this,
            this,
            damage,
            trace.endpos,
            dir,
            trace.plane.normal,
            0,
            0,
            MOD_BEAM,
            -1,
            -1,
            1);
      }
   }
}

void FuncBeam::Activate(Event *ev)
{
   Vector      forward, endpoint;
   trace_t     trace;

   showModel();

   // An entity is targeted
   if(end)
   {
      VectorCopy(end->origin, edict->s.old_origin);
   }
   else
   {
      worldangles.AngleVectors(&forward, nullptr, nullptr);
      endpoint = forward * 8192;
      trace = G_Trace(origin, vec_zero, vec_zero, endpoint, this, MASK_SOLID, "FuncBeam::Activate");
      VectorCopy(trace.endpos, edict->s.old_origin);
   }

   if(damage)
      Shoot(origin, edict->s.old_origin, edict->s.frame);

   // If life is set, then post a deactivate message
   if(life > 0 && !EventPending(EV_FuncBeam_Deactivate))
   {
      PostEvent(EV_FuncBeam_Deactivate, life);
      return;
   }

   // Shoot beam and cause damage
   PostEvent(EV_Activate, 0.1);
}

void FuncBeam::SetTarget(Event *ev)
{
   const char  *target;
   int         num;

   // Set the end position if there is a target set.
   target = Target();
   if(target && strlen(target))
   {
      if((num = G_FindTarget(0, target)))
      {
         end = G_GetEntity(num);
         assert(end);
         VectorCopy(end->origin.vec3(), edict->s.old_origin);
      }
   }
}

/*****************************************************************************/
/*SINED func_beam (0 .8 .5) (-8 -8 -8) (8 8 8) START_ON ANIMATE FAST ROLL RANDSTART ELECTRIC RANDALPHA 

This creates a beam effect from the origin to the target's origin. If no
target is specified, uses angles and projects beam out from there.

"model" Specifies the model to use as the beam
"overlap" Specifies the amount of overlap each beam link should have. Use this to fill 
in the cracks when using electric on beams. (Default is 0)
"minoffset" Amount of electrical variation at the endpoints of beam (Default is 5)
"maxoffset" Amount of electrical variation in the middle of beam (Default is 25)
"color" Vector specifiying the red,green, and blue components. (Default is "1 1 1")
"alpha" Alpha of the beam (Default is 1.0)
"damage" Amount of damage the beam inflicts if beam hits someone (Default is 0)
"angles" Sets the angle of the beam if no target is specified. 
"life" Deactivates the beam life seconds after the beam is activated.  If not set, beam
will not be deactivated.
"beamstyle" Sets the style for this beam to cycle through.  This applies only
to beams without models. sample styles (121 blood, 120 gunsmoke, 123 orangeglow, 124 blueyellow, 
125 debris trail, 128 oil, 129 waterspray 130 blue-yellow-blue) See global0.scr for 
more style numbers

START_ON  - Starts the beam on
ANIMATE   - Plays animation of the model
FAST      - Plays the animation at 20 frames/sec
ROLL      - Rolls the beam
RANDSTART - Starts each segment of the beam's animation on a different frame
ELECTRIC  - Applies a random electric efffect to the beam
RANDALPHA - Randomly generate the alpha for the beam

If the model field is not set, then a straight beam will be created using the color
specified. color, minoffset, maxoffset ,ANIMATE, FAST, ROLL, RANDSTART, 
ELECTRIC, and RANDALPHA are not applicable if the model is not set.

/*****************************************************************************/

FuncBeam::FuncBeam() : ScriptSlave()
{
   Vector      color;
   const char  *target;
   trace_t     trace;
   Vector      endpoint, forward;
   const char  *model = 0;
   qboolean    setangles;

   setSolidType(SOLID_NOT);
   setOrigin(origin);

   end = nullptr;

   target = Target();

   if(target)
      PostEvent(EV_FuncBeam_SetTarget, 0);
   else
      // No target specified, so use angles to get direction and do a trace to get endpoint.
   {
      setangles = (G_GetSpawnArg("angle") || G_GetSpawnArg("angles"));
      if(setangles)
      {
         float angle;
         angle = G_GetFloatArg("angle", 0);
         angles = G_GetVectorArg("angles", Vector(0, angle, 0));

         setAngles(angles);
         angles.AngleVectors(&forward, nullptr, nullptr);
         setAngles(angles);
         endpoint = forward * 8192;
         trace = G_Trace(origin, vec_zero, vec_zero, endpoint, this, MASK_SOLID, "FuncBeam");
         VectorCopy(trace.endpos, edict->s.old_origin);
      }
      else // Nothing specified. Remove this thing
      {
         gi.dprintf("No target or angles set on FuncBeam.\n");
         PostEvent(EV_Remove, 0);
         return;
      }
   }

   // Check for a model and use the beam models if it is set
   model = G_GetSpawnArg("model");
   if(model && strlen(model))
   {
      setModel(model);
      edict->s.effects |= EF_BEAMEFFECT;
      edict->s.frame = G_GetFloatArg("maxoffset", 25);
      edict->s.radius = G_GetFloatArg("minoffset", 5);
      edict->s.lightofs = G_GetIntArg("overlap", 0);
      // skinnum will hold the flags
      edict->s.skinnum = 0;
      if(spawnflags & 0x0002)
         edict->s.skinnum |= BEAM_AUTO_ANIMATE;
      if(spawnflags & 0x0004)
         edict->s.skinnum |= BEAM_ANIMATE_FAST;
      if(spawnflags & 0x0008)
         edict->s.skinnum |= BEAM_ROLL;
      if(spawnflags & 0x0010)
         edict->s.skinnum |= BEAM_ANIMATE_RANDOM_START;
      if(spawnflags & 0x0020)
         edict->s.skinnum |= BEAM_LIGHTNING_EFFECT;
      if(spawnflags & 0x0040)
         edict->s.skinnum |= BEAM_RANDOM_ALPHA;
   }
   else // Otherwise do a Quake2 style beam
   {
      edict->s.renderfx |= RF_BEAM;
      edict->s.frame = G_GetIntArg("diameter", 4);
      edict->s.skinnum = G_GetIntArg("beamstyle", 255);;
      color = G_GetVectorArg("color", Vector(1, 1, 1));
      edict->s.color_r = color.x;
      edict->s.color_g = color.y;
      edict->s.color_b = color.z;
      edict->s.modelindex = 1;	// must be non-zero
   }

   edict->s.alpha = G_GetFloatArg("alpha", 1);
   damage = G_GetFloatArg("damage", 0);
   life = G_GetFloatArg("life", 0);

   if(spawnflags & 0x0001) // Start On
   {
      PostEvent(EV_Activate, 0);
   }
   else
   {
      hideModel();
   }
}

CLASS_DECLARATION(Entity, Beam, nullptr);

ResponseDef Beam::Responses[] =
{
   { nullptr, nullptr }
};

Beam::Beam() : Entity()
{
   hideModel();
   edict->s.frame = 0;
   edict->s.skinnum = 0;

   setMoveType(MOVETYPE_NONE);
   setSolidType(SOLID_NOT);

   edict->s.renderfx |= RF_BEAM;
   edict->s.modelindex = 1;			// must be non-zero
}

void Beam::setBeam(Vector startpos, Vector endpos, int diameter, float r, float g, float b, float alpha, float lifespan)
{
   showModel();

   if(lifespan)
   {
      PostEvent(EV_Remove, lifespan);
   }

   start = startpos;
   end = endpos;

   setOrigin(start);

   edict->s.old_origin[0] = end[0];
   edict->s.old_origin[1] = end[1];
   edict->s.old_origin[2] = end[2];

   edict->s.frame = diameter;
   edict->s.skinnum = 255;

   edict->s.color_r = r;
   edict->s.color_g = g;
   edict->s.color_b = b;
   edict->s.alpha = alpha;

   link();
}

CLASS_DECLARATION(Entity, Projectile, nullptr);

ResponseDef Projectile::Responses[] =
{
   { nullptr, nullptr }
};

Projectile::Projectile() : Entity()
{
   flags |= FL_FATPROJECTILE;
}

void Projectile::Setup(Entity *owner, Vector pos, Vector dir)
{
   // This should never be called, but just in case.
   assert(0);
   PostEvent(EV_Remove, 0);
}

//### return pointer to projectile's owner
Entity *Projectile::Owner()
{
   return G_GetEntity(owner);
}
//###

/*****************************************************************************/
/*SINED fx_fire (0 .8 .5) (-8 -8 -8) (8 8 8) SINGLE STAR PARALLEL

This creates a fire sprite, it defaults to the cross shape

SINGLE - Only one sprite orientated
STAR - 4 sprites in a star shape

If you want flickering

Set style to the identifier contained in groupname in the surfaces to control.

"on_style" light style to set to when "on" (default is a flicker)
"off_style" light style to set to when "off" (default is "a")

/*****************************************************************************/

#define SINGLE    (1<<0)
#define STAR      (1<<1)
#define PARALLEL  (1<<2)
CLASS_DECLARATION( Light, FireSprite, "fx_fire" )

ResponseDef FireSprite::Responses[] =
{
   { nullptr, nullptr }
};

FireSprite::FireSprite() : Light()
{
   if(on_style == str("m"))
      on_style = str("kkllmmnnmmllkkonnmmkkllonnmmkkllnnoonnmmllkkonnnnmmllkko");

   ProcessEvent(EV_Light_TurnOn);
   setMoveType(MOVETYPE_NONE);
   setSolidType(SOLID_NOT);
   showModel();
   edict->svflags &= ~SVF_NOCLIENT;

   edict->s.effects |= EF_AUTO_ANIMATE;
   if(spawnflags & SINGLE)
   {
      setModel("sprites/fire.spr");
   }
   else if(spawnflags & STAR)
   {
      setModel("sprites/fire_star.spr");
   }
   else if(spawnflags & PARALLEL)
   {
      setModel("sprites/fire_single.spr");
   }
   else
   {
      setModel("sprites/fire_cross.spr");
   }

   RandomGlobalEntitySound("snd_fire");
}

FireSprite::~FireSprite()
{
   ProcessEvent(EV_Light_TurnOff);
   detach();
   //
   // spawn an explosion
   //
   if(world)
      CreateExplosion(worldorigin, 100, edict->s.scale * 0.5f, true, this, this, this);
}

CLASS_DECLARATION(Trigger, Sprite, "fx_sprite")

Event EV_Sprite_Activate("activate");
Event EV_Sprite_Deactivate("deactivate");

ResponseDef Sprite::Responses[] =
{
   { &EV_Activate,               (Response)&FuncBeam::Activate },
   { &EV_Sprite_Activate,        (Response)&FuncBeam::Activate },
   { &EV_Sprite_Deactivate,      (Response)&FuncBeam::Deactivate },
   { nullptr, nullptr }
};

void Sprite::Activate(Event *ev)
{
   showModel();
}

void Sprite::Deactivate(Event *ev)
{
   hideModel();
}

Sprite::Sprite() : Trigger()
{
   str         model;
   qboolean    setangles;
   Vector      color;

   model = G_GetStringArg("sprite", "");
   edict->s.frame = G_GetIntArg("frame", 0);
   edict->s.scale = G_GetFloatArg("scale", 1.0);
   edict->s.alpha = G_GetFloatArg("alpha", 1.0);
   color = G_GetVectorArg("color", Vector(1, 1, 1));
   edict->s.color_r = color.x;
   edict->s.color_g = color.y;
   edict->s.color_b = color.z;

   if(!model.length())
   {
      gi.dprintf("fx_sprite: sprite not defined, removing.\n");
      PostEvent(EV_Remove, 0);
   }

   setModel(model.c_str());
   setMoveType(MOVETYPE_NONE);
   setSolidType(SOLID_NOT);
   showModel();
   edict->svflags &= ~SVF_NOCLIENT;

   if(spawnflags & 1)
      edict->s.effects |= EF_AUTO_ANIMATE;

   setangles = (G_GetSpawnArg("angle") || G_GetSpawnArg("angles"));

   if(setangles)
   {
      float angle;

      angle = G_GetFloatArg("angle", 0);
      setAngles(G_GetVectorArg("angles", Vector(0, angle, 0)));
   }

   // If trace to surface, apply sprite and remove this.
   if(spawnflags & 2)
   {
      Vector   dir, end, norm;
      trace_t  trace;
      Entity   *splat;

      dir = G_GetMovedir();
      end = worldorigin + dir * 8192;

      trace = G_Trace(worldorigin, vec_zero, vec_zero, end, this, MASK_SOLIDNONFENCE, "Sprite::Sprite");

      if(HitSky(&trace) || (trace.ent->solid != SOLID_BSP))
      {
         PostEvent(EV_Remove, 0);
         return;
      }

      splat = new Entity();
      splat->setMoveType(MOVETYPE_NONE);
      splat->setSolidType(SOLID_NOT);
      splat->setModel(model.c_str());
      splat->edict->s.frame = edict->s.frame;
      splat->edict->s.scale = edict->s.scale;
      splat->edict->s.alpha = edict->s.alpha;
      splat->edict->s.color_r = edict->s.color_r;
      splat->edict->s.color_g = edict->s.color_g;
      splat->edict->s.color_b = edict->s.color_b;

      if(spawnflags & 1)
         splat->edict->s.effects |= EF_AUTO_ANIMATE;

      norm = trace.plane.normal;
      norm.x = -norm.x;
      norm.y = -norm.y;

      splat->angles = norm.toAngles();
      splat->setAngles(splat->angles);
      splat->setOrigin(Vector(trace.endpos) + (Vector(trace.plane.normal) * 0.2));

      PostEvent(EV_Remove, 0.1f);
   }
}

void ChangeMusic(const char *current, const char *fallback, qboolean force)
{
   int      j;
   edict_t	*other;

   if(current || fallback)
   {
      for(j = 1; j <= game.maxclients; j++)
      {
         other = &g_edicts[j];
         if(other->inuse && other->client)
         {
            Player *client;
            client = (Player *)other->entity;
            client->ChangeMusic(current, fallback, force);
         }
      }
   }
}

void ChangeSoundtrack(const char * soundtrack)
{
   gi.configstring(CS_SOUNDTRACK, soundtrack);
}

//
//### 2015 specialfx
//

/*
================
SpawnThrowerFlame
================
*/
void SpawnThrowerFlame(Vector org, Vector dest)
{
   gi.WriteByte(svc_temp_entity);
   gi.WriteByte(TE_FLAMETHROWER);
   gi.WritePosition(org.vec3());

   if(org == dest)
   {
      gi.WriteByte(0);
      gi.WriteByte(0);
      gi.WriteByte(0);
   }
   else
   {
      for(int i = 0; i < 3; i++)
      {
         int j = (dest[i] - org[i]) * 0.5 + 128;
         if(j > 255)
            j = 255;
         else if(j < 0)
            j = 0;
         gi.WriteByte(j);
      }
   }

   gi.multicast(org.vec3(), MULTICAST_PVS);
}

/*
================
SpawnThrowerFlameRow
================
*/
void SpawnThrowerFlameRow(Vector org, Vector dest, Vector org2, Vector dest2)
{
   gi.WriteByte(svc_temp_entity);
   gi.WriteByte(TE_FLAMETHROWERROW);
   gi.WritePosition(org.vec3());

   // write out first destination offset
   if(org == dest)
   {
      gi.WriteByte(0);
      gi.WriteByte(0);
      gi.WriteByte(0);
   }
   else
   {
      for(int i = 0; i < 3; i++)
      {
         int j = (dest[i] - org[i]) * 0.5 + 128;
         if(j > 255)
            j = 255;
         else if(j < 0)
            j = 0;
         gi.WriteByte(j);
      }
   }

   // write out second start position offset
   if(org2 == org)
   {
      gi.WriteByte(0);
      gi.WriteByte(0);
      gi.WriteByte(0);
   }
   else
   {
      for(int i = 0; i < 3; i++)
      {
         int j = (org2[i] - org[i]) * 0.5 + 128;
         if(j > 255)
            j = 255;
         else if(j < 0)
            j = 0;
         gi.WriteByte(j);
      }
   }

   // write out second destination offset
   if(dest2 == org2)
   {
      gi.WriteByte(0);
      gi.WriteByte(0);
      gi.WriteByte(0);
   }
   else
   {
      for(int i = 0; i < 3; i++)
      {
         int j = (dest2[i] - org2[i])*0.5 + 128;
         if(j > 255)
            j = 255;
         else if(j < 0)
            j = 0;
         gi.WriteByte(j);
      }
   }

   gi.multicast(org.vec3(), MULTICAST_PVS);
}

/*
================
SpawnThrowerFlameHit
================
*/
void SpawnThrowerFlameHit(Vector org, Vector dir)
{
   gi.WriteByte(svc_temp_entity);
   gi.WriteByte(TE_FLAMETHROWERHIT);
   gi.WritePosition(org.vec3());
   gi.WriteDir(dir.vec3());
   gi.multicast(org.vec3(), MULTICAST_PVS);
}

/*
================
SpawnFlame
================
*/
void SpawnFlame(Vector org, Vector dira, Vector dest, Vector dirb, int count, int style, int size)
{
   gi.WriteByte(svc_temp_entity);
   gi.WriteByte(TE_FLAME);
   gi.WritePosition(org.vec3());

   if(org == dest)
   {
      gi.WriteByte(128);
      gi.WriteByte(128);
      gi.WriteByte(128);
   }
   else
   {
      for(int i = 0; i < 3; i++)
      {
         int j = (dest[i] - org[i])*0.5 + 128;
         if(j > 255)
            j = 255;
         else if(j < 0)
            j = 0;
         gi.WriteByte(j);
      }
   }

   Vector direction = dira;
   int    speed     = direction.normalize2() * 0.5;
   if(speed > 255)
      speed = 255;
   gi.WriteDir(direction.vec3());
   gi.WriteByte(speed);

   direction = dirb;
   speed = direction.normalize2() * 0.5;
   if(speed > 255)
      speed = 255;
   gi.WriteDir(direction.vec3());
   gi.WriteByte(speed);

   gi.WriteByte(style);
   if(count > 255)
      count = 255;
   gi.WriteByte(count);
   if(size > 255)
      size = 255;
   gi.WriteByte(size);
   gi.multicast(org.vec3(), MULTICAST_PVS);
}

/*
================
SpawnHoverBoost
================
*/
void SpawnHoverBoost(Vector org, int yaw)
{
   if(yaw < 0)
      yaw += 360;
   if(yaw > 360)
      yaw -= 360;
   yaw /= 2;

   gi.WriteByte(svc_temp_entity);
   gi.WriteByte(TE_HOVERBOOSTER);
   gi.WritePosition(org.vec3());
   gi.WriteByte(yaw);
   gi.multicast(org.vec3(), MULTICAST_PVS);
}

/*
================
SpawnNukeExplosion
================
*/
void SpawnNukeExplosion(Vector org, int size, int lightstyle)
{
   if(size < 0)
      size = 0;
   if(size > 2040)
      size = 2040;
   size *= 0.125;

   gi.WriteByte(svc_temp_entity);
   gi.WriteByte(TE_NUKE_EXPLOSION);
   gi.WritePosition(org.vec3());
   gi.WriteByte(size);
   gi.WriteByte(lightstyle);
   gi.multicast(org.vec3(), MULTICAST_PVS);
}

/*
================
SpawnBowExplosion
================
*/
void SpawnBowExplosion(Vector org, int size)
{
   if(size < 0)
      size = 0;
   if(size > 2040)
      size = 2040;
   size *= 0.125;

   gi.WriteByte(svc_temp_entity);
   gi.WriteByte(TE_BOW_EXPLOSION);
   gi.WritePosition(org.vec3());
   gi.WriteByte(size);
   gi.multicast(org.vec3(), MULTICAST_PVS);
}

/*
================
SizedParticles
================
*/
void SizedParticles(Vector org, Vector norm, int count, int lightstyle, int flags, int size, int growrate)
{
   if(count > 255)
      count = 255;
   if(size > 255)
      size = 255;

   growrate += 128;
   if(growrate < 0)
      growrate = 0;
   else if(growrate > 255)
      growrate = 255;

   gi.WriteByte(svc_temp_entity);
   gi.WriteByte(TE_SIZED_PARTICLES);
   gi.WritePosition(org.vec3());
   gi.WriteDir(norm.vec3());
   gi.WriteByte(lightstyle);
   gi.WriteByte(count);
   gi.WriteByte(flags);
   gi.WriteByte(size);
   gi.WriteByte(growrate);
   gi.multicast(org.vec3(), MULTICAST_PVS);
}

/*
================
FullParticles
================
*/
void FullParticles(Vector org, Vector norm, int count, int lightstyle, int flags, int size, int growrate, int speed, int spread, Vector accel)
{
   if(count > 255)
      count = 255;
   if(size > 255)
      size = 255;

   growrate += 128;
   if(growrate < 0)
      growrate = 0;
   else if(growrate > 255)
      growrate = 255;

   speed *= 0.5;
   if(speed < 0)
      speed = 0;
   else if(speed > 255)
      speed = 255;

   spread *= 0.5;
   if(spread < 0)
      spread = 0;
   else if(spread > 255)
      spread = 255;

   gi.WriteByte(svc_temp_entity);
   gi.WriteByte(TE_PARTICLES_FULL);
   gi.WritePosition(org.vec3());
   gi.WriteDir(norm.vec3());
   gi.WriteByte(lightstyle);
   gi.WriteByte(count);
   gi.WriteByte(flags);
   gi.WriteByte(size);
   gi.WriteByte(growrate);
   gi.WriteByte(speed);
   gi.WriteByte(spread);

   for(int i = 0; i < 3; i++)
   {
      int a = accel[i] + 128;
      if(a < 0)
         a = 0;
      else if(a > 255)
         a = 255;
      gi.WriteByte(a);
   }

   gi.multicast(org.vec3(), MULTICAST_PVS);
}

CLASS_DECLARATION(Trigger, FireHurtField, "trigger_firehurtfield");

ResponseDef FireHurtField::Responses[] =
{
   { &EV_Activate,         (Response)&FireHurtField::SetHurt },
   { &EV_Trigger_Effect,   (Response)&FireHurtField::Hurt    },
   { nullptr, nullptr }
};

FireHurtField::FireHurtField() : TriggerUse()
{
   damage = 1;
   fireon = true;

   respondto = TRIGGER_PLAYERS | TRIGGER_MONSTERS;
}

void FireHurtField::SetHurt(Event *ev)
{
   if(ev->NumArgs() == 1)
   {
      int i = ev->GetInteger(1);

      if(i)
         fireon = true;
      else
         fireon = false;
   }
   else
   {
      if(fireon)
         fireon = false;
      else
         fireon = true;
   }
}

void FireHurtField::Hurt(Event *ev)
{
   if(!fireon)
      return;

   // check that parent is still there
   if(parentfire && !parentfire->edict->inuse)
   {
      PostEvent(EV_Remove, 0);
      return;
   }

   Entity *other = ev->GetEntity( 1 );
   if((damage != 0) && !other->deadflag && !(other->flags & FL_GODMODE))
   {
      other->Damage(this, other, damage, other->origin, vec_zero, vec_zero, 0, DAMAGE_NO_ARMOR | DAMAGE_NO_KNOCKBACK, MOD_FLAMETHROWER, -1, -1, 1.0f);
   }
}

void FireHurtField::Setup(Vector minpos, Vector maxpos, int firedamage)
{
   Vector newpos((minpos + maxpos) * 0.5);
   Vector newmin(minpos - newpos);
   Vector newmax(maxpos - newpos);

   setOrigin(newpos);
   setSize(newmin, newmax);
   damage = firedamage;
}

/*****************************************************************************/
/*SINED func_fire (0 0.5 0) (-4 -4 -4) (4 4 4) START_OFF NO_SOUND

Makes a triggerable particle flame. These flames do not cause any damage.
They can be positioned as a single spot of fire, or a row of fire.

START_OFF starts the flame entity in the off state

NO_DAMAGE makes it not create a damage field around it

count = specifies the density of the flame. Default is 1.

lightstyle = the lightstyle type of the flame. Valid values are from 150 to 159. Default is 151.

offset = a vector offset that flame are made to. Example: If you specified '64 0 0', then
flames would be made between the entity's origin and the point 64 away on the X-axis, resulting
in a solid row of flames. Default is '0 0 0'.

size = the size of the flame particles to make. Default is 20;

startdir = the velocity that the begining flames are given.Example: the default of '0 0 32' would give the flames
a velocity of 32 straight up.

enddir = the velocity that the ending flames are given.Example: the default '0 0 32' would give the flames
a velocity of 324 straight up.

rate = the time delay between each time the flame is updated. Default is 0.2

randomrate = the amount of random time delay there is for the flame updating. Default is 0;

damage = amount of damage the hurt field should do. default is 1

/*****************************************************************************/

CLASS_DECLARATION(TriggerPlaySound, FuncFire, "func_fire");

Event EV_Flame_Animate("flame_animate");

ResponseDef FuncFire::Responses[] =
{
   { &EV_Activate,      (Response)&FuncFire::ToggleFlame },
   { &EV_Flame_Animate, (Response)&FuncFire::Animate     },
   { nullptr, nullptr }
};

void FuncFire::ToggleFlame(Event *ev)
{
   if(flaming)
   {
      flaming = 0;
      CancelEventsOfType(EV_Flame_Animate);
   }
   else
   {
      flaming = 1;
      ProcessEvent(EV_Flame_Animate);
   }

   // put hurt field into proper state
   if(hurtfield)
   {
      auto e = new Event(EV_Activate);
      e->AddInteger(flaming);
      hurtfield->ProcessEvent(e);
   }
}

void FuncFire::Animate(Event *ev)
{
   float timer;

   SpawnFlame(origin, flamevela, flameend, flamevelb, flamecount, flametype, flamesize);

   if(flamerandom)
      timer = flamedelay + floor(G_Random(flamerandom))*0.1;
   else
      timer = flamedelay;

   PostEvent(EV_Flame_Animate, timer);
}

FuncFire::FuncFire() : Entity()
{
   // don't hang around in multiplayer
   if(deathmatch->value)
   {
      PostEvent(EV_Remove, 0);
      return;
   }

   setOrigin(origin);
   setSolidType(SOLID_NOT);
   setMoveType(MOVETYPE_NONE);
   hideModel();

   flamecount = G_GetIntArg("count", 1);
   flameend   = G_GetVectorArg("offset", vec_zero);
   flameend  += worldorigin;
   flamevela  = G_GetVectorArg("startdir", { 0, 0, 32 });
   flamevelb  = G_GetVectorArg("enddir",   { 0, 0, 32 });
   flamedelay = G_GetFloatArg("rate", 0.2);
   if(flamedelay <= 0)
      flamedelay = 0.1;

   flamerandom = G_GetFloatArg("randomrate", 0) * 10;
   if(flamerandom)
      flamerandom += 1;

   flametype = G_GetIntArg("lightstyle", 151);
   flamesize = G_GetIntArg("size", 20);
   if(flamesize < 1)
      flamesize = 1;

   flamedamage = G_GetIntArg("damage", 1);

   if(flamedamage > 0)
   {
      // set hurt field size
      Vector hurtmins = worldorigin + Vector(-1, -1, -1);
      Vector hurtmaxs = worldorigin + Vector(1, 1, 1);

      Vector tmpvec = worldorigin + Vector(flamesize, flamesize, flamesize) * 0.5;
      AddPointToBounds(tmpvec.vec3(), hurtmins.vec3(), hurtmaxs.vec3());
      tmpvec = worldorigin - Vector(flamesize, flamesize, flamesize) * 0.5;
      AddPointToBounds(tmpvec.vec3(), hurtmins.vec3(), hurtmaxs.vec3());

      tmpvec = flameend + Vector(flamesize, flamesize, flamesize) * 0.5;
      AddPointToBounds(tmpvec.vec3(), hurtmins.vec3(), hurtmaxs.vec3());
      tmpvec = flameend - Vector(flamesize, flamesize, flamesize) * 0.5;
      AddPointToBounds(tmpvec.vec3(), hurtmins.vec3(), hurtmaxs.vec3());

      // spawn hurt field
      hurtfield = new FireHurtField();
      hurtfield->Setup(hurtmins, hurtmaxs, flamedamage);
   }

   if(spawnflags & 1)
   {
      flaming = 0;
   }
   else
   {
      flaming = 1;
      PostEvent(EV_Flame_Animate, 0.3);
   }

   // put hurt field into proper state
   if(hurtfield)
   {
      auto e = new Event(EV_Activate);
      e->AddInteger(flaming);
      hurtfield->ProcessEvent(e);
   }
}

void SpawnCrawlerSpitDamage(trace_t *trace, int damage, Entity *attacker)
{
   assert(trace);
   assert(attacker);
   if(!trace)
   {
      return;
   }

   if(!attacker)
   {
      attacker = world;
   }

   surfaceManager.DamageSurface( trace, damage, attacker );
   if ( trace->ent && trace->ent->entity && trace->ent->entity->flags & FL_BLASTMARK )
   {
      auto blastmark = new Entity();
      blastmark->setMoveType( MOVETYPE_NONE );
      blastmark->setSolidType( SOLID_NOT );
      blastmark->setModel( "sprites/crawlerspitmark.spr" );
      blastmark->setSize(vec_zero,vec_zero);
      blastmark->edict->s.scale = G_Random(0.5) + 0.5;

      Vector norm(trace->plane.normal);
      norm.x = -norm.x;
      norm.y = -norm.y;
      blastmark->angles = norm.toAngles();
      blastmark->angles.z = G_Random(360);
      blastmark->setAngles(blastmark->angles);
      blastmark->setOrigin(Vector(trace->endpos) + (Vector(trace->plane.normal) * 0.2));

      blastmark->PostEvent(EV_FadeOut, 5);
   }
}

//
//### End 2015 specialfx
//

// EOF

