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
// region.cpp
//
// 
// History:
//		12/08/95	BRH	Started.
//
//		06/10/96	JMI	Added some "return NO_COLLISION;" (i.e., virtually empty) 
//							function bodies for those that were actually empty.
//							Also added TRACE messages stating that the function was
//							NYI.
//							Commented out some unused locals.  
//							Took some functions that were defined here and added
//							their protos to region.h.
//
//		10/27/96 MJR	Fixed "unused variable" warnings.
//
//		10/31/96 BRH	Changed the class prefix C to R to match the new
//							RSPiX naming convetion.
//
//		11/04/96	JMI	Changed CRectangle to RRectangle and CCircle to RCircle.
//
//		02/18/97	JMI	Filled in Spherical region's collide with 3D ray utilizing
//							super fast code fragment del Jeff.
//
//		02/18/97	JMI	Changed the 3DLine intersection with a sphere back to
//							2D on X and Z.
//
//		02/19/97	JMI	Was doing subtractions backwards in determining line's
//							end point's distance from its start point in 3DLine
//							intersection with sphere.
//
//		02/19/97 BRH	Fixed bug in spherical region collision detect where a
//							+ sign was typoed as = causing the distance to be off. 
//
//		06/28/97 MJR	Changed __int64 to S64 for mac compatability.
//
//////////////////////////////////////////////////////////////////////////////
#include <math.h>


#include "Region.h"

//////////////////////////////////////////////////////////////////////////////
//
// Constructors
//
// Description:
//		Three Constructors for RRectangularRegion
//
// Parameters:
//		type 1:	none
//		type 2:  pRect = pointer to rectangle that will be used to initialize
//					the rectangular region 
//		type 3:	lLeft, lRight, lTop, lBottom = Relative positions from the
//					region's m_lAbs values
//
// Returns:
//		none
//
//////////////////////////////////////////////////////////////////////////////

RRectangularRegion::RRectangularRegion()
{
	m_lAbsX = m_lAbsY = m_lAbsZ = 0;
	rect.lLeft = rect.lRight = rect.lTop = rect.lBottom = 0;
}

RRectangularRegion::RRectangularRegion(RPt* pPoint, RRectangle* pRect)
{
	m_lAbsX = pPoint->X;
	m_lAbsY = pPoint->Y;
	m_lAbsZ = 0;
	rect.lLeft = pRect->lLeft;
	rect.lRight = pRect->lRight;
	rect.lTop = pRect->lTop;
	rect.lBottom = pRect->lBottom;
}

RRectangularRegion::RRectangularRegion(long lX, long lY,
                    long lLeft, long lRight, long lTop, long lBottom)
{
	m_lAbsX = lX;
	m_lAbsY = lY;
	m_lAbsZ = 0;
	rect.lLeft = lLeft;
	rect.lRight = lRight;
	rect.lTop = lTop;
	rect.lBottom = lBottom;
}

//////////////////////////////////////////////////////////////////////////////
//
// Collide (point)
//
// Description:
//		Checks to see if the given point is in the region
//
// Parameters:
//		pPoint = ponter to RPt to be checked against the region
// or
//		lX, lY = X,Y of point to be checked
//
// Returns:
//		NO_COLLISION if the point is not in or on the rectangle
//		COLLISION if it is
//
//////////////////////////////////////////////////////////////////////////////

short RRectangularRegion::Collide(long lX, long lY)
{
	RPt point(lX, lY);
	return Collide(&point);
}

short RRectangularRegion::Collide(RPt* pPoint)		
{
	if (pPoint->X >= rect.lLeft + m_lAbsX  &&
	    pPoint->X <= rect.lRight + m_lAbsX &&
		 pPoint->Y >= rect.lTop + m_lAbsY   &&
		 pPoint->Y <= rect.lBottom + m_lAbsY)
		return COLLISION;
	else
		return NO_COLLISION;
}

short RRectangularRegion::Collide(R2DRay* /*pRay*/)
{
	TRACE("RRectangularRegion::Collide(R2DRay* pRay,...): NYI!\n");
	short sReturn = NO_COLLISION;

//	long fx1, fx2, fy1, fy2;

	// Check Left


	// Check Right
	if (sReturn == NO_COLLISION)
	{

	}

	// Check Bottom
	if (sReturn == NO_COLLISION)
	{

	}

	// Check Top
	if (sReturn == NO_COLLISION)
	{

	}

	return sReturn;
}

short RRectangularRegion::Collide(RRectangularRegion* pRegion, short /*bFullCheck*/)
{
	if (rect.lRight + m_lAbsX < pRegion->rect.lLeft + pRegion->m_lAbsX)
		return NO_COLLISION;
	if (rect.lLeft + m_lAbsX > pRegion->rect.lRight + pRegion->m_lAbsX)
		return NO_COLLISION;
	if (rect.lTop + m_lAbsY > pRegion->rect.lBottom + pRegion->m_lAbsY)
		return NO_COLLISION;
	if (rect.lBottom + m_lAbsY < pRegion->rect.lTop + pRegion->m_lAbsY)
		return NO_COLLISION;
	else
		return COLLISION;
}

short RRectangularRegion::Collide(RCircularRegion* /*pRegion*/, short /*bFullCheck*/)
	{
	TRACE("RRectangularRegion::Collide(RCircularRegion* pRegion,...): NYI!\n");

	return NO_COLLISION;
	}


//////////////////////////////////////////////////////////////////////////////
//
// Constructors
//
// Description:
//		Three constructors for Cubic Regions
//
// Parameters:
//		type 1:	none
//		type 2:  pPoint = originating point of region to which the sides
//					         of the cube are relative
//					pCube = RCube object used to initialize the region's cube
//		type 3:	lX, lY = originating point of region to which the sides of
//								the cube are relative
//					lLeft, lRight, lTop, lBottom, lFront, lBack = 
//						Relative positions from the region's m_lAbs values
//
// Returns:
//		none
//
//////////////////////////////////////////////////////////////////////////////

RCubicRegion::RCubicRegion()
{
	m_lAbsX = m_lAbsY = m_lAbsZ = 0;
	cube.lLeft = cube.lRight = cube.lTop = 
	cube.lBottom = cube.lFront = cube.lBack = 0;
}

RCubicRegion::RCubicRegion(R3DPoint* pPoint, RCube* pCube)
{
	m_lAbsX = pPoint->X;
	m_lAbsY = pPoint->Y;
	m_lAbsZ = pPoint->Z;
	cube.lLeft = pCube->lLeft;
	cube.lRight = pCube->lRight;
	cube.lTop = pCube->lTop;
	cube.lBottom = pCube->lBottom;
	cube.lFront = pCube->lFront;
	cube.lBack = pCube->lBack;
}

RCubicRegion::RCubicRegion(long lX, long lY, long lZ, long lLeft, long lRight, 
                           long lTop, long lBottom, long lFront, long lBack)
{
	m_lAbsX = lX;
	m_lAbsY = lY;
	m_lAbsZ = lZ;
	cube.lLeft = lLeft;
	cube.lRight = lRight;
	cube.lTop = lTop;
	cube.lBottom = lBottom;
	cube.lFront = lFront;
	cube.lBack = lBack;
}

//////////////////////////////////////////////////////////////////////////////

short RCubicRegion::Collide(R3DPoint* pPoint)
{
	if (pPoint->X < cube.lLeft + m_lAbsX)
		return NO_COLLISION;
	if (pPoint->X > cube.lRight + m_lAbsX)
		return NO_COLLISION;
	if (pPoint->Y < cube.lTop + m_lAbsY)
		return NO_COLLISION;
	if (pPoint->Y > cube.lBottom + m_lAbsY)
		return NO_COLLISION;
	if (pPoint->Z < cube.lFront + m_lAbsZ)
		return NO_COLLISION;
	if (pPoint->Z > cube.lBack + m_lAbsZ)
		return NO_COLLISION;
	else
		return COLLISION;
}

short RCubicRegion::Collide(R3DRay* /*pRay*/)
	{
	TRACE("RCubicRegion::Collide(R3DRay* pRay,...): NYI!\n");
	return NO_COLLISION;
	}

short RCubicRegion::Collide(RCubicRegion* pRegion)
{
	if (cube.lRight + m_lAbsX < pRegion->cube.lLeft + pRegion->m_lAbsX)
		return NO_COLLISION;
	if (cube.lLeft + m_lAbsX > pRegion->cube.lRight + pRegion->m_lAbsX)
		return NO_COLLISION;
	if (cube.lTop + m_lAbsY > pRegion->cube.lBottom + pRegion->m_lAbsY)
		return NO_COLLISION;
	if (cube.lBottom + m_lAbsY < pRegion->cube.lTop + pRegion->m_lAbsY)
		return NO_COLLISION;
	if (cube.lBack + m_lAbsZ < pRegion->cube.lFront + pRegion->m_lAbsZ)
		return NO_COLLISION;
	if (cube.lFront + m_lAbsZ > pRegion->cube.lBack + pRegion->m_lAbsZ)
		return NO_COLLISION;
	else
		return COLLISION;
	
}

short RCubicRegion::Collide(RSphericalRegion* /*pRegion*/)
	{
	TRACE("RCubicRegion::Collide(RSphericalRegion* pSpherical,...): NYI!\n");
	return NO_COLLISION;
	}


//////////////////////////////////////////////////////////////////////////////
//
// Constructors
//
// Description:
//		Three Constructors for RCircularRegion
//
// Parameters:
//		type 1:	none
//		type 2:	pPoint = originating point of region to which the center of
//								the circle is relative
//					pCircle = RCircle object used to initialize the region's circle
//		type 3:	lX, lY = originative point of region to which the center of
//								the circle is relative
//					lXCirc, lYCirc = center of the circle relative to the region's
//										  position
//					lRadius = Radius of the circle
//
// Returns:
//		none
//
//////////////////////////////////////////////////////////////////////////////

RCircularRegion::RCircularRegion()
{
	m_lAbsX = m_lAbsY = m_lAbsZ = 0;
	circle.X = circle.Y = circle.lRadius = 0;
}

RCircularRegion::RCircularRegion(R2DPoint* pPoint, RCircle* pCircle)
{
	m_lAbsX = pPoint->X;
	m_lAbsY = pPoint->Y;
	circle.X = pCircle->X;
	circle.Y = pCircle->Y;
	circle.lRadius = pCircle->lRadius;
}

RCircularRegion::RCircularRegion(long lX, long lY, 
                                 long lXCirc, long lYCirc, long lRadius)
{
	m_lAbsX = lX;
	m_lAbsY = lY;
	circle.X = lXCirc;
	circle.Y = lYCirc;
	circle.lRadius = lRadius;
}

//////////////////////////////////////////////////////////////////////////////

short RCircularRegion::Collide(R2DPoint* pPoint)
{
	long dx = pPoint->X - circle.X;
	long dy = pPoint->Y - circle.Y;
	long d2 = dx*dx + dy*dy;
	if (d2 > circle.lRadius*circle.lRadius)
		return NO_COLLISION;
	else
		return COLLISION;
}

short RCircularRegion::Collide(R2DRay* /*pRay*/)
	{
	TRACE("RCircularRegion::Collide(R2DRay* pRay,...): NYI!\n");
	return NO_COLLISION;
	}

short RCircularRegion::Collide(RCircularRegion* pRegion)
{
	long dx = circle.X - pRegion->circle.X;
	long dy = circle.Y - pRegion->circle.Y;
	long d2 = dx*dx + dy*dy;
	long r2 = circle.lRadius + pRegion->circle.lRadius;
	r2 *= r2;
	if (d2 > r2)
		return NO_COLLISION;
	else
		return COLLISION;
}

short RCircularRegion::Collide(RRectangularRegion* /*pRegion*/)
	{
	TRACE("RCircularRegion::Collide(RRectangularRegion* pRegion,...): NYI!\n");
	// same as ray but one for each side of the rectangle
	return NO_COLLISION;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Constructors
//
// Description:
//		Three Constructors for RSphericalRegion
//
// Parameters:
//		type 1:	none
//		type 2:	pPoint = originating point of region to which the center of
//								the circle is relative
//					pSphere = RSphere object used to initialize the region's sphere
//		type 3:	lX, lY, lZ = originative point of region to which the center of
//								    the circle is relative
//					lXSphere, lYSphere, lZSphere = center of sphere relative to
//															 the region's position
//					lRadius = radius of the sphere
//
// Returns:
//		none
//
//////////////////////////////////////////////////////////////////////////////

RSphericalRegion::RSphericalRegion()
{
	m_lAbsX = m_lAbsY = m_lAbsZ = 0;
	sphere.X = sphere.Y = sphere.Z = sphere.lRadius = 0;
}

RSphericalRegion::RSphericalRegion(R3DPoint* pPoint, RSphere* pSphere)
{
	m_lAbsX = pPoint->X;
	m_lAbsY = pPoint->Y;
	m_lAbsZ = pPoint->Z;
	sphere.X = pSphere->X;
	sphere.Y = pSphere->Y;
	sphere.Z = pSphere->Z;
	sphere.lRadius = pSphere->lRadius;
}

RSphericalRegion::RSphericalRegion(long lX, long lY, long lZ,
											  long lXSphere, long lYSphere, long lZSphere,
											  long lRadius)
{
	m_lAbsX = lX;
	m_lAbsY = lY;
	m_lAbsZ = lZ;
	sphere.X = lXSphere;
	sphere.Y = lYSphere;
	sphere.Z = lZSphere;
	sphere.lRadius = lRadius;
}

//////////////////////////////////////////////////////////////////////////////

short RSphericalRegion::Collide(R3DPoint* pPoint)
{
	long dx = sphere.X + m_lAbsX - pPoint->X;
	long dy = sphere.Y + m_lAbsY - pPoint->Y;
	long dz = sphere.Z + m_lAbsZ - pPoint->Z;
	long r2 = sphere.lRadius * sphere.lRadius;
	long d2 = dx*dx + dy*dy + dz*dz;
	if (d2 > r2)
		return NO_COLLISION;
	else
		return COLLISION;
}

short RSphericalRegion::Collide(R3DRay* pRay)
	{
	TRACE("RSphericalRegion::Collide(): NYI!\n");

	return NO_COLLISION;
	}

// Query function to tell if a line intersects.
short RSphericalRegion::Collide(R3DLine* pline)
	{
#if 0		// Currently, I don't understand the input
			// params and/or also I noticed that this
			// seems to be a circle vs line function...
			// Is the jump to 3D trivial?
			// Also, I think there's a typo where
			// it says 'long lCirDist = SQR(lRelCenX) + SQR(lRelCenX);'
			// I'm assuming that the second lRelCenX should be lRelCenY.
			// Blech...making attempt to take to 3D.
	//***************************** FUNCTION UINPUT **
	long lRelCenX = sphere.X - pline->X1;
	long lRelCenY = sphere.Y - pline->Y1;
	// JMI: Added Z component:
	long lRelCenZ = sphere.Z - pline->Z1;

	long lDelX = (pline->X1 - pline->X2);
	long lDelY = (pline->Y1 - pline->Y2); 
	// JMI: Added Z component:
	long lDelZ = (pline->Z1 - pline->Z2);
	short sCirR = sphere.lRadius;	// the circle radius
	//***********************************************

	// DO RANGE CHECKS FIRST SINCE THEY ARE LESS EXPENSIVE:

	// 1) is it in my hemisphere?
	// JMI: Added Z component:
	if ( (lDelX * lRelCenX + lDelY * lRelCenY + lDelZ * lRelCenZ) >= 0)
		{

		// 2) is it in my range?:
		// JMI: Added Z component:
		long lCirDist = SQR(lRelCenX) + SQR(lRelCenY) + SQR(lRelCenZ);
		// JMI: Added Z component:
		long	lLineLen = SQR(lDelX) + SQR(lDelY) + SQR(lDelZ);

		// OVERHEAD for range check:
		double dCirLen = sqrt(double(lCirDist));

		if (lLineLen > (lCirDist + SQR(sCirR) - 2*sCirR * dCirLen) )
			{
			long a = lLineLen;
			// JMI: Added Z component:
			long b = -2 * (lRelCenX * lDelX + lRelCenY * lDelY + lDelZ * lRelCenZ);
			long c = lCirDist - SQR(sCirR);
			__int64 d = (__int64(b)*__int64(b) - __int64(4*a)*__int64(c));

			if (d >= 0)
				{
				//***********************************************
				// WE HAVE A DIRECT HIT !!!!!!!!!!!!!!!!!!!
				//***********************************************

				// DO WHATEVER!
				return COLLISION;
				}
			}

		}
#else	// 2D version on X and Z.
	//***************************** FUNCTION UINPUT **
	long lRelCenX = sphere.X - pline->X1;
//	long lRelCenY = sphere.Y - pline->Y1;
	// JMI: Added Z component:
	long lRelCenZ = sphere.Z - pline->Z1;

	long lDelX = (pline->X2 - pline->X1);
//	long lDelY = (pline->Y2 - pline->Y1); 
	// JMI: Added Z component:
	long lDelZ = (pline->Z2 - pline->Z1);
	short sCirR = sphere.lRadius;	// the circle radius
	//***********************************************

	// DO RANGE CHECKS FIRST SINCE THEY ARE LESS EXPENSIVE:

	// 1) is it in my hemisphere?
	// JMI: Added Z component:
	if ( (lDelX * lRelCenX/* + lDelY * lRelCenY*/ + lDelZ * lRelCenZ) > 0)
		{

		// 2) is it in my range?:
		// JMI: Added Z component:
		long lCirDist = SQR(lRelCenX)/* + SQR(lRelCenY)*/ + SQR(lRelCenZ);
		// JMI: Added Z component:
		long	lLineLen = SQR(lDelX)/* + SQR(lDelY)*/ + SQR(lDelZ);

		// OVERHEAD for range check:
		double dCirLen = sqrt(double(lCirDist));

		if (lLineLen > (lCirDist + SQR(sCirR) - 2*sCirR * dCirLen) )
			{
			long a = lLineLen;
			// JMI: Added Z component:
			long b = -2 * (lRelCenX * lDelX + /*lRelCenY * lDelY*/ + lDelZ * lRelCenZ);
			long c = lCirDist - SQR(sCirR);
			S64 d = (S64(b)*S64(b) - S64(4*a)*S64(c));

			if (d >= 0)
				{
				//***********************************************
				// WE HAVE A DIRECT HIT !!!!!!!!!!!!!!!!!!!
				//***********************************************

				// DO WHATEVER!
				return COLLISION;
				}
			}

		}
#endif

	return NO_COLLISION;
	}

short RSphericalRegion::Collide(RSphericalRegion* pRegion)
{
	long dx = sphere.X + m_lAbsX - pRegion->sphere.X + pRegion->m_lAbsX;
	long dy = sphere.Y + m_lAbsY - pRegion->sphere.Y + pRegion->m_lAbsY;
	long dz = sphere.Z + m_lAbsZ - pRegion->sphere.Z + pRegion->m_lAbsZ;
	long d2 = dx*dx + dy*dy + dz*dz;
	long r2 = sphere.lRadius + pRegion->sphere.lRadius;
	r2 *= r2;
	if (d2 > r2)
		return NO_COLLISION;
	else
		return COLLISION;
}

short RSphericalRegion::Collide(RCubicRegion* /*pRegion*/)
	{
	TRACE("RSphericalRegion::Collide(RCubicRegion* pRegion,...): NYI!\n");
	// Same as ray but one for each side of the cube
	return NO_COLLISION;
	}

//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
