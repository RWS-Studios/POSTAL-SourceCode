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
// Warp.H
// Project: Nostril (aka Postal)
// 
// History:
//		06/02/97 JMI	Started.
//
//		06/07/97	JMI	Added warp in options enum.
//							Also, constructor now initializes static ms_stockpile
//							to CDude defaults, if it has not yet been done.
//							Also, added CreateWarpFromDude().
//
//		06/15/97	JMI	Moved initialization of ms_stockpile into warp.cpp since
//							the stockpile is non aggregatable.
//
//		07/19/97	JMI	Added m_sRotY, the dude's initial rotation around the Y
//							axis.
//
//		07/21/97	JMI	Added GetX(), GetY(), and GetZ().	
//
//////////////////////////////////////////////////////////////////////////////
//
// This CThing-derived class will represent places and settings for dudes to
// 'warp' in at/with.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef WARP_H
#define WARP_H

#include "RSPiX.h"
#include "realm.h"
#include "StockPile.h"
#include "dude.h"

class CWarp : public CThing
	{
	//---------------------------------------------------------------------------
	// Types, enums, etc.
	//---------------------------------------------------------------------------
	public:

		enum	// Warp options.
			{
			None				= 0x0000,
			// Only one of these options may be specified per WarpIn*().
			CopyStockPile	= 0x0001,	// Copy warp stockpile to Dude's.
			UnionStockPile	= 0x0002,	// Union warp stockpile with Dude's.
			AddStockPile	= 0x0003,	// Add stockpile to dude.

			// Mask for stockpile options.
			StockPileMask	= 0x000F

			// These options can be combined per WarpIn*() call.

			};

	//---------------------------------------------------------------------------
	// Variables
	//---------------------------------------------------------------------------
	public:
		double m_dX;										// Dude's initial position.
		double m_dY;										// Dude's initial position.
		double m_dZ;										// Dude's initial position.
		short	m_sRotY;										// Dude's initial rotation 
																// around the Y axis.

		short m_sSuspend;									// Suspend flag
														
		CSprite2	m_sprite;								// Sprite.


	protected:
														
	//---------------------------------------------------------------------------
	// Static Variables
	//---------------------------------------------------------------------------
	public:


		// The stockpile of ammo and health used by all CWarps.
		static CStockPile	ms_stockpile;

		// Tracks file counter so we know when to load/save "common" data 
		static short ms_sFileCount;


	//---------------------------------------------------------------------------
	// Constructor(s) / destructor
	//---------------------------------------------------------------------------
	public:
		// Constructor
		CWarp(CRealm* pRealm)
			: CThing(pRealm, CWarpID)
			{
			m_sSuspend					= 0;
			m_sRotY						= 0;

			m_sprite.m_pthing			= this;
			}

	public:
		// Destructor
		~CWarp()
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
			*ppNew = new CWarp(pRealm);
			if (*ppNew == 0)
				{
				sResult = -1;
				TRACE("CWarp::Construct(): Couldn't construct CWarp (that's a bad thing)\n");
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
		void EditRect(				// Returns nothing.
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

		// Get the coordinates of this thing.
		virtual					// Overriden here.
		double GetX(void)	{ return m_dX; }

		virtual					// Overriden here.
		double GetY(void)	{ return m_dY; }

		virtual					// Overriden here.
		double GetZ(void)	{ return m_dZ; }

	//---------------------------------------------------------------------------
	// Handy external functions
	//---------------------------------------------------------------------------
	public:

		// Stocks, rejuvenates, and places a CDude.  The dude can be passed to this
		// function or allocated by this function.
		short WarpIn(			// Returns 0 on success.
			CDude**	ppdude,	// In:  CDude to 'warp in', *ppdude = NULL to create one.
									// Out: Newly created CDude, if no CDude passed in.
			short	sOptions);	// In:  Options for 'warp in'.

		// Stocks, rejuvenates, and places a CDude at a random warp.  The dude can 
		// be passed to this function or allocated by this function.
		static short WarpInAnywhere(	// Returns 0 on success.
			CRealm*	prealm,				// In:  Realm in which to choose CWarp.
			CDude**	ppdude,				// In:  CDude to 'warp in', *ppdude = NULL to create one.
												// Out: Newly created CDude, if no CDude passed in.
			short	sOptions);				// In:  Options for 'warp in'.

		// Creates a warp based on a dude's settings.
		static short CreateWarpFromDude(	// Returns 0 on success.
			CRealm*	prealm,					// In:  Realm in which to choose CWarp.
			CDude*	pdude,					// In:  Dude to create warp from.
			CWarp**	ppwarp,					// Out: New warp on success.
			bool		bCopyStockPile);		// In:  true to copy stockpile, false otherwise.


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


#endif // WARP_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
