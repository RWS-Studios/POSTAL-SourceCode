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
//	flist.h
// 
// History:
//		06/29/97	MJR	Started.
//
////////////////////////////////////////////////////////////////////////////////
//
//	This file impliments a template for a fast linked list with little or no
// error checking.  It assumes that memory allocations will never fail and the
// interface is designed around the idea of an intelligent user.
//
// This list can handle any data type -- standard types, structs, classes,
// pointers -- anything.  The list is implimented as a doubly-linked list of
// nodes, where each node contains a copy of whatever data type was specified.
//
// Please note that each node CONTAINS A COPY OF THE SPECIFIED DATA.  If you
// insert a short, a new node containing a copy of the short is created.  If
// you insert a pointer, a new node containing a copy of the pointer is created.
// And MOST IMPORTANTLY, if you insert an object, a new node containing a COPY
// OF THE OBJECT is created!
//
// How is this copy created?  For standard types (short, int, pointers, etc.)
// the compiler makes copies the same way it always does -- a simple bitwise
// copy.  But for objects, the compiler calls the object's copy constructor.
// If an object doesn't define a copy constructor, the compiler will supply one.
// In most cases, the supplied copy constructor will simply perform a bitwise
// copy of the object (the specific rules are beyond the scope of this
// documentation).
//
// The point comes down to this: if a bitwise copy is adequate for your object,
// everything will work fine, but otherwise you should supply your own!
// This is a standard C++ issue, but it seemed worth noting.
// 
////////////////////////////////////////////////////////////////////////////////
#ifndef FLIST_H
#define FLIST_H


template <class T>
class RFList
	{
	//------------------------------------------------------------------------------
	// Types, enums, etc.
	//------------------------------------------------------------------------------
	private:
		// The basic node.
		class Node
			{
			public:
				T data;
				Node* pnext;
				Node* pprev;
				
				// The only constructor takes a reference to the data and uses it
				// to initialize the data contained in the node.  Doing the init
				// this particular way minimizes the number of times the data is
				// constructed/initialized (assuming it's a class).
				Node(const T& copydata)
					: data(copydata)
					{
					}
			};

	public:
		// This is primarily done to give the user a type that is not protected!
		// We need the user to have access to a Node*, but we don't want to give
		// the user access to a node.  This seemed the easiest way to accomplish it.
		typedef Node* Pointer;

	//------------------------------------------------------------------------------
	// Variables
	//------------------------------------------------------------------------------
	protected:
		Node* m_phead;
		Node* m_ptail;
		long m_lCount;
		
	//------------------------------------------------------------------------------
	// Functions
	//------------------------------------------------------------------------------
	public:
		////////////////////////////////////////////////////////////////////////////////
		// Default (and only) constructor
		////////////////////////////////////////////////////////////////////////////////
		RFList()
			{
			m_phead = 0;
			m_ptail = 0;
			m_lCount = 0;
			}

		////////////////////////////////////////////////////////////////////////////////
		// Destructor
		////////////////////////////////////////////////////////////////////////////////
		~RFList()
			{
			Reset();
			}

		////////////////////////////////////////////////////////////////////////////////
		// Reset list (removes all nodes and restores list to post-construction state).
		////////////////////////////////////////////////////////////////////////////////
		void Reset()
			{
			// Get rid of any nodes that still exist
			while (m_phead)
				RemoveHead();
			}

		////////////////////////////////////////////////////////////////////////////////
		// Get count of nodes in list
		////////////////////////////////////////////////////////////////////////////////
		long GetCount(void)
			{ return m_lCount; }

		////////////////////////////////////////////////////////////////////////////////
		// Get pointer to head of list.
		// Returns 0 if list is empty, pointer to head node otherwise.
		////////////////////////////////////////////////////////////////////////////////
		Node* GetHead(void)
			{ return m_phead; }

		////////////////////////////////////////////////////////////////////////////////
		// Get pointer to tail of list
		// Returns 0 if list is empty, pointer to tail node otherwise.
		////////////////////////////////////////////////////////////////////////////////
		Node* GetTail(void)
			{ return m_ptail; }

		////////////////////////////////////////////////////////////////////////////////
		// Get node after specified node.
		// Assumes specified node is valid!
		////////////////////////////////////////////////////////////////////////////////
		Node* GetNext(Node* p)
			{ return p->pnext; }

		////////////////////////////////////////////////////////////////////////////////
		// Get node before specified node.
		// Assumes specified node is valid!
		////////////////////////////////////////////////////////////////////////////////
		Node* GetPrev(Node* p)
			{ return p->pprev; }

		////////////////////////////////////////////////////////////////////////////////
		// Get data in head node
		// Assumes there is a head node!
		////////////////////////////////////////////////////////////////////////////////
		T& GetHeadData(void)
			{ return m_phead->data; }

		////////////////////////////////////////////////////////////////////////////////
		// Get data in tail node
		// Assumes there is a tail node!
		////////////////////////////////////////////////////////////////////////////////
		T& GetTailData(void)
			{ return m_ptail->data; }

		////////////////////////////////////////////////////////////////////////////////
		// Get data in specified node.
		// Assumes specified node is valid!
		////////////////////////////////////////////////////////////////////////////////
		T& GetData(Node* p)
			{ return p->data; }

		T& operator*(Node* p)
			{ return p->data; }

		////////////////////////////////////////////////////////////////////////////////
		// Create new node containing copy of specified data and insert at head of list.
		// Returns pointer to new node.
		////////////////////////////////////////////////////////////////////////////////
		Node* InsertHead(const T& data)
			{
			// Create new node using specified data
			Node* pnew = new Node(data);

			// Link into list
			pnew->pnext = m_phead;
			pnew->pprev = 0;
			if (m_phead)
				m_phead->pprev = pnew;
			else
				m_ptail = pnew;
			m_phead = pnew;
			
			// Adjust count
			m_lCount++;
			
			return pnew;
			}

		////////////////////////////////////////////////////////////////////////////////
		// Create new node containing copy of specified data and insert at tail of list.
		// Returns pointer to new node.
		////////////////////////////////////////////////////////////////////////////////
		Node* InsertTail(const T& data)
			{
			// Create new node using specified data
			Node* pnew = new Node(data);

			// Link into list
			pnew->pnext = 0;
			pnew->pprev = m_ptail;
			if (m_ptail)
				m_ptail->pnext = pnew;
			else
				m_phead = pnew;
			m_ptail = pnew;

			// Adjust count
			m_lCount++;

			return pnew;
			}

		////////////////////////////////////////////////////////////////////////////////
		// Create new node containing copy of specified data and insert in list
		// after specified node.
		// Assumes specified node is valid!
		// Returns pointer to new node.
		////////////////////////////////////////////////////////////////////////////////
		Node* InsertAfter(const T& data, Node* p)
			{
			ASSERT(p);
			
			// Create new node using specified data
			Node* pnew = new Node(data);

			// Link into list
			pnew->pnext = p->pnext;
			pnew->pprev = p;
			p->pnext = pnew;
			if (pnew->pnext)
				pnew->pnext->pprev = pnew;
			else
				m_ptail = pnew;

			// Adjust count
			m_lCount++;

			return pnew;
			}

		////////////////////////////////////////////////////////////////////////////////
		// Create new node containing copy of specified data and insert in list
		// before specified node.
		// Assumes specified node is valid!
		// Returns pointer to new node.
		////////////////////////////////////////////////////////////////////////////////
		Node* InsertBefore(const T& data, Node* p)
			{
			ASSERT(p);
			
			// Create new node using specified data
			Node* pnew = new Node(data);

			// Link into list
			pnew->pnext = p;
			pnew->pprev = p->pprev;
			p->pprev = pnew;
			if (pnew->pprev)
				pnew->pprev->pnext = pnew;
			else
				m_phead = pnew;

			// Adjust count
			m_lCount++;

			return pnew;
			}

		////////////////////////////////////////////////////////////////////////////////
		// Remove node at head of list.
		// The data in the node will be destroyed.  Note that if data is a pointer,
		// only the pointer is destroyed, NOT whatever it is pointing to!)
		////////////////////////////////////////////////////////////////////////////////
		void RemoveHead()
			{
			ASSERT(m_phead);

			// Unlink from list
			Node* p = m_phead;
			if (p->pnext)
				p->pnext->pprev = 0;
			else
				m_ptail = p->pprev;
			m_phead = p->pnext;

			// Destroy node and data
			delete p;

			// Adjust count
			m_lCount--;
			}

		////////////////////////////////////////////////////////////////////////////////
		// Remove node at tail of list.
		// The data in the node will be destroyed.  Note that if data is a pointer,
		// only the pointer is destroyed, NOT whatever it is pointing to!)
		////////////////////////////////////////////////////////////////////////////////
		void RemoveTail()
			{
			ASSERT(m_ptail);

			// Unlink from list
			Node* p = m_ptail;
			if (p->pprev)
				p->pprev->pnext = 0;
			else
				m_phead = p->pnext;
			m_ptail = p->pprev;

			// Destroy node and data
			delete p;

			// Adjust count
			m_lCount--;
			}

		////////////////////////////////////////////////////////////////////////////////
		// Remove specified node.
		// The data in the node will be destroyed.  Note that if data is a pointer,
		// only the pointer is destroyed, NOT whatever it is pointing to!)
		////////////////////////////////////////////////////////////////////////////////
		void Remove(Node* p)
			{
			ASSERT(p);

			// Unlink from list
			if (p->pnext)
				p->pnext->pprev = p->pprev;
			else
				m_ptail = p->pprev;
			if (p->pprev)
				p->pprev->pnext = p->pnext;
			else
				m_phead = p->pnext;

			// Destroy node and data
			delete p;

			// Adjust count
			m_lCount--;
			}
	};


#endif // FLIST_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
