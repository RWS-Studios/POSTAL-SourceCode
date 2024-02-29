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
// str.H
// 
// History:
//		05/20/97 JMI	Started.
//
//////////////////////////////////////////////////////////////////////////////
//
// Useful, generic string functions that are not ANSI standard.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef STR_H
#define STR_H

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
#include "System.h"

#ifdef PATHS_IN_INCLUDES

#else

#endif

//////////////////////////////////////////////////////////////////////////////
// Macros.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Protos.
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
extern short rspStricmp(	// Returns 0 if equivalent.
									// Returns < 0 if pszStr1 less than pszStr2.
									// Returns > 0 if pszStr1 greater than pszStr2.
	const char* pszStr1,		// In:  First string to compare.
	const char* pszStr2);	// In:  Second string to compare.

// Perform a lowercase comparison of strings up to n characters.
// If the shorter string ends before n characters and the strings are 
// equal up to the end of the shorter string, that string is lesser.
// Excerpt from VC 5.0 Help on strnicmp():
// "Two strings containing characters located between 'Z' and 'a' in the ASCII
// table ('[', '\', ']', '^', '_', and '`') compare differently, depending on
// their case. For example, the two strings "ABCDE" and "ABCD^" compare one 
// way if the comparison is lowercase ("abcde" > "abcd^") and the other way 
// ("ABCDE" < "ABCD^") if it is uppercase."
extern short rspStrnicmp(	// Returns 0 if equivalent.
									// Returns < 0 if pszStr1 less than pszStr2.
									// Returns > 0 if pszStr1 greater than pszStr2.
	const char* pszStr1,		// In:  First string to compare.
	const char* pszStr2,		// In:  Second string to compare.
	size_t count);				// In:  Number of characters to compare.

//////////////////////////////////////////////////////////////////////////////
// Typedefs.
//////////////////////////////////////////////////////////////////////////////

#endif	// STR_H
//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
