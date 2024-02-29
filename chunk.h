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
// Chunk.H
// Project: Nostril (aka Postal)
// 
// History:
//		05/13/97 JMI	Started.
//
//		05/22/97	JMI	Can support several types of 'chunks'.
//
//		08/18/97	JMI	Now uses its own internal GetRand() and has randomization
//							arguments to Setup() so the caller can still control the
//							the randomization.
//							Now Construct() will not construct a CChunk if particle
//							effects are disabled.
//
//		09/08/97	JMI	Added Kevlar type for pieces of kevlar vest that get
//							splatter off of dudes with vest.
//
//////////////////////////////////////////////////////////////////////////////
//
// This CThing-derived class will represent pieces of bloody yee (or chunks)
// that fly off a recently damaged, complex organism or something.
// Also, can now support bullet casings and shells.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef CHUNK_H
#define CHUNK_H

#include "RSPiX.h"
#include "realm.h"

class CChunk : public CThing
	{
	//---------------------------------------------------------------------------
	// Types, enums, etc.
	//---------------------------------------------------------------------------
	public:

		typedef enum
			{
			Blood,
			BulletCasing,
			Shell,
			Kevlar,

			NumTypes
			} Type;

		typedef struct
			{
			U8		u8ColorIndex;
			short	sLen;
			} TypeInfo;

	//---------------------------------------------------------------------------
	// Variables
	//---------------------------------------------------------------------------
	public:
		double m_dX;
		double m_dY;
		double m_dZ;

		double m_dRot;
		double m_dVel;
		double m_dVertVel;

		long	m_lPrevTime;

		short m_sSuspend;								// Suspend flag

		Type	m_type;

		short	m_sLen;									// Length of item.
														
	protected:
		CSpriteLine2d		m_sprite;				// Sprite.

		// Note that this is never reseeded b/c this is just an 'effect'
		// that does not and SHOULD not affect game play as it can be
		// turned off.
		static long			ms_lGetRandomSeed;	// Seed for GetRand[om]().

		// Chunk info for each type.
		static TypeInfo	ms_atiChunks[NumTypes];
														
	//---------------------------------------------------------------------------
	// Static Variables
	//---------------------------------------------------------------------------
	public:

	//---------------------------------------------------------------------------
	// Constructor(s) / destructor
	//---------------------------------------------------------------------------
	public:
		// Constructor
		CChunk(CRealm* pRealm)
			: CThing(pRealm, CChunkID)
			{
			m_sSuspend			= 0;
			m_dRot				= 0.0;
			m_dVel				= 0.0;
			m_dVertVel			= 0.0;
			m_sLen				= 0;

			m_sprite.m_pthing		= this;
			m_sprite.m_u8Color	= 1;

			m_type				= Blood;
			}

	public:
		// Destructor
		~CChunk()
			{
			// Remove sprite from scene (this is safe even if it was already removed!)
			m_pRealm->m_scene.RemoveSprite(&m_sprite);
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

			// Don't allow chunks when disabled . . .
			if (g_GameSettings.m_sParticleEffects)
				{
				*ppNew = new CChunk(pRealm);
				if (*ppNew == 0)
					{
					sResult = -1;
					TRACE("CChunk::Construct(): Couldn't construct CChunk (that's really "
						"not that bad a thing)\n");
					}
				}
			else
				{
				// Particles disabled.
				sResult	= 1;
				}

			return sResult;
			}

	//---------------------------------------------------------------------------
	// Virtual functions (implementing them as inlines doesn't pay!)
	//---------------------------------------------------------------------------
	public:
		// Suspend object
		void Suspend(void);

		// Resume object
		void Resume(void);

		// Update object
		void Update(void);

		// Render object
		void Render(void);

		// Note that this setup accepts the amount of random sway you want to
		// apply to the particle so you don't have to.  You should not, otherwise
		// you'll ruin it (the game synch that is). Seriously.
		short Setup(					// Returns 0 on success.
			short sX,					// In:  New x coord
			short sY,					// In:  New y coord
			short sZ,					// In:  New z coord
			double dRot,				// In:  Initial direction.
			short	sRandRotSway,		// In:  Random sway on rotation or zero.
			double dVel,				// In:  Initial velocity.
			short	sRandVelSway,		// In:  Random sway on velocity or zero.
			double dVertVel,			// In:  Initial vertical velocity.
			short	sRandVertVelSway,	// In:  Random sway on velocity or zero.
			Type	type);				// In:  Type of chunk.

		// Get a random number that is in no way related to the game's main
		// GetRand().
		static long GetChunkRand(void)
			{
			return (((ms_lGetRandomSeed = ms_lGetRandomSeed * 214013L + 2531011L) >> 16) & 0x7fff);
			}

	//---------------------------------------------------------------------------
	// Internal functions
	//---------------------------------------------------------------------------
	protected:
	};


#endif // CHUNK_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
