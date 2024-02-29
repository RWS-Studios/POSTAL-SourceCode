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
// hood.h
// Project: Nostril (aka Postal)
//
//	History:
//		01/22/97	JMI	Converted many m_spryAlphas/Opaques to arrays and added
//							a macro enum MaxLayers.
//		
//		01/23/97	JMI	Added two very non-major comments and removed a #if 0
//							block.
//		
//		01/26/97	JMI	Made m_imBackground public.  It was the only way I could
//							figure to get any idea of the realm dimensions that I
//							needed for the editor.
//		
//		01/26/97	JMI	Added override of EditRect() that sets the rect to the
//							dimensions of m_pimBackground->
//		
//		01/31/97 MJR	Added GetWidth() and GetHeight().
//		
//		02/03/97	JMI	Updated default relative path used to get hood resources.
//		
//		02/04/97	JMI	Changed all resources to pointers so we can fully utilize
//							the RResMgr.
//		
//		02/07/97	JMI	Added m_sWorldXRot, world transformation x rotation.
//
//		02/13/97	JMI	Changed paths for hoods to be in hoods/ instead of bg/
//							and also now just store the import part (like "city" instead
//							of "hoods/city").
//							Also, now gets resources for lighting effects.
//							Also, now sets the realm's m_phood ptr to this instance.
//
//		02/13/97	JMI	Had to make m_pimBackground public so 3D guys could get in-
//							to its palette.
//
//		03/13/97	JMI	Load now takes a version number.
//
//		04/09/97 BRH	Added RMultiGrid to the hoods for the new type of attribute
//							maps.  These will eventuall replace the RAttribMap but for
//							now they are both here so that all parts of the game
//							will continue to work during testing of the new attribute
//							map.
//
//		05/12/97 JRD	Made it into a trigger CThing so it can load and save trigger
//							attribute maps.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef TRIGGER_H
#define TRIGGER_H

#include "RSPiX.h"
#ifdef PATHS_IN_INCLUDES
	#include "WishPiX/Spry/spry.h"
	#include "ORANGE/MultiGrid/MultiGridIndirect.h"
#else
	#include "spry.h"
	#include "multigridindirect.h"
#endif
#include "thing.h"

// A fake declaration for CRealm pointers...
class CRealm; 

// This is the hood object
class CTrigger : public CThing
	{
	//---------------------------------------------------------------------------
	// Types, enums, etc.
	//---------------------------------------------------------------------------
	public:

	//---------------------------------------------------------------------------
	// Variables
	//---------------------------------------------------------------------------
	public:
		
		RMultiGridIndirect* m_pmgi;							// Attribute map of regions
		USHORT	m_ausPylonUIDs[256];							// Look up for Pylon ID's

	//---------------------------------------------------------------------------
	// Constructor(s) / destructor
	//---------------------------------------------------------------------------
	protected:
		// Constructor
		CTrigger(CRealm* pRealm);

	public:
		// Destructor
		~CTrigger();

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
			*ppNew = new CTrigger(pRealm);
			if (*ppNew == 0)
				{
				sResult = -1;
				TRACE("CTrigger::Construct(): Couldn't construct CTrigger!\n");
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

		// Called by editor to get the clickable pos/area of an object.
		virtual	// If you override this, do NOT call this base class.
		void EditRect(				// Returns nothiing.
			RRect*	prc)			// Out: Clickable pos/area of object.
			{
			// Default implementation makes the object unclickable.
			prc->sX	= 0;
			prc->sY	= 0;
			prc->sW	= 16;
			prc->sH	= 16;
			}

	//---------------------------------------------------------------------------
	// Trigger Specific Functions
	//---------------------------------------------------------------------------
	public:

		// After the game editor creates the attribute data, stick it here
		void	AddData(RMultiGridIndirect* pmgi);
		
	};


#endif //HOOD_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
