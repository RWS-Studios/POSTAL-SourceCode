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
// ImageFile.CPP
// 
// History:
//		12/11/96 JMI	Started.
//
//		12/19/96	JMI	Took out comment regarding adding a version to LOADFUNC.
//
//		02/04/97	JMI	Changed #include "ImageLoad.h" to "ImageFile.h".
//
//		02/04/97	JMI	Now RImageFile::Load() will invoke RImage->LoadDib() for
//							BMP formatted files.
//
//////////////////////////////////////////////////////////////////////////////
//
// Handles file formats for RImage.  Some of these older formats will not be 
// useful to developers of new code.  To include the ability to use a certain
// loader, define one of the macros IMAGE_LOAD_# where # is the number of the
// format desired OR define the macro IMAGE_LOAD_ALL for all available
// support.
//
// NOTES TO THOSE ADDING NEW FILE VERSIONS:
//	Please follow these steps when adding a new version Load or Save:
//	o	Define a new static short RImageFile::LoadVersion#(RImage*, RFile*) 
//	where # is the number of the new version.
//
// o	Call your new LoadVersion#(...) in the switch statement in 
//	RImageFile::Load().
//
// o	Make sure the last versions' case in the switch is surrounded by a
//	#if defined(IMAGE_LOAD_num) #endif block where 'num' is the last version
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
	#include "GREEN/Image/ImageFile.h"
	#include "GREEN/Image/PalFile.h"
#else
	#include "Image.h"
	#include "ImageFile.h"
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
// Loads an RImage with file version 1 into pim from pfile.
//
//////////////////////////////////////////////////////////////////////////////
short RImageFile::LoadVersion1(	// Returns SUCCESS on success or FAILURE on
											// failure.
	RImage*	/*pim*/,					// Image to load into.
	RFile*	/*pfile*/)				// File to load from.
	{
	short	sRes	= SUCCESS;	// Assume success.

	TRACE("LoadVersion1(): No current support for version 1 RImage.\n");
	sRes	= FAILURE;

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Loads an RImage with file version 2 into pim from pfile.
//
//////////////////////////////////////////////////////////////////////////////
short RImageFile::LoadVersion2(	// Returns SUCCESS on success or FAILURE on
											// failure.
	RImage*	pim,						// Image to load into.
	RFile*	pfile)					// File to load from.
	{
	short	sRes	= SUCCESS;	// Assume success.

	// No RFile support for RImage::Type, so we used a U32.
	U32	u32Temp				= 0;
	
	pfile->Read(&u32Temp);
	pim->m_type					= (RImage::Type)u32Temp;
	
	pfile->Read(&u32Temp);
	pim->m_typeDestination	= (RImage::Type)u32Temp;

	pfile->Read(&pim->m_ulSize);

	// Note that the window ones may need to be read after the other ones or,
	// more likely, we may need to put in the same patch that is in version 5.
	pfile->Read(&u32Temp);
	pim->m_sWinWidth			= (short)u32Temp;

	pfile->Read(&u32Temp);
	pim->m_sWinHeight			= (short)u32Temp;

	pfile->Read(&u32Temp);
	pim->m_sWidth				= (short)u32Temp;

	pfile->Read(&u32Temp);
	pim->m_sHeight				= (short)u32Temp;

	pfile->Read(&u32Temp);
	pim->m_sWinX				= (short)u32Temp;

	pfile->Read(&u32Temp);
	pim->m_sWinY				= (short)u32Temp;

	pfile->Read(&pim->m_lPitch);
	pfile->Read(&pim->m_sDepth);

	// If file is still okay . . .
	if (pfile->Error() == FALSE)
		{
		if (pim->CreateData(pim->m_ulSize) == SUCCESS)
			{
			if (pim->ReadPixelData(pfile) == SUCCESS)
				{
				ULONG ulPalSize;
				if (pfile->Read(&ulPalSize) == 1)
					{
					if (ulPalSize > 0)
						{
						pim->CreatePalette();
						if (RPalFile::LoadNoVersion(pim->m_pPalette, pfile) != SUCCESS)
							{
							TRACE("RImageFile::LoadVersion2 - Error loading palette\n");
							pim->DestroyPalette();
							sRes = FAILURE;
							}
						}
					}
				else
					{
					TRACE("RImageFile::LoadVersion2 - Error checking for palette\n");
					sRes = FAILURE;
					}
				}
			else
				{
				TRACE("RImageFile::LoadVersion2 - Error reading pixel data\n");
				pim->DestroyData();
				sRes = FAILURE;
				}
			}
		else
			{
			TRACE("RImageFile::LoadVersion2 - Error: Cannot create memory for "
				"pixel data\n");
			sRes = FAILURE;
			}
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Loads an RImage with file version 3 into pim from pfile.
//
//////////////////////////////////////////////////////////////////////////////
short RImageFile::LoadVersion3(	// Returns SUCCESS on success or FAILURE on
											// failure.
	RImage*	/*pim*/,					// Image to load into.
	RFile*	/*pfile*/)				// File to load from.
	{
	short	sRes	= SUCCESS;	// Assume success.

	TRACE("LoadVersion3(): No current support for version 3 RImage.\n");
	sRes	= FAILURE;

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Loads an RImage with file version 4 into pim from pfile.
//
//////////////////////////////////////////////////////////////////////////////
short RImageFile::LoadVersion4(	// Returns SUCCESS on success or FAILURE on failure.
	RImage*	/*pim*/,					// Image to load into.
	RFile*	/*pfile*/)				// File to load from.
	{
	short	sRes	= SUCCESS;	// Assume success.

	TRACE("LoadVersion4(): No current support for version 4 RImage.\n");
	sRes	= FAILURE;

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Loads an RImage with file version 5 into pim from pfile.
//
//////////////////////////////////////////////////////////////////////////////
short RImageFile::LoadVersion5(	// Returns SUCCESS on success or FAILURE on
											// failure.
	RImage*	pim,						// Image to load into.
	RFile*	pfile)					// File to load from.
	{
	short	sRes	= SUCCESS;	// Assume success.

	// No RFile support for RImage::Type, so we used a U32.
	U32	u32Temp				= 0;
	pfile->Read(&u32Temp);
	pim->m_type					= (RImage::Type)u32Temp;
	pfile->Read(&u32Temp);
	pim->m_typeDestination	= (RImage::Type)u32Temp;
	pfile->Read(&pim->m_ulSize);
	pfile->Read(&pim->m_sWinWidth);
	pfile->Read(&pim->m_sWinHeight);
	pfile->Read(&pim->m_sWidth);
	pfile->Read(&pim->m_sHeight);

	/////////////////////////////////////////////////////////////
	// Begin load patch to support older versions of RImage that
	// that used sBufferWidth and sBufferHeight in a
	// contraversial way.
	// Many images, in the days of m_sBufferWidth and 
	// m_sBufferHeight, saw these vars as representing the width
	// and height of the sub buffer within the image defined by
	// m_sWidth and m_sHeight.  This was incorrect, as Jeff had
	// intended these m_sBuffer* to represent the overall size of
	// the buffer, and m_sWidth and m_sHeight to represent the
	// rectangular sub region.	 Consequently, these incorrect
	// images spread far and wide and even crept into other
	// formats, such as SPRY and kludge, seeping their incorrect
	// interpretations of the Image.
	// Today, images use m_sWidth and m_sHeight to represent the
	// entire buffer and new vars (m_sWinWidth & m_sWinHeight)
	// to represent the sub "window" inside the buffer.  To make
	// this work with older formats, the the pair (m_sWidth,
	// m_sHeight) is read in before the pair (m_sWinWidth, 
	// m_sWinHeight).  This is intended to make the old
	// (m_sBufferWidth, m_sBufferHeight) pair from the files get
	// stored in the new (m_sWidth, m_sHeight) pair in memory and
	// have the old (m_sWidth, m_sHeight) pair from the files get
	// stored in the new (m_sWinWidth, m_sWinHeight).  With this
	// change the file version would not have to be incremented
	// for *.IMG files.  The only problem here is that files,
	// such as *.SPRY, had their m_sBufferWidth and
	// m_sBufferHeight set to 0, incorrectly, as they did not
	// understand the proper use of these vars; hence, the patch
	// below which detects and fixes this problem.
	// To fix the problem, we check for the incorrect condition
	// of having the m_sWidth and m_sHeight vars be larger than
	// the m_sWinWidth and m_sWinHeight vars.  If this is the
	// case, we copy the m_sWinWidth and m_sWinHeight into the 
	// m_sWidth and m_sHeight vars.
	/////////////////////////////////////////////////////////////

	if (pim->m_sWinWidth > pim->m_sWidth && pim->m_sWinHeight > pim->m_sHeight)
		{
		pim->m_sWidth		= pim->m_sWinWidth;
		pim->m_sHeight		= pim->m_sWinHeight;
		}

	/////////////////////////////////////////////////////////////
	// End patch to support older versions of RImage.
	/////////////////////////////////////////////////////////////

	pfile->Read(&pim->m_sWinX);
	pfile->Read(&pim->m_sWinY);
	pfile->Read(&pim->m_lPitch);
	pfile->Read(&pim->m_sDepth);

	// If file is still okay . . .
	if (pfile->Error() == FALSE)
		{
		USHORT usFlag;
		// See if there is any pixel data to be read
		pfile->Read(&usFlag);
		if (usFlag == 1)
			{
			if (pim->CreateData(pim->m_ulSize) == SUCCESS)
				{
				pim->ReadPixelData(pfile);
				}
			else
				{
				TRACE("RImageFile::LoadVersion5 - Error: Cannot allocate memory "
					"for pixel data\n");
				sRes = FAILURE;
				}
			}
		}

	// If file is still okay . . .
	if (pfile->Error() == FALSE)
		{
		USHORT usFlag;
		// See if there is a palette to load
		pfile->Read(&usFlag);
		if (usFlag == 1)
			{
			pim->CreatePalette();
			if (pim->m_pPalette->Load(pfile) != SUCCESS)
				{
				TRACE("RImageFile::LoadVersion5 - Error loading palette\n");
				pim->DestroyPalette();
				sRes = FAILURE;
				}
			}
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
short RImageFile::Load(			// Returns SUCCESS on success or FAILURE on failure.
	RImage*	pim,					// Image to load into.
	RFile*	pfile)				// File to load from.
	{
	short	sRes	= SUCCESS;	// Assume success.

	// Get finger print . . .
	ULONG	ulFinger;
	if (pfile->Read(&ulFinger) == 1)
		{
		if (ulFinger == IMAGE_COOKIE)
			{
			ULONG	ulVersion;
			if (pfile->Read(&ulVersion) == 1)
				{
				switch (ulVersion)
					{
					#if defined(IMAGE_LOAD_1) || defined(IMAGE_LOAD_ALL)
						case 1:
							sRes	= LoadVersion1(pim, pfile);
							break;
					#endif	// IMAGE_LOAD_1

					#if defined(IMAGE_LOAD_2) || defined(IMAGE_LOAD_ALL)
						case 2:
							sRes	= LoadVersion2(pim, pfile);
							break;
					#endif	// IMAGE_LOAD_2

					#if defined(IMAGE_LOAD_3) || defined(IMAGE_LOAD_ALL)
						case 3:
							sRes	= LoadVersion3(pim, pfile);
							break;
					#endif	// IMAGE_LOAD_3

					#if defined(IMAGE_LOAD_4) || defined(IMAGE_LOAD_ALL)
						case 4:
							sRes	= LoadVersion4(pim, pfile);
							break;
					#endif	// IMAGE_LOAD_4

//					#if defined(IMAGE_LOAD_5) || defined(IMAGE_LOAD_ALL)
						case 5:
							sRes	= LoadVersion5(pim, pfile);
							break;
//					#endif	// IMAGE_LOAD_5

					default:	// No current support.
						TRACE("RImage::Load - Error: Unsupported version.\n");
						TRACE("RImage::Load - Current RImage version %d\n", IMAGE_CURRENT_VERSION);
						TRACE("RImage::Load - This file's version %lu\n", ulVersion);
						TRACE("RImage::Load - See ImageFile.cpp for details on "
							"supporting older RImage formats.\n");
						sRes = FAILURE;
						break;
					}

				// If no errors . . .
				if (sRes == SUCCESS && !pfile->Error())
					{
					// Call the special load function for this type if any
					LOADFUNC clf = GETLOADFUNC(pim->m_type);
					if (clf != NULL)
						sRes = (*clf)(pim, pfile/*, ulVersion*/);
					}
				}
			else
				{
				TRACE("RImage::Load - Error reading file version\n");
				sRes = FAILURE;
				}
			}
		else
			{
			// If it matches the BMP cookie . . . 
			if ((ulFinger & 0x0000FFFF) == BMP_COOKIE)
				{
				// Seek back for LoadDib() call.  99% of the time this will not cause a seek
				// b/c it's within the buffered i/o's buffer.
				// The other 1%.....uhhh...well.
				pfile->Seek( - (long)sizeof(ulFinger), SEEK_CUR);
				
				// Invoke LoadDib() . . .
				sRes	= pim->LoadDib(pfile);
				}
			else
				{
				TRACE("RImage::Load - Error: Wrong filetype, Image files should start with 'IM  ' and "
					"DIB files should start with 'BM'.\n");
				sRes	= FAILURE;
				}
			}
		}
	else
		{
		TRACE("ImageLoad(): Error reading RImage format finger print.\n");
		sRes	= FAILURE;
		}

	return sRes;
	}

///////////////////////////////////////////////////////////////////////////////
// EOF
///////////////////////////////////////////////////////////////////////////////
