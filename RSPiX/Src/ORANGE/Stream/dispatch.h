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
#ifndef DISPATCH_H
#define DISPATCH_H

//////////////////////////////////////////////////////////////////////////////
// Headers.
//////////////////////////////////////////////////////////////////////////////
#include "btime.h"
#include "rttypes.h"
#include "slist.h"
#include "filter.h"

//////////////////////////////////////////////////////////////////////////////
// Macros.
//////////////////////////////////////////////////////////////////////////////
#define RET_FREE		0x0000
#define RET_DONTFREE	0x0001

//////////////////////////////////////////////////////////////////////////////
// Typedefs.
//////////////////////////////////////////////////////////////////////////////
// This type is used to call the user to allow them to allocate space for the
// data and return it to be filled.  Return NULL if unable to allocate the
// space or other errror.
typedef UCHAR* (*ALLOC_DISPATCHFUNC)(	long lSize, USHORT usType, UCHAR ucFlags,
													long lUser);

// This type is used to call the user to allow them to DEallocate space al-
// located by a previous call to their ALLOC_DISPATCHFUNC.
typedef void (*FREE_DISPATCHFUNC)(	UCHAR* puc, USHORT usType, UCHAR ucFlags,
												long lUser);

// This type is used to pass the copied chunk to the user ready to be used.
// Returning RET_FREE will cause puc to get freed if it meets the criteria
// for freeing described in the dispatch.cpp header comment.  Return
// RET_DONTFREE to avoid this.
typedef short (*USE_DISPATCHFUNC)(	UCHAR* puc, long lSize, USHORT usType, 
												UCHAR ucFlags, 
												long lTime, long lUser);

// This type is used to pass messages to the handler.
typedef short (*MSG_DISPATCHFUNC)(USHORT usMsg);

// For custom time handlers.
typedef long (*TIME_DISPATCHFUNC)(long lTimeUser);

class CDispatch
	{
	protected:	// Local types.
		// Items waiting to be dispatched.
		typedef struct
			{
			UCHAR*	puc;		// Data.
			long		lSize;	// Size of data in bytes.
			USHORT	usType;	// Type of data.
			UCHAR		ucFlags;	// Flags for data.
			long		lTime;	// Time data is to be dispatched.
			} RTITEM, *PRTITEM;
		

	public:		// Construction/Destruction.
		// Default constructor.
		CDispatch();
		// Destructor.
		~CDispatch();

	public:		// Methods.
		// Sets the data handler for usType to fnUse.
		void SetDataHandler(USHORT usType, USE_DISPATCHFUNC fnUse);

		// Sets the type handler for usType to fnAlloc.
		void SetAllocHandler(USHORT usType, ALLOC_DISPATCHFUNC fnAlloc);

		// Sets the type handler for usType to fnUse.
		void SetFreeHandler(USHORT usType, FREE_DISPATCHFUNC fnFree);

		// Sets the user value for usType to lUser.
		void SetUserVal(USHORT usType, long lUser);

		// Set filter.
		void SetFilter(CFilter* pfilter);

		// Start spewing/blowing chunks.
		// Returns 0 on success.
		short Start(void);
		// Stop spewing/blowing chunks.
		// Returns 0 on success.
		short Suspend(void);

		// Sets the time handler function.
		void SetTimeFunc(TIME_DISPATCHFUNC fnTime, long lTimeUser)
			{ m_fnTime	= fnTime; m_lTimeUser = lTimeUser; }

		// Sends a message to all type handlers.
		// Returns the number of handlers that returned an error.
		short SendHandlerMessage(USHORT usMsg);

		// Adds an item to the list of items to be dispatched.
		// Returns 0 on success.
		short AddItem(	UCHAR* puc, long lSize, USHORT usType, UCHAR ucFlags, 
							long lTime);

	public:		// Querries.
		// Returns the time from the override function if set or, if not set, from
		// Blue.
		long GetTime(void)
			{ return (m_fnTime != NULL ? (*m_fnTime)(m_lTimeUser) : Blu_GetTime()); }

		// Returns TRUE if critical handler is Blue's critical list.
		short IsActive(void)
			{ return m_sActive; }

		// Returns TRUE if there are any chunks to be dispatched in the lists.
		short IsEmpty(void)
			{ return m_slistRtItems.IsEmpty(); }

	protected:	// Internal methods.

		// Sets members w/o regard for current value.
		void Set(void);
		// Resets members. Deallocates memory if necessary.
		void Reset(void);

		// Handles data callbacks from filter.
		void UseCall(	UCHAR* puc, long lSize, USHORT usType, UCHAR ucFlags, 
							long lTime);
		// Callback dispatcher (calls the implied this version).
		static void UseCallStatic(	UCHAR* puc, long lSize, USHORT usType, 
											UCHAR ucFlags, long lTime,
											long l_pDispatch);

		// Handles alloc callbacks from filter.
		UCHAR* AllocCall(long lSize, USHORT usType, UCHAR ucFlags);
		// Callback dispatcher (calls the implied this version).
		static UCHAR* AllocCallStatic(	long lSize, 
													USHORT usType, UCHAR ucFlags, 
													long l_pDispatch);

		// Handles free callbacks from filter.
		void FreeCall(UCHAR* puc, USHORT usType, UCHAR ucFlags);
		// Callback dispatcher (calls the implied this version).
		static void FreeCallStatic(	UCHAR* puc, USHORT usType, UCHAR ucFlags, 
												long l_pDispatch);


		// Called via BlowStatic once Start()'ed.  This blows chunks
		// at handlers at the chunks' specified time.
		void Blow(void);

		// Called by Blue critical once Start()'ed.  Passes control
		// to implied this Blow().
		static void BlowStatic(long l_pDispatch);

	public:		// Members.

	protected:	// Members.
		ALLOC_DISPATCHFUNC	m_afnAlloc[NUM_TYPES];	// User defined allocation
																	// functions.
		FREE_DISPATCHFUNC		m_afnFree[NUM_TYPES];	// User defined 
																	// de-allocation functions.
		USE_DISPATCHFUNC		m_afnUse[NUM_TYPES];		// User function to be
																	// passed data functions.
		MSG_DISPATCHFUNC		m_afnMsg[NUM_TYPES];		// User function that
																	// receives messages per-
																	// tinent to handlers.
		long						m_alUser[NUM_TYPES];		// User defined values.
		// To speed up the following list accesses we could use a hash table
		// of lists (like in CRes).
		CSList <RTITEM, long>	m_slistRtItems;		// Sorted list of items 
																	// waiting for their time
																	// to be dispatched.

		CFilter*					m_pfilter;					// CFilter.

		TIME_DISPATCHFUNC		m_fnTime;					// Time function.
		long						m_lTimeUser;				// User time value sent to 
																	// user time function.

		short						m_sActive;					// TRUE if critical is 
																	// active, FALSE otherwise.
		
	};


#endif // DISPATCH_H
//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
