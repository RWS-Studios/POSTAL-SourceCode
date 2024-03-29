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
// This is the actual 3d engine.  It is resposible for triangle rendering with effects.
// It understands NOTHING of higher level structures -> that is controlled in 
// pipeline.h, which brings all the low level containers together into a
// low level engine
//

#ifndef RENDER_H
#define RENDER_H
//================================================== 
#include "Blue.h"
#ifdef PATHS_IN_INCLUDES
	#include "GREEN/BLiT/BLIT.H" // for rspLine
	#include "GREEN/3D/zbuffer.h" // for rspLine
	#include "ORANGE/color/colormatch.h"
	#include "ORANGE/QuickMath/VectorMath.h"
	#include "ORANGE/QuickMath/FixedPoint.h"
#else
	#include "BLIT.H" // for rspLine
	#include "zbuffer.h" // for rspLine
	#include "colormatch.h"
	#include "vectormath.h"
	#include "fixedpoint.h"
#endif
//================================================== 

// all render effects on!
// Offset pFog to line up with the location of the object.
// Note that a unique fog table is needed for each
// triangle color!
//
extern	void	DrawTri_ZColorFog(uint8_t* pDst,int32_t lDstP,
			RP3d* p1,RP3d* p2,RP3d* p3,
			RZBuffer* pZB,uint8_t* pFog, 
			int16_t sOffsetX = 0,		// In: 2D offset for pZB.
			int16_t sOffsetY = 0); 	// In: 2D offset for pZB.

//================================================== 
// For debugging:
extern	void	DrawTri_wire(RImage* pimDst,int16_t sX,int16_t sY,
			RP3d* p1,RP3d* p2,RP3d* p3,uint8_t ucColor); 

extern	void	DrawTri_ZColor(uint8_t* pDst,int32_t lDstP,
			RP3d* p1,RP3d* p2,RP3d* p3,
			RZBuffer* pZB,uint8_t pFlatColor,
			int16_t sOffsetX = 0,		// In: 2D offset for pZB.
			int16_t sOffsetY = 0); 	// In: 2D offset for pZB.
//================================================== 
//================================================== 
// FLAT SHADED!
// sX and sY are additional offsets into pimDst
// There is NO Z_BUFFER here!  It is JUST a polygon drawer
//
extern	void	DrawTri(uint8_t* pDstOffset,int32_t lDstP,
			RP3d* p1,RP3d* p2,RP3d* p3,
			uint8_t ucFlatColor);

//================================================== 
#endif
