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
// SndRelay.cpp
// Project: Nostril (aka Postal)
// 
// History:
//		08/11/97 JMI	Stole infrastructure from SoundThing.
//
//		08/11/97	JMI	Now verifies that the parent ID is that of a CSoundThing
//							before using it.  Don't know what would happen otherwise
//							but I'm sure it would suck.
//
//		09/27/99	JMI	Eliminated boolean performance warnings.
//
//////////////////////////////////////////////////////////////////////////////
//
// This CThing-derived class will relay sound volumes based on 
//	DistanceToVolume() (i.e., the distance to the ear (usually the local dude))
// to the selected CSoundThing.
//
//////////////////////////////////////////////////////////////////////////////
#define SNDRELAY_CPP

#include "RSPiX.h"
#include "SndRelay.h"
#include "SoundThing.h"
#include "game.h"

// This class has its own GetRandom() to keep it from de-synching the game.
#ifdef GetRandom
	#undef GetRandom
#endif

#ifdef GetRand
	#undef GetRand
#endif


////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////

#define IMAGE_FILE			"soundSatellite.bmp"

#define GUI_FILE_NAME		"res/editor/SndRelay.gui"

////////////////////////////////////////////////////////////////////////////////
// Class statics.
////////////////////////////////////////////////////////////////////////////////

short	CSndRelay::ms_sFileCount			= 0;	// File count.         

////////////////////////////////////////////////////////////////////////////////
// Load object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CSndRelay::Load(								// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,											// In:  File to load from
	bool bEditMode,										// In:  True for edit mode, false otherwise
	short sFileCount,										// In:  File count (unique per file, never 0)
	ULONG	ulFileVersion)									// In:  Version of file format to load.
	{
	short sResult = CThing::Load(pFile, bEditMode, sFileCount, ulFileVersion);
	if (sResult == 0)
		{
		// If new file . . . 
		if (sFileCount != ms_sFileCount)
			{
			ms_sFileCount	= sFileCount;
			
			// Do one time stuff.
			}

		switch (ulFileVersion)
			{
			default:
			case 44:
			case 43:
			case 42:
			case 41:
			case 40:
			case 39:
			case 38:
			case 37:
			case 36:
			case 35:
			case 34:
			case 33:
			case 32:
			case 31:
			case 30:
			case 29:
			case 28:
			case 27:
			case 26:
			case 25:
			case 24:
			case 23:
			case 22:
			case 21:
			case 20:
			case 19:
			case 18:
			case 17:
			case 16:
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

				long	lBool;
				pFile->Read(&lBool);
				m_bInitiallyEnabled	= lBool ? true : false;

				pFile->Read(&m_idParent);

				break;
			}

		// Make sure there were no file errors or format errors . . .
		if (!pFile->Error() && sResult == 0)
			{
			sResult = Init();
			}
		else
			{
			sResult = -1;
			TRACE("CSndRelay::Load(): Error reading from file!\n");
			}
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Save object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CSndRelay::Save(										// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,											// In:  File to save to
	short sFileCount)										// In:  File count (unique per file, never 0)
	{
	short	sResult	= CThing::Save(pFile, sFileCount);
	if (sResult == 0)
		{
		pFile->Write(m_dX);
		pFile->Write(m_dY);
		pFile->Write(m_dZ);
		pFile->Write((long)m_bInitiallyEnabled);
		pFile->Write(m_idParent);

		// Make sure there were no file errors
		sResult	= pFile->Error();
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Startup object
////////////////////////////////////////////////////////////////////////////////
short CSndRelay::Startup(void)								// Returns 0 if successfull, non-zero otherwise
	{
	return 0;
	}


////////////////////////////////////////////////////////////////////////////////
// Shutdown object
////////////////////////////////////////////////////////////////////////////////
short CSndRelay::Shutdown(void)							// Returns 0 if successfull, non-zero otherwise
	{
	return 0;
	}


////////////////////////////////////////////////////////////////////////////////
// Suspend object
////////////////////////////////////////////////////////////////////////////////
void CSndRelay::Suspend(void)
	{
	m_sSuspend++;
	}


////////////////////////////////////////////////////////////////////////////////
// Resume object
////////////////////////////////////////////////////////////////////////////////
void CSndRelay::Resume(void)
	{
	m_sSuspend--;
	}


////////////////////////////////////////////////////////////////////////////////
// Update object
////////////////////////////////////////////////////////////////////////////////
void CSndRelay::Update(void)
	{
	if (!m_sSuspend)
		{
		// If enabled . . .
		if (m_bEnabled == true)
			{
			// Attempt to get ptr to our parent . . .
			CSoundThing*	pst	= NULL;	// Safety.
			if (m_pRealm->m_idbank.GetThingByID((CThing**)&pst, m_idParent) == 0)
				{
				// Make sure this is what we think it is . . .
				if (pst->GetClassID() == CSoundThingID)
					{
					// Report volume based on our distance to the ear.
					pst->RelayVolume(DistanceToVolume(m_dX, m_dY, m_dZ, pst->m_lVolumeHalfLife) );
					}
				else
					{
					TRACE("Update(): ID %hu is not a \"SoundThing\", it is a \"%s\".\n",
						pst->GetClassID(),
						ms_aClassInfo[pst->GetClassID()].pszClassName);
					}
				}
			}

		// Process messages.
		ProcessMessages();

		switch (m_state)
			{
			case State_Happy:
				break;
			case State_Delete:
				// Banzai!
				delete this;
				return ;
			}
		}
	}


////////////////////////////////////////////////////////////////////////////////
// Render object
////////////////////////////////////////////////////////////////////////////////
void CSndRelay::Render(void)
	{
	}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to init new object at specified position
////////////////////////////////////////////////////////////////////////////////
short CSndRelay::EditNew(								// Returns 0 if successfull, non-zero otherwise
	short sX,												// In:  New x coord
	short sY,												// In:  New y coord
	short sZ)												// In:  New z coord
	{
	short sResult = 0;
	
	// Use specified position
	m_dX = (double)sX;
	m_dY = (double)sY;
	m_dZ = (double)sZ;

	sResult	= EditModify();

	return sResult;
	}

////////////////////////////////////////////////////////////////////////////////
// Helper function/macro for changing a GUIs text value.
////////////////////////////////////////////////////////////////////////////////
inline void SetGuiItemVal(	// Returns nothing.
	RGuiItem*	pguiRoot,	// In:  GUI Root.
	long			lId,			// In:  ID of item whose text we'll change.
	long			lVal)			// In:  New value.
	{
	RGuiItem*	pgui	= pguiRoot->GetItemFromId(lId);
	if (pgui)
		{
		pgui->SetText("%ld", lVal);
		pgui->Compose();
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Callback from multibtn checkbox.
////////////////////////////////////////////////////////////////////////////////
static void CheckEnableGuiCall(	// Returns nothing.
	RGuiItem*	pgui_pmb)			// In:  GUI pointer to the multi button that was 
											// pressed.
	{
	ASSERT(pgui_pmb->m_type == RGuiItem::MultiBtn);

	RMultiBtn*	pmb	= (RMultiBtn*)pgui_pmb;

	// Show based on value stored in GUI.
	short	sVisible	= pmb->m_ulUserData;
	// If unchecked . . .
	if (pmb->m_sState == 1)
		{
		// Opposite show/hide state.
		sVisible	= !sVisible;
		}

	RGuiItem*	pguiLoopSettingsContainer	= pmb->GetParent()->GetItemFromId(pmb->m_ulUserInstance);
	if (pguiLoopSettingsContainer)
		{
		pguiLoopSettingsContainer->SetVisible(sVisible);
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Called by editor to modify object
////////////////////////////////////////////////////////////////////////////////
short CSndRelay::EditModify(void)
	{
	short	sResult	= 0;

	// Load gui dialog
	RGuiItem* pgui = RGuiItem::LoadInstantiate(FullPathVD(GUI_FILE_NAME));
	if (pgui != NULL)
		{
		// Init "ID" edit
		REdit* peditParentId = (REdit*)pgui->GetItemFromId(100);
		ASSERT(peditParentId != NULL);
		ASSERT(peditParentId->m_type == RGuiItem::Edit);
		if (m_idParent != CIdBank::IdNil)
			{
			peditParentId->SetText("%hu", m_idParent);
			}
		else
			{
			peditParentId->SetText("");
			}

		peditParentId->Compose();

		// Init "enable" push button
		RMultiBtn* pmbEnable = (RMultiBtn*)pgui->GetItemFromId(200);
		ASSERT(pmbEnable != NULL);
		pmbEnable->m_sState = (m_bInitiallyEnabled == true) ? 2 : 1;
		pmbEnable->Compose();

		// Run the dialog using this super-duper helper funciton
		if (DoGui(pgui) == 1)
			{
			// Get new values from dialog
			m_idParent	= (U16)peditParentId->GetVal();
			if (m_idParent == 0)
				{
				m_idParent	= CIdBank::IdNil;
				}

			m_bInitiallyEnabled = (pmbEnable->m_sState == 2) ? true : false;
			}
		else
			{
			sResult	= 1;
			}
		
		// Done with GUI.
		delete pgui;
		}
	else
		{
		sResult	= -1;
		}

	// If everything's okay, init using new values
	if (sResult == 0)
		sResult = Init();

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to move object to specified position
////////////////////////////////////////////////////////////////////////////////
short CSndRelay::EditMove(									// Returns 0 if successfull, non-zero otherwise
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
void CSndRelay::EditRect(	// Returns nothiing.
	RRect*	prc)				// Out: Clickable pos/area of object.
	{
	Map3Dto2D(
		m_dX,
		m_dY,
		m_dZ,
		&(prc->sX),
		&(prc->sY) );

	prc->sW	= 10;	// Safety.
	prc->sH	= 10;	// Safety.

	if (m_sprite.m_pImage)
		{
		prc->sW	= m_sprite.m_pImage->m_sWidth;
		prc->sH	= m_sprite.m_pImage->m_sHeight;
		}

	prc->sX	-= prc->sW / 2;
	prc->sY	-= prc->sH;
	}

////////////////////////////////////////////////////////////////////////////////
// Called by editor to get the hotspot of an object in 2D.
// (virtual	(Overridden here)).
////////////////////////////////////////////////////////////////////////////////
void CSndRelay::EditHotSpot(	// Returns nothiing.
	short*	psX,					// Out: X coord of 2D hotspot relative to
										// EditRect() pos.
	short*	psY)					// Out: Y coord of 2D hotspot relative to
										// EditRect() pos.
	{
	*psX	= 5;	// Safety.
	*psY	= 5;	// Safety.

	if (m_sprite.m_pImage)
		{
		*psX	= m_sprite.m_pImage->m_sWidth / 2;
		*psY	= m_sprite.m_pImage->m_sHeight;
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Called by editor to update object
////////////////////////////////////////////////////////////////////////////////
void CSndRelay::EditUpdate(void)
	{
	}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to render object
////////////////////////////////////////////////////////////////////////////////
void CSndRelay::EditRender(void)
	{
	// Setup simple, non-animating sprite
	m_sprite.m_sInFlags = 0;

	Map3Dto2D(
		m_dX,
		m_dY,
		m_dZ,
		&(m_sprite.m_sX2),
		&(m_sprite.m_sY2) );

	// Priority is based on bottom edge of sprite
	m_sprite.m_sPriority = m_dZ;

	// Center on image.
	m_sprite.m_sX2	-= m_sprite.m_pImage->m_sWidth / 2;
	m_sprite.m_sY2	-= m_sprite.m_pImage->m_sHeight;

	m_sprite.m_sLayer = CRealm::GetLayerViaAttrib(m_pRealm->GetLayer((short) m_dX, (short) m_dZ));

	// Update sprite in scene
	m_pRealm->m_scene.UpdateSprite(&m_sprite);
	}


////////////////////////////////////////////////////////////////////////////////
// Init object
////////////////////////////////////////////////////////////////////////////////
short CSndRelay::Init(void)							// Returns 0 if successfull, non-zero otherwise
	{
	short sResult = 0;

	Kill();

	m_bEnabled = m_bInitiallyEnabled;

	if (m_sprite.m_pImage == 0)
		{
		sResult = rspGetResource(&g_resmgrGame, m_pRealm->Make2dResPath(IMAGE_FILE), &m_sprite.m_pImage);
		if (sResult == 0)
			{
			// This is a questionable action on a resource managed item, but it's
			// okay if EVERYONE wants it to be an FSPR8.
			if (m_sprite.m_pImage->Convert(RImage::FSPR8) != RImage::FSPR8)
				{
				sResult = -1;
				TRACE("CSndRelay::GetResource() - Couldn't convert to FSPR8\n");
				}
			}
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Kill object
////////////////////////////////////////////////////////////////////////////////
short CSndRelay::Kill(void)							// Returns 0 if successfull, non-zero otherwise
	{
	if (m_sprite.m_pImage != 0)
		rspReleaseResource(&g_resmgrGame, &m_sprite.m_pImage);

	m_pRealm->m_scene.RemoveSprite(&m_sprite);

	return 0;
	}


////////////////////////////////////////////////////////////////////////////////
// Process our message queue.
////////////////////////////////////////////////////////////////////////////////
void CSndRelay::ProcessMessages(void)
	{
	// Check queue of messages.
	GameMessage	msg;
	while (m_MessageQueue.DeQ(&msg) == true)
		{
		switch(msg.msg_Generic.eType)
			{
			case typeObjectDelete:
				m_state	= State_Delete;
				break;
			}
		
		}
	}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
