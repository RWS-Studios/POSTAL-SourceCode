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
#ifndef FRACTIONS_H
#define FRACTIONS_H
//=======================
#include "System.h"
//====================
/*****************************************************************
This header depends on the rest QuickMath, but is not necessary
for use of Quickmath, and deals primarily with high speed fraction
operations.
/*****************************************************************
Hungarian:	fr = either generic fraction or signed 16 (S16 + S16 [ / U16] )
				pfr = pointer to fraction
				frS32 = (S32 + S32 [ / U32] )
				type = RFracS32, members = "frac" and "mod" and "set"

				frS16 = S16 + S16 [ / U16]
				frU16 = S16 + U16 [ / U16]
				type = RFracU16, same members, etc., etc.

				& = implicit pass by reference, do NOT use a pointer
				& is used in almost all cases to prevent VC from creating
				a local stack frame which would slow things down by 20 times.

NOTE: The integral value of the fraction is always a SIGNED quantity.  
The S/U refers to whether or not the NUMERATOR of the fraction is
signed or unsigned.

NOTE: Some functions can take a long AS an frS16.  Understand that this is
a memory cast as a frS16 and does NOT represent the actual long "value"!
Use Get and Set "Value" functions to translate integers to fixed point!
/*****************************************************************
RFracS32 { mod, frac, set }
RFracS16 { mod, frac, set }
RFracU16 { mod, frac, set }

inline void rspfrDiv(&frDst,sNum,sDen) // sDen > 0
inline void rspfrSet(&frDst,lVal) // a memset!
inline void rspfrAdd(&frDst,frAdd,short sDen)
inline void rspfrSub(&frDst,frSub,short sDen)
inline void rspfrAddHalf(&frDst,short sDen) // useful for rounding
inline void rspfrSetValue(&frDst,double dVal,short sDen)
inline void rspMakeProper(&frU16Dst,usNum,usDen)
inline RFracU16* rspfrU16Strafe256(usNum,usDen) // gives 0..255 * fraction!

//*********************** HIGH INTENSITY SPEED! ***********************
inline void rspfrAdd32(&lVal,&lNum,&lDel,&lInc,&lDen,&lAdd,PROPER,POSITIVE)
inline void rspfrAdd32(&lVal,&lNum,&lDel,&lInc,&lDen,&lAdd,IMPROPER,POSITIVE)
inline void rspfrAdd32(&lVal,&lNum,&lDel,&lInc,&lDen,&lAdd,IMPROPER,POSITIVE)
inline void rspfrAdd32(&lVal,&lNum,&lDel,&lInc,&lDen,&lAdd,PROPER,NEGATIVE)
inline void rspfrAdd32(&lVal,&lNum,&lDel,&lInc,&lDen,&lAdd,IMPROPER,NEGATIVE)
inline void rspfrAdd32(&lVal,&lNum,&lDel,&lInc,&lDen,&lPixSize)
/****************************************************************/

  typedef union
	{
	struct {
	U16 mod;
	U16 frac;
			};
	U32 set;
	}	RFracU16;	// No denominator, unsigned values...

//======================================= 
typedef union	{
	long	set;
	struct	
		{
		short	mod;
		short frac;
		};
	} RFracS16;

//=======================================
typedef union	{
	S64	set;
	struct	
		{
		long mod;
		long frac;
		};
	} RFracS32; // good for compound fractions

//************* ALERT ALERT!!! FRACTION STUFF HERE!!!!!  *******************
// There should be more functions to handle simplified cases!
// sDen MUST be positive!
inline void rspfrDiv(RFracS16& fr,short sNum,short sDen)
	{
	fr.mod = sNum / sDen;
	fr.frac = sNum % sDen;
	if (sNum < 0) // special case
		if (fr.frac != 0) // put on correct side of negative number!
			{
			fr.mod--;
			fr.frac += sDen;
			}
	}

inline void rspfrSet(RFracS16& fr,long lVal) { fr.set = lVal; }

// Signed can go either way... use unsigned for speed!
// This also assumes it can add greater than one!
inline void rspfrAdd(RFracS16& frDst,RFracS16& frAdd,short sDen)
	{ 
	frDst.mod += frAdd.mod;
	frDst.frac += frAdd.frac;
	if (frDst.frac >= sDen) {frDst.mod++;frDst.frac -= sDen;}
	if (frDst.frac < 0) {frDst.mod--;frDst.frac += sDen;}
	}

inline void rspfrSub(RFracS16& frDst,RFracS16& frSub,short sDen)
	{ 
	frDst.mod -= frSub.mod;
	frDst.frac -= frSub.frac;
	if (frDst.frac < 0) {frDst.mod--;frDst.frac += sDen;}
	if (frDst.frac >= sDen) {frDst.mod++;frDst.frac -= sDen;}
	}

inline void rspfrAddHalf(RFracS16& frDst,short sDen) // useful for rounding
	{ 
	frDst.frac += (sDen >> 1);
	if (frDst.frac >= sDen) {frDst.mod++;frDst.frac -= sDen;}
	}

//========================= debugging only! (slow)
inline void rspfrSetValue(RFracS16& frDst,double dVal,short sDen)
	{
	frDst.mod = (short) floor(dVal);
	frDst.frac = (short) ((dVal - floor(dVal))*sDen);
	}

// add two fractions of identical denominators...
// both fraction MUST be PROPER! 
// UNSIGNED!!!!
//
inline void	rspfrAdd(RFracU16& pDst,RFracU16& pAdd,short sDen)
	{
	pDst.mod += pAdd.mod;
	if ( (pDst.frac += pAdd.frac) >= sDen)
		{
		pDst.mod++; // ONLY adding proper fractions
		pDst.frac -= sDen;
		}
	}

// Creates a proper fraction from an improper one:
// The sizes MUST be appropriate!
//
inline void rspMakeProper(RFracU16& pDst,USHORT usNum,USHORT usDen)
	{
	pDst.mod = usNum / usDen;
	pDst.frac = usNum % usDen;
	//pDst.frac = usNum - pDst.mod * usDen;// overflow problem
	}

// Creates an array of 256 fractions, uch that 0 is 0,
// 1 is tha base fraction, and 255 is 255 * the base fraction.
// Both must be unsigned!  (uses calloc)
//
inline RFracU16* rspfrU16Strafe256(USHORT usNum,USHORT usDen)
	{
	RFracU16* pu16fNew = (RFracU16*) calloc(256,sizeof(RFracU16));
	RFracU16 u16fInc;
	rspMakeProper(u16fInc,usNum,usDen); // the 2 part mod

	ULONG ulNumInc = 0;
	for (short i = 1; i < 256 ; i++)
		{
		pu16fNew[i].mod = pu16fNew[i-1].mod + u16fInc.mod;
		pu16fNew[i].frac = pu16fNew[i-1].frac + u16fInc.frac;
		if (pu16fNew[i].frac >= usDen)
			{
			pu16fNew[i].frac -= usDen;
			pu16fNew[i].mod++; // unsigned positive increment only!
			}
		}

	return pu16fNew;
	}

//-------------------------- 32 bit signed integer calculus stuff:
//*********************** HIGH INTENSITY SPEED! ***********************

typedef U32 POSITIVE;
typedef S32 NEGATIVE;

typedef U32 PROPER;
typedef S32 IMPROPER;

// lDel is unused here...
// all values are signed, including lInc & lDel ...
//
inline void rspfrAdd32(long &lVal,long &lNum,long &lDel,long &lInc,
						  long &lDen,long &lAdd,PROPER,POSITIVE)
	{
	lNum += lInc;
	if (lNum >= lDen) { lNum -= lDen; lVal += lAdd; }
	}

// all values are signed, including lInc & lDel ...
//
inline void rspfrAdd32(long &lVal,long &lNum,long &lDel,long &lInc,
						  long &lDen,long &lAdd,IMPROPER,POSITIVE)
	{
	lVal += lDel;
	lNum += lInc;
	if (lNum >= lDen) { lNum -= lDen; lVal += lAdd; }
	}

// lDel is unused here...
// all values are signed, including lInc & lDel & lAdd ...
//
inline void rspfrAdd32(long &lVal,long &lNum,long &lDel,long &lInc,
						  long &lDen,long &lAdd,PROPER,NEGATIVE)
	{
	lNum += lInc;
	if (lNum < 0) { lNum += lDen; lVal -= lAdd; }
	}

// all values are signed, including lInc & lDel & lAdd ...
//
inline void rspfrAdd32(long &lVal,long &lNum,long &lDel,long &lInc,
						  long &lDen,long &lAdd,IMPROPER,NEGATIVE)
	{
	lVal += lDel;
	lNum += lInc;
	if (lNum < 0) { lNum += lDen; lVal -= lAdd; }
	}

// all values are signed, including lInc & lDel & lAdd ...
// General (slowest) form... Improper and UNKNOWN SIGN
//
inline void rspfrAdd32(long &lVal,long &lNum,long &lDel,long &lInc,
						  long &lDen,long &lPixSize)
	{
	lVal += lDel;
	lNum += lInc;
	if (lNum < 0) { lNum += lDen; lVal -= lPixSize; }
	if (lNum >= lDen) { lNum -= lDen; lVal += lPixSize; }
	}

//=======================
#endif
