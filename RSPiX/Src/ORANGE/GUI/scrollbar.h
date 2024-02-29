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
// ScrollBar.H
// 
// History:
//		01/14/97 JMI	Started tracking history of this file.
//							Added UpdatePosCall and m_upcUser.
//							Also, range macros now consider m_lMinPos for other than
//							0.
//
//		01/15/97	JMI	Added overrides of base class's Save/LoadChildren() to
//							implement special cases for m_btnThumb, Up, & Down.
//
//		01/18/97	JMI	Converted Do() to take an RInputEvent* instead of a 
//							long*.
//
//		01/21/97	JMI	Added ReadMembers() and WriteMembers() overloads to read
//							and write members of this class.
//
//		01/26/97	JMI	Altered static HotCall to accept an RHot* instead of a
//							ULONG 'as per' new RHot callbacks.
//
//		01/30/97	JMI	Took Do() out of here and put in scrollbar.cpp.
//							Fixed rounding error in POS2PIXEL().
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
//		03/19/97	JMI	Converted to using the RHot::m_iecUser (was using
//							RHot::m_epcUser) so HotCall and CursorEvent now take
//							RInputEvent ptrs.
//
//		04/03/97	JMI	Added components for optional timed, smooth scrolling.
//							This included overriding OnLoseFocus().
//
//		06/27/97	JMI	Moved GetTray() definition from here to scrollbar.cpp.
//							SET_IF_NOT_NULL returned the expression even if the
//							passed ptr was NULL.  This would be okay except for
//							the usage in GetTray() included the very same ptr passed
//							and deferenced as part of the expression.
//
//		09/22/97	JMI	Also, added friend class CScrollBarPropPage for GUI 
//							editor.
//
//////////////////////////////////////////////////////////////////////////////
//
// See CPP for description.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef SCROLLBAR_H
#define SCROLLBAR_H

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
	#include "Btn.h"
#endif // PATHS_IN_INCLUDES

//////////////////////////////////////////////////////////////////////////////
// Macros.
//////////////////////////////////////////////////////////////////////////////

// Converts a position value, lPos, to a short pixel position w/i the scroll 
// bar of orientation o.
#define POS2PIXEL(lPos, o)	((short)((m_oOrientation == o) \
												? ((float)(lPos - m_lMinPos) * m_fPos2PixelRatio + 0.5) \
												: 0))

// Converts a position value to a pixel horizontal position w/i the scroll bar.
#define HPOS2PIXEL(lPos)	POS2PIXEL(lPos, Horizontal)

// Converts a position value to a pixel vertical position w/i the scroll bar.
#define VPOS2PIXEL(lPos)	POS2PIXEL(lPos, Vertical)

// Converts a pixel value (relative to Tray) to a long position value.
#define PIXEL2POS(sPix)		((long)((float)(sPix) / m_fPos2PixelRatio) + m_lMinPos)

// Converts one of two pixel values, dependent on orientation o, to a position
// value.
#define PIXELS2POS(sPixX, sPixY)	PIXEL2POS((m_oOrientation == Vertical) \
													? (sPixY) \
													: (sPixX) )

// Sets a value pointed to if ptr is not NULL.
#define SET_IF_NOT_NULL(pval, val)	(((pval) != NULL) \
															? *(pval) = (val) \
															: (0) )


//////////////////////////////////////////////////////////////////////////////
// Typedefs.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
class RScrollBar : public RGuiItem
	{
	public:	// Construction/Destruction.
		// Default constructor.
		RScrollBar(void);
		// Destructor.
		~RScrollBar(void);

//////////////////////////////////////////////////////////////////////////////

	public:
		
		////////////////////////////////////////////////////////////////////////
		// Enums.
		////////////////////////////////////////////////////////////////////////
		typedef enum
			{
			Vertical,		// For a vertical scrollbar.
			Horizontal		// For a horizontal scrollbar.  Very cryptic.
			} Orientation;

		typedef enum
			{
			Instant,			// Snaps to user specified positions.
			Smooth			// Slides to user specified positions.
			} Scrollage;

		////////////////////////////////////////////////////////////////////////
		// Typedefs.
		////////////////////////////////////////////////////////////////////////

		// User callback on change in position.
		typedef void (*UpdatePosCall)(	// Returns nothing.  Called when scroll
													// position is updated by any means.
			RScrollBar* psb);					// this RScrollBar.

	public:	// Methods.

		////////////////////////////////////////////////////////////////////////
		// Methods.
		////////////////////////////////////////////////////////////////////////

		//////////// Overridden ////////////////////////////////////////////////

		// Compose item.
		virtual					// If you override this, call this base if possible.
		void Compose(			// Returns nothing.
			RImage* pim = NULL);	// Dest image, uses m_im if NULL.

		// Cursor event notification.
		// Events in event area.
		virtual						// If you override this, call this base if possible.
		void CursorEvent(			// Returns nothing.
			RInputEvent* pie);	// In:  Most recent user input event.             
										// Out: pie->sUsed = TRUE, if used.

		// Destroys dynamic display data.
		virtual						// If you override this, call this base if possible.
			void Destroy(void)	// Returns nothing.
			{
			// Destroy children we know of.
			m_btnThumb.Destroy();
			m_btnUp.Destroy();
			m_btnDown.Destroy();

			// Destroy self.
			RGuiItem::Destroy();
			}

		// Activate or deactivate mouse reaction.
		virtual					// If you override this, call this base if possible.
		void SetActive(		// Returns nothing.
			short sActive)		// TRUE to make active, FALSE otherwise.
			{ 
			m_btnThumb.SetActive(sActive);
			m_btnUp.SetActive(sActive);
			m_btnDown.SetActive(sActive);
			RGuiItem::SetActive(sActive); 
			}

		// Called by the static implementation of SetFocus() on the item losing
		// the focus.
		// It is okay to call SetFocus() from this function.
		virtual				// If you override this, call this base if possible.
		void OnLoseFocus(void)
			{
			// If scrolling smoothly . . .
			if (m_sInSmoothScroll != FALSE)
				{
				// Jump to destination, ending scrollage.
				SetPos(m_lScrollToPos);
				}

			// Call base class implementation.
			RGuiItem::OnLoseFocus();
			}

		////////////////////////////////////////////////////////////////////////

		// Set the current position of the thumb in the scroll bar.
		// This value will be clipped to m_lMin and m_lMax.
		void SetPos(		// Returns nothing.
			long lPos)		// New position.
			{
			if (lPos > m_lMaxPos)
				{
				m_lCurPos	= m_lMaxPos;
				}
			else
				{
				if (lPos < m_lMinPos)
					{
					m_lCurPos	= m_lMinPos;
					}
				else
					{
					m_lCurPos	= lPos;
					}
				}

			// If we've reached the destination of a smooth scroll . . .
			if (m_lCurPos == m_lScrollToPos)
				{
				m_sInSmoothScroll	= FALSE;
				}

			short	sX, sY, sW, sH;
			GetTray(&sX, &sY, &sW, &sH);

			m_btnThumb.Move(sX + HPOS2PIXEL(m_lCurPos), sY + VPOS2PIXEL(m_lCurPos));

			// If there's a callback . . .
			if (m_upcUser != NULL)
				{
				// Let user know.
				(*m_upcUser)(this);
				}
			}

		// Set the position the thumb will scroll to.
		// This value will be clipped to m_lMin and m_lMax.
		// This GUI must have the focus for correct operation.
		// If this GUI does not have the focus, SetPos() is called.
		void ScrollToPos(		// Returns nothing.
			long lPos)			// In:  New position.
			{
			// If we don't have the focus . . .
			if (ms_pguiFocus != this)
				{
				SetPos(lPos);
				}
			else
				{
				if (lPos > m_lMaxPos)
					{
					m_lScrollToPos	= m_lMaxPos;
					}
				else
					{
					if (lPos < m_lMinPos)
						{
						m_lScrollToPos	= m_lMinPos;
						}
					else
						{
						m_lScrollToPos	= lPos;
						}
					}

				// Start.
				m_sInSmoothScroll	= TRUE;
				// Reset time.
				m_lLastSmoothTime	= rspGetMilliseconds();
				}
			}

		// Sets the range of the scroll bar and sizes & moves the thumb 
		// appropriately.
		void SetRange(
			long lMin, 
			long lMax);

		// Hot call for thumb positioner.
		void ThumbHotCall(		// Returns nothing.
			RInputEvent* pie);	// In:  Most recent user input event.             
										// Out: pie->sUsed = TRUE, if used.

		// Hot call for up arrow.
		void UpHotCall(			// Returns nothing.
			RInputEvent* pie);	// In:  Most recent user input event.             
										// Out: pie->sUsed = TRUE, if used.

		// Hot call for down arrow.
		void DownHotCall(			// Returns nothing.
			RInputEvent* pie);	// In:  Most recent user input event.             
										// Out: pie->sUsed = TRUE, if used.

		// Called when up/left button pressed.
		void UpBtnPressed(void);

		// Called when down/right button pressed.
		void DownBtnPressed(void);

		// Copies border info from this GuiItem to the specified.
		void CopyBorderInfoTo(
			RGuiItem* pgui)		// In: GuiItem to copy to.
			{
			pgui->m_u32BorderColor				= m_u32BorderColor;
			pgui->m_u32BorderHighlightColor	= m_u32BorderHighlightColor;
			pgui->m_u32BorderEdgeColor			= m_u32BorderEdgeColor;
			pgui->m_u32BorderShadowColor		= m_u32BorderShadowColor;
			pgui->m_u32TextColor					= m_u32TextColor;
			pgui->m_u32BackColor					= m_u32BackColor;

//			pgui->m_sBorderThickness			= m_sBorderThickness;
			}

		// Does tasks that require constant update for scroll bar.
		// If called iteratively, the thumb can be dragged on the bar.
		virtual	// If you override this, call this base if possible.
		void Do(						// Returns nothing.
			RInputEvent* pie);	// In:  Most recent user input event.
										// Out: pie->sUsed = TRUE, if used.

		// Draws an appropriate arrow for this button.
		void DrawUpArrow(	// Returns nothing.
			RImage* pim,	// Image to draw into.  Try to stay within 
								// prc please.
			RRect* prc);	// Where to in image.

		// Draws an appropriate arrow for this button.
		void DrawDownArrow(	// Returns nothing.
			RImage* pim,		// Image to draw into.  Try to stay within 
									// prc please.
			RRect* prc);		// Where to in image.

		////////////////////////////////////////////////////////////////////////
		// Querries.
		////////////////////////////////////////////////////////////////////////

		// Get the current position of the thumb in the scroll bar.
		long GetPos(void)
			{ return m_lCurPos; }

		// Gets the range of the scroll bar.
		void GetRange(		// Returns nothing.
			long* plMin,	// Out: Minimum position unless NULL.
			long* plMax)	// Out: Maximum position unless NULL.
			{
			SET_IF_NOT_NULL(plMin, m_lMinPos);
			SET_IF_NOT_NULL(plMax, m_lMaxPos);
			}

		// Get position/size of tray relative to this item.
		void GetTray(	// Returns nothing.
			short* psX,	// Out: x coordinate of tray unless NULL.
			short* psY,	// Out: y coordinate of tray unless NULL.
			short* psW,	// Out: Width of tray unless NULL.
			short* psH);	// Out: Height of tray unless NULL.

		// Get the "hot" area (i.e., clickable area) relative to this item.
		virtual					// If you override this, call this base if possible.
		void GetHot(			// Returns nothing.
			short* psX,			// Out: X position unless NULL.
			short* psY,			// Out: Y position unless NULL.
			short* psW,			// Out: Width unless NULL.
			short* psH)			// Out: Height unless NULL.
			{
			// Use tray area.
			GetTray(psX, psY, psW, psH);
			}

//////////////////////////////////////////////////////////////////////////////

	public:	// Static

//////////////////////////////////////////////////////////////////////////////

	public:	// Querries.

//////////////////////////////////////////////////////////////////////////////

	protected:	// Internal functions.
		// These handle callbacks for the buttons and thumb.  They get an
		// instance for the scroll bar and call the equivalent instantiated
		// (non-static) functions.
		static void UpHotCall(
			RHot*	phot,			// Ptr to RHot that generated event.
			RInputEvent* pie)	// In:  Most recent user input event.             
									// Out: pie->sUsed = TRUE, if used.
			{ ((RScrollBar*)(phot->m_ulUser))->UpHotCall(pie); }
		static void DownHotCall(
			RHot*	phot,			// Ptr to RHot that generated event.
			RInputEvent* pie)	// In:  Most recent user input event.             
									// Out: pie->sUsed = TRUE, if used.
			{ ((RScrollBar*)(phot->m_ulUser))->DownHotCall(pie); }
		static void ThumbHotCall(
			RHot*	phot,			// Ptr to RHot that generated event.
			RInputEvent* pie)	// In:  Most recent user input event.             
									// Out: pie->sUsed = TRUE, if used.
			{ ((RScrollBar*)(phot->m_ulUser))->ThumbHotCall(pie); }
		static void DrawUpArrow(	// Returns nothing.
			RGuiItem* pgui,			// The RGuiItem being composed (this).
			RImage* pim,				// Image to draw into.  Try to stay within 
											// prc please.
			RRect* prc)					// Where to in image.
			{ ((RScrollBar*)pgui->m_ulUserInstance)->DrawUpArrow(pim, prc); }
		static void DrawDownArrow(	// Returns nothing.
			RGuiItem* pgui,			// The RGuiItem being composed (this).
			RImage* pim,				// Image to draw into.  Try to stay within 
											// prc please.
			RRect* prc)					// Where to in image.
			{ ((RScrollBar*)pgui->m_ulUserInstance)->DrawDownArrow(pim, prc); }

		// Save item's children to the specified file.
		virtual					// Overridden here.
		short SaveChildren(	// Returns 0 on success.
			RFile*	pfile);	// File to save to.

		// Load item's children from the specified file.
		virtual					// Overridden here.
		short LoadChildren(	// Returns 0 on success.
			RFile*	pfile);	// File to load from.

		// Read item's members from file.
		virtual				// Overridden here.
		short ReadMembers(			// Returns 0 on success.
			RFile*	pfile,			// File to read from.
			U32		u32Version);	// File format version to use.

		// Write item's members to file.
		virtual				// Overridden here.
		short WriteMembers(			// Returns 0 on success.
			RFile*	pfile);			// File to write to.

		// Set position, usually as requested by user, using the specified
		// type of user feedback.
		void UserSetPos(	// Returns nothing.
			long	lPos)		// In:  New position.
			{
			switch (m_scrollage)
				{
				case Instant:
					SetPos(lPos);
					break;
				case Smooth:
					ScrollToPos(lPos);
					break;
				}
			}

//////////////////////////////////////////////////////////////////////////////

	public:	// Member variables.
		// Feel free to set the callbacks for these three Guis or the base class
		// to your own functions.  But if you change gui.m_bcUser or 
		// gui.m_ulUserInstace, you are responsible for calling SetPos() or the
		// callbacks (e.g., UpBtnPressed(void), DownBtnPressed(void), or 
		// TrayPressed(void)) as appropriate.
		RBtn			m_btnThumb;			// Mobile positioner/indicator.
		RBtn			m_btnUp;				// Up or left arrow button.
		RBtn			m_btnDown;			// Down or right arrow button.

		long			m_lButtonIncDec;	// Amount to increment or decrement the
												// position when a button is pressed.
		long			m_lTrayIncDec;		// Amount to increment or decrement the
												// position when the tray is pressed.

		short			m_sArrowBorderDistance;	// Number of pixels between arrows'
														// edges and borders.

		UpdatePosCall	m_upcUser;		// User callback on postion change.

		Scrollage	m_scrollage;		// { RScrollBar::Instant,
												// RScrollBar::Smooth }

		long			m_lPosPerSecond;	// Number of positions smoothly scrolled
												// through in a second.  The rate of
												// smooth scrollage.

		long			m_lLastSmoothTime;	// Last time smooth scroll was processed.

		long			m_lScrollToPos;	// Position to smoothly scroll to.
												// This GUI should have the focus to get
												// correct operation.

		short			m_sInSmoothScroll;	// TRUE, if we are in a smooth scroll.
													// FALSE, otherwise.

		// These values may be querried and can be changed directly.
		// BUT, they may not affect the appearance of the scroll bar
		// until Compose() is called.
		Orientation	m_oOrientation;	// { RScrollBar::Vertical,
												// RScrollBar::Horizontal }

		long			m_lMinThumbLength;	// Minimum length for thumb.

		short			m_sClickOffsetX;	// Position in thumb that was clicked.
		short			m_sClickOffsetY;	// Position in thumb that was clicked.

	protected:	// Internal typedefs.

	protected:	// Protected member variables.

		long			m_lMinPos;				// Minimum value.
		long			m_lMaxPos;				// Maximum value.

		long			m_lCurPos;				// Current value.

		float			m_fPos2PixelRatio;	// Current ratio of scroll bar 
													// positions pixels to pixels.

	///////////////////////////////////////////////////////////////////////////
	// Friends.
	///////////////////////////////////////////////////////////////////////////
	friend class CScrollBarPropPage;

	};

#endif // SCROLLBAR_H
//////////////////////////////////////////////////////////////////////////////
//	EOF
//////////////////////////////////////////////////////////////////////////////
