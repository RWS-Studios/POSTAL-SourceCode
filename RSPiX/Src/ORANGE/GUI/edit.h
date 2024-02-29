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
// Edit.H
// 
// History:
//		01/18/97 JMI	Started tracking history of this file.
//							Converted Do() to take an RInputEvent* instead of a 
//							long*.
//
//		01/21/97	JMI	Added ReadMembers() and WriteMembers() overloads to read
//							and write members of this class.
//
//		03/14/97	JMI	Added ClipCaret().
//
//		03/19/97	JMI	Converted to using the RHot::m_iecUser (was using
//							RHot::m_epcUser) so HotCall and CursorEvent now take
//							RInputEvent ptrs.
//
//		08/25/97	JMI	Added m_sFirstVisibleCharIndex which is the first visible
//							character in the field.
//
//		09/22/97	JMI	Also, added friend class CEditPropPage for GUI 
//							editor.
//
//		09/23/97	JMI	Flags for m_sBehavior started at 0 instead of 1.  Fixed.
//
//////////////////////////////////////////////////////////////////////////////
//
// See CPP for description.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef EDIT_H
#define EDIT_H
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
	#include "ORANGE/GUI/txt.h"
#else
	#include "txt.h"
#endif // PATHS_IN_INCLUDES

//////////////////////////////////////////////////////////////////////////////
// Macros.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Typedefs.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
class REdit : public RTxt
	{
	public:	// Construction/Destruction.
		// Default constructor.
		REdit(void);
		// Destructor.
		~REdit(void);

//////////////////////////////////////////////////////////////////////////////

	public:	// Typedefs.
		typedef struct
			{
			short sX;
			short sY;
			} Point;

		typedef void (*EditNotifyCall)(	// Called when a user input notification
													// should occur.
			REdit*	pedit);					// this.

	public:	// Enums.
		enum					// Values for member flags.
			{					
			// Values for m_sBehavior.
			None			= 0x0000,
			NumbersOnly	= 0x0001,	// Allow only numbers as input.
			Multiline	= 0x0002		// Allow GK/SK_ENTER to cause CR/LF.
			};

	public:	// Methods.

		////////////////////////////////////////////////////////////////////////
		// Methods.
		////////////////////////////////////////////////////////////////////////

		// Draw this item and all its subitems into the provided RImage.
		// This override adds in the caret.
		virtual						// If you override this, call this base if possible.
		short Draw(					// Returns 0 on success.
			RImage* pimDst,		// Destination image.
			short sDstX	= 0,		// X position in destination.
			short sDstY	= 0,		// Y position in destination.
			short sSrcX = 0,		// X position in source.
			short sSrcY = 0,		// Y position in source.
			short sW = 0,			// Amount to draw.
			short sH = 0,			// Amount to draw.
			RRect* prc = NULL);	// Clip to.

		// Draw text in m_szText in m_u32TextColor with transparent
		// background at sX, sY with sW and m_sJustification.
		// Does nothing if m_szText is empty.
		virtual						// If you override this, call this base if possible.
		short DrawText(			// Returns 0 on success.
			short sX,				// X position in image.
			short sY,				// Y position in image.
			short sW = 0,			// Width of text area.
			short	sH = 0,			// Height of test area.
			RImage* pim = NULL);	// Destination image.  NULL == use m_im.

		// Does REdit stuff like check for text, update caret, and draw new 
		// text.
		virtual	// If you override this, call this base if possible.
		void Do(						// Returns nothing.
			RInputEvent* pie);	// In:  Most recent user input event.
										// Out: pie->sUsed = TRUE, if used.

		// Calls user input error callback if one is set.
		virtual						// If you override this, call this base if 
										// possible.
		void NotifyCall(void)	// Returns nothing.
			{ if (m_encCall != NULL) (*m_encCall)(this); }

		// Cursor event notification.
		// Events in event area.
		virtual						// If you override this, call this base if possible.
		void CursorEvent(			// Returns nothing.
			RInputEvent* pie);	// In:  Most recent user input event.             
										// Out: pie->sUsed = TRUE, if used.

		// Called by the static implementation of SetFocus() on the item losing
		// the focus.
		// I'm not exactly sure of the effects of calling SetFocus() from within
		// this function, but I would suggest that it might not be as expected.
		virtual				// If you override this, call this base if possible.
		void OnLoseFocus(void)
			{
			// Call base class implementation.
			RTxt::OnLoseFocus();

			// Give up our caret, if we had it.
			m_sCaretState	= 0;
			}

		////////////////////////////////////////////////////////////////////////
		// Querries.
		////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////

	public:	// Static

//////////////////////////////////////////////////////////////////////////////

	protected:	// Internal functions.

		// Read item's members from file.
		virtual				// Overridden here.
		short ReadMembers(			// Returns 0 on success.
			RFile*	pfile,			// File to read from.
			U32		u32Version);	// File format version to use.

		// Write item's members to file.
		virtual				// Overridden here.
		short WriteMembers(			// Returns 0 on success.
			RFile*	pfile);			// File to write to.

		// Clips the caret to within the string length.
		void ClipCaret(void);		// Returns nothing.

//////////////////////////////////////////////////////////////////////////////

	public:	// Member variables.
		char	m_cCaretChar;		// Character to use as caret.
		U32	m_u32CaretColor;	// Color to use for caret.
		short	m_sCaretPos;		// Text position of caret.
		long	m_lCaretBlinkRate;// Rate at which character blinks in ms.  Can be
										// 0 indicating no blinkage.

		short	m_sMaxText;			// Maximum text to allow.  Limited to GUI_MAX_STR.

		short m_sBehavior;		// Flags.  See enums above.

		EditNotifyCall	m_encCall;	// Callback when a user input notification 
											// should occur such as too much input or 
											// invalid character generated (e.g., alphas 
											// in NUMBERS_ONLY mode).  A good place to 
											// generate a beep or something.

		long	m_lNextCaretUpdate;	// Time in ms of next caret update.
		short	m_sCaretState;			// Current state the caret is in until
											// m_lNextCaretUpdate. (0 == hidden, 
											// 1 == shown).

		short	m_sFirstVisibleCharIndex;	// Index of the first visible character.

		Point	m_aptTextPos[GUI_MAX_STR];	// Positions for characters in m_szText.
													// This is no longer dynamically allocated
													// do to the amount of overhead that added
													// when adding characters.


	protected:	// Internal typedefs.

	protected:	// Protected member variables.

	///////////////////////////////////////////////////////////////////////////
	// Friends.
	///////////////////////////////////////////////////////////////////////////
	friend class CEditPropPage;

	};

#endif // EDIT_H
//////////////////////////////////////////////////////////////////////////////
//	EOF
//////////////////////////////////////////////////////////////////////////////
