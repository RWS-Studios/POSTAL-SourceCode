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
#ifndef IMAGE_H
#define IMAGE_H
//////////////////////////////////////////////////////////////////////
//
//	IMAGE.H
//   
//	Created on 		4/20/95	MR 
// Implemented on 4/21/95	JW
// 05/08/95	JMI	Took out CDib dependency and added internal method
//						for loading BMPs.
//
//	05/10/95	JMI	Added prototype for conversion function Convert in
//						CPal that converts from current type to others.
//
//	05/10/95	JMI	Added member to store whether it's okay to free
//						the palette data (i.e., we allocated it in CPal).
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
// 02/27/96 BRH	Changed the Convert functions from using DYNALINK
//						to the friend class CImageSpecialFunc which includes
//						ConvertTo, ConvertFrom, Load, Save, Alloc, and Delete.
//						It became necessary to load and save special formats
//						like fast sprite so that the images could be on the
//						disk in the final form used in the game rather than
//						loading bitmaps and converting them in the game code.
//						It was also necessary to add alloc/delete functions
//						so that special image types that may allocate structures
//						or classes associated with the pSpeical pointer could
//						be freed properly when the CImage destructor was called,
//						and similarly if someone wanted to create a new CImage
//						of a special type with a structure or class, the constructor
//						will call the special Alloc function to allocate the special
//						data associated with the pSpecial pointer.
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
//	10/21/96 BRH	Added file type and version to identify palette files.
//
//	10/30/96	JMI	Pulled CPal stuff out of here and put it into pal.h.
//						Pulled enum stuff out of imagetyp.h and put it here.
//						Removed #include of imagtyp.h.
//						Added #include of pal.h.
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
//						Added a using directive to IMAGELINKLATE.
//
//	10/31/96	JMI	Changed all members to be preceded by m_ (e.g., sDepth
//						m_sDepth).  Changed all position members (i.e., lWidth,
//						lHeight, lBufferWidth, lBufferHeight, lXPos, & lYPos) to
//						be shorts (i.e., m_sWidth, m_sHeight, m_sBufferWidth,
//						m_sBufferHeight, m_sXPos, m_sYPos) and functions associated
//						with these members reflect this change (e.g., long GetWidth()
//						is now short GetWidth()).  Changed ulType to	m_type and 
//						ulDestinationType to m_typeDestination.  Increased file
//						version to 5 since members converted to short will affect
//						the file format.
//
//	11/06/96 MJR	Got rid of "using" syntax in IMAGELINKLATE macro
//						because that didn't work with CodeWarrior on the mac.
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
//	12/18/96 BRH	Added overloaded versions of LoadDib and SaveDib that
//						take an RFile* and can load or save to that rather than
//						just the filename versions.  This was first used
//						in the resource manager but is also otherwise
//						generally useful.
//
//	12/18/96	JMI	Added TOUCH macros to aid in dynamic linking.  See comment
//						there for more detail.
//
//	02/04/97	JMI	Added BMP_COOKIE for in detecting a .BMP formatted file.
//						Made LoadDib() a private member.  Now Load() will load *.BMP
//						or *.IMG formatted files.
//
//	04/16/97	JMI	Changed comment for operator= overload and parameter name.
//
// 05/21/97 JRD	Fixed a bug in GetPitch.
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
// your compressed version and use m_pSpecial to keep track of it.
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


// Blue include files
#include "Blue.h"

// If PATHS_IN_INCLUDES macro is defined, we can utilized relative
// paths to a header file.  In this case we generally go off of our
// RSPiX root directory.  System.h MUST be included before this macro
// is evaluated.  System.h is the header that, based on the current
// platform (or more so in this case on the compiler), defines 
// PATHS_IN_INCLUDES.  Blue.h includes system.h so you can include that
// instead.
#ifdef PATHS_IN_INCLUDES
	// Green include files
	#include "GREEN/Image/pal.h"

	// Orange include files
	#include "ORANGE/File/file.h"
#else
	// Green include files
	#include "pal.h"

	// Orange include files
	#include "file.h"
#endif // PATHS_IN_INCLUDES


///////////////////////////////////////////////////////////////////////////////
// Macros.
///////////////////////////////////////////////////////////////////////////////

#define WIDTHUCHAR(i)   (((i)+3) & ~3)      // ULONG aligned
#define WIDTH128(i)		 (((i)+15) & ~15)	 // 128-bit aligned

#define IMAGE_COOKIE	0x20204d49 // Looks like "IM  " in the file
#define IMAGE_CURRENT_VERSION	5 // Current file version.  Change
										  // this number if you change the .IMG
										  // file format.  It will be compared to
										  // the version number in the file as a check

#define BMP_COOKIE	0x4d42		// Looks like "BM" in the file.

///////////////////////////////////////////////////////////////////////////////
// Typedefs.
///////////////////////////////////////////////////////////////////////////////

// Windows Independent DIB BITMAPFILEHEADER representation.
typedef struct
{
	USHORT	usType;
	ULONG		ulSize;
	USHORT	usReserved1;
	USHORT	usReserved2;
	ULONG		ulOffBits;
} DIBFILEHEADER;

// Windows Independent DIB BITMAPINFOHEADER representation.
typedef struct
{
	ULONG			ulSize;
	long			lWidth;
	long			lHeight;
	USHORT		usPlanes;
	USHORT		usBitCount;
	ULONG			ulCompression;
	ULONG			ulSizeImage;
	long			lXPelsPerMeter;
	long			lYPelsPerMeter;
	ULONG			ulClrUsed;
	ULONG			ulClrImportant;
} DIBHEADER, *PDIBHEADER;

//////////////////////////////////////////////////////////////////////
//
//	RImage class 
//
// RImage is a simple wrapper around various types of images.
// This class does not understand any particular image types.
// I'm not sure that the above sentence is true anymore....?
//
//////////////////////////////////////////////////////////////////////
class RImage
{
	public:	// Typedefs and Enums.

		// Insert your type before the END_OF_TYPES enumeration and make
		// an entry for your convert function in the array of conversion 
		// functions afpConvert[] which is in imageafp.h.  The END_OF_TYPES
		// is used as a check against invalid conversion types and must 
		// be the last type in the enum.
		// IMPORTANT NOTE:  The position of the standard types (BMP8 -
		// SCREEN32_ARGB) must remain the same.  Always add your enum before
		// END_OF_TYPES to be safe!  Also remember to add the name of your
		// type to the string ms_astrTypeNames in image.cpp.
		typedef enum eImageTypes
		{
			NOT_SUPPORTED,			// To indicate error on load, etc.
			BMP8,						// Windows BMP 8-bit buffer, RGBQUAD
			SYSTEM8,					// 8-bit buffer with palette in SYSTEM format (Win/Mac/etc)
			SCREEN8_555,			// 8-bit buffer, screen formatted palette 555
			SCREEN8_565,			// 8-bit buffer, screen formatted palette 565
			SCREEN8_888,			// 8-bit buffer, screen formatted palette 888
			BMP24,					// Windows BMP 24-bit buffer, 
			SCREEN16_555,			// 16-bit buffer with pixel values in screen format 555
			SCREEN16_565,			// 16-bit buffer with pixel values in screen format 565
			SCREEN24_RGB,			// 24-bit buffer with pixel values in screen format 888
			SCREEN32_ARGB,			// 32-bit buffer with pixel values in screen Alpha + RGB format (8888)
			FSPR1,					// Used with BLiT Fast Text
			FSPR8,					// Used with BLiT Fast Sprites
			FSPR16,					// Used with BLiT Fast Sprites
			FSPR32,					// Used with BLiT Fast Sprites
			ROTBUF,					// Used with two current live roration/scale algorithms
			SPECIAL,					// IMAGE needs a sophisticated convert call
			FLX8_888,				// 8-bit buffer indexing a 256 color 
										// R, G, B (in that order) palette.  JMI	03/06/96
			IMAGE_STUB,				// Use to make anything APPEAR to be an image
										// special field = per stub info, buffers = NULL
			BMP8RLE,					// 8-bit compressed (Windows RLE8 bitmap) format.
										// Uses PDIB for palette.  JMI	07/22/96
			BMP1,						// Monochrome bitmap.  No palette 
										// (1 == Black, 0 == White).  JMI	09/04/96
			END_OF_TYPES
		} Type;


	public:	// Member vars.
		Type	m_type;						// Image type
		Type	m_typeDestination;		// Type to convert to upon load
												// (New version 2)
		ULONG			m_ulSize;			// Image data's size
		short			m_sWinWidth;		// Width of image
		short			m_sWinHeight;		// Height of image
		short			m_sWidth;			// Width of buffer   (new version 2)
		short			m_sHeight;			// Height of buffer (new version 2)
		short			m_sWinX;				// Position of image in the buffer
		short			m_sWinY;				// Position of image in the buffer

		long			m_lPitch;			// Pitch of image
		short			m_sDepth;			// Color depth of image
		UCHAR*		m_pData;				// Pointer to data
		RPal*			m_pPalette;			// Pointer to palette class
		UCHAR*		m_pSpecial;			// Generic pointer for expandability
												// (other image formats)
		UCHAR*		m_pSpecialMem;		// Pointer to allocated special memory
		UCHAR*		m_pMem;				// Pointer to the memory buffer
												// (for alloc/dealloc of aligned data)
												// User access in lieu of an AttachData function

		// This array of type names should correspond to the above list of
		// enumerated types.  Whenever you add an image type and an enum, 
		// you need to also insert that name into the corresponding place
		//	in this array in image.cpp.  
		// Note that this uses END_OF_TYPES enum item to size the array.
		static char* ms_astrTypeNames[END_OF_TYPES];

	private:
		RPal*			m_pPalMem;		// Pointer to Image allocated palette

	public:
		// Class initialization
		static void Init(void);

		// Safe method for destroying memory that was originally
		// allocated by Image::CreateData and then detached from
		// the RImage by Image::DetachData.  This routine will use
		// the correct free() function for the memory handle.
		static short DestroyDetachedData(void** hMem);

		// General Constructor
		RImage();

		// Constructor that allocates data for the buffer
		// Same as calling CreateData(ulSize)
		RImage(ULONG ulSize);

		// Constructor that allocates a buffer and loads a bitmap
		// Same as calling LoadDib(pszFilename, ulType)
		RImage(char* pszFilename);

		// General Destructor
		~RImage();

		// Equals overload.
		// Note that this function could fail.
		RImage& operator=(const RImage& imSrc);

		// Create IMAGE's data using the specified values.
		short CreateData(	// Returns 0 if successful
			ULONG ulSize);	// Size of data

		// Create IMAGE's data utilizing passed in fields.
		// Calls CreateData(ULONG) to do the allocation.
		short CreateImage(			// Returns 0 if successful.
			short	sWidth,				// Width of new buffer.
			short	sHeight,				// Height of new buffer.
			Type type,			// Type of new buffer.
			long	lPitch	= 0L,		// Pitch of new buffer or 0 to calculate.
			short	sDepth	= 8);		// Color depth of new buffer.

		// Detach the data from the Image.  This function returns a pointer
		// to the memory buffer which can and should be freed by whoever
		// detached it.  Alternatively you can supply pointers to void* 
		// that will be given the memory and data pointers for the Image 
		// buffer.  This function is usefull when doing conversions between
		// image types.  You can detach the buffer from the Image, have the
		// Image create a new buffer (for the converted data) and then free
		// the detached buffer when you're done with the conversion.
		short DetachData(void** pMem, void** pData);
		void* DetachData();

		// Destroy IMAGE's data
		short DestroyData();

		// Allow the user to set the data pointer.
		short SetData(void* pData);

		// Set the pPalette pointer to a given RPal
		short SetPalette(RPal* pPal);

		// Create palette and assign its pointer to pPalette
		short CreatePalette(ULONG ulSize);

		// Create a palette but do not create a data buffer for the palette
		short CreatePalette(void);

		// Detach the palette from the Image and return a pointer to it.
		// whoever detaches the palette is responsible for it afterward
		// and they must free it when they are done.
		RPal* DetachPalette();

		// Destroy the image's palette
		short DestroyPalette();

		// Saves the current image in DIB format to pszFileName.
		// Returns 0 on success.
		short SaveDib(char* pszFileName);

		// Saves the current image ind DIB format to an open RFile
		short SaveDib(RFile* pcf);

		// Save any image format except data pointed to by m_pSpecial
		// This version takes a filename and saves the file.  If the
		// Image type has m_pSpecial data to save then there should be
		// a special save function registered in the RImageSpecialFunc class
		short Save(char* pszFilename) const;

		// This version takes an open RFile pointer and writes
		// to the file.  This may be useful if you have m_pSpecial data, you
		// can write your own save function that will open a RFile and
		// call this function to write the main image data and then you
		// can write your data to the same RFile before closing it.
		short Save(RFile* pcf) const;

		// Load the standard image formats, ie. those that do not
		// include m_pSpecial data
		short Load(char* pszFilename);

		// This version takes an open RFile pointer and reads the
		// standard image from the file.  This may be useful if you
		// have a special image type that uses the m_pSpecial pointer.  You
		// can write your own load function that opens a RFile
		short Load(RFile* pcf);

		// Converts the RImage from its current type to the new type
		// by converting from the current form to the standard and then
		// converting the standard into the new type.
		Type Convert(Type typeNew);

		// Query functions
		short GetHeight(void) 	{return m_sHeight;};
		short GetWidth(void)		{return m_sWidth;};
		static long GetPitch(short sWidth, short sDepth)
			{return ((long)sWidth * ((long)sDepth / 8L) + 0x0000000F) & 0xFFFFFFF0;}
		Type GetType(void)		{return m_type;};

		// Memory access functions
		UCHAR* GetBuffer(void)	{return m_pData;};
		UCHAR* GetMemory(void)	{return m_pMem;};

	private:
		// Loads a DIB into a standard RImage format.
		short	LoadDib(char* pszFilename);

		// Loads a DIB from an open RFile
		short LoadDib(RFile* pcf);

		// Writes the data in one chunk if the buffer is the same
		// size as the image, or line by line if the buffer is larger
		// than the image.  If the buffer is smaller than the image, 
		// then you have some problem.
		short WritePixelData(RFile* pcf) const;

		// Reads the data in one chunk if the buffer is the same
		// size as the image, or line by line if the buffer is larger
		// than the image.  If the buffer is smaller than the image 
		// then the image won't fit and the program will probably 
		// crash and it will be your fault.
		short ReadPixelData(RFile* pcf);

		// Initialize all members.  Calling this when m_pMem* is set is not a good idea.
		void InitMembers(void);

		//	To allocate memory for the data buffers of CPal
		static short sCreateMem(void **hMem,ULONG ulSize);

		//	To allocate memory and return a pointer aligned to 128-bits
		//	for optimum blit speed.  This is the function used by
		//	CImage when it creates memory for the image buffers.
		static short sCreateAlignedMem(void **hMem, void **hData, ULONG ulSize);

		//	To free the data buffers of CPal and CImage that were created 
		//	using either sCreateMem() or sCreateAlignedMem()
		static short sDestroyMem(void **hMem);


	public:
		// Who's our friend?  RPal!  Yayayyayay.
		// Perhaps, since these two genderlessnesses are so close, they
		// should be friends /wink /wink.
		friend class RPal;
		// RImageFile is our friend so it can load formats for us.
		friend class RImageFile;
};

//////////////////////////////////////////////////////////////////////
//
//	RImageInit class
//
// This class is used to create several arrays of function pointers
// for special image types.  A special image type may support, 
// special allocation and deallocation, conversion functions, 
// and load and save functions.  This class replaces the DYNALINK 
// module.  DYNALINK was used in the previous version to provide
// support for ConvertTo and ConvertFrom functions for special image
// types so that the convert functions could be in different files, 
// and so that convert functions that were needed for a particular
// application were automatically included by including the blit
// module that was being used.
//
//////////////////////////////////////////////////////////////////////

// Conversion from extended to standard function typedef
typedef short (*CONVFROMFUNC)(RImage* pImage);
// Conversion to extended from standard function typedef
typedef short (*CONVTOFUNC)(RImage* pImage);
// Load extension for special types - loads m_pSpecial data
typedef short (*LOADFUNC)(RImage* pImage, RFile* pcf/*, ULONG ulVersion*/);
// Save extension for special types - saves m_pSpecial data
typedef short (*SAVEFUNC)(RImage* pImage, RFile* pcf/*, ULONG ulVersion*/);
// Special data allocation function
typedef short (*ALLOCFUNC)(RImage* pImage);
// Special data deallocation function
typedef short (*DELETEFUNC)(RImage* pImage);


class RImageSpecialFunc : public RImage
{
	public:

		RImageSpecialFunc(Type type,
								CONVTOFUNC pfnConvertTo,
								CONVFROMFUNC pfnConvertFrom,
								LOADFUNC pfnLoad,
								SAVEFUNC pfnSave,
								ALLOCFUNC pfnAlloc,
								DELETEFUNC pfnDelete)
			{
				ASSERT(type < END_OF_TYPES);
				ASSERT(ms_apfnConvTo[type] == NULL);
				ms_apfnConvTo[type] = pfnConvertTo;

				ASSERT(ms_apfnConvFrom[type] == NULL);
				ms_apfnConvFrom[type] = pfnConvertFrom;

				ASSERT(ms_apfnLoad[type] == NULL);
				ms_apfnLoad[type] = pfnLoad;

				ASSERT(ms_apfnSave[type] == NULL);
				ms_apfnSave[type] = pfnSave;

				ASSERT(ms_apfnAlloc[type] == NULL);
				ms_apfnAlloc[type] = pfnAlloc;

				ASSERT(ms_apfnDelete[type] == NULL);
				ms_apfnDelete[type] = pfnDelete;
			};

	friend class RImage;
	// RImageFile is our friend so it can load formats for RImage.
	friend class RImageFile;

	protected:
		static CONVTOFUNC		ms_apfnConvTo[END_OF_TYPES];
		static CONVFROMFUNC	ms_apfnConvFrom[END_OF_TYPES];
		static LOADFUNC		ms_apfnLoad[END_OF_TYPES];
		static SAVEFUNC		ms_apfnSave[END_OF_TYPES];
		static ALLOCFUNC		ms_apfnAlloc[END_OF_TYPES];
		static DELETEFUNC		ms_apfnDelete[END_OF_TYPES];
};


// These macros are used to access each type of special function

#define GETTOFUNC(lIndex) \
	RImageSpecialFunc::ms_apfnConvTo[lIndex]

#define GETFROMFUNC(lIndex) \
	RImageSpecialFunc::ms_apfnConvFrom[lIndex]

#define GETLOADFUNC(lIndex) \
	RImageSpecialFunc::ms_apfnLoad[lIndex]

#define GETSAVEFUNC(lIndex) \
	RImageSpecialFunc::ms_apfnSave[lIndex]

#define GETALLOCFUNC(lIndex) \
	RImageSpecialFunc::ms_apfnAlloc[lIndex]

#define GETDELETEFUNC(lIndex) \
	RImageSpecialFunc::ms_apfnDelete[lIndex]


// This macro needs to be called at file scope of RImage.cpp so that
// the arrays of special function pointers are allocated and initialized
// to NULL.  It will instantiate the RImageSpecialFunc class and its
// arrays of function pointers.  
//
// The arrays are set to the size END_OF_TYPES since the special
// function pointers will be indexed by the image type.

#define IMAGELINKINSTANTIATE() \
	CONVTOFUNC RImageSpecialFunc::ms_apfnConvTo[END_OF_TYPES] = { NULL, }; \
	CONVFROMFUNC RImageSpecialFunc::ms_apfnConvFrom[END_OF_TYPES] = { NULL, }; \
	LOADFUNC RImageSpecialFunc::ms_apfnLoad[END_OF_TYPES] = { NULL, }; \
	SAVEFUNC RImageSpecialFunc::ms_apfnSave[END_OF_TYPES] = { NULL, }; \
	ALLOCFUNC RImageSpecialFunc::ms_apfnAlloc[END_OF_TYPES] = { NULL, }; \
	DELETEFUNC RImageSpecialFunc::ms_apfnDelete[END_OF_TYPES] = { NULL, }

// This macro is used to add the special functions to the image's 
// list.  You must specify a function pointer for each of the 6 special
// types or NULL if your special type does not require or support one
// or more types.  For example, if you support convert to/from and 
//	save/load but do not require any additional buffers or data structures
// then you must provide the functions for convert to/from and load/save 
// and then specify NULL, NULL for the alloc and delete functions.
//
// ulImageType = one of the enumerated image types registered in image.h
// pTo = function pointer to your ConvertTo function
// pFrom = function pointer to your ConvertFrom function
// pLoad = function pointer to your Load function that reads just your
//			  special data.  The standard RImage information is loaded by 
//			  the RImage::Load function and this function will be called
//			  when it gets to the m_pSpecial data for your special image type
// pSave = function pointer to your Save function that writes just your
//			  special data.  The standard RImage information is saved by
//			  the CImgae::Save function and this function will be called
//			  when it is time to write the m_pSpecial data. 
// pAlloc = function pointer to your special memory allocation function.
//			   If your m_pSpecial data is a class or contains multiple
//			   pointers to other data buffers, then you should provide a
//				function that can allocate this data based on the standard
//			   information in the RImage
// pDelete = function pointer to your special memory deallocation 
//				 function.  If your m_pSpecial data is a class or contains
//				 multiple pointers to other data buffers that need to be
//				 freed when the RImage destructor is called, then you
//				 should provide a function that will clean up your m_pSpecial
//				 data.

#define IMAGELINKLATE(type, pTo, pFrom, pLoad, pSave, pAlloc, pDel) \
	static RImageSpecialFunc imSpecial##type(RImage::type, pTo, pFrom, pLoad, pSave, pAlloc, pDel)


///////////////////////////////////////////////////////////////////////////////
// The problem:
// Converters, Loaders, Savers, Allocators, and Deleters for non-standard types
// are lunk in at run time.  The linker is generally not smart enough (or dumb
// enough) to leave this code in the resulting executable unless the functions
// are explicitly referenced by something else that is to be included in the
// executable.
// 
// TOUCH macros:
// These macros allow you to 'touch' a convert function to make sure it will
// be included in the final exe.  It is not recommended that they be used in
// library code b/c of two problems:
// 1) They cannot be used twice.  That is, no two modules can contain two
// references to any *TOUCH(type) for the same type.
//	2) They will only work if the module that contains the *TOUCH(type) is
// included in the exe.  A module that would otherwise be dropped from the exe
// by the linker with a *TOUCH(type) will do no one any good.
//
// So use these in your application level modules to cause the particular
// type's equivalent ConvertTo, ConvertFrom, Save, Load, Alloc, and/or Delete
// func to be included in your exe.
//
//	Non-standard func:						Equivalent TOUCH macro:
//	=====================					========================
//	ConvertToTYPE								CONVTOTOUCH(TYPE)
//	ConvertFromTYPE							CONVFROMTOUCH(TYPE)
//	SaveTYPE										SAVETOUCH(TYPE)
//	LoadTYPE										LOADTOUCH(TYPE)
//	AllocTYPE									ALLOCTOUCH(TYPE)
//	DeleteTYPE									DELETETOUCH(TYPE)
//
// THIS METHOD HAS NOT BEEN TESTED WITH THE MAC COMPILER.  I presume the logic
// for dropping stuff is the same.  There is another plausible solution for
// this problem under Metrowerks.  Let me know (Jo8n) if this doesn't work for
// you!  Let me know if it does (that way I can remove this message).
//
///////////////////////////////////////////////////////////////////////////////
#define CONVTOTOUCH(TYPE)		extern void ReferenceConvTo##TYPE(void)		\
											{														\
											extern short ConvertTo##TYPE(RImage*);		\
											ConvertTo##TYPE(NULL);							\
											}

#define CONVFROMTOUCH(TYPE)	extern void ReferenceConvFrom##TYPE(void)		\
											{														\
											extern short ConvertFrom##TYPE(RImage*);	\
											ConvertFrom##TYPE(NULL);						\
											}

#define SAVETOUCH(TYPE)			extern void ReferenceSave##TYPE(void)			\
											{														\
											extern short Save##TYPE(RImage*, RFile*); \
											Save##TYPE(NULL, NULL); 						\
											}

#define LOADTOUCH(TYPE)			extern void ReferenceLoad##TYPE(void)			\
											{ 														\
											extern short Load##TYPE(RImage*, RFile*); \
											Load##TYPE(NULL, NULL); 						\
											}

#define ALLOCTOUCH(TYPE)		extern void ReferenceAlloc##TYPE(void)			\
											{ 														\
											extern short Alloc##TYPE(RImage*); 			\
											Alloc##TYPE(NULL); 								\
											}

#define DELETETOUCH(TYPE)		extern void ReferenceDelete##TYPE(void)		\
											{ 														\
											extern short Delete##TYPE(RImage*); 		\
											Delete##TYPE(NULL);								\
											}

#endif //IMAGE_H

//////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////

