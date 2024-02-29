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
// deathWad.h
// Project: Nostril (aka Postal)
//
// History:
//		07/30/97 JMI	Started this weapon object from the CRocket.
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
#ifndef DEATHWAD_H
#define DEATHWAD_H

#include "weapon.h"
#include "StockPile.h"

// CDeathWad is an unguided projectile weapon class.
class CDeathWad : public CWeapon
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
		U32			m_u32CollideIncludeBits;
		U32			m_u32CollideDontcareBits;
		U32			m_u32CollideExcludeBits;
		CStockPile	m_stockpile;			// Arsenal.

	protected:
		CAnim3D		m_anim;					// 3D animation
		RTransform	m_trans;					// Transform
		CSmash		m_smash;					// Collision body.
		CSprite3		m_sprite;				// 3D sprite to render this thing.
		bool			m_bInsideTerrain;		// true if we are inside terrain.

		SampleMaster::SoundInstance	m_siThrust;	// Looping thrust play instance.
		double		m_dUnthrustedDistance;	// Distance since last thrust feedback.

		// Tracks file counter so we know when to load/save "common" data 
		static short ms_sFileCount;

		// Constant values for tuning.
		static const double	ms_dAccInternal;				// Internal acceleration.
		static const double	ms_dMaxVelFore;				// Maximum forward velocity
		static const double	ms_dMaxVelBack;				// Maximum backward velocity
		static const double	ms_dTraversalRate;			// Units moved each iteration while traversing the weapon path.
		static const double	ms_dThrustDelta;				// Distance between thrust feedbacks.
		static const short	ms_sOffScreenDist;			// Go off screen this far before blowing up		
		static const long		ms_lSmokeTimeToLive;			// Time for smoke to stick around.
		static const long		ms_lFireBallTimeToLive;		// Time for fireball to stick around.
		static const short	ms_sFinalExplosionStagger;	// Amount to stagger final explosions.
		static const short	ms_sCollisionRadius;			// Collision radius.
		static const double	ms_dKickVelocity;				// Velocity for kick from launch.
		static const CStockPile ms_stockpileMax;			// Max a WAD can hold.

	//---------------------------------------------------------------------------
	// Constructor(s) / destructor
	//---------------------------------------------------------------------------
	public:
		// Constructor
		CDeathWad(CRealm* pRealm)
			: CWeapon(pRealm, CDeathWadID)
			{
			m_sprite.m_pthing				= this;
			m_smash.m_pThing				= this;
			m_siThrust						= 0;
			m_u16ShooterID					= CIdBank::IdNil;
			m_stockpile.Zero();
			m_bInsideTerrain				= false;
			m_u32CollideIncludeBits		= 0;
			m_u32CollideDontcareBits	= 0;
			m_u32CollideExcludeBits		= 0;
			m_dUnthrustedDistance		= 0.0;
			}

	public:
		// Destructor
		~CDeathWad()
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
			*ppNew = new CDeathWad(pRealm);
			if (*ppNew == 0)
				{
				sResult = -1;
				TRACE("CDeathWad::Construct(): Couldn't construct CDeathWad (that's a bad thing)\n");
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

		// Called by another object to set start a new deathwad
		short Setup(
			short sX,
			short sY,
			short sZ);

		// Get this class's sprite.  Note that the type will vary.
		// This is a pure virtual function in the base class.
		virtual			// Overriden here.
		CSprite* GetSprite(void)	// Returns this weapon's sprite.
			{
			return &m_sprite;
			}

		// Feed the WAD prior to moving its state to State_Fire.
		void FeedWad(
			CStockPile*	pstockpile);	// In:  Src for WAD's arsenal.


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

		// Traverse the path until the inside terrain status changes or
		// the destination is reached.
		bool TraversePath(			// Returns true, when destination reached; false, 
											// if terrain change.
			short		sSrcX,			// In:  Starting position.
			short		sSrcY,			// In:  Starting position.
			short		sSrcZ,			// In:  Starting position.
			bool*		pbInTerrain,	// In:  true, if starting in terrain.
											// Out: true, if ending in terrain.
			short		sDstX,			// In:  Destination position.
			short		sDstZ,			// In:  Destination position.
			double*	pdCurX,			// Out: Position of inside terrain status change.
			double*	pdCurZ);			// Out: Position of inside terrain status change.

		// Generate an explosion at the current position.
		void Explosion(void);

		// Generate some thrust at the current position.
		void Thrust(void);

		// Generate the launch kick/debris.
		void Launch(void);

	};



#endif //DOOFUS_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
