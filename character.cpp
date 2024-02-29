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
// character.cpp
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
//		03/04/97	JMI	The m_sprite.m_sInFlags are now only initialized in 
//							Startup() (instead of in Render()).  If it was in Render(),
//							it overwrites any other initializations that take place on
//							these bit flags (such as XRay).
//
//		03/04/97	JMI	UpdateVelocities() now makes sure the delta value is cor-
//							rect even when m_dVel is capped or trimmed.
//							Drags are now added (was subtracting them).
//							Commented out code to restore old velocities (as if accel
//							did not take place) when no new position was obtained.
//
//		03/04/97	JMI	Added support for messages.
//
//		03/05/97	JMI	Converted all rspMod360() calls to new calling convention.
//							Added fire response in OnBurnMsg().
//							Added UpdateFirePosition().
//
//		03/05/97	JMI	Added PrepareWeapon() and ShootWeapon() that can handle
//							any current CThing weapon (i.e., cannot handle bullets).
//
//		03/05/97 BRH	Added code to WhileShot, WhileBurning, WhileBlownUp,
//							WhileDying for default functionality for most of the
//							victims and enemy guys.
//
//		03/05/97	JMI	Now OnDead() draws current animation frame into the back-
//							ground.
//
//		03/05/97	JMI	Added handler for suicide message.
//							WhileBlownUp() now stops all horizontal movement when the
//							character encounters the 'no walk' attrib.
//							Added UpdateVelocity() inline in attempt to unify the way
//							velocity is updated.  Works for two of three cases, haven't
//							tried third for fear of hosedness.
//							Currently vertical tolerance is not considered in
//							ValidatePosition().
//							Made external horizontal drag in OnExplosionMsg() negative.
//
//		03/06/97	JMI	Re-enabled vertical tolerance utilizing new m_bAboveTerrain
//							boolean.
//
//		03/06/97	JMI	Increased horizontal surface drag.
//							Now Render() uses the combined attributes for the layer.
//
//		03/06/97	JMI	No longer relies on m_pWeapon to store weapon ptr.  But,
//							if a derived class sets m_pWeapon to point to a weapon,
//							Render() will still update its transform.
//							This should be removed as soon as no one uses m_pWeapon.
//
//		03/06/97	JMI	Now, if EditRect() does not have enough info to create
//							a rectangle, it tries a couple ways.
//
//		03/06/97	JMI	Now ShootWeapon() gets the rigid body transform from the
//							current animation instead of from the child's sprite.
//
//		03/06/97	JMI	Now allows a character to fall off of things but not onto
//							unless the height difference is within tolerance.
//
//		03/06/97	JMI	Added a GetAttributes() function.
//							Now uses GetAttributes() instead of manually doing it.
//
//		03/12/97	JMI	Actually, mistakenly, last time I put GetAttributes() in
//							WhileBlowingUp() instead of in DeluxeUpdatePosition().
//							Now DeluxeUpdatePosition() utilizes GetAttributes().
//							DeluxeUpdatePosition() now takes the duration in seconds
//							as a parameter instead of calculating it itself (and over-
//							writing m_lPrevTime).
//
//		03/13/97	JMI	GetAttributes() was combining ONLY the layer bits.  What
//							I really wanted it to do was combine all but the height
//							bits.  Now it uses ~ATTRIBUTE_HEIGHT_MASK to combined the
//							bits.
//
//		03/13/97	JMI	Load now takes a version number.
//
//		03/17/97	JMI	Now most of CCharacter's functionality is implemented in
//							its base class, CThing3d.
//
//		03/21/97	JMI	Removed m_pWeapon.
//
//		03/21/97	JMI	ShootWeapon() now sets the rotation of the weapon beforing
//							detaching it.
//
//		03/21/97	JMI	ShootWeapon() now returns a ptr to the weapon.
//
//		04/02/97	JMI	OnDead() now destroys any weapon the character still has.
//							The idea is that, if the weapon was just rendered as a 
//							child object onto the background, it should no longer exist
//							in CThing form.  Also, undeleted child weapons will clutter
//							memory until the realm they're in is destroyed.
//							Removed m_pCurrentAnim.
//
//		04/02/97	JMI	PrepareWeapon() now, also, returns a ptr to the weapon.
//
//		04/15/97 BRH	We were noticing that the guys no longer turned dark after
//							being burnt.  I added the change to m_sBrightness in
//							WhileBurning() so that he turns dark when dead.
//
//		04/23/97	JMI	Added IsPathClear(), a rather deluxe function.
//
//		04/24/97	JMI	#if 0'd out the debug portion of IsPathClear() which 
//							displayed the check path as red or green line depending on
//							whether or not the path was entirely clear.
//
//		04/25/97	JMI	Broke blood pool creation out of MakeBloody() and into
//							MakeBloodPool().
//
//		04/28/97 BRH	Changed PrepareWeapon to use the new fake class ID's to
//							identify weapons that don't have an object associated with
//							them like guns.  Now this function is generic enough that
//							the CDude doesn't have to override its own version of
//							PrepareWeapon.
//
//		04/28/97	JMI	Changed ShootWeapon() to work for CShotGunID, CPistolID, 
//							CMachineGunID, and CFireballID.
//							Also, added FireBullets() to simplify firing of bullets.
//
//		04/29/97	JMI	Added case to ShootWeapon() to handle mines.
//
//		04/29/97	JMI	Changed references to child weapon's m_sprite to 
//							GetSprite() calls and took out special cases for
//							CFireballID and CMineID.
//
//		05/02/97	JMI	FireBullets() now returns type bool indicating whether or
//							not someone/thing was hit by the bullets.
//
//		05/02/97	JMI	Added timer explicitly for CCharacter::While/On* functions.
//							Utilized the timer in WhileBurning() to make sure dude dies
//							within a certain amount of burning and to make the dude
//							darken while he burns instead of just at the end.
//
//		05/13/97	JMI	Now MakeBloody() creates chunks based on the passed damage.
//
//		05/14/97	JMI	Now Render() uses PositionChild() to position the weapon.
//
//		05/16/97	JMI	Added directionality to blood.
//
//		05/22/97	JMI	Added bullet casings and shells.
//							Also, made all particle effects (Blood, Casings, and 
//							Shells) obey g_GameSettings.m_sParticleEffects.
//
//		05/23/97	JMI	Lowered velocity of shells and casings generated by Fire-
//							Bullets().
//
//		05/26/97	JMI	Lowered vertical velocity of shells and casing generated
//							by FireBullets().
//
//		05/26/97 BRH	Added CAssaultWeapon which is just the shot gun allowed
//							to rapid fire.
//
//		05/29/97	JMI	Removed references to m_pRealm->m_pAttribMap which no longer
//							exists.
//
//		05/30/97	JMI	Changed FireBullets() to only play the sample once (through
//							FireDeluxe(...) ).
//
//		06/02/97	JMI	Added an WhileOnLadder().
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
//		06/10/97	JMI	Removed CSmash::Misc from the default bits to collide with
//							when using FireBullets().
//
//		06/11/97 BRH	Added passing of Shooter ID down into the Shot message and
//							to the other weapons in ShootWeapon().  Also added the
//							bullets damage chart to give tunability to different
//							weapons and target vs shooter differences.	
//
//		06/11/97	JMI	Added Preload() for loading assets used during play.
//
//		06/11/97 BRH	Added the ID of the killer and used that to report to
//							the score module at time of death.
//
//		06/12/97	JMI	3D weapons are now initially (in PrepareWeapon() ) set to
//							CWeapon::State_Hide.
//
//		06/13/97	JMI	Added WhileHoldingWeapon().
//
//		06/15/97	JMI	Now OnShotMsg() checks to make sure we're receiving damage
//							before calling MakeBloody() (if someone's wearing a kevlar
//							vest, they may receive the impact forces of the bullet but
//							little or no damage).
//
//		06/17/97	JMI	Converted all occurrences of rand() to GetRand() and
//							srand() to SeedRand().
//
//		06/24/97	JMI	Now intializes animthing's m_msg's priority to 0 on in 
//							MakeBloodPool() so we don't end up with randomly 
//							prioritized messages.
//
//		06/30/97	JMI	Now uses CRealm's new GetRealmWidth() and *Height()
//							for dimensions of realm's X/Z plane.
//
//		07/01/97	JMI	Now passes rigid body transform to PositionChild().
//
//		07/09/97	JMI	Now uses m_pRealm->Make2dResPath() to get the fullpath
//							for 2D image components.
//
//		07/09/97	JMI	Changed Preload() to take a pointer to the calling realm
//							as a parameter.
//
//		07/21/97	JMI	Now has an Update() which updates the weapon's position.
//							Also, added ValidateWeaponPosition().
//
//		07/21/97	JMI	Now ValidateWeaponPosition() checks to make sure the weapon
//							state is not State_Hide before checking the position.
//
//		07/27/97	JMI	No longer calls CRealm::Make2dResPath() on res names
//							given to CAnimThing as it already does that 
//							automatically.
//							Also, now appropriately places blood splats on outside of 
//							character's body based on opposite angle of impact.
//
//		07/30/97	JMI	Now copies necessary ammo from ccharacter's stockpile to
//							the ammo, if required.
//
//		08/02/97 BRH	Added virtual OnHelpMsg function.
//
//		08/07/97	JMI	Added ShootWeapon()/FireBullets() support for the double
//							barrel.
//
//		08/07/97	JMI	Now blood splat lands on ground (at terrrain height) 
//							instead of at the character's Y.
//							Changed RAND_SWAY to an inline that considers the scene 
//							scale.
//
//		08/07/97	JMI	In last version did not put the blood pool on the ground
//							while animating.
//
//		08/08/97	JMI	Now this class plays the flamer sound and handles its
//							loopage.
//
//		08/08/97	JMI	Upgraded PrepareWeapon(), ShootWeapon(), and FireBullets()
//							to handle AutoRifle, Uzi, and SmallPistol.
//
//		08/09/97	JMI	Now MakeBloody() tries to keep shots low for characters
//							that are in the writhing state.
//
//		08/09/97	JMI	Converted a use of TransformPtsToRealm() to GetLinkPoint().
//
//		08/11/97	JMI	Changed DoubleBarrel sound to g_smidDeathWadLaunch (was the
//							ol' wimpy g_smidShotGun).
//
//		08/13/97	JMI	Temporarily shows particle effects regardless of user
//							setting in multiplayer mode.
//
//		08/17/97	JMI	Now FireBullets() will never shoot writhers.
//
//		08/17/97	JMI	Changed m_pthingParent to m_idParent.
//
//		08/18/97	JMI	Now applies no randomization to CChunks and instead allows
//							CChunk::Setup() to apply its own randomization based on
//							sway values passed to it.  Also, CChunk::Construct() will
//							now fail if particles are disabled so we don't need to
//							check for that anymore.
//
//		08/18/97	JMI	FireBullets() now includes the random variation in angle
//							in the bullet message (before it used the UNswayed angle).
//
//		08/18/97	JMI	Changed OnDead() to call DeadRender3D() (which used to be
//							known/called as just another Render() overload).
//
//		08/24/97	JMI	Adjusted shotgun pellets to be 8 when shot by a dude into
//							a non dude.  Also, adjusted machine gun bullets to be 15
//							when shot by a dude into a non dude.
//
//		08/28/97 BRH	Added a virtual put me down message handler.
//
////////////////////////////////////////////////////////////////////////////////
#define CHARACTER_CPP

#include "RSPiX.h"
#include <math.h>

#include "character.h"
#include "reality.h"
#include "AnimThing.h"
#include "fireball.h"
#include "mine.h"
#include "fire.h"
#include "chunk.h"
#include "score.h"
#include "deathWad.h"

////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////

#define MAX_FORE_VEL				80.0
#define MAX_BACK_VEL				-60.0

#define MAX_STEPUP_THRESHOLD	10.0

// Determines the number of elements in the passed array at compile time.
#define NUM_ELEMENTS(a)		(sizeof(a) / sizeof(a[0]) )

// Random amount the fire angle can adjust for bullets.
#define FIRE_ANGLE_Y_SWAY		15
#define FIRE_ANGLE_Z_SWAY		10

#define MAX_BULLET_RANGE		400

#define MAX_SHELL_RANGE			200

#define NUM_SHOTS_PER_SHELL	7

#define FIREBALL_LIVE_MS		500

#define MAX_TIME_WITHOUT_FLAMAGE	500

// Random amount the blood splat can adjust.
#define BLOOD_SPLAT_SWAY		10
// This is the distance in realm units from the characters' centers
// to the outsides of their bodies.  This is used to adjust things
// like blood splats around the outside of their torsos.
#define TORSO_RADIUS						5


#define BLOOD_SPLAT_RES_NAME	"BloodFront.aan"
#define BLOOD_POOL_RES_NAME	"BloodPool.aan"

// Light level for burnt character.
#define BURNT_BRIGHTNESS		-40	// -128 to 127.
// Amount of time for character to burn.
#define BURN_DURATION			2500	// In ms.

// Sets a value pointed to if ptr is not NULL.
#define SET(pval, val)					((pval != NULL) ? *pval = val : val)

// Maximum chunks that can be generated from one MakeBloody().
#define MAX_CHUNKS				10

// Amount chunks can vary from the direction of damage.
#define CHUNKS_DAMAGE_DIR_SWAY	135

#define SHOOTER_IS_DUDE			0x0008
#define TARGET_IS_DUDE			0x0004
#define WEAPON_IS_PISTOL		0x0000
#define WEAPON_IS_MACHINEGUN	0x0001
#define WEAPON_IS_SHOTGUN		0x0002
#define WEAPON_IS_ASSAULT		0x0003

////////////////////////////////////////////////////////////////////////////////
// Variables/data
////////////////////////////////////////////////////////////////////////////////

static short ms_asBulletDamageChart[16] = 
{
//	Damage			Shooter	Target	Weapon
	10,	// 0000	NonDude	NonDude	Pistol
	10,	// 0001	NonDude	NonDude	MachineGun
	10,	// 0010	NonDude	NonDude	ShotGun
	10,	// 0011	NonDude	NonDude	AssaultWeapon
	10,	// 0100	NonDude	Dude		Pistol
	10,	//	0101	NonDude	Dude		MachineGun
	10,	//	0110	NonDude	Dude		ShotGun
	7,		//	0111	NonDude	Dude		AssaultWeapon
	10,	// 1000	Dude		NonDude	Pistol
	15,	// 1001	Dude		NonDude	MachineGun
	8,		// 1010	Dude		NonDude	ShotGun
	10,	// 1011	Dude		NonDude	AssaultWeapon
	10,	// 1100	Dude		Dude		Pistol
	10,	// 1101	Dude		Dude		MachineGun
	10,	// 1110	Dude		Dude		ShotGun
	10,	//	1111	Dude		Dude		AssaultWeapon
};

////////////////////////////////////////////////////////////////////////////////
// Gets a random value that is +/- lRange with sway scaled by the current 
// scene scaling.
////////////////////////////////////////////////////////////////////////////////
inline
long GetRandSway(		// Returns sway value.
	long		lRange,	// In:  Total range of output value.
	double	dScale)	// In:  Scaling.
	{
	// There's two approaches possible here:
	// 1) Scale lRange by current scene scale.
	// 2) Scale result by current scene scale.
	// I think I like the first option better because it allows for more
	// granularity in the result.  But, since that is variable on a per level
	// basis, I'm not sure it's worth worrying about.

	// Going with option 1.
	lRange	*= dScale;

	return (GetRand() % lRange) - lRange / 2;
	}

////////////////////////////////////////////////////////////////////////////////
// Load object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CCharacter::Load(									// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,											// In:  File to load from
	bool bEditMode,										// In:  True for edit mode, false otherwise
	short sFileCount,										// In:  File count (unique per file, never 0)
	ULONG	ulFileVersion)									// In:  Version of file format to load.
	{
	// Call the CThing base class load to get the instance ID
	short sResult = CThing3d::Load(pFile, bEditMode, sFileCount, ulFileVersion);
	if (sResult == 0)
		{
		switch (ulFileVersion)
			{
			default:
			case 3:
				pFile->Read(&m_eWeaponType);
				break;

			case 2:	// Versions 1 and 2, when CCharacter was not descended from
			case 1:	// CThing3d, loaded the weapon type amidst all the other data.
				// Load object data
				// **FUDGE.
				m_eWeaponType	= CThing::CRocketID;
				break;
			}

		// Make sure there were no file errors or format errors . . .
		if (!pFile->Error() && sResult == 0)
			{
			// Success.
			}
		else
			{
			sResult = -1;
			TRACE("CCharacter::Load(): Error reading from file!\n");
			}
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Save object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CCharacter::Save(									// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,											// In:  File to save to
	short sFileCount)										// In:  File count (unique per file, never 0)
	{
	// Call the base class save to save the u16InstanceID
	short	sResult	= CThing3d::Save(pFile, sFileCount);
	if (sResult == 0)
		{
		// Save object data
		pFile->Write(&m_eWeaponType);

		sResult	= pFile->Error();
		}

	return sResult;
	}

////////////////////////////////////////////////////////////////////////////////
// Update object.											
// (virtual).
////////////////////////////////////////////////////////////////////////////////
void CCharacter::Update(void)										// Returns nothing.
	{
	if (m_u16IdWeapon != CIdBank::IdNil)
		{
		CWeapon*	pweapon;
		if (m_pRealm->m_idbank.GetThingByID((CThing**)&pweapon, m_u16IdWeapon) == 0)
			{
			RTransform*	ptransWeapon	= m_panimCur->m_ptransRigid->GetAtTime(m_lAnimTime);
			// Position weapon.
			PositionChild(
				pweapon->GetSprite(),	// In:  Child sprite to position.
				ptransWeapon,				// In:  Transform specifying position.
				&(pweapon->m_dX),			// Out: New position of child. 
				&(pweapon->m_dY),			// Out: New position of child. 
				&(pweapon->m_dZ) );		// Out: New position of child. 
			}
		}

	// If we have a weapon sound play instance . . .
	if (m_siLastWeaponPlayInstance)
		{
		// If time has expired . . .
		if (m_pRealm->m_time.GetGameTime() > m_lStopLoopingWeaponSoundTime)
			{
			// Stop looping the sound.
			StopLoopingSample(m_siLastWeaponPlayInstance);
			// Forget about it.
			m_siLastWeaponPlayInstance	= 0;
			}
		}

	// Call base class.
	CThing3d::Update();
	}

////////////////////////////////////////////////////////////////////////////////
// Render object
////////////////////////////////////////////////////////////////////////////////
void CCharacter::Render(void)
	{

	// Call base class.
	CThing3d::Render();
	}


//---------------------------------------------------------------------------
// Useful generic character state-specific functionality.
//---------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
// Implements basic one-time functionality for each time State_Shot is
// entered.
////////////////////////////////////////////////////////////////////////////////
void CCharacter::OnShot(void)
	{
	CThing3d::OnShot();
	}

////////////////////////////////////////////////////////////////////////////////
// Implements basic functionality while shot and returns true
// until the state is completed.
////////////////////////////////////////////////////////////////////////////////
bool CCharacter::WhileShot(void)	// Returns true until state is complete
	{
	return CThing3d::WhileShot();
	}

////////////////////////////////////////////////////////////////////////////////
// Implements basic functionality while being blown up and returns true
// until the state is completed.
////////////////////////////////////////////////////////////////////////////////
bool CCharacter::WhileBlownUp(void)	// Returns true until state is complete.
	{
	return CThing3d::WhileBlownUp();
	}

////////////////////////////////////////////////////////////////////////////////
// Implements basic functionality while being on fire and returns true
// until the state is completed.
////////////////////////////////////////////////////////////////////////////////
bool CCharacter::WhileBurning(void)	// Returns true until state is complete.
	{
	bool	bStatePersists	= true;	// Assume not done.

	// Get time from last call in seconds.  Should this be passed in so we don't
	// update m_lPrevTime???
	long		lCurTime	= m_pRealm->m_time.GetGameTime();
	double	dSeconds	= double(lCurTime - m_lPrevTime) / 1000.0;
	m_lPrevTime			= lCurTime;

	// Make character run around while on fire, varying in direction.
	m_dAcc = 150;
	m_dRot = rspMod360(m_dRot + (GetRand() % 30) - 15);
	DeluxeUpdatePosVel(dSeconds);

	// If the fire still exists . . .
	CFire*	pfire;
	if (m_pRealm->m_idbank.GetThingByID((CThing**)&pfire, m_u16IdFire) == 0)
		{
		// If the fire is still burning . . .
		if (pfire->IsBurning() != FALSE)
			{
			// Brightness is the ratio of the amount of time expired to the
			// total time multiplied by the destination brightness.
			long	lTimeExpired	= MIN(lCurTime + BURN_DURATION - m_lCharacterTimer, (long)BURN_DURATION);
			m_sBrightness			= lTimeExpired * BURNT_BRIGHTNESS / BURN_DURATION;

			// If time has expired . . .
			if (lTimeExpired >= BURN_DURATION)
				{
				bStatePersists	= false;
				}
			}
		else
			{
			// We're done.
			bStatePersists	= false;
			}
		}
	else
		{
		// We're done.
		bStatePersists = false;
		}

	// If we're done . . .
	if (bStatePersists == false)
		{
		// Let's make sure we're dead.
		m_stockpile.m_sHitPoints	= 0;
		}

	return bStatePersists;
	}

////////////////////////////////////////////////////////////////////////////////
// Implements basic functionality while dying and returns true
// until the state is completed.
////////////////////////////////////////////////////////////////////////////////
bool CCharacter::WhileDying(void)	// Returns true until state is complete.
	{
	bool	bStatePersists	= true;	// Assume not done.

	// When he finishes his current animation (assuming dying anim) then he is
	// officially dead.
	if (m_lAnimTime > m_panimCur->m_psops->TotalTime())
		bStatePersists = false;

	return bStatePersists;
	}

////////////////////////////////////////////////////////////////////////////////
// Implements basic functionality while holding and preparing to release
// a weapon.  Shows the weapon when the event hits 1 and releases the
// weapon via ShootWeapon() when the event hits 2.
////////////////////////////////////////////////////////////////////////////////
bool CCharacter::WhileHoldingWeapon(	// Returns true when weapon is released.
	U32 u32BitsInclude,						// In:  Collision bits to pass to ShootWeapon
	U32 u32BitsDontcare,						// In:  Collision bits to pass to ShootWeapon
	U32 u32BitsExclude)						// In:  Collision bits to pass to ShootWeapon
	{
	bool	bReleased	= false;	// Assume not released.

	U8	u8Event	= *( (U8*)(m_panimCur->m_pevent->GetAtTime(m_lAnimTime) ) );
	// Check for show point in animation . . .
	if (u8Event > 0)
		{
		CWeapon*	pweapon;
		if (m_pRealm->m_idbank.GetThingByID((CThing**)&pweapon, m_u16IdWeapon) == 0)
			{
			// If hidden . . .
			if (pweapon->m_eState == CWeapon::State_Hide)
				{
				// Show.
				pweapon->m_eState = CWeapon::State_Idle;
				}
			// Check for release point . . .
			else if (u8Event > 1)
				{
				// Release.
				ShootWeapon(u32BitsInclude, u32BitsDontcare, u32BitsExclude);
				
				bReleased	= true;
				}
			}
		else
			{
			// Check for release point . . .
			if (u8Event > 1)
				{
				// Release.
				ShootWeapon(u32BitsInclude, u32BitsDontcare, u32BitsExclude);
				
				bReleased	= true;
				}
			}
		}

	return bReleased;
	}

////////////////////////////////////////////////////////////////////////////////
// Implements basic one-time functionality for each time State_Dead is
// entered.
////////////////////////////////////////////////////////////////////////////////
void CCharacter::OnDead(void)
	{
	CHood*	phood	= m_pRealm->m_phood;
	// Render current dead frame into background to stay.
	m_pRealm->m_scene.DeadRender3D(
		phood->m_pimBackground,		// Destination image.
		&m_sprite,						// Tree of 3D sprites to render.
		phood);							// Dst clip rect.
	
	// If we just rendered a child weapon into the background . . .
	CThing*	pthing;
	if (m_pRealm->m_idbank.GetThingByID(&pthing, m_u16IdWeapon) == 0)
		{
		// It has a permanent place in the background and, therefore, is no
		// longer needed.

		// "Shoot" it (release it).
		ShootWeapon();
		// Delete it!
		GameMessage msg;
		msg.msg_ObjectDelete.eType = typeObjectDelete;
		msg.msg_ObjectDelete.sPriority = 0;
		SendThingMessage(&msg, pthing);
		}

	// Register death with the score module
	ScoreRegisterKill(m_pRealm, m_u16InstanceId, m_u16KillerId);
	}

////////////////////////////////////////////////////////////////////////////////
// Implements basic functionality while being run over and returns true
// until the state is completed.
////////////////////////////////////////////////////////////////////////////////
bool CCharacter::WhileRunOver(void)	// Returns true until state is complete.
	{
	return CThing3d::WhileRunOver();
	}

////////////////////////////////////////////////////////////////////////////////
// Implements one-time functionality for when a weapon is destroyed while
// we were moving it (i.e., before we let go or ShootWeapon()'ed it).
// This can occur when a weapon, while traveling along our rigid body,
// enters terrain.
////////////////////////////////////////////////////////////////////////////////
// virtual.
void CCharacter::OnWeaponDestroyed(void)
	{
	// Each higher object should implement this as needed for that type of object
	// or not at all.  Please call this base though, in case we ever add something
	// here.
	}

//---------------------------------------------------------------------------
// Useful generic character functionality.
//---------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
// Process the specified message.  For most messages, this function
// will call the equivalent On* function.
// (virtual).
////////////////////////////////////////////////////////////////////////////////
void CCharacter::ProcessMessage(		// Returns nothing.
	GameMessage* pmsg)					// Message to process.
	{
	// Call base class.
	CThing3d::ProcessMessage(pmsg);

	// Process character specific messages.
	switch (pmsg->msg_Generic.eType)
		{
		case typeDrawBlood:
			OnDrawBloodMsg(&(pmsg->msg_DrawBlood) );
			break;

		case typeSuicide:
			OnSuicideMsg(&(pmsg->msg_Suicide) );
			break;
		
		default:
			// Should this complain when it doesn't know a message type?
			break;
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Handles a msg_Shot.
// (virtual).
////////////////////////////////////////////////////////////////////////////////
void CCharacter::OnShotMsg(	// Returns nothing.
	Shot_Message* pshotmsg)		// In:  Message to handle.
	{
	ASSERT(pshotmsg->u16ShooterID != 0xebeb);
	m_u16KillerId = pshotmsg->u16ShooterID;

	CThing3d::OnShotMsg(pshotmsg);

	// If we're receiving any damage from the shot . . .
	if (pshotmsg->sDamage > 0)
		{
		MakeBloody(
			pshotmsg->sDamage, 
			pshotmsg->sAngle, 
			CHUNKS_DAMAGE_DIR_SWAY);
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Handles an Explosion_Message.
// (virtual).
////////////////////////////////////////////////////////////////////////////////
void CCharacter::OnExplosionMsg(			// Returns nothing.
	Explosion_Message* pexplosionmsg)	// In:  Message to handle.
	{
	ASSERT(pexplosionmsg->u16ShooterID != 0xebeb);
	m_u16KillerId = pexplosionmsg->u16ShooterID;

	MakeBloody(
		pexplosionmsg->sDamage, 
		180,		// Slightly more efficient than 0 (speeds up mod op).
		360);

	CThing3d::OnExplosionMsg(pexplosionmsg);
	}

////////////////////////////////////////////////////////////////////////////////
// Handles a Burn_Message.
// (virtual).
////////////////////////////////////////////////////////////////////////////////
void CCharacter::OnBurnMsg(	// Returns nothing.
	Burn_Message* pburnmsg)		// In:  Message to handle.
	{
	ASSERT(pburnmsg->u16ShooterID != 0xebeb);
	m_u16KillerId = pburnmsg->u16ShooterID;

	CThing3d::OnBurnMsg(pburnmsg);

	// If we are not yet in the burn state . . .
	if (m_state != State_Burning)
		{
		if (StatsAreAllowed) Stat_Burns++;
		// End of burn, if we don't die of 'natural' (being on fire) causes first.
		m_lCharacterTimer	= m_pRealm->m_time.GetGameTime() + BURN_DURATION;
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Handles an ObjectDelete_Message.
// (virtual).
////////////////////////////////////////////////////////////////////////////////
void CCharacter::OnDeleteMsg(				// Returns nothing.
	ObjectDelete_Message* pdeletemsg)	// In:  Message to handle.
	{
	CThing3d::OnDeleteMsg(pdeletemsg);
	}

////////////////////////////////////////////////////////////////////////////////
// Handles a DrawBlood_Message.
// (virtual).
////////////////////////////////////////////////////////////////////////////////
void CCharacter::OnDrawBloodMsg(				// Returns nothing.
	DrawBlood_Message* pdrawbloodmsg)		// In:  Message to handle.
	{
	BloodToBackground(pdrawbloodmsg->s2dX, pdrawbloodmsg->s2dY);
	}

////////////////////////////////////////////////////////////////////////////////
// Handles a Suicide_Message.
// (virtual).
////////////////////////////////////////////////////////////////////////////////
void CCharacter::OnSuicideMsg(		// Returns nothing.
	Suicide_Message* psuicidemsg)		// In:  Message to handle.
	{
	}

////////////////////////////////////////////////////////////////////////////////
// Handles a Help_Message
// (virtual)
////////////////////////////////////////////////////////////////////////////////

void CCharacter::OnHelpMsg(			// Returns nothing
	Help_Message* phelpmsg)
	{
	}

////////////////////////////////////////////////////////////////////////////////
// Handles a PutMeDown_Message
// (virtual)
////////////////////////////////////////////////////////////////////////////////

void CCharacter::OnPutMeDownMsg(		// Returns nothing
	PutMeDown_Message* pputmedownmsg)
	{
	}

////////////////////////////////////////////////////////////////////////////////
// Creates blood splat and pool animations.
////////////////////////////////////////////////////////////////////////////////
void CCharacter::MakeBloody(
	short sDamage,			// In:  Damage to base carnage on.
	short	sDamageAngle,	// In:  Angle in which (NOT from which) damage was
								// applied.
	short	sSwayRange)		// In:  Random amount chunks can sway from the
								// sDamageAngle (If 360, there'll be no noticeable
								// damage direction for the chunks).
	{
	double	dHitY	= m_dY + GetRandSway(BLOOD_SPLAT_SWAY, m_pRealm->m_phood->m_dScale3d);
	double	dHitX;
	double	dHitZ;

	// If writhing on the ground . . .
	if (m_state == State_Writhing)
		{
		// Keep the shot low and on our X/Z execution point.
		dHitX	= m_smash.m_sphere.sphere.X;
		dHitZ	= m_smash.m_sphere.sphere.Z;
		}
	else
		{
		// X/Z position depends on angle of shot (it is opposite).
		// That is, the wound appears on a portion of the dude facing the damage
		// source (where the bullet came from, the epicenter of the explosion, etc.)
		// giving us a little more feedback and realism.
		short	sDeflectionAngle	= rspMod360(sDamageAngle + 180);
		dHitX	= m_dX + COSQ[sDeflectionAngle] * TORSO_RADIUS;
		dHitZ	= m_dZ - SINQ[sDeflectionAngle] * TORSO_RADIUS;

		// Put it up about half way up character's body.
		dHitY	+= m_sprite.m_sRadius;
		}

	// Create blood animation.
	CAnimThing*	pat	= new CAnimThing(m_pRealm);
	if (pat != NULL)
		{
		strcpy(pat->m_szResName, BLOOD_SPLAT_RES_NAME);

		// Start it up:
		// No looping.
		pat->m_sLoop	= FALSE;
		// No notification necessary.
		// NOTE:  sAngle is currently not utilized.
		pat->Setup(dHitX, dHitY, dHitZ + 1);
		}

	// Create some chunks.
	short	sNumChunks	= MIN(sDamage / 2, MAX_CHUNKS);
	short	i;
	for (i = 0; i < sNumChunks; i++)
		{
		// Create blood particles . . .
		CChunk*	pchunk	= NULL;	// Initialized for safety.
		// Note that this will fail if particles are disabled.
		if (Construct(CChunkID, m_pRealm, (CThing**)&pchunk) == 0)
			{
			pchunk->Setup(
				dHitX,				// Source position.
				dHitY,				// Source position.
				dHitZ,				// Source position.
				sDamageAngle,		// Angle of velocity.
				sSwayRange,			// Angle sway.
				40,					// Velocity (X/Z plane).
				80,					// Velocity (X/Z plane) sway.
				50,					// Velocity (Vertical).
				100,					// Velocity (Vertical) sway.
				CChunk::Blood);	// Type of chunk.
			}
		}

	// Let's go ahead and create the pool at the same time.
	// If it gets bigger it might look like the above anim
	// created the pool which, of course, is the desired
	// effect.
	MakeBloodPool();
	}

////////////////////////////////////////////////////////////////////////////////
// Creates blood pool animation.
////////////////////////////////////////////////////////////////////////////////
void CCharacter::MakeBloodPool(void)
	{
	CAnimThing*	pat	= new CAnimThing(m_pRealm);
	if (pat != NULL)
		{
		strcpy(pat->m_szResName, BLOOD_POOL_RES_NAME);

		// Start it up:
		// No looping.
		pat->m_sLoop	= FALSE;
		// Need notification to tell us when to put the animation
		// in the background.
		pat->m_msg.msg_DrawBlood.eType		= typeDrawBlood;
		pat->m_msg.msg_DrawBlood.sPriority	= 0;
		pat->m_u16IdSendMsg						= m_u16InstanceId;

		double	dHitX			= m_dX + GetRandSway(BLOOD_SPLAT_SWAY, m_pRealm->m_phood->m_dScale3d);
		double	dHitZ			= m_dZ + GetRandSway(BLOOD_SPLAT_SWAY, m_pRealm->m_phood->m_dScale3d);
		// Make sure blood lands on the ground (terrain).
		double	dTerrainH	= m_pRealm->GetHeight(dHitX, dHitZ);

		Map3Dto2D(dHitX, dTerrainH, dHitZ,
			&(pat->m_msg.msg_DrawBlood.s2dX),
			&(pat->m_msg.msg_DrawBlood.s2dY) );

		// NOTE:  sAngle is currently not utilized.
		pat->Setup(dHitX, dTerrainH, dHitZ);
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Draws last frame of blood pool into background.
////////////////////////////////////////////////////////////////////////////////
void CCharacter::BloodToBackground(
	short	sAnimX2d,			// Position of animation in 2d.
	short	sAnimY2d)			// Position of animation in 2d.
	{
	// Draw last frame of pool directly into background.
	CAnimThing::ChannelAA*	paachannel;
	if (rspGetResource(&g_resmgrGame, m_pRealm->Make2dResPath(BLOOD_POOL_RES_NAME), &paachannel) == 0)
		{
		// Get last frame.
		CAlphaAnim*	paa = paachannel->GetItem(paachannel->NumItems() - 1);
		ASSERT(paa != NULL);
		
		short	sX	= sAnimX2d + paa->m_sX;
		short	sY	= sAnimY2d + paa->m_sY;
		
		// Note this does not handle alpha case yet.
		if (paa->m_pimAlphaArray != NULL)
			{
			RRect	rcClip(0, 0, 
				m_pRealm->m_phood->m_pimBackground->m_sWidth,
				m_pRealm->m_phood->m_pimBackground->m_sHeight);
			
			// Do the Alpha blit.
			rspGeneralAlphaBlit(
				m_pRealm->m_phood->m_pmaTransparency,	// Levels multialpha table.
				paa->m_pimAlphaArray,						// Alpha mask.
				&(paa->m_imColor),							// Src.
				m_pRealm->m_phood->m_pimBackground,		// Dst.
				sX,												// 2D Dst coord.
				sY,												// 2D Dst coord.
				rcClip);											// Dst.
			}
		else
			{
			// Regular transparency blit.
			rspBlit(
				&(paa->m_imColor),							// Src.
				m_pRealm->m_phood->m_pimBackground,		// Dst.
				sX,												// 2D Dst coord.
				sY,												// 2D Dst coord.
				NULL);											// Dst.
			}
		
		rspReleaseResource(&g_resmgrGame, &paachannel);
		}
	}


////////////////////////////////////////////////////////////////////////////////
// Prepare current weapon (ammo).
// This should be done when the character starts its shoot animation.
// (virtual).
////////////////////////////////////////////////////////////////////////////////
CWeapon* CCharacter::PrepareWeapon(void)	// Returns the weapon ptr or NULL.
	{
	CWeapon*	pweapon	= NULL;

	switch (m_eWeaponType)
	{
		case CPistolID:
		case CMachineGunID:
		case CShotGunID:
		case CAssaultWeaponID:
		case CDoubleBarrelID:
		case CUziID:
		case CAutoRifleID:
		case CSmallPistolID:
			break;

		default:
			if (ConstructWithID(m_eWeaponType, m_pRealm, (CThing**) &pweapon) == 0)
				{
				// Set its parent.
				pweapon->m_idParent = GetInstanceID();
				// Set it up.
				pweapon->Setup(0, 0, 0);
				pweapon->m_dRot = m_dRot;
				// Set its initial state to hidden.
				pweapon->m_eState = CWeapon::State_Hide;
				// Let the scene know to render the weapon as a child of this.
				m_sprite.AddChild(pweapon->GetSprite() );
				// Store ID.
				m_u16IdWeapon	= pweapon->GetInstanceID();
				}
			else
				{
				TRACE("PrepareWeapon(): Failed to construct new %s.\n",
					ms_aClassInfo[m_eWeaponType].pszClassName);
				}
			break;
	}

	return pweapon;
	}

////////////////////////////////////////////////////////////////////////////////
// Shoot current weapon.
// This should be done when the character releases the weapon it's
// shooting.
// (virtual).
////////////////////////////////////////////////////////////////////////////////
CWeapon* CCharacter::ShootWeapon(	// Returns the weapon ptr or NULL
	CSmash::Bits bitsInclude,			// Bits to use for bullet collision (enemies can specify different bits)
	CSmash::Bits bitsDontcare,			// Bits to use for bullet colllsion
	CSmash::Bits bitsExclude)			// Bits to use for bullet collision
	{
	CWeapon*	pweapon	= NULL;
	// Detatch the weapon.
	ASSERT(m_panimCur != NULL);
	
	// Get weapon's position relative to this character.
	double dWeaponRelX, dWeaponRelY, dWeaponRelZ;
	GetLinkPoint(
		m_panimCur->m_ptransRigid->GetAtTime(m_lAnimTime),
		&dWeaponRelX,
		&dWeaponRelY,
		&dWeaponRelZ);

	RP3d	pt3WeaponRel	= { static_cast<float>(dWeaponRelX), static_cast<float>(dWeaponRelY), static_cast<float>(dWeaponRelZ), 1 };

	switch (m_eWeaponType)
	{
		case CPistolID:
		case CMachineGunID:
		case CUziID:
		case CAutoRifleID:
		case CSmallPistolID:
			FireBullets(&pt3WeaponRel, 1, MAX_BULLET_RANGE, g_smidBulletFire, bitsInclude, bitsDontcare, bitsExclude);
			break;
		
		case CShotGunID:
			FireBullets(&pt3WeaponRel, NUM_SHOTS_PER_SHELL, MAX_SHELL_RANGE, g_smidShotgun, bitsInclude, bitsDontcare, bitsExclude);
			break;

		case CAssaultWeaponID:
			FireBullets(&pt3WeaponRel, NUM_SHOTS_PER_SHELL, MAX_SHELL_RANGE, g_smidSprayCannon, bitsInclude, bitsDontcare, bitsExclude);
			break;

		case CDoubleBarrelID:
			FireBullets(&pt3WeaponRel, 2 * NUM_SHOTS_PER_SHELL, MAX_SHELL_RANGE, g_smidDeathWadLaunch, bitsInclude, bitsDontcare, bitsExclude);

			// Double it.
			PlaySample(g_smidShotgun, SampleMaster::Weapon);
			break;

		case CFirestreamID:
			// If there's no sound going on already . . .
			if (m_siLastWeaponPlayInstance == 0)
				{
				PlaySample(										// Returns nothing.
																	// Does not fail.
					g_smidFlameThrower3,						// In:  Identifier of sample you want played.
					SampleMaster::Destruction,				// In:  Sound Volume Category for user adjustment
					255,											// In:  Initial Sound Volume (0 - 255)
					&m_siLastWeaponPlayInstance,			// Out: Handle for adjusting sound volume
					NULL,											// Out: Sample duration in ms, if not NULL.
					250,											// In:  Where to loop back to in milliseconds.
																	//	-1 indicates no looping (unless m_sLoop is
																	// explicitly set).
					750,											// In:  Where to loop back from in milliseconds.
																	// In:  If less than 1, the end + lLoopEndTime is used.
					false);										// In:  Call ReleaseAndPurge rather than Release after playing
				}

			m_lStopLoopingWeaponSoundTime	= m_pRealm->m_time.GetGameTime() + MAX_TIME_WITHOUT_FLAMAGE;

			// Intentional fall through.

		default:
			{
			ASSERT(m_u16IdWeapon != CIdBank::IdNil);

			if (m_pRealm->m_idbank.GetThingByID((CThing**)&pweapon, m_u16IdWeapon) == 0)
				{
				// Set weapon position to character's position offset by rigid body's realm offset.
				pweapon->m_dX = m_dX + pt3WeaponRel.x;
				pweapon->m_dY = m_dY + pt3WeaponRel.y;
				pweapon->m_dZ = m_dZ + pt3WeaponRel.z;
				// I guess we usually want to launch in the direction we are _currently_ facing and
				// not the direction we were in when we prepared the weapon.
				pweapon->m_dRot	= m_dRot;

				// Set the collision bits for the weapon
				pweapon->SetCollideBits(bitsInclude, bitsDontcare, bitsExclude);

				// Set Instance ID of the shooter
				pweapon->m_u16ShooterID = m_u16InstanceId;

				// Set the weapon in motion by changing its state.
				pweapon->m_eState = CWeapon::State_Fire;
				
				// Detach parent pointer
				pweapon->m_idParent = CIdBank::IdNil;
				// Detatch weapon's sprite
				CSprite*	pspriteWeapon	= pweapon->GetSprite();
				if (pspriteWeapon)
					{
					// Some weapons (one weapon, the flamer) are never parented.
					if (pspriteWeapon->m_psprParent)
						{
						m_sprite.RemoveChild(pspriteWeapon);
						}
					}

				// Done with weapon.
				m_u16IdWeapon = CIdBank::IdNil;
				
				// Specific behavior by type.
				switch (pweapon->GetClassID() )
					{
					case CDeathWadID:
						CDeathWad*	pdw	= (CDeathWad*)pweapon;

						// Let it take what it needs.
						pdw->FeedWad(&m_stockpile);

						break;
					}
				}
			break;
			}
	}

	// No longer exists.
	m_u16IdWeapon	= CIdBank::IdNil;

	return pweapon;
	}

////////////////////////////////////////////////////////////////////////////////
// Validate weapon position.  If invalid, the weapon is destroyed and
// the notification function, OnWeaponDestroyed() is called.
////////////////////////////////////////////////////////////////////////////////
bool CCharacter::ValidateWeaponPosition(void)	// Returns true, if weapon is in a valid position.
																// Returns false, if weapon destroyed because it  
																// it is not in a valid position.                 

	{
	bool	bValid	= true;	// Assume valid.

	if (m_u16IdWeapon != CIdBank::IdNil)
		{
		switch (m_eWeaponType)
			{
			case CGrenadeID:
			case CFirebombID:
			case CNapalmID:
			case CRocketID:
			case CHeatseekerID:
				{
				CWeapon*	pweapon;
				if (m_pRealm->m_idbank.GetThingByID((CThing**)&pweapon, m_u16IdWeapon) == 0)
					{
					// If this weapon is not hidden . . .
					if (pweapon->m_eState != CWeapon::State_Hide)
						{
						RTransform*	ptransWeapon	= m_panimCur->m_ptransRigid->GetAtTime(m_lAnimTime);

						// Get the position to check for terrain obstacles.
						// Remember this is an offset from our hotspot (origin).
						double	dX, dY, dZ;
						GetLinkPoint(ptransWeapon, &dX, &dY, &dZ);

						// Check position to make sure it's not inside terrain.
						short	sTerrainH	= m_pRealm->GetHeight(m_dX + dX, m_dZ + dZ);
						if (sTerrainH > m_dY + dY)
							{
							// Send the object a delete message.
							GameMessage	msg;
							msg.msg_ObjectDelete.eType		= typeObjectDelete;
							msg.msg_ObjectDelete.sPriority	= 0;
							SendThingMessage(&msg, pweapon);

							// Clear our instance ID.
							m_u16IdWeapon	= CIdBank::IdNil;

							// Call the notify function.
							OnWeaponDestroyed();

							bValid	= false;
							}
						}
					}
				else
					{
					// Clear our instance ID.
					m_u16IdWeapon	= CIdBank::IdNil;
					}
				break;
				}
			default:
				break;
			}
		}

	return bValid;
	}

////////////////////////////////////////////////////////////////////////////////
// Fire some bullets.
////////////////////////////////////////////////////////////////////////////////
bool CCharacter::FireBullets(				// Returns true, if we hit someone/thing.
	RP3d*				ppt3d,					// In:  Launch pt in Postal units.
	short				sNumShots,				// In:  Number of shots to fire.
	short				sRange,					// In:  Bullet range.
	SampleMasterID	smidAmmo,				// In:  Ammo noise.
	CSmash::Bits	bitsInclude,/*=0*/	// In:  Optional bits we can hit
	CSmash::Bits	bitsDontcare,			// In:  Optional bits not involved in collision
	CSmash::Bits	bitsExclude)			// In:  Optional bits excluded from collision
	{
	bool	bHit	= false;	// Assume no hit.

	if (bitsInclude == 0)
		{
		bitsInclude	= CSmash::Character | CSmash::Barrel | CSmash::Mine;
		}

	// Never shoot dead people or writhers.
	bitsExclude |= (CSmash::Dead | CSmash::AlmostDead);

	const bool isPlayer = (m_id == CDudeID);  // !!! FIXME: make sure it's not a remote multiplayer dude.
	if (isPlayer)
		{
		if (StatsAreAllowed)
			{
			Stat_BulletsFired += sNumShots;
			if (Stat_BulletsFired >= 1000000)
				UnlockAchievement(ACHIEVEMENT_FIRE_1000000_BULLETS);
			}
		}

	short	i;
	for (i = 0; i < sNumShots; i ++)
		{
		short	sX, sY, sZ;	
		short	sFireAngle	= m_dRot + GetRandSway(FIRE_ANGLE_Y_SWAY, m_pRealm->m_phood->m_dScale3d);
		CThing*	pthingTarget;
		bool bResult = m_bullets.FireDeluxe(
			sFireAngle,				// In:  Angle of launch in degrees (on X/Z plane).
			GetRandSway(FIRE_ANGLE_Z_SWAY, m_pRealm->m_phood->m_dScale3d),				// In:  Angle of launch in degrees (on X/Y plane).
			m_dX + ppt3d->x,		// In:  Launch position.
			m_dY + ppt3d->y,		// In:  Launch position.
			m_dZ + ppt3d->z,		// In:  Launch position.
			sRange,					// In:  Maximum distance.
			m_pRealm,				// In:  Realm in which to fire.
			bitsInclude,			// In:  Mask of CSmash masks that this bullet can hit.
			bitsDontcare,			// In:  Mask of CSmash masks that this bullet does not care to hit.
			bitsExclude,			// In:  Mask of CSmash masks that this bullet cannot hit.
			20,						// In:  Maximum angle with terrain that can cause
										// a ricochet (on X/Z plane).
			2,							// In:  The maximum number of ricochets.
			&sX,						// Out: Terrain hit position (i.e., where the bullet
										// would stop if no CThing collisions occurred).
			&sY,						// Out: Terrain hit position (i.e., where the bullet
										// would stop if no CThing collisions occurred).
			&sZ,						// Out: Terrain hit position (i.e., where the bullet
										// would stop if no CThing collisions occurred).
			&pthingTarget,			// Out: Ptr to thing hit or NULL.
			true,						// In:  Draw a tracer at random point along path.
			(i == 0) ? smidAmmo : g_smidNil)	// In:  Use ammo sample.
			
			;
		//!! FIXME: this should simply miss if we shoot ourself. Cheap hack for dude shooting himself in twinstick mode.
		if (pthingTarget != this && bResult)
			{

			// Create a message.

			// Note that right here we can do the tuning on the amount of bullet
			// damage based on m_eWeaponType and the pthingTarget->m_id == CDudeID and
			// this->m_id == CDudeID.

			GameMessage	msg;
			msg.msg_Shot.eType			= typeShot;
			msg.msg_Shot.sPriority		= 0;
			msg.msg_Shot.sAngle			= sFireAngle;
			msg.msg_Shot.u16ShooterID	= m_u16InstanceId;

			short sIndex = 0;
			// Figure out how much damage the bullet should give
			switch (m_eWeaponType)
			{
				case CSmallPistolID:
				case CPistolID:
					sIndex = WEAPON_IS_PISTOL;
					break;
				case CUziID:
				case CAutoRifleID:
				case CMachineGunID:
					sIndex = WEAPON_IS_MACHINEGUN;
					break;
				case CShotGunID:
					sIndex = WEAPON_IS_SHOTGUN;
					break;
				case CAssaultWeaponID:
					sIndex = WEAPON_IS_ASSAULT;
					break;
				case CDoubleBarrelID:
					sIndex = WEAPON_IS_SHOTGUN;
					// There's additional damage via other means.
					break;
			}
			// See if the shooter is a CDude
			if (m_id == CDudeID)
				sIndex |= SHOOTER_IS_DUDE;
			// See if the target is a CDude
			if (pthingTarget->GetClassID() == CDudeID)
				sIndex |= TARGET_IS_DUDE;

			// Based on these parameters, look up the damage in the damage chart.
			msg.msg_Shot.sDamage = ms_asBulletDamageChart[sIndex];

			// Send it the message.
			SendThingMessage(&msg, pthingTarget);

			if (m_eWeaponType == CDoubleBarrelID)
				{
				// This is gonna hurt.
				msg.msg_Explosion.eType = typeExplosion;
				msg.msg_Explosion.sPriority = 0;
				// Spread this out.
				msg.msg_Explosion.sDamage = ms_asBulletDamageChart[sIndex] / sNumShots + 1;
				// Just in front of where bullet hit.
				short	sAngle	= rspMod360(m_dRot);
				msg.msg_Explosion.sX = (short) pthingTarget->GetX() - COSQ[sAngle] * 10;
				msg.msg_Explosion.sY = (short) pthingTarget->GetY();
				msg.msg_Explosion.sZ = (short) pthingTarget->GetZ() + SINQ[sAngle] * 10;
				msg.msg_Explosion.sVelocity = 100;
				msg.msg_Explosion.u16ShooterID = GetInstanceID();

				SendThingMessage(&msg, pthingTarget);
				}

			// Note we hit something.
			bHit	= true;
			if ((isPlayer) && (StatsAreAllowed))
				{
				Stat_BulletsHit++;
				if (Stat_BulletsHit >= 100000)
					UnlockAchievement(ACHIEVEMENT_HIT_100000_TARGETS);
				}
			}
		else
			{
			if ((isPlayer) && (StatsAreAllowed)) Stat_BulletsMissed++;
			}
		}

	// Create shells/casings . . .
	CChunk*	pchunk	= NULL;	// Initialized for safety.
	// Note that this will fail if particles are disabled.
	if (Construct(CChunkID, m_pRealm, (CThing**)&pchunk) == 0)
		{
		pchunk->Setup(
			m_dX + ppt3d->x,			// Source position.
			m_dY + ppt3d->y,			// Source position.
			m_dZ + ppt3d->z,			// Source position.
			m_dRot + 90,				// Angle of velocity.
			0,								// Angle sway.
			30,							// Velocity (X/Z plane).
			20,							// Velocity sway.
			37,							// Velocity (Vertical).
			25,							// Velocity (Vertical) sway.
			(sNumShots > 1) ? CChunk::Shell : CChunk::BulletCasing);	// In:  Type of chunk.
		}

	return bHit;
	}

////////////////////////////////////////////////////////////////////////////////
// Determine if a path is clear of items identified by the specified 
// smash bits and terrain.
////////////////////////////////////////////////////////////////////////////////
bool CCharacter::IsPathClear(	// Returns true, if the entire path is clear.
										// Returns false, if only a portion of the path is clear.
										// (see *psX, *psY, *psZ, *ppthing).
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
	short* psX,						// Out: Point of intercept, if any, on path.
	short* psY,						// Out: Point of intercept, if any, on path.
	short* psZ,						// Out: Point of intercept, if any, on path.
	CThing** ppthing,				// Out: Thing that intercepted us or NULL, if none.
	CSmash*	psmashExclude/*= NULL*/)	// In:  Optional CSmash to exclude or NULL, if none.
	{
	bool	bEntirelyClear	= false;	// Assume entire path is not clear.

	/////////// Determine length of travel if no CThings hit ///////////////////

	// Get most efficient increments that won't miss any attributes.
	// For the rates we use trig with a hypotenuse of 1 which will give
	// us a rate <= 1.0 and then multiply by the the crawl for
	// a reasonable increase in the speed of this alg.
	
	// sAngle must be between 0 and 359.
	sRotY	= rspMod360(sRotY);

	float	fRateX		= COSQ[sRotY] * sCrawlRate;
	float	fRateZ		= -SINQ[sRotY] * sCrawlRate;
	float	fRateY		= 0.0;	// If we ever want vertical movement . . .

	// Set initial position to first point to check (NEVER checks original position).
	float	fPosX			= sX + fRateX;
	float	fPosY			= sY + fRateY;
	float	fPosZ			= sZ + fRateZ;

	// Determine amount traveled per iteration on X/Z plane just once.
	float	fIterDistXZ		= sqrt(ABS2(fRateX, fRateZ) );

	float	fTotalDistXZ	= 0.0F;

	// Store extents.
	short	sMaxX			= m_pRealm->GetRealmWidth();
	short	sMaxY			= 512;					// Robustness: Guard against infinite loop
														// in the remote possibility that we shoot
														// straight up (currently one can only shoot
														// horizontally).
	short	sMaxZ			= m_pRealm->GetRealmHeight();

	short	sMinX			= 0;
	short	sMinY			= -512;
	short	sMinZ			= 0;

	short	sCurH;
	U16	u16Attribute;

	// Scan while in realm.
	while (
			fPosX > sMinX 
		&& fPosY > sMinY 
		&& fPosZ > sMinZ 
		&& fPosX < sMaxX 
		&& fPosY < sMaxY 
		&& fPosZ < sMaxZ
		&& fTotalDistXZ < sRangeXZ)
		{
		GetFloorAttributes((short)fPosX, (short)fPosZ, &u16Attribute, &sCurH);
		// If too big a height difference or completely not walkable . . .
		if (	(u16Attribute & REALM_ATTR_NOT_WALKABLE)
			|| (sCurH - fPosY > sVerticalTolerance) )
			{
			break;
			}

		// Update position.
		fPosX	+= fRateX;
		fPosY	=	sCurH;
		fPosZ	+= fRateZ;
		// Update distance travelled on X/Z plane.
		fTotalDistXZ	+= fIterDistXZ;
		}

	// Check 3D line segments outlining path.
	R3DLine	line1, line2;
	if (fRateX > 0.0F)
		{
		line1.X1	= sX - sRadius;
		line1.X2	= fPosX - sRadius;
		line2.X1	= sX - sRadius;
		line2.X2	= fPosX - sRadius;
		}
	else
		{
		line1.X1	= sX + sRadius;
		line1.X2	= fPosX + sRadius;
		line2.X1	= sX + sRadius;
		line2.X2	= fPosX + sRadius;
		}

	if (fRateY > 0.0F)
		{
		line1.Y1	= sY - sRadius;
		line1.Y2	= fPosY - sRadius;
		line2.Y1	= sY - sRadius;
		line2.Y2	= fPosY - sRadius;
		}
	else
		{
		line1.Y1	= sY + sRadius;
		line1.Y2	= fPosY + sRadius;
		line2.Y1	= sY + sRadius;
		line2.Y2	= fPosY + sRadius;
		}

	if (fRateZ > 0.0F)
		{
		line1.Z1	= sZ - sRadius;
		line1.Z2	= fPosZ - sRadius;
		line2.Z1	= sZ - sRadius;
		line2.Z2	= fPosZ - sRadius;
		}
	else
		{
		line1.Z1	= sZ + sRadius;
		line1.Z2	= fPosZ + sRadius;
		line2.Z1	= sZ + sRadius;
		line2.Z2	= fPosZ + sRadius;
		}

	CSmash*	psmashClosest	= NULL;
	// Determine if anything with specified smash description was hit on along each edge . . .
	CSmash*	psmash1;
	if (m_pRealm->m_smashatorium.QuickCheckClosest(
		&line1, 
		bitsInclude, 
		bitsDontCare, 
		bitsExclude, 
		&psmash1,
		psmashExclude) == false)
		{
		psmash1	= NULL;
		}
	CSmash*	psmash2;
	if (m_pRealm->m_smashatorium.QuickCheckClosest(
		&line2, 
		bitsInclude, 
		bitsDontCare, 
		bitsExclude, 
		&psmash2,
		psmashExclude) == false)
		{
		psmash2	= NULL;
		}

	// If two smashes found . . .
	if (psmash1 != NULL && psmash2 != NULL && psmash1 != psmash2)
		{
		// Determine closer on X/Z plane.
		if (	ABS2(psmash1->m_sphere.sphere.X, psmash1->m_sphere.sphere.Y)
			<	ABS2(psmash2->m_sphere.sphere.X, psmash2->m_sphere.sphere.Y) )
			{
			psmashClosest	= psmash1;
			}
		else
			{
			psmashClosest	= psmash2;
			}
		}
	else
		{
		// Whichever is not NULL.
		if (psmash1 != NULL)
			{
			psmashClosest	= psmash1;
			}
		else
			{
			psmashClosest	= psmash2;
			}
		}

	// If anything hit . . . 
	if (psmashClosest != NULL)
		{
		// Set *ppthing to thing hit.
		*ppthing	= psmashClosest->m_pThing;

		// Set end pt.
		*psX		= psmashClosest->m_sphere.sphere.X;
		*psY		= psmashClosest->m_sphere.sphere.Y;
		*psZ		= psmashClosest->m_sphere.sphere.Z;
		}
	else
		{
		// Clear thing ptr.
		*ppthing	= NULL;
		// Set end pt.
		*psX		= fPosX;
		*psY		= fPosY;
		*psZ		= fPosZ;

		// If we made it the whole way . . .
		if (fTotalDistXZ >= sRangeXZ)
			{
			bEntirelyClear	= true;
			}
		}

#if 0
	// FEEDBACK.
	// Create a line sprite.
	CSpriteLine2d*	psl2d	= new CSpriteLine2d;
	if (psl2d != NULL)
		{
		Map3Dto2D(
			sX, 
			sY, 
			sZ, 
			&(psl2d->m_sX2), 
			&(psl2d->m_sY2) );
		Map3Dto2D(
			*psX, 
			*psY, 
			*psZ, 
			&(psl2d->m_sX2End), 
			&(psl2d->m_sY2End) );
		psl2d->m_sPriority	= sZ;
		psl2d->m_sLayer		= CRealm::GetLayerViaAttrib(m_pRealm->GetLayer(sX, sZ));
		psl2d->m_u8Color		= (bEntirelyClear == false) ? 249 : 250;
		// Destroy when done.
		psl2d->m_sInFlags	= CSprite::InDeleteOnRender;
		// Put 'er there.
		m_pRealm->m_scene.UpdateSprite(psl2d);
		}
#endif

	return bEntirelyClear;
	}


////////////////////////////////////////////////////////////////////////////////
// Light up target if there is one within a cone in the given aiming direction.
////////////////////////////////////////////////////////////////////////////////
bool CCharacter::IlluminateTarget(			// Returns true if there is a target
	short sX,							// In:  Starting x position
	short sY,							// In:  Starting y position
	short sZ,							// In:  Starting z position
	short sRotY,						// In:  Aiming direction (rotation around y axis)
	short sRangeXZ,					// In:  Range on X/Z plane
	short sRadius,						// In:  Radius of path traverser.
	CSmash::Bits bitsInclude,		// In:  Mask of CSmash bits that would count as a hit
	CSmash::Bits bitsDontCare,		// In:  Mask of CSmash bits that would not affect path
	CSmash::Bits bitsExclude,		// In:  Mask of CSmash bits that cannot affect path
	CThing** hThing,					// Out: Handle to thing that is the Target or NULL if none
	CSmash* psmashExclude)			// In: Optional CSmash to exclude or NULL, if none. 
{
	bool bTargetFound = false;

	// sAngle must be between 0 and 359.
	sRotY	= rspMod360(sRotY);

	float	fRateX		= COSQ[sRotY] * sRangeXZ;	
	float	fRateZ		= -SINQ[sRotY] * sRangeXZ;
	float	fRateY		= 0.0;	// If we ever want vertical movement . . .

	// Set initial position to first point to check (NEVER checks original position).
	float	fPosX			= sX + fRateX;
	float	fPosY			= sY + fRateY;
	float	fPosZ			= sZ + fRateZ;

	// Check 3D line segments outlining path.
	R3DLine	line1, line2;
	if (fRateX > 0.0F)
		{
		line1.X1	= sX - sRadius;
		line1.X2	= fPosX - sRadius;
		line2.X1	= sX - sRadius;
		line2.X2	= fPosX - sRadius;
		}
	else
		{
		line1.X1	= sX + sRadius;
		line1.X2	= fPosX + sRadius;
		line2.X1	= sX + sRadius;
		line2.X2	= fPosX + sRadius;
		}

	if (fRateY > 0.0F)
		{
		line1.Y1	= sY - sRadius;
		line1.Y2	= fPosY - sRadius;
		line2.Y1	= sY - sRadius;
		line2.Y2	= fPosY - sRadius;
		}
	else
		{
		line1.Y1	= sY + sRadius;
		line1.Y2	= fPosY + sRadius;
		line2.Y1	= sY + sRadius;
		line2.Y2	= fPosY + sRadius;
		}

	if (fRateZ > 0.0F)
		{
		line1.Z1	= sZ - sRadius;
		line1.Z2	= fPosZ - sRadius;
		line2.Z1	= sZ - sRadius;
		line2.Z2	= fPosZ - sRadius;
		}
	else
		{
		line1.Z1	= sZ + sRadius;
		line1.Z2	= fPosZ + sRadius;
		line2.Z1	= sZ + sRadius;
		line2.Z2	= fPosZ + sRadius;
		}

	CSmash*	psmashClosest	= NULL;
	// Determine if anything with specified smash description was hit on along each edge . . .
	CSmash*	psmash1;
	if (m_pRealm->m_smashatorium.QuickCheckClosest(
		&line1, 
		bitsInclude, 
		bitsDontCare, 
		bitsExclude, 
		&psmash1,
		psmashExclude) == false)
		{
		psmash1	= NULL;
		}
	CSmash*	psmash2;
	if (m_pRealm->m_smashatorium.QuickCheckClosest(
		&line2, 
		bitsInclude, 
		bitsDontCare, 
		bitsExclude, 
		&psmash2,
		psmashExclude) == false)
		{
		psmash2	= NULL;
		}

	// If two smashes found . . .
	if (psmash1 != NULL && psmash2 != NULL && psmash1 != psmash2)
		{
		// Determine closer on X/Z plane.
		if (	ABS2(psmash1->m_sphere.sphere.X, psmash1->m_sphere.sphere.Y)
			<	ABS2(psmash2->m_sphere.sphere.X, psmash2->m_sphere.sphere.Y) )
			{
			psmashClosest	= psmash1;
			}
		else
			{
			psmashClosest	= psmash2;
			}
		}
	else
		{
		// Whichever is not NULL.
		if (psmash1 != NULL)
			{
			psmashClosest	= psmash1;
			}
		else
			{
			psmashClosest	= psmash2;
			}
		}

	// If anything hit . . . 
	if (psmashClosest != NULL)
		{
		// Set *ppthing to thing hit.
		*hThing	= psmashClosest->m_pThing;
		bTargetFound = true;
		// Set end pt.
//		*psX		= psmashClosest->m_sphere.sphere.X;
//		*psY		= psmashClosest->m_sphere.sphere.Y;
//		*psZ		= psmashClosest->m_sphere.sphere.Z;
		}
	else
		{
		// Clear thing ptr.
		*hThing	= NULL;
		// Set end pt.
//		*psX		= fPosX;
//		*psY		= fPosY;
//		*psZ		= fPosZ;

		}

#if 0
	// FEEDBACK.
	// Create a line sprite.
	CSpriteLine2d*	psl2d	= new CSpriteLine2d;
	if (psl2d != NULL)
		{
		Map3Dto2D(
			sX, 
			sY, 
			sZ, 
			&(psl2d->m_sX2), 
			&(psl2d->m_sY2) );
		Map3Dto2D(
			*psX, 
			*psY, 
			*psZ, 
			&(psl2d->m_sX2End), 
			&(psl2d->m_sY2End) );
		psl2d->m_sPriority	= sZ;
		psl2d->m_sLayer		= CRealm::GetLayerViaAttrib(m_pRealm->GetLayer(sX, sZ));
		psl2d->m_u8Color		= (bEntirelyClear == false) ? 249 : 250;
		// Destroy when done.
		psl2d->m_sInFlags	= CSprite::InDeleteOnRender;
		// Put 'er there.
		m_pRealm->m_scene.UpdateSprite(psl2d);
		}
#endif

	return bTargetFound;
}

////////////////////////////////////////////////////////////////////////////////
// Preload - cache the anims that may be used.
// (static).
////////////////////////////////////////////////////////////////////////////////
short CCharacter::Preload(
	CRealm* prealm)				// In:  Calling realm.
	{
	CAnimThing::ChannelAA*	paaCache;
	short	sResult	= 0;

	if (rspGetResource(&g_resmgrGame, prealm->Make2dResPath(BLOOD_SPLAT_RES_NAME), &paaCache) == 0)
		rspReleaseResource(&g_resmgrGame, &paaCache);
	else
		sResult |= 1;

	if (rspGetResource(&g_resmgrGame, prealm->Make2dResPath(BLOOD_POOL_RES_NAME), &paaCache) == 0)
		rspReleaseResource(&g_resmgrGame, &paaCache);
	else
		sResult |= 1;

	// Tell samplemaster to cache (preload) these samples.
	CacheSample(g_smidBulletFire);
	CacheSample(g_smidShotgun);

	CBulletFest::Preload(prealm);

	return sResult;
	}

////////////////////////////////////////////////////////////////////////////////
// Called by destructor to clean up.
////////////////////////////////////////////////////////////////////////////////
void CCharacter::Kill(void)
	{
	// If we have a weapon sound play instance . . .
	if (m_siLastWeaponPlayInstance)
		{
		// Stop looping the sound.
		StopLoopingSample(m_siLastWeaponPlayInstance);
		// Forget about it.
		m_siLastWeaponPlayInstance	= 0;
		}
	}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
