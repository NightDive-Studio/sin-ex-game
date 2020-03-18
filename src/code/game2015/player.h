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
#include "movecapture.h" //###
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
//###
extern Event EV_Player_WeaponSwitch;
extern Event EV_Player_WeaponOverride;
extern Event EV_Player_SetAngleJitter;
extern Event EV_Player_SetOffsetJitter;
extern Event EV_Player_CTF_SpawnRune;
//###
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
   CAMERA_VIEW,
   MISSILE_VIEW  //### added for guided missile
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
   //###
   friend class MoveCapture;
   //###

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

   //###
   // added for nuke view blend
   Vector            nuke_blend;
   float             nuke_alpha;

   // added for view jittering
   float             jitter_angle;
   float             jitter_angle_falloff;
   float             jitter_angle_enttime;
   float             jitter_offset;
   float             jitter_offset_falloff;
   float             jitter_offset_enttime;

   // movement capture thinggy
   MoveCapturePtr    movecapturer;
   //###

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
   virtual void      GravityNodes(void);
   virtual void      Archive(Archiver &arc);
   virtual void      Unarchive(Archiver &arc);

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

   //### 2015 added stuff
   // init 2015 stuff for player
   virtual void      Init2015();

   // added so I can detect player's buttons from other entities
   virtual int       Buttons() { return buttons; }

   // added for nuke view flash
   virtual void      StartNukeFlash(Vector fblend, float falpha);

   // added for concussion gun
   virtual void      ConcussionDamage();
   float             concussion_timer; // timer for cuncussioner removal

   // spawn hoverbike on player
   virtual void      SpawnPlayerBike(int forcespawn);

   // spawn hoverbike cheat
   virtual void      GiveBikeCheat(Event *ev);

   // added stuff for view jitter
   virtual float     JitterAngle() { return jitter_angle; }
   virtual void      SetAngleJitter(float angle, float falloff, float time);
   virtual float     JitterOffset() { return jitter_offset; }
   virtual void      SetOffsetJitter(float offset, float falloff, float time);
   virtual void      AngleJitterEvent(Event *ev);
   virtual void      OffsetJitterEvent(Event *ev);

   // added for checkpoints
   float             checkpoint_debounce;
   str               cp_list;
   int               last_cp_id;
   int               last_goal_time;
   virtual qboolean  CPTouched(int cp);
   virtual qboolean  CPTouchedAll();
   virtual void      CPListAdd(int cp);
   virtual void      CPListClear();

   // command to set bike skin
   void              SetBikeSkin(Event *ev);

   // set self to be informer
   void              SetInformer(Event *ev);

   // set the player's movement capture entity
   virtual void      SetMovementCapture(MoveCapture *capturer);

   // therm-optic goggles stuff
   qboolean          nightvision; // true if using goggles
   virtual void      ToggleGoggles(Event *ev);

   // guided missile overlay
   void              MissileOverlayOn();
   void              MissileOverlayOff();

   // for togglable auto weapon switching
   virtual void      WeaponSwitch(Event *ev);

   // for dissabling weapon select override
   virtual void      WeaponOverride(Event *ev);

   // get player's xyspeed
   virtual float     GetXYspeed() { return xyspeed; }

   // get the player's aiming direction
   Vector            GetVAngle() { return v_angle; }

   // for switching back to a regular weapon from the informergun
   WeaponPtr         lastWeapon;

   // MDF welcome message
   void              WelcomeMessage(Event *ev);

   // misc stuff for hoverbike
   Entity           *WatchCamera() { return watchCamera; }

   int               IsHoldingAttack() { return (buttons & BUTTON_ATTACK); }

   // redefinitions of sentient virtual functions for the hoverbike
   virtual int       IsFiringMissile()         override { return viewmode == MISSILE_VIEW; }
   virtual int       BikeIsTurboing()          override { return (buttons & BUTTON_USE);   }
   virtual void      SetBikeAnim(str bikeanim) override { vehicleanim = bikeanim;          }
   virtual float     WishedBikeYaw()           override { return v_angle[YAW];             }
   virtual float     BikeAimPitch()            override { return v_angle[PITCH] * 1.05;    } // need to scale the player's view pitch up to compensate for the 
                                                                                             // weapons firing from so far forward.
   void              CTF_SpawnRuneSelect(Event *ev);
   void              CTF_SetSpawnRune(Event *ev);
   void              CTF_GiveRune(int runenum);
   //###

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
   void              CTF_DrawHud(); //###
   void              CTF_HideHud(); //###
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
   
   //###
   arc.WriteVector(nuke_blend);
   arc.WriteFloat(nuke_alpha);

   arc.WriteFloat(jitter_angle);
   arc.WriteFloat(jitter_angle_falloff);
   arc.WriteFloat(jitter_angle_enttime);
   arc.WriteFloat(jitter_offset);
   arc.WriteFloat(jitter_offset_falloff);
   arc.WriteFloat(jitter_offset_enttime);

   arc.WriteSafePointer(movecapturer);

   arc.WriteFloat(concussion_timer);

   arc.WriteFloat(checkpoint_debounce);
   arc.WriteString(cp_list);
   arc.WriteInteger(last_cp_id);
   arc.WriteInteger(last_goal_time);
   arc.WriteBoolean(nightvision);

   arc.WriteSafePointer(lastWeapon);
   //###
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

   //###
   arc.ReadVector( &nuke_blend );
   arc.ReadFloat(&nuke_alpha);

    arc.ReadFloat(&jitter_angle);
    arc.ReadFloat(&jitter_angle_falloff);
    arc.ReadFloat(&jitter_angle_enttime);
    arc.ReadFloat(&jitter_offset);
    arc.ReadFloat(&jitter_offset_falloff);
    arc.ReadFloat(&jitter_offset_enttime);

   arc.ReadSafePointer( &movecapturer );

   arc.ReadFloat(&concussion_timer);

   arc.ReadFloat(&checkpoint_debounce);
   arc.ReadString(&cp_list);
   arc.ReadInteger(&last_cp_id);
   arc.ReadInteger(&last_goal_time);
   arc.ReadBoolean(&nightvision);

   arc.ReadSafePointer(&lastWeapon);
   //###
}

inline EXPORT_FROM_DLL str Player::AnimPrefixForPlayer()
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

