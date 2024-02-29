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
// crawler.h
// Project: Nostril (aka Postal)
//
//	History:
//		04/19/97 MJR	Started.
//
//		04/21/97	MJR/JMI	Converted to 3D/Converted to Postal.
//
//		04/21/97	JMI	Removed ASSERTs that values were greater than 0 in Move().
//							These were to protect against errors when using floor(),
//							which we no longer use.
//
//		04/29/97	JMI	CanWalk() no longer sets the clip case of the get attribute
//							call to include a max height (it just relies on 'no walk'
//							now).
//
//		05/29/97	JMI	Changed m_pRealm->m_pHeightMap->GetHeight() call to 
//							m_pRealm->GetTerrainAttributes().
//
//		06/26/97	JMI	Now uses m_prealm->Map3Dto2D() in Plot() (was using
//							the global version defined in reality.h which is no more).
//
//		07/01/97	JMI	Now uses GetHeightAndNoWalk() in place of 
//							GetTerrainAttributes().
//
//		08/10/97	JMI	Now crawls to the 'pushed' point.  If the crawler does make
//							it all the way to the 'pushed' point, we go back to the
//							point we crawled to just previously (which we know is 
//							valid).
//
//		08/11/97	JMI	Changed some casting in comparisons.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef CRAWLER_H
#define CRAWLER_H


#include "RSPiX.h"

#include "realm.h"
#include "reality.h"

class CCrawler
	{
	//------------------------------------------------------------------------------
	// Types, enums, etc.
	//------------------------------------------------------------------------------
	public:
		typedef struct
			{
			// These values are supplied by the user
			short sX;
			short sZ;
			short sHard;
			short sPushDir;
			double dPushMag;

			// These values are for internal use only
			double dPushX;
			double dPushZ;
//			double dDragX;
//			double dDragZ;

			short	sHeight;		// Stored height used only by IsGood().
			} Nub;

	//------------------------------------------------------------------------------
	// Variables
	//------------------------------------------------------------------------------
	protected:
		short m_sNum;
		Nub* m_pnub;
		double m_dPushX;
		double m_dPushZ;
		double m_dPushMaxX;
		double m_dPushMaxZ;

	public:
		CRealm*	m_prealm;			// Used to access the height map, the effect map,
											// the scene, all that good stuff.
		short	m_sVertTolerance;		// Maximum amount crawler can step up.

	//------------------------------------------------------------------------------
	// Functions
	//------------------------------------------------------------------------------
	public:
		////////////////////////////////////////////////////////////////////////////////
		// Constructor
		////////////////////////////////////////////////////////////////////////////////
		CCrawler()
			{
			m_pnub = 0;
			m_sNum = 0;
			m_prealm	= 0;
			}


		////////////////////////////////////////////////////////////////////////////////
		// Destructor
		////////////////////////////////////////////////////////////////////////////////
		~CCrawler()
			{
			m_pnub = 0;
			m_sNum = 0;
			}


		////////////////////////////////////////////////////////////////////////////////
		// Setup crawler
		////////////////////////////////////////////////////////////////////////////////
		void Setup(
			short sNum,										// In:  Number of Nub's in array
			Nub* pnub,										// In:  Pointer to array of Nub's
			double dPushMaxX,								// In:  Maximum push in x direction
			double dPushMaxZ)								// In:  Maximum push in z direction
			{
			// Save info
			m_sNum = sNum;
			m_pnub = pnub;
			m_dPushMaxX = dPushMaxX;
			m_dPushMaxZ	= dPushMaxZ;

			// Precalculate push and drag values based on specified directions and magnitudes
			for (short s = 0; s < m_sNum; s++)
				{
				m_pnub[s].dPushX =  rspCos(rspMod360(m_pnub[s].sPushDir)) * m_pnub[s].dPushMag;
				m_pnub[s].dPushZ = -rspSin(rspMod360(m_pnub[s].sPushDir)) * m_pnub[s].dPushMag;
				}
			}


		////////////////////////////////////////////////////////////////////////////////
		// Start at position #1 and move along the "path" leading to position #2, the
		// goal being to reach position #2.  The attributes encountered along the way
		// will determine how far we get before stopping.
		//
		// The returned position is the resulting position.  In the best case, it will
		// be equal to  position #2.  Worst case, it will be equal to position #1.
		// Otherwise, it will be somewhere between the two.
		//
		// As long as position #1 is valid, this function will be successfull.  This
		// should be the case as long as the caller starts out at a valid position and
		// never moves without using this function.
		//
		// If position #1 is NOT valid, this function will PROBABLY FAIL, as indicated
		// by a non-zero return value.  It is possible that it will manage to find what
		// appears to be a valid position; however, not only is this unlikely, but it
		// is important to note that it may only APPEAR to be valid -- there is no way
		// to know whether it is ACTUALLY valid.
		////////////////////////////////////////////////////////////////////////////////
		short Move(											// Returns 0 if successfull, non-zero otherwise
			double dx1,										// In:  Position #1 xcoord
			double dy1,										// In:  Position #1 ycoord
			double dz1,										// In:  Position #1 zcoord
			double dx2,										// In:  Position #2 xcoord
			double dy2,										// In:  Position #2 ycoord
			double dz2,										// In:  Position #2 zcoord
			double* pdx,									// Out: Final position xcoord
			double* pdy,									// Out: Final position ycoord
			double* pdz,									// Out: Final position zcoord
			short* psTerrainH)							// Out: Final terrain height

			{
			short sResult = 0;

			short sx1 = (short)dx1;
			short sy1 = (short)dy1;
			short sz1 = (short)dz1;
			short sx2 = (short)dx2;
			short sy2 = (short)dy2;
			short sz2 = (short)dz2;

			// Check if starting position is valid
			if (IsGood(sx1, sy1, sz1, psTerrainH))
				{

				// Creep as far as possible towards new position
				short sx;
				short sy;
				short	sz;
				CrawlWhileGood(sx1, sy1, sz1, sx2, sy2, sz2, &sx, &sy, &sz, psTerrainH);

				double dx	= sx;
				double dy	= sy;
				double dz	= sz;
				// If we made it to the end point, we should include the fractional value
				// lost in converting to integers when we initially assigned into
				// sx2 and sz2.
				if (sx == sx2)
					{
					dx = dx2;
					}

				if (sz == sz2)
					{
					dz = dz2;
					}

				if (dy2 < 0.0)
					dy += floor(dy2) - dy2;
				else
					dy += dy2 - floor(dy2);

				// Limit amount of push
				if (m_dPushX > m_dPushMaxX)
					m_dPushX = m_dPushMaxX;
				if (m_dPushZ > m_dPushMaxZ)
					m_dPushZ = m_dPushMaxZ;

				// Calculate "pushed" position and see if it's valid
				double dx2 = dx + m_dPushX;
				double dy2 = dy;
				double dz2 = dz + m_dPushZ;
				short  sTerrainH	= *psTerrainH;	// Start with a good value b/c CrawlWhileGood() 
															// won't update the value unless there's movement.
				CrawlWhileGood((short)dx, (short)dy, (short)dz, (short)dx2, (short)dy2, (short)dz2, &sx, &sy, &sz, &sTerrainH);
				if ((short)dx2 == sx && (short)dy2 == sy && (short)dz2 == sz)
					{
					// Use pushed position
					*pdx = dx2;
					*pdy = dy2;
					*pdz = dz2;
					*psTerrainH	= sTerrainH;
					}
				else
					{
					// Pushed position was not valid, so use known-good position instead
					*pdx = dx;
					*pdy = dy;
					*pdz = dz;
					}
				}
			else
				{
				// Starting position is invalid, so we're pretty much screwed.  As a
				// last-ditch effort, check if the ending position is miraculously valid.
				short  sTerrainH;
				if (IsGood(sx2, sy2, sz2, &sTerrainH) )
					{
					// Use the ending position.  This might make things worse by moving to
					// past some off-limits barrier, but since we don't have any other way
					// to get valid again, we might as well try this and hope for the best.
					*pdx = dx2;
					*pdy = dy2;
					*pdz = dz2;
					*psTerrainH	= sTerrainH;
					TRACE("Crawler::Move(): Starting position is invalid, using psuedo-valid ending position!\n");
					}
				else
					{
					// Stick with the starting position, which is, unfortunately, invalid
					*pdx = dx1;
					*pdy = dy1;
					*pdz = dz1;
					sResult = -1;
					TRACE("Crawler::Move(): Starting position is invalid!\n");
					}
				}

			return sResult;
			}


	protected:
		////////////////////////////////////////////////////////////////////////////////
		// Crawl as far as possible along the path from positon #1 to position #2, the
		// goal being to reach position #2.
		//
		// This crawls along one pixel at a time (just like a line-draw algorithm),
		// checking each point to see if it's valid.  If so, it goes on.  If not, it
		// returns the previous point, which was valid.
		//
		// This function assumes position #1 is valid!  If this is not the case, all
		// bets are off!!!
		////////////////////////////////////////////////////////////////////////////////
		void CrawlWhileGood(
			short sx1,
			short sy1,
			short sz1,
			short sx2,
			short sy2,
			short sz2,
			short* psOutX,
			short* psOutY,
			short* psOutZ,
			short* psTerrainH)	// In/Out: Terrain height.
			{
			short	sMaxTerrainH;

			// Clear push values
			m_dPushX = 0.0;
			m_dPushZ = 0.0;

			// Make a copy of the starting position
			short sx = sx1;
			short sy = sy1;
			short sz = sz1;

			// Set initial "known good" position
			short sGoodX = sx;
			short sGoodY = sy;
			short sGoodZ = sz;
			short	sGoodH = *psTerrainH;

			// Calculate delta x and delta y
			short sdx = sx2 - sx;
			short sdy = sy2 - sy;
			short sdz = sz2 - sz;

			// Make sure there's some movement, otherwise don't bother
			if (sdx || sdy || sdz)
				{

				// Calculate offsets for each delta
				short saddx = (sdx >= 0) ? +1 : -1;
				short saddy = (sdy >= 0) ? +1 : -1;
				short saddz = (sdz >= 0) ? +1 : -1;

				// Make all deltas positive
				sdx = ABS(sdx);
				sdy = ABS(sdy);
				sdz = ABS(sdz);

				// This uses a Bresenham-like algorithm to move one pixel at a time along the
				// line from (sx,sy,sz) to (sx2,sy2,sz2).	 We basically continue along until
				// we hit an invalid position or the ending position, whichever comes first.
				short serry;
				short serrx;
				short serrz;
				if ((sdx >= sdy) && (sdx >= sdz))
					{
					// dx is largest
					serry = sdx / 2;
					serrz = sdx / 2;
					do {
						serry += sdy;
						if (serry >= sdx)
							{
							serry -= sdx;
							sy += saddy;
							}
						serrz += sdz;
						if (serrz >= sdx)
							{
							serrz -= sdx;
							sz += saddz;
							}
						sx += saddx;
						if (!IsGood(sx, sy, sz, &sMaxTerrainH))
							break;
						sGoodX = sx;
						sGoodY = sy;
						sGoodZ = sz;
						sGoodH = sMaxTerrainH;
						} while (sx != sx2);
					}
				else if ((sdy >= sdx) && (sdy >= sdz))
					{
					// dy is largest
					serrx = sdy / 2;
					serrz = sdy / 2;
					do {
						serrx += sdx;
						if (serrx >= sdy)
							{
							serrx -= sdy;
							sx += saddx;
							}
						serrz += sdz;
						if (serrz >= sdy)
							{
							serrz -= sdy;
							sz += saddz;
							}
						sy += saddy;
						if (!IsGood(sx, sy, sz, &sMaxTerrainH))
							break;
						sGoodX = sx;
						sGoodY = sy;
						sGoodZ = sz;
						sGoodH = sMaxTerrainH;
						} while (sy != sy2);
					}
				else
					{
					// dz is largest
					serry = sdz / 2;
					serrx = sdz / 2;
					do {
						serry += sdy;
						if (serry >= sdz)
							{
							serry -= sdz;
							sy += saddy;
							}
						serrx += sdx;
						if (serrx >= sdz)
							{
							serrx -= sdz;
							sx += saddx;
							}
						sz += saddz;
						if (!IsGood(sx, sy, sz, &sMaxTerrainH))
							break;
						sGoodX = sx;
						sGoodY = sy;
						sGoodZ = sz;
						sGoodH = sMaxTerrainH;
						} while (sz != sz2);
					}
				}

			// Return good position
			*psOutX = sGoodX;
			*psOutY = sGoodY;
			*psOutZ = sGoodZ;
			*psTerrainH = sGoodH;
			}

		////////////////////////////////////////////////////////////////////////////////
		// Determine whether specified position is valid.
		//
		// Also updates push values as specified by any nubs are not valid.
		////////////////////////////////////////////////////////////////////////////////
		bool IsGood(
			short sBaseX,				// In
			short sBaseY,				// In
			short sBaseZ,				// In
			short* psMaxTerrainH)	// Out: Maximum terrain height from scan.
			{
			bool bResult = true;

			short sMaxTerrainH	= -32768;	// Default to lowest value.

			short	sTerrainH;
			for (short s = 0; s < m_sNum; s++)
				{
				short sx = sBaseX + m_pnub[s].sX;
				short sz = sBaseZ + m_pnub[s].sZ;

				if (CanWalk(sx, sBaseY, sz, &sTerrainH) == true)
					{
//					Plot((U8)0xfa, sx, sBaseY, sz);
					}
				else
					{
//					Plot((U8)0xf9, sx, sBaseY, sz);
					if (m_pnub[s].sHard)
						bResult = false;
					m_dPushX += m_pnub[s].dPushX;
					m_dPushZ += m_pnub[s].dPushZ;
					}

				// If this is a hard nub . . .
				if (m_pnub[s].sHard)
					{
					// If this height is larger than the previous max . . .
					if (sTerrainH > sMaxTerrainH)
						{
						sMaxTerrainH	= sTerrainH;
						}
					}
				}

			// Store for return.
			*psMaxTerrainH	= sMaxTerrainH;
			
			return bResult;
			}


		////////////////////////////////////////////////////////////////////////////////
		// Centralized location for checking attributes and deciding how to interpret
		// them.
		////////////////////////////////////////////////////////////////////////////////
		bool CanWalk(	// Returns true if we can walk there, false otherwise.
			short sx,	// In:  X position on attribute map.
			short	sy,	// In:  Y position on attribute map.
			short sz,	// In:  Z position on attribute map.
			short* psH)	// Out: Terrain height at X/Z.
			{
			bool	bCanWalk;
			bool	bCannotWalk;
			*psH	= m_prealm->GetHeightAndNoWalk(sx, sz, &bCannotWalk);
			if (bCannotWalk == true								// Not walkable
				|| (*psH - sy > m_sVertTolerance) )			// Terrain higher by m_sVertTolerance.
				{
				bCanWalk	= false;
				}
			else
				{
				bCanWalk	= true;
				}

			return bCanWalk;
			}

		////////////////////////////////////////////////////////////////////////////////
		// Plot a point via the CScene.  
		////////////////////////////////////////////////////////////////////////////////
		void Plot(		// Returns nothing.
			U8	u8Color,	// Color index.
			short	sx,	// In:  X position.
			short	sy,	// In:  Y position.
			short sz)	// In:  Z position.
			{
			// Create a line sprite.
			CSpriteLine2d*	psl2d	= new CSpriteLine2d;
			if (psl2d != NULL)
				{
				m_prealm->Map3Dto2D(
					sx, 
					sy, 
					sz, 
					&(psl2d->m_sX2), 
					&(psl2d->m_sY2) );

				psl2d->m_sX2End		= psl2d->m_sX2;
				psl2d->m_sY2End		= psl2d->m_sY2;

				psl2d->m_sPriority	= sz;
				psl2d->m_sLayer		= CRealm::GetLayerViaAttrib(m_prealm->GetLayer(sx, sz) );
				psl2d->m_u8Color		= u8Color;
				// Destroy when done.
				psl2d->m_sInFlags	= CSprite::InDeleteOnRender;
				// Put 'er there.
				m_prealm->m_scene.UpdateSprite(psl2d);
				}
			}

	};


#endif //CRAWLER_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
