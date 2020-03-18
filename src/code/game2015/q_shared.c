//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/q_shared.c                       $
// $Revision:: 31                                                             $
//   $Author:: Markd                                                          $
//     $Date:: 1/26/99 5:45p                                                  $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// 

#include "q_shared.h"
#include "float.h"

#define DEG2RAD( a ) ( a * M_PI ) / 180.0F

vec3_t vec3_origin = {0,0,0};

#define X 0
#define Y 1
#define Z 2
#define W 3
#define QUAT_EPSILON 0.00001

const gravityaxis_t gravity_axis[GRAVITY_NUM_AXIS] =
{
   { X, Y, Z,  1 },
   { Y, Z, X,  1 },
   { Z, X, Y,  1 },
   { X, Y, Z, -1 },
   { Y, Z, X, -1 },
   { Z, X, Y, -1 }
};

//============================================================================

// SINEX_FIXME: test removal of all optimize("", off) pragmas
#ifdef _WIN32
#pragma optimize( "", off )
#endif

void RotatePointAroundVector(vec3_t dst, const vec3_t dir, const vec3_t point, float degrees)
{
   float m[3][3];
   float im[3][3];
   float zrot[3][3];
   float tmpmat[3][3];
   float rot[3][3];
   int   i;
   vec3_t vr, vup, vf;

   vf[0] = dir[0];
   vf[1] = dir[1];
   vf[2] = dir[2];

   PerpendicularVector(vr, dir);
   CrossProduct(vr, vf, vup);

   m[0][0] = vr[0];
   m[1][0] = vr[1];
   m[2][0] = vr[2];

   m[0][1] = vup[0];
   m[1][1] = vup[1];
   m[2][1] = vup[2];

   m[0][2] = vf[0];
   m[1][2] = vf[1];
   m[2][2] = vf[2];

   memcpy(im, m, sizeof(im));

   im[0][1] = m[1][0];
   im[0][2] = m[2][0];
   im[1][0] = m[0][1];
   im[1][2] = m[2][1];
   im[2][0] = m[0][2];
   im[2][1] = m[1][2];

   memset(zrot, 0, sizeof(zrot));
   zrot[0][0] = zrot[1][1] = zrot[2][2] = 1.0F;

   zrot[0][0] = cos(DEG2RAD(degrees));
   zrot[0][1] = sin(DEG2RAD(degrees));
   zrot[1][0] = -sin(DEG2RAD(degrees));
   zrot[1][1] = cos(DEG2RAD(degrees));

   R_ConcatRotations(m, zrot, tmpmat);
   R_ConcatRotations(tmpmat, im, rot);

   for(i = 0; i < 3; i++)
   {
      dst[i] = rot[i][0] * point[0] + rot[i][1] * point[1] + rot[i][2] * point[2];
   }
}

#ifdef _WIN32
#pragma optimize( "", on )
#endif

void AngleVectors(vec3_t angles, vec3_t forward, vec3_t right, vec3_t up)
{
   float		angle;
   static float		sr, sp, sy, cr, cp, cy;
   // SINEX_FIXME
   // static to help MS compiler fp bugs

   angle = angles[YAW] * (M_PI * 2 / 360);
   sy = sin(angle);
   cy = cos(angle);
   angle = angles[PITCH] * (M_PI * 2 / 360);
   sp = sin(angle);
   cp = cos(angle);
   angle = angles[ROLL] * (M_PI * 2 / 360);
   sr = sin(angle);
   cr = cos(angle);

   if(forward)
   {
      forward[0] = cp*cy;
      forward[1] = cp*sy;
      forward[2] = -sp;
   }
   if(right)
   {
      right[0] = (-1 * sr*sp*cy + -1 * cr*-sy);
      right[1] = (-1 * sr*sp*sy + -1 * cr*cy);
      right[2] = -1 * sr*cp;
   }
   if(up)
   {
      up[0] = (cr*sp*cy + -sr*-sy);
      up[1] = (cr*sp*sy + -sr*cy);
      up[2] = cr*cp;
   }
}

void ProjectPointOnPlane( vec3_t dst, const vec3_t p, const vec3_t normal )
{
   float d;
   vec3_t n;
   float inv_denom;

   inv_denom = 1.0F / DotProduct(normal, normal);

   d = DotProduct(normal, p) * inv_denom;

   n[0] = normal[0] * inv_denom;
   n[1] = normal[1] * inv_denom;
   n[2] = normal[2] * inv_denom;

   dst[0] = p[0] - d * n[0];
   dst[1] = p[1] - d * n[1];
   dst[2] = p[2] - d * n[2];
}

/*
** assumes "src" is normalized
*/
void PerpendicularVector(vec3_t dst, const vec3_t src)
{
   int	pos;
   int i;
   float minelem = 1.0F;
   vec3_t tempvec;

   /*
   ** find the smallest magnitude axially aligned vector
   */
   for(pos = 0, i = 0; i < 3; i++)
   {
      if(fabs(src[i]) < minelem)
      {
         pos = i;
         minelem = fabs(src[i]);
      }
   }
   tempvec[0] = tempvec[1] = tempvec[2] = 0.0F;
   tempvec[pos] = 1.0F;

   /*
   ** project the point onto the plane defined by src
   */
   ProjectPointOnPlane(dst, tempvec, src);

   /*
   ** normalize the result
   */
   VectorNormalize(dst);
}

/*
================
R_ConcatRotations
================
*/
void R_ConcatRotations(float in1[3][3], float in2[3][3], float out[3][3])
{
   out[0][0] = in1[0][0] * in2[0][0] + in1[0][1] * in2[1][0] + in1[0][2] * in2[2][0];
   out[0][1] = in1[0][0] * in2[0][1] + in1[0][1] * in2[1][1] + in1[0][2] * in2[2][1];
   out[0][2] = in1[0][0] * in2[0][2] + in1[0][1] * in2[1][2] + in1[0][2] * in2[2][2];
   out[1][0] = in1[1][0] * in2[0][0] + in1[1][1] * in2[1][0] + in1[1][2] * in2[2][0];
   out[1][1] = in1[1][0] * in2[0][1] + in1[1][1] * in2[1][1] + in1[1][2] * in2[2][1];
   out[1][2] = in1[1][0] * in2[0][2] + in1[1][1] * in2[1][2] + in1[1][2] * in2[2][2];
   out[2][0] = in1[2][0] * in2[0][0] + in1[2][1] * in2[1][0] + in1[2][2] * in2[2][0];
   out[2][1] = in1[2][0] * in2[0][1] + in1[2][1] * in2[1][1] + in1[2][2] * in2[2][1];
   out[2][2] = in1[2][0] * in2[0][2] + in1[2][1] * in2[1][2] + in1[2][2] * in2[2][2];
}

/*
================
R_ConcatTransforms
================
*/
void R_ConcatTransforms (float in1[3][4], float in2[3][4], float out[3][4])
{
   out[0][0] = in1[0][0] * in2[0][0] + in1[0][1] * in2[1][0] + in1[0][2] * in2[2][0];
   out[0][1] = in1[0][0] * in2[0][1] + in1[0][1] * in2[1][1] + in1[0][2] * in2[2][1];
   out[0][2] = in1[0][0] * in2[0][2] + in1[0][1] * in2[1][2] + in1[0][2] * in2[2][2];
   out[0][3] = in1[0][0] * in2[0][3] + in1[0][1] * in2[1][3] + in1[0][2] * in2[2][3] + in1[0][3];
   out[1][0] = in1[1][0] * in2[0][0] + in1[1][1] * in2[1][0] + in1[1][2] * in2[2][0];
   out[1][1] = in1[1][0] * in2[0][1] + in1[1][1] * in2[1][1] + in1[1][2] * in2[2][1];
   out[1][2] = in1[1][0] * in2[0][2] + in1[1][1] * in2[1][2] + in1[1][2] * in2[2][2];
   out[1][3] = in1[1][0] * in2[0][3] + in1[1][1] * in2[1][3] + in1[1][2] * in2[2][3] + in1[1][3];
   out[2][0] = in1[2][0] * in2[0][0] + in1[2][1] * in2[1][0] + in1[2][2] * in2[2][0];
   out[2][1] = in1[2][0] * in2[0][1] + in1[2][1] * in2[1][1] + in1[2][2] * in2[2][1];
   out[2][2] = in1[2][0] * in2[0][2] + in1[2][1] * in2[1][2] + in1[2][2] * in2[2][2];
   out[2][3] = in1[2][0] * in2[0][3] + in1[2][1] * in2[1][3] + in1[2][2] * in2[2][3] + in1[2][3];
}

//============================================================================

float Q_fabs (float f)
{
#if 0
   if(f >= 0)
      return f;
   return -f;
#else
   int tmp = *(int *)&f;
   tmp &= 0x7FFFFFFF;
   return *(float *)&tmp;
#endif
}

#if defined(_M_IX86) && !defined(C_ONLY)
#pragma warning (disable:4035)
__declspec( naked ) long Q_ftol( float f )
{
   static int tmp;
   __asm fld dword ptr[esp + 4]
   __asm fistp tmp
   __asm mov eax, tmp
   __asm ret
}
#pragma warning (default:4035)
#endif

/*
===============
LerpAngle

===============
*/
float LerpAngle(float a2, float a1, float frac)
{
   if(a1 - a2 > 180)
      a1 -= 360;
   if(a1 - a2 < -180)
      a1 += 360;
   return a2 + frac * (a1 - a2);
}

float	anglemod(float a)
{
#if 0
   if(a >= 0)
      a -= 360 * (int)(a / 360);
   else
      a += 360 * (1 + (int)(-a / 360));
#endif
   a = (360.0 / 65536) * ((int)(a*(65536 / 360.0)) & 65535);
   return a;
}

float angledist(float ang)
{
   float a;

   a = anglemod(ang);
   if(a > 180)
   {
      a -= 360;
   }

   return a;
}

// this is the slow, general version
int BoxOnPlaneSide2 (vec3_t emins, vec3_t emaxs, struct cplane_s *p)
{
   int      i;
   float    dist1, dist2;
   int      sides;
   vec3_t   corners[2];

   for(i = 0; i < 3; i++)
   {
      if(p->normal[i] < 0)
      {
         corners[0][i] = emins[i];
         corners[1][i] = emaxs[i];
      }
      else
      {
         corners[1][i] = emins[i];
         corners[0][i] = emaxs[i];
      }
   }
   dist1 = DotProduct(p->normal, corners[0]) - p->dist;
   dist2 = DotProduct(p->normal, corners[1]) - p->dist;
   sides = 0;
   if(dist1 >= 0)
      sides = 1;
   if(dist2 < 0)
      sides |= 2;

   return sides;
}

/*
==================
BoxOnPlaneSide

Returns 1, 2, or 1 + 2
==================
*/
#if !id386 || defined(__linux__)
int BoxOnPlaneSide (vec3_t emins, vec3_t emaxs, struct cplane_s *p)
{
	float	dist1, dist2;
	int		sides;

// fast axial cases
	if (p->type < 3)
	{
		if (p->dist <= emins[p->type])
			return 1;
		if (p->dist >= emaxs[p->type])
			return 2;
		return 3;
	}
	
// general case
	switch (p->signbits)
	{
	case 0:
dist1 = p->normal[0]*emaxs[0] + p->normal[1]*emaxs[1] + p->normal[2]*emaxs[2];
dist2 = p->normal[0]*emins[0] + p->normal[1]*emins[1] + p->normal[2]*emins[2];
		break;
	case 1:
dist1 = p->normal[0]*emins[0] + p->normal[1]*emaxs[1] + p->normal[2]*emaxs[2];
dist2 = p->normal[0]*emaxs[0] + p->normal[1]*emins[1] + p->normal[2]*emins[2];
		break;
	case 2:
dist1 = p->normal[0]*emaxs[0] + p->normal[1]*emins[1] + p->normal[2]*emaxs[2];
dist2 = p->normal[0]*emins[0] + p->normal[1]*emaxs[1] + p->normal[2]*emins[2];
		break;
	case 3:
dist1 = p->normal[0]*emins[0] + p->normal[1]*emins[1] + p->normal[2]*emaxs[2];
dist2 = p->normal[0]*emaxs[0] + p->normal[1]*emaxs[1] + p->normal[2]*emins[2];
		break;
	case 4:
dist1 = p->normal[0]*emaxs[0] + p->normal[1]*emaxs[1] + p->normal[2]*emins[2];
dist2 = p->normal[0]*emins[0] + p->normal[1]*emins[1] + p->normal[2]*emaxs[2];
		break;
	case 5:
dist1 = p->normal[0]*emins[0] + p->normal[1]*emaxs[1] + p->normal[2]*emins[2];
dist2 = p->normal[0]*emaxs[0] + p->normal[1]*emins[1] + p->normal[2]*emaxs[2];
		break;
	case 6:
dist1 = p->normal[0]*emaxs[0] + p->normal[1]*emins[1] + p->normal[2]*emins[2];
dist2 = p->normal[0]*emins[0] + p->normal[1]*emaxs[1] + p->normal[2]*emaxs[2];
		break;
	case 7:
dist1 = p->normal[0]*emins[0] + p->normal[1]*emins[1] + p->normal[2]*emins[2];
dist2 = p->normal[0]*emaxs[0] + p->normal[1]*emaxs[1] + p->normal[2]*emaxs[2];
		break;
	default:
		dist1 = dist2 = 0;		// shut up compiler
#ifdef SIN
		// assert( 1 ) doesn't do jack shit
		assert( 0 );
#else
		assert( 1 );
#endif
		break;
	}

	sides = 0;
	if (dist1 >= p->dist)
		sides = 1;
	if (dist2 < p->dist)
		sides |= 2;

	assert( sides != 0 );

	return sides;
}
#else
#pragma warning( disable: 4035 )

__declspec( naked ) int BoxOnPlaneSide (vec3_t emins, vec3_t emaxs, struct cplane_s *p)
{
   static int bops_initialized;
   static int Ljmptab[8];

   __asm {
		push ebx
			
		cmp bops_initialized, 1
		je  initialized
		mov bops_initialized, 1
		
		mov Ljmptab[0*4], offset Lcase0
		mov Ljmptab[1*4], offset Lcase1
		mov Ljmptab[2*4], offset Lcase2
		mov Ljmptab[3*4], offset Lcase3
		mov Ljmptab[4*4], offset Lcase4
		mov Ljmptab[5*4], offset Lcase5
		mov Ljmptab[6*4], offset Lcase6
		mov Ljmptab[7*4], offset Lcase7
			
initialized:

		mov edx,ds:dword ptr[4+12+esp]
		mov ecx,ds:dword ptr[4+4+esp]
		xor eax,eax
		mov ebx,ds:dword ptr[4+8+esp]
		mov al,ds:byte ptr[17+edx]
		cmp al,8
		jge Lerror
		fld ds:dword ptr[0+edx]
		fld st(0)
		jmp dword ptr[Ljmptab+eax*4]
Lcase0:
		fmul ds:dword ptr[ebx]
		fld ds:dword ptr[0+4+edx]
		fxch st(2)
		fmul ds:dword ptr[ecx]
		fxch st(2)
		fld st(0)
		fmul ds:dword ptr[4+ebx]
		fld ds:dword ptr[0+8+edx]
		fxch st(2)
		fmul ds:dword ptr[4+ecx]
		fxch st(2)
		fld st(0)
		fmul ds:dword ptr[8+ebx]
		fxch st(5)
		faddp st(3),st(0)
		fmul ds:dword ptr[8+ecx]
		fxch st(1)
		faddp st(3),st(0)
		fxch st(3)
		faddp st(2),st(0)
		jmp LSetSides
Lcase1:
		fmul ds:dword ptr[ecx]
		fld ds:dword ptr[0+4+edx]
		fxch st(2)
		fmul ds:dword ptr[ebx]
		fxch st(2)
		fld st(0)
		fmul ds:dword ptr[4+ebx]
		fld ds:dword ptr[0+8+edx]
		fxch st(2)
		fmul ds:dword ptr[4+ecx]
		fxch st(2)
		fld st(0)
		fmul ds:dword ptr[8+ebx]
		fxch st(5)
		faddp st(3),st(0)
		fmul ds:dword ptr[8+ecx]
		fxch st(1)
		faddp st(3),st(0)
		fxch st(3)
		faddp st(2),st(0)
		jmp LSetSides
Lcase2:
		fmul ds:dword ptr[ebx]
		fld ds:dword ptr[0+4+edx]
		fxch st(2)
		fmul ds:dword ptr[ecx]
		fxch st(2)
		fld st(0)
		fmul ds:dword ptr[4+ecx]
		fld ds:dword ptr[0+8+edx]
		fxch st(2)
		fmul ds:dword ptr[4+ebx]
		fxch st(2)
		fld st(0)
		fmul ds:dword ptr[8+ebx]
		fxch st(5)
		faddp st(3),st(0)
		fmul ds:dword ptr[8+ecx]
		fxch st(1)
		faddp st(3),st(0)
		fxch st(3)
		faddp st(2),st(0)
		jmp LSetSides
Lcase3:
		fmul ds:dword ptr[ecx]
		fld ds:dword ptr[0+4+edx]
		fxch st(2)
		fmul ds:dword ptr[ebx]
		fxch st(2)
		fld st(0)
		fmul ds:dword ptr[4+ecx]
		fld ds:dword ptr[0+8+edx]
		fxch st(2)
		fmul ds:dword ptr[4+ebx]
		fxch st(2)
		fld st(0)
		fmul ds:dword ptr[8+ebx]
		fxch st(5)
		faddp st(3),st(0)
		fmul ds:dword ptr[8+ecx]
		fxch st(1)
		faddp st(3),st(0)
		fxch st(3)
		faddp st(2),st(0)
		jmp LSetSides
Lcase4:
		fmul ds:dword ptr[ebx]
		fld ds:dword ptr[0+4+edx]
		fxch st(2)
		fmul ds:dword ptr[ecx]
		fxch st(2)
		fld st(0)
		fmul ds:dword ptr[4+ebx]
		fld ds:dword ptr[0+8+edx]
		fxch st(2)
		fmul ds:dword ptr[4+ecx]
		fxch st(2)
		fld st(0)
		fmul ds:dword ptr[8+ecx]
		fxch st(5)
		faddp st(3),st(0)
		fmul ds:dword ptr[8+ebx]
		fxch st(1)
		faddp st(3),st(0)
		fxch st(3)
		faddp st(2),st(0)
		jmp LSetSides
Lcase5:
		fmul ds:dword ptr[ecx]
		fld ds:dword ptr[0+4+edx]
		fxch st(2)
		fmul ds:dword ptr[ebx]
		fxch st(2)
		fld st(0)
		fmul ds:dword ptr[4+ebx]
		fld ds:dword ptr[0+8+edx]
		fxch st(2)
		fmul ds:dword ptr[4+ecx]
		fxch st(2)
		fld st(0)
		fmul ds:dword ptr[8+ecx]
		fxch st(5)
		faddp st(3),st(0)
		fmul ds:dword ptr[8+ebx]
		fxch st(1)
		faddp st(3),st(0)
		fxch st(3)
		faddp st(2),st(0)
		jmp LSetSides
Lcase6:
		fmul ds:dword ptr[ebx]
		fld ds:dword ptr[0+4+edx]
		fxch st(2)
		fmul ds:dword ptr[ecx]
		fxch st(2)
		fld st(0)
		fmul ds:dword ptr[4+ecx]
		fld ds:dword ptr[0+8+edx]
		fxch st(2)
		fmul ds:dword ptr[4+ebx]
		fxch st(2)
		fld st(0)
		fmul ds:dword ptr[8+ecx]
		fxch st(5)
		faddp st(3),st(0)
		fmul ds:dword ptr[8+ebx]
		fxch st(1)
		faddp st(3),st(0)
		fxch st(3)
		faddp st(2),st(0)
		jmp LSetSides
Lcase7:
		fmul ds:dword ptr[ecx]
		fld ds:dword ptr[0+4+edx]
		fxch st(2)
		fmul ds:dword ptr[ebx]
		fxch st(2)
		fld st(0)
		fmul ds:dword ptr[4+ecx]
		fld ds:dword ptr[0+8+edx]
		fxch st(2)
		fmul ds:dword ptr[4+ebx]
		fxch st(2)
		fld st(0)
		fmul ds:dword ptr[8+ecx]
		fxch st(5)
		faddp st(3),st(0)
		fmul ds:dword ptr[8+ebx]
		fxch st(1)
		faddp st(3),st(0)
		fxch st(3)
		faddp st(2),st(0)
LSetSides:
		faddp st(2),st(0)
		fcomp ds:dword ptr[12+edx]
		xor ecx,ecx
		fnstsw ax
		fcomp ds:dword ptr[12+edx]
		and ah,1
		xor ah,1
		add cl,ah
		fnstsw ax
		and ah,1
		add ah,ah
		add cl,ah
		pop ebx
		mov eax,ecx
		ret
Lerror:
		int 3
	}
}
#pragma warning( default: 4035 )
#endif

void ClearBounds (vec3_t mins, vec3_t maxs)
{
   mins[0] = mins[1] = mins[2] = 99999;
   maxs[0] = maxs[1] = maxs[2] = -99999;
}

void AddPointToBounds (vec3_t v, vec3_t mins, vec3_t maxs)
{
   int   i;
   vec_t val;

   for(i = 0; i < 3; i++)
   {
      val = v[i];
      if(val < mins[i])
         mins[i] = val;
      if(val > maxs[i])
         maxs[i] = val;
   }
}

int VectorCompare (vec3_t v1, vec3_t v2)
{
   if(v1[0] != v2[0] || v1[1] != v2[1] || v1[2] != v2[2])
      return 0;

   return 1;
}

vec_t VectorNormalize(vec3_t v)
{
   float	length, ilength;

   length = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
   length = sqrt(length); // FIXME

   if(length)
   {
      ilength = 1 / length;
      v[0] *= ilength;
      v[1] *= ilength;
      v[2] *= ilength;
   }

   return length;
}

vec_t VectorNormalize2 (vec3_t v, vec3_t out)
{
   float length, ilength;

   length = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
   length = sqrt(length); // FIXME

   if(length)
   {
      ilength = 1 / length;
      out[0] = v[0] * ilength;
      out[1] = v[1] * ilength;
      out[2] = v[2] * ilength;
   }

   return length;
}

void VectorMA(vec3_t veca, float scale, vec3_t vecb, vec3_t vecc)
{
   vecc[0] = veca[0] + scale*vecb[0];
   vecc[1] = veca[1] + scale*vecb[1];
   vecc[2] = veca[2] + scale*vecb[2];
}

vec_t _DotProduct(vec3_t v1, vec3_t v2)
{
   return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}

void _VectorSubtract(vec3_t veca, vec3_t vecb, vec3_t out)
{
   out[0] = veca[0] - vecb[0];
   out[1] = veca[1] - vecb[1];
   out[2] = veca[2] - vecb[2];
}

void _VectorAdd(vec3_t veca, vec3_t vecb, vec3_t out)
{
   out[0] = veca[0] + vecb[0];
   out[1] = veca[1] + vecb[1];
   out[2] = veca[2] + vecb[2];
}

void _VectorCopy(vec3_t in, vec3_t out)
{
   out[0] = in[0];
   out[1] = in[1];
   out[2] = in[2];
}

void CrossProduct(vec3_t v1, vec3_t v2, vec3_t cross)
{
   cross[0] = v1[1] * v2[2] - v1[2] * v2[1];
   cross[1] = v1[2] * v2[0] - v1[0] * v2[2];
   cross[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

double sqrt(double x);

vec_t VectorLength(vec3_t v)
{
   int   i;
   float length;

   length = 0;
   for(i = 0; i < 3; i++)
      length += v[i] * v[i];
   length = sqrt(length);		// FIXME

   return length;
}

void VectorInverse(vec3_t v)
{
   v[0] = -v[0];
   v[1] = -v[1];
   v[2] = -v[2];
}

void VectorScale(vec3_t in, vec_t scale, vec3_t out)
{
   out[0] = in[0] * scale;
   out[1] = in[1] * scale;
   out[2] = in[2] * scale;
}

int Q_log2(int val)
{
   int answer = 0;
   while(val >>= 1)
      answer++;
   return answer;
}

/*
=================
CalculateRotatedBounds
=================
*/         
void CalculateRotatedBounds(vec3_t angles, vec3_t mins, vec3_t maxs)
{
   int i;
   vec3_t forward, right, up;
   vec3_t rotmins, rotmaxs;
   float trans[3][3];

   AngleVectors(angles, forward, right, up);
   for(i = 0; i < 3; i++)
   {
      trans[i][0] = forward[i];
      trans[i][1] = -right[i];
      trans[i][2] = up[i];
   }
   ClearBounds(rotmins, rotmaxs);
   for(i = 0; i < 8; i++)
   {
      vec3_t   tmp, rottemp;

      if(i & 1)
         tmp[0] = mins[0];
      else
         tmp[0] = maxs[0];

      if(i & 2)
         tmp[1] = mins[1];
      else
         tmp[1] = maxs[1];

      if(i & 4)
         tmp[2] = mins[2];
      else
         tmp[2] = maxs[2];

      MatrixTransformVector(tmp, trans, rottemp);
      AddPointToBounds(rottemp, rotmins, rotmaxs);
   }
   VectorCopy(rotmins, mins);
   VectorCopy(rotmaxs, maxs);
}

/*
=================
CalculateRotatedBounds2
=================
*/         
void CalculateRotatedBounds2(float trans[3][3], vec3_t mins, vec3_t maxs)
{
   int i;
   vec3_t rotmins, rotmaxs;

   ClearBounds(rotmins, rotmaxs);
   for(i = 0; i < 8; i++)
   {
      vec3_t   tmp, rottemp;

      if(i & 1)
         tmp[0] = mins[0];
      else
         tmp[0] = maxs[0];

      if(i & 2)
         tmp[1] = mins[1];
      else
         tmp[1] = maxs[1];

      if(i & 4)
         tmp[2] = mins[2];
      else
         tmp[2] = maxs[2];

      MatrixTransformVector(tmp, trans, rottemp);
      AddPointToBounds(rottemp, rotmins, rotmaxs);
   }
   VectorCopy(rotmins, mins);
   VectorCopy(rotmaxs, maxs);
}

/*
=================
OriginFromTriangle
=================
*/
//  coordinate system at center 
void OriginFromTriangle(const float tri[3][3], vec3_t pos)
{
   int i;

   VectorClear(pos);
   for(i = 0; i < 3; i++)
      pos[i] = (tri[0][i] + tri[1][i] + tri[2][i]) / 3;
}

/*
=================
TransformFromTriangle
=================
*/         
//         0
//        /\
//       /  \
//    0 /    \  2
//     /      \
//    /        \
//   /----------\
//  1      1     2
//
// ((type>>2)&0x3)
// 0 -  edge 0 is first edge
// 1 -  edge 1 is first edge
// 2 -  edge 2 is first edge
//
// (type>>4)
// 0 - forward is perpendicular, up is first edge, right is on the same plane as the triangle
// 1 - forward is perpendicular, up is first edge, right is on the same plane as the triangle(inverted)
// 2 - forward is first edge, up is on the same plane as the triangle, up is perpendicular
// 3 - forward is first edge, up is on the same plane as the triangle(inverted), up is perpendicular
// 4 - forward is perpendicular to first edge, up is perpendicular, right is first edge
// 5 - forward is perpendicular to first edge, up is perpendicular, right is first edge(inverted)
// 6 - forward is perpendicular, up is first edge, right is on the same plane as the triangle
// 7 - forward is perpendicular, up is first edge, right is on the same plane as the triangle(inverted)
//
void TransformFromTriangle(const float tri[3][3], float trans[3][3], vec3_t pos)
{
   vec3_t edge[2];
   vec3_t cross;
   vec3_t other;
   vec3_t forward, right, up;

   OriginFromTriangle(tri, pos);

   VectorSubtract(tri[2], tri[0], edge[0]);
   VectorSubtract(tri[1], tri[0], edge[1]);

   VectorNormalize(edge[0]);
   VectorNormalize(edge[1]);
   CrossProduct(edge[1], edge[0], cross);
   CrossProduct(edge[1], cross, other);

   VectorCopy(cross, forward);
   VectorCopy(other, right);
   VectorNegate(edge[1], up);

   VectorCopy(forward, trans[0]);
   VectorCopy(right, trans[1]);
   VectorCopy(up, trans[2]);

/*
   switch( (type>>2)&0x3 )
      {
      case 0:
         VectorSubtract( tri[2], tri[0], edge[0] );
         VectorSubtract( tri[1], tri[0], edge[1] );
         break;
      case 1:
         VectorSubtract( tri[0], tri[1], edge[0] );
         VectorSubtract( tri[2], tri[1], edge[1] );
         break;
      case 2:
         VectorSubtract( tri[1], tri[2], edge[0] );
         VectorSubtract( tri[0], tri[2], edge[1] );
         break;
      default:
         VectorSubtract( tri[2], tri[0], edge[0] );
         VectorSubtract( tri[1], tri[0], edge[1] );
         break;
      }
   VectorNormalize( edge[0] );
   VectorNormalize( edge[1] );
   CrossProduct( edge[1], edge[0], cross );
   CrossProduct( edge[1], cross, other );
   switch( type>>4 )
      {
      case 0:
         VectorCopy( cross, forward );
         VectorCopy( other, right );
         VectorNegate( edge[1], up );
         break;
      case 1:
         VectorCopy( cross, forward );
         VectorCopy( other, right );
         VectorCopy( edge[1], up );
         break;
      case 2:
         VectorCopy( cross, forward );
         VectorCopy( other, right );
         VectorNegate( edge[0], up );
         break;
      case 3:
         VectorCopy( cross, forward );
         VectorCopy( other, right );
         VectorCopy( edge[0], up );
         break;
      case 4:
         VectorCopy( edge[0], forward );
         VectorNegate( other, up );
         VectorCopy( cross, right );
         break;
      case 5:
         VectorCopy( edge[0], forward );
         VectorCopy( other, up );
         VectorCopy( cross, right );
         break;
      case 6:
         VectorCopy( other, forward );
         VectorNegate( edge[0], right );
         VectorCopy( cross, up );
         break;
      case 7:
         VectorCopy( other, forward );
         VectorCopy( edge[0], right );
         VectorCopy( cross, up );
         break;
      case 8:
         VectorCopy( edge[0], forward );
         VectorNegate( other, right );
         VectorCopy( cross, up );
         break;
      case 9:
         VectorCopy( edge[0], forward );
         VectorCopy( other, right );
         VectorCopy( cross, up );
         break;
      default:
         VectorCopy( edge[0], forward );
         VectorNegate( other, right );
         VectorCopy( cross, up );
         break;
      }
   // fill in the matrix
   VectorCopy( forward, trans[0] );
   VectorCopy( right, trans[1] );
   VectorCopy( up, trans[2] );
*/
}

// 2015 code
void TransformFromTriangle_2015(const float tri[3][3], float trans[3][3], vec3_t pos)
{
   vec3_t edge[2];
   vec3_t cross;
   vec3_t other;
   vec3_t forward, right, up;
   
   OriginFromTriangle( tri, pos );
   
   VectorSubtract( tri[2], tri[0], edge[0] );
   VectorSubtract( tri[1], tri[0], edge[1] );
   
   VectorNormalize( edge[0] );
   VectorNormalize( edge[1] );
   CrossProduct( edge[1], edge[0], cross );
   VectorNormalize( cross ); // 
   CrossProduct( edge[1], cross, other );
   VectorNormalize( other ); // 
   
   VectorCopy( cross, forward );
   VectorCopy( other, right );
// VectorNegate( edge[1], up );
   VectorCopy( edge[1], up );
   
   VectorCopy( forward, trans[0] );
   VectorCopy( right, trans[1] );
   VectorCopy( up, trans[2] );
}

static char musicmoods[mood_totalnumber][16] =
{
   "none",
   "normal",
   "action",
   "suspense",
   "mystery",
   "success",
   "failure",
   "surprise",
   "special",
   "aux1",
   "aux2",
   "aux3",
   "aux4",
   "aux5",
   "aux6",
   "aux7"
};

/*
=================
MusicMood_NameToNum
=================
*/         
int MusicMood_NameToNum(const char * name)
{
   int i;

   if(!name)
      return -1;

   for(i = 0; i < mood_totalnumber; i++)
   {
      if(!strcmpi(name, musicmoods[i]))
      {
         return i;
      }
   }
   return -1;
}

/*
=================
MusicMood_NumToName
=================
*/         
const char *MusicMood_NumToName(int num)
{
   if((num < 0) || (num >= mood_totalnumber))
      return "";
   else
      return musicmoods[num];
}

/*
===============
SURFACE_DamageMultiplier
===============
*/
float SURFACE_DamageMultiplier(int flags)
{
   float mult;

   mult = 0;
   switch(flags)
   {
   case SURF_TYPE_FLESH:
   case SURF_TYPE_FABRIC:
   case SURF_TYPE_VEGETATION:
   case SURF_TYPE_WATER:
   case SURF_TYPE_PAPER:
      mult = 0;
      break;
   case SURF_TYPE_DIRT:
      mult = 0.2;
      break;
   case SURF_TYPE_WOOD:
      mult = 0.5;
      break;
   case SURF_TYPE_METAL:
   case SURF_TYPE_DUCT:
   case SURF_TYPE_GRILL:
      mult = 1;
      break;
   case SURF_TYPE_GRAVEL:
   case SURF_TYPE_STONE:
   case SURF_TYPE_CONCRETE:
      mult = 1.5;
      break;
   case SURF_TYPE_GLASS:
   case SURF_TYPE_MONITOR:
      mult = 3;
      break;
   default:
      break;
   }
   return mult;
}

//====================================================================================

/*
============
COM_SkipPath
============
*/
const char *COM_SkipPath(const char *pathname)
{
   const char	*last;

   last = pathname;
   while(*pathname)
   {
      if(*pathname == '/')
         last = pathname + 1;
      pathname++;
   }
   return last;
}

/*
============
COM_ParseHex
============
*/
int COM_ParseHex(const char *hex)
{
   const char    *str;
   int    num;

   num = 0;
   str = hex;

   while(*str)
   {
      num <<= 4;
      if(*str >= '0' && *str <= '9')
         num += *str - '0';
      else if(*str >= 'a' && *str <= 'f')
         num += 10 + *str - 'a';
      else if(*str >= 'A' && *str <= 'F')
         num += 10 + *str - 'A';
      else
         Com_Printf("Bad hex number: %s", hex);
      str++;
   }

   return num;
}

/*
============
COM_StripExtension
============
*/
void COM_StripExtension(const char *in, char *out)
{
   while(*in && *in != '.')
      *out++ = *in++;
   *out = 0;
}

/*
============
COM_FileExtension
============
*/
char *COM_FileExtension(const char *in)
{
   static char exten[8];
   int i;

   while(*in && *in != '.')
      in++;
   if(!*in)
      return "";
   in++;
   for(i = 0; i < 7 && *in; i++, in++)
      exten[i] = *in;
   exten[i] = 0;
   return exten;
}

/*
============
COM_FileBase
============
*/
void COM_FileBase(const char *in, char *out)
{
   const char *s;
   const char *s2;

   s = in + strlen(in) - 1;

   while(s != in && *s != '.')
      s--;

   for(s2 = s; s2 != in && *s2 != '/'; s2--)
      ;

   if(s - s2 < 2)
      out[0] = 0;
   else
   {
      s--;
      strncpy(out, s2 + 1, s - s2);
      out[s - s2] = 0;
   }
}

/*
============
COM_FilePath

Returns the path up to, but not including the last /
============
*/
void COM_FilePath(const char *in, char *out)
{
   const char *s;

   s = in + strlen(in) - 1;

   while(s != in && *s != '/')
      s--;

   strncpy(out, in, s - in);
   out[s - in] = 0;
}

/*
==================
COM_DefaultExtension
==================
*/
void COM_DefaultExtension(char *path, const char *extension)
{
   char    *src;

   //
   // if path doesn't have a .EXT, append extension
   // (extension should include the .)
   //
   src = path + strlen(path) - 1;

   while(*src != '/' && src != path)
   {
      if(*src == '.')
         return;                 // it has an extension
      src--;
   }

   strcat(path, extension);
}

/*
============================================================================

               BYTE ORDER FUNCTIONS

============================================================================
*/

qboolean bigendien;

// can't just use function pointers, or dll linkage can
// mess up when qcommon is included in multiple places
short (*_BigShort) (short l);
short (*_LittleShort) (short l);
int   (*_BigLong) (int l);
int   (*_LittleLong) (int l);
float (*_BigFloat) (float l);
float (*_LittleFloat) (float l);

unsigned short (*_BigUnsignedShort) (unsigned short l);
unsigned short (*_LittleUnsignedShort) (unsigned short l);

short BigShort(short l)     { return _BigShort(l);    }
short LittleShort(short l)  { return _LittleShort(l); }
int   BigLong (int l)       { return _BigLong(l);     }
int   LittleLong (int l)    { return _LittleLong(l);  }
float BigFloat (float l)    { return _BigFloat(l);    }
float LittleFloat (float l) { return _LittleFloat(l); }

unsigned short BigUnsignedShort(unsigned short l)    { return _BigUnsignedShort(l);    }
unsigned short LittleUnsignedShort(unsigned short l) { return _LittleUnsignedShort(l); }

short ShortSwap(short l)
{
   byte    b1, b2;

   b1 = l & 255;
   b2 = (l >> 8) & 255;

   return (b1 << 8) + b2;
}

short ShortNoSwap(short l)
{
   return l;
}

unsigned short UnsignedShortSwap(unsigned short l)
{
   byte    b1, b2;

   b1 = l & 255;
   b2 = (l >> 8) & 255;

   return (b1 << 8) + b2;
}

unsigned short UnsignedShortNoSwap (unsigned short l)
{
   return l;
}

int LongSwap(int l)
{
   byte    b1, b2, b3, b4;

   b1 = l & 255;
   b2 = (l >> 8) & 255;
   b3 = (l >> 16) & 255;
   b4 = (l >> 24) & 255;

   return ((int)b1 << 24) + ((int)b2 << 16) + ((int)b3 << 8) + b4;
}

int LongNoSwap(int l)
{
   return l;
}

float FloatSwap(float f)
{
   union
   {
      float	f;
      byte	b[4];
   } dat1, dat2;


   dat1.f = f;
   dat2.b[0] = dat1.b[3];
   dat2.b[1] = dat1.b[2];
   dat2.b[2] = dat1.b[1];
   dat2.b[3] = dat1.b[0];
   return dat2.f;
}

float FloatNoSwap(float f)
{
   return f;
}

/*
================
Swap_Init
================
*/
void Swap_Init(void)
{
   byte swaptest[2] = { 1,0 };

   // set the byte swapping variables in a portable manner	
   if(*(short *)swaptest == 1)
   {
      bigendien = false;
      _BigShort = ShortSwap;
      _LittleShort = ShortNoSwap;
      _BigLong = LongSwap;
      _LittleLong = LongNoSwap;
      _BigFloat = FloatSwap;
      _LittleFloat = FloatNoSwap;
      _BigUnsignedShort = UnsignedShortSwap;
      _LittleUnsignedShort = UnsignedShortNoSwap;
   }
   else
   {
      bigendien = true;
      _BigShort = ShortNoSwap;
      _LittleShort = ShortSwap;
      _BigLong = LongNoSwap;
      _LittleLong = LongSwap;
      _BigFloat = FloatNoSwap;
      _LittleFloat = FloatSwap;
      _BigUnsignedShort = UnsignedShortNoSwap;
      _LittleUnsignedShort = UnsignedShortSwap;
   }
}

/*
============
va

does a varargs printf into a temp buffer, so I don't need to have
varargs versions of all text functions.
DONE: make this buffer size safe someday
============
*/
const char *va(const char *format, ...)
{
   va_list     argptr;
   static char string[1024];

   va_start(argptr, format);
   vsnprintf(string, sizeof(string), format, argptr);
   va_end(argptr);

   return string;
}

char com_token[MAX_STRING_CHARS]; // SINEX_FIXME: overflowable buffer

#ifdef SIN
/*
==============
COM_GetToken

Parse a token out of a string
==============
*/
const char *COM_GetToken(const char **data_p, qboolean crossline)
{
   int		c;
   int		len;
   const char *data;

   data = *data_p;
   len = 0;
   com_token[0] = 0;

   if(!data)
   {
      *data_p = NULL;
      return "";
   }

   // skip whitespace
skipwhite:
   while((c = *data) <= ' ')
   {
      if(c == '\n' && !crossline)
      {
         *data_p = data;
         return "";
      }

      if(c == 0)
      {
         *data_p = NULL;
         return "";
      }
      data++;
   }

   // skip // comments
   if(c == '/' && data[1] == '/')
   {
      while(*data && *data != '\n')
         data++;
      goto skipwhite;
   }

   // skip /* comments
   if(c == '/' && data[1] == '*')
   {
      data++;
      while(*data)
      {
         if((*(data - 1) == '*') && (*data == '/'))
            break;
         data++;
      }
      while(*data && *data != '\n')
         data++;
      goto skipwhite;
   }

   // handle quoted strings specially
   if(c == '\"')
   {
      data++;
      while(1)
      {
         c = *data++;
         if(c == '\\' && *data == '\"')
         {
            if(len < MAX_STRING_CHARS)
            {
               com_token[len] = '\"';
               len++;
            }
            data++;
         }
         else if(c == '\"' || !c)
         {
            com_token[len] = 0;
            *data_p = data;
            return com_token;
         }
         else if(len < MAX_STRING_CHARS)
         {
#ifdef SIN
            if(c == '\\' && *data == 'n')
            {
               com_token[len] = '\n';
               data++;
            }
            else
            {
               com_token[len] = c;
            }
            len++;
#else
            com_token[len] = c;
            len++;
#endif
         }
      }
   }

   // parse a regular word
   do
   {
      if(len < MAX_STRING_CHARS)
      {
         com_token[len] = c;
         len++;
      }
      data++;
      c = *data;
   }
   while(c > 32);

   if(len == MAX_STRING_CHARS)
   {
      // Com_Printf ("Token exceeded %i chars, discarded.\n", MAX_STRING_CHARS);
      len = 0;
   }
   com_token[len] = 0;

   *data_p = data;
   return com_token;
}

/*
==============
SIN_GetToken

Parse a token out of a string
==============
*/
const char *SIN_GetToken(const char **data_p, qboolean crossline)
{
   int c;
   int len;
   const char *data;

   data = *data_p;
   len = 0;
   com_token[0] = 0;

   if(!data)
   {
      *data_p = NULL;
      return "";
   }

   // skip whitespace
skipwhite:
   while((c = *data) <= ' ')
   {
      if(c == '\n' && !crossline)
      {
         *data_p = data;
         return "";
      }

      if(c == 0)
      {
         *data_p = NULL;
         return "";
      }
      data++;
   }

   // skip // comments
   if(c == '/' && data[1] == '/')
   {
      while(*data && *data != '\n')
         data++;
      goto skipwhite;
   }

   // skip /* comments
   if(c == '/' && data[1] == '*')
   {
      data++;
      while(*data)
      {
         if((*(data - 1) == '*') && (*data == '/'))
            break;
         data++;
      }
      while(*data && *data != '\n')
         data++;
      goto skipwhite;
   }


   // handle quoted strings specially
   if(c == '\"')
   {
      data++;
      while(1)
      {
         c = *data++;
         if(c == '\\' && *data == '\"')
         {
            if(len < MAX_STRING_CHARS)
            {
               com_token[len] = '\"';
               len++;
            }
            data++;
         }
         else if(c == '\"' || !c)
         {
            com_token[len] = 0;
            *data_p = data;
            return com_token;
         }
         else if(len < MAX_STRING_CHARS)
         {
            com_token[len] = c;
            len++;
         }
      }
   }

   // parse a regular word
   do
   {
      if(len < MAX_STRING_CHARS)
      {
         com_token[len] = c;
         len++;
      }
      data++;
      c = *data;
   }
   while(c > 32);

   if(len == MAX_STRING_CHARS)
   {
      // Com_Printf ("Token exceeded %i chars, discarded.\n", MAX_STRING_CHARS);
      len = 0;
   }
   com_token[len] = 0;

   *data_p = data;
   return com_token;
}


const char *SIN_Parse(const char **data_p)
{
   return SIN_GetToken(data_p, true);
}

/*
==============
COM_Parse

Parse a token out of a string
==============
*/
const char *COM_Parse (const char **data_p)
{
   return COM_GetToken( data_p, true );
}
#endif

/*
===============
Com_PageInMemory

===============
*/
int paged_total;

void Com_PageInMemory(byte *buffer, int size)
{
   int i;

   for(i = size - 1; i > 0; i -= 4096)
      paged_total += buffer[i];
}

/*
============================================================================

               LIBRARY REPLACEMENT FUNCTIONS

============================================================================
*/

// FIXME: replace all Q_stricmp with Q_strcasecmp
int Q_stricmp(const char *s1, const char *s2)
{
#if defined(_MSC_VER)
   return _stricmp(s1, s2);
#else
   return strcasecmp(s1, s2);
#endif
}

int Q_strncasecmp(const char *s1, const char *s2, int n)
{
   int c1, c2;

   do
   {
      c1 = *s1++;
      c2 = *s2++;

      if(!n--)
         return 0; // strings are equal until end point

      if(c1 != c2)
      {
         if(c1 >= 'a' && c1 <= 'z')
            c1 -= ('a' - 'A');
         if(c2 >= 'a' && c2 <= 'z')
            c2 -= ('a' - 'A');
         if(c1 != c2)
            return -1; // strings not equal
      }
   }
   while(c1);

   return 0; // strings are equal
}

int Q_strcasecmp(const char *s1, const char *s2)
{
   return Q_strncasecmp(s1, s2, 99999);
}

void Com_sprintf(char *dest, int size, const char *fmt, ...)
{
   char    bigbuffer[0x10000];
   int     len;
   va_list argptr;

   va_start(argptr, fmt);
   len = vsnprintf(bigbuffer, sizeof(bigbuffer), fmt, argptr);
   va_end(argptr);
   if(len >= size)
      Com_Printf("Com_sprintf: overflow of %i in %i\n", len, size);
   strncpy(dest, bigbuffer, size);
   dest[size - 1] = '\0';
}

/*
=====================================================================

BSD STRING UTILITIES - haleyjd 20170610

=====================================================================
*/
/*
 * Copyright (c) 1998 Todd C. Miller <Todd.Miller@courtesan.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Copy src to string dst of size siz.  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz == 0).
 * Returns strlen(src); if retval >= siz, truncation occurred.
 */
size_t Q_strlcpy(char *dst, const char *src, size_t siz)
{
   register char *d = dst;
   register const char *s = src;
   register size_t n = siz;

   /* Copy as many bytes as will fit */
   if(n != 0 && --n != 0)
   {
      do
      {
         if((*d++ = *s++) == 0)
            break;
      }
      while(--n != 0);
   }

   /* Not enough room in dst, add NUL and traverse rest of src */
   if(n == 0)
   {
      if(siz != 0)
         *d = '\0'; /* NUL-terminate dst */
      while(*s++)
         ; // counter loop
   }

   return (s - src - 1); /* count does not include NUL */
}

/*
 * Appends src to string dst of size siz (unlike strncat, siz is the
 * full size of dst, not space left).  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz == 0).
 * Returns strlen(src); if retval >= siz, truncation occurred.
 */
size_t Q_strlcat(char *dst, const char *src, size_t siz)
{
   register char *d = dst;
   register const char *s = src;
   register size_t n = siz;
   size_t dlen;

   /* Find the end of dst and adjust bytes left but don't go past end */
   while(*d != '\0' && n-- != 0)
      d++;
   dlen = d - dst;
   n = siz - dlen;

   if(n == 0)
      return(dlen + strlen(s));
   while(*s != '\0')
   {
      if(n != 1)
      {
         *d++ = *s;
         n--;
      }
      s++;
   }
   *d = '\0';

   return (dlen + (s - src)); /* count does not include NUL */
}

/*
=====================================================================

  INFO STRINGS

=====================================================================
*/

/*
===============
Info_ValueForKey

Searches the string for the given
key and returns the associated value, or an empty string.
===============
*/
const char *Info_ValueForKey(const char *s, const char *key)
{
   char   pkey[512]; // SINEX_FIXME: overflowable buffer
   static char value[2][512];	// use two buffers so compares
                        // work without stomping on each other
   static int valueindex;
   char *o;

   valueindex ^= 1;
   if(*s == '\\')
      s++;
   while(1)
   {
      o = pkey;
      while(*s != '\\')
      {
         if(!*s)
            return "";
         *o++ = *s++;
      }
      *o = 0;
      s++;

      o = value[valueindex];

      while(*s != '\\' && *s)
      {
         if(!*s)
            return "";
         *o++ = *s++;
      }
      *o = 0;

      if(!strcmp(key, pkey))
         return value[valueindex];

      if(!*s)
         return "";
      s++;
   }
}

void Info_RemoveKey(char *s, const char *key)
{
   char *start;
   char  pkey[512];  // SINEX_FIXME: overflowable buffers
   char  value[512];
   char *o;

   if(strstr(key, "\\"))
   {
      // Com_Printf ("Can't use a key with a \\\n");
      return;
   }

   while(1)
   {
      start = s;
      if(*s == '\\')
         s++;
      o = pkey;
      while(*s != '\\')
      {
         if(!*s)
            return;
         *o++ = *s++;
      }
      *o = 0;
      s++;

      o = value;
      while(*s != '\\' && *s)
      {
         if(!*s)
            return;
         *o++ = *s++;
      }
      *o = 0;

      if(!strcmp(key, pkey))
      {
         strcpy(start, s); // remove this part
         return;
      }

      if(!*s)
         return;
   }

}

/*
==================
Info_Validate

Some characters are illegal in info strings because they
can mess up the server's parsing
==================
*/
qboolean Info_Validate(const char *s)
{
   if(strstr(s, "\""))
      return false;
   if(strstr(s, ";"))
      return false;
   return true;
}

void Info_SetValueForKey(char *s, const char *key, const char *value)
{
   char newi[MAX_INFO_STRING], *v; // SINEX_FIXME: overflowable buffer
   int  c;
   int  maxsize = MAX_INFO_STRING;

   if(strstr(key, "\\") || strstr(value, "\\"))
   {
      Com_Printf("Can't use keys or values with a \\\n");
      return;
   }

   if(strstr(key, ";"))
   {
      Com_Printf("Can't use keys or values with a semicolon\n");
      return;
   }

   if(strstr(key, "\"") || strstr(value, "\""))
   {
      Com_Printf("Can't use keys or values with a \"\n");
      return;
   }

   if(strlen(key) > MAX_INFO_KEY - 1 || strlen(value) > MAX_INFO_KEY - 1)
   {
      Com_Printf("Keys and values must be < 64 characters.\n");
      return;
   }
   Info_RemoveKey(s, key);
   if(!value || !strlen(value))
      return;

   Com_sprintf(newi, sizeof(newi), "\\%s\\%s", key, value);

   if(strlen(newi) + strlen(s) > maxsize)
   {
      Com_Printf("Info string length exceeded\n");
      return;
   }

   // only copy ascii values
   s += strlen(s);
   v = newi;
   while(*v)
   {
      c = *v++;
      c &= 127;		// strip high bits
      if(c >= 32 && c < 127)
         *s++ = c;
   }
   *s = 0;
}

//====================================================================


void MatrixTransformVector(vec3_t in, float mat[3][3], vec3_t out)
{
   out[0] = in[0] * mat[0][0] + in[1] * mat[1][0] + in[2] * mat[2][0];
   out[1] = in[0] * mat[0][1] + in[1] * mat[1][1] + in[2] * mat[2][1];
   out[2] = in[0] * mat[0][2] + in[1] * mat[1][2] + in[2] * mat[2][2];
}

void Matrix4TransformVector(vec3_t in, float mat[4][4], vec3_t out)
{
   out[0] = in[0] * mat[0][0] + in[1] * mat[1][0] + in[2] * mat[2][0] + mat[3][0];
   out[1] = in[0] * mat[0][1] + in[1] * mat[1][1] + in[2] * mat[2][1] + mat[3][1];
   out[2] = in[0] * mat[0][2] + in[1] * mat[1][2] + in[2] * mat[2][2] + mat[3][2];
}

void VectorsToEulerAngles(vec3_t forward, vec3_t right, vec3_t up, vec3_t ang)
{
   double theta;
   double cp;
   double sp;

   sp = forward[2];

   // cap off our sin value so that we don't get any NANs
   if(sp > 1.0)
   {
      sp = 1.0;
   }
   if(sp < -1.0)
   {
      sp = -1.0;
   }

   theta = -asin(sp);
   cp = cos(theta);

   if(cp > 8192 * FLT_EPSILON)
   {
      ang[0] = theta * 180 / M_PI;
      ang[1] = atan2(forward[1], forward[0]) * 180 / M_PI;
      ang[2] = atan2(-right[2], up[2]) * 180 / M_PI;
   }
   else
   {
      ang[0] = theta * 180 / M_PI;
      ang[1] = -atan2(right[0], right[1]) * 180 / M_PI;
      ang[2] = 0;
   }
}

void MatrixToEulerAngles(float mat[3][3], vec3_t ang)
{
   double theta;
   double cp;
   double sp;

   sp = mat[0][2];

   // cap off our sin value so that we don't get any NANs
   if(sp > 1.0)
   {
      sp = 1.0;
   }
   if(sp < -1.0)
   {
      sp = -1.0;
   }

   theta = -asin(sp);
   cp = cos(theta);

   if(cp > 8192 * FLT_EPSILON)
   {
      ang[0] = theta * 180 / M_PI;
      ang[1] = atan2(mat[0][1], mat[0][0]) * 180 / M_PI;
      ang[2] = atan2(mat[1][2], mat[2][2]) * 180 / M_PI;
   }
   else
   {
      ang[0] = theta * 180 / M_PI;
      ang[1] = -atan2(mat[1][0], mat[1][1]) * 180 / M_PI;
      ang[2] = 0;
   }
}

void TransposeMatrix(float in[3][3], float out[3][3])
{
   out[0][0] = in[0][0];
   out[0][1] = in[1][0];
   out[0][2] = in[2][0];
   out[1][0] = in[0][1];
   out[1][1] = in[1][1];
   out[1][2] = in[2][1];
   out[2][0] = in[0][2];
   out[2][1] = in[1][2];
   out[2][2] = in[2][2];
}

void OrthoNormalize(float mat[3][3])
{
   VectorNormalize(mat[0]);
   CrossProduct(mat[0], mat[1], mat[2]);
   VectorNormalize(mat[2]);
   CrossProduct(mat[2], mat[0], mat[1]);
   VectorNormalize(mat[1]);
}

float NormalizeQuat(float q[4])
{
   float	length, ilength;

   length = q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3];
   length = sqrt(length);

   if(length)
   {
      ilength = 1 / length;
      q[0] *= ilength;
      q[1] *= ilength;
      q[2] *= ilength;
      q[3] *= ilength;
   }

   return length;
}

void MatToQuat(float srcMatrix[3][3], float destQuat[4])
{
   double  	trace, s;
   int     	i, j, k;
   static int 	next[3] = { Y, Z, X };

   trace = srcMatrix[X][X] + srcMatrix[Y][Y] + srcMatrix[Z][Z];

   if(trace > 0.0)
   {
      s = sqrt(trace + 1.0);
      destQuat[W] = s * 0.5;
      s = 0.5 / s;

      destQuat[X] = (srcMatrix[Z][Y] - srcMatrix[Y][Z]) * s;
      destQuat[Y] = (srcMatrix[X][Z] - srcMatrix[Z][X]) * s;
      destQuat[Z] = (srcMatrix[Y][X] - srcMatrix[X][Y]) * s;
   }
   else
   {
      i = X;
      if(srcMatrix[Y][Y] > srcMatrix[X][X])
         i = Y;
      if(srcMatrix[Z][Z] > srcMatrix[i][i])
         i = Z;
      j = next[i];
      k = next[j];

      s = sqrt((srcMatrix[i][i] - (srcMatrix[j][j] + srcMatrix[k][k])) + 1.0);
      destQuat[i] = s * 0.5;

      s = 0.5 / s;

      destQuat[W] = (srcMatrix[k][j] - srcMatrix[j][k]) * s;
      destQuat[j] = (srcMatrix[j][i] + srcMatrix[i][j]) * s;
      destQuat[k] = (srcMatrix[k][i] + srcMatrix[i][k]) * s;
   }
}

void AnglesToMat(float ang[3], float mat[3][3])
{
   /*
   AngleVectors(ang, mat[0], mat[1], mat[2]);
   VectorNegate(mat[1], mat[1]);
   */
   float *m = *mat;
   AngleVectors(ang, m, m + 3, m + 6);
   VectorNegate((m + 3), (m + 3));
}

void RotateAxis(float axis[3], float angle, float q[4])
{
   float sin_a;
   float inv_sin_a;
   float cos_a;
   float r;

   r = angle * M_PI / 360;

   sin_a = sin(r);
   if(fabs(sin_a) > 0.00000001)
   {
      inv_sin_a = 1 / sin_a;
   }
   else
   {
      inv_sin_a = 0;
   }
   cos_a = cos(r);

   q[X] = axis[0] * inv_sin_a;
   q[Y] = axis[1] * inv_sin_a;
   q[Z] = axis[2] * inv_sin_a;
   q[W] = cos_a;
}

void MultQuat(float q1[4], float q2[4], float out[4])
{
   out[0] = q1[X] * q2[X] - q1[Y] * q2[Y] - q1[Z] * q2[Z] - q1[W] * q2[W];
   out[1] = q1[X] * q2[Y] + q1[Y] * q2[X] + q1[Z] * q2[W] - q1[W] * q2[Z];
   out[2] = q1[X] * q2[Z] - q1[Y] * q2[W] + q1[Z] * q2[X] + q1[W] * q2[Y];
   out[3] = q1[X] * q2[W] + q1[Y] * q2[Z] - q1[Z] * q2[Y] + q1[W] * q2[X];
}

void QuatToMat(float q[4], float m[3][3])
{
   float wx, wy, wz;
   float xx, yy, yz;
   float xy, xz, zz;
   float x2, y2, z2;

   x2 = q[X] + q[X];
   y2 = q[Y] + q[Y];
   z2 = q[Z] + q[Z];

   xx = q[X] * x2;
   xy = q[X] * y2;
   xz = q[X] * z2;

   yy = q[Y] * y2;
   yz = q[Y] * z2;
   zz = q[Z] * z2;

   wx = q[W] * x2;
   wy = q[W] * y2;
   wz = q[W] * z2;

   m[0][0] = 1.0 - (yy + zz);
   m[0][1] = xy - wz;
   m[0][2] = xz + wy;

   m[1][0] = xy + wz;
   m[1][1] = 1.0 - (xx + zz);
   m[1][2] = yz - wx;

   m[2][0] = xz - wy;
   m[2][1] = yz + wx;
   m[2][2] = 1.0 - (xx + yy);
}

#define DELTA 1e-6

void SlerpQuaternion(float from[4], float to[4], float t, float res[4])
{
   float    to1[4];
   double   omega, cosom, sinom, scale0, scale1;

   cosom = from[X] * to[X] + from[Y] * to[Y] + from[Z] * to[Z] + from[W] * to[W];
   if(cosom < 0.0)
   {
      cosom = -cosom;
      to1[X] = -to[X];
      to1[Y] = -to[Y];
      to1[Z] = -to[Z];
      to1[W] = -to[W];
   }
   else if(
      (from[X] == to[X]) &&
         (from[Y] == to[Y]) &&
         (from[Z] == to[Z]) &&
         (from[W] == to[W]))
   {
      // equal case, early exit
      res[X] = to[X];
      res[Y] = to[Y];
      res[Z] = to[Z];
      res[W] = to[W];
      return;
   }
   else
   {
      to1[X] = to[X];
      to1[Y] = to[Y];
      to1[Z] = to[Z];
      to1[W] = to[W];
   }

   if((1.0 - cosom) > DELTA)
   {
      omega = acos(cosom);
      sinom = sin(omega);
      scale0 = sin((1.0 - t) * omega) / sinom;
      scale1 = sin(t * omega) / sinom;
   }
   else
   {
      scale0 = 1.0 - t;
      scale1 = t;
   }

   res[X] = scale0 * from[X] + scale1 * to1[X];
   res[Y] = scale0 * from[Y] + scale1 * to1[Y];
   res[Z] = scale0 * from[Z] + scale1 * to1[Z];
   res[W] = scale0 * from[W] + scale1 * to1[W];
}

#if 0
void EulerToQuat(float ang[3], float q[4])
{
   float cr, cp, cy;
   float sr, sp, sy;
   float cpcy, spsy;
   float spcy, cpsy;
   float r;

   r = M_PI / 360;

   // calculate trig identities

   cr = cos(-ang[ROLL] * r);
   cp = cos(-ang[PITCH] * r);
   cy = cos(-ang[YAW] * r);

   sr = sin(-ang[ROLL] * r);
   sp = sin(-ang[PITCH] * r);
   sy = sin(-ang[YAW] * r);

   cpcy = cp * cy;
   spsy = sp * sy;
   spcy = sp * cy;
   cpsy = cp * sy;

   q[W] = cr * cpcy + sr * spsy;
   q[X] = sr * cpcy - cr * spsy;
   q[Y] = cr * spcy + sr * cpsy;
   q[Z] = cr * cpsy - sr * spcy;
}
#endif

#if 0
float x_axis[ 3 ] = { 1, 0, 0 };
float y_axis[ 3 ] = { 0, 1, 0 };
float z_axis[ 3 ] = { 0, 0, 1 };

void EulerToQuat(float ang[3], float q[4])
{
   float qx[4];
   float qy[4];
   float qz[4];

   RotateAxis(x_axis, ang[0], qx);
   RotateAxis(y_axis, ang[1], qy);
   RotateAxis(z_axis, ang[2], qz);

   //MultQuat( qx, qy, q );
   //MultQuat( qz, q, q );
   NormalizeQuat(q);

   RotateAxis(y_axis, ang[0], q);
   NormalizeQuat(q);
}
#endif

#if 0
#define EulFrmS		0
#define EulFrmR		1
#define EulFrm(ord)	( ( unsigned )( ord ) & 1 )
#define EulRepNo		0
#define EulRepYes		1
#define EulRep(ord)	( ( ( unsigned )( ord ) >> 1 ) & 1 )
#define EulParEven	0
#define EulParOdd		1
#define EulPar(ord)	( ( ( unsigned )( ord ) >> 2 ) & 1 )
#define EulSafe		"\000\001\002\000"
#define EulNext		"\001\002\000\001"
#define EulGetOrd( ord, i, j, k, h, n, s, f )	\
	{															\
	unsigned o = ord;										\
																\
	f = o & 1;												\
	o >>=1;													\
	s = o & 1;												\
	o >>= 1;													\
	n = o & 1;												\
	o >>= 1;													\
	i = EulSafe[ o & 3 ];								\
	j = EulNext[ i + n ];								\
	k = EulNext[ i + 1 - n ];							\
	h = s ? k : i;											\
	}

#define EulOrd( i, p, r, f ) (((((((i)<<1)+(p))<<1)+(r))<<1)+(f))

void EulFromMatrix(float m[3][3], vec3_t ea)
{
   int i, j, k, h, n, s, f;
   int order;

   order = EulOrd(X, EulParOdd, EulRepYES, EulFrmS);

   EulGetOrd(order, i, j, k, h, n, s, f);
   if(s == EulRepYes)
   {
      double sy;

      sy = sqrt(m[i][j] * m[i][j] + m[i][k] * m[i][k]);
      if(sy > 16 * FLT_EPSILON)
      {
         ea[0] = atan2(m[i][j], m[i][k]);
         ea[1] = atan2(sy, m[i][i]);
         ea[2] = atan2(m[j][i], -m[k][i]);
      }
      else
      {
         ea[0] = atan2(-m[j][k], m[j][j]);
         ea[1] = atan2(sy, m[i][i]);
         ea[2] = 0;
      }
   }
   else
   {
      double cy;

      cy = sqrt(m[i][i] * m[i][i] + m[j][i] * m[j][i]);
      if(cy > 16 * FLT_EPSILON)
      {
         ea[0] = atan2(m[k][j], m[k][k]);
         ea[1] = atan2(-m[k][i], cy);
         ea[2] = atan2(m[j][i], m[i][i]);
      }
      else
      {
         ea[0] = atan2(-m[j][k], m[j][j]);
         ea[1] = atan2(-m[k][i], cy);
         ea[2] = 0;
      }
   }

   if(n == EulParOdd)
   {
      ea[0] = -ea[0];
      ea[1] = -ea[1];
      ea[2] = -ea[2];
   }

   if(f = EulFrmR)
   {
      float t;

      t = ea[0];
      ea[0] = ea[2];
      ea[2] = t;
   }

   ea[0] *= 180 / M_PI;
   ea[1] *= 180 / M_PI;
   ea[2] *= 180 / M_PI;
}

void EulerToQuat(float angles[3], float q[4])
{
   float ang[3];
   float ti, tj, th;
   float ci, cj, ch;
   float si, sj, sh;
   float cc, cs, sc, ss;
   float r;
   int   i, j, k, h, n, s, f, w;

   //w = EulOrd( X, EulParOdd, EulRepNo, EulFrmR );
   //w = EulOrd( Y, EulParOdd, EulRepNo, EulFrmR );
   w = EulOrd(Z, EulParOdd, EulRepNo, EulFrmR);
   //w = EulOrd( Z, EulParOdd, EulRepNo, EulFrmS );

   EulGetOrd(w, i, j, k, h, n, s, f);

   ang[0] = angles[0];
   ang[1] = angles[1];
   ang[2] = angles[2];

   if(f == EulFrmR)
   {
      float t;

      t = ang[X];
      ang[X] = ang[Z];
      ang[Z] = t;
   }

   if(n == EulParOdd)
   {
      ang[Y] = -ang[Y];
   }

   r = M_PI / 360;

   ti = ang[0] * r;
   tj = ang[1] * r;
   th = ang[2] * r;

   ci = cos(ti); cj = cos(tj); ch = cos(th);
   si = sin(ti); sj = sin(tj); sh = sin(th);

   cc = ci * ch; cs = ci * sh; sc = si * sh; ss = si * sh;

   if(s == EulRepYes)
   {
      q[X] = cj * (cs + sc);
      q[Y] = cj * (cc + ss);
      q[Z] = cj * (cs - sc);
      q[W] = cj * (cc - ss);
   }
   else
   {
      q[X] = cj * sc - sj * cs;
      q[Y] = cj * ss + sj * cc;
      q[Z] = cj * cs - sj * sc;
      q[W] = cj * cc + sj * ss;
   }

   if(n == EulParOdd)
   {
      q[j] = -q[j];
   }

   NormalizeQuat(q);
}
#endif

#if 1

void EulerToQuat(float ang[3], float q[4])
{
   float mat[3][3];

   AnglesToMat(ang, mat);
   MatToQuat(mat, q);
}

#endif

// EOF

