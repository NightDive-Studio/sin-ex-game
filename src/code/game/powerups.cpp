//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/powerups.cpp                     $
// $Revision:: 24                                                             $
//   $Author:: Aldie                                                          $
//     $Date:: 3/24/99 4:30p                                                  $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Miscellaneous powerups

#include "powerups.h"
#include "specialfx.h"
#include "player.h"

#define POWERUP_TIME 30

CLASS_DECLARATION(InventoryItem, ScubaGear, "inventory_scubagear")

ResponseDef ScubaGear::Responses[] =
{
   { NULL, NULL }
};

ScubaGear::ScubaGear() : InventoryItem()
{
   setModel("scubagear.def");
}

CLASS_DECLARATION(InventoryItem, Adrenaline, "powerup_adrenaline");

Event EV_Adrenaline_Powerdown("adrenaline_powerdown");

ResponseDef Adrenaline::Responses[] =
{
   { &EV_Adrenaline_Powerdown, 		(Response)&Adrenaline::Powerdown },
   { NULL, NULL }
};

Adrenaline::Adrenaline() : InventoryItem()
{
   setModel("adren.def");
   Set(1);
   setRespawnTime(180 + G_Random(60));
}

void Adrenaline::Powerdown(Event *ev)
{
   if(!owner)
   {
      CancelPendingEvents();
      PostEvent(EV_Remove, 0);
      return;
   }

   owner->flags &= ~FL_ADRENALINE;
   owner->edict->s.renderfx &= ~RF_DLIGHT;
   owner->edict->s.color_r = 0;
   owner->edict->s.color_g = 0;
   owner->edict->s.color_b = 0;
   owner->edict->s.radius = 0;

   CancelPendingEvents();
   PostEvent(EV_Remove, 0);
}

void Adrenaline::Use(Event *ev)
{
   str         realname;
   Event       *event;

   if(!owner)
   {
      CancelPendingEvents();
      PostEvent(EV_Remove, 0);
      return;
   }

   if(owner->PowerupActive())
   {
      return;
   }

   // Make sure there is one available
   assert(amount);
   amount--;
   if(amount <= 0)
   {
      owner->RemoveItem(this);
   }

   PostEvent(EV_Adrenaline_Powerdown, POWERUP_TIME);

   owner->flags |= FL_ADRENALINE;

   event = new Event("poweruptimer");
   event->AddInteger(POWERUP_TIME);
   event->AddInteger(P_ADRENALINE);
   owner->edict->s.renderfx |= RF_DLIGHT;
   owner->edict->s.color_r = 1;
   owner->edict->s.color_g = 1;
   owner->edict->s.color_b = 0;
   owner->edict->s.radius = 120;
   owner->ProcessEvent(event);

   realname = GetRandomAlias("snd_activate");

   if(realname.length())
      owner->sound(realname, 1, CHAN_ITEM, ATTN_NORM);
}

CLASS_DECLARATION(InventoryItem, Cloak, "powerups_cloak");

Event EV_Cloak_Powerdown("Cloak_powerdown");

ResponseDef Cloak::Responses[] =
{
   { &EV_Cloak_Powerdown, 		(Response)&Cloak::Powerdown },
   { NULL, NULL }
};

Cloak::Cloak() : InventoryItem()
{
   setModel("cloak.def");
   Set(1);
   setRespawnTime(180 + G_Random(60));
}

void Cloak::Powerdown(Event *ev)
{
   if(!owner)
   {
      CancelPendingEvents();
      PostEvent(EV_Remove, 0);
      return;
   }

   if(owner->flags & FL_CLOAK)
   {
      str realname;

      owner->flags &= ~FL_CLOAK;
      realname = GetRandomAlias("snd_deactivate");
      if(realname.length())
         owner->sound(realname, 1, CHAN_ITEM, ATTN_NORM);
      owner->edict->s.renderfx &= ~RF_DLIGHT;
      owner->edict->s.color_r = 0;
      owner->edict->s.color_g = 0;
      owner->edict->s.color_b = 0;
      owner->edict->s.radius = 0;
   }
   CancelPendingEvents();
   PostEvent(EV_Remove, 0);
}

void Cloak::Use(Event *ev)
{
   str         realname;
   Event       *event;

   if(!owner)
   {
      CancelPendingEvents();
      PostEvent(EV_Remove, 0);
      return;
   }

   if(owner->PowerupActive())
   {
      return;
   }

   // Make sure there is one available
   assert(amount);
   amount--;
   if(amount <= 0)
   {
      owner->RemoveItem(this);
   }

   PostEvent(EV_Cloak_Powerdown, POWERUP_TIME);

   owner->flags |= FL_CLOAK;
   owner->edict->s.renderfx |= RF_DLIGHT;
   owner->edict->s.color_r = 1;
   owner->edict->s.color_g = 1;
   owner->edict->s.color_b = 1;
   owner->edict->s.radius = -120;

   event = new Event("poweruptimer");
   event->AddInteger(POWERUP_TIME);
   event->AddInteger(P_CLOAK);
   owner->ProcessEvent(event);

   realname = GetRandomAlias("snd_activate");

   if(realname.length())
      owner->sound(realname, 1, CHAN_ITEM, ATTN_NORM);
}

CLASS_DECLARATION(InventoryItem, Mutagen, "powerups_mutagen");

Event EV_Mutagen_Powerdown("mutagen_powerdown");

ResponseDef Mutagen::Responses[] =
{
   { &EV_Mutagen_Powerdown, 		(Response)&Mutagen::Powerdown },
   { NULL, NULL }
};

Mutagen::Mutagen() : InventoryItem()
{
   modelIndex("manumit_pl.def");
   modelIndex("view_mutanthands.def");
   setModel("u4vial.def");
   Set(1);
   setRespawnTime(180 + G_Random(60));
}

void Mutagen::Powerdown(Event *ev)
{
   str realname;

   if(!owner)
   {
      CancelPendingEvents();
      PostEvent(EV_Remove, 0);
      return;
   }

   owner->flags &= ~FL_MUTANT;
   realname = GetRandomAlias("snd_deactivate");

   if(realname.length())
      owner->sound(realname, 1, CHAN_ITEM, ATTN_NORM);

   if(owner->isClient())
   {
      int playernum = owner->edict - g_edicts - 1;

      owner->setModel(owner->savemodel);
      owner->RandomAnimate("idle", NULL);

      strcpy(owner->client->pers.model, owner->savemodel.c_str());
      strcpy(owner->client->pers.skin, owner->saveskin.c_str());

      // combine name, skin and model into a configstring
      gi.configstring(CS_PLAYERSKINS + playernum, va("%s\\%s\\%s",
                                                     owner->client->pers.netname,
                                                     owner->client->pers.model,
                                                     owner->client->pers.skin));
   }

   owner->takeWeapon("MutantHands");
   CancelPendingEvents();
   PostEvent(EV_Remove, 0);
}

void Mutagen::Use(Event *ev)
{
   str         realname;
   Event       *event;
   Weapon      *mutanthands;

   if(!owner)
   {
      CancelPendingEvents();
      PostEvent(EV_Remove, 0);
      return;
   }

   if(owner->PowerupActive())
   {
      return;
   }

   // Make sure there is one available
   assert(amount);
   amount--;

   if(amount <= 0)
   {
      owner->RemoveItem(this);
   }

   PostEvent(EV_Mutagen_Powerdown, POWERUP_TIME);
   owner->flags |= FL_MUTANT;

   //Set the timer
   event = new Event("poweruptimer");
   event->AddInteger(POWERUP_TIME);
   event->AddInteger(P_MUTAGEN);
   owner->ProcessEvent(event);

   TesselateModel
   (
      owner,
      1,
      1000,
      { 0, 0, 1 },
      50,
      0.1f,
      owner->tess_thickness,
      vec3_origin
   );

   owner->savemodel = COM_SkipPath(owner->model.c_str());
   owner->saveskin = COM_SkipPath(owner->client->pers.skin);

   owner->setModel("manumit_pl.def");
   owner->RandomAnimate("idle", NULL);

   if(owner->isClient())
   {
      int playernum = owner->edict - g_edicts - 1;

      strcpy(owner->client->pers.model, "manumit_pl.def");
      strcpy(owner->client->pers.skin, "manu_base.tga");
      // combine name, skin and model into a configstring
      gi.configstring(CS_PLAYERSKINS + playernum,
                      va("%s\\%s\\%s",
                         owner->client->pers.netname,
                         owner->client->pers.model,
                         owner->client->pers.skin)
      );

      FixDeadBodiesForPlayer(owner->edict);
   }

   // Give the mutanthands weapon. - Force it.
   mutanthands = owner->giveWeapon("MutantHands");
   owner->ForceChangeWeapon(mutanthands);

   realname = GetRandomAlias("snd_activate");

   if(realname.length())
      owner->sound(realname, 1, CHAN_ITEM, ATTN_NORM);
}

/*****************************************************************************/
/*SINED misc_medkit (1.0 0.2 0.2)
  Heals whoever uses it to their max health.
/*****************************************************************************/
CLASS_DECLARATION(Entity, Medkit, "misc_medkit");

ResponseDef Medkit::Responses[] =
{
   { &EV_Use, 		         (Response)&Medkit::Use },
   { NULL, NULL }
};

Medkit::Medkit() : Entity()
{
   setSolidType(SOLID_BSP);
   setMoveType(MOVETYPE_NONE);
   showModel();
}

void Medkit::Use(Event *ev)
{
   Entity      *other;
   str         realname;

   other = ev->GetEntity(1);
   if(other && other->health < other->max_health)
   {
      other->health = other->max_health;
      realname = GetRandomAlias("snd_activate");

      if(realname.length())
         other->sound(realname, 1, CHAN_ITEM, ATTN_NORM);
   }
}

CLASS_DECLARATION(InventoryItem, Oxygen, "powerups_oxygen");

Event EV_Oxygen_Powerdown("oxygen_powerdown");

ResponseDef Oxygen::Responses[] =
{
   { &EV_Oxygen_Powerdown, 		(Response)&Oxygen::Powerdown },
   { &EV_Item_Pickup,      		(Response)&Oxygen::Pickup },
   { NULL, NULL }
};

Oxygen::Oxygen() : InventoryItem()
{
   setModel("oxygen.def");
   Set(1);
   setRespawnTime(180 + G_Random(60));
}

void Oxygen::Powerdown(Event *ev)
{
   if(!owner)
   {
      CancelPendingEvents();
      PostEvent(EV_Remove, 0);
      return;
   }

   if(owner->flags & FL_OXYGEN)
   {
      str realname;

      owner->flags &= ~FL_OXYGEN;
      realname = GetRandomAlias("snd_deactivate");
      if(realname.length())
         owner->sound(realname, 1, CHAN_ITEM, ATTN_NORM);
   }
   CancelPendingEvents();
   PostEvent(EV_Remove, 0);
}

void Oxygen::Pickup(Event *ev)
{
   Entity   *other;
   Sentient *sen;
   Item     *item;

   other = ev->GetEntity(1);

   if(!other)
   {
      CancelPendingEvents();
      PostEvent(EV_Remove, 0);
      return;
   }

   sen = (Sentient *)other;

   // Single player must have the scuba gear to use oxygen
   if(!deathmatch->value && !sen->FindItem("ScubaGear"))
   {
      Item *item;

      item = (Item *)new ScubaGear;
      item->CancelEventsOfType(EV_Item_DropToFloor);
      item->CancelEventsOfType(EV_Remove);
      item->ProcessPendingEvents();

      gi.centerprintf(other->edict, "jcx yv 20 string \"You need this item to use Oxygen:\" jcx yv -20 icon %d", item->GetIconIndex());
      delete item;

      return;
   }

   if(!ItemPickup(sen))
      return;

   if(sen->isSubclassOf<Player>())
   {
      Player *player = static_cast<Player *>(sen);

      player->GiveOxygen(60);
   }

   item = sen->FindItem(getClassname());
   if(item)
   {
      CancelPendingEvents();
      item->PostEvent(EV_Remove, 0);
   }
}

void Oxygen::Use(Event *ev)
{
   str         realname;
   Event       *event;

   if(!owner)
   {
      CancelPendingEvents();
      PostEvent(EV_Remove, 0);
      return;
   }

   // Single player must have the scuba gear to use oxygen
   if(!deathmatch->value && !owner->FindItem("ScubaGear"))
   {
      Item *item;

      item = (Item *)new ScubaGear;
      item->CancelEventsOfType(EV_Item_DropToFloor);
      item->CancelEventsOfType(EV_Remove);
      item->ProcessPendingEvents();

      gi.centerprintf(owner->edict, "jcx yv 20 string \"You need this item to use Oxygen:\" jcx yv -20 icon %d", item->GetIconIndex());
      delete item;

      return;
   }

   if(owner->PowerupActive())
   {
      return;
   }

   // Make sure there is one available
   assert(amount);
   amount--;
   if(amount <= 0)
   {
      owner->RemoveItem(this);
   }

   PostEvent(EV_Oxygen_Powerdown, 60);

   owner->flags |= FL_OXYGEN;

   event = new Event("poweruptimer");
   event->AddInteger(60);
   event->AddInteger(P_OXYGEN);
   owner->ProcessEvent(event);

   realname = GetRandomAlias("snd_activate");

   if(realname.length())
      owner->sound(realname, 1, CHAN_ITEM, ATTN_NORM);
}

// EOF

