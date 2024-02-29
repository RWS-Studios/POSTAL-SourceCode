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
// Do uncompressed transparent copying!
#ifdef PATHS_IN_INCLUDES
	#include "GREEN/BLiT/BLIT.H"
#else
	#include "BLIT.H"
#endif

#ifdef PATHS_IN_INCLUDES
	#include "GREEN/BLiT/_BlitInt.H"
#else
	#include "_BlitInt.H" 
#endif

//========================================================================================
//========================================================================================

// Insert this into your functions... It assumes pre-clipped, pre-sorted,
// pre-validated rect copy: (Pitch will be sign based!)
//
template <class PIXSIZE>
inline void _BLiTT(PIXSIZE ucTransparent,PIXSIZE* pSrc,PIXSIZE* pDst,long lSrcPitch, long lDstPitch,
						short sHeight,short	sPixWidth)
	{
	union	{
		PIXSIZE *w;
		UCHAR	*b;
		} pSrcLine,pDstLine;

	int i;

	pSrcLine.w = pSrc;
	pDstLine.w = pDst;

	while (sHeight--)
		{
		pSrc = pSrcLine.w;
		pDst = pDstLine.w;

		i = sPixWidth;
		while (i--) 
			{
			if (*pSrc != ucTransparent) *pDst = *pSrc;
			pDst++;pSrc++;
			}

		pSrcLine.b += lSrcPitch;
		pDstLine.b += lDstPitch;

		}
	}



//*****************************************************************************
// This is the main controller... It clips in pixels, then thinks in bytes:
// if prSrc == NULL, no source clipping will occure
// if prDst == NULL, it will clip to the CImage
//
short	rspBlitT(ULONG ucTransparent,RImage* pimSrc,RImage* pimDst,short sSrcX,short sSrcY,short sDstX,
			  short sDstY,short sW,short sH,const RRect* prDst,const RRect* prSrc)
	{
	short sClip;

	// 1) preliminary parameter validation:
#ifdef _DEBUG

	if ((pimSrc == NULL) || (pimDst == NULL))
		{
		TRACE("rspBlitT: null CImage* passed\n");
		return -1;
		}

	if ( (sW < 1) || (sH < 1) )
		{
		TRACE("rspBlitT: zero or negative area passed\n");
		return -1;
		}

	if ( !ImageIsUncompressed(pimSrc->m_type) || 
			!ImageIsUncompressed(pimDst->m_type) )
		{
		TRACE("rspBlitT: To BLiT using COMPRESSED Images, you must specify"
			"your parameters differently (see BLiT.DOC)!\n");
		return -1;
		}

#endif

	
	// 2) Optional Source Clipping
	if (prSrc)
		{
		// clip rect to source
		// clip against user values
		sClip = prSrc->sX - sDstX; // positive = clipped
		if (sClip > 0) { sW -= sClip; sSrcX += sClip; sDstX = prSrc->sX; }
		sClip = prSrc->sY - sDstY; // positive = clipped
		if (sClip > 0) { sH -= sClip; sSrcY += sClip; sDstY = prSrc->sY; }
		sClip = sDstX + sW - prSrc->sX - prSrc->sW; // positive = clipped
		if (sClip > 0) { sW -= sClip; }
		sClip = sDstY + sH - prSrc->sY - prSrc->sH; // positive = clipped
		if (sClip > 0) { sH -= sClip; }

		if ( (sW <= 0) || (sH <= 0) ) return -1; // clipped out!
		}

	// 2) Source clipping is SO critical in locked screen stuff, that we MUST check it:

#ifdef _DEBUG 

	else // no source clipping:
		{
		if ( (sSrcX < 0) || (sSrcY < 0) ||
			((sSrcX + sW) > pimSrc->m_sWidth) || ((sSrcY + sH) > pimSrc->m_sHeight) )
			{
			TRACE("rspBlitT:  Gone outside source buffer.  Must source clip.\n");
			return -1;
			}
		}

#endif


	// 3) Destination Clipping:
	if (prDst)
		{
		// clip against user values
		sClip = prDst->sX - sDstX; // positive = clipped
		if (sClip > 0) { sW -= sClip; sSrcX += sClip; sDstX = prDst->sX; }
		sClip = prDst->sY - sDstY; // positive = clipped
		if (sClip > 0) { sH -= sClip; sSrcY += sClip; sDstY = prDst->sY; }
		sClip = sDstX + sW - prDst->sX - prDst->sW; // positive = clipped
		if (sClip > 0) { sW -= sClip; }
		sClip = sDstY + sH - prDst->sY - prDst->sH; // positive = clipped
		if (sClip > 0) { sH -= sClip; }

		if ( (sW <= 0) || (sH <= 0) ) return -1; // clipped out!
		}
	else	
		{
		// clip against full destination buffer
		if (sDstX < 0) { sW += sDstX; sSrcX -= sDstX; sDstX = 0; }
		if (sDstY < 0) { sH += sDstY; sSrcY -= sDstY; sDstY = 0; }
		sClip = sDstX + sW - pimDst->m_sWidth; // positive = clipped
		if (sClip > 0) sW -= sClip; // positive = clipped
		sClip = sDstY + sH - pimDst->m_sHeight; // positive = clipped
		if (sClip > 0) sH -= sClip; // positive = clipped

		if ((sW <= 0) || (sH <= 0)) return -1; // fully clipped
		}

	//**************  INSERT BUFFER HOOKS HERE!  ************************

	// do OS based copying!
	short sNeedToUnlock = 0; // will be the name of a buffer to unlock.

	short sBlitTypeSrc = 0;
	short sBlitTypeDst = 0;

	// IN RELEASE MODE, GIVE THE USER A CHANCE:
#ifndef _DEBUG

	//if (gsScreenLocked || gsBufferLocked) goto BLIT_PRELOCKED;

#endif

	// IN THIS IMPLEMENTATION, we must do LOCK, BLiT, UNLOCK, so I
	// must record which UNLOCK (if any) needs to be done AFTER the BLiT
	// has completed. (Lord help me if a blit gets interrupted)
	if (pimSrc->m_type == RImage::IMAGE_STUB) sBlitTypeSrc = (short)((long)pimSrc->m_pSpecial);
	if (pimDst->m_type == RImage::IMAGE_STUB) sBlitTypeDst = (short)((long)pimDst->m_pSpecial);

	switch ( (sBlitTypeSrc<<3) + sBlitTypeDst) // 0 = normal image
		{
		case (BUF_MEMORY<<3) + 0: // system buffer to an image
			// need to lock / unlock this one:
/*
			if (rspLockBuffer()
				!=0)
				{
				TRACE("rspBlitT: Unable to lock the system buffer, failed!\n");
				return -1;
				}
			// Locked the system buffer, remember to unlock it:
			sNeedToUnlock = BUF_MEMORY;	
*/
		break;

		case (0<<3) + BUF_MEMORY: // image to system buffer
/*
			// need to lock / unlock this one:
			if (rspLockBuffer()
				!=0)
				{
				TRACE("rspBlitT: Unable to lock the system buffer, failed!\n");
				return -1;
				}
			// Locked the system buffer, remember to unlock it:
			sNeedToUnlock = BUF_MEMORY;		
*/
		break;

		case (BUF_VRAM<<3) + 0: // front screen to image
			// need to lock / unlock this one:
/*
			if (rspLockScreen()
				!=0)
				{
				TRACE("rspBlitT: Unable to lock the OnScreen system buffer, failed!\n");
				return -1;
				}
			// Locked the front VRAM, remember to unlock it:
			sNeedToUnlock = BUF_VRAM;		
*/
		break;

		case (0<<3) + BUF_VRAM: // image to front screen
/*
			// need to lock / unlock this one:
			if (rspLockScreen()
				!=0)
				{
				TRACE("rspBlitT: Unable to lock the OnScreen system buffer, failed!\n");
				return -1;
				}
			// Locked the front VRAM, remember to unlock it:
			sNeedToUnlock = BUF_VRAM;			
*/
		break;

		// HOOK the special case of sytem buffer to front VRAM
		case (BUF_MEMORY<<3) + BUF_VRAM: // system buffer to front screen

			rspUpdateDisplay(sDstX,sDstY,sW,sH); 
			return 0; // DONE!!!!!
		break;

		case (BUF_VRAM<<3) + BUF_MEMORY: // front screen to system buffer
//			sNeedToUnlock = (BUF_VRAM<<3) + BUF_MEMORY;

		break;

		case 0:	// image to image, no need to lock!
		break;

		default:
			TRACE("rspBlitT: This type of copy is not yet supported.\n");
			return -1;
		}

//BLIT_PRELOCKED:
	//********************************************************************

	// Check for locking error:
	if (!pimDst->m_pData)
		{
		TRACE("rspBlitT: Null data - possible locking error.\n");
		return FAILURE;
		}

	// Done clipping, convert to bytes to find best alignment:
	// Currently based on source, assumes source = destination depth:
	switch (pimSrc->m_sDepth)
		{
		case 16:
			sDstX <<= 1;
			sSrcX <<= 1;
			sW <<= 1;
		break;
		case 32:
			sDstX <<= 2;
			sSrcX <<= 2;
			sW <<= 2;
		break;
		case 24:
			sDstX += (sDstX << 1);
			sSrcX += (sSrcX << 1);
			sW += (sW << 1);
		break;
		// 8-bit needs nothing...
		}

	// Calculate memory offsets using signed pitch:
	U8* pSrc = pimSrc->m_pData + sSrcX + pimSrc->m_lPitch * sSrcY;
	U8* pDst = pimDst->m_pData + sDstX + pimDst->m_lPitch * sDstY;
	
	// Copy based on pixel size:
	switch (pimDst->m_sDepth)
		{
		case 8:
			_BLiTT((UCHAR)ucTransparent,(U8*)pSrc,(U8*)pDst,pimSrc->m_lPitch,pimDst->m_lPitch,sH,sW); 
		break;
		case 16:
			_BLiTT((USHORT)ucTransparent,(U16*)pSrc,(U16*)pDst,(pimSrc->m_lPitch),(pimDst->m_lPitch),sH,short(sW>>1)); 
		break;
		case 32:
			_BLiTT((ULONG)ucTransparent,(U32*)pSrc,(U32*)pDst,(pimSrc->m_lPitch),(pimDst->m_lPitch),sH,short(sW>>2)); 
		break;
		default:
			TRACE("rspBlitT: color depth not supported.\n");
		}

	// IN RELEASE MODE, GIVE THE USER A CHANCE:
#ifndef _DEBUG

//	if (gsScreenLocked || gsBufferLocked) goto BLIT_DONTUNLOCK;

#endif

	//********************
	// OS_SPECIFIC:
	//********************  UNLOCK WHATEVER YOU NEED TO
	switch (sNeedToUnlock)
		{
		case 0:
		break;

		case BUF_MEMORY:
	//		rspUnlockBuffer();
		break;
		
		case BUF_VRAM:
	//		rspUnlockScreen();
		break;
		
		case BUF_VRAM2:
			rspUnlockVideoFlipPage();
		break;

		case (BUF_VRAM<<3) + BUF_MEMORY:
//			rspUnlockBuffer();
//			rspUnlockScreen();
		break;
		
		default:
			TRACE("rspBlitT:  Unlocking error!\n");
		}

//BLIT_DONTUNLOCK:	
	return 0;
	}

