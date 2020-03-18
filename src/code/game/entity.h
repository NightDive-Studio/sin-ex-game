//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/entity.h                         $
// $Revision:: 163                                                            $
//   $Author:: Aldie                                                          $
//     $Date:: 3/19/99 4:12p                                                  $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Base class for all enities that are controlled by Sin.  If you have any
// object that should be called on a periodic basis and it is not an entity,
// then you have to have an dummy entity that calls it.
//
// An entity in Sin is any object that is not part of the world.  Any non-world
// object that is visible in Sin is an entity, although it is not required that
// all entities be visible to the player.  Some objects are basically just virtual
// constructs that act as an instigator of certain actions, for example, some 
// triggers are invisible and cannot be touched, but when activated by other
// objects can cause things to happen.
//
// All entities are capable of receiving messages from Sin or from other entities.
// Messages received by an entity may be ignored, passed on to their superclass,
// or acted upon by the entity itself.  The programmer must decide on the proper
// action for the entity to take to any message.  There will be many messages
// that are completely irrelevant to an entity and should be ignored.  Some messages
// may require certain states to exist and if they are received by an entity when
// it these states don't exist may indicate a logic error on the part of the 
// programmer or map designer and should be reported as warnings (if the problem is
// not severe enough for the game to be halted) or as errors (if the problem should 
// not be ignored at any cost).
// 

#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "g_local.h"
#include "class.h"
#include "vector.h"
#include "script.h"
#include "listener.h"

#include <float.h>

typedef enum
{
   DAMAGE_NO,
   DAMAGE_YES,  // will take damage if hit
   DAMAGE_AIM   // auto targeting recognizes this
} damage_t;

//deadflag
#define DEAD_NO          0
#define DEAD_DYING       1
#define DEAD_DEAD        2
#define DEAD_RESPAWNABLE 3

// flags
#define  FL_FLY               0x00000001
#define  FL_SWIM              0x00000002  // implied immunity to drowining
#define  FL_INWATER           0x00000004
#define  FL_GODMODE           0x00000008
#define  FL_NOTARGET          0x00000010
#define  FL_PARTIALGROUND     0x00000020  // not all corners are valid
#define  FL_FATPROJECTILE     0x00000040  // projectile should use fat trace
#define  FL_TEAMSLAVE         0x00000080  // not the first on the team
#define  FL_NO_KNOCKBACK      0x00000100
#define  FL_PRETHINK          0x00000200
#define  FL_POSTTHINK         0x00000400
#define  FL_BLOOD             0x00000800
#define  FL_SPARKS            0x00001000
#define  FL_TESSELATE         0x00002000
#define  FL_BLASTMARK         0x00004000
#define  FL_DIE_TESSELATE     0x00008000
#define  FL_DARKEN            0x00010000
#define  FL_DIE_GIBS          0x00020000
#define  FL_SHIELDS           0x00040000  // sentient has reactive shields
#define  FL_DIE_EXPLODE       0x00080000  // when it dies, it will explode
#define  FL_ADRENALINE        0x00100000  // sentient is under adrenaline effects
#define  FL_CLOAK             0x00200000  // sentient is cloaked
#define  FL_ROTATEDBOUNDS     0x00400000  // model uses rotated mins and maxs
#define  FL_MUTANT            0x00800000  // sentient is in mutant mode
#define  FL_OXYGEN            0x01000000  // sentient has oxygen powerup
#define  FL_SILENCER          0x02000000  // sentient has silencer
#define  FL_SP_MUTANT         0x04000000  // mutant mode single player
#define  FL_MUTATED           0x08000000  // keep track of mutation
#define  FL_FORCEFIELD        0x10000000  // sentient has force field ( invulnerable )
#define  FL_DONTSAVE          0x20000000  // don't add to the savegame
#define  FL_STEALTH           0x40000000  // character is in "stealth" mode
#define  FL_NOION             0x80000000  // don't allow Ion tesselation

// damage flags
#define DAMAGE_RADIUS         0x00000001  // damage was indirect
#define DAMAGE_NO_ARMOR       0x00000002  // armour does not protect from this damage
#define DAMAGE_ENERGY         0x00000004  // damage is from an energy based weapon
#define DAMAGE_NO_KNOCKBACK   0x00000008  // do not affect velocity, just view angles
#define DAMAGE_BULLET         0x00000010  // damage is from a bullet (used for ricochets)
#define DAMAGE_NO_PROTECTION  0x00000020  // armor, shields, invulnerability, and godmode have no effect
#define DAMAGE_NO_SKILL       0x00000040  // damage is not affected by skill level

//
// Sound travel distances
//

#define SOUND_BREAKING_RADIUS 500
#define SOUND_WEAPON_RADIUS   800
#define SOUND_MOVEMENT_RADIUS 256
#define SOUND_PAIN_RADIUS     320
#define SOUND_DEATH_RADIUS    800
#define SOUND_DOOR_RADIUS     240
#define SOUND_MUTANT_RADIUS   256
#define SOUND_VOICE_RADIUS    800
#define SOUND_MACHINE_RADIUS  512
#define SOUND_RADIO_RADIUS    8192

extern Event EV_ClientConnect;
extern Event EV_ClientDisconnect;
extern Event EV_ClientKill;
extern Event EV_ClientMove;
extern Event EV_ClientEndFrame;

// Generic entity events
extern Event EV_Classname;
extern Event EV_Activate;
extern Event EV_Use;
//extern Event EV_Footstep;
extern Event EV_FadeOut;
extern Event EV_Fade;
extern Event EV_Killed;
extern Event EV_GotKill;
extern Event EV_Pain;
extern Event EV_Damage;
extern Event EV_Gib;
extern Event EV_Mutate;

// Physics events
extern Event EV_MoveDone;
extern Event EV_Touch;
extern Event EV_Blocked;
extern Event EV_Attach;
extern Event EV_AttachModel;
extern Event EV_Detach;
extern Event EV_UseBoundingBox;

// Animation events
extern Event EV_NewAnim;
extern Event EV_LastFrame;
extern Event EV_TakeDamage;
extern Event EV_NoDamage;
extern Event EV_SetSkin;

// script stuff
extern Event EV_Hide;
extern Event EV_Show;
extern Event EV_BecomeSolid;
extern Event EV_BecomeNonSolid;
extern Event EV_PlaySound;
extern Event EV_StopSound;
extern Event EV_GravityAxis;
extern Event EV_Bind;
extern Event EV_Unbind;
extern Event EV_JoinTeam;
extern Event EV_QuitTeam;
extern Event EV_SetHealth;
extern Event EV_SetSize;
extern Event EV_SetAlpha;
extern Event EV_SetOrigin;
extern Event EV_SetTargetName;
extern Event EV_SetTarget;
extern Event EV_SetKillTarget;
extern Event EV_SetAngles;
extern Event EV_RegisterAlias;
extern Event EV_RandomSound;
extern Event EV_EntitySound;
extern Event EV_RandomEntitySound;
extern Event EV_RandomGlobalEntitySound;
extern Event EV_StopEntitySound;
extern Event EV_Anim;
extern Event EV_StartAnimating;
extern Event EV_GroupModelEvent;
extern Event EV_DialogEvent;
extern Event EV_RandomPHSSound;
extern Event EV_PHSSound;
extern Event EV_ProcessInitCommands;
// dir is 1
// power is 2
// minsize is 3
// maxsize is 4
// percentage is 5
// thickness 6
// entity is 7
// origin 8
extern Event EV_Tesselate;
extern Event EV_Shatter_MinSize;
extern Event EV_Shatter_MaxSize;
extern Event EV_Shatter_Thickness;
extern Event EV_Shatter_Percentage;

// AI sound events
extern Event EV_WeaponSound;
extern Event EV_MovementSound;
extern Event EV_PainSound;
extern Event EV_DeathSound;
extern Event EV_BreakingSound;
extern Event EV_DoorSound;
extern Event EV_MutantSound;
extern Event EV_VoiceSound;
extern Event EV_MachineSound;
extern Event EV_RadioSound;

extern Event EV_HeardWeapon;
extern Event EV_HeardMovement;
extern Event EV_HeardPain;
extern Event EV_HeardDeath;
extern Event EV_HeardBreaking;
extern Event EV_HeardDoor;
extern Event EV_HeardMutant;
extern Event EV_HeardVoice;
extern Event EV_HeardMachine;
extern Event EV_HeardRadio;
extern Event EV_Hurt;
extern Event EV_IfSkill;

// Define ScriptMaster
class ScriptMaster;

//
// Spawn args
//
// "spawnflags"
// "alpha" default 1.0
// "model"
// "origin"
// "targetname"
// "target"
//
#define MAX_MODEL_CHILDREN 8

class Entity;
#ifdef EXPORT_TEMPLATE
template class EXPORT_FROM_DLL SafePtr<Entity>;
#endif
typedef SafePtr<Entity> EntityPtr;

class EXPORT_FROM_DLL Entity : public Listener
{
public:
   CLASS_PROTOTYPE(Entity);

   // spawning variables
   int               entnum;
   edict_t          *edict;
   gclient_t        *client;
   const char       *classname;
   int               spawnflags;

   // rendering variables
   float             translucence;
   int               viewheight;    // height above origin where eyesight is determined
   int               light_level;   // keeps track of light level at origin

   // Animation variables
   str               model;
   int               next_anim;          // index of next_anim, if an anim change is pending,
                                         // this value is non-negative
   int               next_frame;         // index of next_frame, if a frame change is pending,
                                         // this value is non-negative
   int               last_frame_in_anim; // last frame in the current animation
   Vector            frame_delta;        // current movement from this frame
   Vector            total_delta;        // total unprocessed movement 
   Vector            next_anim_delta;    // total delta of next animation
   float             next_anim_time;     // total time of next animation
   qboolean          animating;          // whether the model is currently animating
   Event            *animDoneEvent;
   float             last_animation_time; // the last server frame this model was animated
   int               num_frames_in_gun_anim; // num frames in the gun animation, if there is one

   // physics variables
   Vector            mins;
   Vector            maxs;
   Vector            absmin;
   Vector            absmax;
   Vector            size;
   Vector            centroid;
   Vector            origin;
   Vector            velocity;
   Vector            avelocity;
   Vector            angles;
   Vector            worldorigin;
   Vector            worldangles;
   Vector            vieworigin;
   Vector            viewangles;
   int               contents;
   int               movetype;
   int               mass;
   float             gravity;  // per entity gravity multiplier (1.0 is normal)
   int               gravaxis; // per entity gravity axis

   edict_t          *groundentity;
   csurface_t       *groundsurface;
   cplane_t          groundplane;
   int               groundcontents;

   int               groundentity_linkcount;

   // Binding variables
   Entity           *bindmaster;
   str               moveteam;
   Entity           *teamchain;
   Entity           *teammaster;
   float             orientation[3][3];

   // Model Binding variables
   int               numchildren;
   int               children[MAX_MODEL_CHILDREN];

   // targeting variables
   str               target;
   str               targetname;
   str               killtarget;

   // Character state
   float             health;
   float             max_health;
   int               deadflag;
   int               flags;

   // underwater variables
   int               watertype;
   int               waterlevel;

   // Pain and damage variables
   damage_t          takedamage;
   EntityPtr         enemy;
   float             pain_finished;
   float             damage_debounce_time;

   // tesselation variables
   int               tess_min_size;
   int               tess_max_size;
   int               tess_thickness;
   float             tess_percentage;

   Entity();
   virtual ~Entity();

   void              SetEntNum(int num);
   void              GetEntName(Event *ev);

   qboolean          DistanceTo(const Vector &pos) const;
   qboolean          DistanceTo(const Entity *ent) const;
   qboolean          WithinDistance(const Vector &pos, float dist) const;
   qboolean          WithinDistance(const Entity *ent, float dist) const;

   const char       *Target() const;
   void              SetTarget(const char *target);
   qboolean          Targeted() const;
   const char       *TargetName() const;
   void              SetTargetName(const char *target);
   void              SetKillTarget(const char *killtarget);
   const char       *KillTarget() const;

   int               modelIndex(const char * mdl) const;
   virtual void      setModel(const char *model);
   virtual void      setModel(str &mdl);
   void              SetModelEvent(Event *ev);
   void              hideModel(void);
   void              EventHideModel(Event *ev);
   void              showModel(void);
   void              EventShowModel(Event *ev);
   qboolean          hidden() const;
   void              ProcessInitCommandsEvent(Event *ev);
   void              ProcessInitCommands(int index);

   void              setAlpha(float alpha);
   float             alpha() const;

   void              setMoveType(int type);
   int               getMoveType() const;

   void              setSolidType(solid_t type);
   int               getSolidType() const;

   Vector            getParentVector(const Vector &vec) const;
   Vector            getLocalVector(const Vector &vec) const;

   virtual void      setSize(Vector min, Vector max);
   void              setOrigin(Vector org);
   qboolean          GetBone(const char *name, Vector *pos, Vector *forward, Vector *right, Vector *up);
   void              setAngles(const Vector &ang);

   void              link();
   void              unlink();

   void              setContents(int type);
   int               getContents() const;
   void              setScale(float scale);

   qboolean          droptofloor(float maxfall);
   qboolean          isClient() const;

   virtual void      SetDeltaAngles();

   virtual void      DamageEvent(Event *event);
   virtual void      Damage(Entity *inflictor,
                            Entity *attacker,
                            int damage,
                            Vector position,
                            Vector direction,
                            Vector normal,
                            int knockback,
                            int flags,
                            int meansofdeath,
                            int groupnum,
                            int trinum,
                            float damage_multiplier);

   virtual qboolean  CanDamage(Entity *target);

   qboolean          IsTouching(const Entity *e1) const;
   void              NextAnim(int animnum);
   void              NextFrame(int framenum);
   void              AnimateFrame();
   void              StopAnimating();
   void              StartAnimating();
   void              RandomAnimate(const char *animname, Event *endevent);
   void              RandomAnimate(const char *animname, Event &endevent);
   qboolean          HasAnim(const char *animname) const;

   void              joinTeam(Entity *teammember);
   void              quitTeam();
   void              EventQuitTeam(Event *ev);
   qboolean          isBoundTo(Entity *master) const;
   void              bind(Entity *master);
   void              unbind();
   void              EventUnbind(Event *ev);

   void              FadeOut(Event *ev);
   void              Fade(Event *ev);

   virtual void      CheckGround();
   qboolean          HitSky(const trace_t *trace) const;
   qboolean          HitSky() const;

   void              BecomeSolid(Event *ev);
   void              BecomeNonSolid(Event *ev);
   void              PlaySound(Event *ev);
   void              StopSound(Event *ev);
   void              SetGravityAxis(int axis);
   void              GravityAxisEvent(Event *ev);
   void              BindEvent(Event *ev);
   void              JoinTeam(Event *ev);
   void              SetHealth(Event *ev);
   void              SetSize(Event *ev);
   void              SetScale(Event *ev);
   void              SetAlpha(Event *ev);
   void              SetOrigin(Event *ev);
   void              SetTargetName(Event *ev);
   void              SetTarget(Event *ev);
   void              SetKillTarget(Event *ev);
   void              SetAngles(Event *ev);

   void              CourseAnglesEvent(Event *ev);
   void              SmoothAnglesEvent(Event *ev);

   str               GetRandomAlias(const str &name) const;
   void              SetWaterType();

   // model binding functions
   qboolean          attach(int parent_entity_num, int group_num, int tri_num, Vector orient);
   void              detach();

   void              RegisterAlias(Event *ev);
   void              RegisterAliasAndCache(Event *ev);

   qboolean          GlobalAliasExists(const char *name) const;
   qboolean          AliasExists(const char *name) const;

   virtual void      positioned_sound(Vector origin, str soundname, float volume = 1.0f,
                                      int channel = CHAN_BODY, int attenuation = ATTN_NORM, float pitch = 1.0f,
                                      float timeofs = 0, float fadetime = 0, int flags = SOUND_SYNCH);

   virtual void      sound(str soundname, float volume = 1.0f, int channel = CHAN_BODY,
                           int attenuation = ATTN_NORM, float pitch = 1.0f, float timeofs = 0,
                           float fadetime = 0, int flags = SOUND_SYNCH);

   virtual void      stopsound(int channel);

   virtual void      RandomPositionedSound(Vector origin, str soundname, float volume = 1.0f,
                                           int channel = CHAN_BODY, int attenuation = ATTN_NORM, float pitch = 1.0f,
                                           float timeofs = 0, float fadetime = 0, int flags = SOUND_SYNCH);

   void              RandomSound(str soundname, float volume = 1.0f, int channel = CHAN_BODY,
                                 int attenuation = ATTN_NORM, float pitch = 1.0f, float timeofs = 0,
                                 float fadetime = 0, int flags = SOUND_SYNCH);

   void              RandomGlobalSound(str soundname, float volume = 1.0f, int channel = CHAN_BODY,
                                       int attenuation = ATTN_NORM, float pitch = 1.0f, float timeofs = 0,
                                       float fadetime = 0, int flags = SOUND_SYNCH);

   void              RandomGlobalEntitySound(str soundname, int attenuation = ATTN_IDLE);
   void              RandomGlobalEntitySoundEvent(Event *ev);

   void              RandomSound(Event *ev);
   void              EntitySound(Event *ev);
   void              StopEntitySound(Event *ev);
   void              RandomEntitySound(Event *ev);
   void              AnimEvent(Event *ev);
   void              StartAnimatingEvent(Event *ev);
   void              StopAnimatingEvent(Event *ev);
   void              EndAnimEvent(Event *ev);
   void              NextAnimEvent(Event *ev);
   void              NextFrameEvent(Event *ev);
   void              PrevFrameEvent(Event *ev);
   void              SetFrameEvent(Event *ev);
   void              SetLight(Event *ev);
   void              LightOn(Event *ev);
   void              LightOff(Event *ev);
   void              LightRed(Event *ev);
   void              LightGreen(Event *ev);
   void              LightBlue(Event *ev);
   void              LightRadius(Event *ev);
   void              Tesselate(Event *ev);
   void              SetShatterMinSize(Event *ev);
   void              SetShatterMaxSize(Event *ev);
   void              SetShatterThickness(Event *ev);
   void              SetShatterPercentage(Event *ev);
   void              Flags(Event *ev);
   void              Effects(Event *ev);
   void              RenderEffects(Event *ev);

   void              BroadcastSound(Event *soundevent, int channel, Event &event, float radius);
   void              WeaponSound(Event *ev);
   void              MovementSound(Event *ev);
   void              PainSound(Event *ev);
   void              DeathSound(Event *ev);
   void              BreakingSound(Event *ev);
   void              DoorSound(Event *ev);
   void              MutantSound(Event *ev);
   void              VoiceSound(Event *ev);
   void              MachineSound(Event *ev);
   void              RadioSound(Event *ev);
   void              SpawnParticles(Event *ev);
   void              Kill(Event *ev);
   void              GroupModelEvent(Event *ev);

   virtual void      Prethink();
   virtual void      Postthink();
   void              DamageSkin(trace_t * trace, float damage);
   virtual void      DialogEvent(Event *ev);
   void              PHSSound(Event *ev);
   void              RandomPHSSound(Event *ev);
   void              AttachEvent(Event *ev);
   void              AttachModelEvent(Event *ev);
   void              DetachEvent(Event *ev);
   void              TakeDamageEvent(Event *ev);
   void              NoDamageEvent(Event *ev);
   void              SetSkinEvent(Event *ev);
   void              Lightoffset(Event *ev);
   void              Gravity(Event *ev);
   void              Minlight(Event *ev);
   void              GiveOxygen(float time);
   void              UseBoundingBoxEvent(Event *ev);
   void              HurtEvent(Event *ev);
   void              IfSkillEvent(Event *ev);
   void              SetMassEvent(Event *ev);
   void              Censor(Event *ev);
   void              Ghost(Event *ev);

   virtual void      Archive(Archiver &arc)   override;
   virtual void      Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL qboolean Entity::DistanceTo(const Vector &pos) const
{
   return (worldorigin - pos).length();
}

inline EXPORT_FROM_DLL qboolean Entity::DistanceTo(const Entity *ent) const
{
   assert(ent);

   if(!ent)
   {
      // "Infinite" distance
      return 999999;
   }

   return (worldorigin - ent->worldorigin).length();
}

inline EXPORT_FROM_DLL qboolean Entity::WithinDistance(const Vector &pos, float dist) const
{
   Vector delta = worldorigin - pos;

   // check squared distance
   return ((delta * delta) < (dist * dist));
}

inline EXPORT_FROM_DLL qboolean Entity::WithinDistance(const Entity *ent, float dist) const
{
   assert(ent);

   if(!ent)
   {
      return false;
   }

   Vector delta = worldorigin - ent->worldorigin;

   // check squared distance
   return ((delta * delta) < (dist * dist));
}

inline EXPORT_FROM_DLL const char *Entity::Target() const
{
   return target.c_str();
}

inline EXPORT_FROM_DLL qboolean Entity::Targeted() const
{
   if(!targetname.length())
   {
      return false;
   }
   return true;
}

inline EXPORT_FROM_DLL const char *Entity::TargetName() const
{
   return targetname.c_str();
}

inline EXPORT_FROM_DLL const char * Entity::KillTarget() const
{
   return killtarget.c_str();
}

inline EXPORT_FROM_DLL qboolean Entity::hidden() const
{
   if(edict->s.renderfx & RF_DONTDRAW)
   {
      return true;
   }
   return false;
}

inline EXPORT_FROM_DLL void Entity::setModel(str &mdl)
{
   setModel(mdl.c_str());
}

inline EXPORT_FROM_DLL void Entity::SetModelEvent(Event *ev)
{
   setModel(ev->GetString(1));
}

inline EXPORT_FROM_DLL void Entity::hideModel(void)
{
   edict->s.renderfx |= RF_DONTDRAW;
   if(getSolidType() <= SOLID_TRIGGER)
   {
      edict->svflags |= SVF_NOCLIENT;
   }
}

inline EXPORT_FROM_DLL void Entity::showModel(void)
{
   edict->s.renderfx &= ~RF_DONTDRAW;
   edict->svflags &= ~SVF_NOCLIENT;
}

inline EXPORT_FROM_DLL float Entity::alpha() const
{
   return 1.0f - translucence;
}

inline EXPORT_FROM_DLL void Entity::setMoveType(int type)
{
   movetype = type;
}

inline EXPORT_FROM_DLL int Entity::getMoveType() const
{
   return movetype;
}

inline EXPORT_FROM_DLL int Entity::getSolidType() const
{
   return edict->solid;
}

inline EXPORT_FROM_DLL void Entity::unlink(void)
{
   gi.unlinkentity(edict);
}

inline EXPORT_FROM_DLL void Entity::setContents(int type)
{
   contents = type;
}

inline EXPORT_FROM_DLL int Entity::getContents() const
{
   return contents;
}

inline EXPORT_FROM_DLL qboolean Entity::isClient() const
{
   if(client)
   {
      return true;
   }
   return false;
}

inline EXPORT_FROM_DLL void Entity::SetDeltaAngles()
{
   int i;

   if(client)
   {
      for(i = 0; i < 3; i++)
      {
         client->ps.pmove.delta_angles[i] = ANGLE2SHORT(client->ps.viewangles[i]);
      }
   }
}

inline EXPORT_FROM_DLL void Entity::RandomAnimate(const char *animname, Event &endevent)
{
   Event *ev;

   ev = new Event(endevent);
   RandomAnimate(animname, ev);
}

inline EXPORT_FROM_DLL qboolean Entity::HasAnim(const char *animname) const
{
   int num = gi.Anim_Random(edict->s.modelindex, animname);
   return (num >= 0);
}

inline EXPORT_FROM_DLL qboolean Entity::GlobalAliasExists(const char *name) const
{
   assert(name);

   return (gi.GlobalAlias_FindRandom(name) != nullptr);
}

inline EXPORT_FROM_DLL qboolean Entity::AliasExists(const char *name) const
{
   assert(name);

   return (gi.Alias_FindRandom(edict->s.modelindex, name) != nullptr);
}

inline EXPORT_FROM_DLL void Entity::stopsound(int channel)
{
   RandomGlobalSound("null_sound", 0.1, channel, 0);
}

inline EXPORT_FROM_DLL str Entity::GetRandomAlias(const str &name) const
{
   str realname;
   const char *s;

   s = gi.Alias_FindRandom(edict->s.modelindex, name.c_str());
   if(s)
   {
      realname = s;
   }

   return realname;
}

inline EXPORT_FROM_DLL qboolean Entity::HitSky(const trace_t *trace) const
{
   assert(trace);
   if(trace->surface && (trace->surface->flags & SURF_SKY))
   {
      return true;
   }
   return false;
}

inline EXPORT_FROM_DLL qboolean Entity::HitSky() const
{
   return HitSky(&level.impact_trace);
}

inline EXPORT_FROM_DLL void Entity::Archive(Archiver &arc)
{
   Listener::Archive(arc);

   G_ArchiveEdict(arc, edict);

   arc.WriteInteger(spawnflags);

   arc.WriteFloat(translucence);
   arc.WriteInteger(viewheight);
   arc.WriteInteger(light_level);

   arc.WriteString(model);
   arc.WriteInteger(next_anim);
   arc.WriteInteger(next_frame);
   arc.WriteInteger(last_frame_in_anim);
   arc.WriteVector(frame_delta);
   arc.WriteVector(total_delta);
   arc.WriteVector(next_anim_delta);
   arc.WriteFloat(next_anim_time);
   arc.WriteBoolean(animating);
   arc.WriteEvent(*animDoneEvent);
   arc.WriteFloat(last_animation_time);
   arc.WriteInteger(num_frames_in_gun_anim);

   arc.WriteVector(mins);
   arc.WriteVector(maxs);
   arc.WriteVector(absmin);
   arc.WriteVector(absmax);
   arc.WriteVector(size);
   arc.WriteVector(centroid);
   arc.WriteVector(origin);
   arc.WriteVector(velocity);
   arc.WriteVector(avelocity);
   arc.WriteVector(angles);
   arc.WriteVector(worldorigin);
   arc.WriteVector(worldangles);
   arc.WriteRaw(orientation, sizeof(orientation));
   arc.WriteVector(vieworigin);
   arc.WriteVector(viewangles);
   arc.WriteInteger(contents);
   arc.WriteInteger(movetype);
   arc.WriteInteger(mass);
   arc.WriteFloat(gravity);
   arc.WriteInteger(gravaxis);

   if(groundentity)
   {
      arc.WriteInteger(groundentity - g_edicts);
   }
   else
   {
      arc.WriteInteger(-1);
   }

   arc.WriteRaw(&groundplane, sizeof(groundplane));
   arc.WriteInteger(groundcontents);

   arc.WriteInteger(groundentity_linkcount);

   arc.WriteObjectPointer(bindmaster);
   arc.WriteString(moveteam);
   arc.WriteObjectPointer(teamchain);
   arc.WriteObjectPointer(teammaster);

   arc.WriteInteger(numchildren);
   arc.WriteRaw(children, sizeof(children));

   arc.WriteString(target);
   arc.WriteString(targetname);
   // add to target list to rebuild targetlists
   arc.WriteString(killtarget);

   arc.WriteFloat(health);
   arc.WriteFloat(max_health);
   arc.WriteInteger(deadflag);
   arc.WriteInteger(flags);

   arc.WriteInteger(watertype);
   arc.WriteInteger(waterlevel);

   arc.WriteInteger((int)takedamage);
   arc.WriteSafePointer(enemy);
   arc.WriteFloat(pain_finished);
   arc.WriteFloat(damage_debounce_time);

   arc.WriteInteger(tess_min_size);
   arc.WriteInteger(tess_max_size);
   arc.WriteInteger(tess_thickness);
   arc.WriteFloat(tess_percentage);
}

inline EXPORT_FROM_DLL void Entity::Unarchive(Archiver &arc)
{
   int temp;

   Listener::Unarchive(arc);

   G_UnarchiveEdict(arc, edict);

   arc.ReadInteger(&spawnflags);

   arc.ReadFloat(&translucence);
   arc.ReadInteger(&viewheight);
   arc.ReadInteger(&light_level);

   arc.ReadString(&model);
   setModel(model);

   arc.ReadInteger(&next_anim);
   arc.ReadInteger(&next_frame);
   arc.ReadInteger(&last_frame_in_anim);
   arc.ReadVector(&frame_delta);
   arc.ReadVector(&total_delta);
   arc.ReadVector(&next_anim_delta);
   arc.ReadFloat(&next_anim_time);
   arc.ReadBoolean(&animating);
   animDoneEvent = new Event(arc.ReadEvent()); // haleyjd 20170608: repaired using proper const & copy constructor
   arc.ReadFloat(&last_animation_time);
   arc.ReadInteger(&num_frames_in_gun_anim);

   arc.ReadVector(&mins);
   arc.ReadVector(&maxs);
   arc.ReadVector(&absmin);
   arc.ReadVector(&absmax);
   arc.ReadVector(&size);
   arc.ReadVector(&centroid);
   arc.ReadVector(&origin);
   arc.ReadVector(&velocity);
   arc.ReadVector(&avelocity);
   arc.ReadVector(&angles);
   arc.ReadVector(&worldorigin);
   arc.ReadVector(&worldangles);
   arc.ReadRaw(orientation, sizeof(orientation));
   arc.ReadVector(&vieworigin);
   arc.ReadVector(&viewangles);
   arc.ReadInteger(&contents);
   arc.ReadInteger(&movetype);
   arc.ReadInteger(&mass);
   arc.ReadFloat(&gravity);
   arc.ReadInteger(&gravaxis);

   temp = arc.ReadInteger();
   if(temp == -1)
      groundentity = NULL;
   else
      groundentity = &g_edicts[temp];

   groundsurface = NULL;

   arc.ReadRaw(&groundplane, sizeof(groundplane));
   arc.ReadInteger(&groundcontents);

   arc.ReadInteger(&groundentity_linkcount);

   arc.ReadObjectPointer((Class **)&bindmaster);
   arc.ReadString(&moveteam);
   arc.ReadObjectPointer((Class **)&teamchain);
   arc.ReadObjectPointer((Class **)&teammaster);

   arc.ReadInteger(&numchildren);
   arc.ReadRaw(children, sizeof(children));

   arc.ReadString(&target);
   arc.ReadString(&targetname);
   arc.ReadString(&killtarget);

   // reset target stuff
   SetTargetName(targetname.c_str());
   SetTarget(target.c_str());

   arc.ReadFloat(&health);
   arc.ReadFloat(&max_health);
   arc.ReadInteger(&deadflag);
   arc.ReadInteger(&flags);

   arc.ReadInteger(&watertype);
   arc.ReadInteger(&waterlevel);

   temp = arc.ReadInteger();
   takedamage = (damage_t)temp;
   arc.ReadSafePointer(&enemy);
   arc.ReadFloat(&pain_finished);
   arc.ReadFloat(&damage_debounce_time);

   arc.ReadInteger(&tess_min_size);
   arc.ReadInteger(&tess_max_size);
   arc.ReadInteger(&tess_thickness);
   arc.ReadFloat(&tess_percentage);

   if(_isnan(angles.x) || _isnan(angles.y) || _isnan(angles.z))
      angles = vec_zero;
}

#include "worldspawn.h"

#endif

// EOF

