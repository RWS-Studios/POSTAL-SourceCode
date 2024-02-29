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
// This file controls all of the uncompressed BLiTting about.
// It hooks references to special areas (screens & their buffers), and
// calls the appropriate functions...

#ifdef PATHS_IN_INCLUDES
	#include "GREEN/BLiT/BLIT.H"
	#include "GREEN/BLiT/_BlitInt.H"
#else
	#include "BLIT.H"
	#include "_BlitInt.H" 
#endif

	template <class PIXSIZE>
	void	rspClipPlot(PIXSIZE color, RImage* pimDst,short sX,short sY,const RRect* prClip)
	{

#ifdef _DEBUG

	if (pimDst == NULL)
		{
		TRACE("rspPlot: NULL Image passed.\n");
		return;
		}

	if (!ImageIsUncompressed(pimDst->m_type))
		{
		TRACE("rspPlot: Only uncompressed surfaces accepted.\n");
		return;
		}

#endif

	RRect rClip(0,0,pimDst->m_sWidth,pimDst->m_sHeight);
	if (prClip) rClip = *prClip;

	if ( (sX < rClip.sX) || (sY < rClip.sY) || (sX >= (rClip.sX + rClip.sW)) || 
		(sY >= (rClip.sY + rClip.sH) ) )
		{
		// Live clipping.
		return;
		}

	short sNeedToUnlock = 0;

	//if (gsScreenLocked || gsBufferLocked) goto PLOT_DONTLOCK;

	// removed locking and unlocking except where needed for special cases:

	switch ((short)(((long)pimDst->m_pSpecial))) // 0 = normal image
		{
		case 0: // normal image, buffer in image
		break;

		case BUF_MEMORY: // draw a rectangle in the system buffer
	/*
			// need to lock / unlock this one:
			if (rspLockBuffer()
				!=0)
				{
				TRACE("rspPlot: Unable to lock the system buffer, failed!\n");
				}
			// Locked the system buffer, remember to unlock it:
			sNeedToUnlock = BUF_MEMORY;	
	*/
		break;

		case BUF_VRAM: // draw a rectangle to the visible vram screen
			// need to lock / unlock this one:
	/*
			if (rspLockScreen()
				!=0)
				{
				TRACE("rspPlot: Unable to lock the OnScreen buffer, failed!\n");
				}
			// Locked the system buffer, remember to unlock it:
			sNeedToUnlock = BUF_VRAM;			
	*/
		break;

		case BUF_VRAM2: // draw a rectangle to the hidden VRAM plane
			// need to lock / unlock this one:
			if (rspLockVideoFlipPage((void**)&(pimDst->m_pData),&(pimDst->m_lPitch))
				!=0)
				{
				TRACE("rspPlot: Unable to lock the OffScreen buffer, failed!\n");
				}
			// Locked the system buffer, remember to unlock it:
			sNeedToUnlock = BUF_VRAM2;			
		break;

		default:
			TRACE("rspPlot: This type of copy is not yet supported.\n");
		}

//PLOT_DONTLOCK:

	// Special check for buffer not locked correctly:
	if (!pimDst->m_pData)
		{
		TRACE("rspPlot: NULL image data - potential locking error\n");
		return;
		}

	// DO THE PLOT!
	PIXSIZE* pTemp = (PIXSIZE*) (pimDst->m_pData + sY * pimDst->m_lPitch) + sX;
	*pTemp = color;

	//********************
	// OS_SPECIFIC:
	//********************  UNLOCK WHATEVER YOU NEED TO

	//if (gsScreenLocked || gsBufferLocked) goto PLOT_DONTUNLOCK;

	switch (sNeedToUnlock)
		{
		case 0:
		break;

		case BUF_MEMORY:
//			rspUnlockBuffer();
		break;
		
		case BUF_VRAM:
//			rspUnlockScreen();
		break;
		
		case BUF_VRAM2:
			rspUnlockVideoFlipPage();
		break;
		
		default:
			TRACE("rspDrawRect:  Unlocking error!\n");
		}

//PLOT_DONTUNLOCK:
	return;
	}

// force instantiation
template void rspClipPlot<UCHAR>(UCHAR color, RImage* pimDst,short sX,short sY,const RRect* prClip);
template void rspClipPlot<USHORT>(USHORT color, RImage* pimDst,short sX,short sY,const RRect* prClip);
template void rspClipPlot<ULONG>(ULONG color, RImage* pimDst,short sX,short sY,const RRect* prClip);


void instantiatePlot(void);
void instantiatePlot()
	{
	RImage* pim = NULL;

	rspPlot((UCHAR)0,pim,(short)0,(short)0);
	rspPlot((USHORT)0,pim,(short)0,(short)0);
	rspPlot((ULONG)0,pim,(short)0,(short)0);

	}

// This is not currently designed for 24-bit mode.
// Returns 0 if something found...
// The direction flags are non-zero to operate a direction.  They refer to
// the side shrinking.
//
template <class PIXSIZE>
short	rspLasso(PIXSIZE ignoreColor,RImage* pimSrc,short &x,short &y,short &w,short &h,
						int lFlag,int rFlag,int tFlag,int bFlag)
	{
	short sWordLen = sizeof(PIXSIZE);

#ifdef _DEBUG

	// Parameter Validation:
	if ((sWordLen != 1) && (sWordLen != 2) &&(sWordLen != 4))
		{
		TRACE("rspLasso: Pixel size not supported.\n");
		return -1;
		}

	if ( (x<0) || (y<0) || ( (x+w) > pimSrc->m_sWidth) || ( (y+h) > pimSrc->m_sHeight) )
		{
		TRACE("rspLasso: Outside Source Buffer!\n");
		return -1;
		}

	if ( !ImageIsUncompressed(pimSrc->m_type) )
		{
		TRACE("rspLasso: Only uncompressed RAM surfaces accepted.\n");
		return -1;
		}

	if ((w < 0) || (h < 0)) 
		{
		TRACE("rspLasso: Negative area passed.\n");
		return -1;
		}

#endif

	short btos[] = {-1,0,1,-1,2};
	PIXSIZE* pCursor = NULL;
	PIXSIZE* pCursorLine = NULL;
	long	lSkipV;
	short i,j;

	if (pimSrc->m_lPitch > 0)
		lSkipV = pimSrc->m_lPitch << btos[sWordLen];
	else
		lSkipV = -((-pimSrc->m_lPitch) << btos[sWordLen]);

	// Move the left wall, if specified:
	if (lFlag)
		{
		pCursorLine = (PIXSIZE*)pimSrc->m_pData + lSkipV * y + x;

		for (i=w;i>0;i--) // w will change!
			{
			pCursor = pCursorLine;
			for (j=0;j<h;j++)
				{
				if ( (*pCursor) != ignoreColor ) goto DoneL; // break all looping!
				pCursor += lSkipV;
				}
			x++;
			w--;
			pCursorLine++;
			}
		TRACE("rspLasso:  ERROR:  nothing found to lasso!\n");
		return -1;
		}
DoneL:
	// Move the right wall, if specified:
	if (rFlag)
		{
		pCursorLine = (PIXSIZE*)pimSrc->m_pData + lSkipV * y + (x + w - 1);

		for (i=w;i>0;i--) // w will change!
			{
			pCursor = pCursorLine;
			for (j=0;j<h;j++)
				{
				if ( (*pCursor) != ignoreColor ) goto DoneR; // break all looping!
				pCursor += lSkipV;
				}
			w--;
			pCursorLine--;
			}
		TRACE("rspLasso:  ERROR:  nothing found to lasso!\n");
		return -1;
		}
DoneR:
	// Move the top wall, if specified:
	if (tFlag)
		{
		pCursorLine = (PIXSIZE*)pimSrc->m_pData + lSkipV * y + x;

		for (i=h;i>0;i--) // h will change!
			{
			pCursor = pCursorLine;
			for (j=0;j<w;j++)
				{
				if ( (*pCursor) != ignoreColor ) goto DoneT; // break all looping!
				pCursor ++;
				}
			y++;
			h--;
			pCursorLine+=lSkipV;
			}
		TRACE("rspLasso:  ERROR:  nothing found to lasso!\n");
		return -1;
		}
DoneT:
	// Move the bottom wall, if specified:
	if (bFlag)
		{
		pCursorLine = (PIXSIZE*)pimSrc->m_pData + lSkipV * (y + h - 1) + x;

		for (i=h;i>0;i--) // h will change!
			{
			pCursor = pCursorLine;
			for (j=0;j<w;j++)
				{
				if ( (*pCursor) != ignoreColor ) goto DoneB; // break all looping!
				pCursor ++;
				}
			h--;
			pCursorLine-=lSkipV;
			}
		TRACE("rspLasso:  ERROR:  nothing found to lasso!\n");
		return -1;
		}
DoneB:

	return 0;	// lasso'd it!
	}


// force instantiation
template short rspLasso<UCHAR>(UCHAR ignoreColor,RImage* pimSrc,short &x,short &y,short &w,short &h,
						int lFlag,int rFlag,int tFlag,int bFlag);
template short rspLasso<USHORT>(USHORT ignoreColor,RImage* pimSrc,short &x,short &y,short &w,short &h,
						int lFlag,int rFlag,int tFlag,int bFlag);
template short rspLasso<ULONG>(ULONG ignoreColor,RImage* pimSrc,short &x,short &y,short &w,short &h,
						int lFlag,int rFlag,int tFlag,int bFlag);


// attempt to create the templated version in the lib:
void instantiateLasso(void);
void instantiateLasso()
	{
	RImage* pim = NULL;
	short i = 0;

	rspLasso( (UCHAR)i, pim, i,i,i,i, 1,1,1,1);
	rspLasso( (USHORT)i, pim, i,i,i,i, 1,1,1,1);
	rspLasso( (ULONG)i, pim, i,i,i,i, 1,1,1,1);

	}


// Insert this into your functions... It assumes pre-clipped, pre-sorted,
// pre-validated rect copy: (Pitch will be sign based!)
//
template <class PIXSIZE>
inline void _BLiT(PIXSIZE* pSrc,PIXSIZE* pDst,long lSrcPitch, long lDstPitch,
						short sHeight,short	sByteWidth)
	{
	union	{
		PIXSIZE *w;
		UCHAR	*b;
		} pSrcLine,pDstLine;

	int i;

	const static short SizeToShift[] = {0,0,1,0,2,0,0,0,3,0,0,0,0,0,0,0,4};
	//                                  0 1 2 3 4 5 6 7 8 9 a b c d e f *
	short sWordWidth = short(sByteWidth >> SizeToShift[sizeof(PIXSIZE)]);

	pSrcLine.w = pSrc;
	pDstLine.w = pDst;

	while (sHeight--)
		{
		pSrc = pSrcLine.w;
		pDst = pDstLine.w;

		i = sWordWidth;
		while (i--) *(pDst++) = *(pSrc++);

		pSrcLine.b += lSrcPitch;
		pDstLine.b += lDstPitch;

		}
	}

// This uses misaligned 32-bit copies to be "faster" than byte copying:
//
// But this is a garatied SegBus on ARM with NEON vectorisation, as misaligned are not possible
inline void _BLiT_MA(UCHAR* pSrc,UCHAR* pDst,long lSrcPitch, long lDstPitch,
						short sHeight,short	sWidth)
	{
#ifdef __ARM_NEON__
	while (sHeight--)
		{
		memcpy(pDst, pSrc, sWidth);
		pSrc += lSrcPitch;
		pDst += lDstPitch;
		}
#else
	union	{
		U32 *w;
		UCHAR	*b;
		} pSrcLine,pDstLine,pS,pD;

	short i;
	short sWordWidth = short (sWidth >> 2); //32 bit words....
	short sExtraWidth = sWidth - short(sWordWidth << 2);

	pS.b = pSrcLine.b = pSrc;
	pD.b = pDstLine.b = pDst;

	while (sHeight--)
		{
		pS.w = pSrcLine.w;
		pD.w = pDstLine.w;

		i = sWordWidth;
		while (i--) *(pD.w++) = *(pS.w++);
		// extra cleanup:
		i = sExtraWidth;
		while (i--) *(pD.b++) = *(pS.b++);

		pSrcLine.b += lSrcPitch;
		pDstLine.b += lDstPitch;

		}
#endif
	}

// Used only to punch a rectangle between two 128-bit aligned buffers.
// Source and Destination (X,Y) must be the same.
// Will widen your rectangle to take advantage of your bus!
// 
short	rspBlitA(RImage* pimSrc,RImage* pimDst,short sX,short sY,
				short sW,short sH,const RRect* prDst,const RRect* prSrc)
	{

#ifdef _DEBUG
	// Since all parameter validation is done in BLiT, we don't try here!

	if (!pimSrc || !pimDst)
		{
		TRACE("BLiTA: null images passed.\n");
		return -1;
		}

	if (pimSrc->m_sDepth == 24)
		{
		TRACE("BLiTA: 24-bit color NYI.  Try 32-bit color.\n");
		return -1;
		}

#endif

	// adjust for color depth based on source:
	short sDepth = pimSrc->m_sDepth >> 3; // depth in bytes (1,2,4)
	short sDepthToShift[] = {-1, 0, 1, -1, 2};
	short sDepthS = sDepthToShift[sDepth]; // convert to a shift value

	const long clChosenAlignment = 4; // in bytes
	// govern the best possible alignment based on pitch
	short sBestAlign = (short)(clChosenAlignment | pimSrc->m_lPitch | pimDst->m_lPitch);
	
	// in bytes!
	short sx1 = sX << sDepthS;
	short	sx2 = (sX + sW) << sDepthS;

	// Optimize the 8-bit case:
	//***********************************************************************
	if (sDepth == 1)
		{
		if (sBestAlign & 15) // try for 128-bit alignment
			{
			if (sx1 & 15) // widen to the left:
				{
				sx1 &= ~15;
				}

			if (sx2 & 15) // widen to the right
				{
				sx2 &= ~15;
				sx2 += 16;
				}

			return rspBlit(pimSrc,pimDst,sX,sY,sX,sY,sW,sH,prDst,prSrc);
			}
		else if (sBestAlign & 7) // try for 64-bit alignment
			{
			if (sx1 & 7) // widen to the left:
				{
				sx1 &= ~7;
				}

			if (sx2 & 7) // widen to the right
				{
				sx2 &= ~7;
				sx2 += 8;
				}

			return rspBlit(pimSrc,pimDst,sX,sY,sX,sY,sW,sH,prDst,prSrc);
			}
		else if (sBestAlign & 3) // try for 32-bit alignment
			{
			if (sx1 & 3) // widen to the left:
				{
				sx1 &= ~3;
				}

			if (sx2 & 3) // widen to the right
				{
				sx2 &= ~3;
				sx2 += 4;
				}

			return rspBlit(pimSrc,pimDst,sX,sY,sX,sY,sW,sH,prDst,prSrc);
			}
		else if (sBestAlign & 1) // try for 16-bit alignment
			{
			if (sx1 & 1) // widen to the left:
				{
				sx1 &= ~1;
				}

			if (sx2 & 1) // widen to the right
				{
				sx2 &= ~1;
				sx2 += 2;
				}

			return rspBlit(pimSrc,pimDst,sX,sY,sX,sY,sW,sH,prDst,prSrc);
			}
		}

	//***********************************************************************
	// Other color depths

	if (sBestAlign & 15) // try for 128-bit alignment
		{
		if (sx1 & 15) // widen to the left:
			{
			sx1 &= ~15;
			// snap left to sDepth
			sx1 = (sx1 >> sDepthS) << sDepthS;
			}

		if (sx2 & 15) // widen to the right
			{
			sx2 &= ~15;
			sx2 += 16;
			// snap right to sDepth
			sx2 = ((sx2 + sDepth - 1) >> sDepthS) << sDepthS;
			}

		sX = sx1 >> sDepthS;
		sW = (sx2 - sx1) >> sDepthS;
		return rspBlit(pimSrc,pimDst,sX,sY,sX,sY,sW,sH,prDst,prSrc);
		}
	else if (sBestAlign & 7) // try for 64-bit alignment
		{
		if (sx1 & 7) // widen to the left:
			{
			sx1 &= ~7;
			// snap left to sDepth
			sx1 = (sx1 >> sDepthS) << sDepthS;
			}

		if (sx2 & 7) // widen to the right
			{
			sx2 &= ~7;
			sx2 += 8;
			// snap right to sDepth
			sx2 = ((sx2 + sDepth - 1) >> sDepthS) << sDepthS;
			}

		sX = sx1 >> sDepthS;
		sW = (sx2 - sx1) >> sDepthS;
		return rspBlit(pimSrc,pimDst,sX,sY,sX,sY,sW,sH,prDst,prSrc);
		}
	else if (sBestAlign & 3) // try for 32-bit alignment
		{
		if (sx1 & 3) // widen to the left:
			{
			sx1 &= ~3;
			// snap left to sDepth
			sx1 = (sx1 >> sDepthS) << sDepthS;
			}

		if (sx2 & 3) // widen to the right
			{
			sx2 &= ~3;
			sx2 += 4;
			// snap right to sDepth
			sx2 = ((sx2 + sDepth - 1) >> sDepthS) << sDepthS;
			}

		sX = sx1 >> sDepthS;
		sW = (sx2 - sx1) >> sDepthS;
		return rspBlit(pimSrc,pimDst,sX,sY,sX,sY,sW,sH,prDst,prSrc);
		}
	else if (sBestAlign & 1) // try for 16-bit alignment
		{
		if (sx1 & 1) // widen to the left:
			{
			sx1 &= ~1;
			// snap left to sDepth
			sx1 = (sx1 >> sDepthS) << sDepthS;
			}

		if (sx2 & 1) // widen to the right
			{
			sx2 &= ~1;
			sx2 += 2;
			// snap right to sDepth
			sx2 = ((sx2 + sDepth - 1) >> sDepthS) << sDepthS;
			}

		sX = sx1 >> sDepthS;
		sW = (sx2 - sx1) >> sDepthS;
		return rspBlit(pimSrc,pimDst,sX,sY,sX,sY,sW,sH,prDst,prSrc);
		}

	return rspBlit(pimSrc,pimDst,sX,sY,sX,sY,sW,sH,prDst,prSrc);
	}

//*****************************************************************************
// This is the main controller... It clips in pixels, then thinks in bytes:
//
short	rspBlit(RImage* pimSrc,RImage* pimDst,short sSrcX,short sSrcY,short sDstX,
			  short sDstY,short sW,short sH,const RRect* prDst,const RRect* prSrc)
	{
	short sClip;

	// 1) preliminary parameter validation:
#ifdef _DEBUG

	if ((pimSrc == NULL) || (pimDst == NULL))
		{
		TRACE("BLiT: null RImage* passed\n");
		return -1;
		}

	if ( (sW < 1) || (sH < 1) )
		{
		TRACE("BLiT: zero or negative area passed\n");
		return -1;
		}

	if ( !ImageIsUncompressed(pimSrc->m_type) || 
			!ImageIsUncompressed(pimDst->m_type) )
		{
		TRACE("BLiT: To BLiT using COMPRESSED Images, you must specify"
			"your parameters differently (see BLiT.DOC)!\n");
		return -1;
		}

#endif

	
	// 2) Optional Source Clipping
	if (prSrc)
		{
		// clip rect to source
		// clip against user values
		/*
		sClip = prSrc->sX - sDstX; // positive = clipped
		if (sClip > 0) { sW -= sClip; sSrcX += sClip; sDstX = prSrc->sX; }
		sClip = prSrc->sY - sDstY; // positive = clipped
		if (sClip > 0) { sH -= sClip; sSrcY += sClip; sDstY = prSrc->sY; }
		sClip = sDstX + sW - prSrc->sX - prSrc->sW; // positive = clipped
		if (sClip > 0) { sW -= sClip; }
		sClip = sDstY + sH - prSrc->sY - prSrc->sH; // positive = clipped
		if (sClip > 0) { sH -= sClip; }
		*/

		sClip = prSrc->sX - sSrcX; // positive = clipped
		if (sClip > 0) { sW -= sClip; sSrcX += sClip; sDstX += sClip; }
		sClip = prSrc->sY - sSrcY; // positive = clipped
		if (sClip > 0) { sH -= sClip; sSrcY += sClip; sDstY += sClip; }
		sClip = sSrcX + sW - prSrc->sX - prSrc->sW; // positive = clipped
		if (sClip > 0) { sW -= sClip; }
		sClip = sSrcY + sH - prSrc->sY - prSrc->sH; // positive = clipped
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
			TRACE("BLiT:  Gone outside source buffer.  Must source clip.\n");
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
	//short sNeedToUnlock = 0; // will be the name of a buffer to unlock.

	short sBlitTypeSrc = 0;
	short sBlitTypeDst = 0;

	//	if (gsScreenLocked || gsBufferLocked) goto BLIT_PRELOCKED;

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
				TRACE("BLiT: Unable to lock the system buffer, failed!\n");
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
				TRACE("BLiT: Unable to lock the system buffer, failed!\n");
				return -1;
				}
			// Locked the system buffer, remember to unlock it:
			sNeedToUnlock = BUF_MEMORY;
*/
		break;

		case (BUF_VRAM<<3) + 0: // front screen to image
/*
			// need to lock / unlock this one:
			if (rspLockScreen()
				!=0)
				{
				TRACE("BLiT: Unable to lock the OnScreen system buffer, failed!\n");
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
				TRACE("BLiT: Unable to lock the OnScreen system buffer, failed!\n");
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
/*
			sNeedToUnlock = (BUF_VRAM<<3) + BUF_MEMORY;
*/
		break;

		case 0:	// image to image, no need to lock!
		break;

		default:
			TRACE("BLiT: This type of copy is not yet supported.\n");
			return -1;
		}

//BLIT_PRELOCKED:

	// Check for locking error:
	if (!pimDst->m_pData)
		{
		TRACE("BLiT: NULL data - possible locking error.\n");
		return FAILURE;
		}

	//********************************************************************
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

	// sSrcX, sDstX, and sSrcW are in BYTES:
	U8* pSrc = pimSrc->m_pData + sSrcX + pimSrc->m_lPitch * sSrcY;
	U8* pDst = pimDst->m_pData + sDstX + pimDst->m_lPitch * sDstY;
	
	// Determine Byte Alignment:
	short sAlign = sSrcX | sDstX | sW | 
		(short)( ABS(pimSrc->m_lPitch) | ABS(pimDst->m_lPitch) );

	// Do the rect copy as fast as possible!
	// we are passing the width in BYTES!
	//
	if ( (sAlign & 15) == 0 )
		{
		// 128-bit copy
		_BLiT((U128*)pSrc,(U128*)pDst,pimSrc->m_lPitch,pimDst->m_lPitch,sH,sW); 
		}
	else if ( (sAlign & 7) == 0)
		{
		// 64-bit copy
		_BLiT((U64*)pSrc,(U64*)pDst,pimSrc->m_lPitch,pimDst->m_lPitch,sH,sW); 
		}
	else if ( (sAlign & 3) == 0)
		{
		// 32-bit copy
		_BLiT((U32*)pSrc,(U32*)pDst,pimSrc->m_lPitch,pimDst->m_lPitch,sH,sW); 
		}
	else if ( (sAlign & 1) == 0)
		{
		// 16-bit copy
		_BLiT((U16*)pSrc,(U16*)pDst,pimSrc->m_lPitch,pimDst->m_lPitch,sH,sW); 
		}
	else
		{
		// 8-bit copy
		//*************  GAMBLE that misaligned 32-bit copies are faster than byte copies,
		// then handle extra off the end:
		if (sW < 16)
			_BLiT(pSrc,pDst,pimSrc->m_lPitch,pimDst->m_lPitch,sH,sW);
		else
			_BLiT_MA(pSrc,pDst,pimSrc->m_lPitch,pimDst->m_lPitch,sH,sW); 
		}

#if 0	// We always want the safety mode.
	// IN RELEASE MODE, GIVE THE USER A CHANCE:
	#ifndef _DEBUG

	//	if (gsScreenLocked || gsBufferLocked) goto BLIT_DONTUNLOCK;

	#endif
#endif

	//********************
	// OS_SPECIFIC:
	//********************  UNLOCK WHATEVER YOU NEED TO
#if 0
	switch (sNeedToUnlock)
		{
		case 0:
		break;

		case BUF_MEMORY:
//			rspUnlockBuffer();
		break;
		
		case BUF_VRAM:
//			rspUnlockScreen();
		break;
		
		case BUF_VRAM2:
//			rspUnlockVideoFlipPage();
		break;

		case (BUF_VRAM<<3) + BUF_MEMORY:
//			rspUnlockBuffer();
//			rspUnlockScreen();
		break;
		
		default:
			TRACE("BLiT:  Unlocking error!\n");
		}
#endif

//BLIT_DONTUNLOCK:	
	return 0;
	}

//************************************************************************** 
//************************************************************************** 
//************************************************************************** 

// Insert this into your functions... It assumes pre-clipped, pre-sorted,
// pre-validated rect copy: (Pitch will be sign based!)
//
template <class WORDSIZE>
inline void _ClearRect(WORDSIZE color,WORDSIZE* pDst,long lDstPitch,
						short sHeight,short	sByteWidth)
	{
	union	{
		WORDSIZE *w;
		UCHAR	*b;
		} pDstLine;

	short i,sWordWidth = sByteWidth;
	const static short SizeToShift[] = {0,0,1,0,2,0,0,0,3,0,0,0,0,0,0,0,4};
	//                                  0 1 2 3 4 5 6 7 8 9 a b c d e f *
	sWordWidth = short(sByteWidth >> SizeToShift[sizeof(WORDSIZE)]);
	pDstLine.w = pDst;

	while (sHeight--)
		{
		pDst = pDstLine.w;

		i = sWordWidth;
		while (i--) *(pDst++) = color;

		pDstLine.b += lDstPitch;
		}
	}

// Must make TC possible!  
//
short rspRect(U32 color,RImage* pimDst,short sX,short sY,short sW,short sH,RRect* prClip)
	{
	// A cheap hook for mono:
	if (pimDst->m_type == RImage::BMP1) // monochrome hook
		{
		return rspRectToMono(color,pimDst,sX,sY,sW,sH);
		}

	// 1) preliminary parameter validation:
#ifdef _DEBUG

	if (pimDst == NULL)
		{
		TRACE("rspRect: null RImage* passed\n");
		return -1;
		}

	/*
	if ( (sW < 1) || (sH < 1) )
		{
		TRACE("rspRect: zero or negative area passed\n");
		return -1;
		}
		*/

	if (!ImageIsUncompressed(pimDst->m_type)) 
		{
		TRACE("rspRect: Can only draw RECTs into uncompressed images!\n");
		return -1;
		}

#endif

	// UNMIRROR THE RECTANGLE:
	if (!sW && !sH) return 0; // nothing to draw

	if (sW < 0) { sX += (sW+1); sW = -sW; }
	if (sH < 0) { sY += (sH+1); sH = -sH; }

	short sClip;

	// 2) Destination Clipping:
	if (prClip)
		{
		// clip against user values
		sClip = prClip->sX - sX; // positive = clipped
		if (sClip > 0) { sW -= sClip; sX = prClip->sX; }
		sClip = prClip->sY - sY; // positive = clipped
		if (sClip > 0) { sH -= sClip; sY = prClip->sY; }
		sClip = sX + sW - prClip->sX - prClip->sW; // positive = clipped
		if (sClip > 0) { sW -= sClip; }
		sClip = sY + sH - prClip->sY - prClip->sH; // positive = clipped
		if (sClip > 0) { sH -= sClip; }

		if ( (sW <= 0) || (sH <= 0) ) return -1; // clipped out!
		}
	else	
		{
		// clip against full destination buffer
		if (sX < 0) { sW += sX; sX = 0; }
		if (sY < 0) { sH += sY; sY = 0; }
		sClip = sX + sW - pimDst->m_sWidth; // positive = clipped
		if (sClip > 0) sW -= sClip; // positive = clipped
		sClip = sY + sH - pimDst->m_sHeight; // positive = clipped
		if (sClip > 0) sH -= sClip; // positive = clipped

		if ((sW <= 0) || (sH <= 0)) return -1; // fully clipped
		}

	//**************  INSERT BUFFER HOOKS HERE!  ************************

	short sNeedToUnlock = 0; // will be the name of a buffer to unlock.
	short sBlitTypeDst = 0;

	// IN RELEASE MODE, GIVE THE USER A CHANCE:
#ifndef _DEBUG

	//if (gsScreenLocked || gsBufferLocked) goto RECT_DONTLOCK;

#endif

	// do OS based copying!

	// IN THIS IMPLEMENTATION, we must do LOCK, BLiT, UNLOCK, so I
	// must record which UNLOCK (if any) needs to be done AFTER the BLiT
	// has completed. (Lord help me if a blit gets interrupted)

	if (pimDst->m_type == RImage::IMAGE_STUB) sBlitTypeDst = (short)((long)pimDst->m_pSpecial);

	switch (sBlitTypeDst) // 0 = normal image
		{
		case 0: // normal image, buffer in image
		break;

		case BUF_MEMORY: // draw a rectangle in the system buffer
			// need to lock / unlock this one:
/*
			if (rspLockBuffer()
				!=0)
				{
				TRACE("rspDrawRect: Unable to lock the system buffer, failed!\n");
				return -1;
				}
			// Locked the system buffer, remember to unlock it:
			sNeedToUnlock = BUF_MEMORY;
*/
		break;

		case BUF_VRAM: // draw a rectangle to the visible vram screen
			// need to lock / unlock this one:
/*
			if (rspLockScreen()
				!=0)
				{
				TRACE("rspDrawRect: Unable to lock the OnScreen buffer, failed!\n");
				return -1;
				}
			// Locked the system buffer, remember to unlock it:
			sNeedToUnlock = BUF_VRAM;	
*/
		break;

		case BUF_VRAM2: // draw a rectangle to the hidden VRAM plane
			// need to lock / unlock this one:
			if (rspLockVideoFlipPage((void**)&(pimDst->m_pData),&(pimDst->m_lPitch))
				!=0)
				{
				TRACE("rspDrawRect: Unable to lock the OffScreen buffer, failed!\n");
				return -1;
				}
			// Locked the system buffer, remember to unlock it:
			sNeedToUnlock = BUF_VRAM2;			
		break;

		default:
			TRACE("rspDrawRect: This type of copy is not yet supported.\n");
			return -1;
		}
//RECT_DONTLOCK:
		
	//********************************************************************

	// Done clipping, convert to bytes to find best alignment:
	// Currently based on source, assumes source = destination depth:
	// Multiply color word based on 8-bit concepts:
	//
	color &= 255;
	short sByteW = sW;

	switch (pimDst->m_sDepth)
		{
		case 16:
			sX <<= 1;
			sByteW <<= 1; // width for ALIGMENT!
		break;
		case 32:
			sX <<= 2;
			sByteW <<= 2;
		break;
		case 24:
			sX += (sX << 1);
			sByteW += (sByteW << 1);
		break;
		// 8-bit needs nothing...
		}

	// Calculate memory offsets using signed pitch:

	// sX and sByteW are in BYTES!
	U8* pDst = pimDst->m_pData + sX + pimDst->m_lPitch * sY;
	
	// Determine Byte Alignment:
	// sX and sByteW are in BYTES!
	short sAlign = sX | sByteW | (short)( ABS(pimDst->m_lPitch) );

	// Do the draw rect as fast as possible!
	if ( (sAlign & 3) == 0)	
		{
		// 32-bit copy
		ULONG ulColor = color;
		ulColor += (color << 8); // 16-bit
		ulColor += (ulColor << 16); // 32-bit

		_ClearRect(ulColor,(U32*)pDst,pimDst->m_lPitch,sH,sByteW); 
		}
	else if ( (sAlign & 1) == 0)
		{
		// 16-bit copy
		USHORT usColor = (USHORT)(color + (color << 8)); // 16-bit;
		_ClearRect(usColor,(U16*)pDst,pimDst->m_lPitch,sH,sByteW); 
		}
	else
		{
		// 8-bit copy
		_ClearRect((U8)color,(U8*)pDst,pimDst->m_lPitch,sH,sByteW); 
		}


	//********************
	// OS_SPECIFIC:
	//********************  UNLOCK WHATEVER YOU NEED TO

	// IN RELEASE MODE, GIVE THE USER A CHANCE:
#ifndef _DEBUG

//	if (gsScreenLocked || gsBufferLocked) goto RECT_DONTUNLOCK;

#endif

	switch (sNeedToUnlock)
		{
		case 0:
		break;

		case BUF_MEMORY:
//			rspUnlockBuffer();
		break;
		
		case BUF_VRAM:
//			rspUnlockScreen();
		break;
		
		case BUF_VRAM2:
			rspUnlockVideoFlipPage();
		break;
		
		default:
			TRACE("rspDrawRect:  Unlocking error!\n");
		}

//RECT_DONTUNLOCK:
	
	return 0;
	}

// Does a "hollow" rectangle! (grows inwards)
//
short rspRect(short sThickness,U32 color,RImage* pimDst,short sX,short sY,short sW,short sH,RRect* prClip)
	{
	// 1) preliminary parameter validation:
#ifdef _DEBUG

	if (pimDst == NULL)
		{
		TRACE("rspRect: null RImage* passed\n");
		return -1;
		}

	if ( /*(sW < 1) || (sH < 1) ||*/ (sThickness < 1))
		{
		TRACE("rspRect: zero or negative thickness passed\n");
		return -1;
		}

	if (!ImageIsUncompressed(pimDst->m_type) && (pimDst->m_type != RImage::BMP1)) 
		{
		TRACE("rspRect: You may onlt draw rects into uncompressed images.!\n");
		return -1;
		}

	if ( (sThickness > (sW >> 1) ) || (sThickness > (sH >> 1) ) )
		{
		TRACE("rspRect: Rect to thick to show!\n");
		return -1;
		}

#endif

	
	// UNMIRROR THE RECTANGLE:
	if (!sW && !sH) return 0; // nothing to draw

	if (sW < 0) { sX += (sW+1); sW = -sW; }
	if (sH < 0) { sY += (sH+1); sH = -sH; }

	// make up 4 rectangles:  (Horizontal Major)
	// order of blit is for maxim VSYNC ability
	//
	rspRect(color,pimDst,sX,sY,sW,sThickness,prClip);	// top
	rspRect(color,pimDst,sX,sY+sThickness,sThickness,sH - (sThickness<<1) ,prClip); // left
	rspRect(color,pimDst,sX + sW - sThickness,sY+sThickness,sThickness,sH - (sThickness<<1) ,prClip); // left
	rspRect(color,pimDst,sX,sY + sH - sThickness,sW,sThickness,prClip);	// bottom

	return 0;
	}

// This creates a new image whose uncompressed image is the requested selection
// of the old image.  You are responsible for "clipping" the requested region.
// Returns 0 on success.  If you supply a RImage destination pointer, it will
// create a new image for you, otherwise it will crop the current image.
// This only crops uncompressed images.
//
// Modify current:
short	rspCrop(RImage* pimSrc,short sX,short sY,short sW,short sH,
				  short sAlign) // sAlign in BYTES
	{
#ifdef _DEBUG

	if (pimSrc == NULL)
		{
		TRACE("rspCrop: NULL image passed\n");
		return -1;
		}

	if ( (sW < 0) || (sH < 0) )
		{
		TRACE("rspCrop: Bad width or height passed!\n");
		return -1;
		}

	if ( (sX < 0) || (sY < 0) || ( (sX + sW) > pimSrc->m_sWidth) ||
			 ( (sY + sH) > pimSrc->m_sHeight) )
		{
		TRACE("rspCrop: Crop rectangle is outside buffer size!\n");
		return -1;
		}

	if (!ImageIsUncompressed(pimSrc->m_type))
		{
		TRACE("rspCrop: Can only crop uncompressed buffers!\n");
		return -1;
		}

#endif

	long lNewPitch;

	// 2) Create a new buffer with 128-bit alignment and pitch:

	// Determine pixel size:
	short	sPixSize = pimSrc->m_sDepth >> 3;
	short sShift[] = {-1,0,1,-1,2};
	short sAlignMask = sAlign - 1; // sAlign is in BYTES
	short sPixShift = sShift[sPixSize];

	// Determine Optimum Pitch...
	lNewPitch = (sW + sAlignMask) & (~sAlignMask);

	// Create a new buffer and Image Stub to BLiT can use it:
	RImage imDst;
	imDst.CreateImage(sW,sH,pimSrc->m_type,
		lNewPitch,pimSrc->m_sDepth);

	// 3) Copy the new one in...
	rspBlit(pimSrc,&imDst,sX,sY,0,0,sW,sH);

	// tricky part: Swap buffers...
	UCHAR	*pSrcMem,*pSrcBuf;
	pimSrc->DetachData((void**)&pSrcMem,(void**)&pSrcBuf);
	// Move the new buffer back to the original
	imDst.DetachData((void**)&(pimSrc->m_pMem),(void**)&(pimSrc->m_pData));

	//*******  IMPORTANT! COPY ALL NEW INFO OVER!
	pimSrc->m_ulSize = imDst.m_ulSize;
	pimSrc->m_sWidth = imDst.m_sWidth;
	pimSrc->m_sHeight = imDst.m_sHeight;
	pimSrc->m_sWinWidth = imDst.m_sWidth;
	pimSrc->m_sWinHeight = imDst.m_sHeight;
	pimSrc->m_lPitch = imDst.m_lPitch;

	// 4) Destroy the old...
	RImage::DestroyDetachedData((void**)&pSrcMem);
	// On exit, imDst will auto destruct....
	
	return 0;
	}

// This creates a new image whose uncompressed image is the requested selection
// of the old image.  
// Returns 0 on success.  
// This only pads uncompressed images.
//
// Modify current:
short	rspPad(RImage* pimSrc,short sX,short sY, // where to move the old image to
					short sW,short sH, // new width and height
					short sAlign) // sAlign in BYTES
	{
#ifdef _DEBUG

	if (pimSrc == NULL)
		{
		TRACE("rspPad: NULL image passed\n");
		return -1;
		}

	if ( (sW < 0) || (sH < 0) )
		{
		TRACE("rspPad: Bad width or height passed!\n");
		return -1;
		}

	/*
	if ( ( sW < pimSrc->m_sWidth) || ( sH < pimSrc->m_sHeight) )
		{
		TRACE("rspPad: You cannot yet combine cropping and padding!\n");
		return -1;
		}

	if ( (sX < 0) || (sY < 0) || ( (sX + pimSrc->m_sWidth) > sW) ||
			 ( (sY + pimSrc->m_sHeight) > sH) )
		{
		TRACE("rspPad: Cannot move image outside of new buffer!\n");
		return -1;
		}
	*/

	if (!ImageIsUncompressed(pimSrc->m_type))
		{
		TRACE("rspPad: Can only crop uncompressed buffers!\n");
		return -1;
		}

#endif

	// Save size of old image...
	long lNewPitch;
	short sOldW = pimSrc->m_sWidth;
	short sOldH = pimSrc->m_sHeight;

	// 2) Create a new buffer with 128-bit alignment and pitch:

	// Determine pixel size:
	short	sPixSize = pimSrc->m_sDepth >> 3;
	short sShift[] = {-1,0,1,-1,2};
	short sAlignMask = sAlign - 1; // sAlign is in BYTES
	short sPixShift = sShift[sPixSize];

	// Determine Optimum Pitch...
	lNewPitch = (sW + sAlignMask) & (~sAlignMask);

	// Create a new buffer and Image Stub to BLiT can use it:
	RImage imDst;
	imDst.CreateImage(sW,sH,pimSrc->m_type,
		lNewPitch,pimSrc->m_sDepth);

	// 3) Copy the new one in...
	// First, it is necessary to source clip the image:
	RRect rCopy(sX,sY,sOldW,sOldH); // source
	RRect rDst(0,0,sW,sH);
	if (rCopy.ClipTo(&rDst))
		{
		TRACE("rspPad: Warning, image clipped out.\n");
		}
	else
		{
		// Express in terms of an offset:
		rCopy.sX -= sX;
		rCopy.sY -= sY;
	
		// do the blit with source clipping
		rspBlit(pimSrc,&imDst,rCopy.sX,rCopy.sY,
			sX + rCopy.sX,sY + rCopy.sY,rCopy.sW,rCopy.sH);
		}

	// tricky part: Swap buffers...
	UCHAR	*pSrcMem,*pSrcBuf;
	pimSrc->DetachData((void**)&pSrcMem,(void**)&pSrcBuf);
	// Move the new buffer back to the original
	imDst.DetachData((void**)&(pimSrc->m_pMem),(void**)&(pimSrc->m_pData));

	//*******  IMPORTANT! COPY ALL NEW INFO OVER!
	pimSrc->m_ulSize = imDst.m_ulSize;
	pimSrc->m_sWidth = imDst.m_sWidth;
	pimSrc->m_sHeight = imDst.m_sHeight;
	pimSrc->m_lPitch = imDst.m_lPitch;
	pimSrc->m_sWinX = sX;
	pimSrc->m_sWinY = sY;

	// 4) Destroy the old...
	RImage::DestroyDetachedData((void**)&pSrcMem);
	// On exit, imDst will auto destruct....
	
	return 0;
	}

// Returns -1 if clipped completely out,
// It clips your rect to the input rect
// If clipped completely, sets rect to 0
// & sets this rect to {0,0,0,0}
//
short RRect::ClipTo(RRect* prClipTo)
		{
		short sClipL,sClipT,sClipR,sClipB;
		sClipL = prClipTo->sX - sX;
		sClipT = prClipTo->sY - sY;
		sClipR = sX + sW - prClipTo->sX - prClipTo->sW;
		sClipB = sY + sH - prClipTo->sY - prClipTo->sH;

		if (sClipL > 0)
			{
			sX += sClipL;
			sW -= sClipL;
			}

		if (sClipT > 0)
			{
			sY += sClipT;
			sH -= sClipT;
			}

		if (sClipR > 0) sW -= sClipR;
		if (sClipB > 0) sH -= sClipB;

		if ( (sW < 0) || (sH < 0) )
			{
			sX = sY = sW = sH = 0;
			return -1;
			}

		return 0;
		}

