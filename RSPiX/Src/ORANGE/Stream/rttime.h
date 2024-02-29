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
#ifndef RTTIME_H
#define RTTIME_H

//////////////////////////////////////////////////////////////////////////////
// Headers.
//////////////////////////////////////////////////////////////////////////////
#include "bdebug.h"
#include "btime.h"
					  
//////////////////////////////////////////////////////////////////////////////
// Macros.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Typedefs.
//////////////////////////////////////////////////////////////////////////////

typedef long (*RTTIMEFUNC)(void);

class CRtTime
	{
	public:		// Construction/Destruction.
		// Default constructor.
		CRtTime()
			{
			m_fnTime			= NULL;
			m_lOffset		= 0L;
			m_sSuspended	= 0;
			// Might as well start out at 0.
			SetTime(0L);
			}

		// Destructor.
		~CRtTime()
			{ }

	public:		// Methods.
		// Sets (or clears, if NULL) the timer override function.
		void SetTimeFunc(RTTIMEFUNC fnTime)
			{ m_fnTime = fnTime; }

		// Set the current time.  Sets the time offset to specified time minus
		// the current GetTime() so that a subsequent call to GetTime() would.
		void SetTime(long lTime)
			{
			// Zero the offset so we get the actual reported time.
			m_lOffset	= 0;
			m_lOffset	= lTime - GetTime(); 
			}

		// Suspends the timer.  Stores the current time.
		// Every call to this requires a subsequent Resume().
		void Suspend(void)
			{
			// If this is the first . . .
			if (m_sSuspended++ == 0)
				{
				// Get the current time.
				m_lSuspended	= GetTime();
				}
			}

		// Resumes the timer.  Basically, does a SetTime() with the time that
		// Suspend was called.
		void Resume(void)
			{
			ASSERT(m_lSuspended > 0);
			// If this is the last necessary resume to release the timer . . .
			if (--m_sSuspended == 0)
				{
				// Set the time to the time the timer was first suspended.
				SetTime(m_lSuspended);
				}
			}

	public:		// Querries.
		// Returns the time from the override function if set or, if not set, 
		// from Blu_GetTime().  If suspended, this function returns the time
		// of the suspension.
		long GetTime(void)
			{
			if (m_sSuspended == 0)
				return (m_fnTime != NULL ? (*m_fnTime)() : Blu_GetTime()) + m_lOffset;
			else
				return m_lSuspended;
			}

		// Static version of above for those that need to call via a ptr and
		// don't know anything about this object.  Send this a long and it
		// will use it to call the appropriate 'this'-based GetTime().
		static long GetTime(long l_pRtTime)
			{
			return ((CRtTime*)l_pRtTime)->GetTime();
			}

	protected:	// Internal methods.

	public:		// Members.

	protected:	// Members.
		
		RTTIMEFUNC	m_fnTime;		// If set, used to get time.
		long			m_lOffset;		// Added to the time to manipulate it.
		long			m_lSuspended;	// Time at which the timer was suspended.
		short			m_sSuspended;	// TRUE if the timer is currently suspended.
	};


#endif // RTTIME_H
//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
