//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/explosion.cpp                    $
// $Revision:: 51                                                             $
//   $Author:: Aldie                                                          $
//     $Date:: 3/30/99 4:51p                                                  $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Standard explosion object that is spawned by other entites and not map designers.
// Explosion is used by many of the weapons for the blast effect, but is also used
// by the Exploder and MultiExploder triggers.  These triggers create one or more
// explosions each time they are activated.
// 

#include "g_local.h"
#include "entity.h"
#include "trigger.h"
#include "explosion.h"
#include "specialfx.h"
#include "player.h"
//###
#include "jitter.h"     // added for view jitter
#include "hoverbike.h"
#include "ctf.h"
//##

#define RANDOM_TIME (1<<1)
#define RANDOM_SCALE (1<<2)
#define BIG_EXPLOSION (1<<3)

void FlashPlayers(Vector org, float r, float g, float b, float a, float rad)
{
   Event    *ev1;
   trace_t  trace;
   Vector   delta;
   float    length;
   Player   *player;
   edict_t  *edict;
   int      i;

   for(i = 0; i < maxclients->value; i++)
   {
      edict = g_edicts + 1 + i;
      if(!edict->inuse || !edict->client || !edict->entity || !edict->entity->isSubclassOf<Player>() ||
         !edict->entity->WithinDistance(org, rad))
      {
         continue;
      }

      player = (Player *)edict->entity;

      trace = G_Trace(org, vec_zero, vec_zero, player->worldorigin + player->eyeposition, player, MASK_OPAQUE, "FlashPlayers");
      if(trace.fraction != 1.0)
      {
         continue;
      }

      delta = org - trace.endpos;
      length = delta.length();

      a = a * (1 - length / rad);

      ev1 = new Event(EV_Player_SetFlashColor);
      ev1->AddFloat(r);
      ev1->AddFloat(g);
      ev1->AddFloat(b);
      ev1->AddFloat(a);
      player->ProcessEvent(ev1);
   }
}

//###
void FlashGogglesPlayers(Vector org, float rad)
{
   Event    *ev1;
   trace_t  trace;
   Vector   delta;
   float    length;
   Player   *player;
   edict_t  *edict;
   float    a;

   for(int i = 0; i < maxclients->value; i++)
   {
      edict = g_edicts + 1 + i;
      if(!edict->inuse || !edict->client || !edict->entity || !edict->entity->isSubclassOf<Player>() ||
         !edict->entity->WithinDistance(org, rad))
      {
         continue;
      }

      player = (Player *)edict->entity;

      // only flash people wearing goggles
      if(!player->nightvision)
      {
         continue;
      }

      trace = G_Trace(org, vec_zero, vec_zero, player->worldorigin + player->eyeposition, player, MASK_OPAQUE, "FlashPlayers");
      if(trace.fraction != 1.0)
      {
         continue;
      }

      delta = org - trace.endpos;
      length = delta.length();

      a = (1 - length / rad) + 0.1;
      if(a > 1.0)
         a = 1.0;

      ev1 = new Event(EV_Player_SetFlashColor);
      ev1->AddFloat(1.0);
      ev1->AddFloat(1.0);
      ev1->AddFloat(1.0);
      ev1->AddFloat(a);
      player->ProcessEvent(ev1);
   }
}
//###

void RadiusDamage(Entity *inflictorent, Entity *attackerent, int damage, Entity *ignoreent, int mod, float falloff_rate /*###*/)
{
   float		points;
   Entity	*ent;
   Vector	org;
   Vector	v;
   float		rad;

   //###
   //	rad = ( float )( damage + 60 );
   if(falloff_rate == 0.5)
      rad = (float)(damage + 60);
   else if(falloff_rate == 0)
      rad = 999999;
   else
      rad = (((float)damage) / falloff_rate)*0.8f;

   // flash player's with goggles
   FlashGogglesPlayers(inflictorent->worldorigin, rad + 256);
   //###


   ent = findradius(NULL, inflictorent->worldorigin.vec3(), rad);
   while(ent)
   {
      //### don't do radius damage to a hoverbike's extra bounding boxes
      //if ( ( ent != ignoreent ) && ( ent->takedamage ) )
      if((ent != ignoreent) && (ent->takedamage) && (!ent->isSubclassOf<HoverbikeBox>()))
      //###
      {
         org = ent->centroid;
         v = org - inflictorent->worldorigin;
         //### use passed in falloff rate
         //points = v.length() * 0.5f;
         points = v.length()*falloff_rate;
         //###

         if(points < 0)
         {
            points = 0;
         }
         points = damage - points;
         if(ent == attackerent)
         {
            points *= 0.5;
         }

         if(points > 0)
         {
            if(inflictorent->CanDamage(ent))
            {
               //###
               //ent->Damage(inflictorent, attackerent, points,
               //            org, v, vec_zero, points, 
               //            DAMAGE_RADIUS, mod,
               //            -1, -1, 1.0f );
               if(mod == MOD_NUKEEXPLOSION)
               {
                  ent->Damage(inflictorent, attackerent, points,
                              org, v, vec_zero, points*0.1, 
                              DAMAGE_RADIUS, mod,
                              -1, -1, 1.0f );
               }
               else
               {
                  ent->Damage(inflictorent, attackerent, points,
                              org, v, vec_zero, points, 
                              //DAMAGE_RADIUS, MOD_ROCKETSPLASH,
                              DAMAGE_RADIUS, mod,
                              -1, -1, 1.0f );
               }
               //###
            }
         }
      }
      ent = findradius(ent, inflictorent->worldorigin.vec3(), rad);
   }
}

void CreateExplosion(
   Vector pos, float damage, float scale, qboolean bigexplosion, Entity *inflictor, Entity *attacker, Entity *ignore, 
   /*###*/ int mod, float falloff_rate, /*###*/ float volume, float attenuation, float r, float g, float b, 
   float light_radius, float life, float decay)
{
   assert(inflictor);

   if(!inflictor)
   {
      return;
   }

   if(!attacker)
   {
      attacker = world;
   }

   if(volume > 4.0f)
      volume = 4.0f;

   //### add ability to disable the sound with a volume of 0 or less
   if(volume > 0)
   {
      if(damage < 120)
      {
         inflictor->RandomPositionedSound(pos, "impact_smallexplosion", volume, CHAN_AUTO, attenuation);
      }
      else
      {
         inflictor->RandomPositionedSound(pos, "impact_bigexplosion", volume, CHAN_AUTO, attenuation);
      }
   }
   //###

   //###
   //RadiusDamage( inflictor, attacker, damage, ignore, MOD_ROCKETSPLASH );
   RadiusDamage(inflictor, attacker, damage, ignore, mod);
   //###
   if(inflictor)
      inflictor->ProcessEvent(EV_WeaponSound);

   //### added ability to disable the visual effect with a scale of 0 or less
   // also added view jitter
   if(scale > 0)
   {
      RadiusJitter *jitter;
      jitter = new RadiusJitter();

      //if( bigexplosion )
      //   SpawnScaledExplosion( pos, scale );
      //else
      //   TempModel( NULL, pos, { 0, 0, 0 }, "sprites/explode.spr", 0, scale, 1.0f, TEMPMODEL_ANIMATE_ONCE, 10 );
      if(bigexplosion)
      {
         SpawnScaledExplosion(pos, scale);
         jitter->SetupLarge(pos);
      }
      else
      {
         TempModel(NULL, pos, { 0, 0, 0 }, "sprites/explode.spr", 0, scale, 1.0f, TEMPMODEL_ANIMATE_ONCE, 10);
         jitter->SetupSmall(pos);
      }
   }
   //###
}

//### added sin ed docs for scale control
/*****************************************************************************/
/*SINED func_exploder (0.4 0 0) (0 0 0) (8 8 8) x x x BIG_EXPLOSION

  Spawns an explosion when triggered.  Triggers any targets.

  "dmg" specifies how much damage to cause. Default indicates 120.
  "volume" volume at which to play explosions (default 1.0)
  "attenuation" attenuation for explosions (default normal)
  "key" The item needed to activate this. (default nothing)
  "scale" specifies the scale of the explosions to make. Default is 1.
/*****************************************************************************/

ResponseDef Exploder::Responses[] =
{
   { &EV_Touch,          NULL },
   { &EV_Trigger_Effect, (Response)&Exploder::MakeExplosion },
   { NULL, NULL }
};

CLASS_DECLARATION(Trigger, Exploder, "func_exploder");

void Exploder::MakeExplosion(Event *ev)
{
   CreateExplosion
   (
      worldorigin,
      edict->s.scale * damage,
      edict->s.scale,
      (spawnflags & BIG_EXPLOSION),
      this,
      ev->GetEntity(1),
      this,
      volume * edict->s.scale,
      attenuation
   );
}

Exploder::Exploder() : Trigger()
{
   damage = G_GetIntArg("dmg", 120);
   if(damage < 0)
   {
      damage = 0;
   }

   modelIndex("sprites/explode.spr");
   attenuation = G_GetFloatArg("attenuation", 1.0);
   volume = G_GetFloatArg("volume", 1.0);
   respondto = TRIGGER_PLAYERS | TRIGGER_MONSTERS | TRIGGER_PROJECTILES;
}

//### added sin ed docs for scale control
/*****************************************************************************/
/*SINED func_multi_exploder (0.4 0 0) ? x RANDOM_TIME RANDOM_SCALE BIG_EXPLOSION

  Spawns an explosion when triggered.  Triggers any targets.
  size of brush determines where explosions will occur.

  "dmg" specifies how much damage to cause from each explosion. (Default 120)
  "delay" delay before exploding (Default 0 seconds)
  "duration" how long to explode for (Default 1 second)
  "wait" time between each explosion (default 0.25 seconds)
  "volume" volume to play explosion sound at (default 0.5)
  "attenuation" attenuation for explosions (default normal)
  "random" random factor (default 0.25)
  "key" The item needed to activate this. (default nothing)
  "scale" specifies the scale of the explosions to make. Default is 1.

  RANDOM_TIME adjusts the wait between each explosion by the random factor
  RANDOM_SCALE adjusts the size of each explosion by the random factor

/*****************************************************************************/

CLASS_DECLARATION(Trigger, MultiExploder, "func_multi_exploder");

ResponseDef MultiExploder::Responses[] =
{
   { &EV_Touch, NULL },
   { &EV_Trigger_Effect, (Response)&MultiExploder::MakeExplosion },
   { NULL, NULL }
};

void MultiExploder::MakeExplosion(Event *ev)
{
   Vector pos;
   float t;
   float r;
   float v;
   Entity *other;
   Event *event;

   other = ev->GetEntity(1);

   // make sure other is valid
   if(!other)
   {
      other = world;
   }

   // prevent the trigger from triggering again
   trigger_time = -1;

   if(!explode_time)
   {
      explode_time = level.time + duration;
   }

   if(spawnflags & RANDOM_TIME)
   {
      t = explodewait * (1 + G_CRandom(randomness));
   }
   else
   {
      t = explodewait;
   }

   event = new Event(EV_Trigger_Effect);
   event->AddEntity(other);
   PostEvent(event, t);

   if(level.time > explode_time)
   {
      PostEvent(EV_Remove, 0);
      return;
   }

   pos[0] = absmin[0] + G_Random(absmax[0] - absmin[0]);
   pos[1] = absmin[1] + G_Random(absmax[1] - absmin[1]);
   pos[2] = absmin[2] + G_Random(absmax[2] - absmin[2]);

   if(spawnflags & RANDOM_SCALE)
   {
      r = edict->s.scale + G_CRandom(randomness);
   }
   else
   {
      r = edict->s.scale;
   }

   if(r < 1)
   {
      v = volume * r;
   }
   else
   {
      v = volume;
   }
   CreateExplosion
   (
      pos,
      damage * r,
      r,
      (spawnflags & BIG_EXPLOSION),
      this,
      other,
      this,
      volume,
      attenuation
   );
}

MultiExploder::MultiExploder() : Trigger()
{
   damage = G_GetIntArg("dmg", 120);
   if(damage < 0)
   {
      damage = 0;
   }

   attenuation  = G_GetFloatArg("attenuation", 1.0);
   volume       = G_GetFloatArg("volume", 1.0);
   duration     = G_GetFloatArg("duration", 1.0);
   explodewait  = G_GetFloatArg("wait", 0.25);
   randomness   = G_GetFloatArg("random", 0.25);
   explode_time = 0;

   // So that we don't get deleted after we're triggered
   count = -1;

   respondto = TRIGGER_PLAYERS | TRIGGER_MONSTERS | TRIGGER_PROJECTILES;
   modelIndex("sprites/explode.spr");
}

// EOF

