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
// Ladder.H
// Project: Nostril (aka Postal)
// 
// History:
//		06/02/97 JMI	Started.
//
//////////////////////////////////////////////////////////////////////////////
//
// This CThing-derived class will represent ladders that the CDudes, and 
// perhaps other characters, can use to climb to heights they cannot step up
// to.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef LADDER_H
#define LADDER_H

#include "RSPiX.h"
#include "realm.h"
#include "character.h"

class CLadder : public CThing
	{
	//---------------------------------------------------------------------------
	// Types, enums, etc.
	//---------------------------------------------------------------------------
	public:

	//---------------------------------------------------------------------------
	// Variables
	//---------------------------------------------------------------------------
	public:
		double m_dX;
		double m_dY;
		double m_dZ;

		short	m_sLen;								// Length of ladder.
		short	m_sHeight;							// Height of ladder.
		short m_sRotY;								// Rotation around Y axis (i.e., 
														// direction on X/Z plane).

		short m_sSuspend;							// Suspend flag
														
		CSmash	m_smashTop;						// Collision smash for the top of the
														// ladder.

		CSmash	m_smashBottom;					// Collision smash for the bottom of the
														// ladder.

		CSprite2	m_sprite;						// Sprite.

		CCharacter*	m_pcharLadderBoy;			// Character currently using the 
														// ladder.
														
	protected:
														
	//---------------------------------------------------------------------------
	// Static Variables
	//---------------------------------------------------------------------------
	public:

	//---------------------------------------------------------------------------
	// Constructor(s) / destructor
	//---------------------------------------------------------------------------
	public:
		// Constructor
		CLadder(CRealm* pRealm)
			: CThing(pRealm, CLadderID)
			{
			m_sSuspend					= 0;

			m_sprite.m_pthing			= this;
			m_smashTop.m_pThing		= this;
			m_smashBottom.m_pThing	= this;
			
			m_sLen						= 0;
			m_sHeight					= 0;
			m_sRotY						= 0;

			m_pcharLadderBoy			= NULL;
			}

	public:
		// Destructor
		~CLadder()
			{
			// Remove sprite from scene (this is safe even if it was already removed!)
			m_pRealm->m_scene.RemoveSprite(&m_sprite);

			// Free resources
			FreeResources();

			// Remove collision thinger.
			m_pRealm->m_smashatorium.Remove(&m_smashTop);
			m_pRealm->m_smashatorium.Remove(&m_smashBottom);
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
			*ppNew = new CLadder(pRealm);
			if (*ppNew == 0)
				{
				sResult = -1;
				TRACE("CLadder::Construct(): Couldn't construct CLadder (that's a bad thing)\n");
				}
			return sResult;
			}

	//---------------------------------------------------------------------------
	// Required virtual functions (implementing them as inlines doesn't pay!)
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

		// Update object
		void Update(void);

		// Render object
		void Render(void);

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

		// Called by editor to get the clickable pos/area of an object in 2D.
		virtual	// Overridden here.
		void EditRect(				// Returns nothiing.
			RRect*	prc);			// Out: Clickable pos/area of object.

		// Called by editor to get the hotspot of an object in 2D.
		virtual	// Overridden here.
		void EditHotSpot(			// Returns nothiing.
			short*	psX,			// Out: X coord of 2D hotspot relative to
										// EditRect() pos.
			short*	psY);			// Out: Y coord of 2D hotspot relative to
										// EditRect() pos.

		// Called by editor to update object
		void EditUpdate(void);

		// Called by editor to render object
		void EditRender(void);

	//---------------------------------------------------------------------------
	// Handy external functions
	//---------------------------------------------------------------------------
	public:

		// Call to try to get on the ladder.
		// This function will return false if there is already someone on the
		// ladder.
		// If you call this function and it returns true, you MUST call GetOff()
		// once done so other characters can use the ladder.
		bool GetOn(					// Returns true, if able to get on, 
										// false otherwise.
			CCharacter* pchar);	// In:  Character attempting to get onto ladder.

		// Call when you get off the ladder.
		// Only call this function, if you have made a successful call to GetOn()
		// and have not, since, made a call to this function.
		void GetOff(void);		// Returns nothing.

		// Get the next position on the ladder.
		void GetNextPos(			// Returns nothing.
			double*	pdX,			// In:  Current x position.
										// Out: New x position.
			double*	pdY,			// In:  Current y position.
										// Out: New y position.
			double*	pdZ,			// In:  Current z position.
										// Out: New z position.
			double	dDistance);	// In:  Distance to travel. Positive is up.

	//---------------------------------------------------------------------------
	// Internal functions
	//---------------------------------------------------------------------------
	protected:
		// Get all required resources
		short GetResources(void);						// Returns 0, if successfull, non-zero otherwise
		
		// Free all resources
		short FreeResources(void);						// Returns 0, if successfull, non-zero otherwise

		// Initialize object.
		short Init(void);									// Returns 0, on success.
	};


#endif // LADDER_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
