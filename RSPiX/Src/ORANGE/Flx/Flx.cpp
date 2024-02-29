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
// FLX.CPP
//
// History:
//		08/05/94 MR		Started. (10h)
//
//		08/07/94 MR		Got decompression working (except for SS2) and started
//							on compression. (13h)
//
//		08/08/94 MR		General cleanup of interfaces, etc. (12h)
//
//		08/09/94 MR		Finished up basic compression.
//
//		08/12/94 MR		Added flags to FLX_BUF struct that indicate whether the
//							pixels and/or the colors were modified by a "Read".
//
//		08/15/94 MR		Fixed bug in ReadFrame() -- if the requested frame was
//							the current frame, it was not copied to the specified buf.
//
//							Also added a function that returns the last frame that
//							was written.
//
//	 03/06/96	JMI	Converted references from PORTABLE.H (e.g., DWORD) to
//							references from SYSTEM.H (e.g., ULONG).
//
//
///////////////////////////////////////////////////////////////////////////////
//
// THIS FILE CONTAINS ONLY THE PUBLIC FUNCTIONS.
// THE PRIVATE FUNCTIONS ARE IN FLXP.CPP
//
///////////////////////////////////////////////////////////////////////////////
//
// Offers a clean and simple interface for reading and writing .FLC files.
//
// MEGA WARNING: The memory management being used in this module relies
// completely on the memory model under which it is compiled.  The recommended
// model is large.  However, in large model, flics larger than 320 x 200 are
// not possible since the image data would exceed 64k!  In the 32-bit future,
// this will no longer be a problem.  For now, we're stuck with it.
// Note that using the huge memory model will not help because many of the
// math and the indexes are "short" (16 bits), so the results would not be/
// valid for larger than 320 x 200!
//
///////////////////////////////////////////////////////////////////////////////

#include "common/system.h"

#include "flx/FLX.H"


///////////////////////////////////////////////////////////////////////////////
//
// Default constructor.  If this is used, then Setup() must be called before
// any other function can be called.
//
///////////////////////////////////////////////////////////////////////////////
CFlx::CFlx(void)
	{
	// Set flags to default states
	m_bOpenForRead = FALSE;
	m_bOpenForWrite = FALSE;

	// Init FLX_BUF
	InitBuf(&m_bufPrev);
	InitBuf(&m_bufFirstFrame);
	
	// Clear file header
	ClearHeader();
	}
	
	
///////////////////////////////////////////////////////////////////////////////
//
// Destructor.
//
///////////////////////////////////////////////////////////////////////////////
CFlx::~CFlx()
	{
	// Close in case file was left open
	Close();
	
	// Clear header in case it's illegally accessed after we're destroyed
	ClearHeader();

	// Free any memory that needs freeing
	FreeBuf(&m_bufPrev);
	FreeBuf(&m_bufFirstFrame);
	
	// Clear flags to default values (same reason we cleared header)
	m_bOpenForRead = FALSE;
	m_bOpenForWrite = FALSE;
	}
	
	
///////////////////////////////////////////////////////////////////////////////
//
// Open an existing FLC/FLI file for reading.  You can optionally get a copy of
// the file header and can optionally have your buf memory allocated for you.
// Returns 0 if successfull, non-zero otherwise.
//
// 10/20/94, Paul Lin,	add code to reset error conditions on the fstream object
//						so that the next time this function is called, it doesn't fail
//
///////////////////////////////////////////////////////////////////////////////
short CFlx::Open(
	char* pszFileName,			// Full path and filename of flic file
	short bSimple,					// TRUE for simple mode, FALSE for advanced stuff
	FLX_FILE_HDR* pfilehdr,		// Copy of header returned here if not NULL
	FLX_BUF* pbuf)					// Memory allocated within struct if not NULL
	{
	short sError = 0;
	
	// Close in case it was left open
	Close();

	// Clear file header.  This is done primarily for the older FLI files
	// so that all fields, even those that aren't used by FLI files, will
	// be set to default values.
	ClearHeader();

	// Open file (only if it already exists -- do not create new file!)
	if (m_file.Open(pszFileName, "rb", ENDIAN_LITTLE) == 0)
		{
		// Set file mode to binary (this seems to be necessary even
		// though we specified ios::binary when we opened the stream)
		
		// Read the header.  Regardless of whether it's a FLC or FLI file,
		// the header is returned as if it was a FLC file.
		if (ReadHeader() == 0)
			{
			
			// Restart animation
			Restart();
						
			// Default is to read both pixels and color data from flic
			m_bReadPixels = TRUE;
			m_bReadColors = TRUE;

			// If user wants simple operation, then we need to allocate buffers for the
			// previous frame and the previous color palette.
			m_bSimple = bSimple;
			if (m_bSimple)
				{
				sError = AllocBuf(&m_bufPrev, m_filehdr.sWidth, m_filehdr.sHeight, 256);
				sError = AllocBuf(&m_bufPrev, m_filehdr.sWidth, m_filehdr.sHeight, 256);
				}
			}
		else
			sError = 1;
		}
	else
		sError = 1;
	
	// Final check for file errors
	if ((sError == 0) && m_file.Error() != FALSE)
		sError = 1;
	
	// If pointer to header not NULL, then return copy of header there
	if ((sError == 0) && (pfilehdr != NULL))
		*pfilehdr = m_filehdr;
	
	// If pointer to buf not NULL, then allocate memory
	if ((sError == 0) && (pbuf != NULL))
		sError = CreateBuf(pbuf, m_filehdr.sWidth, m_filehdr.sHeight, 256);
	
	// If no errors, then file is finally marked "open for reading"
	if (sError == 0)
		m_bOpenForRead = TRUE;
		
	// If error, reset the fstream object
	if (sError == 1)
	{
		// clear the ios's error flags
		m_file.ClearError();
	}
	
	return sError;
	}
	
	
///////////////////////////////////////////////////////////////////////////////
//
// Create a new flic file to be written to (the file cannot already exist).
// The newer FLC format is written unless bOldFLI is TRUE, in which case the
// older FLI format is used.  You can optionally get a copy of the file header
// and can optionally have your buf memory allocated for you.
// Returns 0 if successfull, non-zero otherwise.
//
///////////////////////////////////////////////////////////////////////////////
short CFlx::Create(
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
	FLX_BUF* pbuf)					// Memory allocated within struct if not NULL
	{
	short sError = 0;
	
	// Close in case it was left open
	Close();
			
	// Create file.  Depending on what user selected, either allow for the
	// replacement of existing files or don't.
	short	sSux;
	if (bReplaceExisting)
		sSux	= m_file.Open(pszFileName, "wb", ENDIAN_LITTLE);
	else
		{
		// There's no no-replace flag so we must text.
		if (m_file.Open(pszFileName, "rb", ENDIAN_LITTLE) == 0)
			{
			// File exists.
			sSux	= TRUE;
			m_file.Close();
			}
		else
			{
			// File doesn't exist.
			sSux	= m_file.Open(pszFileName, "wb", ENDIAN_LITTLE);
			}
		}

	if (sSux	== FALSE)
		{
		// Set file mode to binary (this seems to be necessary even
		// though we specified ios::binary when we opened the stream)
				
		// Clear header to set reserved fields to default values.
		ClearHeader();
				
		// Fill in as many real values as possible.  Certain values will
		// need to be updated after all the frames have been written.
		m_filehdr.lEntireFileSize = 0;			// Not yet known!
		if (bOldFLI)
			m_filehdr.wMagic = FLX_MAGIC_FLI;	// Old FLI format
		else
			m_filehdr.wMagic = FLX_MAGIC_FLC;	// New FLC format
		m_filehdr.sNumFrames = 0;					// Not yet known!
		m_filehdr.sWidth = sWidth;					// Set width
		m_filehdr.sHeight = sHeight;				// Set height
		m_filehdr.sDepth = 8;						// Always 8 (256 colors)
		m_filehdr.sFlags = 0;						// 0 until header is written
		m_filehdr.lMilliPerFrame = lMilliPerFrame;	// Set timing
		m_filehdr.dCreatedTime = 0;				// Set to 0 for now (too lazy)
		m_filehdr.dCreator = 0x464c4942;			// Set to "FLIB" as per doc's
		m_filehdr.dUpdatedTime = 0;				// Set to 0 for now (too lazy)
		m_filehdr.dUpdater = 0x464c4942;			// Set to "FLIB" as per doc's
		m_filehdr.sAspectX = sAspectX;			// Set aspect x
		m_filehdr.sAspectY = sAspectY;			// Set aspect y
		m_filehdr.lOffsetFrame1 = 128;			// Immediately after header
		m_filehdr.lOffsetFrame2 = 0;				// Not yet known!
	
		// Write out header (but don't mark as complete yet!)
		WriteHeader();
								
		// Reset current frame number
		m_sFrameNum = 0;
		
		// If user wants simple operation, then we need to allocate buffers for the
		// previous frame and the previous color palette.
		m_bSimple = bSimple;
		if (m_bSimple)
			{
			sError = AllocBuf(&m_bufPrev, m_filehdr.sWidth, m_filehdr.sHeight, 256);
			sError = AllocBuf(&m_bufFirstFrame, m_filehdr.sWidth, m_filehdr.sHeight, 256);
			}
		}
	else
		sError = 1;
		
	// Final check for file errors
	if ((sError == 0) && m_file.Error() != FALSE)
		sError = 1;
	
	// If pointer to header not NULL, then return copy of header there
	if ((sError == 0) && (pfilehdr != NULL))
		*pfilehdr = m_filehdr;
	
	// If pointer to buf not NULL, then allocate memory
	if ((sError == 0) && (pbuf != NULL))
		sError = CreateBuf(pbuf, m_filehdr.sWidth, m_filehdr.sHeight, 256);
	
	// If no errors, then file is finally marked "open for writing"
	if (sError == 0)
		m_bOpenForWrite = TRUE;
	
	return sError;
	}
	
	
///////////////////////////////////////////////////////////////////////////////
//
// Close the currently open file (if any).  If a flic was being written to the
// file, this will NOT properly complete the file.  A separate function is
// supplied for that, and it should be called before this function.
// Returns 0 if successfull, non-zero otherwise.
//
///////////////////////////////////////////////////////////////////////////////
short CFlx::Close(FLX_BUF* pbuf)
	{
	short sError = 1;
	
	// Before we close the file, let's write the ring frame!
	if (m_bOpenForWrite)
	{
		WriteFinish(&m_bufFirstFrame, &m_bufPrev);
	}
	
	// If file is open, try to close it.
	if (m_bOpenForRead || m_bOpenForWrite)
		{
		if (m_file.Close() == 0)
			{
			// Clear flags
			m_bOpenForRead = FALSE;
			m_bOpenForWrite = FALSE;
			
			// Free any memory associated with buf
			FreeBuf(&m_bufPrev);
			
			// Successfull
			sError = 0;
			}
		}
	else
		sError = 0;
		
	// let's free the buffer passed in, if valid
	if (pbuf != NULL)
		FreeBuf(pbuf);
		
	return sError;
	}
	
	
///////////////////////////////////////////////////////////////////////////////
//
// Get copy of flic file header (file must have been opened or created).  When
// creating a new file, certain fields are not valid until the file is closed.
// Returns 0 if successfull, non-zero otherwise.
//
///////////////////////////////////////////////////////////////////////////////
short CFlx::GetHeader(FLX_FILE_HDR* pFileHdr)
	{
	short sError = 1;
	
	if (m_bOpenForRead || m_bOpenForWrite)
		{
		// Copy our header struct to user's struct
		*pFileHdr = m_filehdr;
		sError = 0;
		}
	
	return sError;
	}
	
	
///////////////////////////////////////////////////////////////////////////////
//
// Get the current frame number.  When reading, this is the frame that was
// last read.  When writing, this is the frame that was last written.  In both
// cases, a value of 0 indicates that no frames have been read or written.
// Otherwise, the number will be from 1 to n.
//
///////////////////////////////////////////////////////////////////////////////
short CFlx::GetFrameNum(void)
	{
	return m_sFrameNum;
	}
	
	
///////////////////////////////////////////////////////////////////////////////
//
// Read the specified flic frame (1 to n, anything else is an error).  The
// time it takes to get the frame is proportional to the number of frames
// between it and the last frame that was read.  In simple mode, if the same
// frame is requested more than once in a row, that frame is simply returned
// each time.  In non-simple mode, requesting the same frame again requires
// us to restart the animation and work our way up to that frame again.
// Returns 0 if successfull, non-zero otherwise.
//
///////////////////////////////////////////////////////////////////////////////
short CFlx::ReadFrame(
	short sFrameNum,			// Frame number to be read
	FLX_BUF* pbufRead)		// Buffer for frame being read
	{
	short sError = 0;
	
	if (m_bOpenForRead)
		{
		// Make sure valid frame number was specified
		if ((sFrameNum >= 1) && (sFrameNum <= m_filehdr.sNumFrames))
			{
			// If we're in simple mode and the requested frame is the current
			// frame, then we can handle it quickly.
			if (m_bSimple && (sFrameNum == m_sFrameNum))
				{
				// Copy the frame from our buffer to the user buffer.
				// Note that we copy the modified flags, too!
				CopyBuf(pbufRead, &m_bufPrev);
				pbufRead->bPixelsModified = m_bufPrev.bPixelsModified;
				pbufRead->bColorsModified = m_bufPrev.bColorsModified;
				}
			else
				{
				// If specified frame is before (or equal to) the current frame,
				// we need to restart the animation.
				if (sFrameNum <= m_sFrameNum)
					Restart();
				
				// Go frame-by-frame to the requested frame
				while ((m_sFrameNum < sFrameNum) && (sError == 0))
					sError = ReadNextFrame(pbufRead);
				}
			}
		else
			sError = 1;
		}
	else
		sError = 1;
	
	return sError;
	}
	
	
///////////////////////////////////////////////////////////////////////////////
//
// Read the next flic frame (if flic was just opened, this will read frame 1).
// The "modified" flags are updated in the specified FLX_BUF.
// Returns 0 if successfull, non-zero otherwise.
//
///////////////////////////////////////////////////////////////////////////////
short CFlx::ReadNextFrame(
	FLX_BUF* pbufRead)		// Buffer for frame being read
	{
	short sError = 0;

	if (m_bOpenForRead)
		{
		if (m_bSimple)
			{
			// Apply delta to our buf and copy result to user buf.
			// Note that we copy the modified flags, too!
			sError = DoReadFrame(&m_bufPrev);
			if (sError == 0)
				{
				CopyBuf(pbufRead, &m_bufPrev);
				pbufRead->bPixelsModified = m_bufPrev.bPixelsModified;
				pbufRead->bColorsModified = m_bufPrev.bColorsModified;
				}
			}
		else
			{
			// Apply delta directly to user buf
			DoReadFrame(pbufRead);
			}
		}
	else
		sError = 1;

	return sError;
	}
	
	
///////////////////////////////////////////////////////////////////////////////
//
// Write the next flic frame.  This function can only be used if "simple" mode
// was specified to the Create() function (which must be called before this).
// Don't mix calls to this and other frame writing functions.  After the last
// frame has been written, call WriteFinish() and then Close().
// Returns 0 if successfull, non-zero otherwise.
//
///////////////////////////////////////////////////////////////////////////////
short CFlx::WriteNextFrame(
	FLX_BUF* pbufWrite)		// Buffer of frame to be written
	{
	short sError = 0;
	
	// Verify open for writing and simple mode and header not written yet
	if (m_bOpenForWrite && m_bSimple && (m_filehdr.sFlags != 3))
		{
		// In simple mode, we keep a copy of the previous frame so the user
		// doesn't have to.
		if (m_sFrameNum == 0)
		{
			sError = WriteFirstFrame(pbufWrite);
			// Since this is the first frame, copy it to the first frame buffer for ring frame use later.
			CopyBuf(&m_bufFirstFrame, pbufWrite);
		}
		else
			sError = DoWriteFrame(pbufWrite, &m_bufPrev);
		CopyBuf(&m_bufPrev, pbufWrite);
		}
	else
		sError;
		
	return sError;
	}
	
	
///////////////////////////////////////////////////////////////////////////////
//
// Read the specified flic frame (1 to n, anything else is an error).  The
// time it takes to get the frame is proportional to the number of frames
// between it and the last frame that was read.
// Returns 0 if successfull, non-zero otherwise.
//
///////////////////////////////////////////////////////////////////////////////
short CFlx::WriteFirstFrame(
	FLX_BUF* pbufWrite)		// Buffer of frame to be written
	{
	if (m_bOpenForWrite && (m_sFrameNum == 0))
		return DoWriteFrame(pbufWrite, NULL);
	else
		return 1;
	}


///////////////////////////////////////////////////////////////////////////////
//
// Write the next flic frame (actually, write the delta between it and the
// previous frame.  Both Create() and WriteFirstFrame() must be called
// successfully before calling this function for each of the remaining frames.
// Returns 0 if successfull, non-zero otherwise.
//
///////////////////////////////////////////////////////////////////////////////
short CFlx::WriteNextFrame(
	FLX_BUF* pbufWrite,		// Buffer of next frame to be written
	FLX_BUF* pbufPrev)		// Buffer of previously written frame
	{
	// Verify open for writing, min 1 frame written and header not written yet
	if (m_bOpenForWrite && (m_sFrameNum >= 1) && (m_filehdr.sFlags != 3))
		return DoWriteFrame(pbufWrite, pbufPrev);
	else
		return 1;
	}
	
	
///////////////////////////////////////////////////////////////////////////////
//
// Finish writing the flic file.  This must be called after the last frame was
// written but before closing the file.  The first and last frames are required
// in order to generate the "ring" frame (used for looping the animation).
// If you don't want a ring frame, simply specify NULL for both parameters.
// The header is also updated with the final information for the file.
// Returns 0 if successfull, non-zero otherwise.
//
///////////////////////////////////////////////////////////////////////////////
short CFlx::WriteFinish(
	FLX_BUF* pbufFirst,		// Buffer of first frame that was written or NULL
	FLX_BUF* pbufLast)		// Buffer of last frame that was written or NULL
	{
	short sError = 0;
	
	// Verify open for writing, min 1 frame written and header not written yet
	if (m_bOpenForWrite && (m_sFrameNum >= 1) && (m_filehdr.sFlags != 3))
		{
		// Write out the ring frame (delta between the last and the first frames)
		// unless the user doesn't want a ring frame!
		if ((pbufFirst != NULL) && (pbufLast != NULL))
			{
			sError = DoWriteFrame(pbufFirst, pbufLast);
			m_sFrameNum -= 1;		// Ring frame doesn't count!
			}			
		if (sError == 0)
			{
			// Update file header and write it out
			m_filehdr.lEntireFileSize = m_file.Tell();	// Set file size
			m_filehdr.sNumFrames = m_sFrameNum;				// Set total frames
			m_filehdr.sFlags = 3;								// 3 means header is valid
			sError = WriteHeader();
			}
		}
	else
		sError = 1;
		
	return sError;
	}
	

///////////////////////////////////////////////////////////////////////////////
//
// Create a FLX_BUF based on the specified width, height, and number of colors.
// Returns 0 if successfull, non-zero otherwise.
//
///////////////////////////////////////////////////////////////////////////////
short CFlx::CreateBuf(FLX_BUF* pbuf, short sWidth, short sHeight, short sColors)
	{
	InitBuf(pbuf);
	return AllocBuf(pbuf, sWidth, sHeight, sColors);
	}


///////////////////////////////////////////////////////////////////////////////
//
// Destroy a FLX_BUF that was previously created using CreateBuf().
// The FLX_BUF must not be used after this call!
//
///////////////////////////////////////////////////////////////////////////////
void CFlx::DestroyBuf(FLX_BUF* pbuf)
	{
	FreeBuf(pbuf);
	}


///////////////////////////////////////////////////////////////////////////////
// EOF
///////////////////////////////////////////////////////////////////////////////
