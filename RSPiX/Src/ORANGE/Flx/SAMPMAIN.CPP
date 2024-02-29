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

#include <graph.h>

#include "portable.h"
#include "flx.h"

void DumpHeader(FLX_FILE_HDR* pfilehdr);

int SmartHeap_far_malloc;

///////////////////////////////////////////////////////////////////////////////
//
// Example of reading in an existing flic and creating and writing out a new
// flic.  In other words, we make a duplicate of the flic.
//
///////////////////////////////////////////////////////////////////////////////
void main(void)
	{
	static FLX_BUF buf1;

//	if (flxRead.Open("BUS_DRIV.FLI") == 0)	// Written by ??? - FLI format
//	if (flxRead.Open("DIAGONAL.FLI") == 0)	// Written by AniPro - says FLI but it's FLC!

	// Open existing FLI file for reading
	CFlx flxRead;
	static FLX_FILE_HDR filehdrRead;
	
	// testing with an invalid file to duplicate the error
	if (flxRead.Open("NOT_HERE.FLI", TRUE, &filehdrRead, &buf1) == 0)
	{
		cout << "This is not possible!\n";
	}
	else
	{
		cout << "We've just created an error opening.  See if the next file opens okay...\n";
	}
	
	if (flxRead.Open("abc_b.FLi", TRUE, &filehdrRead, &buf1) == 0)
		{
		DumpHeader(&filehdrRead);

		// Create new FLI file for writing
		CFlx flxWrite;
		static FLX_FILE_HDR filehdrWrite;
/*		if (flxWrite.Create("MY_frm.FLI", TRUE,
			filehdrRead.sWidth, filehdrRead.sHeight, 20,//filehdrRead.lMilliPerFrame,
			filehdrRead.sAspectX, filehdrRead.sAspectY,
			TRUE, TRUE, &filehdrWrite, NULL) == 0)
			{
			DumpHeader(&filehdrWrite);
			
			cout << "Processing frames:\n";
							
			// Read each frame and write it out
			for (short sFrame = 1; sFrame <= filehdrRead.sNumFrames; sFrame++)
				{
				cout << sFrame << "\n";
				
				if (flxRead.ReadNextFrame(&buf1) == 0)
					{
					if (flxWrite.WriteNextFrame(&buf1) == 0)
						{
						}
					else
						{
						cout << "Error reported by CFlx::WriteNextFrame()!\n";
						break;
						}
					}
				else
					{
					cout << "Error reported by CFlx::ReadNextFrame()!\n";
					break;
					}
				}
				
			flxWrite.WriteFinish(NULL, NULL);
			flxWrite.Close();
			}
		else
			cout << "Error reported by CFlx::Create()!\n";
			
		flxRead.Close(&buf1);
		flxRead.Open("Farmtest.FLc", TRUE, &filehdrRead, &buf1);
*/			
		if (flxWrite.Create("MY_abc.fli", TRUE,
			filehdrRead.sWidth, filehdrRead.sHeight, 50,//filehdrRead.lMilliPerFrame,
			filehdrRead.sAspectX, filehdrRead.sAspectY,
			TRUE, TRUE, &filehdrWrite, NULL) == 0)
			{
			DumpHeader(&filehdrWrite);
							
			cout << "Processing frames:\n";
							
			// Read each frame and write it out
			for (short sFrame = 1; sFrame <= filehdrRead.sNumFrames; sFrame++)
				{
				cout << sFrame << "\n";
				
				if (flxRead.ReadNextFrame(&buf1) == 0)
					{
					if (flxWrite.WriteNextFrame(&buf1) == 0)
						{
						}
					else
						{
						cout << "Error reported by CFlx::WriteNextFrame()!\n";
						break;
						}
					}
				else
					{
					cout << "Error reported by CFlx::ReadNextFrame()!\n";
					break;
					}
				}
				
			//flxWrite.WriteFinish(NULL, NULL);
			flxWrite.Close();
			}
		else
			cout << "Error reported by CFlx::Create()!\n";
			
		flxRead.Close(&buf1);
		}
	else
		cout << "Error reported by CFlx::Open()!\n";
	}
	
	
///////////////////////////////////////////////////////////////////////////////
//
// Dump all the info in a flx file header
//
///////////////////////////////////////////////////////////////////////////////
void DumpHeader(FLX_FILE_HDR* pfilehdr)
	{
	cout << "lEntireFileSize = " << pfilehdr->lEntireFileSize << endl;
	cout << "wMagic = " << pfilehdr->wMagic << endl;
	cout << "sNumFrames " << pfilehdr->sNumFrames << endl;
	cout << "sWidth " << pfilehdr->sWidth << endl;
	cout << "sHeight " << pfilehdr->sHeight << endl;
	cout << "sDepth " << pfilehdr->sDepth << endl;
	cout << "sFlags " << pfilehdr->sFlags << endl;
	cout << "lMilliPerFrame " << pfilehdr->lMilliPerFrame << endl;
	cout << "dCreatedTime " << pfilehdr->dCreatedTime << endl;
	cout << "dCreator " << pfilehdr->dCreator << endl;
	cout << "dUpdatedTime " << pfilehdr->dUpdatedTime << endl;
	cout << "dUpdater " << pfilehdr->dUpdater << endl;
	cout << "sAspectX " << pfilehdr->sAspectX << endl;
	cout << "sAspectY " << pfilehdr->sAspectY << endl;
	cout << "lOffsetFrame1 " << pfilehdr->lOffsetFrame1 << endl;
	cout << "lOffsetFrame2 " << pfilehdr->lOffsetFrame2 << endl;
	}
	

///////////////////////////////////////////////////////////////////////////////
// Old code to display image and palette - takes about 10 minutes per frame!!!!
///////////////////////////////////////////////////////////////////////////////
#if 0
				for (short c = 10; c < 246; c++)
					{
					long rgb = ((long)((long)(prgbColors[c].bR) >> (long)2) & 0x000000ffL) |
					           ((long)((long)(prgbColors[c].bG) << (long)6) & 0x0000ff00L) |
					           ((long)((long)(prgbColors[c].bB) << (long)14) & 0x00ff0000L);
					_remappalette(c, rgb);
					}
				for (short y = 0; y < filehdrRead.sHeight; y++)
					{
					for (short x = 0; x < filehdrRead.sWidth; x++)
						{
						_setcolor((short)(pbPixels[y][x]));
						_setpixel(x, y);
						}
					}
#endif
	
	
///////////////////////////////////////////////////////////////////////////////
// EOF
///////////////////////////////////////////////////////////////////////////////

