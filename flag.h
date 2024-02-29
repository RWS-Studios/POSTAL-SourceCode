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
// flag.h
// Project: Postal
//
//	History:
//	
//		06/30/97 BRH	Started this file to contain the Capture the flag object.
//							The flag is the thing that can be carried to the base.
//
//		07/06/97 BRH	Added Time bonus for the time bonus challenge levels.
//
//		07/12/97 BRH	Added m_u16FlagID to match flags and bases.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef FLAG_H
#define FLAG_H

#include "RSPiX.h"
#include "realm.h"
#include "Thing3d.h"

// CFlag is the flag object for capture the flag challenge levels
class CFlag : public CThing3d
{
	//---------------------------------------------------------------------------
	// Types, enums, etc.
	//---------------------------------------------------------------------------
	public:

		typedef enum
		{
			Red = 0,
			Blue,

			EndOfColors
		} FlagColor;

	//---------------------------------------------------------------------------
	// Variables
	//---------------------------------------------------------------------------
	public:
		CAnim3D*		m_panimCurBase;					// current animation for the base
		U16			m_u16FlagID;						// Used to match flag & base

	protected:
		CAnim3D		m_animFlagWave;					// animation for the flag waving
		U32			m_u32IncludeBits;					// Bits to include for Smash collision
		U32			m_u32DontcareBits;				// Bits to ignore for Smash collision
		U32			m_u32ExcludeBits;					// Bits to exclude for Smash collision
		long			m_lTimeBonus;						// Flag stores a time bonus for
																// special game play modes.
		U16			m_u16FlagColor;					// Color of flag;
		short			m_sSavedX;							// Save the position of the flagbase
		short			m_sSavedY;							// Save the position of the flagbase
		short			m_sSavedZ;							// Save the position of the flagbase

		// Tracks file counter so we know when to load/save "common" data 
		static short ms_sFileCount;

		// "Constant" values that we want to be able to tune using the editor
		static double ms_dInRange;				// In range to the base

	//---------------------------------------------------------------------------
	// Constructor(s) / destructor
	//---------------------------------------------------------------------------
	protected:
		// Constructor
		CFlag(CRealm* pRealm)
			: CThing3d(pRealm, CFlagID)
			{
			m_sSuspend = 0;
			m_dRot = 0;
			m_dX = m_dY = m_dZ = m_dVel = m_dAcc = 0;
			m_panimCur = NULL;
			m_sprite.m_pthing	= this;
			m_u16FlagID = 1;
			m_lTimeBonus = 0;
			m_u16FlagColor = Red;
			m_sSavedX = 0;
			m_sSavedY = 0;
			m_sSavedZ = 0;
			}

	public:
		// Destructor
		~CFlag()
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
		static short Construct(									// Returns 0 if successfull, non-zero otherwise
			CRealm* pRealm,										// In:  Pointer to realm this object belongs to
			CThing** ppNew)										// Out: Pointer to new object
			{
			short sResult = 0;
			*ppNew = new CFlag(pRealm);
			if (*ppNew == 0)
				{
				sResult = -1;
				TRACE("CFlag::Construct(): Couldn't construct CFlag (that's a bad thing)\n");
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

		// Update object
		void Update(void);

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


#endif //FLAG_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
