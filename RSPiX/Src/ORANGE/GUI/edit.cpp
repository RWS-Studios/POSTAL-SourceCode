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
// EDIT.CPP
// 
// History:
//		08/15/96 JMI	Started.
//
//		09/24/96	JMI	Changed all BLU_MB?_* macros to RSP_MB?_* macros.
//
//		10/02/96	JMI	No longer checks ms_szText for NULL before calling
//							m_print stuff and updating m_aptTextPos.
//
//		10/27/96 MJR	Fixed "unused variable" warnings.
//
//		10/31/96	JMI	Changed:
//							Old label:		New label:
//							=========		=========
//							CEdit				REdit
//							CTxt				RTxt
//							CImage			RImage
//							CGuiItem			RGuiItem
//							LEFT				RGuiItem::Left
//							MULTILINE		Multiline
//							NUMBERS_ONLY	NumbersOnly
//							POINT				Point
//							EDITNOTIFY		EditNotify
//
//							Moved #include <string.h> to the beginning.
//
//		10/30/96	JMI	Changed:
//							Old label:		New label:
//							=========		=========
//							Rect				RRect
//
//		11/12/96 MJR	Added RSP_ prefix to GK keyboard macros.
//
//		11/14/96	JMI	DrawText() now attempts to clip the string to the edit
//							box.
//
//		11/27/96	JMI	Added initialization of m_type to identify this type
//							of GUI item.
//
//		12/04/96	JMI	Do() can no longer optionally poll for input, does not 
//							call rspGetKey(), and does not return that key.  Instead,
//							you pass the a pointer to the rspGetKeys() formatted key 
//							you want Do() to process.  The key will be zeroed if it
//							is "absorbed" by the edit box.
//
//		12/19/96	JMI	Uses new m_justification (as m_sJustification) and 
//							upgraded to new RFont/RPrint.
//
//		12/31/96	JMI	Do() now calls base implementation in RGuiItem.
//
//		01/01/97	JMI	DrawText() now draws a space with transparent back and
//							foreground colors so that the positions array will be
//							updated so we can use the caret position.
//							Also, now clips caret.
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
//		01/08/97	JMI	There were some typos in Draw() involving clipping.
//							Fixed.
//
//		01/18/97	JMI	Converted Do() to take an RInputEvent* instead of a 
//							long*.
//
//		01/20/97	JMI	Made very few changes for overriding Read/WriteMembers.  
//							Should not change functionality.  Checked in to use at
//							home.
//
//		01/21/97	JMI	Added ReadMembers() and WriteMembers() overloads to read
//							and write members of this class.  Note that they call the
//							base class version to read/write base class members.
//							Support exists for versions 0 and 1.  Version 0 did not
//							contain other than RGuiItem members.
//
//		01/23/97	JMI	Added escape to keys we don't want to process during Do().
//
//		01/23/97	JMI	Now sets m_sShowFocus to FALSE in constructor.  Focus is
//							shown via caret anyhow.
//
//		02/05/97	JMI	Changed position of default: case in ReadMembers().
//
//		03/14/97	JMI	Added ClipCaret().
//
//		03/19/97	JMI	Converted to using the RHot::m_iecUser (was using
//							RHot::m_epcUser) so HotCall and CursorEvent now take
//							RInputEvent ptrs.
//
//		03/28/97	JMI	RSP_MB0_DOUBLECLICK is now treated the same as
//							RSP_MB0_PRESSED.
//
//		04/10/97	JMI	DrawText() considers new m_sFontCellHeight member.
//							Now uses m_sFontCellHeight instead of GetPos() to get
//							cell height.
//
//		04/24/97	JMI	Added m_u32TextShadowColor.
//
//		07/07/97	JMI	Now calls SetTextEffects() before drawing text in 
//							DrawText() and Draw().
//
//		08/25/97	JMI	Added m_sFirstVisibleCharIndex which is the first visible
//							character in the field.
//							Now automagically scrolls left or right based on caret
//							positions.
//
//		08/25/97	JMI	Was not handling the case where the text was truncated by
//							an outside source.  Now handled by calling ClipCaret() in
//							DrawText().
//
//		08/25/97	JMI	Added macros to define extra space on each side of text
//							in the edit field to make sure it doesn't butt up against
//							stuff.
//							Also, effects, justification, size, and word-wrappage
//							were not being set before drawing the caret in Draw().
//
//		08/25/97	JMI	Now scrolls half the field to the left if the caret 
//							passes the left edge (used to only scroll one character).
//
//		08/30/97	JMI	Now repaginates if the caret position is less than the
//							first visible character.
//
//		09/01/97	JMI	Now Do() only functions if this is activated.
//
//		01/15/98	JMI	Added numeric keypad support in Do().
//
//////////////////////////////////////////////////////////////////////////////
//
// This a GUI item that is based on the CTxt item. 
// This overrides CursorEvent() to get information about where a click it
// occurred.
// This overrides DrawText() to store the postion of each character.
// This overrides Draw() to add a cursor.
//
// Enhancements/Uses:
// To change the look of the text, you may want to override DrawText()
// in a derived class or see CTxt for other overridables
// To change the background of the text item, see CGuiItem.
// To get a callback on a click/release pair in the text item, see CTxt.
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Headers.
//////////////////////////////////////////////////////////////////////////////

#include <string.h>

#include "Blue.h"

#ifdef PATHS_IN_INCLUDES
	#include "ORANGE/GUI/edit.h"
#else
	#include "edit.h"
#endif // PATHS_IN_INCLUDES

//////////////////////////////////////////////////////////////////////////////
// Module specific macros.
//////////////////////////////////////////////////////////////////////////////

// Sets val to def if val is -1.
#define DEF(val, def)	((val == -1) ? def : val)

// Number of pixels to reserve as non-client on each vertical edge.
#define VERT_EDGE_RESERVED		1

#define EXTRA_NONCLIENT_EDGE	(m_sShowFocus ? m_sBorderThickness + VERT_EDGE_RESERVED : VERT_EDGE_RESERVED)

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
REdit::REdit()
	{
	///////////////////////////////////////////////////////////////////////////
	// Defaults:
	///////////////////////////////////////////////////////////////////////////
	
	m_cCaretChar		= '_';	// Character to use as caret.
	m_u32CaretColor	= 255;	// Color to use for caret.
	m_sCaretPos			= 0;		// Text position of caret.
	m_lCaretBlinkRate	= 500;	// Rate at which character blinks in ms.  Can be
										// 0 indicating no blinkage.
	m_sMaxText			= GUI_MAX_STR;	// Maximum text to allow.  Limited to 
												// GUI_MAX_STR.
	m_sBehavior			= 0;		// Flags.  See enums in header.

	m_encCall			= NULL;	// Callback when a user input notification 
										// should occur such as too much input or 
										// invalid character generated (e.g., alphas 
										// in NUMBERS_ONLY mode).  A good place to 
										// generate a beep or something.

	m_lNextCaretUpdate	= 0;	// Time in ms of next caret update.
	m_sCaretState			= 0;	// Current state the caret is in until 
										// m_lNextCaretUpdate. (0 == hidden, 
										// 1 == shown).

	m_sInvertedBorder		= TRUE;	// Override RGuiItem's default.

	m_type					= Edit;	// Indicates type of GUI item.

	m_sShowFocus			= FALSE;	// Focus is shown via the caret.

	m_sFirstVisibleCharIndex	= 0;	// First visible char in field.
	}

//////////////////////////////////////////////////////////////////////////////
//
// Destructor.
//
//////////////////////////////////////////////////////////////////////////////
REdit::~REdit()
	{
	}

////////////////////////////////////////////////////////////////////////
// Methods.
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
//
// Draw this item and all its subitems into the provided RImage.
// This override adds in the caret.
//
////////////////////////////////////////////////////////////////////////
short REdit::Draw(			// Returns 0 on success.
	RImage* pimDst,			// Destination image.
	short sDstX	/*= 0*/,		// X position in destination.
	short sDstY	/*= 0*/,		// Y position in destination.
	short sSrcX /*= 0*/,		// X position in source.
	short sSrcY /*= 0*/,		// Y position in source.
	short sW /*= 0*/,			// Amount to draw.
	short sH /*= 0*/,			// Amount to draw.
	RRect* prc /*= NULL*/)	// Clip to.
	{
	short	sRes	= 0;	// Assume success.

	// If visible . . .
	if (m_sVisible != FALSE)
		{
		// Call the base class.
		sRes = RGuiItem::Draw(	pimDst, sDstX, sDstY,
										sSrcX, sSrcY, sW, sH, prc);

		// If the caret is shown . . .
		if (m_sCaretState != 0)
			{
			// Make sure the caret is inside the string.
			ClipCaret();

			// If the caret is less than the first visible index (this can happen
			// if an outside party sets the caret position) . . .
			if (m_sCaretPos < m_sFirstVisibleCharIndex)
				{
				// We'll need to 'repaginate' the field.
				Compose();
				}

			// Setup printer ////////////////////////////////

			// Word wrap must be off.
			short	sWordWrapWas	= (m_pprint->m_eModes & RPrint::WORD_WRAP) ? TRUE : FALSE;
			m_pprint->SetWordWrap(FALSE);

			// Text support is currently 8 bit only.
			m_pprint->SetColor((short)m_u32CaretColor, (short)0, (short)m_u32TextShadowColor);

			m_pprint->SetFont(m_sFontCellHeight);

			SetJustification();
			SetTextEffects();
			m_pprint->SetDestination(pimDst);
	
			// Done setting up printer //////////////////////

			short	sClientX, sClientY, sClientW, sClientH;
			GetClient(&sClientX, &sClientY, &sClientW, &sClientH);

			short	sClipW, sClipH;

			if (sW == 0)
				{
				// Use client.
				sClipW	= sClientW;
				}
			else
				{
				// Adjust by blt position.
				sClipW	= sW - (sClientX + m_sX);
				}

			if (sH == 0)
				{
				// Use client.
				sClipH	= sClientH;
				}
			else
				{
				// Adjust by blt position.
				sClipH	= sH - (sClientY + m_sY);
				}

			// Clip.
			m_pprint->SetColumn(
				sDstX + m_sX + sClientX + EXTRA_NONCLIENT_EDGE, 
				sDstY + m_sY + sClientY, 
				sClipW - EXTRA_NONCLIENT_EDGE * 2, 
				sClipH);

			ASSERT(m_sCaretPos - m_sFirstVisibleCharIndex >= 0);

			m_pprint->print(
				sDstX + m_sX + m_aptTextPos[m_sCaretPos - m_sFirstVisibleCharIndex].sX, 
				sDstY + m_sY + m_aptTextPos[m_sCaretPos - m_sFirstVisibleCharIndex].sY,
				"%c", m_cCaretChar);

			// Restore printer //////////////////////////////

			m_pprint->SetWordWrap(sWordWrapWas);
			}
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////
//
// Draw text in m_szText in m_u32TextColor with transparent
// background at sX, sY with sW and m_sJustification.
// The only additional functionality this has over RGuiItem::DrawText is
// that it blts the characters one at a time and tracks the position of
// each character in m_aptTextPos.
// If m_szText is empty, a space is printed with the foreground and
// background colors set to transparent.  This way, the array of 
// positions is set for the caret.
//
////////////////////////////////////////////////////////////////////////
short REdit::DrawText(		// Returns 0 on success.
	short sX,					// X position in image.
	short sY,					// Y position in image.
	short sW /*= 0*/,			// Width of text area.
	short	sH /*= 0*/,			// Height of test area.
	RImage* pim /*= NULL*/)	// Destination image.  NULL == use m_im.
	{
	short	sRes	= 0;	// Assume success.

	if (pim == NULL)
		{
		// Use internal image.
		pim	= &m_im;
		}

	char*	pszText;
	U32	u32ForeColor;

	if (m_szText[0] != '\0')
		{
		// Use user supplied text.
		pszText	= m_szText;
		// Use user supplied color.
		u32ForeColor	= m_u32TextColor;
		}
	else
		{
		// Use fake text and colors so that the positions
		// array is updated.  It would be handy if the
		// print command would update the array even for
		// printing "".  Then this would not be necessary.
		static char szSpace[]	= " ";

		// Display space so that we can guarantee that the
		// character positions array will be updated.
		pszText	= szSpace;

		// Use transparent color (just in case actually has
		// some pixels).
		u32ForeColor	= 0;
		}

	// Text support is currently 8 bit only.
	m_pprint->SetColor(u32ForeColor, 0, m_u32TextShadowColor);
	
	// Set size.  Hopefully this won't do too much scaling for 
	// caching purposes but I'm not sure.
	m_pprint->SetFont(m_sFontCellHeight);

	// Only left justified editting supported . . .
	if (m_justification == RGuiItem::Left)
		{
		// Added in extra space before starting text.
		sX	+= EXTRA_NONCLIENT_EDGE;
		// Remove extra space before starting text.
		sW	-= EXTRA_NONCLIENT_EDGE * 2;

		// Keep the caret within bounds.
		ClipCaret();

		SetJustification();
		SetTextEffects();

		// Word wrap must be off.
		short	sWordWrapWas	= (m_pprint->m_eModes & RPrint::WORD_WRAP) ? TRUE : FALSE;
		m_pprint->SetWordWrap(FALSE);

		m_pprint->SetDestination(pim);
		m_pprint->SetColumn(sX, sY, sW, sH);
		m_pprint->GetWidth(pszText);

		long	lLen				= strlen(pszText);

		// Whenever the caret hits or passes the left edge, jump the first visible
		// character back.
		if (m_sCaretPos < m_sFirstVisibleCharIndex)
			{
			short	sHalfWidth	= sW / 2;
			// Go backward until we hit the beginning
			// or we get within half the field of the caret.

			while (m_sFirstVisibleCharIndex > 0)
				{
				if (m_pprint->ms_sCharPosX[m_sCaretPos] - m_pprint->ms_sCharPosX[m_sFirstVisibleCharIndex] >= sHalfWidth)
					{
					break;
					}

				m_sFirstVisibleCharIndex--;
				}
			}

		short	sCaretWidth	= m_pprint->GetBlitW(m_cCaretChar);

		// Whenever the caret hits or passes the right edge, jump the first 
		// visible character forward.
		if (m_pprint->ms_sCharPosX[m_sCaretPos] - m_pprint->ms_sCharPosX[m_sFirstVisibleCharIndex] + sCaretWidth >= sW - EXTRA_NONCLIENT_EDGE)
			{
			short	sHalfWidth	= sW / 2;
			// Go forward until we hit the end
			// or we get within half the field of the caret.

			while (m_sFirstVisibleCharIndex < lLen)
				{
				if (m_pprint->ms_sCharPosX[m_sCaretPos] - m_pprint->ms_sCharPosX[m_sFirstVisibleCharIndex] <= sHalfWidth)
					{
					break;
					}

				m_sFirstVisibleCharIndex++;
				}
			}

		ASSERT(m_sFirstVisibleCharIndex <= lLen);
		ASSERT(m_sFirstVisibleCharIndex >= 0);

		m_pprint->print(sX, sY, "%s", pszText + m_sFirstVisibleCharIndex);
	

		REdit::Point*	ppt	= m_aptTextPos;
		long	lIndex;
		for (lIndex	= 0; lIndex < lLen; lIndex++, ppt++)
			{
			// Store its position in the pim.
			ppt->sX	= m_pprint->ms_sCharPosX[lIndex];
			ppt->sY	= sY;	// Eventually will be available as above.
			}

		// Update last position for cursor.
		ppt->sX	= m_pprint->ms_sCharPosX[lIndex];
		ppt->sY	= sY;	// Eventually will be available as above.

		m_pprint->SetWordWrap(sWordWrapWas);
		}
	else
		{
		TRACE("DrawText(): Only left justification supported by REdit "
			"currently.\n");
		sRes	= -1;
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////
//
// Does REdit stuff like check for keys, update caret, and add new 
// text.
//
////////////////////////////////////////////////////////////////////////
void REdit::Do(		// Returns nothing.
	RInputEvent* pie)	// In:  Most recent user input event.
							// Out: pie->sUsed = TRUE, if used.
	{
	// Call base.
	RTxt::Do(pie);

	// Only if currenly active . . .
	if (IsActivated() != FALSE)
		{
		short sRedrawText	= FALSE;	// Redraw text if TRUE.
		short	sRedrawItem	= FALSE;	// Redraw item if TRUE (for event driven 
											// user).
		
		// If unused key event . . .
		if (pie->sUsed == FALSE && pie->type == RInputEvent::Key)
			{
			// Set the caret state to show.
			m_sCaretState	= 1;

			// Get length of current text.  Always handy.
			long	lStrLen	= strlen(m_szText);

			// Remember caret position.
			short	sCaretPosIn	= m_sCaretPos;

			// Char to add or 0.
			long	lNewChar	= 0;

			// Make sure the caret is inside the string.
			ClipCaret();

			// Switch on extended keys.
			switch (pie->lKey & 0x0000FF00)
				{
				case 0x00000000:	// ASCII key.
					// Switch on ASCII key.
					switch (pie->lKey & 0x000000FF)
						{
						case '\b':
							if (m_sCaretPos > 0)
								{
								// Remove character at cursor.
								memmove(	m_szText + m_sCaretPos - 1,
											m_szText + m_sCaretPos,
											lStrLen - m_sCaretPos + 1);
								
								m_sCaretPos--;

								// Need to redraw text.
								sRedrawText	= TRUE;
								}
							else
								{
								NotifyCall();
								}

							// Absorb key.
							pie->sUsed	= TRUE;

							break;

						case '\r':
							if (m_sBehavior & REdit::Multiline)
								{
								TRACE("Do(): REdit::Multiline NYI.\n");
								NotifyCall();

								// Absorb key.
								pie->sUsed	= TRUE;
								}
							break;

						// Keys we don't want to process.
						case '\t':
						case 27:
							break;

						default:

							lNewChar	= pie->lKey;

							// Absorb key.
							pie->sUsed	= TRUE;

							break;
						}
					break;

				// Numpad numeric keys.
				case RSP_GK_NUMPAD_0:
				case RSP_GK_NUMPAD_1:
				case RSP_GK_NUMPAD_2:
				case RSP_GK_NUMPAD_3:
				case RSP_GK_NUMPAD_4:
				case RSP_GK_NUMPAD_5:
				case RSP_GK_NUMPAD_6:
				case RSP_GK_NUMPAD_7:
				case RSP_GK_NUMPAD_8:
				case RSP_GK_NUMPAD_9:

					lNewChar	= '0' + ( (pie->lKey - RSP_GK_NUMPAD_0) >> 8);

					// Absorb key.
					pie->sUsed	= TRUE;

					break;

				// Numpad other keys.
				// NOTE:  This is essentially a table so, if it gets too large, let's use a real table
				// instead.  Maybe there should be a function.
				case RSP_GK_NUMPAD_ASTERISK:
					lNewChar	= '*';
					// Absorb key.
					pie->sUsed	= TRUE;
					break;
				case RSP_GK_NUMPAD_PLUS		:
					lNewChar	= '+';
					// Absorb key.
					pie->sUsed	= TRUE;
					break;
				case RSP_GK_NUMPAD_MINUS	:
					lNewChar	= '-';
					// Absorb key.
					pie->sUsed	= TRUE;
					break;
				case RSP_GK_NUMPAD_DECIMAL	:
					lNewChar	= '.';
					// Absorb key.
					pie->sUsed	= TRUE;
					break;
				case RSP_GK_NUMPAD_DIVIDE	:
					lNewChar	= '/';
					// Absorb key.
					pie->sUsed	= TRUE;
					break;

				// Single line caret movement.
				case RSP_GK_LEFT:
					if (m_sCaretPos > 0)
						{
						m_sCaretPos--;
						}
					else
						{
						NotifyCall();
						}

					// Absorb key.
					pie->sUsed	= TRUE;

					break;

				case RSP_GK_RIGHT:
					if (m_sCaretPos < lStrLen)
						{
						m_sCaretPos++;
						}
					else
						{
						NotifyCall();
						}

					// Absorb key.
					pie->sUsed	= TRUE;

					break;

				case RSP_GK_DELETE:
					if (m_sCaretPos < lStrLen)
						{
						// Remove character at cursor.
						memmove(	m_szText + m_sCaretPos,
									m_szText + m_sCaretPos + 1,
									lStrLen - m_sCaretPos);
						
						// Need to redraw text.
						sRedrawText	= TRUE;
						}
					else
						{
						NotifyCall();
						}

					// Absorb key.
					pie->sUsed	= TRUE;

					break;

				case RSP_GK_HOME:
					m_sCaretPos	= 0;

					// Absorb key.
					pie->sUsed	= TRUE;

					break;

				case RSP_GK_END:
					m_sCaretPos	= (short)lStrLen;

					// Absorb key.
					pie->sUsed	= TRUE;

					break;

					// Multiline caret movement.
				case RSP_GK_UP:
					if (m_sBehavior & REdit::Multiline)
						{
						TRACE("Do(): REdit::Multiline NYI.\n");
						NotifyCall();
						}

					// Absorb key.
					pie->sUsed	= TRUE;

					break;

				case RSP_GK_DOWN:
					if (m_sBehavior & REdit::Multiline)
						{
						TRACE("Do(): REdit::Multiline NYI.\n");
						NotifyCall();
						}

					// Absorb key.
					pie->sUsed	= TRUE;

					break;
				}

			// If we should add a key . . .
			if (lNewChar)
				{
				if (lStrLen < m_sMaxText - 1)
					{
					// Add character to string.
					// Move existing characters (perhaps only '\0')
					// out of the way.
					memmove(	m_szText + m_sCaretPos + 1, 
								m_szText + m_sCaretPos, 
								lStrLen - m_sCaretPos + 1);
					// Place character and advance caret position.
					m_szText[m_sCaretPos++]	= (char)lNewChar;
					// Need to redraw text.
					sRedrawText	= TRUE;
					}
				else
					{
					NotifyCall();
					}
				}

			// If change in caret pos . . .
			if (m_sCaretPos != sCaretPosIn)
				{
				// We must make sure the caret is visible.
				Compose();
				sRedrawItem	= TRUE;
				}
			}

		if (m_lCaretBlinkRate != 0)
			{
			// If the next blink time has expired . . .
			long	lCurTime	= rspGetMilliseconds();
			if (lCurTime >= m_lNextCaretUpdate)
				{
				// Set next blink time.
				m_lNextCaretUpdate	= lCurTime + m_lCaretBlinkRate;
				// If caret is to be shown . . .
				if (m_sCaretState == 1)
					{
					// Set caret state.
					m_sCaretState	= 0;
					}
				else
					{
					// Set caret state.
					m_sCaretState	= 1;
					}

				// Redraw needed.
				sRedrawItem	= TRUE;
				}
			}
		else
			{
			m_sCaretState	= 1;
			}

		// If we need to redraw the text . . .
		if (sRedrawText != FALSE)
			{
			// Recompose the item.
			Compose();
			sRedrawItem	= TRUE;
			}

		// If a redraw is necessary . . .
		if (sRedrawItem != FALSE)
			{
			// Cause a redraw for the event driven user.
			Redraw();
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
void REdit::CursorEvent(	// Returns nothing.
	RInputEvent* pie)			// In:  Most recent user input event.             
									// Out: pie->sUsed = TRUE, if used.
	{
	RGuiItem::CursorEvent(pie);

	switch (pie->sEvent)
		{
		case RSP_MB0_DOUBLECLICK:
		case RSP_MB0_RELEASED:
			{
			// Set the caret position based on the area clicked.
			char*		pszText	= m_szText;
			Point*	ppt		= m_aptTextPos;
			m_sCaretPos			= m_sFirstVisibleCharIndex;

			short	sCellH		= m_sFontCellHeight;

			// Adjust sPosY to bottom of line.
			short sPosY	= pie->sPosY + sCellH;

			while (*pszText++ != '\0')
				{
				if (ppt->sX < pie->sPosX && ppt->sY < sPosY)
					{
					m_sCaretPos++;
					ppt++;
					}
				else
					{
					break;
					}
				}

			// Make sure it gets shown.
			m_lNextCaretUpdate	= 0L;
			m_sCaretState			= 0;

			// Note that we used it.
			pie->sUsed	= TRUE;

			break;
			}
		}
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
short REdit::ReadMembers(			// Returns 0 on success.
	RFile*	pfile,					// File to read from.
	U32		u32Version)				// File format version to use.
	{
	short	sRes	= 0;	// Assume success.

	// Invoke base class to read base members.
	sRes	= RTxt::ReadMembers(pfile, u32Version);

	// If okay so far . . .
	if (sRes == 0)
		{
		ASSERT(pfile != NULL);
		ASSERT(pfile->IsOpen() != FALSE);
		
		// Switch on version.
		switch (u32Version)
			{
			default:
			// Insert additional version numbers here!
			// case 4:	// Version 4 stuff.
			// case 3:	// Version 3 stuff.
			case 2:	// Version 2 stuff.

			case 1:
				// Read this class's members.
				pfile->Read(&m_cCaretChar);
				pfile->Read(&m_u32CaretColor);
				pfile->Read(&m_lCaretBlinkRate);
				pfile->Read(&m_sMaxText);
				pfile->Read(&m_sBehavior);

			case 0:	// In version 0, only base class RGuiItem members were stored.
				// If successful . . .
				if (pfile->Error() == FALSE)
					{
					// Success.
					}
				else
					{
					TRACE("ReadMembers(): Error reading REdit members.\n");
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
short REdit::WriteMembers(			// Returns 0 on success.
	RFile*	pfile)					// File to write to.
	{
	short	sRes	= 0;	// Assume success.

	// Invoke base class to read base members.
	sRes	= RTxt::WriteMembers(pfile);

	// If okay so far . . .
	if (sRes == 0)
		{
		ASSERT(pfile != NULL);
		ASSERT(pfile->IsOpen() != FALSE);
		
		// Write this class's members.
		pfile->Write(&m_cCaretChar);
		pfile->Write(&m_u32CaretColor);
		pfile->Write(&m_lCaretBlinkRate);
		pfile->Write(&m_sMaxText);
		pfile->Write(&m_sBehavior);

		// If successful . . .
		if (pfile->Error() == FALSE)
			{
			// Success.
			}
		else
			{
			TRACE("WriteMembers(): Error writing REdit members.\n");
			sRes	= -1;
			}
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////
// Clips the caret to within the string length.
////////////////////////////////////////////////////////////////////////
void REdit::ClipCaret(void)		// Returns nothing.
	{
	long	lLen	= strlen(m_szText);
	if (m_sCaretPos > lLen)
		{
		// After last char.
		m_sCaretPos	= (short)lLen;
		}
	}

////////////////////////////////////////////////////////////////////////
// Querries.
////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
