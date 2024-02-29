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
//	snd.cpp
// 
// History:
//		04/28/95 JMI	Started.
//
//		05/22/95	JMI	Added GetPos().
//
//		06/08/95	JMI	Added GetTime().
//
//		06/14/95	JMI	Added ability to use any number of buffers to stream.
//							This is limited, of course, by the capabilities of BLUE.
//
//		07/15/95	JMI	Added loop ability through public member m_sLoop.
//
//		07/31/96	JMI	Now responds to BLU_SNDMSG_DATA instead of
//							RSP_SNDMSG_OK/PREPLAYERR/POSTPLAYERR.
//
//		08/26/96	JMI	Changed BLU_SNDMSG_* to RSP_SNDMSG_*.  Removed
//							lPrimeInterval parm from play.  It has not been used
//							since 07/15/95, I think.
//
//		08/30/96	JMI	Abort() now calls m_mix.Suspend() as it should have been.
//
//		09/03/96	JMI	Adapted to newest revision of Blue Sound API (which
//							removed rspStart/StopSoundOutCallbacks, 
//							rspIsSoundOutCallingBack, and callback messages
//							(RSP_SNDMSG_DONE and RSP_SNDMSG_DATA).
//
//		10/27/96 MJR	Fixed "unused variable" warnings.
//
//		10/28/96	JMI	Removed unused variable lPrimeInterval.
//
//		10/30/96	JMI	Changed:
//							Old label:		New label:
//							=========		=========
//							CMix				RMix
//							RSnd				RSnd
//							MIX_STATE_*		*
//							MIX_MSG_*		*
//							SND_STATE_*		*				Macros changed to enum.
//
//		01/29/97	JMI	Added callback on done.
//
//		04/02/97	JMI	When I added the callback, I forgot to initialize it in
//							Init().  Fixed.
//
//		05/09/97	JMI	Added ability to loop sub samples.
//
//		05/13/97	JMI	Casted instances of warning C4018 (signed/unsigned mismatch)
//							to make MSVC 4.1(Alpha) happy (these seem to fall under
//							Warning Level 3 in 4.1(Alpha) but not in 4.2(Intel)).
//
//		07/15/97 JRD	Added members to hold sound volume information.  Made
//							sure callbacks updated volume levels and all levels
//							were passed on.
//
//		07/30/97	JMI	Added ASSERTs so stupid people (let's call one of them 
//							JMI) don't pass loop points that exceed the overall size
//							of the sample buffer causing cool music.
//							Also, added if's to check these in non-TRACENASSERT mode.
//
//		08/01/97	JMI	End loop time parameter to Play() was not working as
//							stated for 0 case b/c it had a < 0 instead of < 1.
//
//		08/11/97	JMI	Placed Sound Done callback such that the user can still
//							access the sample safely, if desired.
//
//		09/25/97	JMI	Now PlayCall() only calls the callback if Abort() was
//							not called (Abort() calls the callback).
//
//////////////////////////////////////////////////////////////////////////////
//
// This thing uses blue to play or stream a music file.  Currently only WAV
// files are supported, but I tried to make it easy for it to be modified to
// autodetect and load other wave files.
//
//////////////////////////////////////////////////////////////////////////////

#include "System.h"

#ifdef PATHS_IN_INCLUDES
	#include "GREEN/Snd/snd.h"
	#include "BLUE/Blue.h"
#else
	#include "snd.h"
	#include "Blue.h"
#endif // PATHS_IN_INCLUDES


//////////////////////////////////////////////////////////////////////////////
// Initialize static member variables.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Macros.
//////////////////////////////////////////////////////////////////////////////
#define SND_TYPE_UNKNOWN	0x0000
#define SND_TYPE_WAVE		0x0001

//////////////////////////////////////////////////////////////////////////////
// Functions.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Default Constructor.
// Returns nothing.
// (public)
//
//////////////////////////////////////////////////////////////////////////////
RSnd::RSnd()
	{
	Init();
	}

//////////////////////////////////////////////////////////////////////////////
//
// Destructor.
// Returns nothing.
// (public)
//
//////////////////////////////////////////////////////////////////////////////
RSnd::~RSnd()
	{
	// Reset and free.
	Reset();
	}

//////////////////////////////////////////////////////////////////////////////
//
// Initialize instantiable members.
// Returns nothing.
// (public)
//
//////////////////////////////////////////////////////////////////////////////
void RSnd::Init(void)
	{
	// Initialize members.
	m_lBufSize			= 0L;
	m_sState				= Stopped;
	m_psample			= NULL;
	m_sOwnSample		= FALSE;
	m_sLoop				= FALSE;
	m_dcUser				= NULL;
	m_lLoopStartPos	= 0;	
	m_lLoopEndPos		= 0;
	m_sTypeVolume		= 255;	// should be overwritten by Play
	m_sChannelVolume	= 255;	// should be overwritten by Play
	}

///////////////////////////////////////////////////////////////////////////////
//
// Reset object.  Release play data and reset variables.
// Returns nothing.
// (public)
//
///////////////////////////////////////////////////////////////////////////////
void RSnd::Reset(void)
	{
	ASSERT(GetState() == Stopped);

	if (GetState() == Stopped)
		{
		if (m_psample != NULL)
			{
			// Unlock the sample.
			m_psample->Unlock();
			// If we are responsible for the sample . . .
			if (m_sOwnSample == TRUE)
				{
				delete m_psample;
				m_psample = NULL;
				}
			}
		
		Init();
		}
	else
		{
		TRACE("Reset():  Attempt to Reset a playing WAVE!\n");
		}
	}

///////////////////////////////////////////////////////////////////////////////
//
// This function instigates streaming of a sound file.  The file is opened, the
// header read, the sound device opened, and a task is started to begin filling
// Blue's available sound output buffers with audio data.  The task starts
// reading data every lPrimeInterval milliseconds to fill all of Blue's buffers
// and then fires them off and suspends itself.  The callback reads the data
// in lReadBufSize chunks and plays in lPlayBufSize chunks.
//
// Returns 0 on success.
// (public)
//
///////////////////////////////////////////////////////////////////////////////
short RSnd::Stream(	char* pszSampleName, long lPlayBufSize, long lReadBufSize,
						 UCHAR	ucMainVolume /* = 255 */, UCHAR ucVolume2 /* = 255 */)
	{
	short sRes = 0;
	
	// Reset variables and free data if any.
	Reset();
	
	// Attempt to create RSample . . .
	m_psample = new RSample;
	if (m_psample != NULL)
		{
		// Remember we're responsible for de-allocating this RSample.
		m_sOwnSample	= TRUE;
		if (m_psample->Open(pszSampleName, lReadBufSize) > 0L)
			{
			// Attempt to open a sound channel . . .
			if (m_mix.OpenChannel(	m_psample->m_lSamplesPerSec, 
											m_psample->m_sBitsPerSample, 
											m_psample->m_sNumChannels) == 0)
				{
				// Store the buffer size to stream with.
				m_lBufSize	= lPlayBufSize;
				// Attempt to start the mixing . . .
				if (m_mix.Start(StreamCallStatic, (ULONG)this) == 0)
					{
					// Success.  Set state to starting.
					m_sState	= Starting;
					}
				else
					{
					TRACE("Stream(\"%s\"): Unable to start mixer.\n", pszSampleName);
					sRes = -7;
					}

				// If any failures . . .
				if (sRes != 0)
					{
					if (m_mix.CloseChannel() != 0)
						{
						TRACE("Stream(\"%s\"): Unable to close sound channel.\n", pszSampleName);
						}
					}
				}
			else
				{
				TRACE("Stream(\"%s\"): Unable to open sound channel.\n", pszSampleName);
				sRes = -6;
				}

			// If any failures . . .
			if (sRes != 0)
				{
				// Close sample.
				m_psample->Close();
				}
			}
		else
			{
			TRACE("Stream(\"%s\"): Unable to open sample file.\n", pszSampleName);
			sRes = -5;
			}

		// If any failures . . .
		if (sRes != 0)
			{
			delete m_psample;
			m_psample = NULL;
			}
		}
	else
		{
		TRACE("Stream(\"%s\"): Unable to allocate RSample.\n", pszSampleName);
		sRes = -3;
		}

	return sRes;
	}
	
///////////////////////////////////////////////////////////////////////////////
//
// Plays RSample supplied via ptr with buffer size of lPlayBufSize
// (this is the size of the chunks sent to RMix).
// (public)
// Note on looping:
//	 Start																		  End
//		1-----------------------------------------------------------4
//				2=======================================3
//			lLoopStartTime									lLoopEndTime
//
// In a looping scenario, 1..2 of the sample is played, then 2..3
// is repeated until m_sLoop is FALSE, at which time, once 3 is reached, 
// 3..4 is played.
//
///////////////////////////////////////////////////////////////////////////////
short RSnd::Play(						// Returns 0 on success.
	RSample* psample,					// In:  Sample to play.
	long lPlayBufSize,				// In:  Size of play buffer in bytes.
	UCHAR	ucMainVolume/* = 255 */,// In:  Primary Volume (0 - 255)
	UCHAR ucVolume2 /* = 255 */,	// In:  Secondary Volume (0 - 255)
	long lLoopStartTime/* = -1*/,	// In:  Where to loop back to in milliseconds.
											//	-1 indicates no looping (unless m_sLoop is
											// explicitly set).
	long lLoopEndTime/* = 0*/)		// In:  Where to loop back from in milliseconds.
											// In:  If less than 1, the end + lLoopEndTime is used.
	{
	short sRes = 0; // Assume success.
	
	ASSERT(psample != NULL);
	ASSERT(GetState() == Stopped);

	// Use supplied sample.
	m_psample		= psample;
	// Remember we're not responsible for this buffer (i.e., freeing it).
	m_sOwnSample	= FALSE;

	// Attempt to lock sample . . .
	if (m_psample->Lock() == 0)
		{
		// Attempt to open a sound channel . . .
		if (m_mix.OpenChannel(	m_psample->m_lSamplesPerSec, 
										m_psample->m_sBitsPerSample,
										m_psample->m_sNumChannels) == 0)
			{
			// Store the buffer size to stream with.
			m_lBufSize = lPlayBufSize;
			// Attempt to play buffer . . .
			if (m_mix.Start(PlayCallStatic, (ULONG)this,ucMainVolume,ucVolume2) == 0)
				{
				// Success.  Set state to starting.
				m_sState				= Starting;
				if (lLoopStartTime > -1)
					{
					// Set looping parameters.
					m_sLoop				= TRUE;
					m_lLoopStartPos	= psample->GetPos(lLoopStartTime);
					m_lLoopEndPos		= psample->GetPos(lLoopEndTime);
					// If using the end . . .
					if (lLoopEndTime < 1)
						{
						// Use the duration plus the specified negative time.
						m_lLoopEndPos		+= psample->m_lBufSize;
						}

					// Cannot be off end of buffer or beginning of buffer.
					ASSERT(m_lLoopStartPos <= psample->m_lBufSize);
					ASSERT(m_lLoopEndPos <= psample->m_lBufSize);
					ASSERT(m_lLoopStartPos >= 0);
					ASSERT(m_lLoopStartPos < m_lLoopEndPos);

					// Fix these values in case we're in release mode.
					if (m_lLoopStartPos > psample->m_lBufSize)
						{
						m_lLoopStartPos = psample->m_lBufSize;
						}

					if (m_lLoopEndPos > psample->m_lBufSize)
						{
						m_lLoopEndPos = psample->m_lBufSize;
						}

					if (m_lLoopStartPos < 0)
						{
						m_lLoopStartPos = 0;
						}

					if (m_lLoopStartPos > m_lLoopEndPos)
						{
						m_lLoopStartPos = m_lLoopEndPos;
						}
					}
				else	// Backwards compatability.
					{
					// If m_sLoop was set . . .
					if (m_sLoop != FALSE)
						{
						m_lLoopStartPos	= 0;
						// Use end of sample as loop back point.
						m_lLoopEndPos		= psample->m_lBufSize;
						}
					}
				}
			else
				{
				TRACE("Play(): Unable to play sound buffer.\n");
				sRes = -3;
				}

			// If any failures . . .
			if (sRes != 0)
				{
				if (m_mix.CloseChannel() != 0)
					{
					TRACE("Play(): Unable to close sound channel.\n");
					}
				}
			}
		else
			{
            // commented out due to spam from --nosound.  --ryan.
			//TRACE("Play(): Unable to open sound output device.\n");
			sRes = -2;
			}

		// If any failures . . .
		if (sRes != 0)
			{
			// Unlock sample.
			m_psample->Unlock();
			// We have no more use for this sample.
			m_psample = NULL;
			}
		}
	else
		{
		TRACE("Play(): Unable to lock supplied sample.\n");
		sRes = -1;
		}

	return sRes;
	}

///////////////////////////////////////////////////////////////////////////////
//
// Aborts current play or stream.
// Returns 0 on success.
// (public)
//
///////////////////////////////////////////////////////////////////////////////
short RSnd::Abort(void)
	{
	short sRes = 0; // Assume success.

	ASSERT(GetState() != Stopped);

	// If we have a sample (i.e., we are streaming/playing) . . .
	if (m_psample != NULL)
		{
		// Unlock the sample.
		m_psample->Unlock();
		
		// Call callback, if specified.
		if (m_dcUser != NULL)
			{
			(*m_dcUser)(this);
			}

		if (m_sOwnSample == TRUE)
			{
			// Destroy sample.
			delete m_psample;
			// We no longer own the sample.
			m_sOwnSample = FALSE;
			}

		// Clear the sample ptr.  This will cause streaming/playing to end.
		m_psample = NULL;

		// Let RMix know we want to abort.
		if (m_mix.Suspend() == 0)
			{
			}
		else
			{
			TRACE("Abort(): RMix::Suspend() failed.\n");
			}
		}
	else
		{
//		TRACE("Abort(): No current RSample.\n");
		sRes = -1;
		}
	
	return sRes;
	}
	
//////////////////////////////////////////////////////////////////////////////
//
// Gets/returns the current position of the audio in bytes.
// (public)
//
//////////////////////////////////////////////////////////////////////////////
long RSnd::GetPos(void)
	{
	return m_mix.GetPos();
	}

//////////////////////////////////////////////////////////////////////////////
//
// Gets/returns the current time of the audio in milliseconds.
// (public)
//
//////////////////////////////////////////////////////////////////////////////
long RSnd::GetTime(void)
	{
	return m_mix.GetTime();
	}

//////////////////////////////////////////////////////////////////////////////
//
// Called from StreamCallStatic when Blue is done playing our buffer.
// Returns current volume information back to the RMix
// Returns pointer to next buffer to play or NULL to end.
//
//////////////////////////////////////////////////////////////////////////////
void* RSnd::StreamCall(RMix::Msg	msg, 
								void*		pData, 
								ULONG*	pulBufSize,
								ULONG		ulUser,
								UCHAR*		pucVolume,
								UCHAR*		pucVol2)
	{
	switch (msg)
		{
		case RMix::Data:
			// If not done . . .
			if (m_psample != NULL)
				{
				// If we were starting . . .
				if (m_sState == Starting)
					{
					// Switch to queueing.
					m_sState = Queueing;
					}

				// Read a buffer . . .
				(*pulBufSize) = m_psample->Read(m_lBufSize);
				if (*pulBufSize > 0)
					{
					// Set pointer.  Technically, m_psample's pointer could change
					// time we call Read().  Remember that!
					pData = m_psample->m_pData;

					// If the message is not queueing and our state is queueing . . .
					if (m_sState == Queueing)
						{
						// Switch state to playing.
						m_sState	= Playing;
						}
					}
				else
					{
					// If done . . .
					if (*pulBufSize == 0)
						{
						}
					else
						{
						TRACE("StreamCall(): Error reading sample data.\n");
						}

					// End streaming.
					pData = NULL;
					}
				}
			else
				{
				// Aborted.
				pData = NULL;
				}

			// If abort, done, or error . . .
			if (pData == NULL)
				{
				// Set state to ending.
				m_sState	= Ending;
				}
			break;

		case RMix::Suspended:
			// If sample still pointed to (this is the case when the sound ends
			// normally (i.e., Abort() was NOT called) ). . .
			if (m_psample != NULL)
				{
				// Close the sample.
				if (m_psample->Close() != 0)
					{
					TRACE("StreamCall(): Unable to close RSample.\n");
					}

				// Call callback, if specified.
				if (m_dcUser != NULL)
					{
					(*m_dcUser)(this);
					}

				// If we own the sample . . .
				if (m_sOwnSample == TRUE)
					{
					// Delete it.
					delete m_psample;
					// No longer own it.
					m_sOwnSample = FALSE;
					}

				m_psample = NULL;
				}

			// Close the sound output channel.
			if (m_mix.CloseChannel() != 0)
				{
				TRACE("StreamCall(): Unable to close sound channel.\n");
				}

			// Set state to stopped.
			m_sState	= Stopped;
			
			break;
		}

	// Get current Sound from internal members
	if (pucVolume) *pucVolume = m_sChannelVolume;
	if (pucVol2) *pucVol2 = m_sTypeVolume;

	// Return next buffer to play.
	return pData;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Called from PlayCallStatic when Blue is done playing our buffer.
// Returns pointer to next buffer to play or NULL to end.
// Sends back current information back to RMix
// Recursive 1 deep.
//
//////////////////////////////////////////////////////////////////////////////
void* RSnd::PlayCall(RMix::Msg	msg,
							void*			pData, 
							ULONG*		pulBufSize,
							UCHAR*		pucVolume,
							UCHAR*		pucVol2)
	{
	switch (msg)
		{
		case RMix::Data:
			// If not done . . .
			if (m_psample != NULL)
				{
				// If we were starting . . .
				if (m_sState == Starting)
					{
					// Switch to queueing.
					m_sState = Queueing;
					// Set remaining amount of data to play.
					m_ulRemaining	= (m_sLoop == FALSE) ? m_psample->m_lBufSize : m_lLoopEndPos;
					// Set size of data to play.
					m_ulSampleSize	= m_ulRemaining;
					}

				// Move to next buffer.
				pData = (UCHAR*)(m_psample->m_pData) + (m_ulSampleSize - m_ulRemaining);

				// Get next buffer size.
				(*pulBufSize) = MIN((ULONG)m_lBufSize, m_ulRemaining);

				if (*pulBufSize > 0)
					{
					// Deduct amount to play.
					m_ulRemaining = m_ulRemaining - (*pulBufSize);

					// If the message is not queueing and our state is queueing . . .
					if (m_sState == Queueing)
						{
						// Switch state to playing.
						m_sState	= Playing;
						}
					}
				else
					{
					// If looping is disabled . . .
					if (m_sLoop == FALSE)
						{
						// If we were NOT looping a sub region . . .
						if (m_ulSampleSize == (ULONG)m_psample->m_lBufSize)
							{
							// Clear pointer.
							pData		= NULL;
							}
						else
							{
							// Play the rest.
							// Set remaining amount of data to play.
							m_ulRemaining	= m_psample->m_lBufSize - m_lLoopEndPos;
							// Set size of data to play.
							m_ulSampleSize	= m_psample->m_lBufSize;

							// Recurse.
							pData	= PlayCall(msg, pData, pulBufSize, pucVolume, pucVol2);
							}
						}
					else
						{
						// Restart at loop pos.
						// Set remaining amount of data to play.
						m_ulRemaining	= m_lLoopEndPos - m_lLoopStartPos;
						// Set size of data to play.
						m_ulSampleSize	= m_lLoopEndPos;

						// Recurse.
						pData	= PlayCall(msg, pData, pulBufSize, pucVolume, pucVol2);
						}
					}
				}
			else
				{
				// Clear pointer.
				pData	= NULL;
				}

			// If done or aborted . . .
			if (pData == NULL)
				{
				// Set state to ending.
				m_sState	= Ending;
				}
			break;

		case RMix::Suspended:

			if (m_psample != NULL)
				{
				// Unlock the sample.
				m_psample->Unlock();

				// Call callback, if specified.
				if (m_dcUser != NULL)
					{
					(*m_dcUser)(this);
					}
				}

			// Close the sound output channel.
			if (m_mix.CloseChannel() != 0)
				{
				TRACE("PlayCall(): Unable to close sound channel.\n");
				}
#if 0
			TRACE("Remaining: %lu, Size: %lu, State: %s, Sample: 0x%08lx.\n",
					m_ulRemaining, m_ulSampleSize,
					(m_sState == Ending ? "Ending" :
						(m_sState == Playing ? "Playing" :
							(m_sState == Queueing ? "Queueing" :
								(m_sState == Starting ? "Starting" :
									(m_sState == Stopped ? "Stopped" : "Invalid") ) ) ) ),
					m_psample
					);
#endif

			// Set state to stopped.
			m_sState	= Stopped;

			// Clear sample.  No longer needed.
			m_psample = NULL;

			break;
		}

	// Get current Sound from internal members
	if (pucVolume) *pucVolume = m_sChannelVolume;
	if (pucVol2) *pucVol2 = m_sTypeVolume;

	// Return next buffer to play.
	return pData;
	}

////////////////////////////// Static functions //////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Called from Blue when it is done playing our buffer.
// Sends back up to the second volume information back to RMix
// Returns pointer to next buffer to play or NULL to end.
// (static)
//
//////////////////////////////////////////////////////////////////////////////
void* RSnd::StreamCallStatic(	RMix::Msg	msg, 
										void*			pData, 
										ULONG*		pulBufSize, 
										ULONG			ulUser,
										UCHAR*		pucVolume,
										UCHAR*		pucVol2)
	{
	return ((PSND)ulUser)->StreamCall(msg, pData, pulBufSize,ulUser,pucVolume,pucVol2);
	}

//////////////////////////////////////////////////////////////////////////////
//
// Called from Blue when it is done playing our buffer.
// Sends back up to the second volume information back to RMix
// Returns pointer to next buffer to play or NULL to end.
// (static)
//
//////////////////////////////////////////////////////////////////////////////
void* RSnd::PlayCallStatic(RMix::Msg	msg, 
									void*			pData, 
									ULONG*		pulBufSize, 
									ULONG			ulUser,
									UCHAR*		pucVolume,
									UCHAR*		pucVol2)
	{
	return ((PSND)ulUser)->PlayCall(msg, pData, pulBufSize, pucVolume, pucVol2);
	}

//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
