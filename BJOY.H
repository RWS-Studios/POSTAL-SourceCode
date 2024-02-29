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
// This is a public header file.  It is to be included by applications that use
// the WinBlue Library.
///////////////////////////////////////////////////////////////////////////////
#ifndef B_JOY_H
#define B_JOY_H

///////////////////////////////////////////////////////////////////////////////
// Typedefs.
///////////////////////////////////////////////////////////////////////////////
// Joystick easy access.
typedef union
	{
	struct
		{
		unsigned button1	: 1;
		unsigned button2	: 1;
		unsigned button3	: 1;
		unsigned button4	: 1;

		unsigned up			: 1;
		unsigned down		: 1;
		unsigned left		: 1;
		unsigned right		: 1;

		unsigned toward	: 1;
		unsigned away		: 1;
		unsigned unused1	: 1;	// Do not use or even reference the unused fields
		unsigned unused2	: 1;	// as they may someday be used and changed in name.

		unsigned unused3	: 1;	// To keep your code compiling when we change these
		unsigned unused4	: 1;	// unused fields to useful things do not use or 
		unsigned unused5	: 1;	// even reference these.  Thanks.
		unsigned unused6	: 1;
		} ;
	USHORT us;
	} JOYSTATE, *PJOYSTATE;

///////////////////////////////////////////////////////////////////////////////
// Macros.
///////////////////////////////////////////////////////////////////////////////

// Joy button types.
// Use 0 to specify first joystick and 1 to specify second.

// Joy button states.
#define JOY_BUT_1			0x0001
#define JOY_BUT_2			0x0002
#define JOY_BUT_3			0x0004
#define JOY_BUT_4			0x0008

// Joy buttons' mask.
#define JOY_BUT			0x000F

// Joy dir states.
#define JOY_DIR_UP		0x0010
#define JOY_DIR_DOWN		0x0020
#define JOY_DIR_LEFT		0x0040
#define JOY_DIR_RIGHT	0x0080
#define JOY_DIR_TOWARD	0x0100	// Toward player.
#define JOY_DIR_AWAY		0x0200	// Away from player.

// Joy dir states.
#define JOY_DIR_STATES	0x03F0


///////////////////////////////////////////////////////////////////////////////
// Prototypes.
///////////////////////////////////////////////////////////////////////////////

// Updates joystick sJoy's current state and makes the current state the 
// previous.
// Returns 0 on success.
extern short Blu_UpdateJoy(short sJoy);

// Puts the coordinates of joystick sJoy's position in your longs.
// Returns nothing.
extern void Blu_GetJoyPos(short sJoy, long *px, long *py, long *pz);

// Puts the coordinates of the previous joystick sJoy's position in your longs.
// Returns nothing.
extern void Blu_GetJoyPrevPos(short sJoy, long *px, long *py, long *pz);

// Returns the current joystick sJoy's state.
extern USHORT Blu_GetJoyState(short sJoy);

// Places the current joystick sJoy's state.
extern void Blu_GetJoyState(short sJoy, PJOYSTATE pjs);

// Returns the previous joystick sJoy's state.
extern USHORT Blu_GetJoyPrevState(short sJoy);

// Places the previous joystick sJoy's state.
extern void Blu_GetJoyPrevState(short sJoy, PJOYSTATE pjs);


#endif // B_JOY_H
//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
