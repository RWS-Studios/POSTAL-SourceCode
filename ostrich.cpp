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
// ostrich.cpp
// Project: Postal
//
//	This module implements the ostrich.
//
// History:
//
//		05/09/97 BRH	Started the ostrich from the band member logic.
//
//		05/22/97 BRH	Render skips the Doofus render and goes right to the
//							Character render since its not using the m_dAnimRot.
//
//		05/26/97 BRH	Changed the call to OnDead to the Doofus version
//							rather than the Character version.
//
//		05/29/97	JMI	Changed instance of REALM_ATTR_FLOOR_MASK to 
//							REALM_ATTR_NOT_WALKABLE.
//
//		06/05/97	JMI	Changed m_sHitPoints to m_stockpile.m_sHitPoints to 
//							accommodate new m_stockpile in base class, CThing3d (which
//							used to contain the m_sHitPoints).
//
//		06/18/97 BRH	Changed over to using GetRandom()
//
//		06/25/97	JMI	Now calls PrepareShadow() in Init() which loads and sets up
//							a shadow sprite.
//
//		06/30/97 MJR	Replaced SAFE_GUI_REF with new GuiItem.h-defined macro.
//
//		06/30/97 BRH	Cached the sound samples in static portion of Load
//							function so that the sound effects will be ready
//							whenever this item is used on a level.
//
//		07/01/97	JMI	Replaced GetFloorMapValue() with GetHeightAndNoWalk() call.
//
//		07/09/97	JMI	Now uses m_pRealm->Make2dResPath() to get the fullpath
//							for 2D image components.
//
//		07/12/97 BRH	Changed panic motion to the same as the base class, but
//							it still didn't fix the problem of them going through the
//							wall of the train car on the farm level.  Also increased
//							their hit points, and made them not die in one shot.
//
//		07/18/97	JMI	Got rid of bogus immitation PlaySample functions.
//							Now there is one PlaySample() function.  Also, you now
//							MUST specify a category and you don't have to specify a
//							SoundInstance ptr to specify a volume.
//
//		07/21/97 BRH	Changed the burning state so that the ostrich will burn 
//							longer.
//
//		08/06/97 BRH	Changed smash bits to zero when dying starts so that things
//							won't hit him as he falls slowly, and so you can shoot 
//							past him.  Also allows the shot animation restart when shot
//							to get a little more reaction since his shot animation
//							is so long.
//
//		08/10/97	JMI	Commented out the code that deleted all the sound things in
//							a realm when the ostriches paniced.
//
//		08/12/97 BRH	Set the flag for victim rather than hostile so that
//							the score will display the correct numbers of each
//							type.
//
//		09/03/97	JMI	Replaced Good Smash bit with Civilian.
//
//		09/03/97 BRH	Put in the real ostrich sounds.
//
//		09/04/97 BRH	Added ostrich dying sound.
//
////////////////////////////////////////////////////////////////////////////////
#define OSTRICH_CPP

#include "RSPiX.h"
#include <math.h>

#include "ostrich.h"
#include "SampleMaster.h"
#include "item3d.h"

////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////

#define BRIGHTNESS_PER_LIGHT_ATTRIBUTE 15
#define NUM_ELEMENTS(a) (sizeof(a) / sizeof(a[0]) )

// Notification message lParm1's.
#define BLOOD_POOL_DONE_NOTIFICATION	1	// Blood pool is done animating.

// Random amount the blood splat can adjust.
#define BLOOD_SPLAT_SWAY		10

// Gets a random between -range / 2 and range / 2.
#define RAND_SWAY(sway)	((GetRandom() % sway) - sway / 2)

#define GUI_ID_OK						1

////////////////////////////////////////////////////////////////////////////////
// Variables/data
////////////////////////////////////////////////////////////////////////////////

double COstrich::ms_dExplosionVelocity = 180.0;
double COstrich::ms_dMaxMarchVel = 30.0;
double COstrich::ms_dMaxRunVel = 80.0;
long COstrich::ms_lStateChangeTime = 10000;
short COstrich::ms_sStartingHitPoints = 100;

// Let this auto-init to 0
short COstrich::ms_sFileCount;

/// Standing Animation Files
// An array of pointers to resource names (one for each channel of the animation)
static char* ms_apszStandResNames[] = 
{
	"3d/ostrich_stand.sop",
	"3d/ostrich_stand.mesh",
	"3d/ostrich_stand.tex",
	"3d/ostrich_stand.hot",
	"3d/ostrich_stand.bounds",
	"3d/ostrich_stand.floor",
	NULL,
	NULL	
};

/// Running Animation Files
// An array of pointers to resource names (one for each channel of the animation)
static char* ms_apszRunResNames[] = 
{
	"3d/ostrich_run.sop",
	"3d/ostrich_run.mesh",
	"3d/ostrich_run.tex",
	"3d/ostrich_run.hot",
	"3d/ostrich_run.bounds",
	"3d/ostrich_run.floor",
	NULL,
	NULL
};

/// Throwing Animation Files 
// An array of pointers to resource names (one for each channel of the animation)
static char* ms_apszWalkResNames[] = 
{
	"3d/ostrich_walk.sop",
	"3d/ostrich_walk.mesh",
	"3d/ostrich_walk.tex",
	"3d/ostrich_walk.hot",
	"3d/ostrich_walk.bounds",
	"3d/ostrich_walk.floor",
	NULL,
	NULL
};

// Shot Animation Files
// An array of pointers to resource names (one for each channel of the animation)
static char* ms_apszShotResNames[] = 
{
	"3d/ostrich_shot.sop",
	"3d/ostrich_shot.mesh",
	"3d/ostrich_shot.tex",
	"3d/ostrich_shot.hot",
	"3d/ostrich_shot.bounds",
	"3d/ostrich_shot.floor",
	NULL,
	NULL
};

/// Blown up Animation Files
// An array of pointers to resource names (one for each channel of the animation)
static char* ms_apszBlownupResNames[] =
{
	"3d/ostrich_blownup.sop",
	"3d/ostrich_blownup.mesh",
	"3d/ostrich_blownup.tex",
	"3d/ostrich_blownup.hot",
	"3d/ostrich_blownup.bounds",
	"3d/ostrich_blownup.floor",
	NULL,
	NULL
};

/// Hide Animation Files
// An array of pointers to resource names (one for each channel of the animation)
static char* ms_apszHideResNames[] =
{
	"3d/ostrich_hide.sop",
	"3d/ostrich_hide.mesh",
	"3d/ostrich_hide.tex",
	"3d/ostrich_hide.hot",
	"3d/ostrich_hide.bounds",
	"3d/ostrich_hide.floor",
	NULL,
	NULL
};

/// Die Animation files
// An array of pointers to resource names (one for each channel of the animation)
static char* ms_apszDieResNames[] = 
{
	"3d/ostrich_die.sop",
	"3d/ostrich_die.mesh",
	"3d/ostrich_die.tex",
	"3d/ostrich_die.hot",
	"3d/ostrich_die.bounds",
	"3d/ostrich_die.floor",
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
short COstrich::Load(					// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,						// In:  File to load from
	bool bEditMode,					// In:  True for edit mode, false otherwise
	short sFileCount,					// In:  File count (unique per file, never 0)
	ULONG	ulFileVersion)				// In:  Version of file format to load.
{
	short sResult = 0;

	// Call the base class load to get the instance ID, position, motion etc.
	sResult	= CDoofus::Load(pFile, bEditMode, sFileCount, ulFileVersion);
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
					CacheSample(g_smidOstrichShot);
					CacheSample(g_smidOstrichBurning);
					CacheSample(g_smidOstrichBlownup);
					CacheSample(g_smidOstrichDie);
					break;
				}
			}

		// Load Rocket Man specific data
			switch (ulFileVersion)
			{
				default:
				case 4:
				case 3:
				case 2:
				case 1:
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
			TRACE("COstrich::Load(): Error reading from file!\n");
		}
	}
	else
	{
	TRACE("CGrenader::Load(): CDoofus::Load() failed.\n");
	}

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Save object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short COstrich::Save(										// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,											// In:  File to save to
	short sFileCount)										// In:  File count (unique per file, never 0)
{
	short sResult = SUCCESS;

	// Call the base class save to save the instance ID, position etc.
	CDoofus::Save(pFile, sFileCount);

	// Save common data just once per file (not with each object)
	if (ms_sFileCount != sFileCount)
	{
		ms_sFileCount = sFileCount;

		// Save static data
	}


	if (!pFile->Error())
	{
		sResult = SUCCESS;
	}
	else
	{
		TRACE("COstrich::Save() - Error writing to file\n");
		sResult = -1;
	}

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Init - Call this after the resources are in place
////////////////////////////////////////////////////////////////////////////////

short COstrich::Init(void)
{
	short sResult = 0;

	// Prepare shadow (get resources and setup sprite).
	sResult	= PrepareShadow();

	// Init position, rotation and velocity
	m_dVel = 0.0;
	m_dRot = rspMod360(GetRandom());
	m_lPrevTime = m_pRealm->m_time.GetGameTime();
	m_state = CCharacter::State_Idle;
	m_lTimer = m_pRealm->m_time.GetGameTime() + 500;
	m_sBrightness = 0;	// Default brightness

	m_smash.m_bits		= CSmash::Civilian | CSmash::Character;
	m_smash.m_pThing	= this;

	m_lAnimTime = 0;
	m_panimCur = &m_animStand;
	m_lTimer = m_pRealm->m_time.GetGameTime();
	m_stockpile.m_sHitPoints = ms_sStartingHitPoints;

	m_state = CCharacter::State_Stand;
	m_dAcc = 150;

	return sResult;
}

////////////////////////////////////////////////////////////////////////////////
// Startup object
////////////////////////////////////////////////////////////////////////////////
short COstrich::Startup(void)								// Returns 0 if successfull, non-zero otherwise
{
	// Register this as a victim rather than a hostile.
	m_bCivilian = true;

	// Set the current height, previous time, and Nav Net by calling the
	// base class startup
	CDoofus::Startup();

	// Init other stuff
	return Init();
}


////////////////////////////////////////////////////////////////////////////////
// Update object
////////////////////////////////////////////////////////////////////////////////
void COstrich::Update(void)
{
	short sHeight = m_sPrevHeight;
	double dNewX;
	double dNewY;
	double dNewZ;
	double dX;
	double dZ;
	long lThisTime;
	long lTimeDifference;


	if (!m_sSuspend)
	{
		// Get new time
		lThisTime = m_pRealm->m_time.GetGameTime(); 
		lTimeDifference = lThisTime - m_lPrevTime;

		// Calculate elapsed time in seconds
		double dSeconds = (double)(lThisTime - m_lPrevTime) / 1000.0;

		// Check for new messages that may change the state
		ProcessMessages();

		// Increment animation time
		m_lAnimTime += lTimeDifference;

		// Check the current state
		switch (m_state)
		{

//-----------------------------------------------------------------------
// Stand or Hide - Either one waits in the current position for the
//						 random timer to expire.
//-----------------------------------------------------------------------

			case State_Stand:
			case State_Hide:
				if (lThisTime > m_lTimer)
					ChangeRandomState();
				break;

//-----------------------------------------------------------------------
// Walk - Walk around a bit
//-----------------------------------------------------------------------

			case State_Walk:
				if (lThisTime > m_lTimer)
				{
					ChangeRandomState();
				}
				else
				{
					dX = m_dX;
					dZ = m_dZ;

					m_dAcc = ms_dAccUser;
					UpdateVelocities(dSeconds, ms_dMaxMarchVel, ms_dMaxMarchVel);
					GetNewPosition(&dNewX, &dNewY, &dNewZ, dSeconds);
					if (MakeValidPosition(&dNewX, &dNewY, &dNewZ, 10) == true)
					{
					// Update Values /////////////////////////////////////////////////////////

						m_dX	= dNewX;
						m_dY	= dNewY;
						m_dZ	= dNewZ;

						UpdateFirePosition();
					}
					else
					{
					// Restore Values ////////////////////////////////////////////////////////

						m_dVel			-= m_dDeltaVel;
					}

					// If he didn't move at all, then turn him so he will
					// avoid the wall
					if (dX == m_dX && dZ == m_dZ)
					{
						// Turn to avoid wall next time
						if (m_sRotDirection == 0)
							m_dRot = rspMod360(m_dRot - 20);
						else
							m_dRot = rspMod360(m_dRot + 20);
					}
				}
		
				break;


//-----------------------------------------------------------------------
// Panic - Pick a random bouy and run to it.  When you are there, pick
//			  a different random bouy and run to it.  
//-----------------------------------------------------------------------

			case State_Panic:
				dX = m_dX;
				dZ = m_dZ;

				DeluxeUpdatePosVel(dSeconds);  
/*

				m_dAcc = ms_dAccUser;
				UpdateVelocities(dSeconds, ms_dMaxRunVel, ms_dMaxRunVel);
				GetNewPosition(&dNewX, &dNewY, &dNewZ, dSeconds);

				// Get height and 'no walk' status at new position.
				bool		bNoWalk;
				sHeight	= m_pRealm->GetHeightAndNoWalk(dNewX, dNewY, &bNoWalk);

				// If too big a height difference or completely not walkable . . .
				if (bNoWalk == true
					|| (sHeight - dNewY > 10) )// && m_bAboveTerrain == false && m_dExtHorzVel == 0.0))
				{
					TRACE("****** - loose ostrish\n");
				// Move like smoke
				}

				if (MakeValidPosition(&dNewX, &dNewY, &dNewZ, 10) == true)
				{
				// Update Values /////////////////////////////////////////////////////////

					m_dX	= dNewX;
					m_dY	= dNewY;
					m_dZ	= dNewZ;

					UpdateFirePosition();
				}
				else
				{
				// Restore Values ////////////////////////////////////////////////////////
	
					m_dVel			-= m_dDeltaVel;
				}
*/
				// If he didn't move at all, then turn him so he will
				// avoid the wall
				if (dX == m_dX && dZ == m_dZ)
				{
					// Turn to avoid wall next time
					if (m_sRotDirection == 0)
						m_dRot = rspMod360(m_dRot - 20);
					else
						m_dRot = rspMod360(m_dRot + 20);
				}


				break;

//-----------------------------------------------------------------------
// Burning - Run around on fire until dead
//-----------------------------------------------------------------------

			case State_Burning:
				if (!CCharacter::WhileBurning())
				{
					if (m_stockpile.m_sHitPoints <= 0)
					{
						m_state = State_Die;
						m_lAnimTime = 0;
						m_panimCur = &m_animDie;
						PlaySample(
							g_smidOstrichDie,
							SampleMaster::Voices);
					}
					else
					{
						m_state = State_Panic;
					}
				}				
				break;

//-----------------------------------------------------------------------
// Blown Up - Do motion into the air until you hit the ground again
//-----------------------------------------------------------------------

			case State_BlownUp:
				if (!CCharacter::WhileBlownUp())
					m_state = State_Dead;
				else
					UpdateFirePosition();
				break;

//-----------------------------------------------------------------------
// Shot - Dies in one shot
//-----------------------------------------------------------------------

			case State_Shot:
				if (!CCharacter::WhileShot())
				{
					if (m_stockpile.m_sHitPoints <= 0)
					{
						m_state = State_Die;
						m_panimCur = &m_animDie;
						m_lAnimTime = 0;
						PlaySample(
							g_smidOstrichDie,
							SampleMaster::Voices);
					}
					else
					{
						m_state = State_Panic;
						m_panimCur = &m_animRun;
						m_lAnimTime = 0;
					}
				}
				break;

//-----------------------------------------------------------------------
// Die - run die animation until done, the you are dead
//-----------------------------------------------------------------------

			case State_Die:
				m_smash.m_bits = 0;
				if (!CCharacter::WhileDying())
					m_state = State_Dead;
				else
					UpdateFirePosition();
				
				break;

//-----------------------------------------------------------------------
// Dead - paste yourself in the background and delete yourself
//-----------------------------------------------------------------------

			case State_Dead:
				OnDead();
				delete this;
				return;

				break;

		}

		m_smash.m_sphere.sphere.X			= m_dX;
		// Fudge center of sphere as half way up the dude.
		// Doesn't work if dude's feet leave the origin.
		m_smash.m_sphere.sphere.Y			= m_dY + m_sprite.m_sRadius;
		m_smash.m_sphere.sphere.Z			= m_dZ;
		m_smash.m_sphere.sphere.lRadius	= m_sprite.m_sRadius;

		// Update the smash.
		m_pRealm->m_smashatorium.Update(&m_smash);

		// Save height for next time
		m_sPrevHeight = sHeight;

		// Save time for next time
		m_lPrevTime = lThisTime;
	}
}


////////////////////////////////////////////////////////////////////////////////
// Render object
////////////////////////////////////////////////////////////////////////////////
void COstrich::Render(void)
{
	// Call base class.
	CCharacter::Render();

}

////////////////////////////////////////////////////////////////////////////////
// Called by editor to init new object at specified position
////////////////////////////////////////////////////////////////////////////////
short COstrich::EditNew(									// Returns 0 if successfull, non-zero otherwise
	short sX,												// In:  New x coord
	short sY,												// In:  New y coord
	short sZ)												// In:  New z coord
{
	short sResult = 0;

	// Call the base class to place the item.
	sResult = CDoofus::EditNew(sX, sY, sZ);

	if (sResult == SUCCESS)
	{
		// Load resources
		sResult = GetResources();
		if (sResult == SUCCESS)
		{
			sResult	= Init();
		}
	}
	
	return sResult;
}

////////////////////////////////////////////////////////////////////////////////
// EditModify - Show dialog box for selecting starting bouy
////////////////////////////////////////////////////////////////////////////////

short COstrich::EditModify(void)
{
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Get all required resources
////////////////////////////////////////////////////////////////////////////////
short COstrich::GetResources(void)						// Returns 0 if successfull, non-zero otherwise
{
	short sResult = 0;

	sResult = m_animRun.Get(ms_apszRunResNames, RChannel_LoopAtStart | RChannel_LoopAtEnd);
	if (sResult == 0)
	{
		sResult = m_animStand.Get(ms_apszStandResNames, RChannel_LoopAtStart | RChannel_LoopAtEnd);
		if (sResult == 0)
		{
			sResult = m_animWalk.Get(ms_apszWalkResNames, RChannel_LoopAtStart | RChannel_LoopAtEnd);
			if (sResult == 0)
			{
				sResult = m_animShot.Get(ms_apszShotResNames);
				if (sResult == 0)
				{
					sResult = m_animBlownup.Get(ms_apszBlownupResNames);
					if (sResult == 0)
					{
						sResult = m_animHide.Get(ms_apszHideResNames);
						if (sResult == 0)
						{
							sResult = m_animDie.Get(ms_apszDieResNames);
							if (sResult == 0)
							{
								// Add more anim gets here if necessary
							}
							else
							{
								TRACE("COstrich::GetResources - Failed to open 3D die animation\n");
							}
						}
						else
						{
							TRACE("COstrich::GetResources - Failed to open 3D hide animation\n");
						}
					}
					else
					{
						TRACE("COstrich::GetResources - Failed to open 3D blownup animation\n");
					}
				}
				else
				{
					TRACE("COstrich::GetResources - Failed to open 3D shot animation\n");
				}	
			}
			else
			{
				TRACE("COstrich::GetResources - Failed to open 3D walk animation\n");
			}
		}
		else
		{
			TRACE("COstrich::GetResources - Failed to open 3D stand animation\n");
		}
	}
	else
	{
		TRACE("COstrich::GetResources - Failed to open 3D run animation\n");
	}	

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Free all resources
////////////////////////////////////////////////////////////////////////////////
short COstrich::FreeResources(void)						// Returns 0 if successfull, non-zero otherwise
{
	m_animRun.Release();
	m_animStand.Release();
	m_animWalk.Release();
	m_animShot.Release();
	m_animBlownup.Release();
	m_animHide.Release();
	m_animDie.Release();		

	return 0;
}


////////////////////////////////////////////////////////////////////////////////
// ProcessMessages - Similar to the base class version but handles a few more
////////////////////////////////////////////////////////////////////////////////

void COstrich::ProcessMessages(void)
{
	// Check queue of messages.
	GameMessage	msg;
	while (m_MessageQueue.DeQ(&msg) == true)
	{
		ProcessMessage(&msg);

		switch(msg.msg_Generic.eType)
		{
			case typePanic:
				OnPanicMsg(&(msg.msg_Panic));
				break;
				
		}

	}

}

////////////////////////////////////////////////////////////////////////////////
// Message handlers
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Shot Message
////////////////////////////////////////////////////////////////////////////////

void COstrich::OnShotMsg(Shot_Message* pMessage)
{
    UnlockAchievement(ACHIEVEMENT_FIGHT_AN_OSTRICH);

	m_stockpile.m_sHitPoints -= pMessage->sDamage;

	if (m_state != State_BlownUp &&
//	    m_state != State_Shot &&
		 m_state != State_Die &&
		 m_state != State_Dead)
	{
		CCharacter::OnShotMsg(pMessage);

		// Restart the animation even if he is shot, 
		m_lAnimTime = 0;
		m_panimCur = &m_animShot;

		if (m_state != State_Shot)
		{
			PlaySample(
				g_smidOstrichShot,
				SampleMaster::Voices);
			m_state = State_Shot;
			AlertFlock();
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// Explosion message
////////////////////////////////////////////////////////////////////////////////

void COstrich::OnExplosionMsg(Explosion_Message* pMessage)
{
	if (m_state != State_BlownUp)
	{
        UnlockAchievement(ACHIEVEMENT_FIGHT_AN_OSTRICH);

		CCharacter::OnExplosionMsg(pMessage);

		// Explosion kills the guy
		m_stockpile.m_sHitPoints = 0;
//		m_dExtVertVel = ms_dExplosionVelocity;
		m_state = State_BlownUp;
		m_lAnimTime = 0;
		m_panimCur = &m_animBlownup;
		PlaySample(
			g_smidOstrichBlownup,
			SampleMaster::Voices);
		AlertFlock();
	}
}

////////////////////////////////////////////////////////////////////////////////
// Burning message
////////////////////////////////////////////////////////////////////////////////

void COstrich::OnBurnMsg(Burn_Message* pMessage)
{
    UnlockAchievement(ACHIEVEMENT_FIGHT_AN_OSTRICH);

	CCharacter::OnBurnMsg(pMessage);
	m_stockpile.m_sHitPoints -= MAX(pMessage->sDamage / 4, 1);

	if (m_state != State_Burning && 
	    m_state != State_BlownUp &&
		 m_state != State_Die &&
		 m_state != State_Dead)
	{
		m_state = State_Burning;
		m_panimCur = &m_animRun;
		m_lAnimTime = 0;
		PlaySample(
			g_smidOstrichBurning,
			SampleMaster::Voices);
		AlertFlock();
	}
}

////////////////////////////////////////////////////////////////////////////////
// Panic message
////////////////////////////////////////////////////////////////////////////////

void COstrich::OnPanicMsg(Panic_Message* pMessage)
{
	if (m_state != State_Die &&
	    m_state != State_Dead &&
		 m_state != State_BlownUp &&
		 m_state != State_Shot &&
		 m_state != State_Burning &&
		 m_state != State_Panic &&
		 m_state != State_Hide)
	{
		m_state = State_Panic;
		m_panimCur = &m_animRun;
		m_lAnimTime = GetRandom() % m_panimCur->m_psops->TotalTime();
	}
}

////////////////////////////////////////////////////////////////////////////////
// AlertFlock
////////////////////////////////////////////////////////////////////////////////

void COstrich::AlertFlock(void)
{
//	CThing::Things::iterator iNext;
	CThing* pThing;
	GameMessage msg;
//	GameMessage msgStopSound;

//	msgStopSound.msg_ObjectDelete.eType = typeObjectDelete;
//	msgStopSound.msg_ObjectDelete.sPriority = 0;

	msg.msg_Panic.eType = typePanic;
	msg.msg_Panic.sPriority = 0;
	msg.msg_Panic.sX = (short) m_dX;
	msg.msg_Panic.sY = (short) m_dY;
	msg.msg_Panic.sZ = (short) m_dZ;

	CListNode<CThing>* pNext = m_pRealm->m_everythingHead.m_pnNext;
	while (pNext->m_powner != NULL)
	{
		pThing = pNext->m_powner;
		if (pThing->GetClassID() == COstrichID && pThing != this)
			SendThingMessage(&msg, pThing);
//		else if (pThing->GetClassID() == CSoundThingID)
//			SendThingMessage(&msgStopSound, pThing);
		pNext = pNext->m_pnNext;
	}	
}


////////////////////////////////////////////////////////////////////////////////
// Implements basic one-time functionality for each time State_Dead is
// entered.
////////////////////////////////////////////////////////////////////////////////
void COstrich::OnDead(void)
	{
	// Call base class.
	CDoofus::OnDead();
	}

////////////////////////////////////////////////////////////////////////////////
// ChangeRandomState - Choose among the 3 normal states, stand, hide or walk
////////////////////////////////////////////////////////////////////////////////

void COstrich::ChangeRandomState(void)
{
	short sMod;
	m_lTimer = m_pRealm->m_time.GetGameTime() + ms_lStateChangeTime + GetRandom() % 5000;
	sMod = m_lTimer % 3;
	m_dRot = rspMod360(m_dRot - 10 + (GetRandom() % 20));
	m_sRotDirection = m_lTimer % 2;

	switch (sMod)
	{
		case 0:
			m_state = State_Hide;
			if (m_panimCur != &m_animHide)
			{
				m_panimCur = &m_animHide;
				m_lAnimTime = 0;
			}
			break;

		case 1:
			m_state = State_Stand;
			if (m_panimCur != &m_animStand)
			{
				m_panimCur = &m_animStand;
				m_lAnimTime = 0;
			}
			break;

		case 2:
			m_state = State_Walk;
			if (m_panimCur != &m_animWalk)
			{
				m_panimCur = &m_animWalk;
				m_lAnimTime = 0;
			}
			break;

		default:
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
