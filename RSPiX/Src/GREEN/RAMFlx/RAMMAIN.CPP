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
// Sample shell for using FLC read/write stuff.
//
// This particular example is designed as a QuickWin application using
// Microsoft Visual C/C++.  The underlying FLC stuff should work in a
// regular Windows app or as a DOS app, too.
//
///////////////////////////////////////////////////////////////////////////////

#include "Blue.h"
#include "Image.h"
#include "ramflx.h"
#include "BLIT.H"
#include "tools.h"

void Test(void);
void LoadBackImage(void);
void BlitFlx(CRamFlx *pFlx, short sTrans);
void DumpHeader(FLX_FILE_HDR* pfilehdr);

int SmartHeap_far_malloc;
static CImage* 	m_pBuf;		// The image buffer where background and flx are combined
static CPalImage 	m_Back;		// The background image

///////////////////////////////////////////////////////////////////////////////
//
// AppMain is entry point for application.
//
///////////////////////////////////////////////////////////////////////////////
void AppMain(void)
	{
	// Create display
	if (Blu_CreateDisplay(640, 480, 16) == 0)
		{
		// Create screen buffer
		m_pBuf = Blit_CreateScrnBuf(640, 480, 16);
		if (m_pBuf != NULL)
			{
			// Tell blue layer about screen buffer for when we want to update screen
			Blu_SetDisplayBuf(m_pBuf->pData, m_pBuf->lWidth, m_pBuf->lHeight, 16 );

			// Tell blue layer to use same buffer to redraw screen when necessary
			Blu_SetRedrawBuf(m_pBuf->pData, m_pBuf->lWidth, m_pBuf->lHeight,
									0, 0, 0, 0, m_pBuf->lWidth, m_pBuf->lHeight, 16 );

			// Test Flx
			Test();

			// Destroy screen buffer
			Blit_DestroyScrnBuf();
			}
		else
			{
			// Error creating buffer
			}
		}
	else
		{
		// Error creating display
		}
	}

///////////////////////////////////////////////////////////////////////////////
//
// Example of reading in an existing flic and creating and writing out a new
// flic.  In other words, we make a duplicate of the flic.
//
///////////////////////////////////////////////////////////////////////////////
void Test(void)
	{
	static CPalImage buf1;

	// Open existing FLI file for reading
	CRamFlx flxRead;
	static FLX_FILE_HDR filehdrRead;
	
	// Load the background image
	LoadBackImage();

	// Open file
	if (flxRead.Open("LEWLIT.FLC", &filehdrRead, NULL) == 0)
		{
		// Show header
		DumpHeader(&filehdrRead);

		TRACE("Processing frames:\n");
		
		// Read each frame until we reach the ring frame
		for (short sFrame = 1; sFrame <= filehdrRead.sNumFrames+3;)
			{
			if (GetLeftMouseEvent()==M_EVENT_DOWN)
				if (flxRead.ReadNextFrame(NULL) == 0)
					{
					TRACE("Actual %hd, Number %hd\n",flxRead.GetFrameNum(),sFrame);
					sFrame++;
					BlitFlx(&flxRead,TRUE);
					}
				else
					{
					TRACE("Error reported by CFlx::ReadNextFrame()!\n");
					break;
					}
			Blu_System();
			}
			
		flxRead.Close(NULL);
		}
	else
		TRACE("Error reported by CFlx::Open()!\n");
	}
	
void LoadBackImage(void)
	{
	// Load background image
	if (m_Back.LoadDib("SR71.BMP",FCC_p555) == 0)
		{
		// Display background image
		Blit_Copy(&m_Back, 0, 0, m_pBuf, 0, 0, m_Back.image.lWidth, m_Back.image.lHeight, 0);
		Blu_UpdateDisplay(0, 0, 0, 0, m_Back.image.lWidth, m_Back.image.lHeight);
		}
	}	

 void BlitFlx(CRamFlx *pFlx, short sTrans)
 	{
	// Blit the image to the buffer
	CPalImage* ppi = pFlx->GetBuffer();
	if (sTrans)
		Blit_Combine(	ppi,
					0,
					0,
					&m_Back,
					0,
					0,
					m_pBuf,
					0,
					0,
					ppi->image.lWidth,
					ppi->image.lHeight);
	else
		Blit_Copy(	ppi,
					0,
					0,
					m_pBuf,
					0,
					0,
					ppi->image.lWidth,
					ppi->image.lHeight,
					sTrans);

	// Update screen from buffer
	Blu_UpdateDisplay(0,			// buffer x
							0,			// buffer y
							0,			// screen x
							0,			// screen y
							ppi->image.lWidth,	// width
							ppi->image.lHeight);	// height

	}

///////////////////////////////////////////////////////////////////////////////
//
// Dump all the info in a flx file header
//
///////////////////////////////////////////////////////////////////////////////
void DumpHeader(FLX_FILE_HDR* pfilehdr)
	{
	TRACE("lEntireFileSize = %ld\n",pfilehdr->lEntireFileSize);
	TRACE("wMagic = %hu\n",pfilehdr->wMagic);
	TRACE("sNumFrames %hd\n",pfilehdr->sNumFrames);
	TRACE("sWidth %hd\n",pfilehdr->sWidth);
	TRACE("sHeight %hd\n",pfilehdr->sHeight);
	TRACE("sDepth %hd\n",pfilehdr->sDepth);
	TRACE("sFlags %hd\n",pfilehdr->sFlags);
	TRACE("lMilliPerFrame %ld\n",pfilehdr->lMilliPerFrame);
	TRACE("dCreatedTime %lu\n",pfilehdr->dCreatedTime);
	TRACE("dCreator %lu\n",pfilehdr->dCreator);
	TRACE("dUpdatedTime %lu\n",pfilehdr->dUpdatedTime);
	TRACE("dUpdater %lu\n",pfilehdr->dUpdater);
	TRACE("sAspectX %hd\n",pfilehdr->sAspectX);
	TRACE("sAspectY %hd\n",pfilehdr->sAspectY);
	TRACE("lOffsetFrame1 %ld\n",pfilehdr->lOffsetFrame1);
	TRACE("lOffsetFrame2 %ld\n",pfilehdr->lOffsetFrame2);
	}
	
///////////////////////////////////////////////////////////////////////////////
// EOF
///////////////////////////////////////////////////////////////////////////////

