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
// InputEvent.H
// 
// History:
//		01/16/96 JMI	Started.
//
//////////////////////////////////////////////////////////////////////////////
//
// API to handle input events as one entity.  Merely an interface to the
// separate Blue APIs fro input events.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef INPUT_EVENT_H
#define INPUT_EVENT_H

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
// Typedefs.
//////////////////////////////////////////////////////////////////////////////

// Input event structure for rsp*InputEvent[s]() functions.
typedef struct
	{
	typedef enum
		{
		None,		// No event.  Use this value to indicate no event.
		Mouse,	// Mouse event.
		Key,		// Keyboard event.
		Joy		// Joystick event (NYI) ???
		} Type;

	Type	type;		// Type of event { Mouse, Key, Joy }.
	long	lTime;	// Time at which event occurred.
	short	sUsed;	// Indicates that this event has been used.
						// Set this if your API utilizes this event so that others
						// know that it's already been absorbed.
	long	lUser;	// User value.  Use as you please.

	union
		{
		struct	// Mouse event.
			{
			short sPosX;		// X position of event.
			short	sPosY;		// Y position of event.
			short	sButtons;	// Buttons status at event (ala rspGetMouseEvent).
			short	sEvent;		// Event type (ala rspGetMouseEvent).
			};
		
		struct	// Keyboard event.
			{
			long	lKey;			// Key value and modifiers (ala rspGetKey).
			};
		};
	} RInputEvent;

// Input event structure for XInput events. (Used only for menu navigation)
const int XInputButtons = 18;
const int XInputAxes = 12;
#define XINPUT_BUTTON_MENUUP g_InputSettings.m_sJoyMenuUpButton
#define XINPUT_BUTTON_MENUDOWN g_InputSettings.m_sJoyMenuDownButton
#define XINPUT_BUTTON_MENULEFT g_InputSettings.m_sJoyMenuLeftButton
#define XINPUT_BUTTON_MENURIGHT g_InputSettings.m_sJoyMenuRightButton
#define XINPUT_AXIS_MENUUP g_InputSettings.m_sJoyMenuUpAxis
#define XINPUT_AXIS_MENUDOWN g_InputSettings.m_sJoyMenuDownAxis
#define XINPUT_AXIS_MENULEFT g_InputSettings.m_sJoyMenuLeftAxis
#define XINPUT_AXIS_MENURIGHT g_InputSettings.m_sJoyMenuRightAxis
#define XINPUT_BUTTON_CONFIRM g_InputSettings.m_sJoyMenuConfirmButton
#define XINPUT_BUTTON_START g_InputSettings.m_sJoyStartButton
#define XINPUT_BUTTON_BACK g_InputSettings.m_sJoyMenuBackButton
#define XINPUT_BUTTON_BACK2 g_InputSettings.m_sJoyMenuBackButton2
typedef struct
{
	typedef enum
	{
		None,		// Nothing to report on this button.
		Press,		// This button was pressed.
		Release		// This button was released.
	} EButtonState;	// State of button (pushed, released, or None)

	EButtonState ButtonState[XInputButtons];	// States of all 16 buttons
	EButtonState AxisState[XInputAxes];			// States of all 6 axes
} XInputState;

//////////////////////////////////////////////////////////////////////////////
// Protos.
//////////////////////////////////////////////////////////////////////////////

// Clears any pending input events.  Calls rspClearMouseEvents() and
// rspClearKeyEvents().
void rspClearAllInputEvents(void);	// Returns nothing.

// Clears any pending key input events.  Calls rspClearKeyEvents().
void rspClearKeyInputEvents(void);	// Returns nothing.

// Clears any pending mouse input events.  Calls rspClearMouseEvents().
void rspClearMouseInputEvents(void);	// Returns nothing.

// Gets the next input event.
short rspGetNextInputEvent(	// Returns 1 if there is an event, 0 if none.
	RInputEvent*	pie);			// Out: Filled with input event type and details.


// XInput menu controls.

// Clears the XInput state. Returns nothing.
void ClearXInputState();

// Gets the next XInput state.
short GetXInputState(XInputState* xis); // Returns 1 on success, 0 if failure or controller not present.
short GetXInputStateNoUpdate(XInputState* xis);
short GetLastXInputState(XInputState* xis);	// Returns the last known XInputState

// Returns 1 if any button on the controller is pressed.
short IsXInputButtonPressed();

#endif	// INPUT_EVENT_H
//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
