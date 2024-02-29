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
// firebomb.cpp
// Project: Postal
//
// This module implements the CFirebomb weapon class which is a hand
// thrown grenade weapon that explodes into a ring of fire..
// 
//
// History:
//		01/17/97 BRH	Started this weapon object.
//
//		02/04/97	JMI	Changed LoadDib() call to Load() (which now supports
//							loading of DIBs).
//
//		02/11/97 BRH	Started the firebomb object from the grenade file
//
//		02/14/97 BRH	Now uses resource manager to get the image.
//
//		02/19/97 BRH	Changed the main part of the weapon to 3D and left
//							the fire fragments as 2D.  Also added some randomness
//							to the pattern and increased the burn times.
//
//		02/19/97 BRH	Added ProcessMessages function to check for 
//							ObjectDeleted messages.
//
//		02/21/97 BRH	Changed fragments to be invisible controllers of the
//							small fire animation rather than an 2d sprite.  This
//							sort of hides the bounce effect which is bad so we may
//							want to put it back.  Also uses the small fire for
//							the fire fragments.
//
//		02/23/97 BRH	Changed the coordinate system to x,-z
//
//		02/23/97 BRH	Added a static Preload() function which will be called 
//							before play begins to cache the resources needed for this
//							object.
//
//		02/24/97	JMI	No longer sets the m_type member of the m_sprite b/c it
//							is set by m_sprite's constructor.
//
//		02/24/97 BRH	Changed to using reality.h motion templates.  Using new
//							algorithm for detecting ground and walls.  Made the 
//							center fire more than 1 sprite and changed to thin
//							fire.  Added sound effect for initial impact.
//
//		02/28/97 BRH	Derived this from the CWeapon base class.
//
//		03/03/97	JMI	Changed reference to CGrenade::State_Deleted to 
//							CWeapon::State_Deleted.
//
//		03/06/97 BRH	Changed to using ID's for keeping track of the file
//							and gettting the pointer from the ID each time.
//
//		03/13/97	JMI	Load()s now take a version number.
//
//		03/21/97 BRH	Changed this to ignore ATTRIBUTE_NOT_WALKABLE so that
//							the cocktails won't bounce off of the edge of the world.
//
//		04/10/97 BRH	Updated this to work with the new multi layer attribute
//							maps.
//
//		05/04/97 BRH	Took out an old unused reference to an STL iterator.
//
//		05/13/97	JMI	CFireBomb was using a formula to compute the direction of
//							8 firefrags such that one appeared in a random position
//							in all of 8 octants.  The problem was the formula subtract-
//							ed 25 which, in the case of the first octant, if the random
//							position 24 or less were chosen, would result in a negative
//							value.  But, to simply remove this could cause values 360
//							or over since the random portion was based on 50.0 and not
//							the size of an octant.  So I removed the -25 and changed the
//							50.0 to (360/8) which, as far as I can tell, makes the 
//							inclusive extents of the formula [0..359].
//
//		05/29/97	JMI	Removed ASSERT on m_pRealm->m_pAttribMap which no longer
//							exists.
//
//		06/03/97 BRH	Changed the cocktails so they don't bounce off of 
//							walls, they just fall to the ground and break.
//
//		06/11/97 BRH	Added shooter ID passing to the fires that it creates.
//
//		06/12/97	JMI	Now handles State_Hide by setting m_sprite's InHidden flag.
//
//		06/12/97 BRH	Fixed order of passing the shooter ID.
//
//		06/16/97 BRH	Fixed starting condition in not walkable area.
//
//		06/18/97 BRH	Changed over to using GetRandom()
//
//		06/25/97 BRH	Added use of base class 2D shadow on the ground, but loaded
//							a smaller shadow resource.
//
//		06/30/97 BRH	Added cache of sound effects in Preload function.
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
//		07/27/97	JMI	Changed to use Z position (i.e., X/Z plane) instead of
//							Y2 position (i.e., viewing plane) position for draw 
//							priority.
//
//		07/30/97	JMI	Same old delete error showed up on Alpha.  
//							ProcessMessages() was deleting the firebomb on a delete msg
//							but, once returned to Update(), it was checking the 
//							m_eState member to see if it should return.  Unfortunately,
//							since 'this' had already been deallocated, it was too late
//							to do such a thing.
//
//		08/17/97	JMI	Changed m_pthingParent to m_idParent.
//
//		08/20/97 BRH	Moved firebomb sound to Destruction volume control.
//
//		08/27/97 BRH	Added large fire sound which had not been used until now.
//
//		08/28/97	JMI	Added a explode counter so we can cap the number of 
//							explosions a firefrag can make.
//
//		08/28/97 BRH	Added cache of large fire sound.
//
////////////////////////////////////////////////////////////////////////////////
#define FIREBOMB_CPP

#include "RSPiX.h"
#include <math.h>

#include "firebomb.h"
#include "dude.h"
#include "fire.h"
#include "SampleMaster.h"
#include "game.h"
#include "reality.h"

////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////

#define PRIMARY_BURN_TIME 10000
#define SECONDARY_BURN_TIME 7000
#define SMALL_SHADOW_FILE "smallshadow.img"

////////////////////////////////////////////////////////////////////////////////
// Variables/data
////////////////////////////////////////////////////////////////////////////////

// These are default values -- actually values are set using the editor!

double CFirebomb::ms_dCloseDistance = 30.0;			// Close enough to hit CDude
double CFirebomb::ms_dThrowVertVel = 40.0;				// Throw up at this velocity
double CFirebomb::ms_dThrowHorizVel = 250;				// Throw out at this velocity

// Let this auto-init to 0
short CFirebomb::ms_sFileCount;

/// Grenade Animation Files
// An array of pointers to res names (one for each animatino component)
static char* ms_apszResNames[] =
{
	"3d/grenade.sop",
	"3d/grenade.mesh",
	"3d/grenade.tex",
	"3d/grenade.hot",
	"3d/grenade.bounds",
	"3d/grenade.floor",
	NULL,
	NULL
};


////////////////////////////////////////////////////////////////////////////////
// Load object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CFirebomb::Load(				// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,						// In:  File to load from
	bool bEditMode,					// In:  True for edit mode, false otherwise
	short sFileCount,					// In:  File count (unique per file, never 0)
	ULONG	ulFileVersion)				// In:  Version of file format to load.
{
	short sResult = 0;

	sResult = CWeapon::Load(pFile, bEditMode, sFileCount, ulFileVersion);
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
			TRACE("CFirebomb::Load(): Error reading from file!\n");
		}
	}

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Save object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CFirebomb::Save(										// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,											// In:  File to save to
	short sFileCount)										// In:  File count (unique per file, never 0)
{
	CWeapon::Save(pFile, sFileCount);

	// Save common data just once per file (not with each object)
	if (ms_sFileCount != sFileCount)
		{
		ms_sFileCount = sFileCount;

		// Save static data
		}

	// Save object data

	return 0;
}



////////////////////////////////////////////////////////////////////////////////
// Update object
////////////////////////////////////////////////////////////////////////////////
void CFirebomb::Update(void)
{
	USHORT usAttrib;
	short sHeight = m_sPrevHeight;
	double dNewX;
	double dNewY;
	double dNewZ;

	if (!m_sSuspend)
	{
		// Get new time
		long lThisTime = m_pRealm->m_time.GetGameTime();

		// Calculate elapsed time in seconds
		double dSeconds = (double)(lThisTime - m_lPrevTime) / 1000.0;

		ProcessMessages();

		if (m_eState == State_Deleted)
			{
			delete this;
			return;
			}

		// Check the current state
		switch (m_eState)
		{

			case CFirebomb::State_Idle:
				break;

			case CFirebomb::State_Fire:
				// Make sure we start in a valid position.  If we are staring
				// inside a wall, just delete this object now.
				usAttrib = m_pRealm->GetFloorAttribute((short) m_dX, (short) m_dZ);
				sHeight = m_pRealm->GetHeight((short) m_dX, (short) m_dZ);
				if (m_dY < sHeight)
				{
					delete this;
					return;
				}
				m_eState = State_Go;
//				m_lTimer = lThisTime + ms_lGrenadeFuseTime;
				break;

//-----------------------------------------------------------------------
// Go - fly through the air until hit the ground, change directions on
//		  obstacle collision.
//-----------------------------------------------------------------------
			case CFirebomb::State_Go:
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
					m_eState = CFirebomb::State_Explode;	
				}
				else
				{
					// If it is above the last known ground and is now lower
					// than the height at its new position, assume it hit
					// a wall and should fall (this is where it used to bounce)
					if (dNewY < sHeight && m_dY < sHeight)
					{
						dNewX = m_dX;	// Restore last x position
						dNewZ = m_dZ;	// Restore last z position
						m_dRot = BounceAngle(m_dRot);	// Change directions
						m_dHorizVel = 0.5;
					}
					else
						m_dY = dNewY;
				}

				m_dX = dNewX;
				m_dZ = dNewZ;
				break;

//-----------------------------------------------------------------------
// Explode - Once it hits the ground, break into fire fragments that 
//			    bounce out from this point.
//-----------------------------------------------------------------------
			case CFirebomb::State_Explode:

				CFire* pFire;
				if (CThing::Construct(CThing::CFireID, m_pRealm, (CThing**) &pFire) == 0)
				{
					pFire->Setup(m_dX, m_dY, m_dZ, PRIMARY_BURN_TIME, true, CFire::LargeFire);
					pFire->m_u16ShooterID = m_u16ShooterID;
					PlaySample(
						g_smidFirebomb, 
						SampleMaster::Destruction,
						DistanceToVolume(m_dX, m_dY, m_dZ, FireBombSndHalfLife) );	// In:  Initial Sound Volume (0 - 255)

					PlaySample(
						g_smidFireLarge,
						SampleMaster::Destruction,
						DistanceToVolume(m_dX, m_dY, m_dZ, FireBombSndHalfLife) ); 
				}

				// Loop to create 8 fragments in a circular pattern
				short i;
				CFirefrag* pFrag;
				for (i = 0; i < 8; i++)
				{
					if (CThing::Construct(CThing::CFirefragID, m_pRealm, (CThing**) &pFrag) == 0)
					{
						pFrag->m_u16ShooterID = m_u16ShooterID;
						pFrag->Setup(m_dX, m_dY, m_dZ);
						pFrag->m_dVertVel = m_dVertVel * -0.5;
						pFrag->m_dHorizVel = 60.0;
//						pFrag->m_dRot = (i * (360/8)) - 25 + (GetRandom() % 50);
						pFrag->m_dRot = (i * (360/8)) + (GetRandom() % (360 / 8));
						pFrag->m_eState = CWeapon::State_Go;												
					}
				}

				delete this;
				return;

				break;
		}

		// Save height for next time
		m_sPrevHeight = sHeight;

		// Save time for next time
		m_lPrevTime = lThisTime;
	}
}


////////////////////////////////////////////////////////////////////////////////
// Render object
////////////////////////////////////////////////////////////////////////////////
void CFirebomb::Render(void)
{
	// Animate

	long lThisTime = m_pRealm->m_time.GetGameTime();

	m_sprite.m_pmesh = (RMesh*) m_anim.m_pmeshes->GetAtTime(lThisTime);
	m_sprite.m_psop = (RSop*) m_anim.m_psops->GetAtTime(lThisTime);
	m_sprite.m_ptex = (RTexture*) m_anim.m_ptextures->GetAtTime(lThisTime);
	m_sprite.m_psphere = (RP3d*) m_anim.m_pbounds->GetAtTime(lThisTime);

	// Eventually this should be channel driven also
	m_sprite.m_sRadius = m_sCurRadius;

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
		// Priority is based on our Z position.
		m_sprite.m_sPriority = m_dZ;

		// Layer should be based on info we get from attribute map
		m_sprite.m_sLayer = CRealm::GetLayerViaAttrib(m_pRealm->GetLayer((short) m_dX, (short) m_dZ));

		m_sprite.m_ptrans		= &m_trans;

		// Update sprite in scene
		m_pRealm->m_scene.UpdateSprite(&m_sprite);

		// Draw the 2D shadow
		CWeapon::Render();
	}
	else
	{
		// m_idParent is setting out transform relative to its position
		// and we are drawn by the scene with the parent.
	}
}

////////////////////////////////////////////////////////////////////////////////
// Setup new object - called by object that created this object
////////////////////////////////////////////////////////////////////////////////

short CFirebomb::Setup(									// Returns 0 if successfull, non-zero otherwise
	short sX,												// In:  New x coord
	short sY,												// In:  New y coord
	short sZ)												// In:  New z coord
{
	short sResult = 0;
	
	// Use specified position
	m_dX = (double)sX;
	m_dY = (double)sY;
	m_dZ = (double)sZ;
	m_dHorizVel = ms_dThrowHorizVel;
	m_dVertVel = ms_dThrowVertVel;

	// HARD-WIRED CODE ALERT!
	// Eventually, this should be set via the bounding sphere radius.
	m_sCurRadius	= 22;		// FOR NOW, always half of scene.cpp:SCREEN_DIAMETER_FOR_3D.


	// Load resources
	sResult = GetResources();

	PrepareShadow();
	
	return sResult;
}

////////////////////////////////////////////////////////////////////////////////
// Get all required resources
////////////////////////////////////////////////////////////////////////////////
short CFirebomb::GetResources(void)						// Returns 0 if successfull, non-zero otherwise
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
		TRACE("CFirebomb::GetResources - Failed to open 3D animation for firebomb\n");
	}
	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Free all resources
////////////////////////////////////////////////////////////////////////////////
short CFirebomb::FreeResources(void)						// Returns 0 if successfull, non-zero otherwise
{
	m_anim.Release();

	return SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
// Preload - basically trick the resource manager into caching resources 
//				 for this object so there won't be a delay the first time it is
//				 created.
////////////////////////////////////////////////////////////////////////////////

short CFirebomb::Preload(
	CRealm* prealm)				// In:  Calling realm.
{
	CAnim3D anim;	
	RImage* pimage;
	short sResult = anim.Get(ms_apszResNames);
	anim.Release();
	rspGetResource(&g_resmgrGame, prealm->Make2dResPath(SMALL_SHADOW_FILE), &pimage, RFile::LittleEndian);
	rspReleaseResource(&g_resmgrGame, &pimage);
	CacheSample(g_smidFirebomb);
	CacheSample(g_smidFireLarge);
	return sResult;	
}


////////////////////////////////////////////////////////////////////////////////
// ProcessMessages
////////////////////////////////////////////////////////////////////////////////

void CFirebomb::ProcessMessages(void)
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
// Firefrag
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////

#define FRAG_IMAGE_FILE			"res\\grenade.bmp"

// Minimum elapsed time (in milliseconds)


////////////////////////////////////////////////////////////////////////////////
// Variables/data
////////////////////////////////////////////////////////////////////////////////

// These are default values -- actually values are set using the editor!
double CFirefrag::ms_dAccUser     = 250.0;				// Acceleration due to user
double CFirefrag::ms_dAccDrag     = 300.0;				// Acceleration due to drag

double CFirefrag::ms_dGravity = -9.5;					// acceleration due to gravity
double CFirefrag::ms_dThrowVertVel = 10.0;				// Throw up at this velocity
double CFirefrag::ms_dThrowHorizVel = 60;				// Throw out at this velocity
double CFirefrag::ms_dMinBounceVel = 30.0;					// Min amount needed to bounce up
double CFirefrag::ms_dVelTransferFract = -0.4;			// Amount of velocity to bounce back up
short	CFirefrag::ms_sMaxExplosions	= 4;					// Maximum explosions before death.
// Let this auto-init to 0
short CFirefrag::ms_sFileCount;


////////////////////////////////////////////////////////////////////////////////
// Load object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CFirefrag::Load(				// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,						// In:  File to load from
	bool bEditMode,					// In:  True for edit mode, false otherwise
	short sFileCount,					// In:  File count (unique per file, never 0)
	ULONG	ulFileVersion)				// In:  Version of file format to load.
{
	short sResult = 0;

	sResult = CWeapon::Load(pFile, bEditMode, sFileCount, ulFileVersion);
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
					pFile->Read(&ms_dAccUser);
					pFile->Read(&ms_dAccDrag);
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
			TRACE("CFirefrag::Load(): Error reading from file!\n");
		}
	}

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Save object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CFirefrag::Save(										// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,											// In:  File to save to
	short sFileCount)										// In:  File count (unique per file, never 0)
{
	CWeapon::Save(pFile, sFileCount);

	// Save common data just once per file (not with each object)
	if (ms_sFileCount != sFileCount)
		{
		ms_sFileCount = sFileCount;

		// Save static data
		pFile->Write(&ms_dAccUser);
		pFile->Write(&ms_dAccDrag);
		}

	// Save object data

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Update object
////////////////////////////////////////////////////////////////////////////////
void CFirefrag::Update(void)
{
	short sHeight = m_sPrevHeight;
	double dPrevVertVel;
	double dNewX;
	double dNewY;
	double dNewZ;

	if (!m_sSuspend)
	{
		// Get new time
		long lThisTime = m_pRealm->m_time.GetGameTime(); 

		// Calculate elapsed time in seconds
		double dSeconds = (double)(lThisTime - m_lPrevTime) / 1000.0;

		// Check the current state
		switch (m_eState)
		{

			case CWeapon::State_Idle:
				break;

			case CWeapon::State_Fire:
				m_eState = State_Go;
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
//				m_dVertVel += ms_dGravity;
//				m_dY += m_dVertVel * dSeconds;
				dPrevVertVel = m_dVertVel;
				dNewY = m_dY;
				AdjustPosVel(&dNewY, &m_dVertVel, dSeconds);
				// Check the height to see if it hit the ground
				sHeight = m_pRealm->GetHeight((short) dNewX, (short) dNewZ);

				// If its lower than the last and current height, assume it
				// hit the ground.
//				if (m_dY <= m_sPrevHeight && m_dY <= sHeight)
				if (dNewY < sHeight && m_dY >= sHeight)
				{
					m_dY = sHeight;
					m_eState = CWeapon::State_Explode;	
					m_dVertVel = dPrevVertVel;
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
						ASSERT(m_dRot >= 0.0 && m_dRot < 360.0);
					}
					else
						m_dY = dNewY;
				}

				m_dX = dNewX;
				m_dZ = dNewZ;
				break;

//-----------------------------------------------------------------------
// Explode - Once it hits the ground, break into fire fragments that 
//			    bounce out from this point.
//-----------------------------------------------------------------------
			case CWeapon::State_Explode:

				CFire* pFire;
				if (CThing::Construct(CThing::CFireID, m_pRealm, (CThing**) &pFire) == 0)
				{
					pFire->Setup(m_dX, m_dY, m_dZ, SECONDARY_BURN_TIME, true, CFire::SmallFire);
					pFire->m_u16ShooterID = m_u16ShooterID;
//							PlaySample(g_smidGrenadeExplode);
				}

				m_sNumExplosions++;

				// If you have enough velocity to bounce and we've not exceeded
				// the maximum number of explosions, redirect velocity
				// upward and bounce, else kill yourself off.
				if (-m_dVertVel > ms_dMinBounceVel && m_sNumExplosions <= ms_sMaxExplosions)
				{
					m_dVertVel = m_dVertVel * ms_dVelTransferFract;
					m_eState = CWeapon::State_Go;
				}
				else
				{
					m_pRealm->m_idbank.GetThingByID((CThing**) &m_pFire, m_u16FireID);
					if (m_pFire)
					{
						GameMessage msg;
						msg.msg_ObjectDelete.eType = typeObjectDelete;
						msg.msg_ObjectDelete.sPriority = 0;
						SendThingMessage(&msg, m_pFire);
					}
					delete this;
					return;
				}

				break;
		}

		m_pRealm->m_idbank.GetThingByID((CThing**) &m_pFire, m_u16FireID);
		if (m_pFire)
		{
			m_pFire->m_dX = m_dX;
			m_pFire->m_dY = m_dY;
			m_pFire->m_dZ = m_dZ;
		}

		// Save height for next time
		m_sPrevHeight = sHeight;

		// Save time for next time
		m_lPrevTime = lThisTime;
	}
}


////////////////////////////////////////////////////////////////////////////////
// Render object
////////////////////////////////////////////////////////////////////////////////
void CFirefrag::Render(void)
{
/*
	// This is a standard 2d sprite
	m_sprite.m_type = CSprite::Standard2d;

	// No special flags
	m_sprite.m_sInFlags = 0; 

	// Map from 3d to 2d coords
	m_sprite.m_sX2 = m_dX - (m_pImage->m_sWidth / 2);
	m_sprite.m_sY2 = (m_dZ - (m_pImage->m_sHeight)) - m_dY;

	// Priority is based on bottom edge of sprite
	m_sprite.m_sPriority = m_sprite.m_sY2 + m_pImage->m_sHeight;

	// Layer should be based on info we get from attribute map, but is hardwired for now
//	m_sprite.m_sLayer = 0;
	ASSERT(m_pRealm					!= NULL);
	ASSERT(m_pRealm->m_pAttribMap	!= NULL);
	// Layer should be based on info we get from attribute map.
	m_sprite.m_sLayer = CRealm::GetLayerViaAttrib(m_pRealm->GetLayer((short) m_dX, (short) m_dZ));

	// Image would normally animate, but doesn't for now
	m_sprite.m_pImage = m_pImage;

	// Update sprite in scene
	m_pRealm->m_scene.UpdateSprite(&m_sprite);
*/
}

////////////////////////////////////////////////////////////////////////////////
// Setup new object - called by object that created this object
////////////////////////////////////////////////////////////////////////////////

short CFirefrag::Setup(									// Returns 0 if successfull, non-zero otherwise
	short sX,												// In:  New x coord
	short sY,												// In:  New y coord
	short sZ)												// In:  New z coord
{
	short sResult = 0;
	
	// Use specified position
	m_dX = (double)sX;
	m_dY = (double)sY;
	m_dZ = (double)sZ;
	m_lPrevTime = m_pRealm->m_time.GetGameTime();
	m_dVertVel = ms_dThrowVertVel;
	m_dHorizVel = ms_dThrowHorizVel;

	// Load resources
//	sResult = GetResources();

	if (CThing::Construct(CThing::CFireID, m_pRealm, (CThing**) &m_pFire) == 0)
	{
		m_pFire->Setup(m_dX, m_dY, m_dZ, SECONDARY_BURN_TIME, true, CFire::SmallFire);
		m_pFire->m_u16ShooterID = m_u16ShooterID;
	}


	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Get all required resources
////////////////////////////////////////////////////////////////////////////////
short CFirefrag::GetResources(void)						// Returns 0 if successfull, non-zero otherwise
{
	short sResult = 0;

	if (m_pImage == 0)
	{
		m_pImage = new RImage;
		if (m_pImage)
		{
			sResult = m_pImage->Load(FRAG_IMAGE_FILE);
			if (sResult == 0)
			{
				if (m_pImage->Convert(RImage::FSPR8) != RImage::FSPR8)
				{
					sResult = -1;
					TRACE("CFirefrag::GetResource(): Couldn't convert to FSPR8!\n");
				}
			}
		}
		else
		{
			sResult = -1;
			TRACE("CFirefrag::GetResources(): Couldn't allocate RImage!\n");
		}
	}

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Free all resources
////////////////////////////////////////////////////////////////////////////////
short CFirefrag::FreeResources(void)						// Returns 0 if successfull, non-zero otherwise
{
	short sResult = 0;

	if (m_pImage != 0)
	{
		delete m_pImage;
		m_pImage = 0;
	}

	return sResult;
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////

