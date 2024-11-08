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
// fireball.h
// Project: Nostril (aka Postal)
// 
// History:
//		04/25/97 BRH	Started this new weapon to be used to make a 
//							flamethrower.
//
//		04/29/97	JMI	Added GetSprite() virtual override to provide access
//							to m_sprite from a lower level.
//							Changed back to being derived from CWeapon (instead of
//							CThing).
//							Added a Setup() that matches the base class virtual 
//							Setup() to make sure it gets overriden.  The 
//							functionality is the same (the new Setup() just calls 
//							the six parm Setup() with some handy defaults).
//							Changed name of ProcessMessages() to 
//							ProcessFireballMessages() to avoid conflicts with 
//							CWeapon's ProcessMessages().
//
//		06/30/97 BRH	Added a fire stream object in this file to emit several
//							fireballs spaced close together to make a better stream
//							for the flamethrower.
//
//		07/04/97 BRH	Changed to an auto alpha level based on the time to
//							live so that the flame gets more alpha-ed toward the
//							end where it burns out.
//
//		07/09/97	JMI	Changed Preload() to take a pointer to the calling realm
//							as a parameter.
//
//		08/08/97	JMI	Changed m_pFireball1, 2, & 3 to m_idFireball1, 2, & 3.
//
//		08/17/97	JMI	Changed m_pthingParent to m_idParent.
//
//////////////////////////////////////////////////////////////////////////////
//
// Fireball.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef FIREBALL_H
#define FIREBALL_H

#include "RSPiX.h"
#include "realm.h"
#include "AlphaAnimType.h"
#include "smash.h"
#include "weapon.h"


/////////////////////////////////////////// CFireball /////////////////////////////////////

// CFireball is a burning flame weapon class
class CFireball : public CWeapon
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
	} CFireballState;

	typedef RChannel<CAlphaAnim> ChannelAA;

	//---------------------------------------------------------------------------
	// Variables
	//---------------------------------------------------------------------------

	public:
		int16_t m_sSpreadDir;					// A Random number will determine the spread
													// when it hits a wall.

	protected:
		int32_t m_lTimer;							// General purpose timer
		int32_t m_lCollisionTimer;				// Check for collisions when this expires
		int32_t m_lBurnUntil;					// Burn until this time.
		int32_t m_lCurrentAlphaTimeout;		// Use current Alpha until this time, then switch
		int32_t m_lAlphaChannelInterval;		// Show each alpha for this amount of time
		int32_t m_lTimeToLive;					// Total time to show this animation
		int32_t m_lTotalFlameTime;				// Total time to show the flame (non adjusted for game time)
		int16_t m_sCurrentAlphaChannel;		// Use this Alpha channel
		int16_t m_sTotalAlphaChannels;
		U32	m_u32CollideIncludeBits;	// bits to use for collision checking
		U32	m_u32CollideDontcareBits;	// bits to use for collision checking
		U32	m_u32CollideExcludeBits;	// bits to use for collision checking
		U16	m_u16ShooterID;				// shooter's ID so you don't hit him.
		bool	m_bSendMessages;				// Whether or not to send messages to other
													// objects telling them to burn or not.
		bool	m_bMoving;						// Once it hits a wall it will stop moving
		int32_t	m_lAnimTime;					// Animation time so anims can be offset frames.
		int32_t m_lPrevTime;						// Previous update time

		CSprite2		m_sprite;				// 2D sprite to render this object.

		ChannelAA*	m_pAnimChannel;		// Alpha animation stored as a channel.
												
		int16_t m_sSuspend;						// Suspend flag

		CSmash		m_smash;					// Collision class

		// Tracks file counter so we know when to load/save "common" data 
		static int16_t ms_sFileCount;
		static int16_t ms_sSmallRadius;
		static int32_t  ms_lCollisionTime;	// Check for collisions this often
		static double ms_dFireVelocity;	// Rate of travel.

		// "Constant" values that we want to be able to tune using the editor

	//---------------------------------------------------------------------------
	// Constructor(s) / destructor
	//---------------------------------------------------------------------------
	public:
		// Constructor
		CFireball(CRealm* pRealm)
			: CWeapon(pRealm, CFireballID)
			{
			m_sSuspend = 0;
			m_lPrevTime = 0;
			m_bSendMessages = true;
			m_u32CollideIncludeBits = 0;
			m_u32CollideDontcareBits = 0;
			m_u32CollideExcludeBits = 0;
			m_sTotalAlphaChannels = 0;
			m_smash.m_pThing = NULL;
			m_smash.m_bits = 0;
			m_bMoving = true;
			m_lAnimTime = 0;

			m_sprite.m_pthing = this;
			}

	public:
		// Destructor
		~CFireball()
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
		static int16_t Construct(									// Returns 0 if successfull, non-zero otherwise
			CRealm* pRealm,										// In:  Pointer to realm this object belongs to
			CThing** ppNew)										// Out: Pointer to new object
			{
			int16_t sResult = 0;
			*ppNew = new CFireball(pRealm);
			if (*ppNew == 0)
				{
				sResult = -1;
				TRACE("CFireball::Construct(): Couldn't construct CFireball (that's a bad thing)\n");
				}
			return sResult;
			}

	//---------------------------------------------------------------------------
	// Optional static functions
	//---------------------------------------------------------------------------

		// Function for this class that is called before play begins to make
		//	the resource manager cache the resources for this object.
		static int16_t Preload(
			CRealm* prealm);				// In:  Calling realm.

	//---------------------------------------------------------------------------
	// Required virtual functions (implimenting them as inlines doesn't pay!)
	//---------------------------------------------------------------------------
	public:
		// Load object (should call base class version!)
		int16_t Load(													// Returns 0 if successfull, non-zero otherwise
			RFile* pFile,											// In:  File to load from
			bool bEditMode,										// In:  True for edit mode, false otherwise
			int16_t sFileCount,										// In:  File count (unique per file, never 0)
			uint32_t	ulFileVersion);								// In:  Version of file format to load.

		// Save object (should call base class version!)
		int16_t Save(													// Returns 0 if successfull, non-zero otherwise
			RFile* pFile,											// In:  File to save to
			int16_t sFileCount);									// In:  File count (unique per file, never 0)

		// Startup object
		int16_t Startup(void);										// Returns 0 if successfull, non-zero otherwise

		// Shutdown object
		int16_t Shutdown(void);									// Returns 0 if successfull, non-zero otherwise

		// Suspend object
		void Suspend(void);

		// Resume object
		void Resume(void);

		// Update object
		void Update(void);

		// Render object
		void Render(void);

		int16_t Setup(												// Returns 0 on success.
			int16_t sX,												// In: New x coord
			int16_t sY,												// In: New y coord
			int16_t sZ,												// In: New z coord
			int16_t sDir,												// In: Direction of travel
			int32_t lTimeToLive,										// In: Milliseconds to burn
			U16 u16ShooterID);									// In: Your ID (the shooter) so it doesn't hit you

		// Override base class Setup().
		virtual				// Overridden here.
		int16_t Setup(
			int16_t sX,												// In: Starting X position
			int16_t sY,												// In: Starting Y position
			int16_t sZ)												// In: Starting Z position
			{
			// Default to 500 ms to live and parent's shooter ID, if there is a parent.
 			return Setup(
				sX, sY, sZ, 
				0,
				500, 
				m_idParent
				);
			}

		// Called by editor to init new object at specified position
		int16_t EditNew(												// Returns 0 if successfull, non-zero otherwise
			int16_t sX,												// In:  New x coord
			int16_t sY,												// In:  New y coord
			int16_t sZ);												// In:  New z coord

		// Called by editor to modify object
		int16_t EditModify(void);									// Returns 0 if successfull, non-zero otherwise

		// Called by editor to move object to specified position
		int16_t EditMove(											// Returns 0 if successfull, non-zero otherwise
			int16_t sX,												// In:  New x coord
			int16_t sY,												// In:  New y coord
			int16_t sZ);												// In:  New z coord

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

		// Get this class's sprite.  Note that the type will vary.
		// This is a pure virtual functionin the base class.
		virtual			// Overriden here.
		CSprite* GetSprite(void)	// Returns this weapon's sprite.
			{
			return &m_sprite;
			}


	//---------------------------------------------------------------------------
	// Internal functions
	//---------------------------------------------------------------------------
	protected:
		// Get all required resources
		int16_t GetResources(void);						// Returns 0 if successfull, non-zero otherwise
		
		// Free all resources
		int16_t FreeResources(void);						// Returns 0 if successfull, non-zero otherwise

		// Process Game Messages
		CFireballState ProcessFireballMessages(void);

		// Initialize the fire for large or small objects
		int16_t Init(void);

	};


/////////////////////////////////// Firestream ////////////////////////////////

// CFirestream is a burning flame weapon class
class CFirestream : public CWeapon
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
	} CFirestreamState;

	typedef RChannel<CAlphaAnim> ChannelAA;

	//---------------------------------------------------------------------------
	// Variables
	//---------------------------------------------------------------------------

	public:
													// when it hits a wall.

	protected:
		int32_t m_lBurnUntil;					// Burn until this time.
		int32_t m_lCurrentAlphaTimeout;		// Use current Alpha until this time, then switch
		int32_t m_lAlphaChannelInterval;		// Show each alpha for this amount of time
		int32_t m_lTimeToLive;					// Total time to show this animation
		int16_t m_sCurrentAlphaChannel;		// Use this Alpha channel
		int16_t m_sTotalAlphaChannels;
		U16	m_u16ShooterID;				// shooter's ID so you don't hit him.
		bool	m_bSendMessages;				// Whether or not to send messages to other
													// objects telling them to burn or not.
		int32_t m_lPrevTime;						// Previous update time

		CSprite2	m_sprite;					// False sprite for positioning info
		U16 m_idFireball1;
		U16 m_idFireball2;
		U16 m_idFireball3;

												
		int16_t m_sSuspend;						// Suspend flag

		// Tracks file counter so we know when to load/save "common" data 
		static int16_t ms_sFileCount;
		static int16_t ms_sOffset1;		// Offset for 2nd fireball
		static int16_t ms_sOffset2;		// Offset for 3rd fireball

		// "Constant" values that we want to be able to tune using the editor

	//---------------------------------------------------------------------------
	// Constructor(s) / destructor
	//---------------------------------------------------------------------------
	public:
		// Constructor
		CFirestream(CRealm* pRealm)
			: CWeapon(pRealm, CFirestreamID)
			{
			m_sSuspend = 0;
			m_lPrevTime = 0;
			m_bSendMessages = true;
			m_sTotalAlphaChannels = 0;

			m_sprite.m_pthing = this;
			m_idFireball1 = CIdBank::IdNil;
			m_idFireball2 = CIdBank::IdNil;
			m_idFireball3 = CIdBank::IdNil;
			}

	public:
		// Destructor
		~CFirestream()
			{
			// Remove sprite from scene (this is safe even if it was already removed!)
			m_pRealm->m_scene.RemoveSprite(&m_sprite);
			}

	//---------------------------------------------------------------------------
	// Required static functions
	//---------------------------------------------------------------------------
	public:
		// Construct object
		static int16_t Construct(									// Returns 0 if successfull, non-zero otherwise
			CRealm* pRealm,										// In:  Pointer to realm this object belongs to
			CThing** ppNew)										// Out: Pointer to new object
			{
			int16_t sResult = 0;
			*ppNew = new CFirestream(pRealm);
			if (*ppNew == 0)
				{
				sResult = -1;
				TRACE("CFirestream::Construct(): Couldn't construct CFirestream (that's a bad thing)\n");
				}
			return sResult;
			}

	//---------------------------------------------------------------------------
	// Optional static functions
	//---------------------------------------------------------------------------

		// Function for this class that is called before play begins to make
		//	the resource manager cache the resources for this object.
		static int16_t Preload(
			CRealm* prealm);				// In:  Calling realm.

	//---------------------------------------------------------------------------
	// Required virtual functions (implimenting them as inlines doesn't pay!)
	//---------------------------------------------------------------------------
	public:
		// Load object (should call base class version!)
		int16_t Load(													// Returns 0 if successfull, non-zero otherwise
			RFile* pFile,											// In:  File to load from
			bool bEditMode,										// In:  True for edit mode, false otherwise
			int16_t sFileCount,										// In:  File count (unique per file, never 0)
			uint32_t	ulFileVersion);								// In:  Version of file format to load.

		// Save object (should call base class version!)
		int16_t Save(													// Returns 0 if successfull, non-zero otherwise
			RFile* pFile,											// In:  File to save to
			int16_t sFileCount);									// In:  File count (unique per file, never 0)

		// Startup object
		int16_t Startup(void);										// Returns 0 if successfull, non-zero otherwise

		// Shutdown object
		int16_t Shutdown(void);									// Returns 0 if successfull, non-zero otherwise

		// Suspend object
		void Suspend(void);

		// Resume object
		void Resume(void);

		// Update object
		void Update(void);

		// Render object
		void Render(void);

		int16_t Setup(												// Returns 0 on success.
			int16_t sX,												// In: New x coord
			int16_t sY,												// In: New y coord
			int16_t sZ,												// In: New z coord
			int16_t sDir,												// In: Direction of travel
			int32_t lTimeToLive,										// In: Milliseconds to burn
			U16 u16ShooterID);									// In: Your ID (the shooter) so it doesn't hit you

		// Override base class Setup().
		virtual				// Overridden here.
		int16_t Setup(
			int16_t sX,												// In: Starting X position
			int16_t sY,												// In: Starting Y position
			int16_t sZ)												// In: Starting Z position
			{
			// Default to 500 ms to live and parent's shooter ID, if there is a parent.
			return Setup(
				sX, sY, sZ, 
				0,
				500, 
				m_idParent
				);
			}

		// Called by editor to init new object at specified position
		int16_t EditNew(												// Returns 0 if successfull, non-zero otherwise
			int16_t sX,												// In:  New x coord
			int16_t sY,												// In:  New y coord
			int16_t sZ);												// In:  New z coord

		// Called by editor to modify object
		int16_t EditModify(void);									// Returns 0 if successfull, non-zero otherwise

		// Called by editor to move object to specified position
		int16_t EditMove(											// Returns 0 if successfull, non-zero otherwise
			int16_t sX,												// In:  New x coord
			int16_t sY,												// In:  New y coord
			int16_t sZ);												// In:  New z coord

		// Called by editor to update object
		void EditUpdate(void);

		// Called by editor to render object
		void EditRender(void);


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

		// Get this class's sprite.  Note that the type will vary.
		// This is a pure virtual functionin the base class.
		virtual			// Overriden here.
		CSprite* GetSprite(void)	// Returns this weapon's sprite.
			{
			return &m_sprite;
			}


	//---------------------------------------------------------------------------
	// Internal functions
	//---------------------------------------------------------------------------
	protected:

		// Process Game Messages
		CFirestreamState ProcessFireballMessages(void);

		// Initialize the fire for large or small objects
		int16_t Init(void);

	};


#endif //FIREBALL_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
