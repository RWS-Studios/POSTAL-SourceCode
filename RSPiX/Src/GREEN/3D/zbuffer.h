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
const int16_t ZB_MIN_Z = -32768;

class	RZBuffer // a 16-bit signed z-buffer
	{
public:
	int16_t m_sW;
	int16_t m_sH;
	int32_t m_lP; // pitch in WORDS! (Not a real pitch!)
	int16_t* m_pBuf; // for now, don't have great need for alignment!
	//----------------------------------------------
	void	Init();
	RZBuffer();
	RZBuffer(int16_t sW,int16_t sH);
	int16_t Create(int16_t sW,int16_t sH);
	~RZBuffer();
	int16_t Destroy();
	//----------------------------------------------
	void Clear(int16_t sVal = ZB_MIN_Z);
	//----------------------------------------------
	// debugging stuff
	int16_t* GetZPtr(int16_t sX,int16_t sY){return (m_pBuf + sX + m_lP*sY);}
	void TestHeight(RImage* pimDst,int16_t sDepth,
		int16_t sX,int16_t sY,int16_t sW,int16_t sH);
	void Dump(RImage* pimDst,int16_t sX,int16_t sY,int16_t sW,int16_t sH,uint8_t* pZCol);
	};




//================================================== 
//================================================== 
//================================================== 
 
//================================================== 
#endif
