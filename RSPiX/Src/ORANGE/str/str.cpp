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
// str.CPP
// 
// History:
//		05/20/97 JMI	Started.
//
//		05/21/97	JMI	Added a table to replace the LOWER() macro for speed as
//							per Jeff's suggestion.
//
//		05/21/97 BRH	Fixed rspStrnicmp to recognize completion correctly.
//
//		06/29/97 MJR	Added const's and switched to size_t for count to bring
//							into full compliance with ASNI equivalent.
//
//////////////////////////////////////////////////////////////////////////////
//
// Useful, generic string functions that are not ANSI standard.
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// C Headers -- Must be included before RSPiX.h b/c RSPiX utilizes SHMalloc.
//////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// RSPiX Headers.
// If PATHS_IN_INCLUDES macro is defined, we can utilize relative
// paths to a header file.  In this case we generally go off of our
// RSPiX root directory.  System.h MUST be included before this macro
// is evaluated.  System.h is the header that, based on the current
// platform (or more so in this case on the compiler), defines 
// PATHS_IN_INCLUDES.  Blue.h includes system.h so you can include that
// instead.
///////////////////////////////////////////////////////////////////////////////
#include "Blue.h"

#ifdef PATHS_IN_INCLUDES
	#include "ORANGE/str/str.h"
#else
	#include "str.h"
#endif

//////////////////////////////////////////////////////////////////////////////
// Module specific macros.
//////////////////////////////////////////////////////////////////////////////

// Get the lowercase equivalent, if alpha.
#define LOWER(c)	( ( (c) >= 'A' && (c) <= 'Z') ? (c) - 32 : (c) )

//////////////////////////////////////////////////////////////////////////////
// Module specific typedefs.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Exported (extern) variables.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Module specific (static) variables / Instantiate class statics.
//////////////////////////////////////////////////////////////////////////////

static short ms_asUpper2Lower[256]	=
	{
	0,
	1,
	2,
	3,
	4,
	5,
	6,
	7,
	8,
	9,
	10,
	11,
	12,
	13,
	14,
	15,
	16,
	17,
	18,
	19,
	20,
	21,
	22,
	23,
	24,
	25,
	26,
	27,
	28,
	29,
	30,
	31,
	32,
	33,
	34,
	35,
	36,
	37,
	38,
	39,
	40,
	41,
	42,
	43,
	44,
	45,
	46,
	47,
	48,
	49,
	50,
	51,
	52,
	53,
	54,
	55,
	56,
	57,
	58,
	59,
	60,
	61,
	62,
	63,
	64,
	'a',
	'b',
	'c',
	'd',
	'e',
	'f',
	'g',
	'h',
	'i',
	'j',
	'k',
	'l',
	'm',
	'n',
	'o',
	'p',
	'q',
	'r',
	's',
	't',
	'u',
	'v',
	'w',
	'x',
	'y',
	'z',
	91,
	92,
	93,
	94,
	95,
	96,
	97,
	98,
	99,
	100,
	101,
	102,
	103,
	104,
	105,
	106,
	107,
	108,
	109,
	110,
	111,
	112,
	113,
	114,
	115,
	116,
	117,
	118,
	119,
	120,
	121,
	122,
	123,
	124,
	125,
	126,
	127,
	128,
	129,
	130,
	131,
	132,
	133,
	134,
	135,
	136,
	137,
	138,
	139,
	140,
	141,
	142,
	143,
	144,
	145,
	146,
	147,
	148,
	149,
	150,
	151,
	152,
	153,
	154,
	155,
	156,
	157,
	158,
	159,
	160,
	161,
	162,
	163,
	164,
	165,
	166,
	167,
	168,
	169,
	170,
	171,
	172,
	173,
	174,
	175,
	176,
	177,
	178,
	179,
	180,
	181,
	182,
	183,
	184,
	185,
	186,
	187,
	188,
	189,
	190,
	191,
	192,
	193,
	194,
	195,
	196,
	197,
	198,
	199,
	200,
	201,
	202,
	203,
	204,
	205,
	206,
	207,
	208,
	209,
	210,
	211,
	212,
	213,
	214,
	215,
	216,
	217,
	218,
	219,
	220,
	221,
	222,
	223,
	224,
	225,
	226,
	227,
	228,
	229,
	230,
	231,
	232,
	233,
	234,
	235,
	236,
	237,
	238,
	239,
	240,
	241,
	242,
	243,
	244,
	245,
	246,
	247,
	248,
	249,
	250,
	251,
	252,
	253,
	254,
	255
	};

//////////////////////////////////////////////////////////////////////////////
// Module specific (static) protos.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Functions.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Perform a lowercase comparison of strings.
// If the strings are equal up to the end of the shorter string, that
// string is lesser.
// Excerpt from VC 5.0 Help on strnicmp():
// "Two strings containing characters located between 'Z' and 'a' in the ASCII
// table ('[', '\', ']', '^', '_', and '`') compare differently, depending on
// their case. For example, the two strings "ABCDE" and "ABCD^" compare one 
// way if the comparison is lowercase ("abcde" > "abcd^") and the other way 
// ("ABCDE" < "ABCD^") if it is uppercase."
//////////////////////////////////////////////////////////////////////////////
extern short rspStricmp(	// Returns 0 if equivalent.
									// Returns < 0 if pszStr1 less than pszStr2.
									// Returns > 0 if pszStr1 greater than pszStr2.
	const char* pszStr1,		// In:  First string to compare.
	const char* pszStr2)		// In:  Second string to compare.
	{
	short	sRes	= 0;	// Assume equivalent.

	while (*pszStr1 != '\0' && *pszStr2 != '\0' && sRes == 0)
		{
		sRes	= ms_asUpper2Lower[*pszStr1++] - ms_asUpper2Lower[*pszStr2++];
		}

	// If identical . . .
	if (sRes == 0)
		{
		// If first string ended prematurely . . .
		if (*pszStr1 == '\0' && *pszStr2 != '\0')
			{
			sRes	= -1;
			}
		// Else, if second string ended prematurely . . .
		else if (*pszStr1 != '\0' && *pszStr2 == '\0')
			{
			sRes	= 1;
			}
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
// Perform a lowercase comparison of strings up to n characters.
// If the shorter string ends before n characters and the strings are 
// equal up to the end of the shorter string, that string is lesser.
// Excerpt from VC 5.0 Help on strnicmp():
// "Two strings containing characters located between 'Z' and 'a' in the ASCII
// table ('[', '\', ']', '^', '_', and '`') compare differently, depending on
// their case. For example, the two strings "ABCDE" and "ABCD^" compare one 
// way if the comparison is lowercase ("abcde" > "abcd^") and the other way 
// ("ABCDE" < "ABCD^") if it is uppercase."
//////////////////////////////////////////////////////////////////////////////
extern short rspStrnicmp(	// Returns 0 if equivalent.
									// Returns < 0 if pszStr1 less than pszStr2.
									// Returns > 0 if pszStr1 greater than pszStr2.
	const char* pszStr1,		// In:  First string to compare.
	const char* pszStr2,		// In:  Second string to compare.
	size_t count)				// In:  Number of characters to compare.
	{
	ASSERT(count >= 0);

	short	sRes	= 0;	// Assume equivalent.

	while (*pszStr1 != '\0' && *pszStr2 != '\0' && sRes == 0 && count--)
		{
		sRes	= ms_asUpper2Lower[*pszStr1++] - ms_asUpper2Lower[*pszStr2++];
		}

	// If identical . . .
	if (sRes == 0 && count != 0)
		{
		// If first string ended prematurely . . .
		if (*pszStr1 == '\0' && *pszStr2 != '\0')
			{
			sRes	= -1;
			}
		// Else, if second string ended prematurely . . .
		else if (*pszStr1 != '\0' && *pszStr2 == '\0')
			{
			sRes	= 1;
			}
		}

	return sRes;
	}

///////////////////////////////////////////////////////////////////////////////
// EOF
///////////////////////////////////////////////////////////////////////////////
