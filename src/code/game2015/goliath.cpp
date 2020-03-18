/*
================================================================
GOLIATH
================================================================

Copyright (C) 2020 by Night Dive Studios, Inc.
All rights reserved.

See the license.txt file for conditions and terms of use for this code.
*/

#include "goliath.h"
#include "object.h"
#include "player.h"

#define GOLIATH_MAX_OBJECTS 20

//===============================================================
// GOLIATH
//===============================================================

Event EV_Goliath_InitThrowTime("throwtime");
Event EV_Goliath_MeleeForce("meleeforce");
Event EV_Goliath_SetLeftHand("setlefthand");
Event EV_Goliath_SetRightHand("setrighthand");
Event EV_Goliath_SetBothHands("setbothhands");
Event EV_Goliath_SetAttackStage("attackstage");

Event EV_Goliath_IfThrowHigh("ifthrowhigh");
Event EV_Goliath_IfThrowLow("ifthrowlow");

CLASS_DECLARATION(Actor, Goliath, "monster_goliath");

ResponseDef Goliath::Responses[] =
{
   { &EV_Goliath_InitThrowTime,  (Response)&Goliath::InitThrowTime },
   { &EV_Goliath_MeleeForce,     (Response)&Goliath::SetMeleeForce },
   { &EV_Goliath_SetLeftHand,    (Response)&Goliath::SetLeftHand },
   { &EV_Goliath_SetRightHand,   (Response)&Goliath::SetRightHand },
   { &EV_Goliath_SetBothHands,   (Response)&Goliath::SetBothHands },
   { &EV_Goliath_SetAttackStage, (Response)&Goliath::SetAttackStage },

   { &EV_Goliath_IfThrowHigh,    (Response)&Goliath::IfThrowHighEvent },
   { &EV_Goliath_IfThrowLow,     (Response)&Goliath::IfThrowLowEvent },

   { NULL, NULL}
};

Goliath::Goliath() : Actor()
{
   randomthrowtime = 0;
   rubbletime      = 0;
   throwhigh       = false;
   leftdamage      = 0;
   leftforce       = 0;
   rightdamage     = 0;
   rightforce      = 0;
   hitsentient     = false;

   if(attackstage < 1 || attackstage > 2)
      attackstage = 1;

   if(actorthread)
      actorthread->Vars()->SetVariable("attackstage", attackstage);

   flags |= FL_POSTTHINK;
}

void Goliath::Prethink(void)
{
   trace_t   trace;
   Vector    tmpvec;
   qboolean  nowthrowing = false;
   Entity   *ent;

   if(attackstage == 2)
   {
      if(randomthrowtime && (randomthrowtime < level.time))
      {
         // we want to throw something...
         Entity *bestent;
         float bestdist;
         float dist;
         float distance;
         Vector delta;

         distance = 300;
         bestent = NULL;
         bestdist = distance * distance;

         ent = NULL;
         while((ent = findradius(ent, worldorigin.vec3(), distance)))
         {
            if(ent->isSubclassOf<ThrowObject>())
            {
               delta = centroid - ent->centroid;
               dist = delta * delta;
               if(dist <= bestdist)
               {
                  bestent = ent;
                  bestdist = dist;
               }
            }
         }

         if(bestent)
         {
            bestent = CheckObjectsAbove(bestent);
            SetVariable("other", bestent);
            if(DoAction("throwthing", false))
            {
               randomthrowtime = 0;
               nowthrowing     = true;

               // easy his rumbling tendancies a bit
               if(rubbletime)
                  rubbletime += 4 + G_Random(2) - skill->value;
            }
         }
      }

      // rumble timmer for when he goes to long wanting to
      // throw something without anything around
      if(randomthrowtime && !nowthrowing)
      {
         if(!rubbletime)
         {
            rubbletime = level.time + 5 + G_Random(2) - (skill->value * 1.5);
         }
         else if(rubbletime < level.time)
         {
            ScriptVariable *var;
            int numobjects;

            // check max objects at one time limit
            var = levelVars.GetVariable("goliathobject_count");
            if(var)
               numobjects = var->intValue();
            else
               numobjects = 0;

            if(numobjects < GOLIATH_MAX_OBJECTS)
            {
               // time to make more stuff to throw >)
               if(DoAction("rumbleattack", false))
               {
                  randomthrowtime = 0;
                  nowthrowing     = true;

                  // ah, feel better now
                  rubbletime = 0;
               }
            }
            else
            {
               // ease off his rubble making tendacies for a bit
               rubbletime = level.time + 1 + G_Random(2);
            }
         }
      }

   }
   // check for having a hissy hit and going into stage two
   else
   {
      if(health < (max_health * 0.5))
      {
         if(DoAction("pissedoff", false))
         {
            // yup, he's pissed
            attackstage = 2;
            nowthrowing = true;

            // make sure rubble attack timmer is inited
            rubbletime = 0;
         }
      }

      // stage one throwing occures less often then stage two throwing
      if(!nowthrowing && randomthrowtime && ((randomthrowtime + 1) < level.time))
      {
         // we want to throw something...
         Entity *bestent;
         float bestdist;
         float dist;
         float distance;
         Vector delta;

         distance = 320;
         bestent  = NULL;
         bestdist = distance * distance;

         ent = NULL;
         while((ent = findradius(ent, worldorigin.vec3(), distance)))
         {
            if(ent->isSubclassOf<ThrowObject>())
            {
               delta = centroid - ent->centroid;
               dist = delta * delta;
               if(dist <= bestdist)
               {
                  bestent = ent;
                  bestdist = dist;
               }
            }
         }

         if(bestent)
         {
            bestent = CheckObjectsAbove(bestent);
            if(bestent)
            {
               SetVariable("other", bestent);
               if(DoAction("throwthing", false))
               {
                  randomthrowtime = 0;
                  nowthrowing     = true;
               }
            }
         }
      }
   }

   if(!nowthrowing && randomthrowtime)
   {
      // if there's ever a throwable thing in our
      // way, then get it out of our way.
      tmpvec = worldorigin + move;
      trace = G_Trace(worldorigin, mins, maxs, tmpvec, this, edict->clipmask, "Goliath::Prethink");
      if((trace.fraction != 1) && (trace.ent))
      {
         // check if we ran into something we can throw
         ent = trace.ent->entity;
         if(ent->isSubclassOf<ThrowObject>())
         {
            ent = CheckObjectsAbove(ent);
            if(ent)
            {
               SetVariable("other", ent);
               // if in attackstage 2, there's a chance we'll just smash it
               if((attackstage == 2) && (G_Random() < 0.6))
                  DoAction("destroyobstruction", false);
               else
                  DoAction("throwobstruction", false);
            }
         }
      }
   }

   //
   // call our superclass
   //
   Actor::Prethink();
}

#define HAND_SIZE 40

// here's where the hand collision detection is done
void Goliath::Postthink(void)
{
   vec3_t   trans[3];
   vec3_t   orient;
   int      groupindex;
   int      tri_num;
   Vector   pos, forward;
   Vector   offset;
   Vector   tmpvec;
   Vector   handmins, handmaxs;
   Vector   min, max;
   float    handdiff;
   int      num;
   edict_t *touch[MAX_EDICTS];

   handmins = { -HAND_SIZE, -HAND_SIZE, -HAND_SIZE };
   handmaxs = { HAND_SIZE, HAND_SIZE, HAND_SIZE };

   if(rightdamage || rightforce)
   {
      if(!gi.GetBoneInfo(edict->s.modelindex, "gun", &groupindex, &tri_num, orient))
      {
         // couldn't get proper bone info
         gi.dprintf("Couldn't find Goliath's gun bone\n");
         return;
      }

      AngleVectors(orient, forward.vec3(), NULL, NULL);

      // get hand position
      offset = vec_zero;
      gi.GetBoneTransform(edict->s.modelindex, groupindex, tri_num, orient, edict->s.anim,
                          edict->s.frame, edict->s.scale, trans, offset.vec3());
      MatrixTransformVector(offset.vec3(), orientation, pos.vec3());
      pos += worldorigin;

      // check at an interpolated position if hand moved enough from last frame
      if(lastrightpos != vec_zero)
      {
         tmpvec = lastrightpos - pos;
         handdiff = tmpvec.length();
         if(handdiff > (HAND_SIZE * 3))
         {
            tmpvec = (lastrightpos + pos)*0.5;

            min = tmpvec + handmins;
            max = tmpvec + handmaxs;

            num = gi.BoxEdicts(min.vec3(), max.vec3(), touch, MAX_EDICTS, AREA_TRIGGERS);
            if(num)
               DoHandHits(tmpvec, forward, rightdamage, rightforce, touch, num);

            num = gi.BoxEdicts(min.vec3(), max.vec3(), touch, MAX_EDICTS, AREA_SOLID);
            if(num)
               DoHandHits(tmpvec, forward, rightdamage, rightforce, touch, num);
         }
      }

      // check current hand position
      min = pos + handmins;
      max = pos + handmaxs;

      num = gi.BoxEdicts(min.vec3(), max.vec3(), touch, MAX_EDICTS, AREA_TRIGGERS);
      if(num)
         DoHandHits(pos, forward, rightdamage, rightforce, touch, num);

      num = gi.BoxEdicts(min.vec3(), max.vec3(), touch, MAX_EDICTS, AREA_SOLID);
      if(num)
         DoHandHits(pos, forward, rightdamage, rightforce, touch, num);

      // save current hand position
      lastrightpos = pos;
   }
   else
   {
      // clear out last hand position
      lastrightpos = vec_zero;
   }


   if(leftdamage || leftforce)
   {
      if(!gi.GetBoneInfo(edict->s.modelindex, "leftgun", &groupindex, &tri_num, orient))
      {
         // couldn't get proper bone info
         gi.dprintf("Couldn't find Goliath's leftgun bone\n");
         return;
      }

      AngleVectors(orient, forward.vec3(), NULL, NULL);

      // get hand position
      offset = vec_zero;
      gi.GetBoneTransform(edict->s.modelindex, groupindex, tri_num, orient, edict->s.anim,
                          edict->s.frame, edict->s.scale, trans, offset.vec3());
      MatrixTransformVector(offset.vec3(), orientation, pos.vec3());
      pos += worldorigin;

      // check at an interpolated position if hand moved enough from last frame
      if(lastleftpos != vec_zero)
      {
         tmpvec = lastleftpos - pos;
         handdiff = tmpvec.length();
         if(handdiff > (HAND_SIZE * 3))
         {
            tmpvec = (lastleftpos + pos)*0.5;

            min = tmpvec + handmins;
            max = tmpvec + handmaxs;

            num = gi.BoxEdicts(min.vec3(), max.vec3(), touch, MAX_EDICTS, AREA_TRIGGERS);
            if(num)
               DoHandHits(tmpvec, forward, leftdamage, leftforce, touch, num);

            num = gi.BoxEdicts(min.vec3(), max.vec3(), touch, MAX_EDICTS, AREA_SOLID);
            if(num)
               DoHandHits(tmpvec, forward, leftdamage, leftforce, touch, num);
         }
      }

      // check current hand position
      min = pos + handmins;
      max = pos + handmaxs;

      num = gi.BoxEdicts(min.vec3(), max.vec3(), touch, MAX_EDICTS, AREA_TRIGGERS);
      if(num)
         DoHandHits(pos, forward, leftdamage, leftforce, touch, num);

      num = gi.BoxEdicts(min.vec3(), max.vec3(), touch, MAX_EDICTS, AREA_SOLID);
      if(num)
         DoHandHits(pos, forward, leftdamage, leftforce, touch, num);

      // save current hand position
      lastleftpos = pos;
   }
   else
   {
      // clear out last hand position
      lastleftpos = vec_zero;
   }
}

void Goliath::InitThrowTime(Event *ev)
{
   float mintime, maxtime, timediff;

   // if fewer than 2 numbers are given, clear timmer
   if(ev->NumArgs() < 2)
   {
      randomthrowtime = 0;
      return;
   }

   mintime = ev->GetFloat(1);
   maxtime = ev->GetFloat(2);
   timediff = maxtime - mintime;

   randomthrowtime = level.time + mintime + G_Random(timediff);
   randomthrowtime = level.time + G_Random(timediff);
}

Entity *Goliath::CheckObjectsAbove(Entity *ent)
{
   Vector  tmpvec, tmpvec2;
   trace_t trace;
   int     mask, i;
   Entity *newent;
   float   tmpflt;

   // make sure we're passed a good thing
   if(!ent)
      return NULL;

   if((ent->health <= 0) || (!ent->isSubclassOf<ThrowObject>()))
      return NULL;

   // check for a better thing on the thing

   mask = MASK_PROJECTILE;

   // first try a full size trace
   tmpvec = ent->worldorigin + Vector(0, 0, 16);
   trace = G_Trace(ent->worldorigin, ent->mins, ent->maxs, tmpvec, ent, mask, "Goliath::CheckObjectsAbove");

   // check for something good
   if(trace.ent)
   {
      newent = trace.ent->entity;

      // check for a stackable object that's above us
      if((newent->spawnflags & 8) && (newent->worldorigin.z > ent->worldorigin.z))
      {
         throwhigh = true;
         return newent;
      }
   }

   // try several different spots
   for(i = 0; i < 13; i++)
   {
      tmpvec = ent->worldorigin;
      switch(i)
      {
         // corners
      case 1:
         tmpvec.x += ent->mins.x + 8;
         tmpvec.y += ent->mins.y + 8;
         break;
      case 2:
         tmpvec.x += ent->mins.x + 8;
         tmpvec.y += ent->maxs.y - 8;
         break;
      case 3:
         tmpvec.x += ent->maxs.x - 8;
         tmpvec.y += ent->mins.y + 8;
         break;
      case 4:
         tmpvec.x += ent->maxs.x - 8;
         tmpvec.y += ent->maxs.y - 8;
         break;
         // edge middles
      case 5:
         tmpvec.x += ent->mins.x + 8;
         break;
      case 6:
         tmpvec.x += ent->maxs.x - 8;
         break;
      case 7:
         tmpvec.y += ent->mins.y + 8;
         break;
      case 8:
         tmpvec.y += ent->maxs.y - 8;
         break;
      }

      tmpvec2 = tmpvec + Vector(0, 0, 16);
      trace = G_Trace(tmpvec, Vector(-4, -4, 0), Vector(4, 4, ent->maxs.z - 1), tmpvec2, ent, mask, "Goliath::CheckObjectsAbove");

      // check for something good
      if(trace.ent)
      {
         newent = trace.ent->entity;

         // check for a stackable object that's above us
         if(newent->worldorigin.z > ent->worldorigin.z)
         {
            throwhigh = true;
            return newent;
         }
      }
   }

   // couldn't find anything, so stick with that we've got
   // check our object's pichup height
   tmpflt = ent->worldorigin.z - worldorigin.z;
   if(tmpflt > 32)
      throwhigh = true;
   else
      throwhigh = false;

   return ent;
}

//===============================================================
// melee attack stuff

void Goliath::SetMeleeForce(Event *ev)
{
   melee_force = ev->GetFloat(1);
   melee_force *= edict->s.scale;
}

void Goliath::SetLeftHand(Event *ev)
{
   leftdamage = ev->GetFloat(1);
   leftforce = ev->GetFloat(2);
}

void Goliath::SetRightHand(Event *ev)
{
   rightdamage = ev->GetFloat(1);
   rightforce = ev->GetFloat(2);
}

void Goliath::SetBothHands(Event *ev)
{
   rightdamage = ev->GetFloat(1);
   rightforce = ev->GetFloat(2);

   leftdamage = rightdamage;
   leftforce = rightforce;
}

void Goliath::DoHandHits(Vector pos, Vector dir, float handdamage, float handforce, edict_t *touch[MAX_EDICTS], int num)
{
   edict_t *hit;
   Entity  *ent;
   float    damage;

   // horizontalize the direction
   dir.z = 0;
   dir.normalize();

   for(int i = 0; i < num; i++)
   {
      hit = touch[i];
      if(!hit->inuse)
         continue;

      assert(hit->entity);

      ent = hit->entity;

      if(ent == this)
         continue;

      if(ent->getSolidType() == SOLID_BSP)
         continue;

      // so goliaths don't hurt each other
      if(ent->isSubclassOf<Goliath>())
         continue;

      if(ent->isSubclassOf<Sentient>() && hitsentient)
         continue;

      if(ent->takedamage)
      {
         if(ent->isSubclassOf<Sentient>())
         {
            damage = melee_damage * handdamage;
            hitsentient = true;
         }
         else
         {
            damage = ent->max_health * 10;

            // make a hit sound
            ent->RandomGlobalSound("impact_smallexplosion", 0.5, CHAN_BODY);
         }

         ent->Damage(this, this, (int)damage, pos,
                     vec_zero, vec_zero, 0, DAMAGE_NO_KNOCKBACK, MOD_MUTANTHANDS, -1, -1, 1.0f);

         ent->velocity -= dir * (melee_force * handforce);
         if(ent->velocity.z < 50)
            ent->velocity.z = 50;
         ent->groundentity = NULL;
      }
   }
}

//===============================================================

void Goliath::SetAttackStage(Event *ev)
{
   attackstage = ev->GetInteger(1);

   if(attackstage < 1)
      attackstage = 1;
   else if(attackstage > 2)
      attackstage = 2;

   if(actorthread)
      actorthread->Vars()->SetVariable("attackstage", attackstage);
}

void Goliath::IfThrowHighEvent(Event *ev)
{
   ScriptThread *thread = ev->GetThread();
   assert(thread);
   if(!thread)
      return;

   if(throwhigh)
      thread->ProcessCommandFromEvent(ev, 1);
}

void Goliath::IfThrowLowEvent(Event *ev)
{
   ScriptThread *thread = ev->GetThread();
   assert(thread);
   if(!thread)
      return;

   if(!throwhigh)
      thread->ProcessCommandFromEvent(ev, 1);
}

//===============================================================

/****************************************************************************

  RumbleAttack Class Definition

  Goliath does an attack that causes the player to get jittered around
  and also causes rubble to fall from the ceiling

****************************************************************************/

CLASS_DECLARATION(Behavior, RumbleAttack, NULL);

Event EV_RumbleAttack_DoRumble("dorumble");

ResponseDef RumbleAttack::Responses[] =
{
   { &EV_Behavior_Args,         (Response)&RumbleAttack::SetArgs  },
   { &EV_Behavior_AnimDone,     (Response)&RumbleAttack::AnimDone },
   { &EV_RumbleAttack_DoRumble, (Response)&RumbleAttack::DoRumble },
   { NULL, NULL }
};

void RumbleAttack::ShowInfo(Actor &self)
{
   Behavior::ShowInfo(self);

   gi.printf("\nanim: %s\n", anim.c_str());
   gi.printf("animdone: %d\n", animdone);
   gi.printf("\nrubbletarget: %s\n", rubbletarget.c_str());
   gi.printf("rubblenums: %d\n", rubblenums);
}

void RumbleAttack::Begin(Actor &self)
{
   if(!anim.length())
      anim = "rumble";

   animdone = false;

   // start off the rumbling animation
   if(self.HasAnim(anim.c_str()))
   {
      self.SetAnim(anim.c_str(), EV_Actor_NotifyBehavior);
   }
   else
   {
      // no animation, so don't do it
      self.SetAnim("idle");
      animdone = true;
   }
}

void RumbleAttack::SetArgs(Event *ev)
{
   anim = ev->GetString(2);
   rubbletarget = ev->GetString(3);
}

void RumbleAttack::AnimDone(Event *ev)
{
   animdone = true;
}

qboolean RumbleAttack::Evaluate(Actor &self)
{
   // when the animation is finished, the behaivior is finished
   if(animdone)
      return false;

   return true;
}

void RumbleAttack::End(Actor &self)
{
   self.SetAnim("idle");
}

void RumbleAttack::DoRumble(Event *ev)
{
   str targ(rubbletarget);

   if(targ.length())
   {
      int num = 0;
      do
      {
         num = G_FindTarget(num, targ.c_str());
         if(!num)
            break;

         Entity *ent = G_GetEntity(num);

         auto event = new Event(EV_Activate);
         event->AddEntity(ev->GetEntity(1));
         ent->PostEvent(event, 0);
      }
      while(1);
   }

   // jitter the player around
   for(int i = 1; i <= maxclients->value; i++)
   {
      if(!g_edicts[i].inuse || !g_edicts[i].entity)
         continue;

      Player *player = (Player *)(g_edicts[i].entity);
      player->SetAngleJitter(12, 5, 1);
      player->SetOffsetJitter(10, 2.5, 1);
   }
}

/****************************************************************************

  GoliathMelee Class Definition

****************************************************************************/

CLASS_DECLARATION(Behavior, GoliathMelee, NULL);

ResponseDef GoliathMelee::Responses[] =
{
   { &EV_Behavior_AnimDone, (Response)&GoliathMelee::AnimDone },
   { NULL, NULL }
};

GoliathMelee::GoliathMelee() : Behavior()
{
}

void GoliathMelee::ShowInfo(Actor &self)
{
   Behavior::ShowInfo(self);

   gi.printf("\naim:\n");
   turnto.ShowInfo(self);

   gi.printf("animdone: %d\n", animdone);
}

void GoliathMelee::Begin(Actor &self)
{
   mode = 0;
   animdone = false;

   // clear Goliaths hitsentient value
   ((Goliath *)&self)->hitsentient = false;
}

void GoliathMelee::SetArgs(Event *ev)
{
}

void GoliathMelee::AnimDone(Event *ev)
{
   animdone = true;
}

qboolean GoliathMelee::Evaluate(Actor &self)
{
   if(animdone)
      return false;

   if(!self.has_melee || !self.currentEnemy)
      return false;

   if(mode == 0)
   {
      float r;
      Vector delta;

      delta = self.currentEnemy->centroid - self.centroid;
      r = delta.length();
      if(r > self.melee_range)
         return false;

      r = delta.toYaw();
      turnto.SetDirection(r);
      turnto.SetTolerance(10);

      if(turnto.Evaluate(self))
         return true;

      // melee
      self.SetAnim("melee", EV_Actor_NotifyBehavior);
      self.Chatter("snd_attacktaunt", 5);

      mode = 1;
   }

   return true;
}

void GoliathMelee::End(Actor &self)
{
   turnto.End(self);
   self.SetAnim("idle");
}

/****************************************************************************

  Goliath Pickup Behavior Class Definition

****************************************************************************/

CLASS_DECLARATION(Behavior, GoliathPickupAndThrow, NULL);

extern Event EV_PickupAndThrow_Pickup;
extern Event EV_PickupAndThrow_Throw;

ResponseDef GoliathPickupAndThrow::Responses[] =
{
   { &EV_Behavior_Args,         (Response)&GoliathPickupAndThrow::SetArgs  },
   { &EV_Behavior_AnimDone,     (Response)&GoliathPickupAndThrow::AnimDone },
   { &EV_PickupAndThrow_Pickup, (Response)&GoliathPickupAndThrow::Pickup   },
   { &EV_PickupAndThrow_Throw,  (Response)&GoliathPickupAndThrow::Throw    },
   { NULL, NULL }
};

GoliathPickupAndThrow::GoliathPickupAndThrow() : Behavior()
{
}

void GoliathPickupAndThrow::ShowInfo(Actor &self)
{
   Behavior::ShowInfo(self);

   gi.printf("\naim:\n");
   aim.ShowInfo(self);

   gi.printf("\nmode: %d\n",   mode);
   gi.printf("\nanim: %s\n",   anim.c_str());
   gi.printf("animdone: %d\n", animdone);

   if(pickup_target)
      gi.printf("\npickup_target: #%d '%s'\n", pickup_target->entnum, pickup_target->targetname.c_str());
   else
      gi.printf("\npickup_target: NULL\n");
}

void GoliathPickupAndThrow::Begin(Actor &self)
{
   mode     = 0;
   animdone = false;

   if(!anim.length())
      anim = "pickup";

   // turn towards the object
   if(pickup_target)
   {
      turnto.SetTarget(pickup_target);
      turnto.SetTolerance(20);
      turnto.Begin(self);
   }
}

void GoliathPickupAndThrow::SetArgs(Event *ev)
{
   pickup_target = ev->GetEntity(2);

   if(ev->NumArgs() > 2)
      anim = ev->GetString(3);
}

void GoliathPickupAndThrow::AnimDone(Event *ev)
{
   animdone = true;
}

void GoliathPickupAndThrow::Pickup(Event *ev)
{
   Entity *ent = ev->GetEntity(1);
   if(pickup_target)
   {
      auto e = new Event(EV_ThrowObject_Pickup);
      e->AddEntity(ent);
      e->AddString(ev->GetString(2));
      pickup_target->ProcessEvent(e);
      turnto.End(*((Actor *)ent));
   }
}

void GoliathPickupAndThrow::Throw(Event *ev)
{
   Actor *act = (Actor *)ev->GetEntity(1);
   if(pickup_target)
   {
      if(!act->currentEnemy)
         return;

      auto e = new Event(EV_ThrowObject_Throw);
      e->AddEntity(act);
      e->AddFloat(800);
      e->AddEntity(act->currentEnemy);
      e->AddFloat(1);
      pickup_target->ProcessEvent(e);
   }
}

qboolean GoliathPickupAndThrow::Evaluate(Actor &self)
{
   if(!self.currentEnemy || !pickup_target)
      return false;

   switch(mode)
   {
   case 0:
      if(self.HasAnim(anim.c_str()))
      {
         animdone = false;
         self.SetAnim(anim.c_str(), EV_Actor_NotifyBehavior);
         mode = 1;
      }
      else
      {
         // skip the pickup animation
         auto ev = new Event(EV_PickupAndThrow_Pickup);
         ev->AddEntity(&self);
         ev->AddString("gun");
         ProcessEvent(ev);
         animdone = true;
      }

   case 1:
      if(!animdone)
         break;
      aim.Begin(self);
      mode = 2;
      if(self.HasAnim("throw_aim"))
      {
         self.SetAnim("throw_aim", EV_Actor_NotifyBehavior);
      }
      else
      {
         self.SetAnim("idle");
      }

   case 2:
      // aim towards our target
      aim.SetTarget(self.currentEnemy);
      if(aim.Evaluate(self))
      {
         break;
      }
      mode = 3;

   case 3:
      // Throwing
      mode = 4;
      if(self.HasAnim("throw"))
      {
         animdone = false;
         self.SetAnim("throw", EV_Actor_NotifyBehavior);
         mode = 4;
      }
      else
      {
         // skip the pickup animation
         auto ev = new Event(EV_PickupAndThrow_Throw);
         ev->AddEntity(&self);
         ProcessEvent(ev);
         animdone = true;
      }
      break;

   case 4:
      if(!animdone)
         break;
      return false;
   }

   return true;
}

void GoliathPickupAndThrow::End(Actor &self)
{
   aim.End(self);
   turnto.End(self);
   self.SetAnim("idle");
}

// EOF

