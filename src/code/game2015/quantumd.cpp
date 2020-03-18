//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/quantumd.cpp                     $
// $Revision:: 50                                                             $
//   $Author:: Aldie                                                          $
//     $Date:: 5/18/99 7:07p                                                  $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Quantum Destabilizer

#include "quantumd.h"
#include "specialfx.h"
#include "explosion.h"
#include "player.h"
#include "surface.h"

#define ION_MODE 1
#define DESTAB_MODE 2

class EXPORT_FROM_DLL IonBurst : public Projectile
{
private:
   float    power;

public:
   CLASS_PROTOTYPE(IonBurst);

   virtual void   Setup(Entity *owner, Vector pos, float power);
   virtual void   Explode(Event *ev);
   virtual void   Archive(Archiver &arc)   override;
   virtual void   Unarchive(Archiver &arc) override;
};

EXPORT_FROM_DLL void IonBurst::Archive(Archiver &arc)
{
   Projectile::Archive(arc);

   arc.WriteFloat(power);
}

EXPORT_FROM_DLL void IonBurst::Unarchive(Archiver &arc)
{
   Projectile::Unarchive(arc);

   arc.ReadFloat(&power);
}

class EXPORT_FROM_DLL Ion : public Projectile
{
private:
   float    power;

public:
   CLASS_PROTOTYPE(Ion);

   virtual void   Setup(Entity *owner, Vector pos, Vector dir, float power);
   virtual void   Explode(Event *ev);
   qboolean       CanSee(Entity *ent);
   virtual void   Seek(Event *ev);
   float          AdjustAngle(float maxadjust, float currangle, float targetangle);
   virtual void   Archive(Archiver &arc)   override;
   virtual void   Unarchive(Archiver &arc) override;
};

EXPORT_FROM_DLL void Ion::Archive(Archiver &arc)
{
   Projectile::Archive(arc);

   arc.WriteFloat(power);
}

EXPORT_FROM_DLL void Ion::Unarchive(Archiver &arc)
{
   Projectile::Unarchive(arc);

   arc.ReadFloat(&power);
}

CLASS_DECLARATION(Projectile, IonBurst, nullptr);

ResponseDef IonBurst::Responses[] =
{
   { &EV_Touch,              (Response)&IonBurst::Explode },
   { nullptr, nullptr }
};

void IonBurst::Explode(Event *ev)
{
   Entity   *other;
   Entity   *owner;
   float    damg;

   other = ev->GetEntity(1);
   assert(other);

   // Do damage to what we hit
   if(other->isSubclassOf<Teleporter>())
   {
      return;
   }

   if(other->entnum == this->owner)
   {
      return;
   }

   // Hit the sky, so remove everything
   if(HitSky())
   {
      PostEvent(EV_Remove, 0);
      return;
   }

   owner = G_GetEntity(this->owner);

   if(!owner)
      owner = world;

   damg = 20 + power * 20;

   if(other->takedamage)
   {
      other->Damage(this, owner, damg, worldorigin, velocity, level.impact_trace.plane.normal, 32, 0, MOD_ION, -1, -1, 1.0f);
      PostEvent(EV_Remove, 0);
   }
}

void IonBurst::Setup(Entity *owner, Vector pos, float power)
{
   Event *event;

   this->owner = owner->entnum;
   edict->owner = owner->edict;

   setMoveType(MOVETYPE_BOUNCE);
   setSolidType(SOLID_BBOX);
   edict->clipmask = MASK_PROJECTILE;
   SetGravityAxis(owner->gravaxis);
   showModel();

   setModel("qp_burst.def");
   RandomAnimate("flyburst", nullptr);

   this->power = power;

   // Set the flying velocity
   velocity[0] = 300.0 * crandom() + 200 * power;
   velocity[1] = 300.0 * crandom() + 200 * power;
   velocity[2] = 300.0 + 300.0 * random() + 200 * power;

   takedamage = DAMAGE_NO;

   // Set the light and effects
   edict->s.renderfx |= RF_DLIGHT;
   edict->s.effects |= EF_EVERYFRAME;
   edict->s.radius = 200;
   edict->s.color_r = 1.0f;
   edict->s.color_g = 0.2;
   edict->s.color_b = 0.1;

   setScale(0.3);
   // Set size and origin
   setSize({ -2, -2, -2 }, { 2, 2, 2 });
   setOrigin(pos);
   worldorigin.copyTo(edict->s.old_origin);

   event = new Event(EV_Remove);
   PostEvent(event, 1.2);
}

CLASS_DECLARATION(Projectile, Ion, nullptr);

Event EV_Ion_Seek("ionseek");

ResponseDef Ion::Responses[] =
{
   { &EV_Touch,              (Response)&Ion::Explode },
   { &EV_Ion_Seek,           (Response)&Ion::Seek },
   { nullptr, nullptr }
};

qboolean Ion::CanSee(Entity *ent)
{
   Vector start;
   Vector end;
   trace_t trace;

   start = worldorigin;
   end = ent->centroid;

   // Check if he's visible
   trace = G_Trace(start, vec_zero, vec_zero, end, this, MASK_OPAQUE, "Ion::CanSee");
   if(trace.fraction == 1.0 || trace.ent == ent->edict)
   {
      return true;
   }

   return false;
}

float Ion::AdjustAngle(float maxadjust, float currangle, float targetangle)
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

void Ion::Seek(Event *ev)
{
   Entity   *ent;
   Vector   delta;
   float    dist;
   Entity   *bestent = nullptr;
   float    bestdist = 999999;
   Vector   newdir;

   ent = findradius(nullptr, worldorigin.vec3(), 256);
   while(ent)
   {
      if((ent->entnum != owner) && (ent->takedamage) && ent->isSubclassOf<Sentient>())
      {
         delta = worldorigin - ent->centroid;
         dist = delta.length();

         if((dist < bestdist) && CanSee(ent))
         {
            bestent = ent;
            bestdist = dist;
         }
      }
      ent = findradius(ent, worldorigin.vec3(), 1000);
   }

   if(!bestent)
   {
      PostEvent(ev, FRAMETIME);
      return;
   }

   newdir = bestent->centroid - worldorigin;
   newdir.normalize();
   velocity = newdir * 250;

   angles = newdir.toAngles();
   angles[PITCH] = -angles[PITCH];
   setAngles(angles);

   PostEvent(ev, FRAMETIME);
}

void Ion::Explode(Event *ev)
{
   Entity      *other;
   Entity      *owner;
   int         damg;
   Vector      v;

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

   setSolidType(SOLID_NOT);

   // Hit the sky, so remove everything
   if(HitSky())
   {
      PostEvent(EV_Remove, 0);
      return;
   }

   damg = 50 + (power * 200);

   // Single player packs a bigger punch
   if(!deathmatch->value && owner->isClient())
      damg *= 1.5;

   if(other->takedamage)
   {
      other->Damage(this, owner, damg, worldorigin, velocity, level.impact_trace.plane.normal, 0, 0, MOD_ION, -1, -1, 1.0f);

      if(other->isSubclassOf<Sentient>())
      {
         auto sent = static_cast<Sentient *>(other);
         sent->ProcessEvent(EV_Sentient_Freeze);
         sent->PostEvent(EV_Sentient_UnFreeze, 1);
      }
   }

   v = velocity;
   v.normalize();
   v = worldorigin - v * 36;

   RadiusDamage(this, owner, damg, this, MOD_ION);

   edict->s.effects &= ~EF_EVERYFRAME;
   // Process the explode animation, then remove this thing
   RandomAnimate("explode", EV_Remove);

#if 0
   // Fire off some gib bursts
   for(i = 0; i < 3; i++)
   {
      IonBurst    *burst;

      burst = new IonBurst();
      burst->Setup(owner, v, power);
   }
#endif

   FlashPlayers(v, 1, 1, 1, 0.5, 500);
}

void Ion::Setup(Entity *owner, Vector pos, Vector dir, float power)
{
   Event *event;

   this->owner = owner->entnum;
   edict->owner = owner->edict;

   this->power = power;
   // Align the projectile
   angles = dir.toAngles();
   angles[PITCH] = -angles[PITCH];
   setAngles(angles);
   edict->s.angles[ROLL] = rand() % 360;

   // Flies like a rocket
   setMoveType(MOVETYPE_FLYMISSILE);
   setSolidType(SOLID_BBOX);
   edict->clipmask = MASK_PROJECTILE;
   setModel("qp_burst.def");

   RandomAnimate("idle", nullptr);

   //PostEvent( EV_Ion_Seek, 0.1f );
   // Set the flying velocity
   velocity = dir * 1000;
   gravity = 0;

   avelocity = { 0, 0, 2000 };

   takedamage = DAMAGE_NO;

   // Set the light and effects
   edict->s.renderfx |= RF_DLIGHT;
   edict->s.effects |= EF_EVERYFRAME;
   edict->s.radius = 200;
   edict->s.color_r = 1.0f;
   edict->s.color_g = 0;
   edict->s.color_b = 0;

   // Adjust scale by power
   edict->s.scale *= power;
   edict->s.scale += 0.2f;

   // Set size and origin
   setSize({ -1, -1, -1 }, { 1, 1, 1 });
   setOrigin(pos);
   worldorigin.copyTo(edict->s.old_origin);

   event = new Event(EV_Remove);
   PostEvent(event, 20);
}

CLASS_DECLARATION(BulletWeapon, QuantumDestabilizer, "weapon_quantumdestabilizer");

Event EV_Quantum_EatAmmo("eatammo");
Event EV_Quantum_Destruct("destruct");
Event EV_Quantum_StartSelfDestruct("destruct_time");
Event EV_Quantum_SentientOverload("sentient_overload");

ResponseDef QuantumDestabilizer::Responses[] =
{
   { &EV_Weapon_Shoot,                 (Response)&QuantumDestabilizer::Shoot },
   { &EV_Quantum_EatAmmo,              (Response)&QuantumDestabilizer::EatAmmo },
   { &EV_Quantum_Destruct,             (Response)&QuantumDestabilizer::Destruct },
   { &EV_Quantum_StartSelfDestruct,    (Response)&QuantumDestabilizer::StartSelfDestruct },
   { &EV_Quantum_SentientOverload,     (Response)&QuantumDestabilizer::SentientOverload },
   { nullptr, nullptr }
};

QuantumDestabilizer::QuantumDestabilizer() : BulletWeapon()
{
#ifdef SIN_DEMO
   PostEvent(EV_Remove, 0);
   return;
#endif
   SetModels("quantum.def", "view_quantum.def");
   modelIndex("qp_burst.def");
   modelIndex("sprites/qp_exhaust.spr");
   modelIndex("sprites/qp_sphere.spr");
   modelIndex("sprites/shockwave2.spr");
   modelIndex("sphere3.def");
   gi.soundindex("weapons/quantum/burst.wav");

   SetAmmo("BulletPulse", 20, 100);
   //SetSecondaryAmmo( "BulletPulse", 5, 100);
   SetRank(90, 90);
   SetType(WEAPON_2HANDED_LO);
   trapped_sent = nullptr;
   //dualmode = true;
}

void QuantumDestabilizer::EatAmmo(Event *ev)
{
   Event *event;
   Ammo * ammo;

   assert(owner);
   if(owner && owner->isClient() && !(owner->flags & FL_GODMODE))
   {
      if(ammotype.length())
      {
         ammo = (Ammo *)owner->FindItem(ammotype.c_str());

         // Eat 1 ammo per charge up
         if(!ammo || !ammo->Use(1))
         {
            // Ran out of ammo, fire this sucker
            event = new Event(EV_Sentient_ReleaseAttack);
            event->AddFloat(level.time - owner->firedowntime);
            owner->ProcessEvent(event);
            owner->firedown = false;
         }
         else
         {
            PostEvent(EV_Quantum_EatAmmo, 0.1);
         }
      }
   }
}

void QuantumDestabilizer::Destruct(Event *ev)
{
   Entity *ent;

   if(ctf->value)
   {
      auto event = new Event(EV_Sentient_ReleaseAttack);
      event->AddFloat(level.time - owner->firedowntime);
      owner->ProcessEvent(event);
      owner->firedown = false;

      return;
   }

   CancelEventsOfType(EV_Quantum_EatAmmo);

   if(!owner)
      return;

   //### added for MFD
   if(deathmatch->value == DEATHMATCH_MFD && this != owner->CurrentWeapon())
   {
      return;
   }
   //###

   CreateExplosion(owner->centroid, 200, 1.0f, true, this, owner, owner);
   
   ent = findradius(nullptr, owner->centroid, 512);
   while(ent)
   {
      if((ent != owner) && (!ent->deadflag) && (ent->takedamage) && !(ent->flags & FL_NOION))
      {
         if(owner->CanDamage(ent))
            ent->Damage(this, owner, 10000 + ent->health, ent->worldorigin, vec_zero, vec_zero, 0, DAMAGE_NO_ARMOR | DAMAGE_NO_SKILL, MOD_ION, -1, -1, 1.0f);
      }
      if(!owner)
         return;
      ent = findradius(ent, owner->centroid, 512);
   }

   // Make owner deceased
   owner->Damage(this, owner, 10000 + owner->health, worldorigin, vec_zero, vec_zero, 0, DAMAGE_NO_ARMOR | DAMAGE_NO_SKILL, MOD_ION_DESTRUCT, -1, -1, 1.0f);
}

void QuantumDestabilizer::SentientOverload(Event *ev)
{
#if 0
   if(trapped_sent)
   {
      if(trapped_sent->isClient())
      {
         Player *player = (Player *)(Sentient *)trapped_sent;

         // Clear this so we don't get thrown out of the gun when it gets dropped
         trapped_sent = NULL;

         player->trappedInQuantum = false;
         player->edict->svflags &= ~SVF_HIDEOWNER;
         player->edict->owner = NULL;
         player->origin = owner->origin;
         player->ProcessEvent(EV_Sentient_UnFreeze);
         player->showModel();
         player->setSolidType(SOLID_BBOX);
         player->unlink();

         SpawnTeleportEffect(player->worldorigin, 123);
         KillBox(player);

         player->setOrigin(player->origin);
         player->worldorigin.copyTo(player->edict->s.old_origin);
         player->SetCamera(NULL);
      }
      else
      {
         // Kill the owner
         owner->health = 0;
         owner->Damage(this, owner, 1000, worldorigin, vec_zero, vec_zero, 0, DAMAGE_NO_ARMOR | DAMAGE_NO_SKILL, MOD_ION_DESTRUCT, -1, -1, 1.0f);
      }
   }
#endif
}

void QuantumDestabilizer::StartSelfDestruct(Event *ev)
{
   float time = ev->GetFloat(1);
   PostEvent(EV_Quantum_Destruct, time);
}

void QuantumDestabilizer::ReleaseFire(float holdfiretime)
{
   if(weaponmode == PRIMARY)
   {
      CancelEventsOfType(EV_Quantum_EatAmmo);
      CancelEventsOfType(EV_Quantum_Destruct);

      if(holdfiretime > 5)
         holdfiretime = 5;
      if(holdfiretime < 0)
         holdfiretime = 0;

      power = holdfiretime / 5.0f;

      RandomAnimate("releasefire", EV_Weapon_DoneFiring);
      last_attack_time = level.time;
   }
}

qboolean QuantumDestabilizer::ShootSentient(Vector pos, Vector dir)
{
   return false;
#if 0
   Vector   end;
   trace_t  trace;

   if(trapped_sent)
   {
      // Check to see if there is empty space in front of the gun
      end = pos + dir * 50;
      trace = G_Trace(pos, trapped_sent->mins, trapped_sent->maxs, end, this, MASK_SOLID, "QuantumDestabilizer::ShootSentient");

      if(trace.fraction != 1.0)
         return false;

      trapped_sent->origin = pos + dir * 25;
      trapped_sent->origin -= (trapped_sent->maxs - trapped_sent->mins) * 0.5f;
      trapped_sent->velocity = pos + dir * 750;
      trapped_sent->setOrigin(trapped_sent->origin);
      trapped_sent->worldorigin.copyTo(trapped_sent->edict->s.old_origin);
      trapped_sent->ProcessEvent(EV_Sentient_UnFreeze);
      trapped_sent->showModel();
      trapped_sent->setSolidType(SOLID_BBOX);
      trapped_sent->setMoveType(MOVETYPE_HURL);

      if(trapped_sent->isClient())
      {
         Player *player = (Player *)(Sentient *)trapped_sent;

         player->edict->svflags &= ~SVF_HIDEOWNER;
         player->edict->owner = NULL;
         player->trappedInQuantum = false;
      }

      trapped_sent = NULL;
      CancelEventsOfType(EV_Quantum_SentientOverload);

      return(true);
   }
   else
   {
      gi.dprintf("No sentient in quantum destabilizer");
      return(false);
   }
#endif
}

void QuantumDestabilizer::SuckSentient(Vector pos, Vector dir)
{
#if 0
   Vector   end;
   trace_t  trace;

   end = pos + dir * 8192;
   trace = G_FullTrace(pos, vec_zero, vec_zero, end, 5, owner, MASK_SHOT, "QuantumDestabilizer::SuckSentient");

   // Check to see if we hit a sentient
   if(trace.ent)
   {
      if(trace.ent->entity->isSubclassOf(Sentient))
      {
         // Freeze them, hide the model, set notsolid
         Sentient *sent = (Sentient *)trace.ent->entity;
         sent->ProcessEvent(EV_Sentient_Freeze);
         sent->hideModel();
         sent->setSolidType(SOLID_NOT);

         // Do the implosion tess effect
         TesselateModel(sent,
                        1,
                        1000,
                        dir,
                        100,
                        1.0f,
                        0,
                        vec3_origin,
                        TESSELATE_IMPLODE,
                        126
         );

         // If they are a client, set some flags so they don't see the owner
         // of this weapon.
         if(sent->isClient())
         {
            Player *player;

            player = (Player *)sent;
            player->trappedInQuantum = true;
            player->SetCamera(owner);
            player->edict->svflags |= SVF_HIDEOWNER;
            player->edict->owner = owner->edict;
         }

         trapped_sent = sent;

         // We have a sentient trapped, set a destruct timer for 5 seconds
         PostEvent(EV_Quantum_SentientOverload, 5);
      }
   }
#endif
}

// This is the trace that the laser portion of this weapon does.
void QuantumDestabilizer::TraceAttack(Vector start, Vector end, int damage, trace_t *trace, int numricochets, int kick, int dflags)
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
         ent->Damage(
            this,
            owner,
            damage,
            trace->endpos,
            dir,
            trace->plane.normal,
            kick,
            dflags,
            MOD_QUANTUM,
            trace->intersect.parentgroup,
            -1,
            trace->intersect.damage_multiplier);
      }
      else
      {
         // We didn't hit any groups, so send in generic damage
         ent->Damage(
            this,
            owner,
            damage,
            trace->endpos,
            dir,
            trace->plane.normal,
            kick,
            dflags,
            MOD_QUANTUM,
            -1,
            -1,
            1);
      }
   }
}

void QuantumDestabilizer::Shoot(Event *ev)
{
   Vector pos, dir;
   auto   ion = new Ion();

   assert(owner);
   if(!owner)
   {
      return;
   }

   GetMuzzlePosition(&pos, &dir);
   ion->Setup(owner, pos, dir, power);
   NextAttack(1);

#if 0
   if(weaponmode == PRIMARY)
   {
      Ion *ion = new Ion();
      ion->Setup(owner, pos, dir, power);
      NextAttack(1);
   }
   else
   {
      Vector end, delta;
      length = ev->GetInteger(1);
      end = pos + dir * length;
      trace = G_FullTrace(pos, vec_zero, vec_zero, end, 30, this, MASK_PROJECTILE, "QuantumDestabilizer::Shoot");
      delta = trace.endpos - pos;

      // Set the pitch of this weapon so the client can use it to flame in the right direction
      dir = Vector(owner->orientation[0]);
      angles = dir.toAngles();
      setAngles(angles);

      damg = 25;
      TraceAttack(pos, trace.endpos, damg, &trace, 0, 0, 0);
      NextAttack(0);
   }
#endif

#if 0
   // DESTABILIZE MODE
   {
      if(trapped_sent)
      {
         ShootSentient(pos, dir);
         ammorequired = 0;
      }
      else
      {
         SuckSentient(pos, dir);
         ammorequired = 20;
      }
   }
   NextAttack(0.5);
#endif
}

qboolean QuantumDestabilizer::Drop()
{
#if 0
   ammorequired = 20;
   if(trapped_sent)
   {
      if(!ShootSentient(worldorigin, Vector(0, 0, 1)))
      {
         // Can't shoot the sentient out, so kill it
         trapped_sent->health = 0;
         trapped_sent->Damage(this, owner, 1000, worldorigin, vec_zero, vec_zero, 0, DAMAGE_NO_ARMOR | DAMAGE_NO_SKILL, MOD_ION_DESTRUCT, -1, -1, 1.0f);
      }
   }
#endif
   CancelEventsOfType(EV_Quantum_EatAmmo);
   CancelEventsOfType(EV_Quantum_Destruct);
   return(Weapon::Drop());
}

//###
void QuantumDestabilizer::SecondaryUse(Event *ev)
{
   // switch to the stinger pack
   owner->useWeapon("StingerPack");
}
//###

// EOF

