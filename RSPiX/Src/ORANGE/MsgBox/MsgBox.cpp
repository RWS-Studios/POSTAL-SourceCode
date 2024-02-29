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
// MsgBox.CPP
// 
// History:
//		08/20/96 JMI	Started.
//
//		08/21/96	JMI	Updated to make new lib.
//
//		09/24/96	JMI	Now uses CGuiItem::GetTopLeftBorderThickness() and
//							CGuiItem::GetBottomRightBorderThickness() instead of
//							old CGuiItem::GetTotalBorderThickness().
//
//		10/01/96	JMI	DrawDirty() now uses rsp[Un]ShieldCursor().  DoModal()
//							was always overwriting m_ulId by when callbacks were
//							used..fixed.  DoModal() no longer calls GetNext(phot).
//							It now calls GetNext() with no args.
//
//		10/03/96	JMI	Added DoModeless() to handle simple non-modal dirty
//							work.
//
//		10/31/96	JMI	Changed:
//							Old label:			New label:
//							=========			=========
//							CMsgBox				RMsgBox
//							CImage				RImage
//							CDirtyRect			RDirtyRects	<-- Yes.  Now plural.
//							DRECT					RDRect
//							PDRECT				RDRect*
//							CHot					RHot
//							CList					RList
//							BMP8					RImage::BMP8
//							CGuiItem				RGuiItem
//							CTxt					RTxt
//							CBtn					RBtn
//							CDlg					RDlg
//							CEdit					REdit
//
//		11/01/96	JMI	Also, changed all members referenced in RImage to  
//							m_ and all position/dimension members referenced in
//							RImage to type short usage.                        
//
//		11/12/96	JMI	Changed GKF_SHIFT to RSP_GKF_SHIFT.
//
//		11/14/96	JMI	Now adds some additional width to Edits when added
//							with AddEdit() so they can fit their caret with their
//							text.
//
//		11/15/96	JMI	Now provides a clip rect when getting new erase data
//							for the dialog and when sets the m_sClipX/Y members of
//							RDirtyRects in DoModal().
//
//		12/04/96	JMI	DoModless() and, consequently, DoModal() now take a
//							pointer to an rspGetKey() formmatted key that is passed
//							to REdits, if they have the focus.  Also, now moves
//							focus through the gui items even if an edit field is not
//							active.
//							Now initializes RRects using new syntax.
//
//		12/16/96	JMI	Focus is can now be specific to any type.
//
//		12/19/96	JMI	Upgraded to new RFont/RPrint.
//
//		12/19/96	JMI	In CopyParms(), I was getting the height from the wrong
//							m_pprint (was using the one in pgui, instead of 'this').
//
//		12/31/96	JMI	Reduced focus support such that it mostly utilized that
//							in RGuiItem, but preserved RMsgBox.SetFocus() so it could
//							steal the focus from an REdit.  Note that this would be
//							a good opportunity for a OnLoseFocus() virtual func so
//							that REdit could handle this itself.
//
//		12/31/96	JMI	Removed RMsgBox.SetFocus() since REdit now handles its
//							own garbage in REdit.OnLoseFocus().
//
//		01/18/97	JMI	NOTE! HIGHLY TEMP FIX:  Commented out calls to Do() with
//							a long*.  Now Do() takes an RInputEvent*.  RMsgBox should
//							be changed to taking this as a parm instead of lKey as
//							well.  The reason I don't just go ahead and do it is b/c
//							I'm not sure if this API is worth keeping since it is
//							currently unused and is mostly obsoleted by new abilites
//							in RGuiItem (e.g., Load(), LoadInstantiate(),
//							Focus/NextPrev(), DoFocus(), etc.).
//
//		01/19/97	JMI	This is actually used by the Menu API, so I upated it.
//							Converted Do*() to taking an RInputEvent* instead of a
//							long*.
//
//		01/23/97	JMI	Now calls m_hot.Do() directly and does not need to
//							deactivate other hotboxes.
//
//		04/11/97	JMI	Now uses m_sFontCellHeight instead of GetPos() to 
//							determine cell height.
//
//		04/24/97	JMI	Add additional parms to Add*() to add extra size to a
//							GUI.
//
//		04/24/97	JMI	CopyParms() now copies new m_u32TextShadowColor parm.
//
//////////////////////////////////////////////////////////////////////////////
//
// This a GUI item that is based on CDlg derived from the basic RGuiItem. 
// This utilizes the m_ulUserData, m_ulUserInstance, and m_bcUser fields.  If 
// you use any of these fields for other purposes MAKE SURE that m_bcUser is
// set to NULL or any other callback besides ItemBtnUpCall!!!!!!!!!!
// 
// If you do m_mbcUser is NULL, DoModal() will call rspDoSystem(), RHot::Do(),
// and this->Do().  If m_mbcUser is not NULL, NONE OF THESE WILL BE CALLED.
// You must call whatever is necessary for the CMsgBox to work with your
// scenario.  Usually this will be the 3 functions listed above and whatever
// others you feel like calling (e.g., rspDoSound()).
//
// Order matters!:
// When you use AddButton, AddText, or AddEdit parameters (including colors,
// border thickness, font, DRAWCALL, BACKCALL, etc.) are copied from this CMsgBox{CDlg}.  If you
// change the settings in CMsgBox after calling one of these functions, 
// the created GUI item will differ by those settings.
//
// Enhancements/Uses:
// You could derive from this. /shrug
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// C Headers.
//////////////////////////////////////////////////////////////////////////////
#include <string.h>

//////////////////////////////////////////////////////////////////////////////
// RSPiX Headers.
//////////////////////////////////////////////////////////////////////////////
#include "Blue.h"

#ifdef PATHS_IN_INCLUDES
	#include "ORANGE/MsgBox/MsgBox.h"
	#include "ORANGE/DirtRect/DirtRect.h"
	#include "ORANGE/GUI/scrollbar.h"
#else
	#include "MsgBox.h"
	#include "dirtrect.h"
	#include "scrollbar.h"
#endif // PATHS_IN_INCLUDES

//////////////////////////////////////////////////////////////////////////////
// Module specific macros.
//////////////////////////////////////////////////////////////////////////////

// Sets val to def if val is -1.
#define DEF(val, def)	((val == -1) ? def : val)

//////////////////////////////////////////////////////////////////////////////
// Module specific typedefs.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Module specific (static) variables.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Construction/Destruction.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Default constructor.
//
//////////////////////////////////////////////////////////////////////////////
RMsgBox::RMsgBox()
	{
	// Assume no button up callback.
	m_mbcUser	= NULL;
	// Active when visible.
	m_sActive	= TRUE;

	// No current clickage.
	m_ulId		= 0;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Destructor.
//
//////////////////////////////////////////////////////////////////////////////
RMsgBox::~RMsgBox()
	{
	// Clean up.
	RemoveAll();
	}

////////////////////////////////////////////////////////////////////////
// Methods.
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
//
// Add a btn with provided text at sX, sY in RMsgBox dlg.
//
////////////////////////////////////////////////////////////////////////
RBtn* RMsgBox::AddButton(	// Returns allocated GUI item on success.
									// Do NOT delete this item; it will be deleted
									// by a RemoveAll() call.
	char* pszText,				// Text for btn item.
	short	sX,					// X position in RMsgBox dlg.
	short	sY,					// Y position in RMsgBox dlg.
	ULONG	ulId,				// ID to return if this item is chosen.
								// There will be no response to this item
								// if lId is 0.
	short	sAddW /*= 0*/,	// Additional width for guis that require more.
	short	sAddH /*= 0*/)	// Additional height for guis that require more.
	{
	short	sError	= 0;

	// Attempt to allocate new btn.
	RBtn*	pbtn	= new RBtn;
	if (pbtn != NULL)
		{
		if (AddItem(pbtn, pszText, sX, sY, ulId, sAddW, sAddH) == 0)
			{
			// Success.
			}
		else
			{
			TRACE("AddButton(): AddItem() failed for allocated RBtn.\n");
			sError	= 2;
			}

		// If any errors occurred after allocation . . .
		if (sError != 0)
			{
			delete pbtn;
			pbtn	= NULL;
			}
		}
	else
		{
		TRACE("AddButton(): Failed to allocate RBtn.\n");
		sError	= 1;
		}

	return pbtn;
	}

////////////////////////////////////////////////////////////////////////
//
// Add a txt item with provided text at sX, sY in RMsgBox dlg.
//
////////////////////////////////////////////////////////////////////////
RTxt* RMsgBox::AddText(	// Returns allocated GUI item on success.
								// Do NOT delete this item; it will be deleted
								// by a RemoveAll() call.
	char* pszText,			// Text for txt item.
	short	sX,				// X position in RMsgBox dlg.
	short	sY,				// Y position in RMsgBox dlg.
	ULONG	ulId,				// ID to return if this item is chosen.
								// There will be no response to this item
								// if lId is 0.
	short	sAddW /*= 0*/,	// Additional width for guis that require more.
	short	sAddH /*= 0*/)	// Additional height for guis that require more.
	{
	short	sError	= 0;

	// Attempt to allocate new btn.
	RTxt*	ptxt	= new RTxt;
	if (ptxt != NULL)
		{
		if (AddItem(ptxt, pszText, sX, sY, ulId, sAddW, sAddH) == 0)
			{
			// Success.
			}
		else
			{
			TRACE("AddText(): AddItem() failed for allocated RTxt.\n");
			sError	= 2;
			}

		// If any errors occurred after allocation . . .
		if (sError != 0)
			{
			delete ptxt;
			ptxt	= NULL;
			}
		}
	else
		{
		TRACE("AddText(): Failed to allocate RTxt.\n");
		sError	= 1;
		}

	return ptxt;
	}

////////////////////////////////////////////////////////////////////////
//
// Add an edit field with provided text at sX, sY in RMsgBox dlg.
//
////////////////////////////////////////////////////////////////////////
REdit* RMsgBox::AddEdit(	// Returns allocated GUI item on success.
									// Do NOT delete this item; it will be deleted
									// by a RemoveAll() call.
	char* pszText,				// Text for edit item.
	short	sX,					// X position in RMsgBox dlg.
	short	sY,					// Y position in RMsgBox dlg.
	ULONG	ulId,				// ID to return if this item is chosen.
								// There will be no response to this item
								// if lId is 0.
	short	sAddW /*= 0*/,	// Additional width for guis that require more.
	short	sAddH /*= 0*/)	// Additional height for guis that require more.
	{
	short	sError	= 0;

	// Attempt to allocate new btn.
	REdit*	pedit	= new REdit;
	if (pedit != NULL)
		{
		// Need to create string so we can determine size of caret.
		char	szCaret[]	= { pedit->m_cCaretChar, '\0' };
		// Store text width.
		short sTextWidth	= pedit->m_pprint->GetWidth(szCaret);

		if (AddItem(pedit, pszText, sX, sY, ulId, sTextWidth + sAddW, sAddH) == 0)
			{
			// Limit text to the number of characters originally used.
//			pedit->m_sMaxText	= strlen(pszText);

			// Success.
			}
		else
			{
			TRACE("AddEdit(): AddItem() failed for allocated REdit.\n");
			sError	= 2;
			}

		// If any errors occurred after allocation . . .
		if (sError != 0)
			{
			delete pedit;
			pedit	= NULL;
			}
		}
	else
		{
		TRACE("AddEdit(): Failed to allocate REdit.\n");
		sError	= 1;
		}

	return pedit;
	}

////////////////////////////////////////////////////////////////////////
//
// Add the RGuiItem of your choice.  pgui->m_ulUserData will be returned
// by DoModal() if this item is chosen.
//
////////////////////////////////////////////////////////////////////////
RGuiItem* RMsgBox::AddItem(	// Returns pgui on success.
	RGuiItem*	pgui)				// The RGuiItem to add (its m_sX/Y should 
										// already have been set).
	{
	// Store RMsgBox instance.
	pgui->m_ulUserInstance	= (ULONG)this;
	// Let RGuiItem know where to call.
	pgui->m_bcUser				= ItemBtnUpCall;

	// Simply sets this RGuiItem as a child of this RMsgBox{RDlg}.
	pgui->SetParent(this);

	return pgui;
	}

////////////////////////////////////////////////////////////////////////
//
// Draw dirty areas if not going direct to screen and
// remove rects from drl.
//
////////////////////////////////////////////////////////////////////////
inline void DrawDirty(	// Returns nothing.
	RImage* pimComp,		// Composite buffer.
	RImage* pimScr,		// Screen buffer.
	RDirtyRects* pdrl)		// List of dirty rects.
	{
	RDRect*	pdr;
	// If not going direct to screen . . .
	if (pimComp != pimScr)
		{
		rspShieldMouseCursor();

		pdr	= pdrl->GetHead();
		while (pdr != NULL)
			{
			// Update screen.
			rspBlit( pimComp, pimScr,
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

////////////////////////////////////////////////////////////////////////
//
// Operates RMsgBox for one iteration.  Does NOT Draw().
// Check m_pguiFocus to determine RGuiItem with focus.
//
////////////////////////////////////////////////////////////////////////
ULONG RMsgBox::DoModeless(		// Returns item clicked or 0, if none.
	RInputEvent* pie)				// In:  Most recent user input event.
										// Out: pie->sUsed = TRUE, if used.
	{
	// If there is a user callback . . .
	if (m_mbcUser != NULL)
		{
		// Allow the user to call stuff.
		ULONG	ulRes	= (*m_mbcUser)(this);
		// If the user returned a non-zero value . . .
		if (ulRes != 0)
			{
			// Override ours.
			m_ulId	= ulRes;
			}
		}
	else
		{
		// Provide minimum system callage.
		rspDoSystem();
		m_hot.Do(pie);
		}

	RGuiItem::DoFocus(pie);

	ULONG	ulResId	= m_ulId;
	// Reset.
	m_ulId			= 0;

	return ulResId;
	}

////////////////////////////////////////////////////////////////////////
//
// Activates this RMsgBox and waits for input until m_mbcUser returns
// non-zero or an item is clicked.  If sDeactivateHots is TRUE, all 
// RHots are deactivated before activating GUI items.  These RHots are
// reactivated before exitting DoModal().
//
////////////////////////////////////////////////////////////////////////
ULONG RMsgBox::DoModal(					// Returns chosen ID on success, 
												// 0 on failure.
	RInputEvent* pie,						// In:  Most recent user input event.
												// Out: pie->sUsed = TRUE, if used.
	RImage*	pimDst		/*= NULL*/)	// Where to draw dialog and rspBlit from.
												// If this is NULL, the system buffer is
												// used.
												// rspBlit is used to update this to the
												// screen image unless pimDst is the screen
												// image.
	{
	m_ulId			= 0;	// ID to return.
	short	sError	= 0;	// No errors yet.

	// Deactivate RHots, adding them to our list.
	// We now can just service our own RHots.

	RImage*	pimScr;
	// If composition buffer provided . . .
	if (pimDst != NULL)
		{
		rspNameBuffers(NULL, &pimScr, NULL);
		}
	else	// No composition buffer provided.
		{
		rspNameBuffers(&pimDst, &pimScr, NULL);
		}

	// Allocate eraser . . .
	RImage imEraser;
	if (imEraser.CreateImage(
		m_im.m_sWidth, 
		m_im.m_sHeight,
		RImage::BMP8, 
		0, 
		m_im.m_sDepth) == 0)
		{
		}
	else
		{
		TRACE("DoModal(): m_imEraser.CreateImage() failed.\n");
		sError	= 1;
		}

	// Activate this and all children.
	SetVisible(TRUE);

	RDRect		dr	= { m_sX, m_sY, m_im.m_sWidth, m_im.m_sHeight };
	RDirtyRects	drl;
	drl.m_sClipX	= pimDst->m_sWidth - 1;
	drl.m_sClipY	= pimDst->m_sHeight - 1;

	// Clipping rectangle for composite buffer.
	RRect			rcCompositeClip(0, 0, drl.m_sClipX, drl.m_sClipY);

	// Main loop:
	// Wait for an error or an ID.
	while (m_ulId == 0 && sError == 0)
		{
		dr.sX	= m_sX;
		dr.sY	= m_sY;

		// Get new erase data.
		rspBlit(	pimDst, &imEraser, 
					dr.sX, dr.sY,
					0, 0,
					dr.sW, dr.sH, 
					NULL,						// Destination clippage.
					&rcCompositeClip);	// Source clippage.

		// Draw new dlg data.
		Draw(pimDst, 0, 0, 0, 0, 0, 0, &rcCompositeClip);

		// Add to dirty rect list.  Most likely will combine.
		drl.Add(&dr);

		// Draw dirty areas if not going direct to screen and
		// remove rects from drl.
		DrawDirty(pimDst, pimScr, &drl);

		// Do GUI maintainence.
		ULONG	ulRes	= DoModeless(pie);
		if (ulRes != 0)
			{
			// Override ours.
			m_ulId	= ulRes;
			}

		// Erase old.
		rspBlit(	&imEraser, pimDst, 0, 0, 
					dr.sX, dr.sY,
					dr.sW, dr.sH);

		// Add to dirty rect list.
		drl.Add(&dr);
		}

	// Erase.
	DrawDirty(pimDst, pimScr, &drl);

	// Free up any memory no longer needed.
	imEraser.DestroyData();

	// Deactivate this and all children.
	SetVisible(FALSE);

	return m_ulId;
	}

////////////////////////////////////////////////////////////////////////
//
// Destroys all allocated items (i.e., items NOT added with
// AddItem(...) (i.e., items allocated by RMsgBox)).
//
////////////////////////////////////////////////////////////////////////
void RMsgBox::RemoveAll(void)	// Returns nothing.
	{
	RGuiItem*	pgui	= m_listGuis.GetHead();
	while (pgui != NULL)
		{
		m_listGuis.Remove();

		delete pgui;

		pgui	= m_listGuis.GetNext();
		}
	}

////////////////////////////////////////////////////////////////////////
// Querries.
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
// Callback points (Non-Static).
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
// Callback points (Static).
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
//
// Called by child items to let us know when they are clicked in.
// (static)
//
////////////////////////////////////////////////////////////////////////
void RMsgBox::ItemBtnUpCall(RGuiItem* pgui)
	{
	ASSERT(pgui->m_ulUserInstance != NULL);

	RMsgBox*	pmb	= (RMsgBox*)(pgui->m_ulUserInstance);

	// Store item ID.
	pmb->m_ulId	= pgui->m_ulUserData;

	// Set focus to.
	pmb->SetFocus(pgui);
	}

////////////////////////////////////////////////////////////////////////
// Internal.
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
//
// Add a GUI item already allocated by this RMsgBox.
//
////////////////////////////////////////////////////////////////////////
short RMsgBox::AddItem(	// Returns 0 on success.
	RGuiItem* pgui,		// Item to add.
	char* pszText,			// Text for item.
	short	sX,				// X position in RMsgBox dlg.
	short	sY,				// Y position in RMsgBox dlg.
	ULONG	ulId,				// ID to return if this item is chosen.
								// There will be no response to this item
								// if lId is 0.
	short	sAddW /*= 0*/,	// Additional width for guis that require more.
	short	sAddH /*= 0*/)	// Additional height for guis that require more.
	{
	short	sRes	= 0;	// Assume success.

	// Copy settings.
	CopyParms(pgui);

	// Store ID.
	pgui->m_ulUserData		= ulId;

	// Set text.
	pgui->SetText(pszText);

	// Store text width.
	short sTextWidth			= pgui->m_pprint->GetWidth(pszText);

	// Active when visible.
	pgui->m_sActive			= TRUE;
	// Set font height for item.
	pgui->m_sFontCellHeight	= m_sFontCellHeight;
	
	// Reserve space for borders, if there are any.
	short sTotalBorderThickness	= pgui->GetTopLeftBorderThickness()
											+ pgui->GetBottomRightBorderThickness();

	// Create to font/text appropriate size . . .
	if (pgui->Create(	sX, sY, 
							sTextWidth + sTotalBorderThickness + sAddW,
							m_sFontCellHeight + sTotalBorderThickness + sAddH,
							m_im.m_sDepth)
		== 0)
		{
		// Add item (i.e., set parent to us) . . .
		if (AddItem(pgui) != NULL)
			{
			// Add to our list of items to be deallocated later . . .
			if (m_listGuis.Add(pgui) == 0)
				{
				// Success.
				}
			else
				{
				TRACE("AddItem(): Failed to add item to list.\n");
				sRes	= -3;
				}
			}
		else
			{
			TRACE("AddItem(): AddItem() failed for allocated item.\n");
			sRes	= -2;
			}
		}
	else
		{
		TRACE("AddItem(): [RGuiItem::]Create failed.\n");
		sRes = -1;
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////
//
// Copies parameters from RMsgBox into *pgui.
// 
////////////////////////////////////////////////////////////////////////
void RMsgBox::CopyParms(RGuiItem* pgui)
	{
	pgui->m_drawcall						= m_drawcall;
	pgui->m_backcall						= m_backcall;

	pgui->m_u32BorderColor				= m_u32BorderColor;
	pgui->m_u32BorderShadowColor		= m_u32BorderShadowColor;
	pgui->m_u32BorderHighlightColor	= m_u32BorderHighlightColor;
	pgui->m_u32BorderEdgeColor			= m_u32BorderEdgeColor;

	pgui->m_u32TextColor					= m_u32TextColor;
	pgui->m_u32BackColor					= m_u32BackColor;
	pgui->m_u32TextShadowColor			= m_u32TextShadowColor;

	pgui->m_sBorderThickness			= m_sBorderThickness;

	// Use this to copy font stuff instead of stuff below:
	pgui->m_pprint							= m_pprint;
	}

//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
