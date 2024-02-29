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
// InputEvent.CPP
// 
// History:
//		01/16/97 JMI	Started.
//
//		01/18/97	JMI	Added rspClear*InputEvents() definitions.  They were
//							already declared in the .h file.
//
//////////////////////////////////////////////////////////////////////////////
//
// API to handle input events as one entity.  Merely an interface to the
// separate Blue APIs fro input events.
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
	#include "GREEN/InputEvent/InputEvent.h"
#else
	#include "InputEvent.h"
#endif

//////////////////////////////////////////////////////////////////////////////
// Module specific macros.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Module specific typedefs.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Exported (extern) variables.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Module specific (static) variables / Instantiate class statics.
//////////////////////////////////////////////////////////////////////////////

static RInputEvent	ms_ieMouse		=			// Next mouse input event.
	{
	RInputEvent::Mouse,
	};

static short			ms_sMouseEvent	= FALSE;	// TRUE, if ms_ieMouse contains
															// a valid input event.

static RInputEvent	ms_ieKey			=			// Next key input event.
	{
	RInputEvent::Key,
	};

static short			ms_sKeyEvent	= FALSE;	// TRUE, if ms_ieKey contains
															// a valid input event.

static XInputState	ms_XInputState = {};		// CURRENT XInput state.

//////////////////////////////////////////////////////////////////////////////
// Module specific (static) protos.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Functions.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Gets the next input event.
//
//////////////////////////////////////////////////////////////////////////////
short rspGetNextInputEvent(	// Returns 1 if there is an event, 0 if none.
	RInputEvent*	pie)			// Out: Filled with input event type and details.
	{
	short	sGotEvent	= 1;	// Assume we have one.

	// If no current mouse event . . .
	if (ms_sMouseEvent == FALSE)
		{
		// Get one, if avaliable.
		if (rspGetMouseEvent(
			&ms_ieMouse.sPosX, 
			&ms_ieMouse.sPosY, 
			&ms_ieMouse.sButtons, 
			&ms_ieMouse.lTime, 
			&ms_ieMouse.sEvent) == 1)
			{
			ms_sMouseEvent	= TRUE;
			}
		}

	// If no current key event . . .
	if (ms_sKeyEvent == FALSE)
		{
		// Get one, if avaliable.
		if (rspGetKey(
			&ms_ieKey.lKey, 
			&ms_ieKey.lTime) == 1)
			{
			ms_sKeyEvent	= TRUE;
			}
		}

	// If there are two events . . .
	if (ms_sMouseEvent != FALSE && ms_sKeyEvent != FALSE)
		{
		// Pick earlier one . . .
		if (ms_ieMouse.lTime < ms_ieKey.lTime)
			{
			*pie	= ms_ieMouse;
			// Invalidate current mouse event.
			ms_sMouseEvent	= FALSE;
			}
		else
			{
			*pie	= ms_ieKey;
			// Invalidate current key event.
			ms_sKeyEvent	= FALSE;
			}
		}
	else
		{
		// Pick whichever is available . . .
		if (ms_sMouseEvent != FALSE)
			{
			*pie	= ms_ieMouse;
			// Invalidate current mouse event.
			ms_sMouseEvent	= FALSE;
			}
		else
			{
			if (ms_sKeyEvent != FALSE)
				{
				*pie	= ms_ieKey;
				// Invalidate current key event.
				ms_sKeyEvent	= FALSE;
				}
			else
				{
				// No event.
				sGotEvent	= 0;
				}
			}
		}

	return sGotEvent;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Clears any pending input events.  Calls rspClearMouseEvents() and
// rspClearKeyEvents().
//
//////////////////////////////////////////////////////////////////////////////
void rspClearAllInputEvents(void)	// Returns nothing.
	{
	// Invalidate any stored events.
	ms_sKeyEvent	= FALSE;
	ms_sMouseEvent	= FALSE;
	// Empty queues.
	rspClearKeyEvents();
	rspClearMouseEvents();
	}

//////////////////////////////////////////////////////////////////////////////
//
// Clears any pending key input events.  Calls rspClearKeyEvents().
//
//////////////////////////////////////////////////////////////////////////////
void rspClearKeyInputEvents(void)	// Returns nothing.
	{
	// Invalidate any stored event.
	ms_sKeyEvent	= FALSE;
	// Empty queues.
	rspClearKeyEvents();
	}

//////////////////////////////////////////////////////////////////////////////
//
// Clears any pending mouse input events.  Calls rspClearMouseEvents().
//
//////////////////////////////////////////////////////////////////////////////
void rspClearMouseInputEvents(void)	// Returns nothing.
	{
	// Invalidate any stored event.
	ms_sMouseEvent	= FALSE;
	// Empty queues.
	rspClearMouseEvents();
	}

///////////////////////////////////////////////////////////////////////////////
// XInput State stuff. Added for menu control.
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Clears current XInput state.
///////////////////////////////////////////////////////////////////////////////
void ClearXInputState()
{
	for (int i = 0; i < XInputButtons; i++)
		ms_XInputState.ButtonState[i] = XInputState::Release;
	for (int i = 0; i < XInputAxes; i++)
		ms_XInputState.AxisState[i] = XInputState::Release;
}

///////////////////////////////////////////////////////////////////////////////
// Updates joystick and returns current button state.
///////////////////////////////////////////////////////////////////////////////
short GetXInputState(XInputState* xis)
{
	rspUpdateJoy(0);
	return GetXInputStateNoUpdate(xis);
}
short GetLastXInputState(XInputState* xis)
{
	if (xis)
	{
		memcpy(xis, &ms_XInputState, sizeof(XInputState));
		return 1;
	}
	return 0;
}
short GetXInputStateNoUpdate(XInputState* xis)
{
	// Get current input state from Blue
	U32 u32Buttons = 0;
	U32 u32Axes = 0;
	rspGetJoyState(0, &u32Buttons, &u32Axes);

	for (int i = 0; i < XInputButtons; i++)
		xis->ButtonState[i] = XInputState::None;
	for (int i = 0; i < XInputAxes; i++)
		xis->AxisState[i] = XInputState::None;

	//printf("BUTTONS %X AXES %X\n", u32Buttons, u32Axes);
	// STICK1 8 4 2 1 U D L R
	// STICK2 80 40 20 10 U D L R
	// LT 100
	// RT 400

	// STICK1 U=4 D=3 L=2 R=1
	// STICK2 U=8 D=7 L=6 R=5
	// LT PRESS=9
	// RT PRESS=10

	// This is pretty clunky. Again.
	if ((u32Axes & RSP_JOY_X_POS) && (ms_XInputState.AxisState[1] == XInputState::Release))
		xis->AxisState[1] = ms_XInputState.AxisState[1] = XInputState::Press;
	else if ((u32Axes & RSP_JOY_X_POS) && (ms_XInputState.AxisState[1] == XInputState::Press))
		xis->AxisState[1] = XInputState::None;
	else if (!(u32Axes & RSP_JOY_X_POS) && (ms_XInputState.AxisState[1] == XInputState::Press))
		xis->AxisState[1] = ms_XInputState.AxisState[1] = XInputState::Release;
	else
		xis->AxisState[1] = XInputState::None;
	if ((u32Axes & RSP_JOY_X_NEG) && (ms_XInputState.AxisState[2] == XInputState::Release))
		xis->AxisState[2] = ms_XInputState.AxisState[2] = XInputState::Press;
	else if ((u32Axes & RSP_JOY_X_NEG) && (ms_XInputState.AxisState[2] == XInputState::Press))
		xis->AxisState[2] = XInputState::None;
	else if (!(u32Axes & RSP_JOY_X_NEG) && (ms_XInputState.AxisState[2] == XInputState::Press))
		xis->AxisState[2] = ms_XInputState.AxisState[2] = XInputState::Release;
	else
		xis->AxisState[2] = XInputState::None;
	if ((u32Axes & RSP_JOY_Y_POS) && (ms_XInputState.AxisState[3] == XInputState::Release))
		xis->AxisState[3] = ms_XInputState.AxisState[3] = XInputState::Press;
	else if ((u32Axes & RSP_JOY_Y_POS) && (ms_XInputState.AxisState[3] == XInputState::Press))
		xis->AxisState[3] = XInputState::None;
	else if (!(u32Axes & RSP_JOY_Y_POS) && (ms_XInputState.AxisState[3] == XInputState::Press))
		xis->AxisState[3] = ms_XInputState.AxisState[3] = XInputState::Release;
	else
		xis->AxisState[3] = XInputState::None;
	if ((u32Axes & RSP_JOY_Y_NEG) && (ms_XInputState.AxisState[4] == XInputState::Release))
		xis->AxisState[4] = ms_XInputState.AxisState[4] = XInputState::Press;
	else if ((u32Axes & RSP_JOY_Y_NEG) && (ms_XInputState.AxisState[4] == XInputState::Press))
		xis->AxisState[4] = XInputState::None;
	else if (!(u32Axes & RSP_JOY_Y_NEG) && (ms_XInputState.AxisState[4] == XInputState::Press))
		xis->AxisState[4] = ms_XInputState.AxisState[4] = XInputState::Release;
	else
		xis->AxisState[4] = XInputState::None;
	if ((u32Axes & RSP_JOY_Z_POS) && (ms_XInputState.AxisState[5] == XInputState::Release))
		xis->AxisState[5] = ms_XInputState.AxisState[5] = XInputState::Press;
	else if ((u32Axes & RSP_JOY_Z_POS) && (ms_XInputState.AxisState[5] == XInputState::Press))
		xis->AxisState[5] = XInputState::None;
	else if (!(u32Axes & RSP_JOY_Z_POS) && (ms_XInputState.AxisState[5] == XInputState::Press))
		xis->AxisState[5] = ms_XInputState.AxisState[5] = XInputState::Release;
	else
		xis->AxisState[5] = XInputState::None;
	if ((u32Axes & RSP_JOY_Z_NEG) && (ms_XInputState.AxisState[6] == XInputState::Release))
		xis->AxisState[6] = ms_XInputState.AxisState[6] = XInputState::Press;
	else if ((u32Axes & RSP_JOY_Z_NEG) && (ms_XInputState.AxisState[6] == XInputState::Press))
		xis->AxisState[6] = XInputState::None;
	else if (!(u32Axes & RSP_JOY_Z_NEG) && (ms_XInputState.AxisState[6] == XInputState::Press))
		xis->AxisState[6] = ms_XInputState.AxisState[6] = XInputState::Release;
	else
		xis->AxisState[6] = XInputState::None;
	if ((u32Axes & RSP_JOY_W_POS) && (ms_XInputState.AxisState[7] == XInputState::Release))
		xis->AxisState[7] = ms_XInputState.AxisState[7] = XInputState::Press;
	else if ((u32Axes & RSP_JOY_W_POS) && (ms_XInputState.AxisState[7] == XInputState::Press))
		xis->AxisState[7] = XInputState::None;
	else if (!(u32Axes & RSP_JOY_W_POS) && (ms_XInputState.AxisState[7] == XInputState::Press))
		xis->AxisState[7] = ms_XInputState.AxisState[7] = XInputState::Release;
	else
		xis->AxisState[7] = XInputState::None;
	if ((u32Axes & RSP_JOY_U_POS) && (ms_XInputState.AxisState[8] == XInputState::Release))
		xis->AxisState[8] = ms_XInputState.AxisState[4] = XInputState::Press;
	else if ((u32Axes & RSP_JOY_U_POS) && (ms_XInputState.AxisState[8] == XInputState::Press))
		xis->AxisState[8] = XInputState::None;
	else if (!(u32Axes & RSP_JOY_U_POS) && (ms_XInputState.AxisState[8] == XInputState::Press))
		xis->AxisState[8] = ms_XInputState.AxisState[8] = XInputState::Release;
	else
		xis->AxisState[8] = XInputState::None;
	if ((u32Axes & RSP_JOY_V_POS) && (ms_XInputState.AxisState[9] == XInputState::Release))
		xis->AxisState[9] = ms_XInputState.AxisState[9] = XInputState::Press;
	else if ((u32Axes & RSP_JOY_V_POS) && (ms_XInputState.AxisState[9] == XInputState::Press))
		xis->AxisState[9] = XInputState::None;
	else if (!(u32Axes & RSP_JOY_V_POS) && (ms_XInputState.AxisState[9] == XInputState::Press))
		xis->AxisState[9] = ms_XInputState.AxisState[9] = XInputState::Release;
	else
		xis->AxisState[9] = XInputState::None;

	if ((u32Buttons & RSP_JOY_BUT_1) && (ms_XInputState.ButtonState[1] == XInputState::Release))
		xis->ButtonState[1] = ms_XInputState.ButtonState[1] = XInputState::Press;
	else if ((u32Buttons & RSP_JOY_BUT_1) && (ms_XInputState.ButtonState[1] == XInputState::Press))
		xis->ButtonState[1] = XInputState::None;
	else if (!(u32Buttons & RSP_JOY_BUT_1) && (ms_XInputState.ButtonState[1] == XInputState::Press))
		xis->ButtonState[1] = ms_XInputState.ButtonState[1] = XInputState::Release;
	else
		xis->ButtonState[1] = XInputState::None;
	if ((u32Buttons & RSP_JOY_BUT_2) && (ms_XInputState.ButtonState[2] == XInputState::Release))
		xis->ButtonState[2] = ms_XInputState.ButtonState[2] = XInputState::Press;
	else if ((u32Buttons & RSP_JOY_BUT_2) && (ms_XInputState.ButtonState[2] == XInputState::Press))
		xis->ButtonState[2] = XInputState::None;
	else if (!(u32Buttons & RSP_JOY_BUT_2) && (ms_XInputState.ButtonState[2] == XInputState::Press))
		xis->ButtonState[2] = ms_XInputState.ButtonState[2] = XInputState::Release;
	else
		xis->ButtonState[2] = XInputState::None;
	if ((u32Buttons & RSP_JOY_BUT_3) && (ms_XInputState.ButtonState[3] == XInputState::Release))
		xis->ButtonState[3] = ms_XInputState.ButtonState[3] = XInputState::Press;
	else if ((u32Buttons & RSP_JOY_BUT_3) && (ms_XInputState.ButtonState[3] == XInputState::Press))
		xis->ButtonState[3] = XInputState::None;
	else if (!(u32Buttons & RSP_JOY_BUT_3) && (ms_XInputState.ButtonState[3] == XInputState::Press))
		xis->ButtonState[3] = ms_XInputState.ButtonState[3] = XInputState::Release;
	else
		xis->ButtonState[3] = XInputState::None;
	if ((u32Buttons & RSP_JOY_BUT_4) && (ms_XInputState.ButtonState[4] == XInputState::Release))
		xis->ButtonState[4] = ms_XInputState.ButtonState[4] = XInputState::Press;
	else if ((u32Buttons & RSP_JOY_BUT_4) && (ms_XInputState.ButtonState[4] == XInputState::Press))
		xis->ButtonState[4] = XInputState::None;
	else if (!(u32Buttons & RSP_JOY_BUT_4) && (ms_XInputState.ButtonState[4] == XInputState::Press))
		xis->ButtonState[4] = ms_XInputState.ButtonState[4] = XInputState::Release;
	else
		xis->ButtonState[4] = XInputState::None;
	if ((u32Buttons & RSP_JOY_BUT_5) && (ms_XInputState.ButtonState[5] == XInputState::Release))
		xis->ButtonState[5] = ms_XInputState.ButtonState[5] = XInputState::Press;
	else if ((u32Buttons & RSP_JOY_BUT_5) && (ms_XInputState.ButtonState[5] == XInputState::Press))
		xis->ButtonState[5] = XInputState::None;
	else if (!(u32Buttons & RSP_JOY_BUT_5) && (ms_XInputState.ButtonState[5] == XInputState::Press))
		xis->ButtonState[5] = ms_XInputState.ButtonState[5] = XInputState::Release;
	else
		xis->ButtonState[5] = XInputState::None;
	if ((u32Buttons & RSP_JOY_BUT_6) && (ms_XInputState.ButtonState[6] == XInputState::Release))
		xis->ButtonState[6] = ms_XInputState.ButtonState[6] = XInputState::Press;
	else if ((u32Buttons & RSP_JOY_BUT_6) && (ms_XInputState.ButtonState[6] == XInputState::Press))
		xis->ButtonState[6] = XInputState::None;
	else if (!(u32Buttons & RSP_JOY_BUT_6) && (ms_XInputState.ButtonState[6] == XInputState::Press))
		xis->ButtonState[6] = ms_XInputState.ButtonState[6] = XInputState::Release;
	else
		xis->ButtonState[6] = XInputState::None;
	if ((u32Buttons & RSP_JOY_BUT_7) && (ms_XInputState.ButtonState[7] == XInputState::Release))
		xis->ButtonState[7] = ms_XInputState.ButtonState[7] = XInputState::Press;
	else if ((u32Buttons & RSP_JOY_BUT_7) && (ms_XInputState.ButtonState[7] == XInputState::Press))
		xis->ButtonState[7] = XInputState::None;
	else if (!(u32Buttons & RSP_JOY_BUT_7) && (ms_XInputState.ButtonState[7] == XInputState::Press))
		xis->ButtonState[7] = ms_XInputState.ButtonState[7] = XInputState::Release;
	else
		xis->ButtonState[7] = XInputState::None;
	if ((u32Buttons & RSP_JOY_BUT_8) && (ms_XInputState.ButtonState[8] == XInputState::Release))
		xis->ButtonState[8] = ms_XInputState.ButtonState[8] = XInputState::Press;
	else if ((u32Buttons & RSP_JOY_BUT_8) && (ms_XInputState.ButtonState[8] == XInputState::Press))
		xis->ButtonState[8] = XInputState::None;
	else if (!(u32Buttons & RSP_JOY_BUT_8) && (ms_XInputState.ButtonState[8] == XInputState::Press))
		xis->ButtonState[8] = ms_XInputState.ButtonState[8] = XInputState::Release;
	else
		xis->ButtonState[8] = XInputState::None;
	if ((u32Buttons & RSP_JOY_BUT_9) && (ms_XInputState.ButtonState[9] == XInputState::Release))
		xis->ButtonState[9] = ms_XInputState.ButtonState[9] = XInputState::Press;
	else if ((u32Buttons & RSP_JOY_BUT_9) && (ms_XInputState.ButtonState[9] == XInputState::Press))
		xis->ButtonState[9] = XInputState::None;
	else if (!(u32Buttons & RSP_JOY_BUT_9) && (ms_XInputState.ButtonState[9] == XInputState::Press))
		xis->ButtonState[9] = ms_XInputState.ButtonState[9] = XInputState::Release;
	else
		xis->ButtonState[9] = XInputState::None;
	if ((u32Buttons & RSP_JOY_BUT_10) && (ms_XInputState.ButtonState[10] == XInputState::Release))
		xis->ButtonState[10] = ms_XInputState.ButtonState[10] = XInputState::Press;
	else if ((u32Buttons & RSP_JOY_BUT_10) && (ms_XInputState.ButtonState[10] == XInputState::Press))
		xis->ButtonState[10] = XInputState::None;
	else if (!(u32Buttons & RSP_JOY_BUT_10) && (ms_XInputState.ButtonState[10] == XInputState::Press))
		xis->ButtonState[10] = ms_XInputState.ButtonState[10] = XInputState::Release;
	else
		xis->ButtonState[10] = XInputState::None;
	if ((u32Buttons & RSP_JOY_BUT_11) && (ms_XInputState.ButtonState[11] == XInputState::Release))
		xis->ButtonState[11] = ms_XInputState.ButtonState[11] = XInputState::Press;
	else if ((u32Buttons & RSP_JOY_BUT_11) && (ms_XInputState.ButtonState[11] == XInputState::Press))
		xis->ButtonState[11] = XInputState::None;
	else if (!(u32Buttons & RSP_JOY_BUT_11) && (ms_XInputState.ButtonState[11] == XInputState::Press))
		xis->ButtonState[11] = ms_XInputState.ButtonState[11] = XInputState::Release;
	else
		xis->ButtonState[11] = XInputState::None;
	if ((u32Buttons & RSP_JOY_BUT_12) && (ms_XInputState.ButtonState[12] == XInputState::Release))
		xis->ButtonState[12] = ms_XInputState.ButtonState[12] = XInputState::Press;
	else if ((u32Buttons & RSP_JOY_BUT_12) && (ms_XInputState.ButtonState[12] == XInputState::Press))
		xis->ButtonState[12] = XInputState::None;
	else if (!(u32Buttons & RSP_JOY_BUT_12) && (ms_XInputState.ButtonState[12] == XInputState::Press))
		xis->ButtonState[12] = ms_XInputState.ButtonState[12] = XInputState::Release;
	else
		xis->ButtonState[12] = XInputState::None;
	if ((u32Buttons & RSP_JOY_BUT_13) && (ms_XInputState.ButtonState[13] == XInputState::Release))
		xis->ButtonState[13] = ms_XInputState.ButtonState[13] = XInputState::Press;
	else if ((u32Buttons & RSP_JOY_BUT_13) && (ms_XInputState.ButtonState[13] == XInputState::Press))
		xis->ButtonState[13] = XInputState::None;
	else if (!(u32Buttons & RSP_JOY_BUT_13) && (ms_XInputState.ButtonState[13] == XInputState::Press))
		xis->ButtonState[13] = ms_XInputState.ButtonState[13] = XInputState::Release;
	else
		xis->ButtonState[13] = XInputState::None;
	if ((u32Buttons & RSP_JOY_BUT_14) && (ms_XInputState.ButtonState[14] == XInputState::Release))
		xis->ButtonState[14] = ms_XInputState.ButtonState[14] = XInputState::Press;
	else if ((u32Buttons & RSP_JOY_BUT_14) && (ms_XInputState.ButtonState[14] == XInputState::Press))
		xis->ButtonState[14] = XInputState::None;
	else if (!(u32Buttons & RSP_JOY_BUT_14) && (ms_XInputState.ButtonState[14] == XInputState::Press))
		xis->ButtonState[14] = ms_XInputState.ButtonState[14] = XInputState::Release;
	else
		xis->ButtonState[14] = XInputState::None;
	if ((u32Buttons & RSP_JOY_BUT_15) && (ms_XInputState.ButtonState[15] == XInputState::Release))
		xis->ButtonState[15] = ms_XInputState.ButtonState[15] = XInputState::Press;
	else if ((u32Buttons & RSP_JOY_BUT_15) && (ms_XInputState.ButtonState[15] == XInputState::Press))
		xis->ButtonState[15] = XInputState::None;
	else if (!(u32Buttons & RSP_JOY_BUT_15) && (ms_XInputState.ButtonState[15] == XInputState::Press))
		xis->ButtonState[15] = ms_XInputState.ButtonState[15] = XInputState::Release;
	else
		xis->ButtonState[15] = XInputState::None;
	if ((u32Buttons & RSP_JOY_BUT_16) && (ms_XInputState.ButtonState[16] == XInputState::Release))
		xis->ButtonState[16] = ms_XInputState.ButtonState[16] = XInputState::Press;
	else if ((u32Buttons & RSP_JOY_BUT_16) && (ms_XInputState.ButtonState[16] == XInputState::Press))
		xis->ButtonState[16] = XInputState::None;
	else if (!(u32Buttons & RSP_JOY_BUT_16) && (ms_XInputState.ButtonState[16] == XInputState::Press))
		xis->ButtonState[16] = ms_XInputState.ButtonState[16] = XInputState::Release;
	else
		xis->ButtonState[16] = XInputState::None;
	if ((u32Buttons & RSP_JOY_BUT_17) && (ms_XInputState.ButtonState[17] == XInputState::Release))
		xis->ButtonState[17] = ms_XInputState.ButtonState[17] = XInputState::Press;
	else if ((u32Buttons & RSP_JOY_BUT_17) && (ms_XInputState.ButtonState[17] == XInputState::Press))
		xis->ButtonState[17] = XInputState::None;
	else if (!(u32Buttons & RSP_JOY_BUT_17) && (ms_XInputState.ButtonState[17] == XInputState::Press))
		xis->ButtonState[17] = ms_XInputState.ButtonState[17] = XInputState::Release;
	else
		xis->ButtonState[17] = XInputState::None;

	return 1; //!! FIXME: return 0 if controller isn't plugged in?
}

///////////////////////////////////////////////////////////////////////////////
// Checks current joystick state and returns 1 if any button is pressed.
///////////////////////////////////////////////////////////////////////////////
short IsXInputButtonPressed()
{
	XInputState xis = {};

	GetXInputState(&xis);

	for (int i = 0; i < XInputButtons; i++)
		if (xis.ButtonState[i] == XInputState::Press)
			return 1;

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// EOF
///////////////////////////////////////////////////////////////////////////////
