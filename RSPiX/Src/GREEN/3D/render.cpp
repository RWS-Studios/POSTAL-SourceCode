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
#include "System.h"
#ifdef PATHS_IN_INCLUDES
	#include "GREEN/3D/render.h"
#else
	#include "render.h"
#endif

typedef struct 
	{ RFixedS32 x; RFixedS32 y; RFixedS32 z;} RRenderPt32; // For internal use


// Fog should be offset such that the first index occurs
// at the minimum z-point of the full3d object being
// rendered.
// sX and sY are additional offsets into pimDst
//
void	DrawTri_ZColorFog(UCHAR* pDstOffset,long lDstP,
			RP3d* p1,RP3d* p2,RP3d* p3,
			RZBuffer* pZB,UCHAR* pFog,
			short sOffsetX/* = 0*/,		// In: 2D offset for pZB.
			short sOffsetY/* = 0*/) 	// In: 2D offset for pZB.
	{
//////////////////////////////////////////////////////////////////
//****************************************************************
//======================  INITIAL SET UP  ========================
//****************************************************************
//////////////////////////////////////////////////////////////////

	// copy the 3d points into screen coordinates:
	RRenderPt32 pt1,pt2,pt3;
	RRenderPt32 *pv1 = &pt1;
	RRenderPt32 *pv2 = &pt2;
	RRenderPt32 *pv3 = &pt3;
	// Cast from REAL to short in fp32 format:
	pt1.x.mod = short(p1->x);
	pt1.y.mod = short(p1->y);
	pt1.z.mod = short(p1->z);
	pt2.x.mod = short(p2->x);
	pt2.y.mod = short(p2->y);
	pt2.z.mod = short(p2->z);
	pt3.x.mod = short(p3->x);
	pt3.y.mod = short(p3->y);
	pt3.z.mod = short(p3->z);
	pt1.x.frac = 
	pt2.x.frac = 
	pt3.x.frac = USHORT(32768); // offset each by 1/2

	/*
	// Catch the special case of a single pixel
	// This is really only useful for POSTAL TM, where polygons are tiny:
	if (pt1.x.mod == pt2.x.mod)	// hierarchal check to save time:
		{
		if ( (pt1.y.mod == pt2.y.mod)	// WE'VE GOT A SINGLE SCREEN POINT!
			 && (pt1.x.mod == pt3.x.mod)
			 && (pt1.y.mod == pt3.y.mod) )

			{
			// Try to best mimick normal behavior by using highest Z;
			// THIS will not exactly mimick the non-point cloud base,
			// but SHOULD work appropriately:

			if (pt2.z.mod > pt1.z.mod) pt1.z.mod = pt2.z.mod;
			if (pt3.z.mod > pt1.z.mod) pt1.z.mod = pt3.z.mod;

			//***** PLOT THE SINGLE POINT!
			short* pBufZ = pZB -> GetZPtr
				(pt1.x.mod + sOffsetX, pt1.y.mod + sOffsetY);

			if (pt1.z.mod > *pBufZ)
				{
				UCHAR* pDst = pDstOffset + lDstP * pt1.y.mod + pt1.x.mod; 
				*pDst = pFog[pt1.z.upper]; // can't forget to set the z-buffer!
				*pBufZ = pt1.z.mod;			
				}

			return;	// DONE THE FAST WAY!
			}
		}
		*/

	// Let's assess general categories:
	// Single points:
	/*
	short sAbort = TRUE;
	if (pt1.x.mod == pt2.x.mod)
		{
		if ( (pt1.y.mod == pt2.y.mod)	// WE'VE GOT A SINGLE SCREEN POINT!
			 && (pt1.x.mod == pt3.x.mod)
			 && (pt1.y.mod == pt3.y.mod) )
			{
			// single point
			sAbort = FALSE;
			}
		}

	// Vertical strips:
	if ( (pt1.x.mod == pt2.x.mod)
		&& (pt1.x.mod == pt3.x.mod) )
		{
		// Vstrip:
		sAbort = FALSE;
		}

	// Horizontal strips:
	if ( (pt1.y.mod == pt2.y.mod)
		&& (pt1.y.mod == pt3.y.mod) )
		{
		// Hstrip:
		sAbort = FALSE;
		}

	if (sAbort == TRUE) return; // isolate the effect!
	*/

	// Now let's try simply not drawing horiz or verical strips at all:
	// Vertical strips:
	/*
	if ( (pt1.x.mod == pt2.x.mod)
		&& (pt1.x.mod == pt3.x.mod) )
		{
		// Vstrip or point:
		return;
		}

	// Horizontal strips:
	if ( (pt1.y.mod == pt2.y.mod)
		&& (pt1.y.mod == pt3.y.mod) )
		{
		// Hstrip:
		return;
		}
		*/
	
	// sort the triangles and choose which mirror case to render.

	// Sort points lowest to highest by y-value:
	if (pv1->y.mod > pv2->y.mod) SWAP(pv1,pv2);
	if (pv1->y.mod > pv3->y.mod) SWAP(pv1,pv3);
	if (pv2->y.mod > pv3->y.mod) SWAP(pv2,pv3);

	// Get point 2 and 3's position relative to point 1:
	// Use 16 bit accuracy in y, 32-bit in x...
	short y1 = pv1->y.mod;

	short	y2 = pv2->y.mod - y1;
	short	y3 = pv3->y.mod - y1;
	short ybot = y3 - y2; // lower half delta

	if (y2 + y3 == 0) return; // don;t bother drawing horiz line

	// get relative floating point x coordinates: (32-bit differences)
	long fx1 = pv1->x.val;
	long fz1 = pv1->z.val;

	long fx2 = pv2->x.val - fx1;
	long fx3 = pv3->x.val - fx1;
	long fz2 = pv2->z.val - fz1;
	long fz3 = pv3->z.val - fz1;

	// calculate the top two edge slopes with 32-bit accuracy:
	long fx2inc,fz2inc;
	if (y2) 
		{
		fx2inc = fx2 / y2; // stuck with division using fx32!
		fz2inc = fz2 / y2; // stuck with division using fx32!
		/* CANDIDATE FOR ONE_OVER!
		fx2inc = ULONG(fx2.mod) * CInitNum::OneOver[y2]; // stuck with division using fx32!
		fz2inc = ULONG(fz2.mod) * CInitNum::OneOver[y2]; // stuck with division using fx32!
		*/
		}

	long fx3inc = fx3 / y3; // stuck with division using fx32!
	long fz3inc = fz3 / y3; // stuck with division using fx32!

	// Set the two absolute edge positions
	RFixedS32 x2,x3,z2,z3; 
	x2.val = x3.val = pv1->x.frac;	// preserve floating point x!
	z2.val = z3.val = pv1->z.frac; // preserve floating point z!

	short sBaseZ = pv1->z.mod; 
	//TRACE("SBASE Zpt = %hd\n",pv1->z.mod);


	long lP = lDstP;
	// add in extra piece uv rounding!
	UCHAR* pDst = pDstOffset + lP * pv1->y.mod + pv1->x.mod + x2.mod; 
	short* pBufZ = pZB -> GetZPtr(pv1->x.mod + x2.mod + sOffsetX, pv1->y.mod + sOffsetY);
	long lZP = pZB->m_lP; // in words!!!

	// Draw the upper triangle! (Assuming fx2inc < fx3inc.....)
	short x,y;
	RFixedS32	fz,fzinc; // for tracing across each scan line:
	short xdel;

//////////////////////////////////////////////////////////////////
//****************************************************************
//======================  CHOOSE II CASES  =======================
//****************************************************************
//////////////////////////////////////////////////////////////////

	//****************************************************************
	//======================  FLAT TOP TRIANGLE  =====================
	//****************************************************************
	// Don't worry about the wasted calculations above!
	// (I STILL THINK YOU CAN STREAMLINE THIS AND INTEGRATE IT INTO
	// THE OTHER CASE -> THERE IS NO CALL FOR THIS!)

	if (y2 == 0) // p1.y == p2.y
		{
		long fx1inc,fz1inc;
		RFixedS32 x1,z1; // Absolute positions

		// Let point I be to the LEFT of point II:
		if (pv2->x.val < pv1->x.val) SWAP(pv1,pv2);
		// again, set initial points relative to p1(0,0):
		fx1 = pv1->x.val;
		fz1 = pv1->z.val;

		fx2 = pv2->x.val - fx1;
		fx3 = pv3->x.val - fx1;
		fz2 = pv2->z.val - fz1;
		fz3 = pv3->z.val - fz1;
		sBaseZ = pv1->z.mod;

		// This time, we want the end slopes:
		//*******   CANDIDATE FOR ONE_OVER!
		fx1inc = fx3 / y3; // from pt 1 to pt 3
		fz1inc = fz3 / y3;
		fx2inc = (fx3 - fx2) / y3; // from pt 2 to pt 3
		fz2inc = (fz3 - fz2) / y3; // from pt 2 to pt 3

		x1.val = pv1->x.frac; // preserve floating point x!
		x2.val = fx2;
		z1.val = pv1->z.frac; // preserve floating point x!
		z2.val = fz2;			// preserve floating point z!

		pDst = pDstOffset + lP * pv1->y.mod + pv1->x.mod + x1.mod; // add in extra piece uv rounding!
		pBufZ = pZB -> GetZPtr(pv1->x.mod + x1.mod + sOffsetX, pv1->y.mod + sOffsetY);

		for (y = y3;y;y--)
			{
			pDst += lP;
			pBufZ += lZP;
			x2.val += fx2inc;
			x1.val += fx1inc;
			z2.val += fz2inc;
			z1.val += fz1inc;

			// Now we scan z across (slow for now)
			fz.val = z1.val;
			fz.mod += sBaseZ; // ****** advance the z-value.
			xdel = x2.mod - x1.mod;
			//if (hzdel) hzinc.val = (z3.val - z2.val) / hzdel;
			//***************8 flipped the inc value:!
			if (xdel) fzinc.val = long(z2.mod - z1.mod) * RInitNum::OneOver[xdel];
			//if (hzdel) Mul(hzinc,z3.val - z2.val,CInitNum::OneOver[hzdel]);

			// Assume 2 to 3:
			for (x = x1.mod; x<= x2.mod;x++)
				{
				if (fz.mod > *(pBufZ+x) )
					{
					*(pDst+x) = pFog[fz.upper];// pFog[fz.upper];	// offset for fog
					*(pBufZ+x) = fz.mod;				// set Z-buffer!
					}
				fz.val += fzinc.val;
				}
			}

		return;
		}
	//****************************************************************
	//======================  NORMAL TRIANGLE  =======================
	//****************************************************************
	if (fx2inc <= fx3inc)
		{
		//_DrawTri(pDst,lP,x2,x3,fx2inc,fx3inc,y2,ucColor);
		for (y = y2;y;y--)
			{
			pDst += lP;
			pBufZ += lZP;
			x2.val += fx2inc;
			x3.val += fx3inc;
			z2.val += fz2inc;
			z3.val += fz3inc;
			//TRACE("SBASE Z = %hd\n",sBaseZ);

			// Assume 2 to 3:
			// Now we scan z across (slow for now)
			fz.val = z2.val;
			fz.mod += sBaseZ; // This is for coloring but it effects true z as well!
			xdel = x3.mod - x2.mod;
			//if (hzdel) hzinc.val = (z3.val - z2.val) / hzdel;
			if (xdel) fzinc.val = ULONG(z3.mod - z2.mod) * RInitNum::OneOver[xdel];
			//if (hzdel) Mul(hzinc,z3.val - z2.val,CInitNum::OneOver[hzdel]);
			// Assume 2 to 3:
			for (x = x2.mod; x<= x3.mod;x++)
				{
				if (fz.mod > *(pBufZ+x) )
					{
					*(pDst+x) = pFog[fz.upper];// pFog[fz.upper];
					*(pBufZ+x) = fz.mod;// set Z-buffer!
					}
				fz.val += fzinc.val;
				}
			}

		//===================================================================
		// Draw the lower triangle if applicatable:
		//===================================================================

		if (ybot)
			{
			// new x2 slope:
			fx2inc = (fx3 - fx2) / ybot;  // stuck with division using fx32!
			fz2inc = (fz3 - fz2) / ybot;  // stuck with division using fx32!
			/*
			fx2inc = ULONG(fx3.mod-fx2.mod) * CInitNum::OneOver[yc]; // stuck with division using fx32!
			fz2inc = ULONG(fz3.mod-fz2.mod) * CInitNum::OneOver[yc]; // stuck with division using fx32!
			*/
			//_DrawTri(pDst,lP,x2,x3,fx2inc,fx3inc,yc,ucColor);

			for (y = ybot;y;y--)
				{
				pDst += lP;
				pBufZ += lZP;
				x2.val += fx2inc;
				x3.val += fx3inc;
				z2.val += fz2inc;
				z3.val += fz3inc;

				// Now we scan z across (slow for now)
				fz.val = z2.val;
				fz.mod += sBaseZ;
				xdel = x3.mod - x2.mod;
				//if (hzdel) hzinc.val = (z3.val - z2.val) / hzdel;
				if (xdel) fzinc.val = long(z3.mod - z2.mod) * RInitNum::OneOver[xdel];
				//if (hzdel) Mul(hzinc,z3.val - z2.val,CInitNum::OneOver[hzdel]);

				// Assume 2 to 3:
				for (x = x2.mod; x<= x3.mod;x++)
					{
					if (fz.mod > *(pBufZ+x) )
						{
						*(pDst+x) = pFog[fz.upper];// pFog[fz.upper];
						*(pBufZ+x) = fz.mod;// set Z-buffer!
						}
					fz.val += fzinc.val;
					}
				}
			}
		}

	else // flip the x drawing order:

		//===================================================================
		//============== DRAW MIRRORED VERION OF STANDARD TRIANGLE! =========
		//===================================================================

		{

		//_DrawTri(pDst,lP,x2,x3,fx2inc,fx3inc,y2,ucColor);
		for (y = y2;y;y--)
			{
			pDst += lP;
			pBufZ += lZP;
			x2.val += fx2inc;
			x3.val += fx3inc;
			z2.val += fz2inc;
			z3.val += fz3inc;
			// Assume 2 to 3:
			// Now we scan z across (slow for now)
			fz.val = z3.val;
			fz.mod += sBaseZ;

			xdel = x2.mod - x3.mod; //+ x to z
			//if (hzdel) hzinc.val = (z2.val - z3.val) / hzdel;
			if (xdel) fzinc.val = long(z2.mod - z3.mod) * RInitNum::OneOver[xdel];
			//if (hzdel) Mul(hzinc,z2.val - z3.val,CInitNum::OneOver[hzdel]);

			// Assume 2 to 3:
			for (x = x3.mod; x<= x2.mod;x++)
				{
				if (fz.mod > *(pBufZ+x) )
					{
					*(pDst+x) = pFog[fz.upper];// pFog[fz.upper];
					*(pBufZ+x) = fz.mod;// set Z-buffer!
					}
				fz.val += fzinc.val;
				}
			}

		//===================================================================
		// Draw the lower triangle if applicatable:
		//===================================================================

		if (ybot)
			{
			// new x2 slope:
			fx2inc = (fx3 - fx2) / ybot;  // stuck with division using fx32!
			fz2inc = (fz3 - fz2) / ybot;  // stuck with division using fx32!
			/*
			fx2inc = ULONG(fx3.mod-fx2.mod) * CInitNum::OneOver[yc]; // stuck with division using fx32!
			fz2inc = ULONG(fz3.mod-fz2.mod) * CInitNum::OneOver[yc]; // stuck with division using fx32!
			*/
			//_DrawTri(pDst,lP,x2,x3,fx2inc,fx3inc,yc,ucColor);

			for (y = ybot;y;y--)
				{
				pDst += lP;
				pBufZ += lZP;
				x2.val += fx2inc;
				x3.val += fx3inc;
				z2.val += fz2inc;
				z3.val += fz3inc;

				// Now we scan z across (slow for now)
				fz.val = z3.val;
				fz.mod += sBaseZ;
				xdel = x2.mod - x3.mod;
				//if (hzdel) hzinc.val = (z2.val - z3.val) / hzdel;
				if (xdel) fzinc.val = ULONG(z2.mod - z3.mod) * RInitNum::OneOver[xdel];
				// Full accuracy fxMul!
				//if (hzdel) Mul(hzinc,z2.val - z3.val,CInitNum::OneOver[hzdel]);
				// Assume 2 to 3:
				for (x = x3.mod; x<= x2.mod;x++)
					{
					if (fz.mod > *(pBufZ+x) )
						{
						*(pDst+x) = pFog[fz.upper];// pFog[fz.upper];
						*(pBufZ+x) = fz.mod;// set Z-buffer!
						}
					fz.val += fzinc.val;
					}
				}
			}
		}
	}

//================================================== 
// For debugging:
void	DrawTri_wire(RImage* pimDst,short sX,short sY,
			RP3d* p1,RP3d* p2,RP3d* p3,UCHAR ucColor)
	{
	rspLine(ucColor,pimDst,
		sX+short(p1->x),sY+short(p1->y),
		sX+short(p2->x),sY+short(p2->y));
	rspLine(ucColor,pimDst,
		sX+short(p1->x),sY+short(p1->y),
		sX+short(p3->x),sY+short(p3->y));
	rspLine(ucColor,pimDst,
		sX+short(p3->x),sY+short(p3->y),
		sX+short(p2->x),sY+short(p2->y));
	}

//================================================== 
// For debugging:
// FLAT SHADED!
// sX and sY are additional offsets into pimDst
//
void	DrawTri_ZColor(UCHAR* pDstOffset,long lDstP,
			RP3d* p1,RP3d* p2,RP3d* p3,
			RZBuffer* pZB,UCHAR ucFlatColor,
			short sOffsetX/* = 0*/,		// In: 2D offset for pZB.
			short sOffsetY/* = 0*/) 	// In: 2D offset for pZB.
	{
//////////////////////////////////////////////////////////////////
//****************************************************************
//======================  INITIAL SET UP  ========================
//****************************************************************
//////////////////////////////////////////////////////////////////

	// copy the 3d points into screen coordinates:
	RRenderPt32 pt1,pt2,pt3;
	RRenderPt32 *pv1 = &pt1;
	RRenderPt32 *pv2 = &pt2;
	RRenderPt32 *pv3 = &pt3;
	// Cast from REAL to short in fp32 format:
	pt1.x.mod = short(p1->x);
	pt1.y.mod = short(p1->y);
	pt1.z.mod = short(p1->z);
	pt2.x.mod = short(p2->x);
	pt2.y.mod = short(p2->y);
	pt2.z.mod = short(p2->z);
	pt3.x.mod = short(p3->x);
	pt3.y.mod = short(p3->y);
	pt3.z.mod = short(p3->z);
	pt1.x.frac = 
	pt2.x.frac = 
	pt3.x.frac = USHORT(32768); // offset each by 1/2
	
	// sort the triangles and choose which mirror case to render.

	// Sort points lowest to highest by y-value:
	if (pv1->y.mod > pv2->y.mod) SWAP(pv1,pv2);
	if (pv1->y.mod > pv3->y.mod) SWAP(pv1,pv3);
	if (pv2->y.mod > pv3->y.mod) SWAP(pv2,pv3);

	// Get point 2 and 3's position relative to point 1:
	// Use 16 bit accuracy in y, 32-bit in x...
	short y1 = pv1->y.mod;

	short	y2 = pv2->y.mod - y1;
	short	y3 = pv3->y.mod - y1;
	short ybot = y3 - y2; // lower half delta

	if (y2 + y3 == 0) return; // don;t bother drawing horiz line

	// get relative floating point x coordinates: (32-bit differences)
	long fx1 = pv1->x.val;
	long fz1 = pv1->z.val;

	long fx2 = pv2->x.val - fx1;
	long fx3 = pv3->x.val - fx1;
	long fz2 = pv2->z.val - fz1;
	long fz3 = pv3->z.val - fz1;

	// calculate the top two edge slopes with 32-bit accuracy:
	long fx2inc,fz2inc;
	if (y2) 
		{
		fx2inc = fx2 / y2; // stuck with division using fx32!
		fz2inc = fz2 / y2; // stuck with division using fx32!
		/* CANDIDATE FOR ONE_OVER!
		fx2inc = ULONG(fx2.mod) * CInitNum::OneOver[y2]; // stuck with division using fx32!
		fz2inc = ULONG(fz2.mod) * CInitNum::OneOver[y2]; // stuck with division using fx32!
		*/
		}

	long fx3inc = fx3 / y3; // stuck with division using fx32!
	long fz3inc = fz3 / y3; // stuck with division using fx32!

	// Set the two absolute edge positions
	RFixedS32 x2,x3,z2,z3; 
	x2.val = x3.val = pv1->x.frac;	// preserve floating point x!
	z2.val = z3.val = pv1->z.frac; // preserve floating point z!

	short sBaseZ = pv1->z.mod; 
	//TRACE("SBASE Zpt = %hd\n",pv1->z.mod);


	long lP = lDstP;
	// add in extra piece uv rounding!
	UCHAR* pDst = pDstOffset + lP * pv1->y.mod + pv1->x.mod + x2.mod; 
	short* pBufZ = pZB -> GetZPtr(pv1->x.mod + x2.mod + sOffsetX, pv1->y.mod + sOffsetY);
	long lZP = pZB->m_lP; // in words!!!

	// Draw the upper triangle! (Assuming fx2inc < fx3inc.....)
	short x,y;
	RFixedS32	fz,fzinc; // for tracing across each scan line:
	short xdel;

//////////////////////////////////////////////////////////////////
//****************************************************************
//======================  CHOOSE II CASES  =======================
//****************************************************************
//////////////////////////////////////////////////////////////////

	//****************************************************************
	//======================  FLAT TOP TRIANGLE  =====================
	//****************************************************************
	// Don't worry about the wasted calculations above!
	// (I STILL THINK YOU CAN STREAMLINE THIS AND INTEGRATE IT INTO
	// THE OTHER CASE -> THERE IS NO CALL FOR THIS!)

	if (y2 == 0) // p1.y == p2.y
		{
		long fx1inc,fz1inc;
		RFixedS32 x1,z1; // Absolute positions

		// Let point I be to the LEFT of point II:
		if (pv2->x.val < pv1->x.val) SWAP(pv1,pv2);
		// again, set initial points relative to p1(0,0):
		fx1 = pv1->x.val;
		fz1 = pv1->z.val;

		fx2 = pv2->x.val - fx1;
		fx3 = pv3->x.val - fx1;
		fz2 = pv2->z.val - fz1;
		fz3 = pv3->z.val - fz1;
		sBaseZ = pv1->z.mod;

		// This time, we want the end slopes:
		//*******   CANDIDATE FOR ONE_OVER!
		fx1inc = fx3 / y3; // from pt 1 to pt 3
		fz1inc = fz3 / y3;
		fx2inc = (fx3 - fx2) / y3; // from pt 2 to pt 3
		fz2inc = (fz3 - fz2) / y3; // from pt 2 to pt 3

		x1.val = pv1->x.frac; // preserve floating point x!
		x2.val = fx2;
		z1.val = pv1->z.frac; // preserve floating point x!
		z2.val = fz2;			// preserve floating point z!

		pDst = pDstOffset + lP * pv1->y.mod + pv1->x.mod + x1.mod; // add in extra piece uv rounding!
		pBufZ = pZB -> GetZPtr(pv1->x.mod + x1.mod + sOffsetX, pv1->y.mod + sOffsetY);

		for (y = y3;y;y--)
			{
			pDst += lP;
			pBufZ += lZP;
			x2.val += fx2inc;
			x1.val += fx1inc;
			z2.val += fz2inc;
			z1.val += fz1inc;

			// Now we scan z across (slow for now)
			fz.val = z1.val;
			fz.mod += sBaseZ; // ****** advance the z-value.
			xdel = x2.mod - x1.mod;
			//if (hzdel) hzinc.val = (z3.val - z2.val) / hzdel;
			//***************8 flipped the inc value:!
			if (xdel) fzinc.val = long(z2.mod - z1.mod) * RInitNum::OneOver[xdel];
			//if (hzdel) Mul(hzinc,z3.val - z2.val,CInitNum::OneOver[hzdel]);

			// Assume 2 to 3:
			for (x = x1.mod; x<= x2.mod;x++)
				{
				if (fz.mod > *(pBufZ+x) )
					{
					*(pDst+x) = ucFlatColor;// pFog[fz.upper];	// offset for fog
					*(pBufZ+x) = fz.mod;				// set Z-buffer!
					}
				fz.val += fzinc.val;
				}
			}

		return;
		}
	//****************************************************************
	//======================  NORMAL TRIANGLE  =======================
	//****************************************************************
	if (fx2inc <= fx3inc)
		{
		//_DrawTri(pDst,lP,x2,x3,fx2inc,fx3inc,y2,ucColor);
		for (y = y2;y;y--)
			{
			pDst += lP;
			pBufZ += lZP;
			x2.val += fx2inc;
			x3.val += fx3inc;
			z2.val += fz2inc;
			z3.val += fz3inc;
			//TRACE("SBASE Z = %hd\n",sBaseZ);

			// Assume 2 to 3:
			// Now we scan z across (slow for now)
			fz.val = z2.val;
			fz.mod += sBaseZ; // This is for coloring but it effects true z as well!
			xdel = x3.mod - x2.mod;
			//if (hzdel) hzinc.val = (z3.val - z2.val) / hzdel;
			if (xdel) fzinc.val = ULONG(z3.mod - z2.mod) * RInitNum::OneOver[xdel];
			//if (hzdel) Mul(hzinc,z3.val - z2.val,CInitNum::OneOver[hzdel]);
			// Assume 2 to 3:
			for (x = x2.mod; x<= x3.mod;x++)
				{
				if (fz.mod > *(pBufZ+x) )
					{
					*(pDst+x) = ucFlatColor;// pFog[fz.upper];
					*(pBufZ+x) = fz.mod;// set Z-buffer!
					}
				fz.val += fzinc.val;
				}
			}

		//===================================================================
		// Draw the lower triangle if applicatable:
		//===================================================================

		if (ybot)
			{
			// new x2 slope:
			fx2inc = (fx3 - fx2) / ybot;  // stuck with division using fx32!
			fz2inc = (fz3 - fz2) / ybot;  // stuck with division using fx32!
			/*
			fx2inc = ULONG(fx3.mod-fx2.mod) * CInitNum::OneOver[yc]; // stuck with division using fx32!
			fz2inc = ULONG(fz3.mod-fz2.mod) * CInitNum::OneOver[yc]; // stuck with division using fx32!
			*/
			//_DrawTri(pDst,lP,x2,x3,fx2inc,fx3inc,yc,ucColor);

			for (y = ybot;y;y--)
				{
				pDst += lP;
				pBufZ += lZP;
				x2.val += fx2inc;
				x3.val += fx3inc;
				z2.val += fz2inc;
				z3.val += fz3inc;

				// Now we scan z across (slow for now)
				fz.val = z2.val;
				fz.mod += sBaseZ;
				xdel = x3.mod - x2.mod;
				//if (hzdel) hzinc.val = (z3.val - z2.val) / hzdel;
				if (xdel) fzinc.val = long(z3.mod - z2.mod) * RInitNum::OneOver[xdel];
				//if (hzdel) Mul(hzinc,z3.val - z2.val,CInitNum::OneOver[hzdel]);

				// Assume 2 to 3:
				for (x = x2.mod; x<= x3.mod;x++)
					{
					if (fz.mod > *(pBufZ+x) )
						{
						*(pDst+x) = ucFlatColor;// pFog[fz.upper];
						*(pBufZ+x) = fz.mod;// set Z-buffer!
						}
					fz.val += fzinc.val;
					}
				}
			}
		}

	else // flip the x drawing order:

		//===================================================================
		//============== DRAW MIRRORED VERION OF STANDARD TRIANGLE! =========
		//===================================================================

		{

		//_DrawTri(pDst,lP,x2,x3,fx2inc,fx3inc,y2,ucColor);
		for (y = y2;y;y--)
			{
			pDst += lP;
			pBufZ += lZP;
			x2.val += fx2inc;
			x3.val += fx3inc;
			z2.val += fz2inc;
			z3.val += fz3inc;
			// Assume 2 to 3:
			// Now we scan z across (slow for now)
			fz.val = z3.val;
			fz.mod += sBaseZ;

			xdel = x2.mod - x3.mod; //+ x to z
			//if (hzdel) hzinc.val = (z2.val - z3.val) / hzdel;
			if (xdel) fzinc.val = long(z2.mod - z3.mod) * RInitNum::OneOver[xdel];
			//if (hzdel) Mul(hzinc,z2.val - z3.val,CInitNum::OneOver[hzdel]);

			// Assume 2 to 3:
			for (x = x3.mod; x<= x2.mod;x++)
				{
				if (fz.mod > *(pBufZ+x) )
					{
					*(pDst+x) = ucFlatColor;// pFog[fz.upper];
					*(pBufZ+x) = fz.mod;// set Z-buffer!
					}
				fz.val += fzinc.val;
				}
			}

		//===================================================================
		// Draw the lower triangle if applicatable:
		//===================================================================

		if (ybot)
			{
			// new x2 slope:
			fx2inc = (fx3 - fx2) / ybot;  // stuck with division using fx32!
			fz2inc = (fz3 - fz2) / ybot;  // stuck with division using fx32!
			/*
			fx2inc = ULONG(fx3.mod-fx2.mod) * CInitNum::OneOver[yc]; // stuck with division using fx32!
			fz2inc = ULONG(fz3.mod-fz2.mod) * CInitNum::OneOver[yc]; // stuck with division using fx32!
			*/
			//_DrawTri(pDst,lP,x2,x3,fx2inc,fx3inc,yc,ucColor);

			for (y = ybot;y;y--)
				{
				pDst += lP;
				pBufZ += lZP;
				x2.val += fx2inc;
				x3.val += fx3inc;
				z2.val += fz2inc;
				z3.val += fz3inc;

				// Now we scan z across (slow for now)
				fz.val = z3.val;
				fz.mod += sBaseZ;
				xdel = x2.mod - x3.mod;
				//if (hzdel) hzinc.val = (z2.val - z3.val) / hzdel;
				if (xdel) fzinc.val = ULONG(z2.mod - z3.mod) * RInitNum::OneOver[xdel];
				// Full accuracy fxMul!
				//if (hzdel) Mul(hzinc,z2.val - z3.val,CInitNum::OneOver[hzdel]);
				// Assume 2 to 3:
				for (x = x3.mod; x<= x2.mod;x++)
					{
					if (fz.mod > *(pBufZ+x) )
						{
						*(pDst+x) = ucFlatColor;// pFog[fz.upper];
						*(pBufZ+x) = fz.mod;// set Z-buffer!
						}
					fz.val += fzinc.val;
					}
				}
			}
		}
	}

//================================================== 
// FLAT SHADED!
// sX and sY are additional offsets into pimDst
// There is NO Z_BUFFER here!  It is JUST a polygon drawer
//
void	DrawTri(UCHAR* pDstOffset,long lDstP,
			RP3d* p1,RP3d* p2,RP3d* p3,
			UCHAR ucFlatColor)
	{
//////////////////////////////////////////////////////////////////
//****************************************************************
//======================  INITIAL SET UP  ========================
//****************************************************************
//////////////////////////////////////////////////////////////////

	// copy the 3d points into screen coordinates:
	RRenderPt32 pt1,pt2,pt3;
	RRenderPt32 *pv1 = &pt1;
	RRenderPt32 *pv2 = &pt2;
	RRenderPt32 *pv3 = &pt3;
	// Cast from REAL to short in fp32 format:
	pt1.x.mod = short(p1->x);
	pt1.y.mod = short(p1->y);
	pt2.x.mod = short(p2->x);
	pt2.y.mod = short(p2->y);
	pt3.x.mod = short(p3->x);
	pt3.y.mod = short(p3->y);
	pt1.x.frac = 
	pt2.x.frac = 
	pt3.x.frac = USHORT(32768); // offset each by 1/2
	
	// sort the triangles and choose which mirror case to render.

	// Sort points lowest to highest by y-value:
	if (pv1->y.mod > pv2->y.mod) SWAP(pv1,pv2);
	if (pv1->y.mod > pv3->y.mod) SWAP(pv1,pv3);
	if (pv2->y.mod > pv3->y.mod) SWAP(pv2,pv3);

	// Get point 2 and 3's position relative to point 1:
	// Use 16 bit accuracy in y, 32-bit in x...
	short y1 = pv1->y.mod;

	short	y2 = pv2->y.mod - y1;
	short	y3 = pv3->y.mod - y1;
	short ybot = y3 - y2; // lower half delta

	if (y2 + y3 == 0) return; // don;t bother drawing horiz line

	// get relative floating point x coordinates: (32-bit differences)
	long fx1 = pv1->x.val;

	long fx2 = pv2->x.val - fx1;
	long fx3 = pv3->x.val - fx1;

	// calculate the top two edge slopes with 32-bit accuracy:
	long fx2inc;
	if (y2) 
		{
		fx2inc = fx2 / y2; // stuck with division using fx32!
		/* CANDIDATE FOR ONE_OVER!
		fx2inc = ULONG(fx2.mod) * CInitNum::OneOver[y2]; // stuck with division using fx32!
		*/
		}

	long fx3inc = fx3 / y3; // stuck with division using fx32!

	// Set the two absolute edge positions
	RFixedS32 x2,x3; 
	x2.val = x3.val = pv1->x.frac;	// preserve floating point x!

	long lP = lDstP;
	// add in extra piece uv rounding!
	UCHAR* pDst = pDstOffset + lP * pv1->y.mod + pv1->x.mod + x2.mod; 

	// Draw the upper triangle! (Assuming fx2inc < fx3inc.....)
	short x,y;
	short xdel;

//////////////////////////////////////////////////////////////////
//****************************************************************
//======================  CHOOSE II CASES  =======================
//****************************************************************
//////////////////////////////////////////////////////////////////

	//****************************************************************
	//======================  FLAT TOP TRIANGLE  =====================
	//****************************************************************
	// Don't worry about the wasted calculations above!
	// (I STILL THINK YOU CAN STREAMLINE THIS AND INTEGRATE IT INTO
	// THE OTHER CASE -> THERE IS NO CALL FOR THIS!)

	if (y2 == 0) // p1.y == p2.y
		{
		long fx1inc;
		RFixedS32 x1; // Absolute positions

		// Let point I be to the LEFT of point II:
		if (pv2->x.val < pv1->x.val) SWAP(pv1,pv2);
		// again, set initial points relative to p1(0,0):
		fx1 = pv1->x.val;

		fx2 = pv2->x.val - fx1;
		fx3 = pv3->x.val - fx1;

		// This time, we want the end slopes:
		//*******   CANDIDATE FOR ONE_OVER!
		fx1inc = fx3 / y3; // from pt 1 to pt 3
		fx2inc = (fx3 - fx2) / y3; // from pt 2 to pt 3

		x1.val = pv1->x.frac; // preserve floating point x!
		x2.val = fx2;

		pDst = pDstOffset + lP * pv1->y.mod + pv1->x.mod + x1.mod; // add in extra piece uv rounding!

		for (y = y3;y;y--)
			{
			pDst += lP;
			x2.val += fx2inc;
			x1.val += fx1inc;

			xdel = x2.mod - x1.mod;
			//***************8 flipped the inc value:!

			// Assume 2 to 3:
			for (x = x1.mod; x<= x2.mod;x++) *(pDst+x) = ucFlatColor;
			}

		return;
		}
	//****************************************************************
	//======================  NORMAL TRIANGLE  =======================
	//****************************************************************
	if (fx2inc <= fx3inc)
		{
		//_DrawTri(pDst,lP,x2,x3,fx2inc,fx3inc,y2,ucColor);
		for (y = y2;y;y--)
			{
			pDst += lP;
			x2.val += fx2inc;
			x3.val += fx3inc;

			// Assume 2 to 3:
			xdel = x3.mod - x2.mod;
			// Assume 2 to 3:
			for (x = x2.mod; x<= x3.mod;x++) *(pDst+x) = ucFlatColor;
			}

		//===================================================================
		// Draw the lower triangle if applicatable:
		//===================================================================

		if (ybot)
			{
			// new x2 slope:
			fx2inc = (fx3 - fx2) / ybot;  // stuck with division using fx32!
			/*
			fx2inc = ULONG(fx3.mod-fx2.mod) * CInitNum::OneOver[yc]; // stuck with division using fx32!
			*/
			//_DrawTri(pDst,lP,x2,x3,fx2inc,fx3inc,yc,ucColor);

			for (y = ybot;y;y--)
				{
				pDst += lP;
				x2.val += fx2inc;
				x3.val += fx3inc;

				xdel = x3.mod - x2.mod;

				// Assume 2 to 3:
				for (x = x2.mod; x<= x3.mod;x++) *(pDst+x) = ucFlatColor;
				}
			}
		}

	else // flip the x drawing order:

		//===================================================================
		//============== DRAW MIRRORED VERION OF STANDARD TRIANGLE! =========
		//===================================================================

		{

		//_DrawTri(pDst,lP,x2,x3,fx2inc,fx3inc,y2,ucColor);
		for (y = y2;y;y--)
			{
			pDst += lP;
			x2.val += fx2inc;
			x3.val += fx3inc;
			// Assume 2 to 3:

			xdel = x2.mod - x3.mod; //+ x to z

			// Assume 2 to 3:
			for (x = x3.mod; x<= x2.mod;x++) *(pDst+x) = ucFlatColor;
			}

		//===================================================================
		// Draw the lower triangle if applicatable:
		//===================================================================

		if (ybot)
			{
			// new x2 slope:
			fx2inc = (fx3 - fx2) / ybot;  // stuck with division using fx32!
			/*
			fx2inc = ULONG(fx3.mod-fx2.mod) * CInitNum::OneOver[yc]; // stuck with division using fx32!
			*/
			//_DrawTri(pDst,lP,x2,x3,fx2inc,fx3inc,yc,ucColor);

			for (y = ybot;y;y--)
				{
				pDst += lP;
				x2.val += fx2inc;
				x3.val += fx3inc;

				xdel = x2.mod - x3.mod;
				// Full accuracy fxMul!
				// Assume 2 to 3:
				for (x = x3.mod; x<= x2.mod;x++) *(pDst+x) = ucFlatColor;
				}
			}
		}
	}

