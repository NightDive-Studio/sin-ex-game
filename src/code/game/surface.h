//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/surface.h                        $
// $Revision:: 19                                                             $
//   $Author:: Jimdose                                                        $
//     $Date:: 10/25/98 11:53p                                                $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Controls surfaces
// 

#ifndef __SURFACE_H__
#define __SURFACE_H__

#include "g_local.h"
#include "listener.h"

extern Event EV_Surface_TransOn;
extern Event EV_Surface_TransOff;
extern Event EV_Surface_TransToggle;
extern Event EV_Surface_TransMag;
extern Event EV_Surface_Translucence;
extern Event EV_Surface_Magnitude;
extern Event EV_Surface_Frequency;
extern Event EV_Surface_TransAngle;
extern Event EV_Surface_DamageFrame;

class EXPORT_FROM_DLL Surface : public Class
{
private:
   str   surface_name;
   int   surface_number;
   int   damage;
   int   state;
   int   threshold;
   int   numframes;
   int   style;

public:
   CLASS_PROTOTYPE(Surface);

   const char  *SurfaceName()   const { return surface_name.c_str(); }
   int          SurfaceNumber() const { return surface_number;       }
   int          Damage()        const { return damage;               }
   int          State()         const { return state;                }
   int          Threshold()     const { return threshold;            }
   int          NumFrames()     const { return numframes;            }
   int          LightStyle()    const { return style;                }
               
   void         SetThreshold(int num)          { threshold      = num;       }
   void         SetNumFrames(int num)          { numframes      = num;       }
   void         SetLightStyle(int num)         { style          = num;       }
   void         SetDamage(int num)             { damage         = num;       }
   void         SetNumber(int num)             { surface_number = num;       }
   void         SetState(int num)              { state          = num;       }
   void         SetName(const char *surf_name) { surface_name   = surf_name; }
   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void Surface::Archive(Archiver &arc)
{
   arc.WriteInteger(damage);
   arc.WriteInteger(state);
}

inline EXPORT_FROM_DLL void Surface::Unarchive(Archiver &arc)
{
   arc.ReadInteger(&damage);
   arc.ReadInteger(&state);
}

#ifdef EXPORT_TEMPLATE
template class EXPORT_FROM_DLL Container<Surface *>;
#endif

class SurfaceModifierMap; // haleyjd 20170608

class EXPORT_FROM_DLL SurfaceModifier : public Listener
{
private:
   Container<Surface *>  surfaceList;
   SurfaceModifierMap   *surfaceMap;
   void     DoDamage(trace_t * trace, const char * group_name, float damage, Entity * attacker = world);

public:
   CLASS_PROTOTYPE(SurfaceModifier);

   SurfaceModifier();
   ~SurfaceModifier();
   void     CreateSurface(const char *surf_name, csurface_t *surfinfo);
   int      AddSurface(Surface *surf);
   void     RemoveSurface(const char *surf_name);
   int      SurfaceExists(const char *surf_name);
   Surface *GetSurface(const char *surf_name);
   void     DamageSurface(trace_t * trace, float damage, Entity * attacker);
   void     Reset();

   void     TranslationOn(Event *ev);
   void     TranslationOff(Event *ev);
   void     TranslationToggle(Event *ev);
   void     SetTranslationMagnitude(Event *ev);
   void     SetTranslationAngle(Event *ev);
   void     SetTranslucence(Event *ev);
   void     SetMagnitude(Event *ev);
   void     SetFrequency(Event *ev);
   void     SetDamageFrame(Event *ev);
   void     SetDamage(Event *ev);
   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void SurfaceModifier::Archive(Archiver &arc)
{
   int i;
   Surface * surf;
   netsurface_t *s;
   int num;

   Listener::Archive(arc);

   num = surfaceList.NumObjects();
   arc.WriteInteger(num);
   for(i = 1; i <= num; i++)
   {
      //
      // we only save out the dynamic fields
      //
      surf = surfaceList.ObjectAt(i);
      arc.WriteObject(surf);
   }

   // write the surface states
   s = g_surfaces;
   for(i = 0; i < game.maxsurfaces; i++, s++)
   {
      arc.WriteBoolean(s->inuse);
      if(s->inuse)
      {
         arc.WriteRaw(&s->s, sizeof(s->s));
      }
   }
}

inline EXPORT_FROM_DLL void SurfaceModifier::Unarchive(Archiver &arc)
{
   int i;
   Surface *surf;
   netsurface_t *s;
   int num;
   char *name;

   Listener::Unarchive(arc);

   // get the number
   arc.ReadInteger(&num);
   for(i = 1; i <= num; i++)
   {
      //
      // we only read in the dynamic fields
      //
      surf = surfaceList.ObjectAt(i);
      arc.ReadObject(surf);
   }

   // read the surface states
   s = g_surfaces;
   for(i = 0; i < game.maxsurfaces; i++, s++)
   {
      arc.ReadBoolean(&s->inuse);
      if(s->inuse)
      {
         name = s->s.name;
         arc.ReadRaw(&s->s, sizeof(s->s));
         s->s.name = name;
      }
   }
}

extern SurfaceModifier surfaceManager;

extern "C" void CreateSurfaces(csurface_t *surfaces, int count);

#endif /* surface.h */

// EOF

