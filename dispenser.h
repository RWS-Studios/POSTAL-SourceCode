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
// dispenser.h
// Project: Nostril (aka Postal)
//
//	History:
//		03/19/97	JMI	Started this dispenser item class using CItem3d as a 
//							template.
//
//		03/20/97	JMI	Added m_ulFileVersion, InstantiateDispensee(), and
//							SaveDispensee() members.  Removed m_idDispensee.
//
//		03/21/97	JMI	Added ms_sDispenseeFileCount.
//
//		04/23/97	JMI	Added new logic type Exists and added a max for the number
//							of dispensees and a current number of dispensees dispensed.
//
//		06/14/97	JMI	Made default logic type Timed.
//
//		06/27/97	JMI	Added m_imRender and m_bEditMode.
//
//		06/28/97	JMI	Added a function to render the dispensee, 
//							RenderDisipensee().
//
//		06/30/97	JMI	m_sDispenseeHotSpotX, m_sDispenseeHotSpotY, and 
//							m_rcDispensee.
//
//		07/10/97	JMI	Added GetClosestDude().
//							Also, added new logic type DistanceToDude.
//
//		07/21/97	JMI	Added GetX(), GetY(), and GetZ().	
//
//		07/28/97	JMI	Changed m_lLogicParm1,2,3 to m_alLogicParms[4] (adding one
//							logic parm while changing the storage technique).
//
////////////////////////////////////////////////////////////////////////////////
#ifndef DISPENSER_H
#define DISPENSER_H

#include "RSPiX.h"

#include "thing.h"
#include "scene.h"
#include "realm.h"
#include "dude.h"

class CDispenser : public CThing
	{
	//---------------------------------------------------------------------------
	// Types, enums, etc.
	//---------------------------------------------------------------------------
	public:
		typedef enum
			{
			Timed,				// Must be 0.
			Exists,		
			DistanceToDude,

			// Add new logic types above.
			NumLogicTypes
			} LogicType;

		typedef enum
			{
			NumParms	= 4
			} Macros;

		typedef struct
			{
			char*	pszName;					// Name of logic (for list box).
			char*	apszParms[NumParms];	// Parm descriptions or NULL for none.
			char*	pszDescription;		// Description of logic (for text box).
			} LogicInfo;

	//---------------------------------------------------------------------------
	// Variables
	//---------------------------------------------------------------------------
	public:

		CSprite2		m_sprite;								// 2D sprite.
		RImage*		m_pim;									// 2D image resource ptr.
		RImage		m_imRender;								// Used to render dispenser
																	// and dispensee in edit mode.

		ClassIDType	m_idDispenseeType;					// Type of object to dispense.
		RFile			m_fileDispensee;						// Mem file representing
																	// dispensee.
		ULONG			m_ulFileVersion;						// File version of data stored
																	// in m_fileDispensee.

		short			m_sX;										// Location of this object,
		short			m_sY;										// the dispenser, which is
		short			m_sZ;										// also the location at which
																	// we will dispense the 
																	// dispensee.

		LogicType	m_logictype;							// Logic used for dispensing.
		long			m_alLogicParms[NumParms];			// Generic parameters for logic.

		short			m_sMaxDispensees;						// Maximum number of dispensees.
		short			m_sNumDispensees;						// Number of dispensees already
																	// dispensed.

		U16			m_u16IdDispensee;						// ID of the last dispensee
																	// we created.

		long			m_lNextUpdate;							// Time of next update.

		bool			m_bEditMode;							// true, if in edit mode, false
																	// otherwise.

		short			m_sDispenseeHotSpotX;				// Hotspot of dispensee.
		short			m_sDispenseeHotSpotY;				// Hotspot of dispensee.
		RRect			m_rcDispensee;							// Rect of dispensee.

		short			m_sSuspend;

		// Tracks file counter so we know when to load/save "common" data 
		static short ms_sFileCount;

		static short ms_sDispenseeFileCount;

	//---------------------------------------------------------------------------
	// Static Variables
	//---------------------------------------------------------------------------
	public:
		// "Constant" values that we want to be able to tune using the editor

		// Descriptions of logic types and their parameters.
		static LogicInfo	ms_aliLogics[NumLogicTypes];

	//---------------------------------------------------------------------------
	// Constructor(s) / destructor
	//---------------------------------------------------------------------------
	protected:
		// Constructor
		CDispenser(CRealm* pRealm)
			: CThing(pRealm, CDispenserID)
			{
			m_pim					= NULL;
			m_idDispenseeType	= TotalIDs;			// This means none.
			m_sSuspend			= FALSE;
			memset(m_alLogicParms, 0, sizeof(m_alLogicParms) );
			m_sMaxDispensees	= 10;
			m_sNumDispensees	= 0;
			m_u16IdDispensee	= CIdBank::IdNil;
			m_logictype			= Timed;
			m_bEditMode			= false;
			m_sDispenseeHotSpotX	= 0;
			m_sDispenseeHotSpotY	= 0;
			}

	public:
		// Destructor
		~CDispenser()
			{
			// Kill dispenser
			Kill();
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
			*ppNew = new CDispenser(pRealm);
			if (*ppNew == 0)
				{
				sResult = -1;
				TRACE("CDispenser::Construct(): Couldn't construct CDispenser!\n");
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
		// (virtual (Overridden here)).
		void Update(void);

		// Called by editor to init new object at specified position
		// (virtual (Overridden here)).
		short EditNew(												// Returns 0 if successfull, non-zero otherwise
			short sX,												// In:  New x coord
			short sY,												// In:  New y coord
			short sZ);												// In:  New z coord

		// Called by editor to modify object.
		// (virtual (Overridden here)).
		short EditModify(void);									// Returns 0 if successfull, non-zero otherwise

		// Called by editor to move object to specified position
		// (virtual (Overridden here)).
		short EditMove(											// Returns 0 if successfull, non-zero otherwise
			short sX,												// In:  New x coord
			short sY,												// In:  New y coord
			short sZ);												// In:  New z coord

		// Called by editor to render object
		// (virtual (Overridden here)).
		void EditRender(void);

		// Give Edit a rectangle around this object
		// (virtual (Overridden here)).
		void EditRect(RRect* pRect);

		// Called by editor to get the hotspot of an object in 2D.
		// (virtual (Overridden here)).
		void EditHotSpot(			// Returns nothiing.
			short*	psX,			// Out: X coord of 2D hotspot relative to
										// EditRect() pos.
			short*	psY);			// Out: Y coord of 2D hotspot relative to
										// EditRect() pos.

		// Get the coordinates of this thing.
		virtual					// Overriden here.
		double GetX(void)	{ return (double)m_sX; }

		virtual					// Overriden here.
		double GetY(void)	{ return (double)m_sY; }

		virtual					// Overriden here.
		double GetZ(void)	{ return (double)m_sZ; }

	//---------------------------------------------------------------------------
	// Other functions
	//---------------------------------------------------------------------------
	public:

		// Message handling functions ////////////////////////////////////////////

	//---------------------------------------------------------------------------
	// Internal functions
	//---------------------------------------------------------------------------
	protected:
		// Init dispenser
		short Init(					// Returns 0 if successfull, non-zero otherwise
			bool	bEditMode);		// true, if in edit mode; false, otherwise.
		
		// Kill dispenser
		void Kill(void);

		// Get all required resources
		short GetResources(void);	// Returns 0 if successfull, non-zero otherwise
		
		// Free all resources
		void FreeResources(void);

		// Create a dispensee from the memfile, if open.
		short InstantiateDispensee(	// Returns 0 on success.
			CThing**	ppthing,				// Out: New thing loaded from m_fileDispensee.
			bool		bEditMode);			// In:  true if in edit mode.

		// Write dispensee to the memfile.
		short SaveDispensee(		// Returns 0 on success.
			CThing*	pthing);		// In:  Instance of Dispensee to save.

		// Render dispensee to m_imRender.
		short RenderDispensee(	// Returns 0 on success.
			CThing*	pthing);		// In:  Instance of Dispensee to render.

		// Get the closest dude.
		short GetClosestDudeDistance(	// Returns 0 on success.  Fails, if no dudes.
			long* plClosestDistance);	// Out:  Distance to closest dude.

		// Destroy an instantiated dispensee.
		void DestroyDispensee(	// Returns nothing.
			CThing**	ppthing);	// In:  Ptr to the instance.
	};


#endif //DISPENSER_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
