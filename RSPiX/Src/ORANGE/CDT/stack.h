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
// stack.h
// 
// History:
//		08/14/95 JMI	Started.
//
//		07/08/96	JMI	No longer automatically makes user type a pointer.
//
//		07/08/96	JMI	Converted to new CList that does not convert your 
//							template type into a pointer.
//
//		07/09/96	JMI	Reconverted to newest CList which does convert
//							your template type into a pointer.  Now it CList
//							is based on CBList which does not do this conversion.
//
//		07/31/96	JMI	ReReconverted to CBList which does NOT convert
//							your template type into a pointer.
//
//		08/01/96 MJR	Bug fixed in Pop() - returned incorrect error code.
//
//		10/30/96	JMI	Changed:
//							Old label:		New label:
//							=========		=========
//							CBList			RBList
//							CStack			RStack
//
//////////////////////////////////////////////////////////////////////////////
//
// This module provides dynamic stack services.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef H_STACK
#define H_STACK

#include "Blue.h"
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


//////////////////////////////////////////////////////////////////////////////
// Typedefs.
//////////////////////////////////////////////////////////////////////////////

template <class T> class RStack
	{
	protected:	// Internal types.
		typedef T STACKDATA, *PSTACKDATA; 

	public:	// Construction/Destruction.
		// Default constructor.
		RStack()		{ m_sNumItems = 0; }
		// Destructor.
		~RStack()	{ }

//////////////////////////////////////////////////////////////////////////////

	public:	// Methods.
		// Push an item onto the stack.
		// Returns 0 on success.
		short Push(STACKDATA sd)
			{
			short	sRes	= m_list.InsertHead(sd);
			if (sRes == 0)
				{
				m_sNumItems++;
				}
			return sRes;
			}

		// Pop an item off the stack.
		// Returns item on success; NULL otherwise.
		short Pop(PSTACKDATA psd)
			{
			short	sRes	= m_list.GetHead(psd);
			if (sRes == 0)
				{
				m_sNumItems--;
				m_list.Remove();
				}
			return sRes;
			}

//////////////////////////////////////////////////////////////////////////////

	public:	// Querries.
		// Returns the number of items currently in stack.
		short GetNumItems(void)
			{ return m_sNumItems; }

		// Returns the top item.
		short GetTop(PSTACKDATA psd)
			{
			return m_list.GetHead(psd);
			}

//////////////////////////////////////////////////////////////////////////////

	protected:	// Internal functions.

//////////////////////////////////////////////////////////////////////////////

	public:	// Member variables.

	protected:	// Protected member variables.
		RBList <T>	m_list;	// The dynamic stack.  Head is top.
		short			m_sNumItems;	// Number of items in stack.
	};



#endif // H_STACK

//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
