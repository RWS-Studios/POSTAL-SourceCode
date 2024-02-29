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
// RSPiX.cpp
// Project: Nostril (aka Postal)
//
// This module implements the precompiled header.
//
// History:
//		01/26/97 JMI	Started.
//
////////////////////////////////////////////////////////////////////////////////
//
// Things included in this file will be in the precompiled header.
// For a CPP to use the precompiled header it must:
//
// - All headers included here must be included in the CPP in the same 
// order they are here and must be the first includes in that CPP.
// - Check the 'Use precompiled header file' option in the Settings|C/C++|
// Precompiled Headers settings dialog with 'Through header:' set to RSPiX.h.
//
// 
// If your CPP does not like utilizing the precompiled header (i.e., it does
// not include these headers or it does not include them first):
//
// - Check the 'Not using precompiled headers' option set in the Settings|C/C++|
// Precompiled Headers settings dialog.
//
//
// Reasons to use this precompiled header:
// - On a P90/64M, using this precompiled header for Postal sped compile time
// down to 1/4 that without it.
//
// Reasons to not use this precompiled header:
// - Sometimes you may need to include another header before the ones included
// here.  For example <malloc.h>, which, b/c of SmartHeap, must be included
// before RSPiX.h.
// - If you don't need to include the headers included here, you probably don't
// need to precompile your headers.
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// RSPiX includes.
////////////////////////////////////////////////////////////////////////////////

#include "RSPiX.h"

////////////////////////////////////////////////////////////////////////////////
// Postal includes.
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
