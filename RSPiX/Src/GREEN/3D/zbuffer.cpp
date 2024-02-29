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
#include <stdlib.h>
#include "System.h"
#ifdef PATHS_IN_INCLUDES
	#include "GREEN/3D/zbuffer.h"
	#include "ORANGE/QuickMath/FixedPoint.h"
#else
	#include "zbuffer.h"
	#include "fixedpoint.h"
#endif

//************************ RZBuffer
void	RZBuffer::Init()
	{
	m_sW = m_sH = 0;
	m_lP = 0;
	m_pBuf = NULL;
	}

RZBuffer::RZBuffer()
	{
	Init();
	}

RZBuffer::RZBuffer(short sW,short sH)
	{
	Init();
	Create(sW,sH);
	}

short RZBuffer::Create(short sW,short sH)
	{
	if (m_pBuf)
		{
		TRACE("RZBuffer::Create: already exists!\n");
		return -1;
		}

	m_sW = sW;
	m_sH = sH;
	m_lP = long((sW+3)&~3); // keep for emergencies!

	long lSize = m_lP * m_sH * sizeof(short);
	m_pBuf = (short*) malloc(lSize);
	// you then may clear it, buddy!

	return 0;
	}

RZBuffer::~RZBuffer()
	{
	Destroy();
	}

short RZBuffer::Destroy()
	{
	if (!m_pBuf)
		{
		TRACE("RZBuffer::Destroy: already deleted!\n");
		return -1;
		}

	free(m_pBuf);

	Init();
	return 0;
	}

//----------------------------------------------
void RZBuffer::Clear(short sVal)
	{
	// Do by 64-bit longs
	// 1) Create the 64-bit long
	union
		{
		U64 word;
		struct
			{
			S16 p1;
			S16 p2;
			S16 p3;
			S16 p4;
			};
		} BigWord;

	BigWord.p1 = BigWord.p2 = BigWord.p3 = BigWord.p4 = sVal;

	//2) Do the copy
	long lWordP = m_lP >> 2; // 4 * 16 = 64
	U64* pWord = (U64*) m_pBuf;

	long lWordLen = lWordP * m_sH;
	for (long i=0;i < lWordLen; i++) *pWord++ = BigWord.word;

	}

//----------------------------------------------
// debugging stuff
void RZBuffer::TestHeight(RImage* pimDst,short sDepth,
				short sX,short sY,short sW,short sH)
	{
	short i,j;
	long lP = pimDst->m_lPitch;

	UCHAR *pDst,*pDstLine = pimDst->m_pData + lP * sY + sX;
	short *pZB,*pZLine = m_pBuf + m_lP * sY + sX;

	for (j=0;j<sH;j++,pDstLine += lP,pZLine += m_lP )
		{
		pDst = pDstLine;
		pZB = pZLine;
		for (i=0;i<sW;i++,pDst++,*pZB++)
			{
			if (*pZB < sDepth)
				*pDst = 255; // draw it!
			}
		}
	}

void RZBuffer::Dump(RImage* pimDst,short sX,short sY,short sW,short sH,UCHAR* pZCol)
	{
	short i,j;
	long lP = pimDst->m_lPitch;

	UCHAR *pDst,*pDstLine = pimDst->m_pData + lP * sY + sX;
	short *pZLine = m_pBuf + m_lP * sY + sX;
	RFixedS16 *pZB;


	for (j=0;j<sH;j++,pDstLine += lP,pZLine += m_lP )
		{
		pDst = pDstLine;
		pZB = (RFixedS16*)pZLine;
		for (i=0;i<sW;i++,pZB++)
			{
			*pDst++ = pZCol[(*pZB).mod + 128]; // color code it!
			}
		}
	}
