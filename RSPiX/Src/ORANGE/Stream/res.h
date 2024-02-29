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
#ifndef RES_H
#define RES_H

//////////////////////////////////////////////////////////////////////////////
// Headers.
//////////////////////////////////////////////////////////////////////////////
#include "dispatch.h"
#include "resitem.h"
#include "list.h"
					  
//////////////////////////////////////////////////////////////////////////////
// Macros.
//////////////////////////////////////////////////////////////////////////////
// Size of hash range.  To get an idea of overhead for this value: multiply
// HASH_SIZE * sizeof(CList <CResItem>) (currently 20 bytes for an empty list).
// Currently the hash function is dependent on the HASH_SIZE being 256.  To
// change this value, YOU MUST REDEFINE THE HASH FUNCTION.
#define HASH_SIZE		256

//////////////////////////////////////////////////////////////////////////////
// Typedefs.
//////////////////////////////////////////////////////////////////////////////

class CRes
	{
	public:		// Construction/Destruction.
		// Default constructor.
		CRes();
		// Destructor.
		~CRes();

	public:		// Methods.
		// Set filter.
		void SetDispatcher(CDispatch* pDispatch);

		// Free all remaining resources and nodes.
		void FreeAll(void);

		// If sFreeOnClose is TRUE, resources opened by the open hook
		// will be freed by the close hook if they are not additionally locked.
		void SetFreeOnClose(short sFreeOnClose)
			{ m_sFreeOnClose	= sFreeOnClose; }

	public:		// Querries.
		// Retrieves the resource identified by pszName.
		// Returns ptr to CResItem on succes, NULL otherwise.
		PRESITEM GetResource(char* pszName);

		// Releases a resource.
		void FreeResource(PRESITEM pri);

	protected:	// Internal methods.

		// Sets members w/o regard for current value.
		void Set(void);
		// Resets members. Deallocates memory if necessary.
		void Reset(void);

		// Returns a resource item based on it's hash value.
		PRESITEM	GetResItem(char* pszName);

		// Handles data callbacks from dispatch.
		short UseCall(	UCHAR* puc, long lSize, USHORT usType, UCHAR ucFlags,
							long lTime);
		// Callback dispatcher (calls the implied this version).
		static short UseCallStatic(UCHAR* puc, long lSize, USHORT usType, 
											UCHAR ucFlags,
											long lTime, long l_pRes);

		// Handles alloc callbacks from dispatch.
		UCHAR* AllocCall(long lSize, USHORT usType, UCHAR ucFlags);
		// Callback dispatcher (calls the implied this version).
		static UCHAR* AllocCallStatic(long lSize, 
												USHORT usType, UCHAR ucFlags, 
												long l_pRes);

		// Handles free callbacks from filter.
		void FreeCall(UCHAR* puc, USHORT usType, UCHAR ucFlags);
		// Callback dispatcher (calls the implied this version).
		static void FreeCallStatic(UCHAR* puc, USHORT usType, UCHAR ucFlags,
											long l_pRes);

		// Hooks calls to CNFile's file Open (NOT memory opens).
		// Returns 0 if file found.
		static short FileOpenHook(	CNFile* pfile, char* pszFileName, 
											char* pszFlags, short sEndian, long lUser);
		// Hooks calls to CNFile's Close.
		// Returns 0 if close was taken care of by this hook.
		static short FileCloseHook(CNFile* pfile, long lUser);

	public:		// Members.

	protected:	// Members.
		CDispatch*				m_pDispatch;				// CDispatch.
		CList	<CResItem>		m_alistRes[HASH_SIZE];	// Uh..De ja vus.
		short						m_sFreeOnClose;			// If TRUE, the close hook
																	// will free the resource
																	// if it's not locked.

		// Static members.
		static CList<CRes>	ms_listRes;					// List of all CRes objects.
		
	};


#endif // RES_H
//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
