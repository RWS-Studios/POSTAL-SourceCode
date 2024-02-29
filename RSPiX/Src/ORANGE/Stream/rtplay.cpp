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
// RtPlay.CPP
// 
// History:
//		09/21/95 JMI	Started.
//
//////////////////////////////////////////////////////////////////////////////
//
// This class controls the flow of the streaming of a file.  By putting this
// at a higher level than the basic read stuff, we have allowed one to write
// their own flow control for a stream that could override or use any of the
// components used by this module (i.e., CFileWin, CFilter, CDispatch, CRes,
// etc.).
//
//////////////////////////////////////////////////////////////////////////////
//
// There are many states that the streaming can be in.  The following table
// shows the states that can be changed from/to.  It is not necessary to
// handle transitions not in this table (this should make message handler
// logic much simpler) (you may want to use a default: case in your switch,
// if you have one, that displays a warning if you get a transition not in
// this table).
//
//	From\To	| Stopped | Starting | Begin | Playing | Aborting | Ending | Pause
//	---------+---------+----------+-------+---------+----------+--------+------
// Stopped	| --      | Yes      | No    | No      | No       | No     | No   
// Starting	| No      | --       | Yes   | No      | Yes      | No     | No   
// Begin		| No      | No       | --    | Yes     | Yes      | No     | No   
// Playing	| No      | No       | No    | --      | Yes      | Yes    | Yes   
// Aborting	| Yes     | No       | No    | No      | --       | No     | No   
// Ending	| Yes     | No       | No    | No      | Yes      | --     | No   
// Pause		| No      | No       | No    | Yes     | Yes      | No     | --   
//
// Currently there are a total of 13 transitions that could be handled by
// your handler.  
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
#include "rtplay.h"
#include "rttypes.h"

//////////////////////////////////////////////////////////////////////////////
// Orange Headers.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Yellow Headers.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Module specific macros.
//////////////////////////////////////////////////////////////////////////////
// If behind more than MAX_LAG a TRACE displays how far behind we are in 
// debug build only.
#define MAX_LAG	500L	// One half second.

// Default size for file window.
#define DEF_WINDOW_SIZE			(60L * 1024L)
#define DEF_INPUTPANE_SIZE		(20L * 1024L)
#define DEF_FILTERPANE_SIZE	(20L * 1024L)

// Commands from the RTF.
#define CMD_INIT				0x0000
#define CMD_SUSPENDREADS	0x0001
#define CMD_RESUMEREADS		0x0002
#define CMD_EOC				0x0003
#define CMD_ENDING			0x0004
#define CMD_STREAMHIT		0x0005

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
CRtPlay::CRtPlay()
	{
	// These values are set only on construction, or by the user.
	m_lWindowSize		= DEF_WINDOW_SIZE;
	m_lInputPaneSize	= DEF_INPUTPANE_SIZE;
	m_lFilterPaneSize	= DEF_FILTERPANE_SIZE;

	Set();
	}

//////////////////////////////////////////////////////////////////////////////
//
// Destructor.
//
//////////////////////////////////////////////////////////////////////////////
CRtPlay::~CRtPlay()
	{
	Reset();
	}

//////////////////////////////////////////////////////////////////////////////
// Internal Functions.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Sets variables w/o regard to current values.
//
//////////////////////////////////////////////////////////////////////////////
void CRtPlay::Set(void)
	{
	// Set data handler (where we get direction from the RT file).
	m_dispatch.SetDataHandler(	RT_TYPE_RTFINFO, RtInfoCallStatic);
	m_dispatch.SetUserVal(RT_TYPE_RTFINFO, (long)this);
	
	// Let resource know who the dispatcher is.
	m_res.SetDispatcher(&m_dispatch);

	// Let dispatcher know who the filter is.
	m_dispatch.SetFilter(&m_filter);
	// Let dispatcher know what time base to use.
	m_dispatch.SetTimeFunc(CRtTime::GetTime, (long)&m_rttime);

	// Let filter know who the file window is.
	m_filter.SetFileWin(&m_filewin);

	// Let file win know what time base to use.
	m_filewin.SetTimeFunc(CRtTime::GetTime, (long)&m_rttime);

	m_usState	= RT_STATE_STOPPED;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Resets variables.  Performs deallocation if necessary.
//
//////////////////////////////////////////////////////////////////////////////
void CRtPlay::Reset(void)
	{
	Set();
	}

//////////////////////////////////////////////////////////////////////////////
//
// Adds an RT_TYPE_RTFINFO chunk to the dispatcher with the specified command
// and parameters.
// Returns 0 on success.
//
//////////////////////////////////////////////////////////////////////////////
short CRtPlay::CreateCmd(USHORT usCmd, long lTime, long lParm1, long lParm2)
	{
	short	sRes	= 0;	// Assume success.
	long	lSize	= sizeof(usCmd) + sizeof(lParm1) + sizeof(lParm2);

	UCHAR*	puc	= (UCHAR*)malloc(lSize);

	// If successful . . .
	if (puc != NULL)
		{
		CNFile file;
		file.Open(puc, lSize, ENDIAN_BIG);

		file.Write(&usCmd);
		file.Write(&lParm1);
		file.Write(&lParm2);

		file.Close();

		// Send chunk to dispatcher.
		if (m_dispatch.AddItem(puc, lSize, RT_TYPE_RTFINFO, 0, lTime) == 0)
			{
			// Success.
			}
		else
			{
			TRACE("CreateCmd(): Unable to add command to dispatcher.\n");
			sRes = -1;
			}

		// If any errors occurred after allocation . . .
		if (sRes != 0)
			{
			free(puc);
			}
		}
	else
		{
		TRACE("CreateCmd(): Unable to allocate chunk for command.\n");
		sRes = -1;
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Handles data callbacks from dispatch.
// Returns RET_FREE if puc should be freed and RET_DONTFREE, otherwise.
//
//////////////////////////////////////////////////////////////////////////////
short CRtPlay::RtInfoCall(	UCHAR* puc, long lSize, USHORT usType, UCHAR ucFlags,
									long lTime)
	{
	short	sError	= 0;

	// If this is the init/first chunk . . .
	if (ucFlags & RT_FLAG_INIT)
		{
		// Suspend reading (remembers time until next read).
		m_filewin.Suspend();
		// Set time to INIT time.
		m_rttime.SetTime(lTime);
		// Resume reading (next read will occur based on time until next read at
		// Suspend() call above).
		m_filewin.Start();
		}
	else
		{
		#if _DEBUG
			if (lTime + MAX_LAG < m_rttime.GetTime())
				{
				TRACE("RtInfoCall(): %ld ms behind!\n", 
						m_rttime.GetTime() - lTime);
				}
		#endif // _DEBUG
		}

	CNFile	file;

	if (file.Open(puc, lSize, ENDIAN_BIG) == 0)
		{
		USHORT	usCmd;
		ULONG		ulChannels;
		long		lTimeVal;
		long		lVal;

		// Get type of info.
		file.Read(&usCmd);

		switch (usCmd)
			{
			case CMD_INIT:	
				// Gives us info on this rtf.  This is, basically, the header and 
				// could contain the record table, etc.  This should be the very
				// first chunk of any RTF.
				// Read the maximum read rate in bytes per second.
				file.Read(&lVal);
				ASSERT(lVal != 0L);
				// Compute the input interval.
				m_filewin.SetInputInterval((m_lInputPaneSize * 1000) / lVal);
				break;

			case CMD_SUSPENDREADS:
				// Suspend the file window until lTimeVal.
				file.Read(&lTimeVal);
				if (m_filewin.Suspend() == 0)
					{
					// Add an RTFINFO chunk at the specified time to tell us to restart.
					if (CreateCmd(CMD_RESUMEREADS, lTimeVal, 0L, 0L) == 0)
						{
						// Success.
						}
					else
						{
						TRACE("RtInfoCall(): Failed to add RTFINFO RESUME command chunk "
								"to the dispatcher.\n");
						sError = -3;
						}
					}
				else
					{
					TRACE("RtInfoCall(): Failed to suspend file window! Aborting.\n");
					sError = -2;
					}
				break;

			case CMD_RESUMEREADS:
				// Restart file window.
				if (m_filewin.Start() == 0)
					{
					// Success.
					}
				else
					{
					TRACE("RtInfoCall(): Failed to restart file window! Aborting.\n");
					sError = -4;
					}
				break;

			case CMD_EOC:
				// Get the channels that are done so far.
				file.Read(&ulChannels);
				// Add to done mask.
				m_ulChannelsDone	|= ulChannels;
				// If all current channels done . . .
				if ((m_ulChannelsDone & m_filter.GetFilter()) == m_filter.GetFilter())
					{
					// Attempt to stop file window, if still running . . . 
					if (m_filewin.Suspend() == 0)
						{
						// Done streaming (processing may need to continue).
						SetState(RT_STATE_ENDING);
						// Dispatcher should be done.
						ASSERT(m_dispatch.IsEmpty() == TRUE);
						}
					else
						{
						TRACE("RtInfoCall(): Failed to suspend file window!  All channels "
								"complete.\n");
						sError = -5;
						}
					}
				break;

			case CMD_STREAMHIT:
				// This is simply data to provide minimum throughput for the stream.
				// On a CD this can be used to periodically hit the disc to keep it
				// from straying too far ahead.
				break;

			default:
				TRACE("RtInfoCall(): Invalid type.\n");
				sError = -3;
				break;
			}

		file.Close();
		}
	else
		{
		TRACE("RtInfoCall(): Unable to open info.\n");
		sError = -1;
		}

	// If we got any errors . . .
	if (sError != 0)
		{
		if (Abort() == 0)
			{
			}
		else
			{
			TRACE("RtInfoCall(): Failed to abort current play after error.\n");
			}
		}

	// Have dispatcher free this buffer for us.
	return RET_FREE;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Callback dispatcher (calls the implied this version).
// Returns RET_FREE if puc should be freed and RET_DONTFREE, otherwise.
// (static)
//
//////////////////////////////////////////////////////////////////////////////
short CRtPlay::RtInfoCallStatic(	UCHAR* puc, long lSize, USHORT usType, 
											UCHAR ucFlags, long lTime, long l_pRtPlay)
	{
	return ((CRtPlay*)l_pRtPlay)->RtInfoCall(puc, lSize, usType, ucFlags, lTime);
	}

//////////////////////////////////////////////////////////////////////////////
//
// Sets the state to the new state.  Generates messages if a state change
// occurs.
// Returns 0 on success.
//
//////////////////////////////////////////////////////////////////////////////
short CRtPlay::SetState(USHORT usState)
	{
	short	sRes	= 0;	// Assume success.

	// If new state . . .
	if (m_usState != usState)
		{
		// Currently there is a one to one correspondence between the 
		// RT_STATE_*'s and the RT_MSG_*'s so we simply copy the state
		// as the message.  If the correspondence is somehow removed,
		// this should be updated.  Of course, if you simply change the
		// macros, you'll never see this until you figure out that there's
		// a problem.
		USHORT	usMsg	= usState;
		short		sNum	= m_dispatch.SendHandlerMessage(usMsg);

		if (sNum == 0)
			{
			// All message handlers okayed the new state.  Change it.
			m_usState	= usState;
			}
		else
			{
			TRACE("SetState(): Attempt to change to %s state rejected by %d "
					"handlers!\n",
					(usState == RT_STATE_STOPPED	? "STOPPED"		:
					(usState == RT_STATE_STARTING	? "STARTING"	: 
					(usState == RT_STATE_BEGIN		? "BEGIN"		:
					(usState == RT_STATE_PLAYING	? "PLAYING"		:
					(usState == RT_STATE_PAUSE		? "PAUSE"		:
					(usState == RT_STATE_ABORTING	? "ABORTING"	:
					(usState == RT_STATE_ENDING	? "ENDING"		: "UNKNOWN"
					) ) ) ) ) ) ), sNum);
			// At least one handler rejected the state change.
			sRes = 1;
			}
		
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Handles current state.  Called by Blue's critical handler list.
//
//////////////////////////////////////////////////////////////////////////////
void CRtPlay::Critical(void)
	{
	short	sError	= 0;

	switch (m_usState)
		{
		case RT_STATE_STOPPED:
			// Done.
			if (Blu_RemoveCritical(CriticalStatic) == 0)
				{
				}
			else
				{
				TRACE("Critical(): Unable to remove critical handler.\n");
				}
			break;

		case RT_STATE_STARTING:
			// If all handlers ready . . .
			if (m_dispatch.SendHandlerMessage(RT_STATE_BEGIN) == 0)
				{
				// Switch to playing state.
				SetState(RT_STATE_PLAYING);
				}
			break;

		case RT_STATE_BEGIN:
			// Should not happen.
			TRACE("Critical(): SUX! Problem: this state should not be reached!\n");
			sError = -1;
			break;

		case RT_STATE_PLAYING:
			break;

		case RT_STATE_PAUSE:
			break;

		case RT_STATE_ENDING:
			// Wait for dispatcher to empty . . .
			if (m_dispatch.IsEmpty() == FALSE)
				{
				break;
				}

			// Intentional fall through to RT_STATE_ABORTING.

		case RT_STATE_ABORTING:
			// Try to end as soon as possible.
			if (m_dispatch.SendHandlerMessage(RT_STATE_STOPPED) == 0)
				{
				// Attempt to stop dispatcher . . .
				if (m_dispatch.Suspend() == 0)
					{
					}
				else
					{
					TRACE("Critical(): Unable to suspend dispatcher.\n");
					}

				SetState(RT_STATE_STOPPED);
				}
			break;
		}

	if (sError != 0)
		{
		Abort();
		}
	}

//////////////////////////////////////////////////////////////////////////////
// Public methods.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Opens a stream file for play.
// Returns 0 on success.
//
//////////////////////////////////////////////////////////////////////////////
short CRtPlay::Open(char* pszFileName)
	{
	short	sRes	= 0;	// Assume success.

	if (Close() == 0)
		{
		// Attempt to open file window . . .
		if (m_filewin.Open(pszFileName) == 0)
			{
			// Attempt to set window and pane sizes . . .
			if (m_filewin.SetSize(m_lWindowSize, m_lInputPaneSize, 
				m_lFilterPaneSize) == 0)
				{
				// Successfully opened.
				}
			else
				{
				TRACE("Open(\"%s\"): Unable to set file window and pane sizes.\n", 
						pszFileName);
				sRes = -3;
				}

			// If any errors occur after opening file . . .
			if (sRes != 0)
				{
				// Close it.
				Close();
				}
			}
		else
			{
			TRACE("Open(\"%s\"): Unable to open file window.\n", pszFileName);
			sRes = -2;
			}
		}
	else
		{
		TRACE("Open(\"%s\"): Unable to close currently open stream file.\n", pszFileName);
		sRes = -1;
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Closes an open stream file.
// Returns 0 on success.
//
//////////////////////////////////////////////////////////////////////////////
short CRtPlay::Close(void)
	{
	short	sRes	= 0;	// Assume success.

	// If stopped . . .
	if (m_usState == RT_STATE_STOPPED)
		{
		if (m_filewin.Close() == 0)
			{
			// Successfully closed.
			// Tell resource manager to free unused resources.
			m_res.FreeAll();
			}
		else
			{
			TRACE("Close(): Failed to close the file window!\n");
			sRes = -2;
			}
		}
	else
		{
		TRACE("Close(): The stream file is not stopped!\n");
		sRes = -1;
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Starts play.
// Returns 0 on success.
//
//////////////////////////////////////////////////////////////////////////////
short CRtPlay::Play(void)
	{
	short	sRes	= 0;	// Assume success.

	// If stopped . . .
	if (m_usState == RT_STATE_STOPPED)
		{
		// Set the time to 0.
		m_rttime.SetTime(0L);
		// Start with no channels done.
		m_ulChannelsDone	= 0;

		// Attempt to start the file window . . .
		if (m_filewin.Start() == 0)
			{
			// Attempt to start the dispatching . . .
			if (m_dispatch.Start() == 0)
				{
				// Attempt to start critical handler . . .
				if (Blu_AddCritical(CriticalStatic, (long)this) == 0)
					{
					if (SetState(RT_STATE_STARTING) == 0)
						{
						// Successfully changed to starting state.
						}
					else
						{
						TRACE("Play(): At least one handler rejected starting.\n");
						sRes = -4;
						}

					// If any errors occurred after we started the critical handler . . .
					if (sRes != 0)
						{
						Blu_RemoveCritical(CriticalStatic);
						}
					}
				else
					{
					TRACE("Play(): Unable to add critical handler.\n");
					sRes = -5;
					}

				// If any errors occurred after we started the dispatcher . . .
				if (sRes != 0)
					{
					m_dispatch.Suspend();
					}
				}
			else
				{
				TRACE("Play(): The dispatcher did not start!\n");
				sRes = -3;
				}

			// If any errors occurred after we started the file window . . .
			if (sRes != 0)
				{
				m_filewin.Suspend();
				}
			}
		else
			{
			TRACE("Play(): The file window did not start!\n");
			sRes = -2;
			}
		
		// If any errors occurred after we started the file window . . .
		if (sRes != 0)
			{
			m_filewin.Suspend();
			}
		}
	else
		{
		TRACE("Play(): The stream file is not stopped!\n");
		sRes = -1;
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Aborts current play.
// Returns 0 on success.
//
//////////////////////////////////////////////////////////////////////////////
short CRtPlay::Abort(void)
	{
	short	sRes	= 0;	// Assume success.

	// If not stopped . . .
	if (m_usState != RT_STATE_STOPPED)
		{
		if (m_filewin.IsActive() == TRUE)
			{
			// Suspend file window processing . . .
			if (m_filewin.Suspend() == 0)	
				{
				}
			else
				{
				TRACE("Abort(): Unable to suspend file window.\n");
				sRes = -2;
				}
			}

		if (m_dispatch.IsActive() == TRUE)
			{
			// Suspend dispatching . . .
			if (m_dispatch.Suspend() == 0)
				{
				}
			else
				{
				TRACE("Abort(): Unable to suspend dispatcher.\n");
				sRes = -3;
				}
			}

		// If completed successfully . . .
		if (sRes == 0)
			{
			// Change state.
			if (SetState(RT_STATE_ABORTING) != 0)
				{
				TRACE("Abort(): At least one handler responded with an error.\n");
				}
			}
		}
	else
		{
		TRACE("Abort(): The stream file is already stopped!\n");
		sRes = -1;
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Pauses play.
// Returns 0 on success.
//
//////////////////////////////////////////////////////////////////////////////
short CRtPlay::Pause(void)
	{
	short	sRes	= 0;	// Assume success.

	// If playing . . .
	if (m_usState == RT_STATE_PLAYING)
		{
		// This should suspend any timer based stuff.
		m_rttime.Suspend();

		// If completed successfully . . .
		if (sRes == 0)
			{
			// Change state.
			if (SetState(RT_STATE_PAUSE) == 0)
				{
				}
			else
				{
				TRACE("Pause(): At least one handler responded with an error to "
						"entering the PAUSE state.\n");
				sRes = -3;
				}
			}
		}
	else
		{
		TRACE("Pause(): The stream file is stopped!\n");
		sRes = -1;
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Resumes play (after Pause()).
// Returns 0 on success.
//
//////////////////////////////////////////////////////////////////////////////
short CRtPlay::Resume(void)
	{
	short	sRes	= 0;	// Assume success.

	if (m_usState == RT_STATE_PAUSE)
		{
		// This should resume any timer based stuff.
		m_rttime.Resume();

		// Change state.
		if (SetState(RT_STATE_PLAYING) == 0)
			{
			}
		else
			{
			TRACE("Resume(): At least one handler responded with an "
					"error to entering the PLAYING state.\n");
			sRes = -4;

			if (Abort() != 0)
				{
				TRACE("Resume(): Unable to abort after failed resume.\n");
				}
			}
		}
	else
		{
		TRACE("Resume(): The stream file is not paused!\n");
		sRes = -1;
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
