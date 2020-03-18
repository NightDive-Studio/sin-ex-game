//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/sentient.cpp                     $
// $Revision:: 198                                                            $
//   $Author:: Aldie                                                          $
//     $Date:: 4/16/99 5:00p                                                  $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Base class of entity that can carry other entities, and use weapons.
//

#include "g_local.h"
#include "entity.h"
#include "sentient.h"
#include "weapon.h"
#include "scriptmaster.h"
#include "ammo.h"
#include "armor.h"
#include "misc.h"
#include "inventoryitem.h"
#include "player.h"
#include "actor.h"
#include "ctf.h"

CLASS_DECLARATION(Entity, Sentient, NULL);

Event EV_Sentient_Attack("fire");
Event EV_Sentient_ReleaseAttack("releasefire");
Event EV_Sentient_GiveWeapon("weapon");
Event EV_Sentient_GiveAmmo("ammo");
Event EV_Sentient_GiveArmor("armor");
Event EV_Sentient_GiveItem("item");
Event EV_Sentient_GiveTargetname("give");
Event EV_Sentient_GiveInventoryItem("invitem");
Event EV_Sentient_GiveHealth("health", EV_CHEAT);
Event EV_Sentient_TakeWeapon("take_weapon");
Event EV_Sentient_TakeAmmo("take_ammo");
Event EV_Sentient_TakeArmor("take_armor");
Event EV_Sentient_TakeItem("take_item");
Event EV_Sentient_WeaponPutAway("weapon_putaway");
Event EV_Sentient_WeaponReady("weapon_ready");
Event EV_Sentient_WeaponDoneFiring("weapon_donefiring");
Event EV_Sentient_AnimLoop("animloop");
Event EV_Sentient_UselessCheck("useless_check");
Event EV_Sentient_TurnOffShadow("noshadow");
Event EV_Sentient_Freeze("freeze");
Event EV_Sentient_UnFreeze("unfreeze");
Event EV_Sentient_ImpactDamage("impact_damage");
Event EV_Sentient_WeaponUse("weaponuse", EV_CONSOLE);
Event EV_Sentient_SetDropWeapon("dropweapon");
Event EV_Sentient_DropWeaponNow("dropweaponnow");

ResponseDef Sentient::Responses[] =
{
   { &EV_Sentient_Attack,				(Response)&Sentient::FireWeapon },
   { &EV_Sentient_ReleaseAttack,		(Response)&Sentient::ReleaseFireWeapon },
   { &EV_Sentient_GiveWeapon,			(Response)&Sentient::EventGiveWeapon },
   { &EV_Sentient_TakeWeapon,			(Response)&Sentient::EventTakeWeapon },
   { &EV_Sentient_GiveAmmo,			(Response)&Sentient::EventGiveAmmo },
   { &EV_Sentient_TakeAmmo,			(Response)&Sentient::EventTakeAmmo },
   { &EV_Sentient_GiveArmor,			(Response)&Sentient::EventGiveArmor },
   { &EV_Sentient_TakeArmor,			(Response)&Sentient::EventTakeArmor },
   { &EV_Sentient_GiveItem,         (Response)&Sentient::EventGiveItem },
   { &EV_Sentient_GiveInventoryItem,(Response)&Sentient::EventGiveInventoryItem },
   { &EV_Sentient_GiveHealth,       (Response)&Sentient::EventGiveHealth },
   { &EV_Sentient_GiveTargetname,   (Response)&Sentient::EventGiveTargetname },
   { &EV_Sentient_WeaponPutAway,		(Response)&Sentient::WeaponPutAway },
   { &EV_Sentient_WeaponReady,		(Response)&Sentient::WeaponReady },
   { &EV_Sentient_WeaponDoneFiring,	(Response)&Sentient::WeaponDoneFiring },
   { &EV_Sentient_AnimLoop,			(Response)&Sentient::AnimLoop },
   { &EV_Damage,                    (Response)&Sentient::ArmorDamage },
   { &EV_Touch,                     (Response)&Sentient::SearchBody },
   { &EV_Sentient_UselessCheck,		(Response)&Sentient::UselessCheck },
   { &EV_Sentient_TurnOffShadow,		(Response)&Sentient::TurnOffShadow },
   { &EV_Sentient_Freeze,		      (Response)&Sentient::Freeze },
   { &EV_Sentient_UnFreeze,	      (Response)&Sentient::UnFreeze },
   { &EV_Sentient_ImpactDamage,     (Response)&Sentient::ImpactDamage },
   { &EV_Sentient_WeaponUse,		   (Response)&Sentient::WeaponUse },
   { &EV_Sentient_SetDropWeapon,	   (Response)&Sentient::SetDropWeapon },
   { &EV_Sentient_DropWeaponNow,	   (Response)&Sentient::DropWeaponNowEvent },
   { &EV_Sentient_TakeItem,	      (Response)&Sentient::EventTakeItem },
   { NULL, NULL }
};

Container<Sentient *> SentientList;

Sentient::Sentient() : Entity()
{
   Vector defangles;
   Sentient *self = this;

   SentientList.AddObject(self);

   inventory.ClearObjectList();
   currentWeapon = nullptr;
   currentItem   = nullptr;
   newWeapon     = nullptr;
   animOverride  = false;
   dropweapon    = true;
   tempAnimEvent = nullptr;

   eyeposition = { 0, 0, 64 };
   gunoffset   = { 0, 0, 56 };

   firedown     = false;
   firedowntime = 0;

   // angles
   defangles = Vector(0, G_GetFloatArg("angle", 0), 0);
   if(defangles.y == -1)
   {
      defangles = Vector(-90, 0, 0);
   }
   else if(defangles.y == -2)
   {
      defangles = Vector(90, 0, 0);
   }
   angles = G_GetVectorArg("angles", defangles);
   setAngles(angles);
   stopanimating_tillchange = false;

   poweruptype = 0;
   poweruptimer = 0;
   sentientFrozen = false;

   // HACK FIXME
   //
   // temporary shadow flag
   //
   edict->s.renderfx |= RF_XFLIP;
}

Sentient::~Sentient()
{
    Sentient *self = this;
    SentientList.RemoveObject(self);
    FreeInventory();
}

Vector Sentient::EyePosition(void)
{
   Vector pos;

   pos = worldorigin;
   pos[gravity_axis[gravaxis].x] += eyeposition[0];
   pos[gravity_axis[gravaxis].y] += eyeposition[1] * gravity_axis[gravaxis].sign;
   pos[gravity_axis[gravaxis].z] += eyeposition[2] * gravity_axis[gravaxis].sign;

   return pos;
}

Vector Sentient::GunPosition(void)
{
   Vector pos;

   pos = worldorigin;
   pos[gravity_axis[gravaxis].x] += gunoffset[0];
   pos[gravity_axis[gravaxis].y] += gunoffset[1] * gravity_axis[gravaxis].sign;
   pos[gravity_axis[gravaxis].z] += gunoffset[2] * gravity_axis[gravaxis].sign;

   return pos;
}

inline void Sentient::GetGunOrientation(Vector pos, Vector *forward, Vector *right, Vector *up)
{
   const gravityaxis_t &grav = gravity_axis[gravaxis];

   if(forward)
   {
      (*forward)[grav.x] = orientation[0][0];
      (*forward)[grav.y] = orientation[0][1] * grav.sign;
      (*forward)[grav.z] = orientation[0][2] * grav.sign;
   }

   if(right)
   {
      (*right)[grav.x] = -orientation[1][0];
      (*right)[grav.y] = -orientation[1][1] * grav.sign;
      (*right)[grav.z] = -orientation[1][2] * grav.sign;
   }

   if(up)
   {
      (*up)[grav.x] = orientation[2][0];
      (*up)[grav.y] = orientation[2][1] * grav.sign;
      (*up)[grav.z] = orientation[2][2] * grav.sign;
   }
}

void Sentient::GetMuzzlePositionAndDirection(Vector *pos, Vector *dir)
{
   *pos = { 0, 0, 0 };
   *dir = { 0, 0, 0 };

   if(currentWeapon)
      currentWeapon->GetMuzzlePosition(pos, dir);
}

void Sentient::EventGiveHealth(Event *ev)
{
   int group_num;

   health = ev->GetFloat(1);

   if(isClient())
   {
      for(group_num = 0; group_num < edict->s.numgroups; group_num++)
      {
         edict->s.groups[group_num] &= ~MDL_GROUP_SKINOFFSET_BIT0;
      }
   }
}

void Sentient::FireWeapon(Event *ev)
{
   if((currentWeapon) && currentWeapon->ReadyToFire() && currentWeapon->HasAmmo())
   {
      currentWeapon->Fire();
   }
}

void Sentient::ReleaseFireWeapon(Event *ev)
{
   // Only need to check if currentWeapon because FireWeapon checks for
   // the weapon ready and if it has ammo.
   if(currentWeapon)
   {
      float fireholdtime = ev->GetFloat(1);
      currentWeapon->ReleaseFire(fireholdtime);
   }
}

void Sentient::AddItem(Item *object)
{
   inventory.AddObject(object->entnum);

   if(object->isSubclassOf<InventoryItem>())
   {
      currentItem = static_cast<InventoryItem *>(object);
   }
   //
   // set our global game variables if client
   //
   if(isClient())
   {
      str fullname;
      ScriptVariable * var;

      fullname = str("playeritem_") + object->getClassname();
      var = gameVars.GetVariable(fullname.c_str());
      if(!var)
      {
         gameVars.SetVariable(fullname.c_str(), 1);
      }
      else
      {
         int amount;

         amount = var->intValue() + 1;
         var->setIntValue(amount);
      }

      var = levelVars.GetVariable(fullname.c_str());
      if(!var)
      {
         levelVars.SetVariable(fullname.c_str(), 1);
      }
      else
      {
         int amount;

         amount = var->intValue() + 1;
         var->setIntValue(amount);
      }
   }
}

void Sentient::RemoveItem(Item *object)
{
   Weapon *weapon;

   inventory.RemoveObject(object->entnum);
   if(currentWeapon == object)
   {
      currentWeapon = NULL;
      if(newWeapon == object)
      {
         newWeapon = NULL;
         weapon = BestWeapon();
      }
      else
      {
         weapon = newWeapon;
      }

      SetCurrentWeapon(weapon);
   }

   //
   // set our global game variables if client
   //
   if(isClient())
   {
      str fullname;
      ScriptVariable * var;

      fullname = str("playeritem_") + object->getClassname();

      var = levelVars.GetVariable(fullname.c_str());
      if(var)
      {
         int amount;

         amount = var->intValue() - 1;
         if(amount < 0)
            amount = 0;
         var->setIntValue(amount);
      }
   }

   if(currentItem == (InventoryItem *)object)
   {
      currentItem = NULL;
      // Try to set the the current item

      currentItem = (InventoryItem *)NextItem(NULL);
      if(!currentItem)
      {
         currentItem = (InventoryItem *)PrevItem(NULL);
      }
   }
}

Item *Sentient::FindItem(const char *itemname)
{
   int	num;
   int	i;
   Item	*item;

   num = inventory.NumObjects();
   for(i = 1; i <= num; i++)
   {
      item = (Item *)G_GetEntity(inventory.ObjectAt(i));
      if(!Q_stricmp(item->getClassname(), itemname))
      {
         return item;
      }
   }

   return NULL;
}

Item *Sentient::HasItemOfSuperclass(const char *superclassname)
{
   int	num;
   int	i;
   Item	*item;

   num = inventory.NumObjects();
   for(i = 1; i <= num; i++)
   {
      item = (Item *)G_GetEntity(inventory.ObjectAt(i));
      if(!Q_stricmp(item->getSuperclass(), superclassname))
      {
         return item;
      }
   }

   return NULL;
}

void Sentient::FreeInventory(void)
{
   int	num;
   int	i;
   Item	*item;

   if(currentWeapon)
      currentWeapon->DetachFromOwner();

   SetCurrentWeapon(NULL);
   newWeapon = NULL;

   num = inventory.NumObjects();
   for(i = num; i > 0; i--)
   {
      item = (Item *)G_GetEntity(inventory.ObjectAt(i));
      delete item;
   }

   inventory.ClearObjectList();
   currentItem = NULL;
}

void Sentient::FreeInventoryOfType(const char *type)
{
   int	num;
   int	i;
   Item	*item;

   num = inventory.NumObjects();
   for(i = num; i > 0; i--)
   {
      item = (Item *)G_GetEntity(inventory.ObjectAt(i));
      if(checkInheritance(type, item->getClassname()))
      {
         inventory.RemoveObject(item->entnum);

         // If the current item is about to be deleted, then look for
         // another item to be current
         if(currentItem == item)
         {
            currentItem = NULL;
            // Try to set the the current item

            currentItem = (InventoryItem *)NextItem(NULL);
            if(!currentItem)
            {
               currentItem = (InventoryItem *)PrevItem(NULL);
            }
         }
         delete item;
      }
   }
}

qboolean Sentient::HasItem(const char *itemname)
{
   return (FindItem(itemname) != NULL);
}

int Sentient::NumWeapons(void)
{
   int	num;
   int	i;
   Item	*item;
   int   numweaps;

   numweaps = 0;

   num = inventory.NumObjects();
   for(i = 1; i <= num; i++)
   {
      item = (Item *)G_GetEntity(inventory.ObjectAt(i));
      if(checkInheritance(&Weapon::ClassInfo, item->getClassname()))
      {
         numweaps++;
      }
   }

   return numweaps;
}

Weapon *Sentient::WeaponNumber(int weaponnum)
{
   int	num;
   int	i;
   Item	*item;

   // make it so that 0 is our first weapon rather than 1
   weaponnum++;
   num = inventory.NumObjects();
   for(i = 1; i <= num; i++)
   {
      item = (Item *)G_GetEntity(inventory.ObjectAt(i));
      if(item->isSubclassOf<Weapon>())
      {
         weaponnum--;
         if(!weaponnum)
         {
            return (Weapon *)item;
         }
      }
   }

   return NULL;
}

void Sentient::ChangeWeapon(Weapon *weapon)
{
   if(weapon == currentWeapon)
   {
      return;
   }

   if(currentWeapon)
   {
      if(!weapon->ReadyToUse())
      {
         return;
      }
      newWeapon = weapon;
      if(!currentWeapon->ReadyToChange())
      {
         return;
      }
      currentWeapon->PutAway();
   }
   else
   {
      SetCurrentWeapon(weapon);
   }
}

void Sentient::ForceChangeWeapon(Weapon *weapon)
{
   if(newWeapon == weapon)
   {
      return;
   }

   if(weapon == currentWeapon)
   {
      return;
   }

   newWeapon = NULL;

   if(currentWeapon)
   {
      currentWeapon->DetachFromOwner();
   }
   SetCurrentWeapon(weapon);
}

qboolean Sentient::CanChangeWeapons(void)
{
   return !((newWeapon != NULL) ||
            (currentWeapon && !currentWeapon->ReadyToChange()) ||
            (flags & FL_MUTANT));
}

void Sentient::SetCurrentWeapon(Weapon *weapon)
{
   const char *name;

   newWeapon = NULL;
   currentWeapon = weapon;
   if(currentWeapon)
   {
      currentWeapon->ReadyWeapon();

      // get the name of the group that the gun bone belongs to
      name = gi.GetBoneGroupName(edict->s.modelindex, "gun");
      if(name)
      {
         gun_bone_group_name = name;
      }
      else
      {
         gun_bone_group_name = "";
      }
   }
}

Weapon *Sentient::CurrentWeapon(void)
{
   return currentWeapon;
}

Weapon *Sentient::BestWeapon(Weapon *ignore)
{
   Item		*next;
   int		n;
   int		j;
   int		bestrank;
   Weapon	*bestweapon;

   n = inventory.NumObjects();

   // Search forewards until we find a weapon
   bestweapon = NULL;
   bestrank = -999999;
   for(j = 1; j <= n; j++)
   {
      next = (Item *)G_GetEntity(inventory.ObjectAt(j));

      assert(next);

      if((next != ignore) && next->isSubclassOf<Weapon>() && (((Weapon *)next)->Rank() > bestrank) &&
         (((Weapon *)next)->HasAmmo()))
      {
         bestweapon = (Weapon *)next;
         bestrank = bestweapon->Rank();
      }
   }

   return bestweapon;
}

Weapon *Sentient::NextWeapon(Weapon *weapon)
{
   Item		*item;
   int		i;
   int		n;
   int		weaponorder;
   Weapon	*choice;
   int		choiceorder;
   Weapon	*bestchoice;
   int		bestorder;
   Weapon	*worstchoice;
   int		worstorder;

   if(!inventory.ObjectInList(weapon->entnum))
   {
      error("NextWeapon", "Weapon not in list");
   }

   weaponorder = weapon->Order();
   bestchoice = weapon;
   bestorder = 65535;
   worstchoice = weapon;
   worstorder = weaponorder;

   n = inventory.NumObjects();
   for(i = 1; i <= n; i++)
   {
      item = (Item *)G_GetEntity(inventory.ObjectAt(i));

      assert(item);

      if(item->isSubclassOf<Weapon>())
      {
         choice = (Weapon *)item;
         if(!choice->HasAmmo() || !choice->AutoChange())
         {
            continue;
         }

         choiceorder = choice->Order();
         if((choiceorder > weaponorder) && (choiceorder < bestorder))
         {
            bestorder = choiceorder;
            bestchoice = choice;
         }

         if(choiceorder < worstorder)
         {
            worstorder = choiceorder;
            worstchoice = choice;
         }
      }
   }

   if(bestchoice == weapon)
   {
      return worstchoice;
   }

   return bestchoice;
}

Weapon *Sentient::PreviousWeapon(Weapon *weapon)
{
   Item		*item;
   int		i;
   int		n;
   int		weaponorder;
   Weapon	*choice;
   int		choiceorder;
   Weapon	*bestchoice;
   int		bestorder;
   Weapon	*worstchoice;
   int		worstorder;

   if(!inventory.ObjectInList(weapon->entnum))
   {
      error("PreviousWeapon", "Weapon not in list");
   }

   weaponorder = weapon->Order();
   bestchoice = weapon;
   bestorder = -65535;
   worstchoice = weapon;
   worstorder = weaponorder;

   n = inventory.NumObjects();
   for(i = 1; i <= n; i++)
   {
      item = (Item *)G_GetEntity(inventory.ObjectAt(i));

      assert(item);

      if(item->isSubclassOf<Weapon>())
      {
         choice = static_cast<Weapon *>(item);
         if(!choice->HasAmmo() || !choice->AutoChange())
         {
            continue;
         }

         choiceorder = choice->Order();
         if((choiceorder < weaponorder) && (choiceorder > bestorder))
         {
            bestorder = choiceorder;
            bestchoice = choice;
         }

         if(choiceorder > worstorder)
         {
            worstorder = choiceorder;
            worstchoice = choice;
         }
      }
   }

   if(bestchoice == weapon)
   {
      return worstchoice;
   }

   return bestchoice;
}

qboolean Sentient::WeaponReady(void)
{
   Weapon *best;

   if(!currentWeapon)
   {
      return false;
   }

   if(!currentWeapon->ReadyToUse())
   {
      best = BestWeapon();
      if(best)
      {
         ChangeWeapon(best);
      }
      return false;
   }

   if(!currentWeapon->ReadyToFire())
   {
      return false;
   }

   if(!currentWeapon->HasAmmo())
   {
      best = BestWeapon();
      if(best)
      {
         ChangeWeapon(best);
      }
      return false;
   }

   return true;
}

void Sentient::DropWeapon(Weapon *weapon)
{
   if(weapon == currentWeapon)
   {
      if(deadflag)
      {
         SetCurrentWeapon(NULL);
      }
      else
      {
         // Choose the best weapon that's not the current
         SetCurrentWeapon(BestWeapon(currentWeapon));
      }
   }

   // check if the sentient can drop the weapon
   if(dropweapon)
   {
      // This one checks if the weapon is droppable
      if(!weapon->Drop() && deadflag)
      {
         // can't drop it, probably because there's no worldmodel
         // since we're dead, just delete the weapon.
         weapon->PostEvent(EV_Remove, 0);
      }
   }
   else
   {
      weapon->PostEvent(EV_Remove, 0);
   }
}

void Sentient::DropCurrentWeapon(void)
{
   if(currentWeapon)
      DropWeapon(currentWeapon);
}

void Sentient::takeWeapon(const char *weaponname)
{
   Weapon *weapon;

   assert(weaponname);

   weapon = (Weapon *)FindItem(weaponname);
   if(!weapon)
   {
      return;
   }

   if(weapon == newWeapon)
   {
      newWeapon = NULL;
   }

   if(weapon == currentWeapon)
   {
      SetCurrentWeapon(BestWeapon(weapon));
   }

   weapon->DetachFromOwner();
   weapon->PostEvent(EV_Remove, 0);
}

void Sentient::EventGiveTargetname(Event *ev)
{
   str      name;
   Item     *item = NULL;
   edict_t  *from;

   name = ev->GetString(1);

   for(from = &g_edicts[1]; from < &g_edicts[globals.num_edicts]; from++)
   {
      if(!from->inuse)
         continue;

      if(from->entity->isSubclassOf<Item>())
      {
         item = static_cast<Item *>(from->entity);
         if(item->itemname == name)
            break;
      }
   }

   // Item should be found
   assert(item);

   if(item)
   {
      item->SetOwner(this);
      item->ProcessPendingEvents();
      AddItem(item);
   }
   else
   {
      ev->Error("Could not give item %s to %s.\n", name, targetname);
   }
}

Item *Sentient::giveItem(const char * itemname, int amount, int icon_index)
{
   const ClassDef *cls;
   Item           *item;
   int             index;

   item = (Item *)FindItem(itemname);

   if(item)
   {
      item->Add(amount);
   }
   else
   {
      cls = getClass(itemname);
      if(!cls)
      {
         gi.dprintf("No item named '%s'\n", itemname);
         return NULL;
      }

      item = (Item *)cls->newInstance();

      item->SetOwner(this);

      item->ProcessPendingEvents();

      item->Set(amount);
      AddItem(item);
   }

   // Send icon to the player
   if(this->isSubclassOf<Player>())
   {
      if(icon_index >= 0)
         index = icon_index;
      else
         index = item->Icon();

      if(index >= 0)
      {
         gi.WriteByte(svc_console_command);
         gi.WriteString(va("pl %d %d", index, amount));
         gi.unicast(edict, NULL);
      }
   }
   return item;
}

void Sentient::takeItem(const char *itemname, int amount)
{
   Item        *item;

   item = (Item *)FindItem(itemname);

   if(item)
   {
      item->Remove(amount);
      if(!item->Amount())
         item->ProcessEvent(EV_Remove);
   }
}

void Sentient::EventTakeItem(Event *ev)
{
   int amount;

   if(ev->NumArgs() > 1)
      amount = ev->GetInteger(2);
   else
      amount = 1;
   takeItem(ev->GetString(1), amount);
}

Weapon *Sentient::giveWeapon(const char *weaponname)
{
   Weapon *weapon;

   assert(weaponname);

   if(!checkInheritance(&Weapon::ClassInfo, weaponname))
   {
      gi.dprintf("A %s is not usable as a weapon\n", weaponname);
      return NULL;
   }

   weapon = (Weapon *)giveItem(weaponname, 1);

   if(!currentWeapon || (weapon->Rank() > currentWeapon->Rank()))
   {
      ChangeWeapon(weapon);
   }

   return weapon;
}

Weapon *Sentient::useWeapon(const char *weaponname)
{
   Weapon *weapon;

   assert(weaponname);

   weapon = (Weapon *)FindItem(weaponname);
   if(weapon)
   {
      ChangeWeapon(weapon);
   }

   return currentWeapon;
}

void Sentient::EventGiveWeapon(Event *ev)
{
   ChangeWeapon(giveWeapon(ev->GetString(1)));
}

void Sentient::EventTakeWeapon(Event *ev)
{
   takeWeapon(ev->GetString(1));
}

void Sentient::EventTakeAmmo(Event *ev)
{
   int			amount;
   const char	*type;
   Ammo			*ammo;
   Weapon		*best;

   type = ev->GetString(1);
   amount = ev->GetInteger(2);

   ammo = (Ammo *)FindItem(type);
   if(ammo)
   {
      ammo->Remove(amount);
   }

   if(currentWeapon && !currentWeapon->HasAmmo())
   {
      best = BestWeapon();
      if(best)
      {
         ChangeWeapon(best);
      }
   }
}

void Sentient::EventGiveAmmo(Event *ev)
{
   int			amount;
   const char	*type;

   type = ev->GetString(1);
   amount = ev->GetInteger(2);

   if(!checkInheritance(&Ammo::ClassInfo, type))
   {
      gi.dprintf("%s is not usable as ammo\n", type);
      return;
   }

   giveItem(type, amount);

   if(currentWeapon && !currentWeapon->HasAmmo())
   {
      Weapon		*best;
      best = BestWeapon();
      if(best)
      {
         ChangeWeapon(best);
      }
   }
}

void Sentient::EventTakeArmor(Event *ev)
{
   int			amount;
   const char	*type;
   Armor			*armor;

   type = ev->GetString(1);
   amount = ev->GetInteger(2);

   armor = (Armor *)FindItem(type);
   if(armor)
   {
      armor->Remove(amount);
   }
}

void Sentient::EventGiveArmor(Event *ev)
{
   int			amount;
   const char	*type;
   Armor			*armor;

   type = ev->GetString(1);
   amount = ev->GetInteger(2);

   if(!checkInheritance(&Armor::ClassInfo, type))
   {
      gi.dprintf("%s is not usable as armor\n", type);
      return;
   }

   armor = (Armor *)FindItem(type);

   if(armor)
   {
      armor->Set(amount);
   }
   else
   {
      giveItem(type, amount);
   }
}

void Sentient::EventGiveItem(Event *ev)
{
   const char	*type;
   float       amount;

   type = ev->GetString(1);
   amount = ev->GetInteger(2);

   giveItem(type, amount);
}

void Sentient::EventGiveInventoryItem(Event *ev)
{
   const char	         *type;
   int                  amount;

   type = ev->GetString(1);
   amount = ev->GetInteger(2);

   if(!checkInheritance(&InventoryItem::ClassInfo, type))
   {
      gi.dprintf("item '%s' is not a InventoryItem\n", type);
      return;
   }

   giveItem(type, amount);
}

void Sentient::WeaponPutAway(Event *ev)
{
   Weapon *best;

   if(newWeapon)
   {
      SetCurrentWeapon(newWeapon);
   }
   else
   {
      best = BestWeapon();
      if(best)
      {
         SetCurrentWeapon(best);
      }
   }
}

void Sentient::WeaponReady(Event *ev)
{
   if(newWeapon)
   {
      ChangeWeapon(newWeapon);
   }
}

void Sentient::WeaponDoneFiring(Event *ev)
{
   Weapon *best;

   if(newWeapon)
   {
      ChangeWeapon(newWeapon);
   }
   else if(!currentWeapon || !currentWeapon->HasAmmo() || !currentWeapon->ReadyToUse())
   {
      best = BestWeapon();
      if(best)
      {
         ChangeWeapon(best);
      }
   }
}

EXPORT_FROM_DLL void Sentient::AnimLoop(Event *ev)
{
   Event *t;

   animOverride = false;

   if(deadflag)
   {
      StopAnimating();
   }
   else if(animating)
   {
      RandomAnimate(currentAnim.c_str(), NULL);
   }
   else
   {
      stopanimating_tillchange = true;
   }

   if(tempAnimEvent)
   {
      t = tempAnimEvent;
      tempAnimEvent = NULL;
      ProcessEvent(t);
   }
}

EXPORT_FROM_DLL void Sentient::SetAnim(const char *anim)
{
   assert(anim);

   assert(!deadflag);

   if(str(anim) == currentAnim)
   {
      if(!animating && !stopanimating_tillchange)
         StartAnimating();
      return;
   }

   stopanimating_tillchange = false;
   currentAnim = str(anim);

   if(animOverride)
   {
      return;
   }

   RandomAnimate(currentAnim.c_str(), EV_Sentient_AnimLoop);

   if(!animating)
      StartAnimating();
}

EXPORT_FROM_DLL void Sentient::TempAnim(const char *anim, Event *event)
{
   assert(!deadflag);

   assert(anim);

   animOverride = true;

   tempAnimEvent = event;

   RandomAnimate(anim, EV_Sentient_AnimLoop);
}

void Sentient::TempAnim(const char *anim, Event &event)
{
   Event *ev;

   ev = new Event(event);
   TempAnim(anim, ev);
}

void Sentient::PrintDamageLocationToAttacker(edict_s *attacker, const char *victim_name, const char *location)
{
   const char *expanded_location;

   expanded_location = ExpandLocation(location);

   if(expanded_location)
      gi.cprintf(attacker, PRINT_MEDIUM, "You hit %s in the %s.\n", victim_name, expanded_location);
}

void Sentient::PrintDamageLocationToVictim(edict_s *victim, const char *location)
{
   const char *expanded_location;

   expanded_location = ExpandLocation(location);

   if(expanded_location)
      gi.cprintf(victim, PRINT_MEDIUM, "%s damage\n", expanded_location);
}

qboolean Sentient::DoGib(int meansofdeath, Entity *inflictor)
{
   if(!(flags & FL_DIE_GIBS) || parentmode->value)
   {
      return false;
   }

   if((meansofdeath == MOD_TELEFRAG) ||
      (meansofdeath == MOD_MUTANTHANDS) ||
      (meansofdeath == MOD_HELIGUN) ||
      (meansofdeath == MOD_CTFTURRET) ||
      (meansofdeath == MOD_LAVA) ||
      (meansofdeath == MOD_SNIPER) ||
      ((meansofdeath == MOD_FISTS) && (inflictor->flags & FL_ADRENALINE)) ||
      ((meansofdeath == MOD_SHOTGUN) && (G_Random() < 0.5)))
   {
      return true;
   }

   if(health > -75)
   {
      return false;
   }

   // Impact and Crush < -75 health
   if((meansofdeath == MOD_IMPACT) ||
      (meansofdeath == MOD_CRUSH)  ||
      (meansofdeath == MOD_VEHICLE)
      )
   {
      return true;
   }

   // Any projectiles except spear
   if(inflictor->isSubclassOf<Projectile>())
   {
      if(meansofdeath != MOD_SPEARGUN)
      {
         return true;
      }
   }

   // Shotgun only 10% of the time
   if(meansofdeath == MOD_SHOTGUN)
   {
      if(G_Random() > 0.5)
      {
         return true;
      }
   }

   return false;
}

void Sentient::ArmorDamage(Event *ev)
{
   Armor    *armor = 0;
   Entity	*inflictor;
   Entity	*attacker;
   float		damage;
   float    armor_damage = 0;
   Vector	momentum;
   Vector   position;
   Vector   normal;
   Vector   direction;
   Event		*event;
   int      dflags;
   int      meansofdeath;
   int      knockback;
   float    damage_mult;
   int      groupnum;
   int      trinum;
   const char *location = "";
   float    unmodified_damage = 0;

   if((takedamage == DAMAGE_NO) || (movetype == MOVETYPE_NOCLIP))
   {
      if(isClient())
      {
         Player *player;

         player = (Player *)this;
         if(!player->GetVehicle())
            return;
      }
      else
      {
         return;
      }
   }

   damage       = ev->GetFloat(1);
   inflictor    = ev->GetEntity(2);
   attacker     = ev->GetEntity(3);
   position     = ev->GetVector(4);
   direction    = ev->GetVector(5);
   normal       = ev->GetVector(6);
   knockback    = ev->GetInteger(7);
   dflags       = ev->GetInteger(8);
   meansofdeath = ev->GetInteger(9);
   groupnum     = ev->GetInteger(10);
   trinum       = ev->GetInteger(11);
   damage_mult  = ev->GetFloat(12);

   // Make sure attacker is there
   if(!attacker)
      return;

   // Forcefields make objects invulnerable
   if(flags & FL_FORCEFIELD)
   {
      float    alpha;
      float    radius;
      Vector   delta;
      Vector   org;
      Entity   *forcefield;
      //
      // spawn forcefield
      //
      forcefield = new Entity();

      delta = centroid - worldorigin;
      radius = delta.length();

      //
      // transform the centroid around by the current orientation
      //
      MatrixTransformVector(delta.vec3(), orientation, org.vec3());
      org += worldorigin;

      forcefield->setModel("sphere2.def");
      forcefield->setOrigin(org);
      forcefield->worldorigin.copyTo(forcefield->edict->s.old_origin);
      forcefield->setMoveType(MOVETYPE_NONE);
      forcefield->setSolidType(SOLID_NOT);
      forcefield->edict->s.scale = radius / 16;
      alpha = (damage * damage_mult) / 100;
      if(alpha > 1)
         alpha = 1;
      if(alpha < 0.15f)
         alpha = 0.15f;
      forcefield->edict->s.alpha = alpha;
      forcefield->edict->s.renderfx |= RF_TRANSLUCENT;
      forcefield->PostEvent(EV_Remove, 0.1f);
      return;
   }

   // if enabled you can't hurt teammates (but you can hurt yourself)
   // knockback still occurs
   if((attacker != this) && (DM_FLAG(DF_MODELTEAMS | DF_SKINTEAMS) || ctf->value))
   {
      if(OnSameTeam(this, attacker))
      {
         // check for friendly fire
         if(DM_FLAG(DF_FRIENDLY_FIRE) || (dflags & DAMAGE_NO_PROTECTION))
            meansofdeath = MOD_FRIENDLY_FIRE;
         else
            damage = 0;
      }
   }

   if(groupnum == -1)
      location = "all";
   else
      location = gi.Group_NumToName(edict->s.modelindex, groupnum);

   // Adjust damage for skill level
   if(isClient() && !deathmatch->value && !(dflags & DAMAGE_NO_SKILL))
   {
      // Clamp the damage multiplier
      if(damage_mult > 2)
         damage_mult = 2;

      switch((int)skill->value)
      {
      case 0:
         damage *= 0.2f;
         break;
      case 1:
         damage *= 0.4f;
         break;
      case 2:
         damage *= 0.6f;
         break;
      default:
         damage *= 0.8f;
         break;
      }
   }

   // Shields reduce damage by 75%
   if(flags & FL_SHIELDS)
   {
      damage *= 0.25f;
   }
   else if(flags & FL_MUTANT)
   {
      damage *= 0.33f;
   }

   // If in deathmatch, client takes a minimum of 33% damage from source
   if(deathmatch->value)
   {
      damage *= 0.66f;
      unmodified_damage = damage * 0.33f;
   }

   // check to see if we have armor in this location to reduce the damage
   if(location[0] && !(dflags & DAMAGE_NO_ARMOR))
   {
      if(!strncmp(location, "all", 3))
      {
         char *armortypes[] = { "RiotHelmet", "FlakJacket", "FlakPants", NULL };
         int i = 0;
         float reduced_damage = 0;

         // Go through all the armor types
         while(armortypes[i])
         {
            float partial_damage = damage / 3;
            float odam = partial_damage;

            armor = (Armor *)FindItem(armortypes[i]);

            // Check for armor in this spot
            if(armor && (armor->Amount() > 0))
            {
               // Reduce the parital damage by the amount of armor
               partial_damage -= armor->Amount();

               if(partial_damage < 0)
                  partial_damage = 0;

               // Remove the amount absorbed by the armor from the armor.
               armor->Remove(odam - partial_damage);

               // Keep track of the damage done to the armor for testing purposes
               armor_damage += (odam - partial_damage);
            }
            // Keep track of damage done after armor has been applied
            reduced_damage += partial_damage;
            i++;
         }
         // This is the damage after all armor is done
         damage = reduced_damage;
      }
      // else check for specific location based
      else if(!strncmp(location, "head", 4))
         armor = (Armor *)FindItem("RiotHelmet");
      else if(!strncmp(location, "torso", 4))
         armor = (Armor *)FindItem("FlakJacket");
      //      else if ( !strncmp( location,"arm",3 ) )
      //         armor = ( Armor * )FindItem( "FlakJacket" );
      else if(!strncmp(location, "leg", 3))
         armor = (Armor *)FindItem("FlakPants");

      // If armor is there, remove the appropriate amount
      if(armor && (armor->Amount() > 0) && !(dflags & DAMAGE_NO_ARMOR))
      {
         float odam = damage;
         damage -= armor->Amount();

         if(damage < 0)
            damage = 0;

         armor->Remove(odam - damage);
         armor_damage += (odam - damage);
      }
   }

   damage += unmodified_damage;

   // Damage multiplier
   damage *= damage_mult;

   if(ctf->value)
   {
      // Check to see if the attacker had double
      if(attacker->isClient())
      {
         Player *pl_attacker;

         pl_attacker = (Player *)(Sentient *)attacker;
         if(pl_attacker->HasItem("CTF_Tech_Double")) // DOUBLE
            damage *= 2;
         else if(pl_attacker->HasItem("CTF_Tech_DeathQuad")) // QUAD
            damage *= 4;
      }

      // Check to see if the client has the shield or empathy
      if(this->isClient())
      {
         Player *pl_defender;
         Event *ctfev;

         pl_defender = (Player *)this;

         if(pl_defender->HasItem("CTF_Tech_Half") && !(dflags & DAMAGE_NO_ARMOR))
         {
            damage *= 0.3f;

            ctfev = new Event(EV_Player_CTF_SoundEvent);
            ctfev->AddInteger(DAMAGE);
            pl_defender->ProcessEvent(ctfev);
         }
         else if(pl_defender->HasItem("CTF_Tech_Empathy"))  // EMPATHY
         {
            ctfev = new Event(EV_Player_CTF_SoundEvent);
            ctfev->AddInteger(DAMAGE);
            pl_defender->ProcessEvent(ctfev);

            // Send the damage back to the attacker if it is a client
            if((attacker != this) && attacker->isClient())
            {
               Event *ev2;

               ev2 = new Event(EV_Damage);
               ev2->AddInteger(damage);
               ev2->AddEntity(this);
               ev2->AddEntity(this);
               ev2->AddVector(vec_zero);
               ev2->AddVector(vec_zero);
               ev2->AddVector(vec_zero);
               ev2->AddInteger(0);
               ev2->AddInteger(DAMAGE_NO_ARMOR);
               ev2->AddInteger(MOD_EMPATHY);
               ev2->AddInteger(-1);
               ev2->AddInteger(-1);
               ev2->AddFloat(1);
               attacker->PostEvent(ev2, 0);
               //attacker->Damage( this, this, damage, vec_zero, vec_zero, vec_zero, 0, DAMAGE_NO_ARMOR, MOD_EMPATHY, -1, -1, 1.0f );
            }
         }
      }
   }

   // Damage skins if armor didn't absorb all damage
   if((damage > 0) && (groupnum != -1) && (flags & FL_BLOOD))
   {
      if(!edict->s.groups[groupnum])
         edict->s.groups[groupnum]++;
   }

   // Show location based damage.
   if(sv_showdamagelocation->value && deathmatch->value)
   {
      // Send message to attacker
      if(attacker->isClient())
      {
         if(this->client)
         {
            PrintDamageLocationToAttacker(attacker->edict, this->client->pers.netname, location);
         }
      }

      // Send message to victim
      if(this->isClient() && attacker != this)
      {
         PrintDamageLocationToVictim(this->edict, location);
      }
   }

   // Shows detailed damage messages 
   if(sv_showdamage->value)
   {
      // Send message to attacker
      if(attacker->isClient())
      {
         if(this->client)
         {
            gi.cprintf(attacker->edict, PRINT_HIGH, "TARG:%s ARMOR_DAM:%0.2f TOT_DAM:%0.2f LOC:%s\n",
                       this->client->pers.netname, armor_damage, damage, location);
         }
         else
         {
            gi.cprintf(attacker->edict, PRINT_HIGH, "TARG:%s ARMOR_DAM:%0.2f TOT_DAM:%0.2f LOC:%s\n",
                       getClassname(), armor_damage, damage, location);
         }
      }

      // Send message to victim
      if(this->isClient() && attacker != this)
      {
         gi.cprintf(this->edict, PRINT_HIGH, "TARG:%s ARMOR_DAM:%0.2f TOT_DAM:%0.2f LOC:%s\n",
                    this->client->pers.netname, armor_damage, damage, location);
      }
   }


   // Blood and sparks effects
   if((damage > 0) &&
      (meansofdeath != MOD_DROWN) &&
      (meansofdeath != MOD_MUTANT_DRAIN))
   {
      // Blood particles
      if(flags & FL_BLOOD)
      {
         Particles(position, normal, __min(150, damage), 127, PARTICLE_RANDOM);

         // Blood splat
         if(dflags & DAMAGE_BULLET)
            SprayBlood(position, direction, damage);
      }
      // Sparks from metal
      else if(flags & FL_SPARKS)
      {
         SpawnSparks(position, normal, __min(damage, 75));
      }
   }
   else if(armor_damage > 0)
   {
      // Sparks off armor
      Particles(position, normal, __min(armor_damage, 75), 122, 0);
   }

   // Gib if we are dead and get hit by a rocket or shotgun
   if(deadflag == DEAD_DEAD)
   {
      health -= damage;
      if(DoGib(meansofdeath, inflictor))
      {
         Event *gibEv;

         gibEv = new Event(EV_Gib);
         gibEv->AddInteger(0);
         ProcessEvent(gibEv);
      }
      return;
   }
   else if(deadflag)
   {
      health -= damage;
      return;
   }

   // Do the kick
   if(!(dflags & DAMAGE_NO_KNOCKBACK))
   {
      if((knockback) &&
         (movetype != MOVETYPE_NONE) &&
         (movetype != MOVETYPE_BOUNCE) &&
         (movetype != MOVETYPE_PUSH) &&
         (movetype != MOVETYPE_STOP))
      {
         float	m;

         if(mass < 50)
            m = 50;
         else
            m = mass;

         direction.normalize();
         if(isClient() && (attacker == this) && deathmatch->value)
            momentum = direction * (2100.0f * (float)knockback / m); // the rocket jump hack...
         else
            momentum = direction * (500.0f * (float)knockback / m);

         if(dflags & DAMAGE_BULLET)
         {
            // Clip the z velocity for bullet weapons
            if(momentum.z > 75)
               momentum.z = 75;
         }
         velocity += momentum;
      }
   }

   // check for godmode or invincibility
   if(flags & FL_GODMODE)
   {
      return;
   }

   // do the damage
   health -= damage;

   if(ctf->value)
      CTF_CheckHurtCarrier(this, attacker);

   // He's dead jim, send out a bunch of events
   if(health <= 0)
   {
      if((meansofdeath == MOD_ION) && !(flags & FL_NOION))
      {
         flags |= FL_DIE_TESSELATE;
         RandomGlobalSound("snd_tesselate");
      }

      event = new Event(EV_Killed);
      event->AddEntity(attacker);
      event->AddInteger(damage);
      event->AddEntity(inflictor);
      event->AddString(location);
      event->AddInteger(meansofdeath);
      ProcessEvent(event);
      return;
   }

   if((damage > 0) && (meansofdeath == MOD_ION) && !(flags & FL_NOION))
   {
      // Do the ion particlizer effect
      TesselateModel
      (
         this,
         1,
         1000,
         direction,
         damage,
         1.0f,
         0,
         vec3_origin,
         TESSELATE_EXPANDANDSHRINK,
         126
      );
   }

   // Tesselate from damage
   if(flags & FL_TESSELATE)
   {
      TesselateModel
      (
         this,
         tess_min_size,
         tess_max_size,
         direction,
         damage,
         1.0f,
         tess_thickness,
         vec3_origin
      );
   }

   // Darken if we are hurt
   if(flags & FL_DARKEN)
   {
      edict->s.renderfx |= RF_LIGHTOFFSET;
      if(max_health)
      {
         edict->s.lightofs = -(40.0f * ((float)(max_health - health) / (float)max_health));
      }
      else
      {
         edict->s.lightofs -= damage;
      }
      if(edict->s.lightofs < -127)
         edict->s.lightofs = -127;
      if(edict->s.lightofs > 127)
         edict->s.lightofs = 127;
   }

   // Drop your weapon if you get hit in the hands
   if(location == gun_bone_group_name && ((damage > 50) || (G_Random(1) > 0.97f)))
   {
      // don't drop weapons when in deathmatch and DF_NO_DROP_WEAPONS is set
      if(currentWeapon && currentWeapon->IsDroppable() && (!deathmatch->value || !DM_FLAG(DF_NO_DROP_WEAPONS)))
      {
         DropCurrentWeapon();
         WeaponKnockedFromHands();
      }
   }

   if((meansofdeath == MOD_MUTANT_DRAIN) || (meansofdeath == MOD_DEATHQUAD))
      return;

   // Send pain event
   event = new Event(EV_Pain);
   event->AddFloat(damage);
   event->AddEntity(attacker);
   event->AddString(location);
   event->AddInteger(meansofdeath);
   ProcessEvent(event);
}

void Sentient::UpdateSilencedWeapons(void)
{
   // If the current weapon is silencable, then change it.
   if(currentWeapon && currentWeapon->IsSilenced())
   {
      currentWeapon->ProcessEvent(EV_Weapon_PutAwayAndRaise);
   }
}

int Sentient::NumInventoryItems(void)
{

   return inventory.NumObjects();

}

Item *Sentient::NextItem(Item *item)
{
   Item		*next_item;
   int		i;
   int		n;
   qboolean item_found = false;

   if(!item)
   {
      item_found = true;
   }
   else if(!inventory.ObjectInList(item->entnum))
   {
      error("NextItem", "Item not in list");
   }

   n = inventory.NumObjects();

   for(i = 1; i <= n; i++)
   {
      next_item = (Item *)G_GetEntity(inventory.ObjectAt(i));
      assert(next_item);

      if(next_item->isSubclassOf<InventoryItem>() && item_found)
         return next_item;

      if(next_item == item)
         item_found = true;
   }

   return NULL;
}

Item *Sentient::PrevItem(Item *item)
{
   Item		*prev_item;
   int		i;
   int		n;
   qboolean item_found = false;

   if(!item)
   {
      item_found = true;
   }
   else if(!inventory.ObjectInList(item->entnum))
   {
      error("NextItem", "Item not in list");
   }

   n = inventory.NumObjects();

   for(i = n; i >= 1; i--)
   {
      prev_item = (Item *)G_GetEntity(inventory.ObjectAt(i));
      assert(prev_item);

      if(prev_item->isSubclassOf<InventoryItem>() && item_found)
         return prev_item;

      if(prev_item == item)
         item_found = true;
   }

   return NULL;
}

// This will search a dead body and create a floating inventory for that player
// which will be displayed on the client as a list of icons.  If the user presses
// use, then he will pick up the items in that floating inventory, and the body
// will be faded out.
void Sentient::SearchBody(Event *ev)
{
   int i, n;
   Item *item;
   Entity *other;
   Player *player;
   Event *event;
   qboolean inventory_created = false;

   other = ev->GetEntity(1);
   assert(other);

   // Only players can touch this
   if(!other->isSubclassOf<Player>())
      return;

   // Can't touch yourself
   if(other == this)
      return;

   // Make sure the body is dead
   if(!deadflag)
      return;

   n = inventory.NumObjects();

   // No items, get out of here
   if(n == 0)
   {
      return;
   }

   player = (Player *)other;

   // Check to see if this inventory is already being displayed for
   // this body.
   if(player->GetFloatingOwner() == this)
   {
      // Still touching the body, so extend the clear time
      player->CancelEventsOfType(EV_Player_ClearFloatingInventory);
      event = new Event(EV_Player_ClearFloatingInventory);
      player->PostEvent(event, 0.4f);
      return;
   }
   else if(player->GetFloatingOwner())
   {
      // Currently looking at an inventory, so don't bother with this one.
      return;
   }
   else
   {
      // Make sure the inventory is cleared out before making a new one.
      // Could have some left over items in there from other bodies
      player->ProcessEvent(EV_Player_ClearFloatingInventory);
   }

   // Create the inventory
   for(i = 1; i <= n; i++)
   {
      item = (Item *)G_GetEntity(inventory.ObjectAt(i));
      assert(item);
      if(item->Amount())
      {
         player->AddItemToFloatingInventory(item);
         inventory_created = true;
      }
   }

   if(!inventory_created)
      return;

   // Set the current owner of the inventory
   player->SetFloatingOwner(this);

   // Send the inventory to the client
   player->SendFloatingInventory();

   // Clear the inventory in the future
   event = new Event(EV_Player_ClearFloatingInventory);
   player->PostEvent(event, 0.3f);
}

void Sentient::UselessCheck(Event *ev)
{
   Item     *item;
   int      n;
   int      i;
   Event    *event;

   // Check to see if we have any inventory and
   // remove if we don't

   n = inventory.NumObjects();
   for(i = 1; i <= n; i++)
   {
      item = (Item *)G_GetEntity(inventory.ObjectAt(i));
      assert(item);
      if(item->Amount())
      {
         PostEvent(EV_Sentient_UselessCheck, 1.0f + G_Random());
         return;
      }
   }

   event = new Event("remove_useless");
   ProcessEvent(event);
}

qboolean Sentient::HasInventoryOfType(const char *type)
{
   int	num;
   int	i;
   Item	*item;

   num = inventory.NumObjects();
   for(i = num; i > 0; i--)
   {
      item = (Item *)G_GetEntity(inventory.ObjectAt(i));
      if(checkInheritance(type, item->getClassname()))
      {
         return true;
      }
   }
   return false;
}

void Sentient::DropInventoryItems(void)
{
   int   num;
   int   i;
   Item  *item;

   // Drop any inventory items
   num = inventory.NumObjects();
   for(i = num; i >= 1; i--)
   {
      item = (Item *)G_GetEntity(inventory.ObjectAt(i));
      if(item->isSubclassOf<InventoryItem>())
      {
         item->Drop();
      }
   }
}

qboolean Sentient::PowerupActive(void)
{
   if(poweruptype && this->client)
   {
      gi.cprintf(edict, PRINT_HIGH, "You are already using a powerup\n");
   }

   return poweruptype;
}

void Sentient::SprayBlood(Vector src, Vector dir, float damage)
{
   trace_t	trace;
   Vector	norm;
   Vector	end;
   Vector   ang;
   float		dist;
   float		scale;
   BloodSplat *splat;

   dir.normalize();
   end = src + dir * 2048;

   trace = G_Trace(src, vec_zero, vec_zero, end, this, MASK_SOLIDNONFENCE, "Sentient::SprayBlood");

   if(HitSky(&trace) || (trace.ent->solid != SOLID_BSP) || (trace.ent->s.number != 0))
   {
      return;
   }

   dist = (Vector(trace.endpos) - src).length();
   scale = damage / (dist * 0.3);
   if(scale > 0.6)
   {
      scale = 0.6;
   }

   if(scale < 0.02)
   {
      return;
   }

   norm = trace.plane.normal;
   norm.x = -norm.x;
   norm.y = -norm.y;
   ang = norm.toAngles();
   ang.z = G_Random(360);
   end = trace.endpos + Vector(trace.plane.normal) * 0.2;

   splat = new BloodSplat(end, ang, scale);
}

EXPORT_FROM_DLL void Sentient::setModel(const char *mdl)
{
   if(currentWeapon)
   {
      // rebind the current gun
      currentWeapon->DetachFromOwner();
      Entity::setModel(mdl);
      currentWeapon->AttachToOwner();
   }
   else
   {
      Entity::setModel(mdl);
   }
}

void Sentient::TurnOffShadow(Event *ev)
{
   // HACK FIXME
   //
   // temporary shadow flag
   //
   edict->s.renderfx &= ~RF_XFLIP;
}

EXPORT_FROM_DLL void Sentient::Archive(Archiver &arc)
{
   int i;
   int num;

   Entity::Archive(arc);

   num = inventory.NumObjects();
   arc.WriteInteger(num);
   for(i = 1; i <= num; i++)
   {
      arc.WriteInteger(inventory.ObjectAt(i));
   }
   arc.WriteObjectPointer(currentWeapon);
   arc.WriteObjectPointer(currentItem);
   arc.WriteObjectPointer(newWeapon);
   arc.WriteString(currentAnim);
   arc.WriteBoolean(animOverride);
   arc.WriteEvent(*tempAnimEvent);
   arc.WriteString(gun_bone_group_name);
   arc.WriteBoolean(stopanimating_tillchange);
   arc.WriteInteger(poweruptype);
   arc.WriteInteger(poweruptimer);
   arc.WriteBoolean(sentientFrozen);
   arc.WriteBoolean(dropweapon);
   arc.WriteVector(gunoffset);
   arc.WriteVector(eyeposition);
   arc.WriteFloat(firedowntime);
   arc.WriteBoolean(firedown);
   arc.WriteString(saveskin);
   arc.WriteString(savemodel);
}

EXPORT_FROM_DLL void Sentient::Unarchive(Archiver &arc)
{
   int i;
   int num;
   int temp;

   Entity::Unarchive(arc);

   inventory.FreeObjectList();
   num = arc.ReadInteger();
   for(i = 1; i <= num; i++)
   {
      temp = arc.ReadInteger();
      inventory.AddObject(temp);
   }

   arc.ReadObjectPointer((Class **)&currentWeapon);
   arc.ReadObjectPointer((Class **)&currentItem);
   arc.ReadObjectPointer((Class **)&newWeapon);
   arc.ReadString(&currentAnim);
   arc.ReadBoolean(&animOverride);
   //FIXME
   // make a ReadEventPointer function
   Event& ev = arc.ReadEvent();
   tempAnimEvent = new Event(ev);          // ksh -- Pulled event out first to fix compilation bugs
   arc.ReadString(&gun_bone_group_name);   // ksh -- Switched to passing in event object to fix compilation bugs
   arc.ReadBoolean(&stopanimating_tillchange);
   arc.ReadInteger(&poweruptype);
   arc.ReadInteger(&poweruptimer);
   arc.ReadBoolean(&sentientFrozen);
   arc.ReadBoolean(&dropweapon);
   arc.ReadVector(&gunoffset);
   arc.ReadVector(&eyeposition);
   arc.ReadFloat(&firedowntime);
   arc.ReadBoolean(&firedown);
   arc.ReadString(&saveskin);
   arc.ReadString(&savemodel);
}

void Sentient::WritePersistantData(SpawnArgGroup &group)
{
   int i;
   int num;
   Entity *ent;

   G_SetSpawnArg("classname", getClassname());
   if(currentWeapon)
   {
      G_SetSpawnArg("current_weapon", currentWeapon->getClassname());
   }
   if(currentItem)
   {
      G_SetSpawnArg("current_item", currentItem->getClassname());
   }

   G_SetIntArg("health", health);
   G_SetIntArg("max_health", max_health);
   G_SetIntArg("savegame", 1);

   group.AddArgs();

   num = inventory.NumObjects();
   for(i = 1; i <= num; i++)
   {
      ent = G_GetEntity(inventory.ObjectAt(i));
      if(ent && ent->isSubclassOf<Item>())
      {
         G_InitSpawnArguments();
         ((Item *)ent)->CreateSpawnArgs();
         G_SetSpawnArg("classname", ent->getClassname());
         G_SetIntArg("savegame", 1);
         group.AddArgs();
      }
   }

   G_InitSpawnArguments();
}

void Sentient::RestorePersistantData(SpawnArgGroup &group)
{
   str weap;
   str item;
   Entity *ent;
   int i;
   int num;
   Item *it;

   num = group.NumInGroup();
   for(i = 2; i <= num; i++)
   {
      group.RestoreArgs(i);

      game.force_entnum = false;
      ent = G_CallSpawn();
      if(!ent->isSubclassOf<Item>())
      {
         error("RestorePersistantData", "Loaded non-Item object of class %s\n", ent->getClassname());
      }

      // G_CallSpawn clears all the spawnargs
      group.RestoreArgs(i);

      it = (Item *)ent;
      it->SetOwner(this);
      it->ProcessPendingEvents();
      AddItem(it);
   }

   group.RestoreArgs(1);

   weap = G_GetStringArg("current_weapon");
   item = G_GetStringArg("current_item");

   health = G_GetIntArg("health", 100);
   max_health = G_GetIntArg("max_health", 100);

   if(weap.length())
   {
      it = FindItem(weap.c_str());
      assert(it);
      if(it)
      {
         if(!it->isSubclassOf<Weapon>())
         {
            error("RestorePersistantData", "Tried to set non-weapon of type %s as current weapon\n", it->getClassname());
         }
         SetCurrentWeapon((Weapon *)it);
      }
   }

   if(item.length())
   {
      it = FindItem(item.c_str());
      assert(it);
      if(it)
      {
         if(!it->isSubclassOf<InventoryItem>())
         {
            error("RestorePersistantData", "Tried to set non-inventory item of type %s as current item\n", it->getClassname());
         }
         currentItem = (InventoryItem *)it;
      }
   }

}

EXPORT_FROM_DLL void Sentient::Freeze(Event *ev)
{
   sentientFrozen = true;
}

EXPORT_FROM_DLL void Sentient::UnFreeze(Event *ev)
{
   sentientFrozen = false;
}

void Sentient::ImpactDamage(Event *ev)
{
   float damg;

   if(deathmatch->value)
   {
      damg = velocity.length();
      damg /= 20.0f;
   }
   else
   {
      damg = 100;
   }

   setMoveType(MOVETYPE_STEP);
   Damage(world, enemy, damg,
          worldorigin, vec_zero, vec_zero, 0,
          0, MOD_IMPACT,
          -1, -1, 1.0f);
}

void Sentient::WeaponUse(Event *ev)
{
   Event *event;

   if(!currentWeapon)
      return;

   currentWeapon->ForceState(WEAPON_READY);

   event = new Event(EV_Weapon_SecondaryUse);
   event->AddEntity(this);
   currentWeapon->ProcessEvent(event);
}

void Sentient::DoubleArmor(void)
{
   int i, n;

   n = inventory.NumObjects();

   for(i = 1; i <= n; i++)
   {
      Item  *item;
      item = (Item *)G_GetEntity(inventory.ObjectAt(i));

      if(item->isSubclassOf<Armor>())
         item->Set(item->Amount() * 2);
   }
}

void Sentient::WeaponKnockedFromHands(void)
{
   str realname;

   realname = GetRandomAlias("snd_lostweapon");
   if(realname.length() > 1)
   {
      sound(realname, 1, CHAN_VOICE);
   }
   else
   {
      RandomSound("snd_pain", 1, CHAN_VOICE);
   }
}

void Sentient::SetDropWeapon(Event *ev)
{
   dropweapon = ev->GetInteger(1);
}

void Sentient::DropWeaponNowEvent(Event *ev)
{
   DropCurrentWeapon();
}

// EOF

