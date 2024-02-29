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
// RTVIDC.CPP
// 
// History:
//		11/13/95 JMI	Started.
//
//////////////////////////////////////////////////////////////////////////////
//
// This class is designed to receive VIDC data in real time and decompress it
// to a specific buffer or buffers on a channel basis.
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// C Headers.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Blue Headers.
//////////////////////////////////////////////////////////////////////////////
#include "System.h"
#include "bdebug.h"

//////////////////////////////////////////////////////////////////////////////
// Green Headers.
//////////////////////////////////////////////////////////////////////////////
#include "rttypes.h"
#include "rtvidc.h"

//////////////////////////////////////////////////////////////////////////////
// Orange Headers.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Yellow Headers.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Module specific macros.
//////////////////////////////////////////////////////////////////////////////
// Types of chunks.
#define VIDC_CHUNK_HEADER	RT_FLAG_INIT
#define VIDC_CHUNK_PALETTE	RT_FLAG_USER1

//////////////////////////////////////////////////////////////////////////////
// Module specific typedefs.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Module specific (static) variables.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Construction/Destruction Functions.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Default constructor.
//
//////////////////////////////////////////////////////////////////////////////
CRtVidc::CRtVidc()
	{
	Set();
	}

//////////////////////////////////////////////////////////////////////////////
//
// Destructor.
//
//////////////////////////////////////////////////////////////////////////////
CRtVidc::~CRtVidc()
	{
	Reset();
	}

//////////////////////////////////////////////////////////////////////////////
// Internal Functions.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Sets variables w/o regard to current values.
//
//////////////////////////////////////////////////////////////////////////////
void CRtVidc::Set(void)
	{
	m_pdispatch		= NULL;
	for (short i = 0; i < MAX_VID_CHANNELS; i++)
		{
		m_avidchdrs[i].sNumFrames		= 0;
		m_avidchdrs[i].pImage			= NULL;
		m_avidchdrs[i].callbackHeader	= NULL;
		m_avidchdrs[i].callbackBefore	= NULL;
		m_avidchdrs[i].callbackAfter	= NULL;
		m_avidchdrs[i].hic				= NULL;
		}
	}

//////////////////////////////////////////////////////////////////////////////
//
// Resets variables.  Performs deallocation if necessary.
//
//////////////////////////////////////////////////////////////////////////////
void CRtVidc::Reset(void)
	{
	}

//////////////////////////////////////////////////////////////////////////////
//
// Reads a BITMAPINFO from the specified CNFile into the provided data.
// Returns number of bytes read on success, negative on error.
//
//////////////////////////////////////////////////////////////////////////////
long ReadBitmapInfo(	BITMAPINFO*	pbmi, CNFile* pfile)
	{
	// Remember current position.
	long	lPos	= pfile->Tell();

	// Write BITMAPINFO.
	pfile->Read(&pbmi->bmiHeader.biSize);
	pfile->Read(&pbmi->bmiHeader.biWidth);
	pfile->Read(&pbmi->bmiHeader.biHeight);
	pfile->Read(&pbmi->bmiHeader.biPlanes);
	pfile->Read(&pbmi->bmiHeader.biBitCount);
	pfile->Read(&pbmi->bmiHeader.biCompression);
	pfile->Read(&pbmi->bmiHeader.biSizeImage);
	pfile->Read(&pbmi->bmiHeader.biXPelsPerMeter);
	pfile->Read(&pbmi->bmiHeader.biYPelsPerMeter);
	pfile->Read(&pbmi->bmiHeader.biClrUsed);
	pfile->Read(&pbmi->bmiHeader.biClrImportant);

	// pfile->Read color info, if any.
	// Get number of bytes left.
	long	lNumColors	= pbmi->bmiHeader.biSize - sizeof(pbmi->bmiHeader);
	// Must be complete RGBQUADs.
	ASSERT(lNumColors % 4 == 0);
	// Convert to number of colors left.
	lNumColors	/= 4L;
	for (long l = 0; l < lNumColors; l++)
		{
		pfile->Read(&pbmi->bmiColors[l].rgbBlue);
		pfile->Read(&pbmi->bmiColors[l].rgbGreen);
		pfile->Read(&pbmi->bmiColors[l].rgbRed);
		pfile->Read(&pbmi->bmiColors[l].rgbReserved);
		}

	return pfile->Tell() - lPos;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Decompresses a VIDC frame using the opened decompressor.
// Returns 0 on success.
//
//////////////////////////////////////////////////////////////////////////////
short CRtVidc::DecompressFrame(	PVIDC_RT_HDR pvidchdr, CNFile* pfile, 
											ULONG ulFlags, PBMI pbmiIn, PBMI pbmiOut)

	{
	short	sRes	= 0;	// Assume success.

	// Get data in compression native format.
	BMI	bmiTempOut = *pbmiIn;
	bmiTempOut.bmiHeader.biCompression	= BI_RGB;

	// Attempt to decompress . . .
//	if (ICDecompress(	pvidchdr->hic, ulFlags, 
//							(BITMAPINFOHEADER*)pbmiIn, pfile->GetMemory() + pfile->Tell(),
//							(BITMAPINFOHEADER*)pbmiOut, pvidchdr->pImage->pData)
//		== ICERR_OK)
	HBITMAP	hbm	= (HBITMAP)ICImageDecompress(	pvidchdr->hic, 0L, (BITMAPINFO*)pbmiIn,
																pfile->GetMemory() + pfile->Tell(),
																(BITMAPINFO*)&bmiTempOut);
	if (hbm != NULL)
		{
		// Successfully decompressed.
		HDC	hDC	= GetDC(NULL);
		if (hDC != NULL)
			{
			pbmiOut->bmiHeader.biSize			= sizeof(pbmiOut->bmiHeader);
			pbmiOut->bmiHeader.biSizeImage	= 0;
			// Reformat.
			if (GetDIBits(	hDC, hbm, 0, pvidchdr->pImage->lHeight,
								pvidchdr->pImage->pData, (BITMAPINFO*)&pbmiOut, DIB_RGB_COLORS) != FALSE)
				{
				// Success.
				}
			else
				{
				TRACE("DecompressFrame(): GetDIBits failed.\n");
				sRes = -3;
				}

			ReleaseDC(NULL, hDC);
			}
		else
			{
			TRACE("DecompressFrame(): GetDC failed.\n");
			sRes = -2;
			}

		// Destroy the bitmap.
		DeleteObject(hbm);
		}
	else
		{
		TRACE("DecompressFrame(): ICDecompress failed.\n");
		sRes	= -1;
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Use handler for RtVidc buffers.
// Returns RET_FREE if done with data on return, RET_DONTFREE otherwise.
//
//////////////////////////////////////////////////////////////////////////////
short CRtVidc::Use(	UCHAR* puc, long lSize, USHORT usType, UCHAR ucFlags, 
							long lTime)
	{
	short	sRes		= RET_FREE;	// Always free.
	short	sError	= 0;

	ASSERT(usType	== RT_TYPE_VIDC);
	ASSERT(puc		!= NULL);

	CNFile file;
	file.Open(puc, lSize, ENDIAN_LITTLE);

	// Read values common to all chunks.

	// Read vidc ID.
	USHORT	usVidcId;
	file.Read (&usVidcId);
	
	// Make sure we're in range.
	ASSERT(usVidcId < MAX_VID_CHANNELS);

	// Get corresponding header.
	PVIDC_RT_HDR	pvidchdr	= &m_avidchdrs[usVidcId];

	// If this is a header chunk . . .
	if (ucFlags & VIDC_CHUNK_HEADER)
		{
		// Handle header chunk.
		file.Read(&pvidchdr->sNumFrames);
		file.Read(&pvidchdr->sWidth);
		file.Read(&pvidchdr->sHeight);
		file.Read(&pvidchdr->sDepth);
		file.Read(&pvidchdr->lMilliPerFrame);
		file.Read(&pvidchdr->sNoDelta);
		file.Read(&pvidchdr->sTransparent);
		file.Read(&pvidchdr->sX);
		file.Read(&pvidchdr->sY);
		file.Read(&pvidchdr->ulFCCHandler);
		
		// Verify we didn't read too much.
		ASSERT(file.Error() == FALSE);

		// Initialize frame counter.
		pvidchdr->sCurFrame			= 0;
		pvidchdr->sColorsModified	= FALSE;
		// Default to one frame's worth of lag before skipping frames.
		pvidchdr->lMaxLag				= pvidchdr->lMilliPerFrame;

		// Attempt to open desired compressor/decompressor . . .
//		pvidchdr->hic	= ICOpen(ICTYPE_VIDEO, pvidchdr->ulFCCHandler, 
//										ICMODE_FASTDECOMPRESS);

		// If there is a callback for the header . . .
		if (pvidchdr->callbackHeader != NULL)
			{
			// Pass user all info.
			(*pvidchdr->callbackHeader)(this, pvidchdr);
			}
		}
	else
		{
		// If there is a callback for before decompression . . .
		if (pvidchdr->callbackBefore != NULL)
			{
			// Pass user all info.
			(*pvidchdr->callbackBefore)(this, pvidchdr);
			}

		// Decompress into image if supplied.
		if (pvidchdr->pImage != NULL)
			{
			ASSERT(pvidchdr->pImage->pData				!= NULL);
			ASSERT(pvidchdr->pImage->pPalette			!= NULL);
			ASSERT(pvidchdr->pImage->pPalette->pData	!= NULL);

			ULONG	ulFlags	= 0;
			if (ucFlags & RT_FLAG_TAG)
				{
				// Key frame.
				}
			else
				{
				ulFlags	|= ICDECOMPRESS_NOTKEYFRAME;
				}

			// If we've exceeded the maximum lag . . .
			if (m_pdispatch->GetTime() - lTime > pvidchdr->lMaxLag)
				{
				ulFlags	|= ICDECOMPRESS_HURRYUP;
				}

			BMI	bmiIn;
			BMI	bmiOut;
			ReadBitmapInfo((BITMAPINFO*)&bmiIn, &file);
			ReadBitmapInfo((BITMAPINFO*)&bmiOut, &file);

			// TEMP
			bmiOut.bmiHeader.biHeight		= bmiIn.bmiHeader.biHeight;
			bmiOut.bmiHeader.biSizeImage	= (pvidchdr->pImage->lPitch 
													* bmiOut.bmiHeader.biHeight
													* bmiOut.bmiHeader.biBitCount)
													/ 8;
			// END TEMP

			if (pvidchdr->hic == NULL)
				{
				pvidchdr->hic	= ICLocate(	ICTYPE_VIDEO, pvidchdr->ulFCCHandler,
													&bmiIn.bmiHeader,
													&bmiOut.bmiHeader,
													ICMODE_FASTDECOMPRESS);

				if (pvidchdr->hic != NULL)
					{
					// Success.
					}
				else
					{
					TRACE("Use(): Unable to locate decompressor.\n");
					}
				}

			// Test.
			ASSERT(ICDecompressQuery(pvidchdr->hic, 
										(BITMAPINFO*)&bmiIn,
										(BITMAPINFO*)&bmiOut) == ICERR_OK);

			if (ICDecompressSetPalette(pvidchdr->hic, &bmiOut) == ICERR_OK)
				{
				// Success.
				}
			else
				{
				TRACE("Use(): Unable to set decompressor palette.\n");
				sError = 7;
				}

			// The rest of this chunk is regular old VIDC data!
			if (DecompressFrame(pvidchdr, &file, ulFlags, &bmiIn, &bmiOut) == 0)
				{
				// Successfully decompressed frames.
				if (ICDecompressGetPalette(pvidchdr->hic, 
													(BITMAPINFOHEADER*)&bmiIn, 
													(BITMAPINFOHEADER*)&bmiOut) == ICERR_OK)
					{
					if (ucFlags & VIDC_CHUNK_PALETTE)
						{
						ASSERT(pvidchdr->pImage->pData				!= NULL);
						ASSERT(pvidchdr->pImage->pPalette			!= NULL);
						ASSERT(pvidchdr->pImage->pPalette->pData	!= NULL);

						CPal		pal;
						pal.ulType					= PDIB;
						pal.ulSize					= bmiOut.bmiHeader.biSize - sizeof(bmiOut.bmiHeader);
						pal.sPalEntrySize			= sizeof(bmiOut.bmiColors[0]);
						pal.pData					= (UCHAR*)bmiOut.bmiColors;
						// Attach the palette to an empty image.
						CImage	imageEmpty;
						imageEmpty.ulSize			= 0L;
						imageEmpty.ulType			= BMP8;
						imageEmpty.pPalette		= &pal;
			
						// If necessary . . .
						if (pal.ulType != pvidchdr->pImage->pPalette->ulType)
							{
							// Attempt to convert to user image type . . .
							if (imageEmpty.Convert(pvidchdr->pImage->ulType) == 0)
								{
								// Successfully converted.
								}
							else
								{
								TRACE("Use(): Failed to convert temp palette data to "
										"user format.\n");
								sError = 3;
								}
							}

						// Copy converted data that the user cares about.
						memcpy(	pvidchdr->pImage->pPalette->pData,
									pal.pData	+ pvidchdr->pImage->pPalette->sStartIndex 
													* pvidchdr->pImage->pPalette->sPalEntrySize,
									pvidchdr->pImage->pPalette->sNumEntries 
									* pvidchdr->pImage->pPalette->sPalEntrySize);

						imageEmpty.pPalette	= NULL;

						pvidchdr->sColorsModified	= TRUE;
						}
					else
						{
						pvidchdr->sColorsModified	= FALSE;
						}
					}
				else
					{
					TRACE("Use(): Unable to get palette from decompressor.\n");
					sError = 6;
					}

				// If there is a callback for after decompression . . .
				if (pvidchdr->callbackAfter != NULL)
					{
					// Pass user all info.
					(*pvidchdr->callbackAfter)(this, pvidchdr);
					}
		
				// Increment frame count.
				pvidchdr->sCurFrame++;
				}
			else
				{
				TRACE("Use(): VIDC decompression failed.\n");
				sError = 1;
				}
			}
		else
			{
			TRACE("Use(): Data with no associated image.\n");
			}

		// If this is the last chunk . . .
		if (ucFlags & RT_FLAG_LAST)
			{
			// If compressor was opened . . .
			if (pvidchdr->hic != NULL)
				{
				ICClose(pvidchdr->hic);
				pvidchdr->hic	= NULL;
				}
			}
	
		// Verify we didn't read too much.
		ASSERT(file.Error() == FALSE);
		}

	file.Close();

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Callback dispatcher (calls the implied this version).
// (static)
//
//////////////////////////////////////////////////////////////////////////////
short CRtVidc::UseStatic(	UCHAR* puc, long lSize, USHORT usType, 
									UCHAR ucFlags, long lTime, long l_pRtVidc)
	{
	return ((CRtVidc*)l_pRtVidc)->Use(puc, lSize, usType, ucFlags, lTime);
	}


//////////////////////////////////////////////////////////////////////////////
// Methods.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Set dispatcher.
//
//////////////////////////////////////////////////////////////////////////////
void CRtVidc::SetDispatcher(CDispatch* pdispatch)
	{
	if (m_pdispatch != NULL)
		{
		m_pdispatch->SetDataHandler(RT_TYPE_VIDC, NULL);
		}

	m_pdispatch	= pdispatch;

	if (m_pdispatch != NULL)
		{
		m_pdispatch->SetDataHandler(RT_TYPE_VIDC, UseStatic);
		m_pdispatch->SetUserVal(RT_TYPE_VIDC, (long)this);
		}
	}

//////////////////////////////////////////////////////////////////////////////
//
// Sets callback(s) called on channel header receipt.
//
//////////////////////////////////////////////////////////////////////////////
void CRtVidc::SetCallbackHeader(RTVIDC_CALL callback)
	{
	for (short i = 0; i < MAX_VID_CHANNELS; i++)
		{
		SetCallbackHeader(callback, i);
		}
	}

//////////////////////////////////////////////////////////////////////////////
//
// Sets callback(s) called on channel header receipt.
//
//////////////////////////////////////////////////////////////////////////////
void CRtVidc::SetCallbackHeader(RTVIDC_CALL callback, short sChannel)
	{
	m_avidchdrs[sChannel].callbackHeader	= callback;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Sets callback(s) called before decompression.
//
//////////////////////////////////////////////////////////////////////////////
void CRtVidc::SetCallbackBefore(RTVIDC_CALL callback)
	{
	for (short i = 0; i < MAX_VID_CHANNELS; i++)
		{
		SetCallbackBefore(callback, i);
		}
	}

//////////////////////////////////////////////////////////////////////////////
//
// Sets callback(s) called before decompression.
//
//////////////////////////////////////////////////////////////////////////////
void CRtVidc::SetCallbackBefore(RTVIDC_CALL callback, short sChannel)
	{
	m_avidchdrs[sChannel].callbackBefore	= callback;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Sets callback(s) called after decompression.
//
//////////////////////////////////////////////////////////////////////////////
void CRtVidc::SetCallbackAfter(RTVIDC_CALL callback)
	{
	for (short i = 0; i < MAX_VID_CHANNELS; i++)
		{
		SetCallbackAfter(callback, i);
		}
	}

//////////////////////////////////////////////////////////////////////////////
//
// Sets callback(s) called after decompression.
//
//////////////////////////////////////////////////////////////////////////////
void CRtVidc::SetCallbackAfter(RTVIDC_CALL callback, short sChannel)
	{
	m_avidchdrs[sChannel].callbackAfter	= callback;
	}

//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
