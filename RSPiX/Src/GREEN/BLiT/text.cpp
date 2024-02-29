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
REMOVE THIS FILE FROM YOUR PROJECT!  It is archaic!

// I am now attempting to remove this file from BLiT...

/*
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
	#include "GREEN/BLiT/_ic.h"
#else
	#include "_ic.h" 
#endif

//============  This deals with the BLiTting aspect of text, 
// primarily the FSPR1 format, a monochrome bitmap with
// compression rivaling postscript and a slew of special
// effects aimed at use with text...
// FSPR1 is a tow dimensional compression extension of FSPR8,
// but does NOT have random line access in order to reduce storage
//

// Do the Image stuff for FSPR1...
//

// UNFORTUNATELY, WE NEED MORE INFORMATION TO do an accurate conversion:
ULONG	gulLassoBackgroundColor = 0;	// to ignore on Lasso
ULONG	gulCompressionBackgroundColor = 0;	// During compression
ULONG	gulConvertFromColor = 255; // reverting back...
UCHAR	gucAscii = (UCHAR) 0;
short gsTrimT = 0,gsMaxH = 0; // to further crop..
ULONG gulDrawBack = 0, gulDrawFront = 1;

// Specify special conversion parameters...
//
void	rspSetConvertToFSPR1(ULONG ulLassoBackCol,ULONG ulComprBackCol,
								short sTrimT,short sMaxH,UCHAR ucAscii)
	{
	gulLassoBackgroundColor = ulLassoBackCol;
	gulCompressionBackgroundColor = ulComprBackCol;
	gsTrimT = sTrimT;
	gsMaxH = sMaxH;
	gucAscii = ucAscii;
	}

// Specify special conversion parameters...
//
void	rspSetConvertFromFSPR1(ULONG ulFrontCol,ULONG ulBackCol)
	{
	gulDrawBack = ulBackCol;
	gulDrawFront = ulFrontCol;
	}

// Will convert from ANY uncompressed format, though currently only 8-bit is supported
//
short   ConvertToFSPR1(CImage* pImage);

// Will convert back to type *8
//
short   ConvertFromFSPR1(CImage* pImage);

// Will delete pSpecial
//
short		DeleteFSPR1(CImage* pImage);


//IMAGELINKLATE(FSPR1,ConvertToFSPR1,ConvertFromFSPR1,  NULL,NULL,NULL,DeleteFSPR1);

// We will use templating to make TC a reality...
template <class PIX>
inline short	_ConvertToFSPR1(CImage* pImage,PIX choose)
	{
	// step 1, lasso the image based on background color:
	short sX = 0,sY = 0,sW = (short)pImage->lWidth,sH = (short)pImage->lHeight;
	
	if (rspLasso( (PIX)gulLassoBackgroundColor,pImage,sX,sY,sW,sH)) 
		{
		// create a legitimate compressed version of nothingness
		
		return -1; // nothing found
		}
	// post adjust:
	sY += gsTrimT;
	sH -= gsTrimT;
	if (gsMaxH) sH = MIN(sH,gsMaxH); // Fix a font to a min height...

	UCHAR*	pBuf = pImage->pData + sX + (long)sY * pImage->lPitch;

	TRACE("Kerning Box found was (%hd,%hd,%hd,%hd)\n",sX,sY,sW,sH); // for testing

	// Step II... Crate the compressed buffer...
	CCompressedMono* pHead = new CCompressedMono;
	pHead->usSourceType = (USHORT) pImage->ulType;	// store previous type...

	// Since compresion rate is unpredictable, pick a maximum likely size:
	long	lTrialSize = sH * 20; // Assum 20 bytes per line on average.
	long	lActualSize = 0;

	// Start the code...
	pHead->pCode = (UCHAR*)calloc(1,lTrialSize);

	// Set the new dimensions:
	pImage->lWidth = (long)sW;
	pImage->lHeight = (long)sH;

	// Might need to store total memory size up front.
	UCHAR*	pCode = pHead->pCode;
	UCHAR*	pBufLine = pBuf;
	short	i,j,sCount;

	UCHAR*	pOldCode = pCode;
	// if there is a null line (a 255 line),
	// the next line is the number of lines to skip (up to 254)
	short	sLineSkipCount = 0;

	for (j=0;j<sH;j++)
		{
		// convert a line:
		i=0;
		sCount = 0;

		pOldCode = pCode;

		pBuf = pBufLine;
		pBufLine += pImage->lPitch;

	NextRun:
		// Background RUN:
		sCount = 0;
		while ( (i<sW) && (*pBuf == gulCompressionBackgroundColor) ) 
			{ i++;sCount++;pBuf++; }
		
		if (i == sW) // end of line:
			{
			if (pOldCode == pCode) // THIS IS A NULL LINE!
				{
				if (sLineSkipCount == 0) // first line to skip
					{
					*(pCode++) = 255;
					*(pCode++) = 1; // skip count! (Don't overwrite!)
					sLineSkipCount = 1;
					continue;
					}
				else	// not the fist to skip:
					{
					(*(pCode-1))++; // add logic for > 254 here!
					sLineSkipCount++;

					// OVERFLOW?
					if (sLineSkipCount == 255) // overflow
						{
						*(pCode-1) = 254;
						*(pCode++) = 1;
						sLineSkipCount = 1;
						}
					continue;
					}
				}
			// End of a normal line!
			*(pCode++) = 255;
			continue; // next y-line
			}

		sLineSkipCount = 0;

		while (sCount > 254)	// multi-add:
			{
			*(pCode++) = 254;
			sCount -=254;
			}
		
		*(pCode++) = (UCHAR)sCount;
		
		// FOREGROUND RUN:
		sCount = 0;
		while ( (i<sW) && (*pBuf != gulCompressionBackgroundColor) ) { i++;sCount++;pBuf++; }
		
		while (sCount > 255)	// multi-add:
			{
			*(pCode++) = 255;
			sCount -=255;
			}
		
		*(pCode++) = (UCHAR)sCount;
		goto NextRun;

		}

	// Shrink the buffer to the TRUE size:
	// Add EOS code FFFF:
	*(pCode++) = (UCHAR)0xff;
	*(pCode++) = (UCHAR)0xff;

	pHead->ulSize = lActualSize = pCode - pHead->pCode; // if m_pCode is OPEN...
	if (pHead->ulSize == 0) return -1; // error!

	TRACE("Compressed size = %d\n",lActualSize);

	pHead->pCode = (UCHAR*) realloc(pHead->pCode,lActualSize);
	pHead->usASCII = (USHORT)gucAscii;

	// Finally, install the new buffer:
	pImage->pSpecial = pImage->pSpecialMem = (UCHAR*) pHead;
	pImage->DestroyData();
	pImage->ulSize = 0;	// BLiT needs to deal with copying, etc....
	pImage->ulType = FSPR1;

	return 0;
	}

// We will use templating to make TC a reality...
template <class PIX>
inline short _ConvertFromFSPR1(CImage* pImage,PIX choose)
	{
	// Assume it is of the correct format:
	if (pImage->ulType != FSPR1) return NOT_SUPPORTED;

	// Will try to restore the previous type of uncompressed, 8-bit buffer:

	// Generate a new buffer:
	// GuessAPitch
	pImage->lPitch = (pImage->lWidth + 15) & (~15); // 128 it!
	pImage->CreateData(pImage->lPitch * pImage->lHeight); // should be blank

		// Right now, pDst refers to the CLIPPED start of the scanline:
	union
		{
		UCHAR*	b;
		PIX* p;
		} pBuf,pLineBuf;

	CSpecialFSPR1*	pHead = (CSpecialFSPR1*)(pImage->pSpecial);
	UCHAR*	pCode = pHead->m_pCode;

	short	i,j;
	short	sCount;
	long lP = pImage->lPitch;
	short sH = (short)pImage->lHeight;
	short sW = (short)pImage->lWidth;

	ULONG ulForeColor = (PIX)gulDrawFront;

	pLineBuf.b = pBuf.b = pImage->pData;

	for (j=0;j<sH;j++)
		{
		//========================
		// check for entire lines:
		//========================
		if ( (*pCode) == 255)
			{
			sCount = *(++pCode);
			while (*pCode == 254) // overflow:
				{
				sCount += *(++pCode);
				}
			pCode++;

			// Do the block:
			pBuf.b = (pLineBuf.b += lP * sCount);

			j += sCount - 1;
			continue;	// more lines!
			}
		
	NextRun:
		//========================
		// do a background run:
		//========================
		sCount = *pCode;
		if (sCount == 254) // overload
			{
			while (*pCode == 254)
				{
				sCount += 254;
				pCode++;
				}
			sCount += *pCode;
			}
		pCode++;

		//======= do the run:
		pBuf.p += sCount;

		//========================
		// do a foreground run:
		//========================
		sCount = *pCode;
		if (sCount == 254) // overload
			{
			while (*pCode == 254)
				{
				sCount += 254;
				pCode++;
				}
			sCount += *pCode;
			}
		pCode++;

		//======= do the run:
		if (ulForeColor)
			for (i=0;i<sCount;i++)
				*(pBuf.p++) = (PIX)ulForeColor;
		else	pBuf.p += sCount;

		//======= next run:
		if (*pCode != 255) goto NextRun;

		pCode++;	// EOL;

		pLineBuf.b += lP;
		pBuf.b = pLineBuf.b;
		}

	// Reset it all
	pImage->ulType = (ULONG)pHead->m_u32OldType; // Set back the type;

	// Remove pSpecial:
	delete (CSpecialFSPR1*) pImage->pSpecial;
	pImage->pSpecial = pImage->pSpecialMem = NULL;

	return (short)pImage->ulType;
	}

	/* FOR NOW!

short	ConvertToFSPR1(CImage* pImage)
	{

#ifdef _DEBUG

	// Source must be uncompressed:
	if ( !ImageIsUncompressed(pImage->ulType))
		{
		TRACE("Convert:  Error, trying to convert a compressed Image format into FSPR1\n");
		return NOT_SUPPORTED;
		}
#endif

	switch(pImage->sDepth)
		{
		case	8:
			if (_ConvertToFSPR1(pImage,(UCHAR)0)) return NOT_SUPPORTED;
		break;

		case 16:
			//_ConvertToFSPR1(pImage,(USHORT)0);  NOT YET!
		break;

		case 32:
			//_ConvertToFSPR1(pImage,(ULONG)0); NOT YET!
		break;


		default:
			TRACE("ConvertToFSPR1:  Color depth not yet implemented.\n");
			return -1;
		}



	return FSPR1;
	}
	*/
/**************************************************************************
short ConvertFromFSPR1(CImage* pImage)
	{
#ifdef _DEBUG

	// Source must be uncompressed:
	if (pImage->ulType != FSPR1)
		{
		return NOT_SUPPORTED;
		}

#endif

	switch(pImage->sDepth)
		{
		case	8:
			if (_ConvertFromFSPR1(pImage,(UCHAR)0)) return NOT_SUPPORTED;
		break;

		case 16:
			//_ConvertToFSPR1(pImage,(USHORT)0);  NOT YET!
		break;

		case 32:
			//_ConvertToFSPR1(pImage,(ULONG)0); NOT YET!
		break;


		default:
			TRACE("ConvertFromFSPR1:  Color depth not yet implemented.\n");
			return -1;
		}

	return (short)pImage->ulType;
	}
	*/

/*
short DeleteFSPR1(CImage* pImage)
	{
	CCompressedMono* pHead = (CCompressedMono*) pImage->pSpecial;
	free(pHead->pCode);
	free(pHead);
	pImage->pSpecial = pImage->pSpecialMem = NULL;

	return 0;
	}
*/

/* archaic?
void	InstantiateBLiT();
void	InstantiateBLiT()
	{
	CImage* pim = NULL;

	rspBlit( (UCHAR)0,(UCHAR)0,pim,pim,(short)0,(short)0);
	}

//*****************************  THE FSPRITE1 BLiT  ******************************
// currently 8-bit, but soon to be full color......
// Must deal with screen locking.
//
short	rspBlit(ULONG ulForeColor,ULONG ulBackColor,CImage* pimSrc,CImage* pimDst,short sDstX,short sDstY,const Rect* prDst,
					  short sAddW)
	{
	
	//short sClip;

	// 1) preliminary parameter validation:
#ifdef _DEBUG

	if ((pimSrc == NULL) || (pimDst == NULL))
		{
		TRACE("BLiT: null CImage* passed\n");
		return -1;
		}

	if (pimSrc->ulType != FSPR1)
		{
		TRACE("BLiT: This form of BLiT is designed for FSPR1 type images!\n");
		return -1;
		}

	if (pimDst->sDepth > 8)
		{
		TRACE("BLiT: TC sprites are not YET implemented for FSPR1.\n");
		return -1;
		}

#endif

	// 2) Destination Clipping is hard here:
	short sClipL=0,sClipR=0,sClipT=0,sClipB=0;
	short sW = (short)pimSrc->lWidth; // clippng parameters...
	short sH = (short)pimSrc->lHeight; // clippng parameters...
	long	lDstP = pimDst->lPitch;

	// For clipping, adjust the destination accordingly but keep the
	// source at the origin for decompressed skipping
	//
	if (prDst)
		{
		// clip against user values
		sClipL = prDst->sX - sDstX; // positive = clipped
		if (sClipL > 0) { sW -= sClipL; sDstX = prDst->sX; }
		sClipT = prDst->sY - sDstY; // positive = clipped
		if (sClipT > 0) { sH -= sClipT; sDstY = prDst->sY; }
		// The order here still works because BOTH sDstX AND sW changed!
		sClipR = sDstX + sW - prDst->sX - prDst->sW; // positive = clipped
		if (sClipR > 0) { sW -= sClipR; }
		sClipB = sDstY + sH - prDst->sY - prDst->sH; // positive = clipped
		if (sClipB > 0) { sH -= sClipB; }

		if ( (sW <= 0) || (sH <= 0) ) return -1; // clipped out!
		}
	else	
		{
		// clip against full destination buffer
		sClipL = -sDstX;
		if (sClipL > 0) { sW -= sClipL; sDstX = 0; }
		sClipT = -sDstY;
		if (sClipR > 0) { sH -= sClipR; sDstY = 0; }
		// The order here still works because BOTH sDstX AND sW changed!
		sClipR = sDstX + sW - (short)pimDst->lWidth; // positive = clipped
		if (sClipR > 0) sW -= sClipR; // positive = clipped
		sClipB = sDstY + sH - (short)pimDst->lHeight; // positive = clipped
		if (sClipB > 0) sH -= sClipB; // positive = clipped

		if ((sW <= 0) || (sH <= 0)) return -1; // fully clipped
		}
		
	// Make positive:
	if (sClipL < 0) sClipL = 0;
	if (sClipR < 0) sClipR = 0;
	if (sClipT < 0) sClipT = 0;
	if (sClipB < 0) sClipB = 0;

	/*
	// fully clipped?
	if ( ((sClipL + sClipR) >= sW) || ((sClipT + sClipB) >= sH)) return -1;

	//**************  INSERT BUFFER HOOKS HERE!  ************************

	// do OS based copying!
	short sNeedToUnlock = 0; // will be the name of a buffer to unlock.
	short sBlitTypeDst = 0;

	// IN RELEASE MODE, GIVE THE USER A CHANCE:
#ifndef _DEBUG

	if (gsScreenLocked) goto BLIT_PRELOCKED;

#endif

	// IN THIS IMPLEMENTATION, we must do LOCK, BLiT, UNLOCK, so I
	// must record which UNLOCK (if any) needs to be done AFTER the BLiT
	// has completed. (Lord help me if a blit gets interrupted)
	if (pimDst->ulType == IMAGE_STUB) sBlitTypeDst = (short)pimDst->pSpecial;

	switch (sBlitTypeDst) // 0 = normal image
		{
		case BUF_MEMORY: // image to system buffer
			// need to lock / unlock this one:
			if (rspLockVideoBuffer((void**)&(pimDst->pData),&(pimDst->lPitch))
				!=0)
				{
				TRACE("BLiT: Unable to lock the system buffer, failed!\n");
				return -1;
				}
			// Locked the system buffer, remember to unlock it:
			sNeedToUnlock = BUF_MEMORY;			
		break;

		case BUF_VRAM: // image to front screen
			// need to lock / unlock this one:
			if (rspLockVideoPage((void**)&(pimDst->pData),&(pimDst->lPitch))
				!=0)
				{
				TRACE("BLiT: Unable to lock the OnScreen system buffer, failed!\n");
				return -1;
				}
			// Locked the front VRAM, remember to unlock it:
			sNeedToUnlock = BUF_VRAM;			
		break;

		case BUF_VRAM2: // image to back screen
			// need to lock / unlock this one:
			if (rspLockVideoFlipPage((void**)&(pimDst->pData),&(pimDst->lPitch))
				!=0)
				{
				TRACE("BLiT: Unable to lock the OffScreen system buffer, failed!\n");
				return -1;
				}
			// Locked the front VRAM, remember to unlock it:
			sNeedToUnlock = BUF_VRAM;			
		break;

		case 0: // normal image
			sNeedToUnlock = 0;
		break;

		default:
			TRACE("BLiT: This type of copy is not yet supported.\n");
			return -1;
		}
BLIT_PRELOCKED:

	//*******************************************************************
	// 8-bit biased!

	// Right now, pDst refers to the CLIPPED start of the scanline:
	union
		{
		UCHAR*	b;
		UCHAR* p; // until get a consistant calling convention for BLiT!
		} pBuf,pLineBuf;

	CSpecialFSPR1*	pHead = (CSpecialFSPR1*)(pimSrc->pSpecial);
	UCHAR*	pCode = pHead->m_pCode;

	short	i,j,k,l;
	short	sCount;
	long lP = pimDst->lPitch;

	pLineBuf.b = pBuf.b = pimDst->pData + sDstX + sDstY * pimDst->lPitch;

	// Handle HClip case separately:
	if (!sClipL && !sClipR)
		{
		// CLIP Vertically

		// Skip the top:
		if (sClipT) // clip source from top:
			for (j=sClipT;j!=0;)
				{
				//========================
				// SKIP entire lines:
				//========================
				if ( (*pCode) == 255)
					{
					j--;
					// Check for multiple skipped lines:
					sCount = *(++pCode);
					if (sCount == 254)
						{
						while (*pCode == 254) // overflow:
							{
							sCount += *(++pCode);
							}
						pCode++; // sCount = # of whole lines to skip.
						j++; // we already decremented it.
						if (sCount <= j) {j -= sCount; continue;}
						else // mixed line skip:
							{
							sCount -= j; // How many left to draw:
							j = 0; // for main loop
							goto ENTER_LINE_DRAW;
							}
						}
					else continue; // simple EOL
					}
				// ELSE
				//========================
				// skip a background run:
				//========================
				sCount = *pCode;
				if (sCount == 254) // overload
					{
					while (*pCode == 254)
						{
						sCount += 254;
						pCode++;
						}
					sCount += *pCode;
					}
				pCode++;
				// THEN
				//========================
				// skip a foreground run:
				//========================
				sCount = *pCode;
				if (sCount == 254) // overload
					{
					while (*pCode == 254)
						{
						sCount += 254;
						pCode++;
						}
					sCount += *pCode;
					}
				pCode++;
				//NEXT CLIPPED LINE
				}

		for (j=0;j<sH;j++)
			{
			//========================
			// check for entire lines:
			//========================
			if ( (*pCode) == 255)
				{
				sCount = *(++pCode);
				while (*pCode == 254) // overflow:
					{
					sCount += *(++pCode);
					}
				pCode++;

		ENTER_LINE_DRAW:
				// Do the block:
				if (ulBackColor)
					{
					// draw a block:
					for (l=0;l<sCount;l++)
						{
						pBuf.b = pLineBuf.b;
						for (k=0;k < sW + sAddW;k++)
							*(pBuf.p++) = ulBackColor;
						pBuf.b = (pLineBuf.b += lP);
						}
					}
				else
					pBuf.b = (pLineBuf.b += lP * sCount);

				j += sCount - 1;
				continue;	// more lines!
				}
			
		NextRun:
			//========================
			// do a background run:
			//========================
			sCount = *pCode;
			if (sCount == 254) // overload
				{
				while (*pCode == 254)
					{
					sCount += 254;
					pCode++;
					}
				sCount += *pCode;
				}
			pCode++;

			//======= do the run:
			if (ulBackColor)
				for (i=0;i<sCount;i++)
					*(pBuf.p++) = ulBackColor;
			else	pBuf.p += sCount;

			//========================
			// do a foreground run:
			//========================
			sCount = *pCode;
			if (sCount == 254) // overload
				{
				while (*pCode == 254)
					{
					sCount += 254;
					pCode++;
					}
				sCount += *pCode;
				}
			pCode++;

			//======= do the run:
			if (ulForeColor)
				for (i=0;i<sCount;i++)
					*(pBuf.p++) = ulForeColor;
			else	pBuf.p += sCount;

			//======= next run:
			if (*pCode != 255) goto NextRun;

			pCode++;	// EOL;

			// Fill out the rest of the line:
			if (ulBackColor)
				{
				while (pBuf.b < (pLineBuf.b + sW + sAddW))
					*(pBuf.p++) = ulBackColor;
				}

			pLineBuf.b += lP;
			pBuf.b = pLineBuf.b;
			}
		}
	else
		{
		// CLIP HORIZONTALLY and Vertically
		short sRemainingW=sW;
		short sClipLeft = sClipL;

		// Skip the top:
		if (sClipT) // clip source from top:
			for (j=sClipT;j!=0;)
				{
				//========================
				// SKIP entire lines:
				//========================
				if ( (*pCode) == 255)
					{
					j--;
					// Check for multiple skipped lines:
					sCount = *(++pCode);
					if (sCount == 254)
						{
						while (*pCode == 254) // overflow:
							{
							sCount += *(++pCode);
							}
						pCode++; // sCount = # of whole lines to skip.
						j++; // we already decremented it.
						if (sCount <= j) {j -= sCount; continue;}
						else // mixed line skip:
							{
							sCount -= j; // How many left to draw:
							j = 0; // for main loop
							goto ENTER_LINE_DRAW_HV;
							}
						}
					else continue; // simple EOL
					}
				// ELSE
				//========================
				// skip a background run:
				//========================
				sCount = *pCode;
				if (sCount == 254) // overload
					{
					while (*pCode == 254)
						{
						sCount += 254;
						pCode++;
						}
					sCount += *pCode;
					}
				pCode++;
				// THEN
				//========================
				// skip a foreground run:
				//========================
				sCount = *pCode;
				if (sCount == 254) // overload
					{
					while (*pCode == 254)
						{
						sCount += 254;
						pCode++;
						}
					sCount += *pCode;
					}
				pCode++;
				//NEXT CLIPPED LINE
				}

		//**************************************************************
		//********************  BEGIN CORE BLIT  ***********************
		//**************************************************************
		for (j=0;j<sH;j++)
			{
			//========================
			// check for entire lines:
			//========================
			if ( (*pCode) == 255)
				{
				sCount = *(++pCode);
				while (*pCode == 254) // overflow:
					{
					sCount += *(++pCode);
					}
				pCode++;

		ENTER_LINE_DRAW_HV:
				// Do the block:
				if (ulBackColor)
					{
					// draw a block:
					for (l=0;l<sCount;l++)
						{
						pBuf.b = pLineBuf.b;
						for (k=0;k < sW + sAddW;k++)
							*(pBuf.p++) = ulBackColor;
						pBuf.b = (pLineBuf.b += lP);
						}
					}
				else
					pBuf.b = (pLineBuf.b += lP * sCount);

				j += sCount - 1;
				continue;	// more lines!
				}

			// Begin a horizontal run...
			sRemainingW=sW;
			//************************************************************
			//******   HANDLE LEFT_CLIPPING (Which will NOT need to know
			//******   about right clipping, as sClipL+sClipR<sW
			//************************************************************
			if (sClipL)
				{
				sClipLeft = sClipL;  // skip it all:
					
			NextRun_LEFT:
				//========================
				// do a background run:
				//========================
				sCount = *pCode;
				if (sCount == 254) // overload
					{
					while (*pCode == 254)
						{
						sCount += 254;
						pCode++;
						}
					sCount += *pCode;
					}
				pCode++;
				if (sCount > sClipLeft) // split run
					{
					sCount -= sClipLeft;
					goto ENTER_BACK_RUN;
					}
				sClipLeft -= sCount;

				//========================
				// do a foreground run:
				//========================
				sCount = *pCode;
				if (sCount == 254) // overload
					{
					while (*pCode == 254)
						{
						sCount += 254;
						pCode++;
						}
					sCount += *pCode;
					}
				pCode++;

				if (sCount > sClipLeft) // split run
					{
					sCount -= sClipLeft;
					goto ENTER_FORE_RUN;
					}
				sClipLeft -= sCount;

				//======= next run:
				if (*pCode != 255) goto NextRun_LEFT;

				pCode++;	// EOL;

				// Fill out the rest of the line:
				goto ENTER_END_RUN;
				}

			//******************************************** IN A SOLID RUN!
		NextRun_HV:
			//========================
			// do a background run:
			//========================
			sCount = *pCode;
			if (sCount == 254) // overload
				{
				while (*pCode == 254)
					{
					sCount += 254;
					pCode++;
					}
				sCount += *pCode;
				}
			pCode++;

			// Check for Right Clipping:
			if (sCount > sRemainingW) sCount = sRemainingW; // partial run!
			sRemainingW -= sCount;

		ENTER_BACK_RUN:
			//======= do the run:
			if (sRemainingW >= 0)
				{
				if (ulBackColor)
					for (i=0;i<sCount;i++)
						*(pBuf.p++) = ulBackColor;
				else	pBuf.p += sCount;
				}

			//========================
			// do a foreground run:
			//========================
			sCount = *pCode;
			if (sCount == 254) // overload
				{
				while (*pCode == 254)
					{
					sCount += 254;
					pCode++;
					}
				sCount += *pCode;
				}
			pCode++;

			// Check for Right Clipping:
			if (sCount > sRemainingW) sCount = sRemainingW; // partial run!
			sRemainingW -= sCount;

		ENTER_FORE_RUN:
			//======= do the run:
			if (sRemainingW >= 0)
				{
				if (ulForeColor)
					for (i=0;i<sCount;i++)
						*(pBuf.p++) = ulForeColor;
				else	pBuf.p += sCount;
				}

			//======= next run:
			if (*pCode != 255) goto NextRun_HV;

			pCode++;	// EOL;

			// Fill out the rest of the line:
		ENTER_END_RUN:
			if (ulBackColor)
				{
				while (pBuf.b < (pLineBuf.b + sW + sAddW))
					*(pBuf.p++) = ulBackColor;
				}

			pLineBuf.b += lP;
			pBuf.b = pLineBuf.b;
			}
		}

	// Don't worry about clipping yet... we're in a rush!

	//*******************************************************************
	// IN RELEASE MODE, GIVE THE USER A CHANCE:
#ifndef _DEBUG

	if (gsScreenLocked) goto BLIT_DONTUNLOCK;

#endif

	//********************
	// OS_SPECIFIC:
	//********************  UNLOCK WHATEVER YOU NEED TO
	switch (sNeedToUnlock)
		{
		case 0: // nothing to unlock
		break;

		case BUF_MEMORY:
			rspUnlockVideoBuffer();
		break;
		
		case BUF_VRAM:
			rspUnlockVideoPage();
		break;
		
		case BUF_VRAM2:
			rspUnlockVideoFlipPage();
		break;
		
		default:
			TRACE("BLiT:  Unlocking error!\n");
		}

BLIT_DONTUNLOCK:	
	return 0;
	}

//*****************************  THE FSPRITE1 SCALING BLiT  ******************************
// currently 8-bit, but soon to be full color......
// Must deal with screen locking.
// CURRENT STATUS -> fine clipping not perfected, Italics NOT integrated with clipping,
// Background not perfected with tabs or interchar spacing.
//
short	rspBlit(ULONG ulForeColor,ULONG ulBackColor,CImage* pimSrc,CImage* pimDst,
	short sDstX,short sDstY,short sDstW,short sDstH,Rect* prDst,short sAddW,short* psItalics)
	{
	// a special patch:
	if (pimDst->ulType == BMP1)
		{
		return rspBlitToMono(pimSrc,pimDst,sDstX,sDstY,sDstW,sDstH);
		}

#ifdef _DEBUG

	if ((pimSrc == NULL) || (pimDst == NULL))
		{
		TRACE("BLiT: Null Images passed!\n");
		return -1;
		}

	if (pimSrc->ulType != FSPR1)
		{
		TRACE("BLiT: Source MUST be type FSPR1 for this BLiT!\n");
		return -1;
		}

	if (!ImageIsUncompressed(pimDst->ulType))
		{
		TRACE("BLiT: Can only BLiT INTO uncompressed image types\n");
		return -1;
		}

	if ( (sDstW < 1) || (sDstH < 1))
		{
		TRACE("BLiT: Bad width or height input\n");
		return -1;
		}

	/*
	if ( (sDstW > pimSrc->lWidth) || (sDstH > pimSrc->lHeight) )
		{
		TRACE("BLiT: Cannot yet magnify an FSPR1 image.\n");
		return -1;
		}

	// Check for Destination clipping!

#endif

	union
		{
		UCHAR* b;
		UCHAR* p; // until get a consistant calling convention for BLiT!
		} pBuf,pLineBuf;
	// (SOB!) Hardcoded to 8-bit for now!
	UCHAR clrBKD = (UCHAR) ulBackColor;
	UCHAR clrLTR = (UCHAR) ulForeColor;

	u16Frac* frSkipX = u16fStrafe256((USHORT)sDstW,(USHORT)pimSrc->lWidth);	// < 1
	u16Frac* frSkipY = u16fStrafe256((USHORT)sDstH,(USHORT)pimSrc->lHeight);	// < 1

	short i,j,k,l,sCount;
	long	lDstP = pimDst->lPitch;
	u16Frac	frPosX = {0},frNewX = {0};
	u16Frac	frPosY = {0},frNewY = {0};
	short sPixSize = pimSrc->sDepth>>3;
	UCHAR* pCode = ((CSpecialFSPR1 *)pimSrc->pSpecial)->m_pCode;
	short sSrcW = (short)pimSrc->lWidth; // for clipping
	short sSrcH = (short)pimSrc->lHeight;
	short sDraw = 1; // This is a scanline skipper
	short sDenX = sSrcW,sDenY = sSrcH;
	short sClipL=0,sClipR=0,sClipT=0,sClipB=0;

	//----------------------- Clipping Control:
	// For clipping, adjust the destination accordingly but keep the
	// source at the origin for decompressed skipping
	//
	if (prDst)
		{
		// clip against user values
		sClipL = prDst->sX - sDstX; // positive = clipped
		if (sClipL > 0) { sDstW -= sClipL; sDstX = prDst->sX; }
		sClipT = prDst->sY - sDstY; // positive = clipped
		if (sClipT > 0) { sDstH -= sClipT; sDstY = prDst->sY; }
		// The order here still works because BOTH sDstX AND sW changed!
		sClipR = sDstX + sDstW - prDst->sX - prDst->sW; // positive = clipped
		if (sClipR > 0) { sDstW -= sClipR; }
		sClipB = sDstY + sDstH - prDst->sY - prDst->sH; // positive = clipped
		if (sClipB > 0) { sDstH -= sClipB; }

		if ( (sDstW <= 0) || (sDstH <= 0) ) return -1; // clipped out!
		}
	else	
		{
		// clip against full destination buffer
		sClipL = -sDstX;
		if (sClipL > 0) { sDstW -= sClipL; sDstX = 0; }
		sClipT = -sDstY;
		if (sClipR > 0) { sDstH -= sClipR; sDstY = 0; }
		// The order here still works because BOTH sDstX AND sW changed!
		sClipR = sDstX + sDstW - (short)pimDst->lWidth; // positive = clipped
		if (sClipR > 0) sDstW -= sClipR; // positive = clipped
		sClipB = sDstY + sDstH - (short)pimDst->lHeight; // positive = clipped
		if (sClipB > 0) sDstH -= sClipB; // positive = clipped

		if ((sDstW <= 0) || (sDstH <= 0)) return -1; // fully clipped
		}
		
	// Make positive:
	if (sClipL < 0) sClipL = 0;
	if (sClipR < 0) sClipR = 0;
	if (sClipT < 0) sClipT = 0;
	if (sClipB < 0) sClipB = 0;
	pLineBuf.b = pBuf.b = pimDst->pData + lDstP * sDstY + sDstX * sPixSize;
	//*******************************************
	//*** NOTE: currently, use of Italics will NOT work with ANY clipping!


	//===================================================================
	// Do Locking!
	// do OS based copying!
	short sNeedToUnlock = 0; // will be the name of a buffer to unlock.
	short sBlitTypeDst = 0;

	// IN RELEASE MODE, GIVE THE USER A CHANCE:
#ifndef _DEBUG

	if (gsScreenLocked) goto BLIT_PRELOCKED_RSPTXTSCL;

#endif

	// IN THIS IMPLEMENTATION, we must do LOCK, BLiT, UNLOCK, so I
	// must record which UNLOCK (if any) needs to be done AFTER the BLiT
	// has completed. (Lord help me if a blit gets interrupted)
	// NOT NECESSARY!!! THe SOURCE WILL ALWAYS BE A BUFFER!
	if (pimDst->ulType == IMAGE_STUB) sBlitTypeDst = (short)pimDst->pSpecial;

	switch (sBlitTypeDst) // 0 = normal image
		{
		case BUF_MEMORY: // image to system buffer
			// need to lock / unlock this one:
			if (rspLockVideoBuffer((void**)&(pimDst->pData),&(pimDst->lPitch))
				!=0)
				{
				TRACE("BLiT: Unable to lock the system buffer, failed!\n");
				return -1;
				}
			// Locked the system buffer, remember to unlock it:
			sNeedToUnlock = BUF_MEMORY;			
		break;

		case BUF_VRAM: // image to front screen
			// need to lock / unlock this one:
			if (rspLockVideoPage((void**)&(pimDst->pData),&(pimDst->lPitch))
				!=0)
				{
				TRACE("BLiT: Unable to lock the OnScreen system buffer, failed!\n");
				return -1;
				}
			// Locked the front VRAM, remember to unlock it:
			sNeedToUnlock = BUF_VRAM;			
		break;

		case BUF_VRAM2: // image to back screen
			// need to lock / unlock this one:
			if (rspLockVideoFlipPage((void**)&(pimDst->pData),&(pimDst->lPitch))
				!=0)
				{
				TRACE("BLiT: Unable to lock the OffScreen system buffer, failed!\n");
				return -1;
				}
			// Locked the front VRAM, remember to unlock it:
			sNeedToUnlock = BUF_VRAM;			
		break;

		case 0:  // image to image
			sNeedToUnlock = 0;
		break;

		default:
			TRACE("BLiT: This type of copy is not yet supported.\n");
			return -1;
		}

BLIT_PRELOCKED_RSPTXTSCL:

//***************************************************************************
//********* DO ITALICS STUFF (WILL NOT WORK WITH CLIPPING! ******************
//***************************************************************************

	// Handle HClip case separately:
	if (!sClipL && !sClipR)
		{
		//***************************************************************************
		//*****************************  TOP CLIP!  *********************************
		//***************************************************************************
		// KEY crucial difference!  Because FSPR1 is non-random access, this is
		// SOURCE based vertical advancement.  Destination position must be monitored!
		// The Litmus is always (frNewY.delta - frPosY.delta)
		//
		if (sClipT)
			while (frNewY.delta < sClipT) // skip over stuff
				{
				if ( (*pCode) == 255) // block of null lines:
					{	// find the total count:
					pCode++;
					do	{  Add(frNewY,frSkipY[*pCode],sDenY);
						} while (*(pCode++) == 254); 

					if ( (sCount = (frNewY.delta - sClipT)) > 0)
						goto ENTER_LINE_DRAW_SCL; // mixed run

					pCode++;
					continue;
					}
				// regular lines
				do	{ // skip a line with data, 254 is overflow code for runs...
					while (*(pCode++) == 254) ; // skip Background run
					while (*(pCode++) == 254) ; // skip Foreground run
					} while (*pCode != 255);

				pCode++; // next line
				Add(frNewY,frSkipY[1],sDenY);
				}
		//***************************************************************************

		//===========================================================================
		//***************************************************************************
		// A core BLiT
		//**********************************************************
		// Do the BLiT!
		for (j=0;j<sDstH;) // j is the dest y location
			{
			// New Line, reset IC:
			frPosX.delta = frPosX.frac = 0;
			frNewX = frPosX; // go for the offset!
			frPosY = frNewY;

			//========================
			// check for entire lines:
			//========================
			if ( (*pCode) == 255)
				{
				//sCount = *(++pCode);
				pCode++;
				Add(frNewY,frSkipY[*pCode],sDenY);
				while (*pCode == 254) // overflow:
					{
					//sCount += *(++pCode);
					pCode++;
					Add(frNewY,frSkipY[*pCode],sDenY);
					}
				pCode++;

				// Do the block:
				sCount = frNewY.delta - frPosY.delta;
				// Check for bottom clipping:
				if (frNewY.delta >= sDstH)
					sCount -= (frNewY.delta - sDstH);

	ENTER_LINE_DRAW_SCL:
				if (clrBKD)
					{
					// draw a block:
					for (l=0;l<sCount;l++,j++)
						{
						pBuf.b = pLineBuf.b + psItalics[j];
						for (k=0;k<sDstW + sAddW;k++)
							*(pBuf.p++) = clrBKD;
						pBuf.b = (pLineBuf.b += lDstP);
						}
					}
				else
					{
					j += sCount;
					pBuf.b = (pLineBuf.b += lDstP * sCount) + psItalics[j];
					}
				//j += sCount;
				sDraw = 1;
				continue;	// more lines!
				}
			
		NextRun:

			//============= SKIP a line!
			if (!sDraw)	// skip over lines as QUICKLY as possible
				{// IGNORE the x position:

				// skip a transparent run!
				while (*pCode == 254) pCode++;
				pCode++;

				// skip an opaque run:
				while (*pCode == 254) pCode++;
				pCode++;
				
				// More runs on this line?
				if (*pCode != 255) goto NextRun;

				pCode++;	// EOL;
				goto NextLine;
				}
			//==================================

			//========================
			// do a background run:
			//========================

			Add(frNewX,frSkipX[*pCode],sDenX);

			if (*pCode == 254) // overload
				{
				while (*pCode == 254)
					{
					Add(frNewX,frSkipX[254],sDenX);
					pCode++;
					}
				Add(frNewX,frSkipX[*pCode],sDenX);
				}
			pCode++;

			//======= do the run:
			sCount = frNewX.delta - frPosX.delta; // scale the run!

			if (clrBKD)
				for (i=0;i<sCount;i++)
					*(pBuf.p++) = clrBKD;
			else	pBuf.p += sCount;

			//========================
			// do a foreground run:
			//========================

			frPosX = frNewX;
			Add(frNewX,frSkipX[*pCode],sDenX);

			if (*pCode == 254) // overload
				{
				while (*pCode == 254)
					{
					Add(frNewX,frSkipX[254],sDenX);
					pCode++;
					}
				Add(frNewX,frSkipX[*pCode],sDenX);
				}
			pCode++;

			//======= do the run:
			sCount = frNewX.delta - frPosX.delta; // scale the run!

			if (clrLTR)
				for (i=0;i<sCount;i++)
					*(pBuf.p++) = clrLTR;
			else	pBuf.p += sCount;

			frPosX = frNewX;

			//======= next run:
			if (*pCode != 255) goto NextRun;

			pCode++;	// EOL;

			// Fill out the rest of the line:
			if (clrBKD)
				{
				while (pBuf.b < (pLineBuf.b + sDstW + sAddW))
					*(pBuf.p++) = clrBKD;
				}

			// STANDARD END OF LINE STUFF:
		NextLine:
			Add(frNewY,frSkipY[1],sDenY);
			if (frNewY.delta - frPosY.delta) 
				{
				pLineBuf.b += lDstP; // overwrite logic
				j++;
				sDraw = 1;
				}
			else	sDraw = 0;	// don't draw until next line!

			pBuf.b = pLineBuf.b + psItalics[j];
			}
		}
	else	// Handle the horizontally clipped case:
		{
		//***************************************************************************
		//*****************************  TOP CLIP!  *********************************
		//***************************************************************************
		// KEY crucial difference!  Because FSPR1 is non-random access, this is
		// SOURCE based vertical advancement.  Destination position must be monitored!
		// The Litmus is always (frNewY.delta - frPosY.delta)
		//
		if (sClipT)
			while (frNewY.delta < sClipT) // skip over stuff
				{
				if ( (*pCode) == 255) // block of null lines:
					{	// find the total count:
					pCode++;
					do	{  Add(frNewY,frSkipY[*pCode],sDenY);
						} while (*(pCode++) == 254); 

					if ( (sCount = (frNewY.delta - sClipT)) > 0)
						goto ENTER_LINE_DRAW_SCL_HV; // mixed run

					pCode++;
					continue;
					}
				// regular lines
				do	{ // skip a line with data, 254 is overflow code for runs...
					while (*(pCode++) == 254) ; // skip Background run
					while (*(pCode++) == 254) ; // skip Foreground run
					} while (*pCode != 255);

				pCode++; // next line
				Add(frNewY,frSkipY[1],sDenY);
				}
		//***************************************************************************

		//===========================================================================
		//***************************************************************************
		// A core BLiT
		//**********************************************************
		// Do the BLiT!
		for (j=0;j<sDstH;) // j is the dest y location
			{
			// New Line, reset IC:
			frPosX.delta = frPosX.frac = 0;
			frNewX = frPosX; // go for the offset!
			frPosY = frNewY;

			//========================
			// check for entire lines:
			//========================
			if ( (*pCode) == 255)
				{
				//sCount = *(++pCode);
				pCode++;
				Add(frNewY,frSkipY[*pCode],sDenY);
				while (*pCode == 254) // overflow:
					{
					//sCount += *(++pCode);
					pCode++;
					Add(frNewY,frSkipY[*pCode],sDenY);
					}
				pCode++;

				// Do the block:
				sCount = frNewY.delta - frPosY.delta;

	ENTER_LINE_DRAW_SCL_HV:
				if (clrBKD)
					{
					// draw a block:
					for (l=0;l<sCount;l++)
						{
						pBuf.b = pLineBuf.b;
						for (k=0;k<sDstW + sAddW;k++)
							*(pBuf.p++) = clrBKD;
						pBuf.b = (pLineBuf.b += lDstP);
						}
					}
				else
					pBuf.b = (pLineBuf.b += lDstP * sCount);

				j += sCount;
				sDraw = 1;
				continue;	// more lines!
				}
			
		NextRunHV:

			//============= SKIP a line!
			if (!sDraw)	// skip over lines as QUICKLY as possible
				{// IGNORE the x position:

				// skip a transparent run!
				while (*pCode == 254) pCode++;
				pCode++;

				// skip an opaque run:
				while (*pCode == 254) pCode++;
				pCode++;
				
				// More runs on this line?
				if (*pCode != 255) goto NextRunHV;

				pCode++;	// EOL;
				goto NextLineHV;
				}
			//==================================

			//========================
			// do a background run:
			//========================

			Add(frNewX,frSkipX[*pCode],sDenX);

			if (*pCode == 254) // overload
				{
				while (*pCode == 254)
					{
					Add(frNewX,frSkipX[254],sDenX);
					pCode++;
					}
				Add(frNewX,frSkipX[*pCode],sDenX);
				}
			pCode++;

			//======= do the run:
			if (frNewX.delta >= sClipL) // LCLIP:
				{
				sCount = frNewX.delta - frPosX.delta; // scale the run!
				if (frPosX.delta < sClipL) sCount -= (sClipL - frPosX.delta);
				if (frNewX.delta >= sDstW + sClipL) sCount -= (frNewX.delta - sDstW - sClipL); 

				if (clrBKD)
					for (i=0;i<sCount;i++)
						*(pBuf.p++) = clrBKD;
				else	pBuf.p += sCount;
				}

			//========================
			// do a foreground run:
			//========================

			frPosX = frNewX;
			Add(frNewX,frSkipX[*pCode],sDenX);

			if (*pCode == 254) // overload
				{
				while (*pCode == 254)
					{
					Add(frNewX,frSkipX[254],sDenX);
					pCode++;
					}
				Add(frNewX,frSkipX[*pCode],sDenX);
				}
			pCode++;

			//======= do the run:
			if (frNewX.delta >= sClipL) // LCLIP:
				{
				sCount = frNewX.delta - frPosX.delta; // scale the run!
				if (frPosX.delta < sClipL) sCount -= (sClipL - frPosX.delta);
				if (frNewX.delta >= sDstW + sClipL) sCount -= (frNewX.delta - sDstW - sClipL); 

				if (clrLTR)
					for (i=0;i<sCount;i++)
						*(pBuf.p++) = clrLTR;
				else	pBuf.p += sCount;
				}

			frPosX = frNewX;

			//======= next run:
			if (*pCode != 255) goto NextRun;

			pCode++;	// EOL;

			// Fill out the rest of the line:
			if (clrBKD)
				{
				while (pBuf.b < (pLineBuf.b + sDstW + sAddW))
					*(pBuf.p++) = clrBKD;
				}

			// STANDARD END OF LINE STUFF:
		NextLineHV:
			Add(frNewY,frSkipY[1],sDenY);
			if (frNewY.delta - frPosY.delta) 
				{
				pLineBuf.b += lDstP; // overwrite logic
				j++;
				sDraw = 1;
				}
			else	sDraw = 0;	// don't draw until next line!

			pBuf.b = pLineBuf.b;
			}
		}

	//***************************************************************************
	//===========================================================================

	// do unlocking......
	//*******************************************************************
	// IN RELEASE MODE, GIVE THE USER A CHANCE:
#ifndef _DEBUG

	if (gsScreenLocked) goto BLIT_DONTUNLOCK_RSPTXTSCL;

#endif

	//********************
	// OS_SPECIFIC:
	//********************  UNLOCK WHATEVER YOU NEED TO
	switch (sNeedToUnlock)
		{
		case 0:  // nothing to unlock
		break;

		case BUF_MEMORY:
			rspUnlockVideoBuffer();
		break;
		
		case BUF_VRAM:
			rspUnlockVideoPage();
		break;
		
		case BUF_VRAM2:
			rspUnlockVideoFlipPage();
		break;
		
		default:
			TRACE("BLiT:  Unlocking error!\n");
		}

BLIT_DONTUNLOCK_RSPTXTSCL:	

	//-----------------------------------------------------------------------------
	free(frSkipX);
	free(frSkipY);
	return 0;
	}
	

// Returns 0 on SUCCESS
// Does NOT currently clip at all
// Is currently hard coded for 8-bit color...
// For FSPR1 ONLY!
// Cannot YET go DTS!
// Right now it ONLY shrinks
// This needs to be called with precalculated skipX and skipY tables
// of 256 values.  Useful for fonts!
//
short	_rspBlit(ULONG ulClrLTR,ULONG ulClrBKD,CImage* pimSrc,CImage* pimDst,
				  short sDstX,short sDstY,short sW,short sH,
				  short sAddW,u16Frac* frSkipY,u16Frac* frSkipX);
short	_rspBlit(ULONG ulClrLTR,ULONG ulClrBKD,CImage* pimSrc,CImage* pimDst,
				  short sDstX,short sDstY,short sW,short sH,
				  short sAddW,u16Frac* frSkipY,u16Frac* frSkipX)
	{

#ifdef _DEBUG

	if ((pimSrc == NULL) || (pimDst == NULL))
		{
		TRACE("BLiT: Null Images passed!\n");
		return -1;
		}

	if (pimSrc->ulType != FSPR1)
		{
		TRACE("BLiT: Source MUST be type FSPR1 for this BLiT!\n");
		return -1;
		}

	if (!ImageIsUncompressed(pimDst->ulType))
		{
		TRACE("BLiT: Can only BLiT INTO uncompressed image types\n");
		return -1;
		}

	if ( (sW < 1) || (sH < 1))
		{
		TRACE("BLiT: Bad width or height input\n");
		return -1;
		}

	if ( (sW > pimSrc->lWidth) || (sH > pimSrc->lHeight) )
		{
		TRACE("BLiT: Cannot yet magnify an FSPR1 image.\n");
		return -1;
		}

	// Check for Destination clipping!

#endif

	union
		{
		UCHAR*	b;
		UCHAR* p; // until get a consistant calling convention for BLiT!
		} pBuf,pLineBuf;
	// (SOB!) Hardcoded to 8-bit for now!
	UCHAR clrBKD = (UCHAR) ulClrBKD;
	UCHAR clrLTR = (UCHAR) ulClrLTR;

	// Do Locking!
	// do OS based copying!
	short sNeedToUnlock = 0; // will be the name of a buffer to unlock.

	short sBlitTypeDst = 0;

	// IN RELEASE MODE, GIVE THE USER A CHANCE:
#ifndef _DEBUG

	if (gsScreenLocked) goto BLIT_PRELOCKED_TXTSCL;

#endif

	// IN THIS IMPLEMENTATION, we must do LOCK, BLiT, UNLOCK, so I
	// must record which UNLOCK (if any) needs to be done AFTER the BLiT
	// has completed. (Lord help me if a blit gets interrupted)
	// NOT NECESSARY!!! THe SOURCE WILL ALWAYS BE A BUFFER!
	if (pimDst->ulType == IMAGE_STUB) sBlitTypeDst = (short)pimDst->pSpecial;

	switch (sBlitTypeDst) // 0 = normal image
		{
		case BUF_MEMORY: // image to system buffer
			// need to lock / unlock this one:
			if (rspLockVideoBuffer((void**)&(pimDst->pData),&(pimDst->lPitch))
				!=0)
				{
				TRACE("BLiT: Unable to lock the system buffer, failed!\n");
				return -1;
				}
			// Locked the system buffer, remember to unlock it:
			sNeedToUnlock = BUF_MEMORY;			
		break;

		case BUF_VRAM: // image to front screen
			// need to lock / unlock this one:
			if (rspLockVideoPage((void**)&(pimDst->pData),&(pimDst->lPitch))
				!=0)
				{
				TRACE("BLiT: Unable to lock the OnScreen system buffer, failed!\n");
				return -1;
				}
			// Locked the front VRAM, remember to unlock it:
			sNeedToUnlock = BUF_VRAM;			
		break;

		case BUF_VRAM2: // image to back screen
			// need to lock / unlock this one:
			if (rspLockVideoFlipPage((void**)&(pimDst->pData),&(pimDst->lPitch))
				!=0)
				{
				TRACE("BLiT: Unable to lock the OffScreen system buffer, failed!\n");
				return -1;
				}
			// Locked the front VRAM, remember to unlock it:
			sNeedToUnlock = BUF_VRAM;			
		break;

		case 0:  // image to image
			sNeedToUnlock = 0;
		break;

		default:
			TRACE("BLiT: This type of copy is not yet supported.\n");
			return -1;
		}

BLIT_PRELOCKED_TXTSCL:

	short sFreeSkip = 0;

	if (frSkipX == NULL)
		{
		frSkipX = u16fStrafe256((USHORT)sW,(USHORT)pimSrc->lWidth);	// < 1
		sFreeSkip = 1;
		}

	if (frSkipY == NULL)
		{
		frSkipY = u16fStrafe256((USHORT)sH,(USHORT)pimSrc->lHeight);	// < 1
		sFreeSkip += 2;
		}

	short i,j,k,l,sCount;
	long	lP = pimDst->lPitch;
	u16Frac	frPosX = {0},frNewX = {0};
	u16Frac	frPosY = {0},frNewY = {0};
	short sPixSize = pimSrc->sDepth>>3;
	pLineBuf.b = pBuf.b = pimDst->pData + lP * sDstY + sDstX * sPixSize;
	UCHAR* pCode = ((CSpecialFSPR1 *)pimSrc->pSpecial)->m_pCode;
	short sSrcW = (short)pimSrc->lWidth;
	short sSrcH = (short)pimSrc->lHeight;
	short sDraw = 1; // This is a scanline skipper
	short sDenX = sSrcW,sDenY = sSrcH;

	//**********************************************************
	// Do the BLiT!
	for (j=0;j<sH;) // j is the dest y location
		{
		// New Line, reset IC:
		frPosX.delta = frPosX.frac = 0;
		frNewX = frPosX; // go for the offset!
		frPosY = frNewY;

		//========================
		// check for entire lines:
		//========================
		if ( (*pCode) == 255)
			{
			//sCount = *(++pCode);
			pCode++;
			Add(frNewY,frSkipY[*pCode],sDenY);
			while (*pCode == 254) // overflow:
				{
				//sCount += *(++pCode);
				pCode++;
				Add(frNewY,frSkipY[*pCode],sDenY);
				}
			pCode++;

			// Do the block:
			sCount = frNewY.delta - frPosY.delta;

			if (clrBKD)
				{
				// draw a block:
				for (l=0;l<sCount;l++)
					{
					pBuf.b = pLineBuf.b;
					for (k=0;k<sW + sAddW;k++)
						*(pBuf.p++) = clrBKD;
					pBuf.b = (pLineBuf.b += lP);
					}
				}
			else
				pBuf.b = (pLineBuf.b += lP * sCount);

			j += sCount;
			sDraw = 1;
			continue;	// more lines!
			}
		
	NextRun:

		//============= SKIP a line!
		if (!sDraw)	// skip over lines as QUICKLY as possible
			{// IGNORE the x position:

			// skip a transparent run!
			while (*pCode == 254) pCode++;
			pCode++;

			// skip an opaque run:
			while (*pCode == 254) pCode++;
			pCode++;
			
			// More runs on this line?
			if (*pCode != 255) goto NextRun;

			pCode++;	// EOL;
			goto NextLine;
			}
		//==================================

		//========================
		// do a background run:
		//========================

		Add(frNewX,frSkipX[*pCode],sDenX);

		if (*pCode == 254) // overload
			{
			while (*pCode == 254)
				{
				Add(frNewX,frSkipX[254],sDenX);
				pCode++;
				}
			Add(frNewX,frSkipX[*pCode],sDenX);
			}
		pCode++;

		//======= do the run:
		sCount = frNewX.delta - frPosX.delta; // scale the run!

		if (clrBKD)
			for (i=0;i<sCount;i++)
				*(pBuf.p++) = clrBKD;
		else	pBuf.p += sCount;

		//========================
		// do a foreground run:
		//========================

		frPosX = frNewX;
		Add(frNewX,frSkipX[*pCode],sDenX);

		if (*pCode == 254) // overload
			{
			while (*pCode == 254)
				{
				Add(frNewX,frSkipX[254],sDenX);
				pCode++;
				}
			Add(frNewX,frSkipX[*pCode],sDenX);
			}
		pCode++;

		//======= do the run:
		sCount = frNewX.delta - frPosX.delta; // scale the run!

		if (clrLTR)
			for (i=0;i<sCount;i++)
				*(pBuf.p++) = clrLTR;
		else	pBuf.p += sCount;

		frPosX = frNewX;

		//======= next run:
		if (*pCode != 255) goto NextRun;

		pCode++;	// EOL;

		// Fill out the rest of the line:
		if (clrBKD)
			{
			while (pBuf.b < (pLineBuf.b + sW + sAddW))
				*(pBuf.p++) = clrBKD;
			}

		// STANDARD END OF LINE STUFF:
	NextLine:
		Add(frNewY,frSkipY[1],sDenY);
		if (frNewY.delta - frPosY.delta) 
			{
			pLineBuf.b += lP; // overwrite logic
			j++;
			sDraw = 1;
			}
		else	sDraw = 0;	// don't draw until next line!

		pBuf.b = pLineBuf.b;
		}

	//**********************************************************

	switch (sFreeSkip)
		{
		case 1:
			free(frSkipX);
		break;
		case 2:
			free(frSkipY);
		break;
		case 3:
			free(frSkipX);
			free(frSkipY);
		break;
		}

	// do unlocking......
	//*******************************************************************
	// IN RELEASE MODE, GIVE THE USER A CHANCE:
#ifndef _DEBUG

	if (gsScreenLocked) goto BLIT_DONTUNLOCK_TXTSCL;

#endif

	//********************
	// OS_SPECIFIC:
	//********************  UNLOCK WHATEVER YOU NEED TO
	switch (sNeedToUnlock)
		{
		case 0:  // nothing to unlock
		break;

		case BUF_MEMORY:
			rspUnlockVideoBuffer();
		break;
		
		case BUF_VRAM:
			rspUnlockVideoPage();
		break;
		
		case BUF_VRAM2:
			rspUnlockVideoFlipPage();
		break;
		
		default:
			TRACE("BLiT:  Unlocking error!\n");
		}

BLIT_DONTUNLOCK_TXTSCL:	

	return 0;
	}

	/*
void	instantiateBLIT()
	{
	CImage* pim = NULL;
	_rspBlit( (UCHAR)0,(UCHAR)0,pim,pim,(short)0,(short)0,(short)0,(short)0);
	_rspBlit( (USHORT)0,(USHORT)0,pim,pim,(short)0,(short)0,(short)0,(short)0);
	_rspBlit( (ULONG)0,(ULONG)0,pim,pim,(short)0,(short)0,(short)0,(short)0);
	}

	*/
