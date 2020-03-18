/*
================================================================
VIEW JITTER STUFF
================================================================

Copyright (C) 2020 by Night Dive Studios, Inc.
All rights reserved.

See the license.txt file for conditions and terms of use for this code.
*/

#include "jitter.h"
#include "player.h"

float anglejitter_time;       // timmer for angle jitter
float anglejitter_magnitude;  // angle jitter magnitude
float anglejitter_falloff;    // falloffrate for angle jitter
float offsetjitter_time;      // timmer for offset jitter
float offsetjitter_magnitude; // offset jitter magnitude
float offsetjitter_falloff;   // falloffrate for offset jitter

//=============================================================================

Event EV_Jitter_DeactivateAngle("jitter_deactivate_angle");
Event EV_Jitter_DeactivateOffset("jitter_deactivate_offset");
Event EV_Jitter_ApplyJitter("jitter_apply");

#define TOGGLE         1
#define START_ON       2
#define CODE_GENERATED 4

CLASS_DECLARATION(Trigger, BaseJitter, "")

ResponseDef BaseJitter::Responses[] =
{
   { nullptr, nullptr }
};

/*****************************************************************************/
/*SINED func_jitter_global (.5 .5 .8) (-8 -8 -8) (8 8 8) TOGGLE START_ON
Causes an angle and offset jitter of players' views.

"duration" is the duration of the jitter. Default is 0.8 seconds.
"angleduration"  is angle jitter duration. Set to -1 for no angle jitter.
"anglemagnitude" is angle jitter magnitude. Default is 3.
"anglefalloff" is angle jitter falloff rate. Default is 4.
"offsetduration"  is offset jitter duration. Set to -1 for no offset jitter.
"offsetmagnitude" is offset jitter magnitude. Default is 2.
"offsetfalloff" is offset jitter falloff rate. Default is 3.

TOGGLE makes it toggle on and off each time it's triggered
START_ON makes it start immediatly. Works with or without toggle set.
/*****************************************************************************/

CLASS_DECLARATION(BaseJitter, GlobalJitter, "func_jitter_global")

ResponseDef GlobalJitter::Responses[] =
{
   { &EV_Touch,                   nullptr                                   },
   { &EV_Trigger_Effect,          (Response)&GlobalJitter::Activate         },
   { &EV_Jitter_DeactivateAngle,  (Response)&GlobalJitter::DeactivateAngle  },
   { &EV_Jitter_DeactivateOffset, (Response)&GlobalJitter::DeactivateOffset },
   { nullptr, nullptr }
};

GlobalJitter::GlobalJitter() : BaseJitter()
{
   if(spawnflags & TOGGLE)
   {
      angleduration = G_GetFloatArg("angleduration", 0);
      if(angleduration < 0)
         angleduration = 0;
      else
         angleduration = -1;

      offsetduration = G_GetFloatArg("offsetduration", 0);
      if(offsetduration < 0)
         offsetduration = 0;
      else
         offsetduration = -1;
   }
   else
   {
      float main_duration;

      main_duration = G_GetFloatArg("duration", 0.8);

      angleduration = G_GetFloatArg("angleduration", 0);
      if(angleduration < 0)
         angleduration = 0;
      else if(!angleduration)
         angleduration = main_duration;

      offsetduration = G_GetFloatArg("offsetduration", 0);
      if(offsetduration < 0)
         offsetduration = 0;
      else if(!offsetduration)
         offsetduration = main_duration;
   }

   anglemagnitude = G_GetFloatArg("anglemagnitude", 3);
   anglefalloff   = G_GetFloatArg("anglefalloff", 4);

   offsetmagnitude = G_GetFloatArg("offsetmagnitude", 2);
   offsetfalloff   = G_GetFloatArg("offsetfalloff", 3);

   anglejitter_time       = 0;
   anglejitter_magnitude  = 0;
   anglejitter_falloff    = 0;
   offsetjitter_time      = 0;
   offsetjitter_magnitude = 0;
   offsetjitter_falloff   = 0;

   if(spawnflags & START_ON)
   {
      // turn on after all entities are spawned
      PostEvent(EV_Trigger_Effect, 0.2);
   }
}

EXPORT_FROM_DLL void GlobalJitter::Activate(Event *ev)
{
   if(spawnflags & TOGGLE)
   {
      if(angleduration)
      {
         if(angleactive)
         {
            ProcessEvent(EV_Jitter_DeactivateAngle);
         }
         else
         {
            // override current angle jitter if of greater magnitude
            if(anglemagnitude > anglejitter_magnitude)
            {
               anglejitter_time = 9999999;
               anglejitter_magnitude = anglemagnitude;
               anglejitter_falloff = anglefalloff;

               angleactive = true;
            }
         }
      }

      if(offsetduration)
      {
         if(offsetactive)
         {
            ProcessEvent(EV_Jitter_DeactivateOffset);
         }
         else
         {
            // override current angle jitter if of greater magnitude
            if(offsetmagnitude > offsetjitter_magnitude)
            {
               offsetjitter_time      = 9999999;
               offsetjitter_magnitude = offsetmagnitude;
               offsetjitter_falloff   = offsetfalloff;

               offsetactive = true;
            }
         }
      }
   }
   else
   {
      // setup angle jitter first
      if(angleduration)
      {
         // override current angle jitter if of greater magnitude
         if(anglemagnitude > anglejitter_magnitude)
         {
            anglejitter_time      = level.time + angleduration;
            anglejitter_magnitude = anglemagnitude;
            anglejitter_falloff   = anglefalloff;

            angleactive = true;
            CancelEventsOfType(EV_Jitter_DeactivateAngle);
            PostEvent(EV_Jitter_DeactivateAngle, angleduration);
         }
      }

      // setup offset jitter now
      if(offsetduration)
      {
         // override current offset jitter if of greater magnitude
         if(offsetmagnitude > offsetjitter_magnitude)
         {
            offsetjitter_time = level.time + offsetduration;
            offsetjitter_magnitude = offsetmagnitude;
            offsetjitter_falloff = offsetfalloff;

            offsetactive = true;
            CancelEventsOfType(EV_Jitter_DeactivateOffset);
            PostEvent(EV_Jitter_DeactivateOffset, offsetduration);
         }
      }
   }
}

EXPORT_FROM_DLL void GlobalJitter::DeactivateAngle(Event *ev)
{
   float new_time, new_magnitude, new_falloff;
   int num;
   GlobalJitter *ent;

   angleactive = false;
   new_time = 0;
   new_magnitude = 0;
   new_falloff = 0;

   num = 0;
   // go through all the GlobalJitters to see if there's any active ones
   while((num = G_FindClass(num, "func_jitter_global")))
   {
      ent = (GlobalJitter *)G_GetEntity(num);

      if(!ent->angleactive)
         continue;

      if(ent->anglemagnitude < new_magnitude)
         continue;

      new_time      = level.time + ent->angleduration;
      new_magnitude = ent->anglemagnitude;
      new_falloff   = ent->anglefalloff;
   }

   // set the new angle jitter values
   anglejitter_time      = new_time;
   anglejitter_magnitude = new_magnitude;
   anglejitter_falloff   = new_falloff;
}

EXPORT_FROM_DLL void GlobalJitter::DeactivateOffset(Event *ev)
{
   float new_time, new_magnitude, new_falloff;
   int num;
   GlobalJitter *ent;

   offsetactive = false;
   new_time = 0;
   new_magnitude = 0;
   new_falloff = 0;

   num = 0;
   // go through all the GlobalJitters to see if there's any active ones
   while((num = G_FindClass(num, "func_jitter_global")))
   {
      ent = static_cast<GlobalJitter *>(G_GetEntity(num));

      if(!ent->offsetactive)
         continue;

      if(ent->offsetmagnitude < new_magnitude)
         continue;

      new_time      = level.time + ent->offsetduration;
      new_magnitude = ent->offsetmagnitude;
      new_falloff   = ent->offsetfalloff;
   }

   // set the new angle jitter values
   offsetjitter_time      = new_time;
   offsetjitter_magnitude = new_magnitude;
   offsetjitter_falloff   = new_falloff;
}

/*****************************************************************************/
/*SINED func_jitter_radius (.5 .5 .8) (-8 -8 -8) (8 8 8) TOGGLE START_ON
Causes an angle and offset jitter of players' views that are within a certain radius of the entity.

"radius" is the max effective radius of the view jitter. Default is 128.
"outerjitter" is the percentage of the full jitter that will be used at the max effective range. Default is 0.25
"duration" is the duration of the jitter. Default is 0.25 seconds.
"angleduration"  is angle jitter duration. Set to -1 for no angle jitter.
"anglemagnitude" is angle jitter magnitude. Default is 6.
"anglefalloff" is angle jitter falloff rate. Default is 9.
"offsetduration"  is offset jitter duration. Set to -1 for no offset jitter.
"offsetmagnitude" is offset jitter magnitude. Default is 5.
"offsetfalloff" is offset jitter falloff rate. Default is 8.

TOGGLE makes it toggle on and off each time it's triggered
START_ON makes it start immediatly. Works with or without toggle set.
/*****************************************************************************/

CLASS_DECLARATION(BaseJitter, RadiusJitter, "func_jitter_radius")

ResponseDef RadiusJitter::Responses[] =
{
   { &EV_Touch,                   nullptr                                   },
   { &EV_Trigger_Effect,          (Response)&RadiusJitter::Activate         },
   { &EV_Jitter_ApplyJitter,      (Response)&RadiusJitter::ApplyJitter      },
   { &EV_Jitter_DeactivateAngle,  (Response)&RadiusJitter::DeactivateAngle  },
   { &EV_Jitter_DeactivateOffset, (Response)&RadiusJitter::DeactivateOffset },
   { nullptr, nullptr }
};

RadiusJitter::RadiusJitter() : BaseJitter()
{
   if(spawnflags & TOGGLE)
   {
      angleduration = G_GetFloatArg("angleduration", 0);
      if(angleduration < 0)
         angleduration = 0;
      else
         angleduration = -1;

      offsetduration = G_GetFloatArg("offsetduration", 0);
      if(offsetduration < 0)
         offsetduration = 0;
      else
         offsetduration = -1;
   }
   else
   {
      float main_duration;

      main_duration = G_GetFloatArg("duration", 0.25);

      angleduration = G_GetFloatArg("angleduration", 0);
      if(angleduration < 0)
         angleduration = 0;
      else if(!angleduration)
         angleduration = main_duration;

      offsetduration = G_GetFloatArg("offsetduration", 0);
      if(offsetduration < 0)
         offsetduration = 0;
      else if(!offsetduration)
         offsetduration = main_duration;
   }

   anglemagnitude = G_GetFloatArg("anglemagnitude", 6);
   anglefalloff   = G_GetFloatArg("anglefalloff", 9);

   offsetmagnitude = G_GetFloatArg("offsetmagnitude", 5);
   offsetfalloff   = G_GetFloatArg("offsetfalloff", 8);

   jitterradius  = G_GetFloatArg("radius", 128);
   radiusfalloff = G_GetFloatArg("outerjitter", 0.25);
   radiusfalloff = 1 - radiusfalloff;
   if(radiusfalloff > 1)
      radiusfalloff = 1;
   else if(radiusfalloff < 0.01)
      radiusfalloff = 0.01;

   angletime    = 0;
   anglecurrent = 0;
   offsettime   = 0;
   anglecurrent = 0;

   // make sure sone dumbass mapper didn't set this
   spawnflags &= ~CODE_GENERATED;

   if(spawnflags & START_ON)
   {
      // turn on after all entities are spawned
      PostEvent(EV_Trigger_Effect, 0.2);
   }
}

void RadiusJitter::Activate(Event *ev)
{
   if(spawnflags & CODE_GENERATED)
      return;

   if(spawnflags & TOGGLE)
   {
      if(angleduration)
      {
         if(angleactive)
         {
            ProcessEvent(EV_Jitter_DeactivateAngle);
         }
         else
         {
            angleactive  = true;
            angletime    = level.time + angleduration;
            anglecurrent = anglemagnitude;
         }
      }

      if(offsetduration)
      {
         if(offsetactive)
         {
            ProcessEvent(EV_Jitter_DeactivateOffset);
         }
         else
         {
            offsetactive  = true;
            offsettime    = level.time + offsetduration;
            offsetcurrent = offsetmagnitude;
         }
      }
   }
   else
   {
      if(angleduration)
      {
         angleactive  = true;
         angletime    = level.time + angleduration;
         anglecurrent = anglemagnitude;
      }

      if(offsetduration)
      {
         offsetactive  = true;
         offsettime    = level.time + offsetduration;
         offsetcurrent = offsetmagnitude;
      }
   }

   if(angleactive || offsetactive)
   {
      CancelEventsOfType(EV_Jitter_ApplyJitter);
      PostEvent(EV_Jitter_ApplyJitter, 0.1);
   }
}

void RadiusJitter::DeactivateAngle(Event *ev)
{
   angleactive  = false;
   angletime    = 0;
   anglecurrent = 0;

   if(!offsetactive)
   {
      if(spawnflags & CODE_GENERATED)
      {
         PostEvent(EV_Remove, 0);
         return;
      }

      CancelEventsOfType(EV_Jitter_ApplyJitter);
   }
}

void RadiusJitter::DeactivateOffset(Event *ev)
{
   offsetactive  = false;
   offsettime    = 0;
   offsetcurrent = 0;

   if(!angleactive)
   {
      if(spawnflags & CODE_GENERATED)
      {
         PostEvent(EV_Remove, 0);
         return;
      }

      CancelEventsOfType(EV_Jitter_ApplyJitter);
   }
}

void RadiusJitter::ApplyJitter(Event *ev)
{
   Entity *ent;
   Player *player;
   Vector tmpvec;
   float dist, amount, falloffamount;
   float applypercent;

   // if not toggle, check for ending
   if(!(spawnflags & TOGGLE))
   {
      // check for angles ending
      if(angleactive && angletime < level.time)
      {
         anglecurrent -= anglefalloff*FRAMETIME;
         if(anglecurrent <= 0)
         {
            angleactive = false;
            angletime = 0;
            anglecurrent = 0;
         }
      }

      // check for offset ending
      if(offsetactive && offsettime < level.time)
      {
         offsetcurrent -= offsetfalloff*FRAMETIME;
         if(offsetcurrent <= 0)
         {
            offsetactive = false;
            offsettime = 0;
            offsetcurrent = 0;
         }
      }
   }

   // make sure that we're still on
   if(!angleactive && !offsetactive)
   {
      if(spawnflags & CODE_GENERATED)
      {
         PostEvent(EV_Remove, 0);
         return;
      }

      CancelEventsOfType(EV_Jitter_ApplyJitter);
      return;
   }

   for(int i = 1; i <= maxclients->value; i++)
   {
      if(!g_edicts[i].inuse || !g_edicts[i].entity)
      {
         continue;
      }

      ent = g_edicts[i].entity;

      tmpvec = ent->origin - origin;
      dist = tmpvec.length();

      if(dist > jitterradius)
         continue;

      if(!ent->isSubclassOf<Player>())
         return;

      player = static_cast<Player *>(ent);

      // calc percentage of jitter to apply
      if(dist <= 1)
         applypercent = 1;
      else
         applypercent = 1 - (dist/jitterradius)*radiusfalloff;

      if(angleactive)
      {
         amount = anglecurrent*applypercent;
         falloffamount = anglefalloff*applypercent;

         player->SetAngleJitter(amount, falloffamount, angletime);
      }

      if(offsetactive)
      {
         amount = offsetcurrent*applypercent;
         falloffamount = offsetfalloff*applypercent;

         player->SetOffsetJitter(amount, falloffamount, angletime);
      }
   }

   PostEvent(EV_Jitter_ApplyJitter, 0.1);
}

void RadiusJitter::Setup(const Vector &org, float rad, float outer, float angdur, float angmag, float angfall, float ofsdur, float ofsmag, float ofsfall)
{
   setOrigin(org);

   spawnflags = CODE_GENERATED;

   angleactive     = false;
   angleduration   = angdur;
   anglemagnitude  = angmag;
   anglefalloff    = angfall;
   offsetactive    = false;
   offsetduration  = ofsdur;
   offsetmagnitude = ofsmag;
   offsetfalloff   = ofsfall;

   jitterradius = rad;

   radiusfalloff = 1 - outer;

   if(angleduration)
   {
      angleactive  = true;
      angletime    = level.time + angleduration;
      anglecurrent = anglemagnitude;
   }

   if(offsetduration)
   {
      offsetactive  = true;
      offsettime    = level.time + offsetduration;
      offsetcurrent = offsetmagnitude;
   }

   if(angleactive || offsetactive)
      PostEvent(EV_Jitter_ApplyJitter, 0);
   else
      PostEvent(EV_Remove, 0);
}

void RadiusJitter::SetupSmall(const Vector &org)
{
   Setup(org, 150, 0.5, 0.1, 3, 3, 0.1, 2, 3);
}

void RadiusJitter::SetupLarge(const Vector &org)
{
   Setup(org, 200, 0.2, 0.1, 5, 7, 0.1, 4, 5);
}

// EOF

