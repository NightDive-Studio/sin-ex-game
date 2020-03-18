//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/gibs.cpp                         $
// $Revision:: 21                                                             $
//   $Author:: Jimdose                                                        $
//     $Date:: 11/19/98 9:29p                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Gibs - nuff said

#include "gibs.h"

const char *const body_parts[] =
{
   "gib1.def", "gibtorso.def", "gibhead.def", "gibleg.def"
   //"gib1.def", "gib2.def", "gibhead.def", "gibleg.def", "gibarm.def", "gibribs.def"
};

const int num_body_parts = (sizeof(body_parts) / sizeof(body_parts[0]));

CLASS_DECLARATION(Entity, Gib, "gib");

Event EV_ThrowGib("throwgib");

ResponseDef Gib::Responses[] =
{
   { &EV_ThrowGib, (Response)&Gib::Throw },
   { &EV_Touch,    (Response)&Gib::Splat },
   { nullptr, nullptr }
};

Gib::Gib(const char *name, qboolean blood_trail) : Entity()
{
   setSize({ 0, 0, 0 }, { 0, 0, 0 });
   setModel(name);
   setMoveType(MOVETYPE_BOUNCE);
   setSolidType(SOLID_BBOX);
   if(blood_trail)
      edict->s.effects |= EF_GIB;
   sprayed = false;
   fadesplat = true;
}

Gib::Gib() : Entity()
{
   setSize({ 0, 0, 0 }, { 0, 0, 0 });
   setModel("gib1.def");
   setMoveType(MOVETYPE_BOUNCE);
   setSolidType(SOLID_BBOX);
   edict->s.effects |= EF_GIB;
   sprayed = false;
   fadesplat = true;
}

void Gib::Splat(Event *ev)
{
   Vector end;

   if(deathmatch->value)
      return;

   if(!sv_gore->value)
      return;

   if(sprayed)
      return;

   sprayed = true;
   end = origin;
   end.z -= 1024;

   SprayBlood(origin, end, 25);

   setSolidType(SOLID_NOT);
}

void Gib::SprayBlood(Vector start, Vector end, int damage)
{
   trace_t     trace;
   float       dist;
   float       scale;
   Entity      *splat;
   Vector      norm;

   trace = G_Trace(start, vec_zero, vec_zero, end, NULL, MASK_SOLIDNONFENCE, "Gib::SprayBlood");

   if(HitSky(&trace) || (trace.ent->solid != SOLID_BSP))
   {
      return;
   }

   dist = (Vector(trace.endpos) - start).length();
   scale = (float)damage / (dist * 0.3);
   if(scale > 0.6)
   {
      scale = 0.6;
   }
   if(scale < 0.02)
   {
      return;
   }

   // Do a bloodsplat
   splat = new Entity();
   splat->setMoveType(MOVETYPE_NONE);
   splat->setSolidType(SOLID_NOT);
   splat->setModel("sprites/bloodsplat.spr");
   splat->edict->s.frame = G_Random(4);
   splat->setSize({ 0, 0, 0 }, { 0, 0, 0 });
   splat->edict->s.scale = scale * this->edict->s.scale;
   norm = trace.plane.normal;
   norm.x = -norm.x;
   norm.y = -norm.y;
   splat->angles = norm.toAngles();
   splat->angles.z = G_Random(360);
   splat->setAngles(splat->angles);
   splat->setOrigin(Vector(trace.endpos) + (Vector(trace.plane.normal) * 0.2));

   if(fadesplat)
      splat->PostEvent(EV_FadeOut, 3);
   else
      splat->PostEvent(EV_FadeOut, 30);
}

void Gib::ClipGibVelocity(void)
{
   if(velocity[0] < -400)
      velocity[0] = -400;
   else if(velocity[0] > 400)
      velocity[0] = 400;
   if(velocity[1] < -400)
      velocity[1] = -400;
   else if(velocity[1] > 400)
      velocity[1] = 400;
   if(velocity[2] < 200)
      velocity[2] = 200;	// always some upwards
   else if(velocity[2] > 600)
      velocity[2] = 600;
}

void Gib::SetVelocity(float damage)
{
   velocity[0] = 100.0 * crandom();
   velocity[1] = 100.0 * crandom();
   velocity[2] = 200.0 + 100.0 * random();

   avelocity = Vector(G_Random(600), G_Random(600), G_Random(600));

   if((damage < -150) && (G_Random() > 0.95f))
      velocity *= 2.0f;
   else if(damage < -100)
      velocity *= 1.5f;

   ClipGibVelocity();
}

void Gib::Throw(Event *ev)
{
   Entity *ent;

   ent = ev->GetEntity(1);
   setOrigin(ent->centroid);
   worldorigin.copyTo(edict->s.old_origin);
   SetVelocity(ev->GetInteger(2));
   edict->s.scale = ev->GetFloat(3);
   PostEvent(EV_FadeOut, 10 + G_Random(5));
}

void CreateGibs(Entity * ent, float damage, float scale, int num, const char * modelname)
{
   int i;
   Gib * gib;

   assert(ent);

   if(!ent)
      return;

   ent->RandomGlobalSound("impact_gib");
   for(i = 0; i < num; i++)
   {
      if(modelname)
      {
         gib = new Gib(modelname);
      }
      else
      {
         gib = new Gib(body_parts[i % num_body_parts]);
      }
      gib->setOrigin(ent->centroid);
      gib->worldorigin.copyTo(gib->edict->s.old_origin);
      gib->SetVelocity(damage);
      gib->edict->s.scale = scale + G_Random(scale * 0.3);
      gib->SetGravityAxis(ent->gravaxis);
      gib->PostEvent(EV_FadeOut, 10 + G_Random(5));
   }
}

// EOF

