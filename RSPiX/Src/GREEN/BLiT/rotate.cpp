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
I AM ARCHAIC !  REMOVE ME AT ONCE AND ADD ROTATE96.CPP!

// This should include RotShrink, RotOrtho, and R90
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
#include <math.h>

// Turn off warnings for assignments within expressions
#ifdef __MWERKS__
	#pragma warn_possunwant off
#endif


class RSaveOld
	{
public:
	ULONG m_type;
	short sX;
	short sY;
	short sW;
	short sH;
	};

// I hate doing this, but let's make a type for a square rotation buffer.
// It can be used by both algorithms (QuickRot and OrthoRot)
// Use of live rotation saves enough memory to kind-of justify the buffer
// space lost....
//
// Will convert from any 8bpp format.
//
short   ConvertToROTBUF(RImage* pImage);

// Will convert back to type BMP8
//
short   ConvertFromROTBUF(RImage* pImage);

// Will delete pSpecial
//
short		DeleteROTBUF(RImage* pImage);

const short	csFlag = -32767;
short gsCX = csFlag,gsCY = csFlag; // Will be reset each time to csFlag! (A default code)

void	rspSetConvertToROTBUF(short sCenterX,short sCenterY)
	{
	gsCX = sCenterX;
	gsCY = sCenterY;
	}

// shoudln't need a special delete!
IMAGELINKLATE(ROTBUF,ConvertToROTBUF,ConvertFromROTBUF,  NULL,NULL,NULL,DeleteROTBUF);

// Use for cool things!
static const double sqrt2 =  (double)1.414213562373;


// Must be an UNCOMPRESSED Image type!
// It does NOT currently Lasso the image!  You must do it fist if you care!
// It DOES use (cx,cy) as sey by rspSetConvertToROTBUF
//
short   ConvertToROTBUF(RImage* pImage)
	{
	if (!ImageIsUncompressed(pImage->m_type))
		{
		TRACE("Convert: Can only make a ROTBUF type from an UNCOMPRESSED typer!\n");
		return RImage::NOT_SUPPORTED;
		}

	// The key to the ROTBUF is to provide an amount of padding around the buffer
	// sufficient to deal with rotation:

	// in standard way, I wil create a dummy image to BLiT to, then swap buffers and
	// destroy it!

	// The theme for this type of extended buffer is as follows:
	// 
	//	long		lBufferWidth; // Width of ROTATING buffer = 2 * sC!!!!!
	// sC = sqrt2 * R, sP = sL = sqrt2 * sR
	//	long		lBufferHeight;// Same number as above
	//	long		lXPos;		  // Position of image in the buffer
	//	long		lYPos;		  // Position of image in the buffer
	// void*		pSpecial;		// This holds the previous data type!
	RImage	imTemp; // this will hold the new image!

	// Get the coordinates of the center of rotation!
	if (gsCX == csFlag) gsCX = (pImage->m_sWidth >> 1);
	if (gsCY == csFlag) gsCY = (pImage->m_sHeight >> 1);

	// reset the input parameters to default:
	long cx = (long)gsCX;
	long cy = (long)gsCY;

	//****************** APPROXIMATE THE RADIUS *********************
	// Use the Sprite's dimensions as a simple way to calculate 
	// it's radius about the cor...
	// NOTE: sqruared values of shorts must be longs!
	long	d1 = (SQR(cx)+SQR(cy)); // Distance from (0,0)
	long	d2 = (SQR(cx)+SQR(pImage->m_sHeight - cy - 1));
	long	d3 = (SQR(pImage->m_sWidth - cx - 1)+SQR(cy));
	long	d4 = (SQR(pImage->m_sWidth - cx - 1)+SQR(pImage->m_sHeight - cy - 1));

	long lR = MAX(MAX(d1,d2),MAX(d3,d4)); // distance squared
	lR = (long)(sqrt(double(lR)) + 0.9999); // actual longest distance, rounded up
	long	lC = (long)(sqrt2 * lR + 1);// round up
	long	lNewW = (long)(lC<<1); // new buffer width a test:

	// Store the rotating window size in this field:
	pImage->m_sWinWidth = pImage->m_sWinHeight = (lR<<1);

	// Find a new pitch
	long	lNewP = (lNewW + 15) & ~15;

	// Create a new buffer:
	imTemp.CreateImage((short)lNewW,(short)lNewW,pImage->m_type,lNewP,
		pImage->m_sDepth);

	// Find the offset:
	pImage->m_sWinX = lC - cx;
	pImage->m_sWinY = lC - cy;

	// BLiT the old image buffer into the new!
	rspBlit(pImage,&imTemp,(short)0,(short)0,
		pImage->m_sWinX,pImage->m_sWinY,
		pImage->m_sWidth,pImage->m_sHeight);
	
	// Now transfer it ALL back to the original buffer!
	UCHAR	*pBuf = NULL,*pMem = NULL;
	imTemp.DetachData((void**)&pMem,(void**)&pBuf);
	// Kill Old:
	pImage->DestroyData();
	// Attach New
	pImage->m_pMem = pMem;
	pImage->m_pData = pBuf;

	// Validate the new Image, and save the old parameters:
	RSaveOld*	pSave = new RSaveOld;
	pSave->m_type = pImage->m_type;
	pSave->sW = pImage->m_sWidth;
	pSave->sH = pImage->m_sHeight;
	pSave->sX = pImage->m_sWinX;
	pSave->sY = pImage->m_sWinY;
	pImage->m_pSpecialMem = pImage->m_pSpecial = (UCHAR*) pSave;

	pImage->m_type = RImage::ROTBUF;
	pImage->m_sWidth = imTemp.m_sWidth;
	pImage->m_sHeight = imTemp.m_sHeight;
	pImage->m_lPitch = imTemp.m_lPitch;
	pImage->m_ulSize = imTemp.m_ulSize;

	// Destroy Entire temporary Image
	// imTemp is on the stack and will auto-delete
	gsCY = gsCX = csFlag;
	return RImage::ROTBUF;
	}

// Will convert back to the original type
//
short   ConvertFromROTBUF(RImage* pImage)
	{
	RSaveOld* pSave = (RSaveOld*)(pImage->m_pSpecial);

	// Get rid of the extra padding:
	rspCrop(pImage,pSave->sX,pSave->sY,pSave->sW,pSave->sH);

	// Restore the member state:
	pImage->m_type = (RImage::Type)pSave->m_type;
	delete pSave;
	pImage->m_pSpecialMem = pImage->m_pSpecial = NULL;
	pImage->m_sWinWidth = pImage->m_sWinHeight = 0;

	pImage->m_sWinX = pImage->m_sWinY = 0;
	
	return pImage->m_type;
	}

// Will delete pSpecial
//
short		DeleteROTBUF(RImage* pImage)
	{
	delete pImage->m_pSpecial;
	return 0;
	}

//************* This will be incorporated into REAL commands later:

typedef	struct	tagPt	{short	x; short	y;} Pt;

// a 2D line based on 2D coordinates
typedef	struct	tagLine1
	{
	long	lNumX;  	// Fractional accumulator
	long	lNumY;
	long	lIncX;  	// Fractional Increment
	long	lIncY;
	long	lDelX;  	// Default Integral Increment for slopes > 1 as 
	long	lDelY;  	// dictated by the Dst W & H....
	long	lDen;
	long	lCurX; 	// Current point position
	long	lCurY;
	long	lPitchX;	// Conditional adding to current position
	long	lPitchY;	
	}	Line1;

// A 2D line based on a single memory pointer...
typedef	struct	tagLine2
	{
	long	lNumX;  	// Fractional accumulator
	long	lNumY;
	long	lIncX;  	// Fractional Increment
	long	lIncY;
	long	lDel; 	// Default Integral Increment for slopes > 1 as 
	long	lDen;
	UCHAR*	pCur; // Current point position
	long	lPitchX;	// Conditional adding to current position
	long	lPitchY;	
	}	Line2;

// Initialize the integral calculus
// Overload the init function for each type of line:

// d = the number of steps from pt 1 to pt 2...
inline void initLine(Line1*	pLine,
							long	x1,long y1,long x2,long y2,long d)
	{
	long	lDelX = x2 - x1,lDelY = y2 - y1;

	pLine->lCurX = x1;
	pLine->lCurY = y1;
	pLine->lDen = d;
	pLine->lNumX = pLine->lNumY = d>>1;
	pLine->lPitchX = SGN(lDelX);
	pLine->lPitchY = SGN(lDelY); // could add pitch
	pLine->lDelX = lDelX / d;	// floor value of default pitch...
	pLine->lDelY = lDelY / d; // could add pitch
	lDelX = ABS(lDelX);
	lDelY = ABS(lDelY);
	pLine->lIncX = lDelX % d;
	pLine->lIncY = lDelY % d;
	}

// d = the number of steps from pt 1 to pt 2...
inline void initLine(Line2*	pLine,UCHAR*	pBase,long	lPitch,
							long	x1,long y1,long x2,long y2,long d)
	{
	long	lDelX = x2 - x1,lDelY = y2 - y1;

	pLine->pCur = pBase + x1 + y1 * lPitch;
	pLine->lDen = d;
	pLine->lNumX = pLine->lNumY = (long)d>>1;
	pLine->lPitchX = SGN(lDelX);
	pLine->lPitchY = SGN(lDelY)*lPitch; 
	pLine->lDel = (lDelX / d) +  (lDelY / d)*lPitch;
	lDelX = ABS(lDelX);
	lDelY = ABS(lDelY);
	pLine->lIncX = lDelX % d;
	pLine->lIncY = lDelY % d;
	}

// Increments along the line
// Overload the init function for each type of line:
inline	void	incLine(Line1& Line)
	{
	Line.lCurX += Line.lDelX;
	Line.lCurY += Line.lDelY;
	Line.lNumX += Line.lIncX;
	Line.lNumY += Line.lIncY;
	if (Line.lNumX > Line.lDen) {Line.lNumX-=Line.lDen;Line.lCurX+=Line.lPitchX;}
	if (Line.lNumY > Line.lDen) {Line.lNumY-=Line.lDen;Line.lCurY+=Line.lPitchY;}
	}

inline	void	incLine(Line2& Line)
	{
	Line.pCur += Line.lDel;
	Line.lNumX += Line.lIncX;
	Line.lNumY += Line.lIncY;
	if (Line.lNumX > Line.lDen) {Line.lNumX-=Line.lDen;Line.pCur+=Line.lPitchX;}
	if (Line.lNumY > Line.lDen) {Line.lNumY-=Line.lDen;Line.pCur+=Line.lPitchY;}
	}

// FLIPPING FLAGS:
#define	  PRE_FLIP_H 		1
#define	  PRE_FLIP_V		2
#define	  POST_FLIP_H		4
#define	  POST_FLIP_V		8


#define	PtSwap(a,b) temp=a;a=b;b=temp

//
// It currently just clips to the destination buffer:
//
void	_RotateShrink(float fDeg,RImage* pimSrc,RImage* pimDst,
						  short sDstX,short sDstY,short sDstW,short sDstH, // = 2R = lBufferWidth
						  short sFlipCode)
	{
#ifdef _DEBUG

	if (pimSrc->m_type != RImage::ROTBUF)
		{
		TRACE("_RotateShrink: Source must be type ROTBUF!\n");
		return;
		}

	if (!ImageIsUncompressed(pimDst->m_type))
		{
		TRACE("_RotateShrink: Dest must be uncompressed!\n");
		return;
		}

#endif
	//****************** Destination Clipper ***********************

	short	sClip=0,sClipL=0,sClipR=0,sClipT=0,sClipB=0;
	short gsClipX,gsClipY,gsClipW,gsClipH;

	// Init clip rect:
	gsClipX = 0;   // Cheezy for now...      
	gsClipY = 0;         
	gsClipW = pimDst->m_sWidth;
	gsClipH = pimDst->m_sHeight;

	// Optimize for noclip case:
	if ((sClip = gsClipX - sDstX)>0) sClipL = sClip;
	if ((sClip = gsClipY - sDstY)>0) sClipT = sClip;
	if ((sClip = sDstX + sDstW - gsClipX - gsClipW)>0) sClipR = sClip;

	if ((sClip = sDstY + sDstH - gsClipY - gsClipH)>0) sClipB = sClip;

	if (!sClipL && !sClipR && !sClipT && !sClipB) // not clipped
		{
		// ONLY implement unclipped BLiT for now!
		static	double	PI = 4.0 * atan(1.0);
		double	dRad = (double)fDeg * PI/(double)180.0;
		double	dCos = cos(dRad);
		double	dSin = sin(dRad);
		double 	x,y;
		short	i,j;
		Pt	pCorners[4];
		Line1	l1Left,l1Right;
		Line2	l2Across; // need 3 lines...
		long	lP = pimDst->m_lPitch;
		UCHAR	*pDst,*pDstLine;
		Pt temp;
		UCHAR	pixel;	// ERROR 8-bit color!

		pDst = pimDst->m_pData + (long)sDstX + ((long)(sDstY))*lP;

		pDstLine = pDst;

		short	sR = (short)((pimSrc->m_sWinWidth>>1)); // rotating box half side...
		short	sC = (short)(sqrt2 * sR); // buffer half side...

		x = y = (double)-sR;
		//sDstW /= sqrt2;// crude patch
		//sDstH /= sqrt2;

		// Rotate Master Corner...
		pCorners[0].x = x * dCos + y * dSin;
		pCorners[0].y = y * dCos - x * dSin;

		// R90 to other corners, add flipping:

		pCorners[1].x =  +pCorners[0].y;
		pCorners[1].y =  -pCorners[0].x;
		pCorners[2].x =  -pCorners[0].y;
		pCorners[2].y =  +pCorners[0].x;
		pCorners[3].x =  -pCorners[0].x;
		pCorners[3].y =  -pCorners[0].y;

		
		// Do the simple flips about the origin:
		if (sFlipCode)
			{
			if (sFlipCode & PRE_FLIP_H) // HFLIP
				for (i=0;i<4;i++)	pCorners[i].x = -pCorners[i].x;

			if (sFlipCode & PRE_FLIP_V) // VFLIP
				for (i=0;i<4;i++)	pCorners[i].y = -pCorners[i].y;

			if (sFlipCode & POST_FLIP_H) // Dst Flip Horiz:
				{
				PtSwap(pCorners[0],pCorners[2]);
				PtSwap(pCorners[1],pCorners[3]);
				}

			if (sFlipCode & POST_FLIP_V) // Dst Flip Horiz:
				{
				PtSwap(pCorners[0],pCorners[1]);
				PtSwap(pCorners[2],pCorners[3]);
				}
			}
		
		for (i=0;i<4;i++)
			{
			pCorners[i].x += sC; // sC should BE buffersize/2
			pCorners[i].y += sC;
			}

		/*
		for (i=0;i<4;i++)
			{
			pCorners[i].x += pimSrc->lWidth / 2;
			pCorners[i].y += pimSrc->lHeight /2;
			}
		*/

		// Set up the struts of the ladder:
		initLine(&l1Left,pCorners[0].x,pCorners[0].y,
				pCorners[1].x,pCorners[1].y,sDstH); // length of side = 2R

		initLine(&l1Right,pCorners[2].x,pCorners[2].y,
				pCorners[3].x,pCorners[3].y,sDstH);

		initLine(&l2Across,pimSrc->m_pData,pimSrc->m_lPitch,pCorners[0].x,pCorners[0].y,
				pCorners[2].x,pCorners[2].y,sDstW);

		// DRAW THE SUCKER!!!! (YAYA!)
		for (j=0;j<sDstH;j++,pDst = (pDstLine += lP) )	// line by line:
			{
			for (i=0,pDst = pDstLine;i<sDstW;i++)
				{
				// leap across:
				if (pixel = *l2Across.pCur) // 0x006fa06c 
					*pDst = pixel;
				pDst++;
				incLine(l2Across);
				}
			// Advance the rungs!
			incLine(l1Left);
			incLine(l1Right);

			// Initialize a new rung across...
			initLine(&l2Across,pimSrc->m_pData,(short)pimSrc->m_lPitch,
					l1Left.lCurX,l1Left.lCurY,
					l1Right.lCurX,l1Right.lCurY,sDstW);
			}	
		return;
		}

	// Check for no draw case:
	if ( sClipL >= sDstW ) return;
	if ( sClipR >= sDstW ) return;
	if ( sClipT >= sDstH ) return;
	if ( sClipB >= sDstH ) return;

	//******************************************************************
	// Partially clipped:
	TRACE("Rotate BLit:  Clipping BLiT NYI!\n");
	//BLT_ProtoRotAndClip(pfrSrc,sDeg,pbkdDst,sDstX,sDstY,sDstW,sDstH,
	//			sClipL,sClipR,sClipT,sClipB,sFlipCode,sAF);
	}



//*****************************************************************************
//*****************************************************************************
//*****************************************************************************

// For now, let's do a crude stub for BLT_Strafe:
// It is only intended to allow CMA to compile for now, 
// and it's functionality WILL change!
//
// The goal is to create a series of pointers and links 
// which are merely copies of the original frame
// The future strafe will NOT use a special structure type,
// and the old strafe will probably be dropped!
//
/*
CStrafe*	BLT_RotStrafe(CImage* pimSrc,short sHotX,short sHotY,short sNumInc,
						  short sDstH,short sNumLinks,short *psX, short* psY)
	{
	/*
#ifdef	_DEBUG
	if (!pimSrc) {TRACE("BLT_STRAFE: null src image!\n"); return NULL;}
	if (sNumInc < 1)  {TRACE("BLT_STRAFE: no frames specified!\n"); return NULL;}
	if (sNumLinks && (!psX || !psY))  
		{TRACE("BLT_STRAFE: null links passed!\n"); return NULL;}
	if (sDstH > (short)pimSrc->lHeight) 
		{TRACE("BLT_STRAFE: magnification not currently supported!\n"); return NULL;}
#endif
	
	Strafe* pStrafe = (Strafe*) calloc(1,sizeof(Strafe));
	pStrafe->sNumFrames = sNumInc;
	pStrafe->sNumLinks = sNumLinks;
	pStrafe->pFrame = (StrafeFrame*) calloc(sNumInc,sizeof(StrafeFrame) );

	short i,j;

	for (i=0;i<sNumLinks;i++)
		{
		psX[i] -= sHotX;	// get position relative to hot spot:
		psY[i] -= sHotY;
		}

	for (i=0;i<sNumInc;i++)
		{
		pStrafe->pFrame[i].pfspr = pimSrc; // pt to original
		pStrafe->pFrame[i].sHotX = 0;
		pStrafe->pFrame[i].sHotY = 0;
		pStrafe->pFrame[i].sCurDeg = 0;
		if (sNumLinks == 0) pStrafe->pFrame[i].psLinkX = pStrafe->pFrame[i].psLinkY = NULL;
		else // create space for links:
			{
			pStrafe->pFrame[i].psLinkX = (short*)calloc(sNumLinks,sizeof(short));
			pStrafe->pFrame[i].psLinkY = (short*)calloc(sNumLinks,sizeof(short));
			// Set all the links:
			for (j=0;j<sNumLinks;j++)
				{
				pStrafe->pFrame[i].psLinkX[j] = psX[j]; // same links!
				pStrafe->pFrame[i].psLinkY[j] = psX[j];
				}
			}
		}

	return pStrafe;
	}
*/

/*
void	BLT_FreeStrafe(Strafe* pStrafe)
	{
	if (pStrafe == NULL) return;
	short i;

	for (i=0;i<pStrafe->sNumFrames;i++)
		{
		if (pStrafe->pFrame != NULL)
			{
			if (pStrafe->pFrame[i].psLinkX != NULL)
				free(pStrafe->pFrame[i].psLinkX);

			if (pStrafe->pFrame[i].psLinkY != NULL)
				free(pStrafe->pFrame[i].psLinkY);
			}
		}
	}
*/

// In this version, You must supply the host structure, which you may define, but which 
// must contain the following:  
//
short rspStrafeRotate(void *pReturnArray,	// Output
							RImage* pimSrc,short sCenterX,short sCenterY,double dScale, // Input
							 short sNumFrames,double dStartDeg,double dDegInc,
							 short sNumLinks,short *psX,short *psY, // input
							 // generic user stucture must be an array:
							 RImage* pIm, short *psHotX, short *psHotY,
							 short **ppsX,short **ppsY,
							 long lStructSize)
	{

#ifdef _DEBUG

	if (!pimSrc)
		{
		TRACE("rspStrafeRotate: NULL source passed!\n");
		return -1;
		}

	if (!pReturnArray)
		{
		TRACE("rspStrafeRotate: NULL receiver passed!\n");
		return -1;
		}

	if (sNumFrames < 1)
		{
		TRACE("rspStrafeRotate: Bad number of frames!\n");
		return -1;
		}

	if (!ImageIsUncompressed(pimSrc->m_type))
		{
		TRACE("rspStrafeRotate:Need an uncompressed image format!\n");
		return -1;
		}

	if ((dScale <= 0.0) || (dScale > 1.0))
		{
		TRACE("rspStrafeRotate: Scale error: can only reduce for now!\n");
		return -1;
		}

#endif
	//* FOR TESTING ONLY:
	RImage* pimBuf,*pimScreen;
	rspNameBuffers(&pimBuf,&pimScreen);

	union { short *pL; UCHAR *pB; } pHotX,pHotY;
	union { short **ppL; UCHAR *pB; } ppLinkX,ppLinkY;
	union { RImage **ppI; UCHAR *pB; } ppBuf;

	// default to a CStrafe:
	//
	if (!pIm) ppBuf.ppI = &(((CStrafe*)pReturnArray)->pImage);
	if (!psHotX) pHotX.pL = &((CStrafe*)pReturnArray)->sHotX;
	if (!psHotY) pHotY.pL = &((CStrafe*)pReturnArray)->sHotY;
	if (!ppsX) ppLinkX.ppL = &((CStrafe*)pReturnArray)->psLinkX;
	if (!ppsY) ppLinkY.ppL = &((CStrafe*)pReturnArray)->psLinkY;

	double dCurDeg = dStartDeg;
	// calculate degree increment in default case:
	if (dDegInc == 0.0) dDegInc = 360.0 / (double)sNumFrames;
	// Phase one:  make the source ROTBUF, and create a destination
	
	rspSetConvertToROTBUF(sCenterX,sCenterY);

	ULONG ulOldType  = pimSrc->m_type;
	if (pimSrc->Convert(RImage::ROTBUF)!= RImage::ROTBUF)
		{
		TRACE("rspStrafeRotate: Internal Conversion error\n");
		return -1;
		}

	//Calculate the appropriate height:
	short sDstH = (short)(dScale * pimSrc->m_sWinHeight);

	// Make a copy of the input links so they can be center adjusted
	short *psLinkX = NULL, *psLinkY = NULL;
	short j;

	if (sNumLinks > 0)
		{
		psLinkX = (short*)calloc(sizeof(short),sNumLinks);
		psLinkY = (short*)calloc(sizeof(short),sNumLinks);

		for (j=0;j<sNumLinks;j++)
			{
			psLinkX[j] = psX[j] - sCenterX;
			psLinkY[j] = psY[j] - sCenterY;
			}
		}

	// DO the strafing:
	short i;
	double	scale = (double)sDstH / (double) pimSrc->m_sWinHeight;

	for (i=0;i<sNumFrames;i++,dCurDeg += dDegInc)
		{	
		// Make a large enough vessel to rotate in:
		*(ppBuf.ppI) = new RImage;
		if ((*(ppBuf.ppI))->CreateImage(sDstH,sDstH,RImage::BMP8)!= SUCCESS)
			{
			TRACE("rspStrafeRotate: Out of memory. Sorry.\n");
			return -1;
			}

		// Do the BLiT:
		_RotateShrink(dCurDeg,pimSrc,(*ppBuf.ppI),0,0,sDstH,sDstH);
		
		// Get the coordinates:
		short sX=0,sY=0,sW=(short)(*(ppBuf.ppI))->m_sWidth,sH = (short)(*(ppBuf.ppI))->m_sHeight;
		rspLasso((UCHAR)0,(*(ppBuf.ppI)),sX,sY,sW,sH);

		rspCrop((*(ppBuf.ppI)),sX,sY,sW,sH);

		// Store the hot offset based on the center of the RotBuf:
		// Subtract this from your position..
		//
		*(pHotX.pL) = (sDstH>>1)-sX;
		*(pHotY.pL) = (sDstH>>1)-sY;

		// Dpo the links, if any:
		*(ppLinkX.ppL) = NULL;
		*(ppLinkY.ppL) = NULL;

		if (sNumLinks > 0)
			{
			*(ppLinkX.ppL) = (short*)calloc(sizeof(short),sNumLinks);
			*(ppLinkY.ppL) = (short*)calloc(sizeof(short),sNumLinks);

			double dRad = dCurDeg *  0.01745329251994;
			double dSin = sin(dRad)*scale;
			double dCos = cos(dRad)*scale;

			for (j=0;j<sNumLinks;j++)
				{
				(*(ppLinkX.ppL))[j] = (short)(dCos * psLinkX[j] - dSin * psLinkY[j]);
				(*(ppLinkY.ppL))[j] = (short)(dCos * psLinkY[j] + dSin * psLinkX[j]);
				}
			}


		// Convert to FSPR8:
		(*(ppBuf.ppI))->Convert(RImage::FSPR8);

		//----------------------------------------------------------------
		// move to the next element in the array:
		ppBuf.pB += lStructSize;
		pHotX.pB += lStructSize;
		pHotY.pB += lStructSize;
		ppLinkX.pB += lStructSize;
		ppLinkY.pB += lStructSize;
		}
	
	if (sNumLinks > 0)
		{
		free(psLinkX);
		free(psLinkY);
		}

	pimSrc->Convert((RImage::Type)ulOldType);
	
	return NULL;
	}


