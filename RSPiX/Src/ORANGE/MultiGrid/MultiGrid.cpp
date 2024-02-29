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
short	RMultiGrid::ms_sShiftToMask[16] = 
	{0,1,3,7,15,31,63,127,255,511,1023,2047,4095,8191,16383};

//////////////////////////////////////////////////////////////////////
//  Internal Support Methods:
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
//  User Methods:
//////////////////////////////////////////////////////////////////////

// For Debugging
void	RMultiGrid::Dump(RImage* pimDst,short sSrcX,short sSrcY,short sDstX,short sDstY,
					short sW,short sH)
	{
	short i,j;

	ASSERT(pimDst);
	ASSERT(m_psGrid);
	ASSERT(!m_sIsCompressed);

	for (j=0;j < sH; j++)
		for (i=0;i< sW;i++)
			{
			short sValue = m_psGrid[i + sSrcX + (j + sSrcY)*long(m_sWidth)];
			*(pimDst->m_pData + 2*(i + sDstX) + (j + sDstY)*pimDst->m_lPitch) = USHORT(sValue)>>8;
			*(pimDst->m_pData + 2*(i + sDstX) + (j + sDstY)*pimDst->m_lPitch+1) = sValue&0xff;
			}
	}

// For Debugging
void	RMultiGrid::DumpGrid(RImage* pimDst)
	{
	short i,j;

	ASSERT(pimDst);
	ASSERT(m_psGrid);
	ASSERT(m_sIsCompressed);

	short sGridW;
	short sGridH;

	GetGridDimensions(&sGridW,&sGridH);

	for (j=0; j < sGridH; j++)
		for (i=0; i < sGridW; i++)
			{
			short sValue = *(m_ppsGridLines[j * (m_sMaskY + 1)] + i);

			*(pimDst->m_pData + 2*(i) + (j)*pimDst->m_lPitch) = USHORT(sValue)>>8;
			*(pimDst->m_pData + 2*(i) + (j)*pimDst->m_lPitch+1) = sValue&0xff;
			}
	}

// For Debugging:
void	RMultiGrid::DumpData(RImage* pimDst)
	{
	short i,j;

	ASSERT(pimDst);
	ASSERT(m_psGrid);
	ASSERT(m_sIsCompressed);

	for (j=0;j < m_sHeight; j ++)
		for (i=0;i< m_sWidth; i++)
			{
			short sValue = GetVal(i,j);

			*(pimDst->m_pData + 2*(i) + (j)*pimDst->m_lPitch) = USHORT(sValue)>>8;
			*(pimDst->m_pData + 2*(i) + (j)*pimDst->m_lPitch+1) = sValue&0xff;
			}
	}

void	RMultiGrid::DumpTiles(RImage* pimDst)
	{
	short sNumTiles = 0 ;// scan for the number of tiles:

	short i,j;
	short sGridW,sGridH;
	GetGridDimensions(&sGridW,&sGridH);

	for (j=0;j < sGridH;j++)
		{
		for (i=0;i < sGridW;i++)
			{
			short sValue = *(m_ppsGridLines[j * (m_sMaskY + 1)] + i);
			if (sValue < 0) sNumTiles = MAX(sNumTiles,short(-sValue));
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

short	RMultiGrid::Alloc(short sW, short sH)
	{
#ifdef	_DEBUG
	if (m_sIsCompressed)
		{
		TRACE("RMultiGrid::Alloc: You've already allocated a grid!\n");
		return FAILURE;
		}
#endif

	if (!(m_psGrid = (short*) calloc(sizeof(short),long(sW)*sH) )) return FAILURE;

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

short	RMultiGrid::AllocGrid(short sScaleW, short sScaleH)
	{
	// Set the parameters:
	//--------------------------------------------- Convert Input to log 2
	short sValue = 16384,sDigit = 14;
	short sLogW,sLogH;
	short i,j;

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
	short sGridW,sGridH;
	GetGridDimensions(&sGridW,&sGridH);
	short	sFullHeight = sGridH * sScaleH;

	short*	psUncompressed = m_psGrid;		// Save old!

	//--------------------------------------------- Allocate the Tile Lists:
	long	lShortTileSize = long(sScaleW) * sScaleH;
	long	lByteTileSize = lShortTileSize << 1;
	long	lLongTileSize = lShortTileSize >> 1;
	// Initial Max
	short		sMaxNumTiles = MIN((long)32767, (long)1 + long(sGridW) * (long)sGridH);

	if (!(m_psTiles = (short*) calloc(lByteTileSize,sMaxNumTiles ) )) return FAILURE;

	//---------------------------------------------Add in the random access:
	short *psTile = m_psTiles;
	if (!(m_ppsTileList = (short**) calloc(sizeof(short*),sMaxNumTiles ) )) return FAILURE;
	for (i=0; i < sMaxNumTiles; i++,psTile += lShortTileSize) m_ppsTileList[i] = psTile;

	short sOff = 0;
	if (!(m_psTileLine = (short*) calloc(sizeof(short),sScaleH ) )) return FAILURE;
	for (i=0;i < sScaleH;i++,sOff += sScaleW) m_psTileLine[i] = sOff;

	//--------------------------------------------- Allocate the coarse grid

	if (!(m_psGrid = (short*) calloc(sizeof(short),long(sGridW)*sGridH) )) return FAILURE;

	//--------------------------------------------- Add in the random access:

	if (!(m_ppsGridLines = (short**) calloc(sizeof(short*),sFullHeight ) )) return FAILURE;
	for (i=0; i < sFullHeight; i++) m_ppsGridLines[i] = m_psGrid + (i >> m_sShiftY)*sGridW;

	//---------------------------------------------  Populate the coarse grid:

	short	*psSrc,*psSrcLine = psUncompressed;
	long	lSrcSkip = long(m_sWidth)*sScaleH;

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

short RMultiGrid::Compress(
	short sTileW,			// Size of tiles to try on this data
	short sTileH,
	long* plSize,			// New Data Size (BYTES)
	long* plNumBlocks,		// Number of unique tiles needed
	short sMatchSame		// If false, NO TILE WILL BE REUSED
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

	short *psUncompressedData = m_psGrid;

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
	short sBlockX,sBlockY,sFullY,i,j;
	short sGridVal,sVal;
	short sTileNumber = 1; // cannot use -0 as a valid offset!

	short sGridW,sGridH;
	GetGridDimensions(&sGridW,&sGridH);

	short sWholeGridW = sGridW - 1; // guaranteed to be whole.
	short	sWholeGridH = sGridH - 1;

	short	sExtraW = m_sWidth - (sWholeGridW << m_sShiftX);
	short	sExtraH = m_sHeight - (sWholeGridH << m_sShiftY);

	short sMaxTile = MIN((long)32767, (long)1 + long(sGridW) * (long)sGridH);
	short	sShortSize = (m_sMaskX+1)*(m_sMaskY+1);
	short sNumMatches = 0;

	short sScanH = m_sMaskY + 1;
	for (sFullY = 0,sBlockY = 0; sBlockY < sGridH; sBlockY++,sFullY += m_sMaskY + 1)
		{
		if (sBlockY == sWholeGridH) sScanH = sExtraH;

		short sScanW = m_sMaskX + 1;
		for (sBlockX = 0; sBlockX < sGridW; sBlockX ++)
			{
			if (sBlockX == sWholeGridW) sScanW = sExtraW;

			sGridVal = *(m_ppsGridLines[sFullY] + sBlockX);
			short* psSrcBlock = psUncompressedData + long(sBlockX) * (m_sMaskX + 1) +
				long(sFullY) * m_sWidth;

			//*********************************************** ANALIZE BLOCK
			short sMatch = 1; // homogeneous block?

			// do slow, shameful way for now...
			// Need to realloc later!
			for (j = 0; j < sScanH; j++)
				{
				for (i = 0; i < sScanW; i++) // only copy what's needed...
					{
					// copy into temp block:
					sVal = psSrcBlock[i + j * long(m_sWidth)];
					*(m_ppsTileList[sTileNumber] + m_psTileLine[ j ] + i) = sVal;
					if (sVal != sGridVal) sMatch = 0; // was not homogeneous
					}
				}

			short sFound = 0;
			if (!sMatch) // store another unique block
				{
				// ADD IN SCAN TO SEE IF YOU MATCH ANY EXISTING BLOCKS>
				if (sMatchSame)
					{
					short k;

					if ((sBlockX != sWholeGridW) && (sBlockY != sWholeGridH)) // do a quick compare:
						{
						for (k=0;k < sTileNumber;k++)
							{
							short sComp = 1;
							// compare last tile to current:
							for (short l=0;l < sShortSize;l++)
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
							short sComp = 1;
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
	short *psOldBlocks = m_psTiles;
	long	lLen = sShortSize * sizeof(short) * sTileNumber;

	m_psTiles = (short*) malloc(lLen);

	memcpy(m_psTiles,psOldBlocks,lLen);
	free(psOldBlocks);

	// Reset the random access
	psOldBlocks = m_psTiles;
	for (i=0; i < sTileNumber; i++,psOldBlocks += sShortSize) m_ppsTileList[i] = psOldBlocks;

	if (plNumBlocks) *plNumBlocks = --sTileNumber;
	if (plSize) *plSize = long(sTileNumber) * (long(sShortSize) * sizeof(short) + sizeof(short*))
		+ long(sGridH) * (sGridW * sizeof(short) + sizeof(short*) );

	return SUCCESS;
	}

// try again!
short RMultiGrid::Decompress()
	{
#ifdef _DEBUG
	if (!m_sIsCompressed)
		{
		TRACE("RMultiGrid::Decompress:  Compress it first, you silly silly man!\n");
		return -1;
		}
#endif

	short *psNewGrid = (short*) calloc(sizeof(short),long(m_sWidth)*m_sHeight);

	if (!psNewGrid) return -1; // allocation error

	// Draw into the new grid:
	short i,j;

	for (j=0;j < m_sHeight;j++)
		{
		for (i=0;i < m_sWidth;i++)
			{
			psNewGrid[i + long(j)*m_sWidth ] = GetVal(i,j);
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

short RMultiGrid::Save(RFile* fp)
	{
	ASSERT(fp);

	if (!m_sIsCompressed)
		{
		TRACE("MultiGrid::Save: Cannot currently save uncompressed MultiGrids.  Sorry.\n");
		return FAILURE;
		}
	
	fp->Write(MULTIGRID_COOKIE);
	fp->Write(short(MULTIGRID_CURRENT_VERSION));

	fp->Write(m_sWidth);
	fp->Write(m_sHeight);
	fp->Write(m_sIsCompressed); // ASSUME IT IS COMPRESSED!
	fp->Write(m_sMaskX);
	fp->Write(m_sMaskY);
	fp->Write(short(sizeof(short))); // For future expansion!

	short sGridW,sGridH;
	GetGridDimensions(&sGridW,&sGridH);

	fp->Write(sGridW);
	fp->Write(sGridH);

	short sNumTiles = GetNumTiles();
	fp->Write(sNumTiles);

	fp->Write(long(0)); // Reserved1
	fp->Write(long(0)); // Reserved2
	fp->Write(long(0)); // Reserved3
	fp->Write(long(0)); // Reserved4

	// Write out the grid of shorts:
	short i,j;

	for (j=0;j < sGridH; j++)
		for (i=0;i<sGridW;i++)
			fp->Write(*(m_ppsGridLines[j<<m_sShiftY] + i));

	// Write out the tiles, omitting the zeroth black tile.
	short sTileLen = (m_sMaskX + 1) * (m_sMaskY + 1);

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

short RMultiGrid::Load(RFile* fp)
	{
	ASSERT(fp);

	if (m_psGrid)
		{
		TRACE("MultiGrid::Load: Cannot load on top of an existing grid!\n");
		return FAILURE;
		}
	
	short sVal;
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

	short sCodeSize;
	fp->Read(&sCodeSize); // normally 2 for short.

	short sGridW,sGridH;

	fp->Read(&sGridW);
	fp->Read(&sGridH);

	// ALLOCATE IT
	m_psGrid = (short*) calloc(sCodeSize,long(sGridW)*sGridH);
	if (!m_psGrid)
		{
		TRACE("MultiGrid::Load: Out of Memory!!!!\n");
		return FAILURE;
		}

	short sNumTiles;
	fp->Read(&sNumTiles);
	short sTileLen = (m_sMaskX + 1) * (m_sMaskY + 1);

	// Allocate the tiles:
	// This appears as a memory leak!
	if (!(m_psTiles = (short*)calloc(sTileLen * sCodeSize,sNumTiles)))
		{
		free(m_psGrid);
		TRACE("MultiGrid::Load: Out of Memory!!!!\n");
		return FAILURE;
		}

	long lVal;
	fp->Read(&lVal); // Reserved1
	fp->Read(&lVal); // Reserved2
	fp->Read(&lVal); // Reserved3
	fp->Read(&lVal); // Reserved4

	short i,j;

	// Must generate the shift members to use them now!
	m_sShiftX = MaskToShift(m_sMaskX);
	m_sShiftY = MaskToShift(m_sMaskY);

	// Set up random access for grid:
	short	sFullHeight = sGridH * (m_sMaskY + 1);
	if (!(m_ppsGridLines = (short**) calloc(sizeof(short*),sFullHeight ) )) return FAILURE;
	for (i=0; i < sFullHeight; i++) m_ppsGridLines[i] = m_psGrid + (i >> m_sShiftY)*sGridW;

	// Read in the grid of shorts:
	for (j=0;j < sGridH; j++)
		for (i=0;i<sGridW;i++)
			fp->Read((m_ppsGridLines[j<<m_sShiftY] + i));

	// Set up random access for tiles:
	short *psTile = m_psTiles;
	if (!(m_ppsTileList = (short**) calloc(sizeof(short*),sNumTiles ) )) return FAILURE;
	for (i=0; i < sNumTiles; i++,psTile += sTileLen) m_ppsTileList[i] = psTile;

	// Read in the tiles, omitting the zeroth black tile.

	for (i=1;i < sNumTiles;i++)
		{
		fp->Read(m_ppsTileList[i],sTileLen);
		}

	// Finish up thae random access:
	short sOff = 0;
	if (!(m_psTileLine = (short*) calloc(sCodeSize,m_sMaskY+1 ) )) return FAILURE;
	for (i=0;i < m_sMaskY+1;i++,sOff += m_sMaskX + 1) m_psTileLine[i] = sOff;

	// Set the shift values:

	return SUCCESS;
	}