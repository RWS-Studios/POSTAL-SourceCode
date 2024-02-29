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

RFixedS32 fpSINQ[csNumRotSteps],fpCOSQ[csNumRotSteps];

void InitTrigFP() // fixed point	
	{
	short i;
	double rad;

	const double cdStepsToRad = 
		rspPI * 2.0 / double(csNumRotSteps);

	for (i=0;i<csNumRotSteps;i++)
		{
		rad = (double)i * cdStepsToRad;
		rspfpSetValue(fpSINQ[i],SINQ[i]);
		rspfpSetValue(fpCOSQ[i],COSQ[i]);
		}
	}

// Auto init:
RQuickTrig	dummyRQuickTrig;

long RInitNum::OneOver[NUM_ONEOVER_FP32];
RInitNum::RInitNum()
	{
	// Populate the oneOver array:
	OneOver[0] = long(2147483647); // error, signed infinity!
	OneOver[1] = long(65535); // full numbers!
	for (short i=2;i<NUM_ONEOVER_FP32;i++) OneOver[i] = long(65536)/i;
	}

RInitNum dummyRInitNum;

