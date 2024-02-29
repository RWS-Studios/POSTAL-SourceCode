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
//	bdisp.cpp
// 
// History:
//		06/04/04 RCG	Started.
//
//////////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////////

#include "SDL.h"
#include "Blue.h"
#include "ORANGE/CDT/slist.h"

#include <ctype.h>

extern SDL_Window *sdlWindow;
static char *sdlAppName;
static SDL_Renderer *sdlRenderer;
static SDL_Texture *sdlTexture;
static int RequestedWidth = 0;
static int RequestedHeight = 0;
static int FramebufferWidth = 0;
static int FramebufferHeight = 0;
static Uint32 *TexturePointer = NULL;
static Uint8 *PalettedTexturePointer = NULL;

typedef struct		// Stores information on usable video modes.
	{
	short				sWidth;
	short				sHeight;
	short				sColorDepth;
	short				sPages;
	} VIDEO_MODE, *PVIDEO_MODE;

static RSList<VIDEO_MODE, short>	slvmModes;	// List of available video modes.

typedef union { struct { Uint8 b; Uint8 g; Uint8 r; Uint8 a; }; Uint32 argb; } ArgbColor;
static ArgbColor	apeApp[256];				// App's palette.  The palette
														// entries the App actually set.

static ArgbColor	apeMapped[256];			// Tweaked palette.
														// This is the palette updated to
														// the hardware.  apeApp is trans-
														// lated through au8MapRed, Green,
														// and Blue and stored here for
														// updating to the hardware on
														// rspUpdatePalette().

static U8					au8MapRed[256];			// Map of red intensities to hardware
														// values.  Initially an identity
														// mapping.
static U8					au8MapGreen[256];			// Map of green intensities to
														// hardware values.  Initially an 
														// identity mapping.
static U8					au8MapBlue[256];			// Map of blue intensities to hardware
														// values.  Initially an identity
														// mapping.

static short				asPalEntryLocks[256];	// TRUE, if an indexed entry is locked.
														// FALSE, if not.  Implemented as 
														// shorts in case we ever do levels of
														// locking.

extern bool mouse_grabbed;

//////////////////////////////////////////////////////////////////////////////
// Module specific macros.
//////////////////////////////////////////////////////////////////////////////

// Only set value if not NULL.
#define SET(ptr, val)		( ((ptr) != NULL) ? *(ptr) = (val) : 0)


//////////////////////////////////////////////////////////////////////////////
//
// Clips [(*px,*py),(*pw,*ph)] into [(sx,sy),(sw, sh)].
//
//////////////////////////////////////////////////////////////////////////////
extern short Clip(	// Returns non-zero if image entirely clipped out.
	short*	px,		// Rectangle to be clipped.
	short*	py, 
	short*	pw, 
	short*	ph,
	short		sx,		// Bounding rectangle.
	short		sy, 
	short		sw, 
	short		sh)
	{
	if (*px < sx)
		{
		TRACE("Clip(): x too small.\n");
		// Adjust width.
		*pw -= sx - *px;
		// Adjust x.
		*px = sx;
		}

	if (*py < sy)
		{
		TRACE("Clip(): y too small.\n");
		// Adjust height.
		*ph -= sy - *py;
		// Adjust y.
		*py = sy;
		}

	if (*px + *pw > sw)
		{
		TRACE("Clip(): Width or x too large.\n");
		// Adjust width.
		*pw -= ((*px + *pw) - sw);
		}

	if (*py + *ph > sh)
		{
		TRACE("Clip(): Height or y too large.\n");
		// Adjust height.
		*ph -= ((*py + *ph) - sh);
		}
	
	// Return 0 (success) if there's a width and a height left.	
	return (short)(*pw > 0 && *ph > 0 ? 0 : -1);
	}

//////////////////////////////////////////////////////////////////////////////
//
// Clips [(*px,*py),(*pw,*ph)] into [(sx,sy),(sw, sh)].
//
//////////////////////////////////////////////////////////////////////////////
extern short ClipQuiet(	// Returns non-zero if image entirely clipped out.
	short*	px,			// Rectangle to be clipped.
	short*	py, 
	short*	pw, 
	short*	ph,
	short		sx,			// Bounding rectangle.
	short		sy, 
	short		sw, 
	short		sh)
	{
	if (*px < sx)
		{
		// Adjust width.
		*pw -= sx - *px;
		// Adjust x.
		*px = sx;
		}

	if (*py < sy)
		{
		// Adjust height.
		*ph -= sy - *py;
		// Adjust y.
		*py = sy;
		}

	if (*px + *pw > sw)
		{
		// Adjust width.
		*pw -= ((*px + *pw) - sw);
		}

	if (*py + *ph > sh)
		{
		// Adjust height.
		*ph -= ((*py + *ph) - sh);
		}
	
	// Return 0 (success) if there's a width and a height left.	
	return (short)(*pw > 0 && *ph > 0 ? 0 : -1);
	}

//////////////////////////////////////////////////////////////////////////////
//
// Clips [(*px,*py),(*pw,*ph)] into [(sx,sy),(sw, sh)].
// Uses short version of function ClipQuiet(...).
//
//////////////////////////////////////////////////////////////////////////////
extern short ClipQuiet(	// Returns non-zero if image entirely clipped out.
	long* px,				// Rectangle to be clipped.
	long* py, 
	long*	pw, 
	long*	ph,
	long	sx,			// Bounding rectangle.
	long	sy, 
	long	sw, 
	long	sh)
	{
	short	sX	= (short)(*px);
	short sY	= (short)(*py);
	short	sW	= (short)(*pw);
	short	sH	= (short)(*ph);

	short	sRes	= ClipQuiet(&sX, &sY, &sW, &sH, 
									(short)sx, (short)sy, (short)sw, (short)sh);

	*px	= sX;
	*py	= sY;
	*pw	= sW;
	*ph	= sH;

	return sRes;
	}


short CompareModes(PVIDEO_MODE pvm1, PVIDEO_MODE pvm2);

extern void Disp_Init(void)	// Returns nothing.
{
    extern char **_argv;
    const int arg = rspCommandLine("resolution");
    if ((arg) && (_argv[arg+1]))
    {
        if (SDL_sscanf(_argv[arg+1], "%dx%d", &RequestedWidth, &RequestedHeight) != 2)
            RequestedWidth = RequestedHeight = 0;
    }

    if ((RequestedWidth <= 0) || (RequestedHeight <= 0))
    {
        if (rspCommandLine("windowed"))
        {
            RequestedWidth = 1024;
            RequestedHeight = 768;
        }
        else
        {
            RequestedWidth = 0;
            RequestedHeight = 0;
        }
    }

	// Initialize maps to indentities.
	short i;
	for (i = 0; i < 256; i++)
		{
		au8MapRed[i]	= i;
		au8MapGreen[i]	= i;
		au8MapBlue[i]	= i;
		}

	// Never ever ever unlock these.
	asPalEntryLocks[0]	= TRUE;
	asPalEntryLocks[255]	= TRUE;

	slvmModes.SetCompareFunc(CompareModes);
}

extern void rspSetApplicationName(
	char* pszName)								// In: Application name
{
    SDL_free(sdlAppName);
    sdlAppName = SDL_strdup(pszName);
    if (sdlWindow)
        SDL_SetWindowTitle(sdlWindow, sdlAppName);
}


//////////////////////////////////////////////////////////////////////////////
//
// Compares two video modes in order of sColorDepth, sWidth, sHeight,
// sPageFlippage.
// Returns 0			if *pvm1 == *pvm2.
// Returns negative	if *pvm1 < *pvm2.
// Returns positive	if *pvm1 > *pvm2.
//
//////////////////////////////////////////////////////////////////////////////
extern short CompareModes(	// Returns as described above.
		PVIDEO_MODE	pvm1,		// First video mode to compare.
		PVIDEO_MODE	pvm2)		// Second video mode to compare.
	{
	short	sRes	= 1;	// Assume *pvm1 > *pvm2.

	if (pvm1->sColorDepth == pvm2->sColorDepth)
		{
		if (pvm1->sWidth == pvm2->sWidth)
			{
			if (pvm1->sHeight == pvm2->sHeight)
				{
				if (pvm1->sPages == pvm2->sPages)
					{
					sRes = 0;
					}
				else
					{
					if (pvm1->sPages < pvm2->sPages)
						{
						sRes = -1;
						}
					}
				}
			else
				{
				if (pvm1->sHeight < pvm2->sHeight)
					{
					sRes = -1;
					}
				}
			}
		else
			{
			if (pvm1->sWidth < pvm2->sWidth)
				{
				sRes = -1;
				}
			}
		}
	else
		{
		if (pvm1->sColorDepth < pvm2->sColorDepth)
			{
			sRes = -1;
			}
		}

	return sRes;
	}



//////////////////////////////////////////////////////////////////////////////
//
// Attempts to find a mode that is sWidth by sHeight or larger
// with the given color depth.  An available mode that closest matches
// the width and height is chosen, if successful.  If no mode is found, 
// *psWidth, *psHeight.  If psPixelDoubling is not NULL and *psPixelDoubling
// is TRUE, a mode may be returned that requires pixel doubling.  If a 
// mode requires pixel doubling, *psPixelDoubling will be TRUE on return;
// otherwise, it will be FALSE.  Passing psPixelDoubling as NULL is
// equivalent to passing *psPixelDoubling with FALSE.
// Utilizes rspQueryVideoMode to find the mode.  Does not affect the current 
// rspQueryVideoMode.
// This function should not be a part of Blue.  It is always implementable
// via rspQueryVideoMode.
//
//////////////////////////////////////////////////////////////////////////////
extern short rspSuggestVideoMode(		// Returns 0 if successfull, non-zero otherwise
	short		sDepth,							// In:  Required depth
	short		sWidth,							// In:  Requested width
	short		sHeight,							// In:  Requested height
	short		sPages,							// In:  Required pages
	short		sScaling,						// In:  Requested scaling
	short*	psDeviceWidth /*= NULL*/,	// Out: Suggested device width (unless NULL)
	short*	psDeviceHeight /*= NULL*/,	// Out: Suggested device height (unless NULL)
	short*	psScaling /*= NULL*/)		// Out: Suggested scaling (unless NULL)
	{
	short	sRes	= 0;	// Assume success.

	// Store video mode that the app is currently iterating.
	PVIDEO_MODE	pvmOldModeQuery	= slvmModes.GetCurrent();

	rspQueryVideoModeReset();

	// Query results.
	short	sModeWidth;
	short	sModeHeight;
	short sModeColorDepth;
	short	sModePages;

	// Best results.
	short sBestModeWidth		= 16380;
	short	sBestModeHeight	= 16380;
	short	sModeFound			= FALSE;

	while (rspQueryVideoMode(&sModeColorDepth, &sModeWidth, &sModeHeight, &sModePages) == 0)
		{
		// Must be same color depth.
		if (sModeColorDepth == sDepth && sPages == sModePages)
			{
			// If the desired resolution would fit into this mode . . .
			if (sWidth <= sModeWidth && sHeight <= sModeHeight)
				{
				// If this mode is closer than a previous one . . .
				float	fFactorOld	= ((float)sBestModeWidth	* (float)sBestModeHeight)
										/ ((float)sWidth				* (float)sHeight);
				float	fFactorNew	= ((float)sModeWidth			* (float)sModeHeight)
										/ ((float)sWidth				* (float)sHeight);
				if (fFactorNew < fFactorOld)
					{
					sBestModeWidth		= sModeWidth;
					sBestModeHeight	= sModeHeight;
					sModeFound			= TRUE;
					}
				}
			}
		}

	// If we found an acceptable mode . . .
	if (sModeFound != FALSE)
		{
		// If pixel doubling was specified . . .
		if (psScaling != NULL)
			{
			// If pixel doubling is allowed . . .
			if (sScaling != FALSE)
				{
				// If the chosen mode is more than or equal to twice the 
				// requested mode . . . 
				if (sWidth * 2 <= sBestModeWidth 
					&& sHeight * 2 <= sBestModeHeight)
					{
					// Okay to pixel double.  Leave *psPixelDoubling as TRUE.
					// Reduce best width and height appropriately.
					sBestModeWidth		/= 2;
					sBestModeHeight	/= 2;
					}
				else
					{
					// No pixel doubling possible for this mode.
					*psScaling	= FALSE;
					}
				}
			}

		*psDeviceWidth		= sBestModeWidth;
		*psDeviceHeight	= sBestModeHeight;
		}
	else
		{
		// Failed to find an acceptable mode.
		sRes	= 1;
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Puts parameters about the hardware video mode into your shorts.
// You may call this function even when in "no mode" (e.g., before 
// rspSetVideoMode is first called, after it fails, or after rspKillVideMode
// is called).  This way you can get information on the user's current mode.
// If in "no mode", psWidth, psHeight, and psPages will receive 0, if not NULL.
//
//////////////////////////////////////////////////////////////////////////////
extern short rspGetVideoMode(
	short*	psDeviceDepth,				// Hardware display color depth returned here 
												// (unless NULL).
	short*	psDeviceWidth,				// Hardware display width returned here 
												// (unless NULL).
	short*	psDeviceHeight,			// Hardware display height returned here 
												// (unless NULL).
	short*	psDevicePages,				// Hardware display back buffers returned here
												// (unless NULL).
	short*	psWidth,						// Display area width returned here 
												// (unless NULL).
	short*	psHeight,					// Display area height returned here
												// (unless NULL).
	short*	psPages/*= NULL*/,			// Number of pages (1 to n) returned here 
												// (unless NULL).  More than 1 indicates a 
												// page flipping scenario.
	short*	psPixelScaling/*= NULL*/)	// Pixel scaling in effect (1) or not (0)
													// (unless NULL).
{
    // lie about everything.
    SET(psPixelScaling, 0);
    SET(psDevicePages, 0);
    SET(psPages, 1);
    SET(psWidth, FramebufferWidth);
    SET(psHeight, FramebufferHeight);
    SET(psDeviceDepth, 8);
    SET(psDeviceHeight, FramebufferWidth);
    SET(psDeviceWidth, FramebufferHeight);

    return 0;
}

//////////////////////////////////////////////////////////////////////////////
//
// Reset the query process.  Must be called before calling QueryVideoModes()
// for the first time.  See QueryVideoModes() for more.
//
//////////////////////////////////////////////////////////////////////////////

static void addMode(int w, int h, int depth)
{
    PVIDEO_MODE pvm;

    if (depth >= 8)
    {
        pvm = new VIDEO_MODE;
        pvm->sWidth = w;
        pvm->sHeight = h;
        pvm->sColorDepth = 8;
        pvm->sPages = 1;
        slvmModes.Insert(pvm);
    }

    if (depth >= 16)
    {
        pvm = new VIDEO_MODE;
        pvm->sWidth = w;
        pvm->sHeight = h;
        pvm->sColorDepth = 16;
        pvm->sPages = 1;
        slvmModes.Insert(pvm);
    }

    if (depth >= 24)
    {
        pvm = new VIDEO_MODE;
        pvm->sWidth = w;
        pvm->sHeight = h;
        pvm->sColorDepth = 32;
        pvm->sPages = 1;
        slvmModes.Insert(pvm);
    }
}

extern void rspQueryVideoModeReset(void)
{
    static bool enumerated = false;
    if (!enumerated)
    {
        ASSERT(SDL_WasInit(SDL_INIT_VIDEO));

		// Attempt to grab user's current desktop resolution instead of forcing 640x480
#ifndef MOBILE
		SDL_DisplayMode dm_Mode;
		int i_Result = SDL_GetDesktopDisplayMode(0, &dm_Mode);
		if (!i_Result)
			addMode(dm_Mode.w, dm_Mode.h, 8);
		else // Fall back to 640x480
#endif
			addMode(640, 480, 8);

        enumerated = true;
    }

	slvmModes.GetHead();
}

//////////////////////////////////////////////////////////////////////////////
//
// Query the available video modes.  The modes are returned in sorted order
// based on increasing color depth, width, and height, in that order.  The
// next time the function is called after the last actual mode was reported,
// it will return non-zero (failure) to indicate that no more modes are
// available, and will continue to do so until QueryVideoReset() is
// called to reset it back to the first video mode.  When the return value is
// non-zero, the other parameters are not updated.
//
//////////////////////////////////////////////////////////////////////////////
extern short rspQueryVideoMode(			// Returns 0 for each valid mode, then non-zero thereafter
	short* psColorDepth,						// Color depth (8, 15, 16, 24, 32)
													// Unless NULL.
	short* psWidth /*= NULL*/,				// Width returned here
													// Unless NULL.
	short* psHeight /*= NULL*/,			// Height returned here
													// Unless NULL.
	short* psPages /*= NULL*/)				// Number of video pages possible.
	{
	short	sRes	= 0;	// Assume success.

	PVIDEO_MODE	pvm	= slvmModes.GetCurrent();

	if (pvm != NULL)
		{
		SET(psColorDepth,	pvm->sColorDepth);
		SET(psWidth,			pvm->sWidth);
		SET(psHeight,			pvm->sHeight);

		SET(psPages,			pvm->sPages);

		// Goto next video mode.
		slvmModes.GetNext();
		}
	else
		{
		sRes = 1;
		}

	return sRes;
	}


static SDL_Renderer *createRendererToggleVsync(SDL_Window *window, const int index, bool vsync)
{
    SDL_Renderer *retval = NULL;
    if (vsync)
        retval = SDL_CreateRenderer(window, index, SDL_RENDERER_PRESENTVSYNC);
    if (!retval)
        retval = SDL_CreateRenderer(window, index, 0);
    return retval;
}

static SDL_Renderer *createRendererByName(SDL_Window *window, const char *name)
{
    const bool vsync = !rspCommandLine("novsync");
    if (name == NULL)
        return createRendererToggleVsync(window, -1, vsync);
    else
    {
        const int max = SDL_GetNumRenderDrivers();
        for (int i = 0; i < max; i++)
        {
            SDL_RendererInfo info;
            if ((SDL_GetRenderDriverInfo(i, &info) == 0) && (SDL_strcmp(info.name, name) == 0))
                return createRendererToggleVsync(window, i, vsync);
        }
    }
    return NULL;
}


//////////////////////////////////////////////////////////////////////////////
//
// Set a new video mode.  Specified color depth, width, and height for the
// device is absolute.  If these parameters cannot be met, the function will
// fail. If the requested resolution is higher than the requested display area,
// it will be centered with a black border around it.  This function can be
// called multiple times to change modes, but it does not create new
// display areas; instead, the previous buffer is destroyed and a new buffer
// is created to take it's place.
// If pixel doubling is allowed (with rspAllowPixelDoubling) and the requested
// display area is less than or equal to half the requested hardware resolution, 
// pixel doubling will be activated.
// If this function fails, you will be in no mode; meaning you may not access
// the display (i.e., call display/palette functions) even if you were in a mode
// before calling this function.  See rspKillVideoMode.
// Before this function is called, you may not call functions that manipulate
// the display.
//
//////////////////////////////////////////////////////////////////////////////
extern short rspSetVideoMode(	// Returns 0 if successfull, non-zero otherwise
	short	sDeviceDepth,			// Specify required device video depth here.
	short	sDeviceWidth,			// Specify required device resolution width here.
	short	sDeviceHeight,			// Specify required device resolution height here.
	short sWidth,					// Specify width of display area on screen.
	short sHeight,					// Specify height of display area on screen.
	short	sPages /*= 1*/,		// Specify number of video pages.  More than 1
										// indicates a page flipping scenario.
	short	sPixelDoubling	/*= FALSE*/)
										// TRUE indicates to set the video mode
										// to twice that indicated by sDeviceWidth,
										// sDeviceHeight and double the coordinate
										// system and blts.
										// FALSE indicates not to use this garbage.
	{
		TRACE("rspSetVideoMode(%i, %i, %i, %i, %i, %i, %i)\n", sDeviceDepth, sDeviceWidth, sDeviceHeight, sWidth, sHeight, sPages, sPixelDoubling);
        ASSERT(sDeviceDepth == 8);
        //ASSERT(sDeviceWidth == 0);
        //ASSERT(sDeviceHeight == 0);
        //ASSERT(sWidth == 640);
        ASSERT(sHeight == 480);

        for (size_t i = 0; i < 256; i++)
            apeApp[i].a = 0xFF;

        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

        if (sPixelDoubling)
        {
            fprintf(stderr, "STUBBED: pixel doubling? %s:%d\n", __FILE__, __LINE__);
            return -1;
        }

        FramebufferWidth = sWidth;
        FramebufferHeight = sHeight;

        mouse_grabbed = !rspCommandLine("nomousegrab");

        Uint32 flags = 0;
        if (!rspCommandLine("windowed"))
        {
            if ((!RequestedWidth) || (!RequestedHeight))
                flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
            else
                flags |= SDL_WINDOW_FULLSCREEN;
        }

        if (mouse_grabbed)
            flags |= SDL_WINDOW_INPUT_GRABBED;

#if PLATFORM_IOS
        flags |= SDL_WINDOW_BORDERLESS;   // don't show the status bar
#endif
        //TRACE("RequestedWidth %d   RequestedHeight %d\n",RequestedWidth,RequestedHeight);

        const char *title = sdlAppName ? sdlAppName : "";
        sdlWindow = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, RequestedWidth, RequestedHeight, flags);
        if (!sdlWindow)
        {
            char buf[128];
            SDL_snprintf(buf, sizeof (buf), "Couldn't create window: %s.", SDL_GetError());
            fprintf(stderr, "POSTAL: %s\n", buf);
            SDL_Quit();
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "POSTAL", buf, NULL);
            exit(1);
        }
		int w = 0, h = 0;
		SDL_GetWindowSize(sdlWindow, &w, &h);
		TRACE("SDL Window initialized at %ix%i\n", w, h);

        bool bRequestedRenderer = true;
        if (rspCommandLine("direct3d"))
            sdlRenderer = createRendererByName(sdlWindow, "direct3d");
        else if (rspCommandLine("opengl"))
            sdlRenderer = createRendererByName(sdlWindow, "opengl");
        else if (rspCommandLine("software"))
            sdlRenderer = createRendererByName(sdlWindow, "software");
        else
        {
            bRequestedRenderer = false;
            sdlRenderer = createRendererByName(sdlWindow, NULL);
        }

        if (!sdlRenderer)
        {
            char buf[128];
            SDL_snprintf(buf, sizeof (buf), "Couldn't create %s renderer: %s", bRequestedRenderer ? "requested" : "a", SDL_GetError());
            fprintf(stderr, "POSTAL: %s\n", buf);
            SDL_DestroyWindow(sdlWindow);
            sdlWindow = NULL;
            SDL_Quit();
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "POSTAL", buf, NULL);
            exit(1);
        }

        SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
        SDL_RenderClear(sdlRenderer);
        SDL_RenderPresent(sdlRenderer);
        SDL_RenderClear(sdlRenderer);
        SDL_RenderPresent(sdlRenderer);
        SDL_RenderClear(sdlRenderer);
        SDL_RenderPresent(sdlRenderer);
#ifndef MOBILE //Need to remove this for the mouse point to be in the correct place, Android And IOS
        SDL_RenderSetLogicalSize(sdlRenderer, FramebufferWidth, FramebufferHeight);
		TRACE("SDL Renderer set: %ix%i\n", FramebufferWidth, FramebufferHeight);
#endif
        sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, FramebufferWidth, FramebufferHeight);
        if (!sdlTexture)
        {
            char buf[128];
            SDL_snprintf(buf, sizeof (buf), "Couldn't create texture: %s", SDL_GetError());
            fprintf(stderr, "POSTAL: %s\n", buf);
            SDL_DestroyRenderer(sdlRenderer);
            sdlRenderer = NULL;
            SDL_DestroyWindow(sdlWindow);
            sdlWindow = NULL;
            SDL_Quit();
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "POSTAL", buf, NULL);
            exit(1);
        }

        TexturePointer = new Uint32[FramebufferWidth * FramebufferHeight];
        PalettedTexturePointer = new Uint8[FramebufferWidth * FramebufferHeight];
        SDL_memset(TexturePointer, '\0', FramebufferWidth * FramebufferHeight * sizeof (Uint32));
        SDL_memset(PalettedTexturePointer, '\0', FramebufferWidth * FramebufferHeight * sizeof (Uint8));
        SDL_UpdateTexture(sdlTexture, NULL, TexturePointer, FramebufferWidth * 4);

    	SDL_ShowCursor(0);
        //SDL_SetRelativeMouseMode(mouse_grabbed ? SDL_TRUE : SDL_FALSE);

        return 0;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Puts you in a state of not having display access.  After this function is
// called (similar to before rspSetVideoMode is called) you may not call
// functions that manipulate the display.  This is similar to the situation
// achieved if rspSetVideoMode fails.
//
//////////////////////////////////////////////////////////////////////////////
extern void rspKillVideoMode(void)
	{
    /* no-op ... SDL_Quit() will catch this. */
	}

//////////////////////////////////////////////////////////////////////////////
//
// Frees the memory (and, perhaps, structure(s)) associated with the memory
// stored in system RAM that is allocate rspSetVideoMode.  If you are not 
// using the system buffer (i.e., you are not calling rspLockVideoBuffer), 
// you can call this to free up some additional memory.  Calls to 
// rspLockVideoBuffer will fail after a call to this function without a 
// subsequent call to rspSetVideoMode.
//
//////////////////////////////////////////////////////////////////////////////
extern void rspKillVideoBuffer(void)
	{
    /* no-op ... SDL_Quit() will catch this. */
	}

//////////////////////////////////////////////////////////////////////////////
//
// Variation #1: Update the entire display from the buffer.  Includes
// pixel doubling as appropriate - see elsewhere for details.
//
//////////////////////////////////////////////////////////////////////////////

extern void rspUpdateDisplayRects(void)
{
    // no-op, just blast it all to the GPU.
}

extern void rspCacheDirtyRect(
	short sX,					// x coord of upper-left corner of area to update
	short sY,					// y coord of upper-left corner of area to update
	short sWidth,				// Width of area to update
	short sHeight)				// Height of area to update
{
}

extern void rspPresentFrame(void)
{
    if (!sdlWindow) return;

    // !!! FIXME: I imagine this is not fast. Maybe keep the dirty rect code at least?
    ASSERT(sizeof (apeApp[0]) == sizeof (Uint32));
    const Uint8 *src = PalettedTexturePointer;
    Uint32 *dst = TexturePointer;
    for (int y = 0; y < FramebufferHeight; y++)
    {
        for (int x = 0; x < FramebufferWidth; x++, src++, dst++)
            *dst = apeApp[*src].argb;
        }

    SDL_UpdateTexture(sdlTexture, NULL, TexturePointer, FramebufferWidth * 4);
    SDL_RenderClear(sdlRenderer);
    SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
    SDL_RenderPresent(sdlRenderer);  // off to the screen with you.

    static Uint32 lastframeticks = 0;
    const Uint32 now = SDL_GetTicks();

    if ((lastframeticks) && (lastframeticks <= now))
    {
        const Uint32 elapsed = (now - lastframeticks);
        if (elapsed <= 5)  // going WAY too fast, maybe OpenGL (and/or no vsync)?
            SDL_Delay(16 - elapsed);  // try to get closer to 60fps.
    }

    lastframeticks = now;

    #if 0
    static Uint32 ticks = 0;
    static Uint32 frames = 0;
    frames++;
    if ((now - ticks) > 5000)
    {
        if (ticks > 0)
            printf("fps: %f\n", (((double) frames) / ((double) (now - ticks))) * 1000.0);
        ticks = now;
        frames = 0;
    }
    #endif
}

extern void rspUpdateDisplay(void)
{
}

///////////////////////////////////////////////////////////////////////////////
//
// Maps calls to API that matches display type.
// See function comment in appropriate CPP (BGDisp/BXDisp).
//
///////////////////////////////////////////////////////////////////////////////
extern void rspUpdateDisplay(
	short sX,					// x coord of upper-left corner of area to update
	short sY,					// y coord of upper-left corner of area to update
	short sWidth,				// Width of area to update
	short sHeight)				// Height of area to update
{
}

///////////////////////////////////////////////////////////////////////////////
//
// Maps calls to API that matches display type.
// See function comment in appropriate CPP (BGDisp/BXDisp).
//
///////////////////////////////////////////////////////////////////////////////
extern short rspLockVideoPage(	// Returns 0 if screen memory could be locked.
											// Returns non-zero otherwise.
	void**	ppvMemory,				// Pointer to display memory returned here.
											// NULL returned if not supported.
	long*		plPitch)					// Pitch of display memory returned here.
	{
	/* no-op. */
	return 1;
	}

///////////////////////////////////////////////////////////////////////////////
//
// Maps calls to API that matches display type.
// See function comment in appropriate CPP (BGDisp/BXDisp).
//
///////////////////////////////////////////////////////////////////////////////
extern void rspUnlockVideoPage(void)	// Returns nothing.
	{
	/* no-op. */
	}

///////////////////////////////////////////////////////////////////////////////
//
// Maps calls to API that matches display type.
// See function comment in appropriate CPP (BGDisp/BXDisp).
//
///////////////////////////////////////////////////////////////////////////////
extern short rspLockVideoFlipPage(	// Returns 0 if flip screen memory could be 
											// locked.  Returns non-zero otherwise.
	void**	ppvMemory,				// Pointer to flip screen memory returned here.
											// NULL returned on failure.
	long*		plPitch)					// Pitch of flip screen memory returned here.
	{
	return -1;
	}

///////////////////////////////////////////////////////////////////////////////
//
// Maps calls to API that matches display type.
// See function comment in appropriate CPP (BGDisp/BXDisp).
//
///////////////////////////////////////////////////////////////////////////////
extern void rspUnlockVideoFlipPage(void)	// Returns nothing.
	{
	}

///////////////////////////////////////////////////////////////////////////////
//
// Maps calls to API that matches display type.
// See function comment in appropriate CPP (BGDisp/BXDisp).
//
///////////////////////////////////////////////////////////////////////////////
extern short rspLockVideoBuffer(	// Returns 0 if system buffer could be locked.
												// Returns non-zero otherwise.
	void**	ppvBuffer,					// Pointer to system buffer returned here.
												// NULL returned on failure.
	long*		plPitch)						// Pitch of system buffer returned here.
	{
    if (!sdlWindow)
        return -1;

    *ppvBuffer = PalettedTexturePointer;
    *plPitch = FramebufferWidth;

    return(0);
	}

///////////////////////////////////////////////////////////////////////////////
//
// Maps calls to API that matches display type.
// See function comment in appropriate CPP (BGDisp/BXDisp).
//
///////////////////////////////////////////////////////////////////////////////
extern void rspUnlockVideoBuffer(void)	// Returns nothing.
	{
	}

///////////////////////////////////////////////////////////////////////////////
//
// Maps calls to API that matches display type.
// See function comment in appropriate CPP (BGDisp/BXDisp).
//
///////////////////////////////////////////////////////////////////////////////
extern short rspAllowPageFlip(void)	// Returns 0 on success.
	{
	return 0;
	}

//////////////////////////////////////////////////////////////////////////////
//	External Palette module functions.
//////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
// Set several palette entries.  Separate pointers to each component
// combined with caller-specified increment for all pointers allows
// use of this function with any (non-packed) arrangement of RGB data.
// Hardware palette is not updated until UpdatePalette() is called.
//
///////////////////////////////////////////////////////////////////////////////
extern void rspSetPaletteEntries(
	short sStartIndex,			// Palette entry to start copying to (has no effect on source!)
	short sCount,					// Number of palette entries to do
	unsigned char* pucRed,		// Pointer to first red component to copy from
	unsigned char* pucGreen,	// Pointer to first green component to copy from
	unsigned char* pucBlue,		// Pointer to first blue component to copy from
	long lIncBytes)				// Number of bytes by which to increment pointers after each copy
	{
	// Set up destination pointers.
	UCHAR*	pucDstRed	= &(apeApp[sStartIndex].r);
	UCHAR*	pucDstGreen	= &(apeApp[sStartIndex].g);
	UCHAR*	pucDstBlue	= &(apeApp[sStartIndex].b);

	// Set up lock pointer.
	short*	psLock		= &(asPalEntryLocks[sStartIndex]);

	while (sCount-- > 0)
		{
		if (*psLock++ == 0)
			{
			*pucDstRed		= *pucRed;
			*pucDstGreen	= *pucGreen;
			*pucDstBlue		= *pucBlue;
			}

		// Increment source.
		pucRed			+= lIncBytes;
		pucGreen		+= lIncBytes;
		pucBlue			+= lIncBytes;

		// Increment destination.
		pucDstRed		+= sizeof(apeApp[0]);
		pucDstGreen		+= sizeof(apeApp[0]);
		pucDstBlue		+= sizeof(apeApp[0]);
		}
	}

///////////////////////////////////////////////////////////////////////////////
//
// Set palette entry.  Hardware palette is not updated until
// UpdatePalette() is called.
//
///////////////////////////////////////////////////////////////////////////////
void rspSetPaletteEntry(
	short sEntry,				// Palette entry (0 to 255)
	UCHAR ucRed,				// Red component (0 to 255)
	UCHAR ucGreen,				// Green component (0 to 255)
	UCHAR ucBlue)				// Blue component (0 to 255)
	{
	ASSERT(sEntry >= 0 && sEntry < 256);

	if (asPalEntryLocks[sEntry] == 0)
		{
		apeApp[sEntry].r	= ucRed;
		apeApp[sEntry].g	= ucGreen;
		apeApp[sEntry].b	= ucBlue;
		}
	}

///////////////////////////////////////////////////////////////////////////////
//
// Get palette entry.  This reflects the palette that may or may NOT have
// been set by calling rspUpdatePalette.
//
///////////////////////////////////////////////////////////////////////////////
void rspGetPaletteEntry(
	short sEntry,				// Palette entry (0 to 255)
	short* psRed,				// Red component (0 to 255) returned if not NULL.
	short* psGreen,			// Green component (0 to 255) returned if not NULL.
	short* psBlue)				// Blue component (0 to 255) returned if not NULL.
	{
	ASSERT(sEntry >= 0 && sEntry < 256);

	SET(psRed,		apeApp[sEntry].r);
	SET(psGreen,	apeApp[sEntry].g);
	SET(psBlue,		apeApp[sEntry].b);
	}

///////////////////////////////////////////////////////////////////////////////
//
// Get several palette entries.  Separate pointers to each component
// combined with caller-specified increment for all pointers allows
// use of this function with any (non-packed) arrangement of RGB data.
// This is the palette that may not necessarily be updated yet in the hardware
// (i.e., some may have set the palette and not called UpdatePalette()).
//
///////////////////////////////////////////////////////////////////////////////
extern void rspGetPaletteEntries(
	short sStartIndex,			// Palette entry to start copying from
	short sCount,					// Number of palette entries to do
	unsigned char* pucRed,		// Pointer to first red component to copy to
	unsigned char* pucGreen,	// Pointer to first green component to copy to
	unsigned char* pucBlue,		// Pointer to first blue component to copy to
	long lIncBytes)				// Number of bytes by which to increment pointers after each copy
	{
	// Set up source pointers.
	UCHAR*	pucSrcRed	= &(apeApp[sStartIndex].r);
	UCHAR*	pucSrcGreen	= &(apeApp[sStartIndex].g);
	UCHAR*	pucSrcBlue	= &(apeApp[sStartIndex].b);

	while (sCount-- > 0)
		{
		*pucRed		= *pucSrcRed;
		*pucGreen	= *pucSrcGreen;
		*pucBlue		= *pucSrcBlue;
		// Increment destination.
		pucRed			+= lIncBytes;
		pucGreen			+= lIncBytes;
		pucBlue			+= lIncBytes;

		// Increment source.
		pucSrcRed		+= sizeof(apeApp[0]);
		pucSrcGreen		+= sizeof(apeApp[0]);
		pucSrcBlue		+= sizeof(apeApp[0]);
		}
	}

///////////////////////////////////////////////////////////////////////////////
//
// Update hardware palette using information that has been set
// using SetPaletteEntry() and SetPaletteEntries().
// In the future, this may support optional VBLANK-synced updates.
//
///////////////////////////////////////////////////////////////////////////////
extern void rspUpdatePalette(void)
	{
    }
///////////////////////////////////////////////////////////////////////////////
//
// Set entries in the color map used to tweak values set via 
// rspSetPaletteEntries().  Those colors' values will be used as indices
// into this map when rspUpdatePalette() is called.  The resulting values
// will be updated to the hardware.
// rspGetPaletteEntries/Entry() will still return the original values set 
// (not mapped values).
// 
///////////////////////////////////////////////////////////////////////////////
extern void rspSetPaletteMaps(
	short sStartIndex,			// Map entry to start copying to (has no effect on source!)
	short sCount,					// Number of map entries to do
	unsigned char* pucRed,		// Pointer to first red component to copy from
	unsigned char* pucGreen,	// Pointer to first green component to copy from
	unsigned char* pucBlue,		// Pointer to first blue component to copy from
	long lIncBytes)				// Number of bytes by which to increment pointers after each copy
	{
	// Set up destination pointers.
	UCHAR*	pucDstRed	= &(au8MapRed[sStartIndex]);
	UCHAR*	pucDstGreen	= &(au8MapGreen[sStartIndex]);
	UCHAR*	pucDstBlue	= &(au8MapBlue[sStartIndex]);

	while (sCount-- > 0)
		{
		*pucDstRed++		= *pucRed;
		*pucDstGreen++		= *pucGreen;
		*pucDstBlue++		= *pucBlue;
		// Increment source.
		pucRed				+= lIncBytes;
		pucGreen				+= lIncBytes;
		pucBlue				+= lIncBytes;
		}
	}

///////////////////////////////////////////////////////////////////////////////
//
// Get entries in the color map used to tweak values set via 
// rspSetPaletteEntries().  Those colors' values will be used as indices
// into this map when rspUpdatePalette() is called.  The resulting values
// will be updated to the hardware.
// rspGetPaletteEntries/Entry() will still return the original values set 
// (not mapped values).
// 
///////////////////////////////////////////////////////////////////////////////
extern void rspGetPaletteMaps(
	short sStartIndex,			// Map entry to start copying from (has no effect on dest!)
	short sCount,					// Number of map entries to do
	unsigned char* pucRed,		// Pointer to first red component to copy to
	unsigned char* pucGreen,	// Pointer to first green component to copy to
	unsigned char* pucBlue,		// Pointer to first blue component to copy to
	long lIncBytes)				// Number of bytes by which to increment pointers after each copy
	{
	// Set up source pointers.
	UCHAR*	pucSrcRed	= &(au8MapRed[sStartIndex]);
	UCHAR*	pucSrcGreen	= &(au8MapGreen[sStartIndex]);
	UCHAR*	pucSrcBlue	= &(au8MapBlue[sStartIndex]);

	while (sCount-- > 0)
		{
		*pucRed		= *pucSrcRed++;
		*pucGreen	= *pucSrcGreen++;
		*pucBlue		= *pucSrcBlue++;
		// Increment destination.
		pucRed				+= lIncBytes;
		pucGreen				+= lIncBytes;
		pucBlue				+= lIncBytes;
		}
	}

///////////////////////////////////////////////////////////////////////////////
// Lock several palette entries.  Locking an entry keeps it from
// being updated by rspSetPaletteEntries() (until it is unlocked
// with rspUnlockPaletteEntries() ).
///////////////////////////////////////////////////////////////////////////////
extern void rspLockPaletteEntries(
	short	sStartIndex,			// Palette entry at which to start locking.
	short	sCount)					// Number of palette entries to lock.
	{
	// Set up iterator pointer.
	short*	psLock	= &(asPalEntryLocks[sStartIndex]);

	while (sCount-- > 0)
		{
		*psLock	= TRUE;
		}
	}

///////////////////////////////////////////////////////////////////////////////
// Unlock several palette entries previously locked by rspLockPaletteEntries().
///////////////////////////////////////////////////////////////////////////////
extern void rspUnlockPaletteEntries(
	short	sStartIndex,			// Palette entry at which to start locking.
	short	sCount)					// Number of palette entries to lock.
	{
	// Set up iterator pointer.
	short*	psLock	= &(asPalEntryLocks[sStartIndex]);

	while (sCount-- > 0)
		{
		*psLock	= FALSE;
		}

	// Never ever ever unlock these.
	asPalEntryLocks[0]	= TRUE;
	asPalEntryLocks[255]	= TRUE;
	}

///////////////////////////////////////////////////////////////////////////////
// Dyna schtuff.
///////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//	External Background module functions.
//////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
// Set a callback to be called when the application moves into the background.
//
///////////////////////////////////////////////////////////////////////////////
extern void rspSetBackgroundCallback(	// Returns nothing.
	void (BackgroundCall)(void))			// Callback when app processing becomes
													// background.  NULL to clear.
	{
        /* no-op. */
	}

///////////////////////////////////////////////////////////////////////////////
//
// Set a callback to be called when the application moves into the foreground.
//
///////////////////////////////////////////////////////////////////////////////
extern void rspSetForegroundCallback(	// Returns nothing.
	void (ForegroundCall)(void))			// Callback when app processing becomes
													// foreground.  NULL to clear.
	{
        /* no-op. */
	}

extern short rspIsBackground(void)			// Returns TRUE if in background, FALSE otherwise
    {
        extern bool GSDLAppIsActive;
        return (short) (!GSDLAppIsActive);
    }
//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
