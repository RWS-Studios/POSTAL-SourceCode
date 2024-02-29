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
// RAMFLX.CPP
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
//	  	05/02/95 JW		Added functionality for RAM buffer to store flic
//
//		05/03/95 JW		Added ability to play uncompressed flics without having
//							to seek to the first frame.
//
//		11/03/95	JMI	Converted to newest CImage and CNFile.
//
//		03/04/96	JMI	ReadFrame now remembers whether the m_sColorsModified and
//							m_sPixelsModified flags were set at all during a read that
//							required parsing many frames and sets them to TRUE in such
//							a case.
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

#include <string.h>
#include <malloc.h>

#include <SMRTHEAP.HPP>

#include "common/system.h"
#include "ramflx/ramflx.h"


///////////////////////////////////////////////////////////////////////////////
//
// Default constructor.  If this is used, then Setup() must be called before
// any other function can be called.
//
///////////////////////////////////////////////////////////////////////////////
CRamFlx::CRamFlx(void)
	{
	// Set flags to default states
	m_sOpenForRead = FALSE;

	// Init CImage
	InitBuf(&m_imagePrev);
	
	// Clear file header
	ClearHeader();
	}
	
	
///////////////////////////////////////////////////////////////////////////////
//
// Destructor.
//
///////////////////////////////////////////////////////////////////////////////
CRamFlx::~CRamFlx()
	{
	// Close in case file was left open
	Close();
	
	// Clear header in case it's illegally accessed after we're destroyed
	ClearHeader();

	// Free any memory that needs freeing
	FreeBuf(&m_imagePrev);
	
	// Clear flags to default values (same reason we cleared header)
	m_sOpenForRead = FALSE;
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
short CRamFlx::Open(
			char* pszFileName,		// Full path and filename of flic file
			FLX_FILE_HDR* pfilehdr,	// Copy of header returned here if not NULL
			CImage* pimage)			// Memory allocated within struct if not NULL
	{
	short sError = 0;
	
	// Close in case it was left open
	Close();

	// Clear file header.  This is done primarily for the older FLI files
	// so that all fields, even those that aren't used by FLI files, will
	// be set to default values.
	ClearHeader();

	CNFile	file;
	// Open file (only if it already exists -- do not create new file!)
	if (file.Open(pszFileName, "rb", ENDIAN_LITTLE) == 0)
		{
		// Read the header.  Regardless of whether it's a FLC or FLI file,
		// the header is returned as if it was a FLC file.
		if (ReadHeader(&file) == 0)
			{
			// Restart animation
			Restart();
						
			// Default is to read both pixels and color data from flic
			m_sReadPixels = TRUE;
			m_sReadColors = TRUE;

			// Is this a flic with no delta compression?
			if (m_filehdr.sReserveA == FLX_RSP_NODELTA)
				{
				// Create list of pointers to frames in the flic(no delta compression)
				sError = CreateFramePointers();
				// Make sure the memory file pointer resets to the first frame
				Restart();
				// Flic has no delta compression
				m_sNoDelta = TRUE;
				}
			else
				{
				// Flic has delta compression
				m_sNoDelta = FALSE;
				}


			// If user doesn' specify a buffer, then we need to allocate buffers for the
			// previous frame and the previous color palette.
			if (pimage == NULL)
				{
				sError = AllocBuf(&m_imagePrev, (long)m_filehdr.sWidth, (long)m_filehdr.sHeight, 256);
				}
			}
		else
			sError = 1;
		
		// Close the file, the flic is loaded into the buffer
		file.Close();
		}
	else
		sError = 1;
	
	// Final check for file errors
	if ((sError == 0) && m_file.Error() == TRUE)
		sError = 1;

	// If pointer to header not NULL, then return copy of header there
	if ((sError == 0) && (pfilehdr != NULL))
		*pfilehdr = m_filehdr;
	
	// If pointer to buf not NULL, then allocate memory
	if ((sError == 0) && (pimage != NULL))
		sError = CreateBuf(pimage, (long)m_filehdr.sWidth, (long)m_filehdr.sHeight, 256);
	
	// If no errors, then file is finally marked "open for reading"
	if (sError == 0)
		m_sOpenForRead = TRUE;
		
	
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
short CRamFlx::Close(CImage* pimage)
	{
	short sError = 1;
	
	// If file is open, try to close it.
	if (m_sOpenForRead)
		{
		// Clear flags
		m_sOpenForRead = FALSE;
		
		// Free any memory associated with image buf, flic buf, and frame pointers
		FreeBuf(&m_imagePrev);

		if (m_pucFlxBuf != NULL)
			{
			m_file.Close();
			free(m_pucFlxBuf);
			}
		
		if (m_sNoDelta == TRUE)
			{
			free(m_plFrames);
			}
			
		// Successfull
		sError = 0;
		}
	else
		sError = 0;
		
	// let's free the buffer passed in, if valid
	if (pimage != NULL)
		FreeBuf(pimage);
		
	return sError;
	}
	
	
///////////////////////////////////////////////////////////////////////////////
//
// Get copy of flic file header (file must have been opened or created).  When
// creating a new file, certain fields are not valid until the file is closed.
// Returns 0 if successfull, non-zero otherwise.
//
///////////////////////////////////////////////////////////////////////////////
short CRamFlx::GetHeader(FLX_FILE_HDR* pFileHdr)
	{
	short sError = 1;
	
	if (m_sOpenForRead)
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
short CRamFlx::GetFrameNum(void)
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
short CRamFlx::ReadFrame(
	short sFrameNum,			// Frame number to be read
	CImage* pimageRead)		// Buffer for frame being read
	{
	short sError = 0;
	
	if (m_sOpenForRead && 
	    (sFrameNum >= 1) && 
	    (sFrameNum <= m_filehdr.sNumFrames))
		{
		if (m_sNoDelta == TRUE)
			{
			// Flics with no delta compression
			if (sFrameNum != m_sFrameNum)
				{
				// Set position in buffer for frame
				m_file.Seek(m_plFrames[sFrameNum], SEEK_SET);
				// Decompress packets into image buffer
				sError = ReadNextFrame(pimageRead);
				// Set the frame number to the correct frame currently in image buffer
				m_sFrameNum = sFrameNum;
				}
     		}
		else
			{
			if (sFrameNum != m_sFrameNum)
				{
				short	sColorsModified	= FALSE;
				short	sPixelsModified	= FALSE;

				// If specified frame is before (or equal to) the current frame,
				// we need to restart the animation.
				if (sFrameNum <= m_sFrameNum)
					{
					Restart();
					// When restarting, we should set these flags for lack
					// of a way of knowing for sure.
					sColorsModified	= TRUE;
					sPixelsModified	= TRUE;
					}
			
				// Go frame-by-frame to the requested frame
				while ((m_sFrameNum < sFrameNum) && (sError == 0))
					{
					sError = ReadNextFrame(pimageRead);
					// If the colors are modified . . .
					if (m_sColorsModified != FALSE)
						{
						// We need to store this info since the next
						// frame may set m_sColorsModified to FALSE.
						sColorsModified	= TRUE;
						}

					// If the pixels are modified . . .
					if (m_sPixelsModified != FALSE)
						{
						// We need to store this info since the next
						// frame may set m_sPixelsModified to FALSE.
						sPixelsModified	= TRUE;
						}
					}

				// If flags were set at all during the loop, 
				// they need to be set now.
				m_sColorsModified	= sColorsModified;
				m_sPixelsModified	= sPixelsModified;
				}
			}
		}
	else
		sError = 1;
	
	return sError;
	}
	
	
///////////////////////////////////////////////////////////////////////////////
//
// Read the next flic frame (if flic was just opened, this will read frame 1).
// The "modified" flags are updated in the specified CImage.
// Returns 0 if successfull, non-zero otherwise.
//
///////////////////////////////////////////////////////////////////////////////
short CRamFlx::ReadNextFrame(
	CImage* pimageRead)		// Buffer for frame being read
	{
	short sError = 0;

	if (m_sOpenForRead)
		{
		if (pimageRead == NULL)
			{
			// Apply delta to our buf
			DoReadFrame(&m_imagePrev);
			}
		else
			{
			// Apply delta directly to user buf
			DoReadFrame(pimageRead);
			}
		}
	else
		sError = 1;

	return sError;
	}
	
	
///////////////////////////////////////////////////////////////////////////////
//
// Create a CImage based on the specified width, height, and number of colors.
// Returns 0 if successfull, non-zero otherwise.
//
///////////////////////////////////////////////////////////////////////////////
short CRamFlx::CreateBuf(CImage* pimage, long lWidth, long lHeight, short sColors)
	{
	InitBuf(pimage);
	return AllocBuf(pimage, lWidth, lHeight, sColors);
	}


///////////////////////////////////////////////////////////////////////////////
//
// Destroy a CImage that was previously created using CreateBuf().
// The CImage must not be used after this call!
//
///////////////////////////////////////////////////////////////////////////////
void CRamFlx::DestroyBuf(CImage* pimage)
	{
	FreeBuf(pimage);
	}


///////////////////////////////////////////////////////////////////////////////
// EOF
///////////////////////////////////////////////////////////////////////////////
