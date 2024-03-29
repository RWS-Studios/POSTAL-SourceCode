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
// region.cpp
// Project: Nostril (aka Postal)
//
// This module facilitates editor usage and creation of region attributes.
// The regions all need to coexist as separate attribute planes, so the
// additional compression of an RMultiGridIndirect was used.  For this reason,
// region attribute access is somewhat slower than normal attribute access,
// and should be used sparingly.
//
// History:
//		05/08/97 JRD	Started.
//
//		05/14/97 BRH	Fixed the reversed Unique ID's in SpewTriggers().
//
////////////////////////////////////////////////////////////////////////////////

#include "RSPiX.h"
#ifdef PATHS_IN_INCLUDES
	#include "ORANGE/MultiGrid/MultiGridIndirect.h"
#else
	#include "multigridindirect.h"
#endif
#include "TriggerRegions.h"
#include "TriggerRgn.h"
#include "realm.h"
#include "thing.h"


// Here is a generic set of tools for creating a region map.
// They are a lower level and don't iteratively optimize:

////////////////////////////////////////////////////////////////////////////////
//	
//  CreateRegionMap - Begin your session
//
//	 UINPUT: Size of map in pixels, maximum number of overlapping planes,
//				and TILE SIZE for palettes.  (Should be as large as possible
//				without causing greater than the maximum number of tiles to overlap.]
//				sMaxPlanes must NOT exceed MGI_MAX_PLANES!
//
//  OUPUT:	An allocated RMultiGridIndirect containing an uncompressed 
//				RMultiGrid within it, ready for writing to.
//
//	 RETURN VALUE:	NULL on failure, else the new grid.  Can fail due to memory
//						alloc errors.
//
////////////////////////////////////////////////////////////////////////////////

RMultiGridIndirect*	CreateRegionMap(int16_t sWidth,int16_t sHeight,int16_t sMaxPlanes,
												 int16_t sTileW,int16_t sTileH)
	{
	ASSERT( (sWidth > 0) && (sHeight > 0) && (sMaxPlanes > 0)
				&& (sTileW > 0) && (sTileH > 0) );
	ASSERT(sMaxPlanes <= MGI_MAX_PLANES);

	RMultiGridIndirect* pMGI = new RMultiGridIndirect;
	if (!pMGI) 
		{
		TRACE("CreateRegionMap: alloc error!\n");

		return NULL;
		}

	if (pMGI->Alloc(sWidth,sHeight,sMaxPlanes,sTileW,sTileH) != SUCCESS) 
		{
		TRACE("CreateRegionMap: alloc error!\n");
		return NULL;
		}

	// Create and install the MultiGrid.
	RMultiGrid*	pmg = new RMultiGrid;

	if (!pmg)
		{
		TRACE("CreateRegionMap: alloc error!\n");
		return NULL;
		}

	if (pmg->Alloc(sWidth,sHeight) != SUCCESS)
		{
		TRACE("CreateRegionMap: alloc error!\n");
		delete pmg;
		return NULL;
		}

	pMGI->InstallMultiGrid(pmg);

	return pMGI;
	}

////////////////////////////////////////////////////////////////////////////////
//	
//	 StrafeAddRegion - add all members of an array of 256 FSPR1's to the map.
//
//  It is assumed that there are 255 trigger regions (1 - 255), whose address
//		is also their "palette value"
//	
//	 RETURN VALUE - SUCCESS or FAILURE.  Will return failure if there were too
//		many overlapping regions in one tile.  (It still will create the map, but
//    on that one tile there will be some region drop out.)  You can fix this by
//    increasing the number of max planes or decreasing the palette tile size.
//
//		NOTE:  When finished, you should compress the MultiGrid as you see fit
//				before saving.
//	
////////////////////////////////////////////////////////////////////////////////

int16_t	StrafeAddRegion(RMultiGridIndirect* pMGI,TriggerRgn regions[256])
	{
	ASSERT(pMGI);
	int16_t sRet = SUCCESS;

	for (int16_t i=1; i < 256;i++)
		{
		if (regions[i].pimRgn)
			{
			if (pMGI->AddFSPR1(regions[i].pimRgn,regions[i].sX,regions[i].sY,
				uint8_t(i),TriggerRgn::MaxRgnWidth,TriggerRgn::MaxRgnHeight)
				!= SUCCESS)
				{
				TRACE("StrafeAddRegion:: Problem installing region %hd\n",i);
				sRet = FAILURE;
				}
			}
		}

	return sRet;
	}

////////////////////////////////////////////////////////////////////////////////
//
//	 CompressMap - currently just compresses to your specifications, but may
//		be expanded to do some iterative testing for optimizing compression:
// 
//  NOTE:
//
////////////////////////////////////////////////////////////////////////////////
int16_t CompressMap(RMultiGridIndirect* pMGI,int16_t sTileW,int16_t sTileH)
	{
	ASSERT(pMGI);
	ASSERT(pMGI->m_pmg);
	// use compression results to optimize
	pMGI->m_pmg->Compress(sTileW,sTileH);
	return SUCCESS;
	}

////////////////////////////////////////////////////////////////////////////////
//
//	SpewTriggers - checks for triggers underneath a main dude and
//					alerts all relevant pylons to his presence
////////////////////////////////////////////////////////////////////////////////
//
void	SpewTriggers(CRealm* pRealm,	uint16_t	usDudeUID,int16_t sX,int16_t sZ)
	{
	uint8_t	aucHitList[MGI_MAX_PLANES];
	if (pRealm->m_pTriggerMap == NULL) return; // No triggers

	int16_t sMax = pRealm->m_pTriggerMap->m_sMaxPlanes;

	// GET THE ATTRIBUTE MAP FOR THE TRIGGERS:
	pRealm->m_pTriggerMap->GetVal(aucHitList,sX,sZ);
	uint8_t*	pHit = aucHitList;
	GameMessage	msg;

	msg.msg_DudeTrigger.eType = typeDudeTrigger;
	msg.msg_DudeTrigger.sPriority = 0;
	msg.msg_DudeTrigger.u16DudeUniqueID = usDudeUID;
	msg.msg_DudeTrigger.dX = double(sX);
	msg.msg_DudeTrigger.dZ = double(sZ);

	int16_t sNum = sMax;
	while (*pHit && sNum) // got a hit:
		{
		// send a trigger message out:
		CThing* pThing = NULL;

		if (pRealm->m_idbank.GetThingByID(&pThing, pRealm->m_asPylonUIDs[*pHit]) == SUCCESS)
			{
			if (pThing) pThing->SendThingMessage(&msg, 0, pThing); // post the trigger!
			}
		pHit++;
		sMax--;
		}
	
	}


////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
