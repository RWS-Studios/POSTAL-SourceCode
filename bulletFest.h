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
// bulletFest.H
// Project: Nostril (aka Postal)
// 
// History:
//		02/18/97 JMI	Started.
//
//		02/19/97	JMI	Now Fire() and FireDeluxe() take all 3 CSmash masks.
//
//		03/10/97	JMI	Added m_u16IdTarget (last known target's ID).
//
//		03/21/97	JMI	Added optional tracers.
//
//		04/02/97	JMI	Changed ms_ati to m_ati.
//							Also, added ms_u8TracerIndex and m_sCurTracerPos.
//
//		04/24/97	JMI	Added smid parameter specifying type of ammo noise to
//							FireDeluxe() and Flare().
//							Also, added vertical bullet angle to FireDeluxe() and
//							Fire().
//
//		06/11/97	JMI	Added Preload() for loading assets used during play.
//
//		07/09/97	JMI	Changed Preload() to take a pointer to the calling realm
//							as a parameter.
//
//		09/18/97	JMI	UpdateTracerColor() now takes a realm ptr.
//
//////////////////////////////////////////////////////////////////////////////
//
// bulletFest handles launching of bullets.  Currently it doesn't seem like
// much more than a function call, but eventually it will need to keep track
// of some per instance data.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BULLETFEST_H
#define BULLETFEST_H

//////////////////////////////////////////////////////////////////////////////
// C Headers -- Must be included before RSPiX.h b/c RSPiX utilizes SHMalloc.
//////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// RSPiX Headers.
// If PATHS_IN_INCLUDES macro is defined, we can utilize relative
// paths to a header file.  In this case we generally go off of our
// RSPiX root directory.  System.h MUST be included before this macro
// is evaluated.  System.h is the header that, based on the current
// platform (or more so in this case on the compiler), defines 
// PATHS_IN_INCLUDES.  Blue.h includes system.h so you can include that
// instead.
///////////////////////////////////////////////////////////////////////////////
#include "System.h"

#ifdef PATHS_IN_INCLUDES

#else

#endif

//////////////////////////////////////////////////////////////////////////////
// Postal headers.
//////////////////////////////////////////////////////////////////////////////
#include "realm.h"
#include "SampleMaster.h"

//////////////////////////////////////////////////////////////////////////////
// Macros.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Protos.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Typedefs.
//////////////////////////////////////////////////////////////////////////////
class CBulletFest
	{
	///////////////////////////////////////////////////////////////////////////
	// Typedefs/enums.
	///////////////////////////////////////////////////////////////////////////
	public:
		// Macros.
		enum
			{
			TargetHistoryTotalPeriod	= 1000,	// Target history duration in ms.
			TargetHistoryUpdatePeriod	= 100,	// Duration between history 
															// updates.
			TargetUpdatesPerPeriod	= TargetHistoryTotalPeriod / TargetHistoryUpdatePeriod
			};

		// History info.
		typedef struct
			{
			bool	bDirChange;		// true, if a direction change (relative to 
										// source) occured at this point; false, 
										// otherwise.
			long	lSqrDistance;	// Squared distance traveled (relative to 
										// source) at this point in time.
			} TargetInfo;

	///////////////////////////////////////////////////////////////////////////
	// Con/Destruction.
	///////////////////////////////////////////////////////////////////////////
	public:
		CBulletFest()
			{
			m_u16IdTarget		= CIdBank::IdNil;
			m_sCurTracerPos	= 0;
			}

		~CBulletFest() 
			{
			}

	///////////////////////////////////////////////////////////////////////////
	// Methods.
	///////////////////////////////////////////////////////////////////////////
	public:

		// Update current targeting information.  This will function will
		// continually track the position of a target in order to determine
		// how much it is moving.  The idea being that targets that move 
		// irratically(sp?) and more often are harder to hit.
		void UpdateTarget(	// Returns nothing.
			short sAngle,		// In:  Angle of aim in degrees (on X/Z plane).
			short	sX,			// In:  Aim position.
			short	sY,			// In:  Aim position.
			short	sZ,			// In:  Aim position.
			CRealm* pRealm);	// In:  Realm in which to target.

		// Launch a bullet, create a muzzle flare at the src (sX, sY, sZ), and,
		// if no CThing hit, create a ricochet or impact where the bullet 
		// contacted terrain (*psX, *psY, *psZ).
		// This same process can be done in pieces with more control to the user
		// by calling Fire(), Flare(), Ricochet(), and Impact().
		bool FireDeluxe(					// Returns what and as Fire() would.
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
			bool	bTracer = true,		// In:  Draw a tracer at random point along path.
			SampleMasterID	smid	= g_smidBulletFire);	// In:  Use ammo sample.


		// Launch a bullet.
		bool Fire(							// Returns true if a hit, false otherwise.
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
			bool	bTracer = true);		// In:  Draw a tracer at random point along path.

		// Create a muzzle flare effect.
		void Flare(						// Returns nothing.
			short sAngle,				// In:  Angle of launch in degrees (on X/Z plane).
			short	sX,					// In:  Launch position.
			short	sY,					// In:  Launch position.
			short	sZ,					// In:  Launch position.
			CRealm* pRealm,			// In:  Realm in which to fire.
			SampleMasterID	smid	= g_smidBulletFire);	// In:  Use ammo sample.

		// Create a impact effect.
		void Impact(				// Returns nothing.
			short sAngle,			// In:  Angle of launch in degrees (on X/Z plane).
			short	sX,				// In:  Launch position.
			short	sY,				// In:  Launch position.
			short	sZ,				// In:  Launch position.
			CRealm* pRealm);		// In:  Realm in which to fire.

		// Create a ricochet effect.
		void Ricochet(				// Returns nothing.
			short sAngle,			// In:  Angle of launch in degrees (on X/Z plane).
			short	sX,				// In:  Launch position.
			short	sY,				// In:  Launch position.
			short	sZ,				// In:  Launch position.
			CRealm* pRealm);		// In:  Realm in which to fire.

		// Updates the static tracer color.
		static void UpdateTracerColor(
			CRealm* prealm);				// In:  Calling realm.

		// Preload any assets that may be used.
		static short Preload(
			CRealm* prealm);				// In:  Calling realm.

	///////////////////////////////////////////////////////////////////////////
	// Querries.
	///////////////////////////////////////////////////////////////////////////
	public:


	///////////////////////////////////////////////////////////////////////////
	// Instantiable data.
	///////////////////////////////////////////////////////////////////////////
	public:

		short	m_sCurTracerPos;	// Cummulative tracer position to give the look
										// of 'forward' movement.

		// Target info.  ***NYI***
		U16	m_u16IdTarget;		// Last known target or IdNil.
		short	m_sDirChanges;		// Direction changes (relative to source) over 
										// last targeting duration.
		long	m_lSqrDistance;	// Squared distance traveled (relative to 
										// source) over last targeting duration.
		RQueue<TargetInfo, TargetUpdatesPerPeriod>	m_qtiHistory;
		TargetInfo	m_ati;		// Array of target info used for history queue.

	///////////////////////////////////////////////////////////////////////////
	// Static data.
	///////////////////////////////////////////////////////////////////////////
		static U8	ms_u8TracerIndex;	// The color index to use for tracers.
												// This value is gotten only once per
												// execution of this program.

	public:
	};

#endif	// BULLETFEST_H
//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
