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
///////////////////////////////////////////////////////////////////////////////
//
//	mix.h
// 
// History:
//		06/17/95 JMI	Started.
//
//		06/26/97	JMI	Started tracking history of this .h file.
//							Added optional constant playing of silence.
//
//		07/16/97	JRD	Added volume members and volume parameters to Start.
//							Modified BlueCall to pass parameters to MixBuf.
//
//		07/17/97 JRD	Removed Volume parameters, as they should be set
//							by the callback.
//							
//		07/17/97 JRD	Added Volume parameters, as they aren't always set
//							by the callback.
//
//		08/05/97	JMI	Added IsPaused(), PauseChannel(), ResumeChannel(), 
//							and IsChannelPaused().
//
//		10/30/97	JMI	Added alternate version of SetMode() which allows more
//							detail as to bit depth quality of samples and mixing.
//							
//////////////////////////////////////////////////////////////////////////////
//
// See CPP header comment for details.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef MIX_H
#define MIX_H
///////////////////////////////////////////////////////////////////////////////
// Headers.
///////////////////////////////////////////////////////////////////////////////
#include "System.h"

// If PATHS_IN_INCLUDES macro is defined, we can utilized relative
// paths to a header file.  In this case we generally go off of our
// RSPiX root directory.  System.h MUST be included before this macro
// is evaluated.  System.h is the header that, based on the current
// platform (or more so in this case on the compiler), defines 
// PATHS_IN_INCLUDES.  Blue.h includes system.h so you can include that
// instead.
#ifdef PATHS_IN_INCLUDES
	#include "ORANGE/CDT/List.h"
	#include "GREEN/Mix/MixBuf.h"
	#include "GREEN/SndFx/SndFx.h"
#else
	#include "List.h"
	#include "MixBuf.h"
	#include "SndFx.h"
#endif // PATHS_IN_INCLUDES

///////////////////////////////////////////////////////////////////////////////
// Macros.
///////////////////////////////////////////////////////////////////////////////
// Maximum number of buffers this class will use.
#define MAX_BUFS	256

///////////////////////////////////////////////////////////////////////////////
// Types.
///////////////////////////////////////////////////////////////////////////////

// Forward declare class for handy-dandy typedef.
class RMix;

// Handy-dandy typedef.
typedef RMix* PMIX;

// Class definition.
class RMix
	{
	public:	// Enums and typedefs.
		typedef enum
			{	// Mix messages (callback can receive in addition to blue messages).
			Suspended,	// Channel has finished.
			Data			// Channel needs new data.
			} Msg;

		typedef enum
			{	// Mix states for ms_sState and GetState().
			Idle,			// No mixing occurring.
			Processing	// There are unfinished channels (i.e.,
							// either mixing is occurring or there
							// are channels that have data that is 
							// still being played by Blue).
			} State;

		// Callback.
		typedef void* (*RMixCall)(	Msg		msg, 
											void*		pData, 
											ULONG*	pulBufSize, 
											ULONG		ulUser,
											UCHAR*	pucVol1,
											UCHAR*	pucVol2);

	public:	// <Con|De>struction.
		// Default constructor.
		RMix();
		// Destructor.
		~RMix();

	public:	// Methods.
		/////////////////////////////////////////////////////////////////////////
		// API that affects a single channel.
		/////////////////////////////////////////////////////////////////////////

		// Open a mix channel.
		// Returns 0 on success.
		short OpenChannel(long	lSampleRate,
								long	lBitsPerSample,
								long	lNumChannels);
		
		// Close a mix channel.
		// Returns 0 on success.
		short CloseChannel(void);

		// Start receiving callbacks to fill channel data.
		// Set the initial mix volumes
		// Returns 0 on success.
		short Start(RMixCall mcUser, ULONG ulUser,
					UCHAR	ucVolume = 255, UCHAR ucVol2 = 255 );

		// Stop receiving callbacks to fill channel data.
		short Suspend(void);	// Returns 0 on success.

		// Pause mix channel.
		void PauseChannel(void);	

		// Resume mix channel.
		void ResumeChannel(void);	

		// Check mix channel's paused status.
		short IsChannelPaused(void);	// Returns TRUE, if sound output is paused; FALSE otherwise.

		// Set or clear (if psndfx is NULL) a RSndFx for this channel.
		void SetFx(				// Returns nothing.
			RSndFx* psndfx)	// FX for this channel.  Clears current, if 
									// NULL.
			{
			m_psndfx	= psndfx;
			}

		/////////////////////////////////////////////////////////////////////////
		// API that affects all channels (static).
		/////////////////////////////////////////////////////////////////////////

		// Set the current audio mode.
		// This will cause any open channels to start playing.
		static short SetMode(			// Returns 0 on success.
			long	lSamplesPerSec,		// Sample rate in samples per second.
			long	lDevBitsPerSample,	// Number of bits per sample for device.
			long	lNumChannels,			// Number of channels (1 == mono,2 == stereo).
			long	lBufferTime,			// Amount of time buffer spends in queue b4
												// being played.
			long	lMaxBufferTime,		// Maximum that lBufferTime can be set to
												// dynamically with RMix::SetBufferTime().
			long	lMixBitsPerSample,	// Bit depth at which samples will be mixed.
			long	lSrcBitsPerSample);	// Bit depth at which samples must be to be
												// mixed or 0 for no preference.

		// Set the current audio mode.
		// This will cause any open channels to start playing.
		static short SetMode(			// Returns 0 on success.
			long	lSamplesPerSec,		// Sample rate in samples per second.
			long	lBitsPerSample,		// Number of bits per sample.
			long	lNumChannels,			// Number of channels (1 == mono,2 == stereo).
			long	lBufferTime,			// Amount of time buffer spends in queue b4
												// being played.
			long	lMaxBufferTime)		// Maximum that lBufferTime can be set to
												// dynamically with RMix::SetBufferTime().
			{
			return SetMode(
				lSamplesPerSec,
				lBitsPerSample,
				lNumChannels,	
				lBufferTime,	
				lMaxBufferTime,
				lBitsPerSample,
				lBitsPerSample);
			}

		// Kills the current audio mode.
		// This will cause any open channels to be closed stops Blue from
		// utilizing the sound audio device.
		static void KillMode(void);

		// Pause currently playing audio.
		// NOTE:  Pause/Resume is implemented in levels by Blue.
		static short Pause(void);	// Returns 0 on success.
		
		// Resume currently paused audio.
		// NOTE:  Pause/Resume is implemented in levels by Blue.
		static short Resume(void);	// Returns 0 on success.

		// Returns TRUE, if sound output is paused; FALSE otherwise.
		static short IsPaused(void);	// Returns TRUE, if sound output is paused; FALSE otherwise.

		// Do stuff specific to RMix and the playing of audio through Blue.
		// This includes calling rspDoSound().
		static long Do(void);	// Returns value returned by rspDoSound() that
										// indicates how much audio, in milliseconds,
										// was required to be queued.

		// Reset all current mix channels.
		static short Reset(void);	// Returns 0 on success.

		// Suspends all current mix channels.
		static short SuspendAll(void);	// Returns 0 on success.

		// Sets the maximum duration that can occur between calls
		// to rspDoSound.
		static void SetBufferTime(long lBufferTime) 
			{ ms_ulBufSize = 0; rspSetSoundOutBufferTime(lBufferTime); }

		// Set or clear (if psndfx is NULL) a RSndFx for all channels.
		static void SetGlobalFx(	// Returns nothing.
			RSndFx* psndfx)			// FX for all channels.  Clears current, if 
											// NULL.
			{
			ms_psndfx	= psndfx;
			}

		// Enable or disable auto-pump feature which will keep silence pumping
		// through Blue's sound interface even when there are no channels to
		// mix.  This, when enabled, keeps delays consistent and removes overhead,
		// if any, for starting Blue's sound stuff.
		static void SetAutoPump(	// Returns nothing.
			short sAutoPump)			// In:  TRUE to auto-pump silence, FALSE othwerise.
			{
			ms_sKeepPumping	= sAutoPump;
			}

	public:	// Querries.
		/////////////////////////////////////////////////////////////////////////
		// API that affects a single channel.
		/////////////////////////////////////////////////////////////////////////

		// Returns TRUE if this mix channel is open; FALSE otherwise.
		short IsOpen(void) { return m_sOpen; }
		// Returns TRUE if this mix channel is active; FALSE otherwise.
		short IsActive(void) { return m_sActive; }

		// Returns the time for this RMix.
		long GetTime(void);

		// Returns the position for this RMix.
		long GetPos(void);

		/////////////////////////////////////////////////////////////////////////
		// API that affects all channels (static).
		/////////////////////////////////////////////////////////////////////////

		// Returns the current state for all RMixes.
		static State GetState() { return ms_sState; }

		// Gets the current mode of the sound output device.
		static short GetMode(						// Returns 0 on success; 
															// nonzero if no mode.
			long*		plSamplesPerSec,				// Sample rate in samples per second
															// returned here, if not NULL.
			long*		plDevBitsPerSample = NULL,	// Bits per sample of device,
															// returned here, if not NULL.
			long*		plNumChannels = NULL,		// Number of channels (1 == mono, 
															// 2 == stereo) returned here, 
															// if not NULL.
			long*		plBufferTime = NULL,			// Amount of time in ms to lead the 
															// current play cursor returned here,
															// if not NULL.  This could also be 
															// described as the maximum amount of
															// time in ms that can occur between 
															// calls to rspDoSound.
			long*		plMaxBufferTime	= NULL,	// Maximum buffer time.  This is the amt
															// that *plBufferTime can be increased to.
															// This is indicative of how much space
															// was/will-be allocated for the sound
															// output device on rspLockSoundOut.
			long*		plMixBitsPerSample = NULL,	// Bits per sample at which samples are
															// mixed, if not NULL.
			long*		plSrcBitsPerSample = NULL);// Bits per sample at which samples must
															// be to be mixed (0 if no requirement), 
															// if not NULL.

	protected:	// Internal use.
		// Intialize members.
		void Init(void);

		// Called when all sound on a channel has finished.
		// Returns 0 on success.
		short ChannelFinished(void);

		// Implied this version of BlueCallStatic, called from BlueCallStatic.
		short BlueCall(			// Returns FALSE when no data mixed.
			long		lDataPos,	// Position that this buffer represents in stream.
			PMIXBUF	pmb);			// Mix buffer to mix into.

		// Callbacks from Blue.
		static short BlueCallStatic(	// Returns TRUE to continue mixing in this
												// buffer or FALSE to not mix this buffer.
			UCHAR*	pucData, 
			long		lBufSize, 
			long		lDataPos,
			ULONG*	pul_ppmixbuf);

	public:	// members

		// Volume information is set from Start and RSND callbacks
		UCHAR			m_ucVolume;				// 0 - 255
		UCHAR			m_ucSecondaryVolume;	// 0 - 255

	protected:	// Members.
		long			m_lSampleRate;			// Sample rate for audio playback/mix.
		long			m_lBitsPerSample;		// Sample size in bits.
		long			m_lNumChannels;		// Number of channels (mono or stereo).

		short			m_sOpen;					// TRUE if channel open; FALSE 
													// otherwise.
		short			m_sActive;				// TRUE if channel active; FALSE
													// otherwise.
		short			m_sSuspending;			// TRUE if channel suspending; FALSE
													// otherwise.
		long			m_lLastDataPos;		// Last byte mixed into.
		RMixCall		m_mcUser;				// User callback.
		ULONG			m_ulUser;				// User value.
		UCHAR*		m_pucData;				// User data.
		ULONG			m_ulAmount;				// Amount of user data remaining.

		long			m_lStartTime;			// Audio time when first buffer entered
													// queue.
		long			m_lStartPos;			// Audio position when first buffer
													// enter queue.

		RSndFx*		m_psndfx;				// Pointer to an RSndFx.

		short			m_sPauseLevel;			// Current pause level.

		static RList<RMix>	ms_listActive;		// List of active channels.
																	
		static short			ms_sSetMode;		// TRUE if we set Blue's sound
															// output mode.

		static State			ms_sState;			// Current state for all RMixes.
		static long				ms_lCurPos;			// Current play position
															// based on absolute start.
		static ULONG			ms_ulBufSize;		// The size to use when allocating
															// RMixBufs.
		static short			ms_sReset;			// Resets Blue and returns all
															// current user buffers.
		static RSndFx*			ms_psndfx;			// Pointer to a global RSndFx.

		static short			ms_sKeepPumping;	// Keep Blue pumped with silence
															// when no channels are playing,
															// if TRUE.

		static RMixBuf			ms_mixbuf;			// One and only mix buffer.
	};

#endif // MIX_H
///////////////////////////////////////////////////////////////////////////////
// EOF
///////////////////////////////////////////////////////////////////////////////
