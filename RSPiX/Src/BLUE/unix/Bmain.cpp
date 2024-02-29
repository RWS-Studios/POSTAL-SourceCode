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
//	bmain.cpp
// 
// History: 06/03/2004  RCG added.
//
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
//
// This module was inspired by "RSPiX - The Making of an Engine (RAINBOW
// DESCRIPTION)" by Jeff Diamond, 4/19/95.  Please see that document for
// further details.
//
//////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifndef WIN32
#include <unistd.h>
#include <sys/param.h>
#endif

#include <time.h>

// Blue //////////////////////////////////////////////////////////////////////
#include "SDL.h"
#include "BLUE/Blue.h"

SDL_Window *sdlWindow = NULL;
SDL_Surface *sdlShadowSurface = NULL;

bool mouse_grabbed = false;
static short ms_sQuit = 0;

//////////////////////////////////////////////////////////////////////////////
// 
//	Call this to init the blue library.  Calls init in the various blue modules
// that needing an init call.
//
// Returns 0 on success.
// 
//////////////////////////////////////////////////////////////////////////////

extern void Disp_Init(void);
extern void Key_Init(void);
extern void Joy_Init(void);

short rspInitBlue(void)
	{
	short	sRes	= 0;	// Assume success.

#ifdef BETAEXPIRE
    if ( time(NULL) > (BETAEXPIRE + 30 * 24 * 60 * 60) )
    {
        fprintf(stderr, "This beta has expired.\n\n");
        fflush(stderr);
        while (1) _exit(42);
    }
#endif

	if (SDL_Init(SDL_INIT_VIDEO) == -1)
	{
		TRACE("SDL_Init() failed: %s.\n", SDL_GetError());
		sRes = -1;
	}

	Disp_Init();
    Key_Init();
    Joy_Init();

	return sRes;
	}
		

//////////////////////////////////////////////////////////////////////////////
// 
//	Call this to kill the blue library (IF you called Blu_Init).
//
// Returns nothing.
// 
//////////////////////////////////////////////////////////////////////////////
void rspKillBlue(void)
	{
	    // Kill display module.
    	SDL_Quit();
	}

//////////////////////////////////////////////////////////////////////////////
// 
// Does tasks critical to Windows:
//		- Services our Windows message queue.
// Returns nothing.
// 
//////////////////////////////////////////////////////////////////////////////

extern void rspSetQuitStatus(short sQuitStatus);

extern void Mouse_Event(SDL_Event *event);
extern void Key_Event(SDL_Event *event);

bool GSDLAppIsActive = true;

extern void rspDoSystem(void)										// Returns nothing.
	{
        rspPresentFrame();

        #if WITH_STEAMWORKS
        extern void RunSteamworksUpkeep();
        RunSteamworksUpkeep();
        #endif

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                //case SDL_MOUSEMOTION:
                //case SDL_JOYBALLMOTION:
                case SDL_MOUSEBUTTONDOWN:
                case SDL_MOUSEBUTTONUP:
				case SDL_MOUSEWHEEL:
                    Mouse_Event(&event);
                    break;

                case SDL_KEYDOWN:
                case SDL_KEYUP:
                    Key_Event(&event);
                    break;

//                case SDL_ACTIVEEVENT:
//                    if (event.active.state & SDL_APPINPUTFOCUS)
//                        GSDLAppIsActive = (event.active.gain != 0);
//                    break;

                case SDL_QUIT:
                    rspSetQuitStatus(1);
                    break;
            }
        }
	}

//////////////////////////////////////////////////////////////////////////////
// 
// Sets the current mode with which the rspDoSystem cooperates with the OS.
// 
//////////////////////////////////////////////////////////////////////////////
extern void rspSetDoSystemMode(	// Returns nothing.
	short sCooperativeLevel)		// In:  One of the RSP_DOSYSTEM_* macros 
											// defining what level of cooperation to use.
	{
        /* no-op */
	}

////////////////////////////////////////////////////////////////////////////////
//
// Get system-specific quit status.
//
////////////////////////////////////////////////////////////////////////////////
extern short rspGetQuitStatus(void)				// Returns TRUE if quit detected, FALSE otherwise
	{
	return ms_sQuit;
	}


////////////////////////////////////////////////////////////////////////////////
//
// Set system-specific quit status.
//
// This allows the app to either clear the status to 0 or to set it to whatever
// other value that app might like.
//
////////////////////////////////////////////////////////////////////////////////
extern void rspSetQuitStatus(
	short sStatus)										// In:  New status
	{
	ms_sQuit = sStatus;
	}

extern int _argc;
extern char **_argv;
extern int rspCommandLine(const char *cmd)
{
    for (int i = 1; i < _argc; i++)
    {
        const char *arg = _argv[i];
        if (*arg == '-') arg++;
        if (*arg == '-') arg++;

        if (strcasecmp(cmd, arg) == 0)
            return i;
    }

    return 0;
}

extern void rspPlatformInit(void)
{
#if PLATFORM_MACOSX
    // MacOS X has this concept of "Application Bundles" which makes the whole
    //  install tree look like a single icon in the Finder, which, when
    //  launched, loads the game binary. They, however, totally fuck up your
    //  game's filesystem...best thing to do is see if the binary is running
    //  in a dir that appears to be embedded in an Application Bundle and
    //  chdir to the game's System dir...  --ryan.
    //
    //  (FIXME: There _are_ Carbon APIs to determine this via process info...)
    const char *argv0 = _argv[0];
    char buf[MAXPATHLEN];
    if ((argv0 != NULL) && (strchr(argv0, '/') != NULL)) // no path specifed?
        strncpy(buf, argv0, sizeof (buf));
    else
    {
        // From the Finder, current working directory is "/", which sucks.
        if ((getcwd(buf, sizeof (buf)) == NULL) || (strcmp(buf, "/") == 0))
            return;  // hail mary...probably fail elsewhere.
    }

    buf[sizeof (buf) - 1] = '\0';  // null terminate, just in case.

    char realbuf[MAXPATHLEN];
    if (realpath(buf, realbuf) == NULL)
        return;

    char *ptr = strstr(realbuf, "/Contents/MacOS/");
    if (ptr != NULL)
        *ptr = '\0';  // chop off bundle bin dirs...
    else
    {
        // not an application bundle, but might be a symlink to elsewhere,
        //  so chdir to there, just in case.
        ptr = strrchr(realbuf, '/');  // chop off binary name.
        if (ptr != NULL)
            *ptr = '\0';
    }

    chdir(realbuf);  // go there.
#endif
}


//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
