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
#ifdef PATHS_IN_INCLUDES
	#include "ORANGE/QuickMath/Fractions.h"
#else
	#include "Fractions.h" 
#endif

// Now, 1 bit scaled BLiTting into an BMP1 from an FSPR1
// NO CLIPPING, no colro info!
//
short rspBlitToMono(
				  RImage* pimSrc,
				  RImage* pimDst,
				  short sDstX,
				  short sDstY,
				  short sDstW,
				  short sDstH
				  )
	{
#ifdef _DEBUG

	if ((pimSrc == NULL) || (pimDst == NULL))
		{
		TRACE("BLiT: null CImage* passed\n");
		return -1;
		}

	if (pimSrc->m_type != RImage::FSPR1)
		{
		TRACE("BLiT: This form of BLiT is designed for FSPR1 type images!\n");
		return -1;
		}

	if (pimDst->m_type != RImage::BMP1)
		{
		TRACE("BLiT: This functions only BLiTs to BMP1 images.\n");
		return -1;
		}

#endif

	if ( (sDstW < 1) || (sDstH < 1))
		{
		TRACE("BLiT: Zero or negative area passed.\n");
		return -1;
		}

	long	lDstP = pimDst->m_lPitch;

	if ( (sDstX < 0) || (sDstY < 0) ||
		( (sDstX + sDstW) > pimDst->m_sWidth) ||
		( (sDstY + sDstH) > pimDst->m_sHeight) )
		{
		TRACE("BLiT: This BLiT does not yet clip!\n");
		return -1;
		}


	UCHAR	*pDst,*pDstLine,*pCode,ucCount;
	pDstLine = pimDst->m_pData + lDstP * sDstY + (sDstX>>3);
	RSpecialFSPR1*	pHead = (RSpecialFSPR1*)(pimSrc->m_pSpecial);
	pCode = pHead->m_pCode;
	const UCHAR FF = (UCHAR)255;

	// Let's scale it, baby! (pre-clipping)
	short sDenX = pimSrc->m_sWidth; 
	short sDenY = pimSrc->m_sHeight; 
	RFracU16 frX = {0};
	RFracU16 frInitX = {0};
	RFracU16 frOldX = {0};
	RFracU16 frOldY = {0},frY = {0};

	RFracU16 *afrSkipX=NULL,*afrSkipY=NULL;
	afrSkipX = rspfrU16Strafe256(sDstW,sDenX);
	afrSkipY = rspfrU16Strafe256(sDstH,sDenY);
	// Make magnification possible:
	short i;
	long *alDstSkip = (long*)calloc(sizeof(long),afrSkipY[1].mod + 2);
	for (i=1;i<(afrSkipY[1].mod + 2);i++) 
		alDstSkip[i] = alDstSkip[i-1] + lDstP;
	UCHAR	bits[] = {128,64,32,16,8,4,2,1};
	short sBit;
	frInitX.mod = (sDstX & 7);

	//***********************************************************
	//*****************  AT LAST!   CODE!  **********************
	//***********************************************************
	while (TRUE)
		{
		if ((*pCode) == FF) // vertical run
			{	// end of sprite?
			if ( (ucCount = *(++pCode)) == FF) break; 
			rspfrAdd(frY,afrSkipY[ucCount],sDenY);
			pDstLine += lDstP * (frY.mod - frOldY.mod);
			pCode++; // open stack
			continue; // next line
			}

		if (frOldY.mod == frY.mod) // do a quick skip of a line:
			{
			while ( (*(pCode++)) != FF) ; // skip line!
			rspfrAdd(frY,afrSkipY[1],sDenY);
			pDstLine += alDstSkip[frY.mod - frOldY.mod];
			}
		else // actually draw it!
			{
			frOldY = frY;
			pDst = pDstLine;
			frX.set = frInitX.set; // start of line!
			while ( (ucCount = *(pCode++)) != FF) // EOL
				{
				frOldX = frX;
				rspfrAdd(frX,afrSkipX[ucCount],sDenX);
				//pDst += (frX.mod - frOldX.mod); // skip
				ucCount = *(pCode++);
				frOldX = frX;
				rspfrAdd(frX,afrSkipX[ucCount],sDenX);
				ucCount = UCHAR(frX.mod - frOldX.mod);
				// Modify this to a rect for solid VMagnification.
				pDst = pDstLine + ((frOldX.mod)>>3);
				sBit = frOldX.mod & 7;

				while (ucCount--) 
					{
					(*pDst) |= bits[sBit]; // watch this!
					sBit++;
					if (sBit > 7)
						{
						sBit = 0;
						pDst++;
						}
					}
				}
			rspfrAdd(frY,afrSkipY[1],sDenY);
			pDstLine += alDstSkip[frY.mod - frOldY.mod];
			}
		}

	free(alDstSkip);
	free(afrSkipX);
	free(afrSkipY);

	//======================= for debugging only:
	/*
	CImage* pimScreen,*pimBuffer;
	rspNameBuffers(&pimBuffer,&pimScreen);

	pimDst->Convert(BMP8);
	// copy safebuf to screen:
	rspBlit(pimDst,pimScreen,0,0,0,0,(short)pimDst->lWidth,
					(short)pimDst->lHeight);
	pimDst->Convert(BMP1);
	rspWaitForClick();
	*/

	return 0;
	}

// mono rect ....
//
short rspRectToMono(ULONG ulColor,RImage* pimDst,short sX,short sY,
						short sW,short sH)
	{
#ifdef _DEBUG

	if (pimDst->m_type != RImage::BMP1)
		{
		TRACE("rspRectMono: Only BMP1 images supported.\n");
		return -1;
		}

	if ( (sW < 1) || (sH < 1) )
		{
		TRACE("rspRectMono: Zero or negative area passed.\n");
		return -1;
		}
#endif

	if ( (sX < 0) || (sY < 0) || ( (sX + sW) > pimDst->m_sWidth) ||
		( (sY + sH) > pimDst->m_sHeight) )
		{
		TRACE("rspRectMono:Clipping not yet supported.\n");
		return -1;
		}

	long lP = pimDst->m_lPitch;

	UCHAR	ucStart = 0,ucEnd = 0;
	UCHAR *pDst,*pDstLine;
	short sMidCount,sStart,sEnd,i,j;

	UCHAR ucBits[] = { 128,64,32,16,8,4,2,1 };
	UCHAR ucStartBits[] = { 255,127,63,31,15,7,3,1 };
	UCHAR ucEndBits[] = { 128,192,224,240,248,252,254,255 };

	sStart = (sX >> 3);
	sEnd = (sX + sW - 1);
	short sEndB = (sEnd >> 3);
	sMidCount = sEndB - sStart - 1;
	if (sMidCount < 1) sMidCount = 0;

	if (sStart == sEndB) // very thin:
		for (i= (sX&7);i<=(sEnd&7);i++) ucStart += ucBits[i];
	else // more normal run:
		{
		ucStart = ucStartBits[sX&7];
		ucEnd = ucEndBits[ sEnd & 7 ];
		}
	
	pDstLine = pimDst->m_pData + lP * sY + sStart;

	if (ulColor) // copy a rect of 1 bits:
		{
		for (j=sH;j!=0;j--)
			{
			pDst = pDstLine;
			(*pDst++) |= ucStart;
			for (i=sMidCount;i!=0;i--) *(pDst++) = 255;
			(*pDst++) |= ucEnd;
			pDstLine += lP;
			}
		}
	else // copy color 0 rect of bits:
		{
		ucStart = ~ucStart;
		ucEnd = ~ucEnd;
		for (j=sH;j!=0;j--)
			{
 			pDst = pDstLine;
			(*pDst++) &= ucStart;
			for (i=sMidCount;i!=0;i--) *(pDst++) = 0;
			(*pDst++) &= ucEnd;
			pDstLine += lP;
			}
		}
	return 0;
	}


