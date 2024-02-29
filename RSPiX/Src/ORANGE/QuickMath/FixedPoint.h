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
#ifndef FIXED_POINT_H
#define FIXED_POINT_H
//====================
#include "System.h" // for environment variables...
#include "QuickMath.h"
//====================
/*****************************************************************
This header depends on the rest QuickMath, but is not necessary
for use of Quickmath, and deals primarily with high speed fixed
point operations.
/*****************************************************************
Hungarian:	fp = either generic fixed point or signed 32 (S16.U16)
				pfp = pointer to fixed point number
						(sorry FILE*)
				fpS32 = S16.U16
				type = RFixedS32, members = "frac" and "mod"

				fpS16 = S8.U8
				fpU16 = U8.U8, etc.
				type = RFixedS16, same members, etc., etc.

				& = implicit pass by reference, do NOT use a pointer
				& is used in almost all cases to prevent VC from creating
				a local stack frame which would slow things down by 20 times.

NOTE: Some functions can take a long AS an fpS32.  Understand that this is
a memory cast as a fpS32 and does NOT represent the actual long "value"!
Use Get and Set "Value" functions to translate integers to fixed point!

/*****************************************************************
RFixedS32 { mod, frac, copy }
RFixedU16 { mod, frac, copy }

inline RFixedS32 rspfpSin(sDeg)
inline RFixedS32 rspfpCos(sDeg)
inline RFixedS32 rspfpOneOver(sDen)

inline void rspfpSet(&fpX,lVal) // lVal is an fpS32 in long form (mem)
inline void rspfpAdd(f&pDst,&fpAdd)
inline void rspfpSub(&fpDst,&fpSub)
inline void rspfpMul(&fpDst,&fpMul)
inline void rspfpMul(&fpDst,&fpA,&fpB)
inline void rspfpMul(&fpDst,long lA,long lB) // fpS32 s as longs (mem)
inline void rspfpAddHalf(&fpDst) // useful for rounding functions
inline void rspfpSetValue(&fpDst,double dVal) // translates VALUE into fp
inline double rspfpGetValue(&fpDst)

/****************************************************************/
// Fixed Point 32S
//======================================= signed 15:16 fixed point
typedef union	{
	long	val; //********* Full 32 bit signed value
	struct	
		{
#ifdef SYS_ENDIAN_BIG // big endian
//-----------------------------------------------
		union
			{
			short	mod; //********* signed 16-bit integer part
			struct {
				signed char upper; 	// for 256v level z-coloring:
				unsigned char lower;
				};
			};
		unsigned short frac; // unsigned 16-bit fractional part
//-----------------------------------------------
#else // little endian
		unsigned short frac;
		union	// for 256v level z-coloring:
			{
			short	mod;
			struct {
				unsigned char lower;
				signed char upper;
				};
			};
#endif
		};
	} RFixedS32;
//====================
inline void rspfpSet(RFixedS32& s32fx,long lVal) 
	{ s32fx.val = lVal; }
inline void rspfpAdd(RFixedS32& s32fx,RFixedS32& s32fxAdd)
	{ s32fx.val += s32fxAdd.val; }
inline void rspfpSub(RFixedS32& s32fx,RFixedS32& s32fxSub)
	{ s32fx.val -= s32fxSub.val; }

// NEED 64-bit multiplication to handle overflow!
#if 0
#ifdef _M_IX86 // INTEL!
	inline void rspfpMul(RFixedS32& s32fx,RFixedS32& s32fxMul)
		{ 
		// Use the 64-bit resultant to get the true value!
		__asm	
			{
			mov eax,s32fx.val
			mov ebx,s32fxMul.val
			imul eax,ebx // result is in EDX::EAX
			mov s32fx.val,edx // only the upper word is valid!
			}
		}

	inline void rspfpMul(RFixedS32& s32fxC,RFixedS32& s32fxA,RFixedS32& s32fxB)
		{ 
		// Use the 64-bit resultant to get the true value!
		__asm	
			{
			mov eax,s32fxA.val
			mov ebx,s32fxB.val
			imul ebx // ([*eax]) result is in EDX::EAX
			mov s32fxC.val,edx // only the upper word is valid!
			}
		}

	inline void rspfpMul(RFixedS32 &lC,long lA,long lB)
		{ 
		union {
			unsigned long val;
			struct { unsigned short lo; unsigned short hi; };
			} temp;
		long temp2;

		// Use the 64-bit resultant to get the true value!
		__asm	
			{
			mov eax,lA
			mov ebx,lB
			imul ebx // ([*eax]) result is in EDX::EAX
			// PROBLEM: we want bits 16-47!!!!!!
			mov temp2,edx // only need hwlf the integral part...
			mov temp,eax	// get upper half!
			}
		lC.mod = (short)temp2; // keep sign, lose upper half!
		lC.frac = temp.hi;
		}
#endif
#endif

inline void rspfpAddHalf(RFixedS32& s32fx) // useful for rounding
	{ s32fx.val += (long)32768; }
//========================= debugging only! (slow)
inline void rspfpSetValue(RFixedS32& s32fx,double dVal)
	{ s32fx.val = (long) (dVal * 65536.0); }
inline double rspfpGetValue(RFixedS32& s32fx)
	{ return (double)s32fx.mod + (double)s32fx.frac / (double)65536.0; }

// For passing back an error:
const RFixedS32	s32fxERROR = {-65536};

// For fixed32 type stuff:
//extern unsigned long OneOver[256];
//extern void SetOneOver();
#define NUM_ONEOVER_FP32 1280

class RInitNum
	{
public:
	RInitNum();
	static long OneOver[NUM_ONEOVER_FP32];
	};

//======================================= unsigned 8:8 fixed point
typedef union	{
	unsigned short	val;
	struct	
		{
#ifdef SYS_ENDIAN_BIG // big endian
		unsigned char	mod;
		unsigned char frac;
#else // little endian
		unsigned char frac;
		unsigned char	mod;
#endif
		};
	} RFixedU16;
//-------------------------------------

//======================================= signed 8:8 fixed point
typedef union	{
	signed short	val;
	struct	
		{
#ifdef SYS_ENDIAN_BIG // big endian
		signed char	mod;
		unsigned char frac;
#else // little endian
		unsigned char frac;
		signed char	mod;
#endif
		};
	} RFixedS16;
//-------------------------------------

//====================
extern RFixedS32 fpSINQ[csNumRotSteps],fpCOSQ[csNumRotSteps];
extern	void InitTrigFP();
inline RFixedS32 rspfpSin(short sDeg) { return fpSINQ[sDeg]; }
inline RFixedS32 rspfpCos(short sDeg) { return fpCOSQ[sDeg]; }
inline long rspfpOneOver(short sDen) { return RInitNum::OneOver[sDen]; }

// Auto Initialize hook!
class RQuickTrigFP
	{
public:
	RQuickTrigFP() { InitTrigFP(); TRACE("QTRIG fixed point initialized!\n"); }
	~RQuickTrigFP() {  }
	};

//====================
#endif
