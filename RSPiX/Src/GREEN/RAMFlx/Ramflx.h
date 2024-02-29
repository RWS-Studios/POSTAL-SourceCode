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
#ifndef RAMFLX_H
#define RAMFLX_H

#include "image/image.h"
#include "file/file.h"

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

// This is the value in the reserveA field of the header
// when a flic with no delta compression is read.
#define FLX_RSP_NODELTA			1

#ifndef FLX_H	// Already defined if FLX.H was included.
	// Define struct that describes everything in a FLC/FLI header.  The header is
	// 128 bytes for both FLC and FLI files, but the usage is somewhat different.
	typedef struct tag_FLX_FILE_HDR
		{
		long lEntireFileSize;		// Size of entire file, including header
		USHORT wMagic;					// Magic number: FLC = $af12, FLI = $af11
		short sNumFrames;				// Number of frames, not including ring. Max 4000.
		short sWidth;					// Width in pixels (always 320 in FLI)
		short sHeight;					// Height in pixels (always 200 in FLI)
		short sDepth;					// Bits per pixel (always 8)
		USHORT sFlags;					// FLC: set to 3 if properly written, FLI: always 0
		long lMilliPerFrame;			// FLC: milliseconds between frames (4 bytes)
											// FLI: jiffies (1/70th) between frames (2 bytes)
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
		UCHAR bReservedB[38];		// Reserved -- set to zeroes
		long lOffsetFrame1;			// Offset from beginning of file to first frame chunk
		long lOffsetFrame2;			// Offset from beginning of file to second frame chunk,
											// used when looping from ring back to second frame
		UCHAR bReservedC[40];		// Reserved -- set to zeroes
		} FLX_FILE_HDR;


	// Define struct that describes frame chunk header.
	typedef struct tag_FLX_FRAME_HDR
		{
		long lChunkSize;				// Size of entire frame chunk, including header
											// and all subordinate chunks
		USHORT wType;					// Frame header chunk id: always 0xF1FA
		short sNumSubChunks;			// Number of subordinate chunks.  0 indicates that
											// this frame is identical to previous frame.
		UCHAR bReserved[8];			// Reserved
		} FLX_FRAME_HDR;


	// Define struct that describes data chunk header.
	typedef struct tag_FLX_DATA_HDR
		{
		long lChunkSize;				// Size of frame data chunk, including header
		USHORT wType;					// Type of frame data chunk
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
#endif	// FLX_H

// Define class
class CRamFlx
	{
	public:
		// Default constructor.  If this is used, then Setup() must be called before
		// any other function can be called.
		CRamFlx(void);

		// Destructor.
		~CRamFlx();
		
		// Open an existing FLC/FLI file for reading.  You can optionally get a copy of
		// the file header and can optionally have your buf memory allocated for you.
		// Returns 0 if successfull, non-zero otherwise.
		short Open(
			char* pszFileName,			// Full path and filename of flic file
			FLX_FILE_HDR* pfilehdr,		// Copy of header returned here if not NULL
			CImage* pbuf);				// Memory allocated within struct if not NULL
		
		// Close the currently open file (if any).
		// Returns 0 if successfull, non-zero otherwise.
		// Modified 10/20/94 to accommodate buffer pointer
		short Close(CImage* pbuf = NULL);

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
			CImage* pbufRead);		// Buffer for frame being read

		// Read the next flic frame (if flic was just opened, this will read frame 1).
		// Returns 0 if successfull, non-zero otherwise.
		short ReadNextFrame(
			CImage* pbufRead);		// Buffer for frame being read

		// Return the current buffer state	
		CImage*	GetBuffer(void)
			{ return &m_imagePrev; };
		
		// Create a CImage based on the specified width, height, and number of colors.
		// Returns 0 if successfull, non-zero otherwise.
		short CreateBuf(CImage* pimage, long lWidth, long lHeight, short sColors);
		
		// Destroy a CImage that was previously created using CreateBuf().
		// The CImage must not be used after this call!
		void DestroyBuf(CImage* pimage); 
      
		// The m_bPixelsModified is set when a frame is read into the buffer
      	// and pixels in this frame have been modified from the previous screen
		short IsPixelsModified(void)
			{ return m_sPixelsModified; };

      	// The m_bColorsModified is set when a frame is read into the buffer
      	// and palette in this frame is different from the previous screen
		short IsColorsModified(void)
			{ return m_sColorsModified; };
			
		// You can provide your own file (currently pointing to the beginning of
		// a FLX frame) to allow this module to decompress your data.  Oh happy,
		// happy day.
		// Returns 0 on success.
		static short DoReadFrame(CImage* pbufRead, CNFile* pfile,
										short* psPixelsModified, short* psColorsModified);
	private:
		void Restart(void);

		short	DoReadFrame(CImage* pbufRead);

		// Work horse functions.
		static short ReadDataColor(CImage* pbufRead, short sDataType, CNFile* pfile,
										short* psColorsModified);
		static short ReadDataBlack(CImage* pbufRead, short* psPixelsModified);
		static short ReadDataCopy(CImage* pbufRead, CNFile* pfile, short* psPixelsModified);
		static short ReadDataBRun(CImage* pbufRead, CNFile* pfile, short* psPixelsModified);
		static short ReadDataLC(CImage* pbufRead, CNFile* pfile, short* psPixelsModified);
		static short ReadDataSS2(CImage* pbufRead, CNFile* pfile, short* psPixelsModified);

		short ReadHeader(CNFile* pfile);
		void  ClearHeader(void);
		
		void  InitBuf(CImage* pbuf);
		short AllocBuf(CImage* pbuf, long lWidth, long lHeight, short sColors);
		void  FreeBuf(CImage* pbuf);
		void  CopyBuf(CImage* pbufDst, CImage* pbufSrc);

		short CreateFramePointers(void);
		
	private:
		short				m_sOpenForRead;	// TRUE if file is open for read

		FLX_FILE_HDR	m_filehdr;			// File header

		CImage			m_imagePrev;		// Buffer for previous frame 

		short				m_sNoDelta;			// Flag to signal no delta compression						
		
		UCHAR*			m_pucFlxBuf;		// RAM buffer for flic frames from .FLC file

		long*				m_plFrames;			// Indexices of frames in flics
													// with no delta compression

		short				m_sReadColors;		// Whether or not to read colors from flic
		short				m_sReadPixels;		// Whether or not to read pixels from flic
		
		short				m_sPixelsModified;// Whether the pixels were modified
		short				m_sColorsModified;// Whether the colors were modified 

		short				m_sFrameNum;		// Current frame number, 1 to n, 0 means none
		
		CNFile			m_file;				// File stream (for buffered file I/O)
	};

#endif // RAMFLX_H
