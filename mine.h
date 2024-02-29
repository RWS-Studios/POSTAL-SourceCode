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
// mine.h
// Project: Postal
//
//	History:
//		03/19/97 BRH	Started this weapon object.
//
//		04/29/97	JMI	Added GetSprite() virtual override to provide access
//							to m_sprite from a lower level.
//							Replaced Setup() with default parm eType = Proximity with
//							a Setup() that matches the base class virtual Setup() to
//							make sure it gets overriden.  The functionality is the
//							same (the new Setup() just calls the four parm Setup()
//							with Proximity as the type).
//
//		04/30/97	JMI	Changed the Setup() override of the CWeapon's Setup() to
//							pass the current mine type to the Setup() with eType.
//							Changed Construct() to take an ID as a parameter and added
//							ConstructProximity(), ConstructTimed(), 
//							ConstructBouncingBetty(), and ConstructRemoteControl() to 
//							allocate that type of mine.
//							Removed m_eMineType (now uses Class ID instead).
//							Removed Setup() that took an eType.
//							Fixed EditRect() and added EditHotSpot().
//
//		06/12/97 BRH	Initialized the Shooter ID to IdNil for mines that
//							are placed in the level, and not placed by a CDude.
//
//		06/27/97	JMI	Modified EditRect() to use Map3Dto2D().
//
//		07/09/97	JMI	Changed Preload() to take a pointer to the calling realm
//							as a parameter.
//
//		07/21/97	JMI	Now handles delete messages.
//
//		08/16/97 BRH	Added a sound handle so that we could have a looping 
//							arming sound that could be stopped when the mine was
//							armed.
//
//		08/17/97	JMI	Destructor now stops looping the arming sound, if it is
//							still running.
//
//		08/28/97 BRH	Added preload function to load the sounds and images.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef MINE_H
#define MINE_H

#include "RSPiX.h"
#include "realm.h"
#include "weapon.h"
#include "bulletFest.h"


// CMine is an unguided missile weapon class
class CMine : public CWeapon
	{
	//---------------------------------------------------------------------------
	// Types, enums, etc.
	//---------------------------------------------------------------------------
	public:

	typedef unsigned char MineType;

	typedef enum
	{
		ProximityMine = 3,
		TimedMine,
		BouncingBettyMine,
		RemoteControlMine,
		NumMineTypes
	};

	//---------------------------------------------------------------------------
	// Variables
	//---------------------------------------------------------------------------
	public:

	protected:
		short m_sPrevHeight;							// Previous height

		RImage*		m_pImage;						// Pointer to mine image
		CSprite2		m_sprite;						// Sprite for 2D mine
		CSmash		m_smash;							// Collision object
		CBulletFest	m_bulletfest;					// Used for bouncing betty
		double		m_dVertVel;						// Vertical velocity 
		double		m_dVertDeltaVel;				// Change in vertical velocity
		long			m_lFuseTime;					// Time before timed mine goes off
		SampleMaster::SoundInstance m_siMineBeep;// Arming beep sound that loops
		// Tracks file counter so we know when to load/save "common" data 
		static short ms_sFileCount;

	public:
		// "Constant" values that we want to be able to tune using the editor
		static short ms_sProximityRadius;		// Distance at which mine goes off
		static short ms_sBettyRadius;				// Distance at which mine goes off
		static short ms_sBettyRange;				// Affected area for Bouncing Betty
		static long ms_lFuseTime;					// Timed mine explodes after this time
		static long ms_lArmingTime;				// Proximity mines arm after this time
		static long ms_lExplosionDelay;			// Delay before explosion triggers mine
		static double ms_dInitialBounceVelocity;//Bouncing Betty popup velocity


	//---------------------------------------------------------------------------
	// Constructor(s) / destructor
	//---------------------------------------------------------------------------
	protected:
		// Constructor
		CMine(CRealm* pRealm)
			: CWeapon(pRealm, CProximityMineID)
			{
			Reset();
			}

		// Constructor
		CMine(CRealm* pRealm, ClassIDType id)
			: CWeapon(pRealm, id)
			{

			Reset();
			}

	public:
		// Destructor
		~CMine()
			{
			// Stop sound, if any.
			StopLoopingSample(m_siMineBeep);

			// Remove sprite from scene (this is safe even if it was already removed!)
			m_pRealm->m_scene.RemoveSprite(&m_sprite);
			m_pRealm->m_smashatorium.Remove(&m_smash);

			// Free resources
			FreeResources();
			}

	//---------------------------------------------------------------------------
	// Required static functions
	//---------------------------------------------------------------------------
	public:
		// Construct mine object.
		static short Construct(									// Returns 0 if successfull, non-zero otherwise
			CRealm* pRealm,										// In:  Pointer to realm this object belongs to
			CThing** ppNew,										// Out: Pointer to new object
			ClassIDType id)										// In:  ID of mine to construct.
			{
			short sResult = 0;
			*ppNew = new CMine(pRealm, id);
			if (*ppNew == 0)
				{
				sResult = -1;
				TRACE("CMine::Construct(): Couldn't construct CMine (that's a bad thing)\n");
				}
			return sResult;
			}

		// Construct proximity mine object.
		static short ConstructProximity(						// Returns 0 if successfull, non-zero otherwise
			CRealm* pRealm,										// In:  Pointer to realm this object belongs to
			CThing** ppNew)										// Out: Pointer to new object
			{
			return Construct(pRealm, ppNew, CProximityMineID);
			}

		// Construct timed mine object.
		static short ConstructTimed(							// Returns 0 if successfull, non-zero otherwise
			CRealm* pRealm,										// In:  Pointer to realm this object belongs to
			CThing** ppNew)										// Out: Pointer to new object
			{
			return Construct(pRealm, ppNew, CTimedMineID);
			}

		// Construct bouncing betty mine object.
		static short ConstructBouncingBetty(				// Returns 0 if successfull, non-zero otherwise
			CRealm* pRealm,										// In:  Pointer to realm this object belongs to
			CThing** ppNew)										// Out: Pointer to new object
			{
			return Construct(pRealm, ppNew, CBouncingBettyMineID);
			}

		// Construct remote control mine object.
		static short ConstructRemoteControl(				// Returns 0 if successfull, non-zero otherwise
			CRealm* pRealm,										// In:  Pointer to realm this object belongs to
			CThing** ppNew)										// Out: Pointer to new object
			{
			return Construct(pRealm, ppNew, CRemoteControlMineID);
			}

	//---------------------------------------------------------------------------
	// Internal functions
	//---------------------------------------------------------------------------
	public:
		// Resets members.
		void Reset(void)
			{
			m_pImage = NULL;
			m_sprite.m_pthing	= this;
			m_lFuseTime = 0;
			m_u16ShooterID = CIdBank::IdNil;
			m_siMineBeep = 0;
			}

		// Called after load to start the object
		short Startup(void);

		// Init - common initialization code for startup, setup & edit new
		short Init(void);

		// Puts up a dialog box in the editor to select mine type
		short EditModify(void);

		// Sets up new item in the editor
		short EditNew(short sX, short sY, short sZ);

		void EditRect(RRect* pRect)
		{
			if (m_pImage)
			{
				// Map from 3d to 2d coords
				Map3Dto2D(
					(short) m_dX, 
					(short) m_dY, 
					(short) m_dZ, 
					&(pRect->sX), 
					&(pRect->sY) );

				// Center on image.
				pRect->sX	-= m_pImage->m_sWidth / 2;
				pRect->sY	-= m_pImage->m_sHeight / 2;
				pRect->sW	= m_pImage->m_sWidth;
				pRect->sH	= m_pImage->m_sHeight;
			}
		}

		void EditHotSpot(			// Returns nothiing.
			short*	psX,			// Out: X coord of 2D hotspot relative to
										// EditRect() pos.
			short*	psY)			// Out: Y coord of 2D hotspot relative to
										// EditRect() pos.
			{
			if (m_pImage)
				{
				*psX	= m_pImage->m_sWidth / 2;
				*psY	= m_pImage->m_sHeight / 2;
				}
			else
				{
				CWeapon::EditHotSpot(psX, psY);
				}
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

		// Called by the object that is creating this weapon - this
		// overloaded version is for timed mines so that the fuse time
		// can be set
		short Setup(
			short sX,												// In: New x coord
			short sY,												// In: New y coord
			short sZ,												// In: New z coord
			long lFuseTime);										// In: Time in ms for fuse

		// Override base class Setup().
		virtual				// Overridden here.
		short Setup(
			short sX,												// In: Starting X position
			short sY,												// In: Starting Y position
			short sZ);												// In: Starting Z position

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
		short GetResources(void);						// Returns 0 if successfull, non-zero otherwise
		
		// Free all resources
		short FreeResources(void);						// Returns 0 if successfull, non-zero otherwise

		// Handle Explosion message
		void OnExplosionMsg(Explosion_Message* pMessage);

		// Handle Trigger message (for remote trigger mines)
		void OnTriggerMsg(Trigger_Message* pMessage);

		// Handles an ObjectDelete_Message.
		void OnDeleteMsg(								// Returns nothing.
			ObjectDelete_Message* pdeletemsg);	// In:  Message to handle.
	};


#endif // MINE_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
