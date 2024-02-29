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
// grenade.cpp
// Project: Nostril (aka Postal)
//
// This module implements the CGrenade weapon class which is a hand
// thrown grenade weapon.
// 
//
// History:
//		01/17/97 BRH	Started this weapon object.
//
//		02/04/97	JMI	Changed LoadDib() call to Load() (which now supports
//							loading of DIBs).
//
//		02/11/97	JMI	Stripped 2D.  Added 3D and a concept of having a parent.
//
//		02/13/97	JMI	Changing RForm3d to RSop.
//
//		02/15/97	JMI	Now sets m_sprite.m_psphere Render().
//
//		02/15/97	JMI	Made rotation to match CDude's in Update().
//
//		02/17/97	JMI	Removed m_sCurRadius from 3D to 2D mapping in Render().
//							Also, changed all querries on the height map to multiply
//							by 4.
//
//		02/18/97 BRH	Fixed Explosion.  Was calling the wrong setup function.
//
//		02/19/97 BRH	Added processing of message queue to check for 
//							ObjectDelted messages.
//
//		02/23/97 BRH	Added Preload() function to cache resources for this
//							object before game play begins.
//
//		02/24/97	JMI	Now uses AdjustPosVel() for accel due to gravity and
//							AdjustVel() for ground drag.
//							Also, plays sound when it hits terrain wall while in air.
//
//		02/24/97 BRH	Made roll state check for ground to see if it is falling,
//							and if so it goes back to airborne State_Go.
//
//		02/24/97 BRH	Changed ground and wall finding algorithm and made the
//							grenade delete itself it is fired in an invalid position
//							i.e. stuck in a wall.
//
//		02/24/97	JMI	No longer sets the m_type member of the m_sprite b/c it
//							is set by m_sprite's constructor.
//
//		02/24/97 BRH	Tuned the distance it travels on the ground by increasing
//							the requirements for bounce.  Also made it possible to
//							blow up in states other than the final state so that the
//							fuse time is consistant.
//
//		03/03/97	JMI	Commented out dHorizVelocity and dVertVelocity parameters
//							to Setup() so that this version would be a virtual over-
//							ride of CWeapon's.
//
//		03/13/97	JMI	Load now takes a version number.
//
//		03/19/97 BRH	Changed ProcessMessages to return void so that it
//							matches the new virtual function in the CWeapon
//							base class.
//
//		03/21/97 BRH	Changed attribute checking to ignore the NOT_WALKABLE
//							attribute so that grenades won't bounce off the edge
//							of the world anymore.
//
//		04/10/97 BRH	Updated this to work with the new multi layer attribute
//							maps.
//
//		04/21/97 BRH	Changed call to explosion Setup function to include a
//							parameter to select the special grenade explosion rather
//							than the default explosion animation.
//
//		04/24/97 BRH	Added a puff of smoke in addition to the explosion.
//
//		05/04/97 BRH	Took out an unused reference to an STL iterator.
//							Increased the drag for the grenade when it hits the grounds.
//
//		05/20/97 BRH	Bumped up the ground drag to make the SetRange more
//							accurate.
//
//		05/29/97	JMI	Removed ASSERT on m_pRealm->m_pAttribMap which no longer
//							exists.
//
//		06/03/97 BRH	Capped the ground velocity and decreased the drag back
//							to a more normal level.  Also added a bit of variance in
//							the angle when it hits the ground.
//
//		06/11/97 BRH	Passes on shooter ID to the explosion that it creates.
//
//		06/12/97 BRH	Added shooter ID to the call to Setup for the explosion.
//							Also added the State_Hide to hide the grenade.
//
//		06/13/97	JMI	Grenade was not unhiding until it had no parent b/c the 
//							part that unhides was inside the 'if I don't have a parent'
//							condition.
//
//		06/16/97 BRH	Fixed starting condition over not walkable attribute.  
//
//		06/17/97 BRH	Changed the velocity of the grenade to 33% of its
//							original value when it hits a wall.
//
//		06/17/97	JMI	Converted all occurrences of rand() to GetRand() and
//							srand() to SeedRand().
//
//		06/25/97 BRH	Added use of base class 2D shadow on the ground, but loaded
//							a smaller shadow resource.
//
//		06/30/97 BRH	Added sound effect cache to the Preload function.
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
//							ProcessMessages() was deleting the grenade on a delete msg
//							but, once returned to Update(), it was checking the 
//							m_eState member to see if it should return.  Unfortunately,
//							since 'this' had already been deallocated, it was too late
//							to do such a thing.
//
//		08/05/97	JMI	Changed priority to use Z position rather than 2D 
//							projected Y position.
//
//		08/08/97	JMI	Added Style so this can be visually represented in multiple
//							ways.
//
//		08/08/97	JMI	Added variables for rotating the weapon and smoke when
//							dynamite.
//
//		08/17/97	JMI	Changed m_pthingParent to m_idParent.
//
////////////////////////////////////////////////////////////////////////////////
#define GRENADE_CPP

#include "RSPiX.h"
#include <math.h>

#include "grenade.h"
#include "dude.h"
#include "explode.h"
#include "fire.h"
#include "SampleMaster.h"
#include "reality.h"


////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////

#define	SMALL_SHADOW_FILE	"smallshadow.img"


////////////////////////////////////////////////////////////////////////////////
// Variables/data
////////////////////////////////////////////////////////////////////////////////

// These are default values -- actually values are set using the editor!
double CGrenade::ms_dAccUser     = 250.0;				// Acceleration due to user
double CGrenade::ms_dAccDrag     = 300.0;				// Acceleration due to drag

double CGrenade::ms_dMaxVelFore  = 150.0;				// Maximum forward velocity
double CGrenade::ms_dMaxVelBack  = -150.0;				// Maximum backward velocity

double CGrenade::ms_dDegPerSec   = 150.0;				// Degrees of rotation per second

double CGrenade::ms_dCloseDistance = 30.0;			// Close enough to hit CDude
double CGrenade::ms_dGravity = -9.5;					// acceleration due to gravity
double CGrenade::ms_dThrowVertVel = 30;				// Throw up at this velocity
double CGrenade::ms_dThrowHorizVel = 200;				// Throw out at this velocity
double CGrenade::ms_dMinBounceVel = 40.0;				// Will bounce up if more than this
double CGrenade::ms_dBounceTransferFract = -0.2;	// Amount of bounce velocity transferred.
long CGrenade::ms_lRandomAvoidTime = 200;				// Time to wander before looking again
long CGrenade::ms_lReseekTime = 1000;					// Do a 'find' again 
long CGrenade::ms_lGrenadeFuseTime = 1500;			// Time from throw to blow
long CGrenade::ms_lSmokeInterval		= 100;			// Time between smokes.

char*	CGrenade::ms_apszResNames[CGrenade::NumStyles]	= // Res names indexed Style.
	{
	"3d/grenade",		// Grenade.
	"3d/dynamite",		// Dynamite.
	};

// Dimishes velocities once it hits the ground.
double	CGrenade::ms_adGroundDimisher[NumStyles]	=
	{
	0.66,	// Grenade. 
	0.66,	// Dynamite.
	};

// Dimishes velocities when bouncing.
double	CGrenade::ms_adBounceDimisher[NumStyles]	=
	{
	0.33,	// Grenade. 
	0.66,	// Dynamite.
	};

// Let this auto-init to 0
short CGrenade::ms_sFileCount;

////////////////////////////////////////////////////////////////////////////////
// Load object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CGrenade::Load(				// Returns 0 if successfull, non-zero otherwise
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
					pFile->Read(&ms_dMaxVelFore);
					pFile->Read(&ms_dMaxVelBack);
					pFile->Read(&ms_dDegPerSec);
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
			TRACE("CGrenade::Load(): Error reading from file!\n");
			}
	}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Save object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CGrenade::Save(										// Returns 0 if successfull, non-zero otherwise
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
		pFile->Write(&ms_dAccUser);
		pFile->Write(&ms_dAccDrag);
		pFile->Write(&ms_dMaxVelFore);
		pFile->Write(&ms_dMaxVelBack);
		pFile->Write(&ms_dDegPerSec);
		}

	// Save object data

	return 0;
	}



////////////////////////////////////////////////////////////////////////////////
// Update object
////////////////////////////////////////////////////////////////////////////////
void CGrenade::Update(void)
	{
	USHORT usAttrib;
	short sHeight = m_sPrevHeight;
	double dPrevVertVel;
	double dNewX;
	double dNewY;
	double dNewZ;

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
			case CWeapon::State_Hide:
			case CWeapon::State_Idle:

				Smoke();

				break;

			case CWeapon::State_Fire:
				sHeight = m_pRealm->GetHeight((short) m_dX, (short) m_dZ);
				usAttrib = m_pRealm->GetFloorAttribute((short) m_dX, (short) m_dZ);
				// If it starts at an invalid position like inside a wall, kill it
				if (m_dY < sHeight)
				{
					delete this;
					return;
				}
				m_eState = State_Go;
				m_lTimer = lThisTime + ms_lGrenadeFuseTime;
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
				dPrevVertVel = m_dVertVel;
				AdjustPosVel(&dNewY, &m_dVertVel, dSeconds/*, dAccelerationDueToGravity*/);
				// Check the height to see if it hit the ground
				sHeight = m_pRealm->GetHeight((short) dNewX, (short) dNewZ);

				// Adjust apparent rotation.
				m_dAnimRotY	= rspMod360(m_dAnimRotY + m_dAnimRotVelY * dSeconds); 
				m_dAnimRotZ	= rspMod360(m_dAnimRotZ + m_dAnimRotVelZ * dSeconds);

				// If its lower than the last and current height, assume it
				// hit the ground.
				if (dNewY < sHeight && m_dY >= sHeight)
				{
					m_dY = sHeight;
					m_dVertVel = dPrevVertVel;
					// If its vertical velocity is high enough, it will bounce, else
					// it will just start rolling
					if (-m_dVertVel > ms_dMinBounceVel)
					{
						m_dVertVel = m_dVertVel * ms_dBounceTransferFract;
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
					}
					else
					{
						m_eState = CWeapon::State_Roll;	

						// Pop a cap on that horizontal velocity
						if (m_dHorizVel > 0)
							m_dHorizVel = MIN(m_dHorizVel, ms_adBounceDimisher[m_style] * ms_dMaxVelFore);
						else
							m_dHorizVel = MAX(m_dHorizVel, ms_adBounceDimisher[m_style] * ms_dMaxVelBack);
						m_dRot = rspMod360(m_dRot - 100 + (GetRand() % 201));
					}
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

						// Cut down its horizontal velocity
						m_dHorizVel *= ms_adBounceDimisher[m_style];
						// Cut down rotations.
						m_dAnimRotY	*= ms_adBounceDimisher[m_style];
						m_dAnimRotZ	*= ms_adBounceDimisher[m_style];

						PlaySample(
							g_smidGrenadeBounce, 
							SampleMaster::Weapon,
							DistanceToVolume(m_dX, m_dY, m_dZ, SideEffectSndHalfLife) );	// In:  Initial Sound Volume (0 - 255)
					}
					else
						m_dY = dNewY;
				}


				m_dX = dNewX;
				m_dZ = dNewZ;

				Smoke();

				// See if its fuse is up
				if (lThisTime > m_lTimer)
					m_eState = CWeapon::State_Explode;

				break;

//-----------------------------------------------------------------------
// Roll - Once it hits the ground, roll until it stops.
//-----------------------------------------------------------------------
			case CWeapon::State_Roll:
				// Ground causes drag
				// Decelerate to zero.  When you reach zero, go
				// to find state.
#if 0
				if (m_dVel > 0)
				{
					m_dVel -= ms_dAccDrag * dSeconds;
					if (m_dVel < 0)
						m_dVel = 0;
				}
				else if (m_dVel < 0)
				{
					m_dVel += ms_dAccDrag * dSeconds;
					if (m_dVel > 0)
						m_dVel = 0;
				}
#else
				if (m_dHorizVel > 0)
				{
					AdjustVel(&m_dHorizVel, dSeconds, -ms_dAccDrag);
					if (m_dHorizVel < 0)
						m_dHorizVel = 0;
				}
				else if (m_dHorizVel < 0)
				{
					AdjustVel(&m_dHorizVel, dSeconds, ms_dAccDrag);
//					if (m_dVel > 0)
						m_dHorizVel = 0;
				}
#endif
				// If it has stopped, then change to find state
				if (m_dHorizVel == 0)
					m_eState = CWeapon::State_Explode;

				dNewX = m_dX + COSQ[(short)m_dRot] * (m_dHorizVel * dSeconds);
				dNewZ = m_dZ - SINQ[(short)m_dRot] * (m_dHorizVel * dSeconds);
				// Check for obstacles
				usAttrib = m_pRealm->GetFloorAttribute((short) dNewX, (short) dNewZ);
				sHeight = m_pRealm->GetHeight((short) dNewX, (short) dNewZ);

				// If it hit any obstacles, make it bounce off
				if (usAttrib & REALM_ATTR_NOT_WALKABLE || sHeight > m_dY)
				{
					// Restore previous position 
					dNewX = m_dX;
					dNewZ = m_dZ;
					// Change directions
					m_dRot = BounceAngle(m_dRot);
				}
				// See if it fell off of something, make it go back to the
				// airborne state.
				if (sHeight < (short) m_dY)
				{
					m_dVertVel = 0;
					m_eState = State_Go;					
				}

				m_dX = dNewX;
				m_dZ = dNewZ;

				Smoke();

				// See if its fuse is up
				if (lThisTime > m_lTimer)
					m_eState = CWeapon::State_Explode;

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
						pExplosion->Setup(m_dX, m_dY, m_dZ, m_u16ShooterID, 1);
						PlaySample(g_smidGrenadeExplode, 
							SampleMaster::Destruction,
							DistanceToVolume(m_dX, m_dY, m_dZ, ExplosionSndHalfLife) );	// In:  Initial Sound Volume (0 - 255)
					}

					short a;
					CFire* pSmoke;
					for (a = 0; a < 4; a++)
					{
						if (CThing::Construct(CThing::CFireID, m_pRealm, (CThing**) &pSmoke) == 0)
						{
							pSmoke->Setup(m_dX + GetRand() % 8, m_dY, m_dZ + GetRand() % 8, 2000, true, CFire::Smoke);
							pSmoke->m_u16ShooterID = m_u16ShooterID;
						}
					}

					delete this;
					return;
				}
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
void CGrenade::Render(void)
	{
	// Animate.
	long	lCurTime			= m_pRealm->m_time.GetGameTime();

	m_sprite.m_pmesh		= (RMesh*)		m_anim.m_pmeshes->GetAtTime(		lCurTime % m_anim.m_pmeshes->TotalTime());
	m_sprite.m_psop		= (RSop*)		m_anim.m_psops->GetAtTime(			lCurTime % m_anim.m_psops->TotalTime());
	m_sprite.m_ptex		= (RTexture*)	m_anim.m_ptextures->GetAtTime(	lCurTime % m_anim.m_ptextures->TotalTime());
	m_sprite.m_psphere	= (RP3d*)		m_anim.m_pbounds->GetAtTime(		lCurTime % m_anim.m_pbounds->TotalTime());

	// This should eventually be channel driven too.
	m_sprite.m_sRadius	= m_sCurRadius;

	// See if it is hidden or not
	if (m_eState == State_Hide)
		m_sprite.m_sInFlags = CSprite::InHidden;
	else
		m_sprite.m_sInFlags	= 0;
	
	// If we're not a child of someone else . . .
	if (m_idParent == CIdBank::IdNil)
		{
		// Map from 3d to 2d coords
		Map3Dto2D((short) m_dX, (short) m_dY, (short) m_dZ, &m_sprite.m_sX2, &m_sprite.m_sY2);

		// Priority is based on bottom edge of sprite
		m_sprite.m_sPriority = m_dZ;

		// Layer should be based on info we get from attribute map.
		m_sprite.m_sLayer = CRealm::GetLayerViaAttrib(m_pRealm->GetLayer((short) m_dX, (short) m_dZ));

		// Adjust transformation based current rotations.
		m_trans.Make1();
		m_trans.Ry(m_dAnimRotY);
		m_trans.Rx(m_dAnimRotZ);

		m_sprite.m_ptrans		= &m_trans;

		// Update sprite in scene
		m_pRealm->m_scene.UpdateSprite(&m_sprite);

		// Let it render the shadow sprite.
		CWeapon::Render();
		}
	else
		{
		// m_idParent is setting our transform relative to its.
		// We are drawn when m_idBank is drawn.  Don't add to scene.
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Setup new object - called by object that created this object
////////////////////////////////////////////////////////////////////////////////

short CGrenade::Setup(									// Returns 0 if successfull, non-zero otherwise
	short sX,												// In:  New x coord
	short sY,												// In:  New y coord
	short sZ/*,												// In:  New z coord
	double dHorizVelocity,								// In:  Horiz Vel (has a default)
	double dVertVelocity*/)								// In:  Vertical velocity (has a default)
	{
	short sResult = 0;
	
	// Use specified position
	m_dX = (double)sX;
	m_dY = (double)sY;
	m_dZ = (double)sZ;
	m_dHorizVel = ms_dThrowHorizVel;//dHorizVelocity;
	m_dVertVel = ms_dThrowVertVel;//dVertVelocity;

	m_dAnimRotVelY		= GetRand() % 180;
	m_dAnimRotVelZ		= GetRand() % 180;

	// HARD-WIRED CODE ALERT!
	// Eventually, this should be set via the bounding sphere radius.
	m_sCurRadius	= 22;		// FOR NOW, always half of scene.cpp:SCREEN_DIAMETER_FOR_3D.

	// Load resources
	sResult = GetResources();
	
	// Enable the 2D shadow.
	PrepareShadow();

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Get all required resources
////////////////////////////////////////////////////////////////////////////////
short CGrenade::GetResources(void)						// Returns 0 if successfull, non-zero otherwise
	{
	short sResult = 0;

	sResult	= m_anim.Get(ms_apszResNames[m_style], NULL, NULL, NULL, RChannel_LoopAtStart | RChannel_LoopAtEnd);
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
		TRACE("CGrenade::GetResources - Failed to open 3D stuff file\n");
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Free all resources
////////////////////////////////////////////////////////////////////////////////
short CGrenade::FreeResources(void)						// Returns 0 if successfull, non-zero otherwise
	{
	short sResult = 0;

	// Release resources for animation.
	m_anim.Release();

	return sResult;
	}

////////////////////////////////////////////////////////////////////////////////
// Preload - basically trick the resource manager into caching resources 
//				 for this object so there won't be a delay the first time it is
//				 created.
////////////////////////////////////////////////////////////////////////////////

short CGrenade::Preload(
	CRealm* prealm)				// In:  Calling realm.
	{
	short	sResult	= 0;

	CAnim3D anim;
	RImage* pimage;

	short	sStyle;
	for (sStyle = 0; sStyle < NumStyles; sStyle++)
		{
		if (anim.Get(ms_apszResNames[sStyle], NULL, NULL, NULL, RChannel_LoopAtStart | RChannel_LoopAtEnd) == 0)
			{
			anim.Release();
			}
		else
			{
			// Go ahead and overwrite any previous error.
			sResult	= 1;
			}
		}

	rspGetResource(&g_resmgrGame, prealm->Make2dResPath(SMALL_SHADOW_FILE), &pimage, RFile::LittleEndian);
	rspReleaseResource(&g_resmgrGame, &pimage);
	CacheSample(g_smidGrenadeBounce);
	CacheSample(g_smidGrenadeExplode);
	return sResult; 
	}

////////////////////////////////////////////////////////////////////////////////
// ProcessMessages
////////////////////////////////////////////////////////////////////////////////

void CGrenade::ProcessMessages(void)
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
// Smoke, if correct time.
////////////////////////////////////////////////////////////////////////////////
void CGrenade::Smoke(void)
	{
	if (m_style == Dynamite)
		{
		long	lCurTime	= m_pRealm->m_time.GetGameTime();

		// If the timer has expired . . .
		if (lCurTime > m_lNextSmokeTime)
			{
			CFire*	pSmoke;
			if (CThing::Construct(CThing::CFireID, m_pRealm, (CThing**) &pSmoke) == 0)
				{
				pSmoke->Setup(m_dX + GetRand() % 8, m_dY, m_dZ + GetRand() % 8, 2000, true, CFire::SmallSmoke);
				pSmoke->m_u16ShooterID = m_u16ShooterID;
				}
			
			m_lNextSmokeTime	= lCurTime + ms_lSmokeInterval;
			}
		}
	}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
