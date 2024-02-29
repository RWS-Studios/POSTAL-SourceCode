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
// napalm.h
// Project: Nostril (aka Postal)
//
// History:
//		01/17/97 BRH	Started this weapon object.
//
//		02/26/97	JMI	Now sets m_sprite.m_pthing = this on construction.
//
//		03/03/97 BRH	Changed to be derived from CWeapon base class
//
//		03/03/97 BRH	Moved the 3D sprite to the CWeapon base class.
//
//		03/03/97	JMI	Commented out dHorizVelocity and dVertVelocity parameters
//							to Setup() so that this version would be a virtual over-
//							ride of CWeapon's.
//
//		03/13/97	JMI	Load now takes a version number.
//
//		04/29/97	JMI	Now defines m_sprite (as a CSprite3), which was previously
//							defined in the base class CWeapon.
//							Also, added GetSprite() virtual override to provide access
//							to the sprite from a lower level.
//
//		05/09/97 BRH	Added SetRangeToTarget function to vary the velocity
//							of the weapon before it is shot in order to hit
//							your target.  
//
//		06/17/97 BRH	Fixed a bug in SetRangeToTarget where the min was not
//							being limited correctly.
//
//		07/09/97	JMI	Changed Preload() to take a pointer to the calling realm
//							as a parameter.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef NAPALM_H
#define NAPALM_H

#include "weapon.h"

// CNapalm is a canister of napalm weapon class
class CNapalm : public CWeapon
	{
	//---------------------------------------------------------------------------
	// Types, enums, etc.
	//---------------------------------------------------------------------------
	public:

	//---------------------------------------------------------------------------
	// Variables
	//---------------------------------------------------------------------------
	public:
//		CSprite3		m_sprite;					// 3D Sprite

	protected:
		double m_dFireX;							// Last position of fire for comparision
		double m_dFireZ;							// Last position of fire for comparision
											
		CAnim3D		m_anim;						// 3D animation
		RTransform	m_trans;						// Transform
		CSprite3		m_sprite;					// 3D sprite to render this thing.

		// Tracks file counter so we know when to load/save "common" data 
		static short ms_sFileCount;

		// "Constant" values that we want to be able to tune using the editor
		static double ms_dAccDrag;				// Acceleration due to drag (always towards 0)
		static long   ms_lGrenadeFuseTime;	// Time from throw to blow
		static double ms_dThrowVertVel;		// Throw up at this velocity
		static double ms_dThrowHorizVel;		// Throw out at this velocity
		static double ms_dMinFireInterval;	// Lay fire down every this amount of distance

	//---------------------------------------------------------------------------
	// Constructor(s) / destructor
	//---------------------------------------------------------------------------
	protected:
		// Constructor
		CNapalm(CRealm* pRealm)
			: CWeapon(pRealm, CNapalmID)
			{
			m_sprite.m_pthing	= this;
			}

	public:
		// Destructor
		~CNapalm()
			{
			// Remove sprite from scene (this is safe even if it was already removed!)
			m_pRealm->m_scene.RemoveSprite(&m_sprite);

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
			*ppNew = new CNapalm(pRealm);
			if (*ppNew == 0)
				{
				sResult = -1;
				TRACE("CNapalm::Construct(): Couldn't construct CNapalm (that's a bad thing)\n");
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

		// Called by the object that is creating this weapon
		short Setup(
			short sX,												// In: New x coord
			short sY,												// In: New y coord
			short sZ/*,												// In: New z coord
			double dHorizVel = ms_dThrowHorizVel,			// In: Starting horizontal velocity
			double dVertVel = ms_dThrowVertVel*/);			// In: Starting vertical velocity

		// Get this class's sprite.  Note that the type will vary.
		// This is a pure virtual functionin the base class.
		virtual			// Overriden here.
		CSprite* GetSprite(void)	// Returns this weapon's sprite.
			{
			return &m_sprite;
			}

		// Function to modify the velocity for a requested range
		virtual short SetRangeToTarget(short sRequestedRange)
		{
			short sSetRange;
			// Must go at least 30 or at most 400 pixels
			sSetRange = MAX(sRequestedRange, (short) 30);
			sSetRange = MIN(sSetRange, (short) 400);
			m_dHorizVel = (double) sSetRange / 0.8544; //0.6782;
			return sSetRange;
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
