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

All sprites automatically align themselves to the surface that
they are placed onto.

All changes made to parameters affect the current sprite selected.

In the below list, $ indicated a parameter is needed

List of console commands used for the Sprite Gun
------------------------------------------------
spg_save			: Saves current sprites to the map's sprite (msf) file
spg_load			: Reloads the sprite file. Good for checking timming offsets.
spg_file $			: Set which sprite to use (no file extension assumes a decal)
spg_glow			: Toggle glowing for current sprite
spg_hitwater		: Set to non-zero to make it collision detect water surfaces
spg_next			: Select the next sprite in the level
spg_previous		: Select the previous sprite in the level
spg_delete			: Delete the currently selected sprite
spg_scale $			: Set sprite scale to use
spg_alpha $			: Set sprite alpha to use
spg_frame $			: Set sprite frame to use
spg_nextframe		: Advance sprite to next frame
spg_prevframe		: Advance sprite to previous frame
spg_movesize $		: Sets the amount a sprite is moved when nudged
spg_moveright		: Nudge the current sprite to its right
spg_moveleft		: Nudge the current sprite to its left
spg_moveup			: Nudge the current sprite to its up
spg_movedown		: Nudge the current sprite to its down
spg_moveforward		: Nudge the current sprite forward
spg_moveback		: Nudge the current sprite backwards
spg_setpitch $		: Set the current sprite's pitch
spg_setyaw $		: Set the current sprite's yaw
spg_setroll $		: Set the current sprite's roll
spg_rotsize $		: sets the free rotation increment
spg_pitchup			: Pitch the current sprite up
spg_pitchdown		: Pitch the current sprite down
spg_yawright		: Yaw the current sprite clockwise
spg_yawleft			: Yaw the current sprite counter-clockwise
spg_rollright		: Roll the current sprite clockwise
spg_rollleft		: Roll the current sprite counter-clockwise
spg_get				: Gets all settings from the current sprite
spg_give			: Gives the current sprite all of the current settings
spg_clear			: Clears all current sprite settings.
spg_flip			: flip the surface normal used by the sprite
spg_animspeed $		: Specifies the amount of time between each frame of animation.
					  0 makes it not animate, and a negative value specifies the
					  animation speed without making it start out animating.
					  No specified value shows the current sprite's set value.
spg_animdelay $		: Delay before the sprite starts animating when the level loads.
spg_pitchspeed $	: Gives the sprite a constant pitch speed. 0 for no pitching.
					  No specified value shows the current sprite's set value.
spg_yawspeed $		: Gives the sprite a constant yaw speed. 0 for no yawing.
					  No specified value shows the current sprite's set value.
spg_rollspeed $		: Gives the sprite a constant roll speed. 0 for no rolling.
					  No specified value shows the current sprite's set value.
spg_rotdelay $		: Delay before the sprite starts rotating when the level loads.
spg_blinkspeed $	: Makes the sprite blink at the rate specified. 0 for no blinking.
					  No specified value shows the current sprite's set value.
spg_blinkdelay $	: Delay before the sprite starts blinking when the level loads.
spg_targetname $	: Sets the targetname value for the current sprite
					  No specified value shows the current sprite's set value.
spg_triggereffect $	: Sets the triggering effect for the current sprite
					  The following is the list of accaptable paramaters
					  "none" : Sets the sprite do to nothing when triggered
					  "toggle" : toggls the sprite's animation on/off when triggered
					  "advance" : Makes the sprite advance one frame when triggered
					  "once" : makes the sprite animate through once when triggered
*/

#include "spritegun.h"
#undef min
#undef max
#include "archive.h"

/*===================================================================

PlacedSprite : event setup stuff, here for the SpriteMaster's use

===================================================================*/

CLASS_DECLARATION(Entity, PlacedSprite, "placedsprite");

Event EV_PlacedSprite_Animate("placedsprite_animate");
Event EV_PlacedSprite_Rotate("placedsprite_rotate");
Event EV_PlacedSprite_Blink("placedsprite_blink");

ResponseDef PlacedSprite::Responses[] =
{
   { &EV_PlacedSprite_Animate, (Response)&PlacedSprite::Animate      },
   { &EV_PlacedSprite_Rotate,  (Response)&PlacedSprite::Rotate       },
   { &EV_PlacedSprite_Blink,   (Response)&PlacedSprite::Blink        },
   { &EV_Activate,             (Response)&PlacedSprite::TriggerEvent },
   { nullptr, nullptr }
};

/*===================================================================

SpriteMaster : Main setup stuff

===================================================================*/

CLASS_DECLARATION(Entity, SpriteMaster, nullptr);

Event EV_SpriteMaster_Load("spg_load", EV_CHEAT);
Event EV_SpriteMaster_Save("spg_save", EV_CHEAT);
Event EV_SpriteMaster_File("spg_file", EV_CHEAT);
Event EV_SpriteMaster_Glow("spg_glow", EV_CHEAT);
Event EV_SpriteMaster_Glow_Flicker("spg_glowflicker");
Event EV_SpriteMaster_HitWater("spg_hitwater", EV_CHEAT);
Event EV_SpriteMaster_Next("spg_next", EV_CHEAT);
Event EV_SpriteMaster_Previous("spg_previous", EV_CHEAT);
Event EV_SpriteMaster_Delete("spg_delete", EV_CHEAT);
Event EV_SpriteMaster_Scale("spg_scale", EV_CHEAT);
Event EV_SpriteMaster_Alpha("spg_alpha", EV_CHEAT);
Event EV_SpriteMaster_Frame("spg_frame", EV_CHEAT);
Event EV_SpriteMaster_NextFrame("spg_nextframe", EV_CHEAT);
Event EV_SpriteMaster_PrevFrame("spg_prevframe", EV_CHEAT);
Event EV_SpriteMaster_MoveSize("spg_movesize", EV_CHEAT);
Event EV_SpriteMaster_MoveRight("spg_moveright", EV_CHEAT);
Event EV_SpriteMaster_MoveLeft("spg_moveleft", EV_CHEAT);
Event EV_SpriteMaster_MoveUp("spg_moveup", EV_CHEAT);
Event EV_SpriteMaster_MoveDown("spg_movedown", EV_CHEAT);
Event EV_SpriteMaster_MoveForward("spg_moveforward", EV_CHEAT);
Event EV_SpriteMaster_MoveBack("spg_moveback", EV_CHEAT);
Event EV_SpriteMaster_SetPitch("spg_setpitch", EV_CHEAT);
Event EV_SpriteMaster_SetYaw("spg_setyaw", EV_CHEAT);
Event EV_SpriteMaster_SetRoll("spg_setroll", EV_CHEAT);
Event EV_SpriteMaster_RotSize("spg_rotsize", EV_CHEAT);
Event EV_SpriteMaster_PitchUp("spg_pitchup", EV_CHEAT);
Event EV_SpriteMaster_PitchDown("spg_pitchdown", EV_CHEAT);
Event EV_SpriteMaster_YawRight("spg_yawright", EV_CHEAT);
Event EV_SpriteMaster_YawLeft("spg_yawleft", EV_CHEAT);
Event EV_SpriteMaster_RollRight("spg_rollright", EV_CHEAT);
Event EV_SpriteMaster_RollLeft("spg_rollleft", EV_CHEAT);
Event EV_SpriteMaster_Get("spg_get", EV_CHEAT);
Event EV_SpriteMaster_Give("spg_give", EV_CHEAT);
Event EV_SpriteMaster_Clear("spg_clear", EV_CHEAT);
Event EV_SpriteMaster_Flip("spg_flip", EV_CHEAT);
Event EV_SpriteMaster_AnimSpeed("spg_animspeed", EV_CHEAT);
Event EV_SpriteMaster_AnimDelay("spg_animdelay", EV_CHEAT);
Event EV_SpriteMaster_PitchSpeed("spg_pitchspeed", EV_CHEAT);
Event EV_SpriteMaster_YawSpeed("spg_yawspeed", EV_CHEAT);
Event EV_SpriteMaster_RollSpeed("spg_rollspeed", EV_CHEAT);
Event EV_SpriteMaster_RotDelay("spg_rotdelay", EV_CHEAT);
Event EV_SpriteMaster_BlinkSpeed("spg_blinkspeed", EV_CHEAT);
Event EV_SpriteMaster_BlinkDelay("spg_blinkdelay", EV_CHEAT);
Event EV_SpriteMaster_Targetname("spg_targetname", EV_CHEAT);
Event EV_SpriteMaster_TriggerEffect("spg_triggereffect", EV_CHEAT);

ResponseDef SpriteMaster::Responses[] =
{
   { &EV_SpriteMaster_Load,          (Response)&SpriteMaster::LoadSpriteFile},
   { &EV_SpriteMaster_Save,          (Response)&SpriteMaster::SaveSpriteFile},
   { &EV_SpriteMaster_File,          (Response)&SpriteMaster::SetSprite},
   { &EV_SpriteMaster_Glow,          (Response)&SpriteMaster::ToggleGlow},
   { &EV_SpriteMaster_Glow_Flicker,  (Response)&SpriteMaster::GlowFlicker},
   { &EV_SpriteMaster_HitWater,      (Response)&SpriteMaster::ToggleHitWater},
   { &EV_SpriteMaster_Next,          (Response)&SpriteMaster::Next},
   { &EV_SpriteMaster_Previous,      (Response)&SpriteMaster::Previous},
   { &EV_SpriteMaster_Delete,        (Response)&SpriteMaster::Delete},
   { &EV_SpriteMaster_Scale,         (Response)&SpriteMaster::SetScale},
   { &EV_SpriteMaster_Alpha,         (Response)&SpriteMaster::SetAlpha},
   { &EV_SpriteMaster_Frame,         (Response)&SpriteMaster::SetFrame},
   { &EV_SpriteMaster_NextFrame,     (Response)&SpriteMaster::NextFrame},
   { &EV_SpriteMaster_PrevFrame,     (Response)&SpriteMaster::PrevFrame},
   { &EV_SpriteMaster_MoveSize,      (Response)&SpriteMaster::SetMoveSize},
   { &EV_SpriteMaster_MoveRight,     (Response)&SpriteMaster::MoveRight},
   { &EV_SpriteMaster_MoveLeft,      (Response)&SpriteMaster::MoveLeft},
   { &EV_SpriteMaster_MoveUp,        (Response)&SpriteMaster::MoveUp},
   { &EV_SpriteMaster_MoveDown,      (Response)&SpriteMaster::MoveDown},
   { &EV_SpriteMaster_MoveForward,   (Response)&SpriteMaster::MoveForward},
   { &EV_SpriteMaster_MoveBack,      (Response)&SpriteMaster::MoveBack},
   { &EV_SpriteMaster_SetPitch,      (Response)&SpriteMaster::SetPitch},
   { &EV_SpriteMaster_SetYaw,        (Response)&SpriteMaster::SetYaw},
   { &EV_SpriteMaster_SetRoll,       (Response)&SpriteMaster::SetRoll},
   { &EV_SpriteMaster_RotSize,       (Response)&SpriteMaster::SetRotateSize},
   { &EV_SpriteMaster_PitchUp,       (Response)&SpriteMaster::PitchUp},
   { &EV_SpriteMaster_PitchDown,     (Response)&SpriteMaster::PitchDown},
   { &EV_SpriteMaster_YawRight,      (Response)&SpriteMaster::YawRight},
   { &EV_SpriteMaster_YawLeft,       (Response)&SpriteMaster::YawLeft},
   { &EV_SpriteMaster_RollRight,     (Response)&SpriteMaster::RollRight},
   { &EV_SpriteMaster_RollLeft,      (Response)&SpriteMaster::RollLeft},
   { &EV_SpriteMaster_Get,           (Response)&SpriteMaster::GetSettings},
   { &EV_SpriteMaster_Give,          (Response)&SpriteMaster::GiveSettings},
   { &EV_SpriteMaster_Clear,         (Response)&SpriteMaster::ClearSettings},
   { &EV_SpriteMaster_Flip,          (Response)&SpriteMaster::FlipSprite},
   { &EV_SpriteMaster_AnimSpeed,     (Response)&SpriteMaster::SetAnimSpeed},
   { &EV_SpriteMaster_AnimDelay,     (Response)&SpriteMaster::SetAnimDelay},
   { &EV_SpriteMaster_PitchSpeed,    (Response)&SpriteMaster::SetPitchSpeed},
   { &EV_SpriteMaster_YawSpeed,      (Response)&SpriteMaster::SetYawSpeed},
   { &EV_SpriteMaster_RollSpeed,     (Response)&SpriteMaster::SetRollSpeed},
   { &EV_SpriteMaster_RotDelay,      (Response)&SpriteMaster::SetRotDelay},
   { &EV_SpriteMaster_BlinkSpeed,    (Response)&SpriteMaster::SetBlinkSpeed},
   { &EV_SpriteMaster_BlinkDelay,    (Response)&SpriteMaster::SetBlinkDelay},
   { &EV_SpriteMaster_Targetname,    (Response)&SpriteMaster::SetTargetname},
   { &EV_SpriteMaster_TriggerEffect, (Response)&SpriteMaster::SetTriggerEffect},
   { nullptr, nullptr}
};

SpriteMaster Spritecontrol;

SpriteMaster::SpriteMaster() : Listener()
{
   num_sprites      = 0;
   spritename       = "sprites/blastmark.spr";
   place_scale      = 1;
   place_alpha      = 1;
   place_animspeed  = 0;
   place_animdelay  = 0;
   place_pitchspeed = 0;
   place_yawspeed   = 0;
   place_rollspeed  = 0;
   place_rotdelay   = 0;
   place_blinkspeed = 0;
   place_blinkdelay = 0;
   place_trigeffect = SEFFECT_NONE;

   curr_sprite = 0;
   glowcurrent = false;
   hitwater    = false;
   movesize    = 8;
   rotsize     = 5;
}

/*===================================================================
SpriteMaster : Sprite Index loading stuff
===================================================================*/

// pointer to the frame index list
sf_t *sf_index;
int   sf_num = 0;

void CleanupSpriteGun()
{
   if(sf_index) {
      delete[] sf_index;		/* Discard the previous one */
      sf_index = 0;
   }
}

// SINEX_FIXME: changed by Burger Becky
#if 0
static Word GrabString(StreamHandle_t *Stream, char *Output, Word Length)
{
   do {
      Output[0] = 0; 	/* I'm paranoid */
      StreamHandleGetString2(Stream, EXTRACTSTRDELIMITLF, Output, Length);
      if(Output[0]) {
         return FALSE;
      }
   }
   while(!StreamHandleGetErrorFlag(Stream));
   return TRUE;
}
#else
static int GrabString(void *Stream, char *Output, int Length)
{
   return false;
}
#endif

void SpriteMaster::ParseSpriteIndex()
{
   // SINEX_FIXME: changed by Burger Becky
#if 0
   char tmpchar[128];
   str indexfile;
   int j;
   size_t length;
   char *buffer;
   void *Stream;

   sf_num = -1;

   indexfile = "decals/decals.lst";

   gi.dprintf("using decal list: %s\n", indexfile.c_str());

   length = gi.LoadFile(indexfile.c_str(), (void **)&buffer, 0);
   if(length == (size_t)(-1)) {
      gi.dprintf("Couldn't find decal list: %s\n", indexfile.c_str());
      return;
   }

   // put the data buffer into an easy to use class
   StreamHandleInitGetPtr(&Stream, buffer, length);
   gi.TagFree((void *)buffer);

   // init the list
   GrabString(&Stream, tmpchar, sizeof(tmpchar));
   sf_num = atoi(tmpchar);
   if(sf_index) {
      delete[] sf_index;		/* Discard the previous one */
   }
   sf_index = new sf_t[sf_num];

   // start making the list
   j = 0;
   while(j < sf_num) {
      char *MarkPtr;
      if(GrabString(&Stream, tmpchar, sizeof(tmpchar))) {
         break;
      }
      MarkPtr = strchr(tmpchar, ' ');
      if(!MarkPtr) {
         break;
      }
      MarkPtr[0] = 0;
      sf_index[j].model = tmpchar;
      sf_index[j].frames = atoi(MarkPtr+1);
      j++;
   }
   StreamHandleDestroy(&Stream);
#endif
}

/*===================================================================
SpriteMaster : Sprite File loading stuff
===================================================================*/

void SpriteMaster::LoadSpriteFile(Event *ev)
{
   // get the file name to use for the sprite file
   SetFileName();
   // parse the sprite index before loading in the sprite file
   ParseSpriteIndex();
   // creat the sprites from the file
   ParseSpriteFile();
}

// finds where the map file is and then tries to load the
// sprite file from there. If it finds it, it sets filename.
void SpriteMaster::SetFileName()
{
   filename = "maps\\";
   filename += level.mapname;
   filename += ".msf";
   gi.dprintf("Sprite filename set to: %s\n", filename.c_str());
}

void SpriteMaster::ParseSpriteFile()
{
   // SINEX_FIXME: changed by Burger Becky
#if 0
   int i, num;
   PlacedSprite *sprite;
   Vector tmpvec;
   int tmpint;
   char tmpchar[128];
   size_t length;
   char *sf_buffer;
   void *Stream;

   // first, clear out any existing sprites
   num = 0;
   for(i = 0; i < num_sprites; i++) {
      num = G_FindClass(num, "placedsprite");
      if(!num) // ran out of sprite entities
         break;
      sprite = (PlacedSprite *)G_GetEntity(num);

      sprite->PostEvent(EV_Remove, 0.1);
   }

   length = gi.LoadFile(filename.c_str(), (void **)&sf_buffer, 0);
   if(length == (size_t)(-1))
   {
      gi.dprintf("Couldn't find sprite file: %s\n", filename.c_str());
      return;
   }

   // put the data buffer into an easy to use class
   StreamHandleInitGetPtr(&Stream, sf_buffer, length);
   // free the file buffer since we don't need it any more
   gi.TagFree((void *)sf_buffer);

   // get the number of sprites to read in
   // read in the number of sprites
   GrabString(&Stream, tmpchar, sizeof(tmpchar));
   num_sprites = atoi(tmpchar);

   for(i = 0; i < num_sprites; i++) {
      char *tmpptr;
      sprite = new PlacedSprite;

      sprite->setMoveType(MOVETYPE_NONE);
      sprite->setSolidType(SOLID_NOT);

      GrabString(&Stream, tmpchar, sizeof(tmpchar));

      sprite->setModel(tmpchar);
      sprite->numframes = NumFrames(tmpchar);
      sprite->setSize(vec_zero, vec_zero);

      GrabString(&Stream, tmpchar, sizeof(tmpchar));
      Get3Floats(tmpchar, &tmpvec.x, &tmpvec.y, &tmpvec.z);
      sprite->setOrigin(tmpvec);

      GrabString(&Stream, tmpchar, sizeof(tmpchar));
      Get3Floats(tmpchar, &tmpvec.x, &tmpvec.y, &tmpvec.z);
      sprite->setAngles(tmpvec);
      sprite->placedangles = tmpvec;

      GrabString(&Stream, tmpchar, sizeof(tmpchar));
      GetInt2Floats(tmpchar, &tmpint, &tmpvec.y, &tmpvec.z);

      sprite->placedframe = tmpint;
      sprite->edict->s.frame = sprite->placedframe;
      sprite->edict->s.scale = tmpvec.y;
      sprite->edict->s.alpha = tmpvec.z;
      if(sprite->edict->s.alpha < 1) { // it's transparent
         sprite->edict->s.renderfx |= RF_TRANSLUCENT;
      }

      GrabString(&Stream, tmpchar, sizeof(tmpchar));
      tmpptr = strchr(tmpchar, ' ');
      if(tmpptr) {
         tmpptr[0] = 0;
      }
      if(Q_strcasecmp(tmpchar, "notargetname")) {
         sprite->SetTargetName(tmpchar);
      }

      if(tmpptr) {
         ++tmpptr;
         if(!Q_strcasecmp(tmpptr, "toggle")) {
            sprite->triggereffect = SEFFECT_TOGGLE;
         }
         else if(!Q_strcasecmp(tmpptr, "advance")) {
            sprite->triggereffect = SEFFECT_ADVANCE;
         }
         else if(!Q_strcasecmp(tmpptr, "once")) {
            sprite->triggereffect = SEFFECT_ONCE;
         }
         else {
            sprite->triggereffect = SEFFECT_NONE;
         }
      }
      else {
         sprite->triggereffect = SEFFECT_NONE;
      }

      GrabString(&Stream, tmpchar, sizeof(tmpchar));
      Get2Floats(tmpchar, &tmpvec.y, &tmpvec.z);
      sprite->animatespeed = tmpvec.y;
      sprite->animatedelay = tmpvec.z;
      if(sprite->animatespeed > 0) {
         if(sprite->animatedelay > 0) {
            sprite->PostEvent(EV_PlacedSprite_Animate, sprite->animatespeed + sprite->animatedelay);
         }
         else {
            sprite->PostEvent(EV_PlacedSprite_Animate, sprite->animatespeed);
         }
      }

      GrabString(&Stream, tmpchar, sizeof(tmpchar));
      Get2Floats(tmpchar, &tmpvec.y, &tmpvec.z);
      sprite->blinkspeed = tmpvec.y;
      sprite->blinkdelay = tmpvec.z;
      if(sprite->blinkspeed) {
         if(sprite->blinkdelay > 0) {
            sprite->PostEvent(EV_PlacedSprite_Blink, sprite->blinkspeed + sprite->blinkdelay);
         }
         else {
            sprite->PostEvent(EV_PlacedSprite_Blink, sprite->blinkspeed);
         }
      }

      GrabString(&Stream, tmpchar, sizeof(tmpchar));
      Get3Int1Float(tmpchar, &sprite->pitchspeed, &sprite->yawspeed, &sprite->rollspeed, &tmpvec.x);
      sprite->rotatedelay = tmpvec.x;
      if(sprite->pitchspeed || sprite->yawspeed || sprite->rollspeed) {
         if(sprite->rotatedelay > 0) {
            sprite->PostEvent(EV_PlacedSprite_Rotate, 0.1 + sprite->rotatedelay);
         }
         else {
            sprite->PostEvent(EV_PlacedSprite_Rotate, 0.1);
         }
      }

      //setup for when glowing is turned on
      sprite->edict->s.color_r = 1;
      sprite->edict->s.color_g = 1;
      sprite->edict->s.color_b = 1;
      sprite->edict->s.radius = 150;

      sprite->link();

      if(StreamHandleGetErrorFlag(&Stream)) { // ran out of data too soon 
         StreamHandleDestroy(&Stream);
         gi.error("Premature EOF in the sprite file %s\n", filename.c_str());
         return;
      }

      // set current sprite
      curr_sprite = sprite->entnum;
   }
   StreamHandleDestroy(&Stream);
   gi.dprintf("Sprite file loaded\n");
#endif
}

void SpriteMaster::SaveSpriteFile(Event *ev)
{
   // SINEX_FIXME: changed by Burger Becky
#if 0
   int i, num;
   PlacedSprite *sprite;
   str spritefile;
   char Buffer[512];

   // only save sprite files if in developer mode
   if(!developer->value) {
      return;
   }

   // set the full file path to save to
   if(gi.GameDir()) {
      spritefile = gi.GameDir();
      spritefile += "\\";
      spritefile += filename;
   }
   else {
      spritefile = "base\\";
      spritefile += filename;
   }

   // make sure the file exists to write
   gi.CreatePath(spritefile.c_str());

   // open the file for writting
   void *OutStream;
   StreamHandleInitPut(&OutStream);

   snprintf(Buffer, sizeof(Buffer), "%d\n\n", num_sprites);
   StreamHandlePutStringNoZero(&OutStream, Buffer);

   // go through and save all the sprites to the file
   num = 0;
   for(i = 0; i < num_sprites; i++) {
      num = G_FindClass(num, "placedsprite");
      if(!num) // ran out of sprite entities
         break;
      sprite = (PlacedSprite *)G_GetEntity(num);

      StreamHandlePutStringNoZero(&OutStream, sprite->model.c_str());

      snprintf(Buffer, sizeof(Buffer), "\n%f %f %f\n", sprite->origin.x, sprite->origin.y, sprite->origin.z);
      StreamHandlePutStringNoZero(&OutStream, Buffer);

      snprintf(Buffer, sizeof(Buffer), "%f %f %f\n", sprite->placedangles.x, sprite->placedangles.y, sprite->placedangles.z);
      StreamHandlePutStringNoZero(&OutStream, Buffer);

      snprintf(Buffer, sizeof(Buffer), "%d %f %f\n", sprite->placedframe, sprite->edict->s.scale, sprite->edict->s.alpha);
      StreamHandlePutStringNoZero(&OutStream, Buffer);

      if(strcmp(sprite->targetname.c_str(), "")) {
         StreamHandlePutStringNoZero(&OutStream, sprite->targetname.c_str());
      }
      else {
         StreamHandlePutStringNoZero(&OutStream, "notargetname");
      }
      StreamHandlePutByte(&OutStream, ' ');
      switch(sprite->triggereffect) {
         //		case SEFFECT_NONE:
      default:
         StreamHandlePutStringNoZero(&OutStream, "none\n");
         break;
      case SEFFECT_TOGGLE:
         StreamHandlePutStringNoZero(&OutStream, "toggle\n");
         break;
      case SEFFECT_ADVANCE:
         StreamHandlePutStringNoZero(&OutStream, "advance\n");
         break;
      case SEFFECT_ONCE:
         StreamHandlePutStringNoZero(&OutStream, "once\n");
         break;
      }
      sprintf(Buffer, "%f %f\n", sprite->animatespeed, sprite->animatedelay);
      StreamHandlePutStringNoZero(&OutStream, Buffer);

      sprintf(Buffer, "%f %f\n", sprite->blinkspeed, sprite->blinkdelay);
      StreamHandlePutStringNoZero(&OutStream, Buffer);

      sprintf(Buffer, "%d %d %d %f\n\n", sprite->pitchspeed, sprite->yawspeed, sprite->rollspeed, sprite->rotatedelay);
      StreamHandlePutStringNoZero(&OutStream, Buffer);
   }

   StreamHandleEndSave(&OutStream);	/* Wrap up */
   if(StreamHandleSaveTextFile(&OutStream, spritefile.c_str())) {  // couldn't create or find the file for writting
      StreamHandleDestroy(&OutStream);
      gi.dprintf("Couldn't open sprite file for writing: %s\n", spritefile.c_str());
      return;
   }
   StreamHandleDestroy(&OutStream);
   gi.dprintf("Sprite file saved\n");
#endif
}

/*===================================================================
SpriteMaster : frame number return function
===================================================================*/

int SpriteMaster::NumFrames(const char *testmodel)
{
   int num;

   num = gi.Anim_NumFrames(gi.modelindex(testmodel), 0);
   if(num) // it's a model so use the engine to count frames
      return num;

   // try loading a list if don't have one yet
   if(!sf_num)
   {
      ParseSpriteIndex();
   }

   // just always return 1 if there's no list to be found
   if(sf_num < 0)
      return 1;

   // it's a sprite, so look it up in our index list
   for(num = 0; num < sf_num; num++)
   {
      if(!Q_strcasecmp(sf_index[num].model.c_str(), testmodel)) // found a match
         return sf_index[num].frames;
   }
   // couldn't find a match, so just return 1
   return 1;
}

/*===================================================================
SpriteMaster : Sprite control commands
===================================================================*/

void SpriteMaster::SetSprite(Event *ev)
{
   const char *newspr;
   char str[128], tmpstr[128], *strptr;

   if(ev->NumArgs())
      newspr = ev->GetString(1);
   else
      newspr = nullptr;

   // display current sprite if none specified
   if(!newspr || !newspr[0])
   {
      gi.dprintf("spg_file = %s\n", spritename);
      return;
   }

   // attach the needed directory stuff to the file name
   str[0] = 0;
   if(newspr[1] != ':')
   {
      strcpy(tmpstr, newspr);
      strptr = tmpstr;
      while((strptr[0] != '.') && (strptr[0] != 0))
         strptr++;

      // if no extension given, assume a decal
      if(strptr[0] == 0)
      {
         // prepend decals/ if needed
         if(Q_strncasecmp(newspr, "decals/", 7))
         {
            strcpy(str, "decals/");
         }
         strcat(str, newspr);
         strcat(str, ".spr");
      }
      else // basically determine if it's a sprite or a model
      {
         if(!strcmp(strptr, ".def")) // it's a model
         {
            // prepend models/ if needed
            if(Q_strncasecmp(newspr, "models/", 7))
            {
               strcpy(str, "models/");
            }
         }
         else if(!strcmp(strptr, ".spr")) // it's a sprite
         {
            // prepend sprites/ if needed
            if(Q_strncasecmp(newspr, "sprites/", 8))
            {
               strcpy(str, "sprites/");
            }
         }
         else // it's something not right
         {
            ev->Error("%s is bad extension. Must be str or def\n", strptr);
            return;
         }
         strcat(str, newspr);
      }
   }

   //set value if given a valid sprite name
   if(gi.modelindex(str) > 0)
   {
      strcpy(spritename, str);
      gi.dprintf("spg_file = %s\n", spritename);
   }
   else
      ev->Error("Must specify a valid sprite or model name");

   // set new sprite model for currently selected sprite
   if(curr_sprite)
   {
      PlacedSprite *sprite;

      sprite = (PlacedSprite *)G_GetEntity(curr_sprite);
      sprite->setModel(spritename);
      sprite->numframes = NumFrames(spritename);
   }
}

void SpriteMaster::ToggleGlow(Event *ev)
{
   PlacedSprite *sprite;

   if(curr_sprite)
      sprite = (PlacedSprite *)G_GetEntity(curr_sprite);

   if(glowcurrent)
   {
      glowcurrent = false;
      if(curr_sprite)
         sprite->edict->s.renderfx &= ~RF_DLIGHT;
      CancelEventsOfType(EV_SpriteMaster_Glow_Flicker);
      gi.dprintf("Current sprite glow off\n");
   }
   else
   {
      glowcurrent = true;
      if(curr_sprite)
         sprite->edict->s.renderfx |= RF_DLIGHT;
      PostEvent(EV_SpriteMaster_Glow_Flicker, 0.1);
      gi.dprintf("Current sprite glow on\n");
   }
}

void SpriteMaster::GlowFlicker(Event *ev)
{
   if(curr_sprite)
   {
      PlacedSprite *sprite;
      int i;

      sprite = static_cast<PlacedSprite *>(G_GetEntity(curr_sprite));
      sprite->edict->s.radius = 100 + random()*50;

      // also draw a debug line from the player to the sprite
      for(i = 1; i <= maxclients->value; i++)
      {
         if(!g_edicts[i].inuse || !g_edicts[i].entity)
         {
            continue;
         }

         // we found a valid player entity to draw to
         break;
      }

      if(i <= maxclients->value)
      {
         Entity *ent;

         ent = g_edicts[i].entity;
         G_DebugLine(ent->centroid, sprite->origin, 1, 1, 1, random()*0.8 + 0.2);
      }
   }
   PostEvent(EV_SpriteMaster_Glow_Flicker, 0.1);
}

void SpriteMaster::ToggleHitWater(Event *ev)
{
   if(hitwater)
   {
      hitwater = false;
      gi.dprintf("water surface tracing off\n");
   }
   else
   {
      hitwater = true;
      gi.dprintf("water surface tracing on\n");
   }
}

void SpriteMaster::Next(Event *ev)
{
   PlacedSprite *sprite;
   int next;

   if(!curr_sprite)
   {
      gi.dprintf("no sprites in the level\n");
      return;
   }

   // turn off glow on current sprite
   if(glowcurrent)
   {
      sprite = static_cast<PlacedSprite *>(G_GetEntity(curr_sprite));
      sprite->edict->s.renderfx &= ~RF_DLIGHT;
   }

   next = G_FindClass(curr_sprite, "placedsprite");

   if(next)
   {
      curr_sprite = next;

      sprite = static_cast<PlacedSprite *>(G_GetEntity(curr_sprite));
      if(glowcurrent) // turn on glow if needed
         sprite->edict->s.renderfx |= RF_DLIGHT;
      gi.dprintf("next sprite selected\n");
   }
   else
   {
      next = G_FindClass(0, "placedsprite");
      curr_sprite = next;
      sprite = static_cast<PlacedSprite *>(G_GetEntity(curr_sprite));
      if(glowcurrent) // turn on glow if needed
         sprite->edict->s.renderfx |= RF_DLIGHT;
      gi.dprintf("first sprite selected\n");
   }
}

void SpriteMaster::Previous(Event *ev)
{
   PlacedSprite *sprite;
   int prev, next;

   if(!curr_sprite)
   {
      gi.dprintf("no sprites in the level\n");
      return;
   }

   next = 0;
   do
   {
      prev = next;
      next = G_FindClass(prev, "placedsprite");
   }
   while(next != curr_sprite);

   if(prev)
   {
      // turn off glow on current sprite
      if(glowcurrent)
      {
         sprite = static_cast<PlacedSprite *>(G_GetEntity(curr_sprite));
         sprite->edict->s.renderfx &= ~RF_DLIGHT;
      }

      curr_sprite = prev;

      sprite = static_cast<PlacedSprite *>(G_GetEntity(curr_sprite));
      if(glowcurrent) // turn on glow if needed
         sprite->edict->s.renderfx |= RF_DLIGHT;
      gi.dprintf("previous sprite selected\n");
   }
   else
   {
      gi.dprintf("first sprite already selected.\n");
   }
}

void SpriteMaster::Delete(Event *ev)
{
   PlacedSprite *sprite;
   int curr;

   if(!curr_sprite)
   {
      gi.dprintf("no sprite to delete\n");
      return;
   }

   sprite = static_cast<PlacedSprite *>(G_GetEntity(curr_sprite));
   curr = curr_sprite;
   ProcessEvent(EV_SpriteMaster_Next);
   sprite->ProcessEvent(EV_Remove);
   // there was only one sprite in the level
   if(curr == curr_sprite)
      curr_sprite = 0;
   // decrement the sprite counter
   num_sprites--;

   gi.dprintf("sprite deleted\n");
}

void SpriteMaster::SetPitch(Event *ev)
{
   const char *newrot;
   PlacedSprite *sprite;

   if(!curr_sprite)
   {
      gi.dprintf("no sprite to rotate\n");
      return;
   }

   if(ev->NumArgs())
      newrot = ev->GetString(1);
   else
      newrot = nullptr;

   sprite = static_cast<PlacedSprite *>(G_GetEntity(curr_sprite));

   // display current sprite if none specified
   if(!newrot || !newrot[0])
   {
      gi.dprintf("sprite pitch = %i\n", sprite->angles[PITCH]);
      return;
   }

   sprite->placedangles[PITCH] = (float)atof(newrot);
   sprite->angles[PITCH] = sprite->placedangles[PITCH];
   sprite->setAngles(sprite->angles);
   gi.dprintf("sprite pitch = %i\n", sprite->angles[PITCH]);
}

void SpriteMaster::SetYaw(Event *ev)
{
   const char *newrot;
   PlacedSprite *sprite;

   if(!curr_sprite)
   {
      gi.dprintf("no sprite to rotate\n");
      return;
   }

   if(ev->NumArgs())
      newrot = ev->GetString(1);
   else
      newrot = nullptr;

   sprite = static_cast<PlacedSprite *>(G_GetEntity(curr_sprite));

   // display current sprite if none specified
   if(!newrot || !newrot[0])
   {
      gi.dprintf("sprite yaw = %i\n", sprite->angles[YAW]);
      return;
   }

   sprite->placedangles[YAW] = (float)atof(newrot);
   sprite->angles[YAW] = sprite->placedangles[PITCH];
   sprite->setAngles(sprite->angles);
   gi.dprintf("sprite yaw = %i\n", sprite->angles[YAW]);
}

void SpriteMaster::SetRoll(Event *ev)
{
   const char *newrot;
   PlacedSprite *sprite;

   if(!curr_sprite)
   {
      gi.dprintf("no sprite to rotate\n");
      return;
   }

   if(ev->NumArgs())
      newrot = ev->GetString(1);
   else
      newrot = nullptr;

   sprite = static_cast<PlacedSprite *>(G_GetEntity(curr_sprite));

   // display current sprite if none specified
   if(!newrot || !newrot[0])
   {
      gi.dprintf("sprite roll = %i\n", (int)sprite->angles[ROLL]);
      return;
   }

   sprite->placedangles[ROLL] = (float)atof(newrot);
   sprite->angles[ROLL] = sprite->placedangles[PITCH];
   sprite->setAngles(sprite->angles);
   gi.dprintf("sprite roll = %i\n", (int)sprite->angles[ROLL]);
}

void SpriteMaster::SetRotateSize(Event *ev)
{
   const char *newmove;

   if(ev->NumArgs())
      newmove = ev->GetString(1);
   else
      newmove = nullptr;

   // display current sprite if none specified
   if(!newmove || !newmove[0])
   {
      gi.dprintf("spg_rotsize = %i\n", rotsize);
      return;
   }

   rotsize = atoi(newmove);
   gi.dprintf("spg_rotsize = %i\n", rotsize);
}

void SpriteMaster::PitchUp(Event *ev)
{
   PlacedSprite *sprite;

   if(!curr_sprite)
   {
      gi.dprintf("no sprite to rotate\n");
      return;
   }

   sprite = static_cast<PlacedSprite *>(G_GetEntity(curr_sprite));
   sprite->angles[PITCH] -= rotsize;
   sprite->setAngles(sprite->angles);

   sprite->placedangles[PITCH] -= rotsize;
   if(sprite->placedangles[PITCH] < 0)
      sprite->placedangles[PITCH] += 360;

   gi.dprintf("sprite pitch = %i\n", (int)sprite->placedangles[PITCH]);
}

void SpriteMaster::PitchDown(Event *ev)
{
   PlacedSprite *sprite;

   if(!curr_sprite)
   {
      gi.dprintf("no sprite to rotate\n");
      return;
   }

   sprite = static_cast<PlacedSprite *>(G_GetEntity(curr_sprite));
   sprite->angles[PITCH] += rotsize;
   sprite->setAngles(sprite->angles);

   sprite->placedangles[PITCH] += rotsize;
   if(sprite->placedangles[PITCH] > 360)
      sprite->placedangles[PITCH] -= 360;

   gi.dprintf("sprite pitch = %i\n", (int)sprite->placedangles[PITCH]);
}

void SpriteMaster::YawRight(Event *ev)
{
   PlacedSprite *sprite;

   if(!curr_sprite)
   {
      gi.dprintf("no sprite to rotate\n");
      return;
   }

   sprite = static_cast<PlacedSprite *>(G_GetEntity(curr_sprite));
   sprite->angles[YAW] += rotsize;
   sprite->setAngles(sprite->angles);

   sprite->placedangles[YAW] += rotsize;
   if(sprite->placedangles[YAW] > 360)
      sprite->placedangles[YAW] -= 360;

   gi.dprintf("sprite yaw = %i\n", (int)sprite->placedangles[YAW]);
}

void SpriteMaster::YawLeft(Event *ev)
{
   PlacedSprite *sprite;

   if(!curr_sprite)
   {
      gi.dprintf("no sprite to rotate\n");
      return;
   }

   sprite = static_cast<PlacedSprite *>(G_GetEntity(curr_sprite));
   sprite->angles[YAW] -= rotsize;
   sprite->setAngles(sprite->angles);

   sprite->placedangles[YAW] -= rotsize;
   if(sprite->placedangles[YAW] < 0)
      sprite->placedangles[YAW] += 360;

   gi.dprintf("sprite yaw = %i\n", (int)sprite->placedangles[YAW]);
}

void SpriteMaster::RollRight(Event *ev)
{
   PlacedSprite *sprite;

   if(!curr_sprite)
   {
      gi.dprintf("no sprite to rotate\n");
      return;
   }

   sprite = static_cast<PlacedSprite *>(G_GetEntity(curr_sprite));
   sprite->angles[ROLL] += rotsize;
   sprite->setAngles(sprite->angles);

   sprite->placedangles[ROLL] += rotsize;
   if(sprite->placedangles[ROLL] > 360)
      sprite->placedangles[ROLL] -= 360;

   gi.dprintf("sprite roll = %i\n", (int)sprite->placedangles[ROLL]);
}

void SpriteMaster::RollLeft(Event *ev)
{
   PlacedSprite *sprite;

   if(!curr_sprite)
   {
      gi.dprintf("no sprite to rotate\n");
      return;
   }

   sprite = static_cast<PlacedSprite *>(G_GetEntity(curr_sprite));
   sprite->angles[ROLL] -= rotsize;
   sprite->setAngles(sprite->angles);

   sprite->placedangles[ROLL] -= rotsize;
   if(sprite->placedangles[ROLL] < 0)
      sprite->placedangles[ROLL] += 360;

   gi.dprintf("sprite roll = %i\n", (int)sprite->placedangles[ROLL]);
}

void SpriteMaster::SetScale(Event *ev)
{
   const char *newscale;

   if(ev->NumArgs())
      newscale = ev->GetString(1);
   else
      newscale = nullptr;

   // display current sprite if none specified
   if(!newscale || !newscale[0])
   {
      gi.dprintf("spg_scale = %f\n", place_scale);
      return;
   }

   place_scale = (float)atof(newscale);
   if(place_scale < 0.0001)
      place_scale = 0.0001;
   gi.dprintf("spg_scale = %f\n", place_scale);

   if(curr_sprite)
   {
      auto sprite = static_cast<PlacedSprite *>(G_GetEntity(curr_sprite));
      sprite->edict->s.scale = place_scale;
   }
}

void SpriteMaster::SetAlpha(Event *ev)
{
   const char *newalpha;

   if(ev->NumArgs())
      newalpha = ev->GetString(1);
   else
      newalpha = nullptr;

   // display current sprite if none specified
   if(!newalpha || !newalpha[0])
   {
      gi.dprintf("spg_alpha = %f\n", place_alpha);
      return;
   }

   place_alpha = (float)atof(newalpha);
   if(place_alpha < 0.001)
      place_alpha = 0.001;
   else if(place_alpha > 1)
      place_alpha = 1;
   gi.dprintf("spg_alpha = %f\n", place_alpha);

   if(curr_sprite)
   {
      auto sprite = static_cast<PlacedSprite *>(G_GetEntity(curr_sprite));
      sprite->edict->s.alpha = place_alpha;
      if(sprite->edict->s.alpha < 1)
         sprite->edict->s.renderfx |= RF_TRANSLUCENT;
      else
         sprite->edict->s.renderfx &= ~RF_TRANSLUCENT;
   }
}

void SpriteMaster::SetFrame(Event *ev)
{
   const char *newframe;
   int maxframes;

   if(ev->NumArgs())
      newframe = ev->GetString(1);
   else
      newframe = nullptr;

   // display current sprite if none specified
   if(!newframe || !newframe[0])
   {
      gi.dprintf("spg_frame = %i\n", place_frame);
      return;
   }

   place_frame = atoi(newframe);
   maxframes = NumFrames(spritename);
   if(place_frame < 0)
      place_frame = 0;
   else if(place_frame > maxframes)
      place_frame = maxframes;
   gi.dprintf("spg_frame = %i\n", place_frame);

   if(curr_sprite)
   {
      auto sprite = static_cast<PlacedSprite *>(G_GetEntity(curr_sprite));
      sprite->edict->s.frame = place_frame;
      if(sprite->edict->s.frame >= sprite->numframes)
         sprite->edict->s.frame = 0;
      sprite->link();
   }
}

void SpriteMaster::NextFrame(Event *ev)
{
   int maxframes;

   place_frame++;
   maxframes = NumFrames(spritename);
   if(place_frame > maxframes)
      place_frame = maxframes;
   gi.dprintf("spg_frame = %i\n", place_frame);

   if(curr_sprite)
   {
      auto sprite = static_cast<PlacedSprite *>(G_GetEntity(curr_sprite));
      sprite->edict->s.frame = place_frame;
      if(sprite->edict->s.frame >= sprite->numframes)
         sprite->edict->s.frame = 0;
      sprite->link();
   }
}

void SpriteMaster::PrevFrame(Event *ev)
{
   int maxframes;

   place_frame--;
   maxframes = NumFrames(spritename);
   if(place_frame < 0)
      place_frame = maxframes - 1;
   gi.dprintf("spg_frame = %i\n", place_frame);

   if(curr_sprite)
   {
      auto sprite = static_cast<PlacedSprite *>(G_GetEntity(curr_sprite));
      sprite->edict->s.frame = place_frame;
      if(sprite->edict->s.frame >= sprite->numframes)
         sprite->edict->s.frame = 0;
      sprite->link();
   }
}

void SpriteMaster::SetMoveSize(Event *ev)
{
   const char *newmove;

   if(ev->NumArgs())
      newmove = ev->GetString(1);
   else
      newmove = nullptr;

   // display current sprite if none specified
   if(!newmove || !newmove[0])
   {
      gi.dprintf("spg_movesize = %f\n", movesize);
      return;
   }

   movesize = (float)atof(newmove);
   if(movesize < 1)
      movesize = 1;
   gi.dprintf("spg_movesize = %f\n", movesize);
}

void SpriteMaster::MoveRight(Event *ev)
{
   PlacedSprite *sprite;
   Vector tmpvec;

   if(!curr_sprite)
   {
      gi.dprintf("no sprite to move\n");
      return;
   }

   sprite = static_cast<PlacedSprite *>(G_GetEntity(curr_sprite));
   tmpvec = sprite->origin;
   tmpvec -= Vector(sprite->orientation[1])*movesize;
   sprite->setOrigin(tmpvec);
   gi.dprintf("sprite moved right\n");
}

void SpriteMaster::MoveLeft(Event *ev)
{
   PlacedSprite *sprite;
   Vector tmpvec;

   if(!curr_sprite)
   {
      gi.dprintf("no sprite to move\n");
      return;
   }

   sprite = static_cast<PlacedSprite *>(G_GetEntity(curr_sprite));
   tmpvec = sprite->origin;
   tmpvec += Vector(sprite->orientation[1])*movesize;
   sprite->setOrigin(tmpvec);
   gi.dprintf("sprite moved left\n");
}

void SpriteMaster::MoveUp(Event *ev)
{
   PlacedSprite *sprite;
   Vector tmpvec;

   if(!curr_sprite)
   {
      gi.dprintf("no sprite to move\n");
      return;
   }

   sprite = static_cast<PlacedSprite *>(G_GetEntity(curr_sprite));
   tmpvec = sprite->origin;
   tmpvec += Vector(sprite->orientation[2])*movesize;
   sprite->setOrigin(tmpvec);
   gi.dprintf("sprite moved up\n");
}

void SpriteMaster::MoveDown(Event *ev)
{
   PlacedSprite *sprite;
   Vector tmpvec;

   if(!curr_sprite)
   {
      gi.dprintf("no sprite to move\n");
      return;
   }

   sprite = static_cast<PlacedSprite *>(G_GetEntity(curr_sprite));
   tmpvec = sprite->origin;
   tmpvec -= Vector(sprite->orientation[2])*movesize;
   sprite->setOrigin(tmpvec);
   gi.dprintf("sprite moved down\n");
}

void SpriteMaster::MoveForward(Event *ev)
{
   PlacedSprite *sprite;
   Vector tmpvec;

   if(!curr_sprite)
   {
      gi.dprintf("no sprite to move\n");
      return;
   }

   sprite = static_cast<PlacedSprite *>(G_GetEntity(curr_sprite));
   tmpvec = sprite->origin;
   tmpvec -= Vector(sprite->orientation[0])*movesize;
   sprite->setOrigin(tmpvec);
   gi.dprintf("sprite moved forward\n");
}

void SpriteMaster::MoveBack(Event *ev)
{
   PlacedSprite *sprite;
   Vector tmpvec;

   if(!curr_sprite)
   {
      gi.dprintf("no sprite to move\n");
      return;
   }

   sprite = static_cast<PlacedSprite *>(G_GetEntity(curr_sprite));
   tmpvec = sprite->origin;
   tmpvec += Vector(sprite->orientation[0])*movesize;
   sprite->setOrigin(tmpvec);
   gi.dprintf("sprite moved back\n");
}

void SpriteMaster::GetSettings(Event *ev)
{
   PlacedSprite *sprite;

   if(!curr_sprite)
   {
      gi.dprintf("no sprite to move\n");
      return;
   }

   sprite = static_cast<PlacedSprite *>(G_GetEntity(curr_sprite));

   strcpy(spritename, sprite->model.c_str());
   gi.dprintf("spg_file = %s\n", spritename);

   place_frame = sprite->placedframe;
   gi.dprintf("spg_frame = %i\n", place_frame);

   place_scale = sprite->edict->s.scale;
   gi.dprintf("spg_scale = %f\n", place_scale);

   place_alpha = sprite->edict->s.alpha;
   gi.dprintf("spg_alpha = %f\n", place_alpha);

   place_animspeed = sprite->animatespeed;
   gi.dprintf("spg_animspeed = %f\n", place_animspeed);

   place_animdelay = sprite->animatedelay;
   gi.dprintf("spg_animdelay = %f\n", place_animdelay);

   place_pitchspeed = sprite->pitchspeed;
   gi.dprintf("spg_pitchspeed = %f\n", place_pitchspeed);

   place_yawspeed = sprite->yawspeed;
   gi.dprintf("spg_yawspeed = %f\n", place_yawspeed);

   place_rollspeed = sprite->rollspeed;
   gi.dprintf("spg_rollspeed = %f\n", place_rollspeed);

   place_rotdelay = sprite->rotatedelay;
   gi.dprintf("spg_rotdelay = %f\n", place_rotdelay);

   place_blinkspeed = sprite->blinkspeed;
   gi.dprintf("spg_blinkspeed = %f\n", place_blinkspeed);

   place_blinkdelay = sprite->blinkdelay;
   gi.dprintf("spg_blinkdelay = %f\n", place_blinkdelay);

   place_trigeffect = sprite->triggereffect;
   switch(place_trigeffect)
   {
   case SEFFECT_NONE:
      gi.dprintf("spg_triggereffect = none\n");
      break;
   case SEFFECT_TOGGLE:
      gi.dprintf("spg_triggereffect = toggle\n");
      break;
   case SEFFECT_ADVANCE:
      gi.dprintf("spg_triggereffect = advance\n");
      break;
   case SEFFECT_ONCE:
      gi.dprintf("spg_triggereffect = once\n");
      break;
   }
}

void SpriteMaster::GiveSettings(Event *ev)
{
   PlacedSprite *sprite;

   if(!curr_sprite)
   {
      gi.dprintf("no sprite to set\n");
      return;
   }

   sprite = static_cast<PlacedSprite *>(G_GetEntity(curr_sprite));
   sprite->setModel(spritename);
   sprite->numframes = NumFrames(spritename);
   sprite->placedframe = place_frame;
   if(sprite->placedframe >= sprite->numframes)
      sprite->placedframe = 0;
   sprite->edict->s.frame = sprite->placedframe;
   sprite->edict->s.scale = place_scale;
   sprite->edict->s.alpha = place_alpha;
   if(sprite->edict->s.alpha < 1)
      sprite->edict->s.renderfx |= RF_TRANSLUCENT;
   else
      sprite->edict->s.renderfx &= ~RF_TRANSLUCENT;
   sprite->animatespeed = place_animspeed;
   sprite->animatedelay = place_animdelay;
   sprite->pitchspeed = place_pitchspeed;
   sprite->yawspeed = place_yawspeed;
   sprite->rollspeed = place_rollspeed;
   sprite->rotatedelay = place_rotdelay;
   sprite->blinkspeed = place_blinkspeed;
   sprite->blinkdelay = place_blinkdelay;
   sprite->triggereffect = place_trigeffect;

   gi.dprintf("sprite now has all current settings\n");
}

void SpriteMaster::ClearSettings(Event *ev)
{
   spritename = "sprites/blastmark.spr";
   place_scale = 1;
   place_alpha = 1;
   place_animspeed = 0;
   place_animdelay = 0;
   place_pitchspeed = 0;
   place_yawspeed = 0;
   place_rollspeed = 0;
   place_rotdelay = 0;
   place_blinkspeed = 0;
   place_blinkdelay = 0;
   place_trigeffect = SEFFECT_NONE;

   gi.dprintf("all current settings have been cleared\n");
}

void SpriteMaster::FlipSprite(Event *ev)
{
   PlacedSprite *sprite;
   Vector norm;
   float currrot;

   if(!curr_sprite)
   {
      gi.dprintf("no sprite to flip\n");
      return;
   }

   sprite = static_cast<PlacedSprite *>(G_GetEntity(curr_sprite));

   norm = Vector(sprite->orientation[0])*(-1);
   norm.x = -norm.x;
   norm.y = -norm.y;
   currrot = sprite->angles[ROLL];
   sprite->angles = norm.toAngles();
   sprite->angles[ROLL] = currrot;
   sprite->setAngles(sprite->angles);
}

void SpriteMaster::SetTargetname(Event *ev)
{
   PlacedSprite *sprite;
   const char *newname;

   if(!curr_sprite)
   {
      gi.dprintf("no sprite to set\n");
      return;
   }

   sprite = static_cast<PlacedSprite *>(G_GetEntity(curr_sprite));

   if(ev->NumArgs())
      newname = ev->GetString(1);
   else
      newname = nullptr;

   // display current sprite if none specified
   if(!newname || !newname[0])
   {
      if(sprite->targetname.length())
         gi.dprintf("sprite targetname = %s\n", sprite->targetname.c_str());
      else
         gi.dprintf("sprite targetname = (null)\n");
      return;
   }

   sprite->SetTargetName(newname);
   gi.dprintf("sprite targetname = %s\n", sprite->targetname.c_str());
}

void SpriteMaster::SetTriggerEffect(Event *ev)
{
   PlacedSprite *sprite;
   const char *neweffect;

   if(!curr_sprite)
   {
      gi.dprintf("no sprite to set\n");
      return;
   }

   sprite = static_cast<PlacedSprite *>(G_GetEntity(curr_sprite));

   if(ev->NumArgs())
      neweffect = ev->GetString(1);
   else
      neweffect = nullptr;

   // display current sprite if none specified
   if(!neweffect || !neweffect[0])
   {
      switch(sprite->triggereffect)
      {
      case SEFFECT_NONE:
         gi.dprintf("sprite triggereffect = none\n");
         break;
      case SEFFECT_TOGGLE:
         gi.dprintf("sprite triggereffect = toggle\n");
         break;
      case SEFFECT_ADVANCE:
         gi.dprintf("sprite triggereffect = advance\n");
         break;
      case SEFFECT_ONCE:
         gi.dprintf("sprite triggereffect = once\n");
         break;
      }
      return;
   }

   if(Q_strcasecmp(neweffect, "toggle") == 0)
      sprite->triggereffect = SEFFECT_TOGGLE;
   else if(Q_strcasecmp(neweffect, "advance") == 0)
      sprite->triggereffect = SEFFECT_ADVANCE;
   else if(Q_strcasecmp(neweffect, "once") == 0)
      sprite->triggereffect = SEFFECT_ONCE;
   else
      sprite->triggereffect = SEFFECT_NONE;

   switch(sprite->triggereffect)
   {
   case SEFFECT_NONE:
      gi.dprintf("sprite triggereffect = none\n");
      break;
   case SEFFECT_TOGGLE:
      gi.dprintf("sprite triggereffect = toggle\n");
      break;
   case SEFFECT_ADVANCE:
      gi.dprintf("sprite triggereffect = advance\n");
      break;
   case SEFFECT_ONCE:
      gi.dprintf("sprite triggereffect = once\n");
      break;
   }
}

void SpriteMaster::SetAnimSpeed(Event *ev)
{
   PlacedSprite *sprite;
   const char *newspeed;

   if(!curr_sprite)
   {
      gi.dprintf("no sprite to set\n");
      return;
   }

   sprite = static_cast<PlacedSprite *>(G_GetEntity(curr_sprite));

   if(ev->NumArgs())
      newspeed = ev->GetString(1);
   else
      newspeed = nullptr;

   // display current sprite if none specified
   if(!newspeed || !newspeed[0])
   {
      gi.dprintf("sprite animspeed = %f\n", sprite->animatespeed);
      return;
   }

   sprite->animatespeed = (float)atof(newspeed);
   sprite->CancelEventsOfType(EV_PlacedSprite_Animate);
   if(sprite->animatespeed > 0)
      sprite->PostEvent(EV_PlacedSprite_Animate, sprite->animatespeed);
   gi.dprintf("sprite animspeed = %f\n", sprite->animatespeed);
}

void SpriteMaster::SetAnimDelay(Event *ev)
{
   PlacedSprite *sprite;
   const char *newspeed;

   if(!curr_sprite)
   {
      gi.dprintf("no sprite to set\n");
      return;
   }

   sprite = static_cast<PlacedSprite *>(G_GetEntity(curr_sprite));

   if(ev->NumArgs())
      newspeed = ev->GetString(1);
   else
      newspeed = nullptr;

   // display current sprite if none specified
   if(!newspeed || !newspeed[0])
   {
      gi.dprintf("sprite animdelay = %f\n", sprite->animatedelay);
      return;
   }

   sprite->animatedelay = (float)atof(newspeed);
   gi.dprintf("sprite animdelay = %f\n", sprite->animatedelay);
}

void SpriteMaster::SetPitchSpeed(Event *ev)
{
   PlacedSprite *sprite;
   const char *newspeed;

   if(!curr_sprite)
   {
      gi.dprintf("no sprite to set\n");
      return;
   }

   sprite = static_cast<PlacedSprite *>(G_GetEntity(curr_sprite));

   if(ev->NumArgs())
      newspeed = ev->GetString(1);
   else
      newspeed = nullptr;

   // display current sprite if none specified
   if(!newspeed || !newspeed[0])
   {
      gi.dprintf("sprite pitch speed = %i\n", sprite->pitchspeed);
      return;
   }

   sprite->pitchspeed = atoi(newspeed);
   sprite->CancelEventsOfType(EV_PlacedSprite_Rotate);
   if(sprite->pitchspeed || sprite->yawspeed || sprite->rollspeed)
      sprite->PostEvent(EV_PlacedSprite_Rotate, 0.1);
   gi.dprintf("sprite pitch speed = %i\n", sprite->pitchspeed);
}

void SpriteMaster::SetYawSpeed(Event *ev)
{
   PlacedSprite *sprite;
   const char *newspeed;

   if(!curr_sprite)
   {
      gi.dprintf("no sprite to set\n");
      return;
   }

   sprite = static_cast<PlacedSprite *>(G_GetEntity(curr_sprite));

   if(ev->NumArgs())
      newspeed = ev->GetString(1);
   else
      newspeed = nullptr;

   // display current sprite if none specified
   if(!newspeed || !newspeed[0])
   {
      gi.dprintf("sprite yaw speed = %i\n", sprite->yawspeed);
      return;
   }

   sprite->yawspeed = atoi(newspeed);
   sprite->CancelEventsOfType(EV_PlacedSprite_Rotate);
   if(sprite->pitchspeed || sprite->yawspeed || sprite->rollspeed)
      sprite->PostEvent(EV_PlacedSprite_Rotate, 0.1);
   gi.dprintf("sprite yaw speed = %i\n", sprite->yawspeed);
}

void SpriteMaster::SetRollSpeed(Event *ev)
{
   PlacedSprite *sprite;
   const char *newspeed;

   if(!curr_sprite)
   {
      gi.dprintf("no sprite to set\n");
      return;
   }

   sprite = static_cast<PlacedSprite *>(G_GetEntity(curr_sprite));

   if(ev->NumArgs())
      newspeed = ev->GetString(1);
   else
      newspeed = nullptr;

   // display current sprite if none specified
   if(!newspeed || !newspeed[0])
   {
      gi.dprintf("sprite pitch speed = %i\n", sprite->pitchspeed);
      return;
   }

   sprite->rollspeed = atoi(newspeed);
   sprite->CancelEventsOfType(EV_PlacedSprite_Rotate);
   if(sprite->pitchspeed || sprite->yawspeed || sprite->rollspeed)
      sprite->PostEvent(EV_PlacedSprite_Rotate, 0.1);
   gi.dprintf("sprite roll speed = %i\n", sprite->rollspeed);
}

void SpriteMaster::SetRotDelay(Event *ev)
{
   PlacedSprite *sprite;
   const char *newspeed;

   if(!curr_sprite)
   {
      gi.dprintf("no sprite to set\n");
      return;
   }

   sprite = static_cast<PlacedSprite *>(G_GetEntity(curr_sprite));

   if(ev->NumArgs())
      newspeed = ev->GetString(1);
   else
      newspeed = nullptr;

   // display current sprite if none specified
   if(!newspeed || !newspeed[0])
   {
      gi.dprintf("sprite rotate delay = %f\n", sprite->rotatedelay);
      return;
   }

   sprite->rotatedelay = (float)atof(newspeed);
   gi.dprintf("sprite rotate delay = %f\n", sprite->rotatedelay);
}

void SpriteMaster::SetBlinkSpeed(Event *ev)
{
   PlacedSprite *sprite;
   const char *newspeed;

   if(!curr_sprite)
   {
      gi.dprintf("no sprite to set\n");
      return;
   }

   sprite = static_cast<PlacedSprite *>(G_GetEntity(curr_sprite));

   if(ev->NumArgs())
      newspeed = ev->GetString(1);
   else
      newspeed = nullptr;

   // display current sprite if none specified
   if(!newspeed || !newspeed[0])
   {
      gi.dprintf("sprite blinkspeed = %f\n", sprite->blinkspeed);
      return;
   }

   sprite->blinkspeed = (float)atof(newspeed);
   sprite->CancelEventsOfType(EV_PlacedSprite_Blink);
   if(sprite->blinkspeed <= 0)
      sprite->showModel();
   else
      sprite->PostEvent(EV_PlacedSprite_Blink, sprite->blinkspeed);
   gi.dprintf("sprite blinkspeed = %f\n", sprite->blinkspeed);
}

void SpriteMaster::SetBlinkDelay(Event *ev)
{
   PlacedSprite *sprite;
   const char *newspeed;

   if(!curr_sprite)
   {
      gi.dprintf("no sprite to set\n");
      return;
   }

   sprite = static_cast<PlacedSprite *>(G_GetEntity(curr_sprite));

   if(ev->NumArgs())
      newspeed = ev->GetString(1);
   else
      newspeed = nullptr;

   // display current sprite if none specified
   if(!newspeed || !newspeed[0])
   {
      gi.dprintf("sprite blinkdelay = %f\n", sprite->blinkdelay);
      return;
   }

   sprite->blinkdelay = (float)atof(newspeed);
   gi.dprintf("sprite blinkspeed = %f\n", sprite->blinkdelay);
}

/*===================================================================

Sprite Class

===================================================================*/

PlacedSprite::PlacedSprite()
{
   setMoveType(MOVETYPE_NONE);
   setSolidType(SOLID_NOT);
   setSize(vec_zero, vec_zero);

   //setup for when glowing is turned on
   edict->s.color_r	= 1;
   edict->s.color_g	= 1;
   edict->s.color_b	= 1;
   edict->s.radius		= 175;

   // make sure a new sprite doesn't have any effects on it
   triggereffect = SEFFECT_NONE;
   animatespeed = 0;
   animatedelay = 0;
   pitchspeed = 0;
   yawspeed = 0;
   rollspeed = 0;
   rotatedelay = 0;
   blinkspeed = 0;
   blinkdelay = 0;
   placedframe = 0;
}

EXPORT_FROM_DLL void PlacedSprite::Setup(Vector pos, Vector facing)
{
   Vector norm;

   setModel(Spritecontrol.spritename);
   numframes = Spritecontrol.NumFrames(Spritecontrol.spritename);
   placedframe = Spritecontrol.place_frame;
   if(placedframe >= numframes)
      placedframe = 0;
   edict->s.frame = placedframe;

   edict->s.scale = Spritecontrol.place_scale;
   edict->s.alpha = Spritecontrol.place_alpha;
   if(edict->s.alpha < 1) // it's transparent
      edict->s.renderfx |= RF_TRANSLUCENT;

   norm = facing;
   norm.x = -norm.x;
   norm.y = -norm.y;
   angles = norm.toAngles();
   setAngles(angles);
   placedangles = angles;

   setOrigin(pos);
   worldorigin.copyTo(edict->s.old_origin);
}

EXPORT_FROM_DLL void PlacedSprite::Animate(Event *ev)
{
   edict->s.frame++;

   if(edict->s.frame == numframes)
   {
      // check for it being animate once
      if(triggereffect == SEFFECT_ONCE)
      {
         edict->s.frame = numframes - 1;
         return;
      }

      edict->s.frame = 0;
   }

   PostEvent(EV_PlacedSprite_Animate, animatespeed);
}

EXPORT_FROM_DLL void PlacedSprite::Rotate(Event *ev)
{
   angles[PITCH] += pitchspeed;
   angles[YAW] += yawspeed;
   angles[ROLL] += rollspeed;
   setAngles(angles);
   PostEvent(EV_PlacedSprite_Rotate, 0.1);
}

EXPORT_FROM_DLL void PlacedSprite::Blink(Event *ev)
{
   if(hidden())
      showModel();
   else
      hideModel();

   PostEvent(EV_PlacedSprite_Blink, blinkspeed);
}

EXPORT_FROM_DLL void PlacedSprite::TriggerEvent(Event *ev)
{
   if(triggereffect == SEFFECT_TOGGLE)
   {
      if(animatespeed > 0)
      {
         // turn animation off
         animatespeed *= -1;
         CancelEventsOfType(EV_PlacedSprite_Animate);
      }
      else
      {
         // turn animation on
         if(!animatespeed)
            animatespeed = 0.1;
         else
            animatespeed *= -1;

         PostEvent(EV_PlacedSprite_Animate, animatespeed);
      }
   }
   else if(triggereffect == SEFFECT_ADVANCE)
   {
      edict->s.frame++;
      if(edict->s.frame == numframes)
         edict->s.frame = 0;
   }
   else if(triggereffect == SEFFECT_ONCE)
   {
      if(!animatespeed)
         animatespeed = 0.1;
      else if(animatespeed < 0)
         animatespeed *= -1;

      // make sure we start on our first frame
      edict->s.frame = placedframe;
      link();

      PostEvent(EV_PlacedSprite_Animate, animatespeed);
   }
}

/*===================================================================

Sprite Gun

===================================================================*/

CLASS_DECLARATION(Weapon, SpriteGun, "weapon_spritegun");

ResponseDef SpriteGun::Responses[] =
{
   { &EV_Weapon_Shoot, (Response)&SpriteGun::Shoot },
   { nullptr, nullptr }
};

SpriteGun::SpriteGun()
{
   SetModels(nullptr, "view_spritegun.def");
   SetAmmo(nullptr, 0, 0);

   notdroppable = true;
}

void SpriteGun::Shoot(Event *ev)
{
   PlacedSprite *sprite, *tmpspr;
   Vector pos;
   Vector end;
   trace_t trace;
   Vector targ, dir, norm;
   int mask;

   assert(owner);
   if(!owner)
      return;

   // set firing position and direction
   pos = owner->origin;
   pos[2] += owner->viewheight;
   end = pos + Vector(owner->orientation[0])*2048;

   // set target location
   mask = MASK_SOLID;
   if(Spritecontrol.hitwater)
      mask |= MASK_WATER;
   trace = G_Trace(pos, Vector(0, 0, 0), Vector(0, 0, 0), end, owner, mask, "SpriteGun::Shoot");
   targ = Vector(trace.endpos);

   sprite = new PlacedSprite();
   targ = Vector(trace.endpos) + (Vector(trace.plane.normal)*0.5);
   sprite->Setup(targ, trace.plane.normal);

   // make sure glowing is off for current sprite
   if(Spritecontrol.glowcurrent)
   {
      if(Spritecontrol.curr_sprite)
      {
         tmpspr = static_cast<PlacedSprite *>(G_GetEntity(Spritecontrol.curr_sprite));
         tmpspr->edict->s.renderfx &= ~RF_DLIGHT;
      }
      sprite->edict->s.renderfx |= RF_DLIGHT;
   }

   // set current sprite
   Spritecontrol.curr_sprite = sprite->entnum;
   // incriment the sprite counter
   Spritecontrol.num_sprites++;

   MuzzleFlash(1.0, 0.1, 0, 250, 0.6, 0.2);
   
   NextAttack(0.5);

}

// EOF
