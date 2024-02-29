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
// advqueue.h
// 
// History:
//		01/08/96	JMI	Started.
//
//		10/30/96	JMI	Changed:
//							Old label:		New label:
//							=========		=========
//							CAdvQueue		RAdvQueue
//							CQueue			RQueue
//
//////////////////////////////////////////////////////////////////////////////
//
// This adds some functionality to the generic queue template in 
// orange/cdt/queue.h.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef ADVQUEUE_H
#define ADVQUEUE_H

#include "queue.h"

template <class T, short sSize> class RAdvQueue	: public RQueue<T, sSize>
	{
	public:
		// Default constructor.
		RAdvQueue() { }
		// Destructor.
		~RAdvQueue() { }

	public:	// Querries.
		// Gets the nth element in the queue from the head.
		T*	Get(short n)
			{
			n	= (n > m_sHead) ? (sSize + (m_sHead - n)) : (m_sHead - n);
			return m_aptQ + n;
			}

		// Same as Get.  Just kind of silly.
		T* operator [](short n)
			{
			return Get(n);
			}

	};

#endif	// ADVQUEUE_H
