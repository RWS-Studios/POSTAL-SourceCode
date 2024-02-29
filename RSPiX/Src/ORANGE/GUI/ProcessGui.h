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
// ProcessGui.H
// 
// History:
//		07/03/97 JMI	Started.
//
//		07/03/97	JMI	Had not implemented the update func to the feature level
//							the comments implied.  Implemented.
//
//		07/03/97	JMI	Ammended miscomment.
//
//		07/03/97	JMI	Added Init() and Kill().  Init() is called by the con-
//							structor.  Kill() is called by the destructor.  The de-
//							structor used to call Unprepare() but the problem with
//							that was that Unprepare() did more than just deallocate
//							the dynamic stuff and that stuff shouldn't have been done
//							upon destruction.  So now Unprepare() and the destructor
//							call Kill() to take care of dynamic items.
//
//		09/01/97	JMI	Added special behavior flags, m_flags.
//
//////////////////////////////////////////////////////////////////////////////
//
// ProcessGui process modal or non-modal GUI input and renders the GUI using
// dirty rectangles.  The update type is customizable via a callback.  There
// is also a default implementation for ease of use.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef PROCESSGUI_H
#define PROCESSGUI_H

//////////////////////////////////////////////////////////////////////////////
// C Headers -- Must be included before RSPiX.h b/c RSPiX utilizes SHMalloc.
//////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// RSPiX Headers.
// If PATHS_IN_INCLUDES macro is defined, we can utilize relative
// paths to a header file.  In this case we generally go off of our
// RSPiX root directory.  System.h MUST be included before this macro
// is evaluated.  System.h is the header that, based on the current
// platform (or more so in this case on the compiler), defines 
// PATHS_IN_INCLUDES.  Blue.h includes system.h so you can include that
// instead.
///////////////////////////////////////////////////////////////////////////////
#include "System.h"

#ifdef PATHS_IN_INCLUDES
	#include "ORANGE/GUI/guiItem.h"
	#include "ORANGE/DirtRect/DirtRect.h"
#else
	#include "guiItem.h"
	#include "DirtRect.h"
#endif

//////////////////////////////////////////////////////////////////////////////
// Macros.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Protos.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Typedefs.
//////////////////////////////////////////////////////////////////////////////
class RProcessGui
	{
	///////////////////////////////////////////////////////////////////////////
	// Typedefs, enums, etc.
	///////////////////////////////////////////////////////////////////////////
	public:
		typedef long (*UpdateFunc)(	// Returns a non-zero ID to abort or zero
												// to continue.
			RInputEvent*	pie);			// Out: Next input event to process.

		typedef enum
			{
			NoCleanScreen	= 0x0001		// Don't clean the dest buffer when exiting.
			} Flags;

	///////////////////////////////////////////////////////////////////////////
	// Con/Destruction.
	///////////////////////////////////////////////////////////////////////////
	public:
		RProcessGui()
			{
			Init();
			}

		~RProcessGui() 
			{
			Kill();
			}

	///////////////////////////////////////////////////////////////////////////
	// Methods.
	///////////////////////////////////////////////////////////////////////////
	public:
		// Prepare to handle a GUI.
		// This must be called to setup components before DoModeless() is called.
		// (DoModal() does this automatically).
		short Prepare(							// Returns 0 on success.
			RGuiItem* pgui,					// In:  GUI to be processed.
			RGuiItem* pguiOk = NULL,		// In:  If not NULL, specifies GUI 
													// activated by ENTER key.
			RGuiItem* pguiCancel = NULL);	// In:  If not NULL, specifies GUI
													// activated by ESCAPE key.

		// This should be called to clean up components when no more DoModeless()
		// calls are to be made on the current GUI.
		// (DoModal() does this automatically).
		void Unprepare(void);	// Returns nothing.

		// Setup GUI to notify this class of presses.
		void SetGuiToNotify(		// Returns nothing.
			RGuiItem*	pgui);	// In:  GUI item to setup to notify this class
										// of presses.

		// Set all unclaimed GUIs' callbacks to go through this class.
		void SetGuisToNotify(	// Returns nothing.
			RGuiItem* pguiRoot);	// In:  All unclaimed child GUIs' 
										// callbacks are directed through
										// ProcessGui.

		// Call this to process a GUI modally.  Once this function is called,
		// it will not return until a GUI pressed callback occurs on a GUI
		// with an ID other than 0 or the update callback, m_fnUpdate, if any,
		// returns non-zero.
		long DoModal(							// Returns ID of pressed GUI that terminated 
													// modal loop or value returned from 
													// m_fnUpdate, if any.
			RGuiItem* pgui,					// In:  GUI to be processed or NULL.
			RGuiItem* pguiOk = NULL,		// In:  If not NULL, specifies GUI 
													// activated by ENTER key.
			RGuiItem* pguiCancel = NULL,	// In:  If not NULL, specifies GUI
													// activated by ESCAPE key.
			RImage* pimDst = NULL);			// Where to draw dialog and rspBlit from.
													// If this is NULL, the system buffer is
													// used.
													// rspBlit is used to update this to the
													// screen image unless pimDst is the screen
													// image.

		// Call this to process a GUI modelessly.  This function processes the
		// GUI for only one iteration allowing the caller continuous control.
		long DoModeless(						// Returns ID of pressed GUI or value.
			RGuiItem* pgui,					// In:  GUI to be processed or NULL.
			RInputEvent* pie,					// In:  Input event to process.
			RImage* pimDst = NULL);			// Where to draw dialog and rspBlit from.
													// If this is NULL, the system buffer is
													// used.
													// rspBlit is used to update this to the
													// screen image unless pimDst is the screen
													// image.

	///////////////////////////////////////////////////////////////////////////
	// Querries.
	///////////////////////////////////////////////////////////////////////////
	public:


	///////////////////////////////////////////////////////////////////////////
	// Internal functions.
	///////////////////////////////////////////////////////////////////////////
	protected:

		// Initialize instantiable data members.
		void Init(void);	// Returns nothing.

		// Deallocate any instantiable dynamic members.
		void Kill(void);	// Returns nothing.

	///////////////////////////////////////////////////////////////////////////
	// Internal static functions.
	///////////////////////////////////////////////////////////////////////////
	protected:

		// Callback for pressed GUIs.
		static void PressedCall(	// Returns nothing.
			RGuiItem*		pgui);	// In:  Pressed GUI.

	///////////////////////////////////////////////////////////////////////////
	// Instantiable data.
	///////////////////////////////////////////////////////////////////////////
	public:

		RDirtyRects	m_drl;			// List of dirtied areas.
		RImage		m_imEraser;		// Erase image.
		RImage*		m_pimLastDst;	// Last composite buffer (used
											// in Unprepare() to clean up display).
		RGuiItem*	m_pguiOk;		// GUI activated by ENTER.
		RGuiItem*	m_pguiCancel;	// GUI activated by ESCAPE.

		UpdateFunc	m_fnUpdate;		// Callback to do updates, if not using
											// default handling.

		short			m_sFlags;		// See Flags.

	///////////////////////////////////////////////////////////////////////////
	// Static data.
	///////////////////////////////////////////////////////////////////////////
	public:

		// ID of last item pressed.
		static long ms_lPressedId;
	};

#endif	// PROCESSGUI_H
//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
