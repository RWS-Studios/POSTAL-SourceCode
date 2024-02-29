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
//	bkey.cpp
// 
// History:
//		06/04/04 RCG	Started.
//
//////////////////////////////////////////////////////////////////////////////
//
// Handles all Windows specific keyboard stuff.
//
//////////////////////////////////////////////////////////////////////////////

#include <map>

#include "SDL.h"
#include "BLUE/Blue.h"
#include "ORANGE/CDT/QUEUE.H"


#define MAX_EVENTS	256
// Only set value if not NULL.
#define SET(ptr, val)		( ((ptr) != NULL) ? *(ptr) = (val) : 0 )
#define INC_N_WRAP(i, max)	(i = (i + 1) % max)

extern SDL_Window *sdlWindow;
extern SDL_Surface *sdlShadowSurface;

static bool sdlKeyRepeat = false;

static std::map<SDL_Keycode,U8> sdl_to_rws_keymap;
static std::map<SDL_Keycode,U16> sdl_to_rws_gkeymap;
static U8 keystates[128];
static U8 ms_au8KeyStatus[128];

typedef struct
	{
	long	lKey;
	long	lTime;
	} RSP_SK_EVENT, *PRSP_SK_EVENT;

// Non-dynamic memory for RSP_SK_EVENTs in queue.
static RSP_SK_EVENT	ms_akeEvents[MAX_EVENTS];

// Queue of keyboard events.
static RQueue<RSP_SK_EVENT, MAX_EVENTS>	ms_qkeEvents;

extern bool mouse_grabbed;

//////////////////////////////////////////////////////////////////////////////
// Extern functions.
//////////////////////////////////////////////////////////////////////////////

extern void rspSetQuitStatus(short sQuitStatus);

extern void Key_Event(SDL_Event *event)
{
    ASSERT((event->type == SDL_KEYUP) || (event->type == SDL_KEYDOWN));
    //ASSERT(event->key.keysym.sym < SDLK_LAST);

    const U8 pushed = (event->type == SDL_KEYDOWN);
    if ((pushed) && (event->key.repeat) && (!sdlKeyRepeat))
        return;  // drop it.

    U8 key = sdl_to_rws_keymap[event->key.keysym.sym];
    U16 gkey = sdl_to_rws_gkeymap[event->key.keysym.sym];
    U8* pu8KeyStatus = (&ms_au8KeyStatus[key]);

    if (key == 0)
        return;

    if (pushed)
    {
        if ( (event->key.keysym.sym == SDLK_g) )
        {
            if (event->key.keysym.mod & KMOD_CTRL) // ctrl-g
            {
                const SDL_bool mode = SDL_GetWindowGrab(sdlWindow) ? SDL_FALSE : SDL_TRUE;
                //SDL_SetRelativeMouseMode(mode);
                SDL_SetWindowGrab(sdlWindow, mode);
                mouse_grabbed = (mode == SDL_TRUE);
                return;  // don't pass this key event on to the game.
            }
        }

        else if ( (event->key.keysym.sym == SDLK_RETURN) )
        {
            if (event->key.keysym.mod & KMOD_ALT) // alt-enter
            {
                if (SDL_GetWindowFlags(sdlWindow) & SDL_WINDOW_FULLSCREEN)
                    SDL_SetWindowFullscreen(sdlWindow, 0);
                else
                    SDL_SetWindowFullscreen(sdlWindow, SDL_WINDOW_FULLSCREEN);
                return;  // don't pass this key event on to the game.
            }
        }

        if (ms_qkeEvents.IsFull() == FALSE)
	    {
        	// Create event.
            static short sEventIndex = 0;
	        PRSP_SK_EVENT	pkeEvent	= ms_akeEvents + INC_N_WRAP(sEventIndex, MAX_EVENTS);
    	    // Fill event.
    	    pkeEvent->lTime	= SDL_GetTicks();
        	pkeEvent->lKey = ((gkey) ? gkey : key);

    	    // Enqueue event . . .
	    	if (ms_qkeEvents.EnQ(pkeEvent) == 0)
		    {
    		    // Success.
	    	}
		    else
    		{
	    	    TRACE("Key_Message(): Unable to enqueue key event.\n");
		    }
    	}

        if (key < sizeof (ms_au8KeyStatus))
        {
		    // If key is even . . .
    		if ( ( (*pu8KeyStatus) & 1) == 0)
	    	{
    		    // Go to next odd state.
	    		*pu8KeyStatus	+= 1;
		    }
    		else
	    	{
		        // Go to next odd state.
			    *pu8KeyStatus	+= 2;
    		}
        }
    }

    else  // not pushed.
    {
        if (key < sizeof (ms_au8KeyStatus))
        {
    		// If key is odd . . .
	    	if ( ( (*pu8KeyStatus) & 1) == 1)
		        *pu8KeyStatus	+= 1;

    		// Note that there is intentionally no else condition even though there
	    	// is one in the key down case.
        }
    }

    if (key < sizeof (keystates))
        keystates[key] = pushed;
}

extern void rspClearKeyEvents(void)
{
	// Dequeue all events in the queue
	while (!ms_qkeEvents.IsEmpty())
		ms_qkeEvents.DeQ();
}

//////////////////////////////////////////////////////////////////////////////
//
// Read state of entire keyboard
// Returns nothing.
//
//////////////////////////////////////////////////////////////////////////////
extern void rspScanKeys(
	UCHAR* pucKeys)	// Array of 128 unsigned chars is returned here.
	{
        memcpy(pucKeys, keystates, sizeof (keystates));
	}

//////////////////////////////////////////////////////////////////////////////
//
// Read next key from keyboard queue.
//
//////////////////////////////////////////////////////////////////////////////
extern short rspGetKey(			// Returns 1 if a key was available; 0 if not.
	long* plKey,					// Key info returned here (or 0 if no key available)
	long* plTime /*= NULL*/)	// Key's time stamp returned here (unless NULL)
	{
	short	sRes	= 0;	// Assume no key.

	PRSP_SK_EVENT	pkeEvent	= ms_qkeEvents.DeQ();
	if (pkeEvent != NULL)
		{
		SET(plKey,	pkeEvent->lKey);
		SET(plTime,	pkeEvent->lTime);
		// Indicate a key was available.
		sRes	= 1;
		}
	else
		{
		SET(plKey, 0);
		SET(plTime, 0L);
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Check if a key is available in the keyboard queue via rspGetKey.
//
//////////////////////////////////////////////////////////////////////////////
extern short rspIsKey(void)		// Returns 1 if a key is available; 0 if not.
	{
	return (ms_qkeEvents.IsEmpty() == FALSE) ? 1 : 0;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Initializes arrays for this module.  See the overview for details involving
// the initializations here.
//
// Returns nothing.
//
//////////////////////////////////////////////////////////////////////////////
#define SET_SDL_TO_RWS_KEYMAP2(x,y) sdl_to_rws_keymap[SDLK_##x] = RSP_SK_##y
#define SET_SDL_TO_RWS_KEYMAP(x) SET_SDL_TO_RWS_KEYMAP2(x, x)
#define SET_SDL_TO_RWS_GKEYMAP2(x,y) sdl_to_rws_gkeymap[SDLK_##x] = RSP_GK_##y
#define SET_SDL_TO_RWS_GKEYMAP(x) SET_SDL_TO_RWS_GKEYMAP2(x, x)
extern void Key_Init(void)
	{
	    memset(ms_au8KeyStatus, '\0', sizeof (ms_au8KeyStatus));

		while(!ms_qkeEvents.IsEmpty())  // just in case.
			ms_qkeEvents.DeQ();

        SET_SDL_TO_RWS_KEYMAP(END);
        SET_SDL_TO_RWS_KEYMAP(HOME);
        SET_SDL_TO_RWS_KEYMAP(LEFT);
        SET_SDL_TO_RWS_KEYMAP(UP);
        SET_SDL_TO_RWS_KEYMAP(DOWN);
        SET_SDL_TO_RWS_KEYMAP(RIGHT);
        SET_SDL_TO_RWS_KEYMAP(BACKSPACE);
        SET_SDL_TO_RWS_KEYMAP(TAB);
        SET_SDL_TO_RWS_KEYMAP(INSERT);
        SET_SDL_TO_RWS_KEYMAP(DELETE);
        SET_SDL_TO_RWS_KEYMAP2(RETURN, ENTER);
        SET_SDL_TO_RWS_KEYMAP2(LSHIFT, SHIFT);
        SET_SDL_TO_RWS_KEYMAP2(RSHIFT, SHIFT);
        SET_SDL_TO_RWS_KEYMAP2(LCTRL, CONTROL);
        SET_SDL_TO_RWS_KEYMAP2(RCTRL, CONTROL);
        SET_SDL_TO_RWS_KEYMAP2(LALT, ALT);
        SET_SDL_TO_RWS_KEYMAP2(RALT, ALT);
        SET_SDL_TO_RWS_KEYMAP(PAGEUP);
        SET_SDL_TO_RWS_KEYMAP(PAGEDOWN);
        SET_SDL_TO_RWS_KEYMAP(ESCAPE);
        SET_SDL_TO_RWS_KEYMAP(PAUSE);
        SET_SDL_TO_RWS_KEYMAP(SPACE);
        SET_SDL_TO_RWS_KEYMAP(PRINTSCREEN);
        SET_SDL_TO_RWS_KEYMAP2(QUOTE, RQUOTE);
        SET_SDL_TO_RWS_KEYMAP(COMMA);
        SET_SDL_TO_RWS_KEYMAP(MINUS);
        SET_SDL_TO_RWS_KEYMAP(PERIOD);
        SET_SDL_TO_RWS_KEYMAP(SLASH);
        SET_SDL_TO_RWS_KEYMAP(0);
        SET_SDL_TO_RWS_KEYMAP(1);
        SET_SDL_TO_RWS_KEYMAP(2);
        SET_SDL_TO_RWS_KEYMAP(3);
        SET_SDL_TO_RWS_KEYMAP(4);
        SET_SDL_TO_RWS_KEYMAP(5);
        SET_SDL_TO_RWS_KEYMAP(6);
        SET_SDL_TO_RWS_KEYMAP(7);
        SET_SDL_TO_RWS_KEYMAP(8);
        SET_SDL_TO_RWS_KEYMAP(9);
        SET_SDL_TO_RWS_KEYMAP(SEMICOLON);
        SET_SDL_TO_RWS_KEYMAP(EQUALS);
        SET_SDL_TO_RWS_KEYMAP2(a, A);
        SET_SDL_TO_RWS_KEYMAP2(b, B);
        SET_SDL_TO_RWS_KEYMAP2(c, C);
        SET_SDL_TO_RWS_KEYMAP2(d, D);
        SET_SDL_TO_RWS_KEYMAP2(e, E);
        SET_SDL_TO_RWS_KEYMAP2(f, F);
        SET_SDL_TO_RWS_KEYMAP2(g, G);
        SET_SDL_TO_RWS_KEYMAP2(h, H);
        SET_SDL_TO_RWS_KEYMAP2(i, I);
        SET_SDL_TO_RWS_KEYMAP2(j, J);
        SET_SDL_TO_RWS_KEYMAP2(k, K);
        SET_SDL_TO_RWS_KEYMAP2(l, L);
        SET_SDL_TO_RWS_KEYMAP2(m, M);
        SET_SDL_TO_RWS_KEYMAP2(n, N);
        SET_SDL_TO_RWS_KEYMAP2(o, O);
        SET_SDL_TO_RWS_KEYMAP2(p, P);
        SET_SDL_TO_RWS_KEYMAP2(q, Q);
        SET_SDL_TO_RWS_KEYMAP2(r, R);
        SET_SDL_TO_RWS_KEYMAP2(s, S);
        SET_SDL_TO_RWS_KEYMAP2(t, T);
        SET_SDL_TO_RWS_KEYMAP2(u, U);
        SET_SDL_TO_RWS_KEYMAP2(v, V);
        SET_SDL_TO_RWS_KEYMAP2(w, W);
        SET_SDL_TO_RWS_KEYMAP2(x, X);
        SET_SDL_TO_RWS_KEYMAP2(y, Y);
        SET_SDL_TO_RWS_KEYMAP2(z, Z);
        SET_SDL_TO_RWS_KEYMAP2(LEFTBRACKET, LBRACKET);
        SET_SDL_TO_RWS_KEYMAP(BACKSLASH);
        SET_SDL_TO_RWS_KEYMAP2(RIGHTBRACKET, RBRACKET);
        SET_SDL_TO_RWS_KEYMAP2(KP_EQUALS, NUMPAD_EQUAL);
        SET_SDL_TO_RWS_KEYMAP2(BACKQUOTE, LQUOTE);
        SET_SDL_TO_RWS_KEYMAP2(KP_0, NUMPAD_0);
        SET_SDL_TO_RWS_KEYMAP2(KP_1, NUMPAD_1);
        SET_SDL_TO_RWS_KEYMAP2(KP_2, NUMPAD_2);
        SET_SDL_TO_RWS_KEYMAP2(KP_3, NUMPAD_3);
        SET_SDL_TO_RWS_KEYMAP2(KP_4, NUMPAD_4);
        SET_SDL_TO_RWS_KEYMAP2(KP_5, NUMPAD_5);
        SET_SDL_TO_RWS_KEYMAP2(KP_6, NUMPAD_6);
        SET_SDL_TO_RWS_KEYMAP2(KP_7, NUMPAD_7);
        SET_SDL_TO_RWS_KEYMAP2(KP_8, NUMPAD_8);
        SET_SDL_TO_RWS_KEYMAP2(KP_9, NUMPAD_9);
        SET_SDL_TO_RWS_KEYMAP2(KP_MULTIPLY, NUMPAD_ASTERISK);
        SET_SDL_TO_RWS_KEYMAP2(KP_PLUS, NUMPAD_PLUS);
        SET_SDL_TO_RWS_KEYMAP2(KP_MINUS, NUMPAD_MINUS);
        SET_SDL_TO_RWS_KEYMAP2(KP_PERIOD, NUMPAD_DECIMAL);
        SET_SDL_TO_RWS_KEYMAP2(KP_DIVIDE, NUMPAD_DIVIDE);

        // Map the keypad enter to regular enter key; this lets us
        //  use it in menus, and it can't be assigned to game usage anyhow.
        //SET_SDL_TO_RWS_KEYMAP2(KP_ENTER, NUMPAD_ENTER);
        SET_SDL_TO_RWS_KEYMAP2(KP_ENTER, ENTER);

        SET_SDL_TO_RWS_KEYMAP(F1);
        SET_SDL_TO_RWS_KEYMAP(F2);
        SET_SDL_TO_RWS_KEYMAP(F3);
        SET_SDL_TO_RWS_KEYMAP(F4);
        SET_SDL_TO_RWS_KEYMAP(F5);
        SET_SDL_TO_RWS_KEYMAP(F6);
        SET_SDL_TO_RWS_KEYMAP(F7);
        SET_SDL_TO_RWS_KEYMAP(F8);
        SET_SDL_TO_RWS_KEYMAP(F9);
        SET_SDL_TO_RWS_KEYMAP(F10);
        SET_SDL_TO_RWS_KEYMAP(F11);
        SET_SDL_TO_RWS_KEYMAP(F12);
        SET_SDL_TO_RWS_KEYMAP2(LGUI, SYSTEM);
        SET_SDL_TO_RWS_KEYMAP2(RGUI, SYSTEM);

        // These "stick" until you hit them again, so we should probably
        //  just not pass them on to the app.  --ryan.
        //SET_SDL_TO_RWS_KEYMAP(CAPSLOCK);
        //SET_SDL_TO_RWS_KEYMAP(NUMLOCKCLEAR);
        //SET_SDL_TO_RWS_KEYMAP(SCROLL);

        SET_SDL_TO_RWS_GKEYMAP(END);
        SET_SDL_TO_RWS_GKEYMAP(HOME);
        SET_SDL_TO_RWS_GKEYMAP(LEFT);
        SET_SDL_TO_RWS_GKEYMAP(UP);
        SET_SDL_TO_RWS_GKEYMAP(RIGHT);
        SET_SDL_TO_RWS_GKEYMAP(DOWN);
        SET_SDL_TO_RWS_GKEYMAP(INSERT);
        SET_SDL_TO_RWS_GKEYMAP(DELETE);
        SET_SDL_TO_RWS_GKEYMAP2(LSHIFT, SHIFT);
        SET_SDL_TO_RWS_GKEYMAP2(RSHIFT, SHIFT);
        SET_SDL_TO_RWS_GKEYMAP2(LCTRL, CONTROL);
        SET_SDL_TO_RWS_GKEYMAP2(RCTRL, CONTROL);
        SET_SDL_TO_RWS_GKEYMAP2(LALT, ALT);
        SET_SDL_TO_RWS_GKEYMAP2(RALT, ALT);
        SET_SDL_TO_RWS_GKEYMAP(PAGEUP);
        SET_SDL_TO_RWS_GKEYMAP(PAGEDOWN);
        SET_SDL_TO_RWS_GKEYMAP(PAUSE);
        SET_SDL_TO_RWS_GKEYMAP(PRINTSCREEN);
        SET_SDL_TO_RWS_GKEYMAP2(KP_0, NUMPAD_0);
        SET_SDL_TO_RWS_GKEYMAP2(KP_1, NUMPAD_1);
        SET_SDL_TO_RWS_GKEYMAP2(KP_2, NUMPAD_2);
        SET_SDL_TO_RWS_GKEYMAP2(KP_3, NUMPAD_3);
        SET_SDL_TO_RWS_GKEYMAP2(KP_4, NUMPAD_4);
        SET_SDL_TO_RWS_GKEYMAP2(KP_5, NUMPAD_5);
        SET_SDL_TO_RWS_GKEYMAP2(KP_6, NUMPAD_6);
        SET_SDL_TO_RWS_GKEYMAP2(KP_7, NUMPAD_7);
        SET_SDL_TO_RWS_GKEYMAP2(KP_8, NUMPAD_8);
        SET_SDL_TO_RWS_GKEYMAP2(KP_9, NUMPAD_9);
        SET_SDL_TO_RWS_GKEYMAP2(KP_MULTIPLY, NUMPAD_ASTERISK);
        SET_SDL_TO_RWS_GKEYMAP2(KP_PLUS, NUMPAD_PLUS);
        SET_SDL_TO_RWS_GKEYMAP2(KP_MINUS, NUMPAD_MINUS);
        SET_SDL_TO_RWS_GKEYMAP2(KP_PERIOD, NUMPAD_DECIMAL);
        SET_SDL_TO_RWS_GKEYMAP2(KP_DIVIDE, NUMPAD_DIVIDE);
        SET_SDL_TO_RWS_GKEYMAP(F1);
        SET_SDL_TO_RWS_GKEYMAP(F2);
        SET_SDL_TO_RWS_GKEYMAP(F3);
        SET_SDL_TO_RWS_GKEYMAP(F4);
        SET_SDL_TO_RWS_GKEYMAP(F5);
        SET_SDL_TO_RWS_GKEYMAP(F6);
        SET_SDL_TO_RWS_GKEYMAP(F7);
        SET_SDL_TO_RWS_GKEYMAP(F8);
        SET_SDL_TO_RWS_GKEYMAP(F9);
        SET_SDL_TO_RWS_GKEYMAP(F10);
        SET_SDL_TO_RWS_GKEYMAP(F11);
        SET_SDL_TO_RWS_GKEYMAP(F12);
        SET_SDL_TO_RWS_GKEYMAP2(LGUI, SYSTEM);
        SET_SDL_TO_RWS_GKEYMAP2(RGUI, SYSTEM);

        //SET_SDL_TO_RWS_GKEYMAP(CAPSLOCK);
        //SET_SDL_TO_RWS_GKEYMAP(NUMLOCKCLEAR);
        //SET_SDL_TO_RWS_GKEYMAP(SCROLL);

        rspClearKeyEvents();
	}


//////////////////////////////////////////////////////////////////////////////
// This function returns a pointer to an array of 128 bytes.  Each byte indexed
// by an RSP_SK_* macro indicates the status of that key.  If any element in
// the array is 0 when the corresponding key is pressed, that key is set to 1.
// When that key is released, it is incremented to 2.  When it is pressed again,
// it is incremented to 3, etc., etc..  This array is only cleared by the caller
// for maximum flexibility.  Note that, if the array is cleared, and a key is
// released the entry put into the array will be 2 (not 1) so that the caller
// can rely upon the meaning of evens vs. odds (key currently down vs. up).
// Also, note that this array is static and, therefore, this function need NOT
// be called for every use of the array.  As a matter of fact, you may only need
// to call this function once for an entire program's execution of scans and 
// clears of the array.
//////////////////////////////////////////////////////////////////////////////
U8* rspGetKeyStatusArray(void)	// Returns a ptr to the key status array.
	{
	return ms_au8KeyStatus;
	}

//////////////////////////////////////////////////////////////////////////////
// Set keys that must be pressed in combination with the system 'quit' key.
//////////////////////////////////////////////////////////////////////////////
extern void rspSetQuitStatusFlags(	// Returns nothing.
	long	lKeyFlags)						// In:  New keyflags (RSP_GKF_*).
												// 0 to clear.
	{
    //fprintf(stderr, "STUBBED: %s:%d\n", __FILE__, __LINE__);
	}

//////////////////////////////////////////////////////////////////////////////
// This function returns the state of the three toggle keys (Caps Lock,
// Num Lock, and Scroll Lock).  A key that is 'on' has one of the following
// corresponding bits set in the returned long.
// Note that the values returned by this function are not guaranteed to be
// in synchronization with any of the other key functions.  Their state is
// obtained as close to the current key status as is possible dependent
// upon the platform.
//////////////////////////////////////////////////////////////////////////////
#define RSP_CAPS_LOCK_ON		0x00000001
#define RSP_NUM_LOCK_ON			0x00000002
#define RSP_SCROLL_LOCK_ON		0x00000004

extern long rspGetToggleKeyStates(void)	// Returns toggle key state flags.
	{
	long	lKeyStates	= 0;
#if 0  // !!! FIXME
    Uint8 *states = SDL_GetKeyState(NULL);
    if (states[SDLK_CAPSLOCK]) lKeyStates |= RSP_CAPS_LOCK_ON;
    if (states[SDLK_NUMLOCKCLEAR]) lKeyStates |= RSP_NUM_LOCK_ON;
    if (states[SDLK_SCROLLLOCK]) lKeyStates |= RSP_SCROLL_LOCK_ON;
#endif
	return lKeyStates;
	}


extern void rspKeyRepeat(int bEnable)
{
    sdlKeyRepeat = bEnable;
}


//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////

