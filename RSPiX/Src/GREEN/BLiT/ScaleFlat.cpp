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
// This is the scaling module for uncompressed 8-bit images
// It will hopefully be a strong candidate forthe RSPiX inliner.
//
// Anticipating this, it will do ROW blitting as separate inline
// functions.  Whatcha gonna do, man?
//
#ifdef PATHS_IN_INCLUDES
	#include "GREEN/BLiT/BLIT.H"
	#include "GREEN/BLiT/_BlitInt.H"
//	#include "GREEN/BLiT/_ic.h"
#else
	#include "BLIT.H"
	#include "_BlitInt.H" 
//	#include "_ic.h" 
#endif

// For now, do the shrink case only...
// This algorithm is based entirely on 15-bit unsigned fractions, allowing
// scaling by 30,000 times without overflow! A bit of overkil, but hey.
// I am leaving in the possibility of mirroring...

//-------- These macros could by inlined once the inliner is on line:

//=========================================== SHRINK, DEST BASED....
// This is pixel size independent:
// pSrc always increments rightwards...
//
#define INC_FRAC_H(pSrc,sDel,sInc,sNum,sDen)	\
	pSrc+=sDel; sNum+=sInc; if (sNum >= sDen) { sNum -= sDen; pSrc++; }

#define INC_FRAC_V(pSrc,sDel,sInc,sNum,lSrcP,sDen)	\
	pSrc+=sDel; sNum+=sInc; if (sNum >= sDen) { sNum -= sDen; pSrc+=lSrcP; }

#define SET_PIX_OPAQUE(pSrc,pDst)	\
	*pDst = *pSrc;

#define SET_PIX_CLEAR(pSrc,pDst,chroma,tempPix)	\
	if ( (tempPix = *pSrc) != chroma) *pDst = *pSrc;

// sSmall is the denominator..
#define INIT_IMPROPER_FRACTION_H(sSmall,sLarge,sDel,sInc,sNumi)	\
	sDel = sLarge / sSmall; sInc = sLarge - sSmall * sDel; sNumi = (sSmall>>1);

// sSmall is the denominator..
#define INIT_IMPROPER_FRACTION_V(sSmall,sLarge,lDel,sInc,sNumi,lSrcP)	\
	lDel = (sLarge / sSmall); sInc = sLarge - sSmall * lDel; sNumi = (sSmall>>1); lDel *= lSrcP;

// The left show value is the # of pixels from sCount to show right...
// It may only be followed with a single command!
// It assumes sCound has ALREADY been decremented!
//
#define LCLIP_PIX(sCount,sShow)	\
	if (sCount < sShow)

//=========================================== MAGNIFY, DEST BASED....=================
// (Source based magnify is faster, but also harder...)
//

// In this rare configuration, there is no del!
//
// sLarge is the denominator.... DO NOT USE FOR 1 to 1!
// sSmall is the incrementor (sInc!)
//
#define INIT_PROPER_FRACTION(sLarge,sNumi)	\
	sNumi = (sLarge >> 1);

// sLarge = denominator, sSmall = sInc
#define INC_PFRAC_H(pSrc,sInc,sNum,sDen)	\
	sNum += sInc; if (sNum >= sDen) { sNum -= sDen; pSrc++; }

// sLarge = denominator, sSmall = sInc
#define INC_PFRAC_V(pSrc,sInc,sNum,sDen,lSrcP)	\
	sNum += sInc; if (sNum >= sDen) { sNum -= sDen; pSrc+=lSrcP; }

//====================================================================================

// If we break out a million little blit's, than we can combine all the if's into one
// bit switch statement.
// I believe the memptrs are to the UNCLIPPED rects...
//
inline void _Blit_HS_VS_C(
							UCHAR* pSrc,long lSrcP,short sSrcW,short sSrcH,
							UCHAR* pDst,long lDstP,short sDstW,short sDstH,
							short sClipL,short sClipT,short sClipR,short sClipB,
							short sMirrorH,short sMirrorV)
	{
	//-------------------------------------------------------------------------
	// Set up vertical variables:
	//
	UCHAR*	pSrcLine = pSrc,*pDstLine = pDst;
	long	lDelMemY;
	short	sIncY,sNumY,sClipH;

	if (sMirrorV == -1) lDstP = -lDstP; // make sure pDst is correctly positioned
	
	// ASSUME SHRINKING FOR NOW:
	//************ THIS CHANGES FOR MAGNIFY!
	INIT_IMPROPER_FRACTION_V(sDstH,sSrcH,lDelMemY,sIncY,sNumY,lSrcP) // sDstH is the numerator
	sClipH = sDstH - sClipB - sClipT; // Fully clipped...
	//-------------------------------------------------------------------------
	// Set up horizontal variables:
	//
	short	sDelX,sIncX,sNumX,sClipW,sShowW;
	// ASSUME SHRINKING FOR NOW:
	INIT_IMPROPER_FRACTION_H(sDstW,sSrcW,sDelX,sIncX,sNumX) // sDstW is the numerator
	sClipW = sDstW - sClipR; // right clip...
	sShowW = sClipW - sClipL;

	short i;
	//-------------------------------------------------------------------------
	// Do vertical clipping:
	if (sClipT)
		while (sClipT--)
			{
			// Do a vertical move:
			INC_FRAC_V(pSrcLine,lDelMemY,sIncY,sNumY,lSrcP,sDstH)
			pDstLine += lDstP; // allow for vertical mirroring..
			}
	//-------------------------------------------------------------------------
	// Do horizontal clipping...
	if (sClipL)
		{
		while (sClipH--)
			{
			pSrc = pSrcLine; pDst = pDstLine; // synch position:
			sNumX = (sDstW>>1);
			UCHAR ucPix;
			i = sClipW;

			while (i--)
				{
				if (i < sShowW)
					SET_PIX_CLEAR(pSrc,pDst,UCHAR(0),ucPix)
				INC_FRAC_H(pSrc,sDelX,sIncX,sNumX,sDstW)
				pDst += sMirrorH;
				}

			INC_FRAC_V(pSrcLine,lDelMemY,sIncY,sNumY,lSrcP,sDstH)
			pDstLine += lDstP; // allow for vertical mirroring..
			}
		}
	else // no horizontal clipping:
		{
		while (sClipH--)
			{
			pSrc = pSrcLine; pDst = pDstLine; // synch position:
			sNumX = (sDstW>>1);
			i = sClipW;
			UCHAR ucPix;

			while (i--)
				{
				SET_PIX_CLEAR(pSrc,pDst,UCHAR(0),ucPix)
				INC_FRAC_H(pSrc,sDelX,sIncX,sNumX,sDstW)
				pDst += sMirrorH;
				}

			INC_FRAC_V(pSrcLine,lDelMemY,sIncY,sNumY,lSrcP,sDstH)
			pDstLine += lDstP; // allow for vertical mirroring..
			}
		}
	}


//====================================================================================

// If we break out a million little blit's, than we can combine all the if's into one
// bit switch statement.
// I believe the memptrs are to the UNCLIPPED rects...
//
inline void _Blit_HM_VM_C(
							UCHAR* pSrc,long lSrcP,short sSrcW,short sSrcH,
							UCHAR* pDst,long lDstP,short sDstW,short sDstH,
							short sClipL,short sClipT,short sClipR,short sClipB,
							short sMirrorH,short sMirrorV)
	{
	//-------------------------------------------------------------------------
	// Set up vertical variables:
	//
	UCHAR*	pSrcLine = pSrc,*pDstLine = pDst;
	short	sNumY,sClipH;

	if (sMirrorV == -1) lDstP = -lDstP; // make sure pDst is correctly positioned
	
	// ASSUME ENLARGING FOR NOW:
	//************ THIS CHANGES FOR SHRINK!
	INIT_PROPER_FRACTION(sDstH,sNumY) // sDstH is the denominator
	sClipH = sDstH - sClipB - sClipT; // Fully clipped...
	//-------------------------------------------------------------------------
	// Set up horizontal variables:
	//
	short	sNumX,sClipW,sShowW;
	// ASSUME ENLARGING FOR NOW:
	INIT_PROPER_FRACTION(sDstW,sNumX) // sDstW is the denominator
	sClipW = sDstW - sClipR; // right clip...
	sShowW = sClipW - sClipL;

	short i;
	//-------------------------------------------------------------------------
	// Do vertical clipping:
	if (sClipT)
		while (sClipT--)
			{
			// Do a vertical move:
			// sIncY = sSrcH, sDstY = denominator
			INC_PFRAC_V(pSrcLine,sSrcH,sNumY,sDstH,lSrcP)
			pDstLine += lDstP; // allow for vertical mirroring..
			}
	//-------------------------------------------------------------------------
	// Do horizontal clipping...
	if (sClipL)
		{
		while (sClipH--)
			{
			pSrc = pSrcLine; pDst = pDstLine; // synch position:
			sNumX = (sDstW>>1);
			UCHAR ucPix;
			i = sClipW;

			while (i--)
				{
				if (i < sShowW)
					SET_PIX_CLEAR(pSrc,pDst,UCHAR(0),ucPix)
				INC_PFRAC_H(pSrc,sSrcW,sNumX,sDstW)
				pDst += sMirrorH;
				}

			INC_PFRAC_V(pSrcLine,sSrcH,sNumY,sDstH,lSrcP)
			pDstLine += lDstP; // allow for vertical mirroring..
			}
		}
	else // no horizontal clipping:
		{
		while (sClipH--)
			{
			pSrc = pSrcLine; pDst = pDstLine; // synch position:
			sNumX = (sDstW>>1);
			i = sClipW;
			UCHAR ucPix;

			while (i--)
				{
				SET_PIX_CLEAR(pSrc,pDst,UCHAR(0),ucPix)
				INC_PFRAC_H(pSrc,sSrcW,sNumX,sDstW)
				pDst += sMirrorH;
				}

			INC_PFRAC_V(pSrcLine,sSrcH,sNumY,sDstH,lSrcP)
			pDstLine += lDstP; // allow for vertical mirroring..
			}
		}
	}

//====================================================================================

// If we break out a million little blit's, than we can combine all the if's into one
// bit switch statement.
// I believe the memptrs are to the UNCLIPPED rects...
//
inline void _Blit_HS_VM_C(
							UCHAR* pSrc,long lSrcP,short sSrcW,short sSrcH,
							UCHAR* pDst,long lDstP,short sDstW,short sDstH,
							short sClipL,short sClipT,short sClipR,short sClipB,
							short sMirrorH,short sMirrorV)
	{
	//-------------------------------------------------------------------------
	// Set up vertical variables:
	//
	UCHAR*	pSrcLine = pSrc,*pDstLine = pDst;
	short	sNumY,sClipH;

	if (sMirrorV == -1) lDstP = -lDstP; // make sure pDst is correctly positioned
	
	// ASSUME ENLARGING FOR NOW:
	//************ THIS CHANGES FOR SHRINK!
	INIT_PROPER_FRACTION(sDstH,sNumY) // sDstH is the denominator
	sClipH = sDstH - sClipB - sClipT; // Fully clipped...
	//-------------------------------------------------------------------------
	// Set up horizontal variables:
	//
	short	sDelX,sIncX,sNumX,sClipW,sShowW;
	// ASSUME SHRINKING FOR NOW:
	INIT_IMPROPER_FRACTION_H(sDstW,sSrcW,sDelX,sIncX,sNumX) // sDstW is the numerator
	sClipW = sDstW - sClipR; // right clip...
	sShowW = sClipW - sClipL;

	short i;
	//-------------------------------------------------------------------------
	// Do vertical clipping:
	if (sClipT)
		while (sClipT--)
			{
			// Do a vertical move:
			// sIncY = sSrcH, sDstY = denominator
			INC_PFRAC_V(pSrcLine,sSrcH,sNumY,sDstH,lSrcP)
			pDstLine += lDstP; // allow for vertical mirroring..
			}
	//-------------------------------------------------------------------------
	// Do horizontal clipping...
	if (sClipL)
		{
		while (sClipH--)
			{
			pSrc = pSrcLine; pDst = pDstLine; // synch position:
			sNumX = (sDstW>>1);
			UCHAR ucPix;
			i = sClipW;

			while (i--)
				{
				if (i < sShowW)
					SET_PIX_CLEAR(pSrc,pDst,UCHAR(0),ucPix)
				INC_FRAC_H(pSrc,sDelX,sIncX,sNumX,sDstW)
				pDst += sMirrorH;
				}

			INC_PFRAC_V(pSrcLine,sSrcH,sNumY,sDstH,lSrcP)
			pDstLine += lDstP; // allow for vertical mirroring..
			}
		}
	else // no horizontal clipping:
		{
		while (sClipH--)
			{
			pSrc = pSrcLine; pDst = pDstLine; // synch position:
			sNumX = (sDstW>>1);
			i = sClipW;
			UCHAR ucPix;

			while (i--)
				{
				SET_PIX_CLEAR(pSrc,pDst,UCHAR(0),ucPix)
				INC_FRAC_H(pSrc,sDelX,sIncX,sNumX,sDstW)
				pDst += sMirrorH;
				}

			INC_PFRAC_V(pSrcLine,sSrcH,sNumY,sDstH,lSrcP)
			pDstLine += lDstP; // allow for vertical mirroring..
			}
		}
	}

//====================================================================================

// If we break out a million little blit's, than we can combine all the if's into one
// bit switch statement.
// I believe the memptrs are to the UNCLIPPED rects...
//
inline void _Blit_HM_VS_C(
							UCHAR* pSrc,long lSrcP,short sSrcW,short sSrcH,
							UCHAR* pDst,long lDstP,short sDstW,short sDstH,
							short sClipL,short sClipT,short sClipR,short sClipB,
							short sMirrorH,short sMirrorV)
	{
	//-------------------------------------------------------------------------
	// Set up vertical variables:
	//
	UCHAR*	pSrcLine = pSrc,*pDstLine = pDst;
	long	lDelMemY;
	short	sIncY,sNumY,sClipH;

	if (sMirrorV == -1) lDstP = -lDstP; // make sure pDst is correctly positioned
	
	// ASSUME SHRINKING FOR NOW:
	//************ THIS CHANGES FOR MAGNIFY!
	INIT_IMPROPER_FRACTION_V(sDstH,sSrcH,lDelMemY,sIncY,sNumY,lSrcP) // sDstH is the numerator
	sClipH = sDstH - sClipB - sClipT; // Fully clipped...
	//-------------------------------------------------------------------------
	// Set up horizontal variables:
	//
	short	sNumX,sClipW,sShowW;
	// ASSUME ENLARGING FOR NOW:
	INIT_PROPER_FRACTION(sDstW,sNumX) // sDstW is the denominator
	sClipW = sDstW - sClipR; // right clip...
	sShowW = sClipW - sClipL;

	short i;
	//-------------------------------------------------------------------------
	// Do vertical clipping:
	if (sClipT)
		while (sClipT--)
			{
			// Do a vertical move:
			INC_FRAC_V(pSrcLine,lDelMemY,sIncY,sNumY,lSrcP,sDstH)
			pDstLine += lDstP; // allow for vertical mirroring..
			}
	//-------------------------------------------------------------------------
	// Do horizontal clipping...
	if (sClipL)
		{
		while (sClipH--)
			{
			pSrc = pSrcLine; pDst = pDstLine; // synch position:
			sNumX = (sDstW>>1);
			UCHAR ucPix;
			i = sClipW;

			while (i--)
				{
				if (i < sShowW)
					SET_PIX_CLEAR(pSrc,pDst,UCHAR(0),ucPix)
				INC_PFRAC_H(pSrc,sSrcW,sNumX,sDstW)
				pDst += sMirrorH;
				}

			INC_FRAC_V(pSrcLine,lDelMemY,sIncY,sNumY,lSrcP,sDstH)
			pDstLine += lDstP; // allow for vertical mirroring..
			}
		}
	else // no horizontal clipping:
		{
		while (sClipH--)
			{
			pSrc = pSrcLine; pDst = pDstLine; // synch position:
			sNumX = (sDstW>>1);
			i = sClipW;
			UCHAR ucPix;

			while (i--)
				{
				SET_PIX_CLEAR(pSrc,pDst,UCHAR(0),ucPix)
				INC_PFRAC_H(pSrc,sSrcW,sNumX,sDstW)
				pDst += sMirrorH;
				}

			INC_FRAC_V(pSrcLine,lDelMemY,sIncY,sNumY,lSrcP,sDstH)
			pDstLine += lDstP; // allow for vertical mirroring..
			}
		}
	}



//-----------------------------------------------------------------------------------
// templated overloads for selecting different inlines...

typedef U32 CHOOSE_MAGNIFY;
typedef S32 CHOOSE_SHRINK;
typedef U32 CHOOSE_HCLIP;
typedef S32 CHOOSE_NO_HCLIP;
typedef U32 CHOOSE_OPAQUE;
typedef S32 CHOOSE_TRANSPARENT;

//-----------------------------------------------------------------------------------

// at the internal level, assume all values have been verified..
//
inline short	_rspBlit(UCHAR* pSrc,long lSrcP,short sSrcW,short sSrcH,
								UCHAR* pDst,long lDstP,short sDstW,short sDstH)
	{
	return 0;
	}

// do the full transparent case with clipping:
// (long version... overload shorter version...)
// FOR THE DEMO, do ONLY shrinking, and do clipping AFTER shrinking.....
//
short rspBlitT(RImage* pimSrc,RImage* pimDst,RRect* prSrc,const RRect* prDst,
				  const RRect* prDstClip,const RRect* prSrcClip)
	{

#ifdef _DEBUG

	if (!pimSrc || !pimDst)
		{
		TRACE("rspBlitT: NULL images passed!\n");
		return -1;
		}

	// do type checking....
	//------------------------------------------------------------------------------
	if (!ImageIsUncompressed(pimSrc->m_type))
		{
		TRACE("rspBlitT: Only use this function with uncompressed Images.\n");
		return -1;
		}

	if (!ImageIsUncompressed(pimDst->m_type))
		{
		TRACE("rspBlitT: Only use this function with uncompressed Images.\n");
		return -1;
		}

	//------------------------------------------------------------------------------
	// do depth checking...
	//------------------------------------------------------------------------------
	if ((pimSrc->m_sDepth != 8) || (pimDst->m_sDepth != 8))
		{
		TRACE("rspBlitT: Currently, only 8-bit Images are fully supported.\n");
		return -1;
		}

#endif

	//------------------------------------------------------------------------------
	// do optional source clipping
	//------------------------------------------------------------------------------
	// NOTE THAT THESE ARE THE WIDTH AND HEIGHT WHICH ARE ACTUALLY SCALED
	//
	short sSrcX = prSrc->sX, sSrcY = prSrc->sY, sSrcW = prSrc->sW, sSrcH = prSrc->sH;

	// TOO complex for now as it drastically alters the scaling...

	if (prSrcClip)
		{
		TRACE("rspBlitT: Source clipping not yet available...\n");
		return -1;
		}

	//------------------------------------------------------------------------------
	// do optional destination clipping
	//------------------------------------------------------------------------------

	// Destination clippiong rectangle:
	// NOTE: effective destination clipping done at a lower level.
	// Only source clipping would effect this...
	//
	short sDstClipX = 0, sDstClipY = 0;
	short	sDstClipW = pimDst->m_sWidth, sDstClipH = pimDst->m_sHeight;

	// NOT FOR CLIPPING PURPOSES -> FIXED UINPUT VALUES!
	short sDstX = prDst->sX, sDstY = prDst->sY, sDstW = prDst->sW, sDstH = prDst->sH;

	if (prDstClip)
		{
		sDstClipX = prDstClip->sX;
		sDstClipY = prDstClip->sY;
		sDstClipW = prDstClip->sW;
		sDstClipH = prDstClip->sH;
		}

	//********************* MIRROR PART I => PRE CLIP:
	short sMirrorH = 1,sMirrorV = 1;

	if (sDstW < 0)
		{
		sMirrorH = -1; // flip the destination square's edges...
		sDstW = -sDstW;
		sDstX -= (sDstW - 1);
		}

	if (sDstH < 0)
		{
		sMirrorV = -1; // flip the destination square's edges...
		sDstH = -sDstH;
		sDstY -= (sDstH - 1);
		}
	//*********************

	//-------- Do the clipping:
	short sClipL,sClipR,sClipT,sClipB; // positive = clipped

	sClipL = sDstClipX - sDstX; if (sClipL < 0) sClipL = 0;
	sClipT = sDstClipY - sDstY; if (sClipT < 0) sClipT = 0;
	sClipR = (sDstX + sDstW - sDstClipX - sDstClipW); if (sClipR < 0) sClipR = 0;
	sClipB = (sDstY + sDstH - sDstClipY - sDstClipH); if (sClipB < 0) sClipB = 0;

	if ( ((sClipL + sClipR) >= sDstW) || ((sClipT + sClipB) >= sDstH) )
		{
		return 0; // fully clipped out
		}

	//********************* MIRROR PART II => POST CLIP, flip back...
	if (sMirrorH == -1)
		{
		sDstX += (sDstW - 1);
		SWAP(sClipL,sClipR); // the drawing order is reversed...
		}

	if (sMirrorV == -1)
		{
		sDstY += (sDstH - 1);
		SWAP(sClipT,sClipB); // the drawing order is reversed...
		}


	//*********************

	//------------------------------------------------------------------------------
	// set up IC
	//------------------------------------------------------------------------------
	UCHAR*	pDst = pimDst->m_pData + pimDst->m_lPitch * sDstY + sDstX;
	UCHAR*	pSrc = pimSrc->m_pData + pimSrc->m_lPitch * sSrcY + sSrcX;

	if ((sDstW <= sSrcW) && (sDstH <= sSrcH))
		_Blit_HS_VS_C(pSrc,pimSrc->m_lPitch,sSrcW,sSrcH,
							pDst,pimDst->m_lPitch,sDstW,sDstH,
							sClipL,sClipT,sClipR,sClipB,sMirrorH,sMirrorV);

	else if ((sDstW > sSrcW) && (sDstH > sSrcH))
		_Blit_HM_VM_C(pSrc,pimSrc->m_lPitch,sSrcW,sSrcH,
							pDst,pimDst->m_lPitch,sDstW,sDstH,
							sClipL,sClipT,sClipR,sClipB,sMirrorH,sMirrorV);

	else if ((sDstW <= sSrcW) && (sDstH > sSrcH))
		_Blit_HS_VM_C(pSrc,pimSrc->m_lPitch,sSrcW,sSrcH,
							pDst,pimDst->m_lPitch,sDstW,sDstH,
							sClipL,sClipT,sClipR,sClipB,sMirrorH,sMirrorV);

	else 
		_Blit_HM_VS_C(pSrc,pimSrc->m_lPitch,sSrcW,sSrcH,
							pDst,pimDst->m_lPitch,sDstW,sDstH,
							sClipL,sClipT,sClipR,sClipB,sMirrorH,sMirrorV);

	return 0;
	}

// easy call overloads:
//
#if WIN32
	inline
#else
	extern
#endif
short rspBlitT(RImage* pimSrc,RImage* pimDst,short sDstX,short sDstY,
					short sDstW,short sDstH,const RRect* prDstClip)
	{
	RRect rSrc,rDst;

#ifdef _DEBUG

	if (!pimSrc || !pimDst)
		{
		TRACE("rspBlitT: NULL images passed!\n");
		return -1;
		}

#endif

	rSrc.sX = rSrc.sY = 0;
	rSrc.sW = pimSrc->m_sWidth;
	rSrc.sH = pimSrc->m_sHeight;

	rDst.sX = sDstX;
	rDst.sY = sDstY;
	rDst.sW = sDstW;
	rDst.sH = sDstH;

	return rspBlitT(pimSrc,pimDst,&rSrc,&rDst,prDstClip);
	}

// easy call overloads:
//
#if WIN32
	inline
#else
	extern
#endif
short rspBlitT(RImage* pimSrc,RImage* pimDst,short sDstX,short sDstY,
							 double dRatW,double dRatH,const RRect* prDstClip)
	{
#ifdef _DEBUG

	if (!pimSrc || !pimDst)
		{
		TRACE("rspBlitT: NULL images passed!\n");
		return -1;
		}

#endif

	return rspBlitT(pimSrc,pimDst,sDstX,sDstY,(short)(dRatW * pimSrc->m_sWidth),
		(short)(dRatH * pimSrc->m_sHeight),prDstClip);
	}
