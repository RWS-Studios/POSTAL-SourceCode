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
// This function should draw 2d and 3d lines, and just keep getting better.
#include <stdlib.h>
#include "System.h"
#ifdef PATHS_IN_INCLUDES
	#include "GREEN/BLiT/BLIT.H"
	//#include <math.h>
	//#include "numbers.h" // allow
	//#include "_QuickTrig.h"  
	//#include "_line.h"
	#include "ORANGE/QuickMath/QuickMath.h"
	#include "ORANGE/QuickMath/FixedPoint.h"
	#include "ORANGE/QuickMath/Fractions.h"
#else
	#include "BLIT.H"
	//#include <math.h>
	//#include "numbers.h" // allow
	//#include "_QuickTrig.h"  
	//#include "_line.h"
	#include "QuickMath.h"
	#include "FixedPoint.h"
	#include "Fractions.h"
#endif

#ifdef MOBILE //Arm RAND_MAX is a full int, code expecting a short!!
#define RAND_MAX 0x7fff
#endif

// Here's a very quick rip off!
// I don't even know if it's 2 way consistent!
// (It sure doesn't clip!)
// VERY cheezy implementation!  For drawing, should do IC stuff inline with pDst
//
void rspLine(UCHAR ucColor,RImage* pimDst,short sX1,short sY1,short sX2,short sY2,const RRect* prClip)
	{
	/*
#ifdef _DEBUG
	if ((sX1 < 0) || (sY1 < 0) || (sX2 >= pimDst->lWidth) || (sY2 >= pimDst->lHeight))
		{
		TRACE("line: Error... Clipped line!\n");
		return;
		}
#endif
		*/

	// use the cheap 3D technique with signed fractions:
	short sDelX = sX2 - sX1; // signed
	short sDelY = sY2 - sY1; // signed
	short sDelZ = MAX(ABS(sDelX),ABS(sDelY)); // a slow trick for now...
	if (sDelZ == 0) // a single point
		{
		rspClipPlot(ucColor,pimDst,sX1,sY1,prClip);
		return;
		}

	RFracS16 frIncX,frIncY;
	rspfrDiv(frIncX,sDelX,sDelZ); // Magnitude < 1
	rspfrDiv(frIncY,sDelY,sDelZ); // Magnitude < 1

	RFracS16 frX,frY;

	frX.mod = sX1; frY.mod = sY1;
	frX.frac = frY.frac = (sDelZ >> 1); // for pixel rounding

	for (short i=0;i < sDelZ;i++)
		{
		rspClipPlot(ucColor,pimDst,frX.mod,frY.mod,prClip);
		rspfrAdd(frX,frIncX,sDelZ);
		rspfrAdd(frY,frIncY,sDelZ);
		}
	}

// returns a short random number between 0 and N-1
//
short rspRand(short sMax)
	{
	return (short)((rand() * sMax) / RAND_MAX);

	}

