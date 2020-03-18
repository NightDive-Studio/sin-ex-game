//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/pulserifle.cpp                   $
// $Revision:: 69                                                             $
//   $Author:: Aldie                                                          $
//     $Date:: 3/02/99 9:14p                                                  $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Pulse rifle
// 

#include "g_local.h"
#include "bullet.h"
#include "pulserifle.h"
#include "specialfx.h"
#include "misc.h"
#include "explosion.h"
#include "surface.h"

#define PULSE_MODE 0

// Projectile portion of the pulse rifle

template class EXPORT_FROM_DLL SafePtr<Beam>;
typedef SafePtr<Beam> BeamPtr;

class EXPORT_FROM_DLL Pulse : public Projectile
{
public:
   CLASS_PROTOTYPE(Pulse);

   virtual void   Setup(Entity *owner, Vector pos, Vector dir) override;
   virtual void   Explode(Event *ev);
};

CLASS_DECLARATION(Projectile, Pulse, nullptr);

Event EV_Pulse_UpdateBeams("update_beams");
Event EV_Pulse_Remove("remove_beams");

ResponseDef Pulse::Responses[] =
{
   { &EV_Touch,                           (Response)&Pulse::Explode },
   { nullptr, nullptr }
};

void Pulse::Explode(Event *ev)
{
   Entity      *other;
   Entity      *owner;
   int         damg;
   Vector      v;
   Vector      norm;
   Vector      shockangles;
   Entity      *pulseexpl;

   other = ev->GetEntity(1);
   assert(other);

   if(other->isSubclassOf<Teleporter>())
   {
      return;
   }

   if(other->entnum == this->owner)
   {
      return;
   }

   owner = G_GetEntity(this->owner);

   if(!owner)
      owner = world;

   stopsound(CHAN_VOICE);
   setSolidType(SOLID_NOT);

   // Hit the shy, so remove everything
   if(HitSky())
   {
      PostEvent(EV_Remove, 0);
      return;
   }

   damg = 50 + (int)G_Random(25);

   // Single player packs a bigger punch
   if(!deathmatch->value && owner->isClient())
      damg *= 1.5;

   if(other->takedamage)
      other->Damage(this, owner, damg, worldorigin, velocity, level.impact_trace.plane.normal, 32, 0, MOD_PULSE, -1, -1, 1.0f);

   // Damage the surface
   surfaceManager.DamageSurface(&level.impact_trace, damg, owner);

   damg = 100;

   // Single player packs a bigger punch
   if(!deathmatch->value && owner->isClient())
      damg *= 1.5;

   // Do an explosion but don't hurt other, since he already took damage.
   RadiusDamage(this, owner, damg, other, MOD_PULSE);

   v = velocity;
   v.normalize();
   v = worldorigin - v * 36;

   norm = level.impact_trace.plane.normal;
   norm.x = -norm.x;
   norm.y = -norm.y;
   shockangles = norm.toAngles();
   shockangles.z = G_Random(360);

   // Start up the shockwave effect

   pulseexpl = new Entity();
   pulseexpl->setModel("pulseshock.def");
   pulseexpl->setOrigin(v);
   pulseexpl->worldorigin.copyTo(pulseexpl->edict->s.old_origin);
   pulseexpl->setAngles(shockangles);
   pulseexpl->setMoveType(MOVETYPE_NONE);
   pulseexpl->setSolidType(SOLID_NOT);
   pulseexpl->RandomAnimate("pulseexplode", NULL);
   pulseexpl->PostEvent(EV_Remove, 0.1f);
   FlashPlayers(v, 0.1, 0.1, 1, 0.5, 500);

   PostEvent(EV_Remove, 0);
}

void Pulse::Setup(Entity *owner, Vector pos, Vector dir)
{
   Event *ev1, *ev2;

   this->owner = owner->entnum;
   edict->owner = owner->edict;

   // Align the projectile
   angles = dir.toAngles();
   angles[PITCH] = -angles[PITCH];
   setAngles(angles);
   edict->s.angles[ROLL] = rand() % 360;

   // Flies like a rocket
   setMoveType(MOVETYPE_TOSS);
   setSolidType(SOLID_BBOX);
   edict->clipmask = MASK_PROJECTILE;
   setModel("sprites/pulsebomb.spr");
   edict->s.effects |= EF_AUTO_ANIMATE;

   // Set the flying velocity
   velocity = dir * 1000;
   gravity = 0.7;

   SetGravityAxis(owner->gravaxis);

   takedamage = DAMAGE_NO;

   // Set the light and effects
   edict->s.renderfx |= RF_DLIGHT;
   edict->s.effects |= EF_PULSE;
   edict->s.radius = 100;
   edict->s.color_r = 0.1f;
   edict->s.color_g = 0.1f;
   edict->s.color_b = 0.9f;

   // setup ambient thrust sound
   ev1 = new Event(EV_RandomEntitySound);
   ev1->AddString("thrust");
   ProcessEvent(ev1);

   // Set size and origin
   setSize({ -1, -1, -1 }, { 1, 1, 1 });
   setOrigin(pos);
   worldorigin.copyTo(edict->s.old_origin);

   // Remove the projectile in the future
   ev2 = new Event(EV_Pulse_Remove);
   PostEvent(ev2, 30);
}

CLASS_DECLARATION(BulletWeapon, PulseRifle, "weapon_pulserifle");

ResponseDef PulseRifle::Responses[] =
{
   { &EV_Weapon_Shoot,              (Response)&PulseRifle::Shoot },
   { nullptr, nullptr }
};

PulseRifle::PulseRifle() : BulletWeapon()
{
#ifdef SIN_DEMO
   PostEvent(EV_Remove, 0);
   return;
#endif

   //###
   if(ctf->value)
      SetModels("pulse2.def", "view_pulse2_ctf.def");
   else
   //###
      SetModels("pulse2.def", "view_pulse2.def");

   SetAmmo("BulletPulse", 10, 50);
   SetSecondaryAmmo("BulletPulse", 5, 50);
   SetRank(80, 80);
   SetType(WEAPON_2HANDED_LO);
   dualmode = true;
   modelIndex("pulse_ammo.def");
   modelIndex("beam.def");
   modelIndex("pulseshock.def");
   modelIndex("sprites/pulsebomb.spr");
   modelIndex("sprites/pulse_muzzle.spr");
}

// This is the trace that the laser portion of this weapon does.
void PulseRifle::TraceAttack(Vector start, Vector end, int damage, trace_t *trace, int numricochets, int kick, int dflags)
{
   Vector   org;
   Vector   dir;
   int      surfflags;
   int      surftype;
   Entity    *ent;

   if(HitSky(trace))
   {
      return;
   }

   dir = end - start;
   dir.normalize();

   org = end - dir * 4;

   if(!trace->surface)
   {
      surfflags = 0;
      surftype = 0;
   }
   else
   {
      surfflags = trace->surface->flags;
      surftype = SURFACETYPE_FROM_FLAGS(surfflags);
      surfaceManager.DamageSurface(trace, damage, owner);
   }

   ent = trace->ent->entity;

   if(ent && ent->takedamage)
   {
      if(trace->intersect.valid)
      {
         // We hit a valid group so send in location based damage
         ent->Damage(this,
                     owner,
                     damage,
                     trace->endpos,
                     dir,
                     trace->plane.normal,
                     kick,
                     dflags,
                     MOD_PULSELASER,
                     trace->intersect.parentgroup,
                     -1,
                     trace->intersect.damage_multiplier);
      }
      else
      {
         // We didn't hit any groups, so send in generic damage
         ent->Damage(this,
                     owner,
                     damage,
                     trace->endpos,
                     dir,
                     trace->plane.normal,
                     kick,
                     dflags,
                     MOD_PULSELASER,
                     -1,
                     -1,
                     1);
      }
   }
}

void PulseRifle::PulseExplosion(trace_t *trace)
{
   Vector      org, v;
   float       damg;
   Entity      *ent;
   float       kick = 250;
   Vector      shockangles;
   Vector      norm;
   Vector      tempvec;
   float       points;
   Vector      endpos;
   float       radius;

   damg = 50 + (int)G_Random(20);
   RadiusDamage(this, owner, damg, nullptr, MOD_PULSE);

   VectorCopy(trace->dir, tempvec.vec3());
   endpos = trace->endpos - (tempvec * 8);

   norm = trace->plane.normal;
   norm.x = -norm.x;
   norm.y = -norm.y;
   shockangles = norm.toAngles();
   shockangles.z = G_Random(360);

   RandomPositionedSound(endpos, "impact_smallexplosion", 1.0, CHAN_AUTO, ATTN_NORM);

   radius = damg + 50;
   ent = findradius(nullptr, endpos, radius);
   while(ent)
   {
      if((ent->takedamage) &&
         (ent->movetype != MOVETYPE_NONE) &&
         (ent->movetype != MOVETYPE_BOUNCE) &&
         (ent->movetype != MOVETYPE_PUSH) &&
         (ent->movetype != MOVETYPE_STOP))
      {
         org = ent->centroid;
         v = org - endpos;
         points = v.length();
         v.normalize();
         ent->velocity += (v * kick);

         points *= (float)0.5;

         if(points < 0)
         {
            points = 0;
         }

         points = damg - points;

         if(points > 0)
         {
            if(this->CanDamage(ent))
            {
               ent->Damage(this, owner, points,
                           org, v, vec_zero, points,
                           DAMAGE_RADIUS, MOD_PULSE,
                           -1, -1, 1.0f);
            }
         }
      }
      ent = findradius(ent, endpos, radius);
   }
}

void PulseRifle::Shoot(Event *ev)
{

   Vector   pos;
   Vector   end;
   Vector   dir;
   Vector   delta;
   trace_t  trace;
   float    dist;
   float    length;
   float    damg;

   assert(owner);
   if(!owner)
   {
      return;
   }

   GetMuzzlePosition(&pos, &dir);

   if(weaponmode == PRIMARY)
   {
      auto pulse = new Pulse();
      pulse->Setup(owner, pos, dir);
      NextAttack(0.5);
   }
   else
   {
      // Fire the beam
      length = ev->GetInteger(1);
      end = pos + dir * length;
      trace = G_Trace(pos, vec_zero, vec_zero, end, owner, MASK_SHOT, "PulseRifle::Shoot");
      delta = trace.endpos - pos;
      dist = delta.length();

      // Set the pitch of this weapon so the client can use it to fire bullets in the right directions   
      dir = Vector(owner->orientation[0]);
      angles = dir.toAngles();
      setAngles(angles);

      if(ctf->value)
         damg = 30;
      else
         damg = 15;
      TraceAttack(pos, trace.endpos, damg, &trace, 0, 0, 0);
      NextAttack(0);
   }
}

CLASS_DECLARATION(PulseRifle, GenericPulseRifle, "weapon_genericpulserifle");

ResponseDef GenericPulseRifle::Responses[] =
{
   { nullptr, nullptr }
};

GenericPulseRifle::GenericPulseRifle() : PulseRifle()
{
   SetModels(nullptr, "view_pulse2.def");
}

// EOF

