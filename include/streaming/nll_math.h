/*

LICENSE
  See end of file for license information.
  
*/
/*
  memory layout  :: [ a00 a01 a02 a03 a04 a05 a06 a07 a08 a09 a10 a11 a12 a13 a14 a15 ]

  logical layout :: ColumnMajorFormat
                    [ a00 a04 a08 a12 ]
                    [ a01 a05 a09 a13 ]
                    [ a02 a06 a10 a14 ]
                    [ a03 a07 a11 a15 ]

  logical layout :: RowMajorFormat
                    [ a00 a01 a02 a03 ]
                    [ a04 a05 a06 a07 ]
                    [ a08 a09 a10 a11 ]
                    [ a12 a13 a14 a15 ]

  internal storage is ColumnMajorFormat (Ã  la OpenGL)
*/

/*

NOTE(nico) I want these functions as basic as possible, populating simple float arrays
NOTE(nico) define 'NLL_MATH_DEFINITIONS' in a C/C++ file to create function definitions

PROBLEMS :

* with `typedef float nllVec3[3];` we can't assign values `nllVec3 a, b; b = a;` leads to an error `array type 'float [3]' is no
t assignable`
* so `typedef struct nllVec3 {...}` would allow assignment, at a syntactic cost when calling functions
* so `typedef struct nllVec3 {...}` would still allow constructs like `nllVec3 a = {0,0,0}`
* KISS => added `nllVec3Assign`

*/

#pragma once

#define NLLM_( m_, row, col ) ((m_)[(row)+((col)*4)])     // column-major layout

#ifndef nll_assert
# include <assert.h>
# define nll_assert(x) assert(x)
#endif

//

typedef float nllMat44[16];
typedef float nllVec3[3];						// FIXME(nico) ce typedef est perturbant : on ne peut paut pas utiliser 'nllVec3 toto; sizeof(toto)' : sizeof(toto) donne la taille d'un pointeur

//

void nllVec3Assign( float *dst, float const *src );
void nllVec3Neg( float *r );
void nllVec3Normalize( float r[3] );

void nllVec3Mulf( float *r, const float *u, const float f );
void nllVec3Add( float *r, const float *u, const float *v );
void nllVec3Sub( float *r, const float *u, const float *v );
void nllVec3Cross( float *r, const float *u, const float *v );

void nllVec3Transform( float *r, const float m[16], const float v[3] );

void nllMat44Identity( float *m );

void nllMat44Mul( float *m, float const *a, float const *b );

void nllMat44Translation(float *m, const float *v);
void nllMat44TranslationInverse(float *m, const float *v);

void nllMat44RotationX( float *m, float cosx, float sinx );
void nllMat44RotationXInverse( float *m, float cosx, float sinx );
void nllMat44RotationY( float *m, float cosx, float sinx );
void nllMat44RotationYInverse( float *m, float cosx, float sinx );
void nllMat44RotationZ( float *m, float cosx, float sinx );
void nllMat44RotationZInverse( float *m, float cosx, float sinx );

void nllMat44Scale( float *m, float x, float y, float z );
void nllMat44ScaleInverse( float *m, float x, float y, float z );

void nllMat44Frustum( float* m, float left, float right, float bottom, float top, float nearVal, float farVal );
void nllMat44Ortho( float *m, float left, float right, float bottom, float top, float nearVal, float farVal );
void nllMat44Orthogonal33( float *m, const float c0[3], const float c1[3], const float c2[3] );
void nllMat44Orthogonal33Inverse( float *m, const float c0[3], const float c1[3], const float c2[3] );

void nllMat44EyeFromWorld( float *m, const float *eye, const float x[3], const float y[3], const float z[3] );

void nllFrustumPlanesFromMat44(float planes[6][4], const float mat[16]);

int nllIsAABBInFrustum(const float aabbCenter[3], const float aabbExtent[3], const float frustumPlanes[6][4], int *planeMask);

//

#ifdef NLL_MATH_DEFINITIONS

#include <math.h>

void nllVec3Assign( float *dst, float const *src )
{
  memcpy(dst, src, sizeof(float[3]));
}

void nllVec3Neg( float *r )
{
  r[0] = -r[0];
  r[1] = -r[1];
  r[2] = -r[2];
}

void nllVec3Normalize( float r[3] )
{
  float length = sqrtf( r[0]*r[0] + r[1]*r[1] + r[2]*r[2] );
  r[0] /= length;
  r[1] /= length;
  r[2] /= length;
}

void nllVec3Mulf( float *r, const float *u, const float f )
{
  r[0] = u[0] * f;
  r[1] = u[1] * f;
  r[2] = u[2] * f;
}

void nllVec3Add( float *r, const float *u, const float *v )
{
  r[0] = u[0] + v[0];
  r[1] = u[1] + v[1];
  r[2] = u[2] + v[2];
}

void nllVec3Sub( float *r, const float *u, const float *v )
{
  r[0] = u[0] - v[0];
  r[1] = u[1] - v[1];
  r[2] = u[2] - v[2];
}

void nllVec3Cross( float *r, const float *u, const float *v )
{
  const float ux = u[0];
  const float uy = u[1];
  const float uz = u[2];
  const float vx = v[0];
  const float vy = v[1];
  const float vz = v[2];

  r[0] = uy * vz - uz * vy;
  r[1] = uz * vx - ux * vz;
  r[2] = ux * vy - uy * vx;
}

void nllVec3Transform( float *r, const float m[16], const float v[3] )
{
  const float src[3] = { v[0], v[1], v[2] };
  r[0] = NLLM_(m, 0, 0) * src[0] + NLLM_(m, 0, 1) * src[1] + NLLM_(m, 0, 2) * src[2] + NLLM_(m, 0, 3);
  r[1] = NLLM_(m, 1, 0) * src[0] + NLLM_(m, 1, 1) * src[1] + NLLM_(m, 1, 2) * src[2] + NLLM_(m, 1, 3);
  r[2] = NLLM_(m, 2, 0) * src[0] + NLLM_(m, 2, 1) * src[1] + NLLM_(m, 2, 2) * src[2] + NLLM_(m, 2, 3);
}

void nllMat44Identity( float *m )
{
  NLLM_(m,0,0) = 1.0f; NLLM_(m,0,1) = 0.0f; NLLM_(m,0,2) = 0.0f; NLLM_(m,0,3) = 0.0f;
  NLLM_(m,1,0) = 0.0f; NLLM_(m,1,1) = 1.0f; NLLM_(m,1,2) = 0.0f; NLLM_(m,1,3) = 0.0f;
  NLLM_(m,2,0) = 0.0f; NLLM_(m,2,1) = 0.0f; NLLM_(m,2,2) = 1.0f; NLLM_(m,2,3) = 0.0f;
  NLLM_(m,3,0) = 0.0f; NLLM_(m,3,1) = 0.0f; NLLM_(m,3,2) = 0.0f; NLLM_(m,3,3) = 1.0f;
}

void nllMat44Mul( float *m, float const *a, float const *b )
{
  nll_assert(m != a && m != b);

  for (int col=0; col<4; col++) {
    for (int row=0; row<4; row++) {
      NLLM_(m,row,col) =
        NLLM_(a,row,0) * NLLM_(b,0,col) + 
        NLLM_(a,row,1) * NLLM_(b,1,col) + 
        NLLM_(a,row,2) * NLLM_(b,2,col) + 
        NLLM_(a,row,3) * NLLM_(b,3,col)
      ;
    }
  }
}

void nllMat44Translation(float *m, const float *v)
{
	NLLM_(m, 0, 0) = 1.0f; NLLM_(m, 0, 1) = 0.0f; NLLM_(m, 0, 2) = 0.0f; NLLM_(m, 0, 3) = v[0];
	NLLM_(m, 1, 0) = 0.0f; NLLM_(m, 1, 1) = 1.0f; NLLM_(m, 1, 2) = 0.0f; NLLM_(m, 1, 3) = v[1];
	NLLM_(m, 2, 0) = 0.0f; NLLM_(m, 2, 1) = 0.0f; NLLM_(m, 2, 2) = 1.0f; NLLM_(m, 2, 3) = v[2];
	NLLM_(m, 3, 0) = 0.0f; NLLM_(m, 3, 1) = 0.0f; NLLM_(m, 3, 2) = 0.0f; NLLM_(m, 3, 3) = 1.0f;
}

void nllMat44TranslationInverse(float *m, const float *v)
{
	NLLM_(m, 0, 0) = 1.0f; NLLM_(m, 0, 1) = 0.0f; NLLM_(m, 0, 2) = 0.0f; NLLM_(m, 0, 3) = -v[0];
	NLLM_(m, 1, 0) = 0.0f; NLLM_(m, 1, 1) = 1.0f; NLLM_(m, 1, 2) = 0.0f; NLLM_(m, 1, 3) = -v[1];
	NLLM_(m, 2, 0) = 0.0f; NLLM_(m, 2, 1) = 0.0f; NLLM_(m, 2, 2) = 1.0f; NLLM_(m, 2, 3) = -v[2];
	NLLM_(m, 3, 0) = 0.0f; NLLM_(m, 3, 1) = 0.0f; NLLM_(m, 3, 2) = 0.0f; NLLM_(m, 3, 3) = 1.0f;
}

void nllMat44RotationX( float *m, float cosx, float sinx )
{
  NLLM_(m,0,0) = 1.0f; NLLM_(m,0,1) = 0.0f; NLLM_(m,0,2) = 0.0f; NLLM_(m,0,3) = 0.0f;
  NLLM_(m,1,0) = 0.0f; NLLM_(m,1,1) = cosx; NLLM_(m,1,2) =-sinx; NLLM_(m,1,3) = 0.0f;
  NLLM_(m,2,0) = 0.0f; NLLM_(m,2,1) = sinx; NLLM_(m,2,2) = cosx; NLLM_(m,2,3) = 0.0f;
  NLLM_(m,3,0) = 0.0f; NLLM_(m,3,1) = 0.0f; NLLM_(m,3,2) = 0.0f; NLLM_(m,3,3) = 1.0f;
}

void nllMat44RotationXInverse( float *m, float cosx, float sinx )
{
  nllMat44RotationY( m, cosx, -sinx );
}

void nllMat44RotationY( float *m, float cosx, float sinx )
{
  NLLM_(m,0,0) = cosx; NLLM_(m,0,1) = 0.0f; NLLM_(m,0,2) = sinx; NLLM_(m,0,3) = 0.0f;
  NLLM_(m,1,0) = 0.0f; NLLM_(m,1,1) = 1.0f; NLLM_(m,1,2) = 0.0f; NLLM_(m,1,3) = 0.0f;
  NLLM_(m,2,0) =-sinx; NLLM_(m,2,1) = 0.0f; NLLM_(m,2,2) = cosx; NLLM_(m,2,3) = 0.0f;
  NLLM_(m,3,0) = 0.0f; NLLM_(m,3,1) = 0.0f; NLLM_(m,3,2) = 0.0f; NLLM_(m,3,3) = 1.0f;
}

void nllMat44RotationYInverse( float *m, float cosx, float sinx )
{
  nllMat44RotationY( m, cosx, -sinx );
}

void nllMat44RotationZ( float *m, float cosx, float sinx )
{
  NLLM_(m,0,0) = cosx; NLLM_(m,0,1) =-sinx; NLLM_(m,0,2) = 0.0f; NLLM_(m,0,3) = 0.0f;
  NLLM_(m,1,0) = sinx; NLLM_(m,1,1) = cosx; NLLM_(m,1,2) = 0.0f; NLLM_(m,1,3) = 0.0f;
  NLLM_(m,2,0) = 0.0f; NLLM_(m,2,1) = 0.0f; NLLM_(m,2,2) = 1.0f; NLLM_(m,2,3) = 0.0f;
  NLLM_(m,3,0) = 0.0f; NLLM_(m,3,1) = 0.0f; NLLM_(m,3,2) = 0.0f; NLLM_(m,3,3) = 1.0f;
}

void nllMat44RotationZInverse( float *m, float cosx, float sinx )
{
  nllMat44RotationZ( m, cosx, -sinx );
}

void nllMat44Scale( float *m, float x, float y, float z )
{
  NLLM_(m,0,0) =    x; NLLM_(m,0,1) = 0.0f; NLLM_(m,0,2) = 0.0f; NLLM_(m,0,3) = 0.0f;
  NLLM_(m,1,0) = 0.0f; NLLM_(m,1,1) =    y; NLLM_(m,1,2) = 0.0f; NLLM_(m,1,3) = 0.0f;
  NLLM_(m,2,0) = 0.0f; NLLM_(m,2,1) = 0.0f; NLLM_(m,2,2) =    z; NLLM_(m,2,3) = 0.0f;
  NLLM_(m,3,0) = 0.0f; NLLM_(m,3,1) = 0.0f; NLLM_(m,3,2) = 0.0f; NLLM_(m,3,3) = 1.0f;
}

void nllMat44ScaleInverse( float *m, float x, float y, float z )
{
  nllMat44Scale( m, 1.f / x, 1.f / y, 1.f / z );
}

void nllMat44Frustum( float* m, float left, float right, float bottom, float top, float nearVal, float farVal )
{
  float A = (right + left) / (right - left);
  float B = (top + bottom) / (top - bottom);

  float X = (2.f * nearVal) / (right - left);
  float Y = (2.f * nearVal) / (top - bottom);

  float C, D;
  C = -(farVal + nearVal) / (farVal - nearVal);
  D = -(2.f * farVal * nearVal) / (farVal - nearVal);

  NLLM_(m,0,0) =    X; NLLM_(m,0,1) = 0.0f; NLLM_(m,0,2) =    A; NLLM_(m,0,3) = 0.0f;
  NLLM_(m,1,0) = 0.0f; NLLM_(m,1,1) =    Y; NLLM_(m,1,2) =    B; NLLM_(m,1,3) = 0.0f;
  NLLM_(m,2,0) = 0.0f; NLLM_(m,2,1) = 0.0f; NLLM_(m,2,2) =    C; NLLM_(m,2,3) =    D;
  NLLM_(m,3,0) = 0.0f; NLLM_(m,3,1) = 0.0f; NLLM_(m,3,2) =-1.0f; NLLM_(m,3,3) = 0.0f;
}

// TODO(nico) nllMat44FrustumInverse

void nllMat44Ortho( float *m, float left, float right, float bottom, float top, float nearVal, float farVal )
{
  float Tx = -(right + left) / (right - left);
  float Ty = -(top + bottom) / (top - bottom);

  float X =  2.f / (right - left);
  float Y =  2.f / (top - bottom);

  float Z = -2.f / (farVal - nearVal);
  float Tz = -(farVal + nearVal) / (farVal - nearVal);

  NLLM_(m,0,0) =    X; NLLM_(m,0,1) = 0.0f; NLLM_(m,0,2) = 0.0f; NLLM_(m,0,3) =   Tx;
  NLLM_(m,1,0) = 0.0f; NLLM_(m,1,1) =    Y; NLLM_(m,1,2) = 0.0f; NLLM_(m,1,3) =   Ty;
  NLLM_(m,2,0) = 0.0f; NLLM_(m,2,1) = 0.0f; NLLM_(m,2,2) =    Z; NLLM_(m,2,3) =   Tz;
  NLLM_(m,3,0) = 0.0f; NLLM_(m,3,1) = 0.0f; NLLM_(m,3,2) = 0.0f; NLLM_(m,3,3) = 1.0f;
}

// TODO(nico) nllMat44OrthoInverse

void nllMat44Orthogonal33( float *m, const float c0[3], const float c1[3], const float c2[3] )
{
  NLLM_(m,0,0) = c0[0]; NLLM_(m,0,1) = c1[0]; NLLM_(m,0,2) = c2[0]; NLLM_(m,0,3) = 0.0f;
  NLLM_(m,1,0) = c0[1]; NLLM_(m,1,1) = c1[1]; NLLM_(m,1,2) = c2[1]; NLLM_(m,1,3) = 0.0f;
  NLLM_(m,2,0) = c0[2]; NLLM_(m,2,1) = c1[2]; NLLM_(m,2,2) = c2[2]; NLLM_(m,2,3) = 0.0f;
  NLLM_(m,3,0) =  0.0f; NLLM_(m,3,1) =  0.0f; NLLM_(m,3,2) =  0.0f; NLLM_(m,3,3) = 1.0f;
}

/* the inverse of an orthogonal matrix is the transpose */
void nllMat44Orthogonal33Inverse( float *m, const float c0[3], const float c1[3], const float c2[3] )
{
  NLLM_(m,0,0) = c0[0]; NLLM_(m,0,1) = c0[1]; NLLM_(m,0,2) = c0[2]; NLLM_(m,0,3) = 0.0f;
  NLLM_(m,1,0) = c1[0]; NLLM_(m,1,1) = c1[1]; NLLM_(m,1,2) = c1[2]; NLLM_(m,1,3) = 0.0f;
  NLLM_(m,2,0) = c2[0]; NLLM_(m,2,1) = c2[1]; NLLM_(m,2,2) = c2[2]; NLLM_(m,2,3) = 0.0f;
  NLLM_(m,3,0) =  0.0f; NLLM_(m,3,1) =  0.0f; NLLM_(m,3,2) =  0.0f; NLLM_(m,3,3) = 1.0f;
}

/* FIXME(nico) not sure why we map 'front' to -Z */
void nllMat44EyeFromWorld( float *m, const float *eye, const float x[3], const float y[3], const float z[3] )
{
  nllMat44 orientation, translation;
  float eyeNeg[3] = { -eye[0], -eye[1], -eye[2] };
  float negZ[3] = { -z[0], -z[1], -z[2] };

  nllMat44Orthogonal33Inverse( orientation, x, y, negZ );
  nllMat44Translation( translation, eyeNeg );
  nllMat44Mul( m, orientation, translation );
}


// from https://fgiesen.wordpress.com/2012/08/31/frustum-planes-from-the-projection-matrix/
// from https://stackoverflow.com/questions/12836967/extracting-view-frustum-planes-hartmann-gribbs-method
void nllFrustumPlanesFromMat44(float planes[6][4], const float mat[16])
{   
  for (int i=0; i<4; i++) planes[0][i] = NLLM_(mat,3,i) + NLLM_(mat,0,i);
  for (int i=0; i<4; i++) planes[1][i] = NLLM_(mat,3,i) - NLLM_(mat,0,i); 
  for (int i=0; i<4; i++) planes[2][i] = NLLM_(mat,3,i) + NLLM_(mat,1,i);
  for (int i=0; i<4; i++) planes[3][i] = NLLM_(mat,3,i) - NLLM_(mat,1,i);
  for (int i=0; i<4; i++) planes[4][i] = NLLM_(mat,3,i) + NLLM_(mat,2,i);
  for (int i=0; i<4; i++) planes[5][i] = NLLM_(mat,3,i) - NLLM_(mat,2,i);
}


// from https://fgiesen.wordpress.com/2010/10/17/view-frustum-culling/
// TODO(nico) check http://www.iquilezles.org/www/articles/frustumcorrect/frustumcorrect.htm
int nllIsAABBInFrustum(const float aabbCenter[3], const float aabbExtent[3], const float frustumPlanes[6][4], int *planeMask) {

  const int inPlaneMask = planeMask ? *planeMask : 0x3F;
  int outPlaneMask = 0;

  if (planeMask) {
    *planeMask = outPlaneMask;
  }

  for (int p=0; p<6; p++) {

    const int thisPlaneBit = (1<<p);
    if (0 == (inPlaneMask & thisPlaneBit)) {
      continue;
    }

    float d = aabbCenter[0]*frustumPlanes[p][0] + aabbCenter[1]*frustumPlanes[p][1] + aabbCenter[2]*frustumPlanes[p][2];
    float r = aabbExtent[0]*fabsf(frustumPlanes[p][0]) + aabbExtent[1]*fabsf(frustumPlanes[p][1]) + aabbExtent[2]*fabsf(frustumPlanes[p][2]);
    
    //if (d + r > -frustumPlanes[p][3])  // partially inside
    //if (d - r >= -frustumPlanes[p][3]) // fully inside

    if (d + r < -frustumPlanes[p][3]) return 0;
    if (d - r < -frustumPlanes[p][3]) outPlaneMask |= thisPlaneBit;
  }

  if (planeMask) {
    *planeMask = outPlaneMask;
  }
  return 1;
}

#endif  // #ifdef NLL_MATH_DEFINITIONS

/*
------------------------------------------------------------------------------
This software is available under 2 licenses -- choose whichever you prefer.
------------------------------------------------------------------------------
ALTERNATIVE A - MIT License
Copyright (c) 2018 Nicolas Lelong
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
------------------------------------------------------------------------------
ALTERNATIVE B - Public Domain (www.unlicense.org)
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.
In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
------------------------------------------------------------------------------
*/
