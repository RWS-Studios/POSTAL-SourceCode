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
// uPath.cpp
// Path API for Unix implimentation of RSPiX CYAN
//
//
// History:
//		06/05/04	RCG	Started.
//
////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Blue.h"
#include "UnixCyan.h"

////////////////////////////////////////////////////////////////////////////////
// Macros
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Types
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Function Prototypes
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//
// Convert a RSPiX path into a system-specific path.
// NOTE: It is safe for the source and destination to point to the same string.
// NOTE: Assumes path lengths are limited to RSP_MAX_PATH.
//
////////////////////////////////////////////////////////////////////////////////
extern char* rspPathToSystem(			// Returns pszSystem
	const char* pszRSPiX,				// In:  RSPiX path
	char* pszSystem)						// Out: System path
	{
	ASSERT(pszRSPiX != NULL);
	ASSERT(pszSystem != NULL);
	ASSERT(strlen(pszRSPiX) <= RSP_MAX_PATH);

	// Check for backslashes, which are NOT supposed to be used in RSPiX paths.
	#ifdef _DEBUG
		if (strchr(pszRSPiX, '\\') != NULL)
			TRACE("rspPathToSystem(): Warning: RSPiX path contains '\\' which is not legal: '%s'\n", pszRSPiX);
	#endif
	
    if (pszRSPiX != pszSystem)  // yes, pointer comparison.
        strcpy(pszSystem, pszRSPiX);
	return pszSystem;
	}
	
	
////////////////////////////////////////////////////////////////////////////////
//
// Convert a RSPiX path into a system-specific path.
// NOTE: This function returns a pointer to a static buffer!!!
// NOTE: Assumes path lengths are limited to RSP_MAX_PATH.
//
////////////////////////////////////////////////////////////////////////////////
extern char* rspPathToSystem(			// Returns pointer to system-specific path (static!!!)
	const char* pszRSPiX)				// In:  RSPiX path
	{
	ASSERT(pszRSPiX != NULL);
	ASSERT(strlen(pszRSPiX) <= RSP_MAX_PATH);
	
	static char acDest[RSP_MAX_PATH+1];
	return rspPathToSystem(pszRSPiX, acDest);
	}


////////////////////////////////////////////////////////////////////////////////
//
// Convert a system-specific path into a RSPiX path.
// NOTE: It is safe for the source and destination to point to the same string.
// NOTE: Assumes path lengths are limited to RSP_MAX_PATH.
//
////////////////////////////////////////////////////////////////////////////////
extern char* rspPathFromSystem(		// Returns pszRSPiX
	const char* pszSystem,				// In:  System path
	char* pszRSPiX)						// Out: RSPiX path
	{
	ASSERT(pszSystem != NULL);
	ASSERT(pszRSPiX != NULL);
	ASSERT(strlen(pszSystem) <= RSP_MAX_PATH);


    if (pszRSPiX != pszSystem)  // yes, pointer comparison.
        strcpy(pszRSPiX, pszSystem);
	return pszRSPiX;
	}


////////////////////////////////////////////////////////////////////////////////
//
// Convert a system-specific path into a RSPiX path.
// NOTE: This function returns a pointer to a static buffer!!!
// NOTE: Assumes path lengths are limited to RSP_MAX_PATH.
//
////////////////////////////////////////////////////////////////////////////////
extern char* rspPathFromSystem(		// Returns pointer to RSPiX path (static!!!)
	const char* pszSystem)				// In:  System path
	{
	ASSERT(pszSystem != NULL);
	ASSERT(strlen(pszSystem) <= RSP_MAX_PATH);
	
	static char acDest[RSP_MAX_PATH+1];
	return rspPathFromSystem(pszSystem, acDest);
	}


////////////////////////////////////////////////////////////////////////////////
//
// Gets a path to a directory used for temporary files.  This is easiest way
// to get a path to a directory with write permissions and, hopefully,
// reasonably fast disk access.  The returned path is in system-specific format!
//
// Mac Implimentation:
//
// Find the preference folder on the user's system volume.  Returns the
// full path of the folder, such as "Macintosh HD:System Folder:Preferences:".
//
////////////////////////////////////////////////////////////////////////////////
extern short rspGetTempPath(			// Returns 0 on success, non-zero otherwise
	char* pszPath,							// Out: Temp path returned here if available.
	short	sMaxPathLen)					// In:  Max path length (to avoid overwrites)
	{
	ASSERT(pszPath != NULL);
	ASSERT(sMaxPathLen > 0);

    strncpy(pszPath, "/tmp", sMaxPathLen);
    pszPath[sMaxPathLen-1] = 0;
	return 0;
	}


////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
