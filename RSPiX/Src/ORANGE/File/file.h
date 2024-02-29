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
// File.h
// 
// History:
//		07/29/95 JMI	Started.
//
//		01/22/97	JMI	Added NeutralEndian which never byte swaps and additional
//							overload for Open() to open memfiles that grow when writes
//							exceed their bounds.
//
//		01/28/97	JMI	Added yet another Open() overload that allows one to open
//							an existing FILE* stream with an RFile.
//
//		01/28/97	JMI	Added yet another Open() overload that allows one to open
//							an existing FILE* stream with an RFile.
//							And yet another Open() which opens from another RFile
//							that is already attached to disk or memory.  This is use-
//							ful for descended classes such as RIff (so they can sub-
//							class an existing RFile).
//
//		02/04/97	MJR/JMI	Added RFileEZSave() and RFileEZLoad() that are meant
//							to replace the myclass.Load(char*) and 
//							myclass.Save(char*) that has been written so far for 
//							nearly every class that loads and saves.  These new funcs
//							are essentially macros that will allocate an RFile, open
//							it with the specified filename, permissions, and endian 
//							format, call the specified class's Load or Save, check
//							for error, close the RFile, and deallocate the RFile.
//
//		02/05/97	JMI	Protected Read and Write for void* (see comments there
//							for details).  Consequently, I had to add an overload
//							for char* that was being caught by the void* version
//							before.
//							Also, added Read and Writes for RPixel32.
//
//		02/10/97	JMI	Added rspAnyLoad() and rspAnySave().  There are two types
//							of implementations for these functions.  One is the 
//							explicit overload where someone defines an rspAnyLoad/Save
//							specific to the overloaded type and the other is implicit
//							in that there is a template function that catches all the
//							ones that don't have an explicit overload.
//							Also, added explicit overloads for most RFile supported
//							types.
//
//		02/10/97	JMI	RFILE_INSTANTIATE_ANYLOAD and SAVE had a bug where they
//							returned the number of items read or written instead of
//							0 indicating success or non-zero indicating failure.
//
//		06/28/97 MJR	Metrowerks compiler has a problem with explicit functions
//							being created AFTER templates of the same name are
//							defined.  I simply rearranged the order so it works.
//
//		10/06/99	JMI	rspEZSave() was not deleting its RFile when done.
//
//////////////////////////////////////////////////////////////////////////////
//
// See CPP comment for details.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef FILE_H
#define FILE_H

//////////////////////////////////////////////////////////////////////////////
// Headers.
//////////////////////////////////////////////////////////////////////////////
#include <stdio.h>

#include "Blue.h"
// If PATHS_IN_INCLUDES macro is defined, we can utilized relative
// paths to a header file.  In this case we generally go off of our
// RSPiX root directory.  System.h MUST be included before this macro
// is evaluated.  System.h is the header that, based on the current
// platform (or more so in this case on the compiler), defines 
// PATHS_IN_INCLUDES.  Blue.h includes system.h so you can include that
// instead.
#ifdef PATHS_IN_INCLUDES
	#ifdef ALLOW_RFILE_REOPEN
		#include "ORANGE/CDT/List.h"
	#endif // ALLOW_RFILE_REOPEN
#else
	#ifdef ALLOW_RFILE_REOPEN
		#include "List.h"
	#endif // ALLOW_RFILE_REOPEN
#endif // PATHS_IN_INCLUDES

//////////////////////////////////////////////////////////////////////////////
// Macros.
//////////////////////////////////////////////////////////////////////////////

#ifdef ALLOW_RFILE_REOPEN
	#define KEEPCONNECTEDANDUPDATELASTACCESS	Reconnect(), m_lLastAccess = rspGetMilliseconds()
#else	// ALLOW_RFILE_REOPEN
	#define KEEPCONNECTEDANDUPDATELASTACCESS
#endif // ALLOW_RFILE_REOPEN

#define MAX_MODE_LEN		6
#define MAX_NAME_LEN		512

// Size of buffer used to byte swap by Write().  It'd be safe to make sure
// this number is a multiple of 8 (our largest base type is 8 bytes).
#define RFILE_SWAP_SIZE	1024

//////////////////////////////////////////////////////////////////////////////
// Typedefs.
//////////////////////////////////////////////////////////////////////////////

// Forward declared for typedef'd protos.
class RFile;

//////////////////////////////////////////////////////////////////////////////
class RFile
	{
	public:	// Typedefs & Enums.
		// Use for flags field of Open():
		typedef enum
			{
			// No flags.
			NoFlags	= 0x0000,
			// Binary mode.  All numbers written to file as numbers.
			Binary	= 0x0001,
			// ASCII mode.  All numbers written to file as text.
			Ascii		= 0x0002
			} Flags;

		// Use for endian field of Open and SetEndian calls.
		typedef enum
			{
			BigEndian,			// Big endian (byte swaps on little endian systems).
			NeutralEndian,		// Neutral endian (Never byte swaps).
			LittleEndian		// Little endian (byte swaps on big endian systems).
			} Endian;

		typedef short (*OpenHook)(	// Returns 0 to bypass default Open's 
											// functionality.  Non-zero for normal ops.
			RFile* pfile,				// Pointer to RFile being opened.
			const char* pszFileName,		// File path and name to open.
			const char* pszFlags,			// Open flags ala fopen.
			Endian endian,				// Endian nature of file.
			long lUser);				// User value.

		typedef short (*CloseHook)(	// Returns 0 to bypass default Close's
												// functionality.
			RFile* pfile,					// Pointer to RFile being closed.
			long lUser);					// User value.

		// This is the type of the function that is called before every disk
		// read or write operation to let the user know of (and, perhaps, act
		// upon) delays that may occur.
		typedef void (*CritiCall)(	// Returns nothing.
			long	lBytes);				// Bytes about to be read or written.

	public:	// Construction/Destruction.
		// Default constructor.
		RFile(void);
		// Destructor.
		~RFile(void);

//////////////////////////////////////////////////////////////////////////////

	public:	// Methods.
		////////////////////////////////////////////////////////////////////////
		// Open, Close, and operation flags.
		////////////////////////////////////////////////////////////////////////

		// Open file pszFileName with fopen flags pszFlags and endian format 
		// endian { RFile::Big, RFile::Little }.
		// Returns 0 on success.
		short Open(					// Returns 0 on success.
			const char* pszFileName,	// Filename to open.
			const char* pszFlags,		// fopen flags to use for opening.
			Endian endian,			// { RFile::BigEndian | RFile::LittleEndian | RFile::NeutralEndian }.
			Flags flags	=			// See comments in Typedefs & Enums section 
				Binary);				// above.

		// Open memory pFile of size lSize and endian format endian.
		// Size and location of memory will not be affected by RFile.
		// { RFile::Big, RFile::Little }.
		// Returns 0 on success.
		short Open(					// Returns 0 on success.
			void* pFile,			// Pointer to memory to open.
			long lSize,				// Size of *pFile in bytes.
			Endian endian);		// { RFile::BigEndian | RFile::LittleEndian | RFile::NeutralEndian }.

		// Open memory pFile of size lSize and endian format endian.
		// RFile may size and or relocate the memory in order to expand the memory file.
		// Deallocates on Close().
		// { RFile::Big, RFile::Little }.
		// Returns 0 on success.
		short Open(					// Returns 0 on success.
			long	lSize,			// Size in bytes to begin with.
			long	lGrowSize,		// Min amount in bytes to grow memory file when written passed end.
										// Note: The larger of lGrowSize and the amount overwritten will
										// be allocated in the case of an overrun.
			Endian endian);		// { RFile::BigEndian | RFile::LittleEndian | RFile::NeutralEndian }.

		// Open an existing FILE* stream.
		// Once a FILE* is opened, you can use this class's Close() instead of fclose(),
		// if that is more convenient.
		short Open(					// Returns 0 on success.
			FILE*	fs,				// FILE* stream to open.
			Endian endian,			// { RFile::BigEndian | RFile::LittleEndian | RFile::NeutralEndian }.
			Flags flags	=			// See comments in Typedefs & Enums section 
				Binary);				// above.

		// Open an existing RFile.
		// "Ack!" you say ... and I agree.
		// This basically begins what I like to think of as a synchronization between two RFiles.
		// This RFile snags the current state (basically copies the members) from the specified
		// RFile.  Then, one can use the new RFile to access the file/memory/whatever that the
		// original RFile is attached to.  When Close() is called, the synchronization is finsished
		// by updating the original RFile with the state from this.
		// Danger:  Do not access the original RFile between Open(RFile*)/Close() pairs!
		short Open(					// Returns 0 on success.
			RFile* pfile);			// RFile to open.

		// Sets an Open(char*...) hook.  NOTE: You CAN call any RFile Open from
		// within an Open hook.  The Open hook should return 0 to bypass RFile's
		// functionality.
		static void SetOpenHook(OpenHook hook, long lUser)
			{ ms_hOpen = hook; ms_lOpenUser = lUser; }

		// Change the endian format used to read/write the file.
		// Returns nothing.
		void SetEndian(Endian endian);

		// Set the size of the buffer used to read from the disk.
		// This function only applies to files open from the disk.
		// Returns 0 on success.
		short SetBufferSize(size_t stBufSize)
			{
			if (IsFile() == TRUE)
				{
				KEEPCONNECTEDANDUPDATELASTACCESS;
				return (short)setvbuf(m_fs, NULL, _IOFBF, stBufSize);
				}
			else
				if (IsMemory() == TRUE)
					return 0;
				else
					return -1;
			}

		// Close a file successfully opened with Open().
		// Returns 0 on success.
		short Close(void);

		// Sets a Close() hook.  NOTE: You CAN call any RFile Close from
		// within a Close hook.  The Close hook should return 0 to bypass RFile's
		// functionality.
		static void SetCloseHook(CloseHook hook, long lUser)
			{ ms_hClose = hook; ms_lCloseUser = lUser; }

		////////////////////////////////////////////////////////////////////////
		// Position.
		////////////////////////////////////////////////////////////////////////

		// Seeks within the file based on the supplied position argument
		// { SEEK_SET, SEEK_CUR, SEEK_END }.
		// Returns 0 on success.
		short Seek(long lPos, long lOrigin);

		// Returns the current file position or -1 on error.
		long Tell(void);

		////////////////////////////////////////////////////////////////////////
		// Read.
		////////////////////////////////////////////////////////////////////////

	protected:
		// NOTE:  THIS FUNCTION IS PROTECTED FOR AN IMPORTANT REASON:
		// This function was a catch all for any unsupported type.  The dis-
		// advantage was that, if you had a type that needed to be swapped
		// but was not supported, this API would happily accept it, but not
		// swap it.  This could cause hard to detect problems.
		// If you are reading or writing something that does not require 
		// swapping, cast it as U8* or S8*.
		// Reads lNum bytes from currently open file.
		// Returns number of bytes successfully read.
		long Read(void* pData, long lNum);

	public:
		// Reads lNum char values from currently open file.
		// Returns number of char values successfully read.
		long Read(char* pcData, long lNum)
			{ return Read((void*)pcData, lNum); }

		// Reads lNum U8 values from currently open file.
		// Returns number of U8 values successfully read.
		long Read(U8*	pu8Data, long lNum = 1L);

		// Reads lNum S8 values from currently open file.
		// Returns number of S8 values successfully read.
		long Read(S8*	ps8Data, long lNum = 1L);

		// Reads lNum U16 values from currently open file.
		// Returns number of U16 values successfully read.
		long Read(U16* pu16Data, long lNum = 1L);
		
		// Reads lNum S16 values from currently open file.
		// Returns number of S16 values successfully read.
		long Read(S16* ps16Data, long lNum = 1L);

		// Reads lNum RPixel24 values from currently open file.
		// Returns number of RPixel24 values successfully read.
		long Read(RPixel24* ppix24, long lNum = 1L);

		// Reads lNum U32 values from currently open file.
		// Returns number of U32 values successfully read.
		long Read(U32* pu32Data, long lNum = 1L);

		// Reads lNum S32 values from currently open file.
		// Returns number of S32 values successfully read.
		long Read(S32* ps32Data, long lNum = 1L);

		// Reads lNum RPixel32 values from currently open file.
		// Returns number of RPixel32 values successfully read.
		long Read(RPixel32* ppix32Data, long lNum = 1L)
			{ return Read((U32*)ppix32Data, lNum); }

		// Reads lNum U64 values from currently open file.
		// Returns number of U64 values successfully read.
		long Read(U64* pu64Data, long lNum = 1L);

		// Reads lNum S64 values from currently open file.
		// Returns number of S64 values successfully read.
		long Read(S64* ps64Data, long lNum = 1L);

		// Reads lNum float values from currently open file.
		// Returns number of float values successfully read.
		long Read(float* pfData, long lNum = 1L);

		// Reads lNum double values from currently open file.
		// Returns number of double values successfully read.
		long Read(double* pdData, long lNum = 1L);

		// Reads a NULL terminated string.  The '\0' must be
		// actually in the file to denote the end of the string.
		// pszString must point to a memory block sufficiently large
		// enough to hold the string.
		// Returns number of characters successfully read,
		// including the NULL terminator (unlike strlen()).
		long Read(char* pszString);

		////////////////////////////////////////////////////////////////////////
		// Write.
		////////////////////////////////////////////////////////////////////////

	protected:
		// NOTE:  THIS FUNCTION IS PROTECTED FOR AN IMPORTANT REASON:
		// This function was a catch all for any unsupported type.  The dis-
		// advantage was that, if you had a type that needed to be swapped
		// but was not supported, this API would happily accept it, but not
		// swap it.  This could cause hard to detect problems.
		// If you are reading or writing something that does not require 
		// swapping, cast it as U8* or S8*.
		// Writes lNum bytes from currently open file.
		// Returns number of bytes successfully written.
		long Write(const void* pData, long lNum);

	public:
		// Writes lNum char values to currently open file.
		// Returns number of U8 values successfully written.
		long Write(const char*	pcData, long lNum)
			{ return Write((void*)pcData, lNum); }

		// Writes lNum U8 values to currently open file.
		// Returns number of U8 values successfully written.
		long Write(const U8*	pu8Data, long lNum = 1L);

		// Writes lNum S8 values to currently open file.
		// Returns number of S8 values successfully written.
		long Write(const S8*	ps8Data, long lNum = 1L);

		// Writes lNum U16 values to currently open file.
		// Returns number of U16 values successfully written.
		long Write(const U16* pu16Data, long lNum = 1L);

		// Writes lNum S16 values to currently open file.
		// Returns number of S16 values successfully written.
		long Write(const S16* ps16Data, long lNum = 1L);

		// Writes lNum RPixel24 values to currently open file.
		// Returns number of RPixel24 values successfully written.
		long Write(const RPixel24* ppix24, long lNum = 1L);

		// Writes lNum U32 values to currently open file.
		// Returns number of U32 values successfully written.
		long Write(const U32* pu32Data, long lNum = 1L);

		// Writes lNum S32 values to currently open file.
		// Returns number of S32 values successfully written.
		long Write(const S32* ps32Data, long lNum = 1L);

		// Writes lNum RPixel32 values to currently open file.
		// Returns number of RPixel32 values successfully written.
		long Write(const RPixel32* ppix32Data, long lNum = 1L)
			{ return Write((U32*)ppix32Data, lNum); }

		// Writes lNum U64 values to currently open file.
		// Returns number of U64 values successfully written.
		long Write(const U64* pu64Data, long lNum = 1L);

		// Writes lNum S64 values to currently open file.
		// Returns number of S64 values successfully written.
		long Write(const S64* ps64Data, long lNum = 1L);

		// Writes lNum float values to the currently open file.
		// Returns number of float values successfully written.
		long Write(const float* pfData, long lNum = 1L);

		// Writes lNum double values to the currently open file.
		// Returns number of double values successfully written.
		long Write(const double* pdData, long lNum = 1L);

		// Writes a NULL terminated string.  The '\0' must be
		// actually written to file to denote the end of the string.
		// Returns number of characters successfully written,
		// including the NULL terminator (unlike strlen()).
		long Write(const char* pszString);

		////////////////////////////////////////////////////////////////////////
		// These functions write one value when passed by value.
		// This is useful for writing literals, variables that should be written
		// by other than their type, and for variables of types that are not
		// pointer overloaded by other Write(type*, long) RFile functions.
		////////////////////////////////////////////////////////////////////////

		// Writes one char.
		long Write(char cVal)
			{ return Write(&cVal, 1); }

		// Writes one U8.
		long Write(U8 u8Val)
			{ return Write(&u8Val); }

		// Writes one S8.
		long Write(S8 s8Val)
			{ return Write(&s8Val); }

		// Writes one U16.
		long Write(U16 u16Val)
			{ return Write(&u16Val); }

		// Writes one S16.
		long Write(S16 s16Val)
			{ return Write(&s16Val); }

		// Writes one RPixel24.
		long Write(RPixel24 pix24Val)
			{ return Write(&pix24Val); }

		// Writes one U32.
		long Write(U32 u32Val)
			{ return Write(&u32Val); }

		// Writes one S32.
		long Write(S32 s32Val)
			{ return Write(&s32Val); }

		// Writes one RPixel32.
		long Write(RPixel32 pix32Val)
			{ return Write(&pix32Val); }

		// Writes one float.
		long Write(float fVal)
			{ return Write(&fVal); }

		// Writes one U64.
		long Write(U64 u64Val)
			{ return Write(&u64Val); }

		// Writes one S64.
		long Write(S64 s64Val)
			{ return Write(&s64Val); }

		// Writes one double.
		long Write(double dVal)
			{ return Write(&dVal); }

//////////////////////////////////////////////////////////////////////////////

	public:	// Querries.
		////////////////////////////////////////////////////////////////////////
		// Querries.
		////////////////////////////////////////////////////////////////////////

		// Returns TRUE, if open, FALSE if closed.
		short IsOpen(void)	{ return (m_fs == NULL && m_pucFile == NULL ? FALSE : TRUE); }

		// Returns TRUE, if connected to a non-memory file.
		short IsFile(void)	{ return (m_fs == NULL ? FALSE : TRUE); }

		// Returns TRUE, if connected to a memory file.
		short IsMemory(void)	{ return (m_pucFile == NULL ? FALSE : TRUE); }

		// Returns FALSE if no error has occurred on this stream; TRUE if an
		// error has occurred.
		short Error(void) 
			{ 
			if (IsFile() == TRUE)
				{
				KEEPCONNECTEDANDUPDATELASTACCESS;
				return (ferror(m_fs) == 0 ? FALSE : TRUE);
				}
			else
				if (IsMemory() == TRUE)
					return m_sMemError;
				else
					return TRUE;
			}

		// Clears any error that has occurred on this stream.
		void ClearError(void) 
			{ 
			if (IsFile() == TRUE)
				{
				KEEPCONNECTEDANDUPDATELASTACCESS;
				clearerr(m_fs);
				}
			else
				if (IsMemory() == TRUE)
					m_sMemError	= FALSE;
			}

		// Returns TRUE if end of file has been reached, FALSE otherwise.
		// For disk files, does not return TRUE until the first i/o operation
		// fails due to EOF.
		short IsEOF(void)
			{
			if (IsFile() == TRUE)
				{
				KEEPCONNECTEDANDUPDATELASTACCESS;
				return (feof(m_fs) == 0 ? FALSE : TRUE);
				}
			else
				if (IsMemory() == TRUE)
					return (m_pucCur == m_pucFile + m_lSize);
				else
					return TRUE;
			}

		// Returns the size of the file on success.  Negative on error.
		long GetSize(void);

		// Returns the endian setting for this object.
		short GetEndian(void)
			{ return m_endian; }

		// Returns the memory ptr if this is a memory file; NULL, otherwise.
		// NOTE:  This buffer may move after any Write() call that exceeds the
		// existing buffer size.
		UCHAR* GetMemory(void)
			{ return m_pucFile; }

//////////////////////////////////////////////////////////////////////////////
	protected:	// Functions for my friends.

		// Sets the instantiable user value.
		void SetUserVal(long lUser)
			{ m_lUser = lUser; }

		// Gets the instantiable user value.
		long GetUserVal(void)
			{ return m_lUser; }

//////////////////////////////////////////////////////////////////////////////

	protected:	// Internal functions.

/////////// Binary Reads /////////////////////////////////////////////////////

		// Reads in 8 bit data, swapped if necessary (BWAH HA).
		long Read8(		// Returns number of 8 bit items read.
			U8*	pu8,		// In:  8 bit data to read (swapping, if necessary).
			long	lNum);	// In:  Number of 8 bit items to read.

		// Reads in 16 bit data, swapped if necessary.
		long Read16(		// Returns number of 16 bit items read.
			U16*	pu16,		// In:  16 bit data to read (swapping, if necessary).
			long	lNum);	// In:  Number of 16 bit items to read.

		// Reads in 24 bit data, swapped if necessary.
		long Read24(			// Returns number of 24 bit items read.
			RPixel24* ppix24,	// In:  24 bit data to read (swapping, if necessary).
			long	lNum);		// In:  Number of 24 bit items to read.

		// Reads in 32 bit data, swapped if necessary.
		long Read32(		// Returns number of 32 bit items read.
			U32*	pu32,		// In:  32 bit data to read (swapping, if necessary).
			long	lNum);	// In:  Number of 32 bit items to read.

		// Reads in 64 bit data, swapped if necessary.
		long Read64(		// Returns number of 64 bit items read.
			U64*	pu64,		// In:  64 bit data to read (swapping, if necessary).
			long	lNum);	// In:  Number of 64 bit items to read.

/////////// Binary Writes ////////////////////////////////////////////////////

		// Writes out 8 bit data, swapped if necessary (BWAH HA).
		long Write8(		// Returns number of 8 bit items written.
			const U8*	pu8,		// In:  8 bit data to write (swapping, if necessary).
			long	lNum);	// In:  Number of 8 bit items to write.

		// Writes out 16 bit data, swapped if necessary.
		long Write16(		// Returns number of 16 bit items written.
			const U16*	pu16,		// In:  16 bit data to write (swapping, if necessary).
			long	lNum);	// In:  Number of 16 bit items to write.

		// Writes out 24 bit data, swapped if necessary.
		long Write24(			// Returns number of 24 bit items written.
			const RPixel24* ppix24,	// In:  24 bit data to write (swapping, if necessary).
			long	lNum);		// In:  Number of 24 bit items to write.

		// Writes out 32 bit data, swapped if necessary.
		long Write32(		// Returns number of 32 bit items written.
			const U32*	pu32,		// In:  32 bit data to write (swapping, if necessary).
			long	lNum);	// In:  Number of 32 bit items to write.

		// Writes out 64 bit data, swapped if necessary.
		long Write64(		// Returns number of 64 bit items written.
			const U64*	pu64,		// In:  64 bit data to write (swapping, if necessary).
			long	lNum);	// In:  Number of 64 bit items to write.

		#ifdef ALLOW_RFILE_REOPEN
			// Disconnects this RFile from the disk temporarily so that another
			// can use the FILE* that is made available.  Returns 0 on success.
			short Disconnect(void);

			// Reconnects a disk file that has been previously disconnected.
			// Does nothing if connected (i.e., if m_sDisconnected == FALSE).
			// Returns 0 on success.
			short Reconnect(void);

			// Disconnect the RFile attached to disk file that was accessed 
			// longest ago.
			// Returns 0 on success.
			static short MakeStreamAvailable(void);
		#endif	// ALLOW_RFILE_REOPEN

//////////////////////////////////////////////////////////////////////////////
		friend class RRes;
//////////////////////////////////////////////////////////////////////////////

	public:	// Member variables.
		FILE*			m_fs;						// Stdio file stream ptr.
		static CritiCall	ms_criticall;	// Called on every read and write with
													// the amount that is about to be
													// processed.

	protected:	// Member variables.
		UCHAR*		m_pucFile;			// Memory file ptr.
		short			m_sOwnMem;			// TRUE, if RFile allocated m_pucFile.
		UCHAR*		m_pucCur;			// Current position in memory file.
		long			m_lSize;				// Size of memory file (in bytes).
		long			m_lGrowSize;		// Amount to grow memfile when buffer 
												// overwritten.
		short			m_sMemError;		// TRUE if memory file access functions
												// caused an error.
		Endian		m_endian;			// Endian type.
		Flags			m_flags;				// Flags.  See comments in Typedefs & 
												// Enums section above.

		RFile*		m_pfileSynch;		// RFile this RFile is synchronized with.
												// Used on Close() to resynch.

		static U8	ms_au8SwapBuf[RFILE_SWAP_SIZE];	// Used to byte swap by Write().

		// Hook stuff.
		long							m_lUser;			// Instantiable hook value.
		static OpenHook			ms_hOpen;		// Hook for calls to Open(char*...).
		static long					ms_lOpenUser;	// User value passed to m_hOpen.
		short							m_sOpenSem;		// Semaphore to block recursion greater
															// than 1.
		static CloseHook			ms_hClose;		// Hook for calls to Close().
		static long					ms_lCloseUser;	// User value passed to m_hClose.
		short							m_sCloseSem;	// Semaphore to block recursion greater
															// than 1.
	#ifdef ALLOW_RFILE_REOPEN
		// Reopen stuff.
		static RList <RFile>	ms_listOpen;				// List of open RFiles.
		long			m_lLastAccess;							// Time of last access.
		char			m_szFlags[MAX_MODE_LEN + 1];		// Last Open's flags.
		short			m_sDisconnected;						// TRUE if file has been 
																	// diskonnected.
		char			m_szFileName[MAX_NAME_LEN + 1];	// Filename for reopening.
	#endif // ALLOW_RFILE_REOPEN
	};


////////////////////////////////////////////////////////////////////////////////
// This template function offers an easy way to use any object's Load()
// function.  Most RSPiX objects have a Load() function that expects an RFile*
// as its only parameter.  This function opens the file, calls the object's
// Load(), and closes the file, all with full error checking.  Note that the
// file mode defaults to "rb" and the endian mode defaults to LittleEndian.
////////////////////////////////////////////////////////////////////////////////
#define	rspEZLoad(a,b) RFileEZLoad(a,b,"rb",RFile::LittleEndian)
#define	rspEZSave(a,b) RFileEZSave(a,b,"wb",RFile::LittleEndian)

#ifdef _DEBUG
	#define RFileEZLoad(a,b,c,d)	RFileEZLoadDebug(__FILE__, __LINE__, a, b, c, d)
#endif

template <class ClassType>
#ifdef _DEBUG
short RFileEZLoadDebug(
	char* FILE_MACRO,
	long LINE_MACRO,
#else
short RFileEZLoad(
#endif
	ClassType* pObject,
	char* pszName,
	char* pszMode,
	RFile::Endian endian)
	{
	short sResult = 0;

	// Create RFile object
	RFile* pFile = new RFile;
	if (pFile != 0)
		{

		// Open the file
		sResult = pFile->Open(pszName, pszMode, endian);
		if (sResult == 0)
			{

			// Call the specified object's Load()
			sResult = pObject->Load(pFile);

			// If no error was returned then check for file I/O errors.  This
			// may be redundant if the object's Load() already checked for file
			// errors, but if it didn't then this becomes very important.
			if ((sResult == 0) && pFile->Error())
				{
				sResult = -1;
				#ifdef _DEBUG
					STRACE("%s(%ld):RFileEZLoad(): File I/O Error!\n", FILE_MACRO, LINE_MACRO);
				#endif
				}

			// Close the file
			pFile->Close();
			}
		else
			{
			sResult = -1;
			#ifdef _DEBUG
				STRACE("%s(%ld):RFileEZLoad(): Couldn't open file '%s'!\n", FILE_MACRO, LINE_MACRO, pszName);
			#endif
			}
		
		// Delete file object
		delete pFile;
		}
	else
		{
		sResult = -1;
		#ifdef _DEBUG
			STRACE("%s(%ld):RFileEZLoad(): Couldn't create RFile object!\n", FILE_MACRO, LINE_MACRO);
		#endif
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// This template function offers an easy way to use any object's Save()
// function.  Most RSPiX objects have a Save() function that expects an RFile*
// as its only parameter.  This function opens the file, calls the object's
// Save(), and closes the file, all with full error checking.  Note that the
// file mode defaults to "wb" and the endian mode defaults to LittleEndian.
////////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
	#define RFileEZSave(a,b,c,d)	RFileEZSaveDebug(__FILE__, __LINE__, a, b, c, d)
#endif

template <class ClassType>
#ifdef _DEBUG
short RFileEZSaveDebug(
	char* FILE_MACRO,
 	long LINE_MACRO,
#else
short RFileEZSave(
#endif
	ClassType* pObject,
	char* pszName, 
	char* pszMode,
	RFile::Endian endian)
	{
	short sResult = 0;

	// Create RFile object
	RFile* pFile = new RFile;
	if (pFile != 0)
		{

		// Open the file
		sResult = pFile->Open(pszName, pszMode, endian);
		if (sResult == 0)
			{

			// Call the specified object's Save()
			sResult = pObject->Save(pFile);

			// If no error was returned then check for file I/O errors.  This
			// may be redundant if the object's Save() already checked for file
			// errors, but if it didn't then this becomes very important.
			if ((sResult == 0) && pFile->Error())
				{
				sResult = -1;
				#ifdef _DEBUG
					STRACE("%s(%ld):RFileEZSave(): File I/O Error!\n", FILE_MACRO, LINE_MACRO);
				#endif
				}

			// Close the file
			pFile->Close();
			}
		else
			{
			sResult = -1;
			#ifdef _DEBUG
				STRACE("%s(%ld):RFileEZSave(): Couldn't open file '%s'!\n", FILE_MACRO, LINE_MACRO, pszName);
			#endif
			}

		delete pFile;
		pFile	= 0;
		}
	else
		{
		sResult = -1;
		#ifdef _DEBUG
			STRACE("%s(%ld):RFileEZSave(): Couldn't create RFile object!\n", FILE_MACRO, LINE_MACRO);
		#endif
		}

	return sResult;
	}


////////// rspAnyLoad() and rspAnySave() /////////////////////////////////////
// There are two types of implementations for these functions.  One is the 
//	explicit overload where someone defines an rspAnyLoad/Save specific to the
// overloaded type and the other is implicit in that there is a template 
// function that catches all the ones that don't have an explicit overload.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Explicit overloads for rspAnyLoad and rspAnySave.
//
// NOTICE: Metrowerks compiler has a problem with these explicit functions
// being created AFTER the templates for them are defined.  The solution is
// simply to define these BEFORE the templates.
//////////////////////////////////////////////////////////////////////////////

// This macro makes it easy to create explicit versions of rspAnyLoad() for
// basic types that are supported by RFile.  Could be enhanced by allowing
// caller to specify how to cast the value before calling RFile::Read().
#define RFILE_INSTANTIATE_ANYLOAD(type)	\
inline																							\
short rspAnyLoad(		/* Returns 0 on success.*/										\
	type*		ptype,	/* Ptr to type to load.*/										\
	RFile*	pfile)	/* Open RFile to load from.  Must have read access.*/	\
	{																								\
	return (pfile->Read(ptype) == 1) ? 0 : 1;											\
	}																								

// This macro makes it easy to create explicit versions of rspAnySave() for
// basic types that are supported by RFile.  Could be enhanced by allowing
// caller to specify how to cast the value before calling RFile::Write().
#define RFILE_INSTANTIATE_ANYSAVE(type)	\
inline																							\
short rspAnySave(		/* Returns 0 on success.*/										\
	type*		ptype,	/* Ptr to float to load.*/										\
	RFile*	pfile)	/* Open RFile to save to.  Must have write access.*/	\
	{																								\
	return (pfile->Write(ptype) == 1) ? 0 : 1;										\
	}																								

RFILE_INSTANTIATE_ANYLOAD(U8)
RFILE_INSTANTIATE_ANYSAVE(U8)
RFILE_INSTANTIATE_ANYLOAD(S8)
RFILE_INSTANTIATE_ANYSAVE(S8)
RFILE_INSTANTIATE_ANYLOAD(U16)
RFILE_INSTANTIATE_ANYSAVE(U16)
RFILE_INSTANTIATE_ANYLOAD(S16)
RFILE_INSTANTIATE_ANYSAVE(S16)
RFILE_INSTANTIATE_ANYLOAD(RPixel24)
RFILE_INSTANTIATE_ANYSAVE(RPixel24)
RFILE_INSTANTIATE_ANYLOAD(U32)
RFILE_INSTANTIATE_ANYSAVE(U32)
RFILE_INSTANTIATE_ANYLOAD(S32)
RFILE_INSTANTIATE_ANYSAVE(S32)
RFILE_INSTANTIATE_ANYLOAD(RPixel32)
RFILE_INSTANTIATE_ANYSAVE(RPixel32)
RFILE_INSTANTIATE_ANYLOAD(float)
RFILE_INSTANTIATE_ANYSAVE(float)
RFILE_INSTANTIATE_ANYLOAD(U64)
RFILE_INSTANTIATE_ANYSAVE(U64)
RFILE_INSTANTIATE_ANYLOAD(S64)
RFILE_INSTANTIATE_ANYSAVE(S64)
RFILE_INSTANTIATE_ANYLOAD(double)
RFILE_INSTANTIATE_ANYSAVE(double)


//////////////////////////////////////////////////////////////////////////////
// Loads any type that does not have a specific overload.  This implicit one
// only works for types that have their own Load(RFile*) member function.
//////////////////////////////////////////////////////////////////////////////
template <class obj>
short rspAnyLoad(		// Returns 0 on success.
	obj*		pobj,		// Ptr to object to load.
	RFile*	pfile)	// Open RFile.  Must have read access.
	{
	return pobj->Load(pfile);
	}

//////////////////////////////////////////////////////////////////////////////
// Saves any type that does not have a specific overload.  This implicit one 
// only works for types that have their own Save(RFile*) member function.
//////////////////////////////////////////////////////////////////////////////
template <class obj>
short rspAnySave(		// Returns 0 on success.
	obj*		pobj,		// Ptr to object to save.
	RFile*	pfile)	// Open RFile.  Must have write access.
	{
	return pobj->Save(pfile);
	}

// Find the correct file. This will favor the prefpath
//  for reading, and always use the prefpath for writing.
//  On systems without a prefpath, it just returns pszName, otherwise
//  it will find the right file name and return it in platform-
//  specific notation.
//  This may return a static buffer, and as such is not thread safe.
extern const char *FindCorrectFile(const char *pszName, const char *pszMode);

#endif // FILE_H
//////////////////////////////////////////////////////////////////////////////
//	EOF
//////////////////////////////////////////////////////////////////////////////
