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
///////////////////////////////////////////////////////////////////////////////
//
//	joy.cpp
// 
// History:
//		08/29/95 JMI	Started.
//
//
//////////////////////////////////////////////////////////////////////////////
//
// Handles keyboard emulation of joystick stuff.
//
//////////////////////////////////////////////////////////////////////////////

#include "common/system.h"

#include "common/bjoy.h"

#include "common/bkey.h"

#include "common/bdebug.h"


#include "joy/joy.h"

//////////////////////////////////////////////////////////////////////////////
// Macros.
//////////////////////////////////////////////////////////////////////////////
#define NUM_JOYSTICKS	2

#define NUM_BUTTONS		4

#define NUM_DIRS			6

//////////////////////////////////////////////////////////////////////////////
// Module specific (static) variables.
//////////////////////////////////////////////////////////////////////////////
static JOYSTATE	ms_ajsCurr[NUM_JOYSTICKS];		// Current joystick state.
static JOYSTATE	ms_ajsPrev[NUM_JOYSTICKS];		// Previous joystick state.

static UCHAR		ms_aucButKeys[NUM_JOYSTICKS][NUM_BUTTONS]	= { 0, };
static UCHAR		ms_aucDirKeys[NUM_JOYSTICKS][NUM_DIRS]		= { 0, };

//////////////////////////////////////////////////////////////////////////////
// Externally callable functions.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Sets the ucKey as the key representing usState on sJoy joystick.
// Returns 0 on success.
//
//////////////////////////////////////////////////////////////////////////////
extern short Joy_SetKey(short sJoy, UCHAR ucKey, USHORT usState)
	{
	short	sRes	= 0;	// Assume success.

	ASSERT(sJoy >= 0 && sJoy < NUM_JOYSTICKS);
	
	// Get portion for buttons.
	USHORT	usButState	= (usState & JOY_BUT);
	// If any buttons represented . . .
	if (usButState != 0)
		{
		// For every bit represented store this key.
		for (short i = 0; i < sizeof(ms_aucButKeys[sJoy]); i++)
			{
			if (usButState & (0x0001 << i))
				{
				ms_aucButKeys[sJoy][i] = ucKey;
				}
			}
		}

	// Get portion for dirs.
	USHORT	usDirState	= (usState & JOY_DIR_STATES);
	// If any directionss represented . . .
	if (usDirState != 0)
		{
		// For every bit represented store this key.
		for (short i = 0; i < sizeof(ms_aucDirKeys[sJoy]); i++)
			{
			if (usDirState & (0x0010 << i))
				{
				ms_aucDirKeys[sJoy][i] = ucKey;
				}
			}
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Updates joystick sJoy's current state and makes the current state the 
// previous.
// Returns 0 on success.
//
//////////////////////////////////////////////////////////////////////////////
extern short Joy_Update(short sJoy)
	{
	short sRes = 0; // Assume success.

	ASSERT(sJoy >= 0 && sJoy < NUM_JOYSTICKS);
	
	if (Blu_UpdateJoy(sJoy) == 0)
		{
		// Copy current state to previous.
		ms_ajsPrev[sJoy] = ms_ajsCurr[sJoy];
		// Get new state.
		Blu_GetJoyState(sJoy, &ms_ajsCurr[sJoy]);

		KEYBOARD	kb;
		// Get key state.
		if (Blu_GetKeyboard(&kb) == 0)
			{
			// Update for keys.
			UCHAR	uc;
			// Buttons:
			for (short i = 0; i < sizeof(ms_aucButKeys[sJoy]); i++)
				{
				// Get key.
				uc = ms_aucButKeys[sJoy][i];

				if (BITINDEXARRAY(kb.auc,uc))
					{
					ms_ajsCurr[sJoy].us |= (0x0001 << i);
					}
				}

			// Directions:
			for (i = 0; i < sizeof(ms_aucDirKeys[sJoy]); i++)
				{
				// Get key.
				uc = ms_aucDirKeys[sJoy][i];

				if (BITINDEXARRAY(kb.auc,uc))
					{
					ms_ajsCurr[sJoy].us |= (0x0010 << i);
					}
				}
			}
		else
			{
			TRACE("Joy_Update(): Unable to get keyboard state.\n");
			sRes = -2;
			}
		}
	else
		{
		TRACE("Joy_Update(): Unable to get joystick state.\n");
		sRes = -1;
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Puts the coordinates of joystick sJoy's position in your longs.
// Returns nothing.
//
//////////////////////////////////////////////////////////////////////////////
extern void Joy_GetPos(short sJoy, long *px, long *py, long *pz)
	{
	ASSERT(sJoy >= 0 && sJoy < NUM_JOYSTICKS);
	
	Blu_GetJoyPos(sJoy, px, py, pz);
	}

//////////////////////////////////////////////////////////////////////////////
//
// Puts the coordinates of the previous joystick sJoy's position in your longs.
// Returns nothing.
//
//////////////////////////////////////////////////////////////////////////////
extern void Joy_GetPrevPos(short sJoy, long *px, long *py, long *pz)
	{
	ASSERT(sJoy >= 0 && sJoy < NUM_JOYSTICKS);

	Blu_GetJoyPrevPos(sJoy, px, py, pz);
	}

//////////////////////////////////////////////////////////////////////////////
//
// Returns the current joystick sJoy's state.
//
//////////////////////////////////////////////////////////////////////////////
extern USHORT Joy_GetState(short sJoy)
	{
	ASSERT(sJoy >= 0 && sJoy < NUM_JOYSTICKS);

	return ms_ajsCurr[sJoy].us;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Returns the previous joystick sJoy's state.
//
//////////////////////////////////////////////////////////////////////////////
extern USHORT Joy_GetPrevState(short sJoy)
	{
	ASSERT(sJoy >= 0 && sJoy < NUM_JOYSTICKS);

	return ms_ajsPrev[sJoy].us;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Places the current joystick sJoy's state in pjs.
//
//////////////////////////////////////////////////////////////////////////////
extern void Joy_GetState(short sJoy, PJOYSTATE pjs)
	{
	ASSERT(sJoy >= 0 && sJoy < NUM_JOYSTICKS);

	*pjs = ms_ajsCurr[sJoy];
	}

//////////////////////////////////////////////////////////////////////////////
//
// Places the previous joystick sJoy's state in pjs.
//
//////////////////////////////////////////////////////////////////////////////
extern void Joy_GetPrevState(short sJoy, PJOYSTATE pjs)
	{
	ASSERT(sJoy >= 0 && sJoy < NUM_JOYSTICKS);

	*pjs = ms_ajsPrev[sJoy];
	}

//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////

