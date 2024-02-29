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
#ifndef TASK_H
#define TASK_H

#include "Blue.h"

// If PATHS_IN_INCLUDES macro is defined, we can utilized relative
// paths to a header file.  In this case we generally go off of our
// RSPiX root directory.  System.h MUST be included before this macro
// is evaluated.  System.h is the header that, based on the current
// platform (or more so in this case on the compiler), defines 
// PATHS_IN_INCLUDES.  Blue.h includes system.h so you can include that
// instead.
#ifdef PATHS_IN_INCLUDES
	#include "ORANGE/CDT/List.h"
#else
	#include "list.h"
#endif // PATHS_IN_INCLUDES

// Forward declare RTask for typedef.
class RTask;

// Handy-dandy typedef.
typedef RTask* PTASK;

class RTask
	{
	/////////////////////// Typedefs & Enums ////////////////////////////////////
	public:
		// Typedef for user task callback.  This is the type of function called.
		typedef void (*TaskFunc)(	// Returns nothing.
			ULONG ulUser);				// User defined m_ulUser.

		// Typedef to override this RTask's timer.  If overridden, this RTask
		// will use this callback to get the time.
		typedef long (*TimeFunc)(	// Returns time as a long in milliseconds.
			long lTimeUser);			// User defined m_lTimeUser.

	/////////////////////// Con/Destruction ////////////////////////////////////
	public:
		// Default constructor.
		RTask();
		// Special constructor that passes parms on to Init().
		RTask(TaskFunc tf, ULONG ulUser);
		// Destructor.
		~RTask();

	////////////////////////// Querries ///////////////////////////////////////
	public:
		// Returns TRUE if this task is currently in the list of tasks to be
		// run; FALSE, otherwise.
		short IsActive(void) { return m_sActive; }

		// Returns the current time based on either the user base or Blue.
		long GetTime(void)
			{ return (m_fnTime == NULL ? rspGetMilliseconds() : (*m_fnTime)(m_lTimeUser)); }
		
	////////////////////////// Methods ////////////////////////////////////////
	public:
		// Initialize task info.
		void Init(TaskFunc tf, ULONG ulUser);
		// Kill task info.
		// Returns 0 on success.
		short Kill(void);

		// Start this task.
		// Returns 0 on success.
		short Start(void);
		// Suspend this task (can be restarted after this is call).
		// Returns 0 on success.
		short Suspend(void);

		// Use a custom timer.
		void SetTimeFunc(TimeFunc fnTime, long lTimeUser)
			{ m_fnTime = fnTime; m_lTimeUser = lTimeUser; }
		
		/////////////////////// Static functions ///////////////////////////////

		// Critical call for all instances (static).
		static void Do(void);

	////////////////////////// Internal Methods ///////////////////////////////
	protected:
		// Initialize instantiable members.
		void Reset(void);


	////////////////////////// Member vars ////////////////////////////////////
	public:
		// The following member variables are safe to tamper with from outside
		// this function, and that is why they're public.

		TaskFunc		m_fnTask;		// User specified task function.
		ULONG			m_ulUser;		// User specified parm to task function.

		long			m_lInterval;	// User specified timer interval.
		long			m_lNextExpiration;	// Next time to call task.

	protected:
		// The following member variables are NOT safe to tamper with from 
		// outside this function, and that is why they're protected.

		short				m_sActive;	// TRUE if active (in list), FALSE otherwise.
		TimeFunc			m_fnTime;	// Custom time function.
		long				m_lTimeUser;// Custom time function user value.


		/////////////////////// Static members /////////////////////////////////
		static RList<RTask>	ms_listActive;	// List of tasks to be called.
		
	};

#endif // TASK_H
//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
