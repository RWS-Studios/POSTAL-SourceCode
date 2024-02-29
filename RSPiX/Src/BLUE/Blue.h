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
//	Blue.h
// 
// History:
//		07/19/96 JMI	Started.
//
//		06/03/97	JMI	Added error messages.
//
//////////////////////////////////////////////////////////////////////////////
//
// This file decides what platform you are on and #includes the correct
// *Blue.h.  If you #include the platform specific *Blue.h instead of this
// one, you will get a compile error.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BLUE_H
#define BLUE_H

#include "unix/UnixBlue.h"

//////////////////////////////////////////////////////////////////////////////
// Error messages.
//////////////////////////////////////////////////////////////////////////////

// Base for all Blue error returns.
#define BLU_ERR_BASE					0x1000

// Blue error returns ////////////////////////////////////////////////////////

// Audio /////////////////////////////////////////////////////////////////////

#define BLU_ERR_DEVICE_IN_USE	(BLU_ERR_BASE + 1)	// Device already in use.
#define BLU_ERR_NO_DEVICE		(BLU_ERR_BASE + 2)	// Device (or driver for
																	// device) not present.
#define BLU_ERR_NOT_SUPPORTED	(BLU_ERR_BASE + 3)	// Format not supported.

#endif // BLUE_H
//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
