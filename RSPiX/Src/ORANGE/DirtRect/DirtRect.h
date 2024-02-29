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
#ifndef DIRTRECT_H
#define DIRTRECT_H

//////////////////////////////////////////////////////////////////////////////
// Headers.
//////////////////////////////////////////////////////////////////////////////
#include "System.h"

// If PATHS_IN_INCLUDES macro is defined, we can utilized relative
// paths to a header file.  In this case we generally go off of our
// RSPiX root directory.  System.h MUST be included before this macro
// is evaluated.  System.h is the header that, based on the current
// platform (or more so in this case on the compiler), defines 
// PATHS_IN_INCLUDES.  Blue.h includes system.h so you can include that
// instead.
#ifdef PATHS_IN_INCLUDES
	#include "ORANGE/CDT/List.h"
#else
	#include "list.h"
#endif // PATHS_IN_INCLUDES

//////////////////////////////////////////////////////////////////////////////
// Macros.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Typedefs.
//////////////////////////////////////////////////////////////////////////////

// This structure is used to encapsulate a rectangle used by RDirtyRects.
typedef struct
	{
	short	sX;
	short	sY;
	short	sW;
	short	sH;
	} RDRect;


//*******************************************************************
// NOTE: Do NOT call Insert, InsertHead, or AddTail unless you want
// a rectangle to be added w/o combination.  A rectangle added later
// may be combined with that one, however.
//*******************************************************************
class RDirtyRects : public RList<RDRect>
	{
	public:	// Typedefs.

	public:	// Con/Destruction.
		RDirtyRects();
		RDirtyRects(	short sMinDistX,		// Copied into m_sMinDistanceX.
						short sMinDistY,		// Copied into m_sMinDistanceY.
						short sClipX	= -1,	// Copied into m_sClipX.
						short	sClipY	= -1);// Copied into m_sClipY.
		~RDirtyRects();

	public:	// Implementation.
		// Use GetHead, GetTail, GetNext, and GetPrev to get the RRects
		// in no particular order.

		// Add a rectangle to the list of dirty rectangles.
		// The rectangle may get combined with another.
		// Returns 0 on success.
		short Add(RDRect* pdr);
		short Add(short sX, short sY, short sW, short sH);

		// Empty the list of dirty rectangles.
		void Empty(void);

	protected:	// Internal use only.
		// Combine a rectangle with rectangles already in
		// the list if possible.
		// Returns 0 if combined.
		short Combine(RDRect* pdr);

		// Expand pdrExpand by pdrNew.
		void Expand(RDRect* pdrExpand, RDRect* pdrNew);

		// Clip sPos + sDistance to 0 to sClipDistance.
		// Returns non-zero if clipped out.
		short Clip(short* psPos, short* psDistance, short sClipDistance);

	public:		// User may/should modify these.
		short	m_sMinDistanceX;	// Minimum x-distance between rectangles.
										// All rectangles closer will be combined.
		short	m_sMinDistanceY;	// Minimum y-distance between rectangles.
										// All rectangles closer will be combined.
		short	m_sClipX;			// Clip input rectangles in the x direction.
		short	m_sClipY;			// Clip input rectangles in the y direction.
	};

#endif // DIRTRECT
//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
