//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/speargun.cpp                     $
// $Revision:: 42                                                             $
//   $Author:: Markd                                                          $
//     $Date:: 11/15/98 11:32p                                                $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Fires a spear.  Used by Seabonites.
// 

#include "g_local.h"
#include "SpearGun.h"
#include "worldspawn.h"
#include "specialfx.h"

CLASS_DECLARATION(Projectile, Spear, NULL);

ResponseDef Spear::Responses[] =
{
   { &EV_Touch,				(Response)&Spear::Hit },
   { NULL, NULL }
};

EXPORT_FROM_DLL void Spear::Hit(Event *ev)
{
   Entity *other;
   Vector org;
   int damg;

   other = ev->GetEntity(1);

   if(other->isSubclassOf<Teleporter>())
   {
      return;
   }

   if(other->entnum == owner)
   {
      return;
   }

   setSolidType(SOLID_NOT);

   if(HitSky() || (velocity == vec_zero))
   {
      PostEvent(EV_Remove, 0);
      return;
   }

   RandomAnimate("idle", NULL);

   org = worldorigin + velocity * 0.01f;
   if(other->takedamage)
   {
      if(other->flags & FL_BLOOD)
      {
         SpawnBlood(org, level.impact_trace.plane.normal, 10);
      }

      damg = 30 + (int)G_Random(50);
      other->Damage(this, G_GetEntity(owner), damg,
                    level.impact_trace.endpos, velocity,
                    level.impact_trace.plane.normal, 10, 0, MOD_SPEARGUN, -1, -1, 1.0f);
      //FIXME
      // do this based on the surface flag
      RandomGlobalSound("impact_goryimpact", 1);
      if(level.impact_trace.intersect.valid)
      {
         vec3_t mins, maxs;

         setMoveType(MOVETYPE_NONE);
         gi.CalculateBounds(edict->s.modelindex, edict->s.scale, mins, maxs);
         origin = { 0, 0, 0 };
         origin[0] = (Vector(maxs) - Vector(mins)).length() * 0.3f;
         setOrigin(origin);
         velocity = { 0, 0, 0 };
         attach
         (
            other->entnum,
            level.impact_trace.intersect.group,
            level.impact_trace.intersect.tri_num,
            vec_zero
         );
         CancelEventsOfType(EV_Remove);
         PostEvent(EV_FadeOut, 45);
         return;
      }
   }
   else
   {
      RandomSound("spear_impact", 1);
   }

   if(other->flags & FL_SPARKS)
      SpawnSparks(org, level.impact_trace.plane.normal, 4);

   if(other->getSolidType() == SOLID_BSP)
   {
      // Stick it into the wall
      velocity = { 0, 0, 0 };
      //bind( other );
      setMoveType(MOVETYPE_NONE);
      PostEvent(EV_FadeOut, 10);
   }
   else
   {
      PostEvent(EV_Remove, 0);
   }
}

EXPORT_FROM_DLL void Spear::Setup(Entity *owner, Vector pos, Vector dir)
{
   trace_t trace;

   this->owner = owner->entnum;
   edict->owner = owner->edict;

   setMoveType(MOVETYPE_FLYMISSILE);
   setSolidType(SOLID_BBOX);
   edict->clipmask = MASK_SHOT;

   // set missile speed
   velocity = dir;
   velocity.normalize();
   velocity *= 500;

   angles = dir.toAngles();
   angles[PITCH] = -angles[PITCH];
   setAngles(angles);

   setModel("spear.def");
   RandomAnimate("fly", NULL);

   setSize({ -2, -2, -2 }, { 2, 2, 2 });
   trace = G_Trace(pos - Vector(orientation[0]) * 48, mins, maxs, pos, owner, MASK_SHOT, "Spear::Setup");
   setOrigin(trace.endpos);
   worldorigin.copyTo(edict->s.old_origin);

   PostEvent(EV_Remove, 5);
}

CLASS_DECLARATION(Weapon, SpearGun, "weapon_speargun");

ResponseDef SpearGun::Responses[] =
{
   { &EV_Weapon_Shoot,	(Response)&SpearGun::Shoot },
   { NULL, NULL }
};

SpearGun::SpearGun() : Weapon()
{
#ifdef SIN_DEMO
   PostEvent(EV_Remove, 0);
   return;
#endif
   SetModels("speargun.def", "view_spgun.def");
   modelIndex("spear.def");
   modelIndex("spear_ammo.def");
   SetAmmo("Spears", 1, 10);
   SetRank(99, 70);
   SetType(WEAPON_2HANDED_LO);
   SetMinRange(100);
}

void SpearGun::Shoot(Event *ev)
{
   Vector	pos;
   Spear		*spear;
   Vector	dir;

   assert(owner);
   if(!owner)
   {
      return;
   }

   GetMuzzlePosition(&pos, &dir);

   spear = new Spear();
   spear->Setup(owner, pos, dir);

   NextAttack(0.8);
}

// EOF

