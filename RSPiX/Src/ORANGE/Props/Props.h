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
// Props.H
// 
// History:
//		01/05/97 JMI	Started.
//
//		01/09/97	JMI	Removed #include <utility>.
//
//		01/14/97	JMI	Changed order of ItemType and KeyType in calls that
//							take both.  Also, RemoveProp() no longer complains
//							about removing props that didn't exist.  Who really
//							wants to check if a prop exists before removing it?
//
//		03/28/97	JMI	Removed all traces of STL.  IN PROGRESS!!!!!
//
//		03/31/97	JMI	Finished converting.  Note that the STL map was much 
//							better suited for this class than the regular "Jon Lists"
//							(as they've been called).  Perhaps RSPiX could use its
//							own implementation of map or a more advanced 'Jon List'
//							suite.  The major difference being the allocation of the
//							data in chunks that are larger than the actual nodes and,
//							in the case of a map, being able to access the data
//							actually allocated by the map.
//
//////////////////////////////////////////////////////////////////////////////
//
// This class keeps track of a group of 'properties' that are identified by
// a key.
// This class does not claim to be especially fast or memory efficient.  It
// is, however, an extremely simple to use way of adding dynamic data
// association to an object.
// A typical use of this class would be to descend from it so that the user
// can dynamically associate data with a class instance simply by calling
// AddProp() and SetProp().
// To clean up this class without complaint by its destructor, call Reset()
// from your classes destructor.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef PROPS_H
#define PROPS_H

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
	#include "ORANGE/CDT/List.h"
#else
	#include "list.h"
#endif

//////////////////////////////////////////////////////////////////////////////
// Macros.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Protos.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Typedefs.
//////////////////////////////////////////////////////////////////////////////
template <class ItemType, class KeyType>
class RProps
	{
	///////////////////////////////////////////////////////////////////////////
	// Typedefs.
	///////////////////////////////////////////////////////////////////////////
	public:
		typedef struct
			{
			ItemType	item;
			KeyType	key;
			} Node;

		typedef RList<Node> Container;

	///////////////////////////////////////////////////////////////////////////
	// Con/Destruction.
	///////////////////////////////////////////////////////////////////////////
	public:
		RProps()
			{
			}

		~RProps() 
			{
			// If there are still props . . .
			if (m_container.IsEmpty() == FALSE)
				{
				TRACE("~RProps(): Warning: Cleaning up unremoved props.\n");
				}

			Reset();
			}

	///////////////////////////////////////////////////////////////////////////
	// Methods.
	///////////////////////////////////////////////////////////////////////////
	public:

		// Adds a new property.
		short AddProp(		// Returns 0 on success.
								// Returns 1 if key already exists.
								// Returns -1 if error.
			KeyType	key,	// Key to identify item.
			ItemType	item)	// Item to add.
			{
			short	sRes	= 0;	// Assume success.

			// Verify key does not exist . . .
			if (Find(&key) == NULL)
				{
				// Insert item . . .
				sRes	= Add(key, item);
				}
			else
				{
				TRACE("AddProp(): Key already exists.  Not adding prop.\n");
				sRes	= 1;
				}

			return sRes;
			}
								
		// Replaces an existing or adds a new property.
		// Be careful with this.  You could inadvertently replace 
		// a prop that was supplied by another user.
		void SetProp(		// Returns 0 on success.
								// Returns -1 if error.
			KeyType	key,	// Key to identify old item.
			ItemType	item)	// New item.
			{
			// If the key already exists . . .
			Node*	pnode	= Find(&key);
			if (pnode != NULL)
				{
				// Change the value.
				pnode->item	= item;
				}
			else
				{
				// Insert item.
				Add(key, item);
				}
			}
								
		// Removes an existing property.
		void RemoveProp(	// Returns nothing.
			KeyType	key)	// Key of item to remove.
			{
			Remove(key);
			}


	///////////////////////////////////////////////////////////////////////////
	// Querries.
	///////////////////////////////////////////////////////////////////////////
	public:
		// Does the specified prop key exist?
		short IsProp(				// Returns TRUE if item exists, FALSE otherwise.
			KeyType		key)		// Key of item to query.
			{
			return (short)(Find(&key) != NULL);
			}

		// Get a property.
		short GetProp(				// Returns 0 on success.
										// Returns 1 if no item with specified key.
			KeyType		key,		// Key of item to get.
			ItemType*	pitem)	// Where to put item.
			{
			short	sRes	= 0;	// Assume success.

			// Find node by key . . .
			Node*	pnode	= Find(&key);
			if (pnode != NULL)
				{
				// Get item.
				*pitem	= pnode->item;
				}
			else
				{
				TRACE("GetProp(): Key does not exist.\n");
				sRes	= 1;
				}

			return sRes;
			}

	///////////////////////////////////////////////////////////////////////////
	// Internal functionality.
	///////////////////////////////////////////////////////////////////////////
	protected:

		// Find the node with the specified key.
		Node* Find(			// Returns the node with the specified key.
			KeyType* pkey)	// In:  Ptr to the key to search for.
			{
			Node*	pn	= m_container.GetHead();
			while (pn != NULL)
				{
				if (pn->key == *pkey)
					break;

				pn	= m_container.GetNext();
				}

			return pn;
			}

		// Add a node, as specified by the supplied item and key.
		short Add(			// Returns 0 on success.
								// Returns -1 if error.
			KeyType	key,	// Key to identify item.
			ItemType	item)	// Item to add.
			{
			short	sRes	= 0;	// Assume success.

			// Allocate node to add . . .
			Node*	pn	= new Node;
			if (pn != NULL)
				{
				pn->key	= key;
				pn->item	= item;
				if (m_container.AddTail(pn) == 0)
					{
					// Success.
					}
				else
					{
					TRACE("Add(): m_container.AddTail() failed.\n");
					sRes	= -2;
					}

				// If any errors occurred after allocation . . .
				if (sRes != 0)
					{
					delete pn;
					}
				}
			else
				{
				TRACE("Add(): Unable to allocate new Node.\n");
				sRes	= -1;
				}

			return sRes;
			}

		// Remove the node identified by the supplied key.
		short Remove(		// Returns 0 on success.
								// Returns -1 if error.
			KeyType	key)	// Key to identify item to remove.
			{
			short	sRes	= 0;	// Assume success.

			Node*	pn	= Find(&key);
			if (pn != NULL)
				{
				// The container's current item is the one we want to remove.
				if (m_container.Remove() == 0)
					{
					// Success.
					delete pn;
					}
				else
					{
					TRACE("Remove(): m_container.Remove() failed.\n");
					sRes	= -1;
					}
				}
			else
				{
				// No such node.
				sRes	= 1;
				}
			
			return sRes;
			}

		// Empty container of current contents.
		void Reset(void)
			{
			Node*	pn	= m_container.GetHead();
			while (pn != NULL)
				{
				m_container.Remove();
				
				delete pn;

				pn	= m_container.GetNext();
				}
			}

	///////////////////////////////////////////////////////////////////////////
	// Instantiable data.
	///////////////////////////////////////////////////////////////////////////
	public:
		Container	m_container;	// Contains the props.

	///////////////////////////////////////////////////////////////////////////
	// Static data.
	///////////////////////////////////////////////////////////////////////////
	public:
	};

#endif	// PROPS_H
//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
