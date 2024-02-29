////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2016 RWS Inc, All Rights Reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of version 2 of the GNU General Public License as published by
// the Free Software Foundation
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
//
////////////////////////////////////////////////////////////////////////////////
//
// VectorMath.h
// Project: RSPiX
//
// This module implements high speed manipulations of standard mathematical
// vectors and matrices.
//
// History:
//		??/??/?? JRD	Started.
//
//		02/11/97	JMI	Added Load() and Save() for compatability with RChannel,
//							RResMgr, and other such file-based thingers.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef VECTOR_MATH_H
#define VECTOR_MATH_H

#include "System.h"
#ifdef PATHS_IN_INCLUDES
	#include "ORANGE/File/file.h"
	#include "ORANGE/QuickMath/QuickMath.h"
#else
	#include "file.h"
	#include "quickmath.h"
#endif
//===================================
// This file contains data and operations
// for high speed manipulations of standard
// mathematical vectors and matrices.

/*===================================
RP3d; // This is a 3d point.
inline void rspMakeHom(RP3d& p) // do if in question
inline void rspCopy(RP3d& a,RP3d& b)
inline void rspMakeUnit(RP3d& p)
inline REAL rspDot(RP3d& a,RP3d& b)
inline void rspCross(RP3d& a,RP3d& b,RP3d& c) // c = a x b
RTransform 
	{ T[16],
	RTransform() // init to an identity transform
	RTransform(REAL* M) // init to a copy of another transform
	void Make1()
	void Make0()
	void PreMulBy(REAL* M)
	void Mul(REAL* A,REAL* B) // 4x4 transforms:
	void Transform(RP3d &p)
	void TransformInto(RP3d &XF, RP3d& p) XF = this x p
	void Trans(REAL x,REAL y,REAL z)
	void Scale(REAL a,REAL b, REAL c)
	void Rz(short sDeg) // CCW!
	void Rx(short sDeg) // CCW!
	void Ry(short sDeg) // CCW!
	void MakeScreenXF(
		REAL x1,REAL y1,REAL w1,REAL h1,
		REAL x2,REAL y2,REAL w2,REAL h2)
	void MakeRotTo(RP3d point,RP3d up)
	void MakeRotFrom(RP3d point,RP3d up)
	}
//=================================*/

#define REAL float // float conserves internal memory..
// Note that if REAL is double, than it should somehow hook
// double sized quick trig, so it doesn't have to do
// conversions each time!

// This is an aggregate type used with RTransform
typedef union
	{
	// 06/30/97 MJR - For the metrowerks compiler, initializations don't
	// seem to work right unless the array comes before the struct!
	REAL v[4];
	struct
		{
		REAL x;
		REAL y;
		REAL z;
		REAL w;
		};

	short Load(RFile* pfile)
		{ return pfile->Read(v, 4) != 4; }

	short Save(RFile* pfile)
		{ return pfile->Write(v, 4) != 4; }

	} RP3d; // This is a 3d point.

inline int operator==(const RP3d& lhs, const RP3d& rhs)
	{
	if (lhs.v == rhs.v)
		return 1;
	return 0;
	}


// divides out the w component: (makes homogeneous)
inline void rspMakeHom(RP3d& p)
	{
#ifdef _DEBUG
	if (p.w == 0.0)
		{
		TRACE("FATAL ERROR - POINT AT INFINITY!");
		return;
		}
#endif
	
	REAL w = p.w;
	p.x /= w;
	p.y /= w;
	p.z /= w;
	p.w = 1.00;
	}

// Can be useful:
// adjusts the length of a vector, ignoring w component
inline void rspMakeUnit(RP3d& p)
	{
	REAL l = sqrt(SQR(p.x)+SQR(p.y)+SQR(p.z));
#ifdef _DEBUG
	if (l == 0.0)
		{
		TRACE("FATAL ERROR - NULL VECTOR!");
		return;
		}
#endif
	p.x /= l;
	p.y /= l;
	p.z /= l;
	}

// returns a dot b
inline REAL rspDot(RP3d& a,RP3d& b)
	{
	return a.x*b.x + a.y*b.y + a.z*b.z;
	}

// a = b, does NOT deal with w!
inline void rspCopy(RP3d& a,RP3d& b)
	{
	a.x = b.x;
	a.y = b.y;
	a.z = b.z;
	}

// c = a x b
inline void rspCross(RP3d& a,RP3d& b,RP3d& c)
	{
	c.x = a.y * b.z - a.z * b.y;
	c.z = a.x * b.y - a.y * b.x;
	c.y = a.z * b.x - a.x * b.z;
	}

// a -= b;
inline void rspSub(RP3d& a,RP3d& b)
	{
	a.x -= b.x;
	a.y -= b.y;
	a.z -= b.z;
	}

// a += b;
inline void rspAdd(RP3d& a,RP3d& b)
	{
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
	}

// a += s;
inline void rspScale(RP3d& a,REAL s)
	{
	a.x *= s;
	a.y *= s;
	a.z *= s;
	}



// And some useful constants for manipulation:
const short ROW[4] = {0,4,8,12};
const short ROW0 = 0;
const short ROW1 = 4;
const short ROW2 = 8;
const short ROW3 = 12;

const REAL Identity[16] = {1.0,0.0,0.0,0.0, 
									0.0,1.0,0.0,0.0,
									0.0,0.0,1.0,0.0,
									0.0,0.0,0.0,1.0};

// NOW, the class based transform allows matrix
// multiplication to occur WITHOUT multiplying
// 2 matrices together.  This prevents a malloc
// nightmare:
//
class RTransform
	{
public:
	REAL T[16]; // This is compatible with the aggregate transform
	RTransform() // init to an identity transform
		{ 
		for (short i=0;i<16;i++) 
			T[i]=Identity[i];
		}

	RTransform(REAL* M) // init to a copy of another transform
		{ 
		for (short i=0;i<16;i++) 
			T[i]=M[i];
		}

	~RTransform(){};

	int operator==(const RTransform& rhs) const
		{
		if (T == rhs.T)
			return 1;
		return 0;
		}

	void Make1() // identity matrix
		{
		for (short i=0;i<16;i++) 
			T[i]=Identity[i];
		}

	void Make0() // null matrix
		{
		for (short i=0;i<15;i++) 
			T[i]=(REAL)0;
		T[15] = (REAL)1;
		}

	//------------------------
	// ALL TRANSFORMATIONS ARE PRE-MULTIPLIES,
	// A Partial transform, assuming R3 = {0,0,0,1};
	// 
	void PreMulBy(REAL* M)
		{
		//REAL* MLine = M;
		//REAL* TCol = T;
		//REAL tot;
		short r,c;
		// Unroll this puppy!
		// Much optimizing needed!
		for (r = 0;r<3;r++) // 3/4 XFORM!
			for (c=0;c<4;c++)
				{
				T[ ROW[r] + c] =
					M[ ROW[r]] * T[c] + 
					M[ ROW[r] + 1] * T[ ROW1 + c] + 
					M[ ROW[r] + 2] * T[ ROW2 + c] + 
					M[ ROW[r] + 3] * T[ ROW3 + c];
				}
		}

	// Oversets the current Transform with the resultant!
	// = A * B
	void Mul(REAL* A,REAL* B) // 4x4 transforms:
		{
		short r,c;
		// Unroll this puppy!
		// Much optimizing needed!
		for (r = 0;r<3;r++) // 3/4 XFORM!
			for (c=0;c<4;c++)
				{
				T[ ROW[r] + c] =
					A[ ROW[r]] * B[c] + 
					A[ ROW[r] + 1] * B[ ROW1 + c] + 
					A[ ROW[r] + 2] * B[ ROW2 + c] + 
					A[ ROW[r] + 3] * B[ ROW3 + c];
				}
		}

	// Transform an actual point ( overwrites old point )
	// Doex a premultiply!
	void Transform(RP3d &p)
		{
		RP3d temp = {0.0F,0.0F,0.0F,1.0F}; // asume 3 row form!
		REAL *pT = T,*pV;
		short i,j;

		for (j=0;j<3;j++) // asume 3 row form!
			for (i=0,pV = p.v;i<4;i++)
				{
				temp.v[j] += (*pV++) * (*pT++);
				}
		// overwrite original
		for (i=0;i<4;i++) p.v[i] = temp.v[i];
		}

	// Transform an actual point, and places the answer into a different pt
	// Doex a premultiply!
	void TransformInto(RP3d& vSrc, RP3d& vDst)
		{
		vDst.v[0] = vDst.v[1] = vDst.v[2] = REAL(0.);
		vDst.v[3] = REAL(1.);

		REAL *pT = T,*pV;
		short i,j;

		for (j=0;j<3;j++) // asume 3 row form!
			for (i=0,pV = vSrc.v;i<4;i++)
				{
				vDst.v[j] += (*pV++) * (*pT++);
				}
		}

	// Assumes R3 = {0,0,0,1}
	void Trans(REAL x,REAL y,REAL z)
		{
		T[ROW0+3] += x;
		T[ROW1+3] += y;
		T[ROW2+3] += z;
		}

	void Scale(REAL a,REAL b, REAL c)
		{
		for (short i=0;i<4;i++)
			{
			T[ ROW0 + i] *= a;	
			T[ ROW1 + i] *= b;	
			T[ ROW2 + i] *= c;	
			}
		}

	void Rz(short sDeg) // CCW!
		{
		REAL S = rspfSin(sDeg);
		REAL C = rspfCos(sDeg);
		REAL NewVal; // two vertical numbers depend on each other

		for (short i=0;i<4;i++)
			{
			NewVal = T[ ROW0 + i] * C - T[ ROW1 + i] * S;
			T[ ROW1 + i] = T[ ROW0 + i] * S + T[ ROW1 + i] * C;
			T[ ROW0 + i] = NewVal;
			}
		}

	void Rx(short sDeg) // CCW!
		{
		REAL S = rspfSin(sDeg);
		REAL C = rspfCos(sDeg);
		REAL NewVal; // two vertical numbers depend on each other

		for (short i=0;i<4;i++)
			{
			NewVal = T[ ROW1 + i] * C - T[ ROW2 + i] * S;
			T[ ROW2 + i] = T[ ROW1 + i] * S + T[ ROW2 + i] * C;
			T[ ROW1 + i] = NewVal;
			}
		}

	void Ry(short sDeg) // CCW!
		{
		REAL S = rspfSin(sDeg);
		REAL C = rspfCos(sDeg);
		REAL NewVal; // two vertical numbers depend on each other

		for (short i=0;i<4;i++)
			{
			NewVal = T[ ROW0 + i] * C + T[ ROW2 + i] * S;
			T[ ROW2 + i] =-T[ ROW0 + i] * S + T[ ROW2 + i] * C;
			T[ ROW0 + i] = NewVal;
			}
		}

	// a 3d ORTHOGONAL mapping from REAL box1 to box2
	// useful in screen and orthogonal view xforms
	// Use rspSub to create w vertices (w,h,d)
	// x1 BECOMES x2.  Note that w1 must NOT have any 0's.
	//
	void MakeBoxXF(RP3d &x1,RP3d &w1,RP3d &x2,RP3d &w2)
		{
		// NOT OF MAXIMUM SPEED!
		Make1();
		Trans(-x1.x,-x1.y,-x1.z);
		Scale(w2.x/w1.x,w2.y/w1.y,w2.z/w1.z);
		Trans(x2.x,x2.y,x2.z);
		}

	// This is NOT hyper fast, and the result IS a rotation matrix
	// For now, point is it's x-axis and up i s it's y-axis.
	void MakeRotTo(RP3d point,RP3d up)
		{
		RP3d third;

		rspMakeUnit(point);
		rspMakeUnit(up);
		rspCross(third,point,up);
		// store as columns
		Make0();
		T[0 + ROW[0] ] = point.x;
		T[0 + ROW[1] ] = point.y;
		T[0 + ROW[2] ] = point.z;

		T[1 + ROW[0] ] = up.x;
		T[1 + ROW[1] ] = up.y;
		T[1 + ROW[2] ] = up.z;

		T[2 + ROW[0] ] = third.x;
		T[2 + ROW[1] ] = third.y;
		T[2 + ROW[2] ] = third.z;

		}

	// This is NOT hyper fast, and the result IS a rotation matrix
	// For now, point is it's x-axis and up i s it's y-axis.
	void MakeRotFrom(RP3d point,RP3d up)
		{
		RP3d third;

		rspMakeUnit(point);
		rspMakeUnit(up);
		rspCross(third,point,up);
		// store as rows
		Make0();
		T[0 + ROW[0] ] = point.x;
		T[1 + ROW[0] ] = point.y;
		T[2 + ROW[0] ] = point.z;

		T[0 + ROW[1] ] = up.x;
		T[1 + ROW[1] ] = up.y;
		T[2 + ROW[1] ] = up.z;

		T[0 + ROW[2] ] = third.x;
		T[1 + ROW[2] ] = third.y;
		T[2 + ROW[2] ] = third.z;
		}

	// Loads instance data for this Transform from the specified
	// file.
	short Load(				// Returns 0 on success.
		RFile*	pfile)	// In:  Ptr to file to load from.  Must be open with
								// read access.
		{
		// Read the entire matrix in one kerchunk.  Bang!  I mean Kerchunk!
		pfile->Read(T, sizeof(T) / sizeof(T[0]) );
		// Success can be measured in terms of I/O errors.
		// Also, we should not have read past the end yet.
		return pfile->Error() | pfile->IsEOF();
		}

	// Saves instance data for this Transform to the specified
	// file.
	short Save(				// Returns 0 on success.
		RFile*	pfile)	// In:  Ptr to file to save to.  Must be open with
								// write access.
		{
		// Write the entire matrix in one kerchunk.  Bang!  I mean Kerchunk!
		pfile->Write(T, sizeof(T) / sizeof(T[0]) );
		// Success can be measured in terms of I/O errors.
		return pfile->Error();
		}

	};


//===================================
#endif
