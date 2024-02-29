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
// bulletFest.CPP
// Project: Postal
// 
// History:
//		02/18/97 JMI	Started.
//
//		02/19/97	JMI	Now Ricochet(), Impact(), and Flare() create animations.
//
//		02/19/97	JMI	Now Fire() and FireDeluxe() take all 3 CSmash masks.
//
//		02/19/97	JMI	FireDeluxe(), forces impact, if terrain contact is off
//							of realm.
//
//		02/27/97	JMI	In progress.
//
//		02/27/97	JMI	Experimented with various ways of determining the terrain
//							angle but it is very difficult considering the attribute
//							maps granularity is 8!
//
//		02/27/97	JMI	Forgot to remove feedback flag to preprocessor.
//
//		02/27/97	JMI	Fongoolishness.
//
//		03/03/97	JMI	Now uses QuickCheckClosest() instead of QuickCheck().
//
//		03/05/97	JMI	Fixed rspMod360() usage to current.
//
//		03/10/97	JMI	Added unimplemented UpdateTarget() stub.
//
//		03/21/97	JMI	Added optional tracers.
//
//		03/21/97 BRH	Fixed a small divide by zero error check.
//
//		04/02/97	JMI	Changed ms_ati to m_ati.
//							Also, added ms_u8TracerIndex.
//							Also, removed all the edge info crap.
//							Also, added two additional ways of displaying tracers.
//
//		04/02/97	JMI	Feedback now uses CSpriteLine2d.
//							Also, now uses mapping using 4 clusters &'d together
//							as an index to determine the terrain angle.
//							Does not seem to work.
//
//		04/02/97	JMI	Switched back to METHOD 2 and fixed a divide by zero bug
//							in there.  Also, reduced TRACER_MAX_LENGTH to 3.
//
//		04/10/97 BRH	Updated this to work with the new multi layer attribute
//							maps.
//
//		04/24/97	JMI	Added smid parameter specifying type of ammo noise to
//							FireDeluxe() and Flare().
//
//		04/29/97	JMI	Fire() now allows bullets to go 10 pixels off the realm
//							to make it appear as if the edge of the realm has no
//							affect on them (except the bottom edge which is greater
//							so that, if the bullet is exceptionally high, it will
//							go, hopefully, entirely off screen).
//
//		05/29/97	JMI	Removed references to m_pRealm->m_pAttribMap which no longer
//							exists.
//
//		06/11/97	JMI	Added Preload() for loading assets used during play.
//
//		06/17/97	JMI	Converted all occurrences of rand() to GetRand() and
//							srand() to SeedRand().
//
//		06/26/97	JMI	Now uses pRealm->Map3Dto2D() in Fire() (was using
//							the global version defined in reality.h which is no more).
//
//		06/28/97	JMI	Missed some Map3Dto2D()'s that were #if 0'ed out.
//
//		06/30/97	JMI	Now uses CRealm's new GetRealmWidth() and *Height()
//							for dimensions of realm's X/Z plane.
//
//		07/01/97	JMI	Now bases ricochets on GetRandom().
//
//		07/09/97	JMI	Now uses m_pRealm->Make2dResPath() to get the fullpath
//							for 2D image components.
//
//		07/09/97	JMI	Changed Preload() to take a pointer to the calling realm
//							as a parameter.
//
//		07/18/97	JMI	Got rid of bogus immitation PlaySample functions.
//							Now there is one PlaySample() function.  Also, you now
//							MUST specify a category and you don't have to specify a
//							SoundInstance ptr to specify a volume.
//
//		07/27/97	JMI	No longer calls CRealm::Make2dResPath() on res names
//							given to CAnimThing as it already does that 
//							automatically.
//
//		08/20/97 BRH	Changed ricochet sounds to Weapon volume control
//							instead of destruction.
//
//		09/18/97	JMI	UpdateTracerColor() now takes a realm ptr and now is 
//							color mapped on the first Fire() call.
//
//////////////////////////////////////////////////////////////////////////////
//
// bulletFest handles launching of bullets.  Currently it doesn't seem like
// much more than a function call, but eventually it will need to keep track
// of some per instance data.
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// C Headers -- Must be included before RSPiX.h b/c RSPiX utilizes SHMalloc.
//////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// RSPiX Headers.
///////////////////////////////////////////////////////////////////////////////

#include "RSPiX.h"

//////////////////////////////////////////////////////////////////////////////
// Postal headers.
//////////////////////////////////////////////////////////////////////////////
#include "bulletFest.h"
#include "AnimThing.h"
#include "SampleMaster.h"
#include "reality.h"

//////////////////////////////////////////////////////////////////////////////
// Module specific macros.
//////////////////////////////////////////////////////////////////////////////

#define IMPACT_RES_NAME		"Ricochet.aan"
#define RICOCHET_RES_NAME	"Ricochet.aan"
#define FLARE_RES_NAME		"MuzzleFlare.aan"

#define TERRAIN_CHECK_DIST				1
#define TERRAIN_CHECK_GRANULARITY	4

#define TRACER_COLOR_RED		250
#define TRACER_COLOR_GREEN		200
#define TRACER_COLOR_BLUE		50

// The color range that is the same on all hoods.
#define CONSTANT_COLOR_START_INDEX	95
#define NUM_CONSTANT_COLOR_INDICES	(256 - CONSTANT_COLOR_START_INDEX - 10)

#define TRACER_WRAP_DISTANCE	10
#define TRACER_MAX_LENGTH		3

#define LEFT_TOP		0x1
#define RIGHT_TOP		0x2
#define LEFT_BOTTOM	0x4
#define RIGHT_BOTTOM	0x8

// This is done as a mask instead of a mod for speed.
// Note that 0xFFFF is 100% of the time, 0x00FF is 50%, etc.
// Chances are number of set bits out of 16.
#define RICOCHET_CHANCE_MASK	0x0001

#define BULLET_SND_HALF_LIFE	1000

//////////////////////////////////////////////////////////////////////////////
// Module specific typedefs.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Exported (extern) variables.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Module specific (static) variables / Instantiate class statics.
//////////////////////////////////////////////////////////////////////////////

U8	CBulletFest::ms_u8TracerIndex	= 0;	// The color index to use for tracers.
													// This value is gotten only once per
													// execution of this program; therefore,
													// it is gotten from the group of colors
													// that don't change from hood to hood.

// Maps a combination of bits to an angle.
// Each bit set to 1 indicates the terrain in that location is higher than
// the specified height.
// Less than 2 bits set is considered not enough info.
// 2 diagonally adjacent bits is considered not enough info.
static short ms_asTerrainAngleMap[16]	=
	{
	-1,	// 0000	==
	-1,	// 0001	== LEFT_TOP
	-1,	// 0010	== RIGHT_TOP
	0,		// 0011	== RIGHT_TOP | LEFT_TOP
	-1,	// 0100	== LEFT_BOTTOM
	90,	// 0101	== LEFT_BOTTOM | LEFT_TOP
	-1,	// 0110	== LEFT_BOTTOM | RIGHT_TOP
	45,	// 0111	== LEFT_BOTTOM | RIGHT_TOP | LEFT_TOP
	-1,	// 1000	== RIGHT_BOTTOM
	-1,	// 1001	== RIGHT_BOTTOM | LEFT_TOP
	270,	// 1010	== RIGHT_BOTTOM | RIGHT_TOP
	315,	// 1011	== RIGHT_BOTTOM | RIGHT_TOP | LEFT_TOP
	180,	// 1100	== RIGHT_BOTTOM | LEFT_BOTTOM
	135,	// 1101	== RIGHT_BOTTOM | LEFT_BOTTOM | LEFT_TOP
	225,	// 1110	== RIGHT_BOTTOM | LEFT_BOTTOM | RIGHT_TOP
	-1,	// 1111	== RIGHT_BOTTOM | LEFT_BOTTOM | RIGHT_TOP | LEFT_TOP
	};

//////////////////////////////////////////////////////////////////////////////
// Module specific (static) protos.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Functions.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Get max height in a rectangle.
//////////////////////////////////////////////////////////////////////////////
inline
short GetMaxHeight(
	CRealm* pRealm,				// In:  Realm in question.
	short sX,						// In:  X position on terrain.
	short sZ,						// In:  Z position on terrain.
	short	sW,						// In:  Amount in X direction.
	short	sH)						// In:  Amount in Z direction.
	{
	short	sX2	= sX + sW;
	short	sZ2	= sZ + sH;

	short sIterX;
	short	sIterZ;
	short	sMaxHeight	= -32767;
	for (sIterZ = sZ; sIterZ < sZ2; sIterZ++)
		{
		for (sIterX = sX; sIterX < sX2; sIterX++)
			{
			// Relying on NON-macro MAX (macro MAX would evaluate one of the paramters
			// twice).
			sMaxHeight = MAX(sMaxHeight, pRealm->GetHeight(sIterX, sIterZ));
#if 0
			// FEEDBACK.
			// Create a line sprite.
			CSpriteLine2d*	psl2d	= new CSpriteLine2d;
			if (psl2d != NULL)
				{
				psl2d->m_sX2		= sIterX, 
				psl2d->m_sY2		= sIterZ;
				psl2d->m_sX2End	= sIterX + 1; 
				psl2d->m_sY2End	= sIterZ + 1;
				psl2d->m_sPriority	= 0;
				psl2d->m_sLayer		= CRealm::GetLayerViaAttrib(m_pRealm->GetLayer(sIterX, sIterZ));
				psl2d->m_u8Color		= 250;
				// Destroy when done.
				psl2d->m_sInFlags	= CSprite::InDeleteOnRender;
				// Put 'er there.
				pRealm->m_scene.UpdateSprite(psl2d);
				}
#endif
			}
		}

	return sMaxHeight;
	}

//////////////////////////////////////////////////////////////////////////////
// 
// Take a point inside terrain and find our way out via X/Z plane.
// Once out, scan along terrain to determine the angle of the terrain that
// exceeds sY in height.
// ASSUMPTION: sX, sY, sZ is a point inside terrain.
//
//////////////////////////////////////////////////////////////////////////////
inline
bool GetTerrainAngle(			// true, if terrain angle determined.
										// false, if not enough iterations available to
										// determine angle.
	CRealm* pRealm,				// In:  Realm in question.
	short sX,						// In:  X position on terrain.
	short sY,						// In:  Y position on terrain.
	short sZ,						// In:  Z position on terrain.
	short sMaxOutIterations,	// In:  Max iterations to get out of terrain
										// (i.e., find terrain edge).
	short sNumScanIterations,	// In:  Num iterations to scan edge of terrain
										// in EACH direction.
	short	sInAngle,				// In:  Angle from which object entered terrain.
										// This angle + 180 is used to scan out.
	short* psAngle)				// Out: Angle of terrain at specified location
										// on X/Z plane.
	{
	bool bFoundAngle	= false;	// Assume not found.
#if 0  // NOT WORKING CORRECTLY.

	////////////////////// Get out of terrain /////////////////////////////////
	

	sInAngle	= rspMod360(sInAngle);
	// Determine rate to get in deeper.
	float fInRateX	= COSQ[sInAngle];
	float fInRateZ	= -SINQ[sInAngle];

	// Get angles to get out.
	short sOutAngle	= rspMod360(90 + sInAngle);	// 180 + sInAngle - 90

	// Determine proper rate to get out of terrain.
	float	fRateX		= COSQ[sOutAngle]/* * pRealm->m_pAttribMap->m_sScaleX*/;
	float	fRateZ		= -SINQ[sOutAngle]/* * pRealm->m_pAttribMap->m_sScaleY*/;

	const	float	fInMult	= (sMaxOutIterations - 3);

	// Set initial position inside terrain.
	float	fPosX		= sX + fInRateX * fInMult;
	float	fPosZ		= sZ + fInRateZ * fInMult;

	// Store extents.
	short	sMaxX			= pRealm->GetRealmWidth();
	short	sMaxZ			= pRealm->GetRealmHeight();
	
	short	sIterations	= 0;

	// Quicker access to attribute map for loop.
	RAttributeMap*	pattrib	= pRealm->m_pAttribMap;
	short	sTerrainH;
	bool	bGotOut	= false;

	// Scan while in realm.
	while (
			fPosX > 0.0 
		&& fPosZ > 0.0 
		&& fPosX < sMaxX 
		&& fPosZ < sMaxZ
		&& sIterations < sMaxOutIterations)
		{
		sTerrainH = m_pRealm->GetHeight((short) fPosX, (short) fPosZ);
		// If above terrain . . .
		if (sY > sTerrainH)
			{
			bGotOut	= true;
//			fPosX		-= fRateX;
//			fPosZ		-= fRateZ;
			break;
			}

		fPosX	+= fRateX;
		fPosZ	+= fRateZ;

		sIterations++;
		}

	// If we did not find our way out . . .
	if (bGotOut == false)
		{
		sOutAngle	= rspMod360(270 + sInAngle);	// 180 + sInAngle + 90
		fRateX		= COSQ[sOutAngle]/* * pRealm->m_pAttribMap->m_sScaleX*/;
		fRateZ		= -SINQ[sOutAngle]/* * pRealm->m_pAttribMap->m_sScaleY*/;
		fPosX		= sX + fInRateX * fInMult;
		fPosZ		= sZ + fInRateZ * fInMult;
		sIterations	= 0;
		// Scan while in realm.
		while (
				fPosX > 0.0 
			&& fPosZ > 0.0 
			&& fPosX < sMaxX 
			&& fPosZ < sMaxZ
			&& sIterations < sMaxOutIterations)
			{
			sTerrainH = m_pRealm->GetHeight((short) fPosX, (short) fPosZ);
			// If above terrain . . .
			if (sY > sTerrainH)
				{
				bGotOut	= true;
//				fPosX		-= fRateX;
//				fPosZ		-= fRateZ;
				break;
				}

			fPosX	+= fRateX;
			fPosZ	+= fRateZ;

			sIterations++;
			}
		}

	// If we found our way out in either case . . .
	if (bGotOut == true)
		{
		// Determine angle of a line between the two points.
		short sDeltaX	= fPosX - (sX - fInRateX);
		short sDeltaZ	= (sZ - fInRateZ) - fPosZ;

		*psAngle	= rspATan(sDeltaX, sDeltaZ);

		// Found angle.
		bFoundAngle	= true;
		}

#else
	// Check 4 surrounding points.
	short	sIndex	= 0;
	if (GetMaxHeight(pRealm, sX - TERRAIN_CHECK_DIST - TERRAIN_CHECK_GRANULARITY / 2, sZ - TERRAIN_CHECK_DIST - TERRAIN_CHECK_GRANULARITY / 2, TERRAIN_CHECK_GRANULARITY, TERRAIN_CHECK_GRANULARITY) > sY)
		sIndex	= LEFT_TOP;
	if (GetMaxHeight(pRealm, sX + TERRAIN_CHECK_DIST + TERRAIN_CHECK_GRANULARITY / 2, sZ - TERRAIN_CHECK_DIST - TERRAIN_CHECK_GRANULARITY / 2, TERRAIN_CHECK_GRANULARITY, TERRAIN_CHECK_GRANULARITY) > sY)
		sIndex	|= RIGHT_TOP;
	if (GetMaxHeight(pRealm, sX - TERRAIN_CHECK_DIST - TERRAIN_CHECK_GRANULARITY / 2, sZ + TERRAIN_CHECK_DIST + TERRAIN_CHECK_GRANULARITY / 2, TERRAIN_CHECK_GRANULARITY, TERRAIN_CHECK_GRANULARITY) > sY)
		sIndex	|= LEFT_BOTTOM;
	if (GetMaxHeight(pRealm, sX + TERRAIN_CHECK_DIST + TERRAIN_CHECK_GRANULARITY / 2, sZ + TERRAIN_CHECK_DIST + TERRAIN_CHECK_GRANULARITY / 2, TERRAIN_CHECK_GRANULARITY, TERRAIN_CHECK_GRANULARITY) > sY)
		sIndex	|= RIGHT_BOTTOM;

	// Get terrain angle.
	*psAngle	= ms_asTerrainAngleMap[sIndex];

	if (*psAngle > -1)
		{
		bFoundAngle	= true;
		}

#endif	

	return bFoundAngle;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Launch a bullet, create a muzzle flare at the src (sX, sY, sZ), and,
// if no CThing hit, create a ricochet at where the bullet contacted 
// terrain (*psX, *psY, *psZ).
// This same process can be done in pieces with more control to the user
// by calling Fire(), Flare(), and Ricochet().
//
//////////////////////////////////////////////////////////////////////////////
bool CBulletFest::FireDeluxe(	// Returns what and as Fire() would.
	short sAngleY,					// In:  Angle of launch in degrees (on X/Z plane).
	short	sAngleZ,					// In:  Angle of launch in degrees (on X/Y plane).
	short	sX,						// In:  Launch position.
	short	sY,						// In:  Launch position.
	short	sZ,						// In:  Launch position.
	short	sRange,					// In:  Maximum distance.
	CRealm* pRealm,				// In:  Realm in which to fire.
	CSmash::Bits bitsInclude,	// In:  Mask of CSmash masks that this bullet can hit.
	CSmash::Bits bitsDontCare,	// In:  Mask of CSmash masks that this bullet does not care to hit.
	CSmash::Bits bitsExclude,	// In:  Mask of CSmash masks that this bullet cannot hit.
	short	sMaxRicochetAngle,	// In:  Maximum angle with terrain that can cause
										// a ricochet (on X/Z plane).
	short	sMaxRicochets,			// In:  The maximum number of ricochets.
	short* psX,						// Out: Hit position.
	short* psY,						// Out: Hit position.
	short* psZ,						// Out: Hit position.
	CThing** ppthing,				// Out: Ptr to thing hit or NULL.
	bool	bTracer /*= true*/,	// In:  Draw a tracer at random point along path.
	SampleMasterID	smid	/*= g_smidBulletFire*/)	// In:  Use ammo sample.
	{
	bool bHit	= false;	// Assume no collision with CThing within u32ThingMask.

	// Create a muzzle flare.
	Flare(sAngleY, sX, sY, sZ, pRealm, smid);

	short	sRicochets		= 0;
	bool	bImpact			= false;
	short	sTerrainAngle	= 0;
	// While no hits and not out of ricochets . . .
	while (bHit == false && sRicochets <= sMaxRicochets && bImpact == false)
		{
		sAngleY	= rspMod360(sAngleY);

		bHit	= Fire(
			sAngleY,			// In:  Angle of launch in degrees (on X/Z plane).
			sAngleZ,			// In:  Angle of launch in degrees (on X/Y plane).
			sX,				// In:  Launch position.
			sY,				// In:  Launch position.
			sZ,				// In:  Launch position.
			sRange,			// In:  Maximum distance.
			pRealm,			// In:  Realm in which to fire.
			bitsInclude,	// In:  Mask of CSmash masks that this bullet can hit.
			bitsDontCare,	// In:  Mask of CSmash masks that this bullet does not care to hit.
			bitsExclude,	// In:  Mask of CSmash masks that this bullet cannot hit.
			psX,				// Out: Hit position.
			psY,				// Out: Hit position.
			psZ,				// Out: Hit position.
			ppthing,			// Out: Ptr to thing hit or NULL.
			bTracer);		// In:  Draw a tracer at random point along path.

		// If no hit . . .
		if (bHit == false)
			{
			// If on screen . . .
			if (	*psX > 0 && *psX < pRealm->GetRealmWidth() 
				&& *psZ > 0 && *psZ - *psY < pRealm->GetRealmHeight() )
				{
#if 0
				// If out of ricochets . . .
				if (sRicochets >= sMaxRicochets)
					{
					// Impact.
					bImpact	= true;
					}
				else
					{
					if (GetTerrainAngle(pRealm, *psX, *psY, *psZ, 10, 10, sAngleY, &sTerrainAngle) == true)
						{
						// Check for ricochet.  If our contact angle is within sMaxRicochetAngle
						// of terrain angle . . .
						short	sAngleDif	= sTerrainAngle - sAngleY;
						if (ABS(sAngleDif) > sMaxRicochetAngle)
							{
							bImpact	= true;
							}
						else
							{
							sAngleY	= sTerrainAngle + sAngleDif;
							}
	
#if 0
						// Might have to break this into four quadrants.

						// This works for the case when sAngle is in quadrant 1
						// and sTerrainAngle is in quadrant 2.
						short sAngleDif	= (180 - sTerrainAngle) + sAngleY;

						if (sAngleDif > 90)
							{
							sAngleDif = 180 - sAngleDif;
							}

						if (sAngleDif > sMaxRicochetAngle)
							{
							// Impact.
							bImpact	= true;
							}
						else	// Ricochet.
							{
							// **** Determine deflection angle here.
							sAngleY	= 360 - sAngleDif;
							}
#endif
						}
					else
						{
						bImpact	= true;
						}
					}

				// If impacting . . .
				if (bImpact == true)
					{
					// Impact at angle of shot.
					Impact(sAngleY, *psX, *psY, *psZ, pRealm);
					}
				else
					{
					// Ricochet at angle of deflection.

					Ricochet(sAngleY, *psX, *psY, *psZ, pRealm);

					sX	= *psX;
					sY	= *psY;
					sZ	= *psZ;
					}
#else	
				// Ricochet is random.
				if (GetRandom() & RICOCHET_CHANCE_MASK)
					{
					// Impact at angle of shot.
					Impact(sAngleY, *psX, *psY, *psZ, pRealm);
					}
				else
					{
					// Ricochet at angle of deflection.
					Ricochet(sAngleY, *psX, *psY, *psZ, pRealm);
					}

				// Always stop though.
				bImpact	= true;
#endif
				}
			}

		sRicochets++;
		}

	return bHit;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Create a muzzle flare effect.
//
//////////////////////////////////////////////////////////////////////////////
void CBulletFest::Flare(		// Returns nothing.
	short sAngle,					// In:  Angle of launch in degrees (on X/Z plane).
	short	sX,						// In:  Launch position.
	short	sY,						// In:  Launch position.
	short	sZ,						// In:  Launch position.
	CRealm* pRealm,				// In:  Realm in which to fire.
	SampleMasterID	smid	/*= g_smidBulletFire*/)	// In:  Use ammo sample.
	{
	// Create the animator . . .
	CAnimThing*	pat	= new CAnimThing(pRealm);
	ASSERT(pat != NULL);

	strcpy(pat->m_szResName, FLARE_RES_NAME);

	// Start it up:
	// No looping.
	pat->m_sLoop	= FALSE;
	// No notification necessary.
	// NOTE:  sAngle is currently not utilized.
	pat->Setup(sX, sY, sZ);

	// Start sample.
	PlaySample(
		smid,
		SampleMaster::Weapon,
		DistanceToVolume(sX, sY, sZ, BULLET_SND_HALF_LIFE) );
	}

//////////////////////////////////////////////////////////////////////////////
//
// Create a impact effect.
//
//////////////////////////////////////////////////////////////////////////////
void CBulletFest::Impact(	// Returns nothing.
	short sAngle,				// In:  Angle of launch in degrees (on X/Z plane).
	short	sX,					// In:  Launch position.
	short	sY,					// In:  Launch position.
	short	sZ,					// In:  Launch position.
	CRealm* pRealm)			// In:  Realm in which to fire.
	{
	// Create the animator . . .
	CAnimThing*	pat	= new CAnimThing(pRealm);
	ASSERT(pat != NULL);

	strcpy(pat->m_szResName, IMPACT_RES_NAME);

	// Start it up:
	// No looping.
	pat->m_sLoop	= FALSE;
	// No notification necessary.
	// NOTE:  sAngle is currently not utilized.
	pat->Setup(sX, sY, sZ);
	}

//////////////////////////////////////////////////////////////////////////////
//
// Create a ricochet effect.
//
//////////////////////////////////////////////////////////////////////////////
void CBulletFest::Ricochet(	// Returns nothing.
	short sAngle,					// In:  Angle of launch in degrees (on X/Z plane).
	short	sX,						// In:  Launch position.
	short	sY,						// In:  Launch position.
	short	sZ,						// In:  Launch position.
	CRealm* pRealm)				// In:  Realm in which to fire.
	{
	// Create the animator . . .
	CAnimThing*	pat	= new CAnimThing(pRealm);
	ASSERT(pat != NULL);

	strcpy(pat->m_szResName, RICOCHET_RES_NAME);

	// Start it up:
	// No looping.
	pat->m_sLoop	= FALSE;
	// No notification necessary.
	// NOTE:  sAngle is currently not utilized.
	pat->Setup(sX, sY, sZ);

	// Start sample.
	PlaySample(
		(GetRand() % 2) ? g_smidRicochet1 : g_smidRicochet2,
		SampleMaster::Weapon,
		DistanceToVolume(sX, sY, sZ, BULLET_SND_HALF_LIFE) );
	}

//////////////////////////////////////////////////////////////////////////////
//
// Launch a bullet.
//
//////////////////////////////////////////////////////////////////////////////
bool CBulletFest::Fire(			// Returns true if a hit, false otherwise.
	short sAngleY,					// In:  Angle of launch in degrees (on X/Z plane).
	short	sAngleZ,					// In:  Angle of launch in degrees (on X/Y plane).
	short	sX,						// In:  Launch position.
	short	sY,						// In:  Launch position.
	short	sZ,						// In:  Launch position.
	short	sRange,					// In:  Maximum distance.
	CRealm* pRealm,				// In:  Realm in which to fire.
	CSmash::Bits bitsInclude,	// In:  Mask of CSmash masks that this bullet can hit.
	CSmash::Bits bitsDontCare,	// In:  Mask of CSmash masks that this bullet does not care to hit.
	CSmash::Bits bitsExclude,	// In:  Mask of CSmash masks that this bullet cannot hit.
	short* psX,						// Out: Hit position.
	short* psY,						// Out: Hit position.
	short* psZ,						// Out: Hit position.
	CThing** ppthing,				// Out: Ptr to thing hit or NULL.
	bool	bTracer /*= true*/)	// In:  Draw a tracer at random point along path.
	{
	bool bHit	= false;	// Assume no collision with CThing within u32ThingMask.

	// If tracer color not yet color mapped . . .
	if (ms_u8TracerIndex == 0)
		{
		UpdateTracerColor(pRealm);
		}
		
	/////////// Determine length of travel if no CThings hit ///////////////////

	// Get most efficient increments that won't miss any attributes.
	// For the rates we use trig with a hypotenuse of 1 which will give
	// us a rate <= 1.0 and then multiply by the scaling of the map for
	// a reasonable doubling or so of the speed of this alg.
	
	// sAngle must be between 0 and 359.
	sAngleY	= rspMod360(sAngleY);
	sAngleZ	= rspMod360(sAngleZ);

	float	fRateX		= COSQ[sAngleY]/* * pRealm->m_pAttribMap->m_sScaleX*/;
	float	fRateZ		= -SINQ[sAngleY]/* * pRealm->m_pAttribMap->m_sScaleY*/;
	float	fRateY		= SINQ[sAngleZ];	

	// Determine amount traveled per iteration just once.
	float	fIterDist	= sqrt(ABS2(fRateX, fRateY, fRateZ) );

	// Set initial position to first point to check (NEVER checks original position).
	float	fPosX			= sX + fRateX;
	float	fPosY			= sY + fRateY;
	float	fPosZ			= sZ + fRateZ;

	float	fTotalDist	= 0.0F;

	// Store extents allowing bullets to go 10 pixels off each boundary.
	// This makes them appear to be unhindered by the edge of the realm.
	// (except the bottom edge which is greater so that, if the bullet 
	// is exceptionally high, it will go, hopefully, entirely off screen).	
	short	sMaxX			= pRealm->GetRealmWidth() + 10;
	short	sMaxY			= 512;					// Robustness: Guard against infinite loop
														// in the remote possibility that we shoot
														// straight up (currently one can only shoot
														// horizontally).
	short	sMaxZ			= pRealm->GetRealmHeight() + sY + 10;

	short	sMinX			= -10;
	short	sMinY			= -512;
	short	sMinZ			= -10;

	short	sCurH;

	// Scan while in realm.
	while (
			fPosX > sMinX 
		&& fPosY > sMinY 
		&& fPosZ > sMinZ 
		&& fPosX < sMaxX 
		&& fPosY < sMaxY 
		&& fPosZ < sMaxZ)
		{
		sCurH = pRealm->GetHeight((short) fPosX, (short) fPosZ);
		// If bullet below or at terrain . . .
		if (fPosY <=  sCurH)
			{
			break;
			}

		// If we've exceeded range . . .
		if (fTotalDist > sRange)
			{
			// Drop bullet via gravity.
			fPosY	-= 1.0;			// FUDGE!
			}

		fPosX	+= fRateX;
		fPosY	+= fRateY;
		fPosZ	+= fRateZ;
		fTotalDist	+= fIterDist;
		}

	// Create 3D line segment.
	R3DLine	line;
	line.X1	= sX;
	line.Y1	= sY;
	line.Z1	= sZ;
	line.X2	= fPosX;
	line.Y2	= fPosY;
	line.Z2	= fPosZ;

	// Determine if anything with mask u32ThingMask was hit . . .
	CSmash*	psmash;
	if (pRealm->m_smashatorium.QuickCheckClosest(
		&line, 
		bitsInclude, 
		bitsDontCare, 
		bitsExclude, 
		&psmash) == true)
		{
		// Set *ppthing to thing hit.
		*ppthing	= psmash->m_pThing;

		bHit	= true;

		// Set end pt.
		*psX		= psmash->m_sphere.sphere.X;
		*psY		= psmash->m_sphere.sphere.Y;
		*psZ		= psmash->m_sphere.sphere.Z;
		}
	else
		{
		// Clear thing ptr.
		*ppthing	= NULL;
		// Set end pt.
		*psX		= fPosX;
		*psY		= fPosY;
		*psZ		= fPosZ;
		}

	if (bTracer == true)
		{
		// Create a line sprite.
		CSpriteLine2d*	psl2d	= new CSpriteLine2d;
		if (psl2d != NULL)
			{
			// Get distance.  Since an interception may have occurred, we need to redetermine the
			// distance.
			// We use one coordinate of the end point against one coordinate of the interception point
			// to get a ratio and apply that to the total distance.
			short	sDistance	= 0;
			if (fPosX - sX != 0.0)
				{
				sDistance	= fTotalDist * ((*psX - sX) / (fPosX - sX));
				}

#define METHOD	2			
#define TRACER_ACCUMMULATION_STEP_MAX	80
#define TRACER_ACCUMMULATION_STEP_MIN	10

			short	sRand		= 0;
#if METHOD == 1	// Method 1:  Random dist between here and there.
			
			if (sDistance != 0)
				{
				sRand	= GetRand() % sDistance;
				}

#elif METHOD == 2	// Method 2:  Cummulating position with wrapping.

			if (sDistance != 0)
				{
				sRand	= m_sCurTracerPos	= (m_sCurTracerPos + GetRand() % TRACER_ACCUMMULATION_STEP_MAX) % sDistance;
				}

#else	METHOD == 3	// Method 3:  Multiple Method 1 and Method 2's.
			
			// Reduce distance.
			sDistance	-= TRACER_ACCUMMULATION_STEP_MAX;

			while (sRand < sDistance)
				{
				sRand	+= (GetRand() % (TRACER_ACCUMMULATION_STEP_MAX - TRACER_ACCUMMULATION_STEP_MIN)) + TRACER_ACCUMMULATION_STEP_MIN;
				// If no current sprite . . . 
				if (psl2d == NULL)
					{
					// Create a line sprite.
					psl2d	= new CSpriteLine2d;
					}

				// If we have a sprite . . .
				if (psl2d != NULL)
					{
#endif

			short	sRand2	= GetRand() % TRACER_MAX_LENGTH;

			float	fStartX	= sX + fRateX * sRand;
			float	fStartY	= sY + fRateY * sRand;
			float	fStartZ	= sZ + fRateZ * sRand;

			pRealm->Map3Dto2D(
				fStartX, 
				fStartY, 
				fStartZ, 
				&(psl2d->m_sX2), 
				&(psl2d->m_sY2) );
			pRealm->Map3Dto2D(
				fStartX + fRateX * sRand2, 
				fStartY + fRateY * sRand2, 
				fStartZ + fRateZ * sRand2, 
				&(psl2d->m_sX2End), 
				&(psl2d->m_sY2End) );
			psl2d->m_sPriority	= fStartZ;
			psl2d->m_sLayer		= CRealm::GetLayerViaAttrib(pRealm->GetLayer((short) fStartX, (short) fStartZ));
			psl2d->m_u8Color		= ms_u8TracerIndex;
			// Destroy when done.
			psl2d->m_sInFlags	= CSprite::InDeleteOnRender;
			// Put 'er there.
			pRealm->m_scene.UpdateSprite(psl2d);
#if METHOD == 3
					
					// Done with this 'sprite'.
					psl2d	= NULL;
					}
				}
#endif
			}
		else
			{
			TRACE("Fire(): Unable to allocate CSpriteLine2d.\n");
			}
		}

#if 0
	// FEEDBACK.
	// Create a line sprite.
	CSpriteLine2d*	psl2d	= new CSpriteLine2d;
	if (psl2d != NULL)
		{
		pRealm->Map3Dto2D(
			sX, 
			sY, 
			sZ, 
			&(psl2d->m_sX2), 
			&(psl2d->m_sY2) );
		pRealm->Map3Dto2D(
			sX + fRateX * fTotalDist, 
			sY + fRateY * fTotalDist, 
			sZ + fRateZ * fTotalDist, 
			&(psl2d->m_sX2End), 
			&(psl2d->m_sY2End) );
		psl2d->m_sPriority	= sZ;
		psl2d->m_sLayer		= CRealm::GetLayerViaAttrib(pRealm->GetLayer(sX, sZ));
		psl2d->m_u8Color		= ms_u8TracerIndex;
		// Destroy when done.
		psl2d->m_sInFlags	= CSprite::InDeleteOnRender;
		// Put 'er there.
		pRealm->m_scene.UpdateSprite(psl2d);
		}
#endif

	return bHit;
	}

///////////////////////////////////////////////////////////////////////////////
// Update current targeting information.  This will function will
// continually track the position of a target in order to determine
// how much it is moving.  The idea being that targets that move 
// irratically(sp?) and more often are harder to hit.
///////////////////////////////////////////////////////////////////////////////
void CBulletFest::UpdateTarget(	// Returns nothing.
	short sAngle,						// In:  Angle of aim in degrees (on X/Z plane).
	short	sX,							// In:  Aim position.
	short	sY,							// In:  Aim position.
	short	sZ,							// In:  Aim position.
	CRealm* pRealm)					// In:  Realm in which to target.
	{
	// NYI!
	}

///////////////////////////////////////////////////////////////////////////////
// Updates the static tracer color.
// (static)
///////////////////////////////////////////////////////////////////////////////
void CBulletFest::UpdateTracerColor(
	CRealm* prealm)							// In:  Calling realm.
	{
	ASSERT(prealm->m_phood);
	ASSERT(prealm->m_phood->m_pimBackground);
	ASSERT(prealm->m_phood->m_pimBackground->m_pPalette);

	RPal*	ppal	= prealm->m_phood->m_pimBackground->m_pPalette;

	ms_u8TracerIndex	= rspMatchColorRGB(	// Out: Matched index.
		TRACER_COLOR_RED,							// In:  Pixel's red value.                                 
		TRACER_COLOR_GREEN,						// In:  Pixel's green value.                               
		TRACER_COLOR_BLUE,						// In:  Pixel's blue value.                                
		CONSTANT_COLOR_START_INDEX,			// In:  Min mappable index (affects source).   
		// Do NOT include the top 10 windows colors!
		NUM_CONSTANT_COLOR_INDICES,			// In:  Num mappable indices (affects source). 
		ppal->Red(0),								// In:  Beginning of red color table.                      
		ppal->Green(0),							// In:  Beginning of green color table.                    
		ppal->Blue(0),								// In:  Beginning of blue color table.                     
		ppal->GetPalEntrySize()					// In:  Size to increment between each index in each table.
		);

	}

///////////////////////////////////////////////////////////////////////////////
// Preload any assets that may be used.
// (static).
///////////////////////////////////////////////////////////////////////////////
short CBulletFest::Preload(
	CRealm* prealm)				// In:  Calling realm.
	{
	CAnimThing::ChannelAA*	paaCache;
	short	sResult	= 0;

	if (rspGetResource(&g_resmgrGame, prealm->Make2dResPath(IMPACT_RES_NAME), &paaCache) == 0)
		rspReleaseResource(&g_resmgrGame, &paaCache);
	else
		sResult |= 1;

	if (rspGetResource(&g_resmgrGame, prealm->Make2dResPath(RICOCHET_RES_NAME), &paaCache) == 0)
		rspReleaseResource(&g_resmgrGame, &paaCache);
	else
		sResult |= 1;
		
	if (rspGetResource(&g_resmgrGame, prealm->Make2dResPath(FLARE_RES_NAME), &paaCache) == 0)
		rspReleaseResource(&g_resmgrGame, &paaCache);
	else
		sResult |= 1;

	return sResult;
	}

///////////////////////////////////////////////////////////////////////////////
// EOF
///////////////////////////////////////////////////////////////////////////////
