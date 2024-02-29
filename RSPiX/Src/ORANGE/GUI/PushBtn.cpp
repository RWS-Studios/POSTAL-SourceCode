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
// PUSHBTN.CPP
// 
// History:
//		02/05/97 JMI	Started.
//
//		02/05/97	JMI	Forgot to make sure we were actually clicked in before
//							processing a release within cursor event area.
//
//		02/05/97	JMI	Forgot to remove a debug TRACE.
//
//		03/19/97	JMI	Converted to using the RHot::m_iecUser (was using
//							RHot::m_epcUser) so HotCall and CursorEvent now take
//							RInputEvent ptrs.
//
//		03/28/97	JMI	RSP_MB0_DOUBLECLICK is now treated the same as
//							RSP_MB0_PRESSED.
//
//		04/16/97	JMI	In previous update, forgot to add case RSP_MB0_DOUBLECLICK
//							in both places need for this implentation of CursorEvent().
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
	#include "ORANGE/GUI/PushBtn.h"
#else
	#include "PushBtn.h"
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
RPushBtn::RPushBtn()
	{
	// Override RGuiItem's default justification.
	m_justification	= RGuiItem::Centered;

	m_type				= PushBtn;	// Indicates type of GUI item.

	// Initialize RPushBtn members.
	m_state	= Off;		// The button's current state (On or Off (see enums)).
	}

//////////////////////////////////////////////////////////////////////////////
//
// Destructor.
//
//////////////////////////////////////////////////////////////////////////////
RPushBtn::~RPushBtn()
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
void RPushBtn::Compose(			// Returns nothing.
	RImage* pim /*= NULL*/)	// Dest image, uses m_im if NULL.
	{
	if (pim == NULL)
		{
		pim	= &m_im;
		}

	if (m_sPressed != FALSE || m_state == On)
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

	// Draw pushbtn stuff.
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
void RPushBtn::CursorEvent(	// Returns nothing.
	RInputEvent* pie)				// In:  Most recent user input event.             
										// Out: pie->sUsed = TRUE, if used.
	{
	switch (pie->sEvent)
		{
		case RSP_MB0_DOUBLECLICK:
		case RSP_MB0_RELEASED:
			// If we were clicked in . . .
			if (m_sPressed != FALSE)
				{
				// Do change of state right away so user callback gets the new
				// value.
				// If within event area . . .
				if (		pie->sPosX >= m_sEventAreaX && pie->sPosX < m_sEventAreaX + m_sEventAreaW
						&&	pie->sPosY >= m_sEventAreaY && pie->sPosY < m_sEventAreaY + m_sEventAreaH)
					{
					// Change state.
					m_state	= (m_state == Off) ? On : Off;
					}
				}

			break;
		}

	// Call base.
	RGuiItem::CursorEvent(pie);

	switch (pie->sEvent)
		{
		case RSP_MB0_DOUBLECLICK:
		case RSP_MB0_PRESSED:
			// Always recompose on press, since there's so many possibilities
			// with this button.
			Compose();

			// Note that we used it.
			pie->sUsed	= TRUE;

			break;

		case RSP_MB0_RELEASED:
			// Always recompose on release, since there's so many possibilities
			// with this button.
			Compose();

			// Note that we used it.
			pie->sUsed	= TRUE;

			break;
		}
	}

//////////////////////////////////////////////////////////////////////////////
//
// Draw border.
// (virtual	(Overridden here)).
//
//////////////////////////////////////////////////////////////////////////////
void RPushBtn::DrawBorder(			// Returns nothing.
	RImage* pim	/*= NULL*/,			// Dest image, uses m_im if NULL.
	short sInvert	/*= FALSE*/)	// Inverts border if TRUE.
	{
	short	sVertShadowPos;
	short	sHorzShadowPos;
	short	sVertHighlightPos;
	short sHorzHighlightPos;
	short sVertEdgePos;
	short sHorzEdgePos;
	short	sShadowThickness		= m_sBorderThickness;
	short	sEdgeThickness			= m_sBorderThickness;
	short	sHighlightThickness	= m_sBorderThickness;

	if (pim == NULL)
		{
		pim	= &m_im;
		}

	short sW	= pim->m_sWidth;
	short	sH	= pim->m_sHeight;

	m_sInvertedBorder	= sInvert;

	if (sInvert == FALSE)
		{
		sVertShadowPos			= sW - m_sBorderThickness;
		sHorzShadowPos			= sH - m_sBorderThickness;
		sVertHighlightPos		= 0;
		sHorzHighlightPos		= 0;
		sVertEdgePos			= sW - m_sBorderThickness * 2;
		sHorzEdgePos			= sH - m_sBorderThickness * 2;
		}
	else
		{
		// Left top becomes thicker when pressed.
		sShadowThickness		= (m_sPressed == FALSE) ? m_sBorderThickness 
																	: (m_sBorderThickness * 2);

		sEdgeThickness			= sShadowThickness;
		sHighlightThickness	= m_sBorderThickness;//sShadowThickness;

		sVertShadowPos			= 0;
		sHorzShadowPos			= 0;
		sVertHighlightPos		= sW - sHighlightThickness;
		sHorzHighlightPos		= sH - sHighlightThickness;
		sVertEdgePos			= sShadowThickness;
		sHorzEdgePos			= sShadowThickness;
		}

	// One pixel for each edge of border gets overwritten.
	rspRect(m_u32BorderHighlightColor, pim, 0, sHorzHighlightPos, sW, sHighlightThickness);
	rspRect(m_u32BorderHighlightColor, pim, sVertHighlightPos, 0, sHighlightThickness, sH);

	rspRect(m_u32BorderShadowColor, pim, 0, sHorzShadowPos, sW, sShadowThickness);
	rspRect(m_u32BorderShadowColor, pim, sVertShadowPos, 0, sShadowThickness, sH);

	rspRect(m_u32BorderEdgeColor, pim, sShadowThickness, sHorzEdgePos, sW - sShadowThickness - sEdgeThickness, sEdgeThickness);
	rspRect(m_u32BorderEdgeColor, pim, sVertEdgePos, sShadowThickness, sEdgeThickness, sH - sShadowThickness - sEdgeThickness);
	}

//////////////////////////////////////////////////////////////////////////////
// Querries.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Gets the thickness of the top/left border (including border edge effect).
// (virtual	(Overridden here)).
//
//////////////////////////////////////////////////////////////////////////////
short RPushBtn::GetTopLeftBorderThickness(void)	// Returns border thickness 
																// including edge effect.
	{
	if (m_sBorderThickness == 0)
		return 0;
	else
		return (m_sBorderThickness
		+ ((m_sInvertedBorder == FALSE) ? 0 : m_sBorderThickness) )
		* ((m_sPressed == FALSE) ? 1 : 2);
	}

//////////////////////////////////////////////////////////////////////////////
//
// Gets the thickness of the bottom/right border (including border edge effect).
// (virtual	(Overridden here)).
//
//////////////////////////////////////////////////////////////////////////////
short RPushBtn::GetBottomRightBorderThickness(void)	// Returns border thickness 
																		// including edge effect.
	{
	if (m_sBorderThickness == 0)
		return 0;
	else
		return (m_sBorderThickness 
		+ ((m_sInvertedBorder == FALSE) ? m_sBorderThickness : 0) )
;//		* ((m_sPressed == FALSE) ? 1 : 2);
	}

//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
