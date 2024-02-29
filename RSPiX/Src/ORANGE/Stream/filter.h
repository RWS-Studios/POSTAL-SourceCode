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
#ifndef FILTER_H
#define FILTER_H

//////////////////////////////////////////////////////////////////////////////
// Headers.
//////////////////////////////////////////////////////////////////////////////
#include "filewin.h"
#include "list.h"

//////////////////////////////////////////////////////////////////////////////
// Macros.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Typedefs.
//////////////////////////////////////////////////////////////////////////////

// Encapsulate our buffer and info.
typedef struct
	{
	UCHAR*	puc;			// Beginning of chunk.
	long		lSize;		// Total size of chunk (puc).
	USHORT	usType;		// Type of buffer.
	UCHAR		ucFlags;		// Flags for buffer.
	long		lId;			// Id of buffer.
	long		lTime;		// Time buffer is supposed to arrive.
	long		lPos;			// Position for next piece.
	} RTCHUNK, *PRTCHUNK;

// This type is used to call the user to allow them to allocate space for the
// data and pass it back to be filled.
typedef UCHAR* (*ALLOC_FILTERFUNC)(	long lSize, USHORT usType, UCHAR ucFlags,
												long lUser);

// This type is used to call the user to allow them to DEallocate space al-
// located by a previous call to their ALLOC_FILTERFUNC.
typedef void (*FREE_FILTERFUNC)(	UCHAR* puc, USHORT usType, UCHAR ucFlags, 
											long lUser);

// This type is used to pass the copied chunk to the user ready to be used.
typedef void (*USE_FILTERFUNC)(	UCHAR* puc, long lSize, USHORT usType,
											UCHAR ucFlags, long lTime, long lUser);

class CFilter
	{
	public:		// Construction/Destruction.
		// Default constructor.
		CFilter();
		// Destructor.
		~CFilter();

	public:		// Methods.
		void SetFilter(ULONG ulFilterMask)
			{ m_ulFilter = ulFilterMask; }

		void SetFileWin(CFileWin* pfw)
			{ 
			m_pfw = pfw; 
			if (pfw != NULL)
				{
				// Point callback at our CFileWin callback dispatcher (calls 
				// implied this version (WinCall)).
				m_pfw->m_call			= (FWFUNC)WinCallStatic;
				m_pfw->m_lUser			= (long)this;
				}
			}

		// Resets members. Deallocates memory if necessary.
		void Reset(void);

	public:		// Querries.
		// Returns the current filter mask.
		ULONG GetFilter(void)
			{ return m_ulFilter; }

	protected:	// Internal methods.

		// Sets members w/o regard for current value.
		void Set(void);
		// Handles callbacks from file window.
		void WinCall(PPANE ppane);
		
		// Callback dispatcher (calls the implied this version).
		static void WinCallStatic(PPANE ppane, CFilter* pFilter);


		// Returns ptr to chunk via lId, returns NULL if not found.
		PRTCHUNK GetChunk(long lId);

		// Add a chunk header.
		// Returns chunk on success, NULL otherwise.
		PRTCHUNK AddChunk(long lSize, USHORT usType, UCHAR ucFlags, long Id, 
								long lTime);

		// Removes a chunk header.
		// Returns 0 on success.
		short RemoveChunk(PRTCHUNK pChunk);

		// Adds a buffer to a chunk.
		// Returns amount added.
		long AddToChunk(CNFile* pfile, long lBufSize);

		// Allocates data via user callback if defined or malloc, otherwise.
		// Returns 0 on success.  See comment of this function in filter.cpp
		// for greater details.
		short AllocChunk(UCHAR** ppuc, long lSize, USHORT usType, UCHAR ucFlags);
		// Deallocates data via user callback if defined or free if both 
		// m_fnAlloc AND m_fnFree are NOT defined.
		void FreeChunk(UCHAR* puc, USHORT usType, UCHAR ucFlags);


	public:		// Members.
		ALLOC_FILTERFUNC	m_fnAlloc;		// Where to ask for data allocation.
		FREE_FILTERFUNC	m_fnFree;		// Where to ask for data DEallocation.
		USE_FILTERFUNC		m_fnUse;			// Where to pass completed chunks.
		long					m_lUser;			// User defined value.


	protected:	// Members.
		ULONG			m_ulFilter;				// Channels allowed to pass.
		long			m_lPadSize;				// Size of current padding.
		long			m_lBufRemaining;		// Amount of current buffer remaining.
		PRTCHUNK		m_pChunk;				// Current chunk.
		CFileWin*	m_pfw;					// File window.
		CList	<RTCHUNK>	m_listPartial;	// List of partial buffers.
	};


#endif // FILTER_H
//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
