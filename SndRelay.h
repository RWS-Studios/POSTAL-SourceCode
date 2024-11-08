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
// SndRelay.H
// Project: Nostril (aka Postal)
// 
// History:
//		08/11/97 JMI	Stole infrastructure from SoundThing.
//
//////////////////////////////////////////////////////////////////////////////
//
// This CThing-derived class will relay sound volumes based on 
//	DistanceToVolume() (i.e., the distance to the ear (usually the local dude))
// to the selected CSoundThing.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef SNDRELAY_H
#define SNDRELAY_H

#include "RSPiX.h"
#include "realm.h"
#include "SampleMaster.h"

class CSndRelay : public CThing
	{
	//---------------------------------------------------------------------------
	// Types, enums, etc.
	//---------------------------------------------------------------------------
	public:

		typedef enum
			{
			State_Happy,		// La, la, la.
			State_Delete		// Delete self next chance.
			} State;

	//---------------------------------------------------------------------------
	// Variables
	//---------------------------------------------------------------------------
	public:
		bool m_bInitiallyEnabled;

		CSprite2 m_sprite;						// Sprite (for editor only)
		double m_dX;								// x coord.
		double m_dY;								// y coord.
		double m_dZ;								// z coord.

		bool m_bEnabled;

		int16_t m_sSuspend;							// Suspend flag

		State	m_state;								// Current state.

		U16	m_idParent;							// Parent CSoundThing.

	protected:

		static int16_t	ms_sFileCount;			// File count.
														
	//---------------------------------------------------------------------------
	// Constructor(s) / destructor
	//---------------------------------------------------------------------------
	public:
		// Constructor
		CSndRelay(CRealm* pRealm)
			: CThing(pRealm, CSndRelayID)
			{
			m_bInitiallyEnabled = true;
			m_bEnabled	= m_bInitiallyEnabled;

			m_sSuspend	= 0;

			m_state		= State_Happy;

			m_idParent	= CIdBank::IdNil;
			}

	public:
		// Destructor
		~CSndRelay()
			{
			Kill();
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
			*ppNew = new CSndRelay(pRealm);
			if (*ppNew == 0)
				{
				sResult = -1;
				TRACE("CSndRelay::Construct(): Couldn't construct CSndRelay (that's a bad thing)\n");
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
			int16_t sZ);												// In: New z coord

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

		// Called by editor to get the clickable pos/area of an object in 2D.
		virtual	// Overridden here.
		void EditRect(				// Returns nothiing.
			RRect*	prc);			// Out: Clickable pos/area of object.

		// Called by editor to get the hotspot of an object in 2D.
		virtual	// Overridden here.
		void EditHotSpot(			// Returns nothiing.
			int16_t*	psX,			// Out: X coord of 2D hotspot relative to
										// EditRect() pos.
			int16_t*	psY);			// Out: Y coord of 2D hotspot relative to
										// EditRect() pos.

		// Called by editor to update object
		void EditUpdate(void);

		// Called by editor to render object
		void EditRender(void);

		// Get the coordinates of this thing.
		virtual					// Overriden here.
		double GetX(void)	{ return m_dX; }

		virtual					// Overriden here.
		double GetY(void)	{ return m_dY; }

		virtual					// Overriden here.
		double GetZ(void)	{ return m_dZ; }

	//---------------------------------------------------------------------------
	// Internal functions
	//---------------------------------------------------------------------------
	protected:
		// Init object
		int16_t Init(void);											// Returns 0 if successfull, non-zero otherwise
		
		// Kill object
		int16_t Kill(void);											// Returns 0 if successfull, non-zero otherwise

		// Process our message queue.
		void ProcessMessages(void);

	};


#endif // SNDRELAY_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
