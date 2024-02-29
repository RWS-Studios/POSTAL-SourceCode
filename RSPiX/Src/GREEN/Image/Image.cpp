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
//////////////////////////////////////////////////////////////////////
//
//	IMAGE.CPP
//   
//	Created on 		4/20/95	MR 
// Implemented on 4/21/95	JW
// Changes Added	4/25/95	PPL	Change from using halloc to malloc
//											since we are in Win32.
// 05/08/95	JMI	Took out CDib dependency and added internal method
//						for loading BMPs.
//
//	05/09/95	JMI	No longer converting from RGBQUAD type palette to
//						555 when loading DIBs since RGBQUAD is the DIB
//						palette.
//
//	05/10/95	JMI	CPal now contains the ability to convert between
//						an RGBQUAD (DIB-type (FCC_pdib)) and a 555
//						(FCC_p555).
//
//	05/10/95	JMI	Added member to store whether it's okay to free
//						the palette data (i.e., we allocated it in CPal).
//
//	05/11/95	JMI	Took out fstream stuff that was including Windows.h
//						causing this module to take weeks to compile.
//
//	09/26/95 BRH	Completely changed the CImage class to a new
//						version that is compatible with Jeff's Blit modules
//						and includes expandability for future image types.
//						This is the new standard image which each new image
//						type will be required to understand and be able to 
//						convert to/from.  This CImage will complete the missing
//						link between the app level image format and Jeff's
//						Blit library.  There is no CPalImage anymore, but the
//						more general CImage may have a valid CPal pointer or
//						NULL if there is no palette.
//
// 11/03/95 BRH	Made a few minor changes to the TRACE messages.
//						Implemented the CImage and CPal Load and Save 
//						functions which will be able to save any standard
//						image and palette.  Currently there are no image
//						types that it cannot save, but an image type that 
//						used a pSpecial to point to data containing other
//						pointers would not be saved correctly with the
//						standard CImage::Load.  If you have a special case
//						like that, you can call write your own save that calls
//						CImage::Load to save the standard part of the image 
//						and then append your own special data at the end.
//
//	11/15/95	BRH	Added CPal::DetachData() so that the pData can be
//						detached from the palette.  This makes the new
//						way of converting palettes easier.  Previously when
//						a CImage converted a palette it created a whole new
//						CPal object, but now it instead keeps the original
//						CPal object and just changes the palette's pData.
//						See note in imagecon.cpp history for more information.
//
// 11/20/95 BRH	Added CPal::Convert() so that palettes can be converted
//						using the existing CImage::Convert routines.  The Pal
//						conversion creates a temporary palettized image and
//						runs the conversion.  Also added CImage::SetPalette()
//						and a mechanism by which a CImage can keep track of
//						palettes that it allocated and those that were set
//						by the user.  Also changed pMem from public to private
//						and added a GetMemory access function to effectively 
//						make the memory pointer read only.
//
//	01/16/96	JMI	Altered Convert to utilize dynamically linked convertors.
//						Updated aPalEntrySizes.
//
//	01/16/96 BRH	Added pSpecialMem to solve a problem that came up when
//						images that included pSpecial data were loaded from a file.
//						Before the user was always responsible for freeing the 
//						pSpecial data but in this case, if a user simply loads
//						a specific image type that happened to have pSpecial data,
//						it would be a burden to them to have to remember to free
//						the special data before the desturctor was called for the image.
//						so the destructor will take care of pSpecial if it was the one
//						that created it (as a result of loading from a file).
//
//						Also added GetPitch(width, depth) to provide an easy calculation
//						of 128-bit aligned pitches.  Some of the conversion functions
//						were not recalculating the pitch of the new image type so this
//						function call will be added to the conversion functions.
//
// 01/23/96 BRH	Adding Operator= function to copy images and palettes which
//						will be used in the new SaveDIB function.  SaveDIB is the
//						counterpart to LoadDIB.  This will allow us to export images
//						back into a common .BMP file for further editing with standard
//						tools.  SaveDIB will use the code from DIB's save and will
//						help CImage start to take the place of DIB in any RSPIX programs.	 
//
// 02/27/96 BRH	Added call to IMAGELINKINSTANTIATE macro to set up the
//						CImageSpecialFunc class's arrays of special functions
//						This version of CImage uses the CImageSpecialFunc class to
//						set up functions for special image types for conversion, 
//						load/save and alloc/delete.  The CImageSpecialFunc class
//						is based on the DYNALINK that was used in the last version
//						for ConvertTo and ConvertFrom functions.  Once we determined
//						that we also needed special version of load, save, alloc, 
//						and delete we decided to make a class similar to DYNALINK that
//						would take care of all of these special functions at once
//						rather than setting up 6 different DYNALINK arrays.
//
// 03/04/96 BRH	Added calls to the special functions.  The ConvertTo
//						and ConvertFrom have been changed from DYNALINK to
//						CImageSpecialFunc but the macros to call them remained
//						the same.  Macro calls to get the function pointers for
//						special image save and load were added to the Image's
//						Save and Load functions.  First the standard image
//						information is written and then the special save for that
//						image type is called if it exists to write the special
//						data to the open CNFile.  I also added calls to special
//						allocation functions in CreateData and a call to the special
//						delete function in the destructor.
//
// 03/18/96	JMI	Added SaveDib() ripped nearly verbatim from DIB.CPP.
//
// 04/30/96	BRH	Added some new fields to the header of the CImage, thus
//						invalidating the previous .IMG files but at this point they
//						weren't in wide use.  The Blit routines required a few
//						additional pieces of information so that they could avoid
//						recreating the buffer as they needed it.  I added 
//						lBufferWidth, lBufferHeight, lXPos, lYPos to accomodate 
//						the Blit functions.  This way a buffer that is larger
//						than the image can be created and the image can be
//						located at a specific position in that buffer. Note that
//						the whole buffer is not saved, just the image but the
//						size of the buffer is saved so that upon load, the buffer
//						can be recreated at its desired size.  
//						I also added ulDestinationType to accomodate image formats
//						that may be saved as one type but are converted to another
//						type once they are loaded.  For example, if you are using
//						sprites and there is no save funciton for sprites, you can
//						save the file with BMP8 type and a destination type of
//						FAST_SPRITE so that when the file is loaded, it is
//						read as a BMP8 format but automatically converted to
//						the fast sprite type.  Then to avoid debugging problems
//						when the CImage header is changed, I added a version number
//						as a #define and write that to the file.  Then when a .IMG file
//						is loaded, the version number is checked with the current
//						version of CImage and if they are different it will return
//						an error.
//
// 05/16/96 BRH	Changed from using malloc to calloc so that image buffers
//						will be cleared to zero before being used.
//
//	07/12/96	JMI	Changed CreateImage so it updates the lPitch member once
//						calculated (if calculated (may be passed in)).
//
// 08/02/96 BRH	Fixed Load so it is in sync with Save.  The Save function
//						had been updated as described above in 4/30/96 comment with the
//						new fields added, but the Load function did not yet read those
//						fields.
//
// 08/04/95 MJR	Added function prototypes for sCreateMem(), sCreateAlignedMem(),
//						and sDestroyMem(), and also made them static, thereby brining
//						them into compliance with ANSI Section 5.75.53 Subsection 5a
//						and Section 8.27 Subsection 2b.  More importantly, it avoided
//						CodeWarrior compiler warnings.
//
//	09/10/96	JMI	Modified LoadDib() to be able to calculate proper pitches for
//						bit depths that don't have 8 as a factor.  Same for 
//						SaveDib().  LoadDib() was not reporting an error for 16 bit 
//						BMPs even though it was setting the ulType field to 
//						NOT_SUPPORTED.  Also, SaveDib() was improperly setting the 
//						size field of the DIB file header to the lPitch * lHeight 
//						when it should have been the lDibPitch * lHeight.  Also, 
//						WIDTHUCHAR and WIDTH128 macros were not 'order-of-operations'
//						safe macros.  Added parenthesis surrounding arguments for 
//						that extra sense of comfort we've come to know and love.  We
//						deserve that kind of protection.
//
//	10/09/96	JMI	Load() was converting to ulDestinationType even
//						if it was NOT_SUPPORTED.  ReadPixelData() was using
//						a comparison that was not compatable with the one
//						used in WritePixelData() for the same data set.
//						Fixed.
//
//	10/10/96	JMI	CreateImage() was computing lPitch wrong.  Fixed.
//
//	10/15/96 MJR	Added GetEntries() to CPal as a standardized method of
//						accessing the palette color info regardless of format or type.
//
//	10/16/96	JMI	Minor fix in CPal::GetEntries() to ASSERTs on undefined vars.
//
//	10/18/96 MJR	Added SetEntries() to CPal as a standardized method of
//						accessing the palette color info regardless of format or type.
//						Added CreatePalette() to CPal as a way to create a palette
//						without having to know much (if anything) about it's format.
//						Fixed GetEntries() to properly handle 555 and 565 palettes.
//						Fixed GetEntries() to properly deal with palette's whose starting
//						index is not 0 (very rare, but a bug nonetheless).
//
//	10/21/96 BRH	Changed all loads and saves so that optional data
//						does not need to be present to save an Image or
//						palette.  Now you can save a palette with no
//						color data, just the header if you want.  Each
//						optional section is flaged in the file.  Also, I
//						modified the load and save routines to check
//						only for critical errors like wrong cookie or
//						wrong version.  Then the error codee is checked
//						after all of the reads or writes have been done
//						before it reports an error.  This cut out most of
//						the if/else pyramid code and made it much easier 
//						to read.  
//
//	10/24/96 BRH	Changed WritePixelData and ReadPixelData functions
//						to read and write their data according to the bit
//						depth rather than treating it as a block of bytes.
//						This was done so that the data will be properly
//						byte swapped when transfering it between the Mac and PC.
//						For example, 16 bit image formats are now written 
//						to the CNFile as a number of USHORTs rather than
//						twice as many UCHARs.  This same functionality
//						needs to be added to load and save for DIBs which
//						will be in the next version.
//
//	10/30/96	JMI	Pulled CPal stuff out of here and put it into pal.cpp.
//						Pulled astrImageTypeNames out of imagetyp.h and put it
//						here as ms_astrTypeNames static member.
//						Attempted to reword comment summary to reflect these
//						changes.
//						Note: I'm not positive but I think the references to
//						imageafp.h are out of date.
//
//	10/30/96	JMI	Changed:
//						Old label:		New label:
//						=========		=========
//						CNFile			RFile
//						CImage			RImage
//						CPal				RPal
//						ULONG ulType	RImage::Type ulType
//
//						The thing that annoys me the most about using actual enums
//						instead of ULONGs is that you have to copy it into a dummy
//						ULONG to use RFile on it.  This isn't very bad, but it's
//						annoying.
//
//	10/31/96	JMI	Changed all members to be preceded by m_ (e.g., sDepth
//						m_sDepth).  Changed all position members (i.e., lWidth,
//						lHeight, lBufferWidth, lBufferHeight, lXPos, & lYPos) to
//						be shorts (i.e., m_sWidth, m_sHeight, m_sBufferWidth,
//						m_sBufferHeight, m_sXPos, m_sYPos) and functions associated
//						with these members reflect this change (e.g., long GetWidth()
//						is now short GetWidth()).  Changed ulType to m_type and 
//						ulDestinationType to m_typeDestination.  Increased file
//						version to 5 since members converted to short will affect
//						the file format.
//
//	11/22/96	JMI	DestroyData() now destroys m_pSpecialMem as well.
//
//	11/25/96	JMI	Because DestroyData() had explicit returns other than the
//						one at the end, the fix above was rarely being processed.
//						Fixed.  Now DestroyData() contains only one return.
//
//	11/26/96	JMI	Now DestroyData() sets pSpecialMem to NULL.
//
//	11/27/96 BRH	Changed names of the buffer and image width around
//						to make the usage more clear and to make sure that
//						code using the previous method would not quit working
//						due to this change.  Previously the widht and height
//						referred to the visible picture which may be stored
//						in a larger buffer.  The BufferWidth and BufferHeight
//						referred to the size of the memory in which the picture
//						was being stored.  Now we will change the width and height
//						to mean the entire memory area and the size of the picture
//						will be referred to by WinWidth and WinHeight.  I am 
//						rearranging the order of the variables in the class so
//						that I won't have to change the version of the file
//						format and I will also change the order that these values
//						are being written and read from the files so that old
//						image files will continue to work.
//
//	12/04/96	JMI	Added patch to make old .IMG files that had m_sBufferWidth
//						and m_sBufferHeight set to 0.  For a more detailed
//						explanation, search for "Begin load patch".
//
//	12/11/96	JMI	Now calls RImageFile::Load(...) to load images.
//						RImageFile::Load(...) has the advantage of potentially
//						supporting older formats.  This required that extended
//						support functions for load and save be aware of the file
//						version number, so LOADFUNC and SAVEFUNC now take a
//						ulVersion.
//
//	12/11/96	JMI	Changed LOADFUNC and SAVEFUNC back to NOT taking a ulVersion
//						because the BLiT libs already use these extensions without
//						the ulVersion.  This change is temporary until such a change
//						can be made to BLiT.
//
//	12/18/96 BRH	Added overloaded versions of LoadDib and SaveDib that
//						take an RFile* and can load or save to that rather than
//						just the filename versions.  This was first used
//						in the resource manager but is also otherwise
//						generally useful.
//
//	12/21/96	JMI	LoadDib() is now able to load itself from a larger file.
//						It was using a Seek() to a non-relative file address.  Now
//						it simply adds the value of an initial Tell() to the address
//						to Seek().
//
//	02/04/97	JMI	Added BMP_COOKIE for in detecting a .BMP formatted file.
//						Made LoadDib() a private member.  Now Load() will load *.BMP
//						or *.IMG formatted files.
//
//	04/16/97	JMI	Ammendded operator= overload.
//
// 05/21/97 JRD	Fixed a bug in the default pitch generation of CreateImage.
//
//	06/08/97 MJR	Removed unecessary (some would say incorrect) TRACE messages
//						and associated error returns from DestroyData() and
//						DestroyPalette().  They were complaining if you called
//						called them when the data had been allocated by the user.
//						Instead, they now silently deal with it.
//
//	Image.h contains the class RImage.  Pal.h contains RPal.  The RImage is used
// to store generic images either with or without palettes.  RPal can also be 
// used alone to store palettes.
//
// The image format sets a standard image format as a middle ground.
// If you wish to create a different image format, such as compressed
// images, then you would supply a convert function that would be able
// to convert from this standard RImage format to your compressed 
// format and a conversion function to convert from your compressed
// format to this standard format.  You can choose to change the pData
// pointer to point to the data in your new format, or you could
// choose to keep the standard form in pData and allocate memory for 
// your compressed version and use pSpecial to keep track of it.
//
// If you create a new image format, you should register your
// enumerated image type in image.h in the eImageTypes enum and add 
// your Convert() function pointer to the array of convert functions
// in imageafp.h.  The array of convert function pointers is indexed
// by the image type.
// Then you may add your conversion function either in your own
// file or you could add its prototype to imagecon.h and the
// code to imagecon.cpp.
//
//////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h> // For malloc, etc.
#include <string.h> // For memcpy...

#include "Blue.h"

#ifdef PATHS_IN_INCLUDES
	#include "GREEN/Image/Image.h"
	#include "GREEN/Image/ImageFile.h"
	#include "ORANGE/File/file.h"
#else
	#include "Image.h"
	#include "ImageFile.h"
	#include "FILE.H"
#endif // PATHS_IN_INCLUDES


// Local function prototypes
static short sCreateMem(void **hMem,ULONG ulSize);
static short sCreateAlignedMem(void **hMem, void **hData, ULONG ulSize);
static short sDestroyMem(void **hMem);


//////////////////////////////////////////////////////////////////////
// Instantiate class statics.
//////////////////////////////////////////////////////////////////////

// This array of type names should correspond to the list of
// enumerated types eImageTypes in image.h.  Whenever you add an image
// type and an enum, you need to also insert that name into the 
// corresponding place in this array.  
// Note that this uses END_OF_TYPES enum item to size the array.

char* RImage::ms_astrTypeNames[END_OF_TYPES] = 
{
	"Same Type", 
	"BMP8", 
	"SYSTEM8", 
	"SCREEN8_555",
	"SCREEN8_565",
	"SCREEN8_888",
	"BMP24",
	"SCREEN16_555",
	"SCREEN16_565",
	"SCREEN24_RGB",
	"SCREEN32_ARGB",
	"FSPR1",
	"FSPR8",
	"FSPR16",
	"FSPR32",
	"ROTBUF",
	"SPECIAL",
	"FLX8_888",
	"IMAGE_STUB",
	"BMP8RLE",
	"BMP1",				// Added 09/04/96	JMI.
};

//////////////////////////////////////////////////////////////////////
// Instantiate Dynamic Arrays
//////////////////////////////////////////////////////////////////////

short	ConvertNoSupport(RImage* pImage);

IMAGELINKINSTANTIATE();

IMAGELINKLATE(NOT_SUPPORTED, ConvertNoSupport, NULL, NULL, NULL, NULL, NULL);
 
//////////////////////////////////////////////////////////////////////
//
//	sCreateMem
// (static)
//
//	Description:	
//		To allocate memory for the data buffers of RPal
//
//	Parameters:	
//		hMem		handle used for the buffer
//		ulSize	size of buffer to allocate in bytes
//
//	Returns:    
//		0		Success
//		-1		Buffer has already been allocated
//		-2		Buffer could not be allocated
//
//////////////////////////////////////////////////////////////////////

short RImage::sCreateMem(void **hMem,ULONG ulSize)
{
	//	Make sure the data
	//	hasn't already been allocated
	if (*hMem != NULL)
	{              
		TRACE("RPal::AllocMem() called by CreateData() -- A buffer has already been allocated\n");
		// Image allocated already
		return ((short)-1);
	}
	else
	{         
		if (ulSize > 0)
		{               
			if ((*hMem = calloc(ulSize, 1)) == NULL)
			{
				TRACE("RPal::AllocMem() called by CreateData() -- The buffer could not be allocated\n");
				// Image buffer couldn't be allocated
				return ((short)-2);
			} 
			else
			{        
				// Success
				return ((short)0);
			}
		}
		else
		{
		 	TRACE("RPal::AllocMem() called by CreateData() - Warning attempting to allocate 0 bytes, quit screwing around\n");
			*hMem = NULL;
			return 0;
		}
	}
}

//////////////////////////////////////////////////////////////////////
//
//	sCreateAlignedMem
// (static)
//
//	Description:	
//		To allocate memory and return a pointer aligned to 128-bits
//		for optimum blit speed.  This is the function used by
//		RImage when it creates memory for the image buffers.
//
//	Parameters:	
//		hMem		handle used for deallocating memory later
//		hData		handle for the aligned image buffer
//		ulSize	size of buffer to allocate in bytes
//
//	Returns:    
//		0		Success
//		-1		Buffer has already been allocated
//		-2		Buffer could not be allocated
//
//////////////////////////////////////////////////////////////////////

short RImage::sCreateAlignedMem(void **hMem, void **hData, ULONG ulSize)
{
 	// Make sure the data hasn't already been allocated
	if (*hMem != NULL)
	{
	 	TRACE("RImage::AllocMem called by CreateData() - buffer has already been allocatd\n");
		// buffer already exists
		return FAILURE;
	}
	else
	{
		if (ulSize > 0)
		{
			// allocate an extra 15 bytes so that the data ponter can be aligned
			// to the nearest 128-bit boundry for Blit speed reasons
			if ((*hMem = calloc(ulSize + 15, 1)) == NULL)
			{
			 	TRACE("RImage::AllocMem() called by CreateData() - buffer could not be allocated\n");
				// calloc failed
				return FAILURE;
			}
			else
			{
				// Set Data buffer to 128-bit alignment
				*hData = (void*) (((unsigned long) *hMem + 0x0f) & 0xfffffff0);
				// success		 	
				return SUCCESS;
			}
		}
		else
		{
		 	TRACE("RImage::AllocMem() called by CreateData() - Warning attempted to create a buffer of 0 bytes, quit screwing around\n");
			*hMem = NULL;
			return SUCCESS;
		}
	}
}

//////////////////////////////////////////////////////////////////////
//
//	sDestroyMem
// (static)
//
//	Description:
//		To free the data buffers of RPal and RImage that were created 
//		using either sCreateMem() or sCreateAlignedMem()
//
//	Parameters:	
//		hMem		handle to the memory used by the buffer
//
//	Returns:    
//		0		Success
//
//////////////////////////////////////////////////////////////////////

short RImage::sDestroyMem(void **hMem)
{   
	// Make sure the memory
	// hasn't already been freed    
	if (*hMem != NULL)
	{
		free(*hMem);
		*hMem = NULL;
	}

	// Always return success because 
	// the memory has been freed
	return SUCCESS;
}


//////////////////////////////////////////////////////////////////////
//////////////////////////// RImage //////////////////////////////////
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//
//	RImage Member Functions
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//
// Init
//
// Sets up the array of conversion handlers for this class
//
//////////////////////////////////////////////////////////////////////

void RImage::Init()
{
 	
}

//////////////////////////////////////////////////////////////////////
//
// RImage::DestroyDetachedData
//
// Description:
//		static member function that deallocates memory that was
//		originally created using RImage::CreateData() and then
//		detached from the RImage using DetachData().  This function
//		will use the correct method to free the memory.
//
//	Parameters:
//		hMem	handle to the memory to be freed
//
// Returns:
//		SUCCESS if the memory was successfully freed
//		FAILURE if either the handle or the pointer to memory was
//				  NULL already
//
//////////////////////////////////////////////////////////////////////

short RImage::DestroyDetachedData(void** hMem)
{
	if (hMem)
		if (*hMem)
			return sDestroyMem(hMem);
		else
			TRACE("Image::DestroyDetachedData - Attempted to free a NULL pointer.\n");
	else
		TRACE("Image::DestroyDetachedData - Attempted to free a NULL handle.\n");

	return FAILURE;

}

//////////////////////////////////////////////////////////////////////
//
// Constructor
//
// Description:
//		Default constructor for the RImage class.  Initializes the
//		member variables but does not create or load any data.
//
// Parameters:
//		none
//
// Returns:
//		none
//
//////////////////////////////////////////////////////////////////////

RImage::RImage()
{            
	// Initialize member variables to zero
	InitMembers();
}	

//////////////////////////////////////////////////////////////////////
//
// Constructor
//
// Description:
//		Initializes class members and creates data of the given size
//
// Parameters:
//		ulNewSize	size of data to be created
//
//	Returns:
//		none
//
//////////////////////////////////////////////////////////////////////

RImage::RImage(ULONG ulNewSize)
{
	// Initialize member variables to zero
	InitMembers();

	CreateData(ulNewSize);
}

//////////////////////////////////////////////////////////////////////
//
// Constructor
//
// Description:
//		Initializes member variables and attempts to load the given
//		BMP from a file.
//
// Parameters:
//		pszFilename	Filename to load
//
// Returns:
//		none
//
//////////////////////////////////////////////////////////////////////

RImage::RImage(char* pszFilename)
{
	// Initialize member variables to zero
	InitMembers();

	LoadDib(pszFilename);
}

//////////////////////////////////////////////////////////////////////
//
// Destructor
//
// Description:
//		Deallocates buffer memory, and palette if any
//
// Parameters:
//		none
//
// Returns:
//		none
//
//////////////////////////////////////////////////////////////////////

RImage::~RImage()
{
	// Free Image data
	// if (m_pMem) // WILL NEVER ALLOW DeleteSpecial
	DestroyData();

	if (m_pPalMem)
		delete(m_pPalMem);
}

//////////////////////////////////////////////////////////////////////
//
// RImage::Init
//
// Description:
//		Initialize all members.  Calling this when m_pMem* is set is 
//		not a good idea.
//
// Parameters:
//		None.
//
//	Affects:
//		All members.
//
// Returns:
//		Nothing.
//
//////////////////////////////////////////////////////////////////////

void RImage::InitMembers(void)
{
	// Initialize member variables to zero
	m_type				= NOT_SUPPORTED;
	m_typeDestination	= NOT_SUPPORTED;
	m_ulSize				= 0;
	m_sWidth				= 0;
	m_sHeight			= 0;
	m_sWinWidth			= 0;
	m_sWinHeight		= 0;
	m_sWinX				= 0;
	m_sWinY				= 0;
	m_lPitch				= 0;
	m_sDepth				= 0;
	m_pMem				= NULL;
	m_pData				= NULL;
	m_pPalette			= NULL;
	m_pPalMem			= NULL;
	m_pSpecial			= NULL;
	m_pSpecialMem		= NULL;
}

//////////////////////////////////////////////////////////////////////
//
// RImage::CreateData
//
// Description:
//		Creates an 128-bit aligned buffer for the image data
//
// Parameters:
//		ulNewSize	Size in bytes of the buffer
//
// Returns:
//		SUCCESS if the memory was alocated successfully 
//		FAILURE if memory could not be allocted
//
//////////////////////////////////////////////////////////////////////

short	RImage::CreateData(ULONG ulNewSize)
{
	if (m_pMem)
	{
		TRACE("RImage::CreateData - Attempting to create data when pMem is still pointing to memory\n");
	 	return FAILURE;
	}

	if (m_pData && !m_pMem)
		TRACE("RImage::CreateData - Warning: pData is pointing to data\n");

	ALLOCFUNC caf = GETALLOCFUNC(m_type);
	if (caf != NULL)
		if ((*caf)(this) != SUCCESS)
			TRACE("RImage::CreateData - Error creating data for special type %d\n", m_type);

	m_ulSize = ulNewSize;

	return sCreateAlignedMem((void**) &m_pMem, (void**) &m_pData, ulNewSize);
}

//////////////////////////////////////////////////////////////////////
//
// RImage::CreateData
//
// Description:
//		Create IMAGE's data utilizing passed in fields.
//		Calls CreateData(ULONG) to do the allocation.
//
// Parameters:
//		As described below.
//
// Returns:
//		Return value from CreateData(ULONG).
//		SUCCESS if the memory was alocated successfully 
//		FAILURE if memory could not be allocted
//
//////////////////////////////////////////////////////////////////////

short RImage::CreateImage(		// Returns 0 if successful.
	short	sWidth,					// Width of new buffer.
	short	sHeight,					// Height of new buffer.
	Type	type,						// Type of new buffer.
	long	lPitch	/*= 0L*/,	// Pitch of new buffer or -1 to calculate.
	short	sDepth	/*= 8*/)		// Color depth of new buffer.
	{
	short	sRes	= SUCCESS;	// Assume success.

	// Fill in fields.
	m_sWidth = m_sWinWidth	 = sWidth;
	m_sHeight = m_sWinHeight = sHeight;
	m_type						 = type;
	m_sDepth						 = sDepth;
	m_sWinX = m_sWinY			 = 0;
	// If no pitch specified . . .
	if (lPitch == 0L)
		{
		lPitch	= GetPitch(sWidth, sDepth);
		}

	// Update member lPitch.
	m_lPitch			= lPitch;
	m_ulSize			= lPitch * (long)sHeight;
	if (m_ulSize > 0)
		{
		sRes	= CreateData(m_ulSize);
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////
//
// RImage::DetachData
//
// Description:
//		This function detaches the image buffer from the RImage
//		and returns the pointer to the original buffer to you.  
//		You can then call RImage::CreateData again to create a new
//		buffer.  You are responsible for keeping track of the detached
//		buffer and deallocating its memory when you are finished with it.
//		You should deallocate the memory by calling
//		RImage::DestroyDetachedData() so it will be freed using a 
//		deallocation function that is compatible with the way the
//		data was allocated.  
//
//		This function is useful when doing image conversion where you 
//		want to create a new buffer for the new data and get rid of
//		the old buffer when you are done with the conversion.  This
//		version of DetachData returns pointer to the memory buffer,
//		and not the pointer to the data (the aligned pointer).  If you
//		need to use the data for a conversion for example, then you
//		should first save a pointer to the data pImage->pData before
//		calling DetachData, or you can call the other version of
//		DetachData where you supply a handle to the memory and to
//		the data.
//
//	Parameters:
//		none
//
// Returns:
//		RImage.pMem, the pointer to the allocated memory
//
//////////////////////////////////////////////////////////////////////

void* RImage::DetachData(void)
{
 	void* pDetachment = m_pMem;
	m_pMem = m_pData = NULL;
	return pDetachment;
}

//////////////////////////////////////////////////////////////////////
//
// RImage::DetachData
//
// Description:
//		This function detaches the image buffer from the RImage
//		and returns the pointer to the original buffer to you.  
//		You can then call RImage::CreateData again to create a new
//		buffer.  You are responsible for keeping track of the detached
//		buffer and deallocating its memory when you are finished with it.
//		You should deallocate the memory by calling
//		RImage::DestroyDetachedData() so it will be freed using a 
//		deallocation function that is compatible with the way the
//		data was allocated.  
//
//		This function is useful when doing image conversion where you 
//		want to create a new buffer for the new data and get rid of
//		the old buffer when you are done with the conversion.  This
//		version of DetachData takes two handles to memory and 
//		gives hMem the pointer to the allocated memory and hData
//		the pointer to the aligned memory where the data begins.
//		Alternatively you could save a pointer to the image data
//		RImage.pData and then call DetachData() which returns 
//		a pointer to the memory buffer.
//
//	Parameters:
//		none
//
// Returns:
//		FAILURE if either handle passed in was NULL
//		SUCCESS otherwise
//
//////////////////////////////////////////////////////////////////////

short RImage::DetachData(void** hMem, void** hData)
{
	if (hMem && hData)
	{
	 	*hMem = m_pMem;
		*hData = m_pData;
		m_pMem = m_pData = NULL;
		return SUCCESS;
	} 	
	else
		return FAILURE;
}

//////////////////////////////////////////////////////////////////////
//
// RImage::DestroyData
//
// Description:
//		Deallocated memory created by CreateData()
//
// Parameters:
//		none
//
// Returns:
//		FAILURE if DestoryData is called with data that was set by
//		        the user using SetData().  
//		SUCCESS after freeing the data created by CreateData()
//
//////////////////////////////////////////////////////////////////////

short	RImage::DestroyData()
	{   
	short	sRes	= 0;	// Assume success.
	
	// Only if the data was not supplied by the user.
	if (m_pMem)
		{
		m_pData = NULL;
		sRes	= sDestroyMem((void**) &m_pMem);
		m_pMem = NULL;
		}
	
	if (m_pSpecialMem)
		{
		// If there is a special delete function for this image type
		// call it so that it can clean up its pSpecial Memory
		DELETEFUNC cdf = GETDELETEFUNC(this->m_type);
		if (cdf != NULL)
			{
			if ((*cdf)(this) == SUCCESS)
				{
				m_pSpecialMem	= NULL;
				}
			}
		else
			{
			// Else do the best you can
			free(m_pSpecialMem);
			
			m_pSpecialMem	= NULL;
			}
		}
	
	return sRes;
	}

//////////////////////////////////////////////////////////////////////
//
// RImage::SetData
//
// Description:
//		Allows the user to set the image data pointer to a buffer
//		that was created by the user.  The user will be responsible
//		for deallocating this buffer, they cannot use DestroyData().
//
// Parameters:
//		pUserData = pointer to an image buffer
//
// Returns:
//		SUCCESS if the memory was set
//		FAILURE if the image already had a buffer
//
//////////////////////////////////////////////////////////////////////

short RImage::SetData(void* pUserData)
{
	if (m_pMem)
	{
		TRACE("Image::SetData - Attempted to set your own data pointer while there was memory allocated\n");
		return FAILURE;
	}
	else
	{
		m_pData = (UCHAR*) pUserData;
		return SUCCESS;
	}
}

//////////////////////////////////////////////////////////////////////
//
// SetPalette
//
// Description:
//		Sets a user palette to the image's pPalette pointer.  This is
//		the safe way of setting your own palette rather than just
//		setting pImage->pPalette to your palette.  This will warn you
//		if the image is already pointing to a palette allocated by
//		the image by a function like CreatePalette().
//
// Parameters:
//		pPal = pointer to your palette
//
// Returns:
//		SUCCESS - in all cases for now anyway, since it will always
//					 set the palette even if it has to delete the old one
//
//////////////////////////////////////////////////////////////////////

short RImage::SetPalette(RPal* pPal)
{
	if (m_pPalMem)
	{
		TRACE("RImage::SetPalette - Warning: m_pPalette points to an Image-allocated palette\n");
		TRACE("                     The previous palette will be deleted and your palette will be set\n");
		delete(m_pPalMem);
		m_pPalMem = NULL;
	}
	m_pPalette = pPal;	
	return SUCCESS;
}

//////////////////////////////////////////////////////////////////////
//
// RImage::CreatePalette
//
// Description:
//		Creates a RPal object but doesn't allocate any palette buffer
//
// Parameters:
//		none
//
// Returns:
//		SUCCESS if the palette was created
//		FAILURE if memory could not be allocated for the palette
//
//////////////////////////////////////////////////////////////////////

short RImage::CreatePalette(void)
{
	m_pPalMem = m_pPalette = new RPal();
	if (m_pPalette == NULL)
		return FAILURE;
	else
		return SUCCESS;
}

//////////////////////////////////////////////////////////////////////
//
// RImage::CreatePalette
//
// Description:
//		Creates a RPal object and allocates a pallete of the given
//		size
//
// Parameters:
//		ulSize = size in bytes of the palette
//
// Returns:
//		SUCCESS if the palette was created
//		FAILURE if memory could not be allocated for the palette
//
//////////////////////////////////////////////////////////////////////

short RImage::CreatePalette(ULONG ulSize)
{
	if (CreatePalette() == SUCCESS)
		return m_pPalette->CreateData(ulSize);
	else
		return FAILURE;
}

//////////////////////////////////////////////////////////////////////
//
// RImage::DetachPalette
//
// Description:
//		Removes the RPal from the image and returns the pointer to you.
//		Now you can call CreatePalette again for the image to create
//		a new palette and you are responsible for the palette returned
// 	to you.  You should use delete to deallocate the palette when
//		you are done with it.  This function is useful when doing
//		palette conversions on an image where you want to create a
//		new palette for the image and only want to use the old one
//		during the conversion and then delete it when you are done.
//
// Parameters:
//		none
//
// Returns:
//		RPal* = pointer to the RPal object
//		NULL if there is no palette for this image
//
//////////////////////////////////////////////////////////////////////

RPal* RImage::DetachPalette(void)
{
	RPal* pDetachment = m_pPalette;
	m_pPalette = m_pPalMem = NULL;
	return pDetachment;	
}

//////////////////////////////////////////////////////////////////////
//
// RImage::DestroyPalette
//
// Description:
//		Deallocates the palette for the image that was created using
//		CreatePalette()
//
// Parameters:
//		none
//
//	Returns:
//		SUCCESS after the palette is deallocated
//		FAILURE if the palette could not be destroyed either because
//			     there was no palette or it was a user set palette
//
//////////////////////////////////////////////////////////////////////

short RImage::DestroyPalette(void)
	{
	if (m_pPalMem)
		{
		delete m_pPalMem;
		m_pPalMem = m_pPalette = NULL;
		}
	return SUCCESS;
	}

//////////////////////////////////////////////////////////////////////
// Equals overload.
// Note that this function could fail.
//////////////////////////////////////////////////////////////////////
RImage& RImage::operator=(const RImage& imSrc)
	{
	// Easiest most likely to succeed way to get an image copy is to
	// write it to a file.
	RFile	file;
	// We know we'll probably need at least m_ulSize bytes so that's a 
	// good start for the size of the mem file.
	// Allow it to grow byte let's say 1K at a time for reasonable
	// efficiency vs memory wastage.
	if (file.Open(imSrc.m_ulSize, 1024, RFile::LittleEndian) == 0)
		{
		// Save the source into the mem file . . .
		if (imSrc.Save(&file) == 0)
			{
			// Go back to beginning.
			file.Seek(0, SEEK_SET);

			// Load the mem file into the dest (this) . . .
			if (Load(&file) == 0)
				{
				// Successful copy!
				}
			else
				{
				TRACE("operator=(): Load() from mem file failed.\n");
				}
			}
		else
			{
			TRACE("operator=(): imSrc.Save() to mem file failed.\n");
			}

		file.Close();
		}
	else
		{
		TRACE("operator=(): file.Open() failed.\n");
		}

	return *this;
	}

//////////////////////////////////////////////////////////////////////
//
// RImage::Convert
//
// Description:
//		This function calls one of the convert functions from the 
//		array ms_afp stored in the CDynaLink template.  The array of 
//		functions is indexed by the type of image that you wish to 
//		convert to.
//		Each convert function will evaluate the current type of the
//		image and if it supports converting from the current type
//		to the new type, it will do the conversion and return the
//		new type.  If it is not supported then it will return
//		NOT_SUPPORTED.
//
// Parameters:
//		ulType = one of the enumerated types in imagetyp.h.  This
//				   is the type you wish to convert to.
//
// Returns:
//		ulType if successful
//		NOT_SUPPORTED if the conversion cannot be done with the
//				        current image type, or if there is no
//						  convert function for the type you specified
//
//////////////////////////////////////////////////////////////////////

RImage::Type RImage::Convert(Type type)
{
	Type	typeRes	= NOT_SUPPORTED;	// Assume sux.

	// If out of range . . .
	if (type >= END_OF_TYPES)
		{
		// Not supported.
		}
	else
		{
		// If current format is extended . . .
		if (m_type > SCREEN32_ARGB)
			{
			// Verify function exists . . .
			CONVFROMFUNC	cft	= GETFROMFUNC(m_type);
			if (cft != NULL)
				{
				// Convert to a standard type.
				typeRes	= (Type)(*cft)(this);
				}
			else
				{
				TRACE("Convert(): Type exists, but no current link to convert to "
						"standard type.  Check for proper module.\n");
				typeRes	= NOT_SUPPORTED;
				}
			}

		// If current format is standard . . .
		if (	m_type	>= BMP8 
			&&	m_type	<= SCREEN32_ARGB)
			{
			// If current format is not the destination format . . .
			if (m_type	!= type)
				{
				// Verify function exists . . .
				CONVTOFUNC	ctt	= GETTOFUNC(type);
				if (ctt != NULL)
					{
					typeRes = (Type)(*ctt)(this);
					}
				else
					{
					TRACE("Convert(): Type exists, but no current link.  Check for "
							"proper module.\n");
					typeRes	= NOT_SUPPORTED;
					}
				}
			else
				{
				// Already in correct format (after standardization).
				typeRes	= type;
				}
			}
		else
			{
			TRACE("Convert(): Not in a standard format.\n");
			// Preserve current return value.
			}
		}

	return typeRes;
}

//////////////////////////////////////////////////////////////////////
//
// RImage::LoadDib
//
// Description
//		This function will allow the user of RPalImage to
//		read in a Window's Dib.
//
// Parameters:	
//		pszFileName = pointer to the filename of the DIB
//			or
//		pcf = pointer to open RFile where a DIB is stored
//
// Returns:
//		  0: 		(SUCCESS) if successful
//		-20:		Palette read error
//		-19:		Pixel data read error
//		-18:		Can't allocate memory for DIB
//		-17:		colors important field not read
//		-16:		colors used field not read
//		-15:		vert pixels per meter field not read
//		-14:		horz pixels per meter field not read
//		-13:		size of image field not read
//		-12:		compression field not read
//		-11:		bit count field not read
//		-10:		planes field not read
//		-9:		height field not read
//		-8:		width field not read
//		-7:		size field not read
//		-6:		offset to bits field not read
//		-5:		reserved2 field not read
//		-4:		reserved1 field not read
//		-3:		size of header field not read or not a BMP file
//		-2:		type field not read
//		-1:		unable to open file
///	1:			if a colordepth of 1 or 4 was read
//		2:			if an unknown colordepth has been encountered
//		3:			if the dib is compressed, can not handle yet
//
//////////////////////////////////////////////////////////////////////

short RImage::LoadDib(char* pszFilename)
{
	RFile cf;
	short sReturn = SUCCESS;

	if (cf.Open(pszFilename, "rb", RFile::LittleEndian) == SUCCESS)
	{
		sReturn = LoadDib(&cf);
		cf.Close();
	}
	else
	{
		TRACE("RImage::LoadDib - could not open Dib file %s - well shucks!\n", pszFilename);
	 	sReturn = FAILURE;
	}

	return sReturn;
}

short RImage::LoadDib(RFile* pcf)
{
	short sRes = 0; // Assume success.
	DIBHEADER		dh;
	DIBFILEHEADER	dfh;
	long 				lDibPitch;

	if (pcf && pcf->IsOpen())
	{
		// Get the beginning of the DIB portion of this file.
		// In some cases, this DIB file may be embedded in a larger
		// collection of files.
		long	lDibFileStartPos	= pcf->Tell();

		//  Read BITMAPFILEHEADER
		if (pcf->Read(&(dfh.usType), 1) == 1)
		{
			if (dfh.usType == BMP_COOKIE)
			{
				if (pcf->Read(&(dfh.ulSize), 1) == 1)
				{
					if (pcf->Read(&(dfh.usReserved1), 1) == 1)
					{
						if (pcf->Read(&(dfh.usReserved2), 1) == 1)
						{
							if (pcf->Read(&(dfh.ulOffBits), 1) == 1)
							{
								// Read BITMAPINFOHEADER
								if (pcf->Read(&(dh.ulSize), 1) == 1)
								{
									if (pcf->Read(&(dh.lWidth), 1) == 1)
									{
										if (pcf->Read(&(dh.lHeight), 1) == 1)
										{
											if (pcf->Read(&(dh.usPlanes), 1) == 1)
											{
												if (pcf->Read(&(dh.usBitCount), 1) == 1)
												{
													if (pcf->Read(&(dh.ulCompression), 1) == 1)
													{
														if (pcf->Read(&(dh.ulSizeImage), 1) == 1)
														{
															if (pcf->Read(&(dh.lXPelsPerMeter), 1) == 1)
															{
																if (pcf->Read(&(dh.lYPelsPerMeter), 1) == 1)
																{
																	if (pcf->Read(&(dh.ulClrUsed), 1) == 1)
																	{
																		if (pcf->Read(&(dh.ulClrImportant), 1) == 1)
																		{
																			m_sDepth = dh.usBitCount;
																			m_sWidth = m_sWinWidth = (short)dh.lWidth;
																			m_sHeight = m_sWinHeight = (short)dh.lHeight;
																			m_sWinX = m_sWinY = 0;

																			// Pre calc width in bits.
																			long lBitsWidth	= dh.lWidth * (long)dh.usBitCount;
																			m_lPitch		= WIDTH128(((lBitsWidth + 7) & ~7) / 8);
																			lDibPitch	= WIDTHUCHAR(((lBitsWidth + 7) & ~7) / 8);

																			// Calculate size.
																			// If not compressed . . .
																			if (dh.ulCompression == 0)
																				{
																				m_ulSize	= m_lPitch * dh.lHeight;
																				}
																			else
																				{
																				// Compressed, use bitmap's header.
																				m_ulSize	= dh.ulSizeImage;
																				}

																			if (CreateData(m_ulSize) == 0)
																			{
																				if (dh.usBitCount <= 8)
																				{
																					// Read palette in one chunk since only on
																					// this platform.
																					short sNumColors = 1 << dh.usBitCount;
																					CreatePalette(sNumColors * 4);
																					m_pPalette->m_sNumEntries = sNumColors;
																					m_pPalette->m_sPalEntrySize = 4;
																					if (pcf->Read(m_pPalette->m_pData, 4 * sNumColors) == 4 * sNumColors)
																					{
																					}
																					else
																					{
																						TRACE("RImage::LoadDib(): Unable to read palette.\n");
																						sRes = -20;
																					}
																				}
						
																				// If success so far . . .
																				if (sRes == 0)
																				{
																					if (pcf->Seek(lDibFileStartPos + dfh.ulOffBits, SEEK_SET) == 0)
																					{
																					// If not compressed . . .
																					if (dh.ulCompression == 0)
																						{
																						// If we read in the upside down way . . .
																						// "Upside down" way.
																						// Read the dib a line at a time and flip it upside down (which is really right side up)
																						for (long l = dh.lHeight - 1L; l >= 0L; l--)
																						{
																							if (pcf->Read(m_pData + (l * m_lPitch), lDibPitch) != lDibPitch)
																							{
																								TRACE("RImage::LoadDib(): Unable to read all the bits.\n");
																								sRes = -19;
																								break;
																							}
																						}
																						}	// Conflict of {} indentage.
																					else
																						{
																						// Read in one kerchunk.
																						if (pcf->Read(m_pData, m_ulSize) != (long)m_ulSize)
																							{
																							TRACE("RImage::LoadDib(): Unable to read all the compressed bits.\n");
																							sRes = -19;
																							}
																						}
																					}
																					else
																					{
																						TRACE("RImage::LoadDib(): Unable to seek to bits.\n");
																						sRes = -20;
																					}
																				}

																				// If any errors occurred . . .
																				if (sRes != 0)
																				{
																					// Free the allocated memory.
																					DestroyPalette();
																					DestroyData();
																				}
																			}
																			else
																			{
																				TRACE("RImage::LoadDib(): Unable to allocate DIB.\n");
																				sRes = -18;
																			}
																		}
																		else
																		{
																			TRACE("RImage::LoadDib(): Unable to read colors important field of bitmap info header.\n");
																			sRes = -17;
																		}
																	}
																	else
																	{
																		TRACE("RImage::LoadDib(): Unable to read colors used field of bitmap info header.\n");
																		sRes = -16;
																	}
																}
																else
																{
																	TRACE("RImage::LoadDib(): Unable to read vert pixels per meter field of bitmap info header.\n");
																	sRes = -15;
																}
															}
															else
															{
																TRACE("RImage::LoadDib(): Unable to read horz pixels per meter field of bitmap info header.\n");
																sRes = -14;
															}
														}
														else
														{
															TRACE("RImage::LoadDib(): Unable to read size of image field of bitmap info header.\n");
															sRes = -13;
														}
													}
													else
													{
														TRACE("RImage::LoadDib(): Unable to read compression field of bitmap info header.\n");
														sRes = -12;
													}
												}
												else
												{
													TRACE("RImage::LoadDib(): Unable to read bit count field of bitmap info header.\n");
													sRes = -11;
												}
											}
											else
											{
												TRACE("RImage::LoadDib(): Unable to read planes field of bitmap info header.\n");
												sRes = -10;
											}
										}
										else
										{
											TRACE("RImage::LoadDib(): Unable to read height field of bitmap info header.\n");
											sRes = -9;
										}
									}
									else
									{
										TRACE("RImage::LoadDib(): Unable to read width field of bitmap info header.\n");
										sRes = -8;
									}
								}
								else
								{
									TRACE("RImage::LoadDib(): Unable to read size field of bitmap info header.\n");
									sRes = -7;
								}
							}
							else
							{
								TRACE("RImage::LoadDib(): Unable to read offset to bits field of bitmap file header.\n");
								sRes = -6;
							}
						}
						else
						{
							TRACE("RImage::LoadDib(): Unable to read reserved2 field of bitmap file header.\n");
							sRes = -5;
						}
					}
					else
					{
						TRACE("RImage::LoadDib(): Unable to read reserved1 field of bitmap file header.\n");
						sRes = -4;
					}
				}
				else
				{
					TRACE("RImage::LoadDib(): Unable to read size field of bitmap file header.\n");
					sRes = -3;
				}
			}
			else
			{
				TRACE("RImage::LoadDib(): NOT a BITMAP file.\n");
				sRes = -3;
			}
		}
		else
		{
			TRACE("RImage::LoadDib(): Unable to read type field of bitmap file header.\n");
			sRes = -2;
		}
	}
	else
	{
		TRACE("RImage::LoadDib - RFile* does not refer to an open file\n");
		sRes = -1;
	}

	// If the dib load was successful.
	if (sRes == 0)
	{
		// Make all the image data members correspond to the loaded dib.
		if (dh.ulCompression == 0)
		{
			// The dib's data is raw, uncompressed.
			switch (dh.usBitCount)
			{
				case 1:
					// raw, 1 bit per pixel (monochrome).
					// (index of 0 == White and 1 == Black).
					m_type	= BMP1;
					m_pPalette->m_type = RPal::PDIB;
					break;

				case 4:
					// This color depth is really not supported by RImage!
					m_type = NOT_SUPPORTED;
					TRACE("RImage:LoadDib() encountered a dib of colordepth 4, not supported!\n");
					sRes = 1;
					break;

				case 8:
					// raw, 8 bits per pixel
					m_type	= BMP8;
					m_pPalette->m_type = RPal::PDIB;
					break;

				case 16:
					// This color depth is really not supported by RImage!
					m_type = NOT_SUPPORTED;
					TRACE("RImage:LoadDib() encountered a dib of colordepth 16, not supported!\n");
					sRes	= 1;
					break;

				case 24:
					// raw, 24 bits per pixel
					m_type	= BMP24;
					break;

				case 32:
					// raw, 32 bits per pixel
					m_type	= BMP24;
					break;
			
				default:
					// unsupported colordepth
					TRACE("RPalImage:LoadDib() encountered an unsupported colordepth!\n");
					m_type = NOT_SUPPORTED;
					sRes = 2;
					break;
			}
		}
		else
		{
		// Handle compressed bitmap type.
		switch (dh.usBitCount)
			{
			case 8:
				// RLE8.
				m_type	= BMP8RLE;
				break;
			default:
				// Unsupported compressed colordepth.
				TRACE("RImage::LoadDib(): Unsupported compressed colordepth "
						"(only 8bpp compression) supported.\n");
				break;
			}
		}

		// The following formats are to be converted automagically.
		switch (m_type)
			{
			case BMP8RLE:
				// Convert to BMP8.
				if (Convert(BMP8) != BMP8)
					{
					TRACE("RImage::LoadDib(): Failed to convert BMP8RLE to BMP8.\n");
					sRes = 2;
					}
				break;
			}
	}
	else
	{
		// CDib failed to load the file correctly.
		// Value in sRes already represents load failure error number.
	}


	return sRes;
}

//////////////////////////////////////////////////////////////////////
//
// SaveDib
//
// Description:
//		Saves the Image in DIB format (.bmp) to the given file.
//
// Parameters:
//		pszFilename = name of BMP file to be saved
//			or
//		pcf = pointer to open RFile where BMP is to be saved
//
// Returns:
//		SUCCESS if the file was saved
//		negative error code on failure
//
//////////////////////////////////////////////////////////////////////

short RImage::SaveDib(char* pszFilename)
{
	RFile cf;
	short sReturn = SUCCESS;

	if (cf.Open(pszFilename, "wb", RFile::LittleEndian) == SUCCESS)
	{
		sReturn = SaveDib(&cf);		
		cf.Close();
	}
	else
	{
		TRACE("RImage::SaveDib - Could not open file %s for saving - huh, ain't that somthing\n", pszFilename);
	 	sReturn = FAILURE;
	}

	return sReturn;
}

short RImage::SaveDib(RFile* pcf)
{
	short sRes = 0; // Assume success.

	if (pcf && pcf->IsOpen())
	{
		long lDibPitch = WIDTHUCHAR((((long)m_sWidth * (long)m_sDepth + 7L) & ~7L) / 8L);

		long	ulColorData	= 0;
		if (m_pPalette != NULL)
		{
			ulColorData	= m_pPalette->m_ulSize;
		}

		DIBFILEHEADER dfh;

		dfh.ulOffBits		= 14 + 40 + ulColorData;
		dfh.ulSize			= (dfh.ulOffBits + lDibPitch * (long)m_sHeight) / 4L;
		dfh.usReserved1	= 0;
		dfh.usReserved2	= 0;

		UCHAR	auc[2]	= { 'B', 'M' };

		//  Write BITMAPFILEHEADER
		if (pcf->Write(auc) == 1L && pcf->Write(auc + 1) == 1L)
		{
			if (pcf->Write(&dfh.ulSize) == 1)
			{
				if (pcf->Write(&dfh.usReserved1) == 1L)
				{
					if (pcf->Write(&dfh.usReserved2) == 1L)
					{
						if (pcf->Write(&dfh.ulOffBits) == 1L)
						{
							DIBHEADER	dh;
							dh.ulSize			= 40L;
							dh.usPlanes			= 1;
							switch (m_type)
								{
								case BMP8RLE:
									dh.ulCompression	= 1;	// BI_RLE8.
									dh.ulSizeImage		= m_ulSize;
									break;
								default:
									dh.ulCompression	= 0L;
									// Can't use ulSize b/c our buffer
									// may have a pitch that does not match
									// a DIB pitch.
									dh.ulSizeImage		= lDibPitch * m_sHeight;
									break;
								}

							dh.lXPelsPerMeter	= 50L;
							dh.lYPelsPerMeter	= 50L;
							dh.ulClrUsed		= 0L;
							dh.ulClrImportant	= 0L;
							if (pcf->Write(&dh.ulSize) == 1L)
							{
								long	lWidth	= (long)m_sWidth;
								if (pcf->Write(&lWidth) == 1L)
								{
									long	lHeight	= (long)m_sHeight;
									if (pcf->Write(&lHeight) == 1L)
									{
										if (pcf->Write(&dh.usPlanes) == 1L)
										{
											if (pcf->Write(&m_sDepth) == 1L)
											{
												if (pcf->Write(&dh.ulCompression) == 1L)
												{
													if (pcf->Write(&dh.ulSizeImage) == 1L)
													{
														if (pcf->Write(&dh.lXPelsPerMeter) == 1L)
														{
															if (pcf->Write(&dh.lYPelsPerMeter) == 1L)
															{
																if (pcf->Write(&dh.ulClrUsed) == 1L)
																{
																	if (pcf->Write(&dh.ulClrImportant) == 1L)
																	{
																		if (m_pPalette != NULL)
																		{
																			if (pcf->Write(m_pPalette->m_pData, m_pPalette->m_ulSize) == (long)m_pPalette->m_ulSize)
																			{
																			}
																			else
																			{
																				TRACE("RImage::SaveDib: Unable to write palette.\n");
																				sRes = -20;
																			}
																		}
					
																		// If success so far . . .
																		if (sRes == 0)
																		{
																			// If not compressed . . .
																			if (dh.ulCompression == 0)
																			{
																				// Upside down way.
																				// Write the dib a line at a time and flip it upside down (which is really right side up)
																				lHeight	= (long)m_sHeight;
																				for (long l = lHeight - 1L; l >= 0L; l--)
																				{
																					if (pcf->Write(m_pData + (l * m_lPitch), lDibPitch) != lDibPitch)
																					{
																						TRACE("RImage::SaveDib: Unable to write all the bits.\n");
																						sRes = -19;
																						break;
																					}
																				}
																			}
																			else
																			{
																				// Write in one big kerchunk.
																				if (pcf->Write(m_pData, m_ulSize) == (long)m_ulSize)
																				{
																				}
																				else
																				{
																				TRACE("RImage::SaveDib: Unable to write all the compressed bits.\n");
																				sRes	= -19;
																				}
																			}
																		}
																	}
																	else
																	{
																		TRACE("RImage::SaveDib: Unable to write colors important field of bitmap info header.\n");
																		sRes = -17;
																	}
																}
																else
																{
																	TRACE("RImage::SaveDib: Unable to write colors used field of bitmap info header.\n");
																	sRes = -16;
																}
															}
															else
															{
																TRACE("RImage::SaveDib: Unable to write vert pixels per meter field of bitmap info header.\n");
																sRes = -15;
															}
														}
														else
														{
															TRACE("RImage::SaveDib: Unable to write horz pixels per meter field of bitmap info header.\n");
															sRes = -14;
														}
													}
													else
													{
														TRACE("RImage::SaveDib: Unable to write size of image field of bitmap info header.\n");
														sRes = -13;
													}
												}
												else
												{
													TRACE("RImage::SaveDib: Unable to write compression field of bitmap info header.\n");
													sRes = -12;
												}
											}
											else
											{
												TRACE("RImage::SaveDib: Unable to write bit count field of bitmap info header.\n");
												sRes = -11;
											}
										}
										else
										{
											TRACE("RImage::SaveDib: Unable to write planes field of bitmap info header.\n");
											sRes = -10;
										}
									}
									else
									{
										TRACE("RImage::SaveDib: Unable to write height field of bitmap info header.\n");
										sRes = -9;
									}
								}
								else
								{
									TRACE("RImage::SaveDib: Unable to write width field of bitmap info header.\n");
									sRes = -8;
								}
							}
							else
							{
								TRACE("RImage::SaveDib: Unable to write size field of bitmap info header.\n");
								sRes = -7;
							}
						}
						else
						{
							TRACE("RImage::SaveDib: Unable to write offset to bits field of bitmap file header.\n");
							sRes = -6;
						}
					}
					else
					{
						TRACE("RImage::SaveDib: Unable to write reserved2 field of bitmap file header.\n");
						sRes = -5;
					}
				}
				else
				{
					TRACE("RImage::SaveDib: Unable to write reserved1 field of bitmap file header.\n");
					sRes = -4;
				}
			}
			else
			{
				TRACE("RImage::SaveDib: Unable to write size field of bitmap file header.\n");
				sRes = -3;
			}
		}
		else
		{
			TRACE("RImage::SaveDib: Unable to write type field of bitmap file header.\n");
			sRes = -2;
		}
	}
	else
	{
		TRACE("RImage::SaveDib - pcf does not refer to an open RFile\n");
		sRes = -1;
	}

	return sRes;
}

//////////////////////////////////////////////////////////////////////
//
// RImage::Save
//
// Description:
//		Writes out any image format except special cases of 
//		images that use pSpecial.  This function writes out
//		the image data and if you supply a size for the 
//		pSpecial buffer it will write that out also.  As long
//		as your pSpecial pointer doesn't point to data containing
//		other pointers, this will work.  If your pSpecial pointed
//		to another RPal for example, then it would not properly
//		save that data since RPal contains a pData pointer.  For
//		that special case you should write your own save function
//		for your special image type and first open a RFile and
//		call Save with that open RFile and a ulSpecialSize of 0
//		which will write the contents of the image except the
//		pSpecial buffer, the write your pSpecial data before
//		closing the RFile.
//
// Parameters:
//		pszFilename = filename of the image to be saved
//		ulSpecialSize = The size in bytes of the buffer pointed to
//						    by pSpecial.  
//
//	Returns:
//		SUCCESS if the file was saved successfully
//		FAILURE if there was an error - TRACE messages will help
//			     pinpoint the failure.
//
//////////////////////////////////////////////////////////////////////

short RImage::Save(char* pszFilename) const
{
	RFile cf;
	short sReturn = SUCCESS;

	if (cf.Open(pszFilename, "wb", RFile::LittleEndian) != SUCCESS)
	{
		TRACE("RImage::Save - could not open file for output\n");
	 	return FAILURE;
	}

	sReturn = Save(&cf);

	cf.Close();

	return sReturn;
}

//////////////////////////////////////////////////////////////////////
//
// RImage::Save
//
// Description:
//		Writes out any image format except special cases of 
//		images that use pSpecial.  This function writes out
//		the image data and if you supply a size for the 
//		pSpecial buffer it will write that out also.  As long
//		as your pSpecial pointer doesn't point to data containing
//		other pointers, this will work.  If your pSpecial pointed
//		to another RPal for example, then it would not properly
//		save that data since RPal contains a pData pointer.  For
//		that special case you should write your own save function
//		for your special image type and first open a RFile and
//		call Save with that open RFile and a ulSpecialSize of 0
//		which will write the contents of the image except the
//		pSpecial buffer, the write your pSpecial data before
//		closing the RFile.
//
//		This function assumes the the RFile object referrs to
//		an open file and will add to the file in the current location.
//		It will not close the file when it is finished so that you
//		could potentially add your own data following the image data.
//
// Parameters:
//		RFile* pcf = the open RFile to which to write the data
//
//	Returns:
//		SUCCESS if the file was saved successfully
//		FAILURE if there was an error - TRACE messages will help
//			     pinpoint the failure.
//
//////////////////////////////////////////////////////////////////////

short RImage::Save(RFile* pcf) const
{
	short sReturn = SUCCESS;
	ULONG ulFileType = IMAGE_COOKIE;
	ULONG	ulCurrentVersion = IMAGE_CURRENT_VERSION;

	if (pcf && pcf->IsOpen())
	{
		pcf->ClearError();
		pcf->Write(&ulFileType);
		pcf->Write(&ulCurrentVersion);
		// No RFile support for RImage::Type, so we use a U32.
		U32	u32Temp	= (ULONG)m_type;
		pcf->Write(&u32Temp);
		u32Temp			= (ULONG)m_typeDestination;
		pcf->Write(&u32Temp);
		pcf->Write(&m_ulSize);
		pcf->Write(&m_sWinWidth);
		pcf->Write(&m_sWinHeight);
		pcf->Write(&m_sWidth);
		pcf->Write(&m_sHeight);
		pcf->Write(&m_sWinX);
		pcf->Write(&m_sWinY);
		pcf->Write(&m_lPitch);
		pcf->Write(&m_sDepth);
		
		if (m_pData)
		{
			USHORT usFlag = 1;
			pcf->Write(&usFlag);
			WritePixelData(pcf);
		}
		else
		{
			USHORT usFlag = 0;
			pcf->Write(&usFlag);
		}

		if (m_pPalette)
		{
			USHORT usOne = 1;
			pcf->Write(&usOne);
			m_pPalette->Save(pcf);
		}
		else
		{
			USHORT usZero = 0;
			pcf->Write(&usZero);
		}


		// Call the special Save function for this type if any
		SAVEFUNC csf = GETSAVEFUNC(m_type);
		if (csf != NULL)
			// Note this must be changed to pass the version.
#ifdef _MSC_VER
	//#pragma message( __FILE__ "(2022) : Calls to SAVEFUNC must be changed to take a version!")
#endif
			sReturn = (*csf)(const_cast<RImage*>(this), pcf/*, ulCurrentVersion*/);
	}
	else
	{
	 	TRACE("RImage::Save - RFile pointer does not refer to an open file\n");
		sReturn = FAILURE;
	}
		
	return sReturn;
}

//////////////////////////////////////////////////////////////////////
//
// WritePixelData
//
// Description:
//		Private function called by Save to write the data in one of
//		two ways.  If the buffer is larger than the image, then it
//		must write the image line by line starting at the lXPos, lYPos
//		position in the buffer.  If a larger buffer is not being used, 
//		then it writes the image data in one chunk.
//
// Parameters:
//		pcf = pointer to an open RFile where the data will be written
//
// Returns:
//		SUCCESS if the pixel data was written correctly
//		FAILURE otherwise
//
//////////////////////////////////////////////////////////////////////

short RImage::WritePixelData(RFile* pcf) const
{
	short sReturn = SUCCESS;
	UCHAR* pLineData = NULL;

	if (m_sWidth <= m_sWinWidth && m_sHeight <= m_sWinHeight)
	{
		switch (m_sDepth)
		{
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
				if (pcf->Write(m_pData, m_ulSize) != (long) m_ulSize)
				{
					TRACE("RImage::WritePixelData - Error writing 8-bit or less pixel data\n");
					sReturn = FAILURE;
				}
				break;

			case 16:
				if (pcf->Write((U16*) m_pData, m_ulSize/2) != (long) m_ulSize/2)
				{
					TRACE("RImage::WritePixelData - Error writing 16-bit pixel data\n");
					sReturn = FAILURE;
				}
				break;

			case 24:
				if (pcf->Write((RPixel24*) m_pData, m_ulSize/3) != (long) m_ulSize/3)
				{
					TRACE("RImage::WritePixelData - Error writing 24-bit pixel data\n");
					sReturn = FAILURE;
				}
				break;

			case 32:
				if (pcf->Write((U32*) m_pData, m_ulSize/4) != (long) m_ulSize/4)
				{
					TRACE("RImage::WritePixelData - Error writing 32-bit pixel data\n");
					sReturn = FAILURE;
				}
				break;
			}
	}
	else
	{
		long	lYPos		= (long)m_sWinY;
		long	lXPos		= (long)m_sWinX;
		long	lHeight	= (long)m_sWinHeight;
		long	lDepth	= (long)m_sDepth;

		long l;
		long lBytesPerLine;
		if (m_sDepth < 8)
		{
			lBytesPerLine = (((long)m_sWinWidth * lDepth) / 8);
			if ((((long)m_sWinWidth * lDepth) % 8) > 0)
				lBytesPerLine++;
		}
		else
		{
			lBytesPerLine = (((long)m_sWinWidth * lDepth) / 8);
		}

		switch (m_sDepth)
		{
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
				for (l = lYPos; l < lYPos + lHeight; l++)
				{
					pLineData = m_pData + (lYPos * m_lPitch) + ((lXPos * lDepth)/8);
					if (pcf->Write(pLineData, lBytesPerLine) != lBytesPerLine)
					{
						TRACE("RImage::WritePixelData - Error writing 8-bit or less line %d of image\n", l-lYPos);
						sReturn = FAILURE;
					}
				} 	
				break;
				
			case 16:
				for (l = lYPos; l < lYPos + lHeight; l++)
				{
					pLineData = m_pData + (lYPos * m_lPitch) + ((lXPos * lDepth)/8);
					if (pcf->Write((U16*) pLineData, lBytesPerLine/2) != lBytesPerLine/2)
					{
						TRACE("RImage::WritePixelData - Error writing 16-bit line %d of image\n", l-lYPos);
						sReturn = FAILURE;
					}
				} 	
				break;

			case 24:
				for (l = lYPos; l < lYPos + lHeight; l++)
				{
					pLineData = m_pData + (lYPos * m_lPitch) + ((lXPos * lDepth) / 8);
					if (pcf->Write((RPixel24*) pLineData, lBytesPerLine/3) != lBytesPerLine/3)
					{
						TRACE("RImage::WritePixelData - Error writing 24-bit line %d of image\n", l-lYPos);
						sReturn = FAILURE;
					}
				} 	
				break;

			case 32:
				for (l = lYPos; l < lYPos + lHeight; l++)
				{
					pLineData = m_pData + (lYPos * m_lPitch) + ((lXPos * lDepth)/8);
					if (pcf->Write(pLineData, lBytesPerLine/4) != lBytesPerLine/4)
					{
						TRACE("RImage::WritePixelData - Error writing 32-bit line %d of image\n", l-lYPos);
						sReturn = FAILURE;
					}
				} 	
				break;
		}

	}
	return sReturn;
}

//////////////////////////////////////////////////////////////////////
//
// Load
//
// Description:
//		Loads any standard image format including any straight
//		forward pSpecial data that was saved.  If there is a special
//		Save function for pSpecial data that contained pointers to
//		other data, then you will have to use the special load function
//		as well.
//
// Parameters:
//		pszFilename = filename of the IMG file to load
//
// Returns:
//		SUCCESS if the file was loaded correctly
//		FAILURE if there was an error - TRACE messages will help
//				  pinpoint the error.
//
//////////////////////////////////////////////////////////////////////

short RImage::Load(char* pszFilename)
{
	RFile cf;
	short sReturn = SUCCESS;

	if (cf.Open(pszFilename, "rb", RFile::LittleEndian) != SUCCESS)
	{
		TRACE("RImage::Load - could not open file for input\n");
	 	return FAILURE;
	}

	sReturn = Load(&cf);

	cf.Close();

	return sReturn;
}

short RImage::Load(RFile* pcf)
{	
	short sReturn = SUCCESS;

	if (pcf && pcf->IsOpen())
	{
		// Load image dependent on version.
		sReturn	= RImageFile::Load(this, pcf);
	}
	else
	{
	 	TRACE("RImage::Load - RFile pointer does not refer to an open file\n");
		sReturn = FAILURE;
	}

	// If the file was loaded in as a different type than the
	// desired destination type, then convert it.
	if (m_type != m_typeDestination && m_typeDestination != NOT_SUPPORTED)
		if (Convert(m_typeDestination) != m_typeDestination)
			TRACE("RImage::Load - Convert to Destination type %d failed\n", m_typeDestination);

	return sReturn;
}

//////////////////////////////////////////////////////////////////////
//
// ReadPixelData
//
// Description:
//		Private function called by Load to read the data in one of
//		two ways.  If the buffer is larger than the image, then it
//		must read the image line by line and put it at the lXPos, lYPos
//		position in the buffer.  If a larger buffer is not being used,
//		then it reads the image data in one chunk.
//
// Parameters:
//		pcf = pointer to an open RFile where the data will be read
//
// Returns:
//		SUCCESS if the pixel data was read correctly
//		FAILURE otherwise
//
//////////////////////////////////////////////////////////////////////

short RImage::ReadPixelData(RFile* pcf)
{
	short sReturn = SUCCESS;
	UCHAR* pLineData = NULL;

	if (m_sWidth <= m_sWinWidth && m_sHeight <= m_sWinHeight)
	{
		switch (m_sDepth)
		{
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
				if (pcf->Read(m_pData, m_ulSize) != (long) m_ulSize)
				{
					TRACE("RImage::ReadPixelData - Error reading 8-bit or less pixel data\n");
					sReturn = FAILURE;
				}
				break;

			case 16:
				if (pcf->Read((U16*) m_pData, m_ulSize/2) != (long) m_ulSize/2)
				{
					TRACE("RImage::ReadPixelData - Error reading 16-bit pixel data\n");
					sReturn = FAILURE;
				}
				break;

			case 24:
				if (pcf->Read((RPixel24*) m_pData, m_ulSize/3) != (long) m_ulSize/3)
				{
					TRACE("RImage::ReadPixelData - Error reading 24-bit pixel data\n");
					sReturn = FAILURE;
				}
				break;

			case 32:
				if (pcf->Read((U32*) m_pData, m_ulSize/4) != (long) m_ulSize/4)
				{
					TRACE("RImage::ReadPixelData - Error reading 32-bit pixel data\n");
					sReturn = FAILURE;
				}
				break;
			}
	}
	else
	{
		long	lYPos		= (long)m_sWinY;
		long	lXPos		= (long)m_sWinX;
		long	lHeight	= (long)m_sWinHeight;
		long	lDepth	= (long)m_sDepth;

		long l;
		long lBytesPerLine;
		if (m_sDepth < 8)
		{
			lBytesPerLine = (((long)m_sWinWidth * lDepth) / 8);
			if ((((long)m_sWinWidth * lDepth) % 8) > 0)
				lBytesPerLine++;
		}
		else
		{
			lBytesPerLine = (((long)m_sWinWidth * lDepth) / 8);
		}

		switch (m_sDepth)
		{
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
				for (l = lYPos; l < lYPos + lHeight; l++)
				{
					pLineData = m_pData + (lYPos*m_lPitch) + ((lXPos*lDepth)/8);
					if (pcf->Read(pLineData, lBytesPerLine) != lBytesPerLine)
					{
						TRACE("RImage::ReadPixelData - Error reading 8-bit or less line %d of image\n", l-lYPos);
						sReturn = FAILURE;
					}
				} 	
				break;
				
			case 16:
				for (l = lYPos; l < lYPos + lHeight; l++)
				{
					pLineData = m_pData + (lYPos*m_lPitch) + ((lXPos*lDepth)/8);
					if (pcf->Read((U16*) pLineData, lBytesPerLine/2) != lBytesPerLine/2)
					{
						TRACE("RImage::ReadPixelData - Error reading 16-bit line %d of image\n", l-lYPos);
						sReturn = FAILURE;
					}
				} 	
				break;

			case 24:
				for (l = lYPos; l < lYPos + lHeight; l++)
				{
					pLineData = m_pData + (lYPos*m_lPitch) + ((lXPos*lDepth)/8);
					if (pcf->Read((RPixel24*) pLineData, lBytesPerLine/3) != lBytesPerLine/3)
					{
						TRACE("RImage::ReadPixelData - Error reading 24-bit line %d of image\n", l-lYPos);
						sReturn = FAILURE;
					}
				} 	
				break;

			case 32:
				for (l = lYPos; l < lYPos + lHeight; l++)
				{
					pLineData = m_pData + (lYPos*m_lPitch) + ((lXPos*lDepth)/8);
					if (pcf->Read(pLineData, lBytesPerLine/4) != lBytesPerLine/4)
					{
						TRACE("RImage::ReadPixelData - Error reading 32-bit line %d of image\n", l-lYPos);
						sReturn = FAILURE;
					}
				} 	
				break;
		}
	}
	return sReturn;
}


///////////////////////////////////////////////////////////////////////////////
// EOF
///////////////////////////////////////////////////////////////////////////////
