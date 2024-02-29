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
// napalm.cpp
// Project: Postal
//
// This module implements the CNapalm weapon class which is a canister of
// napalm gel that breaks apart when it hits the ground lays down a smear
//	of fire.  The canister may be an alternate ordinate for the rocket launcher.
// 
//
// History:
//		01/17/97 BRH	Started this weapon object.
//
//		02/04/97	JMI	Changed LoadDib() call to Load() (which now supports
//							loading of DIBs).
//
//		02/09/97 BRH	Started the CNapalm from the CGrenade object since their
//							initial movement logic is similar.
//
//		02/10/97	JMI	rspReleaseResource() now takes a ptr to a ptr.
//
//		02/18/97 BRH	Added a time setting to the Fire with some randomness
//							to make it look better.
//
//		02/19/97 BRH	Added message processing to check for ObjectDeleted 
//							message.
//
//		02/19/97 BRH	Changed this from 2D to 3D animation.
//
//		02/23/97 BRH	Updated the transform with the angle so that the canister
//							faces the direction it is traveling.  Also changed the
//							coordinate system to x,-z
//
//		02/23/97 BRH	Added Preload() function to cache resources for this object
//							before play begins.
//
//		02/24/97	JMI	No longer sets the m_type member of the m_sprite b/c it
//							is set by m_sprite's constructor.
//
//		02/24/97 BRH	Changed fire to thin fire for more alpha effect since it
//							lays down many layers of fire.  Also hides the napalm
//							canister by skipping the render when in the hidden state.
//
//		02/24/97 BRH	Added sound effects for canister shooting, hitting things,
//							and when it breaks open.  Used reality.h motion templates
//							and changed the algorithm for detecting ground and
//							walls.
//
//		03/03/97 BRH	Derived this from the CWeapon base class.
//
//		03/03/97	JMI	Commented out dHorizVelocity and dVertVelocity parameters
//							to Setup() so that this version would be a virtual over-
//							ride of CWeapon's.
//
//		03/06/97	JMI	Upgraded to current rspMod360 usage.
//
//		03/13/97	JMI	Load now takes a version number.
//
//		03/19/97 BRH	Changed ProcessMessages to return a void so that it matches
//							the new virtual function in the CWeapon base class.
//
//		03/21/97 BRH	Now ignores the ATTRIBUTE_NOT_WALKABLE so that the napalm
//							canisters don't bounce off of the edge of the world.
//
//		04/10/97 BRH	Converted to using the new multi layer attribute maps and
//							the helper functions that go with them.
//
//		05/04/97 BRH	Took out an old unused reference to an STL iterator.
//
//		05/29/97	JMI	Removed ASSERT on m_pRealm->m_pAttribMap which no longer
//							exists.
//
//		06/11/97 BRH	Added shooter ID passing to the fire that is created.
//
//		06/12/97	JMI	Now handles State_Hide by setting m_sprite's InHidden flag.
//
//		06/16/97 BRH	Fixed starting condition in not walkable area.
//
//		06/17/97	JMI	Converted all occurrences of rand() to GetRand() and
//							srand() to SeedRand().
//
//		06/25/97 BRH	Added use of base class 2D shadow on the ground, but loaded
//							a smaller shadow resource.
//
//		06/30/97 BRH	Added sound effect cache to Preload function.
//
//		07/01/97	JMI	Replaced GetFloorMapValue() with GetHeight() call.
//
//		07/09/97	JMI	Now uses m_pRealm->Make2dResPath() to get the fullpath
//							for 2D image components.
//
//		07/09/97	JMI	Changed Preload() to take a pointer to the calling realm
//							as a parameter.
//
//		07/18/97	JMI	Got rid of bogus immitation PlaySample functions.
//							Now there is one PlaySample() function.  Also, you now
//							MUST specify a category and you don't have to specify a
//							SoundInstance ptr to specify a volume.
//
//		07/30/97	JMI	Same old delete error showed up on Alpha.  
//							ProcessMessages() was deleting the napalm on a delete msg
//							but, once returned to Update(), it was checking the 
//							m_eState member to see if it should return.  Unfortunately,
//							since 'this' had already been deallocated, it was too late
//							to do such a thing.
//							Also, m_dFireX and m_dFireZ were uninitialized causing
//							floating point exceptions (due to bad values) on the Alpha.
//
//		08/05/97	JMI	Changed priority to use Z position rather than 2D 
//							projected Y position.
//
//		08/17/97	JMI	Changed m_pthingParent to m_idParent.
//
//		08/27/97 BRH	Added large fire sound which had not been used until now.
//
//		08/28/97 BRH	Added cache of large fire sound.
//
////////////////////////////////////////////////////////////////////////////////
#define NAPALM_CPP

#include "RSPiX.h"
#include <math.h>

#include "napalm.h"
#include "dude.h"
#include "fire.h"
#include "SampleMaster.h"
#include "game.h"
#include "reality.h"

////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////

#define SMALL_SHADOW_FILE "smallshadow.img"

////////////////////////////////////////////////////////////////////////////////
// Variables/data
////////////////////////////////////////////////////////////////////////////////

// These are default values -- actually values are set using the editor!
double CNapalm::ms_dAccDrag     = 300.0;				// Acceleration due to drag
double CNapalm::ms_dThrowVertVel = 30.0;				// Throw up at this velocity
double CNapalm::ms_dThrowHorizVel = 300;				// Throw out at this velocity
double CNapalm::ms_dMinFireInterval = 5*5;
long CNapalm::ms_lGrenadeFuseTime = 1500;			// Time from throw to blow

// Let this auto-init to 0
short CNapalm::ms_sFileCount;

/// Napalm Canister Animation Files
// An array of pointers to res names (one for each animation component)
static char* ms_apszResNames[] =
{
	"3d/napalmcan.sop",
	"3d/napalmcan.mesh",
	"3d/napalmcan.tex",
	"3d/napalmcan.hot",
	"3d/napalmcan.bounds",
	"3d/napalmcan.floor",
	NULL,
	NULL
};


////////////////////////////////////////////////////////////////////////////////
// Load object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CNapalm::Load(										// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,											// In:  File to load from
	bool bEditMode,										// In:  True for edit mode, false otherwise
	short sFileCount,										// In:  File count (unique per file, never 0)
	ULONG	ulFileVersion)									// In:  Version of file format to load.
	{
	short sResult = CWeapon::Load(pFile, bEditMode, sFileCount, ulFileVersion);

	if (sResult == SUCCESS)
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
					pFile->Read(&ms_dAccDrag);
					pFile->Read(&ms_dThrowVertVel);
					pFile->Read(&ms_dThrowHorizVel);
					pFile->Read(&ms_dMinFireInterval);
					pFile->Read(&ms_lGrenadeFuseTime);
					break;
				}
			}

		// Load object data
		switch (ulFileVersion)
			{
			default:
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
			TRACE("CNapalm::Load(): Error reading from file!\n");
			}

	}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Save object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CNapalm::Save(										// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,											// In:  File to save to
	short sFileCount)										// In:  File count (unique per file, never 0)
	{
	CWeapon::Save(pFile, sFileCount);

	// Save common data just once per file (not with each object)
	if (ms_sFileCount != sFileCount)
		{
		ms_sFileCount = sFileCount;

		// Save static data
		pFile->Write(&ms_dAccDrag);
		pFile->Write(&ms_dThrowVertVel);
		pFile->Write(&ms_dThrowHorizVel);
		pFile->Write(&ms_dMinFireInterval);
		pFile->Write(&ms_lGrenadeFuseTime);
		}

	// Save object data

	return 0;
	}


////////////////////////////////////////////////////////////////////////////////
// Update object
////////////////////////////////////////////////////////////////////////////////
void CNapalm::Update(void)
	{
	short sHeight; 
	double dNewX;
	double dNewY;
	double dNewZ;
	double dX;
	double dZ;
	double dDistance;

	if (!m_sSuspend)
		{
		// Get new time
		long lThisTime = m_pRealm->m_time.GetGameTime(); 

		// If elapsed time is too short, skip this update.

		// Calculate elapsed time in seconds
		double dSeconds = (double)(lThisTime - m_lPrevTime) / 1000.0;

		ProcessMessages();
		if (m_eState == State_Deleted)
			{
			delete this;
			// Must return now, now to avoid accessing through deallocated 
			// 'this'.
			return;
			}

		// Check the current state
		switch (m_eState)
		{

			case CWeapon::State_Idle:
				break;

			case CWeapon::State_Fire:
				// Make sure it starts in a valid location.  If it is inside
				// a wall, delete it now.
				sHeight = m_pRealm->GetHeight((short) m_dX, (short) m_dZ);
				if (m_dY < sHeight)
				{
					delete this;
					return;
				}
				m_eState = State_Go;
				m_lTimer = lThisTime + ms_lGrenadeFuseTime;
				PlaySample(
					g_smidNapalmShot,
					SampleMaster::Weapon,
					DistanceToVolume(m_dX, m_dY, m_dZ, LaunchSndHalfLife) );	// In:  Initial Sound Volume (0 - 255)
				break;

//-----------------------------------------------------------------------
// Go - fly through the air until hit the ground, change directions on
//		  obstacle collision.
//-----------------------------------------------------------------------
			case CWeapon::State_Go:
				// Do horizontal velocity
				dNewX = m_dX + COSQ[(short)m_dRot] * (m_dHorizVel * dSeconds);
				dNewZ = m_dZ - SINQ[(short)m_dRot] * (m_dHorizVel * dSeconds);

				// Do vertical velocity
				dNewY = m_dY;
				AdjustPosVel(&dNewY, &m_dVertVel, dSeconds);

				// Check the height to see if it hit the ground
				sHeight = m_pRealm->GetHeight((short) dNewX, (short) dNewZ);

				// If its lower than the last and current height, assume it
				// hit the ground.
				if (dNewY < sHeight && m_dY >= sHeight)
				{
					m_dY = sHeight;
					m_eState = CWeapon::State_Slide;	
					PlaySample(
						g_smidNapalmFire,
						SampleMaster::Destruction,
						DistanceToVolume(m_dX, m_dY, m_dZ, NapalmSndHalfLife) );	// In:  Initial Sound Volume (0 - 255)

					PlaySample(
						g_smidFireLarge,
						SampleMaster::Destruction,
						DistanceToVolume(m_dX, m_dY, m_dZ, NapalmSndHalfLife) );
				}
				else
				{
					// If it is above the last known ground and is now lower
					// than the height at its new position, assume it hit
					// a wall and should bounce.
					if (dNewY < sHeight && m_dY < sHeight)
					{
						dNewX = m_dX;	// Restore last x position
						dNewZ = m_dZ;	// Restore last z position
						m_dRot = BounceAngle(m_dRot);	// Change directions
						PlaySample(
							g_smidNapalmHit,
							SampleMaster::Weapon,
							DistanceToVolume(m_dX, m_dY, m_dZ, SideEffectSndHalfLife) );	// In:  Initial Sound Volume (0 - 255)
					}
					else
						m_dY = dNewY;
				}

				m_dX = dNewX;
				m_dZ = dNewZ;
				break;

//-----------------------------------------------------------------------
// Slide - Once it hits the ground, slide until it stops.
//-----------------------------------------------------------------------
			case CWeapon::State_Slide:

				// As the Napalm canister slides on the ground, it lays 
				// down fire at intervals.  Check the interval to see
				// it its time to creat a new fire yet.
				dX = m_dX - m_dFireX;
				dZ = m_dZ - m_dFireZ;
				dDistance = (dX*dX) + (dZ*dZ);
				if (dDistance > ms_dMinFireInterval)
				{
					m_dFireX = m_dX;
					m_dFireZ = m_dZ;
					// Start a fire here
					CFire* pFire;
					if (CThing::Construct(CThing::CFireID, m_pRealm, (CThing**) &pFire) == 0)
					{
						if (pFire->Setup(m_dX - 20 + (GetRand() % 40), m_dY, m_dZ - 20 + (GetRand() % 40), 
						                 4000 + (GetRand() % 9000), false, CFire::LargeFire) != SUCCESS)
							delete pFire;
						else
							pFire->m_u16ShooterID = m_u16ShooterID;
					}
				}
				// Ground causes drag
				// Decelerate to zero.  When you reach zero, go
				// to find state.
				if (m_dHorizVel > 0)
				{
					AdjustVel(&m_dHorizVel, dSeconds, -ms_dAccDrag);
					if (m_dHorizVel < 0)
						m_dHorizVel = 0;
				}
				else if (m_dHorizVel < 0)
				{
					AdjustVel(&m_dHorizVel, dSeconds, ms_dAccDrag);
					m_dHorizVel = 0;
				}
				// If it has stopped, then change to find state
				if (m_dHorizVel == 0)
					m_eState = CWeapon::State_Explode;

				dNewX = m_dX + COSQ[(short)m_dRot] * (m_dHorizVel * dSeconds);
				dNewZ = m_dZ - SINQ[(short)m_dRot] * (m_dHorizVel * dSeconds);
				// Check for obstacles
				sHeight = m_pRealm->GetHeight((short) dNewX, (short) dNewZ);
				// If it hit any obstacles, make it bounce off
				if (sHeight > m_dY)
				{
					// Restore previous position 
					dNewX = m_dX;
					dNewZ = m_dZ;
					// Change directions
					m_dRot = BounceAngle(m_dRot);
					PlaySample(
						g_smidNapalmHit,
						SampleMaster::Weapon,
						DistanceToVolume(m_dX, m_dY, m_dZ, SideEffectSndHalfLife) );	// In:  Initial Sound Volume (0 - 255)
				}

				// See if it fell off of something.  If so make it go back
				// to the airborne state
				if (sHeight < (short) m_dY)
				{
					m_dVertVel = 0;
					m_eState = State_Go;
				}

				m_dX = dNewX;
				m_dZ = dNewZ;

				break;


//-----------------------------------------------------------------------
// Explode
//-----------------------------------------------------------------------
			case CWeapon::State_Explode:

				delete this;
				return;

				break;
		}

		// Save time for next time
		m_lPrevTime = lThisTime;
	}
}


////////////////////////////////////////////////////////////////////////////////
// Render object
////////////////////////////////////////////////////////////////////////////////
void CNapalm::Render(void)
{
	long lThisTime = m_pRealm->m_time.GetGameTime();

	m_sprite.m_pmesh = (RMesh*) m_anim.m_pmeshes->GetAtTime(lThisTime);
	m_sprite.m_psop = (RSop*) m_anim.m_psops->GetAtTime(lThisTime);
	m_sprite.m_ptex = (RTexture*) m_anim.m_ptextures->GetAtTime(lThisTime);
	m_sprite.m_psphere = (RP3d*) m_anim.m_pbounds->GetAtTime(lThisTime);

	// Eventually this should be channel driven also
	m_sprite.m_sRadius = m_sCurRadius;

	// Reset rotation so it is not cumulative
	m_trans.Make1();

	// Set its pointing direction
	m_trans.Ry(rspMod360(m_dRot));

	if (m_eState == State_Hide)
	{
		// Hide.
		m_sprite.m_sInFlags = CSprite::InHidden;
	}
	else
	{
		// No special flags
		m_sprite.m_sInFlags = 0;
	}

	// If we're not a child of someone else...
	if (m_idParent == CIdBank::IdNil)
	{
		// Map from 3d to 2d coords
		Map3Dto2D((short) m_dX, (short) m_dY, (short) m_dZ, &m_sprite.m_sX2, &m_sprite.m_sY2);
		

		// Priority is based on bottom edge of sprite
		m_sprite.m_sPriority = m_dZ;

		// Layer should be based on info we get from attribute map
		m_sprite.m_sLayer = CRealm::GetLayerViaAttrib(m_pRealm->GetLayer((short) m_dX, (short) m_dZ));

		m_sprite.m_ptrans		= &m_trans;

		// Update sprite in scene
		m_pRealm->m_scene.UpdateSprite(&m_sprite);

		// Render the 2D shadow sprite
		CWeapon::Render();
	}
	else
	{
		// m_idParent is setting our transform relative to its position
		// and we are drawn by the scene with the parent.
	}
}

////////////////////////////////////////////////////////////////////////////////
// Setup new object - called by object that created this object
////////////////////////////////////////////////////////////////////////////////

short CNapalm::Setup(									// Returns 0 if successfull, non-zero otherwise
	short sX,												// In:  New x coord
	short sY,												// In:  New y coord
	short sZ/*,												// In:  New z coord
	double dHorizVel,										// In:  Starting Horizontal Velocity (has default)
	double dVertVel*/)									// In:  Starting Vertical Velocity (has default)
{
	short sResult = 0;
	
	// Use specified position
	m_dX = (double)sX;
	m_dY = (double)sY;
	m_dZ = (double)sZ;
	m_dHorizVel = ms_dThrowHorizVel;//dHorizVelocity;
	m_dVertVel = ms_dThrowVertVel;//dVertVelocity;

	// Default these to the start position so that, when we first enter
	// the slide state, we'll create some fire right away.
	m_dFireX	= m_dX;
	m_dFireZ	= m_dZ;

	// Load resources
	sResult = GetResources();

	// Enable the 2D shadow
	PrepareShadow();

	m_sCurRadius = 10;

	return sResult;
}



////////////////////////////////////////////////////////////////////////////////
// Get all required resources
////////////////////////////////////////////////////////////////////////////////
short CNapalm::GetResources(void)						// Returns 0 if successfull, non-zero otherwise
	{
	short sResult = 0;

	sResult = m_anim.Get(ms_apszResNames);
	if (sResult == 0)
	{
		sResult = rspGetResource(&g_resmgrGame, m_pRealm->Make2dResPath(SMALL_SHADOW_FILE), &(m_spriteShadow.m_pImage), RFile::LittleEndian);
		if (sResult == 0)
		{
			// add more gets
		}
		else
		{
			TRACE("CGrenade::GetResources - Failed to open 2D shadow image\n");
		}
	}										  
	else
	{
		TRACE("CNapalm::GetResources - Failed to open 3D animation for napalm\n");
	}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Free all resources
////////////////////////////////////////////////////////////////////////////////
short CNapalm::FreeResources(void)						// Returns 0 if successfull, non-zero otherwise
	{
	m_anim.Release();

	return 0;
	}

////////////////////////////////////////////////////////////////////////////////
// Preload - basically trick the resource manager into caching resources 
//				 for this object so there won't be a delay the first time it is
//				 created.
////////////////////////////////////////////////////////////////////////////////

short CNapalm::Preload(
	CRealm* prealm)				// In:  Calling realm.
	{
	CAnim3D anim;
	RImage* pimage;
	short sResult = anim.Get(ms_apszResNames);
	anim.Release();
	rspGetResource(&g_resmgrGame, prealm->Make2dResPath(SMALL_SHADOW_FILE), &pimage, RFile::LittleEndian);
	rspReleaseResource(&g_resmgrGame, &pimage);
	CacheSample(g_smidNapalmShot);
	CacheSample(g_smidNapalmHit);
	CacheSample(g_smidNapalmFire);
	CacheSample(g_smidFireLarge);
	return sResult;	
	}


////////////////////////////////////////////////////////////////////////////////
// ProcessMessages
////////////////////////////////////////////////////////////////////////////////

void CNapalm::ProcessMessages(void)
{
	GameMessage msg;

	if (m_MessageQueue.DeQ(&msg) == true)
	{
		switch(msg.msg_Generic.eType)
		{
			case typeObjectDelete:
				m_MessageQueue.Empty();
				m_eState = State_Deleted;
				return;
				break;
		}
	}
	// Dump the rest of the messages
	m_MessageQueue.Empty();

	return;
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
