//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/thrall.cpp                       $
// $Revision:: 18                                                             $
//   $Author:: Jimdose                                                        $
//     $Date:: 8/03/99 7:09p                                                  $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// ThrallMaster
// 

#include "g_local.h"
#include "actor.h"
#include "thrall.h"
#include "explosion.h"
#include "vehicle.h"
#include "object.h"
#include "gibs.h"

#define DRUNKMISSILE_SPEED 1000.0f

Event EV_ThrallMaster_FirePulse("firepulse");
Event EV_ThrallMaster_FireRockets("firerockets");
Event EV_ThrallMaster_GibFest("gibfest");

CLASS_DECLARATION(Actor, ThrallMaster, "boss_thrallmaster");

ResponseDef ThrallMaster::Responses[] =
{
   { &EV_ThrallMaster_FirePulse,     (Response)&ThrallMaster::FirePulse },
   { &EV_ThrallMaster_FireRockets,   (Response)&ThrallMaster::FireRockets },
   { &EV_Sentient_WeaponUse,         (Response)&ThrallMaster::WeaponUse },
   { &EV_ThrallMaster_GibFest,       (Response)&ThrallMaster::GibFest },
   { &EV_FadeOut,                    NULL },
   { NULL, NULL }
};

ThrallMaster::ThrallMaster() : Actor()
{
   modelIndex("sprites/thrallpulse.spr");
   modelIndex("view_genbullet.def");
   modelIndex("trocket.def");
   modelIndex("thrallfire.def");

   setModel("thrall.def");
   weaponmode = PRIMARY;
   gunbone = "gun";
   flags |= FL_NOION;
}

Vector ThrallMaster::GunPosition(void)
{
   vec3_t	trans[3];
   vec3_t   orient;
   int		groupindex;
   int		tri_num;
   Vector	offset = vec_zero;
   Vector	result;

   // get the gun position of the actor
   if(!gi.GetBoneInfo(edict->s.modelindex, gunbone.c_str(), &groupindex, &tri_num, orient))
   {
      // Gun doesn't have a barrel, just return the default
      return worldorigin + gunoffset;
   }

   gi.GetBoneTransform(edict->s.modelindex, groupindex, tri_num, orient, edict->s.anim,
                       edict->s.frame, edict->s.scale, trans, offset.vec3());

   MatrixTransformVector(offset.vec3(), orientation, result.vec3());
   result += worldorigin;

   return result;
}

qboolean ThrallMaster::CanShootFrom(Vector pos, Entity *ent, qboolean usecurrentangles)
{
   int      mask;
   Vector	delta;
   Vector	start;
   Vector	end;
   float		len;
   trace_t	trace;
   Vehicle	*v;
   Entity	*t;
   Vector   ang;

   if(!currentWeapon || !WithinDistance(ent, vision_distance))
   {
      if(!currentWeapon && !has_melee)
         return false;
   }

   if(usecurrentangles)
   {
      Vector	dir;

      start = pos + GunPosition() - worldorigin;
      end = ent->centroid;
      end.z += (ent->absmax.z - ent->centroid.z) * 0.75f;
      delta = end - start;
      ang = delta.toAngles();
      ang.x = -ang.x;
      ang.y = angles.y;
      len = delta.length();
      ang.AngleVectors(&dir, NULL, NULL);
      dir *= len;
      end = start + dir;
   }
   else
   {
      vec3_t	trans[3];
      vec3_t	transtemp[3];
      vec3_t   orient;
      int		groupindex;
      int		tri_num;
      Vector	offset = vec_zero;
      Vector	result;

      // endpos
      end = ent->centroid;
      end.z += (ent->absmax.z - ent->centroid.z) * 0.75f;

      // get the gun position of the actor
      if(!gi.GetBoneInfo(edict->s.modelindex, gunbone.c_str(), &groupindex, &tri_num, orient))
      {
         // Gun doesn't have a barrel, just return the default
         result = gunoffset;
      }
      else
      {
         Vector forward, right, up;

         // get new forward vec
         forward = end - worldorigin;
         forward.z = 0;
         forward.normalize();
         forward.copyTo(trans[0]);

         // new right vec
         right.x = -forward.y;
         right.y = forward.x;
         right.z = 0;
         right.copyTo(trans[1]);

         // new up vec
         up.x = 0;
         up.y = 0;
         up.z = 1;
         up.copyTo(trans[2]);

         gi.GetBoneTransform(edict->s.modelindex, groupindex, tri_num, orient, edict->s.anim,
                             edict->s.frame, edict->s.scale, transtemp, offset.vec3());

         MatrixTransformVector(offset.vec3(), trans, result.vec3());
      }

      start = pos + result;
      delta = end - start;
      len = delta.length();
   }

   // check if we're too far away, or too close
   if(currentWeapon)
   {
      if((len > attack_range) || (len > currentWeapon->GetMaxRange()) || (len < currentWeapon->GetMinRange()))
      {
         return false;
      }
      mask = MASK_SHOT;
   }
   else
   {
      if((len > attack_range) || (len > melee_range))
      {
         return false;
      }
      mask = MASK_PROJECTILE;
   }

   // shoot past the guy we're shooting at
   end += delta * 4;

   // Check if he's visible
   trace = G_Trace(start, vec_zero, vec_zero, end, this, mask, "Actor::CanShootFrom");
   if(trace.startsolid)
   {
      return false;
   }

   // If we hit the guy we wanted, then shoot
   if(trace.ent == ent->edict)
   {
      return true;
   }

   // if we hit a vehicle, check if the driver is someone we want to hit
   t = trace.ent->entity;
   if(t && t->isSubclassOf<Vehicle>())
   {
      v = (Vehicle *)t;
      if((v->Driver() == ent) || IsEnemy(v->Driver()))
      {
         return true;
      }
      return false;
   }

   // If we hit someone else we don't like, then shoot
   if(IsEnemy(t))
   {
      return true;
   }

   // if we hit something breakable, check if shooting it will
   // let us shoot someone.
   if(t->isSubclassOf<Shatter>() ||
      t->isSubclassOf<Object>()  ||
      t->isSubclassOf<DamageThreshold>() ||
      t->isSubclassOf<ScriptModel>())
   {
      trace = G_Trace(Vector(trace.endpos), vec_zero, vec_zero, end, t, mask, "Actor::CanShootFrom 2");
      if(trace.startsolid)
      {
         return false;
      }

      // If we hit the guy we wanted, then shoot
      if(trace.ent == ent->edict)
      {
         return true;
      }

      // If we hit someone else we don't like, then shoot
      if(IsEnemy(trace.ent->entity))
      {
         return true;
      }

      // Forget it then
      return false;
   }

   return false;
}

void ThrallMaster::Chatter(const char *snd, float chance, float volume, int channel)
{
   if(chattime > level.time)
   {
      return;
   }

   RandomSound(snd, volume, channel, ATTN_NONE);

   chattime = level.time + 7 + G_Random(5);
}

void ThrallMaster::WeaponUse(Event *ev)
{
   Sentient::WeaponUse(ev);
   if(weaponmode == PRIMARY)
   {
      weaponmode = SECONDARY;
      gunbone = "chest";
   }
   else
   {
      weaponmode = PRIMARY;
      gunbone = "gun";
   }
}

void ThrallMaster::GibFest(Event *ev)
{
   Vector   pos;
   Gib      *gib1, *gib2;

#ifdef SIN_ARCADE
   //HACK to make game over only be sent once.
   if(max_health != 0)
   {
      max_health = 0;
      gi.WriteByte(svc_stufftext);
      gi.WriteString("gameover");
      gi.multicast(NULL, MULTICAST_ALL);
   }
#endif

   if(sv_gibs->value && !parentmode->value)
   {
      GetBone("chest", &pos, NULL, NULL, NULL);

      pos += worldorigin;

      gib1 = new Gib("gib1.def");
      gib1->setOrigin(pos);
      gib1->worldorigin.copyTo(gib1->edict->s.old_origin);
      gib1->SetVelocity(1000);
      gib1->velocity *= 3;
      gib1->edict->s.scale = 3.0;
      gib1->fadesplat = false;

      gib2 = new Gib("gib2.def");
      gib2->setOrigin(pos);
      gib2->worldorigin.copyTo(gib2->edict->s.old_origin);
      gib2->SetVelocity(1000);
      gib2->velocity *= 3;
      gib2->edict->s.scale = 3.0;
      gib2->fadesplat = false;
   }
}

void ThrallMaster::FirePulse(Event *ev)
{
   if((currentWeapon) && currentWeapon->ReadyToFire() && currentWeapon->HasAmmo())
   {
      weaponmode = SECONDARY;
      currentWeapon->Fire();
   }
}

void ThrallMaster::FireRockets(Event *ev)
{
   if((currentWeapon) && currentWeapon->ReadyToFire() && currentWeapon->HasAmmo())
   {
      weaponmode = PRIMARY;
      currentWeapon->Fire();
   }
}

CLASS_DECLARATION(Weapon, ThrallGun, NULL);

ResponseDef ThrallGun::Responses[] =
{
   { &EV_Weapon_Shoot,        (Response)&ThrallGun::Shoot },
   { &EV_Weapon_SecondaryUse, (Response)&ThrallGun::SecondaryUse },
   { NULL, NULL }
};

ThrallGun::ThrallGun() : Weapon()
{
   SetModels(nullptr, "view_genbullet.def");
   SetAmmo("Rockets", 1, 10);
}

void ThrallGun::Shoot(Event *ev)
{
   DrunkMissile *missile;
   ThrallPulse *pulse;
   Vector	pos;
   Vector	dir;

   assert(owner);
   if(!owner)
   {
      return;
   }

   GetMuzzlePosition(&pos, &dir);

   if(weaponmode == PRIMARY)
   {
      missile = new DrunkMissile();
      missile->Setup(owner, pos, dir);
      NextAttack(0);
   }
   else
   {
      pulse = new ThrallPulse();
      pulse->Setup(owner, pos, dir);
      NextAttack(0);
   }
}

void ThrallGun::SecondaryUse(Event *ev)
{
   if(weaponmode == PRIMARY)
   {
      weaponmode = SECONDARY;
   }
   else
   {
      weaponmode = PRIMARY;
   }
}

Event EV_DrunkMissile_HeatSeek("heatseek");

CLASS_DECLARATION(Projectile, DrunkMissile, NULL);

ResponseDef DrunkMissile::Responses[] =
{
   { &EV_Touch,                   (Response)&DrunkMissile::Explode },
   { &EV_DrunkMissile_HeatSeek,   (Response)&DrunkMissile::HeatSeek },
   { NULL, NULL }
};

EXPORT_FROM_DLL void DrunkMissile::Explode(Event *ev)
{
   int damg;
   Vector v;
   Entity *other;
   Entity *owner;

   other = ev->GetEntity(1);

   owner = G_GetEntity(this->owner);

   if(!owner)
   {
      owner = world;
   }

   if(!other || ((other == owner) && (owner != world)) || (other->isSubclassOf<DrunkMissile>()))
   {
      return;
   }

   flags &= ~FL_PRETHINK;
   stopsound(CHAN_VOICE);
   setSolidType(SOLID_NOT);
   hideModel();

   if(HitSky())
   {
      PostEvent(EV_Remove, 0);
      return;
   }

   damg = 40 + (int)G_Random(20);

   other->Damage(this, owner, damg, worldorigin, velocity, level.impact_trace.plane.normal, 200, 0, MOD_ROCKETSPLASH, -1, -1, 1.0f);

   SpawnBlastDamage(&level.impact_trace, damg, owner);

   v = velocity;
   v.normalize();

   // don't do radius damage to the other, because all the damage
   // was done in the impact
   v = worldorigin - v * 36;
   CreateExplosion(v, damg, 0.7f, true, this, owner, other);
   PostEvent(EV_Remove, 0.1);
}

EXPORT_FROM_DLL float DrunkMissile::ResolveMinimumDistance(Entity *potential_target, float currmin)
{
   float		currdist;
   float		dot;
   Vector	delta;
   Vector	norm;
   float		sine = 0.4f;

   delta = potential_target->centroid - worldorigin;

   norm = delta;
   norm.normalize();

   // Test if the target is in front of the missile
   dot = norm * orientation[0];
   if(dot < 0)
   {
      return currmin;
   }

   // Test if we're within the rocket's viewcone (45 degree cone)
   dot = norm * orientation[1];
   if(fabs(dot) > sine)
   {
      return currmin;
   }

   dot = norm * orientation[2];
   if(fabs(dot) > sine)
   {
      return currmin;
   }

   currdist = delta.length();
   if(currdist < currmin)
   {
      currmin = currdist;
      target = potential_target;
   }

   return currmin;
}

EXPORT_FROM_DLL float DrunkMissile::AdjustAngle(float maxadjust, float currangle, float targetangle)
{
   float dangle;
   float magangle;

   dangle = currangle - targetangle;

   if(dangle)
   {
      magangle = (float)fabs(dangle);
      if(magangle < maxadjust)
      {
         currangle = targetangle;
      }
      else
      {
         if(magangle > 180.0f)
         {
            maxadjust = -maxadjust;
         }
         if(dangle > 0)
         {
            maxadjust = -maxadjust;
         }
         currangle += maxadjust;
      }
   }

   while(currangle >= 360.0f)
   {
      currangle -= 360.0f;
   }

   while(currangle < 0.0f)
   {
      currangle += 360.0f;
   }

   return currangle;
}

EXPORT_FROM_DLL void DrunkMissile::HeatSeek(Event *ev)
{
   float		mindist;
   Entity	*ent;
   trace_t  trace;
   Vector	delta;
   Vector	v;
   int      n;
   int      i;

   if((!target) || (target == world))
   {
      mindist = 8192.0f;

      n = SentientList.NumObjects();
      for(i = 1; i <= n; i++)
      {
         ent = SentientList.ObjectAt(i);
         if(ent->entnum == owner)
         {
            continue;
         }

         if(((ent->takedamage != DAMAGE_AIM) || (ent->health <= 0)) && !(edict->svflags & SVF_MONSTER))
         {
            continue;
         }

         trace = G_Trace(worldorigin, vec_zero, vec_zero, ent->centroid, this, MASK_SHOT, "DrunkMissile::HeatSeek");
         if((trace.fraction != 1.0) && (trace.ent != ent->edict))
         {
            continue;
         }

         mindist = ResolveMinimumDistance(ent, mindist);
      }
   }
   else
   {
      float predict;
      float dist;
      float time;
      float angspeed;

      delta = target->centroid - worldorigin;
      dist = delta.length();
      time = dist * (1 / DRUNKMISSILE_SPEED);
      predict = (time * (0.5 + (skill->value + 1) * 0.125));
      delta += target->velocity * predict;
      delta.z = -delta.z;
      v = delta.toAngles();

      angspeed = 5.0f + skill->value;
      angles.x = AdjustAngle(angspeed, angles.x, v.x);
      angles.y = AdjustAngle(angspeed, angles.y, v.y);
      angles.z = AdjustAngle(angspeed, angles.z, v.z);
   }

   if(!target)
   {
      PostEvent(EV_DrunkMissile_HeatSeek, 0.2);
   }
   else
   {
      PostEvent(EV_DrunkMissile_HeatSeek, 0.1);
   }
}

EXPORT_FROM_DLL void DrunkMissile::Prethink(void)
{
   trace_t trace;
   Vector end;

   angles += Vector(G_CRandom(3), G_CRandom(5), 0);

   // Check if we're about to hit the ground at a shallow angle
   if((velocity.z < 0) && (angles.x < 15.0f))
   {
      end = worldorigin + velocity * 0.1f;
      trace = G_Trace(worldorigin, vec_zero, vec_zero, end, this, MASK_SHOT, "DrunkMissile::Prethink");
      if(trace.fraction != 1)
      {
         if(trace.plane.normal[2] > 0.6f)
         {
            angles.x = -3;
         }
      }
   }

   setAngles(angles);
   velocity = Vector(orientation[0]) * DRUNKMISSILE_SPEED;
}

EXPORT_FROM_DLL void DrunkMissile::Setup(Entity *owner, Vector pos, Vector dir)
{
   Event *ev;

   this->owner = owner->entnum;
   edict->owner = owner->edict;

   flags |= FL_PRETHINK;

   setMoveType(MOVETYPE_FLYMISSILE);
   setSolidType(SOLID_BBOX);
   edict->clipmask = MASK_PROJECTILE;

   // set missile duration
   ev = new Event(EV_Touch);
   ev->AddEntity(world);
   PostEvent(ev, 10);

   PostEvent(EV_DrunkMissile_HeatSeek, 0.1 + G_Random(0.2f));

   // set missile direction
   angles = dir.toAngles();
   angles[PITCH] = -angles[PITCH];
   setAngles(angles);
   velocity = Vector(orientation[0]) * 800.0f;

   target = NULL;

   setModel("trocket.def");
   setSize({ -1, -1, -1 }, { 1, 1, 1 });
   takedamage = DAMAGE_YES;
   health = 10;
   setOrigin(pos);
   worldorigin.copyTo(edict->s.old_origin);

   edict->s.renderfx |= RF_DLIGHT;
   edict->s.effects |= EF_ROCKET;
   edict->s.effects |= EF_EVERYFRAME;
   edict->s.angles[ROLL] = rand() % 360;
   avelocity = { 0, 0, 90 };
   gravity = 0;
   edict->s.color_r = 0.8;
   edict->s.color_g = 0.4;
   edict->s.color_b = 0;
   edict->s.radius = 200;

   // setup ambient thrust
   ev = new Event(EV_RandomEntitySound);
   //ev->AddString( "thrust" );
   ev->AddString("fire");
   ProcessEvent(ev);
}

CLASS_DECLARATION(Projectile, ThrallPulse, NULL);

ResponseDef ThrallPulse::Responses[] =
{
   { &EV_Touch,                   (Response)&ThrallPulse::Explode },
   { NULL, NULL }
};

EXPORT_FROM_DLL void ThrallPulse::Explode(Event *ev)
{
   int damg;
   Vector v;
   Entity *other;
   Entity *owner;
   ThrallPulseDebris *debris;
   int i;

   other = ev->GetEntity(1);

   owner = G_GetEntity(this->owner);

   if(!owner)
      owner = world;

   if(!other || (other == owner) || (other->isSubclassOf<ThrallPulse>()))
   {
      return;
   }

   flags &= ~FL_PRETHINK;
   stopsound(CHAN_VOICE);
   setSolidType(SOLID_NOT);
   hideModel();

   if(HitSky())
   {
      PostEvent(EV_Remove, 0);
      return;
   }

   if(ctf->value)
      damg = 150;
   else
      damg = 160 + (int)G_Random(50);

   other->Damage(this, owner, damg, worldorigin, velocity, level.impact_trace.plane.normal, 320, 0, MOD_THRALLBALL, -1, -1, 1.0f);

   SpawnBlastDamage(&level.impact_trace, damg, owner);

   v = velocity;
   v.normalize();

   // don't do radius damage to the other, because all the damage
   // was done in the impact
   v = worldorigin - v * 36;
   CreateExplosion(v, damg, 0.7f, true, this, owner, other, MOD_THRALLSPLASH);
   PostEvent(EV_Remove, 0.1);
   FlashPlayers(v, 1, 1, 1, 0.5, 768);

#if 0
   if(ctf->value)
   {
      Entity *stuff;

      stuff = new Entity();
      stuff->setModel("ctfpulse.def");
      stuff->setOrigin(origin);
      stuff->showModel();
      stuff->worldorigin.copyTo(stuff->edict->s.old_origin);
      stuff->RandomAnimate("explode", NULL);
      stuff->PostEvent(EV_Remove, FRAMETIME * 2);
   }
   else
#endif
   {
      for(i = 1; i < 4; i++)
      {
         debris = new ThrallPulseDebris();
         debris->Setup(owner, v, i);
      }
   }
}

EXPORT_FROM_DLL void ThrallPulse::Setup(Entity *owner, Vector pos, Vector dir)
{
   Event *ev;

   this->owner = owner->entnum;
   edict->owner = owner->edict;

   setMoveType(MOVETYPE_FLYMISSILE);
   setSolidType(SOLID_BBOX);
   edict->clipmask = MASK_PROJECTILE;

   // set missile duration
   ev = new Event(EV_Touch);
   ev->AddEntity(world);
   PostEvent(ev, 10);

   // set missile direction
   angles = dir.toAngles();
   angles[PITCH] = -angles[PITCH];
   setAngles(angles);
   velocity = Vector(orientation[0]) * 1400.0f;

   setModel("sprites/thrallpulse.spr");

   setSize({ -8, -8, -8 }, { 8, 8, 8 });
   takedamage = DAMAGE_NO;
   setOrigin(pos);
   worldorigin.copyTo(edict->s.old_origin);
   showModel();

   edict->s.renderfx |= RF_DLIGHT;
   edict->s.angles[ROLL] = rand() % 360;
   avelocity = { 0, 0, 90 };
   gravity = 0;
   edict->s.color_r = 0.8;
   edict->s.color_g = 0;
   edict->s.color_b = 0;
   edict->s.radius = 200;
}

CLASS_DECLARATION(Projectile, ThrallPulseDebris, NULL);

ResponseDef ThrallPulseDebris::Responses[] =
{
   { &EV_Touch,            (Response)&ThrallPulseDebris::Touch },
   { NULL, NULL }
};

EXPORT_FROM_DLL void ThrallPulseDebris::Touch(Event *ev)
{
   Entity *other;
   Entity *owner;

   if(level.time < nexttouch)
   {
      return;
   }

   nexttouch = level.time + 1;

   other = ev->GetEntity(1);
   owner = G_GetEntity(this->owner);
   if(!owner)
   {
      owner = world;
   }

   if(!other)
   {
      return;
   }

   other->Damage(this, owner, 10 * edict->s.scale, worldorigin, velocity,
                 level.impact_trace.plane.normal, velocity.length(), 0, MOD_DEBRIS, -1, -1, 1.0f);
}

EXPORT_FROM_DLL void ThrallPulseDebris::Prethink(void)
{
   if((level.time - spawntime) > 4)
   {
      edict->s.scale *= 0.9;
      setSize(Vector(-4, -4, -4) * edict->s.scale, Vector(4, 4, 4) * edict->s.scale);
   }
}

EXPORT_FROM_DLL void ThrallPulseDebris::Setup(Entity *owner, Vector pos, float size)
{
   this->owner = owner->entnum;
   edict->owner = owner->edict;

   nexttouch = 0;
   spawntime = level.time;
   flags |= FL_PRETHINK;
   setModel("thrallfire.def");
   setMoveType(MOVETYPE_BOUNCE);
   setSolidType(SOLID_TRIGGER);
   edict->s.effects |= EF_ROCKET;
   showModel();
   setOrigin(pos);
   worldorigin.copyTo(edict->s.old_origin);
   velocity = Vector(G_CRandom(200), G_CRandom(200), G_Random(100) + 100);
   PostEvent(EV_Remove, 4 + 2 * size);
   edict->s.scale *= size;
   setSize(Vector(-4, -4, -4) * edict->s.scale, Vector(4, 4, 4) * edict->s.scale);
}

// EOF

