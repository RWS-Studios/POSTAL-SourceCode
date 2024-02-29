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
// RTSND.CPP
// 
// History:
//		10/31/95 JMI	Started.
//
//////////////////////////////////////////////////////////////////////////////
//
// This class is designed to receive SND data in real time and play it
// on a channel basis.
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
#include "rttypes.h"
#include "rtsnd.h"

//////////////////////////////////////////////////////////////////////////////
// Orange Headers.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Yellow Headers.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Module specific macros.
//////////////////////////////////////////////////////////////////////////////
// Types of chunks.
#define SND_CHUNK_HEADER	0
#define SND_CHUNK_DATA		1

// Status flags.
#define STATUS_OPENED	0x0001
#define STATUS_STARTED	0x0002
#define STATUS_DONE		0x0004
#define STATUS_CLOSEME	0x0008
#define STATUS_ERROR		0x8000

#define DATACHUNKHEADERSIZE	(sizeof(long))

//////////////////////////////////////////////////////////////////////////////
// Module specific typedefs.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Module specific (static) variables.
//////////////////////////////////////////////////////////////////////////////
CList<CRtSnd::SND_RT_HDR>	CRtSnd::ms_listSndhdrs;		// List of active channels.

//////////////////////////////////////////////////////////////////////////////
// Construction/Destruction Functions.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Default constructor.
//
//////////////////////////////////////////////////////////////////////////////
CRtSnd::CRtSnd()
	{
	Set();
	}

//////////////////////////////////////////////////////////////////////////////
//
// Destructor.
//
//////////////////////////////////////////////////////////////////////////////
CRtSnd::~CRtSnd()
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
void CRtSnd::Set(void)
	{
	m_pdispatch		= NULL;

	for (short i = 0; i < MAX_SND_CHANNELS; i++)
		{
		m_asndhdrs[i].usStatus	= 0;
		}
	}

//////////////////////////////////////////////////////////////////////////////
//
// Resets variables.  Performs deallocation if necessary.
//
//////////////////////////////////////////////////////////////////////////////
void CRtSnd::Reset(void)
	{
	}

//////////////////////////////////////////////////////////////////////////////
//
// Use handler for RtSnd buffers.
// Returns RET_FREE if done with data on return, RET_DONTFREE otherwise.
//
//////////////////////////////////////////////////////////////////////////////
short CRtSnd::Use(UCHAR* puc, long lSize, USHORT usType, UCHAR ucFlags,
						long lTime)
	{
	short	sRes		= RET_FREE;	// Always free.
	short	sError	= 0;

	ASSERT(usType	== RT_TYPE_SND);
	ASSERT(puc		!= NULL);

	CNFile file;
	file.Open(puc, lSize, ENDIAN_LITTLE);

	// Read values common to all chunks.

	// Read Snd ID.
	USHORT	usSndId;
	file.Read (&usSndId);
	
	// Make sure we're in range.
	ASSERT(usSndId < MAX_SND_CHANNELS);

	// Get corresponding header.
	PSND_RT_HDR	psndhdr	= &m_asndhdrs[usSndId];

	// If this is a header chunk . . .
	if (ucFlags & RT_FLAG_INIT)
		{
		// Handle header chunk.
		file.Read(&psndhdr->lSamplesPerSec);
		file.Read(&psndhdr->sNumChannels);
		file.Read(&psndhdr->sBitsPerSample);
		file.Read(&psndhdr->lLead);
		
		// Verify we didn't read too much.
		ASSERT(file.Error() == FALSE);

		// Initialize status.
		psndhdr->usStatus		= 0;
		// Init dispatcher.
		psndhdr->pdispatch	= m_pdispatch;

		// Attempt to open the mixer channel.
		if (psndhdr->mix.OpenChannel(	psndhdr->lSamplesPerSec, psndhdr->sBitsPerSample, 
												psndhdr->sNumChannels) == 0)
			{
			// Successfully opened mixer channel.
			psndhdr->usStatus	|= STATUS_OPENED;
			short	sWasEmpty	= ms_listSndhdrs.IsEmpty();
			// Add to criticial list.
			if (ms_listSndhdrs.Add(psndhdr) == 0)
				{
				// If this is the first . . .
				if (sWasEmpty == TRUE)
					{
					// Start critical handler that starts the mixing . . .
					if (Blu_AddCritical(CritiCall, (ULONG)this) == 0)
						{
						// Success.
						}
					else
						{
						TRACE("Use(): Unable to add CritiCall to critical list.\n");
						sError = 6;
						}
					}
				}
			else
				{
				TRACE("Use(): Unable to add RT_SND_HDR to critical list.\n");
				sError = 5;
				}
			}
		else
			{
			TRACE("Use(): Unable to open mix channel.\n");
			sError	= 1;
			}
		}
	else
		{
		// If no errors have occurred on this channel . . .
		if ((psndhdr->usStatus & STATUS_ERROR) == 0)
			{
			// Mixer channel must be open at this point.
			ASSERT(psndhdr->usStatus & STATUS_OPENED);

			// Create a SNDBUF for this data . . .
			PSNDBUF	psb	= new SNDBUF;
			if (psb != NULL)
				{
				// Fill.
				psb->puc		= puc;
				psb->lSize	= lSize;
				psb->lTime	= lTime + psndhdr->lLead;
				psb->sLast	= ((ucFlags & RT_FLAG_LAST) ? TRUE : FALSE);
				// Add to queue . . .
				if (psndhdr->qsndbufs.EnQ(psb) == 0)
					{
					// Chunk is in queue, do not free.
					sRes = RET_DONTFREE;
					}
				else
					{
					TRACE("Use(): Unable to EnQ SNDBUF.\n");
					sError = 3;
					// Enqueue failed.
					delete psb;
					}
				}
			else
				{
				TRACE("Use(): Unable to allocate new SNDBUF.\n");
				sError = 2;
				}
			}

		// Verify we didn't read too much.
		ASSERT(file.Error() == FALSE);
		}

	file.Close();

	if (sError != 0)
		{
		// If started . . .
		if (psndhdr->usStatus & STATUS_STARTED)
			{
			if (psndhdr->mix.Suspend() == 0)
				{
				// Suspended mixing on this channel.
				}
			else
				{
				TRACE("Use(): Failed to suspend mixing after error.\n");
				}
			}

		// If opened . . .
		if (psndhdr->usStatus & STATUS_OPENED)
			{
			if (psndhdr->mix.CloseChannel() == 0)
				{
				// Closed mixer channel.
				}
			else
				{
				TRACE("Use(): Failed to close mixer channel after error.\n");
				}
			}

		psndhdr->usStatus	= STATUS_ERROR;
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Callback for mixer.
// Returns new buffer to play or NULL if none.
//	(static)
//
//////////////////////////////////////////////////////////////////////////////
void* CRtSnd::MixCall(	USHORT usMsg, void* pData, ULONG* pulBufSize, 
										ULONG ul_psndhdr)
	{
	PSND_RT_HDR	psndhdr	= (PSND_RT_HDR)ul_psndhdr;
	PSNDBUF		psb;
	short			sLast		= FALSE;
	switch (usMsg)
		{
		case BLU_SNDMSG_PREPLAYERR:
		case BLU_SNDMSG_POSTPLAYERR:
		case BLU_SNDMSG_OK:
		case MIX_SNDMSG_QUEUEING:
			// If this is not the first time since we (re)started . . .
			if (pData != NULL)
				{
				// Get buffer that's done.
				psb	= psndhdr->qsndbufs.DeQ();
				// Must get.
				ASSERT(psb != NULL)
				// Should match supplied.
				if /*ASSERT*/(psb->puc + DATACHUNKHEADERSIZE == (UCHAR*)pData)
					TRACE("MixCall(): Not the expected pointer.\n");

				// Set last flag.
				sLast	= psb->sLast;

				// Free buffer.
				free(psb->puc);
				// Delete encapsulator.
				delete psb;
				}
			
			// If previous was not the last buffer . . .
			if (sLast == FALSE)
				{
				// Get the next buffer that's ready . . .
				psb = psndhdr->qsndbufs.Peek();
				if (psb != NULL)
					{
					// Set data pointer and size.
					pData			= psb->puc		+ DATACHUNKHEADERSIZE;
					*pulBufSize	= psb->lSize	- DATACHUNKHEADERSIZE;
					}
				else
					{
					psndhdr->usStatus	|= STATUS_ERROR;
					TRACE("MixCall(): No buffers in queue!!\n");
					pData	= NULL;
					}
				}
			else
				{
				// We're done.  Let mixer know.
				pData	= NULL;
				// Mark channel as done.
				psndhdr->usStatus	|= STATUS_DONE;
				}

			// If we're going to return NULL . . .
			if (pData == NULL)
				{
				// Returning NULL will stop callbacks.
				}
			break;

		case MIX_SNDMSG_SUSPENDED:
			// Remove started flag.
			psndhdr->usStatus &= ~STATUS_STARTED;

			// If we stopped b/c we're done . . .
			if (psndhdr->usStatus & STATUS_DONE)
				{
				if (psndhdr->mix.CloseChannel() == 0)
					{
					// Success.
					// Remove close me flag, opened flag, and done flag.
					psndhdr->usStatus &= ~(STATUS_OPENED | STATUS_DONE);
					// No longer need the callback.
					ms_listSndhdrs.Remove(psndhdr);
					// If the reference count hits zero . . .
					if (ms_listSndhdrs.IsEmpty() == TRUE)
						{
						if (Blu_RemoveCritical(CritiCall) == 0)
							{
							// Success.
							}
						else
							{
							TRACE("CritiCall(): Unable to remove critical handler.\n");
							}
						}
					}
				else
					{
					TRACE("CritiCall(): Unable to close mix channel.\n");
					}
				}
			break;
		}

	return pData;
	}

//////////////////////////////////////////////////////////////////////////////
//
// (Re)starts the mixing in the beginning and whenever a break up occurs due 
// to streaming and closes the mix channel when done.
// (static)
//
//////////////////////////////////////////////////////////////////////////////
void CRtSnd::CritiCall(ULONG)
	{
	PSND_RT_HDR	psndhdr	= ms_listSndhdrs.GetHead();

	while(psndhdr != NULL)
		{
		short	sError	= 0;

		long	lTime	= psndhdr->pdispatch->GetTime();

		// If channel open . . .
		if (psndhdr->usStatus & STATUS_OPENED)
			{
			// If channel not started . . .
			if ((psndhdr->usStatus & STATUS_STARTED) == 0)
				{
				// Look at next chunk.
				PSNDBUF	psb	= psndhdr->qsndbufs.Peek();
				if (psb != NULL)
					{
					// If it is time for this chunk . . .
					if (psb->lTime <= lTime)
						{
						// Attempt to start mixing in our channel . . .
						if (psndhdr->mix.Start(MixCall, (ULONG)psndhdr, 0) == 0)
							{
							psndhdr->usStatus |= STATUS_STARTED;
							}
						else
							{
							TRACE("CritiCall(): Unable to start mixing.\n");
							sError = 2;
							}
						}
					}
				}
			}

		// If any errors occurred . . .
		if (sError != 0)
			{
			psndhdr->usStatus |= STATUS_ERROR;
			}

		psndhdr	= ms_listSndhdrs.GetNext();
		}
	}

//////////////////////////////////////////////////////////////////////////////
//
// Callback dispatcher (calls the implied this version).
// (static)
//
//////////////////////////////////////////////////////////////////////////////
short CRtSnd::UseStatic(	UCHAR* puc, long lSize, USHORT usType, 
									UCHAR ucFlags, long lTime, long l_pRtSnd)
	{
	return ((CRtSnd*)l_pRtSnd)->Use(puc, lSize, usType, ucFlags, lTime);
	}


//////////////////////////////////////////////////////////////////////////////
// Methods.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Set dispatcher.
//
//////////////////////////////////////////////////////////////////////////////
void CRtSnd::SetDispatcher(CDispatch* pdispatch)
	{
	if (m_pdispatch != NULL)
		{
		m_pdispatch->SetDataHandler(RT_TYPE_SND, NULL);
		}

	m_pdispatch	= pdispatch;

	if (m_pdispatch != NULL)
		{
		m_pdispatch->SetDataHandler(RT_TYPE_SND, UseStatic);
		m_pdispatch->SetUserVal(RT_TYPE_SND, (long)this);
		}
	}

//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
