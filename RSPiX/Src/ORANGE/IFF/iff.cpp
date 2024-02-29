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
//////////////////////////////////////////////////////////////////////////////
//
// IFF.CPP
// 
// History:
//		08/14/96 JMI	Started.
//
//		08/30/96	MJR	Corrected AIFF form name (it's "FORM", not "AIFF").
//
//		09/04/96	JMI	Added comment about A/RIFF.
//							Based sizing off of chunk.lSizePos + sizeof(chunk.ulSize)
//							instead of chunk.lDataPos so that the form type is
//							included in the chunk.ulSize.  This made the logic a
//							little trickier and I think it was big time silly for
//							them to have done this.  I would guess it is b/c they 
//							figured it makes it possible to skip a chunk containing
//							subchunks even if you don't know it is a containing
//							chunk.
//							EndChunk() now makes sure that chunks are padded to
//							WORD aligned boundaries.
//							Next() increments to WORD aligned position.
//
//		10/30/96	JMI	Changed:
//							Old label:		New label:
//							=========		=========
//							CIff				RIff
//							CStack			RStack
//		10/31/96	JMI	CNFile			RFile
//							ENDIAN_BIG		BigEndian
//							ENDIAN_LITTLE	LittleEndian
//
//////////////////////////////////////////////////////////////////////////////
//
// This module handles IFF file stuff.  The API is fairly simple but requires
// some explanation.  There are only 3 FORMs currently recognized by this
// module:
//		1) FORM
//		2) RIFF
//		3) LIST
//
// You must use Next() to get even the first chunk.  The first chunk after an
// Open() or Find(".") is always the "RIFF" form (for RIFF files) or "FORM"
// form (for AIFF files).  After you Descend you must use Next() to get the
// first subchunk.  After you Ascend you must use Next() to get the next
// chunk; the Get* functions will return 0 indicating no current chunk until
// Next() is called.
//
// In AIFF/RIFF, the first 8 bytes (i.e., the chunk type and the size) is ex-
// cluded from the size for the chunk.  Note that this is not always the en-
// tire header (FORM types are included in the size).
//
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Headers.
//////////////////////////////////////////////////////////////////////////////
#ifdef PATHS_IN_INCLUDES
	#include "BLUE/system.h"
	#include "BLUE/Blue.h"
	#include "ORANGE/iff/iff.h"
#else
	#include "System.h"
	#include "Blue.h"
	#include "iff.h"
#endif // PATHS_IN_INCLUDES

//////////////////////////////////////////////////////////////////////////////
// Module specific macros.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Module specific typedefs.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Module specific (static) variables.
//////////////////////////////////////////////////////////////////////////////
// Chunk FCC recognized as being FORMs in IFF files.
FCC	RIff::ms_afccIffForms[MAX_FORMS]	=	
	{
	MAKE_IFF_FCC('F', 'O', 'R', 'M'),
	MAKE_IFF_FCC('L', 'I', 'S', 'T'),
	};

// Chunk FCC recognized as being FORMs in RIFF files.
FCC	RIff::ms_afccRiffForms[MAX_FORMS]	=	
	{
	MAKE_RIFF_FCC('R', 'I', 'F', 'F'),
	MAKE_RIFF_FCC('L', 'I', 'S', 'T'),
	};

//////////////////////////////////////////////////////////////////////////////
// Construction/Destruction.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Default constructor.
//
//////////////////////////////////////////////////////////////////////////////
RIff::RIff(void)
	{
	Init();
	}

//////////////////////////////////////////////////////////////////////////////
//
// Destructor.
//
//////////////////////////////////////////////////////////////////////////////
RIff::~RIff(void)
	{
	// If open . . .
	if (RFile::m_fs != NULL)
		{
		Close();
		}
	}

//////////////////////////////////////////////////////////////////////////////
// Methods.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Creates a chunk header of type fcc with room for a 32 bit size field to 
// later be filled in by EndChunk.
// Returns 0 on success.
//
//////////////////////////////////////////////////////////////////////////////
short RIff::CreateChunk(FCC fccChunk, FCC fccForm /*= 0*/)
	{
	short	sRes	= 0;	// Assume success.
	
	// Attempt to allocate CHUNK for stack . . .
	PCHUNK	pChunk = new CHUNK;
	// If successful . . .
	if (pChunk != NULL)
		{
		// Attempt to write the form name . . .
		if (Write(&fccChunk, 4L) == 4L)
			{
			// Store info.
			pChunk->fccChunk	= fccChunk;
			pChunk->fccForm	= fccForm;
			pChunk->lSizePos	= Tell();

			long lDummySize = 0L;
			// Attempt to write 32 bit size field space . . .
			if (Write(&lDummySize, 1L) == 1L)
				{
				pChunk->lDataPos = pChunk->lSizePos + sizeof(lDummySize);

				if (fccForm != 0)
					{
					Write(&fccForm);
					
					pChunk->lDataPos += sizeof(fccForm);
					}

				// Attempt to add to stack . . .
				if (m_stack.Push(pChunk) == 0)
					{
					// Success.
					}
				else
					{
					sRes = -4;
					TRACE("CreateChunk(): Unable to push new CHUNK onto stack.\n");
					}
				}
			else
				{
				sRes = -3;
				TRACE("CreateChunk(): Unable to write space for size field.\n");
				}
			}
		else
			{
			sRes = -2;
			TRACE("CreateChunk(): Unable to write FCC.\n");
			}

		// If an error occurred . . .
		if (sRes != 0)
			{
			// Release memory.
			delete pChunk;
			}
		}
	else
		{
		sRes = -2;
		TRACE("CreateChunk(): Unable to allocate CHUNK for stack.\n");
		}
	
	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Ends a chunk created by CreateChunk.  The fcc parameter is only for
// debugging and may be left out.
// Returns 0 on success.
//
//////////////////////////////////////////////////////////////////////////////
short RIff::EndChunk(FCC fccChunk /*= 0*/, FCC fccForm /*= 0*/)
	{
	short	sRes	= 0;	// Assume success.

	// Get top.
	PCHUNK	pChunk;
	// If successful . . .
	if (m_stack.Pop(&pChunk) == 0)
		{
		// If fcc specified . . . 
		if (fccChunk != 0)
			{
			// Make sure we're in sync.
			ASSERT(fccChunk == pChunk->fccChunk);
			}

		// If fcc specified . . . 
		if (fccForm != 0)
			{
			// Make sure we're in sync.
			ASSERT(fccForm == pChunk->fccForm);
			}

		// Get current file position.
		long	lCurPos	= Tell();
		// Get size.
		long lSize = lCurPos - (pChunk->lSizePos + sizeof(pChunk->ulSize));
		// If size is not WORD aligned . . .
		if ((lSize % 2) != 0)
			{
			// Write a pad byte.
			U8	u8Dummy	= 0;
			Write(&u8Dummy);
			// Increment size and current position.
			lSize++;
			lCurPos++;
			}

		// Seek to size field . . .
		if (Seek(pChunk->lSizePos, SEEK_SET) == 0)
			{
			// Write size.
			if (Write(&lSize, 1L) == 1L)
				{
				// Size successfully written.
				}
			else
				{
				TRACE("EndChunk(): Unable to write size field for chunk.\n");
				sRes = -3;
				}

			// Seek back.
			if (Seek(lCurPos, SEEK_SET) != 0)
				{
				TRACE("EndChunk(): Unable to return to current file pos.\n");
				sRes = -4;
				}
			}
		else
			{
			TRACE("EndChunk(): Unable to seek to chunk's size field.\n");
			sRes = -2;
			}

		// Release the stack item's memory.
		delete pChunk;
		}
	else
		{
		TRACE("EndChunk(): Unable to pop chunk info.\n");
		sRes = -1;
		}
	
	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Closes the file.  We hook this from CNFile to avoid chunks being
// left open.
// Returns 0 on success.
//
//////////////////////////////////////////////////////////////////////////////
short RIff::Close(void)
	{
	short sRes = RFile::Close(); 

	Init();
	
	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Finds the chunk specified by pszPath. "." is used as a separator be-
// tween chunk names.  A "." as the first character of the path, in-
// dicates a full path from the root as opposed to a relative path.
// For example, ".WAVE.fmt " or "fmt ".
// Things to note:
// 1) Relative paths are searched starting at the current level (i.e., the
// current chunk is NOT descended into before searching) and the first 
// chunk checked is the one FOLLOWING the current.
// 2) Any path ending in "." is descended into.
// Examples: ".WAVE." puts you inside the "WAVE" FORM.  ".WAVE" 
// puts you at the "WAVE" FORM.  The path "." puts you above the "RIFF" 
// or "AIFF" chunk.
//
//////////////////////////////////////////////////////////////////////////////
short RIff::Find(char* pszPath)	// Returns 0 on success.
	{
	short	sRes	= 0;	// Assume success.

	if (*pszPath == '.')
		{
		// Discard all chunks and goto root.
		Init();
		
		pszPath++;
		}

	FCC	fccFind;
	FCC	fccNext;
	while (sRes == 0 && *pszPath != '\0')
		{
		// If IFF . . .
		if (m_endian == BigEndian)
			{
			fccFind	= IffStr2FCC(pszPath);
			}
		else
			{
			fccFind	= RiffStr2FCC(pszPath);
			}
		
		do
			{
			// Look at next chunk . .. 
			if (Next() == 0)
				{
				// If this is a FORM . . .
				if (m_chunk.fccForm != 0)
					{
					fccNext	= m_chunk.fccForm;
					}
				else
					{
					fccNext	= m_chunk.fccChunk;
					}
				}
			else
				{
				// No more chunks in this chunk.
				sRes = 1;
				}

			} while (sRes == 0 && fccFind != fccNext);

		// Skip current.
		pszPath	+= 4;

		// If there's a '.' . . .
		if (*pszPath == '.')
			{
			// Go into.
			Descend();
			// Skip '.'.
			pszPath++;
			}
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Move from current chunk to the next chunk at this level.  This must 
// be called before calling Get* functions after an Open.  You
// must call this before calling Descend.  You must descend into chunks 
// that contain multiple sub chunks if you want to use Next to traverse
// them.
//
//////////////////////////////////////////////////////////////////////////////
short RIff::Next(void)	// Returns 0 if successful, 1 if no more chunks, 
								// negative on error.
	{
	short	sRes	= 0;	// Assume success.

	long	lNextPos		= GetNextChunkPos(&m_chunk);
	
	// Get the containing chunk.
	PCHUNK	pchunk;
	if (m_stack.GetTop(&pchunk) == 0)
		{
		// If this seek would not put us passed the end of the containing chunk . . .
		if (lNextPos < GetNextChunkPos(pchunk) )
			{
			// Next chunk is within containing chunk.
			}
		else
			{
			// No more sub chunks.
			sRes	= 1;
			}
		}

	// If okay so far . . .
	if (sRes == 0)
		{
		// Move to next chunk's header.
		if (RelSeek(lNextPos) == 0)
			{
			switch (ReadChunkHeader())
				{
				case 0:	// Successfully moved to next chunk.
					break;

				case 1:	// EOF.
					sRes	= 1;
					break;

				default:	// Error.
					TRACE("Next(): ReadChunkHeader() failed.\n");
					sRes	= -2;
					break;
				}
			}
		else
			{
			TRACE("Next(): SeekRel() failed.\n");
			sRes	= -1;
			}
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Descend into current chunk.  This must be called for the top-level
// chunk in the file in order to parse the sub chunks.  It can be called
// for any chunk that contains sub chunks.
// After calling Descend, there is no current chunk until Next is called.
//
//////////////////////////////////////////////////////////////////////////////
short RIff::Descend(void)	// Returns 0 if successful, 1 if no subchunks,
									// negative on error.                           
	{
	short	sRes	= 0;	// Assume success.

	// We should only descend into chunks that are FORMs.
	if (m_chunk.fccForm != 0L)
		{
		PCHUNK	pchunk	= new CHUNK;
		if (pchunk != NULL)
			{
			// Copy chunk.
			*pchunk	= m_chunk;
			// Push onto stack . . .
			if (m_stack.Push(pchunk) == 0)
				{
				// By simply initializing these, the next Next call will take us to the
				// first subchunk.
				m_chunk.fccChunk	= 0L;
				m_chunk.fccForm	= 0L;
				m_chunk.ulSize		= 4L;	// Skip form type.
				// Leave data/size position so we know where to seek from.
				}
			else
				{
				TRACE("Descend(): Unable to push chunk onto stack.\n");
				sRes = -3;
				}

			// If an error occurred after allocation . .. 
			if (sRes != 0)
				{
				delete pchunk;
				}
			}
		else
			{
			TRACE("Descend(): Unable to allocate new chunk for stack.\n");
			sRes = -2;
			}
		}
	else
		{
		char	szFCC[5];
		GetChunk(szFCC);
		
		TRACE("Descend(): Attempt to descend into a chunk that is not a form "
				"(current chunk <%s>).\n", szFCC);
		sRes	= -1;
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Ascend out of a chunk previously Descend'ed into.  LIFO ordering, of
// course.  You must call Next after calling Ascend before any values
// are valid.
//
//////////////////////////////////////////////////////////////////////////////
short RIff::Ascend(void)	// Returns 0 if successful, 1 if no more chunks,
									// negative on error.                           
	{
	short	sRes	= 0;	// Assume success.

	// Attempt to pop stack . . .
	PCHUNK	pchunk;
	if (m_stack.Pop(&pchunk) == 0)
		{
		m_chunk	= *pchunk;
		// Destroy chunk data.
		delete pchunk;
		// By simply initializing these, the next Next call will take us to the
		// next chunk at this level.
		m_chunk.fccChunk	= 0L;
		m_chunk.fccForm	= 0L;
		m_chunk.ulSize		= 0L;
		// Leave data position so we know where to seek from.
		}
	else
		{
		TRACE("Ascend(): CStack::Pop failed.\n");
		sRes = -1;
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Initialize all members.
//
//////////////////////////////////////////////////////////////////////////////
void RIff::Init()
	{
	m_chunk.fccChunk	= 0L;
	m_chunk.fccForm	= 0L;
	m_chunk.ulSize		= 0L;
	m_chunk.lDataPos	= 0L;
	m_chunk.lSizePos	= 0L;

	// Clean up remaining stack.
	PCHUNK	pchunk;
	while (m_stack.GetNumItems() != 0)
		{
		if (m_stack.Pop(&pchunk) == 0)
			{
			delete pchunk;
			}
		}
	}

//////////////////////////////////////////////////////////////////////////////
//
// Seek to non-relative position lPos but check current position first 
// and do a relative seek (SEEK_CUR).  This should be better for reading
// from CD.  Won't seek if the distance is 0.
// Returns 0 on success.
//
//////////////////////////////////////////////////////////////////////////////
short RIff::RelSeek(long lPos)
	{
	short	sRes	= 0;	// Assume success.

	// Determine distance to destination.
	long	lDistance	= lPos - Tell();
	// If there is a distance . . .
	if (lDistance != 0)
		{
		if (Seek(lDistance, SEEK_CUR) == 0)
			{
			}
		else
			{
			TRACE("RelSeek(): CNFile::Seek() failed.\n");
			sRes	= -1;
			}
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Determine if fcc is a FORM type.
//
//////////////////////////////////////////////////////////////////////////////
short RIff::IsForm(	// Returns TRUE if fcc is a form; FALSE otherwise.
	FCC fcc)
	{
	FCC*	pfcc	= (m_endian == LittleEndian)	? ms_afccRiffForms 
																: ms_afccIffForms;
	while (*pfcc != 0 && *pfcc != fcc)
		{
		pfcc++;
		}

	return (*pfcc == fcc) ? TRUE : FALSE;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Read chunk header.
//
//////////////////////////////////////////////////////////////////////////////
short RIff::ReadChunkHeader(void)	// Returns 0 on success.
	{
	// Read common header info.
	Read(&(m_chunk.fccChunk));
	Read(&(m_chunk.ulSize));
	
	// Get size and data position.
	m_chunk.lDataPos	= Tell();
	m_chunk.lSizePos	= m_chunk.lDataPos - sizeof(m_chunk.ulSize);

	// Check if this is a form.
	if (IsForm(m_chunk.fccChunk) != FALSE)
		{
		Read(&m_chunk.fccForm);
		m_chunk.lDataPos	+= sizeof(m_chunk.fccForm);
		}
	else
		{
		m_chunk.fccForm	= 0L;
		}

	short sRes	= 0;
	// Error only if CNFile thinks so.
	if (Error() == 0)
		{
		if (IsEOF() == 0)
			{
			// Cool.
			}
		else
			{
			// End of file.
			sRes = 1;
			}
		}
	else
		{
		TRACE("ReadChunkHeader(): CNFile::Error() reported an error.\n");
		sRes	= -1;
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
