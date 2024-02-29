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
#ifndef DITHER_MATCH_H
#define DITHER_MATCH_H
//==============================================
#include "System.h"
#ifdef PATHS_IN_INCLUDES
	#include "GREEN/Image/Image.h"
	#include "ORANGE/color/colormatch.h"
#else
	#include "Image.h"
	#include "colormatch.h"
#endif
//==============================================

//==============================================
// Here is the optional user-interactive callback procedure,
// used for long, slow, color mappings.
//==============================================
// dProgress: A value between 0.0 (just started) and
//					1.0 (done).  Use can display as he wishes.
// RETURN: User should return 0 for CONTINUE, and
//		-1 to ABORT the conversion early! (User hits cancel)
// 
//
typedef	short (*PDitherCallBack) (double dProgress);

//==============================================
// You supply a general palette to match to, 
// a BMP24 image as a TC source, and an 
// instantiated, but NOT allocated BMP8 as a 
// destination.
// 
// THIS WILL Dither to background!
//==============================================
// The optional area parameter sets the approximate
// interval for the callback function (it is not
// time based!)  I find 32000 good for a P6/200, but
// smaller numbers can be used for slower machines.
//	
extern	short	rspDither(	
					RImage* pimSrc,	// MUST BE 24-bit!
					RImage* pimDst,	// MUST be 8-bit
					short sStartMap,	// palette index
					short sNumMap,		// # of colors
					UCHAR*	pRed,		// Palette to match to
					UCHAR*	pGreen,
					UCHAR*	pBlue,
					long	lInc = 4,
					// User interaction
					PDitherCallBack func = NULL,
					long  lMilli = 500 // time between callbacks
					);
	
//==============================================
// You supply a general palette to match to, 
// a BMP24 image as a TC source, and an 
// instantiated, but NOT allocated BMP8 as a 
// destination.
// 
// Does NOT dither at all!
//==============================================
// The optional area parameter sets the approximate
// interval for the callback function (it is not
// time based!)  I find 32000 good for a P6/200, but
// smaller numbers can be used for slower machines.
//	
extern	short	rspSimpleMap(	
					RImage* pimSrc,	// MUST BE 24-bit!
					RImage* pimDst,	// MUST be 8-bit
					short sStartMap,	// palette index
					short sNumMap,		// # of colors
					UCHAR*	pRed,		// Palette to match to
					UCHAR*	pGreen,
					UCHAR*	pBlue,
					long	lInc = 4,
					// User interaction
					PDitherCallBack func = NULL,
					long  lMilli = 500 // time between callbacks
					);

//==============================================
// You supply a general palette to match to, 
// a BMP24 image as a TC source, and an 
// instantiated, but NOT allocated BMP8 as a 
// destination.
// 
// this will NOT Dither to the specified background color!
//
// Returns 0 for SUCCESS, -1 for ERROR, 1 for user cancel
//==============================================
//	
extern	short	rspDither(	
					long lBackR,		// Don't dither to this color!
					long lBackG,
					long lBackB,
					UCHAR ucBack,		// index to make BKGD
					RImage* pimSrc,	// MUST BE 24-bit!
					RImage* pimDst,	// MUST be 8-bit
					short sStartMap,	// palette index
					short sNumMap,		// # of colors
					UCHAR*	pRed,		// Palette to match to
					UCHAR*	pGreen,
					UCHAR*	pBlue,
					long	lInc = 4,
					// User interaction
					PDitherCallBack func = NULL,
					long  lMilli = 500 // time between callbacks
					);

//==============================================
// You supply a general palette to match to, 
// a BMP32 image as a TC source, and an 
// instantiated, but NOT allocated BMP8 as a 
// destination.
// 
// THIS WILL NOT Dither to the specified background, which
// is determined by the alpha channel being less than the
// value specified by the user.
//
// Returns 0 for SUCCESS, -1 for ERROR, 1 for user cancel
//==============================================
//	
short	rspDither(	
					UCHAR ucForeAlpha,		// lower limit for foreground
					UCHAR ucBack,		// index to make BKGD
					RImage* pimSrc,	// MUST BE 24-bit!
					RImage* pimDst,	// MUST be 8-bit
					short sStartMap,	// palette index
					short sNumMap,		// # of colors
					UCHAR*	pRed,		// Palette to match to
					UCHAR*	pGreen,
					UCHAR*	pBlue,
					long	lInc = 4,
					// User interaction
					PDitherCallBack func = NULL,
					long  lMilli = 500 // time between callbacks
					);

//==============================================
// You supply a general palette to match to, 
// a BMP32 image as a TC source, and an 
// instantiated, but NOT allocated BMP8 as a 
// destination.
// 
// Does NOT dither at all!
//
// As with dithering, anything below the 
// specified alpha threshold is considered a
// background and set to the specified background
// color.  If a 0 value is specified for the
// alpha threshold, than no special background
// treatment will occur.
//==============================================
//	
short	rspSimpleMap(	
					UCHAR	ucForeAlpha,	// alpha threshhold
					UCHAR ucBack,			// map background to this index
					RImage* pimSrc,	// MUST BE 32-bit!
					RImage* pimDst,	// MUST be 8-bit
					short sStartMap,	// palette index
					short sNumMap,		// # of colors
					UCHAR*	pRed,		// Palette to match to
					UCHAR*	pGreen,
					UCHAR*	pBlue,
					long	lInc = 4,
					// User interaction
					PDitherCallBack func = NULL,
					long  lMilli = 500 // time between callbacks
					);

//==============================================
//==============================================
//==============================================
#endif
