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
	#include "GREEN/3D/pipeline.h"
#else
	#include "pipeline.h"
#endif
///////////////////////////////////////////////////////////////
// This is the highest level considered actually part of the 3d engine.
// It is the highest level control -> it decides how 3d pts map to 2d.
// You can customize 3d efects by instantiating your own versions of the 3d pipeline!
///////////////////////////////////////////////////////////////
//	PIPELINE - History
///////////////////////////////////////////////////////////////
//
//	07/23/97	JRD	Added support for generating shadows
//
///////////////////////////////////////////////////////////////

long RPipeLine::ms_lNumPts = 0;
long	RPipeLine::ms_lNumPipes = 0;

RP3d*  RPipeLine::ms_pPts = NULL;

RPipeLine::RPipeLine()
	{
	ms_lNumPipes++; // Track for deletion purposes!
	Init();
	}

void RPipeLine::Init()
	{
	m_pimClipBuf = NULL;
	m_pimShadowBuf = NULL;
	m_pZB = NULL;
	m_sUseBoundingRect = FALSE;
	m_dShadowScale = 1.0;

	m_tScreen.Make1();
	m_tView.Make1();
	m_tShadow.Make1();
	}

// assume the clip rect is identical situation to zBUF:
//
short RPipeLine::Create(long lNum,short sW)
	{
	if (sW)
		{
		// Will only overwrite memory if it needs MORE!
		if ((m_pZB != NULL) && (sW > m_pZB->m_sW))
			{
			delete m_pZB;
			delete m_pimClipBuf;
			m_pimClipBuf = NULL;
			m_pZB = NULL;
			}

		if (m_pZB == NULL)
			{
			m_pZB = new RZBuffer(sW,sW); // no need to clear it yet!
			m_pimClipBuf = new RImage;
			// clear it when appropriate:
			m_pimClipBuf->CreateImage(sW,sW,RImage::BMP8);
			}
		}

	//----------
	if (!lNum) return 0;
	
	if ((ms_pPts != NULL) && (lNum > ms_lNumPts))
		{
		free(ms_pPts);
		ms_pPts = NULL;
		}

	if (ms_pPts == NULL)
		{
		ms_pPts = (RP3d*) malloc(sizeof(RP3d) * lNum);
		}

	return 0;
	}

short RPipeLine::CreateShadow(short sAngleY,
						double dTanDeclension,short sBufSize)
	{
	ASSERT( (sAngleY >=0 ) && (sAngleY < 360) );
	ASSERT(dTanDeclension > 0.0);

	// Create the shadow transform:
	m_tShadow.Make1();
	m_dShadowScale = dTanDeclension;
	m_tShadow.T[1 + ROW0] = m_dShadowScale * rspCos(sAngleY);
	m_tShadow.T[1 + ROW1] = 0.0;
	m_tShadow.T[1 + ROW2] = m_dShadowScale * rspSin(sAngleY);

	// Allocate the buffer, if applicable:
	if (sBufSize <= 0)	// default case:
		{
		if (m_pimShadowBuf == NULL) // do a default allocation
			{
			if (m_pimClipBuf) // use as reference:
				{
				m_pimShadowBuf = new RImage;
				m_pimShadowBuf->CreateImage(
					m_pimClipBuf->m_sWidth * 2,
					m_pimClipBuf->m_sWidth * 2,
					RImage::BMP8);
				return SUCCESS;
				}
			TRACE("RPipeLine::CreateShadow: warning - no buffer set!\n");
			return SUCCESS;
			}
		else
			{
			// No allocatione needed
			return SUCCESS;
			}
		}
	else	// specified buffer size:
		{
		if (m_pimShadowBuf)
			{
			if (m_pimShadowBuf->m_sWidth >= sBufSize) 
				{
				return SUCCESS;	// don't need to expand it
				}
			else
				{
				delete m_pimShadowBuf;
				m_pimShadowBuf = NULL;
				}
			}
		m_pimShadowBuf = new RImage;
		m_pimShadowBuf->CreateImage(sBufSize,sBufSize,RImage::BMP8);
		}

	return SUCCESS;
	}

void RPipeLine::Destroy()
	{
	if (m_pZB) delete m_pZB;
	if (m_pimClipBuf) delete m_pimClipBuf;
	if (m_pimShadowBuf) delete m_pimShadowBuf;
	}

RPipeLine::~RPipeLine()
	{
	Destroy();
	ms_lNumPipes--; // prepare for full death:
	if (!ms_lNumPipes)	// free ms_pPts
		{
		if (ms_pPts) free(ms_pPts);
		ms_lNumPts = 0;
		ms_pPts = NULL;
		}
	}

void RPipeLine::Transform(RSop* pPts,RTransform& tObj)
	{
	RTransform tFull;
	long i;
	// Use to stretch to z-buffer!

	tFull.Make1();
	tFull.Mul(m_tView.T,tObj.T);
	// If there were inhomogeneous transforms, you would need to 
	// trasnform each pt by two transforms separately!
	tFull.PreMulBy(m_tScreen.T);

	for (i = 0; i < pPts->m_lNum; i++)
		{
		tFull.TransformInto(pPts->m_pArray[i],ms_pPts[i]);
		// Note that you can now use RP3d directly with the renderers! 
		}
	}

// Need to create a slightly more complex pipe:
void RPipeLine::TransformShadow(RSop* pPts,RTransform& tObj,
		short sHeight,short *psOffX,short *psOffY)
	{
	ASSERT(m_pimShadowBuf);

	RTransform tFull;
	long i;
	// Use to stretch to z-buffer!

	tFull.Make1();
	// 1) Create Shadow
	tFull.Mul(m_tShadow.T,tObj.T);
	// 2) Add in normal view
	tFull.PreMulBy(m_tView.T);
	// If there were inhomogeneous transforms, you would need to 
	// trasnform each pt by two transforms separately!

	if (psOffX || psOffY) // calculate shadow offset
		{
		// (1) convert to 3d shadow point:
		RP3d	pOffset = {0.0,static_cast<float>(sHeight),0.0,};

		double dOffX = sHeight * m_tShadow.T[ROW0 + 1];
		double dOffY = 0.0;
		double dOffZ = sHeight * m_tShadow.T[ROW2 + 1]; // y is height here

		// (2) partially project
		m_tShadow.Transform(pOffset);
		m_tView.Transform(pOffset);
		// Undo randy slide:
		RP3d pTemp = {m_tView.T[3 + ROW0],m_tView.T[3 + ROW1],
			m_tView.T[3 + ROW2]};

		rspSub(pOffset,pTemp);
		// Just use screen for scale:
		pOffset.x *= m_tScreen.T[0 + ROW0];
		pOffset.y *= m_tScreen.T[1 + ROW1];

		// store result
		*psOffX = short (pOffset.x);
		*psOffY = short (pOffset.y);
		}

	// 3) Project to the "screen"
	tFull.PreMulBy(m_tScreen.T);

	// 4) Adjust the screen transform to keep scaling and mirroring
	// from normal screen transform, but adjust size for shadow buffer
	// This is hard coded to the postal coordinate system
	tFull.Trans(0.0,m_pimShadowBuf->m_sHeight-m_tScreen.T[3 + ROW1],0.0);

	for (i = 0; i < pPts->m_lNum; i++)
		{
		tFull.TransformInto(pPts->m_pArray[i],ms_pPts[i]);
		// Note that you can now use RP3d directly with the renderers! 
		}

	}

// returns 0 if pts are ClockWise! (Hidden)
// (to be used AFTER the view or screen transformation)
short RPipeLine::NotCulled(RP3d *p1,RP3d *p2,RP3d *p3)
	{
	REAL ax,ay,bx,by;
	ax = p2->x - p1->x;
	ay = p2->y - p1->y;
	bx = p3->x - p1->x;
	by = p3->y - p1->y;

	if ( (ax*by - ay*bx) >= 0) return 0;
	else return 1;
	}

// Currently (sDstX,sDstY) allgns with the upper left half of the z-buffer
// Uses the static transformed point buffer.
//
void RPipeLine::Render(RImage* pimDst,short sDstX,short sDstY,
		RMesh* pMesh,UCHAR ucColor) // wire!
	{
	long i;
	long v1,v2,v3;
	USHORT *psVertex = pMesh->m_pArray;
	long lNumHidden = 0;

	for (i=0;i < pMesh->m_sNum; i++)
		{
		v1 = *psVertex++;
		v2 = *psVertex++;
		v3 = *psVertex++;

		if (NotCulled(ms_pPts+v1,ms_pPts+v2,ms_pPts+v3))
			{
			// Render the sucker!
			DrawTri_wire(pimDst,sDstX,sDstY,
				ms_pPts+v1,ms_pPts+v2,ms_pPts+v3,ucColor);
			}
		else
			{
			lNumHidden++; // cull debug
			}
		}
	//TRACE("Number culled was %ld\n",lNumHidden);
	}

// Currently (sDstX,sDstY) allgns with the upper left half of the z-buffer
// Uses the static transformed point buffer.
//
void RPipeLine::RenderShadow(RImage* pimDst,RMesh* pMesh,UCHAR ucColor)
	{
	long i;
	long v1,v2,v3;
	USHORT *psVertex = pMesh->m_pArray;

	for (i=0;i < pMesh->m_sNum; i++)
		{
		v1 = *psVertex++;
		v2 = *psVertex++;
		v3 = *psVertex++;

		if (NotCulled(ms_pPts+v1,ms_pPts+v2,ms_pPts+v3))
			{
			// Render the sucker!
			DrawTri(pimDst->m_pData,pimDst->m_lPitch,
				ms_pPts+v1,ms_pPts+v2,ms_pPts+v3,ucColor);
			}
		}
	}

// YOU clear the z-buffer before this if you so desire!!!
// Currently (sDstX,sDstY) allgns with the upper left half of the z-buffer
// Uses the static transformed point buffer.
//
void RPipeLine::Render(RImage* pimDst,short sDstX,short sDstY,
		RMesh* pMesh,RZBuffer* pZB,RTexture* pTexColors,
		short sFogOffset,RAlpha* pAlpha,
		short sOffsetX/* = 0*/,		// In: 2D offset for pimDst and pZB.
		short sOffsetY/* = 0*/) 	// In: 2D offset for pimDst and pZB.
	{
	long i;
	long v1,v2,v3;
	USHORT *psVertex = pMesh->m_pArray;
	UCHAR *pColor = pTexColors->m_pIndices;
	long lDstP = pimDst->m_lPitch;
	UCHAR* pDst = pimDst->m_pData + (sDstX + sOffsetX) + lDstP * (sDstY + sOffsetY);

	for (i=0;i < pMesh->m_sNum; i++,pColor++)
		{
		v1 = *psVertex++;
		v2 = *psVertex++;
		v3 = *psVertex++;

		if (1)//NotCulled(ms_pPts+v1,ms_pPts+v2,ms_pPts+v3))
			{
			// Render the sucker!
			DrawTri_ZColorFog(pDst,lDstP,
				ms_pPts+v1,ms_pPts+v2,ms_pPts+v3,pZB,
				pAlpha->m_pAlphas[*pColor] + sFogOffset,
				sOffsetX,		// In: 2D offset for pZB.
				sOffsetY);	 	// In: 2D offset for pZB.
			}
		}
	}

// YOU clear the z-buffer before this if you so desire!!!
// Currently (sDstX,sDstY) allgns with the upper left half of the z-buffer
// FLAT SHADE MODE
//
void RPipeLine::Render(RImage* pimDst,short sDstX,short sDstY,
		RMesh* pMesh,RZBuffer* pZB,RTexture* pTexColors,
		short sOffsetX/* = 0*/,		// In: 2D offset for pimDst and pZB.
		short sOffsetY/* = 0*/) 	// In: 2D offset for pimDst and pZB.
	{
	long i;
	long v1,v2,v3;
	USHORT *psVertex = pMesh->m_pArray;
	UCHAR *pColor = pTexColors->m_pIndices;
	long lDstP = pimDst->m_lPitch;
	UCHAR* pDst = pimDst->m_pData + (sDstX + sOffsetX) + lDstP * (sDstY + sOffsetY);

	for (i=0;i < pMesh->m_sNum; i++,pColor++)
		{
		v1 = *psVertex++;
		v2 = *psVertex++;
		v3 = *psVertex++;

		if (NotCulled(ms_pPts+v1,ms_pPts+v2,ms_pPts+v3))
			{
			// Render the sucker!
			DrawTri_ZColor(pDst,lDstP,
				ms_pPts+v1,ms_pPts+v2,ms_pPts+v3,pZB,
				*pColor,
				sOffsetX,		// In: 2D offset for pZB.
				sOffsetY);	 	// In: 2D offset for pZB.
			}
		}
	}

// THIS IS HACKED!  WILL NOT WORK WITH DISTORTED GUYS!
//
void RPipeLine::BoundingSphereToScreen(RP3d& ptCenter, RP3d& ptRadius, 
		RTransform& tObj)
	{
	// THIS IS HARD WIRED TO WORK WITH OUR CURRENT STYLE OF
	// PROJECTION:
	RTransform tFull;
	tFull.Make1();
	tFull.Mul(m_tView.T,tObj.T); // hold off on screen -> get raw distance:
	tFull.PreMulBy(m_tScreen.T);

	// THIS IS IN UNSCALED OBJECT VIEW
	double dModelRadius = sqrt(
		SQR(ptCenter.x - ptRadius.x) + 
		SQR(ptCenter.y - ptRadius.y) + 
		SQR(ptCenter.z - ptRadius.z) ); // Randy Units

	// Convert from Model To Screen...
	double dScreenRadius = dModelRadius * m_tScreen.T[0];

	// Project the center onto the screen:

	RP3d ptCen/*,ptEnd*/;
	tFull.TransformInto(ptCenter,ptCen); // z is now distorted

	// store in pieline variables...(ALL OF THEM)
	m_sCenX = short(ptCen.x);
	m_sCenY = short(ptCen.y);
	m_sCenZ = short(ptCen.z / 256.0); // Scale Z's by 256 for lighting later

	short	sScreenRadius = short(dScreenRadius+1);
	
	m_sX = m_sCenX - sScreenRadius;
	m_sY = m_sCenY - sScreenRadius;
	m_sZ = m_sCenZ - sScreenRadius;

	m_sW = m_sH = (sScreenRadius * 2.);
	m_sD = (sScreenRadius * 2.);

	m_sUseBoundingRect = TRUE;
	}

void RPipeLine::ClearClipBuffer()
	{
	if (m_pimClipBuf == NULL) return;

	rspRect(ULONG(0),m_pimClipBuf,0,0,
		m_pimClipBuf->m_sWidth,m_pimClipBuf->m_sHeight);
	}

void RPipeLine::ClearShadowBuffer()
	{
	if (m_pimShadowBuf == NULL) return;

	rspRect(ULONG(0),m_pimShadowBuf,0,0,
		m_pimShadowBuf->m_sWidth,m_pimShadowBuf->m_sHeight);
	}
