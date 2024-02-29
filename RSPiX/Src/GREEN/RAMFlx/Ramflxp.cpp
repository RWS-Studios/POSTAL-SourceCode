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
//	11/15/95	JMI	Fixed AllocBuf and FreeBuf to leave more of the dynamic
// 					allocation up to CImage/CPal.
//
//	03/06/96	JMI	Added use of new FLX8_888/PFLX CImage/CPal format and
//						conversions.  Made the default CImage/CPal type FLX8_888/PFLX
//
///////////////////////////////////////////////////////////////////////////////
//
// THIS FILE CONTAINS ONLY THE PRIVATE FUNCTIONS.
// THE PUBLIC FUNCTIONS ARE IN FLX.CPP
// ALL MAJOR COMMENT BLOCKS ARE IN FLX.CPP AS WELL.
//
///////////////////////////////////////////////////////////////////////////////

#include <string.h>
#include <malloc.h>

#include <SMRTHEAP.HPP>

#include "common/SYSTEM.H"

#include "image/IMAGE.H"
#include "ramflx/RAMFLX.H"

#ifdef WIN32
//////////////////////////////////////////////////////////////////////////////////
// Assembly macros for i386.
//////////////////////////////////////////////////////////////////////////////////
// C Declarator for a memfile pointer.
#define MEM	UCHAR*

// Call this first with a pointer to the start of the memory area.
// pFile	== ptr or reg
// pMem	== ptr or reg
#define AMEM_OPEN(pFile, pMem)		mov pFile, pMem
//                        
// Call this to reposition the pointer
// pFile		== ptr or reg
// offset	== reg or immediate
#define AMEM_SEEK(pFile, offset)		add pFile, offset
//
// Call this to Save an offset
// pTell		== ptr or reg to get data.
// pFile		== ptr or reg
// pMem		== ptr or reg used to open mem file
#define AMEM_TELL(pTell, pFile, pMem)	__asm mov pTell, pMem\
													__asm sub pTell, pFile
//
// Call any of these to get a UCHAR, USHORT, or ULONG from the memory area.  The
// pointer is automatically updated to follow the data that was just read.
// These macros also hide the byte-ordering on the Mac vs the PC.
// pDst		== [ptr] or reg to get data
// pFile		== ptr or reg
#define AMEM_BYTE(pDst, pFile)	__asm mov pDst, byte ptr [pFile] \
											__asm add pFile, 1
#define AMEM_WORD(pDst, pFile)	__asm mov pDst, word ptr [pFile]	\
											__asm add pFile, 2
#define AMEM_DWORD(pDst, pFile)	__asm mov pDst, dword ptr [pFile]\
											__asm add pFile, 4

#endif // WIN32

//////////////////////////////////////////////////////////////////////////////////
// Module macros.
//////////////////////////////////////////////////////////////////////////////////
#define DWORDALIGN(i)   ((i+3) & ~3)


//////////////////////////////////////////////////////////////////////////////////
// Module specific prototypes.
//////////////////////////////////////////////////////////////////////////////////
static short ConvToFlx8_888(CImage* pImage);
static short ConvFromFlx8_888(CImage* pImage);

//////////////////////////////////////////////////////////////////////////////////
// Link conversion functions into CImage.
IMAGELINKLATE(FLX8_888, 
				  ConvToFlx8_888, ConvFromFlx8_888,	// Conversions To and From.
				  /*pLoad*/NULL, /*pSave*/NULL,		// Load and Save.
				  /*pAlloc*/NULL, /*pDel*/NULL);		// Alloc and Delete.
//////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
// Helper function that restarts at beginning of the flic.
//
///////////////////////////////////////////////////////////////////////////////
void CRamFlx::Restart(void)
	{
	// Seek to memory file position of frame 1
	m_file.Seek(m_filehdr.lOffsetFrame1, SEEK_SET);
	
	// Set frame number to 0 to indicate that nothing's been read yet.
	m_sFrameNum = 0;
	}
	
///////////////////////////////////////////////////////////////////////////////
//
// Instantiable shell function for workhorse DoReadFrame static.
// Returns 0 if successfull, non-zero otherwise.
//
///////////////////////////////////////////////////////////////////////////////
short CRamFlx::DoReadFrame(CImage* pimageRead)
	{
	short sError = DoReadFrame(pimageRead, &m_file, 
										&m_sPixelsModified, &m_sColorsModified);

	// If everything went fine, update the frame number.
	if (sError == 0)
		{
		if (m_sNoDelta == TRUE)
			{
			if (++m_sFrameNum == m_filehdr.sNumFrames)
				{
				// Seek to file position of frame 1 (the next one we'll do)
				m_file.Seek(m_plFrames[1], SEEK_SET);
				}
			else
				{
				// Reset frame number
				if (m_sFrameNum == (m_filehdr.sNumFrames + 1))
					m_sFrameNum = 1;
				}
			}
		else
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
		}

	return sError;
	}
	
///////////////////////////////////////////////////////////////////////////////
//
// Read the next flic frame (if flic was just opened, this will read frame 1).
// The "modified" flags are updated in the specified CImage.
// Returns 0 if successfull, non-zero otherwise.
// (static)
//
///////////////////////////////////////////////////////////////////////////////
short CRamFlx::DoReadFrame(CImage* pimageRead, CNFile* pfile,
									short* psPixelsModified, short* psColorsModified)
	{
	short sError = 0;

	// Always clear modified flags to FALSE.  The lower-level functions will
	// set the appropriate flag to TRUE as necessary.
	*psPixelsModified = FALSE;
	*psColorsModified = FALSE;
	
	// Get current file position.  After each chunk, we add the chunk size
	// to this position to get to the next chunk.  We must do that seek
	// instead of relying on the amount of data that was read from the
	// chunk because that amount may be less than the indicated chunk size!
	// (This is not clearly documented, but was discovered the hard way!)
	long lFramePos = pfile->Tell(); 
			
	// Read frame chunk header
	FLX_FRAME_HDR framehdr;
	pfile->Read(&framehdr.lChunkSize); 	
	pfile->Read(&framehdr.wType);
	pfile->Read(&framehdr.sNumSubChunks);
	pfile->Read(framehdr.bReserved,8);		
		
	// Verify that it worked and it's the type we're expecting
	if (framehdr.wType == 0xF1FA)
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
			long lDataPos = pfile->Tell();	
					
			// Read data chunk header
			pfile->Read(&datahdr.lChunkSize);	
			pfile->Read(&datahdr.wType);			

			// Size of actual data is chunk size minus header size (6)
			long lDataSize = datahdr.lChunkSize - 6;
					
			// Call the appropriate function based on data type
			switch(datahdr.wType)
				{
				case FLX_DATA_COLOR256:
					//cout << "   DATA_COLOR256 of size " << lDataSize << endl;
					if (pimageRead->pPalette->pData != NULL)
						sError = ReadDataColor(	pimageRead, FLX_DATA_COLOR256, pfile,
														psColorsModified);
					break;
							
				case FLX_DATA_SS2:
					//cout << "   DATA_SS2 of size " << lDataSize << endl;
					if (pimageRead->pData != NULL)
						sError = ReadDataSS2(pimageRead, pfile, psPixelsModified);
					break;
							
				case FLX_DATA_COLOR:
					//cout << "   DATA_COLOR of size " << lDataSize << endl;
					if (pimageRead->pPalette->pData != NULL)
						sError = ReadDataColor(	pimageRead, FLX_DATA_COLOR, pfile, 
														psColorsModified);
					break;
							
				case FLX_DATA_LC:
					//cout << "   DATA_LC of size " << lDataSize << endl;
					if (pimageRead->pData != NULL)
						sError = ReadDataLC(pimageRead, pfile, psPixelsModified);
					break;
							
				case FLX_DATA_BLACK:
					//cout << "   DATA_BLACK of size " << lDataSize << endl;
					if (pimageRead->pData != NULL)
						sError = ReadDataBlack(pimageRead, psPixelsModified);
					break;
							
				case FLX_DATA_BRUN:
					//cout << "   DATA_BRUN of size " << lDataSize << endl;
					if (pimageRead->pData != NULL)
						sError = ReadDataBRun(pimageRead, pfile, psPixelsModified);
					break;
							
				case FLX_DATA_COPY:
					//cout << "   DATA_COPY of size " << lDataSize << endl;
					if (pimageRead->pData != NULL)
						sError = ReadDataCopy(pimageRead, pfile, psPixelsModified);
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
			pfile->Seek(lDataPos + datahdr.lChunkSize, SEEK_SET); 
			}
					
		// Adjust file position based on specified chunk size.
		pfile->Seek(lFramePos + framehdr.lChunkSize, SEEK_SET); 
		}
	else
		sError = 1;
	
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
// (static)
//
///////////////////////////////////////////////////////////////////////////////
short CRamFlx::ReadDataColor(	CImage* pimageRead, short sDataType, 
										CNFile* pfile, short* psColorsModified)
	{
	//assert(pimageRead->prgbColors != NULL);
	// instead of assert, just return error
	if (pimageRead->pPalette->pData == NULL)
		return 1;

	short sError = 0;
	
	// Read number of packets
	short sNumPackets;
	pfile->Read(&sNumPackets);

	// Start the color index at 0 and then process each of the packets
	short sColorIndex = 0;
	short sCnt;
	short sColorDo;
	UCHAR bColorSkip;
	UCHAR bVal;

	// Pointer for more easily dealing with the palette in RGB888 format.
	PRGBT8	prgbt;

	for (short sPack = 0; (sPack < sNumPackets) && (sError == 0); sPack++)
		{
		// Read number of colors to skip and add to color index
		pfile->Read(&bColorSkip);	
		sColorIndex = sColorIndex + (short)bColorSkip;
	
		// Read number of colors to do.  This determines how many sets
		// of r,g,b data (3 bytes) will follow.  If this count is 0, it
		// must be interpreted as a count of 256!
		pfile->Read(&bVal);	
		if (bVal != 0)
			sColorDo = (short)bVal;
		else
			sColorDo = (short)256;

		// Make sure we won't index past end of palette!  This would only
		// happen if we were getting bogus data from the file.
		if ((sColorIndex + sColorDo) <= 256)
			{
			// We need to create a temporary palette in FLX8_888:PFLX and then 
			// convert it to the user's type.  To do this we actually must create 
			// an image as well since the conversion functions work on image's 
			// more than just palettes.
			CPal		palTemp;
			CImage	imageTemp;
			// Point image's palette ptr to the palette.
			imageTemp.pPalette		= &palTemp;
			imageTemp.ulType			= FLX8_888;
			imageTemp.ulSize			= 0L;
			// Create some palette data for RGBQUADs.
			palTemp.sStartIndex		= 0;
			palTemp.sNumEntries		= sColorDo;
			palTemp.ulType				= PFLX;
			palTemp.sPalEntrySize	= CPal::GetPalEntrySize(palTemp.ulType);
			if (palTemp.CreateData(palTemp.sNumEntries * palTemp.sPalEntrySize) == 0)
				{
				prgbt	= (PRGBT8)palTemp.pData;
				pfile->Read(prgbt, sColorDo * palTemp.sPalEntrySize);
				if (sDataType == FLX_DATA_COLOR256)
					{
					// Done.
					}
				else
					{
					// Intensify these values in place.
					for (sCnt = 0; sCnt < sColorDo; sCnt++, prgbt++)
						{
						prgbt->ucRed	= prgbt->ucRed		<< 2;
						prgbt->ucGreen	= prgbt->ucGreen	<< 2;
						prgbt->ucBlue	= prgbt->ucBlue	<< 2;
						}
					}

				// If types are different . . .
				if (imageTemp.ulType != pimageRead->ulType)
					{
					// Convert to user buffer type.
					if (imageTemp.Convert(pimageRead->ulType) != NOT_SUPPORTED)
						{
						// Converted successfully.
						}
					else
						{
						TRACE("ReadDataColor(): No conversion to palette type.\n");
						sError = 2;
						}
					}

				if (sError == 0)
					{
					ASSERT(sColorIndex * pimageRead->pPalette->sPalEntrySize + palTemp.ulSize <= pimageRead->pPalette->ulSize);
					// Copy data.
					memcpy(	(UCHAR*)pimageRead->pPalette->pData + sColorIndex * pimageRead->pPalette->sPalEntrySize, 
								palTemp.pData, 
								palTemp.ulSize);
					}

				// Advance counter.
				sColorIndex += sColorDo;
				// Destroy temp palette.
				palTemp.DestroyData();
				}
			else
				{
				sError = 1;
				}
			}
		else
			sError = 1;
		}

	// Set modified flag
	*psColorsModified = TRUE;

	return sError;
	}
	
	
///////////////////////////////////////////////////////////////////////////////
//
// Handler for data chunks of type FLX_DATA_BLACK.
//
// These chunks contain no data.  They are essentially a command that tells
// us to clear all the pixels to color index 0.
// (static)
//
///////////////////////////////////////////////////////////////////////////////
short CRamFlx::ReadDataBlack(CImage* pimageRead, short* psPixelsModified)
	{
	//assert(pimageRead->pbPixels != NULL);
	//assert(pimageRead->sPitch > 0);
	// let's just return with error instead of asserting and crashing
	if ((pimageRead->pData == NULL) || (pimageRead->lPitch <= 0))
		return 1;

	// Clear the image to 0 one row at a time.  Note that we use the pitch
	// to move from the start of one row to the start of the next row.
	UCHAR* pbMem = (UCHAR*)pimageRead->pData;
	for (short y = 0; y < pimageRead->lHeight; y++)
		{
		memset(pbMem, 0, pimageRead->lWidth);
		pbMem += (ULONG)pimageRead->lPitch;
		}
	
	// Set modified flag
	*psPixelsModified = TRUE;

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
// (static)
//
///////////////////////////////////////////////////////////////////////////////
short CRamFlx::ReadDataCopy(	CImage* pimageRead, CNFile* pfile, 
										short* psPixelsModified)
	{
	//assert(pimageRead->pbPixels != NULL);
	//assert(pimageRead->sPitch > 0);
	// let's just return with error instead of asserting
	if ((pimageRead->pData == NULL) || (pimageRead->lPitch <= 0))
		return 1;

	short sError = 0;
	
	// Read in the image one row at a time.  Note that we use the pitch
	// to move from the start of on row to the start of the next row.
	UCHAR* pbMem = (UCHAR*)pimageRead->pData;
	for (short y = 0; y < pimageRead->lHeight; y++)
		{
		// Copy pixels
//		memcpy(pbMem,m_pCurFlxBuf,m_filehdr.sWidth); 
		// Increment buffer pointers
//		m_pCurFlxBuf = MEM_SEEK(m_pCurFlxBuf,m_filehdr.sWidth);
		pfile->Read(pbMem, pimageRead->lWidth);
		pbMem += (ULONG)pimageRead->lPitch;
		}

	// Set modified flag
	*psPixelsModified = TRUE;

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
// (static)
//
///////////////////////////////////////////////////////////////////////////////
short CRamFlx::ReadDataBRun(	CImage* pimageRead, CNFile* pfile, 
										short* psPixelsModified)
	{
	//assert(pimageRead->pbPixels != NULL);
	//assert(pimageRead->sPitch > 0);
	// let's just return with error instead of asserting
	if ((pimageRead->pData == NULL) || (pimageRead->lPitch <= 0))
		return 1;

	// added 10/20/94 to trap errors and exit! instead of asserting
	short sError = 0;

	UCHAR*	pbRow = (UCHAR*)pimageRead->pData;
					
	#ifndef WIN32

		UCHAR bVal;
		S8		cVal;
		short sCount;
		short x;
		short y;
		UCHAR* pbPix;
	
		// Decompress image one row at a time.  Note that we use the pitch
		// to move from the start of one row to the start of the next row.
		pbRow = (UCHAR*)pimageRead->pData;
		for (y = 0; (y < pimageRead->lHeight) && (sError == 0); y++)
			{
			// First byte is number of packets, which can be ignored (Animator used
			// it, but Animator Pro, which supports width > 320, does not.)
//			cVal = MEM_BYTE(m_pCurFlxBuf);	
			pfile->Read(&cVal);
		
			// Keep processing packets until we reach the width
			pbPix = pbRow;
			x = 0;
			while ((x < pimageRead->lWidth) && (sError == 0))
				{
				// First byte of each packet is type/size.  If bit 7 is 1, bits 6-0
				// are number of pixels to be copied.  If bit 7 is 0, bits 6-0 are
				// the number of times to replicate a single pixel.
//				cVal = MEM_BYTE(m_pCurFlxBuf);	
				pfile->Read(&cVal);
				//assert(cVal != 0);	// Not sure how to handle 0!
				if (cVal != 0)
					{
					sCount = (short)cVal;
					if (sCount < 0)
						{
						sCount = -sCount;
						x += sCount;
						// Copy pixels
//						memcpy(pbPix,m_pCurFlxBuf,sCount);	
						// Increment buffer pointers
//						m_pCurFlxBuf = MEM_SEEK(m_pCurFlxBuf,sCount);
						pfile->Read(pbPix, sCount);
						pbPix += (ULONG)sCount;
						}
					else
						{
						x += sCount;
//						bVal = MEM_BYTE(m_pCurFlxBuf);	
						pfile->Read(&bVal);
						memset(pbPix, (int)bVal, (size_t)sCount);
						pbPix += (ULONG)sCount;
						}
					}
				else
					{
						sError = 1;
					}
				}
			
			pbRow += (ULONG)pimageRead->lPitch;
			}
		#else
			// Locals needed (b/c we can't use this->).
			long		lPitch		= pimageRead->lPitch;
			MEM		pCurFlxBuf	= pfile->GetMemory() + pfile->Tell();
			short		sHeight		= (short)pimageRead->lHeight;
			short		sWidth		= (short)pimageRead->lWidth;
				
			__asm
				{
					// Decompress image one row at a time.  Note that we use the pitch
					// to move from the start of one row to the start of the next row.
					mov	esi, pCurFlxBuf	; File ptr.

					mov	bx, sHeight			; for (y = 0; (y < m_filehdr.sHeight) && (sError == 0); y++)
					cmp	bx, 0					; If sHeight == 0, done.
					je		BRunMainLoopDone		

					cld							; Clear direction flag.
					mov	edi, pbRow			; pbPix = pbRow;
					xor	ecx, ecx				; clear ecx.


				BRunMainLoop:					
					// First byte is number of packets, which can be ignored (Animator used
					// it, but Animator Pro, which supports width > 320, does not.)
					AMEM_BYTE(cl, esi)		; cVal = MEM_BYTE(m_pCurFlxBuf);	
		
					// Keep processing packets until we reach the width
					mov	dx, sWidth			; x = 0;  ASM version counts down instead of up.

				BRunPacketLoop:				; while ((x < m_filehdr.sWidth) && (sError == 0))
					// First byte of each packet is type/size.  If bit 7 is 1, bits 6-0
					// are number of pixels to be copied.  If bit 7 is 0, bits 6-0 are
					// the number of times to replicate a single pixel.
					AMEM_BYTE(cl, esi)		; cVal = MEM_BYTE(m_pCurFlxBuf);	
					//assert(cVal != 0);	// Not sure how to handle 0!
					; sCount = (short)cVal;
					cmp	cl, 0					; if (sCount >= 0)
					jge	BRunRepeat			; handle repeat pixel
													; else handle uniques

					neg	cl						; sCount = -sCount;
					
					sub	dx, cx				; x += sCount;  ASM version counts down instead of up.

					// Copy pixels
					; memcpy(pbPix,m_pCurFlxBuf,sCount);	
					; pbPix += (ULONG)sCount;
					// Increment buffer pointers
					; m_pCurFlxBuf = MEM_SEEK(m_pCurFlxBuf,sCount);

					rep movs	byte ptr [edi], byte ptr [esi]	; Copy by bytes.  Does nothing if cx is 0.
					
					jnz	BRunPacketLoop		; If we do not hit zero (from sub), continue loop.

					jmp	BRunPacketLoopDone
				
				BRunRepeat:
					// Get repeat pixel.
					AMEM_BYTE(al, esi)		; bVal = MEM_BYTE(m_pCurFlxBuf);	
					
					sub	dx, cx				; x += sCount;  ASM version counts down instead of up.

					; memset(pbPix, (int)bVal, (size_t)sCount);
					; pbPix += (ULONG)sCount;
					rep stos	byte ptr [edi]	; Set pixels.  Does nothing if cx is 0.

					jnz	BRunPacketLoop		; If we do not hit zero (from sub), continue loop.
					
				
				BRunPacketLoopDone:

					mov	edi,	pbRow
					add	edi,	lPitch				; pbRow += (ULONG)pimageRead->lPitch;
					mov	pbRow, edi					; Remember for next iteration.
					
					dec	bx								; Decrement y.
					jnz	BRunMainLoop				; If not zero, loop.

				BRunMainLoopDone:
					mov	pCurFlxBuf, esi			; Restore file ptr.
				}
				
				pfile->Seek((long)(pCurFlxBuf - pfile->GetMemory()), SEEK_SET);

		#endif	// def WIN32
		
	// just return if error has been set
	if (sError == 1)
		return sError;
	
	// Set modified flag
	*psPixelsModified = TRUE;

	return 0;
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
// (static)
//
///////////////////////////////////////////////////////////////////////////////
short CRamFlx::ReadDataLC(	CImage* pimageRead, CNFile* pfile, 
									short* psPixelsModified)
	{
	//assert(pimageRead->pbPixels != NULL);
	//assert(pimageRead->sPitch > 0);
	// just return with error instead of asserting
	if ((pimageRead->pData == NULL) || (pimageRead->lPitch <= 0))
		return 1;
	
	short y;
	short lines;
	UCHAR* pbRow;

	// The first word specifies the starting y (another way of looking at it
	// is the number of lines that are unchanged from the previous image).
//	y = MEM_WORD(m_pCurFlxBuf);	
	pfile->Read(&y);

	// Init row pointer to point at start of specified row
	pbRow = (UCHAR*)pimageRead->pData + ((ULONG)y * (ULONG)pimageRead->lPitch);

	// The second word specifies the number of lines in this chunk.
//	lines = MEM_WORD(m_pCurFlxBuf);	
	pfile->Read(&lines);

	// Let's check to see if the pixels are modified from the previous frame by
	// checking the number of delta lines.  If the number of delta lines is zero, then
	// we know that there is no delta.
	if (lines < 1)
		{
		// Set to unmodified.
		*psPixelsModified = FALSE;
		}
	else
		{
		// Set modified flag
		*psPixelsModified = TRUE;
		}

	#ifndef WIN32
		UCHAR	bVal;
		S8		cVal;
		short sCount;
		UCHAR* pbPix;
		short packets;

		while (lines > 0)
			{
			// Set pixel pointer to start of row
			pbPix = pbRow;
		
	// For debugging, prefetch a bunch of values to view them in the debugger
	#if 0
	long lPos = pfile->tellg();
	static UCHAR bData[100];
	pfile->read(bData, sizeof(bData));
	pfile->seekg(lPos);
	#endif

			// The first byte for each line is the number of packets.
			// This can be 0, which indicates no changes on that line.
//			bVal = MEM_BYTE(m_pCurFlxBuf);	
			pfile->Read(&bVal);
			packets = (short)bVal;
		
			while (packets > 0)
				{
				// The first byte of each packet is a column skip.
				// Adjust pixel pointer to skip that number of pixels.
//				bVal = MEM_BYTE(m_pCurFlxBuf);	
				pfile->Read(&bVal);
				pbPix = pbPix + (ULONG)bVal;
  			
				// Second byte of each packet is type/size.  If bit 7 is 0, bits 6-0
				// are number of pixels to be copied.  If bit 7 is 1, bits 6-0 are
				// the number of times to replicate a single pixel.
//				cVal = MEM_BYTE(m_pCurFlxBuf);	
				pfile->Read(&cVal);
	//				assert(cVal != 0);	// Not sure how to handle 0, so stop if it comes up!
				if (cVal == 0)
					cVal = 0;
				
				sCount = (short)cVal;
				if (sCount > 0)
					{
//					memcpy(pbPix,m_pCurFlxBuf,sCount);	
//					m_pCurFlxBuf = MEM_SEEK(m_pCurFlxBuf,sCount);
					pfile->Read(pbPix, sCount);
					pbPix += (ULONG)sCount;
					}
				else
					{
					sCount = -sCount;
//					bVal = MEM_BYTE(m_pCurFlxBuf);	
					pfile->Read(&bVal);
					memset(pbPix, (int)bVal, (size_t)sCount);
					pbPix += (ULONG)sCount;
					}
				
				// Adjust remaining packets
				packets--;
				}

			// Move row pointer to start of next row
			pbRow += (ULONG)pimageRead->lPitch;
		
			// Adjust remaining lines
			lines--;
			}
	#else
		// Locals needed (b/c we can't use this->).
		long		lPitch		= pimageRead->lPitch;
		MEM		pCurFlxBuf	= pfile->GetMemory() + pfile->Tell();
		
		__asm
			{
				mov	esi, pCurFlxBuf			; File ptr.
				mov	dx, lines					; Number of lines to process.
				cmp	dx, 0						; If lines == 0,
				je		LCMainLoopDone				; nothing to do.

				// Set pixel pointer to start of row
				mov	edi, pbRow					; pbPix = pbRow;
				xor	ebx, ebx
				xor	ecx, ecx			

			LCMainLoop:								; while (lines > 0)
				// The first byte for each line is the number of packets.
				// This can be 0, which indicates no changes on that line.
				AMEM_BYTE(ah, esi)				; bVal = MEM_BYTE(m_pCurFlxBuf);	
				
				cmp	ah, 0							; If packets <= 0
				jle	LCPacketLoopDone			; done.
		
				; packets = (short)bVal;

			LCPacketLoop:							; while (packets > 0)
				// The first byte of each packet is a column skip.
				// Adjust pixel pointer to skip that number of pixels.
				AMEM_BYTE(bl, esi)				; bVal = MEM_BYTE(m_pCurFlxBuf);	
				add	edi, ebx						; pbPix = pbPix + (ULONG)bVal;
  			
				// Second byte of each packet is type/size.  If bit 7 is 0, bits 6-0
				// are number of pixels to be copied.  If bit 7 is 1, bits 6-0 are
				// the number of times to replicate a single pixel.
				AMEM_BYTE(cl, esi)				; cVal = MEM_BYTE(m_pCurFlxBuf);	
	//				assert(cVal != 0);	// Not sure how to handle 0, so stop if it comes up!
				
				; sCount = (short)cVal;

				cmp	cl, 0							; if (sCount < 0)
				jl		LCRepeat						; handle repeat pixel
														; else handle uniques
				; memcpy(pbPix,m_pCurFlxBuf,sCount);	
				; m_pCurFlxBuf = MEM_SEEK(m_pCurFlxBuf,sCount);
				; pbPix += (ULONG)sCount;

				rep movs	byte ptr [edi], byte ptr [esi]	; Copy by bytes.  Does nothing if cx is 0.
				
				jmp	LCNextPacket
			
			LCRepeat:
				neg	cl								; sCount = -sCount;
				// Get repeat pixel.
				AMEM_BYTE(al, esi)				; bVal = MEM_BYTE(m_pCurFlxBuf);	
				; memset(pbPix, (int)bVal, (size_t)sCount);
				; pbPix += (ULONG)sCount;
				rep stos	byte ptr [edi]			; Set pixels.  Does nothing if cx is 0.

			LCNextPacket:				
				// Adjust remaining packets
				dec	ah								; packets--;
				jnz	LCPacketLoop				; If not zero, continue looping.

			LCPacketLoopDone:
				// Move row pointer to start of next row
				mov	edi,	pbRow
				add	edi,	lPitch				; pbRow += (ULONG)pimageRead->lPitch;
				mov	pbRow, edi					; Remember for next iteration.

				// Adjust remaining lines
				dec	dx								; lines--;
				jnz	LCMainLoop					; If not zero, continue looping.

			LCMainLoopDone:

				mov	pCurFlxBuf, esi			; Restore file ptr.
			}

		pfile->Seek((long)(pCurFlxBuf - pfile->GetMemory()), SEEK_SET);
		#endif // ndef WIN32

	return 0;
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
// (static)
//
///////////////////////////////////////////////////////////////////////////////
short CRamFlx::ReadDataSS2(CImage* pimageRead, CNFile* pfile, 
									short* psPixelsModified)
	{
	//assert(pimageRead->pbPixels != NULL);
	//assert(pimageRead->sPitch > 0);
	// just return with error instead of asserting
	if ((pimageRead->pData == NULL) || (pimageRead->lPitch <= 0))
		return 1;
	
	short lines;
	short packets;
	UCHAR	byLastByte;
	short	bLastByte = FALSE;

	// The first word specifies the starting y (another way of looking at it
	// is the number of lines that are unchanged from the previous image).
	//	pfile->Read(&y, 2); not in FLI_SS2

	// The first word specifies the number of lines in this chunk.
//	lines = MEM_WORD(m_pCurFlxBuf);	
	pfile->Read(&lines);
	
	// Let's check to see if any actual delta is being processed and set the 
	// pixels modified flag accordingly.
	if (lines < 1)
		{
		// Make sure the modified flag is FALSE.
		*psPixelsModified = FALSE;
		}
	else
		{
		// Set modified flag to true.
		*psPixelsModified = TRUE;
		}
	
#ifndef WIN32		
	UCHAR bVal;
	S8		cVal;
	USHORT wVal;
	short sCount;
	short y;
	UCHAR* pbPix;

	// Start at line 0
	y = 0;

	while (lines > 0)
		{
		// The first word for "each line can begin with some optional words
		// that are used to skip lines and set the last byte in the line for
		// animations with odd widths."
		do
			{
//			wVal = MEM_WORD(m_pCurFlxBuf);	
			pfile->Read(&wVal);
			
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
//					wVal = MEM_WORD(m_pCurFlxBuf);	
					pfile->Read(&wVal);
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
		pbPix = (UCHAR*)pimageRead->pData + ((ULONG)y * (ULONG)pimageRead->lPitch);
		
		while (packets > 0)
			{
			// The first byte of each packet is a column skip.
			// Adjust pixel pointer to skip that number of pixels.
//			bVal = MEM_BYTE(m_pCurFlxBuf);	
			pfile->Read(&bVal);
			pbPix = pbPix + (ULONG)bVal;
			
			// Second byte of each packet is type/size.  If bit 7 is 0, bits 6-0
			// are number of pixel pairs to be copied.  If bit 7 is 1, bits 6-0 are
			// the number of times to replicate a single pixel pair.
//			cVal = MEM_BYTE(m_pCurFlxBuf);	
			pfile->Read(&cVal);
				
			sCount = (short)cVal;
			if (sCount > 0)
				{
				sCount *= sizeof(USHORT);
//				memcpy(pbPix, m_pCurFlxBuf, sCount);	
//				m_pCurFlxBuf = MEM_SEEK(m_pCurFlxBuf,sCount);
				pfile->Read(pbPix, sCount);
				pbPix += (ULONG)(sCount);
				}
			else
				{
				sCount = (short)-sCount;
//				wVal = MEM_WORD(m_pCurFlxBuf);	
				pfile->Read(&wVal);
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
			pbPix = (UCHAR*)pimageRead->pData + (((ULONG)y + 1L) * (ULONG)pimageRead->lPitch) - 1L;
			// Set pixel at end of row.
			*pbPix = byLastByte;
			bLastByte = FALSE;
			}

		// Adjust remaining lines
		lines--;
		
		// Go to next line
		y++;
		}
#else // ifdef WIN32
	// Need a local for this-> stuff.
	MEM	pCurFlxBuf	= pfile->GetMemory() + pfile->Tell();
	void*	pData			= pimageRead->pData;
	long	lPitch		= pimageRead->lPitch;
	long	lWidth		= pimageRead->lWidth;

	__asm
		{
		cmp	lines, 0						; If lines == 0
		je		SS2MainLoopDone			; Done

		cld									; clear direction flag.
		mov	esi, pCurFlxBuf			; Get file ptr.
		xor	ebx, ebx						; Clear so we can use high 16 bits for match.
		xor	edx, edx						; Clear so we can use high 16 bits for match.

	SS2MainLoop:			
		// The first word for "each line can begin with some optional words
		// that are used to skip lines and set the last byte in the line for
		// animations with odd widths."

	OptionalPacketsLoop:
		AMEM_WORD(ax, esi)				; wVal = MEM_WORD(m_pCurFlxBuf);	
	
	// "The high order two bits of the word is used to determine the
	// contents of the word."
		test	ah, 0C0H
	// If 0 0, the word contains the packet count; the packets follow 
	// this word.  This is our signal to stop processing "optional
	// words".
		jz		OptionalPacketsDone		; If 0, we are done.

		test	ah, 040H						; If not in ah then al contains
		jz		StoreLastByte				; contains byte for end of line.

	// 1 1 "The word contains a line skip count.  The number of
	// lines skipped is given by the absolute value of the word.
	// This is NOT a signal to stop processing "optional words".
		// Skip abs(wVal) lines
		neg	ax								; Make positive.
		add	bx, ax						; y += -((short)wVal);

		// Continue
		jmp OptionalPacketsLoop

	StoreLastByte:
	// 1 0 "The low order byte is to be stored in the last byte of
	// the current line.  The packet count always folows this word."
	// This is another signal to stop processing "optional words".
		mov	byLastByte, al				; byLastByte = (UCHAR)(wVal & (USHORT)0x00ff);
		mov	bLastByte, TRUE			; bLastByte = TRUE;
		// Read the packet count.
		AMEM_WORD(ax, esi)				; wVal = MEM_WORD(m_pCurFlxBuf);	
	
	OptionalPacketsDone:

		// The packet count should now be in wVal (ax).
		mov	packets, ax					; packets = (short)wVal;
							
		// Init pointer to point at start of specified row
		;pbPix = (UCHAR*)pimageRead->pData + ((ULONG)y * (ULONG)pimageRead->lPitch);

		mov	eax, lPitch					; pimageRead->lPitch
		mul	ebx							; Multiply by y.  Counting on edx getting 0000.
		mov	edi, eax						; Copy result.
		add	edi, pData					; pimageRead->pData

		// If nothing to do . . .
		cmp	packets, 0					; If packets == 0
		je		SS2PacketLoopDone			; done.

	SS2PacketLoop:
		
		// Make sure everything in edx is clear.
		xor	edx, edx	

		// The first byte of each packet is a column skip.
		// Adjust pixel pointer to skip that number of pixels.
		AMEM_BYTE(dl, esi)				; bVal = MEM_BYTE(pCurFlxBuf);	
		add	edi, edx						; pbPix = pbPix + (ULONG)bVal;

		xor	ecx, ecx						; Clear ecx just in case.
			
		// Second byte of each packet is type/size.  If bit 7 is 0, bits 6-0
		// are number of pixel pairs to be copied.  If bit 7 is 1, bits 6-0 are
		// the number of times to replicate a single pixel pair.
		AMEM_BYTE(cl, esi)				; cVal = MEM_BYTE(pCurFlxBuf);	
		
		// cl contains cVal/sCount.		
		; sCount = (short)cVal
		cmp	cl, 0							; if (sCount > 0)
		jle	SS2Repeat					; handle repeat packets,
												; else, uniques
		// Setup copy
		shr	cx, 1							; by dwords.
												
		; memcpy(pbPix, pCurFlxBuf, sCount);	
		; pCurFlxBuf = MEM_SEEK(pCurFlxBuf,sCount);
		; pbPix += (ULONG)(sCount);
		rep movs	dword ptr [edi], dword ptr [esi]	; Copy by dwords.  Does nothing if cx is 0.

		jnc	SS2RunDone								; If no carry from shr
		movs	word ptr [edi], word ptr [esi]	; Copy last word.

		jmp	SS2RunDone
	
	SS2Repeat:
		neg	cl								; sCount = (short)-sCount;

		// Setup memset.
		// Get repeat pixel.
		// NOTE: High part of eax could be dirty from multiplies, but we
		// shift that out.
		AMEM_WORD(ax, esi)				; wVal = MEM_WORD(pCurFlxBuf);
		; Here, for more speed, we copy by dword, so we duplicate ax in eax high
		mov	dx, ax						; copy ax so we can put it back.
		shl	eax, 16						; shift ax into eax high
		mov	ax, dx						; put ax back into eax low

		shr	cx, 1							; by dwords.
		
		; USHORT* pwPix = (USHORT*)pbPix;
		; for (short i = 0; i < sCount; i++)
		; 	*pwPix++ = wVal;
		; pbPix = (UCHAR*)pwPix;
		rep stos	dword ptr [edi]		; Store.  If cx == 0, skipped.

		jnc	SS2RunDone					; If there was no carry from the shift right
		stos word ptr [edi]				; Store last word.
	
	SS2RunDone:

		// Adjust remaining packets
		dec	packets					; packets--;
		jnz	SS2PacketLoop			; Loop
	
	SS2PacketLoopDone:
		
		// Place last byte if specified.
		cmp	bLastByte, FALSE				; if (bLastByte == FALSE)
		je		NoLastByte						; don't copy last byte.

		// Get pointer to end of this row.
		; pbPix	= (UCHAR*)pimageRead->pData 
		;				+ ((long)y * pimageRead->lPitch) 
		;				+ pimageRead->lWidth - 1L;

		mov	eax, lPitch					; pimageRead->lPitch
		mul	ebx							; Multiply by y.  Counting on edx getting 0000.
		mov	edi, eax						; Copy result.
		add	edi, lWidth					; pimageRead->lWidth
		dec	edi							; -1L
		add	edi, pData					; pimageRead->pData

		// Set pixel at end of row.
		; *pbPix = byLastByte;
		mov	al, byLastByte				; Get byLastByte
		mov	byte ptr [edi], al		; Store it.
		mov	bLastByte, FALSE			; bLastByte = FALSE;

	NoLastByte:

		
		// Go to next line
		inc	bx								; y++;
		// Adjust remaining lines
		dec	lines							; lines--
		jnz	SS2MainLoop				

SS2MainLoopDone:
		// Put back for C++ code.
		mov	pCurFlxBuf, esi			; Restore file ptr.
		}

	pfile->Seek((long)(pCurFlxBuf - pfile->GetMemory()), SEEK_SET);

#endif // ndef WIN32

	return 0;
	}
	
	
///////////////////////////////////////////////////////////////////////////////
//
// Helper function that reads the header of the flic file.  Always returns
// with file position at start of frame 1.
//
///////////////////////////////////////////////////////////////////////////////
short CRamFlx::ReadHeader(CNFile* pfile)
	{
	// Seek to start of file
	pfile->Seek(0, SEEK_SET);
			
	// Read the part of the file header that's common to FLC and FLI files.
	pfile->Read(&m_filehdr.lEntireFileSize);
	pfile->Read(&m_filehdr.wMagic);
	pfile->Read(&m_filehdr.sNumFrames);
	pfile->Read(&m_filehdr.sWidth);
	pfile->Read(&m_filehdr.sHeight);
	pfile->Read(&m_filehdr.sDepth);
	pfile->Read(&m_filehdr.sFlags);

	// The headers become different at this point
	if (m_filehdr.wMagic == FLX_MAGIC_FLC)
		{
		// Read the remainder of the FLC header
		pfile->Read(&m_filehdr.lMilliPerFrame);
		pfile->Read(&m_filehdr.sReserveA);
		pfile->Read(&m_filehdr.dCreatedTime);
		pfile->Read(&m_filehdr.dCreator);
		pfile->Read(&m_filehdr.dUpdatedTime);
		pfile->Read(&m_filehdr.dUpdater);
		pfile->Read(&m_filehdr.sAspectX);
		pfile->Read(&m_filehdr.sAspectY);
		pfile->Read(m_filehdr.bReservedB, sizeof(m_filehdr.bReservedB));
		pfile->Read(&m_filehdr.lOffsetFrame1);
		pfile->Read(&m_filehdr.lOffsetFrame2);
		pfile->Read(m_filehdr.bReservedC, sizeof(m_filehdr.bReservedC));
				
		// In FLC files, an optional prefix chunk may follow the header.
		// According to Autodesk it contains information not related to
		// animation playback and should be ignored.  They also say that
		// programs other than Animator Pro shouldn't write it.
		// We currently ignore this chunk.  In the future, we might
		// consider preserving it in case this flic is written out to a
		// new file.  The easiest way to skip over it is to seek directly
		// to the first frame using the offset specified in the header.
		// Seek directly to first frame.
		pfile->Seek(m_filehdr.lOffsetFrame1, SEEK_SET);
		}
	else
		{
		// Read the FLI's jiffies (a jiffy is 1/70th second) and convert to
		// to FLC's milliseconds.
		short sJiffies;
		pfile->Read(&sJiffies);
		m_filehdr.lMilliPerFrame = (long)( (double)sJiffies * ((double)1000 / (double)70L) + (double)0.5 );
		
		// Set times to 0 for lack of better value (some day, we could read the
		// file's date and time stamp and put it here).  We use "FLIB" for the
		// serial numbers, which is safe according to the doc's.
		m_filehdr.dCreatedTime	= 0;
		m_filehdr.dCreator		= 0x464c4942;
		m_filehdr.dUpdatedTime	= 0;
		m_filehdr.dUpdater		= 0x464c4942;
		
		// Aspect ratio for 320x200 (which is the only FLI size) is 6:5
		m_filehdr.sAspectX		= 6;
		m_filehdr.sAspectY		= 5;
		
		// Skip to end of header.  This is also the starting position of
		// frame 1, which we save in the header.
		pfile->Seek(128, SEEK_SET);
		m_filehdr.lOffsetFrame1 = pfile->Tell();
		
		// Get size of frame 1's chunk in order to calculate the starting
		// position of frame 2.
		long lSizeFrame1;
		pfile->Read(&lSizeFrame1);
		m_filehdr.lOffsetFrame2 = m_filehdr.lOffsetFrame1 + lSizeFrame1;
		
		// Seek to start of frame 1
		pfile->Seek(m_filehdr.lOffsetFrame1, SEEK_SET);
		}

	// Allocate space for RAM buffer
	short sError=0;
 	long lSizeFile = m_filehdr.lEntireFileSize - m_filehdr.lOffsetFrame1;

	if ((m_pucFlxBuf = (UCHAR*)malloc(lSizeFile)) != NULL)
		{
		pfile->Read(m_pucFlxBuf, lSizeFile);
//		m_pCurFlxBuf = MEM_OPEN(m_pFlxBuf);
		// Adjust frame offsets
		m_filehdr.lOffsetFrame2 -= m_filehdr.lOffsetFrame1;
		m_filehdr.lOffsetFrame1	= 0;

		m_file.Open(m_pucFlxBuf, lSizeFile, ENDIAN_LITTLE);
		}
	else sError = -1;
		
	// If good then return success, otherwise return error.
	if (pfile->Error() == FALSE && sError == 0)
		return 0;
	else
		return 1;

	return sError;
	}
	
	
///////////////////////////////////////////////////////////////////////////////
//
// Helper function that clears file header.
//
///////////////////////////////////////////////////////////////////////////////
void CRamFlx::ClearHeader(void)
	{	
	// Clear all fields in file header
	short i;
	m_filehdr.lEntireFileSize	= 0;
	m_filehdr.wMagic				= 0;
	m_filehdr.sNumFrames			= 0;
	m_filehdr.sWidth				= 0;
	m_filehdr.sHeight				= 0;
	m_filehdr.sDepth				= 0;
	m_filehdr.sFlags				= 0;
	m_filehdr.lMilliPerFrame	= 0;
	m_filehdr.sReserveA			= 0;
	m_filehdr.dCreatedTime		= 0;
	m_filehdr.dCreator			= 0;
	m_filehdr.dUpdatedTime		= 0;
	m_filehdr.dUpdater			= 0;
	m_filehdr.sAspectX			= 0;
	m_filehdr.sAspectY			= 0;
	for (i = 0; i < sizeof(m_filehdr.bReservedB); i++)
		m_filehdr.bReservedB[i]	= 0;
	m_filehdr.lOffsetFrame1		= 0;
	m_filehdr.lOffsetFrame2		= 0;
	for (i = 0; i < sizeof(m_filehdr.bReservedC); i++)
		m_filehdr.bReservedC[i]	= 0;
	}
	
	
///////////////////////////////////////////////////////////////////////////////
//
// Helper functions to deal with memory associated with buf's.
//
///////////////////////////////////////////////////////////////////////////////
void CRamFlx::InitBuf(CImage* pimage)
	{
	// The pointers MUST be cleared to NULL so we can tell later on whether
	// any memory needs to be freed.
	pimage->pData				= NULL;
	if (pimage->pPalette != NULL)
		{
		pimage->pPalette->pData	= NULL;
		}
	}
	
	
short CRamFlx::AllocBuf(CImage* pimage, long lWidth, long lHeight, short sColors)
	{
	short sError=0;
	
	// Allocate image buffer for pixels & set pitch to width
	pimage->ulType		= FLX8_888;
	pimage->lWidth		= lWidth;
	pimage->lHeight	= lHeight;
	pimage->lPitch		= DWORDALIGN(lWidth);
	pimage->ulSize		= pimage->lPitch * lHeight;
	pimage->sDepth		= 8;
	sError = pimage->CreateData(pimage->ulSize);	// Allocate image space, returns 0 on success
	if (sError == 0)
		{
		if (pimage->CreatePalette(sColors * CPal::GetPalEntrySize(PFLX)) == 0)
			{
			// Allocate palette buffer for colors 
			pimage->pPalette->ulType			= PFLX;
			pimage->pPalette->sNumEntries		= sColors;
			pimage->pPalette->sPalEntrySize	= CPal::GetPalEntrySize(PFLX);
			}
		else
			{
			sError = 1;
			}
		}
	
	// If it worked then return success
	if (sError == 0)
		return 0;
	// Else free anything that did get allocated and return failure
	else
		{
		FreeBuf(pimage);
		return 1;
		}
	}
	
	
void CRamFlx::FreeBuf(CImage* pimage)
	{ 
	if (pimage != NULL)
		{
		pimage->DestroyData();
		
		if (pimage->pPalette != NULL)
			{
			pimage->DestroyPalette();
			}
		}
	}
	
	
void CRamFlx::CopyBuf(CImage* pimageDst, CImage* pimageSrc)
	{ 
	// Copy initial CImage member variables
	pimageDst->ulType		= pimageSrc->ulType;
	pimageDst->ulSize		= pimageSrc->ulSize;
	pimageDst->lWidth		= pimageSrc->lWidth;
	pimageDst->lHeight	= pimageSrc->lHeight;
	pimageDst->lPitch		= pimageSrc->lPitch;
	pimageDst->sDepth		= pimageSrc->sDepth;
	
	// Copy pixels one row at a time
	UCHAR* pbSrc	= (UCHAR*)pimageSrc->pData;
	UCHAR* pbDst	= (UCHAR*)pimageDst->pData;
	
	for (short y = 0; y < pimageSrc->lHeight; y++)
		{
		memcpy(pbDst, pbSrc, pimageSrc->lWidth);
		pbSrc += (ULONG)pimageSrc->lPitch;
		pbDst += (ULONG)pimageDst->lPitch;
		}                                          
		                             
   // Copy initial CPal member variables
   pimageDst->pPalette->ulType			= pimageSrc->pPalette->ulType;
   pimageDst->pPalette->ulSize			= pimageSrc->pPalette->ulSize;
   pimageDst->pPalette->sStartIndex		= pimageSrc->pPalette->sStartIndex;
   pimageDst->pPalette->sNumEntries		= pimageSrc->pPalette->sNumEntries;
	pimageDst->pPalette->sPalEntrySize	= pimageSrc->pPalette->sPalEntrySize;
			                             
	// Copy colors
	memcpy(	pimageDst->pPalette->pData, pimageSrc->pPalette->pData, 
				pimageDst->pPalette->sNumEntries * pimageDst->pPalette->sPalEntrySize);
	}

/////////////////////////////////////////////////////////////////////////////////////
//	CreateFramePointers
//
//	CreateFramePointers creates a list of byte pointers
//	These byte pointers point to each frame within
//  the flic memory file (frame 1 to n).  To retrieve a frame, 
//	seek m_file to the frame position wanted and call DoReadFrame.  
//  
//	These frame positions can only be used with a flic containing no delta compression
/////////////////////////////////////////////////////////////////////////////////////
short CRamFlx::CreateFramePointers(void)
	{
	short sError = 0;
	long  lSizeFrame;

	// Allocate the space for the frame pointers
	if ((m_plFrames = (long*)malloc((m_filehdr.sNumFrames+1) * sizeof(long))) == NULL)
		sError = -1;
	else
		{
		// Assign all the frame pointers
		for (short sFrame = 1; sFrame <= m_filehdr.sNumFrames; sFrame++)
			{
			// Set pointer to frame
			m_plFrames[sFrame]	= m_file.Tell();
			
			// Get size of frame
//			lSizeFrame = MEM_DWORD(m_pCurFlxBuf);
			m_file.Read(&lSizeFrame);
			// Seek to beginning of next frame
//			m_pCurFlxBuf = MEM_SEEK(m_pCurFlxBuf,lSizeFrame - sizeof(ULONG)); 
			m_file.Seek(lSizeFrame - sizeof(ULONG), SEEK_CUR);
			}
		}
		 
	return sError;
	}

///////////////////////////////////////////////////////////////////////////////
// CImage/CPal FLX8_888/PFLX link late functions.
///////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
//
// Convert to FLX8_888 format from a standard format.
// Returns FLX8_888 on success; NOT_SUPPORTED otherwise.
//
/////////////////////////////////////////////////////////////////////////////////////
static short ConvToFlx8_888(CImage* pImage)
	{
	short	sRes	= NOT_SUPPORTED;	// Assume error.

	ASSERT(pImage	!= NULL);

	switch (pImage->ulType)
		{
		case BMP8:
			// The only difference from BMP8 is the palette.
			if (pImage->pPalette != NULL)
				{
				if (pImage->pPalette->ulType == PDIB)
					{
					CPal*	ppal	= pImage->pPalette;
					// Detatch the original palette data from the Image's palette.
					IM_RGBQUAD* prgbq	= (IM_RGBQUAD*)ppal->DetatchData();

					// Create new palette data for the pImage
					if (ppal->CreateData((ppal->sStartIndex + ppal->sNumEntries) * CPal::GetPalEntrySize(PFLX)) == 0)
						{
						PRGBT8	prgbt		= (PRGBT8)ppal->pData;
						for (short sIndex	= ppal->sStartIndex; sIndex < ppal->sStartIndex + ppal->sNumEntries; sIndex++)
							{
							prgbt[sIndex].ucRed		= prgbq[sIndex].rgbRed;
							prgbt[sIndex].ucGreen	= prgbq[sIndex].rgbGreen;
							prgbt[sIndex].ucBlue		= prgbq[sIndex].rgbBlue;
							}

						// Set new parameters.
						ppal->ulType			= PFLX;
						ppal->sPalEntrySize	= CPal::GetPalEntrySize(ppal->ulType);
						pImage->ulType			= FLX8_888;
						sRes						= FLX8_888;
						}
					else
						{
						TRACE("ConvToFlx8_888(): Unable to allocate new buffer.\n");
						}

					// If successful . . .
					if (sRes != NOT_SUPPORTED)
						{
						// Destroy the old palette data.
						CImage::DestroyDetatchedData((void**)&prgbq);
						}
					else
						{
						// Restore the palette data.
						ppal->pData	= (UCHAR*)prgbq;
						}
					}
				else
					{
					TRACE("ConvToFlx8_888(): Palette is not BMP8:PDIB.\n");
					}
				}
			else
				{
				TRACE("ConvToFlx8_888(): No palette!?\n");
				}
			break;

		default:
			// All others not supported.
			break;
		}

	return sRes;
	}

/////////////////////////////////////////////////////////////////////////////////////
//
// Convert from FLX8_888 format to a standard format.
// Returns new format on success; NOT_SUPPORTED otherwise.
//
/////////////////////////////////////////////////////////////////////////////////////
static short ConvFromFlx8_888(CImage* pImage)
	{
	short	sRes	= NOT_SUPPORTED;	// Assume error.

	ASSERT(pImage != NULL);
	ASSERT(pImage->ulType == FLX8_888);

	// Convert to BMP8.

	// The only difference from BMP8 is the palette.
	if (pImage->pPalette != NULL)
		{
		if (pImage->pPalette->ulType == PFLX)
			{
			CPal*		ppal	= pImage->pPalette;
			// Detatch the original palette data from the Image's palette.
			PRGBT8	prgbt	= (PRGBT8)ppal->DetatchData();

			// Create new palette data for the pImage
			if (ppal->CreateData((ppal->sStartIndex + ppal->sNumEntries) * CPal::GetPalEntrySize(PDIB)) == 0)
				{
				IM_RGBQUAD* prgbq	= (IM_RGBQUAD*)ppal->pData;
				for (short sIndex	= ppal->sStartIndex; sIndex < ppal->sStartIndex + ppal->sNumEntries; sIndex++)
					{
					prgbq[sIndex].rgbRed  	= prgbt[sIndex].ucRed;
					prgbq[sIndex].rgbGreen	= prgbt[sIndex].ucGreen;	
					prgbq[sIndex].rgbBlue 	= prgbt[sIndex].ucBlue;
					}

				// Set new parameters.
				ppal->ulType			= PDIB;
				ppal->sPalEntrySize	= CPal::GetPalEntrySize(ppal->ulType);
				pImage->ulType			= BMP8;
				sRes						= BMP8;
				}
			else
				{
				TRACE("ConvFromFlx8_888(): Unable to allocate new buffer.\n");
				}

			// If successful . . .
			if (sRes != NOT_SUPPORTED)
				{
				// Destroy the old palette data.
				CImage::DestroyDetatchedData((void**)&prgbt);
				}
			else
				{
				// Restore the palette data.
				ppal->pData	= (UCHAR*)prgbt;
				}
			}
		else
			{
			TRACE("ConvFromFlx8_888(): Palette is not FLX8_888:PFLX.\n");
			}
		}
	else
		{
		TRACE("ConvFromFlx8_888(): No palette!?\n");
		}

	return sRes;
	}

///////////////////////////////////////////////////////////////////////////////
// EOF
///////////////////////////////////////////////////////////////////////////////
