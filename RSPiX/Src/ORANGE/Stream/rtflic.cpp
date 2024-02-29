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
// RTFLIC.CPP
// 
// History:
//		10/30/95 JMI	Started.
//
//		11/06/95	JMI	Added sPixelsModified and sColorsModified flags to mark
//							when these changes occur.  Also added actual FLX decomp-
//							ression using CRamFlx's new static functions.
//
//////////////////////////////////////////////////////////////////////////////
//
// This class is designed to receive FLX data in real time and decompress it
// to a specific buffer or buffers on a channel basis.
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// C Headers.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Blue Headers.
//////////////////////////////////////////////////////////////////////////////
#include "System.h"
#include "bdebug.h"

//////////////////////////////////////////////////////////////////////////////
// Green Headers.
//////////////////////////////////////////////////////////////////////////////
#include "rttypes.h"
#include "rtflic.h"

#include "ramflx.h"

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
#define FLX_CHUNK_HEADER	0
#define FLX_CHUNK_DATA		1

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
CRtFlic::CRtFlic()
	{
	Set();
	}

//////////////////////////////////////////////////////////////////////////////
//
// Destructor.
//
//////////////////////////////////////////////////////////////////////////////
CRtFlic::~CRtFlic()
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
void CRtFlic::Set(void)
	{
	m_pdispatch		= NULL;
	for (short i = 0; i < MAX_VID_CHANNELS; i++)
		{
		m_aflxhdrs[i].sNumFrames		= 0;
		m_aflxhdrs[i].pImage				= NULL;
		m_aflxhdrs[i].callbackHeader	= NULL;
		m_aflxhdrs[i].callbackBefore	= NULL;
		m_aflxhdrs[i].callbackAfter	= NULL;
		}
	}

//////////////////////////////////////////////////////////////////////////////
//
// Resets variables.  Performs deallocation if necessary.
//
//////////////////////////////////////////////////////////////////////////////
void CRtFlic::Reset(void)
	{
	}

//////////////////////////////////////////////////////////////////////////////
//
// Use handler for RtFlic buffers.
// Returns RET_FREE if done with data on return, RET_DONTFREE otherwise.
//
//////////////////////////////////////////////////////////////////////////////
short CRtFlic::Use(	UCHAR* puc, long lSize, USHORT usType, UCHAR ucFlags, 
							long lTime)
	{
	short	sRes		= RET_FREE;	// Always free.
	short	sError	= 0;

	ASSERT(usType	== RT_TYPE_FLIC);
	ASSERT(puc		!= NULL);

	CNFile file;
	file.Open(puc, lSize, ENDIAN_LITTLE);

	// Read values common to all chunks.

	// Read flx ID.
	USHORT	usFlxId;
	file.Read (&usFlxId);
	
	// Make sure we're in range.
	ASSERT(usFlxId < MAX_VID_CHANNELS);

	// Get corresponding header.
	PFLX_RT_HDR	pflxhdr	= &m_aflxhdrs[usFlxId];

	// If this is a header chunk . . .
	if (ucFlags & RT_FLAG_INIT)
		{
		// Handle header chunk.
		file.Read(&pflxhdr->sNumFrames);
		file.Read(&pflxhdr->sWidth);
		file.Read(&pflxhdr->sHeight);
		file.Read(&pflxhdr->sDepth);
		file.Read(&pflxhdr->lMilliPerFrame);
		file.Read(&pflxhdr->sNoDelta);
		file.Read(&pflxhdr->sTransparent);
		file.Read(&pflxhdr->sX);
		file.Read(&pflxhdr->sY);
		
		// Verify we didn't read too much.
		ASSERT(file.Error() == FALSE);

		// Initialize frame counter.
		pflxhdr->sCurFrame			= 0;
		pflxhdr->sPixelsModified	= FALSE;
		pflxhdr->sColorsModified	= FALSE;
		// Default to one frame's worth of lag before skipping frames.
		pflxhdr->lMaxLag				= pflxhdr->lMilliPerFrame;


		// If there is a callback for the header . . .
		if (pflxhdr->callbackHeader != NULL)
			{
			// Pass user all info.
			(*pflxhdr->callbackHeader)(this, pflxhdr);
			}
		}
	else
		{
		// If there is a callback for before decompression . . .
		if (pflxhdr->callbackBefore != NULL)
			{
			// Pass user all info.
			(*pflxhdr->callbackBefore)(this, pflxhdr);
			}

		// The rest of this chunk is regular old FLX data!
		// Decompress into image if supplied.
		if (pflxhdr->pImage != NULL)
			{
			ASSERT(pflxhdr->pImage->pData != NULL);
			ASSERT(pflxhdr->pImage->pPalette != NULL);
			ASSERT(pflxhdr->pImage->pPalette->pData != NULL);


			short	sDecompress	= TRUE;
			// If we this flx contains no deltas and this is not a key frame . . .
			if (pflxhdr->sNoDelta == TRUE && (ucFlags & RT_FLAG_TAG == 0))
				{
				// If we've exceeded the maximum lag . . .
				if (m_pdispatch->GetTime() - lTime > pflxhdr->lMaxLag)
					{
					sDecompress = FALSE;
					}
				}

			if (sDecompress == TRUE)
				{
				if (CRamFlx::DoReadFrame(pflxhdr->pImage, &file, 
												&pflxhdr->sPixelsModified,
												&pflxhdr->sColorsModified) == 0)
					{
					// Success.
					// If there is a callback for after decompression . . .
					if (pflxhdr->callbackAfter != NULL)
						{
						// Pass user all info.
						(*pflxhdr->callbackAfter)(this, pflxhdr);
						}
				
					// Increment frame count.
					pflxhdr->sCurFrame++;
					}
				else
					{
					TRACE("Use(): FLX decompression failed.\n");
					}
				}
			}
		else
			{
			TRACE("Use(): Data with no associated image.\n");
			}
	
		// Verify we didn't read too much.
		ASSERT(file.Error() == FALSE);
		}

	file.Close();

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Callback dispatcher (calls the implied this version).
// (static)
//
//////////////////////////////////////////////////////////////////////////////
short CRtFlic::UseStatic(	UCHAR* puc, long lSize, USHORT usType, 
									UCHAR ucFlags, long lTime, long l_pRtFlic)
	{
	return ((CRtFlic*)l_pRtFlic)->Use(puc, lSize, usType, ucFlags, lTime);
	}


//////////////////////////////////////////////////////////////////////////////
// Methods.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Set dispatcher.
//
//////////////////////////////////////////////////////////////////////////////
void CRtFlic::SetDispatcher(CDispatch* pdispatch)
	{
	if (m_pdispatch != NULL)
		{
		m_pdispatch->SetDataHandler(RT_TYPE_FLIC, NULL);
		}

	m_pdispatch	= pdispatch;

	if (m_pdispatch != NULL)
		{
		m_pdispatch->SetDataHandler(RT_TYPE_FLIC, UseStatic);
		m_pdispatch->SetUserVal(RT_TYPE_FLIC, (long)this);
		}
	}

//////////////////////////////////////////////////////////////////////////////
//
// Sets callback(s) called on channel header receipt.
//
//////////////////////////////////////////////////////////////////////////////
void CRtFlic::SetCallbackHeader(RTFLIC_CALL callback)
	{
	for (short i = 0; i < MAX_VID_CHANNELS; i++)
		{
		SetCallbackHeader(callback, i);
		}
	}

//////////////////////////////////////////////////////////////////////////////
//
// Sets callback(s) called on channel header receipt.
//
//////////////////////////////////////////////////////////////////////////////
void CRtFlic::SetCallbackHeader(RTFLIC_CALL callback, short sChannel)
	{
	m_aflxhdrs[sChannel].callbackHeader	= callback;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Sets callback(s) called before decompression.
//
//////////////////////////////////////////////////////////////////////////////
void CRtFlic::SetCallbackBefore(RTFLIC_CALL callback)
	{
	for (short i = 0; i < MAX_VID_CHANNELS; i++)
		{
		SetCallbackBefore(callback, i);
		}
	}

//////////////////////////////////////////////////////////////////////////////
//
// Sets callback(s) called before decompression.
//
//////////////////////////////////////////////////////////////////////////////
void CRtFlic::SetCallbackBefore(RTFLIC_CALL callback, short sChannel)
	{
	m_aflxhdrs[sChannel].callbackBefore	= callback;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Sets callback(s) called after decompression.
//
//////////////////////////////////////////////////////////////////////////////
void CRtFlic::SetCallbackAfter(RTFLIC_CALL callback)
	{
	for (short i = 0; i < MAX_VID_CHANNELS; i++)
		{
		SetCallbackAfter(callback, i);
		}
	}

//////////////////////////////////////////////////////////////////////////////
//
// Sets callback(s) called after decompression.
//
//////////////////////////////////////////////////////////////////////////////
void CRtFlic::SetCallbackAfter(RTFLIC_CALL callback, short sChannel)
	{
	m_aflxhdrs[sChannel].callbackAfter	= callback;
	}

//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
