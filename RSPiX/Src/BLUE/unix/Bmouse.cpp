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
///////////////////////////////////////////////////////////////////////////////
//
//	bmouse.cpp
// 
// History:
//		06/03/04 RCG  Started.
//
//////////////////////////////////////////////////////////////////////////////
//
// Handles all SDL specific mouse stuff.
//
//////////////////////////////////////////////////////////////////////////////

#include "SDL.h"

#include "BLUE/Blue.h"

#include "ORANGE/CDT/QUEUE.H"

extern SDL_Window *sdlWindow;
extern SDL_Surface *sdlShadowSurface;
extern int sdlWindowWidth;
extern int sdlWindowHeight;

typedef struct
	{
	short	sX;
	short	sY;
	short	sButton;
	long	lTime;
	short	sType;
	} RSP_MOUSE_EVENT, *PRSP_MOUSE_EVENT;

#define MAX_EVENTS	256
// Only set value if not NULL.
#define SET(ptr, val)		( ((ptr) != NULL) ? *(ptr) = (val) : 0)
#define INC_N_WRAP(i, max)	(i = (i + 1) % max)

static RSP_MOUSE_EVENT	ms_ameEvents[MAX_EVENTS];

static RQueue<RSP_MOUSE_EVENT, MAX_EVENTS>	ms_qmeEvents;

extern bool mouse_grabbed;

///////////////////////////////////////////////////////////////////////////////
// Module specific (static) globals.
///////////////////////////////////////////////////////////////////////////////
static short				ms_sCursorShowLevel	= 0;

///////////////////////////////////////////////////////////////////////////////
// Functions.
///////////////////////////////////////////////////////////////////////////////
static int MouseWheelState = 0;

///////////////////////////////////////////////////////////////////////////////
//
// Puts the coordinates of the mouse position in your shorts.
// Note GetAsyncKeyState returns current button state info (unlike
// GetKeyState); however, if we do not have keyboard focus, it returns 0, or so
// it is documented.
// Returns nothing.
// 
///////////////////////////////////////////////////////////////////////////////
extern void rspGetMouse(
		short* psX,				// Current x position is returned here (unless NULL)
		short* psY,				// Current y position is returned here (unless NULL)
		short* psButton)		// Current button status is returned here (unless NULL)
	{

    if (!mouse_grabbed)
    {
        int w, h;
        SET(psX, 0);
        SET(psY, 0);
        SET(psButton, 0);
        return;  // drop mouse events if input isn't grabbed.
    }

    int x, y;
    const Uint32 buttons = SDL_GetMouseState(&x, &y);
    SET(psX, x);
    SET(psY, y);

	if (psButton != NULL)
		{
            *psButton = (buttons & SDL_BUTTON_LMASK) ? 0x0001 : 0;
            *psButton |= (buttons & SDL_BUTTON_RMASK) ? 0x0002 : 0;
            *psButton |= (buttons & SDL_BUTTON_MMASK) ? 0x0004 : 0;
			*psButton |= (buttons & SDL_BUTTON_X1MASK) ? 0x0008 : 0;
			*psButton |= (buttons & SDL_BUTTON_X2MASK) ? 0x0010 : 0;
			*psButton |= (MouseWheelState & 0x0020) ? 0x0020 : 0;
			*psButton |= (MouseWheelState & 0x0040) ? 0x0040 : 0;
		}

	MouseWheelState = 0;
	}


extern void Mouse_Event(SDL_Event *event)
{
	static short	sEventIndex	= 0;


    if (!mouse_grabbed)
        return;  // drop mouse events if input isn't grabbed.

	// Get next event.  We do not "new" a RSP_MOUSE_EVENT here to avoid 
	// memory fragmentation.
	PRSP_MOUSE_EVENT	pme = ms_ameEvents + INC_N_WRAP(sEventIndex, MAX_EVENTS);
    pme->lTime = SDL_GetTicks();
    pme->sType = event->type;

    static short buttonState = 0;

	bool bQueueMouseWheelRelease = false;

	buttonState &= ~(0x0020 | 0x0040);

    switch (event->type)
    {
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
        {
            pme->sX = event->button.x;
            pme->sY = event->button.y;
            int val;
            switch (event->button.button)
            {
                case SDL_BUTTON_LEFT: val = 0x0001; break;
                case SDL_BUTTON_RIGHT: val = 0x0002; break;
                case SDL_BUTTON_MIDDLE: val = 0x0004; break;
				case SDL_BUTTON_X1MASK: val = 0x0008; break;
				case SDL_BUTTON_X2MASK: val = 0x0010; break;
                default: val = 0; break;
            }

            if (event->button.state == SDL_PRESSED)
                buttonState |= val;
            else
                buttonState &= ~val;

            pme->sButton = buttonState;
            break;
        }
		case SDL_MOUSEWHEEL:
		{
			int val;
			if (event->wheel.y > 0)
				val = 0x0020;
			else if (event->wheel.y < 0)
				val = 0x0040;
			else
				val = 0;

			buttonState |= val;
			MouseWheelState = val;
			pme->sButton = buttonState;
			bQueueMouseWheelRelease = true;
			break;
		}

        default:  // uh?
            ASSERT(0 && "unexpected mouse event!");
            return;
    }

	if (ms_qmeEvents.IsFull() != FALSE)
		{
		// Discard oldest event.
		ms_qmeEvents.DeQ();
		}

	// Enqueue event . . .
	if (ms_qmeEvents.EnQ(pme) == 0)
		{
		// Success.
		}
	else
		{
		TRACE("Mouse_Message(): Unable to enqueue mouse event.\n");
		}
	
	// Add "dummy" mouse wheel button release event.
	if (bQueueMouseWheelRelease)
	{
		PRSP_MOUSE_EVENT newpme = ms_ameEvents + INC_N_WRAP(sEventIndex, MAX_EVENTS);
		newpme->lTime = SDL_GetTicks();
		newpme->sType = SDL_MOUSEBUTTONUP;
		newpme->sButton = MouseWheelState;
		ms_qmeEvents.EnQ(newpme);
	}
}

///////////////////////////////////////////////////////////////////////////////
//
// Sets the mouse position to your shorts.
// Returns nothing.
// 
///////////////////////////////////////////////////////////////////////////////
extern void rspSetMouse(
		short sX,				// New x position.
		short sY)				// New y position.
	{
        if (!mouse_grabbed)
            return;  // drop mouse events if input isn't grabbed.
        SDL_WarpMouseInWindow(sdlWindow, sX, sY);
	}

//////////////////////////////////////////////////////////////////////////////
//
// Get most recent (last) mouse event from queue (using short coords).
// This function tosses out any events ahead of the last event in the queue!
// 
///////////////////////////////////////////////////////////////////////////////
extern short rspGetLastMouseEvent(	// Returns 0 if no event was available, non-zero otherwise
	short*	psX,						// Event's X position is returned here (unless NULL)
	short*	psY,						// Event's Y position is returned here (unless NULL)
	short*	psButton,				// Event's button status is returned here (unless NULL)
	long*		plTime,					// Event's time stamp returned here (unless NULL)
	short*	psType /*= NULL*/)	// Event's type (as per OS) is returned here (unless NULL)
	{
	short	sRes	= TRUE;	// Assume success.

	PRSP_MOUSE_EVENT	peEvent;
	short					sNumEvents	= ms_qmeEvents.NumItems();

	// Are there any events?
	if (sNumEvents > 0)
		{
		while (sNumEvents-- > 0)
			{
			peEvent	= ms_qmeEvents.DeQ();
			}

		if (peEvent != NULL)
			{
			SET(psX,			peEvent->sX);
			SET(psY,			peEvent->sY);
			SET(psButton,	peEvent->sButton);
			SET(plTime,		peEvent->lTime);
			SET(psType,		peEvent->sType);
			}
		else
			{
			TRACE("rspGetLastMouseEvent(): Unable to dequeue last event.\n");
			sRes = FALSE;
			}
		}
	else
		{
		sRes	= FALSE;
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Get next mouse button event from queue (using short coords).
// Returns 0 on success.
// 
///////////////////////////////////////////////////////////////////////////////
extern short rspGetMouseEvent(	// Returns 0 if no event was available, non-zero otherwise
	short*	psX,						// Event's X position is returned here (unless NULL)
	short*	psY,						// Event's Y position is returned here (unless NULL)
	short*	psButton,				// Event's button status is returned here (unless NULL)
	long*		plTime,					// Event's time stamp returned here (unless NULL)
	short*	psType /*= NULL*/)	// Event's type (as per OS) is returned here (unless NULL)
	{
	short	sRes	= TRUE;	// Assume success.

	PRSP_MOUSE_EVENT	peEvent	= ms_qmeEvents.DeQ();
	if (peEvent != NULL)
		{
		SET(psX,			peEvent->sX);
		SET(psY,			peEvent->sY);
		SET(psButton,	peEvent->sButton);
		SET(plTime,		peEvent->lTime);
		SET(psType,		peEvent->sType);
		}
	else
		{
		sRes = FALSE;
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Clear mouse event queue
// Returns nothing.
// 
///////////////////////////////////////////////////////////////////////////////
extern void rspClearMouseEvents(void)
	{
	while (ms_qmeEvents.DeQ() != NULL);
	}

//////////////////////////////////////////////////////////////////////////////
//
// Hide OS mouse cursor
// Returns nothing.
// 
///////////////////////////////////////////////////////////////////////////////
extern void rspHideMouseCursor(void)
	{
	// Decrement show cursor count.
    if (--ms_sCursorShowLevel < 0)
	    SDL_ShowCursor(0);
	}

//////////////////////////////////////////////////////////////////////////////
//
// Show OS mouse cursor
// Returns nothing.
// 
///////////////////////////////////////////////////////////////////////////////
extern void rspShowMouseCursor(void)
	{
	// Increment show cursor count.
	if (++ms_sCursorShowLevel >= 0)
	    SDL_ShowCursor(1);
	}

///////////////////////////////////////////////////////////////////////////////
// 
// Shield the OS mouse cursor from screen updates.
// This function hides the cursor in the fastest way possible for the current
// type of screen updatage.  Calls to rspShieldMouseCursor() and 
// rspUnshieldMouseCursor() encapsulating screen updates is the preferrable 
// way to shield the cursor from screen updates.
// Note that to get data from the screen you should first hide the cursor
// with rspHideMouseCursor() as this function may not actually 'erase' the
// cursor.
// This is NOT synonymous to rspHideMouseCursor().
// 
///////////////////////////////////////////////////////////////////////////////
extern void rspShieldMouseCursor(void)
	{
	}

///////////////////////////////////////////////////////////////////////////////
// 
// Unshield the OS mouse cursor from screen updates (i.e., show the cursor
// after a rspShieldMouseCursor() call to protect the cursor from a direct
// screen write).
// This is NOT synonymous to rspShowMouseCursor().
// 
///////////////////////////////////////////////////////////////////////////////
extern void rspUnshieldMouseCursor(void)
	{
	}



///////////////////////////////////////////////////////////////////////////////
// 
// Reports current mouse cursor show level.
// 
///////////////////////////////////////////////////////////////////////////////
short rspGetMouseCursorShowLevel(void)	// Returns current mouse cursor show level:
													// Positive indicates cursor is shown.
													// Non-positive indicates cursor is hidden.
	{
	return ms_sCursorShowLevel;
	}

///////////////////////////////////////////////////////////////////////////////
// 
// Sets current mouse cursor show level.
// 
///////////////////////////////////////////////////////////////////////////////
void rspSetMouseCursorShowLevel(	// Returns nothing.
	short sNewShowLevel)				// In:  Current mouse cursor show level:        
											// Positive indicates cursor is shown.     
											// Non-positive indicates cursor is hidden.
	{
	while (ms_sCursorShowLevel < sNewShowLevel)
		{
		rspShowMouseCursor();
		}

	while (ms_sCursorShowLevel > sNewShowLevel)
		{
		rspHideMouseCursor();
		}
	}

//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
