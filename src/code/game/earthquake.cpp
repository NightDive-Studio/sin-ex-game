//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/earthquake.cpp                   $
// $Revision:: 13                                                             $
//   $Author:: Jimdose                                                        $
//     $Date:: 11/08/98 10:47p                                                $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Earthquake trigger causes a localized earthquake when triggered.
// The earthquake effect is visible to the user as the shaking of his screen.
// 

#include "earthquake.h"

/*****************************************************************************/
/*SINED func_earthquake (.5 .5 .8) (-8 -8 -8) (8 8 8)
 Causes an earthquake
"duration" is the duration of the earthquake.  Default is 0.8 seconds.
/*****************************************************************************/

CLASS_DECLARATION(Trigger, Earthquake, "func_earthquake")

Event EV_Earthquake_Deactivate("earthquake_deactivate");

ResponseDef Earthquake::Responses[] =
{
   { &EV_Touch,                     NULL },
   { &EV_Trigger_Effect,            ( Response )&Earthquake::Activate },
   { &EV_Earthquake_Deactivate,     ( Response )&Earthquake::Deactivate },
   { NULL, NULL }
};

Earthquake::Earthquake(void) : Trigger()
{
   const char *name;

   duration = G_GetFloatArg("duration", 0.8f);
   quakeactive = false;

   // cache in the quake sound
   name = gi.GlobalAlias_FindRandom("earthquake");
   gi.soundindex(name);
}

EXPORT_FROM_DLL void Earthquake::Activate(Event *ev)
{
   float newtime;
   Event *event;

   newtime = duration + level.time;
   if(newtime > level.earthquake)
   {
      level.earthquake = newtime;
   }
   quakeactive = true;
   RandomGlobalSound("earthquake", 1, CHAN_VOICE|CHAN_NO_PHS_ADD, ATTN_NONE);
   event = new Event(EV_Earthquake_Deactivate);
   PostEvent(event, duration);
}

EXPORT_FROM_DLL void Earthquake::Deactivate(Event *ev)
{
   quakeactive = false;
   level.earthquake = 0;
   RandomGlobalSound("null_sound", 1, CHAN_VOICE|CHAN_NO_PHS_ADD, ATTN_NORM);
}

// EOF

