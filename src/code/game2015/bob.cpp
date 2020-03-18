/*
================================================================
BOB MONSTER
================================================================

Copyright (C) 2020 by Night Dive Studios, Inc.
All rights reserved.

See the license.txt file for conditions and terms of use for this code.
*/

#include "bob.h"
#include "specialfx.h"
#include "player.h"

//Vector bobfieldmin (-64, -64, -64);
//Vector bobfieldmax (64, 64, 64);

#define BOB_CONCUSSION_RANGE 175

//===============================================================
// BOB
//===============================================================

Event EV_Bob_Concussion("fireconcussion");

CLASS_DECLARATION(Actor, Bob, "monster_bob");

ResponseDef Bob::Responses[] =
{
   { &EV_Bob_Concussion,    (Response)&Bob::FireConcussion   },
   { &EV_Concussion_Effect, (Response)&Bob::ConcussionEffect },
   { &EV_Killed,            (Response)&Bob::Killed           },

   { NULL, NULL }
};

Bob::Bob() : Actor()
{
   concussiontime = level.time + 2;
}

void Bob::Prethink(void)
{
   // do a check for something to concussion
   if(concussiontime < level.time)
   {
      Entity *ent = NULL;

      while((ent = findradius(ent, worldorigin, BOB_CONCUSSION_RANGE)))
      {
         if(ent == this)
            continue;

         if(!ent->isSubclassOf<Projectile>() &&
            !(ent->isSubclassOf<Player>() && ent->takedamage))
         {
            continue;
         }

         if(ent->isSubclassOf<Projectile>()) // do an extra check for Projectiles
         {
            Entity *entowner = ((Projectile *)ent)->Owner();
            assert(entowner);

            if(entowner->isSubclassOf<Bob>())
               continue;
         }

         // make sure that it's infront
         if(InConcussionFOV(ent->centroid) && CanSeeFrom(worldorigin, ent))
         {
            FireConcussion(NULL);
            concussiontime = level.time + 1.5;
            break; // stop looking for a reason to fire
         }
      }

      // if we didn't fire at anything, add a bit of a recheck delay
      if(concussiontime < level.time)
         concussiontime = level.time + 0.15;
   }

   //
   // call our superclass
   //
   Actor::Prethink();
}

void Bob::Killed(Event *ev)
{
   // make Bob go down in flames
   edict->s.effects |= EF_DEATHFLAMES;

   // call actual killed function
   Actor::Killed(ev);
}

inline qboolean Bob::InConcussionFOV(Vector pos)
{
   Vector delta;
   float dot;

   delta = pos - EyePosition();
   if(!delta.x && !delta.y)
   {
      // special case for straight up and down
      return true;
   }

   // give better vertical vision
   delta.z = 0;

   delta.normalize();
   dot = DotProduct(orientation[0], delta.vec3());

   return (dot > CONCUSSION_ARC);
}

void Bob::FireConcussion(Event *ev)
{
   lastfirepos = worldorigin;
   lastfiredir = Vector(orientation[0]);

   // apply kick back
   velocity -= lastfiredir * CONCUSSION_KICKBACK * 0.25;

   // make blast ring
   Vector pos(lastfirepos + lastfiredir * 8);
   auto ring = new ConcussionRing();
   ring->Setup(pos, lastfiredir);
   ring->velocity = lastfiredir * 500;
   ring->edict->s.scale = 0.05;
   ring->edict->s.alpha = 0.75;
   ring->PostEvent(EV_ConcussionRing_Animate2, FRAMETIME);
   ring->PostEvent(EV_Remove, 1);

   // make firing sound
   sound("weapons/concussion/fire5b.wav", 1, CHAN_WEAPON);

   blastcount = 0;
   ProcessEvent(EV_Concussion_Effect);
}

void Bob::ConcussionEffect(Event *ev)
{
   Vector  targvec, vec;
   trace_t trace;
   float   d, f, targdist;

   if(health <= 0)
      return;

   Entity *tmpent = NULL;
   while((tmpent = findradius(tmpent, lastfirepos, CONCUSSION_DIST * 0.75)) != 0)
   {
      if(tmpent == this)
         continue;

      // only effect damagable things and projectiles
      // don't effect BSP model entities
      if(((!tmpent->takedamage) &&
         (!tmpent->isSubclassOf<Projectile>())) ||
          (tmpent->edict->solid == SOLID_BSP))
      {
         continue;
      }

      // don't effect bob projectiles
      if(tmpent->isSubclassOf<Projectile>())
      {
         Entity *entowner = ((Projectile *)tmpent)->Owner();
         assert(entowner);

         if(entowner->isSubclassOf<Bob>())
            continue;
      }

      targvec = tmpent->origin + (tmpent->mins + tmpent->maxs)*0.5;

      trace = G_Trace(lastfirepos, Vector(0, 0, 0), Vector(0, 0, 0), targvec, this, MASK_SOLIDNONFENCE, "ConcussionGun::BlastEffect");
      if(trace.fraction < 1)
         continue;

      vec = targvec - lastfirepos;
      targdist = vec.normalize2();

      d = DotProduct(vec, lastfiredir);
      if(d < CONCUSSION_ARC)
         continue;

      if(tmpent->concussion_debounce < level.time)
      {
         // make a visible indication of a hit
         Vector end(targvec - lastfiredir * 32);
         auto ring = new ConcussionRing();
         ring->Setup(end, lastfiredir);
         ring->velocity = lastfiredir * 50;
         ring->edict->s.scale = 0.02;
         ring->edict->s.alpha = 0.75;
         ring->PostEvent(EV_ConcussionRing_Animate3, FRAMETIME);
         ring->PostEvent(EV_Remove, 0.4);
      }
      tmpent->concussion_debounce = level.time + 0.3;

      f = (CONCUSSION_PUSH * 0.9) - (targdist * CONCUSSION_PUSH_DECAY);
      // scale the force by the directional dot product
      f *= d;
      // scale the force by the mass of the target
      if(tmpent->mass > 20)
         f *= 1 / (tmpent->mass*0.005);
      else
         f = 2000;

      if(f < 0)
         f = 0;
      if(f > 2000)
         f = 2000;

      //apply force, but not to script objects and models
      if((f > 0) && (!tmpent->isSubclassOf<ScriptSlave>()))
      {
         tmpent->velocity += vec * f;

         if(tmpent->velocity.length() > 1500)
         {
            tmpent->velocity.normalize();
            tmpent->velocity *= 1500;
         }

         if(tmpent->velocity[2] <= 0)
            tmpent->velocity[2] = 50;
         else
            tmpent->velocity[2] += 50;

         // set info for wall splatting damage
         tmpent->concussioner = this;

         if(tmpent->isSubclassOf<Player>())
         {
            Player *player = (Player *)tmpent;

            player->concussion_timer = level.time + 1;
            // jitter the poor sucker's view
            f = f / CONCUSSION_PUSH;
            player->SetAngleJitter(f * 10, f * 30, level.time);
            player->SetOffsetJitter(f * 5, f * 20, level.time + 0.1);
         }
      }

      // do some damage if applicable
      f = CONCUSSION_DAMG - (targdist * CONCUSSION_DAMG_DECAY);
      // scale the damage by the directional dot product
      f *= d;
      if(f < 0)
         f = 0;
      if(tmpent->takedamage)
         tmpent->Damage(this, this, f, tmpent->origin, lastfiredir, level.impact_trace.plane.normal, 32, 0, MOD_CONCUSSION, -1, -1, 1.0f);
   }

   // set effect counter as needed
   if(blastcount < CONCUSSION_COUNT)
   {
      // do effect again
      PostEvent(EV_Concussion_Effect, 0.1);
      blastcount++;
   }
}

//===============================================================
// BOB'S WEAPON
//===============================================================

CLASS_DECLARATION(Weapon, BobBow, "");

ResponseDef BobBow::Responses[] =
{
   { &EV_Weapon_Shoot, (Response)&BobBow::Shoot },
   { NULL, NULL }
};

BobBow::BobBow() : Weapon()
{
   SetModels(NULL, "view_bobbow.def");
   modelIndex("bobbolt.def");
   modelIndex("sprites/cbshot.spr");
   SetAmmo("BulletPulse", 10, 90);
   SetType(WEAPON_2HANDED_HI);

   SetMinRange(50);
   SetProjectileSpeed(1000);
}

void BobBow::Shoot(Event *ev)
{
   assert(owner);
   if(!owner)
      return;

   Vector pos, dir;
   GetMuzzlePosition(&pos, &dir, NULL, NULL);

   auto bolt = new PBolt();
   bolt->Setup(owner, pos, dir, -1);

   NextAttack(1.8);
}

// EOF

