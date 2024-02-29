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
// MsgBox.H
// 
// History:
//		01/19/97 JMI	Started tracking history of this file.
//
//		01/19/97	JMI	Converted Do*() to taking an RInputEvent* instead of a
//							long*.
//
//		01/23/97	JMI	Removed sDeactivateHotBoxes parm from DoModal().
//
//		04/24/97	JMI	Add additional parms to Add*() to add extra size to a
//							GUI.
//
//////////////////////////////////////////////////////////////////////////////
//
// Please see the CPP file for an explanation of this API.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef MSGBOX_H
#define MSGBOX_H

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
	#include "ORANGE/GUI/dlg.h"
	#include "ORANGE/GUI/btn.h"
	#include "ORANGE/GUI/txt.h"
	#include "ORANGE/GUI/edit.h"
#else
	#include "dlg.h"
	#include "btn.h"
	#include "txt.h"
	#include "edit.h"
#endif // PATHS_IN_INCLUDES

//////////////////////////////////////////////////////////////////////////////
// Macros.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Typedefs.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
class RMsgBox : public RDlg
	{
	public:	// Construction/Destruction.
		// Default constructor.
		RMsgBox(void);
		// Destructor.
		~RMsgBox(void);

//////////////////////////////////////////////////////////////////////////////

	public:	// Typedefs.

		// Called iteratively while waiting for user response.
		typedef ULONG (*MsgBoxCall)(	// If the return value is non-zero,
												// RMsgBox::DoModal() returns with this value
												// returned by this callback.
			RMsgBox* pmb);					// this.
													
	public:	// Methods.

		////////////////////////////////////////////////////////////////////////
		// Methods.
		////////////////////////////////////////////////////////////////////////
		
		// Add a btn with provided text at sX, sY in RMsgBox dlg.
		RBtn* AddButton(		// Returns allocated GUI item on success.
									// Do NOT delete this item; it will be deleted
									// by a RemoveAll() call.
			char* pszText,		// Text for btn item.
			short	sX,			// X position in RMsgBox dlg.
			short	sY,			// Y position in RMsgBox dlg.
			ULONG	ulId,			// ID to return if this item is chosen.
									// There will be no response to this item
									// if lId is 0.
			short	sAddW = 0,	// Additional width for guis that require more.
			short	sAddH = 0);	// Additional height for guis that require more.

		// Add a txt item with provided text at sX, sY in RMsgBox dlg.
		RTxt* AddText(			// Returns allocated GUI item on success.
									// Do NOT delete this item; it will be deleted
									// by a RemoveAll() call.
			char* pszText,		// Text for txt item.
			short	sX,			// X position in RMsgBox dlg.
			short	sY,			// Y position in RMsgBox dlg.
			ULONG	ulId,			// ID to return if this item is chosen.
									// There will be no response to this item
									// if lId is 0.
			short	sAddW = 0,	// Additional width for guis that require more.
			short	sAddH = 0);	// Additional height for guis that require more.

		// Add an edit field with provided text at sX, sY in RMsgBox dlg.
		REdit* AddEdit(	// Returns allocated GUI item on success.
								// Do NOT delete this item; it will be deleted
								// by a RemoveAll() call.
			char* pszText,	// Text for edit item.
			short	sX,		// X position in RMsgBox dlg.
			short	sY,		// Y position in RMsgBox dlg.
			ULONG	ulId,			// ID to return if this item is chosen.
									// There will be no response to this item
									// if lId is 0.
			short	sAddW = 0,	// Additional width for guis that require more.
			short	sAddH = 0);	// Additional height for guis that require more.

		// Add the RGuiItem of your choice.  pgui->m_ulUserData will be returned
		// by DoModal() if this item is chosen.
		RGuiItem* AddItem(		// Returns pgui on success.
			RGuiItem*	pgui);	// The RGuiItem to add (its m_sX/Y should already
										// have been set).

		// Activates this RMsgBox and waits for input until m_mbcUser returns
		// non-zero or an item is clicked.  If sDeactivateHots is TRUE, all 
		// CHots are deactivated before activating GUI items.  These CHots are
		// reactivated before exiting DoModal().
		ULONG DoModal(								// Returns chosen ID on success,
														// 0 on failure.
			RInputEvent* pie,						// In:  Most recent user input event.
														// Out: pie->sUsed = TRUE, if used.
			RImage*	pimDst			= NULL);	// Where to draw dialog and rspBlit from.
														// If this is NULL, the system buffer is
														// used.
														// rspBlit is used to update this to the
														// screen image unless pimDst is the screen
														// image.

		// Operates RMsgBox for one iteration.  Does NOT Draw().
		// Check m_pguiFocus to determine RGuiItem with focus.
		ULONG DoModeless(			// Returns item clicked or 0, if none.
			RInputEvent* pie);	// In:  Most recent user input event.
										// Out: pie->sUsed = TRUE, if used.

		// Destroys all allocated items (i.e., items NOT added with
		// AddItem(...) (i.e., items allocated by RMsgBox (i.e., AddButton, 
		// AddText, and AddEdit))).
		void RemoveAll(void);	// Returns nothing.

		////////////////////////////////////////////////////////////////////////
		// Querries.
		////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////

	public:	// Non-Static Callbacks.

	public:	// Static Callbacks.

		// Called by child items to let us know when they are clicked in.
		static void ItemBtnUpCall(RGuiItem* pgui);

//////////////////////////////////////////////////////////////////////////////

	public:	// Querries.

//////////////////////////////////////////////////////////////////////////////

	protected:	// Internal functions.

		// Add a GUI item already allocated by this RMsgBox.
		short AddItem(			// Returns 0 on success.
			RGuiItem* pgui,	// Item to add.
			char* pszText,		// Text for item.
			short	sX,			// X position in RMsgBox dlg.
			short	sY,			// Y position in RMsgBox dlg.
			ULONG	ulId,			// ID to return if this item is chosen.
									// There will be no response to this item
									// if lId is 0.
			short	sAddW = 0,	// Additional width for guis that require more.
			short	sAddH = 0);	// Additional height for guis that require more.

		// Copies parameters from RMsgBox into *pgui.
		void CopyParms(RGuiItem* pgui);

//////////////////////////////////////////////////////////////////////////////

	public:	// Member variables.
		MsgBoxCall	m_mbcUser;	// User callback.  Called iteratively with 
										// RGuiItem::m_ulUser as argument.
										// If not defined (i.e., NULL), rspDoSystem
										// and CHot::Do() are called.
		ULONG			m_ulId;		// ID of control that caused DoModal to end.

	protected:	// Internal typedefs.

	protected:	// Protected member variables.
		RList<RGuiItem>	m_listGuis;	// List of RGuiItems allocated by this
												// RMsgBox.
	};

#endif // MSGBOX_H
//////////////////////////////////////////////////////////////////////////////
//	EOF
//////////////////////////////////////////////////////////////////////////////
