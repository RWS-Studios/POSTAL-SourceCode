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
			UCHAR* pu8Dst,	// In:  Destination buffer.
			ULONG ulSize);	// In:  Size of destination buffer in bytes.
		// Destructor.
		~RMixBuf();

	public:	// Methods.
		// Silence buffer.
		void Silence(void);
		
		// Set size of mix buffer in bytes. (Allocates buffer).
		// Returns 0 on success.
		short SetSize(ULONG ulSize);

		// Set the destination buffer.
		void SetDest(
			UCHAR* pu8Dst,	// In:  Destination buffer.
			ULONG ulSize);	// In:  Size of destination buffer in bytes.

		// Mix data in.
		// Returns 0 on success.
		short Mix(	ULONG		ulStartPos,
						U8*		pu8Data, 
						ULONG		ulSize, 
						long		lSampleRate,
						long		lBitsPerSample,
						long		lNumChannels,
						UCHAR		ucVolume = UCHAR(255),
						UCHAR		ucVol2 = UCHAR(255) );

		// Prepare for destination.  If necessary, converts to destination format.
		void PrepareForDest(void);

	public:	// Querries.
		
		// Get ptr to destination data.
		void* GetDstData(void)	{ return m_pu8Dst; }

		// Get ptr to mix data.
		void*	GetMixData(void)	{ return m_pu8Mix; }

		// Get size of destination buffer in bytes.
		ULONG GetDstSize(void)	{ return m_ulDstSize; }

		// Get size of mix buffer in bytes.
		ULONG GetMixSize(void)	{ return m_ulMixSize; }

		// Returns number of RMixBufs allocated.
		static short Num(void)		{ return ms_sNumBufs; }


	public:	// Statics.

		// Sets the global volume for mixed sounds:
		static void	SetVolume(UCHAR ucVolume)
			{
			ms_ucGlobalVolume = ucVolume;
			}

		// Reads the current global volume for mixed sounds:
		static UCHAR GetVolume()
			{
			return ms_ucGlobalVolume;
			}

		// Allows user to set the cut off volume for mixing sound:
		static void SetCutOff(short sVolume)
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
		short			m_sOwnMixBuf;			// TRUE if RMixBuf allocated the mix buffer.
		ULONG			m_ulMixSize;			// Size of mix buffer in bytes.
		ULONG			m_ulDstSize;			// Size of dst buffer in bytes.

		static short	ms_sNumBufs;		// Number of RMixBufs allocated.
		static UCHAR	ms_ucGlobalVolume;// Scale all mixes relative to this
	
	public:	// It is safe to change these.
		short			m_sInUse;				// TRUE if in use, FALSE otherwise.

		static long	ms_lSampleRate;		// Sample rate for audio 
													// playback/mix.
		
		static long	ms_lSrcBitsPerSample;	// Sample size in bits for sample data.
														// 0 for no preference.
		static long	ms_lMixBitsPerSample;	// Sample size in bits for mixing.
		static long	ms_lDstBitsPerSample;	// Sample size in bits for Blue data.

		static long	ms_lNumChannels;		// Number of channels (mono
													//  or stereo).
		static short ms_sCutOffVolume;	// when to not mix samples...
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
	inline	UCHAR	ScaleByte(UCHAR ucByte,UCHAR	ucLevel)
		{
		return UCHAR(CDVA::ms_asHighByte[DVA_SIZE + 
			(ucLevel>>DVA_SHIFT)][ucByte]);
		}

	// More efficient for a block of data scaled the same:
	inline	void	ScaleBytes(short sNumBytes,UCHAR* pucBytesIn,
		UCHAR* pucBytesOut,UCHAR ucLevel)
		{
		ASSERT(pucBytesIn);
		ASSERT(pucBytesOut);

        // Unsigned, always true. --ryan.
		//ASSERT(ucLevel < 256);

		short* psTable = CDVA::ms_asHighByte[DVA_SIZE + (ucLevel>>DVA_SHIFT)];	

		short i;
		for (i=0;i < sNumBytes;i++) 
			{
			pucBytesOut[i] = UCHAR(psTable[pucBytesIn[i]]);
			}
		}

	// we can also scale 16-bit values, but this is not yet accessible to the user.

	//----------------------------------------------------------------------
	CDVA();
	~CDVA(){};	// nothing to do....
	//----------------------------------------------------------------------
	// To save on registers, make this the same array:
	static short	ms_asHighByte[DVA_SIZE * 2][256];	// for 16-bit sound

	};


#endif // MIXBUF_H
///////////////////////////////////////////////////////////////////////////////
// EOF
///////////////////////////////////////////////////////////////////////////////
