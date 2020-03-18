//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/q_shared.h                       $
// $Revision:: 195                                                            $
//   $Author:: Jimdose                                                        $
//     $Date:: 8/03/99 7:09p                                                  $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// included first by ALL program modules
//

#ifndef __Q_SHARED_H__
#define __Q_SHARED_H__

#ifdef _MSC_VER
// unknown pragmas are SUPPOSED to be ignored, but....
#pragma warning(disable : 4244)     // MIPS
#pragma warning(disable : 4136)     // X86
#pragma warning(disable : 4051)     // ALPHA

// SINEX_FIXME: disabling these warnings may be hiding problems
#pragma warning(disable : 4018)     // signed/unsigned mismatch
#pragma warning(disable : 4305)     // truncation from const double to float

#endif

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#ifndef min
#define min(a,b)  (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a,b)  (((a) > (b)) ? (a) : (b))
#endif

#define bound(a,minval,maxval)  ( ((a) > (minval)) ? ( ((a) < (maxval)) ? (a) : (maxval) ) : (minval) )

#if (defined _M_IX86 || defined __i386__) && !defined(C_ONLY) && !defined(__sun__)
#define id386	1
#else
#define id386	0
#endif

#if defined _M_ALPHA && !defined C_ONLY
#define idaxp	1
#else
#define idaxp	0
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char      byte;

#ifdef __cplusplus
typedef int qboolean;
#else
typedef enum { false, true } qboolean;
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif

// angle indexes
#define  PITCH 0  // up / down
#define  YAW   1  // left / right
#define  ROLL  2  // fall over

#define  MAX_STRING_CHARS  1024  // max length of a string passed to Cmd_TokenizeString
#define  MAX_STRING_TOKENS 80    // max tokens resulting from Cmd_TokenizeString
#define  MAX_TOKEN_CHARS   128   // max length of an individual token

// SINEX_FIXME: outdated limits on filename length
#define  MAX_QPATH         64    // max length of a quake game pathname
#define  MAX_OSPATH        128   // max length of a filesystem pathname

//
// per-level limits
//
#define  MAX_CLIENTS       256   // absolute limit
#define  MAX_EDICTS        1024  // must change protocol to increase more
#define  MAX_LIGHTSTYLES   256
#define  MAX_MODELS        640   // these are sent over the net as bytes (SiN: was 256 in Q2)
#define  MAX_SOUNDS        512   // so they cannot be blindly increased (SiN: was 256 in Q2)
#define  MAX_CONSOLES      32
#define  MAX_SURFACES      1024
#define  MAX_IMAGES        256
#define  MAX_ITEMS         256

// game print flags
#define  PRINT_LOW         0     // pickup messages
#define  PRINT_MEDIUM      1     // death messages
#define  PRINT_HIGH        2     // critical messages
#define  PRINT_CHAT        3     // chat messages
#define  PRINT_TALK        4     // speech synthesis messages

#define  ERR_FATAL         0     // exit the entire game with a popup window
#define  ERR_DROP          1     // print to console and disconnect from game
#define  ERR_DISCONNECT    2     // don't kill server

#define  PRINT_ALL         0
#define  PRINT_DEVELOPER   1     // only print when "developer 1"
#define  PRINT_ALERT       2

// destination class for gi.multicast()
typedef enum
{
   MULTICAST_ALL,
   MULTICAST_PHS,
   MULTICAST_PVS,
   MULTICAST_ALL_R,
   MULTICAST_PHS_R,
   MULTICAST_PVS_R
} multicast_t;

/*
==============================================================

MATHLIB

==============================================================
*/

typedef float vec_t;
typedef vec_t vec3_t[3];
typedef vec_t vec5_t[5];

typedef int fixed4_t;
typedef int fixed8_t;
typedef int fixed16_t;

#ifndef M_PI
#define M_PI 3.14159265358979323846 // matches value in gcc v2 math.h
#endif

struct cplane_s;

extern vec3_t vec3_origin;

#define  nanmask (255<<23)

#define  IS_NAN(x) (((*(int *)&x) & nanmask) == nanmask)

// SINEX_FIXME: not likely to be the case any more, especially w/SSE2 enabled
// microsoft's fabs seems to be ungodly slow...
//float Q_fabs (float f);
//#define	fabs(f) Q_fabs(f)
#if !defined(C_ONLY) && !defined(__linux__) && !defined(__sgi)
extern long Q_ftol( float f );
#else
#define Q_ftol( f ) (long)(f)
#endif

#define DotProduct(x,y)       (x[0]*y[0]+x[1]*y[1]+x[2]*y[2])
#define VectorSubtract(a,b,c) (c[0]=a[0]-b[0],c[1]=a[1]-b[1],c[2]=a[2]-b[2])
#define VectorAdd(a,b,c)      (c[0]=a[0]+b[0],c[1]=a[1]+b[1],c[2]=a[2]+b[2])
#define VectorCopy(a,b)       (b[0]=a[0],b[1]=a[1],b[2]=a[2])
#define VectorClear(a)        (a[0]=a[1]=a[2]=0)
#define VectorNegate(a,b)     (b[0]=-a[0],b[1]=-a[1],b[2]=-a[2])
#define VectorSet(v, x, y, z) (v[0]=(x), v[1]=(y), v[2]=(z))

void VectorMA(vec3_t veca, float scale, vec3_t vecb, vec3_t vecc);

// just in case you don't want to use the macros
vec_t _DotProduct(vec3_t v1, vec3_t v2);
void  _VectorSubtract(vec3_t veca, vec3_t vecb, vec3_t out);
void  _VectorAdd(vec3_t veca, vec3_t vecb, vec3_t out);
void  _VectorCopy(vec3_t in, vec3_t out);

void  ClearBounds(vec3_t mins, vec3_t maxs);
void  AddPointToBounds (vec3_t v, vec3_t mins, vec3_t maxs);
int   VectorCompare(vec3_t v1, vec3_t v2);
vec_t VectorLength(vec3_t v);
void  CrossProduct(vec3_t v1, vec3_t v2, vec3_t cross);
vec_t VectorNormalize(vec3_t v); // returns vector length
vec_t VectorNormalize2(vec3_t v, vec3_t out);
void  VectorInverse(vec3_t v);
void  VectorScale(vec3_t in, vec_t scale, vec3_t out);
int   Q_log2(int val);

void R_ConcatRotations(float in1[3][3], float in2[3][3], float out[3][3]);
void R_ConcatTransforms(float in1[3][4], float in2[3][4], float out[3][4]);

void  AngleVectors(vec3_t angles, vec3_t forward, vec3_t right, vec3_t up);
int   BoxOnPlaneSide(vec3_t emins, vec3_t emaxs, struct cplane_s *plane);
float anglemod(float a);
float angledist( float ang );
float LerpAngle(float a1, float a2, float frac);

#define BOX_ON_PLANE_SIDE(emins, emaxs, p)   \
   (((p)->type < 3) ?                        \
   (                                         \
      ((p)->dist <= (emins)[(p)->type]) ?    \
         1                                   \
      :                                      \
      (                                      \
         ((p)->dist >= (emaxs)[(p)->type]) ? \
            2                                \
         :                                   \
            3                                \
      )                                      \
   )                                         \
   :                                         \
      BoxOnPlaneSide( (emins), (emaxs), (p)))

void ProjectPointOnPlane(vec3_t dst, const vec3_t p, const vec3_t normal);
void PerpendicularVector(vec3_t dst, const vec3_t src);
void RotatePointAroundVector(vec3_t dst, const vec3_t dir, const vec3_t point, float degrees);

//=============================================

const char *COM_SkipPath(const char *pathname);
void        COM_StripExtension(const char *in, char *out);
char       *COM_FileExtension(const char *in);
void        COM_FileBase(const char *in, char *out);
void        COM_FilePath(const char *in, char *out);
void        COM_DefaultExtension(char *path, const char *extension);
int         COM_ParseHex(const char *hex);
const char *COM_GetToken(const char **data_p, qboolean crossline);
const char *COM_Parse(const char **data_p);
const char *SIN_GetToken(const char **data_p, qboolean crossline);
const char *SIN_Parse(const char **data_p);

// data is an in/out parm, returns a parsed out token

void Com_sprintf(char *dest, int size, const char *fmt, ...);

void Com_PageInMemory(byte *buffer, int size);

//=============================================

// portable case insensitive compare
int Q_stricmp(const char *s1, const char *s2);
int Q_strcasecmp(const char *s1, const char *s2);
int Q_strncasecmp(const char *s1, const char *s2, int n);

// haleyjd 201706010: BSD string utils for secure strncpy and strncat
size_t Q_strlcpy(char *dst, const char *src, size_t siz);
size_t Q_strlcat(char *dst, const char *src, size_t siz);

//=============================================

short BigShort(short l);
short LittleShort(short l);
int   BigLong(int l);
int   LittleLong(int l);
float BigFloat(float l);
float LittleFloat(float l);

unsigned short BigUnsignedShort(unsigned short l);
unsigned short LittleUnsignedShort(unsigned short l);

void Swap_Init(void);

const char *va(const char *format, ...);

//=============================================

//
// key / value info strings
//
#define  MAX_INFO_KEY      128  // SiN: was 64
#define  MAX_INFO_VALUE    128  // SiN: was 64
#define  MAX_INFO_STRING   512

const char *Info_ValueForKey(const char *s, const char *key);
void        Info_RemoveKey(char *s, const char *key);
void        Info_SetValueForKey(char *s, const char *key, const char *value);

/*
==============================================================

SYSTEM SPECIFIC

==============================================================
*/

extern int curtime; // time returned by last Sys_Milliseconds

int  Sys_Milliseconds(void);
void Sys_Mkdir(const char *path);

// large block stack allocation routines
void *Hunk_Begin(int maxsize);
void *Hunk_Alloc(int size);
void  Hunk_Free(void *buf);
int   Hunk_End(void);

// directory searching
#define SFF_ARCH    0x01
#define SFF_HIDDEN  0x02
#define SFF_RDONLY  0x04
#define SFF_SUBDIR  0x08
#define SFF_SYSTEM  0x10

//
// pass in an attribute mask of things you wish to REJECT
//
const char *Sys_FindFirst(const char *path, unsigned musthave, unsigned canthave);
const char *Sys_FindNext(unsigned musthave, unsigned canthave);
void        Sys_FindClose(void);

// this is only here so the functions in q_shared.c and q_shwin.c can link

void Sys_Error(const char *error, ...);
void Com_Printf(const char *msg, ...);

/*
==========================================================

CVARS (console variables)

==========================================================
*/

#ifndef CVAR
#define CVAR

#define CVAR_ARCHIVE    1  // set to cause it to be saved to vars.rc
#define CVAR_USERINFO   2  // added to userinfo  when changed
#define CVAR_SERVERINFO 4  // added to serverinfo when changed
#define CVAR_NOSET      8  // don't allow change from console at all,
                           // but can be set from the command line
#define CVAR_LATCH      16 // save changes until server restart

// nothing outside the Cvar_*() functions should modify these fields!
typedef struct cvar_s
{

   char     *name;
   char     *string;
   char     *latched_string;  // for CVAR_LATCH vars
   int       flags;
   qboolean  modified;        // set each time the cvar is changed
   float     value;
   struct cvar_s *next;
} cvar_t;

#endif // ifndef CVAR

/*
==============================================================

COLLISION DETECTION

==============================================================
*/

// lower bits are stronger, and will eat weaker brushes completely
#define  CONTENTS_SOLID        1  // an eye is never valid in a solid
#define  CONTENTS_WINDOW       2  // translucent, but not watery
#define  CONTENTS_FENCE        4  // SiN: formerly CONTENTS_AUX
#define  CONTENTS_LAVA         8
#define  CONTENTS_LIGHTVOLUME  16 // SiN: formerly CONTENTS_SLIME
#define  CONTENTS_WATER        32
#define  CONTENTS_MIST         64
#define  LAST_VISIBLE_CONTENTS 64

// remaining contents are non-visible, and don't eat brushes
#define  CONTENTS_DUMMYFENCE   0x1000

#define  CONTENTS_AREAPORTAL   0x8000

#define  CONTENTS_PLAYERCLIP   0x10000
#define  CONTENTS_MONSTERCLIP  0x20000

// currents can be added to any other contents, and may be mixed
#define  CONTENTS_CURRENT_0    0x40000
#define  CONTENTS_CURRENT_90   0x80000
#define  CONTENTS_CURRENT_180  0x100000
#define  CONTENTS_CURRENT_270  0x200000
#define  CONTENTS_CURRENT_UP   0x400000
#define  CONTENTS_CURRENT_DOWN 0x800000

#define  CONTENTS_ORIGIN       0x1000000  // removed before bsping an entity

#define  CONTENTS_MONSTER      0x2000000  // should never be on a brush, only in game
#define  CONTENTS_DEADMONSTER  0x4000000
#define  CONTENTS_DETAIL       0x8000000  // brushes to be added after vis leafs
#define  CONTENTS_TRANSLUCENT  0x10000000 // auto set if any surface has trans
#define  CONTENTS_LADDER       0x20000000
#define  CONTENTS_SHOOTABLE    0x40000000 // is shootable, but may not be blocking


#define  SURF_LIGHT     0x1   // value will hold the light strength
#define  SURF_SLICK     0x2   // effects game physics
#define  SURF_SKY       0x4   // don't draw, but add to skybox
#define  SURF_WARP      0x8   // turbulent water warp
#define  SURF_NONLIT    0x10  // surface is not lit  (SiN: was SURF_TRANS33)
#define  SURF_NOFILTER  0x20  // surface is not filtered (SiN: was SURF_TRANS66)
#define  SURF_CONVEYOR  0x40  // (SiN: was SURF_FLOWING)
#define  SURF_NODRAW    0x80  // don't bother referencing the texture

#define  SURF_WAVY            0x400       // surface has waves
#define  SURF_RICOCHET        0x800       // projectiles bounce literally bounce off this surface
#define  SURF_PRELIT          0x1000      // surface has intensity information for pre-lighting
#define  SURF_CONSOLE         0x4000      // surface has a console on it
#define  SURF_HARDWAREONLY    0x10000     // surface should only do things in hardware
#define  SURF_DAMAGE          0x20000     // surface can be damaged
#define  SURF_WEAK            0x40000     // surface has weak hit points
#define  SURF_NORMAL          0x80000     // surface has normal hit points
#define  SURF_ADD             0x100000    // surface will be additive
#define  SURF_ENVMAPPED       0x200000    // surface is envmapped
#define  SURF_RANDOMANIMATE   0x400000    // surface start animating on a random frame
#define  SURF_ANIMATE         0x800000    // surface animates
#define  SURF_RNDTIME         0x1000000   // time between animations is random
#define  SURF_TRANSLATE       0x2000000   // surface translates
#define  SURF_NOMERGE         0x4000000   // surface is not merged in csg phase
#define  SURF_TYPE_BIT0       0x8000000   // 0 bit of surface type
#define  SURF_TYPE_BIT1       0x10000000  // 1 bit of surface type
#define  SURF_TYPE_BIT2       0x20000000  // 2 bit of surface type
#define  SURF_TYPE_BIT3       0x40000000  // 3 bit of surface type

#define  MASK_SURF_TYPE        (SURF_TYPE_BIT0|SURF_TYPE_BIT1|SURF_TYPE_BIT2|SURF_TYPE_BIT3)

#define  SURF_START_BIT        27
#define  SURFACETYPE_FROM_FLAGS(x) ( ( x >> (SURF_START_BIT) ) & 0xf )

#define  SURF_TYPE_SHIFT(x)   ( (x) << (SURF_START_BIT) ) // macro for getting proper bit mask
#define  SURF_TYPE_NONE       SURF_TYPE_SHIFT(0)
#define  SURF_TYPE_WOOD       SURF_TYPE_SHIFT(1)
#define  SURF_TYPE_METAL      SURF_TYPE_SHIFT(2)
#define  SURF_TYPE_STONE      SURF_TYPE_SHIFT(3)
#define  SURF_TYPE_CONCRETE   SURF_TYPE_SHIFT(4)
#define  SURF_TYPE_DIRT       SURF_TYPE_SHIFT(5)
#define  SURF_TYPE_FLESH      SURF_TYPE_SHIFT(6)
#define  SURF_TYPE_GRILL      SURF_TYPE_SHIFT(7)
#define  SURF_TYPE_GLASS      SURF_TYPE_SHIFT(8)
#define  SURF_TYPE_FABRIC     SURF_TYPE_SHIFT(9)
#define  SURF_TYPE_MONITOR    SURF_TYPE_SHIFT(10)
#define  SURF_TYPE_GRAVEL     SURF_TYPE_SHIFT(11)
#define  SURF_TYPE_VEGETATION SURF_TYPE_SHIFT(12)
#define  SURF_TYPE_PAPER      SURF_TYPE_SHIFT(13)
#define  SURF_TYPE_DUCT       SURF_TYPE_SHIFT(14)
#define  SURF_TYPE_WATER      SURF_TYPE_SHIFT(15)

#define STRONG_DAMAGE_VALUE 200
#define NORMAL_DAMAGE_VALUE 90
#define WEAK_DAMAGE_VALUE 10

// content masks
#define  MASK_ALL           (-1)
#define  MASK_SOLID         (CONTENTS_SOLID|CONTENTS_WINDOW|CONTENTS_FENCE)
#define  MASK_PLAYERSOLID   (CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_WINDOW|CONTENTS_MONSTER|CONTENTS_FENCE)
#define  MASK_DEADSOLID     (CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_WINDOW|CONTENTS_FENCE)
#define  MASK_MONSTERSOLID  (CONTENTS_SOLID|CONTENTS_MONSTERCLIP|CONTENTS_WINDOW|CONTENTS_MONSTER|CONTENTS_FENCE)
#define  MASK_WATER         (CONTENTS_WATER|CONTENTS_LAVA)
#define  MASK_OPAQUE        (CONTENTS_SOLID|CONTENTS_LAVA)
#define  MASK_SHOT          (CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_WINDOW|CONTENTS_DEADMONSTER|CONTENTS_SHOOTABLE)
#define  MASK_PROJECTILE    (CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_WINDOW|CONTENTS_DEADMONSTER|CONTENTS_SHOOTABLE|CONTENTS_FENCE)
#define  MASK_CURRENT       (CONTENTS_CURRENT_0|CONTENTS_CURRENT_90|CONTENTS_CURRENT_180|CONTENTS_CURRENT_270|CONTENTS_CURRENT_UP|CONTENTS_CURRENT_DOWN)
#define  MASK_SOLIDNONFENCE (CONTENTS_SOLID|CONTENTS_WINDOW)

// gi.BoxEdicts() can return a list of either solid or trigger entities
// FIXME: eliminate AREA_ distinction?
#define  AREA_SOLID    1
#define  AREA_TRIGGERS 2

// plane_t structure
// !!! if this is changed, it must be changed in asm code too !!!
typedef struct cplane_s
{
   vec3_t         normal;
   float          dist;
   byte           type;     // for fast side tests
   byte           signbits; // signx + (signy<<1) + (signz<<1)
   byte           pad[2];
} cplane_t;

// structure offset for asm code
#define CPLANE_NORMAL_X 0
#define CPLANE_NORMAL_Y 4
#define CPLANE_NORMAL_Z 8
#define CPLANE_DIST     12
#define CPLANE_TYPE     16
#define CPLANE_SIGNBITS 17
#define CPLANE_PAD0     18
#define CPLANE_PAD1     19

typedef struct cmodel_s
{
   vec3_t mins, maxs;
   vec3_t origin;     // for sounds or lights
   int    headnode;
} cmodel_t;

typedef struct csurface_s
{
   char                groupname[64];     // Name of the surface
   int                 groupnumber;       // Number of the surface
   int                 flags;             // Surface flags
   int                 style;             // Style
   qboolean            trans_state;       // Translation on/off
   float               trans_mag;         // Translation magnitude
   float               trans_angle;       // Translation angle
   float               translucence;      // Translucence of the surface
   float               frequency;         // Frequency of the wavy surface / restitution of that surface
   float               magnitude;         // Magnitude of the wavy surface / friction of that surface
   float               upload_width;      // Power of 2 upload width
   float               upload_height;     // Power of 2 upload height
   vec3_t              transvec;          // Translation vector
   float               nonlit;            // Nonlit vale
   int                 animation_frame;   // Animation frame to display
   int                 numframes;         // Number of frames of animation
   float               animtime;          // Animation time between frames
   int                 frameoffset;       // Offset for random start frame animation
   float               nextanimtime;      // Time in the future to change the animation
   float               delta_s;           // Delta s for the texture coordinates
   float               delta_t;           // Delta t for the texture coordinates
   float               last_update_time;  // Last time the surface was updated
   vec3_t              color;             // Color of the surface
   int                 leaf;              // Leaf number this surface resides in
   struct csurface_s   *next;             // Next surface in the chain
} csurface_t;

typedef struct sinmdl_intersection_s
{
   qboolean valid;
   int    group;
   int    parentgroup;
   int    tri_num;
   vec3_t position;
   vec3_t normal;
   float  damage_multiplier;
} sinmdl_intersection_t;

// a trace is returned when a box is swept through the world
typedef struct trace_s
{
   qboolean    allsolid;   // if true, plane is not valid
   qboolean    startsolid; // if true, the initial point was in a solid area
   float       fraction;   // time completed, 1.0 = didn't hit anything
   vec3_t      endpos;     // final position
   cplane_t    plane;      // surface normal at impact
   csurface_t *surface;    // surface hit
   int         contents;   // contents on other side of surface hit
   struct edict_s *ent;    // not set by CM_*() functions
   vec3_t      dir;        // the direction of the trace
   sinmdl_intersection_t intersect; // set if the trace hit a specific polygon
} trace_t;

// pmove_state_t is the information necessary for client side movement
// prediction
typedef enum
{
   // can accelerate and turn
   PM_NORMAL,
   PM_ZOOM,
   PM_INVEHICLE_ZOOM,
   PM_SPECTATOR,
   PM_INVEHICLE,
   PM_LOCKVIEW,
   // no acceleration or turning
   PM_DEAD,
   PM_GIB,          // different bounding box
   PM_FREEZE,
   PM_MOVECAPTURED, // using a movement capturer
   PM_ONBIKE,       // added for hoverbike
   PM_ATTACHVIEW,   // added for guided missile
   PM_GRAPPLE_PULL,
} pmtype_t;

// pmove->pm_flags
#define  PMF_DUCKED           (1<<0)
#define  PMF_JUMP_HELD        (1<<1)
#define  PMF_ON_GROUND        (1<<2)
#define  PMF_TIME_WATERJUMP   (1<<3)    // pm_time is waterjump
#define  PMF_TIME_LAND        (1<<4)    // pm_time is time before rejump
#define  PMF_TIME_TELEPORT    (1<<5)    // pm_time is non-moving time
#define  PMF_NO_PREDICTION    (1<<6)    // temporarily disables prediction (used for grappling hook)
#define  PMF_MOREBITS         (1<<7)
#define  PMF_OLDNOCLIP        (1<<8)
#define  PMF_MUTANT           (1<<9)
#define  PMF_ADRENALINE       (1<<10)
#define  PMF_NOAIRCLAMP       (1<<11)

#define CROUCH_HEIGHT      36
#define CROUCH_EYE_HEIGHT  30
#define STAND_HEIGHT       72
#define STAND_EYE_HEIGHT   66
// 2015 code 
#define HOVERBIKE_HEIGHT      32
#define HOVERBIKE_EYE_HEIGHT  24

//
// this structure needs to be communicated bit-accurate
// from the server to the client to guarantee that
// prediction stays in sync, so no floats are used.
// if any part of the game code modifies this struct, it
// will result in a prediction error of some degree.
//
typedef struct pmove_state_s
{
   pmtype_t pm_type;

   short    origin[3];     // 12.3
   short    velocity[3];   // 12.3
   short    pm_flags;      // ducked, jump_held, etc
   byte     pm_time;       // each unit = 8 ms

   short    gravity : 13;
   unsigned gravity_axis : 3; // direction of gravity

   short    delta_angles[3];  // add to command angles to get view direction
                              // changed by spawns, rotating objects, and teleporters
} pmove_state_t;

#define GRAVITY_AXIS_SHIFT 13
#define GRAVITY_AXIS_MASK  0xe000
#define GRAVITY_MASK       (~GRAVITY_AXIS_MASK)
#define GRAVITY_NUM_AXIS   6

typedef struct gravityaxis_s
{
   int x;
   int y;
   int z;
   int sign;
} gravityaxis_t;

extern const gravityaxis_t gravity_axis[GRAVITY_NUM_AXIS];

//
// button bits
//
#define BUTTON_ATTACK     1
#define BUTTON_USE        2
// 2015 code
#define BUTTON_JUMP       4    // for detecting when the player is jumping
#define BUTTON_ANY        128  // any key whatsoever

// usercmd_t is sent to the server each client frame
typedef struct usercmd_s
{
   byte  msec;
   byte  buttons;
   short angles[3];
   short forwardmove, sidemove, upmove;
   byte  impulse;    // remove?
   byte  lightlevel; // light level the player is standing on
   byte  debris;     // have we been hit by debris?
} usercmd_t;

#define  MAXTOUCH  32

typedef struct pmove_s
{
   // state (in / out)
   pmove_state_t  s;

   // command (in)
   usercmd_t      cmd;
   qboolean       snapinitial;  // if s has been changed outside pmove

   // results (out)
   int            numtouch;
   struct edict_s *touchents[MAXTOUCH];

   vec3_t         viewangles; // clamped
   float          viewheight;

   vec3_t         mins, maxs; // bounding box size

   struct edict_s *groundentity;

   csurface_t     *groundsurface;
   cplane_t       groundplane;
   int            groundcontents;

   int            watertype;
   int            waterlevel;

   qboolean       onladder;         // is he on a ladder

   // callbacks to test the world
   trace_t (*trace        )(vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end);
   int     (*pointcontents)(vec3_t point);
} pmove_t;


// entity_state_t->effects
// Effects are things handled on the client side (lights, particles, frame animations)
// that happen constantly on the given entity.
// An entity that has effects will be sent to the client
// even if it has a zero index model.
#define  EF_ROTATE         (1<<0)         // rotate (bonus items)
#define  EF_ROCKET         (1<<1)         // redlight + trail
#define  EF_GIB            (1<<2)         // leave a trail
#define  EF_PULSE          (1<<3)         // pulsecannon projectile effect
#define  EF_SMOOTHANGLES   (1<<4)         // 16-bit angles over net
#define  EF_EVERYFRAME     (1<<5)         // def commands will be run every client frame
#define  EF_CROUCH         (1<<6)         // character is crouching
#define  EF_EFFECTS_16     (1<<7)         // reserved for net transmission
#define  EF_GRAVITY_AXIS_0 (1<<8)         // bit 0 of gravity axis
#define  EF_GRAVITY_AXIS_1 (1<<9)         // bit 1 of gravity axis
#define  EF_GRAVITY_AXIS_2 (1<<10)        // bit 2 of gravity axis
#define  EF_BEAMEFFECT     (1<<11)        // Beams effect
#define  EF_AUTO_ANIMATE   (1<<12)        // auto animate
#define  EF_NOFOOTSTEPS    (1<<13)        // don't play footsteps on model
#define  EF_EFFECTS_32     (1<<15)        // reserved for net transmission

#define  EF_WARM            (1<<14)       // used to signify something that should be
                                          // light up by therm-optic goggles
#define  EF_HOVER           (1<<16)       // hovering effect for the hoverbike
#define  EF_HOVERTHRUST     (1<<17)       // engine thrust hoverbike effect
#define  EF_HOVERTURBO      (1<<18)       // turbo thrust hoverbike effect
#define  EF_VIEWMODEL       (1<<19)       // entity is a secondary view model
#define  EF_ANIMEROCKET     (1<<20)       // added effect for anime rockets
#define  EF_FLAMES          (1<<21)       // added flame trail effect
#define  EF_DEATHFLAMES     (1<<22)       // added flame trail effect
#define  EF_PLASMATRAIL1    (1<<23)       // trail effect for plasma bow
#define  EF_PLASMATRAIL2    (1<<24)       // secondary trail effect for plasma bow
#define  EF_NUKETRAIL       (1<<25)       // particle effect for a flying nuke ball

#define  EF_FORCEBASELINE  EF_CROUCH      // used to make sure we create a baseline for entities that wouldn't be processed
                                          // ordinarily.

#define EFFECTS_TO_GRAVITYAXIS( effect ) \
   (((effect) & (EF_GRAVITY_AXIS_0 | EF_GRAVITY_AXIS_1 | EF_GRAVITY_AXIS_2)) >> 8)

#define GRAVITYAXIS_TO_EFFECTS( gravaxis ) \
   (((gravaxis) << 8) & (EF_GRAVITY_AXIS_0 | EF_GRAVITY_AXIS_1 | EF_GRAVITY_AXIS_2))

// model commands

#define  MDL_GROUP_SKINOFFSET_BIT0  0x0001
#define  MDL_GROUP_SKINOFFSET_BIT1  0x0002
#define  MDL_GROUP_FULLBRIGHT       0x0004
#define  MDL_GROUP_ENVMAPPED_GOLD   0x0008
#define  MDL_GROUP_ENVMAPPED_SILVER 0x0010
#define  MDL_GROUP_TRANSLUCENT_33   0x0020
#define  MDL_GROUP_TRANSLUCENT_66   0x0040
#define  MDL_GROUP_NODRAW           0x0080

//
// STATIC flags NEVER sent over the net
//
#define  MDL_GROUP_SURFACETYPE_BIT0 0x0100
#define  MDL_GROUP_SURFACETYPE_BIT1 0x0200
#define  MDL_GROUP_SURFACETYPE_BIT2 0x0400
#define  MDL_GROUP_SURFACETYPE_BIT3 0x0800
#define  MDL_GROUP_RICOCHET         0x1000
#define  MDL_GROUP_SKIN_NO_DAMAGE   0x2000
#define  MDL_GROUP_MASKED           0x4000
#define  MDL_GROUP_TWOSIDED         0x8000

// 2015 code
#define BONE_ORIGIN        0xfff
#define BONE_2015ATTACH    0x800


// Particle flags
#define PARTICLE_RANDOM       (1<<0)
#define PARTICLE_OVERBRIGHT   (1<<1)
// 2015 code
#define PARTICLE_ADDITIVE     (1<<2) // draws the particle additively
#define PARTICLE_FIRE         (1<<3) // draws it as a fire particle
#define PARTICLE_HITSOLID     (1<<4) // collision detect solid objects
#define PARTICLE_HITENTS      (1<<5) // collision detect entities
#define PARTICLE_HITREMOVE    (1<<6) // remove the particle when it hits something

// entity_state_t->renderfx flags
#define  RF_MINLIGHT       (1<<0)      // allways have some light (viewmodel)
#define  RF_VIEWERMODEL    (1<<1)      // don't draw through eyes, only mirrors
#define  RF_WEAPONMODEL    (1<<2)      // only draw through eyes
#define  RF_FULLBRIGHT     (1<<3)      // allways draw full intensity
#define  RF_ENVMAPPED      (1<<4)      // model is environment mapped completely
#define  RF_TRANSLUCENT    (1<<5)
#define  RF_FRAMELERP      (1<<6)
#define  RF_BEAM           (1<<7)
#define  RF_CUSTOMSKIN     (1<<8)      // skin is an index in image_precache
#define  RF_GLOW           (1<<9)      // pulse lighting for bonus items
#define  RF_DONTDRAW       (1<<10)
#define  RF_LENSFLARE      (1<<11)
#define  RF_DLIGHT         (1<<12)
#define  RF_DETAIL         (1<<13)     // Culls a model based on the distance away from you
#define  RF_LIGHTOFFSET    (1<<14)
#define  RF_XFLIP          (1<<15)     // flip the model in x space (used for weapon models)

// player_state_t->refdef flags
#define RDF_UNDERWATER     1     // warp the screen as apropriate
#define RDF_NOWORLDMODEL   2     // used for player configuration screen
#define RDF_NOCLEAR        4     // used for player configuration screen, don't clear behind the screen
#define RDF_NOENTITIES     8     // used for menu, so entites don't get drawn

//
// temp entity events
//
// Temp entity events are for things that happen
// at a location seperate from any existing entity.
// Temporary entity messages are explicitly constructed
// and broadcast.
//
typedef enum
{
   TE_GUNSHOT,
   TE_PARTICLES,
   TE_RANDOM_PARTICLES,
   TE_TESSELATE,
   TE_LASER,
   TE_BURNWALL,
   TE_TEMPMODEL,
   TE_STRIKE,
   TE_DLIGHT,
   TE_ROCKET_EXPLOSION,
   TE_BEAM,
   TE_DAMAGE_WALL,
   TE_TELEPORT_EFFECT,
   TE_BULLET_SPRAY,
   TE_SCALED_EXPLOSION,
   TE_PULSE_EXPLOSION,
   // 2015 code
   TE_FLAMETHROWER,           // flames for the flamethrower
   TE_FLAMETHROWERROW,        // row of flames for the flamethrower
   TE_FLAMETHROWERHIT,        // hit flames for the flamethrower
   TE_FLAME,                  // a puff or row of flames (multipurpose)
   TE_NUKE_EXPLOSION,         // fire explosion entity for the nuke
   TE_BOW_EXPLOSION,          // explosion entity for the bow
   TE_HOVERBOOSTER,           // vertical booster effect for the hoverbike
   TE_SIZED_PARTICLES,
   TE_RANDOM_SIZED_PARTICLES,
   TE_PARTICLES_FULL,
   TE_RANDOM_PARTICLES_FULL,
   TE_TRACER                  // bullet tracer
} temp_event_t;

#define TM_ORIGIN 0x01
#define TM_ANGLES 0x02
#define TM_ANIM   0x04
#define TM_SCALE  0x08
#define TM_FLAGS  0x10
#define TM_OWNER  0x20
#define TM_LIFE   0x40
#define TM_ALPHA  0x80

#define TEMPMODEL_RANDOM_ROLL    0x0001
#define TEMPMODEL_AUTO_ANIMATE   0x0002
#define TEMPMODEL_ANIMATE_ONCE   0x0004
#define TEMPMODEL_ANIMATE_FAST   0x0008
#define TEMPMODEL_RANDOM_PITCH   0x0010
#define TEMPMODEL_RANDOM_YAW     0x0020
#define TEMPMODEL_ALPHAFADE      0x0040
#define TEMPMODEL_ANIMATE_SCALE  0x0080

//
// these flags are not sent over the net for now
//
#define TEMPMODEL_USE_PHYSICS          0x0100
#define TEMPMODEL_BOUNCE               0x0200
#define TEMPMODEL_TOUCH_DIE            0x0400
#define TEMPMODEL_LERP                 0x0800
#define TEMPMODEL_FRAMELERP            0x1000
#define TEMPMODEL_TOUCH_DIE_NO_FADE    0x2000

#define BM_ALPHA                    (1<<0)
#define BM_SCALE                    (1<<1)
#define BM_FLAGS                    (1<<2)
#define BM_LIFE                     (1<<3)

#define BEAM_AUTO_ANIMATE           (1<<0)
#define BEAM_ANIMATE_ONCE           (1<<1)
#define BEAM_ANIMATE_FAST           (1<<2)
#define BEAM_ROLL                   (1<<3)
#define BEAM_ANIMATE_RANDOM_START   (1<<4)
#define BEAM_LIGHTNING_EFFECT       (1<<5)
#define BEAM_RANDOM_ALPHA           (1<<6)

#define TESS_ORIGIN        0x01
#define TESS_MINSIZE       0x02
#define TESS_MAXSIZE       0x04
#define TESS_DIR           0x08
#define TESS_POWER         0x10
#define TESS_PERCENT       0x20
#define TESS_THICK         0x40
#define TESS_ENTNUM        0x80
#define TESS_LIGHTSTYLE    0x100
#define TESS_TYPE          0x200

#define TESSELATE_FALL              1
#define TESSELATE_EXPANDANDSHRINK   2
#define TESSELATE_EXPLODE           3
#define TESSELATE_FALLANDTRAIL      4
#define TESSELATE_IMPLODE           5

#define TESS_DEFAULT_MIN_SIZE    10
#define TESS_DEFAULT_MAX_SIZE    25
#define TESS_DEFAULT_PERCENT     0.15f
#define TESS_DEFAULT_POWER       128
#define TESS_DEFAULT_LIGHTSTYLE  125
#define TESS_DEFAULT_TYPE        0

#define SPLASH_UNKNOWN     0
#define SPLASH_SPARKS      1
#define SPLASH_BLUE_WATER  2
#define SPLASH_BROWN_WATER 3
#define SPLASH_SLIME       4
#define SPLASH_LAVA        5
#define SPLASH_BLOOD       6

//
// sound channels
// channel 0 never willingly overrides
// other channels (1-7) allways override a playing sound on that channel
//
#define  CHAN_AUTO               0
#define  CHAN_WEAPON             1
#define  CHAN_VOICE              2
#define  CHAN_ITEM               3
#define  CHAN_BODY               4
#define  CHAN_DIALOG             5
#define  CHAN_DIALOG_SECONDARY   6
#define  CHAN_WEAPONIDLE         7
// modifier flags
#define  CHAN_NO_PHS_ADD         8  // send to all clients, not just ones in PHS (ATTN 0 will also do this)
#define  CHAN_RELIABLE           16 // send by reliable message, not datagram

// sound attenuation values
#define  ATTN_NONE               0  // full volume the entire level
#define  ATTN_NORM               1
#define  ATTN_IDLE               2
#define  ATTN_STATIC             3  // diminish very rapidly with distance

#define STAT_HEALTH               0
#define STAT_WEAPONLIST           1
#define STAT_AMMO                 2
#define STAT_SELECTED_ICON        3  // Icon of currently selected inventory item
#define STAT_SELECTED_AMOUNT      4  // Amount of this item
#define STAT_PREVIOUS_ICON        5  // Icon of previous inventory item
#define STAT_NEXT_ICON            6  // Icon of next inventory item
#define STAT_PICKUP_ICON          7
#define STAT_PICKUP_STRING        8
#define STAT_LAYOUTS              9
#define STAT_FRAGS               10
#define STAT_FLASHES             11
#define STAT_AMMO_BULLET357      12
#define STAT_AMMO_SHOTGUN        13
#define STAT_AMMO_BULLET10MM     14
#define STAT_AMMO_BULLET50MM     15
#define STAT_AMMO_BULLETPULSE    16
#define STAT_AMMO_BULLETSNIPER   17
#define STAT_AMMO_ROCKETS        18
#define STAT_AMMO_SPIDERMINES    19
#define STAT_ARMOR_HEAD          20
#define STAT_ARMOR_BODY          21
#define STAT_ARMOR_LEGS          22
#define STAT_ARMOR               23
#define STAT_SELECTED_MODELINDEX 24
#define STAT_CURRENT_WEAPON      25
#define STAT_CROSSHAIR           26
#define STAT_CLIPAMMO            27
#define STAT_POWERUPTIMER        28
#define STAT_POWERUPTYPE         29
#define STAT_SELECTED_NAME       30
#define STAT_EXITSIGN            31    // cleared each frame

// 2015 code 
#define STAT_LASTLAP       12 // this is set to the client's last lap time * 10
#define STAT_CURRENTLAP    13 // this is set to the client's current lap time * 10
#define STAT_CPCOUNT       14 // number of checkpoints left to touch this lap
#define STAT_NIGHTVISION   15 // set when player is in night vision mode

#ifdef SIN_ARCADE
#define STAT_FIRSTPLACE       16  // set if the player is in first place
#define STAT_DRAWFIRSTPLACE   17  // set if the player is in first place and we should show the icon
#endif

#define P_SHIELDS                 1
#define P_ADRENALINE              2
#define P_CLOAK                   3
#define P_MUTAGEN                 4
#define P_OXYGEN                  5

#define MAX_STATS             32

#define NUM_AMMO_TYPES        8
// 2015 code
// added new ammo types
#define NUM_2015_AMMO_TYPES   4 // number of additional 2015 ammo types
#define STAT_AMMO_BASE        (STAT_AMMO_BULLET357)
#define NUM_ARMOR_TYPES       3
#define STAT_ARMOR_BASE       (STAT_ARMOR_HEAD)

#define DRAW_STATS            (1<<0)
#define DRAW_SPECTATOR        (1<<1)
#define DRAW_MISSIONCPU       (1<<2)
#define DRAW_SCORES           (1<<3)
#define DRAW_OVERLAY          (1<<4)

// dmflags->value flags
#define DF_NO_HEALTH          (1<<0)
#define DF_NO_POWERUPS        (1<<1)
#define DF_WEAPONS_STAY       (1<<2)
#define DF_NO_FALLING         (1<<3)
#define DF_INSTANT_ITEMS      (1<<4)
#define DF_SAME_LEVEL         (1<<5)
#define DF_SKINTEAMS          (1<<6)
#define DF_MODELTEAMS         (1<<7)
#define DF_FRIENDLY_FIRE      (1<<8)
#define DF_SPAWN_FARTHEST     (1<<9)
#define DF_FORCE_RESPAWN      (1<<10)
#define DF_NO_ARMOR           (1<<11)
#define DF_FAST_WEAPONS       (1<<12)
#define DF_NOEXIT             (1<<13)
#define DF_INFINITE_AMMO      (1<<14)
#define DF_FIXED_FOV          (1<<15)
#define DF_NO_DROP_WEAPONS    (1<<16)
#define DF_NO_WEAPON_CHANGE   (1<<17)

// 2015 code
#define DF_BBOX_BULLETS    (1<<17)
#define DF_AUTO_INFORMER   (1<<18)
#define DF_INFORMER_LOCK   (1<<19)
#define DF_MIDNIGHT        (1<<20) // wasn't able to get implimented
#define DF_FLASHLIGHT      (1<<21)
#define DF_FLASHLIGHTON    (1<<22)
#define DF_GOGGLES         (1<<23)
#define DF_GOGGLESON       (1<<24)

/*
==========================================================

  ELEMENTS COMMUNICATED ACROSS THE NET

==========================================================
*/

#define  ANGLE2SHORT(x)  ((int)((x)*65536/360) & 65535)
#define  SHORT2ANGLE(x)  ((x)*(360.0/65536))

//
// config strings are a general means of communication from
// the server to all connected clients.
// Each config string can be at most MAX_QPATH characters.
//
#define  CS_NAME           0
#define  CS_CDTRACK        1
#define  CS_SKY            2
#define  CS_SKYAXIS        3      // %f %f %f format
#define  CS_SKYROTATE      4
#define  CS_MAXCLIENTS     5
#define  CS_STATUSBAR      6     // display program string
                                 // STATUSBAR reserves configstrings from 6-30
#define  CS_MAPCHECKSUM    31    // for catching cheater maps
#define  CS_MODELS         32
#define  CS_SOUNDS         (CS_MODELS+MAX_MODELS)
#define  CS_IMAGES         (CS_SOUNDS+MAX_SOUNDS)
#define  CS_LIGHTS         (CS_IMAGES+MAX_IMAGES)
#define  CS_ITEMS          (CS_LIGHTS+MAX_LIGHTSTYLES)
#define  CS_PLAYERSKINS    (CS_ITEMS+MAX_ITEMS)
#define  CS_SOUNDTRACK     (CS_PLAYERSKINS+MAX_CLIENTS)

// 2015 code
#define  CS_CHECKPOINTS        (CS_SOUNDTRACK + 1)
#define  CS_BIKESKINS          (CS_CHECKPOINTS + 1)
#define  MAX_CONFIGSTRINGS     (CS_BIKESKINS + MAX_CLIENTS) // # of config strings with 2015 stuff
#define  MAX_NORMCONFIGSTRINGS (CS_SOUNDTRACK + 1)          // # of config strings in original Sin

//==============================================

//
// entity_state_t->event values
// ertity events are for effects that take place reletive
// to an existing entities origin.  Very network efficient.
// All muzzle flashes really should be converted to events...
//
typedef enum
{
   EV_NONE,
   EV_ITEM_RESPAWN,
   EV_FOOTSTEP,
   EV_FALLSHORT,
   EV_MALE_FALL,
   EV_MALE_FALLFAR,
   EV_FEMALE_FALL,
   EV_FEMALE_FALLFAR,
   EV_PLAYER_TELEPORT
} entity_event_t;

#define MAX_MODEL_GROUPS   24
typedef struct bone_s
{
   int              group_num; // the group which the triangle belongs to
   int              tri_num; // the actual triangle at which the bone is
   vec3_t           orientation; // orientation of bone
} bone_t;

//
// entity_state_t is the information conveyed from the server
// in an update message about entities that the client will
// need to render in some way
//
typedef struct entity_state_s
{
   int      number;         // edict index

   vec3_t   origin;
   vec3_t   angles;

   float    quat[4];
   float    mat[3][3];

   vec3_t   old_origin;     // for lerping
   int      modelindex;
   int      frame;
   int      prevframe;
   vec3_t   vieworigin;
   vec3_t   viewangles;
   int      anim;
   float    scale;
   float    alpha;
   float    color_r;
   float    color_g;
   float    color_b;
   int      radius;
   bone_t   bone;
   int      parent;
   int      numgroups;
   byte     groups[MAX_MODEL_GROUPS];
   int      gunanim;
   int      gunframe;
   int      gunmodelindex;
   int      lightofs;

   int      skinnum;
   int      effects;
   int      renderfx;
   int      solid;   // for client side prediction
                     // (bits 0-7) is x radius
                     // (bits 8-15) is y radius
                     // (bits 16-21) is z down distance
                     // (bits 22-30) is z up distance
                     // gi.linkentity sets this properly

   int      sound;   // for looping sounds, to guarantee shutoff
   int      event;   // impulse events -- muzzle flashes, footsteps, etc
                     // events only go out for a single frame, they
                     // are automatically cleared each frame
} entity_state_t;

// means of death flags

typedef enum 
{
   MOD_FISTS,
   MOD_MAGNUM,
   MOD_SHOTGUN,
   MOD_ASSRIFLE,
   MOD_CHAINGUN,
   MOD_GRENADE,
   MOD_ROCKET,
   MOD_ROCKETSPLASH,
   MOD_PULSE,
   MOD_PULSELASER,
   MOD_SPEARGUN,
   MOD_SNIPER,
   MOD_VEHICLE,
   MOD_CRUSH,
   MOD_SHOTROCKET,
   MOD_FALLING,
   MOD_DROWN,
   MOD_SUICIDE,
   MOD_EXPLODEWALL,
   MOD_ELECTRIC,
   MOD_TELEFRAG,
   MOD_GENBULLET,
   MOD_LASER,
   MOD_BETTYSPIKE,
   MOD_HELIGUN,
   MOD_DEBRIS,
   MOD_THROWNOBJECT,
   MOD_LAVA,
   MOD_SLIME,
   MOD_ADRENALINE,
   MOD_ION,
   MOD_ION_DESTRUCT,
   MOD_QUANTUM,
   MOD_BEAM,
   MOD_IMPACT,
   MOD_FRIENDLY_FIRE,
   MOD_SPIDERSPLASH,
   MOD_MUTANTHANDS,
   MOD_MUTANT_DRAIN,
   //### MoD's for 2015 stuff
   MOD_STINGERROCKET,
   MOD_STINGERSPLASH,
   MOD_PLASMABOW,
   MOD_PLASMABOWSPLASH,
   MOD_CONCUSSION,
   MOD_MISSILE,
   MOD_MISSILESPLASH,
   MOD_NUKE,
   MOD_NUKEEXPLOSION,
   MOD_FLAMETHROWER,
   MOD_HOVERBIKE,
   MOD_HB_ROCKET,
   MOD_HB_ROCKETSPLASH,
   MOD_HB_GUN,
   MOD_HB_MINE,
   //###
   MOD_GRAPPLE,
   MOD_EMPATHY,
   MOD_THRALLBALL,
   MOD_THRALLSPLASH,
   MOD_DEATHQUAD,
   MOD_CTFTURRET,
   MOD_RESET         //### used for when the server is reset
} mod_type_t;

// CONSOLE3D State
#define MAIN_CONSOLE             "maincon"
#define GENERIC_CONSOLE          "gencon"
#define MISSION_CONSOLE          "missioncon"
#define MAX_LAYOUT_LENGTH        1024
#define MAX_CONSOLE_NAME         32
#define MAX_BUFFER_LENGTH        1024
#define MAX_CON_LINES            16
#define MAX_STATUSBARS           32
#define MAXCMDLINE               256

typedef enum { MENU3D, CONSOLE3D } consolefocus_t;

typedef struct surface_state_s
{
   qboolean    changed;
   char        *name;
   int         number;
   int         groupnumber;
   qboolean    trans_state;
   float       trans_mag;
   float       trans_angle;
   float       translucence;
   float       magnitude;
   float       frequency;
   int         damage_frame;
} surface_state_t;

typedef struct statusbar_s
{
   float width, height;
   float min, max;
   float value;
   float red, green, blue, alpha;
   float update_time;
} statusbar_t;

typedef struct console_state_s
{
   int            number;                          // Console number
   char           console_name[MAX_CONSOLE_NAME];  // Console name
   int            spawnflags;                      // Console type (flag 2 = Scroll, flag 4 = Menu, flag 8 = IgnorePVS)
   float          create_time;                     // Console has been initially created.
   float          layout_update_time;              // Layout updated
   float          menufile_update_time;            // Menufile updated
   float          layoutfile_update_time;          // Layoutfile updated
   float          cleared_console_time;            // Cleared the console
   float          name_update_time;                // Changed the console name
   float          console_return_time;             // Hit return on the console
   float          red, green, blue, alpha;         // Foreground colors
   char           layout[MAX_LAYOUT_LENGTH];       // Current layout
   char           menu_file[MAX_OSPATH];           // menufile filename
   char           layout_file[MAX_OSPATH];         // layoutfile filename
   float          virtual_width;                   // User defined virtual width
   float          virtual_height;                  // User defined virtual height
   float          fraction;                        // Fraction of the scrolling console to draw
   int            rows;                            // Number of rows
   int            cols;                            // Number of cols
   int            linepos;                         // Cursor position
   char           cmdline[MAXCMDLINE];             // Input line text;
   int            menu_level;                      // Current menu level
   int            sel_menu_item;                   // Currently selected item
   consolefocus_t focus;                           // Current focus (Menu or scrolling)
   qboolean       consoleactive;                   // Console is active
   qboolean       menuactive;                      // Menu is active
   statusbar_t    sbar[MAX_STATUSBARS];            // Status bar states
   int            console_owner;                   // entity number of owner
} console_state_t;

typedef struct console_buffer_state_s
{
   int  start_index;               // Absolute index of the first character in the buffer
   int  end_index;                 // Absolute index of the last character in the buffer
   int  start;                     // Index of the first character in the circular buffer
   int  end;                       // Index of the last character in the circular buffer
   char buffer[MAX_BUFFER_LENGTH]; // State of the buffer for the console
} console_buffer_state_t;

//==============================================

typedef enum
{
   mood_none,
   mood_normal,
   mood_action,
   mood_suspense,
   mood_mystery,
   mood_success,
   mood_failure,
   mood_surprise,
   mood_special,
   mood_aux1,
   mood_aux2,
   mood_aux3,
   mood_aux4,
   mood_aux5,
   mood_aux6,
   mood_aux7,
   mood_totalnumber
} music_mood_t;

//
// player_state_t is the information needed in addition to pmove_state_t
// to rendered a view.  There will only be 10 player_state_t sent each second,
// but the number of pmove_state_t changes will be reletive to client
// frame rates
//
typedef struct player_state_s
{
   pmove_state_t pmove;          // for prediction

   // these fields do not need to be communicated bit-precise

   vec3_t        viewangles;     // for fixed views
   vec3_t        viewoffset;     // add to pmovestate->origin
   vec3_t        kick_angles;    // add to view direction to get render angles
                                 // set by weapon kicks, pain effects, etc

   vec3_t        gunangles;
   vec3_t        gunoffset;

   // for client side commands
   int           last_gunframe;
   int           last_gunanim;

   // client side music mood
   byte          current_music_mood;
   byte          fallback_music_mood;

   float         blend[4];     // rgba full screen effect

   float         fov;          // horizontal field of view

   int           rdflags;      // refdef flags

   short         stats[MAX_STATS]; // fast status bar updates
} player_state_t;

#define SINMDL_CMD_MAX_CMDS 128
#define SINMDL_CMD_MAX_ARGS 32

typedef struct sinmdl_singlecmd_s
{
   int num_args;
   char *args[SINMDL_CMD_MAX_ARGS];
} sinmdl_singlecmd_t;

typedef struct sinmdl_cmd_s
{
   int num_cmds;
   sinmdl_singlecmd_t cmds[SINMDL_CMD_MAX_CMDS];
} sinmdl_cmd_t;

#define SOUND_SYNCH           0x1
#define SOUND_SYNCH_FADE      0x2
#define SOUND_RANDOM_PITCH_20 0x4
#define SOUND_RANDOM_PITCH_40 0x8
#define SOUND_LOCAL_DIALOG    0x10

typedef struct debugline_s
{
   vec3_t   start;
   vec3_t   end;
   vec3_t   color;
   float    alpha;
} debugline_t;

void MatrixToEulerAngles(float mat[3][3], vec3_t ang);
void TransposeMatrix(float in[3][3], float out[3][3]);
void MatrixTransformVector(vec3_t in, float mat[3][3], vec3_t out);
void Matrix4TransformVector(vec3_t in, float mat[4][4], vec3_t out);
void OrthoNormalize(float mat[3][3]);
void QuatToMat(float q[4], float mat[3][3]);
void MatToQuat(float mat[3][3], float q[4]);
void SlerpQuaternion(float p[4], float q[4], float t, float qt[4]);
void AnglesToMat(float ang[3], float mat[3][3]);
void RotateAxis(float axis[3], float angle, float q[4]);
void MultQuat(float q1[4], float q2[4], float out[4]);
void EulerToQuat(float ang[3], float q[4]);
void VectorsToEulerAngles(vec3_t forward, vec3_t right, vec3_t up, vec3_t ang);

void TransformFromTriangle(const float tri[3][3], float trans[3][3], vec3_t pos);

// 2015 code
void TransformFromTriangle_2015(const float tri[3][3], float trans[3][3], vec3_t pos);

void OriginFromTriangle(const float tri[3][3], vec3_t pos);
void CalculateRotatedBounds(vec3_t angles, vec3_t mins, vec3_t maxs);
void CalculateRotatedBounds2(float trans[3][3], vec3_t mins, vec3_t maxs);

int MusicMood_NameToNum(const char * name);
const char * MusicMood_NumToName(int num);

float SURFACE_DamageMultiplier(int flags);

#ifdef __cplusplus
}
#endif

#endif

// EOF

