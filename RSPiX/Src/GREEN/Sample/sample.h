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
//	sample.cpp
// 
// History:
//		06/23/95 JMI	Started.
//
//		01/28/97	JMI	Added Load(RFile*), Save(char*), and Save(RFile*).
//
//		05/09/97	JMI	Added GetDuration(), GetTime(), and GetPos() functions.
//
//////////////////////////////////////////////////////////////////////////////
//
//	See CPP for description of this API.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef SAMPLE_H
#define SAMPLE_H

//////////////////////////////////////////////////////////////////////////////
// Headers.
//////////////////////////////////////////////////////////////////////////////
#include "System.h"

// If PATHS_IN_INCLUDES macro is defined, we can utilized relative
// paths to a header file.  In this case we generally go off of our
// RSPiX root directory.  System.h MUST be included before this macro
// is evaluated.  System.h is the header that, based on the current
// platform (or more so in this case on the compiler), defines 
// PATHS_IN_INCLUDES.  Blue.h includes system.h so you can include that
// instead.
#ifdef PATHS_IN_INCLUDES
	#include "ORANGE/IFF/iff.h"
#else
	#include "iff.h"
#endif // PATHS_IN_INCLUDES

//////////////////////////////////////////////////////////////////////////////
// Macros.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Types.
//////////////////////////////////////////////////////////////////////////////
// Forward declare RSample for typedef.
class RSample;

// Handy-dandy typedef.
typedef RSample* PSAMPLE;

class RSample
	{
	public:
		// Default constructor.
		RSample();
		// Constructor especial.
		RSample(	void *pData, int32_t lBufSize, 
					int32_t lSamplesPerSec, int16_t sBitsPerSample, int16_t sNumChannels);
		// Destructor.
		~RSample();

	////////////////////////// Querries ///////////////////////////////////////
	public:
		// Returns the reference count.  Non-zero == locked, Zero == not locked,
		// Negative == fucked.
		int16_t IsLocked(void) { return m_sRefCnt; }

		// Get the duration of this sample in milliseconds.
		int32_t GetDuration(void)
			{
			return GetTime(m_lBufSize);
			}

		// Get the time in milliseconds indicated by the specified byte position
		// in the sample.
		int32_t GetTime(
			int32_t lPos)	// In:  Position in bytes.
			{
			return lPos / (m_lSamplesPerSec * m_sNumChannels * m_sBitsPerSample / 8000);
			}

		// Get the position in bytes indicated by the specified time in milliseconds
		// in the sample.
		int32_t GetPos(
			int32_t lTime)	// In:  Time in milliseconds.
			{
			return lTime * (m_lSamplesPerSec * m_sNumChannels * m_sBitsPerSample / 8000);
			}
		
	////////////////////////// Methods ////////////////////////////////////////
	public:
		// Reset object.  Release sample data and reset variables.
		void Reset(void);

		// Open a file and read the header.  Locks the RSample automatically.
		// Returns the size of the file's data on success, negative otherwise.
		int32_t Open(char* pszSampleName, int32_t lReadBufSize);

		// Read the specified amount of data from the open file.
		// Returns amount read on success, negative on failure.
		int32_t Read(int32_t lAmount);
		
		// Close the file opened with Open.  Unlocks the RSample automatically.
		// Returns 0 on success.
		int16_t Close(void);

		// Load an entire sample from a file.
		// Returns 0 on success.
		int16_t Load(char* pszSampleName);
		// Same as above, but accepts an open RFile*.
		int16_t Load(				// Returns 0 on success.
			RFile*	pfile);	// Open RFile.

		// Saves entire sample to file specified in RIFF WAVE format.
		int16_t Save(					// Returns 0 on success.
			char* pszFileName);	// Filename for sample file.

		// Saves entire sample to file specified in RIFF WAVE format.
		int16_t Save(					// Returns 0 on success.
			RFile* pfile);			// Open RFile for sample.

		// Lock this sample for use.  Returns 0 on success.
		int16_t Lock(void);

		// Unlock this sample from use.  Returns new reference count.
		int16_t Unlock(void);

		// Convert current sample data from 8 bit unsigned to 16 bit signed.
		// Returns 0 on success.
		int16_t Convert8to16(void);

	////////////////////////// Internal Methods ///////////////////////////////
	protected:
		// Initialize instantiable members.
		void Init(void);

		// Read WAVE info from m_file.
		// Returns size of sample data on success, negative on error.
		int32_t ReadWaveHeader(void);

	////////////////////////// Member vars ////////////////////////////////////
	public:
		int32_t			m_lBufSize;			// Amount of data in CSnd.
		int32_t			m_lSamplesPerSec;	// Rate of data in buffer.
		int16_t			m_sNumChannels;	// Number of channels (i.e., 1==mono,
												// 2==stereo).
		int16_t			m_sBitsPerSample;	// Number of bits per sample.
		void*			m_pData;				// Buffer used for sample data.

	protected:
		int16_t			m_sOwnBuf;		// TRUE if play buffer allocated by RSample.
		int16_t			m_sRefCnt;		// Reference count of locks on this sample's
											// data.
		RIff			m_iff;				// File stream pointer for reading/streaming.

		/////////////////////// Static members /////////////////////////////////
	
	};

#endif // SAMPLE_H
//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
