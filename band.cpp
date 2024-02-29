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
// band.cpp
// Project: Postal
//
//	This module implements the marching band member.
//
// History:
//		03/04/97 BRH	Started this file.
//
//		03/05/97 BRH	Implemented the functionality of this class in this
//							function and in the base class CCharacter.  This uses
//							many of the default base class functions for motion, states
//							etc.  Currently has the logic to follow the parade route
//							and simple after Parade mingling.  Also reacts to
//							shot, fire and explosions.  Still need to add panic
//							mode and panic message sending.
//
//		03/06/97 BRH	Fixed panic message so it won't interrupt dying.  Also
//							Use AlignToBouy function to set the direction to bouy
//							and to periodically readjust the alighment to the bouy.
//
//		03/06/97	JMI	Upgraded to current rspMod360 usage.
//							Was commented out, but just in case it ever gets re-
//							instated.
//
//		03/07/97 BRH	Added dialog box to select starting bouy and also 
//							saves and loads that bouy ID.
//
//		03/13/97	JMI	Load now takes a version number.
//
//		03/18/97	JMI	Now saves and loads child ID.
//							Render() now checks for child item and, if present, updates
//							its transform via the band member's rigid body transform.
//							EditModify() now allows one to select type of child.
//							OnExplosionMsg() now detaches child items.
//
//		03/18/97	JMI	Load() was ignoring versions 2 and 3.  Fixed.
//
//		03/18/97	JMI	OnDead() now drops current child instrument.
//
//		03/18/97	JMI	Update() was calling CCharacter::OnDead() bypassing the
//							CBand::OnDead() override.
//
//		03/19/97 BRH	Added a check to the OnPanicMsg to make sure that the
//							bouy chosen exists before getting its position.
//
//		03/27/97	JMI	Now you cannot create a CBand when there is no bouy.
//
//		04/10/97 BRH	Changed it to work with the new multi layer attribute maps.
//
//		04/16/97 BRH	Changed references to the realm's list of CThings to use
//							the new non-STL method.
//
//		04/29/97	JMI	Now, in Render(), the band guy has safer interaction with
//							his child instrument which, also, does not keep him from
//							having a weapon, like it did before.
//
//		05/12/97 BRH	Added the randomness to the falling down dead state so
//							when you kill the marchers they don't all fall the same
//							direction.
//
//		05/26/97 BRH	Added avoidance of obstacles.
//
//		05/27/97 BRH	Fixed problem in Panic where no random bouy was being
//							selected.
//
//		05/29/97	JMI	Changed instance of REALM_ATTR_FLOOR_MASK to 
//							REALM_ATTR_NOT_WALKABLE.
//
//		06/03/97 BRH	Changed the mingle so they walk around a lot more.  Added
//							screaming sound effects when they panic so that they aren't
//							totally silent as they run around.  Also changed the song
//							to be played internally rather than using a sound thing
//							object.  
//
//		06/04/97	JMI	Now aborts ms_pBandSongSound, if not NULL, in destructor.
//							Also, added ms_bDonePlaying so marchers know when to not
//							restart ms_pBandSongSound.
//
//		06/05/97	JMI	Changed m_sHitPoints to m_stockpile.m_sHitPoints to 
//							accommodate new m_stockpile in base class, CThing3d (which
//							used to contain the m_sHitPoints).
//
//		06/08/97	JMI	Added override for WhileDying() and WhileShot().  In which
//							we make sure to drop child items with some random
//							rotation velocity and our direction and velocity.
//							Also, OnExplosionMsg() now passes the message on to the
//							child item immediately after dropping it.
//
//		06/10/97 BRH	Added message passing to CDemon for all band members.
//
//		06/16/97 BRH	Added more sound effects for the band members.
//
//		06/17/97	JMI	Added NULL in call to PlaySample() corresponding to new
//							param.
//
//		06/17/97	JMI	Converted all occurrences of rand() to GetRand() and
//							srand() to SeedRand().
//
//		06/24/97	JMI	Added some LOG() calls for the synchronization log 
//							mechanism.
//
//		06/25/97	JMI	Now calls PrepareShadow() in Init() which loads and sets up
//							a shadow sprite.
//
//		06/30/97 MJR	Replaced SAFE_GUI_REF with new GuiItem.h-defined macro.
//
//		06/30/97 BRH	Changed most of the PlaySample calls to PlaySampleThenPurge
//							to save memory.  
//
//		07/01/97	JMI	Replaced GetFloorMapValue() with GetHeightAndNoWalk() call.
//
//		07/01/97	JMI	Now passes rigid body transform to DetachChild().
//
//		07/08/97 BRH	Renamed some of the bandguy's animations since the
//							filenames were too long for the delicate MacOS.
//
//		07/09/97	JMI	Removed unused 2D res name macros.
//
//		07/17/97	JMI	Changed ms_pBandSongSound to ms_siBandSongInstance.
//							Now uses new SampleMaster interface for volume and play
//							instance reference.
//
//		07/18/97	JMI	Got rid of bogus immitation PlaySample functions.
//							Now there is one PlaySample() function.  Also, you now
//							MUST specify a category and you don't have to specify a
//							SoundInstance ptr to specify a volume.
//
//		07/31/97 BRH	Changed destination bouy from always being 1 to an editable
//							item in the EditModify dialog box.
//
//		08/12/97	JMI	Now one band member maintains the volume for the band 
//							sample.
//
//		08/26/97 BRH	Added a few more voices to the list.
//
//		09/03/97	JMI	Replaced Good Smash bit with Civilian.
//
//		09/24/97 BRH	Band members will not appear for the non US version.  This
//							needs to be tested with the new project setup once 
//							the band is moved into the localized projects.
//
//		10/03/97	JMI	Now includes CompileOptions.h so it knows what US is when
//							comparing to LOCALE.
//
//		10/14/97	JMI	Added GetInstanceID() as parameter to LOG() calls.
//
//		09/27/99	JMI	Changed to allow band mebmers only in any locale 
//							satisfying the CompilerOptions macro VIOLENT_LOCALE.
//
////////////////////////////////////////////////////////////////////////////////
#define BAND_CPP

#include "RSPiX.h"
#include <math.h>

#include "band.h"
#include "SampleMaster.h"
#include "item3d.h"

#include "CompileOptions.h"

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
#define RAND_SWAY(sway)	((GetRand() % sway) - sway / 2)

#define GUI_ID_OK						1

#define BAND_SONG_HALF_LIFE		500

////////////////////////////////////////////////////////////////////////////////
// Variables/data
////////////////////////////////////////////////////////////////////////////////

double CBand::ms_dCloseToBouy = 6*6;
double CBand::ms_dMingleBouyDist = 15*15;
double CBand::ms_dExplosionVelocity = 180.0;
double CBand::ms_dMaxMarchVel = 30.0;
double CBand::ms_dMaxRunVel = 80.0;
long CBand::ms_lMingleTime = 400;
short CBand::ms_sStartingHitPoints = 100;
SampleMaster::SoundInstance CBand::ms_siBandSongInstance = 0;
U16	CBand::ms_idBandLeader	= CIdBank::IdNil;		// The person who adjusts the band sound
																	// volume or IdNil.

// Let this auto-init to 0
short CBand::ms_sFileCount;

// This value indicates whether the marchers have stopped playing in this level.
bool	CBand::ms_bDonePlaying	= false;

/// Standing Animation Files
// An array of pointers to resource names (one for each channel of the animation)
static char* ms_apszStandResNames[] = 
{
	"3d/bandg_stand.sop",
	"3d/bandg_stand.mesh",
	"3d/bandg_stand.tex",
	"3d/bandg_stand.hot",
	"3d/bandg_stand.bounds",
	"3d/bandg_stand.floor",
	"3d/bandg_stand_instrument.trans",
	NULL	
};

/// Running Animation Files
// An array of pointers to resource names (one for each channel of the animation)
static char* ms_apszRunResNames[] = 
{
	"3d/bandg_run.sop",
	"3d/bandg_run.mesh",
	"3d/bandg_run.tex",
	"3d/bandg_run.hot",
	"3d/bandg_run.bounds",
	"3d/bandg_run.floor",
	"3d/bandg_run_instrument.trans",
	NULL
};

/// Throwing Animation Files 
// An array of pointers to resource names (one for each channel of the animation)
static char* ms_apszMarchResNames[] = 
{
	"3d/bandg_march.sop",
	"3d/bandg_march.mesh",
	"3d/bandg_march.tex",
	"3d/bandg_march.hot",
	"3d/bandg_march.bounds",
	"3d/bandg_march.floor",
	"3d/bandg_march_instrument.trans",
	NULL
};

// Shot Animation Files
// An array of pointers to resource names (one for each channel of the animation)
static char* ms_apszShotResNames[] = 
{
	"3d/bandg_shot.sop",
	"3d/bandg_shot.mesh",
	"3d/bandg_shot.tex",
	"3d/bandg_shot.hot",
	"3d/bandg_shot.bounds",
	"3d/bandg_shot.floor",
	"3d/bandg_shot_instrument.trans",
	NULL
};

/// Blown up Animation Files
// An array of pointers to resource names (one for each channel of the animation)
static char* ms_apszBlownupResNames[] =
{
	"3d/bandg_blownup.sop",
	"3d/bandg_blownup.mesh",
	"3d/bandg_blownup.tex",
	"3d/bandg_blownup.hot",
	"3d/bandg_blownup.bounds",
	"3d/bandg_blownup.floor",
	"3d/bandg_blownup_instrument.trans",
	NULL
};

/// OnFire Animation Files
// An array of pointers to resource names (one for each channel of the animation)
static char* ms_apszOnFireResNames[] =
{
	"3d/bandg_onfire.sop",
	"3d/bandg_onfire.mesh",
	"3d/bandg_onfire.tex",
	"3d/bandg_onfire.hot",
	"3d/bandg_onfire.bounds",
	"3d/bandg_onfire.floor",
	"3d/bandg_onfire_instrument.trans",
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
short CBand::Load(					// Returns 0 if successfull, non-zero otherwise
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
					break;
				}

			// Clear this so we know no one has paniced yet.
			// Note that, if no band members are loaded (i.e., they are all
			// created during play (maybe via a dispenser, although dispensers
			// use load) ), this won't get reset.
			ms_bDonePlaying		= false;
			}

		// Load Rocket Man specific data
			switch (ulFileVersion)
			{
				default:
				case 37:
					pFile->Read(&m_ucDestBouyID);
					pFile->Read(&m_idChildItem);
					pFile->Read(&m_eWeaponType);
					pFile->Read(&m_ucNextBouyID);
					break;
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
					pFile->Read(&m_idChildItem);
					pFile->Read(&m_eWeaponType);
					pFile->Read(&m_ucNextBouyID);
					break;
				case 3:
				case 2:
				case 1:
					pFile->Read(&m_eWeaponType);
					pFile->Read(&m_ucNextBouyID);
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
			TRACE("CBand::Load(): Error reading from file!\n");
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
short CBand::Save(										// Returns 0 if successfull, non-zero otherwise
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

	// Save band member specific data
	pFile->Write(&m_ucDestBouyID);
	pFile->Write(&m_idChildItem);
	pFile->Write(&m_eWeaponType);
	pFile->Write(&m_ucNextBouyID);

	if (!pFile->Error())
	{
		sResult = SUCCESS;
	}
	else
	{
		TRACE("CBand::Save() - Error writing to file\n");
		sResult = -1;
	}

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Init - Call this after the resources are in place
////////////////////////////////////////////////////////////////////////////////

short CBand::Init(void)
{
	short sResult = 0;

	// Prepare shadow (get resources and setup sprite).
	sResult	= PrepareShadow();

	// Init position, rotation and velocity
	m_dVel = 0.0;
	m_dRot = 0.0;
	m_lPrevTime = m_pRealm->m_time.GetGameTime();
	m_state = CCharacter::State_Idle;
	m_lTimer = m_pRealm->m_time.GetGameTime() + 500;
	m_sBrightness = 0;	// Default brightness

	m_smash.m_bits		= CSmash::Civilian | CSmash::Character;
	m_smash.m_pThing	= this;

	m_lAnimTime = 0;
	m_panimCur = &m_animMarch;
	m_stockpile.m_sHitPoints = ms_sStartingHitPoints;

	// Set them facing their first bouy so they are lined up ready to march
//	m_ucDestBouyID = 1;		// This is the end of the parade route bouy
//	m_ucNextBouyID = m_pNavNet->FindNearestBouy(m_dX, m_dZ);
	m_pNextBouy = m_pNavNet->GetBouy(m_ucNextBouyID);
//	ASSERT(m_pNextBouy != NULL);
	if (m_pNextBouy != NULL)
		{
		m_sNextX = m_pNextBouy->GetX();
		m_sNextZ = m_pNextBouy->GetZ();
	//	m_dRot = rspATan(m_dZ - m_sNextZ, m_sNextX - m_dX);
		AlignToBouy();
		}
	else
		{
		TRACE("Init():  Where's the dang, blam, dangin, blamin, BOUY?!\n");
		sResult	= -1;
		}

	m_state = CCharacter::State_March;
	m_dAcc = 150;

	return sResult;
}

////////////////////////////////////////////////////////////////////////////////
// Startup object
////////////////////////////////////////////////////////////////////////////////
short CBand::Startup(void)								// Returns 0 if successfull, non-zero otherwise
{
// If not a violent locale . . . 
#if !VIOLENT_LOCALE
	// We must kill band members in these countries b/c of their lack of tolerance.
	delete this;
	return 0;
#else

	// Set the current height, previous time, and Nav Net by calling the
	// base class startup
	CDoofus::Startup();

	// Init other stuff
	return Init();
#endif
}


////////////////////////////////////////////////////////////////////////////////
// Update object
////////////////////////////////////////////////////////////////////////////////
void CBand::Update(void)
{
	short sHeight = m_sPrevHeight;
	double dNewX;
	double dNewY;
	double dNewZ;
	double dX;
	double dZ;
	double dStartX;
	double dStartZ;
	long lThisTime;
	long lTimeDifference;
	CThing* pDemon = NULL;

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
// March - follow the parade route until you get to the end
//-----------------------------------------------------------------------

			case State_March:
				// The first guy should start the song, if we are not done playing music . . .
				if (ms_siBandSongInstance == 0 && ms_bDonePlaying == false)
				{
					PlaySample(										// Returns nothing.
																		// Does not fail.
						g_smidParadeSong,							// In:  Identifier of sample you want played.
						SampleMaster::Unspecified,				// In:  Sound Volume Category for user adjustment
						255,											// In:  Initial Sound Volume (0 - 255)
						&ms_siBandSongInstance,					// Out: Handle for adjusting sound volume
						NULL,											// Out: Sample duration in ms, if not NULL.
						0,												// In:  Where to loop back to in milliseconds.
																		//	-1 indicates no looping (unless m_sLoop is
																		// explicitly set).
						-1,											// In:  Where to loop back from in milliseconds.
																		// In:  If less than 1, the end + lLoopEndTime is used.
						false);										// In:  Call ReleaseAndPurge rather than Release after playing

					// Make this guy the band leader.
					ms_idBandLeader	= GetInstanceID();
				}

				// If I am the band leader . . .
				if (ms_idBandLeader == GetInstanceID() )
				{
					// If the band song is running . . .
					if (ms_siBandSongInstance != 0)
						{
						// Adjust the sound volume.  This doesn't need to be exact.  So
						// his previous position will be fine.
						SetInstanceVolume(
							ms_siBandSongInstance,
							DistanceToVolume(m_dX, m_dY, m_dZ, BAND_SONG_HALF_LIFE) );
						}
				}

				// Check distance to target bouy
				dX = m_dX - m_sNextX;
				dZ = m_dZ - m_sNextZ;
				if ((dX*dX + dZ*dZ) < ms_dCloseToBouy)
				{
					// Set next bouy, x, z, and rotation
					m_ucNextBouyID = m_pNextBouy->NextRouteNode(m_ucDestBouyID);
					if (m_ucNextBouyID == 0)
					{
						// Note that we're done playing music.
						ms_bDonePlaying	= true;

						m_lTimer = lThisTime;
						m_state = State_Wait;
						// At the end of the parade, end the song
						if (ms_siBandSongInstance != 0)
						{
							AbortSample(ms_siBandSongInstance);
							ms_siBandSongInstance = 0;
							ms_bDonePlaying		= true;
						}
					}
					else
					{
						m_pNextBouy = m_pNavNet->GetBouy(m_ucNextBouyID);
						m_sNextX = m_pNextBouy->GetX();
						m_sNextZ = m_pNextBouy->GetZ();
//						m_dRot = rspATan(m_dZ - m_sNextZ, m_sNextX - m_dX);
						AlignToBouy();
					}
				}
				// Move towards the bouy
				AlignToBouy();
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
		
				break;

//-----------------------------------------------------------------------
// Wait - stand around for a while
//-----------------------------------------------------------------------

			case State_Wait:
				// See if its time to go yet
				if (lThisTime > m_lTimer)
				{
					m_state = State_Mingle;
					m_ucDestBouyID = SelectRandomBouy();
					m_ucNextBouyID = m_pNavNet->FindNearestBouy(m_dX, m_dZ);
					m_pNextBouy = m_pNavNet->GetBouy(m_ucNextBouyID);
					m_lTimer = lThisTime + ms_lMingleTime;
					if (m_ucDestBouyID == 0 || m_pNextBouy == NULL)
					{
						m_state = State_Wait;
					}
					else
					{
						m_ucNextBouyID = m_pNextBouy->NextRouteNode(m_ucDestBouyID);
						m_pNextBouy = m_pNavNet->GetBouy(m_ucNextBouyID);
						if (m_pNextBouy != NULL)
						{
							m_sNextX = m_pNextBouy->GetX();
							m_sNextZ = m_pNextBouy->GetZ();
//							m_dRot = rspATan(m_dZ - m_sNextZ, m_sNextX - m_dX);
							AlignToBouy();
							m_dAcc = 150;
							m_state = State_Mingle;
						}
						else
							m_state = State_Wait;
					}
				}
				break;

//-----------------------------------------------------------------------
// Mingle - Mingle around at the park after the parade
//-----------------------------------------------------------------------
/*
			case State_Mingle:
				// Check distance to target bouy
				dX = m_dX - m_sNextX;
				dZ = m_dZ - m_sNextZ;
				if ((dX*dX + dZ*dZ) < ms_dMingleBouyDist)
				{
					m_lTimer = lThisTime + ms_lMingleTime;
					m_state = State_Wait;
				}
				// Move towards the bouy
				AlignToBouy();
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
					
				break;
*/
//-----------------------------------------------------------------------
// Panic - Pick a random bouy and run to it.  When you are there, pick
//			  a different random bouy and run to it.  
//-----------------------------------------------------------------------

			case State_Panic:
			case State_Mingle:
				// Check distance to target bouy
				dStartX = m_dX;
				dStartZ = m_dZ;
				dX = m_dX - m_sNextX;
				dZ = m_dZ - m_sNextZ;

				// BEGIN TEMP.
				LOG(dX, GetInstanceID() );
				LOG(dZ, GetInstanceID() );

				LOG(m_dX, GetInstanceID() );
				LOG(m_dZ, GetInstanceID() );

				LOG(m_sNextX, GetInstanceID() );
				LOG(m_sNextZ, GetInstanceID() );
				// END TEMP.

				if ((dX*dX + dZ*dZ) < ms_dCloseToBouy)
				{
					// Set next bouy, x, z, and rotation
					m_ucNextBouyID = m_pNextBouy->NextRouteNode(m_ucDestBouyID);
					// BEGIN TEMP.
					LOG(m_pNextBouy->m_ucID, GetInstanceID() );
					LOG(m_ucDestBouyID, GetInstanceID() );
					LOG(m_ucNextBouyID, GetInstanceID() );
					// END TEMP.

					if (m_ucNextBouyID == 0 || m_ucNextBouyID == 255)
					{
						if (m_panimCur != &m_animRun)
							m_panimCur = &m_animRun;
						m_ucDestBouyID = SelectRandomBouy();
						m_ucNextBouyID = m_pNextBouy->NextRouteNode(m_ucDestBouyID);
						m_sNextX = m_pNextBouy->GetX();
						m_sNextZ = m_pNextBouy->GetZ();
						AlignToBouy();
						// BEGIN TEMP.
						LOG(m_ucDestBouyID, GetInstanceID() );
						LOG(m_ucNextBouyID, GetInstanceID() );
						LOG(m_sNextX, GetInstanceID() );
						LOG(m_sNextZ, GetInstanceID() );
						// END TEMP.

						m_sRotateDir = GetRand() % 2;
						if (m_state == State_Mingle)
						{
							m_state = State_Wait;
							m_lTimer = lThisTime + ms_lMingleTime;
						}
						else
						{
							short sRandom = GetRand() % 16;
							switch (sRandom)
							{
								case 0:
									PlaySample(g_smidSteveAhFire, SampleMaster::Voices);
									break;

								case 1:
									PlaySample(g_smidBlownupFemaleYell, SampleMaster::Voices);
									break;

								case 2:
									PlaySample(g_smidCarynScream, SampleMaster::Voices);
									break;

								case 3:
									PlaySample(g_smidTinaScream1, SampleMaster::Voices);
									break;

								case 4:
									PlaySample(g_smidPaulAhah, SampleMaster::Voices);
									break;

								case 5:
									PlaySample(g_smidTinaOhMyGod, SampleMaster::Voices);
									break;

								case 6:
									PlaySample(g_smidAndreaHesPostal, SampleMaster::Voices);
									break;

								case 7:
									PlaySample(g_smidAndreaHesManiac, SampleMaster::Voices);
									break;

								case 8:
									PlaySample(g_smidCelinaRun, SampleMaster::Voices);
									break;

								case 9:
									PlaySample(g_smidPaulCantFeelLegs, SampleMaster::Voices);
									break;

								case 10:
									PlaySample(g_smidAndreaYell, SampleMaster::Voices);
									break;

								case 11:
									PlaySample(g_smidSteveWaFire, SampleMaster::Voices);
									break;

								case 12:
									PlaySample(g_smidRandyCantFeelLegs, SampleMaster::Voices);
									break;

								case 13:
									PlaySample(g_smidSteveMyEyes, SampleMaster::Voices);
									break;

								case 14:
									PlaySample(g_smidSteveMyLeg, SampleMaster::Voices);
									break;

								case 15:
									PlaySample(g_smidSteveCantSeeAny, SampleMaster::Voices);
									break;
							}
						}
					}
					else
					{
						m_pNextBouy = m_pNavNet->GetBouy(m_ucNextBouyID);
						if (m_pNextBouy != NULL)
						{
							m_sNextX = m_pNextBouy->GetX();
							m_sNextZ = m_pNextBouy->GetZ();
							AlignToBouy();
						}
					}
				}

				if (lThisTime > m_lAlignTimer)
					AlignToBouy();

				// Move towards the bouy
//				DeluxeUpdatePosVel();
				if (m_state == State_Mingle)
					UpdateVelocities(dSeconds, ms_dMaxMarchVel, ms_dMaxMarchVel);
				else
					UpdateVelocities(dSeconds, ms_dMaxRunVel, ms_dMaxRunVel);
				GetNewPosition(&dNewX, &dNewY, &dNewZ, dSeconds);

				// Get height and 'no walk' status at new position.
				bool		bNoWalk;
				sHeight	= m_pRealm->GetHeightAndNoWalk(dNewX, dNewY, &bNoWalk);

				// If too big a height difference or completely not walkable . . .
				if (bNoWalk == true
					|| (sHeight - dNewY > 10) )// && m_bAboveTerrain == false && m_dExtHorzVel == 0.0))
				{
					m_ucDestBouyID = SelectRandomBouy();
					m_ucNextBouyID = m_pNavNet->FindNearestBouy(m_dX, m_dZ);
					m_sNextX = m_pNextBouy->GetX();
					m_sNextZ = m_pNextBouy->GetZ();
					m_lAlignTimer = lThisTime + 3000;
					AlignToBouy();
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
					m_ucDestBouyID = SelectRandomBouy();
					m_ucNextBouyID = m_pNavNet->FindNearestBouy(m_dX, m_dZ);
					m_sNextX = m_pNextBouy->GetX();
					m_sNextZ = m_pNextBouy->GetZ();
					m_lAlignTimer = lThisTime + 3000;
					m_dRot = rspMod360(m_dRot + 20);
				}

				// If not moving when you are trying to, rotate
				if (m_dX == dStartX && m_dZ == dStartZ)
				{
					if (m_sRotateDir)
						m_dAnimRot = m_dRot = rspMod360(m_dRot + 20);
					else
						m_dAnimRot = m_dRot = rspMod360(m_dRot - 20);
				}
				else
				{
					m_sRotateDir = GetRand() % 2;
				}

				break;

//-----------------------------------------------------------------------
// Burning - Run around on fire until dead
//-----------------------------------------------------------------------

			case State_Burning:
				if (!WhileBurning())
				{
					m_state = State_Die;
					m_lAnimTime = 0;
					m_panimCur = &m_animShot;
					// Fall down in a more random direction.
					m_dRot = rspMod360(m_dRot - 90 + (GetRand() % 180));
				}				
				break;

//-----------------------------------------------------------------------
// Blown Up - Do motion into the air until you hit the ground again
//-----------------------------------------------------------------------

			case State_BlownUp:
				if (!WhileBlownUp())
					m_state = State_Dead;
				else
					UpdateFirePosition();
				break;

//-----------------------------------------------------------------------
// Shot - Dies in one shot
//-----------------------------------------------------------------------

			case State_Shot:
				if (!WhileShot())
				{
					m_state = State_Die;
					m_dRot = rspMod360(m_dRot - 90 + (GetRand() % 180));
				}
				break;

//-----------------------------------------------------------------------
// Die - run die animation until done, the you are dead
//-----------------------------------------------------------------------

			case State_Die:
				if (!WhileDying())
					m_state = State_Dead;
				else
					UpdateFirePosition();
				
				break;

//-----------------------------------------------------------------------
// Dead - paste yourself in the background and delete yourself
//-----------------------------------------------------------------------

			case State_Dead:
				GameMessage msg;
				msg.msg_Death.eType = typeDeath;
				msg.msg_Death.sPriority = 0;
				pDemon = m_pRealm->m_aclassHeads[CThing::CDemonID].GetNext();
				if (pDemon)
					SendThingMessage(&msg, pDemon);				
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
void CBand::Render(void)
{
	// Call base class.
	CDoofus::Render();

	// Update child, if any . . .
	if (m_idChildItem != CIdBank::IdNil)
	{
		CItem3d*	pitem;
		if (m_pRealm->m_idbank.GetThingByID((CThing**)&pitem, m_idChildItem) == 0)
		{
			// Set transform from our rigid body transfanimation for the child
			// sprite.
			pitem->m_sprite.m_ptrans	= (RTransform*) m_panimCur->m_ptransRigid->GetAtTime(m_lAnimTime);
			// If the item is not our child . . .
			if (pitem->m_sprite.m_psprParent != &m_sprite)
				{
				// Make it so.
				m_sprite.AddChild( &(pitem->m_sprite) );
				}
		}
		else	// Safety:
		{
			// Item is gone.
			m_idChildItem	= CIdBank::IdNil;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// Called by editor to init new object at specified position
////////////////////////////////////////////////////////////////////////////////
short CBand::EditNew(									// Returns 0 if successfull, non-zero otherwise
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

short CBand::EditModify(void)
{
	short sResult = 0;
	RGuiItem* pGui = RGuiItem::LoadInstantiate(FullPathVD("res/editor/band.gui"));
	if (pGui)
	{
		RGuiItem*	pguiStartBouy = pGui->GetItemFromId(10);
		RGuiItem*	pguiDestBouy  = pGui->GetItemFromId(11);

		if (pguiStartBouy)
		{
			RSP_SAFE_GUI_REF_VOID(pguiStartBouy, SetText("%d", m_ucNextBouyID));
			RSP_SAFE_GUI_REF((REdit*) pguiStartBouy, m_sCaretPos = strlen(pguiStartBouy->m_szText));
			RSP_SAFE_GUI_REF_VOID(pguiStartBouy, Compose());
			
			RSP_SAFE_GUI_REF_VOID(pguiDestBouy, SetText("%d", m_ucDestBouyID));
			RSP_SAFE_GUI_REF((REdit*) pguiDestBouy, m_sCaretPos = strlen(pguiDestBouy->m_szText));
			RSP_SAFE_GUI_REF_VOID(pguiDestBouy, Compose());

			CItem3d::ItemType	itChild	= CItem3d::None;
			// If there's currently a child . . .
			CItem3d*	pitem	= NULL;
			if (m_pRealm->m_idbank.GetThingByID((CThing**)&pitem, m_idChildItem) == 0)
				{
				// Get the type.
				itChild	= pitem->m_type;
				}

			RListBox*	plbChildTypes	= (RListBox*)pGui->GetItemFromId(3);
			if (plbChildTypes != NULL)
				{
				ASSERT(plbChildTypes->m_type == RGuiItem::ListBox);
				
				// Add all built-in 3D Item types.
				short	i;
				RGuiItem*	pguiItem;
				for (i = CItem3d::None; i < CItem3d::NumTypes; i++)
					{
					// Don't allow Custom . . .
					if (i != CItem3d::Custom)
						{
						pguiItem	= plbChildTypes->AddString(CItem3d::ms_apszKnownAnimDescriptions[i]);
						if (pguiItem != NULL)
							{
							// Set item number.
							pguiItem->m_ulUserData	= i;
							// If this item is the current item type . . .
							if (i == itChild)
								{
								plbChildTypes->SetSel(pguiItem);
								}
							}
						}
					}

				plbChildTypes->AdjustContents();
				}

			if (DoGui(pGui) == GUI_ID_OK)
			{
				m_ucNextBouyID = RSP_SAFE_GUI_REF(pguiStartBouy, GetVal());
				m_ucDestBouyID = RSP_SAFE_GUI_REF(pguiDestBouy, GetVal());
				if (plbChildTypes != NULL)
					{
					RGuiItem*	pguiSel	= plbChildTypes->GetSel();
					if (pguiSel != NULL)
						{
						itChild	= (CItem3d::ItemType)pguiSel->m_ulUserData;
						}
					else
						{
						// None.
						itChild	= CItem3d::None;
						}

					if (pitem != NULL)
						{
						// If it is not of the new type . . .
						if (pitem->m_type != itChild)
							{
							// Disable item.
							DetachChild(
								&m_idChildItem,
								(RTransform*) m_panimCur->m_ptransRigid->GetAtTime(m_lAnimTime) );
							// Be gone.
							delete pitem;
							pitem	= NULL;
							}
						}

					// If there is no child item . . .
					if (pitem == NULL)
						{
						// If a child is desired . . .
						if (itChild != CItem3d::None)
							{
							// Have one.
							if (ConstructWithID(CItem3dID, m_pRealm, (CThing**)&pitem) == 0)
								{
								// Remember who our child is.
								m_idChildItem	= pitem->GetInstanceID();
								// Setup the child.
								pitem->Setup(0, 0, 0, itChild, NULL, m_u16InstanceId);
								}
							else
								{
								TRACE("EditModify(): ConstructWithID failed for CItem3d.\n");
								}
							}
						}
					}
			}
			else
			{
				// User Abort
				sResult = 1;
			}	
		}
	}
	delete pGui;

	return sResult;

}

////////////////////////////////////////////////////////////////////////////////
// Get all required resources
////////////////////////////////////////////////////////////////////////////////
short CBand::GetResources(void)						// Returns 0 if successfull, non-zero otherwise
{
	short sResult = 0;

	sResult = m_animRun.Get(ms_apszRunResNames, RChannel_LoopAtStart | RChannel_LoopAtEnd);
	if (sResult == 0)
	{
		sResult = m_animStand.Get(ms_apszStandResNames, RChannel_LoopAtStart | RChannel_LoopAtEnd);
		if (sResult == 0)
		{
			sResult = m_animMarch.Get(ms_apszMarchResNames, RChannel_LoopAtStart | RChannel_LoopAtEnd);
			if (sResult == 0)
			{
				sResult = m_animShot.Get(ms_apszShotResNames);
				if (sResult == 0)
				{
					sResult = m_animBlownup.Get(ms_apszBlownupResNames);
					if (sResult == 0)
					{
						sResult = m_animOnFire.Get(ms_apszOnFireResNames, RChannel_LoopAtStart | RChannel_LoopAtEnd);
						if (sResult == 0)
						{
							// Add more anim gets here if necessary
						}
						else
						{
							TRACE("CBand::GetResources - Failed to open 3D on fire animation\n");
						}
					}
					else
					{
						TRACE("CBand::GetResources - Failed to open 3D blownup animation\n");
					}
				}
				else
				{
					TRACE("CBand::GetResources - Failed to open 3D shot animation\n");
				}	
			}
			else
			{
				TRACE("CBand::GetResources - Failed to open 3D march animation\n");
			}
		}
		else
		{
			TRACE("CBand::GetResources - Failed to open 3D stand animation\n");
		}
	}
	else
	{
		TRACE("CBand::GetResources - Failed to open 3D run animation\n");
	}	

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Free all resources
////////////////////////////////////////////////////////////////////////////////
short CBand::FreeResources(void)						// Returns 0 if successfull, non-zero otherwise
{
	m_animRun.Release();
	m_animStand.Release();
	m_animMarch.Release();
	m_animShot.Release();
	m_animBlownup.Release();
	m_animOnFire.Release();
		
	return 0;
}


////////////////////////////////////////////////////////////////////////////////
// ProcessMessages - Similar to the base class version but handles a few more
////////////////////////////////////////////////////////////////////////////////

void CBand::ProcessMessages(void)
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

void CBand::OnShotMsg(Shot_Message* pMessage)
{
	if (m_state != State_BlownUp &&
	    m_state != State_Shot &&
		 m_state != State_Die &&
		 m_state != State_Dead)
	{
		CCharacter::OnShotMsg(pMessage);

		// Start shot animation if he hasn't already.
		m_lAnimTime = 0;
		m_panimCur = &m_animShot;
		switch (GetRand() % 8)
		{
			case 0:
				PlaySample(g_smidAmyMyEyes, SampleMaster::Voices);
				break;

			case 1:
				PlaySample(g_smidAndreaMyLeg, SampleMaster::Voices);
				break;

			case 2:
				PlaySample(g_smidBillGrunt, SampleMaster::Voices);
				break;

			case 3:
				PlaySample(g_smidMikeGrunt, SampleMaster::Voices);
				break;

			case 4:
				PlaySample(g_smidPaulCantFeelLegs, SampleMaster::Voices);
				break;

			case 5:
				PlaySample(g_smidRandyHuu, SampleMaster::Voices);
				break;

			case 6:
				PlaySample(g_smidRandyUg, SampleMaster::Voices);
				break;

			case 7:
				PlaySample(g_smidSteveUrl, SampleMaster::Voices);
				break;
		}
		m_state = State_Shot;
		// Dies in one shot
		m_stockpile.m_sHitPoints =0; 
		AlertBand();
	}
}

////////////////////////////////////////////////////////////////////////////////
// Explosion message
////////////////////////////////////////////////////////////////////////////////

void CBand::OnExplosionMsg(Explosion_Message* pMessage)
{
	if (m_state != State_BlownUp)
	{
		CCharacter::OnExplosionMsg(pMessage);

		// Drop item, if we have one still.
		CThing3d*	pthing3d	= DetachChild(
			&m_idChildItem,
			(RTransform*) m_panimCur->m_ptransRigid->GetAtTime(m_lAnimTime) );
		// If we got something back . . . 
		if (pthing3d != NULL)
			{
			// Let it know about the explosion.
			GameMessage msg;
			msg.msg_Explosion	= *pMessage;
			
			SendThingMessage(&msg, pthing3d);
			}

		// Explosion kills the guy
		m_stockpile.m_sHitPoints = 0;
		m_state = State_BlownUp;
		m_lAnimTime = 0;
		m_panimCur = &m_animBlownup;
		switch (GetRand() % 8)
		{
			case 0:
				PlaySample(g_smidBlownupFemaleYell, SampleMaster::Voices);
				break;

			case 1:
				PlaySample(g_smidCarynScream, SampleMaster::Voices);
				break;

			case 2:
				PlaySample(g_smidTinaScream1, SampleMaster::Voices);
				break;

			case 3:
				PlaySample(g_smidPaulAhah, SampleMaster::Voices);
				break;

			case 4:
				PlaySample(g_smidScottYell1, SampleMaster::Voices);
				break;

			case 5:
				PlaySample(g_smidScottYell2, SampleMaster::Voices);
				break;

			case 6:
				PlaySample(g_smidMikeOhh, SampleMaster::Voices);
				break;

			case 7:
				PlaySample(g_smidSteveAhBlowup, SampleMaster::Voices);
				break;
		}
		AlertBand();
		GameMessage msg;
		msg.msg_Explosion.eType = typeExplosion;
		msg.msg_Explosion.sPriority = 0;
		CThing* pDemon = m_pRealm->m_aclassHeads[CThing::CDemonID].GetNext();
		if (pDemon)
			SendThingMessage(&msg, pDemon);				
	}
}

////////////////////////////////////////////////////////////////////////////////
// Burning message
////////////////////////////////////////////////////////////////////////////////

void CBand::OnBurnMsg(Burn_Message* pMessage)
{
    UnlockAchievement(ACHIEVEMENT_FIREBOMB_THE_BAND);

	CCharacter::OnBurnMsg(pMessage);
	m_stockpile.m_sHitPoints -= pMessage->sDamage;

	if (m_state != State_Burning &&
	    m_state != State_BlownUp &&
		 m_state != State_Die &&
		 m_state != State_Dead)
	{
		m_state = State_Burning;
		m_panimCur = &m_animOnFire;
		m_lAnimTime = 0;
		switch (GetRand() % 8)
		{
			case 0:
				PlaySample(g_smidAmyScream, SampleMaster::Voices);
				break;

			case 1:
				PlaySample(g_smidTinaScream2, SampleMaster::Voices);
				break;

			case 2:
				PlaySample(g_smidTinaScream3, SampleMaster::Voices);
				break;

			case 3:
				PlaySample(g_smidMikeAhh, SampleMaster::Voices);
				break;

			case 4:
				PlaySample(g_smidSteveAhFire, SampleMaster::Voices);
				break;

			case 5:
				PlaySample(g_smidSteveWaFire, SampleMaster::Voices);
				break;

			case 6:
				PlaySample(g_smidAndreaHelp, SampleMaster::Voices);
				break;

			case 7:
				PlaySample(g_smidCarynScream, SampleMaster::Voices);
				break;
		}
		AlertBand();
		GameMessage msg;
		msg.msg_Burn.eType = typeBurn;
		msg.msg_Burn.sPriority = 0;
		CThing* pDemon = m_pRealm->m_aclassHeads[CThing::CDemonID].GetNext();
		if (pDemon)
			SendThingMessage(&msg, pDemon);				
	}
}

////////////////////////////////////////////////////////////////////////////////
// Panic message
////////////////////////////////////////////////////////////////////////////////

void CBand::OnPanicMsg(Panic_Message* pMessage)
{
	if (m_state != State_Die &&
	    m_state != State_Dead &&
		 m_state != State_BlownUp &&
		 m_state != State_Shot &&
		 m_state != State_Burning &&
		 m_state != State_Panic)
	{
		m_state = State_Panic;
		m_panimCur = &m_animOnFire;
		m_lAnimTime = GetRand() % m_panimCur->m_psops->TotalTime();
		// Pick a random bouy to run to
		m_ucDestBouyID = SelectRandomBouy();
		m_ucNextBouyID = m_pNavNet->FindNearestBouy(m_dX, m_dZ);
		m_pNextBouy = m_pNavNet->GetBouy(m_ucNextBouyID);
		if (m_pNextBouy)
		{
			m_sNextX = m_pNextBouy->GetX();
			m_sNextZ = m_pNextBouy->GetZ();
			AlignToBouy();
		}
		short sRandom = GetRand() % 6;
		switch (sRandom)
		{
			case 0:
				PlaySample(g_smidScottYell1, SampleMaster::Voices);
				break;

			case 1:
				PlaySample(g_smidBlownupFemaleYell, SampleMaster::Voices);
				break;

			case 2:
				PlaySample(g_smidTinaScream1, SampleMaster::Voices);
				break;

			default:
				break;
		}

	}
}

////////////////////////////////////////////////////////////////////////////////
// AlertBand
////////////////////////////////////////////////////////////////////////////////

void CBand::AlertBand(void)
{
	CThing* pThing;
	GameMessage msg;
	GameMessage msgStopSound;

	msgStopSound.msg_ObjectDelete.eType = typeObjectDelete;
	msgStopSound.msg_ObjectDelete.sPriority = 0;

	msg.msg_Panic.eType = typePanic;
	msg.msg_Panic.sPriority = 0;
	msg.msg_Panic.sX = (short) m_dX;
	msg.msg_Panic.sY = (short) m_dY;
	msg.msg_Panic.sZ = (short) m_dZ;

	CListNode<CThing>* pNext = m_pRealm->m_aclassHeads[CThing::CBandID].m_pnNext;
	while (pNext->m_powner != NULL)
	{
		pThing = pNext->m_powner;
		if (pThing != this)
			SendThingMessage(&msg, pThing);
		pNext = pNext->m_pnNext;
	}	
	if (ms_siBandSongInstance != 0)
	{
		AbortSample(ms_siBandSongInstance);
		ms_siBandSongInstance = 0;
		ms_bDonePlaying	= true;
	}
}


////////////////////////////////////////////////////////////////////////////////
// Implements basic one-time functionality for each time State_Dead is
// entered.
////////////////////////////////////////////////////////////////////////////////
void CBand::OnDead(void)
	{
	// Drop item.  This does nothing if we've already dropped it.
	DropItem();

	// Call base class.
	CDoofus::OnDead();
	}

////////////////////////////////////////////////////////////////////////////////
// Implements basic functionality while dying and returns true
// until the state is completed.
// (virtual -- Overriden here)
////////////////////////////////////////////////////////////////////////////////
bool CBand::WhileDying(void)	// Returns true until state is complete.
	{
	// Drop item.  This does nothing if we've already dropped it.
	DropItem();

	// Call base class.
	return CDoofus::WhileDying();
	}


////////////////////////////////////////////////////////////////////////////////
// Implements basic functionality while being shot and returns true
// until the state is completed.
// (virtual -- Overriden here)
////////////////////////////////////////////////////////////////////////////////
bool CBand::WhileShot(void)	// Returns true until state is complete.
	{
	// Drop item.  This does nothing if we've already dropped it.
	DropItem();

	// Call base class.
	return CDoofus::WhileShot();
	}

		
////////////////////////////////////////////////////////////////////////////////
// Drop item and apply appropriate forces.
////////////////////////////////////////////////////////////////////////////////
void CBand::DropItem(void)	// Returns nothing.
	{
	// If we still have the child item . . .
	if (m_idChildItem != CIdBank::IdNil)
		{
		// Drop it.
		CThing3d*	pthing3d	= DetachChild(
			&m_idChildItem,
			(RTransform*) m_panimCur->m_ptransRigid->GetAtTime(m_lAnimTime) );
		// If we got something back . . . 
		if (pthing3d != NULL)
			{
			// Send it spinning.
			pthing3d->m_dExtRotVelY	= GetRand() % 720;
			pthing3d->m_dExtRotVelZ	= GetRand() % 720;
			// Send it forward (from our perspective)
			// with our current velocity.
			pthing3d->m_dExtHorzRot	= m_dRot;
			pthing3d->m_dExtHorzVel	= m_dVel;
			// ... and air drag.
			if (pthing3d->m_dExtHorzVel > 0.0)
				{
				pthing3d->m_dExtHorzDrag = -ms_dDefaultAirDrag;
				}
			else if (pthing3d->m_dExtHorzVel < 0.0)
				{
				pthing3d->m_dExtHorzDrag = ms_dDefaultAirDrag;
				}

			// Similar enough to blown up.
			pthing3d->m_state			= State_BlownUp;
			}
		}
	}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
