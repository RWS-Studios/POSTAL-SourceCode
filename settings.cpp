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
// settings.cpp
// Project: Nostril (aka Postal)
//
// This module impliments the CSettings class, which provides a standardized
// method of working with all the various game settings.
//
// History:
//		11/26/96 MJR	Started.
//		12/04/96 MJR	Implimented all the static stuff.
//		04/16/97 MJR	Added test for read-only file when writing prefs, and
//							returned special error code to indicate such a thing.
//
//		05/08/97	JMI	Added conditions for compiler versions' STL
//							differences.
//
//		06/09/97 MJR	Fixed memory leak -- forgot to free the memory that was
//							allocated by PreDemo().
//
//		06/29/97 MJR	Replaced STL vector with an RSP list.  STL is an evil
//							entity that should be banished from the face of the earth.
//							Whoever suggested we use it should be shot.  (Good thing
//							I'm the president -- it's against the rules to shoot me.)
//
//		07/10/97 MJR	Removed use of non-ANSI "t" in mode string for file open.
//
////////////////////////////////////////////////////////////////////////////////
#define SETTINGS_CPP

#include "RSPiX.h"
#ifdef PATHS_IN_INCLUDES
	#include "WishPiX/Prefs/prefs.h"
#else
	#include "prefs.h"
#endif

#include "main.h"
#include "settings.h"


////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Variables/data
////////////////////////////////////////////////////////////////////////////////

// Pointer to container for all CSettings objects.  This is a pointer to the
// container for reasons explained in the constructor.  I'm relying on what I
// understand to be the defined C++ startup sequence: (1) initialize all static
// data without explicit initializers to 0, then (2) initialize global static
// objects in a translation unit (which gets into the whole order problem).
// I take that to mean that this will be set to 0 before any code is executed.
CSettings::SETTINGS* CSettings::ms_pSettings;

// Pointer to memory used for memory file.  While it isn't as critical as
// the above pointer, this one isn't explicitly set to 0 for the same reasons.
void* CSettings::ms_pMem;


////////////////////////////////////////////////////////////////////////////////
// Function prototypes
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//
// Default (and only) constructor
//
////////////////////////////////////////////////////////////////////////////////
CSettings::CSettings(void)
	{
	// If the container itself doesn't exist yet, create it now.  This sucks
	// because we can't inform the caller if an error occurs.  However, there
	// wasn't much choice, as the alternate (having this be the actual object
	// rather than a pointer to it) led to the typical C++ problem of not knowing
	// which would be initialized first -- the container or an object that wants
	// to use it.  This way, the first object that tries to use the container
	// will create the container, so that problem goes away.
	if (ms_pSettings == 0)
		ms_pSettings = new SETTINGS;
	if (ms_pSettings != 0)
		{

		// Add this object to container
		m_pointer = ms_pSettings->InsertTail(this);

		}
	else
		TRACE("CSettings::CSettings(): Couldn't create new container!\n");
	}


////////////////////////////////////////////////////////////////////////////////
//
// Destructor
//
////////////////////////////////////////////////////////////////////////////////
CSettings::~CSettings()
	{
	// Make sure container exists
	if (ms_pSettings != 0)
		{

		// Remove this object from container
		ms_pSettings->Remove(m_pointer);

		// If there are no more objects, then delete the container itself
		if (ms_pSettings->GetHead() == 0)
			{
			delete ms_pSettings;
			ms_pSettings = 0;
			}

		// Make sure to delete memory if it wasn't already deleted
		if (ms_pMem)
			{
			free(ms_pMem);
			ms_pMem = 0;
			}
		}
	}


////////////////////////////////////////////////////////////////////////////////
//
// Load all settings from preference file
//
////////////////////////////////////////////////////////////////////////////////
short CSettings::LoadPrefs(					// Returns 0 if successfull, non-zero otherwise
	char* pszFile)									// In:  Name of prefs file
	{
	short sResult = 0;

	// Make sure container exists
	if (ms_pSettings != 0)
		{

		// Open file for read access (text mode is default)
		RPrefs prefs;
		sResult = prefs.Open(pszFile, "r");
		if (sResult == 0)
			{

			// Do this for all CSettings objects
			for (SETTINGS::Pointer i = ms_pSettings->GetHead(); i != 0; i = ms_pSettings->GetNext(i))
				{
				sResult = ms_pSettings->GetData(i)->LoadPrefs(&prefs);
				if (sResult)
					break;
				}

			// If no errors detected, double-check to be sure no I/O errors occurred
			if (!sResult && prefs.IsError())
				{
				sResult = -1;
				TRACE("CSettings::LoadPrefs(): Error reading prefs file!\n");
				}

			prefs.Close();
			}
		else
			{
			sResult = -1;
			TRACE("CSettings::LoadPrefs(): Couldn't open prefs file: %s !\n", pszFile);
			}
		}
	else
		{
		sResult = -1;
		TRACE("CSettings::LoadPrefs(): No container!\n");
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
//
// Save all settings to preference file
//
// Return values:
//		0 = successfull
//		1 = read-only file (couldn't save)
//		<0 = some other error
//
////////////////////////////////////////////////////////////////////////////////
short CSettings::SavePrefs(					// Returns 0 if successfull, non-zero otherwise
	char* pszFile)									// In:  Name of prefs file
	{
	short sResult = 0;

	// Make sure container exists
	if (ms_pSettings != 0)
		{

		// First open file for read-only access.  If this works, then at least
		// we know the file exists, although it might be a read-only file.
		RPrefs prefs;
		sResult = prefs.Open(pszFile, "r");
		if (sResult == 0)
			{
			prefs.Close();

			// Open file again, this time for read+ (read plus write/append) access.
			// If this doesn't work, the file is most likely a read-only file.
			sResult = prefs.Open(pszFile, "r+");
			if (sResult == 0)
				{

				// Do this for all CSettings objects
				for (SETTINGS::Pointer i = ms_pSettings->GetHead(); i != 0; i = ms_pSettings->GetNext(i))
					{
					sResult = ms_pSettings->GetData(i)->SavePrefs(&prefs);
					if (sResult)
						break;
					}

				// If no errors detected, double-check to be sure no I/O errors occurred
				if (!sResult && prefs.IsError())
					{
					sResult = -1;
					TRACE("CSettings::SavePrefs(): Error writing prefs file!\n");
					}

				prefs.Close();
				}
			else
				{
				sResult = 1; // Special Read-Only error return!
				TRACE("CSettings::SavePrefs: Read-only prefs file: %s !\n", pszFile); 
				}
			}
		else
			{
			sResult = -1;
			TRACE("CSettings::SavePrefs(): Couldn't open prefs file: %s !\n", pszFile);
			}
		}
	else
		{
		sResult = -1;
		TRACE("CSettings::SavePrefs(): No container!\n");
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
//
// Load all settings from game file
//
////////////////////////////////////////////////////////////////////////////////
short CSettings::LoadGame(						// Returns 0 if successfull, non-zero otherwise
	char* pszFile)									// In:  Name of prefs file
	{
	short sResult = 0;

	// Make sure container exists
	if (ms_pSettings != 0)
		{

		// Open file for read access in binary mode
		RFile fileGame;
		sResult = fileGame.Open(pszFile, "rb", RFile::LittleEndian);
		if (sResult == 0)
			{

			// Do this for all CSettings objects
			for (SETTINGS::Pointer i = ms_pSettings->GetHead(); i != 0; i = ms_pSettings->GetNext(i))
				{
				sResult = ms_pSettings->GetData(i)->LoadGame(&fileGame);
				if (sResult)
					break;
				}

			// If no errors detected, double-check to be sure no I/O errors occurred
			if (!sResult && fileGame.Error())
				{
				sResult = -1;
				TRACE("CSettings::LoadGame(): Error reading game file!\n");
				}

			fileGame.Close();
			}
		else
			{
			sResult = -1;
			TRACE("CSettings::LoadGame(): Couldn't open game file: %s !\n", pszFile);
			}
		}
	else
		{
		sResult = -1;
		TRACE("CSettings::LoadGame(): No container!\n");
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
//
// Save all settings to game file
//
////////////////////////////////////////////////////////////////////////////////
short CSettings::SaveGame(						// Returns 0 if successfull, non-zero otherwise
	char* pszFile)									// In:  Name of prefs file
	{
	short sResult = 0;

	// Make sure container exists
	if (ms_pSettings != 0)
		{

		// Open file for write access in binary mode
		// Note: We don't care if the file already exists -- write mode will destroy it
		RFile fileGame;
		sResult = fileGame.Open(pszFile, "wb", RFile::LittleEndian);
		if (sResult == 0)
			{

			// Do this for all CSettings objects
			for (SETTINGS::Pointer i = ms_pSettings->GetHead(); i != 0; i = ms_pSettings->GetNext(i))
				{
				sResult = ms_pSettings->GetData(i)->SaveGame(&fileGame);
				if (sResult)
					break;
				}

			// If no errors detected, double-check to be sure no I/O errors occurred
			if (!sResult && fileGame.Error())
				{
				sResult = -1;
				TRACE("CSettings::SaveGame(): Error writing game file!\n");
				}

			fileGame.Close();
			}
		else
			{
			sResult = -1;
			TRACE("CSettings::SaveGame(): Couldn't open game file: %s !\n", pszFile);
			}
		}
	else
		{
		sResult = -1;
		TRACE("CSettings::SaveGame(): No container!\n");
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
//
// Temporarily set settings for demo mode
//
////////////////////////////////////////////////////////////////////////////////
short CSettings::PreDemo(						// Returns 0 if successfull, non-zero otherwise
	void)
	{
	short sResult = 0;

	// Make sure container exists
	if (ms_pSettings != 0)
		{

		// Allocate a chunk of memory for settings to be saved to
		ms_pMem = malloc(CSettings::MemFileSize);
		if (ms_pMem != 0)
			{

			// Open memory file
			RFile fileMem;
			sResult = fileMem.Open(ms_pMem, CSettings::MemFileSize, RFile::LittleEndian);
			if (sResult == 0)
				{

				// Do this for all CSettings objects
				for (SETTINGS::Pointer i = ms_pSettings->GetHead(); i != 0; i = ms_pSettings->GetNext(i))
					{
					sResult = ms_pSettings->GetData(i)->PreDemo(&fileMem);
					if (sResult)
						break;
					}

				// If no errors detected, double-check to be sure no I/O errors occurred
				if (!sResult && fileMem.Error())
					{
					sResult = -1;
					TRACE("CSettings::PreDemo(): Error writing to mem file (probably too much data)!\n");
					}

				fileMem.Close();
				}
			else
				{
				sResult = -1;
				TRACE("CSettings::PreDemo(): Couldn't open mem file!\n");
				}
			}
		else
			{
			sResult = -1;
			TRACE("CSettings::PreDemo(): Couldn't allocate memory for mem file!\n");
			}
		}
	else
		{
		sResult = -1;
		TRACE("CSettings::PreDemo(): No container!\n");
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
//
// Restore settings to what they were prior to demo mode
//
////////////////////////////////////////////////////////////////////////////////
short CSettings::PostDemo(						// Returns 0 if successfull, non-zero otherwise
	void)
	{
	short sResult = 0;

	// Make sure container exists
	if (ms_pSettings != 0)
		{

		// Make sure memory was allocated by PreDemo()
		if (ms_pMem != 0)
			{

			// Open previously allocated memory file
			RFile fileMem;
			sResult = fileMem.Open(ms_pMem, CSettings::MemFileSize, RFile::LittleEndian);
			if (sResult == 0)
				{

				// Do this for all CSettings objects
				for (SETTINGS::Pointer i = ms_pSettings->GetHead(); i != 0; i = ms_pSettings->GetNext(i))
					{
					sResult = ms_pSettings->GetData(i)->PostDemo(&fileMem);
					if (sResult)
						break;
					}

				// If no errors detected, double-check to be sure no I/O errors occurred
				if (!sResult && fileMem.Error())
					{
					sResult = -1;
					TRACE("CSettings::PostDemo(): Error reading from mem file!\n");
					}

				fileMem.Close();
				}
			else
				{
				sResult = -1;
				TRACE("CSettings::PostDemo(): Couldn't open mem file!\n");
				}

			// Free memory
			free(ms_pMem);
			ms_pMem = 0;
			}
		else
			{
			sResult = -1;
			TRACE("CSettings::PostDemo(): No memory file to read from! (did you forget to call CSettings::PreDemo?)\n");
			}
		}
	else
		{
		sResult = -1;
		TRACE("CSettings::PostDemo(): No container!\n");
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
