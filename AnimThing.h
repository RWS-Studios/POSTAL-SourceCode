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
// AnimThing.H
// Project: Nostril (aka Postal)
// 
// History:
//		02/18/97 JMI	Started.
//
//		02/19/97	JMI	Added ability to send any message to any CThing when done
//							with animation.
//
//		02/19/97	JMI	Unprotected more members.
//
//		02/24/97	JMI	Changed declaration of m_sprite from CAlphaSprite2 to 
//							CSprite2.
//
//		02/24/97	JMI	Changed m_pthingSendMsg to m_u16IdSendMsg.
//
//		02/26/97	JMI	Now sets m_sprite.m_pthing = this on construction.
//
//		03/13/97	JMI	Load now takes a version number.
//
//		06/24/97	JMI	Now intializes m_msg's priority to 0 on construction for
//							safety.
//
//		07/21/97	JMI	Added GetX(), GetY(), and GetZ().	
//
//////////////////////////////////////////////////////////////////////////////
//
// This CThing-derived class will play an animation to its finish and then
// destroy itself.  It's a mini Postal movie player.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef ANIMTHING_H
#define ANIMTHING_H

#include "RSPiX.h"
#include "realm.h"

#include "AlphaAnimType.h"

class CAnimThing : public CThing
	{
	//---------------------------------------------------------------------------
	// Types, enums, etc.
	//---------------------------------------------------------------------------
	public:

		typedef RChannel<CAlphaAnim> ChannelAA;

	//---------------------------------------------------------------------------
	// Variables
	//---------------------------------------------------------------------------
	public:
		double m_dX;
		double m_dY;
		double m_dZ;

		short m_sSuspend;							// Suspend flag
		short	m_sLoop;								// Loops, if true.
		char	m_szResName[RSP_MAX_PATH];		// Resource name.
														
		long	m_lAnimTime;						// Cummulative animation time.
		long	m_lAnimPrevTime;					// Last animation time.
														
		U16			m_u16IdSendMsg;			// ID of CThing to send msg to when done.
		GameMessage	m_msg;						// Message to send to m_pthingSendMsg.

	protected:
		CSprite2		m_sprite;					// Sprite.
		ChannelAA*	m_paachannel;				// Animation (with or without alpha).
														
	//---------------------------------------------------------------------------
	// Constructor(s) / destructor
	//---------------------------------------------------------------------------
	public:
		// Constructor
		CAnimThing(CRealm* pRealm)
			: CThing(pRealm, CAnimThingID)
			{
			m_paachannel		= NULL;
			m_sSuspend			= 0;
			m_sLoop				= TRUE;
			m_szResName[0]		= '\0';
			m_msg.msg_Generic.sPriority	= 0;
			m_u16IdSendMsg		= CIdBank::IdNil;
			m_sprite.m_pthing	= this;
			}

	public:
		// Destructor
		~CAnimThing()
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
			*ppNew = new CAnimThing(pRealm);
			if (*ppNew == 0)
				{
				sResult = -1;
				TRACE("CExplode::Construct(): Couldn't construct CAnimThing (that's a bad thing)\n");
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

		short Setup(												// Returns 0 on success.
			short sX,												// In: New x coord
			short sY,												// In: New y coord
			short sZ);												// In: New z coord

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
		short GetResources(void);						// Returns 0 if successfull, non-zero otherwise
		
		// Free all resources
		short FreeResources(void);						// Returns 0 if successfull, non-zero otherwise
	};


#endif // ANIMTHING_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
