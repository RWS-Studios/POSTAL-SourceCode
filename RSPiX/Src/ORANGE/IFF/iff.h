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
// IFF.H
// 
// History:
//		08/14/96 JMI	Started.
//
//		03/25/97	JMI	Started tracking history of this file.
//							TOTALLY HOSED GetNextChunkPos() so it will not work for
//							root case unless you already happen to be where the root
//							is!  It should be changed back from:
//			// The freshly initialized chunk is a special case . . .
//			if (pchunk->lSizePos == 0)
//				return Tell();
//			else
//				return pchunk->lSizePos							// File position of size.
//						+ sizeof(pchunk->ulSize)				// Size of size field.
//						+ pchunk->ulSize							// Size of chunk.
//						+ ((pchunk->ulSize % 2) ? 1 : 0);	// New chunks on even
//							to:
//			// The freshly initialized chunk is a special case . . .
//			if (pchunk->lSizePos == 0)
//				return 0;
//			else
//				return pchunk->lSizePos							// File position of size.
//						+ sizeof(pchunk->ulSize)				// Size of size field.
//						+ pchunk->ulSize							// Size of chunk.
//						+ ((pchunk->ulSize % 2) ? 1 : 0);	// New chunks on even
//
//////////////////////////////////////////////////////////////////////////////
#ifndef IFF_H
#define IFF_H

//////////////////////////////////////////////////////////////////////////////
// Please see the CPP file for an explanation of this API.
//////////////////////////////////////////////////////////////////////////////

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
	#include "ORANGE/File/file.h"
	#include "ORANGE/CDT/stack.h"
#else
	#include "file.h"
	#include "stack.h"
#endif // PATHS_IN_INCLUDES

//////////////////////////////////////////////////////////////////////////////
// Macros.
//////////////////////////////////////////////////////////////////////////////
// Converts four characters to a four character code (a U32).
//#define MAKE_IFF_FCC(a,b,c,d)		RIff::IffStr2FCC(#a #b #c #d);
//#define MAKE_RIFF_FCC(a,b,c,d)	RIff::RiffStr2FCC(#a #b #c #d);
// Compile time macros.
#define MAKE_IFF_FCC(a, b, c, d)		(((a) << 24) | ((b) << 16) | ((c) << 8) | (d))
#define MAKE_RIFF_FCC(a, b, c, d)	(((d) << 24) | ((c) << 16) | ((b) << 8) | (a))


#define MAX_FORMS		50

//////////////////////////////////////////////////////////////////////////////
// Typedefs.
//////////////////////////////////////////////////////////////////////////////

typedef U32 FCC;

//////////////////////////////////////////////////////////////////////////////
class RIff : public RFile
	{
	public:	// Construction/Destruction.
		// Default constructor.
		RIff(void);
		// Destructor.
		~RIff(void);

//////////////////////////////////////////////////////////////////////////////

	public: // Typedefs.
		typedef struct
			{
			FCC	fccChunk;	// Chunk type.
			FCC	fccForm;		// Form type for Form chunks, otherwise 0.
			ULONG	ulSize;		// Size of this chunk.
			long	lDataPos;	// Position, in file, of data for this chunk.
			long	lSizePos;	// Position, in file, of size for this chunk.  Used
									// for writing chunks.
			} CHUNK, *PCHUNK;

	public:	// Methods.

		////////////////////////////////////////////////////////////////////////
		// General.
		////////////////////////////////////////////////////////////////////////
		
		// Closes the file.  We hook this from CNFile to avoid chunks being
		// left open.
		// Returns 0 on success.
		short Close(void);

		// Gets the string representation of the FCC for the current chunk.
		short GetChunk(	// Returns 0 on success.
			char* pszFCC)	// Array of at least FIVE(5(V)) bytes for string.
			{
			// If this is a RIFF file . . .
			if (m_endian == LittleEndian)
				RiffFCC2Str(m_chunk.fccChunk, pszFCC);
			else
				IffFCC2Str(m_chunk.fccChunk, pszFCC);

			return (m_chunk.fccChunk == 0) ? 1 : 0;
			}

		ULONG GetChunk(void)		// Returns FCC of current chunk.
			{ return m_chunk.fccChunk; }

		// Gets the FCC of the current chunk's FORM, if there is one.
		short GetForm(				// Returns non-zero if no form.
			char* pszFCC)			// Array of at least FIVE(5(V)) bytes for string.
			{
			// If this is a RIFF file . . .
			if (m_endian == LittleEndian)
				RiffFCC2Str(m_chunk.fccForm, pszFCC);
			else
				IffFCC2Str(m_chunk.fccForm, pszFCC);

			return (m_chunk.fccForm == 0) ? 1 : 0;
			}

		ULONG GetForm(void)		// Returns FCC or 0 if no form for this chunk.
			{ return m_chunk.fccForm; }

		ULONG GetSize(void)		// Returns size of current chunk.
			{ return m_chunk.ulSize; }

		// Get the file position of the next to the specified chunk.
		long	GetNextChunkPos(		// Returns the position of the next to the 
											// chunk specified below.
			CHUNK* pchunk)				// Chunk to evaluate.
			{
			// The freshly initialized chunk is a special case . . .
			if (pchunk->lSizePos == 0)
				return Tell();
			else
				return pchunk->lSizePos							// File position of size.
						+ sizeof(pchunk->ulSize)				// Size of size field.
						+ pchunk->ulSize							// Size of chunk.
						+ ((pchunk->ulSize % 2) ? 1 : 0);	// New chunks on even
																		// (16 bit) boundaries.
			}

		////////////////////////////////////////////////////////////////////////
		// Reading.
		////////////////////////////////////////////////////////////////////////

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
		short Find(char* pszPath);	// Returns 0 on success.

		// Move from current chunk to the next chunk at this level.  This must 
		// be called before calling Get* functions after an Open.  You
		// must call this before calling Descend.  You must descend into chunks 
		// that contain multiple sub chunks if you want to use Next to traverse
		// them.
		short Next(void);			// Returns 0 if successful, 1 if no more chunks, 
										// negative on error.
		
		// Descend into current chunk.  This must be called for the top-level
		// chunk in the file in order to parse the sub chunks.  It can be called
		// for any chunk that contains sub chunks.
		// After calling Descend, there is no current chunk until Next is called.
		short Descend(void);		// Returns 0 if successful, 1 if no subchunks,
										// negative on error.                           

		// Ascend out of a chunk previously Descend'ed into.  LIFO ordering, of
		// course.  You must call Next after calling Ascend before any values
		// are valid.
		short Ascend(void);		// Returns 0 if successful, 1 if no more chunks,
										// negative on error.                           

		////////////////////////////////////////////////////////////////////////
		// Writing.
		////////////////////////////////////////////////////////////////////////
		
		// Creates a chunk header of type fccChunk with room for a 32 bit size 
		// field to later be filled in by EndChunk.  If fccForm is not 0, a new 
		// form is created.
		// Returns 0 on success.
		short CreateChunk(FCC fccChunk, FCC fccForm = 0);

		// Ends a chunk created by CreateChunk.  The fcc parameters are only for
		// debugging and may be left out.
		// Returns 0 on success.
		short EndChunk(FCC fccChunk = 0, FCC fccForm = 0);

//////////////////////////////////////////////////////////////////////////////

	public:	// Static
		// Create FCC code for an IFF file based on a 4-character string
		// Platform independent.  Open IFF files as ENDIAN_BIG.
		static ULONG IffStr2FCC(char* pszFCC)
			{ return (pszFCC[0] << 24) | (pszFCC[1] << 16) | (pszFCC[2] << 8) | (pszFCC[3]); }
			
		// Create FCC code for an RIFF file based on a 4-characater string
		// Platform independent.  Open RIFF files as ENDIAN_LITTLE.
		static ULONG RiffStr2FCC(char* pszFCC)
			{ return (pszFCC[3] << 24) | (pszFCC[2] << 16) | (pszFCC[1] << 8) | (pszFCC[0]); }

		// Create 4-character string for an IFF file based on FCC.
		// Platform independent.  Open IFF files as ENDIAN_BIG.
		static void IffFCC2Str(	ULONG ulFCC,	// FCC to convert.
										char* pszFCC)	// String of at least 5 bytes.
			{	
			pszFCC[0]	= (char)(ulFCC >> 24);
			pszFCC[1]	= (char)(ulFCC >> 16);
			pszFCC[2]	= (char)(ulFCC >> 8);
			pszFCC[3]	= (char)(ulFCC);	
			pszFCC[4]	= '\0';
			}
		
		// Create 4-character string for an RIFF file based on FCC.
		// Platform independent.  Open RIFF files as ENDIAN_LITTLE.
		static void RiffFCC2Str(ULONG ulFCC,	// FCC to convert.            
										char* pszFCC)	// String of at least 5 bytes.
			{	
			pszFCC[3]	= (char)(ulFCC >> 24);
			pszFCC[2]	= (char)(ulFCC >> 16);
			pszFCC[1]	= (char)(ulFCC >> 8);
			pszFCC[0]	= (char)(ulFCC);	
			pszFCC[4]	= '\0';
			}
		

//////////////////////////////////////////////////////////////////////////////

	public:	// Querries.

//////////////////////////////////////////////////////////////////////////////

	protected:	// Internal functions.
		// Seek to non-relative position lPos but check current position first 
		// and do a relative seek (SEEK_CUR).  This should be better for reading
		// from CD.  Won't seek if the distance is 0.
		// Returns 0 on success.
		short RelSeek(long lPos);		

		// Initialize members.
		void Init(void);

		// Read chunk header.
		short ReadChunkHeader(void);	// Returns 0 on success.

		// Determine if fcc is a FORM type.
		short IsForm(	// Returns TRUE if fcc is a form; FALSE otherwise.
			FCC fcc);			

//////////////////////////////////////////////////////////////////////////////

	public:	// Member variables.

	protected:	// Protected member variables.

		RStack <PCHUNK>	m_stack;
		CHUNK					m_chunk;

		static FCC			ms_afccIffForms[MAX_FORMS];	// Chunk FCC recognized 
																		// as being FORMs in IFF
																		// files.
		static FCC			ms_afccRiffForms[MAX_FORMS];	// Chunk FCC recognized 
																		// as being FORMs in RIFF
																		// files.

	};

#endif // IFF_H
//////////////////////////////////////////////////////////////////////////////
//	EOF
//////////////////////////////////////////////////////////////////////////////
