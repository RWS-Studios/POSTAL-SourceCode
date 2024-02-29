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
// yatime.h
// Project: Nostril (aka Postal)
//
// History:
//		01/22/97 MJR	Started.
//
//		04/18/97	JMI	Added Suspend() and Resume().
//							I wasn't sure how to handle when Update() was passed a
//							forced elapsed time while the time was suspended.  The two 
//							options are:
//							1) Ignore the forced time and keep suspended anyway.
//							2) Force the time to advance even though we're suspended.
//							I chose 1.
//
//		05/08/97	JMI	Set MaxElapsedRealTime to 200 (was 1250).
//
//		06/05/97	JMI	Changed MaxElapsedRealTime to 67 (15 FPS) (was 200).
//
//		06/09/97	JMI	Put MaxElapsedRealTime back to 200.
//
//		06/26/97 MJR	MaxElapsedRealTime only applies to real time (its name was
//							changed to reflect this).  When a forced elapsed time is
//							being used, that forced time is not limited in any way.
//
//		07/14/97 MJR	Renamed module to yatime.cpp/.h to avoid conflicts with
//							<time.h>, which are only conflicts because the VC++
//							compiler doesn't properly differentiate between #include's
//							using <> and "".
//
////////////////////////////////////////////////////////////////////////////////
#ifndef YATIME_H
#define YATIME_H

#include "Blue.h"


class CTime
	{
	//---------------------------------------------------------------------------
	// Types, enums, etc.
	//---------------------------------------------------------------------------
	public:
		enum
			{
			// This determine the maximum amount of elapsed real time that can occur
			// between calls to Update() before it will fudge elapsed time.  The
			// primarary reason for this is to make debugging possible.  Without it,
			// every time you stopped in the debugger, a huge amount of real time
			// would elapse, and when you resumed the game, everything that was
			// time-based would react to that huge passage of time, which would be
			// rediculous.  Instead, if more than this time elapses, we pretend
			// just some fixed amount of time has elapsed.
			//
			// THIS ONLY APPLIES WHEN USING REAL TIME!
			MaxElapsedRealTime = 200,

			// If the MaxElapsedRealTime is exceeded, then this is the value that is
			// used to fudge the elapsed time.  At some point we might want to
			// calculate and and then use the average elapsed time, but hardwiring
			// it seems to work pretty well.
			//
			// THIS ONLY APPLIES WHEN USING REAL TIME!
			DefaultElapsedRealTime = 100
			};


	//---------------------------------------------------------------------------
	// Variables
	//---------------------------------------------------------------------------
	protected:
		long m_lResetTime;
		long m_lLastTime;
		long m_lGameTime;
		long m_lForceInterval;
		short	m_sNumSuspends;		// Number of Suspend()s that have occurred w/o
											// corresponding Resume()s.

	//---------------------------------------------------------------------------
	// Functions
	//---------------------------------------------------------------------------
	public:
		////////////////////////////////////////////////////////////////////////////////
		// Constructor
		////////////////////////////////////////////////////////////////////////////////
		CTime(void)
			{
			Reset();
			}


		////////////////////////////////////////////////////////////////////////////////
		// Destructor
		////////////////////////////////////////////////////////////////////////////////
		~CTime(void)
			{
			Reset();
			}


		////////////////////////////////////////////////////////////////////////////////
		// Reset game time to 0.
		////////////////////////////////////////////////////////////////////////////////
		void Reset(void)
			{
			m_lResetTime = rspGetMilliseconds();
			m_lLastTime = m_lResetTime;
			m_lGameTime = 0;
			m_lForceInterval = 0;
			m_sNumSuspends	= 0;
			}


		////////////////////////////////////////////////////////////////////////////////
		// Suspend game time.
		////////////////////////////////////////////////////////////////////////////////
		void Suspend(void)
			{
			m_sNumSuspends++;
			}


		////////////////////////////////////////////////////////////////////////////////
		// Resume game time.
		////////////////////////////////////////////////////////////////////////////////
		void Resume(void)
			{
			m_sNumSuspends--;
			}


		////////////////////////////////////////////////////////////////////////////////
		// Update game time.  This must be called once per game loop.  If the real
		// elapsed time between calls to this function exceeds MaxElapsedRealTime, then the
		// game time will only be moved forward by DefaultElapsedRealTime.
		////////////////////////////////////////////////////////////////////////////////
		void Update(
			long lForceElapsed = 0)
			{
			long lNewTime;
			if (lForceElapsed == 0)
				{
				// Get current time
				lNewTime = rspGetMilliseconds();
				}
			else
				{
				// Used specified elapsed time to create a "new" time
				lNewTime = m_lLastTime + lForceElapsed;
				}

			// If suspended . . .
			if (m_sNumSuspends > 0)
				{
				m_lLastTime	= lNewTime;
				}

			// Calculate elapsed time since last update
			long lElapsedTime = lNewTime - m_lLastTime;

			// If we're using real time, we might need to limit the elapsed time
			if (lForceElapsed == 0)
				{
				// If elapsed time is too long, set it to the default elapsed time
				if (lElapsedTime > MaxElapsedRealTime)
					lElapsedTime = DefaultElapsedRealTime;
				}

			// Move game time forward by the elapsed time
			m_lGameTime += lElapsedTime;

			// Save current time for next update
			m_lLastTime = lNewTime;
			}


		////////////////////////////////////////////////////////////////////////////////
		// Get game time since last Reset().  Note that this is NOT compatible with
		// the time returned by rspGetMilliseconds()!
		////////////////////////////////////////////////////////////////////////////////
		long GetGameTime(void)
			{
			// Return current game time
			return m_lGameTime;
			}

		////////////////////////////////////////////////////////////////////////////////
		// Get real time since last Reset().
		////////////////////////////////////////////////////////////////////////////////
		long GetRealTime(void)
			{
			// Return real elapsed time since last reset
			return rspGetMilliseconds() - m_lResetTime;
			}
		};


#endif // YATIME_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
