//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/eonandpeon.cpp                   $
// $Revision:: 8                                                              $
//   $Author:: Markd                                                          $
//     $Date:: 11/09/98 12:31a                                                $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Eon and Peon
// 

#include "g_local.h"
#include "actor.h"
#include "eonandpeon.h"
#include "specialfx.h"
#include "gibs.h"

class EXPORT_FROM_DLL EonBomb : public Projectile
{
public:
   CLASS_PROTOTYPE(EonBomb);

   virtual void   Setup(Entity *owner, Vector pos, Vector vel) override;
   virtual void   EonBombTouch(Event *ev);
};

CLASS_DECLARATION(Projectile, EonBomb, nullptr);

ResponseDef EonBomb::Responses[] =
{
   { &EV_Touch, (Response)&EonBomb::EonBombTouch },
   { nullptr, nullptr }
};

void EonBomb::EonBombTouch(Event *ev)
{
   if(sv_gibs->value && !parentmode->value)
   {
      CreateGibs(this, health, 0.3, 10);
   }
   PostEvent(EV_Remove, 0);
}

void EonBomb::Setup(Entity *owner, Vector pos, Vector vel)
{
   // Flies like a grenade
   setMoveType(MOVETYPE_TOSS);
   setSolidType(SOLID_BBOX);
   edict->clipmask = MASK_PROJECTILE;
   setModel("eon.def");
   RandomAnimate("idle", nullptr);

   // Set the flying velocity
   velocity = vel;

   takedamage = DAMAGE_NO;

   setSize({ -1, -1, -1 }, { 1, 1, 1 });
   setOrigin(pos);
   worldorigin.copyTo(edict->s.old_origin);

   // Remove the projectile in the future
   PostEvent(EV_Remove, 30);
}

CLASS_DECLARATION(Peon, EonAndPeon, "boss_eonandpeon");

Event EV_EonAndPeon_SpawnGoo("spawngoo");

ResponseDef EonAndPeon::Responses[] =
{
   { &EV_EonAndPeon_SpawnGoo, (Response)&EonAndPeon::SpawnGoo },
   { &EV_Killed,              (Response)&EonAndPeon::Killed },
   { &EV_FadeOut,             nullptr },
   { nullptr, nullptr }
};

EonAndPeon::EonAndPeon() : Peon()
{
   eon = new Entity();
   eon->setModel("boss_eon.def");

   levelVars.SetVariable("eon", eon);

   setModel("boss_peon.def");
   flags |= FL_POSTTHINK;
}

void EonAndPeon::Chatter(const char *snd, float chance, float volume, int channel)
{
   if(chattime > level.time)
   {
      return;
   }

   if(eon)
      eon->RandomSound(snd, volume, channel, ATTN_NONE);
   RandomSound(snd, volume, channel, ATTN_NONE);

   chattime = level.time + 7 + G_Random(5);
}

void EonAndPeon::Postthink()
{
   if(eon)
   {
      eon->setOrigin(worldorigin);
      eon->setAngles(worldangles);
      eon->edict->s.anim = edict->s.anim;
      eon->edict->s.frame = edict->s.frame;
      eon->edict->s.scale = edict->s.scale;
   }
}

void EonAndPeon::Killed(Event *ev)
{
   EonBomb * eonbomb;
   Vector vel;
   Vector pos;
   Vector target;
   float  speed;

   pos = worldorigin;
   pos.z = absmax.z;

   speed = 250;

   if(currentEnemy)
   {
      target = G_PredictPosition(pos, currentEnemy->centroid, currentEnemy->velocity, speed);

      vel = G_CalculateImpulse
      (
         pos,
         target,
         speed,
         0.1f
      );
   }
   else
   {
      vel.z = 200;
   }

   eonbomb = new EonBomb();
   eonbomb->gravity = 0.1f;
   eonbomb->Setup(this, pos, vel);
   eonbomb->RandomSound("snd_yell");

   // delete the real eon
   eon->PostEvent(EV_Remove, 0);
   eon = nullptr;
   levelVars.SetVariable("eon", 0);

   //
   // call normal actor function
   //
   Actor::Killed(ev);
}

// EOF

