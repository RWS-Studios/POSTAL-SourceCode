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
// ProcessGui.CPP
// 
// History:
//		07/03/97 JMI	Started.
//
//		07/03/97	JMI	Had not implemented the update func to the feature level
//							the comments implied.  Implemented.
//
//		07/03/97	JMI	Added Init() and Kill().  Init() is called by the con-
//							structor.  Kill() is called by the destructor.  The de-
//							structor used to call Unprepare() but the problem with
//							that was that Unprepare() did more than just deallocate
//							the dynamic stuff and that stuff shouldn't have been done
//							upon destruction.  So now Unprepare() and the destructor
//							call Kill() to take care of dynamic items.
//
//		08/22/97	JMI	Now uses rspGeneralLock/Unlock() to ensure the composite
//							buffer is locked as necessary.
//
//		09/01/97	JMI	Now only sets the focus to the first item if there's no
//							focus or the current focus is not a descendant of the
//							root item.
//							Also, added special behavior flags, m_flags.
//
//		09/06/97	JMI	Now filters out the high word of ie.lKey when checking
//							for enter.  This filters out the the key flags (i.e., 
//							shift, control, alt, system).
//
//////////////////////////////////////////////////////////////////////////////
//
// See .h for details.
//
//////////////////////////////////////////////////////////////////////////////

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
	#include "ORANGE/GUI/ProcessGui.h"
#else
	#include "ProcessGui.h"
#endif

//////////////////////////////////////////////////////////////////////////////
// Module specific macros.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Module specific typedefs.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Exported (extern) variables.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Module specific (static) variables / Instantiate class statics.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Module specific (static) protos.
//////////////////////////////////////////////////////////////////////////////

// ID of last item pressed.
long RProcessGui::ms_lPressedId;

//////////////////////////////////////////////////////////////////////////////
// Helper Functions.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Draw dirty areas if not going direct to screen and
// remove rects from pdrl.
//////////////////////////////////////////////////////////////////////////////
inline
void DrawDirty(			// Returns nothing.
	RImage*	pimDst,		// In:  Destination buffer.
	RImage*	pimScr,		// In:  Screen buffer.
	RDirtyRects* pdrl)	// In:  Dirty rect list of areas to be updated.
	{
	RDRect*	pdr;
	// If not going direct to screen . . .
	if (pimDst != pimScr)
		{
		rspShieldMouseCursor();

		pdr	= pdrl->GetHead();
		while (pdr != NULL)
			{
			// Update screen.
			rspBlit( pimDst, pimScr,
						pdr->sX, pdr->sY, 
						pdr->sX, pdr->sY,
						pdr->sW, pdr->sH);

			pdrl->Remove();

			delete pdr;

			pdr	= pdrl->GetNext();
			}

		rspUnshieldMouseCursor();
		}
	}

//////////////////////////////////////////////////////////////////////////////
// Functions.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Prepare to handle a GUI.
// This must be called to setup components before Do*Gui() is called.
//////////////////////////////////////////////////////////////////////////////
short RProcessGui::Prepare(			// Returns 0 on success.
	RGuiItem* pguiRoot,					// In:  GUI to be processed.
	RGuiItem* pguiOk/* = NULL*/,		// In:  If not NULL, specifies GUI 
												// activated by ENTER key.
	RGuiItem* pguiCancel/* = NULL*/)	// In:  If not NULL, specifies GUI
												// activated by ESCAPE key.
	{
	short	sRes	= 0;	// Assume success.

	// Create erase buffer . . .
	if (m_imEraser.CreateImage(
		pguiRoot->m_im.m_sWidth,
		pguiRoot->m_im.m_sHeight,
		RImage::BMP8,
		0,	// Use default for this width/format/depth.
		pguiRoot->m_im.m_sDepth) == 0)
		{
		// Success.
		}
	else
		{
		TRACE("Prepare():  CreateImage() failed.\n");
		sRes	= -1;
		}

	m_pguiOk			= pguiOk;
	m_pguiCancel	= pguiCancel;

	// These need to call us back.
	if (pguiOk)
		{
		SetGuiToNotify(pguiOk);
		}
	if (pguiCancel)
		{
		SetGuiToNotify(pguiCancel);
		}

	// Make visible and activate.
	pguiRoot->SetVisible(TRUE);

	if (RGuiItem::ms_pguiFocus)
		{
		// If the root is not an ancestor of the current focus . . .
		if (RGuiItem::ms_pguiFocus->IsAncestor(pguiRoot) == FALSE)
			{
			// Clear focus.
			RGuiItem::SetFocus(NULL);
			}
		}

	// If no current focus . . .
	if (RGuiItem::ms_pguiFocus == NULL)
		{
		// Set focus to first child control or none if there's none.
		pguiRoot->SetFocus(pguiRoot->m_listguiChildren.GetHead());
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
// This should be called to clean up components when no more Do*Gui()
// calls are to be made on the current GUI.
//////////////////////////////////////////////////////////////////////////////
void RProcessGui::Unprepare(void)	// Returns nothing.
	{
	// If we're allowed . . .
	if ( (m_sFlags & NoCleanScreen) == 0)
		{
		RImage*	pimScr;
		rspNameBuffers(NULL, &pimScr);

		// Draw remaining dirty areas.
		DrawDirty(m_pimLastDst, pimScr, &m_drl);
		}

	// Deallocate dynamic schtuff.
	Kill();
	}

//////////////////////////////////////////////////////////////////////////////
// Setup GUI to notify this class of presses.
//////////////////////////////////////////////////////////////////////////////
void RProcessGui::SetGuiToNotify(	// Returns nothing.
	RGuiItem*	pgui)						// GUI item to setup to notify this class
												// of presses.
	{
	pgui->m_bcUser	= PressedCall;
	}

//////////////////////////////////////////////////////////////////////////////
// Set all unclaimed GUIs' callbacks to go through this class.
//////////////////////////////////////////////////////////////////////////////
void RProcessGui::SetGuisToNotify(	// Returns nothing.
	RGuiItem* pguiRoot)					// In:  All unclaimed child GUIs' 
												// callbacks are directed through
												// ProcessGui.
	{
	// Enum GUIs directing unclaimed callbacks to PressedCall.
	RGuiItem*	pguiChild	= pguiRoot->m_listguiChildren.GetHead();
	while (pguiChild != NULL)
		{
		// If callback unclaimed . . .
		if (pguiChild->m_bcUser == NULL)
			{
			SetGuiToNotify(pguiChild);
			}

		// Do this item's children.
		SetGuisToNotify(pguiChild);

		pguiChild	= pguiRoot->m_listguiChildren.GetNext();
		}
	}

//////////////////////////////////////////////////////////////////////////////
// Call this to process a GUI modally.  Once this function is called,
// it will not return until a GUI pressed callback occurs on a GUI
// with an ID other than 0 or the update callback, m_fnUpdate, if any,
// returns non-zero.
//////////////////////////////////////////////////////////////////////////////
long RProcessGui::DoModal(				// Returns ID of pressed GUI that terminated 
												// modal loop or value returned from 
												// m_fnUpdate, if any.
	RGuiItem* pgui,						// In:  GUI to be processed or NULL.
	RGuiItem* pguiOk/* = NULL*/,		// In:  If not NULL, specifies GUI 
												// activated by ENTER key.
	RGuiItem* pguiCancel/* = NULL*/,	// In:  If not NULL, specifies GUI
												// activated by ESCAPE key.
	RImage* pimDst /*= NULL*/)			// Where to draw dialog and rspBlit from.
												// If this is NULL, the system buffer is
												// used.
												// rspBlit is used to update this to the
												// screen image unless pimDst is the screen
												// image.
	{
	long	lId	= 0;

	// Set up ptrs and erase buffer.
	Prepare(pgui, pguiOk, pguiCancel);

	RInputEvent	ie;

	// Process GUI.
	while (rspGetQuitStatus() == FALSE && lId == 0)
		{
		ie.type	= RInputEvent::None;
		// If there's a user update func . . .
		if (m_fnUpdate)
			{
			lId = m_fnUpdate(&ie);
			}
		else
			{
			// Minimum system callage.
			rspDoSystem();
			rspGetNextInputEvent(&ie);
			}
		
		if (lId == 0)
			{
			lId	= DoModeless(pgui, &ie, pimDst);
			}
		}

	// Clean up ptrs, erase buffer, and dirty rect list.
	Unprepare();

	// Return GUI pressage that terminated.
	return lId;
	}

//////////////////////////////////////////////////////////////////////////////
// Call this to process a GUI modelessly.  This function processes the
// GUI for only one iteration allowing the caller continuous control.
//////////////////////////////////////////////////////////////////////////////
long RProcessGui::DoModeless(		// Returns ID of pressed GUI or value.
	RGuiItem* pgui,					// In:  GUI to be processed or NULL.
	RInputEvent* pie,					// In:  Input event to process.
	RImage* pimDst /*= NULL*/)		// Where to draw dialog and rspBlit from.
											// If this is NULL, the system buffer is
											// used.
											// rspBlit is used to update this to the
											// screen image unless pimDst is the screen
											// image.
	{
	RImage*	pimScr;

	// We update these things every frame in case the video mode has changed.
	if (pimDst == NULL)
		{
		// Get both buffers.
		rspNameBuffers(&pimDst, &pimScr);
		}
	else
		{
		// Just need screen buffer.
		rspNameBuffers(NULL, &pimScr);
		}

	// Update dirty rect clipping in case video mode changed.
	m_drl.m_sClipX	= pimDst->m_sWidth - 1;
	m_drl.m_sClipY	= pimDst->m_sHeight - 1;

	// Clear callback ID.
	ms_lPressedId	= 0;

	// Update hots with input event.
	// If the event is used by RHot, the sUsed flag will be
	// set to TRUE.
	// RHot calls GUIs with mouse events, if applicable.
	pgui->m_hot.Do(pie);

	// Update GUIs.
	// This GUI item call sends the event, if not yet used,
	// to the GUI focused, and checks for keys that might
	// change the focus.
	pgui->DoFocus(pie);

	// If unused key event . . .
	if (pie->sUsed == FALSE && pie->type == RInputEvent::Key)
		{
		// Check some of our own keys.
		switch (pie->lKey & 0x0000FFFF)
			{
			// Enter == Ok.
			case '\r':
				RSP_SAFE_GUI_REF_VOID(m_pguiOk, SetClicked(TRUE) );
				PressedCall(m_pguiOk);
				// Used key.
				pie->sUsed	= TRUE;
				break;
			// Escape == Cancel.
			case 27:
				RSP_SAFE_GUI_REF_VOID(m_pguiCancel, SetClicked(TRUE) );
				PressedCall(m_pguiCancel);
				// Used key.
				pie->sUsed	= TRUE;
				break;
			}
		}

	// Lock buffer down for access.
	rspGeneralLock(pimDst);

	// Store area of composite buffer that is to be dirtied.
	RRect rect(0, 0, pimDst->m_sWidth, pimDst->m_sHeight);
	rspBlit(
		pimDst,						// Source.
		&m_imEraser,				// Destination.
		pgui->m_sX,					// Source.
		pgui->m_sY,					// Source.
		0,								// Destination.
		0,								// Destination.
		m_imEraser.m_sWidth,		// Both.
		m_imEraser.m_sHeight,	// Both.
		NULL,							// Destination.
		&rect );	// Source.

	// If direct to screen . . .
	if (pimDst == pimScr)
		{
		// Shield cursor.
		rspShieldMouseCursor();
		}

	// Draw the GUI tree.
	pgui->Draw(pimDst);

	// Add rectangle where GUI drew.
	m_drl.Add(
		pgui->m_sX,
		pgui->m_sY,
		pgui->m_im.m_sWidth,
		pgui->m_im.m_sHeight);

	// If not going direct to screen . . .
	if (pimDst != pimScr)
		{
		// We must unlock the buffer so DrawDirty()
		// can do an rspUpdateDisplay().
		rspGeneralUnlock(pimDst);
		}

	// Update areas of RSPiX display.
	DrawDirty(pimDst, pimScr, &m_drl);

	// If not going direct to screen . . .
	if (pimDst != pimScr)
		{
		// Since we unlocked before DrawDirty(),
		// we must relock now so we can continue accessing
		// the buffer.
		rspGeneralLock(pimDst);
		}

	// Undirty buffer of GUIs.
	rspBlit(
		&m_imEraser,				// Source.
		pimDst,						// Destination.
		0,								// Source.
		0,								// Source.
		pgui->m_sX,					// Destination.
		pgui->m_sY,					// Destination.
		m_imEraser.m_sWidth,		// Both.
		m_imEraser.m_sHeight,	// Both.
		NULL,							// Destination.
		NULL);						// Source.

	// Done with buffer.
	rspGeneralUnlock(pimDst);

	// If direct to screen . . .
	if (pimDst == pimScr)
		{
		// Unshield.
		rspUnshieldMouseCursor();
		}
	else	// If not direct to screen . . .
		{
		// Add rectangle that got erased to be update next iteration
		// or in Unprepare().
		m_drl.Add(
			pgui->m_sX,
			pgui->m_sY,
			pgui->m_im.m_sWidth,
			pgui->m_im.m_sHeight);
		}

	// Remember last dst.
	m_pimLastDst	= pimDst;

	// Check GUI pressage.
	return ms_lPressedId;
	}

///////////////////////////////////////////////////////////////////////////
// Querries.
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
// Internal functions.
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// Initialize instantiable data members.
// (protected).
///////////////////////////////////////////////////////////////////////////
void RProcessGui::Init(void)	// Returns nothing.
	{
	m_pguiOk			= NULL;
	m_pguiCancel	= NULL;
	m_fnUpdate		= NULL;
	m_pimLastDst	= NULL;
	m_sFlags			= 0;
	}

///////////////////////////////////////////////////////////////////////////
// Deallocate any instantiable dynamic members.
// (protected).
///////////////////////////////////////////////////////////////////////////
void RProcessGui::Kill(void)	// Returns nothing.
	{
	// Destroy image data.
	m_imEraser.DestroyData();
	// There should be no palette associated with this image.

	// Empty dirty rect list.
	m_drl.Empty();

	// Reset (don't call Init() it does more).
	m_pguiOk			= NULL;
	m_pguiCancel	= NULL;
	m_pimLastDst	= NULL;
	}

///////////////////////////////////////////////////////////////////////////
// Internal static functions.
///////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// Callback for pressed GUIs.
//////////////////////////////////////////////////////////////////////////////
// static.
void RProcessGui::PressedCall(	// Returns nothing.
	RGuiItem*		pgui)				// In:  Pressed GUI.
	{
	if (pgui)
		{
		ms_lPressedId	= pgui->m_lId;
		}
	}

///////////////////////////////////////////////////////////////////////////////
// EOF
///////////////////////////////////////////////////////////////////////////////
