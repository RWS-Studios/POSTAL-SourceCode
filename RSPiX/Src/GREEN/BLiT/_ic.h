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
I AM ARCHAIC - INCLUDE ORANGE/QUICKMATH/FIXEDPOINT.H instead!

// This is an internal BLiT header.  Don't get confused by it - it's not up to user level yet.
#ifndef ic_h
#define ic_h

#include "System.h"

//==========================  Here are some fraction inlines...

// add two fractions of identical denominators...
// both fraction MUST be PROPER! 
//
inline void	Add(u16Frac& pDst,u16Frac& pAdd,short sDen)
	{
	pDst.delta += pAdd.delta;
	if ( (pDst.frac += pAdd.frac) >= sDen)
		{
		pDst.delta++; // ONLY adding proper fractions
		pDst.frac -= sDen;
		}
	}

// Creates a proper fraction from an improper one:
// The sizes MUST be appropriate!
//
inline void MakeProper(u16Frac& pDst,USHORT usNum,USHORT usDen)
	{
	pDst.delta = usNum / usDen;
	pDst.frac = usNum % usDen;
	//pDst.frac = usNum - pDst.delta * usDen;// overflow problem
	}

// Creates an array of 256 fractions, uch that 0 is 0,
// 1 is tha base fraction, and 255 is 255 * the base fraction.
// Both must be unsigned!  (uses calloc)
//
inline u16Frac* u16fStrafe256(USHORT usNum,USHORT usDen)
	{
	u16Frac* pu16fNew = (u16Frac*) calloc(256,sizeof(u16Frac));
	u16Frac u16fInc;
	MakeProper(u16fInc,usNum,usDen); // the 2 part delta

	ULONG ulNumInc = 0;
	for (short i = 1; i < 256 ; i++)
		{
		pu16fNew[i].delta = pu16fNew[i-1].delta + u16fInc.delta;
		pu16fNew[i].frac = pu16fNew[i-1].frac + u16fInc.frac;
		if (pu16fNew[i].frac >= usDen)
			{
			pu16fNew[i].frac -= usDen;
			pu16fNew[i].delta++; // unsigned positive increment only!
			}
		}

	return pu16fNew;
	}

//===========================
#endif
