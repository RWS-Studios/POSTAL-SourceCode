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
//	IMAGECON.H
//   
//	Created on 		09/28/95	BRH 
// Implemented on 09/28/95 BRH
//
// 09/28/95	BRH	Standard conversion functions for the standard
//						image types.  The two main standards are 8 and 24
//						bit images.  The standard converters are explained
//						in the .cpp file before each conversion function.
//
//	10/30/96	JMI	Added a message indicating this was obsoleted as it doesn't
//						seem to do anything.
//
//	11/01/96	JMI	Enhanced message referred to on 10/30/96.
//
///////////////////////////////////////////////////////////////////////////////

// This header is no longer used.
#pragma message ( __FILE__ " : Do not include this header.  It is no longer used." )

#ifndef IMAGECON_H
#define IMAGECON_H

#include "System.h"
// If PATHS_IN_INCLUDES macro is defined, we can utilize relative
// paths to a header file.  In this case we generally go off of our
// RSPiX root directory.  System.h MUST be included before this macro
// is evaluated.  System.h is the header that, based on the current
// platform (or more so in this case on the compiler), defines 
// PATHS_IN_INCLUDES.  Blue.h includes system.h so you can include that
// instead.

// Green include files
#ifdef PATHS_IN_INCLUDES
	#include "GREEN/Image/Image.h"
#else
	#include "Image.h"
#endif // PATHS_IN_INCLUDES

// Orange include files
//#include "dynalink/dynalink.h"

class CImage;		// forward declaration

// Conversion from extended to standard function typedef.
////typedef short (*CONVFROMFUNC)(CImage* pImage);
// Conversion to extended from standard function typedef.
////typedef short (*CONVTOFUNC)(CImage* pImage);

// To use the dynamic linking, we will provide macros to make it easy
// for the implementer of a new type conversion to link their function.
// These macros invoke the LINKELATE macro to allocate a static CDynaLink that
// is used to link in the conversion function.

// There is a specific use of From and To in this header.  Think of it
// in terms of the preposition you use when saying, "Convert [from|to]
// extended."

// Use this to link a new Image Convertor that converts _FROM_ an extended
// format to a standard.
// Place at file scope with your image convertor function and its type enum.
// Example:
// LINK_IMAGECONV_FROM(ConvertFromBMP8, BMP8);
/*
#define LINK_IMAGECONV_FROM(pUserFromFunc, lUserFromFuncIndex)		\
	LINKLATE(CONVFROMFUNC, CImage, MAX_IMAGE_FROMCONVERTORS,				\
				pUserFromFunc, lUserFromFuncIndex)
*/
// Use this to link a new Image Convertor that converts _TO_ an extended
// format from a standard.
// Place at file scope with your image convertor function and its type enum.
// Example:
// LINK_IMAGECONV_TO(ConvertToBMP8, BMP8);
/*
#define LINK_IMAGECONV_TO(pUserToFunc, lUserToFuncIndex)		\
	LINKLATE(CONVTOFUNC, CImage, MAX_IMAGE_TOCONVERTORS,			\
				pUserToFunc, lUserToFuncIndex)
*/
///////////////////////////////////////////////////////////////////////////////
// Internal Image use:
///////////////////////////////////////////////////////////////////////////////
// Use this to get a "to" function.
/*
#define GETTOFUNC(lIndex)		\
	GETLINKFUNC(CONVTOFUNC, CImage, MAX_IMAGE_TOCONVERTORS, lIndex)

// Use this to get a "from" function.
#define GETFROMFUNC(lIndex)	\
	GETLINKFUNC(CONVFROMFUNC, CImage, MAX_IMAGE_FROMCONVERTORS, lIndex)
*/
#endif //IMAGECON_H

///////////////////////////////////////////////////////////////////////////////
// EOF
///////////////////////////////////////////////////////////////////////////////
