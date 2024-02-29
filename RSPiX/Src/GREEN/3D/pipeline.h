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
///////////////////////////////////////////////////////////////
//	PIPELINE - History
///////////////////////////////////////////////////////////////
//
//	07/23/97	JRD	Added support for generating shadows.  Currently
//						all shadows are hard coded to be cast upon the
//						plane y = 0, based on postal needs.
//
///////////////////////////////////////////////////////////////


// This is the highest level considered actually part of the 3d engine.
// It is the highest level control -> it decides how 3d pts map to 2d.
// You can customize 3d efects by instantiating your own versions of the 3d pipeline!
// 
#ifndef PIPELINE_H
#define PIPELINE_H
//================================================== 
#include "System.h"
#ifdef PATHS_IN_INCLUDES
	#include "ORANGE/QuickMath/VectorMath.h"
	#include "GREEN/3D/types3d.h"
	#include "GREEN/3D/zbuffer.h"
	#include "GREEN/3D/render.h"
	#include "ORANGE/color/colormatch.h" 
#else
	#include "vectormath.h"
	#include "types3d.h"
	#include "zbuffer.h"
	#include "render.h"
	#include "ColorMatch.h" 
#endif
//================================================== 

// The point of this class is to hold configurable
// scratch space for doing trsnaformations
//
class RPipeLine
	{
public:
	//-------------------------------------
	RPipeLine();
	~RPipeLine();
	short Create(long lScratchSpace=0,short sZBufWidth=0);
	short CreateShadow(short sAngleY,double dTanDeclension,short sBufSize = -1);
	void Destroy(); // will NOT kill transform scratch space
	void Init();
	//-------------------------------------
	short NotCulled(RP3d *p1,RP3d *p2,RP3d *p3);
	void Transform(RSop* pPts,RTransform& tObj);
	void TransformShadow(RSop* pPts,RTransform& tObj,
		short sHeight = 0,short *psOffX = NULL,short *psOffY = NULL);

	// Do NOT use a z-buffer.  Return offset to current position to
	// draw the image m_pimShadowBuf
	void	RenderShadow(RImage* pimDst,RMesh* pMesh,UCHAR ucColor); // Unicolored!

	void Render(RImage* pimDst,short sDstX,short sDstY,
		RMesh* pMesh,UCHAR ucColor); // wire frame!

	// Flat shaded
	void Render(RImage* pimDst,short sDstX,short sDstY,
		RMesh* pMesh,RZBuffer* pZB,RTexture* pColors,
		short sOffsetX = 0,		// In: 2D offset for pimDst and pZB.
		short sOffsetY = 0); 	// In: 2D offset for pimDst and pZB.

	// Note that pFog must be 256 x # of colors.
	// the offset value moves the fog towards 
	// the front of the z-buffer
	//
	void Render(RImage* pimDst,short sDstX,short sDstY,
		RMesh* pMesh,RZBuffer* pZB,RTexture* pColors,
		short sFogOffset,RAlpha* pFog,
		short sOffsetX = 0,		// In: 2D offset for pimDst and pZB.
		short sOffsetY = 0); 	// In: 2D offset for pimDst and pZB.

	// WARNING: May be inhomogeneous!
	void GetScreenXF(RTransform& tDst)
		{
		tDst.Make1();
		tDst.Mul(m_tScreen.T,m_tView.T);
		}

	// Strictly for convenience:
	//
	void ClearClipBuffer();
	void ClearShadowBuffer();

	// Project a point onto a screen.
	void PointToScreen(RTransform& tObj,RP3d& v3d,short &sDstX,short &sDstY)
		{
		RTransform tFull;
		RP3d ptDst;

		tFull.Make1();
		tFull.Mul(m_tView.T,tObj.T);
		tFull.PreMulBy(m_tScreen.T);

		tFull.TransformInto(v3d,ptDst);
		sDstX = short(ptDst.x);
		sDstY = short(ptDst.y);
		}
	
	// THIS WILL CHANGE WITH TIME:
	// Currently the bounding sphere is described by two points:
	//
	void BoundingSphereToScreen(RP3d& ptCenter, RP3d& ptRadius, 
		RTransform& tObj);

	//-------------------------------------
	// Configurable by instance:
	RZBuffer* m_pZB;
	RImage*	m_pimClipBuf; // For clipping (2 pass rendering)
	RTransform m_tScreen; // map to window
	RTransform m_tView; // lens

	RImage* m_pimShadowBuf;	// For drawing shadows
	RTransform	m_tShadow;	// Turn it into a shadow
	double	m_dShadowScale;// Needed extra parameter
	//-------------------------------------
	// WARNING: THIS WILL LIKELY CHANGE:
	// store a transformed bounding rect for object being rendered:
	// These are screend coordinates relative to the center of
	// the zbuf square / clipping square
	short	m_sX; // far cube point
	short m_sY;
	short m_sZ;
	short m_sW;
	short m_sH;
	short m_sD;

	short	m_sCenX; // for convenience - the cube center
	short m_sCenY; // in 3d screen coordinates
	short m_sCenZ;
	// TRUE of FALSE
	short m_sUseBoundingRect;

	//-------------------------------------
	// static storage:

	// Transformation buffer:
	static long ms_lNumPts;
	static RP3d* ms_pPts;
	static long	ms_lNumPipes; // used to free ms_pPts
	};

//================================================== 
//================================================== 

//================================================== 
#endif
