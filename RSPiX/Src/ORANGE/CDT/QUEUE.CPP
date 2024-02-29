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
// QUEUE.CPP
// 
// History:
//		06/12/94 MR		Added this comment block and made things safe
//						for interrupt-driven usage.
//
//		06/14/94 MR		Added function to remove the item at the tail of
//						the queue.
//
//		06/15/94 MR		Cleaned up the overall documentation to explain
//						how interrupt-driven usage works with the queue.
//
//		06/13/95	JMI	Converted to a template in attempt to make this class
//							more useful.  This particular file becomes unnecessary
//							at this point.  Do not bother including this CPP file
//							in your project (the entirity of this template class
//							is implemented in the .H file).
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

//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
