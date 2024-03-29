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
//////////////////////////////////////////////////////////////////////////////
//
// shapes.h
//
// 
// History:
//		12/08/95	BRH	Started.
//
//		11/01/96	JMI	Changed:
//							Old label:		New label:
//							=========		=========
//							CPt				RPt
//							C2DPoint			R2DPoint
//							C3DPoint			R3DPoint
//							CRay				RRay
//							C2DRay			R2DRay
//							C3DRay			R3dRay
//							CRectangle		RRectangle
//							CCube				RCube
//							CCircle			RCircle
//							CSphere			RSphere
//
//		02/17/97	JMI	Removed ~RSphere() proto (had no body).
//
//		02/18/97	JMI	Added empty bodies to ~RRectangle(), ~RCube(), and 
//							~RCircle().
//							Also, added R2DLine and R3DLine.
//
//////////////////////////////////////////////////////////////////////////////
//
//	These shape classes were initially created to be used with the
// regions and in the games.  For most of the shapes there are 2D and 3D
// versions.  The one you choose would be based on the type of game being
// created.  
//
//////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
#include <stdint.h>
#else
#include <cstdlib>
#endif

#ifndef SHAPES_H
#define SHAPES_H


class RPt
{
	public:
		int32_t X;
		int32_t Y;

		RPt() 
			{X=Y=0;};

		RPt(int32_t lX, int32_t lY)
			{ X = lX; Y = lY; }

		~RPt() {};
};

class R2DPoint : public RPt
{
	public:
		R2DPoint(int32_t lX, int32_t lY)
			{X = lX; Y = lY;};
};

class R3DPoint : public RPt
{
	public:
		int32_t Z;

		R3DPoint()
			{X=Y=Z=0;};
		R3DPoint(int32_t lX, int32_t lY, int32_t lZ)
			{X = lX; Y = lY; Z = lZ;};
};


class RRay
{
	public:
		int32_t X;			// X originating position
		int32_t Y;			// Y originating position
		float fXVect;	// X element of unit vector
		float fYVect;	// Y element of unit vector

		// Constructors
		RRay()
			{X=Y=0; fXVect=fYVect=(float) 0.0;};

		// Destructor
		~RRay() {};	
};

class R2DRay : public RRay
{
	public:
		// Constructors
		R2DRay(int32_t lX, int32_t lY, float fXunit, float fYunit)
			{X=lX; Y=lY, fXVect=fXunit; fYVect=fYunit;};
		R2DRay(int32_t lX1, int32_t lY1, int32_t lX2, int32_t lY2);

		// Destructor
		~R2DRay() {};
};

class R3DRay : public RRay
{
	public:
		int32_t Z;			// Z originating position
		float fZVect;	// Z element of unit vector

		// Constructors
		R3DRay()
			{X=Y=Z=0; fXVect=fYVect=fZVect=(float) 0.0;};
		R3DRay(int32_t lX, int32_t lY, int32_t lZ, float fXunit, float fYunit, float fZunit)
			{X=lX; Y=lY; Z=lZ; fXVect=fXunit; fYVect=fYunit; fZVect=fZunit;};
		R3DRay(int32_t lX1, int32_t lY1, int32_t lZ1, int32_t lX2, int32_t lY2, int32_t lZ2);

		// Destructor
		~R3DRay() {};
};

class R2DLine
{
	public:
		int32_t	X1;			// X originating position
		int32_t	Y1;			// Y originating position
		int32_t	X2;			// X ending position
		int32_t	Y2;			// Y ending position

		// Constructors
		R2DLine()
			{X1=Y1=X2=Y2=0; }

		// Destructor
		~R2DLine() {};	
};

class R3DLine : public R2DLine
{
	public:
		int32_t	Z1;			// Z originating position
		int32_t	Z2;			// Z ending position

		// Constructors
		R3DLine()
			{Z1=Z2=0; }

		// Destructor
		~R3DLine() {};	
};

class RRectangle
{
	public:
		int32_t lLeft;		// Left side
		int32_t lRight;	// Right side
		int32_t lTop;		// Top side
		int32_t lBottom;  // Bottom side

		// Constructors
		RRectangle()
			{lLeft=lRight=lTop=lBottom=0;};
		RRectangle(int32_t lL, int32_t lR, int32_t lT, int32_t lB)
			{lLeft=lL; lRight=lR; lTop=lT; lBottom=lB;};

		// Destructor
		~RRectangle() {};
};

class RCube
{
	public:
		int32_t lLeft;		// Left side
		int32_t lRight;	// Right side
		int32_t lTop;		// Top side
		int32_t lBottom;	// Bottom side
		int32_t lFront;	// Front side
		int32_t lBack;		// Back side

		// Constructors
		RCube()
			{lLeft=lRight=lTop=lBottom=lFront=lBack=0;};
		RCube(int32_t lL, int32_t lR, int32_t lT, int32_t lB, int32_t lF, int32_t lBk)
			{lLeft=lL; lRight=lR; lTop=lT; lBottom=lB; lFront=lF; lBack=lBk;};

		// Destructor
		~RCube() {};
};

class RCircle
{
	public:
		int32_t X;			// X coordinate of center point
		int32_t Y;			// Y coordinate of center point
		int32_t lRadius;	// Radius of the circle

		// Constructors
		RCircle()
			{X=Y=lRadius=0;};
		RCircle(int32_t lX, int32_t lY, int32_t lR)
			{X=lX; Y=lY; lRadius=lR;};

		// Destructor
		~RCircle() {};
};

class RSphere
{
	public:
		int32_t X;			// X coordinate of center point
		int32_t Y;			// Y coordinate of center point
		int32_t Z;			// Z coordinate of center point
		int32_t lRadius;	// Radius of the sphere

		// Constructors
		RSphere()
			{X=Y=Z=lRadius=0;};
		RSphere(int32_t lX, int32_t lY, int32_t lZ, int32_t lR)
			{X=lX; Y=lY; Z=lZ; lRadius=lR;};
};




#endif //SHAPES_H

//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
