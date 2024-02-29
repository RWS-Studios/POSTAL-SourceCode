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
// fire.cpp
// Project: Postal
//
// This module implements the CFire weapon class which is a burning flame
//	for several different effects and weapons.
// 
//
// History:
//		01/17/97 BRH	Started this weapon object.
//
//		01/23/97 BRH	Updated the time to GetGameTime rather than using
//							real time..
//
//		02/04/97	JMI	Changed LoadDib() call to Load() (which now supports
//							loading of DIBs).
//
//		02/06/97 BRH	Added RAnimSprite animation of the explosion for now.  
//							We are going to do an Alpha effect on the explosion, so
//							there are two animations, one of the image and one of
//							the Alpha information stored as a BMP8 animation.  When
//							the Alpha effect is ready, we will pass a frame from
//							each animation to a function to draw it.
//
//		02/06/97 BRH	Fixed problem with timer.  Since all Explosion objects
//							are using the same resource managed animation, they cannot
//							use the animation timer, they have to do the timing 
//							themselves.
//
//		02/07/97 BRH	Changed the sprite from CSprite2 to CSpriteAlpha2 for
//							the Alpha Blit effect.
//
//		02/09/97 BRH	Started the Fire from Explode file since they are
//							similar.
//
//		02/10/97	JMI	rspReleaseResource() now takes a ptr to a ptr.
//
//		02/11/97 BRH	Changed the fire to start on a random frame number
//							so if you have many fires, they don't pulsate or all
//							burn in sync with each other.
//
//		02/14/97 BRH	Changed from using the RAnimSprite to channel data.
//
//		02/17/97 BRH	Now uses the resource manager to get the assets and starts
//							at a random time interval so the fire will be random again.
//
//		02/17/97 BRH	Changed the lifetime to be time based rather than frame
//							based which was causing the fire to live on forever 
//							since being switched from RAnimSprite to RChannel1.
//
//		02/18/97 BRH	Now the fire changes to different Alpha channels as it
//							burns out during its time to live.  
//
//		02/19/97 BRH	Checks for collisions and sends messages.
//
//		02/19/97 BRH	Added the ability to run both small and large fire
//							animations.  Change the duration on the alpha layers
//							so that the initial alpha channel gets played for 80%
//							of the burning time.  Also added bThick parameter to startup
//							which will start using the 0th Alpha channel which is
//							more opaque.  If you want more Alpha, set to false which
//							will start on the next Alpha level down.
//
//		02/23/97 BRH	Added static Preload() funciton which will be called
//							before play begins to cache a resource for this object.
//
//		02/24/97	JMI	No longer sets the m_type member of the m_sprite b/c it
//							is set by m_sprite's constructor.
//
//		02/24/97 BRH	Set the default state in ProcessMessages
//
//		02/24/97 BRH	Added a timer for checkin collisions so it doesn't have
//							to check each time, but it was checking only when changing
//							alpha levels which was too long.
//
//		03/05/97	JMI	Render()'s mapping from 3D to 2D had a typo (was adding m_dY
//							instead of subtracting).  Now uses Map3Dto2D().
//
//		03/13/97	JMI	Load now takes a version number.
//
//		04/10/97 BRH	Updated this to work with the new multi layer attribute
//							maps.
//
//		04/14/97 BRH	Added CSmash::Item to the collide bits so that the fire
//							will send messages to barrels and other items.
//
//		04/21/97 BRH	Added Smoke animation to the fire and the ability of the
//							fire to change to smoke.
//
//		02/22/97 BRH	Adjusted the timer for the smoke effect to eliminate some
//							of the final frames so that the smoke wouldn't pulsate
//							like it did.				
//
//		04/23/97	JMI	Changed this item's m_smash bits from CSmash::Item to
//							CSmash::Fire.
//							Now affects Characters, Miscs, Mines, and Barrels.
//
//		04/24/97 BRH	Added static wind direction variable that will get
//							adjusted slightly by each new creation of smoke which
//							calls WindDirectionUpdate() to randomly vary the wind
//							direction.
//
//		04/25/97 BRH	Fixed problem with smoke that was created as smoke, 
//							setting people on fire.  Also fixed wall detection
//							and added an individual direction variable to each 
//							instance of smoke that initially copies the wind
//							direction and uses it until it hits a wall, then it
//							rotates in one direction or the other until it is
//							free to move again.
//
//		05/09/97	JMI	Update() now moves the smashatorium object when the CFire
//							is not Smoke.
//
//		05/29/97	JMI	Removed ASSERT on m_pRealm->m_pAttribMap which no longer
//							exists.
//
//		06/11/97 BRH	Pass along the m_u16ShooterID value in the Burn message.
//
//		06/15/97 BRH	Fixed Smoke going past animation by 1 frame.
//
//		06/16/97 BRH	Fixed smoke init of static wind direction.  Now it 
//							inits the wind direction on class load so that it doesn't
//							cause problems for the demo mode.
//
//		06/17/97 MJR	Same as previous one for wind velocity.
//
//					MJR	Moved resetting of statics to Preload(), since in most
//							cases, fire or smoke are not Load()'ed.
//
//		06/18/97 BRH	Changed over to using GetRandom()
//
//		06/26/97 BRH	Added CSmash::AlmostDead to the include bits for fire so
//							that writhing guys can be killed by fire.
//
//		07/01/97 BRH	Added small smoke animation.
//
//		07/04/97 BRH	Added an auto alpha blend on the small smoke for the
//							rocket trails so they can blend into alpha based on
//							their time to live.  May need to disable the
//							alpha channel for it to work correctly.
//
//		07/08/97	JMI	Fixed Render() to distribute the homogeneous alpha level
//							better.  Still needs tuning.
//
//		07/09/97	JMI	Now uses m_pRealm->Make2dResPath() to get the fullpath
//							for 2D image components.
//
//		07/09/97	JMI	Changed Preload() to take a pointer to the calling realm
//							as a parameter.
//
//		07/10/97	JMI	Now uses alpha mask and level for animation.
//
//		07/13/97 BRH	Changed the animations to use only 1 alpha mask and change
//							the alpha level based on time.
//
//		07/20/97	JMI	Added some ASSERTs.
//
//		07/23/97 BRH	Changed small fires to create small smokes rather than 
//							large which slows down the game quite a bit.
//
//		07/27/97	JMI	Changed to use Z position (i.e., X/Z plane) instead of
//							Y2 position (i.e., viewing plane) position for draw 
//							priority.
//
//		08/11/97 BRH	If alpha blending is turned off, as a performance option,
//							then don't even blit the smoke since without the alpha
//							effect, you can't see through it at all.
//
//		08/20/97	JMI	Now does a range check on m_sCurrentAlphaLevel after
//							decrementing.
//
//		09/02/97	JMI	Added m_u16FireStarterID.  This is used for a special case
//							when the starter of the fire is not the thing using the
//							fire as a weapon (e.g., when a guy catches fire he can
//							use the fire on other people by running into them causing
//							them to catch on fire; however, if his own fire kills him
//							it is to the creator of the fire's credit that he dies).
//
////////////////////////////////////////////////////////////////////////////////
#define FIRE_CPP

#include "RSPiX.h"
#include <math.h>

#include "fire.h"
#include "dude.h"
#include "game.h"
#include "reality.h"

////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////

#define AA_FILE				"fire.aan"
#define LARGE_FILE			"fire.aan"
#define SMALL_FILE			"smallfire.aan"
#define SMOKE_FILE			"smoke.aan"
#define SMALL_SMOKE_FILE	"tinysmoke.aan"

#define INIT_WIND_DIR		30
#define INIT_WIND_VEL		30

#define MAX_ALPHA				200	// Used for smoke trails

#define THICK_ALPHA			255	// Start alpha level for thick fire
#define THIN_ALPHA			200	// Start alpha level for thin fire
#define DIEDOWN_ALPHA		100	// Point at which it looks like its dying down
#define SMOLDER_ALPHA		30		// Point at which it is too weak to burn anyone

#define BRIGHT_PERCENT		0.80	// Amount of the time it should be more opaque

////////////////////////////////////////////////////////////////////////////////
// Variables/data
////////////////////////////////////////////////////////////////////////////////

// Let this auto-init to 0
short CFire::ms_sFileCount;
short CFire::ms_sLargeRadius = 20;
short CFire::ms_sSmallRadius = 8;
short CFire::ms_sWindDirection = INIT_WIND_DIR;		// Start wind in this direction
long  CFire::ms_lCollisionTime = 250;					// Check for collisions this often
long  CFire::ms_lSmokeTime = 10000;						// Time to let smoke run
double CFire::ms_dWindVelocity = INIT_WIND_VEL;		// Pixels per second drift due to wind


////////////////////////////////////////////////////////////////////////////////
// Load object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CFire::Load(										// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,											// In:  File to load from
	bool bEditMode,										// In:  True for edit mode, false otherwise
	short sFileCount,										// In:  File count (unique per file, never 0)
	ULONG	ulFileVersion)									// In:  Version of file format to load.
	{
	short sResult = CThing::Load(pFile, bEditMode, sFileCount, ulFileVersion);

	if (sResult == 0)
	{
		// Load common data just once per file (not with each object)
		if (ms_sFileCount != sFileCount)
		{
			ms_sFileCount = sFileCount;

			// Init the static wind direction and velocity when the class loads.
			ms_sWindDirection = INIT_WIND_DIR;
			ms_dWindVelocity = INIT_WIND_VEL;

			// Load static data.
			switch (ulFileVersion)
			{
				default:
				case 1:
					break;
			}
		}

		// Load instance data.
		switch (ulFileVersion)
		{
			default:
			case 1:
				pFile->Read(&m_eFireAnim);
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
			TRACE("CFire::Load(): Error reading from file!\n");
		}
	}
	else
	{
		TRACE("CFire::Load():  CThing::Load() failed.\n");
	}

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Save object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CFire::Save(										// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,											// In:  File to save to
	short sFileCount)										// In:  File count (unique per file, never 0)
{
	short	sResult	= CThing::Save(pFile, sFileCount);
	if (sResult == 0)
	{
		// Save common data just once per file (not with each object)
		if (ms_sFileCount != sFileCount)
		{
			ms_sFileCount = sFileCount;

			// Save static data
		}

		pFile->Write(&m_eFireAnim);
	}
	else
	{
		TRACE("CFire::Save(): CThing::Save() failed.\n");
	}

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Startup object
////////////////////////////////////////////////////////////////////////////////
short CFire::Startup(void)								// Returns 0 if successfull, non-zero otherwise
{
	return Init();
}

////////////////////////////////////////////////////////////////////////////////
// Shutdown object
////////////////////////////////////////////////////////////////////////////////
short CFire::Shutdown(void)							// Returns 0 if successfull, non-zero otherwise
{
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Suspend object
////////////////////////////////////////////////////////////////////////////////
void CFire::Suspend(void)
{
	m_sSuspend++;
}

////////////////////////////////////////////////////////////////////////////////
// Resume object
////////////////////////////////////////////////////////////////////////////////
void CFire::Resume(void)
{
	m_sSuspend--;

	// If we're actually going to start updating again, we need to reset
	// the time so as to ignore any time that passed while we were suspended.
	// This method is far from precise, but I'm hoping it's good enough.
	if (m_sSuspend == 0)
		m_lPrevTime = m_pRealm->m_time.GetGameTime();
}


////////////////////////////////////////////////////////////////////////////////
// Update object
////////////////////////////////////////////////////////////////////////////////
void CFire::Update(void)
{
	long lThisTime;
	double dSeconds;
	double dDistance;
	double dNewX;
	double dNewZ;

	if (!m_sSuspend)
	{
		// See if we killed ourselves
		if (ProcessMessages() == State_Deleted)
			return;

		if (m_lTimer < m_lBurnUntil)
		{
			lThisTime = m_pRealm->m_time.GetGameTime();
			m_lTimer += lThisTime - m_lPrevTime;
			// See if its time to change to the next alpha channel
			if (m_lTimer > m_lCurrentAlphaTimeout)
			{
				m_sCurrentAlphaLevel--;
				// Range check.
				if (m_sCurrentAlphaLevel < 0)
					m_sCurrentAlphaLevel	= 0;
				else if (m_sCurrentAlphaLevel > 255)
					m_sCurrentAlphaLevel	= 255;

				if (m_lTimer < m_lAlphaBreakPoint)
					m_lCurrentAlphaTimeout += m_lBrightAlphaInterval;
				else
					m_lCurrentAlphaTimeout += m_lDimAlphaInterval;
			}

			if (lThisTime > m_lCollisionTimer)
			{
				// If the fire is not smoldering out, then it has the ability
				// to set other things on fire and should check collisions
				// to see which things it should tell to burn.
				if (m_bSendMessages && m_sCurrentAlphaLevel > SMOLDER_ALPHA && m_eFireAnim != Smoke && m_eFireAnim != SmallSmoke)
				{
					CSmash* pSmashed = NULL;
					GameMessage msg;
					msg.msg_Burn.eType = typeBurn;
					msg.msg_Burn.sPriority = 0;
					msg.msg_Burn.sDamage = 10;
					msg.msg_Burn.u16ShooterID = m_u16ShooterID;
					m_pRealm->m_smashatorium.QuickCheckReset(&m_smash, m_u32CollideIncludeBits,
																		  m_u32CollideDontcareBits, 
																		  m_u32CollideExcludeBits);
					while (m_pRealm->m_smashatorium.QuickCheckNext(&pSmashed))
						{
						// Default to the standard case where credit is given to the
						// shooter.
						msg.msg_Burn.u16ShooterID	= m_u16ShooterID;

						if ((m_bIsBurningDude) && (pSmashed->m_pThing->GetClassID() != CDudeID))
							UnlockAchievement(ACHIEVEMENT_TOUCH_SOMEONE_WHILE_BURNING);

						// If the fire starter ID is set . . .
						if (m_u16FireStarterID != CIdBank::IdNil)
							{
							// If this is the shooter . . .
							if (pSmashed->m_pThing->GetInstanceID() == m_u16ShooterID)
								{
								// The shooter is damaged by his own fire with credit
								// given to the fire starter.
								msg.msg_Burn.u16ShooterID	= m_u16FireStarterID;
								}
							}

						// Burn.
						SendThingMessage(&msg, pSmashed->m_pThing);				
						}
				}
				// Reset collision timer for next time
				m_lCollisionTimer = lThisTime + ms_lCollisionTime;
			}

			// If this is smoke, make it drift in the wind direction
			if (m_eFireAnim == Smoke || m_eFireAnim == SmallSmoke)
			{
				// Update position using wind direction and velocity
				dSeconds = ((double) lThisTime - (double) m_lPrevTime) / 1000.0;
				// Apply internal velocity.
				dDistance	= ms_dWindVelocity * dSeconds;
				dNewX	= m_dX + COSQ[(short) m_sRot] * dDistance;
				dNewZ	= m_dZ - SINQ[(short) m_sRot] * dDistance;

				// Check attribute map for walls, and if you hit a wall, 
				// set the timer so you will die off next time around.
				short sHeight = m_pRealm->GetHeight((short) dNewX, (short) dNewZ);
				// If it hits a wall taller than itself, then it will rotate in the
				// predetermined direction until it is free to move.
				if ((short) m_dY < sHeight)
				{
					if (m_bTurnRight)
						m_sRot = rspMod360(m_sRot - 20);
					else
						m_sRot = rspMod360(m_sRot + 20);
				}
				else
				// else it is ok, so update its new position
				{
					m_dX = dNewX;
					m_dZ = dNewZ;
				}
			}
			else
			{
				// Update our smashatorium location.
				m_smash.m_sphere.sphere.X = m_dX;
				m_smash.m_sphere.sphere.Y = m_dY;
				m_smash.m_sphere.sphere.Z = m_dZ;
				// Update the smash.
				m_pRealm->m_smashatorium.Update(&m_smash);
			}

			m_lPrevTime = lThisTime;
		}
		else
		{
			// If its done smoking, then delete it
			if (m_eFireAnim == Smoke || m_eFireAnim == SmallSmoke)
			{
				delete this;
			}
			// Else change the fire to smoke
			else
			{
				if (Smokeout() != SUCCESS)
					delete this;
			}
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
// Render object
////////////////////////////////////////////////////////////////////////////////
void CFire::Render(void)
{
	
	CAlphaAnim* pAnim;

	if (m_eFireAnim == Smoke || m_eFireAnim == SmallSmoke)	
	{
		pAnim = (CAlphaAnim*) m_pAnimChannel->GetAtTime(m_lTimer); 
		// For a performance gain, don't blit the smoke at all if alpha
		// blending is turned off - its impossible to see through when
		// alpha blending is off anyway.
		if (g_GameSettings.m_sAlphaBlend)
			m_sprite.m_sInFlags = 0;
		else
			m_sprite.m_sInFlags = CSprite::InHidden;
	}
	else
	{
		pAnim = (CAlphaAnim*) m_pAnimChannel->GetAtTime(m_lTimer % m_pAnimChannel->TotalTime());
	}	

	if (pAnim)
	{
		// Map from 3d to 2d coords
		Map3Dto2D(m_dX, m_dY, m_dZ, &(m_sprite.m_sX2), &(m_sprite.m_sY2) );
		// Offset by animations 2D offsets.
		m_sprite.m_sX2	+= pAnim->m_sX;
		m_sprite.m_sY2	+= pAnim->m_sY;

		// Priority is based on our Z position.
		m_sprite.m_sPriority = m_dZ;

		// Layer should be based on info we get from attribute map.
		m_sprite.m_sLayer = CRealm::GetLayerViaAttrib(m_pRealm->GetLayer((short) m_dX, (short) m_dZ));

		// Copy the color info and the alpha channel to the Alpha Sprite
		m_sprite.m_pImage = &(pAnim->m_imColor);

		// If its the tiny smoke (for trails) 
		// Do the alpha based on the time to live.
		if (m_eFireAnim == SmallSmoke)
		{
			// Set the alpha level so it gets more translucent over time.
			m_sprite.m_sAlphaLevel = MAX_ALPHA - (MAX_ALPHA * (m_lTimer - m_lStartTime) ) / m_lTimeToLive ;
			
			// Do a range check.
			if (m_sprite.m_sAlphaLevel < 0)
				m_sprite.m_sAlphaLevel = 0;
			else if (m_sprite.m_sAlphaLevel > MAX_ALPHA)
				m_sprite.m_sAlphaLevel = MAX_ALPHA;
		}
		else
		{
			m_sprite.m_sAlphaLevel = m_sCurrentAlphaLevel;
		}

		// Now there is only one alpha mask
		m_sprite.m_pimAlpha = &(pAnim->m_pimAlphaArray[0]);

		ASSERT(m_sprite.m_sAlphaLevel <= 255);
		ASSERT(m_sprite.m_sAlphaLevel >= 0);

		// Update sprite in scene
		m_pRealm->m_scene.UpdateSprite(&m_sprite);
		
	}
}

////////////////////////////////////////////////////////////////////////////////
// Setup
////////////////////////////////////////////////////////////////////////////////

short CFire::Setup(									// Returns 0 if successfull, non-zero otherwise
	short sX,												// In:  New x coord
	short sY,												// In:  New y coord
	short sZ,												// In:  New z coord
	long lTimeToLive,										// In:  Number of milliseconds to burn, default 1sec
	bool  bThick,											// In:  Use thick fire (more opaque) default = true
	FireAnim eAnimType)									// In:  Animation type to use default = LargeFire
{
	short sResult = 0;
	
	// Use specified position
	m_dX = (double)sX;
	m_dY = (double)sY;
	m_dZ = (double)sZ;
	m_lPrevTime = m_pRealm->m_time.GetGameTime();
	m_lCollisionTimer = m_lPrevTime + ms_lCollisionTime;

	m_eFireAnim = eAnimType;
	m_lTimeToLive = lTimeToLive;
	if (bThick)
		m_sCurrentAlphaLevel = THICK_ALPHA;
	else
		m_sCurrentAlphaLevel = THIN_ALPHA;
	
	// Load resources
	sResult = GetResources();

	if (sResult == SUCCESS)
		sResult = Init();

	m_sRot = ms_sWindDirection;
	m_bTurnRight = (GetRandom() & 0x01);

	return sResult;

}

////////////////////////////////////////////////////////////////////////////////
// Init
////////////////////////////////////////////////////////////////////////////////

short CFire::Init(void)
{
	short sResult = SUCCESS;
	CAlphaAnim* pAnim = NULL;

	if (m_pAnimChannel != NULL)
	{
		m_lTimer = GetRandom() % m_pAnimChannel->TotalTime();
		m_lStartTime = m_lTimer;
		m_lBurnUntil = m_lTimer + m_lTimeToLive;
		m_lAlphaBreakPoint = m_lTimer + (m_lTimeToLive * BRIGHT_PERCENT);
		pAnim = (CAlphaAnim*) m_pAnimChannel->GetAtTime(0);
		ASSERT(pAnim != NULL);
		m_lBrightAlphaInterval = (m_lTimeToLive * BRIGHT_PERCENT) / MAX(1, m_sCurrentAlphaLevel - DIEDOWN_ALPHA);
		m_lDimAlphaInterval = (m_lTimeToLive * (100.0 - BRIGHT_PERCENT)) / MAX(1, DIEDOWN_ALPHA);
		m_lCurrentAlphaTimeout = m_lTimer + m_lBrightAlphaInterval;
		m_sTotalAlphaChannels = 1;
	}

	switch (m_eFireAnim)
	{
		case LargeFire:
			// Update sphere
			m_smash.m_sphere.sphere.X = m_dX;
			m_smash.m_sphere.sphere.Y = m_dY;
			m_smash.m_sphere.sphere.Z = m_dZ;
			m_smash.m_sphere.sphere.lRadius = ms_sLargeRadius;
			m_smash.m_bits = CSmash::Fire;
			m_smash.m_pThing = this;
			// Update the smash
			m_pRealm->m_smashatorium.Update(&m_smash);
			break;

		case SmallFire:
			// Update sphere
			m_smash.m_sphere.sphere.X = m_dX;
			m_smash.m_sphere.sphere.Y = m_dY;
			m_smash.m_sphere.sphere.Z = m_dZ;
			m_smash.m_sphere.sphere.lRadius = ms_sSmallRadius;
			m_smash.m_bits = CSmash::Fire;
			m_smash.m_pThing = this;
			// Update the smash
			m_pRealm->m_smashatorium.Update(&m_smash);
			break;

		case Smoke:
			m_smash.m_pThing = NULL;			
			m_bSendMessages = false;
			break;

		case SmallSmoke:
			m_smash.m_pThing = NULL;			
			m_bSendMessages = false;
			m_lStartTime = m_lTimer = GetRandom() % m_pAnimChannel->TotalTime() / 3;
			m_lBurnUntil = m_lTimer + m_lTimeToLive;
			break;			
	}

	// Set the collision bits
	m_u32CollideIncludeBits = CSmash::Character | CSmash::Barrel | CSmash::Mine | CSmash::Misc | CSmash::AlmostDead;
	m_u32CollideDontcareBits = CSmash::Good | CSmash::Bad;
	m_u32CollideExcludeBits = 0;

	return sResult;
}

////////////////////////////////////////////////////////////////////////////////
// Smokeout - Change from fire to smoke
////////////////////////////////////////////////////////////////////////////////

short CFire::Smokeout(void)
{
	short sResult = SUCCESS;

	// Modify the wind direction slightly
	WindDirectionUpdate();

	// Remove smash from the smashatorium if it was being used
	if (m_smash.m_pThing)
		m_pRealm->m_smashatorium.Remove(&m_smash);

	m_bSendMessages = false;

	// Release the fire animation and get the smoke animation
	FreeResources();
	if (m_eFireAnim == SmallFire)
		m_eFireAnim = SmallSmoke;
	else
		m_eFireAnim = Smoke;
	sResult = GetResources();

	// Reset timers
	CAlphaAnim* pAnim = NULL;

	if (m_pAnimChannel != NULL)
	{
		// Reset alpha level
		m_sCurrentAlphaLevel = THICK_ALPHA;
		pAnim = (CAlphaAnim*) m_pAnimChannel->GetAtTime(0);
		ASSERT(pAnim != NULL);
//		m_lTimeToLive = m_pAnimChannel->TotalTime();
		// use same time to live as the original
		m_lStartTime = m_lTimer = 0;
		m_lBurnUntil = m_lTimer + m_lTimeToLive;
		m_lAlphaBreakPoint = m_lTimer + (m_lTimeToLive * BRIGHT_PERCENT);
		m_lBrightAlphaInterval = (m_lTimeToLive * BRIGHT_PERCENT) / MAX(1, m_sCurrentAlphaLevel - DIEDOWN_ALPHA);
		m_lDimAlphaInterval = (m_lTimeToLive * (1.0 - BRIGHT_PERCENT)) / MAX(1, DIEDOWN_ALPHA);
		m_lCurrentAlphaTimeout = m_lTimer + m_lBrightAlphaInterval;
		m_sTotalAlphaChannels = 1;
	}

	return sResult;
}

////////////////////////////////////////////////////////////////////////////////
// Called by editor to init new object at specified position
////////////////////////////////////////////////////////////////////////////////
short CFire::EditNew(									// Returns 0 if successfull, non-zero otherwise
	short sX,												// In:  New x coord
	short sY,												// In:  New y coord
	short sZ)												// In:  New z coord
{
	short sResult = 0;
	
	// Use specified position
	m_dX = (double)sX;
	m_dY = (double)sY;
	m_dZ = (double)sZ;
	m_lTimer = GetRandom(); //m_pRealm->m_time.GetGameTime() + 1000;
	m_lPrevTime = m_pRealm->m_time.GetGameTime();

	// Load resources
	sResult = GetResources();

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to modify object
////////////////////////////////////////////////////////////////////////////////
short CFire::EditModify(void)
{
	return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to move object to specified position
////////////////////////////////////////////////////////////////////////////////
short CFire::EditMove(									// Returns 0 if successfull, non-zero otherwise
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
// Called by editor to update object
////////////////////////////////////////////////////////////////////////////////
void CFire::EditUpdate(void)
{
}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to render object
////////////////////////////////////////////////////////////////////////////////
void CFire::EditRender(void)
{
	// In some cases, object's might need to do a special-case render in edit
	// mode because Startup() isn't called.  In this case it doesn't matter, so
	// we can call the normal Render().
	Render();
}


////////////////////////////////////////////////////////////////////////////////
// Get all required resources
////////////////////////////////////////////////////////////////////////////////
short CFire::GetResources(void)			// Returns 0 if successfull, non-zero otherwise
{
	short sResult = SUCCESS;

	switch (m_eFireAnim)
	{
		case LargeFire:
			sResult = rspGetResource(&g_resmgrGame, m_pRealm->Make2dResPath(LARGE_FILE), &m_pAnimChannel, RFile::LittleEndian);
			break;

		case SmallFire:
			sResult = rspGetResource(&g_resmgrGame, m_pRealm->Make2dResPath(SMALL_FILE), &m_pAnimChannel, RFile::LittleEndian);
			break;

		case Smoke:
			sResult = rspGetResource(&g_resmgrGame, m_pRealm->Make2dResPath(SMOKE_FILE), &m_pAnimChannel, RFile::LittleEndian);
			break;

		case SmallSmoke:
			sResult = rspGetResource(&g_resmgrGame, m_pRealm->Make2dResPath(SMALL_SMOKE_FILE), &m_pAnimChannel, RFile::LittleEndian);
			break;
	}

	if (sResult != 0)
		TRACE("CFire::GetResources - Error getting fire animation resource\n");

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Free all resources
////////////////////////////////////////////////////////////////////////////////
short CFire::FreeResources(void)						// Returns 0 if successfull, non-zero otherwise
{
	short sResult = 0;

	rspReleaseResource(&g_resmgrGame, &m_pAnimChannel);

	return sResult;
}

////////////////////////////////////////////////////////////////////////////////
// Preload - basically trick the resource manager into caching resources for fire
//				 animations before play begins so that when a fire is set for
//				 the first time, there won't be a delay while it loads.
////////////////////////////////////////////////////////////////////////////////

short CFire::Preload(
	CRealm* prealm)				// In:  Calling realm.
{
	// Init the static wind direction and velocity when the class loads.
	ms_sWindDirection = INIT_WIND_DIR;
	ms_dWindVelocity = INIT_WIND_VEL;

	ChannelAA* pRes;
	short sResult = rspGetResource(&g_resmgrGame, prealm->Make2dResPath(LARGE_FILE), &pRes, RFile::LittleEndian);
	rspReleaseResource(&g_resmgrGame, &pRes);
	sResult |= rspGetResource(&g_resmgrGame, prealm->Make2dResPath(SMALL_FILE), &pRes, RFile::LittleEndian);
	rspReleaseResource(&g_resmgrGame, &pRes);
	sResult |= rspGetResource(&g_resmgrGame, prealm->Make2dResPath(SMOKE_FILE), &pRes, RFile::LittleEndian);
	rspReleaseResource(&g_resmgrGame, &pRes);
	sResult |= rspGetResource(&g_resmgrGame, prealm->Make2dResPath(SMALL_SMOKE_FILE), &pRes, RFile::LittleEndian);
	rspReleaseResource(&g_resmgrGame, &pRes);
	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// ProcessMessages
////////////////////////////////////////////////////////////////////////////////

CFire::CFireState CFire::ProcessMessages(void)
{
	CFireState eNewState = State_Idle;

	GameMessage msg;

	if (m_MessageQueue.DeQ(&msg) == true)
	{
		switch(msg.msg_Generic.eType)
		{
			case typeObjectDelete:
				m_MessageQueue.Empty();
				delete this;
				return CFire::State_Deleted;
				break;
		}
	}
	// Dump the rest of the messages
	m_MessageQueue.Empty();

	return eNewState;
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
