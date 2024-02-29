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
// rocket.cpp
// Project: Postal
//
// This module implements the CRocket weapon class which is an unguided
//	rocket missile.
// 
//
// History:
//		01/17/97 BRH	Started this weapon object.
//
//		02/04/97	JMI	Changed LoadDib() call to Load() (which now supports
//							loading of DIBs).
//
//		02/06/97 BRH	Moved Explosion position down some.
//
//		02/11/97 BRH	Changed the rocket to use the game res manager rather
//							than loading its assets from a file directly.
//
//		02/16/97 BRH	Rocket now sends a message to the CDude when it hits him.
//
//		02/18/97	JMI	Now uses typeExplosion instead of msg_Explosion.
//
//		02/19/97 BRH	Added ProcessMessage routine to look for ObjectDelete
//							messages.
//
//		02/23/97 BRH	Set the transform for the rocket so it faces the right
//							direction.  Also changed the coordinate system to x,-z
//
//		02/23/97 BRH	Added Preload() function to cache resources for this
//							object before play begins.
//
//		02/23/97 BRH	Added State_Hide so that the rocket can be created but
//							is not shown.  
//
//		02/24/97	JMI	No longer sets the m_type member of the m_sprite b/c it
//							is set by m_sprite's constructor.
//
//		03/03/97 BRH	Derived this from the CWeapon base class
//
//		03/06/97	JMI	Upgraded to current rspMod360 usage.
//
//		03/13/97	JMI	Load now takes a version number.
//
//		03/19/97 BRH	Changed the ProcessMessages function to return void so
//							it matches the new virtual function in the CWeapon
//							base class.
//
//		04/10/97 BRH	Updated this to work with the new multi layer attribute
//							maps.
//
//		04/15/97 BRH	Added CSmash::Item to the collision items so that rockets
//							can blow up barrels and other items.
//
//		04/15/97 BRH	Took out old State_Find code that was used to seek the
//							CDude.  The rocket is now aimed by the shooter and
//							this old code was using the CDude list which will soon
//							be changed and since it is no longer needed, it was
//							best to take it out.
//
//		04/23/97	JMI	Now collides with Characters, Miscs, and Barrels.
//							Also, sets its m_smash's bits to Projectile instead of
//							Character.
//							Also, changed layer priority to simply use Z position.
//
//		04/24/97	JMI	Now when it hits something, it does not update its 
//							position (i.e., it keeps its old valid position).
//
//		04/24/97 BRH	Added puffs of smoke in addition to the explosion.
//
//		04/29/97 BRH	Added an off screen distance at which the rocket will self
//							destruct.  
//
//		05/26/97 BRH	Changed the check for wall collisions to ignore the
//							NOT_WALKABLE attribute which caused it to blow up in
//							the wrong places.  Now only the height is checked.
//
//		05/29/97	JMI	Removed ASSERT on m_pRealm->m_pAttribMap which no longer
//							exists.
//
//		06/10/97 BRH	Increased the rocket arming time from 200ms to 500ms to 
//							avoid killing yourself when you are moving & shooting.
//
//		06/11/97 BRH	Passes shooter ID to the explosion that is created.
//
//		06/12/97 BRH	Added shooter ID to the call to Setup for the explosion.
//
//		06/18/97 BRH	Changed over to using GetRandom()
//
//		06/25/97 BRH	Added use of base class 2D shadow on the ground, but loaded
//							a smaller shadow resource.
//
//		06/30/97 BRH	Added cache samples to the Preload function.
//
//		06/30/97	JMI	Now uses CRealm's new GetRealmWidth() and *Height()
//							for dimensions of realm's X/Z plane.
//
//		06/30/97	JMI	Now uses IsPathClear() to determine if the path to the new
//							position is clear.
//
//		07/01/97 BRH	Added smoke trails.
//
//		07/01/97	JMI	In Update(), when the weapon explodes, it didn't set dNewX
//							and dNewZ but still created smoke at this unitialized
//							position.  Fixed.
//
//		07/07/97	JMI	In ProcessMessages(), it was deleting this.  Then, once
//							ProcessMessages() returned to Update(), it would check 
//							m_eState to see if it had been deleted and then return.
//							The problem is that once deleted you cannot access m_eState.
//							Changed it so ProcessMessages() does not delete this but
//							merely sets the state to delete so that Update() can do it.
//
//		07/08/97 BRH	Adjusted the position of the smoke, and cut down the trail
//							length.
//
//		07/09/97	JMI	Now uses m_pRealm->Make2dResPath() to get the fullpath
//							for 2D image components.
//
//		07/09/97	JMI	Changed Preload() to take a pointer to the calling realm
//							as a parameter.
//
//		07/16/97 BRH	Retuned, or untuned the hotspot for the smoke trails.  
//							Now that the hotspot for the smoke is correct, the
//							smoke does not need to be adjusted here.
//
//		07/18/97	JMI	Added m_siThrust to track our thrust play instance so we
//							can loop it and then terminate the looping when we explode.
//							The sound tapers way too much to be loopable now.  
//							Hopefully, we can get a better one.
//
//		07/18/97	JMI	Got rid of bogus immitation PlaySample functions.
//							Now there is one PlaySample() function.  Also, you now
//							MUST specify a category and you don't have to specify a
//							SoundInstance ptr to specify a volume.
//
//		08/08/97 BRH	Changed the arming so that the missle won't arm until
//							it stops colliding with the shooter's smash.  Also, added
//							a special case so that missiles shot by Sentry guns won't
//							blow up other Sentry guns.
//
//		08/12/97 BRH	Changed collision bits to exclude any object that is
//							ducking (which should only be the main dude when he
//							is ducking down).
//
//		08/15/97 BRH	Made the smash radius larger.
//
//		08/17/97	JMI	Changed m_pthingParent to m_idParent.
//
//		08/24/97 BRH	Now when the rocket goes into the expoode state, it sets
//							its position back to the previous position so that the
//							explosion won't always be created behind the thing it hit
//							and alwyas blow it forward which looked kind of weird.
//
//		08/26/97 BRH	Fixed bug with brackets where rocket always went back to
//							its previous position when fired by a sentry gun.
//
//		08/27/97	JMI	No longer sets the smash radius to m_sCurRadius during 
//							Render().
//
////////////////////////////////////////////////////////////////////////////////
#define ROCKET_CPP

#include "RSPiX.h"
#include <math.h>

#include "rocket.h"
#include "dude.h"
#include "explode.h"
#include "fire.h"
#include "SampleMaster.h"


////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////

#define SMALL_SHADOW_FILE	"smallshadow.img"

////////////////////////////////////////////////////////////////////////////////
// Variables/data
////////////////////////////////////////////////////////////////////////////////

// These are default values -- actually values are set using the editor!
double CRocket::ms_dAccUser     = 250.0;				// Acceleration due to user
double CRocket::ms_dMaxVelFore  = 250.0;				// Maximum forward velocity
double CRocket::ms_dMaxVelBack  = -250.0;				// Maximum backward velocity
double CRocket::ms_dCloseDistance = 30.0;			// Close enough to hit CDude
long CRocket::ms_lArmingTime = 500;					// Time before weapon arms.
short CRocket::ms_sOffScreenDist = 200;				// Go off screen this far before blowing up
long CRocket::ms_lSmokeTrailInterval = 10;			// Time to emit smoke trail.
long CRocket::ms_lSmokeTimeToLive = 1000;				// Time for smoke to stick around.


// Let this auto-init to 0
short CRocket::ms_sFileCount;

/// Rocket Animation Files
static char* ms_apszResNames[] = 
{
	"3d/missile.sop",
	"3d/missile.mesh",
	"3d/missile.tex",
	"3d/missile.hot",
	"3d/missile.bounds",
	"3d/missile.floor",
	NULL,
	NULL
};

////////////////////////////////////////////////////////////////////////////////
// Load object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CRocket::Load(										// Returns 0 if successfull, non-zero otherwise
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
					pFile->Read(&ms_dAccUser);
					pFile->Read(&ms_dMaxVelFore);
					pFile->Read(&ms_dMaxVelBack);
					pFile->Read(&ms_dCloseDistance);
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
		
		// Make sure there were no file errors
		if (!pFile->Error())
		{
			// Get resources
			sResult = GetResources();
		}
		else
		{
			sResult = -1;
			TRACE("CRocket::Load(): Error reading from file!\n");
		}
	}

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Save object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CRocket::Save(										// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,											// In:  File to save to
	short sFileCount)										// In:  File count (unique per file, never 0)
{
	// In most cases, the base class Save() should be called.  In this case it
	// isn't because the base class doesn't have a Save()!

	// Save common data just once per file (not with each object)
	if (ms_sFileCount != sFileCount)
	{
		ms_sFileCount = sFileCount;

		// Save static data
		pFile->Write(&ms_dAccUser);
		pFile->Write(&ms_dMaxVelFore);
		pFile->Write(&ms_dMaxVelBack);
		pFile->Write(&ms_dCloseDistance);
	}

	// Save object data

	return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Update object
////////////////////////////////////////////////////////////////////////////////
void CRocket::Update(void)
{
	USHORT usAttrib;
	short sHeight;
	double dNewX;
	double dNewZ;
	double dPrevX = 0; // compiler warning "not initialized before being used"
	double dPrevZ = 0; // compiler warning "not initialized before being used"

	if (!m_sSuspend)
		{
		// Get new time
		long lThisTime = m_pRealm->m_time.GetGameTime(); 

		// Calculate elapsed time in seconds
		double dSeconds = (double)(lThisTime - m_lPrevTime) / 1000.0;

		ProcessMessages();
		// If we're to be deleted . . .
		// Note that this could be a case in the switch below, but, if
		// for whatever reason, that moves or something else is inserted
		// between here and the switch that might change the state, we 
		// might not get deleted.
		if (m_eState == State_Deleted)
			{
			// We are to be deleted.  Do it.
			delete this;
			// Must get out of here before we touch any of our invalidated 
			// this.
			return;
			}

		// Check the current state
		switch (m_eState)
		{
			case CWeapon::State_Hide:
			case CWeapon::State_Idle:
				dPrevX = m_dX;
				dPrevZ = m_dZ;
				break;

			case CWeapon::State_Fire:
				PlaySample(										// Returns nothing.
																	// Does not fail.
					g_smidRocketFire,							// In:  Identifier of sample you want played.
					SampleMaster::Weapon,					// In:  Sound Volume Category for user adjustment
					DistanceToVolume(m_dX, m_dY, m_dZ, LaunchSndHalfLife),	// In:  Initial Sound Volume (0 - 255)
					&m_siThrust,								// Out: Handle for adjusting sound volume
					NULL,											// Out: Sample duration in ms, if not NULL.
					2841,											// In:  Where to loop back to in milliseconds.
																	//	-1 indicates no looping (unless m_sLoop is
																	// explicitly set).
					3090,											// In:  Where to loop back from in milliseconds.
																	// In:  If less than 1, the end + lLoopEndTime is used.
					false);										// In:  Call ReleaseAndPurge rather than Release after playing
// Old Call:	PlaySample(g_smidRocketFire);
				m_lTimer = lThisTime + ms_lArmingTime;
				m_eState = State_Chase;
				break;

//-----------------------------------------------------------------------
// Chase
//-----------------------------------------------------------------------
			case CWeapon::State_Chase:

				// Accelerate toward the target and check for proximity
				// and obstacles

				// Accelerate doofus up to max velocity
				m_dHorizVel += ms_dAccUser * dSeconds;

				// Limit to maximum velocity
				if (m_dHorizVel > ms_dMaxVelFore)
					m_dHorizVel = ms_dMaxVelFore;
				else if (m_dHorizVel < ms_dMaxVelBack)
					m_dHorizVel = ms_dMaxVelBack;

				// Adjust position based on velocity (this will clearly be optimized later on!)
				dNewX = m_dX + COSQ[(short)m_dRot] * (m_dHorizVel * dSeconds);
				dNewZ = m_dZ - SINQ[(short)m_dRot] * (m_dHorizVel * dSeconds);

				// Check for obstacles
				sHeight = m_pRealm->GetHeight((short) dNewX, (short) dNewZ);
				usAttrib = m_pRealm->GetFloorAttribute((short) dNewX, (short) dNewZ);

				// If the new position's height is too high, the new position is a ways
				// off screen, or the path to the new position is not clear of terrain . . .
				if (sHeight > m_dY                     || 
					 m_dZ > ms_sOffScreenDist + m_pRealm->GetRealmHeight() ||
					 m_dZ < -ms_sOffScreenDist ||
					 m_dX > ms_sOffScreenDist + m_pRealm->GetRealmWidth() ||
					 m_dX < -ms_sOffScreenDist ||
					 !m_pRealm->IsPathClear(	// Returns true, if the entire path is clear.                 
														// Returns false, if only a portion of the path is clear.     
														// (see *psX, *psY, *psZ).                                    
						(short) m_dX, 				// In:  Starting X.                                           
						(short) m_dY, 				// In:  Starting Y.                                           
						(short) m_dZ, 				// In:  Starting Z.                                           
						3.0, 							// In:  Rate at which to scan ('crawl') path in pixels per    
														// iteration.                                                 
														// NOTE: Values less than 1.0 are inefficient.                
														// NOTE: We scan terrain using GetHeight()                    
														// at only one pixel.                                         
														// NOTE: We could change this to a speed in pixels per second 
														// where we'd assume a certain frame rate.                    
						(short) dNewX, 			// In:  Destination X.                                        
						(short) dNewZ,				// In:  Destination Z.                                        
						0,								// In:  Max traverser can step up.                      
						NULL,							// Out: If not NULL, last clear point on path.                
						NULL,							// Out: If not NULL, last clear point on path.                
						NULL,							// Out: If not NULL, last clear point on path.                
						false) )						// In:  If true, will consider the edge of the realm a path
														// inhibitor.  If false, reaching the edge of the realm    
														// indicates a clear path.                                 
				{
					// Blow Up
					m_eState = CWeapon::State_Explode;
					// Note that these need to be set even if we explode; otherwise, they
					// never get initialized and totally hosened values are sent to
					// pSmoke-Setup() which makes Alpha unhappy.
					dPrevX = m_dX;
					dPrevZ = m_dZ;
				}
				else
				{
					dPrevX = m_dX;
					dPrevZ = m_dZ;
					m_dX = dNewX;
					m_dZ = dNewZ;
				}

				// Check for collisions with other characters if
				// the weapon is armed, else see if it is time to arm
				// the weapon yet.
				if (m_bArmed)
				{
					CSmash* pSmashed = NULL;
					m_pRealm->m_smashatorium.QuickCheckReset(
						&m_smash, 
						m_u32CollideIncludeBits,
						m_u32CollideDontcareBits,
						m_u32CollideExcludeBits & ~CSmash::Ducking);

					while (m_pRealm->m_smashatorium.QuickCheckNext(&pSmashed))
					{
						ASSERT(pSmashed->m_pThing);

						const bool bIsPlayer = (pSmashed->m_pThing->GetClassID() == CDudeID);

						// we need to check ducking collisions unconditionally so we can unlock an achievement, but then we carry on if it should have missed.
						if ((m_u32CollideExcludeBits & CSmash::Ducking) && (pSmashed->m_bits & CSmash::Ducking))
							{
							if (bIsPlayer)
								UnlockAchievement(ACHIEVEMENT_DUCK_UNDER_ROCKET);
							continue;  // keep going.
							}

						if (bIsPlayer)
							UnlockAchievement(ACHIEVEMENT_ROCKET_TO_THE_FACE);

						CThing* pShooter;
						m_pRealm->m_idbank.GetThingByID(&pShooter, m_u16ShooterID);
						if (pShooter)
						{
							// If a Sentry gun shot this weapon, and it hit another Sentry gun, then 
							// ignore the collision.
							if (!(pSmashed->m_pThing->GetClassID() == CSentryID && pShooter->GetClassID() == CSentryID))
							{
								m_eState = CWeapon::State_Explode;
								// Move back to previous position where expolosion should appear
								m_dX = dPrevX;
								m_dZ = dPrevZ;
							}
						}
						// Can't determine the shooter,but we did collide, so blow up.
						else
						{
							m_eState = CWeapon::State_Explode;
							// Move back to the previous position before doing the explosion so
							// that the explosion doesn't always go off behind the thing it
							// hits and blow it forward.
							m_dX = dPrevX;
							m_dZ = dPrevZ;
						}
					}
				}
				else
				{
					// Check for collision with self and if no collision, then arm
					CThing* pShooter = NULL;
					m_pRealm->m_idbank.GetThingByID(&pShooter, m_u16ShooterID);
					// If the shooter is valid, then arm when it clears the shooter
					if (pShooter)
					{
						CSmash* pSmashed = pShooter->GetSmash();
						if (pSmashed)
						{
							pSmashed = (CSmash*) &(((CThing3d*) pShooter)->m_smash);
							if (!(m_pRealm->m_smashatorium.QuickCheck(&m_smash, pSmashed)))
								m_bArmed = true;
						}
						else
						{
							if (lThisTime > m_lTimer)
								m_bArmed = true;
						}
					}
					// else do it the old fashioned way, so at least it will arm
					else
					{
						if (lThisTime > m_lTimer)
							m_bArmed = true;
					}
				}

				// See if its time to create a new puff of smoke
				if (lThisTime > m_lSmokeTimer)
				{
					m_lSmokeTimer = lThisTime + ms_lSmokeTrailInterval;
					CFire* pSmoke = NULL;
					if (CThing::Construct(CThing::CFireID, m_pRealm, (CThing**) &pSmoke) == 0)
					{
						// This needs to be fixed by calculating the position of the back end of
						// the rocket in 3D based on the rotation.  
						pSmoke->Setup(dPrevX, m_dY, dPrevZ, ms_lSmokeTimeToLive, true, CFire::SmallSmoke);
						pSmoke->m_u16ShooterID = m_u16ShooterID;
					}
				}

				// Update sound position.
				SetInstanceVolume(m_siThrust, DistanceToVolume(m_dX, m_dY, m_dZ, LaunchSndHalfLife) );
				break;

//-----------------------------------------------------------------------
// RemoteControl 
//-----------------------------------------------------------------------

			case CWeapon::State_RemoteControl:

				m_bArmed = true;
				{
					CSmash* pSmashed = NULL;
					if (m_pRealm->m_smashatorium.QuickCheck(&m_smash, 
																	m_u32CollideIncludeBits, 
																	m_u32CollideDontcareBits,
																	m_u32CollideExcludeBits, &pSmashed))
						m_eState = CWeapon::State_Explode;
				}

				// See if its time to create a new puff of smoke
				if (lThisTime > m_lSmokeTimer)
				{
					m_lSmokeTimer = lThisTime + ms_lSmokeTrailInterval;
					CFire* pSmoke = NULL;
					if (CThing::Construct(CThing::CFireID, m_pRealm, (CThing**) &pSmoke) == 0)
					{
						// This needs to be fixed by calculating the position of the back end of
						// the rocket in 3D based on the rotation.  
						pSmoke->Setup(dPrevX, m_dY, dPrevZ, ms_lSmokeTimeToLive, true, CFire::SmallSmoke);
						pSmoke->m_u16ShooterID = m_u16ShooterID;
					}
				}

				dPrevX = m_dX;
				dPrevZ = m_dZ;

				// Update sound position.
				SetInstanceVolume(m_siThrust, DistanceToVolume(m_dX, m_dY, m_dZ, LaunchSndHalfLife) );
				break;

//-----------------------------------------------------------------------
// Explode
//-----------------------------------------------------------------------
			case CWeapon::State_Explode:


				// Start an explosion object and then kill rocket
				// object
				CExplode* pExplosion;
				if (CThing::Construct(CThing::CExplodeID, m_pRealm, (CThing**) &pExplosion) == 0)
				{
					pExplosion->Setup(m_dX, MAX(m_dY-30, 0.0), m_dZ, m_u16ShooterID);
					PlaySample(										// Returns nothing.
																		// Does not fail.
						g_smidRocketExplode,						// In:  Identifier of sample you want played.
						SampleMaster::Destruction,				// In:  Sound Volume Category for user adjustment
						DistanceToVolume(m_dX, m_dY, m_dZ, ExplosionSndHalfLife) );	// In:  Initial Sound Volume (0 - 255)
// Old call:	PlaySample(g_smidRocketExplode);
				}

				short a;
				CFire* pSmoke;
				for (a = 0; a < 8; a++)
				{
					if (CThing::Construct(CThing::CFireID, m_pRealm, (CThing**) &pSmoke) == 0)
					{
						pSmoke->Setup(m_dX - 4 + GetRandom() % 9, m_dY-20, m_dZ - 4 + GetRandom() % 9, 4000, true, CFire::Smoke);
						pSmoke->m_u16ShooterID = m_u16ShooterID;
					}
				}

				delete this;
				return;
				break;
		}

		// Update sphere.
		m_smash.m_sphere.sphere.X			= m_dX;
		m_smash.m_sphere.sphere.Y			= m_dY;
		m_smash.m_sphere.sphere.Z			= m_dZ;
		m_smash.m_sphere.sphere.lRadius	= 2 * m_sprite.m_sRadius;

		// Update the smash.
		m_pRealm->m_smashatorium.Update(&m_smash);

		// Save time for next time
		m_lPrevTime = lThisTime;
	}
}


////////////////////////////////////////////////////////////////////////////////
// Render object
////////////////////////////////////////////////////////////////////////////////
void CRocket::Render(void)
{
	long lThisTime = m_pRealm->m_time.GetGameTime();

	m_sprite.m_pmesh = (RMesh*) m_anim.m_pmeshes->GetAtTime(lThisTime);
	m_sprite.m_psop = (RSop*) m_anim.m_psops->GetAtTime(lThisTime);
	m_sprite.m_ptex = (RTexture*) m_anim.m_ptextures->GetAtTime(lThisTime);
	m_sprite.m_psphere = (RP3d*) m_anim.m_pbounds->GetAtTime(lThisTime);

	// Reset rotation so it is not cumulative
	m_trans.Make1();

	// Set its pointing direction
	m_trans.Ry(rspMod360(m_dRot));

	// Eventually this should be channel driven also
//	m_sprite.m_sRadius = m_sCurRadius;

	if (m_eState == State_Hide)
		m_sprite.m_sInFlags = CSprite::InHidden;
	else
		m_sprite.m_sInFlags = 0;

	// If we're not a child of someone else...
	if (m_idParent == CIdBank::IdNil)
	{
		// Map from 3d to 2d coords
		Map3Dto2D((short) m_dX, (short) m_dY, (short) m_dZ, &m_sprite.m_sX2, &m_sprite.m_sY2);

		// Priority is based on Z.
		m_sprite.m_sPriority = m_dZ;

		// Layer should be based on info we get from the attribute map
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
// Setup
////////////////////////////////////////////////////////////////////////////////

short CRocket::Setup(									// Returns 0 if successfull, non-zero otherwise
	short sX,												// In:  New x coord
	short sY,												// In:  New y coord
	short sZ)												// In:  New z coord
{
	short sResult = 0;
	
	// Use specified position
	m_dX = (double)sX;
	m_dY = (double)sY;
	m_dZ = (double)sZ;
	m_dHorizVel = 0.0;

	// Load resources
	sResult = GetResources();

	// Enable the 2D shadow sprite
	PrepareShadow();

	m_bArmed = false;

	// Set the collision bits
	m_u32CollideIncludeBits = CSmash::Character | CSmash::Misc | CSmash::Barrel;
	m_u32CollideDontcareBits = CSmash::Good | CSmash::Bad;
	m_u32CollideExcludeBits = CSmash::Ducking;

	m_smash.m_bits = CSmash::Projectile;
	m_smash.m_pThing = this;

	m_sCurRadius = 10 * m_pRealm->m_scene.m_dScale3d;

	m_lSmokeTimer = 0;

	return sResult;
}

////////////////////////////////////////////////////////////////////////////////
// Get all required resources
////////////////////////////////////////////////////////////////////////////////
short CRocket::GetResources(void)						// Returns 0 if successfull, non-zero otherwise
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
		TRACE("CRocket::GetResources - Failed to open 3D animation for rocket\n");
	}

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Free all resources
////////////////////////////////////////////////////////////////////////////////
short CRocket::FreeResources(void)						// Returns 0 if successfull, non-zero otherwise
{
	m_anim.Release();

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Preload - basically trick the resource manager into caching resources 
//				 for this object so there won't be a delay the first time it is
//				 created.
////////////////////////////////////////////////////////////////////////////////

short CRocket::Preload(
	CRealm* prealm)				// In:  Calling realm.
{
	CAnim3D anim;	
	RImage* pimage;
	short sResult = anim.Get(ms_apszResNames);
	anim.Release();
	rspGetResource(&g_resmgrGame, prealm->Make2dResPath(SMALL_SHADOW_FILE), &pimage, RFile::LittleEndian);
	rspReleaseResource(&g_resmgrGame, &pimage);
	CacheSample(g_smidRocketFire);
	CacheSample(g_smidRocketExplode);
	return sResult;	
}

////////////////////////////////////////////////////////////////////////////////
// ProcessMessages
////////////////////////////////////////////////////////////////////////////////

void CRocket::ProcessMessages(void)
{
	GameMessage msg;

	if (m_MessageQueue.DeQ(&msg) == true)
	{
		switch(msg.msg_Generic.eType)
		{
			case typeObjectDelete:
				m_MessageQueue.Empty();
				m_eState = State_Deleted;
				// Don't delete this here.  Instead make sure the state is
				// set so Update() knows to delete us and return immediately
				// (before something else changes our state).
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
