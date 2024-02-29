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
// MemFileFest.cpp
// Project: Nostril (aka Postal)
// 
// History:
//		09/12/97 JMI	Started.
//
//		09/12/97	JMI	Added SHAREWARE_RELEASE files.
//
//		09/12/97	JMI	Now only the end of the requested resource name has to
//							match the full embedded file's name.
//
//		09/17/97 MJR	Renamed to the more-correct ONLINE_DEMO_RELEASE.
//
//		06/24/01 MJR	Got rid of alternate CompUSA level.
//
//////////////////////////////////////////////////////////////////////////////
//
//	Manages a group of memory resources that represent disk files.  Currently
// used for .RLM files to limit the usefulness of crippleware demos.
//
//////////////////////////////////////////////////////////////////////////////

#include "RSPiX.h"

#include "CompileOptions.h"

#include "MemFileFest.h"

//////////////////////////////////////////////////////////////////////////////
// Macros.
//////////////////////////////////////////////////////////////////////////////

#define NUM_ELEMENTS(a)	(sizeof(a) / sizeof(a[0]) )

//////////////////////////////////////////////////////////////////////////////
// Typedefs.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Variables.
//////////////////////////////////////////////////////////////////////////////

// Only compile this hefty chunk of shit if we're only allowing specific realm
#if defined(ENABLE_PLAY_SPECIFIC_REALMS_ONLY)
	#include "GConsite.h"
	#include "EConsite.h"
	#include "MConsite.h"
	#include "HConsite.h"
	#include "PConsite.h"

	FATEntry	ms_memdisk[]	= 
		{
			{	pszCheckPtGConsite,	au8CheckPtGConsite,	sizeof(au8CheckPtGConsite),	},

			{	pszEConsite,			au8EConsite,			sizeof(au8EConsite),				},

			{	pszMConsite,			au8MConsite,			sizeof(au8MConsite),				},

			{	pszHConsite,			au8HConsite,			sizeof(au8HConsite),				},

			{	pszPConsite,			au8PConsite,			sizeof(au8PConsite),				},
		};

//////////////////////////////////////////////////////////////////////////////
// Functions.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Given a filename, open an RFile to the corresponding resource data.
//////////////////////////////////////////////////////////////////////////////
extern short GetMemFileResource(	// Returns 0 on successful open.
	const char*		pszResName,		// In:  Res filename.
	RFile::Endian	endian,			// In:  Endian nature for RFile.
	RFile*			pfile)			// In:  File to open with.
	{
	short	sRes	= 1;	// Assume failure for simplicity.
	ASSERT(pfile);
	ASSERT(pszResName);

	short	sIndex;
	bool	bFound;
	long	lReqResNameLen	= strlen(pszResName);
	for (sIndex = 0, bFound = false; sIndex < NUM_ELEMENTS(ms_memdisk) && bFound == false; sIndex++)
		{
		long lEmbeddedResNameLen	= strlen(ms_memdisk[sIndex].pszResName);
		// If the requested name is long enough . . .
		if (lReqResNameLen >= lEmbeddedResNameLen)
			{
			// If this is the specified res name (the specified name only has to
			// match from the end back lEmbeddedResNameLen characters) . . .
			if (rspStricmp(pszResName + lReqResNameLen - lEmbeddedResNameLen, ms_memdisk[sIndex].pszResName) == 0)
				{
				// Found it.
				bFound	= true;

				// Open the resource data . . .
				sRes	= pfile->Open(ms_memdisk[sIndex].pau8Res, ms_memdisk[sIndex].lResSize, endian);
				}
			}
		}
	
	return sRes;
	}

#endif	// ENABLE_PLAY_SPECIFIC_REALMS_ONLY

//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
