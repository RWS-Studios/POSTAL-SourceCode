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
//	bsound.cpp
// 
// History:
//		10/29/95 JMI	Started.  Based on original WinMM bsound.cpp.
//							This is now the interface by which Blue decides into
//							which namespace to call.
//
//		08/05/97	JMI	Added rspIsSoundOutPaused().
//
//////////////////////////////////////////////////////////////////////////////
//
// This calls into the correct namespace for sound functionality based on
// the current value of gsi.sAudioType via the AUDIO_CALL macro defined
// in wSound.h.
//
//////////////////////////////////////////////////////////////////////////////

#include "SDL.h"
#include "Blue.h"

// Only set value if not NULL.
#define SET(ptr, val)		( ((ptr) != NULL) ? *(ptr) = (val) : 0)

static ULONG callback_data = 0;

static void sdl_audio_callback(void *userdata, Uint8 *stream, int len)
{
    RSP_SND_CALLBACK callback = (RSP_SND_CALLBACK) userdata;
    SDL_memset(stream, '\0', len);
    callback(stream, len, 0, &callback_data);
}

static bool audio_opened = false;
static SDL_AudioSpec desired;
static long cur_buf_time = 0;
static long max_buf_time = 0;

extern short rspSetSoundOutMode(				// Returns 0 if successfull, non-zero otherwise
	long lSampleRate,								// In:  Sample rate
	long lBitsPerSample,							// In:  Bits per sample
	long lChannels,								// In:  Channels (mono = 1, stereo = 2)
	long lCurBufferTime,							// In:  Current buffer time (in ms.)
	long lMaxBufferTime,							// In:  Maximum buffer time (in ms.)
	RSP_SND_CALLBACK callback,					// In:  Callback function
	ULONG ulUser)									// In:  User-defined value to pass to callback
{
    if (audio_opened)
        rspKillSoundOutMode();

    memset(&desired, '\0', sizeof (desired));

    if ((lChannels < 1) || (lChannels > 2))
    {
		TRACE("rspSetSoundOutMode(): Must be 1 or 2 channels.\n");
        return -1;
    }

    if (lBitsPerSample == 8)
        desired.format = AUDIO_U8;
    else if (lBitsPerSample == 16)
        desired.format = AUDIO_S16SYS;
    else
    {
		TRACE("rspSetSoundOutMode(): Format must be 8 or 16 bit.\n");
        return -1;
    }

    // Fragment sizes I used for Serious Sam...seem to work well...
    if (desired.freq <= 11025)
        desired.samples = 512;
    else if (desired.freq <= 22050)
        desired.samples = 1024;
    else if (desired.freq <= 44100)
        desired.samples = 2048;
    else
        desired.samples = 4096;  // (*shrug*)

#ifdef __ANDROID__
    desired.samples = 2048;
#endif

    desired.freq = lSampleRate;
    desired.channels = lChannels;
    desired.callback = sdl_audio_callback;
    desired.userdata = (void *) callback;

    cur_buf_time = lCurBufferTime;
    max_buf_time = lMaxBufferTime;

    callback_data = ulUser;

    if (rspCommandLine("nosound"))
        return BLU_ERR_NO_DEVICE;

    if (!SDL_WasInit(SDL_INIT_AUDIO))
    {
        if (SDL_Init(SDL_INIT_AUDIO) == -1)
            return BLU_ERR_NO_DEVICE;
    }

    if (SDL_OpenAudio(&desired, NULL) == -1)
    {
		TRACE("rspSetSoundOutMode(): SDL_OpenAudio failed: %s.\n", SDL_GetError());
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        return BLU_ERR_NO_DEVICE;
    }

    SDL_PauseAudio(0);

    audio_opened = true;
    return 0;
}

extern short rspGetSoundOutMode(				// Returns 0 if successfull, non-zero otherwise
	long* plSampleRate,							// Out: Sample rate or -1 (unless NULL)
	long* plBitsPerSample,				// Out: Bits per sample or -1 (unless NULL)
	long* plChannels,					// Out: Channels (mono=1, stereo=2) or -1 (unless NULL)
	long* plCurBufferTime,				// Out: Current buffer time or -1 (unless NULL)
	long* plMaxBufferTime)			// Out: Maximum buffer time or -1 (unless NULL)
{
    SET(plSampleRate, desired.freq);
    SET(plBitsPerSample, desired.format & 0x00FF);
    SET(plBitsPerSample, desired.channels);
    SET(plCurBufferTime, cur_buf_time);
    SET(plMaxBufferTime, max_buf_time);
    return 0;
}

extern void rspSetSoundOutBufferTime(
	long lCurBufferTime)						// In:  New buffer time
{
    cur_buf_time = lCurBufferTime;
}

extern void rspKillSoundOutMode(void)		// Returns 0 if successfull, non-zero otherwise
{
    if (audio_opened)
    {
        SDL_PauseAudio(1);
        SDL_CloseAudio();
        audio_opened = false;
    }
}

extern short rspClearSoundOut(void)		// Returns 0 on success, non-zero otherwise
{
    // no-op?
    return 0;
}

extern short rspPauseSoundOut(void)		// Returns 0 on success, non-zero otherwise
{
    if (!audio_opened)
        return 0;

    SDL_PauseAudio(1);
    return 0;
}

extern short rspResumeSoundOut(void)		// Returns 0 on success, non-zero otherwise
{
    if (!audio_opened)
        return 0;

    SDL_PauseAudio(0);
    return 0;
}

extern short rspIsSoundOutPaused(void)	// Returns TRUE if paused, FALSE otherwise
{
    if (!audio_opened)
        return TRUE;

    return((SDL_GetAudioStatus() == SDL_AUDIO_PAUSED) ? TRUE : FALSE);
}

extern long rspGetSoundOutPos(void)		// Returns sound output position in bytes
{
    return 0;
}

extern long rspGetSoundOutTime(void)		// Returns sound output position in time
{
    return 0;
}

extern long rspDoSound(void)
{
    // no-op; in Soviet Russia, audio callback pumps YOU.
    //  seriously, the SDL audio callback runs in a seperate thread.
    return 0;
}

extern void rspLockSound(void)
{
    if (audio_opened)
        SDL_LockAudio();
}

extern void rspUnlockSound(void)
{
    if (audio_opened)
        SDL_UnlockAudio();
}


//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
