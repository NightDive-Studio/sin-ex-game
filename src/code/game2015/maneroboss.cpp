/*
================================================================
MANERO DA BOSS MAN
================================================================

Copyright (C) 2020 by Night Dive Studios, Inc.
All rights reserved.

See the license.txt file for conditions and terms of use for this code.
*/

#include "maneroboss.h"
#include "surface.h"
#include "chaingun.h"
#include "flamethrower.h"


//===============================================================
// MANERO BOSS HELICOPTER
//===============================================================

/*****************************************************************************/
/*SINED monster_helicopterboss (0 .5 .8) (0 0 0) (0 0 0) NOT_SOLID

/*****************************************************************************/

CLASS_DECLARATION(ScriptModel, ManeroCopter, "monster_helicopterboss");

Event EV_ManeroCopter_Attack("startattacking");
Event EV_ManeroCopter_StopAttack("stopattacking");
Event EV_ManeroCopter_FireRockets("firerockets");
Event EV_ManeroCopter_RemoveManero("removemanero");

ResponseDef ManeroCopter::Responses[] =
{
   { &EV_ManeroCopter_Attack,       (Response)&ManeroCopter::StartAttacking },
   { &EV_ManeroCopter_StopAttack,   (Response)&ManeroCopter::StopAttacking  },
   { &EV_ManeroCopter_FireRockets,  (Response)&ManeroCopter::FireRockets    },
   { &EV_ManeroCopter_RemoveManero, (Response)&ManeroCopter::RemoveManero   },
   { nullptr, nullptr },
};

ManeroCopter::ManeroCopter() : ScriptModel()
{
   int groupindex;
   int tri_num;
   Vector orient;

   setModel("lamprey_man.def");
   setSize({ -150, -150, 0 }, { 150, 150, 150 });

   flags &= ~FL_POSTTHINK; // don't start out attacking

   // spawn the manero model
   maneromodel = new Entity();
   maneromodel->setModel("manero.def");
   maneromodel->setSolidType(SOLID_NOT);
   maneromodel->edict->s.effects |= EF_WARM;
   gi.GetBoneInfo(edict->s.modelindex, "seat", &groupindex, &tri_num, orient.vec3());
   maneromodel->attach(entnum, groupindex, tri_num, orient);
   maneromodel->setOrigin(Vector(60, 24, 8));
   maneromodel->RandomAnimate("heliride", NULL);

   manerogun = new Entity();
   manerogun->setModel("hvgun.def");
   manerogun->setSolidType(SOLID_NOT);
   gi.GetBoneInfo(maneromodel->edict->s.modelindex, "gun", &groupindex, &tri_num, orient.vec3());
   manerogun->attach(maneromodel->entnum, groupindex, tri_num, orient);
   manerogun->setOrigin(vec_zero);
}

// this enables the copters attack state
void ManeroCopter::StartAttacking(Event *ev)
{
   lastrockettime  = level.time + 1 + G_Random(1);
   lastguntime     = level.time + 1 + G_Random(3);
   lastgrenadetime = level.time + 1 + G_Random(2);

   // start doing the copters post thinking
   flags |= FL_POSTTHINK;
}

// this disables the copters attack state
void ManeroCopter::StopAttacking(Event *ev)
{
   flags &= ~FL_POSTTHINK;
}

void ManeroCopter::Postthink(void)
{
   float  delta, yawofs;
   Vector playerdir;
   Vector tmpvec;


   // get directional vectors that we need
   Vector forward(orientation[0]);
   Vector left(orientation[1]);
   Vector down(Vector(orientation[2]) * -1);

   // get vector pointing to the player
   playerdir = G_GetEntity(1)->worldorigin - worldorigin;
   playerdir.normalize();

   // point manero's gun at the player
   manerogun->setAngles(vec_zero);
   tmpvec = playerdir.toAngles();
   tmpvec[PITCH] *= -1;
   tmpvec -= manerogun->worldangles;
   manerogun->setAngles(tmpvec);

   // find the yaw region the player is in relative to the copter
   tmpvec = playerdir.toAngles();
   yawofs = tmpvec[YAW] - angles[YAW];
   if(yawofs < -180)
      yawofs += 360;
   else if(yawofs > 180)
      yawofs -= 360;

   if((yawofs < 40) && (yawofs > -40)) // rockets yaw range
   {
      // check for firing rockets
      if(lastrockettime < level.time)
      {
         delta = DotProduct(playerdir.vec3(), forward.vec3());

         if(delta > 0.4)
         {
            lastfiredir = playerdir;
            auto e = new Event(EV_ManeroCopter_FireRockets);
            e->AddInteger(4);
            PostEvent(e, 0.1);

            lastrockettime = level.time + 2 + G_Random(2);
         }
      }
   }
   else if((yawofs > 30) && (yawofs < 150)) // Manero's chaingun yaw range
   {
      if(lastguntime < level.time)
      {
         delta = DotProduct(playerdir.vec3(), left.vec3());

         if(delta > 0.3)
         {
            FireGun();

            // have periodic pauses in the gun fire
            if(G_Random() < 0.075)
               lastguntime = level.time + 0.7 + G_Random(1.5);
         }
      }
   }

   // check for dropping grenades
   delta = DotProduct(playerdir.vec3(), down.vec3());
   if(delta > 0.7 && lastgrenadetime < level.time)
   {
      FireGrenade();

      lastgrenadetime = level.time + 0.7 + G_Random(0.5);
      // let's be nice and delay the chaingun
      lastguntime = level.time + 0.5 + G_Random(0.5);
   }
}

void ManeroCopter::RemoveManero(Event *ev)
{
   // make sure attack mode is off
   flags &= ~FL_POSTTHINK;

   if(manerogun)
   {
      manerogun->hideModel();
      manerogun->PostEvent(EV_Remove, 0.1);
   }

   if(maneromodel)
   {
      maneromodel->hideModel();
      maneromodel->PostEvent(EV_Remove, 0.1);
   }
}

void ManeroCopter::FireRockets(Event *ev)
{
   float yawofs;
   Vector tmpvec;
   int rocketcount = ev->GetInteger(1);

   Vector forward(orientation[0]);
   Vector right(orientation[1]);
   Vector up(orientation[2]);

   // set the place to fire from
   Vector firepos(worldorigin + forward*200);
   Vector playerpos(G_GetEntity(1)->worldorigin);
   switch(rocketcount)
   {
   case 4:
      firepos += right*40;
      firepos += up*32;
      break;
   case 3:
      firepos += forward*20;
      firepos += right*15;
      firepos += up*16;
      // a little bit of lead
      if(G_Random() < 0.7)
         playerpos += G_GetEntity(1)->velocity * 0.25;
      break;
   case 2:
      firepos += forward*20;
      firepos -= right*15;
      firepos += up*16;
      // a bit more lead
      if(G_Random() < 0.7)
         playerpos += G_GetEntity(1)->velocity * 0.5;
      break;
   default:
      firepos -= right*40;
      firepos += up*32;
      // even more lead
      if(G_Random() < 0.7)
         playerpos += G_GetEntity(1)->velocity;
      break;
   }

   Vector playerdir(playerpos - firepos);
   playerdir.normalize();
   tmpvec = playerdir.toAngles();
   yawofs = tmpvec[YAW] - angles[YAW];
   if(yawofs < -180)
      yawofs += 360;
   else if(yawofs > 180)
      yawofs -= 360;

   // use last fire dir if player is now out of fire arc
   if((yawofs < -40) && (yawofs > 40))
   {
      playerdir = lastfiredir;
   }
   else // check fire pitch as well
   {
      yawofs = DotProduct(playerdir.vec3(), forward.vec3());

      if(yawofs < 0.3)
      {
         // nope, use old dir
         playerdir = lastfiredir;
      }
   }

   // fire off the rocket
   tmpvec = firepos + playerdir*256;
   auto rocket = new StingerRocket();
   rocket->Setup(this, firepos, tmpvec, playerdir, playerdir);
   lastfiredir = playerdir;

   // make firing sound
   sound("weapons/rlaunch/mix2.wav");

   rocketcount--;
   if(rocketcount)
   {
      auto e = new Event(EV_ManeroCopter_FireRockets);
      e->AddInteger(rocketcount);
      PostEvent(e, 0.1);
   }
}

extern Event EV_Grenade_Explode;

void ManeroCopter::FireGrenade(void)
{
   Vector playerdir(G_GetEntity(1)->centroid - worldorigin);
   playerdir.normalize();
   playerdir.x *= 1.5;
   playerdir.y *= 1.5;

   auto grenade = new Grenade;
   grenade->Setup(this, worldorigin, playerdir, vec_zero, vec_zero);

   // set it to blow up faster than a regular grenade
   grenade->CancelEventsOfType(EV_Grenade_Explode);
   auto ev = new Event(EV_Grenade_Explode);
   ev->AddEntity(world);
   grenade->PostEvent(ev, 1.0 + G_Random(0.5));

   // make firing sound
   sound("weapons/grenade/fire1.wav");
}

void ManeroCopter::FireGun(void)
{
   Vector firepos(GunPosition());
   Vector playerdir(G_GetEntity(1)->centroid - firepos);
   playerdir.normalize();

   FireBullets({ 150, 150, 150 }, 5, 10, firepos, playerdir);

   // make firing sound
   sound("weapons/hvgun/2a.wav", 3);
}

#define MAX_RICOCHETS 10

void ManeroCopter::TraceAttack(Vector start, Vector end, int damage, trace_t *trace, int numricochets, int kick)
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
   {
      return;
   }

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
      surfaceManager.DamageSurface(trace, damage, this);

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
            if(!ricochet)
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
               gi.WriteByte(0);
               gi.multicast(org.vec3(), MULTICAST_PVS);
            }
            MadeBreakingSound(org, this);
         }

         if(ent->takedamage)
         {
            if(trace->intersect.valid)
            {
               // We hit a valid group so send in location based damage
               ent->Damage(
                  this,
                  this,
                  damage,
                  trace->endpos,
                  dir,
                  trace->plane.normal,
                  kick,
                  0,
                  MOD_CHAINGUN,
                  trace->intersect.parentgroup,
                  -1,
                  trace->intersect.damage_multiplier);
            }
            else
            {
               // We didn't hit any groups, so send in generic damage
               ent->Damage(
                  this,
                  this,
                  damage,
                  trace->endpos,
                  dir,
                  trace->plane.normal,
                  kick,
                  0,
                  MOD_CHAINGUN,
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

   if(numricochets < MAX_RICOCHETS)
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
      // since this is a ricochet, we don't ignore the wewapon owner this time.
      //
      *trace = G_FullTrace(org, vec_zero, vec_zero, endpos, 5, NULL, MASK_SHOT, "BulletWeapon::TraceAttack");
      if(trace->fraction != 1.0)
      {
         endpos = trace->endpos;
         TraceAttack(org, endpos, damage * 0.8f, trace, numricochets - 1, kick);
      }
   }
}

void ManeroCopter::FireBullets(Vector spread, int mindamage, int maxdamage, Vector src, Vector dir)
{
   Vector	end;
   trace_t	trace;
   Vector	right;
   Vector	up;

   maneromodel->angles.AngleVectors(NULL, &right, &up);

   end = src +
      dir   * 8192 +
      right * G_CRandom() * spread.x +
      up    * G_CRandom() * spread.y;

   trace = G_FullTrace(src, vec_zero, vec_zero, end, 5, this, MASK_SHOT, "ManeroCopter::FireBullets");
   FireTracer(src, Vector(trace.endpos));

   if(trace.fraction != 1.0)
      TraceAttack(src, trace.endpos, mindamage + (int)G_Random(maxdamage - mindamage + 1), &trace, MAX_RICOCHETS, 0);
}

void ManeroCopter::FireTracer(Vector src, Vector end)
{
   Vector dir(end - src);
   dir.normalize();

   auto tracer = new ManeroCopterTracer();

   tracer->angles = dir.toAngles();
   tracer->angles[PITCH] = -tracer->angles[PITCH] + 90;

   tracer->setAngles(tracer->angles);
   tracer->setOrigin(manerogun->worldorigin);
   tracer->velocity = dir*2000;
}

Vector ManeroCopter::GunPosition(void)
{
   vec3_t   trans[3];
   vec3_t   orient;
   int      groupindex;
   int      tri_num;
   Vector   offset = vec_zero;
   Vector   result;

   // get the gun position of the actor
   if(!gi.GetBoneInfo(maneromodel->edict->s.modelindex, "gun", &groupindex, &tri_num, orient))
   {
      // Gun doesn't have a barrel, just return the default
      return manerogun->worldorigin;
   }

   gi.GetBoneTransform(maneromodel->edict->s.modelindex, groupindex, tri_num, orient, maneromodel->edict->s.anim,
                       maneromodel->edict->s.frame, maneromodel->edict->s.scale, trans, offset.vec3());

   MatrixTransformVector(offset.vec3(), orientation, result.vec3());
   result += maneromodel->worldorigin;

   return result;
}

//===============================================================

CLASS_DECLARATION(ScriptModel, ManeroCopterTracer, "coptertracer");

ResponseDef ManeroCopterTracer::Responses[] =
{
   { &EV_Touch, (Response)&ManeroCopterTracer::RemoveTracer },
   { NULL, NULL },
};

ManeroCopterTracer::ManeroCopterTracer()
{
   setMoveType(MOVETYPE_FLY);
   setSolidType(SOLID_TRIGGER);
   setModel("sprites/tracer.spr");
   setSize({ -4, -4, -4 }, { 4, 4, 4 });
   edict->s.renderfx &= ~RF_FRAMELERP;
   edict->clipmask = MASK_SOLID;

   PostEvent(EV_Remove, 5);
}

void ManeroCopterTracer::RemoveTracer(Event *ev)
{
   Hide();
   PostEvent(EV_Remove, 0.1);
}

//===============================================================
// MANERO BOSS HIMSELF
//===============================================================

CLASS_DECLARATION(Actor, ManeroBoss, "monster_maneroboss");

Event EV_Manero_ShieldsOn("shieldson");
Event EV_Manero_ShieldsOff("shieldsoff");
Event EV_Manero_CloakOn("cloakon");
Event EV_Manero_CloakOff("cloakoff");
Event EV_Manero_FireNuke("firenuke");

ResponseDef ManeroBoss::Responses[] =
{
   { &EV_Manero_ShieldsOn,  (Response)&ManeroBoss::ShieldsOn   },
   { &EV_Manero_ShieldsOff, (Response)&ManeroBoss::ShieldsOff  },
   { &EV_Manero_CloakOn,    (Response)&ManeroBoss::CloakOn     },
   { &EV_Manero_CloakOff,   (Response)&ManeroBoss::CloakOff    },
   { &EV_Manero_FireNuke,   (Response)&ManeroBoss::FireNuke    },
   { &EV_Damage,            (Response)&ManeroBoss::ArmorDamage },
   { &EV_Pain,              (Response)&ManeroBoss::Pain        },
   { &EV_Killed,            (Response)&ManeroBoss::Killed      },
   { &EV_Actor_Dead,        (Response)&ManeroBoss::Dead        },
   { NULL, NULL },
};

ManeroBoss::ManeroBoss() : Actor()
{
   int armamount;

   // sorry Mr. player, you can't mess with Manero
   mass = 2000;

   // give him a second weapon... the nuke >)
   secondaryWeapon = new ManeroIP36();

   secondaryWeapon->SetOwner(this);
   secondaryWeapon->AttachToOwner();

   dropweapon = false;
   flags |= FL_POSTTHINK;

   // give Manero lotsa armor
   if(skill->value == 0)
      armamount = 200;
   if(skill->value == 1)
      armamount = 250;
   else
      armamount = 300;
   giveItem("RiotHelmet", armamount);
   giveItem("FlakJacket", armamount);
   giveItem("FlakPants",  armamount);
}

void ManeroBoss::Postthink(void)
{
   // ensure that the nuke stays transparent
   // when Manero's cloaked
   if(translucence && secondaryWeapon)
      secondaryWeapon->setAlpha(0.1);
}

void ManeroBoss::ShieldsOn(Event *ev)
{
   int armamount;

   flags |= FL_SHIELDS;
   edict->s.renderfx |= RF_DLIGHT;
   edict->s.renderfx |= RF_ENVMAPPED;
   edict->s.color_r   = 0;
   edict->s.color_g   = 0;
   edict->s.color_b   = 0.75;
   edict->s.radius    = 120;
   edict->s.effects  &= ~EF_WARM;

   if(currentWeapon)
      currentWeapon->edict->s.renderfx |= RF_ENVMAPPED;

   if(secondaryWeapon)
      secondaryWeapon->edict->s.renderfx |= RF_ENVMAPPED;

   // give Manero lotsa armor
   if(skill->value == 0)
      armamount = 200;
   if(skill->value == 1)
      armamount = 250;
   else
      armamount = 300;
   giveItem("RiotHelmet", armamount);
   giveItem("FlakJacket", armamount);
   giveItem("FlakPants",  armamount);
}

void ManeroBoss::ShieldsOff(Event *ev)
{
   flags &= ~FL_SHIELDS;
   edict->s.renderfx &= ~RF_DLIGHT;
   edict->s.renderfx &= ~RF_ENVMAPPED;
   edict->s.color_r   = 0;
   edict->s.color_g   = 0;
   edict->s.color_b   = 0;
   edict->s.radius    = 0;
   edict->s.effects  |= EF_WARM;

   if(currentWeapon)
      currentWeapon->edict->s.renderfx &= ~RF_ENVMAPPED;

   if(secondaryWeapon)
      secondaryWeapon->edict->s.renderfx &= ~RF_ENVMAPPED;

   sound("player/pkup/shield/fail.wav", 2, CHAN_ITEM);
}

void ManeroBoss::CloakOn(Event *ev)
{
   int armamount;

   edict->s.renderfx |= RF_DLIGHT;
   edict->s.color_r   = 0.75;
   edict->s.color_g   = 0.75;
   edict->s.color_b   = 0.75;
   edict->s.radius    = -120;
   setAlpha(0);

   if(currentWeapon)
      currentWeapon->setAlpha(0.1);

   if(secondaryWeapon)
      secondaryWeapon->setAlpha(0.1);

   sound("player/pkup/shield/activate3.wav", 2, CHAN_ITEM);

   // give Manero lotsa armor
   if(skill->value == 0)
      armamount = 200;
   if(skill->value == 1)
      armamount = 250;
   else
      armamount = 300;
   giveItem("RiotHelmet", armamount);
   giveItem("FlakJacket", armamount);
   giveItem("FlakPants",  armamount);
}

void ManeroBoss::CloakOff(Event *ev)
{
   edict->s.renderfx &= ~RF_DLIGHT;
   edict->s.color_r   = 0;
   edict->s.color_g   = 0;
   edict->s.color_b   = 0;
   edict->s.radius    = 0;
   setAlpha(1.0);

   if(currentWeapon)
      currentWeapon->setAlpha(1.0);

   if(secondaryWeapon)
      secondaryWeapon->setAlpha(1.0);

   sound("player/pkup/shield/fail.wav", 2, CHAN_ITEM);
}

void ManeroBoss::FireNuke(Event *ev)
{
   int armamount;

   if(!secondaryWeapon)
      return;

   if(!secondaryWeapon->ReadyToFire())
   {
      PostEvent(EV_Manero_FireNuke, 0.1);
      return;
   }

   secondaryWeapon->Fire();

   // make sure Manero's got some armor
   if(skill->value == 0)
      armamount = 50;
   if(skill->value == 1)
      armamount = 100;
   else
      armamount = 150;
   giveItem("RiotHelmet", armamount);
   giveItem("FlakJacket", armamount);
   giveItem("FlakPants",  armamount);
}

void ManeroBoss::ArmorDamage(Event *ev)
{
   // let his almightyness ignore damaging himself
   Entity *inflictor = ev->GetEntity(2);
   Entity *attacker  = ev->GetEntity(3);

   if((inflictor == this) || (attacker == this))
      return;

   Sentient::ArmorDamage(ev);
}

void ManeroBoss::Pain(Event *ev)
{
   float   damage = ev->GetFloat(1);
   Entity *ent    = ev->GetEntity(2);

   // if it's a Sentient and not liked, attack 'em.
   if(ent && ent->isSubclassOf<Sentient>() && !Likes(ent))
   {
      MakeEnemy(ent);
      if(ent != currentEnemy)
      {
         currentEnemy = BestTarget();
      }
   }

   if(damage <= 0)
      return;

   float oldhealth = (health + damage) / max_health;
   float newhealth = health / max_health;

   SetVariable("other", ev->GetEntity(2));

   // If we pass more than one range,  
   if((oldhealth > 0.875) && (newhealth <= 0.875))
   {
      DoAction("health_good", true);
   }
   if((oldhealth > 0.75) && (newhealth <= 0.75))
   {
      DoAction("health_ok", true);
   }
   if((oldhealth > 0.5) && (newhealth <= 0.5))
   {
      DoAction("health_med", true);
   }
   if((oldhealth > 0.25) && (newhealth <= 0.25))
   {
      DoAction("health_low", true);
   }

   if(damage <= pain_threshold)
   {
      Chatter("snd_pain_taunt", 5, true);
      return;
   }

   if(strncmp(animname.c_str(), "pain", 4) && strncmp(animname.c_str(), "crouch_pain", 11))
   {
      str aname;
      int index;

      //
      // determine pain animation
      //
      if(!strncmp(animname.c_str(), "crouch", 6))
      {
         aname = "crouch_";
      }
      aname += "pain_";
      aname += ev->GetString(3);
      index = gi.Anim_Random(edict->s.modelindex, aname.c_str());
      if((index == -1) && !strncmp(animname.c_str(), "crouch", 6))
      {
         aname = "crouch_pain";
         index = gi.Anim_Random(edict->s.modelindex, aname.c_str());
      }

      if(index == -1)
         aname = "pain";

      SetVariable("painanim", aname.c_str());
      DoAction("pain");
   }
}

void ManeroBoss::Killed(Event *ev)
{
   Vector		dir;
   Event			*event;
   int			i;
   str         dname;

   if(secondaryWeapon)
   {
      secondaryWeapon->hideModel();
      secondaryWeapon->PostEvent(EV_Remove, 0.1);
   }

   ExecuteThread("manero_is_a_dead_dumb_bitch", true);

   CheckWater();
   StopAnimating();
   CancelPendingEvents();

   // don't allow them to fly, think, or swim anymore
   flags &= ~(FL_PRETHINK | FL_SWIM | FL_FLY);

   deadflag     = DEAD_DYING;
   takedamage   = DAMAGE_YES;
   groundentity = NULL;

   Entity *attacker     = ev->GetEntity(1);
   Entity *inflictor    = ev->GetEntity(3);
   int     meansofdeath = ev->GetInteger(5);

   SetVariable("other", ev->GetEntity(1));
   if(!DoAction("killed") && actorthread)
      actorthread->ProcessEvent(EV_ScriptThread_End);

   // Turn off dlight and shadow
   edict->s.renderfx &= ~(RF_DLIGHT|RF_XFLIP);

   // skin darkening for death from flames
   if(inflictor->isSubclassOf<ThrowerFlame>() || meansofdeath == MOD_FLAMETHROWER)
   {
      edict->s.renderfx |= RF_LIGHTOFFSET;
      edict->s.lightofs = -127;

      CancelEventsOfType(EV_Sentient_HurtFlame);
      edict->s.effects  &= ~EF_FLAMES;
      edict->s.effects  |= EF_DEATHFLAMES;
   }
   else
   {
      // turn off the actor's heat signature
      edict->s.effects &= ~EF_WARM;
   }

   if(currentWeapon)
      currentWeapon->PostEvent(EV_Remove, 0.1);

   animOverride = false;

   //
   // determine death animation
   //
   if(!strncmp(animname.c_str(), "crouch", 6))
   {
      dname = "crouch_";
   }

   if(deathgib)
   {
      const char * location;

      location = ev->GetString(4);

      // Check for location first otherwise randomize
      if(!strcmp(location, "torso_upper"))
         dname += "gibdeath_upper";
      else if(!strcmp(location, "torso_lower"))
         dname += "gibdeath_lower";
      else if(strstr(location, "leg"))
         dname += "gibdeath_lower";
      else if(strstr(location, "arm"))
         dname += "gibdeath_upper";
      else if(strstr(location, "head"))
         dname += "gibdeath_upper";
      else if(G_Random() > 0.5)
         dname += "gibdeath_upper";
      else
         dname += "gibdeath_lower";
   }
   else
   {
      dname += "death_";
      dname += ev->GetString(4);
   }

   i = gi.Anim_Random(edict->s.modelindex, dname.c_str());

   if((i == -1) && !strncmp(animname.c_str(), "crouch", 6))
   {
      dname = "crouch_death";
      i = gi.Anim_Random(edict->s.modelindex, dname.c_str());
   }

   if(i == -1)
   {
      dname = "death";
   }

   if((i != -1) &&  (!strncmp(dname.c_str(), "gibdeath", 7)))
   {
      Event *ev1;

      ev1 = new Event(EV_Gib);
      ev1->AddInteger(1);
      ProcessEvent(ev1);
   }
   if(attacker)
   {
      float damage   = ev->GetFloat(2);
      str   location = ev->GetString(4);

      event = new Event(EV_GotKill);
      event->AddEntity(this);
      event->AddInteger(damage);
      event->AddEntity(inflictor);
      event->AddString(location);
      event->AddInteger(meansofdeath);
      event->AddInteger(deathgib);
      attacker->ProcessEvent(event);
   }

   SetAnim(dname.c_str(), EV_Actor_Dead);

   // Call changeanim immediatly since we're no longer calling prethink
   ChangeAnim();

   //
   // moved this here so guys would not be solid right away
   //
   edict->svflags |= SVF_DEADMONSTER;
   edict->clipmask = MASK_DEADSOLID;

   if(velocity.z < 10)
      velocity.z += G_Random(300);

   angles.x = 0;
   angles.z = 0;
   setAngles(angles);
}

void ManeroBoss::Dead(Event *ev)
{
   Vector   min, max;

   StopAnimating();
   if(!groundentity && (velocity != vec_zero))
   {
      // wait until we hit the ground
      PostEvent(ev, FRAMETIME);
      return;
   }

   deadflag = DEAD_DEAD;
   maxs[0] *= 2.0f;
   maxs[1] *= 2.0f;
   maxs[2] *= 0.3f;
   setSize(mins, maxs);
   edict->svflags |= SVF_DEADMONSTER;
   setMoveType(MOVETYPE_NONE);
   setOrigin(worldorigin);
   takedamage = DAMAGE_NO;
}

//===============================================================
// WEAPONS MODIFIED FOR MANERO BOSS
//===============================================================

CLASS_DECLARATION(StingerPack, ManeroStingerPack, "weapon_manerostingerpack");

ResponseDef ManeroStingerPack::Responses[] =
{
   { &EV_Weapon_Shoot, (Response)&ManeroStingerPack::Shoot },
   { NULL, NULL }
};

ManeroStingerPack::ManeroStingerPack() : StingerPack()
{
   SetMinRange(40);
}

void ManeroStingerPack::Shoot(Event *ev)
{
   StingerPack::Shoot(ev);
   NextAttack(2.5 - (skill->value*0.5));
}

CLASS_DECLARATION(IP36, ManeroIP36, "weapon_maneroip36");

ResponseDef ManeroIP36::Responses[] =
{
   { &EV_Weapon_Shoot, (Response)&ManeroIP36::Shoot },
   { NULL, NULL }
};

void ManeroIP36::Shoot(Event *ev)
{
   IP36::Shoot(ev);
   NextAttack(0.1);
}

// EOF

