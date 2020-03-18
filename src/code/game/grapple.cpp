//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/grapple.cpp                      $
// $Revision:: 6                                                              $
//   $Author:: Aldie                                                          $
//     $Date:: 3/25/99 6:29p                                                  $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Grappling Hook

#include "g_local.h"
#include "grapple.h"
#include "player.h"
#include "ctf.h"

CLASS_DECLARATION(Projectile, Hook, NULL);

Event EV_Grapple_PullPlayer("pullplayer");

ResponseDef Hook::Responses[] =
{
   { &EV_Touch,					(Response)&Hook::Touch },
   { NULL, NULL }
};

EXPORT_FROM_DLL void Hook::Touch(Event *ev)
{
   int	 damg;
   Entity *other;
   Entity *owner;
   Player *player;

   other = ev->GetEntity(1);
   assert(other);

   if(other->entnum == this->owner)
   {
      return;
   }

   stopsound(CHAN_VOICE);
   setSolidType(SOLID_NOT);
   setMoveType(MOVETYPE_NONE);

   if(HitSky() || other->isSubclassOf<Teleporter>())
   {
      PostEvent(EV_Remove, 0);
      return;
   }

   // HACK for Richard's map
   if((level.impact_trace.surface) &&
      (level.impact_trace.surface->flags & SURF_DAMAGE))
   {
      PostEvent(EV_Remove, 0);
      return;
   }

   damg = 10;

   owner = G_GetEntity(this->owner);

   if(!owner)
      owner = world;

   if(other->takedamage)
   {
      player = (Player *)(Entity *)owner;
      player->ClearGrapplePull();
      other->Damage(this, owner, damg, worldorigin, velocity, level.impact_trace.plane.normal, 200, 0, MOD_GRAPPLE, -1, -1, 1.0f);
      PostEvent(EV_Remove, 0.1);
      return;
   }

   // Hit a solid wall
   if(owner->isClient())
   {
      setMoveType(MOVETYPE_NONE);
      player = (Player *)(Entity *)owner;
      player->SetGrapplePull(worldorigin, CTF_GRAPPLE_PULL_SPEED);

      RandomAnimate("hooked", NULL);
      // Don't remove the hook, once it's implanted in a wall
      CancelEventsOfType(EV_Remove);
   }
   else
   {
      PostEvent(EV_Remove, 0.1);
   }
}

EXPORT_FROM_DLL void Hook::Setup(Entity *owner, Vector pos, Vector dir)
{
   Event *ev;

   this->owner = owner->entnum;
   edict->owner = owner->edict;

   setMoveType(MOVETYPE_FLYMISSILE);
   setSolidType(SOLID_BBOX);
   edict->clipmask = MASK_PROJECTILE;
   SetGravityAxis(owner->gravaxis);

   angles = dir.toAngles();
   angles[PITCH] = -angles[PITCH];
   setAngles(angles);

   velocity = dir * CTF_GRAPPLE_SPEED;

   // set duration 
   ev = new Event(EV_Remove);
   ev->AddEntity(world);
   PostEvent(ev, 10);

   takedamage = DAMAGE_NO;

   setModel("ctf_grap.def");
   gravity = 0;

   setSize({ -1, -1, -1 }, { 1, 1, 1 });
   setOrigin(pos);
   worldorigin.copyTo(edict->s.old_origin);
}

#if 1 // ksh -- Turned back on to fix linker errors
CLASS_DECLARATION(Weapon, Grapple, "weapon_grapple");

Event EV_Grapple_UpdateBeam("grapple_updatebeam");

ResponseDef Grapple::Responses[] =
{
   { &EV_Grapple_UpdateBeam,	(Response)&Grapple::UpdateBeam },
   { &EV_Weapon_Shoot,			(Response)&Grapple::Shoot },
   { NULL, NULL }
};

Grapple::Grapple() : Weapon()
{
   SetModels("grapple.def", "view_grapple.def");
   SetRank(0, 0);
   SetType(WEAPON_2HANDED_LO);
}

Grapple::~Grapple()
{
   // Kill the beam
   CancelEventsOfType(EV_Grapple_UpdateBeam);

   if(beam)
      beam->PostEvent(EV_Remove, 0);
}

qboolean Grapple::HasAmmo(void)
{
   return true;
}

void Grapple::ReleaseFire(float holdfiretime)
{
   //Client released the fire button, kill the hook
   if(hook)
   {
      hook->ProcessEvent(EV_Remove);
   }

   if(owner->isClient())
   {
      Player *player;

      player = (Player *)(Sentient *)owner;
      player->ClearGrapplePull();
   }

   if(beam)
   {
      beam->PostEvent(EV_Remove, 0);
   }

   NextAttack(0.1);
   CancelEventsOfType(EV_Grapple_UpdateBeam);
   ProcessEvent(EV_Weapon_DoneFiring);
}

void Grapple::UpdateBeam(Event *ev)
{
   Vector pos, dir;

   if(!hook)
   {
      beam->ProcessEvent(EV_Remove);
      return;
   }

   if(beam)
   {
      GetMuzzlePosition(&pos, &dir);
      beam->setBeam(pos, hook->worldorigin, 2, 1, 1, 1, 1, 0);
      PostEvent(EV_Grapple_UpdateBeam, FRAMETIME);
   }
}

void Grapple::Shoot(Event *ev)
{
   Vector	pos;
   Vector	dir;

   assert(owner);

   if(!owner)
   {
      return;
   }

   // Check to see if hook is out, don't fire another one
   if(hook)
   {
      return;
   }

   GetMuzzlePosition(&pos, &dir);

   hook = new Hook();
   hook->Setup(owner, pos, dir);

   beam = new Beam();
   beam->setBeam(pos, hook->worldorigin, 2, 1, 1, 1, 1, 0);
   PostEvent(EV_Grapple_UpdateBeam, 0.1);
}

#endif

// EOF

