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
// IdBank.cpp
// 
// History:
//		01/29/97 JMI	Started.
//
//		01/30/97	JMI	Removed the bug I put in Add() that access the Head of
//							the list to add to, whether or not it was a valid node.
//
//		01/30/97	JMI	Fixed bug in Remove().  In rush for demo.
//
//		02/24/97	JMI	Changed Add() to Insert() and created new Add() that
//							adds at the end.
//							Also, removed m_u16HeadUsedId.  It wasn't useful.
//
//		03/05/97	JMI	GetThingByID() now fails for IdNil without complaint.
//
//////////////////////////////////////////////////////////////////////////////
//
// See .H for details.
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// C Headers -- Must be included before RSPiX.h b/c RSPiX utilizes SHMalloc.
//////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// RSPiX Headers.
// If PATHS_IN_INCLUDES macro is defined, we can utilize relative
// paths to a header file.  In this case we generally go off of our
// RSPiX root directory.  System.h MUST be included before this macro
// is evaluated.  System.h is the header that, based on the current
// platform (or more so in this case on the compiler), defines 
// PATHS_IN_INCLUDES.  Blue.h includes system.h so you can include that
// instead.
///////////////////////////////////////////////////////////////////////////////
#include "RSPiX.h"

#ifdef PATHS_IN_INCLUDES

#else

#endif

//////////////////////////////////////////////////////////////////////////////
// Postal includes.
//////////////////////////////////////////////////////////////////////////////

#include "IdBank.h"

//////////////////////////////////////////////////////////////////////////////
// Module specific macros.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Module specific typedefs.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Exported (extern) variables.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Module specific (static) variables / Instantiate class statics.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Module specific (static) protos.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Functions.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Helper to insert an ID into a particular list.
//
//////////////////////////////////////////////////////////////////////////////
void CIdBank::Insert(	// Returns nothing.
	U16	u16Id,			// ID to insert.
	U16*	pu16IdHead)		// Head of list to add to.
	{
	// Point this node's next at the current head of the free list.
	m_aids[u16Id].u16IdNext			= *pu16IdHead;
	// If there is a head . . .
	if (*pu16IdHead != IdNil)
		{
		// Point current head node's prev at this node.
		m_aids[*pu16IdHead].u16IdPrev	= u16Id;
		}

	// Don't look back.
	m_aids[u16Id].u16IdPrev	= IdNil;

	// Make this node the new head of the list.
	*pu16IdHead								= u16Id;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Helper to add an ID to a particular list.
//
//////////////////////////////////////////////////////////////////////////////
void CIdBank::Add(	// Returns nothing.
	U16	u16Id,		// ID to add.
	U16*	pu16IdTail)	// Tail of list to add to.
	{
	// Point this node's prev at the current tail of the free list.
	m_aids[u16Id].u16IdPrev			= *pu16IdTail;
	// If there is a head . . .
	if (*pu16IdTail != IdNil)
		{
		// Point current tail node's next at this node.
		m_aids[*pu16IdTail].u16IdNext	= u16Id;
		}

	// Don't look forward.
	m_aids[u16Id].u16IdNext	= IdNil;

	// Make this node the new tail of the list.
	*pu16IdTail								= u16Id;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Helper to remove an ID from a particular list.
//
//////////////////////////////////////////////////////////////////////////////
void CIdBank::Remove(	// Returns nothing.
	U16	u16Id,			// ID to remove.
	U16*	pu16IdHead,		// Head of list to remove from.
	U16*	pu16IdTail)		// Tail of list to remove from.
	{
	// If this was the head . . .
	if (*pu16IdHead == u16Id)
		{
		// Make the head this node's next.
		*pu16IdHead	= m_aids[u16Id].u16IdNext;
		// If not the end . . .
		if (*pu16IdHead != IdNil)
			{
			// Make the new head's prev NIL.
			m_aids[*pu16IdHead].u16IdPrev	= IdNil;
			}
		}
	else
		{
		// Set prev's next to this ID's next.
		m_aids[m_aids[u16Id].u16IdPrev].u16IdNext	= m_aids[u16Id].u16IdNext;
		}

	// If this was the tail . . .
	if (*pu16IdTail == u16Id)
		{
		// Make the tail this node's prev.
		*pu16IdTail	= m_aids[u16Id].u16IdPrev;
		// If not the beginning . . .
		if (*pu16IdTail != IdNil)
			{
			// Make the new tail's next NIL.
			m_aids[*pu16IdTail].u16IdNext	= IdNil;
			}
		}
	else
		{
		// Set next's prev to this ID's prev.
		m_aids[m_aids[u16Id].u16IdNext].u16IdPrev	= m_aids[u16Id].u16IdPrev;
		}
	}

//////////////////////////////////////////////////////////////////////////////
//
// Resets all IDs to free.  This must be called before the first use of
// any of the rest of these functions.
//
//////////////////////////////////////////////////////////////////////////////
void CIdBank::Reset(void)
	{
	// Reset all IDs.
	// Initialize all IDs regardless of current contents.
	U16	u16Cur;
	U16	u16Prev;
	for (u16Cur	= 0, u16Prev = IdNil; u16Cur < NumIds; u16Cur++, u16Prev++)
		{
		m_aids[u16Cur].u16IdPrev	= u16Prev;
		m_aids[u16Cur].u16IdNext	= u16Cur + 1;
		m_aids[u16Cur].pthing		= NULL;
		}

	// Last item's next should indicate end.
	m_aids[u16Prev].u16IdNext		= IdNil;

	// Start at beginning for Free List head.
	m_u16HeadFreeId	= 0;

	// Start at end for Free List tail.
	m_u16TailFreeId	= NumIds - 1;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Get a unique ID and associate it with a thing (CThing, that is).
//
//////////////////////////////////////////////////////////////////////////////
short CIdBank::Get(	// Returns 0 on success.
	CThing*	pthing,	// In:  Thing that wants to get an ID and be put in
							// the ID table.
	U16*		pu16ID)	// Out: ID for this particular CThing.
	{
	short	sRes	= 0;	// Assume success.

	// Make sure there's one left . . .
	if (m_u16HeadFreeId != IdNil)
		{
		// Get ID.
		*pu16ID	= m_u16HeadFreeId;

		// Set IDs value.
		m_aids[*pu16ID].pthing		= pthing;

		// Remove from free list and get next head.
		m_u16HeadFreeId	= m_aids[*pu16ID].u16IdNext;
		}
	else
		{
		TRACE("GetUniqueID(): Out of IDs!\n");
		sRes	= -1;
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Take a unique ID and associate it with a thing (CThing).
//
//////////////////////////////////////////////////////////////////////////////
short CIdBank::Take(	// Returns 0 on success.
	CThing*	pthing,	// In:  Thing that wants to take an ID and be put in
							// the ID table.
	U16		u16ID)	// In:  ID for this particular CThing.
	{
	short	sRes	= 0;	// Assume success.

	// Range check.
	ASSERT(u16ID < NumIds);

	// Make sure the ID is available . . .
	if (m_aids[u16ID].pthing == NULL)
		{
		// Set IDs value.
		m_aids[u16ID].pthing		= pthing;

		// Remove from free list.
		Remove(u16ID, &m_u16HeadFreeId, &m_u16TailFreeId);
		}
	else
		{
		TRACE("TakeUniqueID(): ID not available!\n");
		sRes	= -1;
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Release ID and unregister thing associated with it.
//
//////////////////////////////////////////////////////////////////////////////
void CIdBank::Release(	// Returns nothing.
	U16		u16ID)		// ID to release.
	{
	// If a valid ID . . .
	if (u16ID != IdNil)
		{
		// Range check.
		ASSERT(u16ID < NumIds);
		// The ID should be in use.  If not, something has hosened.
		ASSERT(m_aids[u16ID].pthing != NULL);

		// Clear ID.
		m_aids[u16ID].pthing		= NULL;

		// Add to free list.
		Add(u16ID, &m_u16TailFreeId);
		}
	}

//////////////////////////////////////////////////////////////////////////////
//
// Get a CThing via its ID.
//
//////////////////////////////////////////////////////////////////////////////
short CIdBank::GetThingByID(	// Returns 0 on success.
	CThing**	ppthing,				// Out: Ptr to CThing identified by u16ID.
	U16		u16ID)				// In:  ID of thing to get.
	{
	short	sRes	= 0;	// Assume success.

	if (u16ID != IdNil)
		{
		// Range check.
		ASSERT(u16ID < NumIds);

		// Get thing.
		*ppthing	= m_aids[u16ID].pthing;
		
		// This ID should be used.
		if (*ppthing != NULL)
			{
			// Success.
			}
		else
			{
//			TRACE("GetThingByID(): No such ID.\n");
			sRes	= -2;
			}
		}
	else
		{
		*ppthing	= NULL;
		sRes	= -1;
		}

	return sRes;
	}

///////////////////////////////////////////////////////////////////////////////
// EOF
///////////////////////////////////////////////////////////////////////////////
