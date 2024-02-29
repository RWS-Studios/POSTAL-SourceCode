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
////////////////////////////////////////////////////////////////////////////////
//
//	fqueue.h
// 
// History:
//		08/30/97	MJR	Started.
//
//		09/01/97 MJR	Changed to use a counter to keep track of items in queue.
//
////////////////////////////////////////////////////////////////////////////////
//
//	This file impliments a template for a fast queue with little or no error
// checking.  It assumes that memory allocations will never fail and the
// interface is designed around the idea of an intelligent user.
//
// If you try to get an item from an empty queue or put an item to a full
// queue, YOU ARE A MORON AND THIS QUEUE WILL NOT PROTECT YOU FROM YOURSELF!!!
// If you fail to heed this warning, the queue will not crash, but you will
// definitely either overwrite existing values in the queue or retrive
// invalid values from the queue.
//
// Since we're all morons from time to time, the queue will ASSERT() in debug
// mode.
//
// This queue can handle any data type -- standard types, structs, classes,
// pointers -- anything.
//
// HOWEVER, because it is implimented as a simple array, it may not work well
// with all objects.  When this queue is constructed, it will construct N
// copies of your object, where N is the number of items you asked for.  This
// could be a problem if you, say, your class keeps track of all instances of
// itself, and you didn't expect or want there to be any "extra" instances.
// It all depends, but it certainly seemed worth noting here.
//
// In the future, this may be expanded to use STL-like techniques, whereby
// we would allocate raw memory for the objects, but we wouldn't construct
// an object until you actually added it to the queue, and likewise wouldn't
// destroy it until you actually removed it from the queue.
//
// When you add or remove elements from the queue, the contents of your
// objects are copied back and forth.  The objects in the queue are never
// destroyed until the entire queue is destroyed.
//
// Items are added and removed from the queue using operator=.  If you don't
// define a specific operator= for your object, the compiler will supply a
// default version, which typically does a bitwise copy of the data.  If this
// is not adequate for your object, you should supply your own.  This is a
// standard C++ issue, but it seemed worth noting.
// 
////////////////////////////////////////////////////////////////////////////////
#ifndef FQUEUE_H
#define FQUEUE_H


template <class T, long Items>
class FQueue
	{
	//------------------------------------------------------------------------------
	// Types, enums, etc.
	//------------------------------------------------------------------------------
	private:

	//------------------------------------------------------------------------------
	// Variables
	//------------------------------------------------------------------------------
	protected:
		T m_array[Items];									// Array of items
		T* m_pfirst;										// Pointer to the queue itself
		T* m_plast;											// Pointer to the last element of the array
		T* m_pfront;										// Pointer to front of queue
		T* m_pback;											// Pointer to back of queue
		size_t m_count;									// Number of items in the queue

	//------------------------------------------------------------------------------
	// Functions
	//------------------------------------------------------------------------------
	public:
		// Constructor
		FQueue()
			{
			Reset();
			}

		// Destructor
		~FQueue()
			{
			m_pfirst = 0;
			m_plast = 0;
			m_pfront = 0;
			m_pback = 0;
			m_count = 0;
			}

		// Reset to post-construction state
		void Reset(void)
			{
			m_pfirst = m_array;
			m_plast = m_array + (Items - 1);
			m_pfront = m_pfirst;
			m_pback = m_pfirst;
			m_count = 0;
			}

		// Determine whether queue is empty
		bool IsEmpty(void) const
			{
			return m_count == 0;
			}

		// Determine whether queue is full
		bool IsFull(void) const
			{
			return m_count == Items;
			}

		// Determine how many items can be put into the queue (in other words, how much space is left in the queue?)
		size_t HowManyCanBePut(void) const
			{
			return Items - m_count;
			}

		// Determine how many items can be gotten from the queue (in other words, how many items are in the queue?)
		size_t HowManyCanBeGotten(void) const
			{
			return m_count;
			}

		// Put an item at the back of the queue
		void PutAtBack(const T& item)
			{
			ASSERT(!IsFull());
			m_count++;
			*m_pback = item;
			if (++m_pback > m_plast)
				m_pback = m_pfirst;
			}

		// Get an item from the back of the queue
		const T& GetFromBack(void)
			{
			ASSERT(!IsEmpty());
			m_count--;
			if (--m_pback < m_pfirst)
				m_pback = m_plast;
			return *m_pback;
			}

		// Peek at the item at the back of the queue
		const T& PeekAtBack(void)
			{
			ASSERT(!IsEmpty());
			T* p = m_pback;
			if (--p < m_pfirst)
				p = m_plast;
			return *p;
			}

		// Put an item at the front of the queue
		void PutAtFront(const T& item)
			{
			ASSERT(!IsFull());
			m_count++;
			if (--m_pfront < m_pfirst)
				m_pfront = m_plast;
			*m_pfront = item;
			}

		// Get an item from the front of the queue
		const T& GetFromFront(void)
			{
			ASSERT(!IsEmpty());
			m_count--;
			T* p = m_pfront;
			if (++m_pfront > m_plast)
				m_pfront = m_pfirst;
			return *p;
			}

		// Peek at the item at the front of the queue
		const T& PeekAtFront(void)
			{
			ASSERT(!IsEmpty());
			return *m_pfront;
			}
	};


#endif // FQUEUE_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
