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
// list.h
// 
// History:
//		06/23/94 JMI	Started.
//
//		06/13/95	JMI	Converted to a template in attempt to make this class
//							more useful.  This class used to be defined in a .CPP
//							but that proves too cumbersome for templates.
//
//		07/08/96	JMI	Converted to new CList that does not convert your 
//							template type into a poiter.
//
//		07/09/96	JMI	Converted back to CList that does convert your
//							template type into a pointer.  Now utilizes CBList<T>.
//
//		10/09/96	JMI	CList() constructor now has option to skip initialization
//							and automatic dealloaction which is passed on to CBList().
//
//		10/30/96	JMI	Changed:
//							Old label:		New label:
//							=========		=========
//							CList				RList
//							CBList			RBList
//
//		01/05/96	JMI	Added standard forms of GetLogicalNext()/Prev().
//
//////////////////////////////////////////////////////////////////////////////
//
// This module provides dynamic linked list services for pointers to types.
// Use RBList<T> (listbase.h) for non-pointer types such as longs or shorts.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef H_LIST
#define H_LIST

// If PATHS_IN_INCLUDES macro is defined, we can utilized relative
// paths to a header file.  In this case we generally go off of our
// RSPiX root directory.  System.h MUST be included before this macro
// is evaluated.  System.h is the header that, based on the current
// platform (or more so in this case on the compiler), defines 
// PATHS_IN_INCLUDES.  Blue.h includes system.h so you can include that
// instead.
#ifdef PATHS_IN_INCLUDES
	#include "ORANGE/CDT/listbase.h"
#else
	#include "listbase.h"
#endif // PATHS_IN_INCLUDES

template <class T> class RList : public RBList<T*>
	{
	protected:	// Internal types.
		typedef typename RList<T>::LISTDATA PLISTDATA;	// Rename so we remember it's always a ptr.

	public:
		// Get Head
		PLISTDATA GetHead(void)	// Returns data ptr of head of list.
			{
			PLISTDATA	pld;
			return (RBList<T*>::GetHead(&pld) == 0) ? pld : NULL;
			}

		// Get Tail
		PLISTDATA GetTail(void)	// Returns data ptr of tail of list.
			{
			PLISTDATA	pld;
			return (RBList<T*>::GetTail(&pld) == 0) ? pld : NULL;
			}

		// Get node following last GetX
		PLISTDATA GetNext(void)	// Returns data ptr of next in list.
			{
			PLISTDATA	pld;
			return (RBList<T*>::GetNext(&pld) == 0) ? pld : NULL;
			}

		// Get node following pldData
		PLISTDATA GetNext(		// Returns data ptr of next in list after pldData.
			PLISTDATA pldData)	// The data ptr to get the next of.  Blech.
			{
			PLISTDATA	pld;
			return (RBList<T*>::GetNext(pldData, &pld) == 0) ? pld : NULL;
			}

		// Get node logically following last GetX
		PLISTDATA GetLogicalNext(void)	// Returns data ptr of logical next in list.
			{
			PLISTDATA	pld;
			return (RBList<T*>::GetLogicalNext(&pld) == 0) ? pld : NULL;
			}

		// Get node logically following pldData
		PLISTDATA GetLogicalNext(	// Returns data ptr of logical next in list after pldData.
			PLISTDATA pldData)		// The data ptr to get the next of.  Blech.
			{
			PLISTDATA	pld;
			return (RBList<T*>::GetLogicalNext(pldData, &pld) == 0) ? pld : NULL;
			}

		// Get node preceding last GetX
		PLISTDATA GetPrev(void)	// Returns data ptr of prev in list.
			{
			PLISTDATA	pld;
			return (RBList<T*>::GetPrev(&pld) == 0) ? pld : NULL;
			}

		// Get node preceding pldData
		PLISTDATA GetPrev(		// Returns data ptr of prev in list after pldData.
			PLISTDATA pldData)	// The data ptr to get the prev of.  Blech.
			{
			PLISTDATA	pld;
			return (RBList<T*>::GetPrev(pldData, &pld) == 0) ? pld : NULL;
			}

		// Get node logically preceding last GetX
		PLISTDATA GetLogicalPrev(void)	// Returns data ptr of logical prev in list.
			{
			PLISTDATA	pld;
			return (RBList<T*>::GetLogicalPrev(&pld) == 0) ? pld : NULL;
			}

		// Get node logically preceding pldData
		PLISTDATA GetLogicalPrev(	// Returns data ptr of logical prev in list before pldData.
			PLISTDATA pldData)		// The data ptr to get the prev of.  Blech.
			{
			PLISTDATA	pld;
			return (RBList<T*>::GetLogicalPrev(pldData, &pld) == 0) ? pld : NULL;
			}

		PLISTDATA GetCurrent(void)
			{
			PLISTDATA pld;
			return (RBList<T*>::GetCurrent(&pld) == 0) ? pld : NULL; 
			}

		short IsEmpty(void)		// Returns TRUE if empty, FALSE otherwise.
			{ return RBList<T*>::IsEmpty(); }

		// Constructor to merely pass on arguments.
		RList(short sInitialize	= TRUE) :
			RBList<T*>(sInitialize)
			{
			}

	};
   
#endif // H_LIST
//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
