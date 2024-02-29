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
// Pal.CPP
// 
// History:
// 10/30/96	JMI	Broke CPal and some of its associates out of image.cpp and
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
//						Also, Convert() contained a "lack of m_" bug where it was
//						unintentionally doing nothing b/c it was using ulType off
//						of the stack when it meant the one in the class.
//
//						The thing that annoys me the most about using actual enums
//						instead of ULONGs is that you have to copy it into a dummy
//						ULONG to use RFile on it.  This isn't very bad, but it's
//						annoying.
//
//	11/01/96	JMI	Changed all members to be preceded by m_ (e.g., sDepth
//						m_sDepth).  Changed ulType to m_type.
//						Also, fixed bug in CreatePalette() where it was bounds
//						checking the new passed in type against the minimum type and
//						checking the member ulType against the maximum type.  Changed
//						this to perform both checks with the passed in type (which
//						was ulNewType).
//
//	12/13/96	JMI	Now calls RPalFile::Load(...) to load images.
//						RPalFile::Load(...) has the advantage of potentially
//						supporting older formats.
//
//	04/16/97	JMI	Added operator= overload.
//
//	06/28/97 MJR	Added <string.h> for latest mac compatability.
//
///////////////////////////////////////////////////////////////////////////////
//
// This file contains RPal and its associates.  See pal.h/Image.h/cpp for more
// info including info on conversions and types.
//
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
	// Green include files
	#include "GREEN/Image/pal.h"
	#include "GREEN/Image/Image.h"
	#include "GREEN/Image/PalFile.h"
#else
	// Green include files
	#include "pal.h"
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

// This array of type names should correspond to the list of
// enumerated types eImageTypes in image.h.  Whenever you add an image
// type and an enum, you need to also insert that name into the 
// corresponding place in this array.  
// Note that this uses END_REG_PAL enum item to size the array.

char* RPal::ms_astrTypeNames[END_REG_PAL] = 
{
	"Same Type",
	"PDIB BGRA 8888 (RGBQUAD)",
	"PSYS System dependent",
	"P555 RGB 555",
	"P565 RGB 565",
	"P888 BGR 888",
	"PFLX RGB 888",	
};

//////////////////////////////////////////////////////////////////////////////
// Module specific (static) protos.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Functions.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
///////////////////////////// RPal ///////////////////////////////////
//////////////////////////////////////////////////////////////////////

// This array gives the size in bytes of each palette entry
// based on the palette type.  This is used by RPal::GetPalEntrySize
// to return the size of any registered palette type.
// Note that this uses END_REG_PAL enum item to size the array.
short RPal::ms_asPalEntrySizes[END_REG_PAL] = 
{
	0,		//NO_PALETTE
	4,		//PDIB
	4,		//PSYS
	2,		//P555
	2,		//P565
	3,		//P888
	3,		//PFLX	03/06/96	JMI
};

//////////////////////////////////////////////////////////////////////
//
//	RPal Member Functions
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//
// Constructor
//
// Description:
//		Initialize member variables
//
// Parameters:
//		none
//
// Returns:
//		none
//
//////////////////////////////////////////////////////////////////////

RPal::RPal()
{
	//	Initialize member variables to zero
	m_type				= NO_PALETTE;                           
	m_ulSize				= 0;
	m_sStartIndex		= 0;
	m_sNumEntries		= 0;
	m_sPalEntrySize	= 0;
	m_pData				= NULL;
	m_sCanDestroyData	= FALSE;
} 

//////////////////////////////////////////////////////////////////////
//
// Destructor
//
// Description:
//		Deallocate memory for the palette buffer
//
// Parameters:
//		none
//
// Returns:
//		none
//
//////////////////////////////////////////////////////////////////////

RPal::~RPal()
{
	// Free Palette data
	if (m_sCanDestroyData && m_pData)
		DestroyData();
}


//////////////////////////////////////////////////////////////////////
//
// CreatePalette
//
// Description:
//		Creates a default palette of the specified type.  All the
// 	appropriate member variables are set to the default values for
//		this type of palette.  The appropriate amount of data is
//		allocated, too (but the data itself is uninitialized).
//		All values in the newly allocated data will be zero.
//
// Parameters:
//		(see function declaration below)
//
// Returns:
//		SUCCESS if it worked
//		FAILURE if it didn't
//
//////////////////////////////////////////////////////////////////////
short RPal::CreatePalette(
	Type typeNew)
	{
	short sResult = SUCCESS;
	
	if ((typeNew >= NO_PALETTE) && (typeNew < END_REG_PAL))
		{
		// Get size of palette entry
		short size = GetPalEntrySize(typeNew);
		
		// Set number of entries to 256.  This would probably only change
		// if we add support for 1-, 2- or 4-bit image formats that would
		// require smaller palettes.
		short entries = 256;
		CreateData(
			size * entries,
			typeNew,
			size,
			0,
			entries);
		}
	else
		{
		TRACE("RPal::CreatePalette(): Not a registered palette type!\n");
		sResult = FAILURE;
		}
	
	return sResult;
	}
	
	
//////////////////////////////////////////////////////////////////////
//
// GetPalEntrySize
//
// Description:
//		This is the static member function that will return the
//		size in bytes of any registered palette type.
//
// Parameters:
//		ulType = registered palette type (see pal.h)
//
// Returns:
//		number of bytes of each palette entry for ulType
///	-1 if the type given is not a registered type
//
//////////////////////////////////////////////////////////////////////

short RPal::GetPalEntrySize(Type type)
{
 	if (type < END_REG_PAL)
		return ms_asPalEntrySizes[type];
	else
	{
	 	TRACE("RPal::GetPalEntrySize - Not a registered palette type\n");
		return -1;
	}
}

//////////////////////////////////////////////////////////////////////
//
// RPal::CreateData
//
// Description:
//		Create a buffer for the palette data of the given size
//
// Parameters:
//		ulNewSize = size in bytes of the buffer
//		---------------------------------------
//		ulSetType = Palette type
//		sSetPalEntrySize = number of bytes per palette entry
//		sSetStartIndex = Where palette entries start in the palette
//		sSetNumEntries	= Number of palette entries used after sSetStartIndex
//
// Returns:
//		SUCCESS if the memory was allocated
//		FAILURE if the memory could not be allocated
//
//////////////////////////////////////////////////////////////////////

short	RPal::CreateData(ULONG ulNewSize)
{
	if (m_sCanDestroyData && m_pData)
	{
	 	TRACE("RPal::CreateData - Attempted to create new data while "
			"m_pData is still pointing to valid memory\n");
		return FAILURE;
	}

	if (m_pData && !m_sCanDestroyData)
		TRACE("RPal::CreateData - Warning m_pData != NULL\n");

	m_sCanDestroyData = TRUE;
	m_ulSize = ulNewSize;
	return RImage::sCreateMem((void**) &m_pData, ulNewSize);
}

short RPal::CreateData(ULONG ulNewSize, 
                       Type  typeNew,
							  short sSetPalEntrySize,
							  short sSetStartIndex,
							  short sSetNumEntries)
{
	short sReturn = CreateData(ulNewSize);

	if (sReturn == SUCCESS)
	{
		m_type				= typeNew;
		m_sPalEntrySize	= sSetPalEntrySize;
		m_sStartIndex		= sSetStartIndex;
		m_sNumEntries		= sSetNumEntries;
	}
	return sReturn;
}


//////////////////////////////////////////////////////////////////////
//
// RPal::DestroyData
//
// Description:
//		Deallocate the palette buffer created by CreateData()
//
// Paremeters:
//		none
//
// Returns:
//		SUCCESS if the memory was deallocated
//		FAILURE if the palette buffer was not allocated by CreateData(),
//		        but instead set by using SetData() in which case the
//				  user is responsible for deallocating the buffer
//
//////////////////////////////////////////////////////////////////////

short	RPal::DestroyData()
{   
	// Only if the data was not supplied by the user.
	if (m_sCanDestroyData)
		{
		m_sCanDestroyData	= FALSE;
		return RImage::sDestroyMem((void**) &m_pData);
		}
	else 
	{
	 	TRACE("RPal::DestroyData - Attempted to free data supplied by user via SetData\n");
		TRACE("                    You are responsible for freeing m_pData\n");
		return FAILURE;
	}
}

//////////////////////////////////////////////////////////////////////
//
// RPal::SetData
//
// Description:
//		Allows the user to give their own buffer for the palette.  
//		The user is responsible for deallocating this buffer when
//		they are done using it.
//
// Parameters:
//		pUserData = pointer to the buffer that will be used as the
//					   palette's buffer
//
// Returns:
//		FAILURE if the buffer passed in is NULL or if there was
//			     already a palette buffer
//		SUCCESS if the buffer was set
//
//////////////////////////////////////////////////////////////////////

short RPal::SetData(void* pUserData)
{
	if (pUserData != NULL && m_pData == NULL)
	{
		m_pData = (UCHAR*) pUserData;
		m_sCanDestroyData = FALSE;
		return SUCCESS;
	}
	else
	{
	 	TRACE("RPal::SetData - Your data handle or data pointer is NULL\n");
		return FAILURE;
	}
}

//////////////////////////////////////////////////////////////////////
//
// DetachData
//
// Description:
//		This function detaches the memory buffer from the RPal and
//		returns a pointer to it.  This allows the RPal to now create
//		a new buffer without freeing the previous one.  This is useful
//		for palette conversion functions where you want to keep the 
//		same RPal object but just change its data.  The conversion
//		function can create a new RPal and set its pData pointer to
//		the detached palette buffer.  Then the original RPal can 
//		create a new buffer to accept the converted data.  When the
//		conversion function is finished it can delete the RPal it
//		created and along with it, the data.
//
// Parameters:
//		none
//
// Returns:
//		pointer to pData
//
//////////////////////////////////////////////////////////////////////

UCHAR* RPal::DetachData(void)
{
 	UCHAR* pDetachment = m_pData;
	m_pData = NULL;
	m_sCanDestroyData = FALSE;
	return pDetachment;
}

//////////////////////////////////////////////////////////////////////
//
// Convert
//
// Description:
//		This function creates a temporary image of the type that 
//		matches the palette type and then calls the RImage::Convert
//		function to convert the palette to the given format
//
// Parameters:
//		ulType = One of the enumerated palette types from ePaletteType
//
// Returns:
//		ulType if successful
//		NO_PALETTE if there is no appropriate palette conversion
//		
//////////////////////////////////////////////////////////////////////

RPal::Type RPal::Convert(Type typeNew)
{
	if (typeNew >= END_REG_PAL)
		return NO_PALETTE;
	else
	{
		RImage* pTempImage = new RImage();
		// Make image an equivalent RImage type to our current RPal type.
		// Note that there was one of those bugs here b/c of a lack of
		// m_.
		pTempImage->m_type = (RImage::Type)m_type;
		pTempImage->SetPalette(this);
		// Convert RImage to type equivalent to desired RPal type.
		// Note that this reference to typeNew is the local, not the
		// member.
		pTempImage->Convert((RImage::Type)typeNew);
		// Done with image.
		delete pTempImage;
		// Return resulting RPal type.
		return m_type;
	}
}


//////////////////////////////////////////////////////////////////////
//
// Save
//
// Description:
//		Saves the palette information to the given file.  
//
// Parameters:
//		pszFilename = name of the file to be opened for write
//
// Returns:
//		SUCCESS if the file was written
//		FAILURE otherwise
//
//////////////////////////////////////////////////////////////////////

short RPal::Save(char* pszFilename)
{
	RFile cf;
	short sReturn = SUCCESS;

	if (cf.Open(pszFilename, "wb", RFile::LittleEndian) != SUCCESS)
	{
		TRACE("RPal::Save - could not open file for output\n");
	 	return FAILURE;
	}

	sReturn = Save(&cf);

	cf.Close();

	return sReturn;
}

//////////////////////////////////////////////////////////////////////
//
// Save
//
// Description:
//		Saves the palette to the given RFile.  It is assumed that
//		the RFile pointer refers to an open file.  This version
//		of the save is used by RImage::Save when it writes out the
//		image data, it then calls this RPal::Save to write the palette
//		data to the current location in the file.
//
// Parameters:
//		pcf = pointer to an open RFile
//
// Returns:
//		SUCCESS if the file was written
//		FAILURE otherwise
//
//////////////////////////////////////////////////////////////////////

short RPal::Save(RFile* pcf)
{
	short sReturn = SUCCESS;
	ULONG ulFileType = PAL_COOKIE;
	ULONG ulCurrentVersion = PAL_CURRENT_VERSION;

	if (pcf && pcf->IsOpen())
	{
		pcf->ClearError();
		pcf->Write(&ulFileType);
		pcf->Write(&ulCurrentVersion);
		// No RFile support for RImage::Type, so we used a U32.
		U32	u32Temp	= (ULONG)m_type;
		pcf->Write(&u32Temp);
		pcf->Write(&m_ulSize);
		pcf->Write(&m_sStartIndex);
		pcf->Write(&m_sNumEntries);
		pcf->Write(&m_sPalEntrySize);
		if (m_pData)
		{
			USHORT usFlag = 1;
			pcf->Write(&usFlag);
			pcf->Write(m_pData, m_ulSize);
		}
		else
		{
			USHORT usFlag = 0;
			pcf->Write(&usFlag);
		}
		if (pcf->Error())
		{
			TRACE("RPal::Save - Error writing palette data\n");
			sReturn = FAILURE;
		}
	}
	else
	{
		TRACE("RPal::Save - Error: The RFile does not refer to an open file\n");
		sReturn = FAILURE;
	}

	return sReturn;
}

///////////////////////////////////////////////////////////////////////////////
//
// Load
//
// Description:
//		This version of the load takes a filename and opens the PAL file.  
//		
// Parameters:
//		pszFilename = File from which to load the palette
//
// Returns:
//		SUCCESS if the palette was loaded
//		FAILURE otherwise
//				  TRACE messages will help pinpoint the failure
//
///////////////////////////////////////////////////////////////////////////////

short RPal::Load(char* pszFilename)
{
	RFile cf;
	short sReturn = SUCCESS;

	if (cf.Open(pszFilename, "rb", RFile::LittleEndian) != SUCCESS)
	{
		TRACE("RPal::Load - could not open file for input\n");
	 	return FAILURE;
	}

	sReturn = Load(&cf);

	cf.Close();

	return sReturn;
}

///////////////////////////////////////////////////////////////////////////////
//
// Load
//
// Description:
//		This version of the load takes a pointer to an open RFile and loads
//		the palette from the current position.  This version of the RPal::Load
//		function is called by RImage::Load when it is reading a RImage that
//		contains a palette.
//		
// Parameters:
//		pszFilename = File from which to load the palette
//
// Returns:
//		SUCCESS if the palette was loaded
//		FAILURE otherwise
//				  TRACE messages will help pinpoint the failure
//
///////////////////////////////////////////////////////////////////////////////

short RPal::Load(RFile* pcf)
{	
	short sReturn = SUCCESS;
	ULONG ulFileType = 0;
	ULONG ulFileVersion = 0;
	USHORT usFlag = 2;

	if (pcf && pcf->IsOpen())
	{
		pcf->ClearError();

		sReturn	= RPalFile::Load(this, pcf);
	}
	else
	{
		TRACE("RPal::Load - Error: RFile pointer does not refer to an open file\n");
		sReturn = FAILURE;
	}

	return sReturn;
}


///////////////////////////////////////////////////////////////////////////////
//
// GetEntries
//
// Description:
//		Gets one or more entries from this palette.  This function offers a
// 	standardized method for accessing the palette data, regardless of its
// 	format or type.
//		
//		Separate pointers are used for each component (red, green, blue, and
//		alpha) so this function can access those components regardless of what
//		order they're stored in.  The lAddToPointers parameter specifies the
//		value to be added to the pointers so they point to the next set of values.
//
//		All palette formats ought to be supported, but if someone gets lazy,
//		then this will fail for unsupported types.
//
//		At some point, if alpha info becomes more prevalent, an overloaded
//		version of this function should be created to supply alpha info along
//		with red/green/blue info.  This functionality was momentarily built into
// 	this function, but was quickly removed when it became obvious that it
//		would slow things down in cases where there was no alpha info, or more
//		importantly, when the user passed a NULL to indicate a lack of alpha data.
//
// Parameters:
//		(see function declaration below)
//
// Returns:
//		SUCCESS if the palette was loaded
//		FAILURE otherwise
//				  TRACE messages will help pinpoint the failure
//
///////////////////////////////////////////////////////////////////////////////
short RPal::GetEntries(
	short sStart,								// In:  Starting palette entry
	short sCount,								// In:  Number of entries to do
	unsigned char* pDstRed,					// Out: Starting destination red value
	unsigned char* pDstGreen,				// Out: Starting destination green value
	unsigned char* pDstBlue,				// Out: Starting destination blue value
	long lAddToPointers)						// In:  What to add to pointers to move to next value
	{
	// Validate parameters
	ASSERT(pDstRed != NULL);
	ASSERT(pDstGreen != NULL);
	ASSERT(pDstBlue != NULL);
	if ((sStart < m_sStartIndex) || ((sStart + sCount - 1) > (m_sStartIndex + m_sNumEntries - 1)))
		{
		TRACE("RPal::GetEntries(): Specified range (%d to %d) exceeds palette's range (%d to %d)!\n",
			sStart, m_sStartIndex, m_sStartIndex + m_sNumEntries - 1);
		return FAILURE;
		}
	if (sCount < 0)
		{
		TRACE("RPal::GetEntries(): The count cannot be negative!\n");
		return FAILURE;
		}
		
	short sResult = SUCCESS;

	// Calculate pointer to first entry to be modified
	unsigned char* pucSrc = m_pData + ((sStart - m_sStartIndex) * m_sPalEntrySize);
	unsigned char green;

	switch(m_type)
		{
		case NO_PALETTE:
			TRACE("RPal::GetEntries - No palette!\n");
			sResult = FAILURE;
			break;
			
		case PDIB:
			// Source format: BGR888+reserved
			while (sCount--)
				{
				*pDstBlue  = *pucSrc++;
				pDstBlue  += lAddToPointers;
				*pDstGreen = *pucSrc++;
				pDstGreen += lAddToPointers;
				*pDstRed   = *pucSrc++;
				pDstRed   += lAddToPointers;
				pucSrc++;
				}
			break;

		case PSYS:
			TRACE("RPal::GetEntries - Does not support PSYS (system-depedendant palette)!\n");
			sResult = FAILURE;
			break;
			
		case P555:
			// Source format (bits): 0rrrrrgg gggbbbbb
			// To avoid endian issues this was done by bytes instead of words!
			while (sCount--)
				{
				*pDstRed = (*pucSrc << 1) & 0xf8;
				green = (*pucSrc++ << 6) & 0xc0;
				*pDstGreen = green | ((*pucSrc >> 2) & 0x38);
				*pDstBlue = (*pucSrc++ << 3) & 0xf8;
				pDstRed   += lAddToPointers;
				pDstGreen += lAddToPointers;
				pDstBlue  += lAddToPointers;
				}
			break;

		case P565:
			// Source format (bits): rrrrrggg gggbbbbb
			// To avoid endian issues this was done by bytes instead of words!
			while (sCount--)
				{
				*pDstRed = *pucSrc & 0xf8;
				green = (*pucSrc++ << 5) & 0xe0;
				*pDstGreen = green | ((*pucSrc >> 3) & 0x1c);
				*pDstBlue = (*pucSrc++ << 3) & 0xf8;
				pDstRed   += lAddToPointers;
				pDstGreen += lAddToPointers;
				pDstBlue  += lAddToPointers;
				}
			break;
		
		case P888:
			// Source format: BGR888
			while (sCount--)
				{
				*pDstBlue  = *pucSrc++;
				pDstBlue  += lAddToPointers;
				*pDstGreen = *pucSrc++;
				pDstGreen += lAddToPointers;
				*pDstRed   = *pucSrc++;
				pDstRed   += lAddToPointers;
				}
			break;
			
		case PFLX:
			// Source format: RGB888
			while (sCount--)
				{
				*pDstRed   = *pucSrc++;
				pDstRed   += lAddToPointers;
				*pDstGreen = *pucSrc++;
				pDstGreen += lAddToPointers;
				*pDstBlue  = *pucSrc++;
				pDstBlue  += lAddToPointers;
				}
			break;
		
		default:
			TRACE("RPal::GetEntries - Not a registered palette type!\n");
			sResult = FAILURE;
			break;
		}
	
	return sResult;
	}


///////////////////////////////////////////////////////////////////////////////
//
// SettEntries
//
// Description:
//		Sets one or more entries to this palette.  This function offers a
// 	standardized method for accessing the palette data, regardless of its
// 	format or type.
//		
//		Separate pointers are used for each component (red, green, blue, and
//		alpha) so this function can access those components regardless of what
//		order they're stored in.  The lAddToPointers parameter specifies the
//		value to be added to the pointers so they point to the next set of values.
//
//		All palette formats ought to be supported, but if someone gets lazy,
//		then this will fail for unsupported types.
//
//		At some point, if alpha info becomes more prevalent, an overloaded
//		version of this function should be created to supply alpha info along
//		with red/green/blue info.  This functionality was momentarily built into
// 	this function, but was quickly removed when it became obvious that it
//		would slow things down in cases where there was no alpha info, or more
//		importantly, when the user passed a NULL to indicate a lack of alpha data.
//
// Parameters:
//		(see function declaration below)
//
// Returns:
//		SUCCESS if the palette was loaded
//		FAILURE otherwise
//				  TRACE messages will help pinpoint the failure
//
///////////////////////////////////////////////////////////////////////////////
short RPal::SetEntries(
	short sStart,								// In:  Starting palette entry
	short sCount,								// In:  Number of entries to do
	unsigned char* pSrcRed,					// In:  Starting source red value
	unsigned char* pSrcGreen,				// In:  Starting source green value
	unsigned char* pSrcBlue,				// In:  Starting source blue value
	long lAddToPointers)						// In:  What to add to pointers to move to next value
	{
	// Validate parameters
	ASSERT(pSrcRed != NULL);
	ASSERT(pSrcGreen != NULL);
	ASSERT(pSrcBlue != NULL);
	if ((sStart < m_sStartIndex) || ((sStart + sCount - 1) > (m_sStartIndex + m_sNumEntries - 1)))
		{
		TRACE("RPal::PutEntries(): Specified range (%d to %d) exceeds palette's range (%d to %d)!\n",
			sStart, m_sStartIndex, m_sStartIndex + m_sNumEntries - 1);
		return FAILURE;
		}
	if (sCount < 0)
		{
		TRACE("RPal::PutEntries(): The count cannot be negative!\n");
		return FAILURE;
		}
		
	short sResult = SUCCESS;

	// Calculate pointer to first entry to be modified
	unsigned char* pucDst = m_pData + ((sStart - m_sStartIndex) * m_sPalEntrySize);

	switch(m_type)
		{
		case NO_PALETTE:
			TRACE("RPal::GetEntries - No palette!\n");
			sResult = FAILURE;
			break;
			
		case PDIB:
			// Destination format: BGR888+reserved
			while (sCount--)
				{
				*pucDst++  = *pSrcBlue;
				pSrcBlue  += lAddToPointers;
				*pucDst++  = *pSrcGreen;
				pSrcGreen += lAddToPointers;
				*pucDst++  = *pSrcRed;
				pSrcRed   += lAddToPointers;
				pucDst++;
				}
			break;

		case PSYS:
			TRACE("RPal::GetEntries - Does not support PSYS (system-depedendant palette)!\n");
			sResult = FAILURE;
			break;
			
		case P555:
			// Destination format (bits): 0rrrrrgg gggbbbbb
			// To avoid endian issues this was done by bytes instead of words!
			while (sCount--)
				{
				*pucDst++ = ((*pSrcRed >> 1) & 0x7c) | ((*pSrcGreen >> 6) & 0x03);
				*pucDst++ = ((*pSrcGreen << 2) & 0xe0) | ((*pSrcBlue >> 3) & 0x1f);
				pSrcRed   += lAddToPointers;
				pSrcGreen += lAddToPointers;
				pSrcBlue  += lAddToPointers;
				}
			break;

		case P565:
			// Destination format (bits): rrrrrggg gggbbbbb
			// To avoid endian issues this was done by bytes instead of words!
			while (sCount--)
				{
				*pucDst++ = (*pSrcRed & 0xf8) | ((*pSrcGreen >> 5) & 0x07);
				*pucDst++ = ((*pSrcGreen << 3) & 0xe0) | ((*pSrcBlue >> 3) & 0x1f);
				pSrcRed   += lAddToPointers;
				pSrcGreen += lAddToPointers;
				pSrcBlue  += lAddToPointers;
				}
			break;
		
		case P888:
			// Destination format: BGR888
			while (sCount--)
				{
				*pucDst++  = *pSrcBlue;
				pSrcBlue  += lAddToPointers;
				*pucDst++  = *pSrcGreen;
				pSrcGreen += lAddToPointers;
				*pucDst++  = *pSrcRed;
				pSrcRed   += lAddToPointers;
				}
			break;
			
		case PFLX:
			// Destination format: RGB888
			while (sCount--)
				{
				*pucDst++  = *pSrcRed;
				pSrcRed   += lAddToPointers;
				*pucDst++  = *pSrcGreen;
				pSrcGreen += lAddToPointers;
				*pucDst++  = *pSrcBlue;
				pSrcBlue  += lAddToPointers;
				}
			break;
		
		default:
			TRACE("RPal::GetEntries - Not a registered palette type!\n");
			sResult = FAILURE;
			break;
		}
	
	return sResult;
	}

///////////////////////////////////////////////////////////////////////////////
// Copy operator overload.
// Note that this could fail.
///////////////////////////////////////////////////////////////////////////////
RPal& RPal::operator=(RPal &palSrc)
	{
	// Copy members.
	m_type				= palSrc.m_type;
	m_ulSize				= palSrc.m_ulSize;
	m_sStartIndex		= palSrc.m_sStartIndex;
	m_sNumEntries		= palSrc.m_sNumEntries;
	m_sPalEntrySize	= palSrc.m_sPalEntrySize;
	// If there is any data . . .
	if (palSrc.m_ulSize > 0)
		{
		// Allocate space . . .
		if (CreateData(palSrc.m_ulSize) == 0)
			{
			// Copy actual data.
			memcpy(m_pData, palSrc.m_pData, palSrc.m_ulSize);
			}
		else
			{
			TRACE("operator=(): CreateData() failed.\n");
			}
		}

	return *this;
	}

///////////////////////////////////////////////////////////////////////////////
// EOF
///////////////////////////////////////////////////////////////////////////////
