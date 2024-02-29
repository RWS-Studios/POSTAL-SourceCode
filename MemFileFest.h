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
// MemFileFest.h
// Project: Nostril (aka Postal)
// 
// History:
//		09/12/97 JMI	Started.
//
//////////////////////////////////////////////////////////////////////////////
//
//	Manages a group of memory resources that represent disk files.  Currently
// used for .RLM files to limit the usefulness of crippleware demos.
//
//////////////////////////////////////////////////////////////////////////////

#include "RSPiX.h"

#include "CompileOptions.h"

//////////////////////////////////////////////////////////////////////////////
// Typedefs.
//////////////////////////////////////////////////////////////////////////////

typedef struct
	{
	char*	pszResName;	// Resource filename.
	U8*	pau8Res;		// Resource data.
	long	lResSize;	// Amount of resource data in bytes.
	} FATEntry;

//////////////////////////////////////////////////////////////////////////////
// Protos.
//////////////////////////////////////////////////////////////////////////////
#if defined(ENABLE_PLAY_SPECIFIC_REALMS_ONLY)

//////////////////////////////////////////////////////////////////////////////
// Given a filename, open an RFile to the corresponding resource data.
//////////////////////////////////////////////////////////////////////////////
extern short GetMemFileResource(	// Returns 0 on successful open.
	const char*		pszResName,		// In:  Res filename.
	RFile::Endian	endian,			// In:  Endian nature for RFile.
	RFile*			pfile);			// In:  File to open with.

#endif	// ENABLE_PLAY_SPECIFIC_REALMS_ONLY

//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
