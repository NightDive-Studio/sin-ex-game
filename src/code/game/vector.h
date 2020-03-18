//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/vector.h                         $
// $Revision:: 17                                                             $
//   $Author:: Markd                                                          $
//     $Date:: 9/25/98 4:42p                                                  $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// C++ implemention of a Vector object.  Handles standard vector operations
// such as addition, subtraction, normalization, scaling, dot product,
// cross product, length, and decomposition into Euler angles.
// 

#ifndef __VECTOR_H__
#define __VECTOR_H__

#include "g_local.h"
#include <math.h>
#include <stdio.h>

class EXPORT_FROM_DLL Vector
{
public:
   float x = 0.0f;
   float y = 0.0f;
   float z = 0.0f;

   constexpr Vector() noexcept = default;
   Vector(const vec3_t src);
   constexpr Vector(double px, double py, double pz) noexcept
       : x(float(px)), y(float(py)), z(float(pz))
   {
   }
   constexpr Vector(const Vector &other) noexcept = default;

   explicit Vector(const char *text);

   constexpr float pitch() const { return x; }
   constexpr float yaw()   const { return y; }
   constexpr float roll()  const { return z; }

   void setPitch(double pitch) { x = float(pitch); }
   void setYaw(double yaw)     { y = float(yaw);   }
   void setRoll(double roll)   { z = float(roll);  }

   float   *vec3();
   float    operator [] (int index) const;
   float   &operator [] (int index);
   
   void Vector::copyTo(vec3_t vec) const
   {
      vec[0] = x;
      vec[1] = y;
      vec[2] = z;
   }
   
   void setXYZ(double px, double py, double pz)
   {
      x = float(px);
      y = float(py);
      z = float(pz);
   }

   Vector  &operator = (vec3_t a);
   Vector  &operator += (const Vector &a);
   Vector  &operator -= (const Vector &a);
   Vector  &operator *= (double a);
   Vector  &CrossProduct(const Vector &a, const Vector &b);
   float    length() const;
   Vector  &normalize();
   float    normalize2(); //### returns the length of the original vector
   Vector   normalized() const;

   friend constexpr Vector operator + (const Vector &a, const Vector &b)
   {
       return Vector(a.x + b.x, a.y + b.y, a.z + b.z);
   }
   
   friend constexpr Vector operator - (const Vector &a, const Vector &b)
   {
       return Vector(a.x - b.x, a.y - b.y, a.z - b.z);
   }

   friend constexpr Vector operator * (const Vector &a, double b)
   {
       return Vector(a.x * b, a.y * b, a.z * b);
   }

   friend constexpr Vector operator * (double a, const Vector &b)
   {
       return b * a;
   }

   friend constexpr float  operator * (const Vector &a, const Vector &b)
   {
       return a.x * b.x + a.y * b.y + a.z * b.z;
   }
   
   friend constexpr int operator == (const Vector &a, const Vector &b)
   {
       return ((a.x == b.x) && (a.y == b.y) && (a.z == b.z));
   }

   friend constexpr int operator != (const Vector &a, const Vector &b)
   {
       return ((a.x != b.x) || (a.y != b.y) || (a.z != b.z));
   }

   // haleyjd 20170522: fixed; was returning reference to a local temporary
   constexpr Vector operator - () const
   {
       return Vector(-x, -y, -z);
   }

   float    toYaw() const;
   float    toPitch() const;
   Vector   toAngles() const;
   void     AngleVectors(Vector *forward, Vector *right, Vector *up) const;

   friend   Vector fabs(const Vector &a);
   friend   Vector LerpVector(const Vector &w1, const Vector &w2, float t);
   friend   float  MaxValue(const Vector &a);
};

inline float Vector::operator [] (int index) const
{
   assert((index >= 0) && (index < 3));
   return (&x)[index];
}

inline float& Vector::operator [] (int index)
{
   assert((index >= 0) && (index < 3));
   return (&x)[index];
}

inline Vector::Vector(const vec3_t src) : x(src[0]), y(src[1]), z(src[2])
{
}

inline Vector::Vector(const char *text) : Vector()
{
   if(text)
      sscanf(text, "%f %f %f", &x, &y, &z);
}

inline float *Vector::vec3()
{
   return &x;
}

inline Vector &Vector::operator = (vec3_t a)
{
   x = a[0];
   y = a[1];
   z = a[2];

   return *this;
}

inline Vector &Vector::operator += (const Vector &a)
{
   x += a.x;
   y += a.y;
   z += a.z;

   return *this;
}

inline Vector &Vector::operator -= (const Vector &a)
{
   x -= a.x;
   y -= a.y;
   z -= a.z;

   return *this;
}

inline Vector& Vector::operator *= (double a)
{
   x *= a;
   y *= a;
   z *= a;

   return *this;
}

inline Vector &Vector::CrossProduct(const Vector &a, const Vector &b)
{
   // haleyjd: do calculations before modification in case 'this' is an argument
   vec3_t tmp; 
   
   tmp[0] = a.y * b.z - a.z * b.y;
   tmp[1] = a.z * b.x - a.x * b.z;
   tmp[2] = a.x * b.y - a.y * b.x;

   *this = tmp;

   return *this;
}

inline float Vector::length() const
{
   float length = x * x + y * y + z * z;
   return sqrtf(length);
}

inline Vector &Vector::normalize()
{
   float length, ilength;

   length = this->length();
   if(length)
   {
      ilength = 1 / length;
      x *= ilength;
      y *= ilength;
      z *= ilength;
   }

   return *this;
}

//###
// normalize function that returns the length of the original vector
inline float Vector::normalize2()
{
   float length, ilength;

   length = this->length();
   if(length)
   {
      ilength = 1 / length;
      x *= ilength;
      y *= ilength;
      z *= ilength;
   }

   return length;
}
//###

inline Vector Vector::normalized() const
{
   Vector v(*this);
   float length, ilength;

   length = v.length();
   if(length)
   {
      ilength = 1 / length;
      v.x *= ilength;
      v.y *= ilength;
      v.z *= ilength;
   }

   return v;
}

inline Vector fabs(const Vector &a)
{
   return Vector(fabs(a.x), fabs(a.y), fabs(a.z));
}

inline float MaxValue(const Vector &a)
{
   float maxy;
   float maxz;
   float max;

   max  = fabs(a.x);
   maxy = fabs(a.y);
   maxz = fabs(a.z);
   if(maxy > max)
      max = maxy;
   if(maxz > max)
      max = maxz;
   return max;
}

inline float Vector::toYaw() const
{
   float yaw;

   if((y == 0) && (x == 0))
   {
      yaw = 0;
   }
   else
   {
      yaw = (float)((int)(atan2(y, x) * 180 / M_PI));
      if(yaw < 0)
      {
         yaw += 360;
      }
   }

   return yaw;
}

inline float Vector::toPitch() const
{
   float forward;
   float pitch;

   if((x == 0) && (y == 0))
   {
      if(z > 0)
      {
         pitch = 90;
      }
      else
      {
         pitch = 270;
      }
   }
   else
   {
      forward = (float)sqrt(x * x + y * y);
      pitch = (float)((int)(atan2(z, forward) * 180 / M_PI));
      if(pitch < 0)
      {
         pitch += 360;
      }
   }

   return pitch;
}

inline Vector Vector::toAngles() const
{
   float forward;
   float yaw, pitch;

   if((x == 0) && (y == 0))
   {
      yaw = 0;
      if(z > 0)
      {
         pitch = 90;
      }
      else
      {
         pitch = 270;
      }
   }
   else
   {
      yaw = atan2(y, x) * 180 / M_PI;
      if(yaw < 0)
      {
         yaw += 360;
      }

      forward = (float)sqrt(x * x + y * y);
      pitch = atan2(z, forward) * 180 / M_PI;
      if(pitch < 0)
      {
         pitch += 360;
      }
   }

   return Vector(pitch, yaw, 0);
}

inline void Vector::AngleVectors(Vector *forward, Vector *right, Vector *up) const
{
   float          angle;
   // SINEX_FIXME: test if still necessary
   static float   sr, sp, sy, cr, cp, cy; // static to help MS compiler fp bugs

   angle = yaw() * (M_PI * 2 / 360);
   sy = sin(angle);
   cy = cos(angle);

   angle = pitch() * (M_PI * 2 / 360);
   sp = sin(angle);
   cp = cos(angle);

   angle = roll() * (M_PI * 2 / 360);
   sr = sin(angle);
   cr = cos(angle);

   if(forward)
   {
      forward->setXYZ(cp * cy, cp * sy, -sp);
   }

   if(right)
   {
      right->setXYZ(-1 * sr * sp * cy + -1 * cr * -sy, -1 * sr * sp * sy + -1 * cr * cy, -1 * sr * cp);
   }

   if(up)
   {
      up->setXYZ(cr * sp * cy + -sr * -sy, cr * sp * sy + -sr * cy, cr * cp);
   }
}

#define LERP_DELTA 1e-6
inline Vector LerpVector(const Vector &pw1, const Vector &pw2, float t)
{
   double   omega, cosom, sinom, scale0, scale1;

   Vector w1 = pw1.normalized();
   Vector w2 = pw2.normalized();

   cosom = w1 * w2;
   if((1.0 - cosom) > LERP_DELTA)
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

   return (w1 * scale0 + w2 * scale1);
}

class EXPORT_FROM_DLL Quat
{
public:
   float    x = 0.0f;
   float    y = 0.0f;
   float    z = 0.0f;
   float    w = 0.0f;

   constexpr Quat() noexcept = default;
   Quat(Vector &angles);
   Quat(float scrMatrix[3][3]);
   constexpr Quat(double px, double py, double pz, double pw) noexcept
       : x(float(px)), y(float(py)), z(float(pz)), w(float(pw))
   {
   }
   Quat(float q[4]);

   float   *vec4();
   float    operator [] (int index) const;
   float   &operator [] (int index);

   void set(double px, double py, double pz, double pw)
   {
       x = float(px);
       y = float(py);
       z = float(pz);
       w = float(pw);
   }

   Quat    &operator += (const Quat &a);
   Quat    &operator -= (const Quat &a);
   Quat    &operator *= (double a);
   float    length() const;
   Quat    &normalize();
   Vector   toAngles();

   constexpr Quat operator - () const noexcept
   {
       return Quat(-x, -y, -z, -w);
   }

   friend constexpr Quat operator + (const Quat &a, const Quat &b) noexcept
   {
       return Quat(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
   }

   friend constexpr Quat operator - (const Quat &a, const Quat &b) noexcept
   {
       return Quat(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
   }

   friend constexpr Quat operator * (const Quat &a, double b) noexcept
   {
       return Quat(a.x * b, a.y * b, a.z * b, a.w * b);
   }

   friend constexpr Quat operator * (double a, const Quat &b) noexcept
   {
       return b * a;
   }

   friend constexpr int operator == (const Quat &a, const Quat &b) noexcept
   {
       return ((a.x == b.x) && (a.y == b.y) && (a.z == b.z) && (a.w == b.w));
   }

   friend constexpr int operator != (const Quat &a, const Quat &b) noexcept
   {
       return ((a.x != b.x) || (a.y != b.y) || (a.z != b.z) && (a.w != b.w));
   }
};

inline float Quat::operator [] (int index) const
{
   assert((index >= 0) && (index < 4));
   return (&x)[index];
}

inline float& Quat::operator [] (int index)
{
   assert((index >= 0) && (index < 4));
   return (&x)[index];
}

inline float *Quat::vec4()
{
   return &x;
}

inline Quat::Quat(Vector &angles)
{
   EulerToQuat(angles.vec3(), this->vec4());
}

inline Quat::Quat(float srcMatrix[3][3])
{
   MatToQuat(srcMatrix, this->vec4());
}

inline Quat::Quat(float q[4]) : x(q[0]), y(q[1]), z(q[2]), w(q[3])
{
}

inline Quat &Quat::operator += (const Quat &a)
{
   x += a.x;
   y += a.y;
   z += a.z;
   w += a.w;

   return *this;
}

inline Quat &Quat::operator -= (const Quat &a)
{
   x -= a.x;
   y -= a.y;
   z -= a.z;
   w -= a.w;

   return *this;
}

inline Quat &Quat::operator *= (double a)
{
   x *= a;
   y *= a;
   z *= a;
   w *= a;

   return *this;
}

inline float Quat::length() const
{
   float length = x * x + y * y + z * z + w * w;
   return sqrtf(length);
}

inline Quat &Quat::normalize()
{
   float length, ilength;

   length = this->length();
   if(length)
   {
      ilength = 1 / length;
      x *= ilength;
      y *= ilength;
      z *= ilength;
      w *= ilength;
   }

   return *this;
}

inline Vector Quat::toAngles()
{
   float m[3][3];
   vec3_t angles;

   QuatToMat(this->vec4(), m);
   MatrixToEulerAngles(m, angles);
   return Vector(angles);
}

#endif /* Vector.h */

// EOF

