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
// Chunk.cpp
// Project: Nostril (aka Postal)
// 
// History:
//		05/13/97 JMI	Started.
//
//		05/15/97	JMI	Added alpha'ing of blood on the ground.
//
//		05/22/97	JMI	Changed blood alpha level to 60 (was 70).
//
//		05/22/97	JMI	Can support several types of 'chunks'.
//
//		05/26/97	JMI	Changed bullet casing color to 7 (gray) was 3 
//							(dark yellow).
//
//		06/17/97	JMI	Converted all occurrences of rand() to GetRand() and
//							srand() to SeedRand().
//
//		08/18/97	JMI	Now uses its own internal GetRand() and has randomization
//							arguments to Setup() so the caller can still control the
//							the randomization.
//
//		08/25/97	JMI	Setup() was mod'ing m_dRot before adding in the random
//							sway value causing it to sometimes exceed 359.  Fixed.
//
//		09/08/97	JMI	Added Kevlar type for pieces of kevlar vest that 
//							splatter off of dudes with vest.
//
//		09/08/97	JMI	Took out CHUNK_* macros to verify/make-sure they're not
//							used.
//
//////////////////////////////////////////////////////////////////////////////
//
// This CThing-derived class will represent pieces of bloody yee (or chunks)
// that fly off a recently damaged, complex organism or something.
//
//////////////////////////////////////////////////////////////////////////////
#define CHUNK_CPP

#include "RSPiX.h"
#include "chunk.h"
#include "reality.h"

// This class defines its own GetRand()
#undef GetRand
#undef GetRandom

#define GetRand	CChunk::GetChunkRand
#define GetRandom	CChunk::GetChunkRand

////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////

// Gets a random between -range / 2 and range / 2.
#define RAND_SWAY(sway)	((CChunk::GetChunkRand() % sway) - sway / 2)


// Level at which to alpha blood on the ground.
#define ALPHA_LEVEL		60

////////////////////////////////////////////////////////////////////////////////
// Variables/data
////////////////////////////////////////////////////////////////////////////////

// Note that this is never reseeded b/c this is just an 'effect'
// that does not and SHOULD not affect game play as it can be
// turned off.
long		CChunk::ms_lGetRandomSeed	= 0;	// Seed for GetRand[om]().

// Chunk info for each type.
CChunk::TypeInfo	CChunk::ms_atiChunks[CChunk::NumTypes]	=
	{	// u8ColorIndex, sLen
		{	1,		4,	},	// Blood.
		{	7,		3,	},	// BulletCasing.
		{	2,		4,	},	// Shell.
		{	7,		4,	},	// Kevlar.
	};

////////////////////////////////////////////////////////////////////////////////
// Suspend object
////////////////////////////////////////////////////////////////////////////////
void CChunk::Suspend(void)
	{
	m_sSuspend++;
	}


////////////////////////////////////////////////////////////////////////////////
// Resume object
////////////////////////////////////////////////////////////////////////////////
void CChunk::Resume(void)
	{
	m_sSuspend--;
	}

////////////////////////////////////////////////////////////////////////////////
// Update object
////////////////////////////////////////////////////////////////////////////////
void CChunk::Update(void)
	{
	long	lCurTime		= m_pRealm->m_time.GetGameTime();

	double	dSeconds	= (lCurTime - m_lPrevTime) / 1000.0;
	m_lPrevTime			= lCurTime;

	double	dDist		= m_dVel	* dSeconds;

	m_dX					+= COSQ[(short)m_dRot] * dDist;
	m_dZ					-= SINQ[(short)m_dRot] * dDist;

	double dVertDeltaVel	= g_dAccelerationDueToGravity * dSeconds;
	m_dVertVel			+= dVertDeltaVel;

	m_dY					+= (m_dVertVel - dVertDeltaVel / 2) * dSeconds;

	// If we have hit terrain . . .
	if (m_pRealm->GetHeight(m_dX, m_dZ) >= m_dY)
		{
		short	sX2d, sY2d;
		// Map from 3d to 2d coords.
		Map3Dto2D(m_dX, m_dY, m_dZ, &sX2d, &sY2d);

		switch (m_type)
			{
			case Blood:
				{
				RImage*	pim	= m_pRealm->m_phood->m_pimBackground;

				if (	sX2d >= 0 && sY2d >= 0 
					&&	sX2d < pim->m_sWidth
					&& sY2d < pim->m_sHeight)
					{
					// Pixel.  8bpp only!
					U8*	pu8Dst	= pim->m_pData + sX2d + sY2d * pim->m_lPitch;
					
					*pu8Dst	= rspBlendColor(						// Alpha color/index.
						ALPHA_LEVEL,									// Alpha level.
						m_pRealm->m_phood->m_pmaTransparency,	// Multialpha.
						m_sprite.m_u8Color,							// Src color/index to blend.
						*pu8Dst);										// Dst color/index to blend.
					}
				
				break;
				}
			
			case BulletCasing:
			case Shell:
#if 0	// Looks bad.
				rspPlot(
					(U8)251,
					m_pRealm->m_phood->m_pimBackground,
					sX2d, 
					sY2d);

				rspLine(
					m_sprite.m_u8Color,
					m_pRealm->m_phood->m_pimBackground,
					sX2d, 
					sY2d,
					sX2d + RAND_SWAY(BLOOD_SWAY),
					sY2d + RAND_SWAY(BLOOD_SWAY),
					NULL);
#endif
				break;
			}

		// We're done.
		delete this;
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Render object
////////////////////////////////////////////////////////////////////////////////
void CChunk::Render(void)
	{
	// Map from 3d to 2d coords
	Map3Dto2D(m_dX, m_dY, m_dZ, &m_sprite.m_sX2, &m_sprite.m_sY2);
	
	m_sprite.m_sX2End	= m_sprite.m_sX2 + RAND_SWAY(m_sLen);
	m_sprite.m_sY2End	= m_sprite.m_sY2 + RAND_SWAY(m_sLen);

	// Priority is based on bottom edge of sprite on X/Z plane.
	m_sprite.m_sPriority = m_dZ;
	
	// Layer should be based on info we get from attribute map.
	m_sprite.m_sLayer = CRealm::GetLayerViaAttrib(m_pRealm->GetLayer((short) m_dX, (short) m_dZ));

	// Update sprite in scene
	m_pRealm->m_scene.UpdateSprite(&m_sprite);
	}


////////////////////////////////////////////////////////////////////////////////
// Setup object.
////////////////////////////////////////////////////////////////////////////////
short CChunk::Setup(			// Returns 0 if successfull, non-zero otherwise
	short sX,					// In: New x coord
	short sY,					// In: New y coord
	short sZ,					// In: New z coord
	double dRot,				// In: Initial direction.
	short	sRandRotSway,		// In:  Random sway on rotation or zero.
	double dVel,				// In:  Initial velocity.
	short	sRandVelSway,		// In:  Random sway on velocity or zero.
	double dVertVel,			// In:  Initial vertical velocity.
	short	sRandVertVelSway,	// In:  Random sway on velocity or zero.
	Type	type)					// In:  Type of chunk.
	{
	short sResult = 0;
	
	// Use specified position
	m_dX = (double)sX;
	m_dY = (double)sY;
	m_dZ = (double)sZ;

	m_dVel		= dVel;
	m_dVertVel	= dVertVel;

	// Apply randomizations.
	if (sRandRotSway)
		{
		m_dRot	= rspMod360(dRot + RAND_SWAY(sRandRotSway) );
		}
	else
		{
		m_dRot		= rspMod360(dRot);
		}

	if (sRandVelSway)
		{
		m_dVel	+= RAND_SWAY(sRandVelSway);
		}

	if (sRandVertVelSway)
		{
		m_dVertVel	+= RAND_SWAY(sRandVertVelSway);
		}

	m_lPrevTime	= m_pRealm->m_time.GetGameTime();

	m_type		= type;

	ASSERT(type < NumTypes);
	m_sprite.m_u8Color	= ms_atiChunks[type].u8ColorIndex;
	m_sLen					= ms_atiChunks[type].sLen;

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
