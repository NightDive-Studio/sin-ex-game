//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/ctf.h                            $
// $Revision:: 11                                                             $
//   $Author:: Aldie                                                          $
//     $Date:: 5/18/99 7:07p                                                  $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Game code for Threewave Capture the Flag.
//
// The original source for this code was graciously provided by Zoid and
// Id Software.  Many thanks!
//
// Original credits:
//
// Programming             - Dave 'Zoid' Kirsch
// Original CTF Art Design - Brian 'Whaleboy' Cozzens
// 

#ifndef __CTF_H__
#define __CTF_H__

#include "g_local.h"
#include "inventoryitem.h"
#include "specialfx.h"
#include "player.h"

#define CTF_VERSION			1.00
#define CTF_VSTRING2(x)    #x
#define CTF_VSTRING(x)     CTF_VSTRING2(x)
#define CTF_STRING_VERSION CTF_VSTRING(CTF_VERSION)

#define STAT_CTF_TECH               7
#define STAT_CTF_ID_VIEW            8
#define STAT_CTF_TEAM1_PIC          11
#define STAT_CTF_TEAM1_CAPS         12
#define STAT_CTF_TEAM2_PIC          13
#define STAT_CTF_TEAM2_CAPS         14
#define STAT_CTF_FLAG_PIC           15
#define STAT_CTF_JOINED_TEAM1_PIC   16
#define STAT_CTF_JOINED_TEAM2_PIC   17
#define STAT_CTF_TEAM1_HEADER       18
#define STAT_CTF_TEAM2_HEADER       19

typedef enum
{
   CTF_NOTEAM,
   CTF_TEAM_HARDCORPS,
   CTF_TEAM_SINTEK
} ctfteam_t;

typedef enum
{
   CTF_STATE_START,
   CTF_STATE_PLAYING
} ctfstate_t;

typedef enum
{
   CTF_GRAPPLE_STATE_FLY,
   CTF_GRAPPLE_STATE_PULL,
   CTF_GRAPPLE_STATE_HANG
} ctfgrapplestate_t;

typedef enum
{
   FIRE,
   DAMAGE
} ctfsoundevent_t;

typedef struct ctfgame_s
{
   int   team_hardcorps;
   int   team_sintek;
   int   total_hardcorps, total_sintek; // these are only set when going into intermission!
   float last_flag_capture;
   int   last_capture_team;
} ctfgame_t;

extern ctfgame_t ctfgame;

extern cvar_t *ctf;
extern cvar_t *ctf_hardcorps_skin;
extern cvar_t *ctf_sintek_skin;
extern cvar_t *ctf_forcejoin;

#define CTF_CAPTURE_BONUS        15 // what you get for capture
#define CTF_TEAM_BONUS           10 // what your team gets for capture
#define CTF_RECOVERY_BONUS       1  // what you get for recovery
#define CTF_FLAG_BONUS           0  // what you get for picking up enemy flag
#define CTF_FRAG_CARRIER_BONUS   2  // what you get for fragging enemy flag carrier
#define CTF_FLAG_RETURN_TIME     40 // seconds until auto return

#define CTF_CARRIER_DANGER_PROTECT_BONUS 2 // bonus for fraggin someone who has recently hurt your flag carrier
#define CTF_CARRIER_PROTECT_BONUS        1 // bonus for fraggin someone while either you or your target are near your flag carrier
#define CTF_FLAG_DEFENSE_BONUS           1 // bonus for fraggin someone while either you or your target are near your flag
#define CTF_RETURN_FLAG_ASSIST_BONUS     1 // awarded for returning a flag that causes a capture to happen almost immediately
#define CTF_FRAG_CARRIER_ASSIST_BONUS    2 // award for fragging a flag carrier if a capture happens almost immediately

#define CTF_TARGET_PROTECT_RADIUS        400 // the radius around an object being defended where a target will be worth extra frags
#define CTF_ATTACKER_PROTECT_RADIUS      400 // the radius around an object being defended where an attacker will get extra frags when making kills

#define CTF_CARRIER_DANGER_PROTECT_TIMEOUT   8
#define CTF_FRAG_CARRIER_ASSIST_TIMEOUT      10
#define CTF_RETURN_FLAG_ASSIST_TIMEOUT       10
#define CTF_AUTO_FLAG_RETURN_TIMEOUT         30   // number of seconds before dropped flag auto-returns
#define CTF_DROPPED_FLAG_RETURN_TIMEOUT      10   // number of seconds before dropped flag returns
#define CTF_TECH_TIMEOUT                     60   // seconds before techs spawn again
#define CTF_GRAPPLE_SPEED                    1000 // speed of grapple in flight
#define CTF_GRAPPLE_PULL_SPEED               700  // speed player is pulled at
#define CTF_TECH_REGENERATION_HEALTH         300  // max health with regeneration
#define CTF_TECH_REGENERATION_TIME           0.7  // time between regenerations

// FLAGS

class EXPORT_FROM_DLL CTF_Flag : public InventoryItem
{
protected:
   ctfteam_t   ctf_team;

   virtual void PickupFlag(Event *ev);
   virtual void ResetFlag(Event *ev);

public:
   qboolean    limp = false;

   CLASS_PROTOTYPE(CTF_Flag);
   CTF_Flag();
};

class EXPORT_FROM_DLL CTF_Flag_Hardcorps : public CTF_Flag
{
public:
   CLASS_PROTOTYPE(CTF_Flag_Hardcorps);
   CTF_Flag_Hardcorps();
};

class EXPORT_FROM_DLL CTF_Flag_Sintek : public CTF_Flag
{
public:
   CLASS_PROTOTYPE(CTF_Flag_Sintek);
   CTF_Flag_Sintek();
};

// TECH

class EXPORT_FROM_DLL CTF_Tech : public InventoryItem
{
protected:
   Vector            FindSpawnLocation(void);
   void              Timeout(Event *ev);
   virtual void      Pickup(Event *ev);
   virtual qboolean  Pickupable(Entity *other);
   void              HasTechMsg(edict_t *who);

public:
   CLASS_PROTOTYPE(CTF_Tech);
   CTF_Tech();
};

class EXPORT_FROM_DLL CTF_Tech_Regeneration : public CTF_Tech
{
protected:
   void              Regenerate(Event *ev);
   float             last_sound_event;

public:
   CLASS_PROTOTYPE(CTF_Tech_Regeneration);
   CTF_Tech_Regeneration();
};

class EXPORT_FROM_DLL CTF_Tech_Double : public CTF_Tech
{
public:
   CLASS_PROTOTYPE(CTF_Tech_Double);
   CTF_Tech_Double();
};

class EXPORT_FROM_DLL CTF_Tech_Shield : public CTF_Tech
{
public:
   CLASS_PROTOTYPE(CTF_Tech_Shield);
   CTF_Tech_Shield();
};

class EXPORT_FROM_DLL CTF_Tech_Aqua : public CTF_Tech
{
public:
   CLASS_PROTOTYPE(CTF_Tech_Aqua);
   CTF_Tech_Aqua();
};

class EXPORT_FROM_DLL CTF_Tech_Jump : public CTF_Tech
{
public:
   CLASS_PROTOTYPE(CTF_Tech_Jump);
   CTF_Tech_Jump();
};

class EXPORT_FROM_DLL CTF_Tech_Empathy : public CTF_Tech
{
public:
   CLASS_PROTOTYPE(CTF_Tech_Empathy);
   CTF_Tech_Empathy();
};

class EXPORT_FROM_DLL CTF_Tech_DeathQuad : public CTF_Tech
{
private:
   float             last_sound_event;

public:
   CLASS_PROTOTYPE(CTF_Tech_DeathQuad);
   CTF_Tech_DeathQuad();
   void Damage(Event *ev);
};

void     CTF_Init(void);
char     *CTF_TeamName(int team);
char     *CTF_OtherTeamName(int team);
int      CTF_OtherTeam(int team);
void     CTF_UpdateStats(Player *player);
void     CTF_ScoreboardMessage(Entity *ent, Entity *killer);
qboolean CTF_CheckRules(void);
void     CTF_CheckTeams(void);
Entity   *SelectCTFSpawnPoint(edict_t *ent);
void     CTF_InitFlags(void);
void     CTF_FragBonuses(Sentient *targ, Sentient *attacker);
void     CTF_CheckHurtCarrier(Entity *targ, Entity *attacker);
void     CTF_CalcScores(void);

extern Event EV_Flag_Reset;
extern Event EV_Tech_Timeout;

#endif /* ctf.h */

// EOF

