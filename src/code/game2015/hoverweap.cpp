/*
================================================================
HOVERBIKE WEAPONS
================================================================

Copyright (C) 2020 by Night Dive Studios, Inc.
All rights reserved.

See the license.txt file for conditions and terms of use for this code.

serves as both the view model and the world model
for when a bike has a rider. Also does all the weapon
related stuff for hoverbikes.
*/

#include "hoverweap.h"
#include "player.h"
#include "explosion.h"
#include "surface.h"
#include "jitter.h"
#include "misc.h"

CLASS_DECLARATION(Weapon, HoverWeap, "weapon_hoverweap");

ResponseDef HoverWeap::Responses[] =
{
   { &EV_Weapon_Shoot, (Response)&HoverWeap::Shoot },
   { nullptr, nullptr }
};

HoverWeap::HoverWeap() : Weapon()
{
   SetModels("bike_prototype.def", "view_hoverbike.def");
   modelIndex("stinger.def");
   modelIndex("hovermine.def");
   modelIndex("beam_bike.def");
   SetAmmo(nullptr, 0, 0);

   notdroppable = true;

   edict->s.effects |= EF_WARM;
}

void HoverWeap::AttachGun()
{
   int groupindex;
   int tri_num;
   Vector orient;

   if(!owner)
      return;

   if(attached)
      DetachGun();

   // uses the built in fake bone that works like Quake2's modelindexes did.
   gi.GetBoneInfo(owner->edict->s.modelindex, "origin", &groupindex, &tri_num, orient.vec3());
   attached = true;
   attach(owner->entnum, groupindex, tri_num, orient);
   showModel();

   edict->s.gunmodelindex = modelIndex(worldmodel.c_str());
   edict->s.gunanim  = 0;
   edict->s.gunframe = 0;

   // set the bike and rider values for the hoverbike
   if(owner->isSubclassOf<Player>())
   {
      Player *player = static_cast<Player *>(owner.ptr);
      bike = player->GetHoverbike();
   }
   else
      gi.dprintf("ent number %i is not a player\n", owner->entnum);
}

void HoverWeap::DetachGun()
{
   if(attached)
   {
      attached = false;
      detach();
      hideModel();
      edict->s.gunmodelindex = 0;
      edict->s.gunanim  = 0;
      edict->s.gunframe = 0;
   }
}

void HoverWeap::Fire()
{
   int noammo;

   if(!ReadyToFire())
      return;

   // if bike isn't set yet, set it
   if(!bike)
   {
      if(owner->isSubclassOf<Player>())
      {
         Player *player = static_cast<Player *>(owner.ptr);
         bike = player->GetHoverbike();
      }

      // still not set
      if(!bike)
         return;
   }

   // use appropriate type of ammo when appropriate
   noammo = 0;
   if(owner->isClient() && !(owner->flags & FL_GODMODE) && !(DM_FLAG(DF_INFINITE_AMMO)))
   {
      switch(bike->weaponmode)
      {
      case HWMODE_ROCKETS:
         if(bike->rockets < 2)
            noammo = 1;
         else
            bike->rockets -= 2;
         break;
      case HWMODE_CHAINGUN:
         if(bike->bullets < 1)
            noammo = 1;
         else
            bike->bullets -= 1;
         break;
      case HWMODE_MINES:
         if(bike->mines < 1)
            noammo = 1;
         else
            bike->mines -= 1;
         break;
      }
   }

   // don't have enough ammo to fire current weapon
   if(noammo)
   {
      // auto weapon switch
      switch(bike->weaponmode)
      {
      case HWMODE_ROCKETS:
         if(bike->bullets > 0)
            bike->SelectWeapon(HWMODE_CHAINGUN);
         else if(bike->mines > 0)
            bike->SelectWeapon(HWMODE_MINES);
         break;

      case HWMODE_CHAINGUN:
         if(bike->rockets > 0)
            bike->SelectWeapon(HWMODE_ROCKETS);
         else if(bike->mines > 0)
            bike->SelectWeapon(HWMODE_MINES);
         break;

      case HWMODE_MINES:
         if(bike->rockets > 0)
            bike->SelectWeapon(HWMODE_ROCKETS);
         else if(bike->bullets > 0)
            bike->SelectWeapon(HWMODE_CHAINGUN);
         break;

      default:
         break;
      }

      bike->WeaponNoAmmoSound();
      NextAttack(1);
      return;
   }

   weaponstate = WEAPON_FIRING;

   CancelEventsOfType(EV_Weapon_DoneFiring);
   // this is just a precaution that we can re-trigger
   NextAttack(5);
   // activate the corrent firing animation for the view
   if(bike->weaponmode == HWMODE_ROCKETS)
   {
      RandomAnimate("rocket", EV_Weapon_DoneFiring);
   }
   else if(bike->weaponmode == HWMODE_CHAINGUN)
   {
      // fire a laser beam out from the correct side
      if(side > 0)
         RandomAnimate("rightlaser", EV_Weapon_DoneFiring);
      else
         RandomAnimate("leftlaser", EV_Weapon_DoneFiring);
   }
   else if(bike->weaponmode == HWMODE_MINES)
   {
      RandomAnimate("mine", EV_Weapon_DoneFiring);
   }
   last_attack_time = level.time;
}

void HoverWeap::Shoot(Event *ev)
{
   Vector pos, dir, forward, right, up;
   Vector tmpvec;

   if(!owner)
      return;

   if(!bike)
      return;

   tmpvec[PITCH] = owner->BikeAimPitch();

   if(tmpvec[PITCH] > 45)
      tmpvec[PITCH] = 45;
   else if(tmpvec[PITCH] < -45)
      tmpvec[PITCH] = -45;

   tmpvec[YAW] = bike->move_angles[YAW];
   tmpvec[ROLL] = 0;
   tmpvec.AngleVectors(&dir, nullptr, nullptr);

   // trans dir to bike's grav axis
   if(bike->gravaxis)
   {
      Vector tmpvec2;

      tmpvec2[gravity_axis[bike->gravaxis].x] = dir.x;
      tmpvec2[gravity_axis[bike->gravaxis].y] = dir.y*gravity_axis[bike->gravaxis].sign;
      tmpvec2[gravity_axis[bike->gravaxis].z] = dir.z*gravity_axis[bike->gravaxis].sign;
      dir = tmpvec2;
   }

   tmpvec[PITCH] = 0;
   tmpvec.AngleVectors(&forward, &right, &up);

   // trans dir to bike's grav axis
   if(bike->gravaxis)
   {
      Vector tmpvec2;

      tmpvec2[gravity_axis[bike->gravaxis].x] = forward.x;
      tmpvec2[gravity_axis[bike->gravaxis].y] = forward.y*gravity_axis[bike->gravaxis].sign;
      tmpvec2[gravity_axis[bike->gravaxis].z] = forward.z*gravity_axis[bike->gravaxis].sign;
      forward = tmpvec2;

      tmpvec2[gravity_axis[bike->gravaxis].x] = right.x;
      tmpvec2[gravity_axis[bike->gravaxis].y] = right.y*gravity_axis[bike->gravaxis].sign;
      tmpvec2[gravity_axis[bike->gravaxis].z] = right.z*gravity_axis[bike->gravaxis].sign;
      right = tmpvec2;

      tmpvec2[gravity_axis[bike->gravaxis].x] = up.x;
      tmpvec2[gravity_axis[bike->gravaxis].y] = up.y*gravity_axis[bike->gravaxis].sign;
      tmpvec2[gravity_axis[bike->gravaxis].z] = up.z*gravity_axis[bike->gravaxis].sign;
      up = tmpvec2;
   }


   // fire the currently selected weapon mode
   if(bike->weaponmode == HWMODE_ROCKETS)
   {
      HBRocket *rocket;

      pos = bike->worldorigin + forward*40 + right*18*side + up*8;

      rocket = new HBRocket();
      rocket->Setup(owner, pos, dir);

      pos = bike->worldorigin + forward*40 - right*18*side + up*8;

      rocket = new HBRocket();
      rocket->Setup(owner, pos, dir);

      NextAttack(0.8);
   }
   else if(bike->weaponmode == HWMODE_CHAINGUN)
   {
      pos = bike->worldorigin + forward*40 + right*10*side + up*8;
      FireBullets(pos, dir, 1, Vector(100, 100, 100), 6, 8, DAMAGE_BULLET, MOD_HB_GUN, true);

      // also do a bullet trace in the center to make it easier to hit things
      pos = bike->worldorigin + forward*40 + up*8;
      FireBullets(pos, dir, 4, Vector(50, 50, 50), 6, 8, DAMAGE_BULLET, MOD_HB_GUN, false);

      // Set the pitch so the client can use it to fire tracers in the right direction
      angles = dir.toAngles();
      setAngles(angles);

      NextAttack(0.1);
   }
   else if(bike->weaponmode == HWMODE_MINES)
   {
      pos = bike->worldorigin - forward*30 + up*16;
      dir = forward*(-1);

      auto mine = new HBMine();
      mine->Setup(owner, pos, dir);

      NextAttack(1.2);
   }

   // alternate firing side
   side *= -1;
}

// never drop a hoverbike weapon
qboolean HoverWeap::Drop()
{
   return false;
}

//==============================================================
// rocket weapon stuff

#define ROCKET_SPEED  1800
#define ROCKET_RADIUS 150

CLASS_DECLARATION(Projectile, HBRocket, nullptr);

Event EV_HBRocket_Explode("explode");

ResponseDef HBRocket::Responses[] =
{
   { &EV_Touch,            (Response)&HBRocket::Explode },
   { &EV_HBRocket_Explode, (Response)&HBRocket::Explode },
   { nullptr, nullptr }
};

EXPORT_FROM_DLL void HBRocket::Explode(Event *ev)
{
   Entity *other = ev->GetEntity(1);
   assert(other);

   if(!other || other->isSubclassOf<Teleporter>())
      return;

   if(other->entnum == this->owner ||
      other->entnum == bike        ||
      other->entnum == frontbox    ||
      other->entnum == backbox)
   {
      return;
   }

   stopsound(CHAN_VOICE);

   setSolidType(SOLID_NOT);
   hideModel();
   if(HitSky())
   {
      PostEvent(EV_Remove, 0);
      return;
   }

   Entity *owner = G_GetEntity(this->owner);

   int damg = 60 + (int)G_Random(20);

   if(other->takedamage)
      other->Damage(this, owner, damg, origin, velocity, level.impact_trace.plane.normal, 32, 0, MOD_HB_ROCKET, -1, -1, 1.0f);

   SpawnBlastDamage(&level.impact_trace, damg, owner);

   Vector v(velocity);
   v.normalize();
   v = origin - v * 36;

   // don't do radius damage to the other, because all the damage
   // was done in the impact
   CreateExplosion(v, damg, 0.75, true, this, owner, this, MOD_HB_ROCKETSPLASH, 0.35);

   auto jitter = new RadiusJitter();
   jitter->SetupLarge(v);

   PostEvent(EV_Remove, 0.1);
}

EXPORT_FROM_DLL void HBRocket::Setup(Entity *owner, Vector pos, Vector dir)
{
   Event *ev;
   Player *rider;
   Hoverbike *bike;

   this->owner = owner->entnum;
   edict->owner = owner->edict;
   rider = static_cast<Player *>(owner);
   bike = rider->GetHoverbike();
   this->bike = bike->entnum;
   frontbox = bike->frontbox->entnum;
   backbox = bike->backbox->entnum;

   setMoveType(MOVETYPE_FLYMISSILE);
   setSolidType(SOLID_BBOX);
   edict->clipmask = MASK_PROJECTILE;

   angles = dir.toAngles();
   angles[PITCH] = -angles[PITCH];
   setAngles(angles);

   speed    = ROCKET_SPEED;
   velocity = dir * ROCKET_SPEED;

   // set missile duration
   ev = new Event(EV_Remove);
   ev->AddEntity(world);
   PostEvent(ev, 20);

   setModel("stinger.def");
   edict->s.effects  |= EF_ANIMEROCKET;
   edict->s.effects  |= EF_EVERYFRAME;
   edict->s.angles[ROLL] = rand() % 360;
   avelocity.setXYZ(0, 0, 90);
   gravity = 0;

   // setup ambient thrust
   ev = new Event(EV_RandomEntitySound);
   ev->AddString("thrust");
   ProcessEvent(ev);

   setSize({ -1, -1, -1 },  { 1, 1, 1 });
   setOrigin(pos);
   worldorigin.copyTo(edict->s.old_origin);
}

//==============================================================
// bullet weapon stuff

#define MAX_RICOCHETS 4

void HoverWeap::TraceAttack(const Vector &start, const Vector &end, int damage, trace_t *trace, int numricochets, 
                            int kick, int dflags, int meansofdeath, qboolean server_effects)
{
   Vector		org;
   Vector		dir;
   Vector		endpos;
   int			surfflags;
   int			surftype;
   int			timeofs;
   Entity		*ent;
   qboolean    ricochet;

   if(HitSky(trace))
      return;

   ricochet = false;
   dir = end - start;
   dir.normalize();

   org = end - dir;

   ent = trace->ent->entity;

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

      if(surfflags & SURF_RICOCHET)
         ricochet = true;
   }
   if(trace->intersect.valid && ent)
   {
      //
      // see if the parent group has ricochet turned on
      //
      if(trace->intersect.parentgroup >= 0)
      {
         int flags;

         flags = gi.Group_Flags(ent->edict->s.modelindex, trace->intersect.parentgroup);
         if(flags & MDL_GROUP_RICOCHET)
         {
            surftype = (flags >> 8) & 0xf;
            ricochet = true;
         }
      }
   }

   if(ent)
   {
      if(!(ent->flags & FL_SHIELDS))
      {
         if(ent->flags & FL_SPARKS)
         {
            // Take care of ricochet effects on the server
            if(server_effects && !ricochet)
            {
               timeofs = MAX_RICOCHETS - numricochets;
               if(timeofs > 0xf)
               {
                  timeofs = 0xf;
               }

               gi.WriteByte(svc_temp_entity);
               gi.WriteByte(TE_SCALED_EXPLOSION);
               gi.WritePosition(org.vec3());
               gi.WriteByte(16);
               gi.multicast(org.vec3(), MULTICAST_PVS);
            }
            MadeBreakingSound(org, owner);
         }

         if(ent->takedamage)
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
                           meansofdeath,
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
                           meansofdeath,
                           -1,
                           -1,
                           1);
            }
         }
      }
      else
      {
         surftype = SURF_TYPE_METAL;
         ricochet = true;
      }
   }

   if(ricochet && (server_effects || (numricochets < MAX_RICOCHETS)))
   {
      timeofs = MAX_RICOCHETS - numricochets;
      if(timeofs > 0xf)
      {
         timeofs = 0xf;
      }
      gi.WriteByte(svc_temp_entity);
      gi.WriteByte(TE_GUNSHOT);
      gi.WritePosition(org.vec3());
      gi.WriteDir(trace->plane.normal);
      gi.WriteByte(timeofs);
      gi.multicast(org.vec3(), MULTICAST_PVS);
   }

   if(ricochet && numricochets && damage)
   {
      dir += Vector(trace->plane.normal) * 2;
      endpos = org + dir * 8192;

      //
      // since this is a ricochet, we don't ignore the weapon owner this time.
      //
      if(DM_FLAG(DF_BBOX_BULLETS))
         *trace = G_Trace(org, vec_zero, vec_zero, endpos, nullptr, MASK_SHOT, "BulletWeapon::TraceAttack");
      else
         *trace = G_FullTrace(org, vec_zero, vec_zero, endpos, 5, nullptr, MASK_SHOT, "BulletWeapon::TraceAttack");

      if(trace->fraction != 1.0)
      {
         endpos = trace->endpos;
         TraceAttack(org, endpos, damage * 0.8f, trace, numricochets - 1, kick, dflags, meansofdeath, true);
      }
   }
}

void HoverWeap::FireBullets(Vector src, Vector dir, int numbullets, Vector spread, 
                            int mindamage, int maxdamage, int dflags, int meansofdeath, qboolean server_effects)
{
   Vector	end;
   trace_t	trace;
   Vector	right;
   Vector	up;
   int		i;

   assert(owner);
   if(!owner)
      return;

   owner->angles.AngleVectors(nullptr, &right, &up);

   angles = dir.toAngles();
   setAngles(angles);

   for(i = 0; i < numbullets; i++)
   {
      end = src +
         dir       * 8192 +
         right     * G_CRandom() * spread.x +
         up        * G_CRandom() * spread.y;

      // first need to do a regular trace to check for hitting a hoverbike
      trace = G_Trace(src, vec_zero, vec_zero, end, bike->frontbox, MASK_SHOT, "BulletWeapon::FireBullets");
      if(trace.fraction != 1)
      {
         Entity *hit;

         hit = trace.ent->entity;

         if(hit->isSubclassOf<Hoverbike>() || hit->isSubclassOf<HoverbikeBox>())
         {
            trace_t trace2;

            // also do a short full trace to see if we're hitting a player on a hoverbike
            end = trace.endpos + dir * 64;
            trace2 = G_FullTrace(Vector(trace.endpos), vec_zero, vec_zero, end, 5, bike->frontbox, MASK_SHOT, "BulletWeapon::FireBullets");
            if(trace2.fraction != 1)
            {
               Entity *hit2;

               hit2 = trace2.ent->entity;
               if(hit2->takedamage && hit2->isClient())
               {
                  // probably traced to the rider, so hit him instead
                  hit2->Damage(this, owner, mindamage + (int)G_Random(maxdamage - mindamage + 1),
                               trace.endpos, dir, trace.plane.normal, kick, dflags, meansofdeath,
                               trace.intersect.parentgroup, -1, trace.intersect.damage_multiplier);
                  return;
               }
            }

            hit->Damage(this, owner, mindamage + (int)G_Random(maxdamage - mindamage + 1), trace.endpos, dir, 
                        trace.plane.normal, kick, dflags, meansofdeath, -1, -1, 1);

            return; // hit something already, so don't do a regular full trace
         }
      }

      if(!damagedtarget && DM_FLAG(DF_BBOX_BULLETS))
      {
         trace = G_Trace(src, vec_zero, vec_zero, end, owner, MASK_SHOT, "BulletWeapon::FireBullets");

         if(trace.fraction != 1.0)
         {
            // do less than regular damage on a bbox hit
            TraceAttack(src, trace.endpos, (mindamage + (int)G_Random(maxdamage - mindamage + 1))*0.85, &trace, 
                        MAX_RICOCHETS, kick, dflags, meansofdeath, server_effects);
         }
      }
      else
      {
         trace = G_FullTrace(src, vec_zero, vec_zero, end, 5, bike->frontbox, MASK_SHOT, "BulletWeapon::FireBullets");
#if 0
         Com_Printf("Server OWNER  Angles:%0.2f %0.2f %0.2f\n", owner->angles[0], owner->angles[1], owner->angles[2]);
         Com_Printf("Server Bullet Angles:%0.2f %0.2f %0.2f\n", angles[0], angles[1], angles[2]);
         Com_Printf("Right               :%0.2f %0.2f %0.2f\n", right[0], right[1], right[2]);
         Com_Printf("Up                  :%0.2f %0.2f %0.2f\n", up[0], up[1], up[2]);
         Com_Printf("Direction           :%0.2f %0.2f %0.2f\n", dir[0], dir[1], dir[2]);
         Com_Printf("Endpoint            :%0.2f %0.2f %0.2f\n", end[0], end[1], end[2]);
         Com_Printf("Server Trace Start  :%0.2f %0.2f %0.2f\n", src[0], src[1], src[2]);
         Com_Printf("Server Trace End    :%0.2f %0.2f %0.2f\n", trace.endpos[0], trace.endpos[1], trace.endpos[2]);
         Com_Printf("\n");
#endif
         damagedtarget = false;
         if(trace.fraction != 1.0)
         {
            TraceAttack(src, trace.endpos, mindamage + (int)G_Random(maxdamage - mindamage + 1), &trace, 
                        MAX_RICOCHETS, kick, dflags, meansofdeath, server_effects);
         }
      }
   }
}

//==============================================================
// mine weapon stuff

#define MINE_SPEED   300
#define MINE_RADIUS  160
#define MINE_TIME    10
#define MINE_PUSH    750
#define MINE_DAMAGE  120

CLASS_DECLARATION(Projectile, HBMine, nullptr);

Event EV_HBMine_Explode("explode");
Event EV_HBMine_Detect("proximity_detect");

ResponseDef HBMine::Responses[] =
{
   { &EV_HBMine_Explode, (Response)&HBMine::Explode },
   { &EV_HBMine_Detect,  (Response)&HBMine::Detect  },
   { &EV_Killed,         (Response)&HBMine::Explode },
   { nullptr, nullptr }
};

EXPORT_FROM_DLL void HBMine::Detect(Event *ev)
{
   // check if we should blow up now
   if((detonate_time + MINE_TIME) < level.time)
   {
      takedamage = DAMAGE_NO;
      ProcessEvent(EV_HBMine_Explode);
      return;
   }

   Entity *ent = findradius(nullptr, origin.vec3(), MINE_RADIUS);
   while(ent)
   {
      if(ent->isSubclassOf<Sentient>())
      {
         if(!(detonate_time > level.time && ent->entnum == owner))
         {
            takedamage = DAMAGE_NO;
            PostEvent(EV_HBMine_Explode, 0.1);
            return;
         }
      }
      ent = findradius(ent, origin.vec3(), MINE_RADIUS);
   }

   PostEvent(EV_HBMine_Detect, 0.1);
}

EXPORT_FROM_DLL void HBMine::Explode(Event *ev)
{
   int			damg;
   Entity		*ent;
   float		tmpflt;
   Vector		tmpvec;

   CancelPendingEvents();

   stopsound(CHAN_VOICE);

   takedamage = DAMAGE_NO;
   setSolidType(SOLID_NOT);
   hideModel();

   damg = MINE_DAMAGE;

   if(!deathmatch->value)
      damg *= 1.5;

   // don't do radius damage to the other, because all the damage
   // was done in the impact
   CreateExplosion(origin, damg, 1.0f, true, this, G_GetEntity(owner), world, MOD_HB_MINE, 0.1);

   // push sentient's away from the blast
   ent = findradius(nullptr, origin.vec3(), MINE_RADIUS);
   while(ent)
   {
      if(ent->isSubclassOf<Sentient>())
      {
         tmpvec = ent->origin - origin;
         tmpflt = MINE_PUSH - tmpvec.length()*3;
         if((tmpvec[gravity_axis[gravaxis].z]*gravity_axis[gravaxis].sign) < 0)
         {
            tmpvec[gravity_axis[gravaxis].z] = 0;
         }
         // was just tmpvec.normalize(), but the compiler is a wanker
         tmpvec.normalize();
         tmpvec *= tmpflt;
         tmpvec[gravity_axis[gravaxis].z] += 150*gravity_axis[gravaxis].sign;
         // check if it's a player on a bike
         if(ent->isSubclassOf<Player>())
         {
            Player *rider;
            Hoverbike *bike;

            rider = static_cast<Player *>(ent);
            bike = rider->GetHoverbike();

            if(bike)
            {
               // bike's get a bit of slow down from mines
               // but only half the push
               bike->velocity *= 0.9;
               bike->velocity += tmpvec*0.75;
            }
            else
            {
               rider->velocity += tmpvec;
            }
         }
         else
         {
            ent->velocity += tmpvec;
         }
      }
      ent = findradius(ent, origin.vec3(), MINE_RADIUS);
   }

   PostEvent(EV_Remove, 0.1);
}

EXPORT_FROM_DLL void HBMine::Setup(Entity *owner, Vector pos, Vector dir)
{
   Event *ev;
   Vector smins, smaxs;
   Vector forward, right, up;
   Vector t[3];

   this->owner = owner->entnum;
   edict->owner = owner->edict;

   setMoveType(MOVETYPE_TOSS);
   setSolidType(SOLID_BBOX);
   edict->clipmask = MASK_PROJECTILE | MASK_WATER;

   SetGravityAxis(owner->gravaxis);

   angles = vec_zero;
   angles[YAW] = rand() % 360;
   angles.AngleVectors(&t[0], &t[1], &t[2]);
   forward[gravity_axis[gravaxis].x] = t[0][0];
   forward[gravity_axis[gravaxis].y] = t[0][1] * gravity_axis[gravaxis].sign;
   forward[gravity_axis[gravaxis].z] = t[0][2] * gravity_axis[gravaxis].sign;
   right  [gravity_axis[gravaxis].x] = t[1][0];
   right  [gravity_axis[gravaxis].y] = t[1][1] * gravity_axis[gravaxis].sign;
   right  [gravity_axis[gravaxis].z] = t[1][2] * gravity_axis[gravaxis].sign;
   up     [gravity_axis[gravaxis].x] = t[2][0];
   up     [gravity_axis[gravaxis].y] = t[2][1] * gravity_axis[gravaxis].sign;
   up     [gravity_axis[gravaxis].z] = t[2][2] * gravity_axis[gravaxis].sign;
   VectorsToEulerAngles(forward.vec3(), right.vec3(), up.vec3(), angles.vec3());
   setAngles(angles);

   velocity = owner->velocity*0.5 + dir * MINE_SPEED;
   velocity[gravity_axis[gravaxis].z] += 100*gravity_axis[gravaxis].sign;

   // set mine to proximity detect
   detonate_time = level.time + 2;
   PostEvent(EV_HBMine_Detect, 0.5);

   takedamage = DAMAGE_YES;
   health = 25;

   setModel("hovermine.def");
   edict->s.effects  |= EF_ROCKET;
   edict->s.effects  |= EF_EVERYFRAME;
   gravity = 0.75;

   // setup ambient thrust
   ev = new Event(EV_RandomEntitySound);
   ev->AddString("hover");
   ProcessEvent(ev);

   smins = { -10, -10, -10 };
   smaxs = {  10,  10,  10 };
   smaxs[gravity_axis[gravaxis].z] = 18;
   smins[gravity_axis[gravaxis].z] = -18;
   setSize(smins, smaxs);
   setOrigin(pos);
   worldorigin.copyTo(edict->s.old_origin);

   RandomAnimate("idle", nullptr);
}

// EOF
