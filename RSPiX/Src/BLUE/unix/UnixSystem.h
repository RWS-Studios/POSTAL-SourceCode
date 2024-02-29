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
////////////////////////////////////////////////////////////////////////////////
//
//	UnixSystem.h
// 
// History:
//		06/01/04 RCG    Added.

////////////////////////////////////////////////////////////////////////////////
//
//	This file provides typedefs, macros, pragmas, etc. for Unix systems.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef UNIXSYSTEM_H
#define UNIXSYSTEM_H

#include "SDL.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
// !!! FIXME: use SDL_snprintf() in SDL2.
#ifndef snprintf
#define snprintf _snprintf
#endif
#ifndef vsnprintf
#define vsnprintf _vsnprintf
#endif
#ifndef strcasecmp
#define strcasecmp _stricmp
#endif
#endif

#if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
#define SYS_ENDIAN_BIG 1
#else
#define SYS_ENDIAN_LITTLE 1
#endif

#include <map>
using namespace std;

////////////////////////////////////////////////////////////////////////////////
// For compilers that support partial paths in #include's, define this.  For
// those that don't, don't define it.
////////////////////////////////////////////////////////////////////////////////
#define PATHS_IN_INCLUDES


////////////////////////////////////////////////////////////////////////////////
// Usefull pragms
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Old-fashioned base types (BASETYPES macro is for compatibility with WIN32)
////////////////////////////////////////////////////////////////////////////////
#ifndef BASETYPES
#define BASETYPES
typedef unsigned long ULONG;
typedef ULONG *PULONG;
typedef unsigned short USHORT;
typedef USHORT *PUSHORT;
typedef unsigned char UCHAR;
typedef UCHAR *PUCHAR;
typedef char *PSZ;
#endif // !BASETYPES


////////////////////////////////////////////////////////////////////////////////
// Basic RSPiX Types
////////////////////////////////////////////////////////////////////////////////
#ifndef __RSPX_TYPES
#define __RSPX_TYPES

	// The world's most specific types < S | U > < # >
	// S == signed, U == unsigned, # == number of bits
	typedef signed		char		S8;
	typedef unsigned	char		U8;
	typedef signed		short		S16;
	typedef unsigned	short		U16;
	typedef signed		long		S32;
	typedef unsigned	long		U32;
	typedef signed long long S64;
	typedef unsigned long long U64;

	// 128-bit got a little trickier...
	#ifdef SYS_ENDIAN_LITTLE
		typedef struct {	U64	lo;
								S64	hi;} S128;
		typedef struct {	U64	lo;
								U64	hi;} U128;
	#else	// defined(SYS_ENDIAN_BIG)

		typedef struct {	S64	hi;
								U64	lo;} S128;
		typedef struct {	U64	hi;
								U64	lo;} U128;
	#endif


	// Ranges for basic RSPiX types
	#define	S8_MIN	((S8) 0x80)
	#define	S8_MAX	((S8) 0x7F)
	#define	U8_MIN	((U8) 0x0)
	#define	U8_MAX	((U8) 0xFF)
	#define	S16_MIN	((S16) 0x8000)
	#define	S16_MAX	((S16) 0x7FFF)
	#define	U16_MIN	((U16) 0x0)
	#define	U16_MAX	((U16) 0xFFFF)
	#define	S32_MIN	((S32) 0x80000000L)
	#define	S32_MAX	((S32) 0x7FFFFFFFL)
	#define	U32_MIN	((U32) 0x0)
	#define	U32_MAX	((U32) 0xFFFFFFFFUL)
	
	// These pixel types take the endian order of the system into account.
	typedef U8 RPixel;
	typedef U16 RPixel16;
	typedef struct
		{
		U8	u8Red;
		U8	u8Green;
		U8	u8Blue;
		} RPixel24;
	typedef struct
		{
		U8	u8Alpha;
		U8	u8Red;
		U8	u8Green;
		U8	u8Blue;
		} RPixel32;
	inline bool operator==(const RPixel24& lhs, const RPixel24& rhs)
		{ return ((lhs.u8Blue == rhs.u8Blue) && (lhs.u8Green == rhs.u8Green) && (lhs.u8Red == rhs.u8Red)) ? true : false; }
	inline bool operator==(const RPixel32& lhs, const RPixel32& rhs)
		{ return ((lhs.u8Blue == rhs.u8Blue) && (lhs.u8Green == rhs.u8Green) && (lhs.u8Red == rhs.u8Red) && (lhs.u8Alpha == rhs.u8Alpha)) ? true : false; }

#endif // __RSPX_TYPES


////////////////////////////////////////////////////////////////////////////////
// Handy macros.
////////////////////////////////////////////////////////////////////////////////
#ifndef TRUE
#define TRUE	1
#endif

#ifndef FALSE
#define FALSE	0
#endif

#ifndef NULL
#define NULL	0
#endif

#ifndef SUCCESS
#define SUCCESS	0
#endif

#ifndef FAILURE
#define FAILURE	-1
#endif

////////////////////////////////////////////////////////////////////////////////
// Usefull Templates
////////////////////////////////////////////////////////////////////////////////
#undef MIN
template <class T>
inline T MIN(T a,T b) { return (a < b) ? a : b; }

#undef MAX
template <class T>
inline T MAX(T a,T b) { return (a > b) ? a : b; }

#undef SWAP	// Swaps two identical typed variables
template <class T>
inline void SWAP(T &a,T &b) { T temp = a; a = b; b = temp; }

#undef SQR // squares a number
template <class T>
inline T SQR(T x) { return x * x; }

#undef ABS // returns the absolute value of a parameter
template <class T>
inline T ABS(T x) { return (x < 0) ? -x : x; }

template <class T> // returns the square of the absolute value
inline T ABS2(T x,T y) { return SQR(x)+SQR(y); }

template <class T> // returns the square of the absolute value
inline T ABS2(T x,T y,T z) { return SQR(x) + SQR(y) + SQR(z); }

#undef SGN // returns a binary sign (+1 or -1)
template <class T>
inline T SGN(T x) { return (x < 0) ? (T)-1 : (T)1; }

#undef SGN3 // returns a trinary sign (+1, 0, or -1)
template <class T>
inline T SGN3(T x) { return (x == 0) ? (T)0 : ((x < 0) ? (T)-1 : (T)1); }

template <class T> // attaches a sign to a value
inline T ADD_SGN(T sign,T val) { return (sign < 0) ? -val : val; }

template <class T> // symmetric mod wrt sign, good for DELTAS
// (This is the mathematically standard mod)
inline void DIV_MOD(T num,T den,T &div, T &mod) // does NOT check if (den == 0)
	{
	div = num / den;
	mod = num - div * den; 
	}

// Copy an array of objects (just like strcpy() but it works for any type).
// Object must have a default or overloaded operator= for assignment.
template<class T>
inline void rspObjCpy(T* pDst, const T* pSrc, size_t count)
	{
	if (count > 0)
		{
		while (count--)
			*pDst++ = *pSrc++;
		}
	}

// Compare an array of objects (just like strcmp() but it works for any type).
// Object must have a default or overloaded operator== to test for equality.
template<class T>
inline bool rspObjCmp(const T* p1, const T* p2, size_t count)
	{
	if (count > 0)
		{
		// Need to use '==' because that's what we're asking objects to overload!
		// (Hence the odd-looking notation of negating the result of the '=='.)
		while (count--)
			{
			if (!(*p1++ == *p2++))
				return false;
			}
		return true;
		}
	return true;
	}


inline char *ltoa(long l, char *buf, size_t bufsize)
{
    snprintf(buf, bufsize, "%ld", l);
    return(buf);
}

inline char *itoa(int l, char *buf, size_t bufsize)
{
    snprintf(buf, bufsize, "%d", l);
    return(buf);
}

#define strnicmp(x, y, z) strncasecmp(x, y, z)
#define _ltoa(x, y, z) ltoa(x, y, z)

#endif // UNIXSYSTEM_H
//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////

