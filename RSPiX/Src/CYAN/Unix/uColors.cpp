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
////////////////////////////////////////////////////////////////////////////////
//
// mColors.cpp
// Color Palette API for Unix Implementation of RSPiX CYAN.
//
//
// History:
//		06/04/04 RCG	Started.
//
////////////////////////////////////////////////////////////////////////////////

#include "SDL.h"
#include "Blue.h"
#include "../cyan.h"


////////////////////////////////////////////////////////////////////////////////
// Macros, types, enums, etc.
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Data, variables, etc.
////////////////////////////////////////////////////////////////////////////////

static unsigned char m_aucWin32R1[10] = {   0, 128,   0, 128,   0, 128,   0, 128, 192, 166 };
static unsigned char m_aucWin32G1[10] = {   0,   0, 128, 128,   0,   0, 128, 128, 220, 202 };
static unsigned char m_aucWin32B1[10] = {   0,   0,   0,   0, 128, 128, 128, 128, 192, 240 };
static unsigned char m_aucWin32R2[10] = { 255, 160, 128, 255,   0, 255,   0, 255,   0, 255 };
static unsigned char m_aucWin32G2[10] = { 251, 160, 128,   0, 255, 255,   0,   0, 255, 255 };
static unsigned char m_aucWin32B2[10] = { 240, 164, 128,   0,   0,   0, 255, 255, 255, 255 };


////////////////////////////////////////////////////////////////////////////////
// Function Prototypes
////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//
// Set the RSPiX palette to the standard Win32 "static colors".
// If requested, the static colors will be locked.
//
// Under Win32, the first 10 and last 10 palette colors are called the "static
// colors" because, historically, they were never changed.  Nowadays, they
// are often changed to create different desktop appearances or themes.  In
// any case, we often need some basic set of colors when creating cross-
// platform apps, and since Win32 is far more reliant on colors than the Mac
// (which can get by with just 2 colors) we chose the Win32 static colors as
// a common ground.  The RGB values for these colors were chosen based on the
// Win95/NT desktop appearance scheme called "Windows Standard".  These colors
// are very similar to those used by Windows 3.1 (which we support via Win32s)
// and those used by newer Mac apps that use the "3D" GUI look.
//
///////////////////////////////////////////////////////////////////////////////
void rspSetWin32StaticColors(
	short sLock /*= 0*/)										// In:  1 means lock colors, 0 means don't
	{
	// Make sure display module is alive before we call it
	if (1) //( (SDL_WasInit(SDL_INIT_VIDEO)) && (SDL_GetVideoSurface() != NULL) )
		{
		// Set the colors (note that colors 0 and 255 can't really be changed!)
		rspSetPaletteEntries(  0, 10, m_aucWin32R1, m_aucWin32G1, m_aucWin32B1, 1);
		rspSetPaletteEntries(246, 10, m_aucWin32R2, m_aucWin32G2, m_aucWin32B2, 1);

		// Check if we need to lock the colors
		if (sLock)
			{
			rspLockPaletteEntries(  0, 10);
			rspLockPaletteEntries(246, 10);
			}
		}
	else
		{
		TRACE("rspSetWin32StaticColors(): rspInitBlue() must be called before using this function!\n");
		}
	}


////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
