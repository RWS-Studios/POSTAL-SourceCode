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
// Filter.CPP
// 
// History:
//		09/20/95 JMI	Started.
//
//		09/21/95	JMI	Moved the feature that concatenates buffers into one
//							chunk before calling the user func from CDispatch to
//							here to ease suffering.
//
//////////////////////////////////////////////////////////////////////////////
//
// Filters out channels that are not required.  Builds a contiguous chunk and
// gives that to the dispatcher.
//
// The user can optionally provide space for the data to be copied into 
// through the use of an ALLOC_FILTERFUNC (m_fnAlloc).  If no func is provided,
// the data space is allocated at the exact size via malloc.  Whether or not
// an ALLOC_FILTERFUNC is provided the user is responsible for freeing the 
// buffers (they are not tracked!) (use malloc's free() when no 
// ALLOC_FILTERFUNC is provided).  By not managing the data allocation, we 
// allow the user ultimate flexibility in how their data is stored.
//
// IMPORTANT:	If you provide an ALLOC_FILTERFUNC, you SHOULD provide a 
// FREE_FILTERFUNC to deallocate a buffer.  This module will attempt to free
// data if an error occurs causing a buffer to become only partially filled.
// If no ALLOC_FILTERFUNC is provided, it will use free (since it allocated 
// it with malloc).  If an ALLOC_FILTERFUNC was provided AND a FREE_FILTERFUNC
// was provided, the FREE_FILTERFUNC will be called in this case to 
// de-allocate or otherwise handle the buffer.
// Please note that if you do provide a ALLOC_FILTERFUNC and don't provide a 
// FREE_FILTERFUNC, this module will NOT use malloc's free!
// If an ALLOC_FILTERFUNC returns NULL, the process continues as normal.  This
// is so the user can choose to skip chunks or ignore allocation failures.  It
// is, therefore, up to the user to stop the streaming if necessary.
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

//////////////////////////////////////////////////////////////////////////////
// Green Headers.
//////////////////////////////////////////////////////////////////////////////
#include "filter.h"

//////////////////////////////////////////////////////////////////////////////
// Orange Headers.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Yellow Headers.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Module specific macros.
//////////////////////////////////////////////////////////////////////////////
#define HEADERSIZE		(sizeof(UCHAR)		/* Channel		*/	\
								+ sizeof(UCHAR)	/* Flags			*/	\
								+ sizeof(USHORT)	/* Type			*/	\
								+ sizeof(long)		/* ID				*/	\
								+ sizeof(long)		/* Buffer size	*/	\
								+ sizeof(long)		/* Chunk size	*/	\
								+ sizeof(long))	/* Time stamp	*/

//#define ALIGNTO(n,a)   (((n) + ((a)-1)) & ~((a)-1))
#define ALIGNTO(n,a)   ((((n) + ((a)-1)) / (a)) * (a))


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
CFilter::CFilter()
	{
	Set();
	}

//////////////////////////////////////////////////////////////////////////////
//
// Destructor.
//
//////////////////////////////////////////////////////////////////////////////
CFilter::~CFilter()
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
void CFilter::Set(void)
	{
	m_fnAlloc				= NULL;
	m_fnFree					= NULL;
	m_fnUse					= NULL;

	m_ulFilter				= 0;

	m_lPadSize				= 0L;
	m_lBufRemaining		= 0L;
	m_pChunk					= NULL;

	m_pfw						= NULL;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Resets variables.  Performs deallocation if necessary.
//
//////////////////////////////////////////////////////////////////////////////
void CFilter::Reset(void)
	{
	if (m_listPartial.IsEmpty() == FALSE)
		{
		TRACE("Reset(): There are partial buffers.  Deallocating.\n");
		PRTCHUNK	pChunk	= m_listPartial.GetHead();
		while (pChunk != NULL)
			{
			FreeChunk(pChunk->puc, pChunk->usType, pChunk->ucFlags);

			RemoveChunk(pChunk);

			pChunk	= m_listPartial.GetNext();
			}
		}

	Set();
	}

//////////////////////////////////////////////////////////////////////////////
//
// Handles callbacks from file window.  Does not pass chunks that don't match
// the filter (and are not global).  This function is very dependent on both
// the panes and the chunks being HEADERSIZE byte aligned.
// This function deals with TWO types of fragmentation:
//	1) File window pane fragmentation:
//		The current buffer may span past the current pane (lBufSize is the size
//		of the buffer, lTotalBufSize is the size of the buffer plus padding).
//
//	2) Chunk fragmentation:
//		The current buffer may be one of many buffers that make up the total
//		chunk (lChunkSize is the size of the chunk).
//
//////////////////////////////////////////////////////////////////////////////
void CFilter::WinCall(PPANE ppane)
	{
	short	sError	= 0;

	ASSERT(ppane->lSize		>= 0);
	// MUST be 4 byte aligned.
	ASSERT(ppane->lSize % HEADERSIZE	== 0);

	if (ppane->lSize > 0L)
		{
		// For endian swapping.
		CNFile file;
		if (file.Open(ppane->puc, ppane->lSize, ENDIAN_BIG) == 0)
			{
			UCHAR		ucChannel;
			USHORT	usType;
			UCHAR		ucFlags;
			long		lId;
			long		lBufSize;
			long		lChunkSize;
			long		lTime;
			long		lAmt;
		
			while (file.IsEOF() == FALSE && sError == 0)
				{
				ASSERT(m_lBufRemaining >= 0L);

				// If continuing buffer . . .
				if (m_lBufRemaining > 0L)
					{
					// If not being filtered out . . .
					if (m_pChunk != NULL)
						{
						// Read more buffer.
						lAmt = AddToChunk(&file, m_lBufRemaining);
						
						// Deduct amount read.
						m_lBufRemaining -= lAmt;
						// If buffer filled . . .
						if (m_lBufRemaining == 0)
							{
							// Seek past padding . . .
							if (file.Seek(m_lPadSize, SEEK_CUR) == 0)
								{
								// Success.
								}
							else
								{
								TRACE("WinCall(): Error seeking w/i buffer.\n");
								sError = 5;
								}
							}
						}
					else
						{
						lAmt	= MIN(m_lBufRemaining, ppane->lSize - file.Tell());
						// Seek past.
						if (file.Seek(lAmt, SEEK_CUR) == 0)
							{
							// Deduct amount seeked.
							m_lBufRemaining -= lAmt;
							}
						else
							{
							TRACE("WinCall(): Error seeking w/i buffer.\n");
							sError = 4;
							}
						}
					}
				else
					{
					// New buffer.
					file.Read(&ucChannel);
					file.Read(&ucFlags);
					file.Read(&usType);
					file.Read(&lId);
					file.Read(&lBufSize);
					file.Read(&lChunkSize);
					if (file.Read(&lTime) == 1L)
						{
						// Must be aligned to header size.
						m_lPadSize			= ALIGNTO(lBufSize, HEADERSIZE) - lBufSize;
						m_lBufRemaining	= lBufSize;

						// If w/i mask or global . . .
						if (ucChannel == 0 || ((1L << (ucChannel - 1)) & m_ulFilter) )
							{
							m_pChunk	= GetChunk(lId);
							// If no such chunk . . .
							if (m_pChunk == NULL)
								{
								m_pChunk	= AddChunk(lChunkSize, usType, ucFlags, lId, lTime);
								if (m_pChunk != NULL)
									{
									// Success.
									}
								else
									{
									// Skip chunk.  Could be allocation error, but it is
									// more probable that this is simply a chunk filtered
									// out at a higher level.  We rely on the user to
									// handle their own allocation errors.
									}
								}
							}
						else
							{
							// Skip chunk.
							m_pChunk	= NULL;
							}

						// If chunk is to be skipped . . .
						if (m_pChunk == NULL)
							{
							// Lump the padding in with the amount to be skipped.
							m_lBufRemaining	+= m_lPadSize;
							// Clear pad size.
							m_lPadSize			= 0L;
							}
						}
					else
						{
						TRACE("WinCall(): Unable to read channel #, type, or size.\n");
						sError = 2;
						}
					}
				}

			file.Close();
			}
		else
			{
			TRACE("WinCall(): Error opening memory file.\n");
			sError	= 1;
			}
		}

	// If done or file access error . . .
	if (m_pfw->GetStatus() & (STATUS_EOF | ERROR_FILEACCESS) || sError != 0)
		{
		// Done or toast.
		// Close file window.
		m_pfw->Close();
		}
	}

//////////////////////////////////////////////////////////////////////////////
//
// Callback dispatcher (calls the implied this version).
// (static)
//
//////////////////////////////////////////////////////////////////////////////
void CFilter::WinCallStatic(PPANE ppane, CFilter* pFilter)
	{
	// Pass it on.
	pFilter->WinCall(ppane);
	}

//////////////////////////////////////////////////////////////////////////////
//
// Returns ptr to chunk via lId, returns NULL if not found.
//
//////////////////////////////////////////////////////////////////////////////
PRTCHUNK CFilter::GetChunk(long lId)
	{
	PRTCHUNK	pChunk	= m_listPartial.GetHead();

	while (pChunk != NULL)
		{
		if (pChunk->lId == lId)
			{
			break;
			}

		pChunk = m_listPartial.GetNext();
		}

	return pChunk;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Add a chunk header.
// Returns chunk on success, NULL otherwise.
//
//////////////////////////////////////////////////////////////////////////////
PRTCHUNK CFilter::AddChunk(long lSize, USHORT usType, UCHAR ucFlags, long lId,
									long lTime)
	{
	short		sError	= 0;
	PRTCHUNK	pChunk	= NULL;

	// Attempt to allocate chunk . . .
	UCHAR* puc;
	if (AllocChunk(&puc, lSize, usType, ucFlags) == 0)
		{
		if (puc != NULL)
			{
			// Create new chunk header.
			pChunk = new RTCHUNK;
			if (pChunk != NULL)
				{
				// Set fields.
				pChunk->puc			= puc;
				pChunk->lPos		= 0L;
				pChunk->lSize		= lSize;
				pChunk->usType		= usType;
				pChunk->ucFlags	= ucFlags;
				pChunk->lId			= lId;
				pChunk->lTime		= lTime;
			
				// Add to list.
				if (m_listPartial.Add(pChunk) == 0)
					{
					}
				else
					{
					TRACE("AddChunk(): Unable to add chunk to partial chunk list.\n");
					sError = -3;
					}

				// If any errors occurred after header allocation . . .
				if (sError != 0)
					{
					delete pChunk;
					pChunk = NULL;
					}
				}
			else
				{
				TRACE("AddChunk(): Unable to allocate RTCHUNK.\n");
				sError = -2;
				}

			// If any errors occurred after chunk allocation . . .
			if (sError != 0)
				{
				FreeChunk(puc, usType, ucFlags);
				}
			}
		}
	else
		{
		TRACE("AddChunk(): AllocChunk failed.\n");
		sError = -1;
		}

	return pChunk;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Removes a chunk header.
// Returns 0 on success.
//
//////////////////////////////////////////////////////////////////////////////
short CFilter::RemoveChunk(PRTCHUNK pChunk)
	{
	short	sRes	= 0;	// Assume success.

	if (m_listPartial.Remove(pChunk) == 0)
		{
		// Success.
		delete pChunk;
		}
	else
		{
		TRACE("RemoveChunk(): Unable to remove chunk from list "
				"(probably not in there).\n");
		sRes = -1;
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Adds a buffer to a chunk.
// Returns amount added.
//
//////////////////////////////////////////////////////////////////////////////
long CFilter::AddToChunk(	CNFile*	pfile,		// File pointer.	
									long		lBufSize)	// Size of piece to add.
	{
	long	lRes	= 0;

	ASSERT(m_pChunk			!= NULL);

	lRes	= pfile->Read(m_pChunk->puc + m_pChunk->lPos, lBufSize);

	// Move to next position.
	m_pChunk->lPos += lRes;

	// Make sure there's no overflow.
	ASSERT(m_pChunk->lPos <= m_pChunk->lSize);

	// If chunk complete . . .
	if (m_pChunk->lPos == m_pChunk->lSize)
		{
		// Call user callback.
		ASSERT(m_fnUse != NULL)
		
		(*m_fnUse)(	m_pChunk->puc, m_pChunk->lSize, m_pChunk->usType, 
						m_pChunk->ucFlags, m_pChunk->lTime, m_lUser);
		
		// Remove chunk header from the list.
		if (RemoveChunk(m_pChunk) != 0)
			{
			TRACE("AddToChunk(): Unable to remove chunk header from list.\n");
			}
		}

	return lRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Allocates data via user callback if defined or malloc, otherwise.
// Returns 0 on success.  *ppuc of NULL does not necessarily indicate
// error; it means that the user decided it didn't want this data (it could
// be b/c of memory constraints, but probably not).  We rely on the user to
// stop this process if an error occurs during allocation in their alloc func!
// If this gets a malloc failure, that is considered an error.
//
//////////////////////////////////////////////////////////////////////////////
short CFilter::AllocChunk(	UCHAR** ppuc, long lSize, USHORT usType, 
									UCHAR ucFlags)
	{
	short	sRes	= 0;	// Assume success.

	if (m_fnAlloc != NULL)
		{
		*ppuc = (*m_fnAlloc)(lSize, usType, ucFlags, m_lUser);
		}
	else
		{
		*ppuc = (UCHAR*)malloc(lSize);
		// If successful . . .
		if (*ppuc != NULL)
			{
			}
		else
			{
			TRACE("AllocChunk(): Malloc failed.\n");
			sRes = -1;
			}
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Deallocates data via user callback if defined or free if both 
// m_fnAlloc AND m_fnFree are NOT defined.
//
//////////////////////////////////////////////////////////////////////////////
void CFilter::FreeChunk(UCHAR* puc, USHORT usType, UCHAR ucFlags)
	{
	if (puc != NULL)
		{
		// If an allocation function is defined . . .
		if (m_fnAlloc != NULL)
			{
			// If a deallocation function is defined . . .
			if (m_fnFree != NULL)
				{
				// Call it.
				(*m_fnFree)(puc, usType, ucFlags, m_lUser);
				}
			}
		else
			{
			free(puc);
			}
		}
	}

//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
