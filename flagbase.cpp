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
// flagbase.cpp
// Project: Postal
//
// This module implements the flag object for capture the flag gameplay.
//
// History:
//
//		06/30/97 BRH	Started this file for the challenge levels.
//
//		07/12/97 BRH	Added FlagID so that flags can be matched with their
//							bases.  Added loading/saving thereof.  Also added
//							EditModify dialog so that the value can be set.
//
//		07/14/97 BRH	Changed to using the CSmash::Flagbase bits to identify
//							the base.  Also added checking for Flags to update
//							and incrementing the m_sFlagbaseCaptured value in realm
//							when the proper flag meets the base.
//
//		07/16/97 BRH	Changed to using the correct base files rather than
//							the bandguy as a placeholder.
//
//		08/03/97	JMI	Init() was setting the looping parms on a phot which no
//							longer exists.  Now the looping parms are passed via the
//							Get() call in GetResources() instead so they will get set
//							via the CAnim3D which should know which ones are okay to
//							use.
//
//		08/11/97 BRH	Added flagbase color option as a variable that is loaded
//							and saved and can be changed in the EditModify dialog.
//
//		08/18/97	JMI	Changed State_Dead to call DeadRender3D() (which used to be
//							known/called as just another Render() overload).
//
//		08/28/97 BRH	Set the correct bits to detect the flag base.   Finished
//							the code for capturing the flagbase.
//
////////////////////////////////////////////////////////////////////////////////
#define FLAGBASE_CPP

#include "RSPiX.h"
#include "flagbase.h"
#include "flag.h"
#include "SampleMaster.h"

////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////

#define GUI_FLAGID_EDIT_ID		103
#define GUI_COLOR_EDIT_ID		104

////////////////////////////////////////////////////////////////////////////////
// Variables/data
////////////////////////////////////////////////////////////////////////////////

// These are default values -- actually values are set using the editor!
double CFlagbase::ms_dInRange = 30 * 30;			// Sq distance to base

// Let this auto-init to 0
short CFlagbase::ms_sFileCount;

/// Throwing Animation Files ////////////////////////////////////////////////////
// An array of pointers to resource names (one for each channel of the animation)
static char* ms_apszRedResNames[] = 
{
	"3d/rbase.sop",
	"3d/rbase.mesh",
	"3d/rbase.tex",
	"3d/rbase.hot",
	"3d/rbase.bounds",
	"3d/rbase.floor",
	NULL,
	NULL
};

static char* ms_apszBlueResNames[] = 
{
	"3d/bbase.sop",
	"3d/bbase.mesh",
	"3d/bbase.tex",
	"3d/bbase.hot",
	"3d/bbase.bounds",
	"3d/bbase.floor",
	NULL,
	NULL
};


// These are the points that are checked on the attribute map relative to his origin
static RP3d ms_apt3dAttribCheck[] =
{
	{-6, 0, -6},
	{ 0, 0, -6},
	{ 6, 0, -6},
	{-6, 0,  6},
	{ 0, 0,  6},
	{ 6, 0,  6},
};


////////////////////////////////////////////////////////////////////////////////
// Load object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CFlagbase::Load(				// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,						// In:  File to load from
	bool bEditMode,					// In:  True for edit mode, false otherwise
	short sFileCount,					// In:  File count (unique per file, never 0)
	ULONG	ulFileVersion)				// In:  Version of file format to load.
{
	short sResult = 0;
	// Call the base load function to get ID, position, etc.
	sResult = CThing3d::Load(pFile, bEditMode, sFileCount, ulFileVersion);
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
					pFile->Read(&ms_dInRange);
					break;
			}
		}

		// Load other values
		switch (ulFileVersion)
		{
			default:
			case 45:
				pFile->Read(&m_u16Color);

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
			case 0:
				pFile->Read(&m_u16FlagID);
				break;
		}

		// Make sure there were no file errors or format errors . . .
		if (!pFile->Error() && sResult == 0)
		{
			// Get resources
			sResult = GetResources();
		}
		else
		{
			sResult = -1;
			TRACE("CFlagbase::Load(): Error reading from file!\n");
		}
	}
	else
	{
		TRACE("CFlagbase::Load():  CDoofus::Load() failed.\n");
	}

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Save object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CFlagbase::Save(										// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,											// In:  File to save to
	short sFileCount)										// In:  File count (unique per file, never 0)
{
	// Swap the hotspot we want to save in.

	short sResult;

	// Call the base class save to save the instance ID, position, etc
	CThing3d::Save(pFile, sFileCount);

	// Save common data just once per file (not with each object)
	if (ms_sFileCount != sFileCount)
	{
		ms_sFileCount = sFileCount;

		// Save static data
		pFile->Write(&ms_dInRange);
	}

	// Save additinal stuff here.
	pFile->Write(&m_u16Color);
	pFile->Write(&m_u16FlagID);

	if (!pFile->Error())
	{
		sResult = SUCCESS;
	}
	else
	{
		TRACE("CFlagbase::Save() - Error writing to file\n");
		sResult = -1;
	}

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Init - Call this after the resources are in place
////////////////////////////////////////////////////////////////////////////////

short CFlagbase::Init(void)
{
	short sResult = 0;

	// Prepare shadow (get resources and setup sprite).
	sResult	= PrepareShadow();

	// Init other stuff
	m_dVel = 0.0;
	m_dRot = 0.0;
	// Set to different starting state based on the design of the animation, but
	// for now, ok.  Then also set his current animation.
	m_state = CFlagbase::State_Wait;
	m_panimCur = &m_animFlagWave;
	m_lAnimTime = 0;
	m_lTimer = m_pRealm->m_time.GetGameTime() + 500;

	m_smash.m_bits = CSmash::FlagBase;
	m_smash.m_pThing = this;

	m_sBrightness = 0;	// Default Brightness level

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Startup object
////////////////////////////////////////////////////////////////////////////////
short CFlagbase::Startup(void)								// Returns 0 if successfull, non-zero otherwise
{
	short sResult = 0;

	// Set the current height, previous time, and Nav Net
	CThing3d::Startup();

	// Init other stuff
	Init();

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Shutdown object
////////////////////////////////////////////////////////////////////////////////
short CFlagbase::Shutdown(void)							// Returns 0 if successfull, non-zero otherwise
{
	short sResult = 0;

	m_trans.Make1();

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Update object
////////////////////////////////////////////////////////////////////////////////
void CFlagbase::Update(void)
{
	short sHeight = m_sPrevHeight;
	long lThisTime;
	long lTimeDifference;
	long lSqDistanceToDude = 0;
	CSmash* pSmashed = NULL;

	if (!m_sSuspend)
	{
		// Get new time
		lThisTime = m_pRealm->m_time.GetGameTime();
		lTimeDifference = lThisTime - m_lPrevTime;

		// Calculate elapsed time in seconds
		double dSeconds = (double)(lThisTime - m_lPrevTime) / 1000.0;

		// Check for new messages that may change the state
		ProcessMessages();


		switch(m_state)
		{
			case CFlagbase::State_Wait:
				if (lThisTime > m_lTimer)
				{
					m_state = CFlagbase::State_Guard;
				}
	
				// Update sphere.
				m_smash.m_sphere.sphere.X			= m_dX;
				m_smash.m_sphere.sphere.Y			= m_dY;
				m_smash.m_sphere.sphere.Z			= m_dZ;
				m_smash.m_sphere.sphere.lRadius	= 20; //m_spriteBase.m_sRadius;

				// Update the smash.
				m_pRealm->m_smashatorium.Update(&m_smash);
				break;

//-----------------------------------------------------------------------
// Guard - normal operation
//-----------------------------------------------------------------------

			case CFlagbase::State_Guard:
				m_pRealm->m_smashatorium.QuickCheckReset(&m_smash, CSmash::Flag, 0, 0);
				while (m_pRealm->m_smashatorium.QuickCheckNext(&pSmashed))
				{
					if (pSmashed->m_pThing->GetClassID() == CFlagID)
					{
						if (((CFlag*) (pSmashed->m_pThing))->m_u16FlagID == m_u16FlagID)
						{
							m_pRealm->m_sFlagbaseCaptured++;
							m_state = State_Dead;
						}	
					}
				}
				break;

//-----------------------------------------------------------------------
// Blownup - You were blown up so pop up into the air and come down dead
//-----------------------------------------------------------------------

				case CFlagbase::State_BlownUp:
					// Make her animate
					m_lAnimTime += lTimeDifference;

					if (!WhileBlownUp())
						m_state = State_Dead;
					else
					{
						UpdateFirePosition();
					}

					break;


//-----------------------------------------------------------------------
// Dead - You are dead, so lay there and decompose, then go away
//-----------------------------------------------------------------------

				case CFlagbase::State_Dead:
					CHood*	phood	= m_pRealm->m_phood;
					// Render current dead frame into background to stay.
					m_pRealm->m_scene.DeadRender3D(
						phood->m_pimBackground,		// Destination image.
						&m_sprite,						// Tree of 3D sprites to render.
						phood);							// Dst clip rect.

					delete this;
					return;
					break;


		}


				// Update sphere.
				m_smash.m_sphere.sphere.X			= m_dX;
				m_smash.m_sphere.sphere.Y			= m_dY;
				m_smash.m_sphere.sphere.Z			= m_dZ;
				m_smash.m_sphere.sphere.lRadius	= 20; //m_spriteBase.m_sRadius;

				// Update the smash.
				m_pRealm->m_smashatorium.Update(&m_smash);


		// Save time for next time
		m_lPrevTime = lThisTime;
		m_lAnimPrevUpdateTime = m_lAnimTime;
	}
}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to init new object at specified position
////////////////////////////////////////////////////////////////////////////////
short CFlagbase::EditNew(									// Returns 0 if successfull, non-zero otherwise
	short sX,												// In:  New x coord
	short sY,												// In:  New y coord
	short sZ)												// In:  New z coord
{
	short sResult = 0;

	sResult = CThing3d::EditNew(sX, sY, sZ);

	if (sResult == SUCCESS)
	{
		// Load resources
		sResult = GetResources();
		if (sResult == SUCCESS)
		{
			sResult	= Init();
		}
	}
	else
	{
		sResult = -1;
	}

	return sResult;
}

////////////////////////////////////////////////////////////////////////////////
// Edit Move
////////////////////////////////////////////////////////////////////////////////
short CFlagbase::EditMove(short sX, short sY, short sZ)
{
	short sResult = CThing3d::EditMove(sX, sY, sZ);

	return sResult;
}

////////////////////////////////////////////////////////////////////////////////
// Give Edit a rectangle around this object
////////////////////////////////////////////////////////////////////////////////
void CFlagbase::EditRect(RRect* pRect)
	{
	// Call base class.
	CThing3d::EditRect(pRect);

	}

////////////////////////////////////////////////////////////////////////////////
// Called by editor to get the hotspot of an object in 2D.
// (virtual (Overridden here)).
////////////////////////////////////////////////////////////////////////////////
void CFlagbase::EditHotSpot(			// Returns nothiing.
	short*	psX,						// Out: X coord of 2D hotspot relative to
											// EditRect() pos.
	short*	psY)						// Out: Y coord of 2D hotspot relative to
											// EditRect() pos.
	{
	// Get rectangle.
	RRect	rc;
	EditRect(&rc);
	// Get 2D hotspot.
	short	sX;
	short	sY;
	Map3Dto2D(
		m_dX,
		m_dY,
		m_dZ,
		&sX,
		&sY);

	// Get relation.
	*psX	= sX - rc.sX;
	*psY	= sY - rc.sY;
	}

////////////////////////////////////////////////////////////////////////////////
// Called by editor to modify object
////////////////////////////////////////////////////////////////////////////////
short CFlagbase::EditModify(void)
{
	short sResult = 0;
	U16 u16OrigColor = m_u16Color;
	RGuiItem* pGuiItem = NULL;
	RGuiItem* pguiRoot = RGuiItem::LoadInstantiate(FullPathVD("res/editor/flagbase.gui"));
	if (pguiRoot != NULL)
	{
		REdit* peditFlagID = (REdit*) pguiRoot->GetItemFromId(GUI_FLAGID_EDIT_ID);
		REdit* peditColor  = (REdit*) pguiRoot->GetItemFromId(GUI_COLOR_EDIT_ID);

		if (peditFlagID != NULL && peditColor != NULL)
		{
			ASSERT(peditFlagID->m_type == RGuiItem::Edit);
			ASSERT(peditColor->m_type == RGuiItem::Edit);

			peditFlagID->SetText("%d", m_u16FlagID);
			peditFlagID->Compose();
			peditColor->SetText("%d", m_u16Color);
			peditColor->Compose();

			sResult = DoGui(pguiRoot);
			if (sResult == 1)
			{
				m_u16FlagID = peditFlagID->GetVal();
				m_u16Color = MIN((long) (CFlag::EndOfColors - 1), peditColor->GetVal());
			}
		}
	}
	delete pguiRoot;

	// If the user switched colors, get the new resources
	if (m_u16Color != u16OrigColor)
	{
		FreeResources();
		GetResources();
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Get all required resources
////////////////////////////////////////////////////////////////////////////////
short CFlagbase::GetResources(void)						// Returns 0 if successfull, non-zero otherwise
{
	short sResult = 0;

	switch (m_u16Color)
	{
		default:
		case CFlag::Red:
			sResult = m_animFlagWave.Get(ms_apszRedResNames, RChannel_LoopAtStart | RChannel_LoopAtEnd);
			break;

		case CFlag::Blue:
			sResult = m_animFlagWave.Get(ms_apszBlueResNames, RChannel_LoopAtStart | RChannel_LoopAtEnd);
			break;

	}

	if (sResult == 0)
	{
						// Add new animation loads here
	}
	else
	{
		TRACE("CFlagbase::GetResources - Failed to open 3D flag waving animation\n");
	}

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Free all resources
////////////////////////////////////////////////////////////////////////////////
short CFlagbase::FreeResources(void)						// Returns 0 if successfull, non-zero otherwise
{
	m_animFlagWave.Release();

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Message handlers
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Shot Message
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Explosion message
////////////////////////////////////////////////////////////////////////////////

void CFlagbase::OnExplosionMsg(Explosion_Message* pMessage)
{
	if (
	    m_state != State_BlownUp	&&
		 m_state != State_Die		&& 
		 m_state != State_Dead)
	{
//		CCharacter::OnExplosionMsg(pMessage);
		
//		PlaySample(g_smidBlownupFemaleYell);
//		m_ePreviousState = m_state;
		m_state = State_BlownUp;
//		m_panimPrev = m_panimCur;
//		m_panimCur = &m_animDie;
		m_lAnimTime = 0;
//		m_stockpile.m_sHitPoints = 0;
		m_lTimer = m_pRealm->m_time.GetGameTime();

		m_dExtHorzVel *= 1.4; //2.5;
		m_dExtVertVel *= 1.1; //1.4;
		// Send it spinning.
		m_dExtRotVelY	= GetRandom() % 720;
		m_dExtRotVelZ	= GetRandom() % 720;

//		m_panimCur = &m_animDie;
	}
}

////////////////////////////////////////////////////////////////////////////////
// Burning message
////////////////////////////////////////////////////////////////////////////////

void CFlagbase::OnBurnMsg(Burn_Message* pMessage)
{
	// For now we made the sentry fireproof, the only
	// way it can be destroyed is by blowing it up.
}


////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
