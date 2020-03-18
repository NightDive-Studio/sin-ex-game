//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/deadbody.cpp                     $
// $Revision:: 13                                                             $
//   $Author:: Jimdose                                                        $
//     $Date:: 11/19/98 9:28p                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Dead body

#include "deadbody.h"
#include "gibs.h"

CLASS_DECLARATION(Sentient, Deadbody, "deadbody");

//###
Event EV_Body_StopBurning("deadbody_stopburning");
Event EV_Body_Fade("deadbody_fade");
//###

ResponseDef Deadbody::Responses[] =
{
   { &EV_Gib,              (Response)&Deadbody::GibEvent    },
   //###
   { &EV_Body_StopBurning, (Response)&Deadbody::StopBurning },
   { &EV_Body_Fade,       (Response)&Deadbody::BodyFade     },
   //###
   { nullptr, nullptr }
};

void Deadbody::GibEvent(Event *ev)
{
   takedamage = DAMAGE_NO;

   if(sv_gibs->value && !parentmode->value)
   {
      setSolidType(SOLID_NOT);
      hideModel();

      CreateGibs(this, health, 1.0f, 3);
   }
}

void CopyToBodyQueue(edict_t *ent)
{
   edict_t *body;

   // grab a body from the queue and cycle to the next one
   body = &g_edicts[(int)maxclients->value + level.body_queue + 1];
   level.body_queue = (level.body_queue + 1) % BODY_QUEUE_SIZE;

   gi.unlinkentity(ent);
   gi.unlinkentity(body);

   body->s                    = ent->s;
   body->s.number             = body - g_edicts;
   body->svflags              = ent->svflags;
   body->solid                = ent->solid;
   body->clipmask             = ent->clipmask;
   body->owner                = ent->owner;
   body->entity->movetype     = ent->entity->movetype;
   body->entity->takedamage   = DAMAGE_YES;
   body->entity->deadflag     = DEAD_DEAD;
   body->s.renderfx           &= ~RF_DONTDRAW;
   body->entity->origin       = ent->entity->worldorigin;
   body->entity->setSize(ent->mins, ent->maxs);
   //### make sure to copy over death from flames stuff
   if(ent->s.effects & EF_DEATHFLAMES)
      body->s.effects |= EF_DEATHFLAMES;

   body->s.effects &= ~EF_WARM;
   //###
   body->entity->link();
   body->entity->SetGravityAxis(ent->entity->gravaxis);

   //### added dead body fading option
   body->entity->CancelEventsOfType(EV_Body_Fade);
   if(level.bodiesfade)
   {
      body->entity->PostEvent(EV_Body_Fade, 10);
   }
   //###
}

void InitializeBodyQueue()
{
   int	   	i;
   Deadbody    *body;

   if((!LoadingSavegame) && (deathmatch->value || coop->value))
   {
      level.body_queue = 0;
      for(i=0; i<BODY_QUEUE_SIZE; i++)
      {
         body = new Deadbody();
         body->edict->owner = nullptr;
         body->edict->s.skinnum = -1;
         body->flags |= (FL_DIE_GIBS|FL_BLOOD);
      }
   }
}

//### added for deaths from the flamethrower
void Deadbody::StopBurning(Event *ev)
{
   edict->s.effects &= ~EF_FLAMES;
   // make sure it doesn't have a heat signature
   edict->s.effects &= ~EF_WARM;
}

void Deadbody::BodyFade(Event *ev)
{
   PostEvent(EV_Body_Fade, 0.1f);

   edict->s.renderfx |= RF_TRANSLUCENT;
   translucence += 0.03f;
   if(translucence >= 0.96f)
   {
      edict->s.renderfx &= ~RF_TRANSLUCENT;
      takedamage = DAMAGE_NO;
      setSolidType(SOLID_NOT);
      hideModel();
   }
   else
   {
      setAlpha(1.0f - translucence);
   }
}
//###

// EOF

