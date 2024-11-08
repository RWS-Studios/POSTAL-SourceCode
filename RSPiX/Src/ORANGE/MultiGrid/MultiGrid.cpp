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
// MULTIGRID.CPP
//
// Created on	03/23/97 JRD
// Implemented	03/23/97 JRD
//
//		05/08/97	JMI	Added #include <string.h> for strcmp*().  I guess
//							that, in VC <= 4.1, the strcmp*() protos are not
//							in stdlib.h.
//
//		06/24/97 	MJR	Switched to rspStricmp() for mac compatibility.
//						Also changed a few constants to longs instead
//						of ints so that MIN() would work (real strict
//						on mac).
//
//////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <string.h>
#include "System.h"	

#ifdef PATHS_IN_INCLUDES
	#include "ORANGE/MultiGrid/MultiGrid.h"
	#include "ORANGE/str/str.h"
#else
	#include "MULTIGRID.H"
	#include "str.h"
#endif	//PATHS_IN_INCLUDES

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

//////////////////////////////////////////////////////////////////////
//  Static Members:
//////////////////////////////////////////////////////////////////////
int16_t	RMultiGrid::ms_sShiftToMask[16] = 
	{0,1,3,7,15,31,63,127,255,511,1023,2047,4095,8191,16383};

//////////////////////////////////////////////////////////////////////
//  Internal Support Methods:
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
//  User Methods:
//////////////////////////////////////////////////////////////////////

// For Debugging
void	RMultiGrid::Dump(RImage* pimDst,int16_t sSrcX,int16_t sSrcY,int16_t sDstX,int16_t sDstY,
					int16_t sW,int16_t sH)
	{
	int16_t i,j;

	ASSERT(pimDst);
	ASSERT(m_psGrid);
	ASSERT(!m_sIsCompressed);

	for (j=0;j < sH; j++)
		for (i=0;i< sW;i++)
			{
			int16_t sValue = m_psGrid[i + sSrcX + (j + sSrcY)*int32_t(m_sWidth)];
			*(pimDst->m_pData + 2*(i + sDstX) + (j + sDstY)*pimDst->m_lPitch) = uint16_t(sValue)>>8;
			*(pimDst->m_pData + 2*(i + sDstX) + (j + sDstY)*pimDst->m_lPitch+1) = sValue&0xff;
			}
	}

// For Debugging
void	RMultiGrid::DumpGrid(RImage* pimDst)
	{
	int16_t i,j;

	ASSERT(pimDst);
	ASSERT(m_psGrid);
	ASSERT(m_sIsCompressed);

	int16_t sGridW;
	int16_t sGridH;

	GetGridDimensions(&sGridW,&sGridH);

	for (j=0; j < sGridH; j++)
		for (i=0; i < sGridW; i++)
			{
			int16_t sValue = *(m_ppsGridLines[j * (m_sMaskY + 1)] + i);

			*(pimDst->m_pData + 2*(i) + (j)*pimDst->m_lPitch) = uint16_t(sValue)>>8;
			*(pimDst->m_pData + 2*(i) + (j)*pimDst->m_lPitch+1) = sValue&0xff;
			}
	}

// For Debugging:
void	RMultiGrid::DumpData(RImage* pimDst)
	{
	int16_t i,j;

	ASSERT(pimDst);
	ASSERT(m_psGrid);
	ASSERT(m_sIsCompressed);

	for (j=0;j < m_sHeight; j ++)
		for (i=0;i< m_sWidth; i++)
			{
			int16_t sValue = GetVal(i,j);

			*(pimDst->m_pData + 2*(i) + (j)*pimDst->m_lPitch) = uint16_t(sValue)>>8;
			*(pimDst->m_pData + 2*(i) + (j)*pimDst->m_lPitch+1) = sValue&0xff;
			}
	}

void	RMultiGrid::DumpTiles(RImage* pimDst)
	{
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

	TRACE("# of tiles = %hd\n",++sNumTiles);

	for (i=0;i < sNumTiles;i++)
		{

		}
	}

//////////////////////////////////////////////////////////////////////
//
//  Alloc
//
//  Create space for an uncompressed grid:
//  Will overwrite old psGrid, so save it first!
//
//////////////////////////////////////////////////////////////////////

int16_t	RMultiGrid::Alloc(int16_t sW, int16_t sH)
	{
#ifdef	_DEBUG
	if (m_sIsCompressed)
		{
		TRACE("RMultiGrid::Alloc: You've already allocated a grid!\n");
		return FAILURE;
		}
#endif

	if (!(m_psGrid = (int16_t*) calloc(sizeof(int16_t),int32_t(sW)*sH) )) return FAILURE;

	m_sWidth = sW;
	m_sHeight = sH;

	return SUCCESS;
	}

//////////////////////////////////////////////////////////////////////
//
//  AllocGrid
//
//  Create space for the compressed grid:
//
//////////////////////////////////////////////////////////////////////

int16_t	RMultiGrid::AllocGrid(int16_t sScaleW, int16_t sScaleH)
	{
	// Set the parameters:
	//--------------------------------------------- Convert Input to log 2
	int16_t sValue = 16384,sDigit = 14;
	int16_t sLogW,sLogH;
	int16_t i,j;

	while (sScaleW > 0)
		{
		if (sScaleW >= sValue)
			{
		#ifdef _DEBUG
			if (sValue != sScaleW)
				{
				TRACE("RMultiGrid::AllocGrid: WARNING - block size not pow2, truncating.\n");
				}
		#endif
			sScaleW = sValue;
			sLogW = sDigit;
			break;
			}
		sValue >>= 1;
		sDigit--;
		}

	sValue = 16384,sDigit = 14;
	while (sValue > 0)
		{
		if (sScaleH >= sValue)
			{
		#ifdef _DEBUG
			if (sValue != sScaleH)
				{
				TRACE("RMultiGrid::AllocGrid: WARNING - block size not pow2, truncating.\n");
				}
		#endif
			sScaleH = sValue;
			sLogH = sDigit;
			break;
			}

		sValue >>= 1;
		sDigit--;
		}

	//--------------------------------------------- Set the masks:
	m_sShiftX = sLogW;
	m_sShiftY = sLogH;
	m_sMaskX = sScaleW-1;	
	m_sMaskY = sScaleH-1;	


	//--------------------------------------------- Allocate the coarse grid
	int16_t sGridW,sGridH;
	GetGridDimensions(&sGridW,&sGridH);
	int16_t	sFullHeight = sGridH * sScaleH;

	int16_t*	psUncompressed = m_psGrid;		// Save old!

	//--------------------------------------------- Allocate the Tile Lists:
	int32_t	lShortTileSize = int32_t(sScaleW) * sScaleH;
	int32_t	lByteTileSize = lShortTileSize << 1;
	int32_t	lLongTileSize = lShortTileSize >> 1;
	// Initial Max
	int16_t		sMaxNumTiles = MIN((int32_t)32767, (int32_t)1 + int32_t(sGridW) * (int32_t)sGridH);

	if (!(m_psTiles = (int16_t*) calloc(lByteTileSize,sMaxNumTiles ) )) return FAILURE;

	//---------------------------------------------Add in the random access:
	int16_t *psTile = m_psTiles;
	if (!(m_ppsTileList = (int16_t**) calloc(sizeof(int16_t*),sMaxNumTiles ) )) return FAILURE;
	for (i=0; i < sMaxNumTiles; i++,psTile += lShortTileSize) m_ppsTileList[i] = psTile;

	int16_t sOff = 0;
	if (!(m_psTileLine = (int16_t*) calloc(sizeof(int16_t),sScaleH ) )) return FAILURE;
	for (i=0;i < sScaleH;i++,sOff += sScaleW) m_psTileLine[i] = sOff;

	//--------------------------------------------- Allocate the coarse grid

	if (!(m_psGrid = (int16_t*) calloc(sizeof(int16_t),int32_t(sGridW)*sGridH) )) return FAILURE;

	//--------------------------------------------- Add in the random access:

	if (!(m_ppsGridLines = (int16_t**) calloc(sizeof(int16_t*),sFullHeight ) )) return FAILURE;
	for (i=0; i < sFullHeight; i++) m_ppsGridLines[i] = m_psGrid + (i >> m_sShiftY)*sGridW;

	//---------------------------------------------  Populate the coarse grid:

	int16_t	*psSrc,*psSrcLine = psUncompressed;
	int32_t	lSrcSkip = int32_t(m_sWidth)*sScaleH;

	for (j=0;j < sFullHeight;j += sScaleH,psSrcLine += lSrcSkip)
		{
		psSrc = psSrcLine;
		for (i=0;i < sGridW;i++,psSrc += sScaleW)
			{
			*(m_ppsGridLines[j] + i) = *psSrc;
			}
		}

	return SUCCESS;
	}


//////////////////////////////////////////////////////////////////////
// Performs Multigrid compression on the data in place
// Optionally returns the compression statics:
//
// Returns FAILURE if memory could not be allocated,
// or if compression did not succeed ( > 32k blocks were needed)
//
//////////////////////////////////////////////////////////////////////

int16_t RMultiGrid::Compress(
	int16_t sTileW,			// Size of tiles to try on this data
	int16_t sTileH,
	int32_t* plSize,			// New Data Size (BYTES)
	int32_t* plNumBlocks,		// Number of unique tiles needed
	int16_t sMatchSame		// If false, NO TILE WILL BE REUSED
								// which increases the speed of compresion
	)
	{
	ASSERT(sTileW > 0);
	ASSERT(sTileH > 0);

#ifdef _DEBUG

	if (!m_psGrid)
		{
		TRACE("RMultiGrid::Compress: Gechyerself a map first, you unbelievably flaming bastard!\n");
		return -1;
		}

	if (m_ppsTileList || m_sIsCompressed)
		{
		TRACE("RMultiGrid::Compress: Uncompress it first, you unbelievably flaming bastard!\n");
		return -1;
		}

#endif

	int16_t *psUncompressedData = m_psGrid;

	if (AllocGrid(sTileW,sTileH) )	// alloc error
		{
		TRACE("RMultiGrid::Compress: Out of memory!\n");
		if (m_psGrid) free(m_psGrid);
		m_psGrid = psUncompressedData;	// restore old data
		ClearCompressed();
		return FAILURE;
		}

	// Now, attempt to compress things into blocks:
	// First, do only the integral blocks:
	int16_t sBlockX,sBlockY,sFullY,i,j;
	int16_t sGridVal,sVal;
	int16_t sTileNumber = 1; // cannot use -0 as a valid offset!

	int16_t sGridW,sGridH;
	GetGridDimensions(&sGridW,&sGridH);

	int16_t sWholeGridW = sGridW - 1; // guaranteed to be whole.
	int16_t	sWholeGridH = sGridH - 1;

	int16_t	sExtraW = m_sWidth - (sWholeGridW << m_sShiftX);
	int16_t	sExtraH = m_sHeight - (sWholeGridH << m_sShiftY);

	int16_t sMaxTile = MIN((int32_t)32767, (int32_t)1 + int32_t(sGridW) * (int32_t)sGridH);
	int16_t	sShortSize = (m_sMaskX+1)*(m_sMaskY+1);
	int16_t sNumMatches = 0;

	int16_t sScanH = m_sMaskY + 1;
	for (sFullY = 0,sBlockY = 0; sBlockY < sGridH; sBlockY++,sFullY += m_sMaskY + 1)
		{
		if (sBlockY == sWholeGridH) sScanH = sExtraH;

		int16_t sScanW = m_sMaskX + 1;
		for (sBlockX = 0; sBlockX < sGridW; sBlockX ++)
			{
			if (sBlockX == sWholeGridW) sScanW = sExtraW;

			sGridVal = *(m_ppsGridLines[sFullY] + sBlockX);
			int16_t* psSrcBlock = psUncompressedData + int32_t(sBlockX) * (m_sMaskX + 1) +
				int32_t(sFullY) * m_sWidth;

			//*********************************************** ANALIZE BLOCK
			int16_t sMatch = 1; // homogeneous block?

			// do slow, shameful way for now...
			// Need to realloc later!
			for (j = 0; j < sScanH; j++)
				{
				for (i = 0; i < sScanW; i++) // only copy what's needed...
					{
					// copy into temp block:
					sVal = psSrcBlock[i + j * int32_t(m_sWidth)];
					*(m_ppsTileList[sTileNumber] + m_psTileLine[ j ] + i) = sVal;
					if (sVal != sGridVal) sMatch = 0; // was not homogeneous
					}
				}

			int16_t sFound = 0;
			if (!sMatch) // store another unique block
				{
				// ADD IN SCAN TO SEE IF YOU MATCH ANY EXISTING BLOCKS>
				if (sMatchSame)
					{
					int16_t k;

					if ((sBlockX != sWholeGridW) && (sBlockY != sWholeGridH)) // do a quick compare:
						{
						for (k=0;k < sTileNumber;k++)
							{
							int16_t sComp = 1;
							// compare last tile to current:
							for (int16_t l=0;l < sShortSize;l++)
								{
								if (*(m_ppsTileList[k] + l) != *(m_ppsTileList[sTileNumber] + l))
									{
									sComp = 0;
									break;
									}
								}

							if (sComp) // matched an existing tile!
								{
								*(m_ppsGridLines[sFullY] + sBlockX) = -k;
								sFound = 1;
								sNumMatches++;
								break;
								}
							}
						}
					else // only compare the pixels in question:
						{
						for (k=0;k < sTileNumber;k++)
							{
							int16_t sComp = 1;
							for (j = 0; j < sScanH; j++)
								{
								for (i = 0; i < sScanW; i++) // only compare what's needed...
									{
									if (*(m_ppsTileList[k] + m_psTileLine[ j ] + i) != 
										*(m_ppsTileList[sTileNumber] + m_psTileLine[ j ] + i) )
										{
										sComp = 0;
										break;
										}
									}
								}

							if (sComp) // matched an existing tile!
								{
								*(m_ppsGridLines[sFullY] + sBlockX) = -k;
								sFound = 1;
								sNumMatches++;
								break;
								}
							}
						}

					}

				// NEED a new block!
				if (!sFound) 
					{
					*(m_ppsGridLines[sFullY] + sBlockX) = -sTileNumber;
					sTileNumber++;
					if (sTileNumber == 32767)
						{
						TRACE("Compression overflow! > 32K tiles needed!\n");
						if (m_psGrid) free(m_psGrid);
						m_psGrid = psUncompressedData;	// restore old data
						FreeCompressed();
						ClearCompressed();
						return FAILURE;
						}
					}
				}
			
			//*********************************************** 
			}
		}

	// Set to new compressed state:
	m_sIsCompressed = TRUE;
	free(psUncompressedData);	// Lose the uncompressed format

	// Reallocate the list of blocks to the known size:
	int16_t *psOldBlocks = m_psTiles;
	int32_t	lLen = sShortSize * sizeof(int16_t) * sTileNumber;

	m_psTiles = (int16_t*) malloc(lLen);

	memcpy(m_psTiles,psOldBlocks,lLen);
	free(psOldBlocks);

	// Reset the random access
	psOldBlocks = m_psTiles;
	for (i=0; i < sTileNumber; i++,psOldBlocks += sShortSize) m_ppsTileList[i] = psOldBlocks;

	if (plNumBlocks) *plNumBlocks = --sTileNumber;
	if (plSize) *plSize = int32_t(sTileNumber) * (int32_t(sShortSize) * sizeof(int16_t) + sizeof(int16_t*))
		+ int32_t(sGridH) * (sGridW * sizeof(int16_t) + sizeof(int16_t*) );

	return SUCCESS;
	}

// try again!
int16_t RMultiGrid::Decompress()
	{
#ifdef _DEBUG
	if (!m_sIsCompressed)
		{
		TRACE("RMultiGrid::Decompress:  Compress it first, you silly silly man!\n");
		return -1;
		}
#endif

	int16_t *psNewGrid = (int16_t*) calloc(sizeof(int16_t),int32_t(m_sWidth)*m_sHeight);

	if (!psNewGrid) return -1; // allocation error

	// Draw into the new grid:
	int16_t i,j;

	for (j=0;j < m_sHeight;j++)
		{
		for (i=0;i < m_sWidth;i++)
			{
			psNewGrid[i + int32_t(j)*m_sWidth ] = GetVal(i,j);
			}
		}

	// Restore to uncompressed state:
	m_sIsCompressed = 0;
	FreeCompressed();
	ClearCompressed();

	free(m_psGrid);
	m_psGrid = psNewGrid; // Install it...

	return SUCCESS;
	}


//////////////////////////////////////////////////////////////////////
// 
// Save
//
// Returns FAILURE or SUCCESS
// Will currently only save a compressed Multigrid
//
//////////////////////////////////////////////////////////////////////

int16_t RMultiGrid::Save(RFile* fp)
	{
	ASSERT(fp);

	if (!m_sIsCompressed)
		{
		TRACE("MultiGrid::Save: Cannot currently save uncompressed MultiGrids.  Sorry.\n");
		return FAILURE;
		}
	
	fp->Write(MULTIGRID_COOKIE);
	fp->Write(int16_t(MULTIGRID_CURRENT_VERSION));

	fp->Write(m_sWidth);
	fp->Write(m_sHeight);
	fp->Write(m_sIsCompressed); // ASSUME IT IS COMPRESSED!
	fp->Write(m_sMaskX);
	fp->Write(m_sMaskY);
	fp->Write(int16_t(sizeof(int16_t))); // For future expansion!

	int16_t sGridW,sGridH;
	GetGridDimensions(&sGridW,&sGridH);

	fp->Write(sGridW);
	fp->Write(sGridH);

	int16_t sNumTiles = GetNumTiles();
	fp->Write(sNumTiles);

	fp->Write(int32_t(0)); // Reserved1
	fp->Write(int32_t(0)); // Reserved2
	fp->Write(int32_t(0)); // Reserved3
	fp->Write(int32_t(0)); // Reserved4

	// Write out the grid of shorts:
	int16_t i,j;

	for (j=0;j < sGridH; j++)
		for (i=0;i<sGridW;i++)
			fp->Write(*(m_ppsGridLines[j<<m_sShiftY] + i));

	// Write out the tiles, omitting the zeroth black tile.
	int16_t sTileLen = (m_sMaskX + 1) * (m_sMaskY + 1);

	for (i=1;i < sNumTiles;i++)
		{
		fp->Write(m_ppsTileList[i],sTileLen);
		}

	return SUCCESS;
	}


//////////////////////////////////////////////////////////////////////
// 
// Load
//
// Returns FAILURE or SUCCESS
// Will currently only load a compressed Multigrid, version I
//
//////////////////////////////////////////////////////////////////////

int16_t RMultiGrid::Load(RFile* fp)
	{
	ASSERT(fp);

	if (m_psGrid)
		{
		TRACE("MultiGrid::Load: Cannot load on top of an existing grid!\n");
		return FAILURE;
		}
	
	int16_t sVal;
	char	string[20];

	fp->Read(&string[0]);
	if (rspStricmp(string,MULTIGRID_COOKIE))
		{
		TRACE("MultiGrid::Load: Not a MultiGrid File!\n");
		return FAILURE;
		}

	fp->Read(&sVal);
	if (sVal != MULTIGRID_CURRENT_VERSION)
		{
		TRACE("MultiGrid::Load: I don't support this version (%hd).\n",sVal);
		return FAILURE;
		}

	// Let's JAM!
	fp->Read(&m_sWidth);
	fp->Read(&m_sHeight);
	fp->Read(&m_sIsCompressed); // should be compressed
	fp->Read(&m_sMaskX);
	fp->Read(&m_sMaskY);

	int16_t sCodeSize;
	fp->Read(&sCodeSize); // normally 2 for short.

	int16_t sGridW,sGridH;

	fp->Read(&sGridW);
	fp->Read(&sGridH);

	// ALLOCATE IT
	m_psGrid = (int16_t*) calloc(sCodeSize,int32_t(sGridW)*sGridH);
	if (!m_psGrid)
		{
		TRACE("MultiGrid::Load: Out of Memory!!!!\n");
		return FAILURE;
		}

	int16_t sNumTiles;
	fp->Read(&sNumTiles);
	int16_t sTileLen = (m_sMaskX + 1) * (m_sMaskY + 1);

	// Allocate the tiles:
	// This appears as a memory leak!
	if (!(m_psTiles = (int16_t*)calloc(sTileLen * sCodeSize,sNumTiles)))
		{
		free(m_psGrid);
		TRACE("MultiGrid::Load: Out of Memory!!!!\n");
		return FAILURE;
		}

	int32_t lVal;
	fp->Read(&lVal); // Reserved1
	fp->Read(&lVal); // Reserved2
	fp->Read(&lVal); // Reserved3
	fp->Read(&lVal); // Reserved4

	int16_t i,j;

	// Must generate the shift members to use them now!
	m_sShiftX = MaskToShift(m_sMaskX);
	m_sShiftY = MaskToShift(m_sMaskY);

	// Set up random access for grid:
	int16_t	sFullHeight = sGridH * (m_sMaskY + 1);
	if (!(m_ppsGridLines = (int16_t**) calloc(sizeof(int16_t*),sFullHeight ) )) return FAILURE;
	for (i=0; i < sFullHeight; i++) m_ppsGridLines[i] = m_psGrid + (i >> m_sShiftY)*sGridW;

	// Read in the grid of shorts:
	for (j=0;j < sGridH; j++)
		for (i=0;i<sGridW;i++)
			fp->Read((m_ppsGridLines[j<<m_sShiftY] + i));

	// Set up random access for tiles:
	int16_t *psTile = m_psTiles;
	if (!(m_ppsTileList = (int16_t**) calloc(sizeof(int16_t*),sNumTiles ) )) return FAILURE;
	for (i=0; i < sNumTiles; i++,psTile += sTileLen) m_ppsTileList[i] = psTile;

	// Read in the tiles, omitting the zeroth black tile.

	for (i=1;i < sNumTiles;i++)
		{
		fp->Read(m_ppsTileList[i],sTileLen);
		}

	// Finish up thae random access:
	int16_t sOff = 0;
	if (!(m_psTileLine = (int16_t*) calloc(sCodeSize,m_sMaskY+1 ) )) return FAILURE;
	for (i=0;i < m_sMaskY+1;i++,sOff += m_sMaskX + 1) m_psTileLine[i] = sOff;

	// Set the shift values:

	return SUCCESS;
	}
