//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/player.cpp                       $
// $Revision:: 450                                                            $
//   $Author:: Markd                                                          $
//     $Date:: 11/02/99 12:09p                                                $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Class definition of the player.
// 

#include "g_local.h"
#include "entity.h"
#include "player.h"
#include "worldspawn.h"
#include "weapon.h"
#include "trigger.h"
#include "scriptmaster.h"
#include "vehicle.h"
#include "path.h"
#include "navigate.h"
#include "misc.h"
#include "q_shared.h"
#include "console.h"
#include "earthquake.h"
#include "gravpath.h"
#include "armor.h"
#include "inventoryitem.h"
#include "gibs.h"
#include "spidermine.h"
#include "deadbody.h"
#include "actor.h"
#include "ctf.h"

#include <windows.h>

//temporarily put this in.
//#define OLD_DEATH_MESSAGES 1

const Vector power_color(0.0, 1.0, 0.0);
const Vector acolor(1.0, 1.0, 1.0);
const Vector bcolor(1.0, 0.0, 0.0);

static const char *ammo_types[NUM_AMMO_TYPES] =
{
   "Bullet357", "ShotgunClip", "Bullet10mm", "Bullet50mm", "BulletPulse", "BulletSniper", "Rockets", "SpiderMines"
};

static const char *armor_types[NUM_ARMOR_TYPES] =
{
   "RiotHelmet", "FlakJacket", "FlakPants"
};

Event EV_Player_GodCheat("superfuzz", EV_CHEAT);
Event EV_Player_NoTargetCheat("wallflower", EV_CHEAT);
Event EV_Player_NoClipCheat("nocollision", EV_CHEAT);
Event EV_Player_GiveAllCheat("wuss", EV_CHEAT);
Event EV_Player_EndLevel("endlevel");

Event EV_Player_DevGodCheat("god", EV_CHEAT);
Event EV_Player_DevNoTargetCheat("notarget", EV_CHEAT);
Event EV_Player_DevNoClipCheat("noclip", EV_CHEAT);

Event EV_Player_PrevWeapon("weapprev", EV_CONSOLE);
Event EV_Player_NextWeapon("weapnext", EV_CONSOLE);
Event EV_Player_PrevItem("invprev", EV_CONSOLE);
Event EV_Player_NextItem("invnext", EV_CONSOLE);
Event EV_Player_UseInventoryItem("invuse", EV_CONSOLE);
Event EV_Player_GiveCheat("give", EV_CHEAT);
Event EV_Player_Take("take");
Event EV_Player_UseItem("use", EV_CONSOLE);
Event EV_Player_Spectator("spectator", EV_CONSOLE);
Event EV_Player_GameVersion("gameversion", EV_CONSOLE);
Event EV_Player_Fov("fov");
Event EV_Player_SaveFov("savefov", EV_CONSOLE);
Event EV_Player_RestoreFov("restorefov", EV_CONSOLE);
Event EV_Player_ToggleViewMode("toggleviewmode", EV_CONSOLE);
Event EV_Player_ToggleZoomMode("togglezoommode", EV_CHEAT);
Event EV_Player_ZoomOut("zoomout");
Event EV_Player_Kill("playerkill", EV_CONSOLE);
Event EV_Player_Dead("dead");
Event EV_Player_SpawnEntity("spawn", EV_CHEAT);
Event EV_Player_SpawnActor("actor", EV_CHEAT);
Event EV_Player_ShowInfo("showinfo", EV_CONSOLE);
Event EV_Player_ReadyToFire("readytofire");
Event EV_Player_WaitingToFire("waitingtofire");
Event EV_Player_AttackDone("attackdone");
Event EV_Player_AddPathNode("addnode", EV_CHEAT);
Event EV_Player_Respawn("respawn");
Event EV_Player_ClearFloatingInventory("clear_flinv");
Event EV_Player_TestThread("testthread", EV_CHEAT);
Event EV_Player_PowerupTimer("poweruptimer");
Event EV_Player_UpdatePowerupTimer("updatepoweruptime");
Event EV_Player_DrawOverlay("drawoverlay");
Event EV_Player_HideOverlay("hideoverlay");
Event EV_Player_DrawStats("drawstats");
Event EV_Player_HideStats("hidestats");
Event EV_Player_SetFlashColor("setflashcolor");
Event EV_Player_ClearFlashColor("clearflashcolor");
Event EV_Player_Mutate("mutate", EV_CHEAT);
Event EV_Player_Human("human", EV_CHEAT);
Event EV_Player_Skin("skin");

Event EV_Player_WhatIs("whatis", EV_CHEAT);
Event EV_Player_ActorInfo("actorinfo", EV_CHEAT);
Event EV_Player_Taunt("taunt", EV_CONSOLE);
Event EV_Player_KillEnt("killent", EV_CONSOLE | EV_CHEAT);
Event EV_Player_KillClass("killclass", EV_CONSOLE | EV_CHEAT);
Event EV_Player_RemoveEnt("removeent", EV_CONSOLE | EV_CHEAT);
Event EV_Player_RemoveClass("removeclass", EV_CONSOLE | EV_CHEAT);

// CTF
Event EV_Player_CTF_JoinTeamHardcorps("jointeamhardcorps", EV_CONSOLE);
Event EV_Player_CTF_JoinTeamSintek("jointeamsintek", EV_CONSOLE);
Event EV_Player_CTF_Team("team", EV_CONSOLE);
Event EV_Player_CTF_UpdateHookBeam("updatebeam");
Event EV_Player_CTF_DropTech("droptech", EV_CONSOLE);
Event EV_Player_CTF_SoundEvent("ctfsoundevent");
Event EV_Player_DropWeaponEvent("dropweap", EV_CONSOLE);
Event EV_Player_DropFlag("dropflag", EV_CONSOLE);

#define UPRIGHT_SPEED		320.0f
#define CROUCH_SPEED			110.0f
#define ACCELERATION			10.0f
#define TAUNT_TIME			1.0f

/*
==============================================================================

PLAYER

==============================================================================
*/
// deadflag values

cvar_t * s_debugmusic;
cvar_t * whereami;

CLASS_DECLARATION(Sentient, Player, "player");

ResponseDef Player::Responses[] =
{
   { &EV_ClientMove,					(Response)&Player::ClientThink },
   { &EV_ClientEndFrame,			(Response)&Player::EndFrame },
   { &EV_Player_ShowInfo,		   (Response)&Player::ShowInfo },
   { &EV_Vehicle_Enter,				(Response)&Player::EnterVehicle },
   { &EV_Vehicle_Exit,				(Response)&Player::ExitVehicle },

   { &EV_Player_EndLevel,		   (Response)&Player::EndLevel },

   { &EV_Player_AddPathNode,		(Response)&Player::AddPathNode },
   { &EV_Player_PrevWeapon,		(Response)&Player::EventPreviousWeapon },
   { &EV_Player_NextWeapon,		(Response)&Player::EventNextWeapon },
   { &EV_Player_PrevItem,	   	(Response)&Player::EventPreviousItem },
   { &EV_Player_NextItem,  		(Response)&Player::EventNextItem },
   { &EV_Player_UseItem,			(Response)&Player::EventUseItem },
   { &EV_Player_UseInventoryItem,(Response)&Player::EventUseInventoryItem },
   { &EV_Player_GiveCheat,			(Response)&Player::GiveCheat },
   { &EV_Player_GiveAllCheat,		(Response)&Player::GiveAllCheat },
   { &EV_Player_Take,   			(Response)&Player::Take },
   { &EV_Player_GodCheat,			(Response)&Player::GodCheat },
   { &EV_Player_DevGodCheat,		(Response)&Player::GodCheat },
   { &EV_Player_Spectator,			(Response)&Player::Spectator },
   { &EV_Player_NoTargetCheat,	(Response)&Player::NoTargetCheat },
   { &EV_Player_DevNoTargetCheat,(Response)&Player::NoTargetCheat },
   { &EV_Player_NoClipCheat,		(Response)&Player::NoclipCheat },
   { &EV_Player_DevNoClipCheat,	(Response)&Player::NoclipCheat },
   { &EV_Player_GameVersion,		(Response)&Player::GameVersion },
   { &EV_Player_Fov,					(Response)&Player::Fov },
   { &EV_Player_SaveFov,			(Response)&Player::SaveFov },
   { &EV_Player_RestoreFov,		(Response)&Player::RestoreFov },
   { &EV_Player_ToggleViewMode,	(Response)&Player::ToggleViewMode },
   { &EV_Player_ToggleZoomMode,	(Response)&Player::ToggleZoomMode },
   { &EV_Player_ZoomOut, 	      (Response)&Player::ZoomOut },
   { &EV_EnterConsole,           (Response)&Player::EnterConsole },
   { &EV_ExitConsole,            (Response)&Player::ExitConsole },
   { &EV_KickFromConsole,        (Response)&Player::KickConsole },
   { &EV_Player_Kill,				(Response)&Player::Kill },
   { &EV_Player_Dead,				(Response)&Player::Dead },
   { &EV_Player_SpawnEntity,		(Response)&Player::SpawnEntity },
   { &EV_Player_SpawnActor,      (Response)&Player::SpawnActor },
   { &EV_Player_Respawn,			(Response)&Player::Respawn },

   { &EV_Pain,							(Response)&Player::Pain },
   { &EV_Killed,						(Response)&Player::Killed },
   { &EV_Gib,                    (Response)&Player::GibEvent },
   { &EV_GotKill,						(Response)&Player::GotKill },

   { &EV_Player_ReadyToFire,		(Response)&Player::ReadyToFire },
   { &EV_Player_WaitingToFire,	(Response)&Player::WaitingToFire },
   { &EV_Player_AttackDone,		(Response)&Player::DoneFiring },
   { &EV_Player_TestThread,		(Response)&Player::TestThread },

   { &EV_Player_ClearFloatingInventory,   (Response)&Player::ClearFloatingInventory },
   { &EV_Player_PowerupTimer,		         (Response)&Player::SetPowerupTimer },
   { &EV_Player_UpdatePowerupTimer,		   (Response)&Player::UpdatePowerupTimer },

   { &EV_Player_WhatIs,							(Response)&Player::WhatIs },
   { &EV_Player_ActorInfo,                (Response)&Player::ActorInfo },
   { &EV_Player_Taunt,							(Response)&Player::Taunt },
   { &EV_Player_KillEnt,						(Response)&Player::KillEnt },
   { &EV_Player_RemoveEnt,						(Response)&Player::RemoveEnt },
   { &EV_Player_KillClass,						(Response)&Player::KillClass },
   { &EV_Player_RemoveClass,					(Response)&Player::RemoveClass },

   { &EV_Player_DrawOverlay,					(Response)&Player::DrawOverlay },
   { &EV_Player_HideOverlay,					(Response)&Player::HideOverlay },
   { &EV_Player_DrawStats,					   (Response)&Player::DrawStats },
   { &EV_Player_HideStats,					   (Response)&Player::HideStats },

   { &EV_Player_SetFlashColor,    		   (Response)&Player::SetFlashColor },
   { &EV_Player_ClearFlashColor,    		(Response)&Player::ClearFlashColor },
   { &EV_Player_Mutate,    		         (Response)&Player::Mutate },
   { &EV_Player_Human,               		(Response)&Player::Human },
   { &EV_Player_Skin,               		(Response)&Player::SetSkin },

   { &EV_Player_CTF_JoinTeamHardcorps,		(Response)&Player::CTF_JoinTeamHardcorps },
   { &EV_Player_CTF_JoinTeamSintek,			(Response)&Player::CTF_JoinTeamSintek },
   { &EV_Player_CTF_Team,						(Response)&Player::CTF_Team },
   { &EV_Player_CTF_UpdateHookBeam,       (Response)&Player::CTF_UpdateHookBeam },
   { &EV_Player_CTF_DropTech,			      (Response)&Player::CTF_DropTech },
   { &EV_Player_CTF_SoundEvent,		      (Response)&Player::CTF_SoundEvent },
   { &EV_Player_DropWeaponEvent,	         (Response)&Player::DropWeaponEvent },
   { &EV_Player_DropFlag,	               (Response)&Player::CTF_DropFlag },
   { NULL, NULL }
};

Player::Player() : Sentient()
{
   path = NULL;

   respawn_time = -1;

   watchCamera = NULL;
   thirdpersonCamera = NULL;
   movieCamera = NULL;
   spectatorCamera = NULL;

   damage_blood = 0;
   damage_alpha = 0;

   action_level = 0;
   drawoverlay = false;

   defaultViewMode = FIRST_PERSON;

   fov = atof(Info_ValueForKey(client->pers.userinfo, "fov"));
   if(fov < 1)
   {
      fov = 90;
   }
   else if(fov > 160)
   {
      fov = 160;
   }

   savedfov = 0;
   has_thought = false;
   spectator_distance = 128;

   grapple_pull = false;

   s_debugmusic = gi.cvar("s_debugmusic", "0", 0);
   whereami = gi.cvar("whereami", "0", 0);

   // Remove him from the world until we spawn him
   unlink();
}

void Player::InitSkin(void)
{

   // If this is a new level, there will be no custom skin, so go back to the original blade skin.
   if(!LoadingSavegame && !deathmatch->value)
   {
      int playernum = edict - g_edicts - 1;

      strcpy(client->pers.skin, "blade_base");

      // combine name, skin and model into a configstring
      gi.configstring(CS_PLAYERSKINS + playernum, va("%s\\%s\\%s",
                                                     client->pers.netname,
                                                     client->pers.model,
                                                     client->pers.skin));
   }
}

void Player::Init(void)
{
   InitClient();
   InitPhysics();
   InitPowerups();
   InitWorldEffects();
   InitMusic();
   InitPath();
   InitView();
   InitState();
   InitEdict();
   InitModel();
   InitWeapons();
   InitSkin();

   // don't call RestoreEnt when respawning on a training level
   if(!LoadingServer && (deathmatch->value || (level.training && (respawn_time != -1)) ||
                         !PersistantData.RestoreEnt(this)))
   {
      InitInventory();
      InitHealth();
   }

   ChooseSpawnPoint();

   if(!LoadingSavegame && (viewmode == THIRD_PERSON) && !crosshair)
   {
      cvar_t * chair;

      chair = gi.cvar("crosshair", "0", 0);
      crosshair = new Entity();
      crosshair->setModel(va("sprites/crosshair%d.spr", (int)chair->value));
      crosshair->edict->svflags |= SVF_ONLYPARENT;
      crosshair->edict->owner = this->edict;
   }

   if(!deathmatch->value)
   {
      gi.AddCommandString("con_clearfade\n");
   }

   // make sure we put the player back into the world
   link();
}

void Player::WritePersistantData(SpawnArgGroup &group)
{
   str text;
   char t[3];
   int hi;
   int lo;
   int i;

   // encode the damage states into a text string
   t[2] = 0;
   for(i = 0; i < MAX_MODEL_GROUPS; i++)
   {
      hi = (edict->s.groups[i] >> 4) & 0xf;
      lo = edict->s.groups[i] & 0xf;
      t[0] = (char)('A' + hi);
      t[1] = (char)('A' + lo);
      text += t;
   }

   G_SetSpawnArg("savemodel", savemodel.c_str());
   G_SetSpawnArg("saveskin", saveskin.c_str());
   G_SetSpawnArg("damage_groups", text.c_str());
   G_SetFloatArg("fov", fov);
   G_SetIntArg("defaultViewMode", defaultViewMode);
   G_SetIntArg("flags", flags & (FL_GODMODE | FL_NOTARGET | FL_SP_MUTANT));

   Sentient::WritePersistantData(group);
}

void Player::RestorePersistantData(SpawnArgGroup &group)
{
#ifndef SIN_DEMO
   SpiderMine *spidermine;
   Detonator  *detonator;
#endif
   int i;
   int len;
   str text;
   const char *ptr;
   int hi;
   int lo;

   Sentient::RestorePersistantData(group);

   group.RestoreArgs(1);

   // clear the damage states
   memset(edict->s.groups, 0, sizeof(edict->s.groups));

   savemodel = G_GetStringArg("savemodel");
   saveskin = G_GetStringArg("saveskin");

   text = G_GetStringArg("damage_groups");
   len = text.length() >> 1;
   if(len > MAX_MODEL_GROUPS)
   {
      len = MAX_MODEL_GROUPS;
   }

   // decode the damage states from text string
   ptr = text.c_str();
   for(i = 0; i < len; i++)
   {
      hi = (*ptr++ - 'A') & 0xf;
      lo = (*ptr++ - 'A') & 0xf;
      edict->s.groups[i] = (hi << 4) + lo;
   }

   flags |= G_GetIntArg("flags");
   fov = G_GetFloatArg("fov", 90);
   defaultViewMode = (viewmode_t)G_GetIntArg("defaultViewMode", FIRST_PERSON);

#ifndef SIN_DEMO
   spidermine = (SpiderMine *)FindItem("SpiderMine");
   detonator = (Detonator *)FindItem("Detonator");

   if(spidermine)
   {
      spidermine->SetDetonator(detonator);
   }

   if(FindItem("Silencer"))
   {
      flags |= FL_SILENCER;
   }

   if(FindItem("ScubaGear"))
   {
      flags |= FL_OXYGEN;
   }

   if(flags & FL_SP_MUTANT)
   {
      setModel("manumit_pl.def");
      SetAnim("idle");
   }
#endif

   if(defaultViewMode == CAMERA_VIEW)
   {
      defaultViewMode = FIRST_PERSON;
   }

   SetViewMode(defaultViewMode);

   // prevent the player from starting dead
   if(health < 1)
   {
      health = 1;
   }
}

void Player::InitEdict(void)
{
   // entity state stuff
   setSolidType(SOLID_BBOX);
   setMoveType(MOVETYPE_WALK);
   setSize(Vector(-16, -16, 0), Vector(16, 16, STAND_HEIGHT));
   edict->clipmask = MASK_PLAYERSOLID;
   edict->svflags &= ~SVF_DEADMONSTER;
   edict->svflags &= ~SVF_HIDEOWNER;
   edict->owner = NULL;

   // clear entity state values
   edict->s.effects = 0;
   edict->s.frame = 0;
}

void Player::InitMusic(void)
{
   //
   // reset the music 
   //
   client->ps.current_music_mood = mood_normal;
   client->ps.fallback_music_mood = mood_normal;
   ChangeMusic("normal", "normal", false);
   music_forced = false;
}

void Player::InitClient(void)
{
   client_persistant_t	saved;
   client_respawn_t		resp;
   float                save_fov;

   save_fov = client->ps.fov;

   // deathmatch wipes most client data every spawn
   if(deathmatch->value || level.training)
   {
      char userinfo[MAX_INFO_STRING];

      resp = client->resp;
      memcpy(userinfo, client->pers.userinfo, sizeof(userinfo));
      G_InitClientPersistant(client);
      G_ClientUserinfoChanged(edict, userinfo);
   }
   else
   {
      memset(&resp, 0, sizeof(resp));
   }

   // clear everything but the persistant data and fov
   saved = client->pers;
   memset(client, 0, sizeof(*client));
   client->pers = saved;
   client->resp = resp;
   client->ps.fov = save_fov;
}

void Player::InitState(void)
{
   in_console       = false;
   trappedInQuantum = false;
   floating_owner   = NULL;
   gibbed           = false;
   enemy            = NULL;
   lastEnemyTime    = 0;
   lastTauntTime    = 0;
   grapple_pull     = false;
   grapple_speed    = 0;
   grapple_org      = vec_zero;
   grapple_time     = 0;

   takedamage = DAMAGE_AIM;
   deadflag = DEAD_NO;
   flags &= ~FL_NO_KNOCKBACK;
   flags |= (FL_BLOOD | FL_DIE_GIBS);

   if(parentmode->value)
   {
      flags &= ~FL_BLOOD;
      flags &= ~FL_DIE_GIBS;
   }
}

void Player::InitHealth(void)
{
   // Don't do anything if we're loading a server game.
   // This is either a loadgame or a restart
   if(LoadingServer)
   {
      return;
   }

   // reset the health values
   health = 100;
   max_health = 100;
   last_damage_time = 0;

   // clear the damage states
   memset(edict->s.groups, 0, sizeof(edict->s.groups));
}

void Player::InitModel(void)
{
   str model;

   // Model stuff
   edict->s.renderfx |= RF_CUSTOMSKIN;
   edict->s.skinnum = edict->s.number - 1;

   // Make sure that the model is allowed
   model = client->pers.model;
   if(!game.ValidPlayerModels.ObjectInList(model))
   {
      model = "pl_blade.def";
   }

   setModel(model.c_str());
   TempAnim("idle", NULL);
   showModel();
}

void Player::InitPhysics(void)
{
   // Physics stuff
   onladder       = false;
   sentientFrozen = false;
   oldvelocity    = vec_zero;
   velocity       = vec_zero;
   gravity        = 1.0;
   falling        = false;
   fall_time      = 0;
   fall_value     = 0;
   fallsurface    = NULL;
   mass           = 200;
   xyspeed        = 0;
}

void Player::InitPowerups(void)
{
   // powerups
   poweruptimer = 0;
   poweruptype  = 0;
   flags &= ~(FL_SHIELDS | FL_ADRENALINE | FL_CLOAK | FL_MUTANT | FL_SILENCER | FL_OXYGEN);
   edict->s.renderfx &= ~(RF_DLIGHT | RF_ENVMAPPED);
}

void Player::InitWorldEffects(void)
{
   // world effects
   next_drown_time = 0;
   air_finished    = level.time + 20;
   old_waterlevel  = 0;
   drown_damage    = 0;
}

void Player::InitWeapons(void)
{
   // weapon stuff
   firing       = false;
   firedown     = false;
   firedowntime = 0;
   usedown      = false;
   gunoffset    = Vector(0, 0, STAND_HEIGHT);
}

void Player::InitInventory(void)
{
   // Don't do anything if we're loading a server game.
   // This is either a loadgame or a restart
   if(LoadingServer)
   {
      return;
   }

   // Give player the default weapon
   if(!FindItem("Magnum"))
   {
      giveWeapon("Magnum");
   }

   if(!FindItem("Fists"))
   {
      giveWeapon("Fists");
   }

#ifndef SIN_DEMO
   if(!FindItem("SpiderMine"))
   {
      giveWeapon("SpiderMine");
   }
#endif
}

void Player::InitView(void)
{
   // Camera stuff
   zoom_mode = ZOOMED_OUT;
   spectator = false;
   currentCameraTarget = 0;
   checked_dead_camera = false;
   SetViewMode(defaultViewMode);

   // view stuff
   kick_angles   = vec_zero;
   kick_origin   = vec_zero;
   v_angle       = vec_zero;
   oldviewangles = vec_zero;
   v_gunoffset   = vec_zero;
   v_gunangles   = vec_zero;
   viewheight    = STAND_EYE_HEIGHT;

   // blend stuff
   v_dmg_time     = 0;
   damage_blend   = vec_zero;
   flash_color[0] = flash_color[1] = flash_color[2] = flash_color[3] = 0;

   // hud
   hidestats = false;
}

void Player::ChooseSpawnPoint(void)
{
   // set up the player's spawn location
   SelectSpawnPoint(origin, angles, edict, &gravaxis);
   setOrigin(origin + Vector(0, 0, 17));
   worldorigin.copyTo(edict->s.old_origin);
   edict->s.renderfx |= RF_FRAMELERP;

   KillBox(this);

   // set gravity axis
   SetGravityAxis(gravaxis);
   client->ps.pmove.gravity_axis = gravaxis;

   // setup the orientation
   setAngles(angles);
   v_angle = worldangles;

   // set the delta angle
   client->ps.pmove.delta_angles[0] = ANGLE2SHORT(angles.x - client->resp.cmd_angles[0]);
   client->ps.pmove.delta_angles[1] = ANGLE2SHORT(angles.y - client->resp.cmd_angles[1]);
   client->ps.pmove.delta_angles[2] = ANGLE2SHORT(angles.z - client->resp.cmd_angles[2]);

   if(ctf->value && client->resp.ctf_team > 0)
      SpawnTeleportEffect(worldorigin, 124);
   else if(deathmatch->value || coop->value)
      SpawnTeleportEffect(worldorigin, 124);
}

void Player::InitPath(void)
{
   // pathnode placement stuff
   nearestNode = NULL;

   if(ai_createnodes->value)
   {
      nearestNode = new PathNode();
      nearestNode->Setup(worldorigin);
   }

   goalNode = nearestNode;
   lastNode = nearestNode;
   lastVisible = worldorigin;
   searchTime = level.time + 0.1;
}

Player::~Player()
{
   if(client)
   {
      gi.bprintf(PRINT_HIGH, "%s disconnected\n", client->pers.netname);
   }
   //
   // if he is in a vehicle 
   // get him out
   //
   if(vehicle)
   {
      Event * event;

      event = new Event(EV_Use);
      event->AddEntity(this);
      vehicle->ProcessEvent(event);
   }

   if(ctf->value)
   {
      CTF_DeadDropFlag();
      CTF_DeadDropTech();
      DropInventoryItems();
   }

   if(watchCamera && ((watchCamera == thirdpersonCamera) || (watchCamera == spectatorCamera)))
   {
      watchCamera->CancelPendingEvents();
      delete watchCamera;
      watchCamera = NULL;
      thirdpersonCamera = NULL;
      spectatorCamera = NULL;
   }

   if(crosshair)
   {
      crosshair->CancelPendingEvents();
      delete crosshair;
      crosshair = NULL;
   }

   if(path)
   {
      delete path;
      path = NULL;
   }

   edict->s.modelindex = 0;

   gi.configstring(CS_PLAYERSKINS + entnum - 1, "");
}

void Player::EndLevel(Event *ev)
{
   if(flags & FL_MUTANT)
   {
      Human(NULL);
   }

   InitPowerups();
   if(health > max_health)
   {
      health = max_health;
   }

   if(health < 1)
   {
      health = 1;
   }
}

void Player::Respawn(Event *ev)
{
   if(deathmatch->value || coop->value || level.training)
   {
      //assert ( deadflag == DEAD_DEAD );

#ifdef SIN_ARCADE
      if(deathmatch->value)
      {
         FreeInventory();
      }
#else
      if(ctf->value)
      {
         CTF_DeadDropFlag();
         CTF_DeadDropTech();
         DropInventoryItems();
      }

      FreeInventory();
#endif

      if(!gibbed && !level.training)
      {
         CopyToBodyQueue(edict);
      }

      respawn_time = level.time;

      Init();

      // hold in place briefly
      client->ps.pmove.pm_time = 50;
      client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;

      return;
   }

   // force the load game menu to come up
   level.missionfailed = true;
   level.missionfailedtime = level.time - FRAMETIME;
}

void Player::SetDeltaAngles(void)
{
   // Use v_angle since we may be in a camera
   for(int i = 0; i < 3; i++)
   {
      client->ps.pmove.delta_angles[i] = ANGLE2SHORT(v_angle[i]);
   }
}

void Player::Spectator(Event *ev)
{
   if(deadflag)
      return;

   if(spectator)
   {
      client->ps.stats[STAT_LAYOUTS] &= ~DRAW_SPECTATOR;
      spectator = false;
      if(ctf->value)
      {
         InitCTF();
      }
      else
      {
         Respawn(NULL);
      }
      defaultViewMode = FIRST_PERSON;
      SetViewMode(FIRST_PERSON);
   }
   else
   {
      //
      // if he is in a vehicle 
      // get him out
      //
      if(vehicle)
      {
         Event * event;

         event = new Event(EV_Use);
         event->AddEntity(this);
         vehicle->ProcessEvent(event);
      }

      if(ctf->value)
      {
         CTF_DeadDropFlag();
         CTF_DeadDropTech();
         CTF_ReleaseGrapple();
         client->resp.ctf_team = CTF_NOTEAM;
      }

      if(coop->value || ctf->value)
      {
         DropInventoryItems();
      }

      hideModel();
      setSolidType(SOLID_NOT);
      spectator = true;
      takedamage = DAMAGE_NO;
      FreeInventory();
      defaultViewMode = FIRST_PERSON;
      SetViewMode(FIRST_PERSON);

      client->ps.stats[STAT_LAYOUTS] |= DRAW_SPECTATOR;
      CTF_UpdateNumberOfPlayers();
      spectator_distance = 128;
   }
}

void Player::Obituary(Entity *attacker, Entity *inflictor, str location, int meansofdeath)
{
   char msg[1024];
   const char *message1 = NULL;
   const char *message2 = "";
   int num;
   if(!deathmatch->value)
      return;

   // Client killed themself
   if(attacker == this)
   {
      if(dedicated->value)
      {
         switch(meansofdeath)
         {
         case MOD_FALLING:
            message1 = "fell down";
            break;
         case MOD_ADRENALINE:
            message1 = "overdosed on adrenaline";
            break;
         case MOD_ION_DESTRUCT:
            message1 = "overloaded his gun";
            break;
         default:
            message1 = "cracked under the pressure";
         }
         if(message1)
         {
            gi.printf("%s %s.\n", client->pers.netname, message1);
         }
         else
         {
            gi.printf("%s died.\n", client->pers.netname);
         }
      }

#if OLD_DEATH_MESSAGES
      if(message1)
      {
         gi.bprintf(PRINT_MEDIUM, "%s %s.\n", client->pers.netname, message1);
      }
      else
      {
         gi.bprintf(PRINT_MEDIUM, "%s died.\n", client->pers.netname);
      }
#else
      // Send byte to the client to print deathmessage, 2nd parm is -1 beacause player
      // killed themself.
      Com_sprintf(msg, sizeof(msg), "dm %i -1 %i", edict - g_edicts - 1, meansofdeath);
      gi.WriteByte(svc_console_command);
      gi.WriteString(msg);
      gi.multicast(NULL, MULTICAST_ALL_R);
#endif
      client->resp.score--;
      return;
   }

   // Killed by another client
   if(attacker && attacker->isClient())
   {
      if(dedicated->value)
      {
         switch(meansofdeath)
         {
         case MOD_FISTS:  // FISTS
            {
               num = G_Random(6);
               switch(num)
               {
               case 0:
                  message1 = "was knocked out by";
                  break;
               case 1:
                  message1 = "was pummelled by";
                  break;
               case 2:
                  message1 = "was over come by";
                  message2 = "'s ninja technique";
                  break;
               case 3:
                  message1 = "ate a knuckle sandwich courtesy of";
                  break;
               case 4:
                  message1 = "takes";
                  message2 = "'s beating";
                  break;
               case 5:
                  message1 = "liked";
                  message2 = "'s death touch";
                  break;
               default:
                  message1 = "was killed by";
                  message2 = "'s fists";
                  break;
               }
            }
            break;
         case MOD_MUTANTHANDS:
            message1 = "was shredded by";
            break;
         case MOD_MAGNUM:  // Magnum
            {
               num = G_Random(5);
               switch(num)
               {
               case 0:
                  message1 = "was capped in the ass by";
                  break;
               case 1:
                  message1 = "was gunned down in broad daylight by";
                  break;
               case 2:
                  message1 = "eats";
                  message2 = "'s magnum";
                  break;
               case 3:
                  message1 = "was punked by";
                  break;
               case 4:
                  message1 = "was jacked by";
                  break;
               default:
                  message1 = "was killed by";
                  message2 = "'s magnum";
                  break;
               }
               break;
            }
         case MOD_SHOTGUN:  // Shotgun
            {
               num = G_Random(5);
               switch(num)
               {
               case 0:
                  message1 = "is a victim of";
                  message2 = "'s shotgun";
                  break;
               case 1:
                  message1 = "was slaughtered by";
                  break;
               case 2:
                  message1 = "drowns in a rain of";
                  message2 = "'s hot lead";
                  break;
               case 3:
                  message1 = "finds out why";
                  message2 = " got an F in gun safety";
                  break;
               case 4:
                  message1 = "was shotgunned by";
                  break;
               default:
                  message1 = "was killed by";
                  message2 = "'s shotgun";
                  break;
               }
               break;
            }
         case MOD_ASSRIFLE:
            {
               num = G_Random(4);
               switch(num)
               {
               case 0:
                  message1 = "was assaulted by";
                  break;
               case 1:
                  message1 = "was cut in half by";
                  break;
               case 2:
                  message1 = "forgot that";
                  message2 = " had a bigger gun";
                  break;
               case 3:
                  message1 = "was machinegunned down by";
                  break;
               default:
                  message1 = "was killed by";
                  message2 = "'s assault rifle";
                  break;
               }
            }
            break;
         case MOD_CHAINGUN:
            message1 = "was mowed down by";
            message2 = "'s chaingun";
            break;
         case MOD_GRENADE:
            message1 = "was fragged by";
            message2 = "'s grenade";
            break;
         case MOD_THRALLBALL:
            message1 = "ate";
            message2 = "'s ThrallBall";
            break;
         case MOD_THRALLSPLASH:
            message1 = "was bathed in";
            message2 = "'s ThrallBall";
            break;
         case MOD_ROCKET:
            {
               num = G_Random(5);
               switch(num)
               {
               case 0:
                  message1 = "was blown up by";
                  message2 = "'s direct rocket hit";
                  break;
               case 1:
                  message1 = "bows down to";
                  message2 = "'s almighty rocket launcher";
                  break;
               case 2:
                  message1 = "was incinerated by";
                  message2 = "'s rocket";
                  break;
               case 3:
                  message1 = "was obliterated by";
                  message2 = "'s rocket";
                  break;
               case 4:
                  message1 = "likes walking into";
                  message2 = "'s projectiles";
                  break;
               default:
                  message1 = "was killed by";
                  message2 = "'s rocket launcher";
               }
            }
            break;
         case MOD_ROCKETSPLASH:
            {
               num = G_Random(5);
               switch(num)
               {
               case 0:
                  message1 = "was caught in";
                  message2 = "'s explosion";
                  break;
               case 1:
                  message1 = "was burned to a crisp in";
                  message2 = "'s blast";
                  break;
               case 2:
                  message1 = "is pulling shrapnel out of his ass thanks to";
                  break;
               case 3:
                  message1 = "couldn't dodge";
                  message2 = "'s blast damage";
                  break;
               case 4:
                  message1 = "was drenched by";
                  message2 = "'s splash damage";
                  break;
               default:
                  message1 = "was killed by";
                  message2 = "'s explosion";
               }
               break;
            }
         case MOD_SPIDERSPLASH:
            message1 = "was bitten by";
            message2 = "'s spidermine";
            break;
         case MOD_SPEARGUN:
            message1 = "was stuck by";
            message2 = "'s spear";
            break;
         case MOD_PULSE:
            message1 = "was pulsed by";
            break;
         case MOD_PULSELASER:
            message1 = "was burned to a crisp by";
            message2 = "'s laser";
            break;
         case MOD_ION:
         case MOD_ION_DESTRUCT:
            message1 = "Was ionized by";
            break;
         case MOD_QUANTUM:
            message1 = "Was particlized by";
            break;
         case MOD_SNIPER:
            {
               num = G_Random(5);
               switch(num)
               {
               case 0:
                  message1 = "was hollowpointed by";
                  break;
               case 1:
                  message1 = "was put to sleep by";
                  break;
               case 2:
                  message1 = "was sniped by";
                  break;
               case 3:
                  message1 = "never saw it coming from";
                  break;
               case 4:
                  message1 = "was killed by a precision shot from";
                  break;
               default:
                  message1 = "was killed by";
                  message2 = "'s sniper rifle";
               }
               break;
            }
         case MOD_EMPATHY:
            message1 = "felt";
            message2 = "'s pain";
            break;
         case MOD_GRAPPLE:
            message1 = "got stuck on";
            message2 = "'s grapple";
            break;
         case MOD_FRIENDLY_FIRE:
            message1 = "was killed by teammate";
            break;
         case MOD_CTFTURRET:
            message1 = "was caught in the sights of";
            message2 = "'s turret";
            break;
         default:
            message1 = "was killed by";
         }
         gi.printf("%s %s %s%s\n", client->pers.netname,
                   message1,
                   attacker->client->pers.netname,
                   message2);
      }
#if OLD_DEATH_MESSAGES
      gi.bprintf(PRINT_MEDIUM, "%s %s %s%s\n", client->pers.netname,
                 message1,
                 attacker->client->pers.netname,
                 message2);

#else
      Com_sprintf(msg, sizeof(msg), "dm %i %i %i", edict - g_edicts - 1, attacker->edict - g_edicts - 1, meansofdeath);
      gi.WriteByte(svc_console_command);
      gi.WriteString(msg);
      gi.multicast(NULL, MULTICAST_ALL_R);
#endif

      if(meansofdeath != MOD_FRIENDLY_FIRE)
      {
         // Increment attacker's score if it wasn't a friendly kill
         attacker->client->resp.score++;
      }

      return;
   }

   // Killed by a non-client
   switch(meansofdeath)
   {
   case MOD_FALLING:
      {
         // Credit the last person who damaged me with the kill
         if(enemy && enemy->isClient() && (enemy != this) && level.time < lastEnemyTime)
         {
            attacker = enemy;
            message1 = "was pushed over the edge by";
            message2 = "";

            // Give the last enemy a frag
            enemy->client->resp.score++;
         }
         else
         {
            message1 = "fell down";
            message2 = "";
         }
         break;
      }
   case MOD_DEATHQUAD:
      {
         message1 = "held on to the DeathQuad for too long";
         message2 = "";
         break;
      }
   }

   // Killed indirectly by enemy
   if(message1 && attacker->isClient())
   {
      gi.bprintf(PRINT_MEDIUM, "%s %s %s%s\n", client->pers.netname,
                 message1,
                 attacker->client->pers.netname,
                 message2);
   }
   // Killed by world somehow
   else if(message1)
   {
      gi.bprintf(PRINT_MEDIUM, "%s %s\n", client->pers.netname,
                 message1);
      client->resp.score--;
   }
   else
   {
      gi.bprintf(PRINT_MEDIUM, "%s died.\n", client->pers.netname);
      client->resp.score--;
   }
}

void Player::Dead(Event *ev)
{
   deadflag = DEAD_DEAD;
   StopAnimating();

   //
   // drop anything that might be attached to us
   //
   if(numchildren && (level.training != 2))
   {
      Entity * child;
      int i;
      //
      // detach all our children
      //
      for(i = 0; i < MAX_MODEL_CHILDREN; i++)
      {
         if(children[i])
         {
            child = (Entity *)G_GetEntity(children[i]);
            child->ProcessEvent(EV_Detach);
         }
      }
   }

   if(!deathmatch->value && !coop->value && !level.training)
   {
      // Fade to white
      gi.WriteByte(svc_console_command);
      gi.WriteString("fo 0.8 1 1 1");
      gi.unicast(edict, false);
      PostEvent(EV_Player_Respawn, 3.0f);
   }

#ifdef SIN_ARCADE
   gi.WriteByte(svc_stufftext);
   gi.WriteString("playerdead");
   gi.unicast(edict, true);
#endif
}

void Player::Killed(Event *ev)
{
   Entity   *attacker;
   Entity   *inflictor;
   str      location;
   str      prefix;
   str      aname;
   int      meansofdeath;
   int      i;
   qboolean deathgib = false;
   str      dname;
   Event    *ev1;

   attacker     = ev->GetEntity(1);
   inflictor    = ev->GetEntity(3);
   location     = ev->GetString(4);
   meansofdeath = ev->GetInteger(5);

   //
   // if he is in a vehicle 
   // get him out
   //
   if(vehicle)
   {
      Event *event;

      event = new Event(EV_Use);
      event->AddEntity(this);
      vehicle->ProcessEvent(event);
   }

   // Set the enemy to attacker if it is a client, otherwise
   // some world object killed the player.  We don't overwrite
   // enemy with non-clients, so clients can get credit for
   // kills they caused (i.e. shooting someone caused them to fall )
   if(attacker->isClient())
   {
      enemy = attacker;
   }

   // get rid of the third person crosshair if we have one
   if(crosshair)
   {
      crosshair->CancelPendingEvents();
      delete crosshair;
      crosshair = NULL;
   }

   if(ctf->value && (meansofdeath != MOD_FRIENDLY_FIRE))
   {
      if(attacker->isClient())
         CTF_FragBonuses(this, (Sentient *)attacker);
   }

   Obituary(attacker, inflictor, location, meansofdeath);

   StopAnimating();
   CancelPendingEvents();

   //
   // determine death animation
   //
   if(DoGib(meansofdeath, inflictor))
   {
      deathgib = true;
   }

   prefix = AnimPrefixForPlayer();

   if(deathgib)
   {
      str location;

      location = ev->GetString(4);

      // Check for location first otherwise randomize
      if(location == "torso_upper")
         dname += str("gibdeath_upper");
      else if(location == "torso_lower")
         dname += str("gibdeath_lower");
      else if(strstr(location.c_str(), "leg"))
         dname += str("gibdeath_lower");
      else if(strstr(location.c_str(), "arm"))
         dname += str("gibdeath_upper");
      else if(strstr(location.c_str(), "head"))
         dname += str("gibdeath_upper");
      else if(G_Random() > 0.5)
         dname += str("gibdeath_upper");
      else
         dname += str("gibdeath_lower");
   }

   if(deathgib)
      aname = prefix + dname;
   else
      aname = prefix + str("death_") + location;

   i = gi.Anim_Random(edict->s.modelindex, aname.c_str());

   if(i == -1)
   {
      aname = prefix + str("death");
   }

   // moved to before the deadflag being set to prevent a definate game crash
   if(flags & (FL_MUTANT | FL_SP_MUTANT))
   {
      Human(NULL);
   }

   deadflag = DEAD_DYING;

   respawn_time = level.time + 1.0;

   if(currentWeapon)
   {
      if(!level.training)
      {
         DropWeapon(currentWeapon);
      }
      else if(level.training != 2)
      {
         currentWeapon->CancelPendingEvents();
         delete currentWeapon;
         currentWeapon = NULL;
      }
   }

   if(ctf->value)
   {
      CTF_DeadDropFlag();
      CTF_DeadDropTech();
      CTF_ReleaseGrapple();
   }

   if(coop->value || ctf->value)
   {
      DropInventoryItems();
   }

   edict->clipmask = MASK_DEADSOLID;
   edict->svflags |= SVF_DEADMONSTER;

   ProcessEvent(EV_Player_ZoomOut);
   SetViewMode(THIRD_PERSON);
   setMoveType(MOVETYPE_NONE);

   edict->s.renderfx &= ~RF_DLIGHT;
   edict->s.renderfx &= ~RF_ENVMAPPED;

   angles.x = 0;
   angles.z = 0;
   setAngles(angles);

   if(deathmatch->value)
   {
      client->showinfo = true;
      G_DeathmatchScoreboard(this);
   }

   if(flags & (FL_MUTANT | FL_SP_MUTANT))
   {
      ev1 = new Event(EV_Gib);
      ev1->AddInteger(0);
      ProcessEvent(ev1);
      ProcessEvent(EV_Player_Dead);
   }
   else if(strstr(aname.c_str(), "gibdeath"))
   {
      ev1 = new Event(EV_Gib);
      ev1->AddInteger(1);
      ProcessEvent(ev1);
   }

   RandomAnimate(aname.c_str(), EV_Player_Dead);
   animOverride = true;

   //
   // change music
   //
   ChangeMusic("failure", "normal", true);

   if(!deathmatch->value && !level.training && !level.no_jc)
   {
      char name[128];
      int num;

      num = (int)G_Random(4) + 1;
      snprintf(name, sizeof(name), "global/universal_script.scr::jc_talks_blade_dies%d", num);
      ExecuteThread(name, true);
   }
}

void Player::Pain(Event *ev)
{
   float		damage, delta;
   Entity	*attacker;
   int      meansofdeath;

   damage = ev->GetFloat(1);
   attacker = ev->GetEntity(2);
   meansofdeath = ev->GetInteger(4);

   if(viewmode == CAMERA_VIEW)
   {
      if(level.cinematic)
      {
         return;
      }
      else
      {
         ExitConsole(NULL);
         SetCamera(NULL);
      }
   }

   // add to our damage time
   v_dmg_time = level.time + DAMAGE_TIME;

   if(attacker->isClient())
   {
      if(!deadflag)
      {
         enemy = attacker;
         lastEnemyTime = level.time + ENEMY_TIME;
      }
   }

   // increase action level of game
   action_level += damage;

   // add to the damage inflicted on a player this frame
   // the total will be turned into screen blends and view angle kicks
   // at the end of the frame
   damage_blood += damage;

   // white flash signifying no damage was taken.
   if((damage <= 0) && (meansofdeath != MOD_FISTS))
   {
      Event *ev1;

      ProcessEvent(EV_Player_ClearFlashColor);
      ev1 = new Event(EV_Player_SetFlashColor);

      // Flash the player white
      ev1->AddFloat(1);
      ev1->AddFloat(1);
      ev1->AddFloat(1);
      ev1->AddFloat(0.3);
      ProcessEvent(ev1);
   }

   delta = level.time - last_damage_time;

   // Don' play more than one pain sound or animation a second
   if(delta < 1.0)
   {
      return;
   }

   last_damage_time = level.time;

   // If damage > 5 then play a pain animation, otherwise play a pain sound
   if(!firing && !deadflag && !vehicle && damage > 5)
   {
      str prefix;
      str aname;
      int index;

      //
      // determine pain animation
      //
      prefix = AnimPrefixForPlayer();
      aname = prefix + str("pain_") + str(ev->GetString(3));
      index = gi.Anim_Random(edict->s.modelindex, aname.c_str());
      if(index == -1)
      {
         aname = prefix + str("pain");
      }

      TempAnim(aname.c_str(), NULL);
   }
   else
   {
      RandomSound("snd_pain", 1);
      ProcessEvent(EV_PainSound);
   }
}

void Player::DoUse(void)
{
   int		i;
   int		num;
   edict_t	*touch[MAX_EDICTS];
   edict_t	*hit;
   Event		*ev;
   Vector	min;
   Vector	max;
   Vector	offset;
   trace_t	trace;
   Vector	start;
   Vector	end;
   float		t;

   if(spectator)
   {
      ChangeSpectator();
      return;
   }

   if(ctf->value && vehicle)
   {
      ev = new Event(EV_Use);
      ev->AddEntity(this);
      vehicle->ProcessEvent(ev);
      return;
   }

   // Pickup inventory if one exists
   PickupFloatingInventory();

   start = worldorigin + Vector(0, 0, viewheight);
   end = start + Vector(orientation[0]) * 64.0f;

   trace = G_Trace(start, vec_zero, vec_zero, end, this, MASK_SOLID, "Player::DoUse");

   t = 64 * trace.fraction - maxs[0];
   if(t < 0)
   {
      t = 0;
   }

   offset = Vector(orientation[0]) * t;

   min = start + offset + Vector(-16, -16, -16);
   max = start + offset + Vector(16, 16, 16);

   num = gi.BoxEdicts(min.vec3(), max.vec3(), touch, MAX_EDICTS, AREA_TRIGGERS);

   // be careful, it is possible to have an entity in this
   // list removed before we get to it (killtriggered)
   for(i = 0; i < num; i++)
   {
      hit = touch[i];
      if(!hit->inuse)
      {
         continue;
      }

      assert(hit->entity);

      ev = new Event(EV_Use);
      ev->AddEntity(this);
      hit->entity->ProcessEvent(ev);
   }

   num = gi.BoxEdicts(min.vec3(), max.vec3(), touch, MAX_EDICTS, AREA_SOLID);

   // be careful, it is possible to have an entity in this
   // list removed before we get to it (killtriggered)
   for(i = 0; i < num; i++)
   {
      hit = touch[i];
      if(!hit->inuse)
      {
         continue;
      }

      assert(hit->entity);

      ev = new Event(EV_Use);
      ev->AddEntity(this);
      hit->entity->ProcessEvent(ev);
   }

   // Force a reload on a weapon
   if(currentWeapon)
   {
      currentWeapon->ForceReload();
   }
}

static Entity *pm_passent;

// pmove doesn't need to know about passent and contentmask
extern "C" trace_t PM_trace(vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end)
{
   if((pm_passent->flags & FL_GODMODE) || (pm_passent->health > 0))
   {
      return G_Trace(start, mins, maxs, end, pm_passent->edict, MASK_PLAYERSOLID, "PM_trace 1");
   }
   else
   {
      return G_Trace(start, mins, maxs, end, pm_passent->edict, MASK_DEADSOLID, "PM_trace 1");
   }
}

EXPORT_FROM_DLL void Player::SetViewMode(viewmode_t mode, Entity * newCamera)
{
   Camera *c;

   viewmode = mode;

   if(watchCamera && ((watchCamera == thirdpersonCamera) || (watchCamera == spectatorCamera)))
   {
      watchCamera->CancelPendingEvents();
      delete watchCamera;
      watchCamera = NULL;
      thirdpersonCamera = NULL;
      spectatorCamera = NULL;
   }

   if(crosshair)
   {
      crosshair->CancelPendingEvents();
      delete crosshair;
      crosshair = NULL;
   }

   switch(viewmode)
   {
   default:
   case THIRD_PERSON:
      {
         Event * ev;
         Vector pos;

         c = new Camera();

         ev = new Event(EV_Camera_MoveToPos);
         pos = worldorigin;
         pos.z = absmax.z;
         ev->AddVector(pos);
         c->ProcessEvent(ev);

         ev = new Event(EV_Camera_TurnTo);
         ev->AddVector(v_angle);
         c->ProcessEvent(ev);

         if(vehicle)
            c->FollowEntity(this, 256, MASK_OPAQUE);
         else
            c->FollowEntity(this, 128, MASK_PLAYERSOLID);
         watchCamera = c;
         thirdpersonCamera = c;

         if(!deadflag)
         {
            cvar_t * chair;

            chair = gi.cvar("crosshair", "0", 0);
            crosshair = new Entity();
            crosshair->setModel(va("sprites/crosshair%d.spr", (int)chair->value));
            crosshair->edict->svflags |= SVF_ONLYPARENT;
            crosshair->edict->owner = this->edict;
         }
      }
      break;

   case SPECTATOR:
      {
         Event * ev;
         Vector pos;
         edict_t		*cl_ent;

         cl_ent = g_edicts + 1 + currentCameraTarget;
         if
            (
               !cl_ent->inuse ||
               !cl_ent->entity ||
               (!cl_ent->entity->client) ||
               (((Player *)cl_ent->entity)->spectator)
               )
         {
            ChangeSpectator();
            if
               (
                  !cl_ent->inuse ||
                  !cl_ent->entity ||
                  (!cl_ent->entity->client) ||
                  (((Player *)cl_ent->entity)->spectator)
                  )
            {
               SetViewMode(FIRST_PERSON);
               return;
            }
         }
         c = new Camera();

         ev = new Event(EV_Camera_MoveToPos);
         pos = cl_ent->entity->worldorigin;
         pos.z = cl_ent->entity->absmax.z;
         ev->AddVector(pos);
         c->ProcessEvent(ev);

         ev = new Event(EV_Camera_TurnTo);
         ev->AddVector(cl_ent->entity->worldangles);
         c->ProcessEvent(ev);

         c->FollowEntity(cl_ent->entity, spectator_distance, MASK_PLAYERSOLID);
         watchCamera = c;
         spectatorCamera = c;
      }
      break;

   case FIRST_PERSON:
      watchCamera = this;
      break;

   case CAMERA_VIEW:
      watchCamera = newCamera;
      break;
   }
}

EXPORT_FROM_DLL void Player::CheckButtons(void)
{
   // Only process buttons if you're not dying
   if((deadflag != DEAD_NO) || spectator)
   {

      if(spectator && ((new_buttons & BUTTON_ATTACK) && (!firedown)))
      {
         // we set defaultViewMode as if we toggled the camera
         if(defaultViewMode == FIRST_PERSON)
         {
            defaultViewMode = SPECTATOR;
         }
         ChangeSpectator();
      }
      else if
         (
            spectator && (buttons & BUTTON_USE) && (viewmode == SPECTATOR)
            )
      {
         spectator_distance += 2;
         if(spectator_distance > 384)
            spectator_distance = 0;
         if(spectatorCamera)
         {
            Camera * cam;

            cam = (Camera *)((Entity *)spectatorCamera);
            cam->SetCurrentDistance(spectator_distance);
         }
      }
      return;
   }

   // Check for the use key getting pressed
   if(new_buttons & BUTTON_USE)
   {
      if(ctf->value)
      {
         usedown = true;
         CTF_ExtendGrapple();
      }
      DoUse();
   }

   if(ctf->value)
   {
      // Check for use key being released
      if(!(buttons & BUTTON_USE))
      {
         if(usedown)
         {
            CTF_ReleaseGrapple();
            usedown = false;
         }
      }
   }

   // Check for button release
   if(!(buttons & BUTTON_ATTACK))
   {
      if(firedown)
      {
         Event *event;
         event = new Event(EV_Sentient_ReleaseAttack);
         event->AddFloat(level.time - firedowntime);
         ProcessEvent(event);
         firedown = false;
      }
   }

   // fire weapon from final position if needed
   if((buttons & BUTTON_ATTACK) && WeaponReady())
   {
      //
      // raise the action level
      //
      if(currentWeapon)
         action_level += currentWeapon->ActionLevelIncrement();

      firedown = true;
      firedowntime = level.time;

      if(!vehicle)
      {
         str name;
         str prefix;

         prefix = AnimPrefixForPlayer();
         //
         // append the prefix based on which weapon we are holding
         //
         prefix += str(AnimPrefixForWeapon());

         if((xyspeed > 20))
         {
            int num;
            int numframes;

            if((waterlevel > 2) || (xyspeed > 250))
               name = prefix + str("run_fire");
            else
               name = prefix + str("walk_fire");
            num = gi.Anim_Random(edict->s.modelindex, name.c_str());
            if(num != -1)
            {
               numframes = gi.Anim_NumFrames(edict->s.modelindex, num);
               if((last_frame_in_anim + 1) == numframes)
                  edict->s.anim = num;
               else
                  TempAnim(name.c_str(), NULL);
            }
            else
            {
               name = prefix + str("fire");
               TempAnim(name.c_str(), NULL);
            }
         }
         else
         {
            name = prefix + str("fire");
            TempAnim(name.c_str(), NULL);
         }
      }

      ProcessEvent(EV_Sentient_Attack);

      if(ctf->value)
      {
         Event *ctfev;

         ctfev = new Event(EV_Player_CTF_SoundEvent);
         ctfev->AddInteger(FIRE);
         ProcessEvent(ctfev);
      }

      if(ai_createnodes->value)
      {
         goalNode = nearestNode;
         if(!goalNode)
         {
            if(ai_debugpath->value)
            {
               gi.dprintf("New path node\n");
            }
            nearestNode = new PathNode();
            nearestNode->Setup(worldorigin);
            goalNode = nearestNode;
            lastNode = nearestNode;
         }
      }
   }
}

EXPORT_FROM_DLL void Player::TouchStuff(pmove_t *pm)
{
   edict_t	*other;
   Event		*event;
   int		i;
   int		j;

   if(spectator)
   {
      return;
   }

   if(getMoveType() != MOVETYPE_NOCLIP)
   {
      G_TouchTriggers(this);
   }

   // touch other objects
   for(i = 0; i < pm->numtouch; i++)
   {
      other = pm->touchents[i];
      for(j = 0; j < i; j++)
      {
         if(pm->touchents[j] == other)
            break;
      }

      if(j != i)
      {
         // duplicated
         continue;
      }

      // Don't bother touching the world
      if((!other->entity) || (other->entity == world))
      {
         continue;
      }

      event = new Event(EV_Touch);
      event->AddEntity(this);
      other->entity->ProcessEvent(event);

      event = new Event(EV_Touch);
      event->AddEntity(other->entity);
      ProcessEvent(event);
   }
}

EXPORT_FROM_DLL void Player::EnterVehicle(Event *ev)
{
   Entity *ent;

   ent = ev->GetEntity(1);
   if(ent && ent->isSubclassOf<Vehicle>())
   {
      viewheight = STAND_EYE_HEIGHT;
      levelVars.SetVariable("player_in_vehicle", 1);
      velocity = vec_zero;
      vehicle = (Vehicle *)ent;
      if(vehicle->IsDrivable())
         setMoveType(MOVETYPE_VEHICLE);
      else
         setMoveType(MOVETYPE_NOCLIP);
      if(ev->NumArgs() > 1)
         vehicleanim = ev->GetString(2);
      else
         vehicleanim = "drive";
   }
}

EXPORT_FROM_DLL void Player::ExitVehicle(Event *ev)
{
   levelVars.SetVariable("player_in_vehicle", 0);
   setMoveType(MOVETYPE_WALK);
   vehicle = NULL;
}

EXPORT_FROM_DLL void Player::GetMoveInfo(pmove_t *pm)
{
   if(!deadflag)
   {
      v_angle[0] = pm->viewangles[0];
      v_angle[1] = pm->viewangles[1];
      v_angle[2] = pm->viewangles[2];
      AnglesToMat(v_angle.vec3(), orientation);
   }

   client->ps.pmove = pm->s;
   old_pmove = pm->s;

   setOrigin(Vector(pm->s.origin[0], pm->s.origin[1], pm->s.origin[2]) * 0.125);
   velocity = Vector(pm->s.velocity[0], pm->s.velocity[1], pm->s.velocity[2]) * 0.125;

   if((client->ps.pmove.pm_type == PM_FREEZE) ||
      (client->ps.pmove.pm_type == PM_INVEHICLE_ZOOM) ||
      (client->ps.pmove.pm_type == PM_INVEHICLE))
   {
      velocity = vec_zero;
   }

   setSize(pm->mins, pm->maxs);

   gunoffset = Vector(0, 0, pm->viewheight);
   viewheight = pm->viewheight;
   waterlevel = pm->waterlevel;
   watertype = pm->watertype;
   onladder = pm->onladder;
   if(pm->groundentity)
   {
      groundentity = pm->groundentity;
      groundsurface = pm->groundsurface;
      groundplane = pm->groundplane;
      groundcontents = pm->groundcontents;
      groundentity_linkcount = pm->groundentity->linkcount;
   }
   else
   {
      groundentity = NULL;
      groundsurface = NULL;
      groundcontents = 0;
   }
   if(pm->groundsurface)
      fallsurface = pm->groundsurface;
}

EXPORT_FROM_DLL void Player::SetMoveInfo(pmove_t *pm, usercmd_t *ucmd)
{
   pm_passent = this;

   // set up for pmove
   memset(pm, 0, sizeof(pmove_t));

   pm->s = client->ps.pmove;

   pm->s.origin[0] = worldorigin.x * 8;
   pm->s.origin[1] = worldorigin.y * 8;
   pm->s.origin[2] = worldorigin.z * 8;

   pm->s.velocity[0] = velocity.x * 8;
   pm->s.velocity[1] = velocity.y * 8;
   pm->s.velocity[2] = velocity.z * 8;

   if(!vehicle && memcmp(&old_pmove, &pm->s, sizeof(pm->s)))
   {
      pm->snapinitial = true;
   }

   pm->cmd = *ucmd;

   pm->trace = PM_trace;	// adds default parms
   pm->pointcontents = gi.pointcontents;
}

EXPORT_FROM_DLL pmtype_t Player::GetMovePlayerMoveType(void)
{
   if(level.playerfrozen || sentientFrozen)
   {
      return PM_FREEZE;
   }
   else if((viewmode == CAMERA_VIEW) || (viewmode == SPECTATOR))
   {
      return PM_LOCKVIEW;
   }
   else if(vehicle)
   {
      if(zoom_mode == ZOOMED_IN)
      {
         return PM_INVEHICLE_ZOOM;
      }
      else
      {
         return PM_INVEHICLE;
      }
   }
   else if((getMoveType() == MOVETYPE_NOCLIP) || (spectator))
   {
      return PM_SPECTATOR;
   }
   else if(deadflag)
   {
      return PM_DEAD;
   }
   else if(zoom_mode == ZOOMED_IN)
   {
      return PM_ZOOM;
   }
   else if((viewmode == THIRD_PERSON) && (!gravaxis))
   {
      return PM_LOCKVIEW;
   }
   else if(grapple_pull)
   {
      return PM_GRAPPLE_PULL;
   }
   else
   {
      return PM_NORMAL;
   }
}

EXPORT_FROM_DLL void Player::ClientMove(usercmd_t *ucmd)
{
   pmove_t pm;

   client->ps.pmove.pm_type = GetMovePlayerMoveType();
   if(
      (client->ps.pmove.pm_type == PM_LOCKVIEW) ||
      (client->ps.pmove.pm_type == PM_INVEHICLE) ||
      (client->ps.pmove.pm_type == PM_INVEHICLE_ZOOM) ||
      (client->ps.pmove.pm_type == PM_FREEZE) ||
      (client->ps.pmove.pm_type == PM_DEAD) ||
      (client->ps.pmove.pm_type == PM_GRAPPLE_PULL)
      )
      client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
   else
      client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;

   if(flags & FL_MUTANT)
      client->ps.pmove.pm_flags |= PMF_MUTANT;
   else
      client->ps.pmove.pm_flags &= ~PMF_MUTANT;

   if(flags & FL_ADRENALINE)
      client->ps.pmove.pm_flags |= PMF_ADRENALINE;
   else
      client->ps.pmove.pm_flags &= ~PMF_ADRENALINE;

   if(level.airclamp)
      client->ps.pmove.pm_flags &= ~PMF_NOAIRCLAMP;
   else
      client->ps.pmove.pm_flags |= PMF_NOAIRCLAMP;

   edict->s.effects &= ~EF_NOFOOTSTEPS;
   if(!sv_footsteps->value)
   {
      edict->s.effects |= EF_NOFOOTSTEPS;
   }

   client->ps.pmove.gravity = sv_gravity->value * gravity;
   SetMoveInfo(&pm, ucmd);

   // perform a pmove
   gi.Pmove(&pm);

   if(!deadflag && !level.playerfrozen)
   {
      if(waterlevel == 3 && falling)
      {
         Event * ev;

         ev = new Event(EV_Sentient_AnimLoop);
         ProcessEvent(ev);
         falling = false;
      }
      if(groundentity && !pm.groundentity && (pm.cmd.upmove >= 10) && (waterlevel <= 1))
      {
         TempAnim("jump", NULL);
         falling = true;
      }
      if(!groundentity && pm.groundentity && ((pm.s.velocity[2] < -1600) || falling))
      {
         falling = false;
         if(waterlevel <= 2)
         {
            TempAnim("land", NULL);
         }
      }
      if((!onladder) && !pm.groundentity && (velocity[2] < -200) && (pm.s.velocity[2] > -2000))
      {
         if(!waterlevel)
         {
            TempAnim("fall", NULL);
            falling = true;
         }
      }
   }

   // save results of pmove
   GetMoveInfo(&pm);

   TouchStuff(&pm);
   if(!level.playerfrozen)
      CheckButtons();
   if(whereami->value)
   {
      static Vector last;
      if(worldorigin != last)
      {
         last = worldorigin;
         gi.dprintf("x %8.2f y%8.2f z %8.2f area %2d\n", worldorigin[0], worldorigin[1], worldorigin[2], edict->areanum);
      }
   }
}

/*
==============
ClientThink

This will be called once for each client frame, which will
usually be a couple times for each server frame.
==============
*/
EXPORT_FROM_DLL void Player::ClientThink(Event *ev)
{
   if(!has_thought)
   {
      // let any threads waiting on us know they can go ahead
      Director.PlayerSpawned();
      has_thought = true;
   }

   new_buttons = current_ucmd->buttons & ~buttons;
   buttons = current_ucmd->buttons;

   // set the current gravity axis
   client->ps.pmove.gravity_axis = gravaxis;

   // save light level the player is standing on for
   // monster sighting AI
   light_level = current_ucmd->lightlevel;

   if(level.intermissiontime)
   {
      client->ps.pmove.pm_type = PM_FREEZE;

      // can exit intermission after 12 seconds
      if(level.time > level.intermissiontime + 12.0 && (new_buttons & BUTTON_ANY))
      {
         level.exitintermission = true;
      }

      // Save cmd angles so that we can get delta angle movements next frame
      client->resp.cmd_angles[0] = SHORT2ANGLE(current_ucmd->angles[0]);
      client->resp.cmd_angles[1] = SHORT2ANGLE(current_ucmd->angles[1]);
      client->resp.cmd_angles[2] = SHORT2ANGLE(current_ucmd->angles[2]);

      return;
   }
   if(current_ucmd->debris)
   {
      float mult;
      mult = SURFACE_DamageMultiplier(current_ucmd->debris << SURF_START_BIT);
      mult *= 2;
      if(mult > 0)
      {
         Damage(world, world, (int)mult, worldorigin, vec_zero, vec_zero, 0, 0, MOD_DEBRIS, -1, -1, 1.0f);
      }
   }
   if((client->ps.pmove.pm_flags & PMF_TIME_TELEPORT) &&
      (savedfov) &&
      (client->ps.pmove.pm_time < 95))
   {
      Event * ev;

      ev = new Event(EV_Player_RestoreFov);
      ProcessEvent(ev);
   }

   if(!vehicle || !vehicle->Drive(current_ucmd))
   {
      short temporigin[3];

      if(watchCamera != this)
      {
         // 
         // we save off the origin so that camera's origins are not messed up
         //
         temporigin[0] = client->ps.pmove.origin[0];
         temporigin[1] = client->ps.pmove.origin[1];
         temporigin[2] = client->ps.pmove.origin[2];

         ClientMove(current_ucmd);

         client->ps.pmove.origin[0] = temporigin[0];
         client->ps.pmove.origin[1] = temporigin[1];
         client->ps.pmove.origin[2] = temporigin[2];
      }
      else
      {
         ClientMove(current_ucmd);
      }

   }
   else
   {
      CheckWater();
      if(!level.playerfrozen)
         CheckButtons();
   }

   // If we're dying, check for respawn

   assert(!(deadflag && deadflag != DEAD_DEAD && !animating));

   if((deadflag == DEAD_DEAD && (level.time > respawn_time)) ||
      (deadflag && !animating))
   {
      if((deathmatch->value) && (!checked_dead_camera) && (level.time > (respawn_time + 10)))
      {
         int num;

         checked_dead_camera = true;
         // Find the end node
         num = G_FindTarget(0, "endnode1");

         if(num && thirdpersonCamera)
         {
            Entity * path;
            Event * event;

            event = new Event(EV_Camera_NormalAngles);
            thirdpersonCamera->ProcessEvent(event);
            event = new Event(EV_Camera_Orbit);
            path = G_GetEntity(num);
            event->AddEntity(path);
            thirdpersonCamera->ProcessEvent(event);
            event = new Event(EV_Camera_JumpCut);
            thirdpersonCamera->ProcessEvent(event);
         }
      }
      // wait for any button just going down
      if(new_buttons || (DM_FLAG(DF_FORCE_RESPAWN)))
      {
         ProcessEvent(EV_Player_Respawn);
      }
   }

   // 
   // check to see if we want to get out of a cinematic or camera
   //
   if((viewmode == CAMERA_VIEW) &&
      ((buttons & BUTTON_ATTACK) ||
       (buttons & BUTTON_USE) ||
       (abs(current_ucmd->forwardmove) >= 200) ||
       (abs(current_ucmd->sidemove) >= 200) ||
       (current_ucmd->upmove > 0)))
   {
      if(level.cinematic)
      {
         if((buttons & BUTTON_ATTACK) ||
            (buttons & BUTTON_USE) ||
            (current_ucmd->upmove > 0))
         {
            if(world->skipthread.length() > 1)
            {
               ExecuteThread(world->skipthread);
            }
         }
      }
      else
      {
         if(!(buttons & BUTTON_USE) &&
            !(buttons & BUTTON_ATTACK) &&
            !(trappedInQuantum) &&
            !(in_console))
         {
            SetCamera(NULL);
         }
      }
   }

   // Save cmd angles so that we can get delta angle movements next frame
   client->resp.cmd_angles[0] = SHORT2ANGLE(current_ucmd->angles[0]);
   client->resp.cmd_angles[1] = SHORT2ANGLE(current_ucmd->angles[1]);
   client->resp.cmd_angles[2] = SHORT2ANGLE(current_ucmd->angles[2]);
}

void Player::EventUseItem(Event *ev)
{
   const char *name;

   name = ev->GetString(1);
   assert(name);

   if(deadflag)
      return;

   if(flags & (FL_MUTANT | FL_SP_MUTANT))
      return;

   if(currentWeapon && !currentWeapon->ChangingWeapons())
   {
      if(!Q_stricmp(name, currentWeapon->getClassname()))
      {
         Event *event;
         event = new Event(EV_Weapon_SecondaryUse);
         event->AddEntity(this);
         currentWeapon->ProcessEvent(event);
         return;
      }
   }

   ProcessEvent(EV_Player_ZoomOut);

   if(checkInheritance(&Weapon::ClassInfo, name))
      useWeapon(name);
}

void Player::EventUseInventoryItem(Event *ev)
{
   if(deadflag)
      return;

   if(!currentItem)
      return;

   if(flags & (FL_MUTANT | FL_SP_MUTANT))
      return;

   currentItem->ProcessEvent(EV_InventoryItem_Use);
}

Weapon *Player::useWeapon(const char *weaponname)
{
   if(vehicle && vehicle->HasWeapon())
      return NULL;
   else
      return Sentient::useWeapon(weaponname);
}

void Player::Take(Event *ev)
{
   const char *name;

   name = ev->GetString(1);

   assert(name);

   if(deadflag)
   {
      return;
   }

   if(!stricmp(name, "all"))
   {
      if(currentWeapon)
         currentWeapon->DetachFromOwner();

      SetCurrentWeapon(NULL);
      FreeInventoryOfType("Weapon");
      giveWeapon("Fists");
      giveWeapon("SpiderMine");
   }
   else if(checkInheritance(&Weapon::ClassInfo, name))
   {
      takeWeapon(name);
   }
   else if(checkInheritance(&Ammo::ClassInfo, name))
   {
      EventTakeAmmo(ev);
   }
   else if(checkInheritance(&Item::ClassInfo, name))
   {
      EventTakeItem(ev);
   }
   else
   {
      gi.cprintf(edict, PRINT_HIGH, "Unknown take object : %s\n", name);
   }
}

void Player::GiveCheat(Event *ev)
{
   const char *name;

   name = ev->GetString(1);

   assert(name);

   if(deadflag)
   {
      return;
   }

   if(checkInheritance(&Weapon::ClassInfo, name))
   {
      giveWeapon(name);
   }
   else if(checkInheritance(&Ammo::ClassInfo, name))
   {
      EventGiveAmmo(ev);
   }
   else if(checkInheritance(&InventoryItem::ClassInfo, name))
   {
      if(ev->NumArgs() != 2)
      {
         gi.cprintf(edict, PRINT_HIGH, "Usage: give <inventory item name> <amount>\n");
         return;
      }
      EventGiveInventoryItem(ev);
   }
   else if(checkInheritance(&Item::ClassInfo, name))
   {
      EventGiveItem(ev);
   }
}

void Player::GiveAllCheat(Event *ev)
{
   if(deadflag)
   {
      return;
   }

#ifdef SIN_DEMO
   giveWeapon("Fists");
   giveWeapon("RocketLauncher");
   giveWeapon("Magnum");
   giveWeapon("SniperRifle");
   giveWeapon("AssaultRifle");
   giveWeapon("Shotgun");
#else
   giveWeapon("Fists");
   giveWeapon("RocketLauncher");
   giveWeapon("Magnum");
   giveWeapon("SniperRifle");
   giveWeapon("AssaultRifle");
   giveWeapon("ChainGun");
   giveWeapon("PulseRifle");
   giveWeapon("Shotgun");
   giveWeapon("SpearGun");
   giveWeapon("SpiderMine");
   giveWeapon("MutantHands");
   giveWeapon("QuantumDestabilizer");
#endif
   giveItem("RiotHelmet", 100);
   giveItem("FlakJacket", 100);
   giveItem("FlakPants",  100);
}

void Player::GodCheat(Event *ev)
{
   const char *msg;

   if(ev->NumArgs() > 0)
   {
      if(ev->GetInteger(1))
      {
         flags |= FL_GODMODE;
      }
      else
      {
         flags &= ~FL_GODMODE;
      }
   }
   else
   {
      flags ^= FL_GODMODE;
   }

   if(ev->GetSource() == EV_FROM_CONSOLE)
   {
      if(!(flags & FL_GODMODE))
      {
         msg = "godmode OFF\n";
      }
      else
      {
         msg = "godmode ON\n";
      }

      gi.cprintf(edict, PRINT_HIGH, msg);
   }
}

void Player::Kill(Event *ev)
{
   if((level.time - respawn_time) < 5)
   {
      return;
   }

   flags &= ~FL_GODMODE;
   health = 0;
   Damage(this, this, 10, worldorigin, vec_zero, vec_zero, 0, DAMAGE_NO_PROTECTION, MOD_SUICIDE, -1, -1, 1.0f);
}

void Player::NoTargetCheat(Event *ev)
{
   const char *msg;

   flags ^= FL_NOTARGET;
   if(!(flags & FL_NOTARGET))
   {
      msg = "notarget OFF\n";
   }
   else
   {
      msg = "notarget ON\n";
   }

   gi.cprintf(edict, PRINT_HIGH, msg);
}

void Player::NoclipCheat(Event *ev)
{
   const char *msg;

   if(vehicle)
   {
      msg = "Must exit vehicle first\n";
   }
   else if(getMoveType() == MOVETYPE_NOCLIP)
   {
      setMoveType(MOVETYPE_WALK);
      msg = "noclip OFF\n";
   }
   else
   {
      setMoveType(MOVETYPE_NOCLIP);
      msg = "noclip ON\n";
   }

   gi.cprintf(edict, PRINT_HIGH, msg);
}

void Player::SpawnEntity(Event *ev)
{
   Entity         *ent;
   const char     *name;
   const ClassDef *cls;
   str             text;
   Vector          forward;
   Vector          right;
   Vector          up;
   Vector          delta;
   Vector          v;
   int             n;
   int             i;

   if(ev->NumArgs() < 1)
   {
      ev->Error("Usage: spawn entityname [keyname] [value]...");
      return;
   }

   name = ev->GetString(1);
   if(!name || !name[0])
   {
      ev->Error("Must specify an entity name");
      return;
   }

   // create a new entity
   G_InitSpawnArguments();

   if(name)
   {
      cls = getClassForID(name);
      if(!cls)
      {
         cls = getClass(name);
      }

      if(!cls)
      {
         str n;

         n = name;
         if(!strstr(n.c_str(), ".def"))
         {
            n += ".def";
         }
         G_SetSpawnArg("model", n.c_str());
      }
      else
      {
         G_SetSpawnArg("classname", name);
      }
   }

   angles.AngleVectors(&forward, &right, &up);
   v = worldorigin + (forward + up) * 80;
   text = va("%f %f %f", v[0], v[1], v[2]);
   G_SetSpawnArg("origin", text.c_str());

   delta = worldorigin - v;
   v = delta.toAngles();
   text = va("%f", v[1]);
   G_SetSpawnArg("angle", text.c_str());

   if(ev->NumArgs() > 2)
   {
      n = ev->NumArgs();
      for(i = 2; i <= n; i += 2)
      {
         G_SetSpawnArg(ev->GetString(i), ev->GetString(i + 1));
      }
   }

   cls = G_GetClassFromArgs();
   if(!cls)
   {
      ev->Error("'%s' is not a valid entity name", name);
      return;
   }

   ent = (Entity *)cls->newInstance();

   G_InitSpawnArguments();
}

void Player::SpawnActor(Event *ev)
{
   Entity   *ent;
   str      name;
   str      text;
   Vector	forward;
   Vector	right;
   Vector	up;
   Vector	delta;
   Vector	v;
   int		n;
   int		i;
   const ClassDef	*cls;

   if(ev->NumArgs() < 1)
   {
      ev->Error("Usage: actor [modelname] [keyname] [value]...");
      return;
   }

   name = ev->GetString(1);
   if(!name[0])
   {
      ev->Error("Must specify a model name");
      return;
   }

   if(!strstr(name.c_str(), ".def"))
   {
      name += ".def";
   }

   // create a new entity
   G_InitSpawnArguments();

   G_SetSpawnArg("model", name.c_str());

   angles.AngleVectors(&forward, &right, &up);
   v = worldorigin + (forward + up) * 80;
   text = va("%f %f %f", v[0], v[1], v[2]);
   G_SetSpawnArg("origin", text.c_str());

   delta = worldorigin - v;
   v = delta.toAngles();
   text = va("%f", v[1]);
   G_SetSpawnArg("angle", text.c_str());

   if(ev->NumArgs() > 2)
   {
      n = ev->NumArgs();
      for(i = 2; i <= n; i += 2)
      {
         G_SetSpawnArg(ev->GetString(i), ev->GetString(i + 1));
      }
   }

   cls = G_GetClassFromArgs();
   if(!cls)
   {
      G_SetSpawnArg("model", name.c_str());
      ent = new Actor();
   }
   else
   {
      ent = (Entity *)cls->newInstance();
   }

   G_InitSpawnArguments();
}

void Player::EventPreviousWeapon(Event *ev)
{
   if(deadflag)
   {
      return;
   }

   if(vehicle && vehicle->HasWeapon())
      return;

   if(flags & (FL_MUTANT | FL_SP_MUTANT))
      return;

   // Cycle backwards through weapons
   if(currentWeapon && currentWeapon->AttackDone() && !currentWeapon->ChangingWeapons())
   {
      ProcessEvent(EV_Player_ZoomOut);
      ChangeWeapon(PreviousWeapon(currentWeapon));
   }
}

void Player::EventNextWeapon(Event *ev)
{
   if(deadflag)
   {
      return;
   }

   if(vehicle && vehicle->HasWeapon())
      return;

   if(flags & (FL_MUTANT | FL_SP_MUTANT))
      return;

   // Cycle forwards through weapons
   if(currentWeapon && currentWeapon->AttackDone() && !currentWeapon->ChangingWeapons())
   {
      ProcessEvent(EV_Player_ZoomOut);
      ChangeWeapon(NextWeapon(currentWeapon));
   }
}

void Player::EventNextItem(Event *ev)
{
   Item *nextItem;

   if(deadflag)
      return;

   nextItem = NextItem(currentItem);

   if(nextItem)
      currentItem = (InventoryItem *)nextItem;
}

void Player::EventPreviousItem(Event *ev)
{
   Item *prevItem;

   if(deadflag)
      return;

   prevItem = PrevItem(currentItem);

   if(prevItem)
      currentItem = (InventoryItem *)prevItem;
}

void Player::GameVersion(Event *ev)
{
   gi.cprintf(edict, PRINT_HIGH, "%s : %s\n", GAMEVERSION, __DATE__);
}

void Player::Fov(Event *ev)
{
   if(ev->NumArgs() < 1)
   {
      gi.cprintf(edict, PRINT_HIGH, "Fov = %d\n", fov);
      return;
   }

   fov = ev->GetFloat(1);

   if((fov < 90) && DM_FLAG(DF_FIXED_FOV))
   {
      fov = 90;
      return;
   }

   if(fov < 1)
      fov = 90;
   else if(fov > 160)
      fov = 160;
}

void Player::SaveFov(Event *ev)
{
   savedfov = fov;
}

void Player::RestoreFov(Event *ev)
{
   if(savedfov)
      fov = savedfov;
   savedfov = 0;
}

void Player::ToggleViewMode(Event *ev)
{
   if(viewmode == CAMERA_VIEW || (vehicle && !vehicle->IsDrivable()))
   {
      return;
   }

   // Spectators switch between a camera view and noclip
   if(spectator)
   {
      spectator_distance = 128;
      if(defaultViewMode == FIRST_PERSON)
      {
         defaultViewMode = SPECTATOR;
         SetViewMode(SPECTATOR);
         CTF_UpdateNumberOfPlayers();
      }
      else
      {
         defaultViewMode = FIRST_PERSON;
         SetViewMode(FIRST_PERSON);
         CTF_UpdateNumberOfPlayers();
      }
      return;
   }

   if(defaultViewMode == FIRST_PERSON)
   {
      defaultViewMode = THIRD_PERSON;
      SetViewMode(THIRD_PERSON);
   }
   else
   {
      defaultViewMode = FIRST_PERSON;
      SetViewMode(FIRST_PERSON);
   }
}

/*
=========================================================================
Player::ToggleZoomMode - Plays a zoom sound, sets the layout to display a
crosshair and calls the FOV event for the player.
=========================================================================
*/
void Player::ToggleZoomMode(Event *ev)
{
   if(zoom_mode == ZOOMED_IN)
   {
      zoom_mode = ZOOMED_OUT;

      fov = atof(Info_ValueForKey(client->pers.userinfo, "fov"));
      if(fov < 1)
      {
         fov = 90;
      }
      else if(fov > 160)
      {
         fov = 160;
      }

      RandomGlobalSound("scope_zoomout", 1, CHAN_VOICE, ATTN_NORM);
      ProcessEvent(EV_MovementSound);
   }
   else
   {
      RandomGlobalSound("scope_zoomin", 1, CHAN_VOICE, ATTN_NORM);
      ProcessEvent(EV_MovementSound);
      zoom_mode = ZOOMED_IN;
      fov = 20;
   }
}

void Player::ZoomOut(Event *ev)
{
   if(zoom_mode == ZOOMED_IN)
   {
      zoom_mode = ZOOMED_OUT;
      fov = atof(Info_ValueForKey(client->pers.userinfo, "fov"));
      if(fov < 1)
      {
         fov = 90;
      }
      else if(fov > 160)
      {
         fov = 160;
      }
   }
}

void Player::EnterConsole(Event *ev)
{
   velocity = vec_zero;

   in_console = true;
   con_name = ev->GetString(1);

   // Create a console variable with the entnum of this player 
   // who used it.

   Director.CreateConsoleUser(con_name.c_str(), entnum);
}

void Player::ExitConsole(Event *ev)
{
   ProcessEvent(EV_KickFromConsole);
   in_console = false;
   con_name = "";
   SetCamera(NULL);

   showModel();
   hidestats = false;
}

void Player::KickConsole(Event *ev)
{
   char msg[MAX_MSGLEN];

   in_console = false;

   // Send a message to the client to kick us out of this console.
   if(!consoleManager.ConsoleExists(con_name.c_str()))
   {
      Event event = new Event(ev);
      PostEvent(event, 1.0f);
      return;
   }

   snprintf(msg, sizeof(msg), "pku %s", con_name.c_str());
   gi.WriteByte(svc_console_command);
   gi.WriteString(msg);
   gi.unicast(edict, true);
}

/*
===============
CalcRoll

===============
*/
float Player::CalcRoll(void)
{
   float	sign;
   float	side;
   float	value;
   Vector r;

   angles.AngleVectors(NULL, &r, NULL);
   side = velocity * r;
   sign = side < 0 ? -4 : 4;
   side = fabs(side);

   value = sv_rollangle->value;

   if(side < sv_rollspeed->value)
   {
      side = side * value / sv_rollspeed->value;
   }
   else
   {
      side = value;
   }

   return side * sign;
}

/*
===============
CalcViewOffset

Auto pitching on slopes?

  fall from 128: 400 = 160000
  fall from 256: 580 = 336400
  fall from 384: 720 = 518400
  fall from 512: 800 = 640000
  fall from 640: 960 =

  damage = deltavelocity*deltavelocity  * 0.0001

===============
*/
void Player::CalcViewOffset(void)
{
   float		bob;
   float		ratio;
   float		pitch;
   float		roll;
   float		delta;
   Vector	forward;
   Vector	right;
   Vector	ang;

   // Initialize kick for this frame
   v_kick = vec_zero;

   // don't add any kick when dead
   if(!deadflag)
   {
      // add angles based on weapon kick
      v_kick = kick_angles;

      // add angles based on damage kick
      ratio = (v_dmg_time - level.time) / DAMAGE_TIME;
      if(ratio < 0)
      {
         ratio = 0;
         v_dmg_pitch = 0;
         v_dmg_roll = 0;
      }

      pitch = v_kick.pitch() + ratio * v_dmg_pitch;
      roll = v_kick.roll() + ratio * v_dmg_roll;

      // add pitch based on fall kick

      ratio = (fall_time - level.time) / FALL_TIME;
      if(ratio < 0)
      {
         ratio = 0;
      }
      pitch += ratio * fall_value;

      // calculate forward and right based upon yaw
      ang[1] = v_angle[1];
      ang.AngleVectors(&forward, &right, NULL);

      //FIXME: need to handle alternate gravity axis
      // add pitch based on forward velocity
      delta = velocity * forward;
      pitch += delta * run_pitch->value;

      // add roll based on side velocity
      delta = -(velocity * right);
      roll += delta * run_pitch->value;

      // add angles based on bob
      delta = bobfracsin * bob_pitch->value * xyspeed;
      if((waterlevel <= 1) && (client->ps.pmove.pm_flags & PMF_DUCKED))
      {
         delta *= 6;		// crouching
      }
      pitch += delta;
      delta = bobfracsin * bob_roll->value * xyspeed;
      if((waterlevel <= 1) && (client->ps.pmove.pm_flags & PMF_DUCKED))
      {
         delta *= 6;		// crouching
      }
      if(bobcycle & 1)
      {
         delta = -delta;
      }
      roll += delta;

      v_kick.setPitch(pitch);
      v_kick.setRoll(roll);
   }

   // add fall height
   ratio = (fall_time - level.time) / FALL_TIME;
   if(ratio < 0)
   {
      ratio = 0;
   }

   v_offset = Vector(0, 0, viewheight - ratio * fall_value * 0.4);

   // add bob height
   bob = bobfracsin * xyspeed * bob_up->value;
   if(bob > 6)
   {
      bob = 6;
   }

   //gi.DebugGraph (bob *2, 255);
   v_offset.z += bob;

   // add kick offset
   v_offset += kick_origin;

   // absolutely bound offsets
   // so the view can never be outside the player box
   if(v_offset[0] < -14)
   {
      v_offset[0] = -14;
   }
   else if(v_offset[0] > 14)
   {
      v_offset[0] = 14;
   }

   if(v_offset[1] < -14)
   {
      v_offset[1] = -14;
   }
   else if(v_offset[1] > 14)
   {
      v_offset[1] = 14;
   }

   if(v_offset[2] < 0)
   {
      v_offset[2] = 0;
   }
   else if(v_offset[2] > STAND_HEIGHT - 2)
   {
      v_offset[2] = STAND_HEIGHT - 2;
   }

   // clear weapon kicks
   kick_origin = vec_zero;
   kick_angles = vec_zero;
}

/*
==============
CalcGunOffset
==============
*/
void Player::CalcGunOffset(void)
{
   int	i;
   float	delta;

   // gun angles from bobbing
   v_gunangles[ROLL] = xyspeed * bobfracsin * 0.005;
   v_gunangles[YAW] = xyspeed * bobfracsin * 0.01;
   if(bobcycle & 1)
   {
      v_gunangles[ROLL] = -v_gunangles[ROLL];
      v_gunangles[YAW] = -v_gunangles[YAW];
   }

   v_gunangles[PITCH] = xyspeed * bobfracsin * 0.005;

   // gun angles from delta movement
   for(i = 0; i < 3; i++)
   {
      delta = oldviewangles[i] - v_angle[i];
      if(delta > 180)
      {
         delta -= 360;
      }
      if(delta < -180)
      {
         delta += 360;
      }
      if(delta > 45)
      {
         delta = 45;
      }
      if(delta < -45)
      {
         delta = -45;
      }
      if(i == YAW)
      {
         v_gunangles[ROLL] += 0.1 * delta;
      }
      v_gunangles[i] += 0.2 * delta;
   }

   oldviewangles = v_angle;

   // gun height
   VectorClear(v_gunoffset);

   // gun_x / gun_y / gun_z are development tools
   for(i = 0; i < 3; i++)
   {
      v_gunoffset[i] += orientation[0][i] * gun_y->value;
      v_gunoffset[i] += orientation[1][i] * gun_x->value;
      v_gunoffset[i] += orientation[2][i] * -gun_z->value;
   }
   //
   // put gun offset and angles into client playerstate
   //
   client->ps.gunoffset[0] = v_gunoffset[0];
   client->ps.gunoffset[1] = v_gunoffset[1];
   client->ps.gunoffset[2] = v_gunoffset[2];

   client->ps.gunangles[0] = v_gunangles[0];
   client->ps.gunangles[1] = v_gunangles[1];
   client->ps.gunangles[2] = v_gunangles[2];

   if(currentWeapon)
   {
      currentWeapon->SetGravityAxis(gravaxis);
   }
}

void Player::GravityNodes(void)
{
   Vector grav, gravnorm, velnorm;
   float dot;
   qboolean force;

   //
   // Check for gravity pulling points
   //

   // no pull on ladders or during waterjumps
   if(onladder || (client->ps.pmove.pm_flags & PMF_TIME_WATERJUMP))
   {
      return;
   }

   grav = gravPathManager.CalculateGravityPull(*this, worldorigin, &force);

   // Check for unfightable gravity.
   if(force && grav != vec_zero)
   {
      velnorm = velocity;
      velnorm.normalize();

      gravnorm = grav;
      gravnorm.normalize();

      dot = gravnorm.x * velnorm.x + gravnorm.y * velnorm.y + gravnorm.z * velnorm.z;

      // This prevents the player from trying to swim upstream
      if(dot < 0)
      {
         float tempdot;
         Vector tempvec;

         tempdot = 0.2f - dot;
         tempvec = velocity * tempdot;
         velocity = velocity - tempvec;
      }
   }

#if 0
   OutputDebugString(va("DOT : %f\n", dot));
   OutputDebugString(va("GRAV: %f %f %f\n", grav.x, grav.y, grav.z));
   OutputDebugString(va("VELN: %f %f %f\n", velnorm.x, velnorm.y, velnorm.z));
   OutputDebugString(va("GRVN: %f %f %f\n", gravnorm.x, gravnorm.y, gravnorm.z));
#endif
   velocity = velocity + grav;
}

/*
=============
CheckWater
=============
*/
void Player::CheckWater(void)
{
   Vector  point;
   int	  cont;
   int	  sample1;
   int	  sample2;
   const gravityaxis_t &grav = gravity_axis[gravaxis];

   unlink();
   if(vehicle)
   {
      vehicle->unlink();
   }

   //
   // get waterlevel, accounting for ducking
   //
   waterlevel = 0;
   watertype = 0;

   sample2 = viewheight - mins[grav.z];
   sample1 = sample2 / 2;

   point[grav.x] = worldorigin[grav.x];
   point[grav.y] = worldorigin[grav.y];
   point[grav.z] = worldorigin[grav.z] + mins[grav.z] + grav.sign;
   cont = gi.pointcontents(point.vec3());

   if(cont & MASK_WATER)
   {
      watertype = cont;
      waterlevel = 1;
      point[grav.z] = worldorigin[grav.z] + mins[grav.z] + sample1;
      cont = gi.pointcontents(point.vec3());
      if(cont & MASK_WATER)
      {
         waterlevel = 2;
         point[grav.z] = worldorigin[grav.z] + mins[grav.z] + sample2;
         cont = gi.pointcontents(point.vec3());
         if(cont & MASK_WATER)
         {
            waterlevel = 3;
         }
      }
   }

   link();
   if(vehicle)
   {
      vehicle->link();
   }
}

/*
=============
WorldEffects
=============
*/
void Player::WorldEffects(void)
{
   if(movetype == MOVETYPE_NOCLIP)
   {
      // don't need air
      air_finished = level.time + 20;
      return;
   }

   //
   // Check for earthquakes
   //
   if(groundentity && (level.earthquake > level.time))
   {
      velocity += Vector(EARTHQUAKE_STRENGTH*G_CRandom(), EARTHQUAKE_STRENGTH*G_CRandom(), fabs(150 * G_CRandom()));
   }

   //
   // if just entered a water volume, play a sound
   //
   if(!old_waterlevel && waterlevel)
   {
      if(watertype & CONTENTS_LAVA)
      {
         RandomSound("snd_burn", 1, CHAN_BODY, ATTN_NORM);
         ProcessEvent(EV_MovementSound);
      }
      else if((watertype & CONTENTS_WATER) && waterlevel < 3)
      {
         RandomGlobalSound("impact_playersplash", 1, CHAN_BODY, ATTN_NORM);
         ProcessEvent(EV_MovementSound);
      }

      flags |= FL_INWATER;
   }

   //
   // if just completely exited a water volume, play a sound
   //
   if(old_waterlevel && !waterlevel)
   {
      RandomGlobalSound("impact_playerleavewater", 1, CHAN_BODY, ATTN_NORM);
      ProcessEvent(EV_MovementSound);
      flags &= ~FL_INWATER;
   }

   //
   // check for head just going under water
   //
   if(old_waterlevel && old_waterlevel != 3 && waterlevel == 3)
   {
      RandomGlobalSound("impact_playersubmerge", 1, CHAN_BODY, ATTN_NORM);
      ProcessEvent(EV_MovementSound);
   }

   //
   // check for head just coming out of water
   //
   if(old_waterlevel == 3 && waterlevel != 3)
   {
      if(air_finished < level.time)
      {
         // gasp for air
         RandomSound("snd_gasp", 1, CHAN_VOICE, ATTN_NORM);
         ProcessEvent(EV_VoiceSound);
      }
      else if(air_finished < level.time + 11)
      {
         // just break surface
         RandomSound("snd_gasp", 1, CHAN_VOICE, ATTN_NORM);
         ProcessEvent(EV_VoiceSound);
      }
   }

   //
   // check for lava
   //
   if(watertype & CONTENTS_LAVA)
   {
      if(next_drown_time < level.time)
      {
         next_drown_time = level.time + 0.2f;
         Damage(world, world, 10 * waterlevel, worldorigin, vec_zero, vec_zero, 0, DAMAGE_NO_ARMOR, MOD_LAVA, -1, -1, 1.0f);
      }
   }

   //
   // check for drowning
   //
   if(waterlevel == 3)
   {
      // if out of air, start drowning
      if((air_finished < level.time) && !(flags & FL_OXYGEN) && !(flags & FL_GODMODE))
      {
         // drown!
         if(next_drown_time < level.time && health > 0)
         {
            next_drown_time = level.time + 1;

            // take more damage the longer underwater
            drown_damage += 2;
            if(drown_damage > 15)
            {
               drown_damage = 15;
            }

            // play a gurp sound instead of a normal pain sound
            if(health <= drown_damage)
            {
               RandomSound("snd_drown", 1, CHAN_VOICE, ATTN_NORM);
               ProcessEvent(EV_PainSound);
            }
            else if(rand() & 1)
            {
               RandomSound("snd_choke", 1, CHAN_VOICE, ATTN_NORM);
               ProcessEvent(EV_PainSound);
            }
            else
            {
               RandomSound("snd_choke", 1, CHAN_VOICE, ATTN_NORM);
               ProcessEvent(EV_PainSound);
            }

            Damage(world, world, drown_damage, worldorigin, vec_zero, vec_zero, 0, DAMAGE_NO_ARMOR, MOD_DROWN, -1, -1, 1.0f);
         }
      }
   }
   else
   {
      air_finished = level.time + 20;
      drown_damage = 2;
   }

   GravityNodes();

   // Apply grappling hook
   if(grapple_pull)
   {
      Vector   v, dir;
      int      length;

      v = worldorigin;
      v[2] += viewheight;

      v = centroid;

      dir = grapple_org - v;

      length = dir.length();

      if(length > 48)
      {
         dir.normalize();
         velocity = dir * grapple_speed;
      }
      else
      {
         velocity = vec_zero;
         gravity = 0;
         hook->StopEntitySound(NULL);
      }
   }

   old_waterlevel = waterlevel;
}

/*
=============
AddBlend
=============
*/
void Player::AddBlend(float r, float g, float b, float a)
{
   float	a2;
   float a3;

   if(a <= 0)
   {
      return;
   }

   // new total alpha
   a2 = blend[3] + (1 - blend[3]) * a;

   // fraction of color from old
   a3 = blend[3] / a2;

   blend[0] = blend[0] * a3 + r * (1 - a3);
   blend[1] = blend[1] * a3 + g * (1 - a3);
   blend[2] = blend[2] * a3 + b * (1 - a3);
   blend[3] = a2;
}

/*
=============
CalcBlend
=============
*/
void Player::CalcBlend(void)
{
   int		contents;
   Vector	vieworg;
   const gravityaxis_t &grav = gravity_axis[gravaxis];

   blend[0] = blend[1] = blend[2] = blend[3] = 0;

   // add for contents
   vieworg[grav.x] = worldorigin[grav.x] + v_offset[0];
   vieworg[grav.y] = worldorigin[grav.y] + v_offset[1] * grav.sign;
   vieworg[grav.z] = worldorigin[grav.z] + v_offset[2] * grav.sign;

   contents = gi.pointcontents(vieworg.vec3());

   if(contents & CONTENTS_SOLID)
   {
      // Outside of world
      AddBlend(0.8, 0.5, 0.0, 0.2);
   }
   else if(contents & CONTENTS_LAVA)
   {
      AddBlend(level.lava_color[0], level.lava_color[1], level.lava_color[2], level.lava_alpha);
   }
   else if(contents & CONTENTS_WATER)
   {
      AddBlend(level.water_color[0], level.water_color[1], level.water_color[2], level.water_alpha);
   }

   if(contents & CONTENTS_LIGHTVOLUME)
   {
      AddBlend(level.lightvolume_color[0], level.lightvolume_color[1], level.lightvolume_color[2], level.lightvolume_alpha);
   }

   // Flashes
   AddBlend(flash_color[0], flash_color[1], flash_color[2], flash_color[3]);

   // add for zoom
   if(zoom_mode == ZOOMED_IN)
   {
      AddBlend(0.4, 1, 0.1, 0.1);
   }

   // add for damage
   if(damage_alpha > 0)
   {
      AddBlend(damage_blend[0], damage_blend[1], damage_blend[2], damage_alpha);
   }

   if(flags & FL_ADRENALINE)
   {
      AddBlend(1.0f, 1.0f, 0.0f, 0.2f);
   }

   // drop the damage value
   damage_alpha -= 0.06;
   if(damage_alpha < 0)
   {
      damage_alpha = 0;
   }

   // Drop the flash
   flash_color[3] -= 0.06;
   if(flash_color[3] < 0)
   {
      flash_color[3] = 0;
   }
}

/*
===============
P_DamageFeedback

Handles color blends and view kicks
===============
*/
void Player::DamageFeedback(void)
{
   float		realcount;
   float		count;

   // flash the backgrounds behind the status numbers
   client->ps.stats[STAT_FLASHES] = 0;

   if(damage_blood)
   {
      client->ps.stats[STAT_FLASHES] |= 1;
   }

   // total points of damage shot at the player this frame
   if(!damage_blood)
   {
      // didn't take any damage
      return;
   }

   count = damage_blood;
   realcount = count;
   if(count < 10)
   {
      // always make a visible effect
      count = 10;
   }

   // the total alpha of the blend is always proportional to count
   if(damage_alpha < 0)
   {
      damage_alpha = 0;
   }

   damage_alpha += count * 0.01;
   if(damage_alpha < 0.2)
   {
      damage_alpha = 0.2;
   }
   if(damage_alpha > 0.6)
   {
      // don't go too saturated
      damage_alpha = 0.6;
   }

   // the color of the blend will vary based on how much was absorbed
   // by different armors
   damage_blend = vec_zero;
   if(damage_blood)
   {
      damage_blend += (damage_blood / realcount) * bcolor;
   }

   //
   // clear totals
   //
   damage_blood = 0;
}

EXPORT_FROM_DLL const char *Player::AnimPrefixForWeapon(void)
{
   if(currentWeapon)
   {
      switch(currentWeapon->GetType())
      {
      case WEAPON_MELEE:
         return "";
         break;
      case WEAPON_1HANDED:
         return "1hand_";
         break;
      case WEAPON_2HANDED_HI:
         return "hi2hand_";
         break;
      case WEAPON_2HANDED_LO:
         return "lo2hand_";
         break;
      default:
         return "";
         break;
      }
   }
   else
   {
      return "";
   }
   return "";
}

EXPORT_FROM_DLL void Player::ChooseAnim(void)
{
   str prefix;
   str aname;

   if(deadflag)
   {
      return;
   }

   if(vehicle)
   {
      falling = false;
      animOverride = false;
      SetAnim(vehicleanim.c_str());
      return;
   }

   if(onladder)
   {
      if(falling)
      {
         Event * ev;

         ev = new Event(EV_Sentient_AnimLoop);
         ProcessEvent(ev);
         falling = false;
      }
      if(fabs(velocity[2]) > 0)
      {
         SetAnim("climb");
      }
      else if(!animOverride)
      {
         StopAnimating();
      }
      return;
   }

   if(client->ps.pmove.pm_flags & PMF_DUCKED)
   {
      if(!(old_pmove.pm_flags & PMF_DUCKED))
      {
         SetAnim("crouch");
         return;
      }
   }
   else if(old_pmove.pm_flags & PMF_DUCKED)
   {
      SetAnim("uncrouch");
      return;
   }
   //
   // put the appropriate prefix on the animation
   prefix = AnimPrefixForPlayer();

   if(currentWeapon)
   {
      if(currentWeapon->WeaponRaising())
      {
         aname = prefix + str("readyweapon");
         SetAnim(aname.c_str());
         return;
      }
      else if(currentWeapon->WeaponPuttingAway())
      {
         aname = prefix + str("putaway");
         SetAnim(aname.c_str());
         return;
      }
      else if(currentWeapon->Reloading())
      {
         aname = prefix + str("reload");
         SetAnim(aname.c_str());
         return;
      }
   }
   //
   // append the prefix based on which weapon we are holding
   //
   prefix += str(AnimPrefixForWeapon());

   if(waterlevel > 2 || (!groundentity && waterlevel))
   {
      if(xyspeed > 20)
      {
         aname = prefix + str("run");
      }
      else
      {
         aname = prefix + str("idle");
      }
   }
   else
   {
      if(xyspeed > 250)
      {
         aname = prefix + str("run");
      }
      else if(xyspeed > 20)
      {
         if(client->ps.pmove.pm_flags & PMF_DUCKED)
         {
            aname = prefix + str("run");
         }
         else
         {
            aname = prefix + str("walk");
         }
      }
      else
      {
         aname = prefix + str("idle");
      }
   }

   if(grapple_pull)
   {
      aname = "fall";
   }

   // Set the anim for the flag
   if(ctf->value)
   {
      CTF_Flag *flag;

      if((flag = (CTF_Flag *)HasItemOfSuperclass("CTF_Flag")))
      {
         if(xyspeed > 250)
         {
            if(flag->limp)
            {
               flag->RandomAnimate("idle", NULL);
               flag->limp = false;
            }
         }
         else
         {
            if(!flag->limp)
            {
               flag->RandomAnimate("limp", NULL);
               flag->limp = true;
            }
         }
      }
   }

   SetAnim(aname.c_str());
}

void Player::SetCameraValues(Vector position, Vector cameraoffset, Vector ang, Vector camerakick, Vector vel, float camerablend[4], float camerafov)
{
   client->ps.viewangles[0] = ang[0];
   client->ps.viewangles[1] = ang[1];
   client->ps.viewangles[2] = ang[2];

   client->ps.viewoffset[0] = cameraoffset[0];
   client->ps.viewoffset[1] = cameraoffset[1];
   client->ps.viewoffset[2] = cameraoffset[2];

   client->ps.pmove.origin[0] = position[0] * 8;
   client->ps.pmove.origin[1] = position[1] * 8;
   client->ps.pmove.origin[2] = position[2] * 8;

   client->ps.pmove.velocity[0] = vel[0] * 8.0;
   client->ps.pmove.velocity[1] = vel[1] * 8.0;
   client->ps.pmove.velocity[2] = vel[2] * 8.0;

   client->ps.blend[0] = camerablend[0];
   client->ps.blend[1] = camerablend[1];
   client->ps.blend[2] = camerablend[2];
   client->ps.blend[3] = camerablend[3];

   client->ps.fov = camerafov;

   client->ps.kick_angles[0] = camerakick[0];
   client->ps.kick_angles[1] = camerakick[1];
   client->ps.kick_angles[2] = camerakick[2];
}

void Player::SetCameraEntity(Entity *cameraEnt)
{
   assert(cameraEnt);

   // In release, we should never be without a camera
   if(!cameraEnt)
   {
      cameraEnt = this;
   }

   // should we see the player's body?
   if(cameraEnt == this)
   {
      edict->s.renderfx |= RF_VIEWERMODEL;
      if(currentWeapon)
      {
         if(!vehicle || vehicle->ShowWeapon())
         {
            currentWeapon->edict->s.renderfx &= ~RF_DONTDRAW;
            currentWeapon->edict->s.renderfx |= RF_VIEWERMODEL;
         }
         else
         {
            currentWeapon->edict->s.renderfx |= RF_DONTDRAW;
         }
      }
      if(!ctf->value && vehicle && vehicle->IsDrivable())
      {
         SetCameraValues(worldorigin, v_offset, v_angle, v_kick, velocity, blend, fov*1.25);
      }
      else
      {
         SetCameraValues(worldorigin, v_offset, v_angle, v_kick, velocity, blend, fov);
      }
   }
   else
   {
      float noblend[4];
      float camerafov;
      Vector pos;

      /*
            if ( vehicle && cameraEnt == thirdpersonCamera )
               {
               Event * event;

               event = new Event( EV_Camera_SetDistance );
               event->AddFloat( 256 );
               cameraEnt->ProcessEvent( event );
               }
      */

      edict->s.renderfx &= ~RF_VIEWERMODEL;

      if(currentWeapon)
      {
         // Take the weapon out of the view
         currentWeapon->edict->s.renderfx &= ~RF_VIEWERMODEL;
      }

      if(in_console)
      {
         // Make the client act like a viewmodel so it doesn't block
         // the camera to the console.
         edict->s.renderfx |= RF_VIEWERMODEL;
      }

      // FIXME
      // should add all these fields to entity or camera or whatever.
      noblend[0] = 0;
      noblend[1] = 0;
      noblend[2] = 0;
      noblend[3] = 0;
      pos = vec_zero;

      camerafov = 90;
      if(cameraEnt->isSubclassOf<Mine>())
      {
         Vector dir;
         vec3_t mat[3];

         camerafov = ((Projectile *)cameraEnt)->fov;

         AnglesToMat(cameraEnt->worldangles.vec3(), mat);
         dir = mat[0];
         if(cameraEnt->velocity == vec_zero)
         {
            pos = dir * 30;
         }
         else
         {
            pos = pos + Vector(0, 0, 1) * 30;
         }
      }
      else if(cameraEnt->isSubclassOf<Camera>())
      {
         camerafov = ((Camera *)cameraEnt)->fov;
         if(cameraEnt->isSubclassOf<SecurityCamera>())
            cameraEnt->edict->s.renderfx |= RF_DONTDRAW;
      }
      else if(trappedInQuantum)
      {
         Sentient *sent;

         if(cameraEnt->isSubclassOf<Sentient>())
         {
            Vector dir;

            sent = (Sentient *)cameraEnt;
            sent->GetMuzzlePositionAndDirection(&pos, &dir);
         }
      }
      else if(cameraEnt->isSubclassOf<Sentient>())
      {
         pos = ((Sentient *)cameraEnt)->EyePosition() - cameraEnt->worldorigin;
      }

      SetCameraValues(cameraEnt->worldorigin, pos, cameraEnt->worldangles, vec_zero,
                      cameraEnt->velocity, noblend, camerafov);
      //SetCameraValues( "0 0 64", pos, "0 270 0", vec_zero,
      //	cameraEnt->velocity, noblend, camerafov );
   }

   if(flags & FL_SHIELDS)
   {
      edict->s.renderfx |= RF_ENVMAPPED;
   }
   else
      edict->s.renderfx &= ~RF_ENVMAPPED;

   if(flags & FL_CLOAK)
   {
      setAlpha(0);
   }
   else
      setAlpha(1.0f);

   if(currentWeapon)
   {
      if(flags & FL_SHIELDS)
         currentWeapon->edict->s.renderfx |= RF_ENVMAPPED;
      else
         currentWeapon->edict->s.renderfx &= ~RF_ENVMAPPED;

      if(flags & FL_CLOAK)
      {
         currentWeapon->edict->s.alpha = 0.1f;
         currentWeapon->edict->s.renderfx |= RF_TRANSLUCENT;
      }
      else
      {
         currentWeapon->edict->s.alpha = 1.0f;
         currentWeapon->edict->s.renderfx &= ~RF_TRANSLUCENT;
      }
      if(cameraEnt != this)
      {
         if(edict->s.renderfx & RF_DONTDRAW)
         {
            currentWeapon->edict->s.renderfx |= RF_DONTDRAW;
         }
         else
         {
            currentWeapon->edict->s.renderfx &= ~RF_DONTDRAW;
         }
      }
   }

   if((viewmode == THIRD_PERSON) && crosshair && currentWeapon)
   {
      Vector dir, src, end;
      trace_t trace;

      currentWeapon->GetMuzzlePosition(&src, &dir);

      end = src + dir * 8192;
      trace = G_FullTrace(src, vec_zero, vec_zero, end, 5, this, MASK_SHOT, "Player::SetCameraEntity");
      if(trace.intersect.valid)
      {
         crosshair->edict->s.frame = 1;
      }
      else
      {
         crosshair->edict->s.frame = 0;
      }

      crosshair->setOrigin(trace.endpos - dir * 16);
      crosshair->edict->s.scale = (crosshair->worldorigin - worldorigin).length() / 256;
   }
}

EXPORT_FROM_DLL void Player::CalcBob(void)
{
   float		time;
   Vector	hvel;
   const gravityaxis_t &grav = gravity_axis[gravaxis];
   //
   // calculate speed and cycle to be used for
   // all cyclic walking effects
   //
   hvel[grav.x] = velocity[grav.x];
   hvel[grav.y] = velocity[grav.y];
   hvel[grav.z] = 0;
   xyspeed = hvel.length();

   if(xyspeed < 5)
   {
      // start at beginning of cycle again
      bobmove = 0;
      bobtime = 0;
   }
   else if(groundentity)
   {
      // so bobbing only cycles when on ground
      if(xyspeed > 210)
      {
         bobmove = 0.25;
      }
      else if(xyspeed > 100)
      {
         bobmove = 0.125;
      }
      else
      {
         bobmove = 0.0625;
      }
   }

   bobtime += bobmove;
   time = bobtime;
   if(client->ps.pmove.pm_flags & PMF_DUCKED)
   {
      time *= 4;
   }

   bobcycle = (int)time;
   bobfracsin = fabs(sin(time * M_PI));
}

/*
=================
P_FallingDamage
=================
*/
void Player::FallingDamage(void)
{
   float		delta;
   float		damage;
   csurface_t *surf;
   int		z;

   z = gravity_axis[gravaxis].z;

   if(deadflag)
   {
      return;
   }

   if(flags & (FL_MUTANT | FL_SP_MUTANT | FL_ADRENALINE) || grapple_pull || (grapple_time > level.time))
   {
      return;
   }

   if(getMoveType() == MOVETYPE_NOCLIP)
   {
      return;
   }
   if(!fallsurface)
   {
      return;
   }
   surf = fallsurface;
   fallsurface = NULL;
   delta = velocity[z] - oldvelocity[z];

   delta = delta * delta * 0.0001;

   // never take falling damage if completely underwater
   switch(waterlevel)
   {
   case 1:
      delta *= 0.5;
      break;

   case 2:
      delta *= 0.25;
      break;

   case 3:
      return;
      break;
   }

   if(delta < 3)
   {
      return;
   }

   fall_value = delta * 0.5;
   if(fall_value > 40)
   {
      fall_value = 40;
   }

   fall_time = level.time + FALL_TIME;

   if(delta > 30)
   {
      damage = (delta - 30) / 2;

      if(surf)
      {
         // modify damage based on surface
         switch(surf->flags & MASK_SURF_TYPE)
         {
            // Can this happen?
         case SURF_TYPE_WATER:
            break;

            // no damage
         case SURF_TYPE_FABRIC:
            damage *= 0;
            break;

            // low damage
         case SURF_TYPE_FLESH:
         case SURF_TYPE_VEGETATION:
            damage *= 0.5;
            break;

            // medium damage
         case SURF_TYPE_DIRT:
            damage *= 0.6;
            break;

         case SURF_TYPE_WOOD:
            damage *= 0.8;
            break;

            // full damage
         case SURF_TYPE_GRAVEL:
         case SURF_TYPE_GRILL:
         case SURF_TYPE_METAL:
         case SURF_TYPE_STONE:
         case SURF_TYPE_CONCRETE:
         case SURF_TYPE_DUCT:
         default:
            break;
         }
      }

      if(surf &&  surf->frequency != 0)
      {
         damage = 0;
      }

      if((damage >= 1) && (!DM_FLAG(DF_NO_FALLING)))
      {
         Damage(world, world, (int)damage, worldorigin, vec_zero, vec_zero, 0, DAMAGE_NO_ARMOR, MOD_FALLING, -1, -1, 1.0f);
      }
   }
   if(surf &&  surf->frequency != 0)
   {
      fall_time = 0;
      fall_value = 0;
   }
}

EXPORT_FROM_DLL void Player::FinishMove(void)
{
   Vector t[3];
   Vector forward;
   Vector right;
   Vector up;

   if(ai_createnodes->value)
   {
      AddPathNodes();
   }

   //
   // If the origin or velocity have changed since ClientThink(),
   // update the pmove values.  This will happen when the client
   // is pushed by a bmodel or kicked by an explosion.
   // 
   // If it wasn't updated here, the view position would lag a frame
   // behind the body position when pushed -- "sinking into plats"
   //
   if(
      (GetMovePlayerMoveType() != PM_LOCKVIEW) &&
      (GetMovePlayerMoveType() != PM_FREEZE)
      )
   {
      client->ps.pmove.origin[0] = worldorigin.x * 8.0;
      client->ps.pmove.origin[1] = worldorigin.y * 8.0;
      client->ps.pmove.origin[2] = worldorigin.z * 8.0;
      client->ps.pmove.velocity[0] = velocity.x * 8.0;
      client->ps.pmove.velocity[1] = velocity.y * 8.0;
      client->ps.pmove.velocity[2] = velocity.z * 8.0;
   }

   //
   // set model angles from view angles so other things in
   // the world can tell which direction you are looking
   //
   if(vehicle)
   {
      angles[PITCH] = vehicle->SetDriverPitch(v_angle[PITCH]);
   }
   else
   {
      if(waterlevel >= 3)
      {
         angles[PITCH] = v_angle[PITCH];
      }
      else if(v_angle[PITCH] > 180)
      {
         angles[PITCH] = (-360 + v_angle[PITCH]) / 12;
      }
      else
      {
         angles[PITCH] = v_angle[PITCH] / 12;
      }
   }

   angles[YAW] = v_angle[YAW];
   angles[ROLL] = CalcRoll();

   // Orient the angles to coincide with our special gravity vector
   const gravityaxis_t &grav = gravity_axis[gravaxis];

   angles.AngleVectors(&t[0], &t[1], &t[2]);
   forward[grav.x] = t[0][0];
   forward[grav.y] = t[0][1] * grav.sign;
   forward[grav.z] = t[0][2] * grav.sign;
   right[grav.x]   = t[1][0];
   right[grav.y]   = t[1][1] * grav.sign;
   right[grav.z]   = t[1][2] * grav.sign;
   up[grav.x]      = t[2][0];
   up[grav.y]      = t[2][1] * grav.sign;
   up[grav.z]      = t[2][2] * grav.sign;
   VectorsToEulerAngles(forward.vec3(), right.vec3(), up.vec3(), angles.vec3());

   if(!level.playerfrozen)
   {
      setAngles(angles);
      AnglesToMat(v_angle.vec3(), orientation);
   }

   CalcBob();

   // check if we're over the limit health-wise
   if(((int)health > (int)max_health) && ((float)((int)level.time) == level.time))
   {
      // CTF: Regeneration Tech doesn't drain health over 100
      if(!FindItem("CTF_Tech_Regeneration"))
      {
         health -= 1;
      }
   }

   // mutant mode
   if((flags & FL_SP_MUTANT) && !(flags & FL_GODMODE) && ((float)((int)level.time) == level.time))
   {
      Damage(world, world, 1, worldorigin, vec_zero, vec_zero, 0, DAMAGE_NO_ARMOR, MOD_MUTANT_DRAIN, -1, -1, 1.0f);
   }

   // Check for silencer
   if(!(flags & FL_SILENCER) && FindItem("Silencer"))
   {
      flags |= FL_SILENCER;
   }

   // Check for O2
   if(!(flags & FL_OXYGEN) && FindItem("ScubaGear"))
   {
      flags |= FL_OXYGEN;
   }

   // burn from lava, etc
   WorldEffects();
   FallingDamage();

   ChooseAnim();

   // determine the view offsets
   CalcViewOffset();
   CalcGunOffset();
   DamageFeedback();
   CalcBlend();

   oldvelocity = velocity;
}

EXPORT_FROM_DLL qboolean Player::CanMoveTo(Vector pos)
{
   trace_t	trace;
   Vector	start;
   Vector	end;
   Vector	s;

   s = Vector(0, 0, 20);
   start = worldorigin + s;
   end = pos + s;
   trace = G_Trace(start, mins, maxs, end, this, MASK_SOLID, "Player::CanMoveTo");
   if(trace.fraction == 1)
   {
      return true;
   }
   return false;
}

EXPORT_FROM_DLL qboolean Player::ClearPathTo(Vector pos)
{
   Vector	dir;
   Vector	midpos;
   Vector	end_trace;
   Vector	delta;
   Vector	start;
   Vector	min;
   Vector	max;
   trace_t	trace;
   float		dist;
   float		t;

   min   = Vector(-16, -16, 12);
   max   = Vector(16, 16, 40);
   start = worldorigin;
   dir   = pos - worldorigin;

   delta = dir;
   delta[2] = 0;
   if(delta.length() >= PATHMAP_CELLSIZE)
   {
      return false;
   }

   if(!CanMoveTo(pos))
   {
      return false;
   }

   // only do a full test when we train a level
   if(!ai_createnodes->value)
   {
      midpos = start + dir * 0.5;
      end_trace = midpos - Vector(0, 0, 40);

      // check that the midpos is onground (down 28 units)
      trace = G_Trace(midpos, min, max, end_trace, this, MASK_SOLID, "Player::ClearPathTo 1");
      if(trace.fraction == 1)
      {
         return false;
      }

      return true;
   }

   dist = dir.length();
   if(dist < 32)
   {
      return true;
   }

   dir[2] = 0;
   dist = dir.length();
   dir.normalize();

   min = Vector(-16, -16, 0);
   // check the entire move
   midpos = start;
   for(t = 0; t < dist; t += 8)
   {
      midpos[0] = start[0] + t * dir[0];
      midpos[1] = start[1] + t * dir[1];
      midpos[2] += 18;
      end_trace = midpos - Vector(0, 0, 2048);//140 );//128 );

      trace = G_Trace(midpos, min, max, end_trace, this, MASK_SOLID, "Player::ClearPathTo 2");
      if((trace.fraction == 1) || (trace.startsolid))
      {
         return false;
      }

      midpos = trace.endpos;
   }

   // Check if we're close enough
   delta = midpos - pos;
   return (delta.length() < 32);
}

EXPORT_FROM_DLL void Player::AddPathNode(Event *ev)
{
   if(ai_createnodes->value)
   {
      lastNode = new PathNode();
      lastNode->Setup(worldorigin);
   }
}

EXPORT_FROM_DLL void Player::AddPathNodes(void)
{
   StandardMovePath find;

   if(ai_createnodes->value && groundentity)
   {
      if(!lastNode || !lastNode->CheckMove(worldorigin, mins, maxs) ||
         !ClearPathTo(lastNode->worldorigin))
      {
         gi.dprintf("Can't see lastNode\n");
         lastNode = new PathNode();
         lastNode->Setup(lastVisible);

         // so that we don't just keep putting nodes in the same place
         lastVisible = worldorigin;
      }

      nearestNode = PathManager.NearestNode(worldorigin);
      if(!nearestNode)
      {
         gi.dprintf("Can't see NearestNode\n");
         lastNode = new PathNode();
         lastNode->Setup(worldorigin);
         lastVisible = worldorigin;
      }
      else if(lastNode && (!lastNode->CheckPath(nearestNode, mins, maxs)
                           || !nearestNode->CheckPath(lastNode, mins, maxs)))
      {
         gi.dprintf("Nearest can't see last\n");
         lastNode = new PathNode();
         lastNode->Setup(worldorigin);
         lastVisible = worldorigin;
      }

      if(!lastNode || (lastNode->CheckMove(worldorigin, mins, maxs) && ClearPathTo(lastNode->worldorigin)))
      {
         lastVisible = worldorigin;
      }

      if(nearestNode)
      {
         lastNode = nearestNode;
      }

      //gi.dprintf( "Num Nodes %d\n", PathManager.NumNodes() );
   }

   if(searchTime <= level.time)
   {
      nearestNode = PathManager.NearestNode(worldorigin);

      if(path)
      {
         delete path;
         path = NULL;
      }

      if(ai_showpath->value)
      {
         if(nearestNode)
         {
            if(!goalNode)
            {
               goalNode = nearestNode;
            }

            find.heuristic.setSize(size);
            path = find.FindPath(nearestNode, goalNode);
         }
         else if(ai_debugpath->value)
         {
            gi.dprintf("%d : No nearest node\n", level.framenum);
         }
      }

      searchTime = level.time + 0.1;

      if(ai_showpath->value)
      {
         if(path)
         {
            path->DrawPath(0, 0.7, 0, 0.1);
         }

         if(nearestNode)
         {
            G_DebugLine(nearestNode->worldorigin + Vector(0, 0, 16), worldorigin + Vector(0, 0, 16), 0, 0, 0.7, 1);
         }
      }
   }
}

EXPORT_FROM_DLL void Player::UpdateStats(void)
{
   Armor *armor;
   int   i;

   // Current Ammo
   if(currentWeapon)
   {
      client->ps.stats[STAT_AMMO] = currentWeapon->AmmoAvailable();
      client->ps.stats[STAT_CLIPAMMO] = currentWeapon->ClipAmmo();
   }
   else
   {
      client->ps.stats[STAT_AMMO] = 0;
   }

   // not used, so don't set to save net bandwidth
#if 0
   // All ammo types
   for(i = 0; i < NUM_AMMO_TYPES; i++)
   {
      Ammo	*ammo;

      assert(ammo_types[i]);
      ammo = (Ammo *)FindItem(ammo_types[i]);
      if(ammo)
      {
         client->ps.stats[STAT_AMMO_BASE + i] = ammo->Amount();
      }
      else
      {
         client->ps.stats[STAT_AMMO_BASE + i] = 0;
      }
}
#endif

   //
   // Armor
   //
   client->ps.stats[STAT_ARMOR] = 0;

   for(i = 0; i < NUM_ARMOR_TYPES; i++)
   {
      assert(armor_types[i]);
      armor = (Armor *)FindItem(armor_types[i]);
      if(armor)
      {
         client->ps.stats[STAT_ARMOR_BASE + i] = armor->Amount();
         client->ps.stats[STAT_ARMOR] += armor->Amount();
      }
      else
      {
         client->ps.stats[STAT_ARMOR_BASE + i] = 0;
      }
   }

   // Average the armor into a single value
   client->ps.stats[STAT_ARMOR] /= 3;

   if(currentWeapon)
   {
      client->ps.stats[STAT_CURRENT_WEAPON] = currentWeapon->GetIconIndex();
   }
   else
   {
      client->ps.stats[STAT_CURRENT_WEAPON] = 0;
   }

   //
   // Weapon list
   //
   client->ps.stats[STAT_WEAPONLIST] = 0;

   //
   // Inventory
   //
   if(currentItem && (!(flags & (FL_SP_MUTANT | FL_MUTANT))))
   {
      InventoryItem *nextItem;
      InventoryItem *prevItem;

      nextItem = (InventoryItem *)NextItem(currentItem);
      prevItem = (InventoryItem *)PrevItem(currentItem);

      client->ps.stats[STAT_SELECTED_ICON] = currentItem->GetIconIndex();
      client->ps.stats[STAT_SELECTED_NAME] = currentItem->GetItemIndex();
      client->ps.stats[STAT_SELECTED_AMOUNT] = currentItem->Amount();
      client->ps.stats[STAT_SELECTED_MODELINDEX] = currentItem->edict->s.modelindex;

      if(prevItem)
         client->ps.stats[STAT_PREVIOUS_ICON] = prevItem->GetIconIndex();
      else
         client->ps.stats[STAT_PREVIOUS_ICON] = 0;

      if(nextItem)
         client->ps.stats[STAT_NEXT_ICON] = nextItem->GetIconIndex();
      else
         client->ps.stats[STAT_NEXT_ICON] = 0;
   }
   else
   {
      client->ps.stats[STAT_SELECTED_ICON] = 0;
      client->ps.stats[STAT_PREVIOUS_ICON] = 0;
      client->ps.stats[STAT_NEXT_ICON] = 0;
      client->ps.stats[STAT_SELECTED_AMOUNT] = 0;
   }

   //
   // Health
   //
   if((health < 1) && (health > 0))
      client->ps.stats[STAT_HEALTH] = 1;
   else
      client->ps.stats[STAT_HEALTH] = health;

   //
   // Frags
   //
   client->ps.stats[STAT_FRAGS] = client->resp.score;

   if(spectator)
   {
      client->ps.stats[STAT_LAYOUTS] = DRAW_SPECTATOR;
   }
   else
   {
      if(!hidestats)
         client->ps.stats[STAT_LAYOUTS] = DRAW_STATS;
      else
         client->ps.stats[STAT_LAYOUTS] = 0;
   }

   //
   // Overlays
   //
   if(drawoverlay)
   {
      client->ps.stats[STAT_LAYOUTS] |= DRAW_OVERLAY;
   }

   //
   // Show scores in dealthmatch during intermission or if the client wants to see them 
   //
   if(deathmatch->value && (level.intermissiontime || client->showinfo))
   {
      client->ps.stats[STAT_LAYOUTS] |= DRAW_SCORES;
   }

   //
   // Powerups timer
   //
   if(poweruptimer > 0)
   {
      client->ps.stats[STAT_POWERUPTIMER] = poweruptimer;
      client->ps.stats[STAT_POWERUPTYPE] = poweruptype;
   }
   else
   {
      client->ps.stats[STAT_POWERUPTIMER] = 0;
      client->ps.stats[STAT_POWERUPTYPE] = 0;
   }

   //
   // Mission Computer
   //
   if(client->showinfo && !deathmatch->value)
   {
      client->ps.stats[STAT_LAYOUTS] |= DRAW_MISSIONCPU;
   }

   // 
   // Clear out the exit sign
   // if we are in the trigger_exit field, than this will constantly be set
   // so this will clear it out on subsequent calls to UpdateStats
   //
   if(client->ps.stats[STAT_EXITSIGN] > 0)
   {
      client->ps.stats[STAT_EXITSIGN]--;
   }

   if(ctf->value)
      CTF_UpdateStats(this);
   //
   // if the player has not joined a team, then update the number of players 
   // or if the scoreboard is up, update it 
   //
   if(!(level.framenum & 31))
   {
      if(ctf->value && client->resp.ctf_team == CTF_NOTEAM)
      {
         CTF_UpdateNumberOfPlayers();
      }
      else if(client->showinfo && deathmatch->value && !(level.framenum & 31))
      {
         G_DeathmatchScoreboardMessage(this, enemy);
         gi.unicast(edict, false);
      }
   }
}

EXPORT_FROM_DLL void Player::UpdateMusic(void)
{
   if(music_forced)
   {
      client->ps.current_music_mood = music_current_mood;
      client->ps.fallback_music_mood = music_fallback_mood;
   }
   else if(action_level > 30)
   {
      music_current_mood = mood_normal;
      music_fallback_mood = mood_normal;
      client->ps.current_music_mood = mood_action;
      client->ps.fallback_music_mood = mood_normal;
   }
   else if((action_level < 15) && (client->ps.current_music_mood == mood_action))
   {
      music_current_mood = mood_normal;
      music_fallback_mood = mood_normal;
      client->ps.current_music_mood = music_current_mood;
      client->ps.fallback_music_mood = music_fallback_mood;
   }
   else if(client->ps.current_music_mood != mood_action)
   {
      client->ps.current_music_mood = music_current_mood;
      client->ps.fallback_music_mood = music_fallback_mood;
   }

   if(action_level > 0)
   {
      action_level -= 0.2f;
      if(action_level > 80)
         action_level = 80;
   }
   else
      action_level = 0;

   //
   // set the music
   // naturally decay the action level
   //
   if(s_debugmusic->value)
   {
      warning("DebugMusic", "%s's action_level = %4.2f\n", client->pers.netname, action_level);
   }
}

/*
=================
EndFrame

Called for each player at the end of the server frame
and right after spawning
=================
*/
EXPORT_FROM_DLL void Player::EndFrame(Event *ev)
{
   FinishMove();
   UpdateStats();
   UpdateMusic();

   // set the crouch flag
   if(client->ps.pmove.pm_flags & PMF_DUCKED)
   {
      edict->s.effects |= EF_CROUCH;
   }
   else
   {
      edict->s.effects &= ~EF_CROUCH;
   }

   if(movieCamera != CinematicCamera)
   {
      if(movieCamera && movieCamera->isSubclassOf<SecurityCamera>())
      {
         movieCamera->edict->s.renderfx &= ~RF_DONTDRAW;
      }
      movieCamera = CinematicCamera;
      if(movieCamera)
      {
         Entity * ent;
         Camera * cam;
         SetViewMode(CAMERA_VIEW, movieCamera);

         if(movieCamera->isSubclassOf<Camera>())
         {
            ent = movieCamera;

            cam = (Camera *)ent;
            if(cam->Overlay().length())
            {
               SendOverlay(this, cam->Overlay());
               drawoverlay = true;
               hidestats = true;
            }
         }
      }
      else
      {
         drawoverlay = false;
         hidestats = false;
         SetViewMode(defaultViewMode);
      }
   }

   if((viewmode == THIRD_PERSON) && (zoom_mode == ZOOMED_IN))
   {
      SetCameraEntity(this);
      if(crosshair)
      {
         crosshair->hideModel();
      }
   }
   else if((viewmode == THIRD_PERSON) && (gravaxis))
   {
      SetCameraEntity(this);
      if(crosshair)
      {
         crosshair->hideModel();
      }
   }
   else
   {
      if(crosshair)
      {
         crosshair->showModel();
      }
      assert(watchCamera);
      if(!watchCamera)
      {
         // fix it in release
         watchCamera = this;
      }
      SetCameraEntity(watchCamera);
   }
}

EXPORT_FROM_DLL void Player::ShowInfo(Event *ev)
{
   client->showinfo = !client->showinfo;

   // CTF - If ctf and client hasn't joined a team yet, make them open the join menu
   if(ctf->value && client->resp.ctf_team == CTF_NOTEAM)
   {
      CTF_OpenJoinMenu();
      return;
   }

   if(deathmatch->value && client->showinfo)
   {
      G_DeathmatchScoreboard(this);
      return;
   }
}

void Player::ReadyToFire(Event *ev)
{
   int frame;

   frame = ev->GetInteger(1);
   if((buttons & BUTTON_ATTACK) && WeaponReady())
   {
      NextFrame(frame);
   }
}

void Player::WaitingToFire(Event *ev)
{
   int frame;

   frame = ev->GetInteger(1);
   if((buttons & BUTTON_ATTACK) && !WeaponReady())
   {
      NextFrame(frame);
   }
}

void Player::DoneFiring(Event *ev)
{
   firing = false;
}

void Player::AddItemToFloatingInventory(Item *item)
{
   floating_inventory.AddObject(item->entnum);
}

void Player::SendFloatingInventory(void)
{
   int i, n;

   // Send over the floating inventory icons
   n = floating_inventory.NumObjects();
   assert(n < MAX_ITEMS);
   gi.WriteByte(svc_inventory);
   gi.WriteShort(n);
   for(i = 1; i <= n; i++)
   {
      Item *item;
      item = (Item *)G_GetEntity(floating_inventory.ObjectAt(i));
      assert(item);
      if(item)
      {
         gi.WriteShort(item->GetIconIndex());
         gi.WriteShort(item->Amount());
      }
   }
   gi.unicast(this->edict, false);
}

void Player::ClearFloatingInventory(Event *ev)
{
   // Clear the items
   floating_inventory.ClearObjectList();
   // Clear the owner;
   floating_owner = NULL;
   // Clear the client's list
   SendFloatingInventory();
}

void Player::SetFloatingOwner(Sentient *deceased_owner)
{
   floating_owner = deceased_owner;
}

Sentient *Player::GetFloatingOwner(void)
{
   return floating_owner;
}

void Player::PickupFloatingInventory(void)
{
   int      i, n;
   Event    *event;
   qboolean inventory_changed = false;

   // Make sure there is an inventory to pickup
   if(!floating_owner)
      return;

   n = floating_inventory.NumObjects();

   if(!n)
      return;

   assert(n < MAX_ITEMS);

   for(i = n; i > 0; i--)
   {
      Item *item;
      item = (Item *)G_GetEntity(floating_inventory.ObjectAt(i));
      assert(item);

      // Check to see if we want to pick this up
      if(!item->Pickupable(this))
         continue;

      // Remove it from the dead body
      item->Drop();

      // Add item to player's inventory
      event = new Event(EV_Item_Pickup);
      event->AddEntity(this);
      item->ProcessEvent(event);
      item->ProcessEvent(EV_Trigger_StartThread);

      // Remove it from the floating inventory
      floating_inventory.RemoveObjectAt(i);

      inventory_changed = true;
   }

   if(inventory_changed)
   {
      // Update the client if the inventory changed
      SendFloatingInventory();
   }
   else
   {
      // Make this guy useless in 60 seconds
      Event *re;
      re = new Event("remove_useless");
      floating_owner->PostEvent(re, 60);

      RandomSound("snd_refusepickup");
   }

   if(!floating_inventory.NumObjects())
   {
      Event *re;
      re = new Event("remove_useless");
      floating_owner->ProcessEvent(re);
   }
}

void Player::ChangeSpectator(void)
{
   int num;
   edict_t		*cl_ent;

   currentCameraTarget = (currentCameraTarget + 1) % game.maxclients;
   cl_ent = g_edicts + 1 + currentCameraTarget;

   num = 0;
   while
      (
         !cl_ent->inuse ||
         !cl_ent->entity ||
         (!cl_ent->entity->client) ||
         (((Player *)cl_ent->entity)->spectator)
         )
   {
      if(num == game.maxclients)
         break;
      currentCameraTarget = (currentCameraTarget + 1) % game.maxclients;
      cl_ent = g_edicts + 1 + currentCameraTarget;
      num++;
   }

   if(!cl_ent->inuse)
   {
      SetViewMode(FIRST_PERSON);
   }
   else
   {
      SetViewMode(SPECTATOR);
   }
   CTF_UpdateNumberOfPlayers();
}

void Player::TestThread(Event *ev)
{
   const char *scriptfile;
   const char *label = NULL;
   ScriptThread * thread;

   if(ev->NumArgs() < 1)
   {
      gi.cprintf(edict, PRINT_HIGH, "Syntax: testthread scriptfile <label>.\n");
      return;
   }
   scriptfile = ev->GetString(1);
   if(ev->NumArgs() > 1)
      label = ev->GetString(2);
   thread = Director.CreateThread(scriptfile, LEVEL_SCRIPT, label);
   if(thread)
   {
      // start right away
      thread->Start(-1);
   }
}

void Player::GibEvent(Event *ev)
{
   qboolean hidemodel;

   hidemodel = !ev->GetInteger(1);

   if(sv_gibs->value && !parentmode->value)
   {
      if(hidemodel)
      {
         gibbed = true;
         takedamage = DAMAGE_NO;
         setSolidType(SOLID_NOT);
         hideModel();
      }
      CreateGibs(this, health, 0.75f, 3);
   }
}

void Player::GotKill(Event *ev)
{
   Entity *victim;
   Entity *inflictor;
   str     location;
   float   damage;
   int     meansofdeath;
   qboolean gibbed;

   if(deathmatch->value)
   {
      return;
   }

   victim = ev->GetEntity(1);
   damage = ev->GetInteger(2);
   inflictor = ev->GetEntity(3);
   location = ev->GetString(4);
   meansofdeath = ev->GetInteger(5);
   gibbed = ev->GetInteger(6);

   if(victim->isSubclassOf<Actor>())
   {
      Actor * act = static_cast<Actor *>(victim);
      switch(act->actortype)
      {
      case IS_ENEMY:
      case IS_MONSTER:
         if(gibbed)
         {
            if(G_Random(100) < 15)
            {
               RandomSound("snd_gibfest", 1, CHAN_VOICE);
            }
         }
         else if(G_Random(100) < 15)
         {
            RandomSound("snd_taunt", 1, CHAN_VOICE);
         }
         break;
      case IS_FRIEND:
      case IS_CIVILIAN:
         if(
            (!(flags & FL_MUTANT)) &&
            (!deathmatch->value) &&
            (!level.no_jc)
            )
         {
            char name[128];
            int num;

            num = (int)G_Random(4) + 1;
            snprintf(name, sizeof(name), "global/universal_script.scr::blade_kills_innocent%d", num);
            ExecuteThread(name, true);
         }
         break;
      }
   }
   if(victim->isClient())
   {
      if(gibbed)
      {
         if(G_Random(100) < 15)
         {
            RandomSound("snd_gibfest", 1, CHAN_VOICE);
         }
      }
      else if(
         (G_Random(100) < 15)
         )
      {
         RandomSound("snd_taunt", 1, CHAN_VOICE);
      }
   }
}

void Player::SetPowerupTimer(Event *ev)
{
   Event *event;

   poweruptimer = ev->GetInteger(1);
   poweruptype = ev->GetInteger(2);
   event = new Event(EV_Player_UpdatePowerupTimer);
   PostEvent(event, 1);
}

void Player::UpdatePowerupTimer(Event *ev)
{
   poweruptimer -= 1;

   if(poweruptimer > 0)
      PostEvent(ev, 1);
   else
      poweruptype = 0;
}

viewmode_t Player::ViewMode(void)
{
   return viewmode;
}

Camera *Player::CurrentCamera(void)
{
   Entity * ent;
   ent = CinematicCamera;
   return (Camera *)ent;
}

void Player::SetCamera(Entity *ent)
{
   CinematicCamera = ent;
}

void Player::WhatIs(Event *ev)
{
   int num;
   Entity *ent;

   if(ev->NumArgs() != 1)
   {
      gi.cprintf(edict, PRINT_HIGH, "Usage: whatis <entity number>\n");
      return;
   }

   num = ev->GetInteger(1);
   if((num < 0) || (num >= globals.max_edicts))
   {
      gi.cprintf(edict, PRINT_HIGH, "Value out of range.  Possible values range from 0 to %d.\n", globals.max_edicts);
      return;
   }

   ent = G_GetEntity(num);
   if(!ent)
   {
      gi.cprintf(edict, PRINT_HIGH, "Entity not in use.\n");
   }
   else
   {
      const char * animname;
      int master, own;

      animname = NULL;
      if(gi.IsModel(ent->edict->s.modelindex))
         animname = gi.Anim_NameForNum(ent->edict->s.modelindex, ent->edict->s.anim);
      if(!animname)
         animname = "( N/A )";

      if(ent->bindmaster)
         master = ent->bindmaster->entnum;
      else
         master = 0;

      if(ent->edict->owner)
         own = ent->edict->owner - g_edicts;
      else
         own = 0;

      gi.cprintf(edict, PRINT_HIGH,
                 "Entity #   : %d\n"
                 "Class ID   : %s\n"
                 "Classname  : %s\n"
                 "Targetname : %s\n"
                 "Modelname  : %s\n"
                 "Animname   : %s\n"
                 "Origin     : ( %f, %f, %f )\n"
                 "Bounds     : Mins( %.2f, %.2f, %.2f ) Maxs( %.2f, %.2f, %.2f )\n"
                 "Velocity   : ( %f, %f, %f )\n"
                 "SVFLAGS    : %x\n"
                 "Movetype   : %i\n"
                 "Solidtype  : %i\n"
                 "Parent     : %i\n"
                 "Health     : %.1f\n"
                 "Max Health : %.1f\n"
                 "BindMaster : %i\n"
                 "Edict Owner: %i\n",
                 num,
                 ent->getClassID(),
                 ent->getClassname(),
                 ent->TargetName(),
                 ent->model.c_str(),
                 animname,
                 ent->worldorigin.x, ent->worldorigin.y, ent->worldorigin.z,
                 ent->mins.x, ent->mins.y, ent->mins.z, ent->maxs.x, ent->maxs.y, ent->maxs.z,
                 ent->velocity.x, ent->velocity.y, ent->velocity.z,
                 ent->edict->svflags,
                 ent->movetype,
                 ent->edict->solid,
                 ent->edict->s.parent,
                 ent->health,
                 ent->max_health,
                 master,
                 own
      );
   }
}

void Player::KillEnt(Event * ev)
{
   int num;
   Entity *ent;

   if(ev->NumArgs() != 1)
   {
      gi.cprintf(edict, PRINT_HIGH, "Usage: killent <entity number>\n");
      return;
   }

   num = ev->GetInteger(1);
   if((num < 0) || (num >= globals.max_edicts))
   {
      gi.cprintf(edict, PRINT_HIGH, "Value out of range.  Possible values range from 0 to %d.\n", globals.max_edicts);
      return;
   }

   ent = G_GetEntity(num);
   ent->Damage(world, world, ent->max_health + 25, worldorigin, vec_zero, vec_zero, 0, 0, 0, -1, -1, 1);
}

void Player::RemoveEnt(Event * ev)
{
   int num;
   Entity *ent;

   if(ev->NumArgs() != 1)
   {
      gi.cprintf(edict, PRINT_HIGH, "Usage: removeent <entity number>\n");
      return;
   }

   num = ev->GetInteger(1);
   if((num < 0) || (num >= globals.max_edicts))
   {
      gi.cprintf(edict, PRINT_HIGH, "Value out of range.  Possible values range from 0 to %d.\n", globals.max_edicts);
      return;
   }

   ent = G_GetEntity(num);
   ent->PostEvent(Event(EV_Remove), 0);
}

void Player::KillClass(Event * ev)
{
   int except;
   str classname;
   edict_t * from;
   Entity *ent;

   if(ev->NumArgs() < 1)
   {
      gi.cprintf(edict, PRINT_HIGH, "Usage: killclass <classname> [except entity number]\n");
      return;
   }

   classname = ev->GetString(1);

   except = 0;
   if(ev->NumArgs() == 2)
   {
      except = ev->GetInteger(1);
   }

   for(from = this->edict + 1; from < &g_edicts[globals.num_edicts]; from++)
   {
      if(!from->inuse)
      {
         continue;
      }

      assert(from->entity);

      ent = from->entity;

      if(ent->entnum == except)
      {
         continue;
      }

      if(ent->inheritsFrom(classname.c_str()))
      {
         ent->Damage(world, world, ent->max_health + 25, worldorigin, vec_zero, vec_zero, 0, 0, 0, -1, -1, 1);
      }
   }
}

void Player::RemoveClass(Event * ev)
{
   int except;
   str classname;
   edict_t * from;
   Entity *ent;

   if(ev->NumArgs() < 1)
   {
      gi.cprintf(edict, PRINT_HIGH, "Usage: removeclass <classname> [except entity number]\n");
      return;
   }

   classname = ev->GetString(1);

   except = 0;
   if(ev->NumArgs() == 2)
   {
      except = ev->GetInteger(1);
   }

   for(from = this->edict + 1; from < &g_edicts[globals.num_edicts]; from++)
   {
      if(!from->inuse)
      {
         continue;
      }

      assert(from->entity);

      ent = from->entity;

      if(ent->entnum == except)
         continue;

      if(ent->inheritsFrom(classname.c_str()))
      {
         ent->PostEvent(Event(EV_Remove), 0);
      }
   }
}

void Player::ActorInfo(Event *ev)
{
   int num;
   Entity *ent;

   if(ev->NumArgs() != 1)
   {
      gi.cprintf(edict, PRINT_HIGH, "Usage: actorinfo <entity number>\n");
      return;
   }

   num = ev->GetInteger(1);
   if((num < 0) || (num >= globals.max_edicts))
   {
      gi.cprintf(edict, PRINT_HIGH, "Value out of range.  Possible values range from 0 to %d.\n", globals.max_edicts);
      return;
   }

   ent = G_GetEntity(num);
   if(!ent || !ent->isSubclassOf<Actor>())
   {
      gi.cprintf(edict, PRINT_HIGH, "Entity not an Actor.\n");
   }
   else
   {
      static_cast<Actor *>(ent)->ShowInfo();
   }
}

void Player::Taunt(Event *ev)
{
   Event * event;
   str taunt;

   if(level.time < lastTauntTime)
      return;

   taunt = str("snd_taunt") + str(ev->GetString(1));
   event = new Event(EV_VoiceSound);
   event->AddString(taunt);
   event->AddFloat(1);
   ProcessEvent(event);
   lastTauntTime = level.time + TAUNT_TIME;
}

void Player::DrawOverlay(Event *ev)
{
   drawoverlay = true;
}

void Player::HideOverlay(Event *ev)
{
   drawoverlay = false;
}

void Player::DrawStats(Event *ev)
{
   hidestats = false;
}

void Player::HideStats(Event *ev)
{
   hidestats = true;
}

void Player::ChangeMusic(const char * current, const char * fallback, qboolean force)
{
   int current_mood_num;
   int fallback_mood_num;

   music_forced = force;
   if(str(current) == str("normal"))
   {
      music_forced = false;
   }

   if(current)
   {
      current_mood_num = MusicMood_NameToNum(current);
      if(current_mood_num < 0)
      {
         gi.dprintf("current music mood %s not found", current);
      }
      else
      {
         music_current_mood = current_mood_num;
      }
   }
   if(fallback)
   {
      fallback_mood_num = MusicMood_NameToNum(fallback);
      if(fallback_mood_num < 0)
      {
         gi.dprintf("fallback music mood %s not found", fallback);
         fallback = NULL;
      }
      else
      {
         music_fallback_mood = fallback_mood_num;
      }
   }
}

void Player::SetFlashColor(Event *ev)
{
   if(flash_color[3] == 0)
   {
      flash_color[0] = flash_color[1] = flash_color[2] = 0;
   }

   flash_color[0] += ev->GetFloat(1);
   flash_color[1] += ev->GetFloat(2);
   flash_color[2] += ev->GetFloat(3);
   flash_color[3] += ev->GetFloat(4);

   if(flash_color[0] > 1)
      flash_color[0] = 1;
   if(flash_color[1] > 1)
      flash_color[1] = 1;
   if(flash_color[2] > 1)
      flash_color[2] = 1;
   if(flash_color[3] > 1)
      flash_color[3] = 1;
}

void Player::ClearFlashColor(Event *ev)
{
   flash_color[0] = 0;
   flash_color[1] = 0;
   flash_color[2] = 0;
   flash_color[3] = 0;
}

void Player::GiveOxygen(float time)
{
   air_finished = level.time + time;
}

void Player::WeaponUse(Event *ev)
{
   Event *event;

   // CTF: If the current weapon has alternate fire, change the mode to 
   // secondary, and fire it.
   if(currentWeapon && currentWeapon->AlternateFire())
   {
      currentWeapon->SetSecondaryMode();
      if(currentWeapon->ReadyToFire() && currentWeapon->HasAmmo())
      {
         currentWeapon->Fire();
      }
      else
      {
         currentWeapon->SetPrimaryMode();
      }
   }
   else if(currentWeapon && !currentWeapon->ChangingWeapons())
   {
      event = new Event(EV_Weapon_SecondaryUse);
      event->AddEntity(this);
      currentWeapon->ProcessEvent(event);
      return;
   }
}

void Player::Human(Event *ev)
{
   int playernum;

   flags &= ~FL_SP_MUTANT;
   flags &= ~FL_MUTANT;

   setModel(savemodel.c_str());
   SetAnim("idle");

   strcpy(client->pers.model, savemodel.c_str());
   strcpy(client->pers.skin, saveskin.c_str());

   playernum = edict - g_edicts - 1;

   // combine name, skin and model into a configstring
   gi.configstring(CS_PLAYERSKINS + playernum, 
                   va("%s\\%s\\%s", client->pers.netname, client->pers.model, client->pers.skin));

   takeWeapon("MutantHands");
}

void Player::Mutate(Event *ev)
{
   int      playernum;
   Weapon   *mutanthands;

   flags |= FL_SP_MUTANT;

   setModel("manumit_pl.def");
   SetAnim("idle");

   savemodel = client->pers.model;
   saveskin = client->pers.skin;

   strcpy(client->pers.model, "manumit_pl.def");

   playernum = edict - g_edicts - 1;


   // combine name, skin and model into a configstring
   gi.configstring(CS_PLAYERSKINS + playernum,
                   va("%s\\%s\\%s", client->pers.netname, client->pers.model, client->pers.skin));

   // Give the mutanthands weapon. - Force it.
   mutanthands = giveWeapon("MutantHands");
   ForceChangeWeapon(mutanthands);
}

void Player::SetSkin(Event *ev)
{
   int playernum;

   playernum = edict - g_edicts - 1;

   strcpy(client->pers.skin, ev->GetString(1));

   // combine name, skin and model into a configstring
   gi.configstring(CS_PLAYERSKINS + playernum, 
                   va("%s\\%s\\%s", client->pers.netname, client->pers.model, client->pers.skin));

}

void Player::CTF_JoinTeam(int desired_team)
{
   int playernum;

   //
   // if he is in a vehicle 
   // get him out
   //
   if(vehicle)
   {
      Event * event;

      event = new Event(EV_Use);
      event->AddEntity(this);
      vehicle->ProcessEvent(event);
   }

   if(spectator)
   {
      client->ps.stats[STAT_LAYOUTS] &= ~DRAW_SPECTATOR;
      spectator = false;
      takedamage = DAMAGE_YES;
      setSolidType(SOLID_BBOX);
      setMoveType(MOVETYPE_WALK);
      defaultViewMode = FIRST_PERSON;
      SetViewMode(FIRST_PERSON);
   }

   // Drop flag, release grapple, drop inventory, and free weapons
   CTF_DeadDropFlag();
   CTF_DeadDropTech();
   CTF_ReleaseGrapple();
   DropInventoryItems();
   FreeInventory();

   // reset score
   client->resp.score = 0;
   client->resp.ctf_team = desired_team;
   client->resp.ctf_state = CTF_STATE_START;

   // set the respawn time of the player
   respawn_time = level.time;

   Init();

   // Update team, and put state to the starting state so player will respawn near his base

   // hold in place briefly
   client->ps.pmove.pm_time = 50;
   client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;

   switch(client->resp.ctf_team)
   {
   case CTF_TEAM_HARDCORPS:
      strcpy(client->pers.skin, ctf_hardcorps_skin->string);
      break;
   case CTF_TEAM_SINTEK:
      strcpy(client->pers.skin, ctf_sintek_skin->string);
      break;
   default:
      break;
   }

   // Player number
   playernum = edict - g_edicts - 1;

   // combine name, skin and model into a configstring
   gi.configstring(CS_PLAYERSKINS + playernum, 
                   va("%s\\%s\\%s", client->pers.netname, client->pers.model, client->pers.skin));
}

void Player::CTF_JoinTeamHardcorps(Event *ev)
{
   CTF_JoinTeam(CTF_TEAM_HARDCORPS);
   gi.bprintf(PRINT_HIGH, "%s joined the %s team.\n", client->pers.netname, CTF_TeamName(client->resp.ctf_team));
}

void Player::CTF_JoinTeamSintek(Event *ev)
{
   CTF_JoinTeam(CTF_TEAM_SINTEK);
   gi.bprintf(PRINT_HIGH, "%s joined the %s team.\n", client->pers.netname, CTF_TeamName(client->resp.ctf_team));
}

void Player::CTF_Team(Event *ev)
{
   str team;
   int desired_team;

   if(ev->NumArgs() == 0)
   {
      gi.cprintf(edict, PRINT_HIGH, "You are on the %s team.\n",
                 CTF_TeamName(client->resp.ctf_team));
      return;
   }

   team = ev->GetString(1);

   if((!stricmp(team.c_str(), "hardcorps")) || (!stricmp(team.c_str(), "blue")))
   {
      desired_team = CTF_TEAM_HARDCORPS;
   }
   else if((!stricmp(team.c_str(), "sintek")) || (!stricmp(team.c_str(), "red")))
   {
      desired_team = CTF_TEAM_SINTEK;
   }
   else
   {
      gi.cprintf(edict, PRINT_HIGH, "Unknown team %s.\n", team.c_str());
      return;
   }

   if(client->resp.ctf_team == desired_team)
   {
      gi.cprintf(edict, PRINT_HIGH, "You are already on the %s team.\n",
                 CTF_TeamName(client->resp.ctf_team));
      return;
   }

   CTF_JoinTeam(desired_team);

   gi.bprintf(PRINT_HIGH, "%s changed to the %s team.\n",
              client->pers.netname, CTF_TeamName(desired_team));
}

void Player::CTF_AssignTeam(void)
{
   edict_t		*player;
   int         i;
   int         hardcorps_count = 0, sintek_count = 0;

   client->resp.ctf_state = CTF_STATE_START;

   for(i = 1; i <= maxclients->value; i++)
   {
      player = &g_edicts[i];

      if(!player->inuse || player->client == this->client)
         continue;

      switch(player->client->resp.ctf_team)
      {
      case CTF_TEAM_HARDCORPS:
         hardcorps_count++;
         break;
      case CTF_TEAM_SINTEK:
         sintek_count++;
      }
   }

   if(hardcorps_count < sintek_count)
      CTF_JoinTeam(CTF_TEAM_HARDCORPS);
   else if(sintek_count < hardcorps_count)
      CTF_JoinTeam(CTF_TEAM_SINTEK);
   else if(rand() & 1)
      CTF_JoinTeam(CTF_TEAM_HARDCORPS);
   else
      CTF_JoinTeam(CTF_TEAM_SINTEK);
}

void Player::InitCTF(void)
{
   // This player has already joined a team
   if(client->resp.ctf_team != CTF_NOTEAM)
      return;

   if(ctf_forcejoin->value)
   {
      CTF_AssignTeam();
   }
   else
   {
      // start as 'spectator'
      hideModel();
      setSolidType(SOLID_NOT);
      spectator = true;
      takedamage = DAMAGE_NO;
      CTF_DeadDropFlag();
      CTF_DeadDropTech();
      DropInventoryItems();
      FreeInventory();
      CTF_OpenJoinMenu();
   }
}

void Player::CTF_UpdateNumberOfPlayers(void)
{
   char  string[1400];
   int   num_hardcorps = 0, num_sintek = 0;
   int   i;

   if(!ctf->value || client->resp.ctf_team != CTF_NOTEAM)
      return;

   for(i = 0; i < maxclients->value; i++)
   {
      if(!g_edicts[i + 1].inuse)
         continue;

      if(game.clients[i].resp.ctf_team == CTF_TEAM_HARDCORPS)
         num_hardcorps++;
      else if(game.clients[i].resp.ctf_team == CTF_TEAM_SINTEK)
         num_sintek++;
   }

   // Add in an overlay that shows the number of players on each team so far.
   snprintf(string, sizeof(string),
            "jcx yb 24 string \"Team Hardcorps: %d players  Score: %d\""
            "jcx yb 16 string \"Team Sintek: %d players  Score: %d\"",
            num_hardcorps, ctfgame.team_hardcorps, num_sintek, ctfgame.team_sintek);

   if(spectator)
   {
      char entry[1024];

      if(viewmode == FIRST_PERSON)
      {
         gi.WriteByte(svc_layout);
         gi.WriteString(va("jcx jb hstring 0 0 1 \"SPECTATOR MODE\" %s ", string));
         gi.unicast(edict, true);
      }
      else if(viewmode == SPECTATOR)
      {
         edict_t * cl_ent;

         cl_ent = g_edicts + 1 + currentCameraTarget;
         if(cl_ent && cl_ent->entity && cl_ent->entity->client)
         {
            Com_sprintf(entry, sizeof(entry),
                        "client %i %i %i %i %i %i ",
                        100, 176, currentCameraTarget, cl_ent->entity->client->resp.score, cl_ent->entity->client->ping, 
                        (level.framenum - cl_ent->entity->client->resp.enterframe) / 600);
            gi.WriteByte(svc_layout);
            gi.WriteString(va("jcx jb hstring 0 0 1 \"SPECTATOR MODE\" %s %s ", entry, string));
            gi.unicast(edict, true);
         }
      }
      return;
   }
   else
   {
      gi.WriteByte(svc_layout);
      gi.WriteString(string);
      gi.unicast(edict, true);
   }
}

void Player::CTF_OpenJoinMenu(void)
{
   // update the number of players
   CTF_UpdateNumberOfPlayers();

   // Load up the CTF menu so the player can choose which team.
   gi.WriteByte(svc_stufftext);
   gi.WriteString("menu_load ctf.mnu; menu_generic\n");
   gi.unicast(edict, true);
}

void Player::CTF_DropTech(Event *ev)
{
   CTF_Tech *tech;

   if((tech = (CTF_Tech *)HasItemOfSuperclass("CTF_Tech")))
   {
      Vector forward, right;

      tech->setOrigin(worldorigin + Vector(0, 0, 80));
      tech->worldorigin.copyTo(tech->edict->s.old_origin);
      // drop the item
      tech->PlaceItem();

      v_angle.AngleVectors(&forward, &right, NULL);
      tech->velocity = velocity * 0.5 + forward * 200;
      tech->setAngles(angles);
      tech->avelocity = Vector(0, G_CRandom(360), 0);

      tech->SetTriggerTime(level.time + 2);

      tech->spawnflags |= DROPPED_PLAYER_ITEM;

      // Remove this from the owner's item list
      RemoveItem(tech);
      tech->ClearOwner();

      tech->PostEvent(EV_Tech_Timeout, CTF_TECH_TIMEOUT);
   }
}

void Player::CTF_DropFlag(Event *ev)
{
   Item     *dropped_flag;
   str      enemy_flag;
   Vector   forward, right;

   // Figure out the enemy flag
   if(client->resp.ctf_team == CTF_TEAM_HARDCORPS)
   {
      enemy_flag = "CTF_Flag_Sintek";
   }
   else
   {
      enemy_flag = "CTF_Flag_Hardcorps";
   }

   edict->s.color_r = 0;
   edict->s.color_g = 0;
   edict->s.color_b = 0;
   edict->s.radius = 0;
   edict->s.renderfx &= ~RF_DLIGHT;

   if(dropped_flag = FindItem(enemy_flag.c_str()))
   {
      dropped_flag->detach();

      if(enemy_flag == "CTF_Flag_Hardcorps")
      {
         dropped_flag->edict->s.color_r = 0;
         dropped_flag->edict->s.color_g = 0;
         dropped_flag->edict->s.color_b = 1.0;
         dropped_flag->edict->s.radius = 150;
         dropped_flag->edict->s.renderfx |= RF_DLIGHT;
      }
      else if(enemy_flag == "CTF_Flag_Sintek")
      {
         dropped_flag->edict->s.color_r = 1.0;
         dropped_flag->edict->s.color_g = 0;
         dropped_flag->edict->s.color_b = 0;
         dropped_flag->edict->s.radius = 150;
         dropped_flag->edict->s.renderfx |= RF_DLIGHT;
      }

      dropped_flag->RandomAnimate("idle", NULL);
      dropped_flag->PostEvent(EV_Flag_Reset, CTF_DROPPED_FLAG_RETURN_TIMEOUT);
      dropped_flag->setOrigin(worldorigin + Vector(0, 0, 80));
      dropped_flag->worldorigin.copyTo(dropped_flag->edict->s.old_origin);
      // drop the item
      dropped_flag->PlaceItem();
      v_angle.AngleVectors(&forward, &right, NULL);
      dropped_flag->velocity = velocity * 0.5 + forward * 200;
      dropped_flag->setAngles(angles);
      dropped_flag->avelocity = Vector(0, G_CRandom(360), 0);
      dropped_flag->SetTriggerTime(level.time + 2);
      dropped_flag->spawnflags |= DROPPED_PLAYER_ITEM;
      // Remove this from the owner's item list
      RemoveItem(dropped_flag);
      dropped_flag->ClearOwner();
   }
}

void Player::CTF_DeadDropTech(void)
{
   CTF_Tech *tech;

   if(tech = (CTF_Tech *)HasItemOfSuperclass("CTF_Tech"))
   {
      tech->detach();
      tech->PostEvent(EV_Tech_Timeout, CTF_TECH_TIMEOUT);
      // The tech will be dropped by the Player::Killed() function
   }
}

void Player::CTF_DeadDropFlag(void)
{
   Item  *dropped_flag;
   str   enemy_flag;

   // When a player dies, check to see if he's carrying an enemy flag and print a message
   // Also set up a timer to respawn the flag back at it's home base if it's not
   // recovered

   // Figure out the enemy flag
   if(client->resp.ctf_team == CTF_TEAM_HARDCORPS)
   {
      enemy_flag = "CTF_Flag_Sintek";
   }
   else
   {
      enemy_flag = "CTF_Flag_Hardcorps";
   }

   edict->s.color_r = 0;
   edict->s.color_g = 0;
   edict->s.color_b = 0;
   edict->s.radius = 0;
   edict->s.renderfx &= ~RF_DLIGHT;

   if(dropped_flag = FindItem(enemy_flag.c_str()))
   {
      gi.bprintf(PRINT_HIGH, "%s lost the %s flag!\n", client->pers.netname, CTF_OtherTeamName(client->resp.ctf_team));
      dropped_flag->detach();

      if(enemy_flag == "CTF_Flag_Hardcorps")
      {
         dropped_flag->edict->s.color_r = 0;
         dropped_flag->edict->s.color_g = 0;
         dropped_flag->edict->s.color_b = 1.0;
         dropped_flag->edict->s.radius = 150;
         dropped_flag->edict->s.renderfx |= RF_DLIGHT;
      }
      else if(enemy_flag == "CTF_Flag_Sintek")
      {
         dropped_flag->edict->s.color_r = 1.0;
         dropped_flag->edict->s.color_g = 0;
         dropped_flag->edict->s.color_b = 0;
         dropped_flag->edict->s.radius = 150;
         dropped_flag->edict->s.renderfx |= RF_DLIGHT;
      }

      dropped_flag->RandomAnimate("idle", NULL);
      dropped_flag->PostEvent(EV_Flag_Reset, CTF_AUTO_FLAG_RETURN_TIMEOUT);
      // The flag will be physically dropped by the Player::Killed() function
   }
}

Vector Player::CTF_GetGrapplePosition(void)
{
   const gravityaxis_t &grav = gravity_axis[gravaxis];
   Vector right, pos;

   right[grav.x] = -orientation[1][0];
   right[grav.y] = -orientation[1][1] * grav.sign;
   right[grav.z] = -orientation[1][2] * grav.sign;

   pos = GunPosition() + (right * 8);

   return pos;
}

void Player::CTF_UpdateHookBeam(Event *ev)
{
   if(beam && hook)
   {
      beam->setBeam(CTF_GetGrapplePosition(), hook->worldorigin, 1, 0, 0, 0, 1, 0);
      PostEvent(EV_Player_CTF_UpdateHookBeam, FRAMETIME);
   }

   if(!hook && beam)
   {
      beam->ProcessEvent(EV_Remove);
   }
}

void Player::CTF_ExtendGrapple(void)
{
   const gravityaxis_t &grav = gravity_axis[gravaxis];
   Vector pos, forward;

   forward[grav.x] = orientation[0][0];
   forward[grav.y] = orientation[0][1] * grav.sign;
   forward[grav.z] = orientation[0][2] * grav.sign;

   pos = CTF_GetGrapplePosition();

   RandomGlobalSound("snd_grapfire");

   hook = new Hook();
   hook->Setup(this, pos, forward);
   hook->RandomGlobalEntitySound("snd_grapextend");

   beam = new Beam();
   beam->setBeam(pos, hook->worldorigin, 1, 0, 0, 0, 1, 0);
   PostEvent(EV_Player_CTF_UpdateHookBeam, FRAMETIME);
}

void Player::CTF_ReleaseGrapple(void)
{
   //Client released the fire button, kill the hook
   if(hook)
   {
      hook->setSolidType(SOLID_NOT);
      hook->setMoveType(MOVETYPE_NONE);
      hook->PostEvent(EV_Remove, 0);
      hook = NULL;
   }

   if(beam)
   {
      beam->PostEvent(EV_Remove, 0);
      beam = NULL;
   }

   if(grapple_pull)
      RandomGlobalSound("snd_graprels");

   ClearGrapplePull();
}

void Player::SetGrapplePull(Vector org, float speed)
{
   grapple_pull = true;
   grapple_org = org;
   grapple_speed = speed;
}

void Player::ClearGrapplePull(void)
{
   grapple_pull = false;
   grapple_org = vec_zero;
   grapple_speed = 0;
   gravity = 1.0f;
   grapple_time = level.time + 1.0f;
}

void Player::CTF_SoundEvent(Event *ev)
{
   Item *item;
   ctfsoundevent_t type;

   type = (ctfsoundevent_t)ev->GetInteger(1);

   if(type == FIRE)
   {
      if(item = FindItem("CTF_Tech_Double"))
      {
         RandomGlobalSound("snd_double");
      }
      if(item = FindItem("CTF_Tech_DeathQuad"))
      {
         RandomGlobalSound("snd_deathquad");
      }
   }
   else if(type == DAMAGE)
   {
      if(item = FindItem("CTF_Tech_Empathy"))
      {
         RandomGlobalSound("snd_empathy");
      }
      else if(item = FindItem("CTF_Tech_Half"))
      {
         RandomGlobalSound("snd_shield");
      }
      else if(item = FindItem("CTF_Tech_Regeneration"))
      {
         RandomGlobalSound("snd_regen");
      }
      else if(item = FindItem("CTF_Tech_DeathQuad"))
      {
         RandomGlobalSound("snd_dquadpain");
      }
   }
}

void Player::DropWeaponEvent(Event *ev)
{
   if(currentWeapon &&
      currentWeapon->IsDroppable() &&
      (!deathmatch->value || !DM_FLAG(DF_NO_DROP_WEAPONS)))
   {
      currentWeapon->TakeAllAmmo();
      DropCurrentWeapon();
   }
}

void Player::SetViewAngles(Vector ang)
{
   v_angle = ang;
}

void Player::GetPlayerView(Vector *pos, Vector *angle)
{
   if(pos)
   {
      *pos = origin;
      pos->z += viewheight;
   }

   if(angle)
   {
      *angle = v_angle;
   }
}

// EOF

