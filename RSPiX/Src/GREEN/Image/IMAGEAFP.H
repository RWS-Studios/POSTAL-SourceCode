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
//	IMAGEAFP.H
//   
//	Created on 		10/17/95	BRH 
// Implemented on 10/17/95 BRH
//
// 10/17/95	BRH	Split this from imagetyp.h to avoid multiple
//						inclusion of the array of function pointers.
//
//	10/26/95 BRH	Added ConvertToSCREEN24_RGB to support one of the
//						more standard windows screen formats.
//
//	When adding a convert function:
//	------------------------------
// Make sure you add a enumerated type to imagetyp.h, preferably
//	just before END_OF_TYPES and then add your convert function to
// the afpConvert array in the proper place.  Your convert function
//	will be indexed by your image type, for example image type BMP8
// = 1 in the enumerated type list and so afpConvert[1] = 
// ConvertToBMP8.  Then you can add your function prototype to
// imagecon.h and the implementation to imagecon.cpp, or you can
// put your convert function in your own files.  In that case you
// will need to #include your .h file in this header file.
//
//////////////////////////////////////////////////////////////////////

#ifndef IMAGEAFP_H
#define IMAGEAFP_H

#include "imagecon.h"

typedef short (*CONVFUNC)(CImage* pImage);

// These are "convert to..." functions which should first check the current image type
// to see if it can convert from the current type to the specified type
CONVFUNC afpConvert[255] = 
{
	ConvertNoSupport,
	ConvertToBMP8,
	ConvertToSystem,	
	ConvertToSCREEN8_555,	
	ConvertToSCREEN8_565,
	ConvertToSCREEN8_888,
	ConvertToBMP24,
	ConvertToSCREEN16_555,
	ConvertToSCREEN16_565,
	ConvertToSCREEN24_RGB,
	ConvertToSCREEN32_ARGB,
	ConvertToBKD8

};

#endif //IMAGEAFP_H
