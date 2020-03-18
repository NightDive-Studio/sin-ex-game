/*
================================================================
CRAWLER MONSTER
================================================================

Copyright (C) 2020 by Night Dive Studios, Inc.
All rights reserved.

See the license.txt file for conditions and terms of use for this code.
*/

#include "crawler.h"
#include "vehicle.h"
#include "object.h"

//===============================================================
// CRAWLER WEAPON
//===============================================================

CLASS_DECLARATION(Projectile, CrawlerGoo, NULL);

ResponseDef CrawlerGoo::Responses[] =
{
   { &EV_Touch,   (Response)&CrawlerGoo::GooTouch },
   { &EV_FadeOut, (Response)&CrawlerGoo::FadeOut  },
   { NULL, NULL}
};

void CrawlerGoo::GooTouch(Event *ev)
{
   Entity *other = ev->GetEntity(1);
   assert(other);

   if(other->isSubclassOf<Teleporter>())
      return;

   if(other->entnum == this->owner)
      return;

   if(other->isSubclassOf<Crawler>())
      return;

   Entity *owner = G_GetEntity(this->owner);

   if(!owner)
      owner = world;

   setSolidType(SOLID_NOT);

   // Hit the sky, so remove everything
   if(HitSky())
   {
      PostEvent(EV_Remove, 0);
      return;
   }

   int damg = 10 + (int)G_Random(5);

   if(other->takedamage)
      other->Damage(this, owner, damg, worldorigin, velocity, level.impact_trace.plane.normal, 32, 0, MOD_PULSE, -1, -1, 1.0f);

   SpawnCrawlerSpitDamage(&level.impact_trace, damg, owner);

   PostEvent(EV_Remove, 0);
}

void CrawlerGoo::FadeOut(Event *ev)
{
   PostEvent(EV_FadeOut, 0.1f);

   edict->s.renderfx |= RF_TRANSLUCENT;
   translucence += 0.03f;
   if(translucence >= 0.96f)
      PostEvent(EV_Remove, 0);

   setAlpha(1.0f - translucence);
   // fade the dynamic light
   edict->s.color_g = 1.0f - translucence;
}

void CrawlerGoo::Setup(Entity *owner, Vector pos, Vector vel)
{
   this->owner = owner->entnum;
   edict->owner = owner->edict;

   // Align the projectile
   angles = vel.toAngles();
   angles[PITCH] = -angles[PITCH];
   setAngles(angles);

   // Flies like a grenade
   setMoveType(MOVETYPE_TOSS);
   setSolidType(SOLID_BBOX);
   edict->clipmask = MASK_PROJECTILE;
   setModel("crawlergoo.def");
   RandomAnimate("goo", NULL);

   // Set the flying velocity
   velocity = vel * 900;
   gravity  = 0.1;

   takedamage = DAMAGE_NO;

   flags |= FL_DONTSAVE;

   edict->s.effects |= EF_WARM;

   // Set size and origin
   setSize({ -1, -1, -1 }, { 1, 1, 1 });
   setOrigin(pos);
   worldorigin.copyTo(edict->s.old_origin);

   // Remove the projectile in the future
   PostEvent(EV_Remove, 30);
}

//===============================================================

CLASS_DECLARATION(Weapon, CrawlerWeapon, "weapon_crawlerweapon");

ResponseDef CrawlerWeapon::Responses[] =
{
   { &EV_Weapon_Shoot, (Response)&CrawlerWeapon::Shoot },
   { NULL, NULL }
};

CrawlerWeapon::CrawlerWeapon()
{
   SetModels(NULL, "view_crawlerweapon.def");
   SetAmmo("Rockets", 0, 0);

   notdroppable = true;
}

void CrawlerWeapon::Shoot(Event *ev)
{
   assert(owner);
   if(!owner)
   {
      return;
   }

   Vector pos, dir;
   GetMuzzlePosition(&pos, &dir);

   auto goo = new CrawlerGoo();
   goo->Setup(owner, pos, dir);

   NextAttack(0.2);
}

//===============================================================
// CRAWLER
//===============================================================

CLASS_DECLARATION(Actor, Crawler, "monster_crawler");

extern Event EV_Actor_IfCanShoot;
extern Event EV_Actor_Idle;

// action commands
Event EV_Crawler_JumpToCeiling("jumptoceiling");
Event EV_Crawler_OrientToCeiling("orienttoceiling");
// state setting commands
Event EV_Crawler_LikesCeiling("likesceiling");
Event EV_Crawler_LikesFloor("likesfloor");
Event EV_Crawler_CeilingHeight("ceilingheight");
// conditional commands
Event EV_Crawler_IfOnCeiling("ifonceiling");
Event EV_Crawler_IfOnFloor("ifonfloor");
Event EV_Crawler_IfCeilingOK("ifceilingok");

ResponseDef Crawler::Responses[] =
{
   { &EV_Actor_Idle,              (Response)&Crawler::IdleEvent            },
   { &EV_Pain,                    (Response)&Crawler::Pain                 },
   { &EV_Killed,                  (Response)&Crawler::Killed               },
   { &EV_Actor_Dead,              (Response)&Crawler::Dead                 },
   { &EV_Crawler_JumpToCeiling,   (Response)&Crawler::JumpToCeilingEvent   },
   { &EV_Crawler_OrientToCeiling, (Response)&Crawler::OrientToCeilingEvent },

   { &EV_Crawler_LikesCeiling,    (Response)&Crawler::LikesCeilingEvent    },
   { &EV_Crawler_LikesFloor,      (Response)&Crawler::LikesFloorEvent      },
   { &EV_Crawler_CeilingHeight,   (Response)&Crawler::CeilingHeightEvent   },

   { &EV_Crawler_IfOnCeiling,     (Response)&Crawler::IfOnCeilingEvent     },
   { &EV_Crawler_IfOnFloor,       (Response)&Crawler::IfOnFloorEvent       },
   { &EV_Crawler_IfCeilingOK,     (Response)&Crawler::IfCeilingOKEvent     },
     
   { NULL, NULL}
};

Crawler::Crawler() : Actor()
{
   setMoveType(MOVETYPE_STEP);

   modelIndex("crawlergoo.def");
   modelIndex("sprites/crawlerspitmark.spr");

   eyeposition.setXYZ(0, 0, 20);

   shots_per_attack = G_GetFloatArg("shotsperattack", 1 + (1 * skill->value));
}

void Crawler::IdleEvent(Event *ev)
{
   if((deadflag) && (actortype != IS_INANIMATE))
      return;

   auto e = new Event(EV_Behavior_Args);
   e->SetSource(EV_FROM_SCRIPT);
   e->SetThread(ev->GetThread());
   e->SetLineNumber(ev->GetLineNumber());

   e->AddEntity(this);
   e->AddString(ev->GetToken(1));
   e->AddString(ev->GetToken(2));
   SetBehavior(new CrawlerIdle(), e, ev->GetThread());
}

void Crawler::Pain(Event *ev)
{
   float   damage = ev->GetFloat(1);
   Entity *ent    = ev->GetEntity(2);

   // if it's a Sentient and not liked, attack 'em.
   if(ent && ent->isSubclassOf<Sentient>() && !Likes(ent))
   {
      MakeEnemy(ent);
      if(ent != currentEnemy)
         currentEnemy = BestTarget();
   }

   if(damage <= 0)
      return;

   float oldhealth = (health + damage) / max_health;
   float newhealth = health / max_health;

   SetVariable("other", ev->GetEntity(2));

   // If we pass more than one range,  
   if((oldhealth > 0.75) && (newhealth <= 0.75))
   {
      DoAction("health_ok");
   }
   if((oldhealth > 0.5) && (newhealth <= 0.5))
   {
      DoAction("health_med");
   }
   if((oldhealth > 0.25) && (newhealth <= 0.25))
   {
      DoAction("health_low");
   }
   if((oldhealth > 0.1) && (newhealth <= 0.1))
   {
      DoAction("health_danger");
   }

   if(damage <= pain_threshold)
   {
      Chatter("snd_pain_taunt", 5, true);
      return;
   }

   // if on the ceiling, fall off
   if(onceiling)
   {
      onceiling = false;
      setMoveType(MOVETYPE_STEP);
      groundentity = NULL;

      // do falling off ceiling animation
      SetVariable("painanim", "ceiling_pain");

      DoAction("pain");
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
      {
         aname = "pain";
      }

      SetVariable("painanim", aname.c_str());
      DoAction("pain");
   }
}

void Crawler::Killed(Event *ev)
{
   // make sure they don't die on the ceiling
   if(onceiling)
   {
      onceiling = false;
      setMoveType(MOVETYPE_STEP);
      groundentity = NULL;
   }

   Actor::Killed(ev);
}

void Crawler::Dead(Event *ev)
{
   // make sure they don't die on the ceiling
   if(onceiling)
   {
      onceiling = false;
      setMoveType(MOVETYPE_STEP);
      groundentity = NULL;
   }

   Actor::Dead(ev);
}

inline qboolean Crawler::CanMoveTo(Vector pos)
{
   Vector s;
   if(onceiling)
      s = Vector(0, 0, -STEPSIZE);
   else
      s = Vector(0, 0, STEPSIZE);

   Vector  start(worldorigin + s);
   Vector  end(pos + s);
   trace_t trace = G_Trace(start, mins, maxs, end, this, edict->clipmask, "Crawler::CanMoveTo");
   if(trace.fraction == 1)
      return true;

   return false;
}

qboolean Crawler::CanShootFrom(Vector pos, Entity *ent, qboolean usecurrentangles)
{
   int      mask;
   Vector   delta;
   Vector   start;
   Vector   end;
   float    len;
   trace_t  trace;
   Vehicle  *v;
   Entity   *t;
   Vector   ang;

   if(!currentWeapon || !WithinDistance(ent, vision_distance))
   {
      if(!currentWeapon && !has_melee)
         return false;
   }

   if(usecurrentangles)
   {
      Vector dir;

      start = pos + GunPosition() - worldorigin;
      end = ent->centroid;
      if(onceiling)
         end.z += (ent->absmin.z - ent->centroid.z) * 0.75f;
      else
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
      start = pos + GunPosition() - worldorigin;
      end = ent->centroid;
      if(onceiling)
         end.z += (ent->absmin.z - ent->centroid.z) * 0.75f;
      else
         end.z += (ent->absmax.z - ent->centroid.z) * 0.75f;
      delta = end - start;
      len = delta.length();
   }

   // check if we're too far away, or too close
   if(currentWeapon)
   {
      if((len > attack_range) || (len > currentWeapon->GetMaxRange()) || (len < currentWeapon->GetMinRange()))
         return false;

      mask = MASK_SHOT;
   }
   else
   {
      if((len > attack_range) || (len > melee_range))
         return false;

      mask = MASK_PROJECTILE;
   }

   // shoot past the guy we're shooting at
   end += delta * 4;

#if 0
   if(usecurrentangles)
   {
      G_DebugLine(start, end, 1, 0, 0, 1);
   }
   else
   {
      G_DebugLine(start, end, 1, 1, 0, 1);
   }
#endif

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
   if(t->isSubclassOf<Shatter>()         ||
      t->isSubclassOf<Object>()          ||
      t->isSubclassOf<DamageThreshold>() ||
      t->isSubclassOf<ScriptModel>())
   {
      trace = G_Trace(Vector(trace.endpos), vec_zero, vec_zero, end, t, mask, "Actor::CanShootFrom 2");
      if(trace.startsolid)
         return false;

      // If we hit the guy we wanted, then shoot
      if(trace.ent == ent->edict)
         return true;

      // If we hit someone else we don't like, then shoot
      if(IsEnemy(trace.ent->entity))
         return true;

      // Forget it then
      return false;
   }

   return false;
}

//===============================================================

void Crawler::JumpToCeilingEvent(Event *ev)
{
   if(deadflag)
      return;

   auto e = new Event(EV_Behavior_Args);
   e->SetSource(EV_FROM_SCRIPT);
   e->SetThread(ev->GetThread());
   e->SetLineNumber(ev->GetLineNumber());

   e->AddEntity(this);
   e->AddString(ev->GetToken(1));
   SetBehavior(new JumpToCeiling(), e, ev->GetThread());
}

// called when the crawler should actually
// move up towards the ceiling
void Crawler::OrientToCeilingEvent(Event *ev)
{
   onceiling = true;
   setMoveType(MOVETYPE_CEILINGSTEP);
   velocity.z   = 300;
   groundentity = NULL;
}


//===============================================================

void Crawler::LikesCeilingEvent(Event *ev)
{
   likesceiling = true;
}

void Crawler::LikesFloorEvent(Event *ev)
{
   likesceiling = false;
}

void Crawler::CeilingHeightEvent(Event *ev)
{
   if(ev->NumArgs())
      ceilingheight = ev->GetInteger(1);
   else
      ceilingheight = 512;
}

//===============================================================

void Crawler::IfOnCeilingEvent(Event *ev)
{
   ScriptThread *thread = ev->GetThread();
   assert(thread);
   if(!thread)
      return;

   if(onceiling)
      thread->ProcessCommandFromEvent(ev, 1);
}

void Crawler::IfOnFloorEvent(Event *ev)
{
   ScriptThread *thread = ev->GetThread();
   assert(thread);
   if(!thread)
      return;

   if(!onceiling)
      thread->ProcessCommandFromEvent(ev, 1);
}

// check if it's ok to jump up to the ceiling
void Crawler::IfCeilingOKEvent(Event *ev)
{
   ScriptThread *thread = ev->GetThread();
   assert(thread);
   if(!thread)
      return;

   if(onceiling)
      return;

   Vector  ceilingposition(origin + Vector(0, 0, 1024));
   trace_t trace = G_Trace(origin, mins, maxs, ceilingposition, this, edict->clipmask, "Crawler::IfCeilingOKEvent");

   // can't reach the ceiling
   if(trace.fraction == 1)
      return;

   // don't go up to a non-BSP entity
   if(trace.ent && trace.ent->solid != SOLID_BSP)
      return;

   thread->ProcessCommandFromEvent(ev, 1);
}

//===============================================================
// CRAWLER BEHAIVIORS
//===============================================================

CLASS_DECLARATION(Behavior, JumpToCeiling, NULL);

ResponseDef JumpToCeiling::Responses[] =
{
   {&EV_Behavior_Args, (Response)&JumpToCeiling::SetArgs },
   {NULL, NULL}
};

void JumpToCeiling::SetArgs(Event *ev)
{
   jumpchance = ev->GetFloat(2);
   if(!jumpchance)
      jumpchance = 1.0;
}

void JumpToCeiling::ShowInfo(Actor &self)
{
   Behavior::ShowInfo(self);

   gi.printf("\njumpchance : %f\n", jumpchance);
}

void JumpToCeiling::Begin(Actor &self)
{
   // check if we want to jump
   // haleyjd 20170605: added Crawler subclass check
   if(!self.isSubclassOf<Crawler>() || (((Crawler *)&self)->onceiling) || (G_Random() > jumpchance))
   {
      jumpchance = 0; // to make evaluate crap out
      return;
   }

   // check that the ceiling's ok
   Vector  ceilingposition(self.origin + Vector(0, 0, 1024));
   trace_t trace = G_Trace(self.origin, self.mins, self.maxs, ceilingposition, &self, self.edict->clipmask, "JumpToCeiling::Begin");

   if((trace.fraction == 1) || (trace.ent && trace.ent->solid != SOLID_BSP) || self.HitSky(&trace))
   {
      jumpchance = 0;
      return;
   }

   self.SetAnim("ceilingjump");
}

qboolean JumpToCeiling::Evaluate(Actor &self)
{
   // sorry, we don't want to jump
   if(!jumpchance)
      return false;

   Vector ceilingposition(self.origin);
   ceilingposition.z += STEPSIZE;
   trace_t trace = G_Trace(self.origin, self.mins, self.maxs, ceilingposition, &self, self.edict->clipmask, "JumpToCeiling::Evaluate");

   // we should basically be on the ceiling now
   if(trace.fraction < 1)
      return false;

   return true;
}

void JumpToCeiling::End(Actor &self)
{
   // SINEX_FIXME: unsafe cast
   if(((Crawler *)&self)->onceiling)
      self.SetAnim("ceiling_idle");
   else
      self.SetAnim("idle");
}

/****************************************************************************

  CrawlerFindEnemy Class Definition

****************************************************************************/

CLASS_DECLARATION(Behavior, CrawlerFindEnemy, NULL);

ResponseDef CrawlerFindEnemy::Responses[] =
{
   { &EV_Behavior_Args, (Response)&CrawlerFindEnemy::SetArgs },
   { NULL, NULL }
};

void CrawlerFindEnemy::SetArgs(Event *ev)
{
   anim        = ev->GetString(2);
   ceilinganim = ev->GetString(3);
}

void CrawlerFindEnemy::ShowInfo(Actor &self)
{
   Behavior::ShowInfo(self);

   if(lastceilingstate)
   {
      gi.printf("\nceilingchase:\n");
      ceilingchase.ShowInfo(self);
   }
   else
   {
      gi.printf("\nchase:\n");
      chase.ShowInfo(self);
   }

   gi.printf("\nstate: %d\n", state);
   gi.printf("nextsearch: %f\n", nextsearch);
   gi.printf("anim: %s\n", anim.c_str());
   gi.printf("ceilinganim: %s\n", ceilinganim.c_str());
}

void CrawlerFindEnemy::Begin(Actor &self)
{
   if(!anim.length())
      anim = "run";

   if(!ceilinganim.length())
      ceilinganim = "ceiling_walk";

   movegoal         = NULL;
   lastSearchNode   = NULL;
   lastceilingstate = ((Crawler *)&self)->onceiling;
   state = 0;
}

PathNode *CrawlerFindEnemy::FindClosestSightNode(Actor &self)
{
   Vector pos;
   if(self.currentEnemy)
      pos = self.currentEnemy->worldorigin;
   else
      pos = self.worldorigin;

   if(lastceilingstate)
      return NULL;

   PathNode *bestnode = NULL;
   float     bestdist = 999999999; // greater than ( 8192 * sqr(2) ) ^ 2 -- maximum squared distance
   for(int i = 0; i <= ai_maxnode; i++)
   {
      PathNode *node = AI_GetNode(i);
      if(node && ((node->occupiedTime <= level.time) || (node->entnum != self.entnum)))
      {
         // get the distance squared (faster than getting real distance)
         Vector delta(node->worldorigin - pos);
         float  dist = delta * delta;
         if((dist < bestdist) && self.CanSeeFrom(node->worldorigin, self.currentEnemy))
         {
            bestnode = node;
            bestdist = dist;
         }
      }
   }

   return bestnode;
}

qboolean CrawlerFindEnemy::Evaluate(Actor &self)
{
   if(!self.currentEnemy)
      return false;

   // check ceiling state
   if(((Crawler *)&self)->onceiling != lastceilingstate)
   {
      if(lastceilingstate)
         ceilingchase.End(self);
      else
         chase.End(self);

      lastceilingstate = ((Crawler *)&self)->onceiling;
      state = 0;
   }

   if(nextsearch < level.time)
   {
      // check if we should search for the first time
      if(!lastSearchNode)
      {
         state = 0;
      }
      else
      {
         // search less often if we're far away
         nextsearch = self.DistanceTo(self.currentEnemy) * (1.0 / 512.0);
         if(nextsearch < 1)
            nextsearch = 1;

         nextsearch += level.time;

         // don't search again if our enemy hasn't moved very far
         if(!self.currentEnemy->WithinDistance(lastSearchPos, 256))
            state = 0;
      }
   }

   switch(state)
   {
   case 0:
      // Searching for enemy
      if(lastceilingstate)
      {
         ceilingchase.Begin(self);
         lastSearchPos = self.currentEnemy->worldorigin;

         movegoal = NULL;
      }
      else
      {
         chase.Begin(self);
         lastSearchPos = self.currentEnemy->worldorigin;

         movegoal = PathManager.NearestNode(lastSearchPos, &self);
         if(!movegoal)
         {
            movegoal = PathManager.NearestNode(lastSearchPos, &self, false);
         }
      }
      lastSearchNode = movegoal;
      if(movegoal)
      {
         Path *path;
         FindEnemyPath find;
         PathNode *from;

         find.heuristic.self = &self;
         find.heuristic.setSize(self.size);
         find.heuristic.entnum = self.entnum;

         from = PathManager.NearestNode(self.worldorigin, &self);
         if((from == movegoal) && (self.DistanceTo(from->worldorigin) < 8))
         {
            movegoal = NULL;
            from = NULL;
         }

         if(from)
         {
            path = find.FindPath(from, movegoal);
            if(path)
            {
               chase.SetGoal(movegoal);
               chase.SetPath(path);
            }
            else
            {
               movegoal = NULL;
            }
         }
      }

      if(!movegoal)
      {
         if(self.CanSee(self.currentEnemy) || (!self.currentEnemy->groundentity && !self.waterlevel))
         {
            if(lastceilingstate)
               ceilingchase.SetGoalPos(self.currentEnemy->worldorigin);
            else
               chase.SetGoalPos(self.currentEnemy->worldorigin);
         }
         else
         {
            // Couldn't find enemy
            // since we can't reach em
            // clear out enemy state
            //
            self.ClearEnemies();
            return false;
         }
      }

      // Found enemy, going to it
      if(lastceilingstate)
      {
         if(ceilinganim.length() && (ceilinganim != self.animname))
            self.SetAnim(ceilinganim.c_str());
      }
      else
      {
         if(anim.length() && (anim != self.animname))
            self.SetAnim(anim.c_str());
      }

      state = 1;

   case 1:
      if(self.CanShoot(self.currentEnemy, false))
      {
         // Reached enemy
         if(lastceilingstate)
            ceilingchase.End(self);
         else
            chase.End(self);
         return false;
      }

      if(lastceilingstate)
      {
         if(!ceilingchase.Evaluate(self))
         {
            state = 0;
            nextsearch = 0;
         }
      }
      else
      {
         if(!chase.Evaluate(self))
         {
            state = 0;
            nextsearch = 0;
         }
      }
      break;
   }

   return true;
}

void CrawlerFindEnemy::End(Actor &self)
{
   if(lastceilingstate)
      ceilingchase.End(self);
   else
      chase.End(self);
}

/****************************************************************************

  CrawlerIdle Class Definition

****************************************************************************/

CLASS_DECLARATION(Behavior, CrawlerIdle, NULL);

ResponseDef CrawlerIdle::Responses[] =
{
   { &EV_Behavior_Args, (Response)&CrawlerIdle::SetArgs },
   { NULL, NULL }
};

void CrawlerIdle::SetArgs(Event *ev)
{
   anim        = ev->GetString(2);
   ceilinganim = ev->GetString(3);
}

void CrawlerIdle::ShowInfo(Actor &self)
{
   Behavior::ShowInfo(self);

   gi.printf("\nnexttwitch : %f\n", nexttwitch);
   gi.printf("anim : %s\n",         anim.c_str());
   gi.printf("ceilinganim : %s\n",  ceilinganim.c_str()); // haleyjd 20170605: fixed field
}

void CrawlerIdle::Begin(Actor &self)
{
   self.currentEnemy = NULL;
   self.seenEnemy    = false;
   nexttwitch        = level.time + 10 + G_Random(20);
   lastceilingstate  = ((Crawler *)&self)->onceiling;

   if(lastceilingstate)
   {
      if(ceilinganim.length())
         self.SetAnim(ceilinganim.c_str());
   }
   else
   {
      if(anim.length())
         self.SetAnim(anim.c_str());
   }
}

qboolean CrawlerIdle::Evaluate(Actor &self)
{
   if(self.currentEnemy)
   {
      if(self.DoAction("sightenemy"))
      {
         self.seenEnemy = true;
         self.Chatter("snd_sightenemy", 5);
      }
      else
         self.currentEnemy = NULL;

      return true;
   }

   // check ceiling state
   if(lastceilingstate != ((Crawler *)&self)->onceiling)
   {
      lastceilingstate = ((Crawler *)&self)->onceiling;

      if(lastceilingstate)
      {
         if(ceilinganim.length())
            self.SetAnim(ceilinganim.c_str());
      }
      else
      {
         if(anim.length())
            self.SetAnim(anim.c_str());
      }
   }

   if(nexttwitch < level.time)
   {
      self.chattime += 10;
      self.DoAction("twitch");
      return true;
   }
   else
   {
      self.Chatter("snd_idle", 1);
   }

   return true;
}

void CrawlerIdle::End(Actor &self)
{
}

/****************************************************************************

  CrawlerStrafeTo Class Definition

****************************************************************************/

CLASS_DECLARATION(Behavior, CrawlerStrafeTo, NULL);

ResponseDef CrawlerStrafeTo::Responses[] =
{
   { &EV_Behavior_Args, (Response)&CrawlerStrafeTo::SetArgs },
   { NULL, NULL }
};

void CrawlerStrafeTo::ShowInfo(Actor &self)
{
   Behavior::ShowInfo(self);

   gi.printf("goal: ( %f, %f, %f )\n", goal.x, goal.y, goal.z);
   gi.printf("fail: %d\n", fail);

   gi.printf("\nseek:\n");
   seek.ShowInfo(self);
}

void CrawlerStrafeTo::SetArgs(Event *ev)
{
   if(ev->IsVectorAt(2))
   {
      goal = ev->GetVector(2);
   }
   else
   {
      movegoal = AI_FindNode(ev->GetString(2));
      if(movegoal)
      {
         goal = movegoal->worldorigin;
      }
      else
      {
         Entity *ent = ev->GetEntity(2);
         if(ent)
            goal = ent->worldorigin;
      }
   }
}

void CrawlerStrafeTo::Begin(Actor &self)
{
   seek.Begin(self);

   lastceilingstate = ((Crawler *)&self)->onceiling;

   Vector delta(goal - self.worldorigin);
   float  dot = delta * self.orientation[1];

   if(lastceilingstate)
   {
      if(dot < 0)
         self.SetAnim("ceiling_step_right");
      else
         self.SetAnim("ceiling_step_left");
   }
   else
   {
      if(dot < 0)
         self.SetAnim("step_right");
      else
         self.SetAnim("step_left");
   }

   fail = 0;
}

qboolean CrawlerStrafeTo::Evaluate(Actor &self)
{
   seek.SetMaxSpeed(self.movespeed);
   seek.SetPosition(self.worldorigin);
   seek.SetDir(self.movedir);
   seek.SetTargetPosition(goal);

   if(!seek.Evaluate(self))
      return false;

   // stop if ceiling state changes
   if(lastceilingstate != ((Crawler *)&self)->onceiling)
   {
      lastceilingstate = ((Crawler *)&self)->onceiling;
      return false;
   }

   self.Accelerate(seek.steeringforce);

   // prevent him from trying to strafing forever if he's stuck
   if(self.lastmove != STEPMOVE_OK)
   {
      if(fail)
         return false;

      fail++;
   }
   else
      fail = 0;

   return true;
}

void CrawlerStrafeTo::End(Actor &self)
{
   seek.End(self);
   if(((Crawler *)&self)->onceiling)
      self.SetAnim("ceiling_idle");
   else
      self.SetAnim("idle");
}

/****************************************************************************

  CeilingStrafeAttack Class Definition

****************************************************************************/

CLASS_DECLARATION(Behavior, CeilingStrafeAttack, NULL);

ResponseDef CeilingStrafeAttack::Responses[] =
{
   { NULL, NULL }
};

void CeilingStrafeAttack::ShowInfo(Actor &self)
{
   Behavior::ShowInfo(self);

   gi.printf("\nturn:\n");
   turn.ShowInfo(self);

   gi.printf("\nstate: %d\n", state);
}

void CeilingStrafeAttack::Begin(Actor &self)
{
   state = 0;
}

qboolean CeilingStrafeAttack::Evaluate(Actor &self)
{
   int num;
   Vector delta;
   Vector left;
   Vector pos;

   if(!self.currentEnemy)
      return false;

   // stop if not on the ceiling
   if(!((Crawler *)&self)->onceiling)
      return false;

   switch(state)
   {
   case 0:
      delta = self.currentEnemy->worldorigin - self.worldorigin;
      turn.SetDirection(delta.toYaw());
      turn.Begin(self);
      state = 1;
      break;

   case 1:
      if(turn.Evaluate(self))
         return true;

      turn.End(self);
      state = 2;

   case 2:
      delta = self.currentEnemy->worldorigin - self.worldorigin;
      left.x = -delta.y;
      left.y = delta.x;
      left.normalize();

      if(G_Random(10) < 5)
      {
         num = gi.Anim_Random(self.edict->s.modelindex, "ceiling_step_left");
         if(num != -1)
         {
            gi.Anim_Delta(self.edict->s.modelindex, num, delta.vec3());
            delta *= self.edict->s.scale;
            pos = self.worldorigin + left * delta.length();
            if(((Crawler *)&self)->CanMoveTo(pos) && self.CanShootFrom(pos, self.currentEnemy, false))
            {
               self.SetAnim("ceiling_step_left", EV_Actor_FinishedBehavior);
               state = 3;
               return true;
            }
         }

         num = gi.Anim_Random(self.edict->s.modelindex, "ceiling_step_right");
         if(num != -1)
         {
            gi.Anim_Delta(self.edict->s.modelindex, num, delta.vec3());
            delta *= self.edict->s.scale;
            pos = self.worldorigin - left * delta.length();
            if(((Crawler *)&self)->CanMoveTo(pos) && self.CanShootFrom(pos, self.currentEnemy, false))
            {
               self.SetAnim("ceiling_step_right", EV_Actor_FinishedBehavior);
               state = 3;
               return true;
            }
         }
      }
      else
      {
         num = gi.Anim_Random(self.edict->s.modelindex, "ceiling_step_right");
         if(num != -1)
         {
            gi.Anim_Delta(self.edict->s.modelindex, num, delta.vec3());
            delta *= self.edict->s.scale;
            pos = self.worldorigin - left * delta.length();
            if(((Crawler *)&self)->CanMoveTo(pos) && self.CanShootFrom(pos, self.currentEnemy, false))
            {
               self.SetAnim("ceiling_step_right", EV_Actor_FinishedBehavior);
               state = 3;
               return true;
            }
         }

         num = gi.Anim_Random(self.edict->s.modelindex, "ceiling_step_left");
         if(num != -1)
         {
            gi.Anim_Delta(self.edict->s.modelindex, num, delta.vec3());
            delta *= self.edict->s.scale;
            pos = self.worldorigin + left * delta.length();
            if(((Crawler *)&self)->CanMoveTo(pos) && self.CanShootFrom(pos, self.currentEnemy, false))
            {
               self.SetAnim("ceiling_step_left", EV_Actor_FinishedBehavior);
               state = 3;
               return true;
            }
         }

      }

      return false;
      break;
   }

   return true;
}

void CeilingStrafeAttack::End(Actor &self)
{
   turn.End(self);
   if(((Crawler *)&self)->onceiling)
      self.SetAnim("ceiling_idle");
   else
      self.SetAnim("idle");
}

// EOF

