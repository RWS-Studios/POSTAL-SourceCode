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
typedef uint8_t* (*ALLOC_DISPATCHFUNC)(	int32_t lSize, uint16_t usType, uint8_t ucFlags,
													int32_t lUser);

// This type is used to call the user to allow them to DEallocate space al-
// located by a previous call to their ALLOC_DISPATCHFUNC.
typedef void (*FREE_DISPATCHFUNC)(	uint8_t* puc, uint16_t usType, uint8_t ucFlags,
												int32_t lUser);

// This type is used to pass the copied chunk to the user ready to be used.
// Returning RET_FREE will cause puc to get freed if it meets the criteria
// for freeing described in the dispatch.cpp header comment.  Return
// RET_DONTFREE to avoid this.
typedef short (*USE_DISPATCHFUNC)(	uint8_t* puc, int32_t lSize, uint16_t usType, 
												uint8_t ucFlags, 
												int32_t lTime, int32_t lUser);

// This type is used to pass messages to the handler.
typedef short (*MSG_DISPATCHFUNC)(uint16_t usMsg);

// For custom time handlers.
typedef long (*TIME_DISPATCHFUNC)(int32_t lTimeUser);

class CDispatch
	{
	protected:	// Local types.
		// Items waiting to be dispatched.
		typedef struct
			{
			uint8_t*	puc;		// Data.
			int32_t		lSize;	// Size of data in bytes.
			uint16_t	usType;	// Type of data.
			uint8_t		ucFlags;	// Flags for data.
			int32_t		lTime;	// Time data is to be dispatched.
			} RTITEM, *PRTITEM;
		

	public:		// Construction/Destruction.
		// Default constructor.
		CDispatch();
		// Destructor.
		~CDispatch();

	public:		// Methods.
		// Sets the data handler for usType to fnUse.
		void SetDataHandler(uint16_t usType, USE_DISPATCHFUNC fnUse);

		// Sets the type handler for usType to fnAlloc.
		void SetAllocHandler(uint16_t usType, ALLOC_DISPATCHFUNC fnAlloc);

		// Sets the type handler for usType to fnUse.
		void SetFreeHandler(uint16_t usType, FREE_DISPATCHFUNC fnFree);

		// Sets the user value for usType to lUser.
		void SetUserVal(uint16_t usType, int32_t lUser);

		// Set filter.
		void SetFilter(CFilter* pfilter);

		// Start spewing/blowing chunks.
		// Returns 0 on success.
		int16_t Start(void);
		// Stop spewing/blowing chunks.
		// Returns 0 on success.
		int16_t Suspend(void);

		// Sets the time handler function.
		void SetTimeFunc(TIME_DISPATCHFUNC fnTime, int32_t lTimeUser)
			{ m_fnTime	= fnTime; m_lTimeUser = lTimeUser; }

		// Sends a message to all type handlers.
		// Returns the number of handlers that returned an error.
		int16_t SendHandlerMessage(uint16_t usMsg);

		// Adds an item to the list of items to be dispatched.
		// Returns 0 on success.
		int16_t AddItem(	uint8_t* puc, int32_t lSize, uint16_t usType, uint8_t ucFlags, 
							int32_t lTime);

	public:		// Querries.
		// Returns the time from the override function if set or, if not set, from
		// Blue.
		int32_t GetTime(void)
			{ return (m_fnTime != NULL ? (*m_fnTime)(m_lTimeUser) : Blu_GetTime()); }

		// Returns TRUE if critical handler is Blue's critical list.
		int16_t IsActive(void)
			{ return m_sActive; }

		// Returns TRUE if there are any chunks to be dispatched in the lists.
		int16_t IsEmpty(void)
			{ return m_slistRtItems.IsEmpty(); }

	protected:	// Internal methods.

		// Sets members w/o regard for current value.
		void Set(void);
		// Resets members. Deallocates memory if necessary.
		void Reset(void);

		// Handles data callbacks from filter.
		void UseCall(	uint8_t* puc, int32_t lSize, uint16_t usType, uint8_t ucFlags, 
							int32_t lTime);
		// Callback dispatcher (calls the implied this version).
		static void UseCallStatic(	uint8_t* puc, int32_t lSize, uint16_t usType, 
											uint8_t ucFlags, int32_t lTime,
											int32_t l_pDispatch);

		// Handles alloc callbacks from filter.
		uint8_t* AllocCall(int32_t lSize, uint16_t usType, uint8_t ucFlags);
		// Callback dispatcher (calls the implied this version).
		static uint8_t* AllocCallStatic(	int32_t lSize, 
													uint16_t usType, uint8_t ucFlags, 
													int32_t l_pDispatch);

		// Handles free callbacks from filter.
		void FreeCall(uint8_t* puc, uint16_t usType, uint8_t ucFlags);
		// Callback dispatcher (calls the implied this version).
		static void FreeCallStatic(	uint8_t* puc, uint16_t usType, uint8_t ucFlags, 
												int32_t l_pDispatch);


		// Called via BlowStatic once Start()'ed.  This blows chunks
		// at handlers at the chunks' specified time.
		void Blow(void);

		// Called by Blue critical once Start()'ed.  Passes control
		// to implied this Blow().
		static void BlowStatic(int32_t l_pDispatch);

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
		int32_t						m_alUser[NUM_TYPES];		// User defined values.
		// To speed up the following list accesses we could use a hash table
		// of lists (like in CRes).
		CSList <RTITEM, int32_t>	m_slistRtItems;		// Sorted list of items 
																	// waiting for their time
																	// to be dispatched.

		CFilter*					m_pfilter;					// CFilter.

		TIME_DISPATCHFUNC		m_fnTime;					// Time function.
		int32_t						m_lTimeUser;				// User time value sent to 
																	// user time function.

		int16_t						m_sActive;					// TRUE if critical is 
																	// active, FALSE otherwise.
		
	};


#endif // DISPATCH_H
//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
