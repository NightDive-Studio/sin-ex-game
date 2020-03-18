/*
================================================================
STINGER PACK WEAPON
================================================================

Copyright (C) 2020 by Night Dive Studios, Inc.
All rights reserved.

See the license.txt file for conditions and terms of use for this code.
*/

#include "g_local.h"
#include "explosion.h"
#include "rocketpack.h"
#include "worldspawn.h"
#include "surface.h"
#include "misc.h"
#include "actor.h"

#define STINGERROCKET_SPEED  1000
#define STINGERROCKET_RADIUS 120

//=================================================================
// anime rocket class

CLASS_DECLARATION(Projectile, StingerRocket, nullptr);

Event EV_StingerRocket_Explode("explode");
Event EV_StingerRocket_Turn("turn");

ResponseDef StingerRocket::Responses[] =
{
   { &EV_Touch,                 (Response)&StingerRocket::Explode },
   { &EV_StingerRocket_Explode, (Response)&StingerRocket::Explode },
   { &EV_StingerRocket_Turn,    (Response)&StingerRocket::Turn    },
   { nullptr, nullptr }
};

// also used to do velocity adjustments
EXPORT_FROM_DLL void StingerRocket::Turn(Event *ev)
{
   Vector tmpvec, tmpvel;
   int tmpof;
   float tmplength;

   tmpvec = targpos - origin;
   tmplength = tmpvec.length();
   // check for reaching current target position
   if(tmplength < 90)
   {
      // advance primary destination position
      destpos = destpos + movedir*300;

      // get new target position
      for(int i = 0; i < 3; i++)
      {
         if(rand()%4)
            offsets[i] *= -1;
         tmpof = 8 + random()*32;
         targpos[i] = destpos[i] + offsets[i] * (float)tmpof;
      }
   }

   // set velocity towards target position if
   // direction is different enough
   tmpvec = targpos - origin;
   tmpvec.normalize();
   tmpvel = velocity;
   tmpvel.normalize();

   // make the stinger rockets do fewer corrections in deathamtch
   if(deathmatch->value)
   {
      if(DotProduct(tmpvec.vec3(), tmpvel.vec3()) < 0.96)
      {
         velocity = tmpvel*2 + tmpvec*3;
         velocity.normalize();
         velocity *= speed;
      }
   }
   else
   {
      velocity = tmpvel*2 + tmpvec*3;
      velocity.normalize();
      velocity *= speed;

      // constantly correct the rocket's angles in single player
      angles = tmpvec.toAngles();
      angles[PITCH] = -angles[PITCH];
      setAngles(angles);
   }

   PostEvent(EV_StingerRocket_Turn, 0.1);
}

EXPORT_FROM_DLL void StingerRocket::Explode(Event *ev)
{
   int damg;
   Vector v;
   Entity *other;
   Vector norm;
   Entity *owner;

   other = ev->GetEntity(1);
   assert(other);

   if(other->isSubclassOf<Teleporter>())
      return;

   if(other->entnum == this->owner)
      return;

   CancelEventsOfType(EV_StingerRocket_Turn);

   stopsound(CHAN_VOICE);

   setSolidType(SOLID_NOT);
   hideModel();
   if(HitSky())
   {
      PostEvent(EV_Remove, 0);
      return;
   }

   owner = G_GetEntity(this->owner);

   damg = 70 + (int)G_Random(10);

   if(!deathmatch->value && owner->isClient())
      damg *= 1.5;

   if(other->takedamage)
      other->Damage(this, owner, damg, origin, velocity, level.impact_trace.plane.normal, 16, 0, MOD_STINGERROCKET, -1, -1, 1.0f);

   SpawnBlastDamage(&level.impact_trace, damg, owner);

   v = velocity;
   v.normalize();

   // don't do radius damage to the other, because all the damage
   // was done in the impact
   v = origin - v * 24;
   CreateExplosion(v, damg, 0.75f, true, this, owner, other, MOD_STINGERSPLASH, 0.3);
   PostEvent(EV_Remove, 0.1);
}

EXPORT_FROM_DLL void StingerRocket::Setup(Entity *owner, Vector pos, Vector firedest, Vector dir, Vector firedir)
{
   int tmpof;

   this->owner = owner->entnum;
   edict->owner = owner->edict;

   setMoveType(MOVETYPE_FLYMISSILE);
   setSolidType(SOLID_BBOX);
   edict->clipmask = MASK_PROJECTILE;

   angles = dir.toAngles();
   angles[PITCH] = -angles[PITCH];
   setAngles(angles);

   speed = STINGERROCKET_SPEED;
   velocity = dir * STINGERROCKET_SPEED;

   PostEvent(EV_StingerRocket_Turn, 0.1);

   // set missile duration
   auto ev1 = new Event(EV_Remove);
   ev1->AddEntity(world);
   PostEvent(ev1, 20);

   setModel("stinger.def");
   edict->s.effects  |= EF_ANIMEROCKET;
   edict->s.effects  |= EF_EVERYFRAME;
   edict->s.angles[ROLL] = rand() % 360;
   gravity = 0;

   // give it a heat signature
   edict->s.effects |= EF_WARM;

   // setup movement stuff
   movedir = firedir;
   destpos = firedest;
   for(int i = 0; i < 3; i++)
   {
      if(rand()%2)
         offsets[i] = 1;
      else
         offsets[i] = -1;

      tmpof = 16 + random()*24;
      targpos[i] = destpos[i] + offsets[i]*(float)tmpof;
   }

   // setup ambient thrust
   auto ev2 = new Event(EV_RandomEntitySound);
   ev2->AddString("thrust");
   ProcessEvent(ev2);

   // set the rocket's angles
   angles = movedir.toAngles();
   angles[PITCH] = -angles[PITCH];
   setAngles(angles);

   setSize({ -1, -1, -1 }, { 1, 1, 1 });
   setOrigin(pos);
   worldorigin.copyTo(edict->s.old_origin);
}

//=================================================================
// stingerpack weapon class

CLASS_DECLARATION(Weapon, StingerPack, "weapon_stingerpack");

ResponseDef StingerPack::Responses[] =
{
   { &EV_Weapon_Shoot, (Response)&StingerPack::Shoot },
   { nullptr, nullptr }
};

StingerPack::StingerPack() : Weapon()
{
   SetModels("pack_w.def", "view_pack.def");
   modelIndex("stinger.def");
   SetAmmo("Rockets", 2, 10);
   SetRank(78, 85);
   SetType(WEAPON_1HANDED);

   SetMinRange(STINGERROCKET_RADIUS);
   SetProjectileSpeed(STINGERROCKET_SPEED);

   rocketnum = 1;
}

void StingerPack::SecondaryUse(Event *ev)
{
   // switch to the quantum
   owner->useWeapon("QuantumDestabilizer");
}

void StingerPack::Shoot(Event *ev)
{
   StingerRocket *rocket;
   Vector pos;
   Vector end;
   trace_t trace;
   Vector targ, dir;
   Vector forward, right, up;

   if(!owner)
      return;

   if(owner->isClient())
   {
      const gravityaxis_t &grav = gravity_axis[owner->gravaxis];

      // get orientation according to gravityaxis
      forward[grav.x] = owner->orientation[0][0];
      forward[grav.y] = owner->orientation[0][1]*grav.sign;
      forward[grav.z] = owner->orientation[0][2]*grav.sign;
      right  [grav.x] = owner->orientation[1][0];
      right  [grav.y] = owner->orientation[1][1]*grav.sign;
      right  [grav.z] = owner->orientation[1][2]*grav.sign;
      up     [grav.x] = 0;
      up     [grav.y] = 0;
      up     [grav.z] = 1*grav.sign;

      // set firing position and direction
      pos = owner->origin - forward*12 + up*owner->viewheight;
      end = pos + forward*256;
      dir = forward;
   }
   else
   {
      GetMuzzlePosition(&pos, &forward, &right);
      up = Vector(0, 0, 1);

      // set firing position and direction
      pos = owner->origin - forward*12 + up*(owner->maxs.z*0.75);
      if(owner->isSubclassOf<Actor>())
      {
         if(static_cast<Actor *>(owner.ptr)->currentEnemy)
         {
            Vector targpos;

            targpos = static_cast<Actor *>(owner.ptr)->currentEnemy->centroid;

            // lead target if not on easy skill
            if(skill->value && G_Random() < 0.7)
            {
               switch(rocketnum)
               {
               case 1:
                  targpos += static_cast<Actor *>(owner.ptr)->currentEnemy->velocity*0.25;
                  break;
               case 2:
                  targpos += static_cast<Actor *>(owner.ptr)->currentEnemy->velocity*0.5;
                  break;
               case 3:
                  targpos += static_cast<Actor *>(owner.ptr)->currentEnemy->velocity*0.75;
                  break;
               case 4:
                  targpos += static_cast<Actor *>(owner.ptr)->currentEnemy->velocity;
                  break;
               }
            }

            forward = targpos - pos;
            forward.normalize();
         }
      }
      end = pos + forward*256;
      dir = forward;
   }


   switch(rocketnum)
   {
   case 1:
      pos += (right*12 - up*4);
      dir += (right*0.4 + up*0.2);
      break;

   case 2:
      pos += (right*6 + up*6);
      dir += (right*0.2 + up*0.4);
      break;

   case 3:
      pos -= (right*6 - up*6);
      dir -= (right*0.2 - up*0.4);
      break;

   case 4:
      pos -= (right*12 + up*4);
      dir -= (right*0.4 - up*0.2);
      break;
   }

   dir.normalize();

   //set starting position so that they won't go into walls
   trace = G_Trace(owner->origin + up*owner->viewheight, Vector(-1, -1, -1), Vector(1, 1, 1), pos, owner, MASK_PROJECTILE, "StingerPack::Shoot");
   pos = Vector(trace.endpos);

   // set target location
   trace = G_Trace(owner->origin + up*owner->viewheight, vec_zero, vec_zero, end, owner, MASK_PROJECTILE, "StingerPack::Shoot");
   targ = Vector(trace.endpos);

   rocket = new StingerRocket();
   rocket->Setup(owner, pos, targ, dir, forward);

   // force player to fire four at a time
   if(rocketnum < 4)
      PostEvent(EV_Weapon_Shoot, 0.1);

   if(owner->isClient())
      NextAttack(1.2);
   else
      NextAttack(2.0);

   rocketnum++;
   if(rocketnum > 4)
      rocketnum = 1;
}

void StingerPack::DetachGun()
{
   if(attached)
   {
      RandomGlobalSound("null_sound", 1, CHAN_WEAPONIDLE);
      RandomGlobalSound("null_sound", 1, CHAN_WEAPON);
      attached = false;
      detach();
      hideModel();
      edict->s.gunmodelindex = 0;
      edict->s.gunanim       = 0;
      edict->s.gunframe      = 0;
      edict->s.effects      &= ~EF_SMOOTHANGLES;
      edict->s.effects      &= ~EF_WARM;
   }
}

void StingerPack::AttachGun()
{
   int groupindex;
   int tri_num;
   Vector orient;

   if(!owner)
   {
      return;
   }

   if(attached)
      DetachGun();

   if(gi.GetBoneInfo(owner->edict->s.modelindex, "pack", &groupindex, &tri_num, orient.vec3()))
   {
      attached = true;
      attach(owner->entnum, groupindex, tri_num, orient);
      showModel();
      setOrigin(vec_zero);
      edict->s.gunmodelindex	= modelIndex(worldmodel.c_str());
      if(edict->s.gunmodelindex)
      {
         edict->s.gunanim        = gi.Anim_Random(edict->s.gunmodelindex, "idle");
         if(edict->s.gunanim < 0)
            edict->s.gunanim = 0;
         edict->s.gunframe = 0;
      }
      else
      {
         edict->s.gunanim = 0;
         edict->s.gunframe = 0;
      }
      edict->s.effects |= EF_SMOOTHANGLES;
      edict->s.effects |= EF_WARM;
   }
   else
   {
      if(gi.GetBoneInfo(owner->edict->s.modelindex, "gun", &groupindex, &tri_num, orient.vec3()))
      {
         gi.dprintf("attached Stinger Pack to gun bone\n");

         attached = true;
         attach(owner->entnum, groupindex, tri_num, orient);
         showModel();
         setOrigin(vec_zero);
         edict->s.gunmodelindex	= modelIndex(worldmodel.c_str());
         if(edict->s.gunmodelindex)
         {
            edict->s.gunanim        = gi.Anim_Random(edict->s.gunmodelindex, "idle");
            if(edict->s.gunanim < 0)
               edict->s.gunanim = 0;
            edict->s.gunframe = 0;
         }
         else
         {
            edict->s.gunanim = 0;
            edict->s.gunframe = 0;
         }
         edict->s.effects |= EF_SMOOTHANGLES;
      }
      else
      {
         gi.dprintf("attach failed\n");
      }
   }
}

// >) >) >) >) >) >) >) >) >) >) >) >) >)

class EXPORT_FROM_DLL EvilStingerPack : public StingerPack
{
public:
   CLASS_PROTOTYPE(EvilStingerPack);

   EvilStingerPack();
   virtual void Shoot(Event *ev);
};

CLASS_DECLARATION(StingerPack, EvilStingerPack, "weapon_evilstingerpack");

ResponseDef EvilStingerPack::Responses[] =
{
   { &EV_Weapon_Shoot, (Response)&EvilStingerPack::Shoot },
   { nullptr, nullptr }
};

EvilStingerPack::EvilStingerPack() : StingerPack()
{
   SetMinRange(0);
}

void EvilStingerPack::Shoot(Event *ev)
{
   StingerPack::Shoot(ev);
   CancelEventsOfType(EV_Weapon_Shoot);
   StingerPack::Shoot(ev);
   CancelEventsOfType(EV_Weapon_Shoot);
   StingerPack::Shoot(ev);
   CancelEventsOfType(EV_Weapon_Shoot);
   StingerPack::Shoot(ev);
   CancelEventsOfType(EV_Weapon_Shoot);
   NextAttack(0.1);
}

// EOF

