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
// BTN.CPP
// 
// History:
//		08/07/96 JMI	Started.
//
//		08/12/96	JMI	Now utilizes CGuiItem::DrawText to draw text and over-
//							rides CGuiItem's default justification to CENTERED.
//
//		09/24/96	JMI	Now draws border only once during a compose.
//
//		09/24/96	JMI	Changed all BLU_MB?_* macros to RSP_MB?_* macros.
//
//		10/31/96	JMI	Changed:
//							Old label:		New label:
//							=========		=========
//							CBtn				RBtn
//							CImage			RImage
//							CGuiItem			RGuiItem
//							CENTERED			Centered
//
//		11/27/96	JMI	Added initialization of m_type to identify this type
//							of GUI item.
//
//		12/19/96	JMI	Uses new m_justification (as m_sJustification) and now
//							uses new DrawText() call (takes 4 parms instead of 3).
//
//		01/01/96	JMI	HotCall() now calls the base class to do what it used
//							to with the exception of the one thing it still does --
//							Compose().  Also, Compose() no longer sets hot area
//							(now done by base class).
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
//		03/19/97	JMI	Converted to using the RHot::m_iecUser (was using
//							RHot::m_epcUser) so HotCall and CursorEvent now take
//							RInputEvent ptrs.
//
//		03/28/97	JMI	RSP_MB0_DOUBLECLICK is now treated the same as
//							RSP_MB0_PRESSED.
//
//////////////////////////////////////////////////////////////////////////////
//
// This a GUI item that is based on the basic RGuiItem. 
// This overrides HotCall() to get information about where a click in its CHot
// occurred.
// This overrides Compose() to add text.
//
// Enhancements/Uses:
// To change the look of a button when pressed, you may want to override the
// Compose() or DrawBorder() in a derived class.
// To change the background of a button, see RGuiItem.
// To get a callback on a click/release pair in the button, set m_bcUser.
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Headers.
//////////////////////////////////////////////////////////////////////////////
#include "Blue.h"

#ifdef PATHS_IN_INCLUDES
	#include "ORANGE/GUI/btn.h"
#else
	#include "btn.h"
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
RBtn::RBtn()
	{
	// Override RGuiItem's default justification.
	m_justification	= RGuiItem::Centered;

	m_type				= Btn;	// Indicates type of GUI item.
	}

//////////////////////////////////////////////////////////////////////////////
//
// Destructor.
//
//////////////////////////////////////////////////////////////////////////////
RBtn::~RBtn()
	{
	}

////////////////////////////////////////////////////////////////////////
// Methods.
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
//
// Compose item.
//
////////////////////////////////////////////////////////////////////////
void RBtn::Compose(			// Returns nothing.
	RImage* pim /*= NULL*/)	// Dest image, uses m_im if NULL.
	{
	if (pim == NULL)
		{
		pim	= &m_im;
		}

	if (m_sPressed != FALSE)
		{
		// Invert border.
		m_sInvertedBorder	= TRUE;
		}
	else
		{
		m_sInvertedBorder	= FALSE;
		}

	// Call base (draws border and background).
	RGuiItem::Compose(pim);

	// Draw btn stuff.
	short	sX, sY, sW, sH;
	// Get client relative to border so we know where to
	// put the text.
	GetClient(&sX, &sY, &sW, &sH);

	// Draw text.
	DrawText(sX, sY, sW, sH, pim);
	}

////////////////////////////////////////////////////////////////////////
//
// Cursor event notification.
// Events in event area.
// (virtual).
//
////////////////////////////////////////////////////////////////////////
void RBtn::CursorEvent(	// Returns nothing.
	RInputEvent* pie)		// In:  Most recent user input event.             
								// Out: pie->sUsed = TRUE, if used.
	{
	// Call base.
	RGuiItem::CursorEvent(pie);

	switch (pie->sEvent)
		{
		case RSP_MB0_DOUBLECLICK:
		case RSP_MB0_PRESSED:
			// If not inverted already . . .
			if (m_sInvertedBorder == FALSE)
				{
				// Recompose.
				Compose();
				}

			// Note that we used it.
			pie->sUsed	= TRUE;

			break;

		case RSP_MB0_RELEASED:
			// If we were inverted . . .
			if (m_sInvertedBorder != FALSE)
				{
				// Recompose.
				Compose();
				}

			// Note that we used it.
			pie->sUsed	= TRUE;

			break;
		}
	}

////////////////////////////////////////////////////////////////////////
// Querries.
////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
