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
// pylon.h
// Project: Nostril (aka Postal)
//
//	History:
//
//		05/01/97 BRH	Started this object from the bouys.  It will take over
//							the duty of logic suggestions and markers and the bouys
//							will go back to strictly navigation.
//
//		06/17/97 MJR	Moved some vars that were CPylon statics into the realm
//							so they could be instantiated on a realm-by-realm basis.
//
//		06/30/97	JMI	Moved EditRect() and EditHotSpot() from pylon.h to 
//							pylon.cpp.
//
//		07/08/97	JMI	Now removes its smash from the smashatorium in the 
//							destructor.
//
//		07/14/97	JMI	Now memsets m_msg to 0s before initializing the few members
//							that can be accessed via msg_Generic type.
//
//		07/17/97 BRH	Chagned Triggeed function to trigger only if the dude
//							on the traget area is alive.
//
//		07/21/97	JMI	Added GetX(), GetY(), and GetZ().	
//
////////////////////////////////////////////////////////////////////////////////
#ifndef PYLON_H
#define PYLON_H

#include "RSPiX.h"
#include "realm.h"
#include "smash.h"
#include "Thing3d.h"
#include "dude.h"
//#include <vector>

#define PYLON_MAX_PYLONS 254

// CPylon is the class for navigation
class CPylon : public CThing
	{
	friend class CNavigationNet;
	//---------------------------------------------------------------------------
	// Types, enums, etc.
	//---------------------------------------------------------------------------
	public:

	//---------------------------------------------------------------------------
	// Variables
	//---------------------------------------------------------------------------
	public:
		UCHAR	m_ucID;								// Pylon ID
		GameMessage m_msg;						// Place for storing hint messages
		U16	m_u16TargetDudeID;				// ID of dude you are supposed to attack;

	protected:
		double m_dX;								// x coord
		double m_dY;								// y coord
		double m_dZ;								// z coord
													
		RImage* m_pImage;							// Pointer to only image (replace with 3d anim, soon)
		CSprite2 m_sprite;						// Sprite (replace with CSprite3, soon)
		CSmash	m_smash;							// Collision region

		short m_sSuspend;							// Suspend flag

		// Tracks file counter so we know when to load/save "common" data 
		static short ms_sFileCount;

		// "Constant" values that we want to be able to tune using the editor

	//---------------------------------------------------------------------------
	// Constructor(s) / destructor
	//---------------------------------------------------------------------------
	protected:
		// Constructor
		CPylon(CRealm* pRealm)
			: CThing(pRealm, CPylonID)
		{
			m_pImage = 0;
			m_sSuspend = 0;
			
			// Let's default the whole thing to zero.
			memset(&m_msg, 0, sizeof(m_msg) );

			// Then set some portions we can via the generic type.
			m_msg.msg_Generic.eType = typeGeneric;
			m_msg.msg_Generic.sPriority = 0;

			pRealm->m_sNumPylons++;
			m_u16TargetDudeID = 0;
		}

	public:
		// Destructor
		~CPylon()
		{
			// Remove sprite from scene (this is safe even if it was already removed!)
			m_pRealm->m_scene.RemoveSprite(&m_sprite);

			// Remove smash from smashatorium (this is safe even if it was already removed!).
			m_pRealm->m_smashatorium.Remove(&m_smash);

			// Free resources
			FreeResources();

			m_pRealm->m_sNumPylons--;
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

			if (pRealm->m_sNumPylons < PYLON_MAX_PYLONS)
			{
				*ppNew = new CPylon(pRealm);
				if (*ppNew == 0)
				{
					sResult = -1;
					TRACE("CPylon::Construct(): Couldn't construct CPylon (that's a bad thing)\n");
				}
			}
			else
			{
				TRACE("CPylon::CPylon() - No more pylon ID's available\n");
				*ppNew = NULL;
				sResult = -1;
			}

			return sResult;
			}

	//---------------------------------------------------------------------------
	// Enemy logic hint functions
	//---------------------------------------------------------------------------

	// An enemy can call this function to request that a message be sent to it
	// with any hint information that this bouy may have.
	void MessageRequest(CThing* pThing);

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

		// Called by editor to update object
		void EditUpdate(void);

		// Called by editor to render object
		void EditRender(void);

		// Give Edit a rectangle around this object
		void EditRect(RRect* pRect);

		// Called by editor to get the hotspot of an object in 2D.
		void EditHotSpot(			// Returns nothiing.
			short*	psX,			// Out: X coord of 2D hotspot relative to
										// EditRect() pos.
			short*	psY);			// Out: Y coord of 2D hotspot relative to
										// EditRect() pos.

		// Get the coordinates of this thing.
		virtual					// Overriden here.
		double GetX(void)	{ return m_dX; }

		virtual					// Overriden here.
		double GetY(void)	{ return m_dY; }

		virtual					// Overriden here.
		double GetZ(void)	{ return m_dZ; }

		// Search the list of pylons and return a pointer to the one with the given ID
		CPylon* GetPylon(UCHAR ucPylonID);

		// Search the list of pylons and return the instance ID of the one with
		// the given pylon id.
		U16 GetPylonUniqueID(UCHAR ucPylonID);

		// Return true if the pylon was triggered in the last interation
		inline bool Triggered(void)
		{
			bool bTriggered = false;

			if (m_u16TargetDudeID != CIdBank::IdNil)
			{
				CThing* pthing;
				m_pRealm->m_idbank.GetThingByID((CThing**) &pthing, m_u16TargetDudeID);
				if (pthing && pthing->GetClassID() == CDudeID)
				{
					if (((CDude*) pthing)->m_state != CThing3d::State_Dead)
						bTriggered = true;
				}
			}
			return bTriggered;
		}


	//---------------------------------------------------------------------------
	// Internal functions
	//---------------------------------------------------------------------------
	protected:
		// Get all required resources
		short GetResources(void);						// Returns 0 if successfull, non-zero otherwise
		
		// Free all resources
		short FreeResources(void);						// Returns 0 if successfull, non-zero otherwise

		// GetFreePylonID - get the next ID that is not in use
		UCHAR GetFreePylonID(void);
		
		// Process Messages - look for DudeTrigger message
		void ProcessMessages(void);

	};


#endif //PYLON_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
