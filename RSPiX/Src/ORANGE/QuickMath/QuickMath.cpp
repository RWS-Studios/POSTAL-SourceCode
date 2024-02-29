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
#include <math.h>
#include "QuickMath.h"
#include "FixedPoint.h"

double SINQ[csNumRotSteps],COSQ[csNumRotSteps];
float fSINQ[csNumRotSteps],fCOSQ[csNumRotSteps];
short ATANQ[60];
short	SQRTQ[MAX_FAST_SQRT];

short rspATan(short sDeltaY,short sDeltaX)
	{
	short sDelX,sDelY,sDeg; // absolute versions

	if (sDeltaX < 0) sDelX = -sDeltaX;
	else sDelX = sDeltaX;

	if (sDeltaY < 0) sDelY = -sDeltaY;
	else sDelY = sDeltaY;

	if (sDelY <= sDelX)
		if (sDelX != 0)
			sDeg = ATANQ[
			long(0.5 + (rspRadToDeg * sDelY) / sDelX)];
		else
			sDeg = 90;
	else
		if (sDelY) sDeg = 90 - ATANQ[
			long(0.5 + (rspRadToDeg * sDelX) / sDelY)];
		else sDeg = 90;

	// Keep in bounds
	if (sDeltaX < 0) sDeg = 180 - sDeg;
	if (sDeltaY < 0) sDeg = 360 - sDeg;
	if (sDeg == 360) sDeg = 0;

	return sDeg;
	}

short rspATan(double dVal)
	{
	short sDeg; // absolute versions
	double dAbsVal;

	if (dVal < 0) dAbsVal = -dVal;
	else dAbsVal = dVal;

	if (dAbsVal <= 1.0)
		sDeg = ATANQ[
		long(0.5 + rspRadToDeg * dAbsVal)];
	else
		sDeg = 90 - ATANQ[
			long(0.5 + rspRadToDeg  / dAbsVal)];

	// Keep in bounds
	if (dVal < 0.0) sDeg = 360 - sDeg;
	if (sDeg == 360) sDeg = 0;

	return sDeg;
	}

void InitTrig()
	{
	short i;
	double rad;

	const double cdStepsToRad = 
		rspPI * 2.0 / double(csNumRotSteps);

	for (i=0;i<csNumRotSteps;i++)
		{
		rad = (double)i * cdStepsToRad;
		SINQ[i] = (double)sin(rad);
		COSQ[i] = (double)cos(rad);
		fSINQ[i] = (float) SINQ[i];
		fCOSQ[i] = (float) COSQ[i];
		}

	// Set up Arctan 45 degrees:
	for (i=0;i<58;i++)
		{
		ATANQ[i] = short(0.5 + atan( rspDegToRad * i ) *
												rspRadToDeg);
		}

	long l;
	for (l=0; l < MAX_FAST_SQRT; l++)
		{
		SQRTQ[l] = short(sqrt(double(l)));
		}

	TRACE("QTRIG initialized!\n"); 	
	RQuickTrigFP	dummy;
	}

