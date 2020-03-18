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
   //### removed U4 from MFD to prevent major complications
   if(deathmatch->value == DEATHMATCH_MFD)
   {
      PostEvent(EV_Remove, 0);
      return;
   }
   //###

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

   //### can't use U4 on a bike
   if(owner->IsOnBike())
   {
      return;
   }
   //###

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

//### added to prevent people on hoverbikes from picking up the mutagen
qboolean Mutagen::Pickupable(Entity *other)
{
   if(!other->isSubclassOf<Sentient>())
   {
      return false;
   }
   else
   {
      Sentient *sent = static_cast<Sentient *>(other);

      // check if on a hoverbike
      if(sent->IsOnBike())
      {
         return false;
      }

      Item *item = sent->FindItem(getClassname());

      if(item && (item->Amount() >= item->MaxAmount()))
      {
         return false;
      }

      // Mutants can't pick up anything but health
      if(other->flags & (FL_MUTANT | FL_SP_MUTANT))
      {
         return false;
      }

      // If deathmatch and already in a powerup, don't pickup anymore when DF_INSTANT_ITEMS is on
      if(DM_FLAG(DF_INSTANT_ITEMS) &&
         this->isSubclassOf<InventoryItem>() &&
         sent->PowerupActive())
      {
         return false;
      }
   }

   return true;
}
//###

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

      item = new ScubaGear();
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

      item = new ScubaGear();
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

//### =============================================================
// 2015 added items

CLASS_DECLARATION(InventoryItem, Goggles, "powerup_goggles");

Event EV_Goggles_CheckOff("goggles_checkoff");

ResponseDef Goggles::Responses[] =
{
   { &EV_Goggles_CheckOff, (Response)&Goggles::CheckTurnoff },
   { &EV_Item_Pickup,      (Response)&Goggles::Pickup       },
   { nullptr, nullptr }
};

Goggles::Goggles() : InventoryItem()
{
   setModel("goggles.def");
   Set(1);
   setRespawnTime(60 + G_Random(30));
   goggleson = false;
   flags |= FL_POSTTHINK;
}

Goggles::~Goggles()
{
   if(glowent)
   {
      glowent->ProcessEvent(EV_Remove);
   }
}

void Goggles::CheckTurnoff(Event *ev)
{
   // if player is dead or isn't for some reason
   // set as having goggles on, make sure that
   // the goggles are off
   if(owner->isClient())
   {
      if(!(static_cast<Player *>(owner.ptr)->nightvision || owner->deadflag))
      {
         // turn the goggles off
         if(goggleson)
         {
            auto ev2 = new Event(EV_InventoryItem_Use);
            ProcessEvent(ev2);
         }
      }
   }
   else // non-client shouldn't have this
   {
      amount = 0;
      owner->RemoveItem(this);
   }
}

void Goggles::Use(Event *ev)
{
   str realname;

   assert(owner);

   // Make sure there is one available
   assert(amount);

   // only useable by clients
   if(!owner->isClient())
      return;

   // don't allow turning them off in a gogles always on DM game
   if(goggleson && !(DM_FLAG(DF_GOGGLESON)))
   {
      static_cast<Player *>(owner.ptr)->nightvision = false;
      goggleson = false;

      realname = GetRandomAlias("snd_deactivate");
   }
   else
   {
      static_cast<Player *>(owner.ptr)->nightvision = true;
      goggleson = true;

      realname = GetRandomAlias("snd_activate");
   }

   if(realname.length())
      owner->sound(realname, 0.5, CHAN_ITEM, ATTN_NORM);
}

void Goggles::Postthink()
{
   if(goggleson && !deathmatch->value && owner)
   {
      Vector dir, start, pos;
      trace_t trace;
      int mask;

      if(!glowent)
         glowent = new Entity();

      mask = (MASK_PLAYERSOLID | MASK_WATER) & ~(CONTENTS_WINDOW | CONTENTS_FENCE);
      dir = owner->orientation[0];
      start = owner->origin + Vector(0, 0, owner->viewheight);
      pos = start + dir * 50;
      trace = G_Trace(start, vec_zero, vec_zero, pos, owner, mask, "Goggles::PostThink");

      glowent->setModel("sprites/null.spr");
      glowent->setMoveType(MOVETYPE_NONE);
      glowent->setSolidType(SOLID_NOT);
      glowent->edict->s.renderfx |= RF_DLIGHT;
      glowent->edict->s.radius    = 300;
      glowent->edict->s.color_b   = 0.06;
      glowent->edict->s.color_r   = 0.06;
      glowent->edict->s.color_g   = 0.09;
      glowent->setOrigin(trace.endpos);
   }
   else if(glowent)
   {
      glowent->ProcessEvent(EV_Remove);
      glowent = nullptr;
   }
}

void Goggles::Pickup(Event * ev)
{
   ItemPickup(ev->GetEntity(1));
}

qboolean Goggles::Pickupable(Entity *other)
{
   if(!other->isSubclassOf<Sentient>())
   {
      return false;
   }

   Sentient *sent = static_cast<Sentient *>(other);
   Item     *item = sent->FindItem(getClassname());

   if(item && (item->Amount() >= item->MaxAmount()))
   {
      return false;
   }

   // Mutants can't pick up anything but health
   if(other->flags & (FL_MUTANT | FL_SP_MUTANT))
   {
      return false;
   }

   return true;
}

Item *Goggles::ItemPickup(Entity *other)
{
   if(!Pickupable(other))
   {
      return nullptr;
   }

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

CLASS_DECLARATION(InventoryItem, EasterCandy, "powerup_eastercandy");

ResponseDef EasterCandy::Responses[] =
{
   { nullptr, nullptr }
};

EasterCandy::EasterCandy() : InventoryItem()
{
   setModel("eastercandy.def");
   Set(1);
   setRespawnTime(30 + G_Random(30));
}

void EasterCandy::Use(Event *ev)
{
   assert(owner);

   // Make sure there is one available
   assert(amount);

   // the player gets 100 health when he uses this
   owner->health += 100;
   if(owner->health > 200)
      owner->health = 200;

   owner->RemoveItem(this);

   ScriptVariable *var;
   if((var= gameVars.GetVariable("playeritem_EasterCandy")))
      var->setIntValue(0);
}

CLASS_DECLARATION(InventoryItem, EasterCoke, "powerup_eastercoke");

ResponseDef EasterCoke::Responses[] =
{
   { nullptr, nullptr }
};

EasterCoke::EasterCoke() : InventoryItem()
{
   setModel("eastercoke.def");
   Set(1);
   setRespawnTime(30 + G_Random(30));
}

void EasterCoke::Use(Event *ev)
{
   assert(owner);

   // Make sure there is one available
   assert(amount);

   // the player gets 100 health when he uses this
   owner->health += 100;
   if(owner->health > 200)
      owner->health = 200;

   owner->RemoveItem(this);
   
   ScriptVariable *var;
   if((var = gameVars.GetVariable("playeritem_EasterCoke")))
      var->setIntValue(0);
}

// end 2015 added items
//### =============================================================

// EOF

