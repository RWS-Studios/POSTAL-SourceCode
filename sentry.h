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
// sentry.h
// Project: Postal
//
//	History:
//		06/02/97 BRH	Created this sentry gun from gunner.h
//
//		06/30/97	JMI	Added override for EditRect() and EditHotSpot().
//							Also, added UpdatePosition().
//
//		07/01/97 BRH	Added angular velocity for Sentry gun.
//
//		08/11/97	JMI	Added transform for base, m_transBase.
//							Also, added initialization of m_panimCurBase to NULL.
//
//		08/16/97 BRH	Added weapon bits to pass to ShootWeapon.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef SENTRY_H
#define SENTRY_H

#include "RSPiX.h"
#include "realm.h"
#include "doofus.h"

// CSentry is the class for the enemy guys
class CSentry : public CDoofus
{
	//---------------------------------------------------------------------------
	// Types, enums, etc.
	//---------------------------------------------------------------------------
	protected:

	//---------------------------------------------------------------------------
	// Variables
	//---------------------------------------------------------------------------
	public:
		CSprite3		m_spriteBase;						// Base of gun that is stationary
		CAnim3D*		m_panimCurBase;					// current animation for the base
		double		m_dXBase;							// Position of base
		double		m_dYBase;							// Position of base
		double		m_dZBase;							// Position of base
		RTransform	m_transBase;						// Base's rotation/scaling/translation.

	protected:
		CAnim3D		m_animBaseStand;					// animation for the base of the gun
		CAnim3D		m_animBaseDie;						// animation for the base of the gun

		CBulletFest	m_bullets;							// Generic bullet interface.
		long			m_lLastBulletTime;				// Last time a bullet was fired.
		short			m_sNumRounds;						// Number of rounds remaining in the gun
		short			m_sRoundsPerShot;					// How many bullets does it fire at once
		long			m_lSqDistRange;					// Range in pixels squared.
		long			m_lShootDelay;						// Time to wait between shots
		double		m_dAngularVelocity;				// Amount it can turn in degrees/second

		// Tracks file counter so we know when to load/save "common" data 
		static short ms_sFileCount;

		// "Constant" values that we want to be able to tune using the editor
		static double ms_dTooCloseDistance;	// Close enough to hit CDude
		static double ms_dLongRange;			// Out of range to notice or care
		static double ms_dInRangeLow;			// Within weapon range
		static double ms_dInRangeHigh;		// within weapon range
		static double ms_dGravity;				// Gravity for falling
		static double ms_dBlowupVelocity;	// Initial blast velocity
		static long ms_lRandomAvoidTime;		// Time to wander before looking again
		static long ms_lReseekTime;			// Time to wait before doing the next 'find'
		static long ms_lWatchWaitTime;		// Watch your shot go
		static long ms_lPatrolTime;			// Wait this long before shooting
		static long ms_lDeathTimeout;			// Wait around after dying
		static long ms_lBurningRunTime;		// Run this time before turning
		static short ms_sHitLimit;				// Number of starting hit points
		static short ms_sBurntBrightness;	// Brightness level when burnt
		static long ms_lMaxShootTime;			// Maximum in ms of continuous shooting.
		static long ms_lReselectDudeTime;	// Time before looking for a closer dude.
		static U32 ms_u32WeaponIncludeBits;	// Weapons shot from sentry can hit this
		static U32 ms_u32WeaponDontcareBits;// Weapons shot from sentry ignore these bits
		static U32 ms_u32WeaponExcludeBits;	// Weapons shot from sentry do not hit this

	//---------------------------------------------------------------------------
	// Constructor(s) / destructor
	//---------------------------------------------------------------------------
	protected:
		// Constructor
		CSentry(CRealm* pRealm)
			: CDoofus(pRealm, CSentryID)
			{
			m_sSuspend = 0;
			m_dRot = 0;
			m_dX = m_dY = m_dZ = m_dVel = m_dAcc = 0;
			m_panimCur = m_panimPrev = NULL;
			m_panimCurBase	= NULL;
			m_sprite.m_pthing	= this;
			m_sNumRounds = 0;
			m_sRoundsPerShot = 0;
			m_lSqDistRange = 0;
			m_lShootDelay = 0;
			m_dAngularVelocity = 360.0;
			}

	public:
		// Destructor
		~CSentry()
			{
			// Remove sprite from scene (this is safe even if it was already removed!)
			m_pRealm->m_scene.RemoveSprite(&m_sprite);
			m_pRealm->m_scene.RemoveSprite(&m_spriteBase);
			m_pRealm->m_smashatorium.Remove(&m_smash);

			// Free resources
			FreeResources();
			}

	//---------------------------------------------------------------------------
	// Required static functions
	//---------------------------------------------------------------------------
	public:
		// Construct object
		static short Construct(									// Returns 0 if successfull, non-zero otherwise
			CRealm* pRealm,										// In:  Pointer to realm this object belongs to
			CThing** ppNew)										// Out: Pointer to new object
			{
			short sResult = 0;
			*ppNew = new CSentry(pRealm);
			if (*ppNew == 0)
				{
				sResult = -1;
				TRACE("CSentry::Construct(): Couldn't construct CSentry (that's a bad thing)\n");
				}
			return sResult;
			}

	//---------------------------------------------------------------------------
	// Required virtual functions (implimenting them as inlines doesn't pay!)
	//---------------------------------------------------------------------------
	public:
		// Load object (should call base class version!)
		short Load(													// Returns 0 if successfull, non-zero otherwise
			RFile* pFile,											// In:  File to load from
			bool bEditMode,										// In:  True for edit mode, false otherwise
			short sFileCount,										// In:  File count (unique per file, never 0)
			ULONG	ulFileVersion);								// In:  Version of file format to load.

		// Save object (should call base class version!)
		short Save(													// Returns 0 if successfull, non-zero otherwise
			RFile* pFile,											// In:  File to save to
			short sFileCount);									// In:  File count (unique per file, never 0)

		// Startup object
		short Startup(void);										// Returns 0 if successfull, non-zero otherwise

		// Shutdown object
		short Shutdown(void);									// Returns 0 if successfull, non-zero otherwise

		// Suspend object
		void Suspend(void);

		// Resume object
		void Resume(void);

		// Render override
		void Render(void);

		// Update object
		void Update(void);

		// Find angle to a CDude
		short FindDirection(void);

		// Find the squared distance to the CDude (to avoid sqrt)
		double SQDistanceToDude(void);

		// Called by editor to init new object at specified position
		short EditNew(												// Returns 0 if successfull, non-zero otherwise
			short sX,												// In:  New x coord
			short sY,												// In:  New y coord
			short sZ);												// In:  New z coord

		// Called by editor to modify object
		short EditModify(void);									// Returns 0 if successfull, non-zero otherwise

		// Called by editor to move object to specified position
		short EditMove(											// Returns 0 if successfull, non-zero otherwise
			short sX,												// In:  New x coord
			short sY,												// In:  New y coord
			short sZ);												// In:  New z coord

		// Give Edit a rectangle around this object
		void EditRect(RRect* pRect);

		// Called by editor to get the hotspot of an object in 2D.
		// (virtual (Overridden here)).
		void EditHotSpot(			// Returns nothiing.
			short*	psX,			// Out: X coord of 2D hotspot relative to
										// EditRect() pos.
			short*	psY);			// Out: Y coord of 2D hotspot relative to
										// EditRect() pos.

	//---------------------------------------------------------------------------
	// Internal functions
	//---------------------------------------------------------------------------
	protected:
		// Get all required resources
		short GetResources(void);						// Returns 0 if successfull, non-zero otherwise
		
		// Free all resources
		short FreeResources(void);						// Returns 0 if successfull, non-zero otherwise

		// Initialize states, positions etc.
		short Init(void);

		// Update the animation radius based on the current frame
		void UpdateRadius(void);

		// Position the base and turret based on m_dX, Y, & Z.
		void UpdatePosition(void);

		// Message handling functions ////////////////////////////////////////////

		// Handles a Shot_Message.
		virtual			// Override to implement additional functionality.
							// Call base class to get default functionality.
		void OnShotMsg(					// Returns nothing.
			Shot_Message* pshotmsg);	// In:  Message to handle.

		// Handles an Explosion_Message.
		virtual			// Override to implement additional functionality.
							// Call base class to get default functionality.
		void OnExplosionMsg(							// Returns nothing.
			Explosion_Message* pexplosionmsg);	// In:  Message to handle.

		// Handles a Burn_Message.
		virtual			// Override to implement additional functionality.
							// Call base class to get default functionality.
		void OnBurnMsg(					// Returns nothing.
			Burn_Message* pburnmsg);	// In:  Message to handle.

};


#endif //SENTRY_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
