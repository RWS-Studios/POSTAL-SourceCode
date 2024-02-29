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
// Sample/Test application for Video Library.
///////////////////////////////////////////////////////////////////////////////

#include "System.h"

#include "Blue.h"

#include "video.h"

#include "hot.h"

#include "wdisplay.h"

#define VIDEO_FILE	"\\\\METRO\\PROJECTS\\MUPPETS\\BEAKER\\TEMPASSETS\\LOSER1.AVI"
#define CURSOR_FILE	"\\\\METRO\\PROJECTS\\MUPPETS\\BEAKER\\TEMPASSETS\\HOTPTR.CUR"

///////////////////////////////////////////////////////////////////////////////
//
// Hooks Blue's Display's Window Proc.  Return non-zero to have Blue return
// your *plResult.
//
///////////////////////////////////////////////////////////////////////////////
short WinProcHook(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam, 
						LRESULT* plResult)
	{
	short	sRes	= 0; // Assume normal processing.

	switch (uiMsg)
		{
		case WM_SETCURSOR:
//			TRACE("WinProcHook(): WM_SETCURSOR.\n");
//			sRes = TRUE;
//			*plResult = TRUE;
			break;
		}

	return sRes;
	}

///////////////////////////////////////////////////////////////////////////////
//
//	Waits for a mouse click (down, then up) on sButton { MOUSE_BUTTON_LEFT,
// MOUSE_BUTTON_RIGHT }.
//
///////////////////////////////////////////////////////////////////////////////
static void WaitClick(short sButton)
	{
	while (Blu_GetMouseButton(sButton) == MOUSE_BUTTON_UP)
		{
		Blu_System();
		}
	
	while (Blu_GetMouseButton(sButton) == MOUSE_BUTTON_DOWN)
		{
		Blu_System();
		}
	}

///////////////////////////////////////////////////////////////////////////////
//
// Attempt to start or stop a video.  
// Returns 0 on success.
//
///////////////////////////////////////////////////////////////////////////////
#define MCI_TIME_OUT	500
#define MCI_RETRIES	3
static short ToggleVideo(CVideo* pvideo)
	{
	short	sRes	= 0;	// Assume success.

	Blu_HookWinProc(WinProcHook);

	if (pvideo->IsPlaying() == TRUE)
		{
		pvideo->Stop();

		if (pvideo->IsPlaying() == TRUE)
			{
			TRACE("ToggleVideo(): Video did not stop yet.\n");
			sRes = -1;
			}
		else
			{
			pvideo->Close();
			}
		}
	else
		{
		if (pvideo->Open(VIDEO_FILE, 0, 0, 1, 1, 0) == VIDEO_SUCCESS)
			{
			short	sRetries;
			long	lEndTime = 0L;
			for (	sRetries = 0;
					sRetries < MCI_RETRIES && sRes == 0 && pvideo->IsPlaying() == FALSE;
				 )
				{
				if (lEndTime < Blu_GetTime())
					{
					if (++sRetries == MCI_RETRIES)
						{
						TRACE("ToggleVideo(): Exceeded max retries.\n");
						}
					else
						{
						if (pvideo->Play() == VIDEO_SUCCESS)
							{
							// Set next time.
							lEndTime = Blu_GetTime() + MCI_TIME_OUT;
							}
						else
							{
							TRACE("ToggleVideo(): Play returned error.\n");
							sRes = -2;
							}
						}
					}

				Blu_System();
				}

			if (pvideo->IsPlaying() == TRUE)
				{
				TRACE("ToggleVideo(): Took %d tries to start.\n", sRetries);
				}
			}
		else
			{
			Blu_MsgBox(MB_ICN_STOP | MB_BUT_OK, "Sux!", "Unable to open <%s>.", VIDEO_FILE);
			}
		}

	return sRes;
	}

///////////////////////////////////////////////////////////////////////////////
//
//	Test video library functionality.
//
///////////////////////////////////////////////////////////////////////////////
short AppMain(void)
	{
	CVideo video;

	if (Blu_CreateDisplay(640, 400, (short)Blu_GetDisplayInfo(DI_MONITOR_COLORDEPTH))
		== 0)
		{
		short sButtonDn = FALSE;
		CHot	hot;

		if (hot.Create(0, 0, 
							Blu_GetDisplayInfo(DI_MONITOR_WIDTH), Blu_GetDisplayInfo(DI_MONITOR_HEIGHT),
							CURSOR_FILE) == 0)
			{
			hot.SetActive(TRUE);
			}
		else
			{
			TRACE("AppMain(): Unable to create hotbox.\n");
			}

		while (Blu_GetMouseButton(MOUSE_BUTTON_RIGHT) == MOUSE_BUTTON_UP)
			{
			if (Blu_GetMouseButton(MOUSE_BUTTON_LEFT) == MOUSE_BUTTON_DOWN)
				{
				sButtonDn = TRUE;
				}
			else
				{
				if (sButtonDn == TRUE)
					{
					ToggleVideo(&video);
					sButtonDn = FALSE;
					}
				}

			Blu_System();
			}
		
		while (Blu_GetMouseButton(MOUSE_BUTTON_RIGHT) == MOUSE_BUTTON_DOWN)
			{
			Blu_System();
			}
		}
	else
		{
		Blu_MsgBox(MB_ICN_INFO | MB_BUT_OK, "Sux!", "Unable to create display.");
		}

	return 0;
	}

///////////////////////////////////////////////////////////////////////////////
// EOF.
///////////////////////////////////////////////////////////////////////////////
