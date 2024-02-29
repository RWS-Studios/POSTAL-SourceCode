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
////////////////////////////////////////////////////////////////////////////////
//
// types3d.cpp
// Project: RSPiX/Green/3d
//
// History:
//		02/01/97 JRD	Started.
//
//		02/05/97 MJR	Filled in.
//
//		02/10/97 MJR	Removed the no-longer-necessary "long" casts for loading
//							and saving of RPixel32's.
//							Renamed RForm3d to RSop (Sea-Of-Points) since that's what
//							we all think of it as, so it may as well be called that.
//
//		10/06/99	JMI	Added Unmap() and Adjust().
//
// This module impliments the "high level" data types (containers) needed by the
// renderer.
//
////////////////////////////////////////////////////////////////////////////////

#include "types3d.h"


////////////////////////////////////////////////////////////////////////////////
// RTexture Functions
////////////////////////////////////////////////////////////////////////////////

// Allocate specified number of indices and colors
void RTexture::Alloc(short sNum)
	{
	m_sNum = sNum;
	AllocIndices();
	AllocColors();
	}


// Allocate same number of indices as current number of colors
void RTexture::AllocIndices(void)
	{
	FreeIndices();
	m_pIndices = (UCHAR*)calloc(m_sNum, 1);
	ASSERT(m_pIndices != 0);
	}


// Allocate same number of colors as current number of indices
void RTexture::AllocColors(void)
	{
	FreeColors();
	m_pColors = (RPixel32*)calloc(m_sNum, sizeof(RPixel32));
	ASSERT(m_pColors != 0);
	}


// Free indices and colors
void RTexture::Free(void)
	{
	FreeIndices();
	FreeColors();
	Init();
	}


// Free indices only
void RTexture::FreeIndices(void)
	{
	if (m_pIndices)
		{
		free(m_pIndices);
		m_pIndices = 0;
		}
	}


// Free colors only
void RTexture::FreeColors(void)
	{
	if (m_pColors)
		{
		free(m_pColors);
		m_pColors = 0;
		}
	}


short RTexture::Load(RFile* fp)
	{
	Free();

	short sResult = 0;
	if (fp->Read(&m_sNum) == 1)
		{
		short sFlags;
		if (fp->Read(&sFlags) == 1)
			{
			if (sFlags & HasIndices)
				{
				AllocIndices();
				fp->Read(m_pIndices, m_sNum);
				}
			if (sFlags & HasColors)
				{
				AllocColors();
				fp->Read(m_pColors, m_sNum);
				}
			}
		}

	if (fp->Error())
		{
		sResult = -1;
		TRACE("RTexture::Load(): Error reading from file!\n");
		}
	return sResult;
	}


short RTexture::Save(RFile* fp)
	{
	short sResult = 0;

	fp->Write(&m_sNum);

	short sFlags = 0;
	if (m_pIndices)
		sFlags |= HasIndices;
	if (m_pColors)
		sFlags |= HasColors;
	fp->Write(&sFlags);

	if (m_pIndices)
		fp->Write(m_pIndices, m_sNum);
	if (m_pColors)
		fp->Write(m_pColors, m_sNum);

	if (fp->Error())
		{
		sResult = -1;
		TRACE("RTexture::Save(): Error writing to file!\n");
		}
	return sResult;
	}


// Map colors onto the specified palette.  For each color, the best
// matching color is found in the  palette, and the associated palette
// index is written to the array of indices.  If the array of indices
// doesn't exist, it will be created.
void RTexture::Remap(
	short sStartIndex,
	short sNumIndex,
	UCHAR* pr,
	UCHAR* pg,
	UCHAR* pb,
	long linc)
	{
	ASSERT(m_pColors);
		
	if (m_pIndices == 0)
		AllocIndices();

	for (short i = 0; i < m_sNum; i++)
		{
		m_pIndices[i] = rspMatchColorRGB(
			long(m_pColors[i].u8Red),
			long(m_pColors[i].u8Green),
			long(m_pColors[i].u8Blue),
			sStartIndex,sNumIndex,
			pr,pg,pb,linc);
		}
	}


////////////////////////////////////////////////////////////////////////////////
// Unmap colors from the specified palette and put them into the colors
// array.  If the array of colors doesn't exist, it will be created.
////////////////////////////////////////////////////////////////////////////////
void 
RTexture::Unmap(
	UCHAR* pr,
	UCHAR* pg,
	UCHAR* pb,
	long lInc)
	{
	ASSERT(m_pIndices);
		
	if (m_pColors == 0)
		AllocColors();

	U8*			pu8	= m_pIndices;
	RPixel32*	ppix	= m_pColors;
	short	sCount		= m_sNum;
	while (sCount--)
		{
		ppix->u8Red		= pr[*pu8];
		ppix->u8Green	= pg[*pu8];
		ppix->u8Blue	= pb[*pu8];

		ppix++;
		pu8++;
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Muddy or brighten or darken.  Applies the specified brightness value
// to every nth color (where n == lInc).
////////////////////////////////////////////////////////////////////////////////
void
RTexture::Adjust(
	float fAdjustment,	// In:  Adjustment factor (1.0 == same, < 1 == dimmer, > 1 == brighter).
	long lInc)				// In:  Number of colors to skip.
	{
	ASSERT(m_pColors);
	ASSERT(fAdjustment >= 0.0f);

#define CLAMP255(u8Color, fColor)	( (u8Color) = ( (fColor) < 255) ? (fColor) + 0.5f : 255)

	RPixel32*	ppix	= m_pColors;
	short	sCount		= m_sNum / lInc;
	float	fColor;
	while (sCount--)
		{
		fColor	= ppix->u8Red		* fAdjustment;
		CLAMP255(ppix->u8Red, fColor);

		fColor	= ppix->u8Green	* fAdjustment;
		CLAMP255(ppix->u8Green, fColor);

		fColor	= ppix->u8Blue		* fAdjustment;
		CLAMP255(ppix->u8Blue, fColor);

		ppix += lInc;
		}
	}

////////////////////////////////////////////////////////////////////////////////
// RMesh Functions
////////////////////////////////////////////////////////////////////////////////
void RMesh::Alloc(short sNum)
	{
	Free();
	m_sNum = sNum;
	m_pArray = (USHORT*)calloc((long)m_sNum * 3, sizeof(USHORT));
	ASSERT(m_pArray != 0);
	}


void RMesh::Free(void)
	{
	if (m_pArray)
		free(m_pArray);
	Init();
	}


short RMesh::Load(RFile* fp)
	{
	Free();

	short sResult = 0;
	if (fp->Read(&m_sNum) == 1)
		{
		Alloc(m_sNum);
		fp->Read(m_pArray, (long)m_sNum * 3);
		}
	if (fp->Error())
		{
		sResult = -1;
		TRACE("RMesh::Load(): Error reading from file!\n");
		}
	return sResult;
	}


short RMesh::Save(RFile* fp)
	{
	short sResult = 0;
	fp->Write(&m_sNum);
	fp->Write(m_pArray, (long)m_sNum * 3);
	if (fp->Error())
		{
		sResult = -1;
		TRACE("RMesh::Save(): Error writing to file!\n");
		}
	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// RSop Functions
////////////////////////////////////////////////////////////////////////////////
void RSop::Alloc(long lNum)
	{
	Free();
	m_lNum = lNum;
	m_pArray = (RP3d*)calloc(m_lNum, sizeof(RP3d));
	ASSERT(m_pArray != 0);
	}


void RSop::Free(void)
	{
	if (m_pArray)
		free(m_pArray);
	Init();
	}


short RSop::Load(RFile* fp)
	{
	Free();

	short sResult = 0;
	if (fp->Read(&m_lNum) == 1)
		{
		Alloc(m_lNum);
		ASSERT(sizeof(RP3d) == (sizeof(REAL) * 4));
		fp->Read((REAL*)m_pArray, m_lNum * 4);
		}
	if (fp->Error())
		{
		sResult = -1;
		TRACE("RSop::Load(): Error reading from file!\n");
		}
	return sResult;
	}


short RSop::Save(RFile* fp)
	{
	short sResult = 0;
	fp->Write(&m_lNum);
	ASSERT(sizeof(RP3d) == (sizeof(REAL) * 4));
	fp->Write((REAL*)m_pArray, m_lNum * 4);
	if (fp->Error())
		{
		sResult = -1;
		TRACE("RSop::Save(): Error writing to file!\n");
		}
	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
