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
// deathWad.cpp
// Project: Nostril (aka Postal)
//
// This module implements the CDeathWad weapon class which is an unguided
//	projectile.
// 
//
// History:
//		07/30/97 JMI	Started this weapon object from the CRocket.
//
//		08/07/97	JMI	Added additional parameter to CAnim3D::Get() call.
//
//		08/17/97	JMI	Changed m_pthingParent to m_idParent.
//
////////////////////////////////////////////////////////////////////////////////
//
// Wad vt wad-ded; wad-ding (1579)  1  a: to insert a wad into <~ a gun>
// b: to hold in by a wad <~ a bullet in a gun>  2: to form into a wad or
// wadding; esp : to roll or crush into a tight wad  3: to stuff or line with 
// some soft substance -- wad-der n
//
// This weapon is a wad of several ammunitions stuffed (or wadded) into a rocket
// cylinder and a napalm canister.  Fuel is used to propel the weapon, grenades 
// for extra (in addition to the rockets normal payload) explosive power, and 
// napalm for lasting fire(burn) power.  The rocket cylinder stores the fuel and
// provides the propulsion.  The napalm canister stores the extra explosive/fire
// powerload using some up with every collision.  It is for these reasons this
// weapon requires:
//  - exactly 1 rocket cylinder (including its original payload (solid fuel and 
//		explosive power))
//  - exactly 1 napalm canister (including its original payload (let's call it
//		liquid fire) )
//  - at least 1 canister fluid fuel (e.g., from flame thrower) (more provides
//		greater distance).
//  - at least 1 grenade (more provides greater explosive power over longer
//		distances).
//
////////////////////////////////////////////////////////////////////////////////
#define DEATHWAD_CPP

#include "RSPiX.h"
#include <math.h>

#include "deathWad.h"
#include "dude.h"
#include "explode.h"
#include "fire.h"
#include "SampleMaster.h"
#include "fireball.h"

////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////

#define SMALL_SHADOW_FILE	"smallshadow.img"

#define RES_BASE_NAME		"3d/missile"

// Define this if you want the empty missile casing (when there's no final 
// explosive power) to become a powerup flung through the air from the point
// at which it runs out of fuel.
//#define CAN_CHANGE_TO_POWERUP	1

// Gets a random between -range / 2 and range / 2.
#define RAND_SWAY(sway)	((GetRand() % sway) - sway / 2)

////////////////////////////////////////////////////////////////////////////////
// Variables/data
////////////////////////////////////////////////////////////////////////////////

// These are constant values!

// Internal acceleration.
const double	CDeathWad::ms_dAccInternal					= 350.0;
// Maximum forward velocity
const double	CDeathWad::ms_dMaxVelFore					= 350.0;
// Maximum backward velocity
const double	CDeathWad::ms_dMaxVelBack					= -350.0;
// Units moved each iteration while traversing the weapon path.
const double	CDeathWad::ms_dTraversalRate				= 3.0;		
// Distance between thrust feedbacks.
const double	CDeathWad::ms_dThrustDelta					= 9.0;
// Go off screen this far before blowing up
const short		CDeathWad::ms_sOffScreenDist				= 200;
// Time for smoke to stick around.
const long		CDeathWad::ms_lSmokeTimeToLive			= 500;
// Time for fireball to stick around.
const long		CDeathWad::ms_lFireBallTimeToLive		= 500;
// Amount to stagger final explosions.
const short		CDeathWad::ms_sFinalExplosionStagger	= 5;
// Radius of collision area (whether sphere or cylinder).
const short		CDeathWad::ms_sCollisionRadius			= 30;
// Velocity for kick from launch.
const double	CDeathWad::ms_dKickVelocity				= 350.0;
// Max a WAD can hold.
const CStockPile CDeathWad::ms_stockpileMax				=
	{
	0,				// m_sHitPoints
					                     
	5,				// m_sNumGrenades
	0,				// m_sNumFireBombs
	1,				// m_sNumMissiles
	1,				// m_sNumNapalms
	0,				// m_sNumBullets
	0,				// m_sNumShells
	50,			// m_sNumFuel
	0,				// m_sNumMines
	0,				// m_sNumHeatseekers
					                     
	0,				// m_sMachineGun
	0,				// m_sMissileLauncher
	0,				// m_sShotGun
	0,				// m_sSprayCannon
	0,				// m_sFlameThrower
	0,				// m_sNapalmLauncher
	0,				// m_sDeathWadLauncher
					                     
	0,				// m_sKevlarLayers
					                     
	0,				// m_sBackpack
	};

// Let this auto-init to 0
short CDeathWad::ms_sFileCount;

////////////////////////////////////////////////////////////////////////////////
// Load object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CDeathWad::Load(										// Returns 0 if successfull, non-zero otherwise
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
			TRACE("CDeathWad::Load(): Error reading from file!\n");
		}
	}

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Save object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CDeathWad::Save(										// Returns 0 if successfull, non-zero otherwise
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
	}

	// Save object data

	return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Update object
////////////////////////////////////////////////////////////////////////////////
void CDeathWad::Update(void)
{
	double dNewX;
	double dNewZ;

	ASSERT(m_dRot >= 0);
	ASSERT(m_dRot < 360);

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
				break;

			case CWeapon::State_Fire:
				
				Launch();

				m_eState = State_Chase;
				break;

//-----------------------------------------------------------------------
// Chase
//-----------------------------------------------------------------------
			case CWeapon::State_Chase:
				{
				// Accelerate toward the target and check for proximity
				// and obstacles

				// Accelerate up to max velocity
				m_dHorizVel += ms_dAccInternal * dSeconds;

				// Limit to maximum velocity
				if (m_dHorizVel > ms_dMaxVelFore)
					m_dHorizVel = ms_dMaxVelFore;
				else if (m_dHorizVel < ms_dMaxVelBack)
					m_dHorizVel = ms_dMaxVelBack;

				// Adjust position based on velocity.
				dNewX = m_dX + COSQ[(short)m_dRot] * (m_dHorizVel * dSeconds);
				dNewZ = m_dZ - SINQ[(short)m_dRot] * (m_dHorizVel * dSeconds);

				// If the new position is a ways off screen
				if (	m_dZ > ms_sOffScreenDist + m_pRealm->GetRealmHeight()
					||	m_dZ < -ms_sOffScreenDist
					||	m_dX > ms_sOffScreenDist + m_pRealm->GetRealmWidth() 
					||	m_dX < -ms_sOffScreenDist)
					{
					// Blow Up
					m_eState = CWeapon::State_Explode;
					}
				else 
					{
					// Traverse the path until we hit the newest position.
					while (TraversePath(		// Returns true, when destination reached; false,
													// if terrain change.                            
						m_dX,						// In:  Starting position.                       
						m_dY,						// In:  Starting position.                       
						m_dZ,						// In:  Starting position.                       
						&m_bInsideTerrain,	// In:  true, if starting in terrain.            
													// Out: true, if ending in terrain.              
						dNewX,					// In:  Destination position.                    
						dNewZ,					// In:  Destination position.                    
						&m_dX,					// Out: Position of inside terrain status change.
						&m_dZ)					// Out: Position of inside terrain status change.
						== false)
						{
						// Explosion at this point.
						Explosion();
						}
					}

				// If we have any charge left . . .
				if (m_stockpile.m_sNumGrenades)
					{
					// If we hit someone . . .
					CSmash* pSmashed = NULL;
					if (m_pRealm->m_smashatorium.QuickCheck(
						&m_smash, 
						m_u32CollideIncludeBits, 
						m_u32CollideDontcareBits,
						m_u32CollideExcludeBits, 
						&pSmashed) == true)
						{
						ASSERT(pSmashed->m_pThing);
						// Protect the launcher of the death wad . . .
						if (pSmashed->m_pThing->GetInstanceID() != m_u16ShooterID)
							{
							m_stockpile.m_sNumGrenades--;
							Explosion();
							}
						}
					}

				// If out of fuel . . .
				if (m_stockpile.m_sNumFuel <= 0)
					{
					// Blow Up
					m_eState = CWeapon::State_Explode;
					}

				// Update sound position.
				short	sVolumeHalfLife	= LaunchSndHalfLife;
				// If inside terrain . . .
				if (m_bInsideTerrain == true)
					{
					// Half half life.
					sVolumeHalfLife	/= 4;
					}

				SetInstanceVolume(m_siThrust, DistanceToVolume(m_dX, m_dY, m_dZ, sVolumeHalfLife) );

				// If no rocket or napalm canister . . .
				if (m_stockpile.m_sNumMissiles < 0 || m_stockpile.m_sNumNapalms < 0)
					{
					// Blow Up.
					m_eState	= CWeapon::State_Explode;
					}

				break;
				}

//-----------------------------------------------------------------------
// Explode
//-----------------------------------------------------------------------
			case CWeapon::State_Explode:

				// Start explosion objects and then kill deathwad
				// object.
#ifdef CAN_CHANGE_TO_POWERUP
				if (m_stockpile.m_sNumGrenades > 0)
					{
#endif
					while (m_stockpile.m_sNumGrenades > 0 || m_stockpile.m_sNumMissiles > 0)
						{
						Explosion();

						// Stagger.
						m_dX	+= RAND_SWAY(ms_sFinalExplosionStagger);
						m_dZ	+= RAND_SWAY(ms_sFinalExplosionStagger);

						m_stockpile.m_sNumGrenades--;
						m_stockpile.m_sNumMissiles--;
						}
#ifdef CAN_CHANGE_TO_POWERUP
					}
				else
					{
					// Otherwise, persist as powerup.
					CPowerUp*	ppowerup	= NULL;
					if (CThing::Construct(CPowerUpID, m_pRealm, (CThing**)&ppowerup) == 0)
						{
						// Copy whatever's left.
						ppowerup->m_stockpile.Copy(&m_stockpile);

						// Our's should now be empty for safety.  Matter cannot be created or destroyed
						// and all.
						m_stockpile.Zero();

						// Place powerup at our current location.
						ppowerup->Setup(m_dX, m_dY, m_dZ);

						// Blow it up.
						GameMessage	msg;
						msg.msg_Explosion.eType				= typeExplosion;
						msg.msg_Explosion.sPriority		= 0;
						msg.msg_Explosion.sDamage			= 0;
						msg.msg_Explosion.sX					= m_dX;
						msg.msg_Explosion.sY					= m_dY;
						msg.msg_Explosion.sZ					= m_dZ;
						msg.msg_Explosion.sVelocity		= 130;
						msg.msg_Explosion.u16ShooterID	= m_u16ShooterID;

						SendThingMessage(&msg, msg.msg_Explosion.sPriority, ppowerup);
						}
					else
						{
						TRACE("Update(): Failed to allocate new CPowerUp.\n");
						}
					}
#endif

				delete this;
				return;
				break;
		}

		// Update sphere.
		m_smash.m_sphere.sphere.X			= m_dX;
		m_smash.m_sphere.sphere.Y			= m_dY;
		m_smash.m_sphere.sphere.Z			= m_dZ;
		m_smash.m_sphere.sphere.lRadius	= m_sprite.m_sRadius;

		// Update the smash.
		m_pRealm->m_smashatorium.Update(&m_smash);

		// Save time for next time
		m_lPrevTime = lThisTime;
	}
}


////////////////////////////////////////////////////////////////////////////////
// Render object
////////////////////////////////////////////////////////////////////////////////
void CDeathWad::Render(void)
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
	m_sprite.m_sRadius = ms_sCollisionRadius;

	if (m_eState == State_Hide || m_bInsideTerrain == true)
		{
		m_sprite.m_sInFlags = CSprite::InHidden;
		}
	else
		{
		m_sprite.m_sInFlags = 0;
		}

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

short CDeathWad::Setup(									// Returns 0 if successfull, non-zero otherwise
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

	// Set the collision bits
	m_u32CollideIncludeBits = CSmash::Character | CSmash::Misc | CSmash::Barrel;
	m_u32CollideDontcareBits = CSmash::Good | CSmash::Bad;
	m_u32CollideExcludeBits = 0;

	m_smash.m_bits = CSmash::Projectile;
	m_smash.m_pThing = this;

	return sResult;
}

////////////////////////////////////////////////////////////////////////////////
// Get all required resources
////////////////////////////////////////////////////////////////////////////////
short CDeathWad::GetResources(void)						// Returns 0 if successfull, non-zero otherwise
	{
	short sResult = 0;
	
	sResult = m_anim.Get(RES_BASE_NAME, NULL, NULL, NULL, 0);
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
		TRACE("CDeathWad::GetResources - Failed to open 3D animation for deathwad projectile\n");
		}
	
	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Free all resources
////////////////////////////////////////////////////////////////////////////////
short CDeathWad::FreeResources(void)						// Returns 0 if successfull, non-zero otherwise
{
	m_anim.Release();

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Preload - basically trick the resource manager into caching resources 
//				 for this object so there won't be a delay the first time it is
//				 created.
////////////////////////////////////////////////////////////////////////////////

short CDeathWad::Preload(
	CRealm* prealm)				// In:  Calling realm.
{
	CAnim3D anim;	
	RImage* pimage;
	short sResult = anim.Get(RES_BASE_NAME, NULL, NULL, NULL, 0);
	if (sResult == 0)
		{
		anim.Release();
		}
	
	if (rspGetResource(&g_resmgrGame, prealm->Make2dResPath(SMALL_SHADOW_FILE), &pimage, RFile::LittleEndian) == 0)
		{
		rspReleaseResource(&g_resmgrGame, &pimage);
		}
	else
		{
		sResult	= -1;
		}

	CacheSample(g_smidDeathWadLaunch);
	CacheSample(g_smidDeathWadThrust);
	CacheSample(g_smidDeathWadExplode);
	return sResult;	
}

////////////////////////////////////////////////////////////////////////////////
// ProcessMessages
////////////////////////////////////////////////////////////////////////////////

void CDeathWad::ProcessMessages(void)
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
// Traverse the path until the inside terrain status changes or
// the destination is reached.
////////////////////////////////////////////////////////////////////////////////
bool CDeathWad::TraversePath(	// Returns true, when destination reached; false, 
										// if terrain change.
	short		sSrcX,				// In:  Starting position.
	short		sSrcY,				// In:  Starting position.
	short		sSrcZ,				// In:  Starting position.
	bool*		pbInTerrain,		// In:  true, if starting in terrain.
										// Out: true, if ending in terrain.
	short		sDstX,				// In:  Destination position.
	short		sDstZ,				// In:  Destination position.
	double*	pdCurX,				// Out: Position of inside terrain status change.
	double*	pdCurZ)				// Out: Position of inside terrain status change.
	{
	bool	bMadeDestination	= true;	// Assume we make it.

	ASSERT(pbInTerrain);
	ASSERT(pdCurX);
	ASSERT(pdCurZ);
	ASSERT(m_dRot >= 0);
	ASSERT(m_dRot < 360);

	// Determine distance on X/Z plane to destination.
	double	dDistance	= rspSqrt(ABS2(float(sSrcX - sDstX), float(sSrcZ - sDstZ) ) );

	// Set starting position.
	double	dX	= sSrcX;
	double	dZ	= sSrcZ;
	// Determine iteration rate on X and Z.
	double	dRateX	= COSQ[(short)m_dRot] * ms_dTraversalRate;
	double	dRateZ	= -SINQ[(short)m_dRot] * ms_dTraversalRate;
	// Store original status.
	bool	bInitiallyInTerrain	= *pbInTerrain;

	// Loop until change in status or we hit destination.
	while (
		dDistance > 0 &&
		*pbInTerrain == bInitiallyInTerrain)
		{
		if (m_pRealm->GetHeight(dX, dZ) > sSrcY)
			*pbInTerrain	= true;
		else
			*pbInTerrain	= false;

		// See if it's time to create a thrust . . .
		m_dUnthrustedDistance	+= ms_dTraversalRate;
		if (m_dUnthrustedDistance >= ms_dThrustDelta)
			{
			// Thrustage.
			Thrust();
			// Reset for next.
			m_dUnthrustedDistance	= 0.0;
			}

		dX				+= dRateX;
		dZ				+= dRateZ;
		dDistance	-= ms_dTraversalRate;
		}

	// If we did not make the destination . . .
	if (dDistance > 0)
		{
		bMadeDestination	= false;
		}

	// Store new position.
	*pdCurX	= dX;
	*pdCurZ	= dZ;

	return bMadeDestination;
	}

////////////////////////////////////////////////////////////////////////////////
// Generate an explosion at the current position.
////////////////////////////////////////////////////////////////////////////////
void CDeathWad::Explosion(void)
	{
	// Start an explosion object and some smoke (doesn't an explosion object
	// automatically make smoke??).
	CExplode* pExplosion;
	if (CThing::Construct(CThing::CExplodeID, m_pRealm, (CThing**) &pExplosion) == 0)
		{
		// Don't blow us up.
		pExplosion->m_u16ExceptID	= m_u16ShooterID;

		pExplosion->Setup(m_dX, MAX(m_dY-30, 0.0), m_dZ, m_u16ShooterID);
		PlaySample(										// Returns nothing.
															// Does not fail.
			g_smidDeathWadExplode,					// In:  Identifier of sample you want played.
			SampleMaster::Destruction,				// In:  Sound Volume Category for user adjustment
			DistanceToVolume(m_dX, m_dY, m_dZ, ExplosionSndHalfLife) );	// In:  Initial Sound Volume (0 - 255)
		}
	
	short a;
	CFire* pSmoke;
	for (a = 0; a < 8; a++)
		{
		if (CThing::Construct(CThing::CFireID, m_pRealm, (CThing**) &pSmoke) == 0)
			{
			pSmoke->Setup(m_dX - 4 + GetRandom() % 9, m_dY-20, m_dZ - 4 + GetRandom() % 9, ms_lSmokeTimeToLive, true, CFire::Smoke);
			pSmoke->m_u16ShooterID = m_u16ShooterID;
			}
		}


	if (m_stockpile.m_sNumFuel > 0 || m_stockpile.m_sNumNapalms > 1)
		{
		// Also, create a fire.
		}
	}


////////////////////////////////////////////////////////////////////////////////
// Generate some thrust at the current position.
////////////////////////////////////////////////////////////////////////////////
void CDeathWad::Thrust(void)
	{
	m_stockpile.m_sNumFuel--;


	if (m_bInsideTerrain == false)
		{
		CFire* pSmoke = NULL;
		if (CThing::Construct(CThing::CFireID, m_pRealm, (CThing**) &pSmoke) == 0)
			{
			// This needs to be fixed by calculating the position of the back end of
			// the deathwad in 3D based on the rotation.  
			pSmoke->Setup(m_dX, m_dY, m_dZ, ms_lSmokeTimeToLive, true, CFire::SmallSmoke);
			pSmoke->m_u16ShooterID = m_u16ShooterID;
			}

		// Also, create a fire (moving at the wad's velocity?).
		CFireball*	pfireball	= NULL;
		if (CThing::Construct(CFireballID, m_pRealm, (CThing**) &pfireball) == 0)
			{
			pfireball->Setup(m_dX, m_dY, m_dZ, m_dRot, ms_lFireBallTimeToLive, m_u16ShooterID);
			pfireball->m_dHorizVel	= m_dHorizVel / 4.0;
			pfireball->m_eState		= State_Fire;
			}
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Generate the launch kick/debris.
////////////////////////////////////////////////////////////////////////////////
void CDeathWad::Launch(void)
	{
	// The launch sound.
	PlaySample(										// Returns nothing.
														// Does not fail.
		g_smidDeathWadLaunch,					// In:  Identifier of sample you want played.
		SampleMaster::Weapon,					// In:  Sound Volume Category for user adjustment
		DistanceToVolume(m_dX, m_dY, m_dZ, LaunchSndHalfLife) );	// In:  Initial Sound Volume (0 - 255)
	
	// The looping thrust sound.
	PlaySample(										// Returns nothing.
														// Does not fail.
		g_smidDeathWadThrust,					// In:  Identifier of sample you want played.
		SampleMaster::Weapon,					// In:  Sound Volume Category for user adjustment
		DistanceToVolume(m_dX, m_dY, m_dZ, LaunchSndHalfLife),	// In:  Initial Sound Volume (0 - 255)
		&m_siThrust,								// Out: Handle for adjusting sound volume
		NULL,											// Out: Sample duration in ms, if not NULL.
		100,											// In:  Where to loop back to in milliseconds.
														//	-1 indicates no looping (unless m_sLoop is
														// explicitly set).
		500,											// In:  Where to loop back from in milliseconds.
														// In:  If less than 1, the end + lLoopEndTime is used.
		false);										// In:  Call ReleaseAndPurge rather than Release after playing

	Explosion();

	CThing*	pthing	= NULL;
	// Get the launcher . . .
	if (m_pRealm->m_idbank.GetThingByID(&pthing, m_u16ShooterID) == 0)
		{
		// If it's a dude . . .
		if (pthing->GetClassID() == CDudeID)
			{
			CDude*	pdude	= (CDude*)pthing;
			// Add force vector for kick.  See ya.
			pdude->AddForceVector(ms_dKickVelocity, m_dRot - 180);
			}
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Feed the WAD prior to moving its state to State_Fire.
////////////////////////////////////////////////////////////////////////////////
void CDeathWad::FeedWad(
	CStockPile*	pstockpile)	// In:  Src for WAD's arsenal.
	{
	// Take needed ammo.
	m_stockpile.m_sNumMissiles	= pstockpile->m_sNumMissiles;
	m_stockpile.m_sNumNapalms	= pstockpile->m_sNumNapalms;
	m_stockpile.m_sNumFuel		= pstockpile->m_sNumFuel;
	m_stockpile.m_sNumGrenades	= pstockpile->m_sNumGrenades;
	// Truncate to max we can hold.
	m_stockpile.Intersect(&ms_stockpileMax);
	// Subtract from provider.
	pstockpile->Sub(&m_stockpile);
	}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
