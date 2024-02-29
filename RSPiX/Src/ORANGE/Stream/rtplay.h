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
#ifndef RTPLAY_H
#define RTPLAY_H

//////////////////////////////////////////////////////////////////////////////
// Headers.
//////////////////////////////////////////////////////////////////////////////
#include "filewin.h"
#include "filter.h"
#include "dispatch.h"
#include "res.h"
#include "rttime.h"
#include "task.h"
					  
//////////////////////////////////////////////////////////////////////////////
// Macros.
//////////////////////////////////////////////////////////////////////////////
// Values for m_usState.
#define RT_STATE_STOPPED	0x0000	// Currently no file is being streamed.
#define RT_STATE_STARTING	0x0001	// Preparing to stream (building up enough
												// input that we could attain stream data
												// usage).
#define RT_STATE_BEGIN		0x0002	// Internal use only!!
#define RT_STATE_PLAYING	0x0003	// Streaming.
#define RT_STATE_PAUSE		0x0004	// Streaming is temporarily suspended.
#define RT_STATE_ABORTING	0x0005	// Terminating stream early.
#define RT_STATE_ENDING		0x0006	// Ending due to end of stream data.

// Values for messages passed to handlers.
#define RT_MSG_STOPPED		0x0000	// The last thing a message handler will
												// receive from a stream session, indicat-
												// ing that streaming is done completely.
#define RT_MSG_STARTING		0x0001	// The first thing a message handler will
												// receive indicating we are preparing to
												// stream.
#define RT_MSG_BEGIN			0x0002	// This let's the handler know it's time
												// to open the flood-gates.  An audio
												// handler might use this as an
												// opportunity to actually start playing
												// data.
#define RT_MSG_PLAYING		0x0003	// Streaming.
#define RT_MSG_PAUSE			0x0004	// Streaming is paused.
#define RT_MSG_ABORTING		0x0005	// Prematurely ending streaming.
#define RT_MSG_ENDING		0x0006	// Ending streaming due to end of data.

//////////////////////////////////////////////////////////////////////////////
// Typedefs.
//////////////////////////////////////////////////////////////////////////////

class CRtPlay
	{
	public:		// Construction/Destruction.
		// Default constructor.
		CRtPlay();
		// Destructor.
		~CRtPlay();

	public:		// Methods.
		// Opens a stream file for play.
		// Returns 0 on success.
		short Open(char* pszFileName);
		// Closes an open stream file.
		// Returns 0 on success.
		short Close(void);
		// Starts play.
		// Returns 0 on success.
		short Play(void);
		// Aborts current play.
		// Returns 0 on success.
		short Abort(void);
		// Pauses play.
		// Returns 0 on success.
		short Pause(void);
		// Resumes play (after Pause()).
		// Returns 0 on success.
		short Resume(void);

		// Sets the time handler function in our CRtTime.
		void SetTimeFunc(RTTIMEFUNC fnTime)
			{ m_rttime.SetTimeFunc(fnTime); }

		// Sets the channels to play.
		void SetChannels(USHORT usFilter)
			{ m_filter.SetFilter(usFilter); }

		// Sets the window and panes sizes for the file window (which is set 
		// imeediately after opening).
		void SetSizes(	long lWindowSize, long lInputPaneSize, 
							long lFilterPaneSize)
			{
			m_lWindowSize		= lWindowSize;
			m_lInputPaneSize	= lInputPaneSize;
			m_lFilterPaneSize	= lFilterPaneSize;
			}

	public:		// Querries.
		// Get current status.
		USHORT GetState(void)
			{ return m_usState; }

	protected:	// Internal methods.

		// Sets members w/o regard for current value.
		void Set(void);
		// Resets members. Deallocates memory if necessary.
		void Reset(void);

		// Sets the state to the new state.  Generates messages if a state change
		// occurs.
		// Returns 0 on success.
		short SetState(USHORT usState);

		// Adds an RT_TYPE_RTFINFO chunk to the dispatcher with the specified command
		// and parameters.
		// Returns 0 on success.
		short CreateCmd(USHORT usCmd, long lTime, long lParm1, long lParm2);

		// Handles data callbacks from dispatch.
		// Returns RET_FREE if puc should be freed and RET_DONTFREE, otherwise.
		short RtInfoCall(	UCHAR* puc, long lSize, USHORT usType, UCHAR ucFlags,
								long lTime);
		// Callback dispatcher (calls the implied this version).
		// Returns RET_FREE if puc should be freed and RET_DONTFREE, otherwise.
		static short RtInfoCallStatic(UCHAR* puc, long lSize, USHORT usType, 
												UCHAR ucFlags, long lTime, long l_pRtPlay);

		// Handles current state.  Called by Blue's critical handler list.
		void Critical(void);
		// Static version of above.
		static void CriticalStatic(ULONG ul_pRtPlay)
			{ ((CRtPlay*)ul_pRtPlay)->Critical(); }


	public:		// Members.
		CDispatch	m_dispatch;				// Dispatches types to handlers.
		CFilter		m_filter;				// Filters channels and builds
													// contiguous buffers.
		CFileWin		m_filewin;				// Window into stream file.
		CRes			m_res;					// Stores types to be requested.
		CRtTime		m_rttime;				// Keeps time for us in a nice,
													// pausable/resumable fashion.

	protected:	// Members.
		
		USHORT		m_usState;				// Current state of class.

		ULONG			m_ulChannelsDone;		// Masks of channels that have 
													// completed.

		long			m_lWindowSize;			// Size for file window.
		long			m_lInputPaneSize;		// Size for input pane of file window.
		long			m_lFilterPaneSize;	// Size for filter pane of file window.

	};


#endif // RTPLAY_H
//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
