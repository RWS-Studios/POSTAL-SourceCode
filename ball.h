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
// ball.h
// Project: Nostril (aka Postal)
//
//	History:
//	
//		01/19/97	JMI	Added some initializations to constructor.
//
//		01/27/97	JMI	Added override for EditRect() to make this object clickable.
//
//		02/02/97	JMI	Added EditHotSpot() override.
//
//		02/07/97	JMI	Added members for 3D.
//
//		02/07/97	JMI	Removed m_pipeline and associated members and setup since
//							the CScene now owns the pipeline.
//
//		02/13/97	JMI	Changing RForm3d to RSop.
//
//		02/23/97	JMI	Brought up to date so we can continue to use this as a
//							test object.
//
//		02/26/97	JMI	Now sets m_sprite.m_pthing = this on construction.
//
//		03/13/97	JMI	Load now takes a version number.
//
//		07/21/97	JMI	Added GetX(), GetY(), and GetZ().	
//
////////////////////////////////////////////////////////////////////////////////
#ifndef BALL_H
#define BALL_H

#include "RSPiX.h"
#include "realm.h"


// This is a sample game object
class CBall : public CThing
	{
	//---------------------------------------------------------------------------
	// Types, enums, etc.
	//---------------------------------------------------------------------------
	protected:

	//---------------------------------------------------------------------------
	// Variables
	//---------------------------------------------------------------------------
	protected:
		double m_dX;
		double m_dY;
		double m_dZ;
		double m_dDX;
		double m_dDY;
		double m_dDZ;
		short m_sPrevHeight;
		short m_sSuspend;

		long	m_lPrevTime;
		
		CSprite3		m_sprite;	// Container (contains ref's to below).

		CAnim3D		m_anim;		// 3D animation.

		RTransform	m_trans;		// Current transformation.

		short			m_sCurRadius;	// Objects radius (currently fudged).
		

		// Tracks file counter so we know when to load/save "common" data 
		static short ms_sFileCount;

	//---------------------------------------------------------------------------
	// Constructor(s) / destructor
	//---------------------------------------------------------------------------
	protected:
		// Constructor
		CBall(CRealm* pRealm)
			: CThing(pRealm, CBallID)
			{
			m_sSuspend	= 0;
			m_dDX			= 0;
			m_dDY			= 0;
			m_dDZ			= 0;
			m_sprite.m_pthing	= this;
			}

	public:
		// Destructor
		~CBall()
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
			*ppNew = new CBall(pRealm);
			if (*ppNew == 0)
				{
				sResult = -1;
				TRACE("CBall::Construct(): Couldn't construct CBall!\n");
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
		// Get all required resources
		short GetResources(void);								// Returns 0 if successfull, non-zero otherwise
		
		// Free all resources
		short FreeResources(void);								// Returns 0 if successfull, non-zero otherwise
	};


#endif //BALL_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
