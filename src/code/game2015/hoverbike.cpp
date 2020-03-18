/*
================================================================
HOVERBIKE
================================================================

Copyright (C) 2020 by Night Dive Studios, Inc.
All rights reserved.

See the license.txt file for conditions and terms of use for this code.

AI NOTE:
Although I didn't have time to impliment it, the hoverbike should be
(for the most part) structured properly to allow for AI control of one.
I was planning on having the AI communicate to the hoverbike through
fake usercmd_t info that the AI would put together and send to bike to
tell it were to go. All bits of code refering to bots assumes that
they will be clients. This note's here in hopes that you're an
industrious soul and want to make a Sin bot that can use hoverbikes. ;)
*/

#include "hoverbike.h"
#include "player.h"
#include "ctf.h"

//==============================================================

// hovering stuff
#define HOVER_HEIGHT        48
#define HOVER_FORCE         60
#define HOVER_GROUNDGRAVITY 275
// forward movement stuff
#define HOVER_MAX_SPEED     800
#define HOVER_ACCEL_CEILING 1200
#define HOVER_ACCEL         300
//braking stuff
#define HOVER_MIN_SPEED -100
#define HOVER_BRAKES     600
// side movement stuff
#define HOVER_STRAFE      700
#define HOVER_MAX_SIDE    250
#define HOVER_STRAFE_ROLL 20
#define HOVER_ROLL        50
// vertical booster
#define HOVER_BOOSTER         300
#define HOVER_BOOSTER_USERATE 5
// non-movement drift %
#define HOVER_DRIFT      -0.65
#define HOVER_SIDE_DRIFT -0.575
#define HOVER_MIN_DRIFT   2
#define HOVER_MAX_DRIFT   12
// for pitch adjustments made mid-air
#define HOVER_FLY_PITCH   -30
#define HOVER_PITCH_SPEED  35
// for turbo booster stuff
#define HOVER_TURBO_REFILL   0.3
#define HOVER_TURBO_USERATE  3
#define HOVER_TURBO_BOOST    400
#define HOVER_TURBO_MAXSPEED 1200
// hoverbike health
#define HOVER_MAX_HEALTH   250
// timming delays for some stuff
#define HOVER_EFFECTS_TIME 0.2

// CTF
#define HOVER_SF_HC 1 // can only be used by HardCorps team
#define HOVER_SF_ST 2 // can only be used by Sin Tech team

//==============================================================
// various physics functions
void G_Impact(Entity *e1, trace_t *trace);

//==============================================================
// check for world effects on hoverbike (like lava)

static void Hoverbike_WorldEffects(Hoverbike *bike)
{
   bike->watertype = gi.pointcontents(bike->worldorigin.vec3());

   if(bike->watertype & CONTENTS_LAVA)
   {
      bike->Damage(world, world, level.lavadamage, bike->worldorigin, vec_zero, vec_zero, 0, DAMAGE_NO_ARMOR, MOD_LAVA, -1, -1, 1.0f);
   }
}

//==============================================================
/* Hoverbike movement physics
First goes through each of the bike's seperate bounding boxes
to see how far the whole bike can actually move. After that, it
then actually moves the bike and the rider.
*/

#define MAX_CLIP_PLANES 5
#define STOP_EPSILON 0.1
#define sv_friction 6
#define sv_stopspeed 100

int G_Physics_Hoverbike(Hoverbike *bike)
{
   Entity	*hit, *passent;
   trace_t	trace, trace2;
   int		mask;
   int		bumpcount;
   Vector	dir, org, forward;
   float	nearest_hit, speed;
   int		hit_pos;
   float	friction, newspeed, control;
   float bounceback;

   Vector end(0, bike->angles[YAW], 0);
   end.AngleVectors(&forward, nullptr, nullptr);

   assert(bike->frontbox);
   HoverbikeBox *front = bike->frontbox;

   assert(bike->backbox);
   HoverbikeBox *back = bike->backbox;

   if(bike->rider)
      passent = bike->rider;
   else
   {
      // refill turbo booster if needed
      bike->turbo += HOVER_TURBO_REFILL;
      if(bike->turbo > 100)
         bike->turbo = 100;

      // add gravity
      if(!bike->groundentity)
         G_AddGravity(bike);

      if(bike->groundentity)
      {
         if(bike->velocity.z > 0)
         {
            bike->groundentity = nullptr;
         }
         else
         {
            bike->velocity = vec_zero;
            return 0;
         }
      }

      passent = nullptr;
      // need to set the seperate parts as not solid
      // to prevent collision problems between them
      bike->setSolidType(SOLID_NOT);
      bike->frontbox->setSolidType(SOLID_NOT);
      bike->backbox->setSolidType(SOLID_NOT);
   }

   int numbumps = 4;

   G_CheckVelocity(bike);

   int blocked = 0;
   Vector original_velocity(bike->velocity);

   float time_left = FRAMETIME;

   if((bike->ridertype == RIDER_PLAYER) || (bike->ridertype == RIDER_BOT))
      mask = MASK_PLAYERSOLID;
   else
      mask = MASK_MONSTERSOLID;

   for(bumpcount = 0; bumpcount < numbumps; bumpcount++)
   {
      // we need to do distance checks for each box to see what's the first thing we hit
      nearest_hit = 1;
      friction = 0;
      bounceback = 1.25;

      // first check front
      org = bike->worldorigin + forward*front->offset;
      end = org + time_left*bike->velocity;
      trace2 = G_Trace(org, front->mins, front->maxs, end, passent, mask, "G_Physics_Hoverbike");
      // check for running something over
      speed = bike->velocity.length();
      if((passent) && (trace2.ent) && speed > 200)
      {
         hit = trace2.ent->entity;
         if(hit->takedamage)
         {
            int damg;

            if((hit->isSubclassOf<Sentient>() && static_cast<Sentient *>(hit)->IsOnBike()) ||
               hit->isSubclassOf<Hoverbike>() ||
               hit->isSubclassOf<HoverbikeBox>())
               damg = 0;
            else if(hit->isSubclassOf<Sentient>())
               damg = (speed - 150)*0.5;
            else
               damg = (speed - 150)*0.2;

            if(damg > 0)
            {
               if(bike->rider)
                  hit->Damage(bike, bike->rider, damg, bike->origin, bike->velocity, vec_zero, (damg*0.5), 0, MOD_HOVERBIKE, -1, -1, 1.0f);
               else
                  hit->Damage(bike, bike, damg, bike->origin, bike->velocity, vec_zero, (damg*0.5), 0, MOD_HOVERBIKE, -1, -1, 1.0f);
            }
         }
      }
      // basically ignore the trace if the extra bounding box is in something solid
      if(trace2.startsolid || trace2.allsolid)
      {
         trace2.fraction = 1;
      }
      else if(trace2.fraction < nearest_hit)
      {
         // set collision stuff
         nearest_hit = trace2.fraction;
         hit_pos = 1; // indicates that it was the front that bumped
         trace = trace2;
      }

      // now check back
      org = bike->worldorigin + forward*back->offset;
      end = org + time_left*bike->velocity;
      trace2 = G_Trace(org, back->mins, back->maxs, end, passent, mask, "G_Physics_Hoverbike");
      // basically ignore the trace if the extra bounding box is in something solid
      if(trace2.startsolid || trace2.allsolid)
      {
         trace2.fraction = 1;
      }
      else if(trace2.fraction < nearest_hit)
      {
         // set collision stuff
         nearest_hit = trace2.fraction;
         hit_pos = 2; // indicates that it was the back that bumped
         trace = trace2;
      }

      // now check main bike box
      end = bike->worldorigin + time_left*bike->velocity;
      trace2 = G_Trace(bike->worldorigin, bike->mins, bike->maxs, end, passent, mask, "G_Physics_Hoverbike");
      if(trace2.allsolid)
      {
         // bike is trapped in another solid
         if(!bike->rider)
         {
            bike->setSolidType(SOLID_BBOX);
            bike->frontbox->setSolidType(SOLID_BBOX);
            bike->backbox->setSolidType(SOLID_BBOX);
         }
         return 3;
      }
      // use this trace if main trace structure hasn't been set yet
      if(trace2.fraction < nearest_hit || nearest_hit == 1)
      {
         // set collision stuff
         nearest_hit = trace2.fraction;
         hit_pos = 0; // indicates that it was the middle that bumped
         trace = trace2;
      }
      else // do a trace for the middle according to the shortest distance moveable
      {
         nearest_hit -= 0.01;
         if(nearest_hit < 0)
            nearest_hit = 0;
         end = bike->worldorigin + bike->velocity*time_left*nearest_hit;
         trace2 = G_Trace(bike->worldorigin, bike->mins, bike->maxs, end, passent, mask, "G_Physics_Hoverbike");
         trace.endpos[0] = trace2.endpos[0];
         trace.endpos[1] = trace2.endpos[1];
         trace.endpos[2] = trace2.endpos[2];
      }

      if(trace.fraction > 0)
      {
         // actually covered some distance
         bike->setOrigin(trace.endpos);
         org = bike->worldorigin + forward*front->offset;
         front->setOrigin(org);
         org = bike->worldorigin + forward*back->offset;
         back->setOrigin(org);
      }

      if(trace.fraction == 1) // moved the entire distance
         break;

      hit = trace.ent->entity;

      if((trace.plane.normal[gravity_axis[bike->gravaxis].z]*gravity_axis[bike->gravaxis].sign) > 0.7)
      {
         // floor
         blocked |= 1;

         // apply friction to non-hovering bikes
         if(!bike->rider)
         {
            if(hit->getSolidType() == SOLID_BSP)
            {
               bike->groundentity = hit->edict;
               bike->groundentity_linkcount = hit->edict->linkcount;
               bike->groundplane = trace.plane;
               bike->groundsurface = trace.surface;
               bike->groundcontents = trace.contents;
            }

            friction = sv_friction;
         }
         else
         {
            bounceback = 1;
         }
      }
      else if((trace.plane.normal[gravity_axis[bike->gravaxis].z]*gravity_axis[bike->gravaxis].sign) < -0.7)
      {
         if(bike->velocity[gravity_axis[bike->gravaxis].z] > 400)
         {
            friction = 2;
         }
         else if(bike->velocity[gravity_axis[bike->gravaxis].z] > 50)
         {
            friction = 1;
         }

         bounceback = 1.1;
      }
      else // slow down a bit from hitting a wall
      {
         speed = sqrtf(bike->velocity[gravity_axis[bike->gravaxis].x]*bike->velocity[gravity_axis[bike->gravaxis].x]
                       + bike->velocity[gravity_axis[bike->gravaxis].y]*bike->velocity[gravity_axis[bike->gravaxis].y]);
         if(speed > HOVER_MAX_SPEED*0.6)
            friction = 1;
         else if(speed > 75)
            friction = 0.5;
      }

      // apply friction if needed
      if(friction)
      {
         speed = sqrtf(bike->velocity[gravity_axis[bike->gravaxis].x]*bike->velocity[gravity_axis[bike->gravaxis].x]
                       + bike->velocity[gravity_axis[bike->gravaxis].y]*bike->velocity[gravity_axis[bike->gravaxis].y]);

         control = speed < sv_stopspeed ? sv_stopspeed : speed;
         newspeed = speed - FRAMETIME*control*friction;

         if(newspeed < 0)
            newspeed = 0;

         if(speed <= 0)
            newspeed = 1;
         else
            newspeed /= speed;

         // apply the friction in the way needed
         if(trace.plane.normal[gravity_axis[bike->gravaxis].z] > 0.7)
         {
            bike->velocity.x *= newspeed;
            bike->velocity.y *= newspeed;
            if(bike->velocity.z < 0)
               bike->velocity.z = 0;
         }
         else if(trace.plane.normal[gravity_axis[bike->gravaxis].z] < -0.7)
         {
            bike->velocity.x *= newspeed;
            bike->velocity.y *= newspeed;
            if(bike->velocity.z > 0)
               bike->velocity.z = 0;
         }
         else
            bike->velocity *= newspeed;
      }

      Vector FooVec(trace.plane.normal);
      G_ClipVelocity(bike->velocity, FooVec, bike->velocity,
                     bounceback, bike->gravaxis);

      // run the impact function
      // run impact function for rider if touched an item
      if(hit_pos == 1)
         G_Impact(front, &trace);
      else if(hit_pos == 2)
         G_Impact(back, &trace);
      else
         G_Impact(bike, &trace);
      if(!bike->edict->inuse)
         break; // removed by the impact function

      time_left -= time_left * trace.fraction;
   }

   // make appropriate collision sound if needed
   bike->CollisionSound(original_velocity, bike->velocity);

   // check for damage from world effects at final location
   org = bike->worldorigin + Vector(0, 0, bike->mins.z);
   if(gi.pointcontents(org.vec3()) & CONTENTS_LAVA)
   {
      // check a bit higher up too
      org = bike->worldorigin;
      if(gi.pointcontents(org.vec3()) & CONTENTS_LAVA)
      {
         bike->Damage(world, world, level.lavadamage * 3, bike->worldorigin, vec_zero, vec_zero, 0, DAMAGE_NO_ARMOR, MOD_LAVA, -1, -1, 1.0f);
      }
      else
      {
         bike->Damage(world, world, level.lavadamage * 2, bike->worldorigin, vec_zero, vec_zero, 0, DAMAGE_NO_ARMOR, MOD_LAVA, -1, -1, 1.0f);
      }
   }

   if(!bike->edict->inuse)
      return blocked; // destroyed by lava

   if(!bike->rider)
   {
      bike->setSolidType(SOLID_BBOX);
      bike->frontbox->setSolidType(SOLID_BBOX);
      bike->backbox->setSolidType(SOLID_BBOX);
   }

   return blocked;
}

//==============================================================

// this variation of G_TouchTriggers allows riders to touch items with their hoverbike
static void Hoverbike_TouchTriggers(Entity *ent, Entity *rider)
{
   int      num;
   edict_t *touch[MAX_EDICTS];
   edict_t *hit;
   Vector   end;

   // dead things don't activate triggers!
   if(rider->health <= 0)
      return;

   Vector absmin(ent->absmin);
   Vector absmax(ent->absmax);
   // extend touch area down to the ground for picking up items
   if(ent->isSubclassOf<Hoverbike>())
   {
      if(ent->groundentity)
         absmin[gravity_axis[rider->gravaxis].z] -= 32*gravity_axis[rider->gravaxis].sign;
   }
   else
   {
      if(((HoverbikeBox *)ent)->bike->groundentity)
         absmin[gravity_axis[rider->gravaxis].z] -= 32*gravity_axis[rider->gravaxis].sign;
   }

   num = gi.BoxEdicts(absmin.vec3(), absmax.vec3(), touch, MAX_EDICTS, AREA_TRIGGERS);

   // be careful, it is possible to have an entity in this
   // list removed before we get to it (killtriggered)
   for(int i = 0; i < num; i++)
   {
      hit = touch[i];
      if(!hit->inuse || (hit->entity == ent))
         continue;

      assert(hit->entity);

      // FIXME
      // post the events on the list with zero time
      auto ev = new Event(EV_Touch);
      ev->AddEntity(rider);
      hit->entity->ProcessEvent(ev);
   }
}

//==============================================================
// HoverbikeBox entity
// serves as the extra bounding boxes for a hoverbike
//==============================================================

CLASS_DECLARATION(Entity, HoverbikeBox, nullptr);

ResponseDef HoverbikeBox::Responses[] =
{
   { &EV_Use,    (Response)&HoverbikeBox::BikeUse    },
   { &EV_Damage, (Response)&HoverbikeBox::BikeDamage },
   { nullptr, nullptr }
};

HoverbikeBox::HoverbikeBox() : Entity()
{
   setSolidType(SOLID_BBOX);
   setMoveType(MOVETYPE_NONE);
   hideModel();

   takedamage = DAMAGE_YES;
   health     = 32000;

   // don't need to send hoverbike bounding boxes to clients
   edict->svflags |= SVF_NOCLIENT;
}

// a hoverbike's bounding box was used, so pass
// the use event on to the main bike entity
void HoverbikeBox::BikeUse(Event *ev)
{
   // just to be safe
   if(!bike)
   {
      PostEvent(EV_Remove, 0);
      return;
   }

   if(bike->isSubclassOf<Hoverbike>()) // haleyjd: type safety check
       static_cast<Hoverbike *>(bike.ptr)->BikeUse(ev);
}

void HoverbikeBox::BikeDamage(Event *ev)
{
   // make sure we have a bike
   if(!bike)
      return;

   // don't bother if bike is dead
   if(bike->deadflag)
      return;

   // pass damage along to the main bike entity
   auto event = new Event(ev);
   bike->ProcessEvent(event);
}

//==============================================================
// Hoverbike
// main entity for a hoverbike
//==============================================================

CLASS_DECLARATION(Entity, Hoverbike, "vehicles_hoverbike");

Event EV_Hoverbike_Respawn("respawn");

ResponseDef Hoverbike::Responses[] =
{
   { &EV_Use,               (Response)&Hoverbike::BikeUse     },
   { &EV_Pain,              (Response)&Hoverbike::BikePain    },
   { &EV_Killed,            (Response)&Hoverbike::BikeKilled  },
   { &EV_Hoverbike_Respawn, (Response)&Hoverbike::BikeRespawn },
   { nullptr, nullptr }
};

Hoverbike::Hoverbike() : Entity()
{
   Vector tmpvec;

   // hoverbikes don't work with MFD
   if(deathmatch->value == DEATHMATCH_MFD)
   {
      gi.printf("Marked for Death does not work with hoverbikes... bike being removed\n");
      PostEvent(EV_Remove, 0);
      return;
   }

   takedamage   = DAMAGE_YES;
   health       = HOVER_MAX_HEALTH;
   max_health   = HOVER_MAX_HEALTH;
   deadflag     = 0;
   mass         = 800;

   if(deathmatch->value)
   {
      rockets = 40;
      bullets = 60;
      mines = 5;
   }
   else
   {
      rockets = 100;
      bullets = 250;
      mines = 10;
   }

   flags      |= FL_POSTTHINK;
   // just marks this as a hoverbike to the game since
   // it's not attached to a parent
   edict->s.effects |= EF_HOVER; //***

   setOrigin(origin);
   setSolidType(SOLID_BBOX);
   setMoveType(MOVETYPE_HOVERBIKE);

   angles = Vector(0, G_GetFloatArg("angle", 0), 0);
   setAngles(angles);

   setModel("bike_prototype.def");
   showModel();

   // check for team specific bikes in CTF
   if(ctf->value)
   {
      if(spawnflags & HOVER_SF_HC)
         edict->s.skinnum = 1;
      else if(spawnflags & HOVER_SF_ST)
         edict->s.skinnum = 2;
   }

   // precache some other models
   modelIndex("view_hoverbike.def");
   modelIndex("hb_ammonum.def");
   modelIndex("hb_health.def");
   modelIndex("hb_speedbar.def");
   modelIndex("hb_speednum.def");
   modelIndex("hb_turbo.def");
   modelIndex("hb_health.def");
   modelIndex("hb_ammonum.def");
   modelIndex("hb_weap.def");
   modelIndex("stinger.def");
   modelIndex("hovermine.def");
   modelIndex("beam_bike.def");

   // make the front and back bounding boxes now
   frontbox = new HoverbikeBox();
   backbox  = new HoverbikeBox();

   // set gravityaxis and bounding box sizes
   gravaxis = G_GetIntArg("gravityaxis", 0);
   if(gravaxis < 0)
      gravaxis = 0;
   else if(gravaxis > 5)
      gravaxis = 5;
   SetGravityAxis(gravaxis);

   // make front bounding box
   frontbox->offset = 40;
   //tmpvec = origin + Vector(orientation[0])*box->offset;
   tmpvec[gravity_axis[gravaxis].x] = orientation[0][0]*frontbox->offset;
   tmpvec[gravity_axis[gravaxis].y] = orientation[0][1]*frontbox->offset*gravity_axis[gravaxis].sign;
   tmpvec[gravity_axis[gravaxis].z] = orientation[0][2]*frontbox->offset*gravity_axis[gravaxis].sign;
   tmpvec += origin;
   frontbox->setOrigin(tmpvec);
   frontbox->bike = this;

   // make back bounding box
   backbox->offset = -40;
   //	tmpvec = origin + Vector(orientation[0])*box->offset;
   tmpvec[gravity_axis[gravaxis].x] = orientation[0][0]*backbox->offset;
   tmpvec[gravity_axis[gravaxis].y] = orientation[0][1]*backbox->offset*gravity_axis[gravaxis].sign;
   tmpvec[gravity_axis[gravaxis].z] = orientation[0][2]*backbox->offset*gravity_axis[gravaxis].sign;
   tmpvec += origin;
   backbox->setOrigin(tmpvec);
   backbox->bike = this;

   move_angles = angles;
   ApplyMoveAngles();
   // setup values for respawning
   spawnspot   = origin;
   spawnangles = angles;
}

void Hoverbike::RespawnSetup(Vector spot, Vector ang, float respawndelay, int grav, int flags)
{
   setSolidType(SOLID_NOT);
   setMoveType(MOVETYPE_NONE);
   hideModel();
   edict->s.effects &= ~EF_HOVER;

   spawnspot   = spot;
   spawnangles = ang;
   gravaxis    = grav;
   spawnflags  = flags;

   PostEvent(EV_Hoverbike_Respawn, respawndelay);
}

void Hoverbike::BikeRespawn(Event *ev)
{
   trace_t trace;
   Vector pos, forward;
   Vector tmins, tmaxs;

   Vector tmpvec;

   spawnangles.AngleVectors(&forward, nullptr, nullptr);

   frontbox = new HoverbikeBox();
   backbox  = new HoverbikeBox();

   // check that area's clear for respawn
   tmins = mins;
   tmaxs = maxs;
   pos   = spawnspot;
   trace = G_Trace(pos, tmins, tmaxs, pos, nullptr, MASK_PLAYERSOLID, "Hoverbike::BikeRespawn");
   if(trace.allsolid) // can't respawn yet
   {
      // try again later
      PostEvent(EV_Hoverbike_Respawn, 1);
      frontbox->ProcessEvent(EV_Remove);
      backbox->ProcessEvent(EV_Remove);
      return;
   }

   tmins = frontbox->mins;
   tmaxs = frontbox->maxs;
   pos   = spawnspot + forward*40;
   trace = G_Trace(pos, tmins, tmaxs, pos, nullptr, MASK_PLAYERSOLID, "Hoverbike::BikeRespawn");
   if(trace.allsolid) // can't respawn yet
   {
      // try again later
      PostEvent(EV_Hoverbike_Respawn, 1);
      frontbox->ProcessEvent(EV_Remove);
      backbox->ProcessEvent(EV_Remove);
      return;
   }

   tmins = backbox->mins;
   tmaxs = backbox->maxs;
   pos   = spawnspot - forward*40;
   trace = G_Trace(pos, tmins, tmaxs, pos, nullptr, MASK_PLAYERSOLID, "Hoverbike::BikeRespawn");
   if(trace.allsolid) // can't respawn yet
   {
      // try again later
      PostEvent(EV_Hoverbike_Respawn, 1);
      frontbox->ProcessEvent(EV_Remove);
      backbox->ProcessEvent(EV_Remove);
      return;
   }

   // call inti function to set everything up
   takedamage = DAMAGE_YES;
   health     = HOVER_MAX_HEALTH;
   max_health = HOVER_MAX_HEALTH;
   deadflag   = 0;

   rider        = nullptr;
   ridertype    = RIDER_OTHER;
   respawntimer = 0;
   strafingroll = 0;
   bobsin       = 0;
   bobfrac      = 0;
   weaponmode   = 0;
   rockets      = 40;
   bullets      = 60;
   mines        = 5;
   sndflags     = HBSND_NONE;
   turbo        = 100;
   forwardmove  = 0;
   sidemove     = 0;
   upmove       = 0;
   flags       |= FL_POSTTHINK;
   edict->s.effects |= EF_HOVER;

   setOrigin(spawnspot);
   setSolidType(SOLID_BBOX);
   setMoveType(MOVETYPE_HOVERBIKE);
   velocity = vec_zero;

   angles = spawnangles;
   setAngles(angles);

   edict->svflags &= ~SVF_NOCLIENT;
   setModel("bike_prototype.def");
   showModel();

   // check for team specific bikes in CTF
   if(ctf->value)
   {
      if(spawnflags & HOVER_SF_HC)
         edict->s.skinnum = 1;
      else if(spawnflags & HOVER_SF_ST)
         edict->s.skinnum = 2;
   }

   SetGravityAxis(gravaxis);

   // make front bounding box
   frontbox->offset = 40;
   //	tmpvec = origin + Vector(orientation[0])*box->offset;
   tmpvec[gravity_axis[gravaxis].x] = orientation[0][0]*frontbox->offset;
   tmpvec[gravity_axis[gravaxis].y] = orientation[0][1]*frontbox->offset*gravity_axis[gravaxis].sign;
   tmpvec[gravity_axis[gravaxis].z] = orientation[0][2]*frontbox->offset*gravity_axis[gravaxis].sign;
   tmpvec += origin;
   frontbox->setOrigin(tmpvec);
   frontbox->bike = this;

   // make back bounding box
   backbox->offset = -40;
   //	tmpvec = origin + Vector(orientation[0])*box->offset;
   tmpvec[gravity_axis[gravaxis].x] = orientation[0][0]*backbox->offset;
   tmpvec[gravity_axis[gravaxis].y] = orientation[0][1]*backbox->offset*gravity_axis[gravaxis].sign;
   tmpvec[gravity_axis[gravaxis].z] = orientation[0][2]*backbox->offset*gravity_axis[gravaxis].sign;
   tmpvec += origin;
   backbox->setOrigin(tmpvec);
   backbox->bike = this;

   // make visual effects
   SpawnTeleportEffect(origin, 124);
   SpawnTeleportEffect(frontbox->origin, 124);
   SpawnTeleportEffect(backbox->origin, 124);
}

//==============================================================

void Hoverbike::SetRiderAngles(Vector angles)
{
   if(!rider)
      return;

   if(ridertype != RIDER_PLAYER)
      return;

   for(int i = 0; i < 2; i++)
      rider->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(angles[i] - rider->client->resp.cmd_angles[i]);
}

void Hoverbike::SetRiderYaw(float yawangle)
{
   if(!rider)
      return;

   if(ridertype != RIDER_PLAYER)
      return;

   rider->client->ps.pmove.delta_angles[YAW] = ANGLE2SHORT(yawangle - rider->client->resp.cmd_angles[YAW]);
}

void Hoverbike::BikeUse(Event *ev)
{
   Entity *other;
   Weapon *weapon;
   Sentient *sent;

   // already in use
   if(rider)
      return;

   other = ev->GetEntity(1);

   if(!other)
      return;

   // only sentients allowed
   if(!other->isSubclassOf<Sentient>())
      return;

   // get other as a sentient
   sent = static_cast<Sentient *>(other);

   // mutants can't use hoverbikes
   if(other->flags & (FL_MUTANT|FL_SP_MUTANT))
      return;

   // already using a bike
   if(sent->GetHoverbike())
      return;

   // can't get on a bike while using a guided missile
   if(sent->IsFiringMissile())
      return;

   // set if the rider is a player
   if(other->isSubclassOf<Player>())
      ridertype = RIDER_PLAYER;
   else
      ridertype = RIDER_OTHER;

   // never got around to adding AI support for hoverbikes
   if(ridertype != RIDER_PLAYER)
      return;

   // CTF
   if(ctf->value)
   { 
      // check for being a team specific bike
      if((spawnflags & HOVER_SF_HC) && other->client->resp.ctf_team != CTF_TEAM_HARDCORPS)
         return;
      if((spawnflags & HOVER_SF_ST) && other->client->resp.ctf_team != CTF_TEAM_SINTEK)
         return;
   }

   // we've got outself a new rider
   rider = other;

   // set stuff for the bike
   setMoveType(MOVETYPE_NONE);
   SetGravityAxis(gravaxis);
   hideModel();
   edict->s.effects &= ~EF_HOVER;
   oldweapon = sent->CurrentWeapon();
   // debounce the use (turbo) key
   getontimer = level.time + 0.5;
   // don't need to send hoverbike since it's just a bounding box now
   edict->svflags |= SVF_NOCLIENT;

   // setup a bike respawn if we need to
   if(deathmatch->value && !respawntimer)
   {
      auto newbike = new Hoverbike();
      newbike->RespawnSetup(spawnspot, spawnangles, 20, gravaxis, spawnflags);
      respawntimer = 1;
   }

   // now that we've been used, remove team specific flags
   spawnflags = 0;

   // set stuff for the rider
   rider->setOrigin(origin);
   rider->velocity = velocity;
   SetRiderAngles(angles);
   weapon = sent->giveWeapon("HoverWeap");
   assert(weapon);
   sent->ForceChangeWeapon(weapon);
   sent->SetGravityAxis(gravaxis);

   weapon = sent->CurrentWeapon();

   weapon->edict->s.effects |= EF_HOVER;
   effectstimmer = level.time;
   soundtimmer = level.time + HOVER_EFFECTS_TIME*0.5;

   MakeGuages();

   sent->SetHoverbike(this);
   sent->SetBikeAnim("ride");

   // do human player specific typestuff
   if(ridertype == RIDER_PLAYER)
   {
      Player *pent;
      str model;

      pent = static_cast<Player *>(other);

      if(pent->ViewMode() == THIRD_PERSON)
         pent->SetViewMode(THIRD_PERSON);

      // set bike's model & skin to the player's choice
      edict->s.renderfx |= RF_CUSTOMSKIN;
      edict->s.skinnum = pent->edict->s.number - 1;

      // Make sure that the model is allowed
      model = rider->client->pers.bikemodel;
      if(!game.ValidBikeModels.ObjectInList(model))
      {
         model = "bike_prototype.def";
      }

      setModel(model.c_str());

      // also set skin for visible hoverbike representation
      weapon->edict->s.renderfx |= RF_CUSTOMSKIN;
      weapon->edict->s.skinnum = pent->edict->s.number - 1;
      weapon->SetModels(model.c_str(), "view_hoverbike.def");
   }
}

void Hoverbike::BikeGetOff()
{
   trace_t trace;
   Vector  tmpmins, tmpmaxs, right;
   Vector  t[3];
   int mask, rideroff;
   Weapon *weapon;
   Sentient *sent;

   if(!rider)
      return;

   // if dead, then we're getting the rider off elsewere
   if(deadflag)
      return;

   // don't effect the side vector by roll or pitch
   Vector pos(0, angles[YAW], 0);
   pos.AngleVectors(&t[0], &t[1], &t[2]);
   right[gravity_axis[gravaxis].x] = t[1][0];
   right[gravity_axis[gravaxis].y] = t[1][1] * gravity_axis[gravaxis].sign;
   right[gravity_axis[gravaxis].z] = t[1][2] * gravity_axis[gravaxis].sign;
   right *= -1;

   // move the rider off of the bike
   rideroff = 0;
   if((ridertype == RIDER_PLAYER) || (ridertype == RIDER_BOT))
      mask = MASK_PLAYERSOLID;
   else
      mask = MASK_MONSTERSOLID;
   tmpmins = { -16, -16, -16 };
   tmpmaxs = { 16, 16, 16 };
   if(rider->health <= 0)
   {
      if(gravity_axis[gravaxis].sign < 0)
      {
         tmpmins[gravity_axis[gravaxis].z] = -CROUCH_HEIGHT;
         tmpmaxs[gravity_axis[gravaxis].z] = 0;
      }
      else
      {
         tmpmins[gravity_axis[gravaxis].z] = 0;
         tmpmaxs[gravity_axis[gravaxis].z] = CROUCH_HEIGHT;
      }
   }
   else
   {
      if(gravity_axis[gravaxis].sign < 0)
      {
         tmpmins[gravity_axis[gravaxis].z] = -STAND_HEIGHT;
         tmpmaxs[gravity_axis[gravaxis].z] = 0;
      }
      else
      {
         tmpmins[gravity_axis[gravaxis].z] = 0;
         tmpmaxs[gravity_axis[gravaxis].z] = STAND_HEIGHT;
      }
   }

   if(velocity.length() > 400)
   {
      pos = origin;
      pos[gravity_axis[gravaxis].z] += 40*gravity_axis[gravaxis].sign;
      trace = gi.trace(pos.vec3(), tmpmins.vec3(), tmpmaxs.vec3(), pos.vec3(), nullptr, mask);
      if(!trace.allsolid)
      {
         rider->setOrigin(trace.endpos);
         rideroff = 1;
         velocity *= 0.75;
         rider->velocity[gravity_axis[gravaxis].z] += 150*gravity_axis[gravaxis].sign;
      }
   }

   if(!rideroff)
   {
      pos = origin + right*48;
      trace = gi.trace(pos.vec3(), tmpmins.vec3(), tmpmaxs.vec3(), pos.vec3(), nullptr, mask);
      if(!trace.allsolid)
      {
         rider->setOrigin(trace.endpos);
         rideroff = 1;
      }

      if(!rideroff)
      {
         pos = origin - right*48;
         trace = gi.trace(pos.vec3(), tmpmins.vec3(), tmpmaxs.vec3(), pos.vec3(), nullptr, mask);
         if(!trace.allsolid)
         {
            rider->setOrigin(trace.endpos);
            rideroff = 1;
         }

         if(!rideroff)
         {
            pos = origin;
            pos[gravity_axis[gravaxis].z] += 40*gravity_axis[gravaxis].sign;
            trace = gi.trace(pos.vec3(), tmpmins.vec3(), tmpmaxs.vec3(), pos.vec3(), nullptr, mask);
            if(!trace.allsolid)
            {
               rider->setOrigin(trace.endpos);
               rideroff = 1;
               velocity *= 0.75;
               rider->velocity[gravity_axis[gravaxis].z] += 150*gravity_axis[gravaxis].sign;
            }
         }
      }
   }

   // rider couldn't get off
   if(!rideroff)
   {
      // force a dead rider to get off
      if(rider->health <= 0)
      {
         pos = origin;
         pos[gravity_axis[gravaxis].z] += 16*gravity_axis[gravaxis].sign;
         rider->setOrigin(pos);
         rideroff = 1;
      }
      else
         return;
   }

   // we need the rider as a sentient beyond this point
   sent = (Sentient *)rider.ptr;

   //set stuff for the rider
   // only worry about weapon stuff if rider is not dead
   if(rider->health > 0)
   {
      // switch rider back to previous weapon
      if(oldweapon)
      {
         weapon = static_cast<Weapon *>(oldweapon.ptr);
      }
      else
      {
         // switch player to fists
         weapon = static_cast<Weapon *>(sent->FindItem("fists"));
      }

      if(weapon)
         sent->ForceChangeWeapon(weapon);
   }
   sent->takeWeapon("HoverWeap");

   KillGuages();

   sent->SetHoverbike(nullptr);

   // do some human player type stuff
   if(ridertype == RIDER_PLAYER)
   {
      Player *pent = static_cast<Player *>(sent);

      if(pent->ViewMode() == THIRD_PERSON)
         pent->SetViewMode(THIRD_PERSON);
   }

   // turn off the hovering sounds
   sndflags    = HBSND_NONE;
   soundtimmer = 0;
   rider->edict->s.sound = 0;

   // set stuff for the bike
   rider     = nullptr;
   ridertype = RIDER_OTHER;
   setMoveType(MOVETYPE_HOVERBIKE);
   //set the hoverbike's angles
   move_angles[PITCH] = move_angles[ROLL] = 0;
   ApplyMoveAngles();
   SetGravityAxis(gravaxis);
   showModel();
   edict->s.effects |= EF_HOVER;
   // send hoverbike to client again
   edict->svflags &= ~SVF_NOCLIENT;

   // remove after a minute of not being used if in DM
   if(deathmatch->value)
   {
      // remove after only 5 sec in a bikes only map
      if(level.bikesonly)
         respawntimer = level.time + 5;
      else
      {
         respawntimer = health * 0.2;
         if(respawntimer < 3)
            respawntimer = 3;
         respawntimer += level.time;
      }
   }
}

//==============================================================

void Hoverbike::BikePain(Event *ev)
{
   Vector pos, dir;
   int count;

   // make a visual pain cue
   pos = ev->GetVector(3);
   if(pos != vec_zero)
   {
      count = static_cast<int>(ev->GetFloat(1));
      dir = pos - origin;
      dir.normalize();
      SpawnSparks(pos, dir, count);
   }

   if(damagetimer < level.time)
   {
      RandomSound("hb_collide", 1, CHAN_VOICE, ATTN_NORM);
      damagetimer = level.time + 1;
   }
}

void Hoverbike::BikeKilled(Event *ev)
{
   // bike is already dead
   if(deadflag)
      return;

   // set the bike as being dead
   deadflag = 1;

   // if in a bike only map, also kill the rider
   if(level.bikesonly && rider)
   {
      auto event = new Event(EV_Killed);
      event->AddEntity(ev->GetEntity(1)); // attacker
      event->AddInteger(1000); // damage (doesn't matter)
      event->AddEntity(ev->GetEntity(3)); // inflictor
      event->AddString("all"); // just say they were hit in the all
      event->AddInteger(ev->GetInteger(5)); // MOD
      rider->ProcessEvent(event);
   }

   // if the bike had a rider, give the killer an extra frag
   if(rider && ((ridertype == RIDER_PLAYER) || (ridertype == RIDER_BOT)) && !level.bikesonly)
   {
      Entity *attacker;
      const char *message1 = nullptr;
      const char *message2 = "";
      int num;

      attacker = ev->GetEntity(1);
      if(attacker && attacker->isClient())
      {
         if(attacker == rider)
         {
            num = G_Random(2);
            switch(num)
            {
            case 0:
               message1 = "shot his hoverbike in the foot";
               break;
            default:
               message1 = "felt it would be better to go on foot";
               break;
            }
            gi.bprintf(PRINT_MEDIUM, "%s %s\n", rider->client->pers.netname, message1);
            attacker->client->resp.score--;
         }
         else
         {
            num = G_Random(3);
            switch(num)
            {
            case 0:
               message1 = "'s hoverbike was destroyed by";
               break;
            case 1:
               message1 = " lost his hoverbike to";
               break;
            default:
               message1 = " gave up his hoverbike to give";
               message2 = " a chance";
               break;
            }

            gi.bprintf(PRINT_MEDIUM, "%s%s %s%s\n", rider->client->pers.netname,
                       message1,
                       attacker->client->pers.netname,
                       message2);
            attacker->client->resp.score++;
         }
      }

      // make sure that we toss our rider
      BikeGetOff();
   }

   // he still here? just toss 'em
   if(rider)
   {
      Weapon   *weapon;
      Sentient *sent = static_cast<Sentient *>(rider.ptr);

      // only worry about weapon stuff if player is not dead
      if(rider->health > 0)
      {
         // switch player back to previous weapon
         if(oldweapon)
         {
            weapon = static_cast<Weapon *>(oldweapon.ptr);
         }
         else
         {
            // switch player to fists
            weapon = static_cast<Weapon *>(sent->FindItem("fists"));
         }

         if(weapon)
            sent->ForceChangeWeapon(weapon);
      }
      sent->takeWeapon("HoverWeap");

      KillGuages();

      sent->SetHoverbike(nullptr);

      if(ridertype == RIDER_PLAYER)
      {
         Player *pent = static_cast<Player *>(rider.ptr);
         if(pent->ViewMode() == THIRD_PERSON)
            pent->SetViewMode(THIRD_PERSON);
      }
      // turn off the hovering sounds
      sndflags = HBSND_NONE;
      SetHoverSound();

      // set stuff for the bike
      rider         = nullptr;
      ridertype     = RIDER_OTHER;
      angles[PITCH] = 0;
      if(rand()%2)
         angles[ROLL] = 5;
      else
         angles[ROLL] = -5;
      move_angles[PITCH] = move_angles[ROLL] = 0;
      setAngles(angles);
   }

   SpawnRocketExplosion(frontbox->origin);
   SpawnRocketExplosion(backbox->origin);

   frontbox->setSolidType(SOLID_NOT);
   frontbox->PostEvent(EV_Remove, 0.1);

   backbox->setSolidType(SOLID_NOT);
   backbox->PostEvent(EV_Remove, 0.1);

   setSolidType(SOLID_NOT);
   setMoveType(MOVETYPE_NONE);
   hideModel();
   edict->s.effects &= ~EF_HOVER;

   // if in deathmatch, respawn after 15 seconds
   if(deathmatch->value && !respawntimer)
   {
      auto newbike = new Hoverbike();
      newbike->RespawnSetup(spawnspot, spawnangles, 15, gravaxis, spawnflags);
      respawntimer = 1;
   }

   PostEvent(EV_Remove, 0.1);
}

void Hoverbike::GiveExtraFrag(Entity *attacker)
{
   const char *message1 = nullptr;
   int num;

   if(attacker && attacker->isClient())
   {
      if(attacker != rider)
      {
         num = G_Random(3);
         switch(num)
         {
         case 0:
            message1 = " was shot off his bike by";
            break;
         case 1:
            message1 = " was pushed off his bike by";
            break;
         default:
            message1 = " was convinced to not ride a bike by";
            break;
         }

         gi.bprintf(PRINT_MEDIUM, "%s%s %s\n", rider->client->pers.netname,
                    message1, attacker->client->pers.netname);
         attacker->client->resp.score++;
      }
   }
}

//==============================================================

qboolean Hoverbike::Ride(usercmd_t *ucmd)
{
   if(!rider)
      return false;

   // store movement commands in the bike
   forwardmove = ucmd->forwardmove;
   sidemove    = ucmd->sidemove;
   upmove      = ucmd->upmove;

   if(ridertype == RIDER_PLAYER)
   {
      // set player position and velocity
      if(static_cast<Player *>(rider.ptr)->WatchCamera() != rider)
      {
         short temporigin[3];

         // 
         // we save off the origin so that camera's origins are not messed up
         //
         temporigin[0] = rider->client->ps.pmove.origin[0];
         temporigin[1] = rider->client->ps.pmove.origin[1];
         temporigin[2] = rider->client->ps.pmove.origin[2];

         RiderMove(ucmd);

         rider->client->ps.pmove.origin[0] = temporigin[0];
         rider->client->ps.pmove.origin[1] = temporigin[1];
         rider->client->ps.pmove.origin[2] = temporigin[2];
      }
      else
      {
         RiderMove(ucmd);
      }
   }

   rider->setAngles(angles);

   return true;
}

// only used for player riders
void Hoverbike::RiderMove(usercmd_t *ucmd)
{
   pmove_t pm;
   Player *pent = static_cast<Player *>(rider.ptr);

   // decide wether or not to do client side prediction
   if(pent->ViewMode() == THIRD_PERSON)
      pent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
   else
      pent->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;

   pent->client->ps.pmove.pm_type = PM_ONBIKE;
   pent->client->ps.pmove.gravity = 0;
   pent->SetMoveInfo(&pm, ucmd);
   pm.snapinitial = false;

   // perform a pmove
   gi.Pmove(&pm);

   pent->GetMoveInfo(&pm);
}

void Hoverbike::Postthink()
{
   Vector tmpvec;
   Sentient *sent;

   // stuff to do if we don't have a rider
   if(!rider)
   {
      // check for respawning if in deathmatch
      if((deathmatch->value) && (respawntimer > 0) && (respawntimer < level.time))
      {
         // cause a respawn by killing the bike
         ProcessEvent(EV_Killed);
      }
      return;
   }

   // get rider as a sentient
   sent = static_cast<Sentient *>(rider.ptr);

   if(!sent->IsOnBike())
   {
      // shouldn't happen, but just incase
      return;
   }

   // check for the player getting off
   // player can get off by ducking
   // can't get off in a bikes only map
   if(!level.bikesonly && (upmove < -50) && (getontimer < level.time))
   {
      BikeGetOff();
      return;
   }

   // temporarily make the bike not solid
   setSolidType(SOLID_NOT);
   frontbox->setSolidType(SOLID_NOT);
   backbox->setSolidType(SOLID_NOT);

   move_angles[YAW] = sent->WishedBikeYaw();

   //calc the hover force & pitch angle
   Hover();
   //apply client's controls to the bike
   ApplyControls();
   //apply bike angle changes
   ApplyMoveAngles();
   //apply hoverbike movement physics
   G_Physics_Hoverbike(this);
   // check if bike is still alive
   if(health <= 0)
      return;
   // check to see if the rider is no longer on the bike
   if(!rider)
      return;

   //calculate the hoverbike's horizontal speed
   speed = sqrtf(velocity[0] * velocity[0] + velocity[1] * velocity[1]);
   forward_speed = speed;

   // set hoverbike's hovering sound
   SetHoverSound();

   // set origin for the weapon model
   sent->CurrentWeapon()->setOrigin(origin);

   //update info guages
   UpdateGuages();

   // set rider's origin and velocity to the bike's
   rider->setOrigin(origin);
   rider->velocity = velocity;

   // stuff to do for player riders
   if(ridertype == RIDER_PLAYER)
   {
      // incriment view bobbing
      bobsin += 0.2;
      if(bobsin > M_PI*2)
         bobsin -= M_PI*2;
      bobfrac = sin(bobsin);
      if(bobfrac < 0)
         bobfrac *= -1;
   }

   // make the bike solid again
   setSolidType(SOLID_BBOX);
   frontbox->setSolidType(SOLID_BBOX);
   backbox->setSolidType(SOLID_BBOX);

   // touch any triggers we may be in
   Hoverbike_TouchTriggers(this, rider);
   Hoverbike_TouchTriggers(frontbox, rider);
   Hoverbike_TouchTriggers(backbox, rider);
}

void Hoverbike::Hover()
{
   trace_t f_trace, m_trace, b_trace;
   Vector down, start, end;
   Vector tmins, tmaxs;
   float hoverdist, tmpflt;
   int content;
   const gravityaxis_t &grav = gravity_axis[gravaxis];

   // clip mask to use for hovering
   int mask = MASK_PLAYERSOLID | MASK_WATER;

   // this is used to keep track of various different
   // things from the hovering traces
   int   hoverflags  = 0;
   float hoverpoints = 1;

   down[grav.z] -= grav.sign;
   tmins = Vector(-18, -18, -18);
   tmaxs = Vector(18, 18, 18);
   if(grav.sign > 0)
   {
      tmins[grav.z] = -12;
      tmaxs[grav.z] = 0;
   }
   else
   {
      tmins[grav.z] = 0;
      tmaxs[grav.z] = 12;
   }

   //get the trace for the front
   start = frontbox->origin;
   end = start;
   end[grav.z] = origin[grav.z] - HOVER_HEIGHT*grav.sign;
   f_trace = G_Trace(start, tmins, tmaxs, end, rider, mask, "Hoverbike::Hover");
   // check if front is in something solid
   if(f_trace.allsolid)
   {
      f_trace.fraction = 1;
      hoverflags |= 8;
   }
   else
      hoverpoints++;
   // check if front is in water, if so, make sure there's some thrust
   end = start + down*12;
   content = gi.pointcontents(end.vec3());
   if(content & MASK_WATER)
   {
      if(f_trace.fraction > 0.1)
         f_trace.fraction = 0.1;
      hoverflags |= 1;
   }

   //get the trace for the middle
   start = origin;
   end   = start;
   end[grav.z] = origin[grav.z] - HOVER_HEIGHT*grav.sign;
   m_trace = G_Trace(start, tmins, tmaxs, end, rider, mask, "Hoverbike::Hover");
   // check if middle is in water, if so, make sure there's some thrust
   end = start + down*12;
   content = gi.pointcontents(end.vec3());
   if(content & MASK_WATER)
   {
      if(m_trace.fraction > 0.1)
         m_trace.fraction = 0.1;
      hoverflags |= 2;
   }

   //get the trace for the back
   start = backbox->origin;
   end   = start;
   end[grav.z] = origin[grav.z] - HOVER_HEIGHT*grav.sign;
   b_trace = G_Trace(start, tmins, tmaxs, end, rider, mask, "Hoverbike::Hover");
   // check if front is in something solid
   if(b_trace.allsolid)
   {
      b_trace.fraction = 1;
      hoverflags |= 16;
   }
   else
      hoverpoints++;
   // check if back is in water, if so, make sure there's some thrust
   end     = start + down*12;
   content = gi.pointcontents(end.vec3());
   if(content & MASK_WATER)
   {
      if(b_trace.fraction > 0.1)
         b_trace.fraction = 0.1;
      hoverflags |= 4;
   }

   //check if not even near the ground & add the correct
   //amount of gravity while we're at it
   if(f_trace.fraction == 1 && m_trace.fraction == 1 && b_trace.fraction == 1)
   {
      // add gravity (hacked to smooth out arial movement and bouncing)
      if((velocity[grav.z]*grav.sign) > 0)
      {
         tmpflt = gravity*sv_gravity->value*FRAMETIME*0.9;
         velocity += down*tmpflt;
      }
      else
      {
         tmpflt = gravity*sv_gravity->value*FRAMETIME*0.7;
         velocity += down*tmpflt;
      }

      if((velocity[grav.z]*grav.sign) < 0 && airpitch_timmer < level.time)
      {
         // tilt hoverbike's pitch in air for a nice, simple effect
         if(move_angles[PITCH] > HOVER_FLY_PITCH)
         {
            move_angles[PITCH] -= HOVER_PITCH_SPEED*FRAMETIME;
            if(move_angles[PITCH] < HOVER_FLY_PITCH)
               move_angles[PITCH] = HOVER_FLY_PITCH;
         }
         else if(move_angles[PITCH] < HOVER_FLY_PITCH)
         {
            move_angles[PITCH] += HOVER_PITCH_SPEED*FRAMETIME;
            if(move_angles[PITCH] > HOVER_FLY_PITCH)
               move_angles[PITCH] = HOVER_FLY_PITCH;
         }
      }
      // stop playing hovering sound
      sndflags &= ~HBSND_HOVERING;
      sndflags &= ~HBSND_CLOSE;

      groundentity = NULL;

      return;
   }
   else
      airpitch_timmer = level.time + 0.5;

   //add downward force
   velocity += down*HOVER_GROUNDGRAVITY*FRAMETIME;

   // set bike's ground entity
   if(f_trace.fraction < m_trace.fraction)
   {
      if(f_trace.fraction < b_trace.fraction)
      {
         groundentity = f_trace.ent;
         groundentity_linkcount = f_trace.ent->linkcount;
         if(groundentity->entity->getSolidType() == SOLID_BSP)
         {
            groundplane = f_trace.plane;
            groundsurface = f_trace.surface;
            groundcontents = f_trace.contents;
         }
      }
      else
      {
         groundentity = b_trace.ent;
         groundentity_linkcount = f_trace.ent->linkcount;
         if(groundentity->entity->getSolidType() == SOLID_BSP)
         {
            groundplane = b_trace.plane;
            groundsurface = b_trace.surface;
            groundcontents = b_trace.contents;
         }
      }
   }
   else
   {
      if(m_trace.fraction < b_trace.fraction)
      {
         groundentity = m_trace.ent;
         groundentity_linkcount = f_trace.ent->linkcount;
         if(groundentity->entity->getSolidType() == SOLID_BSP)
         {
            groundplane = m_trace.plane;
            groundsurface = m_trace.surface;
            groundcontents = m_trace.contents;
         }
      }
      else
      {
         groundentity = b_trace.ent;
         groundentity_linkcount = f_trace.ent->linkcount;
         if(groundentity->entity->getSolidType() == SOLID_BSP)
         {
            groundplane = b_trace.plane;
            groundsurface = b_trace.surface;
            groundcontents = b_trace.contents;
         }
      }
   }

   // calc hover distance
   hoverdist = m_trace.fraction;
   if(!(hoverflags & 8)) // front not stuck in something solid
      hoverdist += f_trace.fraction;
   if(!(hoverflags & 16)) // back not stuck in something solid
      hoverdist += b_trace.fraction;
   if(hoverdist)
      hoverdist /= hoverpoints;

   // calc hover direction (for when on extremly sloped surfaces)
   down *= -0.8;
   if(f_trace.fraction < 1 && !(hoverflags & 1))
      down += Vector(f_trace.plane.normal);
   if(m_trace.fraction < 1 && !(hoverflags & 2))
      down += Vector(m_trace.plane.normal);
   if(b_trace.fraction < 1 && !(hoverflags & 4))
      down += Vector(b_trace.plane.normal);
   down.normalize();

   //calc hovering force
   tmpflt  = 1 - hoverdist;
   tmpflt *= HOVER_FORCE;
   tmpflt *= tmpflt;
   tmpflt *= 0.5;
   tmpflt  = tmpflt*FRAMETIME;

   velocity += down*tmpflt;

   // an extra lift if still moving towards ground
   if((velocity[grav.z]*grav.sign) < 0)
   {
      velocity += down*tmpflt;

      if((velocity[grav.z]*grav.sign) > 0)
         velocity[grav.z] = 0;
   }

   // set down back to normal
   down = vec_zero;
   down[grav.z] -= grav.sign;

   // dampen upward velocity if not vertical boosting
   if((velocity[grav.z]*grav.sign) < 0 || boosttimmer < level.time)
      velocity[gravity_axis[gravaxis].z] *= 0.65;

   // play correct hovering sound
   sndflags |= HBSND_HOVERING;
   if(hoverdist < 0.5)
      sndflags |= HBSND_CLOSE;
   else
      sndflags &= ~HBSND_CLOSE;

   // calc pitch angle of the bike
   if(hoverflags & 8) // front is inside a solid
   {
      if(hoverflags & 16) // back is also inside a solid
      {
         start = { 1, 0, 0 }; // will result in 0 pitch
      }
      else
      {
         // start = Vector(m_trace.endpos) - Vector(b_trace.endpos);
         tmins[0] = m_trace.endpos[grav.x];
         tmins[1] = m_trace.endpos[grav.y]*grav.sign;
         tmins[2] = m_trace.endpos[grav.z]*grav.sign;

         tmaxs[0] = b_trace.endpos[grav.x];
         tmaxs[1] = b_trace.endpos[grav.y]*grav.sign;
         tmaxs[2] = b_trace.endpos[grav.z]*grav.sign;

         start = tmins - tmaxs;
      }
   }
   else if(hoverflags & 16) // just the back is inside a solid
   {
      // start = Vector(f_trace.endpos) - Vector(m_trace.endpos);
      tmins[0] = f_trace.endpos[grav.x];
      tmins[1] = f_trace.endpos[grav.y]*grav.sign;
      tmins[2] = f_trace.endpos[grav.z]*grav.sign;

      tmaxs[0] = m_trace.endpos[grav.x];
      tmaxs[1] = m_trace.endpos[grav.y]*grav.sign;
      tmaxs[2] = m_trace.endpos[grav.z]*grav.sign;

      start = tmins - tmaxs;
   }
   else // both front and back are ok
   {
      // start = Vector(f_trace.endpos) - Vector(b_trace.endpos);
      tmins[0] = f_trace.endpos[grav.x];
      tmins[1] = f_trace.endpos[grav.y]*grav.sign;
      tmins[2] = f_trace.endpos[grav.z]*grav.sign;

      tmaxs[0] = b_trace.endpos[grav.x];
      tmaxs[1] = b_trace.endpos[grav.y]*grav.sign;
      tmaxs[2] = b_trace.endpos[grav.z]*grav.sign;

      start = tmins - tmaxs;
   }
   end = start.toAngles();
   move_angles[PITCH] = -end[PITCH];
   if(move_angles[PITCH] < -180)
      move_angles[PITCH] += 360;
}

void Hoverbike::ApplyControls()
{
   Vector forward, right, tmpvec;
   float f_speed, s_speed, tmpflt;
   Vector t[3];
   const gravityaxis_t &grav = gravity_axis[gravaxis];
   qboolean isturboing, seteffects;
   Sentient *sent;
   Weapon *weapon;

   // determine if we need to set the hoverbike's effects
   if(effectstimmer < level.time)
   {
      seteffects    = true;
      effectstimmer = level.time + HOVER_EFFECTS_TIME;
   }
   else
      seteffects = false;

   // don't want to include roll in direction
   // calcs, because some weird shit will happen
   tmpvec[YAW] = move_angles[YAW];
   if(!groundentity) // don't take pitch into account if in the air
      tmpvec[PITCH] = 0;
   else if(angles[PITCH] < 0)
      tmpvec[PITCH] = move_angles[PITCH]*0.5;
   else
      tmpvec[PITCH] = move_angles[PITCH]*0.3;

   // orient velocity directions to bike's gravityaxis

   tmpvec.AngleVectors(&t[0], &t[1], &t[2]);
   forward[grav.x] = t[0][0];
   forward[grav.y] = t[0][1] * grav.sign;
   forward[grav.z] = t[0][2] * grav.sign;
   right  [grav.x] = t[1][0];
   right  [grav.y] = t[1][1] * grav.sign;
   right  [grav.z] = t[1][2] * grav.sign;

   //get forward & side speeds
   f_speed = DotProduct(velocity.vec3(), forward.vec3());
   s_speed = DotProduct(velocity.vec3(), right.vec3());

   // get rider as a sentient
   sent = (Sentient *)rider.ptr;
   // get rider's weapon too
   weapon = sent->CurrentWeapon();

   // set turboing value
   isturboing = false;
   if(sent->BikeIsTurboing())
      isturboing = true;

   //forward movement

   //check for acceleration
   if(forwardmove > 0 && f_speed < HOVER_MAX_SPEED)
   {
      if(f_speed < 0)
         tmpflt = 1.5;
      else
      {
         tmpflt = 1- (f_speed/HOVER_ACCEL_CEILING);
         tmpflt *= tmpflt;
         if(f_speed < (HOVER_MAX_SPEED*0.5))
            tmpflt *= 1.5;
      }

      f_speed += HOVER_ACCEL*FRAMETIME*tmpflt;

      // limit speed to max speed to prevent jerky movement
      if(f_speed > HOVER_MAX_SPEED)
         f_speed = HOVER_MAX_SPEED;

      //change sound to accelerating sound
      sndflags |= HBSND_ACCEL;
      sndflags &= ~HBSND_BRAKE;

      // set thruster effect
      if(seteffects)
         weapon->edict->s.effects |= EF_HOVERTHRUST;
   }
   else if(forwardmove < 0 && f_speed > HOVER_MIN_SPEED) //check for applying brakes
   {
      f_speed -= (HOVER_BRAKES*FRAMETIME);

      // limit speed to max speed to prevent jerky movement
      if(f_speed < HOVER_MIN_SPEED)
         f_speed = HOVER_MIN_SPEED;

      // play braking sound
      sndflags |= HBSND_BRAKE;
      sndflags &= ~HBSND_ACCEL;

      // set thruster effect off
      if(seteffects)
         weapon->edict->s.effects &= ~EF_HOVERTHRUST;
   }
   else
   {
      // set acceleration and brake sound flags
      if(forwardmove > 0)
      {
         //change sound to accelerating sound
         sndflags |= HBSND_ACCEL;
         sndflags &= ~HBSND_BRAKE;

         // set thruster effect
         if(seteffects)
            weapon->edict->s.effects |= EF_HOVERTHRUST;
      }
      else if(forwardmove < 0)
      {
         // play braking sound
         sndflags |= HBSND_BRAKE;
         sndflags &= ~HBSND_ACCEL;

         // set thruster effect off
         if(seteffects)
            weapon->edict->s.effects &= ~EF_HOVERTHRUST;
      }
      else
      {
         // don't play either
         sndflags &= ~HBSND_ACCEL;
         sndflags &= ~HBSND_BRAKE;

         // set thruster effect off
         if(seteffects)
            weapon->edict->s.effects &= ~EF_HOVERTHRUST;
      }

      // drift diff. over one second
      tmpflt = f_speed*HOVER_DRIFT;
      // scale it to a frame
      tmpflt *= FRAMETIME;
      // minimum amount of drift
      if(tmpflt < HOVER_MIN_DRIFT && tmpflt > -HOVER_MIN_DRIFT)
      {
         if(tmpflt < 0)
            tmpflt = -HOVER_MIN_DRIFT;
         else if(tmpflt > 0)
            tmpflt = HOVER_MIN_DRIFT;
      }
      // max amount of drift
      else if(tmpflt > HOVER_MAX_DRIFT || tmpflt < -HOVER_MAX_DRIFT)
      {
         if(tmpflt > 0)
            tmpflt = HOVER_MAX_DRIFT;
         else
            tmpflt = -HOVER_MAX_DRIFT;
      }
      // almost no drift when in the air
      if(!groundentity)
         tmpflt *= 0.01;
      // take the drift out of the backward movement
      if(f_speed < 0)
      {
         // don't drift below min speed if braking
         if(forwardmove < 0 && f_speed < HOVER_MIN_SPEED)
         {
            f_speed += tmpflt;
            if(f_speed > HOVER_MIN_SPEED)
               f_speed = HOVER_MIN_SPEED;
         }
         else
         {
            f_speed += tmpflt;
            if(f_speed > 0)
               f_speed = 0;
         }
      }
      else if(f_speed > 0) // take the drift out of the forward movement
      {
         // don't drift below max turbo speed if turboing
         if(isturboing && turbo >= HOVER_TURBO_USERATE)
         {
            if(f_speed > HOVER_TURBO_MAXSPEED)
            {
               f_speed += tmpflt;
               if(f_speed < HOVER_TURBO_MAXSPEED)
                  f_speed = HOVER_TURBO_MAXSPEED;
            }
         }
         // don't drift below max speed if accelerating
         else if(forwardmove > 0 && f_speed > HOVER_MAX_SPEED)
         {
            f_speed += tmpflt;
            if(f_speed < HOVER_MAX_SPEED)
               f_speed = HOVER_MAX_SPEED;
         }
         else
         {
            f_speed += tmpflt;
            if(f_speed < 0)
               f_speed = 0;
         }
      }
   }

   // side movement

      //moving to the left
   if(sidemove < 0 && s_speed >(-HOVER_MAX_SIDE))
   {
      // less straifing control in the air
      if(groundentity)
         s_speed -= (HOVER_STRAFE*FRAMETIME);
      else
         s_speed -= (HOVER_STRAFE*FRAMETIME*0.75);

      // limit speed to max speed to prevent jerky movement
      if(s_speed < (-HOVER_MAX_SIDE))
         s_speed = -HOVER_MAX_SIDE;

      // roll bike to the left
      if(strafingroll >(-HOVER_STRAFE_ROLL))
         strafingroll -= HOVER_STRAFE_ROLL*0.4;
      else
         strafingroll = -HOVER_STRAFE_ROLL;
   }
   //moving to the right
   else if(sidemove > 0 && s_speed < HOVER_MAX_SIDE)
   {
      // less straifing control in the air
      if(groundentity)
         s_speed += (HOVER_STRAFE*FRAMETIME);
      else
         s_speed += (HOVER_STRAFE*FRAMETIME*0.75);

      // limit speed to max speed to prevent jerky movement
      if(s_speed > HOVER_MAX_SIDE)
         s_speed = HOVER_MAX_SIDE;

      // roll bike to the right
      if(strafingroll < HOVER_STRAFE_ROLL)
         strafingroll += HOVER_STRAFE_ROLL*0.4;
      else
         strafingroll = HOVER_STRAFE_ROLL;
   }
   // drift down the sideways movement
   else
   {
      // drift diff. over one second
      tmpflt = s_speed*HOVER_SIDE_DRIFT;
      // scale it to a frame
      tmpflt *= FRAMETIME;
      // minimum amount of drift
      if(tmpflt > 0 && tmpflt < 1)
         tmpflt = 1;
      else if(tmpflt < 0 && tmpflt > -1)
         tmpflt = -1;
      // almost no drift when in the air
      if(!groundentity)
         tmpflt *= 0.05;
      // take the drift out of the side movement
      if(s_speed < 0)
      {
         // don't drift past max side speed if strafing left
         if(sidemove < 0 && s_speed < (-HOVER_MAX_SIDE))
         {
            s_speed += tmpflt;
            if(s_speed >(-HOVER_MAX_SIDE))
               s_speed = (-HOVER_MAX_SIDE);
         }
         else
         {
            s_speed += tmpflt;
            if(s_speed > 0)
               s_speed = 0;
         }
      }
      else if(s_speed > 0)
      {
         // don't drift past max side speed if strafing left
         if(sidemove > 0 && s_speed > HOVER_MAX_SIDE)
         {
            s_speed += tmpflt;
            if(s_speed < HOVER_MAX_SIDE)
               s_speed = HOVER_MAX_SIDE;
         }
         else
         {
            s_speed += tmpflt;
            if(s_speed < 0)
               s_speed = 0;
         }
      }
      // take care of strafe roll stuff
      if(sidemove < 0) // still strafing left
      {
         // roll bike to the left
         if(strafingroll >(-HOVER_STRAFE_ROLL))
            strafingroll -= HOVER_STRAFE_ROLL*0.3;
         else
            strafingroll = -HOVER_STRAFE_ROLL;
      }
      else if(sidemove > 0) // still strafing right
      {
         // roll bike to the right
         if(strafingroll < HOVER_STRAFE_ROLL)
            strafingroll += HOVER_STRAFE_ROLL*0.3;
         else
            strafingroll = HOVER_STRAFE_ROLL;
      }
      else // not strafing
      {
         // reduce strafe roll if not 0
         if(strafingroll < 0)
         {
            strafingroll += HOVER_STRAFE_ROLL*0.1;
            if(strafingroll > 0)
               strafingroll = 0;
         }
         else if(strafingroll > 0)
         {
            strafingroll -= HOVER_STRAFE_ROLL*0.1;
            if(strafingroll < 0)
               strafingroll = 0;
         }
      }
   }

   // vertical booster movement

   // check for vertical booster
   if(upmove > 0)
   {
      if(boosttimmer < level.time)
      {
         if(groundentity)
         {
            if((velocity[grav.z]*grav.sign) < HOVER_BOOSTER)
               velocity[grav.z] = HOVER_BOOSTER*grav.sign;

            // play sound
            RandomSound("hb_booster", 1, CHAN_BODY, ATTN_NORM);

            // make particle effect for booster
            SpawnHoverBoost(origin, angles[YAW]);

            // set button release timmer
            boosttimmer = level.time + 0.2;
         }
      }
      else // player must release button to use again
         boosttimmer = level.time + 0.2;
   }
   else
      boosttimmer = 0;

   // check for turbo booster
   if(isturboing && (turbo >= HOVER_TURBO_USERATE) && (getontimer < level.time))
   {
      turbo -= HOVER_TURBO_USERATE;
      if(turbo < 0)
         turbo = 0;

      if(f_speed < HOVER_TURBO_MAXSPEED)
      {
         if(f_speed < 0)
            tmpflt = 3;
         else
         {
            tmpflt = 1- (f_speed/HOVER_ACCEL_CEILING);
            tmpflt *= tmpflt;
            if(f_speed < (HOVER_MAX_SPEED*0.5))
               tmpflt *= 3;
            else
               tmpflt *= 2;
         }

         tmpflt = HOVER_ACCEL*FRAMETIME*tmpflt;
         // make sure we have a minimal amount of thrust
         if(tmpflt < HOVER_TURBO_BOOST*FRAMETIME)
            tmpflt = HOVER_TURBO_BOOST*FRAMETIME;

         f_speed += tmpflt;

         if(f_speed > HOVER_TURBO_MAXSPEED)
            f_speed = HOVER_TURBO_MAXSPEED;
      }

      // set turbo effect on
      if(seteffects)
         weapon->edict->s.effects |= EF_HOVERTURBO;
   }
   else
   {
      if(!isturboing) // only refill if not trying to boost
      {
         turbo += HOVER_TURBO_REFILL;
         if(turbo > 100)
            turbo = 100;
      }

      // set turbo effect off
      if(seteffects)
         weapon->edict->s.effects &= ~EF_HOVERTURBO;
   }

   // keep turbo filled if in god mode
   if(rider->flags & FL_GODMODE)
      turbo = 100;

   //calc the roll angle for the sideways movement
   if(s_speed == 0)
      tmpflt = 0;
   else
   {
      tmpflt = s_speed/HOVER_MAX_SPEED;
      tmpflt *= -HOVER_ROLL;
      if(tmpflt < (-HOVER_ROLL))
         tmpflt = -HOVER_ROLL;
      else if(tmpflt > HOVER_ROLL)
         tmpflt = HOVER_ROLL;
   }
   move_angles[ROLL] = tmpflt + strafingroll;

   //calc the new velocity
   velocity[grav.x] = 0;
   velocity[grav.y] = 0;
   velocity += f_speed*forward;
   velocity += s_speed*right;
}

void Hoverbike::ApplyMoveAngles()
{
   Vector t[3];
   Vector forward;
   Vector right;
   Vector up;
   const gravityaxis_t &grav = gravity_axis[gravaxis];

   // orient the bike's angles to its gravityaxis and apply them

   move_angles.AngleVectors(&t[0], &t[1], &t[2]);
   forward[grav.x] = t[0][0];
   forward[grav.y] = t[0][1] * grav.sign;
   forward[grav.z] = t[0][2] * grav.sign;
   right  [grav.x] = t[1][0];
   right  [grav.y] = t[1][1] * grav.sign;
   right  [grav.z] = t[1][2] * grav.sign;
   up     [grav.x] = t[2][0];
   up     [grav.y] = t[2][1] * grav.sign;
   up     [grav.z] = t[2][2] * grav.sign;
   VectorsToEulerAngles(forward.vec3(), right.vec3(), up.vec3(), angles.vec3());

   setAngles(angles);
}

void Hoverbike::SetGravityAxis(int axis)
{
   Vector newmins, newmaxs;

   edict->s.effects &= ~(EF_GRAVITY_AXIS_0 | EF_GRAVITY_AXIS_1 | EF_GRAVITY_AXIS_2);
   edict->s.effects |= GRAVITYAXIS_TO_EFFECTS(axis);
   gravaxis          = EFFECTS_TO_GRAVITYAXIS(edict->s.effects);
   groundentity      = nullptr;

   const gravityaxis_t &grav = gravity_axis[gravaxis];

   // orient bounding boxes to new gravityaxis
   newmins[grav.x] = -20;
   newmins[grav.y] = -20;
   if(rider)
   {
      if(grav.sign < 0)
         newmins[grav.z] = -32;
      else
         newmins[grav.z] = -12;
   }
   else
   {
      if(grav.sign < 0)
         newmins[grav.z] = -16;
      else
         newmins[grav.z] = -12;
   }

   newmaxs[grav.x] = 20;
   newmaxs[grav.y] = 20;
   if(rider)
   {
      if(grav.sign < 0)
         newmaxs[grav.z] = 12;
      else
         newmaxs[grav.z] = 32;
   }
   else
   {
      if(grav.sign < 0)
         newmaxs[grav.z] = 12;
      else
         newmaxs[grav.z] = 16;
   }

   setSize(newmins, newmaxs);

   if(gravity_axis[gravaxis].sign < 0)
   {
      newmins[grav.z] = -32;
      newmaxs[grav.z] = 12;
   }
   else
   {
      newmins[grav.z] = -12;
      newmaxs[grav.z] = 32;
   }

   frontbox->setSize(newmins, newmaxs);

   if(gravity_axis[gravaxis].sign < 0)
   {
      newmins[grav.z] = -24;
      newmaxs[grav.z] = 12;
   }
   else
   {
      newmins[grav.z] = -12;
      newmaxs[grav.z] = 24;
   }

   backbox->setSize(newmins, newmaxs);
}

//==============================================================
// set the hovering sound to play for the hoverbike

void Hoverbike::SetHoverSound()
{
   const char *newsound;
   Weapon *weapon;

   // only set the bike's sound every other frame
   if(soundtimmer < level.time)
      soundtimmer = level.time + HOVER_EFFECTS_TIME;
   else
      return;

   // turbo sound takes presidence over the other sounds
   weapon = static_cast<Sentient *>(rider.ptr)->CurrentWeapon();
   if(weapon->edict->s.effects & EF_HOVERTURBO) 
   {
      if(sndflags & HBSND_HOVERING) 
      {
         if(sndflags & HBSND_ACCEL) 
            newsound = "hb_turboaccel";
         else 
            newsound = "hb_turbohover";
      }
      else
         newsound = "hb_turbo";
   }
   else if(sndflags & HBSND_HOVERING) 
   {
      if(sndflags & HBSND_CLOSE) 
      {
         if(sndflags & HBSND_ACCEL) 
            newsound = "hb_closeaccel";
         else if(sndflags & HBSND_BRAKE) 
            newsound = "hb_closebrake";
         else 
            newsound = "hb_closehover";
      }
      else 
      {
         if(sndflags & HBSND_ACCEL)
            newsound = "hb_accel";
         else if(sndflags & HBSND_BRAKE)
            newsound = "hb_brake";
         else
            newsound = "hb_hover";
      }
   }
   else 
   {
      if(sndflags & HBSND_ACCEL)
         newsound = "hb_accel";
      else if(sndflags & HBSND_BRAKE)
         newsound = "hb_brake";
      else
         newsound = "";
   }

   // hover sound is set each time to prevent special case problems
   if(!newsound[0])
      rider->edict->s.sound = 0;
   else 
   {
      const char *soundname = gi.Alias_FindRandom(edict->s.modelindex, newsound);
      rider->edict->s.sound = gi.soundindex(soundname);
      rider->edict->s.sound |= ATTN_NORM<<14;
   }

   currsound = newsound;
}

//==============================================================

void Hoverbike::WeaponChangeSound()
{
   RandomSound("hb_weapswitch", 0.75, CHAN_BODY, ATTN_NORM);
}

void Hoverbike::WeaponNoAmmoSound()
{
   RandomSound("hb_noammo", 1, CHAN_BODY, ATTN_NORM);
}

void Hoverbike::CollisionSound(Vector before, Vector after)
{
   Vector diff(after - before);
   float  delta = diff.length();

   if(delta > 800)
      RandomSound("hb_collide", 1, CHAN_VOICE, ATTN_NORM);    // hard collision
   else if(delta > 400)
      RandomSound("hb_collide", 0.75, CHAN_VOICE, ATTN_NORM); // medium collision
   else if(delta > 250)
      RandomSound("hb_scrape", 1, CHAN_VOICE, ATTN_NORM);     // just a scrape
   else if(delta > 100)
      RandomSound("hb_scrape", 0.5, CHAN_VOICE, ATTN_NORM);   // just a light scrape
}

//==============================================================
// weapon changing functions

void Hoverbike::NextWeapon()
{
   weaponmode++;
   if(weaponmode >= NUM_HOVERWEAPONS)
      weaponmode = 0;

   WeaponChangeSound();
   ((Sentient *)rider.ptr)->CurrentWeapon()->NextAttack(0.5);
}

void Hoverbike::PreviousWeapon()
{
   weaponmode--;
   if(weaponmode < 0)
      weaponmode = NUM_HOVERWEAPONS - 1;

   WeaponChangeSound();
   ((Sentient *)rider.ptr)->CurrentWeapon()->NextAttack(0.5);
}

void Hoverbike::SelectWeapon(int weapon)
{
   if(weapon < 0)
      weapon = 0;
   else if(weapon >= NUM_HOVERWEAPONS)
      weapon = NUM_HOVERWEAPONS - 1;

   weaponmode = weapon;

   WeaponChangeSound();
   ((Sentient *)rider.ptr)->CurrentWeapon()->NextAttack(1);
}

//==============================================================

void Hoverbike::MakeGuages()
{
   HoverbikeGuage *guage;

   // only players need the guages
   if(ridertype != RIDER_PLAYER)
      return;

   if(!(guage = speed_bar))
      guage = new HoverbikeGuage();
   guage->Setup(rider, HBGUAGE_SPEEDBAR);
   speed_bar = guage;

   if(!(guage = speed_number))
      guage = new HoverbikeGuage();
   guage->Setup(rider, HBGUAGE_SPEEDNUM);
   speed_number = guage;

   if(!(guage = turbo_bar))
      guage = new HoverbikeGuage();
   guage->Setup(rider, HBGUAGE_TURBOBAR);
   turbo_bar = guage;

   if(!(guage = health_bar))
      guage = new HoverbikeGuage();
   guage->Setup(rider, HBGUAGE_HEALTHBAR);
   health_bar = guage;

   if(!(guage = ammo_number))
      guage = new HoverbikeGuage();
   guage->Setup(rider, HBGUAGE_AMMONUM);
   ammo_number = guage;

   if(!(guage = weapon_icon))
      guage = new HoverbikeGuage();
   guage->Setup(rider, HBGUAGE_WEAPONICON);
   weapon_icon = guage;
}

void Hoverbike::KillGuages()
{
   if(speed_bar)
      speed_bar->ProcessEvent(EV_Remove);
   if(speed_number)
      speed_number->ProcessEvent(EV_Remove);
   if(turbo_bar)
      turbo_bar->ProcessEvent(EV_Remove);
   if(health_bar)
      health_bar->ProcessEvent(EV_Remove);
   if(ammo_number)
      ammo_number->ProcessEvent(EV_Remove);
   if(weapon_icon)
      weapon_icon->ProcessEvent(EV_Remove);
}

void Hoverbike::GuagesViewerOn()
{
   // not needed if rider not a player
   if(ridertype != RIDER_PLAYER)
      return;

   if(speed_bar)
      speed_bar->edict->s.renderfx |= RF_VIEWERMODEL;

   if(speed_number)
      speed_number->edict->s.renderfx |= RF_VIEWERMODEL;

   if(turbo_bar)
      turbo_bar->edict->s.renderfx |= RF_VIEWERMODEL;

   if(health_bar)
      health_bar->edict->s.renderfx |= RF_VIEWERMODEL;

   if(ammo_number)
      ammo_number->edict->s.renderfx |= RF_VIEWERMODEL;

   if(weapon_icon)
      weapon_icon->edict->s.renderfx |= RF_VIEWERMODEL;
}

void Hoverbike::GuagesViewerOff()
{
   // not needed if rider not a player
   if(ridertype != RIDER_PLAYER)
      return;

   if(speed_bar)
      speed_bar->edict->s.renderfx &= ~RF_VIEWERMODEL;
   if(speed_number)
      speed_number->edict->s.renderfx &= ~RF_VIEWERMODEL;
   if(turbo_bar)
      turbo_bar->edict->s.renderfx &= ~RF_VIEWERMODEL;
   if(health_bar)
      health_bar->edict->s.renderfx &= ~RF_VIEWERMODEL;
   if(ammo_number)
      ammo_number->edict->s.renderfx &= ~RF_VIEWERMODEL;
   if(weapon_icon)
      weapon_icon->edict->s.renderfx &= ~RF_VIEWERMODEL;
}


void Hoverbike::UpdateGuages()
{
   // not needed if rider not a player
   if(ridertype != RIDER_PLAYER)
      return;

   if(speed_bar)
      speed_bar->SetValue(forward_speed);
   if(speed_number)
      speed_number->SetValue(forward_speed);
   if(turbo_bar)
      turbo_bar->SetValue(turbo);
   if(health_bar)
      health_bar->SetValue(health);
   if(ammo_number)
   {
      if(weaponmode == HWMODE_ROCKETS)
         ammo_number->SetValue(rockets);
      else if(weaponmode == HWMODE_CHAINGUN)
         ammo_number->SetValue(bullets);
      else
         ammo_number->SetValue(mines);
   }
   if(weapon_icon)
      weapon_icon->SetValue(weaponmode);
}

//==============================================================
// Hoverbike Viewmodel Guages
//==============================================================

CLASS_DECLARATION(Entity, HoverbikeGuage, nullptr);

ResponseDef HoverbikeGuage::Responses[] =
{
	{ nullptr, nullptr }
};

void HoverbikeGuage::Setup(Entity *owner, int type)
{
   int    groupindex;
   int    tri_num;
   Vector orient;
   str    guagemodel;

   // set guage type
   guagetype = type;

   // NOTE: gunmodelindex is used to specify which secondary
   // view model it should be. Can be from 1 to 8
   switch(guagetype)
   {
   case HBGUAGE_SPEEDBAR:
      guagemodel = "hb_speedbar.def";
      edict->s.gunmodelindex  = 1;
      break;
   case HBGUAGE_SPEEDNUM:
      guagemodel = "hb_speednum.def";
      edict->s.gunmodelindex  = 2;
      break;
   case HBGUAGE_TURBOBAR:
      guagemodel = "hb_turbo.def";
      edict->s.gunmodelindex  = 3;
      break;
   case HBGUAGE_HEALTHBAR:
      guagemodel = "hb_health.def";
      edict->s.gunmodelindex  = 4;
      break;
   case HBGUAGE_AMMONUM:
      guagemodel = "hb_ammonum.def";
      edict->s.gunmodelindex  = 5;
      break;
   case HBGUAGE_WEAPONICON:
      guagemodel = "hb_weap.def";
      edict->s.gunmodelindex  = 6;
      break;
   default:
      gi.dprintf("HoverbikeGuage::Setup : bad guage type\n");
      PostEvent(EV_Remove, 0);
      return;
   }

   setSolidType(SOLID_NOT);
   setMoveType(MOVETYPE_NONE);

   // uses the built in fake bone that works like Quake2's modelindexes did.
   gi.GetBoneInfo(owner->edict->s.modelindex, "origin", &groupindex, &tri_num, orient.vec3());
   attach(owner->entnum, groupindex, tri_num, orient);
   setOrigin(vec_zero);
   setModel(guagemodel.c_str());
   showModel();

   StopAnimating(); // make sure the thing doesn't animate

   edict->s.effects  |= EF_VIEWMODEL; // specify it as a secondary view model

   // only send this to it's owner
   edict->owner    = owner->edict;
   edict->svflags |= SVF_ONLYPARENT;
}

void HoverbikeGuage::SetValue(int value)
{
   switch(guagetype)
   {
   case HBGUAGE_SPEEDBAR:
   case HBGUAGE_SPEEDNUM:
      value *= 0.2;
      if(value < 0)
         value = 0;
      if(value > 250)
         value = 250;
      if(edict->s.frame != value)
         edict->s.frame = value;
      break;
   case HBGUAGE_TURBOBAR:
      if(edict->s.frame != value)
         edict->s.frame = value;
      break;
   case HBGUAGE_HEALTHBAR:
      if(value < 0)
         value = 0;
      else if(value > HOVER_MAX_HEALTH)
         value = HOVER_MAX_HEALTH;
      value *= (50.0/HOVER_MAX_HEALTH);
      if(edict->s.frame != value)
         edict->s.frame = value;
      break;
   case HBGUAGE_AMMONUM:
      if(edict->s.frame != value)
         edict->s.frame = value;
      break;
   case HBGUAGE_WEAPONICON:
      if(edict->s.frame != value)
         edict->s.frame = value;
      break;
   default:
      gi.dprintf("HoverbikeGuage::SetValue : bad guage type\n");
      break;
   }
}

// EOF
