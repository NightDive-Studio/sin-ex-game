//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/entity.cpp                       $
// $Revision:: 297                                                            $
//   $Author:: Aldie                                                          $
//     $Date:: 3/18/99 6:44p                                                  $
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

#include "entity.h"
#include "worldspawn.h"
#include "scriptmaster.h"
#include "sentient.h"
#include "misc.h"
#include "specialfx.h"
#include "object.h"
#include "player.h"

CLASS_DECLARATION(Listener, Entity, NULL);

// Player events
Event EV_ClientConnect("client_connect");
Event EV_ClientDisconnect("client_disconnect");
Event EV_ClientKill("client_kill");
Event EV_ClientMove("client_move");
Event EV_ClientEndFrame("client_endframe");

// Generic entity events
Event EV_GetEntName("getentname");
Event EV_Classname("classname");
Event EV_Activate("doActivate");
Event EV_Use("doUse");
//Event EV_Footstep( "footstep" );
Event EV_FadeOut("fadeout");
Event EV_Fade("fade");
Event EV_Killed("killed");
Event EV_GotKill("gotkill");
Event EV_Pain("pain");
Event EV_Damage("damage");
Event EV_Kill("kill", EV_CONSOLE);
Event EV_Gib("gib");
Event EV_Hurt("hurt");

Event EV_CourseAngles("courseangles");
Event EV_SmoothAngles("smoothangles");
Event EV_TakeDamage("takedamage");
Event EV_NoDamage("nodamage");

// Physics events
Event EV_MoveDone("movedone");
Event EV_Touch("doTouch");
Event EV_Blocked("doBlocked");
Event EV_UseBoundingBox("usebbox");

// Animation events
Event EV_NewAnim("animChanged");
Event EV_LastFrame("lastFrame");
Event EV_NextAnim("nextanim");
Event EV_NextFrame("nextframe");
Event EV_PrevFrame("prevframe");
Event EV_SetFrame("setframe");
Event EV_StopAnim("stopanim");
Event EV_EndAnim("endanim");
Event EV_ProcessInitCommands("processinit");
Event EV_Attach("attach");
Event EV_AttachModel("attachmodel");
Event EV_Detach("detach");

// script stuff
Event EV_Model("model");
Event EV_Hide("hide");
Event EV_Show("show");
Event EV_BecomeSolid("solid");
Event EV_BecomeNonSolid("notsolid");
Event EV_Ghost("ghost");
Event EV_PlaySound("playsound");
Event EV_PHSSound("phssound");
Event EV_StopSound("stopsound");
Event EV_GravityAxis("gravityaxis", EV_CHEAT);
Event EV_Bind("bind");
Event EV_Unbind("unbind");
Event EV_JoinTeam("joinTeam");
Event EV_QuitTeam("quitTeam");
Event EV_SetHealth("health", EV_CHEAT);
Event EV_SetScale("scale");
Event EV_SetSize("setsize");
Event EV_SetAlpha("alpha");
Event EV_SetOrigin("origin");
Event EV_SetTargetName("targetname");
Event EV_SetTarget("target");
Event EV_SetKillTarget("killtarget");
Event EV_SetAngles("angles");
Event EV_RegisterAlias("alias");
Event EV_RegisterAliasAndCache("aliascache");
Event EV_RandomSound("randomsound");
Event EV_RandomPHSSound("randomphssound");
Event EV_Tesselate("shatter");
Event EV_SetMass("mass");

//HACK HACK
Event EV_EntitySound("ambientsound");
Event EV_RandomGlobalEntitySound("randomglobalambientsound");
Event EV_RandomEntitySound("randomambientsound");
Event EV_StopEntitySound("stopambientsound");
Event EV_Anim("anim");
Event EV_StartAnimating("animate");
Event EV_GroupModelEvent("group");
Event EV_DialogEvent("dialog");
Event EV_SetSkin("skin");

// AI sound events
Event EV_WeaponSound("weaponsound");
Event EV_MovementSound("movementsound");
Event EV_PainSound("painsound");
Event EV_DeathSound("deathsound");
Event EV_BreakingSound("breakingsound");
Event EV_DoorSound("doorsound");
Event EV_MutantSound("mutantsound");
Event EV_VoiceSound("voicesound");
Event EV_MachineSound("machinesound");
Event EV_RadioSound("radiosound");

Event EV_HeardWeapon("heardweapon");
Event EV_HeardMovement("heardmovement");
Event EV_HeardPain("heardpain");
Event EV_HeardDeath("hearddeath");
Event EV_HeardBreaking("heardbreaking");
Event EV_HeardDoor("hearddoor");
Event EV_HeardMutant("heardmutant");
Event EV_HeardVoice("heardvoice");
Event EV_HeardMachine("heardmachine");
Event EV_HeardRadio("heardradio");

// Conditionals
Event EV_IfSkill("ifskill");

// Lighting
Event EV_SetLight("light");
Event EV_LightOn("lightOn");
Event EV_LightOff("lightOff");
Event EV_LightRed("lightRed");
Event EV_LightGreen("lightGreen");
Event EV_LightBlue("lightBlue");
Event EV_LightRadius("lightRadius");

Event EV_Lightoffset("lightoffset");
Event EV_Minlight("minlight");
Event EV_Gravity("gravity");

// Entity flag specific
Event EV_EntityFlags("flags");
Event EV_EntityRenderEffects("rendereffects");
Event EV_EntityEffects("effects");

// Special Effects
Event EV_SpawnParticles("sparks");

// Tesselation setup events
Event EV_Shatter_MinSize("shatter_minsize");
Event EV_Shatter_MaxSize("shatter_maxsize");
Event EV_Shatter_Thickness("shatter_thickness");
Event EV_Shatter_Percentage("shatter_percentage");

Event EV_Mutate("mutate", EV_CHEAT);
Event EV_Censor("censor");

ResponseDef Entity::Responses[] =
{
   { &EV_Damage,				( Response )&Entity::DamageEvent },
   { &EV_Kill, 				( Response )&Entity::Kill },
   { &EV_FadeOut,				( Response )&Entity::FadeOut },
   { &EV_Fade,				   ( Response )&Entity::Fade },
   { &EV_Hide,					( Response )&Entity::EventHideModel },
   { &EV_Show,					( Response )&Entity::EventShowModel },
   { &EV_BecomeSolid,		( Response )&Entity::BecomeSolid },
   { &EV_BecomeNonSolid,	( Response )&Entity::BecomeNonSolid },
   { &EV_Ghost,	         ( Response )&Entity::Ghost },
   { &EV_PlaySound,			( Response )&Entity::PlaySound },
   { &EV_StopSound,			( Response )&Entity::StopSound },
   { &EV_GravityAxis,		( Response )&Entity::GravityAxisEvent },
   { &EV_Bind,					( Response )&Entity::BindEvent },
   { &EV_Unbind,				( Response )&Entity::EventUnbind },
   { &EV_JoinTeam,			( Response )&Entity::JoinTeam },
   { &EV_QuitTeam,			( Response )&Entity::EventQuitTeam },
   { &EV_SetHealth,			( Response )&Entity::SetHealth },
   { &EV_SetSize,				( Response )&Entity::SetSize },
   { &EV_SetScale,			( Response )&Entity::SetScale },
   { &EV_SetAlpha,			( Response )&Entity::SetAlpha },
   { &EV_SetOrigin,			( Response )&Entity::SetOrigin },
   { &EV_SetTargetName,		( Response )&Entity::SetTargetName },
   { &EV_SetTarget,			( Response )&Entity::SetTarget },
   { &EV_SetKillTarget,		( Response )&Entity::SetKillTarget },
   { &EV_SetAngles,			( Response )&Entity::SetAngles },
   { &EV_SetMass,          ( Response )&Entity::SetMassEvent },

   { &EV_CourseAngles,		( Response )&Entity::CourseAnglesEvent },
   { &EV_SmoothAngles,		( Response )&Entity::SmoothAnglesEvent },
   { &EV_RegisterAlias,	   ( Response )&Entity::RegisterAlias },
   { &EV_RegisterAliasAndCache, ( Response )&Entity::RegisterAliasAndCache },
   { &EV_RandomSound,	   ( Response )&Entity::RandomSound },
   { &EV_EntitySound,	   ( Response )&Entity::EntitySound },
   { &EV_RandomEntitySound,( Response )&Entity::RandomEntitySound },
   { &EV_RandomGlobalEntitySound, ( Response )&Entity::RandomGlobalEntitySoundEvent },
   { &EV_StopEntitySound,	( Response )&Entity::StopEntitySound },
   { &EV_Anim,             ( Response )&Entity::AnimEvent },
   { &EV_StartAnimating,	( Response )&Entity::StartAnimatingEvent },
   { &EV_NextAnim,			( Response )&Entity::NextAnimEvent },
   { &EV_NextFrame,			( Response )&Entity::NextFrameEvent },
   { &EV_PrevFrame,			( Response )&Entity::PrevFrameEvent },
   { &EV_SetFrame,			( Response )&Entity::SetFrameEvent },
   { &EV_StopAnim,			( Response )&Entity::StopAnimatingEvent },
   { &EV_EndAnim,				( Response )&Entity::EndAnimEvent },
   { &EV_Model,				( Response )&Entity::SetModelEvent },
   { &EV_SetLight,			( Response )&Entity::SetLight },
   { &EV_LightOn,				( Response )&Entity::LightOn },
   { &EV_LightOff,		   ( Response )&Entity::LightOff },
   { &EV_LightRed,		   ( Response )&Entity::LightRed },
   { &EV_LightGreen,		   ( Response )&Entity::LightGreen },
   { &EV_LightBlue,		   ( Response )&Entity::LightBlue },
   { &EV_LightRadius,	   ( Response )&Entity::LightRadius },
   { &EV_Tesselate,	      ( Response )&Entity::Tesselate },
   { &EV_EntityFlags,	   ( Response )&Entity::Flags },
   { &EV_EntityEffects,	   ( Response )&Entity::Effects },
   { &EV_EntityRenderEffects,	   ( Response )&Entity::RenderEffects },
   { &EV_RandomPHSSound,	( Response )&Entity::RandomPHSSound },
   { &EV_PHSSound,	      ( Response )&Entity::PHSSound },

   { &EV_WeaponSound,		( Response )&Entity::WeaponSound },
   { &EV_MovementSound,		( Response )&Entity::MovementSound },
   { &EV_PainSound,			( Response )&Entity::PainSound },
   { &EV_DeathSound,			( Response )&Entity::DeathSound },
   { &EV_BreakingSound,		( Response )&Entity::BreakingSound },
   { &EV_DoorSound,			( Response )&Entity::DoorSound },
   { &EV_MutantSound,		( Response )&Entity::MutantSound },
   { &EV_VoiceSound,			( Response )&Entity::VoiceSound },
   { &EV_MachineSound,		( Response )&Entity::MachineSound },
   { &EV_RadioSound,			( Response )&Entity::RadioSound },
   { &EV_SpawnParticles,	( Response )&Entity::SpawnParticles },
   { &EV_GroupModelEvent,	( Response )&Entity::GroupModelEvent },
   { &EV_DialogEvent,	   ( Response )&Entity::DialogEvent },
   { &EV_ProcessInitCommands,( Response )&Entity::ProcessInitCommandsEvent },
   { &EV_Attach,           ( Response )&Entity::AttachEvent },
   { &EV_AttachModel,      ( Response )&Entity::AttachModelEvent },
   { &EV_Detach,           ( Response )&Entity::DetachEvent },
   { &EV_TakeDamage,       ( Response )&Entity::TakeDamageEvent },
   { &EV_NoDamage,         ( Response )&Entity::NoDamageEvent },
   { &EV_SetSkin,          ( Response )&Entity::SetSkinEvent },
   { &EV_Lightoffset,      ( Response )&Entity::Lightoffset },
   { &EV_Minlight,         ( Response )&Entity::Minlight },
   { &EV_Gravity,          ( Response )&Entity::Gravity },

   { &EV_Shatter_MinSize,	( Response )&Entity::SetShatterMinSize },
   { &EV_Shatter_MaxSize,	( Response )&Entity::SetShatterMaxSize },
   { &EV_Shatter_Thickness,( Response )&Entity::SetShatterThickness },
   { &EV_Shatter_Percentage,( Response )&Entity::SetShatterPercentage },

   { &EV_UseBoundingBox,   ( Response )&Entity::UseBoundingBoxEvent },
   { &EV_Hurt,             ( Response )&Entity::HurtEvent },
   { &EV_IfSkill,          ( Response )&Entity::IfSkillEvent },

   { &EV_GetEntName,       ( Response )&Entity::GetEntName },
   { &EV_Censor,           ( Response )&Entity::Censor },

   { NULL, NULL }
};

Entity::Entity() : Listener()
{
   const char *m;
   Event *ev;
   int   minlight;

   classname = this->getClassname();

   if(game.force_entnum)
   {
      game.force_entnum = false;
      edict = &g_edicts[game.spawn_entnum];
      LL_Remove(edict, next, prev);
      G_InitEdict(edict);
      LL_Add(&active_edicts, edict, next, prev);
   }
   else
   {
      edict = G_Spawn();
   }

   client = edict->client;
   edict->entity = this;
   entnum = edict->s.number;

   m = G_GetSpawnArg("classname");
   if(m)
   {
      Q_strlcpy(edict->entname, m, sizeof(edict->entname));
   }

   // spawning variables
   spawnflags = G_GetIntArg("spawnflags");
   if(spawnflags & SPAWNFLAG_DETAIL)
   {
      edict->s.renderfx |= RF_DETAIL;
   }

   // rendering variables
   setAlpha(G_GetFloatArg("alpha", 1.0f));
   setScale(G_GetFloatArg("scale", 1.0f));

   minlight = G_GetIntArg("minlight", 0);
   if(minlight)
      edict->s.renderfx |= RF_MINLIGHT;

   edict->s.lightofs = G_GetFloatArg("lightoffset", 0);
   if(edict->s.lightofs)
      edict->s.renderfx |= RF_LIGHTOFFSET;

   viewheight = 0;
   light_level = 0;

   // Animation variables
   next_anim              = -1;
   next_frame             = -1;
   last_frame_in_anim     = -1;
   frame_delta            = { 0, 0, 0 };
   next_anim_delta        = { 0, 0, 0 };
   next_anim_time         = 0;
   total_delta            = { 0, 0, 0 };
   animDoneEvent          = NULL;
   animating              = false;
   last_frame_in_anim     = 0;
   last_animation_time    = -1;
   num_frames_in_gun_anim = 0;

   // team variables
   teamchain = NULL;
   teammaster = NULL;
   m = G_GetSpawnArg("team");
   if(m)
   {
      moveteam = str(m);
   }

   // physics variables
   contents               = 0;
   mass                   = 0;
   gravity                = 1.0;
   groundentity           = NULL;
   groundsurface          = NULL;
   groundcontents         = 0;
   groundentity_linkcount = 0;
   bindmaster             = NULL;
   velocity               = vec_zero;
   avelocity              = vec_zero;
   angles                 = vec_zero;
   worldorigin            = vec_zero;
   worldangles            = vec_zero;
   vieworigin             = vec_zero;
   viewangles             = vec_zero;

   SetGravityAxis(G_GetIntArg("gravityaxis", 0));

   // model binding variables
   numchildren = 0;
   memset(&children, 0, sizeof(children));

   setOrigin(Vector(G_GetSpawnArg("origin")));
   worldorigin.copyTo(edict->s.old_origin);

   setMoveType(MOVETYPE_NONE);
   setSolidType(SOLID_NOT);

   // targeting variables
   SetTargetName(G_GetSpawnArg("targetname"));
   SetTarget(G_GetSpawnArg("target"));

   // Character state
   health     = 0;
   max_health = 0;
   deadflag   = DEAD_NO;
   flags      = 0;

   // underwater variables
   watertype  = 0;
   waterlevel = 0;

   // Pain and damage variables
   takedamage           = DAMAGE_NO;
   enemy                = NULL;
   pain_finished        = 0;
   damage_debounce_time = 0;

   m = G_GetSpawnArg("model");
   if(m)
   {
      setModel(m);
   }

   //
   // see if we have a mins and maxs set for this model
   //
   if(gi.IsModel(edict->s.modelindex) && !mins.length() && !maxs.length())
   {
      vec3_t tempmins, tempmaxs;
      gi.CalculateBounds(edict->s.modelindex, edict->s.scale, tempmins, tempmaxs);
      setSize(tempmins, tempmaxs);
   }

   //
   // get the number of groups for this model
   //
   edict->s.numgroups = gi.NumGroups( edict->s.modelindex );

   m = G_GetSpawnArg("bind");
   if(m)
   {
      str name;

      ev = new Event(EV_Bind);

      // construct an object name
      name = "$";
      name += m;
      ev->AddString(name);

      // Wait until all entities are spawned.
      PostEvent(ev, 0);
   }

   //
   // initialize tesselation variables
   //
   tess_max_size = size.length() / 4;
   tess_min_size = tess_max_size / 3;

   if(tess_min_size < 8)
   {
      tess_min_size = 8;
   }

   if(tess_max_size <= tess_min_size)
   {
      tess_max_size = tess_min_size * 2;
   }

   tess_thickness = tess_min_size;
   tess_percentage = TESS_DEFAULT_PERCENT;
}

Entity::~Entity()
{
   Container<Entity *> bindlist;
   Entity *ent;
   int num;
   int i;

   // unbind any entities that are bound to me
   // can't unbind within this loop, so make an array
   // and unbind them outside of it.
   num = 0;
   for(ent = teamchain; ent; ent = ent->teamchain)
   {
      if(ent->bindmaster == this)
      {
         bindlist.AddObject(ent);
      }
   }

   num = bindlist.NumObjects();
   for(i = 1; i <= num; i++)
   {
      bindlist.ObjectAt(i)->unbind();
   }

   bindlist.FreeObjectList();

   unbind();
   quitTeam();
   detach();

   //
   // go through and set our children
   //
   num = numchildren;
   for(i = 0; (i < MAX_MODEL_CHILDREN) && num; i++)
   {
      if(!children[i])
      {
         continue;
      }
      ent = (Entity *)G_GetEntity(children[i]);
      ent->PostEvent(EV_Remove, 0);
      num--;
   }

   if(targetname.length() && world)
   {
      world->RemoveTargetEntity(targetname, this);
   }

   this->CancelPendingEvents();
   G_FreeEdict(edict);
}

EXPORT_FROM_DLL void Entity::SetEntNum(int num)
{
   if(edict)
   {
      G_FreeEdict(edict);
   }

   edict = &g_edicts[num];
   LL_Remove(edict, next, prev);
   G_InitEdict(edict);
   LL_Add(&active_edicts, edict, next, prev);

   client = edict->client;
   edict->entity = this;
   entnum = num;
}

EXPORT_FROM_DLL void Entity::GetEntName(Event *ev)
{
   Q_strlcpy(edict->entname, getClassname(), sizeof(edict->entname));
}

EXPORT_FROM_DLL void Entity::SetTarget(const char *text)
{
   if(text)
   {
      target = text;
   }
   else
   {
      target = "";
   }
}

EXPORT_FROM_DLL void Entity::SetTargetName(const char *text)
{
   if(targetname.length() && world)
   {
      world->RemoveTargetEntity(targetname, this);
   }

   if(text)
   {
      if(text[0] == '$')
         text++;
      targetname = text;
   }
   else
   {
      targetname = "";
   }

   if(targetname.length() && world)
   {
      world->AddTargetEntity(targetname, this);
   }
}

EXPORT_FROM_DLL void Entity::SetKillTarget(const char *text)
{
   if(text)
   {
      killtarget = text;
   }
   else
   {
      killtarget = "";
   }
}

EXPORT_FROM_DLL int Entity::modelIndex(const char *mdl) const
{
   assert(mdl);

   if(!mdl)
   {
      return 0;
   }
   
   str name;
   
   // Prepend 'models/' to make things easier
   if(!strchr(mdl, '*') && !strchr(mdl, '\\') && !strchr(mdl, '/'))
   {
      name = "models/";
   }

   name += mdl;

   return gi.modelindex(name.c_str());
}

EXPORT_FROM_DLL void Entity::setModel(const char *mdl)
{
   str temp;

   if(!mdl)
   {
      mdl = "";
   }

   // Prepend 'models/' to make things easier
   temp = "";
   if(!strchr(mdl, '*') && !strchr(mdl, '\\') && !strchr(mdl, '/'))
   {
      temp = "models/";
   }
   temp += mdl;

   // we use a temp string so that if model was passed into here, we don't 
   // accidentally free up the string that we're using in the process.
   model = temp;

   gi.setmodel(edict, model.c_str());

   if(gi.IsModel(edict->s.modelindex))
   {
      Event *ev;

      edict->s.numgroups = gi.NumGroups(edict->s.modelindex);

      if(!LoadingSavegame)
      {
         CancelEventsOfType(EV_ProcessInitCommands);

         ev = new Event(EV_ProcessInitCommands);
         ev->AddInteger(edict->s.modelindex);
         PostEvent(ev, 0);
      }
   }

   // Sanity check to see if we're expecting a B-Model
   assert(!((edict->solid == SOLID_BSP) && !edict->s.modelindex));
   if((edict->solid == SOLID_BSP) && !edict->s.modelindex)
   {
      const char *name;

      name = getClassID();
      if(!name)
      {
         name = getClassname();
      }
      gi.dprintf("%s with SOLID_BSP and no model - '%s'(%d)\n", name, targetname.c_str(), entnum);

      // Make it non-solid so that the collision code doesn't kick us out.
      setSolidType(SOLID_NOT);
   }

   mins = edict->mins;
   maxs = edict->maxs;
   size = edict->size;
}

EXPORT_FROM_DLL void Entity::ProcessInitCommands(int index)
{
   sinmdl_cmd_t *cmds;

   if(LoadingSavegame)
   {
      // Don't process init commands when loading a savegame since
      // it will cause items to be added to inventories unnecessarily.
      // All variables affected by the init commands will be set
      // by the unarchive functions.
      return;
   }

   cmds = gi.InitCommands(index);
   if(cmds)
   {
      int i, j;
      Event		*event;

      for(i=0; i<cmds->num_cmds; i++)
      {
         event = new Event(cmds->cmds[i].args[0]);
         for(j=1; j<cmds->cmds[i].num_args; j++)
         {
            event->AddToken(cmds->cmds[i].args[j]);
         }
         ProcessEvent(event);
      }
   }
}

EXPORT_FROM_DLL void Entity::ProcessInitCommandsEvent(Event *ev)
{
   int index;

   index = ev->GetInteger(1);
   ProcessInitCommands(index);
}

EXPORT_FROM_DLL void Entity::EventHideModel(Event *ev)
{
   hideModel();
}

EXPORT_FROM_DLL void Entity::EventShowModel(Event *ev)
{
   showModel();
}

EXPORT_FROM_DLL void Entity::setAlpha(float alpha)
{
   if(alpha > 1.0f)
   {
      alpha = 1.0f;
   }
   if(alpha < 0)
   {
      alpha = 0;
   }
   translucence = 1.0f - alpha;
   edict->s.alpha = alpha;
   edict->s.renderfx &= ~RF_TRANSLUCENT;

   if((translucence > 0) && (translucence <= 1.0))
   {
      edict->s.renderfx |= RF_TRANSLUCENT;
   }
}

EXPORT_FROM_DLL void Entity::setScale(float scale)
{
   if(scale > 255.0f)
   {
      scale = 255.0f;
   }
   if(scale < 0.004f)
   {
      scale = 0.004f;
   }
   edict->s.scale = scale;
}

EXPORT_FROM_DLL void Entity::setSolidType(solid_t type)
{
   if(
      (!LoadingSavegame) &&
      (type == SOLID_BSP) &&
      (this != world) &&
      (
         !model.length() ||
         (
         (model[0] != '*') &&
            (!strstr(model.c_str(), ".bsp"))
            )
         )
      )
   {
      error("setSolidType", "SOLID_BSP entity at x%.2f y%.2f z%.2f with no BSP model", worldorigin[0], worldorigin[1], worldorigin[2]);
   }
   edict->solid = type;
   link();

   edict->svflags &= ~SVF_NOCLIENT;
   if(hidden())
   {
      edict->svflags |= SVF_NOCLIENT;
   }
}

EXPORT_FROM_DLL void Entity::setSize(Vector min, Vector max)
{
   if(flags & FL_ROTATEDBOUNDS)
   {
      vec3_t tempmins, tempmaxs;
      float mat[3][3];

      //
      // rotate the mins and maxs for the model
      //
      min.copyTo(tempmins);
      max.copyTo(tempmaxs);

      VectorCopy(orientation[0], mat[0]);
      VectorNegate(orientation[1], mat[1]);
      VectorCopy(orientation[2], mat[2]);

      CalculateRotatedBounds2(mat, tempmins, tempmaxs);

      mins = Vector(tempmins);
      maxs = Vector(tempmaxs);
      size = max - min;

      mins.copyTo(edict->mins);
      maxs.copyTo(edict->maxs);
      size.copyTo(edict->size);
      mins.copyTo(edict->fullmins);
      maxs.copyTo(edict->fullmaxs);
      edict->fullradius = size.length() * 0.5;
   }
   else
   {
      if((min == edict->mins) && (max == edict->maxs))
      {
         return;
      }

      mins = min;
      maxs = max;
      size = max - min;

      mins.copyTo(edict->mins);
      maxs.copyTo(edict->maxs);
      size.copyTo(edict->size);

      //
      // get the full mins and maxs for this model
      //
      if(gi.IsModel(edict->s.modelindex))
      {
         Vector delta;
         vec3_t fmins;
         vec3_t fmaxs;
         const gravityaxis_t &grav = gravity_axis[gravaxis];

         gi.CalculateBounds(edict->s.modelindex, edict->s.scale, fmins, fmaxs);
         edict->fullmins[0] = fmins[grav.x];
         edict->fullmaxs[0] = fmaxs[grav.x];

         if(grav.sign > 0)
         {
            edict->fullmins[1] = fmins[grav.y];
            edict->fullmins[2] = fmins[grav.z];
            edict->fullmaxs[1] = fmaxs[grav.y];
            edict->fullmaxs[2] = fmaxs[grav.z];
         }
         else
         {
            edict->fullmins[1] = -fmaxs[grav.y];
            edict->fullmins[2] = -fmaxs[grav.z];
            edict->fullmaxs[1] = -fmins[grav.y];
            edict->fullmaxs[2] = -fmins[grav.z];
         }

         delta = Vector(edict->fullmaxs) - Vector(edict->fullmins);
         edict->fullradius = delta.length() * 0.5f;
      }
      else
      {
         mins.copyTo(edict->fullmins);
         maxs.copyTo(edict->fullmaxs);
         edict->fullradius = size.length() * 0.5;
      }
   }

   link();
}

EXPORT_FROM_DLL Vector Entity::getLocalVector(const Vector &vec) const
{
   Vector pos;

   pos[0] = vec * orientation[0];
   pos[1] = vec * orientation[1];
   pos[2] = vec * orientation[2];

   return pos;
}

EXPORT_FROM_DLL Vector Entity::getParentVector(const Vector &vec) const
{
   if(!bindmaster)
   {
      return vec;
   }
   
   Vector pos;
   
   pos[0] = vec * bindmaster->orientation[0];
   pos[1] = vec * bindmaster->orientation[1];
   pos[2] = vec * bindmaster->orientation[2];

   return pos;
}

EXPORT_FROM_DLL void Entity::link()
{
   gi.linkentity(edict);
   absmin = edict->absmin;
   absmax = edict->absmax;
   centroid = (absmin + absmax) * 0.5;
   centroid.copyTo(edict->centroid);

   // If this has a parent, then set the areanum the same
   // as the parent's
   if(edict->s.parent)
   {
      edict->areanum = g_edicts[edict->s.parent].areanum;
   }
}

EXPORT_FROM_DLL void Entity::setOrigin(Vector org)
{
   Entity * ent;
   int i, num;

   origin = org;
   if(bindmaster)
   {
      MatrixTransformVector(origin.vec3(), bindmaster->orientation, worldorigin.vec3());
      worldorigin += bindmaster->worldorigin;
      worldorigin.copyTo(edict->s.vieworigin);
   }
   else if(edict->s.parent)
   {
      org.copyTo(edict->s.vieworigin);
      ent = (Entity *)G_GetEntity(edict->s.parent);
      worldorigin = ent->centroid;
   }
   else
   {
      worldorigin = origin;
      worldorigin.copyTo(edict->s.vieworigin);
   }

   worldorigin.copyTo(edict->s.origin);
   link();

   //
   // go through and set our children
   //
   num = numchildren;
   for(i = 0; (i < MAX_MODEL_CHILDREN) && num; i++)
   {
      if(!children[i])
      {
         continue;
      }
      ent = (Entity *)G_GetEntity(children[i]);
      ent->setOrigin(ent->origin);
      num--;
   }
}

EXPORT_FROM_DLL qboolean Entity::GetBone(const char *name, Vector *pos, Vector *forward, Vector *right, Vector *up)
{
   vec3_t	trans[3];
   vec3_t	p1, p2;
   vec3_t   orient;
   int		groupindex;
   int		tri_num;

   // get the bone information
   if(!gi.GetBoneInfo(edict->s.modelindex, name, &groupindex, &tri_num, orient))
   {
      return false;
   }
   if(!gi.GetBoneTransform(edict->s.modelindex, groupindex, tri_num, orient, edict->s.anim, edict->s.frame,
                           edict->s.scale, trans, p1))
   {
      return false;
   }

   if(forward || right || up)
   {
      R_ConcatRotations(trans, orientation, trans);
   }

   if(pos)
   {
      MatrixTransformVector(p1, orientation, p2);
      *pos = Vector(p2);
   }
   if(forward)
   {
      *forward = Vector(trans[0]);
   }
   if(right)
   {
      *right = Vector(trans[1]);
   }
   if(up)
   {
      *up = Vector(trans[2]);
   }

   return true;
}

EXPORT_FROM_DLL void Entity::setAngles(const Vector &ang)
{
   Entity * ent;
   float	mat[3][3];
   int num, i;

   assert(!isnan(ang[0]));
   assert(!isnan(ang[1]));
   assert(!isnan(ang[2]));

   angles[0] = angmod(ang[0]);
   angles[1] = angmod(ang[1]);
   angles[2] = angmod(ang[2]);

   if(bindmaster)
   {
      AnglesToMat(angles.vec3(), mat);
      R_ConcatRotations(mat, bindmaster->orientation, orientation);
      MatrixToEulerAngles(orientation, worldangles.vec3());
      worldangles.copyTo(edict->s.viewangles);
   }
   else if(edict->s.parent)
   {
      float trans[3][3];
      float local_trans[3][3];
      vec3_t p1;

      ent = (Entity *)G_GetEntity(edict->s.parent);
      ang.copyTo(edict->s.viewangles);

      if(gi.GetBoneTransform(ent->edict->s.modelindex, edict->s.bone.group_num, edict->s.bone.tri_num, edict->s.bone.orientation,
                             ent->edict->s.anim, ent->edict->s.frame, ent->edict->s.scale, trans, p1))
      {
         AnglesToMat(angles.vec3(), mat);
         R_ConcatRotations(mat, trans, local_trans);
         R_ConcatRotations(local_trans, ent->orientation, orientation);
         MatrixToEulerAngles(orientation, worldangles.vec3());
      }
   }
   else
   {
      worldangles = angles;
      AnglesToMat(worldangles.vec3(), orientation);
      worldangles.copyTo(edict->s.viewangles);
   }

   worldangles.copyTo(edict->s.angles);

   // Fill the edicts matrix
   VectorCopy(orientation[0], edict->s.mat[0]);
   VectorCopy(orientation[1], edict->s.mat[1]);
   VectorCopy(orientation[2], edict->s.mat[2]);

   //
   // go through and set our children
   //
   num = numchildren;
   for(i=0; (i < MAX_MODEL_CHILDREN) && num; i++)
   {
      if(!children[i])
         continue;
      ent = (Entity *)G_GetEntity(children[i]);
      ent->setAngles(ent->angles);
      num--;
   }
}

EXPORT_FROM_DLL qboolean Entity::droptofloor(float maxfall)
{
   trace_t	trace;
   Vector	end;

   end    = origin;
   end[2] -= maxfall;
   origin += { 0, 0, 1 };

   trace = G_Trace(origin, mins, maxs, end, this, MASK_SOLID, "Entity::droptofloor");
   if(trace.fraction == 1 || trace.allsolid)
   {
      groundentity = nullptr;
      return false;
   }

   setOrigin(trace.endpos);

   groundentity				= trace.ent;
   groundentity_linkcount	= trace.ent->linkcount;

   return true;
}

void Entity::Damage(Entity *inflictor, Entity *attacker, int damage, Vector position, Vector direction, Vector normal, 
                    int knockback, int dflags, int meansofdeath, int groupnum, int trinum, float damage_multiplier)
{
   Event	*ev;

   if(!attacker)
   {
      attacker = world;
   }
   if(!inflictor)
   {
      inflictor = world;
   }

   ev = new Event(EV_Damage);
   ev->AddInteger(damage);
   ev->AddEntity(inflictor);
   ev->AddEntity(attacker);
   ev->AddVector(position);
   ev->AddVector(direction);
   ev->AddVector(normal);
   ev->AddInteger(knockback);
   ev->AddInteger(dflags);
   ev->AddInteger(meansofdeath);
   ev->AddInteger(groupnum);
   ev->AddInteger(trinum);
   ev->AddFloat(damage_multiplier);
   ProcessEvent(ev);
}

void Entity::DamageEvent(Event *ev)
{
   Entity	*inflictor;
   Entity	*attacker;
   int		damage;
   Vector	dir;
   Vector	momentum;
   Event		*event;
   float		m;

   if((takedamage == DAMAGE_NO) || (movetype == MOVETYPE_NOCLIP))
   {
      return;
   }

   damage		= ev->GetInteger(1);
   inflictor	= ev->GetEntity(2);
   attacker		= ev->GetEntity(3);

   // figure momentum add
   if((inflictor != world) &&
      (movetype != MOVETYPE_NONE) &&
      (movetype != MOVETYPE_BOUNCE) &&
      (movetype != MOVETYPE_PUSH) &&
      (movetype != MOVETYPE_STOP))
   {
      dir = worldorigin - (inflictor->worldorigin + (inflictor->mins + inflictor->maxs) * 0.5);
      dir.normalize();

      if(mass < 50)
      {
         m = 50;
      }
      else
      {
         m = mass;
      }

      momentum = dir * damage * (1700.0 / m);
      velocity += momentum;
   }

   // check for godmode or invincibility
   if(flags & FL_GODMODE)
   {
      return;
   }

   // Forcefields make objects invulnerable
   if(flags & FL_FORCEFIELD)
   {
      float    alpha;
      float    radius;
      Entity   *forcefield;
      //
      // spawn forcefield
      //
      forcefield = new Entity();

      radius = (centroid - worldorigin).length();
      forcefield->setModel("sphere2.def");
      forcefield->setOrigin(centroid);
      forcefield->worldorigin.copyTo(forcefield->edict->s.old_origin);
      forcefield->setMoveType(MOVETYPE_NONE);
      forcefield->setSolidType(SOLID_NOT);
      forcefield->edict->s.scale = radius / 16;
      alpha = damage / 100;
      if(alpha > 1)
         alpha = 1;
      if(alpha < 0.15f)
         alpha = 0.15f;
      forcefield->edict->s.alpha = alpha;
      forcefield->edict->s.renderfx |= RF_TRANSLUCENT;
      forcefield->PostEvent(EV_Remove, 0.1f);
      return;
   }

   // team play damage avoidance
   //if ( ( global->teamplay == 1 ) && ( edict->team > 0 ) && ( edict->team == attacker->edict->team ) )
   //	{
   //	return;
   //	}

   if(!deathmatch->value && isSubclassOf<Player>())
   {
      damage *= 0.15;
   }

   if(deadflag)
   {
      // Check for gib.
      if(inflictor->isSubclassOf<Projectile>())
      {
         Event *gibEv;

         health -= damage;

         gibEv = new Event(EV_Gib);
         gibEv->AddEntity(this);
         gibEv->AddFloat(health);
         ProcessEvent(gibEv);
      }
      return;
   }

   // do the damage
   health -= damage;
   if(health <= 0)
   {
      if(attacker)
      {
         event = new Event(EV_GotKill);
         event->AddEntity(this);
         event->AddInteger(damage);
         event->AddEntity(inflictor);
         // location based damage
         event->AddString(ev->GetString(4));
         event->AddInteger(ev->GetInteger(9));
         event->AddInteger(0);
         attacker->ProcessEvent(event);
      }

      event = new Event(EV_Killed);
      event->AddEntity(attacker);
      event->AddInteger(damage);
      event->AddEntity(inflictor);
      // location based damage
      event->AddString(ev->GetString(4));
      ProcessEvent(event);
      return;
   }

   if(flags & FL_TESSELATE)
   {
      TesselateModel
      (
         this,
         tess_min_size,
         tess_max_size,
         dir,
         damage,
         tess_percentage*0.5f,
         tess_thickness,
         ev->GetVector(5)
      );
   }

   if(flags & FL_DARKEN)
   {
      edict->s.renderfx |= RF_LIGHTOFFSET;
      if(max_health)
      {
         edict->s.lightofs = -(40.0f * ((float)(max_health - health) / (float)max_health));
      }
      else
      {
         edict->s.lightofs -= damage;
      }
      if(edict->s.lightofs < -127)
         edict->s.lightofs = -127;
      if(edict->s.lightofs > 127)
         edict->s.lightofs = 127;
   }

   event = new Event(EV_Pain);
   event->AddFloat(damage);
   event->AddEntity(attacker);
   // location based damage
   event->AddString(ev->GetString(4));
   ProcessEvent(event);
}

/*
============
CanDamage

Returns true if the inflictor can directly damage the target.  Used for
explosions and melee attacks.
============
*/
qboolean Entity::CanDamage(Entity *target)
{
   trace_t	trace;
   Vector	pos;

   // bmodels need special checking because their origin is 0,0,0
   if(target->getMoveType() == MOVETYPE_PUSH)
   {
      pos = (target->absmin + target->absmax) * 0.5;
      trace = G_Trace(origin, vec_origin, vec_origin, pos, this, MASK_SHOT, "Entity::CanDamage 1");
      if(trace.fraction == 1 || trace.ent == target->edict)
      {
         return true;
      }
      return false;
   }

   trace = G_Trace(origin, vec_origin, vec_origin, target->centroid, this, MASK_SHOT, "Entity::CanDamage 2");
   if(trace.fraction == 1 || trace.ent == target->edict)
   {
      return true;
   }
   pos = target->centroid + Vector(15, 15, 0);
   trace = G_Trace(origin, vec_origin, vec_origin, pos, this, MASK_SHOT, "Entity::CanDamage 3");
   if(trace.fraction == 1 || trace.ent == target->edict)
   {
      return true;
   }
   pos = target->centroid + Vector(-15, 15, 0);
   trace = G_Trace(origin, vec_zero, vec_zero, pos, this, MASK_SHOT, "Entity::CanDamage 4");
   if(trace.fraction == 1 || trace.ent == target->edict)
   {
      return true;
   }
   pos = target->centroid + Vector(15, -15, 0);
   trace = G_Trace(origin, vec_zero, vec_zero, pos, this, MASK_SHOT, "Entity::CanDamage 5");
   if(trace.fraction == 1 || trace.ent == target->edict)
   {
      return true;
   }
   pos = target->centroid + Vector(-15, -15, 0);
   trace = G_Trace(origin, vec_zero, vec_zero, pos, this, MASK_SHOT, "Entity::CanDamage 6");
   if(trace.fraction == 1 || trace.ent == target->edict)
   {
      return true;
   }

   return false;
}

EXPORT_FROM_DLL qboolean Entity::IsTouching(const Entity *e1) const
{
   if(e1->absmin.x > absmax.x)
   {
      return false;
   }
   if(e1->absmin.y > absmax.y)
   {
      return false;
   }
   if(e1->absmin.z > absmax.z)
   {
      return false;
   }
   if(e1->absmax.x < absmin.x)
   {
      return false;
   }
   if(e1->absmax.y < absmin.y)
   {
      return false;
   }
   if(e1->absmax.z < absmin.z)
   {
      return false;
   }

   return true;
}

void Entity::StopAnimating()
{
   // Cancel all animating events
   last_animation_time = -1;
   animating = false;
   total_delta = vec_zero;
   if(animDoneEvent)
   {
      CancelEventsOfType(animDoneEvent);
      delete animDoneEvent;
      animDoneEvent = nullptr;
   }
}

void Entity::StartAnimating()
{
   // start animating
   AnimateFrame();
   animating = true;
}

void Entity::NextAnim(int animnum)
{
   if((animnum >= 0) && (animnum < gi.NumAnims(edict->s.modelindex)))
   {
      next_anim = animnum;
   }
   else
   {
      // bad value
      return;
   }

   // get the next anim delta
   gi.Anim_Delta(edict->s.modelindex, next_anim, next_anim_delta.vec3());
   next_anim_delta *= edict->s.scale;

   // get the next anim time
   next_anim_time = gi.Anim_Time(edict->s.modelindex, next_anim);
   NextFrame(0);
}

void Entity::NextFrame(int framenum)
{
   if((framenum >= 0) && (framenum <= last_frame_in_anim))
   {
      next_frame = framenum;
   }
   else
   {
      // bad value
      return;
   }
}

void Entity::AnimateFrame(void)
{
   float delta;
   sinmdl_cmd_t * cmds;
   Event *ev;
   int i;
   int j;

   //
   // see if we have already animated this frame
   //
   if(
      (level.time == last_animation_time) &&
      (next_anim < 0) &&
      (next_frame == edict->s.frame + 1)
      )
   {
      return;
   }

   // see if we have an anim change pending
   if(next_anim >= 0)
   {
      edict->s.anim = next_anim;
      last_frame_in_anim = gi.Anim_NumFrames(edict->s.modelindex, edict->s.anim) - 1;
      next_anim = -1;
      if(edict->s.gunmodelindex)
      {
         const char * animname;
         animname = gi.Anim_NameForNum(edict->s.modelindex, edict->s.anim);
         //
         // see if the anim exists in the world model
         //
         edict->s.gunanim        = gi.Anim_Random(edict->s.gunmodelindex, animname);
         if(edict->s.gunanim < 0)
         {
            //
            // see if at least we have an idle 
            //
            edict->s.gunanim        = gi.Anim_Random(edict->s.gunmodelindex, "idle");
         }
         if(edict->s.gunanim >= 0)
         {
            num_frames_in_gun_anim = gi.Anim_NumFrames(edict->s.gunmodelindex, edict->s.gunanim);
         }
         else
         {
            edict->s.gunanim = 0;
            num_frames_in_gun_anim = 0;
         }
      }
   }

   // see if we have a frame change pending
   if(next_frame >= 0)
   {
      edict->s.frame = next_frame;
      next_frame = -1;
   }
#if 0
   {
      const char * animname;
      animname = gi.Anim_NameForNum(edict->s.modelindex, edict->s.anim);
      warning("aframe", "%d anim %s frame %d", entnum, animname, edict->s.frame);
   }
#endif

   delta = gi.Frame_Time(edict->s.modelindex, edict->s.anim, edict->s.frame);
   if(!delta)
   {
      delta = FRAMETIME;
   }
   next_frame = edict->s.frame + (int)((float)FRAMETIME / delta);

   // should never be greater...but just in case
   if((edict->s.frame >= last_frame_in_anim) || (next_frame > last_frame_in_anim))
   {
      PostEvent(EV_LastFrame, 0);
      if(animDoneEvent)
      {
         PostEvent(animDoneEvent, 0);
         animDoneEvent = NULL;
      }
      next_frame -= last_frame_in_anim+1;
   }

   // get the current frame delta
   gi.Frame_Delta(edict->s.modelindex, edict->s.anim, edict->s.frame, frame_delta.vec3());
   total_delta += frame_delta * edict->s.scale;
   cmds = gi.Frame_Commands(edict->s.modelindex, edict->s.anim, edict->s.frame);
   if(cmds)
   {
      for(i = 0; i < cmds->num_cmds; i++)
      {
         ev = new Event(cmds->cmds[i].args[0]);
         for(j = 1; j < cmds->cmds[i].num_args; j++)
         {
            ev->AddToken(cmds->cmds[i].args[j]);
         }
         ProcessEvent(ev);
      }
   }

   last_animation_time = level.time;
   //
   // check to see if we have a secondary animation system going on here
   //
   if(edict->s.gunmodelindex)
   {
      if(num_frames_in_gun_anim > 1)
      {
         edict->s.gunframe = (edict->s.frame * num_frames_in_gun_anim) / (last_frame_in_anim + 1);
      }
      else
      {
         edict->s.gunframe = 0;
      }
   }
}

void Entity::RandomAnimate(const char *animname, Event *endevent)
{
   int num;

   num = gi.Anim_Random(edict->s.modelindex, animname);

   //
   // if we have an event that hasn't been processed, kill the current one
   //
   if(animDoneEvent)
   {
      CancelEventsOfType(animDoneEvent);
      delete animDoneEvent;
      animDoneEvent = NULL;
   }
   //
   // see if we even have a valid animation at all
   //
   if(num == -1)
   {
      if(endevent)
      {
         PostEvent(endevent, FRAMETIME);
      }

      animDoneEvent = NULL;
      return;
   }

   NextAnim(num);

   animDoneEvent = endevent;
   if(!animating)
   {
      StartAnimating();
   }
   last_animation_time = -1;
}

EXPORT_FROM_DLL void Entity::joinTeam(Entity *teammember)
{
   Entity *ent;
   Entity *master;
   Entity *prev;
   Entity *next;

   if(teammaster && (teammaster != this))
   {
      quitTeam();
   }

   assert(teammember);
   if(!teammember)
   {
      warning("joinTeam", "Null entity");
      return;
   }

   master = teammember->teammaster;
   if(!master)
   {
      master = teammember;
      teammember->teammaster = teammember;
      teammember->teamchain = this;

      // make anyone who's bound to me part of the new team
      for(ent = teamchain; ent != NULL; ent = ent->teamchain)
      {
         ent->teammaster = master;
      }
   }
   else
   {
      // skip past the chain members bound to the entity we're teaming up with
      prev = teammember;
      next = teammember->teamchain;
      if(bindmaster)
      {
         // if we have a bindmaster, joing after any entities bound to the entity
         // we're joining
         while(next && next->isBoundTo(teammember))
         {
            prev = next;
            next = next->teamchain;
         }
      }
      else
      {
         // if we're not bound to someone, then put us at the end of the team
         while(next)
         {
            prev = next;
            next = next->teamchain;
         }
      }

      // make anyone who's bound to me part of the new team and
      // also find the last member of my team
      for(ent = this; ent->teamchain != NULL; ent = ent->teamchain)
      {
         ent->teamchain->teammaster = master;
      }

      prev->teamchain = this;
      ent->teamchain = next;
   }

   teammaster = master;
   flags |= FL_TEAMSLAVE;
}

EXPORT_FROM_DLL void Entity::quitTeam(void)
{
   Entity *ent;

   if(!teammaster)
   {
      return;
   }

   if(teammaster == this)
   {
      if(!teamchain->teamchain)
      {
         teamchain->teammaster = NULL;
      }
      else
      {
         // make next teammate the teammaster
         for(ent = teamchain; ent; ent = ent->teamchain)
         {
            ent->teammaster = teamchain;
         }
      }

      teamchain->flags &= ~FL_TEAMSLAVE;
   }
   else
   {
      assert(flags & FL_TEAMSLAVE);
      assert(teammaster->teamchain);

      ent = teammaster;
      while(ent->teamchain != this)
      {
         // this should never happen
         assert(ent->teamchain);

         ent = ent->teamchain;
      }

      ent->teamchain = teamchain;

      if(!teammaster->teamchain)
      {
         teammaster->teammaster = NULL;
      }
   }

   teammaster = NULL;
   teamchain = NULL;
   flags &= ~FL_TEAMSLAVE;
}

EXPORT_FROM_DLL void Entity::EventQuitTeam(Event *ev)
{
   quitTeam();
}

qboolean Entity::isBoundTo(Entity *master) const
{
   for(Entity *ent = bindmaster; ent != nullptr; ent = ent->bindmaster)
   {
      if(ent == master)
      {
         return true;
      }
   }

   return false;
}

EXPORT_FROM_DLL void Entity::bind(Entity *master)
{
   float  mat[3][3];
   float  local[3][3];
   Vector ang;

   assert(master);
   if(!master)
   {
      warning("bind", "Null master entity");
      return;
   }

   if(master == this)
   {
      warning("bind", "Trying to bind to oneself.");
      return;
   }

   // unbind myself from my master
   unbind();

   bindmaster = master;

   // We are now separated from our previous team and are either
   // an individual, or have a team of our own.  Now we can join
   // the new bindmaster's team.  Bindmaster must be set before
   // joining the team, or we will be placed in the wrong position
   // on the team.
   joinTeam(master);

   // calculate local angles
   TransposeMatrix(bindmaster->orientation, mat);
   R_ConcatRotations(mat, orientation, local);
   MatrixToEulerAngles(local, ang.vec3());
   setAngles(ang);

   setOrigin(getParentVector(origin - bindmaster->worldorigin));

   return;
}

EXPORT_FROM_DLL void Entity::unbind(void)
{
   Entity *prev;
   Entity *next;
   Entity *last;
   Entity *ent;

   if(!bindmaster)
   {
      return;
   }

   //bindmaster = NULL;

   origin = Vector(edict->s.origin);
   angles = Vector(edict->s.angles);

   if(!teammaster)
   {
      bindmaster = NULL;
      //Teammaster already has been freed
      return;
   }

   // We're still part of a team, so that means I have to extricate myself
   // and any entities that are bound to me from the old team.
   // Find the node previous to me in the team
   prev = teammaster;

   for(ent = teammaster->teamchain; ent && (ent != this); ent = ent->teamchain)
   {
      prev = ent;
   }

   // If ent is not pointing to me, then something is very wrong.
   assert(ent);
   if(!ent)
   {
      error("unbind", "corrupt team chain\n");
   }

   // Find the last node in my team that is bound to me.
   // Also find the first node not bound to me, if one exists.
   last = this;
   for(next = teamchain; next != NULL; next = next->teamchain)
   {
      if(!next->isBoundTo(this))
      {
         break;
      }

      // Tell them I'm now the teammaster
      next->teammaster = this;
      last = next;
   }

   // disconnect the last member of our team from the old team
   last->teamchain = NULL;

   // connect up the previous member of the old team to the node that
   // follow the last node bound to me (if one exists).
   if(teammaster != this)
   {
      prev->teamchain = next;
      if(!next && (teammaster == prev))
      {
         prev->teammaster = NULL;
      }
   }
   else if(next)
   {
      // If we were the teammaster, then the nodes that were not bound to me are now
      // a disconnected chain.  Make them into their own team.
      for(ent = next; ent->teamchain != NULL; ent = ent->teamchain)
      {
         ent->teammaster = next;
      }
      next->teammaster = next;
      next->flags &= ~FL_TEAMSLAVE;
   }

   // If we don't have anyone on our team, then clear the team variables.
   if(teamchain)
   {
      // make myself my own team
      teammaster = this;
   }
   else
   {
      // no longer a team
      teammaster = NULL;
   }

   flags &= ~FL_TEAMSLAVE;
   bindmaster = NULL;
}

EXPORT_FROM_DLL void Entity::EventUnbind(Event *ev)
{
   unbind();
}

EXPORT_FROM_DLL void Entity::FadeOut(Event *ev)
{
   PostEvent(EV_FadeOut, 0.1f);

   edict->s.renderfx |= RF_TRANSLUCENT;
   translucence += 0.03f;
   if(translucence >= 0.96f)
   {
      PostEvent(EV_Remove, 0);
   }

   setAlpha(1.0f - translucence);
}

EXPORT_FROM_DLL void Entity::Fade(Event *ev)
{
   float rate = ev->GetFloat(1);

   edict->s.renderfx |= RF_TRANSLUCENT;
   translucence += rate;
   setAlpha(1.0f - translucence);

   if(translucence <= 1)
      PostEvent(EV_FadeOut, 0.1f);
}

EXPORT_FROM_DLL void Entity::SetMassEvent(Event *ev)
{
   mass = ev->GetFloat(1);
}

void Entity::CheckGround()
{
   Vector	point;
   trace_t	trace;
   const gravityaxis_t &grav = gravity_axis[gravaxis];

   if(flags & (FL_SWIM | FL_FLY))
   {
      return;
   }

   if(velocity[grav.z] > 100)
   {
      groundentity = NULL;
      return;
   }

   // if the hull point one-quarter unit down is solid the entity is on ground
   point = worldorigin;
   point[grav.z] -= 0.25 * grav.sign;
   trace = G_Trace(worldorigin, mins, maxs, point, this, MASK_MONSTERSOLID, "Entity::CheckGround");

   // check steepness
   if(((trace.plane.normal[grav.z] * grav.sign) <= 0.7) && !trace.startsolid)
   {
      groundentity = NULL;
      return;
   }

   groundentity = trace.ent;
   groundentity_linkcount = trace.ent->linkcount;
   groundplane = trace.plane;
   groundsurface = trace.surface;
   groundcontents = trace.contents;

   if(!trace.startsolid && !trace.allsolid)
   {
      setOrigin(trace.endpos);
      velocity[grav.z] = 0;
   }
}

EXPORT_FROM_DLL void Entity::BecomeSolid(Event *ev)
{
   if((model.length()) && ((model[0] == '*') || (strstr(model.c_str(), ".bsp"))))
   {
      setSolidType(SOLID_BSP);
   }
   else
   {
      setSolidType(SOLID_BBOX);
   }
}

EXPORT_FROM_DLL void Entity::BecomeNonSolid(Event *ev)
{
   setSolidType(SOLID_NOT);
}

EXPORT_FROM_DLL void Entity::Ghost(Event *ev)
{
   // Make not solid, but send still send over whether it is hidden or not
   setSolidType(SOLID_NOT);
   edict->svflags &= ~SVF_NOCLIENT;
}

EXPORT_FROM_DLL void Entity::PlaySound(Event *ev)
{
   char name[128];
   float volume;
   int channel;
   float attenuation;
   float pitch;
   float timeofs;
   float fadetime;
   int flags;
   int i;

   //
   // set defaults
   //
   name[0] = 0;
   volume = 1.0f;
   channel = CHAN_BODY;
   attenuation = ATTN_NORM;
   pitch = 1.0f;
   timeofs = 0;
   fadetime = 0;
   flags = SOUND_SYNCH;
   for(i = 1; i <= ev->NumArgs(); i++)
   {
      switch(i-1)
      {
      case 0:
         strcpy(name, ev->GetString(i));
         break;
      case 1:
         volume = ev->GetFloat(i);
         break;
      case 2:
         channel = ev->GetInteger(i);
         break;
      case 3:
         attenuation = ev->GetFloat(i);
         break;
      case 4:
         pitch = ev->GetFloat(i);
         break;
      case 5:
         timeofs = ev->GetFloat(i);
         break;
      case 6:
         fadetime = ev->GetFloat(i);
         break;
      case 7:
         flags = ev->GetInteger(i);
         break;
      default:
         break;
      }
   }
   channel |= CHAN_NO_PHS_ADD;
   sound(name, volume, channel, attenuation, pitch, timeofs, fadetime, flags);
}

EXPORT_FROM_DLL void Entity::StopSound(Event *ev)
{
   if(ev->NumArgs() < 1)
      stopsound(CHAN_BODY);
   else
      stopsound(ev->GetInteger(1));
}

EXPORT_FROM_DLL void Entity::SetGravityAxis(int axis)
{
   Vector min;
   Vector max;

   if((axis < 0) || (axis > 5))
   {
      axis = 0;
   }

   // don't do anything if the axis has been already set
   if(axis == gravaxis)
      return;

   edict->s.effects &= ~(EF_GRAVITY_AXIS_0 | EF_GRAVITY_AXIS_1 | EF_GRAVITY_AXIS_2);
   edict->s.effects |= GRAVITYAXIS_TO_EFFECTS(axis);
   gravaxis = EFFECTS_TO_GRAVITYAXIS(edict->s.effects);
   groundentity = NULL;

   const gravityaxis_t &grav = gravity_axis[gravaxis];

   min[grav.x] = mins[0];
   min[grav.y] = mins[1] * grav.sign;
   min[grav.z] = mins[2] * grav.sign;
   max[grav.x] = maxs[0];
   max[grav.y] = maxs[1] * grav.sign;
   max[grav.z] = maxs[2] * grav.sign;

   setSize(min, max);
}

EXPORT_FROM_DLL void Entity::GravityAxisEvent(Event *ev)
{
   SetGravityAxis(ev->GetInteger(1));
}

EXPORT_FROM_DLL void Entity::BindEvent(Event *ev)
{
   Entity *ent;

   ent = ev->GetEntity(1);
   if(ent)
   {
      bind(ent);
   }
}

EXPORT_FROM_DLL void Entity::JoinTeam(Event *ev)
{
   Entity *ent;

   ent = ev->GetEntity(1);
   if(ent)
   {
      joinTeam(ent);
   }
}

EXPORT_FROM_DLL void Entity::SetLight(Event *ev)
{
   edict->s.renderfx |= RF_DLIGHT;
   edict->s.color_r = ev->GetFloat(1);
   edict->s.color_g = ev->GetFloat(2);
   edict->s.color_b = ev->GetFloat(3);
   edict->s.radius  = ev->GetFloat(4);
}

EXPORT_FROM_DLL void Entity::LightOn(Event *ev)
{
   edict->s.renderfx |= RF_DLIGHT;
}

EXPORT_FROM_DLL void Entity::LightOff(Event *ev)
{
   edict->s.renderfx &= ~RF_DLIGHT;
}

EXPORT_FROM_DLL void Entity::LightRed(Event *ev)
{
   edict->s.renderfx |= RF_DLIGHT;
   edict->s.color_r = ev->GetFloat(1);
}

EXPORT_FROM_DLL void Entity::LightGreen(Event *ev)
{
   edict->s.renderfx |= RF_DLIGHT;
   edict->s.color_g = ev->GetFloat(1);
}

EXPORT_FROM_DLL void Entity::LightBlue(Event *ev)
{
   edict->s.renderfx |= RF_DLIGHT;
   edict->s.color_b = ev->GetFloat(1);
}

EXPORT_FROM_DLL void Entity::LightRadius(Event *ev)
{
   edict->s.renderfx |= RF_DLIGHT;
   edict->s.radius = ev->GetFloat(1);
}

EXPORT_FROM_DLL void Entity::SetHealth(Event *ev)
{
   health = ev->GetFloat(1);
   max_health = health;
}

EXPORT_FROM_DLL void Entity::SetSize(Event *ev)
{
   Vector min, max;

   min = ev->GetVector(1);
   max = ev->GetVector(2);
   setSize(min, max);
}

EXPORT_FROM_DLL void Entity::SetScale(Event *ev)
{
   setScale(ev->GetFloat(1));
}

EXPORT_FROM_DLL void Entity::SetAlpha(Event *ev)
{
   setAlpha(ev->GetFloat(1));
}

EXPORT_FROM_DLL void Entity::SetOrigin(Event *ev)
{
   setOrigin(ev->GetVector(1));
}

EXPORT_FROM_DLL void Entity::SetTargetName(Event *ev)
{
   SetTargetName(ev->GetString(1));
}

EXPORT_FROM_DLL void Entity::SetTarget(Event *ev)
{
   SetTarget(ev->GetString(1));
}

EXPORT_FROM_DLL void Entity::SetKillTarget(Event *ev)
{
   SetKillTarget(ev->GetString(1));
}

EXPORT_FROM_DLL void Entity::SetAngles(Event *ev)
{
   setAngles(ev->GetVector(1));
}

EXPORT_FROM_DLL void Entity::CourseAnglesEvent(Event *ev)
{
   // Angles will be sent over the net as 8-bit values (default)
   edict->s.effects &= ~EF_SMOOTHANGLES;
}

EXPORT_FROM_DLL void Entity::SmoothAnglesEvent(Event *ev)
{
   // Angles will be sent over the net as 16-bit values for smoother rotation (or slow rotation)
   edict->s.effects |= EF_SMOOTHANGLES;
}

EXPORT_FROM_DLL void Entity::RegisterAlias(Event *ev)
{
   if(ev->NumArgs() < 3)
   {
      gi.Alias_Add(edict->s.modelindex, ev->GetString(1), ev->GetString(2), 1);
   }
   else
   {
      gi.Alias_Add(edict->s.modelindex, ev->GetString(1), ev->GetString(2), ev->GetInteger(3));
   }
}

EXPORT_FROM_DLL void Entity::RegisterAliasAndCache(Event *ev)
{
   int length;
   str realname;
   const char * ptr;

   realname = ev->GetString(2);

   if(ev->NumArgs() < 3)
   {
      gi.Alias_Add(edict->s.modelindex, ev->GetString(1), realname.c_str(), 1);
   }
   else
   {
      gi.Alias_Add(edict->s.modelindex, ev->GetString(1), realname.c_str(), ev->GetInteger(3));
   }

   length = realname.length();
   ptr = realname.c_str();
   ptr += length - 4;
   if((length > 4) && (!strcmpi(ptr, ".wav")))
   {
      gi.soundindex(realname.c_str());
   }
   else if((length > 4) && (!strcmpi(ptr, ".def")))
   {
      gi.modelindex(realname.c_str());
   }
}

EXPORT_FROM_DLL void Entity::positioned_sound(Vector origin, str soundname, float volume, int channel, int attenuation, 
                                              float pitch, float timeofs, float fadetime, int flags)
{
   if(soundname.length())
   {
      gi.positioned_sound(worldorigin.vec3(), edict, channel, gi.soundindex(soundname.c_str()),
                          volume, attenuation, timeofs, pitch, fadetime, flags);
   }
   else
   {
      warning("sound", "Null sample pointer");
   }
}

EXPORT_FROM_DLL void Entity::RandomPositionedSound(Vector origin, str soundname, float volume, int channel, int attenuation, 
                                                   float pitch, float timeofs, float fadetime, int flags)
{
   const char * name;

   name = gi.GlobalAlias_FindRandom(soundname.c_str());
   if(name)
   {
      positioned_sound(worldorigin, name, volume, channel, attenuation, pitch, timeofs, fadetime, flags);
   }
   else
   {
      warning("RandomPositionedSound", "Couldn't find alias for %s", soundname.c_str());
   }
}

EXPORT_FROM_DLL void Entity::sound(str soundname, float volume, int channel, int attenuation, 
                                   float pitch, float timeofs, float fadetime, int flags)
{
   if(soundname.length())
   {
      gi.sound(edict, channel, gi.soundindex(soundname.c_str()), volume,
               attenuation, timeofs, pitch, fadetime, flags);
   }
   else
   {
      warning("sound", "Null sample pointer");
   }
}

EXPORT_FROM_DLL void Entity::RandomGlobalSound(str soundname, float volume, int channel, int attenuation, 
                                               float pitch, float timeofs, float fadetime, int flags)
{
   const char * name;

   name = gi.GlobalAlias_FindRandom(soundname.c_str());
   if(name)
   {
      sound(name, volume, channel, attenuation, pitch, timeofs, fadetime, flags);
   }
   else
   {
      warning("RandomGlobalSound", "Couldn't find alias for %s", soundname.c_str());
   }
}

EXPORT_FROM_DLL void Entity::RandomSound(str soundname, float volume, int channel, int attenuation,
                                         float pitch, float timeofs, float fadetime, int flags)
{
   str realname;

   realname = GetRandomAlias(soundname);
   sound(realname, volume, channel, attenuation, pitch, timeofs, fadetime, flags);
}

EXPORT_FROM_DLL void Entity::RandomSound(Event *ev)
{
   str name;
   float volume;
   int   channel;
   float attenuation;
   float pitch;
   float timeofs;
   float fadetime;
   int flags;
   int i;

   //
   // set defaults
   //
   volume = 1.0f;
   channel = CHAN_BODY;
   attenuation = ATTN_NORM;
   pitch = 1.0f;
   timeofs = 0;
   fadetime = 0;
   flags = SOUND_SYNCH;
   for(i = 1; i <= ev->NumArgs(); i++)
   {
      switch(i-1)
      {
      case 0:
         name = ev->GetString(i);
         break;
      case 1:
         volume = ev->GetFloat(i);
         break;
      case 2:
         channel = ev->GetInteger(i);
         break;
      case 3:
         attenuation = ev->GetFloat(i);
         break;
      case 4:
         pitch = ev->GetFloat(i);
         break;
      case 5:
         timeofs = ev->GetFloat(i);
         break;
      case 6:
         fadetime = ev->GetFloat(i);
         break;
      case 7:
         flags = ev->GetInteger(i);
         break;
      default:
         break;
      }
   }
   channel |= CHAN_NO_PHS_ADD;
   RandomSound(name, volume, channel, attenuation, pitch, timeofs, fadetime, flags);
}

EXPORT_FROM_DLL void Entity::RandomPHSSound(Event *ev)
{
   str name;
   float volume;
   int   channel;
   float attenuation;
   float pitch;
   float timeofs;
   float fadetime;
   int flags;
   int i;

   //
   // set defaults
   //
   volume = 1.0f;
   channel = CHAN_BODY;
   attenuation = ATTN_NORM;
   pitch = 1.0f;
   timeofs = 0;
   fadetime = 0;
   flags = SOUND_SYNCH;
   for(i = 1; i <= ev->NumArgs(); i++)
   {
      switch(i-1)
      {
      case 0:
         name = ev->GetString(i);
         break;
      case 1:
         volume = ev->GetFloat(i);
         break;
      case 2:
         channel = ev->GetInteger(i);
         break;
      case 3:
         attenuation = ev->GetFloat(i);
         break;
      case 4:
         pitch = ev->GetFloat(i);
         break;
      case 5:
         timeofs = ev->GetFloat(i);
         break;
      case 6:
         fadetime = ev->GetFloat(i);
         break;
      case 7:
         flags = ev->GetInteger(i);
         break;
      default:
         break;
      }
   }
   RandomSound(name, volume, channel, attenuation, pitch, timeofs, fadetime, flags);
}

EXPORT_FROM_DLL void Entity::PHSSound(Event *ev)
{
   char name[128];
   float volume;
   int channel;
   float attenuation;
   float pitch;
   float timeofs;
   float fadetime;
   int flags;
   int i;

   //
   // set defaults
   //
   name[0] = 0;
   volume = 1.0f;
   channel = CHAN_BODY;
   attenuation = ATTN_NORM;
   pitch = 1.0f;
   timeofs = 0;
   fadetime = 0;
   flags = SOUND_SYNCH;
   for(i = 1; i <= ev->NumArgs(); i++)
   {
      switch(i-1)
      {
      case 0:
         strcpy(name, ev->GetString(i));
         break;
      case 1:
         volume = ev->GetFloat(i);
         break;
      case 2:
         channel = ev->GetInteger(i);
         break;
      case 3:
         attenuation = ev->GetFloat(i);
         break;
      case 4:
         pitch = ev->GetFloat(i);
         break;
      case 5:
         timeofs = ev->GetFloat(i);
         break;
      case 6:
         fadetime = ev->GetFloat(i);
         break;
      case 7:
         flags = ev->GetInteger(i);
         break;
      default:
         break;
      }
   }
   sound(name, volume, channel, attenuation, pitch, timeofs, fadetime, flags);
}

EXPORT_FROM_DLL void Entity::EntitySound(Event *ev)
{
   edict->s.sound = gi.soundindex(ev->GetString(1));
   if(ev->NumArgs() > 1)
   {
      int attenuation;
      attenuation = ev->GetInteger(2);
      if(attenuation > 3) attenuation = 3;
      if(attenuation < 0) attenuation = 0;
      edict->s.sound |= attenuation<<14;
   }
   else
   {
      edict->s.sound |= ATTN_IDLE<<14;
   }
}

EXPORT_FROM_DLL void Entity::StopEntitySound(Event *ev)
{
   edict->s.sound = 0;
}

EXPORT_FROM_DLL void Entity::RandomEntitySound(Event *ev)
{
   const char * alias;
   const char * soundname;

   alias = ev->GetString(1);

   soundname = gi.Alias_FindRandom(edict->s.modelindex, alias);
   edict->s.sound = gi.soundindex(soundname);
   if(ev->NumArgs() > 1)
   {
      int attenuation;
      attenuation = ev->GetInteger(2);
      if(attenuation > 3) attenuation = 3;
      if(attenuation < 0) attenuation = 0;
      edict->s.sound |= attenuation<<14;
   }
   else
   {
      edict->s.sound |= ATTN_IDLE<<14;
   }
}

EXPORT_FROM_DLL void Entity::RandomGlobalEntitySound(str soundname, int attenuation)
{
   const char * name;

   name = gi.GlobalAlias_FindRandom(soundname.c_str());
   if(name)
   {
      edict->s.sound = gi.soundindex(name);

      bound(attenuation, 0, 3);
      edict->s.sound |= attenuation<<14;
   }
   else
   {
      warning("RandomGlobalEntitySound", "Couldn't find alias for %s", soundname.c_str());
   }
}

EXPORT_FROM_DLL void Entity::RandomGlobalEntitySoundEvent(Event *ev)
{
   const char *alias;
   int attenuation;

   alias = ev->GetString(1);

   attenuation = ATTN_IDLE;
   if(ev->NumArgs() > 1)
   {
      attenuation = ev->GetInteger(2);
   }

   RandomGlobalEntitySound(alias, attenuation);
}

EXPORT_FROM_DLL qboolean Entity::attach(int parent_entity_num, int group_num, int tri_num, Vector orient)
{
   int i;
   Entity * parent;

   if(entnum == parent_entity_num)
   {
      warning("attach", "Trying to attach to oneself.");
      return false;
   }

   if(edict->s.parent)
      detach();
   //
   // get the parent
   //
   parent = (Entity *)G_GetEntity(parent_entity_num);

   if(parent->numchildren < MAX_MODEL_CHILDREN)
   {
      //
      // find a free spot in the parent
      //
      for(i=0; i < MAX_MODEL_CHILDREN; i++)
         if(!parent->children[i])
         {
            break;
         }
      parent->children[i] = entnum;
      parent->numchildren++;
      edict->s.parent = parent_entity_num;
      edict->s.bone.group_num = group_num;
      edict->s.bone.tri_num = tri_num;
      VectorCopy(orient.vec3(), edict->s.bone.orientation);
      setOrigin(origin);
      return true;
   }
   return false;
}

EXPORT_FROM_DLL void Entity::detach(void)
{
   int i;
   int num;
   Entity * parent;

   if(!edict->s.parent)
      return;
   parent = (Entity *)G_GetEntity(edict->s.parent);
   if(!parent)
      return;
   for(i=0, num = parent->numchildren; i < MAX_MODEL_CHILDREN; i++)
   {
      if(!parent->children[i])
      {
         continue;
      }
      if(parent->children[i] == entnum)
      {
         parent->children[i] = 0;
         parent->numchildren--;
         break;
      }
      num--;
      if(!num)
         break;
   }
   edict->s.parent = 0;

   //
   // i don't think we want to do this automatically, we might later, but for right now lets not
   //
   //setOrigin( edict->origin + parent->origin );
}

void Entity::AnimEvent(Event *ev)
{
   int num;

   num = gi.Anim_Random(edict->s.modelindex, ev->GetString(1));
   NextAnim(num);
   if(!animating)
   {
      StartAnimating();
   }
}

void Entity::StartAnimatingEvent(Event *ev)
{
   StartAnimating();
}

void Entity::StopAnimatingEvent(Event *ev)
{
   StopAnimating();
}

void Entity::EndAnimEvent(Event *ev)
{
   PostEvent(EV_LastFrame, 0);
   if(animDoneEvent)
   {
      PostEvent(animDoneEvent, 0);
      animDoneEvent = NULL;
   }
   next_frame = 0;
}

void Entity::NextAnimEvent(Event *ev)
{
   int num;

   num = gi.Anim_Random(edict->s.modelindex, ev->GetString(1));
   NextAnim(num);
}

void Entity::NextFrameEvent(Event *ev)
{
   NextFrame(ev->GetInteger(1));
}

void Entity::PrevFrameEvent(Event *ev)
{
   edict->s.prevframe = ev->GetInteger(1);
}

void Entity::SetFrameEvent(Event *ev)
{
   int framenum;

   framenum = ev->GetInteger(1);
   edict->s.frame = framenum;
   NextFrame(framenum + 1);
}

void Entity::Tesselate(Event *ev)
{
   Vector origin, dir, temp;
   Entity * ent;
   int i, power, min_size, max_size, thickness;
   float percentage;

   // dir is 1
   // power is 2
   // minsize is 3
   // maxsize is 4
   // percentage is 5
   // thickness 6
   // entity is 7
   // origin 8

   //
   // initialize some variables
   //
   ent = this;
   min_size = TESS_DEFAULT_MIN_SIZE;
   max_size = TESS_DEFAULT_MIN_SIZE;
   thickness = min_size;
   percentage = TESS_DEFAULT_PERCENT;
   VectorCopy(vec3_origin, origin);
   VectorCopy(vec3_origin, dir);

   for(i = 1; i <= ev->NumArgs(); i++)
   {
      switch(i)
      {
      case 1:
         temp = ev->GetVector(i);
         temp.AngleVectors(&dir, NULL, NULL);
         break;
      case 2:
         power = ev->GetInteger(i);
         break;
      case 3:
         min_size = ev->GetInteger(i);
         break;
      case 4:
         max_size = ev->GetInteger(i);
         break;
      case 5:
         percentage = ev->GetFloat(i);
         break;
      case 6:
         thickness = ev->GetInteger(i);
         break;
      case 7:
         ent = ev->GetEntity(i);
         break;
      case 8:
         origin = ev->GetVector(i);
         break;
      }
   }
   TesselateModel
   (
      ent,
      min_size,
      max_size,
      dir,
      power,
      percentage,
      thickness,
      origin
   );
}

void Entity::SetShatterMinSize(Event *ev)
{
   tess_min_size = ev->GetInteger(1);
}

void Entity::SetShatterMaxSize(Event *ev)
{
   tess_max_size = ev->GetInteger(1);
}

void Entity::SetShatterThickness(Event *ev)
{
   tess_thickness = ev->GetInteger(1);
}

void Entity::SetShatterPercentage(Event *ev)
{
   tess_percentage = ev->GetFloat(1) / 100.0f;;
}

void Entity::Flags(Event *ev)
{
   const char *flag;
   int mask;
   int action;
   int i;

#define FLAG_IGNORE  0
#define FLAG_SET     1
#define FLAG_CLEAR   2
#define FLAG_ADD     3

   for(i = 1; i <= ev->NumArgs(); i++)
   {
      action = FLAG_IGNORE;
      flag = ev->GetString(i);
      switch(flag[0])
      {
      case '+':
         action = FLAG_ADD;
         flag++;
         break;
      case '-':
         action = FLAG_CLEAR;
         flag++;
         break;
      default:
         action = FLAG_SET;
         break;
      }

      if(!stricmp(flag, "blood"))
         mask = FL_BLOOD;
      else if(!stricmp(flag, "sparks"))
         mask = FL_SPARKS;
      else if(!stricmp(flag, "shatter"))
         mask = FL_TESSELATE;
      else if(!stricmp(flag, "blast"))
         mask = FL_BLASTMARK;
      else if(!stricmp(flag, "die_shatter"))
         mask = FL_DIE_TESSELATE;
      else if(!stricmp(flag, "explode"))
         mask = FL_DIE_EXPLODE;
      else if(!stricmp(flag, "die_gibs"))
         mask = FL_DIE_GIBS;
      else if(!stricmp(flag, "darken"))
         mask = FL_DARKEN;
      else if(!stricmp(flag, "forcefield"))
         mask = FL_FORCEFIELD;
      else if(!stricmp(flag, "stealth"))
         mask = FL_STEALTH;
      else
      {
         action = FLAG_IGNORE;
         ev->Error("Unknown flag '%s'", flag);
      }
      switch(action)
      {
      case FLAG_SET:
         // preserver non-configurable bits
         flags &= (FL_BLOOD - 1);
      case FLAG_ADD:
         flags |= mask;
         break;
      case FLAG_CLEAR:
         flags &= ~mask;
         break;
      case FLAG_IGNORE:
         break;
      }
   }
   if(parentmode->value)
   {
      if(flags & (FL_BLOOD|FL_DIE_GIBS))
      {
         flags &= ~FL_BLOOD;
         flags &= ~FL_DIE_GIBS;
      }
   }
}

void Entity::Effects(Event *ev)
{
   const char *flag;
   int mask=0;
   int action;
   int i;

#define FLAG_IGNORE  0
#define FLAG_SET     1
#define FLAG_CLEAR   2
#define FLAG_ADD     3
   for(i = 1; i <= ev->NumArgs(); i++)
   {
      action = 0;
      flag = ev->GetString(i);
      switch(flag[0])
      {
      case '+':
         action = FLAG_ADD;
         flag++;
         break;
      case '-':
         action = FLAG_CLEAR;
         flag++;
         break;
      default:
         action = FLAG_SET;
         break;
      }

      if(!stricmp(flag, "rotate"))
         mask = EF_ROTATE;
      else if(!stricmp(flag, "rocket"))
         mask = EF_ROCKET;
      else if(!stricmp(flag, "gib"))
         mask = EF_GIB;
      else if(!stricmp(flag, "pulse"))
         mask = EF_PULSE;
      else if(!stricmp(flag, "everyframe"))
         mask = EF_EVERYFRAME;
      else if(!stricmp(flag, "autoanimate"))
         mask = EF_AUTO_ANIMATE;
      else
      {
         action = FLAG_IGNORE;
         ev->Error("Unknown token %s.", flag);
      }

      switch(action)
      {
      case FLAG_SET:
      case FLAG_ADD:
         edict->s.effects |= mask;
         break;
      case FLAG_CLEAR:
         edict->s.effects &= ~mask;
         break;
      case FLAG_IGNORE:
         break;
      }
   }
}

void Entity::RenderEffects(Event *ev)
{
   const char *flag;
   int mask=0;
   int action;
   int i;

#define FLAG_IGNORE  0
#define FLAG_SET     1
#define FLAG_CLEAR   2
#define FLAG_ADD     3

   for(i = 1; i <= ev->NumArgs(); i++)
   {
      action = 0;
      flag = ev->GetString(i);
      switch(flag[0])
      {
      case '+':
         action = FLAG_ADD;
         flag++;
         break;
      case '-':
         action = FLAG_CLEAR;
         flag++;
         break;
      default:
         action = FLAG_SET;
         break;
      }

      if(!stricmp(flag, "minlight"))
         mask = RF_MINLIGHT;
      else if(!stricmp(flag, "fullbright"))
         mask = RF_FULLBRIGHT;
      else if(!stricmp(flag, "envmapped"))
         mask = RF_ENVMAPPED;
      else if(!stricmp(flag, "glow"))
         mask = RF_GLOW;
      else if(!stricmp(flag, "dontdraw"))
         mask = RF_DONTDRAW;
      else
      {
         action = FLAG_IGNORE;
         ev->Error("Unknown token %s.", flag);
      }

      switch(action)
      {
      case FLAG_SET:
      case FLAG_ADD:
         edict->s.renderfx |= mask;
         break;
      case FLAG_CLEAR:
         edict->s.renderfx &= ~mask;
         break;
      case FLAG_IGNORE:
         break;
      }
   }
}

void Entity::BroadcastSound(Event *soundevent, int channel, Event &event, float radius)
{
   Sentient *ent;
   Vector	delta;
   Event		*ev;
   str		name;
   float    r2;
   float    dist2;
   float		volume;
   float		attenuation;
   float		pitch;
   float		timeofs;
   float		fadetime;
   int		flags;
   int		i;
   int		n;
#if 0
   int		count;

   count = 0;
#endif

   if(((int)event != (int)NullEvent) && !(this->flags & FL_NOTARGET))
   {
      r2 = radius * radius;
      n = SentientList.NumObjects();
      for(i = 1; i <= n; i++)
      {
         ent = SentientList.ObjectAt(i);
         if(ent->deadflag || (ent == this))
         {
            continue;
         }

         delta = centroid - ent->centroid;

         // dot product returns length squared
         dist2 = delta * delta;
         if(
            (dist2 <= r2) &&
            (
            (edict->areanum == ent->edict->areanum) ||
               (gi.AreasConnected(edict->areanum, ent->edict->areanum))
               )
            )

         {
            ev = new Event(event);
            ev->AddEntity(this);
            ev->AddVector(worldorigin);
            ent->PostEvent(ev, 0);
#if 0
            count++;
#endif
         }
      }
   }

#if 0
   gi.dprintf("Broadcast sound to %d entities\n", count);
#endif

   if(!soundevent->NumArgs())
   {
      return;
   }

   //
   // set defaults
   //
   volume = 1.0f;
   attenuation = ATTN_NORM;
   pitch = 1.0f;
   timeofs = 0;
   fadetime = 0;
   flags = 0;
   for(i = 1; i <= soundevent->NumArgs(); i++)
   {
      switch(i-1)
      {
      case 0:
         name = soundevent->GetString(i);
         break;
      case 1:
         volume = soundevent->GetFloat(i);
         break;
      case 2:
         attenuation = soundevent->GetFloat(i);
         break;
      case 3:
         pitch = soundevent->GetFloat(i);
         break;
      case 4:
         timeofs = soundevent->GetFloat(i);
         break;
      case 5:
         fadetime = soundevent->GetFloat(i);
         break;
      case 6:
         flags = soundevent->GetInteger(i);
         break;
      default:
         break;
      }
   }

   RandomSound(name.c_str(), volume, channel, attenuation, pitch, timeofs, fadetime, flags);
}

void Entity::WeaponSound(Event *ev)
{
   BroadcastSound(ev, CHAN_WEAPON, EV_HeardWeapon, SOUND_WEAPON_RADIUS);
}

void Entity::MovementSound(Event *ev)
{
   static int movement_count = 0;
   //
   // movement sounds now happen very infrequently, unless this is a client
   //
   if(isClient() || (movement_count++ > 15))
   {
      BroadcastSound(ev, CHAN_BODY, EV_HeardMovement, SOUND_MOVEMENT_RADIUS);
      if(movement_count > 15)
         movement_count = 0;
   }
}

void Entity::PainSound(Event *ev)
{
   BroadcastSound(ev, CHAN_VOICE, EV_HeardPain, SOUND_PAIN_RADIUS);
}

void Entity::DeathSound(Event *ev)
{
   BroadcastSound(ev, CHAN_VOICE, EV_HeardDeath, SOUND_DEATH_RADIUS);
}

void Entity::BreakingSound(Event *ev)
{
   BroadcastSound(ev, CHAN_AUTO, EV_HeardBreaking, SOUND_BREAKING_RADIUS);
}

void Entity::DoorSound(Event *ev)
{
   BroadcastSound(ev, CHAN_AUTO, EV_HeardDoor, SOUND_DOOR_RADIUS);
}

void Entity::MutantSound(Event *ev)
{
   BroadcastSound(ev, CHAN_VOICE, EV_HeardMutant, SOUND_MUTANT_RADIUS);
}

void Entity::VoiceSound(Event *ev)
{
   BroadcastSound(ev, CHAN_VOICE, EV_HeardVoice, SOUND_VOICE_RADIUS);
}

void Entity::MachineSound(Event *ev)
{
   BroadcastSound(ev, CHAN_AUTO, EV_HeardMachine, SOUND_MACHINE_RADIUS);
}

void Entity::RadioSound(Event *ev)
{
   BroadcastSound(ev, CHAN_VOICE, EV_HeardRadio, SOUND_RADIO_RADIUS);
}

void Entity::SpawnParticles(Event *ev)
{
   int i;
   Vector norm;
   int count;
   int lightstyle;

   norm = orientation[0];
   count = 4;
   lightstyle = 122;
   for(i = 1; i <= ev->NumArgs(); i++)
   {
      switch(i)
      {
      case 1:
         norm = ev->GetVector(1);
         break;
      case 2:
         count = ev->GetInteger(2);
         break;
      case 3:
         lightstyle = ev->GetInteger(3);
         break;
      case 4:
         flags = ev->GetInteger(4);
      default:
         break;
      }
   }
   Particles(worldorigin, norm, count, lightstyle, flags);
}

void Entity::Prethink(void)
{
}

void Entity::Postthink(void)
{
}

void Entity::SetWaterType(void)
{
   qboolean isinwater;

   watertype = gi.pointcontents(worldorigin.vec3());
   isinwater = watertype & MASK_WATER;

   if(isinwater)
   {
      waterlevel = 1;
   }
   else
   {
      waterlevel = 0;
   }
}

void Entity::DamageSkin(trace_t *trace, float damage)
{
   int group;

   group = trace->intersect.parentgroup;
   if(!edict->s.groups[group])
   {
      edict->s.groups[group]++;
   }
}

void Entity::Kill(Event *ev)
{
   health = 0;
   Damage(this, this, 10, worldorigin, vec_zero, vec_zero, 0, 0, MOD_SUICIDE, -1, -1, 1.0f);
}

void Entity::GroupModelEvent(Event *ev)
{
   const char * group_name;
   const char * token;
   int i, group_num, argnum, flags;
   int mask;
   int action;
   qboolean do_all;

#define FLAG_IGNORE  0
#define FLAG_SET     1
#define FLAG_CLEAR   2
#define FLAG_ADD     3

   do_all = false;
   // "group" is first
   group_name = ev->GetString(1);
   if(str(group_name) != str("all"))
   {
      group_num = gi.Group_NameToNum(edict->s.modelindex, group_name);
      if(group_num < 0)
      {
         ev->Error("group %s not found.", group_name);
      }
   }
   else
   {
      group_num = 0;
      do_all = true;
   }
   flags = 0;
   argnum = 2;
   for(i = argnum; i <= ev->NumArgs(); i++)
   {
      token = ev->GetString(i);
      action = 0;
      switch(token[0])
      {
      case '+':
         action = FLAG_ADD;
         token++;
         break;
      case '-':
         action = FLAG_CLEAR;
         token++;
         break;
      default:
         action = FLAG_SET;
         break;
      }
      if(!strcmpi(token, "skin1"))
      {
         mask = MDL_GROUP_SKINOFFSET_BIT0;
      }
      else if(!strcmpi(token, "skin2"))
      {
         mask = MDL_GROUP_SKINOFFSET_BIT1;
      }
      else if(!strcmpi(token, "nodraw"))
      {
         mask = MDL_GROUP_NODRAW;
      }
      else if(!strcmpi(token, "envmapped"))
      {
         mask = MDL_GROUP_ENVMAPPED_SILVER;
      }
      else if(!strcmpi(token, "goldenvmapped"))
      {
         mask = MDL_GROUP_ENVMAPPED_GOLD;
      }
      else if(!strcmpi(token, "translucent33"))
      {
         mask = MDL_GROUP_TRANSLUCENT_33;
      }
      else if(!strcmpi(token, "translucent66"))
      {
         mask = MDL_GROUP_TRANSLUCENT_66;
      }
      else if(!strcmpi(token, "fullbright"))
      {
         mask = MDL_GROUP_FULLBRIGHT;
      }
      else
      {
         ev->Error("Unknown token %s.", token);
         action = FLAG_IGNORE;
      }
      for(; group_num < edict->s.numgroups; group_num++)
      {
         switch(action)
         {
         case FLAG_SET:
            // clear out group
            edict->s.groups[group_num] = 0;
         case FLAG_ADD:
            edict->s.groups[group_num] |= mask;
            break;
         case FLAG_CLEAR:
            edict->s.groups[group_num] &= ~mask;
            break;
         case FLAG_IGNORE:
            break;
         }
         if(!do_all)
            break;
      }
   }
}

void Entity::DialogEvent(Event * ev)
{
   str name;
   float volume;
   int   channel;
   float attenuation;
   float pitch;
   float timeofs;
   float fadetime;
   int flags;
   int i;
   str icon_name;
   str dialog_text;

   if((dialog->value == 1) || (dialog->value == 3))
   {
      icon_name = ev->GetString(1);
      dialog_text = ev->GetString(2);
      SendDialog(icon_name.c_str(), dialog_text.c_str());
   }

   if((dialog->value == 0) || (dialog->value == 1))
      return;

   //
   // set defaults
   //
   volume = 1.0f;
   channel = CHAN_DIALOG_SECONDARY | CHAN_NO_PHS_ADD;
   attenuation = ATTN_NORM;
   pitch = 1.0f;
   timeofs = 0;
   fadetime = 0;
   flags = SOUND_SYNCH;
   for(i = 3; i <= ev->NumArgs(); i++)
   {
      switch(i-3)
      {
      case 0:
         name = ev->GetString(i);
         break;
      case 1:
         volume = ev->GetFloat(i);
         break;
      case 2:
         channel = ev->GetInteger(i);
         break;
      case 3:
         attenuation = ev->GetFloat(i);
         break;
      case 4:
         pitch = ev->GetFloat(i);
         break;
      case 5:
         timeofs = ev->GetFloat(i);
         break;
      case 6:
         fadetime = ev->GetFloat(i);
         break;
      case 7:
         flags = ev->GetInteger(i);
         break;
      default:
         break;
      }
   }
   sound(name, volume, channel, attenuation, pitch, timeofs, fadetime, flags);
}

void Entity::AttachEvent(Event * ev)
{
   Entity * parent;
   const char * bone;
   int groupindex;
   int tri_num;
   vec3_t orient;

   parent = ev->GetEntity(1);
   bone = ev->GetString(2);

   if(!parent)
      return;

   if(gi.GetBoneInfo(parent->edict->s.modelindex, bone, &groupindex, &tri_num, orient))
   {
      attach(parent->entnum, groupindex, tri_num, Vector(orient));
   }
   else
   {
      warning("AttachEvent", "GetBoneInfo failed for bone %s", bone);
   }
}

void Entity::AttachModelEvent(Event *ev)
{
   ThrowObject * tobj;
   const char * bone;
   int groupindex;
   int tri_num;
   vec3_t orient;
   str modelname;

   tobj = new ThrowObject();

   modelname = ev->GetString(1);
   bone = ev->GetString(2);
   if(ev->NumArgs() > 2)
   {
      tobj->SetTargetName(ev->GetString(3));
   }

   tobj->setModel(modelname);

   if(gi.GetBoneInfo(edict->s.modelindex, bone, &groupindex, &tri_num, orient))
   {
      tobj->attach(this->entnum, groupindex, tri_num, Vector(orient));
   }
   else
   {
      warning("AttachModelEvent", "GetBoneInfo failed for bone %s", bone);
   }
}

void Entity::DetachEvent(Event * ev)
{
   float trans[3][3];
   Entity * ent;
   vec3_t p1, p2;

   if(edict->s.parent <= 0)
   {
      return;
   }

   ent = (Entity *)G_GetEntity(edict->s.parent);

   if(gi.GetBoneTransform(ent->edict->s.modelindex, edict->s.bone.group_num, edict->s.bone.tri_num, edict->s.bone.orientation,
                          ent->edict->s.anim, ent->edict->s.frame, ent->edict->s.scale, trans, p1))
   {
      VectorAdd(p1, origin.vec3(), p1);
      MatrixTransformVector(p1, ent->orientation, p2);
      VectorAdd(p2, ent->worldorigin, p2);
   }

   detach();

   setOrigin(p2);
   worldorigin.copyTo(edict->s.old_origin);
}

void Entity::TakeDamageEvent(Event * ev)
{
   takedamage = DAMAGE_YES;
}

void Entity::NoDamageEvent(Event * ev)
{
   takedamage = DAMAGE_NO;
}

void Entity::SetSkinEvent(Event *ev) 
{
   int num;

   num = gi.Skin_NumForName(edict->s.modelindex, ev->GetString(1));
   if(num >= 0)
   {
      edict->s.skinnum = num;
   }
   else
   {
      ev->Error("Could not find %s as a skin name.\n", ev->GetString(1));
   }
}

void Entity::Lightoffset(Event *ev)
{
   edict->s.lightofs = ev->GetFloat(1);

   if(edict->s.lightofs != 0)
      edict->s.renderfx |= RF_LIGHTOFFSET;
   else
      edict->s.renderfx &= ~RF_LIGHTOFFSET;
}

void Entity::Gravity(Event *ev)
{
   gravity = ev->GetFloat(1);
}

void Entity::Minlight(Event *ev)
{

   if(ev->GetInteger(1))
      edict->s.renderfx |= RF_MINLIGHT;
   else
      edict->s.renderfx &= ~RF_MINLIGHT;
}

void Entity::UseBoundingBoxEvent(Event *ev)
{
   edict->svflags |= SVF_USEBBOX;
}

void Entity::HurtEvent(Event *ev)
{
   Vector normal;
   float dmg;

   if(ev->NumArgs() < 1)
   {
      dmg = 50;
   }
   else
   {
      dmg = ev->GetFloat(1);
   }
   normal = Vector(orientation[0]);
   Damage(world, world, dmg, worldorigin, vec_zero, normal, dmg, 0, MOD_CRUSH, -1, -1, 1.0f);
}

void Entity::IfSkillEvent(Event *ev)
{
   float skilllevel;

   skilllevel = ev->GetFloat(1);

   if(skill->value == skilllevel)
   {
      int			argc;
      int			numargs;
      Event       *event;
      int			i;

      numargs = ev->NumArgs();
      argc = numargs - 2 + 1;

      event = new Event(ev->GetToken(2));

      for(i = 1; i < argc; i++)
      {
         event->AddToken(ev->GetToken(2 + i));
      }
      ProcessEvent(event);
   }
}

void Entity::Censor(Event *ev)
{
   Vector delta;
   float oldsize;
   float newsize;

   if(!parentmode->value)
      return;

   oldsize = size.length();
   setSolidType(SOLID_NOT);
   setModel("censored.def");
   gi.CalculateBounds(edict->s.modelindex, 1, mins.vec3(), maxs.vec3());
   delta = maxs - mins;
   newsize = delta.length();
   edict->s.scale = oldsize / newsize;
   mins *= edict->s.scale;
   maxs *= edict->s.scale;
   setSize(mins, maxs);
   setOrigin(origin);
}

// EOF

