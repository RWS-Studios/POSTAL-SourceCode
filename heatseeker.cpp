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
// heatseeker.cpp
// Project: Postal
//
// This module implements the CHeatseeker weapon class which is a heat seeking
// guided missile which will hit people or fire.
//
// History:
//		05/13/97 BRH	Started this weapon object from the CHeatseeker code
//
//		05/14/97 BRH	Added the IsPathClear() check to help avoid walls.
//
//		05/15/97 BRH	Changed the graphic for the missile.  Added different
//							masks for seek and collide.  Changed the offscreen
//							behavior so the missile will turn around when it has gone
//							off of the edge of the world.
//
//		05/15/97 BRH	Changed seeking to QuickCheckClosest so that it picks
//							the closest target rather than the first one in the
//							list of collisions.  
//
//		05/15/97	JMI	In ProcessMessages(), on a delete message, the CHeatseeker
//							was deleting itself.  Then, after ProcessMessages()
//							returned, it was checking m_eState, and, if set to 
//							State_Delete, was returning to avoid possible problems
//							referencing a deleted this.  The problem is that you cannot
//							reference m_eState after the object is deleted b/c that
//							would require utilizing the this.  Also, on Alpha (and 
//							Intel (SHMALLOC) too) the memory is altered to a certain
//							value to try to catch such problems.  This caused m_eState
//							to NOT be State_Delete, which resulted in writes to
//							freed memory as well.
//							To fix this problem, I changed ProcessMessages() to merely
//							set the state to State_Delete and utilized the switch in
//							Update() to delete this and return.
//
//		05/26/97 BRH	Changed check for obstacles to only check for height.  It
//							had been checking for NOT_WALKABLE which caused it to 
//							blow up in the wrong places.
//
//		05/29/97	JMI	Removed ASSERT on m_pRealm->m_pAttribMap which no longer
//							exists.
//
//		06/10/97 BRH	Increased the rocket arming time from 200ms to 500ms to 
//							avoid killing yourself when you are moving & shooting.
//
//		06/11/97 BRH	Added shooter ID passing to the explosion that is created.
//
//		06/12/97 BRH	Added shooter ID to the call to Setup for the explosion.
//
//		06/18/97 BRH	Changed over to using GetRandom()
//
//		06/25/97 BRH	Added use of base class 2D shadow on the ground, but loaded
//							a smaller shadow resource.
//
//		06/29/97 BRH	Added an IsPathClear check during flight to make sure it
//							doesn't go through thin fences.
//
//		06/30/97 BRH	Added CacheSample for the sound effects to the Preload.
//
//		06/30/97	JMI	Now uses CRealm's new GetRealmWidth() and *Height()
//							for dimensions of realm's X/Z plane.
//
//		06/30/97	JMI	Have to check in now, now.
//
//		06/30/97	JMI	Now uses IsPathClear()'s bCheckExtents = false to avoid
//							having the edge of the realm count as a path obstacle.
//
//		07/01/97 BRH	Added smoke trails to the heatseeker.
//
//		07/01/97	JMI	In Update(), when the weapon explodes, it didn't set dNewX
//							and dNewZ but still created smoke at this unitialized
//							position.  Fixed.
//
//		07/04/97 BRH	Cut down the trail time to live to make the trails shorter.
//
//		07/08/97 BRH	Adjusted the position of the smoke trail.
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
//		08/14/97 BRH	Added SetCollideBits function so that the the collision
//							bits can be set differently for the Dude and Doofus.  
//							The bits for the collision will bet set to the 
//							default values for the missile, but then can be changed
//							by calling the SetCollideBits function.
//
//		08/15/97 BRH	Made the smash radius larger.
//
//		08/17/97 BRH	Added looping thrust sound like the rocket.
//
//		08/17/97	JMI	Changed m_pthingParent to m_idParent.
//
//		08/23/97	JMI	Changed ms_u32SeekExcludeBits to include CSmash::AlmostDead
//							so the heatseeker wouldn't seek writhers.
//
//		08/24/97 BRH	Added seek bits that can be changed by the shooter so that
//							hostiles can set heatseekers that don't seek other enemies
//							Also set heatseeker back to previous position before
//							exploding so that it doesn't always hit the target from
//							behind and blow him forward.
//
//		08/26/97 BRH	Fixed problem with brackets that always set the missile
//							back a position.
//
//		08/27/97	JMI	No longer sets the smash radius to m_sCurRadius during 
//							Render().
//
////////////////////////////////////////////////////////////////////////////////
#define HEATSEEKER_CPP

#include "RSPiX.h"
#include <math.h>

#include "heatseeker.h"
#include "dude.h"
#include "explode.h"
#include "fire.h"
#include "SampleMaster.h"


////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////

#define SMALL_SHADOW_FILE "smallshadow.img"


////////////////////////////////////////////////////////////////////////////////
// Variables/data
////////////////////////////////////////////////////////////////////////////////

// These are default values -- actually values are set using the editor!
double CHeatseeker::ms_dAccUser     = 250.0;				// Acceleration due to user
double CHeatseeker::ms_dMaxVelFore  = 250.0;				// Maximum forward velocity
double CHeatseeker::ms_dMaxVelBack  = -250.0;			// Maximum backward velocity
double CHeatseeker::ms_dCloseDistance = 30.0;			// Close enough to hit CDude
double CHeatseeker::ms_dLineCheckRate = 15.0;			// Pixel distance for line checking
long CHeatseeker::ms_lArmingTime = 500;					// Time before weapon arms.
long CHeatseeker::ms_lSeekRadius = 150;						// Radius of heatseeking circle
short CHeatseeker::ms_sOffScreenDist = 200;				// Go off screen this far before blowing up
short CHeatseeker::ms_sAngularVelocity = 120;				// Degrees per second

// Set the collision bits
U32 CHeatseeker::ms_u32SeekIncludeBits = CSmash::Character | CSmash::Fire;
U32 CHeatseeker::ms_u32SeekDontcareBits = CSmash::Good | CSmash::Bad;
U32 CHeatseeker::ms_u32SeekExcludeBits = CSmash::Ducking | CSmash::AlmostDead;
U32 CHeatseeker::ms_u32CollideIncludeBits = CSmash::Character | CSmash::Misc | CSmash::Barrel | CSmash::Fire;
U32 CHeatseeker::ms_u32CollideDontcareBits = CSmash::Good | CSmash::Bad;
U32 CHeatseeker::ms_u32CollideExcludeBits = CSmash::Ducking; // Miss if they are ducking
long CHeatseeker::ms_lSmokeTrailInterval = 10;			// MS between smoke releases
long CHeatseeker::ms_lSmokeTimeToLive = 1000;			// MS for smoke to stay around.

// Let this auto-init to 0
short CHeatseeker::ms_sFileCount;

/// Rocket Animation Files
static char* ms_apszResNames[] = 
{
	"3d/Gmissile.sop",
	"3d/Gmissile.mesh",
	"3d/Gmissile.tex",
	"3d/Gmissile.hot",
	"3d/Gmissile.bounds",
	"3d/Gmissile.floor",
	NULL,
	NULL
};

////////////////////////////////////////////////////////////////////////////////
// Load object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CHeatseeker::Load(										// Returns 0 if successfull, non-zero otherwise
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
			TRACE("CHeatseeker::Load(): Error reading from file!\n");
		}
	}

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Save object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CHeatseeker::Save(										// Returns 0 if successfull, non-zero otherwise
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
void CHeatseeker::Update(void)
{
	USHORT usAttrib;
	short sHeight;
	double dNewX;
	double dNewZ;
	double dPrevX;
	double dPrevZ;

	if (!m_sSuspend)
		{
		// Get new time
		long lThisTime = m_pRealm->m_time.GetGameTime(); 

		// Calculate elapsed time in seconds
		double dSeconds = (double)(lThisTime - m_lPrevTime) / 1000.0;

		ProcessMessages();

		// Check the current state
		switch (m_eState)
		{
			case CWeapon::State_Hide:
			case CWeapon::State_Idle:
				break;

			case CWeapon::State_Fire:
				PlaySample(
					g_smidRocketFire,			// In:  Sample to play
					SampleMaster::Weapon,	// In:  User volume adjustment category
					DistanceToVolume(m_dX, m_dY, m_dZ, LaunchSndHalfLife), // In:  distance to dude
					&m_siThrust,				// Out: Handle for adjusting sound volume
					NULL,							// Out: Sample duration
					2841,							// In:  Where to loop back to in ms
					3090,							// In:  Where to loop fro in ms
					false );	// In:  Initial Sound Volume (0 - 255)
				m_lTimer = lThisTime + ms_lArmingTime;
				m_eState = State_Chase;
				break;

//-----------------------------------------------------------------------
// Chase
//-----------------------------------------------------------------------
			case CWeapon::State_Chase:
			{
				// Accelerate toward the target and check for proximity
				// and obstacles

				// Accelerate doofus up to max velocity
				m_dHorizVel += ms_dAccUser * dSeconds;

				// Limit to maximum velocity
				if (m_dHorizVel > ms_dMaxVelFore)
					m_dHorizVel = ms_dMaxVelFore;
				else if (m_dHorizVel < ms_dMaxVelBack)
					m_dHorizVel = ms_dMaxVelBack;

				// Adjust position based on velocity 
				dNewX = m_dX + rspCos(m_dRot) * (m_dHorizVel * dSeconds);
				dNewZ = m_dZ - rspSin(m_dRot) * (m_dHorizVel * dSeconds);

				// Check for obstacles
				sHeight = m_pRealm->GetHeight((short) dNewX, (short) dNewZ);
				usAttrib = m_pRealm->GetFloorAttribute((short) dNewX, (short) dNewZ);

				short	sRealmH	= m_pRealm->GetRealmHeight();
				short	sRealmW	= m_pRealm->GetRealmWidth();

				// Once a bit off screen, it should start turning back towards
				// the center of the hood.
				if (m_dZ > ms_sOffScreenDist + sRealmH || 
					 m_dZ < -ms_sOffScreenDist ||
					 m_dX > ms_sOffScreenDist + sRealmW ||
					 m_dX < -ms_sOffScreenDist)
				{
					short sTargetAngle = FindAngleTo(sRealmW / 2, 
																sRealmH / 2);
					short sAngleCCL = rspMod360(sTargetAngle - m_dRot);
					short sAngleCL  = rspMod360((360 - sTargetAngle) + m_dRot);
					short sAngleDistance = MIN(sAngleCCL, sAngleCL);
					double dAngleChange = MIN((double) sAngleDistance, ms_sAngularVelocity * dSeconds);
					if (sAngleCCL < sAngleCL)
						m_dRot = rspMod360(m_dRot + dAngleChange);
					else
						m_dRot = rspMod360(m_dRot - dAngleChange);
				}

				if (sHeight > m_dY || 
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

					// Change this to quick check closest
					if (m_pRealm->m_smashatorium.QuickCheckClosest(&m_smashSeeker,
																				  m_u32SeekBitsInclude,
																				  m_u32SeekBitsDontCare,
																				  m_u32SeekBitsExclude, &pSmashed))
					// Find the angle to the closest thing
					{
						if (m_pRealm->IsPathClear((short) m_dX, (short) m_dY, (short) m_dZ, ms_dLineCheckRate,
						                (short) pSmashed->m_sphere.sphere.X, (short) pSmashed->m_sphere.sphere.Z) )
						{
							short sTargetAngle = FindAngleTo(pSmashed->m_sphere.sphere.X, pSmashed->m_sphere.sphere.Z);
							short sAngleCCL = rspMod360(sTargetAngle - m_dRot);
							short sAngleCL  = rspMod360((360 - sTargetAngle) + m_dRot);
							short sAngleDistance = MIN(sAngleCCL, sAngleCL);
							double dAngleChange = MIN((double) sAngleDistance, ms_sAngularVelocity * dSeconds);
							if (sAngleCCL < sAngleCL)
								m_dRot = rspMod360(m_dRot + dAngleChange);
							else
								m_dRot = rspMod360(m_dRot - dAngleChange);
						}
					}
					m_pRealm->m_smashatorium.QuickCheckReset(
						&m_smash, 
						m_u32CollideBitsInclude,
						m_u32CollideBitsDontCare,
						m_u32CollideBitsExclude & ~CSmash::Ducking);

					while (m_pRealm->m_smashatorium.QuickCheckNext(&pSmashed))
					{
						ASSERT(pSmashed->m_pThing);

						const bool bIsPlayer = (pSmashed->m_pThing->GetClassID() == CDudeID);

						// we need to check ducking collisions unconditionally so we can unlock an achievement, but then we carry on if it should have missed.
						if ((m_u32CollideBitsExclude & CSmash::Ducking) && (pSmashed->m_bits & CSmash::Ducking))
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
								// Go back to previous spot where explosion should be created
								m_dX = dPrevX;
								m_dZ = dPrevZ;
							}
						}
						// Can't determine the shooter,but we did collide, so blow up.
						else
						{
							m_eState = CWeapon::State_Explode;
							// Go back to last position before exploding so it doesn't hit behind
							// the target.
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

				// Update sound position
				SetInstanceVolume(m_siThrust, DistanceToVolume(m_dX, m_dY, m_dZ, LaunchSndHalfLife) );

				break;
			}



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
					PlaySample(
						g_smidRocketExplode,
						SampleMaster::Destruction,
						DistanceToVolume(m_dX, m_dY, m_dZ, ExplosionSndHalfLife) );	// In:  Initial Sound Volume (0 - 255)
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

			case CWeapon::State_Deleted:
				delete this;
				return;
				break;
		}

		// Update sphere.
		m_smash.m_sphere.sphere.X			= m_dX;
		m_smash.m_sphere.sphere.Y			= m_dY;
		m_smash.m_sphere.sphere.Z			= m_dZ;
		m_smash.m_sphere.sphere.lRadius	= 2 * m_sprite.m_sRadius;

		m_smashSeeker.m_sphere.sphere.X = m_dX + (rspCos(m_dRot) * ms_lSeekRadius);
		m_smashSeeker.m_sphere.sphere.Y = m_dY;
		m_smashSeeker.m_sphere.sphere.Z = m_dZ - (rspSin(m_dRot) * ms_lSeekRadius);
		m_smashSeeker.m_sphere.sphere.lRadius = ms_lSeekRadius;

		// Update the smash.
		m_pRealm->m_smashatorium.Update(&m_smash);

		// Save time for next time
		m_lPrevTime = lThisTime;
	}
}


////////////////////////////////////////////////////////////////////////////////
// Render object
////////////////////////////////////////////////////////////////////////////////
void CHeatseeker::Render(void)
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

		// Draw the 2D shadow sprite
		CWeapon::Render();

#if 0
		// FEEDBACK.
		// Create a line sprite.
		CSpriteLine2d*	psl2d	= new CSpriteLine2d;
		if (psl2d != NULL)
			{
			Map3Dto2D(
				m_dX, 
				m_dY, 
				m_dZ, 
				&(psl2d->m_sX2), 
				&(psl2d->m_sY2) );
			Map3Dto2D(
				m_smashSeeker.m_sphere.sphere.X, 
				m_smashSeeker.m_sphere.sphere.Y, 
				m_smashSeeker.m_sphere.sphere.Z, 
				&(psl2d->m_sX2End), 
				&(psl2d->m_sY2End) );
			psl2d->m_sPriority	= m_smashSeeker.m_sphere.sphere.Z;
			psl2d->m_sLayer		= m_pRealm->GetLayerViaAttrib(m_pRealm->GetLayer(m_smashSeeker.m_sphere.sphere.X, m_smashSeeker.m_sphere.sphere.Z));
			psl2d->m_u8Color		= 249;
			// Destroy when done.
			psl2d->m_sInFlags	= CSprite::InDeleteOnRender;
			// Put 'er there.
			m_pRealm->m_scene.UpdateSprite(psl2d);
			}
#endif

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

short CHeatseeker::Setup(									// Returns 0 if successfull, non-zero otherwise
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

	m_smash.m_sphere.sphere.X			= m_dX;
	m_smash.m_sphere.sphere.Y			= m_dY;
	m_smash.m_sphere.sphere.Z			= m_dZ;
	m_smash.m_bits = CSmash::Projectile;
	m_smash.m_pThing = this;

	m_smashSeeker.m_sphere.sphere.X = m_dX + (rspCos(m_dRot) * ms_lSeekRadius);
	m_smashSeeker.m_sphere.sphere.Y = m_dY;
	m_smashSeeker.m_sphere.sphere.Z = m_dZ - (rspSin(m_dRot) * ms_lSeekRadius);
	m_smashSeeker.m_bits = 0;
	m_smashSeeker.m_pThing = this;

	m_u32CollideBitsInclude = ms_u32CollideIncludeBits;
	m_u32CollideBitsDontCare = ms_u32CollideDontcareBits;
	m_u32CollideBitsExclude = ms_u32CollideExcludeBits;

	m_u32SeekBitsInclude = ms_u32SeekIncludeBits;
	m_u32SeekBitsDontCare = ms_u32SeekDontcareBits;
	m_u32SeekBitsExclude = ms_u32SeekExcludeBits;

	m_sCurRadius = 10 * m_pRealm->m_scene.m_dScale3d;

	return sResult;
}

////////////////////////////////////////////////////////////////////////////////
// Get all required resources
////////////////////////////////////////////////////////////////////////////////
short CHeatseeker::GetResources(void)						// Returns 0 if successfull, non-zero otherwise
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
		TRACE("CHeatseeker::GetResources - Failed to open 3D animation for rocket\n");
	}

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Free all resources
////////////////////////////////////////////////////////////////////////////////
short CHeatseeker::FreeResources(void)						// Returns 0 if successfull, non-zero otherwise
{
	m_anim.Release();

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Preload - basically trick the resource manager into caching resources 
//				 for this object so there won't be a delay the first time it is
//				 created.
////////////////////////////////////////////////////////////////////////////////

short CHeatseeker::Preload(
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

void CHeatseeker::ProcessMessages(void)
{
	GameMessage msg;

	if (m_MessageQueue.DeQ(&msg) == true)
	{
		switch(msg.msg_Generic.eType)
		{
			case typeObjectDelete:
				m_MessageQueue.Empty();
				m_eState = State_Deleted;
				// This object is deleted later.
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
