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
//	IMAGECON.CPP
//   
//	Created on 		09/28/95	BRH 
// Implemented on 09/28/95 BRH
//
// 09/28/95	BRH	Standard conversion functions for the standard
//						image types.  The two main standards are 8 and 24
//						bit images.  The standard converters are explained
//						below
//
//	11/03/95 BRH	Added support for RPal's new sPalEntrySize.  Any
//						conversion that changes the palette will also 
//						update this variable.  
//
//	11/06/95 BRH	Fixed the ConvertToSCREEN8_888 to use the proper
//						24-bit palette entries rather than the 32-bit
//						that it was using.
//
//	11/07/95	JMI	Changed all occurrences of RGBQUAD to IM_RGBQUAD &
//						RGBTRIPLE to IM_RGBTRIPLE.
//
// 11/08/95 BRH	Added several direct conversions to BMP8 and BMP24.
//						Previously we had BMP8 and BMP24 as our two base
//						types which then were converted to SCREEN8_xxx formats
//						and others.  By providing a few more direct conversions
//						I can then provide almost any other conversion by
//						calling two conversion functions, one from the current
//						to BMP8 and then from BMP8 to the new format.  The 
//						only conversions not supported are high color or
//						true color images being converted to palette images.
//
// 11/15/95	BRH	Change the conversion routines that deal with palettes
//						so that the images keep the original RPal object in
//						their pPalette pointers when the are converted.  
//						Previously the conversion routines detached the
//						palette from the RImage and then created a new palette
//						and attached that to the RImage.  This causes trouble
//						if there are other external pointers to the same RPal, 
//						for example if several images use the same palette we
//						don't want to throw the palette out.  Now the image
//						keeps the same RPal object and only the palette's data
//						is discarded during the conversion.
//
//	11/20/95 BRH	Finished up all of the cross conversions between formats
//						using intermediate BMP8 or BMP24 translations when necessary.
//						Currently any standard format can be converted to any other
//						standard format with the exception that 32, 24, or 16 bit 
//						images cannot be converted to any of the 8 bit formats due
//						to the lack of a good color reduction algorithm.  
//
// 01/15/96 JMI	Changed the array of conversion functions to a dynamic
//						linking mechanism of ConvertTo and ConvertFrom function
//						pointers so that new conversion functions can be added in
//						.cpp files other than imagecon.cpp.  
//
//	01/16/96	BRH	Added calls to RImage::GetPitch(width,depth) to calculate
//						the 128-bit aligned pitch for converted image types.  
//						previously, several of the conversion functions forgot
//						to change the pitch for the new format.
//
// 01/19/96 BRH	Fixed a bug in creating new buffers.  Most calls were
//						incorrectly using width*height*(depth/8) where they should
//						have been using pitch instead of width.
//
// 02/27/95	BRH	Changed over from using DYNALINK to the RImageSpecialFunc
//						class for Conversion functions.  The new class is based on
//						the same concept as DYNALINK but is specialized for RImage
//						in that it supports 6 special function types for conversion
//						to/from, load/save, and alloc/delete.  I moved the instantiation
//						of the static arrays from imagecon.cpp to image.cpp since
//						it now instantiates 6 arrays, 4 of which are not conversion
//						related.  Then I changed the calls to LINKLATE macros (DYNALINK)
//						to IMAGELINKLATE macros (RImageSpecialFunc).
//
//	07/22/96	JMI	Added ConvertFromBMP8RLE to allow LoadDib to load RLE8
//						compressed DIBs.  BMP8RLE format can be used as any other
//						image format.  Will implement ConvertToBMP8RLE soon.
//
//	07/23/96	JMI	Added ConvertToBMP8RLE to allow one to convert into
//						BMP8RLE (RLE8 Windows' bitmap compressed format).  The convert
//						flips upside down as it compresses making it official
//						as far as BMP files go.  SaveDib is aware of this use of 
//						RLE8 and can save these preserving compression.
//						Also, made ConvertFromBMP8RLE flip the image so you get
//						a DIB that is upside down as far as DIBs go, but right side
//						up to the naked eye.  This is functionality was implemented
//						since LoadDib does this as well.
//
//	08/04/96 MJR	Commented-out the "pImage" parameter to ConvertNoSupport()
//						to avoid compiler warning about "unused variable".
//						Also modified ConvertToSystem() to use WIN32 instead of
//						_WINDOWS and to use MAC instead of _MAC.  However, note that
//						ConvertToSystem() still does nothing on the mac.
//
//	09/04/96	JMI	Added BMP1 IMAGELINKLATE's and corresponding ConvertToBMP1()
//						and ConvertFromBMP1().  BMP8 is, as usual, the only type 
//						convertible to BMP1.
//
//	10/10/96	JMI	ConvertToBMP24() was not checking for error return from
//						CreateData() and so it was crashing when memory was
//						low.
//
//	10/30/96	JMI	Changed:
//						Old label:		New label:
//						=========		=========
//						RImage			RImage
//						RPal				RPal
//						ULONG ulType	RImage::Type ulType
//						
//						Removed #include of "imagetyp.h" b/c it has been obsoleted.
//						Removed #include of "imagecon.h" b/c all it does is include
//						image.h and then redeclare RImage (now RImage anyway) and
//						replaced with #include of image.h.
//
//	11/01/96	JMI	Changed all members of image to be preceded by m_ (e.g.,
//						sDepth to m_sDepth).  Changed all position members (i.e., 
//						lWidth, lHeight, lBufferWidth, lBufferHeight, lXPos, & lYPos)
//						to be shorts (i.e., m_sWidth, m_sHeight, m_sBufferWidth,
//						m_sBufferHeight, m_sXPos, m_sYPos).  Changed ulType to
//						m_type and ulDestinationType to m_typeDestination.
//
//	11/06/96 MJR	Added "RImage::: before many instances of BMP8, BMP24, etc.
//						This was suddenly required because the IMAGELINKLATE macro
//						no longer used the "using" syntax which had, as a side-effect,
//						allowed the use of the enums without a preceeding "RImage::".				
//
//	11/08/96	JMI	Added more "RImage::"s where necessary in the #ifdef WIN32
//						block.
//
//	11/08/96	JMI	Changed reference of BMP8RLE to RImage::BMP8RLE in an ASSERT.
//
// Standard Converters:
//
// Buffer Conversions:
//
// BMP8 (8-bit buffer 32-bit color pal)	-> 16-bit buffer (555)
// BMP8 (8-bit buffer 32-bit color pal)	-> 16-bit buffer (565)
// BMP8 (8-bit buffer 32-bit color pal)	-> 24-bit buffer (888)	RGB
// BMP8 (8-bit buffer 32-bit color pal)	-> 32-bit buffer (8888) ARGB
// BMP24 (24-bit buffer)						-> 16-bit buffer (555)
// BMP24 (24-bit buffer)						-> 16-bit buffer (565)
// BMP24 (24-bit buffer)						-> 32-bit buffer (8888) ARGB
//
// Palette Conversions:
//
// BMP8 (IM_RGBQUAD)	-> 555 Palette
// BMP8 (IM_RGBQUAD) -> 565 Palette
// BMP8 (IM_RGBQUAD) -> 888 Palette
// BMP8 (IM_RGBQUAD) -> SYSTEM Palette (for setting colors on a platform
//							basis) Windows = IM_RGBQUAD, Mac = Mac System Pal
//							
//
//////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <string.h>

#include "Image.h"

//////////////////////////////////////////////////////////////////////
// Prototypes.
//////////////////////////////////////////////////////////////////////
short	ConvertNoSupport(RImage* pImage);
short	ConvertToBMP8(RImage* pImage);
short	ConvertToBMP24(RImage* pImage);
short	ConvertToSystem(RImage* pImage);	
short	ConvertToSCREEN8_555(RImage* pImage);
short ConvertToSCREEN8_565(RImage* pImage);
short ConvertToSCREEN8_888(RImage* pImage);
short ConvertToSCREEN16_555(RImage* pImage);
short ConvertToSCREEN16_565(RImage* pImage);
short ConvertToSCREEN24_RGB(RImage* pImage);
short ConvertToSCREEN32_ARGB(RImage* pImage);

IMAGELINKLATE(BMP8, ConvertToBMP8, NULL, NULL, NULL, NULL, NULL);
IMAGELINKLATE(SYSTEM8, ConvertToSystem, NULL, NULL, NULL, NULL, NULL);
IMAGELINKLATE(SCREEN8_555, ConvertToSCREEN8_555, NULL, NULL, NULL, NULL, NULL);
IMAGELINKLATE(SCREEN8_565, ConvertToSCREEN8_565, NULL, NULL, NULL, NULL, NULL);
IMAGELINKLATE(SCREEN8_888, ConvertToSCREEN8_888, NULL, NULL, NULL, NULL, NULL);
IMAGELINKLATE(BMP24, ConvertToBMP24, NULL, NULL, NULL, NULL, NULL);
IMAGELINKLATE(SCREEN16_555, ConvertToSCREEN16_555, NULL, NULL, NULL, NULL, NULL);
IMAGELINKLATE(SCREEN16_565, ConvertToSCREEN16_565, NULL, NULL, NULL, NULL, NULL);
IMAGELINKLATE(SCREEN24_RGB, ConvertToSCREEN24_RGB, NULL, NULL, NULL, NULL, NULL);
IMAGELINKLATE(SCREEN32_ARGB, ConvertToSCREEN32_ARGB, NULL, NULL, NULL, NULL, NULL);

//////////////////////////////////////////////////////////////////////
// More non-standard, always supported types:
//////////////////////////////////////////////////////////////////////

// BMP compression format (BMP8RLE).
short ConvertToBMP8RLE(RImage* pImage);
short ConvertFromBMP8RLE(RImage* pImage);
// Link in conversion functions.
IMAGELINKLATE(BMP8RLE, ConvertToBMP8RLE, ConvertFromBMP8RLE, NULL, NULL, NULL, NULL);

// Monochrome BMP (BMP1).
short ConvertToBMP1(RImage* pImage);
short ConvertFromBMP1(RImage* pImage);
// Link in conversion functions.
IMAGELINKLATE(BMP1, ConvertToBMP1, ConvertFromBMP1, NULL, NULL, NULL, NULL);

//////////////////////////////////////////////////////////////////////
//
// ConvertNoSupport:
//
//	Catch function in case somebody passes in a format that is not
// supported.  This function will just return RImage::NOT_SUPPORTED
//
//////////////////////////////////////////////////////////////////////

short ConvertNoSupport(RImage* /*pImage*/)
{
	TRACE("RImage::Convert - Conversion not supported - Invalid image type\n");
	return RImage::NOT_SUPPORTED;
}

//////////////////////////////////////////////////////////////////////
//
// ConvertToBMP8
//
// For now this conversion is not supported.  RImage::BMP8 is a standard
// load format so it is not apparent at this time why one couldn't
// simply use the load function to obtain a RImage::BMP8 format.  We don't 
// see the need to convert from one of the other formats into RImage::BMP8 
// but if it becomes necessary or convenient then it can be supported
//
//////////////////////////////////////////////////////////////////////

short	ConvertToBMP8(RImage* pImage)
{
	short sReturn;

	switch (pImage->m_type)
	{
		case RImage::BMP8:
		{
			sReturn = RImage::BMP8;
			break;
		}

		case RImage::SYSTEM8:
		{
			#ifdef _WINDOWS
				pImage->m_type = RImage::BMP8;
				pImage->m_pPalette->m_type = RPal::PDIB;
				return RImage::BMP8;
			#endif
			break;
		}

		case RImage::SCREEN8_555:
		{
			// Make a copy of the original palette
			RPal* p555Pal = new RPal();
			*p555Pal = *pImage->m_pPalette;

			// Detach the original palette data from the Image's palette
			p555Pal->m_pData = pImage->m_pPalette->DetachData();

			// Create new palette data for the pImage
			pImage->m_pPalette->CreateData((p555Pal->m_sStartIndex + p555Pal->m_sNumEntries) * RPal::GetPalEntrySize(RPal::PDIB));
			pImage->m_pPalette->m_type = RPal::PDIB;
			pImage->m_pPalette->m_sPalEntrySize = RPal::GetPalEntrySize(RPal::PDIB);

			USHORT* usp555 = (USHORT*) p555Pal->m_pData;
			ULONG* ulpDib = (ULONG*) pImage->m_pPalette->m_pData;

			short i;
		
			// The 555 viewed as USHORT as xR5|G5|B5
			//         viewed as 2 BYTES   G3|B5|x|R5|G2
			// The DIB viewed as ULONG is Reserved|R|G|B
			//         viewed as 4 BYTES  B|G|R|Reserved

			for (i = p555Pal->m_sStartIndex; i < p555Pal->m_sStartIndex + p555Pal->m_sNumEntries; i++)
				ulpDib[i] = (ULONG) (((usp555[i] & 0x7c00) << 9) + // Red
				                     ((usp555[i] & 0x03e0) << 6) + // Green
											((usp555[i] & 0x001f) << 3)); // Blue

			delete p555Pal;

			pImage->m_type = RImage::BMP8;
			sReturn = RImage::BMP8;
			break;
		}

		case RImage::SCREEN8_565:
		{
			// Make a copy of the original palette
			RPal* p565Pal = new RPal();
			*p565Pal = *pImage->m_pPalette;

			// Detach the original palette data from the Image's palette
			p565Pal->m_pData = pImage->m_pPalette->DetachData();

			// Create new palette data for the pImage
			pImage->m_pPalette->CreateData((p565Pal->m_sStartIndex + p565Pal->m_sNumEntries) * RPal::GetPalEntrySize(RPal::PDIB));
			pImage->m_pPalette->m_type = RPal::PDIB;
			pImage->m_pPalette->m_sPalEntrySize = RPal::GetPalEntrySize(RPal::PDIB);

			USHORT* usp565 = (USHORT*) p565Pal->m_pData;
			ULONG* ulpDib = (ULONG*) pImage->m_pPalette->m_pData;

			short i;
		
			// The 565 when viewed as USHORT  R5|G6|B5
			//              viewed as 2 BYTES G3|B5|R5|G3
			// The DIB when viewed as ULONG   Reserved|R|G|B
			//              viewed as 4 BYTES B|G|R|Reserved

			for (i = p565Pal->m_sStartIndex; i < p565Pal->m_sStartIndex + p565Pal->m_sNumEntries; i++)
				ulpDib[i] = (ULONG) (((usp565[i] & 0xf800) << 8) + // Red
				                     ((usp565[i] & 0x07e0) << 5) + // Green
											((usp565[i] & 0x001f) << 3)); // Blue

			delete p565Pal;

			pImage->m_type = RImage::BMP8;
			sReturn = RImage::BMP8;
			break;
		}

		case RImage::SCREEN8_888:
		{
			// Make a copy of the original palette
			RPal* p888Pal = new RPal();
			*p888Pal = *pImage->m_pPalette;

			// Detach the original palette data form the Image's palette
			p888Pal->m_pData = pImage->m_pPalette->DetachData();

			// Create new palette data for the pImage
			pImage->m_pPalette->CreateData((p888Pal->m_sStartIndex + p888Pal->m_sNumEntries) * sizeof(IM_RGBQUAD));
			pImage->m_pPalette->m_type = RPal::PDIB;
			pImage->m_pPalette->m_sPalEntrySize = RPal::GetPalEntrySize(RPal::PDIB);

			IM_RGBTRIPLE* tp888 = (IM_RGBTRIPLE*) p888Pal->m_pData;
			IM_RGBQUAD* qpDib = (IM_RGBQUAD*) pImage->m_pPalette->m_pData;
		
			short i;

			// The DIB format when viewed as a IM_RGBQUAD = B|G|R|Reserved
			// The 888 palette     viewed as IM_RGBTRIPLE   B|G|R

			for (i = p888Pal->m_sStartIndex; i < p888Pal->m_sStartIndex + p888Pal->m_sNumEntries; i++)
			{
			 	qpDib[i].rgbBlue = tp888[i].rgbtBlue;
				qpDib[i].rgbGreen = tp888[i].rgbtGreen;
				qpDib[i].rgbRed = tp888[i].rgbtRed;
				qpDib[i].rgbReserved = 0;
			}

			delete p888Pal;

			pImage->m_type = RImage::BMP8;
			sReturn = RImage::BMP8;
			break;
		}
	
		default:
		{
			sReturn = RImage::NOT_SUPPORTED;
			break;
		}
	}

	return sReturn;
}

//////////////////////////////////////////////////////////////////////
//
// ConvertToBMP24
//
// For now this conversion is not supported.  BMP24 is a standard
// load format which would normally either be used as BMP24 after 
// the load or converted to one of the other image formats.
//
//////////////////////////////////////////////////////////////////////

short	ConvertToBMP24(RImage* pImage)
{
	short sReturn;

	switch (pImage->m_type)
	{
		case RImage::BMP24:
		{
		 	sReturn = RImage::BMP24;
			break;
		}

		case RImage::BMP8:
		{
			ConvertToSCREEN24_RGB(pImage);
			pImage->m_type = RImage::BMP24;
			sReturn = RImage::BMP24;
			break;
		}

		case RImage::SCREEN8_555:
		case RImage::SCREEN8_565:
		case RImage::SCREEN8_888:
		case RImage::SYSTEM8:
		{
		 	if (ConvertToBMP8(pImage) == RImage::BMP8)
				sReturn = ConvertToBMP24(pImage);
			else
			{
			 	TRACE("RImage::Convert - BMP24: Intermediate RImage::BMP8 translation error\n");
				sReturn = RImage::NOT_SUPPORTED;
			}
			break;
		}


		case RImage::SCREEN24_RGB:
		{
		 	pImage->m_type = RImage::BMP24;
			sReturn = RImage::BMP24;
			break;
		}

		case RImage::SCREEN32_ARGB:
		{
			long r, c, height, width, sPitch;

			// Make sure that we copy the whole thing even if the
			// height and width are negative values
			height = labs((long)pImage->m_sHeight);
			width = labs((long)pImage->m_sWidth);
			sPitch = pImage->m_lPitch;

			// Set a pointer to the 32-bit buffer before detaching
			ULONG* ulp32 = (ULONG*) pImage->m_pData;

			// Detach the 32-bit buffer from the Image
			void* pDetachedMem = pImage->DetachData();

			// Create a new 24-bit buffer and set a pointer to the data
			pImage->m_sDepth = 24;
			pImage->m_lPitch = RImage::GetPitch(pImage->m_sWidth, pImage->m_sDepth);
			if (pImage->CreateData(pImage->m_lPitch * (long)pImage->m_sHeight) == 0)
				{
				UCHAR* ucp24 = (UCHAR*) pImage->m_pData;

				long sPitchWidth = sPitch / 4;
				for (r = 0; r < height; r++)
					for (c = 0; c < width; c++)
					{
						ULONG ulPixel = ulp32[r*sPitchWidth + c];
						ucp24[r*pImage->m_lPitch + 3*c+0]	= (UCHAR)  (ulPixel & 0x000000ff);       // Red
						ucp24[r*pImage->m_lPitch + 3*c+1]	= (UCHAR) ((ulPixel & 0x0000ff00) >> 8); // Green
				 		ucp24[r*pImage->m_lPitch + 3*c+2]	= (UCHAR) ((ulPixel & 0x00ff0000) >> 16);// Blue
					}
  		
				RImage::DestroyDetachedData(&pDetachedMem);

				pImage->m_type = RImage::BMP24;
				sReturn = RImage::BMP24;
				}
			else
				{
				// Restore memory.
				pImage->m_pMem		= (UCHAR*)pDetachedMem;
				pImage->m_pData	= (UCHAR*)ulp32;
				TRACE("ConvertToBMP24(): CreateData() failed.\n");
				// Return old type.
				sReturn = (short)pImage->m_type;
				}
			break;
		}

		case RImage::SCREEN16_555:
		{
			long r, c, height, width, sPitch;

			// Make sure that we copy the whole thing even if the
			// height and width are negative values
			height = labs((long)pImage->m_sHeight);
			width = labs((long)pImage->m_sWidth);
			sPitch = pImage->m_lPitch;

	  		// Set up a pointer to the 16-bit buffer before detaching
			USHORT* usp16 = (USHORT*) pImage->m_pData;

			// Detach old 16-bit buffer from the Image
			void* pDetachedMem = pImage->DetachData();

			// Create a new 24-bit buffer and set a pointer to the data
			pImage->m_sDepth = 24;
			pImage->m_lPitch = RImage::GetPitch(pImage->m_sWidth, pImage->m_sDepth);
			pImage->CreateData(pImage->m_lPitch * (long)pImage->m_sHeight);
			UCHAR* ucp24 = (UCHAR*) pImage->m_pData;

			long sPitchWidth = sPitch/2;
			for (r = 0; r < height; r++)
				for (c = 0; c < width; c++)
				{
				 	USHORT usPixel = usp16[r*sPitchWidth + c];
					ucp24[r*pImage->m_lPitch + 3*c+2] = (UCHAR) ((usPixel & 0x7c00) >> 7); // Red
					ucp24[r*pImage->m_lPitch + 3*c+1] = (UCHAR) ((usPixel & 0x03e0) >> 2); // Green
					ucp24[r*pImage->m_lPitch + 3*c+0] = (UCHAR) ((usPixel & 0x001f) << 3); // Blue
				}
			 	
			RImage::DestroyDetachedData(&pDetachedMem);

			pImage->m_type = RImage::BMP24;
			sReturn = RImage::BMP24;
			break;
		}

		case RImage::SCREEN16_565:
		{
			long r, c, height, width, sPitch;

			// Make sure that we copy the whole thing even if the
			// height and width are negative values
			height = labs((long)pImage->m_sHeight);
			width = labs((long)pImage->m_sWidth);
			sPitch = pImage->m_lPitch;

			// Set a pointer to the 16-bit buffer before detaching
			USHORT* usp16 = (USHORT*) pImage->m_pData;

			// Detach the 16-bit buffer from the Image
			void* pDetachedMem = pImage->DetachData();

			// Create a new 24-bit buffer and set a pointer to the data
			pImage->m_sDepth = 24;
			pImage->m_lPitch = RImage::GetPitch(pImage->m_sWidth, pImage->m_sDepth);
			pImage->CreateData(pImage->m_lPitch * (long)pImage->m_sHeight);
			UCHAR* ucp24 = (UCHAR*) pImage->m_pData;
			long sPitchWidth = sPitch/2;

			for (r = 0; r < height; r++)
				for (c = 0; c < width; c++)
				{
				 	USHORT usPixel = usp16[r*sPitchWidth + c];
					ucp24[r*pImage->m_lPitch + 3*c+2] = (UCHAR) ((usPixel & 0xf800) >> 8); // Red
					ucp24[r*pImage->m_lPitch + 3*c+1] = (UCHAR) ((usPixel & 0x07e0) >> 3); // Green
					ucp24[r*pImage->m_lPitch + 3*c+0] = (UCHAR) ((usPixel & 0x001f) << 3); // Blue
				}

			RImage::DestroyDetachedData(&pDetachedMem);

			pImage->m_type = RImage::BMP24;
			sReturn = RImage::BMP24;
			break;
		}

		default:
		{
			sReturn = RImage::NOT_SUPPORTED;
			break;
		}	
	}

	return sReturn;
}

//////////////////////////////////////////////////////////////////////
//
// ConvertToSystem
//
// This convert function will depened on what platform it is complied
// on.  Our standard load formats are Windows BMP.  This function for
// Windows will only change the type from RImage::BMP8 to RImage::SYSTEM8.  On a Mac
// it would convert the palette from IM_RGBQUAD to the Mac's system
// palette.
//
//////////////////////////////////////////////////////////////////////

short ConvertToSystem(RImage* pImage)
{
	short sReturn = RImage::NOT_SUPPORTED;

#ifdef WIN32
	switch (pImage->m_type)
	{
		case RImage::SYSTEM8:
		{
		 	sReturn = RImage::SYSTEM8;
			break;
		}			
		
		case RImage::BMP8:
		{
			pImage->m_type = RImage::SYSTEM8;
			pImage->m_pPalette->m_type = RPal::PSYS;
			sReturn = RImage::SYSTEM8;
			break;
		}

		case RImage::SCREEN8_555:
		case RImage::SCREEN8_565:
		case RImage::SCREEN8_888:
		{
		 	if (ConvertToBMP8(pImage) == RImage::BMP8)
				sReturn = ConvertToSystem(pImage);
			else
			{
			 	TRACE("RImage::Convert - RImage::SYSTEM8: Intermediate RImage::BMP8 translation error\n");
				sReturn = RImage::NOT_SUPPORTED;
			}
			break;
		}

		default:
		{
		 	sReturn = RImage::NOT_SUPPORTED;
			break;
		}
	}
#endif

    return sReturn;
}

//////////////////////////////////////////////////////////////////////
//
// ConvertToSCREEN8_555
//
// Converts from RImage::BMP8 to SCREEN8_555 which means that it transforms
// the palette entries from IM_RGBQUAD to the 555 "Screen Format" palette
//
//////////////////////////////////////////////////////////////////////

short	ConvertToSCREEN8_555(RImage* pImage)
{
	short sReturn;

	switch (pImage->m_type)
	{
		case RImage::SCREEN8_555:
		{
		 	sReturn = RImage::SCREEN8_555;
			break;
		}

		case RImage::BMP8:
		{
			// Make a copy of the original palette
			RPal* pDibPal = new RPal();
			*pDibPal = *pImage->m_pPalette;

			// Detach the original palette data from the Image's palette
			pDibPal->m_pData = pImage->m_pPalette->DetachData();

			// Create new palette data for the Image
			pImage->m_pPalette->CreateData((pDibPal->m_sStartIndex + pDibPal->m_sNumEntries) * 2);
			pImage->m_pPalette->m_type = RPal::P555;
			pImage->m_pPalette->m_sPalEntrySize = RPal::GetPalEntrySize(RPal::P555);

			ULONG* ulpDib = (ULONG*) pDibPal->m_pData;
			USHORT* usp555 = (USHORT*) pImage->m_pPalette->m_pData;

			short i;
		
			// The DIB format when viewed as a ULONG is Reserved|R|G|B 
			//                when viewed as 4 BYTES is B|G|R|Reserved
			// Converting to 555 viewed as USHORT as xR5|G5|B5
			//                   viewed as 2 BYTES   G3|B5|x|R5|G2

			for (i = pDibPal->m_sStartIndex; i < pDibPal->m_sStartIndex + pDibPal->m_sNumEntries; i++)
				usp555[i] = (USHORT) (((ulpDib[i] & 0x00f80000) >> 9) +	// Red
							   			 ((ulpDib[i] & 0x0000f800) >> 6) +	// Green
											 ((ulpDib[i] & 0x000000f8) >> 3));	// Blue

			delete pDibPal;

			pImage->m_type = RImage::SCREEN8_555;
			sReturn = RImage::SCREEN8_555;
			break;
		}

		case RImage::SYSTEM8:
		case RImage::SCREEN8_565:
		case RImage::SCREEN8_888:
		{
		 	if (ConvertToBMP8(pImage) == RImage::BMP8)
				sReturn = ConvertToSCREEN8_555(pImage);
			else
			{
			 	TRACE("RImage::Convert - SCREEN8_555 Intermediate RImage::BMP8 translation error\n");
				sReturn = RImage::NOT_SUPPORTED;
			}
		}

		default:
		{
		 	sReturn = RImage::NOT_SUPPORTED;
			break;
		}

	}	
	return sReturn;
}

//////////////////////////////////////////////////////////////////////
//
// ConvertToSCREEN8_565
//
// Converts from RImage::BMP8 to SCREEN8_565 which means that it transforms 
// the palette entries from IM_RGBQUAD to the 565 "Screen Format" palette
//
//////////////////////////////////////////////////////////////////////

short ConvertToSCREEN8_565(RImage* pImage)
{
	short sReturn;

	switch (pImage->m_type)
	{
		case RImage::SCREEN8_565:
		{
		 	sReturn = RImage::SCREEN8_565;
			break;
		}

		case RImage::BMP8:
		{
			// Make a copy of the original palette
			RPal* pDibPal = new RPal();
			*pDibPal = *pImage->m_pPalette;

			// Detach the original palette data from the Image's palette
			pDibPal->m_pData = pImage->m_pPalette->DetachData();

			// Create new palette data for the pImage
			pImage->m_pPalette->CreateData((pDibPal->m_sStartIndex + pDibPal->m_sNumEntries) * 2);
			pImage->m_pPalette->m_type = RPal::P565;
			pImage->m_pPalette->m_sPalEntrySize = RPal::GetPalEntrySize(RPal::P565);

			ULONG* ulpDib = (ULONG*) pDibPal->m_pData;
			USHORT* usp565 = (USHORT*) pImage->m_pPalette->m_pData;

			short i;
		
			// The DIB format when viewed as a ULONG is Reserved|R|G|B 
			//                when viewed as 4 BYTES is B|G|R|Reserved
			// Converting to 565 viewed as USHORT as R5|G6|B5
			//                   viewed as 2 BYTES   G3|B5|R5|G3

			for (i = pDibPal->m_sStartIndex; i < pDibPal->m_sStartIndex + pDibPal->m_sNumEntries; i++)
				usp565[i] = (USHORT) (((ulpDib[i] & 0x00f80000) >> 8) +	// Red
							   			 ((ulpDib[i] & 0x0000fc00) >> 5) +	// Green
											 ((ulpDib[i] & 0x000000f8) >> 3));	// Blue

			delete pDibPal;

			pImage->m_type = RImage::SCREEN8_565;
			sReturn = RImage::SCREEN8_565;
			break;
		}

		case RImage::SYSTEM8:
		case RImage::SCREEN8_555:
		case RImage::SCREEN8_888:
		{
		 	if (ConvertToBMP8(pImage) == RImage::BMP8)
				sReturn = ConvertToSCREEN8_565(pImage);
			else
			{
			 	TRACE("RImage::Convert - SCREEN8_565 Intermediate RImage::BMP8 translation error\n");
				sReturn = RImage::NOT_SUPPORTED;
			}
			break;
		}

		default:
		{
		 	sReturn = RImage::NOT_SUPPORTED;
			break;
		}
	
	}
	return sReturn;
}

//////////////////////////////////////////////////////////////////////
//
// ConvertToSCREEN8_888
//
// Converts from RImage::BMP8 to SCREEN8_888 which means that it transforms
// the palette entries from IM_RGBQUAD to the 888 "Screen Format" palette
//
// The normal RImage::BMP8 DIB palette
//		when viewed as ULONG is Reserved|R|G|B
//
// And the screen format is the same, so just change the image type
//
//////////////////////////////////////////////////////////////////////

short ConvertToSCREEN8_888(RImage* pImage)
{
	short sReturn;

	switch(pImage->m_type)
	{
		case RImage::SCREEN8_888:
		{
		 	sReturn = RImage::SCREEN8_888;
			break;
		}

		case RImage::BMP8:
		{
			RPal* pDibPal = new RPal();
			*pDibPal = *pImage->m_pPalette;

			// Detach the original palette data from the Image's palette
			pDibPal->m_pData = pImage->m_pPalette->DetachData();

			// Create new palette data for the pImage
			pImage->m_pPalette->CreateData((pDibPal->m_sStartIndex + pDibPal->m_sNumEntries) * sizeof(IM_RGBTRIPLE));
			pImage->m_pPalette->m_type = RPal::P888;
			pImage->m_pPalette->m_sPalEntrySize = RPal::GetPalEntrySize(RPal::P888);

			IM_RGBQUAD* qpDib = (IM_RGBQUAD*) pDibPal->m_pData;
			IM_RGBTRIPLE* tp888 = (IM_RGBTRIPLE*) pImage->m_pPalette->m_pData;

			short i;

			// The DIB format when viewed as a IM_RGBQUAD = B|G|R|Reserved
			// Converting to 888 viewed as IM_RGBTRIPLE as  B|G|R

			for (i = pDibPal->m_sStartIndex; i < pDibPal->m_sStartIndex + pDibPal->m_sNumEntries; i++)
			{
				tp888[i].rgbtBlue = qpDib[i].rgbBlue;
				tp888[i].rgbtGreen = qpDib[i].rgbGreen;
				tp888[i].rgbtRed = qpDib[i].rgbRed;
			}

			delete pDibPal;

			pImage->m_type = RImage::SCREEN8_888;
			sReturn = RImage::SCREEN8_888;
			break;
		}

		case RImage::SYSTEM8:
		case RImage::SCREEN8_555:
		case RImage::SCREEN8_565:
		{
			if (ConvertToBMP8(pImage) == RImage::BMP8)
				sReturn = ConvertToSCREEN8_888(pImage);
			else
			{
			 	TRACE("RImage::Convert - SCREEN8_888 Intermediate RImage::BMP8 translation error\n");
				sReturn = RImage::NOT_SUPPORTED;
			} 	
			break;
		}

		default:
		{
		 	sReturn = RImage::NOT_SUPPORTED;
			break;
		}
	}
		return sReturn;
}

//////////////////////////////////////////////////////////////////////
//
// ConvertToSCREEN16_555
//
// Converts from RImage::BMP8 or BMP24 to SCREEN16_555.  For RImage::BMP8 it creates
// a 16-bit image buffer and then dereferences the color palette for
// each pixel value in the 8-bit buffer.  For BMP24 it creates a 16-bit
// image buffer and converts the 24-bit buffer to a 16-bit image.
//
//////////////////////////////////////////////////////////////////////

short ConvertToSCREEN16_555(RImage* pImage)
{
	short sReturn = RImage::NOT_SUPPORTED;

	switch (pImage->m_type)
	{
		case RImage::SCREEN16_555:
		{
		 	sReturn = RImage::SCREEN16_555;
			break;
		}

		case RImage::BMP8:
		{
			long r, c, height, width, pitch;

			// Make sure that we copy the whole thing even if the
			// height and width are negative values

			height = labs((long)pImage->m_sHeight);
			width = labs((long)pImage->m_sWidth);
			pitch = pImage->m_lPitch;

			// Set up pointers to the 8-bit buffer before detaching
			UCHAR* ucp8 = pImage->m_pData;
			ULONG* ulpPal = (ULONG*) pImage->m_pPalette->m_pData;

			// Detach the 8-bit buffer from the Image
			void* pDetachedMemory = pImage->DetachData();

			// Create a new 16-bit buffer
			pImage->m_sDepth = 16;
			pImage->m_lPitch = RImage::GetPitch(pImage->m_sWidth, pImage->m_sDepth);
			pImage->CreateData(pImage->m_lPitch * (short)pImage->m_sHeight);
			USHORT* usp16 = (USHORT*) pImage->m_pData;
			UCHAR  ucIndex;
			long dPitch = pImage->m_lPitch / (pImage->m_sDepth / 8);

			for (r = 0; r < height; r++)
				for (c = 0; c < width; c++)
				{
					ucIndex = ucp8[r*pitch + c];
					usp16[r*dPitch + c] = (USHORT) (((ulpPal[ucIndex] & 0x00f80000) >> 9) + // Red
					                                ((ulpPal[ucIndex] & 0x0000f800) >> 6) + // Green
															  ((ulpPal[ucIndex] & 0x000000f8) >> 3)); // Blue
				}

			RImage::DestroyDetachedData(&pDetachedMemory);
			pImage->DestroyPalette();		

			pImage->m_type = RImage::SCREEN16_555;
			sReturn = RImage::SCREEN16_555;
			break;
		}

		case RImage::BMP24:
		{
			long r, c, height, width, sPitch;

			// Make sure that we copy the whole thing even if the
			// height and width are negative values
			height = labs((long)pImage->m_sHeight);
			width = labs((long)pImage->m_sWidth);
			sPitch = pImage->m_lPitch;

	  		// Set up a pointer to the 24-bit buffer before detaching
			UCHAR* ucp24 = (UCHAR*) pImage->m_pData;

			// Detach old 24-bit buffer from the Image
			void* pDetachedMem = pImage->DetachData();

			// Create a new 16-bit buffer and set a pointer to the data
			pImage->m_sDepth = 16;
			pImage->m_lPitch = RImage::GetPitch(pImage->m_sWidth, pImage->m_sDepth);
			pImage->CreateData(pImage->m_lPitch * (long)pImage->m_sHeight);
			USHORT* usp16 = (USHORT*) pImage->m_pData;
			long dPitch = pImage->m_lPitch / (pImage->m_sDepth / 8);

			for (r = 0 ; r < height; r++)
				for (c = 0; c < width; c++)
				 	usp16[r*dPitch + c] = (USHORT) (
				 								 ((((USHORT) ucp24[r*sPitch + 3*c+2]) & 0x00f8) << 7) + // Red
												 ((((USHORT) ucp24[r*sPitch + 3*c+1]) & 0x00f8) << 2) + // Green
												 ((((USHORT) ucp24[r*sPitch + 3*c+0]) & 0x00f8) >> 3)); // Blue
			 	
			RImage::DestroyDetachedData(&pDetachedMem);

			pImage->m_type = RImage::SCREEN16_555;
		 	sReturn = RImage::SCREEN16_555;
			break;
		}

		case RImage::SYSTEM8:
		case RImage::SCREEN8_555:
		case RImage::SCREEN8_565:
		case RImage::SCREEN8_888:
		{
			if (ConvertToBMP8(pImage) == RImage::BMP8)
				sReturn = ConvertToSCREEN16_555(pImage);
			else
			{
			 	TRACE("RImage::Convert - SCREEN16_555 Intermediate RImage::BMP8 translation error\n");
				sReturn = RImage::NOT_SUPPORTED;
			}
			break;
		}

		case RImage::SCREEN16_565:
		case RImage::SCREEN24_RGB:
		case RImage::SCREEN32_ARGB:
		{
			if (ConvertToBMP24(pImage) == RImage::BMP24)
				sReturn = ConvertToSCREEN16_555(pImage);
			else
			{
				TRACE("RImage::Convert - SCREEN16_555 Intermediate BMP24 translation error\n");
				sReturn = RImage::NOT_SUPPORTED;
			}	
			break;
		}

		default:
		{
		 	sReturn = RImage::NOT_SUPPORTED;
			break;
		}
	}
	return sReturn;
}

//////////////////////////////////////////////////////////////////////
//
// ConvertToSCREEN16_565
//
// Converts from RImage::BMP8 or BMP24 to SCREEN16_565.  For RImage::BMP8 it creates
// a 16-bit image buffer and then dereferences the color palette for
// each pixel value in the 8-bit buffer.  For BMP24 it creates a 16-bit
// image buffer and converts the 24-bit buffer to a 16-bit image.
//
//////////////////////////////////////////////////////////////////////

short ConvertToSCREEN16_565(RImage* pImage)
{
 	short sReturn = RImage::NOT_SUPPORTED;

	switch (pImage->m_type)
	{
		case RImage::SCREEN16_565:
		{
		 	sReturn = RImage::SCREEN16_565;
			break;
		}

		case RImage::BMP8:
		{
			long r, c, height, width, pitch;

			// Make sure that we copy the whole thing even if the
			// height and width are negative values

			height = labs((long)pImage->m_sHeight);
			width = labs((long)pImage->m_sWidth);
			pitch = pImage->m_lPitch;

			// Set up a pointer to the old 8-bit buffer before detaching
			UCHAR* ucp8 = pImage->m_pData;
			ULONG* ulpPal = (ULONG*) pImage->m_pPalette->m_pData;

			// Detach the 8-bit buffer from the Image
			void* pDetachedMem = pImage->DetachData();

			// Create a new 16-bit buffer and set pointers to the data
			pImage->m_sDepth = 16;
			pImage->m_lPitch = RImage::GetPitch(pImage->m_sWidth, pImage->m_sDepth);
			pImage->CreateData(pImage->m_lPitch * (long)pImage->m_sHeight);
			USHORT* usp16 = (USHORT*) pImage->m_pData;
			UCHAR  ucIndex;
			long dPitch = pImage->m_lPitch / (pImage->m_sDepth/8);

			for (r = 0; r < height; r++)
				for (c = 0; c < width; c++)
				{
					ucIndex = ucp8[r*pitch + c];
					usp16[r*dPitch + c] = (USHORT) (((ulpPal[ucIndex] & 0x00f80000) >> 8) + // Red
					                                ((ulpPal[ucIndex] & 0x0000fc00) >> 5) + // Green
											   			  ((ulpPal[ucIndex] & 0x000000f8) >> 3)); // Blue
				}

			RImage::DestroyDetachedData(&pDetachedMem);
			pImage->DestroyPalette();

			pImage->m_type = RImage::SCREEN16_565;
			sReturn = RImage::SCREEN16_565;
			break;
		}

		case RImage::BMP24:
		{
			long r, c, height, width, sPitch;

			// Make sure that we copy the whole thing even if the
			// height and width are negative values
			height = labs((long)pImage->m_sHeight);
			width = labs((long)pImage->m_sWidth);
			sPitch = pImage->m_lPitch;

			// Set a pointer to the 24-bit buffer before detaching
			UCHAR* ucp24 = (UCHAR*) pImage->m_pData;

			// Detach the 24-bit buffer from the Image
			void* pDetachedMem = pImage->DetachData();

			// Create a new 16-bit buffer and set a pointer to the data
			pImage->m_sDepth = 16;
			pImage->m_lPitch = RImage::GetPitch(pImage->m_sWidth, pImage->m_sDepth);
			pImage->CreateData(pImage->m_lPitch * (long)pImage->m_sHeight);
			USHORT* usp16 = (USHORT*) pImage->m_pData;
			long dPitch = pImage->m_lPitch / (pImage->m_sDepth / 8);

			for (r = 0 ; r < height; r++)
				for (c = 0; c < width; c++)  
					usp16[r*dPitch + c] = (USHORT) (
													((((USHORT) ucp24[r*sPitch + 3*c+2]) & 0x00f8) << 8) + // Red
													((((USHORT) ucp24[r*sPitch + 3*c+1]) & 0x00fc) << 3) + // Green
													((((USHORT) ucp24[r*sPitch + 3*c+0]) & 0x00f8) >> 3)); // Blue
  	
			RImage::DestroyDetachedData(&pDetachedMem);

			pImage->m_type = RImage::SCREEN16_565;
			sReturn = RImage::SCREEN16_565;
			break;
		}
	 	
		case RImage::SYSTEM8:
		case RImage::SCREEN8_555:
		case RImage::SCREEN8_565:
		case RImage::SCREEN8_888:
		{
			if (ConvertToBMP8(pImage) == RImage::BMP8)
				sReturn = ConvertToSCREEN16_565(pImage);
			else
			{
				TRACE("RImage::Convert - SCREEN16_565 Intermediate RImage::BMP8 translation error\n");
				sReturn = RImage::NOT_SUPPORTED;
			}
			break;
		}

		case RImage::SCREEN16_555:
		case RImage::SCREEN24_RGB:
		case RImage::SCREEN32_ARGB:
		{
		 	if (ConvertToBMP24(pImage) == RImage::BMP24)
				sReturn = ConvertToSCREEN16_565(pImage);
			else
			{
			 	TRACE("RImage::Convert - SCREEN16_565 Intermediate BMP24 translation error\n");
				sReturn = RImage::NOT_SUPPORTED;
			}
			break;
		}

		default:
		{
		 	sReturn = RImage::NOT_SUPPORTED;
			break;
		}
	}
	return sReturn;
}


//////////////////////////////////////////////////////////////////////
//
// ConvertToSCREEN24_RGB
//
// Converts from RImage::BMP8 or BMP24 to SCREEN32_RGB.  For RImage::BMP8 it creates
// a 24-bit buffer and then dereferences the color palette for each
// pixel value in the 8-bit buffer.  For BMP24 it is already in the
// correct format and just changes the image type.
//
//////////////////////////////////////////////////////////////////////

short ConvertToSCREEN24_RGB(RImage* pImage)
{
	short sReturn = RImage::NOT_SUPPORTED;

	switch (pImage->m_type)
	{
		case RImage::SCREEN24_RGB:
		{
			sReturn = RImage::SCREEN24_RGB;
			break;
		}	

		case RImage::BMP8:
		{
			long r, c, height, width, sPitch, dPitch;

			// Make sure that we copy the whole thing even if the
			// height and width are negative values

			height = labs((long)pImage->m_sHeight);
			width = labs((long)pImage->m_sWidth);
			sPitch = pImage->m_lPitch;

			// Set up a pointer to the 8-bit buffer before detaching
			UCHAR* ucp8 = pImage->m_pData;
			ULONG* ulPal = (ULONG*) pImage->m_pPalette->m_pData;

			// Detach the 8-bit buffer from the Image
			void* pDetachedMem = pImage->DetachData();

			// Create a new 24-bit buffer and set a pointer to the data
			pImage->m_sDepth = 24;
			dPitch = pImage->m_lPitch = RImage::GetPitch(pImage->m_sWidth, pImage->m_sDepth);
			pImage->CreateData(pImage->m_lPitch * (long)pImage->m_sHeight);
			UCHAR* ucp24 = (UCHAR*) pImage->m_pData;

			ULONG ulColor;

			for (r = 0; r < height; r++)
				for (c = 0; c < width; c++)
				{
					ulColor = ulPal[ucp8[r*sPitch + c]];
					ucp24[r*dPitch + 3*c+0] = (UCHAR) (ulColor & 0x000000ff);
					ucp24[r*dPitch + 3*c+1] = (UCHAR) ((ulColor & 0x0000ff00) / 0x100);
					ucp24[r*dPitch + 3*c+2] = (UCHAR) ((ulColor & 0x00ff0000) / 0x10000);
				}
		
			RImage::DestroyDetachedData(&pDetachedMem);
			pImage->DestroyPalette();

		 	pImage->m_type = RImage::SCREEN24_RGB;
			sReturn = RImage::SCREEN24_RGB;
			break;
		}

		case RImage::BMP24:
		{
			pImage->m_type = RImage::SCREEN24_RGB;
			sReturn = RImage::SCREEN24_RGB;
			break;
		}

		case RImage::SYSTEM8:
		case RImage::SCREEN8_555:
		case RImage::SCREEN8_565:
		case RImage::SCREEN8_888:
		{
			if (ConvertToBMP8(pImage) == RImage::BMP8)
				sReturn = ConvertToSCREEN24_RGB(pImage);
			else
			{
			 	TRACE("RImage::Convert - SCREEN24_RGB Intermediate RImage::BMP8 translation error\n");
				sReturn = RImage::NOT_SUPPORTED;
			}
			break;
		}

		case RImage::SCREEN16_555:
		case RImage::SCREEN16_565:
		case RImage::SCREEN32_ARGB:
		{
			if (ConvertToBMP24(pImage) == RImage::BMP24)
				sReturn = ConvertToSCREEN24_RGB(pImage);
			else
			{
			 	TRACE("RImage::Convert - SCREEN24_RGB Intermediate BMP24 translation error\n");
				sReturn = RImage::NOT_SUPPORTED;
			}
			break;
		}

		default:
		{
		 	sReturn = RImage::NOT_SUPPORTED;
			break;
		}
	}
	return sReturn;
}

//////////////////////////////////////////////////////////////////////
//
// ConvertToSCREEN32_ARGB
//
// Converts from RImage::BMP8 or BMP24 to SCREEN32_RGBA.  For RImage::BMP8 it creates
// a 32-bit buffer and then dereferences the color palette for each
// pixel value in the 8-bit buffer.  For BMP24 it creates a 32-bit
// buffer and then translates the IM_RGBQUAD pixel values into RGBAlpha
// "Screen Format" pixel values.
//
//////////////////////////////////////////////////////////////////////

short ConvertToSCREEN32_ARGB(RImage* pImage)
{
	short sReturn = RImage::NOT_SUPPORTED;

	switch (pImage->m_type)
	{
		case RImage::SCREEN32_ARGB:
		{
		 	sReturn = RImage::SCREEN32_ARGB;
			break;
		}

		case RImage::BMP8:
		{
			long r, c, height, width, pitch;

			// Make sure that we copy the whole thing even if the
			// height and width are negative values

			height = labs((long)pImage->m_sHeight);
			width = labs((long)pImage->m_sWidth);
			pitch = pImage->m_lPitch;

			// Set up a pointer to the 8-bit buffer before detaching
			UCHAR* ucp8 = pImage->m_pData;
			ULONG* ulPal = (ULONG*) pImage->m_pPalette->m_pData;

			// Detach the 8-bit buffer from the Image
			void* pDetachedMem = pImage->DetachData();

			// Create a new 32-bit buffer and set a pointer to the data
			pImage->m_sDepth = 32;
			pImage->m_lPitch = RImage::GetPitch(pImage->m_sWidth, pImage->m_sDepth);
			pImage->CreateData(pImage->m_lPitch * (long)pImage->m_sHeight);
			ULONG* ulp32 = (ULONG*) pImage->m_pData;
			long dLongPitch = pImage->m_lPitch/4;

			for (r = 0; r < height; r++)
				for (c = 0; c < width; c++)
					ulp32[r*dLongPitch + c] = ulPal[ucp8[r*pitch + c]];
		
			RImage::DestroyDetachedData(&pDetachedMem);
			pImage->DestroyPalette();

		 	pImage->m_type = RImage::SCREEN32_ARGB;
			sReturn = RImage::SCREEN32_ARGB;
			break;
		}

		case RImage::BMP24:
		{
			long r, c, height, width, sPitch;

			// Make sure that we copy the whole thing even if the
			// height and width are negative values
			height = labs((long)pImage->m_sHeight);
			width = labs((long)pImage->m_sWidth);
			sPitch = pImage->m_lPitch;

			// Set a pointer to the 24-bit buffer before detaching
			UCHAR* ucp24 = (UCHAR*) pImage->m_pData;

			// Detach the 24-bit buffer from the Image
			void* pDetachedMem = pImage->DetachData();

			// Create a new 32-bit buffer and set a pointer to the data
			pImage->m_sDepth = 32;
			pImage->m_lPitch = RImage::GetPitch(pImage->m_sWidth, pImage->m_sDepth);
			pImage->CreateData(pImage->m_lPitch * (long)pImage->m_sHeight);
			ULONG* ulp32 = (ULONG*) pImage->m_pData;
			long dLongPitch = pImage->m_lPitch/4;

			for (r = 0 ; r < height; r++)
				for (c = 0; c < width; c++)  
					ulp32[r*dLongPitch + c] = (ULONG) (
														((ULONG) ucp24[r*sPitch + 3*c+2] * 0x10000) +
														((ULONG) ucp24[r*sPitch + 3*c+1] * 0x100) +
														((ULONG) ucp24[r*sPitch + 3*c+0])) ;
  	
			RImage::DestroyDetachedData(&pDetachedMem);

			pImage->m_type = RImage::SCREEN32_ARGB;
			sReturn = RImage::SCREEN32_ARGB;
			break;
		}

		case RImage::SYSTEM8:
		case RImage::SCREEN8_555:
		case RImage::SCREEN8_565:
		case RImage::SCREEN8_888:
		{
	 		if (ConvertToBMP8(pImage) == RImage::BMP8)
				sReturn = ConvertToSCREEN32_ARGB(pImage);
			else
			{
			 	TRACE("RImage::Convert - SCREEN32_ARGB Intermediate RImage::BMP8 translation error\n");
				sReturn = RImage::NOT_SUPPORTED;
			}
			break;
		}

		case RImage::SCREEN16_555:
		case RImage::SCREEN16_565:
		case RImage::SCREEN24_RGB:
		{
			if (ConvertToBMP24(pImage) == RImage::BMP24)
				sReturn = ConvertToSCREEN32_ARGB(pImage);
			else
			{
			 	TRACE("RImage::Convert - SCREEN32_ARGB Intermediate BMP24 translation error\n");
				sReturn = RImage::NOT_SUPPORTED;
			}
			break;
		}

		default:
		{
		 	sReturn = RImage::NOT_SUPPORTED;
			break;
		}
	}
	return sReturn;
}

//////////////////////////////////////////////////////////////////////
//
// Converts from the Window's compression format for 8 bit BMPs (RLE8)
// RImage::BMP8RLE to RImage::BMP8 format.  Palette is not touched.
// 
// Description of this process from Win32 documentation:
// This format can be compressed in encoded or absolute modes. Both 
// modes can occur anywhere in the same bitmap. 
// ·	Encoded mode consists of two bytes: the first byte specifies the
// number of consecutive pixels to be drawn using the color index 
// contained in the second byte. In addition, the first byte of the pair 
// can be set to zero to indicate an escape that denotes an end of line, 
// end of bitmap, or delta. The interpretation of the escape depends on 
// the value of the second byte of the pair, which can be one of the 
// following: 
//   
// Value	Meaning
// 
// 0	End of line.
// 1	End of bitmap.
// 2	Delta. The two bytes following the escape contain unsigned values 
// indicating the horizontal and vertical offsets of the next pixel from 
// the current position.
//
// ·	In absolute mode, the first byte is zero and the second byte is a 
// value in the range 03H through FFH. The second byte represents the 
// number of bytes that follow, each of which contains the color index 
// of a single pixel. When the second byte is 2 or less, the escape has 
// the same meaning as in encoded mode. In absolute mode, each run must 
// be aligned on a word boundary. 
//   
//
//////////////////////////////////////////////////////////////////////
short ConvertFromBMP8RLE(RImage* pImage)
	{
	short	sRes	= RImage::NOT_SUPPORTED;	// Assume failure.

	ASSERT(pImage->m_type == RImage::BMP8RLE);

	// Set up a pointer to the 8-bit compressed buffer before detaching
	U8*	pu8Comp	= pImage->m_pData;
	U8*	pu8End	= pu8Comp + pImage->m_ulSize;

	// Detach the 8-bit compressed buffer from the Image
	void* pvDetachedMem = pImage->DetachData();

	// Create a new 8-bit uncompressed buffer and set a pointer to the 
	// data.  Leave the current pitch.
	if (pImage->CreateData(pImage->m_lPitch * (long)pImage->m_sHeight) == 0)
		{
		U8*	pu8Uncomp	= pImage->m_pData;

		// We must flip the image during decompression.
		// This should work for both negative and positive
		// pitch buffers (i.e., upside down and right side up).
		long	lPitch	= -pImage->m_lPitch;
		pu8Uncomp		= pu8Uncomp + pImage->m_lPitch * ((long)pImage->m_sHeight - 1);
		U8*	pu8Row	= pu8Uncomp;

		// Actual decompression.  See function header for details.
		U8	u8Num;	// Num pixels to run.
		U8	u8Pixel;	// Pixel to run.
		short	sDone	= FALSE;
		while (sDone == FALSE && pu8Comp < pu8End)
			{
			// First byte is number of pixels to run, if not 0.
			u8Num	= *pu8Comp++;
			if (u8Num > 0)
				{
				// Second byte is the pixel to run the u8Num pixels.
				u8Pixel	= *pu8Comp++;

				// Intrinsic version of this should be faster than C.
				memset(pu8Uncomp, u8Pixel, u8Num);

				pu8Uncomp += u8Num;
				}
			else
				{
				// The number of bytes is 0, this is an "escape" or 
				// absolute mode.
				// Second byte is the type of escape, unless 3 or more
				// which indicates the number of absolute bytes to copy.
				u8Num	= *pu8Comp++;
				switch (u8Num)
					{
					case 0:	// End of line.
						// Move to next line of destination.
						// We require these end of lines for correct decompression.
						pu8Row		+= lPitch;
						pu8Uncomp	= pu8Row;
						break;
					case 1:	// End of bitmap.
						sDone	= TRUE;
						break;
					case 2:
						{
						// Move to position indicated by next two bytes relative
						// to this position.  Horizontal first, vertical second.
						// This should only be used in the case of deltas and
						// I cannot see how that could ever be since this isn't
						// an animation format.
						U8	u8Horz	= *pu8Comp++;
						U8	u8Vert	= *pu8Comp++;
						pu8Uncomp	+= u8Horz + (u8Vert * lPitch);
						break;
						}
					default:	// Absolute mode.
						// Intrinsic version of memcpy is fast; it aligns its copies
						// unless the compiler sucks.
						memcpy(pu8Uncomp, pu8Comp, u8Num);
						pu8Comp		+= u8Num;
						pu8Uncomp	+= u8Num;
						// This copy is supposed to be word aligned.  It is not clearly
						// stated in the dox, but it seems that, if this was an odd
						// number of bytes, we should skip the next byte.
						if (u8Num % 2 != 0)
							{
							pu8Comp++;
							}
						break;
					}
				}
			}

		// Deallocate old buffer.
		RImage::DestroyDetachedData(&pvDetachedMem);

		pImage->m_type = RImage::BMP8;

		sRes = (short)pImage->m_type;
		}
	else
		{
		TRACE("ConvertFromBMP8RLE(): CreateData() failed.\n");
		// Re-attach old buffer.
		pImage->m_pMem	= (UCHAR*)pvDetachedMem;
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////
//
// Converts to the Window's compression format for 8 bit BMPs (RLE8)
// RImage::BMP8RLE from RImage::BMP8.  Palette is not touched.  See
// description of this compression in comment header for 
// ConvertFromBMP8RLE.
//
//////////////////////////////////////////////////////////////////////
short ConvertToBMP8RLE(RImage* pImage)
	{
	short	sRes	= RImage::NOT_SUPPORTED;	// Assume failure.

	// Only certain types can be converted from.
	switch (pImage->m_type)
		{
		case RImage::BMP8:
			{
			// Set up a pointer to the 8-bit uncompressed buffer before detaching
			U8*	pu8Uncomp	= pImage->m_pData;

			// We must flip the image during compression.
			// This should work for both negative and positive
			// pitch buffers (i.e., upside down and right side up).
			long	lPitch	= -pImage->m_lPitch;
			pu8Uncomp		= pu8Uncomp + pImage->m_lPitch * ((long)pImage->m_sHeight - 1);

			// Detach the 8-bit uncompressed buffer from the Image
			void* pvDetachedMem = pImage->DetachData();

			// Create a new 8-bit compressed buffer and set a pointer to the 
			// data.  Leave the current pitch.  This buffer must be big enough for
			// the worst case compression (it could be infinitely large if I were
			// dumb and added many deltas that didn't do anything, but I won't
			// do that).  I think the worst case for my algorithm is a run of 1 pixel 
			// followed by a run of 1 different pixel, followed by a run of 1 or more 
			// pixels different than the previous run.  That's 6 bytes for 3 pixels?
			// plus end of lines, which 1 to 1 for the height.  Therefore, I'm
			// assuming 2 times the pixels plus the height times 2 (2 bytes for each end
			// of line) plus 2 bytes for the end of bitmap is good enough.  
			// 2 * ulSize + lHeight * 2 + 2.
			if (pImage->CreateData(2 * pImage->m_ulSize + (long)pImage->m_sHeight * 2 + 2) == 0)
				{
				U8*	pu8Comp		= pImage->m_pData;

				// Actual compression.  See function header for details.
				U8*	pu8Row	= pu8Uncomp;
				long	lRowRemain;
				long	lRun;

				// Compress until we exceed buffer.  Nothing has explicitly
				// said we should, but we will try to be row based.  This works
				// well for flipping the image while we're compressing.
				// Note that we never use the escape for moving the current position.
				// It seems to me this would only be useful in animation.
				for (long lNumLines = (long)pImage->m_sHeight; lNumLines > 0; lNumLines--)
					{
					lRowRemain	= (long)pImage->m_sWidth;

					while (lRowRemain > 0)
						{
						// Look for verbatim runs first since they are limited
						// to 3 or more.
						for (lRun	= 1; lRun < lRowRemain && lRun < 255; lRun++)
							{
							if (pu8Uncomp[lRun] == pu8Uncomp[lRun - 1])
								{
								break;
								}
							}

						// If less than 3, do a one color run . . .
						if (lRun < 3)
							{
							// Look for length of run with one color index.
							for (lRun	= 1; lRun < lRowRemain && lRun < 255; lRun++)
								{
								if (pu8Uncomp[lRun] != *pu8Uncomp)
									{
									break;
									}
								}
							
							// Number of pixels to run this color index.
							*pu8Comp++	= (U8)lRun;
							// Color index to run.
							*pu8Comp++	= *pu8Uncomp;
							// Advance past run.
							pu8Uncomp	+= lRun;
							}
						else
							{
							// 0 indicates "absolute mode".
							*pu8Comp++	= 0;
							// Number of absolute pixels.
							*pu8Comp++	= (U8)lRun;

							// Verbatim run.
							memcpy(pu8Comp, pu8Uncomp, lRun);

							// Advance uncomp.
							pu8Uncomp	+= lRun;
							// Advance comp.
							pu8Comp		+= lRun;

							// If this is an odd number . . .
							if ((lRun % 2) == 1)
								{
								// Skip to maintain "word align"ment.  Make 
								// ignored byte 0 for obviousness.
								*pu8Comp++	= 0;
								}
							}
						
						// Reduce amount left this row.
						lRowRemain	-= lRun;
						
						ASSERT(lRowRemain >= 0);
						}

					// Add end of line escape.
					*pu8Comp++		= 0;
					*pu8Comp++		= 0;

					// Update row.
					pu8Row		+= lPitch;
					// Set ptr to new row.
					pu8Uncomp	= pu8Row;
					}

				// Add end of BMP escape.
				*pu8Comp++		= 0;
				*pu8Comp++		= 1;


				// Get size of compressed buffer.
				ULONG	ulSize	= (pu8Comp - pImage->m_pData);

				// Deallocate old buffer.
				RImage::DestroyDetachedData(&pvDetachedMem);

				// Allocate again and copy.
				pu8Comp			= pImage->m_pData;
				pvDetachedMem	= pImage->DetachData();

				// Attempt to allocate new correctly sized buffer . . .
				if (pImage->CreateData(ulSize) == 0)
					{
					// Copy data.
					memcpy(pImage->m_pData, pu8Comp, pImage->m_ulSize);

					// Destroy old, too large compressed buffer.
					RImage::DestroyDetachedData(&pvDetachedMem);
					}
				else
					{
					TRACE("ConvertToBMP8RLE(): CreateData() failed for second compressed buffer.\n");
					// Re-attach old buffer.
					pImage->m_pMem	= (UCHAR*)pvDetachedMem;
					}

				pImage->m_type = RImage::BMP8RLE;

				sRes = (short)pImage->m_type;
				}
			else
				{
				TRACE("ConvertToBMP8RLE(): CreateData() failed.\n");
				// Re-attach old buffer.
				pImage->m_pMem	= (UCHAR*)pvDetachedMem;
				}

			break;
			}
		
		default:
			// Not supported.
			break;
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////
//
// Converts from RImage::BMP1 to RImage::BMP8.  Palette is not touched, but
// is NOT significant to BMP1 format (1 == Black, 0 == White).
//
//////////////////////////////////////////////////////////////////////
short ConvertFromBMP1(RImage* pImage)
	{
	short	sRes	= RImage::NOT_SUPPORTED;	// Assume failure.

	// Set up a pointer to the 1-bit packed buffer before detaching.
	U8*	pu8Src	= pImage->m_pData;

	// Detach the 1-bit packed buffer from the Image
	void* pvDetachedMem = pImage->DetachData();

	// Create a new 8-bit buffer and set a pointer to the 
	// data.  Alter the current pitch.
	
	// Set new depth.
	pImage->m_sDepth	= 8;

	// Remember old pitch.
	long	lSrcPitch	= pImage->m_lPitch;

	// Compute the new pitch.  Preserve sign.
	if (pImage->m_lPitch > 0)
		{
		pImage->m_lPitch	= RImage::GetPitch(pImage->m_sWidth, pImage->m_sDepth);
		}
	else
		{
		pImage->m_lPitch	= -RImage::GetPitch(pImage->m_sWidth, pImage->m_sDepth);
		}

	// Allocate new data.  This call sets ulSize.  Preserve m_sWidth, m_sHeight.
	if (pImage->CreateData(pImage->m_sHeight * ABS(pImage->m_lPitch)) == 0)
		{
		// Destination.
		U8*	pu8Dst	= pImage->m_pData;
		// Row trackers.
		U8*	pu8SrcRow	= pu8Src;
		U8*	pu8DstRow	= pu8Dst;

		long	lRows	= (long)pImage->m_sHeight;
		long	lCols;
		while (lRows-- > 0)
			{
			lCols	= (long)pImage->m_sWidth / 8;
			while (lCols-- > 0)
				{
				// Pack a byte.
				*pu8Dst++	= *pu8Src >> 7;
				*pu8Dst++	= (*pu8Src & 0x40) >> 6;
				*pu8Dst++	= (*pu8Src & 0x20) >> 5;
				*pu8Dst++	= (*pu8Src & 0x10) >> 4;
				*pu8Dst++	= (*pu8Src & 0x08) >> 3;
				*pu8Dst++	= (*pu8Src & 0x04) >> 2;
				*pu8Dst++	= (*pu8Src & 0x02) >> 1;
				*pu8Dst++	= (*pu8Src++ & 0x01);
				}

			// Move "down" a row.
			pu8SrcRow	+= lSrcPitch;
			pu8DstRow	+= pImage->m_lPitch;
			// Update columns to beginning of row.
			pu8Src		= pu8SrcRow;
			pu8Dst		= pu8DstRow;
			}

		// Deallocate old buffer.
		RImage::DestroyDetachedData(&pvDetachedMem);

		// Set new type.
		pImage->m_type = RImage::BMP8;

		// Set return value.
		sRes = (short)pImage->m_type;
		}
	else
		{
		TRACE("ConvertToBMP1(): CreateData() failed.\n");
		// Re-attach old buffer.
		pImage->m_pMem	= (UCHAR*)pvDetachedMem;
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////
//
// Converts to RImage::BMP1 from RImage::BMP8.  Palette is not touched, but
// is NOT significant to this format (1 == Black, 0 == White).
//
//////////////////////////////////////////////////////////////////////
short ConvertToBMP1(RImage* pImage)
	{
	short	sRes	= RImage::NOT_SUPPORTED;	// Assume failure.

	// Only certain types can be converted from.
	switch (pImage->m_type)
		{
		case RImage::BMP8:
			{
			// Set up a pointer to the 8-bit buffer before detaching.
			U8*	pu8Src	= pImage->m_pData;

			// Detach the 8-bit buffer from the Image
			void* pvDetachedMem = pImage->DetachData();

			// Create a new 1-bit packed buffer and set a pointer to the 
			// data.  Alter the current pitch.
			
			// Set new depth.
			pImage->m_sDepth	= 1;

			// Remember old pitch.
			long	lSrcPitch	= pImage->m_lPitch;

			// If there is a partial byte . . .
			if ((pImage->m_lPitch % 8) != 0)
				{
				pImage->m_lPitch	= (pImage->m_lPitch / 8);
				
				// Adjust appropriately for positive pitch . . .
				if (pImage->m_lPitch > 0)
					{
					pImage->m_lPitch++;
					}
				else	// Negative pitch.
					{
					pImage->m_lPitch--;
					}
				}
			else
				{
				pImage->m_lPitch	= (pImage->m_lPitch / 8);
				}

			// Allocate new data.  This call sets ulSize.  Preserve lWidth, lHeight.
			if (pImage->CreateData((long)pImage->m_sHeight * ABS(pImage->m_lPitch)) == 0)
				{
				// Bit packed destination.
				U8*	pu8Dst	= pImage->m_pData;
				// Row trackers.
				U8*	pu8SrcRow	= pu8Src;
				U8*	pu8DstRow	= pu8Dst;

				// Converts non-zero values to a 1 and increments ptr.
				#define CHECK_NONZERO_INC(p)	((*(p)++ == 0) ? 0 : 1)

				long	lRows	= (long)pImage->m_sHeight;
				long	lCols;
				while (lRows-- > 0)
					{
					lCols	= (long)pImage->m_sWidth / 8;
					while (lCols-- > 0)
						{
						// Pack a byte.
						*pu8Dst		= CHECK_NONZERO_INC(pu8Src) << 7;
						*pu8Dst		|= CHECK_NONZERO_INC(pu8Src) << 6;
						*pu8Dst		|= CHECK_NONZERO_INC(pu8Src) << 5;
						*pu8Dst		|= CHECK_NONZERO_INC(pu8Src) << 4;
						*pu8Dst		|= CHECK_NONZERO_INC(pu8Src) << 3;
						*pu8Dst		|= CHECK_NONZERO_INC(pu8Src) << 2;
						*pu8Dst		|= CHECK_NONZERO_INC(pu8Src) << 1;
						*pu8Dst++	|= CHECK_NONZERO_INC(pu8Src) << 0;
						}

					// Move "down" a row.
					pu8SrcRow	+= lSrcPitch;
					pu8DstRow	+= pImage->m_lPitch;
					// Update columns to beginning of row.
					pu8Src		= pu8SrcRow;
					pu8Dst		= pu8DstRow;
					}

				// Deallocate old buffer.
				RImage::DestroyDetachedData(&pvDetachedMem);

				// Set new type.
				pImage->m_type = RImage::BMP1;

				// Set return value.
				sRes = (short)pImage->m_type;
				}
			else
				{
				TRACE("ConvertToBMP1(): CreateData() failed.\n");
				// Re-attach old buffer.
				pImage->m_pMem	= (UCHAR*)pvDetachedMem;
				}

			break;
			}
		
		default:
			// Not supported.
			break;
		}

	return sRes;
	}


//////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////
