//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/fists.cpp                           $
// $Revision:: 34                                                             $
//   $Author:: Markd                                                          $
//     $Date:: 11/17/98 1:31a                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Normal Hands
// 

#include "g_local.h"
#include "fists.h"
#include "misc.h"
#include "specialfx.h"
#include "surface.h"

CLASS_DECLARATION(Weapon, Fists, NULL);

ResponseDef Fists::Responses[] =
{
   { &EV_Weapon_Shoot, (Response)&Fists::Shoot },
   { NULL, NULL }
};

Fists::Fists() : Weapon()
{
   SetModels(NULL, "view_punch.def");
   SetAmmo(NULL, 0, 0);
   SetRank(10, 10);
   strike_reach = 64;
   strike_damage = 20;
   SetMaxRange(strike_reach);
   SetType(WEAPON_MELEE);
   kick = 40;
   meansofdeath = MOD_FISTS;
}

void Fists::Shoot(Event * ev)
{
   trace_t	trace;
   Vector	start;
   Vector	end;
   float    damage;
   Vector   org;
   Vector   dir;
   int      surfflags;
   int      surftype;

   assert(owner);
   if(!owner)
   {
      return;
   }

   NextAttack(1);

   damage = G_Random(strike_damage)+strike_damage;

   GetMuzzlePosition(&start, &dir);
   end	= start + dir * strike_reach;

   trace = G_FullTrace(start, vec_zero, vec_zero, end, 64, owner, MASK_PROJECTILE, "Fists::Shoot");

   if(!trace.surface)
   {
      surfflags = 0;
      surftype = 0;
   }
   else
   {
      surfflags = trace.surface->flags;
      surftype = SURFACETYPE_FROM_FLAGS(surfflags);
      surfaceManager.DamageSurface(&trace, damage, owner);
   }
   dir = Vector(trace.endpos) - start;
   dir.normalize();

   org = Vector(trace.endpos) - dir;

   if((trace.fraction < 1.0f))
   {
      if(trace.ent->entity && trace.ent->entity->takedamage)
      {
         if(trace.ent->entity->flags & FL_BLOOD)
         {
            if((meansofdeath == MOD_MUTANTHANDS) || (trace.ent->entity->health < -500))
            {
               owner->RandomGlobalSound("impact_goryimpact");
            }
            else
            {
               owner->RandomGlobalSound("impact_bodyimpact");
            }
            SpawnBlood(org, trace.plane.normal, damage);
         }
         else
         {
            gi.WriteByte(svc_temp_entity);
            gi.WriteByte(TE_STRIKE);
            gi.WritePosition(org.vec3());
            gi.WriteDir(trace.plane.normal);
            gi.WriteByte(120);
            gi.WriteByte(surftype);
            gi.multicast(org.vec3(), MULTICAST_PVS);
         }
         if(trace.intersect.valid)
         {
            // take the ground out so that the kick works
            trace.ent->entity->groundentity = NULL;

            // We hit a valid group so send in location based damage
            trace.ent->entity->Damage(
               this,
               owner,
               damage,
               trace.endpos,
               dir,
               trace.plane.normal,
               kick,
               0,
               meansofdeath,
               trace.intersect.parentgroup,
               -1,
               1);
            //trace.intersect.damage_multiplier );
         }
         else
         {
            // We didn't hit any groups, so send in generic damage
            trace.ent->entity->Damage(
               this,
               owner,
               damage,
               trace.endpos,
               dir,
               trace.plane.normal,
               kick,
               0,
               meansofdeath,
               -1,
               -1,
               1);
         }
      }
      else
      {
         gi.WriteByte(svc_temp_entity);
         gi.WriteByte(TE_STRIKE);
         gi.WritePosition(org.vec3());
         gi.WriteDir(trace.plane.normal);
         gi.WriteByte(120);
         gi.WriteByte(surftype);
         gi.multicast(org.vec3(), MULTICAST_PVS);
      }
   }
}

// EOF

