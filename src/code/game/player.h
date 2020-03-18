//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/player.h                         $
// $Revision:: 125                                                            $
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

#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "g_local.h"
#include "vector.h"
#include "entity.h"
#include "weapon.h"
#include "sentient.h"
#include "navigate.h"
#include "misc.h"
#include "bspline.h"
#include "camera.h"
#include "vehicle.h"
#include "specialfx.h"
#include "grapple.h"

extern Event EV_Player_EndLevel;
extern Event EV_Player_PrevWeapon;
extern Event EV_Player_NextWeapon;
extern Event EV_Player_GiveCheat;
extern Event EV_Player_GodCheat;
extern Event EV_Player_NoTargetCheat;
extern Event EV_Player_NoClipCheat;
extern Event EV_Player_GameVersion;
extern Event EV_Player_Fov;
extern Event EV_Player_SaveFov;
extern Event EV_Player_RestoreFov;
extern Event EV_Player_ZoomOut;
extern Event EV_Player_ToggleZoomMode;
extern Event EV_Player_ClearFloatingInventory;
extern Event EV_Player_WhatIs;
extern Event EV_Player_DrawOverlay;
extern Event EV_Player_HideOverlay;
extern Event EV_Player_DrawStats;
extern Event EV_Player_HideStats;
extern Event EV_Player_SetFlashColor;
extern Event EV_Player_ClearFlashColor;
extern Event EV_Player_Respawn;
extern Event EV_Player_CTF_SoundEvent;

typedef enum
{
   ZOOMED_OUT,
   ZOOMED_IN
} zoom_mode_t;

typedef enum
{
   FIRST_PERSON,
   THIRD_PERSON,
   SPECTATOR,
   CAMERA_VIEW
} viewmode_t;

// view pitching times
#define  DAMAGE_TIME          0.5
#define  FALL_TIME            0.3
#define  ENEMY_TIME           5.0

class EXPORT_FROM_DLL Player : public Sentient
{
protected:
   pmove_state_t     old_pmove;     // for detecting out-of-pmove changes

   // bobbing
   float             xyspeed;
   float             bobtime;
   float             bobmove;
   int               bobcycle;      // odd cycles are right foot going forward
   float             bobfracsin;    // sin(bobfrac*M_PI)

   Vector            oldvelocity;

   // weapon kicks
   Vector            kick_angles;
   Vector            kick_origin;

   // damage kicks
   float             v_dmg_roll;
   float             v_dmg_pitch;
   float             v_dmg_time;

   // for view drop on fall
   float             fall_time;
   float             fall_value;

   // blend //FIXME move to entity?
   float             blend[4];      // rgba full screen effect
   float             fov;           // horizontal field of view 
   float             savedfov;      // saved horizontal field of view for teleports

   // view weapon orientation
   Vector            v_gunoffset;
   Vector            v_gunangles;

   friend class Camera;
   friend class Vehicle;

   // vehicle stuff
   VehiclePtr        vehicle;
   str               vehicleanim;

   // aiming direction
   Vector            v_angle;
   Vector            v_kick;
   Vector            oldviewangles;
   Vector            v_offset;

   int               buttons;
   int               new_buttons;
   viewmode_t        viewmode;
   viewmode_t        defaultViewMode;
   zoom_mode_t       zoom_mode;

   qboolean          firing;
   qboolean          in_console;
   qboolean          has_thought;

   // damage blend
   float             damage_blood;
   float             damage_alpha;
   Vector            damage_blend;
   float             bonus_alpha;

   float             next_drown_time;
   float             air_finished;

   int               old_waterlevel;
   int               drown_damage;

   Path             *path;
   PathNodePtr       goalNode;
   PathNodePtr       nearestNode;
   PathNodePtr       lastNode;
   Vector            lastVisible;
   Vector            lastGround;
   float             searchTime;
   str               con_name;

   EntityPtr         thirdpersonCamera;
   EntityPtr         watchCamera;
   EntityPtr         movieCamera;
   EntityPtr         spectatorCamera;
   EntityPtr         CinematicCamera;
   EntityPtr         crosshair;

   qboolean          onladder;

   qboolean          spectator;
   qboolean          hidestats;
   qboolean          drawoverlay;

   qboolean          grapple_pull;
   Vector            grapple_org;
   float             grapple_speed;
   float             grapple_time;

   // music mood stuff
   float             action_level;
   int               music_current_mood;
   int               music_fallback_mood;

   // falling damage stuff
   csurface_t       *fallsurface;

   // Pickup inventory stuff
   SentientPtr       floating_owner;
   Container<int>    floating_inventory;

   float             lastEnemyTime;

   int               currentCameraTarget;
   qboolean          gibbed;
   qboolean          drawstats;

   float             lastTauntTime;

   // variable to see if we checked the dead camera
   qboolean          checked_dead_camera;

   qboolean          falling;
   float             flash_color[4];

   float             last_damage_time;
   qboolean          music_forced;

   // CTF
   HookPtr           hook;
   BeamPtr           beam;
   float             spectator_distance;

public:

   CLASS_PROTOTYPE(Player);

   float             respawn_time;
   qboolean          trappedInQuantum;

   Player();
   virtual ~Player();

   virtual void      Init();
   virtual void      RestorePersistantData(SpawnArgGroup &group);
   virtual void      WritePersistantData(SpawnArgGroup &group);

   virtual void      InitMusic();
   virtual void      InitEdict();
   virtual void      InitClient();
   virtual void      InitPath();
   virtual void      InitPhysics();
   virtual void      InitPowerups();
   virtual void      InitWorldEffects();
   virtual void      InitWeapons();
   virtual void      InitView();
   virtual void      InitModel();
   virtual void      InitState();
   virtual void      InitHealth();
   virtual void      InitInventory();
   virtual void      InitSkin();
   virtual void      ChooseSpawnPoint();

   virtual void      EndLevel(Event *ev);
   virtual void      Respawn(Event *ev);

   virtual void      SetDeltaAngles();

   virtual void      DoUse();
   virtual void      Obituary(Entity *attacker, Entity *inflictor, str location, int meansofdeath);
   virtual void      Killed(Event *ev);
   virtual void      Dead(Event *ev);
   virtual void      Pain(Event *ev);
   virtual void      Spectator(Event *ev);

   virtual void      CheckButtons();
   virtual void      TouchStuff(pmove_t *pm);
   void              EnterVehicle(Event *ev);
   void              ExitVehicle(Event *ev);

   virtual void      GetMoveInfo(pmove_t *pm);
   virtual void      SetMoveInfo(pmove_t *pm, usercmd_t *ucmd);
   virtual pmtype_t  GetMovePlayerMoveType();
   virtual void      ClientMove(usercmd_t *ucmd);
   void              ClientThink(Event *ev);

   virtual void      EventUseItem(Event *ev);

   void              GiveCheat(Event *ev);
   void              GiveAllCheat(Event *ev);
   void              Take(Event *ev);
   void              GodCheat(Event *ev);
   void              NoTargetCheat(Event *ev);
   void              NoclipCheat(Event *ev);
   void              Kill(Event *ev);
   void              GibEvent(Event *ev);
   void              SpawnEntity(Event *ev);
   void              SpawnActor(Event *ev);

   void              EventPreviousWeapon(Event *ev);
   void              EventNextWeapon(Event *ev);
   void              EventPreviousItem(Event *ev);
   void              EventNextItem(Event *ev);
   void              EventUseInventoryItem(Event *ev);
   void              GameVersion(Event *ev);
   void              Fov(Event *ev);
   void              SaveFov(Event *ev);
   void              RestoreFov(Event *ev);
   void              ToggleViewMode(Event *ev);
   void              ToggleZoomMode(Event *ev);
   void              ZoomOut(Event *ev);
   void              CrosshairLayout(Entity *ent);
   void              EnterConsole(Event *ev);
   void              ExitConsole(Event *ev);
   void              KickConsole(Event *ev);
   virtual void      WeaponUse(Event *ev);
   virtual float     CalcRoll();
   virtual void      CalcBob();
   virtual void      CalcViewOffset();
   virtual void      CalcGunOffset();
   virtual void      CheckWater();
   virtual void      WorldEffects();
   virtual void      AddBlend(float r, float g, float b, float a);
   virtual void      CalcBlend();
   virtual void      DamageFeedback();
   void              SetGrapplePull(Vector dir, float speed);
   void              ClearGrapplePull();
   virtual void      ChooseAnim();

   virtual qboolean  CanMoveTo(Vector pos);
   virtual qboolean  ClearPathTo(Vector pos);

   virtual void      AddPathNode(Event *ev);
   virtual void      AddPathNodes();

   virtual void      UpdateStats();
   virtual void      UpdateMusic();

   virtual void      SetViewMode(viewmode_t mode, Entity * newCamera = NULL);
   virtual void      SetCameraValues(Vector position, Vector cameraoffset, Vector ang, Vector camerakick, Vector vel, 
                                     float camerablend[4], float camerafov);

   virtual void      SetCameraEntity(Entity *cameraEnt);
   virtual void      FallingDamage();
   virtual void      FinishMove();
   virtual void      EndFrame(Event *ev);

   virtual void      ShowInfo(Event *ev);

   virtual void      ReadyToFire(Event *ev);
   virtual void      WaitingToFire(Event *ev);
   virtual void      DoneFiring(Event *ev);

   virtual void      TestThread(Event *ev);
   virtual Weapon   *useWeapon(const char *weaponname);

   virtual void        AddItemToFloatingInventory(Item *item);
   virtual void        SetFloatingOwner(Sentient *deceased_owner);
   virtual Sentient   *GetFloatingOwner();
   virtual void        SendFloatingInventory();
   virtual void        ClearFloatingInventory(Event *ev);
   virtual void        PickupFloatingInventory();
   virtual void        ChangeSpectator();
   virtual const char *AnimPrefixForWeapon();
   virtual void        GotKill(Event *ev);
   virtual VehiclePtr  GetVehicle() { return vehicle; };
   virtual void        SetPowerupTimer(Event *ev);
   virtual void        UpdatePowerupTimer(Event *ev);

   virtual viewmode_t ViewMode();
   virtual Camera    *CurrentCamera();
   virtual void       SetCamera(Entity * ent);
   void               GetPlayerView(Vector *pos, Vector *angle);

   void              WhatIs(Event *ev);
   void              ActorInfo(Event *ev);
   virtual void      Taunt(Event *ev);
   str               AnimPrefixForPlayer();

   virtual void      DrawOverlay(Event *ev);
   virtual void      HideOverlay(Event *ev);
   virtual void      HideStats(Event *ev);
   virtual void      DrawStats(Event *ev);
   virtual void      ChangeMusic(const char * current, const char * fallback, qboolean force);
   virtual void      GravityNodes();
   virtual void      Archive(Archiver &arc)   override;
   virtual void      Unarchive(Archiver &arc) override;

   virtual void      SetFlashColor(Event *ev);
   virtual void      ClearFlashColor(Event *ev);
   void              Mutate(Event *ev);
   void              Human(Event *ev);
   void              SetSkin(Event *ev);
   void              GiveOxygen(float time);

   void              KillEnt(Event *ev);
   void              RemoveEnt(Event *ev);
   void              KillClass(Event *ev);
   void              RemoveClass(Event *ev);

   // CTF
   void              InitCTF();
   void              CTF_JoinTeamHardcorps(Event *ev);
   void              CTF_JoinTeamSintek(Event *ev);
   void              CTF_JoinTeam(int desired_team);
   void              CTF_Team(Event *ev);
   void              CTF_ExtendGrapple();
   void              CTF_ReleaseGrapple();
   void              CTF_DeadDropFlag();
   void              CTF_UpdateHookBeam(Event *ev);
   Vector            CTF_GetGrapplePosition();
   void              CTF_AssignTeam();
   void              CTF_OpenJoinMenu();
   void              CTF_UpdateNumberOfPlayers();
   void              CTF_DeadDropTech();
   void              CTF_DropTech(Event *ev);
   void              CTF_SoundEvent(Event *ev);
   void              SetViewAngles(Vector ang);
   void              DropWeaponEvent(Event *ev);
   void              CTF_DropFlag(Event *ev);
};

inline EXPORT_FROM_DLL void Player::Archive(Archiver &arc)
{
   Sentient::Archive(arc);

   arc.WriteRaw(&old_pmove, sizeof(old_pmove));

   // save bobbing variables
   arc.WriteFloat(xyspeed);
   arc.WriteFloat(bobtime);
   arc.WriteFloat(bobmove);
   arc.WriteInteger(bobcycle);
   arc.WriteFloat(bobfracsin);

   arc.WriteVector(oldvelocity);

   // save weapon kicks, damage kicks, view drop, and blend weapon kicks
   arc.WriteVector(kick_angles);
   arc.WriteVector(kick_origin);

   // damage kicks
   arc.WriteFloat(v_dmg_roll);
   arc.WriteFloat(v_dmg_pitch);
   arc.WriteFloat(v_dmg_time);

   // for view drop on fall
   arc.WriteFloat(fall_time);
   arc.WriteFloat(fall_value);

   // blend //FIXME move to entity?
   arc.WriteFloat(blend[0]);
   arc.WriteFloat(blend[1]);
   arc.WriteFloat(blend[2]);
   arc.WriteFloat(blend[3]);

   arc.WriteFloat(fov);
   arc.WriteFloat(savedfov);

   // save off v_gunangles, v_gunoffset for view weapon orientation
   arc.WriteVector(v_gunoffset);
   arc.WriteVector(v_gunangles);

   arc.WriteSafePointer(vehicle);
   arc.WriteString(vehicleanim);

   arc.WriteVector(v_angle);
   arc.WriteVector(v_kick);
   arc.WriteVector(oldviewangles);
   arc.WriteVector(v_offset);

   arc.WriteInteger(buttons);
   arc.WriteInteger(new_buttons);
   arc.WriteInteger(viewmode);
   arc.WriteInteger(defaultViewMode);
   arc.WriteInteger(zoom_mode);
   arc.WriteFloat(respawn_time);

   arc.WriteBoolean(firing);
   arc.WriteBoolean(in_console);
   arc.WriteBoolean(has_thought);

   arc.WriteFloat(damage_blood);
   arc.WriteFloat(damage_alpha);
   arc.WriteVector(damage_blend);
   arc.WriteFloat(bonus_alpha);

   arc.WriteFloat(next_drown_time);
   arc.WriteFloat(air_finished);

   arc.WriteInteger(old_waterlevel);
   arc.WriteInteger(drown_damage);

   arc.WriteObjectPointer(path);
   arc.WriteSafePointer(goalNode);
   arc.WriteSafePointer(nearestNode);
   arc.WriteSafePointer(lastNode);
   arc.WriteVector(lastVisible);
   arc.WriteVector(lastGround);
   arc.WriteFloat(searchTime);
   arc.WriteString(con_name);

   arc.WriteSafePointer(thirdpersonCamera);
   arc.WriteSafePointer(watchCamera);
   arc.WriteSafePointer(movieCamera);
   arc.WriteSafePointer(spectatorCamera);
   arc.WriteSafePointer(CinematicCamera);
   arc.WriteSafePointer(crosshair);

   arc.WriteBoolean(onladder);

   arc.WriteBoolean(spectator);
   arc.WriteBoolean(hidestats);
   arc.WriteBoolean(drawoverlay);

   arc.WriteFloat(action_level);
   arc.WriteInteger(music_current_mood);
   arc.WriteInteger(music_fallback_mood);

   // Can't archive this stuff
   // fallsurface
   // Pickup inventory stuff
   // SentientPtr       floating_owner;
   //Container<int>		floating_inventory;

   arc.WriteFloat(lastEnemyTime);

   arc.WriteInteger(currentCameraTarget);
   arc.WriteBoolean(gibbed);
   arc.WriteBoolean(drawstats);

   arc.WriteFloat(lastTauntTime);
   arc.WriteBoolean(checked_dead_camera);

   arc.WriteBoolean(falling);

   arc.WriteFloat(flash_color[0]);
   arc.WriteFloat(flash_color[1]);
   arc.WriteFloat(flash_color[2]);
   arc.WriteFloat(flash_color[3]);

   arc.WriteFloat(last_damage_time);
   arc.WriteBoolean(music_forced);

   arc.WriteBoolean(trappedInQuantum);
}

inline EXPORT_FROM_DLL void Player::Unarchive(Archiver &arc)
{
   int temp;

   Sentient::Unarchive(arc);

   arc.ReadRaw(&old_pmove, sizeof(old_pmove));

   arc.ReadFloat(&xyspeed);
   arc.ReadFloat(&bobtime);
   arc.ReadFloat(&bobmove);
   arc.ReadInteger(&bobcycle);
   arc.ReadFloat(&bobfracsin);

   arc.ReadVector(&oldvelocity);

   // save weapon kicks, damage kicks, view drop, and blend weapon kicks
   arc.ReadVector(&kick_angles);
   arc.ReadVector(&kick_origin);

   // damage kicks
   arc.ReadFloat(&v_dmg_roll);
   arc.ReadFloat(&v_dmg_pitch);
   arc.ReadFloat(&v_dmg_time);

   // for view drop on fall
   arc.ReadFloat(&fall_time);
   arc.ReadFloat(&fall_value);

   // blend //FIXME move to entity?
   arc.ReadFloat(&blend[0]);
   arc.ReadFloat(&blend[1]);
   arc.ReadFloat(&blend[2]);
   arc.ReadFloat(&blend[3]);

   arc.ReadFloat(&fov);
   arc.ReadFloat(&savedfov);

   // save off v_gunangles, v_gunoffset for view weapon orientation
   arc.ReadVector(&v_gunoffset);
   arc.ReadVector(&v_gunangles);

   arc.ReadSafePointer(&vehicle);
   arc.ReadString(&vehicleanim);

   arc.ReadVector(&v_angle);
   arc.ReadVector(&v_kick);
   arc.ReadVector(&oldviewangles);
   arc.ReadVector(&v_offset);

   arc.ReadInteger(&buttons);
   arc.ReadInteger(&new_buttons);

   arc.ReadInteger(&temp);
   viewmode = (viewmode_t)temp;

   arc.ReadInteger(&temp);
   defaultViewMode = (viewmode_t)temp;

   arc.ReadInteger(&temp);
   zoom_mode = (zoom_mode_t)temp;

   arc.ReadFloat(&respawn_time);

   arc.ReadBoolean(&firing);
   arc.ReadBoolean(&in_console);
   arc.ReadBoolean(&has_thought);

   arc.ReadFloat(&damage_blood);
   arc.ReadFloat(&damage_alpha);
   arc.ReadVector(&damage_blend);
   arc.ReadFloat(&bonus_alpha);

   arc.ReadFloat(&next_drown_time);
   arc.ReadFloat(&air_finished);

   arc.ReadInteger(&old_waterlevel);
   arc.ReadInteger(&drown_damage);

   arc.ReadObjectPointer((Class **)&path);
   arc.ReadSafePointer(&goalNode);
   arc.ReadSafePointer(&nearestNode);
   arc.ReadSafePointer(&lastNode);
   arc.ReadVector(&lastVisible);
   arc.ReadVector(&lastGround);
   arc.ReadFloat(&searchTime);
   arc.ReadString(&con_name);

   arc.ReadSafePointer(&thirdpersonCamera);
   arc.ReadSafePointer(&watchCamera);
   arc.ReadSafePointer(&movieCamera);
   arc.ReadSafePointer(&spectatorCamera);
   arc.ReadSafePointer(&CinematicCamera);
   arc.ReadSafePointer(&crosshair);

   arc.ReadBoolean(&onladder);

   arc.ReadBoolean(&spectator);
   arc.ReadBoolean(&hidestats);
   arc.ReadBoolean(&drawoverlay);

   arc.ReadFloat(&action_level);
   arc.ReadInteger(&music_current_mood);
   arc.ReadInteger(&music_fallback_mood);

   // Can't archive this stuff
   fallsurface = NULL;
   floating_owner = NULL;
   floating_inventory.FreeObjectList();

   arc.ReadFloat(&lastEnemyTime);

   arc.ReadInteger(&currentCameraTarget);
   arc.ReadBoolean(&gibbed);
   arc.ReadBoolean(&drawstats);

   arc.ReadFloat(&lastTauntTime);
   arc.ReadBoolean(&checked_dead_camera);

   arc.ReadBoolean(&falling);

   arc.ReadFloat(&flash_color[0]);
   arc.ReadFloat(&flash_color[1]);
   arc.ReadFloat(&flash_color[2]);
   arc.ReadFloat(&flash_color[3]);

   arc.ReadFloat(&last_damage_time);
   arc.ReadBoolean(&music_forced);

   arc.ReadBoolean(&trappedInQuantum);
}

inline EXPORT_FROM_DLL str Player::AnimPrefixForPlayer(void)
{
   str prefix;

   if(waterlevel > 2 || (!groundentity && waterlevel))
   {
      prefix = str("swim_");
   }
   else if(client && client->ps.pmove.pm_flags & PMF_DUCKED)
   {
      prefix = str("crouch_");
   }
   return prefix;
}

#endif /* player.h */

// EOF

