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

ResponseDef Deadbody::Responses[] =
{
   { &EV_Gib, (Response)&Deadbody::GibEvent },
   { NULL, NULL }
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
   body->entity->link();
   body->entity->SetGravityAxis(ent->entity->gravaxis);
}

void InitializeBodyQueue(void)
{
   int	   	i;
   Deadbody    *body;

   if((!LoadingSavegame) && (deathmatch->value || coop->value))
   {
      level.body_queue = 0;
      for(i=0; i<BODY_QUEUE_SIZE; i++)
      {
         body = new Deadbody();
         body->edict->owner = NULL;
         body->edict->s.skinnum = -1;
         body->flags |= (FL_DIE_GIBS|FL_BLOOD);
      }
   }
}

// EOF

