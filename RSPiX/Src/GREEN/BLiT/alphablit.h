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
// The USER version of this will be moved to BLit.h eventually.
#ifndef ALPHA_BLIT_H
#define ALPHA_BLIT_H
//===========================================================================
#include "GREEN/BLiT/BLIT.H"
#include "ORANGE/color/colormatch.h"
//===========================================================================
//////////////////////////////////////////////////////////////////////
//
// ALPHABLIT.H
//
// Created in 1996 JRD
// Implemented	throughout 1996 and 1997 - JRD
//
//		07/10/97	JRD	Finally documented this file and added history section.
//
//////////////////////////////////////////////////////////////////////

// Here is a comand summary which hopefully is descriptive:

//********************************************************************
//--------------------- CONVENTIONAL ALPHA BLiTs ---------------------
//********************************************************************
//
// All alpha blits need a MultiAlpha table set for the current palette
// in order to translate true color transparency effects into 8-bit color
//
// All alpha objects and classes are part of Orange/Color/colormatch.cpp
// This is only the set of related blits.
//
// An "alpha table" descibes how colors map at a SET, SINGLE transparency
// level, such as 128 (50%).  It is a memory efficient way to do fixed,
// homogeneous level alpha blending.
//
// A "multialpha table" has a set of alpha tables for different alpha
// levels, so that blits can be used which involve many different alpha
// levels on one image or varying alpha levels.  Multialphas are memory
// intensive and tend to blow caching, so they are slower.  But they
// allow the fullest alpha effects.
//
// A "fast multialpha table" is an optimized data structure which 
// compresses memory by only including data for colors likely to 
// be found in the source and destination.  Once you KNOW your
// assets, this can be a useful memory saver.
//
// An "alpha channel value" ranges from 255 = opaque source, 
//	to 0 = invisible source
//
// Homogeneous alpha blits let you specify one alpha level for an 
// entire sprite.  Masked alpha blits let you specify a second BMP 
// in which every pixel represents the alpha level for each corresponding
// pixel in the source sprite.
//
// Alpha "BlitT" functions allow you to specify source index 0 as
// fully transparent, even if the alpha level is not zero.  It is 
// normally better to set the alpha mask to zero if possible.
//
// EFFICIENCY NOTE:  All alpha blits AUTOMATICALLY switch to normal
// blits in the fully opaque case, and do nothing in the fully transparent 
// case, so the app need not worry about such things.
//---------------------------------------------------------------------

// DOCUMENTATION SECTIONS:
//
// 1) Alpha BLiTs -> use RAlphas and RMultiAlphas
// 2) Fast Alpha Blits -> use Fast MultiAlphas
// 3)	General Color functions (inlines) for your pleasure
// 4) special blits designed to dynamically alter and create alpha masks!

//====================================================================
//	rspAlphaBlit - "opaque blit" which uses a single RAlpha table
//									Note that in this BLiT, a
//									source color of ZERO will be interpreted
//									by it's COLOR - sheet of glass effect.
// THIS IS NOT A MULTIALPHA BLIT.  It creates the effect of a single,
// fixed alpha level - the alpha level is IMPLIED in the creation of
// the given alpha table and cannot be varied. (fixed homogeneous)
// This is the fastest and most memory efficient alpha blit.  It is 
// also the most limited.
//====================================================================
extern	void rspAlphaBlit(
					RAlpha* pX,			// Set for current palette and alpha level
					RImage* pimSrc,	//	BMP8 source
					RImage* pimDst,	//	BMP8 destination
					short sDstX,short sDstY);

//====================================================================
//	rspAlphaBlitT - "transparent blit" which uses a single RAlpha
//									Note that in this BLiT, a
//									source color of ZERO will be interpreted
//									as fully transparent.  (useful)
//
// THIS IS NOT A MULTIALPHA BLIT.  It creates the effect of a single,
// fixed alpha level - the alpha level is IMPLIED in the creation of
// the given alpha table and cannot be varied. (fixed homogeneous)
// This is the fastest and most memory efficient alpha blit.  It is 
// also the most limited.
//====================================================================
extern	void rspAlphaBlitT(
					RAlpha* pX,			// Set for current palette and alpha level
					RImage* pimSrc,	//	BMP8 source
					RImage* pimDst,	//	BMP8 destination
					short sDstX,short sDstY);

//====================================================================
//	rspAlphaBlitT - "transparent blit" which uses a single alpha level
//									In this BLiT, a source color of ZERO will be 
//									interpreted as fully transparent, REGARDLESS of
//									the alpha level (which could be useful)
// This is a homogeneous blit - NO ALPHA MASK is used.  Rather, the 
// entire sprite is blit at the alphalevel specified, except for 0
// source pixels, which are taken as 100% transparent.
//
// This is the only case of a BlitT function that MUST be used, since
// there's no alpha mask to describe the sprite shape with.
//
//====================================================================
extern	void rspAlphaBlitT(
						short sAlphaLevel,		// 255 = opaque
						RMultiAlpha* pMultiX,	// For current palette
						RImage* pimSrc,			// BMP8 source image
						RImage* pimDst,			// BMP8 destination
						short sDstX,short sDstY,
						RRect* prDst = NULL);

//====================================================================
//	rspGeneralAlphaBlit - "opaque blit" which uses a BMP8 per pixel
//									alpha mask.  Note that in this BLiT, a
//									source color of ZERO will be interpreted
//									by it's COLOR - sheet of glass effect.
//====================================================================
extern	void rspGeneralAlphaBlit(
								RMultiAlpha* pX,	// For current palette
								RImage* pimMask,	// BMP8 alpha channel mask
								RImage* pimSrc,	// BMP8 source image
								RImage* pimDst,	// BMP8 destination
								short sDstX,
								short sDstY,
								RRect &rDstClip);

//====================================================================
//	rspGeneralAlphaBlit - "opaque blit" which uses a BMP8 per pixel
//									alpha mask.  Note that in this BLiT, a
//									source color of ZERO will be interpreted
//									by it's COLOR - sheet of glass effect.
//
//	This variation allows real time "dimming" of the curent mask.
// If sLevel = 255, the mask will BLiT normally, but less than 255
// and the image will fade away.
//
//====================================================================
extern	void rspGeneralAlphaBlit(
							short sLevel,		// 0-255, 255 = unchanged
							RMultiAlpha* pX,	// for current palette
							RImage* pimMask,	// BMP8 alpha channel mask
							RImage* pimSrc,	// BMP8 source
							RImage* pimDst,	// BMP8 destination
							short sDstX,
							short sDstY,
							RRect &rDstClip);

//====================================================================
//	rspGeneralAlphaBlitT - "transparent blit" which uses a BMP8 per pixel
//									alpha mask.  
//									In this BLiT, a source color of ZERO will be 
//									interpreted as fully transparent, REGARDLESS of
//									the alpha mask (which could be useful)
//
//	This variation allows real time "dimming" of the curent mask.
// If sLevel = 255, the mask will BLiT normally, but less than 255
// and the image will fade away.
//
// NOTE: It is better to use rspGeneralAlphaBlit and put the transparency 
//			in the alpha mask!
//
//====================================================================
extern	void rspGeneralAlphaBlitT(
							short sLevel,		// 0-255, 255 = unchanged
							RMultiAlpha* pX,	// for current palette
							RImage* pimMask,	// BMP8 alpha channel mask
							RImage* pimSrc,	// BMP8 source
							RImage* pimDst,	// BMP8 destination
							short sDstX,
							short sDstY,
							RRect &rDstClip);

//********************************************************************
//------------------------- FAST ALPHA BLiTs -------------------------
//********************************************************************
// Fast Multi Alphas are initially created from normal Multi Alphas.
// They are touchy, because they only apply to a subset of source and 
// destination colors, and if these ranges are not met, these
// blits can crash.  But, they can be FAR more memory efficient.
//
// Not all of the regular alpha blits have been ported over to the fast format

//====================================================================
//	rspFastAlphaBlitT - "transparent blit" 
//									In this BLiT, a source color of ZERO will be 
//									interpreted as fully transparent, REGARDLESS of
//									the alpha level (which could be useful)
// This is a homogeneous blit - NO ALPHA MASK is used.  Rather, the 
// entire sprite is blit at the alphalevel specified, except for 0
// source pixels, which are taken as 100% transparent.
//
// This is the only case of a BlitT function that MUST be used, since
// there's no alpha mask to describe the sprite shape with.
//
//====================================================================
extern	void rspFastAlphaBlitT(			
						short sAlphaLevel,	// 0-255, 255 = unchanged
						UCHAR*** pMultiX,		// A fast multialpha table
						RImage* pimSrc,		// BMP8 source	
						RImage* pimDst,		// BMP8 destination
						short sDstX,short sDstY,
						RRect* prDst = NULL);


//====================================================================
//	rspFastMaskAlphaBlit - "opaque blit" which uses a BMP8 per pixel
//									alpha mask.  Note that in this BLiT, a
//									source color of ZERO will be interpreted
//									by it's COLOR - sheet of glass effect.
//====================================================================
extern	void rspFastMaskAlphaBlit(
						UCHAR*** pfaX,		// A fast multialpha table
						RImage* pimMask,	// BMP8 source alpha channel mask
						RImage* pimSrc,	// BMP8 source	
						RImage* pimDst,	// BMP8 destination
						short sDstX,	
						short sDstY,
						RRect &rDstClip);

//====================================================================
//	rspFastMaskAlphaBlitT - "transparent blit" which uses a BMP8 per pixel
//									alpha mask.  
//									In this BLiT, a source color of ZERO will be 
//									interpreted as fully transparent, REGARDLESS of
//									the alpha mask (which could be useful)
//====================================================================
extern	void rspFastMaskAlphaBlitT(
						UCHAR*** pfaX,		// A fast multialpha table	
						RImage* pimMask,	// BMP8 source alpha channel mask	
						RImage* pimSrc,	// BMP8 source		
						RImage* pimDst,	// BMP8 destination		
						short sDstX,short sDstY,
						RRect &rDstClip);



//********************************************************************
//--------------  Color Blending Tools for Custom Functions  ---------
//********************************************************************

// THIS SHOULD MOVE TO OFFICIAL BLIT STATUS and not staty in alpha
//
inline short rspSimpleClip(short &sSrcX,short &sSrcY,short &sDstX,short &sDstY,
						 short &sDstW,short &sDstH,
						 short sClipX,short sClipY,short sClipW,short sClipH)
	{
	short sClipL,sClipR,sClipT,sClipB;

	//-------- Do the clipping:
	sClipL = sClipX - sDstX; if (sClipL < 0) sClipL = 0;
	sClipT = sClipY - sDstY; if (sClipT < 0) sClipT = 0;
	sClipR = (sDstX + sDstW - sClipX - sClipW); if (sClipR < 0) sClipR = 0;
	sClipB = (sDstY + sDstH - sClipY - sClipH); if (sClipB < 0) sClipB = 0;

	if ( ((sClipL + sClipR) >= sDstW) || ((sClipT + sClipB) >= sDstH) )
		{
		return -1; // fully clipped out
		}

	sSrcX += sClipL; sDstX += sClipL;
	sSrcY += sClipT; sDstY += sClipT;
	sDstW -= (sClipL + sClipR);
	sDstH -= (sClipT + sClipB);

	return 0;
	}

//====================================================================
//	rspScaleAlphaMask - you can adjust an existing alpha mask
//--------------------------------------------------------------------
//		if dScale < 1.0, you will make the mask more transparent
//		if dScale > 1.0, the maskwill be more opaque and will  saturate
//				at 255.
//
// NOTE: This process is lossy, and if done repeatedly should use the
//			original source mask each time.
//====================================================================
extern	void rspScaleAlphaMask(RImage* pimSrcMask, // BMP8 alpha mask
										  double dScale,		 // 1.0 = no change
										  RImage* pimDstMask);// BMP8 alpha mask


//===========================================================================
//  Here are some low level access functions to the alpha stuff so you can
//  build alpha graphics primatives:  There is a random access once, but for
//  sequential speed you should track your own pointers:

//===========================================================================
//
//	rspBlendColor
//
// RETURN the new color index based on a source and destination index:
// WARNING:  will not bounds check so be careful:
// sLeve will range from 255 (solid) to 0 (transparent)
//===========================================================================
inline UCHAR	rspBlendColor(short sLevel,RMultiAlpha* pX,UCHAR ucSrc,UCHAR ucDst)
	{
	ASSERT(pX);

	if (sLevel > *(pX->m_pLevelOpacity))
		{
		UCHAR** ppucAlpha = pX->m_pGeneralAlpha[sLevel]; 
		if (ppucAlpha) return ppucAlpha[ucSrc][ucDst]; // alpha'ed
		return ucSrc; // opaque
		}
	return ucDst; // transparent
	}

//===========================================================================
//
//	rspBlendColor
//
//	UINPUT:	sLevel -> the alpha level (0-255) you wish to work with
//	OUTPUT:	psOpaque -> a special case flag, is TRUE if FULLY opaque (normal BLiT)
// RETURN:	ppuc, a short cut to the fast blend function, or NULL if you hit the
//				extreme cases of fully opaque of fully transparent.
//===========================================================================
inline UCHAR**	rspFindBlend(short sLevel,RMultiAlpha* pX,short* psOpaque)
	{
	ASSERT(pX);
	ASSERT(psOpaque);
	*psOpaque = FALSE;
	
	if (sLevel <= *(pX->m_pLevelOpacity)) return NULL;	// fully transparent
	UCHAR** ppucAlpha = pX->m_pGeneralAlpha[sLevel]; 

	if (ppucAlpha) return ppucAlpha;
	*psOpaque = TRUE;
	return NULL;
	}

// A fast way to do repeated blits of the same alpha level, once you've used rspFindBlend:
inline	UCHAR	rspBlendColor(UCHAR** ppucAlpha,RMultiAlpha* pX,UCHAR ucSrc,UCHAR ucDst)
	{
	ASSERT(ppucAlpha);
	ASSERT(pX);

	return ppucAlpha[ucSrc][ucDst];
	}

//********************************************************************
//-----------  SPECIAL BLITs used in creation of X-RAY effect  -------
//********************************************************************

//====================================================================
//	rspMaskBlit - a blit used to dynamically create an alpha mask
//--------------------------------------------------------------------
// The OPPOSITE of a sprite blit, this is a blit that draws only
// where the DESTINATION is not zero!  It was useful to draw a
// transparent "hole" in an existing alpha mask, but keep the hole
// from going off the edge of the mask.  
//
// NOTE: THIS BLiT Will clip to the destination
//====================================================================
extern	void rspMaskBlit(
					RImage* pimSrc,	// BMP8
					RImage* pimDst,
					short sDstX,short sDstY);

//====================================================================
//	rspMakeMask - a blit used to dynamically create an alpha mask
//--------------------------------------------------------------------
//	This takes a given sprite, and reduces it's colors to 0 and
// ucVal.  This is useful for giving a given sprite a matching
// alpha mask of a set homogeneous opacity level.
//====================================================================
extern	void rspMakeMask(RImage* pimSrc,	// SOURCE AND DESTINATION
								  UCHAR ucVal);
//====================================================================
//	rspCopyAsMask - a blit used to dynamically create an alpha mask
//--------------------------------------------------------------------
//	This takes a given sprite, and reduces it's colors to 0 and
// ucVal.  This is useful for giving a given sprite a matching
// alpha mask of a set homogeneous opacity level.
//
// This does the same as rspMakeMask, but does not alter the original
//====================================================================
extern	void rspCopyAsMask(
					RImage* pimSrc,
					RImage* pimDst,
					UCHAR ucVal);


//===========================================================================
#endif

