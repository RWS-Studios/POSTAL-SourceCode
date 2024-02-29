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
//	mix.cpp
// 
// History:
//		06/17/95 JMI	Started.
//
//		06/26/95	JMI	Added critical handler to allow asynchronous closing
//							of Blue's sound output.
//
//		06/27/95	JMI	Added check to make sure the task was not running before
//							attempting to start it.  Also, moved ms_lCurPos = 0 init
//							to OpenChannel when Blue is opened.
//
//		07/15/95	JMI	Removed potential for confusion on value of ms_lCurPos
//							when adding buffers to what was an ending mix stream.
//							TaskCallStatic now handles starting and adding
//							differently.  Also, ms_lCurPos is added when it is de-
//							cided that the buffer is needed.
//
//		07/17/95	JMI	Added Reset() funcion.  Simply calling Blue's ResetSound-
//							Out is not enough, given the new way we get data from the
//							app (using buffers of any size).  We must ResetSoundOut
//							and return all current user buffers.
//
//		09/07/95	JMI	If a callback provided less than the buffer size worth
//							of sound data, and then returned that it was done,
//							BlueCall would return NULL (representing it was done
//							mixing this channel), causing BlueCallStatic to discard
//							the buffer (even though something was mixed in) IF no 
//							other channels returned non-NULL.  This has been fixed.
//							This fix brought to light a problem that probably would
//							not have otherwise surfaced but may have affected some-
//							one somewhere by some rarity.  The problem was that
//							The task call assumed that it would be called in a
//							situation where it was definite that there were not
//							enough CMixBufs allocated.  Now it makes sure.
//
//		11/28/95	JMI	Added a more deferred method of suspending channels.
//
//		12/18/95	JMI	Made changes corresponding to new Blue sound method.
//							This greatly simplied starting the mixing process.
//
//		07/08/96	JMI	Converted to new CList that does not convert your 
//							template type into a poiter.
//
//		07/31/96	JMI	Updating so this can be included in Release 02.00.04 even
//							though it is not completed.  It functions.
//
//		08/26/96	JMI	Updated to use new Blue sound API.  Took out a few
//							unused pieces of code and variables.  Basic functionality
//							is the same.
//
//		08/30/96	JMI	Suspend() now sets m_lLastDataPos to 0 to make sure that,
//							if it is not set otherwise, the Mix() will finish very
//							soon.  Also added fail safe to BlueCallStatic() to 
//							'finish' all channels that are suspended on a usMsg ==
//							RSP_SNDMSG_DONE.
//
//		08/30/96	JMI	RSP_SND_CALLBACK not returns 0 on success and 
//							non-zero on failure.                                
//
//		09/03/96	JMI	Adapted to newest revision of Blue Sound API (which
//							removed rspStart/StopSoundOutCallbacks, 
//							rspIsSoundOutCallingBack, and callback messages
//							(RSP_SNDMSG_DONE and RSP_SNDMSG_DATA).  Now this, CMix,
//							provides a complete interface to the sound instead of
//							the partial one it used to.
//
//		09/04/96 MJR	Changed callback from ULONG to long for size parameter.
//
//		09/06/96	JMI	I patched Reset() so it might work.
//
//		09/10/96	JMI	Removed Reset() patch.  Blue has been updated to fix
//							the problem.
//
//		10/30/96	JMI	Changed:
//							Old label:		New label:
//							=========		=========
//							CMixBuf			RMixBuf
//							CMix				RMix
//							MIXCALL			RMixCall
//							CSndFx			RSndFx
//							MIX_STATE_*		*				Macros changed to enum.
//							MIX_MSG_*		*				Macros changed to enum.
//							CList				RList
//
//		11/06/96	JMI	Reset ms_lCurPos to 0 in SetMode().
//
//		03/24/97	JMI	OpenChannel() now fails, if there's no mode.
//
//		06/03/97	JMI	Now SetMode() returns the value returned from 
//							rspSetSoundOutMode(), if that fails.
//
//		06/13/97	JMI	Now calls ChannelFinished() right from Supsend().
//
//		06/26/97	JMI	Added optional constant playing of silence to keep Blue
//							pumping when no channels are playing.
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
//		09/17/97 JMI	Make sure m_sPauseLevel is initialized to 0.  Didn't seem
//							to have affected Postal because the samples were statically
//							allocate -- which initializes to zero.
//
//		10/30/97	JMI	Added alternate version of SetMode() which allows more
//							detail as to bit depth quality of samples and mixing.
//							Also, added a GetMode().
//							
//////////////////////////////////////////////////////////////////////////////
//
// This module slides between a Blu_*SoundOut module and Blu_*SoundOut itself.
// The advantage of using this module is that more than one person can use it
// at once w/o making a mess.
//
// Normal limitations: This module can mix data of the same sample rate with 
// different buffer sizes per channel, with different sample sizes (8 or
// 16 bit PCM), and different numbers of channels (mono or stereo).
//
// For now limitations: In order to get this module running smoothly, it only
// currently accepts data of the same sample rates, sample sizes, and number 
// of channels.
//
// For ending mixes we make the suspend message come after all
// of the corresponding channel's data has been played.  Basically, we set the
// m_lLastDataPos member to the position of the last byte played and call
// Suspend() which sets the m_sSuspending flag. In every callback we check if 
// m_sSuspending is set, and, when set, check to see if rspGetSoundOutPos() 
// has exceeded m_lLastDataPos; if so, we call ChannelFinished().  When 
//	m_sSuspending is set, we will not mix from that channel or call it back.
//
// NEVER CALL rspClearSoundOut() when using CMix.
//
//////////////////////////////////////////////////////////////////////////////

#ifdef PATHS_IN_INCLUDES
	#include "BLUE/system.h"
	#include "BLUE/Blue.h"
	#include "GREEN/Mix/mix.h"
#else
	#include "System.h"
	#include "Blue.h"
	#include "mix.h"
#endif // PATHS_IN_INCLUDES


//////////////////////////////////////////////////////////////////////////////
// Macros.
//////////////////////////////////////////////////////////////////////////////
#define DEF_BUFSIZE			4096L
#define DEF_BUFFERTIME		185
#define DEF_MAXBUFFERTIME	200

//////////////////////////////////////////////////////////////////////////////
// Instantiate static members.
//////////////////////////////////////////////////////////////////////////////
RList<RMix>		RMix::ms_listActive;						// List of active channels.

short				RMix::ms_sSetMode	= FALSE;				// TRUE if we set Blue's 
																	// sound output mode.

RMix::State		RMix::ms_sState	= Idle;				// Current state for all
																	// RMixes.
long				RMix::ms_lCurPos	= 0L;					// Current play position
																	// based on absolute start.
ULONG				RMix::ms_ulBufSize	= 0xFFFFFFFF;	// The size to use when al-
																	// locating RMixBufs.

short				RMix::ms_sReset	= FALSE;				// If TRUE, current user
																	// buffers are returned.

RSndFx*			RMix::ms_psndfx	= NULL;				// Pointer to a global 
																	// RSndFx.

short				RMix::ms_sKeepPumping	= FALSE;		// Keep Blue pumped with
																	// silence when no channels
																	// are playing, if TRUE.

RMixBuf			RMix::ms_mixbuf;							// One and only mix buffer.

//////////////////////////////////////////////////////////////////////////////
// <Con|De>struction.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Default Constructor.
//
//////////////////////////////////////////////////////////////////////////////
RMix::RMix(void)
	{
	Init();
	}

//////////////////////////////////////////////////////////////////////////////
//
// Destructor.
//
//////////////////////////////////////////////////////////////////////////////
RMix::~RMix(void)
	{
	ASSERT(m_sActive == FALSE);
	ASSERT(m_sOpen == FALSE);

	if (m_sActive == TRUE)
		{
		TRACE("~RMix(): Destroying an active RMix.\n");
		}

	if (m_sOpen == TRUE)
		{
		TRACE("~RMix(): Destroying an open RMix.\n");
		CloseChannel();
		}
	}

//////////////////////////////////////////////////////////////////////////////
// Internal Use.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Intialize members.
//
//////////////////////////////////////////////////////////////////////////////
void RMix::Init(void)
	{
	m_lSampleRate		= 0L;
	m_lBitsPerSample	= 0L;
	m_lNumChannels		= 0L;

	m_sOpen				= FALSE;
	m_sActive			= FALSE;
	m_sSuspending		= FALSE;

	m_lLastDataPos		= 0L;

	m_mcUser				= NULL;
	m_ulUser				= 0L;
	m_pucData			= NULL;
	m_ulAmount			= 0L;

	m_lStartTime		= -1L;
	m_lStartPos			= -1L;

	m_psndfx				= NULL;

	if (ms_ulBufSize == 0xFFFFFFFF)
		{
		ms_ulBufSize = 0;//rspGetSoundOutPaneSize();
		}

	m_ucVolume = 255;				
	m_ucSecondaryVolume = 255;	

	m_sPauseLevel = 0;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Implied this version of BlueCallStatic, called from BlueCallStatic.
//
//////////////////////////////////////////////////////////////////////////////
short RMix::BlueCall(	// Returns FALSE when done.
	long		lDataPos,	// Position that this buffer represents in stream.
	PMIXBUF	pmb)			// Mix buffer to mix into.
	{
	short	sRes	= FALSE;	// Assume no sound mixed in.

	// If this channel is not suspending and not paused . . .
	if (m_sSuspending == FALSE && m_sPauseLevel == 0)
		{
		// If first time for this RMix . . .
		if (m_lStartTime == -1L)
			{
			// Set start time and position.
			m_lStartPos		= ms_lCurPos;
			m_lStartTime	= (long)(((float)ms_lCurPos * (float)1000) / 
										((float)m_lSampleRate * ((float)m_lBitsPerSample / (float)8) 
											* (float)m_lNumChannels
										) );
			}
		
		ULONG ulTotalMixedIn	= 0L;
		ULONG	ulMixBufSize	= pmb->GetMixSize();
		ULONG	ulCurMix;

		// If we were recently reset . . .
		if (ms_sReset == TRUE)
			{
			// Release current user buffer.
			// Setting this to zero will cause a callback whether we have more
			// data or not.
			m_ulAmount = 0L;
			}

		do
			{
			if (m_ulAmount == 0L)
				{
				// Call user callback to get more data and the current volume!.
				m_pucData = (UCHAR*) (*m_mcUser)(Data, m_pucData, &m_ulAmount, 
					m_ulUser, &m_ucVolume, &m_ucSecondaryVolume);
				}
			
			// If not done . . .
			if (m_pucData != NULL)
				{
				// Amount to mix in now.
				ulCurMix = MIN(m_ulAmount, ulMixBufSize - ulTotalMixedIn);

				// If an effect is defined . . .
				if (m_psndfx != NULL)
					{
					// If there is an active effect . . .
					if (m_psndfx->GetCurrentFX() != 0)
						{
						// Attempt to allocate temp buffer . . .
						U8*	pu8	= (U8*)malloc(ulCurMix);
						if (pu8 != NULL)
							{
							// Perform effect into buffer.
							m_psndfx->Do(m_pucData, ulCurMix, pu8);
							// Mix.
							pmb->Mix(ulTotalMixedIn, pu8, ulCurMix, 
										m_lSampleRate, m_lBitsPerSample, m_lNumChannels,
										m_ucVolume,m_ucSecondaryVolume);
							// Free temp memory.
							free(pu8);
							}
						else
							{
							TRACE("BlueCall(): Not enough memory to perform effect.\n");
							}
						}
					}
				else
					{
					// Mix.
					pmb->Mix(ulTotalMixedIn, m_pucData, ulCurMix, 
								m_lSampleRate, m_lBitsPerSample, m_lNumChannels,
								m_ucVolume,m_ucSecondaryVolume);
					}
				
				// Add to amount mixed in.
				ulTotalMixedIn += ulCurMix;
				// Deduct from amount available.
				m_ulAmount -= ulCurMix;
				// Move in buffer.
				m_pucData += ulCurMix;

				// Let caller know we used buffer.
				sRes = TRUE;
				}
			else
				{
				// Done.
				break;
				}
			// Until we've filled the buffer from this stream . . .
			} while (ulTotalMixedIn < ulMixBufSize);

		// If we've exhausted this stream . . .
		if (m_pucData == NULL)
			{
			// Suspend this channel.  What we mixed in this iteration will still get
			// played, though.
			Suspend();

			// Remember the last byte mixed into.
			m_lLastDataPos	= lDataPos + ulTotalMixedIn;
			}
		}
	else
		{
		// We did not use buffer.
		sRes	= FALSE;
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Callbacks from Blue.  Call each RMix in active list.
// (static)
//
//////////////////////////////////////////////////////////////////////////////
short RMix::BlueCallStatic(	// Returns TRUE to continue mixing in this
										// buffer or FALSE to not mix this buffer.
	UCHAR*	pucData,				// Buffer to mix into.
	long		lBufSize,			// Size of memory that pucData points to.
	long		lDataPos,			// Position this buffer will take in the overall
										// stream.
	ULONG*	/*pul_ppmixbuf*/)	// An unused user value.
	{
	short sBufDone	= TRUE;	// Assume buffer not needed.

	// Get head of list of active channels.
	RMix*	pmix	= ms_listActive.GetHead();

	// If there are any active channels or we are auto-pumping . . .
	if (pmix != NULL || ms_sKeepPumping != FALSE)
		{
		ms_mixbuf.SetDest(pucData, lBufSize);

		while (pmix != NULL)
			{
			// Call mix channel.  If data mixed into buffer . . .
			if (pmix->BlueCall(lDataPos, &ms_mixbuf) != FALSE)
				{
				// Mark buffer as needed.
				sBufDone	= FALSE;
				}

			// Get next.
			pmix	= ms_listActive.GetNext();
			}

		// Prepare for playback.
		ms_mixbuf.PrepareForDest();

		// Clear buffer from mixer.
		ms_mixbuf.SetDest(NULL, 0);

		// If we were reset recently, we are done processing the reset.
		ms_sReset = FALSE;

		// If auto-pumping . . .
		if (ms_sKeepPumping != FALSE)
			{
			// We always need the buffer.
			sBufDone	= FALSE;
			}

		// If buffer needed . . .
		if (sBufDone == FALSE)
			{
			// Buffer is going back into queue.
			
			// If there are sound fx . . .
			if (ms_psndfx != NULL)
				{
				// Perform them on buffer.
				ms_psndfx->Do(pucData, lBufSize);
				}

			// Update current position in audio.
			ms_lCurPos	+= lBufSize;
			}
		}
		
	// Non-zero return value indicates 
	// that pucData should not be         
	// played.  Additionally, rspDoSound()
	// will not call the callback until   
	// the next rspDoSound() call.        
	return sBufDone;
	}

//////////////////////////////////////////////////////////////////////////////
// Methods.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Set the current audio mode.
// This will cause any open channels to start playing.
// (static)
//
//////////////////////////////////////////////////////////////////////////////
short RMix::SetMode(				// Returns 0 on success.
	long	lSamplesPerSec,		// Sample rate in samples per second.
	long	lDstBitsPerSample,	// Number of bits per sample.
	long	lNumChannels,			// Number of channels (1 == mono,2 == stereo).
	long	lBufferTime,			// Amount of time buffer spends in queue b4
										// being played.
	long	lMaxBufferTime,		// Maximum that lBufferTime can be set to
										// dynamically with RMix::SetBufferTime().
	long	lMixBitsPerSample,	// Bit depth at which samples will be mixed.
	long	lSrcBitsPerSample)	// Bit depth at which samples must be to be
										// mixed or 0 for no preference.
	{
	short	sRes	= 0;	// Assume success.

	if (ms_sSetMode == FALSE)
		{
		// Set up mix to values.
		RMixBuf::ms_lSampleRate			= lSamplesPerSec;
		RMixBuf::ms_lNumChannels		= lNumChannels;
		RMixBuf::ms_lSrcBitsPerSample	= lSrcBitsPerSample;
		RMixBuf::ms_lMixBitsPerSample	= lMixBitsPerSample;
		RMixBuf::ms_lDstBitsPerSample	= lDstBitsPerSample;

		// Clear position.
		ms_lCurPos	= 0L;

		// Set the mode to this data type.
		sRes	= rspSetSoundOutMode(
			lSamplesPerSec, 
			lDstBitsPerSample, 
			lNumChannels, 
			lBufferTime, 
			lMaxBufferTime, 
			BlueCallStatic, 
			0L);

		if (sRes == 0)
			{
			// Remember we set the mode, so we know to kill the mode.
			ms_sSetMode	= TRUE;
			}
		else
			{
			TRACE("SetMode(): rspSetSoundOutMode failed.\n");
			}
		}
	else
		{
		TRACE("SetMode(): Already in a mode.\n");
		sRes = 1;
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Kills the current audio mode.
// This will cause any open channels to be closed stops Blue from
// utilizing the sound audio device.
// (static)
//
//////////////////////////////////////////////////////////////////////////////
void RMix::KillMode(void)
	{
	if (ms_sSetMode != FALSE)
		{
		// Attempt to end the audio mode.
		rspKillSoundOutMode();

		ms_sSetMode	= FALSE;
		}
	else
		{
		TRACE("KillMode(): Not in a mode.\n");
		}
	}

//////////////////////////////////////////////////////////////////////////////
//
// Pause currently playing audio.
// NOTE:  Pause/Resume is implemented in levels by Blue.
// (static)
//
//////////////////////////////////////////////////////////////////////////////
short RMix::Pause(void)	// Returns 0 on success.
	{
	short	sRes	= 0;	// Assume success.

	if (ms_sSetMode != FALSE)
		{
		if (rspPauseSoundOut() == 0)
			{
			}
		else
			{
			TRACE("Pause(): rspPauseSoundOut() failed.\n");
			sRes	= -1;
			}
		}
	else
		{
		TRACE("Pause(): Not in a mode.\n");
		sRes	= 1;
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Resume currently paused audio.
// NOTE:  Pause/Resume is implemented in levels by Blue.
// (static)
//
//////////////////////////////////////////////////////////////////////////////
short RMix::Resume(void)	// Returns 0 on success.
	{
	short	sRes	= 0;	// Assume success.

	if (ms_sSetMode != FALSE)
		{
		if (rspResumeSoundOut() == 0)
			{
			}
		else
			{
			TRACE("Resume(): rspResumeSoundOut() failed.\n");
			sRes	= -1;
			}
		}
	else
		{
		TRACE("Resume(): Not in a mode.\n");
		sRes	= 1;
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Returns TRUE, if sound output is paused; FALSE otherwise.
// (static)
//
//////////////////////////////////////////////////////////////////////////////
short RMix::IsPaused(void)	// Returns TRUE, if sound output is paused; FALSE otherwise.
	{
	return rspIsSoundOutPaused();
	}

//////////////////////////////////////////////////////////////////////////////
//
// Do stuff specific to RMix and the playing of audio through Blue.
// This includes calling rspDoSound().
// (static)
//
//////////////////////////////////////////////////////////////////////////////
long RMix::Do(void)	// Returns value returned by rspDoSound() that
							// indicates how much audio, in milliseconds,
							// was required to be queued.
	{
   ///////////////////////////////////////////////////////////////////////////
	// Check for buffers that have completed playing:
   ///////////////////////////////////////////////////////////////////////////

    rspLockSound();

	long	lPlayPos;
	// If in a mode . . .
	if (ms_sSetMode != FALSE)
		{
		// Get current play cursor position.
		lPlayPos	= rspGetSoundOutPos();
		}
	else
		{
		// Everything might as well just be done.
		lPlayPos	= 0x7FFFFFFF;
		}

	PMIX	pmix	= ms_listActive.GetHead();
	while (pmix != NULL)
		{
		// If pmix is suspending . . .
		if (pmix->m_sSuspending != FALSE)
			{
			// If the play cursor has passed the last data put in from pmix 
			// or we were recently reset . . .
			if (lPlayPos > pmix->m_lLastDataPos || ms_sReset != FALSE)
				{
				// Finish this channel.  Note that this function removes pmix
				// from the list, but since it is pmix that gets removed, when
				// we do a GetNext() we get the one after pmix.
				if (pmix->ChannelFinished() == 0)
					{
					// Successfully suspended.
					}
				else
					{
					TRACE("Do(): ChannelFinished() failed.\n");
					}
				}
			}

		pmix	= ms_listActive.GetNext();
		}

   ///////////////////////////////////////////////////////////////////////////
	// Monitor RMix state:
   ///////////////////////////////////////////////////////////////////////////

   ///////////////////////////////////////////////////////////////////////////
	// Let Blue do its Sound schtuff:
   ///////////////////////////////////////////////////////////////////////////

	long rc = rspDoSound();
    rspUnlockSound();
    return(rc);
	}

//////////////////////////////////////////////////////////////////////////////
//
// Open a mix channel.
// Returns 0 on success, 1 if no mode, negative on error.
//
//////////////////////////////////////////////////////////////////////////////
short RMix::OpenChannel(long	lSampleRate,
								long	lBitsPerSample,
								long	lNumChannels)
	{
	short		sRes	= 0;	// Assume success.

	// There must be a mode . . .
	if (ms_sSetMode != FALSE)
		{
		if (m_sOpen == FALSE)
			{
			// Set up channel.
			m_lSampleRate		= lSampleRate;
			m_lBitsPerSample	= lBitsPerSample;
			m_lNumChannels		= lNumChannels;

			// Set open flag.
			m_sOpen	= TRUE;
			}
		else
			{
			TRACE("OpenChannel(): Already open.\n");
			sRes = -1;
			}
		}
	else
		{
		// No current mode.
		sRes	= 1;
		}

	return sRes;
	}
		
//////////////////////////////////////////////////////////////////////////////
//
// Close a mix channel.
// Returns 0 on success.
//
//////////////////////////////////////////////////////////////////////////////
short RMix::CloseChannel(void)
	{
	short		sRes	= 0;	// Assume success.
	
	if (m_sOpen == TRUE)
		{
		if (m_sActive == FALSE)
			{
			// Clear open flag.
			m_sOpen	= FALSE;
			}
		else
			{
			TRACE("CloseChannel(): Channel is active.\n");
			sRes = -2;
			}
		}
	else
		{
		TRACE("CloseChannel(): Not open.\n");
		sRes = -1;
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Start receiving callbacks to fill channel data.
// Set the initial mix volumes
// Returns 0 on success.
//
//////////////////////////////////////////////////////////////////////////////
short RMix::Start(RMixCall mcUser, ULONG ulUser,
					UCHAR	ucVolume /* = 255 */, UCHAR ucVol2 /* = 255 */)
	{						 
	short	sRes	= 0;	// Assume success.

	ASSERT(m_sOpen == TRUE);
	ASSERT(mcUser	!= NULL);

	// Add to active list . . .
	if (ms_listActive.Add(this) == 0)
		{
		// Set user parms.
		m_mcUser	= mcUser;
		m_ulUser	= ulUser;
		
		// Init user data.
		m_pucData			= NULL;
		m_ulAmount			= 0L;

		// Flag start time/pos to be set later.
		m_lStartTime		= -1L;
		m_lStartPos			= -1L;

		// Make sure we don't suspend right away.
		m_sSuspending		= FALSE;

		// If any errors occurred . . .
		if (sRes != 0)
			{
			if (ms_listActive.Remove(this) == 0)
				{
				}
			else
				{
				TRACE("Start(): Unable to remove from active list after error.\n");
				}
			}
		else
			{
			// Set the initial sound volumes
			m_ucVolume = ucVolume;
			m_ucSecondaryVolume = ucVol2;

			// Mark as active.
			m_sActive = TRUE;
			
			// We are now mixing or will be on the next Do().
			ms_sState	= Processing;
			}
		}
	else
		{
		TRACE("Start(): Unable to add to active list.\n");
		sRes = -1;
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Stop receiving callbacks to fill channel data.
// Returns 0 on success.
//
//////////////////////////////////////////////////////////////////////////////
short RMix::Suspend(void)
	{
	short	sRes	= 0;	// Assume success.

	if (m_sActive == TRUE)
		{
		// Let callback know this channel is suspending.
		m_sSuspending	= TRUE;
		// Set point at which we will be done to very, very soon.
		m_lLastDataPos	= 0L;
		// Finish now, now.
		ChannelFinished();
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Pause mix channel.
//
//////////////////////////////////////////////////////////////////////////////
void RMix::PauseChannel(void)	
	{
	ASSERT(m_sPauseLevel < 32767);
	m_sPauseLevel++;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Resume mix channel.
//
//////////////////////////////////////////////////////////////////////////////
void RMix::ResumeChannel(void)
	{
	ASSERT(m_sPauseLevel > 0);
	m_sPauseLevel--;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Check mix channel's paused status.
//
//////////////////////////////////////////////////////////////////////////////
short RMix::IsChannelPaused(void)	// Returns TRUE, if sound output is paused; FALSE otherwise.
	{
	return (m_sPauseLevel == 0) ? FALSE : TRUE;
	}

//////////////////////////////////////////////////////////////////////////////
//
// This function resets all current channels (but does not suspend) them.
// Basically, it function much like Windows' Reset command in that it simply
// returns any buffers currently being used.  It works asynchronously.
// NEVER CALL rspClearSoundOut() when using RMix.
//	Returns 0 on success.  (static)
//
//////////////////////////////////////////////////////////////////////////////
short RMix::Reset(void)
	{
	short	sRes	= 0;	// Assume success.

	// Attempt to reset sound output . . .
	if (rspClearSoundOut() == 0)
		{
		// Get the current position.
		// Update ms_lCurPos appropriately.
		ms_lCurPos	= rspGetSoundOutPos();

		// Success.  Flag callbacks.
		ms_sReset	= TRUE;
		}
	else
		{
		TRACE("Reset(): Unable to reset Blue's Sound Out.\n");
		sRes = -1;
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Suspends all current mix channels.
// (static)
//
//////////////////////////////////////////////////////////////////////////////
short RMix::SuspendAll(void)	// Returns 0 on success.
	{
	short	sRes	= 0;	// Assume success.

	RMix*	pmix		= ms_listActive.GetHead();
	while (pmix != NULL)
		{
		// Finish this channel.  Note that this function removes pmix
		// from the list, but since it is pmix that gets removed, when
		// we do a GetNext() we get the one after pmix.
		pmix->ChannelFinished();

		pmix	= ms_listActive.GetNext();
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Called when all sound on a channel has finished.
// Returns 0 on success.
//
//////////////////////////////////////////////////////////////////////////////
short RMix::ChannelFinished(void)
	{
	short	sRes	= 0;	// Assume success.

	ASSERT(m_sActive == TRUE);

	if (ms_listActive.Remove(this) == 0)
		{
		m_sActive = FALSE;

		// Call user callback to let it know we're suspended.
		(*m_mcUser)(Suspended, NULL, NULL, m_ulUser, NULL, NULL);

		// If there are no more buffers . . 
		if (ms_listActive.IsEmpty() != FALSE)
			{
			ms_sState	= Idle;
			}
		}
	else
		{
		sRes = -1;
		TRACE("ChannelFinished(): Unable to remove Mixer from active list.\n");
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
// Querries.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Returns the time for this RMix (positive if successful).
//
//////////////////////////////////////////////////////////////////////////////
long RMix::GetTime(void)
	{
	long lRes;

	if (m_lStartTime >= 0L)
		{
		lRes = rspGetSoundOutTime();

		if (lRes != -1L)
			{
			lRes -= m_lStartTime;
			}
		else
			{
			TRACE("GetTime(): Unable to get time from Blue.\n");
			}
		}
	else
		{
		lRes = -1L;
		}

	return lRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Returns the position for this RMix (positive if successful).
//
//////////////////////////////////////////////////////////////////////////////
long RMix::GetPos(void)
	{
	long lRes;

	if (m_lStartPos >= 0L)
		{
		lRes	= rspGetSoundOutPos();

		if (lRes != -1L)
			{
			lRes -= m_lStartPos;
			}
		else
			{
			TRACE("GetPos(): Unable to get pos from Blue.\n");
			}
		}
	else
		{
		lRes = -1L;
		}

	return lRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Gets the current mode of the sound output device.
// (static).
//
//////////////////////////////////////////////////////////////////////////////
short RMix::GetMode(							// Returns 0 on success; 
													// nonzero if no mode.
	long*		plSamplesPerSec,				// Sample rate in samples per second
													// returned here, if not NULL.
	long*		plDevBitsPerSample,			// Bits per sample of device,
													// returned here, if not NULL.
	long*		plNumChannels,					// Number of channels (1 == mono, 
													// 2 == stereo) returned here, 
													// if not NULL.
	long*		plBufferTime,					// Amount of time in ms to lead the 
													// current play cursor returned here,
													// if not NULL.  This could also be 
													// described as the maximum amount of
													// time in ms that can occur between 
													// calls to rspDoSound.
	long*		plMaxBufferTime,				// Maximum buffer time.  This is the amt
													// that *plBufferTime can be increased to.
													// This is indicative of how much space
													// was/will-be allocated for the sound
													// output device on rspLockSoundOut.
	long*		plMixBitsPerSample,			// Bits per sample at which samples are
													// mixed, if not NULL.
	long*		plSrcBitsPerSample)			// Bits per sample at which samples must
													// be to be mixed (0 if no requirement), 
													// if not NULL.
	{
	short	sRes	= rspGetSoundOutMode(
		plSamplesPerSec, 
		plDevBitsPerSample,
		plNumChannels,
		plBufferTime,
		plMaxBufferTime);

	if (plMixBitsPerSample)
		{
		*plMixBitsPerSample	= RMixBuf::ms_lMixBitsPerSample;
		}

	if (plSrcBitsPerSample)
		{
		*plSrcBitsPerSample	= RMixBuf::ms_lSrcBitsPerSample;
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
