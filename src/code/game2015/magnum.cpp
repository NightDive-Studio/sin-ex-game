//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/magnum.cpp                       $
// $Revision:: 46                                                             $
//   $Author:: Markd                                                          $
//     $Date:: 11/13/98 3:30p                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Magnum.
// 

#include "g_local.h"
#include "magnum.h"
#include "dualmagnum.h" //###

CLASS_DECLARATION(BulletWeapon, Magnum, "weapon_magnum");

ResponseDef Magnum::Responses[] =
{
   { &EV_Weapon_Shoot, (Response)&Magnum::Shoot },
   { nullptr, nullptr }
};

Magnum::Magnum() : BulletWeapon()
{
   SetModels("magnum.def", "view_magnum.def");
   SetAmmo("Bullet10mm", 1, 100);
   SetRank(20, 20);
   SetType(WEAPON_1HANDED);
   modelIndex("10mm.def");
   silenced = true;
   //###
   // precaches for the dual magnum
   modelIndex("dualmagnum_w.def");
   modelIndex("view_dualmagnum.def");
   //###
}

void Magnum::Shoot(Event *ev)
{
   NextAttack(0.20);
   FireBullets(1, { 10, 10, 10 }, 12, 24, DAMAGE_BULLET, MOD_MAGNUM, false);
}

//###
void Magnum::ReadyWeapon()
{
   str animname;

   if(weaponstate != WEAPON_HOLSTERED)
   {
      return;
   }

   weaponstate = WEAPON_RAISING;

   AttachGun();

   if((owner) && (owner->flags & FL_SILENCER) && (silenced))
      animname = "silready";
   else
   {
      if(weaponmode == SECONDARY)
         animname = "secondaryready";
      else
         animname = "ready";

   }

   // added dual magnum ammo syncing
   if(!ammosynced)
   {
      Weapon *weapon;

      weapon = static_cast<Weapon *>(owner->FindItem("DualMagnum"));
      if(weapon)
      {
         int amount;

         amount = weapon->ClipAmmo() * 0.5;
         if(amount > ammo_clip_size)
            amount = ammo_clip_size;
         weapon->SetAmmoAmount(weapon->ClipAmmo() - amount);

         SetAmmoAmount(amount);

         ammosynced = true;
         static_cast<DualMagnum *>(weapon)->ammosynced = false;
      }
   }

   if(!HasAnim(animname.c_str()) || (deathmatch->value && ((int)dmflags->value & DF_FAST_WEAPONS)))
   {
      ProcessEvent(EV_Weapon_DoneRaising);
      return;
   }

   RandomAnimate(animname.c_str(), EV_Weapon_DoneRaising);
}

void Magnum::SecondaryUse(Event *ev)
{
   // switch to the dual magnums
   owner->useWeapon("DualMagnum");
}
//###

qboolean Magnum::Drop()
{
   //### drop magnums now so people can get dual magnums
#if 0
   // Don't leave magnums around
   if(owner && owner->deadflag && deathmatch->value)
   {
      return FALSE;
   }

   return BulletWeapon::Drop();
#endif

   Weapon *weapon;
   Sentient *sent;

   const ClassDef *cls;
   Item *item;

   if(!owner)
   {
      return false;
   }

   if(!IsDroppable())
   {
      return false;
   }

   // check if player had two magnums
   weapon = static_cast<Weapon *>(owner->FindItem("DualMagnum"));

   if(!weapon) // only had one magnum, so drop it
   {
      return BulletWeapon::Drop();
   }

   // has two magnums

   sent = static_cast<Sentient *>(owner.ptr);

   // get rid of secondary magnum
   sent->takeWeapon("DualMagnum");

   // drop this magnum
   BulletWeapon::Drop();

   // give the player a new magnum since he had two
   item = static_cast<Item *>(sent->FindItem("Magnum"));

   if(!weapon)
   {
      cls = getClass("Magnum");
      if(!cls)
      {
         // somethin's majorly f'ed to get here :P
         gi.dprintf("No item named 'Magnum'\n");
         return true;
      }

      item = static_cast<Item *>(cls->newInstance());

      item->SetOwner(sent);

      item->ProcessPendingEvents();

      item->Set(1);
      sent->AddItem(item);
   }
   else
   {
      // for some reason the player still had a magnum
      gi.dprintf("Player still had magnum after dropping it\n");
   }

   return true;
   //###
}

// EOF

