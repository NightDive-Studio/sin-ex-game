/*
================================================================
FLASHLIGHT
================================================================

Copyright (C) 2020 by Night Dive Studios, Inc.
All rights reserved.

See the license.txt file for conditions and terms of use for this code.
*/

#include "q_shared.h"
#include "flashlight.h"
#include "specialfx.h"
#include "player.h" 
#include "hoverbike.h" 

#define FLASHLIGHT_RANGE 1000

CLASS_DECLARATION(InventoryItem, Flashlight, "powerups_flashlight");

Event EV_Flashlight_EmitLight("flashlight_emitlight");

ResponseDef Flashlight::Responses[] =
{
   { &EV_Flashlight_EmitLight, (Response)&Flashlight::EmitLight },
   { &EV_Item_Pickup,          (Response)&Flashlight::Pickup },
   { NULL, NULL }
};

Flashlight::Flashlight() : InventoryItem()
{
   setModel("flashlight.def");
   gi.soundindex("environment/switch/smallsw1.wav");
   gi.soundindex("environment/switch/metalsw3.wav");
   Set(300);
   setRespawnTime(20);
}

Flashlight::~Flashlight()
{
   if(lightent)
   {
      lightent->ProcessEvent(EV_Remove);
      lightent = nullptr;
   }

   if(lightent2)
   {
      lightent2->ProcessEvent(EV_Remove);
      lightent2 = nullptr;
   }
}

// toggles the flashlight on and off
void Flashlight::Use(Event *ev)
{
   assert(owner);
   assert(amount);

   // don't allow turning it off in a flashlight on DM game
   if((lighton > 0) && !(DM_FLAG(DF_FLASHLIGHTON)))
   {
      lighton *= -1;
      CancelEventsOfType(EV_Flashlight_EmitLight);
      owner->sound("environment/switch/metalsw3.wav", 1, CHAN_ITEM, ATTN_NORM);
      if(lightent)
      {
         lightent->PostEvent(EV_Remove, 0);
         lightent = nullptr;
      }

      if(lightent2)
      {
         lightent2->PostEvent(EV_Remove, 0);
         lightent2 = nullptr;
      }
   }
   else
   {
      if(lighton < 0)
         lighton *= -1;
      PostEvent(EV_Flashlight_EmitLight, 0.1);
      owner->sound("environment/switch/smallsw1.wav", 1, CHAN_ITEM, ATTN_NORM);
   }
}

void Flashlight::EmitLight(Event *ev)
{
   Vector dir, start, pos;
   trace_t trace;
   float dist, tmpflt;
   int mask;
   Entity *ignoreent;

   static int flashlight_sound_count = 0;

   // check on our owner
   if(!owner || owner->deadflag)
   {
      PostEvent(EV_Remove, 0);
      return;
   }

   lighton++;
   if(lighton > 10)
   {
      lighton = 1;

      // don't drain flashlight in a flashlight DM game
      if(!(DM_FLAG(DF_FLASHLIGHT)))
      {
         if(!(owner->flags & FL_GODMODE))
            amount--;

         if(amount <= 0)
         {
            // also remove the light entitys
            if(lightent)
            {
               lightent->PostEvent(EV_Remove, 0);
               lightent = nullptr;
            }

            if(lightent2)
            {
               lightent2->PostEvent(EV_Remove, 0);
               lightent2 = nullptr;
            }

            owner->RemoveItem(this);
            return;
         }
      }
   }

   // place beam entity
   dir = owner->orientation[0];
   // do the flashlight a bit differently if owner is on a bike
   if(owner->isClient() && owner->IsOnBike())
   {
      Hoverbike *bike;
      Vector tmpvec;

      start = owner->origin + Vector(0, 0, owner->viewheight) + dir * 40;
      // recalc dir based apon player's view direction
      tmpvec    = ((Player *)owner.ptr)->GetVAngle();
      tmpvec.AngleVectors(&dir, NULL, NULL);
      pos       = start + dir*FLASHLIGHT_RANGE;
      bike      = ((Player *)owner.ptr)->GetHoverbike();
      ignoreent = bike->frontbox;
   }
   else
   {
      start = owner->origin + Vector(0, 0, owner->viewheight);
      pos = start + dir*FLASHLIGHT_RANGE;
      ignoreent = owner;
   }

   mask = (MASK_PLAYERSOLID | MASK_WATER) & ~(CONTENTS_WINDOW | CONTENTS_FENCE);
   trace = G_Trace(start, vec_zero, vec_zero, pos, ignoreent, mask, "Flashlight::EmitLight");
   pos = start - Vector(trace.endpos);
   
   dist = FLASHLIGHT_RANGE - pos.length();
   if(dist < 1)
      tmpflt = 0;
   else
      tmpflt = dist / FLASHLIGHT_RANGE;
   
   // make sure we have entities here
   if(!lightent)
      lightent = new Entity();

   lightent->setModel("sprites/null.spr");
   lightent->setMoveType(MOVETYPE_NONE);
   lightent->setSolidType(SOLID_NOT);
   lightent->edict->s.renderfx |= RF_DLIGHT;
   lightent->edict->s.radius = 250 - tmpflt * 125;
   lightent->edict->s.color_b = 0.1 + tmpflt*0.5;
   
   tmpflt = 0.25 + tmpflt*0.75;
   if(tmpflt > 1)
      tmpflt = 1;
   
   lightent->edict->s.color_r = tmpflt;
   lightent->edict->s.color_g = tmpflt;
   lightent->setOrigin(trace.endpos);

   // have the beam light entity generate a movement sound
   if(flashlight_sound_count++ > 10)
   {
      float r2, dist2;
      int n, i;
      Sentient *ent;
      Vector delta;

      if(!(owner->flags & FL_NOTARGET))
      {
         r2 = SOUND_MOVEMENT_RADIUS * SOUND_MOVEMENT_RADIUS;
         n = SentientList.NumObjects();
         for(i = 1; i <= n; i++)
         {
            ent = SentientList.ObjectAt(i);
            if(ent->deadflag || (ent == owner))
               continue;

            delta = lightent->worldorigin - ent->centroid;

            // dot product returns length squared
            dist2 = delta * delta;
            if((dist2 <= r2) &&
               ((lightent->edict->areanum == ent->edict->areanum) ||
                (gi.AreasConnected(lightent->edict->areanum, ent->edict->areanum))))
            {
               ev = new Event(EV_HeardMovement);
               ev->AddEntity(owner);
               ev->AddVector(lightent->worldorigin);
               ent->PostEvent(ev, 0);
            }
         }
      }

      if(flashlight_sound_count > 15)
         flashlight_sound_count = 0;
   }

   // place the near area light entity if not in deathmatch
   if(!deathmatch->value)
   {
      start = owner->origin + Vector(0, 0, owner->viewheight);
      pos   = start + dir * 100;
      trace = G_Trace(start, vec_zero, vec_zero, pos, owner, mask, "Flashlight::EmitLight");

      if(!lightent2)
         lightent2 = new Entity();
      lightent2->setModel("sprites/null.spr");
      lightent2->setMoveType(MOVETYPE_NONE);
      lightent2->setSolidType(SOLID_NOT);
      lightent2->edict->s.renderfx |= RF_DLIGHT;
      lightent2->edict->s.radius = 150;
      lightent2->edict->s.color_b = 0.15;
      lightent2->edict->s.color_r = 0.35;
      lightent2->edict->s.color_g = 0.35;
      lightent2->setOrigin(trace.endpos);
   }

   PostEvent(EV_Flashlight_EmitLight, 0.1);
}

void Flashlight::Pickup(Event * ev)
{
   ItemPickup(ev->GetEntity(1));
}

qboolean Flashlight::Pickupable(Entity *other)
{
   if(!other->isSubclassOf<Sentient>())
      return false;

   Sentient *sent = static_cast<Sentient *>(other);
   Item     *item = sent->FindItem(getClassname());

   if(item && (item->Amount() >= item->MaxAmount()))
      return false;

   // Mutants can't pick up anything but health
   if(other->flags & (FL_MUTANT | FL_SP_MUTANT))
      return false;

   return true;
}

Item *Flashlight::ItemPickup(Entity *other)
{
   if(!Pickupable(other))
      return nullptr;

   Sentient *sent = (Sentient *)other;
   Item     *item = sent->giveItem(getClassname(), Amount(), icon_index);

   if(!item)
      return nullptr;

   str realname = GetRandomAlias("snd_pickup");
   if(realname.length() > 1)
      sent->sound(realname, 1, CHAN_ITEM, ATTN_NORM);

   if(!Removable())
   {
      // leave the item for others to pickup
      return item;
   }

   CancelEventsOfType(EV_Item_DropToFloor);
   CancelEventsOfType(EV_Item_Respawn);
   CancelEventsOfType(EV_FadeOut);

   setSolidType(SOLID_NOT);
   hideModel();

   if(Respawnable())
      PostEvent(EV_Item_Respawn, RespawnTime());
   else
      PostEvent(EV_Remove, 0.1);

   return item;
}

// EOF

