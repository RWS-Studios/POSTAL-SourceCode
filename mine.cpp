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
// mine.cpp
// Project: Nostril (aka Postal)
//
// This module implements the CMine weapon class which is a hand
// thrown grenade weapon.
// 
//
// History:
//		03/19/97 BRH	Started this weapon object.
//
//		03/19/97 BRH	Added 4 types of mines to this file.  Still need to
//							create a dialog in the gui editor to select which type
//							of mine to place in the editor.  Also need to add motion
//							functions to the base class and use them for the 
//							Bouncing Betty in Update.
//
//		03/20/97 BRH	Added dialog box for selecting mine type.  Still need to
//							do the bouncing betty mine.
//
//		03/21/97 BRH	Added the Bouncing Betty functionality.
//
//		04/10/97 BRH	Updated this to work with the new multi layer attribute
//							maps.
//
//		04/15/97 BRH	Added fuse timer dialog item to edit modify to allow
//							the timed mine fuse to be set.  Also added an overloaded
//							Setup() function to set the time for the fuse.  Changed
//							Load and Save to deal with this new fuse value.
//
//		04/23/97	JMI	Now sets its m_smash's bits to Mine instead of Item.
//
//		04/25/97	JMI	Added angle Z (vertical angle adjustment) of 0 to the
//							CBulletFest::Fire(...) call.
//
//		04/29/97	JMI	Changed State_Fire to merely jump to State_Idle.  The
//							reason is that CCharacter uses State_Fire to notify things
//							that they should arm.  Perhaps this state should be changed
//							to 'arm', but, technically, some things don't really even
//							arm until after that.  Perhaps it should be State_Go or
//							something.
//							Anyways, in order to use State_Fire as the 'arm' trigger
//							from the placer, I had to change State_Go to do what State_Fire
//							used to for the bouncing betty; namely, wait for the time
//							to expire.
//							Also, Render() now subtracts half the width and half the
//							height from the 2D render location in an attempt to better
//							center the image.  This should help especially when this
//							object is the child of another to make it appear in the
//							right spot.
//
//		04/30/97	JMI	Changed the Setup() override of the CWeapon's Setup() to
//							pass the current mine type to the Setup() with eType.
//							Changed Construct() to take an ID as a parameter and added
//							ConstructProximity(), ConstructTimed(), 
//							ConstructBouncingBetty(), and ConstructRemoteControl() to 
//							allocate that type of mine.
//							Removed m_eMineType (now uses Class ID instead).
//							Removed Setup() that took an eType.
//							Filled in PreLoad() (but it still needs to convert each
//							RImage to whatever type is most efficient).
//							Also, GetResources() was new'ing m_pImage and then calling
//							rspGetResource() (which gives you an entirely new instance
//							of an image); so it was basically wasting an RImage worth
//							of memory.
//
//		05/28/97 BRH	Increased arming time for Betty and Proximity mines to make
//							them easier to place and get away.
//
//		05/29/97	JMI	Removed ASSERT on m_pRealm->m_pAttribMap which no longer
//							exists.
//
//		06/11/97 BRH	Added shooter ID's to the shot messages and passed it
//							along to the explosion.
//
//		06/12/97 BRH	Added shooter ID to the call to Setup for the explosion.
//
//		06/13/97	JMI	Now obeys State_Hide.
//
//		06/30/97 BRH	Added cache sound effects to Preload function.
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
//		07/21/97	JMI	Now handles delete messages.
//
//		08/05/97	JMI	Changed priority to use Z position rather than 2D 
//							projected Y position.
//
//		08/16/97 BRH	Added an arming beep sound and a click sound when the
//							mine is armed.
//
//		08/17/97	JMI	Now, instead of aborting the sample, when the mine arms,
//							it stops the sample's loopage.
//							Now sets the volume every iteration b/c, although the mine
//							stays still, the distance to the local dude varies as the
//							local dude moves.
//							Also, changed m_pthingParent to m_idParent.
//
//		08/28/97 BRH	Added preload function to cache the sounds and images.
//
////////////////////////////////////////////////////////////////////////////////
#define MINE_CPP

#include "RSPiX.h"
#include <math.h>

#include "mine.h"
#include "explode.h"
#include "SampleMaster.h"
#include "reality.h"


////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////

#define TIMEDMINE_FILE				"TimeMine.img"
#define PROXIMITYMINE_FILE			"ProxMine.img"
#define BOUNCINGBETTYMINE_FILE	"BettyMine.img"
#define REMOTEMINE_FILE				"RemoteMine.img"

////////////////////////////////////////////////////////////////////////////////
// Variables/data
////////////////////////////////////////////////////////////////////////////////

// These are default values -- actually values are set using the editor!
short CMine::ms_sProximityRadius	= 10;
short CMine::ms_sBettyRadius = 60;
short CMine::ms_sBettyRange = 1000;
long CMine::ms_lFuseTime = 6000;
long CMine::ms_lArmingTime = 5000;
long CMine::ms_lExplosionDelay = 150;
double CMine::ms_dInitialBounceVelocity = 80.0;

// Let this auto-init to 0
short CMine::ms_sFileCount;



////////////////////////////////////////////////////////////////////////////////
// Load object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CMine::Load(				// Returns 0 if successfull, non-zero otherwise
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
					pFile->Read(&ms_sProximityRadius);
					pFile->Read(&ms_sBettyRadius);
					pFile->Read(&ms_sBettyRange);
					pFile->Read(&ms_lFuseTime);
					pFile->Read(&ms_lArmingTime);
					pFile->Read(&ms_lExplosionDelay);
					pFile->Read(&ms_dInitialBounceVelocity);
					break;
			}
		}

		// Load object data
		MineType	type;
		switch (ulFileVersion)
		{
			case 1:
			case 2:
			case 3:
			case 4:
				pFile->Read(&type);
				break;

			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
			case 10:
				pFile->Read(&type);
			
			case 11:
			default:
				pFile->Read(&m_lFuseTime);
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
			TRACE("CMine::Load(): Error reading from file!\n");
		}
	}

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Save object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CMine::Save(										// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,											// In:  File to save to
	short sFileCount)										// In:  File count (unique per file, never 0)
{
	// In most cases, the base class Save() should be called.  In this case it
	// isn't because the base class doesn't have a Save()!
	CWeapon::Save(pFile, sFileCount);

	// Save common data just once per file (not with each object)
	if (ms_sFileCount != sFileCount)
	{
		ms_sFileCount = sFileCount;

		// Save static data
		pFile->Write(&ms_sProximityRadius);
		pFile->Write(&ms_sBettyRadius);
		pFile->Write(&ms_sBettyRange);
		pFile->Write(&ms_lFuseTime);
		pFile->Write(&ms_lArmingTime);
		pFile->Write(&ms_lExplosionDelay);
		pFile->Write(&ms_dInitialBounceVelocity);
	}

	// Save object data
	pFile->Write(&m_lFuseTime);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Startup
////////////////////////////////////////////////////////////////////////////////

short CMine::Startup(void)
{
	return Init();
}

////////////////////////////////////////////////////////////////////////////////
// Init
////////////////////////////////////////////////////////////////////////////////

short CMine::Init(void)
{
	short sResult = SUCCESS;

	m_eState = State_Idle;

	long lThisTime = m_pRealm->m_time.GetGameTime();

	if (m_lFuseTime == 0)
		m_lFuseTime = ms_lFuseTime;

	switch (m_id)
	{
		case CTimedMineID:
			m_lTimer = m_lFuseTime + lThisTime;

			//This is needed to fix the crash when dropping timed mine
			//Bug is something to do with collision detection later on in smash.cpp
			m_sCurRadius = ms_sProximityRadius;

			break;

		case CProximityMineID:
			m_lTimer = ms_lArmingTime + lThisTime;
			m_sCurRadius = ms_sProximityRadius;
			break;

		case CBouncingBettyMineID:
			m_lTimer = ms_lArmingTime + lThisTime;
			m_sCurRadius = ms_sBettyRadius;
			break;

		case CRemoteControlMineID:
			m_lTimer = lThisTime;
			break;
	}

	// Set up collision object
	m_smash.m_bits = CSmash::Mine;
	m_smash.m_pThing = this;

	// Load resources
	sResult = GetResources();

	return sResult;

}

////////////////////////////////////////////////////////////////////////////////
// This inline updates a specified velocity with a specified drag over a
// specified time.
////////////////////////////////////////////////////////////////////////////////
inline
bool UpdateVelocity(		// Returns true if velocity reaches zero because of the
								// supplied accelration, false otherwise.
	double* pdVel,			// In:  Initial velocity.
								// Out: New velocity.
	double* pdDeltaVel,	// Out: Delta velocity.
	double dAcc,			// In:  Acceleration.
	double dSeconds)		// In:  Elapsed time in seconds.
	{
	bool	bAcceleratedToZero	= false;

	double	dVelPrev	= *pdVel;
	*pdDeltaVel			= dAcc * dSeconds;
	*pdVel				+= *pdDeltaVel;

	// I think this can be consdensed into a subtraction and one or two comparisons,
	// but I'm not sure that's really faster than the max 3 comparisons here.
	// If previously traveling forward . . .
	if (dVelPrev > 0.0)
		{
		// Passing 0 is considered at rest . . .
		if (*pdVel < 0.0)
			{
			// Update delta.
			*pdDeltaVel	-= *pdVel;
			// Zero velocity.
			*pdVel	= 0.0;
			}
		}
	else
		{
		// If previously traveling backward . . .
		if (dVelPrev < 0.0)
			{
			// Passing 0 is considered at rest . . .
			if (*pdVel > 0.0)
				{
				// Update delta.
				*pdDeltaVel	-= *pdVel;
				// Zero velocity.
				*pdVel	= 0.0;
				}
			}
		}

	// If velocity is now zero . . .
	if (*pdVel == 0.0)
		{
		// If drag opposed the previous velocity . . .
		if ((dVelPrev > 0.0 && dAcc < 0.0) || (dVelPrev < 0.0 && dAcc > 0.0))
			{
			// Drag has achieved its goal.
			bAcceleratedToZero = true;
			}
		}

	return bAcceleratedToZero;
	}

////////////////////////////////////////////////////////////////////////////////
// Update object
////////////////////////////////////////////////////////////////////////////////
void CMine::Update(void)
{
	CSmash* pSmashed = NULL;
	double dDistance;
	short sShootAngle;
	short sShotX;
	short sShotY;
	short sShotZ;
	CThing* pShotThing = NULL;
	GameMessage msg;

	if (!m_sSuspend)
	{
		// Get new time
		long lThisTime = m_pRealm->m_time.GetGameTime(); 

		ProcessMessages();
		if (m_eState == State_Deleted)
			{
			delete this;
			return;
			}

		// Calculate elapsed time in seconds
		double dSeconds = (double)(lThisTime - m_lPrevTime) / 1000.0;

		// Check the current state
		switch (m_eState)
		{

//-----------------------------------------------------------------------
// Idle - waiting to arm
//-----------------------------------------------------------------------
			case CWeapon::State_Idle:
				if (lThisTime > m_lTimer)
				{
					// End the looping arming sound
					if (m_siMineBeep != 0)
					{
						StopLoopingSample(m_siMineBeep);
						m_siMineBeep = 0;
					}
					switch (m_id)
					{
						case CTimedMineID:
							m_eState = State_Explode;
							break;

						case CProximityMineID:
						case CBouncingBettyMineID:
							m_eState = State_Armed;
							PlaySample(
								g_smidMineSet,				// Sample to play
								SampleMaster::Weapon,	// Category for user sound adjustment
								DistanceToVolume(m_dX, m_dY, m_dZ, MineSndHalfLife) ); //Pos
							break;

						case CRemoteControlMineID:
						default:
							break;
					}
				}
				else
				{
					short	sX	= m_dX;
					short	sY	= m_dY;
					short	sZ	= m_dZ;

					// If we have a parent . . .
					CThing*	pthing	= NULL;	// Initialized for safety.
					if (m_pRealm->m_idbank.GetThingByID(&pthing, m_idParent) == 0)
						{
						// Add in its position.
						sX	+= pthing->GetX();
						sY	+= pthing->GetY();
						sZ	+= pthing->GetZ();
						}

					// Update sound position.
					SetInstanceVolume(m_siMineBeep, DistanceToVolume(sX, sY, sZ, MineSndHalfLife) );
				}
				break;

//-----------------------------------------------------------------------
// Armed - State for Proximity mine & bouncing betty where they check
//			  for collisions with other characters and react.
//-----------------------------------------------------------------------

			case CWeapon::State_Armed:
				if (m_pRealm->m_smashatorium.QuickCheck(&m_smash, 
															CSmash::Character, 
														   CSmash::Good | CSmash::Bad,
															0, &pSmashed))
				{
					switch (m_id)
					{
						case CBouncingBettyMineID:
							m_eState = State_Go;
							m_dVertVel = ms_dInitialBounceVelocity;
							// Make it go off right away
							m_lTimer = lThisTime;
							break;

						case CProximityMineID:
							m_eState = State_Explode;
							// Make it go off right away
							m_lTimer = lThisTime;
							break;
					}
				}
				break;

//-----------------------------------------------------------------------
// Fire - Initial triggering notification from character (if placed by
//			 character).
//-----------------------------------------------------------------------

			case CWeapon::State_Fire:
				// Go back to waiting to arm.
				m_eState	= State_Idle;

				break;


//-----------------------------------------------------------------------
// Go - used for bouncing betty when it is bouncing up
//-----------------------------------------------------------------------
			case CWeapon::State_Go:
				if (lThisTime > m_lTimer)
				{
					PlaySample(
						g_smidBounceLaunch,
						SampleMaster::Weapon,
						DistanceToVolume(m_dX, m_dY, m_dZ, LaunchSndHalfLife) );	// In:  Initial Sound Volume (0 - 255)
					// Do motion
					///////////////////////// Vertical Velocity /////////////////////////////////
					UpdateVelocity(&m_dVertVel, &m_dVertDeltaVel, g_dAccelerationDueToGravity, dSeconds);
					
					// Apply external vertical velocity.
					dDistance	= (m_dVertVel - m_dVertDeltaVel / 2) * dSeconds;
					m_dY = m_dY + dDistance;

					// If velocity is negative, then explode and shoot in 
					// several directions using deluxe shot or something.
					if (m_dVertVel <= 0.0)
					{
						// Make a small explosion noise
						PlaySample(
							g_smidBounceExplode,
							SampleMaster::Weapon,
							DistanceToVolume(m_dX, m_dY, m_dZ, ExplosionSndHalfLife) );	// In:  Initial Sound Volume (0 - 255)

						// Draw some kind of flash on the mine
						m_bulletfest.Impact(0, m_dX, m_dY, m_dZ, m_pRealm);
						m_bulletfest.Impact(0, m_dX + 5, m_dY, m_dZ, m_pRealm);
						m_bulletfest.Impact(0, m_dX - 5, m_dY, m_dZ, m_pRealm);
						m_bulletfest.Impact(0, m_dX, m_dY + 5, m_dZ, m_pRealm);
						m_bulletfest.Impact(0, m_dX, m_dY - 5, m_dZ, m_pRealm);

						// Shoot in all directions
						for (sShootAngle = 0; sShootAngle < 360; sShootAngle += 20)
						{
							m_bulletfest.Fire(sShootAngle,
													0,
													(short) m_dX, 
													(short) m_dY, 
													(short) m_dZ,
													ms_sBettyRange,
													m_pRealm,
													CSmash::Character,
													CSmash::Good | CSmash::Bad,
													0,
													&sShotX,
													&sShotY,
													&sShotZ,
													&pShotThing);
							if (pShotThing != NULL)
							{
								msg.msg_Shot.eType = typeShot;
								msg.msg_Shot.sPriority = 0;
								msg.msg_Shot.sDamage = 50;
								msg.msg_Shot.sAngle = rspATan(m_dZ - sShotZ, sShotX - m_dX);
								msg.msg_Shot.u16ShooterID = m_u16ShooterID;
								// Tell this thing that it got shot
								SendThingMessage(&msg, pShotThing);
							}
						}
						// Get rid of the mine
						delete this;
						return;
					}
				}

				break;

//-----------------------------------------------------------------------
// Explode
//-----------------------------------------------------------------------
			case CWeapon::State_Explode:

				if (lThisTime > m_lTimer)
				{
					// Start an explosion object and then kill rocket
					// object
					CExplode* pExplosion;
					if (CThing::Construct(CThing::CExplodeID, m_pRealm, (CThing**) &pExplosion) == 0)
					{
						pExplosion->Setup(m_dX, m_dY, m_dZ, m_u16ShooterID);
						PlaySample(
							g_smidGrenadeExplode,
							SampleMaster::Destruction,
							DistanceToVolume(m_dX, m_dY, m_dZ, ExplosionSndHalfLife) );	// In:  Initial Sound Volume (0 - 255)
					}

					delete this;
					return;
				}
				break;
		}

		// Save time for next time
		m_lPrevTime = lThisTime;

		// Update sphere.
		m_smash.m_sphere.sphere.X			= m_dX;
		m_smash.m_sphere.sphere.Y			= m_dY;
		m_smash.m_sphere.sphere.Z			= m_dZ;
		m_smash.m_sphere.sphere.lRadius	= m_sCurRadius;

		// Update the smash.
		m_pRealm->m_smashatorium.Update(&m_smash);

	}
}


////////////////////////////////////////////////////////////////////////////////
// Render object
////////////////////////////////////////////////////////////////////////////////
void CMine::Render(void)
{
	if (m_pImage)
	{
		// Image would normally animate, but doesn't for now
		m_sprite.m_pImage = m_pImage;

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

		// Map from 3d to 2d coords
		Map3Dto2D(
			(short) m_dX, 
			(short) m_dY, 
			(short) m_dZ, 
			&m_sprite.m_sX2, 
			&m_sprite.m_sY2);

		// Center on image.
		m_sprite.m_sX2	-= m_pImage->m_sWidth / 2;
		m_sprite.m_sY2	-= m_pImage->m_sHeight / 2;

		// Priority is based on bottom edge of sprite
		m_sprite.m_sPriority = m_dZ;

		// Layer should be based on info we get from attribute map.
		m_sprite.m_sLayer = CRealm::GetLayerViaAttrib(m_pRealm->GetLayer((short) m_dX, (short) m_dZ));

		// Update sprite in scene
		m_pRealm->m_scene.UpdateSprite(&m_sprite);
	}
}

////////////////////////////////////////////////////////////////////////////////
// Setup new object - called by object that created this object
//							 This version is meant for the timed mine so that
//							 the fuse time can be set.
////////////////////////////////////////////////////////////////////////////////

short CMine::Setup(									// Returns 0 if successful, non-zero otherwise
	short sX,											// In:  X coord placement
	short sY,											// In:  Y coord placement
	short sZ,											// In:  Z coord placement
	long lFuseTime)									// In:  ms before mine goes off (timed mine only)
{
	m_lFuseTime = lFuseTime;
	return Setup(sX, sY, sZ);
}

////////////////////////////////////////////////////////////////////////////////
// Setup new object - called by object that created this object
////////////////////////////////////////////////////////////////////////////////

short CMine::Setup(									// Returns 0 if successfull, non-zero otherwise
	short sX,											// In:  New x coord
	short sY,											// In:  New y coord
	short sZ)											// In:  New z coord
	{
	// Loop the Arming sound
	PlaySample(
		g_smidMineBeep,								// In:  sound to play
		SampleMaster::Weapon,						// In:  user volume adjustment category
		DistanceToVolume(sX, sY, sZ, MineSndHalfLife), // Position
		&m_siMineBeep,									// Out: Handle to sound so it can be stopped
		NULL,												// Out: Sample duration in ms
		0,													// In:  Where to loop to
		-1,												// In:  Where to loop from
															// In:  If less than 1, the end + lLoopEndTime is used.
		false);											// In:  Call ReleaseAndPurge rather than Release at end

	
	short	sResult	=  CWeapon::Setup(sX, sY, sZ);
	if (sResult == 0)
		{
		sResult	= Init();
		}
	
	return sResult;
	}

////////////////////////////////////////////////////////////////////////////////
// Get all required resources
////////////////////////////////////////////////////////////////////////////////
short CMine::GetResources(void)						// Returns 0 if successfull, non-zero otherwise
{
	short sResult = 0;

	if (m_pImage == 0)
	{
		switch (m_id)
		{	
			case CTimedMineID:
				sResult = rspGetResource(&g_resmgrGame, m_pRealm->Make2dResPath(TIMEDMINE_FILE), &m_pImage, RFile::LittleEndian);
				break;

			case CProximityMineID:
				sResult = rspGetResource(&g_resmgrGame, m_pRealm->Make2dResPath(PROXIMITYMINE_FILE), &m_pImage, RFile::LittleEndian);
				break;

			case CBouncingBettyMineID:
				sResult = rspGetResource(&g_resmgrGame, m_pRealm->Make2dResPath(BOUNCINGBETTYMINE_FILE), &m_pImage, RFile::LittleEndian);
				break;

			case CRemoteControlMineID:
				sResult = rspGetResource(&g_resmgrGame, m_pRealm->Make2dResPath(REMOTEMINE_FILE), &m_pImage, RFile::LittleEndian);
				break;
		}
	}

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Free all resources
////////////////////////////////////////////////////////////////////////////////
short CMine::FreeResources(void)						// Returns 0 if successfull, non-zero otherwise
{
	rspReleaseResource(&g_resmgrGame, &m_pImage);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Helper inline to get a GUI, set its text to the value, and recompose it.
////////////////////////////////////////////////////////////////////////////////
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
// Edit Modify
////////////////////////////////////////////////////////////////////////////////

short CMine::EditModify(void)
{
	short sResult = 0;
	RGuiItem* pGui = RGuiItem::LoadInstantiate(FullPathVD("res/editor/mine.gui"));
	if (pGui)
	{
		SetText(pGui, 7, m_lFuseTime);

		sResult = DoGui(pGui);
		if (sResult == 1)
		{
			m_lFuseTime = pGui->GetVal(7);
		}
	}
	delete pGui;

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Called by editor to init new object at specified position
////////////////////////////////////////////////////////////////////////////////
short CMine::EditNew(									// Returns 0 if successfull, non-zero otherwise
	short sX,												// In:  New x coord
	short sY,												// In:  New y coord
	short sZ)												// In:  New z coord
{
	short sResult = 0;

	if (sResult == SUCCESS)
	{
		// Load resources
		sResult = Setup(sX, sY, sZ);
	}
	else
	{
		sResult = -1;
	}

	return sResult;
}

////////////////////////////////////////////////////////////////////////////////
// Preload - basically trick the resource manager into caching resources 
//				 for this object so there won't be a delay the first time it is
//				 created.
////////////////////////////////////////////////////////////////////////////////

short CMine::Preload(
	CRealm* prealm)				// In:  Calling realm.
{
	short	sResult;
	RImage*	pim;

	sResult = rspGetResource(&g_resmgrGame, prealm->Make2dResPath(TIMEDMINE_FILE), &pim, RFile::LittleEndian);
	if (sResult == 0)
		{
		rspReleaseResource(&g_resmgrGame, &pim);
		sResult = rspGetResource(&g_resmgrGame, prealm->Make2dResPath(PROXIMITYMINE_FILE), &pim, RFile::LittleEndian);
		if (sResult == 0)
			{
			rspReleaseResource(&g_resmgrGame, &pim);
			sResult = rspGetResource(&g_resmgrGame, prealm->Make2dResPath(BOUNCINGBETTYMINE_FILE), &pim, RFile::LittleEndian);
			if (sResult == 0)
				{
				rspReleaseResource(&g_resmgrGame, &pim);
				}
			}
		}

	CacheSample(g_smidBounceLaunch);
	CacheSample(g_smidBounceExplode);
	CacheSample(g_smidGrenadeExplode);
	CacheSample(g_smidMineBeep);
	CacheSample(g_smidMineSet);

	// This should convert each one of these images.
	return sResult; 
}

////////////////////////////////////////////////////////////////////////////////
// Explosion Message handler
////////////////////////////////////////////////////////////////////////////////

void CMine::OnExplosionMsg(Explosion_Message* pMessage)
{
	// If we got blown up, go off in whatever manner this type
	// of mine would normally go off.
	switch (m_id)
	{
		// If its a bouncing betty, and hasn't already been
		// triggered, then trigger it to bounce up.
		case CBouncingBettyMineID:
			if (m_eState == State_Idle || m_eState == State_Armed) 
			{
				m_eState = State_Go;
				m_dVertVel = ms_dInitialBounceVelocity;
				m_lTimer = m_pRealm->m_time.GetGameTime() + ms_lExplosionDelay;
			}
			break;

		// If its any other type, just make it explode if it
		// is not exploding already
		default:
			if (m_eState != State_Explode)
			{
				m_eState = State_Explode;
				m_lTimer = m_pRealm->m_time.GetGameTime() + ms_lExplosionDelay;
			}
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////
// Trigger Message handler
////////////////////////////////////////////////////////////////////////////////

void CMine::OnTriggerMsg(Trigger_Message* pMessage)
{
	// If we are a remote control mine & we got the trigger message,
	// then blow up.
	if (m_id == CRemoteControlMineID)
		m_eState = State_Explode;
}

////////////////////////////////////////////////////////////////////////////////
// Handles an ObjectDelete_Message.
////////////////////////////////////////////////////////////////////////////////
void CMine::OnDeleteMsg(					// Returns nothing.
	ObjectDelete_Message* pdeletemsg)	// In:  Message to handle.
	{
	// Go to deleted state.  Update() will delete us.
	m_eState	= State_Deleted;
	}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
