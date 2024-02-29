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
#ifndef SNDFX_H
#define SNDFX_H

#include <stdio.h> // For FILE*

#include "System.h"

// If PATHS_IN_INCLUDES macro is defined, we can utilized relative
// paths to a header file.  In this case we generally go off of our
// RSPiX root directory.  System.h MUST be included before this macro
// is evaluated.  System.h is the header that, based on the current
// platform (or more so in this case on the compiler), defines 
// PATHS_IN_INCLUDES.  Blue.h includes system.h so you can include that
// instead.
#ifdef PATHS_IN_INCLUDES
#else
#endif // PATHS_IN_INCLUDES

//////////////////////////////////////////////////////////////////////////////
// Macros.
//////////////////////////////////////////////////////////////////////////////
#define RSP_SNDFX_MAX_BITSPERSAMPLE		32			// Maximum bits per sample.
#define RSP_SNDFX_NUM_FADE_INTERVALS	10			// Size of fades table.

//////////////////////////////////////////////////////////////////////////////
// Types.
//////////////////////////////////////////////////////////////////////////////
// Forward declare RSndFx for typedef.
class RSndFx;

// Handy-dandy typedef.
typedef RSndFx* PSNDFX;

class RSndFx
	{
	/////////////////////// Typedefs & Enums ///////////////////////////////////
	public:
		// Effects that can be active simulatneously.
		// GetCurrentFX() returns these combined by | for all active fx:
		typedef enum
			{
			NoFX				= 0x0000,	// No current FX.
			// FX that affect all channels.
			FadeIn			= 0x0001,	// Fade in all channels.
			FadeOut			= 0x0002,	// Fade out all channels.
			// FX that affect individual channels.
			DecimateLeft	= 0x0004,	// Decimate left channel.	NYI.
			DecimateRight	= 0x0008		// Decimate right channel.	NYI.
			} FX;

	public:
		// Default constructor.
		RSndFx();
		// Destructor.
		~RSndFx();

	////////////////////////// Querries ///////////////////////////////////////
	public:
		// Returns the currently active fx as FX enums combined with |.
		FX GetCurrentFX(void)	{ return m_fx; }
		
	/////////////////////// Global Methods ////////////////////////////////////
	public:
		// Sets the fade accuracy (i.e., the number of steps to perform a fade).
		// This costs sNumStemps * 256 * bits per sample / 8 bytes of memory.
		// This function should only be called after the bits per sample have
		// been set via a call to SetDataType().
		static short SetFadeAccuracy(	// Returns 0 on success.
			short sNumSteps);				// Number of steps to fades; see above.

		// Release any dynamic memory referenced by static members.
		static void CleanUp(void);

	////////////////////////// Methods ////////////////////////////////////////
	public:
		// Release any dynamic memory and reset variables.
		// Clears all effects.
		void Clear(void);

		// Reset effects to start.
		void Reset(void);

		// Set type of PCM data in use.  This will reset all effects.
		static void SetDataType(	// Returns nothing.
			long lSamplesPerSec,		// Samples per second.
			long lBitsPerSample,		// Bits per sample.
			long lNumChannels);		// Number of channels.
		
		// Implements the effect on the provided buffer.
		void Do(								// Returns nothing.
			UCHAR* pucSrcData,			// Data to affect.
			long lBufSize,					// Amount of data.
			UCHAR* pucDstData = NULL);	// Destination for data, defaults to
												// same as source.
		
		/////////////////////////////////////////////////////////////////////////
		// Various FX.
		/////////////////////////////////////////////////////////////////////////

		/////////////////////// Fade In /////////////////////////////////////////

		// Set up a fade in.
		short SetUpFadeIn(	// Returns 0 on success.
			long lDuration);	// Duration until silence in milliseconds.

		// Activate/Deactivate fade in.
		void ActivateFadeIn(	// Returns nothing.
			short	sActivate);	// TRUE to activate, FALSE to deactivate.

		/////////////////////// Fade Out ////////////////////////////////////////

		// Set up a fade out.
		short SetUpFadeOut(	// Returns 0 on success.
			long lDuration);	// Duration until full volume in milliseconds.
		
		// Activate/Deactivate fade out.
		void ActivateFadeOut(	// Returns nothing.
			short	sActivate);		// TRUE to activate, FALSE to deactivate.

	////////////////////////// Internal Methods ///////////////////////////////
	protected:
		// Initialize instantiable members.
		void Init(void);

	////////////////////////// Member vars ////////////////////////////////////
	public:
				
	protected:
		FX		m_fx;				// Currently active effects.

		/////////////////////////////////////////////////////////////////////////
		// Various FX.
		/////////////////////////////////////////////////////////////////////////

		/////////////////////// Fade In /////////////////////////////////////////
		
		long	m_lFadeInMillisecondsDuration;	// Original duration.
		long	m_lFadeInBytesDurationAffected;	// Amount left to fade.
		long	m_lFadeInBytesDuration;				// Duration in bytes.

		long	m_lFadeInRate;

		/////////////////////// Fade Out ////////////////////////////////////////

		long	m_lFadeOutMillisecondsDuration;		// Original duration.
		long	m_lFadeOutBytesDurationRemaining;	// Amount left to fade.
		long	m_lFadeOutBytesDuration;				// Duration in bytes.

		long	m_lFadeOutRate;

		///////////////////// Protected Typedefs ///////////////////////////////
		typedef struct		// Stores info particular PCM type.
			{
			long	lMin;		// Mininum value (silence).
			long	lMax;		// Maximum value (saturation).
			} PCMINFO;

		/////////////////////// Static members /////////////////////////////////

		static long	ms_lSamplesPerSec;	// Samples per second.
		static long	ms_lBitsPerSample;	// Bits per sample.
		static long	ms_lNumChannels;		// Number of channels.

		static long	ms_lBitsPerSec;	// Number of bits per second.
												// Can be used to convert bytes to milliseconds
												// and convert milliseconds into bytes.  See
												// macros BYTES2MS and MS2BYTES in SndFx.CPP.

		static PCMINFO	ms_apcminfo[RSP_SNDFX_MAX_BITSPERSAMPLE + 1];	// Stores info 
																			// particular to each 
																			// PCM type.

		static U8*		ms_pu8Fade;			// Unsigned 8 bit output
													// fade table.
		static S16*		ms_ps16Fade;		// Signed 16 bit output
													// fade table.

		static short	ms_sNumFadeSteps;		// Number of fade steps.
	};

#endif // SNDFX_H
//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
