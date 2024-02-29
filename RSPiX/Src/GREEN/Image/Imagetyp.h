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
//////////////////////////////////////////////////////////////////////
//
//	IMAGETYP.H
//   
//	Created on 		09/28/95	BRH 
// Implemented on 09/28/95 BRH
//
// 09/28/95	BRH	Started this file with the defines for the standard
//						types of images and palettes. 
//
//	03/06/96	JMI	Added FLX8_888 and PFLX enums.
//
//	07/22/96	JMI	Added BMP8RLE enum.
//
// 08/01/96 BRH	Added static array of image type names for use
//						in utilities.  For example, it will be used in
//						the AnimCreate utility to initialize a listbox of
//						image type names from which the user can select
//						a destination type for the images.
//
//	09/04/96	JMI	Added BMP1, monochrome bitmap.  No palette 
//						(1 == Black, 0 == White).
//
//	10/30/96	JMI	Removed all the cool stuff from this file and
//						put it in pal.h, pal.cpp, image.h, & image.cpp.
//						Although it is a bit more clumsy to add types
//						in that everything is not all in one place any-
//						more, these things are now more strictly
//						associated with CPal and CImage.  Also, by moving
//						the astrImageTypeNames to w/i CImage, there is
//						now only one copy of that array (ms_astrTypeNames).
//						Before, there was an individual static copy of
//						the array for every module that included it
//						(whether included directly or indirectly through
//						image.h and/or rspix.h).
//
//	10/30/96	JMI	Added a message indicating this was obsoleted as it doesn't
//						do anything anymore.
// 
//	This file contains the registered CImage types that are currently
//	supported.  Any new image type can be added to this file by
// checking it out and adding a define for the type and adding
// a conversion function to the array.  Your conversion function
// needs to be able to convert from one of the standard types to
// your new type.  You may choose also to provide a reverse 
// conversion function to convert from your type to one of the 
// standard types.
//
//////////////////////////////////////////////////////////////////////

// This header is no longer used.
#pragma message ( __FILE__ "(" __LINE__ ") : Do not include this header." )

#ifndef IMAGETYP_H
#define IMAGETYP_H

#include "System.h"

// If PATHS_IN_INCLUDES macro is defined, we can utilized relative
// paths to a header file.  In this case we generally go off of our
// RSPiX root directory.  System.h MUST be included before this macro
// is evaluated.  System.h is the header that, based on the current
// platform (or more so in this case on the compiler), defines 
// PATHS_IN_INCLUDES.  Blue.h includes system.h so you can include that
// instead.

// Green include files
#ifdef PATHS_IN_INCLUDES
	#include "GREEN/Image/Image.h"
	#include "GREEN/Image/imagecon.h"
#else
	#include "Image.h"
	#include "imagecon.h"
#endif // PATHS_IN_INCLUDES





// This is kind of a sux, but the difference between a "to" and
// a "from" is that the "to" arrays are correctly sized and the
// "from" arrays contain one additional unused element.  There
// must be at least one thing different between the from and the
// to CDynaLinks.  Since the function typedefs and the friends
// were the same, the only options left were to either store them
// in the same array or do this.
#define MAX_IMAGE_TOCONVERTORS	(END_OF_TYPES)
#define MAX_IMAGE_FROMCONVERTORS	(END_OF_TYPES + 1)

#endif //IMAGETYP_H
