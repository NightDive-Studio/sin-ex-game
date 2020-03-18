//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/lensflare.cpp                    $
// $Revision:: 17                                                             $
//   $Author:: Jimdose                                                        $
//     $Date:: 10/19/98 12:07a                                                $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:LensFlare effect
// 

#include "g_local.h"
#include "entity.h"
#include "lensflare.h"
#include "light.h"

CLASS_DECLARATION(Trigger, LensFlare, "lensflare");

/*****************************************************************************/
/*SINED lensflare (0 1 0) (-8 -8 -8) (8 8 8)
   
"color"      (r g b) 3 values between 0 and 1.0 (Default is 1.0 1.0 1.0)
"light"      If set,make the flare emit a dynamic light. (Default is not set)
"radius"     Radius of the dynamic light (Default is 200) 
"scale"      Factor to scale the lensflare (Default is 0.5)
"sprite"     Sprite model to use for the flare (Default is sprites/glow.spr")
"lightstyle" Lightstyle of the lensflare (Default is none)
/*****************************************************************************/

Event EV_LensFlare_Activate("activate");
Event EV_LensFlare_Deactivate("deactivate");
Event EV_LensFlare_Lightstyle("lightstyle");
Event EV_LensFlare_SetLightstyle("setlightstyle");

ResponseDef LensFlare::Responses[] =
{
   { &EV_LensFlare_Activate,        (Response)&LensFlare::Activate },
   { &EV_LensFlare_Deactivate,      (Response)&LensFlare::Deactivate },
   { &EV_LensFlare_Lightstyle,      (Response)&LensFlare::Lightstyle },
   { &EV_LensFlare_SetLightstyle,   (Response)&LensFlare::SetLightstyle },
   { NULL, NULL }
};

LensFlare::LensFlare() : Entity()
{
   Vector   color;
   int      dlight, radius;
   float    scale;
   int      lightstyle;

   color  = G_GetVectorArg("color", Vector(1, 1, 1));
   dlight = G_GetIntArg("light", 0);
   radius = G_GetIntArg("radius", 0);
   scale  = G_GetFloatArg("scale", 0.5f);
   model  = G_GetSpawnArg("sprite", "sprites/glow.spr");
   lightstyle = G_GetIntArg("lightstyle", 255);

   setModel(model);
   setMoveType(MOVETYPE_NONE);
   setSolidType(SOLID_NOT);

   edict->s.renderfx |= (RF_LENSFLARE);

   if(dlight)
      edict->s.renderfx |= (RF_DLIGHT);

   edict->s.angles[ROLL] = rand() % 360;
   edict->s.alpha = 1.0f;
   edict->s.color_r = color.x;
   edict->s.color_g = color.y;
   edict->s.color_b = color.z;
   edict->s.radius = radius;
   edict->s.scale = scale;
   edict->s.skinnum = lightstyle;

   PostEvent(EV_LensFlare_SetLightstyle, 0);
}

void LensFlare::SetLightstyle(Event *ev)
{
   int         num;
   const char  *name;
   Entity      *ent;

   name = Target();
   if(name && strcmp(name, ""))
   {
      num = 0;
      do
      {
         num = G_FindTarget(num, name);
         if(!num)
         {
            break;
         }

         ent = G_GetEntity(num);
         assert(ent);
         if(ent->isSubclassOf<Light>())
         {
            Light *light;

            light = (Light *)ent;
            edict->s.skinnum = light->GetStyle();
         }
      }
      while(1);
   }
}

void LensFlare::Activate(Event *ev)
{
   //setSolidType( SOLID_BSP ); ### what the hell is this???
   showModel();
}

void LensFlare::Deactivate(Event *ev)
{
   hideModel();
}

void LensFlare::Lightstyle(Event *ev)
{
   edict->s.skinnum = ev->GetInteger(1);
}

// EOF

