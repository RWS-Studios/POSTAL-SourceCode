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
//////////////////////////////////////////////////////////////////////
//
// MULTIGRID.H
//
// Created on	03/23/97 JRD
// Implemented	03/23/97 JRD
//
//	History:
//
//		04/08/97 BRH	Fixed problems with include files
//
//		04/09/97 BRH	Added GetValueUncompressed for the utility which
//							added attributes from each layer.  It needed to
//							get the value fromt the previous layer and OR
//							them together.
//
//////////////////////////////////////////////////////////////////////

#ifndef MULTIGRID_H
#define MULTIGRID_H

#include "System.h"

#ifdef PATHS_IN_INCLUDES
	#include "BLUE/Blue.h"
	#include "ORANGE/File/file.h"
	#include "GREEN/Image/Image.h" // For Debugging only
#else
	#include "Blue.h"
	#include "file.h"
	#include "Image.h"
#endif // PATHS_IN_INCLUDES
 
 #define MULTIGRID_COOKIE "_MultiGrid_"
 #define MULTIGRID_CURRENT_VERSION 1

//////////////////////////////////////////////////////////////////////
//
// CURRENT CONSTRAINTS (03/23/97)
//
// - Only one level of grid hierarchy
// - Supports only 15-bit data
// - Coarse Grid scale must be in powers of two from (2 - 16384)
//
// BACKWARDS COMPATIBILITY (03/23/97)
//
// - Supports loading RAttribute Files up to version 4
//
// PLANNED ENHANCEMENTS
//
// - Template support for data of any type
// - Multiple hierarchical levels
// - Disjoint grids (hierarchy only where detail is needed)
//
//////////////////////////////////////////////////////////////////////
//
// TUNING PARAMETERS
//
// By turning off clipping, access speed is greatly enhanced
//
#define	MULTIGRID_CLIP	TRUE // undefine to turn off clip to world...
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//
// RMultiGrid class
//
// This class provides efficient, high speed compression of a 2d data
// field in a way that is transparent to the user.  It supports load,
// save, compress, and decompress within the class to facilitate
// utilities.
//
// It compresses by breaking a 2d data field into a coarse grid, and
// attempts to compress the data by 1) replicating tiles wherever
// possible, and 2) describing blocks which are all one value by a 
// single value, like a 2d run length encoding.
//
//////////////////////////////////////////////////////////////////////

class	RMultiGrid
	{
public:
	//////////////////////////////////////////////////////////////////////
	//  User members:
	//////////////////////////////////////////////////////////////////////

	int16_t	m_sWidth;	// With compression, you might get huge objects!
	int16_t	m_sHeight;

	//////////////////////////////////////////////////////////////////////
	//  User methods:
	//////////////////////////////////////////////////////////////////////

	// This inline does a high speed lookup into the compressed data.
	// It ONLY works AFTER the data has been compressed!
	//
	int16_t	GetVal(int16_t sX, int16_t sY,int16_t sClipVal = -1)
		{
		//-----------------------------------------------------------------
		ASSERT(m_sIsCompressed);

	#ifdef MULTIGRID_CLIP
		if ( (sX < 0) || (sY < 0) || (sX >= m_sWidth) || (sY >= m_sHeight) )
			return	sClipVal;
	#endif
		//-----------------------------------------------------------------

		int16_t sVal = *( m_ppsGridLines[sY] + (sX >> m_sShiftX) );
		if (sVal >=0) return sVal; 

		// Cache miss -> must look into a stored tile:
		//return -sVal; // For debugging
		return *( m_ppsTileList[-sVal] + m_psTileLine[ sY & m_sMaskY ]
						+ (sX & m_sMaskX) );
		}

	// If you wish to know the scale, you can get it from
	// the mask members:
	//
	int32_t	GetScale(int16_t sMask) // decode the mask
		{
		return ++sMask;
		}

	// If you wish to know the coarse grid dimensions:
	void	GetGridDimensions(int16_t *psW,int16_t *psH)
		{
		*psW = m_sWidth >> m_sShiftX;
		*psH = m_sHeight >> m_sShiftY;
		if ( m_sWidth & m_sMaskX ) *psW = *psW + 1;	// Partial tiles
		if ( m_sHeight & m_sMaskY ) *psH = *psH + 1;	// Partial tiles
		}

	// If you wish to know the enumber of unique blocks:
	int16_t	GetNumTiles()
		{
		ASSERT(m_sIsCompressed);

		int16_t sNumTiles = 0 ;// scan for the number of tiles:
		int16_t i,j;
		int16_t sGridW,sGridH;
		GetGridDimensions(&sGridW,&sGridH);

		for (j=0;j < sGridH;j++)
			{
			for (i=0;i < sGridW;i++)
				{
				int16_t sValue = *(m_ppsGridLines[j * (m_sMaskY + 1)] + i);
				if (sValue < 0) sNumTiles = MAX(sNumTiles,int16_t(-sValue));
				}
			}
		return sNumTiles + 1;
		}

	// Load a compressed data set from disk
	//
	int16_t Load(RFile* prFile);
	//////////////////////////////////////////////////////////////////////

	// Save a compressed data set to disk
	//
	int16_t Save(RFile* prFile);
	
	//////////////////////////////////////////////////////////////////////
	// These user methods are for initially creating the 2d data
	//////////////////////////////////////////////////////////////////////

	// Returns SUCCESS or FAILURE
	// Sets up the UNCOMPRESSED data
	//
	int16_t	Alloc(int16_t sW, int16_t sH);

	// UNCOMPRESSED ACCESS:
	//
	void	SetValueUncompressed(int16_t sVal,int16_t sX,int16_t sY)
		{
		ASSERT(m_psGrid);
		ASSERT(sVal >= 0);
		ASSERT(!m_sIsCompressed);
		ASSERT( (sX >=0) && (sY >= 0) && (sX < m_sWidth) && (sY < m_sHeight));

		*(m_psGrid + sX + int32_t(sY) * m_sWidth) = sVal;
		}

	// UNCOMPRESSED READ ACCESS
	//
	int16_t GetValueUncompressed(int16_t sX, int16_t sY)
		{
		ASSERT(m_psGrid);
		ASSERT(!m_sIsCompressed);
		ASSERT( (sX >= 0) && (sY >= 0) && (sX < m_sWidth) && (sY < m_sHeight));

		return *(m_psGrid + sX + int32_t(sY) * m_sWidth);
		}

	// A visual Debug View: (Uncompressed)
	//
	void	Dump(RImage* pimDst,int16_t sSrcX,int16_t sSrcY,int16_t sDstX,int16_t sDstY,
					int16_t sW,int16_t sH);

	// A visual Debug View: (Compressed)
	//
	void	DumpGrid(RImage* pimDst);

	// For Debugging:
	void	DumpData(RImage* pimDst);

	// For Debugging:
	void	DumpTiles(RImage* pimDst);

	//////////////////////////////////////////////////////////////////////
	// Performs Multigrid compression on the data in place
	// Optionally returns the compression statics:
	//
	// Returns FAILURE if memory could not be allocated,
	// or if compression did not succeed ( > 32k blocks were needed)
	//
	//////////////////////////////////////////////////////////////////////
	int16_t Compress(
		int16_t sTileW,				// Size of tiles to try on this data
		int16_t sTileH,
		int32_t* plSize = NULL,		// New Data Size (BYTES)
		int32_t* lNumBlocks = NULL,// Number of unique tiles needed
		int16_t sMatchSame = TRUE	// If false, NO TILE WILL BE REUSED
										// which increases the speed of compresion
		);

	//////////////////////////////////////////////////////////////////////
	// Returns the Data to an uncompressed state to try another
	// compression tile size.
	//////////////////////////////////////////////////////////////////////
	int16_t	Decompress();

	//////////////////////////////////////////////////////////////////////
	//  Internal members:
	//////////////////////////////////////////////////////////////////////
	int16_t	m_sIsCompressed;
	int16_t	m_sMaskX;	// Encodes the scale of the tiles
	int16_t m_sMaskY;
	int16_t m_sShiftX;
	int16_t m_sShiftY;	// For user convenience ONLY

	//////////////////////////////////////////////////////////////////////
	//  Internal methods:
	//////////////////////////////////////////////////////////////////////

	void	ClearUncompressed()	
		{
		m_sWidth = m_sHeight = 0;
		m_psGrid = NULL;
		}

	void	ClearCompressed()	
		{
		m_sIsCompressed = m_sMaskX = m_sMaskY = m_sShiftX = m_sShiftY = 0;
		m_psTiles = m_psTileLine = NULL;
		m_ppsGridLines = m_ppsTileList = NULL;
		}

	void	FreeCompressed() 
		{
		if (m_psTiles) free(m_psTiles); // This should be freed!
		if (m_psTileLine) free(m_psTileLine);
		if (m_ppsGridLines) free(m_ppsGridLines);
		if (m_ppsTileList) free(m_ppsTileList);

		ClearCompressed();
		}

	void	FreeUncompressed()
		{
		if (m_psGrid) free(m_psGrid);
		ClearUncompressed();
		}

	void	Free()
		{
		FreeCompressed();
		FreeUncompressed();
		}

	RMultiGrid()	
		{
		ClearCompressed();
		ClearUncompressed();
		}

	~RMultiGrid()
		{
		Free();
		}

	int16_t	AllocGrid(int16_t sScaleW, int16_t sScaleH);

	int32_t	MaskToShift(int16_t sMask)
		{
		int32_t	lShift = 0;
		int32_t	lValue = 1;
		int32_t	lMask = int32_t(sMask);

		while (lValue < lMask)
			{
			lShift++;
			lValue <<= 1;
			}

		return lShift;
		}

public:
	//////////////////////////////////////////////////////////////////////
	//  Data Structures
	//////////////////////////////////////////////////////////////////////
	int16_t*	m_psGrid; // Stores the grid data, compressed or uncompressed
	int16_t**	m_ppsGridLines;	// fast access into the grid lines
	int16_t*	m_psTiles;			// Stores the array of tiles
	int16_t**	m_ppsTileList;		// fast access into the tile array
	int16_t*	m_psTileLine;		// fast access into each tile line

	//////////////////////////////////////////////////////////////////////
	//  Statics:
	//////////////////////////////////////////////////////////////////////
	static	int16_t	ms_sShiftToMask[16];  // general conversion
	};


#endif //MULTIGRID_H

//////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////
