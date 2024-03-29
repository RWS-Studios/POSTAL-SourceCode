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
// flagbase.h
// Project: Postal
//
//	History:
//	
//		06/30/97 BRH	Started this file to contain the base object for the
//							capture the flag mode.  When the flag meets the base, 
//							the game is complete.
//
//		07/12/97 BRH	Added m_u16FlagID to match flags with bases.
//
//		08/11/97 BRH	Added color variable which is loaded and saved.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef FLAGBASE_H
#define FLAGBASE_H

#include "RSPiX.h"
#include "realm.h"
#include "Thing3d.h"

// CFlagbasebase is the base where the flag goes in capture the flag challenge levels
class CFlagbase : public CThing3d
{
	//---------------------------------------------------------------------------
	// Types, enums, etc.
	//---------------------------------------------------------------------------
	protected:

	//---------------------------------------------------------------------------
	// Variables
	//---------------------------------------------------------------------------
	public:
		CAnim3D*		m_panimCurBase;					// current animation for the base
		U16			m_u16FlagID;						// ID to match flag and base

	protected:
		CAnim3D		m_animFlagWave;					// animation for the flag waving
		U16			m_u16Color;							// Color of base


		// Tracks file counter so we know when to load/save "common" data 
		static int16_t ms_sFileCount;

		// "Constant" values that we want to be able to tune using the editor
		static double ms_dInRange;				// In range to the base

	//---------------------------------------------------------------------------
	// Constructor(s) / destructor
	//---------------------------------------------------------------------------
	protected:
		// Constructor
		CFlagbase(CRealm* pRealm)
			: CThing3d(pRealm, CFlagbaseID)
			{
			m_sSuspend = 0;
			m_dRot = 0;
			m_dX = m_dY = m_dZ = m_dVel = m_dAcc = 0;
			m_panimCur = NULL;
			m_sprite.m_pthing	= this;
			m_u16FlagID = 1;
			m_u16Color = 0;
			}

	public:
		// Destructor
		~CFlagbase()
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
			*ppNew = new CFlagbase(pRealm);
			if (*ppNew == 0)
				{
				sResult = -1;
				TRACE("CFlagbase::Construct(): Couldn't construct CFlagbase (that's a bad thing)\n");
				}
			return sResult;
			}

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

		// Update object
		void Update(void);

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

		// Give Edit a rectangle around this object
		void EditRect(RRect* pRect);

		// Called by editor to get the hotspot of an object in 2D.
		// (virtual (Overridden here)).
		void EditHotSpot(			// Returns nothiing.
			int16_t*	psX,			// Out: X coord of 2D hotspot relative to
										// EditRect() pos.
			int16_t*	psY);			// Out: Y coord of 2D hotspot relative to
										// EditRect() pos.

	//---------------------------------------------------------------------------
	// Internal functions
	//---------------------------------------------------------------------------
	protected:
		// Get all required resources
		int16_t GetResources(void);						// Returns 0 if successfull, non-zero otherwise
		
		// Free all resources
		int16_t FreeResources(void);						// Returns 0 if successfull, non-zero otherwise

		// Initialize states, positions etc.
		int16_t Init(void);

		// Update the animation radius based on the current frame
		void UpdateRadius(void);

		// Message handling functions ////////////////////////////////////////////

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


#endif //FLAGBASE_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
