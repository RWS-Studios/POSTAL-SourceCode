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
#ifndef FILEWIN_H
#define FILEWIN_H

//////////////////////////////////////////////////////////////////////////////
// Headers
//////////////////////////////////////////////////////////////////////////////
#include "btime.h"
#include "file.h"

//////////////////////////////////////////////////////////////////////////////
// Macros.
//////////////////////////////////////////////////////////////////////////////

// Status flags.
#define STATUS_WAITING		0x0001	// Waiting for user to advance a pane.
#define ERROR_FILEACCESS	0x0002	// Error reading file.
#define STATUS_EOF			0x0004	// EOF has been reached.

//////////////////////////////////////////////////////////////////////////////
// Typedefs.
//////////////////////////////////////////////////////////////////////////////

// Access into window.
typedef struct
	{
	UCHAR		*puc;		// Pane data.
	long		lSize;	// Size of pane.
	long		lPos;		// Pane's position in Window.
	} PANE, *PPANE;

// User callback.
typedef void (*FWFUNC)(PPANE ppane, long lUser);

typedef long (*FILEWIN_TIMEFUNC)(long lUserTime);

class CFileWin
	{
	public:		// Construction/Destruction.
		// Default constructor.
		CFileWin();
		// Destructor.
		~CFileWin();

	public:		// Methods.
		// Sets size of window, i/o pane, and user pane.
		// Returns 0 on success.
		short SetSize(long lWinSize, long lIOPaneSize, long lUserPaneSize);
		
		// Sets interval between input access.
		void SetInputInterval(long lInputInterval)
			{ m_lInputInterval = lInputInterval; }

		// Opens a window into pszFile.  Read only!
		// Returns 0 on success.
		short Open(char* pszFile);

		// Closes the currently open file.
		// Returns 0 on success.
		short Close(void);

		// Move to next user pane.
		// Returns 0 on success.
		short NextPane(void);

		// Start reading.
		// Returns 0 on success.
		short Start(void);

		// Suspend reading.
		// Returns 0 on success.
		short Suspend(void);

		// Clear status flag.
		void ClearStatus(void)
			{ m_usStatus = 0; }

		// Set time function.  Set to NULL to clear.
		void SetTimeFunc(FILEWIN_TIMEFUNC fnTime, long lTimeUser)
			{ m_fnTime = fnTime; m_lTimeUser = lTimeUser; }

	public:		// Querries.
		// Returns ptr to user access pane.
		PPANE GetCurPane(void)
			{
			return &m_paneUser;
			}

		// Returns TRUE if next user pane is ready; FALSE otherwise.
		short IsNextPaneReady(void);
		// Returns TRUE if next i/o pane is ready; FALSE otherwise.
		short IsNextInputPaneReady(void);

		// Current callback status.
		USHORT GetStatus(void)
			{ return m_usStatus; }

		// Get time based on user handler or blue.
		long GetTime(void)
			{ return (m_fnTime == NULL ? Blu_GetTime() : (*m_fnTime)(m_lTimeUser) ); }

		// Returns TRUE if critical handler is Blue's critical list.
		short IsActive(void)
			{ return m_sActive; }

	protected:	// Internal methods.
		// Sets variables for the first time.
		void Set(void);
		// Resets variables.
		void Reset(void);
		
		// Allocates the file window of size lSize.
		// Returns 0 on success.
		short Alloc(long lSize);
		// Frees the file window.
		void Free(void);

		// Implied this version of CriticalStatic.
		void Critical(void);
		// Called by Blue every time Blu_System is called.  Reads file.
		static void CriticalStatic(CFileWin* pfw);

		// Move to next i/o pane.
		// Returns 0 on success.
		short NextIOPane(void);

	public:		// Members.
		long		m_lUser;				// Set this to whatever you want!
		FWFUNC	m_call;				// User callback (point wherever you want).
	
	protected:	// Members.
		CNFile	m_file;				// File object use to read data.
		UCHAR*	m_pucWindow;		// Window into file.
		long		m_lWinSize;			// Size of window.
		long		m_lInputInterval;	// Time between pane input accesses.
		long		m_lNextTime;		// Time of next pane input.
		PANE		m_paneIn;			// Window pane for input.
		PANE		m_paneUser;			// Window pane for user access.
		USHORT	m_usStatus;			// Current status.
		FILEWIN_TIMEFUNC	m_fnTime;// Custom time handler.
		long		m_lTimeUser;		// User value for time handler.
		short		m_sActive;			// TRUE if critical is active, FALSE 
											// otherwise.
		short		m_sSuspend;			// Number of calls to suspend w/o matching
											// start.
	};


#endif // FILEWIN_H
//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
