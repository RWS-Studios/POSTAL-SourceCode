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
//		09/13/95	JMI	Added Convert8to16 to convert 8 bit sample to 16 bit 
//							sample size.
//
//		08/04/96 MJR	Minor fix of typo in ASSERT().
//							Revised some IFF stuff to work on Mac.  Will be
//							further revised when Jon finishes new IFF stuff.
//
//		10/30/96	JMI	Changed:
//							Old label:		New label:
//							=========		=========
//							CSample			RSample
//							CIff				RIff
//		10/31/96	JMI	CNFile			RFile
//							ENDIAN_BIG		BigEndian
//							ENDIAN_LITTLE	LittleEndian
//
//		01/28/97	JMI	Added Load(RFile*), Save(char*), and Save(RFile*).
//
//		02/04/97	JMI	Load(RFile*) forgot to make sure the ref count was 0
//							before loading and also forgot to Lock() the sample
//							before loading.
//
//		02/05/97	JMI	Had to cast Reads and Writes that used void* overload
//							in RFile to use U8* since void* is now protected
//							(see file.h for more details).
//
//		03/25/97	JMI	Changed a Seek(0, SEEK_SET) to a relative seek.
//
//		07/05/97 MJR	Changed so Read() now reads 16-bit values as words so
//							they will be properly byte-swapped as necessary.
//
//////////////////////////////////////////////////////////////////////////////
//
// This is a generic wrapper for an audio sample.  Currently only WAV files 
// are supported as far as loading goes, but I tried to make it easy for it 
// to be modified to autodetect and load other file formats.  No warranteis
// express or implied.
//
//////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "System.h"

#ifdef PATHS_IN_INCLUDES
	#include "BLUE/Blue.h"
	#include "GREEN/Sample/sample.h"
#else
	#include "Blue.h"
	#include "sample.h"
#endif // PATHS_IN_INCLUDES

//////////////////////////////////////////////////////////////////////////////
// Initialize static member variables.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Macros.
//////////////////////////////////////////////////////////////////////////////
#define SAMPLE_TYPE_UNKNOWN	0x0000
#define SAMPLE_TYPE_WAVE		0x0001

#ifndef WAVE_FORMAT_PCM
#define WAVE_FORMAT_PCM		0x0001
#define WAVE_FORMAT_ADPCM	0x0002
#endif // WAVE_FORMAT_PCM

#define DEFAULT_READBUFSIZE	16384

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
RSample::RSample(void)
	{
	// Intialize instantiables.
	Init();
	}

//////////////////////////////////////////////////////////////////////////////
//
// Constructor Especial.
// Returns no nutin' under no circumstance no how.
// (public)
//
//////////////////////////////////////////////////////////////////////////////
RSample::RSample(	void *pData, long lBufSize, long lSamplesPerSec, 
						short sBitsPerSample, short sNumChannels)
	{
	// Intialize instantiables.
	Init();
	// Fill in supplied members.
	m_pData				= pData;
	m_lBufSize			= lBufSize;
	m_lSamplesPerSec	= lSamplesPerSec;
	m_sBitsPerSample	= sBitsPerSample;
	m_sNumChannels		= sNumChannels;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Destructor.
// Returns nothing.
// (public)
//
//////////////////////////////////////////////////////////////////////////////
RSample::~RSample(void)
	{
	// Make sure memory is freed and we're unlocked.
	Reset();
	}

//////////////////////////////////////////////////////////////////////////////
//
// Initialize instantiable members.
// Returns nothing.
// (public)
//
//////////////////////////////////////////////////////////////////////////////
void RSample::Init(void)
	{
	// Initialize members.
	m_pData				= NULL;
	m_sOwnBuf			= FALSE;
	m_lBufSize			= 0L;
	m_lSamplesPerSec	= 0L;
	m_sBitsPerSample	= 0;
	m_sNumChannels		= 0;
	m_sRefCnt			= 0;
	}

///////////////////////////////////////////////////////////////////////////////
//
// Reset object.  Release play data and reset variables.
// Returns nothing.
// (public)
//
///////////////////////////////////////////////////////////////////////////////
void RSample::Reset(void)
	{
	ASSERT(IsLocked() == FALSE);

	if (IsLocked() == FALSE)
		{
		if (m_sOwnBuf == TRUE)
			{
			if (m_pData != NULL)
				{
				// Free data.
				free(m_pData);
				m_pData = NULL;
				}
			}
		
		Init();
		}
	else
		{
		TRACE("Reset():  Attempt to Reset a locked RSample!\n");
		}
	}

///////////////////////////////////////////////////////////////////////////////
//
// Read from file until we hit pcForm chunk, and then read size of chunk.
// Returns size of chunk on success.
//
///////////////////////////////////////////////////////////////////////////////
static long IffReadUntil(char* pcForm, FILE* fsIn)
	{
	long lRes = 0;

	// Search for pcForm.
	short sMatch = 0;
	int	c;
	
	while (lRes == 0)
		{
		c = fgetc(fsIn);
		if (c == EOF)
			{
			lRes = -1;
			}
		else
			{
			if (pcForm[sMatch] == c)
				{
				if (++sMatch == 4)
					{
					// Attempt to read size of chunk.
					if (fread(&lRes, 4, 1, fsIn) != 1)
						{
						TRACE("IffReadUntil(): Error reading size of chunk.\n");
						lRes = -2;
						}
					else
						{
						break;
						}
					}
				}
			else
				{
				sMatch = 0;
				}
			}
		}

	return lRes;
	}

///////////////////////////////////////////////////////////////////////////////
//
// Read WAVE info from fsIn.
// Returns size of data chunk on success, negative on error. (protected)
//
///////////////////////////////////////////////////////////////////////////////
long RSample::ReadWaveHeader(void)
	{
	long	lRes = 0L; // Assume success.

	// Skip RIFF garbage
	if (m_iff.Find(".WAVE.fmt ") == 0)
		{
		USHORT usFormatTag;
		// Read PCMWAVEFORMAT header.
		if (m_iff.Read(&usFormatTag) == 1L)
			{
			ASSERT(usFormatTag == WAVE_FORMAT_PCM);

			if (m_iff.Read(&m_sNumChannels) == 1L)
				{
				if (m_iff.Read(&m_lSamplesPerSec) == 1L)
					{
					long lAvgBytesPerSec;
					if (m_iff.Read(&lAvgBytesPerSec) == 1L)
						{
						short sBlockAlign;
						if (m_iff.Read(&sBlockAlign) == 1L)
							{
							if (m_iff.Read(&m_sBitsPerSample) == 1L)
								{
								ASSERT(m_sBitsPerSample * m_sNumChannels / 8 == sBlockAlign);

								// If ADPCM . . .
								if (usFormatTag == WAVE_FORMAT_ADPCM)
									{
									// Read rest of 'fmt ' chunk.
									USHORT usExtendedSize;
									if (m_iff.Read(&usExtendedSize) == 1L)
										{
										// Read number of samples per block.
										USHORT usSamplesPerBlock;
										if (m_iff.Read(&usSamplesPerBlock) == 1L)
											{
											// Read number of coefficients.
											USHORT usNumCoefs;
											if (m_iff.Read(&usNumCoefs) == 1L)
												{
												// Read coefficients.
												for (USHORT usIndex = 0; usIndex < usNumCoefs; usIndex++)
													{

													}
												}
											else
												{
												TRACE("ReadWaveHeader(): Unable to read number of coefficients.\n");
												lRes = -11L;
												}
											}
										else
											{
											TRACE("ReadWaveHeader(): Unable to read number of samples per block.\n");
											lRes = -10L;
											}
										}
									else
										{
										TRACE("ReadWaveHeader(): Unable to read cbSize field of extended 'fmt '.\n");
										lRes = -9L;
										}
									}

								// If successful so far . . .
								if (lRes == 0L)
									{
									// Skip to "data" chunk.
									if (m_iff.Find("data") == 0)
										{
										// Success.
										lRes	= m_iff.GetSize();
										}
									else
										{
										TRACE("ReadWaveHeader(): Unable to skip to \"data\" chunk.\n");
										lRes = -8L;
										}
									}
								}
							else
								{
								TRACE("ReadWaveHeader(): Unable to read number of bits per second.\n");
								lRes = -7L;
								}
							}
						else
							{
							TRACE("ReadWaveHeader(): Unable to read block size of data.\n");
							lRes = -6L;
							}
						}
					else
						{
						TRACE("ReadWaveHeader(): Unable to read average bytes per second.\n");
						lRes = -5L;
						}
					}
				else
					{
					TRACE("ReadWaveHeader(): Unable to read sample rate.\n");
					lRes = -4L;
					}
				}
			else
				{
				TRACE("ReadWaveHeader(): Unable to read number of channels (i.e. mono, stereo, etc.).\n");
				lRes = -3L;
				}
			}
		else
			{
			TRACE("ReadWaveHeader(): Unable to read format type.\n");
			lRes = -2L;
			}
		}
	else
		{
		TRACE("ReadWaveHeader(): Unable to skip to identifier \"fmt \".\n");
		lRes = -1L;
		}

	return lRes;
	}

///////////////////////////////////////////////////////////////////////////////
//
//	Determines the file type of the supplied file stream, if possible, and 
// returns a short value indicating that file type.
//
///////////////////////////////////////////////////////////////////////////////
static short GetFileType(RIff* piff)
	{
	short sRes = SAMPLE_TYPE_UNKNOWN; // Assume unknown.

	// Read some info to determine file type.
	static char acHeader[128];
	long	lBeginPos	= piff->Tell();
	long	lNumRead		= piff->Read(acHeader, sizeof(acHeader));
	// Seek back to beginning.
	if (piff->Seek(lBeginPos, SEEK_SET) == 0)
		{
		if (lNumRead > 0L)
			{
			// Check for WAV type.
			if (strncmp(acHeader, "RIFF", 4) == 0)
				{
				if (strncmp(acHeader + 8, "WAVE", 4) == 0)
					{
					sRes = SAMPLE_TYPE_WAVE;
					}
				}

			if (sRes == SAMPLE_TYPE_UNKNOWN)
				{
				// Check for another type.
				}
			}
		else
			{
			TRACE("GetFileType(): Unable to read header info.\n");
			sRes = -2;
			}
		}
	else
		{
		TRACE("GetFileType(): Unable to seek back to beginning.\n");
		sRes = -1;
		}
	
	return sRes;
	}

///////////////////////////////////////////////////////////////////////////////
//
// Open a file and read the header.  Locks the RSample automatically.
// Returns the size of the file's data on success, negative otherwise.
// (public)
//
///////////////////////////////////////////////////////////////////////////////
long RSample::Open(char* pszSampleName, long lReadBufSize)
	{
	long	lRes = 0L;

	// Reset variables and free data if any.
	Reset();
	
	// Make sure no other locks . . . 
	if (m_sRefCnt == 0)
		{
		// Lock.
		Lock();

		// Attempt to open file.
		if (m_iff.Open(pszSampleName, "rb", RFile::LittleEndian) == 0)
			{
			// Ask stdio to allocate a buffer of the size we are reading to optimize 
			// read performance . . .
			if (m_iff.SetBufferSize(lReadBufSize) == 0L)
				{
				// Success.
				}
			else
				{
				TRACE("Open(\"%s\"): Unable setup read buffer.\n", pszSampleName);
				lRes = -5L;
				}

			switch (GetFileType(&m_iff))
				{
				case SAMPLE_TYPE_WAVE:
					lRes = ReadWaveHeader();
					break;
				case SAMPLE_TYPE_UNKNOWN:
					TRACE("Open(\"%s\"): Unknown sound file type.\n", pszSampleName);
					lRes = -1L;
					break;
				default:
					TRACE("Open(\"%s\"): Error reading header.\n", pszSampleName);
					lRes = -2L;
					break;
				}

			// If any errors occurred . . .
			if (lRes < 0L)
				{
				m_iff.Close();
				}
			}
		else
			{
			TRACE("Open(\"%s\"): Unable to open file.\n", pszSampleName);
			lRes = -3;
			}

		// If any errors occurred . . .
		if (lRes < 0L)
			{
			// Unlock and reset . . .
			if (Unlock() == 0)
				{
				Reset();
				}
			}
		}
	else
		{
		TRACE("Open(\"%s\"): Unable to lock RSample.\n", pszSampleName);
		lRes = -4;
		}
		
	return lRes;
	}
	
///////////////////////////////////////////////////////////////////////////////
//
// Read the specified amount of data from the open file.
// Returns amount read on success, negative on failure.
//
///////////////////////////////////////////////////////////////////////////////
long RSample::Read(long lAmount)
	{
	long	lRes	= 0L;

	ASSERT(m_iff.IsOpen() != FALSE);

	// If current data size is different than desired or is not allocated . . .
	if (m_pData == NULL || m_lBufSize != lAmount)
		{
		// Allocate desired amount.
		m_pData = (void*)malloc(lAmount);
		// If successful . . .
		if (m_pData != NULL)
			{
			// Remember we own this buffer (i.e., we allocated it and should
			// be responsible for freeing it).
			m_sOwnBuf	= TRUE;
			// Set data buffer size.
			m_lBufSize	= lAmount;
			}
		else
			{
			TRACE("Read(%lu): Unable to allocate data chunk.\n", lAmount);
			lRes = -1L;
			}
		}

	// If successful so far . . .
	if (lRes == 0L)
		{
		// Attempt to read amount requested.  For 8-bit data, or if the data
		// size hasn't been set yet, we read the data as a block of bytes.
		// Otherwise, we read the data as words so the system's endian nature
		// wil be taken into account.
		if (m_sBitsPerSample < 16)
			lRes = m_iff.Read((U8*)m_pData, lAmount);
		else
			lRes = m_iff.Read((U16*)m_pData, lAmount / 2) * 2;
		// If data read . . .
		if (lRes > 0L)
			{
			// Success.
			}
		else
			{
			// Check for error.
			if (m_iff.Error() == FALSE)
				{
				// EOF.
				}
			else
				{
				TRACE("Read(%lu): Error reading stream.\n");
				lRes = -2L;
				}
			}
		}

	return lRes;
	}
		
///////////////////////////////////////////////////////////////////////////////
//
// Close the file opened with Open.  Unlocks the RSample automatically.
// Returns 0 on success.
// (public)
//
///////////////////////////////////////////////////////////////////////////////
short RSample::Close(void)
	{
	short sRes = 0;

	ASSERT(m_iff.IsOpen() != FALSE);

	if (m_iff.Close() == 0)
		{
		// If unlocking leaves this RSample totally unlocked . . .
		if (Unlock() == 0)
			{
			// Success.
			}
		}
	else
		{
		TRACE("Close(): Unable to close file.\n");
		sRes = -1;
		}

	return sRes;
	}
	
///////////////////////////////////////////////////////////////////////////////
//
// Load an entire sound file.
// Returns 0 on success.
// (public)
//
///////////////////////////////////////////////////////////////////////////////
short RSample::Load(char* pszSampleName)
	{
	short sRes = 0;
	
	// Attempt to open sample file.
	m_lBufSize = Open(pszSampleName, DEFAULT_READBUFSIZE);
	// If successful . . .
	if (m_lBufSize >= 0L)
		{
		// Attempt to read entire sample.
		// If we get all the data . . .
		if (Read(m_lBufSize) == m_lBufSize)
			{
			// Success.
			}
		else
			{
			TRACE("Load(\"%s\"): Unable to read entire file.\n", pszSampleName);
			sRes = -2;
			}

		// Close file and clean up.
		Close();
		}
	else
		{
		sRes = -1;
		}
		
	return sRes;
	}
	
///////////////////////////////////////////////////////////////////////////////
//
// Same as above, but accepts an open RFile* and uses existing read buffer
// size.
//
///////////////////////////////////////////////////////////////////////////////
short RSample::Load(	// Returns 0 on success.
	RFile*	pfile)	// Open RFile.
	{
	short	sRes	= 0;	// Assume success.

	// Reset variables and free data if any.
	Reset();
	
	// Make sure no other locks . . . 
	if (m_sRefCnt == 0)
		{
		// Lock.
		Lock();

		// Attempt to synch with supplied RFile* . . .
		if (m_iff.Open(pfile) == 0)
			{
			// Get input file type.
			switch (GetFileType(&m_iff))
				{
				case SAMPLE_TYPE_WAVE:
					m_lBufSize = ReadWaveHeader();
					break;
				case SAMPLE_TYPE_UNKNOWN:
					TRACE("Load(): Unknown sound file type.\n");
					sRes = -6;
					break;
				default:
					TRACE("Load(): Error reading header.\n");
					sRes = -5;
					break;
				}

			// If successful so far . . .
			if (m_lBufSize >= 0L && sRes == 0)
				{
				// Attempt to read entire sample.
				// If we get all the data . . .
				if (Read(m_lBufSize) == m_lBufSize)
					{
					// Success.
					}
				else
					{
					TRACE("Load(): Unable to read entire file.\n");
					sRes = -4;
					}
				}
			else
				{
				TRACE("Load(): ReadWaveHeader() failed.\n");
				sRes	= -3;
				}

			// Close and clean up.  Will 'unsynch' with pfile via m_iff.Close() call.
			Close();
			}
		else
			{
			TRACE("Load(): m_iff.Open(pfile) failed.\n");
			sRes	= -2;
			}
		}
	else
		{
		TRACE("Load(): Unable to lock RSample.\n");
		sRes = -1;
		}

	return sRes;
	}

///////////////////////////////////////////////////////////////////////////////
//
// Saves entire sample to file specified in RIFF WAVE format.
//
///////////////////////////////////////////////////////////////////////////////
short RSample::Save(		// Returns 0 on success.
	char* pszFileName)	// Filename for sample file.
	{
	short	sRes	= 0;	// Assume success.

	// Open file for write . . .
	RFile file;
	if (file.Open(pszFileName, "wb", RFile::LittleEndian) == 0)
		{
		// Do it.
		sRes	= Save(&file);
		}
	else
		{
		TRACE("Save(\"%s\"): file.Open() failed.\n", pszFileName);
		sRes	= -1;
		}

	return sRes;
	}

///////////////////////////////////////////////////////////////////////////////
//
// Saves entire sample to file specified in RIFF WAVE format.
//
///////////////////////////////////////////////////////////////////////////////
short RSample::Save(		// Returns 0 on success.
	RFile* pfile)			// Open RFile for sample.
	{
	short	sRes	= 0;	// Assume success.

	// Attempt to synchronize . . .
	RIff	iff;
	if (iff.Open(pfile) == 0)
		{
		// Create 'RIFF' 'WAVE' form . . .
		if (iff.CreateChunk(RIff::RiffStr2FCC("RIFF"), RIff::RiffStr2FCC("WAVE")) == 0)
			{
			// Create 'fmt ' chunk . . .
			if (iff.CreateChunk(RIff::RiffStr2FCC("fmt ") ) == 0)
				{
				// Fill in the chunk.
				// Write PCMWAVEFORMAT header.
				iff.Write((U16)WAVE_FORMAT_PCM);
				iff.Write(m_sNumChannels);
				iff.Write(m_lSamplesPerSec);
				// Average bytes per second.
				iff.Write((m_lSamplesPerSec * (long)m_sBitsPerSample * (long)m_sNumChannels) / 8L);
				// Block align.
				iff.Write((short)(m_sBitsPerSample * m_sNumChannels / 8) );
				iff.Write(m_sBitsPerSample);

				// End 'fmt ' chunk.
				iff.EndChunk(RIff::RiffStr2FCC("fmt ") );

				// Create 'data' chunk . . .
				if (iff.CreateChunk(RIff::RiffStr2FCC("data") ) == 0)
					{
					// Write data da whole thing ... kerbang!
					if (iff.Write((U8*)m_pData, m_lBufSize) == m_lBufSize)
						{
						// Success.
						}
					else
						{
						TRACE("Save(): Failed to write entire data buffer to 'data' chunk.\n");
						sRes	= -5;
						}

					// End 'data' chunk.
					iff.EndChunk(RIff::RiffStr2FCC("data") );
					}
				else
					{
					TRACE("Save(): Failed to create 'data' chunk.\n");
					sRes	= -4;
					}
				}
			else
				{
				TRACE("Save(): Failed to create 'fmt ' chunk.\n");
				sRes	= -3;
				}

			// End 'RIFF' 'WAVE' form.
			iff.EndChunk(RIff::RiffStr2FCC("RIFF"), RIff::RiffStr2FCC("WAVE") );
			}
		else
			{
			TRACE("Save(): Failed to create RIFF WAVE form.\n");
			sRes	= -2;
			}
		}
	else
		{
		TRACE("Save(): iff.Open(pfile) failed.\n");
		sRes	= -1;
		}

	return sRes;
	}

///////////////////////////////////////////////////////////////////////////////
//
// Lock this sample for use.
// Returns 0 on success.
// (public)
//
///////////////////////////////////////////////////////////////////////////////
short RSample::Lock(void)
	{
	short sRes = 0; // Assume success.

	// If not being read . . .
	if (m_iff.IsOpen() == FALSE || m_sRefCnt == 0)
		{
		// Increase reference count.
		m_sRefCnt++;
		}
	else
		{
		TRACE("Lock(): Attempt to lock a RSample being read/streamed.\n");
		sRes = -1;
		}
	
	return sRes;
	}

///////////////////////////////////////////////////////////////////////////////
//
// Unlock this sample from use.
// Returns new reference count.
// (public)
//
///////////////////////////////////////////////////////////////////////////////
short RSample::Unlock(void)
	{
	ASSERT(m_sRefCnt > 0);

	// Deduct one from the reference count and return the new value.
	return --m_sRefCnt;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Convert current sample data from 8 bit unsigned to 16 bit signed.
// Returns 0 on success.
//
//////////////////////////////////////////////////////////////////////////////
short RSample::Convert8to16(void)
	{
	short	sRes	= 0;	// Assume success.

	ASSERT(m_sBitsPerSample	== 8);
	ASSERT(m_sRefCnt			== 0);
	ASSERT(m_pData				!= NULL);

	// Allocate space for new data.
	S16*	ps16Dst	= (S16*)malloc(m_lBufSize);

	// If successful . . .
	if (ps16Dst != NULL)
		{
		U8* pu8Src	= (U8*)m_pData;

		for (long l = 0L; l < m_lBufSize; l++)
			{
			ps16Dst[l]	= (S16)((pu8Src[l] << 8) ^ 0x8000);
			}

		// Discard old data.
		// If we own it . . .
		if (m_sOwnBuf == TRUE)
			{
			free(m_pData);
			}

		// Set new data.
		m_pData				= (void*)ps16Dst;
		// Set new size.
		m_lBufSize			*= 2L;
		// Set new sample size.
		m_sBitsPerSample	= 16;
		}
	else
		{
		sRes = -1;
		TRACE("Convert8to16(): Unable to allocate space for new data.\n");
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
