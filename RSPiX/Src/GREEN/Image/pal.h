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
#ifndef PAL_H
#define PAL_H
///////////////////////////////////////////////////////////////////////////////
//
//	PAL.H
//   
// 
// 10/30/96	JMI	Broke CPal and some of its associates out of image.h and
//						imagetyp.h.
//
//	10/30/96	JMI	Changed:
//						Old label:			New label:
//						=========			=========
//						CNFile				RFile
//						CPal					RPal
//						ULONG ulType		RPal::Type ulType
//						m_bCanDestroyData	m_sCanDestroyData
//
//	11/01/96	JMI	Changed all members to be preceded by m_ (e.g., sDepth
//						m_sDepth).  Changed ulType to m_type.
//
// 12/08/96 MJR	Added Red(), Green() and Blue() functions to give users
//						access to those components without needed to know the palette
//						type.  Implimented entirely in this header.  See functions
//						for more information.
//
//	04/16/97	JMI	Added operator= overload.
//
///////////////////////////////////////////////////////////////////////////////
//
// This file contains RPal and its associates.  See Image.h/cpp for more info
// including info on conversions and types.
//
///////////////////////////////////////////////////////////////////////////////

// Blue include files
#include "Blue.h"

// If PATHS_IN_INCLUDES macro is defined, we can utilize relative
// paths to a header file.  In this case we generally go off of our
// RSPiX root directory.  System.h MUST be included before this macro
// is evaluated.  System.h is the header that, based on the current
// platform (or more so in this case on the compiler), defines 
// PATHS_IN_INCLUDES.  Blue.h includes system.h so you can include that
// instead.
#ifdef PATHS_IN_INCLUDES
	// Orange include files
	#include "ORANGE/File/file.h"
#else
	// Orange include files
	#include "file.h"
#endif // PATHS_IN_INCLUDES

///////////////////////////////////////////////////////////////////////////////
// Macros.
///////////////////////////////////////////////////////////////////////////////

#define PAL_CURRENT_VERSION 3	  // Current file verison of RPal
#define PAL_COOKIE 0x4c415043	  // Looks like "CPAL" in the file

///////////////////////////////////////////////////////////////////////////////
// Typedefs.
///////////////////////////////////////////////////////////////////////////////

typedef struct  
{
    UCHAR rgbtBlue;
    UCHAR rgbtGreen;
    UCHAR rgbtRed;
} IM_RGBTRIPLE;

typedef struct 
{
    UCHAR    rgbBlue;
    UCHAR    rgbGreen;
    UCHAR    rgbRed;
    UCHAR    rgbReserved;
} IM_RGBQUAD, *IM_PRGBQUAD;


//////////////////////////////////////////////////////////////////////
//
//	RPal class
//
// RPal is a simple wrapper around various types of palettes.
// This class does not understand any particular palette types.
// I'm not sure that the above sentence is true anymore....?
//
//////////////////////////////////////////////////////////////////////
class RPal
{
	public:	// Typedefs and Enums.
		// These palette types are set to correspond to the appropriate image type
		// in CImage::ImageType.  For example, PDIB is the same number as BMP8 
		// which is the image that uses PDIB.  If you are adding your own palette 
		// type, put it at the end of the list.  If you are creating a new image 
		// type to go with the new palette type, insert it in the appropriate 
		// place in the eImageTypes list in CImage::ImageType (Image.h).
		// Also remember to add the name of your type to the ms_astrTypeNames
		// array in pal.cpp and the size of palette entries of your type to the
		// aPalEntrySizes array in pal.cpp.
		typedef enum ePaletteTypes
		{
			NO_PALETTE,
			PDIB,					// BMP8			DIB format(RGBQUAD = BGR 888 + Reserved)
			PSYS,					// SYSTEM8		System Palette - platform dependant
			P555,					// SCREEE8_555	RGB 555 palette format
			P565,					// SCREEN8_565	RGB 565 palette format
			P888,					// SCREEN8_888 BGR 888 palette format - MJR 10/15/96 - "BGR" is correct order!
			PFLX,					// FLX8_888 RGB (in that order) 888 palette 
									// format.  03/06/96	JMI
			END_REG_PAL			// End of registered palette types
		} Type;


	public:	// Member vars.
		Type		m_type;				// Palette type
		ULONG		m_ulSize;			// Size of data
		short		m_sStartIndex;		// Starting index
		short		m_sNumEntries;		// Number of entries
		short		m_sPalEntrySize;	// Number of bytes in each palette entry
		UCHAR*	m_pData;			   // Pointer to data

		// This array of type names should correspond to the above list of
		// enumerated types.  Whenever you add an image type and an enum, 
		// you need to also insert that name into the corresponding place
		//	in this array in pal.cpp.  
		// Note that this uses END_REG_PAL enum item to size the array.
		static char* ms_astrTypeNames[END_REG_PAL];

		// This array gives the size in bytes of each palette entry
		// based on the palette type.  This is used by RPal::GetPalEntrySize
		// to return the size of any registered palette type.
		// Note that this uses END_REG_PAL enum item to size the array.
		static short ms_asPalEntrySizes[END_REG_PAL];

	private:
		short	m_sCanDestroyData;// Flags whether DestroyData can destroy
										// the data.

	public:
		RPal();
		~RPal();

		// Create default palette of the specified type.
		short CreatePalette(
			Type ulNewType);

		// Static member function that will tell you the number of 
		// bytes per palette entry for any registered palette type
		static short GetPalEntrySize(Type type);

		// Will tell you the number of bytes per palette entry for the
		// this instance of the palette
		short GetPalEntrySize()	{return m_sPalEntrySize;};

		// Create PAL's data using the specified values.
		short CreateData(	// Returns 0 if successful
			ULONG	ulSize);	// Size of data

		short CreateData(
			ULONG ulSize, 			// Size of data
			Type	type, 		// Palette type
			short sPalEntrySize, // Size in bytes of each Pal entry
			short sStartIndex,	// Starting index of colors
			short sNumEntries);	// Number of significant palette entries

		// Destroy PAL's data
		short DestroyData();

		// Allow the user to set the data pointer.
		short SetData(void* pData);

		// Save the palette to the given file
		short Save(char* pszFilename);

		// Save the palette to the open RFile
		short Save(RFile* pcf);

		// Load palette data from the given file
		short Load(char* pszFilename);

		// Load palette data from the open RFile
		short Load(RFile* pcf);

		// Convert the palette to the new palette type
		Type Convert(Type typeNew);

		// Detach the data from the Palette.  This function returns a pointer
		// to the memory buffer which can and should be freed by whoever
		// detached it.  This function is usefull when doing conversions between
		// palette types.  You can detach the buffer from the palette, have the
		// palette create a new buffer (for the converted data) and then free
		// the detached buffer when you're done with the conversion.
		UCHAR* DetachData();

		// Gets a pointer to the red, green or blue component of the specified
		// palette entry.  This presents a method of accessing this data in a
		// generic manner, without needing to know the palette type.  That said,
		// it's unfortunate that this only works with palettes that are based
		// around the concept of one byte per component.  In an attempt to make
		// these functions more usable, no result code is returned.  Instead,
		// the returned pointers will be 0 if the current palette type is not
		// supported, and a TRACE message will be generated.  It's far from
		// perfect, but it's better than having everyone implimenting some
		// version of these functions when they need to access the palette, in
		// which case they are already making assumptions about its format.
		unsigned char* Red(
			short sStart = 0)							// In:  Starting palette entry
			{
			// Calculate pointer to specified entry
			unsigned char* pucDst = m_pData + ((sStart - m_sStartIndex) * m_sPalEntrySize);

			// If it's a supported palette type, return the pointer.  Otherwise, return 0.
			if (m_type == PDIB)	// BGR888+reserved
				return pucDst+2;
			else if (m_type == P888) // BGR888
				return pucDst+2;
			else if (m_type == PFLX) // RGB888
				return pucDst+0;
			TRACE("RPal::Red(): Unsupported palette format - returning NULL!\n");
			return 0;
			}

		unsigned char* Green(
			short sStart = 0)							// In:  Starting palette entry
			{
			// Calculate pointer to specified entry
			unsigned char* pucDst = m_pData + ((sStart - m_sStartIndex) * m_sPalEntrySize);

			// If it's a supported palette type, return the pointer.  Otherwise, return 0.
			if (m_type == PDIB)	// BGR888+reserved
				return pucDst+1;
			else if (m_type == P888) // BGR888
				return pucDst+1;
			else if (m_type == PFLX) // RGB888
				return pucDst+1;
			TRACE("RPal::Green(): Unsupported palette format - returning NULL!\n");
			return 0;
			}

		unsigned char* Blue(
			short sStart = 0)							// In:  Starting palette entry
			{
			// Calculate pointer to specified entry
			unsigned char* pucDst = m_pData + ((sStart - m_sStartIndex) * m_sPalEntrySize);

			// If it's a supported palette type, return the pointer.  Otherwise, return 0.
			if (m_type == PDIB)	// BGR888+reserved
				return pucDst+0;
			else if (m_type == P888) // BGR888
				return pucDst+0;
			else if (m_type == PFLX) // RGB888
				return pucDst+2;
			TRACE("RPal::Blue(): Unsupported palette format - returning NULL!\n");
			return 0;
			}

		// Get RGB entries from palette
		short GetEntries(
			short sStart,								// In:  Starting palette entry
			short sCount,								// In:  Number of entries to do
			unsigned char* pDstRed,					// Out: Starting destination red value
			unsigned char* pDstGreen,				// Out: Starting destination green value
			unsigned char* pDstBlue,				// Out: Starting destination blue value
			long lAddToPointers);					// In:  What to add to pointers to move to next value

		short SetEntries(
			short sStart,								// In:  Starting palette entry
			short sCount,								// In:  Number of entries to do
			unsigned char* pSrcRed,					// In:  Starting source red value
			unsigned char* pSrcGreen,				// In:  Starting source green value
			unsigned char* pSrcBlue,				// In:  Starting source blue value
			long lAddToPointers);					// In:  What to add to pointers to move to next value

		// Copy operator overload.
		// Note that this function could fail.
		RPal& operator=(RPal &palSrc);

};


#endif // PAL_H
///////////////////////////////////////////////////////////////////////////////
// EOF
///////////////////////////////////////////////////////////////////////////////
