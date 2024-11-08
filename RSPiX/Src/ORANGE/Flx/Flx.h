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
	// 128 uint8_ts for both FLC and FLI files, but the usage is somewhat different.
	typedef struct tag_FLX_FILE_HDR
		{
		int32_t lEntireFileSize;		// Size of entire file, including header
		uint16_t wMagic;					// Magic number: FLC = $af12, FLI = $af11
		int16_t sNumFrames;				// Number of frames, not including ring. Max 4000.
		int16_t sWidth;					// Width in pixels (always 320 in FLI)
		int16_t sHeight;					// Height in pixels (always 200 in FLI)
		int16_t sDepth;					// Bits per pixel (always 8)
		uint16_t sFlags;					// FLC: set to 3 if properly written, FLI: always 0
		int32_t lMilliPerFrame;			// FLC: milliseconds between frames (4 uint8_ts)
											// FLI: jiffies (1/70th) between frames (2 uint8_ts)
		// The rest is for FLC files only -- for FLI files, it's all reserved.
		uint16_t sReserveA;				// Reserved -- set to zero
		uint32_t dCreatedTime;			// MS-DOS-formatted date and time of file's creation
		uint32_t dCreator;				// Serial number of Animator Pro program used to
											// create file -- $464c4942 is a good one ("FLIB")
		uint32_t dUpdatedTime;			// MS-DOS-formatted date and time of file's update
		uint32_t dUpdater;				// Serial number of Animator Pro program used to
											// update file -- $464c4942 is a good one ("FLIB")
		int16_t sAspectX;				// X-axis aspect ratio at which file was created
		int16_t sAspectY;				// Y-axis aspect ratio at which file was created
		uint8_t bReservedB[38];			// Reserved -- set to zeroes
		int32_t lOffsetFrame1;			// Offset from beginning of file to first frame chunk
		int32_t lOffsetFrame2;			// Offset from beginning of file to second frame chunk,
											// used when looping from ring back to second frame
		uint8_t bReservedC[40];			// Reserved -- set to zeroes
		} FLX_FILE_HDR;


	// Define struct that describes frame chunk header.
	typedef struct tag_FLX_FRAME_HDR
		{
		int32_t lChunkSize;				// Size of entire frame chunk, including header
											// and all subordinate chunks
		uint16_t wType;						// Frame header chunk id: always 0xF1FA
		int16_t sNumSubChunks;			// Number of subordinate chunks.  0 indicates that
											// this frame is identical to previous frame.
		uint8_t bReserved[8];			// Reserved
		} FLX_FRAME_HDR;


	// Define struct that describes data chunk header.
	typedef struct tag_FLX_DATA_HDR
		{
		int32_t lChunkSize;				// Size of frame data chunk, including header
		uint16_t wType;						// Type of frame data chunk
		// NOTE: The actual data follows these two items, but is not
		// included in this struct because it has a variable size!
		} FLX_DATA_HDR;


	// Define struct that describes RGB color data as used by a FLC/FLI
	typedef struct tag_FLX_RGB
		{
		uint8_t bR;
		uint8_t bG;
		uint8_t bB;
		} FLX_RGB;
#endif	// RAMFLX_H

// Define struct that describes the buffers where a frame's data is stored
typedef struct tag_FLX_BUF
	{
	uint8_t* pbPixels;			// Pointer to memory for pixel data
	int16_t sPitch;				// Pitch to be used for pixel data
	FLX_RGB* prgbColors;		// Pointer to memory for color data (normally 256 colors)
	int16_t bPixelsModified;	// TRUE if pixels were modified (only valid after a "Read")
	int16_t bColorsModified;	// TRUE if colors were modified (only valid after a "Read")
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
		int16_t Open(
			char* pszFileName,			// Full path and filename of flic file
			int16_t bSimple,					// TRUE for simple mode, FALSE for advanced stuff
			FLX_FILE_HDR* pfilehdr,		// Copy of header returned here if not NULL
			FLX_BUF* pbuf);				// Memory allocated within struct if not NULL
		
		// Create a new flic file to be written to (the file cannot already exist).
		// The newer FLC format is written unless bOldFLI is TRUE, in which case the
		// older FLI format is used.  You can optionally get a copy of the file header
		// and can optionally have your buf memory allocated for you.
		// Returns 0 if successfull, non-zero otherwise.
		int16_t Create(
			char* pszFileName,			// Full path and filename of flic file
			int16_t bReplaceExisting,		// TRUE if okay to replace existing file
			int16_t sWidth,					// Width of flic
			int16_t sHeight,					// Height of flic
			int32_t lMilliPerFrame,			// Milliseconds between frames
			int16_t sAspectX,				// X aspect ratio
			int16_t sAspectY,				// Y aspect ratio
			int16_t bOldFLI,					// TRUE for old FLI format, FALSE for new FLC
			int16_t bSimple,					// TRUE for simple mode, FALSE for advanced stuff
			FLX_FILE_HDR* pfilehdr,		// Copy of header returned here if not NULL
			FLX_BUF* pbuf);				// Memory allocated within struct if not NULL

		// Close the currently open file (if any).
		// Returns 0 if successfull, non-zero otherwise.
		// Modified 10/20/94 to accommodate buffer pointer
		int16_t Close(FLX_BUF* pbuf = NULL);

		// Get copy of flic file header (file must have been opened or created).  When
		// creating a new file, certain fields are not valid until the file is closed.
		// Returns 0 if successfull, non-zero otherwise.
		int16_t GetHeader(FLX_FILE_HDR* pHeader);
		
		// Get the current frame number.  When reading, this is the frame that was
		// last read.  When writing, this is the frame that was last written.  In both
		// cases, a value of 0 indicates that no frames have been read or written.
		// Otherwise, the number will be from 1 to n.
		int16_t GetFrameNum(void);
		
		// Read the specified flic frame (1 to n, anything else is an error).  The
		// time it takes to get the frame is proportional to the number of frames
		// between it and the last frame that was read.  In simple mode, if the same
		// frame is requested more than once in a row, that frame is simply returned
		// each time.  In non-simple mode, requesting the same frame again requires
		// us to restart the animation and work our way up to that frame again.
		// Returns 0 if successfull, non-zero otherwise.
		int16_t ReadFrame(
			int16_t sFrameNum,			// Frame number to be read
			FLX_BUF* pbufRead);		// Buffer for frame being read

		// Read the next flic frame (if flic was just opened, this will read frame 1).
		// Returns 0 if successfull, non-zero otherwise.
		int16_t ReadNextFrame(
			FLX_BUF* pbufRead);		// Buffer for frame being read
		
		// Write the next flic frame.  If this function is used at all, it must be used
		// for writing every frame of the flic (do not use the other frame writing
		// functions).  Create() must have been called before calling this function.
		// After the last frame has been written, call WriteFinish() and then Close().
		// Returns 0 if successfull, non-zero otherwise.
		int16_t WriteNextFrame(
			FLX_BUF* pbufWrite);		// Buffer of frame to be written
			
		// Finish writing the flic file.  This must be called after the last frame was
		// written but before closing the file.  The first and last frames are required
		// in order to generate the "ring" frame (used for looping the animation).
		// If you don't want a ring frame, simply specify NULL for both parameters.
		// The header is also updated with the final information for the file.
		// Returns 0 if successfull, non-zero otherwise.
		int16_t WriteFinish(
			FLX_BUF* pbufFirst,		// Buffer of first frame that was written or NULL
			FLX_BUF* pbufLast);		// Buffer of last frame that was written or NULL

		// This is a lower-level function.  You probably don't want to use it.
		// Returns 0 if successfull, non-zero otherwise.
		int16_t WriteFirstFrame(
			FLX_BUF* pbufWrite);		// Buffer of frame to be written
		
		// This is a lower-level function (same name as other function but different
		// parameters!)  You probably don't want to use it.
		int16_t WriteNextFrame(
			FLX_BUF* pbufWrite,		// Buffer of frame to be written
			FLX_BUF* pbufPrev);		// Buffer of previous frame (already written)

		// Create a FLX_BUF based on the specified width, height, and number of colors.
		// Returns 0 if successfull, non-zero otherwise.
		int16_t CreateBuf(FLX_BUF* pbuf, int16_t sWidth, int16_t sHeight, int16_t sColors);
		
		// Destroy a FLX_BUF that was previously created using CreateBuf().
		// The FLX_BUF must not be used after this call!
		void DestroyBuf(FLX_BUF* pbuf);
		
	private:
		void Restart(void);

		int16_t DoReadFrame(FLX_BUF* pbufRead);
		int16_t ReadDataColor(FLX_BUF* pbufRead, int16_t sDataType);
		int16_t ReadDataBlack(FLX_BUF* pbufRead);
		int16_t ReadDataCopy(FLX_BUF* pbufRead);
		int16_t ReadDataBRun(FLX_BUF* pbufRead);
		int16_t ReadDataLC(FLX_BUF* pbufRead);
		int16_t ReadDataSS2(FLX_BUF* pbufRead);

		int16_t DoWriteFrame(FLX_BUF* pbufWrite, FLX_BUF* pbufPrev);
		int16_t WriteColorDelta(FLX_BUF* pbufNext, FLX_BUF* pbufPrev, uint8_t* pBuf, int32_t* plChunkSize);
		int16_t WritePixelDelta(FLX_BUF* pbufNext, FLX_BUF* pbufPrev, uint8_t* pBuf, int32_t* plChunkSize);
		int16_t WriteDataChunk(uint8_t* pData, int32_t lSize, uint16_t wType, int32_t* plChunkSize);
		
		int16_t CompressLineDelta(
			int16_t y,				// Current line to compress, used to calculate offset.
			FLX_BUF* pbufNext,		// Pointer to current flx frame.
			FLX_BUF* pbufPrev,		// Pointer to previous flx frame.
			uint8_t* pbDst,			// Pointer to the chunk storage.
			int32_t& lSize,			// Size of the data used by current compressed line.
			int16_t sAlign,			// 1 = uint8_t oriented, 2 = uint16_t oriented
			int16_t sLineSkipCount = 0);	// Used only for uint16_t oriented delta compression during which
										// the line skip count will be written out to the chunk.

		int32_t CompressBRUN(
			uint8_t* pbIn,				// Pointer to input (pixels to be compressed)
			int16_t sPitch,			// Pitch (distance from one pixel to the pixel below it)
			int16_t sSrcX,			// Starting x of rectangular area to compress
			int16_t sSrcY,			// Starting y of rectangular area to compress
			int16_t sWidth,			// Width of rectangular area to compress
			int16_t sHeight,			// Height of rectangular area to compress
			uint8_t* pbOut);			// Pointer to output (compressed data)
		
		int16_t ReadHeader(void);
		int16_t WriteHeader(void);
		void ClearHeader(void);
		
		void InitBuf(FLX_BUF* pbuf);
		int16_t AllocBuf(FLX_BUF* pbuf, int16_t sWidth, int16_t sHeight, int16_t sColors);
		void FreeBuf(FLX_BUF* pbuf);
		void CopyBuf(FLX_BUF* pbufDst, FLX_BUF* pbufSrc);
		
	private:
		int16_t m_bOpenForRead;				// TRUE if file is open for read
		int16_t m_bOpenForWrite;			// TRUE if file is open for write

		int16_t m_bSimple;					// TRUE for simple mode (simple for the user)
		
		FLX_FILE_HDR m_filehdr;			// File header

		FLX_BUF m_bufPrev;				// Buf for previous frame 
		FLX_BUF m_bufFirstFrame;		// Buf for the first frame
		
		int16_t m_bReadColors;				// Whether or not to read colors from flic
		int16_t m_bReadPixels;				// Whether or not to read pixels from flic
		
		int16_t m_sFrameNum;				// Current frame number, 1 to n, 0 means none
		
		CNFile	m_file;					// File stream (for buffered file I/O)
	};

#endif // FLX_H
