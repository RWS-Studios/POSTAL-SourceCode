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
//	MixBuf.h

#ifndef MIXBUF_H
#define MIXBUF_H

///////////////////////////////////////////////////////////////////////////////
// Macros.
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Types.
///////////////////////////////////////////////////////////////////////////////

// Forward declare class for handy-dandy typedef.
class RMixBuf;

// Handy-dandy typedef.
typedef RMixBuf* PMIXBUF;

// Class definition.
class RMixBuf
	{
	public:	// <Con|De>struction.
		// Default constructor.
		RMixBuf();
		// Constructor Especial.
		RMixBuf(				// Returns whatever it is a constructor returns.
			uint8_t* pu8Dst,	// In:  Destination buffer.
			uint32_t ulSize);	// In:  Size of destination buffer in bytes.
		// Destructor.
		~RMixBuf();

	public:	// Methods.
		// Silence buffer.
		void Silence(void);
		
		// Set size of mix buffer in bytes. (Allocates buffer).
		// Returns 0 on success.
		int16_t SetSize(uint32_t ulSize);

		// Set the destination buffer.
		void SetDest(
			uint8_t* pu8Dst,	// In:  Destination buffer.
			uint32_t ulSize);	// In:  Size of destination buffer in bytes.

		// Mix data in.
		// Returns 0 on success.
		int16_t Mix(	uint32_t		ulStartPos,
						U8*		pu8Data, 
						uint32_t		ulSize, 
						int32_t		lSampleRate,
						int32_t		lBitsPerSample,
						int32_t		lNumChannels,
						uint8_t		ucVolume = uint8_t(255),
						uint8_t		ucVol2 = uint8_t(255) );

		// Prepare for destination.  If necessary, converts to destination format.
		void PrepareForDest(void);

	public:	// Querries.
		
		// Get ptr to destination data.
		void* GetDstData(void)	{ return m_pu8Dst; }

		// Get ptr to mix data.
		void*	GetMixData(void)	{ return m_pu8Mix; }

		// Get size of destination buffer in bytes.
		uint32_t GetDstSize(void)	{ return m_ulDstSize; }

		// Get size of mix buffer in bytes.
		uint32_t GetMixSize(void)	{ return m_ulMixSize; }

		// Returns number of RMixBufs allocated.
		static int16_t Num(void)		{ return ms_sNumBufs; }


	public:	// Statics.

		// Sets the global volume for mixed sounds:
		static void	SetVolume(uint8_t ucVolume)
			{
			ms_ucGlobalVolume = ucVolume;
			}

		// Reads the current global volume for mixed sounds:
		static uint8_t GetVolume()
			{
			return ms_ucGlobalVolume;
			}

		// Allows user to set the cut off volume for mixing sound:
		static void SetCutOff(int16_t sVolume)
			{
			if ( (sVolume >= 0) && (sVolume < 256) )
				{
				ms_sCutOffVolume = sVolume;
				}
			}

	protected:	// Internal use.
		// Intialize members.
		void Init(void);
		// Free dynamic data and re-init.
		void Reset(void);

	protected:	// Members.
		U8*			m_pu8Mix;				// Mix buffer.
		U8*			m_pu8Dst;				// Destination buffer.
		int16_t			m_sOwnMixBuf;			// TRUE if RMixBuf allocated the mix buffer.
		uint32_t			m_ulMixSize;			// Size of mix buffer in bytes.
		uint32_t			m_ulDstSize;			// Size of dst buffer in bytes.

		static int16_t	ms_sNumBufs;		// Number of RMixBufs allocated.
		static uint8_t	ms_ucGlobalVolume;// Scale all mixes relative to this
	
	public:	// It is safe to change these.
		int16_t			m_sInUse;				// TRUE if in use, FALSE otherwise.

		static int32_t	ms_lSampleRate;		// Sample rate for audio 
													// playback/mix.
		
		static int32_t	ms_lSrcBitsPerSample;	// Sample size in bits for sample data.
														// 0 for no preference.
		static int32_t	ms_lMixBitsPerSample;	// Sample size in bits for mixing.
		static int32_t	ms_lDstBitsPerSample;	// Sample size in bits for Blue data.

		static int32_t	ms_lNumChannels;		// Number of channels (mono
													//  or stereo).
		static int16_t ms_sCutOffVolume;	// when to not mix samples...
	};

//////////////////////////////////////////////////////////////////////////////
//
// Build table for dynamic volume adjustment (DVA) - now available to the
// the outside world
//
//////////////////////////////////////////////////////////////////////////////

// Cutting back on the number of volume levels is strictly to save memory
#define	DVA_RESOLUTION	1	// significant volume change in 0-255
#define	DVA_SHIFT		0	// must match the resolution in shift bits
#define	DVA_SIZE		(256>>DVA_SHIFT)	// size of tables
#define	DVA_LOW_OFFSET	(256 * DVA_SIZE * 2)

class	CDVA	// a complete dummy
	{
public:
	//////////////////////////////////////////////////////////////////////////////
	//  Allow use of mixing volume for other applications:
	//  A level of 255 is identity.
	//////////////////////////////////////////////////////////////////////////////
	inline	uint8_t	ScaleByte(uint8_t ucByte,uint8_t	ucLevel)
		{
		return uint8_t(CDVA::ms_asHighByte[DVA_SIZE + 
			(ucLevel>>DVA_SHIFT)][ucByte]);
		}

	// More efficient for a block of data scaled the same:
	inline	void	ScaleBytes(int16_t sNumBytes,uint8_t* pucBytesIn,
		uint8_t* pucBytesOut,uint8_t ucLevel)
		{
		ASSERT(pucBytesIn);
		ASSERT(pucBytesOut);

        // Unsigned, always true. --ryan.
		//ASSERT(ucLevel < 256);

		int16_t* psTable = CDVA::ms_asHighByte[DVA_SIZE + (ucLevel>>DVA_SHIFT)];	

		int16_t i;
		for (i=0;i < sNumBytes;i++) 
			{
			pucBytesOut[i] = uint8_t(psTable[pucBytesIn[i]]);
			}
		}

	// we can also scale 16-bit values, but this is not yet accessible to the user.

	//----------------------------------------------------------------------
	CDVA();
	~CDVA(){};	// nothing to do....
	//----------------------------------------------------------------------
	// To save on registers, make this the same array:
	static int16_t	ms_asHighByte[DVA_SIZE * 2][256];	// for 16-bit sound

	};


#endif // MIXBUF_H
///////////////////////////////////////////////////////////////////////////////
// EOF
///////////////////////////////////////////////////////////////////////////////
