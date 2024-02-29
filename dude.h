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
// dude.h
// Project: Postal
//
//	History:
//		01/27/97	JMI	Added EditRect() override to make this object clickable.
//
//		02/07/97	JMI	Removed m_pipe and m_sScreenRadius.  Added m_sCurRadius
//							to represent the radius of the dude's current frame.
//
//		02/10/97	JMI	Changed 3D stuff to use the new CAnim3D encapsulation 
//							class.  I suppose that eventually we will have several
//							of these per 3D thing.
//
//		02/11/97	JMI	Moved CAnim3D to thing.h from dude.h.
//
//		02/11/97	JMI	Added an EditHotSpot() override.
//
//		02/11/97	JMI	Added the 3D animation for throwing and an associated
//							rigid body transform for a grenade.
//
//		02/12/97	JMI	Fixed EditRect().
//
//		02/14/97	JMI	Moved EditRect() and EditHotSpot() bodies into the CPP.
//							I was getting annoyed compiling half the project for each
//							little tweak while experimenting.
//
//		02/16/97	JMI	Added Stand animation member.
//
//		02/18/97 MJR	Added fast velocity members.
//
//		02/18/97	JMI	Added a CBulletFest member, a GetCollisionSphere() 
//							override, and a CSmash member.
//
//		02/19/97	JMI	Added some states.
//
//		02/19/97	JMI	Added proto for new ReleaseGrenade() member.
//
//		02/19/97	JMI	Added StateDamage and m_sHitPoints.
//
//		02/26/97	JMI	Now sets m_sprite.m_pthing = this on construction.
//
//		03/03/97	JMI	Added last persistent state member.
//
//		03/03/97	JMI	Added states for strafing and multiweapon logic.
//
//		03/03/97	JMI	Changed m_pGrenade to m_pWeapon (a CWeapon* instead of
//							a CGrenade*) so more weapons can be handle generically.
//
//		03/03/97	JMI	Now CDude is based upon CCharacter.
//							Not much base class functionality utilized yet, though.
//
//		03/04/97	JMI	Now utilizes more of the base class functionality.
//							All CCharacter::Edit*() functions are used.
//							Moved stuff I was incorrectly doing in Render() into
//							Update() so CDude now uses CCharacter::Render().
//
//		03/04/97	JMI	Added ProcessMessages() to service the message queue.
//
//		03/05/97	JMI	Removed ProcessMessages() (now base class is fine).
//
//		03/05/97	JMI	Removed ReleaseWeapon() now uses CCharacter::ShootWeapon().
//
//		03/05/97	JMI	Added m_lLastShotTime representing the last time the dude
//							was shot.
//
//		03/05/97	JMI	Added m_animSuicide.
//							Made m_sDudeNum public.
//							Made SetState() public.
//							Added StartBrainSplat() and m_bBrainSplatted.
//
//		03/06/97	JMI	OnSuicideMsg() override.
//
//		03/13/97	JMI	Added EditModify() member function.
//							Also, added m_sNumFireBombs/Missiles/Napalms.
//							Load now takes a version number.
//
//		03/13/97	JMI	Added DrawStatus() member function, 
//							m_lStatusUpdateDoneTime, m_print, and GetWeaponInfo().
//
//		03/13/97	JMI	Changed default hitpoints to 500.
//
//		03/14/97	JMI	Added ms_dVertVelJump.
//
//		03/21/97	JMI	Added firing over the shoulder weapon animation, 
//							m_animLaunch.
//
//		03/27/97	JMI	Added many animations.
//
//		04/03/97	JMI	Added color index members for font.
//
//		04/07/97	JMI	Added jumps and fall components.
//
//		04/21/97	JMI	Incorporated Mike's new crawler thinger for smooth rubbin'
//							and slidin' up against schtuff.
//
//		04/21/97	JMI	Added override of WhileBlownUp().
//
//		04/24/97	JMI	Added shotgun weapon components.
//
//		04/25/97	JMI	Added a state for executing dudes.
//
//		04/25/97	JMI	Added flame thrower weapon support.  Required fuel ammo
//							info and special case on release of flamage.
//
//		04/28/97	JMI	Removed PrepareWeapon() override since the base class
//							version now handles bullet oriented weapons.
//							Also, moved FireBullets() to base class, CCharacter.
//
//		04/29/97	JMI	Added m_animPickPut for picking up and putting things down.
//							Also, added mine weapon.  You cannot choose the type now,
//							though (it's the default which is proximity).
//
//		04/29/97	JMI	Changed DefNumFuel to 10,000.
//
//		04/30/97	JMI	Changed Mine enum value to ProximityMine, TimedMine, 
//							RemoteMine, and BouncingBettyMine.
//
//		05/02/97	JMI	Added m_bGenericEvent1.
//
//		05/04/97 BRH	Change number of CDudes from being baseed on the STL 
//							container size to based on m_asClassNumThings][CDudeID],
//							now that the STL version of the things lists has been
//							taken out of realm completely.
//
//		05/08/97	JMI	Added the NoWeapon weapon.
//
//		05/09/97	JMI	Added m_psnd which we use to track which RSnd is
//							playing our sample.  This is useful for weapons like the
//							flame thrower which play a sound and then keep it looping
//							until they're done.
//
//		05/13/97	JMI	Added DefNumHeatseekers, Heatseeker, and m_sNumHeatseekers.
//
//		05/14/97	JMI	Added m_u16IdChild, m_szMessages, and Message() proto.
//
//		05/20/97 BRH	Added GetHealth() function for the AI logic tables to
//							access.
//
//		05/22/97	JMI	Added CDudeAnim3D so we could use Get() but have it not
//							load textures (they are loaded separately so the dude
//							color can be changed by the user).
//							Also, added m_sTextureIndex which is used to determine the
//							texture this dude is to use for all his animations.
//							Also, made enums and typedefs public.
//
//		05/23/97	JMI	Update() was nearly a thousand lines and that was just
//							too damned much.  So I broke it up a bit.
//							Also, changed FireCurrentWeapon() to ArmWeapon() which
//							takes as a parameter the weapon to arm.
//
//		05/23/97	JMI	m_sTextureIndex is now set in the constructor to 0, just
//							in case.
//
//		05/23/97	JMI	Changed reference to CThing::CAnim3D to CAnim3D.
//							Also, enhanced CDudeAnim3D to handle events.
//
//		05/26/97	JMI	Added m_lLastYellTime.
//
//		05/26/97	JMI	Added m_sOrigHitPoints so dude can show percentage based
//							on initial hitpoints.
//
//		05/29/97	JMI	Added a Revive() function which is the best way to revive
//							a dude that has died.
//							Added a timer for changing weapons that use the same input.
//
//		05/30/97	JMI	Added SprayCannon enum and m_inputLast.
//
//		06/02/97	JMI	Added climb state.
//
//		06/02/97	JMI	Removed climb state.
//
//		06/05/97	JMI	Removed m_sHitPoints and m_sNum*.  Now uses CThing3d's 
//							m_stockpile instead.
//
//		06/07/97	JMI	Made m_sOrigHitPoints and Init() public.
//
//		06/08/97 BRH	Added targeting sprite to the dude to help show
//							what he is aiming at.
//
//		06/09/97	JMI	Removed m_inputLast and m_lLastWeaponChangeTime which are
//							no longer necessary now that we can use the auto-repeat
//							feature with the new event driven rspGetKeyStatusArray()
//							used by input.cpp.
//							Also, added m_animIdle and m_lNextIdleTime.
//
//		06/11/97	JMI	Added handy functions, SetWeapon(), NextWeapon(), and
//							PrevWeapon().
//
//		06/11/97	JMI	Added DropPowerUp().
//
//		06/12/97	JMI	Added ms_apszAmmoNames to differentiate between weapons
//							and ammo strings.
//
//		06/12/97	JMI	Added m_bUpdateStatus.
//
//		06/15/97	JMI	Added more DefNum* type macros for dudes' stockpiles.
//
//		06/16/97	JMI	Added MessageChange() and m_lMsgUpdateDoneTime.
//
//		06/16/97	JMI	Added parms to make ShootWeapon() comply with virtual
//							base class version.
//							Also, m_bClearedStatus.
//
//		06/16/97	JMI	Added m_u16KillerId to track who killed this dude.
//
//		06/17/97	JMI	Added m_u8LastEvent to track the last animation event
//							and PlayStep() to play a step noise at the proper time.
//							Also, added OnExecute().
//
//		06/25/97	JMI	Added m_idVictim.
//
//		06/25/97	JMI	Replaced m_lStatusUpdateDoneTime with 
//							m_lNextStatusUpdateTime and removed m_bUpdateStatus.
//
//		07/01/97	JMI	Added new rigid bodies to CDudeAnim3D and moved its 
//							function defs into dude.cpp.
//							Also, added m_idFlagItem.
//
//		07/07/97	JMI	DrawStatus() now returns true if it drew to the provide
//							image.
//
//		07/15/97	JMI	Moved CDude() and ~CDude() from dude.h to dude.cpp.
//
//		07/15/97	JMI	Added TakePowerup().
//
//		07/16/97	JMI	Made DropPowerUp() return the newly created powerup.
//							Also, added CreateCheat().
//
//		07/16/97	JMI	Changed the order of the weapons.
//
//		07/17/97	JMI	Changed m_psnd to m_siLastPlayInstance.
//							Now uses new SampleMaster interface for volume and play
//							instance reference.
//
//		07/18/97	JMI	Now UpdateFontColors() takes a CRealm* to match the palette
//							to the hood of.
//
//		07/21/97	JMI	Now TakePowerUp() returns the powerup if it persisted.
//							Also, added ms_apszWeaponStatusFormats[].
//
//		07/21/97	JMI	Added OnWeaponDestroyed() override of CCharacter version.
//							Also, added m_weaponShooting.
//
//		07/23/97	JMI	Added DefHasNapalmLauncher enum.
//
//		07/25/97	JMI	Added option to Revive() to allow him to revive in-place
//							(rather than warping in).
//							Added m_bDead and IsDead() which are true when the dude
//							is dead.
//
//		07/25/97	JMI	Added StartAnim() which starts a CAnimThing.
//
//		07/29/97	JMI	Added GetCurrentWeapon().
//							Made Set/Next/PrevWeapon() functions public.
//
//		07/29/97	JMI	Changed IsDude() to IsDead().  Major typo.
//
//		07/30/97	JMI	Added DefHasDeathWadLauncher and DeathWad.
//							Also, added m_sDeadMsgNum.
//
//		08/04/97	JMI	Added array of weapon anims, m_aanimWeapons[], and 
//							a sprite for showing the current weapon, m_spriteWeapon.
//							Also, condensed ms_apszAmmoNames, ms_apszWeaponNames,
//							ms_apszWeaponStatusFormats, and ms_apszWeaponResNames
//							into ms_awdWeapons.
//
//		08/05/97	JMI	Added an ID to Message().  The idea being that, when the
//							same message ID is specified w/i a certain amount of time,
//							the more recent message is ignored.
//
//		08/07/97	JMI	Added DefHasDoubleBarrel and DoubleBarrel.
//							Also, added sMinAmmoRequired to weapon details database.
//
//		08/07/97	JMI	Added m_animBackpack and m_spriteBackpack for his backpack 
//							when he has it.
//
//		08/08/97	JMI	Removed m_siLastPlayInstance.
//
//		08/08/97	JMI	Added TossPowerUp().
//							Also, changed TakePowerup() to TakePowerUp().
//
//		08/10/97	JMI	Added m_dLastCrawledToPosX, Z which are the last position
//							successfully crawled to by the crawler (except on the
//							first iteration where they are simply set to the dude's
//							starting point).  In TRACENASSERT mode, an ASSERT is 
//							generated if the position gets modified by other than the
//							crawler.  In release mode, it sets him back to the last
//							successful crawled position.
//							Added a function to set m_dX, Y, & Z, SetPosition().
//
//		08/12/97	JMI	Removed unused anims: m_animJump, JumpForward, Land, 
//							LandForward, Fall.
//
//		08/14/97 BRH	Added static collision bits to be passed as the default
//							bits for weapons that the dude shoots.  The default
//							arguments in the ShootWeapon are unreliable and may
//							get set to the bits of the base class default args.
//							Also changed call of WhileHoldingWeapon to include
//							these default collision bits.
//
//		08/17/97	JMI	Got rid of m_szMessages and all message related functions
//							and variables from CDude since we are now using the toolbar 
//							for dude status feedback to the user.  This includes:  
//							MsgTypeInfo, m_lNextStatusUpdateTime, m_lMsgUpdateDoneTime, 
//							m_print, m_bClearedStatus, m_szMessages[], m_sDeadMsgNum, 
//							ms_amtfMessages[], ms_u8FontForeIndex, ms_u8FontBackIndex,
//							ms_u8FontShadowIndex, DrawStatus(), StatusChange(), 
//							MessageChange(), Message(), UpdateFontColors(), 
//							CPowerUp::ms_apszPowerUpTypeNames[], 
//							CPowerUp::GetDescription(), and some strings and a string
//							array in localize.*.
//
//		08/18/97	JMI	Moved StartAnim() from CDude to CThing3d so more things
//							could use it.
//
//		08/18/97	JMI	Added m_bInvincible which gets set via cheat.
//
//		08/28/97 BRH	Added OnPutMeDownMsg to handle the message sent from the
//							flag to drop it.
//
//		08/30/97	JMI	Removed m_idFlagItem (now uses the sprite list and, for
//							flag child sprites, updates them all).
//							Also, added GetNextFlag(), DropAllFlags().
//
//		09/03/97	JMI	Changed the ShootWeapon() default parameters to use the
//							CDude statics for bits (ms_u32CollideBits*).
//
//		11/21/97	JMI	Removed version of ShootWeapon() that took 3 default 
//							parameters and replaced it with a version that takes no
//							parameters so we could control the defaults from within the
//							CPP.
//
//		12/08/97	JMI	Added an option for DropPowerUp() to only drop the 
//							currently selected weapon.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef DUDE_H
#define DUDE_H

#include "RSPiX.h"

#include "realm.h"
#include "game.h"
#include "weapon.h"
#include "character.h"
#include "crawler.h"
#include "input.h"
#include "PowerUp.h"
#include "flag.h"

// First shot at a dude, which is a player-controlled character
class CDude : public CCharacter
	{
	//---------------------------------------------------------------------------
	// Types, enums, etc.
	//---------------------------------------------------------------------------
	public:

		// Macros.
		enum
			{
			MaxTextures					= 8,
											
			DefHitPoints				= 500,
											
			DefNumBullets				= 50,
			DefNumGrenades				= 5,
			DefNumFireBombs			= 5,
			DefNumMissiles				= 3,
			DefNumNapalms				= 3,
			DefNumShells				= 25,
			DefNumFuel					= 50,
			DefNumMines					= 3,
			DefNumHeatseekers			= 3,
											
			DefHasMachineGun			= 1,
			DefHasLauncher				= 0,
			DefHasShotgun				= 0,
			DefHasSprayCannon			= 0,
			DefHasFlamer				= 0,
			DefHasNapalmLauncher		= 0,
			DefHasDeathWadLauncher	= 0,
			DefHasDoubleBarrel		= 0,

			DefKevlarLayers			= 0,
										
			DefHasBackpack				= 0
			};

		typedef enum
			{
			CurrentWeapon = -1,
			NoWeapon,
			SemiAutomatic,
			ShotGun,
			SprayCannon,
			Grenade,
			Rocket,
			Heatseeker,
			FireBomb,
			Napalm,
			FlameThrower,
			ProximityMine, 
			TimedMine, 
			RemoteMine, 
			BouncingBettyMine,
			DeathWad,
			DoubleBarrel,

			NumWeaponTypes
			} WeaponType;

		typedef enum
			{
			MsgIdPickedUpPowerUp,
			MsgIdDontHaveExecuteWeapon,
			MsgIdDontHaveWeaponButHaveAmmo,
			MsgIdDontHaveWeaponOrAmmo,
			MsgIdDontHaveSuicideWeapon,

			NumMsgTypes
			} MsgId;

		typedef struct
			{
			char*	pszWeaponName;
			char*	pszAmmoName;
			char*	pszStatusFormat;
			char*	pszWeaponResName;
			short	sMinAmmoRequired;
			} WeaponDetails;

		// This special version overrides the CAnim3D Get(char*, char*, short).
		class CDudeAnim3D : public CAnim3D
			{
			public:

				ChanTransform*	m_ptransLeft;	// Rigid body transforms for left hand.
				ChanTransform* m_ptransRight;	// Rigid body transforms for right hand.
				ChanTransform* m_ptransBack;	// Rigid body transforms for his backpack.

				// Get the various components of this animation from the resource names
				// specified in the provided array of pointers to strings.
				virtual								// Overridden here.
				short Get(							// Returns 0 on success.
					char*		pszBaseFileName,	// In:  Base string for resource filenames.
					char*		pszRigidName,		// In:  String to add for rigid transform channel
														// or NULL for none.
					char*		pszEventName,		// In:  String to add for event states channel
														// or NULL for none.
					short		sLoopFlags);		// In:  Looping flags to apply to all channels
														// in this anim.

				// Release all resources.
				virtual						// Overridden here.
				void Release(void);		// Returns nothing.
			};

	//---------------------------------------------------------------------------
	// Variables
	//---------------------------------------------------------------------------
	public:

		short			m_sDudeNum;							// This dude's number for multiplayer mode

		short			m_sTextureIndex;					// This dude's texture index.  Used as an
																// index into m_aptextures[].

		short			m_sOrigHitPoints;					// Initial hitpoints.
		bool			m_bTargetingHelpEnabled;		// Show targeting sprite when enabled.

		// INPUT HACK. Normally we can't move in one direction while facing another. Android version
		// works by reading input while calculating the dude's velocity, but we can't do that with
		// keyboard input. Thus, these dude-only "twinstick" rotational values.
		bool			m_bUseRotTS;	// TRUE if we should use m_dRotTS to calculate velocity instead of m_dRot
		double			m_dRotTS;		// Direction we want to go when twinsticking

		// Actual joystick inputs
		double			m_dJoyMoveVel;
		double			m_dJoyMoveAngle;
		bool			m_bJoyFire;
		double			m_dJoyFireAngle;



	protected:

		CDudeAnim3D		m_animStand;					// Standing animation.
		CDudeAnim3D		m_animRun;						// Running animation.
		CDudeAnim3D		m_animThrow;					// Throwing animation.
		CDudeAnim3D		m_animDie;						// Dying animation.
		CDudeAnim3D		m_animShoot;					// Shooting animation.
		CDudeAnim3D		m_animRunShoot;				// Running and shooting animation.
		CDudeAnim3D		m_animDamage;					// Received damage animation.
		CDudeAnim3D		m_animBurning;					// Dude's on fire.
		CDudeAnim3D		m_animStrafe;					// Dude strafing with no forward movement.
		CDudeAnim3D		m_animStrafeShoot;			// Dude strafing and shooting with no forward movement.
		CDudeAnim3D		m_animSuicide;					// Dude commits suicide.
		CDudeAnim3D		m_animLaunch;					// Dude launches something with over the shoulder weapon.
		CDudeAnim3D		m_animBlownUp;					// Dude gets blown up.
		CDudeAnim3D		m_animGetUp;					// Dude gets up.
		CDudeAnim3D		m_animDuck;						// Dude ducks.
		CDudeAnim3D		m_animRise;						// Dude rises from duck.
		CDudeAnim3D		m_animExecute;					// Dude executes a victim/enemy.
		CDudeAnim3D		m_animPickPut;					// Dude picks something up or puts something down
																// depending upon direction played.
		CDudeAnim3D		m_animIdle;						// Dude hangs out -- idle animation.

		CAnim3D			m_aanimWeapons[NumWeaponTypes];	// Weapons' anims.

		CSprite3			m_spriteWeapon;						// Weapons' sprite.

		CAnim3D			m_animBackpack;						// Backpack's anim.
		CSprite3			m_spriteBackpack;						// Backpack's sprite.

		ChanTexture*	m_aptextures[MaxTextures];	// Colors for all dude animations.

		State	m_statePersistent;						// Last persistent state.  For example,
																// StateBurning is persistent.  He may get
																// shot and switch to damage state, but he
																// should switch back to burning when done.

		bool	m_bBrainSplatted;							// If true, brain has been splatted.
		bool	m_bJumpVerticalTrigger;					// If true, vertical acceleration for jump
																// has been applied.

		bool	m_bGenericEvent1;							// Generic event that can be used by
																// any state to note if it has or has not
																// yet been triggered.


		long			m_lNextBulletTime;				// Next time a bullet can be fired.
		long			m_lLastShotTime;					// Last time the dude was shot.
		long			m_lLastYellTime;					// Last time the dude yelled in pain
																// from being shot or something.
		long			m_lNextIdleTime;					// Idle animation timer.

		WeaponType	m_weapontypeCur;					// Dude's current weapon type.
		WeaponType	m_weaponShooting;					// The weapon type the dude is currently
																// shooting (or about to shoot).

		CCrawler		m_crawler;							// The device that allows us to slide
																// along edges and stuff.

		U16			m_u16IdChild;						// ID of generic child item.
																// Used by State_PickUp currently.

		CSprite2		m_TargetSprite;					// Targeting sprite to show what he is aiming
																// at.

		U16			m_u16KillerId;						// Instance ID of our killer.

		U8				m_u8LastEvent;						// Last anim event.

		U16			m_idVictim;							// Instance ID of victim to be executed or
																// used as human shield.

		bool			m_bDead;								// true, if dead; false otherwise.

		double		m_dLastCrawledToPosX;			// Last position successfully crawled to.
		double		m_dLastCrawledToPosZ;			// Last position successfully crawled to.

		bool			m_bInvincible;						// Dude does not loose health when invincible.

		// Tracks file counter so we know when to load/save "common" data 
		static short ms_sFileCount;

	//---------------------------------------------------------------------------
	// Static Variables
	//---------------------------------------------------------------------------
	public:
		// "Constant" values that we want to be able to tune using the editor
		static double ms_dAccUser;						// Acceleration due to user
		static double ms_dAccDrag;						// Drag on user velocity.

		static double ms_dMaxVelFore;					// Maximum forward velocity
		static double ms_dMaxVelBack;					// Maximum backward velocity

		static double ms_dMaxVelForeFast;			// Maximum forward velocity
		static double ms_dMaxVelBackFast;			// Maximum backward velocity

		static double ms_dDegPerSec;					// Degrees of rotation per second
		
		static double ms_dVertVelJump;				// Velocity of jump.

		// Weapon details database.
		static WeaponDetails	ms_awdWeapons[NumWeaponTypes];

		// Dude's default stockpile.
		static CStockPile	ms_stockpileDefault;

		// Dude's default weapon collision bits ie. what its weapons can hit
		static U32	ms_u32CollideBitsInclude;	// Bits that determine a collision
		static U32	ms_u32CollideBitsDontcare;	// Bits that are ignored for collision
		static U32	ms_u32CollideBitsExclude;	// Bits that invalidate collision

	//---------------------------------------------------------------------------
	// Constructor(s) / destructor
	//---------------------------------------------------------------------------
	protected:
		// Constructor
		CDude(CRealm* pRealm);

	public:
		// Destructor
		~CDude();

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
			*ppNew = new CDude(pRealm);
			if (*ppNew == 0)
				{
				sResult = -1;
				TRACE("CDude::Construct(): Couldn't construct CDude!\n");
				}

			return sResult;
			}

	//---------------------------------------------------------------------------
	// Required virtual functions (implimenting them as inlines doesn't pay!)
	//---------------------------------------------------------------------------
	public:
		// Load object (should call base class version!)
		short Load(													// Returns 0 if successfull, non-zero otherwise
			RFile* pFile,											// In:  File to load from
			bool bEditMode,										// In:  True for edit mode, false otherwise
			short sFileCount,										// In:  File count (unique per file, never 0)
			ULONG	ulFileVersion);								// In:  File version being loaded.

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

		// Called by editor to init new object at specified position
		short EditNew(												// Returns 0 if successfull, non-zero otherwise
			short sX,												// In:  New x coord
			short sY,												// In:  New y coord
			short sZ);												// In:  New z coord

		// Called by editor to modify object
		short EditModify(void);									// Returns 0 if successfull, non-zero otherwise

	//---------------------------------------------------------------------------
	// Other functions
	//---------------------------------------------------------------------------
	public:
		// Return the X position
		double GetX()
			{return m_dX;};

		// Return the Y position
		double GetY()
			{return m_dY;};

		// Return the Z position
		double GetZ()
			{return m_dZ;};

		// Return the dude's hit points
		short GetHealth()
			{return m_stockpile.m_sHitPoints;};

		// Sets a new state based on supplied state enum.  Will set animation ptr
		// to proper animation for state and reset animation timer.
		bool SetState(		// Returns true if new state realized, false otherwise.
			State	state);	// New state.

		// Gets info on specified weapon.
		void GetWeaponInfo(				// Returns nothing.
			WeaponType		weapon,		// In:  Weapon type to query.
			ClassIDType*	pidWeapon,	// Out: CThing class ID of weapon.
			short**			ppsNum);		// Out: Ptr to the weapon's counter.

		// Determines if supplied position is valid tweaking it if necessary.
		virtual									// Overridden here.
		bool MakeValidPosition(				// Returns true, if new position was validitable.
													// Returns false, if could not reach new position.
			double*	pdNewX,					// In:  x position to validate.
													// Out: New x position.
			double*	pdNewY,					// In:  y position to validate.
													// Out: New y position.
			double*	pdNewZ,					// In:  z position to validate.
													// Out: New z position.
			short	sVertTolerance = 0);		// Vertical tolerance.

		// Shoot current weapon.
		// This should be done when the character releases the weapon it's
		// shooting.
		virtual			// Overriden here.
		CWeapon* ShootWeapon(				// Returns the weapon ptr or NULL.
			CSmash::Bits bitsInclude,
			CSmash::Bits bitsDontcare,
			CSmash::Bits bitsExclude);

		CWeapon* ShootWeapon(void);		// Returns the weapoin ptr or NULL.

		// Determine if the dude is dead.
		bool IsDead(void)	// Returns true, if dead; false otherwise.
			{
			return m_bDead;
			}

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

		// Handles a Suicide_Message.
		virtual			// Override to implement additional functionality.
							// Call base class to get default functionality.
		void OnSuicideMsg(						// Returns nothing.
			Suicide_Message* psuicidemsg);	// In:  Message to handle.

		// Handles a PutMeDown_Message
		virtual			// Override to implement additional functionality
							// Call base calss to get default functionality
		void OnPutMeDownMsg(						// Returns nothing
			PutMeDown_Message* pputmedownmsg);	//In:  Message to handle

		// State handling functions /////////////////////////////////////////////

		// Implements basic functionality while being blown up and returns true
		// until the state is completed.
		virtual							// Overriden here.
		bool WhileBlownUp(void);	// Returns true until state is complete.

		// Execute the nearest writhing guy, if any.
		void OnExecute(void);		// Returns nothing.

		// Implements one-time functionality for when a weapon is destroyed while
		// we were moving it (i.e., before we let go or ShootWeapon()'ed it).
		// This can occur when a weapon, while traveling along our rigid body,
		// enters terrain.
		virtual			// Overriden here.
		void OnWeaponDestroyed(void);


		// Other functions //////////////////////////////////////////////////////

		// Revive a dead dude.  This is a more graceful way than just setting
		// his state.  This will restore hitpoints and make him animate to a
		// standing position.  If bWarp is true, he'll use a warp when available.
		void Revive(					// Returns nothing.
			bool	bWarpIn	= true);	// In:  true, to warp in, false to just get up.

		// Initialize dude.  Must be done before dude is used.
		short Init(void);											// Returns 0 if successfull, non-zero otherwise

		// Next weapon please.
		void NextWeapon(void);

		// Previous weapon please.
		void PrevWeapon(void);

		// Set the current weapon.
		bool SetWeapon(						// Returns true if weapon could be set as current.
			WeaponType weapon,				// In:  New weapon to attempt to make the current.
			bool	bSetIfNoAmmo = true);	// In:  true to set weapon (even if no ammo).

		// Get the current weapon the dude has ready to use.
		WeaponType GetCurrentWeapon(void);

		
	//---------------------------------------------------------------------------
	// Internal functions
	//---------------------------------------------------------------------------
	protected:
		// Kill dude
		void Kill(void);

		// Get all required resources
		short GetResources(void);								// Returns 0 if successfull, non-zero otherwise
		
		// Free all resources
		void FreeResources(void);

		// Update the animation radius based on current frame.
		void UpdateRadius(void);	// Returns and accepts nothing.

		// Fire specified weapon type.
		void ArmWeapon(								// Returns nothing.
			WeaponType weapon = CurrentWeapon);	// In:  Weapon to fire.

		// Receive damage.
		void Damage(						// Returns nothing.
			short	sHitPoints,				// Hit points of damage to do.
			U16	u16ShooterId);			// In:  Thing responsible for damage.

		// Start the brain splat anim on its way.
		void StartBrainSplat(void);	// Returns nothing.

		// Attempts user motivated state transitions.
		void ProcessInput(				// Returns nothing.
			double*	pdMaxForeVel,		// Out: Maximum forward velocity.
			double*	pdMaxBackVel,		// Out: Maximum backward velocity.
			short*	psStrafeAngle);	// Out: Strafe angle.

		// Applies accelerations, velocities, reacts to terrain obstructions, etc.
		void ProcessForces(				// Returns nothing.
			long		lCurTime,			// In:  Current game time.
			double	dMaxForeVel,		// Out: Maximum forward velocity.
			double	dMaxBackVel,		// Out: Maximum backward velocity.
			short		sStrafeAngle);		// Out: Strafe angle.

		// If the targeting aid is enabled, this function will look for a target
		// and if it finds one, it will show the Targeting sprite on the target.
		void ShowTarget(void);

		// Drop a powerup with the settings described by the specified stockpile.
		CPowerUp* DropPowerUp(				// Returns new powerup on success; NULL on failure.
			CStockPile*	pstockpile,			// In:  Settings for powerup.
			bool			bCurWeaponOnly);	// In:  true, if only the current weapon should be
													// in the powerup; false, if all.

		// Create a cheat powerup.
		CPowerUp* CreateCheat(			// Returns new powerup on success; NULL on failure.
			CStockPile*	pstockpile);	// In:  Settings for powerup.

		// Play a step noise if the event is different from the last.
		void PlayStep(void);				// Returns nothing.

		// Find someone to execute.
		bool FindExecutee(void);		// Returns true, if we found one; false, otherwise.

		// Track executee.
		bool TrackExecutee(				// Returns true to persist, false, if we lost the target.
			double dSeconds);				// In:  Seconds since last iteration.

		// Take a powerup.
		void TakePowerUp(					// Returns nothing.
			CPowerUp**	pppowerup);		// In:  Power up to take from.
												// Out: Ptr to powerup, if it persisted; NULL otherwise.

		// Break a powerup open and toss it.
		void TossPowerUp(					// Returns nothing.
			CPowerUp*	ppowerup,		// In:  Powerup to toss.
			short			sVelocity);		// In:  Velocity of toss.

		// Make sure warp can access this function.
		friend class CWarp;

		// Sets the dude's position.  It is very important that the dude is not
		// moved by outside things, other than the warp.
		void SetPosition(					// Returns nothing.
			double	dX,					// In:  New position for dude.
			double	dY,					// In:  New position for dude.
			double	dZ);					// In:  New position for dude.

		// Get the next child flag item after the specified flag item.
		CFlag* GetNextFlag(			// Returns the next flag item after pflag.
			CFlag*	pflag);			// In:  The flag to get the follower of.
											// NULL for first child flag.
		
		// Drop all child flag items.
		void DropAllFlags(
			GameMessage*	pmsg);		// In:  Message to pass to flags.

	};


#endif //DUDE_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
