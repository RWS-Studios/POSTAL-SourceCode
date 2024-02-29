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
// MultiBtn.h
// 
// History:
//		04/10/97 JMI	Started this using RPushBtn as a template.
//
//		04/17/97	JMI	Added Load and Save components.
//
//		04/22/97	JMI	Added NextState().
//
//		09/22/97	JMI	Also, added friend class CMultiBtnPropPage for GUI 
//							editor.
//
//////////////////////////////////////////////////////////////////////////////
//
// Please see the CPP file for an explanation of this API.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef MULTIBTN_H
#define MULTIBTN_H


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
	#include "ORANGE/GUI/btn.h"
#else
	#include "btn.h"
#endif // PATHS_IN_INCLUDES

//////////////////////////////////////////////////////////////////////////////
// Macros.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Typedefs.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
class RMultiBtn : public RBtn
	{
	public:	// Construction/Destruction.
		// Default constructor.
		RMultiBtn(void);
		// Destructor.
		~RMultiBtn(void);

//////////////////////////////////////////////////////////////////////////////

	public:	// Typedefs/enums.

//////////////////////////////////////////////////////////////////////////////

	public:	// Methods.

		////////////////////////////////////////////////////////////////////////
		// Methods.
		////////////////////////////////////////////////////////////////////////

		// Cursor event notification.
		// Events in event area.
		virtual						// If you override this, call this base if possible.
		void CursorEvent(			// Returns nothing.
			RInputEvent* pie);	// In:  Most recent user input event.             
										// Out: pie->sUsed = TRUE, if used.

		// Draw background resource, if one is specified.
		// Utilizes m_*BkdRes* parameters to get, place, and BLiT the resource.
		virtual							// Overridden here.
		void DrawBackgroundRes(		// Returns nothing.
			RImage* pim = NULL);		// Dest image, uses m_im, if NULL.

		// Set number of states.
		// This will clear all existing state images including the feedback
		// state.
		short SetNumStates(		// Returns 0 on success.
			short sNumStates);	// In:  New number of states.

		// Set button state or feedback state image.
		// The feedback state image is always the first image.
		short SetState(		// Returns 0 on success.
			RImage*	pim,		// In:  Image for state sState.
			short		sState);	// In:  State to update (0 == feedback state,
									// 1..n == state number).

		// Set button state or feedback state image.
		// The feedback state image is always the first image.
		short SetState(			// Returns 0 on success.
			char*	pszImageName,	// In:  File name of image for state sState.
			short		sState);		// In:  State to update (0 == feedback state,
										// 1..n == state number).

		// Clear button state or feedback state image.
		// The feedback state image is always the first image.
		void ClearState(			// Returns nothing.
			short	sState);			// In:  State to clear (0 == feedback state,
										// 1..n == state number).

		// Go to the next logical state.
		short NextState(void);	// Returns new state.


		////////////////////////////////////////////////////////////////////////
		// Querries.
		////////////////////////////////////////////////////////////////////////

		// Get the current image for the specified state.
		RImage* GetState(			// Returns image, if available; NULL, otherwise.
			short	sState);			// In:  State to get (0 == feedback state,
										// 1..n == state number).

//////////////////////////////////////////////////////////////////////////////

	public:	// Static

//////////////////////////////////////////////////////////////////////////////

	public:	// Querries.

//////////////////////////////////////////////////////////////////////////////

	protected:	// Internal functions.

		// Destroys current state bitmaps.
		void DestroyStates(void);	// Returns nothing.

		// Read item's members from file.
		virtual				// Overridden here.
		short ReadMembers(			// Returns 0 on success.
			RFile*	pfile,			// File to read from.
			U32		u32Version);	// File format version to use.

		// Write item's members to file.
		virtual				// Overridden here.
		short WriteMembers(			// Returns 0 on success.
			RFile*	pfile);			// File to write to.

//////////////////////////////////////////////////////////////////////////////

	public:	// Member variables.

		short		m_sState;		// The button's current state, 0..m_sNumStates - 1.
		short		m_sNumStates;	// Number of button states.
		RImage**	m_papimStates;	// Ptr to array of m_sNumStates + 1 ptrs to button 
										// state images.

	protected:	// Internal typedefs.

	protected:	// Protected member variables.

	///////////////////////////////////////////////////////////////////////////
	// Friends.
	///////////////////////////////////////////////////////////////////////////
	friend class CMultiBtnPropPage;

	};

#endif // MULTIBTN_H
//////////////////////////////////////////////////////////////////////////////
//	EOF
//////////////////////////////////////////////////////////////////////////////
