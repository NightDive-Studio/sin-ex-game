//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/areaportal.h                     $
// $Revision:: 5                                                              $
//   $Author:: Markd                                                          $
//     $Date:: 9/29/98 5:58p                                                  $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// 

#ifndef __AREAPORTAL_H__
#define __AREAPORTAL_H__

#include "g_local.h"
#include "entity.h"

extern Event EV_AreaPortal_Open;
extern Event EV_AreaPortal_Close;

void SetAreaPortals( const char *name, qboolean open );

class EXPORT_FROM_DLL AreaPortal : public Entity
{
private:
   int portalstate;
   int portalnum;

public:
   CLASS_PROTOTYPE(AreaPortal);

   AreaPortal();
   void     SetPortalState(qboolean state);
   qboolean PortalOpen(void);
   void     Open(Event *ev);
   void     Close(Event *ev);
   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void AreaPortal::Archive(Archiver &arc)
{
   Entity::Archive(arc);

   arc.WriteInteger(portalstate);
   arc.WriteInteger(portalnum);
}

inline EXPORT_FROM_DLL void AreaPortal::Unarchive(Archiver &arc)
{
   Entity::Unarchive(arc);

   arc.ReadInteger(&portalstate);
   arc.ReadInteger(&portalnum);
}

#endif

// EOF

