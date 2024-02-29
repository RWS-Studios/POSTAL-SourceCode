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
// deals with general manipulation and use of signed 16-bit z-buffers (0 = center)
#ifndef ZBUFFER_H
#define ZBUFFER_H
//================================================== 
#include "Blue.h"
#ifdef PATHS_IN_INCLUDES
	#include "ORANGE/QuickMath/FixedPoint.h" // for RFixedS32
	#include "ORANGE/color/colormatch.h" // for RFixedS32
#else
	#include "FixedPoint.h" // for RFixedS32
	#include "ColorMatch.h" // for RFixedS32
#endif
//================================================== 
// Note that z-buffer assumes that positive z is
// towards the viewer.
//================================================== 
const short ZB_MIN_Z = -32768;

class	RZBuffer // a 16-bit signed z-buffer
	{
public:
	short m_sW;
	short m_sH;
	long m_lP; // pitch in WORDS! (Not a real pitch!)
	short* m_pBuf; // for now, don't have great need for alignment!
	//----------------------------------------------
	void	Init();
	RZBuffer();
	RZBuffer(short sW,short sH);
	short Create(short sW,short sH);
	~RZBuffer();
	short Destroy();
	//----------------------------------------------
	void Clear(short sVal = ZB_MIN_Z);
	//----------------------------------------------
	// debugging stuff
	short* GetZPtr(short sX,short sY){return (m_pBuf + sX + m_lP*sY);}
	void TestHeight(RImage* pimDst,short sDepth,
		short sX,short sY,short sW,short sH);
	void Dump(RImage* pimDst,short sX,short sY,short sW,short sH,UCHAR* pZCol);
	};




//================================================== 
//================================================== 
//================================================== 
 
//================================================== 
#endif
