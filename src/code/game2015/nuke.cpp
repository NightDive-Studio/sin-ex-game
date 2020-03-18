/*
================================================================
NUKE LAUNCHER aka IP36
================================================================

Copyright (C) 2020 by Night Dive Studios, Inc.
All rights reserved.

See the license.txt file for conditions and terms of use for this code.
*/

#include "g_local.h"
#include "explosion.h"
#include "nuke.h"
#include "worldspawn.h"
#include "misc.h"
#include "surface.h"
#include "jitter.h"
#include "ctf.h"

#define NUKE_SPEED  700
#define NUKE_RADIUS 800

//=================================================================
// nuke flash maker

CLASS_DECLARATION(Entity, NukeFlash, nullptr);

Event EV_NukeFlash_Flash("nukeflash_flash");

ResponseDef NukeFlash::Responses[] =
{
   { &EV_NukeFlash_Flash, (Response)&NukeFlash::Flash },
   { nullptr, nullptr }
};

EXPORT_FROM_DLL void NukeFlash::Flash(Event *ev)
{
   Entity *ent = G_NextEntity(NULL);
   while(ent)
   {
      // only apply view flash to players
      if(ent->isSubclassOf<Player>())
      {
         if(CanDamage(ent))
         {
            static_cast<Player *>(ent)->StartNukeFlash(fblend, falpha);
         }
      }

      ent = G_NextEntity(ent);
   }

   // do fade in
   if(falpha > 1)
   {
      //fade in the flash
      falpha += 0.4;
      // switch to fade out when we reach full opacity
      if(falpha >= 2)
         falpha = 1;
   }
   else // do fade out
   {
      // decay color to red
      fblend[1] -= 0.02;
      fblend[2] = fblend[1];

      // decay alpha
      if(falpha > 0.6)
         falpha -= 0.1;
      else
      {
         falpha -= 0.035;
         if(falpha < 0)
            falpha = 0;
      }
   }

   if(falpha > 0)
      PostEvent(EV_NukeFlash_Flash, 0.1);
   else
      ProcessEvent(EV_Remove);
}

void NukeFlash::Setup(Vector pos, float delay)
{
   setMoveType(MOVETYPE_NONE);
   setSolidType(SOLID_NOT);

   setOrigin(pos);
   worldorigin.copyTo(edict->s.old_origin);

   fblend[0] = fblend[1] = fblend[2] = 1;
   falpha = 1.01;

   if(delay)
      PostEvent(EV_NukeFlash_Flash, delay);
   else
      ProcessEvent(EV_NukeFlash_Flash);
}
//=================================================================
// nuke fireball

CLASS_DECLARATION(Projectile, NukeFireball, nullptr);

Event EV_NukeFireball_Animate("fireball_animate");
Event EV_NukeFireball_Damage("fireball_damage");
Event EV_NukeFireball_Fade("fireball_fade");

ResponseDef NukeFireball::Responses[] =
{
   { &EV_NukeFireball_Animate, (Response)&NukeFireball::Animate  },
   { &EV_NukeFireball_Damage,  (Response)&NukeFireball::DoDamage },
   { &EV_NukeFireball_Fade,    (Response)&NukeFireball::Fade     },
   { nullptr, nullptr }
};

EXPORT_FROM_DLL void NukeFireball::DoDamage(Event *ev)
{
   int damage;

   if(deathmatch->value)
      damage = 80;
   else
      damage = 120;

   float rad    = edict->s.scale*64 + 128;
   float minrad = rad - 256;

   Entity *ownerent = G_GetEntity(owner);

   Entity *ent = findradius(nullptr, origin.vec3(), rad);
   while(ent)
   {
      if(ent->takedamage)
      {
         Vector org(ent->origin + (ent->mins + ent->maxs)*0.5);
         Vector v(origin - org);
         float dist = v.length();

         if(dist > minrad)
         {
            float points =  dist*(float)0.04;
            if(points < 0)
            {
               points = 0;
            }
            points = damage - points;
            if(ent == ownerent)
            {
               points *= 0.75;
            }

            if(points > 0)
            {
               if(CanDamage(ent))
               {
                  points += 0.5;
                  ent->Damage(this, ownerent, (int)points, ent->origin, v*(-1), vec_zero, points*0.1, DAMAGE_RADIUS, MOD_NUKEEXPLOSION, -1, -1, 1.0f);
               }
            }
         }
      }
      ent = findradius(ent, origin.vec3(), rad);
   }

   // continue doing damage every 0.1 seconds
   PostEvent(EV_NukeFireball_Damage, 0.1);
}

EXPORT_FROM_DLL void NukeFireball::Fade(Event *ev)
{
   PostEvent(EV_NukeFireball_Fade, 0.1);

   edict->s.renderfx |= RF_TRANSLUCENT;
   translucence += 0.15;
   if(translucence >= 0.98)
   {
      ProcessEvent(EV_Remove);
      return;
   }
   setAlpha(1.0 - translucence);
}

EXPORT_FROM_DLL void NukeFireball::Animate(Event *ev)
{
   edict->s.scale += 0.4;
   PostEvent(EV_NukeFireball_Animate, 0.1);
}

void NukeFireball::Setup(Entity *owner, Vector pos, float life, int type)
{
   this->owner  = owner->entnum;
   edict->owner = owner->edict;

   setMoveType(MOVETYPE_FLY);
   setSolidType(SOLID_NOT);

   edict->s.frame = 0;

   angles[PITCH] = 0;
   angles[YAW  ] = random() * 360;
   angles[ROLL ] = 0;
   setAngles(angles);

   setModel("sprites/nukering.spr");
   edict->s.scale = 0.1;
   // setup it's transparency
   edict->s.renderfx |= RF_TRANSLUCENT;
   translucence = 0;
   setAlpha(1.0f - translucence);
   // the outward most fireball does the expanding damage
   PostEvent(EV_NukeFireball_Damage,  0.1);
   PostEvent(EV_NukeFireball_Animate, 0.2);

   setOrigin(pos);
   worldorigin.copyTo(edict->s.old_origin);

   PostEvent(EV_NukeFireball_Fade, life);
}

//=================================================================
// nuke energyball

CLASS_DECLARATION(Projectile, NukeBall, nullptr);

Event EV_NukeBall_Animate("nuke_animate");
Event EV_NukeBall_Collapse("nuke_collapse");
Event EV_NukeBall_Explode("nuke_explode");
Event EV_NukeBall_ThrowRing("nuke_throwring");
Event EV_NukeBall_ExplosionBall("nuke_explosionball");

ResponseDef NukeBall::Responses[] =
{
   { &EV_Touch,                  (Response)&NukeBall::Collapse      },
   { &EV_NukeBall_Animate,       (Response)&NukeBall::Animate       },
   { &EV_NukeBall_Collapse,      (Response)&NukeBall::Collapse      },
   { &EV_NukeBall_Explode,       (Response)&NukeBall::Explode       },
   { &EV_NukeBall_ThrowRing,     (Response)&NukeBall::ThrowRing     },
   { &EV_NukeBall_ExplosionBall, (Response)&NukeBall::ExplosionBall },
   { nullptr, nullptr }
};

EXPORT_FROM_DLL void NukeBall::Animate(Event *ev)
{
   edict->s.frame++;
   if(edict->s.frame >= 10)
      edict->s.frame = 0;

   PostEvent(EV_NukeBall_Animate, FRAMETIME);
}

EXPORT_FROM_DLL void NukeBall::Collapse(Event *ev)
{
   Entity	*other;
   Entity	*owner;
   int damg;

   other = ev->GetEntity(1);

   owner =  G_GetEntity(this->owner);

   if(deathmatch->value)
      damg = 250;
   else
      damg = 300;

   // touched something that triggered exploding
   if(ballstate < NBS_COLLAPSING)
   {
      qboolean antizone;
      Entity *ent;

      assert(other);

      if(other->isSubclassOf<Teleporter>())
         return;
      if(other->entnum == this->owner)
         return;

      // whak touched entity
      if(other->takedamage)
         other->Damage(this, owner, damg, origin, velocity, level.impact_trace.plane.normal, 32, 0, MOD_NUKE, -1, -1, 1.0f);

      surfaceManager.DamageSurface(&level.impact_trace, damg, owner);

      // make it collapse
      CancelEventsOfType(EV_NukeBall_Animate);

      setMoveType(MOVETYPE_FLY);
      setSolidType(SOLID_NOT);
      velocity.normalize();
      setOrigin(origin - velocity*24);
      velocity = vec_zero;
      edict->s.scale = 0.2;

      TempModel(nullptr, origin, vec_zero, "sprites/implosion.spr", 0, 1, 1.0f, TEMPMODEL_ANIMATE_ONCE|TEMPMODEL_ANIMATE_FAST, 2);

      // post explosion event for end of collapsing
      PostEvent(EV_NukeBall_Explode, 0.4);

      //create flash entity
      auto flash = new NukeFlash();
      flash->Setup(origin, 0.2);

      // check for touching NoNukeZone
      antizone = false;
      ent = G_NextEntity(nullptr);
      while(ent)
      {
         if(ent->isSubclassOf<NoNukeZone>())
         {
            if((origin.x < ent->absmax.x) && (origin.x > ent->absmin.x) &&
               (origin.y < ent->absmax.y) && (origin.y > ent->absmin.y) &&
               (origin.z < ent->absmax.z) && (origin.z > ent->absmin.z))
            {
               // we're inside one
               antizone = true;
               break;
            }
         }

         ent = G_NextEntity(ent);
      }

      // check for evilness...
      if(ballstate == NBS_EVIL && antizone)
      {
         // evilness, here we come...
         int       evilcount;
         Vector    evildirection;
         Vector    pos, dir;
         Vector    right, up;
         Vector    tmpvec;

         for(evilcount = 0; evilcount < 6; evilcount++)
         {
            switch(evilcount)
            {
            case 0:
               evildirection = Vector(1, 0, 0);
               break;
            case 1:
               evildirection = Vector(-1, 0, 0);
               break;
            case 2:
               evildirection = Vector(0, 1, 0);
               break;
            case 3:
               evildirection = Vector(0, -1, 0);
               break;
            case 4:
               evildirection = Vector(0, 0, 1);
               break;
            case 5:
            default:
               evildirection = Vector(0, 0, -1);
               break;
            }

            tmpvec = worldorigin + evildirection*24;

            auto ball = new NukeBall();
            ball->Setup(owner, tmpvec, evildirection);
            ball->takedamage = DAMAGE_NO;
         }
      }

      //setup imploding stuff
      ballstate = NBS_COLLAPSING;
   }
   // always check if something was touched
   else if(other)
   {
      if(other->takedamage)
         other->Damage(this, owner, damg, origin, velocity, level.impact_trace.plane.normal, 32, 0, MOD_NUKE, -1, -1, 1.0f);

      surfaceManager.DamageSurface(&level.impact_trace, damg, owner);
   }

   // collapse & animate the model
   if(edict->s.scale > 0.2)
   {
      edict->s.scale -= 0.2;
      if(edict->s.scale <= 0.2)
      {
         edict->s.scale = 0.2;
         edict->s.frame = 0;
         stopsound(CHAN_VOICE);
         hideModel();
      }
      else
      {
         edict->s.frame++;
         if(edict->s.frame >= 10)
            edict->s.frame = 0;
      }
   }
}

EXPORT_FROM_DLL void NukeBall::Explode(Event *ev)
{
   NukeFireball *fball;
   Event *event;
   Entity *owner;
   qboolean antizone;
   Entity *ent;

   CancelEventsOfType(EV_NukeBall_Animate);
   CancelEventsOfType(EV_NukeBall_Collapse);

   takedamage = DAMAGE_NO;

   owner = G_GetEntity(this->owner);

   // check for touching NoNukeZone
   antizone = false;
   ent = G_NextEntity(nullptr);
   while(ent)
   {
      if(ent->isSubclassOf<NoNukeZone>())
      {
         if((origin.x < ent->absmax.x) && (origin.x > ent->absmin.x) &&
            (origin.y < ent->absmax.y) && (origin.y > ent->absmin.y) &&
            (origin.z < ent->absmax.z) && (origin.z > ent->absmin.z))
         {
            // we're inside one
            antizone = true;
            break;
         }
      }

      ent = G_NextEntity(ent);
   }

   // small amount of damage to everyone visible from initial flash
   RadiusDamage(this, owner, 20, nullptr, MOD_NUKEEXPLOSION, 0.002);

   // alota damage to anything near the initial blast
   if(antizone) // don't do as much explosive damage
      RadiusDamage(this, owner, 200, nullptr, MOD_NUKEEXPLOSION, 0.35);
   else if(deathmatch->value)
      RadiusDamage(this, owner, 250, nullptr, MOD_NUKEEXPLOSION, 0.4);
   else
      RadiusDamage(this, owner, 300, nullptr, MOD_NUKEEXPLOSION, 0.45);

   // post the particle explosion events
   if(antizone)
   { 
      // much smaller explosion effect when in an anti-nuke zone
      event = new Event(EV_NukeBall_ExplosionBall);
      event->AddInteger(40);
      ProcessEvent(event);

      event = new Event(EV_NukeBall_ExplosionBall);
      event->AddInteger(64);
      PostEvent(event, 0.4);

      // no expanding fireball of death, just a visual effect
      TempModel(nullptr, origin, Vector(90, 0, G_Random(360)), "sprites/nukering2.spr", 0, 5.0f, 1.0f, TEMPMODEL_ANIMATE_SCALE|TEMPMODEL_ALPHAFADE, 15.0f);
   }
   else
   {
      event = new Event(EV_NukeBall_ExplosionBall);
      event->AddInteger(64);
      ProcessEvent(event);

      event = new Event(EV_NukeBall_ExplosionBall);
      event->AddInteger(128);
      PostEvent(event, 0.6);

      event = new Event(EV_NukeBall_ExplosionBall);
      event->AddInteger(240);
      PostEvent(event, 1.1);

      fball = new NukeFireball;
      fball->Setup(owner, origin, 1.8, 1);

      TempModel(NULL, origin, Vector(90, 0, G_Random(360)), "sprites/nukering2.spr", 0, 10.0f, 1.0f, TEMPMODEL_ANIMATE_SCALE|TEMPMODEL_ALPHAFADE, 20.0f);
   }

   // nuke explosion sounds
   RandomPositionedSound(origin, "impact_nukeexplosion", 1, CHAN_AUTO, 0.75);

   // add in the ass whoopin' view jitter
   auto jitter = new RadiusJitter();
   jitter->Setup(origin, 1600, 0.03,
                 0.8, 15, 4,
                 1.5, 10, 5);

   PostEvent(EV_Remove, 1.5);

   //do a bit of dialog for the first time firing it in single player
   if(owner->isClient() && !deathmatch->value)
   {
      ScriptVariable *var = gameVars.GetVariable("firstnukefire");

      if(!var) // hasn't been fired yet this game
      {
         var = gameVars.CreateVariable("firstnukefire", 1);

         // call the dialog thread for it
         ExecuteThread("global/nukeweapon.scr::nuke_first_fire", true);
      }
   }
}

EXPORT_FROM_DLL void NukeBall::ThrowRing(Event *ev)
{
   TempModel(NULL, origin, Vector(90, G_Random(360), 0), "sprites/nukering2.spr",
             0, 8.0f, 1.0f, TEMPMODEL_ANIMATE_SCALE|TEMPMODEL_ALPHAFADE, 15.0f);
   TempModel(NULL, origin, Vector(-90, G_Random(360), 0), "sprites/nukering2.spr",
             0, 8.0f, 1.0f, TEMPMODEL_ANIMATE_SCALE|TEMPMODEL_ALPHAFADE, 15.0f);
}

EXPORT_FROM_DLL void NukeBall::ExplosionBall(Event *ev)
{
   int size = ev->GetInteger(1);
   SpawnNukeExplosion(origin, size, 153);
}

EXPORT_FROM_DLL void NukeBall::Setup(Entity *owner, Vector pos, Vector dir)
{
   this->owner = owner->entnum;
   edict->owner = owner->edict;

   setMoveType(MOVETYPE_FLYMISSILE);
   setSolidType(SOLID_BBOX);
   edict->clipmask = MASK_PROJECTILE;

   edict->s.frame = 0;

   angles = dir.toAngles();
   angles[PITCH] = -angles[PITCH];
   setAngles(angles);

   speed = NUKE_SPEED;
   velocity = dir * NUKE_SPEED;

   // set nuke duration
   PostEvent(EV_NukeBall_Animate, FRAMETIME);
   auto ev = new Event(EV_NukeBall_Collapse);
   ev->AddEntity(world);
   PostEvent(ev, 1.5);

   ballstate = NBS_FLYING;

   setModel("sprites/nukeball.spr");
   edict->s.renderfx |= RF_DLIGHT;
   edict->s.effects  |= EF_NUKETRAIL;
   gravity = 0;
   edict->s.color_r = 0.8;
   edict->s.color_g = 0.2;
   edict->s.color_b = 0.3;
   edict->s.radius  = 350;

   // setup ambient thrust
   edict->s.sound  = gi.soundindex("weapons/nuke/ballsound.wav");
   edict->s.sound |= ATTN_IDLE<<14;

   setSize({ -8, -8, -8 }, { 8, 8, 8 });
   setOrigin(pos);
   worldorigin.copyTo(edict->s.old_origin);
}

// for devious purposes...
EXPORT_FROM_DLL void NukeBall::EvilSetup(Entity *owner, Vector pos, Vector dir)
{
   Setup(owner, pos, dir);
   ballstate = NBS_EVIL;
}

//=================================================================
// nuke launcher

CLASS_DECLARATION(Weapon, IP36, "weapon_ip36");

Event EV_Nuke_Launch("nuke_launch");

ResponseDef IP36::Responses[] =
{
   { &EV_Weapon_Shoot, (Response)&IP36::Shoot  },
   { &EV_Nuke_Launch,  (Response)&IP36::Launch },
   { nullptr, nullptr }
};

IP36::IP36() : Weapon()
{
   SetModels("nuke_w.def", "view_nuke.def");
   modelIndex("sprites/nukeball.spr");
   modelIndex("sprites/implosion.spr");
   modelIndex("sprites/nukering.spr");
   modelIndex("sprites/nukering2.spr");
   SetAmmo("IlludiumModules", 1, 1);
   SetRank(110, 0);
   SetType(WEAPON_2HANDED_LO);

   SetMinRange(NUKE_RADIUS);
   SetProjectileSpeed(NUKE_SPEED);

   // precache the first fire dialog if needed
   if(!deathmatch->value)
   {
      ScriptVariable *var = gameVars.GetVariable("firstnukefire");

      // only bother to precache if hasn't been fired yet
      if(!var)
         G_LoadAndExecScript("global/nukeweapon.scr", "nuke_first_fire_precache");
   }
}

void IP36::Shoot(Event *ev)
{
   Vector tmpvec;

   if((!owner) || (owner->IsOnBike()))
      return;

   Vector pos, dir, right, up;
   GetMuzzlePosition(&pos, &dir, &right, &up);
   tmpvec = pos - right*8;

   auto ball = new NukeBall();
   ball->Setup(owner, tmpvec, dir);

   NextAttack(3);
}

void IP36::Launch(Event *ev)
{
   Vector tmpvec;

   if((!owner) || (owner->IsOnBike()))
      return;

   Vector pos, dir, right, up;
   GetMuzzlePosition(&pos, &dir, &right, &up);
   tmpvec = pos - right*8;

   auto ball = new NukeBall();
   ball->Setup(owner, tmpvec, dir);
}

//==================================================================
// And yae, from yonder, hither weapon doth seth fourth evil things...

class EXPORT_FROM_DLL EvilIP36 : public IP36
{
public:
   CLASS_PROTOTYPE(EvilIP36);

   virtual void Shoot(Event *ev);
};

CLASS_DECLARATION(IP36, EvilIP36, "weapon_evilip36");

ResponseDef EvilIP36::Responses[] =
{
   { &EV_Weapon_Shoot, (Response)&EvilIP36::Shoot },
   { nullptr, nullptr }
};

void EvilIP36::Shoot (Event *ev)
{
   Vector tmpvec;

   if((!owner) || (owner->IsOnBike()))
      return;

   Vector pos, dir, right, up;
   GetMuzzlePosition(&pos, &dir, &right, &up);
   tmpvec = pos - right*8;

   auto ball = new NukeBall();
   ball->EvilSetup(owner, tmpvec, dir);
   ball->takedamage = DAMAGE_NO;

   NextAttack(2);
}

// EOF

