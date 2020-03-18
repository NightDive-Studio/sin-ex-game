//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/keys.cpp                        $
// $Revision:: 24                                                             $
//   $Author:: Markd                                                          $
//     $Date:: 11/15/98 4:30p                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Access cards and keys

#include "inventoryitem.h" 
#include "player.h"

class EXPORT_FROM_DLL BlueCard : public InventoryItem
{
public:
   CLASS_PROTOTYPE(BlueCard);
   BlueCard();
};

CLASS_DECLARATION(InventoryItem, BlueCard, "inventory_bluecard")

ResponseDef BlueCard::Responses[] =
{
   { NULL, NULL }
};

BlueCard::BlueCard() : InventoryItem()
{
   setModel("card_blu.def");
}

class EXPORT_FROM_DLL OrangeCard : public InventoryItem
{
public:
   CLASS_PROTOTYPE(OrangeCard);
   OrangeCard();
};

CLASS_DECLARATION(InventoryItem, OrangeCard, "inventory_orangecard")

ResponseDef OrangeCard::Responses[] =
{
   { NULL, NULL }
};

OrangeCard::OrangeCard() : InventoryItem()
{
   setModel("card_orng.def");
}

class EXPORT_FROM_DLL YellowCard : public InventoryItem
{
public:
   CLASS_PROTOTYPE(YellowCard);
   YellowCard();
};

CLASS_DECLARATION(InventoryItem, YellowCard, "inventory_yellowcard")

ResponseDef YellowCard::Responses[] =
{
   { NULL, NULL }
};

YellowCard::YellowCard() : InventoryItem()
{
   setModel("card_yel.def");
}

class EXPORT_FROM_DLL GreenCard : public InventoryItem
{
public:
   CLASS_PROTOTYPE(GreenCard);
   GreenCard();
};

CLASS_DECLARATION(InventoryItem, GreenCard, "inventory_greencard")

ResponseDef GreenCard::Responses[] =
{
   { NULL, NULL }
};

GreenCard::GreenCard() : InventoryItem()
{
   setModel("card_grn.def");
}

class EXPORT_FROM_DLL IdentCard : public InventoryItem
{
public:
   CLASS_PROTOTYPE(IdentCard);
   IdentCard();
};

CLASS_DECLARATION(InventoryItem, IdentCard, "inventory_identcard")

ResponseDef IdentCard::Responses[] =
{
   { NULL, NULL }
};

IdentCard::IdentCard() : InventoryItem()
{
   setModel("identcard.def");
}

class EXPORT_FROM_DLL Cookies : public InventoryItem
{
public:
   CLASS_PROTOTYPE(Cookies);
   Cookies();
};

CLASS_DECLARATION(InventoryItem, Cookies, "inventory_cookies")

ResponseDef Cookies::Responses[] =
{
   { NULL, NULL }
};

Cookies::Cookies() : InventoryItem()
{
   setModel("cookies.def");
}

class EXPORT_FROM_DLL ComLink : public InventoryItem
{
public:
   CLASS_PROTOTYPE(ComLink);
   ComLink();
};

CLASS_DECLARATION(InventoryItem, ComLink, "inventory_comlink")

ResponseDef ComLink::Responses[] =
{
   { NULL, NULL }
};

ComLink::ComLink() : InventoryItem()
{
   setModel("comlink.def");
}

class EXPORT_FROM_DLL Coin : public InventoryItem
{
public:
   CLASS_PROTOTYPE(Coin);
   Coin();
};

CLASS_DECLARATION(InventoryItem, Coin, "inventory_coin")

ResponseDef Coin::Responses[] =
{
   { NULL, NULL }
};

Coin::Coin() : InventoryItem()
{
   setModel("coin.def");
}

class EXPORT_FROM_DLL Code : public InventoryItem
{
public:
   CLASS_PROTOTYPE(Code);
   Code();
};

CLASS_DECLARATION(InventoryItem, Code, "inventory_code")

ResponseDef Code::Responses[] =
{
   { NULL, NULL }
};

Code::Code() : InventoryItem()
{
   setModel("code.def");
}

class EXPORT_FROM_DLL KeyRing : public InventoryItem
{
public:
   CLASS_PROTOTYPE(KeyRing);
   KeyRing();
};

CLASS_DECLARATION(InventoryItem, KeyRing, "inventory_keyring")

ResponseDef KeyRing::Responses[] =
{
   { NULL, NULL }
};

KeyRing::KeyRing() : InventoryItem()
{
   setModel("keys.def");
}

class EXPORT_FROM_DLL MoneyBag : public InventoryItem
{
public:
   CLASS_PROTOTYPE(MoneyBag);
   MoneyBag();
};

CLASS_DECLARATION(InventoryItem, MoneyBag, "inventory_moneybag")

ResponseDef MoneyBag::Responses[] =
{
   { NULL, NULL }
};

MoneyBag::MoneyBag() : InventoryItem()
{
   setModel("moneybag_inv.def");
}

class EXPORT_FROM_DLL Dollar : public InventoryItem
{
public:
   CLASS_PROTOTYPE(Dollar);
   Dollar();
};

CLASS_DECLARATION(InventoryItem, Dollar, "inventory_dollar")

ResponseDef Dollar::Responses[] =
{
   { NULL, NULL }
};

Dollar::Dollar() : InventoryItem()
{
   setModel("dollar.def");
}

class EXPORT_FROM_DLL Evidence : public InventoryItem
{
public:
   CLASS_PROTOTYPE(Evidence);
   Evidence();
};

CLASS_DECLARATION(InventoryItem, Evidence, "inventory_evidence")

ResponseDef Evidence::Responses[] =
{
   { NULL, NULL }
};

Evidence::Evidence() : InventoryItem()
{
   setModel("evidence.def");
}

class EXPORT_FROM_DLL Decoder : public InventoryItem
{
public:
   CLASS_PROTOTYPE(Decoder);
   Decoder();
};

CLASS_DECLARATION(InventoryItem, Decoder, "inventory_decoder")

ResponseDef Decoder::Responses[] =
{
   { NULL, NULL }
};

Decoder::Decoder() : InventoryItem()
{
   setModel("code.def");
}

class EXPORT_FROM_DLL PulsePart1 : public InventoryItem
{
public:
   CLASS_PROTOTYPE(PulsePart1);
   PulsePart1();
   virtual Item   *ItemPickup(Entity *other);
};

CLASS_DECLARATION(InventoryItem, PulsePart1, "inventory_pulsepart1")

ResponseDef PulsePart1::Responses[] =
{
   { NULL, NULL }
};

PulsePart1::PulsePart1() : InventoryItem()
{
   ExecuteThread("global/pulse_parts.scr::precache", true);
   setModel("pulsepart1.def");
}

Item *PulsePart1::ItemPickup(Entity *other)
{
   if(!level.no_jc)
      ExecuteThread("global/pulse_parts.scr::blade_finds_piece1", true);
   gameVars.CreateVariable("pulse1", 1);
   if(other->isClient())
   {
      Player *player;
      ScriptVariable	*var1, *var2, *var3;

      var1 = gameVars.GetVariable("pulse1");
      var2 = gameVars.GetVariable("pulse2");
      var3 = gameVars.GetVariable("pulse3");

      if(var1 && var2 && var3)
      {
         player = (Player *)other;
         player->giveWeapon("PulseRifle");
         player->FreeInventoryOfType("PulsePart2");
         player->FreeInventoryOfType("PulsePart3");
         CancelEventsOfType(EV_Item_DropToFloor);
         CancelEventsOfType(EV_Item_Respawn);
         CancelEventsOfType(EV_FadeOut);
         PostEvent(EV_Remove, 0);
         return NULL;
      }
   }
   return Item::ItemPickup(other);
}

class EXPORT_FROM_DLL PulsePart2 : public InventoryItem
{
public:
   CLASS_PROTOTYPE(PulsePart2);
   PulsePart2();
   virtual Item   *ItemPickup(Entity *other);
};

CLASS_DECLARATION(InventoryItem, PulsePart2, "inventory_pulsepart2")

ResponseDef PulsePart2::Responses[] =
{
   { NULL, NULL }
};

PulsePart2::PulsePart2() : InventoryItem()
{
   ExecuteThread("global/pulse_parts.scr::precache", true);
   setModel("pulsepart2.def");
}

Item *PulsePart2::ItemPickup(Entity *other)
{
   if(!level.no_jc)
      ExecuteThread("global/pulse_parts.scr::blade_finds_piece2", true);
   gameVars.CreateVariable("pulse2", 1);
   if(other->isClient())
   {
      Player *player;
      ScriptVariable	*var1, *var2, *var3;

      var1 = gameVars.GetVariable("pulse1");
      var2 = gameVars.GetVariable("pulse2");
      var3 = gameVars.GetVariable("pulse3");

      if(var1 && var2 && var3)
      {
         player = (Player *)other;
         player->giveWeapon("PulseRifle");
         player->FreeInventoryOfType("PulsePart1");
         player->FreeInventoryOfType("PulsePart3");
         CancelEventsOfType(EV_Item_DropToFloor);
         CancelEventsOfType(EV_Item_Respawn);
         CancelEventsOfType(EV_FadeOut);
         PostEvent(EV_Remove, 0);
         return NULL;
      }
   }
   return Item::ItemPickup(other);
}

class EXPORT_FROM_DLL PulsePart3 : public InventoryItem
{
public:
   CLASS_PROTOTYPE(PulsePart3);
   PulsePart3();
   virtual Item   *ItemPickup(Entity *other);
};

CLASS_DECLARATION(InventoryItem, PulsePart3, "inventory_pulsepart3")

ResponseDef PulsePart3::Responses[] =
{
   { NULL, NULL }
};

PulsePart3::PulsePart3() : InventoryItem()
{
   ExecuteThread("global/pulse_parts.scr::precache", true);
   setModel("pulsepart3.def");
}

Item *PulsePart3::ItemPickup(Entity *other)
{
   if(!level.no_jc)
      ExecuteThread("global/pulse_parts.scr::blade_finds_piece3", true);
   gameVars.CreateVariable("pulse3", 1);
   if(other->isClient())
   {
      Player *player;
      ScriptVariable	*var1, *var2, *var3;

      var1 = gameVars.GetVariable("pulse1");
      var2 = gameVars.GetVariable("pulse2");
      var3 = gameVars.GetVariable("pulse3");

      if(var1 && var2 && var3)
      {
         player = (Player *)other;
         player->giveWeapon("PulseRifle");
         player->FreeInventoryOfType("PulsePart1");
         player->FreeInventoryOfType("PulsePart2");
         CancelEventsOfType(EV_Item_DropToFloor);
         CancelEventsOfType(EV_Item_Respawn);
         CancelEventsOfType(EV_FadeOut);
         PostEvent(EV_Remove, 0);
         return NULL;
      }
   }
   return Item::ItemPickup(other);
}

class EXPORT_FROM_DLL Chemsuit : public InventoryItem
{
public:
   CLASS_PROTOTYPE(Chemsuit);
   Chemsuit();
};

CLASS_DECLARATION(InventoryItem, Chemsuit, "inventory_chembiosuit")

ResponseDef Chemsuit::Responses[] =
{
   { NULL, NULL }
};

Chemsuit::Chemsuit() : InventoryItem()
{
   setModel("chemsuit.def");
}

class EXPORT_FROM_DLL Blueprints : public InventoryItem
{
public:
   CLASS_PROTOTYPE(Blueprints);
   Blueprints();
};

CLASS_DECLARATION(InventoryItem, Blueprints, "inventory_blueprints")

ResponseDef Blueprints::Responses[] =
{
   { NULL, NULL }
};

Blueprints::Blueprints() : InventoryItem()
{
   setModel("blueprints.def");
}

class EXPORT_FROM_DLL U4Sample : public InventoryItem
{
public:
   CLASS_PROTOTYPE(U4Sample);
   U4Sample();
};

CLASS_DECLARATION(InventoryItem, U4Sample, "inventory_u4sample")

ResponseDef U4Sample::Responses[] =
{
   { NULL, NULL }
};

U4Sample::U4Sample() : InventoryItem()
{
   setModel("u4_sample.def");
}

class EXPORT_FROM_DLL Envelope : public InventoryItem
{
public:
   CLASS_PROTOTYPE(Envelope);
   Envelope();
};

CLASS_DECLARATION(InventoryItem, Envelope, "inventory_envelope")

ResponseDef Envelope::Responses[] =
{
   { NULL, NULL }
};

Envelope::Envelope() : InventoryItem()
{
   setModel("envelope.def");
}

class EXPORT_FROM_DLL CandyBar : public InventoryItem
{
public:
   CLASS_PROTOTYPE(CandyBar);
   CandyBar();
};

CLASS_DECLARATION(InventoryItem, CandyBar, "inventory_candybar")

ResponseDef CandyBar::Responses[] =
{
   { NULL, NULL }
};

CandyBar::CandyBar() : InventoryItem()
{
   setModel("CandyBar.def");
}

class EXPORT_FROM_DLL PassCode : public InventoryItem
{
public:
   CLASS_PROTOTYPE(PassCode);
   PassCode();
};

CLASS_DECLARATION(InventoryItem, PassCode, "inventory_passcode")

ResponseDef PassCode::Responses[] =
{
   { NULL, NULL }
};

PassCode::PassCode() : InventoryItem()
{
   setModel("password.def");
}

class EXPORT_FROM_DLL PassCode2 : public InventoryItem
{
public:
   CLASS_PROTOTYPE(PassCode2);
   PassCode2();
};

CLASS_DECLARATION(InventoryItem, PassCode2, "inventory_passcode2")

ResponseDef PassCode2::Responses[] =
{
   { NULL, NULL }
};

PassCode2::PassCode2() : InventoryItem()
{
   setModel("password2.def");
}

class EXPORT_FROM_DLL Hand : public InventoryItem
{
public:
   CLASS_PROTOTYPE(Hand);
   Hand();
};

CLASS_DECLARATION(InventoryItem, Hand, "inventory_hand")

ResponseDef Hand::Responses[] =
{
   { NULL, NULL }
};

Hand::Hand() : InventoryItem()
{
   setModel("hand.def");
}

/*SINED inventory_genericpulsepart (.3 1 .3) (-8 -8 0) (8 8 16) NOT_SOLID NOT_DAMAGABLE
Pulse Weapon Part - Will always spawn as the next piece that the user needs
to complete the weapon.  When the user picks up 3 of these, they will get the PulseRifle
*/

class EXPORT_FROM_DLL GenericPulsePart : public InventoryItem
{
public:
   CLASS_PROTOTYPE(GenericPulsePart);
   GenericPulsePart();
};

CLASS_DECLARATION(InventoryItem, GenericPulsePart, "inventory_genericpulsepart")

ResponseDef GenericPulsePart::Responses[] =
{
   { NULL, NULL }
};

GenericPulsePart::GenericPulsePart() : InventoryItem()
{
   ScriptVariable	*var1, *var2, *var3;

   var1 = gameVars.GetVariable("pulse1");
   var2 = gameVars.GetVariable("pulse2");
   var3 = gameVars.GetVariable("pulse3");

   // Based on what pulserifle pieces that have already been found,
   // spawn the correct model in the game.

   if(!var1)
   {
      PulsePart1 *part1;

      part1 = new PulsePart1();
      part1->setModel("pulsepart1.def");
      part1->setOrigin(origin);
      part1->worldorigin.copyTo(part1->edict->s.old_origin);
   }
   else if(!var2)
   {
      PulsePart2 *part2;

      part2 = new PulsePart2();
      part2->setModel("pulsepart2.def");
      part2->setOrigin(origin);
      part2->worldorigin.copyTo(part2->edict->s.old_origin);
   }
   else if(!var3)
   {
      PulsePart3 *part3;

      part3 = new PulsePart3();
      part3->setModel("pulsepart3.def");
      part3->setOrigin(origin);
      part3->worldorigin.copyTo(part3->edict->s.old_origin);
   }

   PostEvent(EV_Remove, 0);
}

// EOF

