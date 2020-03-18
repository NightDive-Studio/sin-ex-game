//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/sentient.h                       $
// $Revision:: 71                                                             $
//   $Author:: Aldie                                                          $
//     $Date:: 3/02/99 9:16p                                                  $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Base class of entity that can carry other entities, and use weapons.
//

#ifndef __SENTIENT_H__
#define __SENTIENT_H__

#include "g_local.h"
#include "entity.h"
#include "container.h"

extern Event EV_Sentient_Attack;
extern Event EV_Sentient_ReleaseAttack;
extern Event EV_Sentient_GiveWeapon;
extern Event EV_Sentient_GiveAmmo;
extern Event EV_Sentient_TakeAmmo;
extern Event EV_Sentient_GiveArmor;
extern Event EV_Sentient_TakeArmor;
extern Event EV_Sentient_TakeItem;
extern Event EV_Sentient_GiveItem;
extern Event EV_Sentient_GiveInventoryItem;
extern Event EV_Sentient_GiveHealth;
extern Event EV_Sentient_WeaponPutAway;
extern Event EV_Sentient_WeaponReady;
extern Event EV_Sentient_WeaponDoneFiring;
extern Event EV_Sentient_AnimLoop;
extern Event EV_Sentient_UselessCheck;
extern Event EV_Sentient_TurnOffShadow;
extern Event EV_Sentient_Freeze;
extern Event EV_Sentient_UnFreeze;
extern Event EV_Sentient_ImpactDamage;
extern Event EV_Sentient_WeaponUse;
extern Event EV_Sentient_SetDropWeapon;

// Shutup compiler
class Weapon;
class Item;
class InventoryItem;

class EXPORT_FROM_DLL Sentient : public Entity
{
private:
   Container<int>		inventory;

protected:
   Weapon           *currentWeapon;
   InventoryItem    *currentItem;
   Weapon           *newWeapon;
   str               currentAnim;
   qboolean          animOverride;
   Event            *tempAnimEvent;
   str               gun_bone_group_name;
   qboolean          stopanimating_tillchange;
   int               poweruptype;
   int               poweruptimer;
   qboolean          sentientFrozen;
   qboolean          dropweapon;

   virtual void      EventGiveWeapon(Event *ev);
   virtual void      EventTakeWeapon(Event *ev);
   virtual void      EventGiveAmmo(Event *ev);
   virtual void      EventTakeAmmo(Event *ev);
   virtual void      EventGiveArmor(Event *ev);
   virtual void      EventTakeArmor(Event *ev);
   virtual void      EventGiveItem(Event *ev);
   virtual void      EventGiveHealth(Event *ev);
   virtual void      EventGiveInventoryItem(Event *ev);
   virtual void      EventGiveTargetname(Event *ev);
   virtual void      EventTakeItem(Event *ev);
   virtual void      WeaponPutAway(Event *ev);
   virtual void      WeaponReady(Event *ev);
   virtual void      WeaponDoneFiring(Event *ev);
   virtual void      AnimLoop(Event *ev);
   virtual void      ArmorDamage(Event *ev);
   virtual void      TurnOffShadow(Event *ev);
   virtual void      Freeze(Event *ev);
   virtual void      UnFreeze(Event *ev);
   virtual void      WeaponKnockedFromHands(void);

public:
   Vector            gunoffset;
   Vector            eyeposition;

   // Weapon charging stuff
   float             firedowntime;
   qboolean          firedown;
   qboolean          usedown;

   str               saveskin;
   str               savemodel;

   CLASS_PROTOTYPE(Sentient);

   Sentient();
   virtual ~Sentient();

   virtual void      RestorePersistantData(SpawnArgGroup &group);
   virtual void      WritePersistantData(SpawnArgGroup &group);
   Vector            EyePosition();
   virtual Vector    GunPosition();
   virtual void      GetGunOrientation(Vector pos, Vector *forward, Vector *right, Vector *up);
   virtual Item     *giveItem(const char * itemname, int amount, int icon_index = -1);
   virtual void      FireWeapon(Event *ev);
   virtual void      ReleaseFireWeapon(Event *ev);
   virtual void      AddItem(Item *object);
   virtual void      RemoveItem(Item *object);
   virtual Item     *FindItem(const char *itemname);
   virtual void      FreeInventory();
   virtual void      FreeInventoryOfType(const char *weaptype);
   virtual qboolean  HasItem(const char *itemname);
   virtual void      ForceChangeWeapon(Weapon *weapon);
   virtual void      ChangeWeapon(Weapon *weapon);
   virtual void      SetCurrentWeapon(Weapon *weapon);
   virtual Weapon   *CurrentWeapon();
   virtual Weapon   *BestWeapon(Weapon *ignore = nullptr);
   virtual Weapon   *NextWeapon(Weapon *weapon);
   virtual Weapon   *PreviousWeapon(Weapon *weapon);
   virtual qboolean  WeaponReady();
   virtual void      DropWeapon(Weapon *weapon);
   virtual void      DropCurrentWeapon(void);
   virtual Weapon   *giveWeapon(const char *weaponname);
   virtual void      takeWeapon(const char *weaponname);
   virtual void      takeItem(const char *weaponname, int amount);
   virtual Weapon   *useWeapon(const char *weaponname);
   virtual int       NumWeapons();
   virtual Weapon   *WeaponNumber(int weaponnum);
   virtual void      SetAnim(const char *anim);
   virtual void      TempAnim(const char *anim, Event *event);
   virtual void      TempAnim(const char *anim, Event &event);
   virtual void      UpdateSilencedWeapons();
   virtual int       NumInventoryItems();
   virtual Item     *NextItem(Item *item);
   virtual Item     *PrevItem(Item *item);
   virtual void      SearchBody(Event *ev);
   virtual qboolean  CanChangeWeapons();
   virtual void      UselessCheck(Event *ev);
   virtual qboolean  HasInventoryOfType(const char *);
   virtual void      DropInventoryItems();
   void              SprayBlood(Vector src, Vector dir, float damage);
   void              PrintDamageLocationToAttacker(edict_s *attacker, const char *victim_name, const char *location);
   void              PrintDamageLocationToVictim(edict_s *victim, const char *location);
   qboolean          PowerupActive();
   virtual void      setModel(const char *model);
   virtual void      setModel(str &mdl);
   virtual void      Archive(Archiver &arc)   override;
   virtual void      Unarchive(Archiver &arc) override;
   virtual void      ImpactDamage(Event *ev);
   virtual void      WeaponUse(Event *ev);
   void              GetMuzzlePositionAndDirection(Vector *pos, Vector *dir);
   void              DoubleArmor();
   virtual qboolean  DoGib(int meansofdeath, Entity *inflictor);
   virtual void      SetDropWeapon(Event *ev);
   virtual void      DropWeaponNowEvent(Event *ev);

   // CTF
   virtual Item     *HasItemOfSuperclass(const char *superclassname);
};

inline EXPORT_FROM_DLL void Sentient::setModel(str &mdl)
{
   setModel(mdl.c_str());
}

#ifdef EXPORT_TEMPLATE
template class EXPORT_FROM_DLL SafePtr<Sentient>;
template class EXPORT_FROM_DLL Container<Sentient *>;
#endif

typedef SafePtr<Sentient> SentientPtr;

extern Container<Sentient *> SentientList;

void EXPORT_FROM_DLL ResetBloodSplats();

#endif /* sentient.h */

// EOF

