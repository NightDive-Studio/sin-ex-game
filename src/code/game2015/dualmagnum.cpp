/*
================================================================
DUAL MAGNUMS
================================================================

Copyright (C) 2020 by Night Dive Studios, Inc.
All rights reserved.

See the license.txt file for conditions and terms of use for this code.

This weapon is given to the player when he picks up a second magnum
*/

#include "g_local.h"
#include "dualmagnum.h"
#include "magnum.h"

CLASS_DECLARATION(BulletWeapon, DualMagnum, NULL);

ResponseDef DualMagnum::Responses[] =
{
   {&EV_Weapon_Shoot, (Response)&DualMagnum::Shoot},
   {NULL, NULL}
};

DualMagnum::DualMagnum()
{
   SetModels("dualmagnum_w.def", "view_dualmagnum.def");
   SetAmmo("Bullet10mm", 2, 0);
   SetRank(25, 28);
   SetType(WEAPON_1HANDED);
   silenced   = true;
}

void DualMagnum::ReadyWeapon(void)
{
   str   animname;

   if(weaponstate != WEAPON_HOLSTERED)
      return;

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
      Weapon *weapon = (Weapon *)owner->FindItem("Magnum");
      if(weapon)
      {
         int amount;

         amount = weapon->ClipAmmo();
         weapon->SetAmmoAmount(0);

         amount += ClipAmmo();
         if(amount > ammo_clip_size)
            amount = ammo_clip_size;
         SetAmmoAmount(amount);

         ammosynced = true;
         ((Magnum *)weapon)->ammosynced = false;
      }
   }

   if(!HasAnim(animname.c_str()) || (deathmatch->value && ((int)dmflags->value & DF_FAST_WEAPONS)))
   {
      ProcessEvent(EV_Weapon_DoneRaising);
      return;
   }

   RandomAnimate(animname.c_str(), EV_Weapon_DoneRaising);
}

void DualMagnum::Shoot(Event *ev)
{
   NextAttack(0.2);
   FireBullets(1, Vector(10, 10, 10), 12, 24, DAMAGE_BULLET, MOD_MAGNUM, false);
}

void DualMagnum::SecondaryUse(Event *ev)
{
   //	if (weaponstate != WEAPON_READY)
   //		return;

      // make sure he has it, but only in deathmatch
   //	if(!owner->HasItem("MissileLauncher") && deathmatch->value)
   //		owner->giveWeapon("MissileLauncher");

   owner->useWeapon("Magnum");
}

qboolean DualMagnum::Drop(void)
{
   // drop a regular magnum
   auto magnum = new Magnum();
   magnum->SetOwner(owner);
   magnum->SetAmmo("Bullet10mm", 1, 0);
   magnum->BulletWeapon::Drop();

   // remove this dual magnum

   // Cancel reloading events
   //CancelEventsOfType(EV_Weapon_DoneReloading);

   // Remove this from the owner's item list
   if(owner)
      owner->RemoveItem(this);

   delete this;
   return true;
}

// EOF

