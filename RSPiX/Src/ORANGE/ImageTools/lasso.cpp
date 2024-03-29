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
//	lasso.cpp
// 
// History:
//		10/04/96 JMI	Started.
//
//		10/08/96	JMI	Removed #pragma message ("") pragmas.
//
//		10/11/96	JMI	Now has a callback so you can do whatever the funk you
//							want for comparing shapes and empty space or something.
//
//		10/22/96	JMI	Put back rspRect since it now supports 32 bit.
//
//		10/28/96	JMI	Put back suxRect since it seems to cause the rspLassoNext
//							to work correctly.  Not sure what the difference is.
//
//		10/28/96	JMI	Now only uses implicit instantiation on WIN32, MAC seems
//							to assume such.
//
//		11/01/96	JMI	Changed:
//							Old label:				New label:
//							=========				=========
//							LASSONEXT_EVAL_CALL	RLassoNextEvalCall
//							CImage					RImage
//							CSList					RSList
//
//							Also, changed all members referenced in RImage to
//							m_ and all position/dimension members of referenced in
//							RImage to type short usage.
//
//////////////////////////////////////////////////////////////////////////////
//
// Lassoes shapes from an image.
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Includes.
//////////////////////////////////////////////////////////////////////////////

#include <string.h>

#include "System.h"

// Green /////////////////////////////////////////////////////////////////////
// If PATHS_IN_INCLUDES macro is defined, we can utilized relative
// paths to a header file.  In this case we generally go off of our
// RSPiX root directory.  System.h MUST be included before this macro
// is evaluated.  System.h is the header that, based on the current
// platform (or more so in this case on the compiler), defines 
// PATHS_IN_INCLUDES.  Blue.h includes system.h so you can include that
// instead.
#ifdef PATHS_IN_INCLUDES
	#include "GREEN/BLiT/BLIT.H"
	#include "ORANGE/ImageTools/lasso.h"
	#include "ORANGE/CDT/slist.h"
#else
	#include "BLIT.H"
	#include "lasso.h"
	#include "slist.h"
#endif // PATHS_IN_INCLUDES

//////////////////////////////////////////////////////////////////////////////
// Typedefs.
//////////////////////////////////////////////////////////////////////////////

// A list for X's that indicate portions of shape's current row to copy; sorted
// by the X.
typedef RSList<int16_t, int16_t> SLIST_SHORTS;

// Contains minimums and maximums for x and y.
typedef struct
	{
	int16_t	sMinX;
	int16_t	sMinY;
	int16_t	sMaxX;
	int16_t	sMaxY;
	} EXTENTS;

//////////////////////////////////////////////////////////////////////////////
// Macros.
//////////////////////////////////////////////////////////////////////////////

// Only set value if not NULL.
#define SET(ptr, val)		( ((ptr) != NULL) ? *(ptr) = (val) : 0 )

// The new table utilizes 3 entries:
// 1) The two new pixel values, when used as an index.
#define NEW00		0x00	// 00 00 0000
#define NEW01		0x40	// 01 00 0000
#define NEW10		0x80	// 10 00 0000
#define NEW11		0xC0	// 11 00 0000
#define NEWMASK	0xC0	// Mask of NEW??.

// 2) The last direction moved, when used as an index,
//		the new direction, when used from the table.
#define DIRRIGHT	0x00	// 00 00 0000
#define DIRDOWN	0x10	// 01 00 0000
#define DIRLEFT	0x20	// 10 00 0000
#define DIRUP		0x30	// 11 00 0000
#define DIRMASK	0x30	// Mask of DIR*.

// 3) The last pixel values, when used as an index,
//		the new values, when used from the table.
#define PIX0000	0x0	// 0000
#define PIX0001	0x1	// 0001
#define PIX0010	0x2	// 0010
#define PIX0011	0x3	// 0011
#define PIX0100	0x4	// 0100
#define PIX0101	0x5	// 0101
#define PIX0110	0x6	// 0110
#define PIX0111	0x7	// 0111
#define PIX1000	0x8	// 1000
#define PIX1001	0x9	// 1001
#define PIX1010	0xA	// 1010
#define PIX1011	0xB	// 1011
#define PIX1100	0xC	// 1100
#define PIX1101	0xD	// 1101
#define PIX1110	0xE	// 1110
#define PIX1111	0xF	// 1111
#define PIXMASK	0xF	// Mask of PIX*.

#define ERROR		PIX1111

//////////////////////////////////////////////////////////////////////////////
// Globals.
//////////////////////////////////////////////////////////////////////////////

// This table, when provided with the two new pixels (macroed as NEW??),
// the last direction moved (macroed as DIR*), and the last pixel values 
// (macroed as PIX????), produces the new direction to move and the new 
// pixel values.
// ERROR is the result, if a bad situation is fed as an index.
// There are several impossiblities that are also flagged by ERROR.

static U16	ms_au16EdgeInfo[256]	=
	{
	ERROR,								// Index:  NEW00 | DIRRIGHT	| PIX0000 is an error b/c we lost the shape.
	DIRDOWN	| PIX0010,				// Index:  NEW00 | DIRRIGHT	| PIX0001
	ERROR,								// Index:  NEW00 | DIRRIGHT	| PIX0010
	DIRDOWN	| PIX0010,				// Index:  NEW00 | DIRRIGHT	| PIX0011
	ERROR,								// Index:  NEW00 | DIRRIGHT	| PIX0100
	ERROR,								// Index:  NEW00 | DIRRIGHT	| PIX0101
	ERROR,								// Index:  NEW00 | DIRRIGHT	| PIX0110
	ERROR,								// Index:  NEW00 | DIRRIGHT	| PIX0111
	ERROR,								// Index:  NEW00 | DIRRIGHT	| PIX1000
	DIRDOWN	| PIX0010,				// Index:  NEW00 | DIRRIGHT	| PIX1001
	ERROR,								// Index:  NEW00 | DIRRIGHT	| PIX1010
	DIRDOWN	| PIX0010,				// Index:  NEW00 | DIRRIGHT	| PIX1011
	ERROR,								// Index:  NEW00 | DIRRIGHT	| PIX1100
	ERROR,								// Index:  NEW00 | DIRRIGHT	| PIX1101
	ERROR,								// Index:  NEW00 | DIRRIGHT	| PIX1110
	ERROR,								// Index:  NEW00 | DIRRIGHT	| PIX1111 is an error, b/c we are inside the shape.

	ERROR,								// Index:  NEW00 | DIRDOWN		| PIX0000 is an error b/c we lost the shape.
	ERROR,								// Index:  NEW00 | DIRDOWN		| PIX0001
	DIRLEFT	| PIX1000,				// Index:  NEW00 | DIRDOWN		| PIX0010
	ERROR,								// Index:  NEW00 | DIRDOWN		| PIX0011
	ERROR,								// Index:  NEW00 | DIRDOWN		| PIX0100
	ERROR,								// Index:  NEW00 | DIRDOWN		| PIX0101
	DIRLEFT	| PIX1000,				// Index:  NEW00 | DIRDOWN		| PIX0110
	ERROR,								// Index:  NEW00 | DIRDOWN		| PIX0111
	ERROR,								// Index:  NEW00 | DIRDOWN		| PIX1000
	ERROR,								// Index:  NEW00 | DIRDOWN		| PIX1001
	DIRLEFT	| PIX1000,				// Index:  NEW00 | DIRDOWN		| PIX1010
	ERROR,								// Index:  NEW00 | DIRDOWN		| PIX1011
	ERROR,								// Index:  NEW00 | DIRDOWN		| PIX1100
	ERROR,								// Index:  NEW00 | DIRDOWN		| PIX1101
	DIRLEFT	| PIX1000,				// Index:  NEW00 | DIRDOWN		| PIX1110
	ERROR,								// Index:  NEW00 | DIRDOWN		| PIX1111 is an error, b/c we are inside the shape.

	ERROR,								// Index:  NEW00 | DIRLEFT		| PIX0000 is an error b/c we lost the shape.
	ERROR,								// Index:  NEW00 | DIRLEFT		| PIX0001
	ERROR,								// Index:  NEW00 | DIRLEFT		| PIX0010
	ERROR,								// Index:  NEW00 | DIRLEFT		| PIX0011
	ERROR,								// Index:  NEW00 | DIRLEFT		| PIX0100
	ERROR,								// Index:  NEW00 | DIRLEFT		| PIX0101
	ERROR,								// Index:  NEW00 | DIRLEFT		| PIX0110
	ERROR,								// Index:  NEW00 | DIRLEFT		| PIX0111
	DIRUP		| PIX0100,				// Index:  NEW00 | DIRLEFT		| PIX1000
	DIRUP		| PIX0100,				// Index:  NEW00 | DIRLEFT		| PIX1001
	ERROR,								// Index:  NEW00 | DIRLEFT		| PIX1010
	ERROR,								// Index:  NEW00 | DIRLEFT		| PIX1011
	DIRUP		| PIX0100,				// Index:  NEW00 | DIRLEFT		| PIX1100
	DIRUP		| PIX0100,				// Index:  NEW00 | DIRLEFT		| PIX1101
	ERROR,								// Index:  NEW00 | DIRLEFT		| PIX1110
	ERROR,								// Index:  NEW00 | DIRLEFT		| PIX1111 is an error, b/c we are inside the shape.

	ERROR,								// Index:  NEW00 | DIRUP		| PIX0000 is an error b/c we lost the shape.
	ERROR,								// Index:  NEW00 | DIRUP		| PIX0001
	ERROR,								// Index:  NEW00 | DIRUP		| PIX0010
	ERROR,								// Index:  NEW00 | DIRUP		| PIX0011
	DIRRIGHT	| PIX0001,				// Index:  NEW00 | DIRUP		| PIX0100
	DIRRIGHT	| PIX0001,				// Index:  NEW00 | DIRUP		| PIX0101
	DIRRIGHT	| PIX0001,				// Index:  NEW00 | DIRUP		| PIX0110
	DIRRIGHT	| PIX0001,				// Index:  NEW00 | DIRUP		| PIX0111
	ERROR,								// Index:  NEW00 | DIRUP		| PIX1000
	ERROR,								// Index:  NEW00 | DIRUP		| PIX1001
	ERROR,								// Index:  NEW00 | DIRUP		| PIX1010
	ERROR,								// Index:  NEW00 | DIRUP		| PIX1011
	ERROR,								// Index:  NEW00 | DIRUP		| PIX1100
	ERROR,								// Index:  NEW00 | DIRUP		| PIX1101
	ERROR,								// Index:  NEW00 | DIRUP		| PIX1110
	ERROR,								// Index:  NEW00 | DIRUP		| PIX1111 is an error, b/c we are inside the shape.

	
	ERROR,								// Index:  NEW01 | DIRRIGHT	| PIX0000 is an error b/c we lost the shape.
	DIRRIGHT	| PIX0011,				// Index:  NEW01 | DIRRIGHT	| PIX0001
	ERROR,								// Index:  NEW01 | DIRRIGHT	| PIX0010
	DIRRIGHT	| PIX0011,				// Index:  NEW01 | DIRRIGHT	| PIX0011
	ERROR,								// Index:  NEW01 | DIRRIGHT	| PIX0100
	ERROR,								// Index:  NEW01 | DIRRIGHT	| PIX0101
	ERROR,								// Index:  NEW01 | DIRRIGHT	| PIX0110
	ERROR,								// Index:  NEW01 | DIRRIGHT	| PIX0111
	ERROR,								// Index:  NEW01 | DIRRIGHT	| PIX1000
	DIRRIGHT	| PIX0011,				// Index:  NEW01 | DIRRIGHT	| PIX1001
	ERROR,								// Index:  NEW01 | DIRRIGHT	| PIX1010
	DIRRIGHT	| PIX0011,				// Index:  NEW01 | DIRRIGHT	| PIX1011
	ERROR,								// Index:  NEW01 | DIRRIGHT	| PIX1100
	ERROR,								// Index:  NEW01 | DIRRIGHT	| PIX1101
	ERROR,								// Index:  NEW01 | DIRRIGHT	| PIX1110
	ERROR,								// Index:  NEW01 | DIRRIGHT	| PIX1111 is an error, b/c we are inside the shape.
														       
	ERROR,								// Index:  NEW01 | DIRDOWN		| PIX0000 is an error b/c we lost the shape.
	ERROR,								// Index:  NEW01 | DIRDOWN		| PIX0001
	DIRRIGHT	| PIX1001,				// Index:  NEW01 | DIRDOWN		| PIX0010
	ERROR,								// Index:  NEW01 | DIRDOWN		| PIX0011
	ERROR,								// Index:  NEW01 | DIRDOWN		| PIX0100
	ERROR,								// Index:  NEW01 | DIRDOWN		| PIX0101
	DIRRIGHT	| PIX1001,				// Index:  NEW01 | DIRDOWN		| PIX0110
	ERROR,								// Index:  NEW01 | DIRDOWN		| PIX0111
	ERROR,								// Index:  NEW01 | DIRDOWN		| PIX1000
	ERROR,								// Index:  NEW01 | DIRDOWN		| PIX1001
	DIRRIGHT	| PIX1001,				// Index:  NEW01 | DIRDOWN		| PIX1010
	ERROR,								// Index:  NEW01 | DIRDOWN		| PIX1011
	ERROR,								// Index:  NEW01 | DIRDOWN		| PIX1100
	ERROR,								// Index:  NEW01 | DIRDOWN		| PIX1101
	DIRRIGHT	| PIX1001,				// Index:  NEW01 | DIRDOWN		| PIX1110
	ERROR,								// Index:  NEW01 | DIRDOWN		| PIX1111 is an error, b/c we are inside the shape.
														       
	ERROR,								// Index:  NEW01 | DIRLEFT		| PIX0000 is an error b/c we lost the shape.
	ERROR,								// Index:  NEW01 | DIRLEFT		| PIX0001
	ERROR,								// Index:  NEW01 | DIRLEFT		| PIX0010
	ERROR,								// Index:  NEW01 | DIRLEFT		| PIX0011
	ERROR,								// Index:  NEW01 | DIRLEFT		| PIX0100
	ERROR,								// Index:  NEW01 | DIRLEFT		| PIX0101
	ERROR,								// Index:  NEW01 | DIRLEFT		| PIX0110
	ERROR,								// Index:  NEW01 | DIRLEFT		| PIX0111
	DIRDOWN	| PIX0110,				// Index:  NEW01 | DIRLEFT		| PIX1000
	DIRDOWN	| PIX0110,				// Index:  NEW01 | DIRLEFT		| PIX1001
	ERROR,								// Index:  NEW01 | DIRLEFT		| PIX1010
	ERROR,								// Index:  NEW01 | DIRLEFT		| PIX1011
	DIRDOWN	| PIX0110,				// Index:  NEW01 | DIRLEFT		| PIX1100
	DIRDOWN	| PIX0110,				// Index:  NEW01 | DIRLEFT		| PIX1101
	ERROR,								// Index:  NEW01 | DIRLEFT		| PIX1110
	ERROR,								// Index:  NEW01 | DIRLEFT		| PIX1111 is an error, b/c we are inside the shape.
														       
	ERROR,								// Index:  NEW01 | DIRUP		| PIX0000 is an error b/c we lost the shape.
	ERROR,								// Index:  NEW01 | DIRUP		| PIX0001
	ERROR,								// Index:  NEW01 | DIRUP		| PIX0010
	ERROR,								// Index:  NEW01 | DIRUP		| PIX0011
	DIRUP		| PIX0101,				// Index:  NEW01 | DIRUP		| PIX0100
	DIRUP		| PIX0101,				// Index:  NEW01 | DIRUP		| PIX0101
	DIRUP		| PIX0101,				// Index:  NEW01 | DIRUP		| PIX0110
	DIRUP		| PIX0101,				// Index:  NEW01 | DIRUP		| PIX0111
	ERROR,								// Index:  NEW01 | DIRUP		| PIX1000
	ERROR,								// Index:  NEW01 | DIRUP		| PIX1001
	ERROR,								// Index:  NEW01 | DIRUP		| PIX1010
	ERROR,								// Index:  NEW01 | DIRUP		| PIX1011
	ERROR,								// Index:  NEW01 | DIRUP		| PIX1100
	ERROR,								// Index:  NEW01 | DIRUP		| PIX1101
	ERROR,								// Index:  NEW01 | DIRUP		| PIX1110
	ERROR,								// Index:  NEW01 | DIRUP		| PIX1111 is an error, b/c we are inside the shape.
														       
														       
	ERROR,								// Index:  NEW10 | DIRRIGHT	| PIX0000 is an error b/c we lost the shape.
	DIRUP		| PIX0110,				// Index:  NEW10 | DIRRIGHT	| PIX0001
	ERROR,								// Index:  NEW10 | DIRRIGHT	| PIX0010
	DIRUP		| PIX0110,				// Index:  NEW10 | DIRRIGHT	| PIX0011
	ERROR,								// Index:  NEW10 | DIRRIGHT	| PIX0100
	ERROR,								// Index:  NEW10 | DIRRIGHT	| PIX0101
	ERROR,								// Index:  NEW10 | DIRRIGHT	| PIX0110
	ERROR,								// Index:  NEW10 | DIRRIGHT	| PIX0111
	ERROR,								// Index:  NEW10 | DIRRIGHT	| PIX1000
	DIRUP		| PIX0110,				// Index:  NEW10 | DIRRIGHT	| PIX1001
	ERROR,								// Index:  NEW10 | DIRRIGHT	| PIX1010
	DIRUP		| PIX0110,				// Index:  NEW10 | DIRRIGHT	| PIX1011
	ERROR,								// Index:  NEW10 | DIRRIGHT	| PIX1100
	ERROR,								// Index:  NEW10 | DIRRIGHT	| PIX1101
	ERROR,								// Index:  NEW10 | DIRRIGHT	| PIX1110
	ERROR,								// Index:  NEW10 | DIRRIGHT	| PIX1111 is an error, b/c we are inside the shape.
														       
	ERROR,								// Index:  NEW10 | DIRDOWN		| PIX0000 is an error b/c we lost the shape.
	ERROR,								// Index:  NEW10 | DIRDOWN		| PIX0001
	DIRDOWN	| PIX1010,				// Index:  NEW10 | DIRDOWN		| PIX0010
	ERROR,								// Index:  NEW10 | DIRDOWN		| PIX0011
	ERROR,								// Index:  NEW10 | DIRDOWN		| PIX0100
	ERROR,								// Index:  NEW10 | DIRDOWN		| PIX0101
	DIRDOWN	| PIX1010,				// Index:  NEW10 | DIRDOWN		| PIX0110
	ERROR,								// Index:  NEW10 | DIRDOWN		| PIX0111
	ERROR,								// Index:  NEW10 | DIRDOWN		| PIX1000
	ERROR,								// Index:  NEW10 | DIRDOWN		| PIX1001
	DIRDOWN	| PIX1010,				// Index:  NEW10 | DIRDOWN		| PIX1010
	ERROR,								// Index:  NEW10 | DIRDOWN		| PIX1011
	ERROR,								// Index:  NEW10 | DIRDOWN		| PIX1100
	ERROR,								// Index:  NEW10 | DIRDOWN		| PIX1101
	DIRDOWN	| PIX1010,				// Index:  NEW10 | DIRDOWN		| PIX1110
	ERROR,								// Index:  NEW10 | DIRDOWN		| PIX1111 is an error, b/c we are inside the shape.
														       
	ERROR,								// Index:  NEW10 | DIRLEFT		| PIX0000 is an error b/c we lost the shape.
	ERROR,								// Index:  NEW10 | DIRLEFT		| PIX0001
	ERROR,								// Index:  NEW10 | DIRLEFT		| PIX0010
	ERROR,								// Index:  NEW10 | DIRLEFT		| PIX0011
	ERROR,								// Index:  NEW10 | DIRLEFT		| PIX0100
	ERROR,								// Index:  NEW10 | DIRLEFT		| PIX0101
	ERROR,								// Index:  NEW10 | DIRLEFT		| PIX0110
	ERROR,								// Index:  NEW10 | DIRLEFT		| PIX0111
	DIRLEFT	| PIX1100,				// Index:  NEW10 | DIRLEFT		| PIX1000
	DIRLEFT	| PIX1100,				// Index:  NEW10 | DIRLEFT		| PIX1001
	ERROR,								// Index:  NEW10 | DIRLEFT		| PIX1010
	ERROR,								// Index:  NEW10 | DIRLEFT		| PIX1011
	DIRLEFT	| PIX1100,				// Index:  NEW10 | DIRLEFT		| PIX1100
	DIRLEFT	| PIX1100,				// Index:  NEW10 | DIRLEFT		| PIX1101
	ERROR,								// Index:  NEW10 | DIRLEFT		| PIX1110
	ERROR,								// Index:  NEW10 | DIRLEFT		| PIX1111 is an error, b/c we are inside the shape.
														       
	ERROR,								// Index:  NEW10 | DIRUP		| PIX0000 is an error b/c we lost the shape.
	ERROR,								// Index:  NEW10 | DIRUP		| PIX0001
	ERROR,								// Index:  NEW10 | DIRUP		| PIX0010
	ERROR,								// Index:  NEW10 | DIRUP		| PIX0011
	DIRLEFT	| PIX1001,				// Index:  NEW10 | DIRUP		| PIX0100
	DIRLEFT	| PIX1001,				// Index:  NEW10 | DIRUP		| PIX0101
	DIRLEFT	| PIX1001,				// Index:  NEW10 | DIRUP		| PIX0110
	DIRLEFT	| PIX1001,				// Index:  NEW10 | DIRUP		| PIX0111
	ERROR,								// Index:  NEW10 | DIRUP		| PIX1000
	ERROR,								// Index:  NEW10 | DIRUP		| PIX1001
	ERROR,								// Index:  NEW10 | DIRUP		| PIX1010
	ERROR,								// Index:  NEW10 | DIRUP		| PIX1011
	ERROR,								// Index:  NEW10 | DIRUP		| PIX1100
	ERROR,								// Index:  NEW10 | DIRUP		| PIX1101
	ERROR,								// Index:  NEW10 | DIRUP		| PIX1110
	ERROR,								// Index:  NEW10 | DIRUP		| PIX1111 is an error, b/c we are inside the shape.
														       
														       
	ERROR,								// Index:  NEW11 | DIRRIGHT	| PIX0000 is an error b/c we lost the shape.
	DIRUP		| PIX0111,				// Index:  NEW11 | DIRRIGHT	| PIX0001
	ERROR,								// Index:  NEW11 | DIRRIGHT	| PIX0010
	DIRUP		| PIX0111,				// Index:  NEW11 | DIRRIGHT	| PIX0011
	ERROR,								// Index:  NEW11 | DIRRIGHT	| PIX0100
	ERROR,								// Index:  NEW11 | DIRRIGHT	| PIX0101
	ERROR,								// Index:  NEW11 | DIRRIGHT	| PIX0110
	ERROR,								// Index:  NEW11 | DIRRIGHT	| PIX0111
	ERROR,								// Index:  NEW11 | DIRRIGHT	| PIX1000
	DIRUP		| PIX0111,				// Index:  NEW11 | DIRRIGHT	| PIX1001
	ERROR,								// Index:  NEW11 | DIRRIGHT	| PIX1010
	DIRUP		| PIX0111,				// Index:  NEW11 | DIRRIGHT	| PIX1011
	ERROR,								// Index:  NEW11 | DIRRIGHT	| PIX1100
	ERROR,								// Index:  NEW11 | DIRRIGHT	| PIX1101
	ERROR,								// Index:  NEW11 | DIRRIGHT	| PIX1110
	ERROR,								// Index:  NEW11 | DIRRIGHT	| PIX1111 is an error, b/c we are inside the shape.
														       
	ERROR,								// Index:  NEW11 | DIRDOWN		| PIX0000 is an error b/c we lost the shape.
	ERROR,								// Index:  NEW11 | DIRDOWN		| PIX0001
	DIRRIGHT	| PIX1011,				// Index:  NEW11 | DIRDOWN		| PIX0010
	ERROR,								// Index:  NEW11 | DIRDOWN		| PIX0011
	ERROR,								// Index:  NEW11 | DIRDOWN		| PIX0100
	ERROR,								// Index:  NEW11 | DIRDOWN		| PIX0101
	DIRRIGHT	| PIX1011,				// Index:  NEW11 | DIRDOWN		| PIX0110
	ERROR,								// Index:  NEW11 | DIRDOWN		| PIX0111
	ERROR,								// Index:  NEW11 | DIRDOWN		| PIX1000
	ERROR,								// Index:  NEW11 | DIRDOWN		| PIX1001
	DIRRIGHT	| PIX1011,				// Index:  NEW11 | DIRDOWN		| PIX1010
	ERROR,								// Index:  NEW11 | DIRDOWN		| PIX1011
	ERROR,								// Index:  NEW11 | DIRDOWN		| PIX1100
	ERROR,								// Index:  NEW11 | DIRDOWN		| PIX1101
	DIRRIGHT	| PIX1011,				// Index:  NEW11 | DIRDOWN		| PIX1110
	ERROR,								// Index:  NEW11 | DIRDOWN		| PIX1111 is an error, b/c we are inside the shape.
														       
	ERROR,								// Index:  NEW11 | DIRLEFT		| PIX0000 is an error b/c we lost the shape.
	ERROR,								// Index:  NEW11 | DIRLEFT		| PIX0001
	ERROR,								// Index:  NEW11 | DIRLEFT		| PIX0010
	ERROR,								// Index:  NEW11 | DIRLEFT		| PIX0011
	ERROR,								// Index:  NEW11 | DIRLEFT		| PIX0100
	ERROR,								// Index:  NEW11 | DIRLEFT		| PIX0101
	ERROR,								// Index:  NEW11 | DIRLEFT		| PIX0110
	ERROR,								// Index:  NEW11 | DIRLEFT		| PIX0111
	DIRDOWN	| PIX1110,				// Index:  NEW11 | DIRLEFT		| PIX1000
	DIRDOWN	| PIX1110,				// Index:  NEW11 | DIRLEFT		| PIX1001
	ERROR,								// Index:  NEW11 | DIRLEFT		| PIX1010
	ERROR,								// Index:  NEW11 | DIRLEFT		| PIX1011
	DIRDOWN	| PIX1110,				// Index:  NEW11 | DIRLEFT		| PIX1100
	DIRDOWN	| PIX1110,				// Index:  NEW11 | DIRLEFT		| PIX1101
	ERROR,								// Index:  NEW11 | DIRLEFT		| PIX1110
	ERROR,								// Index:  NEW11 | DIRLEFT		| PIX1111 is an error, b/c we are inside the shape.
														       
	ERROR,								// Index:  NEW11 | DIRUP		| PIX0000 is an error b/c we lost the shape.
	ERROR,								// Index:  NEW11 | DIRUP		| PIX0001
	ERROR,								// Index:  NEW11 | DIRUP		| PIX0010
	ERROR,								// Index:  NEW11 | DIRUP		| PIX0011
	DIRLEFT	| PIX1101,				// Index:  NEW11 | DIRUP		| PIX0100
	DIRLEFT	| PIX1101,				// Index:  NEW11 | DIRUP		| PIX0101
	DIRLEFT	| PIX1101,				// Index:  NEW11 | DIRUP		| PIX0110
	DIRLEFT	| PIX1101,				// Index:  NEW11 | DIRUP		| PIX0111
	ERROR,								// Index:  NEW11 | DIRUP		| PIX1000
	ERROR,								// Index:  NEW11 | DIRUP		| PIX1001
	ERROR,								// Index:  NEW11 | DIRUP		| PIX1010
	ERROR,								// Index:  NEW11 | DIRUP		| PIX1011
	ERROR,								// Index:  NEW11 | DIRUP		| PIX1100
	ERROR,								// Index:  NEW11 | DIRUP		| PIX1101
	ERROR,								// Index:  NEW11 | DIRUP		| PIX1110
	ERROR,								// Index:  NEW11 | DIRUP		| PIX1111 is an error, b/c we are inside the shape.
	};

///////////////////////////////////////////////////////////////////////////
// Functions.
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
//
// Temporary displacement for rspRect while BLiT is busy with 3D.
// Currently rspRect isn't fully working for non 8 bit.
//
///////////////////////////////////////////////////////////////////////////
template <class COLOR>
inline
int16_t suxRect(			// Returns 0 on success, 1 if clipped out entirely.
	COLOR clr,			// Color to fill with.
	RImage* pimDst,	// Destination image.
	int16_t sX,			// X coordinate for rectangle.
	int16_t	sY,			// Y coordinate for rectangle.
	int16_t sW,			// Width for rectangle.
	int16_t sH)			// Height for rectangle.
	{
	int16_t	sRes	= 0;	// Assume success.

	if (sX < 0)
		{
		sW	+= sX;
		sX	= 0;
		}

	if ((sX + sW) >= pimDst->m_sWidth)
		{
		sW	= pimDst->m_sWidth - sX;
		}

	if (sY < 0)
		{
		sH	+= sY;
		sY	= 0;
		}
	
	if ((sY + sH) >= pimDst->m_sHeight)
		{
		sH	= pimDst->m_sHeight - sY;
		}

	if (sW > 0 && sH > 0)
		{
		int32_t		lPitch	= pimDst->m_lPitch;
		COLOR*	pclrRow	= (COLOR*)(pimDst->m_pData + sY * lPitch) + sX;
		COLOR*	pclrBlt;
		int16_t		sWidth;

		while (sH-- > 0)
			{
			pclrBlt	= pclrRow;
			sWidth	= sW;

			while (sWidth--)
				{
				*pclrBlt++	= clr;
				}

			pclrRow	=	(COLOR*)((U8*)pclrRow + lPitch);
			}
		}
	else
		{
		sRes	= 1;
		}

	return sRes;
	}

///////////////////////////////////////////////////////////////////////////
//
// Gets a pixel from pu8Data at sX, sY.
//
///////////////////////////////////////////////////////////////////////////
template <class COLOR>
inline
int16_t EvalPixel(						// Returns TRUE if pixel is not clrDisjoin.
	COLOR* pclrData,					// Pixel data.
	int16_t sX,							// X coordinate to check.
	int16_t sY,							// Y coordinate to check.
	int32_t lPitch,						// Pitch of pclrData data.
	int16_t	sMinX,						// Minimum value of x that is valid.
	int16_t sMinY,						// Minimum value of y that is valid.
	int16_t sMaxX,						// Maximum value of x that is valid.
	int16_t sMaxY,						// Maximum value of y that is valid.
	COLOR	clrDisjoin,					// Color that indicates disjoint.
	RLassoNextEvalCall fnEval)	// Optional function to use to evaluate 
											// the pixel.
	{
	int16_t	sNonDisjoin	= FALSE;

	if (sX >= sMinX && sY >= sMinY && sX <= sMaxX && sY <= sMaxY)
		{
		// If no callback . . .
		if (fnEval == NULL)
			{
			// Note that sY * lPitch is added in U8 sized elements and
			// sX is added in COLOR sized elements.
			if (*((COLOR*)((U8*)pclrData + (int32_t)sY * lPitch) + sX) != clrDisjoin)
				{
				sNonDisjoin	= TRUE;
				}
			}
		else
			{
			sNonDisjoin	= (*fnEval)(sX, sY);
			}
		}

	return sNonDisjoin;
	}

///////////////////////////////////////////////////////////////////////////
//
// Add sX to list sY's list.
//
///////////////////////////////////////////////////////////////////////////
inline int16_t Add(					// Returns 0 on success.
	SLIST_SHORTS* psls,			// Pointer to array of sorted row lists 
										// containing and sorted by x coordinate 
										// shorts.
	int16_t sX,						// X coordinate to add.
	int16_t sY,						// Y coordinate of row whose list to add to.
	int16_t	sStartY,					// Y from which our rows are relative.
	EXTENTS* pextents)			// Mins and maxes to update.
	{
	int16_t	sRes	= 0;	// Assume success.

	// Allocate item . . .
	int16_t*	psX	= new int16_t;
	if (psX != NULL)
		{
		// Copy.
		*psX	= sX;
		// Insert item, *psX, with sort key *psX.
		if (psls[sY - (sStartY + 1)].Insert(psX, psX) == 0)
			{
			// Success.
			if (sX > pextents->sMaxX)
				{
				pextents->sMaxX	= sX;
				}

			if (sX < pextents->sMinX)
				{
				pextents->sMinX	= sX;
				}

			if (sY > pextents->sMaxY)
				{
				pextents->sMaxY	= sY;
				}

			if (sY < pextents->sMinY)
				{
				pextents->sMinY	= sY;
				}
			}
		else
			{
			TRACE("Add(): Unable to insert short into list.\n");
			sRes	= -2;
			}

		// If an error occurred after allocation . . .
		if (sRes != 0)
			{
			delete psX;
			}
		}
	else
		{
		TRACE("Add(): Unable to allocate short for list.\n");
		sRes	= -1;
		}

	return sRes;
	}

///////////////////////////////////////////////////////////////////////////
//
// Lasso the next shape in an image.  If a shape is sliced by the sub 
// region specified, the shape will be sliced in output.  The shape will
// be removed (i.e., filled with the disjoining color) on successful return
// so that the next rspLassoNext() will return the next shape.
//
// METHOD:
// The shapes are scanned for left to right, top to bottom within the
// provided rectangle (sSrcX, sSrcY, sSrcW, sSrcH).
// Once a shape is found the current position is set such that a four pixel
// window on the buffer would have the pixel found in the scan in its
// lower, right corner.  From then on each pixel in the window is converted
// to a bit that is used, together with the last direction and the new pixel
// values in that direction, as an index into the ms_au16EdgeInfo map which 
// produces the next bit pattern for the window.  The intial value is composed
// with only the pixel in the lower, right set, the direction set to right,
// and the new pixel values as 0 and 0.
// As we follow the shape clockwise, everytime we go down, we add an x pos
// to our sorted list for that row, and, everytime we go up, we add an x
// pos to our sorted list for that row.
// Once we have followed the shape clockwise back to the start, we create
// image data in pimDst, if not already allocated, that is the size of
// the shape's minimum bounding rectangle.  pimDst is then filled with 
// clrDstEmptyColor.  
// Each pair of points in the list for each row is then used as a line 
// segment to be copied inclusively into pimDst from pimSrc.  This copy
// is, of course, clipped to pimDst.  As this copy occurs, we erase the
// shape in pimSrc so that the next rspLassoNext will scan right by it.
//
///////////////////////////////////////////////////////////////////////////
template <class COLOR>		// Can be U8, U16, or U32.
#ifdef WIN32	// Mac assumes extern.
	extern 
#endif // WIN32
int16_t rspLassoNext(	// Returns 0 if a polygon found,
									// 1 if no polygon found,
									// negative if an error occurred (most likely
									// allocation problems or image bit depth mis-
									// matches).
	RImage*	pimSrc,			// In:  Image to search in sub region sSrcX, sSrcY,
									// sSrcW, sSrcH.
	RImage*	pimDst,			// In/Out: Destination image.  If too small, polygon 
									// will be clipped.  If not yet allocated, will be
									// allocated to the correct minimum size.
	int16_t	sSrcX,				// In:  X coordinate of sub region to search.
	int16_t	sSrcY,				// In:  Y coordinate of sub region to search.
	int16_t	sSrcW,				// In:  Width of sub region to search.
	int16_t	sSrcH,				// In:  Height of sub region to search.
	COLOR	clrDisjoin,			// In:  Color that separates shapes.  This is the
									// color that, to this function.
									// Cast or use U8 for 8 bit, U16 for 16 bit,
									// or U32 for 32 bit.
	COLOR	clrDstEmpty,		// In:  Color that will be used to initialize 
									// pimDst, if pimDst is allocated by this function.
									// Type must be same size as clrDisjoinColor/COLOR.
	int16_t* psShapeX,			// Out: X coordinate of poly relative to pimSrc 0,0;
									// NOT relative to sSrcX.
	int16_t* psShapeY,			// Out: Y coordinate of poly relative to pimSrc 0,0;
									// NOT relative to sSrcY.
	int16_t* psShapeW,			// Out: Width of shape output to pimDst.
	int16_t* psShapeH,			// Out: Height of shape output to pimDst.
	RLassoNextEvalCall	fnEval)	// In:  Specifies function to call to determine
											// whether a pixel is part of a shape or not.
											// Values will be clipped before calling this
											// function.  If this is not NULL, it is used
											// instead of clrDisjoin.
	{
	int16_t	sRes	= 0;	// Assume none found.

	// If source bit depth is equal to provided bit depth . . .
	if (pimSrc->m_sDepth == sizeof(COLOR) * 8)
		{
		// If destination was preallocated . . .
		if (pimDst->m_pData != NULL)
			{
			// Make sure bit depth matches . . .
			if (pimDst->m_sDepth == sizeof(COLOR) * 8)
				{
				// Okay.
				}
			else
				{
				TRACE("rspLassoNext(): clrDisjoin implied bit depth of %d but "
					"preallocated pimDst has bit depth of %d.\n", 
					sizeof(clrDisjoin) * 8, 
					pimDst->m_sDepth);
				sRes	= -2;
				}
			}

		// If successful so far . . .
		if (sRes == 0)
			{
			//////////////////////////////////////////////////////////////////////
			// Scan for first/next shape.
			//////////////////////////////////////////////////////////////////////
			int32_t		lPitch	= pimSrc->m_lPitch;
			// Note that this addition takes COLOR's bits per pixel into
			// account.
			COLOR*	pclrRowStart	= (COLOR*)pimSrc->m_pData + sSrcX;
			// Note that the height offset is computed at U8* and THEN it
			// is casted to a COLOR*.
			pclrRowStart				= (COLOR*)((U8*)pclrRowStart + (int32_t)sSrcY * lPitch);
			// Our scanner.
			COLOR*	pclrSrc			= pclrRowStart;

			// Precalculate max extents.
			int16_t	sSrcX2	= sSrcX + sSrcW - 1;
			int16_t	sSrcY2	= sSrcY + sSrcH - 1;

			int16_t		sX;
			int16_t		sY;
			int16_t		sFoundShape	= FALSE;
			// Look for non-separator color.
			for (sY = 0; sY < sSrcH; sY++)
				{
				// Get row start position.
				pclrSrc	= pclrRowStart;
				// Init width.
				sX	= 0;
				// Scan for non-separator color.
				while (sFoundShape == FALSE)
					{
					if (sX < sSrcW)
						{
						if (EvalPixel((COLOR*)pimSrc->m_pData, sX, sY, lPitch, sSrcX, sSrcY, sSrcX2, sSrcY2, clrDisjoin, fnEval) == FALSE)
							{
							pclrSrc++;
							sX++;
							}
						else
							{
							sFoundShape	= TRUE;
							}
						}
					else
						{
						// End of row.
						break;
						}
					}

				// If we found a shape . . .
				if (sFoundShape != FALSE)
					{
					// Get out now.
					break;
					}

				// Get next row.  Add pitch at byte depth since it is a count
				// of bytes.
				pclrRowStart	= (COLOR*)((U8*)pclrRowStart + lPitch);
				}

			// Either we've exhausted the image or we've found a shape.
			if (sY < sSrcH)
				{
				///////////////////////////////////////////////////////////////////
				// Found one.
				///////////////////////////////////////////////////////////////////

				///////////////////////////////////////////////////////////////////
				// Trace shape clockwise generating mask.
				///////////////////////////////////////////////////////////////////

				// Compute maximum rows that can be used.
				int16_t	sMaxRows	= sSrcY2 - sY + 1;

				// Reposition sX, sY and record starting point.
				int16_t	sStartX	= --sX;
				int16_t	sStartY	= --sY;

				// Pointer to array of row lists of x coordinates.
				SLIST_SHORTS*	plistRows	= new SLIST_SHORTS[sMaxRows];

				if (plistRows != NULL)
					{
					// Store bounding rectangle.
					EXTENTS	extents	= { 0x7FFF, 0x7FFF, 0, 0 };

					// Default to the case we know will be for starting.
					U16	u16Last	= NEW00 | DIRRIGHT | PIX0001;
					
					do
						{
						// Get new values and move sX, sY dependent on direction.
						switch (u16Last & DIRMASK)
							{
							case DIRRIGHT:
								sX++;
								
								u16Last	|= ((EvalPixel((COLOR*)pimSrc->m_pData, (int16_t)(sX + 1), sY, lPitch, sSrcX, sSrcY, sSrcX2, sSrcY2, clrDisjoin, fnEval)					!= FALSE)	? NEW10 : NEW00)
											|	((EvalPixel((COLOR*)pimSrc->m_pData, (int16_t)(sX + 1), (int16_t)(sY + 1), lPitch, sSrcX, sSrcY, sSrcX2, sSrcY2, clrDisjoin, fnEval)	!= FALSE)	? NEW01 : NEW00);
								break;
							case DIRDOWN:
								sY++;
								
								u16Last	|= ((EvalPixel((COLOR*)pimSrc->m_pData, sX, (int16_t)(sY + 1), lPitch, sSrcX, sSrcY, sSrcX2, sSrcY2, clrDisjoin, fnEval)					!= FALSE)	? NEW10 : NEW00)
											|	((EvalPixel((COLOR*)pimSrc->m_pData, (int16_t)(sX + 1), (int16_t)(sY + 1), lPitch, sSrcX, sSrcY, sSrcX2, sSrcY2, clrDisjoin, fnEval)	!= FALSE)	? NEW01 : NEW00);

								ASSERT(sY - (sStartY + 1) < sMaxRows);
								ASSERT(sY - (sStartY + 1) >= 0);
								// When we go down we must add a position.
								sRes	= Add(plistRows, sX, sY, sStartY, &extents);

								break;
							case DIRLEFT:
								sX--;
								
								u16Last	|= ((EvalPixel((COLOR*)pimSrc->m_pData, sX, sY, lPitch, sSrcX, sSrcY, sSrcX2, sSrcY2, clrDisjoin, fnEval)					!= FALSE)	? NEW10 : NEW00)
											|	((EvalPixel((COLOR*)pimSrc->m_pData, sX, (int16_t)(sY + 1), lPitch, sSrcX, sSrcY, sSrcX2, sSrcY2, clrDisjoin, fnEval)	!= FALSE)	? NEW01 : NEW00);
								break;
							case DIRUP:
								ASSERT(sY - (sStartY + 1) < sMaxRows);
								ASSERT(sY - (sStartY + 1) >= 0);
								// When we go up we must add a position.
								sRes	= Add(plistRows, sX + 1, sY, sStartY, &extents);

								sY--;

								u16Last	|= ((EvalPixel((COLOR*)pimSrc->m_pData, sX, sY, lPitch, sSrcX, sSrcY, sSrcX2, sSrcY2, clrDisjoin, fnEval)					!= FALSE)	? NEW10 : NEW00)
											|	((EvalPixel((COLOR*)pimSrc->m_pData, (int16_t)(sX + 1), sY, lPitch, sSrcX, sSrcY, sSrcX2, sSrcY2, clrDisjoin, fnEval)	!= FALSE)	? NEW01 : NEW00);
								break;
							}

						// Get next.  Value mapped will always have NEW00 since these are the
						// unknown factor.
						u16Last = ms_au16EdgeInfo[u16Last];

						ASSERT((u16Last & ERROR) != ERROR);

						// Continue until we are back at start.
						} while ((sX != sStartX || sY != sStartY) && sRes == 0);

					// If successful so far . . .
					if (sRes == 0)
						{
						/////////////////////////////////////////////////////////////
						// Allocation destination if necessary.
						/////////////////////////////////////////////////////////////
						// If image not already allocated . . .
						if (pimDst->m_pData == NULL)
							{
							if (pimDst->CreateImage(
								extents.sMaxX - extents.sMinX + 1,	// Width: Use largest width of shape.
								extents.sMaxY - extents.sMinY + 1,	// Height: Use largest height of shape.
								pimSrc->m_type,			// Type: Use same as pimSrc.
								0,								// Pitch: 0 indicates to figure it out.
								pimSrc->m_sDepth)			// Bit depth per pixel: Use same as pimSrc.
								== 0)
								{
								// Successfully allocated image.
								}
							else
								{
								TRACE("rspLassoNext(): CreateImage failed for pimDst.\n");
								sRes	= -3;
								}
							}

						// If successful so far . . .
						if (sRes == 0)
							{
							//////////////////////////////////////////////////////////
							// Duplicate shape and contents.
							//////////////////////////////////////////////////////////
							// Row counter.
							int16_t sRow		= 0;
							// Clip to image height or shape height, whichever is smaller.
							int16_t	sHeight	= MIN(pimDst->m_sHeight, (int16_t)(extents.sMaxY - extents.sMinY + 1));
							// Pointer to line start position.
							int16_t*	psX1;
							// Pointer to line end position.
							int16_t*	psX2;
							// Clear destination.
#if 0		// Only 8 bit currently.
							rspRect(
#else
							suxRect(
#endif
								clrDstEmpty, 
								pimDst, 
								(int16_t)0, (int16_t)0, 
								pimDst->m_sWidth, pimDst->m_sHeight);

							// Loop for all rows.
							while (sRow < sHeight)
								{
								psX1	= plistRows[sRow].GetHead();
								psX2	= plistRows[sRow].GetNext();
								while (psX1 != NULL)
									{
									// For every start there must be an end.
									ASSERT(psX2 != NULL);
									
									// Copy.
									rspBlit(	pimSrc, 
												pimDst,
												*psX1,
												sRow + extents.sMinY,
												(*psX1 - extents.sMinX),
												sRow,
												*psX2 - *psX1 + 1, 1);	// Inclusive.

									// Blank original.
#if 0		// Only 8 bit currently.
								rspRect(
#else
								suxRect(
#endif
									clrDisjoin, 
									pimSrc, 
									*psX1, (int16_t)(extents.sMinY + sRow),
									(int16_t)(*psX2 - *psX1 + 1), 
									(int16_t)1);

									plistRows[sRow].Remove(psX1);
									plistRows[sRow].Remove(psX2);
									delete psX1;
									delete psX2;

									// Get next points.
									psX1	= plistRows[sRow].GetHead();
									psX2	= plistRows[sRow].GetNext();
									}

								sRow++;
								}

							SET(psShapeX, extents.sMinX);
							SET(psShapeW, extents.sMaxX - extents.sMinX);
							SET(psShapeY, extents.sMinY);
							SET(psShapeH, extents.sMaxY - extents.sMinY);

							// Success... or sludge.
							}
						}
					else
						{
						TRACE("rspLassoNext(): Hosen during process.  Done.\n");
						}

					// Get rid of list.
					delete []plistRows;
					}
				else
					{
					TRACE("rspLassoNext(): Failed to allocate %d lists needed for processing.\n",
						sMaxRows);
					sRes	= -2;
					}
				}
			else
				{
				// None found.
				sRes	= 1;
				}
			}
		}
	else
		{
		TRACE("rspLassoNext(): clrDisjoin implied bit depth of %d but "
			"pimSrc has bit depth of %d.\n", 
			sizeof(clrDisjoin) * 8, 
			pimSrc->m_sDepth);
		sRes	= -1;
		}

	return sRes;
	}

void InstantiateLasso(void);
void InstantiateLasso(void)
	{
	RImage im;
	// Instantiate U8 version.
	rspLassoNext(&im, &im, 0, 0, 0, 0, (U8)0, (U8)0, (int16_t*)NULL, (int16_t*)NULL, (int16_t*)NULL, (int16_t*)NULL, (RLassoNextEvalCall)NULL);
	// Instantiate U16 version.
	rspLassoNext(&im, &im, 0, 0, 0, 0, (U16)0, (U16)0, (int16_t*)NULL, (int16_t*)NULL, (int16_t*)NULL, (int16_t*)NULL, (RLassoNextEvalCall)NULL);
	// Instantiate U32 version.
	rspLassoNext(&im, &im, 0, 0, 0, 0, (U32)0, (U32)0, (int16_t*)NULL, (int16_t*)NULL, (int16_t*)NULL, (int16_t*)NULL, (RLassoNextEvalCall)NULL);
	}

//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
