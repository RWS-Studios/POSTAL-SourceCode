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
// Includes.
//////////////////////////////////////////////////////////////////////////////
#ifndef ALPHA_H
#define ALPHA_H
#include "System.h"

#ifdef PATHS_IN_INCLUDES
	#include "GREEN/Image/Image.h"
	#include "GREEN/BLiT/BLIT.H"
#else
	#include "Image.h"
	#include "BLIT.H"
#endif

//////////////////////////////////////////////////////////////////////////////
// Macros.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Typedefs.
//////////////////////////////////////////////////////////////////////////////
class CAlpha
	{
	///////////////////////////////////////////////////////////////////////////
	// Con/Destruction.
	///////////////////////////////////////////////////////////////////////////
	public:
		CAlpha()
			{
			m_sShadowX	= 0;
			m_sShadowY	= 0;
			m_sShadowW	= 0;
			m_sShadowH	= 0;
			}

		~CAlpha() {  }

	///////////////////////////////////////////////////////////////////////////
	// Functions.
	///////////////////////////////////////////////////////////////////////////
	public:
		
		// BLiTs using loaded m_im (8 bpp only) as mask.
		// 0 in mask indicates opaque.  Other values are punch through.
		short Blit(				// Returns 0 on success.
			RImage*	pimSrc,	// Source image.
			RImage*	pimDst,	// Destination image.
			short	sSrcX,		// Source coordinate in pimSrc to start blit.
			short	sSrcY,		// Source coordinate in pimSrc to start blit.
			short	sDstX,		// Destination coordinate in pimDst for pimSrc(0,0).
			short	sDstY,		// Destination coordinate in pimDst for pimSrc(0,0).
			RRect*	prc);		// Rectangle to clip Dst to.

		// Loads the specified file into m_imMask using LoadDib() and calls
		// Convert(FSPR1) to prepare the data.
		short Load(					// Returns 0 on success.
			char*	pszFileName);	// Filename to load.

	///////////////////////////////////////////////////////////////////////////
	// Data.
	///////////////////////////////////////////////////////////////////////////
	public:
		RImage	m_imMask;	// Mask for "alpha" blit.
		short		m_sShadowX;	// X position from user sprite of shadow.
		short		m_sShadowY;	// Y posiiton from user sprite of shadow.
		short		m_sShadowW;	// Width of shadow.
		short		m_sShadowH;	// Height of shadow.

	///////////////////////////////////////////////////////////////////////////
	// Static data.
	///////////////////////////////////////////////////////////////////////////
	protected:
	};

#endif	// ALPHA_H
//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
