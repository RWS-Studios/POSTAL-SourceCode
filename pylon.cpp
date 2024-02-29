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
// pylon.cpp
// Project: Nostril (aka Postal)
//
// This module implements the bouy marker for use with the network navagation
//	system that will help the enemy guys get around the world.
//
// History:
//
//		05/01/97 BRH	Started this object from the Bouy.  It will take over the
//							responsibility of logic suggestions and locations for the 
//							enemy AI and the bouys will go back to dealing only with
//							navigation.
//
//		05/02/97 BRH	Added message processing and changed the Popout and 
//							ShootCycle messages to be similar in their parameters.
//							Stores the information about the other pylon and the
//							dude when they become available.
//
//		05/05/97 BRH	Fixed problems in the EditModify and the EditNew.  Now
//							properly sets the image for the sprite.
//
//		05/06/97	JMI	GetThingByID() was being passed a pylon pointer instead of
//							a pointer to a pylon pointer.
//
//		05/17/97 BRH	Moved the clearing of the triggered flag to the top of
//							Update so that it will stay alive an entire interation.
//
//		05/29/97	JMI	Removed ASSERT on m_pRealm->m_pAttribMap which no longer
//							exists.
//
//		06/17/97 MJR	Moved some vars that were CPylon statics into the realm
//							so they could be instantiated on a realm-by-realm basis.
//
//		06/30/97	JMI	Moved EditRect() and EditHotSpot() from pylon.h to 
//							pylon.cpp.
//							Also, converted EditRect(), EditRender(), and/or Render()
//							to use Map3Dto2D().
//
//		06/30/97	JMI	Now maps the Z to 3D when loading fileversions previous to
//							24.
//
//		07/09/97	JMI	Now uses m_pRealm->Make2dResPath() to get the fullpath
//							for 2D image components.
//
//		07/14/97	JMI	Fixed two spots that did not check the return value from
//							GetThingById() before using the passed thing pointer.
//							Now EditModify() sets m_msg.msg_Popout.ucIDNext to 0, if
//							the user's specified pylon ID did not exist.
//							A dilemna existed for GetPylon(), if the user entered an
//							invalid pylon ID in the editor OR, more likely, the user 
//							entered a valid pylon ID and then deleted the pylon, what
//							did GetPylon() return as the end pylon?  It currently
//							returns itself since that pylon is guaranteed to exist.
//							/shrug.
//
//		08/05/97	JMI	Changed priority to use Z position rather than 2D 
//							projected Y position.
//
////////////////////////////////////////////////////////////////////////////////
#define PYLON_CPP

#include "RSPiX.h"
#include <math.h>

#include "pylon.h"

////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////

#define IMAGE_FILE			"pylon.bmp"

////////////////////////////////////////////////////////////////////////////////
// Variables/data
////////////////////////////////////////////////////////////////////////////////

// These are default values -- actually values are set using the editor!

// Let this auto-init to 0
short CPylon::ms_sFileCount;


////////////////////////////////////////////////////////////////////////////////
// Load object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CPylon::Load(										// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,											// In:  File to load from
	bool bEditMode,										// In:  True for edit mode, false otherwise
	short sFileCount,										// In:  File count (unique per file, never 0)
	ULONG	ulFileVersion)									// In:  Version of file format to load.
{
	// Call the base load to get the u16InstanceID
	short sResult = CThing::Load(pFile, bEditMode, sFileCount, ulFileVersion);
	if (sResult == 0)
	{
		// Load common data just once per file (not with each object)
		if (ms_sFileCount != sFileCount)
		{
			ms_sFileCount = sFileCount;

			// Load static data
			switch (ulFileVersion)
			{
				default:
				case 1:
					break;
			}
		}

		// Load object data
		pFile->Read(&m_dX);
		pFile->Read(&m_dY);
		pFile->Read(&m_dZ);

		// Switch on the parts that have changed
		switch (ulFileVersion)
		{
			default:
			case 6:
				pFile->Read(&m_ucID);
				m_msg.Load(pFile);
				break;

			case 5:
			case 4:
			case 3:
			case 2:
			case 1:
			case 0:
				break;

		}

		// If the file version is earlier than the change to real 3D coords . . .
		if (ulFileVersion < 24)
			{
			// Convert to 3D.
			m_pRealm->MapY2DtoZ3D(
				m_dZ,
				&m_dZ);
			}

		// Make sure there were no file errors or format errors . . .
		if (!pFile->Error() && sResult == 0)
		{
			// ONLY IN EDIT MODE . . .
			if (bEditMode == true)
				{
				// Get resources
	//			sResult = GetResources();
				}
		}
		else
		{
			sResult = -1;
			TRACE("CPylon::Load(): Error reading from file!\n");
		}
	}
	else
	{
		TRACE("CPylon::Load(): CThing::Load() failed.\n");
	}

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Save object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CPylon::Save(										// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,											// In:  File to save to
	short sFileCount)										// In:  File count (unique per file, never 0)
{
	// Call the base class save to save the u16InstanceID
	CThing::Save(pFile, sFileCount);

	// Save common data just once per file (not with each object)
	if (ms_sFileCount != sFileCount)
	{
		ms_sFileCount = sFileCount;

		// Save static data
	}

	// Save object data
	pFile->Write(&m_dX);
	pFile->Write(&m_dY);
	pFile->Write(&m_dZ);

	pFile->Write(&m_ucID);

	m_msg.Save(pFile);

	if (pFile->Error())
		return FAILURE;
	else
		return SUCCESS;
}


////////////////////////////////////////////////////////////////////////////////
// Startup object
////////////////////////////////////////////////////////////////////////////////
short CPylon::Startup(void)								// Returns 0 if successfull, non-zero otherwise
{
	short sResult = 0;

	// At this point we can assume the CHood was loaded, so we init our height
	m_dY = m_pRealm->GetHeight((short) m_dX, (short) m_dZ);

	// Init other stuff
	sResult = GetResources();

	// Update sphere.
	m_smash.m_sphere.sphere.X			= m_dX;
	m_smash.m_sphere.sphere.Y			= m_dY;
	m_smash.m_sphere.sphere.Z			= m_dZ;
	m_smash.m_sphere.sphere.lRadius	= m_sprite.m_pImage->m_sWidth;
	m_smash.m_bits		= CSmash::Pylon;
	m_smash.m_pThing	= this;

	// Update the smash.
	m_pRealm->m_smashatorium.Update(&m_smash);

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Shutdown object
////////////////////////////////////////////////////////////////////////////////
short CPylon::Shutdown(void)							// Returns 0 if successfull, non-zero otherwise
{
	return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Suspend object
////////////////////////////////////////////////////////////////////////////////
void CPylon::Suspend(void)
{
	m_sSuspend++;
}


////////////////////////////////////////////////////////////////////////////////
// Resume object
////////////////////////////////////////////////////////////////////////////////
void CPylon::Resume(void)
{
	m_sSuspend--;

	// If we're actually going to start updating again, we need to reset
	// the time so as to ignore any time that passed while we were suspended.
	// This method is far from precise, but I'm hoping it's good enough.
}


////////////////////////////////////////////////////////////////////////////////
// Update object
////////////////////////////////////////////////////////////////////////////////
void CPylon::Update(void)
{
	// Clear the target dude.
	m_u16TargetDudeID = 0;

	// Check for Dude trigger messages and set the flag
	ProcessMessages();
}



////////////////////////////////////////////////////////////////////////////////
// Render object
////////////////////////////////////////////////////////////////////////////////
void CPylon::Render(void)
{
}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to init new object at specified position
////////////////////////////////////////////////////////////////////////////////
short CPylon::EditNew(									// Returns 0 if successfull, non-zero otherwise
	short sX,												// In:  New x coord
	short sY,												// In:  New y coord
	short sZ)												// In:  New z coord
{
	short sResult = 0;
	
	// Use specified position
	m_dX = (double)sX;
	m_dY = (double)sY;
	m_dZ = (double)sZ;

	// Get Pylon ID
	m_ucID = GetFreePylonID();

	// Load resources
	sResult = GetResources();

	// Update sphere.
	m_smash.m_sphere.sphere.X			= m_dX;
	m_smash.m_sphere.sphere.Y			= m_dY;
	m_smash.m_sphere.sphere.Z			= m_dZ;
	m_smash.m_sphere.sphere.lRadius	= m_sprite.m_pImage->m_sWidth;
	m_smash.m_bits		= CSmash::Pylon;
	m_smash.m_pThing	= this;

	// Update the smash.
	m_pRealm->m_smashatorium.Update(&m_smash);

	return sResult;
}

inline
void SetText(					// Returns nothing.
	RGuiItem*	pguiRoot,	// In:  Root GUI.
	long			lId,			// In:  ID of GUI to set text.
	long			lVal)			// In:  Value to set text to.
	{
	RGuiItem*	pgui	= pguiRoot->GetItemFromId(lId);
	if (pgui != NULL)
		{
		pgui->SetText("%ld", lVal);
		pgui->Compose(); 
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Called by editor to modify object
////////////////////////////////////////////////////////////////////////////////
short CPylon::EditModify(void)
{
	short sResult = 0;
	RGuiItem* pGui = RGuiItem::LoadInstantiate(FullPathVD("res/editor/bouy.gui"));
	RGuiItem* pSecondaryGui = NULL;
	if (pGui)
	{
		RListBox* pList = (RListBox*) pGui->GetItemFromId(3);
		if (pList)
		{
			switch(m_msg.msg_Generic.eType)
			{
				case typePopout:
					pList->SetSel(pGui->GetItemFromId(21));
					break;

				case typeShootCycle:
					pList->SetSel(pGui->GetItemFromId(23));
					break;

				case typeSafeSpot:
					pList->SetSel(pGui->GetItemFromId(22));
					break;

				default:
					pList->SetSel(pGui->GetItemFromId(20));
					break;
			}

			sResult = DoGui(pGui);
			if (sResult == 1)
			{
				{
					RGuiItem* pSelection = pList->GetSel();																
					if (pSelection)
					{
						switch (pSelection->m_lId)
						{
/*
							// Start Popout dialog
							case 21:
								pSecondaryGui = RGuiItem::LoadInstantiate(FullPathVD("res/editor/bouy_pop.gui"));
								if (pSecondaryGui)
								{
									SetText(pSecondaryGui, 3, m_msg.msg_Popout.sAngle);
									SetText(pSecondaryGui, 4, m_msg.msg_Popout.sDistance);
									sResult = DoGui(pSecondaryGui);
									if (sResult == 1)
									{
										// Get results back and store in message
										m_msg.msg_Popout.sAngle = pSecondaryGui->GetVal(3);
										m_msg.msg_Popout.sDistance = pSecondaryGui->GetVal(4);
									}
								}
								break;
*/
							// SafeSpot
							case 20:
								m_msg.msg_SafeSpot.eType = typeSafeSpot;
								break;


							// Popout dialog (same as run and shoot)
							case 21:
								pSecondaryGui = RGuiItem::LoadInstantiate(FullPathVD("res/editor/bouy_rs.gui"));
								if (pSecondaryGui)
								{
									SetText(pSecondaryGui, 3, m_msg.msg_Popout.ucIDNext);
									sResult = DoGui(pSecondaryGui);
									if (sResult == 1)
									{
										// Get results back and store in message
										m_msg.msg_Popout.eType = typePopout;
										m_msg.msg_Popout.ucIDNext = pSecondaryGui->GetVal(3);
										m_msg.msg_Popout.u16UniquePylonID = GetPylonUniqueID(m_msg.msg_Popout.ucIDNext);
										CPylon* pPylon;
										if (m_pRealm->m_idbank.GetThingByID((CThing**) &pPylon, m_msg.msg_Popout.u16UniquePylonID) == 0)
										{
											m_msg.msg_Popout.sNextPylonX = pPylon->GetX();
											m_msg.msg_Popout.sNextPylonZ = pPylon->GetZ();
										}
										else
										{
											m_msg.msg_Popout.ucIDNext	= 0;
										}
										
									}
								}
								break;

							// Run & Shoot Cycle dialog box
							case 23:
								pSecondaryGui = RGuiItem::LoadInstantiate(FullPathVD("res/editor/bouy_rs.gui"));
								if (pSecondaryGui)
								{
									SetText(pSecondaryGui, 3, m_msg.msg_ShootCycle.ucIDNext);
									sResult = DoGui(pSecondaryGui);
									if (sResult == 1)
									{
										// Get results back and store in message
										m_msg.msg_ShootCycle.eType = typeShootCycle;
										m_msg.msg_ShootCycle.ucIDNext = pSecondaryGui->GetVal(3);
										m_msg.msg_ShootCycle.u16UniquePylonID = GetPylonUniqueID(m_msg.msg_ShootCycle.ucIDNext);
										CPylon* pPylon;
										if (m_pRealm->m_idbank.GetThingByID((CThing**) &pPylon, m_msg.msg_ShootCycle.u16UniquePylonID) == 0)
										{
											m_msg.msg_ShootCycle.sNextPylonX = pPylon->GetX();
											m_msg.msg_ShootCycle.sNextPylonZ = pPylon->GetZ();
										}
										else
										{
											m_msg.msg_Popout.ucIDNext	= 0;
										}
									}
								}
								break;

						}
					}
				}
			}
		}
	}
	delete pGui;
	delete pSecondaryGui;

	return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to move object to specified position
////////////////////////////////////////////////////////////////////////////////
short CPylon::EditMove(									// Returns 0 if successfull, non-zero otherwise
	short sX,												// In:  New x coord
	short sY,												// In:  New y coord
	short sZ)												// In:  New z coord
{
	m_dX = (double)sX;
	m_dY = (double)sY;
	m_dZ = (double)sZ;

	// Update sphere.
	m_smash.m_sphere.sphere.X			= m_dX;
	m_smash.m_sphere.sphere.Y			= m_dY;
	m_smash.m_sphere.sphere.Z			= m_dZ;
	m_smash.m_sphere.sphere.lRadius	= m_sprite.m_pImage->m_sWidth;
	m_smash.m_bits		= CSmash::Pylon;
	m_smash.m_pThing	= this;

	// Update the smash.
	m_pRealm->m_smashatorium.Update(&m_smash);

	return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to update object
////////////////////////////////////////////////////////////////////////////////
void CPylon::EditUpdate(void)
{
}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to render object
////////////////////////////////////////////////////////////////////////////////
void CPylon::EditRender(void)
{
	// No special flags
	m_sprite.m_sInFlags = 0;

	// Map from 3d to 2d coords
	Map3Dto2D(
		(short) m_dX, 
		(short) m_dY, 
		(short) m_dZ, 
		&m_sprite.m_sX2, 
		&m_sprite.m_sY2);

	// Priority is based on bottom edge of sprite
	m_sprite.m_sPriority = m_dZ;

	// Center on image.
	m_sprite.m_sX2	-= m_pImage->m_sWidth / 2;
	m_sprite.m_sY2	-= m_pImage->m_sHeight;

	// Layer should be based on info we get from attribute map.
	m_sprite.m_sLayer = CRealm::GetLayerViaAttrib(m_pRealm->GetLayer((short) m_dX, (short) m_dZ));

	// Image would normally animate, but doesn't for now
	m_sprite.m_pImage = m_pImage;

	// Update sprite in scene
	m_pRealm->m_scene.UpdateSprite(&m_sprite);
}

////////////////////////////////////////////////////////////////////////////////
// Give Edit a rectangle around this object
////////////////////////////////////////////////////////////////////////////////
void CPylon::EditRect(RRect* pRect)
{
	Map3Dto2D(
		m_dX,
		m_dY,
		m_dZ,
		&(pRect->sX),
		&(pRect->sY) );

	pRect->sW	= 10;	// Safety.
	pRect->sH	= 10;	// Safety.

	if (m_pImage != NULL)
		{
		pRect->sW	= m_pImage->m_sWidth;
		pRect->sH	= m_pImage->m_sHeight;
		}

	pRect->sX	-= pRect->sW / 2;
	pRect->sY	-= pRect->sH;
}

////////////////////////////////////////////////////////////////////////////////
// Called by editor to get the hotspot of an object in 2D.
////////////////////////////////////////////////////////////////////////////////
void CPylon::EditHotSpot(	// Returns nothiing.
	short*	psX,				// Out: X coord of 2D hotspot relative to
									// EditRect() pos.
	short*	psY)				// Out: Y coord of 2D hotspot relative to
									// EditRect() pos.
	{
	// Base of pylon is hotspot.
	*psX	= (m_pImage->m_sWidth / 2);
	*psY	= m_pImage->m_sHeight;
	}


////////////////////////////////////////////////////////////////////////////////
// Get all required resources
////////////////////////////////////////////////////////////////////////////////
short CPylon::GetResources(void)						// Returns 0 if successfull, non-zero otherwise
	{
	short sResult = 0;
	
	if (m_pImage == 0)
		{
		RImage*	pimBouyRes;
		sResult = rspGetResource(&g_resmgrGame, m_pRealm->Make2dResPath(IMAGE_FILE), &pimBouyRes);
		if (sResult == 0)
			{
			// Allocate image . . .
			m_pImage	= new RImage;
			if (m_pImage != NULL)
				{
				// Allocate image data . . .
				if (m_pImage->CreateImage(
					pimBouyRes->m_sWidth,
					pimBouyRes->m_sHeight,
					RImage::BMP8) == 0)
					{
					// Blt bouy res.
					rspBlit(
						pimBouyRes,		// Src.
						m_pImage,		// Dst.
						0,					// Dst.
						0,					// Dst.
						NULL);			// Dst clip.

					// Put in ID.
					RPrint	print;
					print.SetFont(19, &g_fontBig);
					print.SetColor(1);
					print.SetJustifyCenter();
					print.print(
						m_pImage,			// Dst.
						0,						// Dst.
						18,						// Dst.
						"%d",					// Format.
						(short)m_ucID);	// Src.

					// Convert to efficient transparent blit format . . .
					if (m_pImage->Convert(RImage::FSPR8) != RImage::FSPR8)
						{
						sResult = -3;
						TRACE("CPylon::GetResource() - Couldn't convert to FSPR8\n");
						}
					else
						{
							m_sprite.m_pImage = m_pImage;
						}
					}
				else
					{
					sResult	= -2;
					TRACE("CPylon::GetResource() - m_pImage->CreateImage() failed.\n");
					}

				// If an error occurred after allocation . . .
				if (sResult != 0)
					{
					delete m_pImage;
					m_pImage	= NULL;
					}
				}
			else
				{
				sResult	= -1;
				TRACE("CPylon::GetResource(): Failed to allocate RImage.\n");
				}
			
			rspReleaseResource(&g_resmgrGame, &pimBouyRes);
			}
		}
	
	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Free all resources
////////////////////////////////////////////////////////////////////////////////
short CPylon::FreeResources(void)						// Returns 0 if successfull, non-zero otherwise
{
	if (m_pImage != NULL)
		{
		delete m_pImage;
		m_pImage	= NULL;
		}

	return 0;
}


////////////////////////////////////////////////////////////////////////////////
// MessageRequest - Other Things can ask for the hint message from this bouy
////////////////////////////////////////////////////////////////////////////////

void CPylon::MessageRequest(CThing* pRequestingThing)
{
	if (pRequestingThing)
	{
		SendThingMessage(&m_msg, pRequestingThing);
	}
}

////////////////////////////////////////////////////////////////////////////////
// GetFreePylonID - Scan the list of CPylons and make sure that the ID is
//						  not used before it is given out.
////////////////////////////////////////////////////////////////////////////////

UCHAR CPylon::GetFreePylonID(void)
{
	UCHAR id = m_pRealm->m_ucNextPylonID;

	if (m_pRealm->m_sNumPylons >= PYLON_MAX_PYLONS)
		return 0;

	CListNode<CThing>* pNext = NULL; 
	bool bIdInUse = false;

	do
	{
		bIdInUse = false;
		// Loop through list of CPylons and see if they already have this ID
		pNext = m_pRealm->m_aclassHeads[CThing::CPylonID].m_pnNext;
		while (pNext && pNext->m_powner && !bIdInUse)
		{
			if (((CPylon*) pNext->m_powner)->m_ucID == id)
				bIdInUse = true;
			else
				pNext = pNext->m_pnNext;
		}

		if (bIdInUse)
		{
			if (id == 255)
				id = 1;
			else
				id++;
		}
			
	} while (bIdInUse);

	// Will wrap when it goes past 255.  Even though number of pylons is
	// limited to PYLON_MAX_PYLONS, this var just keeps counting up, so if
	// you delete a bunch and then create a bunch more, this could wrap past
	// PYLON_MAX_PYLONS.  The point is that it's safe.
	m_pRealm->m_ucNextPylonID = id + 1;

	return id;
}

////////////////////////////////////////////////////////////////////////////////
// GetPylon
////////////////////////////////////////////////////////////////////////////////

CPylon* CPylon::GetPylon(UCHAR ucPylonID)
{
	CPylon* pPylon = NULL;;

	if (m_pRealm->m_idbank.GetThingByID((CThing**) &pPylon, GetPylonUniqueID(ucPylonID)) != SUCCESS)
		pPylon = this;

	return pPylon;
}

////////////////////////////////////////////////////////////////////////////////
// GetPylonUniqueID - loop through list of pylons to get Unique ID
////////////////////////////////////////////////////////////////////////////////

U16 CPylon::GetPylonUniqueID(UCHAR ucPylonID)
{
	U16 u16UniqueID = CIdBank::IdNil;
	CListNode<CThing>* pNext = m_pRealm->m_aclassHeads[CThing::CPylonID].m_pnNext;
	
	bool bSearching = true;

	while (pNext && pNext->m_powner && bSearching)
	{
		if (((CPylon*) pNext->m_powner)->m_ucID == ucPylonID)
		{
			u16UniqueID = ((CThing*) pNext->m_powner)->GetInstanceID();			
			bSearching = false;
		}
		else
			pNext = pNext->m_pnNext;
	}		

	return u16UniqueID;
}

////////////////////////////////////////////////////////////////////////////////
// ProcessMessages - Similar to the base class version but handles a few more
////////////////////////////////////////////////////////////////////////////////

void CPylon::ProcessMessages(void)
{
	double dMinSqDistance = 1.0e200;
	double dTempSqDistance = 0;

	// Check queue of messages.
	GameMessage	msg;
	while (m_MessageQueue.DeQ(&msg) == true)
	{

		switch(msg.msg_Generic.eType)
		{
			case typeDudeTrigger:
				dTempSqDistance = (m_dX-msg.msg_DudeTrigger.dX) * (m_dX-msg.msg_DudeTrigger.dX) +
				                  (m_dZ-msg.msg_DudeTrigger.dZ) * (m_dZ-msg.msg_DudeTrigger.dZ);
				if (dTempSqDistance < dMinSqDistance)
				{
					m_msg.msg_Popout.u16UniqueDudeID = m_u16TargetDudeID = msg.msg_DudeTrigger.u16DudeUniqueID;
					dMinSqDistance = dTempSqDistance;
				}
				break;
				
		}

	}

}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
