//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/powerups.h                       $
// $Revision:: 10                                                             $
//   $Author::                                                                $
//     $Date::                                                                $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Powerups
// 

#ifndef __POWERUPS_H__
#define __POWERUPS_H__

#include "g_local.h"
#include "inventoryitem.h"

class EXPORT_FROM_DLL ScubaGear : public InventoryItem
{
public:
   CLASS_PROTOTYPE(ScubaGear);
   ScubaGear();
};

class EXPORT_FROM_DLL Adrenaline : public InventoryItem
{
private:
   float             health_delta;
public:
   CLASS_PROTOTYPE(Adrenaline);

   Adrenaline();
   virtual void      Use(Event *ev);
   void              Powerdown(Event *ev);
   virtual void      Archive(Archiver &arc);
   virtual void      Unarchive(Archiver &arc);
};

inline EXPORT_FROM_DLL void Adrenaline::Archive(Archiver &arc)
{
   InventoryItem::Archive(arc);

   arc.WriteFloat(health_delta);
}

inline EXPORT_FROM_DLL void Adrenaline::Unarchive(Archiver &arc)
{
   InventoryItem::Unarchive(arc);

   arc.ReadFloat(&health_delta);
}

class EXPORT_FROM_DLL Cloak : public InventoryItem
{
public:
   CLASS_PROTOTYPE(Cloak);

   Cloak();
   virtual void      Use(Event *ev);
   void              Powerdown(Event *ev);
};

class EXPORT_FROM_DLL Medkit : public Entity
{
public:
   CLASS_PROTOTYPE(Medkit);

   Medkit();
   virtual void      Use(Event *ev);
};

class EXPORT_FROM_DLL Mutagen : public InventoryItem
{
public:
   CLASS_PROTOTYPE(Mutagen);

   Mutagen();
   virtual void      Use(Event *ev);
   void              Powerdown(Event *ev);
   virtual qboolean  Pickupable( Entity *other ); //###
};

class EXPORT_FROM_DLL Oxygen : public InventoryItem
{
public:
   CLASS_PROTOTYPE(Oxygen);

   Oxygen();
   virtual void      Use(Event *ev);
   virtual void      Pickup(Event *ev);
   void              Powerdown(Event *ev);
};

//###
class EXPORT_FROM_DLL Goggles : public InventoryItem
{
private:
   EntityPtr glowent;

public:
   CLASS_PROTOTYPE(Goggles);

   qboolean goggleson;

   Goggles();
   ~Goggles();
   virtual void      Postthink(void);
   virtual void      Use(Event *ev);
   virtual void      CheckTurnoff(Event *ev);
   virtual qboolean  Pickupable(Entity *other);
   virtual Item     *ItemPickup(Entity *other);
   virtual void      Pickup(Event *ev);

   virtual void      Archive(Archiver &arc);
   virtual void      Unarchive(Archiver &arc);
};

inline EXPORT_FROM_DLL void Goggles::Archive(Archiver &arc)
{
   InventoryItem::Archive(arc);

   arc.WriteBoolean(goggleson);
   arc.WriteSafePointer(glowent);
}

inline EXPORT_FROM_DLL void Goggles::Unarchive(Archiver &arc)
{
   InventoryItem::Unarchive(arc);

   goggleson = arc.ReadBoolean();
   arc.ReadSafePointer(&glowent);
}

class EXPORT_FROM_DLL EasterCandy : public InventoryItem
{
public:
   CLASS_PROTOTYPE( EasterCandy );

   EasterCandy();
   virtual void      Use( Event *ev );
};

class EXPORT_FROM_DLL EasterCoke : public InventoryItem
{
public:
   CLASS_PROTOTYPE( EasterCoke );

   EasterCoke();
   virtual void      Use( Event *ev );
};
//###

#endif /* powerups.h */

// EOF
