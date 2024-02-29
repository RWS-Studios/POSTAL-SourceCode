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
// barrel.cpp
// Project: Postal
//
// This module implements 55 gallon gas barrels that explode when shot or
//	set on fire.  They first explode, and then burn a large fire for some
//	time afterward.  We may also animate the exploding barrel.
//
// History:
//		03/14/97 BRH	Started this object based on the napalm object.
//
//		03/17/97 BRH	Restarted this object based on the new CThing3D base
//							class which will make it easier to do the motion
//							and message processing.
//
//		03/18/97 BRH	Tuned the delay on the explosion and added a 
//							multiplication factor to the standard external
//							velocities to react to an explosion.
//
//		03/21/97 BRH	Changed CSmash type to Item instead of Character
//							so that its proximity to a mine will not set off the
//							mine.
//
//		04/03/97	JMI	Barrel now rotates when exploded.
//
//		04/04/97	JMI	Added barrel spin animation which has the origin at the
//							barrel's center of gravity for better spinning.
//
//		04/04/97	JMI	In previous update, forgot to release new anim for 
//							spinning.
//							Also, EditNew() was not checking return value from
//							GetResources() before calling Init() and was not storing
//							result from Init() for its return value.
//
//		04/04/97	JMI	Now plays sound (not a very good one) when barrel hits
//							the ground.
//
//		04/23/97	JMI	Now sets its m_smash's bits to Barrel instead of Item.
//
//		04/24/97	JMI	Now saves and loads m_dRot and picks a random value for
//							m_dRot when EditNew() is called.
//
//		05/15/97 BRH	Moved the smash sphere up so that the sphere will encircle
//							the barrel and not the hotspot on the ground so that
//							rockets and missiles can hit them.
//
//		06/05/97	JMI	Changed m_sHitPoints to m_stockpile.m_sHitPoints to 
//							accommodate new m_stockpile in base class, CThing3d (which
//							used to contain the m_sHitPoints).
//
//		06/11/97 BRH	Passes along the shooter information from who set off
//							this barrel to who it hit.
//
//		06/12/97 BRH	Added shooter ID to the call to Setup for the explosion.
//
//		06/18/97 BRH	Changed over to using GetRandom()
//
//		06/25/97 BRH	Added sprite and alpha animation for the barrel as a test.
//							If it looks good, the shadow will probably be moved
//							into the base class CThing3d.
//
//		06/25/97 BRH	Moved shadow sprite into CThing3D and the render code as well.
//
//		06/30/97 BRH	Added cache sound effects to the static part of Load
//							so that the sound effects that the barrels use are
//							ready on any level containing barrels.
//
//		07/15/97 BRH	Added sound effects when bullets hit the barrels so that
//							you get a better indication that you are hitting them.  
//							We should probably get a better sound effect for this
//							so that it is unique, rather than using the ricochet
//							sounds again.
//
//		07/18/97	JMI	Got rid of bogus immitation PlaySample functions.
//							Now there is one PlaySample() function.  Also, you now
//							MUST specify a category and you don't have to specify a
//							SoundInstance ptr to specify a volume.
//
//		08/15/97 BRH	Added a special barrel flag so that this type of 
//							barrel can only be destroyed by the CDude.  Added an
//							EditModify dialog to set the option and added it to
//							the load and save.
//
//		08/18/97	JMI	Now plays impact animation and ricochet noise when hit by 
//							bullets.
//
//		08/20/97 BRH	Changed ricochet from destruction to weapon volume slider.
//
//		08/26/97 BRH	Changed barrel sound effects, hitting ground and getting
//							shot.
//
////////////////////////////////////////////////////////////////////////////////
#define BARREL_CPP

#include "RSPiX.h"
#include <math.h>

#include "barrel.h"
#include "dude.h"
#include "game.h"
#include "SampleMaster.h"
#include "reality.h"
#include "explode.h"


////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////

// Anim for when a barrel is hit by a bullet.
#define BARREL_HIT_RES_NAME	"Ricochet.aan"

#define HULL_RADIUS				(m_sprite.m_sRadius / 2)


////////////////////////////////////////////////////////////////////////////////
// Variables/data
////////////////////////////////////////////////////////////////////////////////

// These are default values -- actually values are set using the editor!
long CBarrel::ms_lExplosionWait = 0;			// Time to wait after explosion before fire
long CBarrel::ms_lExplosionDelay = 150;		// Delay before explosion is set off by another
short CBarrel::ms_sNumFires = 6;					// Number of fires to create after explosion

// Let this auto-init to 0
short CBarrel::ms_sFileCount;

/// Still Animation Files ///////////////////////////////////////////////////////
// An array of pointers to resource names (one for each channel of the animation)
static char* ms_apszStillResNames[] = 
{
	"3d/barrel_still.sop",
	"3d/barrel_still.mesh",
	"3d/barrel_still.tex",
	"3d/barrel_still.hot",
	"3d/barrel_still.bounds",
	"3d/barrel_still.floor",
	NULL,
	NULL
};

/// Spinning Animation Files ///////////////////////////////////////////////////
// An array of pointers to resource names (one for each channel of the animation)
static char* ms_apszSpinResNames[] = 
{
	"3d/barrel_spin.sop",
	"3d/barrel_spin.mesh",
	"3d/barrel_spin.tex",
	"3d/barrel_spin.hot",
	"3d/barrel_spin.bounds",
	"3d/barrel_spin.floor",
	NULL,
	NULL
};



////////////////////////////////////////////////////////////////////////////////
// Load object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CBarrel::Load(				// Returns 0 if successfull, non-zero otherwise
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
					pFile->Read(&ms_lExplosionWait);
					CacheSample(g_smidBarrelCrash1);
					CacheSample(g_smidBarrelCrash2);
					CacheSample(g_smidShotBarrel1);
					CacheSample(g_smidShotBarrel2);
					CacheSample(g_smidGrenadeExplode);
					break;
			}
		}

		short sData = 0;
		// Load any barrel specific data
		switch (ulFileVersion)
			{
			default:
			case 46:
				pFile->Read(&sData);
				if (sData == 0)
					m_bSpecial = false;
				else
					m_bSpecial = true;

			case 45:
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
				pFile->Read(&m_dRot);
			case 7:
			case 6:
			case 5:
			case 4:
			case 3:
			case 2:
			case 1:
			case 0:
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
			TRACE("CBarrel::Load(): Error reading from file!\n");
		}
	}
	else
	{
	TRACE("CBarrel::Load(): CDoofus::Load() failed.\n");
	}

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Save object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CBarrel::Save(										// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,											// In:  File to save to
	short sFileCount)										// In:  File count (unique per file, never 0)
{
	short sResult;

	// Call the base class save to save the instance ID, position, etc
	CThing3d::Save(pFile, sFileCount);

	// Save common data just once per file (not with each object)
	if (ms_sFileCount != sFileCount)
	{
		ms_sFileCount = sFileCount;

		// Save static data
		pFile->Write(&ms_lExplosionWait);
	}

	// Save barrel specific data
	short sData;
	if (m_bSpecial)
		sData = 1;
	else
		sData = 0;
	pFile->Write(sData);
	pFile->Write(m_dRot);

	if (!pFile->Error())
	{
		sResult = SUCCESS;
	}
	else
	{
		TRACE("CBarrel::Save() - Error writing to file\n");
		sResult = -1;
	}

	return sResult;
}

////////////////////////////////////////////////////////////////////////////////
// Init - Call this after the resources are in place
////////////////////////////////////////////////////////////////////////////////

short CBarrel::Init(void)
{
	short sResult = 0;

	// Init other stuff
	m_dVel = 0.0;
	// Set to different starting state based on the design of the animation, but
	// for now, ok.  Then also set his current animation.
	m_state = CThing3d::State_Idle;
	m_panimCur = &m_animStill;
	m_lTimer = m_pRealm->m_time.GetGameTime() + 500;

	m_stockpile.m_sHitPoints = 2 * DefHitPoints;

	// If its a special barrel that can only be destroyed by dude...
	if (m_bSpecial)
		m_smash.m_bits = CSmash::Barrel | CSmash::SpecialBarrel;
	else
		m_smash.m_bits = CSmash::Barrel;
	m_smash.m_pThing = this;

	m_sBrightness = 0;	// Default Brightness level

	// Make the shadow visible
//	m_spriteShadow.m_sInFlags &= ~CSprite::InHidden;
	PrepareShadow();

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Startup object
////////////////////////////////////////////////////////////////////////////////
short CBarrel::Startup(void)								// Returns 0 if successfull, non-zero otherwise
{
	short sResult = 0;

	// Set the current height, previous time, and Nav Net
	CThing3d::Startup();

	// Init other stuff
	sResult	= Init();

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Shutdown object
////////////////////////////////////////////////////////////////////////////////
short CBarrel::Shutdown(void)							// Returns 0 if successfull, non-zero otherwise
{
	// Call base class.
	short sResult = CThing3d::Shutdown();

	m_trans.Make1();

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Suspend object
////////////////////////////////////////////////////////////////////////////////
void CBarrel::Suspend(void)
{
	// Call base class suspend, and add anything else here if necessary
	CThing3d::Suspend();
}

////////////////////////////////////////////////////////////////////////////////
// Resume object
////////////////////////////////////////////////////////////////////////////////
void CBarrel::Resume(void)
{
	// Call the base class resume and add anything else you suspended
	CThing3d::Resume();
}


////////////////////////////////////////////////////////////////////////////////
// Update object
////////////////////////////////////////////////////////////////////////////////
void CBarrel::Update(void)
{
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

		// Check the current state
		switch (m_state)
		{

//-----------------------------------------------------------------------
// Idle - just sitting around waiting to be hit
//-----------------------------------------------------------------------

			case CBarrel::State_Idle:
				break;

//-----------------------------------------------------------------------
// Hit by explosion, waiting a bit for dramatic effect
//-----------------------------------------------------------------------

			case CBarrel::State_Wait:
				if (lThisTime > m_lTimer)
				{
					CExplode* pExplosion;
					if (CThing::Construct(CThing::CExplodeID, m_pRealm, (CThing**) &pExplosion) == 0)
					{
						pExplosion->Setup(m_dX, m_dY, m_dZ, m_u16ShooterID);
						PlaySample(g_smidGrenadeExplode, SampleMaster::Destruction);
					}
					m_state = State_BlownUp;
				}
				break;


//-----------------------------------------------------------------------
// Blownup - You were blown up so pop up into the air and come down dead
//-----------------------------------------------------------------------

			case CBarrel::State_BlownUp:
				// Make it react to the explosion
				if (lThisTime > m_lTimer)
				{
					m_lAnimTime += lTimeDifference;
					if (!WhileBlownUp())
					{
						m_state = State_Burning;
						m_lTimer = lThisTime + ms_lExplosionWait;
						if (GetRandom() & 0x01)
							PlaySample(g_smidBarrelCrash1, SampleMaster::Destruction);
						else
							PlaySample(g_smidBarrelCrash2, SampleMaster::Destruction);
					}
				}

				break;

//-----------------------------------------------------------------------
// Burning - You are on fire, so run around burning until you are dead
//-----------------------------------------------------------------------

			case CBarrel::State_Burning:
				if (lThisTime > m_lTimer)
				{
					short i;
					CFire* pFire;
					for (i = 0; i < ms_sNumFires; i++)
					{
						if (CThing::Construct(CThing::CFireID, m_pRealm, (CThing**) &pFire) == 0)
						{
							if (pFire->Setup(m_dX - 20 + (GetRandom() % 40), m_dY, m_dZ - 20 + (GetRandom() % 40), 
											  4000 + (GetRandom() % 9000), false, CFire::LargeFire) != SUCCESS)
								delete pFire;
							else
								pFire->m_u16ShooterID = m_u16ShooterID;
						}
					}
					// Delete this barrel object and just let the fire burn
					// where it was.
					delete this;
					return;
				}

				break;

		}

		// Update sphere.
		m_smash.m_sphere.sphere.X			= m_dX;
		m_smash.m_sphere.sphere.Y		   = m_dY + m_sprite.m_sRadius;
		m_smash.m_sphere.sphere.Z			= m_dZ;
		m_smash.m_sphere.sphere.lRadius	= m_sprite.m_sRadius;

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
short CBarrel::EditNew(									// Returns 0 if successfull, non-zero otherwise
	short sX,												// In:  New x coord
	short sY,												// In:  New y coord
	short sZ)												// In:  New z coord
{
	short sResult = 0;

	sResult = CThing3d::EditNew(sX, sY, sZ);

	// Pick a random rotation.
	m_dRot	= (double)(GetRandom() % 360);

	if (sResult == SUCCESS)
	{
		// Load resources
		sResult = GetResources();
		if (sResult == 0)
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
// Called by editor to modify object
////////////////////////////////////////////////////////////////////////////////
short CBarrel::EditModify(void)
{
	short sResult = 0;
	RGuiItem* pGuiItem = NULL;
	RGuiItem* pGui = RGuiItem::LoadInstantiate(FullPathVD("res/editor/barrel.gui"));
	if (pGui)
	{
		RMultiBtn* pmbtnSpecial = (RMultiBtn*) pGui->GetItemFromId(100);
		if (pmbtnSpecial)
		{
			// Verify these are the type we think they are before accessing type specific
			// members.
			ASSERT(pmbtnSpecial->m_type == RGuiItem::MultiBtn);

			// Set current state for show state check box.
			pmbtnSpecial->m_sState = (m_bSpecial == false) ? 1 : 2;
			// Reflect changes.
			pmbtnSpecial->Compose();

			sResult = DoGui(pGui);
			if (sResult == 1)
			{
				// Determine whether or not to display state on screen.
				m_bSpecial	= (pmbtnSpecial->m_sState	== 2) ? true : false;
				if (m_bSpecial)
					m_smash.m_bits = CSmash::Barrel | CSmash::SpecialBarrel;
				else
					m_smash.m_bits = CSmash::Barrel;
			}
		}
	}
	delete pGui;

	return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Get all required resources
////////////////////////////////////////////////////////////////////////////////
short CBarrel::GetResources(void)						// Returns 0 if successfull, non-zero otherwise
{
	short sResult = 0;

	sResult	= m_animStill.Get(ms_apszStillResNames, RChannel_LoopAtStart | RChannel_LoopAtEnd);
	sResult	|= m_animSpin.Get(ms_apszSpinResNames, RChannel_LoopAtStart | RChannel_LoopAtEnd);
	if (sResult == 0)
	{
		// Add additional gets here
	}
	else
	{
		TRACE("CBarrel::GetResources - Failed to open 3D barrel animation(s)\n");
	}

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Free all resources
////////////////////////////////////////////////////////////////////////////////
short CBarrel::FreeResources(void)						// Returns 0 if successfull, non-zero otherwise
{
	m_animStill.Release();
	m_animSpin.Release();
	// Release the shadow image

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Message handlers
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Shot Message
////////////////////////////////////////////////////////////////////////////////

void CBarrel::OnShotMsg(Shot_Message* pMessage)
{
	// If the barrel hasn't exploded yet, deduct a semi-random
	// amount of hit points so that the barrel doesn't always
	// explode with a certain number of shots.
	if (m_state == State_Idle)
	{
		if (GetRandom() & 0x01)
			PlaySample(g_smidShotBarrel1, SampleMaster::Weapon);
		else
			PlaySample(g_smidShotBarrel2, SampleMaster::Weapon);
		m_u16ShooterID = pMessage->u16ShooterID;
		m_stockpile.m_sHitPoints -= pMessage->sDamage * GetRandom() % 50;
		if (m_stockpile.m_sHitPoints <= 0)
		{
			m_state = State_BlownUp;
			m_lAnimTime = 0;
			CExplode* pExplosion;
			if (CThing::Construct(CThing::CExplodeID, m_pRealm, (CThing**) &pExplosion) == 0)
			{
				pExplosion->Setup(m_dX, m_dY, m_dZ, m_u16ShooterID);
				PlaySample(g_smidGrenadeExplode, SampleMaster::Destruction);
			}
		}
		else
		{
			// Audible and visual feedback.
			PlaySample(g_smidShotBarrel1, SampleMaster::Weapon);
			// X/Z position depends on angle of shot (it is opposite).
			short	sDeflectionAngle	= rspMod360(pMessage->sAngle + 180);
			double	dHitX	= m_dX + COSQ[sDeflectionAngle] * HULL_RADIUS;
			double	dHitZ	= m_dZ - SINQ[sDeflectionAngle] * HULL_RADIUS;
			StartAnim(
				BARREL_HIT_RES_NAME, 
				dHitX, 
				m_dY + m_sprite.m_sRadius + (GetRandom() % m_sprite.m_sRadius),
				dHitZ,
				false);
		}

		// No need to call this since the barrel doesn't move while idle
		// anyway.
		//CThing3d::OnShotMsg(pMessage);
	}
}

////////////////////////////////////////////////////////////////////////////////
// Explosion message
////////////////////////////////////////////////////////////////////////////////

void CBarrel::OnExplosionMsg(Explosion_Message* pMessage)
{
	// If the barrel hasn't exploded already
	if (m_state == State_Idle)
	{
		m_u16ShooterID = pMessage->u16ShooterID;
		// This will calculate the reaction external velocity to 
		// the explosion.  
		CThing3d::OnExplosionMsg(pMessage);
		// Make the external velocities more pronouced for the barrels
		// so that the effect will be more dramatic.
		m_dExtHorzVel *= 2.5;
		m_dExtVertVel *= 1.4;
		m_state = State_Wait;
		m_stockpile.m_sHitPoints = 0;
		m_lTimer = m_pRealm->m_time.GetGameTime() + ms_lExplosionDelay;
		m_lAnimTime = 0;
		// Send it spinning.
		m_dExtRotVelY	= GetRandom() % 720;
		m_dExtRotVelZ	= GetRandom() % 720;
		// Spin, spin, spin.
		m_panimCur		= &m_animSpin;
		// For this animation, we need to know where the bottom of the barrel
		// is (since it's not at the origin).
		// ***FUDGE***
		RP3d	pt3dSrc	= { 0.0F, 2.0F, 0.0F, 1.0F } ;	// In:  RANDY UNITS!!!
		RP3d	pt3dDst;	// Out: Postal units.

		m_pRealm->m_scene.TransformPtsToRealm(&m_trans, &pt3dSrc, &pt3dDst, 1 );

		m_dY	+=	pt3dDst.y;
	}
}

////////////////////////////////////////////////////////////////////////////////
// Burning message
////////////////////////////////////////////////////////////////////////////////

void CBarrel::OnBurnMsg(Burn_Message* pMessage)
{
	if (m_state == State_Idle)
	{
		m_u16ShooterID = pMessage->u16ShooterID;
		m_stockpile.m_sHitPoints -= pMessage->sDamage;
		if (m_stockpile.m_sHitPoints <= 0)
		{
			m_state = State_BlownUp;
			m_lTimer = m_pRealm->m_time.GetGameTime();
			m_lAnimTime = 0;
			CExplode* pExplosion;
			if (CThing::Construct(CThing::CExplodeID, m_pRealm, (CThing**) &pExplosion) == 0)
			{
				pExplosion->Setup(m_dX, m_dY, m_dZ, m_u16ShooterID);
				PlaySample(g_smidGrenadeExplode, SampleMaster::Destruction);
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// Handles an ObjectDelete_Message.
// (virtual).
////////////////////////////////////////////////////////////////////////////////
void CBarrel::OnDeleteMsg(					// Returns nothing.
	ObjectDelete_Message* pdeletemsg)	// In:  Message to handle.
	{
	CThing3d::OnDeleteMsg(pdeletemsg);
	}


////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
