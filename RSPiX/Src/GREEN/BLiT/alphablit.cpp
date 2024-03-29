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
#include "GREEN/BLiT/BLIT.H"
#include "ORANGE/color/colormatch.h"
#include "GREEN/BLiT/alphablit.h" // do NOT yet put in with blit.h (until done)
//////////////////////////////////////////////////////////////////////
//
// ALPHABLIT.CPP
//
// Created in 1996 JRD
// Implemented	throughout 1996 and 1997 - JRD
//
//		07/10/97	JRD	Finally added history section.
//
//////////////////////////////////////////////////////////////////////
//***********************************************************************************
// Here are some easy utilities that are useful in dealing with alpha masks:
//***********************************************************************************

/////////////////////////////////////////////////////////////////////////////////////
//  Allows "Ad Hoc" alpha adjustment with accumulative error.
//  (CAUTION, not reversable, UNLESS a DIFFERENT destination
//  mask is used from the source.  Will saturate at 255.
/////////////////////////////////////////////////////////////////////////////////////
//  UINPUT: 1.0 = no change in alpha effect, pimDst may equal pimMask
/////////////////////////////////////////////////////////////////////////////////////
//
void rspScaleAlphaMask(RImage* pimSrc, double dScale, RImage* pimDst)
	{
	int16_t i,j;

	int32_t lSrcP = pimSrc->m_lPitch;
	int32_t lDstP = pimDst->m_lPitch;
	uint8_t* pSrc,*pSrcLine = pimSrc->m_pData;
	uint8_t* pDst,*pDstLine = pimDst->m_pData;

	for (j=0;j<pimSrc->m_sHeight;j++,pSrcLine += lSrcP,pDstLine += lDstP)
		{
		pSrc = pSrcLine;
		pDst = pDstLine;
		for (i=0;i<pimSrc->m_sWidth;i++,pSrc++,pDst++)
			{
			double dVal = double(*pSrc);
			dVal *= dScale;
			if (dVal < 256.0)
				*pDst = uint8_t(dVal);
			else
				*pDst = uint8_t(255);
			}
		}
	}


// The mask must be as big as the source
// Should work on both main types.
//
void rspGeneralAlphaBlit(RMultiAlpha* pX,RImage* pimMask,
									RImage* pimSrc,RImage* pimDst,int16_t sDstX,int16_t sDstY,
									RRect &rDstClip)
	{
	int16_t sSrcX = 0,sSrcY = 0,sDstW = pimSrc->m_sWidth,sDstH = pimSrc->m_sHeight;

	// right here adjust things if you need to clip to other thatn the full dst im
	if (rspSimpleClip(sSrcX,sSrcY,sDstX,sDstY,sDstW,sDstH,
		rDstClip.sX,rDstClip.sY,rDstClip.sW,rDstClip.sH) == -1) return ; // clipped out
	
	int16_t i,j;
	int32_t lSrcP = pimSrc->m_lPitch;
	int32_t lDstP = pimDst->m_lPitch;
	int32_t lMaskP = pimMask->m_lPitch;
	uint8_t* pSrc,*pSrcLine = pimSrc->m_pData + sSrcX + sSrcY * lSrcP;
	uint8_t* pMask,*pMaskLine = pimMask->m_pData + sSrcX + sSrcY * lSrcP;
	uint8_t* pDst,*pDstLine = pimDst->m_pData + sDstX + lDstP * sDstY;
	uint8_t*** pppucAlphaList = pX->m_pGeneralAlpha;
	uint8_t ucTransparent = *(pX->m_pLevelOpacity);
	// set clip off at half lowest level value!
	// MUST ROUND UP!
	ucTransparent = (ucTransparent+1) >> 1; 

	for (j=0;j<sDstH;j++,pSrcLine += lSrcP,pDstLine += lDstP,pMaskLine += lMaskP)
		{
		pSrc = pSrcLine;
		pDst = pDstLine;
		pMask = pMaskLine;
		uint8_t ucMask;
		uint8_t** pucTable;

		for (i=0;i<sDstW;i++,pSrc++,pDst++,pMask++)
			{
			ucMask = *pMask;
			if (ucMask >= ucTransparent)
				{
				pucTable = pppucAlphaList[ucMask];
				if (pucTable) // There is an alpha channel
					{
					*pDst = pucTable[*pSrc][*pDst];
					}
				else // it is opaque
					{
					*pDst = *pSrc;
					}
				}
			}
		}
	}

// The mask must be as big as the source
// Should work on both main types.
// This does a pre-dimming of the mask based on sLevel -> 255 = as is!
//
void rspGeneralAlphaBlit(int16_t sLevel,RMultiAlpha* pX,RImage* pimMask,
									RImage* pimSrc,RImage* pimDst,int16_t sDstX,int16_t sDstY,
									RRect &rDstClip)
	{
	ASSERT( (sLevel >= 0 ) && (sLevel < 256) );

	int16_t sSrcX = 0,sSrcY = 0,sDstW = pimSrc->m_sWidth,sDstH = pimSrc->m_sHeight;

	// right here adjust things if you need to clip to other thatn the full dst im
	if (rspSimpleClip(sSrcX,sSrcY,sDstX,sDstY,sDstW,sDstH,
		rDstClip.sX,rDstClip.sY,rDstClip.sW,rDstClip.sH) == -1) return ; // clipped out
	
	int16_t i,j;
	int32_t lSrcP = pimSrc->m_lPitch;
	int32_t lDstP = pimDst->m_lPitch;
	int32_t lMaskP = pimMask->m_lPitch;
	uint8_t* pSrc,*pSrcLine = pimSrc->m_pData + sSrcX + sSrcY * lSrcP;
	uint8_t* pMask,*pMaskLine = pimMask->m_pData + sSrcX + sSrcY * lSrcP;
	uint8_t* pDst,*pDstLine = pimDst->m_pData + sDstX + lDstP * sDstY;
	uint8_t*** pppucAlphaList = pX->m_pGeneralAlpha;
	uint8_t ucTransparent = *(pX->m_pLevelOpacity);
	// set clip off at half lowest level value!
	// MUST ROUND UP!
	ucTransparent = (ucTransparent+1) >> 1; 
	// Set up the dimming parameter
	uint8_t*	pucDim = &RMultiAlpha::ms_aucLiveDimming[sLevel*256];

	for (j=0;j<sDstH;j++,pSrcLine += lSrcP,pDstLine += lDstP,pMaskLine += lMaskP)
		{
		pSrc = pSrcLine;
		pDst = pDstLine;
		pMask = pMaskLine;
		uint8_t ucMask;
		uint8_t** pucTable;

		for (i=0;i<sDstW;i++,pSrc++,pDst++,pMask++)
			{
			ucMask = pucDim[*pMask]; // do the dynamic dimming!
			if (ucMask >= ucTransparent)
				{
				pucTable = pppucAlphaList[ucMask];
				if (pucTable) // There is an alpha channel
					{
					*pDst = pucTable[*pSrc][*pDst];
					}
				else // it is opaque
					{
					*pDst = *pSrc;
					}
				}
			}
		}
	}

// The mask must be as big as the source
// Should work on both main types.
// This does a pre-dimming of the mask based on sLevel -> 255 = as is!
//
extern void rspGeneralAlphaBlitT(int16_t sLevel,RMultiAlpha* pX,RImage* pimMask,
									RImage* pimSrc,RImage* pimDst,int16_t sDstX,int16_t sDstY,
									RRect &rDstClip)
	{
	ASSERT( (sLevel >= 0 ) && (sLevel < 256) );

	int16_t sSrcX = 0,sSrcY = 0,sDstW = pimSrc->m_sWidth,sDstH = pimSrc->m_sHeight;

	// right here adjust things if you need to clip to other thatn the full dst im
	if (rspSimpleClip(sSrcX,sSrcY,sDstX,sDstY,sDstW,sDstH,
		rDstClip.sX,rDstClip.sY,rDstClip.sW,rDstClip.sH) == -1) return ; // clipped out
	
	int16_t i,j;
	int32_t lSrcP = pimSrc->m_lPitch;
	int32_t lDstP = pimDst->m_lPitch;
	int32_t lMaskP = pimMask->m_lPitch;
	uint8_t* pSrc,*pSrcLine = pimSrc->m_pData + sSrcX + sSrcY * lSrcP;
	uint8_t* pMask,*pMaskLine = pimMask->m_pData + sSrcX + sSrcY * lSrcP;
	uint8_t* pDst,*pDstLine = pimDst->m_pData + sDstX + lDstP * sDstY;
	uint8_t*** pppucAlphaList = pX->m_pGeneralAlpha;
	uint8_t ucTransparent = *(pX->m_pLevelOpacity);
	// set clip off at half lowest level value!
	// MUST ROUND UP!
	ucTransparent = (ucTransparent+1) >> 1; 
	// Set up the dimming parameter
	uint8_t*	pucDim = &RMultiAlpha::ms_aucLiveDimming[sLevel*256];

	for (j=0;j<sDstH;j++,pSrcLine += lSrcP,pDstLine += lDstP,pMaskLine += lMaskP)
		{
		pSrc = pSrcLine;
		pDst = pDstLine;
		pMask = pMaskLine;
		uint8_t ucMask;
		uint8_t** pucTable;

		for (i=0;i<sDstW;i++,pSrc++,pDst++,pMask++)
			{
			uint8_t ucSrc = *pSrc;
			if (ucSrc)
				{
				ucMask = pucDim[*pMask]; // do the dynamic dimming!
				if (ucMask >= ucTransparent)
					{
					pucTable = pppucAlphaList[ucMask];
					if (pucTable) // There is an alpha channel
						{
						*pDst = pucTable[*pSrc][*pDst];
						}
					else // it is opaque
						{
						*pDst = *pSrc;
						}
					}
				}
			}
		}
	}

// The mask must be as big as the source
// This Uses a Fast Multi Alpha, which leave NO ROOM for the SLIGHTEST error!
// In release mode, this will likely crash if a blit occurs which
// leaves the range of source or destination colors.
//
void rspFastMaskAlphaBlit(uint8_t*** pfaX,RImage* pimMask,
									RImage* pimSrc,RImage* pimDst,int16_t sDstX,int16_t sDstY,
									RRect &rDstClip)
	{
	int16_t sSrcX = 0,sSrcY = 0,sDstW = pimSrc->m_sWidth,sDstH = pimSrc->m_sHeight;

	// right here adjust things if you need to clip to other thatn the full dst im
	if (rspSimpleClip(sSrcX,sSrcY,sDstX,sDstY,sDstW,sDstH,
		rDstClip.sX,rDstClip.sY,rDstClip.sW,rDstClip.sH) == -1) return ; // clipped out
	
	int16_t i,j;
	int32_t lSrcP = pimSrc->m_lPitch;
	int32_t lDstP = pimDst->m_lPitch;
	int32_t lMaskP = pimMask->m_lPitch;

	uint8_t* pSrc,*pSrcLine = pimSrc->m_pData + sSrcX + sSrcY * lSrcP;
	uint8_t* pMask,*pMaskLine = pimMask->m_pData + sSrcX + sSrcY * lSrcP;
	uint8_t* pDst,*pDstLine = pimDst->m_pData + sDstX + lDstP * sDstY;

	uint8_t ucTransparent = *( (uint8_t*)pfaX ); // secret code!

	for (j=0;j<sDstH;j++,pSrcLine += lSrcP,pDstLine += lDstP,pMaskLine += lMaskP)
		{
		pSrc = pSrcLine;
		pDst = pDstLine;
		pMask = pMaskLine;
		uint8_t ucMask;
		uint8_t** pucTable;

		for (i=0;i<sDstW;i++,pSrc++,pDst++,pMask++)
			{
			ucMask = *pMask;
			if (ucMask >= ucTransparent)
				{
				pucTable = pfaX[ucMask];
				if (pucTable) // There is an alpha channel
					{
					ASSERT(pucTable[*pSrc]); // catch source errors
					*pDst = *(pucTable[*pSrc] + *pDst);
					}
				else // it is opaque
					{
					*pDst = *pSrc;
					}
				}
			}
		}
	}

// The mask must be as big as the source
// This Uses a Fast Multi Alpha, which leave NO ROOM for the SLIGHTEST error!
// In release mode, this will likely crash if a blit occurs which
// leaves the range of source or destination colors.
//
// One exception here is that 
//
void rspFastMaskAlphaBlitT(uint8_t*** pfaX,RImage* pimMask,
									RImage* pimSrc,RImage* pimDst,int16_t sDstX,int16_t sDstY,
									RRect &rDstClip)
	{
	int16_t sSrcX = 0,sSrcY = 0,sDstW = pimSrc->m_sWidth,sDstH = pimSrc->m_sHeight;

	// right here adjust things if you need to clip to other thatn the full dst im
	if (rspSimpleClip(sSrcX,sSrcY,sDstX,sDstY,sDstW,sDstH,
		rDstClip.sX,rDstClip.sY,rDstClip.sW,rDstClip.sH) == -1) return ; // clipped out
	
	int16_t i,j;
	int32_t lSrcP = pimSrc->m_lPitch;
	int32_t lDstP = pimDst->m_lPitch;
	int32_t lMaskP = pimMask->m_lPitch;

	uint8_t* pSrc,*pSrcLine = pimSrc->m_pData + sSrcX + sSrcY * lSrcP;
	uint8_t* pMask,*pMaskLine = pimMask->m_pData + sSrcX + sSrcY * lSrcP;
	uint8_t* pDst,*pDstLine = pimDst->m_pData + sDstX + lDstP * sDstY;

	uint8_t ucTransparent = *( (uint8_t*)pfaX ); // secret code!

	for (j=0;j<sDstH;j++,pSrcLine += lSrcP,pDstLine += lDstP,pMaskLine += lMaskP)
		{
		pSrc = pSrcLine;
		pDst = pDstLine;
		pMask = pMaskLine;
		uint8_t ucMask;
		uint8_t** pucTable;

		for (i=0;i<sDstW;i++,pSrc++,pDst++,pMask++)
			{
			uint8_t ucPix = *pSrc;
			if (ucPix)
				{
				ucMask = *pMask;
				if (ucMask >= ucTransparent)
					{
					pucTable = pfaX[ucMask];
					if (pucTable) // There is an alpha channel
						{
						ASSERT(pucTable[ucPix]); // catch source errors
						*pDst = *(pucTable[ucPix] + *pDst);
						}
					else // it is opaque
						{
						*pDst = ucPix;
						}
					}
				}
			}
		}
	}


// The mask must be as big as the source
// ARCHAIC - this was back when instead of a mask specifying 0-255, it specified
// the layer number
//
/*
extern void rspAlphaMaskBlit(RMultiAlpha* pX,RImage* pimMask,
									RImage* pimSrc,RImage* pimDst,short sDstX,short sDstY,
									RRect &rDstClip)
	{
	short sSrcX = 0,sSrcY = 0,sDstW = pimSrc->m_sWidth,sDstH = pimSrc->m_sHeight;

	// right here adjust things if you need to clip to other thatn the full dst im
	if (rspSimpleClip(sSrcX,sSrcY,sDstX,sDstY,sDstW,sDstH,
		rDstClip.sX,rDstClip.sY,rDstClip.sW,rDstClip.sH) == -1) return ; // clipped out
	
	short i,j;
	long lSrcP = pimSrc->m_lPitch;
	long lDstP = pimDst->m_lPitch;
	long lMaskP = pimMask->m_lPitch;
	uint8_t* pSrc,*pSrcLine = pimSrc->m_pData + sSrcX + sSrcY * lSrcP;
	uint8_t* pMask,*pMaskLine = pimMask->m_pData + sSrcX + sSrcY * lSrcP;
	uint8_t* pDst,*pDstLine = pimDst->m_pData + sDstX + lDstP * sDstY;
	uint8_t ucOpaque = (uint8_t) pX->m_sNumLevels;

	for (j=0;j<sDstH;j++,pSrcLine += lSrcP,pDstLine += lDstP,pMaskLine += lMaskP)
		{
		pSrc = pSrcLine;
		pDst = pDstLine;
		pMask = pMaskLine;
		uint8_t ucMask;

		for (i=0;i<sDstW;i++,pSrc++,pDst++,pMask++)
			{
			ucMask = *pMask;
			if (ucMask)
				{
				if (ucMask == ucOpaque) // optimized for mostly opqaue mask:
					{
					*pDst = *pSrc;
					}
				else
					{
					*pDst = pX->m_pAlphaList[ucMask]->m_pAlphas[*pSrc][*pDst];
					}
				}
			}
		}
	}
*/

// Here is a wrapper so that the homogenous alpha blit can use a multialpha:
// sAlphaLevel goes from 0 (you see background) to 255 (you see the sprite)
// As the name implies, this does not consider source colors of index zero.
//
void rspAlphaBlitT(int16_t sAlphaLevel,RMultiAlpha* pMultiX,RImage* pimSrc,RImage* pimDst,int16_t sDstX,int16_t sDstY,
						 RRect* prDst)
	{
	int16_t sSrcX = 0,sSrcY = 0,sDstW = pimSrc->m_sWidth,sDstH = pimSrc->m_sHeight;

	// right here adjust things if you need to clip to other thatn the full dst im
	if (prDst == NULL)
		{
		if (rspSimpleClip(sSrcX,sSrcY,sDstX,sDstY,sDstW,sDstH,0,0,
			pimDst->m_sWidth,pimDst->m_sHeight) == -1) return ; // clipped out
		}
	else
		{
		if (rspSimpleClip(sSrcX,sSrcY,sDstX,sDstY,sDstW,sDstH,prDst->sX,prDst->sY,
			prDst->sW,prDst->sH) == -1) return ; // clipped out
		}
	
	int16_t i,j;
	int32_t lSrcP = pimSrc->m_lPitch;
	int32_t lDstP = pimDst->m_lPitch;
	uint8_t* pSrc,*pSrcLine = pimSrc->m_pData + sSrcX + sSrcY * lSrcP;
	uint8_t* pDst,*pDstLine = pimDst->m_pData + sDstX + lDstP * sDstY;
	uint8_t ucTransparent = *(pMultiX->m_pLevelOpacity);

	// Test out trivial conditions:
	// Check on 1/2 the lowest vales as a cross over point!
	// Must round up!
	if (sAlphaLevel <= int16_t((ucTransparent+1)>>1)) return;

	uint8_t** ppucAlpha = pMultiX->m_pGeneralAlpha[sAlphaLevel];
	if (!ppucAlpha) // it is opaque!
		{
		rspBlitT(0,pimSrc,pimDst,sSrcX,sSrcY,sDstX,sDstY,sDstW,sDstH,NULL,NULL);
		return;
		}

	for (j=0;j<sDstH;j++,pSrcLine += lSrcP,pDstLine += lDstP)
		{
		pSrc = pSrcLine;
		pDst = pDstLine;

		for (i=0;i<sDstW;i++,pSrc++,pDst++)
			{
			uint8_t ucPix = *pSrc;
			if (ucPix) *pDst = ppucAlpha[ucPix][*pDst];
			}
		}
	}


// Here is a wrapper so that the homogenous alpha blit can use a multialpha:
// sAlphaLevel goes from 0 (you see background) to 255 (you see the sprite)
// As the name implies, this does not consider source colors of index zero.
//
void rspFastAlphaBlitT(int16_t sAlphaLevel,uint8_t*** pMultiX,RImage* pimSrc,RImage* pimDst,int16_t sDstX,int16_t sDstY,
						 RRect* prDst)
	{
	int16_t sSrcX = 0,sSrcY = 0,sDstW = pimSrc->m_sWidth,sDstH = pimSrc->m_sHeight;

	// right here adjust things if you need to clip to other thatn the full dst im
	if (prDst == NULL)
		{
		if (rspSimpleClip(sSrcX,sSrcY,sDstX,sDstY,sDstW,sDstH,0,0,
			pimDst->m_sWidth,pimDst->m_sHeight) == -1) return ; // clipped out
		}
	else
		{
		if (rspSimpleClip(sSrcX,sSrcY,sDstX,sDstY,sDstW,sDstH,prDst->sX,prDst->sY,
			prDst->sW,prDst->sH) == -1) return ; // clipped out
		}
	
	int16_t i,j;
	int32_t lSrcP = pimSrc->m_lPitch;
	int32_t lDstP = pimDst->m_lPitch;
	uint8_t* pSrc,*pSrcLine = pimSrc->m_pData + sSrcX + sSrcY * lSrcP;
	uint8_t* pDst,*pDstLine = pimDst->m_pData + sDstX + lDstP * sDstY;
	uint8_t ucTransparent = *((uint8_t*)pMultiX); // Secret Code

	// Test out trivial conditions:
	if (sAlphaLevel <= int16_t(ucTransparent)) return;

	uint8_t** ppucAlpha = pMultiX[sAlphaLevel];
	if (!ppucAlpha) // it is opaque!
		{
		rspBlitT(0,pimSrc,pimDst,sSrcX,sSrcY,sDstX,sDstY,sDstW,sDstH,NULL,NULL);
		return;
		}

	for (j=0;j<sDstH;j++,pSrcLine += lSrcP,pDstLine += lDstP)
		{
		pSrc = pSrcLine;
		pDst = pDstLine;

		for (i=0;i<sDstW;i++,pSrc++,pDst++)
			{
			uint8_t ucPix = *pSrc;
			if (ucPix) *pDst = ppucAlpha[ucPix][*pDst];
			}
		}
	}


void rspAlphaBlit(RAlpha* pX,RImage* pimSrc,RImage* pimDst,int16_t sDstX,int16_t sDstY)
	{
	int16_t sSrcX = 0,sSrcY = 0,sDstW = pimSrc->m_sWidth,sDstH = pimSrc->m_sHeight;

	// right here adjust things if you need to clip to other thatn the full dst im
	if (rspSimpleClip(sSrcX,sSrcY,sDstX,sDstY,sDstW,sDstH,0,0,
		pimDst->m_sWidth,pimDst->m_sHeight) == -1) return ; // clipped out
	
	int16_t i,j;
	int32_t lSrcP = pimSrc->m_lPitch;
	int32_t lDstP = pimDst->m_lPitch;
	uint8_t* pSrc,*pSrcLine = pimSrc->m_pData + sSrcX + sSrcY * lSrcP;
	uint8_t* pDst,*pDstLine = pimDst->m_pData + sDstX + lDstP * sDstY;

	for (j=0;j<sDstH;j++,pSrcLine += lSrcP,pDstLine += lDstP)
		{
		pSrc = pSrcLine;
		pDst = pDstLine;

		for (i=0;i<sDstW;i++,pSrc++,pDst++)
			{
			*pDst = pX->m_pAlphas[*pSrc][*pDst];
			}
		}
	}

void rspAlphaBlitT(RAlpha* pX,RImage* pimSrc,RImage* pimDst,int16_t sDstX,int16_t sDstY)
	{
	int16_t sSrcX = 0,sSrcY = 0,sDstW = pimSrc->m_sWidth,sDstH = pimSrc->m_sHeight;

	// right here adjust things if you need to clip to other thatn the full dst im
	if (rspSimpleClip(sSrcX,sSrcY,sDstX,sDstY,sDstW,sDstH,0,0,
		pimDst->m_sWidth,pimDst->m_sHeight) == -1) return ; // clipped out
	
	int16_t i,j;
	int32_t lSrcP = pimSrc->m_lPitch;
	int32_t lDstP = pimDst->m_lPitch;
	uint8_t* pSrc,*pSrcLine = pimSrc->m_pData + sSrcX + sSrcY * lSrcP;
	uint8_t* pDst,*pDstLine = pimDst->m_pData + sDstX + lDstP * sDstY;

	for (j=0;j<sDstH;j++,pSrcLine += lSrcP,pDstLine += lDstP)
		{
		pSrc = pSrcLine;
		pDst = pDstLine;

		for (i=0;i<sDstW;i++,pSrc++,pDst++)
			{
			uint8_t ucSrc = *pSrc;
			if (ucSrc)
				*pDst = pX->m_pAlphas[ucSrc][*pDst];
			}
		}
	}

// This draws source to destination with clipping only if destination is not zero!
// NOTE that this technique is only valid for adding to fully opaque masks.
// For more deluxe compound masking, you need to only change if source < dest!
//
void rspMaskBlit(RImage* pimSrc,RImage* pimDst,int16_t sDstX,int16_t sDstY)
	{
	int16_t sSrcX = 0,sSrcY = 0,sDstW = pimSrc->m_sWidth,sDstH = pimSrc->m_sHeight;

	// right here adjust things if you need to clip to other thatn the full dst im
	if (rspSimpleClip(sSrcX,sSrcY,sDstX,sDstY,sDstW,sDstH,0,0,
		pimDst->m_sWidth,pimDst->m_sHeight) == -1) return ; // clipped out
	
	int16_t i,j;
	int32_t lSrcP = pimSrc->m_lPitch;
	int32_t lDstP = pimDst->m_lPitch;
	uint8_t* pSrc,*pSrcLine = pimSrc->m_pData + sSrcX + sSrcY * lSrcP;
	uint8_t* pDst,*pDstLine = pimDst->m_pData + sDstX + lDstP * sDstY;

	for (j=0;j<sDstH;j++,pSrcLine += lSrcP,pDstLine += lDstP)
		{
		pSrc = pSrcLine;
		pDst = pDstLine;

		for (i=0;i<sDstW;i++,pSrc++,pDst++)
			{
			if (*pDst) *pDst = *pSrc;
			}
		}
	}

// Takes a BMP8 and converts it to a mask of 0 and ucVal
void rspMakeMask(RImage* pimSrc,uint8_t ucVal)
	{
	int16_t i,j;
	int32_t lSrcP = pimSrc->m_lPitch;
	uint8_t* pSrc,*pSrcLine = pimSrc->m_pData;

	for (j=0;j<pimSrc->m_sHeight;j++,pSrcLine += lSrcP)
		{
		pSrc = pSrcLine;
		for (i=0;i<pimSrc->m_sWidth;i++,pSrc++)
			{
			if (*pSrc) *pSrc = ucVal; // replace all as mask
			}
		}
	}

// Takes a BMP8 and converts it to a mask of 0 and ucVal
// Currently, no clipping or positioning possible
void rspCopyAsMask(RImage* pimSrc,RImage* pimDst,uint8_t ucVal)
	{
	int16_t i,j;
	int32_t lSrcP = pimSrc->m_lPitch;
	int32_t lDstP = pimDst->m_lPitch;
	uint8_t* pSrc,*pSrcLine = pimSrc->m_pData;
	uint8_t* pDst,*pDstLine = pimDst->m_pData;

	int16_t sDstW = pimSrc->m_sWidth;
	int16_t sDstH = pimSrc->m_sHeight;

	for (j=0;j<sDstH;j++,pSrcLine += lSrcP,pDstLine += lDstP)
		{
		pSrc = pSrcLine;
		pDst = pDstLine;

		for (i=0;i<sDstW;i++,pSrc++,pDst++)
			{
			if (*pSrc) *pDst = ucVal; // replace all as mask
			else 
				*pDst = 0;
			}
		}
	}
