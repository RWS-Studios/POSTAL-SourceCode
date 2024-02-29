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
//	bjoy.cpp
// 
// History:
//		06/02/04 RCG	Started.
//
//////////////////////////////////////////////////////////////////////////////
//
// Handles all SDL specific joystick stuff.
//
//////////////////////////////////////////////////////////////////////////////

#include "SDL.h"
#include "Blue.h"
#include <math.h>

//////////////////////////////////////////////////////////////////////////////
// Macros.
//////////////////////////////////////////////////////////////////////////////
#define NUM_JOYSTICKS	2

// For simplicity of CHECK_AXIS_* macros, define RSP macros that match the
// Windows name for the W axis (Windows uses R for Rudder).
#define RSP_JOY_R_POS	RSP_JOY_W_POS
#define RSP_JOY_R_NEG	RSP_JOY_W_NEG

#define MID_THRESHOLD_PERCENT	((float)20)	// In % (e.g., 25 would be 25%).

#define SET(p, val)	(p ? *p = val : val)

//////////////////////////////////////////////////////////////////////////////
// Typedefs.
//////////////////////////////////////////////////////////////////////////////

typedef struct
	{
	U32	u32Buttons;
	U32	u32Axes;
#if defined(ALLOW_TWINSTICK)
	Sint16 axis_MoveUpDown, axis_MoveLeftRight, axis_FireUpDown, axis_FireLeftRight;
#endif
	} JoyState;

//////////////////////////////////////////////////////////////////////////////
// Module specific (static) variables.
//////////////////////////////////////////////////////////////////////////////

static JoyState		ms_ajsCurr[NUM_JOYSTICKS];		// Current joystick state.
static JoyState		ms_ajsPrev[NUM_JOYSTICKS];		// Previous joystick state.
static SDL_GameController *ms_Controllers[NUM_JOYSTICKS];

//////////////////////////////////////////////////////////////////////////////
// Internal functions.
//////////////////////////////////////////////////////////////////////////////

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
        if (SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) == -1)
            return;
        SDL_JoystickEventState(SDL_IGNORE);
        SDL_GameControllerEventState(SDL_IGNORE);

        const int sticks = SDL_NumJoysticks();
        int opened = 0;
        int seen = 0;
        for (int i = 0; i < sticks; i++)
        {
            if (!SDL_IsGameController(i))
                continue;
            seen++;
            SDL_GameController *controller = SDL_GameControllerOpen(i);
            if (!controller)
                continue;
            ms_Controllers[opened++] = controller;
            if (opened >= NUM_JOYSTICKS)
                break;
        }

        if (sticks && !seen)
        {
            ASSERT(!opened);
            // !!! FIXME: maybe show a message box saying "we see a stick, configure it in Big Picture"?
        }
	}


static void calcAxis(SDL_GameController *controller, const SDL_GameControllerAxis axis, U32 *bits, const U32 posbit, const U32 negbit)
{
    const Sint16 deadzone = (Sint16) (32767.0f * (MID_THRESHOLD_PERCENT / 100.0f));
    const Sint16 val = SDL_GameControllerGetAxis(controller, axis);
    if ((val < 0) && ((-val) > deadzone))
        *bits |= negbit;
    else if ((val > 0) && (val > deadzone))
        *bits |= posbit;
}

static inline void calcButton(SDL_GameController *controller, const SDL_GameControllerButton button, U32 *bits, const U32 posbit)
{
    if (SDL_GameControllerGetButton(controller, button) != 0)
        *bits |= posbit;
}

static void GetAxesNew(SDL_GameController* controller, Sint16* out_axis_MoveUpDown, Sint16* out_axis_MoveLeftRight, Sint16* out_axis_FireUpDown, Sint16* out_axis_FireLeftRight)
{
	// deadzone shit
#define DEADZONE_PERCENT 25.f
	const Sint16 deadzone = (Sint16)(32767.0f * (DEADZONE_PERCENT / 100.0f));

	// Grab axes
	Sint16 axis_MoveUpDown = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY);
	Sint16 axis_MoveLeftRight = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX);
	Sint16 axis_FireUpDown = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTY);
	Sint16 axis_FireLeftRight = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTX);

	// Clamp to sane values
	if (axis_MoveUpDown < deadzone && axis_MoveUpDown > -deadzone && axis_MoveLeftRight < deadzone && axis_MoveLeftRight > -deadzone)
	{
		axis_MoveUpDown = 0;
		axis_MoveLeftRight = 0;
	}
	if (axis_FireUpDown < deadzone && axis_FireUpDown > -deadzone && axis_FireLeftRight < deadzone && axis_FireLeftRight > -deadzone)
	{
		axis_FireUpDown = 0;
		axis_FireLeftRight = 0;
	}

	if (out_axis_MoveUpDown) *out_axis_MoveUpDown = axis_MoveUpDown;
	if (out_axis_MoveLeftRight) *out_axis_MoveLeftRight = axis_MoveLeftRight;
	if (out_axis_FireUpDown) *out_axis_FireUpDown = axis_FireUpDown;
	if (out_axis_FireLeftRight) *out_axis_FireLeftRight = axis_FireLeftRight;

	//TRACE("SDL Axes: %f.5 %f.5 %f.5 %f.5\n", axis_MoveUpDown, axis_MoveLeftRight, axis_FireUpDown, axis_FireLeftRight);
}

static inline double GetStickAngle(Sint32 axis_X, Sint32 axis_Y)
{
	double d_Angle = 0.f;

	if (axis_X != 0)
	{
		d_Angle = atan((double)-axis_Y / (double)axis_X) * 180.f / M_PI;
		if (axis_X < 0)
			d_Angle -= 180.f;
	}
	else if (axis_Y < 0)
		d_Angle = 90.f;
	else if (axis_Y > 0)
		d_Angle = -90.f;
	else
		d_Angle = 0;

	return d_Angle;
}

//////////////////////////////////////////////////////////////////////////////
// Gets dude's MOVEMENT velocity based on joystick input.
//////////////////////////////////////////////////////////////////////////////
#if defined(ALLOW_TWINSTICK)
extern void GetDudeVelocity(double* d_Velocity, double* d_Angle)
{
	SDL_GameController* controller = ms_Controllers[0];

	Sint16 axis_MoveUpDown = ms_ajsCurr[0].axis_MoveUpDown;
	Sint16 axis_MoveLeftRight = ms_ajsCurr[0].axis_MoveLeftRight;

	*d_Velocity = sqrt(pow(axis_MoveUpDown, 2) + pow(axis_MoveLeftRight, 2)) / 32768.f;
	if (axis_MoveLeftRight != 0)
		*d_Angle = GetStickAngle(axis_MoveLeftRight, axis_MoveUpDown);
	else
		*d_Angle = 0.f; //!! FIXME

	//TRACE("DudeVel X%i Y%i Vel %f Angle %f\n", (int)axis_MoveLeftRight, (int)axis_MoveUpDown, *d_Velocity, *d_Angle);
}

//////////////////////////////////////////////////////////////////////////////
// Gets dude's firing angle. Returns TRUE if dude is shooting, FALSE if not
//////////////////////////////////////////////////////////////////////////////
extern bool GetDudeFireAngle(double* d_Angle)
{
	SDL_GameController* controller = ms_Controllers[0];

	Sint16 axis_FireUpDown = ms_ajsCurr[0].axis_FireUpDown;
	Sint16 axis_FireLeftRight = ms_ajsCurr[0].axis_FireLeftRight;

	if (axis_FireLeftRight != 0)
		*d_Angle = GetStickAngle(axis_FireLeftRight, axis_FireUpDown);
	else if (axis_FireUpDown != 0)
		*d_Angle = 0.f; //!! FIXME
	else
	{
		//TRACE("DudeFire FALSE X%i Y%i", (int)axis_FireLeftRight, (int)axis_FireUpDown);
		return false;
	}

	//TRACE("DudeFire TRUE X%i Y%i Angle %f\n", (int)axis_FireLeftRight, (int)axis_FireUpDown, *d_Angle);
	return true;
}
#endif // ALLOW_TWINSTICK

//////////////////////////////////////////////////////////////////////////////
//
// Updates joystick sJoy's current state and makes the current state the 
// previous.
//
//////////////////////////////////////////////////////////////////////////////
extern void rspUpdateJoy(short sJoy)
	{
        if ((sJoy > NUM_JOYSTICKS) || (ms_Controllers[sJoy] == NULL))
            return;

        SDL_GameController *controller = ms_Controllers[sJoy];
        SDL_GameControllerUpdate();

        if (!SDL_GameControllerGetAttached(controller))
        {
            // uhoh, controller was unplugged.
            SDL_GameControllerClose(controller);
            ms_Controllers[sJoy] = NULL;
            SDL_memset(&ms_ajsCurr[sJoy], '\0', sizeof (JoyState));
            SDL_memset(&ms_ajsPrev[sJoy], '\0', sizeof (JoyState));
        }

        SDL_memcpy(&ms_ajsPrev[sJoy], &ms_ajsCurr[sJoy], sizeof (JoyState));

        ms_ajsCurr[sJoy].u32Axes = 0;

		// fuck it just stub out the old shit entirely
#if defined(ALLOW_TWINSTICK)
		GetAxesNew(controller, &ms_ajsCurr[sJoy].axis_MoveUpDown, &ms_ajsCurr[sJoy].axis_MoveLeftRight, &ms_ajsCurr[sJoy].axis_FireUpDown, &ms_ajsCurr[sJoy].axis_FireLeftRight);
//#else
#endif
        calcAxis(controller, SDL_CONTROLLER_AXIS_LEFTX, &ms_ajsCurr[sJoy].u32Axes, RSP_JOY_X_POS, RSP_JOY_X_NEG);
        calcAxis(controller, SDL_CONTROLLER_AXIS_LEFTY, &ms_ajsCurr[sJoy].u32Axes, RSP_JOY_Y_POS, RSP_JOY_Y_NEG);
        calcAxis(controller, SDL_CONTROLLER_AXIS_RIGHTX, &ms_ajsCurr[sJoy].u32Axes, RSP_JOY_Z_POS, RSP_JOY_Z_NEG);
        calcAxis(controller, SDL_CONTROLLER_AXIS_RIGHTY, &ms_ajsCurr[sJoy].u32Axes, RSP_JOY_W_POS, RSP_JOY_W_NEG);
        //calcAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT, &ms_ajsCurr[sJoy].u32Axes, RSP_JOY_U_POS, RSP_JOY_U_NEG);
        //calcAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT, &ms_ajsCurr[sJoy].u32Axes, RSP_JOY_V_POS, RSP_JOY_V_NEG);
//#endif

        ms_ajsCurr[sJoy].u32Buttons = 0;
        calcButton(controller, SDL_CONTROLLER_BUTTON_A, &ms_ajsCurr[sJoy].u32Buttons, RSP_JOY_BUT_1);
        calcButton(controller, SDL_CONTROLLER_BUTTON_B, &ms_ajsCurr[sJoy].u32Buttons, RSP_JOY_BUT_2);
        calcButton(controller, SDL_CONTROLLER_BUTTON_X, &ms_ajsCurr[sJoy].u32Buttons, RSP_JOY_BUT_3);
        calcButton(controller, SDL_CONTROLLER_BUTTON_Y, &ms_ajsCurr[sJoy].u32Buttons, RSP_JOY_BUT_4);
        calcButton(controller, SDL_CONTROLLER_BUTTON_BACK, &ms_ajsCurr[sJoy].u32Buttons, RSP_JOY_BUT_5);
        calcButton(controller, SDL_CONTROLLER_BUTTON_GUIDE, &ms_ajsCurr[sJoy].u32Buttons, RSP_JOY_BUT_6);
        calcButton(controller, SDL_CONTROLLER_BUTTON_START, &ms_ajsCurr[sJoy].u32Buttons, RSP_JOY_BUT_7);
        calcButton(controller, SDL_CONTROLLER_BUTTON_LEFTSTICK, &ms_ajsCurr[sJoy].u32Buttons, RSP_JOY_BUT_8);
        calcButton(controller, SDL_CONTROLLER_BUTTON_RIGHTSTICK, &ms_ajsCurr[sJoy].u32Buttons, RSP_JOY_BUT_9);
        calcButton(controller, SDL_CONTROLLER_BUTTON_LEFTSHOULDER, &ms_ajsCurr[sJoy].u32Buttons, RSP_JOY_BUT_10);
        calcButton(controller, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, &ms_ajsCurr[sJoy].u32Buttons, RSP_JOY_BUT_11);
        calcButton(controller, SDL_CONTROLLER_BUTTON_DPAD_UP, &ms_ajsCurr[sJoy].u32Buttons, RSP_JOY_BUT_12);
        calcButton(controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN, &ms_ajsCurr[sJoy].u32Buttons, RSP_JOY_BUT_13);
        calcButton(controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT, &ms_ajsCurr[sJoy].u32Buttons, RSP_JOY_BUT_14);
        calcButton(controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT, &ms_ajsCurr[sJoy].u32Buttons, RSP_JOY_BUT_15);

		//!! HACK
		Sint16 TriggerLeft = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT);
		Sint16 TriggerRight = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT);
		if (TriggerLeft > 16384)
			ms_ajsCurr[sJoy].u32Buttons |= RSP_JOY_BUT_16;
		if (TriggerRight > 16384)
			ms_ajsCurr[sJoy].u32Buttons |= RSP_JOY_BUT_17;

	}

//////////////////////////////////////////////////////////////////////////////
//
// Reads the joystick sJoy's current state.
// This function returns directions in a digital format (up, down, centered).
//
//////////////////////////////////////////////////////////////////////////////
extern void rspGetJoyState(
	short sJoy,						// In:  Joystick to query.
	U32*	pu32Buttons,			// Out: Buttons that are down, if not NULL.
										// An RSP_JOY_BUT_## bit field that is set indicates
										// that button is down.
	U32*	pu32Axes /*= NULL*/)	// Out: Directions that are specificed, if not NULL.
										// An RSP_JOY_?_POS bit set indicates the ? axis is positive.
										// An RSP_JOY_?_NEG bit set indicates the ? axis is negative.
										// If neither is set for ? axis, that axis is 0.
	{
	SET(pu32Buttons,	ms_ajsCurr[sJoy].u32Buttons);
	SET(pu32Axes,		ms_ajsCurr[sJoy].u32Axes);
	}

//////////////////////////////////////////////////////////////////////////////
//
// Reads the joystick sJoy's previous state.
// This function returns directions in a digital format (up, down, centered).
//
//////////////////////////////////////////////////////////////////////////////
extern void rspGetJoyPrevState(
	short sJoy,						// In:  Joystick to query.
	U32*	pu32Buttons,			// Out: Buttons that are down, if not NULL.
										// An RSP_JOY_BUT_## bit field that is set indicates
										// that button is down.
	U32*	pu32Axes /*= NULL*/)	// Out: Directions that are specificed, if not NULL.
										// An RSP_JOY_?_POS bit set indicates the ? axis is positive.
										// An RSP_JOY_?_NEG bit set indicates the ? axis is negative.
										// If neither is set for ? axis, that axis is 0.
	{
	SET(pu32Buttons,	ms_ajsPrev[sJoy].u32Buttons);
	SET(pu32Axes,		ms_ajsPrev[sJoy].u32Axes);
	}

//////////////////////////////////////////////////////////////////////////////
// Functions to convert bitfields to joybutton numbers and back again.
//////////////////////////////////////////////////////////////////////////////
extern short MouseBitfieldToIndex(U32 bitfield)
{
	// Also clunky
	short result;
	if (bitfield & 0x0001)
		result = 1;
	else if (bitfield & 0x0002)
		result = 2;
	else if (bitfield & 0x0004)
		result = 3;
	else if (bitfield & 0x0008)
		result = 4;
	else if (bitfield & 0x0010)
		result = 5;
	else if (bitfield & 0x0020)
		result = 6;
	else if (bitfield & 0x0040)
		result = 7;
	else
		result = 0;

	if (bitfield != 0 || result != 0)
		TRACE("MouseBitfieldToIndex(%i) = %i\n", bitfield, result);

	return result;
}
extern short JoyBitfieldToIndex(U32 bitfield)
{
	// Clunky.
	short result;
	if (bitfield & RSP_JOY_BUT_1)
		result = 1;
	else if (bitfield & RSP_JOY_BUT_2)
		result = 2;
	else if (bitfield & RSP_JOY_BUT_3)
		result = 3;
	else if (bitfield & RSP_JOY_BUT_4)
		result = 4;
	else if (bitfield & RSP_JOY_BUT_5)
		result = 5;
	else if (bitfield & RSP_JOY_BUT_6)
		result = 6;
	else if (bitfield & RSP_JOY_BUT_7)
		result = 7;
	else if (bitfield & RSP_JOY_BUT_8)
		result = 8;
	else if (bitfield & RSP_JOY_BUT_9)
		result = 9;
	else if (bitfield & RSP_JOY_BUT_10)
		result = 10;
	else if (bitfield & RSP_JOY_BUT_11)
		result = 11;
	else if (bitfield & RSP_JOY_BUT_12)
		result = 12;
	else if (bitfield & RSP_JOY_BUT_13)
		result = 13;
	else if (bitfield & RSP_JOY_BUT_14)
		result = 14;
	else if (bitfield & RSP_JOY_BUT_15)
		result = 15;
	else if (bitfield & RSP_JOY_BUT_16)
		result = 16;
	else if (bitfield & RSP_JOY_BUT_17)
		result = 17;
	else if (bitfield & RSP_JOY_BUT_18)
		result = 18;
	else
		// Default
		result = 0;

	if (bitfield != 0 || result != 0)
		TRACE("JoyBitfieldToIndex(%i) = %i\n", bitfield, result);

	return result;
}

extern U32 MouseIndexToBitfield(short index)
{
	// Still clunky
	U32 result;
	if (index == 1)
		result = 0x1;
	else if (index == 2)
		result = 0x2;
	else if (index == 3)
		result = 0x4;
	else if (index == 4)
		result = 0x8;
	else if (index == 5)
		result = 0x10;
	else if (index == 6)
		result = 0x20;
	else if (index == 7)
		result = 0x40;
	else
		result = 0;

	if (index != 0 || result != 0)
		TRACE("MouseIndexToBitfield(%i) = %i\n", index, result);

	return result;
}

extern U32 JoyIndexToBitfield(short index)
{
	// Also clunky.
	U32 result;
	switch (index)
	{
		case 1:
			result = RSP_JOY_BUT_1;
			break;
		case 2:
			result = RSP_JOY_BUT_2;
			break;
		case 3:
			result = RSP_JOY_BUT_3;
			break;
		case 4:
			result = RSP_JOY_BUT_4;
			break;
		case 5:
			result = RSP_JOY_BUT_5;
			break;
		case 6:
			result = RSP_JOY_BUT_6;
			break;
		case 7:
			result = RSP_JOY_BUT_7;
			break;
		case 8:
			result = RSP_JOY_BUT_8;
			break;
		case 9:
			result = RSP_JOY_BUT_9;
			break;
		case 10:
			result = RSP_JOY_BUT_10;
			break;
		case 11:
			result = RSP_JOY_BUT_11;
			break;
		case 12:
			result = RSP_JOY_BUT_12;
			break;
		case 13:
			result = RSP_JOY_BUT_13;
			break;
		case 14:
			result = RSP_JOY_BUT_14;
			break;
		case 15:
			result = RSP_JOY_BUT_15;
			break;
		case 16:
			result = RSP_JOY_BUT_16;
			break;
		case 17:
			result = RSP_JOY_BUT_17;
			break;
		case 18:
			result = RSP_JOY_BUT_18;
			break;
		default:
			result = 0;
			break;
	}

	if (index != 0 || result != 0)
		printf("JoyIndexToBitfield(%i) = %i\n", index, result);

	return result;
}

//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////

