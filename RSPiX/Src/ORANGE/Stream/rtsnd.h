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
		short Use(	UCHAR* puc, long lSize, USHORT usType, UCHAR ucFlags, 
						long lTime);
		// Static entry point for above.
		static short UseStatic(	UCHAR* puc, long lSize, USHORT usType, 
										UCHAR ucFlags, long lTime, long l_pRtSnd);

		// Callback for mixer.
		// Returns new buffer to play or NULL if none.
		static void* MixCall(USHORT usMsg, void* pData, ULONG* pulBufSize, 
									ULONG ul_psndhdr);

		// Keeps the mixer channel open and starts the mixing in the beginning
		// and whenever a break up occurs due to streaming for all active 
		// channels.
		static void CritiCall(ULONG);

	public:	// Internal typedefs.

		typedef struct
			{
			UCHAR*	puc;		// Data.
			long		lSize;	// Amount of data in bytes.
			long		lTime;	// Time for chunk to be played.
			short		sLast;	// TRUE if the last buffer, FALSE otherwise.
			} SNDBUF, *PSNDBUF;

		typedef struct
			{
			// Header info from stream.
			long		lSamplesPerSec;	// Sample rate.
			short		sBitsPerSample;	// Number of bits that constitute a sample.
			short		sNumChannels;		// Number of channels: 1 == mono, 2 == stereo.
			long		lLead;				// Amount of time data is received of ahead
												// of being used.

			// Header info for our use.
			CMix				mix;						// Mixer channel.
			CQueue<SNDBUF, MAXBUFS>	qsndbufs;	// Queue of SNDBUFs for this channel.
			USHORT			usStatus;				// Status of current channel.
			CDispatch*		pdispatch;
			} SND_RT_HDR, *PSND_RT_HDR;

	public:		// Members.
		

	protected:	// Members.
		SND_RT_HDR	m_asndhdrs[MAX_SND_CHANNELS];// Info for each channel.
		USHORT		m_usState;					// The current state of this CRtSnd.
		CDispatch*	m_pdispatch;				// The dispatcher for this CRtSnd.
		
		static CList<SND_RT_HDR>	ms_listSndhdrs;	// List of active channels.

	};


#endif // RTSND_H
//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
