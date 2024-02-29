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
// DLG.CPP
// 
// History:
//		08/07/96 JMI	Started.
//
//		08/12/96	JMI	Now does not have a title bar when there is no text in
//							m_szText.  Sets the hot to the entire client in that
//							case.
//
//		08/12/96	JMI	Now utilizes CGuiItem::DrawText to draw text and over-
//							rides CGuiItem's default justification to CENTERED.
//
//		09/24/96	JMI	Changed all BLU_MB?_* macros to RSP_MB?_* macros.
//
//		10/31/96	JMI	Changed:
//							Old label:		New label:
//							=========		=========
//							CDlg				RDlg
//							CImage			RImage
//							CGuiItem			RGuiItem
//							CENTERED			RGuiItem::Centered
//
//		11/27/96	JMI	Added initialization of m_type to identify this type
//							of GUI item and virtual base function void Do(void).
//
//		12/19/96	JMI	Uses new m_justification (as m_sJustification) and 
//							upgraded to new RFont/RPrint.
//
//		12/31/96	JMI	Do() now calls base implementation in RGuiItem.
//
//		01/01/96	JMI	Now overrides GetHot() to restrict hot area to the title
//							bar, when present.  Also, Compose() no longer sets hot
//							area (now done by base class).
//
//		01/04/96	JMI	Upgraded HotCall() to new CursorEvent().  This upgrade
//							is in response to RGuiItem now using relative RHots.
//							Now m_hot.m_sX/Y is parent item relative just like
//							m_sX/Y.  This should simplify a lot of stuff and even
//							fix some odd features like being able to click a GUI
//							item that exceeds the boundary of its parent.  This fix
//							will be essential for the not-yet-existent RListBox since
//							it will most likely scroll many children through a small
//							client area.
//							Now there are two regions associated with cursor events.
//							The first is the 'hot' area.  This is the area that m_hot
//							is set to include.  Child items can only receive cursor
//							events through this area.  The second is the 'event' area.
//							This is the area where the item really is actually con-
//							cerned with cursor events.  Example:  For a Dlg, the
//							entire window is the 'hot' area and the title bar is the
//							'event' area.
//
//		01/18/97	JMI	Converted Do() to take an RInputEvent* instead of a 
//							long*.
//
//		01/23/97	JMI	Changed Do() such that you cannot drag an item outside
//							of its parent.  I was reluctant to do this b/c it assumes
//							you are using the RSPiX Blue coordinate system, but then
//							I realized that since it is calling rspGetMouse(), it was
//							already using this coordinate system.  When the RHots
//							start supporting Move events, I'll change this to work on
//							the callback and to not call rspSetMouse().
//
//		03/19/97	JMI	Converted to using the RHot::m_iecUser (was using
//							RHot::m_epcUser) so HotCall and CursorEvent now take
//							RInputEvent ptrs.
//
//		03/28/97	JMI	RSP_MB0_DOUBLECLICK is now treated the same as
//							RSP_MB0_PRESSED.
//
//		04/10/97	JMI	Now uses m_sFontCellHeight instead of GetPos() to get
//							cell height.
//
//		07/01/97	JMI	Was passing a deference of the ptr parm to SET as 2nd
//							arg.
//
//////////////////////////////////////////////////////////////////////////////
//
// This a GUI item that is based on the basic RGuiItem.  It provides one
// additional function (Do) that allows it to be moved.  Simply not calling
// the Do function will keep it from being moved.
// This overrides HotCall() to get information about where a click in its CHot
// occurred.
// This overrides Compose() to add a title bar and set the dimensions of
// the CHot to the title bar.
// This overrides GetClient() to allow space for the title bar, when present.
// This overrides GetHot() to restrict hot area to title bar, when present.
//
// Enhancements/Uses:
// To change the background of a button, see RGuiItem.
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Headers.
//////////////////////////////////////////////////////////////////////////////
#include "Blue.h"

#ifdef PATHS_IN_INCLUDES
	#include "ORANGE/GUI/dlg.h"
#else
	#include "dlg.h"
#endif // PATHS_IN_INCLUDES

//////////////////////////////////////////////////////////////////////////////
// Module specific macros.
//////////////////////////////////////////////////////////////////////////////

// Sets val to def if val is -1.
#define DEF(val, def)	((val == -1) ? def : val)

// Sets a value pointed to if ptr is not NULL.
#define SET(pval, val)					((pval != NULL) ? *pval = val : val)

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
RDlg::RDlg()
	{
	// Override RGuiItem's default justification.
	m_justification	= RGuiItem::Centered;

	m_type				= Dlg;	// Indicates type of GUI item.
	}

//////////////////////////////////////////////////////////////////////////////
//
// Destructor.
//
//////////////////////////////////////////////////////////////////////////////
RDlg::~RDlg()
	{
	}

////////////////////////////////////////////////////////////////////////
// Methods.
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
//
// Does as Dlg would Do.
//
////////////////////////////////////////////////////////////////////////
void RDlg::Do(			// Returns nothing.
	RInputEvent* pie)	// In:  Most recent user input event.
							// Out: pie->sUsed = TRUE, if used.
	{
	// Call base.
	RGuiItem::Do(pie);

	// If we're active . . .
	if (m_hot.IsActive() != FALSE)
		{
		// If we're pressed . . .
		if (m_sPressed != FALSE)
			{
			short	sTopPosX, sTopPosY;
			short	sPosX, sPosY;
			short	sParentW, sParentH;

			// Get mouse position RSPiX relative.
			rspGetMouse(&sTopPosX, &sTopPosY, NULL);
			sPosX	= sTopPosX;
			sPosY	= sTopPosY;

			RGuiItem*	pguiParent	= GetParent();
			if (pguiParent != NULL)
				{
				pguiParent->TopPosToChild(&sPosX, &sPosY);
				sParentW	= pguiParent->m_im.m_sWidth;
				sParentH	= pguiParent->m_im.m_sHeight;
				}
			else
				{
				rspGetVideoMode(NULL, NULL, NULL, NULL, &sParentW, &sParentH);
				}

			// Stay within parent.
			short sClippedPosX	= MAX((short)0, MIN(sPosX, sParentW) );
			short sClippedPosY	= MAX((short)0, MIN(sPosY, sParentH) );

			// If clipped . . .
			if (sClippedPosX != sPosX || sClippedPosY != sPosY)
				{
				// Reposition cursor based on clipped position.
				rspSetMouse(
					sClippedPosX + sTopPosX - sPosX,
					sClippedPosY + sTopPosY - sPosY);
				}

			// Finally move to new position.
			Move(	sClippedPosX - m_sMoveOffsetX, 
					sClippedPosY - m_sMoveOffsetY);
			}
		}
	}

////////////////////////////////////////////////////////////////////////
//
// Cursor event notification.
// Events in event area.
// (virtual).
//
////////////////////////////////////////////////////////////////////////
void RDlg::CursorEvent(	// Returns nothing.
	RInputEvent* pie)		// In:  Most recent user input event.             
								// Out: pie->sUsed = TRUE, if used.
	{
	RGuiItem::CursorEvent(pie);

	switch (pie->sEvent)
		{
		case RSP_MB0_DOUBLECLICK:
		case RSP_MB0_PRESSED:
			// Store offsets for nice drag.
			m_sMoveOffsetX = pie->sPosX;
			m_sMoveOffsetY = pie->sPosY;

			// Note that we used it.
			pie->sUsed	= TRUE;

			break;
		}
	}

////////////////////////////////////////////////////////////////////////
//
// Compose item.
//
////////////////////////////////////////////////////////////////////////
void RDlg::Compose(			// Returns nothing.
	RImage* pim /*= NULL*/)	// Dest image, uses m_im if NULL.
	{
	if (pim == NULL)
		{
		pim	= &m_im;
		}

	// Call base (draws border and background).
	RGuiItem::Compose(pim);

	// Draw dlg stuff.
	short	sX, sY, sW, sH;
	
	// Get client relative to border (minus title) so we know where to
	// put title.
	RGuiItem::GetClient(&sX, &sY, &sW, &sH);

	// Draw text.
	if (m_szText[0] != '\0')
		{
		short	sTextHeight	= m_sFontCellHeight;

		// Draw title bar.
		rspRect( m_u32BorderColor, pim,
					sX, sY,
					sW,
					sTextHeight);

		DrawText(sX, sY, sW, sTextHeight, pim);
		}
	}

//////////////////////////////////////////////////////////////////////////////
//
// Set this item's event area.  This is the area where cursor events are
// interesting to the item.
// (virtual).
//
//////////////////////////////////////////////////////////////////////////////
void RDlg::SetEventArea(void)	// Returns nothing.
	{
	// Call base to set to defaults.
	RGuiItem::GetClient(
		&m_sEventAreaX,
		&m_sEventAreaY,
		&m_sEventAreaW,
		&m_sEventAreaH);
	
	// If there is title text . . .
	if (m_szText[0] != '\0')
		{
		// Use height of text.
		m_sEventAreaH	= m_sFontCellHeight;
		}
	}

////////////////////////////////////////////////////////////////////////
// Querries.
////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Get the "client" area (i.e., non-border/title area) relative to this
// item.  Calls base class version.
//
//////////////////////////////////////////////////////////////////////////////
void RDlg::GetClient(	// Returns nothing.
	short* psX,				// Out: X position unless NULL.
	short* psY,				// Out: Y position unless NULL.
	short* psW,				// Out: Width unless NULL.
	short* psH)				// Out: Height unless NULL.
	{
	// Call base.
	RGuiItem::GetClient(psX, psY, psW, psH);
	
	// If there is title text . . .
	if (m_szText[0] != '\0')
		{
		// Reduce for title bar.
		if (psY)
			*psY	= *psY + m_sFontCellHeight;
		if (psH)
			*psH	= *psH - m_sFontCellHeight;
		}
	}

//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
