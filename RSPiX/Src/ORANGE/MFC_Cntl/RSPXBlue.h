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
		void SetDisplayImage(CImage* pim, short sFlip = FALSE);

		// Set the palette Blue uses to draw and redraw the display.
		void SetDisplayPalette(CPal* ppal);


		////////////////////////////////////////////////////////////////////////////
		// Indirection to Blue calls in the proverbial EXE.
		// NOTE:  These are not documented as the WinBlue version could change w/o 
		// this file being updated.  Please refer to the b*.h or b*.cpp file for
		// details.
		////////////////////////////////////////////////////////////////////////////
		// BCRITIC.H
		short Blu_RemoveCritical(CRITICALL cc)	
			{ return ::Blu_RemoveCritical(cc); }
		short Blu_AddCritical(CRITICALL cc, ULONG ulUser)
			{ return ::Blu_AddCritical(cc, ulUser); }
		// BDEBUG.H
		#undef TRACE	// We will use MFC's TRACE
		#define TRACE	::AfxTrace("%s(%d) :", __FILE__, __LINE__),::AfxTrace
		#undef STRACE
		#define STRACE	::AfxTrace
		// BDIALOG.H
		short Blu_MsgBox(USHORT usFlags, char *pszTitle, char *pszFrmt, ...)
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
		short Blu_OpenBox(char* pszBoxTitle, char *pszFileName, short sStrSize)
			{ return ::Blu_OpenBox(pszBoxTitle, pszFileName, sStrSize); }
		short Blu_MultiOpenBox(	char* pszBoxTitle, 		
										char* pszFileNameMemory,
										long	lNumBytes,			
										char **ppszFileNames, 	
										short sMaxPtrs,			
										short* psNumFiles			
										)
			{
			return ::Blu_MultiOpenBox(	pszBoxTitle, pszFileNameMemory,
												lNumBytes, ppszFileNames,
												sMaxPtrs, psNumFiles);
			}
		// BDISPLAY.H
		short Blu_CreateDisplay(long lWidth, long lHeight, short sColorDepth)
			{ return ::Blu_CreateDisplay(lWidth, lHeight, sColorDepth); }
		void Blu_SetRedrawBuf(	void* pvBuf, 				
										long lBufW, long lBufH,	
										long sx, long sy,			
										long dx, long dy, 		
										long lBltW, long lBltH,	
										short sColorDepth,		
										short sVertFlip = FALSE)
			{
			::Blu_SetRedrawBuf(	pvBuf, lBufW, lBufH, sx, sy, dx, dy,
										lBltW, lBltH, sColorDepth, sVertFlip); 
			}
		void Blu_SetDisplayBuf(	void* pvBuf, long lWidth, long lHeight,
										short sColorDepth, short sVertFlip = FALSE)
			{ ::Blu_SetDisplayBuf(pvBuf, lWidth, lHeight, sColorDepth, sVertFlip); }
		U16*	Blu_GetPaletteTranslation(void)
			{ return ::Blu_GetPaletteTranslation(); }
		short Blu_UpdateDisplay(long sx, long sy, 
										long dx, long dy, 
										long w, long h)
			{ return ::Blu_UpdateDisplay(sx, sy, dx, dy, w, h); }
		short Blu_SetWindowControls(short sControls)
			{ return ::Blu_SetWindowControls(sControls); }
		long Blu_GetDisplayInfo(USHORT usInfo)
			{ return ::Blu_GetDisplayInfo(usInfo); }
		short Blu_SetWindowTitle(char *pszTitleNameText)
			{ return ::Blu_SetWindowTitle(pszTitleNameText); }
		short Blu_ValidateRect(long lX, long lY, long lW, long lH)
			{ return ::Blu_ValidateRect(lX, lY, lW, lH); }
		// BJOY.H
		short Blu_UpdateJoy(short sJoy)
			{ return ::Blu_UpdateJoy(sJoy); }
		void Blu_GetJoyPos(short sJoy, long *px, long *py, long *pz)
			{ ::Blu_GetJoyPos(sJoy, px, py, pz); }
		void Blu_GetJoyPrevPos(short sJoy, long *px, long *py, long *pz)
			{ ::Blu_GetJoyPrevPos(sJoy, px, py, pz); }
		USHORT Blu_GetJoyState(short sJoy)
			{ return ::Blu_GetJoyState(sJoy); }
		void Blu_GetJoyState(short sJoy, PJOYSTATE pjs)
			{ ::Blu_GetJoyState(sJoy, pjs); }
		USHORT Blu_GetJoyPrevState(short sJoy)
			{ return ::Blu_GetJoyPrevState(sJoy); }
		void Blu_GetJoyPrevState(short sJoy, PJOYSTATE pjs)
			{ ::Blu_GetJoyPrevState(sJoy, pjs); }
		// BKEY.H
		short Blu_GetKeyboard(PKEYBOARD pkb)
			{ return ::Blu_GetKeyboard(pkb); }
		// BMAIN.H
		void Blu_System(void)
			{ ::Blu_System(); }
		void Blu_DispatchEvents(short sDispatch)
			{ ::Blu_DispatchEvents(sDispatch); }
		// BMOUSE.H
		short Blu_GetMousePos(long* px, long* py)
			{ return ::Blu_GetMousePos(px, py); }
		short Blu_SetMousePos(long x, long y)
			{ return ::Blu_SetMousePos(x, y); }
		short Blu_GetMouseButton(short sButton)
			{ return ::Blu_GetMouseButton(sButton); }
		// BPALETTE.H
		short Blu_SetPalette(PRGBT8 prgbt8, short sIndex, short sNumEntries)
			{ return ::Blu_SetPalette(prgbt8, sIndex, sNumEntries); }
		short Blu_SetPalette(PRGBQ8 prgbq8, short sIndex, short sNumEntries)
			{ return ::Blu_SetPalette(prgbq8, sIndex, sNumEntries); }
		short Blu_SetPalette(PRGBT5 prgbt5, short sIndex, short sNumEntries)
			{ return ::Blu_SetPalette(prgbt5, sIndex, sNumEntries); }
		short Blu_SetPaletteUsage(short sFull)
			{ return ::Blu_SetPaletteUsage(sFull); }
		void Blu_SetPaletteWindowVisibility(short sVisible)
			{ ::Blu_SetPaletteWindowVisibility(sVisible); }
		// WPALETTE.H
		short Blu_SetPalette(RGBQUAD* prgbq, short sIndex, short sNumEntries)
			{ return ::Blu_SetPalette(prgbq, sIndex, sNumEntries); }
		RGBQUAD* Blu_GetPalette(void)
			{ return ::Blu_GetPalette(); }
		HWND Blu_GetPaletteWindow(void)
			{ return ::Blu_GetPaletteWindow(); }
		// BPARMS.H
		short Blu_GetNumParms(void)
			{ return ::Blu_GetNumParms(); }
		char* Blu_GetParm(short sNum)
			{ return ::Blu_GetParm(sNum); }
		// BSOUND.H
		long Blu_GetSoundOutWindowSize(void)
			{ return ::Blu_GetSoundOutWindowSize(); }
		void Blu_SetSoundOutWindowSize(long lWindowSize)
			{ ::Blu_SetSoundOutWindowSize(lWindowSize); }
		long Blu_GetSoundOutPaneSize(void)
			{ return ::Blu_GetSoundOutPaneSize(); }
		void Blu_SetSoundOutPaneSize(long lPaneSize)
			{ ::Blu_SetSoundOutPaneSize(lPaneSize); }
		short Blu_OpenSoundOut(	ULONG		ulSampleRate,
										USHORT	usBitsPerSample,
										USHORT	usNumChannels)
			{
			return ::Blu_OpenSoundOut(	ulSampleRate,
												usBitsPerSample,
												usNumChannels);
			}
		short Blu_CloseSoundOut(void)
			{ return ::Blu_CloseSoundOut(); }
		short Blu_StartSoundOut(BLU_SND_CALLBACK callback,
										ULONG ulUser)
			{ return ::Blu_StartSoundOut(callback, ulUser); }
		short Blu_IsSoundOutOpen(void)
			{ return ::Blu_IsSoundOutOpen(); }
		short Blu_ResetSoundOut(void)
			{ return ::Blu_ResetSoundOut(); }
		short Blu_PauseSoundOut(void)
			{ return ::Blu_PauseSoundOut(); }
		short Blu_ResumeSoundOut(void)
			{ return ::Blu_ResumeSoundOut(); }
		long Blu_GetSoundOutPos(void)
			{ return ::Blu_GetSoundOutPos(); }
		long Blu_GetSoundOutTime(void)
			{ return ::Blu_GetSoundOutTime(); }
		// BTIME.H
		long Blu_GetTime(void)
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
