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
#include "System.h"
//********************************************
//*** This file should be called "ToBMP1"
//*** Currently, You can BLiT FSPR1 scaled and
//*** unscaled into a BMP1.
//       ( support for FSRP8 NYI )
//********************************************

#ifdef PATHS_IN_INCLUDES
	#include "GREEN/BLiT/BLIT.H"
	#include "GREEN/BLiT/_BlitInt.H"
	#include "ORANGE/QuickMath/Fractions.h"
	#include "GREEN/Image/SpecialTyp.h"
#else
	#include "BLIT.H"
	#include "_BlitInt.H" 
	#include "Fractions.h" 
	#include "specialtyp.h"
#endif

#include <string.h>

//***************************************************************************
// Mimicking the success of FSPR8, the newest FSPR1 is GREATLY simplified.  
// The result is that large images with runs above 254 will have worse
// compression, and decode slower, but images with runs less than 255
// will decode faster.  Also, simplified encoding/decoding will enhance
// portability.  To monitor this inefficiency, it will return the number
// of overruns.
//***************************************************************************
// THIS CHANGE REPRESENTS FSPR1 as of 9/23/96
//***************************************************************************
// FILE note... Mac compatibility seems to work using ASCII (printf/scanf)
// provided the PC opens the file using "wb" and adds its own newlines.
//***************************************************************************
// FONT note:  Font specific information will NOT be stored in the FSPR1
// itself, but in the CFNT Letter description associated with the FSPR1.
// UPDATE **** Some font info will be in there ...
// The new conversion does NOT assume the method of lasso, so that you 
// can "capture" regions of a larger BMP (useful for fonts.)  The default is
// to convert the BMP REGION AS IS, including padding around the edges.
//***************************************************************************
// NOTE: Unlike FSPR8, the palette will NOT be copied or preserved.
// This is based on the idea that since all the color information is
// lost from the image, it isn't worth taking up space with the palette,
// since the old image can't be restored.

//----------------  Predeclare linklate functions ---------------------------
short		DeleteFSPR1(RImage* pImage);
short		ConvertToFSPR1(RImage* pImage);
short		ConvertFromFSPR1(RImage* pImage);
short		LoadFSPR1(RImage* pImage, RFile* pcf);
short		SaveFSPR1(RImage* pImage, RFile* pcf);

//----------------  EXTRA Conversion Information ----------------------------

typedef	struct
	{
	U32	u32TransparentColor;	// ConvertTo 
	S16	sX,sY,sW,sH;			// ConvertTo (-1 == don't use)
	RImage** ppimNew;				// Create a separate CImage if not NULL!
	U32	u32ForeColor;			// ConvertFrom
	U32	u32BackColor;			// ConvertFrom (if s16Transparent==FALSE)
	S16	s16Transparent;		// ConvertFrom (flag)
	S16	sOverRun;				// Statistical info!
	} ConversionInfoFSPR1;

ConversionInfoFSPR1 gFSPR1 = 
	{
	(U32) 0,
	(S16) -1,
	(S16) -1,
	(S16) -1,
	(S16) -1,
	(RImage**) NULL,
	(U32)	0xffffff01,	// And the color to the correct depth
	(U32)	0,
	(S16) TRUE
	}; // Defaults...

void ResetFSPR1();
void ResetFSPR1()	// only traits DESIRABLE to reset between Converts
	{
	gFSPR1.sX=-1;
	gFSPR1.sY=-1;
	gFSPR1.sW=-1;
	gFSPR1.sH=-1;
	gFSPR1.ppimNew=NULL;
	}

short GetOverRuns();
short GetOverRuns() { return gFSPR1.sOverRun; }

//----------------  Set Conversion Information  ----------------

void SetConvertToFSPR1
	(	
	U32	u32TransparentColor,
	S16	sX,	// Use (-1) to use default value
	S16	sY,	// Use (-1) to use default value
	S16	sW,	// Use (-1) to use default value
	S16	sH,	// Use (-1) to use default value
	RImage**	ppimCopy
	)
	{
	gFSPR1.u32TransparentColor = u32TransparentColor;
	gFSPR1.sX = sX;
	gFSPR1.sY = sY;
	gFSPR1.sW = sW;
	gFSPR1.sH = sH;
	gFSPR1.ppimNew = ppimCopy;
	} 

void SetConvertFromFSPR1
	(
	U32	u32ForeColor,
	S16	sTransparent,
	U32	u32BackColor	// matters only if sTransparent = FALSE
	)
	{
	gFSPR1.u32ForeColor = u32ForeColor;
	gFSPR1.s16Transparent = sTransparent;
	gFSPR1.u32BackColor = u32BackColor;
	}

//-------------------  HOOK into the CImage world ------------------------
IMAGELINKLATE(FSPR1,ConvertToFSPR1,ConvertFromFSPR1,
				  LoadFSPR1,SaveFSPR1,NULL,DeleteFSPR1);

short		DeleteFSPR1(RImage* pImage) // from CImage ONLY
	{
	delete (RSpecialFSPR1*) pImage->m_pSpecialMem;
	return SUCCESS;
	}
//------------------------------------------------------------------------

// Assume that pImage is valid...
//
short ConvertToFSPR1(RImage* pImage)
	{

#ifdef _DEBUG

	if (!ImageIsUncompressed(pImage->m_type))
		{
		TRACE("ConvertToFSPR1: Source must be uncompressed.\n");
		return RImage::NOT_SUPPORTED;
		}

#endif

	//==========================  Let's Rock!  ========================

	// What should we snag here?
	short sX = 0, sY = 0, sW = pImage->m_sWidth,sH = pImage->m_sHeight;
	if (gFSPR1.sX != -1) sX = gFSPR1.sX;
	if (gFSPR1.sY != -1) sY = gFSPR1.sY;
	if (gFSPR1.sW != -1) sW = gFSPR1.sW;
	if (gFSPR1.sH != -1) sH = gFSPR1.sH;

	//----------------------------------- Autoclip:
	short sTemp;
	if (sX < 0) sX = 0;
	if (sY < 0) sY = 0;
	if ( (sTemp = sX + sW - pImage->m_sWidth) > 0) sW -= sTemp;
	if ( (sTemp = sY + sH - pImage->m_sHeight) > 0) sH -= sTemp;

	if ( (sW < 1) || (sH < 1) )
		{
		TRACE("ConvertToFSPR1:  Bad region passed.\n");
		return RImage::NOT_SUPPORTED;
		}

	//----------------------------------- Begin Compressing:
	short sBlankLineCount = 0;
	short sCount = 0;

	// The fastest way is still to create a buffer of fixed size, then to 
	// shrink it.  This is risky, since there is no true max size for this
	// compression techique.  If there are no over-runs, a checkerboard
	// pattern would need h * (w+1) + 2.  This seems like a reasonable Max,
	// Since every run of 3 counteracts an overrun.

	long	lMaxSize = (long)sW * (sH + 2) * 2 + sH;
	lMaxSize = (lMaxSize + 15) & ~15; // 128 bit alignment

	UCHAR*	pCodeBuf = (UCHAR*) calloc(1,lMaxSize);
	if (pCodeBuf == NULL)
		{
		TRACE("ConvertToFSPR1: Out of memory ERROR!");
		ResetFSPR1();
		return RImage::NOT_SUPPORTED;
		}
	
	UCHAR*	pCode = pCodeBuf;
	long	lP =  pImage->m_lPitch;
	UCHAR	*pBuf,*pBufLine = pImage->m_pData + sX + lP*sY; // 8-bit for now!
	UCHAR	ucTranCol = (UCHAR) (gFSPR1.u32TransparentColor & 0xff); // 8-bit for now!
	short sLineLen,sLineW = sW;
	short y;

	sBlankLineCount = 0;
	short sBlank;
	short sOverRun = 0; // For diagnostics!

	//-------------------- COMPRESS IT! --------------------
	for (y=0;y<sH;y++)
		{
		//********************* Do a line: *********************
		pBuf = pBufLine;
		sBlank = TRUE;
		sLineLen = sLineW;

		while (TRUE) // ENTER RUNS:
			{
			//---------- Skip Run:
			sCount = 0;
			while ((*pBuf == ucTranCol) && sLineLen)
				{ 
				pBuf++; sCount++; sLineLen--;
				}

			//************ CHECK for EOL in a skip run:
			if (!sLineLen) // EOL:
				{
				if (sBlank == TRUE)	// nothing on this line, write nothing!
					sBlankLineCount++;// until we count them all!
				else
					*pCode++ = 255; // Cancel the skip, insert EOL!  
				goto NEXT_Y;		// begin the next line!
				}

			// ELSE... Not a blank line... enter skip line count and 
			// then the clear run count!
			//************ FILL IN POSSIBLE SKIPPED BLANK LINES
			if (sBlankLineCount > 0) // Now add in the blank line skip code:
				{
				while (sBlankLineCount > 254)
					{
					*pCode++ = 255;
					*pCode++ = 254;
					sBlankLineCount -= 254;
					}
				*pCode++ = 255;
				*pCode++ = (UCHAR)sBlankLineCount;
				sBlankLineCount = 0;
				}

			//=========== Insert the skip run...
			while (sCount > 254) // overflow
				{
				sOverRun++; // for diagnostics!
				*pCode++ = 254; // Cycle through runs...
				*pCode++ = 0;  // No skip
				sCount -= 254;
				}
			*pCode++ = (UCHAR)sCount; // Enter skip run..

			//---------- Opaque Run:
			sCount = 0;
			while ((*pBuf != ucTranCol) && sLineLen)
				{ 
				sBlank = FALSE; pBuf++; sCount++; sLineLen--;
				}

			//================= Insert the opaque run:
			while (sCount > 255) // overflow
				{
				sOverRun++; // for diagnostics!
				*pCode++ = 255; // Cycle through runs...
				*pCode++ = 0;   // Next clear run
				sCount -= 255;
				}
			*pCode++ = (UCHAR)sCount; // Enter opaque run..
			}
	NEXT_Y:
		pBufLine += lP;
		}

	// ADD in final trailing line skipping:
	if (sBlankLineCount > 0) // Now add in the blank line skip code:
		{
		while (sBlankLineCount > 254)
			{
			*pCode++ = 255;
			*pCode++ = 254;
			sBlankLineCount -= 254;
			}
		*pCode++ = 255;
		*pCode++ = (UCHAR)sBlankLineCount;
		}

	// Final Code:  (Safety cap)
	*pCode++ = 255;
	*pCode++ = 255;

	//****************** SHRINK THE BUFFER!
	long lCompressedSize = pCode - pCodeBuf + 1;
	long lAlignSize = (lCompressedSize + 15) & ~15;
	UCHAR* pNewCodeBuf = (UCHAR*) calloc(1,lAlignSize); //+ Free problem
	if (pNewCodeBuf == NULL)
		{
		TRACE("ConvertToFSPR1: Out of memory ERROR!");
		free(pCodeBuf);
		ResetFSPR1();
		return RImage::NOT_SUPPORTED;
		}

	memcpy(pNewCodeBuf,pCodeBuf,lCompressedSize);
	if (long(pCode - pCodeBuf + 1) > lMaxSize)
		{
		TRACE("ConvertToFSPR1: I overran my own buffer!\n");
		}
	free(pCodeBuf);
	gFSPR1.sOverRun = sOverRun; // for diagnostics
	//******************************************************************
	//-------  Let's make a new Image!
	RImage* pimNew = new RImage;
	pimNew->m_type = RImage::FSPR1;
	RSpecialFSPR1* pSpecial = new RSpecialFSPR1; //+ Free problem
	pSpecial->m_OldType = pImage->m_type;
	pSpecial->m_pCode = pNewCodeBuf;
	pSpecial->m_lSize = lCompressedSize; // Set font specific stuff yourself!
	pSpecial->m_u16Width = USHORT(sW); // so the default kerning makes sense!
	pimNew->m_pSpecial = pimNew->m_pSpecialMem = (UCHAR*)pSpecial;
	pimNew->m_sWidth = sW;
	pimNew->m_sHeight = sH;
	pimNew->m_sWinWidth = sW;
	pimNew->m_sWinHeight = sH;
	pimNew->m_sWinX = 0;
	pimNew->m_sWinY = 0;
	pimNew->m_sDepth = (short)8;
	pimNew->m_lPitch = (long)pimNew->m_sWidth; // Pitch is meaningless here!

	//*********************  should we transfer it over?  **************
	if (gFSPR1.ppimNew != NULL) // make a copy:
		{
		*(gFSPR1.ppimNew) = pimNew;
		ResetFSPR1();
		return RImage::FSPR1;
		}
	//---------------- transfer to the original:
	pImage->m_pSpecial = pImage->m_pSpecialMem = pimNew->m_pSpecialMem;
	// WE MUST REMOVE THE OLD BUFFER:
	//pImage->DestroyData();
	void* pTempData = pImage->DetachData();
	RImage::DestroyDetachedData(&pTempData);

	pImage->DestroyPalette(); // Don't save the palette info!
	pImage->m_ulSize = 0;	// BLiT needs to deal with copying, etc....
	pImage->m_type = RImage::FSPR1;
	// free the copy header:
	pimNew->m_pSpecialMem = NULL;
	delete pimNew; // it's gone!

	return RImage::FSPR1;
	}

// Internal BLitting of FSPR1
// Designed for use by rspBLiT, so assume all the input is validated, etc...
//
// DOES NOT CURRENTLY CLIP!!!!!
// is currently 8-bit color!
//
void _rspBLiT(UCHAR ucColor,RImage* pimSrc,RImage* pimDst,
				  short sDstX,short sDstY)
	{

#ifdef _DEBUG
	
	if (pimSrc->m_pSpecialMem == NULL)
		{
		TRACE("_rspBLiT (FSPR1): corrupted source image!\n");
		return;
		}

	if ( (sDstX < 0) || (sDstY < 0) ||
		( (sDstX + pimSrc->m_sWidth) > pimDst->m_sWidth) ||
		( (sDstY + pimSrc->m_sHeight) > pimDst->m_sHeight) )
		{
		TRACE("_rspBLiT (FSPR1): Cannot yet clip the image!\n");
		return;
		}

#endif

	long lP = pimDst->m_lPitch;
	UCHAR *pDst,*pDstLine = pimDst->m_pData + sDstX + lP*sDstY;
	UCHAR	*pCode = ((RSpecialFSPR1*)pimSrc->m_pSpecialMem)->m_pCode;
	// Take advantage of the new FSPR1 EOS safety code:
	UCHAR	ucCode,ucCount;

	while (TRUE) // loop through drawing lines:
		{
		// new line!
		pDst = pDstLine;
		if ((ucCode = *pCode++) == 255) // multiline skip:
			{
			if ((ucCount = *pCode++) == 255) break;	// DONE DRAWING!!!!!!
			pDstLine += lP * ucCount; // Advance n lines
			continue; ///********** Start next scanline!
			}

		do	{ // Draw the scanline!
			pDst += ucCode;//HSKIP
			ucCount = *pCode++;
			while (ucCount--) *pDst++ = ucColor;//HRUN
			} while ((ucCode = *pCode++) != 255);

		pDstLine += lP;
		}
	}


// Will convert back to the source 8-bit format.
// 
short		ConvertFromFSPR1(RImage* pImage)
	{
	// Here's the pattern:  create the appropriate buffer, BLiT
	// pSpecial into it, then kill pSpecial!
	// MUST USE ConvertFrom Parameters...
	//
	RSpecialFSPR1* pHead = (RSpecialFSPR1*)pImage->m_pSpecialMem; // Got it!
	// Do it 8-bit for now...
	pImage->CreateImage(pImage->m_sWidth,pImage->m_sHeight,(RImage::Type)pHead->m_OldType);

	// color the background:
	if (gFSPR1.s16Transparent != TRUE) 
		{
		rspRect(gFSPR1.u32BackColor,pImage,0,0,pImage->m_sWidth,
			pImage->m_sHeight);
		}
	
	// Blit it in:
	// Sadly, we must make a FAKE wrapper for the destination to use the same
	// rspBLiT... wait a sec...
	// This one should fool it!  It only works in 8-bit color for now!
	//
	_rspBLiT((UCHAR)gFSPR1.u32ForeColor,pImage,pImage,(short)0,(short)0);

	// Now jettison the FSPR1 data:
	delete pHead;
	pImage->m_pSpecial = pImage->m_pSpecialMem = NULL;

	return (short)pImage->m_type;
	}

// Assumes all of the FSPR1 has successfully been
// loaded EXCEPT for pSpecial[Mem]
//
short		LoadFSPR1(RImage* pImage, RFile* pcf)
	{
	long	lBogus1	= pcf->Tell();

	//------------------
	// Initial Security:
	//------------------
	char szTemp[10];
	pcf->Read(&szTemp[0]);

	// Check type:
	if (strcmp(szTemp,"__FSPR1__")) // not equal
		{
		TRACE("Load FSPR1: Not correct file type!\n");
		return -1;
		}

	// Check Version:
	U16 u16Temp;
	pcf->Read(&u16Temp);

	if (u16Temp != ((3<<8) + 5) )
		{
		TRACE("Load FSPR1: This is an older FSPR1 format!\n");
		return -1;
		}

	//------------------
	// Info
	//------------------

	RSpecialFSPR1* pSpec = new RSpecialFSPR1;
	pImage->m_pSpecialMem = pImage->m_pSpecial = (UCHAR*)pSpec;

	pcf->Read((U32*)(&pSpec->m_OldType));
	pcf->Read(&pSpec->m_lSize);
	pcf->Read(&pSpec->m_u16ASCII);
	pcf->Read(&pSpec->m_s16KernL); // True type compatibilit
	pcf->Read(&pSpec->m_u16Width);
	pcf->Read(&pSpec->m_s16KernR);

	//------------------
	// Reserved Space
	//------------------

	// Reserved for future expansion:
	U32 u32Temp[4];

	pcf->Read(u32Temp,4); // 16 bytes reserved as of version 3.5

	//------------------
	// The Image Data
	//------------------

	// Now the actual data, which needs no alignment:
	pSpec->m_pCode = (U8*) malloc(pSpec->m_lSize);    //+ Not freed!
	pcf->Read((U8*)(pSpec->m_pCode),pSpec->m_lSize);

	return SUCCESS;
	}

// SAVES ONLY the pSpecial information!
//
short		SaveFSPR1(RImage* pImage, RFile* pcf)
	{
	// Assume valid pImage and cnfile:
	RSpecialFSPR1* pSpec = (RSpecialFSPR1*) pImage->m_pSpecialMem;

	if (!pSpec)
		{
		TRACE("Save FSPR1: Bad FSPR1!\n");
		return -1;
		}

	//------------------
	// Initial Security:
	//------------------

	pcf->Write("__FSPR1__"); // image type
	U16 version = (U16)((3<<8) + 5); // Sprite incarnation 3, File Format 5
	pcf->Write(&version);

	//------------------
	// Info
	//------------------

	// NOTE: Some font info is stored here:
	pcf->Write((U32*)(&(pSpec->m_OldType)));
	pcf->Write(&(pSpec->m_lSize));
	pcf->Write(&(pSpec->m_u16ASCII));
	pcf->Write(&(pSpec->m_s16KernL)); // True type compatibilit
	pcf->Write(&(pSpec->m_u16Width));
	pcf->Write(&(pSpec->m_s16KernR));

	//------------------
	// Reserved Space
	//------------------

	// Reserved for future expansion
	U32 reserved[4] = {0,0,0,0};
	pcf->Write(reserved,4); // 16 bytes reserved as of version 3.5

	//------------------
	// The Image Data
	//------------------

	// Now the actual data, which needs no alignment:
	pcf->Write((U8*)pSpec->m_pCode,pSpec->m_lSize);

	return SUCCESS;
	}

//************************************************************
//*****************  BLiTting onto BMP8's  *******************
//************************************************************


//************************************************************
//*********************  NON-SCALING!  ***********************
//************************************************************
// 8-bit color for now!

// Doesn't clip...
// This WILL draw (ulForeColor == 0) into the bitmap!
// /* if (ulBackColor != 0) it will pre-rect it in. */
// Let a higher level function do tha background rect!
// Make s for an easier transition!
//
short rspBlit(
				  ULONG ulForeColor,
				  RImage* pimSrc,
				  RImage* pimDst,
				  short sDstX,
				  short sDstY,
				  const RRect* prDst
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

	if (pimDst->m_sDepth > 8)
		{
		TRACE("BLiT: TC sprites are not YET implemented for FSPR1.\n");
		return -1;
		}

#endif
	
	// transfer colors:
	UCHAR	ucForeColor = (UCHAR) ulForeColor;

	// Clip!
	short sClipL=0,sClipR=0,sClipT=0,sClipB=0;
	short sW = pimSrc->m_sWidth; // clippng parameters...
	short sH = pimSrc->m_sHeight; // clippng parameters...
	long	lDstP = pimDst->m_lPitch;

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
		sClipR = sDstX + sW - pimDst->m_sWidth; // positive = clipped
		if (sClipR > 0) sW -= sClipR; // positive = clipped
		sClipB = sDstY + sH - pimDst->m_sHeight; // positive = clipped
		if (sClipB > 0) sH -= sClipB; // positive = clipped

		if ((sW <= 0) || (sH <= 0)) return -1; // fully clipped
		}
		
	// Make positive:
	if (sClipL < 0) sClipL = 0;
	if (sClipR < 0) sClipR = 0;
	if (sClipT < 0) sClipT = 0;
	if (sClipB < 0) sClipB = 0;

	//**************  INSERT BUFFER HOOKS HERE!  ************************

	// do OS based copying!
	short sNeedToUnlock = 0; // will be the name of a buffer to unlock.
	short sBlitTypeDst = 0;

	// IN RELEASE MODE, GIVE THE USER A CHANCE:
#ifndef _DEBUG

	//if (gsScreenLocked || gsBufferLocked) goto BLIT_PRELOCKED;

#endif

	// IN THIS IMPLEMENTATION, we must do LOCK, BLiT, UNLOCK, so I
	// must record which UNLOCK (if any) needs to be done AFTER the BLiT
	// has completed. (Lord help me if a blit gets interrupted)
	if (pimDst->m_type == RImage::IMAGE_STUB) sBlitTypeDst = (short)((long)pimDst->m_pSpecial);

	switch (sBlitTypeDst) // 0 = normal image
		{
		case BUF_MEMORY: // image to system buffer
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

		case BUF_VRAM: // image to front screen
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

		case BUF_VRAM2: // image to back screen
			// need to lock / unlock this one:
			if (rspLockVideoFlipPage((void**)&(pimDst->m_pData),&(pimDst->m_lPitch))
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
//BLIT_PRELOCKED:

	// Check for locking error:
	if (!pimDst->m_pData)
		{
		TRACE("BLiT: NULL data - possible locking error.\n");
		return FAILURE;
		}

	UCHAR	*pDst,*pDstLine,*pCode,ucCount;
	pDstLine = pimDst->m_pData + lDstP * sDstY + sDstX;
	RSpecialFSPR1*	pHead = (RSpecialFSPR1*)(pimSrc->m_pSpecialMem);
	pCode = pHead->m_pCode;
	const UCHAR FF = (UCHAR)255;


	//***********************************************************
	//*****************  AT LAST!   CODE!  **********************
	//***********************************************************
	//
	
	//***********************************************************
	//*********  No clip case!
	//***********************************************************
	if ( (sClipL|sClipR) != (UCHAR)0)	// FULL clip case!
		{
		TRACE("BLiT: FSPR1=>BMP8, clipping NYI!\n");
		}
	else if ( (sClipT|sClipB) != (UCHAR)0) // VCLIP case!
		{
		TRACE("BLiT: FSPR1=>BMP8, clipping NYI!\n");
		/*
		while (TRUE)
			{
			if ((*pCode) == FF) // vertical run
				{	// end of sprite?
				if ( (ucCount = *(++pCode)) == FF) break; 
				pDstLine += lDstP * ucCount;
				pCode++; // open stack
				continue; // next line
				}
			pDst = pDstLine;
			while ( (ucCount = *(pCode++)) != FF) // EOL
				{
				pDst += ucCount;
				ucCount = *(pCode++);
				while (ucCount--) *(pDst++) = ucForeColor;
				}
			pDstLine += lDstP;
			}
		*/
		}
	else // NO CLIP CASE!
		{
		while (TRUE)
			{
			if ((*pCode) == FF) // vertical run
				{	// end of sprite?
				if ( (ucCount = *(++pCode)) == FF) break; 
				pDstLine += lDstP * ucCount;
				pCode++; // open stack
				continue; // next line
				}
			pDst = pDstLine;
			while ( (ucCount = *(pCode++)) != FF) // EOL
				{
				pDst += ucCount;
				ucCount = *(pCode++);
				while (ucCount--) *(pDst++) = ucForeColor;
				}
			pDstLine += lDstP;
			}
		}


	//***********************************************************
	//*******************************************************************
	// IN RELEASE MODE, GIVE THE USER A CHANCE:
#ifndef _DEBUG

	//if (gsScreenLocked || gsBufferLocked) goto BLIT_DONTUNLOCK;

#endif

	//********************
	// OS_SPECIFIC:
	//********************  UNLOCK WHATEVER YOU NEED TO
	switch (sNeedToUnlock)
		{
		case 0: // nothing to unlock
		break;

		case BUF_MEMORY:
		//	rspUnlockBuffer();
		break;
		
		case BUF_VRAM:
		//	rspUnlockScreen();
		break;
		
		case BUF_VRAM2:
			rspUnlockVideoFlipPage();
		break;
		
		default:
			TRACE("BLiT:  Unlocking error!\n");
		}

//BLIT_DONTUNLOCK:	
	return 0;
	}

//************************************************************
//************************************************************
//************************************************************
//************************************************************
//***********************  SCALING!  *************************
//************************************************************
//************************************************************
//************************************************************
//************************************************************
// 8-bit color for now!

// Doesn't clip...
// The WILL draw (ulForeColor == 0) into the bitmap!
// /*if (ulBackColor != 0) it will pre-rect it in.*/
// For transition reasons, coloring the background is
// left up to a higher level.
//
short rspBlit(
				  ULONG ulForeColor, // will draw color 0
				  RImage* pimSrc,
				  RImage* pimDst,
				  short sDstX,
				  short sDstY,
				  short sDstW,
				  short sDstH,
				  const RRect* prDst
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

	if (pimDst->m_sDepth > 8)
		{
		TRACE("BLiT: TC sprites are not YET implemented for FSPR1.\n");
		return -1;
		}

	/*
	if ( (sDstH > pimSrc->lHeight) || (sDstW > pimSrc->lWidth))
		{
		//TRACE("BLiT: Magnifying an FSPR1 NYI.\n");
		return -1;
		}

	if ( (sDstW < 1) || (sDstH < 1) )
		{
		//TRACE("BLiT: Zero or negative area passed.\n");
		return -1;
		}
	*/

#endif

	if ( (pimSrc->m_sWidth == sDstW) && (pimSrc->m_sHeight == sDstH) )
		{
		return rspBlit(ulForeColor,pimSrc,pimDst,sDstX,sDstY,prDst);
		}
	
	// transfer colors:
	UCHAR	ucForeColor = (UCHAR) ulForeColor;

	// Clip!
	short sClipL=0,sClipR=0,sClipT=0,sClipB=0;
	short sW = sDstW; // clippng parameters...
	short sH = sDstH; // clippng parameters...
	long	lDstP = pimDst->m_lPitch;

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
		sClipR = sDstX + sW - pimDst->m_sWidth; // positive = clipped
		if (sClipR > 0) sW -= sClipR; // positive = clipped
		sClipB = sDstY + sH - pimDst->m_sHeight; // positive = clipped
		if (sClipB > 0) sH -= sClipB; // positive = clipped

		if ((sW <= 0) || (sH <= 0)) return -1; // fully clipped
		}
		
	// Make positive:
	if (sClipL < 0) sClipL = 0;
	if (sClipR < 0) sClipR = 0;
	if (sClipT < 0) sClipT = 0;
	if (sClipB < 0) sClipB = 0;

	//**************  INSERT BUFFER HOOKS HERE!  ************************

	// do OS based copying!
	short sNeedToUnlock = 0; // will be the name of a buffer to unlock.
	short sBlitTypeDst = 0;

	// IN RELEASE MODE, GIVE THE USER A CHANCE:
#ifndef _DEBUG

	//if (gsScreenLocked || gsBufferLocked) goto BLIT_PRELOCKED;

#endif

	// IN THIS IMPLEMENTATION, we must do LOCK, BLiT, UNLOCK, so I
	// must record which UNLOCK (if any) needs to be done AFTER the BLiT
	// has completed. (Lord help me if a blit gets interrupted)
	if (pimDst->m_type == RImage::IMAGE_STUB) sBlitTypeDst = (short)((long)pimDst->m_pSpecial);

	switch (sBlitTypeDst) // 0 = normal image
		{
		case BUF_MEMORY: // image to system buffer
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

		case BUF_VRAM: // image to front screen
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
		break;
		*/

		case BUF_VRAM2: // image to back screen
			// need to lock / unlock this one:
			if (rspLockVideoFlipPage((void**)&(pimDst->m_pData),&(pimDst->m_lPitch))
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
//BLIT_PRELOCKED:

	// Check for locking error:
	if (!pimDst->m_pData)
		{
		TRACE("BLiT: NULL data - possible locking error.\n");
		return FAILURE;
		}

	UCHAR	*pDst,*pDstLine,*pCode,ucCount;
	pDstLine = pimDst->m_pData + lDstP * sDstY + sDstX;
	RSpecialFSPR1*	pHead = (RSpecialFSPR1*)(pimSrc->m_pSpecial);
	pCode = pHead->m_pCode;
	const UCHAR FF = (UCHAR)255;

	// Let's scale it, baby! (pre-clipping)
	short sDenX = pimSrc->m_sWidth; 
	short sDenY = pimSrc->m_sHeight; 
	RFracU16 frX = {0};
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

	//***********************************************************
	//*****************  AT LAST!   CODE!  **********************
	//***********************************************************
	// 
	short sCount; // to allow horizontal magnification > 255...

	//***********************************************************
	//*********  No clip case!
	//***********************************************************
	if ( (sClipL|sClipR) != (UCHAR)0)	// FULL clip case!
		{
		TRACE("BLiT: FSPR1=>BMP8 + SCALE, clipping NYI!\n");
		}
	else if ( (sClipT|sClipB) != (UCHAR)0) // VCLIP case!
		{
		TRACE("BLiT: FSPR1=>BMP8 + SCALE, clipping NYI!\n");
		}
	else // NO CLIP CASE!
		{
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
				frX.set = 0; // start of line!
				while ( (ucCount = *(pCode++)) != FF) // EOL
					{
					frOldX = frX;
					rspfrAdd(frX,afrSkipX[ucCount],sDenX);
					pDst += (frX.mod - frOldX.mod); // skip
					ucCount = *(pCode++);
					frOldX = frX;
					rspfrAdd(frX,afrSkipX[ucCount],sDenX);
					// To allow magnification beyond 255:
					sCount = frX.mod - frOldX.mod;
					// Modify this to a rect for solid VMagnification.
					while (sCount--) *(pDst++) = ucForeColor;
					}
				rspfrAdd(frY,afrSkipY[1],sDenY);
				pDstLine += alDstSkip[frY.mod - frOldY.mod];
				}
			}
		}

	free(alDstSkip);
	free(afrSkipX);
	free(afrSkipY);

	//***********************************************************
	//*******************************************************************
	// IN RELEASE MODE, GIVE THE USER A CHANCE:
#ifndef _DEBUG

	//if (gsScreenLocked || gsBufferLocked) goto BLIT_DONTUNLOCK;

#endif

	//********************
	// OS_SPECIFIC:
	//********************  UNLOCK WHATEVER YOU NEED TO
	switch (sNeedToUnlock)
		{
		case 0: // nothing to unlock
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
			TRACE("BLiT:  Unlocking error!\n");
		}

//BLIT_DONTUNLOCK:	
	return 0;
	}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//XXXXXXXXXXXX     Text Effects (no clipping!)    XXXXXXXXXXXX
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

//************************************************************
//*********************  NON-SCALING!  ***********************
//************************************************************
// 8-bit color for now!

// Doesn't clip...
// This WILL draw (ulForeColor == 0) into the bitmap!
// /* if (ulBackColor != 0) it will pre-rect it in. */
// Let a higher level function do tha background rect!
// Make s for an easier transition!
//
short rspBlit(
				  ULONG ulForeColor,
				  RImage* pimSrc,
				  RImage* pimDst,
				  short sDstX,
				  short sDstY,
				  short* psLineOffset // Must be as long as the height!
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

	if (pimDst->m_sDepth > 8)
		{
		TRACE("BLiT: TC sprites are not YET implemented for FSPR1.\n");
		return -1;
		}

#endif
	
	// transfer colors:
	UCHAR	ucForeColor = (UCHAR) ulForeColor;
	long	lDstP = pimDst->m_lPitch;

	//**************  INSERT BUFFER HOOKS HERE!  ************************

	// do OS based copying!
	short sNeedToUnlock = 0; // will be the name of a buffer to unlock.
	short sBlitTypeDst = 0;

	// IN RELEASE MODE, GIVE THE USER A CHANCE:
#ifndef _DEBUG

	//if (gsScreenLocked || gsBufferLocked) goto BLIT_PRELOCKED;

#endif

	// IN THIS IMPLEMENTATION, we must do LOCK, BLiT, UNLOCK, so I
	// must record which UNLOCK (if any) needs to be done AFTER the BLiT
	// has completed. (Lord help me if a blit gets interrupted)
	if (pimDst->m_type == RImage::IMAGE_STUB) sBlitTypeDst = (short)((long)pimDst->m_pSpecial);

	switch (sBlitTypeDst) // 0 = normal image
		{
		case BUF_MEMORY: // image to system buffer
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

		case BUF_VRAM: // image to front screen
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

		case BUF_VRAM2: // image to back screen
			// need to lock / unlock this one:
			if (rspLockVideoFlipPage((void**)&(pimDst->m_pData),&(pimDst->m_lPitch))
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
//BLIT_PRELOCKED:

	// check for locking error:
	if (!pimDst->m_pData)
		{
		TRACE("Blit: NULL data - possible locking error.\n");
		return FAILURE;
		}

	UCHAR	*pDst,*pDstLine,*pCode,ucCount;
	pDstLine = pimDst->m_pData + lDstP * sDstY + sDstX;
	RSpecialFSPR1*	pHead = (RSpecialFSPR1*)(pimSrc->m_pSpecialMem);
	pCode = pHead->m_pCode;
	const UCHAR FF = (UCHAR)255;


	//***********************************************************
	//*****************  AT LAST!   CODE!  **********************
	//***********************************************************
	//
	
	//***********************************************************
	//*********  No clip case!
	//***********************************************************

	// Add Italics ability:
	short* psOffX = psLineOffset;

	while (TRUE)
		{
		if ((*pCode) == FF) // vertical run
			{	// end of sprite?
			if ( (ucCount = *(++pCode)) == FF) break; 
			pDstLine += lDstP * ucCount;
			psOffX += ucCount;		// Italics
			pCode++; // open stack
			continue; // next line
			}

		pDst = pDstLine + *psOffX; // Italics

		while ( (ucCount = *(pCode++)) != FF) // EOL
			{
			pDst += ucCount;
			ucCount = *(pCode++);
			while (ucCount--) *(pDst++) = ucForeColor;
			}
		pDstLine += lDstP;
		psOffX++;
		}


	//***********************************************************
	//*******************************************************************
	// IN RELEASE MODE, GIVE THE USER A CHANCE:
#ifndef _DEBUG

	//if (gsScreenLocked || gsBufferLocked) goto BLIT_DONTUNLOCK;

#endif

	//********************
	// OS_SPECIFIC:
	//********************  UNLOCK WHATEVER YOU NEED TO
	switch (sNeedToUnlock)
		{
		case 0: // nothing to unlock
		break;

		case BUF_MEMORY:
		//	rspUnlockBuffer();
		break;
		
		case BUF_VRAM:
		//	rspUnlockScreen();
		break;
		
		case BUF_VRAM2:
			rspUnlockVideoFlipPage();
		break;
		
		default:
			TRACE("BLiT:  Unlocking error!\n");
		}

//BLIT_DONTUNLOCK:	
	return 0;
	}

//************************************************************
//************************************************************
//************************************************************
//************************************************************
//***********************  SCALING!  *************************
//************************************************************
//************************************************************
//************************************************************
//************************************************************
// 8-bit color for now!

// Doesn't clip...
// The WILL draw (ulForeColor == 0) into the bitmap!
// /*if (ulBackColor != 0) it will pre-rect it in.*/
// For transition reasons, coloring the background is
// left up to a higher level.
//
short rspBlit(
				  ULONG ulForeColor, // will draw color 0
				  RImage* pimSrc,
				  RImage* pimDst,
				  short sDstX,
				  short sDstY,
				  short sDstW,
				  short sDstH,
				  short* psLineOffset // Must be as long as the height!
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

	if (pimDst->m_sDepth > 8)
		{
		TRACE("BLiT: TC sprites are not YET implemented for FSPR1.\n");
		return -1;
		}

#endif

	if ( (sDstW < 1) || (sDstH < 1) )
		{
		//TRACE("BLiT: Zero or negative area passed.\n");
		return -1;
		}

	// transfer colors:
	UCHAR	ucForeColor = (UCHAR) ulForeColor;

	short sW = sDstW; // clippng parameters...
	short sH = sDstH; // clippng parameters...
	long	lDstP = pimDst->m_lPitch;

	//**************  INSERT BUFFER HOOKS HERE!  ************************

	// do OS based copying!
	short sNeedToUnlock = 0; // will be the name of a buffer to unlock.
	short sBlitTypeDst = 0;

	// IN RELEASE MODE, GIVE THE USER A CHANCE:
#ifndef _DEBUG

	//if (gsScreenLocked || gsBufferLocked) goto BLIT_PRELOCKED;

#endif

	// IN THIS IMPLEMENTATION, we must do LOCK, BLiT, UNLOCK, so I
	// must record which UNLOCK (if any) needs to be done AFTER the BLiT
	// has completed. (Lord help me if a blit gets interrupted)
	if (pimDst->m_type == RImage::IMAGE_STUB) sBlitTypeDst = (short)((long)pimDst->m_pSpecial);

	switch (sBlitTypeDst) // 0 = normal image
		{
		case BUF_MEMORY: // image to system buffer
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

		case BUF_VRAM: // image to front screen
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

		case BUF_VRAM2: // image to back screen
			// need to lock / unlock this one:
			if (rspLockVideoFlipPage((void**)&(pimDst->m_pData),&(pimDst->m_lPitch))
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
//BLIT_PRELOCKED:

	// Check for locking error:
	if (!pimDst->m_pData)
		{
		TRACE("BLiT: NULL data - possible locking error.\n");
		return FAILURE;
		}

	UCHAR	*pDst,*pDstLine,*pCode,ucCount;
	pDstLine = pimDst->m_pData + lDstP * sDstY + sDstX;
	RSpecialFSPR1*	pHead = (RSpecialFSPR1*)(pimSrc->m_pSpecial);
	pCode = pHead->m_pCode;
	const UCHAR FF = (UCHAR)255;

	// Let's scale it, baby! (pre-clipping)
	short sDenX = pimSrc->m_sWidth; 
	short sDenY = pimSrc->m_sHeight; 
	RFracU16 frX = {0};
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

	//***********************************************************
	//*****************  AT LAST!   CODE!  **********************
	//***********************************************************
	// 

	//***********************************************************
	//*********  No clip case!
	//***********************************************************

	// Add Italics ability:
	short* psOffX = psLineOffset;
	short sCount; // Allows magnified runs > 255...

	while (TRUE)
		{
		if ((*pCode) == FF) // vertical run
			{	// end of sprite?
			if ( (ucCount = *(++pCode)) == FF) break; 
			rspfrAdd(frY,afrSkipY[ucCount],sDenY);
			pDstLine += lDstP * (frY.mod - frOldY.mod);
			psOffX += (frY.mod - frOldY.mod);		// Italics
			pCode++; // open stack
			continue; // next line
			}

		if (frOldY.mod == frY.mod) // do a quick skip of a line:
			{
			while ( (*(pCode++)) != FF) ; // skip line!
			rspfrAdd(frY,afrSkipY[1],sDenY);
			pDstLine += alDstSkip[frY.mod - frOldY.mod];
			psOffX += frY.mod - frOldY.mod;
			}
		else // actually draw it!
			{
			frOldY = frY;

			pDst = pDstLine + *psOffX; // Italics

			frX.set = 0; // start of line!
			while ( (ucCount = *(pCode++)) != FF) // EOL
				{
				frOldX = frX;
				rspfrAdd(frX,afrSkipX[ucCount],sDenX);
				pDst += (frX.mod - frOldX.mod); // skip
				ucCount = *(pCode++);
				frOldX = frX;
				rspfrAdd(frX,afrSkipX[ucCount],sDenX);
				// This is to allow magnified runs > 255:
				sCount = frX.mod - frOldX.mod; 
				// Modify this to a rect for solid VMagnification.
				while (sCount--) *(pDst++) = ucForeColor;
				}
			rspfrAdd(frY,afrSkipY[1],sDenY);
			pDstLine += alDstSkip[frY.mod - frOldY.mod];
			psOffX += frY.mod - frOldY.mod;
			}
		}

	free(alDstSkip);
	free(afrSkipX);
	free(afrSkipY);

	//***********************************************************
	//*******************************************************************
	// IN RELEASE MODE, GIVE THE USER A CHANCE:
#ifndef _DEBUG

	//if (gsScreenLocked || gsBufferLocked) goto BLIT_DONTUNLOCK;

#endif

	//********************
	// OS_SPECIFIC:
	//********************  UNLOCK WHATEVER YOU NEED TO
	switch (sNeedToUnlock)
		{
		case 0: // nothing to unlock
		break;

		case BUF_MEMORY:
		//	rspUnlockBuffer();
		break;
		
		case BUF_VRAM:
		//	rspUnlockScreen();
		break;
		
		case BUF_VRAM2:
			rspUnlockVideoFlipPage();
		break;
		
		default:
			TRACE("BLiT:  Unlocking error!\n");
		}

//BLIT_DONTUNLOCK:	
	return 0;
	}

