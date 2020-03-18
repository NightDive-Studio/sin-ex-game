/*
================================================================
HOVERBIKE
================================================================

Copyright (C) 2020 by Night Dive Studios, Inc.
All rights reserved.

See the license.txt file for conditions and terms of use for this code.

AI NOTE:
Although I didn't have time to impliment it, the hoverbike should be
(for the most part) structured properly to allow for AI control of one.
I was planning on having the AI communicate to the hoverbike through
fake usercmd_t info that the AI would put together and send to bike to
tell it were to go. All bits of code refering to bots assumes that
they will be clients. This note's here in hopes that you're an
industrious soul and want to make a Sin bot that can use hoverbikes. ;)
*/

#ifndef __HOVERBIKE_H__
#define __HOVERBIKE_H__

#include "g_local.h"
#include "entity.h"
#include "misc.h"

// weapon defines
#define NUM_HOVERWEAPONS 3
#define HWMODE_ROCKETS   0
#define HWMODE_CHAINGUN  1
#define HWMODE_MINES     2

// guage defines
#define HBGUAGE_SPEEDBAR   1
#define HBGUAGE_SPEEDNUM   2
#define HBGUAGE_TURBOBAR   3
#define HBGUAGE_HEALTHBAR  4
#define HBGUAGE_AMMONUM    5
#define HBGUAGE_WEAPONICON 6

// hoverbike movement flags used when
// deciding what hovering sound to play
#define HBSND_NONE      0
#define HBSND_HOVERING  1 // set when the bike is hovering off the ground
#define HBSND_CLOSE     2 // set when hoverbike is hover close to the ground
#define HBSND_ACCEL     4 // set when hoverbike is accelerating
#define HBSND_BRAKE     8 // set when hoverbike is braking

// rider type defines
typedef enum
{
   RIDER_OTHER,
   RIDER_PLAYER,
   RIDER_BOT // hint hint ;)
} ridertype_t;

// the extra bounding boxes for a hoverbike
class EXPORT_FROM_DLL HoverbikeBox : public Entity
{
public:
   float offset; // forward offset from the main hoverbike entity
   EntityPtr bike; // pointer to owner hoverbike

   CLASS_PROTOTYPE(HoverbikeBox);

   HoverbikeBox();
   virtual void BikeUse(Event *ev);
   virtual void BikeDamage(Event *ev);

   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void HoverbikeBox::Archive(Archiver &arc)
{
   Entity::Archive(arc);

   arc.WriteFloat(offset);
   arc.WriteSafePointer(bike);
}

inline EXPORT_FROM_DLL void HoverbikeBox::Unarchive(Archiver &arc)
{
   Entity::Unarchive(arc);

   arc.ReadFloat(&offset);
   arc.ReadSafePointer(&bike);
}

template class EXPORT_FROM_DLL	SafePtr<HoverbikeBox>;
typedef SafePtr<HoverbikeBox> HoverbikeBoxPtr;

// control guages
class EXPORT_FROM_DLL HoverbikeGuage : public Entity
{
private:
   int guagetype; // the kind of guage that it is

public:
   CLASS_PROTOTYPE(HoverbikeGuage);

   virtual void Setup(Entity *owner, int type);
   virtual void SetValue(int value);

   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void HoverbikeGuage::Archive(Archiver &arc)
{
   Entity::Archive(arc);

   arc.WriteInteger(guagetype);
}

inline EXPORT_FROM_DLL void HoverbikeGuage::Unarchive(Archiver &arc)
{
   Entity::Unarchive(arc);

   arc.ReadInteger(&guagetype);
}

template class EXPORT_FROM_DLL	SafePtr<HoverbikeGuage>;
typedef SafePtr<HoverbikeGuage> HoverbikeGuagePtr;

class Player;

class EXPORT_FROM_DLL Hoverbike : public Entity
{
public:
   HoverbikeBoxPtr frontbox; // pointer to front bounding box
   HoverbikeBoxPtr backbox; // pointer to back bounding box

   EntityPtr   rider     = nullptr; // pointer to riding player
   ridertype_t ridertype = RIDER_OTHER; // set to quickly tell what kind of rider we have

   HoverbikeGuagePtr speed_bar;
   HoverbikeGuagePtr speed_number;
   HoverbikeGuagePtr turbo_bar;
   HoverbikeGuagePtr health_bar;
   HoverbikeGuagePtr ammo_number;
   HoverbikeGuagePtr weapon_icon;

   Vector spawnspot;           // place where it was spawned
   Vector spawnangles;         // angles at which it was spawned
   float  respawntimer = 0.0f; // timmer for respawning
   float  damagetimer;
   float  getontimer;          // use debounce timer for when getting on

   float bobsin        = 0.0f; // for rider view bobbing
   float bobfrac       = 0.0f; // for rider view bobbing

   EntityPtr oldweapon;
   int rockets;
   int bullets;
   int mines;

   float forwardmove   = 0.0f;
   float sidemove      = 0.0f;
   float upmove        = 0.0f;

   Vector move_angles;
   float  airpitch_timmer;
   float  speed;
   int    forward_speed;
   float  strafingroll = 0.0f;
   float  boosttimmer;
   float  turbo        = 100.0f;

   int weaponmode      = 0; // bike's current weapon mode

   int sndflags        = HBSND_NONE; // hovering sound flags
   str currsound;                    // alias of current hovering sound

   float effectstimmer = 0.0f;
   float soundtimmer   = 0.0f;

   CLASS_PROTOTYPE(Hoverbike);

   Hoverbike();

   virtual void RespawnSetup(Vector spot, Vector ang, float respawndelay, int grav, int flags);
   virtual void BikeRespawn(Event *ev);

   virtual void SetRiderAngles(Vector angles);
   virtual void SetRiderYaw(float yawangle);
   virtual void BikeUse(Event *ev);
   virtual void BikeGetOff(void);
   virtual void BikePain(Event *ev);
   virtual void BikeKilled(Event *ev);
   virtual void GiveExtraFrag(Entity *attacker);

   virtual qboolean Ride(usercmd_t *ucmd);
   virtual void RiderMove(usercmd_t *ucmd);
   virtual void Postthink() override;
   virtual void Hover();
   virtual void ApplyControls();
   virtual void ApplyMoveAngles();
   void SetGravityAxis(int axis);

   virtual void WeaponChangeSound();
   virtual void WeaponNoAmmoSound();
   virtual void SetHoverSound();
   virtual void CollisionSound(Vector before, Vector after);

   virtual void NextWeapon();
   virtual void PreviousWeapon();
   virtual void SelectWeapon(int weapon);

   virtual void MakeGuages();
   virtual void KillGuages();
   virtual void UpdateGuages();
   virtual void GuagesViewerOn();
   virtual void GuagesViewerOff();

   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void Hoverbike::Archive (Archiver &arc)
{
   Entity::Archive(arc);

   arc.WriteSafePointer(frontbox);
   arc.WriteSafePointer(backbox);

   arc.WriteSafePointer(rider);
   arc.WriteInteger(ridertype);

   arc.WriteSafePointer(speed_bar);
   arc.WriteSafePointer(speed_number);
   arc.WriteSafePointer(turbo_bar);
   arc.WriteSafePointer(health_bar);
   arc.WriteSafePointer(ammo_number);
   arc.WriteSafePointer(weapon_icon);

   arc.WriteVector(spawnspot);
   arc.WriteVector(spawnangles);
   arc.WriteFloat(respawntimer);
   arc.WriteFloat(damagetimer);
   arc.WriteFloat(getontimer);

   arc.WriteFloat(bobsin);
   arc.WriteFloat(bobfrac);

   arc.WriteSafePointer(oldweapon);
   arc.WriteInteger(rockets);
   arc.WriteInteger(bullets);
   arc.WriteInteger(mines);

   arc.WriteFloat(forwardmove);
   arc.WriteFloat(sidemove);
   arc.WriteFloat(upmove);

   arc.WriteVector(move_angles);
   arc.WriteFloat(airpitch_timmer);
   arc.WriteFloat(speed);
   arc.WriteInteger(forward_speed);
   arc.WriteFloat(strafingroll);
   arc.WriteFloat(boosttimmer);
   arc.WriteFloat(turbo);

   arc.WriteInteger(weaponmode);

   arc.WriteInteger(sndflags);
   arc.WriteString(currsound);

   arc.WriteFloat(effectstimmer);
   arc.WriteFloat(soundtimmer);
}

inline EXPORT_FROM_DLL void Hoverbike::Unarchive(Archiver &arc)
{
   int temp;

   Entity::Unarchive(arc);

   arc.ReadSafePointer(&frontbox);
   arc.ReadSafePointer(&backbox);

   arc.ReadSafePointer(&rider);
   temp = arc.ReadInteger();
   ridertype = (ridertype_t)temp;

   arc.ReadSafePointer(&speed_bar);
   arc.ReadSafePointer(&speed_number);
   arc.ReadSafePointer(&turbo_bar);
   arc.ReadSafePointer(&health_bar);
   arc.ReadSafePointer(&ammo_number);
   arc.ReadSafePointer(&weapon_icon);

   arc.ReadVector(&spawnspot);
   arc.ReadVector(&spawnangles);
   respawntimer = arc.ReadFloat();
   damagetimer  = arc.ReadFloat();
   getontimer   = arc.ReadFloat();

   bobsin  = arc.ReadFloat();
   bobfrac = arc.ReadFloat();

   arc.ReadSafePointer(&oldweapon);
   rockets = arc.ReadInteger();
   bullets = arc.ReadInteger();
   mines   = arc.ReadInteger();

   forwardmove = arc.ReadFloat();
   sidemove    = arc.ReadFloat();
   upmove      = arc.ReadFloat();

   arc.ReadVector(&move_angles);
   airpitch_timmer = arc.ReadFloat();
   speed           = arc.ReadFloat();
   forward_speed   = arc.ReadInteger();
   strafingroll    = arc.ReadFloat();
   boosttimmer     = arc.ReadFloat();
   turbo           = arc.ReadFloat();

   weaponmode = arc.ReadInteger();

   sndflags = arc.ReadInteger();
   arc.ReadString(&currsound);

   effectstimmer = arc.ReadFloat();
   soundtimmer   = arc.ReadFloat();
}

template class EXPORT_FROM_DLL SafePtr<Hoverbike>;
typedef SafePtr<Hoverbike> HoverbikePtr;

extern int SV_Physics_Hoverbike(Hoverbike *bike);
extern int G_Physics_Hoverbike(Hoverbike *bike);

#endif /* hoverbike.h */
