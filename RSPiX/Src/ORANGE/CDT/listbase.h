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
// listbase.h
// 
// History:
//		07/09/96 JMI	Started.  Copied from list.h.  Making version of CBList
//							that basically supports types that aren't pointers.  This
//							requires a different calling interface.  CList will be
//							based on this.
//
//		10/09/96	JMI	CBList() constructor now has option to skip initialization
//							and automatic dealloaction.
//
//		10/30/96	JMI	Changed:
//							Old label:		New label:
//							=========		=========
//							CBList			RBList
//
//		01/05/97	JMI	Added standard forms of GetLogicalNext()/Prev().
//
//		03/31/97	JMI	Remove() had default parameter that was inappropriate for
//							other than pointer types.  Changed it to overload with
//							no parameters.
//
//////////////////////////////////////////////////////////////////////////////
//
// This module provides dynamic linked list services.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef H_LISTBASE
#define H_LISTBASE

// If PATHS_IN_INCLUDES macro is defined, we can utilized relative
// paths to a header file.  In this case we generally go off of our
// RSPiX root directory.  System.h MUST be included before this macro
// is evaluated.  System.h is the header that, based on the current
// platform (or more so in this case on the compiler), defines 
// PATHS_IN_INCLUDES.  Blue.h includes system.h so you can include that
// instead.
#include "Blue.h"

template <class T> class RBList
	{
	protected:	// Internal types.
		typedef T LISTDATA, *PLISTDATA; 

		typedef struct	tagNODE
			{
			LISTDATA	ldData;
			tagNODE*	pNext;
			tagNODE*	pPrev;
			} NODE, *PNODE, *NODELIST;

	public:
		// Add a node to the list at the tail (it will be the new tail)
		short AddTail(LISTDATA ldNew)
			{
			short sRes = 0; // Assume success.
			// Create new node.
			PNODE pnNew = new NODE;
			// If successful . . .
			if (pnNew != NULL)
				{
				// Point to provided data.
				pnNew->ldData = ldNew;
				pnNew->pNext = NULL;

				// If a tail exists . . .
				if (m_pnTail != NULL)
					{
					// Make the tail's next point to the new.
					m_pnTail->pNext = pnNew;
					// Make the new's previous point to the tail.
					pnNew->pPrev    = m_pnTail;
					}
				else
					{
					// Make the new's previous NULL.
					pnNew->pPrev	= NULL;
					// Make the head the tail, since, if no tail exists,
					// there is no head.
					m_pnHead			= pnNew;
					}                 
	
				// Update current node info.
				m_pnPrev		= pnNew->pPrev;
				m_pnCurrent = pnNew;
				m_pnNext		= pnNew->pNext;
				m_pnTail 	= pnNew;
				}
			else
				{
				TRACE("RBList::AddTail(): Unable to allocate new node.\n");
				sRes = -1;
				}

			return sRes;
			}
		
		// Insert a node at the Head (it will be the new head)
		short InsertHead(LISTDATA ldNew)
			{
			short sRes = 0; // Assume success.
			// Allocate new node.
			PNODE pnNew = new NODE;
			// If successful . . .
			if (pnNew != NULL)
				{
				// Point to data provided.
				pnNew->ldData = ldNew;
				pnNew->pPrev = NULL;
				// If there is currently a head node . . .
				if (m_pnHead != NULL)
					{
					// Point the head's previous to the new.
					m_pnHead->pPrev = pnNew;
					// Point the new's next to the head.
					pnNew->pNext    = m_pnHead;
					}
				else                     
					{
					// Make the new's next NULL.
					pnNew->pNext    = NULL;
					// Make the tail the new since, if there is no head,
					// there must not be a tail.
					m_pnTail        = pnNew;
					}

				// Update current node info.
				m_pnHead		= pnNew;
				m_pnPrev		= pnNew->pPrev;
				m_pnCurrent = pnNew;
				m_pnNext		= pnNew->pNext;
				}
			else
				{
				TRACE("RBList::InsertHead(): Unable to allocate new node.\n");
				sRes = -1;
				}

			return sRes;
			}


		// Insert ldNew after ldAfter
		short InsertAfter(LISTDATA ldAfter, LISTDATA ldNew)
			{
			short sRes = 0; // Assume success.
			// Make sure the list is not empty
			if (IsEmpty() == FALSE)
				{
				// Find the node to insert after.
				PNODE pnAfter = Find(ldAfter);
				// If such a node exists . . .
				if (pnAfter != NULL)
					{
					// Allocate new node.
					PNODE pnNew  = new NODE;
					// If successful . . .
					if (pnNew != NULL)
						{
						// Point to provided data.
						pnNew->ldData = ldNew;
						// Point next to node to insert after's next.
						pnNew->pNext = pnAfter->pNext;
						// If there is a next node . . .
						if (pnNew->pNext != NULL)
							{
							// Have the next's previous point to new.
							pnNew->pNext->pPrev = pnNew;
							}
						else
							{
							// Make new the tail.
							m_pnTail = pnNew;
							}

						// Have the new's previous point the one to insert after.
						pnNew->pPrev = pnAfter;
						// Have the node to insert after's next point to the new.
						pnAfter->pNext = pnNew;
						// Update current info.
						m_pnPrev		= pnNew->pPrev;
						m_pnCurrent = pnNew;       
						m_pnNext		= pnNew->pNext;
						}
					else
						{
						TRACE("RBList::InsertAfter(): Unable to allocate new node.\n");
						sRes = -3;
						}
					}
				else
					{
					TRACE("RBList::InsertAfter():  Unable to locate node to insert after.\n");
					sRes = -2;
					}
				}
			else
				{
				TRACE("RBList::InsertAfter():  The list is empty.\n");
				sRes = -1;
				}

			return sRes;
			}

		// Insert pnNew before lnBefore
		short InsertBefore(LISTDATA ldBefore, LISTDATA ldNew)
			{
			short sRes = 0; // Assume success.
			// Make sure the list is not empty
			if (IsEmpty() == FALSE)
				{
				// Find the node to insert before.
				PNODE pnBefore = Find(ldBefore);
				// If found a node . . .
				if (pnBefore != NULL)
					{
					// Allocate new node.
					PNODE pnNew = new NODE;
					// If successful . . .
					if (pnNew != NULL)
						{
						// Point to supplied data.
						pnNew->ldData = ldNew;
						// Point previous to node to insert before's previous.
						pnNew->pPrev = pnBefore->pPrev;
						// If there is a previous . . .
						if (pnNew->pPrev != NULL)
							{
							// Have previous' next point to new.
							pnNew->pPrev->pNext = pnNew;
							}
						else
							{
							// Make new the head.
							m_pnHead = pnNew;
							}
						// Point new's next to node to insert before.
						pnNew->pNext = pnBefore;
						// Point before's previous to new.
						pnBefore->pPrev = pnNew;
				
						// Update current info.
						m_pnPrev		= pnNew->pPrev;
						m_pnCurrent = pnNew;       
						m_pnNext		= pnNew->pNext;
						}
					else
						{
						sRes = -3;
						TRACE("RBList::InsertBefore():  Unable to allocate new node.\n");
						}
					}
				else
					{
					sRes = -2;
					TRACE("RBList::InsertBefore():  Unable to find node to insert before.\n");
					}
				}
			else
				{
				sRes = -1;
				TRACE("RBList::InsertBefore():  List is empty.\n");
				}

			return sRes;
			}

		// Add pnNew after current
		short Add(LISTDATA ldNew)
		{ return (m_pnCurrent ? InsertAfter(m_pnCurrent->ldData, ldNew) : 
				(m_pnNext ? InsertAfter(m_pnNext->ldData, ldNew) : AddTail(ldNew) ) ); }

		// Insert pnNew before current
		short Insert(LISTDATA ldNew)
		{ return (m_pnCurrent ? InsertBefore(m_pnCurrent->ldData, ldNew) : 
				(m_pnNext ? InsertBefore(m_pnNext->ldData, ldNew) : AddTail(ldNew) ) ); }

		// Remove current node from the list.
		short Remove(void)
			{
			return Remove(m_pnCurrent);
			}

		// Remove a node from the list.
		short Remove(LISTDATA ldRem)
			{
			return Remove(Find(ldRem));
			}

		short GetHead(			// Returns 0 on success.
			PLISTDATA pldHead)	// Where to store head data.
			{
			short	sRes	= 0;	// Assume success.

			m_pnCurrent = m_pnHead;
			// If there is a head . . .
			if (m_pnCurrent != NULL)
				{
				m_pnPrev = m_pnCurrent->pPrev; 
				m_pnNext = m_pnCurrent->pNext;

				// Store.
				*pldHead	= m_pnCurrent->ldData;
				}
			else
				{
				sRes = 1;
				}

			return sRes;
			}

		short GetTail(			// Returns 0 on success.
			PLISTDATA pldTail)	// Where to store tail data.
			{
			short	sRes	= 0;	// Assume success.

			// If there is a tail . . .
			m_pnCurrent = m_pnTail;
			if (m_pnCurrent != NULL)
				{
				m_pnPrev = m_pnCurrent->pPrev; 
				m_pnNext = m_pnCurrent->pNext;

				*pldTail	= m_pnCurrent->ldData;
				}
			else
				{
				sRes = 1;
				}

			return sRes;
			}

		// Get node following last GetX
		short GetNext(			// Returns 0 on success.
			PLISTDATA pldNext)	// Where to store next data.
			{
			short	sRes	= 0;	// Assume success.

			m_pnCurrent = m_pnNext;
			if (m_pnCurrent != NULL)
				{
				m_pnPrev = m_pnCurrent->pPrev; 
				m_pnNext = m_pnCurrent->pNext;

				*pldNext	= m_pnCurrent->ldData;
				}
			else
				{
				sRes = 1;
				}

			return sRes;
			}

		// Get node following ldData
		short GetNext(			// Returns 0 on success.
			LISTDATA ldData, 	// Node to get next of.
			PLISTDATA pldNext)	// Where to store next data.
			{
			short	sRes	= 0;	// Assume success.

			// Make sure the list is not empty
			if (IsEmpty() == FALSE)
				{
				// Attempt to find node.
				PNODE pn = Find(ldData);
				// If node found . . .
				if (pn != NULL)
					{
					// Make global previous node found.
					m_pnPrev		= pn;
					// Make global current node's next.
					m_pnCurrent = pn->pNext;
					// If new current exists . . .
					if (m_pnCurrent != NULL)
						{
						// Make global next current's next.
						m_pnNext	= m_pnCurrent->pNext;       
		      		// Return current's data.
						*pldNext = m_pnCurrent->ldData;
						}
					else
						{
						// There is no next to supplied node.
						sRes = 1;
						}
					}
				else
					{
					TRACE("RBList::GetNext(): Unable to find supplied node.\n");
					sRes = -2;
					}
				}
			else
				{
				TRACE("RBList::GetNext():  The list is empty.\n");
				sRes = -1;
				}

			return sRes;
			}

		// Get node logically following last GetX
		short GetLogicalNext(	// Returns 0 on success.
			PLISTDATA pldNext)	// Where to store next data.
			{
			short	sRes	= GetNext(pldNext);
			if (sRes != 0)
				{
				sRes	= GetHead(pldNext);
				}

			return sRes;
			}

		// Get node logically following ldData
		short GetLogicalNext(	// Returns 0 on success.
			LISTDATA ldData,	 	// Node to get next of.
			PLISTDATA pldNext)	// Where to store next data.
			{
			short	sRes	= GetNext(ldData, pldNext);
			if (sRes != 0)
				{
				sRes	= GetHead(pldNext);
				}

			return sRes;
			}

		// Get node preceding last GetX
		short GetPrev(			// Returns 0 on success.
			PLISTDATA pldPrev)	// Where to store previous data.
			{
			short	sRes	= 0;	// Assume success.

			m_pnCurrent = m_pnPrev;
			if (m_pnCurrent != NULL)
				{
				m_pnPrev = m_pnCurrent->pPrev; 
				m_pnNext = m_pnCurrent->pNext;

				*pldPrev	= m_pnCurrent->ldData;
				}
			else
				{
				sRes = 1;
				}

			return sRes;
			}

		// Get node preceding ldData
		short GetPrev(			// Returns 0 on success.
			LISTDATA ldData,	// Node to get previous of.
			PLISTDATA pldPrev)	// Where to store previous data.
			{
			short	sRes	= 0;	// Assume success.

			// Make sure the list is not empty.
			if (IsEmpty() == FALSE)
				{
				// Attempt to find node.
				PNODE pn = Find(ldData);
				// If node found . . .
				if (pn != NULL)
					{
					// Make global next found node.
					m_pnNext		= pn;
					// Make global current node's previous.
					m_pnCurrent = pn->pPrev;
					// If new current exists . . .
					if (m_pnCurrent)
						{
						// Make global previous current's previous.
						m_pnPrev	= m_pnCurrent->pPrev;       
		      		// Return current's data.
						*pldPrev	= m_pnCurrent->ldData;
						}
					else
						{
						// There is no previous to supplied node.
						sRes	= 1;
						}
					}
				else
					{
					TRACE("RBList::GetPrev():  Unable to find supplied node.\n");
					sRes	= -1;
					}
				}
			else
				{
				TRACE("RBList::GetPrev():  The list is empty.\n");
				sRes	= -2;
				}

			return sRes;
			}


		// Get node logically preceding last GetX
		short GetLogicalPrev(	// Returns 0 on success.
			PLISTDATA pldPrev)	// Where to store prev data.
			{
			short	sRes	= GetPrev(pldPrev);
			if (sRes != 0)
				{
				sRes	= GetTail(pldPrev);
				}

			return sRes;
			}

		// Get node logically preceding ldData
		short GetLogicalPrev(	// Returns 0 on success.
			LISTDATA ldData,	 	// Node to get prev of.
			PLISTDATA pldPrev)	// Where to store prev data.
			{
			short	sRes	= GetPrev(ldData, pldPrev);
			if (sRes != 0)
				{
				sRes	= GetTail(pldPrev);
				}

			return sRes;
			}

		short GetCurrent(		// Returns 0 on success.
			PLISTDATA	pldCur)	// Where to store current data.
			{
			short	sRes	= 0;	// Assume success.

			if (m_pnCurrent != NULL)
				{
				*pldCur	= m_pnCurrent->ldData;
				}
			else
				{
				sRes = 1;
				}

			return sRes; 
			}

		short IsEmpty(void)	// Returns TRUE if empty, FALSE otherwise.
			{ 
			return (m_pnHead == NULL) ? TRUE : FALSE; 
			}

		// Find the node with the value LISTDATA.
		PNODE Find(LISTDATA ldFind)
			{
			PNODE pn, pnTemp;
	
			// Start at head.
			pn = m_pnHead;
			while (pn && pn->ldData != ldFind)
				{
				pnTemp = pn;
				pn = pn->pNext;
				}                             

			return pn;               
			}

		void Reset(void)
			{ Free(); }

	public:
		RBList(
			short sInitialize	= TRUE)	// If this flag is FALSE, no initialization
												// or freeing will be done.  It will be the
												// user's responsibility!
			{
			if (sInitialize != FALSE)
				{
				m_pnHead    = NULL;  
				m_pnPrev		= NULL;
				m_pnCurrent = NULL;
				m_pnNext		= NULL;
				m_pnTail    = NULL;
				}

			m_sInitialize	= sInitialize;
			}

		~RBList()
			{
			if (m_sInitialize != FALSE)
				{
				// Free all nodes.
				Free();
				}
			}

	protected:

		// Removes supplied node.
		short Remove(			// Returns 0 on success.
			NODE*	pn)			// In:  Node to remove.
			{
			short	sRes	= 0;

			// Make sure the list is not empty
			if (IsEmpty() == FALSE)
				{
				// If we have a valid node to remove . . .
				if (pn != NULL)
					{
					// If there is a node previous to pn . . .
					if (pn->pPrev != NULL)
						{
						// Make pn's previous' next point to pn's next.
						pn->pPrev->pNext = pn->pNext;
						}
					else
						{
						// pn is the head...make pn's next the head.
						m_pnHead = pn->pNext;
						}
			
					// If there is a node after pn . . .
					if (pn->pNext != NULL)
						{
						// Make pn's next's previous point to pn's previous.
						pn->pNext->pPrev = pn->pPrev;
						}
					else
						{
						// pn is the tail...make pn's previous the tail.
						m_pnTail = pn->pPrev;
						}
			
					// Update current info.
					m_pnPrev		= pn->pPrev;
					m_pnCurrent = NULL;     
					m_pnNext		= pn->pNext;
			
					delete pn;
					}
				else
					{
					sRes = -2;
					TRACE("RBList::Remove():  Unable to find supplied node or no current node.\n");
					}
				}
			else
				{
				sRes = -1;
				TRACE("RBList::Remove():  The list is empty.\n");
				}

			return sRes;
			}

		// Free the entire list.
		void Free(void)
			{
			PNODE pn, pnTemp;

			// Start at the head.
			pn = m_pnHead;
			while (pn)
				{
				pnTemp = pn;
				pn = pn->pNext;

				delete pnTemp;  
				}                                            
	
			// Clear all node pointers.
			m_pnHead    = NULL;  
			m_pnPrev		= NULL;
			m_pnCurrent = NULL;
			m_pnNext		= NULL;
			m_pnTail    = NULL;
			}

		PNODE		m_pnHead;
		PNODE		m_pnPrev;
		PNODE		m_pnCurrent;
		PNODE		m_pnNext;
		PNODE		m_pnTail;
		short		m_sInitialize;		// TRUE if this item should handle intializing
											// and freeing the list and members.
	};
   
#endif // H_LISTBASE
//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
