//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/health.cpp                       $
// $Revision:: 20                                                             $
//   $Author:: Aldie                                                          $
//     $Date:: 3/19/99 5:03p                                                  $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Health powerup 
// 

#include "g_local.h"
#include "item.h"
#include "sentient.h"
#include "health.h"
#include "player.h"   //###
#include "ctf.h"

CLASS_DECLARATION(Item, Health, "health_020");

ResponseDef Health::Responses[] =
{
   { &EV_Item_Pickup,      				(Response)&Health::PickupHealth },
   { NULL, NULL }
};

Health::Health() : Item()
{
   if(DM_FLAG(DF_NO_HEALTH))
   {
      PostEvent(EV_Remove, 0);
      return;
   }

   Set(20);
   if(!edict->s.modelindex)
      setModel("health.def");
}

void Health::PickupHealth(Event *ev)
{
   Sentient *sen;
   Item * item;
   Entity *other;
   int giveamount; //###

   other = ev->GetEntity(1);
   if(!other || !other->isSubclassOf<Sentient>())
   {
      return;
   }

   sen = static_cast<Sentient *>(other);

   //
   // We don't want the player to hold on to a box of health!
   // This can happen if a player is given a health object,
   // so as a precaution, get rid of any health he's carrying.
   //
   item = sen->FindItem(getClassname());
   if(item)
   {
      sen->RemoveItem(item);
      item->PostEvent(EV_Remove, 0);
   }

   if(!ItemPickup(other))
   {
      return;
   }

   //### added support for healing hoverbikes
   giveamount = amount;
   if(sen->IsOnBike())
   {
      Hoverbike *bike;

      if(sen->isSubclassOf<Player>())
      {
         bike = ((Player *)sen)->GetHoverbike();
      }
      else
      {
         bike = NULL;
      }

      if(bike)
      {
         // bike needs some health
         // bikes recieve twich the amount that players do
         if(bike->health < bike->max_health)
         {
            // give the bike all the health
            if(sen->health >= 100)
            {
               // scale the health to the bike's max
               giveamount = (int)(((float)amount)*0.01*bike->max_health + 0.5);
               bike->health += amount;
               if(bike->health > bike->max_health)
                  bike->health = bike->max_health;
               giveamount = 0;
            }
            // split it between bike and player
            else if(sen->health >= 50)
            {
               // scale the health to the bike's max
               giveamount = (int)(((float)amount)*0.005*bike->max_health + 0.5);
               bike->health += giveamount;
               if(bike->health > bike->max_health)
                  bike->health = bike->max_health;
               giveamount = (int)(((float)amount + 0.5)*0.5);
            }
            // if player has less than 50 health give it all to the player
         }
      }
   }   
   //sen->health += amount;
   sen->health += giveamount;
   //###

   //###
   if(ctf->value)
   {
      if(sen->HasItem("CTF_Tech_Regeneration"))
      {
         if(sen->health > CTF_TECH_REGENERATION_HEALTH)
         {
            sen->health = CTF_TECH_REGENERATION_HEALTH;
         }
      }
      else if(sen->HasItem("CTF_Tech_Vampire"))
      {
         if(sen->health > CTF_TECH_VAMPIRE_HEALTH)
         {
            sen->health = CTF_TECH_VAMPIRE_HEALTH;
         }
      }
      else if(sen->health > 200) // don't forget to check to regular health limit :p
      {
         sen->health = 200;
      }
   }
   //###
   else if(sen->health > 200)
   {
      sen->health = 200;
   }

   //
   // clear out damage if healed
   //
   if(sen->health > 90)
   {
      // clear the damage states
      memset(sen->edict->s.groups, 0, sizeof(sen->edict->s.groups));
   }

   //
   // we don't want the player to hold on to a box of health!
   //
   item = sen->FindItem(getClassname());
   if(item)
   {
      sen->RemoveItem(item);
      item->PostEvent(EV_Remove, 0);
   }
}

CLASS_DECLARATION(Health, SmallHealth, "health_005");

ResponseDef SmallHealth::Responses[] =
{
   { NULL, NULL }
};

SmallHealth::SmallHealth() : Health()
{
   Set(5);
   setModel("health_small.def");
}

CLASS_DECLARATION(Health, LargeHealth, "health_050");

ResponseDef LargeHealth::Responses[] =
{
   { NULL, NULL }
};

LargeHealth::LargeHealth() : Health()
{
   Set(50);
   setModel("health_large.def");
}

CLASS_DECLARATION(Health, MegaHealth, "health_100");

ResponseDef MegaHealth::Responses[] =
{
   { NULL, NULL }
};

MegaHealth::MegaHealth() : Health()
{
   Set(100);
   setModel("health_medkit.def");
}

CLASS_DECLARATION(Health, Apple, NULL);

ResponseDef Apple::Responses[] =
{
   { NULL, NULL }
};

Apple::Apple() : Health()
{
   setModel("health_apple.def");
}

CLASS_DECLARATION(Health, Banana, NULL);

ResponseDef Banana::Responses[] =
{
   { NULL, NULL }
};

Banana::Banana() : Health()
{
   setModel("health_banana.def");
}

CLASS_DECLARATION(Health, Sandwich, NULL);

ResponseDef Sandwich::Responses[] =
{
   { NULL, NULL }
};

Sandwich::Sandwich() : Health()
{
   setModel("health_sandwich.def");
}

CLASS_DECLARATION(Health, Soda, NULL);

ResponseDef Soda::Responses[] =
{
   { NULL, NULL }
};

Soda::Soda() : Health()
{
   setModel("health_soda.def");
}

//### very important 2015 added stuff ;)
CLASS_DECLARATION(Health, Poofs, NULL);

ResponseDef Poofs::Responses[] =
{
   { &EV_Item_Pickup, (Response)&Poofs::PickupPoofs },
   { NULL, NULL }
};

Poofs::Poofs() : Health()
{
}

void Poofs::PickupPoofs(Event *ev)
{
   ScriptVariable * var;

   // incriment the pickedup poofs counter
   var = gameVars.GetVariable("poofs_pickedup");
   if(!var)
   {
      gameVars.SetVariable("poofs_pickedup", 1);
   }
   else
   {
      int poofscount;

      poofscount = var->intValue() + 1;
      var->setIntValue(poofscount);
   }

   var = gameVars.GetVariable("poofs_pickedup");
   if(var)
   {
      int gotten;

      gotten = var->intValue();
      gi.dprintf("game.poofs_pickedup = %i\n", gotten);
   }

   // now call the regular health pickup function
   // to easy any suspicions that there's something up
   // with the poofs
   Health::PickupHealth(ev);
}
//###

// EOF

