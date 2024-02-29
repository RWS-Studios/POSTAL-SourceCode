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
// Organ.h
// 
// History:
//		08/17/97 JMI	Started this header.
//
//////////////////////////////////////////////////////////////////////////////
//
// Allows a player to play with his organ
//
//////////////////////////////////////////////////////////////////////////////

// Relying on paths working on Mac.
#include "WishPiX/Menu/menu.h"

// Choice callback from menu.
extern bool Organ_MenuChoice(	// Returns true to accept choice, false to deny.
	Menu*	pmenuCurrent,			// Current menu.
	short	sMenuItem);				// Item chosen.

//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
