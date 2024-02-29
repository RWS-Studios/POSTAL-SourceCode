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
//*****************************************************************************
//
// ATTRIB.CPP
//
//
// History:
//
//		02/21/95	BH		Started this file, based on the
//
//*****************************************************************************
				
#include <stdafx.h>
#include "attrib.h"
#include "univ.h"

extern CUniverse g_GameUniverse;

//*****************************************************************************
// 
// Default Constructor
//
//*****************************************************************************

CAttribute::CAttribute()
{
	g_GameUniverse.GetAttributes(m_pAttribMap, m_pAttribCont, &m_sMapWidth);
}

//*****************************************************************************
//
// Destructor
//
//*****************************************************************************

CAttribute::~CAttribute()
{

}


//*****************************************************************************
//
// Reset
//
// Description:
//
//
//*****************************************************************************


void CAttribute::Reset()
{

}

//*****************************************************************************
//
// FromBlock
//
// Description:
//
// Input:
//
// Uses:
//
// Output:
//
// Return:
//
//*****************************************************************************

void CAttribute::FromBlock()
{

}

//*****************************************************************************
//
// ToBlock
//
// Description:
//
// Input:
//
// Uses:
//
// Output:
//
// Return:
//
//*****************************************************************************

void CAttribute::ToBlock()
{

}

//*****************************************************************************
//
// PutBlock
//
// Description:
//		Get the block number of the block, match the block number from
//		m_sAttrRewardFrom and put the new block (from m_sAttrRewardTo).
//
// Input:
//		sX = world x coordinate of point to be used
//		sY = world y coordinate of point to be used
//
// Uses:
//
// Output:
//		none
//
// Return:
//		none
//
//*****************************************************************************

void CAttribute::PutBlock()
{

}

//*****************************************************************************
//
// GetPointAttrib
//
// Description:
//		This routine checks the BG attributes for a specific point.
//
//		If the specified point is not on the map, then $0000 is used
//		as the attribute.
//
// Input:
//		sX = world x coordinate of point to be checked
//		sY = world y coordinate of point to be checked
//
// Uses:
//		m_pAttribMap
//		m_pAttribCont
//
// Output:
//		sAttribute = attribute for the block that the point falls into
//
// Return:
//		TRUE if the attribute applies to the point, FALSE otherwise
//
//*****************************************************************************

const WORD cPixelMasks[16] = 
{
	0x8000,
	0x4000,
	0x2000,
	0x1000,
	0x0800,
	0x0400,
	0x0200,
	0x0100,
	0x0080,
	0x0040,
	0x0020,
	0x0010,
	0x0008,
	0x0004,
	0x0002,
	0x0001
};

BOOL CAttribute::GetPointAttrib(short sX, short sY, WORD* pwAttribute)
{
	short sBlockX = sX / 16;
	short sBlockY = sY / 16;
	short sPixelX = sX % 16;
	short sPixelY = sY % 16;

	WORD wAttribute = m_pAttribMap[sBlockY * m_sMapWidth + sBlockX];
	*pwAttribute = wAttribute & ATTRIBUTE_MASK;

	CONTOUR cCurrent = m_pAttribCont[wAttribute & CONTOUR_MASK];

	WORD wLine = cCurrent.block[sPixelY];

	if (wLine & cPixelMasks[sPixelX])
		return TRUE;
	else
		return FALSE;
}

//*****************************************************************************
//
// GetPointSurface
//
// Description:
//		This routine tries to find the surface directly above or below a given
//		point.  We will only go as far as 1 block above or 1 block below
//		because further than that will not help anyway.
//		It only deals with the solid/hollow attribute and any other types of
//		attributes are regarded as air.
//
// Input:
//		sX = world x coordinate of point to be checked
//		sY = world y coordinate of point to be checked
//
// Uses:
//		m_pAttribMap
//		m_pAttribCont
//
// Output:
//		sYSurface = world y coordinate of the surface at the given x
//		sAttribute = attribute description flags for the given block
//						 (ie, SOLID, HOLLOW etc.)
//
// Return:
//
//*****************************************************************************

short CAttribute::GetPointSurface(short sX, short sY)
{
	short sBlockX = sX / 16;
	short sBlockY = sY / 16;
	short sPixelX = sX % 16;
	short sPixelY = sY % 16;
	short sSurfaceY = sY;
//	CONTOUR cCurrent;
	CONTOUR cAdjacent;

	WORD wAttribute = m_pAttribMap[sBlockY * m_sMapWidth + sBlockX];
	
//	cCurrent = m_pAttribCont[wAttribute & CONTOUR_MASK];

//-----------------------------------------------------------------------------
// See if the surface is in the current block by checking the top and 
// bottom rows of the block in the current column.  If the top is 0 and the
// bottom is 1 then the surface is somewhere in this block.  If the top
// and bottom are 1 then the surface is in some block above and if the top
// and bottom are 0 then there could either be a platform above or ground below
// and we will try the down direction first.  If the top is 1 and the bottom 0
// then we assume this is a platform and the top of the platform must be in
// a block above this one.
//-----------------------------------------------------------------------------

	WORD wTop;
	WORD wBottom;




//
//	*pwAttribute = wAttribute & ATTRIBUTE_MASK;

	CONTOUR cCurrent = m_pAttribCont[wAttribute & CONTOUR_MASK];

	WORD wLine = cCurrent.block[sPixelY];

	if (wLine & cPixelMasks[sPixelX])
		return TRUE;
	else
		return FALSE;



	return 0;		//return the Y position of the surface
}


//*****************************************************************************
// EOF
//*****************************************************************************
