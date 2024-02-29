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
//	alpha.cpp
// 
// History:
//		10/04/96 JMI	Started.
//
//
//////////////////////////////////////////////////////////////////////////////
//
// Cheezy ass alpha blit.
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Includes.
//////////////////////////////////////////////////////////////////////////////
/*
#include "System.h"

#ifdef PATHS_IN_INCLUDES
	#include "GREEN/Image/Image.h"
	#include "GREEN/BLiT/BLIT.H"
#else
	#include "Image.h"
	#include "BLIT.H"
#endif
#else
*/
#include "RSPiX.h"
#include "alpha.h"


//////////////////////////////////////////////////////////////////////////////
// Macros.
//////////////////////////////////////////////////////////////////////////////
#define TRANS_INDEX	14

//////////////////////////////////////////////////////////////////////////////
// HACKS.
//////////////////////////////////////////////////////////////////////////////
#if 0
//========================================================================================
extern	short	rspBlitT(U8 u8Trans, RImage* pimSrc,RImage* pimDst,short sSrcX,short sSrcY,short sDstX,
			  short sDstY,short sW,short sH,RRect* prDst,const RRect* prSrc);
//========================================================================================
#endif

//////////////////////////////////////////////////////////////////////////////
// Instantiate statics.
//////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// Con/Destruction.
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// Functions.
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
//
// Whatever this ends up doing, it will suck and eventually be replaced
// so that's the comment or something.
//
///////////////////////////////////////////////////////////////////////////
static short Alpha(	// Returns 0 on success.
	RImage*	pimSrc,	// Source to blit through pimMask.
	RImage*	pimMask,	// Mask.
	RImage*	pimDst,	// Destination for masked blit.
	short		sSrcX,	// X coordinate in source.
	short		sSrcY,	// Y coordinate in source.
	short		sDstX,	// X coordinate in dest.
	short		sDstY,	// Y coordinate in dest.
	short		sW,		// Width to blt.
	short		sH)		// Height to blt.
	{
	short	sRes	= 0;	// Assume success.

	short	sMaskX	= 0;
	short	sMaskY	= 0;

	// X/W Clip.
	if (sSrcX < 0)
		{
		sW		+= sSrcX;
		sDstX -= sSrcX;
		sMaskX	-= sSrcX;
		sSrcX	= 0;
		}

	if (sDstX < 0)
		{
		sW		+= sDstX;
		sSrcX	-= sDstX;
		sMaskX	-= sDstX;
		sDstX	= 0;
		}

	if (sSrcX + sW > pimSrc->m_sWidth)
		{
		sW	= pimSrc->m_sWidth - sSrcX;
		}

	if (sDstX + sW > pimDst->m_sWidth)
		{
		sW	= pimDst->m_sWidth - sDstX;
		}

	if (sMaskX + sW > pimMask->m_sWidth)
		{
		sW	= pimMask->m_sWidth;
		}

	// Y/H Clip.
	if (sSrcY < 0)
		{
		sH		+= sSrcY;
		sDstY -= sSrcY;
		sMaskY	-= sSrcY;
		sSrcY	= 0;
		}

	if (sDstY < 0)
		{
		sH		+= sDstY;
		sSrcY	-= sDstY;
		sMaskY	-= sDstY;
		sDstY	= 0;
		}

	if (sSrcY + sH > pimSrc->m_sHeight)
		{
		sH	= pimSrc->m_sHeight - sSrcY;
		}

	if (sDstY + sH > pimDst->m_sHeight)
		{
		sH	= pimDst->m_sHeight - sDstY;
		}

	if (sMaskY + sH > pimMask->m_sHeight)
		{
		sH	= pimMask->m_sHeight;
		}

	// If there's anything left . . .
	if (sW > 0 && sH > 0)
		{
		U8*	pu8SrcRow	= pimSrc->m_pData + sSrcX + sSrcY * pimSrc->m_lPitch;
		U8*	pu8SrcBlt;
		U8*	pu8MaskRow	= pimMask->m_pData + sMaskX + sMaskY * pimMask->m_lPitch;
		U8*	pu8MaskBlt;
		U8*	pu8DstRow	= pimDst->m_pData + sDstX + sDstY * pimDst->m_lPitch;
		U8*	pu8DstBlt;
		
		short	sWidth;

		while (sH--)
			{
			pu8SrcBlt	= pu8SrcRow;
			pu8MaskBlt	= pu8MaskRow;
			pu8DstBlt	= pu8DstRow;

			sWidth	= sW;
			while (sWidth--)
				{
				if (*pu8MaskBlt++ != 0)
					{
					*pu8DstBlt	= *pu8SrcBlt;
					}

				pu8DstBlt++;
				pu8SrcBlt++;
				}

			pu8SrcRow	+= pimSrc->m_lPitch;
			pu8MaskRow	+= pimMask->m_lPitch;
			pu8DstRow	+= pimDst->m_lPitch;
			}
		}

	return sRes;
	}

///////////////////////////////////////////////////////////////////////////
//
// BLiTs using loaded m_imMask (8 bpp only) as mask.
// 0 in mask indicates opaque.  Other values are punch through.
//
///////////////////////////////////////////////////////////////////////////
short CAlpha::Blit(	// Returns 0 on success.
	RImage*	pimSrc,	// Source image.
	RImage*	pimDst,	// Destination image.
	short	sSrcX,		// Source coordinate in pimSrc to start effect.  Can
							// be negative.
	short	sSrcY,		// Source coordinate in pimSrc to start effect.  Can
							// be negative.
	short	sDstX,		// Destination coordinate in pimDst for pimSrc(0,0).
	short	sDstY,		// Destination coordinate in pimDst for pimSrc(0,0).
	RRect*	prc)			// Rectangle to clip Dst to.
	{
	short	sRes	= 0;	// Assume success.

	// If there is any data . . .
	if (m_imMask.m_pData != NULL || m_imMask.m_pSpecial != NULL)
		{
		RImage	imDecompress;
		// Allocate image.
		if (imDecompress.CreateImage(
			pimSrc->m_sWidth, 
			pimSrc->m_sHeight, 
			RImage::BMP8, 
			0,		// Use default pitch.
			pimSrc->m_sDepth) == 0)
			{
			// Decompress sprite.
			rspBlit(pimSrc, &imDecompress, 0, 0);
			
			// Blit transparency into decompression buffer.
			rspBlitT(TRANS_INDEX,
				&m_imMask, 
				&imDecompress, 
				0, 0,
				sSrcX, sSrcY,
				m_imMask.m_sWidth, m_imMask.m_sHeight, 
				NULL, 
				NULL);

			// Blit to screen.
			rspBlitT(
				0,
				&imDecompress, 
				pimDst, 
				0, 0, 
				sDstX, sDstY, 
				imDecompress.m_sWidth, imDecompress.m_sHeight, 
				prc,
				NULL);
			}
		else
			{
			TRACE("Blit(): Unable to allocate image to decompress pimSrc.\n");
			sRes	= -2;
			}
		}
	else
		{
		TRACE("Blit(): No mask!  Use regular blt.\n");
		sRes	= -1;
		}

	return sRes;
	}

///////////////////////////////////////////////////////////////////////////
//
// Loads the specified file into m_imMask using RImage::Load() and calls
// Convert(FSPR1) to prepare the data.
//
///////////////////////////////////////////////////////////////////////////
short CAlpha::Load(		// Returns 0 on success.
	char*	pszFileName)	// Filename to load.
	{
	short	sRes	= 0;	// Assume success.


	if (m_imMask.Load(pszFileName) == 0)
		{
#if 0
		rspSetConvertToFSPR1(128, 0);

		if (m_imMask.Convert(FSPR1) == FSPR1)
			{
			// Success.
			}
		else
			{
			TRACE("Load(): RImage::Convert(FSPR1) failed.\n");
			sRes	= -2;
			}
#endif
		m_sShadowW	= m_imMask.m_sWidth;
		m_sShadowH	= 5;
		m_sShadowX	= 0;
		m_sShadowY	= m_imMask.m_sHeight - m_sShadowH;
		}
	else
		{
		TRACE("Load(): RImage::Load() failed.\n");
		sRes	= -1;
		}

	return sRes;
	}


//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
