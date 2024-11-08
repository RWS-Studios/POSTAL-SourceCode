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
// ostrich.h
// Project: Nostril (aka Postal)
//
// History:
//
//		05/09/97 BRH	Started the ostrich object from the band file.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef OSTRICH_H
#define OSTRICH_H

#include "RSPiX.h"
#include "doofus.h"

// COstrich is the object for the ostriches wandering about in the game.
class COstrich : public CDoofus
{
	//---------------------------------------------------------------------------
	// Types, enums, etc.
	//---------------------------------------------------------------------------
	protected:

	//---------------------------------------------------------------------------
	// Variables
	//---------------------------------------------------------------------------
	protected:
		CCharacter::State m_ePreviousState;	// State variable to remember what he was
														// Doing before he was shot, etc.
		CAnim3D*	m_pPreviousAnim;				// Previous state's animation

		CAnim3D m_animStand;						// Stand animation
		CAnim3D m_animHide;						// Hide head in sand
		CAnim3D m_animWalk;						// Marching animation
		CAnim3D m_animRun;						// Running away animation
		CAnim3D m_animShot;						// Shot dead animation
		CAnim3D m_animBlownup;					// Blown up by explosion
		CAnim3D m_animDie;						// Fall down dead

		int16_t m_sRotDirection;					// Which direction to rotate to avoid walls

		// Tracks file counter so we know when to load/save "common" data 
		static int16_t ms_sFileCount;

		// "Constant" values that we want to be able to tune using the editor
		static double ms_dExplosionVelocity;// How high he will get blown up.
		static double ms_dMaxMarchVel;		// How fast to march
		static double ms_dMaxRunVel;			// Hos fast to run
		static int32_t ms_lStateChangeTime;		// How long to go before changing states
		static int16_t ms_sStartingHitPoints;	// How many hit points to start with
	
	//---------------------------------------------------------------------------
	// Constructor(s) / destructor
	//---------------------------------------------------------------------------
	protected:
		// Constructor
		COstrich(CRealm* pRealm)
			: CDoofus(pRealm, COstrichID)
			{
				m_sRotDirection = 0;
			}

	public:
		// Destructor
		~COstrich()
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
			*ppNew = new COstrich(pRealm);
			if (*ppNew == 0)
				{
				sResult = -1;
				TRACE("COstrich::Construct(): Couldn't construct COstrich (that's a bad thing)\n");
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

		// Update object
		void Update(void);

		// Render object
		void Render(void);

		// Called by editor when a new object is created
		int16_t EditNew(int16_t sX, int16_t sY, int16_t sZ);

		// Called by editor to modify object
		int16_t EditModify(void);									// Returns 0 if successfull, non-zero otherwise
		// Called by editor to render object
//		void EditRender(void);

	//---------------------------------------------------------------------------
	// Message handlers that are called by CCharacter ProcessMessage().  These
	// have code to set the correct animation, state, etc for these messages
	//---------------------------------------------------------------------------
	public:

		void OnShotMsg(Shot_Message* pMessage);

		void OnBurnMsg(Burn_Message* pMessage);

		void OnExplosionMsg(Explosion_Message* pMessage);

		void OnPanicMsg(Panic_Message* pMessage);

	//---------------------------------------------------------------------------
	// Useful generic character state-specific functionality.
	//---------------------------------------------------------------------------
	public:

		// Implements basic one-time functionality for each time State_Dead is
		// entered.
		void OnDead(void);

	//---------------------------------------------------------------------------
	// Internal functions
	//---------------------------------------------------------------------------
	protected:
		// Get all required resources
		int16_t GetResources(void);						// Returns 0 if successfull, non-zero otherwise
		
		// Free all resources
		int16_t FreeResources(void);						// Returns 0 if successfull, non-zero otherwise

		// Initalize the object - this should be called after the resources are loaded
		int16_t Init(void);

		// Go through the message queue and change the state if necessary
		void ProcessMessages(void);

		// Send panic message to other band members
		void AlertFlock(void);

		// Change randomly to one of the normal states, Walk, Stand, Hide
		void ChangeRandomState(void);
	};


#endif //OSTRICH_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
