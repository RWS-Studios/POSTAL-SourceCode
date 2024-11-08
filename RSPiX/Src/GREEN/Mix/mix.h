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
											uint32_t*	pulBufSize, 
											uintptr_t		ulUser,
											uint8_t*	pucVol1,
											uint8_t*	pucVol2);

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
		int16_t OpenChannel(int32_t	lSampleRate,
								int32_t	lBitsPerSample,
								int32_t	lNumChannels);
		
		// Close a mix channel.
		// Returns 0 on success.
		int16_t CloseChannel(void);

		// Start receiving callbacks to fill channel data.
		// Set the initial mix volumes
		// Returns 0 on success.
		int16_t Start(RMixCall mcUser, uintptr_t ulUser,
					uint8_t	ucVolume = 255, uint8_t ucVol2 = 255 );

		// Stop receiving callbacks to fill channel data.
		int16_t Suspend(void);	// Returns 0 on success.

		// Pause mix channel.
		void PauseChannel(void);	

		// Resume mix channel.
		void ResumeChannel(void);	

		// Check mix channel's paused status.
		int16_t IsChannelPaused(void);	// Returns TRUE, if sound output is paused; FALSE otherwise.

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
		static int16_t SetMode(			// Returns 0 on success.
			int32_t	lSamplesPerSec,		// Sample rate in samples per second.
			int32_t	lDevBitsPerSample,	// Number of bits per sample for device.
			int32_t	lNumChannels,			// Number of channels (1 == mono,2 == stereo).
			int32_t	lBufferTime,			// Amount of time buffer spends in queue b4
												// being played.
			int32_t	lMaxBufferTime,		// Maximum that lBufferTime can be set to
												// dynamically with RMix::SetBufferTime().
			int32_t	lMixBitsPerSample,	// Bit depth at which samples will be mixed.
			int32_t	lSrcBitsPerSample);	// Bit depth at which samples must be to be
												// mixed or 0 for no preference.

		// Set the current audio mode.
		// This will cause any open channels to start playing.
		static int16_t SetMode(			// Returns 0 on success.
			int32_t	lSamplesPerSec,		// Sample rate in samples per second.
			int32_t	lBitsPerSample,		// Number of bits per sample.
			int32_t	lNumChannels,			// Number of channels (1 == mono,2 == stereo).
			int32_t	lBufferTime,			// Amount of time buffer spends in queue b4
												// being played.
			int32_t	lMaxBufferTime)		// Maximum that lBufferTime can be set to
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
		static int16_t Pause(void);	// Returns 0 on success.
		
		// Resume currently paused audio.
		// NOTE:  Pause/Resume is implemented in levels by Blue.
		static int16_t Resume(void);	// Returns 0 on success.

		// Returns TRUE, if sound output is paused; FALSE otherwise.
		static int16_t IsPaused(void);	// Returns TRUE, if sound output is paused; FALSE otherwise.

		// Do stuff specific to RMix and the playing of audio through Blue.
		// This includes calling rspDoSound().
		static int32_t Do(void);	// Returns value returned by rspDoSound() that
										// indicates how much audio, in milliseconds,
										// was required to be queued.

		// Reset all current mix channels.
		static int16_t Reset(void);	// Returns 0 on success.

		// Suspends all current mix channels.
		static int16_t SuspendAll(void);	// Returns 0 on success.

		// Sets the maximum duration that can occur between calls
		// to rspDoSound.
		static void SetBufferTime(int32_t lBufferTime) 
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
			int16_t sAutoPump)			// In:  TRUE to auto-pump silence, FALSE othwerise.
			{
			ms_sKeepPumping	= sAutoPump;
			}

	public:	// Querries.
		/////////////////////////////////////////////////////////////////////////
		// API that affects a single channel.
		/////////////////////////////////////////////////////////////////////////

		// Returns TRUE if this mix channel is open; FALSE otherwise.
		int16_t IsOpen(void) { return m_sOpen; }
		// Returns TRUE if this mix channel is active; FALSE otherwise.
		int16_t IsActive(void) { return m_sActive; }

		// Returns the time for this RMix.
		int32_t GetTime(void);

		// Returns the position for this RMix.
		int32_t GetPos(void);

		/////////////////////////////////////////////////////////////////////////
		// API that affects all channels (static).
		/////////////////////////////////////////////////////////////////////////

		// Returns the current state for all RMixes.
		static State GetState() { return ms_sState; }

		// Gets the current mode of the sound output device.
		static int16_t GetMode(						// Returns 0 on success; 
															// nonzero if no mode.
			int32_t*		plSamplesPerSec,				// Sample rate in samples per second
															// returned here, if not NULL.
			int32_t*		plDevBitsPerSample = NULL,	// Bits per sample of device,
															// returned here, if not NULL.
			int32_t*		plNumChannels = NULL,		// Number of channels (1 == mono, 
															// 2 == stereo) returned here, 
															// if not NULL.
			int32_t*		plBufferTime = NULL,			// Amount of time in ms to lead the 
															// current play cursor returned here,
															// if not NULL.  This could also be 
															// described as the maximum amount of
															// time in ms that can occur between 
															// calls to rspDoSound.
			int32_t*		plMaxBufferTime	= NULL,	// Maximum buffer time.  This is the amt
															// that *plBufferTime can be increased to.
															// This is indicative of how much space
															// was/will-be allocated for the sound
															// output device on rspLockSoundOut.
			int32_t*		plMixBitsPerSample = NULL,	// Bits per sample at which samples are
															// mixed, if not NULL.
			int32_t*		plSrcBitsPerSample = NULL);// Bits per sample at which samples must
															// be to be mixed (0 if no requirement), 
															// if not NULL.

	protected:	// Internal use.
		// Intialize members.
		void Init(void);

		// Called when all sound on a channel has finished.
		// Returns 0 on success.
		int16_t ChannelFinished(void);

		// Implied this version of BlueCallStatic, called from BlueCallStatic.
		int16_t BlueCall(			// Returns FALSE when no data mixed.
			int32_t		lDataPos,	// Position that this buffer represents in stream.
			PMIXBUF	pmb);			// Mix buffer to mix into.

		// Callbacks from Blue.
		static int16_t BlueCallStatic(	// Returns TRUE to continue mixing in this
												// buffer or FALSE to not mix this buffer.
			uint8_t*	pucData, 
			int32_t		lBufSize, 
			int32_t		lDataPos,
			uint32_t*	pul_ppmixbuf);

	public:	// members

		// Volume information is set from Start and RSND callbacks
		uint8_t			m_ucVolume;				// 0 - 255
		uint8_t			m_ucSecondaryVolume;	// 0 - 255

	protected:	// Members.
		int32_t			m_lSampleRate;			// Sample rate for audio playback/mix.
		int32_t			m_lBitsPerSample;		// Sample size in bits.
		int32_t			m_lNumChannels;		// Number of channels (mono or stereo).

		int16_t			m_sOpen;					// TRUE if channel open; FALSE 
													// otherwise.
		int16_t			m_sActive;				// TRUE if channel active; FALSE
													// otherwise.
		int16_t			m_sSuspending;			// TRUE if channel suspending; FALSE
													// otherwise.
		int32_t			m_lLastDataPos;		// Last byte mixed into.
		RMixCall		m_mcUser;				// User callback.
		uintptr_t			m_ulUser;				// User value.
		uint8_t*		m_pucData;				// User data.
		uint32_t			m_ulAmount;				// Amount of user data remaining.

		int32_t			m_lStartTime;			// Audio time when first buffer entered
													// queue.
		int32_t			m_lStartPos;			// Audio position when first buffer
													// enter queue.

		RSndFx*		m_psndfx;				// Pointer to an RSndFx.

		int16_t			m_sPauseLevel;			// Current pause level.

		static RList<RMix>	ms_listActive;		// List of active channels.
																	
		static int16_t			ms_sSetMode;		// TRUE if we set Blue's sound
															// output mode.

		static State			ms_sState;			// Current state for all RMixes.
		static int32_t				ms_lCurPos;			// Current play position
															// based on absolute start.
		static uint32_t			ms_ulBufSize;		// The size to use when allocating
															// RMixBufs.
		static int16_t			ms_sReset;			// Resets Blue and returns all
															// current user buffers.
		static RSndFx*			ms_psndfx;			// Pointer to a global RSndFx.

		static int16_t			ms_sKeepPumping;	// Keep Blue pumped with silence
															// when no channels are playing,
															// if TRUE.

		static RMixBuf			ms_mixbuf;			// One and only mix buffer.
	};

#endif // MIX_H
///////////////////////////////////////////////////////////////////////////////
// EOF
///////////////////////////////////////////////////////////////////////////////
