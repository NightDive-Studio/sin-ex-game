/*
================================================================
PLASMA BOW WEAPON
================================================================

Copyright (C) 2020 by Night Dive Studios, Inc.
All rights reserved.

See the license.txt file for conditions and terms of use for this code.
*/

#include "crossbow.h"
#include "explosion.h"
#include "surface.h"
#include "jitter.h"
#include "misc.h"
#include "player.h"

//=================================================================
// plasma bow bolt class

CLASS_DECLARATION(Projectile, PBolt, NULL);

Event EV_PBolt_Explode("explode");
Event EV_PBolt_EnergyTrail("energytrail");

ResponseDef PBolt::Responses[] =
{
   { &EV_Touch,             (Response)&PBolt::BoltTouch   },
   { &EV_PBolt_Explode,     (Response)&PBolt::Explode     },
   { &EV_PBolt_EnergyTrail, (Response)&PBolt::EnergyTrail },
   { NULL, NULL }
};

// also used to do velocity adjustments
EXPORT_FROM_DLL void PBolt::EnergyTrail(Event *ev)
{
   Vector tmpvec;
   int    roll;
   Vector grav;

   PostEvent(EV_PBolt_EnergyTrail, 0.1);

   roll = angles[ROLL] + 35;
   grav[gravity_axis[gravaxis].z] = gravity * sv_gravity->value * FRAMETIME * 2 * gravity_axis[gravaxis].sign;
   tmpvec = velocity - grav;
   angles = tmpvec.toAngles();
   angles[PITCH] = -angles[PITCH];
   angles[ROLL] = roll;
   setAngles(angles);
}

void PBolt::Explode(Event *ev)
{
   Entity *owner = G_GetEntity(this->owner);

   float damg;
   if(!owner->isClient())
   {
      damg = 20 + (int)G_Random(10);
   }
   else
   {
      damg = 110 + (int)G_Random(15);
      if(!deathmatch->value)
         damg *= 1.5;
   }

   CreateExplosion(origin, damg, 0, false, this, owner, NULL, MOD_PLASMABOWSPLASH, 0.3, 0, ATTN_NORM, 0.2, 1.0, 0.2, 300, 0.6, 0.75);

   // make the visual explosion
   TempModel(NULL, origin, vec_zero, "sprites/cbshot.spr", 0, 3.0, 0.8f, TEMPMODEL_ANIMATE_ONCE | TEMPMODEL_ANIMATE_FAST, 2);

   damg = damg*0.2 + 32;
   SpawnBowExplosion(origin, damg);

   // make the view jitter for the explosion
   auto jitter = new RadiusJitter();
   jitter->Setup(origin, 150, 0.25, 0.2, 6, 9, 0.2, 4, 6);

   // make the explosion sound
   RandomPositionedSound(origin, "impact_largeplasmaexplosion", 1.0, CHAN_AUTO, ATTN_NORM);

   hideModel();
   PostEvent(EV_Remove, 0.1);
}

void PBolt::BoltTouch(Event *ev)
{
   int damg, num;
   float decay;
   Vector v;
   Vector norm;
   float f1, f2;

   Entity *other = ev->GetEntity(1);

   if(other)
   {
      if(other->isSubclassOf<Teleporter>())
         return;

      if(other->entnum == this->owner)
         return;
   }

   CancelEventsOfType(EV_PBolt_EnergyTrail);
   CancelEventsOfType(EV_PBolt_Explode);

   setSolidType(SOLID_NOT);
   setMoveType(MOVETYPE_NONE);
   if(HitSky())
   {
      PostEvent(EV_Remove, 0);
      return;
   }

   Entity *owner = G_GetEntity(this->owner);

   // here's the different explosion code for the bob's bolts
   if(!owner->isClient())
   {
      damg = 25 + (int)G_Random(10);

      if(other && other->takedamage)
      {
         other->Damage(this, owner, damg, origin, velocity, level.impact_trace.plane.normal, 32, 0, MOD_PLASMABOW, -1, -1, 1.0f);
      }

      surfaceManager.DamageSurface(&level.impact_trace, damg, owner);

      v = velocity;
      v.normalize();

      // don't do radius damage to the other, because all the damage
      // was done in the impact
      v = origin - v * 32;
      num = ((float)charge)*0.005;

      decay = 0.1 + ((float)charge)*0.0045;

      // make the visual explosion
      SpawnBowExplosion(v, 0);

      charge = 100; // for view jitter

      // make the view jitter for the explosion
      f1 = 6;
      f2 = 4;

      auto jitter = new RadiusJitter();
      jitter->Setup(v, damg + 40, 0.25, 0.2, f1, f1*1.5, 0.2, f2, f2*1.5);

      // make the explosion sound
      RandomPositionedSound(v, "impact_smallplasmaexplosion", 1.0, CHAN_AUTO, ATTN_NORM);

      hideModel();
      PostEvent(EV_Remove, 0.1);

      return;
   }

   v = velocity;
   v.normalize();

   // don't do radius damage to the other, because all the damage
   // was done in the impact
   v = worldorigin - v * 24;

   damg = 95 + (int)G_Random(30);
   if(!deathmatch->value)
      damg *= 1.5;

   // hit a damagable entity, let's blow up now
   if(other && other->takedamage)
   {
      other->Damage(this, owner, damg, origin, velocity, level.impact_trace.plane.normal, 32, 0, MOD_PLASMABOW, -1, -1, 1.0f);

      TempModel(NULL, v, vec_zero, "sprites/cbshot.spr", 0, 0.5, 0.8f, TEMPMODEL_ANIMATE_ONCE | TEMPMODEL_ANIMATE_FAST, 2);

      // make the view jitter for the explosion
      auto jitter = new RadiusJitter();
      jitter->Setup(v, 64, 0.25, 0.2, 6, 9, 0.2, 4, 6);

      RandomPositionedSound(v, "impact_smallplasmaexplosion", 1.0, CHAN_AUTO, ATTN_NORM);

      PostEvent(EV_Remove, 0.1);
      return;
   }

   // stick it into the wall
   setOrigin(v);
   velocity = vec_zero;

   surfaceManager.DamageSurface(&level.impact_trace, damg, owner);

   RandomAnimate("stick", NULL);

   // play sticking sound
   RandomSound("snd_stick", 1.0, CHAN_AUTO, ATTN_NORM);

   PostEvent(EV_PBolt_Explode, 1);
}

EXPORT_FROM_DLL void PBolt::Setup(Entity *owner, Vector pos, Vector dir, int firecharge)
{
   float fltcharge;

   this->owner = owner->entnum;
   edict->owner = owner->edict;

   // set energy bolt's charge amount
   charge = firecharge;
   fltcharge = charge;

   setMoveType(MOVETYPE_TOSS);
   setSolidType(SOLID_BBOX);
   edict->clipmask = MASK_SHOT;

   SetGravityAxis(owner->gravaxis);

   angles = dir.toAngles();
   angles[PITCH] = -angles[PITCH];
   angles[ROLL] = 45;
   setAngles(angles);

   if(charge < 0)
   {
      if(owner->isClient())
         speed = 1800;
      else
         speed = 1000;
   }
   else
   {
      speed = 600 + 14 * charge;
      if(charge > 100)
         speed *= 0.5;
   }
   velocity = dir * speed;

   PostEvent(EV_PBolt_EnergyTrail, 0.1);

   // set missile duration
   auto ev = new Event(EV_PBolt_Explode);
   ev->AddEntity(world);
   if(charge > 100) // overloaded, so blow mid-air
      PostEvent(ev, 0);
   else
      PostEvent(ev, 10);

   if(owner->isClient())
      setModel("pbolt.def");
   else
      setModel("bobbolt.def");
   edict->s.renderfx |= RF_DLIGHT;

   // pick the effect trail to use
   if(charge < 0)
   {
      edict->s.effects |= EF_PLASMATRAIL2;

      gravity = 0.15;
      edict->s.color_r = 0.5;
      edict->s.color_g = 1;
      edict->s.color_b = 0.5;
      edict->s.radius = 100;
   }
   else
   {
      edict->s.effects |= EF_PLASMATRAIL1;

      gravity = 0.6 - fltcharge*0.004;
      // adjust bolt glow according to charge
      edict->s.color_r = fltcharge*0.003;
      edict->s.color_g = 1;
      edict->s.color_b = fltcharge*0.003;
      if(charge > 100) // overloaded, so blow mid-air
         edict->s.radius = 200;
      else
         edict->s.radius = 150;
   }

   setSize({ -2, -2, -2 }, { 2, 2, 2 });
   setOrigin(pos);
   worldorigin.copyTo(edict->s.old_origin);
}

//=================================================================
// plasmabow weapon class

Event EV_PlasmaBow_ReleaseCheck("plasmabow_releasecheck");
Event EV_PlasmaBow_Full("plasmabow_full");

CLASS_DECLARATION(Weapon, PlasmaBow, "weapon_plasmabow");

ResponseDef PlasmaBow::Responses[] =
{
   // charge release check
   { &EV_PlasmaBow_ReleaseCheck, (Response)&PlasmaBow::ReleaseCheck},
   // retain full charge
   { &EV_PlasmaBow_Full,         (Response)&PlasmaBow::FullRetain },
   // actual firing from button release
   { &EV_Weapon_Shoot,           (Response)&PlasmaBow::Shoot },
   { NULL, NULL }
};

PlasmaBow::PlasmaBow()
{
   SetModels("crossbow_w.def", "view_crossbow.def");
   modelIndex("pbolt.def");
   modelIndex("sprites/cbshot.spr");
   SetAmmo("BulletPulse", 5, 90);
   SetSecondaryAmmo("BulletPulse", 10, 0);
   SetRank(65, 75);
   SetType(WEAPON_2HANDED_HI);
   dualmode = true;
   primary_ammo_type = "BulletPulse";
   secondary_ammo_type = "BulletPulse";

   SetMinRange(50);
   SetProjectileSpeed(500);
}

void PlasmaBow::SecondaryUse(Event *ev)
{
   // switch to the shotgun
   owner->useWeapon("Shotgun");
}

// called when attack button is first pressed
void PlasmaBow::Fire(void)
{
   if(!ReadyToFire())
      return;

   if(!HasAmmoInClip())
   {
      CheckReload();
      return;
   }

   // charge fire mode
   if(weaponmode == PRIMARY)
   {
      UseAmmo(5);
      drainammo = 1;

      // init. charging state
      weaponstate = WEAPON_CHARGING;
      fullcycles  = MAX_FULL_CYCLES;
      chargetime  = level.time;

      CancelEventsOfType(EV_Weapon_DoneFiring);
      RandomAnimate("charge", EV_PlasmaBow_Full);
      // check for button release every 0.1 seconds
      PostEvent(EV_PlasmaBow_ReleaseCheck, 0.1);
      // this is just a precaution that we can re-trigger
      NextAttack(5);
   }
   else // instant fire mode
   {
      UseAmmo(secondary_ammorequired);

      weaponstate = WEAPON_FIRING;

      chargetime = level.time + 10;

      CancelEventsOfType(EV_Weapon_DoneFiring);
      RandomAnimate("fire", EV_Weapon_DoneFiring);
      // this is just a precaution that we can re-trigger
      NextAttack(5);
   }

   last_attack_time = level.time;
}

void PlasmaBow::ReleaseCheck(Event *ev)
{
   // check for owner dying or getting on a hoverbike
   if((!owner) || (this != owner->CurrentWeapon()) || owner->GetHoverbike() ||
      (deathmatch->value == DEATHMATCH_MFD && owner->isClient() && owner->client->resp.informer))
   {
      //cancel weapon charging
      CancelEventsOfType(EV_PlasmaBow_ReleaseCheck);
      CancelEventsOfType(EV_PlasmaBow_Full);
      weaponstate = WEAPON_READY;
      StopAnimating();
      DetachFromOwner();
      return;
   }

   // check for attack button release for players
   // also fire if you run out of ammo to charge up
   if(!owner->IsHoldingAttack())
   {
      CancelEventsOfType(EV_PlasmaBow_Full);

      weaponstate = WEAPON_FIRING;
      RandomAnimate("fire", EV_Weapon_DoneFiring);

      return;
   }

   PostEvent(EV_PlasmaBow_ReleaseCheck, 0.1);
}

void PlasmaBow::FullRetain(Event *ev)
{
   // check for owner dying or getting on a hoverbike
   if((!owner) || owner->GetHoverbike())
   {
      CancelEventsOfType(EV_PlasmaBow_ReleaseCheck);
      CancelEventsOfType(EV_PlasmaBow_Full);
      weaponstate = WEAPON_READY;
      StopAnimating();
      return;
   }

   RandomAnimate("full", EV_PlasmaBow_Full);
}

// release button function
void PlasmaBow::Shoot(Event *ev)
{
   Vector pos, dir, r, u;
   float boltcharge;

   assert(owner);
   if(!owner)
      return;

   // calc. charge amount
   boltcharge = level.time - chargetime;
   if(boltcharge < 0)
   {
      boltcharge = -1;
   }
   else if(boltcharge >= (CHARGE_TIME + MAX_FULL_CYCLES * 2))
   {
      boltcharge = 100; //bolt is overloaded
   }
   else
   {
      boltcharge = floor((boltcharge / CHARGE_TIME) * 100);
      if(boltcharge > 100)
         boltcharge = 100;
   }

   if(boltcharge >= 0)
   {
      // put owner into a firing animation
      if(owner->isSubclassOf<Player>())
      {
         str name;
         Player *player;

         player = (Player *)owner.ptr;

         str prefix(player->AnimPrefixForPlayer());
         //
         // append the prefix based on which weapon we are holding
         //
         prefix += player->AnimPrefixForWeapon();

         if((player->GetXYspeed() > 20))
         {
            int num;
            int numframes;

            name = prefix;
            if((player->waterlevel > 2) || (player->GetXYspeed() > 250))
               name += "run_fire";
            else
               name += "walk_fire";
            num = gi.Anim_Random(player->edict->s.modelindex, name.c_str());
            if(num != -1)
            {
               numframes = gi.Anim_NumFrames(player->edict->s.modelindex, num);
               if((player->last_frame_in_anim + 1) == numframes)
                  player->edict->s.anim = num;
               else
                  player->TempAnim(name.c_str(), NULL);
            }
            else
            {
               name = prefix;
               name += "fire";
               player->TempAnim(name.c_str(), NULL);
            }
         }
         else
         {
            name = prefix;
            name += "fire";
            player->TempAnim(name.c_str(), NULL);
         }
      }
      else
      {
         owner->TempAnim("fire", NULL);
      }
   }


   GetMuzzlePosition(&pos, &dir, &r, &u);
   pos -= (r * 4 + u * 4);

   auto bolt = new PBolt();
   bolt->Setup(owner, pos, dir, boltcharge);

   if(boltcharge < 0)
      NextAttack(1.2);
   else if(fullcycles == 0)
      NextAttack(1.5);
   else
      NextAttack(0.7);
}

//==================================================================
// And yae, from yonder, hither weapon doth seth fourth evil things...

class EXPORT_FROM_DLL EvilPlasmaBow : public PlasmaBow
{
public:
   CLASS_PROTOTYPE(EvilPlasmaBow);

   EvilPlasmaBow();
   virtual void Shoot(Event *ev);
   virtual void Fire(void);
};

CLASS_DECLARATION(PlasmaBow, EvilPlasmaBow, "weapon_evilplasmabow");

ResponseDef EvilPlasmaBow::Responses[] =
{
   { &EV_Weapon_Shoot, (Response)&EvilPlasmaBow::Shoot },
   { NULL, NULL }
};

EvilPlasmaBow::EvilPlasmaBow() : PlasmaBow()
{
}

void EvilPlasmaBow::Fire(void)
{
   if(!ReadyToFire())
      return;

   if(!HasAmmoInClip())
   {
      CheckReload();
      return;
   }

   UseAmmo(5);

   // init. charging state
   fullcycles = MAX_FULL_CYCLES;
   chargetime = level.time - CHARGE_TIME;

   CancelEventsOfType(EV_Weapon_DoneFiring);
   weaponstate = WEAPON_READY;
   RandomAnimate("fire", EV_Weapon_DoneFiring);

   // this is just a precaution that we can re-trigger
   NextAttack(5);

   last_attack_time = level.time;
}

void EvilPlasmaBow::Shoot(Event *ev)
{
   PlasmaBow::Shoot(ev);
   NextAttack(0.1);
}

// EOF

