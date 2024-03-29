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
#ifndef RTSND_H
#define RTSND_H

//////////////////////////////////////////////////////////////////////////////
// Headers.
//////////////////////////////////////////////////////////////////////////////
#include "dispatch.h"
#include "Image.h"
#include "mix.h"
					  
//////////////////////////////////////////////////////////////////////////////
// Macros.
//////////////////////////////////////////////////////////////////////////////
// This value can be from 0 to 64K - 1.
// The overhead per channel is sizeof(SND_RT_HDR) bytes.
#define MAX_SND_CHANNELS	50

#define MAXBUFS				60

//////////////////////////////////////////////////////////////////////////////
// Typedefs.
//////////////////////////////////////////////////////////////////////////////

class CRtSnd
	{
	public:		// Construction/Destruction.
		// Default constructor.
		CRtSnd();
		// Destructor.
		~CRtSnd();

	public:		// Methods.
		// Sets the dispatcher.
		void SetDispatcher(CDispatch* pdispatch);

	protected:	// Internal methods.
		// Sets variables w/o regard to current values.
		void Set();
		// Resets variables.  Performs deallocation if necessary.
		void Reset();

		// Use handler for RtSnd buffers.
		// Returns RET_FREE if done with data on return, RET_DONTFREE otherwise.
		int16_t Use(	uint8_t* puc, int32_t lSize, uint16_t usType, uint8_t ucFlags, 
						int32_t lTime);
		// Static entry point for above.
		static int16_t UseStatic(	uint8_t* puc, int32_t lSize, uint16_t usType, 
										uint8_t ucFlags, int32_t lTime, int32_t l_pRtSnd);

		// Callback for mixer.
		// Returns new buffer to play or NULL if none.
		static void* MixCall(uint16_t usMsg, void* pData, uint32_t* pulBufSize, 
									uint32_t ul_psndhdr);

		// Keeps the mixer channel open and starts the mixing in the beginning
		// and whenever a break up occurs due to streaming for all active 
		// channels.
		static void CritiCall(uint32_t);

	public:	// Internal typedefs.

		typedef struct
			{
			uint8_t*	puc;		// Data.
			int32_t		lSize;	// Amount of data in bytes.
			int32_t		lTime;	// Time for chunk to be played.
			int16_t		sLast;	// TRUE if the last buffer, FALSE otherwise.
			} SNDBUF, *PSNDBUF;

		typedef struct
			{
			// Header info from stream.
			int32_t		lSamplesPerSec;	// Sample rate.
			int16_t		sBitsPerSample;	// Number of bits that constitute a sample.
			int16_t		sNumChannels;		// Number of channels: 1 == mono, 2 == stereo.
			int32_t		lLead;				// Amount of time data is received of ahead
												// of being used.

			// Header info for our use.
			CMix				mix;						// Mixer channel.
			CQueue<SNDBUF, MAXBUFS>	qsndbufs;	// Queue of SNDBUFs for this channel.
			uint16_t			usStatus;				// Status of current channel.
			CDispatch*		pdispatch;
			} SND_RT_HDR, *PSND_RT_HDR;

	public:		// Members.
		

	protected:	// Members.
		SND_RT_HDR	m_asndhdrs[MAX_SND_CHANNELS];// Info for each channel.
		uint16_t		m_usState;					// The current state of this CRtSnd.
		CDispatch*	m_pdispatch;				// The dispatcher for this CRtSnd.
		
		static CList<SND_RT_HDR>	ms_listSndhdrs;	// List of active channels.

	};


#endif // RTSND_H
//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
