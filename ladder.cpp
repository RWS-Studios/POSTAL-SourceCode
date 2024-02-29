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
// Ladder.cpp
// Project: Nostril (aka Postal)
// 
// History:
//		06/02/97 JMI	Started.
//
//		06/02/97	JMI	Was not previously setting the position of the smashes'
//							spheres.  Fixed.
//
//		07/09/97	JMI	Now uses m_pRealm->Make2dResPath() to get the fullpath
//							for 2D image components.
//
//////////////////////////////////////////////////////////////////////////////
//
// This CThing-derived class will represent ladders that the CDudes, and 
// perhaps other characters, can use to climb to heights they cannot step up
// to.
//
//////////////////////////////////////////////////////////////////////////////
#define LADDER_CPP

#include "RSPiX.h"
#include <math.h>

#include "Ladder.h"
#include "game.h"
#include "reality.h"

////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////

#define GUI_FILE_NAME				"res/editor/Ladder.gui"

// This is for edit mode only.
#define LADDER_IMAGE_FILENAME		"ladder.bmp"

#define GUI_ID_LEN					3
#define GUI_ID_HEIGHT				4
#define GUI_ID_ROT					5

#define GUI_ID_TYPE_BASE			10

#define LADDER_ENDS_RADIUS			10

////////////////////////////////////////////////////////////////////////////////
// Variables/data
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Load object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CLadder::Load(								// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,										// In:  File to load from
	bool bEditMode,									// In:  True for edit mode, false otherwise
	short sFileCount,									// In:  File count (unique per file, never 0)
	ULONG	ulFileVersion)								// In:  Version of file format to load.
	{
	short sResult = CThing::Load(pFile, bEditMode, sFileCount, ulFileVersion);
	if (sResult == 0)
		{
		switch (ulFileVersion)
			{
			default:
			case 15:
			case 14:
			case 13:
			case 12:
			case 11:
			case 10:
			case 9:
			case 8:
			case 7:
			case 6:
			case 5:
			case 4:
			case 3:
			case 2:
			case 1:
				pFile->Read(&m_dX);
				pFile->Read(&m_dY);
				pFile->Read(&m_dZ);
				pFile->Read(&m_sLen);
				pFile->Read(&m_sHeight);
				pFile->Read(&m_sRotY);
				break;
			}

		// Make sure there were no file errors or format errors . . .
		if (!pFile->Error() && sResult == 0)
			{
			// Get resources and initialize.
			sResult = Init();
			}
		else
			{
			sResult = -1;
			TRACE("CLadder::Load(): Error reading from file!\n");
			}
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Save object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CLadder::Save(										// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,											// In:  File to save to
	short sFileCount)										// In:  File count (unique per file, never 0)
	{
	short	sResult	= CThing::Save(pFile, sFileCount);
	if (sResult == 0)
		{
		pFile->Write(&m_dX);
		pFile->Write(&m_dY);
		pFile->Write(&m_dZ);
		pFile->Write(&m_sLen);
		pFile->Write(&m_sHeight);
		pFile->Write(&m_sRotY);

		// Make sure there were no file errors
		sResult	= pFile->Error();
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Startup object
////////////////////////////////////////////////////////////////////////////////
short CLadder::Startup(void)								// Returns 0 if successfull, non-zero otherwise
	{
	return 0;
	}


////////////////////////////////////////////////////////////////////////////////
// Shutdown object
////////////////////////////////////////////////////////////////////////////////
short CLadder::Shutdown(void)							// Returns 0 if successfull, non-zero otherwise
	{
	return 0;
	}


////////////////////////////////////////////////////////////////////////////////
// Suspend object
////////////////////////////////////////////////////////////////////////////////
void CLadder::Suspend(void)
	{
	m_sSuspend++;
	}


////////////////////////////////////////////////////////////////////////////////
// Resume object
////////////////////////////////////////////////////////////////////////////////
void CLadder::Resume(void)
	{
	m_sSuspend--;
	}

////////////////////////////////////////////////////////////////////////////////
// Update object
////////////////////////////////////////////////////////////////////////////////
void CLadder::Update(void)
	{
	// Do schtuff.
	}

////////////////////////////////////////////////////////////////////////////////
// Render object
////////////////////////////////////////////////////////////////////////////////
void CLadder::Render(void)
	{
	// Doesn't normally draw anything (see EditRender() for render during
	// edit mode).
	}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to init new object at specified position
////////////////////////////////////////////////////////////////////////////////
short CLadder::EditNew(								// Returns 0 if successfull, non-zero otherwise
	short sX,												// In:  New x coord
	short sY,												// In:  New y coord
	short sZ)												// In:  New z coord
	{
	short sResult = 0;
	
	// Use specified position
	m_dX = (double)sX;
	m_dY = (double)sY;
	m_dZ = (double)sZ;

	sResult	= Init();

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to modify object
////////////////////////////////////////////////////////////////////////////////
short CLadder::EditModify(void)
	{
	short	sResult	= 0;

	RGuiItem* pgui = RGuiItem::LoadInstantiate(FullPathVD(GUI_FILE_NAME));
	if (pgui != NULL)
		{
		RGuiItem*	pguiLen		= pgui->GetItemFromId(GUI_ID_LEN);
		RGuiItem*	pguiHeight	= pgui->GetItemFromId(GUI_ID_HEIGHT);
		RGuiItem*	pguiRot		= pgui->GetItemFromId(GUI_ID_ROT);
		if (pguiLen != NULL && pguiHeight != NULL && pguiRot != NULL)
			{
			// Set text.
			pguiLen->SetText("%hd", m_sLen);
			// Realize text.
			pguiLen->Compose();
			
			// Set text.
			pguiHeight->SetText("%hd", m_sHeight);
			// Realize text.
			pguiHeight->Compose();
			
			// Set text.
			pguiRot->SetText("%hd", m_sRotY);
			// Realize text.
			pguiRot->Compose();
			
			if (DoGui(pgui) == 1)
				{
				// Get new values.
				m_sLen		= pguiLen->GetVal();
				m_sHeight	= pguiHeight->GetVal();
				m_sRotY		= rspMod360(pguiRot->GetVal() );
				}
			else
				{
				sResult	= 1;
				}
			}
		else
			{
			sResult	= -2;
			}
		
		// Done with GUI.
		delete pgui;
		}
	else
		{
		sResult	= -1;
		}

	// If successful so far . . .
	if (sResult == 0)
		{
		// Load resources and initialize.
		sResult = Init();
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to move object to specified position
////////////////////////////////////////////////////////////////////////////////
short CLadder::EditMove(								// Returns 0 if successfull, non-zero otherwise
	short sX,												// In:  New x coord
	short sY,												// In:  New y coord
	short sZ)												// In:  New z coord
	{
	m_dX = (double)sX;
	m_dY = (double)sY;
	m_dZ = (double)sZ;

	return 0;
	}

////////////////////////////////////////////////////////////////////////////////
// Called by editor to get the clickable pos/area of an object in 2D.
// (virtual	(Overridden here)).
////////////////////////////////////////////////////////////////////////////////
void CLadder::EditRect(	// Returns nothiing.
	RRect*	prc)				// Out: Clickable pos/area of object.
	{
	prc->sX	= m_dX;
	prc->sY	= m_dZ - m_dY;
	prc->sW	= 10;	// Safety.
	prc->sH	= 10;	// Safety.

	if (m_sprite.m_pImage != NULL)
		{
		prc->sW	= m_sprite.m_pImage->m_sWidth;
		prc->sH	= m_sprite.m_pImage->m_sHeight;
		}

	prc->sX	-= prc->sW / 2;
	prc->sY	-= prc->sH / 2;
	}

////////////////////////////////////////////////////////////////////////////////
// Called by editor to get the hotspot of an object in 2D.
// (virtual	(Overridden here)).
////////////////////////////////////////////////////////////////////////////////
void CLadder::EditHotSpot(	// Returns nothiing.
	short*	psX,					// Out: X coord of 2D hotspot relative to
										// EditRect() pos.
	short*	psY)					// Out: Y coord of 2D hotspot relative to
										// EditRect() pos.
	{
	*psX	= 0;	// Safety.
	*psY	= 0;	// Safety.

	if (m_sprite.m_pImage != NULL)
		{
		*psX	= m_sprite.m_pImage->m_sWidth / 2;
		*psY	= m_sprite.m_pImage->m_sHeight / 2;
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Called by editor to update object
////////////////////////////////////////////////////////////////////////////////
void CLadder::EditUpdate(void)
	{
	// The editor schtuff.
	}

////////////////////////////////////////////////////////////////////////////////
// Called by editor to render object
////////////////////////////////////////////////////////////////////////////////
void CLadder::EditRender(void)
	{
	// Map from 3d to 2d coords
	Map3Dto2D(
		(short) m_dX, 
		(short) m_dY, 
		(short) m_dZ, 
		&m_sprite.m_sX2, 
		&m_sprite.m_sY2);

	// Center on image.
	m_sprite.m_sX2	-= m_sprite.m_pImage->m_sWidth / 2;
	m_sprite.m_sY2	-= m_sprite.m_pImage->m_sHeight / 2;

	// Priority is based on bottom edge of sprite on X/Z plane.
	m_sprite.m_sPriority = m_dZ + m_sprite.m_pImage->m_sHeight / 2;
		
	// Layer should be based on info we get from attribute map.
	m_sprite.m_sLayer = CRealm::GetLayerViaAttrib(m_pRealm->GetLayer((short) m_dX, (short) m_dZ));

	// Update sprite in scene
	m_pRealm->m_scene.UpdateSprite(&m_sprite);
	}


////////////////////////////////////////////////////////////////////////////////
// Initialize object.
////////////////////////////////////////////////////////////////////////////////
short CLadder::Init(void)	// Returns 0 on success.
	{
	short	sRes	= GetResources();

	// Set up collision object.
	m_smashTop.m_bits								= CSmash::Ladder;
	m_smashTop.m_sphere.sphere.lRadius		= LADDER_ENDS_RADIUS;
	m_smashTop.m_sphere.sphere.X				= m_dX + COSQ[m_sRotY] * m_sLen;
	m_smashTop.m_sphere.sphere.Y				= m_dY + m_sHeight;
	m_smashTop.m_sphere.sphere.Z				= m_dZ - SINQ[m_sRotY] * m_sLen;
	m_smashBottom.m_bits							= CSmash::Ladder;
	m_smashBottom.m_sphere.sphere.lRadius	= LADDER_ENDS_RADIUS;
	m_smashBottom.m_sphere.sphere.X			= m_dX;
	m_smashBottom.m_sphere.sphere.Y			= m_dY;
	m_smashBottom.m_sphere.sphere.Z			= m_dZ;
	// Update the smash.
	m_pRealm->m_smashatorium.Update(&m_smashTop);
	m_pRealm->m_smashatorium.Update(&m_smashBottom);

	return sRes;
	}

////////////////////////////////////////////////////////////////////////////////
// Get all required resources
////////////////////////////////////////////////////////////////////////////////
short CLadder::GetResources(void)						// Returns 0 if successfull, non-zero otherwise
	{
	short sResult = 0;

	// Safe to call even if no resource.
	FreeResources();

	sResult	= rspGetResource(
		&g_resmgrGame, 
		m_pRealm->Make2dResPath(LADDER_IMAGE_FILENAME),
		&m_sprite.m_pImage);

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Free all resources
////////////////////////////////////////////////////////////////////////////////
short CLadder::FreeResources(void)						// Returns 0 if successfull, non-zero otherwise
	{
	short sResult = 0;

	if (m_sprite.m_pImage != NULL)
		{
		rspReleaseResource(&g_resmgrGame, &m_sprite.m_pImage);
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Call to to try to get on the ladder.
// This function will return false if there is already someone on the
// ladder.
////////////////////////////////////////////////////////////////////////////////
bool CLadder::GetOn(		// Returns true, if able to get on, 
								// false otherwise.
	CCharacter* pchar)	// In:  Character attempting to get onto ladder.
	{
	bool	bGotOn	= true;	// Assume success.

	if (m_pcharLadderBoy == NULL)
		{
		m_pcharLadderBoy	= pchar;
		}
	else
		{
		bGotOn	= false;
		}

	return bGotOn;
	}

////////////////////////////////////////////////////////////////////////////////
// Call when you get off the ladder.
// Only call this function, if you have made a successful call to GetOn()
// and have not, since, made a call to this function.
////////////////////////////////////////////////////////////////////////////////
void CLadder::GetOff(void)	// Returns nothing.
	{
	m_pcharLadderBoy	= NULL;
	}

////////////////////////////////////////////////////////////////////////////////
// Get the next position on the ladder.
////////////////////////////////////////////////////////////////////////////////
void CLadder::GetNextPos(	// Returns nothing.
	double*	pdX,				// In:  Current x position.
									// Out: New x position.
	double*	pdY,				// In:  Current y position.
									// Out: New y position.
	double*	pdZ,				// In:  Current z position.
									// Out: New z position.
	double	dDistance)		// In:  Distance to travel. Positive is up.
	{
	TRACE("GetNextPos(): NYI!!\n");
	}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
