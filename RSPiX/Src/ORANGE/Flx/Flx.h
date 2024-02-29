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
#ifndef FLX_H
#define FLX_H


#include  "file/file.h"


// Define the magic numbers for FLC and FLI files.
#define FLX_MAGIC_FLI			0xAF11
#define FLX_MAGIC_FLC			0xAF12

// Define operation modes
#define FLX_RETURN_DELTAS		0
#define FLX_RETURN_FULL			1

// Define data chunk types
#define FLX_DATA_COLOR256		4
#define FLX_DATA_SS2				7
#define FLX_DATA_COLOR			11
#define FLX_DATA_LC				12
#define FLX_DATA_BLACK			13
#define FLX_DATA_BRUN			15
#define FLX_DATA_COPY			16
#define FLX_DATA_PSTAMP			18


#ifndef RAMFLX_H	// Already defined if RAMFLX.H was included.
	// Define struct that describes everything in a FLC/FLI header.  The header is
	// 128 UCHARs for both FLC and FLI files, but the usage is somewhat different.
	typedef struct tag_FLX_FILE_HDR
		{
		long lEntireFileSize;		// Size of entire file, including header
		USHORT wMagic;					// Magic number: FLC = $af12, FLI = $af11
		short sNumFrames;				// Number of frames, not including ring. Max 4000.
		short sWidth;					// Width in pixels (always 320 in FLI)
		short sHeight;					// Height in pixels (always 200 in FLI)
		short sDepth;					// Bits per pixel (always 8)
		USHORT sFlags;					// FLC: set to 3 if properly written, FLI: always 0
		long lMilliPerFrame;			// FLC: milliseconds between frames (4 UCHARs)
											// FLI: jiffies (1/70th) between frames (2 UCHARs)
		// The rest is for FLC files only -- for FLI files, it's all reserved.
		USHORT sReserveA;				// Reserved -- set to zero
		ULONG dCreatedTime;			// MS-DOS-formatted date and time of file's creation
		ULONG dCreator;				// Serial number of Animator Pro program used to
											// create file -- $464c4942 is a good one ("FLIB")
		ULONG dUpdatedTime;			// MS-DOS-formatted date and time of file's update
		ULONG dUpdater;				// Serial number of Animator Pro program used to
											// update file -- $464c4942 is a good one ("FLIB")
		short sAspectX;				// X-axis aspect ratio at which file was created
		short sAspectY;				// Y-axis aspect ratio at which file was created
		UCHAR bReservedB[38];			// Reserved -- set to zeroes
		long lOffsetFrame1;			// Offset from beginning of file to first frame chunk
		long lOffsetFrame2;			// Offset from beginning of file to second frame chunk,
											// used when looping from ring back to second frame
		UCHAR bReservedC[40];			// Reserved -- set to zeroes
		} FLX_FILE_HDR;


	// Define struct that describes frame chunk header.
	typedef struct tag_FLX_FRAME_HDR
		{
		long lChunkSize;				// Size of entire frame chunk, including header
											// and all subordinate chunks
		USHORT wType;						// Frame header chunk id: always 0xF1FA
		short sNumSubChunks;			// Number of subordinate chunks.  0 indicates that
											// this frame is identical to previous frame.
		UCHAR bReserved[8];			// Reserved
		} FLX_FRAME_HDR;


	// Define struct that describes data chunk header.
	typedef struct tag_FLX_DATA_HDR
		{
		long lChunkSize;				// Size of frame data chunk, including header
		USHORT wType;						// Type of frame data chunk
		// NOTE: The actual data follows these two items, but is not
		// included in this struct because it has a variable size!
		} FLX_DATA_HDR;


	// Define struct that describes RGB color data as used by a FLC/FLI
	typedef struct tag_FLX_RGB
		{
		UCHAR bR;
		UCHAR bG;
		UCHAR bB;
		} FLX_RGB;
#endif	// RAMFLX_H

// Define struct that describes the buffers where a frame's data is stored
typedef struct tag_FLX_BUF
	{
	UCHAR* pbPixels;			// Pointer to memory for pixel data
	short sPitch;				// Pitch to be used for pixel data
	FLX_RGB* prgbColors;		// Pointer to memory for color data (normally 256 colors)
	short bPixelsModified;	// TRUE if pixels were modified (only valid after a "Read")
	short bColorsModified;	// TRUE if colors were modified (only valid after a "Read")
	} FLX_BUF;
	
	
// Define class
class CFlx
	{
	public:
		// Default constructor.  If this is used, then Setup() must be called before
		// any other function can be called.
		CFlx(void);

		// Destructor.
		~CFlx();
		
		// Open an existing FLC/FLI file for reading.  You can optionally get a copy of
		// the file header and can optionally have your buf memory allocated for you.
		// Returns 0 if successfull, non-zero otherwise.
		short Open(
			char* pszFileName,			// Full path and filename of flic file
			short bSimple,					// TRUE for simple mode, FALSE for advanced stuff
			FLX_FILE_HDR* pfilehdr,		// Copy of header returned here if not NULL
			FLX_BUF* pbuf);				// Memory allocated within struct if not NULL
		
		// Create a new flic file to be written to (the file cannot already exist).
		// The newer FLC format is written unless bOldFLI is TRUE, in which case the
		// older FLI format is used.  You can optionally get a copy of the file header
		// and can optionally have your buf memory allocated for you.
		// Returns 0 if successfull, non-zero otherwise.
		short Create(
			char* pszFileName,			// Full path and filename of flic file
			short bReplaceExisting,		// TRUE if okay to replace existing file
			short sWidth,					// Width of flic
			short sHeight,					// Height of flic
			long lMilliPerFrame,			// Milliseconds between frames
			short sAspectX,				// X aspect ratio
			short sAspectY,				// Y aspect ratio
			short bOldFLI,					// TRUE for old FLI format, FALSE for new FLC
			short bSimple,					// TRUE for simple mode, FALSE for advanced stuff
			FLX_FILE_HDR* pfilehdr,		// Copy of header returned here if not NULL
			FLX_BUF* pbuf);				// Memory allocated within struct if not NULL

		// Close the currently open file (if any).
		// Returns 0 if successfull, non-zero otherwise.
		// Modified 10/20/94 to accommodate buffer pointer
		short Close(FLX_BUF* pbuf = NULL);

		// Get copy of flic file header (file must have been opened or created).  When
		// creating a new file, certain fields are not valid until the file is closed.
		// Returns 0 if successfull, non-zero otherwise.
		short GetHeader(FLX_FILE_HDR* pHeader);
		
		// Get the current frame number.  When reading, this is the frame that was
		// last read.  When writing, this is the frame that was last written.  In both
		// cases, a value of 0 indicates that no frames have been read or written.
		// Otherwise, the number will be from 1 to n.
		short GetFrameNum(void);
		
		// Read the specified flic frame (1 to n, anything else is an error).  The
		// time it takes to get the frame is proportional to the number of frames
		// between it and the last frame that was read.  In simple mode, if the same
		// frame is requested more than once in a row, that frame is simply returned
		// each time.  In non-simple mode, requesting the same frame again requires
		// us to restart the animation and work our way up to that frame again.
		// Returns 0 if successfull, non-zero otherwise.
		short ReadFrame(
			short sFrameNum,			// Frame number to be read
			FLX_BUF* pbufRead);		// Buffer for frame being read

		// Read the next flic frame (if flic was just opened, this will read frame 1).
		// Returns 0 if successfull, non-zero otherwise.
		short ReadNextFrame(
			FLX_BUF* pbufRead);		// Buffer for frame being read
		
		// Write the next flic frame.  If this function is used at all, it must be used
		// for writing every frame of the flic (do not use the other frame writing
		// functions).  Create() must have been called before calling this function.
		// After the last frame has been written, call WriteFinish() and then Close().
		// Returns 0 if successfull, non-zero otherwise.
		short WriteNextFrame(
			FLX_BUF* pbufWrite);		// Buffer of frame to be written
			
		// Finish writing the flic file.  This must be called after the last frame was
		// written but before closing the file.  The first and last frames are required
		// in order to generate the "ring" frame (used for looping the animation).
		// If you don't want a ring frame, simply specify NULL for both parameters.
		// The header is also updated with the final information for the file.
		// Returns 0 if successfull, non-zero otherwise.
		short WriteFinish(
			FLX_BUF* pbufFirst,		// Buffer of first frame that was written or NULL
			FLX_BUF* pbufLast);		// Buffer of last frame that was written or NULL

		// This is a lower-level function.  You probably don't want to use it.
		// Returns 0 if successfull, non-zero otherwise.
		short WriteFirstFrame(
			FLX_BUF* pbufWrite);		// Buffer of frame to be written
		
		// This is a lower-level function (same name as other function but different
		// parameters!)  You probably don't want to use it.
		short WriteNextFrame(
			FLX_BUF* pbufWrite,		// Buffer of frame to be written
			FLX_BUF* pbufPrev);		// Buffer of previous frame (already written)

		// Create a FLX_BUF based on the specified width, height, and number of colors.
		// Returns 0 if successfull, non-zero otherwise.
		short CreateBuf(FLX_BUF* pbuf, short sWidth, short sHeight, short sColors);
		
		// Destroy a FLX_BUF that was previously created using CreateBuf().
		// The FLX_BUF must not be used after this call!
		void DestroyBuf(FLX_BUF* pbuf);
		
	private:
		void Restart(void);

		short DoReadFrame(FLX_BUF* pbufRead);
		short ReadDataColor(FLX_BUF* pbufRead, short sDataType);
		short ReadDataBlack(FLX_BUF* pbufRead);
		short ReadDataCopy(FLX_BUF* pbufRead);
		short ReadDataBRun(FLX_BUF* pbufRead);
		short ReadDataLC(FLX_BUF* pbufRead);
		short ReadDataSS2(FLX_BUF* pbufRead);

		short DoWriteFrame(FLX_BUF* pbufWrite, FLX_BUF* pbufPrev);
		short WriteColorDelta(FLX_BUF* pbufNext, FLX_BUF* pbufPrev, UCHAR* pBuf, long* plChunkSize);
		short WritePixelDelta(FLX_BUF* pbufNext, FLX_BUF* pbufPrev, UCHAR* pBuf, long* plChunkSize);
		short WriteDataChunk(UCHAR* pData, long lSize, USHORT wType, long* plChunkSize);
		
		short CompressLineDelta(
			short y,				// Current line to compress, used to calculate offset.
			FLX_BUF* pbufNext,		// Pointer to current flx frame.
			FLX_BUF* pbufPrev,		// Pointer to previous flx frame.
			UCHAR* pbDst,			// Pointer to the chunk storage.
			long& lSize,			// Size of the data used by current compressed line.
			short sAlign,			// 1 = UCHAR oriented, 2 = USHORT oriented
			short sLineSkipCount = 0);	// Used only for USHORT oriented delta compression during which
										// the line skip count will be written out to the chunk.

		long CompressBRUN(
			UCHAR* pbIn,				// Pointer to input (pixels to be compressed)
			short sPitch,			// Pitch (distance from one pixel to the pixel below it)
			short sSrcX,			// Starting x of rectangular area to compress
			short sSrcY,			// Starting y of rectangular area to compress
			short sWidth,			// Width of rectangular area to compress
			short sHeight,			// Height of rectangular area to compress
			UCHAR* pbOut);			// Pointer to output (compressed data)
		
		short ReadHeader(void);
		short WriteHeader(void);
		void ClearHeader(void);
		
		void InitBuf(FLX_BUF* pbuf);
		short AllocBuf(FLX_BUF* pbuf, short sWidth, short sHeight, short sColors);
		void FreeBuf(FLX_BUF* pbuf);
		void CopyBuf(FLX_BUF* pbufDst, FLX_BUF* pbufSrc);
		
	private:
		short m_bOpenForRead;				// TRUE if file is open for read
		short m_bOpenForWrite;			// TRUE if file is open for write

		short m_bSimple;					// TRUE for simple mode (simple for the user)
		
		FLX_FILE_HDR m_filehdr;			// File header

		FLX_BUF m_bufPrev;				// Buf for previous frame 
		FLX_BUF m_bufFirstFrame;		// Buf for the first frame
		
		short m_bReadColors;				// Whether or not to read colors from flic
		short m_bReadPixels;				// Whether or not to read pixels from flic
		
		short m_sFrameNum;				// Current frame number, 1 to n, 0 means none
		
		CNFile	m_file;					// File stream (for buffered file I/O)
	};

#endif // FLX_H
