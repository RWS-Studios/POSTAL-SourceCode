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
//////////////////////////////////////////////////////////////////////////////
//
// PalFile.CPP
// 
// History:
//		12/11/96 JMI	Started.
//
//		02/04/97	JMI	Changed #include "ImageLoad.h" to "PalFile.h".
//
//////////////////////////////////////////////////////////////////////////////
//
// Handles file formats for RPal.  Some of these older formats will not be 
// useful to developers of new code.  To include the ability to use a certain
// loader, define one of the macros PAL_LOAD_# where # is the number of the
// format desired OR define the macro PAL_LOAD_ALL for all available
// support.
//
// NOTES TO THOSE ADDING NEW FILE VERSIONS:
//	Please follow these steps when adding a new version Load or Save:
//	o	Define a new static short RPalFile::LoadVersion#(RPal*, RFile*) 
//	where # is the number of the new version.
//
// o	Call your new LoadVersion#(...) in the switch statement in 
//	RPalFile::Load().
//
// o	Make sure the last versions' case in the switch is surrounded by a
//	#if defined(PAL_LOAD_num) #endif block where 'num' is the last version
// number. 
//
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// C Headers -- Must be included before RSPiX.h b/c RSPiX utilizes SHMalloc.
//////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// RSPiX Headers.
// If PATHS_IN_INCLUDES macro is defined, we can utilize relative
// paths to a header file.  In this case we generally go off of our
// RSPiX root directory.  System.h MUST be included before this macro
// is evaluated.  System.h is the header that, based on the current
// platform (or more so in this case on the compiler), defines 
// PATHS_IN_INCLUDES.  Blue.h includes system.h so you can include that
// instead.
///////////////////////////////////////////////////////////////////////////////
#include "System.h"

#ifdef PATHS_IN_INCLUDES
	#include "GREEN/Image/Image.h"
	#include "GREEN/Image/PalFile.h"
#else
	#include "Image.h"
	#include "PalFile.h"
#endif

//////////////////////////////////////////////////////////////////////////////
// Module specific macros.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Module specific typedefs.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Exported (extern) variables.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Module specific (static) variables / Instantiate class statics.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Module specific (static) protos.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Internal Functions.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Loads an RPal with no file version into ppal from pfile.
//
//////////////////////////////////////////////////////////////////////////////
short RPalFile::LoadNoVersion(	// Returns SUCCESS on success or FAILURE on
											// failure.
	RPal*		ppal,						// Pal to load into.
	RFile*	pfile)					// File to load from.
	{
	short	sRes	= SUCCESS;	// Assume success.

	// No RFile support for RPal::Type, so we used a U32.
	U32	u32Temp	= 0;
	pfile->Read(&u32Temp);
	ppal->m_type	= (RPal::Type)u32Temp;
	pfile->Read(&ppal->m_ulSize);
	pfile->Read(&ppal->m_sStartIndex);
	pfile->Read(&ppal->m_sNumEntries);
	pfile->Read(&ppal->m_sPalEntrySize);

	if (ppal->CreateData(ppal->m_ulSize) == SUCCESS)
		{
		pfile->Read(ppal->m_pData, ppal->m_ulSize);
		}
	else
		{
		TRACE("RPalFile::LoadNoVersion - Unable to allocate memory for the palette data\n");
		sRes = FAILURE;
		}

	if (pfile->Error())
		{
		TRACE("RPalFile::LoadNoVersion - Error reading palette\n");
		sRes = FAILURE;
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Loads an RPal with file version 1 into ppal from pfile.
//
//////////////////////////////////////////////////////////////////////////////
short RPalFile::LoadVersion1(	// Returns SUCCESS on success or FAILURE on
										// failure.
	RPal*		/*ppal*/,			// Pal to load into.
	RFile*	/*pfile*/)			// File to load from.
	{
	short	sRes	= SUCCESS;	// Assume success.

	TRACE("LoadVersion1(): No current support for version 1 RPal.\n");
	sRes	= FAILURE;

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Loads an RPal with file version 2 into ppal from pfile.
//
//////////////////////////////////////////////////////////////////////////////
short RPalFile::LoadVersion2(	// Returns SUCCESS on success or FAILURE on
										// failure.
	RPal*		/*ppal*/,			// Pal to load into.
	RFile*	/*pfile*/)			// File to load from.
	{
	short	sRes	= SUCCESS;	// Assume success.

	TRACE("LoadVersion2(): No current support for version 2 RPal.\n");
	sRes	= FAILURE;

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Loads an RPal with file version 3 into ppal from pfile.
//
//////////////////////////////////////////////////////////////////////////////
short RPalFile::LoadVersion3(	// Returns SUCCESS on success or FAILURE on
										// failure.
	RPal*		ppal,					// Pal to load into.
	RFile*	pfile)				// File to load from.
	{
	short	sRes	= SUCCESS;	// Assume success.

	// No RFile support for RPal::Type, so we used a U32.
	U32	u32Temp	= 0;
	pfile->Read(&u32Temp);
	ppal->m_type	= (RPal::Type)u32Temp;
	pfile->Read(&ppal->m_ulSize);
	pfile->Read(&ppal->m_sStartIndex);
	pfile->Read(&ppal->m_sNumEntries);
	pfile->Read(&ppal->m_sPalEntrySize);

	USHORT	usFlag	= 2;
	pfile->Read(&usFlag);
	if (usFlag == 1)
		{
		if (ppal->CreateData(ppal->m_ulSize) == SUCCESS)
			{
			pfile->Read(ppal->m_pData, ppal->m_ulSize);
			}
		else
			{
			TRACE("RPalFile::LoadVersion3 - Unable to allocate memory for the palette data\n");
			sRes = FAILURE;
			}
		}

	if (pfile->Error())
		{
		TRACE("RPalFile::LoadVersion3 - Error reading palette\n");
		sRes = FAILURE;
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
// External Functions.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Maps a particular file load onto the appropriate function, if available.
//
//////////////////////////////////////////////////////////////////////////////
short RPalFile::Load(			// Returns SUCCESS on success or FAILURE on failure.
	RPal*		ppal,					// Pal to load into.
	RFile*	pfile)				// File to load from.
	{
	short	sRes	= SUCCESS;	// Assume success.

	// Get finger print . . .
	ULONG	ulFinger;
	if (pfile->Read(&ulFinger) == 1)
		{
		if (ulFinger == PAL_COOKIE)
			{
			ULONG	ulVersion;
			if (pfile->Read(&ulVersion) == 1)
				{
				switch (ulVersion)
					{
					#if defined(PAL_LOAD_1) || defined(PAL_LOAD_ALL)
						case 1:
							sRes	= LoadVersion1(ppal, pfile);
							break;
					#endif	// PAL_LOAD_1

					#if defined(PAL_LOAD_2) || defined(PAL_LOAD_ALL)
						case 2:
							sRes	= LoadVersion2(ppal, pfile);
							break;
					#endif	// PAL_LOAD_2

//					#if defined(PAL_LOAD_3) || defined(PAL_LOAD_ALL)
						case 3:
							sRes	= LoadVersion3(ppal, pfile);
							break;
//					#endif	// PAL_LOAD_3

					default:	// No current support.
						TRACE("RPal::Load - Error: Unsupported version.\n");
						TRACE("RPal::Load - Current RPal version %d\n", PAL_CURRENT_VERSION);
						TRACE("RPal::Load - This file's version %lu\n", ulVersion);
						TRACE("RPal::Load - See PalFile.cpp for details on "
							"supporting older RPal formats.\n");
						sRes = FAILURE;
						break;
					}
				}
			else
				{
				TRACE("RPal::Load - Error reading file version\n");
				sRes = FAILURE;
				}
			}
		else
			{
			TRACE("RPal::Load - Error: Wrong filetype, RPal files should start with"
				"\"CPAL\"\n");
			TRACE("RPal::Load - Note that some older RPal formats had no filetype, and,\n");
			TRACE("therefore, can no longer be loaded unless in an RImage file.\n");
			sRes	= FAILURE;
			}
		}
	else
		{
		TRACE("ImageLoad(): Error reading RPal format finger print.\n");
		sRes	= FAILURE;
		}

	return sRes;
	}

///////////////////////////////////////////////////////////////////////////////
// EOF
///////////////////////////////////////////////////////////////////////////////
