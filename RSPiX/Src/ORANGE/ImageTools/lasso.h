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
//	Lasso.h
#ifndef LASSO_H
#define LASSO_H
//////////////////////////////////////////////////////////////////////////////
// Includes.
//////////////////////////////////////////////////////////////////////////////
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
	#include "GREEN/Image/Image.h"
#else
	#include "Image.h"
#endif // PATHS_IN_INCLUDES

//////////////////////////////////////////////////////////////////////////////
// Typedefs.
//////////////////////////////////////////////////////////////////////////////

// This callback is called to determine if a pixel is part of a shape or the
// empty space between shapes.  This function is never called for sX, sY pairs
// that are outside of the rectangle specified in the call to rspLassoNext.
typedef short (*RLassoNextEvalCall)(	// Returns TRUE if the specified pixel
													// is part of a shape.  FALSE if it is
													// the empty space between shapes.
	short sX,									// X coordinate of pixel in question.
													// Already clipped.
	short	sY);									// Y coordinate of pixel in question.
													// Already clipped.

//////////////////////////////////////////////////////////////////////////////
// Protos.
//////////////////////////////////////////////////////////////////////////////

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
template <class COLOR>		// Can be U8, U16, or U32.
#ifdef WIN32	// Mac assumes extern.
	extern 
#endif // WIN32
short rspLassoNext(	// Returns 0 if a polygon found,
									// 1 if no polygon found,
									// negative if an error occurred (most likely
									// allocation problems or image bit depth mis-
									// matches).
	RImage*	pimSrc,			// In:  Image to search in sub region sSrcX, sSrcY,
									// sSrcW, sSrcH.
	RImage*	pimDst,			// In/Out: Destination image.  If too small, polygon 
									// will be clipped.  If not yet allocated, will be
									// allocated to the correct minimum size.
	short	sSrcX,				// In:  X coordinate of sub region to search.
	short	sSrcY,				// In:  Y coordinate of sub region to search.
	short	sSrcW,				// In:  Width of sub region to search.
	short	sSrcH,				// In:  Height of sub region to search.
	COLOR	clrDisjoin,			// In:  Color that separates shapes.  This is the
									// color that, to this function.
									// Cast or use U8 for 8 bit, U16 for 16 bit,
									// or U32 for 32 bit.
	COLOR	clrDstEmpty,		// In:  Color that will be used to initialize 
									// pimDst, if pimDst is allocated by this function.
									// Type must be same size as clrDisjoinColor/COLOR.
	short* psShapeX,			// Out: X coordinate of poly relative to pimSrc 0,0;
									// NOT relative to sSrcX.
	short* psShapeY,			// Out: Y coordinate of poly relative to pimSrc 0,0;
									// NOT relative to sSrcY.
	short* psShapeW,			// Out: Width of shape output to pimDst.
	short* psShapeH,			// Out: Height of shape output to pimDst.
	RLassoNextEvalCall	fnEval);	// In:  Specifies function to call to determine
											// whether a pixel is part of a shape or not.
											// Values will be clipped before calling this
											// function.  If this is not NULL, it is used
											// instead of clrDisjoin.


#endif	// LASSO_H
//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
