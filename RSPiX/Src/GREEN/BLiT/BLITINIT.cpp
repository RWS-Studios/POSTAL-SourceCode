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
// It's time for a major change in BLiT.
// Due to a revision in the blue layer and a limiting of platform scope,
// and the invention of a common image format,
// great simplifications and changes in BLiT are about to happen.

//**************************************************************************
//**************************  THE NEW BLiT
//**************************************************************************

// Blue has taken out a lot of the screen abstraction from BLiT.  It has
// provided calls to obtain the screen address on the fly.  It does
// automatic centering and clipping.  The only accepted alien screen format
// is the Dib, and this has been hidden by use of a negative pitch and
// offset buffer pointers.  128-bit alignment has been set as a standard.

// The result is that for the first time, BLiT has a change at being a 
// disembodied library with no extra user baggage.  This is consistent with
// the more "inside out" user driven call philosophy of RSPiX.

// 1) No more "screen" versus "view".  No more backwards support is provided.
// The blue layer supports one official "buffer" for the screen.  The user is
// resposible for centering stuff in a 16-pix aligned way.  This effects the
// need for clipping boxes, which are now passed to the BLiT functions unless
// defaulting to the destination buffer.  All of this should bring the BLiT
// initialization to a minimum (maybe even optional).  The BLiT macros for the
// main buffer dimensions will NOLONGER be supported!

// 2) A port to image.  Multi-color depth BLitting will be supported to any
// uncompressed image format.  Only the FSPR8 compressed format will remain
// as special.  The main BLiT objects will finally be wrapped as images.  We
// will finally make use of images.  This will effect the way alignment is
// stored and processed.
// NOTE:  24-bit color will be the last supported.  For now, 8, 16, and 32-bit
// modes WILL be supported

// NOTE:  The initial BLiT will actual be a bit slower, because it will take
// advantage of templated inline functions which VC++ will refuse to inline.
// This problem will be remedies as fast as possible to return BLiT to native
// speed.

//===========================================================================

//=========  Really, the only decision made now is whether or not the destination
//=========  is the actual screen buffer.
//=========  Current plan:  Wrap the screen buffer in an special image format that
//=========  I define and the user should never touch.

//#include "System.h"

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

class RInitBLiT
	{
	// do an initialization automatically:
public:
	RInitBLiT();
	~RInitBLiT();
	short sLastDitchEffortToTellWhatIsHappening;
	
	static	RImage*	pimScreenBuffer;
	static	RImage*  pimScreenVisible;
	static	RImage*	pimScreenBackPlane;	// Only used with double buffering
	};

extern RInitBLiT	RStartBlitting;	// only included once, of course!
extern short   ConvertFromFSPR8(RImage* pImage);
extern short   ConvertToFSPR8(RImage* pImage);
extern short   DeleteFSPR8(RImage* pImage);
extern short   ConvertFromFSPR1(RImage* pImage);
extern short   ConvertToFSPR1(RImage* pImage);
extern short   DeleteFSPR1(RImage* pImage);

void	LinkImage();
void	LinkImage()
	{
	RImage* pim = NULL;
	ConvertFromFSPR8(pim);
	ConvertToFSPR8(pim);
	DeleteFSPR8(pim);
	ConvertFromFSPR1(pim);
	ConvertToFSPR1(pim);
	DeleteFSPR1(pim);
	}

// BLiT pre-init
RInitBLiT::RInitBLiT()
	{
	pimScreenBuffer = new RImage;
	pimScreenVisible = new RImage;
	pimScreenBackPlane = new RImage;

	// trick the compiler into instantiating the BLiT Image types...
	if (((long)pimScreenBuffer + (long)pimScreenVisible + (long)pimScreenBackPlane) == 0)
		{
		LinkImage(); // NEVER Really do this!
		}
	
	// Do NOT assume the blue layer has had a chance to initialize yet!
	// In fact, we don't know a darn thing yet!
	pimScreenBuffer->CreateImage(0,0,RImage::IMAGE_STUB,0,0);	// pSpecial contains info #!
	pimScreenVisible->CreateImage(0,0,RImage::IMAGE_STUB,0,0);	
	pimScreenBackPlane->CreateImage(0,0,RImage::IMAGE_STUB,0,0);	

	pimScreenBuffer->m_pSpecial = (UCHAR*) BUF_MEMORY;
	pimScreenVisible->m_pSpecial = (UCHAR*) BUF_VRAM;
	pimScreenBackPlane->m_pSpecial = (UCHAR*) BUF_VRAM2;

#ifdef WIN32
	TRACE("BLiT has initialized\n");
#endif
	}

// Be very careful (and precise) about this stuff:
short gsScreenLocked = 0;	// begin unlocked!
short gsBufferLocked = 0;	// begin unlocked!


// BLiT Kill
RInitBLiT::~RInitBLiT()
	{
	// Do final unlocking:
	while (gsScreenLocked) rspUnlockScreen();
	while (gsBufferLocked) rspUnlockBuffer();

	delete pimScreenBuffer;
	delete pimScreenVisible;
	delete pimScreenBackPlane;
//	TRACE("BLiT has deceased\n");
	}

RImage*	RInitBLiT::pimScreenBuffer = NULL;
RImage*  RInitBLiT::pimScreenVisible = NULL;
RImage*	RInitBLiT::pimScreenBackPlane = NULL;

// this function is a way to refer to buffers in the new BLiT:
// Note that a dib buffer MUST return a negative pitch!
// 
void rspNameBuffers(RImage** ppimMemBuf,RImage** ppimVidBuf,RImage** ppimBackBuf)
	{
	// Set aliases to the buffers and get the most current values possible.
	short sVidW,sVidH,sVidD,sMemW,sMemH;
	rspGetVideoMode(&sVidD,&sVidW,&sVidH,NULL,&sMemW,&sMemH);

	if (ppimMemBuf != NULL) 
		{
		*ppimMemBuf = RInitBLiT::pimScreenBuffer;
		(*ppimMemBuf)->m_sWidth = sMemW;	// assume same  for all...
		(*ppimMemBuf)->m_sHeight = sMemH;	// assume same  for all...
		(*ppimMemBuf)->m_sDepth = sVidD;	// assume same  for all...
		// If not already locked . . .
		if (!gsBufferLocked)
			{
			void*	pvTmp;	// JMI:  Use a temp var (otherwise we end up having to 
								// clear RInitBlit::pimScreenBuffer->m_pData right after
								// this the way we used to).

			// Really all we get out of this is the pitch.
			if (rspLockVideoBuffer(&pvTmp, &((*ppimMemBuf)->m_lPitch)) == 0)
				{
				rspUnlockVideoBuffer();
				}
			}
		else
			{
			// It is already locked and, therefore, the m_pData and m_lPitch are already
			// valid.
			}
		}

	if (ppimVidBuf != NULL) 
		{
		*ppimVidBuf = RInitBLiT::pimScreenVisible;
		(*ppimVidBuf)->m_sWidth = sMemW;	//double if pixel doubled
		(*ppimVidBuf)->m_sHeight = sMemH;	// double if pixel doubled
		(*ppimVidBuf)->m_sDepth = sVidD;	// assume same  for all...
		// If not already locked . . .
		if (!gsBufferLocked)
			{
			void*	pvTmp;	// JMI:  Use a temp var (otherwise we end up having to 
								// clear RInitBlit::pimScreenVisible->m_pData right after
								// this the way we used to).

			// Really all we get out of this is the pitch.
			if (rspLockVideoPage(&pvTmp,&((*ppimVidBuf)->m_lPitch)) == 0)
				{
				rspUnlockVideoPage(); // pitch is correct (unscaled)
				}
			}
		else
			{
			// It is already locked and, therefore, the m_pData and m_lPitch are already
			// valid.
			}
		}

	if (ppimBackBuf != NULL)
		{
		*ppimBackBuf = RInitBLiT::pimScreenBackPlane;
		(*ppimBackBuf)->m_sWidth = sMemW;	// assume same  for all...
		(*ppimBackBuf)->m_sHeight = sMemH;	// assume same  for all...
		(*ppimBackBuf)->m_sDepth = sVidD;	// assume same  for all...
		// If not already locked . . .
		if (!gsBufferLocked)
			{
			void*	pvTmp;	// JMI:  Use a temp var (otherwise we end up having to 
								// clear RInitBlit::pimScreenBackPlane->m_pData right after
								// this the way we used to).

			// Really all we get out of this is the pitch.
			if (rspLockVideoFlipPage(&pvTmp, &((*ppimBackBuf)->m_lPitch)) == 0)
				{
				rspUnlockVideoFlipPage(); // pitch is correct (unscaled)
				}
			}
		else
			{
			// It is already locked and, therefore, the m_pData and m_lPitch are already
			// valid.
			}
		}

	}

RInitBLiT	RStartBlittingForFun;	// only included once, of course!

// For your convenience, this lets you emulate normal windows static colors...
//
//#include ".h"	// need the macro

short rspSetWindowColors()
	{
#ifdef _DEBUG

	TRACE("rspSetWindowColors:  Not yet implemented on this platform!\n");

#endif

	return -1;
	}


// waits for any click!
void rspWaitForClick(short sWait)
	{
	short sDummy,sB;
	if (sWait == 0) return;

	TRACE("Please click %s mouse button.\n",
		(sWait==1) ? "the left" :
		(sWait==2) ? "the right" :
		"any");

	do	{
		rspGetMouse(&sDummy,&sDummy,&sB);
		rspDoSystem();
		} while ((sB&sWait)!=0);

	do	{
		rspGetMouse(&sDummy,&sDummy,&sB);
		rspDoSystem();
		} while ((sB&sWait)==0);

	do	{
		rspGetMouse(&sDummy,&sDummy,&sB);
		rspDoSystem();
		} while ((sB&sWait)!=0);
	}

// A COPY of the class definition:
// Not a complete descriptor -> just replaces the pData of an uncompressed buffer.
class RCompressedImageData
	{
public:
	USHORT	usCompType;	// = FSPR8 image type
	USHORT	usSourceType;	// uncompressed Image pre-compressed type
	UCHAR*	pCBuf;		// Start of compressed picture data, 128-aligned, NULL for monochrome
	UCHAR*	pCMem;
	UCHAR* pControlBlock;// 32-aligned run length code for compressed BLiT
	UCHAR** pLineArry;	// 32-aligned, arry of ptrs to pCBuf scanlines, 32-bit align assumed
	UCHAR** pCtlArry;		// 32-aligned, arry of offset ptrs into CtlBlock

	RCompressedImageData()
		{
		usCompType = usSourceType = 0;
		pCBuf = pCMem = pControlBlock = NULL;
		pLineArry = pCtlArry = NULL;
		}

	~RCompressedImageData()
		{
		if (pCMem) free(pCMem);
		if (pControlBlock) free(pControlBlock);
		if (pLineArry) free(pLineArry);
		if (pCtlArry) free(pCtlArry);
		}
	}; 

// for your convenience:
void	rspSetBMPColors(RImage* pim,short sStartIndex,short sNum)
	{
#ifdef	_DEBUG

	// Figure out later how to tell if the source type was BMP8...
	// Actually, we ONLY need to worry about type FSPR8
	if (pim->m_type == RImage::FSPR8)
		{
		if ( ((RCompressedImageData*)pim->m_pSpecial)->usSourceType != (USHORT)RImage::BMP8)
			{
			TRACE("rspSetBMPColors:  Palette not type BMP8\n");
			return;
			}
		}

	else if (pim->m_type != RImage::BMP8)
		{
		TRACE("rspSetBMPColors:  Palette not type BMP8\n");
		return;
		}

#endif

	typedef	struct
		{
		UCHAR	b;
		UCHAR g;
		UCHAR r;
		UCHAR a;
		}	RGB;

	RGB*	pPal = (RGB*)pim->m_pPalette->m_pData;

	rspSetPaletteEntries(sStartIndex,sNum,&pPal[sStartIndex].r,
		&pPal[sStartIndex].g,&pPal[sStartIndex].b,sizeof(RGB));

	rspUpdatePalette();
	}

// This is the official composite buffer of the OS
short	rspLockBuffer()
	{

	if (!gsBufferLocked) // we haven't actually locked it yet!
		{
		if (rspLockVideoBuffer((void**)&(RInitBLiT::pimScreenBuffer->m_pData),
			&(RInitBLiT::pimScreenBuffer->m_lPitch))
			!=0)
			{
			TRACE("rspLockBuffer: Unable to lock the Video Buffer, failed!\n");
			return -1;
			}
		else
			{
			// Get the up to date dimensions as well!
			short sVidW,sVidH,sVidD,sMemW,sMemH;
			rspGetVideoMode(&sVidD,&sVidW,&sVidH,NULL,&sMemW,&sMemH);

			RInitBLiT::pimScreenBuffer->m_sWidth = sMemW;	// assume same  for all...
			RInitBLiT::pimScreenBuffer->m_sHeight = sMemH;	// assume same  for all...
			RInitBLiT::pimScreenBuffer->m_sDepth = sVidD;	// assume same  for all...
			}
		}

	gsBufferLocked++;
	return 0;
	}

// This is the official OS composite buffer
short	rspUnlockBuffer()
	{
	gsBufferLocked--;

#ifdef	_DEBUG

	if (gsBufferLocked < 0)
		{
		TRACE("rspUnLockBuffer: FATAL ERROR!  More screen UNLOCKS than LOCKS!\n");
		gsScreenLocked = 0;
		return -1;
		}

#endif
	
	if (!gsBufferLocked) // need to actually DO the unlock
		{
		rspUnlockVideoBuffer();
		RInitBLiT::pimScreenBuffer->m_pData = NULL;
		}


	return 0;
	}

// This is the hardware video screen
short	rspLockScreen()
	{
	if (!gsScreenLocked) // we haven't actually locked it yet!
		{
		if (rspLockVideoPage((void**)&(RInitBLiT::pimScreenVisible->m_pData),
			&(RInitBLiT::pimScreenVisible->m_lPitch))
			!=0)
			{
			TRACE("rspLockScreen: Unable to lock the OnScreen system buffer, failed!\n");
			return -1;
			}
		else
			{
			// Set aliases to the buffers and get the most current values possible.
			short sVidW,sVidH,sVidD,sMemW,sMemH;
			rspGetVideoMode(&sVidD,&sVidW,&sVidH,NULL,&sMemW,&sMemH);

			// Get up to date dimensions:
			RInitBLiT::pimScreenVisible->m_sWidth = sMemW;	//double if pixel doubled
			RInitBLiT::pimScreenVisible->m_sHeight = sMemH;	// double if pixel doubled
			RInitBLiT::pimScreenVisible->m_sDepth = sVidD;	// assume same  for all...
			}
		}

	gsScreenLocked++;
	return 0;
	}

short	rspUnlockScreen()
	{

	gsScreenLocked--;

#ifdef	_DEBUG

	if (gsScreenLocked < 0)
		{
		TRACE("rspUnLockScreen: FATAL ERROR!  More screen UNLOCKS than LOCKS!\n");
		gsScreenLocked = 0;
		return -1;
		}

#endif
	
	if (!gsScreenLocked) // need to actually DO the unlock
		{
		rspUnlockVideoPage();
		RInitBLiT::pimScreenVisible->m_pData = NULL;
		}

	return 0;
	}

/////////////////////////////////////////////////////////////////////////
//
//		General Locking - help the user know how to lock / unlock in a 
//    general case:
//
/////////////////////////////////////////////////////////////////////////

// This locks the required buffer.
// NOTE: will NOT be appropriate for an rspUpdateDisplay scenario!
short	rspGeneralLock(RImage* pimDst)
	{
	ASSERT(pimDst);

	// If it is a stub . . .
	if (pimDst->m_type == RImage::IMAGE_STUB)
		{
		switch (((short)(((long)pimDst->m_pSpecial)))) // 0 = normal image
			{
			case 0:	// it's YOUR IMAGE!
				return SUCCESS;
			break;

			case BUF_MEMORY:	// it the SYSTEM BUFFER
				return rspLockBuffer();
			break;

			case BUF_VRAM:		// it is the FRONT SCREEN PAGE
				return rspLockScreen();
			break;

			case BUF_VRAM2:	// it si the seconds, hidden VRAM page
				return rspLockVideoFlipPage((void**)&(pimDst->m_pData),&(pimDst->m_lPitch));
			break;

			default:
				TRACE("rspGeneralLock: unrecognized image case - probably FSPR.\n");
			}
		}

	return SUCCESS;
	}

// This locks the required buffer.
// NOTE: will NOT be appropriate for an rspUpdateDisplay scenario!
short	rspGeneralUnlock(RImage* pimDst)
	{
	ASSERT(pimDst);

	// If it is a stub . . .
	if (pimDst->m_type == RImage::IMAGE_STUB)
		{
		switch (((short)(((long)pimDst->m_pSpecial)))) // 0 = normal image
			{
			case 0:	// it's YOUR IMAGE!
				return SUCCESS;
			break;

			case BUF_MEMORY:	// it the SYSTEM BUFFER
				return rspUnlockBuffer();
			break;

			case BUF_VRAM:		// it is the FRONT SCREEN PAGE
				return rspUnlockScreen();
			break;

			case BUF_VRAM2:	// it si the seconds, hidden VRAM page
				rspUnlockVideoFlipPage();
				return SUCCESS;
			break;

			default:
				TRACE("rspGeneralLock: unrecognized image case - probably FSPR.\n");
			}
		}

	return SUCCESS;
	}

