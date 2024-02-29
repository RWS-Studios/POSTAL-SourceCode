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
//	SPECIALTYP.H
//   
//	Created on 		09/28/96	JRD 
// Implemented on 09/28/96 JRD
//
// 09/28/96	JRD	Started this file with the defines for the extended
//						types of images and palettes. 
//
//	10/24/96	JRD	Added FSPR1 version 5 format
//
//	10/25/96	JRD	Adding FSPR8 version 3 (in progress)
//
// 
// This allows (if desired) user access to special image types, 
// as well as needed extra parameters during the conversion process. 
// The file declares the form of pSpecial for each type, as well as
// any needed extensions to other standard RImage functions in
// accommodation of special types.
//
//////////////////////////////////////////////////////////////////////

#ifndef SPECIAL_TYP_H
#define SPECIAL_TYP_H
// This currently depends only on "system.h"

//*******************************************************************
//*****  NOTE:  In the same convention as m_pData, 
//*****  m_pSpecial points to the possibly aligned special
//*****  data, and m_pSpecialMem refers EITHER to the 
//*****  originally newed/calloced memory OR is NULL to 
//*****  indicate that this Image is a COPY and should not free
//*****  its own pSpecial.
//*******************************************************************

//===================================================================
// Current Plug and Play list:
//
//(FSPR8,ConvertToFSPR8,ConvertFromFSPR8,LoadFSPR8,SaveFSPR8,NULL,DeleteFSPR8)
//(FSPR1,ConvertToFSPR1,ConvertFromFSPR1,LoadFSPR1,SaveFSPR1,NULL,DeleteFSPR1)

//===================================================================

//*************************************************
//******  FSPR1 => high speed monochrome **********
//******  used for text and masks        **********
//*************************************************

//------------- Installed RImage Plugins: -------------------------
// ConvertTo:		YES => from any 8-bit
// ConvertFrom:	YES => to any 8-bit
// Load:				YES
// Save:				YES
// Alloc:			NO
// Delete:			YES
// Copy:				NO
//-----------------------------------------------------------------
//
// CONVERION NOTE:  Will jettison the palette, as all color info is lost.
//		Therefore, DO NOT use an FSPR1 as a "palette holder"
//
class	RSpecialFSPR1
	{
public:
	UCHAR*	m_pCode;	// actual FSPR1 data
	S32	m_lSize;		// actual size of FSPR1 data
	U16	m_u16ASCII;	// allow extended character sets
	S16	m_s16KernL;	// move left position
	U16	m_u16Width;	// physical width of image, SHOULD = lWidth!
	S16	m_s16KernR; // offset from right side
	RImage::Type	m_OldType;// previous type
	S16*	m_psKernList;// Used in printing only
	// When you implement m_psKernList, please augment the destructor!

	RSpecialFSPR1()
		{
		m_pCode = NULL;
		m_psKernList = NULL;
		m_s16KernL = m_s16KernR = (S16)0;
		m_u16ASCII = m_u16Width = (U16)0;
		m_lSize = (S32)0;
		}

	~RSpecialFSPR1()
		{
		// using calloc / free!!!
		if (m_pCode) free (m_pCode);
		m_pCode = NULL;
		}
	};

//=======================================================
//========= Convert extensions for FSPR1: ===============
//=======================================================

// Note: ASCII and kerning information is used and stored
// by CFNT, and is NOT used in raw blitting!

// NOTE:  does NOT trim!  (use rspLasso)
//
extern void SetConvertToFSPR1
	(	
	U32	u32TransparentColor = 0, // Make transparent
	S16	sX = -1,	// Convert only from this...
	S16	sY = -1,	// rectangle.  Default values ...
	S16	sW = -1,	// move to edges of image.
	S16	sH = -1,	// Use (-1) to use image edge
	// If you make a copy, the original RImage will NOT be
	// altered!  **pimCopy will 
	RImage**	ppimCopy = NULL // To make a copy
	);

extern	void SetConvertFromFSPR1
	(
	U32	u32ForeColor,				// Make it this color
	S16	sTransparent = TRUE,		// 1 or 2 color?
	U32	u32BackColor = (U32)0	// matters only if sTransparent = FALSE
	);

//*************************************************
//******  FSPR8 => high speed color		  **********
//******  used for sprites               **********
//*************************************************

//------------- Installed RImage Plugins: -------------------------
// ConvertTo:		YES => from any 8-bit
// ConvertFrom:	YES => to any 8-bit
// Load:				YES
// Save:				YES
// Alloc:			NO
// Delete:			YES
// Copy:				NO
//-----------------------------------------------------------------
//
class	RSpecialFSPR8
	{
public:
	USHORT	m_usCompType;	// = FSPR8 image type
	ULONG		m_lBufSize;		// Size of the opaque pixel data
	ULONG		m_lCodeSize;	// Size of the control block (compression codes)
	USHORT	m_usSourceType;// uncompressed Image pre-compressed type

	UCHAR*	m_pCompBuf;		// Compressed picture data, 128bit-aligned
	UCHAR*	m_pCompMem;		// For alignment
	UCHAR*	m_pCodeBuf;		// 32-aligned compression codes
	UCHAR**	m_pBufArry;		// 32-aligned, arry of ptrs to m_pCompBuf scanlines
	UCHAR**	m_pCodeArry;	// 32-aligned, arry of ptrs into m_pCodeBuf scanlines

	RSpecialFSPR8()
		{
		m_usCompType = m_usSourceType = 0;
		m_pCompBuf = m_pCompMem = m_pCodeBuf = NULL;
		m_pBufArry = m_pCodeArry = NULL;
		m_lBufSize = m_lCodeSize = 0;
		}

	~RSpecialFSPR8()
		{
		if (m_pCompMem) free(m_pCompMem);
		if (m_pCodeBuf) free(m_pCodeBuf);
		if (m_pBufArry) free(m_pBufArry);
		if (m_pCodeArry) free(m_pCodeArry);
		}
	};

//=======================================================
//========= Convert extensions for FSPR8: ===============
//=======================================================

// CURENTLY, NO EXTRA CONTROL IS GIVEN WITH FSPR8 CONVERSIONS

//*************************************************
//******  ROTBUF => texture buffer       **********
//******  also used for rotation         **********
//*************************************************
// Curently, the normal extensions of Image are
// being used for textures and such.

//=======================================================
//========= Convert extensions for ROTBUF: ===============
//=======================================================


//===================
#endif // special type 