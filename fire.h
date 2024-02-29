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
// fire.h
// Project: Postal
// 
// History:
//		01/17/97 BRH	Started this weapon object.
//
//		02/24/97	JMI	Changed declaration of m_sprite from CAlphaSprite2 to 
//							CSprite2.
//
//		03/13/97	JMI	Load now takes a version number.
//
//		04/24/97 BRH	Added a static variable for wind direction that will
//							be changed slightly every time a smoke is created, but
//							will generally cause them all to drift in the same
//							direction.
//
//		05/02/97	JMI	Added GetTimeLeftToLive() which returns the amount of
//							time left before the fire goes out or the smoke thins
//							out.
//
//		06/11/97 BRH	Added m_u16ShooterID to store the shooter ID which
//							will get passed along in the Burn Message.
//
//		06/17/97	JMI	Converted all occurrences of rand() to GetRand() and
//							srand() to SeedRand().
//
//		07/01/97 BRH	Added Small smoke animation.
//
//		07/04/97 BRH	Added starting time used to calculate the alpha
//							level based on time to live.
//
//		07/09/97	JMI	Changed Preload() to take a pointer to the calling realm
//							as a parameter.
//
//		07/13/97 BRH	Added some variables and changed others for the new
//							method of using 1 alpha mask and setting the level
//							in the code.
//
//		07/23/97 BRH	Changed the limits on the wind velocity so it can
//							be higher.
//
//		09/02/97	JMI	Added m_u16FireStarterID.  This is used for a special case
//							when the starter of the fire is not the thing using the
//							fire as a weapon (e.g., when a guy catches fire he can
//							use the fire on other people by running into them causing
//							them to catch on fire; however, if his own fire kills him
//							it is to the creator of the fire's credit that he dies).
//
//////////////////////////////////////////////////////////////////////////////
//
// Fire.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef FIRE_H
#define FIRE_H

#include "RSPiX.h"
#include "realm.h"
#include "AlphaAnimType.h"
#include "smash.h"


// CFire is a burning flame weapon class
class CFire : public CThing
	{
	//---------------------------------------------------------------------------
	// Types, enums, etc.
	//---------------------------------------------------------------------------
	public:

	typedef enum
	{
		State_Idle,
		State_Fire,
		State_Find,
		State_Chase,
		State_Explode,
		State_Deleted
	} CFireState;

	typedef unsigned char FireAnim;

	typedef enum
	{
		LargeFire,
		SmallFire,
		Smoke,
		SmallSmoke
	};

	typedef RChannel<CAlphaAnim> ChannelAA;

	//---------------------------------------------------------------------------
	// Variables
	//---------------------------------------------------------------------------

	public:
		double m_dX;
		double m_dY;
		double m_dZ;
		short m_sRot;
		bool  m_bTurnRight;					// A Random number will determine if the 
													// smoke will curl left or right when it hits an
													// obstacle.
        bool m_bIsBurningDude;

		U16	m_u16ShooterID;				// Store the shooter ID to pass along in the burn message
		U16	m_u16FireStarterID;			// Fire's creator.  The ID of the thing that
													// caused this fire to be created.  Generally
													// used by a thing3d when creating an internal
													// fire in response to Burn messages.


	protected:
		long m_lTimer;							// General purpose timer
		long m_lCollisionTimer;				// Check for collisions when this expires
		long m_lBurnUntil;					// Burn until this time.
		long m_lCurrentAlphaTimeout;		// Use current Alpha until this time, then switch
		long m_lBrightAlphaInterval;		// Show each alpha for this amount of time
		long m_lDimAlphaInterval;			// Show dim alpha level for this amount of time
		long m_lTimeToLive;					// Total time to show this animation
		long m_lAlphaBreakPoint;			// Time to switch from Bright to Dim
		long m_lStartTime;					// Starting time used to calc the Alpha %
		short m_sCurrentAlphaLevel;		// Use this Alpha level
		short m_sTotalAlphaChannels;
		U32	m_u32CollideIncludeBits;	// bits to use for collision checking
		U32	m_u32CollideDontcareBits;	// bits to use for collision checking
		U32	m_u32CollideExcludeBits;	// bits to use for collision checking
		bool	m_bSendMessages;				// Whether or not to send messages to other
													// objects telling them to burn or not.
		FireAnim m_eFireAnim;				// Which animation to use for the fire		

		long m_lPrevTime;						// Previous update time

		CSprite2		m_sprite;				// Sprite (replace with CSprite3, soon)

		ChannelAA*	m_pAnimChannel;		// Alpha animation stored as a channel.
												
		short m_sSuspend;						// Suspend flag

		CSmash		m_smash;					// Collision class

		// Tracks file counter so we know when to load/save "common" data 
		static short ms_sFileCount;
		static short ms_sLargeRadius;
		static short ms_sSmallRadius;
		static long  ms_lCollisionTime;	// Check for collisions this often
		static long  ms_lSmokeTime;		// Time to let smoke run
		static short ms_sWindDirection;	// Direction the wind is blowing, will
													// get changed slightly by each new smoke.
		static double ms_dWindVelocity;	// Smoke drift velocity

		// "Constant" values that we want to be able to tune using the editor

	//---------------------------------------------------------------------------
	// Constructor(s) / destructor
	//---------------------------------------------------------------------------
	public:
		// Constructor
		CFire(CRealm* pRealm)
			: CThing(pRealm, CFireID)
			{
			m_bIsBurningDude = false;
			m_sSuspend = 0;
			m_lPrevTime = 0;
			m_bSendMessages = true;
			m_u32CollideIncludeBits = 0;
			m_u32CollideDontcareBits = 0;
			m_u32CollideExcludeBits = 0;
			m_sTotalAlphaChannels = 0;
			m_smash.m_pThing = NULL;
			m_smash.m_bits = 0;
			m_lStartTime = 0;
			m_u16FireStarterID = CIdBank::IdNil;
			}

	public:
		// Destructor
		~CFire()
			{
			// Remove sprite from scene (this is safe even if it was already removed!)
			m_pRealm->m_scene.RemoveSprite(&m_sprite);
			// Remove yourself from the collision list if it was in use
			// (switching to smoke removes it from the smashatorium and sets
			// the m_pThing field to NULL)
			if (m_smash.m_pThing)
				m_pRealm->m_smashatorium.Remove(&m_smash);

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
			*ppNew = new CFire(pRealm);
			if (*ppNew == 0)
				{
				sResult = -1;
				TRACE("CFire::Construct(): Couldn't construct CFire (that's a bad thing)\n");
				}
			return sResult;
			}

	//---------------------------------------------------------------------------
	// Optional static functions
	//---------------------------------------------------------------------------

		// Function for this class that is called before play begins to make
		//	the resource manager cache the resources for this object.
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

		// Startup object
		short Startup(void);										// Returns 0 if successfull, non-zero otherwise

		// Shutdown object
		short Shutdown(void);									// Returns 0 if successfull, non-zero otherwise

		// Suspend object
		void Suspend(void);

		// Resume object
		void Resume(void);

		// Update object
		void Update(void);

		// Render object
		void Render(void);

		short Setup(												// Returns 0 on success.
			short sX,												// In: New x coord
			short sY,												// In: New y coord
			short sZ,												// In: New z coord
			long lTimeToLive = 1000,							// In: Milliseconds to burn
			bool bThick = true,									// In: Use thick fire (more opaque)
			FireAnim eFireAnim = LargeFire);					// In: Which anim to use

		// Called by editor to init new object at specified position
		short EditNew(												// Returns 0 if successfull, non-zero otherwise
			short sX,												// In:  New x coord
			short sY,												// In:  New y coord
			short sZ);												// In:  New z coord

		// Called by editor to modify object
		short EditModify(void);									// Returns 0 if successfull, non-zero otherwise

		// Called by editor to move object to specified position
		short EditMove(											// Returns 0 if successfull, non-zero otherwise
			short sX,												// In:  New x coord
			short sY,												// In:  New y coord
			short sZ);												// In:  New z coord

		// Called by editor to update object
		void EditUpdate(void);

		// Called by editor to render object
		void EditRender(void);

		// Allows whoever creates the fire to control what gets burned by it
		// the defaults are set initially to Characters
		void SetCollideBits(U32 u32Include, U32 u32Dontcare, U32 u32Exclude)
		{
			m_u32CollideIncludeBits = u32Include;
			m_u32CollideDontcareBits = u32Dontcare;
			m_u32CollideExcludeBits = u32Exclude;
		};

		// Turns messages on which will send burn messages to things the fire
		// is touching.
		void MessagesOn(void)
		{
			m_bSendMessages = true;
		}

		// Turns messages off which allows for fire that is just a visual effect
		void MessagesOff(void)
		{
			m_bSendMessages = false;
		}

		inline short IsBurning(void)
		{
			return m_eFireAnim != Smoke && m_eFireAnim != SmallSmoke;
		}

		// Get the time left to live.
		// Check IsBurning() to determine whether this applies to the fire or
		// the smoke.
		long GetTimeLeftToLive(void)
			{
			return m_lBurnUntil - m_pRealm->m_time.GetGameTime();
			}

	//---------------------------------------------------------------------------
	// Internal functions
	//---------------------------------------------------------------------------
	protected:
		// Get all required resources
		short GetResources(void);						// Returns 0 if successfull, non-zero otherwise
		
		// Free all resources
		short FreeResources(void);						// Returns 0 if successfull, non-zero otherwise

		// Process Game Messages
		CFire::CFireState ProcessMessages(void);

		// Initialize the fire for large or small objects
		short Init(void);

		// Fire it out, let the smoke start.  This funciton will change animations,
		// remove from the smash so it won't collide with anything, reset the timers, etc.
		short Smokeout(void);

		inline void WindDirectionUpdate(void)
		{
			ms_sWindDirection = rspMod360(ms_sWindDirection -2 + (GetRand() % 4)); // Biased 1 direction
			ms_dWindVelocity = ms_dWindVelocity - 0.1 + ((GetRand() % 3) / 10.0);
			if (ms_dWindVelocity < 10.0)
				ms_dWindVelocity = 11.0;
			if (ms_dWindVelocity > 40.0)
				ms_dWindVelocity = 39.0;
		}
	};


#endif //DOOFUS_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
