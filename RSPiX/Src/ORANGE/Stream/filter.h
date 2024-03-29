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
	uint8_t*	puc;			// Beginning of chunk.
	int32_t		lSize;		// Total size of chunk (puc).
	uint16_t	usType;		// Type of buffer.
	uint8_t		ucFlags;		// Flags for buffer.
	int32_t		lId;			// Id of buffer.
	int32_t		lTime;		// Time buffer is supposed to arrive.
	int32_t		lPos;			// Position for next piece.
	} RTCHUNK, *PRTCHUNK;

// This type is used to call the user to allow them to allocate space for the
// data and pass it back to be filled.
typedef uint8_t* (*ALLOC_FILTERFUNC)(	int32_t lSize, uint16_t usType, uint8_t ucFlags,
												int32_t lUser);

// This type is used to call the user to allow them to DEallocate space al-
// located by a previous call to their ALLOC_FILTERFUNC.
typedef void (*FREE_FILTERFUNC)(	uint8_t* puc, uint16_t usType, uint8_t ucFlags, 
											int32_t lUser);

// This type is used to pass the copied chunk to the user ready to be used.
typedef void (*USE_FILTERFUNC)(	uint8_t* puc, int32_t lSize, uint16_t usType,
											uint8_t ucFlags, int32_t lTime, int32_t lUser);

class CFilter
	{
	public:		// Construction/Destruction.
		// Default constructor.
		CFilter();
		// Destructor.
		~CFilter();

	public:		// Methods.
		void SetFilter(uint32_t ulFilterMask)
			{ m_ulFilter = ulFilterMask; }

		void SetFileWin(CFileWin* pfw)
			{ 
			m_pfw = pfw; 
			if (pfw != NULL)
				{
				// Point callback at our CFileWin callback dispatcher (calls 
				// implied this version (WinCall)).
				m_pfw->m_call			= (FWFUNC)WinCallStatic;
				m_pfw->m_lUser			= (int32_t)this;
				}
			}

		// Resets members. Deallocates memory if necessary.
		void Reset(void);

	public:		// Querries.
		// Returns the current filter mask.
		uint32_t GetFilter(void)
			{ return m_ulFilter; }

	protected:	// Internal methods.

		// Sets members w/o regard for current value.
		void Set(void);
		// Handles callbacks from file window.
		void WinCall(PPANE ppane);
		
		// Callback dispatcher (calls the implied this version).
		static void WinCallStatic(PPANE ppane, CFilter* pFilter);


		// Returns ptr to chunk via lId, returns NULL if not found.
		PRTCHUNK GetChunk(int32_t lId);

		// Add a chunk header.
		// Returns chunk on success, NULL otherwise.
		PRTCHUNK AddChunk(int32_t lSize, uint16_t usType, uint8_t ucFlags, int32_t Id, 
								int32_t lTime);

		// Removes a chunk header.
		// Returns 0 on success.
		int16_t RemoveChunk(PRTCHUNK pChunk);

		// Adds a buffer to a chunk.
		// Returns amount added.
		int32_t AddToChunk(CNFile* pfile, int32_t lBufSize);

		// Allocates data via user callback if defined or malloc, otherwise.
		// Returns 0 on success.  See comment of this function in filter.cpp
		// for greater details.
		int16_t AllocChunk(uint8_t** ppuc, int32_t lSize, uint16_t usType, uint8_t ucFlags);
		// Deallocates data via user callback if defined or free if both 
		// m_fnAlloc AND m_fnFree are NOT defined.
		void FreeChunk(uint8_t* puc, uint16_t usType, uint8_t ucFlags);


	public:		// Members.
		ALLOC_FILTERFUNC	m_fnAlloc;		// Where to ask for data allocation.
		FREE_FILTERFUNC	m_fnFree;		// Where to ask for data DEallocation.
		USE_FILTERFUNC		m_fnUse;			// Where to pass completed chunks.
		int32_t					m_lUser;			// User defined value.


	protected:	// Members.
		uint32_t			m_ulFilter;				// Channels allowed to pass.
		int32_t			m_lPadSize;				// Size of current padding.
		int32_t			m_lBufRemaining;		// Amount of current buffer remaining.
		PRTCHUNK		m_pChunk;				// Current chunk.
		CFileWin*	m_pfw;					// File window.
		CList	<RTCHUNK>	m_listPartial;	// List of partial buffers.
	};


#endif // FILTER_H
//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
