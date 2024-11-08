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
// region.h
// Project: Postal
//
// This module impliments the CSmashatorium, which handles things smashing
// together (also known as collision detection).
//
// History:
//		02/18/97 MJR	Started.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef TRIGGER_REGIONS_H
#define TRIGGER_REGIONS_H

#include "RSPiX.h"
#include "TriggerRgn.h"
#ifdef PATHS_IN_INCLUDES
	#include "ORANGE/MultiGrid/MultiGridIndirect.h"
#else
	#include "multigridindirect.h"
#endif
#include "realm.h"

// See the cpp file for usage details...

extern	RMultiGridIndirect*	CreateRegionMap(int16_t sWidth,int16_t sHeight,int16_t sMaxPlanes,
												 int16_t sTileW,int16_t sTileH);

extern	int16_t	StrafeAddRegion(RMultiGridIndirect* pMGI,TriggerRgn regions[256]);

extern	int16_t CompressMap(RMultiGridIndirect* pMGI,int16_t sTileW,int16_t sTileH);

// Have main guys alert pylons to their presence.
extern	void	SpewTriggers(CRealm* pRealm,	uint16_t	usDudeUID,int16_t sX,int16_t sZ);


////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
#endif
