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
// This uses a class to pre-initialize the size/cosine tables.
// It is currently set to use only integer degrees as input.
#ifndef QUICK_MATH_H
#define QUICK_MATH_H
#include <math.h>
#include "Blue.h"

#define rspPI  3.14159265359
#define rspRadToDeg  57.29577951308
#define rspDegToRad  0.01745329251994
#define rspSQRT2  1.414213562373
#define rspSQRT3	1.732050807569

#define	MAX_FAST_SQRT 131072	// Will take a LOT of memory
/*****************************************************************
Quickmath contains several extansions, but FixedPoint.cpp is
required because it is called to initialize from the normal
quickmath initialization.  QuickMath is based upon the concept
of integer degrees, from 0-359 inclusive.  

  Hungarian notation: 
				& = implicit pass by reference, do NOT use a pointer
				& is used in almost all cases to prevent VC from creating
				a local stack frame which would slow things down by 20 times.
				T = a single templated type
				-64 = enhanced 64-bit math mode...

/*****************************************************************
inline void rspMod360(&sDeg) // USE BEFORE ALL CALLS IF IN QUESTION!
inline void rspMod(&sValue,&sRange) 

inline void rspDivMod(T num,T den,T &div, T &mod) // symmetric (methematical)
inline void rspDivModA(T num,T den,T &div, T &mod) // asymmetric
inline void rspDivModA64(S64 num,S32 den,S32 &div,S32 &mod) // asymmetric

short rspATan(sDeltaY,sDeltaX);
inline short rspDegDelta(sDegSrc,sDegDst)
short rspATan(dVal);
inline double rspSin(sDeg)
inline double rspCos(sDeg)
inline float rspfSin(sDeg)
inline float rspfCos(sDeg)

inline short rspSqrt(long lVal)

/*****************************************************************

/****************************************************************/
const short csNumRotSteps = 360;
extern double SINQ[csNumRotSteps],COSQ[csNumRotSteps];
extern float fSINQ[csNumRotSteps],fCOSQ[csNumRotSteps];
extern short ATANQ[60];
extern short SQRTQ[MAX_FAST_SQRT];
/****************************************************************/
inline short	rspSqrt(long lVal)
	{
	if (lVal < MAX_FAST_SQRT) return SQRTQ[lVal];
	return short(sqrt(double(lVal)));
	}

inline double rspSin(short sDeg) 
	{ 
#ifdef _DEBUG
	ASSERT((sDeg >= 0) || (sDeg < 360));
#endif

	return SINQ[sDeg]; 
	}

inline double rspCos(short sDeg) 
	{ 
#ifdef _DEBUG
	ASSERT((sDeg >= 0) || (sDeg < 360));
#endif

	return COSQ[sDeg]; 
	}

inline float rspfSin(short sDeg) 
	{ 
#ifdef _DEBUG
	ASSERT((sDeg >= 0) || (sDeg < 360));
#endif

	return fSINQ[sDeg];
	}

inline float rspfCos(short sDeg) 
	{ 
#ifdef _DEBUG
	ASSERT((sDeg >= 0) || (sDeg < 360));
#endif

	return fCOSQ[sDeg]; 
	}

extern	short rspATan(short sDeltaY,short sDeltaX);
extern	short rspATan(double dVal);
inline	short rspDegDelta(short sDegSrc,short sDegDst)
	{
	short sDel = sDegDst - sDegSrc;

	if (sDel >= 0) // positive turn
		{
		if (sDel > 180) sDel -= 360; // neg turn
		}
	else
		{// negative turn
		if (sDel < -180) sDel += 360; // pos turn
		}

	return sDel;
	}

// Use individual whiles if you know what 
// to expect
//
inline short rspMod360(short sDeg)
	{
	while (sDeg < 0) sDeg += 360;
	while (sDeg > 359) sDeg -= 360;
	return sDeg;
	}

inline short rspMod(short sVal,short &sRange)
	{
	while (sVal < 0) sVal += sRange;
	while (sVal >= sRange) sVal -= sRange;
	return sVal;
	}

inline void rspMod360(short *sDeg)
	{
	*sDeg = rspMod360(*sDeg);
	}

inline void rspMod(short *sVal,short &sRange)
	{
	*sVal = rspMod(*sVal,sRange);
	}

extern	void InitTrig();

// Auto Initialize hook!
class RQuickTrig
	{
public:
	RQuickTrig() { InitTrig(); }
	~RQuickTrig() {  }
	};

template <class T> // aymmetric mod wrt sign, good for deltas
inline void rspDivMod(T num,T den,T &div, T &mod) // does NOT check if (den == 0)
	{	// Algorithm not verified for neqative denominator!!!!
	div = num / den;
	mod = num - div * den;
	}

template <class T> // asymmetric mod wrt sign, good for POSITION
inline void rspDivModA(T num,T den,T &div, T &mod) // does NOT check if (den == 0)
	{	// Algorithm not verified for neqative denominator!!!!
	div = num / den;
	mod = num - div * den;
	if (mod)
		if (div < 0)  { div--; mod += den;}
	else if (!div)
		if (mod < 0) { div--; mod += den;}
	}

// This is MACHINE specific and handles temporary overflows:
inline void rspDivModA64(S64 num,S32 den,S32 &div,S32 &mod) // does NOT check if (den == 0)
	{	// Algorithm not verified for neqative denominator!!!!
	div = num / den;
	mod = num - div * den;
	if (mod)
		if (div < 0)  { div--; mod += den;}
	else if (!div)
		if (mod < 0) { div--; mod += den;}
	}



//-------------------------------
#endif
