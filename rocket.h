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
// rocket.h
// Project: Postal
//
// History:
//		01/17/97 BRH	Started this weapon object.
//
//		02/26/97	JMI	Now sets m_sprite.m_pthing = this on construction.
//
//		03/03/97 BRH	Derived this from the CWeapon base class
//
//		03/03/97 BRH	Moved the 3D sprite to the CWeapon base class.
//
//		03/13/97	JMI	Load now takes a version number.
//
//		04/29/97 BRH	Added an off screen distance at which the rocket will self
//							destruct.  
//
//		04/29/97	JMI	Now defines m_sprite (as a CSprite3), which was previously
//							defined in the base class CWeapon.
//							Also, added GetSprite() virtual override to provide access
//							to the sprite from a lower level.
//
//		07/01/97 BRH	Added smoke timer for creating a trail of smoke.
//
//		07/09/97	JMI	Changed Preload() to take a pointer to the calling realm
//							as a parameter.
//
//		07/18/97	JMI	Added m_siThrust to track our thrust play instance so we
//							can loop it and then terminate the looping when we explode.
//
//		08/14/97 BRH	Added SetCollideBits function so that the Dude and Doofus
//							could set the collision bits differently.
//
//		08/23/97	JMI	Added CSmash::AlmostDead to exclude bits.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef ROCKET_H
#define ROCKET_H

#include "weapon.h"

// CRocket is an unguided missile weapon class
class CRocket : public CWeapon
	{
	//---------------------------------------------------------------------------
	// Types, enums, etc.
	//---------------------------------------------------------------------------
	public:

	//---------------------------------------------------------------------------
	// Variables
	//---------------------------------------------------------------------------
	public:
		// Collision bits
		U32 m_u32CollideIncludeBits;
		U32 m_u32CollideDontcareBits;
		U32 m_u32CollideExcludeBits;

	protected:
		CAnim3D		m_anim;					// 3D animation
		RTransform	m_trans;					// Transform
		CSmash		m_smash;
		bool			m_bArmed;				// Initially missile is not armed so it doesn't
													// collide with the person who shot it.
		CSprite3		m_sprite;				// 3D sprite to render this thing.
		long			m_lSmokeTimer;			// Time between emitting smoke puffs

		SampleMaster::SoundInstance	m_siThrust;	// Looping thrust play instance.

		// Tracks file counter so we know when to load/save "common" data 
		static short ms_sFileCount;

		// "Constant" values that we want to be able to tune using the editor
		static double ms_dAccUser;			// Acceleration due to user
		static double ms_dMaxVelFore;		// Maximum forward velocity
		static double ms_dMaxVelBack;		// Maximum backward velocity
		static double ms_dCloseDistance;	// Close enough to hit CDude
		static long ms_lArmingTime;		// Time before weapons arms.
		static short ms_sOffScreenDist;  // Distance off screen before self destructing
		static long ms_lSmokeTrailInterval;// Time between creating puffs of smoke
		static long ms_lSmokeTimeToLive;	  // Time for smoke to stick around.

	//---------------------------------------------------------------------------
	// Constructor(s) / destructor
	//---------------------------------------------------------------------------
	public:
		// Constructor
		CRocket(CRealm* pRealm)
			: CWeapon(pRealm, CRocketID)
			{
			m_sprite.m_pthing	= this;
			m_lSmokeTimer		= 0;
			m_siThrust			= 0;
			}

	public:
		// Destructor
		~CRocket()
			{
			// Stop sound, if any.
			StopLoopingSample(m_siThrust);

			// Remove sprite from scene (this is safe even if it was already removed!)
			m_pRealm->m_scene.RemoveSprite(&m_sprite);
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
			*ppNew = new CRocket(pRealm);
			if (*ppNew == 0)
				{
				sResult = -1;
				TRACE("CRocket::Construct(): Couldn't construct CRocket (that's a bad thing)\n");
				}
			return sResult;
			}

	//---------------------------------------------------------------------------
	// Optional static functions
	//---------------------------------------------------------------------------

		// Called before play begins to cache resources for this object
		static short Preload(
			CRealm* prealm);				// In:  Calling realm.

	public:
		void SetTransform(RTransform* pTransform)
			{
				m_sprite.m_ptrans = pTransform;
			};


	//---------------------------------------------------------------------------
	// Optional virtual functions
	//---------------------------------------------------------------------------
	public:

		virtual
		void SetCollideBits(					// Returns nothing
			U32 u32CollideBitsInclude,		// Bits considered in collision
			U32 u32CollideBitsDontCare,	// Bits ignored for collisions
			U32 u32CollideBitsExclude)		// Bits that invalidate a collision
		{
			m_u32CollideIncludeBits = u32CollideBitsInclude;
			m_u32CollideDontcareBits = u32CollideBitsDontCare;
			m_u32CollideExcludeBits	= u32CollideBitsExclude | CSmash::Ducking | CSmash::AlmostDead;
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

		// Update object
		void Update(void);

		// Render object
		void Render(void);

		// Called by another object to set start a new rocket
		short Setup(
			short sX,
			short sY,
			short sZ);

		// Get this class's sprite.  Note that the type will vary.
		// This is a pure virtual functionin the base class.
		virtual			// Overriden here.
		CSprite* GetSprite(void)	// Returns this weapon's sprite.
			{
			return &m_sprite;
			}

	//---------------------------------------------------------------------------
	// Internal functions
	//---------------------------------------------------------------------------
	protected:
		// Get all required resources
		short GetResources(void);						// Returns 0 if successfull, non-zero otherwise
		
		// Free all resources
		short FreeResources(void);						// Returns 0 if successfull, non-zero otherwise

		// Process messages in the message queue.
		void ProcessMessages(void);
	};



#endif //DOOFUS_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
