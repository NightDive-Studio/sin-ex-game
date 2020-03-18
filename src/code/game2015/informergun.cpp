/*
================================================================
INFORMER GUN
================================================================

Copyright (C) 2020 by Night Dive Studios, Inc.
All rights reserved.

See the license.txt file for conditions and terms of use for this code.
*/

#include "informergun.h"
#include "worldspawn.h"
#include "specialfx.h"

CLASS_DECLARATION(Beam, InformerBeam, nullptr);

Event EV_InformerBeam_FadeBeam("fadebeam");

ResponseDef InformerBeam::Responses[] =
{
   { &EV_InformerBeam_FadeBeam, (Response)&InformerBeam::FadeBeam },
   { nullptr, nullptr }
};

InformerBeam::InformerBeam() : Entity()
{
   hideModel();
   edict->s.frame   = 0;
   edict->s.skinnum = 0;

   setMoveType(MOVETYPE_NONE);
   setSolidType(SOLID_NOT);

   edict->s.renderfx   |= RF_BEAM;
   edict->s.modelindex  = 1;      // must be non-zero
}

void InformerBeam::setBeam(Vector startpos, Vector endpos, int diameter, float r, float g, float b, float alpha, float lifespan)
{
   showModel();

   if(lifespan)
      PostEvent(EV_Remove, lifespan);

   start = startpos;
   end = endpos;

   setOrigin(start);

   edict->s.old_origin[0] = end[0];
   edict->s.old_origin[1] = end[1];
   edict->s.old_origin[2] = end[2];

   edict->s.frame   = diameter;
   edict->s.skinnum = 255;

   edict->s.color_r = r;
   edict->s.color_g = g;
   edict->s.color_b = b;
   edict->s.alpha   = alpha;

   link();
}

void InformerBeam::FadeBeam(Event *ev)
{
   Vector beamvec, newstart, newend;
   float newalpha;

   edict->s.renderfx |= RF_TRANSLUCENT;
   beamvec = end - start;
   switch(beamstate)
   {
   case 1:
      newalpha = 0.3;
      newstart = start;
      newend   = start + beamvec*0.5;
      break;
   case 2:
      newalpha = 0.25;
      newstart = start + beamvec*0.25;
      newend   = start + beamvec*0.75;
      break;
   case 3:
      newalpha = 0.2;
      newstart = start + beamvec*0.5;
      newend   = start + beamvec;
      break;
   default:
      PostEvent(EV_Remove, 0);
      return;
      break;
   }

   beamstate++;

   setOrigin(newstart);

   edict->s.old_origin[0] = newend[0];
   edict->s.old_origin[1] = newend[1];
   edict->s.old_origin[2] = newend[2];

   edict->s.frame   = 2;
   edict->s.skinnum = 255;

   edict->s.color_r = 0.3;
   edict->s.color_g = 0.3;
   edict->s.color_b = 1;
   edict->s.alpha   = newalpha;

   link();

   PostEvent(EV_InformerBeam_FadeBeam, 0.1);
}

//========================================================

CLASS_DECLARATION(Weapon, InformerGun, nullptr);

ResponseDef InformerGun::Responses[] =
{
   { &EV_Weapon_Shoot, (Response)&InformerGun::Shoot },
   { nullptr, nullptr }
};

InformerGun::InformerGun() : Weapon()
{
   SetModels("informergun.def", "view_informergun.def");
   SetAmmo(nullptr, 0, 0);
   SetRank(0, 0);
   SetType(WEAPON_2HANDED_LO);
}

void InformerGun::Informify(Entity *ent)
{
   char userinfo[MAX_INFO_STRING];
   edict_t *tmpent;

   // first make owner not the informer
   owner->client->resp.informer = false;
   // correct the owner's model and skin
   memcpy(userinfo, owner->client->pers.userinfo, sizeof(userinfo));
   G_ClientUserinfoChanged(owner->edict, userinfo);

   // now make our victim into the informer
   ent->client->resp.informer = true;
   // correct the victim's model and skin
   memcpy(userinfo, ent->client->pers.userinfo, sizeof(userinfo));
   G_ClientUserinfoChanged(ent->edict, userinfo);
   // make the "I'm the informer" effects
   ent->sound("environment/levelsnds/druglab/ping-sound.wav", 0.75, CHAN_BODY, ATTN_IDLE);
   SpawnTeleportEffect(ent->centroid, 165);

   // broadcast a messages about who the new informer is
   for(int i = 1; i <= maxclients->value; i++)
   {
      if(!g_edicts[i].inuse)
      {
         continue;
      }

      tmpent = &g_edicts[i];

      if(tmpent == ent->edict)
         gi.centerprintf(tmpent, "jcx yv -80 boxtext \"You are the Informer!\"");
      else
         gi.centerprintf(tmpent, "jcx yv -80 string \"%s is the Informer!\"", ent->client->pers.netname);
   }
}

void InformerGun::Shoot(Event *ev)
{
   Vector pos;
   Vector dir;
   Vector end;
   trace_t trace;
   InformerBeam *beam;
   Vector right, up;
   int mask;

   assert(owner);
   if(!owner)
      return;

   NextAttack(0.8);

   GetMuzzlePosition(&pos, &dir, &right, &up);
   pos -= up*4;

   if(!fadingbeam)
   {
      beam = new InformerBeam();
      fadingbeam = beam;
   }
   else
   {
      beam = static_cast<InformerBeam *>(fadingbeam.ptr);
   }
   beamstate = 1;

   mask      = MASK_PROJECTILE;
   mask     &= ~CONTENTS_FENCE;
   end       = pos + dir*8192;
   trace     = G_Trace(pos, vec_zero, vec_zero, end, owner, mask, "InformerGun::Shoot");
   beamstart = pos;
   beamvec   = Vector(trace.endpos) - pos;

   if(trace.ent)
   {
      Entity *ent = trace.ent->entity;

      if(ent->isClient() && !ent->deadflag)
      {
         // informify the poor bastard
         Informify(ent);
      }
      else
         ent->Damage(owner, owner, 15 + G_Random(5), Vector(trace.endpos), dir, vec_zero, 0, 0, 0, -1, -1, 0);
   }
   else
   {
      trace     = G_Trace(pos, Vector(-4, -4, -4), Vector(4, 4, 4), end, owner, mask, "InformerGun::Shoot");
      beamstart = pos;
      beamvec   = Vector(trace.endpos) - pos;

      if(trace.ent)
      {
         Entity *ent = trace.ent->entity;

         if(ent->isClient() && !ent->deadflag)
         {
            // informify the poor bastard
            Informify(ent);
         }
         else
            ent->Damage(owner, owner, 15 + G_Random(5), Vector(trace.endpos), dir, vec_zero, 0, 0, 0, -1, -1, 0);
      }
      else
      {
         trace     = G_Trace(pos, Vector(-8, -8, -8), Vector(8, 8, 8), end, owner, mask, "InformerGun::Shoot");
         beamstart = pos;
         beamvec   = Vector(trace.endpos) - pos;

         if(trace.ent)
         {
            Entity *ent;

            ent = trace.ent->entity;

            if(ent->isClient() && !ent->deadflag)
            {
               // informify the poor bastard
               Informify(ent);
            }
            else
               ent->Damage(owner, owner, 15 + G_Random(5), Vector(trace.endpos), dir, vec_zero, 0, 0, 0, -1, -1, 0);
         }
      }
   }

   pos = beamstart + beamvec;
   beam->setBeam(beamstart, pos, 4, 0.5, 0.5, 1, 0.5, 0.5);
   beam->beamstate = 1;
   beam->CancelEventsOfType(EV_InformerBeam_FadeBeam);
   beam->PostEvent(EV_InformerBeam_FadeBeam, 0.1);
}

qboolean InformerGun::AutoChange()
{
   return false;
}

// Don't drop informerguns
qboolean InformerGun::Drop()
{
   return false;
}

qboolean InformerGun::ReadyToChange()
{
   return false;
}

// EOF

