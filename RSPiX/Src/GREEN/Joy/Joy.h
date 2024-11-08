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
// This is a public header file.  It is to be included by applications that use
// the WinBlue Library.
///////////////////////////////////////////////////////////////////////////////
#ifndef JOY_H
#define JOY_H

///////////////////////////////////////////////////////////////////////////////
// Headers.
///////////////////////////////////////////////////////////////////////////////
#include "common/bjoy.h"

///////////////////////////////////////////////////////////////////////////////
// Typedefs.
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Macros.
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Prototypes.
///////////////////////////////////////////////////////////////////////////////

// Sets the ucKey as the key representing usState on sJoy joystick.
// Returns 0 on success.
extern int16_t Joy_SetKey(int16_t sJoy, uint8_t ucKey, uint16_t usState);

// Updates joystick sJoy's current state and makes the current state the 
// previous.
// Returns 0 on success.
extern int16_t Joy_Update(int16_t sJoy);

// Puts the coordinates of joystick sJoy's position in your longs.
// Returns nothing.
extern void Joy_GetPos(int16_t sJoy, int32_t *px, int32_t *py, int32_t *pz);

// Puts the coordinates of the previous joystick sJoy's position in your longs.
// Returns nothing.
extern void Joy_GetPrevPos(int16_t sJoy, int32_t *px, int32_t *py, int32_t *pz);

// Returns the current joystick sJoy's state.
extern uint16_t Joy_GetState(int16_t sJoy);

// Places the current joystick sJoy's state.
extern void Joy_GetState(int16_t sJoy, PJOYSTATE pjs);

// Returns the previous joystick sJoy's state.
extern uint16_t Joy_GetPrevState(int16_t sJoy);

// Places the previous joystick sJoy's state.
extern void Joy_GetPrevState(int16_t sJoy, PJOYSTATE pjs);

#endif // JOY_H
//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
