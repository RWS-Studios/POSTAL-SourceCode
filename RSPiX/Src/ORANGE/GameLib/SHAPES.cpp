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
// shapes.cpp
//
// 
// History:
//		12/08/95	BRH	Started.
//		12/11/95	BRH	Added Ray constructor that takes two points and
//							calculates the unit vector for the ray.
//
//		11/04/96	JMI	Changed:
//							Old label:		New label:
//							=========		=========
//							C2DRay			R2DRay
//							C3DRay			R3dRay
//
//////////////////////////////////////////////////////////////////////////////

#include "Shapes.h"

//////////////////////////////////////////////////////////////////////////////
//
// Constructor
//
// Description:
//		Takes two points and calculates the unit vector for the ray
//
// Parameters:
//		lX1 = x coordinate of originating point
//		lY2 = y coordinate of originating point
//		lX2 = x coordinate in direction of ray
//		lY2 = y coordinate in direction of ray
//
// Returns:
//		none
//
//////////////////////////////////////////////////////////////////////////////

R2DRay::R2DRay(long lX1, long lY1, long lX2, long lY2)
{
	X = lX1;
	Y = lY1;

	fXVect = (float) lX2 - lX1;
	fYVect = (float) lY2 - lY1;

	if (fXVect > fYVect)
	{
		fYVect /= fXVect;
		fXVect = (float) 1.0; //dx /= dx;
	}
	else
	{
		fXVect /= fYVect;
		fYVect = (float) 1.0; //dy /= dy;
	}
}

R3DRay::R3DRay(long lX1, long lY1, long lZ1, long lX2, long lY2, long lZ2)
{
	X = lX1;
	Y = lY1;
	Z = lZ1;

	fXVect = (float) lX2 - lX1;
	fYVect = (float) lY2 - lY1;
	fZVect = (float) lZ2 - lZ1;

	if (fXVect > fYVect && fXVect > fZVect)
	{
		fYVect /= fXVect;
		fZVect /= fXVect;
		fXVect = (float) 1.0; // fXVect /= fXVect;
	}
	else
		if (fYVect > fXVect && fYVect > fZVect)
		{
			fXVect /= fYVect;
			fZVect /= fYVect;
			fYVect = (float) 1.0; // fYVect /= fYVect;
		}
		else
		{
			fXVect /= fZVect;
			fYVect /= fZVect;
			fZVect = (float) 1.0; // fZVect /= fZVect;
		}
}

//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////

