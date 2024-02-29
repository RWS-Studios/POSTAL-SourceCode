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
// RSPiXBlue.cpp : implementation file
//

#include "stdafx.h"
#include "MFC_CNTL/RSPXBlue.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Jon's includes.
/////////////////////////////////////////////////////////////////////////////
#include "common/system.h"
#include "common/wmain.h"
#include "common/bdisplay.h"
#include "common/bpalette.h"
#include "common/wpalette.h"
#include "common/bluewin.h"

/////////////////////////////////////////////////////////////////////////////
// CRSPiXBlue

CRSPiXBlue::CRSPiXBlue()
	{
	m_pim	= NULL;
	}

CRSPiXBlue::~CRSPiXBlue()
	{
	}


BEGIN_MESSAGE_MAP(CRSPiXBlue, CStatic)
	//{{AFX_MSG_MAP(CRSPiXBlue)
	ON_WM_DESTROY()
	ON_WM_SETFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRSPiXBlue message handlers

///////////////////////////////////////////////////////////////////////////////
//
// Set the buffer Blue uses to draw and redraw the display.
//
///////////////////////////////////////////////////////////////////////////////
void CRSPiXBlue::SetDisplayImage(CImage* pim, short sFlip /*= FALSE*/)
	{
	ASSERT(pim != NULL);

	// Calculate DIB type width.
	long	lWinWidth	= pim->lPitch / ((long)pim->sDepth / 8L);
	
	Blu_SetDisplayBuf(pim->pData,
							lWinWidth, pim->lHeight, 
							pim->sDepth,
							sFlip);

	Blu_SetRedrawBuf(	pim->pData,
							lWinWidth, pim->lHeight,
							0, 0, 0, 0,
							pim->lWidth, pim->lHeight, pim->sDepth,
							sFlip);

	::MoveWindow(gsi.hWnd, 0, 0, pim->lWidth, pim->lHeight, TRUE);

	RECT	rcWindow;
	GetWindowRect(&rcWindow);
	ScreenToClient(&rcWindow);
	RECT	rcClient;
	GetClientRect(&rcClient);

	MoveWindow(0, 0,	pim->lWidth		- (rcClient.left	- rcWindow.left),
							pim->lHeight	- (rcClient.top	- rcWindow.top));

	m_pim	= pim;
	}

///////////////////////////////////////////////////////////////////////////////
//
// Set the palette Blue uses to draw and redraw the display.
//
///////////////////////////////////////////////////////////////////////////////
void CRSPiXBlue::SetDisplayPalette(CPal* ppal)
	{
	ASSERT(ppal != NULL);

	SetFocus();

	switch (ppal->ulType)
		{
		case NO_PALETTE:
			break;
		case PDIB:
		case PSYS:
			Blu_SetPalette((RGBQUAD*)ppal->pData, 
								ppal->sStartIndex, 
								ppal->sNumEntries);
			break;
		case P555:
			Blu_SetPalette((PRGBT5)ppal->pData,
								ppal->sStartIndex, 
								ppal->sNumEntries);
			break;
		case P888:
		case PFLX:
			Blu_SetPalette((PRGBT8)ppal->pData,
								ppal->sStartIndex, 
								ppal->sNumEntries);
			break;
		case P565:
			TRACE("SetDisplayPalette(): Unsupported palette type.\n");
			break;
		}
	}

///////////////////////////////////////////////////////////////////////////////
//
// On initial subclass, create the RSPiX window to the size of the parent.
//
///////////////////////////////////////////////////////////////////////////////
void CRSPiXBlue::PreSubclassWindow() 
	{
	short	sError	= 0;	// Assume success.

	// Initialize Blue . . . 
	if (Blu_Init(AfxGetApp()->m_hInstance, m_hWnd) == 0)
		{
		RECT	rcClient;
		GetClientRect(&rcClient);

		// Create a display that fills this window completely . . .
		if (Blu_CreateDisplay(	rcClient.right, rcClient.bottom,
										(short)Blu_GetDisplayInfo(DI_MONITOR_COLORDEPTH))
			== 0)
			{
			// Success.
			m_wndRSPiX.Attach(gsi.hWnd);

			m_wndPal.Attach(Blu_GetPaletteWindow());

			if (m_wndPal.m_hWnd != NULL)
				{
				m_wndPal.SetParent(this);
				}
			}
		else
			{
			TRACE("OnCreate(): Blu_CreateDisplay() failed.\n");
			sError = -2;
			}

		// If we fail after initing Blue . . .
		if (sError != 0)
			{
			Blu_Kill();
			}
		}
	else
		{
		TRACE("OnCreate(): Blu_Init() failed.\n");
		sError = -1;
		}
	
	CStatic::PreSubclassWindow();
	}

///////////////////////////////////////////////////////////////////////////////
//
// When the parent is destroyed, kill Blue.
//
///////////////////////////////////////////////////////////////////////////////
void CRSPiXBlue::OnDestroy() 
	{
	m_wndPal.Detach();

	m_wndRSPiX.Detach();

	Blu_Kill();

	CStatic::OnDestroy();
	}

///////////////////////////////////////////////////////////////////////////////
//
// Redraw the RSPiX Blue window.
//
///////////////////////////////////////////////////////////////////////////////
void CRSPiXBlue::Redraw(void)
	{
	ASSERT(m_pim != NULL);

	Blu_UpdateDisplay(0, 0, 0, 0, m_pim->lWidth, m_pim->lHeight);
	}


///////////////////////////////////////////////////////////////////////////////
//
// Pass focus to RSPiX.
//
///////////////////////////////////////////////////////////////////////////////
void CRSPiXBlue::OnSetFocus(CWnd* pOldWnd) 
	{
	CStatic::OnSetFocus(pOldWnd);
	
	if (gsi.hWnd != NULL)
		{
		::SetFocus(gsi.hWnd);
		}
	}

///////////////////////////////////////////////////////////////////////////////
//
// Pass through to RSPiX.
//
///////////////////////////////////////////////////////////////////////////////
#if 0
void CRSPiXBlue::OnPaletteChanged(CWnd* pFocusWnd) 
	{
	CStatic::OnPaletteChanged(pFocusWnd);
	
	if (gsi.hWnd != NULL)
		{
		::SendMessage(	gsi.hWnd, WM_PALETTECHANGED, 
							(WPARAM)pFocusWnd->GetSafeHwnd(), 0L);
		}
	}

///////////////////////////////////////////////////////////////////////////////
//
// Pass through to RSPiX.
//
///////////////////////////////////////////////////////////////////////////////
BOOL CRSPiXBlue::OnQueryNewPalette() 
	{
	BOOL	bRes	= TRUE;

	if (gsi.hWnd != NULL)
		{
		bRes	= ::SendMessage(	gsi.hWnd, WM_QUERYNEWPALETTE, 
										0L, 0L);
		}
	
	// Bah!  Who cares what the static control returns.  
	CStatic::OnQueryNewPalette();

	return bRes;
	}
#endif

///////////////////////////////////////////////////////////////////////////////
//
// Returns a CWnd* to the palette window.  Returns NULL if no window.
//
///////////////////////////////////////////////////////////////////////////////
CWnd* CRSPiXBlue::GetPaletteWindow(void)
	{
	return (m_wndPal.m_hWnd == NULL) ? NULL : &m_wndPal;
	}

///////////////////////////////////////////////////////////////////////////////
//
// Returns a CWnd* to the RSPiX window.
//
///////////////////////////////////////////////////////////////////////////////
CWnd*	CRSPiXBlue::GetBlueWindow()
	{
	return &m_wndRSPiX;
	}

///////////////////////////////////////////////////////////////////////////////
//
// Passes messages vital to RSPiX on to the actual RSPiX window.
//
///////////////////////////////////////////////////////////////////////////////
LRESULT CRSPiXBlue::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
	{
	switch (message)
		{
		/////////////////////////////////////////////////////////////////////////
		// These messages should be passed to the Blue window.
		/////////////////////////////////////////////////////////////////////////

		//////////////////////////// Window Oriented ////////////////////////////
		case WM_ACTIVATEAPP:
		//////////////////////////// Mouse Oriented /////////////////////////////
//		case WM_LBUTTONDOWN:	// Mouse messages should go straight there anyways.
//		case WM_MBUTTONDOWN:
//		case WM_RBUTTONDOWN:
//		case WM_MOUSEMOVE:
//		case WM_LBUTTONUP:
//		case WM_MBUTTONUP:
//		case WM_RBUTTONUP:
		//////////////////////////// Key Oriented ///////////////////////////////
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_CHAR:
		//////////////////////////// Palette Oriented ///////////////////////////
		case WM_QUERYNEWPALETTE:
		case WM_PALETTECHANGED:
			if (gsi.hWnd != NULL)
				{
				::SendMessage(gsi.hWnd, message, wParam, lParam);
				}
			break;
		}
	
	return CStatic::WindowProc(message, wParam, lParam);
	}
