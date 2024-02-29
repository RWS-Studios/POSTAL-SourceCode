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
// This file is automatically included as part of every file in the project.
//
// Thie file sets includes the common settings and any settings that are
// specific to NON-DEBUG mode.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef POSTALPREFIX_H
#define POSTALPREFIX_H

// We normally want to include the precompiled header file, except when this
// prefix file is being included by the file that actually generates the
// precompiled header.  Only that particular file will define this macro.
#ifndef POSTAL_PRECOMPILING_NOW
	#include "PostalPrecomp"
#endif

// Include common settings
#include "PostalPrefixCommon.h"

// SmartHeap Stuff.  Normally, we define these in debug mode and comment them out otherwise.
#define NOSHMALLOC
//#define MEM_DEBUG
//#define DEFINE_NEW_MACRO

// Define this for debug mode, comment it out otherwise.
//#define _DEBUG


#endif // POSTALPREFIX_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
