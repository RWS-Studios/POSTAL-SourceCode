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
// This is one of the fist advanced BLiT functions supporting any color depth!

#include "System.h"

#ifdef PATHS_IN_INCLUDES
	#include "GREEN/BLiT/BLIT.H"
	#include "GREEN/BLiT/Cfnt.h"
	#include "ORANGE/QuickMath/Fractions.h"
#else
	#include "BLIT.H"
	#include "Cfnt.h"
	#include "fractions.h"
#endif

#define _ROT64 // 64-bit math

// This function will be made modular assuming the RSPiX inliner
// will become a reality...
//

// For example, it will use the identical clipping and mirroring
// technique that ScaleFlat96 used.
//-------------------------------------------------------------------

// DstW and DstH MUST be positive (not zero)!!!
// This is POST CLIPPING, POST MIRRORING, POST PARAMETER VALIDATION!
//
inline void _BlitRot(short sDeg,short sHeight, // = 2R + 1?
				  UCHAR* pSrcData,long lSrcXP,long lSrcP,
				  UCHAR* pDstData,long lDstXP,long lDstP,
				  short sDstW,short sDstH, // Target this for inlining!
				  short sClipL,short sClipR,short sClipT,short sClipB)
	{
	// This algorithm uses 4 divisions per BLiT
	// Second order stuff...

	//--------------------------------------------------
	//----------- Initialize the large scale parameters:
	sDeg = rspMod360(sDeg);
	short sDegR = sDeg, sDegL = sDeg + 90, sDegP = sDeg + 225;
	// we know sDegV and sDeP must be positive, so don't do a full mod:
	while (sDegL > 359) sDegL -= 360; // FMOD/2
	while (sDegP > 359) sDegP -= 360; // FMOD/2

	//*************** Find P
	short sR = ( (sHeight-1) >> 1);		// spinning corner radius
	short sEdge = short(sR * rspSQRT2);      // square edge size
	// all things start at this source location:
	UCHAR* pP;

	long	lDen = long(sDstW) * sDstH; // compound fraction

	// Calculate the sub pixel offset:
	long lLadNumX,lLadNumY,lRungNumX,lRungNumY;

	//**********************************************
	//******  OVERFLOW AREA - NEEDS 64bit math!
	//**********************************************
	// OVERFLOW LIMIT: lDstW * lDstH * sSrcW * sSrcH;
	// MUST CUSTOMIZE FOR THE MAC:

#ifdef _ROT64

	// 64-bit version:
	S64 l64PX = S64(lDen) * sR,l64PY = S64(lDen) * sR; // go into Denominator space
	// Stay in denominator space!
	l64PX += S64(COSQ[sDegP] * sR * lDen + 0.5 * lDen); // offset to pixel center
	l64PY += S64(SINQ[sDegP] * sR * lDen  + 0.5 * lDen); // offset to pixel center
	// Now MUST convert to an asymettrical signed proper fraction:
	long lPX,lPY;
	rspDivModA64(l64PX,lDen,lPX,lLadNumX);
	rspDivModA64(l64PY,lDen,lPY,lLadNumY);

	pP = pSrcData + lSrcXP * lPX + lSrcP * lPY;

#else

	// 32-bit version
	long lPX = lDen * sR,lPY = lDen * sR; // go into Denominator space
	// Stay in denominator space!
	lPX += long(COSQ[sDegP] * sR * lDen + 0.5 * lDen); // offset to pixel center
	lPY += long(SINQ[sDegP] * sR * lDen  + 0.5 * lDen); // offset to pixel center
	// Now MUST convert to an asymettrical signed proper fraction:
	rspDivModA64(lPX,lDen,lPX,lLadNumX);
	rspDivModA64(lPY,lDen,lPY,lLadNumY);

	pP = pSrcData + lSrcXP * lPX + lSrcP * lPY;

#endif

	//**********************************************

	//--------------------------------------------------
	//--------- Initialize the four ratios...
	//
	// 1) Unnormalized ratios:
	//
	// Find the signed vector length of the rungs, in compound
	// fraction form...

	long lRungW = long(COSQ[sDegR] * sEdge * sDstH); // horizontal
	long lRungH = long(SINQ[sDegR] * sEdge * sDstH);
	long lLadW = long(COSQ[sDegL] * sEdge * sDstW);  // vertical
	long lLadH = long(SINQ[sDegL] * sEdge * sDstW);
	// Convert from a point offset to true width and height:

	lRungW += SGN3(lRungW); // 3 phase sign
	lRungH += SGN3(lRungH); // 3 phase sign
	lLadW += SGN3(lLadW); // 3 phase sign
	lLadH += SGN3(lLadH); // 3 phase sign

	//
	// 2) Normalize the ratios if needed...
	//
	long lRungIncX = lRungW;
	long lRungIncY = lRungH;
	long lLadIncX = lLadW;
	long lLadIncY = lLadH;
	// General parameters:
	long lRungDelX=0,lRungDelY=0,lLadDelX=0,lLadDelY=0;
	// For now, only distinguish Rung IC, keep Ladder general:
	short sCondition = 0; // CONDITION FLAG

#define sNegRungX 4
#define sNegRungY 1
#define sImproperRungX 8
#define sImproperRungY 2
#define sPosRungX 0
#define sPosRungY 0
#define sProperRungX 0
#define sProperRungY 0

	// NOTE:  ANSI does NOT define the sign of mod, but since the
	// denominator is ALWAYS positive, I can adjust for MOD issues...
	// The integral divisor is the same magnitude as the positive devisor
	//
	// FOR DELTA fractions, I will use a symmetric mod.
	// For COORDINATE fractions, I wil use an asymmetric mod:
	//
	//=========================== USE GENERAL IC for any case....
	if (ABS(lLadIncX) >= lDen)
		{
		DIV_MOD(lLadIncX,lDen,lLadDelX,lLadIncX);
		lLadDelX *= lSrcXP; // make the delta variables into screen space
		}

	if (ABS(lLadIncY) >= lDen)
		{
		DIV_MOD(lLadIncY,lDen,lLadDelY,lLadIncY);
		lLadDelY *= lSrcP; // make the delta variables into screen space
		}

	//============ DECIDE CHOICE FOR FUNCTION IC OVERLOAD ===========

	// Set of the Rung stuff for template IC overloading!
	if (lRungIncX <= 0) // negative increment
		{
		sCondition |= sNegRungX; // for IC function overload
		if (-lRungIncX >= lDen)
			{
			sCondition |= sImproperRungX; // for IC function overload
			DIV_MOD(lRungIncX,lDen,lRungDelX,lRungIncX);
			lRungDelX *= lSrcXP; // make the delta variables into screen space
			}
		}
	else // positive increment
		{
		if (lRungIncX >= lDen)
			{
			sCondition |= sImproperRungX; // for IC function overload
			DIV_MOD(lRungIncX,lDen,lRungDelX,lRungIncX);
			lRungDelX *= lSrcXP; // make the delta variables into screen space
			}
		}

	// Y component...
	if (lRungIncY <= 0) // negative increment
		{
		sCondition |= sNegRungY; // for IC function overload
		if (-lRungIncY >= lDen)
			{
			sCondition |= sImproperRungY; // for IC function overload
			DIV_MOD(lRungIncY,lDen,lRungDelY,lRungIncY);
			lRungDelY *= lSrcP; // make the delta variables into screen space
			}
		}
	else // positive increment
		{
		if (lRungIncY >= lDen)
			{
			sCondition |= sImproperRungY; // for IC function overload
			DIV_MOD(lRungIncY,lDen,lRungDelY,lRungIncY);
			lRungDelY *= lSrcP; // make the delta variables into screen space
			}
		}

	//==================================================================
	// relative to pP:
	long lLadX = 0,lLadY = 0,lRungX,lRungY;

	// test template, condition 0, no clip:
	UCHAR *pDst,*pDstLine = pDstData; // offset included for us...
	short i,j;

	// Left Top Clipping:
	// We can Top Left lip simply by moving the staring position
	// and reducing the width and height of the window:

	// 1) move up the ladder by sClipT:
	if (sClipT)
		{
		for (j = sClipT; j!=0; j--)
			{
			//-------------------- Advance the Ladder...
			// Use most general (slow) IC case...
			rspfrAdd32(lLadX,lLadNumX,lLadDelX,lLadIncX,lDen,lSrcXP);
			rspfrAdd32(lLadY,lLadNumY,lLadDelY,lLadIncY,lDen,lSrcP);

			pDstLine += lDstP;
			}

		sDstH -= sClipT;
		}

	// 2) move across the rung by sClipL:
	if (sClipL)
		{
		for (i = sClipL; i != 0; i--)
			{
			// Advance the Ladder in the rung direction...
			// Use general case for testing, then specific case later
			rspfrAdd32(lLadX,lLadNumX,lRungDelX,lRungIncX,lDen,lSrcXP);
			rspfrAdd32(lLadY,lLadNumY,lRungDelY,lRungIncY,lDen,lSrcP);

			pDstLine += lDstXP;
			}

		sDstW -= sClipL;
		}

	// Right Bottom Clipping:  Easy
	sDstW -= sClipR;
	sDstH -= sClipB;

	for (j = sDstH; j!=0; j--)
		{
		//------------------- Set Rung to Ladder...
		pDst = pDstLine;
		lRungX = lLadX;
		lRungY = lLadY;
		lRungNumX = lLadNumX;
		lRungNumY = lLadNumY;
		//---------------------------------------------------------
		// Zeroth order stuff....
		PROPER proper = 0;		// Value doesn't matter since var isn't REALLY used -- just want to avoid unitialized warning
		IMPROPER improper = 0;	// Value doesn't matter since var isn't REALLY used -- just want to avoid unitialized warning
		POSITIVE positive = 0;	// Value doesn't matter since var isn't REALLY used -- just want to avoid unitialized warning
		NEGATIVE negative = 0;	// Value doesn't matter since var isn't REALLY used -- just want to avoid unitialized warning
		switch(sCondition)
			{
			UCHAR ucPix;	// The only 8-bit concept:

			case (sProperRungX | sPosRungX | sProperRungY | sPosRungY):
				for (i = sDstW; i != 0; i--)
					{
					ucPix = *(pP + lRungX + lRungY);if (ucPix) *pDst = ucPix;
					rspfrAdd32(lRungX,lRungNumX,lRungDelX,lRungIncX,lDen,lSrcXP, proper, positive);
					rspfrAdd32(lRungY,lRungNumY,lRungDelY,lRungIncY,lDen,lSrcP, proper, positive);
					pDst += lDstXP;
					}
			break;
			case (sProperRungX | sPosRungX | sProperRungY | sNegRungY):
				for (i = sDstW; i != 0; i--)
					{
					ucPix = *(pP + lRungX + lRungY);if (ucPix) *pDst = ucPix;
					rspfrAdd32(lRungX,lRungNumX,lRungDelX,lRungIncX,lDen,lSrcXP, proper, positive);
					rspfrAdd32(lRungY,lRungNumY,lRungDelY,lRungIncY,lDen,lSrcP, proper, negative);
					pDst += lDstXP;
					}
			break;
			case (sProperRungX | sPosRungX | sImproperRungY | sPosRungY):
				for (i = sDstW; i != 0; i--)
					{
					ucPix = *(pP + lRungX + lRungY);if (ucPix) *pDst = ucPix;
					rspfrAdd32(lRungX,lRungNumX,lRungDelX,lRungIncX,lDen,lSrcXP, proper, positive);
					rspfrAdd32(lRungY,lRungNumY,lRungDelY,lRungIncY,lDen,lSrcP, improper, positive);
					pDst += lDstXP;
					}
			break;
			case (sProperRungX | sPosRungX | sImproperRungY | sNegRungY):
				for (i = sDstW; i != 0; i--)
					{
					ucPix = *(pP + lRungX + lRungY);if (ucPix) *pDst = ucPix;
					rspfrAdd32(lRungX,lRungNumX,lRungDelX,lRungIncX,lDen,lSrcXP, proper, positive);
					rspfrAdd32(lRungY,lRungNumY,lRungDelY,lRungIncY,lDen,lSrcP, improper, negative);
					pDst += lDstXP;
					}
			break;
			case (sProperRungX | sNegRungX | sProperRungY | sPosRungY):
				for (i = sDstW; i != 0; i--)
					{
					ucPix = *(pP + lRungX + lRungY);if (ucPix) *pDst = ucPix;
					rspfrAdd32(lRungX,lRungNumX,lRungDelX,lRungIncX,lDen,lSrcXP, proper, negative);
					rspfrAdd32(lRungY,lRungNumY,lRungDelY,lRungIncY,lDen,lSrcP, proper, positive);
					pDst += lDstXP;
					}
			break;
			case (sProperRungX | sNegRungX | sProperRungY | sNegRungY):
				for (i = sDstW; i != 0; i--)
					{
					ucPix = *(pP + lRungX + lRungY);if (ucPix) *pDst = ucPix;
					rspfrAdd32(lRungX,lRungNumX,lRungDelX,lRungIncX,lDen,lSrcXP, proper, negative);
					rspfrAdd32(lRungY,lRungNumY,lRungDelY,lRungIncY,lDen,lSrcP, proper, negative);
					pDst += lDstXP;
					}
			break;
			case (sProperRungX | sNegRungX | sImproperRungY | sPosRungY):
				for (i = sDstW; i != 0; i--)
					{
					ucPix = *(pP + lRungX + lRungY);if (ucPix) *pDst = ucPix;
					rspfrAdd32(lRungX,lRungNumX,lRungDelX,lRungIncX,lDen,lSrcXP, proper, negative);
					rspfrAdd32(lRungY,lRungNumY,lRungDelY,lRungIncY,lDen,lSrcP, improper, positive);
					pDst += lDstXP;
					}
			break;
			case (sProperRungX | sNegRungX | sImproperRungY | sNegRungY):
				for (i = sDstW; i != 0; i--)
					{
					ucPix = *(pP + lRungX + lRungY);if (ucPix) *pDst = ucPix;
					rspfrAdd32(lRungX,lRungNumX,lRungDelX,lRungIncX,lDen,lSrcXP, proper, negative);
					rspfrAdd32(lRungY,lRungNumY,lRungDelY,lRungIncY,lDen,lSrcP, improper, negative);
					pDst += lDstXP;
					}
			break;
			case (sImproperRungX | sPosRungX | sProperRungY | sPosRungY):
				for (i = sDstW; i != 0; i--)
					{
					ucPix = *(pP + lRungX + lRungY);if (ucPix) *pDst = ucPix;
					rspfrAdd32(lRungX,lRungNumX,lRungDelX,lRungIncX,lDen,lSrcXP, improper, positive);
					rspfrAdd32(lRungY,lRungNumY,lRungDelY,lRungIncY,lDen,lSrcP, proper, positive);
					pDst += lDstXP;
					}
			break;
			case (sImproperRungX | sPosRungX | sProperRungY | sNegRungY):
				for (i = sDstW; i != 0; i--)
					{
					ucPix = *(pP + lRungX + lRungY);if (ucPix) *pDst = ucPix;
					rspfrAdd32(lRungX,lRungNumX,lRungDelX,lRungIncX,lDen,lSrcXP, improper, positive);
					rspfrAdd32(lRungY,lRungNumY,lRungDelY,lRungIncY,lDen,lSrcP, proper, negative);
					pDst += lDstXP;
					}
			break;
			case (sImproperRungX | sPosRungX | sImproperRungY | sPosRungY):
				for (i = sDstW; i != 0; i--)
					{
					ucPix = *(pP + lRungX + lRungY);if (ucPix) *pDst = ucPix;
					rspfrAdd32(lRungX,lRungNumX,lRungDelX,lRungIncX,lDen,lSrcXP, improper, positive);
					rspfrAdd32(lRungY,lRungNumY,lRungDelY,lRungIncY,lDen,lSrcP, improper, positive);
					pDst += lDstXP;
					}
			break;
			case (sImproperRungX | sPosRungX | sImproperRungY | sNegRungY):
				for (i = sDstW; i != 0; i--)
					{
					ucPix = *(pP + lRungX + lRungY);if (ucPix) *pDst = ucPix;
					rspfrAdd32(lRungX,lRungNumX,lRungDelX,lRungIncX,lDen,lSrcXP, improper, positive);
					rspfrAdd32(lRungY,lRungNumY,lRungDelY,lRungIncY,lDen,lSrcP, improper, negative);
					pDst += lDstXP;
					}
			break;
			case (sImproperRungX | sNegRungX | sProperRungY | sPosRungY):
				for (i = sDstW; i != 0; i--)
					{
					ucPix = *(pP + lRungX + lRungY);if (ucPix) *pDst = ucPix;
					rspfrAdd32(lRungX,lRungNumX,lRungDelX,lRungIncX,lDen,lSrcXP, improper, negative);
					rspfrAdd32(lRungY,lRungNumY,lRungDelY,lRungIncY,lDen,lSrcP, proper, positive);
					pDst += lDstXP;
					}
			break;
			case (sImproperRungX | sNegRungX | sProperRungY | sNegRungY):
				for (i = sDstW; i != 0; i--)
					{
					ucPix = *(pP + lRungX + lRungY);if (ucPix) *pDst = ucPix;
					rspfrAdd32(lRungX,lRungNumX,lRungDelX,lRungIncX,lDen,lSrcXP, improper, negative);
					rspfrAdd32(lRungY,lRungNumY,lRungDelY,lRungIncY,lDen,lSrcP, proper, negative);
					pDst += lDstXP;
					}
			break;
			case (sImproperRungX | sNegRungX | sImproperRungY | sPosRungY):
				for (i = sDstW; i != 0; i--)
					{
					ucPix = *(pP + lRungX + lRungY);if (ucPix) *pDst = ucPix;
					rspfrAdd32(lRungX,lRungNumX,lRungDelX,lRungIncX,lDen,lSrcXP, improper, negative);
					rspfrAdd32(lRungY,lRungNumY,lRungDelY,lRungIncY,lDen,lSrcP, improper, positive);
					pDst += lDstXP;
					}
			break;
			case (sImproperRungX | sNegRungX | sImproperRungY | sNegRungY):
				for (i = sDstW; i != 0; i--)
					{
					ucPix = *(pP + lRungX + lRungY);if (ucPix) *pDst = ucPix;
					rspfrAdd32(lRungX,lRungNumX,lRungDelX,lRungIncX,lDen,lSrcXP, improper, negative);
					rspfrAdd32(lRungY,lRungNumY,lRungDelY,lRungIncY,lDen,lSrcP, improper, negative);
					pDst += lDstXP;
					}
			break;

			}

		/*
		for (i = sDstW; i != 0; i--)
			{
			// The only 8-bit concept:
			UCHAR ucPix;
			ucPix = *(pP + lRungX + lRungY); // pitch included!
			if (ucPix) *pDst = ucPix;

			//------------------ Advance the rung...
			// Use general case for testing, then specific case later
			rspfrAdd32(lRungX,lRungNumX,lRungDelX,lRungIncX,lDen,lSrcXP);
			rspfrAdd32(lRungY,lRungNumY,lRungDelY,lRungIncY,lDen,lSrcP);

			pDst += lDstXP;
			}
		*/

		//---------------------------------------------------------
		//-------------------- Advance the Ladder...
		// Use most general (slow) IC case...
		rspfrAdd32(lLadX,lLadNumX,lLadDelX,lLadIncX,lDen,lSrcXP);
		rspfrAdd32(lLadY,lLadNumY,lLadDelY,lLadIncY,lDen,lSrcP);

		pDstLine += lDstP;
		}
	}

// With the inliner, this may become a very valuable tool.
// This only performs DESTINATION CLIPPING AND MIRRORING..
//
// returns -1 if clipped out
// returns 0 if shows at all
// if (sClipL), use an HCLIP routine!
//
// returns pDst and sClips based on mirroring...
// does NOT currently check input parameters...
//
inline short rspClipMirrorDst(RImage* pimDst, // input:
										short sClipX,	// MUST be set
										short sClipY,
										short sClipW,
										short sClipH,
										short &sDstX,  // Input AND output:
										short &sDstY,
										short &sDstW, 	// Negative for mirror 
										short &sDstH,	// Negative for mirror
										UCHAR* &pDst,	// OUTPUT:
										short &sClipL, // positive = clip...
										short &sClipR,
										short &sClipT,
										short &sClipB,
										long	&lDstP,	// Including mirroring
										long	&lDstPX	// Incl. Mirroring & pixDepth
										)
	{
	short sMirrorX = 1,sMirrorY = 1; // direction flags...
	//********************* MIRROR PART I => PRE CLIP:
	lDstP = pimDst->m_lPitch;
	lDstPX = (pimDst->m_sDepth>>8);

	if (sDstW < 0)
		{
		sMirrorX = -1; // flip the destination square's edges...
		sDstW = -sDstW;
		sDstX -= (sDstW - 1);
		lDstPX = -lDstPX;
		}

	if (sDstH < 0)
		{
		sMirrorY = -1; // flip the destination square's edges...
		sDstH = -sDstH;
		sDstY -= (sDstH - 1);
		lDstP = -lDstP;
		}

	//-------- Do the clipping:
	sClipL = sClipX - sDstX; if (sClipL < 0) sClipL = 0;
	sClipT = sClipY - sDstY; if (sClipT < 0) sClipT = 0;
	sClipR = (sDstX + sDstW - sClipX - sClipW); if (sClipR < 0) sClipR = 0;
	sClipB = (sDstY + sDstH - sClipY - sClipH); if (sClipB < 0) sClipB = 0;

	if ( ((sClipL + sClipR) >= sDstW) || ((sClipT + sClipB) >= sDstH) )
		{
		return -1; // fully clipped out
		}

	//********************* MIRROR PART II => POST CLIP, flip back...
	if (sMirrorX == -1)
		{
		sDstX += (sDstW - 1);
		SWAP(sClipL,sClipR); // the drawing order is reversed...
		}

	if (sMirrorY == -1)
		{
		sDstY += (sDstH - 1);
		SWAP(sClipT,sClipB); // the drawing order is reversed...
		}

	// Use original pitch signs for offsets...
	pDst = pimDst->m_pData + pimDst->m_lPitch * sDstY + sDstX;
	
	return 0;
	}

// Will allocate a larger buffer and copy the image so that it
// can be rotated about the center (sHotX,sHotY).
// m_lXPos and m_lYPos will point to the moved origin of the 
// RImage
//
short rspAddRotationPadding(RImage* pimSrc,short sHotX,short sHotY)
	{
#ifdef _DEBUG

	// Check to make sure this is at least possibly correct styled assets..
	if (!pimSrc)
		{
		TRACE("rspAddRotationPadding: Null input images!\n");
		return -1;
		}

		// do type checking....
	//------------------------------------------------------------------------------
	if (!ImageIsUncompressed(pimSrc->m_type))
		{
		TRACE("rspAddRotationPadding: Only use this function with uncompressed Images.\n");
		return -1;
		}

	//------------------------------------------------------------------------------
	// do depth checking...
	//------------------------------------------------------------------------------
	if ((pimSrc->m_sDepth != 8))
		{
		TRACE("rspAddRotationPadding: Currently, only 8-bit Images are fully supported.\n");
		return -1;
		}

	if (pimSrc->m_sWinX || pimSrc->m_sWinY)
		{
		TRACE("rspAddRotationPadding: This image already has padding of some kind!\n");
		return -1;
		}

#endif

	// Calculate maximum radius needed for a rotating square about the
	// center encapsulating the entire image...
	//

	long lR=0; // squared at first...

	long lHotXS = SQR((long)sHotX);
	long lHotYS = SQR((long)sHotY);
	// The radius will also equal the center of the buffer, and the buffer
	// size will be 2R+1 to deal with the half pixel buffer on all sides.
	//
	lR = MAX(lR,(long)SQR((long)sHotX - (pimSrc->m_sWidth - 1) ) + 
		(long)SQR((long)sHotY - (pimSrc->m_sHeight - 1) ) ); // LR corner
	lR = MAX(lR,lHotXS + (long)SQR((long)sHotY - (pimSrc->m_sHeight - 1) ) ); // UR corner
	lR = MAX(lR,SQR(sHotX - (pimSrc->m_sWidth - 1) ) + lHotYS ); // LL corner
	lR = MAX(lR,lHotXS + lHotYS ); // UL corner

	lR = long(0.999999 + sqrt(double(lR) * 2.0)); // round up
	// The sqrt2 factor is needed because the moving window must enclose the circle.

	short sSize = short(1 + (lR << 1) ); // buffer = 2R + 1
	// Calculate new position of image within the buffer:
	short sX = short (lR - sHotX); // new offset...
	short sY = short (lR - sHotY);
	short sOldW = pimSrc->m_sWidth;
	short sOldH = pimSrc->m_sHeight;

	rspPad(pimSrc,sX,sY,sSize,sSize,1); // go to 8-bit alignment since offset may not align

	return 0;
	}

// This uses the built in image description of padding.
// This should really be a part of RImage.
// This function currently only supports 8bit, uncompressed images..
//
short rspRemovePadding(RImage* pimSrc)
	{
#ifdef _DEBUG

	// Check to make sure this is at least possibly correct styled assets..
	if (!pimSrc)
		{
		TRACE("rspRemovePadding: Null input images!\n");
		return -1;
		}

		// do type checking....
	//------------------------------------------------------------------------------
	if (!ImageIsUncompressed(pimSrc->m_type))
		{
		TRACE("rspRemovePadding: Only use this function with uncompressed Images.\n");
		return -1;
		}

	//------------------------------------------------------------------------------
	// do depth checking...
	//------------------------------------------------------------------------------
	if ((pimSrc->m_sDepth != 8))
		{
		TRACE("rspRemovePadding: Currently, only 8-bit Images are fully supported.\n");
		return -1;
		}

#endif

	if (!pimSrc->m_sWinX && !pimSrc->m_sWinY)
		{
		// Don't need to do anything
		return 0;
		}

	// Create a new buffer and Image Stub to BLiT can use it:
	RImage imDst;
	long lNewPitch = (pimSrc->m_sWinWidth + 15) & ~15; // 128 bit align it
	imDst.CreateImage(pimSrc->m_sWinWidth,pimSrc->m_sWinHeight,pimSrc->m_type,
		lNewPitch,pimSrc->m_sDepth);

	// 3) Copy the new one in...
	rspBlit(pimSrc,&imDst,pimSrc->m_sWinX,pimSrc->m_sWinY,0,0,
		pimSrc->m_sWinWidth,pimSrc->m_sWinHeight);

	// tricky part: Swap buffers...
	UCHAR	*pSrcMem,*pSrcBuf;
	pimSrc->DetachData((void**)&pSrcMem,(void**)&pSrcBuf);
	// Move the new buffer back to the original
	imDst.DetachData((void**)&(pimSrc->m_pMem),(void**)&(pimSrc->m_pData));

	//*******  IMPORTANT! COPY ALL NEW INFO OVER!
	pimSrc->m_ulSize = imDst.m_ulSize;
	pimSrc->m_sWidth = imDst.m_sWidth; // width and height shouldn't change...
	pimSrc->m_sHeight = imDst.m_sHeight;
	pimSrc->m_lPitch = imDst.m_lPitch;
	pimSrc->m_sWinX = 0;
	pimSrc->m_sWinY = 0;

	// 4) Destroy the old...
	RImage::DestroyDetachedData((void**)&pSrcMem);
	// On exit, imDst will auto destruct....
	return 0;
	}



// Assumes you are inputing the location of the center of rotation,
// which is, in reality, the exact center of the buffered source
// image.
//
short rspBlitRot(short sDeg,RImage* pimSrc,RImage* pimDst,
					 short sDstX,short sDstY,short sDstW,short sDstH,
					 const RRect* prDstClip)
	{
#ifdef _DEBUG
	// Check to make sure this is at least possibly correct styled assets..
	if (!pimSrc || !pimDst)
		{
		TRACE("rspBlitRot: Null input images!\n");
		return -1;
		}

		// do type checking....
	//------------------------------------------------------------------------------
	if (!ImageIsUncompressed(pimSrc->m_type))
		{
		TRACE("rspBlitT: Only use this function with uncompressed Images.\n");
		return -1;
		}

	if (!ImageIsUncompressed(pimDst->m_type))
		{
		TRACE("rspBlitT: Only use this function with uncompressed Images.\n");
		return -1;
		}

	//------------------------------------------------------------------------------
	// do depth checking...
	//------------------------------------------------------------------------------
	if ((pimSrc->m_sDepth != 8) || (pimDst->m_sDepth != 8))
		{
		TRACE("rspBlitT: Currently, only 8-bit Images are fully supported.\n");
		return -1;
		}

	if (!pimSrc->m_sWinX || !pimSrc->m_sWinY)
		{
		TRACE("rspBlitRot: You must add buffer padding to the source image!\n");
		return -1;
		}

#endif

	//------------------------------------------------------------------------------
	// do optional destination clipping
	//------------------------------------------------------------------------------

	// Destination clippiong rectangle:
	// NOTE: effective destination clipping done at a lower level.
	// Only source clipping would effect this...
	//
	short sDstClipX = 0, sDstClipY = 0;
	short	sDstClipW = pimDst->m_sWidth, sDstClipH = pimDst->m_sHeight;

	// Suck it out!
	if (prDstClip)
		{
		sDstClipX = prDstClip->sX;
		sDstClipY = prDstClip->sY;
		sDstClipW = prDstClip->sW;
		sDstClipH = prDstClip->sH;
		}

	//********************* MIRROR PART I => PRE CLIP:
	// Instead of mirror FLAGS, make use of the destination pitch:
	//
	long lDstP = pimDst->m_lPitch,lDstXP = (pimDst->m_sDepth>>3);
	short sMirrorH = 1,sMirrorV = 1;

	if (sDstW < 0)
		{
		sMirrorH = -1; // flip the destination square's edges...
		sDstW = -sDstW;
		sDstX -= (sDstW - 1);
		lDstXP = -lDstXP;
		}

	if (sDstH < 0)
		{
		sMirrorV = -1; // flip the destination square's edges...
		sDstH = -sDstH;
		sDstY -= (sDstH - 1);
		lDstP = -lDstP;
		}
	//*********************

	//-------- Do the clipping:
	short sClipL,sClipR,sClipT,sClipB; // positive = clipped

	sClipL = sDstClipX - sDstX; if (sClipL < 0) sClipL = 0;
	sClipT = sDstClipY - sDstY; if (sClipT < 0) sClipT = 0;
	sClipR = (sDstX + sDstW - sDstClipX - sDstClipW); if (sClipR < 0) sClipR = 0;
	sClipB = (sDstY + sDstH - sDstClipY - sDstClipH); if (sClipB < 0) sClipB = 0;

	if ( ((sClipL + sClipR) >= sDstW) || ((sClipT + sClipB) >= sDstH) )
		{
		return 0; // fully clipped out
		}

	//********************* MIRROR PART II => POST CLIP, flip back...
	if (sMirrorH == -1)
		{
		sDstX += (sDstW - 1);
		SWAP(sClipL,sClipR); // the drawing order is reversed...
		}

	if (sMirrorV == -1)
		{
		sDstY += (sDstH - 1);
		SWAP(sClipT,sClipB); // the drawing order is reversed...
		}


	//*********************

	//------------------------------------------------------------------------------
	// set up IC
	//------------------------------------------------------------------------------
	// Use the old pitch because the coordinates have been changed
	UCHAR*	pDst = pimDst->m_pData + pimDst->m_lPitch * sDstY + sDstX;
	// There is NO source position or source clipping here!
	UCHAR*	pSrc = pimSrc->m_pData;

	// pass the mirrored pitches....
	_BlitRot(sDeg,pimSrc->m_sHeight,pSrc,pimSrc->m_sDepth>>3,pimSrc->m_lPitch,
				pDst,lDstXP,lDstP,sDstW,sDstH,sClipL,sClipR,sClipT,sClipB);

	return 0;
	}

// Assumes you are inputing the location of the center of rotation,
// which is, in reality, the exact center of the buffered source
// image.
// The scale is in terms of the source image, i.e. (1.0,1.0) = actual size
// Negative values mirror!
//
short rspBlitRot(short sDeg,RImage* pimSrc,RImage* pimDst,
					 short sDstX,short sDstY,double dScaleX,double dScaleY,
					 const RRect* prDstClip)
	{
	short sDstW,sDstH;

#ifdef _DEBUG // Do the minimum redundant validation necessary:

	// Check to make sure this is at least possibly correct styled assets..
	if (!pimSrc || !pimDst)
		{
		TRACE("rspBlitRot: Null input images!\n");
		return -1;
		}

#endif

	// Calculate the true dimensions.
	// This occurs when DstW = EdgeW
	short sR = ( (pimSrc->m_sHeight-1) >> 1);			// spinning corner radius
	short sEdge = short(sR * rspSQRT2);			// square edge size

	sDstW = short(dScaleX * sEdge);
	sDstH = short(dScaleY * sEdge);

	return rspBlitRot(sDeg,pimSrc,pimDst,sDstX,sDstY,sDstW,sDstH,prDstClip);
	}

//**************************************************************
//***  BECAUSE THE OLD SrafeRot used GENERIC input structures
//***  to hold the auxiliary data, it should still be useful!
//**************************************************************

// In this version, You must supply the host structure, which you may define, but which 
// must contain the following:  
//
// NOTE: StrafeRotate has been made backwards compatible in two ways:
// 1) it will AGAIN default to using a "CSTRAFE" output structure
// 2) Degrees will be CLOCKWISE, and the offsets will be HOTSPOT convention.
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

#endif

	union { short *pL; UCHAR *pB; } pHotX,pHotY;
	union { short **ppL; UCHAR *pB; } ppLinkX,ppLinkY;
	union { RImage **ppI; UCHAR *pB; } ppBuf;

	// IN PREVIOUS VERSIONS, THE USER COULD NOT UINPUT VALUES,
	// And then I would fill in a CStrafe for them.

	// NOTE: The CStrafe structure is not OFFICIALLY supported by RSPiX
	//  but I am offering backwards compatibility for now...

	if (!pIm) ppBuf.ppI = &(((CStrafe*)pReturnArray)->pImage);
	if (!psHotX) pHotX.pL = &((CStrafe*)pReturnArray)->sHotX;
	if (!psHotY) pHotY.pL = &((CStrafe*)pReturnArray)->sHotY;
	if (!ppsX) ppLinkX.ppL = &((CStrafe*)pReturnArray)->psLinkX;
	if (!ppsY) ppLinkY.ppL = &((CStrafe*)pReturnArray)->psLinkY;

	double dCurDeg = dStartDeg;
	// calculate degree increment in default case:
	if (dDegInc == 0.0) dDegInc = 360.0 / (double)sNumFrames;
	// Phase one:  make the source ROTBUF, and create a destination
	
	rspAddRotationPadding(pimSrc,sCenterX,sCenterY);
	short sSrcH = pimSrc->m_sHeight; // used for making a copy
	short sDstH = (sSrcH * dScale); // used for making a copy

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

	for (i=0;i<sNumFrames;i++,dCurDeg += dDegInc)
		{	
		// Make a large enough vessel to rotate in:
		*(ppBuf.ppI) = new RImage;
		if ((*(ppBuf.ppI))->CreateImage(sDstH,sDstH,RImage::BMP8)!= SUCCESS)
			{
			TRACE("rspStrafeRotate: Out of memory. Sorry.\n");
			return -1;
			}

		// Do the BLiT such that the hot spot is in the center of the buffer:

		//_RotateShrink(dCurDeg,pimSrc,(*ppBuf.ppI),0,0,sDstH,sDstH);
		
		// CREATE A CLOCKWISE SENSE:
		rspBlitRot(short(360.0 - dCurDeg),pimSrc,(*ppBuf.ppI),0,0,dScale,dScale);
		
		// Get the coordinates:
		short sX=0,sY=0,sW=(short)(*(ppBuf.ppI))->m_sWidth,sH = (short)(*(ppBuf.ppI))->m_sHeight;
		rspLasso((UCHAR)0,(*(ppBuf.ppI)),sX,sY,sW,sH);

		rspCrop((*(ppBuf.ppI)),sX,sY,sW,sH); // sX,sY are the blitting offset

		// Store the hot offset using the center of the RotBuf as origin
		// Subtract this from position you wish center to appear
		//
		*(pHotX.pL) = rspSQRT2 / 4. * sDstH - sX;
		*(pHotY.pL) = rspSQRT2 / 4. * sDstH - sY;
			

		// Dpo the links, if any:
		*(ppLinkX.ppL) = NULL;
		*(ppLinkY.ppL) = NULL;

		if (sNumLinks > 0)
			{
			*(ppLinkX.ppL) = (short*)calloc(sizeof(short),sNumLinks);
			*(ppLinkY.ppL) = (short*)calloc(sizeof(short),sNumLinks);

			//double dRad = dCurDeg *  0.01745329251994;
			short sCurDeg = short(dCurDeg); // CLOCKWISE SENSE
			sCurDeg = rspMod360(sCurDeg);

			double dSin = SINQ[sCurDeg]*dScale;
			double dCos = COSQ[sCurDeg]*dScale;

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

	// Restore the source picture to it's original form.
	rspRemovePadding(pimSrc);
	
	return NULL;
	}

//**************************************************************
