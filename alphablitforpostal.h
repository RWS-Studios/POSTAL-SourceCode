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
// alphaBLiTforPostal.h
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
////////////////////////////////////////////////////////////////////////////////

#ifndef ALPHA_BLIT_FOR_POSTAL_H
#define ALPHA_BLIT_FOR_POSTAL_H
//=================================

// This draws source to destination with clipping only if NEITHER are zero!
extern	void rspMaskBlit(RImage* pimSrc,RImage* pimDst,short sDstX,short sDstY);

// Takes a BMP8 and converts it to a mask of 0 and ucVal
extern	void rspCopyAsMask(RImage* pimSrc,RImage* pimDst,UCHAR ucVal);

extern	void g_alphaBlit(
		RImage* pimSrc,			// Source image. (wall)
		RImage* pimDst,			// Destination image.
		RImage* pimMask,			// Mask of alphable area.
		RMultiAlpha* pma,			// Table of alphas or something.
		short sAlphaX,				// Source coordinate in pimSrc to put alphamask.
		short sAlphaY,				// Source coordinate in pimSrc to put alphamask.
		short sDstX,				// Destination coordinate in pimDst for pimSrc(0,0).
		short sDstY,				// Destination coordinate in pimDst for pimSrc(0,0).
		RRect &rDstClip);			// Rectangle to clip Dst to.




//=================================
#endif
