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
// SCROLLBAR.CPP
// 
// History:
//		09/25/96 JMI	Started.
//
//		10/31/96	JMI	Changed:
//							Old label:		New label:
//							=========		=========
//							CScrollBar		RScrollBar
//							CImage			RImage
//							CGuiItem			RGuiItem
//							ORIENTATION		Orientation
//							HORIZONTAL		Horizontal
//							VERTICAL			Vertical
//							CBtn				RBtn
//
//		11/01/96	JMI	Changed:
//							Old label:		New label:
//							=========		=========
//							Rect				RRect
//
//							Also, changed all members referenced in RImage to
//							m_ and all position/dimension members referenced in
//							RImage to type short usage.
//
//		11/27/96	JMI	Added initialization of m_type to identify this type
//							of GUI item.
//
//		12/16/96	JMI	Now checks for and reports when width or height is too
//							small in Compose().
//
//		12/16/96	JMI	Now ASSERTs if width or height is too small in DrawArrow.
//
//		12/31/96	JMI	Do() now calls base implementation in RGuiItem.
//
//		01/02/97	JMI	Overrode GetHot() to use tray area instead of client.
//							GetTray() now checks for NULL pointers before assigning
//							values.  Changed m_guiThumb to m_btnThumb.
//
//		01/02/97	JMI	What was the macro ARROW_BORDER_THICKNESS is now the
//							member m_sArrowBorderDistance which is initialized to
//							DEF_ARROW_BORDER_THICKNESS.
//
//		01/04/97	JMI	Upgraded HotCall() to new CursorEvent().  This upgrade
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
//		01/14/97	JMI	Now initializes m_upcUser to NULL.  Also, SetRange()
//							now allows a range of 0 size.  Still cannot have a range
//							of negative size though.  This would be useful.
//
//		01/15/97	JMI	Added overrides of base class's Save/LoadChildren() to
//							implement special cases for m_btnThumb, Up, & Down.
//
//		01/20/97	JMI	No longer ASSERTs on odd sizes.  Just does the best it
//							can with what you give it.  You choose a screwy size, you
//							get it.
//
//		01/21/97	JMI	Added ReadMembers() and WriteMembers() overloads to read
//							and write members of this class.  Note that they call the
//							base class version to read/write base class members.
//							Support exists for versions 0 and 1.  Version 0 did not
//							contain other than RGuiItem members.
//
//		01/27/97	JMI	Now, if in a call to SetRange() the min is greater than 
//							the max, the range is set to 0 instead of ASSERTing.
//
//		01/30/97	JMI	Fixed rounding error in POS2PIXEL().
//
//		02/05/97	JMI	Changed position of default: case in ReadMembers().
//
//		02/05/97	JMI	Now has repeatability on up/left button, down/right
//							button, and tray presses.  To do this in a consistent
//							way from the Do(), the up and down buttons had to be
//							changed to being hooked at the HotCall() level (like
//							the thumb has always been done).
//							This removed the need for static versions of the Up/
//							DownBtnCall()s.
//							Also, moved instantiable Up/DownBtnCall definitions to
//							the CPP.
//
//		02/25/97	JMI	SaveChildren() now goes through the children in reverse
//							order so they, on load, get added back to their parent in
//							the order they were originally added to this parent.
//
//		03/19/97	JMI	HotCall() now adapts sPosX,Y from hotbox coords instead
//							of hotbox's parent's coords.
//
//		03/19/97	JMI	Converted to using the RHot::m_iecUser (was using
//							RHot::m_epcUser) so HotCall and CursorEvent now take
//							RInputEvent ptrs.
//
//		03/20/97	JMI	Still had a reference to m_epcUser in Do().  Fixed.
//
//		03/28/97	JMI	RSP_MB0_DOUBLECLICK is now treated the same as
//							RSP_MB0_PRESSED.
//
//		04/01/97	JMI	Changed short m_sCanBeFocused (TRUE, FALSE) to 
//							m_targetFocus (Self, Parent, Sibling).
//
//		04/03/97	JMI	Added components for optional timed, smooth scrolling.
//
//		04/04/97	JMI	Added reading/writing of version 3 members.
//
//		05/14/97	JMI	Removed possible divide by 0's in SetRange().
//
//		06/27/97	JMI	Moved GetTray() definition to here from scrollbar.h.
//
//////////////////////////////////////////////////////////////////////////////
//
// This a GUI item that is based on the basic RGuiItem. 
// This overrides HotCall() to get information about where a click in its CHot
// occurred.
// This overrides Compose() to add buttons and a thumb control.
//
// Enhancements/Uses:
// To change the look of a scrollbar, you may want to override the Compose() 
// or DrawBorder() in a derived class (being sure to call 
// CSrollBar::DrawBorder() and RScrollBar::Compose() as appropriate).
//
// To change the background of a button, see RGuiItem.
// To get a callback on a click/release pair in the button, set m_bcUser.
// CAUTION:
// You may set the callbacks for these m_scrollbarUp, m_scrollbarDown, or m_btnThumb, or 
// the base class to your own functions.  But if you change gui.m_bcUser or 
// gui.m_ulUserInstace, you are responsible for calling SetPos() or the
// callbacks (e.g., UpScrollBarPressed(void), DownScrollBarPressed(void), or 
// TrayPressed(void)) as appropriate.  Also, if you change gui.m_ulUserInstance
// on any of the gui children, be sure to set or clear gui.m_bcUser and 
// gui.m_backcall from their original values.
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Headers.
//////////////////////////////////////////////////////////////////////////////
#include "Blue.h"

#ifdef PATHS_IN_INCLUDES
	#include "ORANGE/GUI/scrollbar.h"
#else
	#include "ScrollBar.h"
#endif // PATHS_IN_INCLUDES

//////////////////////////////////////////////////////////////////////////////
// Module specific macros.
//////////////////////////////////////////////////////////////////////////////

// Sets val to def if val is -1.
#define DEF(val, def)		((val == -1) ? def : val)

#define DEF_MIN_THUMB_LENGTH		10

#define DEF_ARROW_BORDER_THICKNESS	2

#define REPEAT_INITIAL_TIMEOUT		500
#define REPEAT_SEQUENTIAL_TIMEOUT	100

#define DEF_SCROLL_RATE					100	// Positions per second.

//////////////////////////////////////////////////////////////////////////////
// Module specific typedefs.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Module specific (static) variables.
//////////////////////////////////////////////////////////////////////////////

static long	ms_lNextEventTime	= 0;	// Time of next repeat event.

//////////////////////////////////////////////////////////////////////////////
// Construction/Destruction.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Default constructor.
//
//////////////////////////////////////////////////////////////////////////////
RScrollBar::RScrollBar()
	{
	m_lButtonIncDec	= 1;
	m_lTrayIncDec		= 10;
	m_oOrientation		= Vertical;
	m_lMinPos			= 0;
	m_lMaxPos			= 100;
	m_lCurPos			= 0;
	m_lMinThumbLength	= DEF_MIN_THUMB_LENGTH;

	m_sArrowBorderDistance	= DEF_ARROW_BORDER_THICKNESS;

	m_sInvertedBorder	= TRUE;

	m_btnThumb.SetParent(this);
	m_btnThumb.m_hot.m_iecUser	= ThumbHotCall;
	m_btnThumb.m_hot.m_ulUser	= (ULONG)this;
	m_btnThumb.m_targetFocus	= Parent;	// Passes focus to parent (this).

	m_btnUp.SetParent(this);
	m_btnUp.m_hot.m_iecUser		= UpHotCall;
	m_btnUp.m_hot.m_ulUser		= (ULONG)this;
	m_btnUp.m_ulUserInstance	= (ULONG)this;
	m_btnUp.m_backcall			= DrawUpArrow;
	m_btnUp.m_targetFocus		= Parent;	// Passes focus to parent (this).

	m_btnDown.SetParent(this);
	m_btnDown.m_hot.m_iecUser	= DownHotCall;
	m_btnDown.m_hot.m_ulUser	= (ULONG)this;
	m_btnDown.m_ulUserInstance	= (ULONG)this;
	m_btnDown.m_backcall			= DrawDownArrow;
	m_btnDown.m_targetFocus		= Parent;	// Passes focus to parent (this).

	m_type							= ScrollBar;	// Indicates type of GUI item.

	m_upcUser						= NULL;

	m_scrollage						= Instant;
	m_lScrollToPos					= m_lCurPos;
	m_lPosPerSecond				= DEF_SCROLL_RATE;
	m_sInSmoothScroll				= FALSE;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Destructor.
//
//////////////////////////////////////////////////////////////////////////////
RScrollBar::~RScrollBar()
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
void RScrollBar::Compose(			// Returns nothing.
	RImage* pim /*= NULL*/)	// Dest image, uses m_im if NULL.
	{
	if (pim == NULL)
		{
		pim	= &m_im;
		}

	// Call base (draws border and background).
	RGuiItem::Compose(pim);

	// Draw scrollbar stuff.

	// Get client relative to border so we know where to
	// set up the buttons.
	short	sX, sY, sW, sH;
	GetClient(&sX, &sY, &sW, &sH);
	
	// Set width and height for buttons.
	short sBtnWidth;
	short sBtnHeight;
	if (m_oOrientation == Vertical)
		{
		sBtnWidth	= sW;
		sBtnHeight	= sW;
		}
	else	// Horizontal
		{
		sBtnWidth	= sH;
		sBtnHeight	= sH;
		}

	CopyBorderInfoTo(&m_btnUp);

	// Create up/left button . . .
	if (m_btnUp.Create(sX, sY, sBtnWidth, sBtnHeight, m_im.m_sDepth) == 0)
		{
		CopyBorderInfoTo(&m_btnDown);

		// Create down/right button . . .
		if (m_btnDown.Create(sX + sW - sBtnWidth, sY + sH - sBtnHeight, 
			sBtnWidth, sBtnHeight, m_im.m_sDepth) == 0)
			{
			}
		else
			{
			TRACE("Compose(): RBtn::Create() failed for down/right button.\n");
			}
		}
	else
		{
		TRACE("Compose(): RBtn::Create() failed for up/left button.\n");
		}

	CopyBorderInfoTo(&m_btnThumb);

	// Create thumb . . .
	SetRange(m_lMinPos, m_lMaxPos);
	}

////////////////////////////////////////////////////////////////////////
//
// Cursor event notification.
// Events in event area.
// (virtual).
//
/////////////////////////////////////////////////////////////
///////////
void RScrollBar::CursorEvent(	// Returns nothing.
	RInputEvent* pie)				// In:  Most recent user input event.             
										// Out: pie->sUsed = TRUE, if used.
	{
	switch (pie->sEvent)
		{
		case RSP_MB0_DOUBLECLICK:
		case RSP_MB0_PRESSED:
			{
			// Get focus now even though this is done in the base class b/c,
			// If using smooth scrollage, we must have the focus.
			SetFocus();

			// Store time of next event.
			ms_lNextEventTime	= rspGetMilliseconds();
			if (m_sPressed == FALSE) 
				{
				ms_lNextEventTime += REPEAT_INITIAL_TIMEOUT;
				}
			else
				{
				ms_lNextEventTime += REPEAT_SEQUENTIAL_TIMEOUT;
				}

			// If outside the thumb . . .
			// NOTE: If the m_lTrayIncDec is greater than the thickness of the
			// thumb, this will still hop around a bit.
			if (	pie->sPosX < m_btnThumb.m_sX 
				||	pie->sPosX >= m_btnThumb.m_sX + m_btnThumb.m_im.m_sWidth
				||	pie->sPosY < m_btnThumb.m_sY
				|| pie->sPosY >= m_btnThumb.m_sY + m_btnThumb.m_im.m_sHeight)
				{
				short	sTrayPosX, sTrayPosY, sTrayRelPosX, sTrayRelPosY;
				GetTray(&sTrayPosX, &sTrayPosY, NULL, NULL);
				sTrayRelPosX	= pie->sPosX - sTrayPosX;
				sTrayRelPosY	= pie->sPosY - sTrayPosY;

				// If less than existing pos . . .
				if (PIXELS2POS(sTrayRelPosX, sTrayRelPosY)	<	m_lCurPos)
					{
					// Set the thumb position forward.
					UserSetPos(m_lCurPos - m_lTrayIncDec);
					}
				else
					{
					// Set the thumb position backward.
					UserSetPos(m_lCurPos + m_lTrayIncDec);
					}
				}

			// Note that we used it.
			pie->sUsed	= TRUE;

			break;
			}

		case RSP_MB0_RELEASED:
			{
			// Note that we used it.
			pie->sUsed	= TRUE;

			break;
			}
		}

	RGuiItem::CursorEvent(pie);
	}

////////////////////////////////////////////////////////////////////////
//
// Called when up/left button pressed.
//
////////////////////////////////////////////////////////////////////////
void RScrollBar::UpBtnPressed(void)
	{
	UserSetPos(m_lCurPos - m_lButtonIncDec); 
	}

////////////////////////////////////////////////////////////////////////
//
// Called when down/right button pressed.
//
////////////////////////////////////////////////////////////////////////
void RScrollBar::DownBtnPressed(void)
	{
	UserSetPos(m_lCurPos + m_lButtonIncDec); 
	}

////////////////////////////////////////////////////////////////////////
//
// Sets the range of the scroll bar and sizes & moves the thumb 
// appropriately.
//
////////////////////////////////////////////////////////////////////////
void RScrollBar::SetRange(
	long lMin, 
	long lMax)
	{
	ASSERT(m_lMinThumbLength > 0);

	// Right now we don't support backwards max/min . . .
	if (lMin > lMax)
		{
		// Range is 0.
		lMin	= lMax;
		}

	m_lMinPos	= lMin;
	m_lMaxPos	= lMax;

	// Compute the range of positions.  That is, the number of positions.
	long	lRange	= lMax - lMin;

	// Compute the scroll bar size that gives us that many positions.
	short	sX;
	short	sY;
	short	sW;
	short	sH;
	GetTray(&sX, &sY, &sW, &sH);

	short		sThumbW;				// Width for thumb.
	short		sThumbH;				// Height for thumb.
	short*	psTrayLength;		// Pointer to tray length.
	short*	psTrayThickness;	// Pointer to tray thickness.
	short*	psThumbLength;		// Pointer to thumb length.
	short*	psThumbThickness;	// Pointer to thumb thickness.

	if (m_oOrientation == Vertical)
		{
		psTrayLength		= &sH;
		psTrayThickness	= &sW;
		psThumbLength		= &sThumbH;
		psThumbThickness	= &sThumbW;
		}
	else
		{
		psTrayLength		= &sW;
		psTrayThickness	= &sH;
		psThumbLength		= &sThumbW;
		psThumbThickness	= &sThumbH;
		}

	// Determine length for thumb.
	long lLength	= (long)(*psTrayLength) - lRange;

	// If the size is less than the minimum . . .
	if (lLength < m_lMinThumbLength)
		{
		lLength	= MIN(m_lMinThumbLength, (long)(*psTrayLength));
		}

	// Compute position to pixel ratio.
	float	fLenDif	= (float)(*psTrayLength - lLength);
	// If neither value is 0 . . .
	if (fLenDif != 0.0F && lRange != 0)
		{
		if (lRange > lLength)
			{
			m_fPos2PixelRatio	= fLenDif / (float)lRange;
			}
		else
			{
			m_fPos2PixelRatio	= (float)lRange / fLenDif;
			}
		}
	else
		{
		m_fPos2PixelRatio	= 0.00001F;
		}

	*psThumbLength		= (short)lLength;
	*psThumbThickness	= *psTrayThickness;

	m_btnThumb.Destroy();

	// Create thumb . . .
	if (m_btnThumb.Create(0, 0, MAX(sThumbW, (short)1), MAX(sThumbH, (short)1), m_im.m_sDepth) == 0)
		{
		// Set up hot.
		m_btnThumb.SetHotArea();
		// Set position.
		SetPos(m_lCurPos);
		}
	else
		{
		TRACE("SetRange(): RGuiItem::Create() failed for thumb.\n");
		}
	}

////////////////////////////////////////////////////////////////////////
//
// Hot call for thumb positioner.
//
////////////////////////////////////////////////////////////////////////
void RScrollBar::ThumbHotCall(	// Returns nothing.
	RInputEvent* pie)					// In:  Most recent user input event.             
											// Out: pie->sUsed = TRUE, if used.
	{
	short	sUsed	= pie->sUsed;

	// Call base.
	m_btnThumb.HotCall(pie);

	// If this is an unused mouse event . . .
	if (pie->type == RInputEvent::Mouse && sUsed == FALSE)
		{
		short	sHotPosX	= m_btnThumb.m_hot.m_sX;
		short	sHotPosY	= m_btnThumb.m_hot.m_sY;
		// Make relative to Thumb GUI item (rather than its hotbox).
		pie->sPosX	-= sHotPosX;
		pie->sPosY	-= sHotPosY;
		
		switch (pie->sEvent)
			{
			case RSP_MB0_PRESSED:
				// Start dragging thumb on Do().
				// Note position clicked so we know what part of bar was 
				// pressed.
				m_sClickOffsetX	= pie->sPosX;
				m_sClickOffsetY	= pie->sPosY;

				// Note that we used the event.
				pie->sUsed			= TRUE;
				break;
			case RSP_MB0_RELEASED:
				// Note that we used the event.
				pie->sUsed			= TRUE;
				break;
			}

		// Put position back.
		pie->sPosX	+= sHotPosX;
		pie->sPosY	+= sHotPosY;
		}
	}

////////////////////////////////////////////////////////////////////////
//
// Hot call for up button.
//
////////////////////////////////////////////////////////////////////////
void RScrollBar::UpHotCall(	// Returns nothing.
	RInputEvent* pie)				// In:  Most recent user input event.             
										// Out: pie->sUsed = TRUE, if used.
	{
	short	sUsed	= pie->sUsed;

	// Call base.
	m_btnUp.HotCall(pie);

	// If this is an unused mouse event . . .
	if (pie->type == RInputEvent::Mouse && sUsed == FALSE)
		{
		short	sHotPosX	= m_btnUp.m_hot.m_sX;
		short	sHotPosY	= m_btnUp.m_hot.m_sY;
		// Make relative to Up GUI item (rather than its hotbox).
		pie->sPosX	-= sHotPosX;
		pie->sPosY	-= sHotPosY;
		
		switch (pie->sEvent)
			{
			case RSP_MB0_PRESSED:
				// Store time of next event.
				ms_lNextEventTime	= rspGetMilliseconds();
				if (m_btnUp.m_sPressed == FALSE) 
					{
					ms_lNextEventTime += REPEAT_INITIAL_TIMEOUT;
					}
				else
					{
					ms_lNextEventTime += REPEAT_SEQUENTIAL_TIMEOUT;
					}

				UpBtnPressed();

				// Note that we used the event.
				pie->sUsed			= TRUE;
				break;
			case RSP_MB0_RELEASED:

				// Note that we used the event.
				pie->sUsed			= TRUE;
				break;
			}

		// Put position back.
		pie->sPosX	+= sHotPosX;
		pie->sPosY	+= sHotPosY;
		}
	}

////////////////////////////////////////////////////////////////////////
//
// Hot call for down button.
//
////////////////////////////////////////////////////////////////////////
void RScrollBar::DownHotCall(	// Returns nothing.
	RInputEvent* pie)				// In:  Most recent user input event.             
										// Out: pie->sUsed = TRUE, if used.
	{
	short	sUsed	= pie->sUsed;

	// Call base.
	m_btnDown.HotCall(pie);

	// If this is an unused mouse event . . .
	if (pie->type == RInputEvent::Mouse && sUsed == FALSE)
		{
		short	sHotPosX	= m_btnDown.m_hot.m_sX;
		short	sHotPosY	= m_btnDown.m_hot.m_sY;
		// Make relative to Down GUI item (rather than its hotbox).
		pie->sPosX	-= sHotPosX;
		pie->sPosY	-= sHotPosY;
		
		switch (pie->sEvent)
			{
			case RSP_MB0_PRESSED:
				// Store time of next event.
				ms_lNextEventTime	= rspGetMilliseconds();
				if (m_btnDown.m_sPressed == FALSE) 
					{
					ms_lNextEventTime += REPEAT_INITIAL_TIMEOUT;
					}
				else
					{
					ms_lNextEventTime += REPEAT_SEQUENTIAL_TIMEOUT;
					}

				DownBtnPressed();
				break;
			case RSP_MB0_RELEASED:
				break;
			}

		// Put position back.
		pie->sPosX	+= sHotPosX;
		pie->sPosY	+= sHotPosY;
		}
	}

////////////////////////////////////////////////////////////////////////
//
// Does tasks that require constant update for scroll bar.
// If called iteratively, the thumb can be dragged on the bar.
// (virtual (overridden here)).
//
////////////////////////////////////////////////////////////////////////
void RScrollBar::Do(	// Returns nothing.
	RInputEvent* pie)	// In:  Most recent user input event.
							// Out: pie->sUsed = TRUE, if used.
	{
	// Call base.
	RGuiItem::Do(pie);

#ifdef MOBILE
	//if (pie->sUsed == FALSE && pie->type == RInputEvent::Mouse)
	if ( pie->type == RInputEvent::Mouse)
	{
		TRACE("Mouse scrollbar %d  %d  %d", pie->sPosX , pie->sPosY, pie->sButtons);

		if ( pie->sButtons)
		{
			short	sX, sY, sTrayX, sTrayY;
			sX =  pie->sPosX;
			sY =  pie->sPosY;
			//rspGetMouse(&sX, &sY, NULL);
			TopPosToChild(&sX, &sY);
			GetTray(&sTrayX, &sTrayY, NULL, NULL);
			sX	-= + sTrayX;
			//sX	+= (800-640);
			//sY	-= m_sClickOffsetY + sTrayY;

			SetPos(PIXELS2POS(sX, sY) );
		}
		pie->sUsed	= TRUE;
	}
#endif

	if (m_btnThumb.m_sPressed != FALSE)
		{				
		short	sX, sY, sTrayX, sTrayY;
		rspGetMouse(&sX, &sY, NULL);
		TopPosToChild(&sX, &sY);
		GetTray(&sTrayX, &sTrayY, NULL, NULL);
		sX	-= m_sClickOffsetX + sTrayX;
		sY	-= m_sClickOffsetY + sTrayY;

		SetPos(PIXELS2POS(sX, sY) );
		}
	else
		{
		// If the event is unused . . .
		if (pie->sUsed == FALSE)
			{
			switch (pie->type)
				{
				case RInputEvent::Key:
					{
					long*	plIncDec	= &m_lButtonIncDec;	// Amount to move thumb.
					// If control held . . .
					if ( (pie->lKey & RSP_GKF_CONTROL) != 0)
						{
						// Use alternate amount.
						plIncDec	= &m_lTrayIncDec;
						}

					// Assume we use it.  Easier.
					pie->sUsed	= TRUE;

					switch (pie->lKey & 0x0000FFFF)
						{
						case RSP_GK_LEFT:
							if (m_oOrientation == Horizontal)
								{
								UserSetPos(GetPos() - *plIncDec); 
								}
							break;
						case RSP_GK_RIGHT:
							if (m_oOrientation == Horizontal)
								{
								UserSetPos(GetPos() + *plIncDec); 
								}
							break;
						case RSP_GK_UP:
							if (m_oOrientation == Vertical)
								{
								UserSetPos(GetPos() - *plIncDec); 
								}
							break;
						case RSP_GK_DOWN:
							if (m_oOrientation == Vertical)
								{
								UserSetPos(GetPos() + *plIncDec); 
								}
							break;
						case RSP_GK_HOME:
							UserSetPos(m_lMinPos);
							break;
						case RSP_GK_END:
							UserSetPos(m_lMaxPos);
							break;
						
						default:	// Did not use.
							// Better clear flag.
							pie->sUsed	= FALSE;
							break;
						}
					
					break;	// on case RInputEvent::Key.
					}
				}
			}

		RGuiItem*	pguiPressed	= NULL;
		// If tray currently pressed . . .
		if (m_sPressed != FALSE)
			{
			pguiPressed	= this;
			}
		else
			{
			// If up currently pressed . . .
			if (m_btnUp.m_sPressed != FALSE)
				{
				pguiPressed	= &m_btnUp;
				}
			else
				{
				// If down currently pressed . . .
				if (m_btnDown.m_sPressed != FALSE)
					{
					pguiPressed	= &m_btnDown;
					}
				}
			}

		// If we found a pressed repeatable GUI . . .
		if (pguiPressed != NULL)
			{
			long	lCurTime	= rspGetMilliseconds();
			if (lCurTime > ms_lNextEventTime)
				{
				// Get current mouse cursor position.
				short	sPosX, sPosY;
				rspGetMouse(&sPosX, &sPosY, NULL);
				// Convert to our coordinate system.
				pguiPressed->TopPosToChild(&sPosX, &sPosY);
				
				// Generate event in pressed item.
				RInputEvent	ie;
				ie.type		= RInputEvent::Mouse;
				ie.lTime		= lCurTime;
				ie.sEvent	= RSP_MB0_PRESSED;
				ie.sButtons	= 1;
				ie.sPosX		= sPosX;
				ie.sPosY		= sPosY;
				ie.sUsed		= FALSE;
				ie.lUser		= 0;
				pguiPressed->m_hot.m_iecUser(&pguiPressed->m_hot, &ie);
				}
			}

		// If we are smooth scrolling . . .
		if (m_sInSmoothScroll != FALSE)
			{
			long	lCurTime	= rspGetMilliseconds();
			// If any time has occurred . . .
			if (lCurTime > m_lLastSmoothTime)
				{
				// Determine amount to scroll.
				// This could overflow in extreme circumstances.
				long	lScroll	= (m_lPosPerSecond * (lCurTime - m_lLastSmoothTime) ) / 1000L;

				// Determine distance to destination.
				long	lDist		= m_lScrollToPos - m_lCurPos;
				// If positive distance . . .
				if (lDist > 0)
					{
					// Don't scroll too far.
					lScroll	= MIN(lScroll, lDist);
					}
				else
					{
					// Don't scroll too far.
					lScroll	= MAX(-lScroll, lDist);
					}

				// Update position (absolute).
				SetPos(m_lCurPos + lScroll);

				// Store time for next iteration.
				m_lLastSmoothTime	= lCurTime;
				}
			}
		}
	}

////////////////////////////////////////////////////////////////////////
//
// Draws an arrow pointing sDirection where the directions are:
//	  0
//	1 + 3
//	  2
// with color or index u32Color into pim at (sX, sY, sW, sH).
// Very cheesy but it'll do for now.
//
////////////////////////////////////////////////////////////////////////
inline void DrawArrow(	// Returns nothing.
	short	sDirection,		// Direction of arrow; see above.
	U32 u32Color,			// Color or index to draw with.
	RImage* pim,			// Destination image.
	short sX,				// Position at which to draw.
	short sY,				// Position at which to draw.
	short sW,				// Amount with which to draw.
	short sH)				// Amount with which to draw.
	{

	// If there is an area . . .
	if (sW > 0 && sH > 0)
		{
		short	sCurX	= sX;
		short	sCurY	= sY;
		short	sCurW	= 1;
		short	sCurH	= 1;
		short* psCurLine;
		short* psCurLinePos;
		short* psCurLineLen;
		short	sLineLenInc;
		short	sLinePosDec;
		short sMaxLineLen;
		short	sMaxLine;
		short	sMinLinePos;

		switch (sDirection)
			{
			case 0:	// Up.
				ASSERT(sW > 0);

				sLineLenInc		= (sH * 2) / sW;
				sLinePosDec		= sLineLenInc;
				sCurY				= sY + sH - sLineLenInc;
				sMaxLineLen		= sH;
				sMaxLine			= sW;
				sMinLinePos		= sY;
				psCurLine		= &sCurX;
				psCurLinePos	= &sCurY;
				psCurLineLen	= &sCurH;
				break;
			case 1:	// Left.
				ASSERT(sH > 0);

				sLineLenInc		= (sW * 2) / sH;
				sLinePosDec		= sLineLenInc;
				sCurX				= sX + sW - sLineLenInc;
				sMaxLineLen		= sW;
				sMaxLine			= sH;
				sMinLinePos		= sX;
				psCurLine		= &sCurY;
				psCurLinePos	= &sCurX;
				psCurLineLen	= &sCurW;
				break;
			case 2:	// Down.
				ASSERT(sW > 0);

				sLineLenInc		= (sH * 2) / sW;
				sLinePosDec		= 0;
				sMaxLineLen		= sH;
				sMaxLine			= sW;
				sMinLinePos		= sY;
				psCurLine		= &sCurX;
				psCurLinePos	= &sCurY;
				psCurLineLen	= &sCurH;
				break;
			case 3:	// Right.
				ASSERT(sH > 0);

				sLineLenInc		= (sW * 2) / sH;
				sLinePosDec		= 0;
				sMaxLineLen		= sW;
				sMaxLine			= sH;
				sMinLinePos		= sX;
				psCurLine		= &sCurY;
				psCurLinePos	= &sCurX;
				psCurLineLen	= &sCurW;
				break;
			}

		*psCurLineLen	= sLineLenInc;

		while (*psCurLineLen < sMaxLineLen)
			{
			rspRect(u32Color, pim, sCurX, sCurY, sCurW, sCurH);
			*psCurLine		+= 1;
			*psCurLineLen	+= sLineLenInc;
			*psCurLinePos	-= sLinePosDec;
			}

		// Store current values just in case height is odd (b/c rounding
		// errors will have occurred and we'll need restore the in-error
		// values).
		short	sTempLineLen	= *psCurLineLen;
		short	sTempLinePos	= *psCurLinePos;

		*psCurLineLen			= sMaxLineLen;
		*psCurLinePos			= sMinLinePos;

		// Add tip line.
		rspRect(u32Color, pim, sCurX, sCurY, sCurW, sCurH);
		
		// If num lines is odd . . .
		if (sMaxLine % 2 == 1)
			{
			// Restore error values so that arrow is symmetrical.
			*psCurLineLen	= sTempLineLen - sLineLenInc;
			*psCurLinePos	= sTempLinePos + sLinePosDec;
			}

		// Next line.
		*psCurLine	+= 1;

		while (*psCurLineLen > 0)
			{
			rspRect(u32Color, pim, sCurX, sCurY, sCurW, sCurH);
			*psCurLine		+= 1;
			*psCurLineLen	-= sLineLenInc;
			*psCurLinePos	+= sLinePosDec;
			}
		}
	}

////////////////////////////////////////////////////////////////////////
//
// Draws an appropriate arrow for this button.
//
////////////////////////////////////////////////////////////////////////
void RScrollBar::DrawUpArrow(	// Returns nothing.
	RImage* pim,					// Image to draw into.  Try to stay within 
										// prc please.
	RRect* prc)						// Where to in image.
	{
	// Fill.
	rspRect(m_btnUp.m_u32BackColor, pim, prc->sX, prc->sY, prc->sW, prc->sH);

	// Draw.
	DrawArrow(m_oOrientation, 
		m_btnUp.m_u32TextColor, pim, 
		prc->sX + m_sArrowBorderDistance,
		prc->sY + m_sArrowBorderDistance,
		prc->sW - m_sArrowBorderDistance * 2,
		prc->sH - m_sArrowBorderDistance * 2);
	}

////////////////////////////////////////////////////////////////////////
//
// Draws an appropriate arrow for this button.
//
////////////////////////////////////////////////////////////////////////
void RScrollBar::DrawDownArrow(	// Returns nothing.
	RImage* pim,						// Image to draw into.  Try to stay within 
											// prc please.
	RRect* prc)							// Where to in image.
	{
	// Fill.
	rspRect(m_btnUp.m_u32BackColor, pim, prc->sX, prc->sY, prc->sW, prc->sH);

	// Draw.
	DrawArrow(m_oOrientation + 2, 
		m_btnUp.m_u32TextColor, pim, 
		prc->sX + m_sArrowBorderDistance,
		prc->sY + m_sArrowBorderDistance,
		prc->sW - m_sArrowBorderDistance * 2,
		prc->sH - m_sArrowBorderDistance * 2);
	}

////////////////////////////////////////////////////////////////////////
//
// Load item's children from the specified file.
// (virtual override).
// (protected).
//
////////////////////////////////////////////////////////////////////////
short RScrollBar::LoadChildren(	// Returns 0 on success.
	RFile*	pfile)					// File to load from.
	{
	short	sRes	= 0;	// Assume success.

	ASSERT(pfile->IsOpen() != FALSE);

	short	sNum;
	// Read number of children.
	pfile->Read(&sNum);

	// The first three are our arrow and thumb buttons.
	ASSERT(sNum >= 3);

	// Load directly into these special children.
	if (m_btnThumb.Load(pfile) == 0)
		{
		if (m_btnUp.Load(pfile) == 0)
			{
			if (m_btnDown.Load(pfile) == 0)
				{
				// Subtract these three children from total.
				sNum	-= 3;
				}
			else
				{
				TRACE("LoadChildren(): m_btnDown.Load() failed.\n");
				sRes	= -3;
				}
			}
		else
			{
			TRACE("LoadChildren(): m_btnUp.Load() failed.\n");
			sRes	= -2;
			}
		}
	else
		{
		TRACE("LoadChildren(): m_btnThumb.Load() failed.\n");
		sRes	= -1;
		}

	// Instantiate rest of children.
	RGuiItem* pgui;
	short	sCurChild;
	for (	sCurChild	= 0; 
			sCurChild < sNum && sRes == 0 && pfile->Error() == FALSE; 
			sCurChild++)
		{
		pgui	= LoadInstantiate(pfile);
		if (pgui != NULL)
			{
			pgui->SetParent(this);
			}
		else
			{
			TRACE("LoadChildren(): LoadInstantiate() failed.\n");
			sRes	= -1;
			}
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////
//
// Save item's children to the specified file.
// (virtual override).
// (protected).
//
////////////////////////////////////////////////////////////////////////
short RScrollBar::SaveChildren(	// Returns 0 on success.
	RFile*	pfile)					// File to save to.
	{
	short	sRes	= 0;	// Assume success.

	ASSERT(pfile->IsOpen() != FALSE);

	// Determine number of child items.
	short	sNum	= 0;
	RGuiItem*	pgui = m_listguiChildren.GetHead();
	while (pgui != NULL)
		{
		sNum++;

		pgui	= m_listguiChildren.GetNext();
		}

	// Write number of children.
	pfile->Write(sNum);

	// These should definitely be a child of 'this' item.
	ASSERT(m_btnThumb.GetParent()	== this);
	ASSERT(m_btnUp.GetParent()		== this);
	ASSERT(m_btnDown.GetParent()	== this);

	// Always write our 3 buttons (arrows and thumb) first so we know
	// where to get them on load.
	if (m_btnThumb.Save(pfile) == 0)
		{
		if (m_btnUp.Save(pfile) == 0)
			{
			if (m_btnDown.Save(pfile) == 0)
				{
				// Subtract these three children from total.
				// Currently this number is not used during save,
				// but just in case it ever is.
				sNum	-= 3;
				}
			else
				{
				TRACE("SaveChildren(): m_btnDown.Save() failed.\n");
				sRes	= -3;
				}
			}
		else
			{
			TRACE("SaveChildren(): m_btnUp.Save() failed.\n");
			sRes	= -2;
			}
		}
	else
		{
		TRACE("SaveChildren(): m_btnThumb.Save() failed.\n");
		sRes	= -1;
		}

	// Save children.  Note that we go through the children in reverse
	// order so they, on load, get added back to their parent in the
	// order they were originally added to this parent.
	pgui	= m_listguiChildren.GetTail();
	while (pgui != NULL && sRes == 0 && pfile->Error() == FALSE)
		{
		// Don't write these 3 again . . .
		if (pgui != &m_btnThumb && pgui != &m_btnUp && pgui != &m_btnDown)
			{
			// Save child.
			sRes	= pgui->Save(pfile);
			}

		pgui	= m_listguiChildren.GetPrev();
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////
// Internal.
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
//
// Read item's members from file.
// (virtual/protected (overriden here)).
//
////////////////////////////////////////////////////////////////////////
short RScrollBar::ReadMembers(	// Returns 0 on success.
	RFile*	pfile,					// File to read from.
	U32		u32Version)				// File format version to use.
	{
	short	sRes	= 0;	// Assume success.

	// Invoke base class to read base members.
	sRes	= RGuiItem::ReadMembers(pfile, u32Version);

	// If okay so far . . .
	if (sRes == 0)
		{
		ASSERT(pfile != NULL);
		ASSERT(pfile->IsOpen() != FALSE);
		
		U32	u32Temp;
		
		// Switch on version.
		switch (u32Version)
			{
			default:
			// Insert additional version numbers here!
			// case 4:	// Version 4 stuff.
			case 3:	// Version 3 stuff.  Smooth scrollage added.
				short	sScrollage;
				pfile->Read(&sScrollage);
				m_scrollage	= (Scrollage)sScrollage;

				pfile->Read(&m_lPosPerSecond);

			case 2:	// Version 2 stuff.

			case 1:
				// Read this class's members.
				pfile->Read(&m_lButtonIncDec);
				pfile->Read(&m_lTrayIncDec);
				pfile->Read(&m_sArrowBorderDistance);
				
				pfile->Read(&u32Temp);
				m_oOrientation	= (Orientation)u32Temp;

				pfile->Read(&m_lMinThumbLength);
				pfile->Read(&m_lMinPos);
				pfile->Read(&m_lMaxPos);
				pfile->Read(&m_lCurPos);

			case 0:	// In version 0, only base class RGuiItem members were stored.
				// If successful . . .
				if (pfile->Error() == FALSE)
					{
					// Success.
					}
				else
					{
					TRACE("ReadMembers(): Error reading RScrollBar members.\n");
					sRes	= -1;
					}
				break;
			}
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////
//
// Write item's members to file.
// (virtual/protected (overriden here)).
//
////////////////////////////////////////////////////////////////////////
short RScrollBar::WriteMembers(	// Returns 0 on success.
	RFile*	pfile)					// File to write to.
	{
	short	sRes	= 0;	// Assume success.

	// Invoke base class to read base members.
	sRes	= RGuiItem::WriteMembers(pfile);

	// If okay so far . . .
	if (sRes == 0)
		{
		ASSERT(pfile != NULL);
		ASSERT(pfile->IsOpen() != FALSE);
		
		// Write this class's members.

		// Version 3.
		pfile->Write((short)m_scrollage);
		pfile->Write(&m_lPosPerSecond);
		// Version 1.
		pfile->Write(&m_lButtonIncDec);
		pfile->Write(&m_lTrayIncDec);
		pfile->Write(&m_sArrowBorderDistance);
		pfile->Write((U32)m_oOrientation);
		pfile->Write(&m_lMinThumbLength);
		pfile->Write(&m_lMinPos);
		pfile->Write(&m_lMaxPos);
		pfile->Write(&m_lCurPos);

		// If successful . . .
		if (pfile->Error() == FALSE)
			{
			// Success.
			}
		else
			{
			TRACE("WriteMembers(): Error writing RScrollBar members.\n");
			sRes	= -1;
			}
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////
// Querries.
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
// Get position/size of tray relative to this item.
////////////////////////////////////////////////////////////////////////
void RScrollBar::GetTray(	// Returns nothing.
	short* psX,					// Out: x coordinate of tray unless NULL.
	short* psY,					// Out: y coordinate of tray unless NULL.
	short* psW,					// Out: Width of tray unless NULL.
	short* psH)					// Out: Height of tray unless NULL.
	{
	GetClient(psX, psY, psW, psH);
	if (m_oOrientation == Vertical)
		{
		SET_IF_NOT_NULL(psY, *psY + m_btnUp.m_im.m_sHeight);
		SET_IF_NOT_NULL(psH, *psH - (m_btnUp.m_im.m_sHeight + m_btnDown.m_im.m_sHeight) );
		}
	else
		{
		SET_IF_NOT_NULL(psX, *psX + m_btnUp.m_im.m_sWidth);
		SET_IF_NOT_NULL(psW, *psW - (m_btnUp.m_im.m_sWidth + m_btnDown.m_im.m_sWidth) );
		}
	}

//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
