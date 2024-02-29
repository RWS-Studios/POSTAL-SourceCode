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
// doofus.h
// Project: Postal
//
// History:
//		01/13/97 BRH	Started this file from CDude and modified it
//							to do some enemy logic using the same assets
//							as the sample 2D guy.  
//
//		03/13/97	JMI	Load now takes a version number.
//
//		04/22/97 BRH	Moved common variables like the animations and previous
//							state and animation variables to the base class since
//							all of the enemies had their own versions.  Added logic
//							functions that can be called from a derrived class.
//
//		04/24/97 BRH	Added TryClearDirection function that uses the 
//							IsPathClear() funciton to try 3 directions to see if they
//							are clear.
//
//		04/25/97	JMI	Added m_animWrithing.
//
//		05/06/97 BRH	Added a detection smash that is much larger than the
//							normal collision smash.  This one is use to detect pylons
//							in the area.
//
//		05/07/97 BRH	Added pylon pointers for Popout and Run/Shoot logic to make
//							it eaiser to know where to go.
//
//		05/09/97 BRH	Incorporated some more logic from CPerson into CDoofus.
//
//		05/11/97 BRH	Made the logic routines virtual functions so that the
//							derived classes can make their own for special case 
//							purposes.
//
//		05/12/97 BRH	Added destination state so that an overall goal state
//							could be set and held on to even when interrupted by
//							intermediate states like Popout or moving.
//
//		05/18/97 BRH	Added some logic routines for the victims to use.
//
//		05/20/97 BRH	Changed logic tables to use Suggested actions rather
//							than changing the states directly.  Added an Action
//							enum to Doofus for the logic variables to use.  Also
//							Added current and suggested action values to the doofus,
//							and an logic table evaluation timer.
//
//		05/23/97 BRH	Added TryClearShot function to check for clear shooting
//							angle.
//
//		05/25/97 BRH	Added the m_ShootAngle variable and an override for
//							ShootWeapon that uses this angle to aim the weapon.
//
//		05/26/97 BRH	Added an overload to ShootWeapon that uses the 
//							m_dShootAngle for aiming.  Also it sets the CSmash bits
//							so that enemy bullets don't hit other enemies.
//
//		05/31/97	JMI	Replaced m_pDude with m_idDude.  The problem was that, by
//							just using a pointer to the dude, we never found out when
//							the dude was gone (deleted).  Although this is rare for
//							CDudes, it does happen.  For example, in the beginning of
//							a level all CDudes that do not have an associated player
//							are sent a Delete msg.  They do not process this message
//							until their respective Update() calls.  If a CDoofus 
//							derived guy happened to be placed in the level before a 
//							CDude (that is, the CDoofus' Update() got called before the
//							CDude's), and the CDoofus happened to point its m_pDude at 
//							this CDude (that was destined to soon be deleted), later 
//							when referencing the pointer to the freed and/or reallocated
//							memory, the CDoofus could cause a protection	fault or	math 
//							overflow (due to invalid values returned by 
//							m_pDude->GetX, Y, Z() with the non-CDude 'this' pointer).
//
//		06/02/97 BRH	Added AdvanceHold action and state so that once he reaches
//							the end of the advancement, he goes into this hold state
//							rather than Engage automatically.  This way the logic
//							table can have more control over the next state.
//
//		06/10/97 BRH	Added Crouch and Search animations for idle animation.
//
//		06/14/97 BRH	Added virtual sound effect functions that can be defined
//							also at the Person level to choose the sound effect
//							from the personatorium.  Also added comment timer and 
//							comment counter to regulate the number of comments made
//							so they don't get too repetative.
//
//		06/17/97 BRH	Added a timer value for the sounds playing to store
//							the estimated finish time of the current sample, rather
//							than using the IsSamplePlaying() which screws up the
//							network mode.
//
//		06/18/97 BRH	Added an override function for PrepareWeapon so that the
//							Shooting sounds can be played.
//
//		06/18/97	JMI	Changed PlaySoundWrithing() to return the duration of the
//							played sample.
//
//		07/06/97 BRH	Added static panic flag to the doofus for the victims to
//							use.  It will be cleared at the beginning of the level
//							and will be set by the first victim to get shot so that 
//							all of the other victims will know to run around scared.
//							Also added a few logic routines for victims.
//
//		07/09/97 BRH	Added walk and panic actions for victim logic tables.
//
//		07/10/97 BRH	Added madness and march actions for victim and protestor 
//							logic.
//
//		07/11/97 BRH	Added call to inline Cheater() to disable game if necessary
//
//		07/17/97 BRH	Added Logic_DelayShoot so that you can choose to put
//							the guy into a shooting state but wait for a timer to
//							expire.
//
//		07/17/97	JMI	Commented out m_prsndIsPlaying and m_psmidIsPlaying since
//							they are not used.  If they are needed in the future,
//							m_prsndIsPlaying should be a SampleMaster::SoundInstance
//							instead of an RSnd*.
//							Changed RSnd*'s to SampleMaster::SoundInstances.
//
//		07/23/97 BRH	Added tunable values for three different timeouts.
//
//		07/25/97 BRH	Integrated the cookie check into Cheater.
//
//		08/02/97 BRH	Added a few functions for avoiding fire.
//
//		08/02/97 BRH	Added YellForHelp function which will alert others within
//							line of sight that you have been shot, then they can 
//							decide to take action.  Added OnHelpMsg function to handle
//							the call for help.
//
//		08/06/97	JMI	Added m_ptransExecutionTarget link point for execution
//							sphere.  Also, added PositionSmash() to provide overridable
//							method for updating the collision sphere.
//
//		08/07/97	JMI	Added ms_awdWeapons[], ms_apszWeaponResNames[], and 
//							GetResources() and FreeResources() for loading these anims.
//							Also, added ms_lWeaponResRefCount so we could know when the
//							weapons were no longer needed.
//
//		08/08/97	JMI	Added more weapons:  UZI, AutoRifle, SmallPistol, Dynamite.
//
//		08/08/97 BRH	Added Logic_MarchBegin function for the marching.
//
//		08/10/97	JMI	Moved CDoofus() and ~CDoofus() into doofus.cpp.
//							Added m_bRegisteredBirth which is true once we have 
//							registered our birth with the realm.
//
//		08/10/97	JMI	Moved NoWeapon up to enum value 0 and created a new one
//							to take its -1 place as an invalid weapon (InvalidWeapon).
//							Also, added block in PrepareWeapon() for the NoWeapon case.
//							Also, moved prepare weapon from the .H to the .CPP.
//
//		08/11/97	JMI	Added fallback weapon type, m_eFallbackWeaponType, which
//							can signify a weapon to use when there is no weapon anim 
//							for the current weapon type.  Whew.
//							Also, changed incorrectly name ms_awtType2Id to 
//							ms_awtId2Type mapping.
//
//		08/17/97 BRH	Added m_sStuckCounter to detect situations where the 
//							enemy is trying to move in engage mode, but has got
//							stuck in some narrow terrain and should go back to the
//							bouys.
//
//		08/18/97 BRH	Added virtual WhileHoldingWeapon override for Doofus so
//							that for the higher difficulty settings where the guys
//							aim after preparing weapon, they can do it every frame
//							in between so that they don't end up flipping around
//							quick when they shoot the weapon, especially the rocket
//							man which has a long shoot-prepare animation.
//
//		08/21/97 BRH	Added a blood pool counter so that the blood could be cut
//							down a little bit.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef DOOFUS_H
#define DOOFUS_H

#include "RSPiX.h"
#include "realm.h"
#include "navnet.h"
#include "bouy.h"
#include "dude.h"
#include "pylon.h"
#include "CompileOptions.h"

// CDoofus is the class for the enemy guys
class CDoofus : public CCharacter
	{
	//---------------------------------------------------------------------------
	// Types, enums, etc.
	//---------------------------------------------------------------------------
	public:
			typedef enum						// Suggested logic actions basec on 
			{										// evaluation of situation
				Action_Guard,					// Stay in place and check for proximity to Dude
				Action_Advance,				// Advance toward your target Dude
				Action_Retreat,				// Retreat away from the area
				Action_Engage,					// Shoot and move, shoot and move
				Action_Popout,					// Hide at pylon until triggered, then popout & shoot
				Action_RunShoot,				// Hide at pylon until triggered, then run to other pylon
				Action_Hide,					// Hide at safe place pylon
				Action_AdvanceHold,			// You are as close as you can get on the bouy network
				Action_Walk,					// Walk around - victims
				Action_Panic,					// Panic - victims
				Action_March,					// March along a bouy network nearby.
				Action_Madness,				// Run all over - not on bouy network.
				Action_Help,					// Help when you hear a call for help

				// Add actions before this
				NumActions
			} Action;


			// This describes a weapon scheme that can be used as an index
			// into ms_awdWeapons[].
			typedef enum
			{
				InvalidWeapon	= -1,
				NoWeapon,
				Rocket,
				Grenade,
				Napalm,
				Firebomb,
				ProximityMine,
				TimedMine,
				RemoteControlMine,
				BouncingBettyMine,
				Flamer,
				Pistol,
				MachineGun,
				ShotGun,
				Heatseeker,
				Assault,
				DeathWad,
				DoubleBarrel,
				Uzi, 
				AutoRifle, 
				SmallPistol,
				Dynamite,
				
				NumWeaponTypes
			} WeaponType;

			typedef struct
			{
				char*			pszName;
				char*			pszResName;
				ClassIDType	id;
			} WeaponDetails;


	//---------------------------------------------------------------------------
	// Variables
	//---------------------------------------------------------------------------
	protected:
		// General position, motion and time variables
		U16	m_idDude;						// The target CDude 

		// Animations
		CAnim3D	m_animStand;				// Standing animation
		CAnim3D	m_animRun;					// Running animation
		CAnim3D	m_animShoot;				// Shoot gun animation
		CAnim3D	m_animShootRun;			// Shoot while running
		CAnim3D	m_animShootRunR0;			// Shoot at 45 degree right angle
		CAnim3D	m_animShootRunR1;			// Shoot at 90 degree right angle
		CAnim3D	m_animShootRunBack;		// Shoot backwards
		CAnim3D	m_animShootRunL0;			// Shoot at 45 degree left angle
		CAnim3D	m_animShootRunL1;			// Shoot at 90 degree left angle
		CAnim3D	m_animShot;					// Get shot animation
		CAnim3D	m_animDie;					// Die Die!
		CAnim3D	m_animWrithing;			// Writhing in predeathness.
		CAnim3D	m_animExecuted;			// Death by Execution
		CAnim3D	m_animOnfire;				// Running around on fire
		CAnim3D	m_animWalk;					// Walk animation - mostly for victims
		CAnim3D	m_animCrouch;				// Crouch down when idle
		CAnim3D	m_animSearch;				// Look around when idle


		// Navigation Net control
		CNavigationNet* m_pNavNet;			// The network I should use
		U16 m_u16NavNetID;					// My network's ID				
		UCHAR m_ucDestBouyID;				// Destination bouy
		UCHAR m_ucNextBouyID;				// Next bouy to go to
		UCHAR m_ucSpecialBouy0ID;			// Starting bouy for special cases like marching
		UCHAR m_ucSpecialBouy1ID;			// Ending bouy for special cases like marching
		CBouy* m_pNextBouy;					// pointer to next bouy to go to.
		short m_sNextX;						// Position of next Bouy
		short m_sNextZ;						// Position of next Bouy
		short m_sRotateDir;					// Direction to rotate when avoiding obstacles
		long	m_lAlignTimer;					// Recheck position to bouy every so often
		long	m_lEvalTimer;					// Reevaluate state every so often
		long	m_lShotTimeout;				// Only do Shot animation every so often
		long	m_lStuckTimeout;				// time given to recovery from stuck state
		long	m_lShootTimer;					// Limit number of shots from a gun.
		long	m_lCommentTimer;				// Time between random comments
		short	m_usCommentCounter;			// Number of comments
		CDoofus::Action m_eSuggestedAction;	// Suggested logic action
		CDoofus::Action m_eCurrentAction;	// Currently running action
		CCharacter::State m_eDestinationState; // Final state you wish to achieve
		CCharacter::State m_ePreviousState;
		CCharacter::State m_eNextState;	// This can be used to make states like Shoot more
													// reusable.  Several states can go to State Shoot and
													// then when it is done, it can go to this next state
													// so that it can be part of several different state loops
		CAnim3D*			m_panimPrev;		// Previous state's animation

		// Channel of execution points for 'writhing' anim/state.
		ChanTransform*	m_ptransExecutionTarget;


		CSmash			m_smashDetect;		// Smash used to detect pylons - has large radius
		CSmash			m_smashAvoid;		// Smash used to avoid fire
		CPylon*			m_pPylonStart;		// Starting pylon for popout or run/shoot
		CPylon*			m_pPylonEnd;		// Ending pylon for popout or run/shoot
		short				m_sDistRemaining;	// Distance to new position for fighting.
		bool				m_bPylonSafeAvailable;
		bool				m_bPylonPopoutAvailable;
		bool				m_bPylonRunShootAvailable;
		bool				m_bPanic;			// Has been alerted to panic or not.
		double			m_dAnimRot;			// Animation rotation, used to face the animation
													// in one direction while moving in another direction.
		double			m_dShootAngle;

		long				m_lIdleTimer;		// Timer for idle animations.
		bool				m_bAnimUp;			// Run animation up or down for idle animation. (crouch)

		long				m_lSampleTimeIsPlaying; // Expected time for this sample
		bool				m_bRecentlyStuck;			// Flag for when you get stuck on a wall.		
		bool				m_bCivilian;				// Flag for civilian/hostile
		bool				m_bRegisteredBirth;		// true, once we've registered our birth with the realm.

		long				m_lGuardTimeout;			// Tunable personatorium value with doofus default
		long				m_lShootTimeout;			// Tunable time between shots - based on difficulty level
		long				m_lRunShootInterval;		// Tunable personatorium value with doofus default.
		long				m_lShotReactionTimeout;	// Tunable personatorium value with doofus default
		long				m_lLastHelpCallTime;	// Last time someone called for help

		CSprite3			m_spriteWeapon;			// Sprite for weapon.
		ClassIDType		m_eFallbackWeaponType;	// Fallback weapon type or TotalIDs for none.
		short				m_sStuckCounter;			// Number of times he tried to move in the current state
		USHORT			m_usBloodCounter;			// Counter to limit the blood pools.
		SampleMaster::SoundInstance m_siPlaying;	// Sound instance that is playing - in case it needs to be stopped


		// Tracks file counter so we know when to load/save "common" data 
		static short ms_sFileCount;

		// "Constant" values that we want to be able to tune using the editor
		static double ms_dAccUser;			// Acceleration due to user
		static double ms_dAccDrag;			// Acceleration due to drag (always towards 0)

		static double ms_dMaxVelFore;		// Maximum forward velocity
		static double ms_dMaxVelBack;		// Maximum backward velocity
												
		static double ms_dDegPerSec;		// Degrees of rotation per second
		static double ms_dOffScreenDistance;
		static double ms_dGuardDistance;	// Sq distance before he begins attacking.
		static double ms_dThrowHorizVel;	// Horizontal throw velocity
		static double ms_dMinFightDistance; // Min distance for fighting
		static double ms_dMedFightDistance;	// Median distance for fighting
		static double ms_dMaxFightDistance;	// Max distance for fighting
		static double ms_dMinFightDistanceSQ;
		static double ms_dMedFightDistanceSQ;
		static double ms_dMaxFightDistanceSQ;
		static double ms_dMarchVelocity;	// How fast to walk when marching.
		static long ms_lDefaultAlignTime;// How often to recalibrate angle to bouy
		static long ms_lGuardTimeoutMin;	// How often to check for CDudes proximity
		static long ms_lGuardTimeoutInc;	// Amount of time between for each level of difficulty
		static long ms_lShootTimeoutMin;	// How often to wait between shots, min
		static long ms_lShootTimeoutInc;	// Variance between shot times based on difficulty level
		static long ms_lDetectionRadius;	// Radius of detection sphere
		static long ms_lRunShootInterval;// Time to run between shooting
		static long ms_lReseekTime;		// Time to go before seeking the dude's position again
		static long ms_lShotTimeout;		// Time to go before doing full shot anim when shot
													// this will give him time to escape.
		static long ms_lAvoidRadius;		// Radius of fire avoidance smash
		static long ms_lYellRadius;		// Radius of alerting smash
		static long ms_lHelpTimeout;		// Time to react to a call for help.
		static long ms_lDelayShootTimeout;//time before shooting
		static long ms_lHelpingTimeout;	// time before shooting when helping
		static long ms_lStuckRecoveryTime;//time to allow recovery from stuck position

		static U32 ms_u32CollideBitsInclude;	// Default weapon collision bits
		static U32 ms_u32CollideBitsDontcare;	// Default weapon collision bits
		static U32 ms_u32CollideBitsExclude;	// Default weapon collision bits

		static short ms_sStuckLimit;				// Number of retrys before changing states to get unstuck

		static CAnim3D			ms_aanimWeapons[NumWeaponTypes];	// Weapon animations.
		static long				ms_lWeaponResRefCount;				// Current ref count on ms_aanimWeapons[].
		static WeaponDetails	ms_awdWeapons[NumWeaponTypes];	// Weapon details (descriptions,
																				// res names, etc.).
		static WeaponType		ms_awtId2Type[TotalIDs];			// Maps a CThing ID to a WeaponType enum.

	public:
		static char* ms_apszActionNames[];// Names of the logic actions

	//---------------------------------------------------------------------------
	// Constructor(s) / destructor
	//---------------------------------------------------------------------------
	protected:
		// Constructor
		CDoofus(CRealm* pRealm, ClassIDType id);

	public:
		// Destructor
		~CDoofus();

	//---------------------------------------------------------------------------
	// Required static functions
	//---------------------------------------------------------------------------
	public:
		// Construct object
		static short Construct(									// Returns 0 if successfull, non-zero otherwise
			CRealm* pRealm,										// In:  Pointer to realm this object belongs to
			CThing** ppNew)										// Out: Pointer to new object
			{
			return 0;
			}

	//---------------------------------------------------------------------------
	// Required virtual functions (implimenting them as inlines doesn't pay!)
	//---------------------------------------------------------------------------
	public:
		// Load object (should call base class version!)
		virtual short Load(										// Returns 0 if successfull, non-zero otherwise
			RFile* pFile,											// In:  File to load from
			bool bEditMode,										// In:  True for edit mode, false otherwise
			short sFileCount,										// In:  File count (unique per file, never 0)
			ULONG	ulFileVersion);								// In:  Version of file format to load.

		// Save object (should call base class version!)
		virtual short Save(													// Returns 0 if successfull, non-zero otherwise
			RFile* pFile,											// In:  File to save to
			short sFileCount);									// In:  File count (unique per file, never 0)

		// Startup object
		virtual short Startup(void);							// Returns 0 if successfull, non-zero otherwise

		// Called by editor to init new object at specified position
		virtual short EditNew(									// Returns 0 if successfull, non-zero otherwise
			short sX,												// In:  New x coord
			short sY,												// In:  New y coord
			short sZ);												// In:  New z coord

		// Called by editor to render object
		virtual void EditRender(void);

		// Override to swap the animation and direction rotations temporarily
		virtual void Render(void);
	
		// Derived classes should use their own Update function
		virtual void Update(void);

		// Guard the area until a CDude is nearby
		virtual void Logic_Guard(void);

		// When a CDude is nearby, keep an eye on him and attack
		// if he gets aggressive
		virtual void Logic_Patrol(void);

		// Find a CDude and use the network to get him wherever he is
		virtual void Logic_Hunt(void);

		// Stay in Hunt mode, looking for a closer bouy
		virtual void Logic_HuntHold(void);

		// Find the next bouy en route to the final destination
		virtual void Logic_MoveNext(void);

		// Detect nearby pylons and see what logic they suggest
		virtual void Logic_PylonDetect(void);

		// Start Hide sequence by going to the 'cover' pylon
		virtual void Logic_HideBegin(void);

		// Hide here and keep checking for a different Action suggestion
		virtual void Logic_Hide(void);

		// Start popout sequence by going to the 'cover' pylon
		virtual void Logic_PopBegin(void);

		// Wait for Dude to come in range to popout on him.
		virtual void Logic_PopWait(void);

		// Popout from a popout bouy and shoot, then return to cover
		virtual void Logic_Popout(void);

		// Run and Shoot and seek cover between two bouys
		virtual void Logic_RunShoot(void);

		// Run and Shoot begin
		virtual void Logic_RunShootBegin(void);

		// Run and Shoot wait - wait at endpoints before going again
		virtual void Logic_RunShootWait(void);

		// Stay at a SafeSpot bouy if while the CDude is facing you
		virtual void Logic_BeSafe(void);

		// Once injured or in a dangerous situation, you may choose
		// to run for the hills
		virtual void Logic_Retreat(void);

		// Firefight - you are in a shootout, so shoot & move
		virtual void Logic_Firefight(void);

		// Engage logic - Get into position and start fighting
		virtual void Logic_Engage(void);

		// Shoot the weapon - this state uses the m_eNextState variable
		// to know where to go after the shot is fired and the animation
		// is over
		virtual void Logic_Shoot(void);

		// Shoot while running - uses m_eNextState to know where to go
		// after shot is fired and the animation is over.
		virtual void Logic_ShootRun(void);

		// Shot - what you do when you get shot
		virtual void Logic_Shot(void);

		// Blownup - what you do when someone blows you up
		virtual void Logic_BlownUp(void);

		// Burning - what you do when you are on fire
		virtual void Logic_Burning(void);

		// Die - what you do when you are dying
		virtual void Logic_Die(void);

		// Writhing on the ground in pain
		virtual void Logic_Writhing(void);

		// Setup to get into position for fighting
		virtual void Logic_PositionSet(void);

		// Move into position for fighting
		virtual void Logic_PositionMove(void);

		// Wait for timer to expire, then shoot
		virtual void Logic_DelayShoot(void);

		// Victim panic - pick a random bouy to run to
		virtual void Logic_PanicBegin(void);

		// Victim panic again once you reach the destination
		virtual void Logic_PanicContinue(void);

		// Victim walk - pick a random bouy to walk to
		virtual void Logic_WalkBegin(void);

		// Victim walk again once you reach the destination
		virtual void Logic_WalkContinue(void);

		// Avoid fire - wait for fire danger to disappear before
		// going back to your previous state.
		virtual void Logic_AvoidFire(void);

		// When someone nearby gets shot and 'yells for help', this
		// is the reaction state that can be set so that you can shoot
		// from where you are without screwing up your current state.
		virtual void Logic_Helping(void);

		// Start a march - like a protest.  Pick one endpoint bouy and then
		// MarchNext until you get there, then pick the other one.
		virtual void Logic_MarchBegin(void);

		// This can be called when a timer expires, or if a situation
		// changes, etc.  It will take into consideration the current state, 
		// position of CDude, and enemy attributes and sometimes random
		// numbers to pick a state for the guy.
		bool ReevaluateState(void);

		// Overloaded version of ShootWeapon to use m_dShootAngle
		// and which sets the smash bits so enemy bullets won't hit other
		// enemies.
		virtual CWeapon* ShootWeapon(
			CSmash::Bits bitsInclude = CSmash::Character, 
			CSmash::Bits bitsDontcare = 0,
			CSmash::Bits bitsExclude = CSmash::Bad | CSmash::SpecialBarrel);

		// Function to choose and play the writhing sound effect
		virtual SampleMaster::SoundInstance PlaySoundWrithing(
			long* plDuration)					// Out:  Duration of sample, if not NULL.
			{
			if (plDuration != NULL)
				{
				*plDuration	= 0;
				}

			return 0;
			}

		// Function to choose and play the Shot sound effect
		virtual SampleMaster::SoundInstance PlaySoundShot(void)
			{return 0;};

		// Function to choose and play the Blown up sound effect
		virtual SampleMaster::SoundInstance PlaySoundBlownup(void)
			{return 0;};

		// Funciton to choose and play the Burning sound effect
		virtual SampleMaster::SoundInstance PlaySoundBurning(void)
			{return 0;};

		// Function to choose and play the shooting comment
		virtual SampleMaster::SoundInstance PlaySoundShooting(void)
			{return 0;};

		// Function to choose and play the dying sound.
		virtual SampleMaster::SoundInstance PlaySoundDying(void)
			{return 0;};

		// Function to choose and play the Random comments
		virtual SampleMaster::SoundInstance PlaySoundRandom(void)
			{return 0;};

		// Prepare current weapon (ammo).
		// This should be done when the character starts its shoot animation.
		virtual			// Override to implement additional functionality.
							// Call base class to get default functionality.
		CWeapon* PrepareWeapon(void);	// Returns the weapon ptr or NULL.

		// Implements basic functionality while holding and preparing to release
		// a weapon.  Shows the weapon when the event hits 1 and releases the
		// weapon via ShootWeapon() when the event hits 2.
		virtual			// Override to implement additional functionality.
							// Call base class to get default functionality.
		bool WhileHoldingWeapon(	// Returns true when weapon is released.
			U32 u32BitsInclude,		// In:  Collision bits passed to ShootWeapon
			U32 u32BitsDontcare,		// In:  Collision bits passed to ShootWeapon
			U32 u32BitsExclude);		// In:  Collision bits passed to ShootWeapon


	//---------------------------------------------------------------------------
	// Internal functions
	//---------------------------------------------------------------------------
	protected:
		// Get the bouy that the CDude is closest to
		short SelectDudeBouy(void);					// Returns 0 if successful, non-zero otherwise

		// Return a valid random bouy or 0 if no bouys exist.
		UCHAR SelectRandomBouy(void);

		// Set a pointer to the CDude you are tracking for other CDude related
		// functions like FindDirection and SQDistanceToDude
		short SelectDude(void);				// Returns 0 if successful, non-zero if no Dudes found

		// Find the angle to the selected CDude
		short FindDirection(void);

		// If Alignment timer is up, recalc the direction to the bouy.
		void AlignToBouy(void);

		// Find the squared distance to the CDude (to avoid sqrt)
		double SQDistanceToDude(void);

		// Attempt 3 paths
		bool TryClearDirection(double* pdRot, short sVariance);

		// Check shooting angle before firing to make sure no
		// walls are in the way.
		bool TryClearShot(double dRot, short sVariance);

		// Check for fire in your path and change to a safe state rather
		// than blindly running into the fire.
		bool AvoidFire(void);	// Returns true if there is a fire danger

		// When you get shot call this function which will alert other enemies
		// in the area in line of sight so that they can choose to react.
		void YellForHelp(void);

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

		// Handles a Help_Message
		virtual			// Override to implement additional functionality
							// Call base class to get default functionality
		void OnHelpMsg(								// Returns nothing
			Help_Message* phelpmsg);

		// Override for CCharacter::OnDead
		virtual void OnDead(void);

		// Run an idle animation and switch them up every so often
		void RunIdleAnimation(void);

		// Position our smash approriately.
		virtual			// Override to implement additional functionality.
							// Call base class to get default functionality
		void PositionSmash(void);

		// Check for cheaters and make the game interesting
		inline void Cheater(void)
		{
            #if 0 //!PLATFORM_UNIX   // This isn't cheating, this is beta expiration. --ryan.
			if (g_lRegValue < 0 || g_lExpValue < 0 || g_lCookieMonster == SAFE_DATE)
			{
				GameMessage msg;
				CThing* pThing = NULL;

				msg.msg_Burn.eType = typeBurn;
				msg.msg_Burn.sPriority = 0;
				msg.msg_Burn.sDamage = 1000;
				msg.msg_Burn.u16ShooterID = GetInstanceID();;

				CListNode<CThing>* pNext = m_pRealm->m_everythingHead.m_pnNext;
				while (pNext->m_powner != NULL)
				{
					pThing = pNext->m_powner;
					SendThingMessage(&msg, pThing);
					pNext = pNext->m_pnNext;
				}	
			}
            #endif
		}

		// Look up a WeaponDetails by CThing class ID.
		inline WeaponDetails* GetWeaponDetails(	// Returns ptr to details or NULL, if none.
			ClassIDType	id)								// In:  ID to look up.
			{
			ASSERT(id <= TotalIDs);

			WeaponDetails*	pwd	= NULL;

			if (id < TotalIDs)
				{
				WeaponType	wt	= ms_awtId2Type[id];
				if (wt != InvalidWeapon)
					{
					pwd	= &(ms_awdWeapons[wt]);
					}
				}
			else
				{
				pwd	= &(ms_awdWeapons[NoWeapon]);
				}

			return pwd;
			}

		// Look up a weapon animation by CThing class ID.
		inline CAnim3D* GetWeaponAnim(		// Returns ptr to anim or NULL, if none.
			ClassIDType	id)						// In:  ID to look up.
			{
			ASSERT(id <= TotalIDs);

			CAnim3D*	panim	= NULL;

			if (id < TotalIDs)
				{
				WeaponType	wt	= ms_awtId2Type[id];
				if (wt != InvalidWeapon)
					{
					panim	= &(ms_aanimWeapons[wt]);
					}
				}
			else
				{
				panim	= &(ms_aanimWeapons[NoWeapon]);
				}

			return panim;
			}

		// A way for the base class to get resources.  If you are going to use
		// any of this class's resources (e.g., ms_aanimWeapons[]), call this
		// when getting your resources.
		short GetResources(void);

		// A way for the base class to release resources.  If you are going to use
		// any of this class's resources (e.g., ms_aanimWeapons[]), call this
		// when releasing your resources.
		void ReleaseResources(void);
	};


#endif //DOOFUS_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
