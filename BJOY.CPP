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
//
//	joy.cpp
// 
// History:
//		04/22/95 JMI	Started.
//
//		10/10/97	JMI	Added temporarily directly to Postal but want to make
//							this part of RSPiX soon (need to add event driven
//							stuff for rspGetNextInpuEvent() ).
//
//////////////////////////////////////////////////////////////////////////////
//
// Handles all Windows specific joystick stuff.
//
//////////////////////////////////////////////////////////////////////////////

#include "BLUE/win32/win.h"

#include <mmsystem.h>

#include "bjoy.h"	// For typedefs and macros.

#include "Blue.h"

//////////////////////////////////////////////////////////////////////////////
// Macros.
//////////////////////////////////////////////////////////////////////////////
#define NUM_JOYSTICKS	2

#define MID_THRESHOLD_PERCENT	((float)20)	// In % (e.g., 25 would be 25%).

//////////////////////////////////////////////////////////////////////////////
// Module specific (static) variables.
//////////////////////////////////////////////////////////////////////////////
static JOYCAPS		ms_ajoycaps[NUM_JOYSTICKS];	// Capabilities of each joy.
static USHORT		ms_ausXmids[NUM_JOYSTICKS];	// Middle positions for x.
static USHORT		ms_ausYmids[NUM_JOYSTICKS];	// Middle positions for y.
static USHORT		ms_ausZmids[NUM_JOYSTICKS];	// Middle positions for z.
static USHORT		ms_ausXThresh[NUM_JOYSTICKS];	// Center thresholds for x.
static USHORT		ms_ausYThresh[NUM_JOYSTICKS];	// Center thresholds for y.
static USHORT		ms_ausZThresh[NUM_JOYSTICKS];	// Center thresholds for z.

static JOYINFO		ms_ajiCurr[NUM_JOYSTICKS];		// Current joystick info.
static JOYINFO		ms_ajiPrev[NUM_JOYSTICKS];		// Previous joystick info.

static JOYSTATE	ms_ajsCurr[NUM_JOYSTICKS];		// Current joystick state.
static JOYSTATE	ms_ajsPrev[NUM_JOYSTICKS];		// Previous joystick state.

//////////////////////////////////////////////////////////////////////////////
// Externally callable functions.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// To be called by the Blue library itself, only.
// Initializes the joystick module.  NOTE:  May fail if no joysticks attached.
// Returns nothing in order to remind us that even if the init fails the app
// should still be called.
//
//////////////////////////////////////////////////////////////////////////////
extern void Joy_Init(void)
	{
	short sNum = (short)joyGetNumDevs();

	USHORT	usRangeX, usRangeY, usRangeZ;

	for (short i = 0; i < sNum && i < NUM_JOYSTICKS; i++)
		{
		switch (joyGetDevCaps((i == 0 ? JOYSTICKID1 : JOYSTICKID2), 
									 &(ms_ajoycaps[i]), 
									 sizeof(ms_ajoycaps[i]) 
									)
				 )
			{
			case JOYERR_NOERROR:
				// Calculate ranges.
				usRangeX	= (ms_ajoycaps[i].wXmax - ms_ajoycaps[i].wXmin);
				usRangeY	= (ms_ajoycaps[i].wYmax - ms_ajoycaps[i].wYmin);
				usRangeZ	= (ms_ajoycaps[i].wZmax - ms_ajoycaps[i].wZmin);

				// Calculate middle.
				ms_ausXmids[i]		= ms_ajoycaps[i].wXmin + usRangeX / 2;
				ms_ausYmids[i]		= ms_ajoycaps[i].wYmin + usRangeY / 2;
				ms_ausZmids[i]		= ms_ajoycaps[i].wZmin + usRangeZ / 2;
				ms_ausXThresh[i]	= (float)usRangeX * (float)((float)MID_THRESHOLD_PERCENT / 100.0F);
				ms_ausYThresh[i]	= (float)usRangeY * (float)((float)MID_THRESHOLD_PERCENT / 100.0F);
				ms_ausZThresh[i]	= (float)usRangeZ * (float)((float)MID_THRESHOLD_PERCENT / 100.0F);

#if 0
				TRACE("wMid:\t0x%04X\n"
						"wPid:\t0x%04X\n"
						"szPname:\t%s\n"
						"wXmin:\t%u\n"
						"wXmax:\t%u\n"
						"wYmin:\t%u\n"
						"wYmax:\t%u\n"
						"wZmin:\t%u\n"
						"wZmax:\t%u\n"
						"wNumButtons:\t%u\n"
						"wPeriodMin:\t%u\n"
						"wPeriodMax:\t%u\n",
						ms_ajoycaps[i].wMid,
						ms_ajoycaps[i].wPid,
						ms_ajoycaps[i].szPname,
						ms_ajoycaps[i].wXmin,
						ms_ajoycaps[i].wXmax,
						ms_ajoycaps[i].wYmin,
						ms_ajoycaps[i].wYmax,
						ms_ajoycaps[i].wZmin,
						ms_ajoycaps[i].wZmax,
						ms_ajoycaps[i].wNumButtons,
						ms_ajoycaps[i].wPeriodMin,
						ms_ajoycaps[i].wPeriodMax);
#endif
				// Success.
				break;
			case MMSYSERR_NODRIVER:
				TRACE("BLUE:Joy_Init(): The joystick driver is not present.\n");
				break;
			case MMSYSERR_INVALPARAM:
				TRACE("BLUE:Joy_Init(): An invalid parameter was passed.\n");
				break;
			}
		}
	}

//////////////////////////////////////////////////////////////////////////////
//
// Updates joystick sJoy's current state and makes the current state the 
// previous.
// Returns 0 on success.
//
//////////////////////////////////////////////////////////////////////////////
extern short Blu_UpdateJoy(short sJoy)
	{
	short sRes = 0; // Assume success.

	ASSERT(sJoy == 0 || sJoy == 1);

	// Get the joystick info.  Only update our variables, if successful.
	JOYINFO jiTemp;
	switch (joyGetPos( (sJoy == 0) ? JOYSTICKID1 : JOYSTICKID2, &jiTemp) )
		{
		case JOYERR_NOERROR:
			// Success.  Set values.

			// Set previous joyinfo to current.
			ms_ajiPrev[sJoy]	= ms_ajiCurr[sJoy];

			// Set the current to the temp.
			ms_ajiCurr[sJoy]	= jiTemp;
			
			// Set previous joy state to current.
			ms_ajsPrev[sJoy]	= ms_ajsCurr[sJoy];

			// Fill in new state fields.
			ms_ajsCurr[sJoy].button1	= (((jiTemp.wButtons & JOY_BUTTON1) != 0) ? 1 : 0);
			ms_ajsCurr[sJoy].button2	= (((jiTemp.wButtons & JOY_BUTTON2) != 0) ? 1 : 0);
			ms_ajsCurr[sJoy].button3	= (((jiTemp.wButtons & JOY_BUTTON3) != 0) ? 1 : 0);
			ms_ajsCurr[sJoy].button4	= (((jiTemp.wButtons & JOY_BUTTON4) != 0) ? 1 : 0);
			ms_ajsCurr[sJoy].left		= (jiTemp.wXpos < (USHORT)(ms_ausXmids[sJoy] - ms_ausXThresh[sJoy]));
			ms_ajsCurr[sJoy].right		= (jiTemp.wXpos > (USHORT)(ms_ausXmids[sJoy] + ms_ausXThresh[sJoy]));
			ms_ajsCurr[sJoy].up			= (jiTemp.wYpos < (USHORT)(ms_ausYmids[sJoy] - ms_ausYThresh[sJoy]));
			ms_ajsCurr[sJoy].down		= (jiTemp.wYpos > (USHORT)(ms_ausYmids[sJoy] + ms_ausYThresh[sJoy]));
			ms_ajsCurr[sJoy].toward		= (jiTemp.wZpos < (USHORT)(ms_ausZmids[sJoy] - ms_ausZThresh[sJoy]));
			ms_ajsCurr[sJoy].away		= (jiTemp.wZpos > (USHORT)(ms_ausZmids[sJoy] + ms_ausZThresh[sJoy]));
			break;
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Puts the coordinates of joystick sJoy's position in your longs.
// Returns nothing.
//
//////////////////////////////////////////////////////////////////////////////
extern void Blu_GetJoyPos(short sJoy, long *px, long *py, long *pz)
	{
	ASSERT(sJoy == 0 || sJoy == 1);

	// Copy the coordinates.
	*px = (long)ms_ajiCurr[sJoy].wXpos;
	*py = (long)ms_ajiCurr[sJoy].wYpos;
	*pz = (long)ms_ajiCurr[sJoy].wZpos;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Puts the coordinates of the previous joystick sJoy's position in your longs.
// Returns nothing.
//
//////////////////////////////////////////////////////////////////////////////
extern void Blu_GetJoyPrevPos(short sJoy, long *px, long *py, long *pz)
	{
	ASSERT(sJoy == 0 || sJoy == 1);

	// Copy the coordinates.
	*px = (long)ms_ajiPrev[sJoy].wXpos;
	*py = (long)ms_ajiPrev[sJoy].wYpos;
	*pz = (long)ms_ajiPrev[sJoy].wZpos;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Returns the current joystick sJoy's state.
//
//////////////////////////////////////////////////////////////////////////////
extern USHORT Blu_GetJoyState(short sJoy)
	{
	ASSERT(sJoy == 0 || sJoy == 1);

	return ms_ajsCurr[sJoy].us;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Returns the previous joystick sJoy's state.
//
//////////////////////////////////////////////////////////////////////////////
extern USHORT Blu_GetJoyPrevState(short sJoy)
	{
	ASSERT(sJoy == 0 || sJoy == 1);

	return ms_ajsPrev[sJoy].us;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Places the current joystick sJoy's state.
//
//////////////////////////////////////////////////////////////////////////////
extern void Blu_GetJoyState(short sJoy, PJOYSTATE pjs)
	{
	ASSERT(sJoy == 0 || sJoy == 1);

	*pjs = ms_ajsCurr[sJoy];
	}

//////////////////////////////////////////////////////////////////////////////
//
// Places the previous joystick sJoy's state.
//
//////////////////////////////////////////////////////////////////////////////
extern void Blu_GetJoyPrevState(short sJoy, PJOYSTATE pjs)
	{
	ASSERT(sJoy == 0 || sJoy == 1);

	*pjs = ms_ajsPrev[sJoy];
	}

//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////

