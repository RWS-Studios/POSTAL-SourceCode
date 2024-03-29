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
#include "ORANGE/color/dithermatch.h"
//==============================================
// This module is designed to take a 24-bit color
// bmp file and create an 8-bit BMP image by 
// dithering the 24-bit image to the palette
// provided.  It uses an enhanced bi-driection 
// sweeping error diffusion technique, and 
// for large pictures, allows optional progress
// bar callbacks. 
// Currently, I am clipping error at the edge!
//==============================================
// For the special case of not dithering to 
// background color, the current approach is
// a near lossless feedback loop off an edge.
//==============================================
// CURRENT STATE: one pass, bidirectional
//	diffusion, full callback/cancel, 
// no no-dither to bkd
//==============================================
/*

IMPORTANT NOTE!!!!!!
CURRENTLY, THE RETURNED BMP8's DO NOT HAVE A PALETTE!
IT IS UP TO THE USER TO CREATE A COPY OF THE MATCHED
COLOR PALETTE INTO THE BMP IF DESIRED!

*/

/* This extension was eeded for analysis:
					short	lClip = 256,  // JEFF DEBUGGING!
					short* ppsSaveErrorR = NULL,  // JEFF DEBUGGING!
					short* ppsSaveErrorG = NULL,  // JEFF DEBUGGING!
					short* ppsSaveErrorB = NULL  // JEFF DEBUGGING!
*/


//==============================================
// You supply a general palette to match to, 
// a BMP24 image as a TC source, and an 
// instantiated, but NOT allocated BMP8 as a 
// destination.
// 
// THIS WILL Dither to background!
//
// Returns 0 for SUCCESS, -1 for ERROR, 1 for user cancel
//==============================================
//	
int16_t	rspDither(	
					RImage* pimSrc,	// MUST BE 24-bit!
					RImage* pimDst,	// MUST be 8-bit
					int16_t sStartMap,	// palette index
					int16_t sNumMap,		// # of colors
					uint8_t*	pRed,		// Palette to match to
					uint8_t*	pGreen,
					uint8_t*	pBlue,
					int32_t	lInc,
					// User interaction
					PDitherCallBack func,
					int32_t  lMilli // milliseconds per callback
					)
	{
	ASSERT(pimSrc);
	ASSERT(pimDst);
	ASSERT(pRed);
	ASSERT(pGreen);
	ASSERT(pBlue);
	ASSERT(lInc);
	ASSERT(pimSrc->m_sDepth == 24);
	//ASSERT(pimDst->m_sDepth == 8);
	ASSERT(lInc);
	ASSERT(lMilli > 0);
	ASSERT(sStartMap >= 0);
	ASSERT(sNumMap > 0);
	ASSERT(pimDst->m_pData == NULL); // we create it!

#ifdef _DEBUG
#endif

	int16_t sRet = 0;
	int32_t	lPalOffset = lInc * sStartMap;
	int32_t	lLastTime = rspGetMilliseconds();

	//--------- Set up receiving vessel:
	if (pimDst->CreateImage(pimSrc->m_sWidth,pimSrc->m_sHeight,
		RImage::BMP8))
		{
		TRACE("rspDither: Unable to allocate new buffer!\n");
		return 1;
		}

	//--------- Set up callback timing:
	int16_t sDoCall = 0;
	double dH = double(pimSrc->m_sHeight);

	if (func)
		{
		sDoCall = 1;
		// do the first callback:
		(*func)(double(0.0));
		}

	//--------- Begin:
	int32_t lErrorRed = 0,lErrorGreen = 0,lErrorBlue = 0;

	int16_t i,j;
	int32_t lSrcP = pimSrc->m_lPitch;
	int32_t lDstP = pimDst->m_lPitch;
	int16_t sW = pimSrc->m_sWidth;
	int16_t sH = pimSrc->m_sHeight;

	int32_t lSrcXP = 3; // hard coded for 24-bit bmp's!!!
	
	uint8_t* pSrc = pimSrc->m_pData;
	uint8_t* pDst = pimDst->m_pData;

	// Must store two lines of diffused error:
	// Keep in same array so to maximize caching
	// I give a one element buffer on either side
	// to allow clipped errors!
	//
	int16_t *psErrorR = (int16_t*)calloc(sizeof(int16_t),2 * (sW+2));
	int16_t *psCurrentErrorR = psErrorR + 1; // allow clipping
	int16_t *psNextErrorR = psCurrentErrorR + sW + 2; // allow clipping

	int16_t *psErrorG = (int16_t*)calloc(sizeof(int16_t),2 * (sW+2));
	int16_t *psCurrentErrorG = psErrorG + 1; // allow clipping
	int16_t *psNextErrorG = psCurrentErrorG + sW + 2; // allow clipping

	int16_t *psErrorB = (int16_t*)calloc(sizeof(int16_t),2 * (sW+2));
	int16_t *psCurrentErrorB = psErrorB + 1; // allow clipping
	int16_t *psNextErrorB = psCurrentErrorB + sW + 2; // allow clipping
	int16_t	*psSwap;

	// Used with the look ahead pixel!
	int16_t sNextErrorR = 0,sNextErrorG = 0,sNextErrorB = 0;
	int16_t sDirection = 1; // Bidirectional sweeping!
	int16_t sErrPos = 0;
	const int16_t csClipValue = 256; // optimal results

	for (j=sH;j; j--)
		{	
		for (i=0; i < sW; i++,pSrc += lSrcXP,pDst += sDirection,sErrPos += sDirection)
			{
			int16_t sCurErrorR,sCurErrorG,sCurErrorB;
			int16_t sTotErrorR,sTotErrorG,sTotErrorB;

			// This is the desired target color
			int32_t	lRed = int32_t(pSrc[2] + sNextErrorR + psCurrentErrorR[sErrPos]);
			int32_t lGreen = int32_t(pSrc[1] + sNextErrorG + psCurrentErrorG[sErrPos]);
			int32_t	lBlue = int32_t(pSrc[0] + sNextErrorB + psCurrentErrorB[sErrPos]);

			uint8_t ucIndex = rspMatchColorRGB(lRed,lGreen,lBlue,
				sStartMap,sNumMap,pRed,pGreen,pBlue,lInc);
			*pDst = ucIndex;

			// This is the mismatch:
			// Calculate new error for this point:
			int32_t lOffset = ucIndex * lInc;
			sCurErrorR = lRed - pRed[lOffset];
			sCurErrorG = lGreen - pGreen[lOffset];
			sCurErrorB = lBlue - pBlue[lOffset];

			// TRY CLIPPING THE ERROR TO STABLIZE:
			if (sCurErrorR < -csClipValue) sCurErrorR = -csClipValue;
			if (sCurErrorR > +csClipValue) sCurErrorR = csClipValue;

			if (sCurErrorG < -csClipValue) sCurErrorG = -csClipValue;
			if (sCurErrorG > +csClipValue) sCurErrorG = csClipValue;

			if (sCurErrorB < -csClipValue) sCurErrorB = -csClipValue;
			if (sCurErrorB > +csClipValue) sCurErrorB = csClipValue;

			// Here is an optimization to avoid multiplying:
			int16_t sAddErrR = sCurErrorR << 1;
			int16_t sAddErrG = sCurErrorG << 1;
			int16_t sAddErrB = sCurErrorB << 1;

			int16_t sRunErrR = sCurErrorR + sAddErrR;
			int16_t sRunErrG = sCurErrorG + sAddErrG;
			int16_t sRunErrB = sCurErrorB + sAddErrB;

			int16_t sReg;

			// Diffuse the remaining error into the next line:
			sReg = (sRunErrR)>>4;
			psNextErrorR[sErrPos - sDirection] += sReg;
			sTotErrorR = sReg;

			sReg = (sRunErrG)>>4;
			psNextErrorG[sErrPos - sDirection] += sReg;
			sTotErrorG = sReg;

			sReg = (sRunErrB)>>4;
			psNextErrorB[sErrPos - sDirection] += sReg;
			sTotErrorB = sReg;

			sRunErrR += sAddErrR;
			sRunErrG += sAddErrG;
			sRunErrB += sAddErrB;

			// Diffuse the remaining error into the next line:
			sReg = (sRunErrR)>>4;
			psNextErrorR[sErrPos] += sReg;
			sTotErrorR += sReg;

			sReg = (sRunErrG)>>4;
			psNextErrorG[sErrPos] += sReg;
			sTotErrorG += sReg;

			sReg = (sRunErrB)>>4;
			psNextErrorB[sErrPos] += sReg;
			sTotErrorB += sReg;

			// Diffuse the error into the surrounding error map:
			sRunErrR += sAddErrR;
			sRunErrG += sAddErrG;
			sRunErrB += sAddErrB;

			// Diffuse to the right:
			sReg = (sRunErrR)>>4;
			sNextErrorR = sReg;
			sTotErrorR += sReg;

			sReg = (sRunErrG)>>4;
			sNextErrorG = sReg;
			sTotErrorG += sReg;

			sReg = (sRunErrB)>>4;
			sNextErrorB = sReg;
			sTotErrorB += sReg;

			// THIS SHOULD BE INCLUDING ROUNDOFF ERROR!
			psNextErrorR[sErrPos + sDirection] = sCurErrorR - sTotErrorR;
			psNextErrorG[sErrPos + sDirection] = sCurErrorG - sTotErrorG;
			psNextErrorB[sErrPos + sDirection] = sCurErrorB - sTotErrorB;

			}

		// Set up the reverse sweep:
		pDst += lDstP; // Jump down onw line!
		pSrc += lSrcP; // Jump down onw line!
		sDirection = -sDirection; // reverse direction
		lSrcXP = -lSrcXP; // reverse direction
		pDst += sDirection; // Back up one pixel
		pSrc += lSrcXP;  // Back up one pixel
		sErrPos += sDirection; // Back up one pixel

		// Reverse pitches for my convenience!

		// Reset the error:
		sNextErrorR = 0,sNextErrorG = 0,sNextErrorB = 0;

		// Save the error line out to disk for debugging:
		/*
		long lOffset = sW * (sH - j);

		for(i=0;i < sW;i++)
			{
			if (ppsSaveErrorR) ppsSaveErrorR[lOffset + i] = psNextErrorR[i];
			if (ppsSaveErrorG) ppsSaveErrorG[lOffset + i] = psNextErrorG[i];
			if (ppsSaveErrorB) ppsSaveErrorB[lOffset + i] = psNextErrorB[i];
			}
		*/

		// Swap error lines:
		psSwap = psCurrentErrorR;
		psCurrentErrorR = psNextErrorR;
		psNextErrorR = psSwap;

		psSwap = psCurrentErrorG;
		psCurrentErrorG = psNextErrorG;
		psNextErrorG = psSwap;

		psSwap = psCurrentErrorB;
		psCurrentErrorB = psNextErrorB;
		psNextErrorB = psSwap;

		// Clear the upcoming error layer:
		for (i=-1;i <= sW;i++)
			{
			psNextErrorR[i] = psNextErrorG[i] = psNextErrorB[i] = int16_t(0);
			}

		// Give progress feedback
		if (sDoCall)
			{
			if ((rspGetMilliseconds() - lLastTime) > lMilli)
				{
				lLastTime = rspGetMilliseconds();
				if ((*func)(1.0 - double(j)/dH) == -1) // user abort
					{
					j = 0; // premature exit!
					sRet = 1;
					}
				}
			}
		}

	free(psErrorR);
	free(psErrorG);
	free(psErrorB);

	return sRet;
	}

//==============================================
// You supply a general palette to match to, 
// a BMP24 image as a TC source, and an 
// instantiated, but NOT allocated BMP8 as a 
// destination.
// 
// Does NOT dither at all!
//==============================================
//	
int16_t	rspSimpleMap(	
					RImage* pimSrc,	// MUST BE 24-bit!
					RImage* pimDst,	// MUST be 8-bit
					int16_t sStartMap,	// palette index
					int16_t sNumMap,		// # of colors
					uint8_t*	pRed,		// Palette to match to
					uint8_t*	pGreen,
					uint8_t*	pBlue,
					int32_t	lInc,
					// User interaction
					PDitherCallBack func,
					int32_t  lMilli // milliseconds per callback
					)
	{
	ASSERT(pimSrc);
	ASSERT(pimDst);
	ASSERT(pRed);
	ASSERT(pGreen);
	ASSERT(pBlue);
	ASSERT(lInc);
	ASSERT(pimSrc->m_sDepth == 24);
	//ASSERT(pimDst->m_sDepth == 8);
	ASSERT(lInc);
	ASSERT(lMilli > 0);
	ASSERT(sStartMap >= 0);
	ASSERT(sNumMap > 0);
	ASSERT(pimDst->m_pData == NULL); // we create it!

#ifdef _DEBUG
#endif

	int16_t sRet = 0;
	int32_t	lPalOffset = lInc * sStartMap;
	int32_t	lLastTime = rspGetMilliseconds();

	//--------- Set up receiving vessel:
	if (pimDst->CreateImage(pimSrc->m_sWidth,pimSrc->m_sHeight,
		RImage::BMP8))
		{
		TRACE("rspSimpleMap: Unable to allocate new buffer!\n");
		return 1;
		}

	//--------- Set up callback timing:
	int16_t sDoCall = 0;
	double dH = double(pimSrc->m_sHeight);

	if (func)
		{
		sDoCall = 1;
		// do the first callback:
		(*func)(double(0.0));
		}

	//--------- Begin:
	int32_t lErrorRed = 0,lErrorGreen = 0,lErrorBlue = 0;

	int16_t i,j;
	int32_t lSrcP = pimSrc->m_lPitch;
	int32_t lDstP = pimDst->m_lPitch;
	int32_t lSrcXP = 3; // hard coded for 24-bit bmp's!!!
	
	uint8_t* pSrc, *pSrcLine = pimSrc->m_pData;
	uint8_t* pDst, *pDstLine = pimDst->m_pData;

	for (j=pimSrc->m_sHeight;j; j--,pSrcLine += lSrcP,pDstLine += lDstP)
		{	
		pSrc = pSrcLine;
		pDst = pDstLine;

		for (i=0; i < pimSrc->m_sWidth; i++,pSrc += lSrcXP,pDst++)
			{
			// This is the desired target color
			int32_t	lRed = int32_t(pSrc[2]);
			int32_t lGreen = int32_t(pSrc[1]);
			int32_t	lBlue = int32_t(pSrc[0]);

			uint8_t ucIndex = rspMatchColorRGB(lRed,lGreen,lBlue,
				sStartMap,sNumMap,pRed,pGreen,pBlue,lInc);
			*pDst = ucIndex;

			}
		// Give progress feedback
		if (sDoCall)
			{
			if ((rspGetMilliseconds() - lLastTime) > lMilli)
				{
				lLastTime = rspGetMilliseconds();
				if ((*func)(1.0 - double(j)/dH) == -1) // user abort
					{
					j = 0; // premature exit!
					sRet = 1;
					}
				}
			}
		}

	return 0;
	}

					
//==============================================
// You supply a general palette to match to, 
// a BMP24 image as a TC source, and an 
// instantiated, but NOT allocated BMP8 as a 
// destination.
// 
// THIS WILL NOT Dither to the specified background color!
//
// Returns 0 for SUCCESS, -1 for ERROR, 1 for user cancel
//==============================================
//	
int16_t	rspDither(	
					int32_t lBackR,		// Don't dither to this color!
					int32_t lBackG,
					int32_t lBackB,
					uint8_t ucBack,		// index to make BKGD
					RImage* pimSrc,	// MUST BE 24-bit!
					RImage* pimDst,	// MUST be 8-bit
					int16_t sStartMap,	// palette index
					int16_t sNumMap,		// # of colors
					uint8_t*	pRed,		// Palette to match to
					uint8_t*	pGreen,
					uint8_t*	pBlue,
					int32_t	lInc,
					// User interaction
					PDitherCallBack func,
					int32_t  lMilli // milliseconds per callback
					)
	{
	ASSERT(pimSrc);
	ASSERT(pimDst);
	ASSERT(pRed);
	ASSERT(pGreen);
	ASSERT(pBlue);
	ASSERT(lInc);
	ASSERT(pimSrc->m_sDepth == 24);
	//ASSERT(pimDst->m_sDepth == 8);
	ASSERT(lInc);
	ASSERT(lMilli > 0);
	ASSERT(sStartMap >= 0);
	ASSERT(sNumMap > 0);
	ASSERT(pimDst->m_pData == NULL); // we create it!

#ifdef _DEBUG
#endif

	int16_t sRet = 0;
	int32_t	lPalOffset = lInc * sStartMap;
	int32_t	lLastTime = rspGetMilliseconds();

	//--------- Set up receiving vessel:
	if (pimDst->CreateImage(pimSrc->m_sWidth,pimSrc->m_sHeight,
		RImage::BMP8))
		{
		TRACE("rspDither: Unable to allocate new buffer!\n");
		return 1;
		}

	//--------- Set up callback timing:
	int16_t sDoCall = 0;
	double dH = double(pimSrc->m_sHeight);

	if (func)
		{
		sDoCall = 1;
		// do the first callback:
		(*func)(double(0.0));
		}

	int32_t lErrorRed = 0,lErrorGreen = 0,lErrorBlue = 0;

	int16_t i,j;
	int32_t lSrcP = pimSrc->m_lPitch;
	int32_t lDstP = pimDst->m_lPitch;
	int16_t sW = pimSrc->m_sWidth;
	int16_t sH = pimSrc->m_sHeight;

	int32_t lSrcXP = 3; // hard coded for 24-bit bmp's!!!
	
	uint8_t* pSrc = pimSrc->m_pData;
	uint8_t* pDst = pimDst->m_pData;

	// Must store two lines of diffused error:
	// Keep in same array so to maximize caching
	// I give a one element buffer on either side
	// to allow clipped errors!
	//
	int16_t *psErrorR = (int16_t*)calloc(sizeof(int16_t),2 * (sW+2));
	int16_t *psCurrentErrorR = psErrorR + 1; // allow clipping
	int16_t *psNextErrorR = psCurrentErrorR + sW + 2; // allow clipping

	int16_t *psErrorG = (int16_t*)calloc(sizeof(int16_t),2 * (sW+2));
	int16_t *psCurrentErrorG = psErrorG + 1; // allow clipping
	int16_t *psNextErrorG = psCurrentErrorG + sW + 2; // allow clipping

	int16_t *psErrorB = (int16_t*)calloc(sizeof(int16_t),2 * (sW+2));
	int16_t *psCurrentErrorB = psErrorB + 1; // allow clipping
	int16_t *psNextErrorB = psCurrentErrorB + sW + 2; // allow clipping
	int16_t	*psSwap;

	// Used with the look ahead pixel!
	int16_t sNextErrorR = 0,sNextErrorG = 0,sNextErrorB = 0;
	int16_t sDirection = 1; // Bidirectional sweeping!
	int16_t sErrPos = 0;
	const int16_t csClipValue = 256; // optimal results

	for (j=sH;j; j--)
		{	
		for (i=0; i < sW; i++,pSrc += lSrcXP,pDst += sDirection,sErrPos += sDirection)
			{
			int16_t sBackground = 0;
			int16_t sCurErrorR,sCurErrorG,sCurErrorB;
			int16_t sTotErrorR,sTotErrorG,sTotErrorB;

			// This is the desired target color
			int32_t	lRed = int32_t(pSrc[2]);
			int32_t lGreen = int32_t(pSrc[1]);
			int32_t	lBlue = int32_t(pSrc[0]); 

			if ( (lRed == lBackR) && (lGreen == lBackG) && (lBlue == lBackB) )
				sBackground = 1;

			lRed += int32_t(sNextErrorR + psCurrentErrorR[sErrPos]);
			lGreen += int32_t(sNextErrorG + psCurrentErrorG[sErrPos]);
			lBlue += int32_t(sNextErrorB + psCurrentErrorB[sErrPos]);

			uint8_t ucIndex = rspMatchColorRGB(lRed,lGreen,lBlue,
				sStartMap,sNumMap,pRed,pGreen,pBlue,lInc);

			if (sBackground) *pDst = ucBack;
			else	*pDst = ucIndex;

			// This is the mismatch:
			// Calculate new error for this point:
			int32_t lOffset = ucIndex * lInc;
			sCurErrorR = lRed - pRed[lOffset];
			sCurErrorG = lGreen - pGreen[lOffset];
			sCurErrorB = lBlue - pBlue[lOffset];

			// TRY CLIPPING THE ERROR TO STABLIZE:
			if (sCurErrorR < -csClipValue) sCurErrorR = -csClipValue;
			if (sCurErrorR > +csClipValue) sCurErrorR = csClipValue;

			if (sCurErrorG < -csClipValue) sCurErrorG = -csClipValue;
			if (sCurErrorG > +csClipValue) sCurErrorG = csClipValue;

			if (sCurErrorB < -csClipValue) sCurErrorB = -csClipValue;
			if (sCurErrorB > +csClipValue) sCurErrorB = csClipValue;

			// Here is an optimization to avoid multiplying:
			int16_t sAddErrR = sCurErrorR << 1;
			int16_t sAddErrG = sCurErrorG << 1;
			int16_t sAddErrB = sCurErrorB << 1;

			int16_t sRunErrR = sCurErrorR + sAddErrR;
			int16_t sRunErrG = sCurErrorG + sAddErrG;
			int16_t sRunErrB = sCurErrorB + sAddErrB;

			int16_t sReg;

			// Diffuse the remaining error into the next line:
			sReg = (sRunErrR)>>4;
			psNextErrorR[sErrPos - sDirection] += sReg;
			sTotErrorR = sReg;

			sReg = (sRunErrG)>>4;
			psNextErrorG[sErrPos - sDirection] += sReg;
			sTotErrorG = sReg;

			sReg = (sRunErrB)>>4;
			psNextErrorB[sErrPos - sDirection] += sReg;
			sTotErrorB = sReg;

			sRunErrR += sAddErrR;
			sRunErrG += sAddErrG;
			sRunErrB += sAddErrB;

			// Diffuse the remaining error into the next line:
			sReg = (sRunErrR)>>4;
			psNextErrorR[sErrPos] += sReg;
			sTotErrorR += sReg;

			sReg = (sRunErrG)>>4;
			psNextErrorG[sErrPos] += sReg;
			sTotErrorG += sReg;

			sReg = (sRunErrB)>>4;
			psNextErrorB[sErrPos] += sReg;
			sTotErrorB += sReg;

			// Diffuse the error into the surrounding error map:
			sRunErrR += sAddErrR;
			sRunErrG += sAddErrG;
			sRunErrB += sAddErrB;

			// Diffuse to the right:
			sReg = (sRunErrR)>>4;
			sNextErrorR = sReg;
			sTotErrorR += sReg;

			sReg = (sRunErrG)>>4;
			sNextErrorG = sReg;
			sTotErrorG += sReg;

			sReg = (sRunErrB)>>4;
			sNextErrorB = sReg;
			sTotErrorB += sReg;

			// THIS SHOULD BE INCLUDING ROUNDOFF ERROR!
			psNextErrorR[sErrPos + sDirection] = sCurErrorR - sTotErrorR;
			psNextErrorG[sErrPos + sDirection] = sCurErrorG - sTotErrorG;
			psNextErrorB[sErrPos + sDirection] = sCurErrorB - sTotErrorB;

			}

		// Set up the reverse sweep:
		pDst += lDstP; // Jump down onw line!
		pSrc += lSrcP; // Jump down onw line!
		sDirection = -sDirection; // reverse direction
		lSrcXP = -lSrcXP; // reverse direction
		pDst += sDirection; // Back up one pixel
		pSrc += lSrcXP;  // Back up one pixel
		sErrPos += sDirection; // Back up one pixel

		// Reverse pitches for my convenience!

		// Reset the error:
		sNextErrorR = 0,sNextErrorG = 0,sNextErrorB = 0;

		// Save the error line out to disk for debugging:
		/*
		long lOffset = sW * (sH - j);

		for(i=0;i < sW;i++)
			{
			if (ppsSaveErrorR) ppsSaveErrorR[lOffset + i] = psNextErrorR[i];
			if (ppsSaveErrorG) ppsSaveErrorG[lOffset + i] = psNextErrorG[i];
			if (ppsSaveErrorB) ppsSaveErrorB[lOffset + i] = psNextErrorB[i];
			}
			*/

		// Swap error lines:
		psSwap = psCurrentErrorR;
		psCurrentErrorR = psNextErrorR;
		psNextErrorR = psSwap;

		psSwap = psCurrentErrorG;
		psCurrentErrorG = psNextErrorG;
		psNextErrorG = psSwap;

		psSwap = psCurrentErrorB;
		psCurrentErrorB = psNextErrorB;
		psNextErrorB = psSwap;

		// Clear the upcoming error layer:
		for (i=-1;i <= sW;i++)
			{
			psNextErrorR[i] = psNextErrorG[i] = psNextErrorB[i] = int16_t(0);
			}

		// Give progress feedback
		if (sDoCall)
			{
			if ((rspGetMilliseconds() - lLastTime) > lMilli)
				{
				lLastTime = rspGetMilliseconds();
				if ((*func)(1.0 - double(j)/dH) == -1) // user abort
					{
					j = 0; // premature exit!
					sRet = 1;
					}
				}
			}
		
		}

	free(psErrorR);
	free(psErrorG);
	free(psErrorB);

	return sRet;
	}

//*************************************************************************
//*  In 32-bit sourced versions of rspDither and rspDitherMatch,
//*  this utilizes the concept of the alpha channel as representing 
//*  transparency.  The user gives a value from 0-255 representing the 
//*  most transparent pixel to dither.  Alpha values less than this
//*  are considered the background, ,dithering will not occur, and the 
//*  pixel will be mapped to the specified background index color.
//*************************************************************************
//*  Note that if dithering to background is DESIRED, setting the alpha
//*  limit to zero prevents any of the image from being considered a 
//*  background.
//*************************************************************************

//*************************************************************************
//==============================================
// You supply a general palette to match to, 
// a SCREEN32_ARGB image as a TC source, and an 
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
int16_t	rspDither(	
					uint8_t ucForeAlpha,		// lower limit for foreground
					uint8_t ucBack,		// index to make BKGD
					RImage* pimSrc,	// MUST BE 32-bit!
					RImage* pimDst,	// MUST be 8-bit
					int16_t sStartMap,	// palette index
					int16_t sNumMap,		// # of colors
					uint8_t*	pRed,		// Palette to match to
					uint8_t*	pGreen,
					uint8_t*	pBlue,
					int32_t	lInc,
					// User interaction
					PDitherCallBack func,
					int32_t  lMilli // milliseconds per callback
					)
	{
	ASSERT(pimSrc);
	ASSERT(pimDst);
	ASSERT(pRed);
	ASSERT(pGreen);
	ASSERT(pBlue);
	ASSERT(lInc);
	ASSERT(pimSrc->m_sDepth == 32);
	//ASSERT(pimDst->m_sDepth == 8);
	ASSERT(lInc);
	ASSERT(lMilli > 0);
	ASSERT(sStartMap >= 0);
	ASSERT(sNumMap > 0);
	ASSERT(pimDst->m_pData == NULL); // we create it!

#ifdef _DEBUG
#endif

	int16_t sRet = 0;
	int32_t	lPalOffset = lInc * sStartMap;
	int32_t	lLastTime = rspGetMilliseconds();

	//--------- Set up receiving vessel:
	if (pimDst->CreateImage(pimSrc->m_sWidth,pimSrc->m_sHeight,
		RImage::BMP8))
		{
		TRACE("rspDither: Unable to allocate new buffer!\n");
		return 1;
		}

	//--------- Set up callback timing:
	int16_t sDoCall = 0;
	double dH = double(pimSrc->m_sHeight);

	if (func)
		{
		sDoCall = 1;
		// do the first callback:
		(*func)(double(0.0));
		}

	//--------- Begin:
	int32_t lErrorRed = 0,lErrorGreen = 0,lErrorBlue = 0;

	int16_t i,j;
	int32_t lSrcP = pimSrc->m_lPitch;
	int32_t lDstP = pimDst->m_lPitch;
	int16_t sW = pimSrc->m_sWidth;
	int16_t sH = pimSrc->m_sHeight;

	int32_t lSrcXP = 4; // hard coded for 32-bit bmp's!!!
	
	uint8_t* pSrc = pimSrc->m_pData;
	uint8_t* pDst = pimDst->m_pData;

	// Must store two lines of diffused error:
	// Keep in same array so to maximize caching
	// I give a one element buffer on either side
	// to allow clipped errors!
	//
	int16_t *psErrorR = (int16_t*)calloc(sizeof(int16_t),2 * (sW+2));
	int16_t *psCurrentErrorR = psErrorR + 1; // allow clipping
	int16_t *psNextErrorR = psCurrentErrorR + sW + 2; // allow clipping

	int16_t *psErrorG = (int16_t*)calloc(sizeof(int16_t),2 * (sW+2));
	int16_t *psCurrentErrorG = psErrorG + 1; // allow clipping
	int16_t *psNextErrorG = psCurrentErrorG + sW + 2; // allow clipping

	int16_t *psErrorB = (int16_t*)calloc(sizeof(int16_t),2 * (sW+2));
	int16_t *psCurrentErrorB = psErrorB + 1; // allow clipping
	int16_t *psNextErrorB = psCurrentErrorB + sW + 2; // allow clipping
	int16_t	*psSwap;

	// Used with the look ahead pixel!
	int16_t sNextErrorR = 0,sNextErrorG = 0,sNextErrorB = 0;
	int16_t sDirection = 1; // Bidirectional sweeping!
	int16_t sErrPos = 0;
	const int16_t csClipValue = 256; // optimal results

	for (j=sH;j; j--)
		{	
		for (i=0; i < sW; i++,pSrc += lSrcXP,pDst += sDirection,sErrPos += sDirection)
			{
			int16_t sCurErrorR,sCurErrorG,sCurErrorB;
			int16_t sTotErrorR,sTotErrorG,sTotErrorB;

			// This is the desired target color
			uint8_t	ucAlpha = int32_t(pSrc[3]); 
			int32_t	lRed = int32_t(pSrc[2] + sNextErrorR + psCurrentErrorR[sErrPos]);
			int32_t lGreen = int32_t(pSrc[1] + sNextErrorG + psCurrentErrorG[sErrPos]);
			int32_t	lBlue = int32_t(pSrc[0] + sNextErrorB + psCurrentErrorB[sErrPos]);

			uint8_t ucIndex = rspMatchColorRGB(lRed,lGreen,lBlue,
				sStartMap,sNumMap,pRed,pGreen,pBlue,lInc);

			if (ucAlpha < ucForeAlpha) *pDst = ucBack;
			else	*pDst = ucIndex;

			// This is the mismatch:
			// Calculate new error for this point:
			int32_t lOffset = ucIndex * lInc;
			sCurErrorR = lRed - pRed[lOffset];
			sCurErrorG = lGreen - pGreen[lOffset];
			sCurErrorB = lBlue - pBlue[lOffset];

			// TRY CLIPPING THE ERROR TO STABLIZE:
			if (sCurErrorR < -csClipValue) sCurErrorR = -csClipValue;
			if (sCurErrorR > +csClipValue) sCurErrorR = csClipValue;

			if (sCurErrorG < -csClipValue) sCurErrorG = -csClipValue;
			if (sCurErrorG > +csClipValue) sCurErrorG = csClipValue;

			if (sCurErrorB < -csClipValue) sCurErrorB = -csClipValue;
			if (sCurErrorB > +csClipValue) sCurErrorB = csClipValue;

			// Here is an optimization to avoid multiplying:
			int16_t sAddErrR = sCurErrorR << 1;
			int16_t sAddErrG = sCurErrorG << 1;
			int16_t sAddErrB = sCurErrorB << 1;

			int16_t sRunErrR = sCurErrorR + sAddErrR;
			int16_t sRunErrG = sCurErrorG + sAddErrG;
			int16_t sRunErrB = sCurErrorB + sAddErrB;

			int16_t sReg;

			// Diffuse the remaining error into the next line:
			sReg = (sRunErrR)>>4;
			psNextErrorR[sErrPos - sDirection] += sReg;
			sTotErrorR = sReg;

			sReg = (sRunErrG)>>4;
			psNextErrorG[sErrPos - sDirection] += sReg;
			sTotErrorG = sReg;

			sReg = (sRunErrB)>>4;
			psNextErrorB[sErrPos - sDirection] += sReg;
			sTotErrorB = sReg;

			sRunErrR += sAddErrR;
			sRunErrG += sAddErrG;
			sRunErrB += sAddErrB;

			// Diffuse the remaining error into the next line:
			sReg = (sRunErrR)>>4;
			psNextErrorR[sErrPos] += sReg;
			sTotErrorR += sReg;

			sReg = (sRunErrG)>>4;
			psNextErrorG[sErrPos] += sReg;
			sTotErrorG += sReg;

			sReg = (sRunErrB)>>4;
			psNextErrorB[sErrPos] += sReg;
			sTotErrorB += sReg;

			// Diffuse the error into the surrounding error map:
			sRunErrR += sAddErrR;
			sRunErrG += sAddErrG;
			sRunErrB += sAddErrB;

			// Diffuse to the right:
			sReg = (sRunErrR)>>4;
			sNextErrorR = sReg;
			sTotErrorR += sReg;

			sReg = (sRunErrG)>>4;
			sNextErrorG = sReg;
			sTotErrorG += sReg;

			sReg = (sRunErrB)>>4;
			sNextErrorB = sReg;
			sTotErrorB += sReg;

			// THIS SHOULD BE INCLUDING ROUNDOFF ERROR!
			psNextErrorR[sErrPos + sDirection] = sCurErrorR - sTotErrorR;
			psNextErrorG[sErrPos + sDirection] = sCurErrorG - sTotErrorG;
			psNextErrorB[sErrPos + sDirection] = sCurErrorB - sTotErrorB;

			}

		// Set up the reverse sweep:
		pDst += lDstP; // Jump down onw line!
		pSrc += lSrcP; // Jump down onw line!
		sDirection = -sDirection; // reverse direction
		lSrcXP = -lSrcXP; // reverse direction
		pDst += sDirection; // Back up one pixel
		pSrc += lSrcXP;  // Back up one pixel
		sErrPos += sDirection; // Back up one pixel

		// Reverse pitches for my convenience!

		// Reset the error:
		sNextErrorR = 0,sNextErrorG = 0,sNextErrorB = 0;

		// Save the error line out to disk for debugging:
		/*
		long lOffset = sW * (sH - j);

		for(i=0;i < sW;i++)
			{
			if (ppsSaveErrorR) ppsSaveErrorR[lOffset + i] = psNextErrorR[i];
			if (ppsSaveErrorG) ppsSaveErrorG[lOffset + i] = psNextErrorG[i];
			if (ppsSaveErrorB) ppsSaveErrorB[lOffset + i] = psNextErrorB[i];
			}
			*/

		// Swap error lines:
		psSwap = psCurrentErrorR;
		psCurrentErrorR = psNextErrorR;
		psNextErrorR = psSwap;

		psSwap = psCurrentErrorG;
		psCurrentErrorG = psNextErrorG;
		psNextErrorG = psSwap;

		psSwap = psCurrentErrorB;
		psCurrentErrorB = psNextErrorB;
		psNextErrorB = psSwap;

		// Clear the upcoming error layer:
		for (i=-1;i <= sW;i++)
			{
			psNextErrorR[i] = psNextErrorG[i] = psNextErrorB[i] = int16_t(0);
			}

		// Give progress feedback
		if (sDoCall)
			{
			if ((rspGetMilliseconds() - lLastTime) > lMilli)
				{
				lLastTime = rspGetMilliseconds();
				if ((*func)(1.0 - double(j)/dH) == -1) // user abort
					{
					j = 0; // premature exit!
					sRet = 1;
					}
				}
			}
		}

	free(psErrorR);
	free(psErrorG);
	free(psErrorB);

	return sRet;
	}


//*************************************************************************
//==============================================
// You supply a general palette to match to, 
// a SCREEN32_ARGB image as a TC source, and an 
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
int16_t	rspSimpleMap(	
					uint8_t	ucForeAlpha,	// alpha threshhold
					uint8_t ucBack,			// map background to this index
					RImage* pimSrc,	// MUST BE 32-bit!
					RImage* pimDst,	// MUST be 8-bit
					int16_t sStartMap,	// palette index
					int16_t sNumMap,		// # of colors
					uint8_t*	pRed,		// Palette to match to
					uint8_t*	pGreen,
					uint8_t*	pBlue,
					int32_t	lInc,
					// User interaction
					PDitherCallBack func,
					int32_t  lMilli // milliseconds per callback
					)
	{
	ASSERT(pimSrc);
	ASSERT(pimDst);
	ASSERT(pRed);
	ASSERT(pGreen);
	ASSERT(pBlue);
	ASSERT(lInc);
	ASSERT(pimSrc->m_sDepth == 32);
	//ASSERT(pimDst->m_sDepth == 8);
	ASSERT(lInc);
	ASSERT(lMilli > 0);
	ASSERT(sStartMap >= 0);
	ASSERT(sNumMap > 0);
	ASSERT(pimDst->m_pData == NULL); // we create it!

#ifdef _DEBUG
#endif

	int16_t sRet = 0;
	int32_t	lPalOffset = lInc * sStartMap;
	int32_t	lLastTime = rspGetMilliseconds();

	//--------- Set up receiving vessel:
	if (pimDst->CreateImage(pimSrc->m_sWidth,pimSrc->m_sHeight,
		RImage::BMP8))
		{
		TRACE("rspSimpleMap: Unable to allocate new buffer!\n");
		return 1;
		}

	//--------- Set up callback timing:
	int16_t sDoCall = 0;
	double dH = double(pimSrc->m_sHeight);

	if (func)
		{
		sDoCall = 1;
		// do the first callback:
		(*func)(double(0.0));
		}

	//--------- Begin:
	int32_t lErrorRed = 0,lErrorGreen = 0,lErrorBlue = 0;

	int16_t i,j;
	int32_t lSrcP = pimSrc->m_lPitch;
	int32_t lDstP = pimDst->m_lPitch;
	int32_t lSrcXP = 4; // hard coded for 32-bit bmp's!!!
	
	uint8_t* pSrc, *pSrcLine = pimSrc->m_pData;
	uint8_t* pDst, *pDstLine = pimDst->m_pData;

	for (j=pimSrc->m_sHeight;j; j--,pSrcLine += lSrcP,pDstLine += lDstP)
		{	
		pSrc = pSrcLine;
		pDst = pDstLine;

		for (i=0; i < pimSrc->m_sWidth; i++,pSrc += lSrcXP,pDst++)
			{
			// This is the desired target color
			uint8_t	ucAlpha = pSrc[3];
			int32_t	lRed = int32_t(pSrc[2]);
			int32_t lGreen = int32_t(pSrc[1]);
			int32_t	lBlue = int32_t(pSrc[0]);

			uint8_t ucIndex;

			if (ucAlpha < ucForeAlpha) *pDst = ucBack;
			else 
				{
				ucIndex = rspMatchColorRGB(lRed,lGreen,lBlue,
				sStartMap,sNumMap,pRed,pGreen,pBlue,lInc);
				*pDst = ucIndex;
				}

			}
		// Give progress feedback
		if (sDoCall)
			{
			if ((rspGetMilliseconds() - lLastTime) > lMilli)
				{
				lLastTime = rspGetMilliseconds();
				if ((*func)(1.0 - double(j)/dH) == -1) // user abort
					{
					j = 0; // premature exit!
					sRet = 1;
					}
				}
			}
		}

	return 0;
	}

					

//*************************************************************************

	

	
