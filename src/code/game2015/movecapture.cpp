/*
================================================================
MOVEMENT CAPTURING ENTITY
================================================================

Copyright (C) 2020 by Night Dive Studios, Inc.
All rights reserved.

See the license.txt file for conditions and terms of use for this code.
*/

#include "movecapture.h"
#include "player.h"

// the list of flags used to put together the output value
#define MCF_OFF       0 // indicates that the movement capturer is not being used
#define MCF_ACTIVE    1 // indicates that the movement capturer is being used
#define MCF_ATTACK    2 // player is pressing the attack key
#define MCF_FORWARD   4 // player is pressing forward
#define MCF_BACK      8 // player is pressing backward
#define MCF_RIGHT    16 // player is pressing right strafe key
#define MCF_LEFT     32 // player is pressing left strafe key
#define MCF_UP       64 // player is pressing jump key
#define MCF_DOWN    128 // player is pressing duck key

CLASS_DECLARATION(TriggerUse, MoveCapture, "trigger_movecapture");

ResponseDef MoveCapture::Responses[] =
{
   { &EV_Trigger_Effect, (Response)&MoveCapture::Activate },
   { &EV_Use,            (Response)&MoveCapture::Use      },
   { nullptr, nullptr }
};

/*****************************************************************************/
/*SINED trigger_movecapture (0 .5 .8) ? NOUSE

"outputvar" is the name of the script variable to output the controls info to (required)

NOUSE means that a user can't use it directly to activate it
/*****************************************************************************/

MoveCapture::MoveCapture() : TriggerUse()
{
   setMoveType(MOVETYPE_NONE);
   setSolidType(SOLID_TRIGGER);
   hideModel();

   if(!LoadingSavegame)
   {
      // make sure that there's a variable name specified
      outputvar = G_GetSpawnArg("outputvar", "");
      if(!outputvar.length())
         error("MoveCapture", "no outputvar specified\n");

      if(!Target())
         error("MoveCapture", "no camera specified\n");

      // init script variable
      SetMCVariable(MCF_OFF);
   }

   // so the function will set the variable
   lastvalue = MCF_ACTIVE;

   usedebounce = 0;
}

void MoveCapture::Use(Event *ev)
{
   // Don't respond to users using me
   if(spawnflags & 1)
      return;

   // can't use if already being used
   if(usingplayer)
      return;

   TriggerStuff(ev);
}

void MoveCapture::Activate(Event *ev)
{
   Entity *other;
   Event  *ev2;
   Camera *cam;
   int     num;
   Player *player;

   // can't activate if already being used
   if(usingplayer)
      return;

   other = ev->GetEntity(1);

   // only clients can use this
   if(!other->isClient())
      return;

   player = static_cast<Player *>(other);

   // check for the player already being in a movement capturer
   if(player == usingplayer)
      return;

   if(player->movecapturer)
      player->movecapturer->Deactivate();

   num = G_FindTarget(0, Target());
   if(!num)
      return;

   cam = static_cast<Camera *>(G_GetEntity(num));
   assert(cam);
   player->SetCamera(cam);
   ev2 = new Event(EV_Player_HideStats);
   player->ProcessEvent(ev2);
   player->movecapturer = this;

   usingplayer = player;
   usedebounce = level.time + 1;
}

void MoveCapture::Deactivate(void)
{
   if(!usingplayer)
      return;

   auto player = static_cast<Player *>(usingplayer.ptr);

   auto ev2 = new Event(EV_Player_DrawStats);
   player->ProcessEvent(ev2);

   player->SetCamera(nullptr);

   player->movecapturer = nullptr;

   usingplayer = nullptr;
   SetMCVariable(MCF_OFF);
}

void MoveCapture::CaptureMovement(usercmd_t *ucmd)
{
   auto player = static_cast<Player *>(usingplayer.ptr);

   // check for the player wanting out
   if((player->new_buttons & BUTTON_USE) &&
      (usedebounce < level.time))
   {
      Deactivate();
      player->new_buttons &= ~BUTTON_USE;
      return;
   }

   // construct the new output value
   int i = MCF_ACTIVE;

   if(player->buttons & BUTTON_ATTACK)
      i |= MCF_ATTACK;

   if(ucmd->forwardmove > 0)
      i |= MCF_FORWARD;
   else if(ucmd->forwardmove < 0)
      i |= MCF_BACK;

   if(ucmd->sidemove > 0)
      i |= MCF_RIGHT;
   else if(ucmd->sidemove < 0)
      i |= MCF_LEFT;

   if(ucmd->upmove > 0)
      i |= MCF_UP;
   else if(ucmd->upmove < 0)
      i |= MCF_DOWN;


   // output the new value
   SetMCVariable(i);

   // clear out the ucmd so that the player doesn't move around
   memset(ucmd, 0, sizeof(ucmd));
   player->new_buttons = 0;
   player->buttons = 0;
}

void MoveCapture::SetMCVariable(int newvalue)
{
   char valuestring[10];

   // don't set the variable if the new
   // value is the same as the old value
   if(newvalue == lastvalue)
      return;

   snprintf(valuestring, sizeof(valuestring), "%i", newvalue);
   Director.ConsoleVariable(outputvar.c_str(), valuestring);

   lastvalue = newvalue;
}

// EOF
