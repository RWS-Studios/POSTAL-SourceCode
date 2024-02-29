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
// FLXP.CPP
//
//	??/??/??	???	Created.
//
// 03/06/96	JMI	Converted references from PORTABLE.H (e.g., DWORD) to
//						references from SYSTEM.H (e.g., ULONG).
//
///////////////////////////////////////////////////////////////////////////////
//
// THIS FILE CONTAINS ONLY THE PRIVATE FUNCTIONS.
// THE PUBLIC FUNCTIONS ARE IN FLX.CPP
// ALL MAJOR COMMENT BLOCKS ARE IN FLX.CPP AS WELL.
//
///////////////////////////////////////////////////////////////////////////////

#include <malloc.h>
#include <memory.h>

#include <SMRTHEAP.HPP>

#include "common/system.H"

#include "flx/FLX.H"


///////////////////////////////////////////////////////////////////////////////
//
// Helper function that restarts at beginning of the flic.
//
///////////////////////////////////////////////////////////////////////////////
void CFlx::Restart(void)
	{
	// Seek to file position of frame 1
	m_file.Seek(m_filehdr.lOffsetFrame1, SEEK_SET);
	
	// Set frame number to 0 to indicate that nothing's been read yet.
	m_sFrameNum = 0;
	}
	
	
///////////////////////////////////////////////////////////////////////////////
//
// Read the next flic frame (if flic was just opened, this will read frame 1).
// The "modified" flags are updated in the specified FLX_BUF.
// Returns 0 if successfull, non-zero otherwise.
//
///////////////////////////////////////////////////////////////////////////////
short CFlx::DoReadFrame(FLX_BUF* pbufRead)
	{
	short sError = 0;

	// Always clear modified flags to FALSE.  The lower-level functions will
	// set the appropriate flag to TRUE as necessary.
	pbufRead->bPixelsModified = FALSE;
	pbufRead->bColorsModified = FALSE;
	
	// Get current file position.  After each chunk, we add the chunk size
	// to this position to get to the next chunk.  We must do that seek
	// instead of relying on the amount of data that was read from the
	// chunk because that amount may be less than the indicated chunk size!
	// (This is not clearly documented, but was discovered the hard way!)
	long lFramePos = m_file.Tell();
			
	// Read frame chunk header
	FLX_FRAME_HDR framehdr;
	m_file.Read(&framehdr.lChunkSize);
	m_file.Read(&framehdr.wType);
	m_file.Read(&framehdr.sNumSubChunks);
	m_file.Read(framehdr.bReserved, 8);
		
	// Verify that it worked and it's the type we're expecting
	if (m_file.Error() == FALSE && framehdr.wType == 0xF1FA)
		{
		//cout << "Frame #" << m_sFrameNum << " has " << framehdr.sNumSubChunks << " data chunks" << endl;
	
		// Go through each of the sub-chunks.  If there are no sub-chunks, then
		// frame is identical to the previous frame and there's nothing to do.
		FLX_DATA_HDR datahdr;
		for (short sSub = 0; sSub < framehdr.sNumSubChunks && sError == 0; sSub++)
			{
			// Get current file position.  After each chunk, we add the chunk size
			// to this position to get to the next chunk.  We must do that seek
			// instead of relying on the amount of data that was read from the
			// chunk because that amount may be less than the indicated chunk size!
			// (This is not clearly documented, but was discovered the hard way!)
			long lDataPos = m_file.Tell();
					
			// Read data chunk header
			m_file.Read(&datahdr.lChunkSize);
			m_file.Read(&datahdr.wType);
			if (m_file.Error() == FALSE)
				{
				// Size of actual data is chunk size minus header size (6)
				long lDataSize = datahdr.lChunkSize - 6;
						
				// Call the appropriate function based on data type
				switch(datahdr.wType)
					{
					case FLX_DATA_COLOR256:
						//cout << "   DATA_COLOR256 of size " << lDataSize << endl;
						if (pbufRead->prgbColors != NULL)
							sError = ReadDataColor(pbufRead, FLX_DATA_COLOR256);
						break;
								
					case FLX_DATA_SS2:
						//cout << "   DATA_SS2 of size " << lDataSize << endl;
						if (pbufRead->pbPixels != NULL)
							sError = ReadDataSS2(pbufRead);
						break;
								
					case FLX_DATA_COLOR:
						//cout << "   DATA_COLOR of size " << lDataSize << endl;
						if (pbufRead->prgbColors != NULL)
							sError = ReadDataColor(pbufRead, FLX_DATA_COLOR);
						break;
								
					case FLX_DATA_LC:
						//cout << "   DATA_LC of size " << lDataSize << endl;
						if (pbufRead->pbPixels != NULL)
							sError = ReadDataLC(pbufRead);
						break;
								
					case FLX_DATA_BLACK:
						//cout << "   DATA_BLACK of size " << lDataSize << endl;
						if (pbufRead->pbPixels != NULL)
							sError = ReadDataBlack(pbufRead);
						break;
								
					case FLX_DATA_BRUN:
						//cout << "   DATA_BRUN of size " << lDataSize << endl;
						if (pbufRead->pbPixels != NULL)
							sError = ReadDataBRun(pbufRead);
						break;
								
					case FLX_DATA_COPY:
						//cout << "   DATA_COPY of size " << lDataSize << endl;
						if (pbufRead->pbPixels != NULL)
							sError = ReadDataCopy(pbufRead);
						break;
								
					case FLX_DATA_PSTAMP:
						//cout << "   DATA_PSTAMP of size " << lDataSize << endl;
						// We always ignore postage stamp data for now.
						break;
	
					default:
						//cout << "   DATA UNKNOWN!!!! of size " << lDataSize << endl;
						//comment out the assert 10/20/94 to prevent crash
						//assert(0);	// Should never get here!
						sError = 1;
						break;
					}
							
				// Adjust file position based on specified chunk size.
				m_file.Seek(lDataPos + datahdr.lChunkSize, SEEK_SET);
				}
			else
				sError = 1;
			}
					
		// Adjust file position based on specified chunk size.
		m_file.Seek(lFramePos + framehdr.lChunkSize, SEEK_SET);
		}
	else
		sError = 1;
	
	// If everything went fine, update the frame number.
	if (sError == 0)
		{
		// If frame number reaches NumFrames+1, then we just did the "ring"
		// frame, which is the delta between the flic's last and first frames.
		if (++m_sFrameNum == (m_filehdr.sNumFrames + 1))
			{
			// Reset frame number
			m_sFrameNum = 1;
				
			// Seek to file position of frame 2 (the next one we'll do)
			m_file.Seek(m_filehdr.lOffsetFrame2, SEEK_SET);
			}
		}
		
	return sError;
	}
	
	
///////////////////////////////////////////////////////////////////////////////
//
// Handler for data chunks containing color information (FLX_DATA_COLOR256 and
// FLX_DATA_COLOR.)
//
// The first word of data specifies the number of "packets" that follow.
// Each packet consists of a byte that specifies the number of colors to
// skip, a byte that specifies the number of colors to do, and three bytes
// of RGB data for each of the colors to do.
//
// The idea is that the color palette index starts out at 0.  For each
// packet, we add the number of colors to skip to the color palette index.
// We then get the number of colors to do and, for each one, we read the
// 3 byte RGB data and copy it to the next color in the palette.  This is
// repeated for each packet.
//
// NOTE: The Autodesk doc's don't mention this, but if the number of colors
// to do is 0, it really means 256!  This was discovered the hard way.
//
// As an example, to change palette colors 2, 7, 8 and 9, the following data
// would be used:
//
//				2										; word specifies 2 packets
//				2, 1, r,g,b							; skip 2, do 1
//				4, 3, r,g,b, r,g,b, r,g,b		; skip 4, do 3
//
// The only difference between the two color-oriented data types are that
// for FLX_DATA_COLOR256, the RGB values range from 0 to 255, while for
// FLX_DATA_COLOR, they range from 0 to 63.  This is an older format, so
// we convert them to the newer format by shifting them left 2 bits.
//
///////////////////////////////////////////////////////////////////////////////
short CFlx::ReadDataColor(FLX_BUF* pbufRead, short sDataType)
	{
	//assert(pbufRead->prgbColors != NULL);
	// instead of assert, just return error
	if (pbufRead->prgbColors == NULL)
		return 1;

	short sError = 0;
	
	// Read number of packets
	short sNumPackets;
	m_file.Read(&sNumPackets);

	// Start the color index at 0 and then process each of the packets
	short sColorIndex = 0;
	short sCnt;
	short sColorDo;
	UCHAR bColorSkip;
	UCHAR bVal;
	
	for (short sPack = 0; (sPack < sNumPackets) && (sError == 0); sPack++)
		{
		// Read number of colors to skip and add to color index
		m_file.Read(&bColorSkip);
		sColorIndex = sColorIndex + (short)bColorSkip;
		
		// Read number of colors to do.  This determines how many sets
		// of r,g,b data (3 bytes) will follow.  If this count is 0, it
		// must be interpreted as a count of 256!
		m_file.Read(&bVal);
		if (bVal != 0)
			sColorDo = (short)bVal;
		else
			sColorDo = (short)256;

		// Make sure we won't index past end of palette!  This would only
		// happen if we were getting bogus data from the file.
		if ((sColorIndex + sColorDo) <= 256)
			{
			// Read the specified number of RGB values (3 bytes each) into the
			// proper color entry(s) in the palette
			m_file.Read(&(pbufRead->prgbColors[sColorIndex]), 3 * sColorDo);
			if (sDataType == FLX_DATA_COLOR256)
				{
				sColorIndex = sColorIndex + sColorDo;
				}
			else
				{
				// For FLX_DATA_COLOR, the RGB values range from 0 to 63, and must
				// be shifted left 2 bits to bring them up to 0 to 255.
				for (sCnt = 0; sCnt < sColorDo; sCnt++)
					{
					pbufRead->prgbColors[sColorIndex].bR <<= 2;
					pbufRead->prgbColors[sColorIndex].bG <<= 2;
					pbufRead->prgbColors[sColorIndex].bB <<= 2;
					sColorIndex++;
					}
				}
			}
		else
			sError = 1;
		}

	// Set modified flag
	pbufRead->bColorsModified = TRUE;

	// If not good, then flag error
	if (m_file.Error() != FALSE)
		sError = 1;
		
	return sError;
	}
	
	
///////////////////////////////////////////////////////////////////////////////
//
// Handler for data chunks of type FLX_DATA_BLACK.
//
// These chunks contain no data.  They are essentially a command that tells
// us to clear all the pixels to color index 0.
//
///////////////////////////////////////////////////////////////////////////////
short CFlx::ReadDataBlack(FLX_BUF* pbufRead)
	{
	//assert(pbufRead->pbPixels != NULL);
	//assert(pbufRead->sPitch > 0);
	// let's just return with error instead of asserting and crashing
	if ((pbufRead->pbPixels == NULL) || (pbufRead->sPitch <= 0))
		return 1;

	// Clear the image to 0 one row at a time.  Note that we use the pitch
	// to move from the start of on row to the start of the next row.
	UCHAR* pbMem = pbufRead->pbPixels;
	for (short y = 0; y < m_filehdr.sHeight; y++)
		{
		memset(pbMem, 0, m_filehdr.sWidth);
		pbMem += (ULONG)pbufRead->sPitch;
		}
	
	// Set modified flag
	pbufRead->bPixelsModified = TRUE;

	// There can be no error!
	return 0;
	}
	
	
///////////////////////////////////////////////////////////////////////////////
//
// Handler for data chunks of type FLX_DATA_COPY.
//
// These chunks contain an uncompressed image of the frame.  The number of
// bytes following the chunk is the animation's width times its height.
// The data goes from left to right and then top to bottom.
//
// These chunks occur rarely, being used only if the compressed frame would
// take up more room than the uncompressed frame!
//
///////////////////////////////////////////////////////////////////////////////
short CFlx::ReadDataCopy(FLX_BUF* pbufRead)
	{
	//assert(pbufRead->pbPixels != NULL);
	//assert(pbufRead->sPitch > 0);
	// let's just return with error instead of asserting
	if ((pbufRead->pbPixels == NULL) || (pbufRead->sPitch <= 0))
		return 1;

	short sError = 0;
	
	// Read in the image one row at a time.  Note that we use the pitch
	// to move from the start of on row to the start of the next row.
	UCHAR* pbMem = pbufRead->pbPixels;
	for (short y = 0; y < m_filehdr.sHeight; y++)
		{
		m_file.Read(pbMem, m_filehdr.sWidth);
		pbMem += (ULONG)pbufRead->sPitch;
		}

	// Set modified flag
	pbufRead->bPixelsModified = TRUE;

	// If not good, then flag error
	if (m_file.Error() != FALSE)
		sError = 1;
		
	return sError;
	}
	
	
///////////////////////////////////////////////////////////////////////////////
//
// Handler for data chunks of type FLX_DATA_BRUN.
//
// These chunks contain the entire image in compressed form.  This is usually
// used for the first frame or within a postage stamp image chunk.
//
// The data is organized into lines, starting at the top of the image and
// moving down.  The number of lines is based on the height in the flic header.
//
// The data for each line starts with a byte that contains the number of
// packets for that line.  This is a holdover from the original Autodesk
// Animator which only supported a width of 320, so it didn't need more than
// 255 packets.  Animator Pro, which supports much larger widths, may need
// more than 255 packets, so it can't use a byte.  The officially sanctioned
// way to deal with this byte is to ignore it, and to instead use the width
// (from the flic header) to determine when each line is done (simply keep
// count of the number of pixels that have been decompressed for that line,
// and when it reaches the width, the line is done).
//
// Each packet contains a count byte followed by one or more pixels.  If the
// count is negative (bit 7 = 1) then its absolute value is the number of pixels
// that follow it.  If the count is positive (bit 7 = 0) then a single pixel
// follows it and that pixel is to be replicated that number of times.
//
///////////////////////////////////////////////////////////////////////////////
short CFlx::ReadDataBRun(FLX_BUF* pbufRead)
	{
	//assert(pbufRead->pbPixels != NULL);
	//assert(pbufRead->sPitch > 0);
	// let's just return with error instead of asserting
	if ((pbufRead->pbPixels == NULL) || (pbufRead->sPitch <= 0))
		return 1;

	// added 10/20/94 to trap errors and exit! instead of asserting
	short sError = 0;

	UCHAR bVal;
	S8		cVal;
	short sCount;
	short x;
	short y;
	UCHAR* pbRow;
	UCHAR* pbPix;
	
	// Decompress image one row at a time.  Note that we use the pitch
	// to move from the start of one row to the start of the next row.
	pbRow = pbufRead->pbPixels;
	for (y = 0; (y < m_filehdr.sHeight) && (sError == 0); y++)
		{
		// First byte is number of packets, which can be ignored (Animator used
		// it, but Animator Pro, which supports width > 320, does not.)
		m_file.Read(&cVal);
		
		// Keep processing packets until we reach the width
		pbPix = pbRow;
		x = 0;
		while ((x < m_filehdr.sWidth) && (sError == 0))
			{
			// First byte of each packet is type/size.  If bit 7 is 1, bits 6-0
			// are number of pixels to be copied.  If bit 7 is 0, bits 6-0 are
			// the number of times to replicate a single pixel.
			m_file.Read(&cVal);
			//assert(cVal != 0);	// Not sure how to handle 0!
			if (cVal != 0)
				{
				sCount = (short)cVal;
				if (sCount < 0)
					{
					sCount = -sCount;
					x += sCount;
					m_file.Read(pbPix, sCount);
					pbPix += (ULONG)sCount;
					}
				else
					{
					x += sCount;
					m_file.Read(&bVal);
					memset(pbPix, (int)bVal, (size_t)sCount);
					pbPix += (ULONG)sCount;
					}
				}
			else
				{
					sError = 1;
				}
			}
			
		pbRow += (ULONG)pbufRead->sPitch;
		}
		
	// just return if error has been set
	if (sError == 1)
		return sError;
	
	// Set modified flag
	pbufRead->bPixelsModified = TRUE;

	if (m_file.Error() == FALSE)
		return 0;
	else
		return 1;
	}
	
	
///////////////////////////////////////////////////////////////////////////////
//
// Handler for data chunks of type FLX_DATA_LC.
//
// These chunks contain the differences between the previous frame and this
// frame.  These are the most common types of pixel data chunks in the older
// FLI files written by the original Autodesk animator.  This is no longer used
// by Animator Pro, but they may appear in an Animator Pro file if Animator Pro
// reads an older file and modifies only some of its frames.
//
// The first word (16 bits) specifies the y coordinate of the first line that
// was different from the previous image.  This value can range from 0 to
// height - 1.
//
// The second word (16 bits) specifies the number of lines that are represented
// in this data chunk.
//
// The following data is organized into lines, starting at the specified y
// coordinate and moving down.
//
// The data for each line starts with a byte that contains the number of
// packets for that line.  (Note: Unlike the BRUN compression, this packet
// count cannot be ignored because there's no other way to know how many pixels
// will be updated on each line.)
//
// Each packet starts with a byte that indicates how many pixels to move to the
// right.  At the start of each line, the position is assumed to be at the
// first (left-most) pixel on that line.  This skip count is added to that
// position to move to the first pixel that will be changed by this packet.
// This process continues across the line, with each packet adding on to the
// position that the previous packet ended up on.  For instance, if the first
// packet specified a skip of 8 and then copied 3 pixels to the screen, then
// second packet would start at 11 and would add its skip count to that.
//
// The skip byte is followed by a count byte which is followed by one or more
// pixels.  If the count is positive (bit 7 = 0) then that is the number of
// pixels that follow it.  If the count is negative (bit 7 = 1) then a single
// pixel follows it and the count's absolute value specifies how often that
// pixel is to be replicated.  (Note: The positive/negative nature of the
// count is reversed from the BTUN compression!)
//
///////////////////////////////////////////////////////////////////////////////
short CFlx::ReadDataLC(FLX_BUF* pbufRead)
	{
	//assert(pbufRead->pbPixels != NULL);
	//assert(pbufRead->sPitch > 0);
	// just return with error instead of asserting
	if ((pbufRead->pbPixels == NULL) || (pbufRead->sPitch <= 0))
		return 1;

	UCHAR	bVal;
	S8		cVal;
	short sCount;
	short y;
	short lines;
	short packets;
	UCHAR* pbRow;
	UCHAR* pbPix;

	// The first word specifies the starting y (another way of looking at it
	// is the number of lines that are unchanged from the previous image).
	m_file.Read(&y);

	// Init row pointer to point at start of specified row
	pbRow = pbufRead->pbPixels + ((ULONG)y * (ULONG)pbufRead->sPitch);

	// The second word specifies the number of lines in this chunk.
	m_file.Read(&lines);
	
	// Let's check to see if the pixels are modified from the previous frame by
	// checking the number of delta lines.  If the number of delta lines is zero, then
	// we know that there is no delta.
	if (lines < 1)
		{
		// Set to unmodified.
		pbufRead->bPixelsModified = FALSE;
		}
	else
		{
		// Set modified flag
		pbufRead->bPixelsModified = TRUE;
		}
	
	// If all's well...
	if (m_file.Error() == FALSE)
		{

		while (lines > 0)
			{
			// Set pixel pointer to start of row
			pbPix = pbRow;
			
// For debugging, prefetch a bunch of values to view them in the debugger
#if 0
	long lPos = m_file.Tell();
	static UCHAR bData[100];

	m_file.Read(bData, sizeof(bData));
	m_file.Seek(lPos, SEEK_SET);
#endif

			// The first byte for each line is the number of packets.
			// This can be 0, which indicates no changes on that line.
			m_file.Read(&bVal);
			packets = (short)bVal;
			
			while (packets > 0)
				{
				// The first byte of each packet is a column skip.
				// Adjust pixel pointer to skip that number of pixels.
				m_file.Read(&bVal);
				pbPix = pbPix + (ULONG)bVal;
	  			
				// Second byte of each packet is type/size.  If bit 7 is 0, bits 6-0
				// are number of pixels to be copied.  If bit 7 is 1, bits 6-0 are
				// the number of times to replicate a single pixel.
				m_file.Read(&cVal);
//				assert(cVal != 0);	// Not sure how to handle 0, so stop if it comes up!
				if (cVal == 0)
					cVal = 0;
					
				sCount = (short)cVal;
				if (sCount > 0)
					{
					m_file.Read(pbPix, sCount);
					pbPix += (ULONG)sCount;
					}
				else
					{
					sCount = -sCount;
					m_file.Read(&bVal);
					memset(pbPix, (int)bVal, (size_t)sCount);
					pbPix += (ULONG)sCount;
					}
					
				// Adjust remaining packets
				packets--;
				}

			// Move row pointer to start of next row
			pbRow += (ULONG)pbufRead->sPitch;
			
			// Adjust remaining lines
			lines--;
			}
		}

	if (m_file.Error() == FALSE)
		return 0;
	else
		return 1;
	}
	
	
///////////////////////////////////////////////////////////////////////////////
//
// Handler for data chunks of type FLX_DATA_SS2.
//
// These chunks contain the differences between the previous frame and this
// frame.  They are similar to FLX_DATA_LC, but are word oriented instead of
// byte oriented.
//
// The first word for "each line can begin with some optional words
// that are used to skip lines and set the last byte in the line for
// animations with odd widths."  The next word will be the number of
// packets.  The first byte of each packet is a column skip.
// Second byte of each packet is type/size.  If bit 7 is 0, bits 6-0
// are number of pixel pairs to be copied.  If bit 7 is 1, bits 6-0 are
// the number of times to replicate a single pixel pair.
//
///////////////////////////////////////////////////////////////////////////////
short CFlx::ReadDataSS2(FLX_BUF* pbufRead)
	{
	//assert(pbufRead->pbPixels != NULL);
	//assert(pbufRead->sPitch > 0);
	// just return with error instead of asserting
	if ((pbufRead->pbPixels == NULL) || (pbufRead->sPitch <= 0))
		return 1;
	
	UCHAR	bVal;
	S8		cVal;
	USHORT wVal;
	short sCount;
	short y;
	short lines;
	short packets;
	UCHAR* pbPix;
	UCHAR	byLastByte;
	short	bLastByte = FALSE;

	// The first word specifies the starting y (another way of looking at it
	// is the number of lines that are unchanged from the previous image).

	// The first word specifies the number of lines in this chunk.
	m_file.Read(&lines);
	
	// Let's check to see if any actual delta is being processed and set the 
	// pixels modified flag accordingly.
	if (lines < 1)
		{
		// Make sure the modified flag is FALSE.
		pbufRead->bPixelsModified = FALSE;
		}
	else
		{
		// Set modified flag to true.
		pbufRead->bPixelsModified = TRUE;
		}
	
	// If all's well...
	if (m_file.Error() == FALSE)
		{                         

		// Start at line 0
		y = 0;
			
		while (lines > 0)
			{
// For debugging, prefetch a bunch of values to view them in the debugger
#if 0
	long lPos = m_file.Tell();
	static UCHAR bData[100];
	m_file.Read(bData, sizeof(bData));
	m_file.Seek(lPos, SEEK_SET);
#endif
			// The first word for "each line can begin with some optional words
			// that are used to skip lines and set the last byte in the line for
			// animations with odd widths."
			do
				{
				m_file.Read(&wVal);
				
				// "The high order two bits of the word is used to determine the
				// contents of the word."
				switch (wVal & 0xC000)
					{
					// 0 0 The word contains the packet count; the packets follow 
					// this word.  This is our signal to stop processing "optional
					// words".
					case 0x0000:
						break;
					
					// 1 0 "The low order byte is to be stored in the last bye of
					// the current line.  The packet count always folows this word."
					// This is another signal to stop processing "optional words".
					case 0x8000:
						//assert(bLastByte != TRUE); // We should not have already set the "last byte".
						// if this error condition occurs, let's just break out of everything and return error
						// this should not cause any problems with the stack since return should clear it
						if (bLastByte == TRUE)
							return 1;
							
						byLastByte = (UCHAR)(wVal & (USHORT)0x00ff);
						bLastByte = TRUE;
						// Read the packet count.
						m_file.Read(&wVal);
						break;
						
					// 1 1 "The word contains a line skip count.  The number of
					// lines skipped is given by the absolute value of the word.
					// This is NOT a signal to stop processing "optional words".
					case 0xC000:
						// Skip abs(wVal) lines
						y += -((short)wVal);
						break;
					}
				} while ((wVal & 0xC000) == 0xC000);

			// The packet count should now be in wVal.
			packets = (short)wVal;
						
			// Init pointer to point at start of specified row
			pbPix = pbufRead->pbPixels + ((ULONG)y * (ULONG)pbufRead->sPitch);
			
			while (packets > 0)
				{
				// The first byte of each packet is a column skip.
				// Adjust pixel pointer to skip that number of pixels.
				m_file.Read(&bVal);
				pbPix = pbPix + (ULONG)bVal;
				
				// Second byte of each packet is type/size.  If bit 7 is 0, bits 6-0
				// are number of pixel pairs to be copied.  If bit 7 is 1, bits 6-0 are
				// the number of times to replicate a single pixel pair.
				m_file.Read(&cVal);
//				assert(cVal != 0);	// Not sure how to handle 0, so stop if it comes up!
#if 0 // this seems to do nothing, so I'm removing it -- Jon 9/28/94
				if (cVal == 0)
					cVal = 0;
#endif
					
				sCount = (short)cVal;
				if (sCount > 0)
					{
					sCount *= sizeof(USHORT);
					m_file.Read(pbPix, sCount);
					pbPix += (ULONG)(sCount);
					}
				else
					{
					sCount = (short)-sCount;
					m_file.Read(&wVal);
//					memset(pbPix, (int)wVal, (size_t)sCount);
					USHORT* pwPix = (USHORT*)pbPix;
					for (short i = 0; i < sCount; i++)
						*pwPix++ = wVal;
					pbPix = (UCHAR*)pwPix;
					}
					
				// Adjust remaining packets
				packets--;
				}
			
			// Place last byte if specified.
			if (bLastByte == TRUE)
				{
				// Get pointer to end of this row.
				pbPix = pbufRead->pbPixels + (((ULONG)y + 1L) * (ULONG)pbufRead->sPitch) - 1L;
				// Set pixel at end of row.
				*pbPix = byLastByte;
				bLastByte = FALSE;
				}

			// Adjust remaining lines
			lines--;
			
			// Go to next line
			y++;
			}
		}

	if (m_file.Error() == FALSE)
		return 0;
	else
		return 1;
	}
	
	
///////////////////////////////////////////////////////////////////////////////
//
// Helper function that actually writes the delta between two frames, or
// just writes the frame if its the first one.
// Since this is an "internal" function, we can relax our validation tests.
// Always returns with file position at next byte after frame that was written.
// Returns 0 if successfull, non-zero otherwise.
//
///////////////////////////////////////////////////////////////////////////////
short CFlx::DoWriteFrame(FLX_BUF* pbufWrite, FLX_BUF* pbufPrev)
	{
	long lDataChunkSize;

	short sError = 0;

	// Update the frame number
	m_sFrameNum++;
		
	// Get current file position.  We will seek back to this position to
	// update the frame chunk's header after its contents have been written.
	long lFramePos = m_file.Tell();
		
	// Set frame chunk header fields to initial values.
	FLX_FRAME_HDR framehdr;
	framehdr.lChunkSize = 16L;				// Start with size of this header
	framehdr.wType = 0xF1FA;				// Frames are always this type
	framehdr.sNumSubChunks = 0;			// No sub-chunks yet
	memset(framehdr.bReserved, 0, 8);	// Zero reserved bytes
		
	// Write temporary frame chunk header
	m_file.Write(&framehdr.lChunkSize);
	m_file.Write(&framehdr.wType);
	m_file.Write(&framehdr.sNumSubChunks);
	m_file.Write(framehdr.bReserved, 8);
	
	// Allocate buffer into which delta data can be written.  Minumum size
	// is width times height plus an extra margin.
	// WARNING: This will only support up to 64k!!!
//	double dSize = (double)m_filehdr.sWidth * (double)m_filehdr.sHeight * (double)1.5;
	UCHAR* pBuf = (UCHAR*)malloc((USHORT)m_filehdr.sWidth * (USHORT)m_filehdr.sHeight);
	if (pBuf != NULL)
		{
		
		// Generate color palette delta.  For first frame, pbufPrev will be
		// NULL, so all the colors will be considered as being changed.
		sError = WriteColorDelta(pbufWrite, pbufPrev, pBuf, &lDataChunkSize);
		if ((sError == 0) && (lDataChunkSize > 0))
			{
			// Update total frame chunk size and number of sub-chunks
			framehdr.lChunkSize += lDataChunkSize;
			framehdr.sNumSubChunks++;
			}
		
		// Generate pixel delta
		sError = WritePixelDelta(pbufWrite, pbufPrev, pBuf, &lDataChunkSize);
		if ((sError == 0) && (lDataChunkSize > 0))
			{
			// Update total frame chunk size and number of sub-chunks
			framehdr.lChunkSize += lDataChunkSize;
			framehdr.sNumSubChunks++;
			}
		
		// Seek back to start of frame header, write the updated version,
		// and then seek to position after end of frame chunk.
		m_file.Seek(lFramePos, SEEK_SET);
		m_file.Write(&framehdr.lChunkSize);
		m_file.Write(&framehdr.wType);
		m_file.Write(&framehdr.sNumSubChunks);
		m_file.Write(framehdr.bReserved, sizeof(framehdr.bReserved));
		m_file.Seek(lFramePos + framehdr.lChunkSize, SEEK_SET);
			
		// If we just did frame 1, then the current file position is the start
		// of frame 2, which is saved in the flic file header.
		if (m_sFrameNum == 1)
			m_filehdr.lOffsetFrame2 = m_file.Tell();
		
		// Print diagnostic info about frame
		//cout << "Frame #" << m_sFrameNum << " has " << framehdr.sNumSubChunks << " data chunks" << endl;
	
		// Free the buffer
		free(pBuf);
		
		}
	else
		sError = 1;		// Out of memory
		
	return sError;
	}
	
	
///////////////////////////////////////////////////////////////////////////////
//
// Generate a color delta data chunk based on the differences between the
// two specified palettes.  If there is no difference, then no data is written.
// If pBufPrev is NULL, then we assume all the colors changed.
//
///////////////////////////////////////////////////////////////////////////////
short CFlx::WriteColorDelta(FLX_BUF* pbufNext, FLX_BUF* pbufPrev, UCHAR* pBuf, long* plChunkSize)
	{
	short sError = 0;

	// Always default to chunk size of 0 in case no data is written
	*plChunkSize = 0;

	// Set up data chunk type based on FLC -vs- FLI.
	USHORT wType;
	if (m_filehdr.wMagic == FLX_MAGIC_FLC)
		wType = FLX_DATA_COLOR256;
	else
		wType = FLX_DATA_COLOR;
	
	// Set up easier-to-use pointers to colors
	FLX_RGB* pNext = pbufNext->prgbColors;
	FLX_RGB* pPrev;
	if (pbufPrev != NULL)
		pPrev = pbufPrev->prgbColors;

	// Set up copy of pointer to buffer than can be moved around
	UCHAR* pOut = pBuf;
	
	// First word of output is number of packets, which starts out at 0.
	USHORT* pwPackets = (USHORT*)pOut;
	pOut = pOut + 2;
	*pwPackets = 0;
	
	// Keep track of size of output data (includes the word above)
	long lSize = 2;
	
	// Keep looping through colors, generating packets that describe the delta
	// between the two palettes.  Each packet tells how many colors were the
	// same, how many changed, and what they changed to (r,g,b).
	short sSame;
	short sChanged;
	short sIndex;
	short sStart = 0;
	while (sStart < 256)
		{
		// Count number of colors that are the same (stop on first changed color)
		if (pbufPrev != NULL)
			{
			for (sSame = 0; (sStart + sSame) < 256; sSame++)
				{
				if ((pNext[sStart + sSame].bR != pPrev[sStart + sSame].bR) ||
					 (pNext[sStart + sSame].bG != pPrev[sStart + sSame].bG) ||
					 (pNext[sStart + sSame].bB != pPrev[sStart + sSame].bB))
					break;
				}
			}
		else
			sSame = 0;
			
		// Adjust start to skip over the colors that were the same
		sStart += sSame;
		
		// Count number of colors that have changed (stop on first same color)
		// If there are no previous colors, then we say that they all changed
		if (pbufPrev != NULL)
			{
			for (sChanged = 0; (sStart + sChanged) < 256; sChanged++)
				{
				if ((pNext[sStart + sChanged].bR == pPrev[sStart + sChanged].bR) &&
					 (pNext[sStart + sChanged].bG == pPrev[sStart + sChanged].bG) &&
					 (pNext[sStart + sChanged].bB == pPrev[sStart + sChanged].bB))
					break;
				}
			}
		else
			sChanged = 256;
		
		// If any colors changed, we generate a new packet
		if (sChanged > 0)
			{
			// Adjust number of packets
			(*pwPackets)++;
			
			// Write number of colors to skip and number to be changed.
			// Note that 256 is written as a 0 since it must fit in a byte!
			*pOut++ = (UCHAR)sSame;
			*pOut++ = (UCHAR)sChanged;
			lSize += 2;
			
			// Write out the r,g,b values for the colors that changed
			for (sIndex = 0; sIndex < sChanged; sIndex++)
				{
				// For FLX_DATA_COLOR256, colors use the full 0-255 range
				if (wType == FLX_DATA_COLOR256)
					{
					*pOut++ = pNext[sStart + sIndex].bR;
					*pOut++ = pNext[sStart + sIndex].bG;
					*pOut++ = pNext[sStart + sIndex].bB;
					}
				// For FLX_DATA_COLOR, colors use only a 0-63 range
				else
					{
					*pOut++ = (UCHAR)(pNext[sStart + sIndex].bR >> 2) & (UCHAR)0x3f;
					*pOut++ = (UCHAR)(pNext[sStart + sIndex].bG >> 2) & (UCHAR)0x3f;
					*pOut++ = (UCHAR)(pNext[sStart + sIndex].bB >> 2) & (UCHAR)0x3f;
					}
				lSize += 3;
				}
			}
		
		// Adjust start to skip over the colors that were different
		sStart += sChanged;
		}
	
	// If any packets were generated, then we need to write out the data
	if (*pwPackets > 0)
		{
		// Write out the data chunk (size is returned into plChunkSize!)
		sError = WriteDataChunk(pBuf, lSize, wType, plChunkSize);
		}

	return sError;
	}
	
	
///////////////////////////////////////////////////////////////////////////////
//
// Generate a color delta data chunk based on the differences between the
// two specified palettes.  If there is no difference, then no data is written.
//
// This is an outline of the logic that should be used based on the doc's:
// 1. If image is all zeroes then generate FLX_DATA_BLACK chunk
// 2. Compress using FLX_DATA_BRUN or BLX_DATA_SS2 (FLC) or FLX_DATA_LC (FLI)
// 3. If compressed size < original size then write compressed chunk.
// 4. If compressed size > original size then write FLX_DATA_COPY chunk.
//
// Instead, we simply always write out a FLX_DATA_BRUN chunk.  This was done
// only because of time constraints.  In the future, the "real" logic should
// be implemented.
//
// 10/20/94, Paul Lin,	Modified this routine to write both the FLX_DATA_SS2 and
//						FLX_DATA_LC formats; either format will automatically be
//						selected depending on the file format (FLI/FLC) and whether
//						the current frame written is the first frame or any other
//						frame.
//
///////////////////////////////////////////////////////////////////////////////
short CFlx::WritePixelDelta(FLX_BUF* pbufNext, FLX_BUF* pbufPrev, UCHAR* pBuf, long* plChunkSize)
{
	short sError = 0;

	// Always default to chunk size of 0 in case no data is written
	*plChunkSize = 0;

#if 0
	// Copy image one row at a time into buffer.
	UCHAR* pbSrc = pbufNext->pbPixels;
	UCHAR* pbDst = pBuf;
	for (short y = 0; y < m_filehdr.sHeight; y++)
		{
		memcpy(pbDst, pbSrc, m_filehdr.sWidth);	// Copy source to buffer
		pbSrc += (ULONG)pbufNext->sPitch;		// Use pitch on source
		pbDst += (ULONG)m_filehdr.sWidth;		// Use width on buffer
		}
	long lSize = (long)m_filehdr.sWidth * (long)m_filehdr.sHeight;

	// Write out the chunk
	sError = WriteDataChunk(pBuf, lSize, FLX_DATA_COPY, plChunkSize);
#endif

#if 0
	long lSize = CompressBRUN(pbufNext->pbPixels, pbufNext->sPitch,
		0, 0, m_filehdr.sWidth, m_filehdr.sHeight, pBuf);
		
	// Write out the chunk
	sError = WriteDataChunk(pBuf, lSize, FLX_DATA_BRUN, plChunkSize);
	
	return sError;

#endif
	
	if (pbufNext == NULL)
		return 1;

	// find out whether we are dealing with the first frame or not
	if (pbufPrev == NULL)
	{
		// since the previous frame buffer has not been defined, let's assume that this is the first frame
		// let's first do the BRUN compression and see what is the size returned
		long lSizeBRUN = CompressBRUN(pbufNext->pbPixels, pbufNext->sPitch,
								  0, 0, m_filehdr.sWidth, m_filehdr.sHeight, pBuf);
		long lSizeCOPY = (long)m_filehdr.sWidth * (long)m_filehdr.sHeight;
		
		if (lSizeBRUN <= lSizeCOPY)
		{
			// the size attained by BRUN compress is smaller,
			// let's write out the chunk
			sError = WriteDataChunk(pBuf, lSizeBRUN, FLX_DATA_BRUN, plChunkSize);
		}
		else
		{
			// the size attained by BRUN is actually larger, must be a complicated image
			// let's use the FLX_DATA_COPY format instead!
			
			// Copy image one row at a time into buffer.
			UCHAR* pbSrc = pbufNext->pbPixels;
			UCHAR* pbDst = pBuf;
			for (short y = 0; y < m_filehdr.sHeight; y++)
			{
				memcpy(pbDst, pbSrc, m_filehdr.sWidth);	// Copy source to buffer
				pbSrc += (ULONG)pbufNext->sPitch;		// Use pitch on source
				pbDst += (ULONG)m_filehdr.sWidth;		// Use width on buffer
			}
			
			// Write out the chunk
			sError = WriteDataChunk(pBuf, lSizeCOPY, FLX_DATA_COPY, plChunkSize);
		}
	}
	else
	{
		// the current frame is not the first frame, we need to determine which encoding scheme to use
		if (m_filehdr.wMagic == FLX_MAGIC_FLI)
		{
			// Since the current format is FLI, let's encode with FLX_DATA_LC
			short y;								// used to index through the pixel buffers
			long lSize;								// size of the current compressed line
			short sFirstYPos = m_filehdr.sHeight-1;	// the first line which is different
			short sLineCount = 0; 					// number of lines in the chunk
			UCHAR* pbDst = pBuf + 4;					// point to the chunk data storage, the first 4 bytes are used to store
													// number of initial unchanged lines and # of lines in the chunk
			long lSizeLC = 4;						// size of the chunk in pBuf
			short sEmptyLineCount = 0;				// number of empty delta lines encountered back to back
			
			// Do this line by line.
			for (y = 0; y < m_filehdr.sHeight; y++)
			{
				// Process/encode the current line.
				sError = CompressLineDelta(y, pbufNext, pbufPrev, pbDst, lSize, sizeof(UCHAR), sEmptyLineCount);
				
				// Trap real errors.
				if (sError == -1)
					return sError;
				
				// Check to results of the line delta compression.
				if (sError != 0)
				{
					// The current line did not compress.  Find out whether previous lines has been compressed before.
					// If no other previous line has been compressed yet, we can simply go onto the next line.
					if (sLineCount != 0)
					{
						// Other previous lines has been compressed already.  Determine whether to maintain a empty delta line count.
						// Increment the empty line count.
						sEmptyLineCount++;
					}
				}
				else
				{
					// The current line compressed.  We need to check to see if this was the first compressible line.
					if (sEmptyLineCount > 0)
					{
						sLineCount += sEmptyLineCount;
						sEmptyLineCount = 0;
					}

					if (sLineCount == 0)
					{
						// This is the first compressed line.  Need to set the line skip for the chunk.
						sFirstYPos = y;
						sLineCount++;
					}
					else
					{
						// Since this is not the first compressed line, increment the line count.
						sLineCount++;
					}
					
					// Make sure we increment the size count and the chunk pointer.
					pbDst += lSize;
					lSizeLC += lSize;
				}
			}
			
			// Done with all the lines.  Write out the number of lines encoded.
			*(USHORT*)pBuf = (USHORT)sFirstYPos;
			*(USHORT*)(pBuf + 2) = (USHORT)sLineCount;
			
			// Write out the chunk.
			sError = WriteDataChunk(pBuf, lSizeLC, FLX_DATA_LC, plChunkSize);
		}
		else
		{
			// Since the current format is FLC, let's encode with FLX_DATA_SS2
			short y; 								// used to index through the pixel buffers
			long lSize;								// size of the current compressed line
			UCHAR* pbDst = pBuf + 2;					// point to the chunk storage data, past the line count word
			long lSizeSS2 = 2;						// size of the chunk in pBuf
			short sLines = 0;						// number of line in this chunk
			short sLineSkipCount = 0;				// number of lines to skip
			
			// Do this line by line.
			for (y = 0; y < m_filehdr.sHeight; y++)
			{
				// Process/encode the current line.
				sError = CompressLineDelta(y, pbufNext, pbufPrev, pbDst, lSize, sizeof(USHORT), sLineSkipCount);
				
				// Trap real errors.
				if (sError == -1)
					return sError;
				
				// Check to see if the current line compressed.
				if (sError != 0)
				{
					// Since the current line didn't compress, increment the sLineSkipCount to keep track of how many
					// unchanged lines were skipped.
					sLineSkipCount++;
				}
				else
				{
					// The current line compressed.  We need to reset the line skip count to zero.  Also increment line count.
					sLineSkipCount = 0;
					sLines++;
					
					// Make sure we increment the size count and the chunk pointer.
					pbDst += lSize;
					lSizeSS2 += lSize;
				}
			}
			
			// Now that we're done with all the lines, let's write the number of lines in the chunk.
			*(short*)pBuf = sLines;
			
			// Write out the chunk!
			sError = WriteDataChunk(pBuf, lSizeSS2, FLX_DATA_SS2, plChunkSize);
		}
	}

	return sError;
}

/////////////////////////////////////////////////////////////////////////////
//
// Name:		CompressLineDelta
//               
// Description:	This function will perform either UCHAR/USHORT oriented delta
//				compression, given the current line.  If compression is possible,
//				the compressed data will be written to the buffer provided.
//				Otherwise, an error will be returned to indicate no compression.
//
// Input:		y = 				current line to compress
//				pbufNext = 			pointer to the current flx frame
//				pbufPrev = 			pointer to the previous flx frame
//				pbDst = 			pointer to the chunk area to be written to, updated
//				lSize = 			size of the current compressed line
//				sAlign =			specifies the data size alignment
//				sLineSkipCount =	skip lines for word oriented delta compression for word aligned
//									or contains number of empty lines for byte aligned
//
// Output:		(short)
//				0 =		Compression success
//				1 =		No compression took place
//				-1 =	A real error occurred!
//
// History:		10/25/94, Paul Lin, original coding.
//
/////////////////////////////////////////////////////////////////////////////
short CFlx::CompressLineDelta(short y, 
							  FLX_BUF* pbufNext, 
							  FLX_BUF* pbufPrev,
							  UCHAR* pbDst,
							  long& lSize,
							  short sAlign,
							  short sLineSkipCount)
{
	// Local variables.
	ULONG dwOffset;							// Offset into the pixel data.
	UCHAR* pbSrcNext = pbufNext->pbPixels;	// Pointer to the pixel data of the current frame.
	UCHAR* pbSrcPrev = pbufPrev->pbPixels;	// Pointer to the pixel data of the previous frame.
	short sPacket = 0;						// Count the number of packets.
	UCHAR* pbPacketCount;					// Pointer to the packet count.
	short x = 0;							// Current position within the current line.
	short sAdjustedPitch;					// Pitch of the current line, adjusted if word aligned
	short sSkipCount;						// Count the bytes/words skipped over.
	UCHAR* pbByteCount;						// Used to hold the position for the byte count temporarily.
	short sIndex;							// Used as an index variable.  
	
	// Initialize the size.
	lSize = 0;
	sAdjustedPitch = pbufNext->sPitch;
	
	// Check to make sure that all pointers to the buffers are not null.
	if ((pbufNext == NULL) || (pbufPrev == NULL) || (pbDst == NULL))
		return -1;
	
	// First let's see if the current line really need to be compressed.
	dwOffset = (ULONG)y * (ULONG)pbufNext->sPitch;
	if (memcmp(pbSrcNext + dwOffset, pbSrcPrev + dwOffset, (size_t)pbufNext->sPitch) == 0)
	{
		// The current line does not need to be compressed!
		return 1;
	}
	
	// Since the current line contains delta info between current and previous frame, let's do actual delta compression.
	
	// Do we need to do anything before we start constructing the packets?
	if ((sAlign == 1) && (sLineSkipCount != 0))
	{
		// Let's put in sLineSkipCount number of 0 packets.
		for (sIndex = 0; sIndex < sLineSkipCount; sIndex++)
		{
			*(pbDst + (ULONG)lSize++) = 0;
		}
	}
	else if (sAlign == 2)
	{
		// For word aligned delta compression, we need to do the optional words, as applicable.
		if (sLineSkipCount > 0)
		{
			// Since we have skipped some lines prior to current line, let's write it to the chunk.
			*(short*)(pbDst + (ULONG)lSize) = -sLineSkipCount;
			lSize += 2;
		}
		
		// If the pitch is odd, we need to store the last byte if different.
//		if ((pbufNext->sPitch % 2) == 1)
//		{
		dwOffset = ((ULONG)y * (ULONG)pbufNext->sPitch) + (ULONG)(pbufNext->sPitch - 1);
		if (pbSrcNext[dwOffset] != pbSrcPrev[dwOffset])
		{
			// The last byte is different.  We need to save it!
			// Put the value in the low-order byte.
			USHORT wLastByte = (USHORT)pbSrcNext[dwOffset];
			wLastByte = wLastByte | 0x8000;
				
			// Save it to the chunk.
			*(USHORT*)(pbDst + (ULONG)lSize) = wLastByte;
			lSize += 2;
		}
			
			// Adjust the pitch to eliminate the odd byte.  Useful for testing limits later.
//			sAdjustedPitch--;
//		}
	}
	
	// Save the position in the chunk for storing the packet count.
	pbPacketCount = pbDst + (ULONG)lSize;
	if (sAlign == 1)
		lSize++;
	else
		lSize += 2;
		
	sSkipCount = 0;
	x = 0;
	sPacket = 0;
	while (((sAlign == 1) && (x < sAdjustedPitch)) || ((sAlign == 2) && (x < sAdjustedPitch - 1)))
	{
		// Do the skip count first.
		dwOffset = (ULONG)y * (ULONG)pbufNext->sPitch;
		while ((x < sAdjustedPitch) && (pbSrcNext[dwOffset + x] == pbSrcPrev[dwOffset + x]))
		{
			// Since the pixel data are still the same, let's increment the skip count and x.
			x++;
			sSkipCount++;
		}
		
		// Continue only if we have not skipped past the end.
		if (((sAlign == 1) && (x < sAdjustedPitch)) || ((sAlign == 2) && (x < sAdjustedPitch - 1)))
		{
			// Even though the codes are very similar between the byte and word aligned delta compression,
			// for ease of reading and simplification, the two will be separated from this point on.
			if (sAlign == 1)
			{
				// Update the pointer offset.
				dwOffset = (ULONG)y * (ULONG)pbufNext->sPitch + x - sSkipCount;
			
				// Do delta compression for byte aligned.
				while (sSkipCount > 255)
				{
					// Put in a packet of one byte and 255 for skip count.
					*(pbDst + (ULONG)lSize++) = 255;
					*(pbDst + (ULONG)lSize++) = 1;
					
					// Increment the pixel data offset by 255       
					dwOffset += 255;
					*(pbDst + (ULONG)lSize++) = pbSrcNext[dwOffset];
					
					// Increment the packet count and decrement the skip count.
					sPacket++;
					sSkipCount -= 256;
				}
				
				// Write out the skip count for the current packet and reset the skip count.
				*(pbDst + (ULONG)lSize++) = (UCHAR)sSkipCount;
				sSkipCount = 0;
				
				// Set the byte counter before we start compressing.
				UCHAR nBytes = 1;
				
				// Determine the packet type.
				dwOffset = (ULONG)y * (ULONG)pbufNext->sPitch;
				if ((x < (sAdjustedPitch - 1)) && (pbSrcNext[dwOffset + x] == pbSrcNext[dwOffset + x + 1]))
				{
					// Let's process as count encoding for bytes of similar values.
					x++;
					while ((nBytes < 127) &&
						   (x < sAdjustedPitch) &&
						   (pbSrcNext[dwOffset + (x - 1)] == pbSrcNext[dwOffset + x]) &&
						   (pbSrcNext[dwOffset + x] != pbSrcPrev[dwOffset + x]))
					{
						x++;
						nBytes++;
					}
					
					// Write the rest of the packet out.
					// We need the byte count to be in 2's complement (negative).
					nBytes = 255 - nBytes + 1;
					*(pbDst + (ULONG)lSize++) = nBytes;
					*(pbDst + (ULONG)lSize++) = pbSrcNext[dwOffset + (x - 1)];
					sPacket++;
				}
				else
				{
					// Let's process as count of different bytes.
					pbByteCount = pbDst + (ULONG)lSize++;
					*(pbDst + (ULONG)lSize++) = pbSrcNext[dwOffset + x];
					x++;
					
					while ((nBytes < 127) &&
						   (x < sAdjustedPitch) &&
						   (pbSrcNext[dwOffset + x] != pbSrcPrev[dwOffset + x]) &&
						   !((x < (sAdjustedPitch - 1)) && (pbSrcNext[dwOffset + x] == pbSrcNext[dwOffset + x + 1])))
					{ 
						*(pbDst + (ULONG)lSize++) = pbSrcNext[dwOffset + x];
						x++;
						nBytes++;
					}
					
					// Save the byte count to complete the current packet.
					*pbByteCount = nBytes;
					sPacket++;
				}
			}
			else
			{
				// Do delta compression for word aligned.
				
				// Update the pointer offset.
				dwOffset = (ULONG)y * (ULONG)pbufNext->sPitch + x - sSkipCount;
			
				// Do delta compression for byte aligned.
				while (sSkipCount > 255)
				{
					// Put in a packet of one byte and 255 for skip count.
					*(pbDst + (ULONG)lSize++) = 254;
					*(pbDst + (ULONG)lSize++) = 1;
					
					// Increment the pixel data offset by 255.  Remember to copy a word value!       
					dwOffset += 254;
					*(pbDst + (ULONG)lSize++) = pbSrcNext[dwOffset++];
					*(pbDst + (ULONG)lSize++) = pbSrcNext[dwOffset++];
					
					// Increment the packet count and decrement the skip count.
					sPacket++;
					sSkipCount -= 256;
				}
				
				// Write out the skip count for the current packet and reset the skip count.
				*(pbDst + (ULONG)lSize++) = (UCHAR)sSkipCount;
				sSkipCount = 0;
				
				// Set the byte counter before we start compressing.
				UCHAR nBytes = 1;
				
				// Determine the packet type.
				dwOffset = (ULONG)y * (ULONG)pbufNext->sPitch;
				if ((x < (sAdjustedPitch - 3)) && 
					(pbSrcNext[dwOffset + x] == pbSrcNext[dwOffset + x + 2]) && 
					(pbSrcNext[dwOffset + x + 1] == pbSrcNext[dwOffset + x + 3]))
				{
					// Let's process as count encoding for words of similar values.
					x += 2;
					while ((nBytes < 127) &&
						   (x < (sAdjustedPitch - 1)) &&
						   (pbSrcNext[dwOffset + x - 2] == pbSrcNext[dwOffset + x]) &&
						   (pbSrcNext[dwOffset + x - 1] == pbSrcNext[dwOffset + x + 1]) &&
						   (pbSrcNext[dwOffset + x + 1] != pbSrcPrev[dwOffset + x + 1]) && 
						   (pbSrcNext[dwOffset + x] != pbSrcPrev[dwOffset + x]))
					{
						x += 2;
						nBytes++;
					}
					
					// Write the rest of the packet out.
					// We need the byte count to be in 2's complement (negative).
					nBytes = 255 - nBytes + 1;
					*(pbDst + (ULONG)lSize++) = nBytes;
					*(pbDst + (ULONG)lSize++) = pbSrcNext[dwOffset + x - 2];
					*(pbDst + (ULONG)lSize++) = pbSrcNext[dwOffset + x - 1];
					sPacket++;
				}
				else
				{
					// Let's process as count of different bytes.
					pbByteCount = pbDst + (ULONG)lSize++;
					*(pbDst + (ULONG)lSize++) = pbSrcNext[dwOffset + x];
					*(pbDst + (ULONG)lSize++) = pbSrcNext[dwOffset + x + 1];
					x += 2;
					
					while ((nBytes < 127) &&
						   (x < (sAdjustedPitch - 1)) &&
						   ((pbSrcNext[dwOffset + x] != pbSrcPrev[dwOffset + x]) ||
						    (pbSrcNext[dwOffset + x + 1] != pbSrcPrev[dwOffset + x + 1])) &&
						   !((x < (sAdjustedPitch - 3)) && 
							 (pbSrcNext[dwOffset + x] == pbSrcNext[dwOffset + x + 2]) && 
							 (pbSrcNext[dwOffset + x + 1] == pbSrcNext[dwOffset + x + 3])))
					{ 
						*(pbDst + (ULONG)lSize++) = pbSrcNext[dwOffset + x];
						*(pbDst + (ULONG)lSize++) = pbSrcNext[dwOffset + x + 1];
						x += 2;
						nBytes++;
					}
					
					// Save the byte count to complete the current packet.
					*pbByteCount = nBytes;
					sPacket++;
				}
			}
		}
	}
				
	// Remember to write out the number of packets. 
	if (sAlign == 1)
		*pbPacketCount = (UCHAR)sPacket;
	else
		*(USHORT*)pbPacketCount = (USHORT)sPacket;
	
	// Compression successful.
	return 0;
}
	
	
/////////////////////////////////////////////////////////////////////////////
//
// Compresses pixels using the BRUN method.
//
/////////////////////////////////////////////////////////////////////////////
long CFlx::CompressBRUN(
	UCHAR* pbIn,				// Pointer to input (pixels to be compressed)
	short sPitch,			// Pitch (distance from one pixel to the pixel below it)
	short sSrcX,			// Starting x of rectangular area to compress
	short sSrcY,			// Starting y of rectangular area to compress
	short sWidth,			// Width of rectangular area to compress
	short sHeight,			// Height of rectangular area to compress
	UCHAR* pbOut)			// Pointer to output (compressed data)
	{
	long	lUniqueX;
	long lUniqueCnt;
	long lRepeatCnt;
	UCHAR	bRepeatPix;
	long	x;
	long	y;
	long	lOutCnt = 0;
	UCHAR* pbPackets;
	
	// Adjust input pointer based on starting x and y
	pbIn = pbIn + ((ULONG)sSrcY * (ULONG)sPitch) + (ULONG)sSrcX;
	
	// Loop through the scanlines
	for (y = 0; y < sHeight; y++)
		{
		// Write out number of packets on this line.  This is a holdover from
		// the older Animator program.  Autodesk recommends that it be ignored
		// and that the width be used to figure out when the line is done.
		// Just to be safe, we update it anyway.  Note, however, that if
		// we are compressing a very wide image, then this count could go over
		// 256, but it's only a byte, so it'll get truncated.  By the way, this
		// is why Autodesk ignores it in the new FLI file!
		pbPackets = pbOut;	// Get pointer to this byte
		*pbPackets = 0;		// Init packet count to 0
		pbOut++;					// Skip over packet count
		lOutCnt++;
		
		// Loop through the pixels in the current scanline
		lUniqueCnt = 0;
		lUniqueX = 0;
		x = 0;
		do	{
	
			// Count how many additional pixels match current pixel
			lRepeatCnt = 0;
			bRepeatPix = pbIn[x];
			lRepeatCnt++;
			while ((x + lRepeatCnt < sWidth) && (lRepeatCnt < 127))
				{
				if (pbIn[x + lRepeatCnt] == bRepeatPix)
					lRepeatCnt++;
				else
					break;
				}

			// If repeat is greater than or equal to minimum then it
			// qualifies for its own repeat packet.
			if (lRepeatCnt >= 3)
				{
				// If necessary, write out pending unique packet.
				if (lUniqueCnt > 0)
					{
					(*pbPackets)++;
					*pbOut++ = (UCHAR)(-lUniqueCnt);	// Unique counts are negative
					lOutCnt++;
					do	{
						*pbOut++ = pbIn[lUniqueX++];
						lOutCnt++;
						} while (--lUniqueCnt);
					}

				// Now write out the repeat packet.
				(*pbPackets)++;
				*pbOut++ = (UCHAR)lRepeatCnt;
				lOutCnt++;
				*pbOut++ = bRepeatPix;
				lOutCnt++;
				
				// Move ahead to next next pixel after the repeated pixels.
				x += lRepeatCnt;
					
				// Reset unique stuff
				lUniqueX = x;
				lUniqueCnt = 0;
				}
			else
				{
				// Move ahead to next pixel
				x++;
				
				// Inc unique size
				lUniqueCnt++;
				
				// If necessary, write out unique packet.
				if (lUniqueCnt == 127)
					{
					(*pbPackets)++;
					*pbOut++ = (UCHAR)(-lUniqueCnt);	// Unique counts are negative
					lOutCnt++;
					do	{
						*pbOut++ = pbIn[lUniqueX++];
						lOutCnt++;
						} while (--lUniqueCnt);
					}
				}
			} while (x < sWidth);

		// If necessary, write out pending unique packet.
		if (lUniqueCnt > 0)
			{
			(*pbPackets)++;
			*pbOut++ = (UCHAR)(-lUniqueCnt);	// Unique counts are negative
			lOutCnt++;
			do	{
				*pbOut++ = pbIn[lUniqueX++];
				lOutCnt++;
				} while (--lUniqueCnt);
			}
						
		// Update pointer to point at next scan line.  Note that we use a
		// different width here since the scan line could be wider than the
		// number of pixels being compressed.
		pbIn += (ULONG)sPitch;
		}

	return lOutCnt;
	}


///////////////////////////////////////////////////////////////////////////////
//
// Helper function that write a data chunk using the specified data/values.
//
///////////////////////////////////////////////////////////////////////////////
short CFlx::WriteDataChunk(UCHAR* pbData, long lSize, USHORT wType, long* plChunkSize)
	{
	FLX_DATA_HDR datahdr;
	
	// Fill in the data chunk header
	datahdr.lChunkSize = 6 + lSize;	// Set to header size plus data size
	datahdr.wType = wType;				// Set indicated type
	
	// If chunk size is odd, round it up to the next even value
	if (datahdr.lChunkSize & 1)
		datahdr.lChunkSize++;
	
	// Get current file position (the start of the data chunk header)
	long lDataPos = m_file.Tell();
	
	// Write data chunk header
	m_file.Write(&datahdr.lChunkSize);
	m_file.Write(&datahdr.wType);
	
	// Write the actual data (first make sure there is some!)
	if (lSize > 0)
		{
		// Because intel sucks, we must use multiple writes for large data.
		// The write size is a 16-bit int, so it is limited to <= 32767!
		// We use 16384 (16k) because it's easier to think about.
		while(lSize >= 16384L)
			{
			m_file.Write(pbData, (int)16384);
			pbData += (ULONG)16384;
			lSize -= 16384;
			}
		if (lSize > 0)
			m_file.Write(pbData, lSize);
		}
	
	// Seek to end of data chunk, which, due to rounding, may be slightly
	// past the end of the data.
	m_file.Seek(lDataPos + datahdr.lChunkSize, SEEK_SET);
	
	// Return the chunk size, which is different than the data size
	*plChunkSize = datahdr.lChunkSize;
	
	// If good then return success, otherwise return error.
	if (m_file.Error() == FALSE)
		return 0;
	else
		return 1;
	}
	
	
///////////////////////////////////////////////////////////////////////////////
//
// Helper function that reads the header of the flic file.  Always returns
// with file position at start of frame 1.
//
///////////////////////////////////////////////////////////////////////////////
short CFlx::ReadHeader(void)
	{
	// Seek to start of file
	m_file.Seek(0, SEEK_SET);
			
	// Read the part of the file header that's common to FLC and FLI files.
	m_file.Read(&m_filehdr.lEntireFileSize);
	m_file.Read(&m_filehdr.wMagic);
	m_file.Read(&m_filehdr.sNumFrames);
	m_file.Read(&m_filehdr.sWidth);
	m_file.Read(&m_filehdr.sHeight);
	m_file.Read(&m_filehdr.sDepth);
	m_file.Read(&m_filehdr.sFlags);

	// The headers become different at this point
	if (m_filehdr.wMagic == FLX_MAGIC_FLC)
		{
		// Read the remainder of the FLC header
		m_file.Read(&m_filehdr.lMilliPerFrame);
		m_file.Read(&m_filehdr.sReserveA);
		m_file.Read(&m_filehdr.dCreatedTime);
		m_file.Read(&m_filehdr.dCreator);
		m_file.Read(&m_filehdr.dUpdatedTime);
		m_file.Read(&m_filehdr.dUpdater);
		m_file.Read(&m_filehdr.sAspectX);
		m_file.Read(&m_filehdr.sAspectY);
		m_file.Read(m_filehdr.bReservedB, sizeof(m_filehdr.bReservedB));
		m_file.Read(&m_filehdr.lOffsetFrame1);
		m_file.Read(&m_filehdr.lOffsetFrame2);
		m_file.Read(m_filehdr.bReservedC, sizeof(m_filehdr.bReservedC));
				
		// In FLC files, an optional prefix chunk may follow the header.
		// According to Autodesk it contains information not related to
		// animation playback and should be ignored.  They also say that
		// programs other than Animator Pro shouldn't write it.
		// We currently ignore this chunk.  In the future, we might
		// consider preserving it in case this flic is written out to a
		// new file.  The easiest way to skip over it is to seek directly
		// to the first frame using the offset specified in the header.
		// Seek directly to first frame.
		m_file.Seek(m_filehdr.lOffsetFrame1, SEEK_SET);
		}
	else
		{
		// Read the FLI's jiffies (a jiffy is 1/70th second) and convert to
		// to FLC's milliseconds.
		short sJiffies;
		m_file.Read(&sJiffies);
		m_filehdr.lMilliPerFrame = (long)( (double)sJiffies * ((double)1000 / (double)70L) + (double)0.5 );
		
		// Set times to 0 for lack of better value (some day, we could read the
		// file's date and time stamp and put it here).  We use "FLIB" for the
		// serial numbers, which is safe according to the doc's.
		m_filehdr.dCreatedTime = 0;
		m_filehdr.dCreator = 0x464c4942;
		m_filehdr.dUpdatedTime = 0;
		m_filehdr.dUpdater = 0x464c4942;
		
		// Aspect ratio for 320x200 (which is the only FLI size) is 6:5
		m_filehdr.sAspectX = 6;
		m_filehdr.sAspectY = 5;
		
		// Skip to end of header.  This is also the starting position of
		// frame 1, which we save in the header.
		m_file.Seek(128, SEEK_SET);
		m_filehdr.lOffsetFrame1 = m_file.Tell();
		
		// Get size of frame 1's chunk in order to calculate the starting
		// position of frame 2.
		long lSizeFrame1;
		m_file.Read(&lSizeFrame1);
		m_filehdr.lOffsetFrame2 = m_filehdr.lOffsetFrame1 + lSizeFrame1;
		
		// Seek to start of frame 1
		m_file.Seek(m_filehdr.lOffsetFrame1, SEEK_SET);
		}

	// If good then return success, otherwise return error.
	if (m_file.Error() == FALSE)
		return 0;
	else
		return 1;
	}
	
	
///////////////////////////////////////////////////////////////////////////////
//
// Helper function that writes the header of the flic file.  Always returns
// with file position immediately following the header.
//
///////////////////////////////////////////////////////////////////////////////
short CFlx::WriteHeader(void)
	{
	// Seek to start of file
	m_file.Seek(0, SEEK_SET);
	
	// Write the part of the file header that's common to FLC and FLI files.
	m_file.Write(&m_filehdr.lEntireFileSize);
	m_file.Write(&m_filehdr.wMagic);
	m_file.Write(&m_filehdr.sNumFrames);
	m_file.Write(&m_filehdr.sWidth);
	m_file.Write(&m_filehdr.sHeight);
	m_file.Write(&m_filehdr.sDepth);
	m_file.Write(&m_filehdr.sFlags);

	// The headers become different at this point
	if (m_filehdr.wMagic == FLX_MAGIC_FLC)
		{
		// Read the remainder of the FLC header
		m_file.Write(&m_filehdr.lMilliPerFrame);
		m_file.Write(&m_filehdr.sReserveA);
		m_file.Write(&m_filehdr.dCreatedTime);
		m_file.Write(&m_filehdr.dCreator);
		m_file.Write(&m_filehdr.dUpdatedTime);
		m_file.Write(&m_filehdr.dUpdater);
		m_file.Write(&m_filehdr.sAspectX);
		m_file.Write(&m_filehdr.sAspectY);
		m_file.Write(m_filehdr.bReservedB, sizeof(m_filehdr.bReservedB));
		m_file.Write(&m_filehdr.lOffsetFrame1);
		m_file.Write(&m_filehdr.lOffsetFrame2);
		m_file.Write(m_filehdr.bReservedC, sizeof(m_filehdr.bReservedC));
		
		// In FLC files, an optional prefix chunk may follow the header.
		// According to Autodesk it contains information not related to
		// animation playback and should be ignored.  They also say that
		// programs other than Animator Pro shouldn't write it, so we don't.
		
		// Seek to position immediately after header, which is always 128 bytes.
		m_file.Seek(128, SEEK_SET);
		}
	else
		{
		// Convert from milliseconds to FLI's jiffies (a jiffy is 1/70th second)
		// and write that out.
		short sJiffies = (short)( (double)m_filehdr.lMilliPerFrame * ((double)70 / (double)1000) + (double)0.5 );
		m_file.Write(&sJiffies);

		// Write 0's to rest of header, which is officially reserved
		UCHAR bZero = 0;
		for (short i = 0; i < 110; i++)
			m_file.Write(&bZero);

		// Seek to position immediately after header, which is always 128 bytes.
		m_file.Seek(128, SEEK_SET);
		}

	// If good then return success, otherwise return error.
	if (m_file.Error() == FALSE)
		return 0;
	else
		return 1;
	}
	
	
///////////////////////////////////////////////////////////////////////////////
//
// Helper function that clears file header.
//
///////////////////////////////////////////////////////////////////////////////
void CFlx::ClearHeader(void)
	{	
	// Clear all fields in file header
	short i;
	m_filehdr.lEntireFileSize = 0;
	m_filehdr.wMagic = 0;
	m_filehdr.sNumFrames = 0;
	m_filehdr.sWidth = 0;
	m_filehdr.sHeight = 0;
	m_filehdr.sDepth = 0;
	m_filehdr.sFlags = 0;
	m_filehdr.lMilliPerFrame = 0;
	m_filehdr.sReserveA = 0;
	m_filehdr.dCreatedTime = 0;
	m_filehdr.dCreator = 0;
	m_filehdr.dUpdatedTime = 0;
	m_filehdr.dUpdater = 0;
	m_filehdr.sAspectX = 0;
	m_filehdr.sAspectY = 0;
	for (i = 0; i < sizeof(m_filehdr.bReservedB); i++)
		m_filehdr.bReservedB[i] = 0;
	m_filehdr.lOffsetFrame1 = 0;
	m_filehdr.lOffsetFrame2 = 0;
	for (i = 0; i < sizeof(m_filehdr.bReservedC); i++)
		m_filehdr.bReservedC[i] = 0;
	}
	
	
///////////////////////////////////////////////////////////////////////////////
//
// Helper functions to deal with memory associated with buf's.
//
///////////////////////////////////////////////////////////////////////////////
void CFlx::InitBuf(FLX_BUF* pbuf)
	{
	// The pointers MUST be cleared to NULL so we can tell later on whether
	// any memory needs to be freed.
	pbuf->pbPixels = NULL;
	pbuf->prgbColors = NULL;
	}
	
	
short CFlx::AllocBuf(FLX_BUF* pbuf, short sWidth, short sHeight, short sColors)
	{
	// Allocate buffer for pixels & set pitch to width
	pbuf->pbPixels = (UCHAR*)malloc((size_t)sWidth * (size_t)sHeight);
	pbuf->sPitch = sWidth;
	
	// Allocate buffer for colors
	pbuf->prgbColors = (FLX_RGB*)malloc((size_t)sColors * sizeof(FLX_RGB));
	
	// If it worked then return success
	if ((pbuf->pbPixels != NULL) && (pbuf->prgbColors != NULL))
		return 0;
	// Else free anything that did get allocated and return failure
	else
		{
		FreeBuf(pbuf);
		return 1;
		}
	}
	
	
void CFlx::FreeBuf(FLX_BUF* pbuf)
	{
	if (pbuf->pbPixels != NULL)
		{
		free(pbuf->pbPixels);
		pbuf->pbPixels = NULL;
		}
	if (pbuf->prgbColors != NULL)
		{
		free(pbuf->prgbColors);
		pbuf->prgbColors = NULL;
		}
	}
	
	
void CFlx::CopyBuf(FLX_BUF* pbufDst, FLX_BUF* pbufSrc)
	{
	// Copy pixels one row at a time
	UCHAR* pbSrc = pbufSrc->pbPixels;
	UCHAR* pbDst = pbufDst->pbPixels;
	for (short y = 0; y < m_filehdr.sHeight; y++)
		{
		memcpy(pbDst, pbSrc, m_filehdr.sWidth);
		pbSrc += (ULONG)pbufSrc->sPitch;
		pbDst += (ULONG)pbufDst->sPitch;
		}
		
	// Copy colors (assume 256 of them)
	memcpy(pbufDst->prgbColors, pbufSrc->prgbColors, 256 * sizeof(FLX_RGB));
	}


///////////////////////////////////////////////////////////////////////////////
// EOF
///////////////////////////////////////////////////////////////////////////////
