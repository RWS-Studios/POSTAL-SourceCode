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
// sentry.cpp
// Project: Postal
//
// This module implements the automatic sentry gun
//
// History:
//		06/02/97 BRH	Created this Sentry gun from gunner.cpp.
//
//		06/05/97	JMI	Changed m_sHitPoints to m_stockpile.m_sHitPoints to 
//							accommodate new m_stockpile in base class, CThing3d (which
//							used to contain the m_sHitPoints).
//
//		06/13/97 BRH	Added Turret and Base for the Sentry.
//
//		06/16/97 BRH	Added blown up animation, dialog box for weapon selection
//							and settings.  Fixed positioning problems.
//
//		06/17/97 BRH	Added SetRangeToTarget call for weapons that 
//							require range adjustment.
//
//		06/18/97 BRH	Changed over to using GetRandom()
//
//		06/25/97	JMI	Now calls PrepareShadow() in Init() which loads and sets up
//							a shadow sprite.
//
//		06/30/97	JMI	Added override for EditRect() and EditHotSpot().
//							Now sets priority and layer for turret from base's values.
//
//					MJR	Replaced SAFE_GUI_REF with new GuiItem.h-defined macro.
//
//					BRH	Caches the sound effects during the static portion of
//							load so that the sound effect will be ready on any
//							level that has a sentry gun.
//
//		07/01/97 BRH	Added angular velocity to allow tuning of the rotation
//							rate of the sentry gun.  Still need to edit the dialog
//							box and EditModify to set the change.
//
//		07/02/97 BRH	Added angular velocity setting to edit modify dialog.
//
//		07/18/97	JMI	Got rid of bogus immitation PlaySample functions.
//							Now there is one PlaySample() function.  Also, you now
//							MUST specify a category and you don't have to specify a
//							SoundInstance ptr to specify a volume.
//
//		07/31/97	JMI	Changed m_sPriority to use 3D z position like CThin3d does.
//
//		08/01/97 BRH	Took out reference to animation hots since we are not
//							using those.
//
//		08/08/97	JMI	Now dynamically builds the weapons list for the 
//							EditModify().
//							Also, since it doesn't call the base class Update(), it
//							has to monitor the flamage sample.
//
//		08/11/97	JMI	Added transform for base, m_transBase.
//							Also, made UpdatePosition() bypass its logic when the 
//							animations are not yet set.
//
//		08/16/97 BRH	Added collision bits for the Sentry gun to pass to 
//							ShootWeapon.
//
//		08/18/97	JMI	Now plays impact animation when hit by bullets.
//
//		08/18/97	JMI	Changed State_Dead to call DeadRender3D() (which used to be
//							known/called as just another Render() overload).
//
//		08/20/97 BRH	Changed ricochet sounds from Destruction to Weapon volume
//							slider.
//
//		08/26/97 BRH	Changed sentry gun getting hit by bullets sound.
//
//		09/02/97	JMI	Changed use of Misc bit to Sentry bit.
//
//		09/03/97	JMI	Sentries now exclude CSmash::Bads and CSmash::Civilians.
//
////////////////////////////////////////////////////////////////////////////////
#define SENTRY_CPP

#include "RSPiX.h"
#include "sentry.h"
#include "game.h"
#include "SampleMaster.h"

#include "reality.h"

////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////

// Anim for when a barrel is hit by a bullet.
#define SENTRY_HIT_RES_NAME	"Ricochet.aan"

#define HULL_RADIUS				(m_sprite.m_sRadius / 2)

// Gets a GetRandom()om between -range / 2 and range / 2.
#define RAND_SWAY(sway)		((GetRandom() % sway) - sway / 2)

// Tunable bullet parameters.
#define MAX_BULLET_RANGE		400
#define MAX_BULLETS_PER_SEC	6

#define MS_BETWEEN_BULLETS		(1000 / MAX_BULLETS_PER_SEC)

////////////////////////////////////////////////////////////////////////////////
// Variables/data
////////////////////////////////////////////////////////////////////////////////

// These are default values -- actually values are set using the editor!
double CSentry::ms_dTooCloseDistance = 190*190;	// Close enough to hit CDude
double CSentry::ms_dLongRange = 500*500;	// Squared distance (500 pixels away)
double CSentry::ms_dInRangeLow = 30*30;	// Squared distance to be in range with weapon
double CSentry::ms_dInRangeHigh = 230*230;// Squared distance to be in range with weapon
double CSentry::ms_dGravity = -19.5;		// Cheater gravity
double CSentry::ms_dBlowupVelocity = 190;	// Initial vertical velocity
long CSentry::ms_lRandomAvoidTime = 200;	// Time to wander before looking again
long CSentry::ms_lReseekTime = 1000;		// Do a 'find' again 
long CSentry::ms_lWatchWaitTime = 2500;		// Time to watch shot go
long CSentry::ms_lPatrolTime = 5000;		// Time to patrol before shooting
long CSentry::ms_lDeathTimeout = 20000;	// Wait around after dying
long CSentry::ms_lBurningRunTime = 50;		// Run this time before turning
short CSentry::ms_sHitLimit = 150;			// Number of starting hit points
short CSentry::ms_sBurntBrightness = -40;	// Brightness after being burnt
long CSentry::ms_lMaxShootTime = MS_BETWEEN_BULLETS;		// Maximum in ms of continuous shooting.
long CSentry::ms_lReselectDudeTime	= 3000;	// Time to go without finding a dude
															// before calling SelectDude() to find
															// possibly a closer one.
U32 CSentry::ms_u32WeaponIncludeBits = CSmash::Character | CSmash::Barrel | CSmash::Misc;
U32 CSentry::ms_u32WeaponDontcareBits = CSmash::Good | CSmash::Bad;
U32 CSentry::ms_u32WeaponExcludeBits = CSmash::SpecialBarrel | CSmash::Ducking | CSmash::Bad | CSmash::Civilian;

// Let this auto-init to 0
short CSentry::ms_sFileCount;

/// Throwing Animation Files ////////////////////////////////////////////////////
// An array of pointers to resource names (one for each channel of the animation)
static char* ms_apszShootResNames[] = 
{
	"3d/sentry_shoot.sop",
	"3d/sentry_shoot.mesh",
	"3d/sentry_shoot.tex",
	"3d/sentry_shoot.hot",
	"3d/sentry_shoot.bounds",
	"3d/sentry_shoot.floor",
	"3d/sentry_shoot_tip.trans",
	NULL
};

static char* ms_apszStandResNames[] = 
{
	"3d/sentry_still.sop",
	"3d/sentry_still.mesh",
	"3d/sentry_still.tex",
	"3d/sentry_still.hot",
	"3d/sentry_still.bounds",
	"3d/sentry_still.floor",
	"3d/sentry_still_tip.trans",
	NULL
};

static char* ms_apszDieResNames[] = 
{
	"3d/sentry_damaged.sop",
	"3d/sentry_damaged.mesh",
	"3d/sentry_damaged.tex",
	"3d/sentry_damaged.hot",
	"3d/sentry_damaged.bounds",
	"3d/sentry_damaged.floor",
	"3d/sentry_damaged_tip.trans",
	NULL
};

static char* ms_apszBaseStandResNames[] = 
{
	"3d/stand_still.sop",
	"3d/stand_still.mesh",
	"3d/stand_still.tex",
	"3d/stand_still.hot",
	"3d/stand_still.bounds",
	"3d/stand_still.floor",
	"3d/stand_still_stand.trans",
	NULL
};

static char* ms_apszBaseDieResNames[] = 
{
	"3d/stand_damaged.sop",
	"3d/stand_damaged.mesh",
	"3d/stand_damaged.tex",
	"3d/stand_damaged.hot",
	"3d/stand_damaged.bounds",
	"3d/stand_damaged.floor",
	"3d/stand_damaged_stand.trans",
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
short CSentry::Load(				// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,						// In:  File to load from
	bool bEditMode,					// In:  True for edit mode, false otherwise
	short sFileCount,					// In:  File count (unique per file, never 0)
	ULONG	ulFileVersion)				// In:  Version of file format to load.
{
	short sResult = 0;
	// Call the base load function to get ID, position, etc.
	sResult = CDoofus::Load(pFile, bEditMode, sFileCount, ulFileVersion);
	if (sResult == 0)
	{
		// Load common data just once per file (not with each object)
		if (ms_sFileCount != sFileCount)
		{
			ms_sFileCount = sFileCount;

			CacheSample(g_smidShotSentry1);
			CacheSample(g_smidShotSentry2);
			CacheSample(g_smidShotSentry3);


			// Load static data		
			switch (ulFileVersion)
			{
				default:
				case 1:
					pFile->Read(&ms_dTooCloseDistance);
					pFile->Read(&ms_dLongRange);
					pFile->Read(&ms_dInRangeLow);
					pFile->Read(&ms_dInRangeHigh);
					pFile->Read(&ms_lRandomAvoidTime);
					pFile->Read(&ms_lReseekTime);
					pFile->Read(&ms_lWatchWaitTime);
					pFile->Read(&ms_lPatrolTime);
					break;
			}
		}

		// Load other values
		// for now, temporarily set values here to default values
		pFile->Read(&m_sNumRounds);
		pFile->Read(&m_sRoundsPerShot);
		pFile->Read(&m_lSqDistRange);
		pFile->Read(&m_lShootDelay);
		pFile->Read(&m_eWeaponType);

		if (ulFileVersion > 24)
			pFile->Read(&m_dAngularVelocity);
//		m_sNumRounds = 32000;
//		m_sRoundsPerShot = 2;
//		m_lSqDistRange = 280*280;
//		m_eWeaponType = CShotGunID;
//		m_eWeaponType = CShotGunID;
//		m_lShootDelay = 500;

		// Make sure there were no file errors or format errors . . .
		if (!pFile->Error() && sResult == 0)
		{
			// Get resources
			sResult = GetResources();
		}
		else
		{
			sResult = -1;
			TRACE("CSentry::Load(): Error reading from file!\n");
		}
	}
	else
	{
		TRACE("CSentry::Load():  CDoofus::Load() failed.\n");
	}

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Save object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CSentry::Save(										// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,											// In:  File to save to
	short sFileCount)										// In:  File count (unique per file, never 0)
{
	// Swap the hotspot we want to save in.
	double dTempX = m_dX;
	double dTempY = m_dY;
	double dTempZ = m_dZ;
	m_dX = m_dXBase;
	m_dY = m_dYBase;
	m_dZ = m_dZBase;

	short sResult;

	// Call the base class save to save the instance ID, position, etc
	CDoofus::Save(pFile, sFileCount);

	// Save common data just once per file (not with each object)
	if (ms_sFileCount != sFileCount)
	{
		ms_sFileCount = sFileCount;

		// Save static data
		pFile->Write(&ms_dTooCloseDistance);
		pFile->Write(&ms_dLongRange);
		pFile->Write(&ms_dInRangeLow);
		pFile->Write(&ms_dInRangeHigh);
		pFile->Write(&ms_lRandomAvoidTime);
		pFile->Write(&ms_lReseekTime);
		pFile->Write(&ms_lWatchWaitTime);
		pFile->Write(&ms_lPatrolTime);
	}

	// Save additinal stuff here.
	pFile->Write(&m_sNumRounds);
	pFile->Write(&m_sRoundsPerShot);
	pFile->Write(&m_lSqDistRange);
	pFile->Write(&m_lShootDelay);
	pFile->Write(&m_eWeaponType);
	pFile->Write(&m_dAngularVelocity);

	if (!pFile->Error())
	{
		sResult = SUCCESS;
	}
	else
	{
		TRACE("CSentry::Save() - Error writing to file\n");
		sResult = -1;
	}

	m_dX = dTempX;
	m_dY = dTempY;
	m_dZ = dTempZ;

	return sResult;
}

////////////////////////////////////////////////////////////////////////////////
// Render - Override to skip over CDoofus::Render right to CCharacter::Render
////////////////////////////////////////////////////////////////////////////////

void CSentry::Render(void)
{

	// Do our own render of the stationary base
	U16	u16CombinedAttributes;
	short	sLightTally;
	GetEffectAttributes(m_dXBase, m_dZBase, &u16CombinedAttributes, &sLightTally);

	// Brightness.
	m_spriteBase.m_sBrightness	= m_sBrightness + sLightTally * gsGlobalBrightnessPerLightAttribute;

	// If no parent . . .
	if (m_u16IdParent == CIdBank::IdNil)
		{
		// Reset transform back to start to set absolute rather than cummulative rotation
		m_trans.Make1();
//		m_transBase.Make1(); Not currently needed since the base does not change its transform.

		m_trans.Ry(rspMod360(m_dRot) );
		m_trans.Rz(rspMod360(m_dRotZ) );

		// Map from 3d to 2d coords
		Map3Dto2D((short) m_dXBase, (short) m_dYBase, (short) m_dZBase, &m_spriteBase.m_sX2, &m_spriteBase.m_sY2);

		// Layer should be based on info from attribute map.
		GetLayer(m_dXBase, m_dZBase, &(m_spriteBase.m_sLayer) );

		// Priority is based on bottom edge of sprite which is currently the origin
		m_spriteBase.m_sPriority = m_dZBase;

		// Update sprite in scene
		m_pRealm->m_scene.UpdateSprite(&m_spriteBase);
		
		// Set transform.
		m_spriteBase.m_ptrans = &m_transBase;
		}

	ASSERT(m_panimCurBase != NULL);

	m_spriteBase.m_pmesh = (RMesh*) m_panimCurBase->m_pmeshes->GetAtTime(m_lAnimTime);
	m_spriteBase.m_psop = (RSop*) m_panimCurBase->m_psops->GetAtTime(m_lAnimTime);
	m_spriteBase.m_ptex = (RTexture*) m_panimCurBase->m_ptextures->GetAtTime(m_lAnimTime);
	m_spriteBase.m_psphere = (RP3d*) m_panimCurBase->m_pbounds->GetAtTime(m_lAnimTime);

	CCharacter::Render();

	// The turret is always at a just higher priority than the base.
	m_sprite.m_sPriority	= m_spriteBase.m_sPriority + 1;
	m_sprite.m_sLayer		= m_spriteBase.m_sLayer;

	// Update sprite in scene
	m_pRealm->m_scene.UpdateSprite(&m_sprite);
		
}

////////////////////////////////////////////////////////////////////////////////
// Init - Call this after the resources are in place
////////////////////////////////////////////////////////////////////////////////

short CSentry::Init(void)
{
	short sResult = 0;

	// Prepare shadow (get resources and setup sprite).
	sResult	= PrepareShadow();

	// Init other stuff
	m_dVel = 0.0;
	m_dRot = 0.0;
	m_dShootAngle = 0.0;
	// Set to different starting state based on the design of the animation, but
	// for now, ok.  Then also set his current animation.
	m_state = CSentry::State_Wait;
	m_dAcc = ms_dAccUser;
	m_panimCur = &m_animStand;
	m_panimCurBase = &m_animBaseStand;
	m_lAnimTime = 0;
	m_lTimer = m_pRealm->m_time.GetGameTime() + 500;

	// Set up the animations that are supposed to loop.
	m_animShoot.m_psops->SetLooping(RChannel_LoopAtStart | RChannel_LoopAtEnd);
	m_animShoot.m_pmeshes->SetLooping(RChannel_LoopAtStart | RChannel_LoopAtEnd);
	m_animShoot.m_ptextures->SetLooping(RChannel_LoopAtStart | RChannel_LoopAtEnd);
//	m_animShoot.m_phots->SetLooping(RChannel_LoopAtStart | RChannel_LoopAtEnd);
	m_animShoot.m_pbounds->SetLooping(RChannel_LoopAtStart | RChannel_LoopAtEnd);

	// Set up the base sprite so we can get the position
	m_spriteBase.m_pmesh = (RMesh*) m_panimCurBase->m_pmeshes->GetAtTime(m_lAnimTime);
	m_spriteBase.m_psop = (RSop*) m_panimCurBase->m_psops->GetAtTime(m_lAnimTime);
	m_spriteBase.m_ptex = (RTexture*) m_panimCurBase->m_ptextures->GetAtTime(m_lAnimTime);
	m_spriteBase.m_psphere = (RP3d*) m_panimCurBase->m_pbounds->GetAtTime(m_lAnimTime);
	m_spriteBase.m_ptrans = (RTransform*) m_panimCurBase->m_ptransRigid->GetAtTime(m_lAnimTime);

	// Update base and turret position via m_dX, Y, & Z.
	UpdatePosition();

	m_stockpile.m_sHitPoints = ms_sHitLimit;

	m_smash.m_bits = CSmash::Bad | CSmash::Sentry;
	m_smash.m_pThing = this;

	m_sBrightness = 0;	// Default Brightness level

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Position the base and turret based on m_dX, Y, & Z.
////////////////////////////////////////////////////////////////////////////////
void CSentry::UpdatePosition(void)
	{
	// Move Base and Turret into position
	m_dXBase = m_dX;
	m_dYBase = m_dY;
	m_dZBase = m_dZ;

	if (m_panimCurBase != NULL)
		{
		// Below was copied from DetachChild in Thing3d

		// Update its position.
		// set up translation based on the combined last character and child transforms
		RTransform transChildAbsolute;
		RTransform*	ptransRigid	= m_panimCurBase->m_ptransRigid->GetAtTime(m_lAnimTime);
		
		// Apply child and parent to transChildAbs
		transChildAbsolute.Mul(m_trans.T, ptransRigid->T);
		// Set up pt at origin for child.
		RP3d pt3Src = {0, 0, 0, 1};
		RP3d pt3Dst;
		// Get last transition position by mapping origin.
		m_pRealm->m_scene.TransformPtsToRealm(&transChildAbsolute, &pt3Src, &pt3Dst, 1);
		// Set child position to character's position offset by rigid body's realm offset.
		m_dX = m_dXBase + pt3Dst.x;
		m_dY = m_dYBase + pt3Dst.y;
		m_dZ = m_dZBase + pt3Dst.z;
		}
	}


////////////////////////////////////////////////////////////////////////////////
// Startup object
////////////////////////////////////////////////////////////////////////////////
short CSentry::Startup(void)								// Returns 0 if successfull, non-zero otherwise
{
	short sResult = 0;

	// Set the current height, previous time, and Nav Net
	CDoofus::Startup();

	// Init other stuff
	Init();

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Shutdown object
////////////////////////////////////////////////////////////////////////////////
short CSentry::Shutdown(void)							// Returns 0 if successfull, non-zero otherwise
{
	short sResult = 0;

	m_trans.Make1();
	m_transBase.Make1();

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Suspend object
////////////////////////////////////////////////////////////////////////////////
void CSentry::Suspend(void)
{
	// Call base class suspend, and add anything else here if necessary
	CDoofus::Suspend();
}

////////////////////////////////////////////////////////////////////////////////
// Resume object
////////////////////////////////////////////////////////////////////////////////
void CSentry::Resume(void)
{
	// Call the base class resume and add anything else you suspended
	CDoofus::Resume();
}


////////////////////////////////////////////////////////////////////////////////
// Update object
////////////////////////////////////////////////////////////////////////////////
void CSentry::Update(void)
{
	short sHeight = m_sPrevHeight;
	long lThisTime;
	long lTimeDifference;
	long lSqDistanceToDude = 0;
	short sTargetAngle;
	short sAngleCCL;
	short sAngleCL;
	short sAngleDistance;
	double dRotDistance;
	bool bShootThisTime = false;

	if (!m_sSuspend)
	{
		// Get new time
		lThisTime = m_pRealm->m_time.GetGameTime();
		lTimeDifference = lThisTime - m_lPrevTime;

		// Calculate elapsed time in seconds
		double dSeconds = (double)(lThisTime - m_lPrevTime) / 1000.0;

		// Check for new messages that may change the state
		ProcessMessages();


		switch(m_state)
		{
			case CSentry::State_Wait:
				if (lThisTime > m_lTimer)
				{
					m_state = CSentry::State_Guard;
				}
	
				// Update sphere.
				m_smash.m_sphere.sphere.X			= m_dXBase;
				m_smash.m_sphere.sphere.Y			= m_dYBase;
				m_smash.m_sphere.sphere.Z			= m_dZBase;
				m_smash.m_sphere.sphere.lRadius	= 30; //m_spriteBase.m_sRadius;

				// Update the smash.
				m_pRealm->m_smashatorium.Update(&m_smash);
				break;

//-----------------------------------------------------------------------
// Guard - normal operation
//-----------------------------------------------------------------------

			case CSentry::State_Guard:

				// Target closest Dude
				SelectDude();

				sTargetAngle = CDoofus::FindDirection();
				sAngleCCL = rspMod360(sTargetAngle - m_dRot);
				sAngleCL  = rspMod360((360 - sTargetAngle) + m_dRot);
				sAngleDistance = MIN(sAngleCCL, sAngleCL);
				// Calculate the amount it can turn this time.
				dRotDistance = dSeconds * m_dAngularVelocity;
				// Don't over rotate - if it is within reach this time, then its OK to shoot
				if (sAngleDistance < dRotDistance)
				{
					dRotDistance = sAngleDistance;
					bShootThisTime = true;
				}
				if (sAngleCCL < sAngleCL)
				// Rotate Counter Clockwise
				{
					m_dShootAngle = rspMod360(m_dShootAngle + dRotDistance);
					m_dRot = m_dAnimRot = m_dShootAngle;
				}
				else
				// Rotate Clockwise
				{
					m_dShootAngle = rspMod360(m_dShootAngle - dRotDistance);
					m_dRot = m_dAnimRot = m_dShootAngle;
				}

				// Turn to him directly for now.
//				m_dRot = m_dAnimRot = m_dShootAngle = CDoofus::FindDirection();
				lSqDistanceToDude = CDoofus::SQDistanceToDude();

				if (bShootThisTime && 
				    m_idDude != CIdBank::IdNil && 
				    lSqDistanceToDude < m_lSqDistRange &&
					 lThisTime > m_lTimer &&
					 m_sNumRounds > 0)
				{

					if (TryClearShot(m_dShootAngle, 3))
					{
						m_panimCur = &m_animShoot;
						m_lAnimTime += lTimeDifference;
						CWeapon* pweapon = PrepareWeapon();
						if (pweapon != NULL)
							pweapon->SetRangeToTarget(rspSqrt(lSqDistanceToDude));
						ShootWeapon(ms_u32WeaponIncludeBits, ms_u32WeaponDontcareBits, ms_u32WeaponExcludeBits);
						m_sNumRounds--;
						m_lTimer = lThisTime + m_lShootDelay;
					}
					else
					{
						m_lAnimTime = 0;
						m_panimCur = &m_animStand;
					}
				}
				break;

//-----------------------------------------------------------------------
// Blownup - You were blown up so pop up into the air and come down dead
//-----------------------------------------------------------------------

				case CSentry::State_BlownUp:
					// Make her animate
					m_lAnimTime += lTimeDifference;

					if (!WhileBlownUp())
						m_state = State_Dead;
					else
					{
						if (lThisTime > m_lTimer && m_sNumRounds > 0)
						{
							m_dShootAngle = m_dRot;
							PrepareWeapon();
							ShootWeapon(ms_u32WeaponIncludeBits, ms_u32WeaponDontcareBits, ms_u32WeaponExcludeBits);
							m_sNumRounds--;
							m_lTimer = lThisTime + m_lShootDelay;
						}
						UpdateFirePosition();
					}

					break;


//-----------------------------------------------------------------------
// Dead - You are dead, so lay there and decompose, then go away
//-----------------------------------------------------------------------

				case CSentry::State_Dead:
					CHood*	phood	= m_pRealm->m_phood;
					// Render current dead frame into background to stay.
					m_pRealm->m_scene.DeadRender3D(
						phood->m_pimBackground,		// Destination image.
						&m_spriteBase,					// Tree of 3D sprites to render.
						phood);							// Dst clip rect.

					CDoofus::OnDead();
					delete this;
					return;
					break;


		}


		// Here's a little piece of CCharacter::Update() since this class
		// doesn't use the base class update.

		// If we have a weapon sound play instance . . .
		if (m_siLastWeaponPlayInstance)
			{
			// If time has expired . . .
			if (m_pRealm->m_time.GetGameTime() > m_lStopLoopingWeaponSoundTime)
				{
				// Stop looping the sound.
				StopLoopingSample(m_siLastWeaponPlayInstance);
				// Forget about it.
				m_siLastWeaponPlayInstance	= 0;
				}
			}

		// Save time for next time
		m_lPrevTime = lThisTime;
		m_lAnimPrevUpdateTime = m_lAnimTime;
	}
}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to init new object at specified position
////////////////////////////////////////////////////////////////////////////////
short CSentry::EditNew(									// Returns 0 if successfull, non-zero otherwise
	short sX,												// In:  New x coord
	short sY,												// In:  New y coord
	short sZ)												// In:  New z coord
{
	short sResult = 0;

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
	else
	{
		sResult = -1;
	}

	return sResult;
}

////////////////////////////////////////////////////////////////////////////////
// Edit Move
////////////////////////////////////////////////////////////////////////////////
short CSentry::EditMove(short sX, short sY, short sZ)
{
	short sResult = CDoofus::EditMove(sX, sY, sZ);

	UpdatePosition();

	return sResult;
}

////////////////////////////////////////////////////////////////////////////////
// Give Edit a rectangle around this object
////////////////////////////////////////////////////////////////////////////////
void CSentry::EditRect(RRect* pRect)
	{
	// Swap the hotspot and anim we want to get the rect of in.
	double dTempX = m_dX;
	double dTempY = m_dY;
	double dTempZ = m_dZ;
	
	m_dX = m_dXBase;
	m_dY = m_dYBase;
	m_dZ = m_dZBase;

	CAnim3D*	panimTemp	= m_panimCur;

	m_panimCur				= m_panimCurBase;

	// Call base class.
	CDoofus::EditRect(pRect);

	// Restore.
	m_dX	= dTempX;
	m_dY	= dTempY;
	m_dZ	= dTempZ;

	m_panimCur	= panimTemp;
	}

////////////////////////////////////////////////////////////////////////////////
// Called by editor to get the hotspot of an object in 2D.
// (virtual (Overridden here)).
////////////////////////////////////////////////////////////////////////////////
void CSentry::EditHotSpot(			// Returns nothiing.
	short*	psX,						// Out: X coord of 2D hotspot relative to
											// EditRect() pos.
	short*	psY)						// Out: Y coord of 2D hotspot relative to
											// EditRect() pos.
	{
	// Get rectangle.
	RRect	rc;
	EditRect(&rc);
	// Get 2D hotspot.
	short	sX;
	short	sY;
	Map3Dto2D(
		m_dXBase,
		m_dYBase,
		m_dZBase,
		&sX,
		&sY);

	// Get relation.
	*psX	= sX - rc.sX;
	*psY	= sY - rc.sY;
	}

////////////////////////////////////////////////////////////////////////////////
// Called by editor to modify object
////////////////////////////////////////////////////////////////////////////////
short CSentry::EditModify(void)
{
	short sResult = 0;
	RGuiItem* pGuiItem = NULL;
	RGuiItem* pGui = RGuiItem::LoadInstantiate(FullPathVD("res/editor/sentry.gui"));
	if (pGui)
	{
		RListBox* pWeaponList = (RListBox*) pGui->GetItemFromId(4);
		REdit* peditAmmoCount = (REdit*) pGui->GetItemFromId(101);
		REdit* peditShotDelay = (REdit*) pGui->GetItemFromId(102);
		REdit* peditRange     = (REdit*) pGui->GetItemFromId(103);
		REdit* peditRotVel    = (REdit*) pGui->GetItemFromId(104);
		if (	pWeaponList && peditAmmoCount && peditShotDelay && peditRange && peditRotVel) 
		{
			// Verify these are the type we think they are before accessing type specific
			// members.
			ASSERT(pWeaponList->m_type == RGuiItem::ListBox);
			ASSERT(peditAmmoCount->m_type == RGuiItem::Edit);
			ASSERT(peditShotDelay->m_type == RGuiItem::Edit);
			ASSERT(peditRange->m_type == RGuiItem::Edit);
			ASSERT(peditRotVel->m_type == RGuiItem::Edit);

#if 0
			// Show which weapon is currently selected
			pWeaponList->SetSel(pGui->GetItemFromId(m_eWeaponType));
#else
			// Empty list box.  We don't want to modify the .GUI resource just yet
			// so we don't screw up people using the current .EXE on the server.
			pWeaponList->RemoveAll();

			// Fill in the list box with current available weapons.
			short	i;
			for (i = 0; i < NumWeaponTypes; i++)
				{
				if (	(i != DeathWad || CStockPile::ms_sEnableDeathWad) &&
						(i != DoubleBarrel || CStockPile::ms_sEnableDoubleBarrel) &&
						(i != ProximityMine) &&
						(i != TimedMine) &&
						(i != RemoteControlMine) &&
						(i != BouncingBettyMine) )
					{
					pGuiItem	= pWeaponList->AddString(ms_awdWeapons[i].pszName);
					if (pGuiItem != NULL)
						{
						// Store class ID so we can determine user selection
						pGuiItem->m_lId	= ms_awdWeapons[i].id;
						}
					}
				}

			pWeaponList->AdjustContents();

			// Show which weapon is currently selected
			pGuiItem	= pGui->GetItemFromId(m_eWeaponType);
			if (pGuiItem)
				{
				pWeaponList->SetSel(pGuiItem);
				pWeaponList->EnsureVisible(pGuiItem);
				}

#endif

			// Set current Ammo Count
			peditAmmoCount->SetText("%d", m_sNumRounds);
			// Reflect changes
			peditAmmoCount->Compose();

			// Set current range
			peditRange->SetText("%d", rspSqrt(m_lSqDistRange));
			// Reflect changes
			peditRange->Compose();

			// Set current delay
			peditShotDelay->SetText("%d", m_lShootDelay);
			// Reflect changes
			peditShotDelay->Compose();

			// Set current rotational velocity
			peditRotVel->SetText("%d", (short) m_dAngularVelocity);
			peditRotVel->Compose();
				
			sResult = DoGui(pGui);
			if (sResult == 1)
			{
				{
					RGuiItem* pSelection = pWeaponList->GetSel();
					if (pSelection)
					{
						m_eWeaponType	= pSelection->m_lId;
					}

					m_sNumRounds = RSP_SAFE_GUI_REF(peditAmmoCount, GetVal());
					long lDist = RSP_SAFE_GUI_REF(peditRange, GetVal());
					m_lSqDistRange = lDist * lDist;
					m_lShootDelay = RSP_SAFE_GUI_REF(peditShotDelay, GetVal());
					m_dAngularVelocity = (double) RSP_SAFE_GUI_REF(peditRotVel, GetVal());
				}
			}
		}
	}
	delete pGui;

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Get all required resources
////////////////////////////////////////////////////////////////////////////////
short CSentry::GetResources(void)						// Returns 0 if successfull, non-zero otherwise
{
	short sResult = 0;

	sResult = m_animShoot.Get(ms_apszShootResNames);
	if (sResult == 0)
	{
		sResult = m_animStand.Get(ms_apszStandResNames);
		if (sResult == 0)
		{
			sResult = m_animDie.Get(ms_apszDieResNames);
			if (sResult == 0)
			{
				sResult = m_animBaseStand.Get(ms_apszBaseStandResNames);
				if (sResult == 0)
				{
					sResult = m_animBaseDie.Get(ms_apszBaseDieResNames);
					if (sResult == 0)
					{
						// Add new animation loads here
					}
					else
					{
						TRACE("CSentry::GetResources - Failed to load base damaged animation\n");
					}
				}
				else
				{
					TRACE("CSentry::GetResources - Failed to load base still animation\n");
				}
			}
			else
			{
				TRACE("CSentry::GetResources - Failed to load turret damaged animation\n");
			}
		}
		else
		{
			TRACE("CSentry::GetResources - Failed to open 3D turret still animation\n");
		}
	}
	else
	{
		TRACE("CSentry::GetResources - Failed to open 3D turret shoot animation\n");
	}

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Free all resources
////////////////////////////////////////////////////////////////////////////////
short CSentry::FreeResources(void)						// Returns 0 if successfull, non-zero otherwise
{
	m_animShoot.Release();
	m_animStand.Release();
	m_animDie.Release();
	m_animBaseStand.Release();
	m_animBaseDie.Release();

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Message handlers
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Shot Message
////////////////////////////////////////////////////////////////////////////////

void CSentry::OnShotMsg(Shot_Message* pMessage)
{
	// Audible and visual feedback.
	short sSound = GetRandom() % 3;

	switch (sSound)
	{
		case 0:
			PlaySample(g_smidShotSentry1, SampleMaster::Weapon);
			break;

		case 1:
			PlaySample(g_smidShotSentry2, SampleMaster::Weapon);
			break;

		case 2:
			PlaySample(g_smidShotSentry3, SampleMaster::Weapon);
			break;
	}

	// X/Z position depends on angle of shot (it is opposite).
	short	sDeflectionAngle	= rspMod360(pMessage->sAngle + 180);
	double	dHitX	= m_dX + COSQ[sDeflectionAngle] * HULL_RADIUS + RAND_SWAY(4);
	double	dHitZ	= m_dZ - SINQ[sDeflectionAngle] * HULL_RADIUS + RAND_SWAY(4);
	StartAnim(
		SENTRY_HIT_RES_NAME, 
		dHitX, 
		m_dY + RAND_SWAY(10),
		dHitZ,
		false);

	// Fow now we made the sentry bulletproof, the only
	// way it can be destroyed is by blowing it up.
}

////////////////////////////////////////////////////////////////////////////////
// Explosion message
////////////////////////////////////////////////////////////////////////////////

void CSentry::OnExplosionMsg(Explosion_Message* pMessage)
{
	if (
	    m_state != State_BlownUp	&&
		 m_state != State_Die		&& 
		 m_state != State_Dead)
	{
		CCharacter::OnExplosionMsg(pMessage);
		
		m_ePreviousState = m_state;
		m_state = State_BlownUp;
		m_panimPrev = m_panimCur;
		m_panimCur = &m_animDie;
		m_lAnimTime = 0;
		m_stockpile.m_sHitPoints = 0;
		m_lTimer = m_pRealm->m_time.GetGameTime();

		m_dExtHorzVel *= 1.4; //2.5;
		m_dExtVertVel *= 1.1; //1.4;
		// Send it spinning.
		m_dExtRotVelY	= GetRandom() % 720;
		m_dExtRotVelZ	= GetRandom() % 720;

		// Show the gun as damaged
		m_panimCurBase = &m_animBaseDie;
		m_panimCur = &m_animDie;
	}
}

////////////////////////////////////////////////////////////////////////////////
// Burning message
////////////////////////////////////////////////////////////////////////////////

void CSentry::OnBurnMsg(Burn_Message* pMessage)
{
	// For now we made the sentry fireproof, the only
	// way it can be destroyed is by blowing it up.
}


////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
