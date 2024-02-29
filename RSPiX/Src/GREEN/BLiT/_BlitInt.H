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
// This is designed to handle all the non-user particulars amoung the BLiT source:
#ifndef _BLITINT_H
#define _BLITINT_H

//============================== INTERNAL STUFF  ==================================
enum { BUF_MEMORY = 1,BUF_VRAM,BUF_VRAM2,BUF_WIN };

extern short gsScreenLocked;
extern short gsBufferLocked;

// This list of Image types needs to always be kept as up to date as humanly possible:
// It is only intended for use in debug mode.  It returns 0 if it is NOT the desired type,
// one if it IS the desired type:
// (Although long and tedious, we MUST validate supported types rather than invalidate
// unsupported types.)

// NOTE: These represent ONLY compressed types BLiT can access.  Use ImageIsUncompressed
// otherwise.
//
inline short ImageIsCompressed(ULONG ulType) // an image type
	{
	short	sRet = 0; // 0 if it is NOT a supported compressed type, else 1!
	
	switch(ulType)
		{
		case	RImage::FSPR1:
		case	RImage::FSPR8:
		case	RImage::FSPR16:
		case	RImage::FSPR32:
			sRet = 1;
		}
	return sRet;
	}

// NOTE: These represent ONLY uncompressed types BLiT can access.  
//
inline short ImageIsUncompressed(ULONG ulType) // an image type
	{
	short	sRet = 0; // 0 if it is NOT a supported uncompressed type, else 1!
	
	switch(ulType)
		{
		case RImage::ROTBUF:
		case RImage::BMP8:
		case RImage::SYSTEM8:
		case RImage::SCREEN8_555:
		case RImage::SCREEN8_565:
		case RImage::SCREEN8_888:
		case RImage::BMP24:
		case RImage::SCREEN16_555:
		case RImage::SCREEN16_565:
		case RImage::SCREEN24_RGB:
		case RImage::SCREEN32_ARGB:
		case RImage::IMAGE_STUB:	// careful with this one
			sRet = 1;
		}
	return sRet;
	}

/*
// Not a complete descriptor -> just replaces the pData of an uncompressed buffer.
class CCompressedMono // only a 96-bit header!
	{
public:
	USHORT	usASCII;	// = ascii value
	USHORT	usSourceType;	// uncompressed Image pre-compressed type
	UCHAR*	pCode;		// It's all in one buffer for this one
	ULONG		ulSize;		// Size of compressed buffer

	CCompressedMono()
		{
		usASCII = usSourceType = 0;
		pCode = NULL;
		ulSize = 0;
		}

	~CCompressedMono()
		{
		if (pCode) free(pCode);
		}

	}; 
	*/

extern	void _rspBLiT(UCHAR ucColor,RImage* pimSrc,RImage* pimDst,
				  short sDstX,short sDstY);

//=========================
#endif