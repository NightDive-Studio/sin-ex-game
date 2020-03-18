/*
================================================================
SPRITE PLACEMENT GUN
================================================================

Copyright (C) 2020 by Night Dive Studios, Inc.
All rights reserved.

See the license.txt file for conditions and terms of use for this code.

The purpose of this device is to allow level designers to
actually walk through their levels and place sprite decals
wherever they want.
*/

#ifndef __SPRITEGUN_H__
#define __SPRITEGUN_H__

#include "g_local.h"
#include "item.h"
#include "weapon.h"
#include "misc.h"
#include "entity.h"

//===================================================================

typedef struct sf_t 
{
   str model;
   int frames;
} sf_t;

//===================================================================

#define SEFFECT_NONE    0
#define SEFFECT_TOGGLE  1
#define SEFFECT_ADVANCE 2
#define SEFFECT_ONCE    3

class EXPORT_FROM_DLL PlacedSprite : public Entity
{
public:
   CLASS_PROTOTYPE(PlacedSprite);

   int      numframes; // the number of frames in the sprite/model
   int      triggereffect; // effect that happens when triggered
   float    animatespeed;
   float    animatedelay;
   int      pitchspeed;
   int      yawspeed;
   int      rollspeed;
   float    rotatedelay;
   float    blinkspeed;
   float    blinkdelay;

   // this data keeps track of data to put into the sprite file
   int      placedframe;
   Vector   placedangles;

   PlacedSprite();

   virtual void Setup(Vector pos, Vector facing);
   virtual void Animate(Event *ev);
   virtual void Rotate(Event *ev);
   virtual void Blink(Event *ev);
   virtual void TriggerEvent(Event *ev);

   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void PlacedSprite::Archive(Archiver &arc)
{
   Entity::Archive(arc);

   arc.WriteInteger(numframes);
   arc.WriteInteger(triggereffect);
   arc.WriteFloat(animatespeed);
   arc.WriteFloat(animatedelay);
   arc.WriteInteger(pitchspeed);
   arc.WriteInteger(yawspeed);
   arc.WriteInteger(rollspeed);
   arc.WriteFloat(rotatedelay);
   arc.WriteFloat(blinkspeed);
   arc.WriteFloat(blinkdelay);
   arc.WriteInteger(placedframe);
   arc.WriteVector(placedangles);
}

inline EXPORT_FROM_DLL void PlacedSprite::Unarchive(Archiver &arc)
{
   Entity::Unarchive(arc);

   arc.ReadInteger(&numframes);
   arc.ReadInteger(&triggereffect);
   arc.ReadFloat(&animatespeed);
   arc.ReadFloat(&animatedelay);
   arc.ReadInteger(&pitchspeed);
   arc.ReadInteger(&yawspeed);
   arc.ReadInteger(&rollspeed);
   arc.ReadFloat(&rotatedelay);
   arc.ReadFloat(&blinkspeed);
   arc.ReadFloat(&blinkdelay);
   arc.ReadInteger(&placedframe);
   arc.ReadVector(&placedangles);
}

//===================================================================

class EXPORT_FROM_DLL SpriteMaster : public Listener
{
public:
   CLASS_PROTOTYPE(SpriteMaster);

   str      filename;
   int      num_sprites;
   char    *spritename;
   int      place_frame;
   float    place_scale;
   float    place_alpha;
   float    place_animspeed;
   float    place_animdelay;
   float    place_pitchspeed;
   float    place_yawspeed;
   float    place_rollspeed;
   float    place_rotdelay;
   float    place_blinkspeed;
   float    place_blinkdelay;
   int      place_trigeffect;

   int      curr_sprite; // pointer to current sprite entity
   qboolean glowcurrent; // toggle for making the current sprite glow
   qboolean hitwater;    // if true, will trace to water surfaces
   float    movesize;    // amount to move a sprite each time it's nudged
   int      rotsize;     // amount to move a sprite each time it's nudged

   SpriteMaster();

   void LoadSpriteFile(Event *ev);
   void SetFileName();
   void ParseSpriteIndex();
   void ParseSpriteFile();
   void SaveSpriteFile(Event *ev);

   int  NumFrames(const char *testmodel);

   void SetSprite(Event *ev);
   void ToggleGlow(Event *ev);
   void GlowFlicker(Event *ev);
   void ToggleHitWater(Event *ev);
   void Next(Event *ev);
   void Previous(Event *ev);
   void Delete(Event *ev);
   void SetScale(Event *ev);
   void SetAlpha(Event *ev);
   void SetFrame(Event *ev);
   void NextFrame(Event *ev);
   void PrevFrame(Event *ev);
   void SetMoveSize(Event *ev);
   void MoveRight(Event *ev);
   void MoveLeft(Event *ev);
   void MoveUp(Event *ev);
   void MoveDown(Event *ev);
   void MoveForward(Event *ev);
   void MoveBack(Event *ev);
   void SetPitch(Event *ev);
   void SetYaw(Event *ev);
   void SetRoll(Event *ev);
   void SetRotateSize(Event *ev);
   void PitchUp(Event *ev);
   void PitchDown(Event *ev);
   void YawRight(Event *ev);
   void YawLeft(Event *ev);
   void RollRight(Event *ev);
   void RollLeft(Event *ev);
   void GetSettings(Event *ev);
   void GiveSettings(Event *ev);
   void ClearSettings(Event *ev);
   void FlipSprite(Event *ev);
   void SetAnimSpeed(Event *ev);
   void SetAnimDelay(Event *ev);
   void SetPitchSpeed(Event *ev);
   void SetYawSpeed(Event *ev);
   void SetRollSpeed(Event *ev);
   void SetRotDelay(Event *ev);
   void SetBlinkSpeed(Event *ev);
   void SetBlinkDelay(Event *ev);
   void SetTargetname(Event *ev);
   void SetTriggerEffect(Event *ev);

   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void SpriteMaster::Archive(Archiver &arc)
{
   str tmpstr;

   Listener::Archive(arc);

   arc.WriteString(filename);
   arc.WriteInteger(num_sprites);
   tmpstr = spritename;
   arc.WriteString(tmpstr);
   arc.WriteInteger(place_frame);
   arc.WriteFloat(place_scale);
   arc.WriteFloat(place_alpha);
   arc.WriteFloat(place_animspeed);
   arc.WriteFloat(place_animdelay);
   arc.WriteFloat(place_pitchspeed);
   arc.WriteFloat(place_yawspeed);
   arc.WriteFloat(place_rollspeed);
   arc.WriteFloat(place_rotdelay);
   arc.WriteFloat(place_blinkspeed);
   arc.WriteFloat(place_blinkdelay);
   arc.WriteInteger(place_trigeffect);

   arc.WriteInteger(curr_sprite);
   arc.WriteBoolean(glowcurrent);
   arc.WriteBoolean(hitwater);
   arc.WriteFloat(movesize);
   arc.WriteInteger(rotsize);
}

inline EXPORT_FROM_DLL void SpriteMaster::Unarchive(Archiver &arc)
{
   str tmpstr;

   Listener::Unarchive(arc);

   arc.ReadString(&filename);
   arc.ReadInteger(&num_sprites);
   arc.ReadString(&tmpstr);
   spritename = new char [256];
   snprintf(spritename, 256, "%s", tmpstr.c_str());
   arc.ReadInteger(&place_frame);
   arc.ReadFloat(&place_scale);
   arc.ReadFloat(&place_alpha);
   arc.ReadFloat(&place_animspeed);
   arc.ReadFloat(&place_animdelay);
   arc.ReadFloat(&place_pitchspeed);
   arc.ReadFloat(&place_yawspeed);
   arc.ReadFloat(&place_rollspeed);
   arc.ReadFloat(&place_rotdelay);
   arc.ReadFloat(&place_blinkspeed);
   arc.ReadFloat(&place_blinkdelay);
   arc.ReadInteger(&place_trigeffect);

   arc.ReadInteger(&curr_sprite);
   arc.ReadBoolean(&glowcurrent);
   arc.ReadBoolean(&hitwater);
   arc.ReadFloat(&movesize);
   arc.ReadInteger(&rotsize);
}

extern SpriteMaster Spritecontrol;

//===================================================================

class EXPORT_FROM_DLL SpriteGun : public Weapon
{
public:
   CLASS_PROTOTYPE(SpriteGun);

   SpriteGun();
   virtual void Shoot(Event *ev);
};

extern void CleanupSpriteGun();

#endif /* spritegun.h */

// EOF

