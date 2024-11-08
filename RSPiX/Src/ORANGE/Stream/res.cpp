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
// RES.CPP
// 
// History:
//		09/22/95 JMI	Started.
//
//		10/26/95	JMI	Added CNFile Open and Close hooking.
//
//////////////////////////////////////////////////////////////////////////////
//
// This class stores data identified by a string in a CResItem stored in 
// HASH_SIZE Clist <CResItem>'s.	 Use GetResource to get a resource and
// FreeResource to release it.
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// C Headers.
//////////////////////////////////////////////////////////////////////////////
#include <malloc.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////////////
// Blue Headers.
//////////////////////////////////////////////////////////////////////////////
#include "System.h"
#include "bdebug.h"

//////////////////////////////////////////////////////////////////////////////
// Green Headers.
//////////////////////////////////////////////////////////////////////////////
#include "rttypes.h"
#include "res.h"

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
CList<CRes>	CRes::ms_listRes;					// List of all CRes objects.

//////////////////////////////////////////////////////////////////////////////
// Construction/Destruction Functions.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Default constructor.
//
//////////////////////////////////////////////////////////////////////////////
CRes::CRes()
	{
	Set();

	// If this is the first . . .
	if (ms_listRes.IsEmpty() == TRUE)
		{
		CNFile::SetOpenHook(FileOpenHook, 0L);
		CNFile::SetCloseHook(FileCloseHook, 0L);
		}
	
	// Add this to list of all CRes.
	if (ms_listRes.Add(this) == 0)
		{
		}
	else
		{
		TRACE("CRes(): Unable to add this to ms_listRes.\n");
		}
	}

//////////////////////////////////////////////////////////////////////////////
//
// Destructor.
//
//////////////////////////////////////////////////////////////////////////////
CRes::~CRes()
	{
	Reset();

	// Remove this from list of all CRes.
	if (ms_listRes.Remove(this) == 0)
		{
		}
	else
		{
		TRACE("CRes(): Unable to remove this from ms_listRes.\n");
		}
	}

//////////////////////////////////////////////////////////////////////////////
// Internal Functions.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Returns hash value for psz.
//
//////////////////////////////////////////////////////////////////////////////
int32_t HashFunc(char* psz)
	{
	int32_t lHash	= 0L;
	
	while (*psz != '\0')
		{
		// Add all characters.
		lHash += (int32_t)*psz;

		psz++;
		}

	return (lHash & (HASH_SIZE-1));
	}

//////////////////////////////////////////////////////////////////////////////
//
// Sets variables w/o regard to current values.
//
//////////////////////////////////////////////////////////////////////////////
void CRes::Set(void)
	{
	m_pDispatch		= NULL;
	m_sFreeOnClose	= TRUE;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Resets variables.  Performs deallocation if necessary.
//
//////////////////////////////////////////////////////////////////////////////
void CRes::Reset(void)
	{
	#ifdef _DEBUG
		// All lists should be empty.
		for (int16_t i = 0; i < HASH_SIZE; i++)
			ASSERT(m_alistRes[i].IsEmpty() == TRUE);
	#endif // _DEBUG

	// Free any remaining resources and their nodes.
	FreeAll();
	}

//////////////////////////////////////////////////////////////////////////////
//
// Handles data callbacks from dispatch.
//
//////////////////////////////////////////////////////////////////////////////
int16_t CRes::UseCall(	uint8_t* puc, int32_t lSize, uint16_t usType, uint8_t ucFlags, 
							int32_t lTime)
	{
	int16_t	sRes		= RET_DONTFREE;	// Assume success.
	int16_t	sError	= 0;

	ASSERT(usType	== RT_TYPE_FILEIMAGE);
	ASSERT(puc		!= NULL);
	
	char*		pszName	= (char*)puc;
	int32_t		lLen		= strlen(pszName) + 1;

	// Allocate a CResItem.
	PRESITEM	pri	= new CResItem(pszName, puc + lLen, lSize - lLen, this);
	if (pri != NULL)
		{
		// Check for duplicate resource names.
		ASSERT(GetResItem(pszName) == NULL);

		// Get hash value.
		int32_t	lHash	= HashFunc(pszName);

		ASSERT(lHash < HASH_SIZE);
		// Add item.
		if (m_alistRes[lHash].Add(pri) == 0)
			{
			// Success.
			}
		else
			{
			TRACE("UseCall(): Unable to add CResItem to res list.\n");
			sError = -2;
			}

		// If any errors occurred after CResItem allocation . . .	
		if (sError != 0)
			{
			delete pri;
			}
		}
	else
		{
		TRACE("UseCall(): Unable to allocate CResItem for %s.\n", pszName);
		sError = -1;
		}
	
	// If any errors occur . . .	
	if (sError != 0)
		{
		// We won't be needing this.
		sRes	= RET_FREE;
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Callback dispatcher (calls the implied this version).
// (static)
//
//////////////////////////////////////////////////////////////////////////////
int16_t CRes::UseCallStatic(	uint8_t* puc, int32_t lSize, uint16_t usType, 
									uint8_t ucFlags, int32_t lTime, int32_t l_pRes)
	{
	return ((CRes*)l_pRes)->UseCall(puc, lSize, usType, ucFlags, lTime);
	}

//////////////////////////////////////////////////////////////////////////////
//
// Handles alloc callbacks from dispatch.
//
//////////////////////////////////////////////////////////////////////////////
uint8_t* CRes::AllocCall(int32_t lSize, uint16_t usType, uint8_t ucFlags)
	{
	ASSERT(usType == RT_TYPE_FILEIMAGE);

	uint8_t*	puc	= (uint8_t*)malloc(lSize);

	if (puc != NULL)
		{
		// Success.
		}
	else
		{
		TRACE("AllocCall(): Failed to allocate %ld bytes for resource.\n",
				lSize);
		}	

	return puc;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Callback dispatcher (calls the implied this version).
// (static)
//
//////////////////////////////////////////////////////////////////////////////
uint8_t* CRes::AllocCallStatic(	int32_t lSize, uint16_t usType, uint8_t ucFlags,
										int32_t l_pRes)
	{
	return ((CRes*)l_pRes)->AllocCall(lSize, usType, ucFlags);
	}

//////////////////////////////////////////////////////////////////////////////
//
// Handles free callbacks from filter.
//
//////////////////////////////////////////////////////////////////////////////
void CRes::FreeCall(uint8_t* puc, uint16_t usType, uint8_t ucFlags)
	{
	ASSERT(usType	== RT_TYPE_FILEIMAGE);
	ASSERT(puc		!= NULL);

	free(puc);
	}

//////////////////////////////////////////////////////////////////////////////
//
// Callback dispatcher (calls the implied this version).
// (static)
//
//////////////////////////////////////////////////////////////////////////////
void CRes::FreeCallStatic(	uint8_t* puc, uint16_t usType, uint8_t ucFlags, 
									int32_t l_pRes)
	{
	((CRes*)l_pRes)->FreeCall(puc, usType, ucFlags);
	}

//////////////////////////////////////////////////////////////////////////////
//
// Returns a resource item based on it's hash value and .
//
//////////////////////////////////////////////////////////////////////////////
PRESITEM	CRes::GetResItem(char* pszName)
	{
	int32_t lHash		= HashFunc(pszName);

	PRESITEM	pri	= m_alistRes[lHash].GetHead();

	while (pri != NULL)
		{
		if (strcmp(pri->m_pszName, pszName) == 0)
			{
			break;
			}

		pri	= m_alistRes[lHash].GetNext();
		}

	return pri;
	}

//////////////////////////////////////////////////////////////////////////////
// Methods.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Set filter.
//
//////////////////////////////////////////////////////////////////////////////
void CRes::SetDispatcher(CDispatch* pDispatch)
	{
	if (m_pDispatch != NULL)
		{
		m_pDispatch->SetDataHandler(RT_TYPE_FILEIMAGE, NULL);
		m_pDispatch->SetAllocHandler(RT_TYPE_FILEIMAGE, NULL);
		m_pDispatch->SetFreeHandler(RT_TYPE_FILEIMAGE, NULL);
		}

	m_pDispatch	= pDispatch;

	if (m_pDispatch != NULL)
		{
		m_pDispatch->SetDataHandler(	RT_TYPE_FILEIMAGE, UseCallStatic);
		m_pDispatch->SetAllocHandler(	RT_TYPE_FILEIMAGE, AllocCallStatic);
		m_pDispatch->SetFreeHandler(	RT_TYPE_FILEIMAGE, FreeCallStatic);
		m_pDispatch->SetUserVal(RT_TYPE_FILEIMAGE, (int32_t)this);
		}
	}

//////////////////////////////////////////////////////////////////////////////
//
// Retrieves the resource identified by pszName.
// Returns ptr to CResItem on succes, NULL otherwise.
//
//////////////////////////////////////////////////////////////////////////////
PRESITEM CRes::GetResource(char* pszName)
	{
	PRESITEM	pri	= GetResItem(pszName);

	if (pri != NULL)
		{
		// Lock resource.
		pri->Lock();
		}

	return pri;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Releases a resource.
//
//////////////////////////////////////////////////////////////////////////////
void CRes::FreeResource(PRESITEM pri)
	{
	ASSERT(GetResItem(pri->m_pszName) != NULL);

	// Unlock.  If no more references to this object . . .
	if (pri->Unlock() == 0)
		{
		// Remove from list.
		int32_t	lHash	= HashFunc(pri->m_pszName);
		m_alistRes[lHash].Remove(pri);

		// Hasta.
		delete pri;
		}
	}

//////////////////////////////////////////////////////////////////////////////
//
// Free all remaining resources and nodes.
//
//////////////////////////////////////////////////////////////////////////////
void CRes::FreeAll(void)
	{
	PRESITEM	pri;
	
	for (int16_t i = 0; i < HASH_SIZE; i++)
		{
		pri = m_alistRes[i].GetHead();
		while (pri != NULL)
			{
			// Remove from list.
			m_alistRes[i].Remove();
			// Deallocate.
			delete pri;
			// Get next item.
			pri = m_alistRes[i].GetNext();
			}
		}
	}

//////////////////////////////////////////////////////////////////////////////
//
// Hooks calls to CNFile's file Open (NOT memory opens).
// Returns 0 if file found, positive if not, negative on error.
// (static)
//
//////////////////////////////////////////////////////////////////////////////
int16_t CRes::FileOpenHook(	CNFile* pfile, char* pszFileName, 
									char* pszFlags, int16_t sEndian, int32_t lUser)
	{
	int16_t	sRes	= 1;	// Assume we fail to find file.

	CRes*	pres	= ms_listRes.GetHead();
	while (pres != NULL && sRes > 0)
		{
		// Attempt to get file from resource manager.
		PRESITEM	pri	= pres->GetResource(pszFileName);
		if (pri != NULL)
			{
			// Open the file.
			if (pfile->Open(pri->m_puc, pri->m_lSize, sEndian) == 0)
				{
				// Set the user value to the ptr to the res item so we know which
				// one to free on close.
				pfile->SetUserVal((int32_t)pri);
				// Success.
				sRes = 0;
				}
			else
				{
				TRACE("FileOpenHook(): Failed to open memory file.\n");
				sRes = -1;
				}

			// If any errors occurred after we got the resource . . .
			if (sRes != 0)
				{
				pres->FreeResource(pri);
				}
			}
		else
			{
			sRes = 1;
			}

		// Get next resource manager.
		pres = ms_listRes.GetNext();
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Hooks calls to CNFile's Close.
// Returns 0 if close was taken care of by this hook.
// (static)
//
//////////////////////////////////////////////////////////////////////////////
int16_t CRes::FileCloseHook(CNFile* pfile, int32_t lUser)
	{
	int16_t	sRes	= 0;	// Assume we find the file.

	// Get user value (which is the pointer to the CResItem).
	PRESITEM	pri	= (PRESITEM)pfile->GetUserVal();
	if (pri != NULL)
		{
		CRes*		pres	= pri->m_pRes;
		#ifdef _DEBUG
			// Verify it's a valid one.
			pres	= ms_listRes.GetHead();
			while (pres != NULL)
				{
				if (pres == pri->m_pRes)
					{
					break;
					}
				pres = ms_listRes.GetNext();
				}
			// We should have found the CRes that matched pri's.
			ASSERT(pres != NULL);
		#endif // _DEBUG

		// Close the file.
		if (pfile->Close() == 0)
			{
			// Clear the user value.
			pfile->SetUserVal(0);
			}
		else
			{
			TRACE("FileCloseHook(): Failed to close memory file.\n");
			sRes = -1;
			}
		
		if (pres->m_sFreeOnClose == TRUE)
			{
			// Release the resource.
			pres->FreeResource(pri);
			}
		else
			{
			// Unlock the resource.
			pri->Unlock();
			}
		}
	else
		{
		sRes = 1;
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
