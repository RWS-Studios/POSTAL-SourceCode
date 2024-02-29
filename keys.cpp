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
// Keys.cpp
// Project: Nostril (aka Postal)
//
// History:
//		03/31/97	JMI	Started.
//
//		07/06/97	JMI	Changed pu8ScanKey parm in KeyDescriptionToValue
//							call from a U8 to a short.
//							Also, changed g_apszButtonDescriptions to 
//							g_apszMouseButtonDescriptions.
//
//		08/10/97	JMI	Changed description of 0 to "None" (was "NULL").
//
//		08/24/97	JMI	Changed descriptions of Numpad <whatever>s to Numpad 
//							<whatever symbol>s.
//							Also, changed 'UNUSED 59' to 'Semicolon' and 'UNUSED 61'
//							to 'Equal'.
//
//		08/27/97	JMI	Reduced mouse strings to minimum verbosity.
//
//		10/10/97	JMI	Added g_apszJoyButtonDescriptions and
//							JoyButtonDescriptionToMask().
//
////////////////////////////////////////////////////////////////////////////////
//
//	Key stuff.  I'm not sure if this will ever amount to more than just the 
// descriptions.
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// RSPiX Includes.
////////////////////////////////////////////////////////////////////////////////
#include "RSPiX.h"

////////////////////////////////////////////////////////////////////////////////
// C Includes.
////////////////////////////////////////////////////////////////////////////////
#include <string.h>

////////////////////////////////////////////////////////////////////////////////
// Postal Includes.
////////////////////////////////////////////////////////////////////////////////
#include "keys.h"

////////////////////////////////////////////////////////////////////////////////
// Macros.
////////////////////////////////////////////////////////////////////////////////

// Determines the number of elements in the passed array at compile time.
#define NUM_ELEMENTS(a)		(sizeof(a) / sizeof(a[0]) )

////////////////////////////////////////////////////////////////////////////////
// Data.
////////////////////////////////////////////////////////////////////////////////

// Array of key descriptors.
extern char* g_apszKeyDescriptions[128]	=
	{
	"None",
	"End",
	"Home",
	"Left",
	"Up",
	"Right",
	"Down",
	"UNUSED 7",
	"Backspace",
	"Tab",
	"Insert",
	"Delete",
	"UNUSED 12",
	"Enter",
	"Left Shift",
	"Shift",
	"Right Shift",
	"Left Control",
	"Control",
	"Right Control",
	"Left Alt",
	"Alt",
	"Right Alt",
	"UNUSED 23",
	"UNUSED 24",
	"Page Up",
	"Page Down",
	"Escape",
	"Pause",
	"Capital",
	"Numlock",
	"Scroll",
	"Space",
	"Print Screen",
	"UNUSED 34",
	"UNUSED 35",
	"UNUSED 36",
	"UNUSED 37",
	"UNUSED 38",
	"Right Quote",
	"UNUSED 40",
	"UNUSED 41",
	"UNUSED 42",
	"UNUSED 43",
	"Comma",
	"Minus",
	"Period",
	"Slash",
	"0",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9",
	"UNUSED 58",
	"Semicolon",
	"UNUSED 60",
	"Equals",
	"UNUSED 62",
	"UNUSED 63",
	"UNUSED 64",
	"A",
	"B",
	"C",
	"D",
	"E",
	"F",
	"G",
	"H",
	"I",
	"J",
	"K",
	"L",
	"M",
	"N",
	"O",
	"P",
	"Q",
	"R",
	"S",
	"T",
	"U",
	"V",
	"W",
	"X",
	"Y",
	"Z",
	"Left Bracket",
	"Back Slash",
	"Right Bracket",
	"UNUSED 94",
	"UNUSED 95",
	"Left Quote",
	"Numpad 0",
	"Numpad 1",
	"Numpad 2",
	"Numpad 3",
	"Numpad 4",
	"Numpad 5",
	"Numpad 6",
	"Numpad 7",
	"Numpad 8",
	"Numpad 9",
	"Numpad *",
	"Numpad +",
	"Numpad -",
	"Numpad .",
	"Numpad /",
	"UNUSED 112",
	"F1",
	"F2",
	"F3",
	"F4",
	"F5",
	"F6",
	"F7",
	"F8",
	"F9",
	"F10",
	"F11",
	"F12",
	"Left System",
	"System",
	"Right System"
	};

// Array of mouse button descriptors.
extern char* g_apszMouseButtonDescriptions[8]	=
	{
	"None",
	"Left",
	"Right",
	"Middle",
	"Button 4",
	"Button 5",
	"Wheel Up",
	"Wheel Down",
	};

// Array of joy button descriptors.
extern char* g_apszJoyButtonDescriptions[18] =
{
	"None",
	"A",
	"B",
	"X",
	"Y",
	"Back",
	"Guide",
	"Start",
	"LS",
	"RS",
	"LB",
	"RB",
	"Up",
	"Down",
	"Left",
	"Right",
	"LT",
	"RT"
};
/*
extern char* g_apszJoyButtonDescriptions[16]	=
	{
	"None",								// 0000
	"A",									// 0001
	"B",									// 0010
	"A,B",								// 0011
	"C",									// 0100
	"A,C",								// 0101
	"B,C",								// 0110
	"A,B,C",								// 0111
	"D",									// 1000
	"A,D",								// 1001
	"B,D",								// 1010
	"A,B,D",								// 1011
	"C,D",								// 1100
	"A,C,D",								// 1101
	"B,C,D",								// 1110
	"A,B,C,D",							// 1111
	};
	*/
////////////////////////////////////////////////////////////////////////////////
// Functions.
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Given a string, returns the appropriate key code.
////////////////////////////////////////////////////////////////////////////////
extern short KeyDescriptionToValue(	// Returns 0 on success.  Returns non-zero, if
												// key not found.
	char*		pszKeyDescriptor,			// In:  Description of key.
	U32*	psScanKey)					// Out: Key value.
	{
	short	sRes	= 1;	// Assume failure.

	U8	u8KeyIndex;
	for (u8KeyIndex = 0; u8KeyIndex < NUM_ELEMENTS(g_apszKeyDescriptions); u8KeyIndex++)
		{
		if (rspStricmp(pszKeyDescriptor, g_apszKeyDescriptions[u8KeyIndex]) == 0)
			{
			// Found it!
			*psScanKey	= u8KeyIndex;
			sRes	= 0;

			break;
			}
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////////////
// Given a string, returns the appropriate button mask.
////////////////////////////////////////////////////////////////////////////////
extern short MouseButtonDescriptionToMask(	// Returns 0 on success.  Returns
															// non-zero, if description not 
															// found.
	char*		pszButtonDescriptor,					// In:  Description of button.
	U32*	psButtonMask)							// Out: Button mask.
	{
	short	sRes	= 1;	// Assume failure.

	short	sButtonIndex;
	for (sButtonIndex = 0; sButtonIndex < NUM_ELEMENTS(g_apszMouseButtonDescriptions); sButtonIndex++)
		{
		if (rspStricmp(pszButtonDescriptor, g_apszMouseButtonDescriptions[sButtonIndex]) == 0)
			{
			// Found it!
			*psButtonMask = MouseIndexToBitfield(sButtonIndex);
			sRes	= 0;

			break;
			}
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////////////
// Given a string, returns the appropriate button mask.
////////////////////////////////////////////////////////////////////////////////
extern short JoyButtonDescriptionToMask(	// Returns 0 on success.  Returns 
														// non-zero, if description not found.
	char*		pszButtonDescriptor,				// In:  Description of button.
	U32*	psButtonMask)						// Out: Button mask.
	{
	short	sRes	= 1;	// Assume failure.

	short	sButtonIndex;
	for (sButtonIndex = 0; sButtonIndex < NUM_ELEMENTS(g_apszJoyButtonDescriptions); sButtonIndex++)
		{
		if (rspStricmp(pszButtonDescriptor, g_apszJoyButtonDescriptions[sButtonIndex]) == 0)
			{
			// Found it!
			*psButtonMask = JoyIndexToBitfield(sButtonIndex);
			sRes	= 0;

			break;
			}
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
