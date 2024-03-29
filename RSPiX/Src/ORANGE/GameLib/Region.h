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
// region.h
//
// 
// History:
//		12/08/95	BRH	Started.
//
//		06/10/96	JMI	Made all functions in CRegion pure virtual since they
//							cannot be implemented not knowing anything about the
//							shape of a region.  CRegion is by nature a pure virtual
//							class when it comes to any function that requires know-
//							ledge of that region's actual shape.  Also fixed some
//							typos (e.g., Regtangle->Rectangle, COLLIDE->COLLISION)
//							and added some prototypes.
//
//		10/31/96 BRH	Changed the prefixes on these classes from C to R to
//							conform to the new RSPiX naming convention.
//
//		11/01/96	JMI	Changed:
//							Old label:		New label:
//							=========		=========
//							CRectangle		RRectangle
//							CCube				RCube
//
//		02/17/97	JMI	Changed protected shape members to public and #if 0'd
//							out pure virtuals in base class for now.
//							Also, added Collide overload for R3DLine*.
//
//
//////////////////////////////////////////////////////////////////////////////
//
// This is the base class for regions which can be used for sprite 
//	collision detection among other things.  We may provide several different
//	types of regions to suit particular types of games.  Circular or Spherical
// regions make checking easier for guy based games or 3D mapped games but
// the traditional collision box may suit other types of games better, like
// cars for example.  
//
//////////////////////////////////////////////////////////////////////////////

#ifndef REGION_H
#define REGION_H


// Blue include files
#include "Blue.h"

// If PATHS_IN_INCLUDES macro is defined, we can utilized relative
// paths to a header file.  In this case we generally go off of our
// RSPiX root directory.  System.h MUST be included before this macro
// is evaluated.  System.h is the header that, based on the current
// platform (or more so in this case on the compiler), defines 
// PATHS_IN_INCLUDES.  Blue.h includes system.h so you can include that
// instead.
#ifdef PATHS_IN_INCLUDES
	// Orange include files
	#include "ORANGE/GameLib/Shapes.h"
#else
	// Orange include files
	#include "shapes.h"
#endif // PATHS_IN_INCLUDES


#define	NO_COLLISION	0
#define	COLLISION		1


class RRegion
{
	protected:
		uint16_t usType;	// User defined region type
		int32_t m_lAbsX;	// Absolute X position
		int32_t m_lAbsY;	// Absolute Y position
		int32_t m_lAbsZ;	// Absolute Z position

	public:
		// Constructor
		RRegion()
			{usType=0; m_lAbsX = m_lAbsY = m_lAbsZ = 0;};

		// Destructor
		~RRegion()
			{ }

		// Query functions
//		virtual short Collide(RPt* pPoint);
//		virtual short Collide(RRay* pRay);
//		virtual short Collide(RRegion* pRegion);

#if 0	// This seems cool and all, but if it is to be done
		// we'll need some Runtime type resolution whether
		// it be RTTI or just the ol' enums we've done
		// for other RSPiX stuff.
		// For now, these just get in the way.
		virtual short Collide(RPt* pPoint)			= 0;
		virtual short Collide(RRay* pRay)			= 0;
		virtual short Collide(RRegion* pRegion)	= 0;
#endif
};

// Forward declaration
class RRectangularRegion;
class RCubicRegion;

class RCircularRegion : virtual public RRegion
{
	public:
		RCircle	circle;

	public:
		// Constructors
		RCircularRegion();
		RCircularRegion(RCircle* pCircle);
		RCircularRegion(int32_t lX, int32_t lY, int32_t lR);
		RCircularRegion(int32_t lX, int32_t lY, 
				                           int32_t lXCirc, int32_t lYCirc, 
													int32_t lRadius);
		RCircularRegion(R2DPoint* pPoint, RCircle* pCircle);

		// Destructor
		~RCircularRegion()
			{};

		// Query function to tell if a point (x,y) is in the circle
		int16_t Collide(R2DPoint* pPoint);

		// Query function to tell if a line will intersect
		int16_t Collide(R2DRay* pRay);

		// Query function to tell if two circular regions collide
		int16_t Collide(RCircularRegion* pCircRegion);

		// Query function to tell if this circular region collides with a rectangle
		int16_t Collide(RRectangularRegion* pRectRegion);
};

class RSphericalRegion : virtual public RRegion
{
	public:
		RSphere	sphere;
		
	public:
		// Constructors
		RSphericalRegion();
		RSphericalRegion(RSphere* pSphere);
		RSphericalRegion(int32_t lX, int32_t lY, int32_t lZ, int32_t lR);
		RSphericalRegion(int32_t lX, int32_t lY, int32_t lZ,
							  int32_t lXSphere, int32_t lYSphere, int32_t lZSphere,
							  int32_t lRadius);
		RSphericalRegion(R3DPoint* pPoint, RSphere* pSphere);
		
		// Destructor
		~RSphericalRegion()
			{};
		
		// Query function to tell if a point (x,y,z) is in the sphere
		int16_t Collide(R3DPoint* pPoint);

		// Query function to tell if a line will intersect
		int16_t Collide(R3DRay* pRay);

		// Query function to tell if a line intersects.
		int16_t Collide(R3DLine* pline);

		// Query function to tell if two spherical regions collide
		int16_t Collide(RSphericalRegion* pSphereRegion);

		// Query function to tell if this sphere collides with a cube
		int16_t Collide(RCubicRegion* pCubeRegion);

		// Query function to tell if this sphere collides with a generic region.
};

class RRectangularRegion : virtual public RRegion
{
	public:
		RRectangle	rect;

	public:
		// Constructors
		RRectangularRegion();
		RRectangularRegion(RPt* pPoint, RRectangle* pRect);
		RRectangularRegion(int32_t lX, int32_t lY, 
		                   int32_t lLeft, int32_t lRight, int32_t lTop, int32_t lBottom);

		// Destructor
		~RRectangularRegion()
			{};

		// Query function to tell if a point (x,y) is in the rectangle
		int16_t Collide(RPt* pPoint);
		// Query function to tell is a point (lX, lY) is in the rectangle.
		int16_t Collide(int32_t lX, int32_t lY);

		// Query function to tell if a line will intersect
		int16_t Collide(R2DRay* pRay);

		// Query function to tell if two rectangular regions collide
		// The default bFullCheck = FALSE will cause the function to
		// return COLLIDE or NO_COLLIDE, but will not bother to check
		// to see if one region fully encloses the other.
		int16_t Collide(RRectangularRegion* pRectRegion, int16_t bFullCheck = FALSE);

		// Query function to tell if this rectangle collides with a circle
		int16_t Collide(RCircularRegion* pCircRegion, int16_t bFullCheck = FALSE);
};

class RCubicRegion : virtual public RRegion
{
	public:
		RCube cube;

	public:
		// Constructors
		RCubicRegion();
		RCubicRegion(R3DPoint* pPoint, RCube* pCube);
		RCubicRegion(int32_t lX, int32_t lY, int32_t lLeft, int32_t lRight, 
		             int32_t lTop, int32_t lBottom, int32_t lFront, int32_t lBack);
		RCubicRegion(int32_t lX, int32_t lY, int32_t lZ, int32_t lLeft, int32_t lRight, 
						 int32_t lTop, int32_t lBottom, int32_t lFront, int32_t lBack);

		// Destructor
		~RCubicRegion()
			{};

		// Query function to tell if a point (x,y,z) is in the cube
		int16_t Collide(R3DPoint* pPoint);

		// Query function to tell if a line will intersect
		int16_t Collide(R3DRay* pRay);

		// Query function to tell if two cubic regions collide
		int16_t Collide(RCubicRegion* pCubeRegion);

		// Query function to tell if this cube collides with a sphere
		int16_t Collide(RSphericalRegion* pSphereRegion);
};




#endif //REGION_H

//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
