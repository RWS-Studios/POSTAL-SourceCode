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
// grenade.h
// Project: Postal
//
//	History:
//		01/17/97 BRH	Started this weapon object.
//
//		02/11/97	JMI	Stripped 2D.  Added 3D and a concept of having a parent.
//
//		02/21/97	JMI	Made static constants public.
//
//		02/26/97	JMI	Now sets m_sprite.m_pthing = this on construction.
//
//		02/28/97 BRH	Derived this from the CWeapon base class and moved
//							many members and some functions to the base class.
//
//		03/03/97 BRH	Moved 3D sprite to CWeapon base class.
//
//		03/03/97	JMI	Commented out dHorizVelocity and dVertVelocity parameters
//							to Setup() so that this version would be a virtual over-
//							ride of CWeapon's.
//
//		03/13/97	JMI	Load now takes a version number.
//
//		04/29/97	JMI	Now defines m_sprite (as a CSprite3), which was previously
//							defined in the base class CWeapon.
//							Also, added GetSprite() virtual override to provide access
//							to the sprite from a lower level.
//
//		05/09/97 BRH	Added SetRangeToTarget function to vary the velocity
//							of the weapon before it is shot in order to hit
//							your target.  
//
//		06/03/97 BRH	Tuned the SetRangeToTarget function now that the drag
//							has changed for the grenade.
//
//		06/17/97 BRH	Increased Min distance in SetRangeToTarget so that enemies
//							won't blow themselves up.  Fixed a bug in the SetRange
//							function as well.
//
//		07/09/97	JMI	Changed Preload() to take a pointer to the calling realm
//							as a parameter.
//
//		08/08/97	JMI	Added Style so this can be visually represented in multiple
//							ways.
//
//		08/08/97	JMI	Added variables for rotating the weapon.
//
//		08/28/97 BRH	Tuned distance for dynamite.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef GRENADE_H
#define GRENADE_H

#include "RSPiX.h"
#include "realm.h"
#include "weapon.h"


// CGrenade is an unguided missile weapon class
class CGrenade : public CWeapon
	{
	//---------------------------------------------------------------------------
	// Types, enums, etc.
	//---------------------------------------------------------------------------
	public:

		typedef enum
			{
			Grenade,
			Dynamite,

			NumStyles
			} Style;

	//---------------------------------------------------------------------------
	// Variables
	//---------------------------------------------------------------------------
	public:

	protected:
		short m_sPrevHeight;										// Previous height

		CAnim3D		m_anim;										// 3D animation.  One should be enough, I think.
		RTransform	m_trans;										// Transform.
		CSprite3		m_sprite;									// 3D sprite to render this thing.
		Style			m_style;										// Style of thrown weapon.

		double		m_dAnimRotY;								// Apparent rotation around Y axis.
		double		m_dAnimRotZ;								// Apparent rotation around X axis.
		double		m_dAnimRotVelY;							// Rate of apparent rotation around Y axis.
		double		m_dAnimRotVelZ;							// Rate of apparent rotation around X axis.

		long			m_lNextSmokeTime;							// Time of dispersion of next smoke.

		// Tracks file counter so we know when to load/save "common" data 
		static short ms_sFileCount;

	public:
		// "Constant" values that we want to be able to tune using the editor
		static double ms_dAccUser;								// Acceleration due to user
		static double ms_dAccDrag;								// Acceleration due to drag (always towards 0)

		static double ms_dMaxVelFore;							// Maximum forward velocity
		static double ms_dMaxVelBack;							// Maximum backward velocity

		static double ms_dDegPerSec;							// Degrees of rotation per second
		static double ms_dCloseDistance;						// Close enough to hit CDude
		static long ms_lRandomAvoidTime;						// Time to wander before looking again
		static long ms_lReseekTime;							// Time to wait before doing the next 'find'
		static long ms_lGrenadeFuseTime;						// Time from throw to blow
		static long ms_lSmokeInterval;						// Time between smokes.
		static double ms_dGravity;								// Acceleration due to gravity
		static double ms_dThrowVertVel;						// Throw up at this velocity
		static double ms_dThrowHorizVel;						// Throw out at this velocity
		static double ms_dMinBounceVel;						// Will bounce up if velocity is this high
		static double ms_dBounceTransferFract;				// Amount of velocity transferred in bounce
		static double ms_adGroundDimisher[NumStyles];	// Dimishes velocities once it hits the ground.
		static double ms_adBounceDimisher[NumStyles];	// Dimishes velocities when bouncing.

		static char*	ms_apszResNames[NumStyles];		// Res names indexed Style.

	//---------------------------------------------------------------------------
	// Constructor(s) / destructor
	//---------------------------------------------------------------------------
	protected:
		// Constructor
		CGrenade(CRealm* pRealm)
			: CWeapon(pRealm, CGrenadeID)
			{
			m_sprite.m_pthing	= this;
			m_style				= Grenade;
			m_dAnimRotY			= 0.0;	
			m_dAnimRotZ			= 0.0;	
			m_dAnimRotVelY		= 0.0;
			m_dAnimRotVelZ		= 0.0;
			m_lNextSmokeTime	= 0;
			}

	public:
		// Destructor
		~CGrenade()
			{
			// Remove sprite from scene (this is safe even if it was already removed!)
			m_pRealm->m_scene.RemoveSprite(&m_sprite);

			// Free resources
			FreeResources();
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
			*ppNew = new CGrenade(pRealm);
			if (*ppNew == 0)
				{
				sResult = -1;
				TRACE("CGrenade::Construct(): Couldn't construct CGrenade (that's a bad thing)\n");
				}
			return sResult;
			}

		// Construct object as dynamite.
		static short ConstructDynamite(						// Returns 0 if successfull, non-zero otherwise
			CRealm* pRealm,										// In:  Pointer to realm this object belongs to
			CThing** ppNew)										// Out: Pointer to new object
			{
			short	sRes	= Construct(pRealm, ppNew);
			if (sRes == 0)
				{
				( (CGrenade*)(*ppNew) )->m_style	= Dynamite;
				}

			return sRes;
			}

	//---------------------------------------------------------------------------
	// Optional static functions
	//---------------------------------------------------------------------------

		// Called before play begins to cache resources for this object.
		static short Preload(
			CRealm* prealm);				// In:  Calling realm.

	//---------------------------------------------------------------------------
	// Required virtual functions (implimenting them as inlines doesn't pay!)
	//---------------------------------------------------------------------------
	public:
		// Load object (should call base class version!)
		short Load(													// Returns 0 if successfull, non-zero otherwise
			RFile* pFile,											// In:  File to load from
			bool bEditMode,										// In:  True for edit mode, false otherwise
			short sFileCount,										// In:  File count (unique per file, never 0)
			ULONG	ulFileVersion);								// In:  Version of file format to load.

		// Save object (should call base class version!)
		short Save(													// Returns 0 if successfull, non-zero otherwise
			RFile* pFile,											// In:  File to save to
			short sFileCount);									// In:  File count (unique per file, never 0)

		// Update object
		void Update(void);

		// Render object
		void Render(void);

		// Called by the object that is creating this weapon
		short Setup(
			short sX,												// In: New x coord
			short sY,												// In: New y coord
			short sZ/*,												// In: New z coord
			double dHorizVelocity = ms_dThrowHorizVel,	// In: Starting horiz velocity with default
			double dVertVelocity = ms_dThrowVertVel*/);	// In: Starting vert velocity with default

		// Get this class's sprite.  Note that the type will vary.
		// This is a pure virtual functionin the base class.
		virtual			// Overriden here.
		CSprite* GetSprite(void)	// Returns this weapon's sprite.
			{
			return &m_sprite;
			}

		// Function to modify the velocity for a requested range
		virtual short SetRangeToTarget(short sRequestedRange)
		{
			short sSetRange;
			// Must go at least 40 or at most 400 pixels
			sSetRange = MAX(sRequestedRange, (short) 40);
			sSetRange = MIN(sSetRange, (short) 400);
			if (m_style == Grenade)
				m_dHorizVel = (double) sSetRange / 0.8476; //0.6855;
			else
				m_dHorizVel = (double) sSetRange / 1.010;
			return sSetRange;
		}

		// Smoke, if correct time.
		void Smoke(void);

	//---------------------------------------------------------------------------
	// Internal functions
	//---------------------------------------------------------------------------
	protected:
		// Get all required resources
		short GetResources(void);						// Returns 0 if successfull, non-zero otherwise
		
		// Free all resources
		short FreeResources(void);						// Returns 0 if successfull, non-zero otherwise

		// Process Message queue
		void ProcessMessages(void);
	};


#endif // GRENADE_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
