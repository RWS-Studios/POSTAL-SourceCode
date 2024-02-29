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
// ATTRIBUTE.CPP
//
// Created on	10/03/96 BRH
// Implemented	10/03/96 BRH
//
//	10/03/96 BRH	Started this class for use in the Postal
//						demo to provide a way to load the attribute
//						map and to access the data
//
// 10/14/96 BRH	Added scaling to the attribute map utility and
//						changed the Load to read in the scale from
//						the new version of the attribute map file, and
//						changed GetAttribute functions to scale the
//						real world coordinates to the scaled map size.  
//						This allows maps to be smaller and take less
//						less memory.
//
//	10/15/96 BRH	Expanded the attribute map to 16 bits.  The
//						High 8 bits are for height rather than the
//						simple walk/no walk attribute.  The lower 8
//						bits are for flags.  Whenever an attribute 
//						for a region is returned, it is the ORed 
//						combination of the low 8 bits, and the max
//						height in the high 8 bits.
//
//	10/22/96	JMI	Moved #include <stdlib.h> to before
//						#include "System.h".
//
//	10/28/96 MJR	Switched from __min and __max to MIN and MAX for
//						compatibility with CodeWarrior.
//
// 10/31/96 BRH	Changed CAttributeMap to RAttributeMap which is 
//						the new RSPiX naming convention.
//
//	11/01/96	JMI	Changed:
//						Old label:		New label:
//						=========		=========
//						LITTLE_ENDIAN	RFile::LittleEndian
//
//	11/20/96 BRH	Changed the load function to load the new attribute
//						format which includes block maps and detail maps.
//						Also changed the GetAttriubte functions to 
//						interpret the new format.  
//
//	11/21/96 BRH	Changed the GetAttribute rectangle version for the
//						new attribute format.  It now loops through the
//						rectangle but skips to the next block if the 
//						attribute from the block it just read contains
//						all of the same attributes.  	The height is now
//						height and depth in the low 8 bits.  It can be
//						cast to a signed char to get the value of the height
//						or depth.  The attribute flag bits are in the upper 7
//						bits of the attribute and the high bit is the
//						flag to mark the map entry as a lookup or complete
//						block with a single attribute.
//
// 11/22/96 BRH	Fixed problem with casting the result of the 
//						attribute height/depth for the GetAttribute
//						function.
//
// 01/14/97 BRH	Fixed problem of checking an attribute right on the
//						right or bottom edge of the world.  If you called
//						GetAttribute with a point one pixel past the edge
//						of the world it would crash, if it were inside or
//						further outside it was fine.  
//
//	02/03/97 BRH	Added a Load that takes an RFile* in addition to
//						the one that takes the filename as char*.
//
//////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include "System.h"	

 #ifdef PATHS_IN_INCLUDES
	#include "ORANGE/Attribute/attribute.h"
	#include "ORANGE/File/file.h"

#else
	#include "ATTRIBUTE.H"
	#include "FILE.H"

#endif	//PATHS_IN_INCLUDES

//////////////////////////////////////////////////////////////////////
//
// Constructor
//
// Description:
//		Default contstuctor for the RAttributeMap class.  Initializes
//		the map buffer pointer.
//
// Parameters:
//		none
//
// Returns:
//		none
//
//////////////////////////////////////////////////////////////////////

RAttributeMap::RAttributeMap()
{
	m_pusDetailMap = NULL;
	m_pusMap = NULL;
	m_lWidth = 0;
	m_lHeight = 0;
	m_ucFlags = 0;
	m_ucMaxHeight = 0;
	m_ucMinHeight = 0;
	m_usLastAttribute = 0;
	m_sBlockDataSize = 0;
	m_sNumDetailMaps = 0;
	m_lWorldWidth = 0;
	m_lWorldHeight = 0;
}

//////////////////////////////////////////////////////////////////////
//
// Constructor
//
// Description:
//		This constructor takes the filename of the map to load and
//		first initializes a RAttributeMap and then calls the load
//		function.
//
// Parameters:
//		char* pszFilename = filename of map file to load
//
// Returns:
//		none
//
//////////////////////////////////////////////////////////////////////

RAttributeMap::RAttributeMap(char* pszFilename)
{
	m_pusDetailMap = NULL;
	m_pusMap = NULL;
	m_lWidth = 0;
	m_lHeight = 0;
	m_ucFlags = 0;
	m_ucMaxHeight = 0;
	m_ucMinHeight = 0;
	m_usLastAttribute = 0;
	m_sBlockDataSize = 0;
	m_sNumDetailMaps = 0;
	m_lWorldWidth = 0;
	m_lWorldHeight = 0;
	Load(pszFilename);
}

//////////////////////////////////////////////////////////////////////
//
// Destructor
//
// Description:
//		General destructor for RAttributeMap class.  Frees the map
//		pointer if one was allocated.
//
// Parameters:
//		none
//
// Returns:
//		none
//
//////////////////////////////////////////////////////////////////////

RAttributeMap::~RAttributeMap()
{
	FreeMap();
}

//////////////////////////////////////////////////////////////////////
//
// FreeMap
//
// Description:
//		Frees the map buffer if one was allocated
//
// Parameters:
//		none
//
// Returns:
//		none
//
//////////////////////////////////////////////////////////////////////

void RAttributeMap::FreeMap()
{
	if (m_pusMap)
	{
		delete []m_pusMap;
		m_pusMap = NULL;
	}
	if (m_pusDetailMap)
	{
		delete []m_pusDetailMap;
		m_pusDetailMap = NULL;
	}
}

//////////////////////////////////////////////////////////////////////
//
// AllocateMap
//
// Description:
//		Allocates memory for a map of the given size
//
// Parameters:
//		ULONG ulSize = size of buffer to allocate
//
// Returns:
//		SUCCESS if memory was allocated
//		FAILURE otherwise
//
//////////////////////////////////////////////////////////////////////

short RAttributeMap::AllocateMap(ULONG ulSize, ULONG ulDetailMapSize)
{
	short sReturn = SUCCESS;

	if (m_pusMap == NULL)
		m_pusMap = new USHORT[ulSize];

	if (m_pusDetailMap == NULL)
		m_pusDetailMap = new USHORT[ulDetailMapSize];

	if (m_pusMap == NULL || m_pusDetailMap == NULL)
		return FAILURE;
	else
		return SUCCESS;
}

//////////////////////////////////////////////////////////////////////
//
// GetAttribute
//
// Description:
//		This is the single point version which returns the attribute
//		at the given position on the map.
//
// Parameters:
//		ULONG ulX = x position of point
//		ULONG ulY = y position of point
//
// Returns:
//		USHORT = attribute at the specified point
//
//////////////////////////////////////////////////////////////////////
/*
USHORT RAttributeMap::GetAttribute(long lX, long lY)
{
	if ((lX/m_sScaleX) > m_lWidth || (lY/m_sScaleY) > m_lHeight || lX < 0 || lY < 0)
		return ATTRIBUTE_NOT_WALKABLE;

	m_usLastAttribute = m_pusMap[(lY/m_sScaleY)*m_lWidth + (lX/m_sScaleX)];
	m_ucFlags = (UCHAR) (m_usLastAttribute & 0x00ff);
	m_ucMinHeight = m_ucMaxHeight = (UCHAR) ((m_usLastAttribute & 0xff00) >> 8);

	return m_usLastAttribute;
}
*/

USHORT RAttributeMap::GetAttribute(long lX, long lY)
{
	USHORT usBlockData;

	if ((lX/m_sScaleX) >= m_lWidth || (lY/m_sScaleY) >= m_lHeight || lX < 0 || lY < 0)
		return ATTRIBUTE_NOT_WALKABLE;

	// See if the block being accessed is an attribute or 
	// a detail map lookup
	usBlockData = m_pusMap[(lY/m_sScaleY)*m_lWidth + (lX/m_sScaleX)];
	if (usBlockData & 0x8000)
	// Lookup the value for this pixel
	{
		m_usLastAttribute = m_pusDetailMap[((usBlockData & 0x7fff) * m_sBlockDataSize) + (((lY % m_sScaleY)*m_sScaleY) + (lX % m_sScaleX))];
		m_ucFlags = (UCHAR) ((m_usLastAttribute & 0xff00) >> 8);
		m_ucMinHeight = m_ucMaxHeight = (UCHAR) (m_usLastAttribute & 0x00ff); 
	}
	else
	{
		m_usLastAttribute = usBlockData;
		m_ucFlags = (UCHAR) ((usBlockData & 0xff00) >> 8);
		m_ucMinHeight = m_ucMaxHeight = (UCHAR) (usBlockData & 0x00ff);
	}
	return m_usLastAttribute;		
}	

//////////////////////////////////////////////////////////////////////
//
// GetAttribute
//
// Description:
//		This is the rectangle version which returns the ORed 
//		combination of the attributes within the rectangle.  The
//		attribute bits have been arranged so that an ORed combination
//		has a real meaning.  If any point within the rectnagle is 
//		non-walkable, then the non-walkable bit is set and the other
//		bits pertain to shadow behind different layers, so the ORed
//		combination will tell you each layer that the object is
//		behind.  The coordinates given are scaled down to the map size.
//		It converts the real world coordinates to map coordinates
//		by dividing them by the scale for the Map.  The maps may be
//		created such that a single map position refers to more than
//		one pixel location.  This help saves memory by having smaller
//		maps, and it also results in faster checking of this version
//		of GetAttribute that takes the rectangular area to be checked.
//
//		Note the rectangle is clipped to the map size if it happens
//		to exceed the boundries.
//
// Parameters:
//		ulTopCoord = top of box to check.
//		ulBottomCoord = bottom of box to check
//		ulLeftCoord = left size of box to check
//		ulRightCoord = right side of box to check
//
// Returns
//		USHORT = ORed combination of attribute bits within this rectangle
//
//////////////////////////////////////////////////////////////////////

/*
USHORT RAttributeMap::GetAttribute(long lTopCoord, long lBottomCoord,
											 long lLeftCoord, long lRightCoord)
{
	long lTop = lTopCoord / m_sScaleY;
	long lBottom = lBottomCoord / m_sScaleY;
	long lLeft = lLeftCoord / m_sScaleX;
	long lRight = lRightCoord / m_sScaleX;

	// Clip the box to the map size if necessary
	if (lTop >= m_lHeight)
		lTop = m_lHeight - 1;
	if (lTop < 0)
		lTop = 0;
	if (lBottom >= m_lHeight)
		lBottom = m_lHeight - 1;
	if (lBottom < 0)
		lBottom = 0;
	if (lLeft >= m_lWidth)
		lLeft = m_lWidth - 1;
	if (lLeft < 0)
		lLeft = 0;
	if (lRight >= m_lWidth)
		lRight = m_lWidth - 1;
	if (lRight < 0)
		lRight = 0;

	long lRow;
	long lCol;
	USHORT usResult = 0;
	USHORT usAttrib = 0;
	USHORT usMin = m_pusMap[lTop*m_lWidth + lLeft];
	USHORT usMax = m_pusMap[lTop*m_lWidth + lLeft];
	USHORT usFlags = 0;

	for (lRow = lTop; lRow <= lBottom; lRow++)
		for (lCol = lLeft; lCol <= lRight; lCol++)
		{
			usAttrib = m_pusMap[lRow*m_lWidth + lCol];
			usFlags |= usAttrib;
			usMin = MIN(usMin, usAttrib);
			usMax = MAX(usMax, usAttrib);
		}

	m_ucFlags = (UCHAR) (usFlags & 0x00ff);
	m_ucMinHeight = (UCHAR) (usMin >> 8);
	m_ucMaxHeight = (UCHAR) (usMax >> 8);

	usResult = (usMax & 0xff00) | (usFlags & 0x00ff);

	return usResult;
}
*/

USHORT RAttributeMap::GetAttribute(long lTopCoord, long lBottomCoord,
											 long lLeftCoord, long lRightCoord)
{
	long lTop = lTopCoord;// / m_sScaleY;
	long lBottom = lBottomCoord;// / m_sScaleY;
	long lLeft = lLeftCoord;// / m_sScaleX;
	long lRight = lRightCoord;// / m_sScaleX;

	// Clip the box to the map size if necessary
	if (lTop >= m_lWorldHeight)
		lTop = m_lWorldHeight - 1;
	if (lTop < 0)
		lTop = 0;
	if (lBottom >= m_lWorldHeight)
		lBottom = m_lWorldHeight - 1;
	if (lBottom < 0)
		lBottom = 0;
	if (lLeft >= m_lWorldWidth)
		lLeft = m_lWorldWidth - 1;
	if (lLeft < 0)
		lLeft = 0;
	if (lRight >= m_lWorldWidth)
		lRight = m_lWorldWidth - 1;
	if (lRight < 0)
		lRight = 0;

	long lRow;
	long lCol;
	USHORT usResult = 0;
	USHORT usAttrib = 0;
	USHORT usFlags = 0;
	char cMax = (char) 0x80;

	for (lRow = lTop; lRow <= lBottom; lRow++)
		for (lCol = lLeft; lCol <= lRight; lCol++)
		{
			usAttrib = m_pusMap[((lRow/m_sScaleY) * m_lWidth ) + (lCol / m_sScaleX)];
			if (usAttrib & 0x8000)
			// Lookup the value in the detail map
			{
				usAttrib = m_pusDetailMap[((usAttrib & 0x7fff) * m_sBlockDataSize) +
				                          (((lRow % m_sScaleY) * m_sScaleX) +
												  (lCol % m_sScaleX))];
				usFlags |= usAttrib;
				cMax = MAX(cMax, (char) (usAttrib & 0x00ff));
			}
			else
			// Get the attribute for this whole block
			{
				usAttrib &= 0x7fff;
				usFlags |= usAttrib;
				cMax = MAX(cMax, (char) (usAttrib & 0x00ff));
				// Skip to next block since these are all the same
				lCol += m_sScaleX - (lCol % m_sScaleX);
			}
		}

	usResult = (usFlags & 0xff00) | (U8) cMax;

	return usResult;
}


//////////////////////////////////////////////////////////////////////
//
// Load
//
// Description:
//		Loads the map file into a buffer.  Checks to make sure that
//		the file is the right type and version before loading.
//
// Parameters:
//		char* pszFilename = filename of map file to be loaded
//
// Returns:
//		SUCCESS if loaded correctly
//		-1 if file not found
//		-2 if wrong file type
//		-3 if wrong version
//		-4	if error reading width
//		-5 if error reading height
//		-6 if error reading X scale
//		-7 if error reading Y scale
//		-8 if error allocating memory for map buffer
//		-9 if error reading map data
//
//////////////////////////////////////////////////////////////////////

short RAttributeMap::Load(char* pszFilename)
{
	RFile cf;
	short sReturn = SUCCESS;

	if (cf.Open(pszFilename, "rb", RFile::LittleEndian) != SUCCESS)
	{
		TRACE("RAttributeMap::Load - could not open file %s\n", pszFilename);
	 	return FAILURE;
	}

	sReturn = Load(&cf);

	cf.Close();

	return sReturn;
}

short RAttributeMap::Load(RFile* prf)
{
	short sReturn = SUCCESS;
	ULONG ulFileType;
	ULONG ulVersion;

	if (prf && prf->IsOpen())
	{
		if (prf->Read(&ulFileType) == 1)
		{
			if (ulFileType == ATTRIBUTE_MAP_COOKIE)
			{
				if (prf->Read(&ulVersion) == 1)
				{
					if (ulVersion == ATTRIBUTE_CURRENT_VERSION)
					{
						if (prf->Read(&m_lWidth) == 1)
						{
							if (prf->Read(&m_lHeight) == 1)
							{
								if (prf->Read(&m_sScaleX) == 1)
								{
									if (prf->Read(&m_sScaleY) == 1)
									{
										if (prf->Read(&m_sNumDetailMaps) == 1)
										{
											if (AllocateMap(m_lWidth * m_lHeight, m_sNumDetailMaps * m_sScaleX * m_sScaleY) == SUCCESS)
											{
												if (prf->Read(m_pusMap, m_lWidth*m_lHeight) == (long) (m_lWidth*m_lHeight))
												{
													if (prf->Read(m_pusDetailMap, m_sNumDetailMaps*m_sScaleX*m_sScaleY) == m_sNumDetailMaps*m_sScaleX*m_sScaleY)
													{
														sReturn = SUCCESS;
														m_sBlockDataSize = m_sScaleX * m_sScaleY;
													}	
													else
													{
														TRACE("RAttributeMap::Load - Error reading detail maps\n");
														sReturn = -11;
													}
												}
												else
												{
													TRACE("RAttributeMap::Load - Error reading map data\n");
													sReturn = -9;
												}
											}
											else
											{
												TRACE("RAttributeMap::Load - Error allocating map buffer\n");
												sReturn = -8;
											}	
										}
										else
										{
											TRACE("RAttributeMap::Load - Error reading number of detail maps\n");
											sReturn = -10;
										}
									}
									else
									{
										TRACE("RAttributeMap::Load - Error reading Y scaling value\n");
										sReturn = -7;
									}
								}
								else
								{
									TRACE("RAttributeMap::Load - Error reading X scaling value\n");
									sReturn = -6;
								}
							}
							else
							{
								TRACE("RAttributeMap::Load - Error reading height\n");
								sReturn = -5;
							}
						}
						else
						{
							TRACE("RAttributeMap::Load - Error reading width\n");
							sReturn = -4;
						}					
					}
					else
					{
						TRACE("RAttributeMap::Load - Wrong version: This file %d current version %d\n", ulVersion, ATTRIBUTE_CURRENT_VERSION);
						sReturn = -3;
					}				
				}
				else
				{
					TRACE("RAttributeMap::Load - Error reading version\n");
					sReturn = -3;
				}			
			}
			else
			{
				TRACE("RAttributeMap::Load - Wrong file type, should be \"ATRM\"\n");
				sReturn = -2;
			}		
		}
		else
		{
			TRACE("RAttributeMap::Load - Error reading file type\n");
			sReturn = -2;
		}	
	}
	else
	{
		TRACE("RAttributeMap::Load - prf does not refer to an open RFile*\n");
		sReturn = -1;
	}

	m_lWorldWidth = m_lWidth * m_sScaleX;
	m_lWorldHeight = m_lHeight * m_sScaleY;
	
	return sReturn;		
}