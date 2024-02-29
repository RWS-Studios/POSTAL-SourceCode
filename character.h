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
// character.h
// Project: Postal
//
// This module implements the CCharacter class which is the class of generic
// character functionality for game characters.
//
// History:
//
//		03/03/97	BRH,JMI	Started this generic character object to reduce the
//							amount of redundant code.
//
//		03/04/97	JMI	Changed calling convention and naming for velocities and
//							position functions and added a Deluxe updater.
//
//		03/04/97 BRH	Added a few states for the band members.
//
//		03/04/97	JMI	Now initializes m_dDrag and m_dExtHorzDrag in constructor.
//
//		03/04/97	JMI	Added support for messages.
//
//		03/04/97 BRH	Added states from enemy guys.
//
//		03/04/97	JMI	Added State_Delete and functions for blood.
//
//		03/04/97	JMI	Added proto for UpdateFirePosition() member.
//
//		03/05/97	JMI	Added PrepareWeapon() and ShootWeapon() that can handle
//							any current CThing weapon (i.e., cannot handle bullets).
//
//		03/05/97 BRH	Added WhileShot function.
//
//		03/05/97	JMI	Added OnSuicideMsg() handler function.
//
//		03/06/97	JMI	Added m_bAboveTerrain member indicating whether on terrrain
//							or above it.
//
//		03/06/97	JMI	Added a GetAttributes() function.
//
//		03/12/97	JMI	DeluxeUpdatePosition() now takes the duration in seconds
//							as a parameter instead of calculating it itself (and over-
//							writing m_lPrevTime).
//
//		03/13/97	JMI	Load now takes a version number.
//
//		03/17/97	JMI	Now most of CCharacter's functionality is implemented in
//							its base class, CThing3d.
//
//		03/18/97	JMI	Made On and While functions virtual.
//
//		03/21/97	JMI	Removed m_pWeapon.
//
//		03/21/97	JMI	ShootWeapon() now returns a ptr to the weapon.
//
//		04/02/97	JMI	PrepareWeapon() now, also, returns a ptr to the weapon.
//							Also, removed #include of fire.h.
//
//		04/23/97	JMI	Added IsPathClear(), a rather deluxe function.
//
//		04/25/97	JMI	Added MakeBloodPool().
//
//		04/28/97	JMI	Added m_bullets and FireBullets().
//
//		05/02/97	JMI	FireBullets() now returns type bool indicating whether or
//							not someone/thing was hit by the bullets.
//
//		05/02/97	JMI	Added timer explicitly for CCharacter::While/On* functions.
//
//		05/07/97 BRH	Added FindAngleTo(x,z) function that all characters can
//							use to set their rotational angle to a given position.
//
//		05/13/97	JMI	You can now pass the amount of damage to MakeBloody()
//							which affects the amount of carnage.
//
//		05/16/97	JMI	Added directionality to blood.
//
//		05/26/97 BRH	Added CSmash bits to ShootWeapon and FireBullets so that
//							enemies can pass in different collision bits allowing
//							enemy bullets to ignore other enemies.
//
//		06/02/97	JMI	Added an WhileOnLadder() and m_idLadder.
//
//		06/02/97	JMI	Removed ladder stuff.
//
//		06/05/97	JMI	Changed m_sHitPoints to m_stockpile.m_sHitPoints to 
//							accommodate new m_stockpile in base class, CThing3d (which
//							used to contain the m_sHitPoints).
//
//		06/08/97 BRH	Added IlluminateTarget() function to check for targets within
//							a cone in the direction you specify.  This will be used by the
//							CDude for targeting feedback.  He will use it to place a
//							target sprite on the target he is aiming at.
//
//		06/11/97	JMI	Added Preload() for loading assets used during play.
//
//		06/11/97 BRH	Added ID for whoever kills you so you can report it
//							to the score module.  This value will be copied from
//							the message information.
//
//		06/13/97	JMI	Added WhileHoldingWeapon().
//
//		07/09/97	JMI	Changed Preload() to take a pointer to the calling realm
//							as a parameter.
//
//		07/21/97	JMI	Now overrides base class's Update().
//							Also, added OnWeaponDestroyed().
//							Also, added ValidateWeaponPosition().
//
//		08/02/97 BRH	Added virtual OnHelpMsg function.
//
//		08/08/97	JMI	Added Kill() to hook destruction and members for playing
//							weapon noises.
//
//		08/28/97 BRH	Added virtual put me down message handler.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef CHARACTER_H
#define CHARACTER_H

////////////////////////////////////////////////////////////////////////////////
// Includes.
////////////////////////////////////////////////////////////////////////////////
#include "Thing3d.h"
#include "weapon.h"
#include "SampleMaster.h"
#include "bulletFest.h"

class CCharacter : public CThing3d
	{
	/////////////////////////////////////////////////////////////////////////////
	// Typedefs/enums.
	/////////////////////////////////////////////////////////////////////////////
	public:

	//---------------------------------------------------------------------------
	// Variables
	//---------------------------------------------------------------------------
	public:
		U16	m_u16IdWeapon;						// ID of your generic weapon.
		CThing::ClassIDType m_eWeaponType;	// Type of weapon to be shot
		CBulletFest	m_bullets;					// Generic bullet interface.

		long	m_lCharacterTimer;				// This timer is intended for use by
														// CCharacter's On/While* functions.
		U16	m_u16KillerId;						// ID of the person who killed you
		// Used to track the current channel
		// playing our sound so we can update
		// its looping parameters.
		SampleMaster::SoundInstance	m_siLastWeaponPlayInstance;
		// Set time that sound should be stopped (as long as we keep updating
		// this, it won't get stopped).
		long	m_lStopLoopingWeaponSoundTime;



	//---------------------------------------------------------------------------
	// Static Variables
	//---------------------------------------------------------------------------
	public:

	//---------------------------------------------------------------------------
	// Constructor(s) / destructor
	//---------------------------------------------------------------------------
	protected:
		// Constructor
		CCharacter(CRealm* pRealm, CThing::ClassIDType id)
			: CThing3d(pRealm, id)
			{
			m_eWeaponType = CThing::CRocketID;
			m_u16IdWeapon	= CIdBank::IdNil;
			m_siLastWeaponPlayInstance	= 0;
			m_lStopLoopingWeaponSoundTime	= 0;
			}

	public:
		// Destructor
		~CCharacter()
			{
			Kill();
			}

	//---------------------------------------------------------------------------
	// Required static functions - None for this non-instantiable object.
	//---------------------------------------------------------------------------
	public:

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

		// Render object
		virtual														// Overridden here.
		void Render(void);										// Returns nothing.

		// Update object.											
		virtual														// Overridden here.
		void Update(void);										// Returns nothing.

	//---------------------------------------------------------------------------
	// Useful generic character message-specific functionality.
	//---------------------------------------------------------------------------
	public:

		// Process the specified message.  For most messages, this function
		// will call the equivalent On* function.
		virtual			// Override to implement additional functionality.
		void ProcessMessage(		// Returns nothing.
			GameMessage* pmsg);	// Message to process.

		// Message handling functions ////////////////////////////////////////////

		// Handles a Shot_Message.
		virtual			// Override to implement additional functionality.
							// Call base class to get default functionality.
		void OnShotMsg(					// Returns nothing.
			Shot_Message* pshotmsg);	// In:  Message to handle.

		// Handles an Explosion_Message.
		virtual			// Override to implement additional functionality.
							// Call base class to get default functionality.
		void OnExplosionMsg(							// Returns nothing.
			Explosion_Message* pexplosionmsg);	// In:  Message to handle.

		// Handles a Burn_Message.
		virtual			// Override to implement additional functionality.
							// Call base class to get default functionality.
		void OnBurnMsg(					// Returns nothing.
			Burn_Message* pburnmsg);	// In:  Message to handle.

		// Handles an ObjectDelete_Message.
		virtual			// Override to implement additional functionality.
							// Call base class to get default functionality.
		void OnDeleteMsg(								// Returns nothing.
			ObjectDelete_Message* pdeletemsg);	// In:  Message to handle.

		// Handles a DrawBlood_Message.
		virtual			// Override to implement additional functionality.
							// Call base class to get default functionality.
		void OnDrawBloodMsg(							// Returns nothing.
			DrawBlood_Message* pdrawbloodmsg);	// In:  Message to handle.

		// Handles a Suicide_Message.
		virtual			// Override to implement additional functionality.
							// Call base class to get default functionality.
		void OnSuicideMsg(						// Returns nothing.
			Suicide_Message* psuicidemsg);	// In:  Message to handle.

		// Handles a Help_Message
		virtual			// Override to implement additional functionality
							// Call base class to get default functionality
		void OnHelpMsg(							// Returns nothing
			Help_Message* phelpmsg);			// In:  Message to handle

		// Handles the put me down message
		virtual			// Override to implement additional functionality
							// Call base class to get default functionality
		void OnPutMeDownMsg(						// Returns nothing
			PutMeDown_Message* pputmedownmsg);	//In:  Message to handle



	//---------------------------------------------------------------------------
	// Useful generic character state-specific functionality.
	//---------------------------------------------------------------------------
	public:

		// Implements basic one-time functionality for each time State_Shot is
		// entered.
		virtual			// Override to implement additional functionality.
							// Call base class to get default functionality.
		void OnShot(void);

		// Implements basic functionality while being shot and returns true
		// until the state is completed.
		virtual			// Override to implement additional functionality.
							// Call base class to get default functionality.
		bool WhileShot(void);

		// Implements basic functionality while being blown up and returns true
		// until the state is completed.
		virtual			// Override to implement additional functionality.
							// Call base class to get default functionality.
		bool WhileBlownUp(void);	// Returns true until state is complete.

		// Implements basic functionality while being on fire and returns true
		// until the state is completed.
		virtual			// Override to implement additional functionality.
							// Call base class to get default functionality.
		bool WhileBurning(void);	// Returns true until state is complete.

		// Implements basic functionality while dying and returns true
		// until the state is completed.
		virtual			// Override to implement additional functionality.
							// Call base class to get default functionality.
		bool WhileDying(void);	// Returns true until state is complete.

		// Implements basic functionality while holding and preparing to release
		// a weapon.  Shows the weapon when the event hits 1 and releases the
		// weapon via ShootWeapon() when the event hits 2.
		virtual			// Override to implement additional functionality.
							// Call base class to get default functionality.
		bool WhileHoldingWeapon(	// Returns true when weapon is released.
			U32 u32BitsInclude,		// In:  Collision bits passed to ShootWeapon
			U32 u32BitsDontcare,		// In:  Collision bits passed to ShootWeapon
			U32 u32BitsExclude);		// In:  Collision bits passed to ShootWeapon

		// Implements basic one-time functionality for each time State_Dead is
		// entered.
		virtual			// Override to implement additional functionality.
							// Call base class to get default functionality.
		void OnDead(void);

		// Implements basic functionality while being run over and returns true
		// until the state is completed.
		virtual			// Override to implement additional functionality.
							// Call base class to get default functionality.
		bool WhileRunOver(void);	// Returns true until state is complete.

		// Implements one-time functionality for when a weapon is destroyed while
		// we were moving it (i.e., before we let go or ShootWeapon()'ed it).
		// This can occur when a weapon, while traveling along our rigid body,
		// enters terrain.
		virtual			// Override to implement additional functionality.
							// Call base class to get default functionality.
		void OnWeaponDestroyed(void);

		// Preload - cache the anims that may be used.
		static short Preload(
			CRealm* prealm);				// In:  Calling realm.

	//---------------------------------------------------------------------------
	// Useful generic character functionality.
	//---------------------------------------------------------------------------
	public:

		// Creates blood splat and pool animations.
		void MakeBloody(
			short sDamage,			// In:  Damage to base carnage on.
			short	sDamageAngle,	// In:  Angle in which (NOT from which) damage was
										// applied.
			short	sSwayRange);	// In:  Random amount chunks can sway from the
										// sDamageAngle (If 360, there'll be no noticeable
										// damage direction for the chunks).

		// Creates blood pool animation.
		void MakeBloodPool(void);

		// Draws last frame of blood pool into background.
		void BloodToBackground(
			short	sAnimX2d,			// Position of animation in 2d.
			short	sAnimY2d);			// Position of animation in 2d.

		// Prepare current weapon (ammo).
		// This should be done when the character starts its shoot animation.
		virtual			// Override to implement additional functionality.
							// Call base class to get default functionality.
		CWeapon* PrepareWeapon(void);	// Returns the weapon ptr or NULL.

		// Shoot current weapon.
		// This should be done when the character releases the weapon it's
		// shooting.
		virtual			// Override to implement additional functionality.
							// Call base class to get default functionality.
		CWeapon* ShootWeapon(	// Returns the weapon ptr or NULL.
					CSmash::Bits bitsInclude = 0,
					CSmash::Bits bitsDontcare = CSmash::Bad | CSmash::Good,
					CSmash::Bits bitsExclude = 0);

		// Validate weapon position.  If invalid, the weapon is destroyed and
		// the notification function, OnWeaponDestroyed() is called.
		bool ValidateWeaponPosition(void);	// Returns true, if weapon is in a valid position.
														// Returns false, if weapon destroyed because it
														// it is not in a valid position.

		// Fire a bullet.
		bool FireBullets(							// Returns true, if we hit someone/thing.
			RP3d*				ppt3d,				// In:  Launch pt in Postal units.
			short				sNumShots,			// In:  Number of shots to fire.
			short				sRange,				// In:  Bullet range.
			SampleMasterID	smidAmmo,			// In:  Ammo noise.
			CSmash::Bits	bitsInclude	= 0,	// In:  Optional bits we can hit
			CSmash::Bits	bitsDontcare = 0,	// In:  Optional bits for don't care
			CSmash::Bits	bitsExclude = 0);	// In:  Optional bits for exclude

		// Determine if a path is clear of items identified by the specified 
		// smash bits and terrain.
		bool IsPathClear(					// Returns true, if the entire path is clear.
												// Returns false, if only a portion of the path is clear.
												// (see *psX, *psY, *psZ).
			short sX,						// In:  Starting X.
			short	sY,						// In:  Starting Y.
			short sZ,						// In:  Starting Z.
			short sRotY,					// In:  Rotation around y axis (direction on X/Z plane).
			short sCrawlRate,				// In:  Rate at which to scan ('crawl') path in pixels per
												// iteration.
												// NOTE: We scan terrain using GetFloorAttributes()
												// so small values of sCrawl are not necessary.
												// NOTE: We could change this to a speed in pixels per second
												// where we'd assume a certain frame rate.
			short	sRangeXZ,				// In:  Range on X/Z plane.
			short sRadius,					// In:  Radius of path traverser.
			short sVerticalTolerance,	// In:  Max traverser can step up.
			CSmash::Bits bitsInclude,	// In:  Mask of CSmash bits that would terminate path.
			CSmash::Bits bitsDontCare,	// In:  Mask of CSmash bits that would not affect path.
			CSmash::Bits bitsExclude,	// In:  Mask of CSmash bits that cannot affect path.
			short* psX,						// Out: Last clear point on path.
			short* psY,						// Out: Last clear point on path.
			short* psZ,						// Out: Last clear point on path.
			CThing** ppthing,				// Out: Thing that intercepted us or NULL, if none.
			CSmash*	psmashExclude = NULL);// In:  Optional CSmash to exclude or NULL, if none.

			// Show a target sprite on whoever you would hit when aiming in the given
			// direction.  This will probably only be used by the CDude to help give feedback
			// in aiming.  Looks for targets in range in the aiming direction but does not
			// check terrain to see if it is a clear shot.
		bool IlluminateTarget(			// Returns true if there is a target
			short sX,						// In:  Starting x position
			short sY,						// In:  Starting y position
			short sZ,						// In:  Starting z position
			short sRotY,					// In:  Aiming direction (rotation around y axis)
			short sRangeXZ,				// In:  Range on X/Z plane
			short sRadius,					// In:  Radius of path traverser.
			CSmash::Bits bitsInclude,	// In:  Mask of CSmash bits that would count as a hit
			CSmash::Bits bitsDontCare,	// In:  Mask of CSmash bits that would not affect path
			CSmash::Bits bitsExclude,	// In:  Mask of CSmash bits that cannot affect path
			CThing** hThing,				// Out: Handle to thing that is the Target or NULL if none
			CSmash* psmashExclude = NULL);// In: Optional CSmash to exclude or NULL, if none. 

		// Give the angle from yourself to this x,z position
		inline short FindAngleTo(double dX, double dZ)
		{
			return rspATan((m_dZ - dZ), (dX - m_dX));	
		}

		// Called by destructor.
		void Kill(void);


	};

#endif	// CHARACTER_H

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
