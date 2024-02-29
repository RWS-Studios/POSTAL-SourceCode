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
// TXT.CPP
// 
// History:
//		08/07/96 JMI	Started.
//
//		08/12/96	JMI	Now utilizes CGuiItem::DrawText to draw text.
//
//		09/24/96	JMI	Changed all BLU_MB?_* macros to RSP_MB?_* macros.
//
//		10/31/96	JMI	Changed:
//							Old label:		New label:
//							=========		=========
//							CTxt				RTxt
//							CImage			RImage
//							CGuiItem			RGuiItem
//							CENTERED			RGuiItem::Centered
//
//		11/27/96	JMI	Added initialization of m_type to identify this type
//							of GUI item.
//
//		01/01/96	JMI	Compose() no longer sets hot area (now done by base 
//							class).
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
//////////////////////////////////////////////////////////////////////////////
//
// This a GUI item that is based on the basic RGuiItem. 
// This overrides HotCall() to get information about where a click in its RHot
// occurred.
// This overrides Compose() to add text.
//
// Enhancements/Uses:
// To change the look of text when pressed, you may want to override the
// Compose() or DrawBorder() in a derived class.
// To change the background of the text item, see RGuiItem.
// To get a callback on a click/release pair in the text item, set m_tcUser.
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Headers.
//////////////////////////////////////////////////////////////////////////////
#include "Blue.h"

#ifdef PATHS_IN_INCLUDES
	#include "ORANGE/GUI/txt.h"
#else
	#include "txt.h"
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
RTxt::RTxt()
	{
	m_type					= Txt;	// Indicates type of GUI item.
	}

//////////////////////////////////////////////////////////////////////////////
//
// Destructor.
//
//////////////////////////////////////////////////////////////////////////////
RTxt::~RTxt()
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
void RTxt::Compose(			// Returns nothing.
	RImage* pim /*= NULL*/)	// Dest image, uses m_im if NULL.
	{
	if (pim == NULL)
		{
		pim	= &m_im;
		}

	// Call base (draws border and background).
	RGuiItem::Compose(pim);

	// Draw txt stuff.
	short	sX, sY, sW, sH;
	// Get client relative to border so we know where to put the text.
	GetClient(&sX, &sY, &sW, &sH);

	// Draw text.
	DrawText(sX, sY, sW, sH, pim);
	}

////////////////////////////////////////////////////////////////////////
// Querries.
////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
