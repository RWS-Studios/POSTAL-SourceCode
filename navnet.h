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
// navigationnet.h
// Project: Nostril (aka Postal)
//
// History:
//		01/28/97 BRH	Added navigation net objects as the thing that holds 
//							a group of bouys together in a navigational network.
//
//		03/13/97	JMI	Load now takes a version number.
//
//		06/30/97	JMI	Moved definitions of EditRect() and EditHotSpot() into 
//							navnet.cpp.
//
//		07/21/97	JMI	Added GetX(), GetY(), and GetZ().	
//
//		08/08/97 BRH	Added EditPostLoad function which adds the Nav Net name
//							to the editor's list box, just as it does when EditNew is
//							called.  This way the NavNets loaded from the realm file
//							are now correctly displayed.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef NAVIGATIONNET_H
#define NAVIGATIONNET_H

#include "RSPiX.h"
#include "realm.h"
#include "bouy.h"
#include <map>

// CNavigationNet is the class for navigation
class CNavigationNet : public CThing
	{
	//---------------------------------------------------------------------------
	// Types, enums, etc.
	//---------------------------------------------------------------------------
	public:

		#if _MSC_VER >= 1020 || __MWERKS__ >= 0x1100
			#if __MWERKS__ >= 0x1100
				ITERATOR_TRAITS(const CBouy*);
			#endif
			typedef map <UCHAR, CBouy*, less<UCHAR>, allocator<CBouy*> > nodeMap;
		#else
			typedef map <UCHAR, CBouy*, less<UCHAR> > nodeMap;
		#endif

	//---------------------------------------------------------------------------
	// Variables
	//---------------------------------------------------------------------------
	public:
		nodeMap	m_NodeMap;										// Map of ID's to CBouy nodes

	protected:
		double m_dX;												// x coord
		double m_dY;												// y coord
		double m_dZ;												// z coord
		UCHAR	 m_ucNextID;
		UCHAR	 m_ucNumSavedBouys;
		RImage* m_pImage;											// Pointer to only image (replace with 3d anim, soon)
		CSprite2 m_sprite;										// Sprite (replace with CSprite3, soon)
		RString  m_rstrNetName;									// Name of Nav Net
		TreeListNode m_BouyTreeListHead;						// Head of sorted list
		TreeListNode m_BouyTreeListTail;						// Tail of sorted list

		short m_sSuspend;											// Suspend flag

		// Tracks file counter so we know when to load/save "common" data 
		static short ms_sFileCount;

		// "Constant" values that we want to be able to tune using the editor

	//---------------------------------------------------------------------------
	// Constructor(s) / destructor
	//---------------------------------------------------------------------------
	protected:
		// Constructor
		CNavigationNet(CRealm* pRealm)
			: CThing(pRealm, CNavigationNetID)
			{
			m_pImage = 0;
			m_sSuspend = 0;
			m_ucNextID = 1;
			// Set yourself to be the new current Nav Net in the realm
			pRealm->m_pCurrentNavNet = this;
			// Set default name as NavNetxx where xx is CThing ID
			m_rstrNetName = "Untitled NavNet";
			// Initialize the dummy nodes for the sorted tree/list
			m_BouyTreeListHead.m_powner = NULL;
			m_BouyTreeListHead.m_pnPrev = NULL;
			m_BouyTreeListHead.m_pnRight = NULL;
			m_BouyTreeListHead.m_pnLeft = NULL;
			m_BouyTreeListHead.m_pnNext = &m_BouyTreeListTail;
			m_BouyTreeListTail.m_powner = NULL;
			m_BouyTreeListTail.m_pnPrev = &m_BouyTreeListHead;
			m_BouyTreeListTail.m_pnNext = NULL;
			m_BouyTreeListTail.m_pnRight = NULL;
			m_BouyTreeListTail.m_pnLeft = NULL;
			}

	public:
		// Destructor
		~CNavigationNet()
			{
			// Remove sprite from scene (this is safe even if it was already removed!)
			m_pRealm->m_scene.RemoveSprite(&m_sprite);

			// Free resources
			FreeResources();
			}

	//---------------------------------------------------------------------------
	// Required static functions
	//---------------------------------------------------------------------------
	public:
		// Construct object
		static short Construct(									// Returns 0 if successfull, non-zero otherwise
			CRealm* pRealm,										// In:  Pointer to realm this object belongs to
			CThing** ppNew)										// Out: Pointer to new object
			{
			short sResult = 0;
			*ppNew = new CNavigationNet(pRealm);
			if (*ppNew == 0)
				{
				sResult = -1;
				TRACE("CNavigationNet::Construct(): Couldn't construct CNavigationNet (that's a bad thing)\n");
				}
			return sResult;
			}

	//---------------------------------------------------------------------------
	// Required virtual functions (implimenting them as inlines doesn't pay!)
	//---------------------------------------------------------------------------
	public:
		// Load object (should call base class version!)
		short Load(													// Returns 0 if successfull, non-zero otherwise
			RFile* pFile,											// In:  File to load from
			bool bEditMode,										// In:  True for edit mode, false otherwise
			short sFileCount,										// In:  File count (unique per file, never 0)
			ULONG	ulFileVersion);								// In:  Version of file format to load.

		// Save object (should call base class version!)
		short Save(													// Returns 0 if successfull, non-zero otherwise
			RFile* pFile,											// In:  File to save to
			short sFileCount);									// In:  File count (unique per file, never 0)

		// Startup object
		short Startup(void);										// Returns 0 if successfull, non-zero otherwise

		// Shutdown object
		short Shutdown(void);									// Returns 0 if successfull, non-zero otherwise

		// Suspend object
		void Suspend(void);

		// Resume object
		void Resume(void);

		// Update object
		void Update(void);

		// Render object
		void Render(void);

		// Called by editor to init new object at specified position
		short EditNew(												// Returns 0 if successfull, non-zero otherwise
			short sX,												// In:  New x coord
			short sY,												// In:  New y coord
			short sZ);												// In:  New z coord

		// Called by editor to init nav net after it is loaded
		short EditPostLoad(void);

		// Called by editor to modify object
		short EditModify(void);									// Returns 0 if successfull, non-zero otherwise

		// Called by editor to move object to specified position
		short EditMove(											// Returns 0 if successfull, non-zero otherwise
			short sX,												// In:  New x coord
			short sY,												// In:  New y coord
			short sZ);												// In:  New z coord

		// Called by editor to update object
		void EditUpdate(void);

		// Called by editor to render object
		void EditRender(void);

		// Give Edit a rectangle around this object
		void EditRect(RRect* pRect);

		// Called by editor to get the hotspot of an object in 2D.
		void EditHotSpot(			// Returns nothiing.
			short*	psX,			// Out: X coord of 2D hotspot relative to
										// EditRect() pos.
			short*	psY);			// Out: Y coord of 2D hotspot relative to
										// EditRect() pos.

		// Get the coordinates of this thing.
		virtual					// Overriden here.
		double GetX(void)	{ return m_dX; }

		virtual					// Overriden here.
		double GetY(void)	{ return m_dY; }

		virtual					// Overriden here.
		double GetZ(void)	{ return m_dZ; }

		// Add a bouy to this network and assign it an ID
		UCHAR AddBouy(CBouy* pBouy);

		// Remove a bouy from the network
		void RemoveBouy(UCHAR ucBouyID);

		// Get the address of the Bouy with this ID
		CBouy* GetBouy(UCHAR ucBouy);

		// Find the bouy closest to this location in the world
		UCHAR FindNearestBouy(short sX, short sZ);

		// Preprocess the routing tables by pinging all nodes
		void UpdateRoutingTables(void);

		// Print the routing tables for debugging purposes
		void PrintRoutingTables(void);

		// Ping - return minimum number of hops from source to destination nodes
//		UCHAR Ping(UCHAR dst, UCHAR src, UCHAR depth, UCHAR maxdepth);
		UCHAR Ping(UCHAR dst, UCHAR src, UCHAR depth);

		UCHAR GetNumNodes(void)
			{ return m_ucNextID;}

		// Set this NavNet as the default one for the Realm.  
		short SetAsDefault(void)
			{  short sReturn = SUCCESS;
				if (m_pRealm)
				{
					m_pRealm->m_pCurrentNavNet = this;
				}
				else
					sReturn = FAILURE;
				return sReturn;
			}

		// Delete all bouys from this network.
		short DeleteNetwork(void);

	//---------------------------------------------------------------------------
	// Internal functions
	//---------------------------------------------------------------------------
	protected:
		// Get all required resources
		short GetResources(void);						// Returns 0 if successfull, non-zero otherwise
		
		// Free all resources
		short FreeResources(void);						// Returns 0 if successfull, non-zero otherwise
	};


#endif //DOOFUS_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
