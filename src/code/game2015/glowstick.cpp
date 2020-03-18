//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/glowstick.cpp                    $
// $Revision:: 12                                                             $
//   $Author:: Aldie                                                          $
//     $Date:: 10/24/98 3:14p                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Glowstick for a lightsource

#include "inventoryitem.h" 

class EXPORT_FROM_DLL GlowStick : public InventoryItem
{
public:
   CLASS_PROTOTYPE(GlowStick);
   GlowStick();

   virtual void Use(Event *ev) override;
};

CLASS_DECLARATION(InventoryItem, GlowStick, "powerups_glowstick")

ResponseDef GlowStick::Responses[] =
{
   { &EV_InventoryItem_Use,				(Response)&GlowStick::Use },
   { nullptr, nullptr }
};

GlowStick::GlowStick() : InventoryItem()
{
#ifdef SIN_DEMO
   PostEvent(EV_Remove, 0);
   return;
#endif
   setModel("glowstick.def");
   Set(1);
}

void GlowStick::Use(Event *ev)
{
   Entity   *glowstick;
   Vector   dir;

   assert(owner);

   // Make sure there is a glowstick to 
   assert(amount);

   amount--;

   if(amount <= 0)
   {
      owner->RemoveItem(this);
   }

   dir = owner->orientation[0];

   glowstick = new Entity();

   glowstick->angles = dir.toAngles();
   glowstick->setAngles(glowstick->angles);
   glowstick->setMoveType(MOVETYPE_BOUNCE);
   glowstick->setSolidType(SOLID_NOT);
   glowstick->setModel("glowstick.def");
   glowstick->edict->s.renderfx |= RF_DLIGHT;
   glowstick->avelocity = { 500, 0, 0 };
   glowstick->velocity = dir * 500;
   glowstick->edict->s.color_r = 0.4;
   glowstick->edict->s.color_g = 1.0;
   glowstick->edict->s.color_b = 0.1;
   glowstick->edict->s.radius = 200;
   glowstick->setOrigin(owner->worldorigin + Vector(0, 0, owner->viewheight));
   glowstick->PostEvent(EV_Remove, 60);
}

// EOF
