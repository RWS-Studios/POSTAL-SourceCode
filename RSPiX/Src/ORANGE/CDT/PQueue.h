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
// PQueue.h
// 
// History:
//
//		10/18/96	JMI	Converted CQueue class to priority queue and made
//							dynamic using CSList.
//
//		10/30/96	JMI	Changed:
//							Old label:		New label:
//							=========		=========
//							CSList			RSList
//							CPQueue			RPQueue
//
//		02/18/97	JMI	Now can be used correctly with non-pointer types.
//
//////////////////////////////////////////////////////////////////////////////
//
// This object implements a generic queue of type T pointers with type K
// as sort key.  Please note when working on this file that the head of
// the list is currently the tail of the queue (see PROBLEMS/NOTES below).
// This class has equivalent functions for every callable function in 
// CQueue except for IsFull() and IsNotFull() b/c this queue is dynamically
// sized and is only full when you run out of memory in the heap/system.
//
// PROBLEMS/NOTES:
// = Because RSList defaults to sort ascending, we must make the head of our
// queue the tail of our list.  I think that ascending is a good default for
// RSList, I just think RSList should be alterable as to its sort direction
// without having to override the compare function or overload the < and >
// operators for type K.
// = Eventually RSList should be converted to using CBSList which does not yet
// exist.  Once that happens this class can become a generic queue of type
// T (instead of type T pointers) by utilizing CBSList instead of RSList.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef PQUEUE_H
#define PQUEUE_H

#include "slist.h"

// Node for RPQueue.
template <class T, class K> class RPQueueNode
	{
	public:
		T	t;
		K	k;
	};

template <class T, class K> class RPQueue : public RSList<RPQueueNode<T, K>, K>
	{
	public:	// Typedefs.

		typedef RPQueueNode<T, K>	Node;

	public:
		// Default constructor.
		RPQueue(short sInitialize	= TRUE) :
			RSList<Node, K>(sInitialize)
			{ 
			if (sInitialize != FALSE)
				{
				m_sNumItems	= 0;
				}
			}

		// Destructor.
		~RPQueue(void)
			{
			Empty();
			}

		// Enqueue the indicated item.
		short EnQ(	// Returns 0 on success.
			T* pT,	// In:  Pointer to item to add.
			K* pK)	// In:  Pointer to key to sort by (i.e., priority level).
			{
			// Create node.
			Node*	pnode	= new Node;
			// Copy values.
			pnode->t		= *pT;
			pnode->k		= *pK;
			// Add value starting at tail of queue (which, unfortunately, is
			// head of RSList for now).
			return Insert(pnode);
			}

		
		// Dequeue the next item (NULL = empty).
		bool DeQ(	// Returns true if item dequeued, false if empty.
			T*	pT)	// Out: Item dequeued on success.
			{
			// Get value at head of queue (which is, for now, the tail of
			// the list) and remove it.
			Node* pnode	= this->GetTail();
			if (pnode != NULL)
				{
				// Copy it.
				*pT	= pnode->t;
				// Remove it.
				Remove();
				// Delete it.
				delete pnode;

				return true;
				}

			return false;
			}
		
		// Remove the item at the tail of the queue (NULL = empty).
		bool RemoveTail(	// Returns true if item removed, false if empty.
			T*	pT)			// Out: Item removed on success.
			{
			Node* pnode	= this->GetHead();
			// Check if there's anything to remove . . .
			if (pnode != NULL)
				{
				// Copy it.
				*pT	= pnode->t;
				// Remove it.
				Remove();
				// Delete it.
				delete pnode;
		
				return true;
				}

			return false;
			}

		// Empty queue.
		void Empty(void)
			{
			T	t;
			while (DeQ(&t) == true)
				(void)0;
			}
		
		// Get number of items currently in the queue
		short NumItems(void)
			{ 
			return m_sNumItems; 
			}
			
		// "Peek" at head item in the queue
		bool Peek(	// Returns true if there is an item to peek at, false otherwise.
			T*	pT)	// Out: Item removed on success.
			{ 
			// Get value at head of queue (which is, for now, the tail of
			// the list) but don't remove it.
			Node*	pnode	= this->GetTail();
			if (pnode != NULL)
				{
				// Copy it.
				*pT	= pnode->t;

				return true;
				}

			return false;
			}
		
		// "Peek" at tail item in the queue.
		bool PeekTail(	// Returns true if there is an item to peek at, false otherwise.
			T*	pT)		// Out: Item removed on success.
			{
			// Get value at tail of queue (which is, for now, the head of
			// the list) but don't remove it.
			Node*	pnode	= this->GetHead();
			if (pnode != NULL)
				{
				// Copy it.
				*pT	= pnode->t;

				return true;
				}

			return false;
			}

	protected:	// The following functions should not be called by a RPQueue user
					// unless extending this class's functionality.

		// Hook Insert() to update count of items.
		short Insert(		// Returns 0 on success.
			Node* pnode)	// Pointer to add.
			{
			short	sRes	= 0;	// Assume success.
			// If able to insert into RSList . . .
			if (RSList<Node, K>::Insert(pnode, &(pnode->k)) == 0)
				{
				// Update count.
				m_sNumItems++;
				}
			else
				{
				TRACE("Insert(): RSList()::Insert() failed.\n");
				sRes	= -1;
				}

			return sRes;
			}

		// Hook Remove() to update count of items.
		short Remove(				// Returns 0 on success.
			Node* pnode = NULL)	// Item to remove or NULL to remove current.
			{
			short	sRes	= 0;	// Assume success.
			// If able to remove from RSList . . .
			if (RSList<Node, K>::Remove(pnode) == 0)
				{
				// Update count.
				m_sNumItems--;
				}
			else
				{
				TRACE("Remove(): RSList::Remove() failed.\n");
				sRes	= -1;
				}

			return sRes;
			}
		
	protected:
		short	m_sNumItems;
	};
	
#endif // PQUEUE_H
