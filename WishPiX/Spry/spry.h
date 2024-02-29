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
// spry.h
//
////////////////////////////////////////////////////////////////////////////////
#ifndef SPRY_H
#define SPRY_H

#include "System.h"
#ifdef PATHS_IN_INCLUDES
	#include "ORANGE/CDT/flist.h"
	#include "ORANGE/GameLib/SPRITE.H"
#else
	#include "flist.h"
	#include "sprite.h"
#endif


class RSpry
	{
	//---------------------------------------------------------------------------
	// Types, enums, etc.
	//---------------------------------------------------------------------------

    // This is accessed externally in hood.cpp.  --ryan.
	//private:
	public:
		// Typedef for list of RSprite's
		typedef RFList<RSprite*> ListOfSprites;

		// Miscellaneous enums
		enum
			{
			FileID = 0x59525053,
			FileVersion = 1
			};

	//---------------------------------------------------------------------------
	// Member variables
	//---------------------------------------------------------------------------
	public:
		ListOfSprites m_listSprites;

	//---------------------------------------------------------------------------
	// Member functions
	//---------------------------------------------------------------------------
	public:
		// Default (and only) constructor
		RSpry();

		// Destructor
		~RSpry();

		// Clear
		short Clear(void);

		// Load from specified file
		short Load(
			char* pszFile);

		// Load from already-open file
		short Load(
			RFile* pFile);

		// Save to specified file
		short Save(
			char* pszFile);

		// Save to already-open file
		short Save(
			RFile* pFile);

		// Convert to specified RImage type
		short Convert(
			RImage::Type type);
	};


#endif //SPRY_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
