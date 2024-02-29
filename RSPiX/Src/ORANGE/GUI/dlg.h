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
// Dlg.H
// 
// History:
//		01/18/97 JMI	Started tracking history of this file.
//							Converted Do() to take an RInputEvent* instead of a 
//							long*.
//
//		03/19/97	JMI	Converted to using the RHot::m_iecUser (was using
//							RHot::m_epcUser) so HotCall and CursorEvent now take
//							RInputEvent ptrs.
//
//////////////////////////////////////////////////////////////////////////////
//
// See CPP for description.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef DLG_H
#define DLG_H
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
class RDlg : public RGuiItem
	{
	public:	// Construction/Destruction.
		// Default constructor.
		RDlg(void);
		// Destructor.
		~RDlg(void);

//////////////////////////////////////////////////////////////////////////////

	public:	// Methods.

		////////////////////////////////////////////////////////////////////////
		// Methods.
		////////////////////////////////////////////////////////////////////////

		// Does as Dlg would Do.
		virtual	// If you override this, call this base if possible.
		void Do(						// Returns nothing.
			RInputEvent* pie);	// In:  Most recent user input event.
										// Out: pie->sUsed = TRUE, if used.

		// Set this item's event area.  This is the area where cursor events are
		// interesting to the item.
		virtual					// If you override this, call this base if possible.
		void SetEventArea(void);	// Returns nothing.

		// Cursor event notification.
		// Events in event area.
		virtual						// If you override this, call this base if possible.
		void CursorEvent(			// Returns nothing.
			RInputEvent* pie);	// In:  Most recent user input event.             
										// Out: pie->sUsed = TRUE, if used.

		// Compose item.
		virtual					// If you override this, call this base if possible.
		void Compose(			// Returns nothing.
			RImage* pim = NULL);	// Dest image, uses m_im if NULL.

		////////////////////////////////////////////////////////////////////////
		// Querries.
		////////////////////////////////////////////////////////////////////////

		// Get the "client" area (i.e., non-border/title area) relative to this
		// item.  Calls base class version.
		void GetClient(		// Returns nothing.
			short* psX,			// Out: X position unless NULL.
			short* psY,			// Out: Y position unless NULL.
			short* psW,			// Out: Width unless NULL.
			short* psH);		// Out: Height unless NULL.

//////////////////////////////////////////////////////////////////////////////

	public:	// Static

//////////////////////////////////////////////////////////////////////////////

	public:	// Querries.

//////////////////////////////////////////////////////////////////////////////

	protected:	// Internal functions.

//////////////////////////////////////////////////////////////////////////////

	public:	// Member variables.
		short m_sMoveOffsetX;
		short m_sMoveOffsetY;

	protected:	// Internal typedefs.

	protected:	// Protected member variables.
	};

#endif // DLG_H
//////////////////////////////////////////////////////////////////////////////
//	EOF
//////////////////////////////////////////////////////////////////////////////
