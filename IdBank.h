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
// IdBank.H
// 
// History:
//		01/29/97 JMI	Started.
//
//		02/24/97	JMI	Changed Add() to Insert() and created new Add() that
//							adds at the end.
//							Also, removed m_u16HeadUsedId.  It wasn't useful.
//
//////////////////////////////////////////////////////////////////////////////
//
// This API dispenses unique IDs to all those who request such until all the
// IDs are exhausted.  IDs must be released when done.
// Once an object gets an ID (which causes it to be registered in the ID
// lookup table), it can be referenced by that ID.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef IDBANK_H
#define IDBANK_H

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
#include "System.h"

#ifdef PATHS_IN_INCLUDES

#else

#endif

//////////////////////////////////////////////////////////////////////////////
// Postal Headers.
//////////////////////////////////////////////////////////////////////////////

#include "thing.h"

//////////////////////////////////////////////////////////////////////////////
// Macros.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Protos.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Typedefs.
//////////////////////////////////////////////////////////////////////////////

class CIdBank
	{
//////////////////////////////////////////////////////////////////////////////
	public:	// Macros.
//////////////////////////////////////////////////////////////////////////////
		enum
			{
			NumIds	= 10000,
			IdNil		= 0xFFFF
			};

//////////////////////////////////////////////////////////////////////////////
	public:	// Typedefs.
//////////////////////////////////////////////////////////////////////////////
		
		typedef struct
			{
			CThing*	pthing;		// CThing associate with this ID.
			U16		u16IdNext;	// Next ID in used or free list.
			U16		u16IdPrev;	// Prev ID in used or free list.
			} IdNode;

//////////////////////////////////////////////////////////////////////////////
	public:	// Con/Destruction.
//////////////////////////////////////////////////////////////////////////////

		// Default constructor.
		CIdBank()
			{
			Reset();
			}

		// Destructor.
		~CIdBank()
			{
			}

//////////////////////////////////////////////////////////////////////////////
	public:	// Public calls.
//////////////////////////////////////////////////////////////////////////////

		// Resets all IDs to free.  Called by constructor.
		void Reset(void);

		// Get a unique ID and associate it with a thing (CThing, that is).
		short Get(				// Returns 0 on success.
			CThing*	pthing,	// In:  Thing that wants to get an ID and be put in
									// the ID table.
			U16*		pu16ID);	// Out: ID for this particular CThing.

		// Take a unique ID and associate it with a thing (CThing).
		short Take(				// Returns 0 on success.
			CThing*	pthing,	// In:  Thing that wants to take an ID and be put in
									// the ID table.
			U16		u16ID);	// In:  ID for this particular CThing.

		// Release ID and unregister thing associated with it.
		void Release(			// Returns nothing.
			U16		u16ID);	// ID to release.

		// Get a CThing via its ID.
		short GetThingByID(	// Returns 0 on success.
			CThing**	ppthing,	// Out: Ptr to CThing identified by u16ID.
			U16		u16ID);	// In:  ID of thing to get.

//////////////////////////////////////////////////////////////////////////////
	protected:	// Internal calls.
//////////////////////////////////////////////////////////////////////////////

		// Helper to insert an ID into a particular list.
		void Insert(				// Returns nothing.
			U16	u16Id,			// ID to insert.
			U16*	pu16IdHead);	// Head of list to add to.
		
		// Helper to add an ID to a particular list.
		void Add(					// Returns nothing.
			U16	u16Id,			// ID to add.
			U16*	pu16IdTail);	// Tail of list to add to.
		
		// Helper to remove an ID from a particular list.
		void Remove(				// Returns nothing.
			U16	u16Id,			// ID to remove.
			U16*	pu16IdHead,		// Head of list to remove from.
			U16*	pu16IdTail);	// Tail of list to remove from.

//////////////////////////////////////////////////////////////////////////////
	protected:	// Internal variables.
//////////////////////////////////////////////////////////////////////////////

		// Bank of ID nodes.
		IdNode	m_aids[NumIds];

		// Head of Free IDs.
		U16		m_u16HeadFreeId;
		// Tail of Free IDs.
		U16		m_u16TailFreeId;

	};

#endif	// IDBANK_H
//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
