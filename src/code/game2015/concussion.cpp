/*
================================================================
CONCUSSION GUN
================================================================

Copyright (C) 2020 by Night Dive Studios, Inc.
All rights reserved.

See the license.txt file for conditions and terms of use for this code.
*/

#include "g_local.h"
#include "concussion.h"
#include "worldspawn.h"
#include "player.h"

#define MAX_CONCUSSION_AMMO 100
#define CONCUSSION_REGEN_TIME_DM 0.5
#define CONCUSSION_REGEN_TIME 0.6

//==================================================================

CLASS_DECLARATION(Entity, ConcussionRing, NULL);

Event EV_ConcussionRing_Animate2("concussionring_animate2");
Event EV_ConcussionRing_Animate3("concussionring_animate3");

ResponseDef ConcussionRing::Responses[] =
{
   { &EV_ConcussionRing_Animate2, (Response)&ConcussionRing::Animate2 },
   { &EV_ConcussionRing_Animate3, (Response)&ConcussionRing::Animate3 },
   { NULL, NULL }
};

void ConcussionRing::Animate2(Event *ev)
{
   edict->s.scale += 0.4;
   edict->s.alpha *= 0.6;
   edict->s.renderfx |= RF_TRANSLUCENT;
   PostEvent(EV_ConcussionRing_Animate2, FRAMETIME);
}

void ConcussionRing::Animate3(Event *ev)
{
   edict->s.scale += 0.1;
   edict->s.alpha *= 0.5;
   edict->s.renderfx |= RF_TRANSLUCENT;
   PostEvent(EV_ConcussionRing_Animate3, FRAMETIME);
}

void ConcussionRing::Setup(Vector pos, Vector dir)
{
   setMoveType(MOVETYPE_FLY);
   setSolidType(SOLID_NOT);
   setOrigin(pos);
   worldorigin.copyTo(edict->s.old_origin);

   angles = dir.toAngles();
   angles[PITCH] = -angles[PITCH];
   angles[ROLL ] = rand() % 360;
   avelocity[2] = crandom() * 120;
   setAngles(angles);

   setModel("sprites/concussion.spr");

   edict->s.renderfx |= RF_TRANSLUCENT;
}

//==================================================================

Event EV_Concussion_Effect("concussion_effect");
Event EV_Concussion_Regen("concussion_regen");

CLASS_DECLARATION(Weapon, ConcussionGun, "weapon_concussiongun");

ResponseDef ConcussionGun::Responses[] =
{
   { &EV_Weapon_Shoot,      (Response)&ConcussionGun::Shoot        },
   { &EV_Concussion_Effect, (Response)&ConcussionGun::BlastEffect  },
   { &EV_Concussion_Regen,  (Response)&ConcussionGun::BatteryRegen },
   { NULL, NULL }
};

ConcussionGun::ConcussionGun() : Weapon()
{
   SetModels("concussion_w.def", "view_concussion.def");
   modelIndex("sprites/concussion.spr");
   SetAmmo("ConcussionBattery", 10, MAX_CONCUSSION_AMMO);
   SetRank(75, 25);
   SetType(WEAPON_2HANDED_LO);

   SetMinRange(0);
   SetProjectileSpeed(0);
}

void ConcussionGun::SecondaryUse(Event *ev)
{
   // switch to the fists
   owner->useWeapon("Fists");
}

void ConcussionGun::BatteryRegen(Event *ev)
{
   // don't regen battery if owner is dead
   if(!owner)
      return;

   // only do for players
   if(!owner->isClient())
      return;

   // first make sure that we still have a living owner
   if(!owner || owner->health < 1)
   {
      CancelEventsOfType(EV_Concussion_Regen);
      return;
   }

   if(AmmoAvailable() < MAX_CONCUSSION_AMMO)
   {
      owner->giveItem(ammotype.c_str(), 2);

      // if still below max, then need to do it again
      if(AmmoAvailable() < MAX_CONCUSSION_AMMO)
      {
         if(deathmatch->value)
            PostEvent(EV_Concussion_Regen, CONCUSSION_REGEN_TIME_DM);
         else
            PostEvent(EV_Concussion_Regen, CONCUSSION_REGEN_TIME);
      }
   }
}

// start the concussion gun's firing sequence
void ConcussionGun::Shoot(Event *ev)
{
   assert(owner);

   Vector pos, dir, right;
   GetMuzzlePosition(&pos, &dir, &right);
   Vector end(pos - dir * 40);

   trace_t trace = G_Trace(pos, Vector(-1, -1, -1), Vector(1, 1, 1), end, owner, MASK_PLAYERSOLID, "ConcussionGun::Shoot");
   pos = trace.endpos;

   // do kick back
   end = pos + (dir * CONCUSSION_DIST * 0.5);
   trace = G_Trace(pos, Vector(-4, -4, -4), Vector(4, 4, 4), end, owner, MASK_PLAYERSOLID, "ConcussionGun::Shoot");
   
   float f = (1 - trace.fraction)*CONCUSSION_KICKBACK;
   if(f < 400)
      f = 400;

   // reduce upwards kickback in singleplayer
   if(!deathmatch->value)
   {
      owner->velocity.x -= dir.x*f;
      owner->velocity.y -= dir.y*f;
      if(dir.z < 0)
         owner->velocity.z -= dir.z*f*0.3;
      else
         owner->velocity.z -= dir.z*f*0.5;
   }
   else // in DM, let 'em fly
   {
      owner->velocity -= dir*f;
   }

   // store firing position & dir for later
   lastfirepos = pos;
   lastfiredir = dir;

   // make a narrow moving ring at the start of the blast
   pos += dir * 2 + right * 4;
   auto ring = new ConcussionRing();
   ring->Setup(pos, lastfiredir);
   ring->velocity = lastfiredir * 500;
   ring->edict->s.scale = 0.05;
   ring->edict->s.alpha = 0.75;
   ring->PostEvent(EV_ConcussionRing_Animate2, FRAMETIME);
   ring->PostEvent(EV_Remove, 1);

   NextAttack(1);

   blastcount = 0;
   ProcessEvent(EV_Concussion_Effect);

   // post the battery regenerating event
   if(owner->isClient())
   {
      CancelEventsOfType(EV_Concussion_Regen);
      PostEvent(EV_Concussion_Regen, CONCUSSION_REGEN_TIME);
   }
}

void ConcussionGun::BlastEffect(Event *ev)
{
   if(!owner)
   {
      weaponstate = WEAPON_READY;
      return;
   }

   Entity *tmpent = NULL;
   while((tmpent = findradius(tmpent, lastfirepos, CONCUSSION_DIST)))
   {
      if(tmpent == owner)
         continue;

      // only effect damagable things and projectiles
      // don't effect BSP model entities
      if(((!tmpent->takedamage) &&
         (!tmpent->isSubclassOf<Projectile>())) ||
          (tmpent->edict->solid == SOLID_BSP))
      {
         continue;
      }

      Vector targvec(tmpent->origin + (tmpent->mins + tmpent->maxs)*0.5);

      trace_t trace = G_Trace(lastfirepos, Vector(0, 0, 0), Vector(0, 0, 0), targvec, owner, MASK_SOLIDNONFENCE, "ConcussionGun::BlastEffect");
      if(trace.fraction < 1)
         continue;

      Vector vec(targvec - lastfirepos);
      float targdist = vec.normalize2();

      float d = DotProduct(vec, lastfiredir);
      if(d < CONCUSSION_ARC)
         continue;

      if(!deathmatch->value) // don't make extra rings in DM, too much net traffic
      {
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
      }

      float f = CONCUSSION_PUSH - (targdist * CONCUSSION_PUSH_DECAY);
      // scale the force by the directional dot product
      f *= d;
      // scale the force by the mass of the target
      f *= 1 / (tmpent->mass*0.005);
      if(f < 0)
         f = 0;

      //apply force, but not to script objects and models
      if((f > 0) && (!tmpent->isSubclassOf<ScriptSlave>()))
      {
         tmpent->velocity += vec * f;

         if(!deathmatch->value)
         {
            if(tmpent->velocity[2] <= 0)
               tmpent->velocity[2] = 50;
            else
               tmpent->velocity[2] += 50;
         }
         else
         {
            if(tmpent->velocity[2] <= 0)
               tmpent->velocity[2] = 75;
            else
               tmpent->velocity[2] += 75;
         }

         // set info for wall splatting damage
         tmpent->concussioner = owner;

         if(tmpent->isSubclassOf<Player>())
         {
            Player *player;

            player = (Player *)tmpent;
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
         tmpent->Damage(this, owner, f, tmpent->origin, lastfiredir, level.impact_trace.plane.normal, 32, 0, MOD_CONCUSSION, -1, -1, 1.0f);
   }


   // set effect counter as needed
   if(blastcount < CONCUSSION_COUNT)
   {
      // do effect again
      PostEvent(EV_Concussion_Effect, 0.1);
      blastcount++;
   }
}

// this stuff was added to prevent changing weapons
// when doesn't have enough ammo to fire

void ConcussionGun::Fire(void)
{
   if(!ReadyToFire())
      return;

   if(!(((ammo_clip_size && ammo_in_clip >= ammorequired) || AmmoAvailable() >= ammorequired)))
      return;

   UseAmmo(ammorequired);

   weaponstate = WEAPON_FIRING;

   CancelEventsOfType(EV_Weapon_DoneFiring);
   // this is just a precaution that we can re-trigger
   NextAttack(5);

   RandomAnimate("fire", EV_Weapon_DoneFiring);

   last_attack_time = level.time;
}

qboolean ConcussionGun::HasAmmo(void)
{
   if(!owner)
      return false;

   // always return TRUE from this
   return true;
}

//==================================================================
// And yae, from yonder, hither weapon doth seth fourth evil things...

class EXPORT_FROM_DLL EvilConcussionGun : public ConcussionGun
{
public:
   CLASS_PROTOTYPE(EvilConcussionGun);

   EvilConcussionGun();
   virtual void Shoot(Event *ev);
};

CLASS_DECLARATION(ConcussionGun, EvilConcussionGun, "weapon_evilconcussiongun");

ResponseDef EvilConcussionGun::Responses[] =
{
   {&EV_Weapon_Shoot, (Response)&EvilConcussionGun::Shoot},
   {NULL, NULL}
};

EvilConcussionGun::EvilConcussionGun() : ConcussionGun()
{
}

void EvilConcussionGun::Shoot(Event *ev)
{
   ConcussionGun::Shoot(ev);
   NextAttack(0.5);
   weaponstate = WEAPON_READY;
}

//EOF

