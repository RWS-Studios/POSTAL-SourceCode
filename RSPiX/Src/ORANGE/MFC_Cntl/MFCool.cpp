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
// MFCool.CPP
//
// 04/10/96	JMI	Started. 
//
///////////////////////////////////////////////////////////////////////////////
//
// Cool stuff for MFC.
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Includes.
///////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "MFC_CNTL/MFCool.h"

///////////////////////////////////////////////////////////////////////////////
// Internal functions.
///////////////////////////////////////////////////////////////////////////////
static BOOL CALLBACK EnumChildIconizeProc(HWND hWnd, long)
	{
	BOOL	bRes	= TRUE;	// Assume we will continue enumeration.

	// Get class name . . .
	char	szName[256];
	if (GetClassName(hWnd, szName, sizeof(szName)) > 0)
		{
		// If it is a button . . .
		if (stricmp(szName, "BUTTON") == 0)
			{
			// If it is an icon button . . .
			if ((GetWindowLong(hWnd, GWL_STYLE) & BS_ICON) > 0)
				{
				// Get the button text . . .
				if (GetWindowText(hWnd, szName, sizeof(szName)) > 0)
					{
					// Get the icon . . .
					HICON	hIcon	= LoadIcon(AfxGetResourceHandle(), szName);
					if (hIcon != NULL)
						{
						// Set the icon.
						SendMessage(hWnd, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hIcon);
						}
					else
						{
						TRACE("EnumChildIconizeProc(): LoadIcon failed.\n");
						}
					}
				else
					{
					TRACE("EnumChildIconizeProc(): GetWindowText failed.\n");
					}
				}
			}
		}
	else
		{
		TRACE("EnumChildIconizeProc(): Unable to get class name.\n");
		}

	return bRes;
	}

///////////////////////////////////////////////////////////////////////////////
// External functions.
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
// Associates all icon style buttons that are children of pdlg with the icon
// specified by the window text of that button.
// Returns 0 on success.
//
///////////////////////////////////////////////////////////////////////////////
extern short Iconize(CDialog* pdlg)
	{
	short	sRes	= 0;	// Assume success.

	if (EnumChildWindows(pdlg->GetSafeHwnd(), EnumChildIconizeProc, 0L) != FALSE)
		{
		// Success.
		}
	else
		{
		TRACE("Iconize(): EnumChildWindows failed.\n");
		sRes = -1;
		}

	return sRes;
	}

///////////////////////////////////////////////////////////////////////////////
//
// Store position of this window in the module's INI.
// Returns 0 on success.
//
///////////////////////////////////////////////////////////////////////////////
extern short StorePosition(CWnd* pwnd)
	{
	short	sRes	= 0;	// Assume success.

	// Construct section name.
	char	szSection[512];
	pwnd->GetWindowText(szSection, sizeof(szSection));
	strcat(szSection, " Position");

	RECT	rcWindow;
	pwnd->GetWindowRect(&rcWindow);

	AfxGetApp()->WriteProfileInt(szSection, "left",		rcWindow.left);
	AfxGetApp()->WriteProfileInt(szSection, "top",		rcWindow.top);
	AfxGetApp()->WriteProfileInt(szSection, "right",	rcWindow.right);
	AfxGetApp()->WriteProfileInt(szSection, "bottom",	rcWindow.bottom);

	return sRes;
	}

///////////////////////////////////////////////////////////////////////////////
//
// Restore position of this window from the module's INI.  No change if no 
// settings stored.
// Returns 0 on success.
//
///////////////////////////////////////////////////////////////////////////////
extern short RestorePosition(CWnd* pwnd)
	{
	short	sRes	= 0;	// Assume success.

	// Construct section name.
	char	szSection[512];
	pwnd->GetWindowText(szSection, sizeof(szSection));
	strcat(szSection, " Position");

	RECT	rcWindow;
	pwnd->GetWindowRect(&rcWindow);

	rcWindow.left		= AfxGetApp()->GetProfileInt(szSection, "left",		rcWindow.left);
	rcWindow.top		= AfxGetApp()->GetProfileInt(szSection, "top",		rcWindow.top);
	rcWindow.right		= AfxGetApp()->GetProfileInt(szSection, "right",	rcWindow.right);
	rcWindow.bottom	= AfxGetApp()->GetProfileInt(szSection, "bottom",	rcWindow.bottom);

	if (rcWindow.top < 0)
		{
		rcWindow.bottom	-= rcWindow.top;
		rcWindow.top		= 0;
		}

	if (rcWindow.left < 0)
		{
		rcWindow.right	-= rcWindow.left;
		rcWindow.left	= 0;
		}

	CDC*	pdc	= pwnd->GetDC();
	if (pdc != NULL)
		{
		long	lScrWidth	= pdc->GetDeviceCaps(HORZRES);
		long	lScrHeight	= pdc->GetDeviceCaps(VERTRES);
		
		if (rcWindow.bottom > lScrHeight)
			{
			rcWindow.top		-= (rcWindow.bottom - lScrHeight);
			rcWindow.bottom	-= (rcWindow.bottom - lScrHeight);
			}

		if (rcWindow.right > lScrWidth)
			{
			rcWindow.left		-= (rcWindow.right - lScrWidth);
			rcWindow.right		-= (rcWindow.right - lScrWidth);
			}

		pwnd->ReleaseDC(pdc);
		}
	else
		{
		TRACE("RestorePosition(): Unable to get CDC.  Cannot bound window by "
				"desktop width & height.\n");
		}
		
		

	pwnd->MoveWindow(	rcWindow.left, rcWindow.top,
							rcWindow.right		- rcWindow.left,
							rcWindow.bottom	- rcWindow.top);

	return sRes;
	}

///////////////////////////////////////////////////////////////////////////////
// EOF
///////////////////////////////////////////////////////////////////////////////
