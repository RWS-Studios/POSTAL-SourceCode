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
#include "RSPiX.h"
#include "smash.h" 
#include "realm.h"

//#define SMASH_DEBUG

#ifdef	SMASH_DEBUG

	#include "debugSmash.H"

#endif

////////////////////////////////////////////////////////////////////////////////
//
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
//		07/05/97	JRD	Fixed MANY bugs in the line collision algorithms.  
//							Handled special 
//							cases of near vertical lines and vertical clipping.
//							Still can't deal with fat smash objects.
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
//		08/09/97	JMI	CSmashatorium::Update() was checking the realm's 
//							m_flags.bEditPlay flag (which signifies that we're playing
//							a game from the editor) instead of the m_flags.bEditing
//							flag which indicates we're editting.  Fixed.
//
//		08/18/97 BRH	Fixed typo that used m_pSmasher instead of pSmasher
//							that was passed in.  Fixed CollideCyl that was checking
//							X and Y collisions to checking X and Z collisions.
//
//		08/31/97	JMI	Added some ASSERTs to foil SmartHeap's fancy smancy bounds
//							check padding areas and allow us to debug 'read' bounds
//							errors in debug mode.
//
//		09/02/97	JMI	Added some more ASSERTs for debug mode bounds checking.
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
////////////////////////////////////////////////////////////////////////////////
//
//  The current Smashatorium "HIERARCHY OF OBJECTS"  :
//
//	CSmashLink -> A 128-bit struct which is the manipulation block for the grid.
//               It currently points back to it's CSmash parent and it's old grid
//				     location.  For efficiency, we could redundantly store the
//               smash bits here.
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

CSmash::CSmash()
	{
	Erase();
	// Link up the links to their parent:
	m_link1.m_pParent = m_link2.m_pParent = m_link3.m_pParent = m_link4.m_pParent = this;
	}

CSmash::~CSmash()
	{
	// Make sure we've been removed.
	ASSERT(m_link1.m_pLast == NULL);
	ASSERT(m_sInGrid == FALSE);
	
	// As a back up, if not removed . . .
	if (m_link1.m_pLast != NULL)
		{
		if (m_pThing != NULL)
			{
			// Use the pthing to get to the realm and finally the smashatorium that
			// we are in.
			// Remove this smash.  This is for safety for uncaught release mode
			// bugs.
			m_pThing->m_pRealm->m_smashatorium.Remove(this);
			}
		}

	delete m_pFat; // Safe if NULL
	Erase();
	}

////////////////////////////////////////////////////////////////////////////////
//	CSmashatorium::CollideCyl - check for special collision with sphere case
////////////////////////////////////////////////////////////////////////////////
// This is a VERY special case scenario - currently, if the Smashee is a CDude,
// and IF his sphere collides, we THEN need to check if his "cylinder" collides.
// In this case, his cylinder is fixed at half his sphere radius.
////////////////////////////////////////////////////////////////////////////////
short	CSmashatorium::CollideCyl(CSmash* pSmashee,RSphere* pSphere) // sphere of Smasher
	{
	if (!pSmashee->m_pThing) return SUCCESS;	// not a dude
	if (pSmashee->m_pThing->GetClassID() != CThing::CDudeID) return SUCCESS;	// not a dude
	// it's a dude !  see if the cylinder collides:
	long	lCylR = pSmashee->m_sphere.sphere.lRadius / 3;	// go with half the sphere radius
	// NOTE that if it's a DUDE colliding with a DUDE, only one uses a cylinder:
	if (ABS2(pSmashee->m_sphere.sphere.X - pSphere->X,
		pSmashee->m_sphere.sphere.Z - pSphere->Z) >
		SQR(lCylR + pSphere->lRadius) ) return FAILURE;	// A MISS!

	return SUCCESS;	// a hit!
	}

////////////////////////////////////////////////////////////////////////////////
//	CSmashatorium::CollideCyl - check for special collision with line case
////////////////////////////////////////////////////////////////////////////////
// This is a VERY special case scenario - currently, if the Smashee is a CDude,
// and IF his sphere collides, we THEN need to check if his "cylinder" collides.
// In this case, his cylinder is fixed at half his sphere radius.
////////////////////////////////////////////////////////////////////////////////
short	CSmashatorium::CollideCyl(CSmash* pSmashee,R3DLine* pLine) // sphere of Smasher
	{
	if (!pSmashee->m_pThing) return SUCCESS;	// not a dude
	if (pSmashee->m_pThing->GetClassID() != CThing::CDudeID) return SUCCESS;	// not a dude
	// it's a dude !  see if the cylinder collides:
	long	lCylR = pSmashee->m_sphere.sphere.lRadius / 3;	// go with half the sphere radius
	long	lOldR = pSmashee->m_sphere.sphere.lRadius;
	pSmashee->m_sphere.sphere.lRadius = lCylR;	// shrink it:
	short sCollide = pSmashee->m_sphere.Collide(pLine);
	pSmashee->m_sphere.sphere.lRadius = lOldR;	// shrink it:

	if (sCollide == COLLISION) return SUCCESS;	// a hit!

	return FAILURE;	// a miss!
	}

////////////////////////////////////////////////////////////////////////////////
//  CSmashatorium::Alloc - create a grid of smash lists:
////////////////////////////////////////////////////////////////////////////////
short CSmashatorium::Alloc(short sWorldW,short sWorldH,short sTileW,short sTileH)
		{
		//-------------------------------------------------------------
		ASSERT(!m_psAccessX); // previous grid?
		ASSERT(!m_psAccessY);
		ASSERT(!m_ppslAccessY);
		ASSERT(!m_pGrid);
		
		ASSERT(sWorldW > 0); // bad input?
		ASSERT(sWorldH > 0);
		ASSERT(sTileW > 0);
		ASSERT(sTileH > 0);
		//-------------------------------------------------------------
		m_sWorldW = sWorldW;
		m_sWorldH = sWorldH;

		m_sClipW = m_sWorldW + (sTileW << 1);	// Add a one tile border
		m_sClipH = m_sWorldH + (sTileH << 1);

		m_sTileW = sTileW;	// For debugging & clipping
		m_sTileH = sTileH;	

		m_sGridW = (m_sClipW + sTileW - 1) / sTileW;
		m_sGridH = (m_sClipH + sTileH - 1) / sTileH;

		// For current logic convenience, do not allow partial tiles to exist:
		m_sClipW = long(m_sGridW) * sTileW;
		m_sClipH = long(m_sGridH) * sTileH;

		// Note that we must add 2 tile lengths to each access line:
		m_pGrid = new CSmashatoriumList[long(m_sGridW) * m_sGridH];
		m_psAccessX = (short*) calloc(sizeof(short),m_sClipW);
		m_psAccessY = (short*) calloc(sizeof(short),m_sClipH);
		m_ppslAccessY = (CSmashatoriumList**) calloc(sizeof (CSmashatoriumList*),m_sClipH);

		if (!m_psAccessX || !m_psAccessX || !m_ppslAccessY || !m_pGrid)
			{
			TRACE("CSmashatorium::Ran out of memory!\n");
			Destroy();
			Erase();
			return FAILURE;
			}

		// THE OFFICIAL RANGE HERE is from
		// -m_sTileW to (m_sWorldW + m_sTileW)
		// FULL CLIP is from 0 to (m_sWorldW - 1)
		//
		m_psClipX = m_psAccessX + m_sTileW;	// Offset values
		m_psClipY = m_psAccessY + m_sTileH;	// Offset values
		m_ppslClipY = m_ppslAccessY + m_sTileH;	// Offset values
	
		// Populate the access tables....
		short i,j,p,g;

		for (g=0, p = 0,i=0 ; i < m_sClipW; i += sTileW, g++)
			{
			for (j=0; j < sTileW; j++, p++)
				{
				m_psAccessX[p] = g;
				}
			}

		for (g=0, p = 0,i=0 ; i < m_sClipH; i += sTileH, g++)
			{
			for (j=0; j < sTileH; j++, p++)
				{
				m_psAccessY[p] = g;
				// Remember you are in pointer arithmetic mode!!!!
				m_ppslAccessY[p] = m_pGrid + long(m_sGridW) * g;
				}
			}

		m_sNumInSmash = m_sMaxNumInSmash = 0;

		return SUCCESS;
		}

////////////////////////////////////////////////////////////////////////////////
//
//	Reset - 
//
//	Reset does NOT DEALLOCATE any portion of the Smashatorium.
//	It is just a short cut to reset each of the grid's
//	SmashLists.  But Each Smash must reset it's own Links!
//
////////////////////////////////////////////////////////////////////////////////
void	CSmashatorium::Reset()
	{
	//----------------------------------------------------------------
	// Reset any search in progress: STEAL this code for a real func
	m_pCurrentSmashee = NULL;
	m_pSmasher = NULL;
	m_sCurrentListX = m_sCurrentListY = m_sSearchW = m_sSearchH = 0;
	//----------------------------------------------------------------
	// Go down the list of CSmashatoriumList's:
	long lCur;
	for (lCur = 0; lCur < long(m_sGridW) * m_sGridH; lCur++)
		{
		CSmashatoriumList	*pCur = m_pGrid + lCur;
		pCur->m_sNum = 0;
		pCur->m_slHead.Erase();
		pCur->m_slTail.Erase();
		}

	m_sNumInSmash = 0;
	}

////////////////////////////////////////////////////////////////////////////////
//
//	QuickCheckReset
//
//	Reset QuickCheckNext() using the specified parameters.
//	Begin a multicall collision search based on a smashee
// If the Smashee is in the Smashatorium, it must be small enough to fit.
//
////////////////////////////////////////////////////////////////////////////////
void CSmashatorium::QuickCheckReset(// Returns true if collision detected, false otherwise
	CSmash* pSmasher,						// In:  CSmash to check
	CSmash::Bits include,				// In:  Bits that must be 1 to collide with a given CSmash
	CSmash::Bits dontcare,				// In:  Bits that you don't care about
	CSmash::Bits exclude)				// In:  Bits that must be 0 to collide with a given CSmash
	{
	ASSERT(pSmasher);
	//---------------------------
	m_pSmasher = pSmasher;
	m_include = include;
	m_dontcare = dontcare;
	m_exclude = exclude;
	//--------------------------- preset size and position: ---------
	// (1) cast into a square:
	//---------------------------------------------------------------
	RSphere* pSphere = &(m_pSmasher->m_sphere.sphere);
	long lR = pSphere->lRadius;

	// Find upper left & lower right position:
	long	lX,lY,lX2,lY2;
	lX = pSphere->X - lR;
	lY = pSphere->Z - lR;

	m_lCurrentSearchCode++;			// prepare for a new searching code

	// Now do something different for a smashee that's in the 'torium
	// and one that's not...
	if (pSmasher->m_sInGrid) // we KNOW it's fully clipped and of legal size...
		{
		if ( (lX <= -m_sTileW) || (lY < -m_sTileH) || 
			(lX >= m_sWorldW) || (lY >= m_sWorldH) )
			{
			// We have FULL CLIP OUT!
			m_pSmasher = NULL; // this search has ended!
			
			return; 
			}

		// We know to do 2 x 2:
		m_pCurrentList = m_ppslClipY[lY] + m_psClipX[lX];
		m_sCurrentListX = m_sCurrentListY = 0;
		m_sSearchW = m_sSearchH = 2;
		m_pCurrentSmashee = NULL; // Pending first request

		if (m_lCurrentSearchCode < 0)	// unfortunate wrapping around...
			{
			ASSERT(0);	// Need to "detag" the smashatorium - detag could be a function
			}

		return; // Done!
		}

	// Handle the case of a monstrosity!
	lR += lR; // lR is a diameter now!
	lX2 = lX + lR;
	lY2 = lY + lR;

	//==========================================
	// Do tight clipping:
	//==========================================
	if (lX < 0) lX = 0;
	if (lY < 0) lY = 0;

	if (lX2 >= m_sWorldW) lX2 = m_sWorldW - 1;
	if (lY2 >= m_sWorldH) lY2 = m_sWorldH - 1;
	
	if ( (lX2 <= lX) || (lY2 <= lY) )
		{
		// Fully clipped out!
		m_pSmasher = NULL; // this search has ended!
		
		return; 
		}

	// Set up the search parameters:
	m_pCurrentSmashee = NULL; // Pending first request
	m_pCurrentList = m_ppslClipY[lY] + m_psClipX[lX];
	m_sCurrentListX = 0; // m_psClipX[lX];	// CURRENTLY, these are used merely as iterators
	m_sCurrentListY = 0; //m_psClipY[lY];

	m_sSearchW = 1 + m_psClipX[lX2] - m_psClipX[lX];
	m_sSearchH = 1 + m_psClipY[lY2] - m_psClipY[lY];
	}

////////////////////////////////////////////////////////////////////////////////
//
//
//


//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//
//
//


//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//
//
//


//
////////////////////////////////////////////////////////////////////////////////

//===========================================================================
// Currently stubs for now...
//===========================================================================

// Reset QuickCheckNext() using the specified paramters.
void CSmashatorium::QuickCheckReset(				// Returns true if collision detected, false otherwise
	CSmash::Bits include,								// In:  Bits, of which, one must be set to collide with a given CSmash
	CSmash::Bits dontcare,								// In:  Bits that you don't care about
	CSmash::Bits exclude)								// In:  Bits that must be 0 to collide with a given CSmash
	{
	TRACE("NEVER USED!\n");
	ASSERT(0);
	}

// Returns the next object being collided with, using the parameters that were
// passed to QuickCheckReset().  This will return all the objects being collided
// with in an arbitrary order.  Other functions will someday return the objects
// in some particular order.  The function will return false when there are no
// more colisions.
bool CSmashatorium::QuickCheckNext(					// Returns true if collision detected, false otherwise
	R3DLine*	pline,										// In:  Line segment to collide against.
	CSmash** pSmashee,								// Out: Thing being smashed into if any (unless 0)
	CSmash*	pSmasher)								// Out: Smash that should be excluded from search.
	{
	ASSERT(0);
	return false;	// NEVER USED ANYMORE!
	}

////////////////////////////////////////////////////////////////////////////////
//
//	QuickCheckNext - Smasher against smashee
//
// Returns true if collision detected, false otherwise
// Out: The Next Thing being smashed into if any (unless 0)	
// ***  ppSmashee is ONLY for output!
//
// NOTE: YOU Must set up this call using QuickCheckReset
// Returns NULL AND resets the QuickSearch if no more to find:
//
////////////////////////////////////////////////////////////////////////////////
CSmash *CSmashatorium::GetNextSmash()
	{
	short sNextList = FALSE;
	CSmash *pReturn = NULL;
	short sSearching = TRUE;

	while (sSearching)
		{
		if (!m_pCurrentSmashee)	// first in list:
			{
			m_pCurrentSmashee = &m_pCurrentList->m_slHead; // Start it off
			}

		if (m_pCurrentSmashee->m_pNext == &m_pCurrentList->m_slTail)
			{
			sNextList = TRUE;
			}
		else // We've got one!
			{
			m_pCurrentSmashee = m_pCurrentSmashee->m_pNext;
			pReturn = m_pCurrentSmashee->m_pParent;
			sSearching = FALSE;
			}
		
		if (sNextList)
			{
			m_pCurrentSmashee = NULL;
			sNextList = FALSE;

			// Find the next list
			m_sCurrentListX++;
			m_pCurrentList++;

			if (m_sCurrentListX >= m_sSearchW)
				{
				m_sCurrentListX = 0;
				m_sCurrentListY++;
				m_pCurrentList += m_sGridW - m_sSearchW;

				if (m_sCurrentListY >= m_sSearchH) // You're DONE
					{
					m_sCurrentListX = m_sCurrentListY = m_sSearchW = 
						m_sSearchH = 0;
					
					m_pCurrentList = NULL;
					pReturn = NULL;
					sSearching = FALSE;
					m_pSmasher = NULL;  // The real deactivation
					}
				}
			}
		}

	return pReturn;
	}

bool CSmashatorium::QuickCheckNext(CSmash** ppSmashee) 
	{ 
	// First, handle the easy case of a guaranteed 2x2 object:
	CSmash *pSmashee = NULL;

	// 1) Is a search in progress?
	if (!m_pSmasher) return false; // reset at end of search

	// 2) The QuickCheckReset parameters can tell the size:
	//		Look for a collision with our requirements
	pSmashee = GetNextSmash();
	while (pSmashee)	// compare this with what we want
		{
		if (pSmashee->m_lSearchTagCode != m_lCurrentSearchCode)
			{	// Avoid redundancy
			pSmashee->m_lSearchTagCode = m_lCurrentSearchCode;

			if (!(pSmashee->m_bits & m_exclude) && ((pSmashee->m_bits & ~m_dontcare) 
				& m_include) && pSmashee != m_pSmasher)
				{
				if (pSmashee->m_sphere.Collide(&m_pSmasher->m_sphere) == COLLISION)
					{
					if (CollideCyl(pSmashee,&m_pSmasher->m_sphere.sphere) == SUCCESS)
						{
						*ppSmashee = m_pCurrentSmashee->m_pParent;
						return true;
						}
					}
				}
			}

		pSmashee = GetNextSmash();
		}

	return false;  // USED BY FIRE
	}						

////////////////////////////////////////////////////////////////////////////////
//
//	QuickCheck - Smasher against smashee
//
// Returns true if collision detected, false otherwise
// Sets *ppSmashee to the thing collided with.
//
// This function is like QuickCheckNext, except it just returns the 
// FIRST thing it finds that is a hit.  (Arbitrary)
// 
// Returns NULL if nothing is colliding
//
////////////////////////////////////////////////////////////////////////////////
bool CSmashatorium::QuickCheck(						// Returns true if collision detected, false otherwise
	CSmash* pSmasher,										// In:  CSmash to check
	CSmash::Bits include,								// In:  Bits that must be 1 to collide with a given CSmash
	CSmash::Bits dontcare,								// In:  Bits that you don't care about
	CSmash::Bits exclude,								// In:  Bits that must be 0 to collide with a given CSmash
	CSmash** ppSmashee)									// Out: Thing being smashed into if any (unless 0)
	{
	// This routine combines the logic of QuickCheckNext and QuickCheckReset into one!
	ASSERT(pSmasher);
	ASSERT(ppSmashee);

	m_lCurrentSearchCode++;			// prepare for a new searching code
	if (m_lCurrentSearchCode < 0)	// unfortunate wrapping around...
		{
		ASSERT(0);	// Need to "detag" the smashatorium - detag could be a function
		}

	// Determine the grid expanse of the Smasher's radius:
	// (1) cast into a square:
	//---------------------------------------------------------------
	RSphere* pSphere = &(pSmasher->m_sphere.sphere);
	long lR = pSphere->lRadius;

	// Find upper left & lower right position:
	long	lX,lY,lX2,lY2;
	lX = pSphere->X - lR;
	lY = pSphere->Z - lR;

	short sW=0,sH=0,i,j;
	CSmashatoriumList* pCurrentList = NULL;

	// Now do something different for a smashee that's in the 'torium
	// and one that's not...
	if (pSmasher->m_sInGrid) // we KNOW it's fully clipped and of legal size...
		{
		if ( (lX <= -m_sTileW) || (lY < -m_sTileH) || 
			(lX >= m_sWorldW) || (lY >= m_sWorldH) )
			{
			// We have FULL CLIP OUT!
			*ppSmashee = NULL;
			return false;	// this search has ended!
			}

		// We know to do 2 x 2:
		pCurrentList = m_ppslClipY[lY] + m_psClipX[lX];
		sW = sH = 2;
		}
	else
		{
		// Handle the case of a monstrosity!
		lR += lR; // lR is a diameter now!
		lX2 = lX + lR;
		lY2 = lY + lR;

		//==========================================
		// Do tight clipping:
		//==========================================
		if (lX < 0) lX = 0;
		if (lY < 0) lY = 0;

		if (lX2 >= m_sWorldW) lX2 = m_sWorldW - 1;
		if (lY2 >= m_sWorldH) lY2 = m_sWorldH - 1;
		
		if ( (lX2 <= lX) || (lY2 <= lY) )
			{
			// Fully clipped out!
			*ppSmashee = NULL;
			return false;	// this search has ended!
			}

		// Set up the search parameters:
		pCurrentList = m_ppslClipY[lY] + m_psClipX[lX];

		sW = 1 + m_psClipX[lX2] - m_psClipX[lX];
		sH = 1 + m_psClipY[lY2] - m_psClipY[lY];
		}

	// Do the search
	for (j=0; j < sH; j++, pCurrentList += m_sGridW - sW)
		{
		for (i=0; i < sW; i++,pCurrentList++)
			{
			if (pCurrentList->m_sNum)
				{
				CSmashLink* pLink = pCurrentList->m_slHead.m_pNext;
				CSmash* pSmashee;
				while (pLink != &pCurrentList->m_slTail)
					{
					pSmashee = pLink->m_pParent;

					// Test for the collision!
					if (pSmashee->m_lSearchTagCode != m_lCurrentSearchCode)
						{	// Avoid redundancy
						pSmashee->m_lSearchTagCode = m_lCurrentSearchCode;

						if (!(pSmashee->m_bits & exclude) && ((pSmashee->m_bits & ~dontcare) 
							& include) && pSmashee != pSmasher)
							{
							if (pSmashee->m_sphere.Collide(&pSmasher->m_sphere) == COLLISION)
								{
								if (CollideCyl(pSmashee,&pSmasher->m_sphere.sphere) == SUCCESS)
									{
									*ppSmashee = pSmashee;
									return true;
									}
								}
							}
						}

					pLink = pLink->m_pNext;
					}
				}
			}
		}

	return false; // Used by missile
	}

////////////////////////////////////////////////////////////////////////////////
//
//	QuickCheckClosest - Smasher against smashee
//
// Returns true if collision detected, false otherwise
// Sets *ppSmashee to the thing collided with.
//
// This function is like QuickCheckNext, except it just returns the 
// CLOSEST thing it finds that is a hit.  (Front or back)
// 
// Returns NULL if nothing is colliding
//
////////////////////////////////////////////////////////////////////////////////
bool CSmashatorium::QuickCheckClosest(				// Returns true if collision detected, false otherwise
	CSmash* pSmasher,										// In:  CSmash to check
	CSmash::Bits include,								// In:  Bits that must be 1 to collide with a given CSmash
	CSmash::Bits dontcare,								// In:  Bits that you don't care about
	CSmash::Bits exclude,								// In:  Bits that must be 0 to collide with a given CSmash
	CSmash** ppSmashee)
	{
	// This routine combines the logic of QuickCheckNext and QuickCheckReset into one!
	ASSERT(pSmasher);
	ASSERT(ppSmashee);

	m_lCurrentSearchCode++;			// prepare for a new searching code
	if (m_lCurrentSearchCode < 0)	// unfortunate wrapping around...
		{
		ASSERT(0);	// Need to "detag" the smashatorium - detag could be a function
		}


	// Determine the grid expanse of the Smasher's radius:
	// (1) cast into a square:
	//---------------------------------------------------------------
	RSphere* pSphere = &(pSmasher->m_sphere.sphere);
	long lR = pSphere->lRadius;

	long lClosestDist2 = 2000000000; // a large number
	long lCurDist2;

	long lSmasherX = pSphere->X;
	long lSmasherY = pSphere->Z;

	CSmash* pClosestSmash = NULL;

	// Find upper left & lower right position:
	long	lX,lY,lX2,lY2;
	lX = lSmasherX - lR;
	lY = lSmasherY - lR;

	short sW=0,sH=0,i,j;
	CSmashatoriumList* pCurrentList = NULL;

	// Now do something different for a smashee that's in the 'torium
	// and one that's not...
	if (pSmasher->m_sInGrid) // we KNOW it's fully clipped and of legal size...
		{
		if ( (lX <= -m_sTileW) || (lY < -m_sTileH) || 
			(lX >= m_sWorldW) || (lY >= m_sWorldH) )
			{
			// We have FULL CLIP OUT!
			*ppSmashee = NULL;
			return false;	// this search has ended!
			}

		// We know to do 2 x 2:
		pCurrentList = m_ppslClipY[lY] + m_psClipX[lX];
		sW = sH = 2;
		}
	else
		{
		// Handle the case of a monstrosity!
		lR += lR; // lR is a diameter now!
		lX2 = lX + lR;
		lY2 = lY + lR;

		//==========================================
		// Do tight clipping:
		//==========================================
		if (lX < 0) lX = 0;
		if (lY < 0) lY = 0;

		if (lX2 >= m_sWorldW) lX2 = m_sWorldW - 1;
		if (lY2 >= m_sWorldH) lY2 = m_sWorldH - 1;
		
		if ( (lX2 <= lX) || (lY2 <= lY) )
			{
			// Fully clipped out!
			*ppSmashee = NULL;
			return false;	// this search has ended!
			}

		// Set up the search parameters:
		pCurrentList = m_ppslClipY[lY] + m_psClipX[lX];

		sW = 1 + m_psClipX[lX2] - m_psClipX[lX];
		sH = 1 + m_psClipY[lY2] - m_psClipY[lY];
		}

	// Do the search
	for (j=0; j < sH; j++, pCurrentList += m_sGridW - sW)
		{
		for (i=0; i < sW; i++,pCurrentList++)
			{
			if (pCurrentList->m_sNum)
				{
				CSmashLink* pLink = pCurrentList->m_slHead.m_pNext;
				CSmash* pSmashee;
				while (pLink != &pCurrentList->m_slTail)
					{
					pSmashee = pLink->m_pParent;

					// Test for the collision!
					if (pSmashee->m_lSearchTagCode != m_lCurrentSearchCode)
						{	// Avoid redundancy
						pSmashee->m_lSearchTagCode = m_lCurrentSearchCode;

						// Test for the colision!
						if (!(pSmashee->m_bits & exclude) && ((pSmashee->m_bits & ~dontcare) 
							& include) && pSmashee != pSmasher)
							{
							if (pSmashee->m_sphere.Collide(&pSmasher->m_sphere) == COLLISION)
								{
								if (CollideCyl(pSmashee,&pSmasher->m_sphere.sphere) == SUCCESS)
									{
									// Is this hit the closest?
									lCurDist2 = SQR(lSmasherX - pSmashee->m_sphere.sphere.X) + 
										SQR(lSmasherY - pSmashee->m_sphere.sphere.Z);
									if (lCurDist2 < lClosestDist2)
										{
										pClosestSmash = pSmashee;
										lClosestDist2 = lCurDist2;
										}
									}
								}
							}
						}

					pLink = pLink->m_pNext;
					}
				}
			}
		}
	
	*ppSmashee = pClosestSmash;
	if (pClosestSmash) return true;

	return false; // Used by lock-on missile
	}

////////////////////////////////////////////////////////////////////////////////
//
//	QuickCheck - collide a line with the smash
//	
// Determine whether specified R3DLine is colliding with anything, and
// if so, (optionally) return the first thing it's colliding with.
//
////////////////////////////////////////////////////////////////////////////////
bool CSmashatorium::QuickCheck(// Returns true if collision detected, false otherwise
	R3DLine* pLine,				// In:  Line to check
	CSmash::Bits include,		// In:  Bits that must be 1 to collide with a given CSmash
	CSmash::Bits dontcare,		// In:  Bits that you don't care about
	CSmash::Bits exclude,		// In:  Bits that must be 0 to collide with a given CSmash
	CSmash** ppSmashee,			// Out: Thing being smashed into if any (unless 0)
	CSmash*	pSmasher)			// Out: Smash that should be excluded from search.
	{
	ASSERT(0);

	return false;		// NEVER USED ANYMORE!
	}

// I just use this for anything I need to debug at the moment.
// Currently, it dumps out the grid amount numbers.
void	CSmashatorium::Debug()
	{
	// PURE DEBUGGING HELL!
		FILE* fp = fopen("smashout.txt","w");

		fprintf(fp,"GridW = %hd\nGridH = %hd\n",m_sGridW,m_sGridH);
		fprintf(fp,"# in smash = %hd;  # in each list:\n\n",m_sNumInSmash);

		short di,dj;

		// Try direct grid access:
		short p = 0;
		for (dj = 0; dj < m_sGridH; dj++)
			{
			for (di = 0; di < m_sGridW; di++)
				{
				fprintf(fp,"%2hd ",m_pGrid[p++].m_sNum);
				}
			fprintf(fp,"\n");
			}

		fprintf(fp,"\n\n***********\n");
		// Try Indirect Access
		for (dj = 0; dj < m_sWorldH; dj+=m_sTileH)
			{
			for (di = 0; di < m_sWorldW;di+=m_sTileW)
				{
				fprintf(fp,"%2hd ",(m_ppslClipY[dj] + m_psClipX[di])->m_sNum);
				}
			fprintf(fp,"\n");
			}
		//
		fclose(fp);
	}

////////////////////////////////////////////////////////////////////////////////
//
//	QuickCheckClosest -	collide a line with the smash, excluding myself
//								Find the closest hit to the FIRST point in the line!
//	
// Determine whether specified R3DLine is colliding with anything, and
// if so, (optionally) return the closet thing (to the CSmash) it's colliding with.
// Additionally, specify a CSmash to EXCLUDE from the search.
//
// Note that it is actually a line segment
//
////////////////////////////////////////////////////////////////////////////////
bool CSmashatorium::QuickCheckClosest(	// Returns true if collision detected, false otherwise
	R3DLine* pline,							// In:  Line to check
	CSmash::Bits include,					// In:  Bits that must be 1 to collide with a given CSmash
	CSmash::Bits dontcare,					// In:  Bits that you don't care about
	CSmash::Bits exclude,					// In:  Bits that must be 0 to collide with a given CSmash
	CSmash** ppSmashee,						// Out: Thing being smashed into if any.
	CSmash*	pSmasher)						// Out: Smash that should be excluded from search.
	{
	// This is a tricky line, because far from the standard 8-connect line, this must include
	// ALL regions the line even glances through!  And cliping is a nightmare!

	// This routine combines the logic of QuickCheckNext and QuickCheckReset into one!
	// pSmasher can be NULL!
	ASSERT(ppSmashee);

	// Current Implementation:
	// 1) NO CLIPPING YET!
	// 2) NO vertical line case:

	// Set up the line, clipping where needed: (We only look at 2d)
	// We will draw the line left to right:

	//************************************************************************************
	// Sort points left to right:

	long lLeft = pline->X1;
	long lRight = pline->X2;
	long lLeftY = pline->Z1;
	long lRightY = pline->Z2;

	// Inverted case
	if (lRight < lLeft)
		{
		lLeft = pline->X2;
		lRight = pline->X1;
		lLeftY = pline->Z2;		
		lRightY = pline->Z1;
		}
	//************************************************************************************
	// Calculate y major line coefficients: (later, adapt to clipping values)
	long	lDelX = lRight - lLeft;
	long	lDelY = lRightY - lLeftY;
	long	lDet = lDelX * lRightY - lDelY * lRight;
	long	lDetY = lDelY * lRight - lDelX * lRightY; // for x-major line form

	//************************************************************************************
	// Handle Clipping, x-major line caclulation, and special cases
				//***************  NYI!  **************

	// Check for horizontal clipping (easy)
	long	lClipLeft = lLeft;// = MAX(0,lLeft);
	long	lClipRight = lRight;// = MIN(m_sWorldW - 1,lRight);
	long	lClipLeftY = lLeftY;
	long	lClipRightY = lRightY;

	short sVerticalStrip = false;
	long	lGridLeft;
	long	lGridRight;

	if (lDelX)	// determine x-clipping values:
		{

		if (lLeft < 0) 
			{
			lClipLeft = 0;
			lClipLeftY = lDet / lDelX; //********** CAREFUL
			}

		if (lRight >= m_sWorldW)
			{
			lClipRight = m_sWorldW - 1;
			lClipRightY = (lClipRight * lDelY + lDet) / lDelX;	//******** CAREFUL!
			}
		}
	else	// certical strip case:
		{
		if ( (lLeft < 0) || (lRight >= m_sWorldW) ) return false;
		}

 	// Check for case of reverse clip out:
	if ( (lLeft >= m_sWorldW) || (lRight < 0) ) return false;	// clipped out!

	lGridLeft = (long)m_psClipX[lClipLeft];	// These represent pts BETWEEN grid squares
	lGridRight = 1 + (long)m_psClipX[lClipRight];

	if ((lDelX == 0) || ( (lGridRight - lGridLeft) <= 1) ) 
		{
		sVerticalStrip = true;	// initial x clipping
		}
//		}
//	else 

	if (sVerticalStrip)	// do special clipping:
		{
		// Handle the vertical strip case:
		if (lClipRight < lClipLeft) return false;	// vertical strip off screen

		// Clip Vertically
		lGridRight = lGridLeft + 1;	// for compatibility
		if (lClipRightY < 0) lClipRightY = 0;
		if (lClipRightY >= m_sWorldH) lClipRightY = m_sWorldH;
		if (lClipLeftY < 0) lClipLeftY = 0;
		if (lClipLeftY >= m_sWorldH) lClipLeftY = m_sWorldH;
		}

	//************************************************************************************
	// Check for Vertical Clipping:  (even for strip case)
	// Must recalculate grid positions of X changes:

	// NEED true CLIP OUT SCENARIOS FOR VERTICAL!!!!

	short sClippingY = false;

	if (lDelY != 0) // Horizontal strip case
		{													// Note that the vertical case has
															// twice the possibilities
		if (lDelY > 0)	// lClipLeftY < lClipRightY:
			{
			if (lClipLeftY < 0) 
				{
				lClipLeftY = 0;
				lClipLeft = lDetY / lDelY; //********** CAREFUL

				if ( (lClipLeft < 0) || (lClipLeft >= m_sWorldW) ) return false;

				lGridLeft = (long)m_psClipX[lClipLeft];
				sClippingY = true;
				}

			if (lClipRightY >= m_sWorldH)
				{
				lClipRightY = m_sWorldH - 1;
				lClipRight = (lClipRightY * lDelX + lDetY) / lDelY;	//******** CAREFUL!

				if ( (lClipRight < 0) || (lClipRight >= m_sWorldW) ) return false;

				lGridRight = 1 + (long)m_psClipX[lClipRight];
				sClippingY = true;
				}

			// Check of clipout:
			if (lClipRightY < lClipLeftY) return false; // clipped out
			}
		else	// lClipLeftY > lClipRightY
			{
			if (lClipRightY < 0) 
				{
				// Since lDelY must be negative for us to get here,
				// lDetY must be negative for lClipRight to be
				// a positive index.
				//ASSERT(lDetY < 0);

				lClipRightY = 0;
				lClipRight = lDetY / lDelY; //********** CAREFUL

				// Assert on the actual value, duh!  I don't know what
				// I was thinking with that fancy smancy ASSERT above.
				//ASSERT(lClipRight >= 0);

				if ( (lClipRight < 0) || (lClipRight >= m_sWorldW) ) return false;

				lGridRight = 1 + (long)m_psClipX[lClipRight];
				sClippingY = true;
				}

			if (lClipLeftY >= m_sWorldH)
				{
				lClipLeftY = m_sWorldH - 1;
				lClipLeft = (lClipLeftY * lDelX + lDetY) / lDelY;	//******** CAREFUL!

				// Since lDelY must be negative for us to get here,
				// lDetY must be positive enough to get us out of negatives
				// OR lClipLeftY must be negative 
				// OR lDelX must be negative
				// but not both, but perhaps all three could be true.
				// The easiest way to watch out for this value is by the result.

				//ASSERT(lClipLeft >= 0);

				if ( (lClipLeft < 0) || (lClipLeft >= m_sWorldW) ) return false;

				lGridLeft = (long)m_psClipX[lClipLeft];
				sClippingY = true;
				}

			// Check out clipout
			if (lClipRightY > lClipLeftY) return false; // clipped out
			}

		if (sClippingY) // recalculate clipping situation:
			{
			if ((lDelX == 0) || ( (lGridRight - lGridLeft) == 1) ) 
				sVerticalStrip = true;
			}
		}
	else	// horizontal strip case:
		{
		if ( (lClipLeftY < 0) || (lClipLeftY >= m_sWorldH) ) return false;
		}

	//************************************************************************************
	// Calculate grid points for the line: (later, switch to clipped values)

	// allocate on the stack a local point chart:
#define MAX_GRID_W 1024	//************************************ NEED TO DEAL WITH THIS!

	long	alPointsY[MAX_GRID_W + 1];
	short i,x;

	// Get end points:
	alPointsY[lGridLeft] = (long)m_psClipY[lClipLeftY];
	alPointsY[lGridRight] = (long)m_psClipY[lClipRightY];

	//************************************************************************************
	// Handle vertical strip case, if applicable:

	if (sVerticalStrip == false)	// Load theintermediate values:
		{	// not vertical strip:

		// Populate the point array:
		x = lGridLeft * m_sTileW; // now using clip instead of access, which is one over

		for (i = lGridLeft + 1; i < lGridRight; i++, x += m_sTileW)
			{
			alPointsY[i] = (x * lDelY + lDet) / lDelX; // WARNING - watch for lDelX
			alPointsY[i] = (long)m_psClipY[alPointsY[i]]; // convert to grid coordinates
			}
		}

	//************************************************************************************
	//  Move acros all the grid points crossed by the line, and process each Smash List!
	short j;

	// SPLIT between positive and negative cases:
	short sSignY = 1;
	if (lDelY < 0) sSignY = -1;

	// SET UP DISTANCE VARIABLES:
	long lClosestDist2 = 2000000000; // a large number
	long lCurDist2;
	CSmash* pClosestSmash = NULL;

	m_lCurrentSearchCode++;			// prepare for a new searching code
	if (m_lCurrentSearchCode < 0)	// unfortunate wrapping around...
		{
		ASSERT(0);	// Need to "detag" the smashatorium - detag could be a function
		}

	for (i = lGridLeft; i < lGridRight; i++)
		{
		// Now, a little tricky - do a bidirectional loop to cover both quadrants:
		for (j = alPointsY[i]; j != (alPointsY[i + 1] + sSignY); j += sSignY)
			{
#ifdef	SMASH_DEBUG
	
	DebugSmash.DrawSmashSquare(3,0,i-1,j-1);

#endif
			// Get the list:
			ASSERT(j * m_sTileH < m_sWorldH + 2 * m_sTileH);
			ASSERT(i * m_sTileW < m_sWorldW + 2 * m_sTileW);
			ASSERT(j * m_sTileH >= 0);
			ASSERT(i * m_sTileW >= 0);
			CSmashatoriumList* pCurrentList = m_ppslAccessY[j * m_sTileH] + m_psAccessX[i * m_sTileW];

			//***************************************************************************
			// Now, process this smash grid in a standard loop like any other.
			if (pCurrentList->m_sNum)
				{
				CSmashLink* pLink = pCurrentList->m_slHead.m_pNext;
				CSmash* pSmashee;
				while (pLink != &pCurrentList->m_slTail)
					{
					pSmashee = pLink->m_pParent;

					// Test for the collision!
					if (pSmashee->m_lSearchTagCode != m_lCurrentSearchCode)
						{	// Avoid redundancy
						pSmashee->m_lSearchTagCode = m_lCurrentSearchCode;

						// Test for the colision!
						if (!(pSmashee->m_bits & exclude) && ((pSmashee->m_bits & ~dontcare) 
							& include) && pSmashee != pSmasher)
							{
							if (pSmashee->m_sphere.Collide(pline) == COLLISION)
								{
								if (CollideCyl(pSmashee,pline) == SUCCESS)
									{
									// Is this hit the closest?
									// Calculate distance from FIRST point in the line

									lCurDist2 = ABS2(
										pSmashee->m_sphere.sphere.X - pline->X1,
										pSmashee->m_sphere.sphere.Y - pline->Y1,
										pSmashee->m_sphere.sphere.Z - pline->Z1);

									// If there's not currently a closest or this one is closer . . .
									if (lCurDist2 < lClosestDist2)
										{
										// Make this the closest.
										pClosestSmash	= pSmashee;
										lClosestDist2	= lCurDist2;
										}
									}
								}
							}
						}

					pLink = pLink->m_pNext;
					}
				}
			}
		}

	// Set the result:
	*ppSmashee = pClosestSmash;
	if (pClosestSmash) return true;

	return false; // #1 most used function! (All guns)
	}

//==============================================================================

////////////////////////////////////////////////////////////////////////////////
//  The following functions SHOULD be moved back into the class so that
//  they can be inlined.  Do this as soon as the code is fully stable!
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//
//	AddLimb
// 
// Lower level function for adding each leg of the Smash into it's quadrant
//
////////////////////////////////////////////////////////////////////////////////
void	CSmashatorium::AddLimb(CSmashatoriumList* pList, CSmashLink* pLink)
	{
	ASSERT(pLink);
	ASSERT(pList);
	//--------------------------------------
	pList->m_sNum++;

	pLink->m_pLast = pList;

	CSmashLink* pTail = &pList->m_slTail;
	CSmashLink* pPrev = pTail->m_pPrev;

	pLink->m_pNext = pTail;
	pLink->m_pPrev = pPrev;
	pPrev->m_pNext = pLink;
	pTail->m_pPrev = pLink;
	}

////////////////////////////////////////////////////////////////////////////////
//
//	RemoveLimb
//	
// Lower level function for removing each leg of the Smash from it's quadrant
//
////////////////////////////////////////////////////////////////////////////////
void	CSmashatorium::RemoveLimb(CSmashatoriumList* pList,CSmashLink* pLink)
	{
	ASSERT(pLink);
	ASSERT(pList);
	ASSERT(pList->m_sNum);
	//--------------------------------------
	pList->m_sNum--;

	pLink->m_pLast = NULL;

	CSmashLink* pPrev = pLink->m_pPrev;
	CSmashLink* pNext = pLink->m_pNext;

	pPrev->m_pNext = pNext;
	pNext->m_pPrev = pPrev;
	pLink->m_pPrev = pLink->m_pNext = NULL;
	}

////////////////////////////////////////////////////////////////////////////////
//
//		Update
//	
// Update the specified CSmash.  If it isn't already in the smashatorium, it
// is automatically added.  Whenever the CSmash is modified, this must be
// called before using the smashotorium to check for collisions!
// This version of the update is less efficient, as it always searches
// four quadrants, and usually the desired object is only in one of the
// four.  But I am trying it because it offers very fast updates.
//
////////////////////////////////////////////////////////////////////////////////
short sMaxRad = 0;

void CSmashatorium::Update(CSmash* pSmash)	// In:  CSmash to be updated
	{
	ASSERT(pSmash);

	// Check the flag in the realm to see if we're editing.
	// If so, DO NOT actually let things go in the smashatorium:
	if (pSmash->m_pThing->m_pRealm->m_flags.bEditing
		&& (!pSmash->m_pThing->m_pRealm->m_flags.bEditPlay))
		{
		return;	// pretend you're in the smash
		}

	// ASSUME THAT THE OBJECT IS GRID SIZED!
	//---------------------------------------------------------------
	// (1) Cast the sphere into a 2 point square
	RSphere* pSphere = &(pSmash->m_sphere.sphere);
	long lR = pSphere->lRadius;
	long lD = lR << 1;

	// Find upper left position:
	long	lX,lY;
	lX = pSphere->X - lR;
	lY = pSphere->Z - lR;
	
	// Hook in the special case of a FAT body in the smash!
	if ( (lD > m_sTileW) || (lD > m_sTileH) )
		{
		//================================================================== FAT SMASH
		// Create and insert a fat smash into the smashatorium.
		CFatSmash*	pFat = pSmash->m_pFat;

		if (!pSmash->m_pFat)	// we need to create a fat for you boy!
			{
			ASSERT(!pSmash->m_sInGrid); // a growing small object has popped it's boundaries

			//============================================ CREATE FAT EXTENSION
			// Create a new fat extention to the smash:
			pFat = new CFatSmash;
			pSmash->m_pFat = pFat;
			pFat->m_pParent = pSmash;	// ahhhh, a family

			if (!pFat)
				{
				TRACE("CSmashatorium::Update: memory alloc error! Couldn't add to smashatorium!\n");
				return;
				}
			
			// Remember where it was
			pFat->m_lX = lX;
			pFat->m_lY = lY;

			// Find dimensions of smash and allocate:
			pFat->m_sW = m_psAccessX[lD + m_sTileW - 1] + 1;	// min tiles + 1
			pFat->m_sH = m_psAccessY[lD + m_sTileH - 1] + 1;	// min tiles + 1

			pFat->m_sNumGrids = pFat->m_sW * pFat->m_sH;
			if (pFat->Alloc(pFat->m_sNumGrids) != SUCCESS)
				{
				TRACE("CSmashatorium::Update: memory alloc error! Couldn't add to smashatorium!\n");
				return;
				}	// (all links are now NULL!

			// Set all the SmashLinks to point to their parent:
			for (short i=0; i < pFat->m_sNumGrids; i++)
				{
				pFat->m_pLinks[i].m_pParent = pSmash;
				}

			// at this point, assume the smash is fat
			}

		///////////////////////////////////
		// (2) Catch the case of FULL clipping:
		if ( (lX <= -lD) || (lY <= -lD) || 
			(lX >= m_sWorldW) || (lY >= m_sWorldH) )
			{
			// We have FULL CLIP OUT!
			if (pSmash->m_sInGrid)	RemoveFat(pFat); // set's InGrid to false
			else	pSmash->m_sInGrid = FALSE;
			
			return; 
			}

		///////////////////////////////////
		// (4) is it's position different?
		// (If it wasn't in the grid, than this is irrelevant:)
		//
		// Has it changed or is it reentering the grid?
		if ( (lX != pFat->m_lX) || (lY != pFat->m_lY) || (!pSmash->m_sInGrid)) 
			{
			if (pSmash->m_sInGrid) 
				{
				RemoveFat(pFat);  // Remove from old
				}

			///////////////////////////////////
			// ADD FAT AND SET FAT POSITION!
			///////////////////////////////////
			// Remember where it was
			pFat->m_lX = lX;
			pFat->m_lY = lY;

			// (3) calculate current grid clipping state:
			// Because a fat smash should NOT be moving, we shouldn't be doing this 
			// more than once!

			short sClipX = MAX(0L,lX);
			short sClipY = MAX(0L,lY);
			short sClipX2 = MIN(m_sWorldW-1L,lX + lD);
			short sClipY2 = MIN(m_sWorldH-1L,lY + lD);

			pFat->m_pClippedGrid = m_ppslClipY[sClipY] + m_psClipX[sClipX];

			// We can't access grid locations if lX and lY are negative!
			short sGridX,sGridY;
			if (lX < 0) sGridX = -m_psAccessX[-lX] - 1; // mirror it!
			else	sGridX = m_psAccessX[lX];

			if (lY < 0) sGridY = -m_psAccessY[-lY] - 1; // mirror it!
			else	sGridY = m_psAccessX[lY];

			// Convert to grid coordinates:
			sClipX = m_psAccessX[sClipX];
			sClipY = m_psAccessY[sClipY];
			sClipX2 = m_psAccessX[sClipX2];
			sClipY2 = m_psAccessY[sClipY2];

			// Map to local fat smash:
			// These are relative grid positions:

			pFat->m_sClipX = sClipX - sGridX;
			pFat->m_sClipY = sClipY - sGridY;
			pFat->m_sClipW = sClipX2 - sClipX + 1;
			pFat->m_sClipH = sClipY2 - sClipY + 1;

			// Where do we start in smashatorium?
			pFat->m_pFirstLink = pFat->m_pLinks + pFat->m_sW * pFat->m_sClipY + pFat->m_sClipX;

			AddFat(pFat);  // Will set flag to in grid
			}

		return;
		//================================================================== FAT SMASH
		}

	///////////////////////////////////
	// (2) Catch the case of FULL clipping:
	if ( (lX <= -m_sTileW) || (lY < -m_sTileH) || 
		(lX >= m_sWorldW) || (lY >= m_sWorldH) )
		{
		// We have FULL CLIP OUT!
		if (pSmash->m_sInGrid)	Remove(pSmash); // set's InGrid to false
		else	pSmash->m_sInGrid = FALSE;
		
		return; 
		}

	///////////////////////////////////
	// (3) calculate grid bas position:
	CSmashatoriumList *pCurrent = m_ppslClipY[lY] + m_psClipX[lX];

	///////////////////////////////////
	// (4) is it's position different?
	// (If it wasn't in the grid, than this is irrelevant:)
	//
	if (!pSmash->m_sInGrid)	// Re-entering Grid
		{
		Add(pSmash,pCurrent);  // Will set flag to in grid
		}
	else if (pSmash->m_link1.m_pLast != pCurrent) 
		{
		Remove(pSmash);  // Remove from old

		Add(pSmash,pCurrent);  // Will set flag to in grid
		}
	}

////////////////////////////////////////////////////////////////////////////////
//
//		ADD
//
// Higher Level -> add an entire CSmash into the 'torium
// User calls Update, which checks for clipping
// This routine ASSUMES not clipped out!
//
////////////////////////////////////////////////////////////////////////////////
void	CSmashatorium::Add(CSmash* pSmash,CSmashatoriumList *pList)
	{
	ASSERT(pList);
	ASSERT(pSmash);
	if (pSmash->m_sInGrid) return; // Don't need to re-add it!
	if (pSmash->m_pFat)
		{
		AddFat(pSmash->m_pFat);
		return;
		}
	//------------------------------------
	pSmash->m_sInGrid = TRUE;

	AddLimb(pList,&pSmash->m_link1);
	AddLimb(pList + m_sGridW,&pSmash->m_link3);
	AddLimb(++pList,&pSmash->m_link2);
	AddLimb(pList + m_sGridW,&pSmash->m_link4);

	m_sNumInSmash++;
	if (m_sNumInSmash > m_sMaxNumInSmash) m_sMaxNumInSmash = m_sNumInSmash;
	}

////////////////////////////////////////////////////////////////////////////////
//
//		AddFat
//
// Higher Level -> add an entire CSmash into the 'torium
// User calls Update, which checks for clipping
// This routine ASSUMES not clipped out!
// This is specially tailored to a fat smash object.
// Clipping information should be set in the FatSmash before passing.
// All links should be NULLED if clipped!
//
////////////////////////////////////////////////////////////////////////////////
void	CSmashatorium::AddFat(CFatSmash* pFatSmash)
	{
	ASSERT(pFatSmash);
	if (pFatSmash->m_pParent->m_sInGrid) return; // Don't need to re-add it!
	//=====================================
	pFatSmash->m_pParent->m_sInGrid = TRUE;
	short i,j;
	CSmashatoriumList*	pList = pFatSmash->m_pClippedGrid;	// assume not clipped out!
	CSmashLink*	pLink = pFatSmash->m_pFirstLink;
	//-------------------------------------
	for (j=0; j < pFatSmash->m_sClipH; j++,pList += m_sGridW - pFatSmash->m_sClipW, 
													pLink += pFatSmash->m_sW - pFatSmash->m_sClipW)
		{
		for (i=0; i < pFatSmash->m_sClipW; i++,pList++,pLink++)
			{
			AddLimb(pList,pLink);  // ASSUME already cleared out!
			}
		}
	//=====================================
	m_sNumInSmash++;
	if (m_sNumInSmash > m_sMaxNumInSmash) m_sMaxNumInSmash = m_sNumInSmash;

	TRACE("Fat added\n");
	}

////////////////////////////////////////////////////////////////////////////////
//
//
//	Remove
//
// This is on a per object level:
// Remove the CSmash from the smashatorium
//
////////////////////////////////////////////////////////////////////////////////
void	CSmashatorium::Remove(CSmash* pSmash)
	{
	if (pSmash->m_sInGrid == FALSE) return; // don't need to remove it!
	if (pSmash->m_pFat)
		{
		RemoveFat(pSmash->m_pFat);
		return;
		}

	m_sNumInSmash--;
	pSmash->m_sInGrid = FALSE;
	CSmashLink* pLink;

	pLink = &pSmash->m_link1;
	// In the current scheme, it's all or nothing:
	if (pLink->m_pLast) // assume all four legs are here
		{
		RemoveLimb(pLink->m_pLast,pLink);
		pLink = &pSmash->m_link2;
		RemoveLimb(pLink->m_pLast,pLink);
		pLink = &pSmash->m_link3;
		RemoveLimb(pLink->m_pLast,pLink);
		pLink = &pSmash->m_link4;
		RemoveLimb(pLink->m_pLast,pLink);
		}
	}

////////////////////////////////////////////////////////////////////////////////
//
//
//	RemoveFat
//
// This is on a per object level:
// Remove the CSmash from the smashatorium
// Specialized to remove a fat object.
// It assumes that the range given takes clipping into account
//
////////////////////////////////////////////////////////////////////////////////
void	CSmashatorium::RemoveFat(CFatSmash* pFatSmash)
	{
	ASSERT(pFatSmash);
	if (pFatSmash->m_pParent->m_sInGrid == FALSE) return; // don't need to remove it!
	//===========================================
	pFatSmash->m_pParent->m_sInGrid = FALSE;
	short i,j;

	//****** HERE IS A BIG DESIGN FLAW!!!!! *****
	CSmashLink*	pLink = pFatSmash->m_pLinks;	// do them all!
	//-------------------------------------
	for (j=0; j < pFatSmash->m_sH; j++)
		{
		for (i=0; i < pFatSmash->m_sW; i++,pLink++)	// pLink will wrap to the next line
			{
			if (pLink->m_pLast)
				{
				RemoveLimb(pLink->m_pLast,pLink);  // ASSUME already cleared out!
				}
			}
		}
	//===========================================
	m_sNumInSmash--;

	TRACE("Fat removed\n");
	}

//******************************************************************************
//********************************  CFatSmash  *********************************
//******************************************************************************

////////////////////////////////////////////////////////////////////////////////
//
//	CFatSmash::Erase - clear all values but do not deallocate
//
////////////////////////////////////////////////////////////////////////////////
void	CFatSmash::Erase()
	{
	m_sClipX = m_sClipY = m_sClipW = m_sClipH = m_sW = 
		m_sH = m_sNumGrids = 0;
	m_pClippedGrid = NULL;
	m_pLinks = m_pFirstLink = NULL; // Must be deleted first!
	m_pParent = NULL;
	m_lX = m_lY = 0;
	}


////////////////////////////////////////////////////////////////////////////////
//
//	CFatSmash::Destroy - deallocate the extra smash links...
//
////////////////////////////////////////////////////////////////////////////////
void	CFatSmash::Destroy()
	{
	ASSERT(m_pLinks);

	delete [] m_pLinks;	// SHOULD be safe
	Erase();
	}


////////////////////////////////////////////////////////////////////////////////
//
//	CFatSmash::Alloc - This instantiates a list of extra SmashLinks
//
//	RETURNS:	SUCCESS OR FAILURE
//
////////////////////////////////////////////////////////////////////////////////
short	CFatSmash::Alloc(short sNumLinks)
	{
	m_pLinks = new CSmashLink[sNumLinks];
	if (m_pLinks) return SUCCESS;

	return FAILURE;
	}

////////////////////////////////////////////////////////////////////////////////
//
//
//


//
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
	//---------------------------------------------------------------------------

