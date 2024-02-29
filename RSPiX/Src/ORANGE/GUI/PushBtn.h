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
//////////////////////////////////////////////////////////////////////////////
//
// PushBtn.h
// 
// History:
//		02/04/97 JMI	Started.
//
//		03/19/97	JMI	Converted to using the RHot::m_iecUser (was using
//							RHot::m_epcUser) so HotCall and CursorEvent now take
//							RInputEvent ptrs.
//
//////////////////////////////////////////////////////////////////////////////
//
// Please see the CPP file for an explanation of this API.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef PUSHBTN_H
#define PUSHBTN_H


//////////////////////////////////////////////////////////////////////////////
// Headers.
//////////////////////////////////////////////////////////////////////////////

#include "System.h"
// If PATHS_IN_INCLUDES macro is defined, we can utilized relative
// paths to a header file.  In this case we generally go off of our
// RSPiX root directory.  System.h MUST be included before this macro
// is evaluated.  System.h is the header that, based on the current
// platform (or more so in this case on the compiler), defines 
// PATHS_IN_INCLUDES.  Blue.h includes system.h so you can include that
// instead.
#ifdef PATHS_IN_INCLUDES
	#include "ORANGE/GUI/guiItem.h"
#else
	#include "GuiItem.h"
#endif // PATHS_IN_INCLUDES

//////////////////////////////////////////////////////////////////////////////
// Macros.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Typedefs.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
class RPushBtn : public RGuiItem
	{
	public:	// Construction/Destruction.
		// Default constructor.
		RPushBtn(void);
		// Destructor.
		~RPushBtn(void);

//////////////////////////////////////////////////////////////////////////////

	public:	// Typedefs/enums.

		typedef enum
			{
			On,
			Off
			} State;

//////////////////////////////////////////////////////////////////////////////

	public:	// Methods.

		////////////////////////////////////////////////////////////////////////
		// Methods.
		////////////////////////////////////////////////////////////////////////

		// Compose item.
		virtual					// If you override this, call this base if possible.
		void Compose(			// Returns nothing.
			RImage* pim = NULL);	// Dest image, uses m_im if NULL.

		// Cursor event notification.
		// Events in event area.
		virtual						// If you override this, call this base if possible.
		void CursorEvent(			// Returns nothing.
			RInputEvent* pie);	// In:  Most recent user input event.             
										// Out: pie->sUsed = TRUE, if used.

		// Draw border.
		virtual					// Overridden here.
		void DrawBorder(		// Returns nothing.
			RImage* pim	= NULL,			// Dest image, uses m_im if NULL.
			short sInvert	= FALSE);	// Inverts border if TRUE.

		////////////////////////////////////////////////////////////////////////
		// Querries.
		////////////////////////////////////////////////////////////////////////

		// Gets the thickness of the top/left border (including border edge effect).
		virtual		// Overridden here.
		short GetTopLeftBorderThickness(void);	// Returns border thickness 
															// including edge effect.                      

		// Gets the thickness of the bottom/right border (including border edge effect).
		virtual		// Overridden here.
		short GetBottomRightBorderThickness(void);	// Returns border thickness 
																	// including edge effect.                      


//////////////////////////////////////////////////////////////////////////////

	public:	// Static

//////////////////////////////////////////////////////////////////////////////

	public:	// Querries.

//////////////////////////////////////////////////////////////////////////////

	protected:	// Internal functions.

//////////////////////////////////////////////////////////////////////////////

	public:	// Member variables.

		State	m_state;		// The button's current state (On or Off (see enums)).

	protected:	// Internal typedefs.

	protected:	// Protected member variables.

	};

#endif // PUSHBTN_H
//////////////////////////////////////////////////////////////////////////////
//	EOF
//////////////////////////////////////////////////////////////////////////////
