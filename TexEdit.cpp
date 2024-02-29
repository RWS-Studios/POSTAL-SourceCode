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
// TexEdit.cpp
// Project: Nostril (aka Postal)
//
// This module implements the texture editor.
//
// History:
//		10/03/99 JMI	Started.
//
//		10/06/99	JMI	Now DoModal() accepts two lights to use and some other 
//							stuff too.
//
//		10/07/99	JMI	Changed the default Mudify settings to that of Assets.mak.
//
//					JMI	Replaced m_fAlt and m_fAzi with a transform so rotations
//							can always be relative to the current orientation.
//							Also added 'D' as an additional paint key so those keys
//							could all be on one hand.
//
//		10/08/99	JMI	Holding down shift now causes the manips to ignore the Y
//							input and holding control causes them to ignore the X 
//							input.
//
//					JMI	Added ValidateTextures() to check for and fix out-of-synch
//							texture files.
//							In Adjust(), we now free the colors array (otherwise, the
//							colors will get saved).  Some probably already exist.
// 
////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
// Includes.
//------------------------------------------------------------------------------

#include "RSPiX.h"

#include "TexEdit.h"

#include "Anim3D.h"
#include "update.h"
#include "game.h"	// For Paths.
#include "localize.h"

//------------------------------------------------------------------------------
// Macros.
//------------------------------------------------------------------------------

const RString	c_strGuiFile	= "res/editor/TexEdit.gui";

const long		c_lIdAnim			= 100;
const long		c_lIdPal				= 200;
const	long		c_lIdSpotLight		= 601;
const	long		c_lIdBrightness	= 602;
const	long		c_lIdAdjust			= 701;
const	long		c_lIdFrequency		= 702;
const	long		c_lIdAmount			= 703;
const long		c_lIdCurColor		= 201;
const long		c_lIdStatus			= 500;
const	long		c_lIdApply			= 301;
const	long		c_lIdSave			= 302;
const	long		c_lIdRevert			= 303;
const long		c_lIdQuit			= 399;
const long		c_lIdTrans			= 401;
const long		c_lIdScale			= 402;
const long		c_lIdRotate			= 403;
const long		c_lIdPaint			= 404;

const double	c_dScale			= 8.0;

const double	c_fTransRate	= 0.25f / c_dScale;
const double	c_fScaleRate	= 0.1f / c_dScale;
const double	c_fRotRate		= 1.0f;

const short		c_sPalStart		= 106;
const short		c_sPalEnd		= 201;
const	short		c_sPalNum		= c_sPalEnd - c_sPalStart + 1;

#if 0	// Swatch center colors only -- to guarantee good matching.
	const short		c_sPalColorsPerSwatch	= 8;
	const short		c_sPalCols		= 2;
#else	// All swatch colors -- to allow more flexibility when texturing guys.
	const short		c_sPalColorsPerSwatch	= 1;
	const short		c_sPalCols		= 8;
#endif

const short		c_sPalDispColorOffset	= c_sPalColorsPerSwatch / 2;
const short		g_sPalFirstColor			= c_sPalStart + c_sPalDispColorOffset;

const	short		c_sMinBrightness		= -64;
const	short		c_sMaxBrightness		= 64;
const short		c_sBrightnessRange	= c_sMaxBrightness - c_sMinBrightness + 1;

const	short		c_sDefAdjustFrequency	= 2;
const	float		c_fDefAdjustment			= 0.85f;


//------------------------------------------------------------------------------
// Functions.
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
// Set the state of the specified push button.
////////////////////////////////////////////////////////////////////////////////
inline
void
SetPushBtnState(
	RGuiItem* pguiRoot,		// In:  Root GUI.
	long lBtnId,				// In:  ID of btn whose state will be set.
	RPushBtn::State state)	// In:  New state for btn.
	{
	ASSERT(pguiRoot);
	RPushBtn*	pbtn = (RPushBtn*)pguiRoot->GetItemFromId(lBtnId);
	if (pbtn)
		{
		pbtn->m_state	= state;
		pbtn->Compose();
		}
	}


////////////////////////////////////////////////////////////////////////////////
// Checks to see if v1 and v2 are going in the same sign direction.
////////////////////////////////////////////////////////////////////////////////
bool					// Returns true if x, y, and z of v1 and v2 are the same (respective 3) signs.
SameSigns_Vector(
	RP3d *v1, 	// In: vector 1
	RP3d *v2)	// In: vector 1
	{
	 //this is a slooow check!, check the sign bit, or something
	 if(((int)v1->x >= 0 && (int)v2->x >= 0) || ((int)v1->x < 0 && (int)v2->x < 0))
		 {
		  if(((int)v1->y >= 0 && (int)v2->y >= 0) || ((int)v1->y < 0 && (int)v2->y < 0))
			  {
				if(((int)v1->z >= 0 && (int)v2->z >= 0) || ((int)v1->z < 0 && (int)v2->z < 0))
					return true;
			  }
		 }
	 return false;
	};

////////////////////////////////////////////////////////////////////////////////
// Using the equation in comments below, this returns the value for 't' from the 
//	parametric equations of line seg 0--1 and the plane of a, b, c, and D.
// if true, then you can use t, else, t will not have been assigned anything.
//         -(a*x0 + b*y0 + c*z0 + D)        = t
//	      a*(x1-x0)+b*(y1-y0)+c*(z1-z0)
////////////////////////////////////////////////////////////////////////////////
bool				// Returns: true of t valid, false otherwise
GetIntersectSubT(
	RP3d &pt0, // In: line segment point 0
	RP3d &ptSub, // In: line segment point 1
	RP3d &ptCoeff,
	//double a, double b, double c, double D, // In: coefficients
	float fD,
	float &t)  // In: return t
	{
	 float fdenom=ptCoeff.x*(ptSub.x) + ptCoeff.y*(ptSub.y) + ptCoeff.z*(ptSub.z);
	 if(fdenom!=0)
		 {
		  t= (ptCoeff.x*pt0.x + ptCoeff.y*pt0.y + ptCoeff.z*pt0.z + fD)/fdenom;
		  t*=-1;
		  return(true);
		 }
	 else 
		{
		return(false);
		};
	};

////////////////////////////////////////////////////////////////////////////////
// Checks to see if a point is inside (in the plane of and contained in) the
// given triangle.
////////////////////////////////////////////////////////////////////////////////
bool					// Return true if the given point is inside the given triangle
PointInsideTri(
	RP3d &pt0A, 	// In: point 1 of triangle
	RP3d &pt1A, 	// In: point 2 of triangle
	RP3d &pt2A, 	// In: point 3 of triangle
	RP3d &hitpoint)	// In: point to check inside triangle
	{
	/*
	if(cheapequalsRP3d(pt0A, hitpoint)
		|| cheapequalsRP3d(pt1A, hitpoint)
		|| cheapequalsRP3d(pt2A, hitpoint))
		return true;
	*/


	RP3d pt0, pt1, pt2;//pt0=pt0A, pt1=pt1A, pt2=pt2A;
	RP3d main_cross;
	RP3d result_cross;

	//if(cheapequalsRP3d(pt0, hitpoint) || cheapequalsRP3d(pt1, hitpoint)
	//	|| cheapequalsRP3d(pt2, hitpoint))
	//	return true;


	//rspSub(pt0, pt2);
	//rspSub(pt1, pt2);
	pt0.x=pt0A.x - pt2A.x;
	pt0.y=pt0A.y - pt2A.y;
	pt0.z=pt0A.z - pt2A.z;

	pt1.x=pt1A.x - pt2A.x;
	pt1.y=pt1A.y - pt2A.y;
	pt1.z=pt1A.z - pt2A.z;

	rspCross(pt0, pt1, main_cross);


	RP3d hit=hitpoint;


	//pt2A is the origin
	//pt0=pt0A, pt1=pt1A;
	//rspSub(pt0, pt2);
	//rspSub(hitpoint, pt2);
	//rspCross(pt0, hitpoint, result_cross); //check 0 to hitpoint 

   pt0.x=pt0A.x - pt2A.x;
   pt0.y=pt0A.y - pt2A.y;
   pt0.z=pt0A.z - pt2A.z;
	rspSub(hitpoint, pt2A);
	rspCross(pt0, hitpoint, result_cross); //check 0 to hitpoint 

	if(SameSigns_Vector(&result_cross, &main_cross))
		{
		 //pt0 is the origin
		 //pt0=pt0A;
		 //hitpoint=hit;
		 //rspSub(pt1, pt0);
		 //rspSub(hitpoint, pt0);

		 //pt0 is the origin
		 pt1.x=pt1A.x-pt0A.x;
		 pt1.y=pt1A.y-pt0A.y;
		 pt1.z=pt1A.z-pt0A.z;
		 hitpoint.x=hit.x-pt0A.x;
		 hitpoint.y=hit.y-pt0A.y;
		 hitpoint.z=hit.z-pt0A.z;
		 rspCross(pt1, hitpoint, result_cross); //check 1 to hitpoint 

		 if(SameSigns_Vector(&result_cross, &main_cross))
			{
			//pt1 is the origin
			//pt1=pt1A;
			//hitpoint=hit;
			//rspSub(pt2, pt1);
			//rspSub(hitpoint, pt1);
			//rspCross(pt2, hitpoint, result_cross); //check 2 to hitpoint 

			//pt1 is the origin
			pt2.x=pt2A.x-pt1A.x;
			pt2.y=pt2A.y-pt1A.y;
			pt2.z=pt2A.z-pt1A.z;
			hitpoint.x=hit.x-pt1A.x;
			hitpoint.y=hit.y-pt1A.y;
			hitpoint.z=hit.z-pt1A.z;
			rspCross(pt2, hitpoint, result_cross); //check 2 to hitpoint 

			if(SameSigns_Vector(&result_cross, &main_cross))
				{
				hitpoint=hit;
				return(true);//point is definitely inside the triangle!
				};//check 2
			};//check 1 
		};//check 0
	hitpoint=hit;
	return(false);
	};

////////////////////////////////////////////////////////////////////////////////
// Checks if a line segment and a triangle intersect
////////////////////////////////////////////////////////////////////////////////
bool							// Returns true if the two shapes intersect.
TriAIntersectsLineSegmentB(
	RP3d & normalA,		// In: normal vector of triangle A
	float fBigA,				// In: constant from parametric equation of A
	RP3d & pt0A,			// In: three points of triangle A
	RP3d & pt1A, 
	RP3d & pt2A, 
	RP3d & pt0B,			// In: two points of line segment B
	RP3d & pt1B,
	RP3d & hitpoint)		// Out: Exact point where line hits triangle
	{
	float intersect_t;
	// point 0--1
	RP3d ptSub;
	ptSub.x=pt1B.x-pt0B.x;
	ptSub.y=pt1B.y-pt0B.y;
	ptSub.z=pt1B.z-pt0B.z;

	if(GetIntersectSubT(pt0B, ptSub, normalA, fBigA, intersect_t))
		{
		if(intersect_t>=0 && 
			intersect_t<=1)
			{
			//GetIntersectPointSub(pt0B, ptSub, intersect_t, hitpoint);
			 hitpoint.x=pt0B.x+ptSub.x*intersect_t;
			 hitpoint.y=pt0B.y+ptSub.y*intersect_t;
			 hitpoint.z=pt0B.z+ptSub.z*intersect_t;

			if(PointInsideTri(pt0A, pt1A, pt2A, hitpoint))
				return(true);								 
			};
		};

	//hitpoint.x=hitpoint.y=hitpoint.z=0; 
	return(false);
	};

////////////////////////////////////////////////////////////////////////////////
bool shorterdist(RP3d &pt1, RP3d &pt2)
// true if pt1 is shorter dist than pt2
	{
	return ((pt1.x*pt1.x + pt1.y*pt1.y + pt1.z*pt1.z) < (pt2.x*pt2.x + pt2.y*pt2.y + pt2.z*pt2.z));
	};
void sqrdist(RP3d &pt1, float &fdist)
// returns sqrd dist of point
	{
	fdist=(pt1.x*pt1.x + pt1.y*pt1.y + pt1.z*pt1.z);
	};
////////////////////////////////////////////////////////////////////////////////
bool 
TrianglesIntersectLineSegment(
	RP3d &linept1,				// In: line segment point 1
	RP3d &linept2, 			// In: line segment point 2 closest to this point. this should be the first point.
	U16 *ptri, 					// In: mesh
	RP3d *soparr, 				// In: points for mesh
	short smeshNum,			// In: number of points in mesh
	RP3d &hitpoint,			// Out: point where line hit triangle
	long &lHitTriIndex)		// Out: index of triangle it hit

	{    
	lHitTriIndex=-1;
	short sJ=0;
	RP3d normalA;
	RP3d pt0A, pt1A, pt2A;
	RP3d t_pt1A, t_pt2A;
	float fBigA=-1;
	RP3d ptclosest, ptwork;
	float fclosest=(float)INT_MAX, fdist;
	bool bhit=false;

	hitpoint.x=hitpoint.y=hitpoint.z=0;
	hitpoint.w=1;
	normalA.w=1;

	for(sJ=0; sJ<smeshNum; sJ++)
		{
		//assign points
		pt0A=soparr[*ptri++];
		pt1A=soparr[*ptri++];
		pt2A=soparr[*ptri++];

		//get the normals of triangle 1
		t_pt1A.x=pt1A.x-pt0A.x;
		t_pt1A.y=pt1A.y-pt0A.y;
		t_pt1A.z=pt1A.z-pt0A.z;
		t_pt2A.x=pt2A.x-pt0A.x;
		t_pt2A.y=pt2A.y-pt0A.y;
		t_pt2A.z=pt2A.z-pt0A.z;
		rspCross(t_pt1A, t_pt2A, normalA);

		//and the constant
		fBigA=(-normalA.x*pt0A.x - normalA.y*pt0A.y - normalA.z*pt0A.z);

		// Linesegment B intersects polygon A -------------------------------------------------------------------
		if(TriAIntersectsLineSegmentB(normalA, fBigA, pt0A, pt1A, pt2A, linept1, linept2, hitpoint))
			{ 
			rspCopy(ptwork, linept2);
			rspSub(ptwork, hitpoint);
			sqrdist(ptwork, fdist);
			if(fdist < fclosest)
				{
				fclosest=fdist;
				lHitTriIndex=sJ;
				rspCopy(ptclosest, hitpoint);
				bhit=true;
				};
			//return(true);
			};
		};//sJ for-loop

	if(bhit)
		{
		rspCopy(hitpoint, ptclosest);
		};
	return(bhit);
	};


////////////////////////////////////////////////////////////////////////////////
// Transform psopSrc into psopDst with the supplied transform in addition to
// the screen/view pipeline.
////////////////////////////////////////////////////////////////////////////////
void Transform(RSop* psopSrc, RSop* psopDst, RPipeLine* ppipe, RTransform& tObj)
	{
	RTransform tFull;
	long i;
	// Use to stretch to z-buffer!

	tFull.Make1();
	tFull.Mul(ppipe->m_tView.T,tObj.T);
	// If there were inhomogeneous transforms, you would need to 
	// trasnform each pt by two transforms separately!
	tFull.PreMulBy(ppipe->m_tScreen.T);
	// Add this in to get the model in the correctly offset spot.
	const short sMisUnderstoodValueX	= -85;	// *** WTF is this not sOffsetX?  No time for that now.
	const short sMisUnderstoodValueY	= -20;	// *** WTF is this not sOffsetY?  No time for that now.

#if 0	// Find the magic offset.
	static short sOffX = 0;
	static short sOffY = 0;
	static U8*	pau8KeyStatus = rspGetKeyStatusArray();
	if (pau8KeyStatus[RSP_SK_LEFT] & 1)
		sOffX--;
	if (pau8KeyStatus[RSP_SK_RIGHT] & 1)
		sOffX++;
	if (pau8KeyStatus[RSP_SK_UP] & 1)
		sOffY--;
	if (pau8KeyStatus[RSP_SK_DOWN] & 1)
		sOffY++;
	tFull.Trans(sMisUnderstoodValueX + sOffX, sMisUnderstoodValueY + sOffY, 0);
#else
	tFull.Trans(sMisUnderstoodValueX, sMisUnderstoodValueY, 0);
#endif


	for (i = 0; i < psopSrc->m_lNum; i++)
		{
		tFull.TransformInto(psopSrc->m_pArray[i], psopDst->m_pArray[i]);
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Create a 3D palette in the specified GUI.
////////////////////////////////////////////////////////////////////////////////
void
CreatePalette(
	RGuiItem* pgui)		// In:  GUI to contain palette.
	{
	short sCells	= c_sPalNum / c_sPalColorsPerSwatch;
	short	sCols		= c_sPalCols;
	short	sRows		= sCells / sCols;

	RRect	rcClient;
	pgui->GetClient(&rcClient.sX, &rcClient.sY, &rcClient.sW, &rcClient.sH);

	short sCellW	= rcClient.sW / sCols;
	short	sCellH	= rcClient.sH / sRows;

	short	sRow, sCol;
	short	sX, sY;
	short sColor = g_sPalFirstColor;
	for (sRow = 0, sY = rcClient.sY; sRow < sRows; sRow++, sY += sCellH)
		{
		for (sCol = 0, sX = rcClient.sX; sCol < sCols; sCol++, sX += sCellW)
			{
			rspRect(
				sColor,
				&pgui->m_im,
				sX, sY,
				sCellW, sCellH,
				NULL);
			
			sColor += c_sPalColorsPerSwatch;
			}
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Set the specified GUI's text and recompose it to reflect the change.
////////////////////////////////////////////////////////////////////////////////
void
SetText(
	RGuiItem* pguiRoot,		// In:  Root GUI.
	long	lId,					// In:  ID of GUI to update.
	const char* pszFrmt,		// In:  Format specifier ala sprintf.
	...)							// In:  Arguments as specified by format.
	{
	RGuiItem*	pgui = pguiRoot->GetItemFromId(lId);
	if (pgui)
		{
		va_list val;
		va_start(val, pszFrmt);    
		  
		vsprintf(pgui->m_szText, pszFrmt, val);

		// Recompose with new text.
		pgui->Compose();
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Some texture files were out of date when Special Delivery was made.  As a 
// result, it's _possible_ (however unlikely) that a crash could occur when
// reading the ends of the gramps textures.
//
// It appears the problem is that a cane was added to the gramps' meshes after 
// the extra various textures were exported but the extra textures were never
// re-exported so they don't contain enough entries for the cane.
////////////////////////////////////////////////////////////////////////////////
void
ValidateTextures(
	RTexture*	ptex,	// In:  Texture to validate.
	short			sNum)	// In:  Number of textures it should have.
	{
	if (ptex->m_sNum < sNum)
		{
		short	sRes = rspMsgBox(
			RSP_MB_ICN_QUERY | RSP_MB_BUT_YESNO,
			"Incorrect Texture File",
			"This texture file does not have enough entries to cover the entire mesh.\n"
			"Do you want this utility to recreate the texture scheme filling in the\n"
			"empty entries with bright green?");
		switch (sRes)
			{
			case RSP_MB_RET_YES:
				{
				short	sOrigNum	= ptex->m_sNum;

				// Create temp space for the existing colors.
				U8*	pau8	= new U8[sOrigNum];
				
				// Duplicate the existing colors.
				short sColor;
				for (sColor = 0; sColor < sOrigNum; sColor++)
					{
					pau8[sColor]	= ptex->m_pIndices[sColor];
					}

				// Free the existing colors.
				ptex->FreeIndices();

				// Resize.
				ptex->m_sNum	= sNum;
				ptex->AllocIndices();

				// Copy the original colors back.
				for (sColor = 0; sColor < sOrigNum; sColor++)
					{
					ptex->m_pIndices[sColor]	= pau8[sColor];
					}

				// Fill the remaining colors as bright green.
				for ( ; sColor < sNum; sColor++)
					{
					ptex->m_pIndices[sColor]	= 250;	// Part of static Postal palette.
					}

				delete pau8;
				pau8	= 0;

				break;
				}
			case RSP_MB_RET_NO:
				break;
			}
		}
	}


//------------------------------------------------------------------------------
// Construction.
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
// Default constructor.
////////////////////////////////////////////////////////////////////////////////
CTexEdit::CTexEdit(void)
	{
	m_pguiRoot			= NULL;
	m_pguiAnim			= NULL;
	m_pguiCurColor		= NULL;
	m_pguiPal			= NULL;

	m_scene.SetupPipeline(NULL, NULL, c_dScale);

	m_manip	= Trans;

	m_bDragging = false;

	m_sCursorResetX = 0;
	m_sCursorResetY = 0;

	ResetTransformation();

	m_bQuit	= false;

	m_u8Color	= g_sPalFirstColor;

	m_lTriIndex	= -1;

	m_ptexSrc		= NULL;
	m_ptexchanSrc	= NULL;

	m_bModified	= false;

	m_bSpotLight	= false;
	m_sBrightness	= 0;
	}

////////////////////////////////////////////////////////////////////////////////
// Destructor.
////////////////////////////////////////////////////////////////////////////////
CTexEdit::~CTexEdit(void)
	{
	}

//------------------------------------------------------------------------------
// Methods.
//------------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////
// Edit the texture of the specified 3D animation in a modal dialog.
//////////////////////////////////////////////////////////////////////////////
void
CTexEdit::DoModal(
	CAnim3D* panim,			// In:  3D animation to paint on.
	RAlpha* pltAmbient,		// In:  Ambient light.
	RAlpha* pltSpot,			// In:  Spot light.
	const RString& strFile)	// In:  Filename to save modified texture as.
	{
	m_pguiRoot = RGuiItem::LoadInstantiate(FullPathVD(c_strGuiFile) );
	if (m_pguiRoot)
		{
		RProcessGui gm;

		// Get composite buffer.
		RImage* pimComposite;
		rspNameBuffers(&pimComposite);

		// Remember center.
		m_sCursorResetX	= pimComposite->m_sWidth / 2;
		m_sCursorResetY	= pimComposite->m_sHeight / 2;

		// Center.
		m_pguiRoot->Move(
			pimComposite->m_sWidth / 2 - m_pguiRoot->m_im.m_sWidth / 2,
			pimComposite->m_sHeight / 2 - m_pguiRoot->m_im.m_sHeight / 2);

		// Get the 3D render surface.
		m_pguiAnim = m_pguiRoot->GetItemFromId(c_lIdAnim);
		ASSERT(m_pguiAnim);

		// Get the palette surface.
		m_pguiPal = m_pguiRoot->GetItemFromId(c_lIdPal);
		ASSERT(m_pguiPal);

		CreatePalette(m_pguiPal);

		m_pguiCurColor = m_pguiRoot->GetItemFromId(c_lIdCurColor);
		ASSERT(m_pguiCurColor);

		SetColor(g_sPalFirstColor);

		m_bSpotLight	= false;

		// Get light to use.
		RRect rcClient;
		m_pguiAnim->GetClient(&rcClient.sX, &rcClient.sY, &rcClient.sW, &rcClient.sH);

		short sOffsetX = rcClient.sX + rcClient.sW / 2;
		short sOffsetY = rcClient.sY + rcClient.sH - 5;

		ResetTransformation();

		SetManip(Paint);

		// Even though there's many channels of many textures per person, they are all the
		// very same resource (on disk & in memory).
		m_ptexchanSrc	= panim->m_ptextures;
		m_ptexSrc		= m_ptexchanSrc->GetAtTime(0);

		// Validate texture thinger.
		ValidateTextures(m_ptexSrc, panim->m_pmeshes->GetAtTime(0)->m_sNum);


		// Duplicate into a care-free work area.
		m_texWork	= *m_ptexSrc;

		m_bModified	= false;

		m_strFileName	= strFile;

		// Set notifications.
		SetToNotify(c_lIdQuit,			QuitCall_Static);
		SetToNotify(c_lIdTrans,			ManipCall_Static);
		SetToNotify(c_lIdScale,			ManipCall_Static);
		SetToNotify(c_lIdRotate,		ManipCall_Static);
		SetToNotify(c_lIdPaint,			ManipCall_Static);
		SetToNotify(c_lIdPal,			ColorCall_Static);
		SetToNotify(c_lIdApply,			ApplyCall_Static);
		SetToNotify(c_lIdSave,			SaveCall_Static);
		SetToNotify(c_lIdRevert,		RevertCall_Static);
		SetToNotify(c_lIdSpotLight,	SpotCall_Static);
		SetToNotify(c_lIdBrightness,	BrightnessCall_Static);
		SetToNotify(c_lIdAdjust,		AdjustCall_Static);
		SetToNotify(c_lIdAnim,			AnimCall_Static);

		// Set initial brightness.
		RScrollBar*	psb	= (RScrollBar*)m_pguiRoot->GetItemFromId(c_lIdBrightness);
		ASSERT(psb);
		ASSERT(psb->m_type == RGuiItem::ScrollBar);
		psb->SetPos(50);

		SetText(m_pguiRoot, c_lIdFrequency, "%hd", c_sDefAdjustFrequency);
		SetText(m_pguiRoot, c_lIdAmount, "%g", c_fDefAdjustment);

		// Get up to two controls that can end the processing that can be
		// passed on the DoModal() line.  More buttons can be set though.
		// Set up ptrs and erase buffer.
		gm.Prepare(m_pguiRoot, NULL, NULL);

		RInputEvent	ie;

		m_bQuit	= false;
	
		m_lTriIndex	= -1;

		RTransform	trans;
		CSprite3		sprite;
		RSop			sopView;
		long			lTime = 0;

		// Process GUI.
		while (m_bQuit == false)
			{
			// System Update //////////////////////////////////////
			ie.type	= RInputEvent::None;
		
			// Critical callage.
			UpdateSystem();
			rspGetNextInputEvent(&ie);

			// Setup current 3D info ///////////////////////////////
			sprite.m_pmesh			= panim->m_pmeshes->GetAtTime(lTime);
			sprite.m_psop			= panim->m_psops->GetAtTime(lTime);
			sprite.m_ptrans		= &trans;
			sprite.m_ptex			= &m_texWork;
			sprite.m_psphere		= panim->m_pbounds->GetAtTime(lTime);
			sprite.m_sBrightness	= m_sBrightness;

			// Transformation //////////////////////////////////////
			trans.Make1();
			ComposeTransform(trans);

			// View Space SOP //////////////////////////////////////

			// Create View Space SOP for checking mouse against triangles and
			// drawing feedback for such.
			if (sopView.m_lNum != sprite.m_psop->m_lNum) 
				sopView.Alloc(sprite.m_psop->m_lNum);

			Transform(sprite.m_psop, &sopView, &m_scene.m_pipeline, trans);

			// Process User Manips /////////////////////////////////

			ProcessManip(m_pguiAnim->m_sPressed != 0, &ie, &sprite, &sopView);

			// Output //////////////////////////////////////////////
			rspRect(
				0,
				&m_pguiAnim->m_im,
				0, 0,
				m_pguiAnim->m_im.m_sWidth,
				m_pguiAnim->m_im.m_sHeight,
				&rcClient);

			DoOutput(&sprite, &sopView, trans, m_bSpotLight ? pltSpot : pltAmbient, &m_pguiAnim->m_im, sOffsetX, sOffsetY, rcClient);

			gm.DoModeless(m_pguiRoot, &ie, pimComposite);

			// If quiting . . .
			if (m_bQuit)
				{
				// If modified . . .
				if (m_bModified)
					{
					// Query if user wants to apply the work textures (and not lose changes).
					short	sRes	= rspMsgBox(
						RSP_MB_ICN_QUERY | RSP_MB_BUT_YESNOCANCEL,
						g_pszAppName,
						"Apply changes before exiting texture editor?");

					switch (sRes)
						{
						case RSP_MB_RET_YES:		// Yes - apply.
							Apply();
							break;
						case RSP_MB_RET_NO:		// No - don't apply.
							break;
						case RSP_MB_RET_CANCEL:	// Cancel - don't quit.
							m_bQuit	= false;
							rspSetQuitStatus(FALSE);
							break;
						}
					}
				}
			}

		// Clean up ptrs, erase buffer, and dirty rect list.
		gm.Unprepare();

		m_ptexSrc		= NULL;
		m_ptexchanSrc	= NULL;
		m_texWork.FreeIndices();

		delete m_pguiRoot;
		m_pguiRoot = NULL;
		m_pguiAnim = NULL;
		m_pguiCurColor = NULL;
		m_pguiPal = NULL;
		}
	else
		{
		rspMsgBox(
			RSP_MB_ICN_STOP | RSP_MB_BUT_OK,
			g_pszAppName,
			g_pszFileOpenError_s,
			FullPathVD(c_strGuiFile) );
		}
	}

//////////////////////////////////////////////////////////////////////////////
// Render the 3D animation at the specified time.
//////////////////////////////////////////////////////////////////////////////
void
CTexEdit::DoOutput(
	CSprite3* psprite,	// In:  3D data to render.
	RSop* psopView,		// In:  View space SOP.
	RTransform& trans,	// In:  Transformation.
	RAlpha* palphaLight,	// In:  Light.
	RImage* pimDst,		// In:  Destination for result.
	short sOffsetX,		// In:  X offset.
	short sOffsetY,		// In:  Y offset.
	RRect& rcClip)			// In:  Dst clip rect.
	{
	m_scene.Render3D(
		pimDst,			// Destination image.     
		sOffsetX,		// Destination 2D x coord.
		sOffsetY,		// Destination 2D y coord.
		psprite,			// 3D sprite to render.   
		palphaLight,	// Light to render with.  
		&rcClip);		// Dst clip rect.         

#if 0	// Draw wire frame.
	RMesh*	pmesh	= psprite->m_pmesh;
	short sTris = pmesh->m_sNum;
	U16* pu16Vertex	= pmesh->m_pArray;
	while (sTris--)
		{
		const RP3d&	v1	= psopView->m_pArray[*pu16Vertex++];
		const RP3d&	v2	= psopView->m_pArray[*pu16Vertex++];
		const RP3d&	v3	= psopView->m_pArray[*pu16Vertex++];
		rspLine(255, pimDst, v1.x, v1.y, v2.x, v2.y, &rcClip);
		rspLine(255, pimDst, v2.x, v2.y, v3.x, v3.y, &rcClip);
		rspLine(255, pimDst, v3.x, v3.y, v1.x, v1.y, &rcClip);
		}
#endif
	
	// If cursor shown . . .
	if (m_manip == Paint)
		{
		if (m_lTriIndex >= 0)
			{
			RMesh*	pmesh	= psprite->m_pmesh;
			ASSERT(m_lTriIndex < pmesh->m_sNum);

			if (m_lTriIndex < pmesh->m_sNum)
				{
				long	lVertexIndex	= m_lTriIndex * 3;

				const RP3d&	v1	= psopView->m_pArray[pmesh->m_pArray[lVertexIndex++] ];
				const RP3d&	v2	= psopView->m_pArray[pmesh->m_pArray[lVertexIndex++] ];
				const RP3d&	v3	= psopView->m_pArray[pmesh->m_pArray[lVertexIndex++] ];
				rspLine(255, pimDst, v1.x, v1.y, v2.x, v2.y, &rcClip);
				rspLine(255, pimDst, v2.x, v2.y, v3.x, v3.y, &rcClip);
				rspLine(255, pimDst, v3.x, v3.y, v1.x, v1.y, &rcClip);
				}
			}
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Processes drags and keys into transform stuff.
////////////////////////////////////////////////////////////////////////////////
void
CTexEdit::ProcessManip(
	bool bButtonDown,		// In:  true if mouse button down.
	RInputEvent* pie,		// In:  Current input event.
	CSprite3* psprite,	// In:  3D data to process.
	RSop* psopView)		// In:  View space SOP.
	{
	// Transformation Manipulation Processing /////////////////
	if (bButtonDown && m_manip != Paint)
		{
		// If first time . . .
		if (m_bDragging == false)
			{
			rspSetMouse(m_sCursorResetX, m_sCursorResetY);
			rspHideMouseCursor();
			m_bDragging = true;
			}

		short sCursorX, sCursorY;
		rspGetMouse(&sCursorX, &sCursorY, NULL);
		rspSetMouse(m_sCursorResetX, m_sCursorResetY);

		short sDeltaX = sCursorX - m_sCursorResetX;
		short sDeltaY = m_sCursorResetY - sCursorY;

		static U8*	pau8KeyStatus = rspGetKeyStatusArray();
		
		if (pau8KeyStatus[RSP_SK_SHIFT] & 1)
			sDeltaY = 0;
		if (pau8KeyStatus[RSP_SK_CONTROL] & 1)
			sDeltaX = 0;

		switch (m_manip)
			{
			case Trans:
				m_fX += sDeltaX * c_fTransRate;
				m_fY += sDeltaY * c_fTransRate;
				break;
			case Scale:
				if (sDeltaY)
					{
					m_fScale	+= sDeltaY * c_fScaleRate;

					if (m_fScale < 0.1f)
						m_fScale = 0.1f;
					if (m_fScale > 1.0f)
						m_fScale = 1.0f;
					}
				break;
			case Rot:
				m_transRot.Ry(rspMod360(sDeltaX * c_fRotRate) );
				m_transRot.Rx(rspMod360(-sDeltaY * c_fRotRate) );
				break;
			}
		}
	else
		{
		if (m_bDragging == true)
			{
			rspShowMouseCursor();
			m_bDragging	= false;
			}
		}

	// Paint Processing /////////////////////////////////
	if (m_manip == Paint)
		{
		RP3d	hitpoint;
		RP3d	linept1, linept2;

		short sMouseX, sMouseY;
		rspGetMouse(&sMouseX, &sMouseY, NULL);
		m_pguiAnim->TopPosToChild(&sMouseX, &sMouseY);

		linept1.x	= sMouseX;
		linept1.y	= sMouseY;
		linept1.z	= -SHRT_MAX;
		linept2.x	= sMouseX;
		linept2.y	= sMouseY;
		linept2.z	= SHRT_MAX;

		long	lTriIndex;
		bool bHit = TrianglesIntersectLineSegment(
			linept1,								// In: line segment point 1
			linept2, 							// In: line segment point 2 closest to this point. this should be the first point.
			psprite->m_pmesh->m_pArray,	// In: mesh
			psopView->m_pArray, 				// In: points for mesh
			psprite->m_pmesh->m_sNum,		// In: number of points in mesh
			hitpoint,							// Out: point where line hit triangle
			lTriIndex);							// Out: index of triangle it hit

		if (bHit)
			{
			m_lTriIndex	= lTriIndex;

			SetStatusText("Triangle %ld", m_lTriIndex);

			if (bButtonDown)
				{
				RTexture*	ptex	= psprite->m_ptex;
				// Get into texture and replace current triangle index with our current color.
				if (ptex->m_pIndices)
					{
					ASSERT(m_lTriIndex < ptex->m_sNum);

					// Set new color for this texture.
					ptex->m_pIndices[m_lTriIndex]	= m_u8Color;

					// Note modification.
					m_bModified	= true;
					}
				}
			}
		else
			{
			// If last time we found a triangle . . .
			if (m_lTriIndex >= 0)
				{
				// Clear status text.
				SetStatusText("");
				}
			else
				{
				// Otherwise, text does not refer to a triangle.  So leave it.
				}
			
			m_lTriIndex = -1;
			}
		}

	// Color Palette Processing ///////////////////
	if (m_pguiPal->m_sPressed)
		{
		short sMouseX, sMouseY;
		rspGetMouse(&sMouseX, &sMouseY, NULL);
		m_pguiPal->TopPosToChild(&sMouseX, &sMouseY);

		// Get color directly out of GUI.
		U8	u8Color	= *(m_pguiPal->m_im.m_pData + (sMouseY * m_pguiPal->m_im.m_lPitch) + sMouseX);
		SetColor(u8Color);
		// Go into paint mode when a color is chosen.  Feels right somehow.
		SetManip(Paint);
		}

	// Input processing ///////////////////////////
	if (pie->sUsed == FALSE)
		{
		if (pie->type == RInputEvent::Key)
			{
			switch (pie->lKey)
				{
				case 'T':
				case 't':
					SetManip(Trans);
					break;
				case 'S':
				case 's':
					SetManip(Scale);
					break;
				case 'R':
				case 'r':
					SetManip(Rot);
					break;
				case 'P':
				case 'p':
				case 'D':
				case 'd':
					SetManip(Paint);
					break;
				
				case 'I':
				case 'i':
					ResetTransformation();
					break;

				case 27:
					m_bQuit = true;
					break;

				case '\r':
					m_transRot.Make1();
					break;
				}
			}
		}

	if (rspGetQuitStatus() != FALSE)
		{
		m_bQuit	= true;
		}
	}

//------------------------------------------------------------------------------
// Querries.
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Internal Functions.
//------------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////
// Apply work colors to real thing.
//////////////////////////////////////////////////////////////////////////////
void
CTexEdit::Apply(void)
	{
	if (m_ptexSrc)
		{
		*m_ptexSrc	= m_texWork;
		m_bModified	= false;
		SetStatusText("Applied.");
		}
	}

//////////////////////////////////////////////////////////////////////////////
// Revert work colors to real thing.
//////////////////////////////////////////////////////////////////////////////
void
CTexEdit::Revert(void)
	{
	if (m_ptexSrc)
		{
		m_texWork	= *m_ptexSrc;
		m_bModified	= false;

		SetStatusText("Reverted.");
		}
	}


//////////////////////////////////////////////////////////////////////////////
// Save work colors to real thing.
//////////////////////////////////////////////////////////////////////////////
void
CTexEdit::Save(void)
	{
	Apply();

	if (m_ptexchanSrc)
		{
		if (rspEZSave(m_ptexchanSrc, m_strFileName) == 0)
			{
			SetStatusText("Applied; Saved \"%s\".", (const char*)m_strFileName);
			}
		else
			{
			SetStatusText("Applied; Failed to save \"%s\".", (const char*)m_strFileName);
			}
		}
	}

//////////////////////////////////////////////////////////////////////////////
// Set the current manipulation type.
//////////////////////////////////////////////////////////////////////////////
void
CTexEdit::SetManip(
	Manip manip)	// In:  New manipulation type.
	{
	if (m_pguiRoot)
		{
		SetPushBtnState(m_pguiRoot, m_manip + c_lIdTrans, RPushBtn::Off);

		m_manip	= manip;

		SetPushBtnState(m_pguiRoot, m_manip + c_lIdTrans, RPushBtn::On);
		}
	}

//////////////////////////////////////////////////////////////////////////////
// Set the current palette color.
//////////////////////////////////////////////////////////////////////////////
void
CTexEdit::SetColor(
	U8	u8Color)		// In:  New color index.
	{
	// Make sure it's in range . . .
	if (u8Color >= c_sPalStart && u8Color <= c_sPalEnd)
		{
		m_u8Color	= u8Color;

		if (m_pguiCurColor)
			{
			// Show current color in this item.
			short	sX, sY, sW, sH;
			m_pguiCurColor->GetClient(&sX, &sY, &sW, &sH);
			rspRect(
				u8Color,
				&m_pguiCurColor->m_im,
				sX, sY,
				sW, sH,
				NULL);
			}
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Set the specified button to notify.
////////////////////////////////////////////////////////////////////////////////
void
CTexEdit::SetToNotify(
	long lBtnId,					// In:  ID of btn whose callback will be set.
	RGuiItem::BtnUpCall pfn)	// In:  Function to notify.
	{
	ASSERT(m_pguiRoot);
	RGuiItem*	pgui	= m_pguiRoot->GetItemFromId(lBtnId);
	if (pgui)
		{
		pgui->m_bcUser				= pfn;
		pgui->m_ulUserInstance	= (ULONG)this;
		}
	}

//////////////////////////////////////////////////////////////////////////////
// Set the specified gui to notify.
//////////////////////////////////////////////////////////////////////////////
void
CTexEdit::SetToNotify(
	long lId,							// In:  ID of gui whose callback will be set.
	RGuiItem::InputEventCall pfn)	// In:  Function to notify.
	{
	ASSERT(m_pguiRoot);
	RGuiItem*	pgui	= m_pguiRoot->GetItemFromId(lId);
	if (pgui)
		{
		pgui->m_fnInputEvent		= pfn;
		pgui->m_ulUserInstance	= (ULONG)this;
		}
	}

//////////////////////////////////////////////////////////////////////////////
// Set the specified scrollbar to notify.
//////////////////////////////////////////////////////////////////////////////
void
CTexEdit::SetToNotify(
	long lId,								// In:  ID of scrollbar whose callback will be set.
	RScrollBar::UpdatePosCall pfn)	// In:  Function to notify.
	{
	ASSERT(m_pguiRoot);
	RScrollBar*	psb	= (RScrollBar*)m_pguiRoot->GetItemFromId(lId);
	if (psb)
		{
		if (psb->m_type == RGuiItem::ScrollBar)
			{
			psb->m_upcUser			= pfn;
			psb->m_ulUserInstance	= (ULONG)this;
			}
		}
	}

//////////////////////////////////////////////////////////////////////////////
// Set the status text field to that specified.
//////////////////////////////////////////////////////////////////////////////
void
CTexEdit::SetStatusText(
	const char* pszFrmt,		// In:  Format specifier ala sprintf.
	...)							// In:  Arguments as specified by format.
	{
	RGuiItem*	pguiStatus = m_pguiRoot->GetItemFromId(c_lIdStatus);
	if (pguiStatus)
		{
		va_list val;
		va_start(val, pszFrmt);    
		  
		vsprintf(pguiStatus->m_szText, pszFrmt, val);

		// Recompose with new text.
		pguiStatus->Compose();
		}
	}

//////////////////////////////////////////////////////////////////////////////
// Compose tarnsformations in the specified transform.  Init transform before
// calling as necessary.
//////////////////////////////////////////////////////////////////////////////
void
CTexEdit::ComposeTransform(
	RTransform& trans)		// In:  Transform to compose in.
	{
	trans	= m_transRot;
	trans.Scale(m_fScale, m_fScale, m_fScale);
	trans.Trans(m_fX, m_fY, 0.0f);
	}

//------------------------------------------------------------------------------
// Callbacks.
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
// Called when user presses Quit.
////////////////////////////////////////////////////////////////////////////////
void
CTexEdit::QuitCall(RGuiItem* pgui)
	{
	m_bQuit	= true;
	}

////////////////////////////////////////////////////////////////////////////////
// Called when user presses a manipulation mode button.
////////////////////////////////////////////////////////////////////////////////
void
CTexEdit::ManipCall(RGuiItem* pgui)
	{
	SetManip(Manip(pgui->m_lId - c_lIdTrans) );
	}

////////////////////////////////////////////////////////////////////////////////
// Called when user clicks in the color palette
////////////////////////////////////////////////////////////////////////////////
void
CTexEdit::ColorCall(RGuiItem* pgui, RInputEvent* pie)
	{
	if (pie->type == RInputEvent::Mouse)
		{
		switch (pie->sEvent)
			{
			case RSP_MB0_PRESSED:
				{
#if 0	// This is now done in ProcessManip().
				// Get color directly out of GUI.
				U8	u8Color	= *(pgui->m_im.m_pData + (pie->sPosY * pgui->m_im.m_lPitch) + pie->sPosX);
				SetColor(u8Color);
				// Go into paint mode when a color is chosen.  Feels right somehow.
				SetManip(Paint);
#endif
				break;
				}
			}
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Called when user presses Apply.
////////////////////////////////////////////////////////////////////////////////
void
CTexEdit::ApplyCall(RGuiItem* pgui)
	{
	Apply();
	}

////////////////////////////////////////////////////////////////////////////////
// Called when user presses Save.
////////////////////////////////////////////////////////////////////////////////
void
CTexEdit::SaveCall(RGuiItem* pgui)
	{
	Save();
	}

////////////////////////////////////////////////////////////////////////////////
// Called when user presses Revert.
////////////////////////////////////////////////////////////////////////////////
void
CTexEdit::RevertCall(RGuiItem* pgui)
	{
	Revert();
	}

////////////////////////////////////////////////////////////////////////////////
// Called when user presses Spot light button.
////////////////////////////////////////////////////////////////////////////////
void
CTexEdit::SpotCall(RGuiItem* pgui)
	{
	ASSERT(pgui->m_type == RGuiItem::PushBtn);
	RPushBtn*	pbtn	= (RPushBtn*)pgui;
	m_bSpotLight	= (pbtn->m_state == RPushBtn::On) ? true : false;
	}

////////////////////////////////////////////////////////////////////////////////
// Called when user adjust Brightness scrollbar.
////////////////////////////////////////////////////////////////////////////////
void
CTexEdit::BrightnessCall(RScrollBar* psb)
	{
	long	lMin, lMax;
	psb->GetRange(&lMin, &lMax);
	float	fRange	= lMax - lMin + 1;

	if (fRange != 0.0f)
		m_sBrightness	= c_sMinBrightness + (psb->GetPos() / fRange) * c_sBrightnessRange;
	}


////////////////////////////////////////////////////////////////////////////////
// Called when user presses Adjust button.
////////////////////////////////////////////////////////////////////////////////
void
CTexEdit::AdjustCall(RGuiItem* pgui)
	{
	// Get frequency.
	long	lFreq	= m_pguiRoot->GetVal(c_lIdFrequency);
	
	char	szText[GUI_MAX_STR];
	m_pguiRoot->GetText(c_lIdAmount, szText, sizeof(szText) );

	float	fAdjust	= strtod(szText, NULL);

	// Get palette to work with.
	U8	au8Red[256];
	U8	au8Green[256];
	U8	au8Blue[256];
	rspGetPaletteEntries(
		0,					// Palette entry to start copying from                           
		256,				// Number of palette entries to do                               
		au8Red,			// Pointer to first red component to copy to                     
		au8Green,		// Pointer to first green component to copy to                   
		au8Blue,			// Pointer to first blue component to copy to                    
		sizeof(U8) );	// Number of bytes by which to increment pointers after each copy


	// Unmap colors from palette into full color values.
	m_texWork.Unmap(
		au8Red,
		au8Green,
		au8Blue,
		sizeof(U8) );

	// Adjust colors.
	m_texWork.Adjust(
		fAdjust,
		lFreq);
	
	// Remap onto palette.
	m_texWork.Remap(
		c_sPalStart,
		c_sPalNum,
		au8Red,
		au8Green,
		au8Blue,
		sizeof(U8) );

	// Get rid of full colors.
	m_texWork.FreeColors();
	}

////////////////////////////////////////////////////////////////////////////////
// Called when user clicks in the anim surface.
////////////////////////////////////////////////////////////////////////////////
void
CTexEdit::AnimCall(RGuiItem* pgui, RInputEvent* pie)
	{
	if (pie->type == RInputEvent::Mouse)
		{
		switch (pie->sEvent)
			{
			case RSP_MB1_PRESSED:
				// Do NOT Get color directly out of GUI.  The color there has already
				// undergone lighting effects.  Get the color from the actual texture.
				if (m_lTriIndex)
					{
					U8	u8Color	= m_texWork.m_pIndices[m_lTriIndex];
					SetColor(u8Color);
					}
				break;
			}
		}
	}

//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------