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
//
// queue.h
// 
// History:
//		06/12/94 MR		Added this comment block and made things safe
//							for interrupt-driven usage.
//
//		06/14/94 MR		Added function to remove the item at the tail of
//							the queue.
//
//		06/15/94 MR		Cleaned up the overall documentation to explain
//							how interrupt-driven usage works with the queue.
//
//		06/13/95	JMI	Converted to a template in attempt to make this class
//							more useful.  This class used to be defined in a .CPP
//							but that proves too cumbersome for templates.
//
//		07/25/96	JMI	PeekTail had a bug where it was returning the next after
//							the tail.
//
//		10/30/96	JMI	Changed:
//							Old label:		New label:
//							=========		=========
//							CQueue			RQueue
//
//////////////////////////////////////////////////////////////////////////////
//
// This object implements a generic queue of void pointers.
//
// The queue GENERALLY works properly even if it is being accessed by both
// interrupt-driven and normal (non-interrupt-driven) code at the same time.
//
// A typical application would be for normal code to add items to the queue
// and for interrupt-driven code to remove those items and process them.
// Alternately, the interrupt code could add the items and the normal code
// could remove them.  Either way, this queue basically works as expected.
//
// However, there are a few specific functions which could yield innacurate
// results or even cause major problems in such an environment!  Each function
// is documented as to whether it will work or what the potential problems are.
//
// For example, let's say normal code is adding items to the queue and
// interrupt-driven code is removing them.  Now let's say that the normal code
// calls NumItems() to see how many items are in the queue.  If an interrupt
// were to occur right after that call, then the number that it returned
// would no longer be valid.  This is, of course, a typical problem in an
// interrupt-driven environment.  On the other hand, NumItems() really does
// have a specific problem!  If an interrupt were to occur while NumItems()
// was calculating the number of items in the queue, and the queue happened
// to be in a certain state with regards to the head and tail "wrapping
// around" past the end, then the calculated number of items could be
// completely wrong, and could even be returned as a negative number!
//
// A work-around for this is to call NumItems() repeatedly until it returns
// the same value twice in a row.  This value will be a valid number,
// although it still only refers to the state that the queue was in at
// that time!  This solution assumes that interrupts are not occuring
// relentlessly at an extremely high rate.  In other words, if the value
// returned by two successive calls is different, then it is due to an
// interrupt having occurred sometime during the course of those two calls.
// As long as interrupts are occuring at a reasonable rate, we can be
// reasonably sure that another won't occur in the time it takes to perform
// one or two more calls to NumItems().  Once again, these are typical
// problems and solutions in interrupt-driven environments.
//
// The head and tail indices are declared as volatile so that the compiler
// will not optimize the usage of those variables.  This is important because
// you wouldn't want the compiler to put those variables into registers and
// make changes to them and then later write them back, because interrupt-
// driven code could have changed the same variable in the meantime!  There
// currently aren't any situations where this would matter in this code, but
// it seems safer to make them volatile....just in case.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef QUEUE_H
#define QUEUE_H

// Size of our queue.
#define QSIZE			(sizeof(m_aptQ) / sizeof(m_aptQ[0]))

// When given a queue index, these macro return the "next" or "previous" index,
// taking warp-around into account.  Note that the given index must NEVER be
// modified, even temporarily, because that could cause bugs when the queue is
// used by interrupt-driven and normal code at the same time.
#define QNEXT(i) 		(((i)+(short)1) >= QSIZE ? (short)0 : ((i)+(short)1))
#define QPREV(i)		(((i)-(short)1) >= 0 ? ((i)-(short)1) : (QSIZE-(short)1))

template <class T, short sSize> class RQueue
	{
	public:
		// Default constructor.  You must call SetSize() before queue can be used.
		//   NOTICE: With mixed usage of normal and interrupt-driven code, this would
		//   typically be called from the normal code.  It could be called from
		//   interrupt, but it seems sort of weird.
		RQueue(void) { Reset(); }

		// Destructor.
		//   NOTICE: With mixed usage of normal and interrupt-driven code, this would
		//   typically be called from the normal code.  It could be called from
		//   interrupt, but it seems sort of weird.
		~RQueue(void) { }
		
		// Reset the queue, thereby losing track of everything that may be in it.
		//   NOTICE: No problems with mixed usage of normal and interrupt-driven code!
		void Reset(void)
			{
			// Head and tail both start at 0
			m_sHead = 0;
			m_sTail = 0;
			}
		
		// Enqueue the indicated item (0 = success, non-zero = didn't fit)
		//   NOTICE: No problems with mixed usage of normal and interrupt-driven code!
		short EnQ(T* pT)
			{
			// Make sure it isn't already full	
			if (IsNotFull())
				{
				// Add value at tail of queue, then increment tail (NOTE: Must be
				// done in this order to allow interrupt-driven usage.)
				m_aptQ[m_sTail] = pT;
				m_sTail = QNEXT(m_sTail);
				return 0;	// success!
				}
			else
				return 1;	// failure!
			}

		
		// Dequeue the next item (NULL = empty)
		//   NOTICE: No problems with mixed usage of normal and interrupt-driven code!
		T* DeQ(void)
			{
			// Check if there's anything to dequeue
			if (IsNotEmpty())
				{
				// Get value at head of queue, then increment head (NOTE: Must be
				// done in this order to allow interrupt-driven usage.)
				T* ptTemp = m_aptQ[m_sHead];
				m_sHead = QNEXT(m_sHead);
				return ptTemp;
				}
			else
				return NULL;
			}
		
		// Remove the item at the tail of the queue (NULL = empty).
		//   WARNING: NEVER call this under mixed usage of normal and interrupt-driven
		//   code!  It would only be safe if (1) called from normal code when interrupts
		//   are disabled or (2) from interrupt code when it is certain that normal code
		//   is not accessing the queue. 
		T* RemoveTail(void)
			{
			// Check if there's anything to remove
			if (IsNotEmpty())
				{
				// Decrement tail, then get the item that was at the tail.
				m_sTail = QPREV(m_sTail);
				return m_aptQ[m_sTail];
				}
			else
				return NULL;
			}
		
		// Check if queue is empty (returns TRUE if it's empty)
		//   NOTICE: No problems with mixed usage of normal and interrupt-driven code!
		short IsEmpty(void)
			{ return (short)(m_sHead == m_sTail); }
		
		// Check if queue is not empty (returns TRUE if it's not empty)
		//   NOTICE: No problems with mixed usage of normal and interrupt-driven code!
		short IsNotEmpty(void)
			{ return (short)(m_sHead != m_sTail); }
		
		// Check if queue is full (returns TRUE if it's full)
		//   NOTICE: No problems with mixed usage of normal and interrupt-driven code!
		short IsFull(void)
			{ return (short)(QNEXT(m_sTail) == m_sHead); }
		
		// Check if queue is not full (returns TRUE if it's not full)
		//   NOTICE: No problems with mixed usage of normal and interrupt-driven code!
		short IsNotFull(void)
			{ return (short)(QNEXT(m_sTail) != m_sHead); }
			
		// Get number of items currently in the queue
		//   WARNING: With mixed usage of normal and interrupt-driven code, this could
		//   yield innacurate results!  In such cases you can call this function repeatedly
		//   until it returns the same result two times in a row.  This works fine as long
		//   as interrupts aren't occuring relentlessly at an extremely high rate!
		short NumItems(void)
			{ return m_sTail >= m_sHead ? m_sTail - m_sHead : (m_sTail + QSIZE) - m_sHead; };
			
		// "Peek" at head item in the queue
		//   WARNING: Could yield inaccurate results if EnQ() or DeQ() are interrupt-driven!
		T* Peek(void) { return (IsNotEmpty() ? m_aptQ[m_sHead] : NULL); }
		
		// "Peek" at tail item in the queue.
		//   WARNING: Could yield inaccurate results if EnQ() or DeQ() are interrupt-driven!
		T* PeekTail(void) { return (IsNotEmpty() ? m_aptQ[QPREV(m_sTail)] : NULL); }

	protected:		
		volatile short m_sHead;		// volatile since it could change on interrupt!
		volatile short m_sTail;		// volatile since it could change on interrupt!
		T*					m_aptQ[sSize + 1];
	};
	
#endif // QUEUE_H
