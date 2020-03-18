//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/box.cpp                          $
// $Revision:: 36                                                             $
//   $Author:: Markd                                                          $
//     $Date:: 11/15/98 7:51p                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Explodable box that falls when boxes below it are destroyed.
// 

#include "g_local.h"
#include "entity.h"
#include "box.h"
#include "ammo.h"
#include "health.h"
#include "specialfx.h"

Event EV_Box_Think("think");
//###
Event EV_Box_StartThread("triggerthread");
Event EV_Box_SetThread("setthread");
//###

ResponseDef Box::Responses[] =
{
   { &EV_Box_Think,       (Response)&Box::Falling        },
   { &EV_Killed,          (Response)&Box::Killed         },
   //###
   { &EV_Box_StartThread, (Response)&Box::StartThread    },
   { &EV_Box_SetThread,   (Response)&Box::EventSetThread },
   //###
   { NULL, NULL }
};

//### added thread to be activated when killed
/*****************************************************************************/
/*SINED func_box (0 .5 .8) ?

  Explodable box that falls when boxes below it are destroyed.  
"items" - List of classnames to spawn when the box is destroyed.  Separate
each classname by spaces (E.g. Adrenaline RocketLauncher).  Default is NULL.
"angles" - Orientation of the item that is spawned.
"health" - Health of the box ( default is 60 )
"thread" name of thread to trigger when killed. This can be in a different
script file as well by using the '::' notation.
/*****************************************************************************/

CLASS_DECLARATION( Entity, Box, "func_box" );

EXPORT_FROM_DLL void Box::StartFalling(void)
{
   movetime = 0;
   velocity += { 0, 0, 100 };
   setMoveType(MOVETYPE_TOSS);
   setSolidType(SOLID_BBOX);
   setOrigin(worldorigin + Vector(0, 0, 1));
   PostEvent(EV_Box_Think, FRAMETIME);
}

EXPORT_FROM_DLL void Box::Falling(Event *ev)
{
   if(velocity != vec_zero)
   {
      movetime = level.time + 1;
   }

   if(movetime < level.time)
   {
      setMoveType(MOVETYPE_PUSH);
      setSolidType(SOLID_BSP);
   }
   else
   {
      PostEvent(EV_Box_Think, FRAMETIME);
   }
}

void Box::TellNeighborsToFall(void)
{
   Entity	*ent;
   //Event		*e;
   Vector	min;
   Vector	max;
   Entity	*next;

   min = absmin + Vector(6, 6, 6);
   max = absmax + Vector(-6, -6, 0);

   for(ent = G_NextEntity(world); ent != NULL; ent = next)
   {
      next = G_NextEntity(ent);

      if((ent != this) && ent->isSubclassOf<Box>())
      {
         if(!((ent->absmax[0] < min[0]) ||
            (ent->absmax[1] < min[1]) ||
            (ent->absmax[2] < min[2]) ||
            (ent->absmin[0] > max[0]) ||
            (ent->absmin[1] > max[1]) ||
            (ent->absmin[2] > max[2])))

         {
            if(ent->takedamage != DAMAGE_NO)
               static_cast<Box *>(ent)->StartFalling();
            // Ok, it's a hack.
            //if ( ent->takedamage != DAMAGE_NO )
            //	{
            //	e = new Event( ev );
            //	ent->ProcessEvent( e );
            //	}
         }
      }
   }
}

void Box::Killed(Event *ev)
{
   Entity      *attacker;
   Vector      dir;
   Vector      org;
   Entity      *ent;
   const char  *s;
   const char  *token;
   int         width = 0;
   int         depth = 0;
   int         boxwidth;
   char        temp[128];
   const char	*name;
   int         num;
   Event       *event;
   qboolean    spawned;
   static float last_dialog_time = 0;

   hideModel();
   RandomGlobalSound("impact_crateexplo", 1, CHAN_BODY, ATTN_NORM);

   takedamage = DAMAGE_NO;

   TellNeighborsToFall();

   ProcessEvent(EV_BreakingSound);

   attacker = ev->GetEntity(1);
   dir = worldorigin - attacker->worldorigin;
   TesselateModel
   (
      this,
      tess_min_size,
      tess_max_size,
      dir,
      ev->GetInteger(2),
      tess_percentage,
      tess_thickness,
      vec3_origin
   );

   //
   // fire off targets
   //
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

         event = new Event(EV_Activate);
         event->AddEntity(attacker);
         ent->PostEvent(event, 0);
      }
      while(1);
   }

   // items holds the list of def files to spawn
   s = items.c_str();

   G_InitSpawnArguments();

   if(setangles)
   {
      snprintf(temp, sizeof(temp), "%f %f %f", angles[0], angles[1], angles[2]);
      G_SetSpawnArg("angles", temp);
   }

   spawned  = false;
   boxwidth = static_cast<int>(maxs[0]);
   while(1)
   {
      token = COM_Parse(&s);

      if(!token[0])
         break;

      G_SetSpawnArg("model", token);

      if((width * 32) > boxwidth)
      {
         width = 0;
         depth++;
      }

      // Calculate and set the origin
      org = worldorigin + Vector(0, 0, 32) + Vector(32, 0, 0) * width + Vector(0, 32, 0) * depth;
      width++;
      snprintf(temp, sizeof(temp), "%f %f %f", org[0], org[1], org[2]);
      G_SetSpawnArg("origin", temp);

      // Create the item
      ent = (Entity *)G_CallSpawn();
      spawned = true;

      // Postpone the Drop because the box is still there.
      ent->PostponeEvent(EV_Item_DropToFloor, 0.1f);
   }
   G_InitSpawnArguments();

   if(spawned &&
      attacker->isClient() &&
      (last_dialog_time < level.time) &&
      (!(attacker->flags & FL_SP_MUTANT)) &&
      (!deathmatch->value)
     )
   {
      char name[128];
      int num;

      last_dialog_time = level.time + 25;
      if(level.no_jc)
      {
         num = (int)G_Random(3) + 1;
      }
      else
      {
         num = (int)G_Random(5) + 1;
      }
      snprintf(name, sizeof(name), "global/universal_script.scr::blade_finds_item%d", num);
      ExecuteThread(name, true);
   }

   //###
   ProcessEvent(EV_Box_StartThread);
   //###

   PostEvent(EV_Remove, 0);
}

Box::Box() : Entity()
{
   const char  *text;
   const char  *s;
   char        token[MAX_TOKEN_CHARS];

   showModel();
   setMoveType(MOVETYPE_PUSH);
   setSolidType(SOLID_BSP);
   setOrigin(origin);

   health         = G_GetIntArg("health", 60);
   max_health     = health;
   takedamage     = DAMAGE_YES;
   tess_thickness = 20;
   text           = G_GetSpawnArg("items");
   mass           = 300; //### so stacked ones don't fly around when hit

   if(text)
   {
      items = text;
      s = items.c_str();
      while(1)
      {
         strcpy(token, COM_Parse(&s));
         if(!token[0])
            break;
         modelIndex(token);
      }
   }

   setangles = (G_GetSpawnArg("angle") || G_GetSpawnArg("angles"));
   if(setangles)
   {
      float angle;
      angle = G_GetFloatArg("angle", 0);
      angles = G_GetVectorArg("angles", Vector(0, angle, 0));
   }
}

//###
void Box::StartThread(Event *ev)
{
   if(thread.length())
   {
      if(!ExecuteThread(thread))
      {
         warning("StartThread", "Null game script");
      }
   }
}

void Box::EventSetThread(Event *ev)
{
   thread = ev->GetString(1);
}
//###

// EOF

