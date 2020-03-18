//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/areaportal.cpp                   $
// $Revision:: 8                                                              $
//   $Author:: Jimdose                                                        $
//     $Date:: 10/19/98 6:08p                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// 

#include "g_local.h"
#include "entity.h"
#include "areaportal.h"

Event EV_AreaPortal_Open( "open" );
Event EV_AreaPortal_Close( "close" );

void SetAreaPortals(const char *name, qboolean open)
{
   int		t;
   Entity	*ent;
   float		time;
   Event		event;

   if(!name)
   {
      return;
   }

   // delay turning a portal off so that lerping models are in place when the portal goes off
   if(open)
   {
      time = 0;
      event = EV_AreaPortal_Open;
   }
   else
   {
      time = FRAMETIME;
      event = EV_AreaPortal_Close;
   }

   t = 0;
   while((t = G_FindTarget(t, name)))
   {
      ent = G_GetEntity(t);
      assert(ent);
      if(Q_stricmp(ent->getClassID(), "func_areaportal") == 0)
      {
         // Cancel any waiting portal events
         ent->CancelEventsOfType(EV_AreaPortal_Open);
         ent->CancelEventsOfType(EV_AreaPortal_Close);
         ent->PostEvent(event, time);
      }
   }
}

/*****************************************************************************/
/*SINED func_areaportal (0 0 0) ?

This is a non-visible object that divides the world into
areas that are seperated when this portal is not activated.
Usually enclosed in the middle of a door.

/*****************************************************************************/

CLASS_DECLARATION(Entity, AreaPortal, "func_areaportal");

ResponseDef AreaPortal::Responses[] =
{
   { &EV_AreaPortal_Open,			(Response)&AreaPortal::Open },
   { &EV_AreaPortal_Close,			(Response)&AreaPortal::Close },
   { nullptr, nullptr }
};

void AreaPortal::SetPortalState(qboolean state)
{
   portalstate = state;
   gi.SetAreaPortalState(portalnum, portalstate);
}

qboolean AreaPortal::PortalOpen()
{
   return portalstate;
}

void AreaPortal::Open(Event *ev)
{
   const char *name;

   SetPortalState(true);

   //
   // fire targets
   //
   //### added extended targeting stuff
   for(int i = 0; i < 4; i++)
   {
      switch(i)
      {
      case 0:
         name = Target();
         break;
      case 1:
         name = Target2();
         break;
      case 2:
         name = Target3();
         break;
      case 3:
         name = Target4();
         break;
      }

      if(name && strcmp(name, ""))
      {
         int		num;
         Event		*event;
         Entity   *ent;
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
            event->AddEntity(world);
            ent->ProcessEvent(event);
         }
         while(1);
      }
   }
   //###
}

void AreaPortal::Close(Event *ev)
{
   const char *name;
   SetPortalState(false);

   //
   // fire targets
   //
   name = Target();
   if(name && strcmp(name, ""))
   {
      int		num;
      Event		*event;
      Entity   *ent;
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
         event->AddEntity(world);
         ent->ProcessEvent(event);
      }
      while(1);
   }
}

AreaPortal::AreaPortal() : Entity()
{
   portalnum = G_GetIntArg("style");

   if(!LoadingSavegame)
   {
      // always start closed, except during savegames
      SetPortalState(false);
   }
}

// EOF

