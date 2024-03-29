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
// RSPiXBlue.h : header file
//
#ifndef RSPIXBLUE_H
#define RSPIXBLUE_H

///////////////////////////////////////////////////////////////////////////////
// Macros.
///////////////////////////////////////////////////////////////////////////////
#define DllExport	__declspec( dllexport )
#define DllImport	__declspec( dllimport )

#ifdef _WINDLL
	#define DLL2EXE	DllExport
	#define EXE2DLL	DllImport
#else
	#define DLL2EXE	DllImport
	#define EXE2DLL	DllExport
#endif	// _WINDLL

///////////////////////////////////////////////////////////////////////////////
// Blue Includes.
///////////////////////////////////////////////////////////////////////////////
#include "common/system.h"

#include "common/bluewin.h"

#include "common/bcritic.h"
#include "common/bdialog.h"
#include "common/bdisplay.h"
#include "common/bjoy.h"
#include "common/bkey.h"
#include "common/bmain.h"
#include "common/bmouse.h"
#include "common/bpalette.h"
#include "common/wpalette.h"
#include "common/bparms.h"
#include "common/bsound.h"
#include "common/btime.h"

///////////////////////////////////////////////////////////////////////////////
// Green Includes.
///////////////////////////////////////////////////////////////////////////////
#include "image/image.h"

/////////////////////////////////////////////////////////////////////////////
// CRSPiXBlue window

class EXE2DLL CRSPiXBlue : public CStatic
	{
	// Construction
	public:
		CWnd m_wndRSPiX;
		CWnd m_wndPal;
		CImage* m_pim;
		void Redraw(void);
		CRSPiXBlue();

	// Attributes
	public:

	// Operations
	public:
		////////////////////////////////////////////////////////////////////////////
		// Extensions/Interfaces.
		////////////////////////////////////////////////////////////////////////////

		// Set the buffer Blue uses to draw and redraw the display.
		void SetDisplayImage(CImage* pim, int16_t sFlip = FALSE);

		// Set the palette Blue uses to draw and redraw the display.
		void SetDisplayPalette(CPal* ppal);


		////////////////////////////////////////////////////////////////////////////
		// Indirection to Blue calls in the proverbial EXE.
		// NOTE:  These are not documented as the WinBlue version could change w/o 
		// this file being updated.  Please refer to the b*.h or b*.cpp file for
		// details.
		////////////////////////////////////////////////////////////////////////////
		// BCRITIC.H
		int16_t Blu_RemoveCritical(CRITICALL cc)	
			{ return ::Blu_RemoveCritical(cc); }
		int16_t Blu_AddCritical(CRITICALL cc, uint32_t ulUser)
			{ return ::Blu_AddCritical(cc, ulUser); }
		// BDEBUG.H
		#undef TRACE	// We will use MFC's TRACE
		#define TRACE	::AfxTrace("%s(%d) :", __FILE__, __LINE__),::AfxTrace
		#undef STRACE
		#define STRACE	::AfxTrace
		// BDIALOG.H
		int16_t Blu_MsgBox(uint16_t usFlags, char *pszTitle, char *pszFrmt, ...)
			{
			char	szOutput[16384];
			va_list varp;
			// Get pointer to the arguments.
			va_start(varp, pszFrmt);    
			// Compose string.
			vsprintf(szOutput, pszFrmt, varp);
			// Done with var arguments.
			va_end(varp);
			return ::Blu_MsgBox(usFlags, pszTitle, szOutput);
			}
		int16_t Blu_OpenBox(char* pszBoxTitle, char *pszFileName, int16_t sStrSize)
			{ return ::Blu_OpenBox(pszBoxTitle, pszFileName, sStrSize); }
		int16_t Blu_MultiOpenBox(	char* pszBoxTitle, 		
										char* pszFileNameMemory,
										int32_t	lNumBytes,			
										char **ppszFileNames, 	
										int16_t sMaxPtrs,			
										int16_t* psNumFiles			
										)
			{
			return ::Blu_MultiOpenBox(	pszBoxTitle, pszFileNameMemory,
												lNumBytes, ppszFileNames,
												sMaxPtrs, psNumFiles);
			}
		// BDISPLAY.H
		int16_t Blu_CreateDisplay(int32_t lWidth, int32_t lHeight, int16_t sColorDepth)
			{ return ::Blu_CreateDisplay(lWidth, lHeight, sColorDepth); }
		void Blu_SetRedrawBuf(	void* pvBuf, 				
										int32_t lBufW, int32_t lBufH,	
										int32_t sx, int32_t sy,			
										int32_t dx, int32_t dy, 		
										int32_t lBltW, int32_t lBltH,	
										int16_t sColorDepth,		
										int16_t sVertFlip = FALSE)
			{
			::Blu_SetRedrawBuf(	pvBuf, lBufW, lBufH, sx, sy, dx, dy,
										lBltW, lBltH, sColorDepth, sVertFlip); 
			}
		void Blu_SetDisplayBuf(	void* pvBuf, int32_t lWidth, int32_t lHeight,
										int16_t sColorDepth, int16_t sVertFlip = FALSE)
			{ ::Blu_SetDisplayBuf(pvBuf, lWidth, lHeight, sColorDepth, sVertFlip); }
		U16*	Blu_GetPaletteTranslation(void)
			{ return ::Blu_GetPaletteTranslation(); }
		int16_t Blu_UpdateDisplay(int32_t sx, int32_t sy, 
										int32_t dx, int32_t dy, 
										int32_t w, int32_t h)
			{ return ::Blu_UpdateDisplay(sx, sy, dx, dy, w, h); }
		int16_t Blu_SetWindowControls(int16_t sControls)
			{ return ::Blu_SetWindowControls(sControls); }
		int32_t Blu_GetDisplayInfo(uint16_t usInfo)
			{ return ::Blu_GetDisplayInfo(usInfo); }
		int16_t Blu_SetWindowTitle(char *pszTitleNameText)
			{ return ::Blu_SetWindowTitle(pszTitleNameText); }
		int16_t Blu_ValidateRect(int32_t lX, int32_t lY, int32_t lW, int32_t lH)
			{ return ::Blu_ValidateRect(lX, lY, lW, lH); }
		// BJOY.H
		int16_t Blu_UpdateJoy(int16_t sJoy)
			{ return ::Blu_UpdateJoy(sJoy); }
		void Blu_GetJoyPos(int16_t sJoy, int32_t *px, int32_t *py, int32_t *pz)
			{ ::Blu_GetJoyPos(sJoy, px, py, pz); }
		void Blu_GetJoyPrevPos(int16_t sJoy, int32_t *px, int32_t *py, int32_t *pz)
			{ ::Blu_GetJoyPrevPos(sJoy, px, py, pz); }
		uint16_t Blu_GetJoyState(int16_t sJoy)
			{ return ::Blu_GetJoyState(sJoy); }
		void Blu_GetJoyState(int16_t sJoy, PJOYSTATE pjs)
			{ ::Blu_GetJoyState(sJoy, pjs); }
		uint16_t Blu_GetJoyPrevState(int16_t sJoy)
			{ return ::Blu_GetJoyPrevState(sJoy); }
		void Blu_GetJoyPrevState(int16_t sJoy, PJOYSTATE pjs)
			{ ::Blu_GetJoyPrevState(sJoy, pjs); }
		// BKEY.H
		int16_t Blu_GetKeyboard(PKEYBOARD pkb)
			{ return ::Blu_GetKeyboard(pkb); }
		// BMAIN.H
		void Blu_System(void)
			{ ::Blu_System(); }
		void Blu_DispatchEvents(int16_t sDispatch)
			{ ::Blu_DispatchEvents(sDispatch); }
		// BMOUSE.H
		int16_t Blu_GetMousePos(int32_t* px, int32_t* py)
			{ return ::Blu_GetMousePos(px, py); }
		int16_t Blu_SetMousePos(int32_t x, int32_t y)
			{ return ::Blu_SetMousePos(x, y); }
		int16_t Blu_GetMouseButton(int16_t sButton)
			{ return ::Blu_GetMouseButton(sButton); }
		// BPALETTE.H
		int16_t Blu_SetPalette(PRGBT8 prgbt8, int16_t sIndex, int16_t sNumEntries)
			{ return ::Blu_SetPalette(prgbt8, sIndex, sNumEntries); }
		int16_t Blu_SetPalette(PRGBQ8 prgbq8, int16_t sIndex, int16_t sNumEntries)
			{ return ::Blu_SetPalette(prgbq8, sIndex, sNumEntries); }
		int16_t Blu_SetPalette(PRGBT5 prgbt5, int16_t sIndex, int16_t sNumEntries)
			{ return ::Blu_SetPalette(prgbt5, sIndex, sNumEntries); }
		int16_t Blu_SetPaletteUsage(int16_t sFull)
			{ return ::Blu_SetPaletteUsage(sFull); }
		void Blu_SetPaletteWindowVisibility(int16_t sVisible)
			{ ::Blu_SetPaletteWindowVisibility(sVisible); }
		// WPALETTE.H
		int16_t Blu_SetPalette(RGBQUAD* prgbq, int16_t sIndex, int16_t sNumEntries)
			{ return ::Blu_SetPalette(prgbq, sIndex, sNumEntries); }
		RGBQUAD* Blu_GetPalette(void)
			{ return ::Blu_GetPalette(); }
		HWND Blu_GetPaletteWindow(void)
			{ return ::Blu_GetPaletteWindow(); }
		// BPARMS.H
		int16_t Blu_GetNumParms(void)
			{ return ::Blu_GetNumParms(); }
		char* Blu_GetParm(int16_t sNum)
			{ return ::Blu_GetParm(sNum); }
		// BSOUND.H
		int32_t Blu_GetSoundOutWindowSize(void)
			{ return ::Blu_GetSoundOutWindowSize(); }
		void Blu_SetSoundOutWindowSize(int32_t lWindowSize)
			{ ::Blu_SetSoundOutWindowSize(lWindowSize); }
		int32_t Blu_GetSoundOutPaneSize(void)
			{ return ::Blu_GetSoundOutPaneSize(); }
		void Blu_SetSoundOutPaneSize(int32_t lPaneSize)
			{ ::Blu_SetSoundOutPaneSize(lPaneSize); }
		int16_t Blu_OpenSoundOut(	uint32_t		ulSampleRate,
										uint16_t	usBitsPerSample,
										uint16_t	usNumChannels)
			{
			return ::Blu_OpenSoundOut(	ulSampleRate,
												usBitsPerSample,
												usNumChannels);
			}
		int16_t Blu_CloseSoundOut(void)
			{ return ::Blu_CloseSoundOut(); }
		int16_t Blu_StartSoundOut(BLU_SND_CALLBACK callback,
										uint32_t ulUser)
			{ return ::Blu_StartSoundOut(callback, ulUser); }
		int16_t Blu_IsSoundOutOpen(void)
			{ return ::Blu_IsSoundOutOpen(); }
		int16_t Blu_ResetSoundOut(void)
			{ return ::Blu_ResetSoundOut(); }
		int16_t Blu_PauseSoundOut(void)
			{ return ::Blu_PauseSoundOut(); }
		int16_t Blu_ResumeSoundOut(void)
			{ return ::Blu_ResumeSoundOut(); }
		int32_t Blu_GetSoundOutPos(void)
			{ return ::Blu_GetSoundOutPos(); }
		int32_t Blu_GetSoundOutTime(void)
			{ return ::Blu_GetSoundOutTime(); }
		// BTIME.H
		int32_t Blu_GetTime(void)
			{ return ::Blu_GetTime(); }
		// SYSINFO
		SYSINFO* GetSysInfo(void)
			{ return &gsi; }

		// Additional non-blue.
		// Returns a CWnd* to the RSPiX window.
		CWnd*	GetBlueWindow();
		
		// Returns a CWnd* to the palette window.
		CWnd* GetPaletteWindow();

		////////////////////////////////////////////////////////////////////////////
		// Overrides
		////////////////////////////////////////////////////////////////////////////
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(CRSPiXBlue)
	protected:
		virtual void PreSubclassWindow();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

	// Implementation
	public:
		virtual ~CRSPiXBlue();

		// Generated message map functions
	protected:
		//{{AFX_MSG(CRSPiXBlue)
		afx_msg void OnDestroy();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	//}}AFX_MSG

		DECLARE_MESSAGE_MAP()
	};

/////////////////////////////////////////////////////////////////////////////
#endif // RSPIXBLUE_H
