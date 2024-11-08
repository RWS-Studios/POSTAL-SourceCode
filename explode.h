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
// explode.h
// Project: Nostril (aka Postal)
// 
// History:
//		01/17/97 BRH	Started this weapon object.
//
//		02/24/97	JMI	Changed declaration of m_sprite from CAlphaSprite2 to 
//							CSprite2.
//
//		03/05/97 BRH	Added ms_sProjectVelocity as default velocity to throw
//							other objects nearby.
//
//		03/13/97	JMI	Load now takes a version number.
//
//		06/11/97 BRH	Added m_u16ShooterID to store the shooter as it
//							passes the information along to the explosion message.
//
//		07/09/97	JMI	Changed Preload() to take a pointer to the calling realm
//							as a parameter.
//
//		07/30/97	JMI	Added m_u16ExceptID (an ID to except when sending 
//							explosion messages).
//
//////////////////////////////////////////////////////////////////////////////
//
// Explosion.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef EXPLODE_H
#define EXPLODE_H

#include "RSPiX.h"
#include "realm.h"
#include "AlphaAnimType.h"
#include "smash.h"


// CExplode is a firey explosion weapon class
class CExplode : public CThing
	{
	//---------------------------------------------------------------------------
	// Types, enums, etc.
	//---------------------------------------------------------------------------
	public:

	typedef enum
	{
		State_Idle,
		State_Fire,
		State_Find,
		State_Chase,
		State_Explode
	} CExplodeState;

	typedef RChannel<CAlphaAnim> ChannelAA;

	//---------------------------------------------------------------------------
	// Variables
	//---------------------------------------------------------------------------
	public:
		double	m_dX;
		double	m_dY;
		double	m_dZ;
		U16		m_u16ShooterID;
		U16		m_u16ExceptID;									// ID of object to except from explosion.


	protected:
		int32_t m_lTimer;												// General purpose timer

		int32_t m_lPrevTime;											// Previous update time

		CSprite2		m_sprite;									// Sprite 
		ChannelAA*	m_pAnimChannel;							// Alpha Explosion animation stored as a channel

		int16_t m_sSuspend;											// Suspend flag

		CSmash		m_smash;										// Collision class

		// Tracks file counter so we know when to load/save "common" data 
		static int16_t ms_sFileCount;
		static int16_t ms_sBlastRadius;
		static int16_t ms_sProjectVelocity;

		// "Constant" values that we want to be able to tune using the editor

	//---------------------------------------------------------------------------
	// Constructor(s) / destructor
	//---------------------------------------------------------------------------
	public:
		// Constructor
		CExplode(CRealm* pRealm)
			: CThing(pRealm, CExplodeID)
			{
			m_sSuspend		= 0;
			m_u16ExceptID	= CIdBank::IdNil;
			}

	public:
		// Destructor
		~CExplode()
			{
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
		static int16_t Construct(									// Returns 0 if successfull, non-zero otherwise
			CRealm* pRealm,										// In:  Pointer to realm this object belongs to
			CThing** ppNew)										// Out: Pointer to new object
			{
			int16_t sResult = 0;
			*ppNew = new CExplode(pRealm);
			if (*ppNew == 0)
				{
				sResult = -1;
				TRACE("CExplode::Construct(): Couldn't construct CExplode (that's a bad thing)\n");
				}
			return sResult;
			}

	//---------------------------------------------------------------------------
	// Optional static functions
	//---------------------------------------------------------------------------

		static int16_t Preload(
			CRealm* prealm);				// In:  Calling realm.

	//---------------------------------------------------------------------------
	// Required virtual functions (implimenting them as inlines doesn't pay!)
	//---------------------------------------------------------------------------
	public:
		// Load object (should call base class version!)
		int16_t Load(													// Returns 0 if successfull, non-zero otherwise
			RFile* pFile,											// In:  File to load from
			bool bEditMode,										// In:  True for edit mode, false otherwise
			int16_t sFileCount,										// In:  File count (unique per file, never 0)
			uint32_t	ulFileVersion);								// In:  Version of file format to load.

		// Save object (should call base class version!)
		int16_t Save(													// Returns 0 if successfull, non-zero otherwise
			RFile* pFile,											// In:  File to save to
			int16_t sFileCount);									// In:  File count (unique per file, never 0)

		// Startup object
		int16_t Startup(void);										// Returns 0 if successfull, non-zero otherwise

		// Shutdown object
		int16_t Shutdown(void);									// Returns 0 if successfull, non-zero otherwise

		// Suspend object
		void Suspend(void);

		// Resume object
		void Resume(void);

		// Update object
		void Update(void);

		// Render object
		void Render(void);

		int16_t Setup(												// Returns 0 on success.
			int16_t sX,												// In: New x coord
			int16_t sY,												// In: New y coord
			int16_t sZ,												// In: New z coord
			U16	u16ShooterID,									// In: Who is responsible for this explosion
			int16_t sAnim = 0);										// In: Which explosion to use, standard = 0,
																		//     grenade = 1 etc.

		// Called by editor to init new object at specified position
		int16_t EditNew(												// Returns 0 if successfull, non-zero otherwise
			int16_t sX,												// In:  New x coord
			int16_t sY,												// In:  New y coord
			int16_t sZ);												// In:  New z coord

		// Called by editor to modify object
		int16_t EditModify(void);									// Returns 0 if successfull, non-zero otherwise

		// Called by editor to move object to specified position
		int16_t EditMove(											// Returns 0 if successfull, non-zero otherwise
			int16_t sX,												// In:  New x coord
			int16_t sY,												// In:  New y coord
			int16_t sZ);												// In:  New z coord

		// Called by editor to update object
		void EditUpdate(void);

		// Called by editor to render object
		void EditRender(void);

	//---------------------------------------------------------------------------
	// Internal functions
	//---------------------------------------------------------------------------
	protected:
		// Get all required resources
		int16_t GetResources(int16_t sAnim = 0);		// Returns 0 if successfull, non-zero otherwise
		
		// Free all resources
		int16_t FreeResources(void);						// Returns 0 if successfull, non-zero otherwise
	};


#endif //DOOFUS_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
