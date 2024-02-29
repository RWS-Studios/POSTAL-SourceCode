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
// DirtRect.CPP
// 
// History:
//		04/26/96 JMI	Started.
//
//		06/19/96	JMI	Added intialization constructors.
//
//		07/08/96	JMI	Converted to new CList that does not convert your 
//							template type into a poiter.
//
//		10/30/96	JMI	Changed:
//							Old label:		New label:
//							=========		=========
//							DRECT				RDRect
//							PDRECT			RDRect*
//							CDirtyRect		RDirtyRects	<-- Yes; now plural.
//							CList				RList
//
//		11/15/96	JMI	Add() was clipping the actual passed in RDRect.  Instead
//							we now clip an intermediate RDRect so that the user's 
//							will not be altered.
//
//		01/23/97	JMI	Clip() was not considering that an object might need to
//							be clipped on both edges (i.e., if the it needed shrink-
//							ing on top edge, it was assuming the bottom edge was
//							fine).
//
//////////////////////////////////////////////////////////////////////////////
//
// Combines rectangles into possibly more efficient larger rectangles.
// There are four options as to the operation of a RDirtyRects:
//	1) Minimum distance between rectangles in the x direction, default = 0
//		(m_sMinDistanceX).
// 2) Minimum distance between rectangles in the y direction, default = 0
//		(m_sMinDistanceY).
// 3) Clipping in the x direction, default = -1 (m_sClipX).
// 4) Clipping in the y direction, default = -1 (m_sClipY).
// -1 for any of the above members indicates to RDirtyRects to ignore that
// functionality (e.g., if m_sMinDistanceX == -1, no rectangles will be com-
//	bined).
//
// Use Add(...) to add a rectangle to the list.
// Use GetHead(), GetNext(), GetPrev(), and GetTail() to access the
// rectangles.
// Use Empty() to empty the list.
//
// FUTURE STRATEGIES (04/96):
// The Combine function could be optimized.  I better storage class (instead
// of list) could be used to somehow sort the rectangles (might not be worth
// small improvement for standard apps, but could be good for things that
// require many, many dirty rectangles).
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// C Headers.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Blue headers.
//////////////////////////////////////////////////////////////////////////////
#ifdef PATHS_IN_INCLUDES
	#include "BLUE/system.h"
#else
	#include "System.h"
#endif // PATHS_IN_INCLUDES

//////////////////////////////////////////////////////////////////////////////
// Green headers.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Orange headers.
//////////////////////////////////////////////////////////////////////////////
#ifdef PATHS_IN_INCLUDES
	#include "ORANGE/DirtRect/DirtRect.h"
#else
	#include "DirtRect.h"
#endif // PATHS_IN_INCLUDES

//////////////////////////////////////////////////////////////////////////////
// Yellow headers.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Module specific macros.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Module specific typedefs.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Module specific (static) variables.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Con/Destruction.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Constructor.
//
//////////////////////////////////////////////////////////////////////////////
RDirtyRects::RDirtyRects()
	{
	m_sMinDistanceX	= 0;
	m_sMinDistanceY	= 0;

	m_sClipX				= -1;
	m_sClipY				= -1;
	}

RDirtyRects::RDirtyRects(	
				short sMinDistX,				// Copied into m_sMinDistanceX.
				short sMinDistY,				// Copied into m_sMinDistanceY.
				short sClipX	/*= -1*/,	// Copied into m_sClipX.
				short	sClipY	/*= -1*/)	// Copied into m_sClipY.
	{
	m_sMinDistanceX	= sMinDistX;
	m_sMinDistanceY	= sMinDistY;

	m_sClipX	= sClipX;
	m_sClipY	= sClipY;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Destructor.
//
//////////////////////////////////////////////////////////////////////////////
RDirtyRects::~RDirtyRects()
	{
	Empty();
	}

//////////////////////////////////////////////////////////////////////////////
// Implementation.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Add a rectangle to the list of dirty rectangles.
// The rectangle may get combined with another.
// Returns 0 on success.
//
//////////////////////////////////////////////////////////////////////////////
short RDirtyRects::Add(RDRect* pdr)
	{
	short	sRes	= 0;	// Assume success.

	short		sClippedOut	= FALSE;
	RDRect	drTemp		= *pdr;	
	
	if (m_sClipX >= 0)
		{
		// If not clipped out in x direction . . .
		if (Clip(&drTemp.sX, &drTemp.sW, m_sClipX) == 0)
			{
			}
		else
			{
			sClippedOut	= TRUE;
			}
		}

	if (m_sClipY >= 0)
		{
		// If not clipped out in y direction . . .
		if (Clip(&drTemp.sY, &drTemp.sH, m_sClipY) == 0)
			{
			}
		else
			{
			sClippedOut	= TRUE;
			}
		}
	
	if (sClippedOut == FALSE)
		{
		if (Combine(&drTemp) == 0)
			{
			// Combined into an existing rectangle.
			}
		else
			{
			// No existing rectangle was close enough.
			RDRect*	pdrNew	= new RDRect;
			if (pdrNew != NULL)
				{
				if (RList<RDRect>::Add(pdrNew) == 0)
					{
					*pdrNew = drTemp;
					}
				else
					{
					TRACE("Add(): Unable to add *RDRect to list.\n");
					sRes = -2;
					}

				// If any errors occurred after allocation . . .
				if (sRes != 0)
					{
					delete pdrNew;
					}
				}
			else
				{
				TRACE("Add(): Unable to allocate new RDRect.\n");
				sRes = -1;
				}
			}
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Add a rectangle to the list of dirty rectangles.
// The rectangle may get combined with another.
// Returns 0 on success.
//
//////////////////////////////////////////////////////////////////////////////
short RDirtyRects::Add(short sX, short sY, short sW, short sH)
	{
	RDRect	dr	= { sX, sY, sW, sH };
		
	return Add(&dr);
	}

//////////////////////////////////////////////////////////////////////////////
//
// Empty the list of dirty rectangles.
//
//////////////////////////////////////////////////////////////////////////////
void RDirtyRects::Empty(void)
	{
	RDRect*	pdr	= GetHead();
	while (pdr != NULL)
		{
		Remove();
		delete pdr;

		pdr	= GetNext();
		}
	}

//////////////////////////////////////////////////////////////////////////////
//
// Combine a rectangle with rectangles already in
// the list if possible.
// Returns 0 if combined.
//
//////////////////////////////////////////////////////////////////////////////
short RDirtyRects::Combine(RDRect* pdr)
	{
	short	sRes	= 1;	// Assume not combined.

	// If combinable . . .
	if (m_sMinDistanceX >= 0)
		{
		RDRect*	pdrExisting	= GetHead();
		while (pdrExisting != NULL)
			{
			if (pdrExisting->sX - (pdr->sX + pdr->sW) <= m_sMinDistanceX)
				{
				if (pdr->sX < pdrExisting->sX + pdrExisting->sW + m_sMinDistanceX)
					{
					if (pdrExisting->sY - (pdr->sY + pdr->sH) <= m_sMinDistanceY)
						{
						if (pdr->sY < pdrExisting->sY + pdrExisting->sH + m_sMinDistanceY)
							{
							Expand(pdrExisting, pdr);
							
							// Remove temporarily.
							Remove();
							// This new rectangle may combine further.
							// If it does . . .
							if (Combine(pdrExisting) == 0)
								{
								// We can delete it.
								delete pdrExisting;
								}
							else
								{
								// Re-insert . . .
								if (RList<RDRect>::Add(pdrExisting) == 0)
									{
									// Success.
									}
								else
									{
									TRACE("Combine(): Unable to reAdd rectangle.\n");
									delete pdrExisting;
									}
								}
							
							sRes = 0;
							break;
							}
						}
					}
				}

			pdrExisting	= GetNext();
			}
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Expand pdrExpand by pdrNew.
//
//////////////////////////////////////////////////////////////////////////////
void RDirtyRects::Expand(RDRect* pdrExpand, RDRect* pdrNew)
	{
	short	sExpandRight	= pdrExpand->sX + pdrExpand->sW;
	short	sExpandBottom	= pdrExpand->sY + pdrExpand->sH;

	short sNewRight		= pdrNew->sX + pdrNew->sW;
	short	sNewBottom		= pdrNew->sY + pdrNew->sH;

	if (pdrNew->sX < pdrExpand->sX)
		{
		pdrExpand->sX	= pdrNew->sX;
		}

	if (pdrNew->sY < pdrExpand->sY)
		{
		pdrExpand->sY	= pdrNew->sY;
		}

	if (sNewRight > sExpandRight)
		{
		sExpandRight	= sNewRight;
		}

	if (sNewBottom > sExpandBottom)
		{
		sExpandBottom	= sNewBottom;
		}

	pdrExpand->sW	= sExpandRight		- pdrExpand->sX;
	pdrExpand->sH	= sExpandBottom	- pdrExpand->sY;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Clip sPos + sDistance to 0 to sClipDistance.
// Returns non-zero if clipped out.
//
//////////////////////////////////////////////////////////////////////////////
short RDirtyRects::Clip(short* psPos, short* psDistance, short sClipDistance)
	{
	short	sRes	= 0;	// Assume not clipped out.

	if (*psPos > sClipDistance)
		{
		sRes = 1;
		}
	else
		{
		if (*psPos < 0)
			{
			// Reduce by *psPos's underflow.
			*psDistance	+= *psPos;
			*psPos		= 0;

			// If clipped out . . .
			if (*psDistance <= 0)
				{
				sRes	= 1;
				}
			}

		// Get amount clipped out.
		short	sClippedEnd	= (*psPos + *psDistance) - sClipDistance;

		// If anything clipped out . . .
		if (sClippedEnd > 0)
			{
			*psDistance	-= sClippedEnd;

			// If clipped out . . .
			if (*psDistance <= 0)
				{
				sRes	= 1;
				}
			}
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
