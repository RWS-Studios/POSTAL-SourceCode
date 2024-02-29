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
//////////////////////////////////////////////////////////////////////////////
//
// FILEWIN.CPP
// 
// History:
//		09/19/95 JMI	Started.
//
//		09/20/95	JMI	Made read only.
//
//////////////////////////////////////////////////////////////////////////////
//
// This object maintains a window on the file.  It fills the panes on
// interval to be read by the user.
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// C Headers.
//////////////////////////////////////////////////////////////////////////////
#include <malloc.h>

//////////////////////////////////////////////////////////////////////////////
// Blue Headers.
//////////////////////////////////////////////////////////////////////////////
#include "System.h"
#include "bdebug.h"
#include "bcritic.h"

//////////////////////////////////////////////////////////////////////////////
// Green Headers.
//////////////////////////////////////////////////////////////////////////////
#include "filewin.h"

//////////////////////////////////////////////////////////////////////////////
// Orange Headers.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Yellow Headers.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Module specific macros.
//////////////////////////////////////////////////////////////////////////////
#define WINDOWINDEX(l)	(l % m_lWinSize)

//////////////////////////////////////////////////////////////////////////////
// Module specific typedefs.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Module specific (static) variables.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Construction/Destruction Functions.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Default constructor.
//
//////////////////////////////////////////////////////////////////////////////
CFileWin::CFileWin()
	{
	Set();
	}

//////////////////////////////////////////////////////////////////////////////
//
// Destructor.
//
//////////////////////////////////////////////////////////////////////////////
CFileWin::~CFileWin()
	{
	Close();
	}

//////////////////////////////////////////////////////////////////////////////
// Internal Functions.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Sets variables w/o regard to current values.
//
//////////////////////////////////////////////////////////////////////////////
void CFileWin::Set(void)
	{
	m_pucWindow		= NULL;
	m_paneUser.puc	= NULL;
	m_paneIn.puc	= NULL;

	m_usStatus		= 0;
	m_call			= NULL;
	m_fnTime			= NULL;

	m_sActive		= FALSE;
	m_sSuspend		= 1;		// Start out suspended.
	}

//////////////////////////////////////////////////////////////////////////////
//
// Resets variables.  Performs deallocation if necessary.
//
//////////////////////////////////////////////////////////////////////////////
void CFileWin::Reset(void)
	{
	ASSERT(m_sActive == FALSE);
	
	Suspend();

	Free();

	Set();
	}

//////////////////////////////////////////////////////////////////////////////
//
// Allocates the file window of size lSize.
// Returns 0 on success.
//
//////////////////////////////////////////////////////////////////////////////
short CFileWin::Alloc(long lSize)
	{
	short	sRes	= 0;	// Assume success.

	// Allocate new window . . .
	m_pucWindow	= (UCHAR*)malloc(lSize);
	if (m_pucWindow != NULL)
		{
		m_paneUser.puc		= m_pucWindow;
		m_paneIn.puc		= m_pucWindow;
		m_lWinSize			= lSize;

		m_paneUser.lPos	= 0L;
		m_paneIn.lPos		= 0L;
		}
	else
		{
		TRACE("Alloc(%ld): Unable to allocate file window.\n", lSize);
		sRes = -1;
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Frees the file window.
//
//////////////////////////////////////////////////////////////////////////////
void CFileWin::Free(void)
	{
	if (m_pucWindow != NULL)
		{
		free(m_pucWindow);
		
		m_pucWindow		= NULL;
		m_paneUser.puc	= NULL;
		m_paneIn.puc	= NULL;
		}
	}

//////////////////////////////////////////////////////////////////////////////
//
// Move to next i/o pane.
// Returns 0 on success.
//
//////////////////////////////////////////////////////////////////////////////
short CFileWin::NextIOPane(void)
	{
	short	sRes	= 0;	// Assume success.

	// If next pane complete . . .
	if (IsNextInputPaneReady() == TRUE)
		{
		// Get position of next pane.
		m_paneIn.lPos	+= m_paneIn.lSize;
		// Move ptr to position.
		m_paneIn.puc	= m_pucWindow + WINDOWINDEX(m_paneIn.lPos);
		}
	else
		{
		sRes = -1;
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Implied this version of CriticalStatic.  Reads/writes file.
//
//////////////////////////////////////////////////////////////////////////////
void CFileWin::Critical(void)
	{
	// If time to update . . .
	if (GetTime() > m_lNextTime)
		{
		if (NextIOPane() == 0)
			{
			// Attempt to fill pane.
			m_paneIn.lSize = m_file.Read(m_paneIn.puc, m_paneIn.lSize);
			
			// Get next read time.
			m_lNextTime = GetTime() + m_lInputInterval;
			}
		else
			{
			TRACE("Critical(): Waiting for application to free up current "
					"pane!\n");
			m_usStatus	|= STATUS_WAITING;
			}
		}

	// If callback provided . . .
	if (m_call != NULL)
		{
		// If next user pane is complete . . .
		if (NextPane() == 0)
			{
			// Call user.
			(*m_call)(&m_paneUser, m_lUser);
			}
		else
			{
			// If EOF . . .
			if (m_file.IsEOF() == TRUE)
				{
				m_usStatus	|= STATUS_EOF;
				}
			else
				{
				// If read error . . .
				if (m_file.Error() == TRUE)
					{
					TRACE("Critical(): A read error stopped reading before complete.\n");
					m_usStatus	|= ERROR_FILEACCESS;
					}
				}

			// If not ready b/c file input has stopped . . .
			if (m_usStatus & (STATUS_EOF | ERROR_FILEACCESS))
				{
				// If there is not a full pane of data . . .
				if (m_paneUser.lPos + m_paneUser.lSize * 2 > m_paneIn.lPos + m_paneIn.lSize)
					{
					// Advance.
					m_paneUser.lPos	+= m_paneUser.lSize;
					m_paneUser.puc		= m_pucWindow + WINDOWINDEX(m_paneUser.lPos);
					// Adjust pane size.
					m_paneUser.lSize	= (m_paneIn.lPos + m_paneIn.lSize) - m_paneUser.lPos;
					// Call user.
					(*m_call)(&m_paneUser, m_lUser);
					}

				// Suspend this.
				Suspend();
				}
			else
				{
				// User pane caught up with input even though no error nor EOF.
				// This could occur if the user pane is larger than the input 
				// pane.
				}
			}
		}
	}

//////////////////////////////////////////////////////////////////////////////
//
// Called by Blue every time Blu_System is called.
//
//////////////////////////////////////////////////////////////////////////////
void CFileWin::CriticalStatic(CFileWin* pfw)
	{
	pfw->Critical();
	}
	
//////////////////////////////////////////////////////////////////////////////
// Public Functions.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Opens a window into pszFile.  sMode indicates read or write
// { FW_MODE_READ, FW_MODE_WRITE }.
// Returns 0 on success.
//
//////////////////////////////////////////////////////////////////////////////
short CFileWin::Open(char* pszFile)
	{
	short	sRes	= 0;	// Assume success.

	// Attempt to open file . . .
	// Endianness doesn't matter (only UCHAR reads).
	if (m_file.Open(pszFile, "rb", ENDIAN_BIG) == 0)
		{
		}
	else
		{
		TRACE("Open(\"%s\"): Unable to open file.\n");
		sRes = -1;
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Closes the currently open file.
// Returns 0 on success.
//
//////////////////////////////////////////////////////////////////////////////
short CFileWin::Close(void)
	{
	short	sRes	= 0;	// Assume success.

	// If file is open . . .
	if (m_file.IsOpen() == TRUE)
		{
		// Close.
		if (m_file.Close() == 0)
			{
			// Success.
			Suspend();
			Reset();
			}
		else
			{
			TRACE("Close(): CNFile::Close() failed.\n");
			sRes = -1;
			}
		}

	return sRes;
	}


//////////////////////////////////////////////////////////////////////////////
//
// Sets size of window, user pane, and i/o pane.  Anything currently in the 
// window will be lost.	
// This function causes a seek.
// If this fails you could end up with no more buffer, even if you had one 
// before calling this, so CHECK THE RETURN VALUE!
// ASSUMPTIONS: File is open.
// Returns 0 on success.
//
//////////////////////////////////////////////////////////////////////////////
short CFileWin::SetSize(long lWinSize, long lIOPaneSize, long lUserPaneSize)
	{
	short	sRes	= 0;	// Assume success.

	ASSERT(m_file.IsOpen() == TRUE);
	ASSERT(lWinSize % lIOPaneSize == 0);
	ASSERT(lWinSize % lUserPaneSize == 0);

	// Free current window.
	Free();

	// Allocate new window . . .
	if (Alloc(lWinSize) == 0)
		{
		m_paneUser.lSize	= lUserPaneSize;

		// Store current file position.  Changing the buffer size of a stream
		// requires a seek.
		long	lPos	= m_file.Tell();
		
		// Attempt to set file buffer size . . .
		if (m_file.SetBufferSize(lIOPaneSize) == 0)
			{
			m_paneIn.lSize	= lIOPaneSize;
			
			// Attempt to seek back to current position . . .
			if (m_file.Seek(lPos, SEEK_SET) == 0)
				{
				// Success.
				}
			else
				{
				TRACE("SetSize(): CNFile::Seek failed.\n");
				sRes = -3;
				}
			}
		else
			{
			TRACE("SetSize(): CNFile::SetBufferSize failed.\n");
			sRes = -2;
			}
		
		// If any errors occurred after allocation . . .
		if (sRes != 0)
			{
			Free();
			}
		}
	else
		{
		TRACE("SetSize(): Unable to allocate new window.\n");
		sRes = -1;
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Move to next user pane.
// Returns 0 on success.
//
//////////////////////////////////////////////////////////////////////////////
short CFileWin::NextPane(void)
	{
	short	sRes	= 0;	// Assume success.

	// If next pane complete . . .
	if (IsNextPaneReady() == TRUE)
		{
		// Get position of next pane.
		m_paneUser.lPos	+= m_paneUser.lSize;
		// Move ptr to position.
		m_paneUser.puc		= m_pucWindow + WINDOWINDEX(m_paneUser.lPos);
		}
	else
		{
		sRes = -1;
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Start reading/writing.
// Returns 0 on success.
//
//////////////////////////////////////////////////////////////////////////////
short CFileWin::Start(void)
	{
	short	sRes	= 0;	// Assume success.
	
	if (--m_sSuspend == 0)
		{
		if (m_sActive == FALSE)
			{
			if (Blu_AddCritical((CRITICALL)CriticalStatic, (ULONG)this) == 0)
				{
				// Pick up where we left off.
				m_lNextTime	= GetTime() + m_lNextTime;
				m_sActive	= TRUE;
				}
			else
				{
				sRes = -1;
				TRACE("Start(): Unable to add critical function.\n");
				}
			}
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Suspend reading/writing.
// Returns 0 on success.
//
//////////////////////////////////////////////////////////////////////////////
short CFileWin::Suspend(void)
	{
	short	sRes	= 0;	// Assume success.

	m_sSuspend++;

	if (m_sActive == TRUE)
		{
		if (Blu_RemoveCritical((CRITICALL)CriticalStatic) == 0)
			{
			m_sActive	= FALSE;
			// Remember how long from suspension until next.
			m_lNextTime	= m_lNextTime - GetTime();
			}
		else
			{
			sRes = -1;
			TRACE("Suspend(): Unable to remove critical function.\n");
			}
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Returns TRUE if next user pane is ready; FALSE otherwise.
//
//////////////////////////////////////////////////////////////////////////////
short CFileWin::IsNextPaneReady(void)
	{
	short	sRes	= TRUE;	// Assume ready.
	
	// If next pane is where the next read is going to occur . . .
	if (m_paneUser.lPos + m_paneUser.lSize * 2L > m_paneIn.lPos + m_paneIn.lSize)
		{
		sRes = FALSE;
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Returns TRUE if next input pane is ready; FALSE otherwise.
//
//////////////////////////////////////////////////////////////////////////////
short CFileWin::IsNextInputPaneReady(void)
	{
	short	sRes	= TRUE;	// Assume ready.

	long	lNextInputPaneStart	= WINDOWINDEX(m_paneIn.lPos + m_paneIn.lSize);
	long	lNextInputPaneEnd		= WINDOWINDEX(m_paneIn.lPos + m_paneIn.lSize * 2L);

	long	lCurUserPaneStart		= WINDOWINDEX(m_paneUser.lPos);
	long	lCurUserPaneEnd		= WINDOWINDEX(m_paneUser.lPos + m_paneUser.lSize);

	// If the next input pane intersects the current user pane . . .
	if (	lNextInputPaneStart	< lCurUserPaneEnd 
		&&	lNextInputPaneEnd		> lCurUserPaneStart)
		{
		// You can't fight the seither.
		sRes = FALSE;
		}

	return sRes;
	}


//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
