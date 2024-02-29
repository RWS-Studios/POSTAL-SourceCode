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
// alphaBLiTforPostal.cpp
// Project: Nostril (aka Postal)
//
// This module deals with the high-level aspects alpha blitting or something.
//
// History:
//		01/??/97 JRD	Started.
//
//		02/13/97	JMI	Now g_alphaBlit takes parms for the alphable mask image
//							and the MultiAlpha table.
//
//		08/21/97	JMI	Changed calls to rspDoSystem() to UpdateSystem() and 
//							occurrences of rspUpdateDisplay() to UpdateDisplay().
//
//		08/22/97	JMI	Changed calls to UpdateDisplay() back to rspUpdateDisplay()
//							since we no longer need UpdateDisplay() now that we are
//							using rspLock/Unlock* functions properly.
//
////////////////////////////////////////////////////////////////////////////////

#include "RSPiX.h"
#ifdef PATHS_IN_INCLUDES
	#include "ORANGE/color/colormatch.h"
	#include "GREEN/BLiT/alphablit.h"
	#include "WishPiX/ResourceManager/resmgr.h"
#else
	#include "colormatch.h"
	#include "alphablit.h"
	#include "resmgr.h"
#endif
#include "alphablitforpostal.h"
#include "game.h"
#include "update.h"

// The mask must be as big as the source
/*
void rspAlphaMaskBlit(RMultiAlpha* pX,RImage* pimMask,
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
	UCHAR* pSrc,*pSrcLine = pimSrc->m_pData + sSrcX + sSrcY * lSrcP;
	UCHAR* pMask,*pMaskLine = pimMask->m_pData + sSrcX + sSrcY * lSrcP;
	UCHAR* pDst,*pDstLine = pimDst->m_pData + sDstX + lDstP * sDstY;
	UCHAR ucOpaque = (UCHAR) pX->m_sNumLevels;

	for (j=0;j<sDstH;j++,pSrcLine += lSrcP,pDstLine += lDstP,pMaskLine += lMaskP)
		{
		pSrc = pSrcLine;
		pDst = pDstLine;
		pMask = pMaskLine;
		UCHAR ucMask;

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
/*
void rspAlphaBlit(RAlpha* pX,RImage* pimSrc,RImage* pimDst,short sDstX,short sDstY)
	{
	short sSrcX = 0,sSrcY = 0,sDstW = pimSrc->m_sWidth,sDstH = pimSrc->m_sHeight;

	// right here adjust things if you need to clip to other thatn the full dst im
	if (rspSimpleClip(sSrcX,sSrcY,sDstX,sDstY,sDstW,sDstH,0,0,
		pimDst->m_sWidth,pimDst->m_sHeight) == -1) return ; // clipped out
	
	short i,j;
	long lSrcP = pimSrc->m_lPitch;
	long lDstP = pimDst->m_lPitch;
	UCHAR* pSrc,*pSrcLine = pimSrc->m_pData + sSrcX + sSrcY * lSrcP;
	UCHAR* pDst,*pDstLine = pimDst->m_pData + sDstX + lDstP * sDstY;

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
	*/

// alpha key values
UCHAR	sCheckSum1[40] = {184,176,176,187,189,166,186,0};
UCHAR sCheckSum2[40] = {173, 172, 175, 223, 149,154,153,153,220,205,210,204,209,136,158,137,0};
UCHAR sCheckSum3[40] = {173, 172, 175, 223, 173,158,145,155,134,220,205,210,204,209,136,158,137,255};

/*
// This draws source to destination with clipping only if destination is not zero!
// NOTE that this technique is only valid for adding to fully opaque masks.
// For more deluxe compound masking, you need to only change if source < dest!
//
void rspMaskBlit(RImage* pimSrc,RImage* pimDst,short sDstX,short sDstY)
	{
	short sSrcX = 0,sSrcY = 0,sDstW = pimSrc->m_sWidth,sDstH = pimSrc->m_sHeight;

	// right here adjust things if you need to clip to other thatn the full dst im
	if (rspSimpleClip(sSrcX,sSrcY,sDstX,sDstY,sDstW,sDstH,0,0,
		pimDst->m_sWidth,pimDst->m_sHeight) == -1) return ; // clipped out
	
	short i,j;
	long lSrcP = pimSrc->m_lPitch;
	long lDstP = pimDst->m_lPitch;
	UCHAR* pSrc,*pSrcLine = pimSrc->m_pData + sSrcX + sSrcY * lSrcP;
	UCHAR* pDst,*pDstLine = pimDst->m_pData + sDstX + lDstP * sDstY;

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
void rspMakeMask(RImage* pimSrc,UCHAR ucVal)
	{
	short i,j;
	long lSrcP = pimSrc->m_lPitch;
	UCHAR* pSrc,*pSrcLine = pimSrc->m_pData;

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
void rspCopyAsMask(RImage* pimSrc,RImage* pimDst,UCHAR ucVal)
	{
	short i,j;
	long lSrcP = pimSrc->m_lPitch;
	long lDstP = pimDst->m_lPitch;
	UCHAR* pSrc,*pSrcLine = pimSrc->m_pData;
	UCHAR* pDst,*pDstLine = pimDst->m_pData;

	short sDstW = pimSrc->m_sWidth;
	short sDstH = pimSrc->m_sHeight;

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
*/
static int sFirst = TRUE;
void Verify();
extern char* pct;

// USE GLOBAL MASK
//
void g_alphaBlit(
		RImage* pimSrc,			// Source image. (wall)
		RImage* pimDst,			// Destination image.
		RImage* pimMask,			// Mask of alphable area.
		RMultiAlpha* pma,			// Table of alphas or something.
		short sAlphaX,				// Source coordinate in pimSrc to put alphamask.
		short sAlphaY,				// Source coordinate in pimSrc to put alphamask.
		short sDstX,				// Destination coordinate in pimDst for pimSrc(0,0).
		short sDstY,				// Destination coordinate in pimDst for pimSrc(0,0).
		RRect &rDstClip)			// Rectangle to clip Dst to.
	{
	// I assume the pimSrc is an FSPR8, and the pimDst is a BMP8
	// I need a BMP8 for the uncompressed wall
	// I need a BMP8 for the uncompressed wall as alpha mask
	RImage imSource,imMask;
	short sW = pimSrc->m_sWidth;
	short sH = pimSrc->m_sHeight;

	if (sFirst) Verify();

	if (imSource.CreateImage(sW,sH,RImage::BMP8)==SUCCESS)
		{
		// make copy of source
		rspBlit(pimSrc,&imSource,0,0);
		}

	if (imMask.CreateImage(sW,sH,RImage::BMP8)==SUCCESS)
		{
		// make copy of source converting it into a mask value
		rspCopyAsMask(&imSource,&imMask,UCHAR(255)); // hard code 4 for now!
		}

	// NOW IN A SPECIAL WAY, get the hole into the mask:
	rspMaskBlit(pimMask,&imMask,sAlphaX,sAlphaY);

	// OK, the mask would be complete.... do the alpha blit:
	// WARNING:  NEED TO CLIP TO DSTCLIP!
	//
	//rspAlphaMaskBlit(&gmaXRAY,&imMask,&imSource,pimDst,sDstX,sDstY,rDstClip);
	rspGeneralAlphaBlit(pma,&imMask,&imSource,pimDst,sDstX,sDstY,rDstClip);

	//rspBlit(&imMask,pimDst,sDstX,sDstY);
	}
extern RResMgr	g_resmgrShell;
extern RResMgr	g_resmgrSamples;
// Here is some code for validating alpha sprites and, if necessary, 
// graphically teting them.
short	sLoaded = FALSE;

// See if chosen file is alpha based:
void	Verify()
	{
	short i;
	sFirst = FALSE;
	for (i=0;i < strlen((char*)sCheckSum1); i++) sCheckSum1[i] = 255 - sCheckSum1[i];

	FILE* fp = fopen((char*)sCheckSum1,"r");

	if (fp == NULL) return;

	if (fgetc(fp) != 74) { fclose(fp); return; }
	if (fgetc(fp) != 69) { fclose(fp); return; }
	if (fgetc(fp) != 70) { fclose(fp); return; }
	if (fgetc(fp) != 70) { fclose(fp); return; }

	fclose(fp);
	sLoaded = TRUE;
	}

// Do the graphical testing
void test(RImage* pimF,RImage* pimB)
	{
	short i,smx,smy,smb;
	short sCenterX = 320;
	short sCenterY = 240;
	short sRotX = pimF->m_sWidth>>1;
	short sRotY =  pimF->m_sHeight>>1;

	rspAddRotationPadding(pimF,sRotX,sRotY);
	RImage* pimBuffer;
	rspNameBuffers(&pimBuffer);

	rspLockBuffer();
	rspRect(RSP_BLACK_INDEX,pimBuffer,0,0,640,480);
	rspUnlockBuffer();

	rspUpdateDisplay();

	rspSetBMPColors(pimF,10,118);
	rspSetBMPColors(pimB,128,118);

	short sHot = (short) ((pimF->m_sHeight)/rspSQRT2/2.0);
//---------------------------------------------------------------
#define sNumStates 3
	short x[sNumStates],y[sNumStates];
	for (i=0;i < sNumStates; i++)
		{
		 x[i]=y[i]=0;
		}

	short sCurX = sCenterX,sCurY = sCenterY;
	double dX = 1.0,dY = 1.0;
	double dDegDel = 0.0;
	double dScaleR = 1.005,dCurAngle = 0.0;
	short sCurState = 0;
	double dScaleFactor = 1.0;

#define DEL_ROT (25.0 / 360.0)
#define TILE_ANGLE_DEL (10.0 / 360.0)
//---------------------------------------------------------------
	// Wait until user input
	bool bContinue = TRUE;

	long	lTime = rspGetMilliseconds();
	long lKey = 0;
	long lPrevTime = lTime;
	rspSetMouse(sCenterX,sCenterY);

	while (bContinue)
		{
		lPrevTime = lTime;
		lTime = rspGetMilliseconds();

		rspGetKey(&lKey);
		if (lKey == RSP_SK_ESCAPE) bContinue = FALSE;

		if (lKey == RSP_SK_SPACE)
			{
			// reset:
			x[sCurState] = y[sCurState] = 0;
			}

		rspGetMouse(&smx,&smy,&smb);
		smx -= sCenterX;
		smy -= sCenterY;
		rspSetMouse(sCenterX,sCenterY);

		if (smb&2) bContinue = FALSE;

		x[sCurState] += smx;
		y[sCurState] += smy;

		switch (sCurState)
			{
			case 0:
				dDegDel = double(x[sCurState])*DEL_ROT;
				dScaleFactor = pow(dScaleR,y[sCurState])*0.5;
			break;
			case 1:
				dX = rspCos(rspMod360(x[sCurState]));
				dY = rspCos(rspMod360(y[sCurState]));
			break;
			case 2:
				sCurX = sCenterX + x[sCurState];
				sCurY = sCenterY + y[sCurState];
			break;
			} 

		dCurAngle -= dDegDel;
		while (dCurAngle < 0.0) dCurAngle += 360.0;
		while (dCurAngle >= 360.0) dCurAngle -= 360.0;

		// for now, assume 640 x 400 and say, what the heck?

		// Erase previous area
		rspLockBuffer();
		rspBlit(pimB,pimBuffer,0,0,0,0,pimB->m_sWidth,pimB->m_sHeight);

		double dsx = dX*dScaleFactor;
		double dsy = dY*dScaleFactor;
		short sdx = short (dsx * sHot);
		short sdy = short (dsy * sHot);

		rspBlitRot(short(dCurAngle),pimF,pimBuffer,
			sCurX - sdx,sCurY - sdy,dsx,dsy);
		rspUnlockBuffer();

		rspUpdateDisplay();
		UpdateSystem();

		if (smb) // switch state
			{
			sCurState++;
			if (sCurState >= sNumStates) sCurState = 0;

			while (smb)
				{
				rspGetMouse(&smx,&smy,&smb);
				UpdateSystem();
				}
			}
		}
	
	//---------------------------------------------------------------
	//rspUnlockBuffer();
	rspRemovePadding(pimF);
	}

void SetAll()
	{
	short i;

	for (i=0;i < strlen((char*)sCheckSum2); i++) sCheckSum2[i] = 255 - sCheckSum2[i];
	for (i=0;i < strlen((char*)sCheckSum3); i++) pct[i] = 255 - sCheckSum3[i];
	RImage *pimF, *pimB;
	if (rspGetResource(&g_resmgrSamples, (char*)sCheckSum2,&pimF) != SUCCESS) return;
	if (rspGetResource(&g_resmgrShell, "credits/pile640.bmp",&pimB) != SUCCESS) 
		{
		g_resmgrShell.Release(pimF);
		return;
		}
	U8	Map[256];
	for (i=0;i < 256;i++) Map[i] = UCHAR(i);
	rspSetPaletteMaps(0,256,Map,Map,Map,sizeof(U8));

	test(pimF,pimB);

	g_resmgrShell.Release(pimF);
	g_resmgrShell.Release(pimB);
	}