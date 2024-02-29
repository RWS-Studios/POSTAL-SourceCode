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
//-------------------------------------------------
// smash.h (grid based edition)
// Project: Postal
//
//	History:
//		05/26/97	JRD	Started.
//
//		06/04/97 JRD	Integrated with Postal for testing using NEW_SMASH project
//							setting so dependent files could still work with old smash
//
//		06/20/97	JRD	Removed backwards smash compatibility
//
//		07/03/97	JRD	Added an incremental tagged search to eliminate redundant
//							parts.  Implemented a system of ray tracking accross the grid.
//
//		07/05/97	JRD	Fixed MANY bugs in the line collision algorithms.  Added clipping.
//							Handled special cases of near vertical lines and vertical clipping.
//
//		07/07/97 JRD	Used a new class to implement fat smash objects without impacting 
//							performance - CFatSmash.
//
//		07/08/97	JMI	Added debug ASSERTs for "bridge's out" condition to help
//					BRH	us catch which CThings don't remove their smash(es).
//							Also, added release mode protection against "bridge's out"
//							condition.
//							Also, added bits for flags and flag bases.
//							Also, moved definition of ~CSmash() into smash.cpp b/c
//							of circular dependency (a Philips chain of command of
//							sorts) between smashatorium and realm.
//							Also, moved CSmash b/c Bill thinks it's better to keep
//							the destructor with the constructor or ease of findage.
//
//		07/10/97	JRD	Finally copleted and debugged ful support for Fat Smash 
//							objects.  The main criteria in determining what size is
//							"fat", is that "fat" objects shouldn't move very often.
//							NOTE that the n2 collision space in the new smash is
//							equivalent to NINE smash tiles, so if the tile size is
//							72, the collision areas will be 216 x 216!  So be careful!
//
//		08/08/97 BRH	Added a QuickCheck that checks for collision between two
//							given smashes.  This function computes the collision
//							based only on the distance and the radius so it doesn't
//							use the grid at all.  It provides a convenient way to
//							check to see if the weapon you are launching has cleared
//							your own smash region before arming it, as used in the
//							rockets and heatseekers.
//
//		08/12/97 BRH	Added a bit for ducking so that missiles won't hit you.
//
//		08/14/97 BRH	Added bits for special barrels that can't be destroyed
//							by enemy weapons, only CDude weapons.
//
//		08/18/97 BRH	Renamed Mext as Next.
//
//		09/02/97	JMI	Added Sentry bit.
//
//		09/03/97	JMI	Added Civilian bit.
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//  NOTE:  CURRENT USER LEVEL CHANGE FROM OLD SMASHATORIUM:  You must pass
//  four parameters to allocate a CSmashatorium (there is no default anymore.)
//  These parameters are the world X and Z size, and a grid size larger than 
//  any smash region.
////////////////////////////////////////////////////////////////////////////////
//  The new smashatorium attempts to partition lists of objects by location
//  Currently, each grid location only maintains a single hodge podge list as
//  the old Smash.  This may be enhanced (memory permitting) to split into 
//  multiple lists by type, or into multiple smashatoriums fo different grid
//  sizes so static objects could be contained in a fine grid.
////////////////////////////////////////////////////////////////////////////////
//
//	The current model demands that objects in the smashatorium exist in no more
// than four grid locations simultaneously.  Large smash objects used to detect
// objects-within-a-distance are hooked as a special type of smashee and use
// different collision routines.  Similarly, line collisions need higher level
// logic to march through the grid.  In short, it is a completely new 
// Smashatorium masquerading through the old API.
//
// If you MUST install a large object in the smash, it should be one that 
// rarely if ever moves.  If so, it can now be supported by a CFatSmash Object.
//
////////////////////////////////////////////////////////////////////////////////
//
//  The current Smashatorium "HIERARCHY OF OBJECTS"  :
//
//	CSmashLink -> A 128-bit struct which is the manipulation block for the grid.
//               It currently points back to it's CSmash parent and it's old grid
//				     location.  For efficiency, we could redundantly store the
//               smash bits here.
//
// CFatSmash -> An extention to CSmash that holds an overflow of CSmashLinks
//              for illegally big objects.
//
// CSmash -> One of more of these is held by actual game objects.  It contains
//				 a pointer back to the thing parent, a spherical collision region,
//           the smash bits, and four SmashLinks to track the corners of the
//           objects in the Smashatorium Grid.
//
// CSmashatoriumList -> manages each grid's linked list.  The grid is a 2d array
//								of these nodes.
//
// CSmashatorium -> Hold the grid and world clipping info.  Handles all the user
//						 functions.  Holds the state for the sequential checking state.
// 
////////////////////////////////////////////////////////////////////////////////
#ifndef SMASH_H
#define SMASH_H
#include "RSPiX.h"
#include "thing.h" // we are tying the nodes back to the things
#define NEW_SMASH	// We'll risk it!
////////////////////////////////////////////////////////////////////////////////
//		FORWARD DECLARATIONS
////////////////////////////////////////////////////////////////////////////////
class CSmashLink;
class CSmash;
class CSmashatoriumList;
class CSmashatorium;
class CFatSmash;

////////////////////////////////////////////////////////////////////////////////
//	 CSmashLink -> the node used in all the Smashatorium lists:
////////////////////////////////////////////////////////////////////////////////
class CSmashLink
	{
public:
	//---------------------------------------------------------------------------
	CSmashLink* m_pPrev;				// The link
	CSmashLink* m_pNext;				
	CSmash*	m_pParent;				// Access to bits
	CSmashatoriumList*	m_pLast;	// Where did it reside?
	// Might be prudent to store smash bits in here.
	//---------------------------------------------------------------------------
	void	Erase()
		{
		m_pPrev = m_pNext = NULL;
		m_pLast = NULL;
		m_pParent = NULL;
		}

	CSmashLink() { Erase(); }
	~CSmashLink() 
		{ 
		ASSERT(m_pPrev == NULL);
		ASSERT(m_pNext == NULL);
		ASSERT(m_pLast == NULL);

		Erase(); 
		}
	};

////////////////////////////////////////////////////////////////////////////////
//	 CFatSmash -> An extension to CSmash used for tracking oversized objects in
//						the Smashatorium:
////////////////////////////////////////////////////////////////////////////////
class CFatSmash
	{
public:
	CSmash*					m_pParent;		// Backwards pointer
	//--------------------------------------------------------------------
	short						m_sClipX;		// In tile relative to this fat
	short						m_sClipY;		// shows active grid based on clipping
	short						m_sClipW;		// Links outside of this region should
	short						m_sClipH;		// have NULL list pointers
	//--------------------------------------------------------------------
	short						m_sW;				// Actual size (in grids)
	short						m_sH;				// Actual Size (in grids)
	//--------------------------------------------------------------------
	short						m_sNumGrids;	// For convenience
	CSmashLink*				m_pLinks;		// 1D representation
	CSmashLink*				m_pFirstLink;	// offset into pLinks...
	//--------------------------------------------------------------------
	CSmashatoriumList*	m_pClippedGrid;// Start Grid in 'torium
	long						m_lX;				// See if it's moved!
	long						m_lY;
	//--------------------------------------------------------------------
	void	Erase();
	void	Destroy();
	CFatSmash() { Erase(); }
	~CFatSmash() { Destroy(); Erase(); }

	short	Alloc(short sNumGrids);
	};

////////////////////////////////////////////////////////////////////////////////
//  CSmash:  The user level object which describe the collision region:
////////////////////////////////////////////////////////////////////////////////
class CSmash
	{
	//---------------------------------------------------------------------------
	// Friends
	//---------------------------------------------------------------------------
	// friend class CSmashatorium; // do I need this?

	//---------------------------------------------------------------------------
	// Types, enums, etc.
	//---------------------------------------------------------------------------
	public:
		typedef U32 Bits;

		// Available bits for classifying a CSmash
		enum
			{
			Good				= 0x00000001,
			Bad				= 0x00000002,
			Character		= 0x00000004,
			Projectile		= 0x00000008,
			Misc				= 0x00000010,
			Dead				= 0x00000020,
			Barrel			= 0x00000040,
			Mine				= 0x00000080,
			Bouy				= 0x00000100,
			Fire				= 0x00000200,
			AlmostDead		= 0x00000400,
			Pylon				= 0x00000800,
			PowerUp			= 0x00001000,
			Flag				= 0x00002000,
			FlagBase			= 0x00004000,
			Ducking			= 0x00008000,
			SpecialBarrel	= 0x00010000,
			Sentry			= 0x00020000,
			Civilian			= 0x00040000
			};

	//---------------------------------------------------------------------------
	// Variables
	//---------------------------------------------------------------------------

	public:
		Bits m_bits;						// Bits indicating CSmash's classification
		CThing* m_pThing;					// Pointer to parental thing
		RSphericalRegion m_sphere;		// Will eventually be a base class like "CRegion"
		short	m_sInGrid;					// short cut to tell if in a grid...

		//---- these remain separate for fater access, since compilers SUCK
		long	m_lSearchTagCode;			// By using a 32bit value, we don't need to untag!
		CSmashLink	m_link1;
		CSmashLink	m_link2;
		CSmashLink	m_link3;
		CSmashLink	m_link4;

		CFatSmash*	m_pFat;					// Used in special case of fat smash object

	//---------------------------------------------------------------------------
	// Functions
	//---------------------------------------------------------------------------
	public:
		void	Erase()
			{
			m_bits = 0;
			m_sInGrid = FALSE;
			m_link1.Erase();	// Clear these out...
			m_link2.Erase();
			m_link3.Erase();
			m_link4.Erase();
			m_lSearchTagCode = 0; // Not searched upon!
			m_pFat = NULL;
			}

		CSmash();
		~CSmash();

	};

///////////////////////////////////////////////////////////////////////////////////
//	 CSmashatoriumList -> the node used in the Smashatorium Grid to hold each list
///////////////////////////////////////////////////////////////////////////////////
class	CSmashatoriumList
	{
public:
	//---------------------------------------------------------------------------
	CSmashLink	m_slHead;	// permanent imbedded bookends!
	CSmashLink	m_slTail;	// permanent imbedded bookends!
	short	m_sNum;
	//---------------------------------------------------------------------------
	void	Erase() // will NOT free any of the nodes in the list!
		{
		m_sNum = 0;
		m_slHead.m_pNext = &m_slTail;
		m_slTail.m_pPrev = &m_slHead;
		m_slHead.m_pPrev = m_slTail.m_pNext = NULL;
		}

	CSmashatoriumList()	{ Erase();	}
	~CSmashatoriumList()	
		{ 
		Erase();	
		// The book ends are never Remove()'d so we clean them up explicitly here.
		m_slHead.Erase();
		m_slTail.Erase();
		}
	};

///////////////////////////////////////////////////////////////////////////////////
//	 CSmashatorium -> Master of it all -> "the collision engine of the 90's!"
///////////////////////////////////////////////////////////////////////////////////
class CSmashatorium
	{
public:
	//---------------------------------------------------------------------------
	short	m_sWorldW;	// for general logic
	short m_sWorldH;
	short m_sClipW;	// For clipping border
	short	m_sClipH;
	short	m_sGridW;	// NOT TileW -> this is the NUMBER of nodes!
	short m_sGridH;
	short m_sTileW;	// For catching size errors
	short m_sTileH;

	CSmashatoriumList	*m_pGrid; // actually a 2d array

	//------------------- ACCESS VARIABLES:
	short	*m_psAccessX;	// m_sWorldW in size
	short *m_psAccessY;	// m_sWorldH in size
	CSmashatoriumList **m_ppslAccessY;	// m_sWorldH in size

	short	*m_psClipX;	// m_sWorldW + 2 Tiles in size
	short *m_psClipY;	// m_sWorldH + 2 Tiles in size
	CSmashatoriumList **m_ppslClipY;	// m_sWorldH + 2 Tiles in size

	//------------------- SEARCHING STATE INFORMATION: (QuickCheck info)
	// This must also handle large regions!
	// The design is largely for backwards compatibility.
	CSmash* m_pSmasher;					// NULL if search NOT in progress
	CSmashLink*	m_pCurrentSmashee;	// 

	CSmash::Bits m_include;
	CSmash::Bits m_dontcare;
	CSmash::Bits m_exclude;

	CSmashatoriumList *m_pCurrentList;
	short	m_sCurrentListX;
	short m_sCurrentListY;
	short m_sSearchW;	//  base 1 !
	short m_sSearchH; //	 base 1 !

	long	m_lCurrentSearchCode;	// change it for each search

	short m_sNumInSmash;	// Used for debugging
	short m_sMaxNumInSmash;	// Used for debugging

	//---------------------------------------------------------------------------
	// Update the specified CSmash.  If it isn't already in the smashatorium, it
	// is automatically added.  Whenever the CSmash is modified, this must be
	// called before using the smashotorium to check for collisions!

	// This version of the update is less efficient, as it always searches
	// four quadrants, and usually the desired object is only in one of the
	// four.  But I am trying it because it offers very fast updates.
	void Update(CSmash* pSmash);	// In:  CSmash to be updated
	//---------------------------------------------------------------------------
	void	Erase()	// does NOT deallocate anything!
		{
		m_sWorldW = m_sWorldH = m_sGridW = m_sGridH = 
			m_sClipW = m_sClipH = m_sTileW = m_sTileH = 0;

		m_psAccessX = m_psAccessY = m_psClipX = m_psClipY = NULL;
		m_pGrid = NULL;
		m_ppslAccessY = m_ppslClipY = NULL;
		m_lCurrentSearchCode = 1; // zero is NOT a valid search key!

		m_sNumInSmash = m_sMaxNumInSmash = 0;
		}

	void	Destroy()
		{
		if (m_pGrid) delete [] m_pGrid;
		if (m_psAccessX) free (m_psAccessX);
		if (m_psAccessY) free (m_psAccessY);
		if (m_ppslAccessY) free (m_ppslAccessY);

		Erase();
		}

	short Alloc(short sWorldW,short sWorldH,short sTileW,short sTileH);

	// Lower Level Inline!
	// These are done multiple times for a true remove
	void	RemoveLimb(CSmashatoriumList* pList,CSmashLink* pLink);

	// This is on a per object level:
	void	Remove(CSmash* pSmash);

	// This is on a per object level:
	// Used for fat objects
	void	RemoveFat(CFatSmash* pFatSmash);

	// Insert at tail...
	// Lower level inline
	void	AddLimb(CSmashatoriumList* pList, CSmashLink* pLink);

	// Higher Level -> add an entire CSmash into the 'torium
	// User calls Update, which checks for clipping
	// This routine ASSUMES not clipped out!
	void	Add(CSmash* pSmash,CSmashatoriumList *pList);

	// Higher Level -> add an entire CSmash into the 'torium
	// User calls Update, which checks for clipping
	// This routine ASSUMES not clipped out!
	// Used for fat objects
	void	AddFat(CFatSmash* pFatSmash);

	//===========================================================================
	// Currently stubs for now...
	//===========================================================================

	// Reset QuickCheckNext() using the specified parameters.
	// Begin a multicall collision search based on a smashee
	//
	void QuickCheckReset(									// Returns true if collision detected, false otherwise
		CSmash* pSmasher,										// In:  CSmash to check
		CSmash::Bits include,								// In:  Bits that must be 1 to collide with a given CSmash
		CSmash::Bits dontcare,								// In:  Bits that you don't care about
		CSmash::Bits exclude);								// In:  Bits that must be 0 to collide with a given CSmash

	// Reset QuickCheckNext() using the specified paramters.
	void QuickCheckReset(									// Returns true if collision detected, false otherwise
		CSmash::Bits include,								// In:  Bits, of which, one must be set to collide with a given CSmash
		CSmash::Bits dontcare,								// In:  Bits that you don't care about
		CSmash::Bits exclude);								// In:  Bits that must be 0 to collide with a given CSmash

	CSmash *GetNextSmash();	// Internal - used to aid in Next searches

	// Returns the next object being collided with, using the parameters that were
	// passed to QuickCheckReset().  This will return all the objects being collided
	// with in an arbitrary order.  Other functions will someday return the objects
	// in some particular order.  The function will return false when there are no
	// more colisions.
	bool QuickCheckNext(										// Returns true if collision detected, false otherwise
		R3DLine*	pline,										// In:  Line segment to collide against.
		CSmash** pSmashee = 0,								// Out: Thing being smashed into if any (unless 0)
		CSmash*	pSmasher = 0);								// Out: Smash that should be excluded from search.

	// Returns true if collision detected, false otherwise
	// Out: Thing being smashed into if any (unless 0)	
	bool QuickCheckNext(CSmash** pSmashee = 0);	

	// Determine whether specified CSmash is colliding with anything, and
	// if so, (optionally) return the first thing it's colliding with.  If
	// you want to know about all things being collided with or otherwise
	// want more control, use a CSmashatorium::Finder object.
	bool QuickCheck(											// Returns true if collision detected, false otherwise
		CSmash* pSmasher,										// In:  CSmash to check
		CSmash::Bits include,								// In:  Bits that must be 1 to collide with a given CSmash
		CSmash::Bits dontcare,								// In:  Bits that you don't care about
		CSmash::Bits exclude,								// In:  Bits that must be 0 to collide with a given CSmash
		CSmash** pSmashee = 0);								// Out: Thing being smashed into if any (unless 0)

	bool QuickCheckClosest(									// Returns true if collision detected, false otherwise
		CSmash* pSmasher,										// In:  CSmash to check
		CSmash::Bits include,								// In:  Bits that must be 1 to collide with a given CSmash
		CSmash::Bits dontcare,								// In:  Bits that you don't care about
		CSmash::Bits exclude,								// In:  Bits that must be 0 to collide with a given CSmash
		CSmash** pSmashee);

	// Determine whether specified R3DLine is colliding with anything, and
	// if so, (optionally) return the first thing it's colliding with.  If
	// you want to know about all things being collided with or otherwise
	// want more control, use a CSmashatorium::Finder object.
	bool QuickCheck(											// Returns true if collision detected, false otherwise
		R3DLine* pLine,										// In:  Line to check
		CSmash::Bits include,								// In:  Bits that must be 1 to collide with a given CSmash
		CSmash::Bits dontcare,								// In:  Bits that you don't care about
		CSmash::Bits exclude,								// In:  Bits that must be 0 to collide with a given CSmash
		CSmash** pSmashee = 0,								// Out: Thing being smashed into if any (unless 0)
		CSmash*	pSmasher = 0);								// Out: Smash that should be excluded from search.

	// Determine whether the two smashes have collided.  This is based purely on
	// the distance between them and their radius so it doesn't have to check
	// the grid at all.  
	bool QuickCheck(											// Returns true if collision detected, false otherwise
		CSmash* pSmasher,										// In:  One input smash
		CSmash* pSmashee)										// In:  Second input smash
	{
		return (ABS2(pSmasher->m_sphere.sphere.X - pSmashee->m_sphere.sphere.X,
		         pSmasher->m_sphere.sphere.Y - pSmashee->m_sphere.sphere.Y,
					pSmasher->m_sphere.sphere.Z - pSmashee->m_sphere.sphere.Z) <
					((pSmasher->m_sphere.sphere.lRadius + pSmashee->m_sphere.sphere.lRadius) *
					 (pSmasher->m_sphere.sphere.lRadius + pSmashee->m_sphere.sphere.lRadius)));
	}

	// Determine whether specified R3DLine is colliding with anything, and
	// if so, (optionally) return the closest (from start of segment) thing
	// it's colliding with.  If you want to know about all things being 
	// collided with or otherwise want more control, use a 
	// CSmashatorium::Finder object.
	bool QuickCheckClosest(									// Returns true if collision detected, false otherwise
		R3DLine* pline,										// In:  Line to check
		CSmash::Bits include,								// In:  Bits that must be 1 to collide with a given CSmash
		CSmash::Bits dontcare,								// In:  Bits that you don't care about
		CSmash::Bits exclude,								// In:  Bits that must be 0 to collide with a given CSmash
		CSmash** pSmashee,									// Out: Thing being smashed into if any.
		CSmash*	pSmasher = 0);								// Out: Smash that should be excluded from search.

	// Does a 2d XZ collision between two spheres.
	short	CollideCyl(CSmash* pSmashee,RSphere* pSphere);

	// Does a 2d XZ collision between a sphere and a line
	short	CollideCyl(CSmash* pSmashee,R3DLine* pLine);

	//---------------------------------------------------------------------------
	CSmashatorium() { Erase(); } // Needed for defaul construction

	void	Debug();		// Used for a plethora of things.

	// Reset does NOT DEALLOCATE any portion of the Smashatorium.
	// It is just a short cut to reset each of the grid's
	// SmashLists.  But Each Smash must reset it's own Links!
	//
	void Reset(void); 

	CSmashatorium(short sWorldW,short sWorldH,short sTileW,short sTileH) 
		{ Erase(); Alloc(sWorldW,sWorldH,sTileW,sTileH); }
	~CSmashatorium() { Destroy(); }
	};

//-------------------------------------------------
#endif


		
	/* This update saves search space, but is too slow on update
	void Update(CSmash* pSmash)	// In:  CSmash to be updated
		{
		ASSERT(pSmash);

		// ASSUME THAT THE OBJECT IS GRID SIZED!
		//---------------------------------------------------------------
		// (1) Cast the sphere into a 2 point square
		RSphere* pSphere = &(pSmash->m_sphere->sphere);
		long lR = pSphere->lRadius;

		long	x1,x2,y1,y2;
		x1 = x2 = pSphere->X;
		y1 = y2 = pSphere->Z;
		x1 -= lR;
		x2 += lR;
		y1 -= lR;
		y2 += lR;

		///////////////////////////////////
		// (2) Catch the case of clipping:
		if ( (x1 < 0) || (y1 < 0) || (x2 >= m_sWorldW) || (y2 >= m_sWorldH) )
			{
			// Handle partial clipping
			ASSERT(0);
			return; // NYI!
			}
		///////////////////////////////////
		// No clip case
		// (3) process each of the four corners:
		CSmashatoriumList **pCurrent = NULL;

		//-------------------------------------------------------------
		pCurrent = m_ppslAccessY[y1] + m_psAccessX[x1];
		if (m_link1.m_pLast && (pCurrent != m_link1.m_pLast) )
			{
			////////////// Pull the old out of the list!
			Remove(m_link1.m_pLast);
			}
		m_link1.m_pLast = pCurrent; // NOT YET INSTALLED THOUGH!
		//-------------------------------------------------------------
		//-------------------------------------------------------------
		pCurrent = m_ppslAccessY[y1] + m_psAccessX[x2];
		if (m_link2.m_pLast && (pCurrent != m_link2.m_pLast) )
			{
			////////////// Pull the old out of the list!
			Remove(m_link2.m_pLast);
			}
		m_link2.m_pLast = pCurrent; // NOT YET INSTALLED THOUGH!
		//-------------------------------------------------------------
		//-------------------------------------------------------------
		pCurrent = m_ppslAccessY[y2] + m_psAccessX[x1];
		if (m_link3.m_pLast && (pCurrent != m_link3.m_pLast) )
			{
			////////////// Pull the old out of the list!
			Remove(m_link3.m_pLast); 
			}
		m_link3.m_pLast = pCurrent; // NOT YET INSTALLED THOUGH!
		//-------------------------------------------------------------
		//-------------------------------------------------------------
		pCurrent = m_ppslAccessY[y2] + m_psAccessX[x2];
		if (m_link4.m_pLast && (pCurrent != m_link4.m_pLast) )
			{
			////////////// Pull the old out of the list!
			Remove(m_link4.m_pLast);
			}
		m_link4.m_pLast = pCurrent; // NOT YET INSTALLED THOUGH!
		//-------------------------------------------------------------

		//**************************************************
		// there is an error here! There could have been a 
		// quadrant which did not CHANGE (so was not removed), 
		// but is now REDUNDANT.  Think about combining these 
		// sections!
		//**************************************************

		// (4) Check for redundancies before installing:
		// USE a NULL pointer to signify redundancy
		if (m_link2.m_pLast == m_link3.m_pLast)
			{
			// all four in same region, 3 redundant:
			m_link2.m_pLast = m_link3.m_pLast = m_link4.m_pLast = NULL;
			}
		else if (m_link1.m_pLast == m_link3.m_pLast)
			{
			// vertically redundant:
			m_link3.m_pLast = m_link4.m_pLast = NULL;
			}
		else if (m_link1.m_pLast == m_link2.m_pLast)
			{
			// horizontally redundant:
			m_link2.m_pLast = m_link4.m_pLast = NULL;
			}

		// Link them all in if not redundant
		if (m_link1.m_pLast) Add(m_link1.m_pLast,&m_link1);
		if (m_link2.m_pLast) Add(m_link2.m_pLast,&m_link1);
		if (m_link3.m_pLast) Add(m_link3.m_pLast,&m_link1);
		if (m_link4.m_pLast) Add(m_link4.m_pLast,&m_link1);
		}
	*/

