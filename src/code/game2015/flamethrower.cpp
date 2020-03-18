/*
================================================================
FLAMETHROWER
================================================================

Copyright (C) 2020 by Night Dive Studios, Inc.
All rights reserved.

See the license.txt file for conditions and terms of use for this code.
*/

#include "g_local.h"
#include "bullet.h"
#include "flamethrower.h"
#include "specialfx.h"
#include "player.h"
#include "surface.h"
#include "hoverbike.h"

#define FLAMETHROWER_MINBURST 0.8
#define FLAMETHROWER_MOVETOLERANCE 24
#define FLAMETHROWER_LENGTH 256
#define FLAMETHROWER_ARCFALLOFF 1
#define FLAMETHROWER_DAMAGE 2.2

#define FLAMETHROWER_DM_LENGTH 256
#define FLAMETHROWER_DM_ARCFALLOFF 0.5
#define FLAMETHROWER_DM_DAMAGE 1.75

//=================================================================
// ThrowerFlame is what does the actual damage for the Flamethrower

CLASS_DECLARATION(Entity, ThrowerFlame, NULL);

Event EV_ThrowerFlame_Burn("flame_burn");

ResponseDef ThrowerFlame::Responses[] =
{
   { &EV_ThrowerFlame_Burn, (Response)&ThrowerFlame::Burn },
   { NULL, NULL }
};

qboolean ThrowerFlame::CanToast(Entity *target, float arc)
{
   trace_t  trace;
   Vector   org, pos, v;
   float    dot, dist;
   qboolean in_arc;

   // these are used to set where
   // to spawn hit flames from
   hitpos = vec_zero;

   in_arc = false;

   // bmodels need special checking because their origin is 0,0,0
   if(target->getMoveType() == MOVETYPE_PUSH)
   {
      org = (target->absmin + target->absmax) * 0.5;
      v = org - origin;
      dist = v.normalize2();
      dot = DotProduct(dir.vec3(), v.vec3());
      if(dot > arc)
      {
         in_arc = true;
         hitpos = org;
      }

      trace = G_Trace(origin, vec_zero, vec_zero, org, this, MASK_SHOT, "ThrowerFlame::CanToast 1a");
      if((trace.fraction == 1 || trace.ent == target->edict) && in_arc)
         return true;
   }
   else
   {
      org = target->centroid;
      v = org - origin;
      dist = v.normalize2();
      dot = DotProduct(dir.vec3(), v.vec3());
      if(dot > arc)
      {
         in_arc = true;
         hitpos = org;
      }

      trace = G_Trace(origin, vec_zero, vec_zero, org, this, MASK_SHOT, "ThrowerFlame::CanToast 1b");
      if((trace.fraction == 1 || trace.ent == target->edict) && in_arc)
         return true;
   }

   // we didn't hit the center, so try the centers of the six sides
   for(int i = 0; i < 6; i++)
   {
      pos = org;
      switch(i)
      {
      case 0:
         pos.x = target->absmin.x;
         break;
      case 1:
         pos.x = target->absmax.x;
         break;
      case 2:
         pos.y = target->absmin.y;
         break;
      case 3:
         pos.y = target->absmax.y;
         break;
      case 4:
         pos.z = target->absmin.z;
         break;
      case 5:
         pos.z = target->absmax.z;
         break;
      }

      v = pos - origin;
      dist = v.normalize2();
      dot  = DotProduct(dir.vec3(), v.vec3());
      if(dot > arc)
      {
         in_arc = true;
         hitpos = pos;
      }

      trace = G_Trace(origin, vec_zero, vec_zero, pos, this, MASK_SHOT, "ThrowerFlame::CanToast 2");
      if((trace.fraction == 1 || trace.ent == target->edict) && in_arc)
      {
         return true;
      }
   }

   return false;
}

void ThrowerFlame::Burn(Event *ev)
{
   float  points;
   Vector org;
   Vector v;
   float  entdist;
   float  entarc, entdot;
   float  burnlength, arcfalloff, burndamage;

   Entity *ent = G_GetEntity(owner);
   if(!ent) // remove self if owner is gone
   {
      PostEvent(EV_Remove, 0.1);
      return;
   }

   if(deathmatch->value)
   {
      burnlength = FLAMETHROWER_DM_LENGTH;
      arcfalloff = FLAMETHROWER_DM_ARCFALLOFF;
      burndamage = FLAMETHROWER_DM_DAMAGE;
   }
   else
   {
      burnlength = FLAMETHROWER_LENGTH;
      arcfalloff = FLAMETHROWER_ARCFALLOFF;
      burndamage = FLAMETHROWER_DAMAGE;
   }

   // monsters do less damage with the flamethrower
   if(!ent->isClient())
   {
      burndamage *= 0.6;
   }

   ent = NULL;

   do
   {
      ent = findradius(ent, origin, burnlength);

      if(!ent)
         continue;

      if((ent == G_GetEntity(owner)) ||
         (!ent->takedamage) ||
         (ent->deadflag) ||
         (ent->isSubclassOf<HoverbikeBox>()))
      {
         continue;
      }

      org = ent->origin + (ent->mins + ent->maxs)*0.5;
      v = org - origin;
      entdist = v.normalize2();

      // adjust for length of different timming counts
      if(counter == 3)
      {
         if(entdist < 64)
            continue;
      }
      else if(counter < 3)
      {
         if(entdist < 128)
            continue;
      }

      entarc = DotProduct(dir.vec3(), v.vec3());

      // make sure entity is in front
      if(entarc < 0)
         continue;

      // determine amount of damage to do
      points = 1 - ((1 - entarc)*arcfalloff);
      points *= burndamage;

      if(points <= 0)
         continue;

      // see if entity is in damage arc
      if(entdist < 16)
      {
         entdot = 0.4;
      }
      else if(entdist < 32)
      {
         entdot = 0.7;
      }
      else if(entdist < 64)
      {
         entdot = 0.9;
      }
      else if(entdist < 128)
      {
         entdot = 0.95;
      }
      else if(entdist < 192)
      {
         entdot = 0.97;
      }
      else
      {
         entdot = 0.98;
      }

      if(!CanToast(ent, entdot))
         continue;

      ent->Damage(this, G_GetEntity(owner), (int)(points + 0.5), org, v, vec_zero, 0, DAMAGE_NO_ARMOR | DAMAGE_NO_KNOCKBACK, MOD_FLAMETHROWER, -1, -1, 1.0f);
      // make a bit of a visual cue that you hit something
      if(ent->isSubclassOf<Sentient>() && !ent->deadflag)
      {
         Sentient *sent = (Sentient *)ent;
         sent->CancelEventsOfType(EV_Sentient_HurtFlame);
         ent->edict->s.effects |= EF_FLAMES;
         sent->PostEvent(EV_Sentient_HurtFlame, 1);
      }
      else
      {
         v = hitpos + Vector(-6, -6, 0);
         org = hitpos + Vector(6, 6, 0);
         SpawnFlame(org, ent->velocity*0.5 + Vector(24, 24, 40), v, ent->velocity*0.5 + Vector(-24, -24, 40), 1, 154, 20);
         v.x = hitpos.x + 6;
         org.x = hitpos.x - 6;
         SpawnFlame(org, ent->velocity*0.5 + Vector(-24, 24, 40), v, ent->velocity*0.5 + Vector(24, 24, 40), 1, 154, 20);
      }

   }
   while(ent);

   counter--;
   if(counter)
      PostEvent(EV_ThrowerFlame_Burn, 0.1);
   else
      PostEvent(EV_Remove, 0.1);
}

void ThrowerFlame::Setup(Entity *owner, Vector pos, Vector streamend)
{
   this->owner  = owner->entnum;
   edict->owner = owner->edict;

   setMoveType(MOVETYPE_NONE);
   setSolidType(SOLID_NOT);

   hideModel();

   // make sure not to send this to clients
   edict->svflags |= SVF_NOCLIENT;

   setOrigin(pos);

   end = streamend;

   dir = end - origin;
   length = dir.normalize2();

   counter = 4;

   CancelEventsOfType(EV_ThrowerFlame_Burn);
   ProcessEvent(EV_ThrowerFlame_Burn);
}

//=======================================================

CLASS_DECLARATION(Weapon, FlameThrower, "weapon_flamethrower");

Event EV_FlameThrower_BlastTimer("flameblast_timer");

ResponseDef FlameThrower::Responses[] =
{
   { &EV_Weapon_Shoot,            (Response)&FlameThrower::Shoot },
   { &EV_FlameThrower_BlastTimer, (Response)&FlameThrower::BlastTimer },
   { NULL, NULL }
};

FlameThrower::FlameThrower() : Weapon()
{
   SetModels("flamethrower.def", "view_flame.def");
   SetAmmo("FlameFuel", 1, 100);
   modelIndex("flamethrowerfuel.def");
   SetRank(85, 45);
   SetType(WEAPON_2HANDED_LO);

   SetMinRange(0);
   SetMaxRange(250);
}

void FlameThrower::SecondaryUse(Event *ev)
{
   // switch to the assaultrifle
   owner->useWeapon("AssaultRifle");
}

void FlameThrower::BlastTimer(Event *ev)
{
   // check for owner dying or getting on a hoverbike
   if((!owner) || (owner->IsOnBike()))
   {
      CancelEventsOfType(EV_Weapon_Shoot);
      CancelEventsOfType(EV_FlameThrower_BlastTimer);
      weaponstate = WEAPON_READY;
      StopAnimating();
      return;
   }

   // only do this stuff for clients
   if(!owner->isClient())
      return;

   // check if we still need to keep the weapon firing by itself
   if((!owner->IsHoldingAttack()) && (blastcounter > level.time))
      Fire();

   // check if we need to keep doing this
   if(blastcounter > level.time && HasAmmo() && !ChangingWeapons())
      PostEvent(EV_FlameThrower_BlastTimer, 0.1);
}

void FlameThrower::Fire(void)
{
   if(!ReadyToFire())
      return;

   if(!HasAmmoInClip())
   {
      CheckReload();
      return;
   }

   UseAmmo(ammorequired);

   weaponstate = WEAPON_FIRING;

   CancelEventsOfType(EV_Weapon_DoneFiring);
   // this is just a precaution that we can re-trigger
   NextAttack(5);

   if(lastanimtime < level.time)
   {
      RandomAnimate("fire_sound", EV_Weapon_DoneFiring);
      lastanimtime = level.time + 0.4;
   }
   else
   {
      RandomAnimate("fire", EV_Weapon_DoneFiring);
   }

   last_attack_time = level.time;
}

void FlameThrower::Shoot(Event *ev)
{
   Vector tmpvec, dest;
   float length;
   trace_t trace;

   assert(owner);
   if(!owner)
      return;

   int mask = MASK_SHOT | MASK_WATER;

   Vector pos, dir, up, right;
   GetMuzzlePosition(&pos, &dir, &right, &up);

   tmpvec = pos + dir * 4 - up;
   pos -= dir * 4;
   trace = G_Trace(pos, Vector(-2, -2, -2), Vector(2, 2, 2), tmpvec, owner, mask, "FlameThrower::Shoot 1");
   pos = trace.endpos - dir * 2;

   // can't fire if underwater
   if(gi.pointcontents(pos.vec3()) & MASK_WATER)
   {
      if((level.time - lastfiretime) > 0.3) // starting a new stream, so init the needed stuff
      {
         // only do blast timmer for clients
         if(owner->isClient())
         {
            // init firing timer
            blastcounter = level.time + FLAMETHROWER_MINBURST;
            PostEvent(EV_FlameThrower_BlastTimer, 0.1);
         }
      }

      for(mask = 0; mask < 2; mask++)
      {
         auto bubble = new Bubble();
         bubble->Setup(pos);
         bubble->velocity += dir * 128;
      }

      flamelength  = 0;
      lastpos      = vec_zero;
      lastfiretime = level.time;

      NextAttack(0);
      return;
   }

   if((level.time - lastfiretime) > 0.3) // starting a new stream, so init the needed stuff
   {
      flamelength = 128;
      lastpos = vec_zero;

      // only do blast timmer for clients
      if(owner->isClient())
      {
         // init firing timer
         blastcounter = level.time + FLAMETHROWER_MINBURST;
         PostEvent(EV_FlameThrower_BlastTimer, 0.1);
      }
   }
   else // continuing a stream
   {
      flamelength += 128;
      if(deathmatch->value)
      {
         if(flamelength > FLAMETHROWER_DM_LENGTH)
            flamelength = FLAMETHROWER_DM_LENGTH;
      }
      else
      {
         if(flamelength > FLAMETHROWER_LENGTH)
            flamelength = FLAMETHROWER_LENGTH;
      }
   }

   dest = pos + dir*flamelength;
   trace = G_Trace(pos, Vector(-2, -2, -2), Vector(2, 2, 2), dest, owner, mask, "FlameThrower::Shoot 2");

   // make the flamethrower damage surfaces
   surfaceManager.DamageSurface(&trace, FLAMETHROWER_DAMAGE, owner);

   mask = MASK_SOLIDNONFENCE | MASK_WATER;
   trace = G_Trace(pos, Vector(-2, -2, -2), Vector(2, 2, 2), dest, owner, mask, "FlameThrower::Shoot 3");
   dest = trace.endpos;

   // set the damage entity
   if(!mainflame)
   {
      auto flame = new ThrowerFlame();
      mainflame = flame;
   }
   mainflame->Setup(owner, pos, dest);

   // make a flame splash for stream hiting something solid
   if(trace.fraction != 1)
   {
      tmpvec = dest - dir * 16;
      SpawnThrowerFlameHit(tmpvec, dir);
   }

   // see if we need to make movement streams
   if(lastpos != vec_zero)
   {
      tmpvec = lastdest - dest;
      length = tmpvec.length();

      if(length > FLAMETHROWER_MOVETOLERANCE)
      {
         Vector tmppos, tmpdest;
         float poslength;
         Vector posmove, destmove;

         tmpvec = lastpos - pos;
         poslength = tmpvec.length();
         length /= FLAMETHROWER_MOVETOLERANCE;
         poslength /= length;

         posmove = lastpos - pos;
         posmove.normalize();

         destmove = lastdest - dest;
         destmove.normalize();
         destmove *= FLAMETHROWER_MOVETOLERANCE;

         tmppos = pos;
         tmpdest = dest;

         while(length >= 1)
         {
            tmppos += posmove*poslength;
            tmpdest += destmove;

            // make a damage entity for this stream
            auto flame = new ThrowerFlame();
            flame->Setup(owner, tmppos, tmpdest);

            length--;
         }

         SpawnThrowerFlameRow(pos, dest, lastpos, lastdest);
      }
      else
         SpawnThrowerFlame(pos, dest);
   }
   else
      SpawnThrowerFlame(pos, dest);

   lastfiretime = level.time;
   lastpos = pos;
   lastdest = dest;

   NextAttack(0);
}

//==================================================================
// And yae, from yonder, hither weapon doth seth fourth evil things...
#include "actor.h"

class EXPORT_FROM_DLL EvilFlameThrower : public FlameThrower
{
public:
   CLASS_PROTOTYPE(EvilFlameThrower);

   EvilFlameThrower();
   virtual void Shoot(Event *ev);
};

CLASS_DECLARATION(FlameThrower, EvilFlameThrower, "weapon_evilflamethrower");

ResponseDef EvilFlameThrower::Responses[] =
{
   { &EV_Weapon_Shoot, (Response)&EvilFlameThrower::Shoot },
   { NULL, NULL }
};

EvilFlameThrower::EvilFlameThrower() : FlameThrower()
{
}

void EvilFlameThrower::Shoot(Event *ev)
{
   Vector tmpvec, dest;
   float length;
   trace_t trace;

   float entdelta, nearest;

   assert(owner);
   if(!owner)
      return;

   int mask = MASK_SHOT | MASK_WATER;

   Vector pos, dir, up, right;
   GetMuzzlePosition(&pos, &dir, &right, &up);
   tmpvec = pos + dir * 4 - up;
   pos -= dir * 4;

   // search for a sentient target within reach
   if(deathmatch->value)
      nearest = FLAMETHROWER_DM_LENGTH;
   else
      nearest = FLAMETHROWER_LENGTH;

   Sentient *eviltarget = NULL;
   int n = SentientList.NumObjects();
   for(int i = 1; i <= n; i++)
   {
      Sentient *ent = SentientList.ObjectAt(i);

      if(ent == owner)
         continue;

      if(ent->deadflag || ent->health <= 0 || !ent->takedamage)
         continue;

      if(!ent->isClient() && !ent->isSubclassOf<Actor>())
         continue;

      dest = ent->centroid - pos;
      entdelta = dest.length();

      if(entdelta < nearest)
      {
         nearest = entdelta;
         eviltarget = ent;
      }
   }

   if(eviltarget)
   {
      dir = eviltarget->centroid - pos;
      dir.normalize();
   }

   trace = G_Trace(pos, Vector(-2, -2, -2), Vector(2, 2, 2), tmpvec, owner, mask, "FlameThrower::Shoot 1");
   pos   = trace.endpos - dir * 2;

   if((level.time - lastfiretime) > 0.3) // starting a new stream, so init the needed stuff
   {
      flamelength = 128;
      lastpos = vec_zero;

      // only do blast timmer for clients
      if(owner->isClient())
      {
         // init firing timer
         blastcounter = level.time + FLAMETHROWER_MINBURST;
         PostEvent(EV_FlameThrower_BlastTimer, 0.1);
      }
   }
   else // continuing a stream
   {
      flamelength += 128;
      if(deathmatch->value)
      {
         if(flamelength > FLAMETHROWER_DM_LENGTH)
            flamelength = FLAMETHROWER_DM_LENGTH;
      }
      else
      {
         if(flamelength > FLAMETHROWER_LENGTH)
            flamelength = FLAMETHROWER_LENGTH;
      }
   }

   dest = pos + dir*flamelength;
   trace = G_Trace(pos, Vector(-2, -2, -2), Vector(2, 2, 2), dest, owner, mask, "FlameThrower::Shoot 2");

   // make the flamethrower damage surfaces
   surfaceManager.DamageSurface(&trace, FLAMETHROWER_DAMAGE, owner);

   mask = MASK_SOLIDNONFENCE | MASK_WATER;
   trace = G_Trace(pos, Vector(-2, -2, -2), Vector(2, 2, 2), dest, owner, mask, "FlameThrower::Shoot 3");
   dest = trace.endpos;

   // set the damage entity
   if(!mainflame)
   {
      auto flame = new ThrowerFlame();
      mainflame = flame;
   }
   mainflame->Setup(owner, pos, dest);

   // make a flame splash for stream hiting something solid
   if(trace.fraction != 1)
   {
      tmpvec = dest - dir * 16;
      SpawnThrowerFlameHit(tmpvec, dir);
   }

   // see if we need to make movement streams
   if(lastpos != vec_zero)
   {
      tmpvec = lastdest - dest;
      length = tmpvec.length();

      if(length > FLAMETHROWER_MOVETOLERANCE)
      {
         Vector tmppos, tmpdest;
         float poslength;
         Vector posmove, destmove;

         tmpvec = lastpos - pos;
         poslength = tmpvec.length();
         length /= FLAMETHROWER_MOVETOLERANCE;
         poslength /= length;

         posmove = lastpos - pos;
         posmove.normalize();

         destmove = lastdest - dest;
         destmove.normalize();
         destmove *= FLAMETHROWER_MOVETOLERANCE;

         tmppos = pos;
         tmpdest = dest;

         while(length >= 1)
         {
            tmppos += posmove*poslength;
            tmpdest += destmove;

            // make a damage entity for this stream
            auto flame = new ThrowerFlame();
            flame->Setup(owner, tmppos, tmpdest);

            length--;
         }

         SpawnThrowerFlameRow(pos, dest, lastpos, lastdest);
      }
      else
         SpawnThrowerFlame(pos, dest);
   }
   else
      SpawnThrowerFlame(pos, dest);

   lastfiretime = level.time;
   lastpos = pos;
   lastdest = dest;

   NextAttack(0);
}

// EOF

