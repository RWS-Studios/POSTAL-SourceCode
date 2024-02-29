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
// flag.cpp
// Project: Postal
//
// This module implements the flag object for capture the flag gameplay.
//
// History:
//
//		06/30/97 BRH	Started this file for the challenge levels.
//
//		07/06/97 BRH	Added time bonus.
//
//		07/12/97 BRH	Added EditModify dialog for bonus time and flag ID.
//							Added Flag ID and loading/saving thereof so that
//							a flag can be matched with a base.  Also adds bonus time 
//							to realm timer.
//
//		07/14/97 BRH	Changed smash bits to CSmash::Flag.  Incremented
//							the flag captured count in the realm.
//
//		08/03/97	JMI	Init() was setting the looping parms on a phot which no  
//							longer exists.  Now the looping parms are passed via the 
//							Get() call in GetResources() instead so they will get set
//							via the CAnim3D which should know which ones are okay to 
//							use.                                                     
//
//		08/10/97	JMI	Once blown up, the returns to the guard state but never
//							restored its smash bits (they were changed when it left
//							guard mode so, I think, the flag could search for the base).
//							Now the bits are restored to looking for dude mode.  Maybe
//							we should have two sets of bits, one set for each state.
//							Also, was only updating the smash in State_Wait.  Changed
//							so it always does it which may not be necessary.  Have to
//							ask.
//
//		08/11/97 BRH	Added a flag color variable which is loaded and saved
//							and can be changed in the EditModify dialog box.
//
//		08/18/97	JMI	Changed State_Dead to call DeadRender3D() (which used to be
//							known/called as just another Render() overload).
//
//		08/24/97 BRH	Processes the OnBurnMsg so that it can set its state
//							back to guard mode.  The Cdude drops the flag when he
//							is burned, and then forwards the mesage to the flag so
//							it can go back to the mode where it looks for someone
//							to pick it up.  Previously, when the dude dropped it in
//							this state, it was only looking for a flag base and so
//							could not be picked up again.  Added the State_Burning
//							to Update() so that the Dude couldn't pick up the flag
//							instantly after being burned.  Otherwise he kept picking
//							it up and dropping it.
//
//		08/25/97 BRH	Fixed bug where flags incremented the flag score when 
//							picked up but didn't decrement the count when dropped.
//
//		08/28/97 BRH	Set the correct bits to detect the flag base.   Finished
//							the code for capturing the flagbase.
//
//		08/30/97	JMI	Since CDude no longer contains an m_idFlagItem, this object
//							no longer sets it.  Simply adding a CFlag's sprite to the
//							dude's sprite children is sufficient for him to handle the
//							flag. 
//
////////////////////////////////////////////////////////////////////////////////
#define FLAG_CPP

#include "RSPiX.h"
#include "flag.h"
#include "flagbase.h"
#include "SampleMaster.h"
#include "dude.h"

#include "reality.h"

////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////

#define GUI_MINUTES_EDIT_ID	101
#define GUI_SECONDS_EDIT_ID	102
#define GUI_FLAGID_EDIT_ID		103
#define GUI_COLOR_EDIT_ID		104

#define FLAG_BURNED_TIMEOUT 1500

////////////////////////////////////////////////////////////////////////////////
// Variables/data
////////////////////////////////////////////////////////////////////////////////

// These are default values -- actually values are set using the editor!
double CFlag::ms_dInRange = 30 * 30;			// Sq distance to base

// Let this auto-init to 0
short CFlag::ms_sFileCount;

/// Throwing Animation Files ////////////////////////////////////////////////////
// An array of pointers to resource names (one for each channel of the animation)
static char* ms_apszRedResNames[] = 
{
	"3d/rflag.sop",
	"3d/rflag.mesh",
	"3d/rflag.tex",
	"3d/rflag.hot",
	"3d/rflag.bounds",
	"3d/rflag.floor",
	NULL,
	NULL
};

static char* ms_apszBlueResNames[] = 
{
	"3d/bflag.sop",
	"3d/bflag.mesh",
	"3d/bflag.tex",
	"3d/bflag.hot",
	"3d/bflag.bounds",
	"3d/bflag.floor",
	NULL,
	NULL
};

////////////////////////////////////////////////////////////////////////////////
// Load object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CFlag::Load(				// Returns 0 if successfull, non-zero otherwise
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
				pFile->Read(&m_u16FlagColor);

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
				pFile->Read(&m_lTimeBonus);
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
			TRACE("CFlag::Load(): Error reading from file!\n");
		}
	}
	else
	{
		TRACE("CFlag::Load():  CDoofus::Load() failed.\n");
	}

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Save object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CFlag::Save(										// Returns 0 if successfull, non-zero otherwise
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
	pFile->Write(&m_u16FlagColor);
	pFile->Write(&m_lTimeBonus);
	pFile->Write(&m_u16FlagID);

	if (!pFile->Error())
	{
		sResult = SUCCESS;
	}
	else
	{
		TRACE("CFlag::Save() - Error writing to file\n");
		sResult = -1;
	}

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Init - Call this after the resources are in place
////////////////////////////////////////////////////////////////////////////////

short CFlag::Init(void)
{
	short sResult = 0;

	// Prepare shadow (get resources and setup sprite).
	sResult	= PrepareShadow();

	// Init other stuff
	m_dVel = 0.0;
	m_dRot = 0.0;
	// Set to different starting state based on the design of the animation, but
	// for now, ok.  Then also set his current animation.
	m_state = CFlag::State_Wait;
	m_panimCur = &m_animFlagWave;
	m_lAnimTime = 0;
	m_lTimer = m_pRealm->m_time.GetGameTime() + 500;

	m_smash.m_bits = CSmash::Flag; 
	m_smash.m_pThing = this;

	m_u32IncludeBits = CSmash::Good | CSmash::Character;
	m_u32DontcareBits = 0;
	m_u32ExcludeBits = CSmash::Dead;

	m_sBrightness = 0;	// Default Brightness level

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Startup object
////////////////////////////////////////////////////////////////////////////////
short CFlag::Startup(void)								// Returns 0 if successfull, non-zero otherwise
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
short CFlag::Shutdown(void)							// Returns 0 if successfull, non-zero otherwise
{
	short sResult = 0;

	m_trans.Make1();

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Update object
////////////////////////////////////////////////////////////////////////////////
void CFlag::Update(void)
{
	short sHeight = m_sPrevHeight;
	long lThisTime;
	long lTimeDifference;
	CSmash* pSmashed = NULL;

	if (!m_sSuspend)
	{
		// Get new time
		lThisTime = m_pRealm->m_time.GetGameTime();
		lTimeDifference = lThisTime - m_lPrevTime;

		m_lAnimTime += lTimeDifference;

		// Calculate elapsed time in seconds
		double dSeconds = (double)(lThisTime - m_lPrevTime) / 1000.0;

		// Check for new messages that may change the state
		ProcessMessages();

		switch(m_state)
		{
			case CFlag::State_Wait:
				if (lThisTime > m_lTimer)
				{
					m_state = CFlag::State_Guard;
				}
				break;

//-----------------------------------------------------------------------
// Guard - normal operation
//-----------------------------------------------------------------------

			case CFlag::State_Guard:
				if (m_pRealm->m_smashatorium.QuickCheckClosest(&m_smash,
														m_u32IncludeBits,
														m_u32DontcareBits,
														m_u32ExcludeBits, &pSmashed))
				{
					// Add child to smashed
					if (pSmashed->m_pThing->GetClassID() == CDudeID)
					{
						((CDude*) pSmashed->m_pThing)->m_sprite.AddChild(&m_sprite);
						m_u16IdParent = pSmashed->m_pThing->GetInstanceID();
						m_pRealm->m_scene.RemoveSprite(&m_sprite);
						m_state = State_Patrol;
						//m_u32IncludeBits = CSmash::Flagbase;
						m_u32IncludeBits = CSmash::FlagBase;
						m_u32DontcareBits = CSmash::Good | CSmash::Bad;
						m_u32ExcludeBits = 0;
						// Add bonus time (if any) to realm timer
						m_pRealm->m_lScoreTimeDisplay += m_lTimeBonus;
						m_pRealm->m_sFlagsCaptured++;
						// Feedback.
						PlaySample(
							g_smidDemonYes2, 
							SampleMaster::Demon, 
							DistanceToVolume(m_dX, m_dY, m_dZ, 250) );
					}
				}

				// QuickCheckClosest (CDudeID);
				break;

//-----------------------------------------------------------------------
// Patrol - look for base while being carried
//-----------------------------------------------------------------------

				case CFlag::State_Patrol:
					if (m_pRealm->m_smashatorium.QuickCheckClosest(&m_smash, 
														m_u32IncludeBits,
														m_u32DontcareBits,
														m_u32ExcludeBits, &pSmashed))
					{
						if (pSmashed)
						{
							if (((CFlagbase*) pSmashed->m_pThing)->m_u16FlagID == m_u16FlagID)
							{
								m_sSavedX = pSmashed->m_sphere.sphere.X;
								m_sSavedY = pSmashed->m_sphere.sphere.Y + 15;
								m_sSavedZ = pSmashed->m_sphere.sphere.Z;

								m_pRealm->m_sFlagbaseCaptured++;

								CThing3d* pParent = NULL;
								GameMessage msg;
								msg.msg_PutMeDown.eType = typePutMeDown;
								msg.msg_PutMeDown.sPriority = 0;
								msg.msg_PutMeDown.u16FlagInstanceID = GetInstanceID();

								if (m_pRealm->m_idbank.GetThingByID((CThing**)&pParent, m_u16IdParent) == 0)
								{
									SendThingMessage(&msg, pParent);
								}
							}
						}
					}
					break;


//-----------------------------------------------------------------------
// Die - You collided with the flag base, so just set your position
//			to the base position and then go to dead.
//-----------------------------------------------------------------------

				case CFlag::State_Die:
					m_dX = m_sSavedX;
					m_dY = m_sSavedY;
					m_dZ = m_sSavedZ;
					m_smash.m_bits = 0;
					m_state = State_Dead;
					break;


//-----------------------------------------------------------------------
// Blownup - You were blown up so pop up into the air and come down
//-----------------------------------------------------------------------

				case CFlag::State_BlownUp:
					// Make her animate
					m_lAnimTime += lTimeDifference;

					if (!WhileBlownUp())
					{
						m_state = State_Guard;
						// Stop the spinning.
						m_dExtRotVelY	= 0;
						m_dExtRotVelZ	= 0;
						// Return the smash bits to guard mode (i.e., looking
						// for dude).
						m_u32IncludeBits = CSmash::Good | CSmash::Character;
						m_u32DontcareBits = 0;
						m_u32ExcludeBits = CSmash::Dead;
						m_pRealm->m_sFlagsCaptured--;
					}

					else
					{
						UpdateFirePosition();
					}

					break;

//-----------------------------------------------------------------------
// Burning - The CDude carrying you got burnt, and dropped you so wait
//				 a while before going active again so he separates from you.
//-----------------------------------------------------------------------

				case CFlag::State_Burning:
					if (lThisTime > m_lTimer)
					{
						m_state = State_Guard;

						// Return the smash bits to guard mode (i.e., looking
						// for dude).
						m_u32IncludeBits = CSmash::Good | CSmash::Character;
						m_u32DontcareBits = 0;
						m_u32ExcludeBits = CSmash::Dead;
						m_pRealm->m_sFlagsCaptured--;
					}
					break;

//-----------------------------------------------------------------------
// Dead - You are dead, so lay there and decompose, then go away
//-----------------------------------------------------------------------

				case CFlag::State_Dead:
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
		m_smash.m_sphere.sphere.lRadius	= 30; //m_spriteBase.m_sRadius;

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
short CFlag::EditNew(									// Returns 0 if successfull, non-zero otherwise
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
short CFlag::EditMove(short sX, short sY, short sZ)
{
	short sResult = CThing3d::EditMove(sX, sY, sZ);

	return sResult;
}

////////////////////////////////////////////////////////////////////////////////
// Give Edit a rectangle around this object
////////////////////////////////////////////////////////////////////////////////
void CFlag::EditRect(RRect* pRect)
	{
	// Call base class.
	CThing3d::EditRect(pRect);

	}

////////////////////////////////////////////////////////////////////////////////
// Called by editor to get the hotspot of an object in 2D.
// (virtual (Overridden here)).
////////////////////////////////////////////////////////////////////////////////
void CFlag::EditHotSpot(			// Returns nothiing.
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
short CFlag::EditModify(void)
{
	short sResult = 0;
	U16 u16OrigColor = m_u16FlagColor;

	RGuiItem* pGuiItem = NULL;
	RGuiItem* pguiRoot = RGuiItem::LoadInstantiate(FullPathVD("res/editor/flag.gui"));
	if (pguiRoot != NULL)
	{
		REdit* peditMinutes = (REdit*) pguiRoot->GetItemFromId(GUI_MINUTES_EDIT_ID);
		REdit* peditSeconds = (REdit*) pguiRoot->GetItemFromId(GUI_SECONDS_EDIT_ID);
		REdit* peditFlagID  = (REdit*) pguiRoot->GetItemFromId(GUI_FLAGID_EDIT_ID);
		REdit* peditColor   = (REdit*) pguiRoot->GetItemFromId(GUI_COLOR_EDIT_ID);
		long lMinutes;
		long lSeconds;

		if (peditMinutes != NULL && peditSeconds != NULL && peditFlagID != NULL && peditColor != NULL)
		{
			ASSERT(peditMinutes->m_type == RGuiItem::Edit);
			ASSERT(peditSeconds->m_type == RGuiItem::Edit);
			ASSERT(peditFlagID->m_type == RGuiItem::Edit);
			ASSERT(peditColor->m_type == RGuiItem::Edit);

			lMinutes = m_lTimeBonus / 60000;
			lSeconds = (m_lTimeBonus / 1000) % 60;

			peditMinutes->SetText("%ld", lMinutes);
			peditSeconds->SetText("%2.2ld", lSeconds);
			peditFlagID->SetText("%d", m_u16FlagID);
			peditColor->SetText("%d", m_u16FlagColor);
			peditMinutes->Compose();
			peditSeconds->Compose();
			peditFlagID->Compose();
			peditColor->Compose();

			sResult = DoGui(pguiRoot);
			if (sResult == 1)
			{
				lMinutes = peditMinutes->GetVal();
				lSeconds = peditSeconds->GetVal() % 60;
				m_lTimeBonus = (lMinutes * 60000) + (lSeconds * 1000);
				m_u16FlagID = peditFlagID->GetVal();
				m_u16FlagColor = MIN((long) (EndOfColors-1), peditColor->GetVal());
			}
		}
	}
	delete pguiRoot;

	// If they changed the flag apperance, load new assets.
	if (m_u16FlagColor != u16OrigColor)
	{
		FreeResources();
		GetResources();
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Get all required resources
////////////////////////////////////////////////////////////////////////////////
short CFlag::GetResources(void)						// Returns 0 if successfull, non-zero otherwise
{
	short sResult = 0;

	switch (m_u16FlagColor)
	{
		default:
		case Red:
			sResult = m_animFlagWave.Get(ms_apszRedResNames, RChannel_LoopAtStart | RChannel_LoopAtEnd);
			break;

		case Blue:
			sResult = m_animFlagWave.Get(ms_apszBlueResNames, RChannel_LoopAtStart | RChannel_LoopAtEnd);
			break;
	}
	if (sResult == 0)
	{
						// Add new animation loads here
	}
	else
	{
		TRACE("CFlag::GetResources - Failed to open 3D flag waving animation\n");
	}

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Free all resources
////////////////////////////////////////////////////////////////////////////////
short CFlag::FreeResources(void)						// Returns 0 if successfull, non-zero otherwise
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

void CFlag::OnExplosionMsg(Explosion_Message* pMessage)
{
	if (
	    m_state != State_BlownUp	&&
		 m_state != State_Die		&& 
		 m_state != State_Dead)
	{
		CThing3d::OnExplosionMsg(pMessage);

		m_state = State_BlownUp;
		m_lAnimTime = 0;
		m_lTimer = m_pRealm->m_time.GetGameTime();

		m_dExtHorzVel *= -1.4; //2.5;
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

void CFlag::OnBurnMsg(Burn_Message* pMessage)
{
	// For now we made the sentry fireproof, the only
	// way it can be destroyed is by blowing it up.
	m_state = State_Burning;
	m_lTimer = m_pRealm->m_time.GetGameTime() + FLAG_BURNED_TIMEOUT;
}


////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
