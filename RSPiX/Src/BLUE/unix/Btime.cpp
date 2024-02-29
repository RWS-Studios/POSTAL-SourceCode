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
//	btime.cpp
// 
// History:
//		06/03/04 RCG	Started.
//
//////////////////////////////////////////////////////////////////////////////
//
// Does all SDL specific time stuff.
//
//////////////////////////////////////////////////////////////////////////////

#include "Blue.h"
#include "SDL.h"

static Uint32 MicrosecondsBase = 0;

//////////////////////////////////////////////////////////////////////////////
// Functions.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Initializes the time module.
//
//////////////////////////////////////////////////////////////////////////////
extern void Time_Init(void)
	{
    MicrosecondsBase = SDL_GetTicks();
	}


//////////////////////////////////////////////////////////////////////////////
//
// Get the current Windows' time.
// Returns the time in a long.
//
//////////////////////////////////////////////////////////////////////////////
extern long rspGetMilliseconds(void)
	{
        return (long) (SDL_GetTicks());
	}

//////////////////////////////////////////////////////////////////////////////
//
// Get time since last rspGetMicroseconds(TRUE) call in microseconds.  May 
// not always be accurate to the nearest microsecond.  It is always on the
// Mac but possibly not in Windows; however, every machine tested produced
// good to excellent resolution.
// Returns the time in a long.
//
//////////////////////////////////////////////////////////////////////////////
extern long rspGetMicroseconds(	// Returns microseconds between now and
											// last
	short sReset /*= FALSE*/)		// Set to TRUE to reset timer.  If you never
											// reset the timer, it will wrap within
											// just over 35 minutes.
	{
    Uint32 microsecs = SDL_GetTicks();
    long lTime = (long) (microsecs - MicrosecondsBase);

		// If reset requested . . .
	if (sReset != FALSE)
		MicrosecondsBase = microsecs;

	return lTime * 1000;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Get time since App started in microseconds.  This is safe for global use
// because it cannot be reset by anyone.  It requires 64-bit mathm however!
//
// May not always be accurate to the nearest microsecond.  It is always on the
// Mac but possibly not in Windows; however, every machine tested produced
// good to excellent resolution.
//
// Returns the time in an __int64.
//
//////////////////////////////////////////////////////////////////////////////
extern S64 rspGetAppMicroseconds()
	{
        return ((S64) SDL_GetTicks()) * 1000;
	}

//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
