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
// settings.h
// Project: Nostril (aka Postal)
//
////////////////////////////////////////////////////////////////////////////////
#ifndef SETTINGS_H
#define SETTINGS_H

#include "RSPiX.h"
#ifdef PATHS_IN_INCLUDES
	#include "ORANGE/CDT/flist.h"
#else
	#include "flist.h"
#endif

class RPrefs;
class RFile;

class CSettings
	{
	private:
		typedef RFList<CSettings*> SETTINGS;

		typedef enum
			{
			MemFileSize = 512
			};

	//---------------------------------------------------------------------------
	// This stuff deals with all CSettings objects
	//---------------------------------------------------------------------------
	private:
		static SETTINGS* ms_pSettings;	// Pointer to list of all CSettings objects
		static void* ms_pMem;				// Pointer to memory used for memory file

		SETTINGS::Pointer m_pointer;		// Pointer to this object's location in list
		
	public:
		// Read settings that are stored in preference file
		static short LoadPrefs(
			char* pszFile);

		// Write settings that are stored in preference file
		static short SavePrefs(
			char* pszFile);

		// Load settings that are stored in game file
		static short LoadGame(
			char* pszFile);

		// Save settings that are stored in game file
		static short SaveGame(
			char* pszFile);

		// Temporarily set settings for demo mode
		static short PreDemo(
			void);

		// Restore settings to what they were prior to demo mode
		static short PostDemo(
			void);

	//---------------------------------------------------------------------------
	// This stuff applies to an individual CSettings object
	//---------------------------------------------------------------------------
	public:
		// Set settings to default values
		CSettings(void);

		// Destructor
		~CSettings();

		// Read settings that are stored in preference file
		virtual short LoadPrefs(
			RPrefs* pPrefs) = 0;

		// Write settings that are stored in preference file
		virtual short SavePrefs(
			RPrefs* pPrefs) = 0;

		// Load settings that are stored in game file
		virtual short LoadGame(
			RFile* pFile) = 0;

		// Save settings that are stored in game file
		virtual short SaveGame(
			RFile* pFile) = 0;

		// Temporarily set settings for demo mode (file is for saving current settings)
		virtual short PreDemo(
			RFile* pFile) = 0;

		// Restore settings to what they were prior to demo mode
		virtual short PostDemo(
			RFile* pFile) = 0;
	};


#endif // SETTINGS_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
