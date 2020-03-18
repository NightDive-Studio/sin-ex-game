//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/surface.cpp                      $
// $Revision:: 27                                                             $
//   $Author:: Jimdose                                                        $
//     $Date:: 12/18/98 11:02p                                                $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Controls surfaces
// 

#include "g_local.h"
#include "listener.h"
#include "surface.h"
#include "misc.h"
#include "specialfx.h"
#include <ctype.h>

#include "../elib/qstringmap.h" // haleyjd 20170608

CLASS_DECLARATION(Class, Surface, nullptr);

ResponseDef Surface::Responses[] =
{
   { nullptr, nullptr }
};

CLASS_DECLARATION(Listener, SurfaceModifier, nullptr);

Event EV_Surface_TransOn("transOn");
Event EV_Surface_TransOff("transOff");
Event EV_Surface_TransToggle("transToggle");
Event EV_Surface_TransMag("trans_mag");
Event EV_Surface_Translucence("translucence");
Event EV_Surface_Magnitude("magnitude");
Event EV_Surface_Frequency("frequency");
Event EV_Surface_TransAngle("trans_angle");
Event EV_Surface_DamageFrame("frame");
Event EV_Surface_Damage("damage");

ResponseDef SurfaceModifier::Responses[] =
{
   { &EV_Surface_TransOn,      (Response)&SurfaceModifier::TranslationOn           },
   { &EV_Surface_TransOff,     (Response)&SurfaceModifier::TranslationOff          },
   { &EV_Surface_TransToggle,  (Response)&SurfaceModifier::TranslationToggle       },
   { &EV_Surface_TransMag,     (Response)&SurfaceModifier::SetTranslationMagnitude },
   { &EV_Surface_TransAngle,   (Response)&SurfaceModifier::SetTranslationAngle     },
   { &EV_Surface_Translucence, (Response)&SurfaceModifier::SetTranslucence         },
   { &EV_Surface_Magnitude,    (Response)&SurfaceModifier::SetMagnitude            },
   { &EV_Surface_Frequency,    (Response)&SurfaceModifier::SetFrequency            },
   { &EV_Surface_DamageFrame,  (Response)&SurfaceModifier::SetDamageFrame          },
   { &EV_Surface_Damage,       (Response)&SurfaceModifier::SetDamage               },
   { nullptr, nullptr }
};

SurfaceModifier    surfaceManager;

// haleyjd 20170608: Efficient map for fast lookups
using keyfunc_t = const char *(*)(Surface *);
class SurfaceModifierMap : public qstringmap<Surface *, keyfunc_t> 
{
public:
   using qstringmap::qstringmap;
};

SurfaceModifier::SurfaceModifier() : Listener(), surfaceList()
{
   surfaceMap = new SurfaceModifierMap([] (Surface *p) { return p->SurfaceName(); });
}

SurfaceModifier::~SurfaceModifier()
{
   Reset();
   if(surfaceMap)
   {
      delete surfaceMap;
      surfaceMap = nullptr;
   }
}

//==========
//AddSurface - to the surfaceManager
//==========
int SurfaceModifier::AddSurface(Surface *surf)
{
   int num = surfaceList.AddObject(surf);
   surfaceMap->insert(surf);
   return num;
}

//=============
//SurfaceExists - returns the number of the surface, 0 if not found.
//=============
int SurfaceModifier::SurfaceExists(const char *surf_name)
{
   Surface *p;

   return (p = surfaceMap->find(surf_name)) ? p->SurfaceNumber() : 0;
}

//=============
//GetSurface - returns the surface, NULL if not found.
//=============
Surface *SurfaceModifier::GetSurface(const char *surf_name)
{
   return surfaceMap->find(surf_name);
}

//=====
//Reset
//=====
void SurfaceModifier::Reset()
{
   int num = surfaceList.NumObjects();
   for(int i = num; i > 0; i--)
   {
      delete surfaceList.ObjectAt(i);
   }

   surfaceList.FreeObjectList();
   surfaceMap->clear();
}

//=============
//CreateSurface - Creates a server game surface that can be modified through scripts and events
//=============
void SurfaceModifier::CreateSurface(const char *surf_name, csurface_t *surfinfo)
{
   csurface_t     *surfptr;
   netsurface_t   *netsurf;
   Surface        *surf;
   int            numframes;

   globals.num_surfaces++;
   surf = new Surface();
   surf->SetDamage(0);
   surf->SetState(0);
   surf->SetNumber(globals.num_surfaces);
   surf->SetName(surf_name);

   if((surfinfo->flags & SURF_WEAK) && (surfinfo->flags & SURF_NORMAL))
      surf->SetThreshold(STRONG_DAMAGE_VALUE);
   else if(surfinfo->flags & SURF_WEAK)
      surf->SetThreshold(WEAK_DAMAGE_VALUE);
   else if(surfinfo->flags & SURF_NORMAL)
      surf->SetThreshold(NORMAL_DAMAGE_VALUE);

   surfptr = surfinfo;
   numframes = 0;
   while(surfptr->next)
   {
      numframes++;
      surfptr = surfptr->next;
      if(numframes > 128 || surfptr == surfinfo)
      {
         gi.dprintf("CreateSurface: Possible infinite animation/damage loop for %s.\n", surf_name);
         break;
      }
   }
   surf->SetNumFrames(numframes);
   surf->SetLightStyle(surfinfo->style);

   AddSurface(surf);
   netsurf = &g_surfaces[globals.num_surfaces];

   netsurf->s.name = G_CopyString(surf_name);
   netsurf->inuse = true;
   netsurf->s.number = globals.num_surfaces;
   netsurf->s.groupnumber = surfinfo->groupnumber;
   netsurf->s.trans_mag = surfinfo->trans_mag;
   netsurf->s.trans_angle = surfinfo->trans_angle;
   netsurf->s.translucence = surfinfo->translucence;
   netsurf->s.magnitude = surfinfo->magnitude;
   netsurf->s.frequency = surfinfo->frequency;
   netsurf->s.damage_frame = 0;
   if(surfinfo->flags & SURF_TRANSLATE)
      netsurf->s.trans_state = true;
   else
      netsurf->s.trans_state = false;
   netsurf->s.changed = true;
}

//==============
//SetDamageFrame
//==============
void SurfaceModifier::SetDamageFrame(Event *ev)
{
   const char		*surf_name;
   netsurface_t   *svsurf;
   int            num;
   int            frame;

   surf_name = ev->GetString(1);
   if(!(num = SurfaceExists(surf_name)))
   {
      ev->Error("SurfaceModifier::SetDamage", "surface name %s is not found\n", surf_name);
   }

   svsurf = &g_surfaces[num];
   frame = ev->GetInteger(2);
   if(svsurf->s.damage_frame != frame)
   {
      svsurf->s.changed = true;
   }
   svsurf->s.damage_frame = frame;
}

//============
//SetFrequency
//============
void SurfaceModifier::SetFrequency(Event *ev)
{
   const char		*surf_name;
   netsurface_t   *svsurf;
   int            num;

   surf_name = ev->GetString(1);
   if(!(num = SurfaceExists(surf_name)))
   {
      ev->Error("SurfaceModifier::SetFrequency", "surface name %s is not found\n", surf_name);
   }

   svsurf = &g_surfaces[num];
   svsurf->s.frequency = ev->GetFloat(2);
   svsurf->s.changed = true;
}

//============
//SetMagnitude
//============
void SurfaceModifier::SetMagnitude(Event *ev)
{
   const char		*surf_name;
   netsurface_t   *svsurf;
   int            num;

   surf_name = ev->GetString(1);
   if(!(num = SurfaceExists(surf_name)))
   {
      ev->Error("SurfaceModifier::SetMagnitude", "surface name %s is not found\n", surf_name);
   }

   svsurf = &g_surfaces[num];
   svsurf->s.magnitude = ev->GetFloat(2);
   svsurf->s.changed = true;
}

//===============
//SetTranslucence
//===============
void SurfaceModifier::SetTranslucence(Event *ev)
{
   const char		*surf_name;
   netsurface_t   *svsurf;
   int            num;

   surf_name = ev->GetString(1);
   if(!(num = SurfaceExists(surf_name)))
   {
      ev->Error("SurfaceModifier::SetTranslucence", "surface name %s is not found\n", surf_name);
   }

   svsurf = &g_surfaces[num];
   svsurf->s.translucence = ev->GetFloat(2);
   svsurf->s.changed = true;
}

//===================
//SetTranslationAngle
//===================
void SurfaceModifier::SetTranslationAngle(Event *ev)
{
   const char		*surf_name;
   netsurface_t   *svsurf;
   int            num;

   surf_name = ev->GetString(1);
   if(!(num = SurfaceExists(surf_name)))
   {
      ev->Error("SurfaceModifier::SetDamage", "surface name %s is not found\n", surf_name);
   }

   svsurf = &g_surfaces[num];
   svsurf->s.trans_angle = ev->GetInteger(2);
   svsurf->s.changed = true;
}

//=======================
//SetTranslationMagnitude
//=======================
void SurfaceModifier::SetTranslationMagnitude(Event *ev)
{
   const char		*surf_name;
   netsurface_t   *svsurf;
   int            num;

   surf_name = ev->GetString(1);
   if(!(num = SurfaceExists(surf_name)))
   {
      ev->Error("SurfaceModifier::SetTranslationMagnitude", "surface name %s is not found\n", surf_name);
   }

   svsurf = &g_surfaces[num];
   svsurf->s.trans_mag = ev->GetInteger(2);
   svsurf->s.changed = true;
}

//=================
//TranslationToggle
//=================
void SurfaceModifier::TranslationToggle(Event *ev)
{
   const char		*surf_name;
   netsurface_t   *svsurf;
   int            num;

   surf_name = ev->GetString(1);
   if(!(num = SurfaceExists(surf_name)))
   {
      ev->Error("SurfaceModifier::TranslationToggle", "surface name %s is not found\n", surf_name);
   }

   svsurf = &g_surfaces[num];
   svsurf->s.trans_state = ~svsurf->s.trans_state;
   svsurf->s.changed = true;
}

//=============
//TranslationOn
//=============
void SurfaceModifier::TranslationOn(Event *ev)
{
   const char		*surf_name;
   netsurface_t   *svsurf;
   int            num;

   surf_name = ev->GetString(1);
   if(!(num = SurfaceExists(surf_name)))
   {
      ev->Error("SurfaceModifier::TranslationOn", "surface name %s is not found\n", surf_name);
   }

   svsurf = &g_surfaces[num];
   svsurf->s.trans_state = true;
   svsurf->s.changed = true;
}

//==============
//TranslationOff
//==============
void SurfaceModifier::TranslationOff(Event *ev)
{
   const char		*surf_name;
   netsurface_t   *svsurf;
   int            num;

   surf_name = ev->GetString(1);
   if(!(num = SurfaceExists(surf_name)))
   {
      ev->Error("SurfaceModifier::TranslationOff", "surface name %s is not found\n", surf_name);
   }

   svsurf = &g_surfaces[num];
   svsurf->s.trans_state = false;
   svsurf->s.changed = true;
}

//=========
//SetDamage
//=========
void SurfaceModifier::SetDamage(Event *ev)
{
   const char		*surf_name;
   int            num;
   int            damage;

   surf_name = ev->GetString(1);
   if(!(num = SurfaceExists(surf_name)))
   {
      ev->Error("SurfaceModifier::SetDamage", "surface name %s is not found\n", surf_name);
   }

   damage = ev->GetInteger(2);
   DoDamage(nullptr, surf_name, damage);
}

//=========
//DoDamage
//=========
void SurfaceModifier::DoDamage(trace_t * trace, const char * groupname, float damage, Entity * attacker)
{
   Surface     *surf;
   int			frame;
   int			total_damage;
   int			letter;
   int         state;
   int         currentstate;
   int         numframes;
   int         lightstyle;
   qboolean		dotrigger;
   qboolean    damaged;
   Vector		pos;
   Vector		dir;
   float       surface_damage_threshold;

   surf = GetSurface(groupname);

   if(!surf)
   {
      warning("SurfaceModifier::Damage", "surface name %s is not found\n", groupname);
      return;
   }

   frame = 0;
   if(trace)
   {
      dir = Vector(trace->dir);
      pos = Vector(trace->endpos) - dir;
   }

   // let the ai know
   if(trace)
   {
      MadeBreakingSound(pos, nullptr);
   }

   surface_damage_threshold = surf->Threshold();

   // get the current damage of this surface
   total_damage = surf->Damage();
   // see if it has been damaged past the threshold before
   damaged = (total_damage > surface_damage_threshold);
   // get the current state
   currentstate = surf->State();
   // get the number of frames
   numframes = surf->NumFrames();
   // get the light style
   lightstyle = surf->LightStyle();
   // add the damage and the new damage
   total_damage += damage;
   // store the damage back into the surface
   surf->SetDamage(total_damage);
   // init the state to -1 so that it gets put to 0 only if the threshold is exceeded
   state = -1;
   while(total_damage > surface_damage_threshold)
   {
      if(!damaged && trace)
      {
         damaged = true;
         TesselateModel
         (
            nullptr,
            TESS_DEFAULT_MIN_SIZE,
            TESS_DEFAULT_MAX_SIZE,
            dir,
            damage,
            TESS_DEFAULT_PERCENT,
            10,
            pos
         );
         SpawnSparks
         (
            pos,
            dir * -1.0f,
            20
         );
      }

      total_damage -= surface_damage_threshold;
      state++;

      if(frame < numframes)
         frame++;
   }
   if((lightstyle >= 0) && (state >= currentstate))
   {
      int length;
      int pos;
      int lastvalid;
      qboolean done;

      // get length of name
      length = strlen(groupname);
      lastvalid = 0;
      done = false;
      while((lastvalid < length) && !done)
      {
         switch(tolower(groupname[lastvalid]))
         {
         case 'o':
         case 'f':
         case 's':
         case 'q':
         case 'h':
            lastvalid++;
            break;
         default:
            done = true;
            break;
         }
      }
      dotrigger = false;

      if(currentstate <= lastvalid)
      {
         // copy the current position to a local variable
         pos = state;
         if(pos >= lastvalid)
            pos = lastvalid - 1;
         letter = tolower(groupname[pos]);
         switch(letter)
         {
         case 'o':
            gi.configstring(CS_LIGHTS + lightstyle, "a");
            state++;
            dotrigger = true;
            break;
         case 'f':
            gi.configstring(CS_LIGHTS + lightstyle, "aanannanaanann");
            state++;
            dotrigger = true;
            break;
         case 's':
            gi.configstring(CS_LIGHTS + lightstyle, "aaaanaaannaanaaaaan");
            state++;
            dotrigger = true;
            break;
         case 'q':
            gi.configstring(CS_LIGHTS + lightstyle, "d");
            state++;
            dotrigger = true;
            break;
         case 'h':
            gi.configstring(CS_LIGHTS + lightstyle, "g");
            state++;
            dotrigger = true;
            break;
         default:
            break;
         }
      }
      surf->SetState(state);
      if(dotrigger)
      {
         const char * t;

         t = strchr(groupname, '_');
         if(t)
         {
            Event * event;
            Entity * ent;
            int n = 0;
            t++;
            // 
            // see if this target exists
            //
            n = G_FindTarget(n, t);
            if(n)
            {
               do
               {
                  ent = G_GetEntity(n);

                  event = new Event(EV_Activate);
                  event->AddEntity(attacker);
                  ent->ProcessEvent(event);

                  n = G_FindTarget(n, t);
                  if(!n)
                     break;
               }
               while(1);
            }
            // 
            // otherwise treat it like a thread
            //
            else
            {
               if(!ExecuteThread(str(t)))
               {
                  warning("DoDamage", "%s not found as a script.", t);
               }
            }
         }
      }
   }
   if(frame)
   {
      netsurface_t   *svsurf;
      int            num;

      num = surf->SurfaceNumber();
      if(num)
      {
         svsurf = &g_surfaces[num];
         if(svsurf->s.damage_frame != frame)
         {
            svsurf->s.damage_frame = frame;
            svsurf->s.changed = true;
         }
      }
   }
}

//=========
//DamageSurface
//=========
void SurfaceModifier::DamageSurface(trace_t * trace, float damage, Entity * attacker)
{
   csurface_t  *csurf;

   if(!trace)
      return;

   csurf = trace->surface;
   // Make sure there is a surface here.
   if(
      !csurf ||
      !(csurf->flags & (SURF_WEAK | SURF_NORMAL))
      )
   {
      return;
   }
   DoDamage(trace, csurf->groupname, damage, attacker);
}

//==============
//CreateSurfaces
//==============
void CreateSurfaces(csurface_t *surfinfo, int count)
{
   int i;

   for(i = 0; i < count; i++)
   {
      if(!isdigit(surfinfo[i].groupname[0]) || (surfinfo[i].flags & SURF_DAMAGE))
      {
         if(!surfaceManager.SurfaceExists(surfinfo[i].groupname))
         {
            surfaceManager.CreateSurface(surfinfo[i].groupname, &surfinfo[i]);
         }
      }
   }
}

// EOF

