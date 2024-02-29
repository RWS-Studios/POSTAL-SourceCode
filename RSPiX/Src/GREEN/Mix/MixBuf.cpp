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
//	MixBuf.cpp
// 
// History:
//		06/19/95 JMI	Started.
//
//		12/19/95	JMI	Altered to accept user supplied data space as well as
//							the space allocated by this module.  Changed use of void*
//							in calls to UCHAR*.
//
//		08/02/96 MJR	Updated non-WIN32 code (was using m_pData, should have
//							been m_pucData).
//
//		08/26/96	JMI	Changed names of ms_usBitsPerSample, ms_ulSampleRate,
//							and ms_usNumChannels to ms_lBitsPerSample,
//							ms_lSampleRate, ms_lNumChannels.
//
//		08/30/96	JMI	Silence() now uses memset() which seems to have improved
//							performance 4x for 8 bit and 2x for 16 bit b/c I was
//							doing a stupid 8 bit or 16 bit copy depending on sample
//							size.
//
//		10/30/96	JMI	Changed:
//							Old label:		New label:
//							=========		=========
//							CMixBuf			RMixBuf
//
//		02/04/97 MJR	Removed a lingering <smrtheap.hpp>.  Also found out
//							that WIN32 was mistaken for _M_IX86.
//
//		07/05/97 MJR	Optimized generic C versions of mixing loops.
//							Also optimized Intel x86 versions of mixing loops.
//
//		07/15/97	JRD	Added optional dynamic volume adjustment to mix, both
//							globally and on a per sound chunk basis
//
//		07/17/97 JRD	Enhanced multiple volume scaling accuracy
//
//		07/17/97 MJR	Added FORCE_GENERIC_CODE macro to force use of generic
//							C/C++ code on all systems.
//
//		08/10/97	JRD	Added a user setable parameter to cut off sound at a set
//							global volume.  Note that this does NOT have to do with
//							the volume of the SAMPLE WAVE DATA, but rather the mix
//							volume passed to MixBuf
//
//		08/14/97	JRD	Made dynamic (8-bit) scaling available to user directly for use
//							in ither applications.
//
//		08/20/97	JMI	Fixed indexing problem with ASM 8bit scaling loop.  Now
//							sounds very crunchy when scaling though.  Also, moved
//							local var declarations that don't get used in ASM version
//							into #if/#endif block for non-ASM code.
//
//		10/30/97	JMI	Broke mixers out into different functions to make it
//							easier to find a particular mixer and easier to add more
//							mixers.  Also, added 8 to 16 and 16 to 8 bit conversion
//							functions so we can mix at a different bit depth than
//							we playback.
//
//////////////////////////////////////////////////////////////////////////////
//
// This module does the actual mixing for CMix.  Each buffer mixes to its own
// settings.
//
// Normal limitations: This module can mix data of the same sample rate with 
// different buffer sizes per channel, with different sample sizes (8 or
// 16 bit PCM), and different numbers of channels (mono or stereo).
//
// For now limitations: In order to get this module running smoothly, it only
// currently accepts data of the same sample rates, buffer sizes, sample
// sizes, and number of channels.
//
//////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <string.h>

#ifdef PATHS_IN_INCLUDES
	#include "BLUE/system.h"
	#include "BLUE/Blue.h"
	#include "GREEN/Mix/MixBuf.h"
#else
	#include "System.h"
	#include "Blue.h"
	#include "MixBuf.h"
#endif // PATHS_IN_INCLUDES


//////////////////////////////////////////////////////////////////////////////
// Module specific macros.
//////////////////////////////////////////////////////////////////////////////

// Define this to force generic C/C++ code to be used for all processors
// instead of any processor-specific code that may exit.  Otherwise,
// comment this out to use any processor-specific code that may exist.
#define FORCE_GENERIC_CODE

// Silence for 8 bit.
#define SILENCE_8		0x80
// Silence for 16 bit.
#define SILENCE_16	0x0000

//////////////////////////////////////////////////////////////////////////////
// Module specific typedefs.
//////////////////////////////////////////////////////////////////////////////

// Handy typedef for referring to a pointer as signed and unsigned char.
typedef union	
	{ 
	S8* ps8; 
	U8* pu8; 
	} P8;

//////////////////////////////////////////////////////////////////////////////
// Instantiate static members.
//////////////////////////////////////////////////////////////////////////////
long	RMixBuf::ms_lSampleRate;			// Sample rate for audio 
													// playback/mix.
long	RMixBuf::ms_lSrcBitsPerSample;	// Sample size in bits for sample data.
													// 0 for no preference.
long	RMixBuf::ms_lMixBitsPerSample;	// Sample size in bits for mixing.
long	RMixBuf::ms_lDstBitsPerSample;	// Sample size in bits for Blue data.
long	RMixBuf::ms_lNumChannels;			// Number of channels (mono
													//  or stereo).
short	RMixBuf::ms_sNumBufs	= 0;			// Number of RMixBufs allocated.
UCHAR	RMixBuf::ms_ucGlobalVolume = UCHAR(255);	// Full volume is standard

short	RMixBuf::ms_sCutOffVolume = 1;	// Volume to not bother mixing.

//////////////////////////////////////////////////////////////////////////////
// Handy conversion functions used by RMixBuf.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// 
// Convert from 8 bps to 16 bps.
// 
//////////////////////////////////////////////////////////////////////////////
inline void Conv8to16(	// Returns nothing.
	U8*	pu8Src,			// In:  8 bit src.
	S16*	ps16Dst,			// Out: 16 bit dst.  Cannot be the same as the src.
	long	lSamples)		// In:  Samples to convert.
	{
	while (lSamples--)
		{
		// Shift up to 16 bits and toggle sign bit based on 16 bit value (i.e., 
		// if it was 128 or more, we want no sign and, if it was under 128, we 
		// want a sign -- I'm not sure if C does this for us or tries something 
		// similar and screws me...need to check the assembly).
		*ps16Dst++	= (*pu8Src++ << 8) ^ 0x8000;
		}
	}

//////////////////////////////////////////////////////////////////////////////
// 
// Convert from 16 bps to 8 bps.
// 
//////////////////////////////////////////////////////////////////////////////
inline void Conv16to8(	// Returns nothing.
	S16*	ps16Src,			// In:  16 bit src.
	U8*	pu8Dst,			// Out: 8 bit dst.  _Can_ be the same as the src.
	long	lSamples)		// In:  Samples to convert.
	{
	while (lSamples--)
		{
		// Toggle sign bit based on 16 bit value, add 256 / 2 for rounding, and
		// shift down to 8 bits.
		*pu8Dst++	= ( (*ps16Src++ ^ 0x8000) + 128) >> 8;
		}
	}

//////////////////////////////////////////////////////////////////////////////
// Handy mixer functions used by RMixBuf.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// 
// Mix (8 bit src, 8 bit destination, no volume scaling, generic).
// 
//////////////////////////////////////////////////////////////////////////////
inline void Mix(		// Returns nothing.
	U8*	pu8Src,		// In:  Src.
	U8*	pu8Dst,		// In:  Dst.
	long	lSamples)	// In:  Number of samples to mix.
	{
	short	sVal;

	while (lSamples--)
		{
		// Convert unsigned values into signed shorts, add them, clip sum,
		// convert back to unsigned value, and save result.
		
		// Add values after converting to signed.
		sVal	= ((short)(*pu8Src++) - 128) + ((short)(*pu8Dst) - 128);

		// Clip.
		if (sVal > 127)
			sVal = 127;
		else if (sVal < -128)
			sVal = -128;
		
		// Stored as unsigned.
		*pu8Dst++ = (UCHAR)(sVal + 128);
		}
	}

#if defined(SYS_BIN_X86)

//////////////////////////////////////////////////////////////////////////////
// 
// Mix (8 bit src, 8 bit destination, no volume scaling, Intel x86).
// 
//////////////////////////////////////////////////////////////////////////////
inline void MixX86(	// Returns nothing.
	U8*	pu8Src,		// In:  Src.
	U8*	pu8Dst,		// In:  Dst.
	long	lSamples)	// In:  Number of samples to mix.
	{
	_asm
		{
		// Safety with C:
		push esi;
		push edi;

			mov	ecx, lSamples	; // Number of iterations.
			mov	esi, pu8Src		; // Src pointer.
			mov	edi, pu8Dst		; // Dst pointer.

		MixLoop8:
			mov	al, [esi]		; // Get src val.
			sub	al, 128			; // Make signed.
			mov	ah, [edi]		; // Get dst val.
			sub	ah, 128			; // Make signed.
			add	ah, al			; // Mix.
			jno	MixDone8			; // If no overflow then we're done
			js		MixWasPos8		; // If negative then it overflowed positive
			mov	ah, 080H			; // Set to max negative.
			jmp	MixDone8
		MixWasPos8:
			mov	ah, 07fH			; // Set to max positive.
		MixDone8:
			add	ah, 128			; // Unsign.
			mov	[edi], ah		; // Store mixed value.
			inc	esi				; // Next source value.
			inc	edi				; // Next dest value.
			loop	MixLoop8

		pop edi;
		pop esi;
		}
	}

#endif	// defined(SYS_BIN_X86)

//////////////////////////////////////////////////////////////////////////////
// 
// Mix (16 bit src, 16 bit destination, no volume scaling, generic).
// 
//////////////////////////////////////////////////////////////////////////////
inline void Mix(		// Returns nothing.
	S16*	ps16Src,		// In:  Src.
	S16*	ps16Dst,		// In:  Dst.
	long	lSamples)	// In:  Number of samples to mix.
	{
	long	lVal;

	while (lSamples--)
		{
		// Add two signed values, clip sum to fit a 16 bit value and save 
		// result.

		lVal	= (long)(*ps16Src++) + (long)(*ps16Dst);

		if (lVal > 32767)
			lVal = 32767;
		else if (lVal < -32768)
			lVal = -32768;
		
		*ps16Dst++ = (S16)(lVal);
		}
	}

#if defined(SYS_BIN_X86)

//////////////////////////////////////////////////////////////////////////////
// 
// Mix (16 bit src, 16 bit destination, no volume scaling, Intel x86).
// 
//////////////////////////////////////////////////////////////////////////////
inline void MixX86(	// Returns nothing.
	S16*	ps16Src,		// In:  Src.
	S16*	ps16Dst,		// In:  Dst.
	long	lSamples)	// In:  Number of samples to mix.
	{
	_asm
		{
		// Safety with C:
		push esi;
		push edi;

			mov	ecx, lSamples	; // Number of iterations.
			mov	esi, ps16Src	; // Src pointer.
			mov	edi, ps16Dst	; // Dst pointer.

		MixLoop16:
			mov	ax, [esi]		; // Get src val.
			add	ax, [edi]		; // Add dst val.
			jno	MixDone16		; // If no overflow then we're done
			js		MixWasPos16		; // If negative then it overflowed positive
			mov	ax, 08000H		; // Set to max negative.
			jmp	MixDone16
		MixWasPos16:
			mov	ax, 07fffH		; // Set to max positive.
		MixDone16:
			mov	[edi], ax		; // Store mixed value.
			add	esi, 2			; // Next source value.
			add	edi, 2			; // Next dest value.
			loop	MixLoop16

		pop edi;
		pop esi;
		}
	}

#endif	// defined(SYS_BIN_X86)

//////////////////////////////////////////////////////////////////////////////
// 
// Mix (8 bit src, 8 bit destination, volume scaling, generic).
// 
//////////////////////////////////////////////////////////////////////////////
inline void Mix(		// Returns nothing.
	U8*	pu8Src,		// In:  Src.
	U8*	pu8Dst,		// In:  Dst.
	long	lSamples,	// In:  Number of samples to mix.
	S16*	psLowTable)	// In:  Volume scale table.
	{
	short	sVal;

	while (lSamples--)
		{
		// Convert unsigned values into signed shorts, add them, clip sum,
		// convert back to unsigned value, and save result

		// Get signed source value, scaled by volume:
		// ENDIAN ALERT! Assuming first byte is low!
		sVal = psLowTable[*pu8Src++];	// scaled

		sVal	= (sVal - 128) + (*pu8Dst - 128);
		if (sVal > 127)
			sVal = 127;
		else if (sVal < -128)
			sVal = -128;

		*pu8Dst++ = (UCHAR)(sVal + 128);
		}
	}

#if defined(SYS_BIN_X86)

//////////////////////////////////////////////////////////////////////////////
// 
// Mix (8 bit src, 8 bit destination, volume scaling, Intel x86).
// 
//////////////////////////////////////////////////////////////////////////////
inline void MixX86(	// Returns nothing.
	U8*	pu8Src,		// In:  Src.
	U8*	pu8Dst,		// In:  Dst.
	long	lSamples,	// In:  Number of samples to mix.
	S16*	psLowTable)	// In:  Volume scale table.
	{
	_asm
		{
		// Safety with C:
		push esi;
		push edi;

			mov	ecx, lSamples	; // Number of iterations.
			mov	esi, pu8Src		; // Src pointer.
			mov	edi, pu8Dst		; // Dst pointer.
			mov	ebx, 0			; // Pre Clear
			mov	edx, psLowTable; // Can't Dereference

		MixVolLoop8:
			mov	bl, [esi]		; // Get src val.
			mov	ax, [edx + ebx * 2]	; // Cut volume
			sub	al, 128			; // Make signed.
			mov	ah, [edi]		; // Get dst val.
			sub	ah, 128			; // Make signed.
			add	ah, al			; // Mix.
			jno	MixVolDone8		; // If no overflow then we're done
			js		MixVolWasPos8	; // If negative then it overflowed positive
			mov	ah, 080H			; // Set to max negative.
			jmp	MixVolDone8
		MixVolWasPos8:
			mov	ah, 07fH			; // Set to max positive.
		MixVolDone8:
			add	ah, 128			; // Unsign.
			mov	[edi], ah		; // Store mixed value.
			inc	esi				; // Next source value.
			inc	edi				; // Next dest value.
			loop	MixVolLoop8

		pop edi;
		pop esi;
		}
	}

#endif	// defined(SYS_BIN_X86)

//////////////////////////////////////////////////////////////////////////////
// 
// Mix (16 bit src, 16 bit destination, volume scaling, generic).
// 
//////////////////////////////////////////////////////////////////////////////
inline void Mix(			// Returns nothing.
	S16*	ps16Src,			// In:  Src.
	S16*	ps16Dst,			// In:  Dst.
	long	lSamples,		// In:  Number of samples to mix.
	S16*	psLowTable,		// In:  Volume scale table for low byte.
	S16*	psHighTable)	// In:  Volume scale table for high byte.
	{
	long	lVal;
	short	sVal;
	short	sHighSrc;
	short	sLowSrc;

	// Move into a byte by byte, sign adjustable reference wrapper.
	P8	p8Src	= { (S8*)ps16Src };

	while (lSamples--)
		{
		// Get signed source value, scaled by volume:
		
		// NOTE this confused the Hell out of me at first b/c it 
		// looks as if these two sections are executing the same
		// code to access two different members of a struct but
		// in a different order.  However, the order is actually
		// important as p8Src is a union (not a struct) and pu8
		// & ps8 refer to the very same pointer (just one as 
		// unsigned and the other as signed).  The order in this
		// case determines which byte is read first -- the signed
		// or the unsigned.
		#if defined(SYS_ENDIAN_LITTLE)
			sLowSrc = short (*p8Src.pu8++);
			sHighSrc = short (*p8Src.ps8++);
		#else
			sHighSrc = short (*p8Src.ps8++);
			sLowSrc = short (*p8Src.pu8++);
		#endif

		sVal = psLowTable[sLowSrc] + psHighTable[sHighSrc];	// scaled

		// Add two signed values, clip sum to fit a 16 bit value, and save 
		// result.
		lVal	= (long)(sVal) + (long)(*ps16Dst);

		if (lVal > 32767)
			lVal = 32767;
		else if (lVal < -32768)
			lVal = -32768;

		*ps16Dst++ = (short)(lVal);
		}
	}

#if defined(SYS_BIN_X86)

//////////////////////////////////////////////////////////////////////////////
// 
// Mix (16 bit src, 16 bit destination, volume scaling, Intel x86).
// 
//////////////////////////////////////////////////////////////////////////////
inline void MixX86(		// Returns nothing.
	S16*	ps16Src,			// In:  Src.
	S16*	ps16Dst,			// In:  Dst.
	long	lSamples,		// In:  Number of samples to mix.
	S16*	psTable)			// In:  Volume scale tables for high AND low byte.
								// One table is used to save registers but we could
								// have the second table as well instead of offseting
								// this table by DVA_LOW_OFFSET.
	{
	_asm
		{
		// Safety with C:
		push esi;
		push edi;

			mov	ecx, lSamples	; // Number of iterations.
			mov	esi, ps16Src	; // Src pointer.
			mov	edi, ps16Dst	; // Dst pointer.
			mov	ebx, 0			; // Pre Clear
			mov	edx, psTable	; // Can't Dereference Directly

		MixVolLoop16:
			// Do a source byte at a time:
			mov	bl, [esi]		; // Get src val low byte.
			mov	ax, [edx + DVA_LOW_OFFSET + ebx*2]	; // Cut volume
			inc	esi				; // 
			mov	bl, [esi]		; // Get src val high byte
			add	bl,128			; // Make unsigned
			add	ax, [edx + ebx*2]	; //	Cut Volume
			inc	esi				; // Next source value.
			add	ax, [edi]		; // Add dst val.
			jno	MixVolDone16	; // If no overflow then we're done
			js		MixVolWasPos16	; // If negative then it overflowed positive
			mov	ax, 08000H		; // Set to max negative.
			jmp	MixVolDone16
		MixVolWasPos16:
			mov	ax, 07fffH		; // Set to max positive.
		MixVolDone16:
			mov	[edi], ax		; // Store mixed value.
			add	edi, 2			; // Next dest value.
			loop	MixVolLoop16

		pop edi;
		pop esi;
		}
	}

#endif	// defined(SYS_BIN_X86)


//////////////////////////////////////////////////////////////////////////////
// RMixBuf ///////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// <Con|De>struction.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Default constructor.
//
//////////////////////////////////////////////////////////////////////////////
RMixBuf::RMixBuf(void)
	{
	ms_sNumBufs++;

	// Initialize members.
	Init();
	}

//////////////////////////////////////////////////////////////////////////////
//
// Constructor Especial.
//
//////////////////////////////////////////////////////////////////////////////
RMixBuf::RMixBuf(
	UCHAR* pu8Dst,	// In:  Destination buffer.
	ULONG ulSize)	// In:  Size of destination buffer in bytes.
	{
	ms_sNumBufs++;

	// Initialize members.
	Init();

	// Set data ptr and size.
	m_pu8Dst		= pu8Dst;
	m_ulDstSize	= ulSize;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Destructor.
//
//////////////////////////////////////////////////////////////////////////////
RMixBuf::~RMixBuf(void)
	{
	Reset();

	ms_sNumBufs--;
	}

//////////////////////////////////////////////////////////////////////////////
// Internal Use.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Intialize members.
//
//////////////////////////////////////////////////////////////////////////////
void RMixBuf::Init(void)
	{
	m_pu8Dst			= NULL;
	m_ulDstSize		= 0L;
	m_pu8Mix			= NULL;
	m_ulMixSize		= 0L;
	m_sInUse			= FALSE;
	m_sOwnMixBuf	= FALSE;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Free dynamic data and re-init.
//
//////////////////////////////////////////////////////////////////////////////
void RMixBuf::Reset(void)
	{
	ASSERT(m_sInUse == FALSE);

	if (m_sOwnMixBuf == TRUE)
		{
		if (m_pu8Mix != NULL)
			{
			free(m_pu8Mix);
			}
		}

	Init();
	}

//////////////////////////////////////////////////////////////////////////////
// Methods.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Set all data to silence.
//
//////////////////////////////////////////////////////////////////////////////
void RMixBuf::Silence(void)
	{
	ASSERT(m_sInUse == FALSE);

	if (m_pu8Mix != NULL)
		{
		switch (ms_lMixBitsPerSample)
			{
			case 8:
				memset(m_pu8Mix, SILENCE_8, m_ulMixSize);
				break;

			case 16:
				memset(m_pu8Mix, SILENCE_16, m_ulMixSize);
				break;

			default:
				TRACE("Silence(): Unsupported mixing bits per sample: %ld.\n",
						ms_lMixBitsPerSample);
				break;
			}
		}
	else
		{
		TRACE("Silence(): No mix buffer.\n");
		}
	}

//////////////////////////////////////////////////////////////////////////////
//
// Set size of mix buffer in bytes. (Allocates buffer).
// Returns 0 on success.
//
//////////////////////////////////////////////////////////////////////////////
short RMixBuf::SetSize(ULONG ulSize)
	{
	short sRes = 0;	// Assume success.

	ASSERT(m_sInUse == FALSE);

	// If there's already a buffer . . .
	if (m_pu8Mix)
		{
		// If we own the buffer . . .
		if (m_sOwnMixBuf)
			{
			// Free it.
			free(m_pu8Mix);
			
			// We no longer own it.
			m_sOwnMixBuf	= FALSE;
			}

		m_ulMixSize	= 0L;
		}

	// Allocate new data chunk.
	m_pu8Mix = (UCHAR*)malloc(ulSize);
	// If successful . . .
	if (m_pu8Mix != NULL)
		{
		// Success.
		m_sOwnMixBuf	= TRUE;
		m_ulMixSize		= ulSize;
		// Initialize to silence.
		Silence();
		}
	else
		{
		TRACE("SetSize(%lu): Unable to allocate buffer.\n", ulSize);
		sRes = -1;
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Set the destination buffer.
//
//////////////////////////////////////////////////////////////////////////////
void RMixBuf::SetDest(	// Returns nothing.
	UCHAR* pu8Dst,			// In:  Destination buffer.
	ULONG ulSize)			// In:  Size of destination buffer in bytes.
	{
	// If mix is using dest . . .
	if (m_pu8Mix == m_pu8Dst)
		{
		// Clear it.
		m_pu8Mix		= NULL;
		m_ulMixSize	= 0;
		}

	m_pu8Dst		= pu8Dst;
	m_ulDstSize	= ulSize;

	// If incorrect size for mixer buffer . . .
	if (m_ulMixSize != m_ulDstSize && m_ulDstSize)
		{
		// If we can use the destination buffer (i.e., if the mix sample depth and 
		// playback sample depth are the same) . . .
		if (ms_lMixBitsPerSample == ms_lDstBitsPerSample)
			{
			// Just use the destination buffer.
			m_pu8Mix		= m_pu8Dst;
			m_ulMixSize	= m_ulDstSize;

			// Since this is our first use, Silence() it.
			Silence();
			}
		else
			{
			// Set mix size to the size of the playback buffer but at the mix quality.
			long	lNumSamples	= m_ulDstSize * 8 / ms_lDstBitsPerSample;
			long	lSize	= lNumSamples * ms_lMixBitsPerSample / 8;
			if (SetSize(lSize) == 0)
				{
				// Note that SetSize() always Silence()s buffer.
				}
			else
				{
				TRACE("SetDest(): Unable to set size to %ld.\n", lSize);
				}
			}
		}
	}

//////////////////////////////////////////////////////////////////////////////
//
// Build table for dynamic volume adjustment (DVA)
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// CVA static members
//////////////////////////////////////////////////////////////////////////////
// This stores both the high and low arrasy in one, to save registers
// (hence the '*2')
short	CDVA::ms_asHighByte[DVA_SIZE * 2][256];	// for 16-bit sound

CDVA	initVolumeControl;	// create the tables

CDVA::CDVA()	// Create the tables!
		{
		// Create the 16-bit scaling codes..
		// The 8-bit codes are found by looking at the lower byte of the 
		// table, as in ms_asLowByte.u8[i*2 + DVA_LOW_BYTE]

		// Note that volume scaling goes from 0 to 255 in steps of DVA_RESOLUTION
		// (this is to save memory)

		//================================================================
		// Input: 0-255 representing -32768 to +32767. ( (Input-128) * 256)
		//	DIM Value:	0-255, 255 being identity
		// Output: 16 bit table yielding Input*256*DIM/255
		//================================================================
		long	lSrcVal,lDimVal,lCurValue,lNumerator;
		short*	psCur = &ms_asHighByte[0][0];	// assume linear addressing:
		double	dSrcVal;
		// This is DimVal major!

		// I'm going to drop integral calculus for now and use multiplication.
		// This can be sped up later with I.C.
		for (lDimVal = 0; lDimVal < 256; lDimVal += DVA_RESOLUTION)
			{
			double dScale = double(lDimVal)/255.0;
			// offset for signed!
			for (dSrcVal = -32768.00; dSrcVal < 32767.0; dSrcVal += 256.0,psCur++)
				{
				*psCur = short(dSrcVal * dScale);	// use I.C. to speed up!
				}
			}

		// Now, do the low byte... same as RMultiAlpha::ms_aucLiveDimming,
		// except, for speed's sake, we store the value as 16-bit:
		// The low byte array should simply follow the high byte array,
		// so I don't need to reset psCur.

		for (lDimVal = 0; lDimVal < 256; lDimVal += DVA_RESOLUTION)
			{
			lNumerator = 127;	// for rounding
			lCurValue = 0;

			*psCur++ = 0;	// initial value

			for (lSrcVal = 1; lSrcVal < 256; lSrcVal++,psCur++)
				{
				lNumerator += lDimVal;

				if (lNumerator >= 255) 
					{
					lNumerator -= 255; 
					lCurValue++;
					}

				*psCur = short(lCurValue);
				}
			}
		TRACE("Volume Control Initialized.\n");
		}
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Mix data in, with optional global x local1 x local2 volume adjustment
// Returns 0 on success.
//
//////////////////////////////////////////////////////////////////////////////
short RMixBuf::Mix(	ULONG		ulStartPos,
							U8*		pu8Data, 
							ULONG		ulSize, 
							long		lSampleRate,
							long		lBitsPerSample,
							long		lNumChannels,
							UCHAR		ucVolume,
							UCHAR		ucVol2)
	{
	short sRes	= 0;	// Assume success.

	ASSERT(m_sInUse == FALSE);

	// If there's a mix buffer . . .
	if (m_ulMixSize != 0)
		{
		// Must be the same.
		ASSERT(lSampleRate		== ms_lSampleRate);
		ASSERT(lNumChannels		== ms_lNumChannels);

		// If ms_lSrcBitsPerSample is non-zero, lBitsPerSample must match.
		ASSERT(lBitsPerSample	== ms_lSrcBitsPerSample || ms_lSrcBitsPerSample == 0);

		// *** TEMP ***
		// Temporarily enforce use of same bit depth on sources since we have 
		// functions yet that convert and mix.
		// Delete this line once we have support.
		ASSERT(lBitsPerSample	== ms_lSrcBitsPerSample);
		// *** END TEMP ***

		// If you do not have full accuracy in volume settings, round results
		// up to next actual volume.
		
		// Calculate current effective volume scaling on type * global:
		// This is actually the low byte. (hence DVA_SIZE + )
		short sCurVolume = CDVA::ms_asHighByte
			[DVA_SIZE + (ms_ucGlobalVolume>>DVA_SHIFT)][ucVol2];

		// Round up:
		sCurVolume = (sCurVolume + (DVA_RESOLUTION-1)) >> DVA_SHIFT;
		if (sCurVolume >= DVA_SIZE) sCurVolume = (DVA_SIZE-1); // overflow

		// Factor in current sample volume:
		sCurVolume = CDVA::ms_asHighByte
			[DVA_SIZE + sCurVolume][ucVolume];

		if (sCurVolume < DVA_RESOLUTION ) return sRes;	// sound is off
		if (sCurVolume < ms_sCutOffVolume) return sRes; // Sound is clipped by user

		ASSERT(ulSize <= (m_ulMixSize - ulStartPos) );
		
		ULONG ulNum	= MIN(ulSize, m_ulMixSize - ulStartPos);

		if (ulNum > 0)
			{
			if (sCurVolume > (255 - DVA_RESOLUTION))	// full volume, no scaling
				{
				switch (lBitsPerSample)
					{
					case 8:
						#if defined(FORCE_GENERIC_CODE) || !defined(SYS_BIN_X86)

							::Mix( (U8*)pu8Data, (U8*)(m_pu8Mix + ulStartPos), ulNum);

						#else

							MixX86( (U8*)pu8Data, (U8*)(m_pu8Mix + ulStartPos), ulNum);

						#endif

						break;

					case 16:
						#if defined(FORCE_GENERIC_CODE) || !defined(SYS_BIN_X86)

							::Mix( (S16*)pu8Data, (S16*)(m_pu8Mix + ulStartPos), ulNum / 2);

						#else

							MixX86( (S16*)pu8Data, (S16*)(m_pu8Mix + ulStartPos), ulNum / 2);

						#endif

						break;

					default:
						TRACE("Mix(): Unsupported bits per sample: %ld.\n",
								lBitsPerSample);
						sRes = -1;
						break;
					}
				}
			////////////////////////////////////////////////////////////////////////////////
			else	// do volume scaling
			////////////////////////////////////////////////////////////////////////////////
				{
				// First, figure out which table apply to the current volume level:
				sCurVolume >>= DVA_SHIFT; // scale to an offset

				// Offset high by 128 to represent signed upper bytes
				short*	psHighTable = 128 + CDVA::ms_asHighByte[sCurVolume];
				// The assembler doesn't know the offset is signed
				short*	psASMHighTable = CDVA::ms_asHighByte[sCurVolume];
				// Low byte is by nature unsigned, so no offset
				// This is packed into the same table, offset by DVA_SIZE entries
				short*	psLowTable = CDVA::ms_asHighByte[DVA_SIZE + sCurVolume];

				switch (lBitsPerSample)
					{
					case 8:
						#if defined(FORCE_GENERIC_CODE) || !defined(SYS_BIN_X86)
							
							::Mix( (U8*)pu8Data, (U8*)(m_pu8Mix + ulStartPos), ulNum, psLowTable);

						#else

							MixX86( (U8*)pu8Data, (U8*)(m_pu8Mix + ulStartPos), ulNum, psLowTable);

						#endif

						break;

					case 16:
						#if defined(FORCE_GENERIC_CODE) || !defined(SYS_BIN_X86)

							::Mix( (S16*)pu8Data, (S16*)(m_pu8Mix + ulStartPos), ulNum / 2, psLowTable, psHighTable);

						#else

							MixX86( (S16*)pu8Data, (S16*)(m_pu8Mix + ulStartPos), ulNum / 2, psASMHighTable);

						#endif

						break;

					default:
						TRACE("Mix(): Unsupported bits per sample: %u.\n",
								lBitsPerSample);
						sRes = -1;
						break;
					}
				}
			}
		}
	else
		{
		TRACE("Mix():  No mix buffer.\n");
		sRes	= -2;
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Prepare for destination.  If necessary, converts to destination format.
//
//////////////////////////////////////////////////////////////////////////////
void RMixBuf::PrepareForDest(void)
	{
	// If we are not mixing at the sample depth we intend to play at . . .
	if (ms_lDstBitsPerSample != ms_lMixBitsPerSample)
		{
		switch (ms_lDstBitsPerSample)
			{
			case 8:
				
				// Convert to 8 bit for playback.
				Conv16to8( (S16*)m_pu8Mix, m_pu8Dst, m_ulDstSize);

				break;

			case 16:

				// Convert to 16 bit for playback.
				Conv8to16(m_pu8Mix, (S16*)m_pu8Dst, m_ulMixSize);

				break;
			}
		}
	else
		{
		// Already the same.  Should be using the same buffer.
		ASSERT(m_pu8Mix == m_pu8Dst);
		}
	}

//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
