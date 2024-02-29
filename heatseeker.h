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
// heatseeker.h
// Project: Postal
//
// History:
//		05/13/97 BRH	Started this weapon object fromt the CHeatseeker code.
//
//		07/01/97 BRH	Added smoke timer for making smoke trails.
//
//		07/09/97	JMI	Changed Preload() to take a pointer to the calling realm
//							as a parameter.
//
//		08/14/97 BRH	Added SetCollideBits and the collision bit fields so that
//							they can be set by the Doofus or Dude when they shoot it
//							and they can collide differently, rather than the
//							standard default behavoir.
//
//		08/17/97 BRH	Added thrust sound instance so it is like the rocket.
//
//		08/23/97	JMI	Added CSmash::AlmostDead to exclude bits.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef HEATSEEKER_H
#define HEATSEEKER_H

#include "weapon.h"

// CHeatseeker is a heat seeking missile
class CHeatseeker : public CWeapon
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

	protected:
		CAnim3D		m_anim;					// 3D animation
		RTransform	m_trans;					// Transform
		CSmash		m_smash;					// Smash used for explosion collisions (small)
		CSmash		m_smashSeeker;			// Smash used to detect heat sources (larger)
		bool			m_bArmed;				// Initially missile is not armed so it doesn't
													// collide with the person who shot it.
		CSprite3		m_sprite;				// 3D sprite to render this thing.
		long			m_lSmokeTimer;			// Time to wait between emitting smoke
		U32			m_u32CollideBitsInclude;	// Bits that cause a collision
		U32			m_u32CollideBitsDontCare;	// Bits that are ignored for collisions
		U32			m_u32CollideBitsExclude;	// Bits that invalidate a collision


		U32			m_u32SeekBitsInclude;		// Bits that cause a collision
		U32			m_u32SeekBitsDontCare;		// Bits that are ignored for collisions
		U32			m_u32SeekBitsExclude;		// bits taht invalidate a collision

		SampleMaster::SoundInstance m_siThrust; // Looping thrust play instance

		// Tracks file counter so we know when to load/save "common" data 
		static short ms_sFileCount;

		// "Constant" values that we want to be able to tune using the editor
		static double ms_dAccUser;			// Acceleration due to user
		static double ms_dMaxVelFore;		// Maximum forward velocity
		static double ms_dMaxVelBack;		// Maximum backward velocity
		static double ms_dCloseDistance;	// Close enough to hit CDude
		static double ms_dLineCheckRate;	// Pixel distance for line checking
		static long ms_lArmingTime;		// Time before weapons arms.
		static long ms_lSeekRadius;		// Radius of Heatseeking circle
		static short ms_sOffScreenDist;  // Distance off screen before self destructing
		static short ms_sAngularVelocity;// Degrees per second that it can turn
		static U32 ms_u32CollideIncludeBits;
		static U32 ms_u32CollideDontcareBits;
		static U32 ms_u32CollideExcludeBits;
		static U32 ms_u32SeekIncludeBits;
		static U32 ms_u32SeekDontcareBits;
		static U32 ms_u32SeekExcludeBits;
		static long ms_lSmokeTrailInterval;	// Time between smoke releases
		static long ms_lSmokeTimeToLive;		// Time for smoke to stick around.

	//---------------------------------------------------------------------------
	// Constructor(s) / destructor
	//---------------------------------------------------------------------------
	public:
		// Constructor
		CHeatseeker(CRealm* pRealm)
			: CWeapon(pRealm, CHeatseekerID)
			{
			m_sprite.m_pthing	= this;
			m_lSmokeTimer = 0;
			m_siThrust = 0;
			}

	public:
		// Destructor
		~CHeatseeker()
			{
			// Stop sound if any
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
			*ppNew = new CHeatseeker(pRealm);
			if (*ppNew == 0)
				{
				sResult = -1;
				TRACE("CHeatseeker::Construct(): Couldn't construct CHeatseeker (that's a bad thing)\n");
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
		
		// Used to set the collision bit fields
		virtual
		void SetCollideBits(	   // Returns nothing
			U32 u32BitsInclude,	// Bits to detect in collisions
			U32 u32BitsDontCare,	// Bits that don't matter for collision detection
			U32 u32BitsExclude)	// Bits that invalidate collision
		{
			m_u32CollideBitsInclude = u32BitsInclude | CSmash::Fire;
			m_u32CollideBitsDontCare = u32BitsDontCare;
			m_u32CollideBitsExclude = u32BitsExclude | CSmash::Ducking | CSmash::AlmostDead;
		}

		// Used to set the detection bit fields
		virtual
		void SetDetectionBits(	// Returns nothing
			U32 u32BitsInclude,	// Bits to detect in collisions
			U32 u32BitsDontcare,	// Bits that don't matter for collision detection
			U32 u32BitsExclude)	// Bits that invalidate collision
		{
			m_u32SeekBitsInclude = u32BitsInclude | CSmash::Fire;
			m_u32SeekBitsDontCare = u32BitsDontcare;
			m_u32SeekBitsExclude = u32BitsExclude | CSmash::Ducking | CSmash::AlmostDead;
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

		inline short FindAngleTo(double dX, double dZ)
		{
			return rspATan((m_dZ - dZ), (dX - m_dX));	
		}
	};



#endif //HEATSEEKER_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
