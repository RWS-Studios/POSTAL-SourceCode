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
#include <stdio.h>
#include <stdarg.h>

#include "Blue.h"
#include "../cyan.h"

extern SDL_Window *sdlWindow;
extern SDL_Surface *sdlShadowSurface;
extern int sdlWindowWidth;
extern int sdlWindowHeight;

extern short rspMsgBox(	// Returns RSP_MB_RET_*.  See switch statement below.
	USHORT usFlags,		// MB_BUT/ICO_* flags specifying buttons and icons.
	char *pszTitle,		// Title for box.
	char *pszFrmt,			// Format for string.
	...)						// Various shit.
{
	char szOutput[4096];
	va_list varp;
	// Get pointer to the arguments.
	va_start(varp, pszFrmt);    
	// Compose string.
	SDL_vsnprintf(szOutput, sizeof (szOutput), pszFrmt, varp);
	// Done with var arguments.
	va_end(varp);

    SDL_MessageBoxData data;
    SDL_zero(data);

    switch (usFlags & RSP_MB_ICN_MASK)
    {
        case RSP_MB_ICN_STOP: data.flags |= SDL_MESSAGEBOX_ERROR; break;
        case RSP_MB_ICN_QUERY: data.flags |= SDL_MESSAGEBOX_INFORMATION; break;
        case RSP_MB_ICN_EXCLAIM: data.flags |= SDL_MESSAGEBOX_WARNING; break;
        case RSP_MB_ICN_INFO: data.flags |= SDL_MESSAGEBOX_INFORMATION; break;
        default: break;
    }

    data.window = sdlWindow;
    data.title = pszTitle;
    data.message = szOutput;

    SDL_MessageBoxButtonData buttons[3];
    SDL_zero(buttons);
    switch (usFlags & RSP_MB_BUT_MASK)
    {
        case RSP_MB_BUT_OK:
            data.numbuttons = 1;
            buttons[0].flags = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT |
                               SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT;
            buttons[0].buttonid = RSP_MB_RET_OK;
            buttons[0].text = "OK";
            break;

        case RSP_MB_BUT_OKCANCEL:
            data.numbuttons = 2;
            buttons[0].flags = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;
            buttons[0].buttonid = RSP_MB_RET_OK;
            buttons[0].text = "OK";
            buttons[1].flags = SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT;
            buttons[1].buttonid = RSP_MB_RET_CANCEL;
            buttons[1].text = "Cancel";
            break;

        case RSP_MB_BUT_ABORTRETRYIGNORE:
            data.numbuttons = 3;
            buttons[0].flags = SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT;
            buttons[0].buttonid = RSP_MB_RET_ABORT;
            buttons[0].text = "Abort";
            buttons[1].flags = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;
            buttons[1].buttonid = RSP_MB_RET_RETRY;
            buttons[1].text = "Retry";
            buttons[2].flags = 0;
            buttons[2].buttonid = RSP_MB_RET_IGNORE;
            buttons[2].text = "Ignore";
            break;

        case RSP_MB_BUT_YESNOCANCEL:
            data.numbuttons = 3;
            buttons[0].flags = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;
            buttons[0].buttonid = RSP_MB_RET_YES;
            buttons[0].text = "Yes";
            buttons[1].flags = 0;
            buttons[1].buttonid = RSP_MB_RET_NO;
            buttons[1].text = "No";
            buttons[2].flags = SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT;
            buttons[2].buttonid = RSP_MB_RET_CANCEL;
            buttons[2].text = "Cancel";
            break;

        case RSP_MB_BUT_YESNO:
            data.numbuttons = 2;
            buttons[0].flags = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;
            buttons[0].buttonid = RSP_MB_RET_YES;
            buttons[0].text = "Yes";
            buttons[1].flags = SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT;
            buttons[1].buttonid = RSP_MB_RET_NO;
            buttons[1].text = "No";
            break;

        case RSP_MB_BUT_RETRYCANCEL:
            data.numbuttons = 2;
            buttons[0].flags = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;
            buttons[0].buttonid = RSP_MB_RET_RETRY;
            buttons[0].text = "Retry";
            buttons[1].flags = SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT;
            buttons[1].buttonid = RSP_MB_RET_CANCEL;
            buttons[1].text = "Cancel";
            break;

        default: ASSERT(false); return -1;
    }

    data.buttons = buttons;

    int button = 0;
    const int rc = SDL_ShowMessageBox(&data, &button);
    return (rc == 0) ? button : -1;
}


extern short rspOpenBox(								// Returns 0 if successfull, non-zero otherwise
	const char* pszBoxTitle,							// In:  Title of box
	const char*	pszDefaultPath,						// In:  Default directory and file
	char* pszSelectedFile,								// Out: File that user selected
	short sSelectedFileBufSize,						// In:  Size of buffer pointed to by pszSelectedFile
	const char*	pszFilter /*= NULL*/)				// In:  Filename filter or NULL for none
{
    fprintf(stderr, "STUBBED: %s:%d\n", __FILE__, __LINE__);
    return -1;
}


extern short rspSaveBox(			// Returns 0 on success.
	const char* pszBoxTitle,				// In:  Title of box.
	const char*	pszDefFileName,			// In:  Default filename.
	char* pszChosenFileName,		// Out: User's choice.
	short sStrSize,					// In:  Amount of memory pointed to by pszChosenFileName.
	const char*	pszFilter /*= NULL*/)	// In:  If not NULL, '.' delimited extension based filename
											//	filter specification.  Ex: ".cpp.h.exe.lib" or "cpp.h.exe.lib"
											// Note: Cannot use '.' in filter.  Preceding '.' ignored.
{
    fprintf(stderr, "STUBBED: %s:%d\n", __FILE__, __LINE__);
    return -1;
}


extern void rspSetCursor(
	short sCursorID)						// In:  ID of built-in cursor (use RSP_CURSOR_* macros)
{
    fprintf(stderr, "STUBBED: %s:%d\n", __FILE__, __LINE__);
}

