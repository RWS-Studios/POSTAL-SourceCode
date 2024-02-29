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
// Dispatch.CPP
// 
// History:
//		09/21/95 JMI	Started.
//
//////////////////////////////////////////////////////////////////////////////
//
// The user can optionally provide space for the data to be copied into 
// through the use of an ALLOC_DISPATCHFUNC (m_fnAlloc).  If no func is 
// provided, the data space is allocated at the exact size via malloc.  
// Whether or not an ALLOC_DISPATCHFUNC is provided the user is responsible 
// for freeing the buffers (they are not tracked!) (use malloc's free() when 
// no ALLOC_DISPATCHFUNC is provided).  By not managing the data allocation, 
// we allow the user ultimate flexibility in how their data is stored.
// This class will NOT use malloc for a type that has no USE_DISPATCHFUNC;
// otherwise would be silly.
//
// IMPORTANT:	If you provide an ALLOC_DISPATCHFUNC, you SHOULD provide a 
// FREE_DISPATCHFUNC to deallocate a buffer.  This module will attempt to free
// data if an error occurs causing a buffer to become only partially filled.
// If no ALLOC_DISPATCHFUNC is provided, it will use free (since it allocated 
// it with malloc).  If an ALLOC_DISPATCHFUNC was provided AND a 
// FREE_DISPATCHFUNC was provided, the FREE_DISPATCHFUNC will be called in this
// case to de-allocate or otherwise handle the buffer.
// Please note that if you do provide a ALLOC_DISPATCHFUNC and don't provide a 
// FREE_DISPATCHFUNC, this module will NOT use malloc's free!
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
#include "dispatch.h"

//////////////////////////////////////////////////////////////////////////////
// Orange Headers.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Yellow Headers.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Module specific macros.
//////////////////////////////////////////////////////////////////////////////

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
CDispatch::CDispatch()
	{
	Set();
	}

//////////////////////////////////////////////////////////////////////////////
//
// Destructor.
//
//////////////////////////////////////////////////////////////////////////////
CDispatch::~CDispatch()
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
void CDispatch::Set(void)
	{
	for (long l = 0L; l < NUM_TYPES; l++)
		{
		m_afnAlloc[l]	= NULL;
		m_afnFree[l]	= NULL;
		m_afnUse[l]		= NULL;
		}

	m_fnTime		= NULL;
	m_pfilter	= NULL;

	m_sActive	= FALSE;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Resets variables.  Performs deallocation if necessary.
//
//////////////////////////////////////////////////////////////////////////////
void CDispatch::Reset(void)
	{
	ASSERT(m_sActive == FALSE);

	Suspend();

	// Free items waiting to be dispatched.
	PRTITEM	pri	= m_slistRtItems.GetHead();
	while (pri != NULL)
		{
		// Free buffer.
		FreeCall(pri->puc, pri->usType, pri->ucFlags);
		// Remove item from list.
		m_slistRtItems.Remove();
		// Destroy item container.
		delete pri;
		// Get Next.
		pri	= m_slistRtItems.GetNext();
		}

	Set();
	}

//////////////////////////////////////////////////////////////////////////////
//
// Handles data callbacks from filter.
//
//////////////////////////////////////////////////////////////////////////////
void CDispatch::UseCall(UCHAR* pucBuffer, long lSize, USHORT usType, 
								UCHAR ucFlags, long lTime)
	{
	// If data handled . . .
	if (m_afnUse[usType] != NULL)
		{
		if (AddItem(pucBuffer, lSize, usType, ucFlags, lTime) == 0)
			{
			// Success.
			}
		else
			{
			TRACE("UseCall(): AddItem failed.  Discarding data.\n");
			FreeCall(pucBuffer, usType, ucFlags);
			}
		}
	else
		{
		// Useless, discard.
		FreeCall(pucBuffer, usType, ucFlags);
		}
	}

//////////////////////////////////////////////////////////////////////////////
//
// Callback dispatcher (calls the implied this version).
// (static)
//
//////////////////////////////////////////////////////////////////////////////
void CDispatch::UseCallStatic(UCHAR* pucBuffer, long lSize, USHORT usType, 
										UCHAR ucFlags, long lTime, long l_pDispatch)
	{
	((CDispatch*)l_pDispatch)->UseCall(pucBuffer, lSize, usType, ucFlags, lTime);
	}

//////////////////////////////////////////////////////////////////////////////
//
// Handles alloc callbacks from filter.
//
//////////////////////////////////////////////////////////////////////////////
UCHAR* CDispatch::AllocCall(long lSize, USHORT usType, UCHAR ucFlags)
	{
	UCHAR*	puc	= NULL;

	if (m_afnAlloc[usType] != NULL)
		{
		puc = (*m_afnAlloc[usType])(lSize, usType, ucFlags, m_alUser[usType]);
		}
	else
		{
		// Only allocate if there is a use handler (otherwise, data is useless).
		if (m_afnUse[usType] != NULL)
			{
			puc = (UCHAR*)malloc(lSize);
			}
		else
			{
			TRACE("AllocCall(lSize:%ld, usType:%02X, ucFlags:%02X): No use "
					"handler for this data.\n", lSize, usType, ucFlags);
			}
		}

	if (puc != NULL)
		{
		// Success.
		}
	else
		{
		TRACE("AllocCall(lSize:%ld, usType:%02X, ucFlags:%02X): Unable to allocate "
				"buffer.\n", lSize, usType, ucFlags);
		}

	return puc;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Callback dispatcher (calls the implied this version).
// (static)
//
//////////////////////////////////////////////////////////////////////////////
UCHAR* CDispatch::AllocCallStatic(	long lSize, 
												USHORT usType, UCHAR ucFlags,  
												long l_pDispatch)
	{
	return ((CDispatch*)l_pDispatch)->AllocCall(lSize, usType, ucFlags);
	}

//////////////////////////////////////////////////////////////////////////////
//
// Handles free callbacks from filter.
//
//////////////////////////////////////////////////////////////////////////////
void CDispatch::FreeCall(UCHAR* puc, USHORT usType, UCHAR ucFlags)
	{
	if (puc != NULL)
		{
		// If an allocation function is defined . . .
		if (m_afnAlloc[usType] != NULL)
			{
			// If a deallocation function is defined . . .
			if (m_afnFree[usType] != NULL)
				{
				// Call it.
				(*m_afnFree[usType])(puc, usType, ucFlags, m_alUser[usType]);
				}
			}
		else
			{
			free(puc);
			}
		}
	}

//////////////////////////////////////////////////////////////////////////////
//
// Callback dispatcher (calls the implied this version).
// (static)
//
//////////////////////////////////////////////////////////////////////////////
void CDispatch::FreeCallStatic(	UCHAR* pucBuffer, USHORT usType, 
											UCHAR ucFlags, long l_pDispatch)
	{
	((CDispatch*)l_pDispatch)->FreeCall(pucBuffer, usType, ucFlags);
	}

//////////////////////////////////////////////////////////////////////////////
//
// Adds an item to the list of items to be dispatched.
// Returns 0 on success.
//
//////////////////////////////////////////////////////////////////////////////
short CDispatch::AddItem(	UCHAR* puc, long lSize, USHORT usType, 
									UCHAR ucFlags, long lTime)
	{
	short	sRes	= 0;	// Assume success.

	// Attempt to allocate a RTITEM for this chunk.
	PRTITEM	pri	= new RTITEM;

	// If successful . . .
	if (pri != NULL)
		{
		// Set up item.
		pri->puc			= puc;
		pri->lSize		= lSize;
		pri->usType		= usType;
		pri->ucFlags	= ucFlags;
		pri->lTime		= lTime;
		// Add item to sorted list with time as sort key . . .
		if (m_slistRtItems.Insert(pri, &(pri->lTime)) == 0)
			{
			// Success.
			}
		else
			{
			TRACE("AddItem(): Unable to add RTITEM to list.\n");
			sRes = -2;
			}

		// If any erros occurred since allocation . . .
		if (sRes != 0)
			{
			delete pri;
			}
		}
	else
		{
		TRACE("AddItem(): Unable to allocate new RTITEM.\n");
		sRes = -1;
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Called via BlowStatic once Start()'ed.  This blows chunks
// at handlers at the chunks' specified time.
//
//////////////////////////////////////////////////////////////////////////////
void CDispatch::Blow(void)
	{
	// Get the lowest time (always the head, since it is sorted).
	PRTITEM	pri	= m_slistRtItems.GetHead();
	// Get the current time.
	long		lTime	= GetTime();

	// Do until none left or time exceeds current.
	while (pri != NULL && pri->lTime < lTime)
		{
		// Remove item from list.
		m_slistRtItems.Remove();

		// Call handler.  If handler doesn't need buffer anymore . . .
		if ((*m_afnUse[pri->usType])(	pri->puc, pri->lSize, pri->usType, 
												pri->ucFlags, pri->lTime,
												m_alUser[pri->usType]) == RET_FREE)
			{
			FreeCall(pri->puc, pri->usType, pri->ucFlags);
			}

		// Destroy item container.
		delete pri;

		// Get next node.
		pri	= m_slistRtItems.GetHead();

		// Get the "current" time in case handler was slow.
		lTime = GetTime();
		}
	}

//////////////////////////////////////////////////////////////////////////////
//
// Called by Blue critical once Start()'ed.  Passes control
// to implied this Blow().
//
//////////////////////////////////////////////////////////////////////////////
void CDispatch::BlowStatic(long l_pDispatch)
	{
	((CDispatch*)l_pDispatch)->Blow();
	}

//////////////////////////////////////////////////////////////////////////////
// Public Functions.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Sets the type handler for usType to fnUse.
//
//////////////////////////////////////////////////////////////////////////////
void CDispatch::SetDataHandler(USHORT usType, USE_DISPATCHFUNC fnUse)
	{
	ASSERT(usType < NUM_TYPES);

	m_afnUse[usType]	= fnUse;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Sets the data handler for usType to fnAlloc.
//
//////////////////////////////////////////////////////////////////////////////
void CDispatch::SetAllocHandler(USHORT usType, ALLOC_DISPATCHFUNC fnAlloc)
	{
	ASSERT(usType < NUM_TYPES);

	m_afnAlloc[usType]	= fnAlloc;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Sets the type handler for usType to fnFree.
//
//////////////////////////////////////////////////////////////////////////////
void CDispatch::SetFreeHandler(USHORT usType, FREE_DISPATCHFUNC fnFree)
	{
	ASSERT(usType < NUM_TYPES);

	m_afnFree[usType]	= fnFree;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Sets the user value for usType to lUser.
//
//////////////////////////////////////////////////////////////////////////////
void CDispatch::SetUserVal(USHORT usType, long lUser)
	{
	ASSERT(usType < NUM_TYPES);

	m_alUser[usType]	= lUser;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Set filter.
//
//////////////////////////////////////////////////////////////////////////////
void CDispatch::SetFilter(CFilter* pfilter)
	{
	if (m_pfilter != NULL)
		{
		// Clear callbacks that point at our CFilter callback dispatchers.
		m_pfilter->m_fnAlloc	= NULL;
		m_pfilter->m_fnFree	= NULL;
		m_pfilter->m_fnUse	= NULL;
		}

	m_pfilter = pfilter;

	if (pfilter != NULL)
		{
		// Point callbacks at our CFilter callback dispatchers (calls implied this
		// version (Filter*Call)).
		m_pfilter->m_fnAlloc	= AllocCallStatic;
		m_pfilter->m_fnFree	= FreeCallStatic;
		m_pfilter->m_fnUse	= UseCallStatic;
		m_pfilter->m_lUser	= (long)this;
		}
	}

//////////////////////////////////////////////////////////////////////////////
//
// Start spewing/blowing chunks.
// Returns 0 on success.
//
//////////////////////////////////////////////////////////////////////////////
short CDispatch::Start(void)
	{
	short	sRes	= 0;	// Assume success.

	if (m_sActive == FALSE)
		{
		if (Blu_AddCritical((CRITICALL)BlowStatic, (long)this) == 0)
			{
			// Success.
			m_sActive = TRUE;
			}
		else
			{
			TRACE("Start(): Unable to add critical handler BlowStatic.\n");
			sRes = -1;
			}
		}
	
	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Stop spewing/blowing chunks.
// Returns 0 on success.
//
//////////////////////////////////////////////////////////////////////////////
short CDispatch::Suspend(void)
	{
	short	sRes	= 0;	// Assume success.

	if (m_sActive == TRUE)
		{
		if (Blu_RemoveCritical((CRITICALL)BlowStatic) == 0)
			{
			// Success.
			m_sActive = FALSE;
			}
		else
			{
			TRACE("Suspend(): Unable to remove critical handler BlowStatic.\n");
			sRes = -1;
			}
		}
	
	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Sends a message to all type handlers.
// Returns the number of handlers that returned an error.
//
//////////////////////////////////////////////////////////////////////////////
short CDispatch::SendHandlerMessage(USHORT usMsg)
	{
	short sNum	= 0;

	for (long l = 0L; l < NUM_TYPES; l++)
		{
		if (m_afnMsg[l] != NULL)
			{
			if ((*m_afnMsg[l])(usMsg) != 0)
				{
				// Unhappy handler.
				sNum++;
				}
			}
		}

	return sNum;
	}

//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
