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
// dude.cpp
// Project: Postal
//
// This module implements the CDude class, which is the main, player-controlled
// character in the game.
//
// History:
//		01/12/97 MJR	Started.
//
//		01/15/97 BRH	Changed 3D to 2D render position to subtract the
//							Y rather than Add it.
//
//		01/23/97	JMI	Now calls CRealm::GetLayerFromAttrib() in Render() to
//							get on the proper layer.
//
//		01/31/97 BRH	Added the ability to throw Grenades.
//
//		01/29/97	JMI	Now Load() and Save() call the base class versions.
//
//		02/04/97	JMI	Changed LoadDib() call to Load() (which now supports
//							loading of DIBs).
//
//		02/07/97 BRH	Changed to using the single frame 3D wire frame
//							instead of the 2D sprite.
//
//		02/07/97	JMI	Added XRay flag to dude.
//
//		02/07/97	JMI	GetResources() never closed its RFile.  Fixed.
//
//		02/07/97	JMI	Now CScene has its own pipeline, so we don't need ours
//							anymore.  And we don't need to set any of that schtuff
//							up.  Removed all the pipeline initialization stuff.
//
//		02/10/97	JMI	Added the 3D animation for walking (or is it running?).
//
//		02/11/97	JMI	Added the 3D animation for throwing and an associated
//							rigid body transform for a grenade.
//
//		02/12/97	JMI	Forgot to set m_pGrenade->m_dRot again on release so the
//							grenade would have correct trajectory even if the dude
//							had rotated since the start of the throw.  Fixed.
//
//		02/12/97	JMI	Now animates backwards for running backwards.
//
//		02/13/97	JMI	Changing RForm3d to RSop.
//
//		02/13/97	JMI	Now the dude maps his textures to the palette on load.
//
//		02/14/97	JMI	Removed Remap().  This'll now be done by a utility before
//							we even load the textures.
//							Also, changed the direction and rotation the guy does.
//							Currently, his position is hosed but I need to check it
//							in to get it at home.
//
//		02/17/97	JMI	Now uses actual standing anim for standing state.
//							Changed resource filenames to not use two '.'s.
//							Now uses new m_pRealm->m_scene.TransformPtsToRealm() func
//							to get relative position of grenade on release.
//							For the mapping from 3D to 2D, now does not take radius
//							into account b/c the scene actually draws relative to the
//							origin.
//							Added UpdateRadius() function to get the current 
//							m_sSphereRadius of the guy.  But this is incorrect b/c 
//							we should not be using m_sSphereRadius, but instead some-
//							thing like m_sCylinderR and m_sCylinderH to determine
//							the xray position, EditRect(), and EditHotSpot().
//
//		02/17/97	JMI	Now changes the lighting if the lighting bit is set in the
//							checked attribute zone.
//
//		02/18/97 MJR	Tuned guy turning and running speeds, added use of "shift"
//							key to run faster, switched all animations over to newly
//							existant main-guy versions.
//
//		02/18/97	JMI	Added firing of bullets.
//							Also, removed #if 0, #elif 0, .... blocks.
//							Also, added dieing animation and die state.
//							Now goes into Die state when hit in response to several
//							messages.
//
//		02/19/97	JMI	Now passes more masks to FireDeluxe() and uses Character
//							bit.
//							Also, does not subtract 90 before blitting.
//
//		02/19/97	JMI	Slowed bullets to MAX_BULLETS_PER_SEC.
//
//		02/19/97	JMI	Enhanced SetState().  Not it checks via its current state
//							if it's okay to change to the new state, and if it's okay,
//							cleans up the old state, sets the new state, and returns 
//							true, if successful.
//
//		02/19/97	JMI	Added shoot while still and while running.
//
//		02/19/97	JMI	Changed main_rungun.* to main_run.* for running anim b/c
//							main_rungun looked too much like main_runshoot.
//							Also, now cannot rotate at user request when dead.
//							Now has a running and standing shooting anim.
//							Encapsulated release of grenade into ReleaseGrenade() so
//							it can be reused.
//							Fixed a bunch of little things in SetState().
//
//		02/19/97	JMI	Brightness tuned slightly due to everything seeming to
//							change in brightness after tonight's assets fest.
//
//		02/19/97	JMI	Added damage state.
//
//		02/20/97 MJR	Added use of new input functions as part of enabling
//							multiplayer network mode.
//
//		02/20/97	JMI	Changed bit masks to FireDeluxe so CDudes can shoot any
//							other characters.
//
//		02/20/97	JMI	Now processes delete message.
//
//		02/20/97	JMI	Added blood and now sets m_sprite flags only once (in 
//							Init()).
//
//		02/21/97	JMI	Had forgotten to release the 3d/main_shot.* resource.  
//							Fixed.
//
//		02/21/97	JMI	ThrowRelease was allowing any state change when it was the
//							current state.
//							Also, now if the dude changes animations during a throw, 
//							the grenade just drops instead of exploding immediately.
//
//		02/21/97	JMI	The grenade now rolls off randomly when dropped accidently.
//
//		02/23/97	JMI	Now any state can jump out of damage state.
//
//		02/24/97	JMI	Use 3D 'link'-like pt for bullet start position.
//
//		02/24/97	JMI	Better blood splat and added blood pool.
//
//		02/24/97	JMI	No longer sets the m_type member of the m_sprite b/c it
//							is set by m_sprite's constructor.
//
//		02/24/97	JMI	Now State_Damage cannot interrupt State_Damage which looks
//							a lot better.  It would look even better still, I think,
//							if the shot animation was a little shorter/faster.
//
//		02/24/97	JMI	Now tweaks angle slightly before calling FireDeluxe() so
//							the gun does not aim perfectly.
//
//		02/24/97	JMI	Added on-fire state.
//
//		02/24/97	JMI	Tweaked on-fire state.
//
//		02/24/97	JMI	Changed reference to CAnimThing's m_pthingSendMsg member to
//							m_u16IdSendMsg.
//
//		02/24/97	JMI	No longer bleeds due to fire.
//
//		02/25/97	JMI	Added new burning yell sound.
//
//		02/26/97	JMI	Now <Control> fires weapon and <Alt> throws grenade.
//
//		03/03/97	JMI	Added m_statePersistent.
//							Now sets m_statePersistent if State_Damage interrupts 
//							State_Burning.
//
//		03/03/97	JMI	Now passes CSmash::Dead as a bit that FireDeluxe() will not
//							hit.
//							Now sets m_smash.m_bits to include CSmash::Dead when we
//							reach State_Dead and removes it again when be leave 
//							State_Dead.
//
//		03/03/97	JMI	Added states for strafing and multiweapon logic.
//							Currently, loads run and shoot for strafe anims which
//							should do until the strafe equivalents are ready.
//
//		03/03/97	JMI	Changed m_pGrenade to m_pWeapon (a CWeapon* instead of
//							a CGrenade*) so more weapons can be handled generically.
//
//		03/03/97	JMI	Now CDude is based upon CCharacter.
//							Not much base class functionality utilized yet, though.
//
//		03/04/97	JMI	Now utilizes more of the base class functionality.
//							All CCharacter::Edit*() functions are used.
//							Now uses real strafe animations.
//							Moved stuff I was incorrectly doing in Render() into
//							Update() so CDude now uses CCharacter::Render().
//							Update() now uses functions in CCharacter (i.e., 
//							UpdateVelocities(), GetNewPosition(), and 
//							MakeValidPosition() ) to process acceleration, 
//							velocities, and positions.
//
//		03/04/97	JMI	Got rid of #if 0'd out code in Update().
//
//		03/04/97	JMI	Now utilizes message overrides from CCharacter.
//
//		03/04/97	JMI	Removed UpdateRadius().
//							Now calls CCharacter::UpdateFirePosition() to keep fire
//							position up to date.
//							Converted all rspMod360() calls to new calling convention.
//							Removed m_panimthing for fire (now uses base class fire).
//							Removed ProcessMessages() (now base class is fine).
//
//		03/05/97	JMI	Was checking for fire gone incorrectly.  Fixed.
//
//		03/05/97	JMI	Now fires weapons (except SemiAutomatic) through base class.
//							Eventually, might move that to base class too??
//
//		03/05/97	JMI	Removed ReleaseWeapon() now uses CCharacter::ShootWeapon().
//
//		03/05/97	JMI	m_pWeapon no longer referenced.
//							Uses new multishot animation for when he's been shot.
//
//		03/05/97	JMI	Removed ms_apt3dAttribCheck (these points are now stored
//							and checked in CCharacer).
//							Added Suicide.
//
//		03/06/97	JMI	Drag no longer gets set to zero when velocity reaches zero
//							by this class (now handled by base class).
//							Added override to OnSuicideMsg().
//							Added a little fudge to BrainSplat location.
//
//		03/06/97	JMI	No longer does hack to get ID from m_pWeapon since the base
//							class now stores the ID in m_u16IdWeapon (and does not
//							rely on m_pWeapon to reference the weapon).
//
//		03/06/97	JMI	Now sets his velocity to 0.0 when committing suicide.
//
//		03/06/97	JMI	Changed fudge factor delay for suicide brain splat to
//							1400 ms (was 700 ms).
//
//		03/06/97	JMI	User can no longer influence velocity when dude is being 
//							shot.
//
//		03/13/97	JMI	Added EditModify() member function.
//							Also, added m_sNumFireBombs/Missiles/Napalms.
//							Load now takes a version number.
//
//		03/13/97	JMI	Added file format version 2 load stuff:
//							m_sNumFireBombs/Missiles/Napalms.
//							Now considers amount of particular weapon left before
//							firing it.
//
//		03/13/97	JMI	Added DrawStatus() member function.
//							Now can run out of bullets.
//							BUG:  Wierd glitchiness when firing empty gun and running
//							which is exaggerated by the distance between shots.  Longer
//							times between empty shots seem to cause more noticable 
//							glitches.
//
//		03/13/97	JMI	Dude now takes additional burn damage while on fire.
//
//		03/14/97	JMI	Changed the fire angle sway to 20 (was 10).
//
//		03/14/97	JMI	Added very simple jump.
//
//		03/21/97	JMI	Added launching animation.
//
//		03/21/97	JMI	Now deletes launched weapon when launch is interrupted.
//							Also, added a minimum time to the got shot animation.
//
//		03/24/97	JMI	Now utilize new input rotation embedded in UINPUT value.
//
//		03/25/97	JMI	Now uses main_runnogun*.* for run animation.
//
//		03/26/97	JMI	Changed STATUS_FONT_SIZE from 30 to 24.
//
//		03/26/97	JMI	When I added the rotation value built into the UINPUT,
//							and then ignored the rotation value when dying, the net
//							effect was the guy would snap to rotation 0, when dying.
//							Fixed.
//							Also, when I switched to having the guy get damaged 
//							more than just once by the same fire, it caused the dude
//							to react to fire even if we was previously dead.  Fixed.
//
//		03/27/97	JMI	Added many animations.
//
//		03/27/97	JMI	Timing for blown up animation was being done through the
//							rigid body transform for some unknown reason.  Fixed.
//
//		03/27/97	JMI	Can no longer move forward when ducking, exploding,
//							or getting up from explosion.
//
//		03/28/97	JMI	Now dude will enter the blown up animation, even if he is
//							dead (returning to the dead anim).
//							Also, now damage anim will loop.
//
//		03/31/97	JMI	Took out INPUT_* macro definitions since input.h now
//							defines them correctly.
//
//		04/01/97	JMI	UINPUT now stores the delta rotation in the first 10 bits
//							as 0..720 representing delta values between -360 and 360.
//
//		04/02/97	JMI	Changed all resource animation names to use the 
//							CREATERESNAMES macro that some were already using.
//
//		04/02/97	JMI	OnExplosionMsg() now beefs up his vertical velocity due to
//							the explosion, if a successful state change occurs.
//
//		04/03/97	JMI	Added additional link point for shooting from nealing pos.
//
//		04/03/97	JMI	Changed font color to RGB.  The index is gotten on Init().
//
//		04/03/97	JMI	Hard coded ms_u8FontBackIndex to RSP_BLACK_INDEX.
//
//		04/03/97	JMI	Now that PrepareWeapon() returns the pointer to the new
//							weapon or NULL.  The return value should not be compared to
//							0 for success (now that indicates failure).
//
//		04/03/97	JMI	Now, if the rise state is entered before the ducking anim
//							was finished, he jumps into the rise anim at the inverse of
//							his relative position in the duck anim so he appears to
//							rise back starting at the position he was in during the
//							duck.  It looks suprisingly good thanks to the consistency
//							of the art.
//							Also, if shoot is requested while ducking, the dude auto-
//							magically switches to rising state first.  Firing cannot
//							override rising, so it is not entered until the rise is
//							complete.
//
//		04/07/97	JMI	Added jumps, landings, and fall components.
//
//		04/08/97	JMI	He was not responding to State_Delete.  Now he deletes self
//							at the end of Update().
//
//		04/14/97 BRH	Added CSmash::Item to the type of things that bullets could
//							hit, so he can shoot barrels.
//
//		04/14/97	JMI	Now DrawStatus() will only display <= 100% health.
//
//		04/18/97	JMI	Changed EXPLOSION_VERTICAL_VEL_MULTIPLIER from 1.5 to 1.0
//							which should basically make is vertical explosion velocity
//							the same as all the other characters.
//
//		04/21/97	JMI	Changed EXPLOSION_VERTICAL_VEL_MULTIPLIER from 1.0 to 1.3.
//
//		04/21/97	JMI	Incorporated Mike's new crawler thinger for smooth rubbin'
//							and slidin' up against schtuff.
//							The nubs can be heavily tuned.
//
//		04/21/97	JMI	Added override of WhileBlownUp().
//
//		04/22/97 BRH	In the burning state, he used to check to see when the
//							fire object was dead.  Now since the fire changes to smoke,
//							he was running around on smoke.  So I added a call to
//							IsBurning so that he knows when he can resume normal
//							control.
//
//		04/22/97	JMI	Moved the setup of the font stuff to startup when the
//							palette for the hood should already have been set.  This
//							way we get the correct colors for the DrawStatus().
//
//		04/22/97	JMI	Fixed my use of shadow even though currently that code does
//							not get used.
//							Also, changed STATUS_FONT_SIZE from 24 to 26 which is a 
//							currently cached scale for the g_fontBig.
//							Also, disabled jump for current demo for GT.
//
//		04/22/97	JMI	Dude can no longer get up after dying.
//
//		04/23/97	JMI	Now just uses a test version of IsPathClear().
//
//		04/23/97	JMI	Now our bullets can hit Characters, Miscs, Mines,  
//							Barrels, and Fire.
//
//		04/23/97	JMI	Disabled IsPathClear() test.
//
//		04/24/97	JMI	Added shotgun weapon components.
//
//		04/25/97	JMI	ShootWeapon() now decrements m_sNumShells.
//							Also, previously when the guy was reacting to being shot,
//							he couldn't die.
//
//		04/25/97	JMI	Added a state for executing dudes.
//
//		04/25/97	JMI	Can no longer move forward or back or turn while executing.
//
//		04/25/97	JMI	Added flame thrower weapon support.  Required fuel ammo
//							info and special case on release of flamage.
//
//		04/25/97	JMI	Removed input for flame thrower for temp even though it will
//							soon rock.
//
//		04/27/97	JMI	Changed link point for execute and re-enabled flame thrower.
//							Also, execute now checks amount of bullets.
//
//		04/28/97	JMI	Removed PrepareWeapon() override since the base class
//							version now handles bullet oriented weapons.
//							Also, moved FireBullets() to base class, CCharacter.
//
//		04/29/97	JMI	Changed CAnim3D::Get()'s over to new version that does not
//							require an array of char pointers.
//							Added m_animPickPut for picking up and putting things down.
//							Also, added mine weapon.  You cannot choose the type now,  
//							though (it's the default which is proximity).              
//
//		04/29/97	JMI	I was unnecessarily making the dude go into State_Rise
//							after the State_PutDown animation finished.  But, as it
//							turns out, the m_animPickPut already contains a rise
//							which makes things a bit easier.
//
//		04/29/97	JMI	Flame thrower would only work while not moving.  Fixed.
//
//		04/30/97	JMI	Changed Mine enum value to ProximityMine, TimedMine, 
//							RemoteMine, and BouncingBettyMine.
//
//		04/30/97	JMI	Changed m_animPickPut to use 'hand' instead of 'guntip'.
//
//		05/01/97	JMI	Now his animation speed for his run is in ratio to 
//							RUN_ANIM_VELOCITY and he enters the stand state when his
//							speed falls below MIN_RUN_VEL and there's no user 
//							acceleration.
//							Also, fixed some transition bugs involving 
//							strafing/standing/shooting to strafing/running/shooting and
//							everything in between.
//
//		05/02/97	JMI	Now plays sample g_smidExecute1 or g_smidExecute2, if
//							he actually hits someone during execute.
//
//		05/02/97	JMI	Uses new inputs where weapons are a value stored in 4 bits.
//
//		05/08/97	JMI	Now shows status change even if the weapon pressed is
//							already the current weapon.
//							Added the NoWeapon weapon.
//
//		05/09/97	JMI	Changed MAX_BULLETS_PER_SEC from 10 to 6.
//							Added flame thrower noises.
//
//		05/12/97 JRD	Added a call to SpewTriggers on dude move to alert Pylons
//
//		05/13/97	JMI	Changed INPUT_WALK (was INPUT_RUN) from functioning as a 
//							'fast' key to a 'slow' key.
//							Also, I made the slow key affect the rate the dude turns
//							by dividing the input rotation delta by SLOW_TURN_DIVISOR.
//							Also, added ability to fire the heatseeker.
//
//		05/14/97	JMI	Dude can now pick up and cash in on power ups.
//							Also, added Message() which allows you to add messages
//							to the dude's status display.
//
//		05/22/97	JMI	Added CDudeAnim3D so we could use Get() but have it not
//							load textures (they are loaded separately so the dude
//							color can be changed by the user).
//
//		05/23/97	JMI	Update() was nearly a thousand lines and that was just
//							too damned much.  So I broke it up a bit.
//							Also, changed FireCurrentWeapon() to ArmWeapon() which
//							takes as a parameter the weapon to fire.
//
//		05/23/97	JMI	Changed all the CAnim3D::Get()s to take the new 
//							pszEventName parameter.
//
//		05/23/97	JMI	Changed names of loaded textures to "main_color%02d" (was
//							"main_%d".
//
//		05/23/97	JMI	Now loads the "main_missile" event data and uses it to
//							show and release the missile.
//							Consequently, most of the time the missile seems to be in 
//							the wrong spot and kills us almost immediately... .
//
//		05/25/97	JMI	Restored main_missile anim to NOT using events b/c those
//							events happend so much earlier than the old FUDGE method,
//							the dude kills himself with the missiles.
//
//		05/26/97	JMI	Now will only repeat shot state every 
//							MIN_CANNOT_BE_SHOT_DURATION ms (will groan and get bloody
//							though).
//							Also, will only yell due to being shot or blown up every
//							MIN_BETWEEN_YELLS ms.
//
//		05/26/97	JMI	Added m_sOrigHitPoints so dude can show percentage based
//							on initial hitpoints.
//
//		05/27/97	JMI	Changed 'beers' to 'health'.
//
//		05/29/97	JMI	Made it so the dude can go from blownup state to blownup 
//							state repeatedly (not sure why I limited it before).
//							Attempted to make it so the dude does not cry out in
//							pain while blowing up.
//							Added a Revive() function which is the best way to revive
//							a dude that has died.
//							Added a timer for changing weapons that use the same input.
//
//		05/29/97	JMI	Now the INPUT_JUMP doubles as a revive (when he is dead).
//
//		05/29/97	JMI	Fixed comparison on mine delay timer that caused mines to
//							be unselectable.
//
//		05/30/97	JMI	Added SprayCannon weapon.
//
//		05/30/97	JMI	Changed SHELLS_PER_SPRAY_CANNON_FIRE to 4 (was 2) and 
//							disabled the flame thrower.
//
//		05/30/97	JMI	Fixed Spray Cannon so it won't fire if there's less than
//							SHELLS_PER_SPRAY_CANNON_FIRE shells.
//
//		06/02/97	JMI	Added climb state.
//
//		06/02/97	JMI	Removed climb state.
//
//		06/03/97	JMI	Changed EXPLOSION_VERTICAL_VEL_MULTIPLIER from 1.3 to 1.0.
//
//		06/03/97	JMI	Got rid of 2d brain splat anim and replaced with CChunk
//							pieces.
//							Also, changed weapon used to shotgun.
//
//		06/03/97	JMI	Changed event name from 'mainmissile' to 'mainevent' and
//							changed the suicide to use the event for when to fire the
//							gun (and splat is brains).  Also, now uses the 'head' 
//							transform for the location of the source of the brain 
//							splat.
//
//		06/05/97	JMI	Removed m_sHitPoints and m_sNum*.  Now uses CThing3d's 
//							m_stockpile instead.
//							Changed EditModify() to bring up the user settings for
//							m_stockpile when the 'StockPile...' button is pressed.
//
//		06/05/97	JMI	Now rotates at a reduced rate when standing still.
//							Also, fixed bug with divisors when input for rotation
//							is nullified.
//
//		06/06/97	JMI	Now utilizes CPowerUps' new CStockPile and GetDescription().
//							Also, reduced status font size so we can see moreat 640x480
//							(from 26 to 15).
//
//		06/07/97	JMI	Now Revive() will try to use a CWarp.
//
//		06/08/97 BRH	Added targeting sprite to the dude and using it
//							when calling IlluminateTarget() to display which enemy
//							or item is being targeted.  Temporarily disabled the
//							targeting aid.  Still need to add a key to toggle
//							m_bTargetingHelpEnabled and the position of the target
//							sprite needs to be adjusted.
//
//		06/09/97 BRH	Adjusted the target position on the child and enabled
//							the targeting help.
//
//		06/09/97	JMI	Now Revive() empties the message queue b/c we don't want
//							him responding to things that happened in his old location
//							(i.e., his location before he warped in).
//							Removed m_inputLast and m_lLastWeaponChangeTime which are
//							no longer necessary now that we can use the auto-repeat
//							feature with the new event driven rspGetKeyStatusArray()
//							used by input.cpp.
//							Added idle animation components.
//
//		06/09/97 BRH	Hid the target when the option is disabled.  Set the
//							default state to disabled.
//
//		06/09/97	JMI	Disabled the SprayCannon.
//
//		06/09/97	JMI	Changed INPUT_WALK to INPUT_RUN.
//
//		06/10/97 BRH	Sends WeaponSelect and WeaponFire messages to CDemon
//							so that comments can be made about these actions.
//
//		06/09/97	JMI	Added weapon selection feedback specific to whether weapons
//							are loaded or empty.
//
//		06/10/97	JMI	Changed SLOW_TURN_DIVISOR from 1.5 to 1.25.
//
//		06/10/97	JMI	Got rid of the SLOW_TURN_DIVISOR and added a second rate
//							inside of input to vary the rate.
//
//		06/10/97	JMI	Now notifies demon of suicides.
//
//		06/11/97	JMI	Commented out code that starts idle animation.
//							Also, made suicide zero your hitpoints.
//
//		06/11/97	JMI	Revive() now drops a powerup before warping dude.
//
//		06/12/97	JMI	Dude now checks to make sure he has the proper weapon to
//							fire the requested ammo.
//							Added handy functions, SetWeapon(), NextWeapon(), and
//							PrevWeapon().
//
//		06/12/97	JMI	Added powerup responses to inputs 10 to 14.
//
//		06/12/97	JMI	Adjusted some of the cheat powerups that were silly, silly
//							amounts.
//
//		06/12/97	JMI	Made launch and throw anims use the events to show and
//							release weapons.
//							Now registers suicide death with Score module (but will
//							probably have to add m_pRealm as a parm once that accepts
//							it).
//							Now napalm launches from the missile launcher.
//							Also, re-enabled idle animation.
//
//		06/12/97	JMI	Added ms_apszAmmoNames to differentiate between weapons
//							and ammo strings.
//
//		06/12/97	JMI	Now DrawStatus() only draws once per StatusChange() call.
//
//		06/13/97	JMI	Now State_ThrowRelease, State_LaunchRelease, and 
//							State_PutDown all use WhileHoldingWeapon() to unhide and
//							release the weapon at the appropriate event driven times.
//
//		06/13/97	JMI	Now correctly uses columns and word wrap with its m_print.
//
//		06/14/97	JMI	Now picks up powerups when you run over them.
//
//		06/15/97	JMI	Now minimizes total stockpile after adding in powerups.
//							Now initializes stockpiles since CStockPile no longer has
//							a constructor (I wanted it to be aggregatable).
//
//		06/16/97	JMI	Now plays a sound when his torso hits the ground when dying.
//
//		06/16/97	JMI	The target now alpha blits.
//
//		06/16/97	JMI	Added MessageChange() and m_lMsgUpdateDoneTime.
//
//		06/16/97	JMI	Plays audible feedback via powerup when powerups picked up.
//							Up, up, up, up.
//
//		06/16/97	JMI	Added parms to make ShootWeapon() comply with virtual
//							base class version.
//							Also, fixed bug where, when out of bullets, we went ahead
//							and fired anyways.
//
//		06/16/97	JMI	Now applies resets shot timer when a shot kills us.
//
//					MJR	Changed strafe speed.
//
//					JMI	Now registers death when he enters the State_Dead.
//
//		06/17/97	JMI	Changed the hit the ground noise event val to 2 (was 1).
//
//					JMI	Added PlayStep() which is commented out b/c of sound probs.
//							Added m_u8LastEvent and OnExecute().
//
//					JMI	Added NULL in call to PlaySample() corresponding to new
//							param.
//
//		06/17/97	JMI	Converted all occurrences of rand() to GetRand() and
//							srand() to SeedRand().
//
//		06/24/97	JMI	Now notifies the demon of suicide in SetState() (used to
//							be in OnSuicideMsg() which is not the only way a suicide
//							can be initiated).
//							Also, now SetWeapon() only sends a message to the demon
//							if the new weapon has ammo.
//
//		06/25/97	JMI	Added FindExecutee() which is called to find an executee
//							before entering the execute state.  If none is found, we
//							do not enter the state.  If we do enter the state, while
//							he is leading up to the shooting portion of the animation,
//							he turns toward the executee.  When he fires at the 
//							executee, it does not use FireBullets() but instead merely
//							creates a muzzle flare and sound for feedback and sends a
//							message to the executee telling him he's been shot.
//
//		06/25/97	JMI	Now calls PrepareShadow() in Init() which loads and sets up
//							a shadow sprite.
//							Also, now prints 'you are dead' when you are out of 
//							hitpoints.
//
//		06/25/97	JMI	Replaced m_lStatusUpdateDoneTime with 
//							m_lNextStatusUpdateTime and removed m_bUpdateStatus.
//							Also, changed SHELLS_PER_SPRAY_CANNON_FIRE from 4 to 1.
//							ms_apszAmmoNames[] now contain the sprintf format 
//							specifiers to provide greater to control to each type over
//							whether to display a number and/or where to display it.
//							The machine gun no longer uses up bullets.  So, once you
//							have bullets, you never run out.
//							Now hitting the rocket key while the rocket is selected
//							switches to heatseekers and vice-versa.
//							Now switches to the machine gun when you run out of
//							a particular weapon.
//
//		06/26/97	JMI	The 'You are dead' message now flashes.
//							Target now goes away when you are dead.
//							Also, changed 'You are dead' to something else to get
//							Steve inspired enough to think of a cool one.
//
//		06/26/97	JMI	Now handles throw weapons the same way launch weapons are
//							handled when leaving the animation early.  That is, they
//							no longer drop, but, instead, just get deleted via a 
//							message.
//
//		06/26/97	JMI	Re-activated foot step sound.
//
//		06/27/97	JMI	If the dude cannot move (i.e., MakeValidPos() fails), he
//							resets his velocity, acceleration, and drag to zero.
//							The only possible problem that I know of with this is that,
//							if there's an external force that's causing him to push up
//							against terrain, even if he is fighting the force by run-
//							ning against it, he won't be able to accelerate until the
//							force dissipates.  Currently, though, all external forces 
//							have drag.
//
//		06/28/97	JMI	Now scales the Dude's Z through the realm before calling
//							SpewTriggers().
//
//		06/29/97	JMI	Now uses the new name for the function that scales the Z
//							through the realm (MapZ3DtoY2D (was ScaleZ) ).
//
//		06/30/97 BRH	Cached sound effects during the load so they will be
//							ready for their first use.
//
//		07/01/97	JMI	Changed name of animation file for strafe from 'sidestep'
//							to 'strafe' and strafe and shoot from 'sideshoot' to
//							'strafe'.
//							Added new rigid bodies to CDudeAnim3D and moved its 
//							function defs into dude.cpp.
//							Also, added m_idFlagItem.
//
//		07/01/97	JMI	Strafing was not playing anim backwards for left strafe
//							when shooting.
//							Also, a bug caused the strafe to not go backwards for
//							the regular (non-shooting) strafe if m_dRot was less than
//							90.
//
//		07/02/97 BRH	Changed the flamethrower to use CFirestreamID rather than
//							CFireballID.
//
//		07/03/97	JMI	Now uses SetGuiToNotify() to make a button able to end a
//							DoGui() session.
//
//		07/04/97 BRH	Made the Dude drop the flag if he gets blownup, burned or
//							he dies, also it tries to use the current animation's
//							rigid body transform for the release point but if it doesn't
//							have one it will just use the guy's position.
//
//		07/07/97	JMI	DrawStatus() now returns true if it drew to the provide
//							image.
//
//		07/08/97 BRH	Changed some of the animation names that were too long
//							for the delicate MacOS.	  Also changed hand transform
//							and backpack animation names.
//
//		07/09/97	JMI	Changed MAX_STEPUP_THRESHOLD to use CThing3d's macro enum
//							MaxStepUpThreshold.
//
//		07/09/97	JMI	Now uses m_pRealm->Make2dResPath() to get the fullpath
//							for 2D image components.
//
//		07/10/97	JMI	Now uses "lfhand" for main rigid body for pick put anim.
//
//		07/10/97	JMI	Changed names of loaded textures from "main_color%02d" to
//							"main_%d".  Also, now it's zero based instead of 1 based.
//
//		07/12/97	JMI	Now m_pRealm->m_bMultiplayer must be true in order to
//							Revive() a CDude.
//
//		07/13/97	JMI	Set ms_dAccUser and ms_dAccDrag to 1000.0 each so the Dude
//							will start and stop nearly immediately.
//							Attempt to adjust the RUN_ANIM_VELOCITY but got nowhere.
//							Somebody who's better at judging distances should do that.
//
//		07/14/97	JMI	Now applies damange to dude in OnBurnMsg() after doing all
//							other processing.  The problem was that, when he was being
//							damaged before other processing, he would die if the damage
//							was great enough and then the other processing would not
//							be done.
//							Also, only destroys his fire when exiting the burning state
//							to go to other than the die state.
//
//		07/14/97	JMI	Was subtracting from fuel for flamethrower in both 
//							ArmWeapon() and ShootWeapon().
//
//		07/15/97	JMI	Moved CDude() and ~CDude() from dude.h to dude.cpp.
//							Also, ShootWeapon() no longer checks for out of fuel con-
//							dition as this is done in ArmWeapon().
//
//		07/15/97	JMI	Now dude will leave portions of powerups that he doesn't
//							need.
//
//		07/15/97	JMI	Added TakePowerup().
//
//		07/16/97	JMI	Made DropPowerUp() return the newly created powerup.
//							Added CreateCheat() which trims the amount the dude cannot
//							use from the stockpile and passes that to DropPowerUp().
//
//		07/16/97	JMI	Changed the order of the weapons.
//
//		07/17/97	JMI	Changed m_psnd to m_siLastPlayInstance.
//							Now uses new SampleMaster interface for volume and play
//							instance reference.
//
//		07/18/97	JMI	Now uses StopLoopingSample() to stop looping the flame-
//							thrower sound (instead of GetInstanceChannel() ).
//
//		07/18/97	JMI	Got rid of bogus immitation PlaySample functions.
//							Now there is one PlaySample() function.  Also, you now
//							MUST specify a category and you don't have to specify a
//							SoundInstance ptr to specify a volume.
//
//		07/18/97	JMI	Now updates font colors based on hood background's palette
//							rather than the RSPiX system palette.
//
//		07/19/97	JMI	In previous fix, forgot to change the increment value for
//							the palette red, green, & blue arrays.
//							Also, now we pass our GUI to the stockpile editor.
//							Also, now Revive() allows revival in single player mode if
//							the dude has health greater than 0.
//							Also, the 'give health' cheat can now be activated while
//							dead.
//
//		07/20/97 BRH	Added a few more sample caches for weapon selection and
//							powerup pickups.
//
//		07/20/97	JMI	Now, if there's nothing the dude needs out of a powerup,
//							he plays a feedback sample and throws the powerup anyways.
//
//		07/20/97	JMI	Spray cannon is now governed by MAX_SPRAYS_PER_SEC (10).
//
//		07/21/97	JMI	Now TakePowerUp() returns the powerup if it persisted.
//							CreateCheat() now immediately hands the created powerup
//							to TakePowerUp() so there's no chance we'll miss it.
//							Also, TakePowerUp() will use the max stockpile for 
//							backpacks not only if the dude has a backpack but, also,
//							if the dude will have a backpack (i.e., if there's one in
//							the powerup).
//							Also, added ms_apszWeaponStatusFormats[].
//							Now, if you have ammo but no weapon, it reports that you
//							have ammo but no weapon.  Ingenious.
//
//		07/21/97	JMI	Added OnWeaponDestroyed() override of CCharacter version.
//							Also, Update() now calls CCharacter version.
//							Also, ShootWeapon() now decrements ammo supplies (not
//							ArmWeapon() ).
//							Also, added m_weaponShooting.
//							Removed SHELLS_PER_SPRAY_CANNON_FIRE.
//
//		07/22/97	JMI	Now can never run out of machine gun bullets again.
//
//		07/23/97	JMI	Changed MIN_CANNOT_BE_SHOT_DURATION to 3000 (was 1500) ms.
//
//		07/23/97	JMI	Added DefHasNapalmLauncher enum and added strings to in-
//							corporate new napalm launcher (used to be launched by the
//							missile launcher).
//
//		07/25/97	JMI	Removed PickUp input.
//							Added option to Revive() to allow him to revive in-place
//							(rather than warping in).
//							Added m_bDead and IsDead() which are true when the dude
//							is dead.
//							Now uses m_bDead in most cases that m_state == State_Dead
//							and m_smash.m_bits & CSmash::Dead were used.
//
//		07/25/97	JMI	Added StartAnim() which starts a CAnimThing.
//							Added visual feedback (bullet CChunks that fly off) when
//							wearing the kevlar layers.
//
//		07/27/97	JMI	Now, like the chunks, ricochets appear on the appropriate
//							part of the dude's torso (based on the angle of the bullet).
//							It might make sense to eventually put the height of the shot
//							in the shot message so we can place the splat at the 
//							appropriate height giving more feedback as to what's/who's
//							shooting us.  It'd also be neat to adjust the damage based on
//							the vertical position.
//							Also, now StartAnim() does not call CRealm::Make2dResPath() 
//							on passed in paths since CAnimThing does this 
//							automatically.
//
//		07/28/97	JMI	Now checks m_bDead (instead of m_stockpile.m_sHitPoints) to
//							determine if dude is dead in DrawStatus().
//							Also, only bobbles powerups when alive.
//
//		07/28/97	JMI	Fixed resurection w/o powerup that would occur sometimes.
//							There are only two ways to leave the Die state, BlowUp and
//							Dead.  Sometimes he would get blown up while dying skipping
//							ever getting his m_bDead flag set.  Now the Die state sets
//							this flag as well since we know we want him to die if he 
//							enters the die state.
//							Since, once his dead flag is set, he can accept the 
//							resurection cheat, there's some misleading feedback in the
//							Die state if that cheat is entered.
//							To avoid this problem we could not set the flag and instead
//							set the persistent state to State_Dead so that when he's
//							done blowing up he'll go to the dead state, but setting the
//							flag seems much more likely to make sure he dies.
//
//		07/29/97	JMI	Added GetCurrentWeapon().
//
//		07/30/97	JMI	Added DeathWad components.
//							Now displays a random message from g_apszDeathMessages[]
//							when dead.
//
//		07/30/97	JMI	Now the death wad cheat enables the death wad option in the
//							editor.
//
//		08/01/97 BRH	CreateCheat() now sends a message to the CDemon to let
//							it know that a cheat code was entered so the Demon can
//							mock the player.  Also added it in the revive case.
//							Also took out loading of .hot files since they aren't used
//							and will soon be deleted from the directories.
//							Changed fire damage to be based on difficulty setting.
//
//		08/04/97	JMI	Now Damage() function will set m_bDead even if 
//							SetState(State_Die) fails.  This way, he'll die when he
//							comes out of the explosion state (he cannot go to die from
//							explode (unless explode is done) b/c that would look really
//							wrong).
//
//		08/04/97	JMI	Added array of weapon anims, m_aanimWeapons[], and 
//							a sprite for showing the current weapon, m_spriteWeapon.
//							Now changes weapons visually based on current weapon type.
//
//		08/04/97	JMI	Now uses Jeff's rspDegDelta() to better determine which
//							direction to turn when executing.
//
//		08/05/97	JMI	Changed reference of m_pRealm->m_bMultiplayer to 
//							m_pRealm->m_flags.bMultiplayer.
//							Changed "rthand" to "gun" for right hand rigid body trans-
//							form.
//
//		08/05/97	JMI	Added an ID to Message().  The idea being that, when the
//							same message ID is specified w/i a certain amount of time,
//							the more recent message is ignored.
//
//		08/06/97	JMI	Now OnExecute(), TrackExecutee(), and FindExecutee() can
//							work with any CThing that defines GetX/Y/Z() (not just
//							CThing3d's).
//
//		08/07/97	JMI	Added DoubleBarrel components.
//
//		08/07/97	JMI	Forgot a break in switch in GetWeaponInfo().
//
//		08/07/97	JMI	Added m_animBackpack and m_spriteBackpack for his backpack 
//							when he has it.
//
//		08/07/97	JMI	Removed commented out hot channel stuff from CDudeAnim3D.
//							Added release of backpack anim in FreeResources().
//
//		08/08/97	JMI	Now that CCharacter handles flamer noise, this class now
//							longer has to (and doesn't).
//
//		08/08/97	JMI	Added TossPowerUp().
//							Also, changed TakePowerup() to TakePowerUp().
//							Now, when he enters the dead state in multiplayer mode,
//							he drops his schtuff.
//
//		08/08/97	JMI	Majorly condensed code segments for detaching flag by 
//							calling DetachChild() which does most of what we were doing
//							here.  Also, removed some conditions that were checking to
//							make sure m_panimCur and m_panimCur->m_ptransRigid were
//							valid but they were not very useful since if m_panimCur were
//							NULL we'd be fucked AND m_ptransRigid was not the
//							rigid body transforms being used for the flag anyways (we're
//							using m_ptransLeft).
//
//		08/09/97	JMI	Draws dead frame into background just before reviving.
//
//		08/10/97	JMI	Added StrafeLeft and StrafeRight inputs.
//							Also, changed Jump to Revive and INPUT_JUMP to INPUT_REVIVE.
//
//		08/10/97	JMI	Changed doubles dDistX and Z to shorts sDistX and Z.  The
//							compiler seemed to lock up when inlining was enabled and we
//							passed these two doubles to rspATan.  With a different 
//							function it worked fine and with one of the vars as a short
//							it worked fine.  /shrug  Looking for a VC patch.
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
//		08/12/97 BRH	Added CSmash::Ducking bits to the smash when he is
//							ducking down.  When he is ducking, he won't get hit
//							by the missiles.
//
//		08/12/97	JMI	Removed unused anims: m_animJump, JumpForward, Land, 
//							LandForward, Fall.
//
//		08/13/97	JMI	Temporarily shows particle effects regardless of user
//							setting in multiplayer mode.
//
//		08/14/97	JMI	Switched references to g_GameSettings.m_sDifficulty to
//							m_pRealm->m_flags.sDifficulty.
//
//		08/14/97 BRH	Added static collision bits to be passed as the default
//							bits for weapons that the dude shoots.  The default
//							arguments in the ShootWeapon are unreliable and may
//							get set to the bits of the base class default args.
//							Also changed call of WhileHoldingWeapon to include
//							these default collision bits.
//
//		08/15/97 BRH	Set the XrayAll flag when the Dude is dead, so if he
//							dies behind opaque, you will be able to see that he is 
//							dead.
//
//		08/17/97	JMI	Changed to only find writhers as executees and will 
//							definitely hit the executee.
//
//		08/17/97	JMI	Removed all occurrences of the SetXRayAll() call as this
//							would not be a good place to do that since this dude is
//							not necessarily the local dude.  Soon we will change Play
//							to do it since it knows that.
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
//		08/18/97	JMI	Now applies no randomization to CChunks and instead allows
//							CChunk::Setup() to apply its own randomization based on
//							sway values passed to it.  Also, CChunk::Construct() will
//							now fail if particles are disabled so we don't need to
//							check for that anymore.
//
//		08/18/97	JMI	Moved StartAnim() from CDude to CThing3d so more things
//							could use it.
//
//		08/18/97	JMI	Added m_bInvincible which gets set via cheat.
//							Added many cheats and revamped originals.  There is one
//							left.
//
//		08/18/97	JMI	Added cheat code for sales people.
//
//		08/18/97	JMI	Changed Revive() to call DeadRender3D() (which used to be
//							known/called as just another Render() overload).
//
//		08/19/97	JMI	No longer shoots CSmash::Misc items.
//
//		08/24/97 BRH	Forwards the burn message to the child flag so that it
//							can react.  Before he was dropping the flag when blown up
//							or burned, but only telling the flag when it was blown up,
//							so if you got burned while holding the flag, the flag was
//							in the wrong state and never looked for anyone to pick it
//							up again.  So the flag would be stuck in place and you could
//							not grab it again.
//
//		08/24/97	JMI	Now instead of setting his brightness to zero in the dead
//							state, he clears it in Revive().  This way he stays charred
//							even when laying there dead or being blown up but cannot 
//							come back to life charred.
//
//		08/24/97	JMI	Now INPUT_WEAPON_0 is no weapon and INPUT_WEAPON_10 is 
//							mines.
//
//		08/28/97 BRH	Moved the caching of samples to Init() now that the
//							Load function is not being called now that the warps
//							create a dude.  Also added OnPutMeDowmMsg handler function
//							that the flag sends when it wants to be set down (on the
//							flagbase).
//
//		08/30/97	JMI	Removed m_idFlagItem (now uses the sprite list and, for
//							flag child sprites, updates them all).
//
//		09/01/97	JMI	Now ArmWeapon() sets m_weaponShooting to NoWeapon if it
//							fails to arm a weapon (either b/c of no ammo or the shoot
//							state is not attainable).
//
//		09/01/97	JMI	Previous changed was hosing the shotgun so I made it a 
//							little less strict.
//
//		09/02/97	JMI	Now targets CSmash::Sentry in ms_u32CollideBitsInclude.
//
//		09/07/97 MJR	Fixed bug in DropAllFlags() that didn't read the message
//							properly.
//
//		09/08/97	JMI	Added Kevlar type for pieces of kevlar vest that 
//							splatter off of dudes with vest.
//
//		11/21/97	JMI	Now varies whether CSmash::Good is a desirable target to
//							our weapons based on the realm's multiplayer cooperative
//							flag.
//							Removed version of ShootWeapon() that took 3 default 
//							parameters and replaced it with a version that takes no
//							parameters so we could control the defaults from within the
//							CPP.
//							Also, now sets CWeapons' detection bits explicitly via
//							SetDetectionBits() to NOT seek other players in Cooperative
//							multiplayer mode.
//
//		12/08/97	JMI	Added an option for DropPowerUp() to only drop the 
//							currently selected weapon.
//							Also, now sets m_sHitPoints to zero before calling 
//							SetState(State_Dead) which was previously generating a
//							health powerup if m_sHitPoints was greater than zero which
//							was the case for suicide.
//
//		12/09/97	JMI	Now drops all weapons when reviving in single player mode.
//							In general, drops all weapons when reviving at location of
//							death and drops only the current weapon when reviving via
//							a warp (the warp gives you additional stuff).
//
////////////////////////////////////////////////////////////////////////////////
#define DUDE_CPP

#include "RSPiX.h"
#include <math.h>

#include "dude.h"
#include "grenade.h"
#include "game.h"
#include "SampleMaster.h"
#include "input.h"
#include "AnimThing.h"
#include "reality.h"
#include "fire.h"
#include "TriggerRegions.h"
#include "PowerUp.h"
#include "chunk.h"
#include "warp.h"
#include "score.h"
#include "CompileOptions.h"	// For sales cheat.
#include "play.h"

//#ifdef MOBILE
bool demoCompat = false; //Set in Play.cpp
//#endif

#if defined(ALLOW_TWINSTICK)
void GetDudeVelocity(double* d_Velocity, double* d_Angle);
extern bool GetDudeFireAngle(double* d_Angle);
#endif

////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////

// Determines the number of elements in the passed array at compile time.
#define NUM_ELEMENTS(a)		(sizeof(a) / sizeof(a[0]) )

// Tunable bullet parameters.
#define MAX_BULLETS_PER_SEC		6

#define MS_BETWEEN_BULLETS			(1000 / MAX_BULLETS_PER_SEC)

#define MAX_SPRAYS_PER_SEC			10

#define MS_BETWEEN_SPRAYS			(1000 / MAX_SPRAYS_PER_SEC)

#define MAX_FLAMES_PER_SEC			1000

#define MS_BETWEEN_FLAMETHROWS	(1000 / MAX_FLAMES_PER_SEC)

#define MODIFY_GUI_FILE				"res/editor/EditDude.gui"

// Random amount the fire angle can adjust.
#define FIRE_ANGLE_Y_SWAY			15
#define FIRE_ANGLE_Z_SWAY			10

// Maximum the guy can tweak your angle while you're on fire.
#define ON_FIRE_ROT_TWEAKAGE	22
#define ON_FIRE_VEL_TWEAKAGE	((short)ms_dMaxVelForeFast)

// Amount per damage pt that translate to velocity.
#define VEL_PER_DAMAGE			0.5

// Gets a random between -range / 2 and range / 2.
#define RAND_SWAY(sway)	((GetRand() % sway) - sway / 2)

// Velocity for strafing.
#define STRAFE_VEL				60.0

#define EXPLOSION_VERTICAL_VEL_MULTIPLIER	1.0

// Amount of time to suffer from shot.
#define MIN_SHOT_DURATION				500

// Amount of time before we can suffer another shot.
#define MIN_CANNOT_BE_SHOT_DURATION	3000

// Minimum duration between yells (for blown up and shot).
#define MIN_BETWEEN_YELLS		750 // Current yell ('groan_male1.wav') is 640 ms.

// IDs for GUIs.
#define GUI_ID_STOCKPILE		3

// The color range that is the same on all hoods.
#define CONSTANT_COLOR_START_INDEX	95
#define NUM_CONSTANT_COLOR_INDICES	(256 - CONSTANT_COLOR_START_INDEX)

// Size of font used for status updates.
#define STATUS_FONT_SIZE				15
// 0 for any of these colors is transparent.
#define STATUS_FONT_FORE_RED			128
#define STATUS_FONT_FORE_GREEN		0
#define STATUS_FONT_FORE_BLUE			0
#define STATUS_FONT_BACK_RED			0
#define STATUS_FONT_BACK_GREEN		0
#define STATUS_FONT_BACK_BLUE			0
#define STATUS_FONT_SHADOW_RED		127
#define STATUS_FONT_SHADOW_GREEN		127
#define STATUS_FONT_SHADOW_BLUE		127

#define STATUS_PRINT_X					0
#define STATUS_PRINT_Y					0

#define STATUS_UPDATE_DURATION		5000	// In ms.
#define MESSAGE_UPDATE_DURATION		3000	// In ms.

#define SHADOW_DEPTH_X					1
#define SHADOW_DEPTH_Y					1

// Rate at which run animation was made to travel in pixels per second.
// This is used to compute a ratio to the current speed.  The resulting 
// ratio is used to tune the animation rate dynamically.
#define RUN_ANIM_VELOCITY				85.0	// Pix/Sec.

// When below this speed, the dude will not be in his running anim.
#define MIN_RUN_VEL						40.0	// Pix/Sec.

// Maximum springiness for crawler.
#define MAX_CRAWLER_PUSH_X				1.5
#define MAX_CRAWLER_PUSH_Z				1.5

// Time between cycling weapons that are chosen using the same input.
#define WEAPON_CYCLE_TIME				250

// Ladder tolerance.
#define LADDER_DIR_TOLERANCE				20	// +/- this amount in degrees.

#define BRAIN_SPLAT_NUM_CHUNKS		200
#define BRAIN_SPLAT_SWAY				30

// Reduced rate for standing still rotation.
#define STILL_TURN_DIVISOR				2.0

// Handy macro to define char* array with one 'base' name.
#define CREATERESNAMES(var, base, rigid)		\
	static char* var[]	=							\
		{													\
		"3d/main_" #base ".sop",					\
		"3d/main_" #base ".mesh",					\
		"3d/main_" #base ".tex",					\
		"3d/main_" #base ".hot",					\
		"3d/main_" #base ".bounds",				\
		"3d/main_" #base ".floor",					\
		"3d/main_" #base "_" #rigid ".trans",	\
		NULL,	/* Safety */							\
		};

#define TARGETING_FILE 	"target.img"

// IDLE_ANIM_TIMEOUT is NOT evaluated every frame.
#define IDLE_ANIM_TIMEOUT	(5000 + (GetRand() % 5000) )
#define IDLE_ANIM_LOOPS		1

// This is multiplied our current number of kevlar layers and the
// result is divided into the bullet damage to produce the damage
// the dude receives.
#define KEVLAR_PROTECTION_MULTIPLIER	3

// Alpha level used when blit'ing the target crosshairs.
#define TARGET_ALPHA_LEVEL					100

// Max distance to writher when executing.
#define EXECUTE_RADIUS						50

// Time between constant status updates in milliseconds.
#define STATUS_UPDATE_DELAY				500	// In ms.
// Max lag time between a known status change and the next update.
#define MAX_STATUS_LAG						250	// In ms.

// These may not be correct for now, but just to get them up and running:
// These are the names of the rigid body transforms that are loaded for
// every dude animation.
#define LEFT_HAND_RIGID_ANIM_NAME	"lfhand"
#define RIGHT_HAND_RIGID_ANIM_NAME	"gun"
#define BACK_RIGID_ANIM_NAME			"backpack"

// This value indicates no strafe motion and must be a totally invalid
// strafe angle value.
#define INVALID_STRAFE_ANGLE			0x7fff

#define VEST_HIT_SWAY					10

#define VEST_HIT_RES_NAME				"Ricochet.aan"

// This is the distance in realm units from the dude's center
// to the outside of his jacket.  This is used to adjust things
// like ricochets off of his kevlar vest around the outside of 
// his torso.
#define TORSO_RADIUS						5

// The minimum square distance the dude wants to be back from
// his target point when executing.
#define MIN_SQR_DISTANCE_TO_EXECUTEE	250.0

// The backpack animation basename.
#define BACKPACK_RES_NAME					"3d/backpack"

#define COLLISION_BITS_INCLUDE		(ms_u32CollideBitsInclude)
#define COLLISION_BITS_DONTCARE		(ms_u32CollideBitsDontcare | (m_pRealm->m_flags.bCoopMode ? 0 : CSmash::Good) )
#define COLLISION_BITS_EXCLUDE		(ms_u32CollideBitsExclude | (m_pRealm->m_flags.bCoopMode ? CSmash::Good : 0) )

////////////////////////////////////////////////////////////////////////////////
// Variables/data
////////////////////////////////////////////////////////////////////////////////

// These are default values -- actually values are set using the editor!
double CDude::ms_dAccUser        = 1000.0; //150.0;			// Acceleration due to user
double CDude::ms_dAccDrag        = 1000.0; //110.0;			// Acceleration due to drag

double CDude::ms_dMaxVelFore     = 85.0;			// Maximum forward velocity
double CDude::ms_dMaxVelBack     = -60.0;			// Maximum backward velocity
double CDude::ms_dMaxVelForeFast = 120.0;			// Maximum forward velocity
double CDude::ms_dMaxVelBackFast = -90.0;			// Maximum backward velocity

double CDude::ms_dDegPerSec      = 240.0;			// Degrees of rotation per second

double CDude::ms_dVertVelJump		= 100.0;			// Velocity of jump.

U32	CDude::ms_u32CollideBitsInclude = CSmash::Character | CSmash::Barrel | CSmash::SpecialBarrel | CSmash::Sentry;
U32	CDude::ms_u32CollideBitsDontcare = CSmash::Bad;
U32	CDude::ms_u32CollideBitsExclude = 0;

// Let this auto-init to 0
short CDude::ms_sFileCount;

// Weapon details database.
CDude::WeaponDetails	CDude::ms_awdWeapons[NumWeaponTypes]	=
	{
	// NoWeapon.
		{
		"No Weapon",										// Weapon name.
		"bare hands",										// Ammo name.
		"   No Weapon",									// Status format.
		NULL,													// Weapon resource name.
		1,														// Min ammo required.
		},														
																
	// SemiAutomatic.										
		{														
		"Machine Gun",										// Weapon name.
		"Bullets",											// Ammo name.
		"   Machine Gun",									// Status format.
		"3d/machinegun",									// Weapon resource name.
		1,														// Min ammo required.
		},														
																
	// ShotGun.												
		{														
		"Shotgun",											// Weapon name.
		"Shells",											// Ammo name.
		"   Shotgun -- %d Shells",						// Status format.
		"3d/shotgun",										// Weapon resource name.
		1,														// Min ammo required.
		},														
																
	// SprayCannon.										
		{														
		"Spray Cannon",									// Weapon name.
		"Shells",											// Ammo name.
		"   Spray Cannon -- %d Shells",				// Status format.
		"3d/spraygun",										// Weapon resource name.
		1,														// Min ammo required.
		},														
																
	// Grenade.												
		{														
		"Grenades",											// Weapon name.
		"Grenades",											// Ammo name.
		"   %d Grenades",									// Status format.
		NULL,													// Weapon resource name.
		1,														// Min ammo required.
		},														
																
	// Rocket.												
		{														
		"Missile Launcher",								// Weapon name.
		"Missiles",											// Ammo name.
		"   Missile Launcher -- %d Missiles",		// Status format.
		"3d/launcher",										// Weapon resource name.
		1,														// Min ammo required.
		},

	// Heatseeker.       
		{
		"Missile Launcher",								// Weapon name.
		"Heatseekers",										// Ammo name.
		"   Missile Launcher -- %d Heatseekers",	// Status format.
		"3d/launcher",										// Weapon resource name.
		1,														// Min ammo required.
		},

	// FireBomb.         
		{
		"Cocktails",										// Weapon name.
		"Cocktails",										// Ammo name.
		"   %d Cocktails",								// Status format.
		NULL,													// Weapon resource name.
		1,														// Min ammo required.
		},

	// Napalm.           
		{
		"Napalm Launcher",								// Weapon name.
		"Napalm Canisters",								// Ammo name.
		"   Napalm Launcher -- %d Canisters",		// Status format.
		"3d/napalmer",										// Weapon resource name.
		1,														// Min ammo required.
		},

	// Fuel.             
		{
		"Flame Thrower",									// Weapon name.
		"Fuel Canisters",									// Ammo name.
		"   Flamer -- %d Fuel Canisters",			// Status format.
		"3d/flmthrower",									// Weapon resource name.
		1,														// Min ammo required.
		},

	// ProximityMine.    
		{
		"Proximity Mines",								// Weapon name.
		"Proximity Mines",								// Ammo name.
		"   %d Proximity Mines",						// Status format.
		NULL,													// Weapon resource name.
		1,														// Min ammo required.
		},

	// TimedMine.        
		{
		"Timed Mines",										// Weapon name.
		"Timed Mines",										// Ammo name.
		"   %d Timed Mines",								// Status format.
		NULL,													// Weapon resource name.
		1,														// Min ammo required.
		},

	// RemoteMine.       
		{
		"Remote Mines",									// Weapon name.
		"Remote Mines",									// Ammo name.
		"   %d Remote Mines",							// Status format.
		NULL,													// Weapon resource name.
		1,														// Min ammo required.
		},

	// BouncingBettyMine.
		{
		"Bouncing Bettys",								// Weapon name.
		"Bouncing Bettys",								// Ammo name.
		"   %d Bouncing Bettys",						// Status format.
		NULL,													// Weapon resource name.
		1,														// Min ammo required.
		},

	// DeathWad.         
		{
		"Wad Launcher",									// Weapon name.
		"Wads",												// Ammo name.
		"   Wad -- %d missiles, %d napalm canisters, %d fuel canisters, %d grenades",	// Status format.
		"3d/napalmer",										// Weapon resource name.
		1,														// Min ammo required.
		},

	// DoubleBarrel.
		{														
		"Double Barrel",									// Weapon name.
		"Shells",											// Ammo name.
		"   Double Barrel -- %d Shells",				// Status format.
		"3d/shotgun",										// Weapon resource name.
		2,														// Min ammo required.
		},														
	};

// Dude's default stockpile.
CStockPile	CDude::ms_stockpileDefault	= 
	{
	DefHitPoints,				// m_sHitPoints
					                     
	DefNumGrenades,			// m_sNumGrenades
	DefNumFireBombs,			// m_sNumFireBombs
	DefNumMissiles,			// m_sNumMissiles
	DefNumNapalms,				// m_sNumNapalms
	DefNumBullets,				// m_sNumBullets
	DefNumShells,				// m_sNumShells
	DefNumFuel,					// m_sNumFuel
	DefNumMines,				// m_sNumMines
	DefNumHeatseekers,		// m_sNumHeatseekers
					                     
	DefHasMachineGun,			// m_sMachineGun
	DefHasLauncher,			// m_sMissileLauncher
	DefHasShotgun,				// m_sShotGun
	DefHasSprayCannon,		// m_sSprayCannon
	DefHasFlamer,				// m_sFlameThrower
	DefHasNapalmLauncher,	// m_sNapalmLauncher.
	DefHasDeathWadLauncher,	// m_sDeathWadLauncher.
	DefHasDoubleBarrel,		// m_sDoubleBarrel.
					                     
	DefKevlarLayers,			// m_sKevlarLayers
					                     
	DefHasBackpack,			// m_sBackpack
	};

// Nubs (a CCrawler tool).  These are the realtive points on the X/Z plane that
// define the hard and soft 'springs' that deflect the dude from terrain 
// obstacles.
static CCrawler::Nub ms_anubs[] =
	{
		// Hard nubs
		{	  8,   0, 1, 180, 1.0 },
		{	  6,  -6, 1, 225, 1.0 },
		{	  0,  -8, 1, 270, 1.0 },
		{	 -6,  -6, 1, 315, 1.0 },
		{	 -8,   0, 1,   0, 1.0 },
		{	 -6,   6, 1,  45, 1.0 },
		{	  0,   8, 1,  90, 1.0 },
		{	  6,   6, 1, 135, 1.0 },
		// Soft nubs
		{	 12,   0, 0, 180, 1.0 },
		{	  9,  -9, 0, 225, 1.0 },
		{	  0, -12, 0, 270, 1.0 },
		{	 -9,  -9, 0, 315, 1.0 },
		{	-12,   0, 0,   0, 1.0 },
		{	 -9,   9, 0,  45, 1.0 },
		{	  0,  12, 0,  90, 1.0 },
		{	  9,   9, 0, 135, 1.0 }
	};

////////////////////////////////////////////////////////////////////////////////
// Get the various components of this animation from the resource names
// specified in the provided array of pointers to strings.
////////////////////////////////////////////////////////////////////////////////
// virtual								// Overridden here.
short CDude::CDudeAnim3D::Get(	// Returns 0 on success.
	char*		pszBaseFileName,		// In:  Base string for resource filenames.
	char*		pszRigidName,			// In:  String to add for rigid transform channel
											// or NULL for none.
	char*		pszEventName,			// In:  String to add for event states channel
											// or NULL for none.
	short		sLoopFlags)				// In:  Looping flags to apply to all channels
											// in this anim.
	{
	short	sRes;
	char	szResName[RSP_MAX_PATH];
	sprintf(szResName, "%s.sop", pszBaseFileName);
	sRes	=  rspGetResource(&g_resmgrGame, szResName, &m_psops);
	sprintf(szResName, "%s.mesh", pszBaseFileName);
	sRes	|= rspGetResource(&g_resmgrGame, szResName, &m_pmeshes);
	sprintf(szResName, "%s.bounds", pszBaseFileName);
	sRes	|= rspGetResource(&g_resmgrGame, szResName, &m_pbounds);
	if (pszRigidName != NULL)
		{
		sprintf(szResName, "%s_%s.trans", pszBaseFileName, pszRigidName);
		sRes	|= rspGetResource(&g_resmgrGame, szResName, &m_ptransRigid);
		}

	if (pszEventName != NULL)
		{
		sprintf(szResName, "%s_%s.event", pszBaseFileName, pszEventName);
		sRes	|= rspGetResource(&g_resmgrGame, szResName, &m_pevent);
		}

	// We always load these transforms.
	sprintf(szResName, "%s_" LEFT_HAND_RIGID_ANIM_NAME ".trans", pszBaseFileName);
	sRes	|= rspGetResource(&g_resmgrGame, szResName, &m_ptransLeft);
	sprintf(szResName, "%s_" RIGHT_HAND_RIGID_ANIM_NAME ".trans", pszBaseFileName);
	sRes	|= rspGetResource(&g_resmgrGame, szResName, &m_ptransRight);
	sprintf(szResName, "%s_" BACK_RIGID_ANIM_NAME ".trans", pszBaseFileName);
	sRes	|= rspGetResource(&g_resmgrGame, szResName, &m_ptransBack);

	// If successful . . .
	if (sRes == 0)
		{
		m_psops->SetLooping(sLoopFlags);
		m_pmeshes->SetLooping(sLoopFlags);
		m_pbounds->SetLooping(sLoopFlags);
		if (m_ptransRigid != NULL)
			m_ptransRigid->SetLooping(sLoopFlags);
		if (m_pevent != NULL)
			m_pevent->SetLooping(sLoopFlags);

		m_ptransLeft->SetLooping(sLoopFlags);
		m_ptransRight->SetLooping(sLoopFlags);
		m_ptransBack->SetLooping(sLoopFlags);
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////////////
// Release all resources.
////////////////////////////////////////////////////////////////////////////////
// virtual										// Overridden here.
void CDude::CDudeAnim3D::Release(void)	// Returns nothing.
	{
	rspReleaseResource(&g_resmgrGame, &m_psops);
	rspReleaseResource(&g_resmgrGame, &m_pmeshes);
	rspReleaseResource(&g_resmgrGame, &m_pbounds);
	if (m_ptransRigid != NULL)
		rspReleaseResource(&g_resmgrGame, &m_ptransRigid);
	if (m_pevent != NULL)
		rspReleaseResource(&g_resmgrGame, &m_pevent);

	rspReleaseResource(&g_resmgrGame, &m_ptransLeft);
	rspReleaseResource(&g_resmgrGame, &m_ptransRight);
	rspReleaseResource(&g_resmgrGame, &m_ptransBack);
	}


////////////////////////////////////////////////////////////////////////////////
// Constructor.
////////////////////////////////////////////////////////////////////////////////
CDude::CDude(CRealm* pRealm)
	: CCharacter(pRealm, CDudeID)
	{
	m_statePersistent	= State_Stand;

	// Set default stockpile.
	m_stockpile.Copy(&ms_stockpileDefault);

	m_weapontypeCur	= SemiAutomatic;
	m_weaponShooting	= NoWeapon;

	m_lLastShotTime	= 0;
	m_lLastYellTime	= 0;

	m_u16IdChild		= CIdBank::IdNil;

	m_sTextureIndex	= 0;

	m_u8LastEvent		= 0;

	m_idVictim			= CIdBank::IdNil;

	m_bDead				= false;

	m_bInvincible		= false;

	// Base the dude number of the number of dude's in the realm.  Note that
	// this number already includes this dude, so we subtract 1 from so the
	// assigned dude numbers will start at 0.
	ASSERT(m_pRealm->m_asClassNumThings[CDudeID] > 0);
	m_sDudeNum = m_pRealm->m_asClassNumThings[CDudeID] - 1;
	}

////////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////////
CDude::~CDude()
	{
	// Kill dude
	Kill();
	}

////////////////////////////////////////////////////////////////////////////////
// Load object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CDude::Load(										// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,											// In:  File to load from
	bool bEditMode,										// In:  True for edit mode, false otherwise
	short sFileCount,										// In:  File count (unique per file, never 0)
	ULONG	ulFileVersion)									// In:  File version being loaded.
	{
	short sResult = 0;

	// In most cases, the base class Load() should be called.
	sResult	= CCharacter::Load(pFile, bEditMode, sFileCount, ulFileVersion);
	if (sResult == 0)
		{
		// Load common data just once per file (not with each object)
		if (ms_sFileCount != sFileCount)
			{
			ms_sFileCount = sFileCount;

			switch (ulFileVersion)
				{
				default:	// Newer versions where this format has not changed.
				case 1:
					// Load static data
					pFile->Read(&ms_dAccUser);
					pFile->Read(&ms_dAccDrag);
					pFile->Read(&ms_dMaxVelFore);
					pFile->Read(&ms_dMaxVelBack);
					pFile->Read(&ms_dMaxVelForeFast);
					pFile->Read(&ms_dMaxVelBackFast);
					pFile->Read(&ms_dDegPerSec);
					break;
				}
			}

			switch (ulFileVersion)
				{
				default:	// Newer versions where this format has not changed.
				case 16:
					// Nothing to load here.
					break;
				case 15:
				case 14:
				case 13:
					pFile->Read(&m_stockpile.m_sNumHeatseekers);
					pFile->Read(&m_stockpile.m_sNumMines);
				case 12:
				case 11:
				case 10:
					pFile->Read(&m_stockpile.m_sNumFuel);
				case 9:
					pFile->Read(&m_stockpile.m_sNumShells);
				case 8:
				case 7:
				case 6:
				case 5:
				case 4:
				case 3:
				case 2:
					pFile->Read(&m_stockpile.m_sNumBullets);
					pFile->Read(&m_stockpile.m_sNumFireBombs);
					pFile->Read(&m_stockpile.m_sNumMissiles);
					pFile->Read(&m_stockpile.m_sNumNapalms);
				case 1:
					{
					pFile->Read(&m_stockpile.m_sNumGrenades);

					U32	u32Temp;
					pFile->Read(&u32Temp);
					m_state	= (CCharacter::State)u32Temp;

					break;
					}
				}

		// Make sure there were no file errors or format errors . . .
		if (!pFile->Error() && sResult == 0)
			{
			// Init dude
			sResult = Init();
			}
		else
			{
			sResult = -1;
			TRACE("CDude::Load(): Error reading from file!\n");
			}
		}
	else
		{
		TRACE("CDude::Load(): CCharacter::Load() failed.\n");
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Save object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CDude::Save(										// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,											// In:  File to save to
	short sFileCount)										// In:  File count (unique per file, never 0)
	{
	short sResult = 0;

	// In most cases, the base class Save() should be called.
	sResult	= CCharacter::Save(pFile, sFileCount);
	if (sResult == 0)
		{
		// Save common data just once per file (not with each object)
		if (ms_sFileCount != sFileCount)
			{
			ms_sFileCount = sFileCount;

			// Save static data.
			pFile->Write(&ms_dAccUser);
			pFile->Write(&ms_dAccDrag);
			pFile->Write(&ms_dMaxVelFore);
			pFile->Write(&ms_dMaxVelBack);
			pFile->Write(&ms_dMaxVelForeFast);
			pFile->Write(&ms_dMaxVelBackFast);
			pFile->Write(&ms_dDegPerSec);
			}

		// Save object data.
		sResult	= pFile->Error();
		}
	else
		{
		TRACE("CDude::Save(): CCharacter::Save() failed.\n");
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Startup object
////////////////////////////////////////////////////////////////////////////////
short CDude::Startup(void)						// Returns 0 if successfull, non-zero otherwise
	{
	short	sResult	= CCharacter::Startup();
	
	// Init other stuff
	m_lNextBulletTime = m_pRealm->m_time.GetGameTime() + MS_BETWEEN_BULLETS;

	// Set start position for crawler verification.
	m_dLastCrawledToPosX	= m_dX; 
	m_dLastCrawledToPosZ	= m_dZ; 

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Shutdown object
////////////////////////////////////////////////////////////////////////////////
short CDude::Shutdown(void)							// Returns 0 if successfull, non-zero otherwise
	{
	short	sResult	= CCharacter::Shutdown();
	return sResult;
	}

////////////////////////////////////////////////////////////////////////////////
// Suspend object
////////////////////////////////////////////////////////////////////////////////
void CDude::Suspend(void)
	{
	if (m_sSuspend == 0)
		{
		// Store current delta so we can restore it.
		long	lCurTime		= m_pRealm->m_time.GetGameTime();
		m_lNextBulletTime	= lCurTime - m_lNextBulletTime;
		}

	CCharacter::Suspend();
	}

////////////////////////////////////////////////////////////////////////////////
// Resume object
////////////////////////////////////////////////////////////////////////////////
void CDude::Resume(void)
	{
	CCharacter::Resume();

	if (m_sSuspend == 0)
		{
		long	lCurTime		= m_pRealm->m_time.GetGameTime();
		m_lNextBulletTime	= lCurTime - m_lNextBulletTime;
		}
	}


////////////////////////////////////////////////////////////////////////////////
// Update object
////////////////////////////////////////////////////////////////////////////////
void CDude::Update(void)
	{
	if (!m_sSuspend)
		{
		// Get new time
		long lThisTime = m_pRealm->m_time.GetGameTime();

		// Advance the animation timer.
		long	lDifTime		= lThisTime - m_lAnimPrevUpdateTime;
		m_lAnimTime			+= lDifTime;

		// Update prev time.
		m_lAnimPrevUpdateTime	= lThisTime;

		// Process Input ////////////////////////////////////////////////////////

		double	dMaxForeVel;
		double	dMaxBackVel;
		short		sStrafeAngle	= INVALID_STRAFE_ANGLE;	// Angle of motion from strafing.

		ProcessInput(
			&dMaxForeVel,
			&dMaxBackVel,
			&sStrafeAngle
			);

		// Process Forces ///////////////////////////////////////////////////////

		switch (m_state)
			{
			case State_BlownUp:
				break;
			default:
				ProcessForces(
					lThisTime,
					dMaxForeVel,
					dMaxBackVel,
					sStrafeAngle);
				break;
			}
		
		// Specific transitions by state dependent on motion ////////////////////

		switch (m_state)
			{
			default:	// Catch all.  Go to idle if NYI state.
			case State_Idle:
				break;
			case State_Dead:
			case State_Strafe:
			case State_StrafeAndShoot:
			case State_Stand:
			case State_Shooting:
				// If there's any movement . . . 
				if (m_dAcc != 0.0)
					{
					// Move to running.
					switch (m_state)
						{
						case State_Stand:
						case State_Strafe:
							SetState(State_Run);
							break;
						case State_StrafeAndShoot:
						case State_Shooting:
							SetState(State_RunAndShoot);
							break;
						}
					}
				else
					{
					// If there's strafe motion . . .
					if (sStrafeAngle != INVALID_STRAFE_ANGLE)
						{
						switch (m_state)
							{
							case State_Stand:
								SetState(State_Strafe);
								break;
							case State_Shooting:
								SetState(State_StrafeAndShoot);
								break;
							case State_Strafe:
							case State_StrafeAndShoot:
								break;
							}
						}
					else
						{
						switch (m_state)
							{
							case State_Stand:
							case State_Shooting:
								break;
							case State_Strafe:
								SetState(State_Stand);
								break;
							case State_StrafeAndShoot:
								SetState(State_Shooting);
								break;
							}
						}
					}
				break;
			case State_Burning:
				{
				// Tweak out.
				m_dRot	= rspMod360(m_dRot + RAND_SWAY(ON_FIRE_ROT_TWEAKAGE) );
				m_dVel	+= RAND_SWAY(ON_FIRE_VEL_TWEAKAGE) + ON_FIRE_VEL_TWEAKAGE / 4;	// **FUDGE**
				}

				// Intentional fall through.
			case State_Run:
			case State_RunAndShoot:
				// If there's little movement . . .
				if ((m_dVel < MIN_RUN_VEL && m_dVel > -MIN_RUN_VEL && m_dAcc == 0.0))
					{
					// If not strafing . . .
					if (sStrafeAngle == INVALID_STRAFE_ANGLE)
						{
						// Move to standing.
						switch (m_state)
							{
							case State_Run:
								SetState(State_Stand);
								break;
							case State_RunAndShoot:
								SetState(State_Shooting);
								break;
							case State_Burning:
								break;
							}
						}
					else	// We are strafing in some way.
						{
						// Move to standing.
						switch (m_state)
							{
							case State_Run:
								SetState(State_Strafe);
								break;
							case State_RunAndShoot:
								SetState(State_StrafeAndShoot);
								break;
							case State_Burning:
								break;
							}
						}
					}
				else
					{
						if (GetInput(m_sDudeNum) & INPUT_RUN)
							{
							if (StatsAreAllowed)
								{
								Stat_TimeRunning += lDifTime;
								if (Stat_TimeRunning >= (60 * 5 * 1000))
									UnlockAchievement(ACHIEVEMENT_RUN_5_MINUTES);
								}
							}
					}
				break;
			case State_Throw:
			case State_ThrowRelease:
			case State_ThrowDone:
			case State_ThrowFinish:
			case State_Launch:
			case State_LaunchRelease:
			case State_LaunchDone:
			case State_LaunchFinish:
				// Let's just say you can't move while throwing/launching.
				m_dVel	= 0.0;
				m_dAcc	= 0.0;
				m_dDrag	= 0.0;
				break;
			}

		// Service message queue ////////////////////////////////////////////////
		ProcessMessages();

		// Switch on state.
		switch (m_state)
			{
			case State_Stand:
				{
#if 1	// No idle anim currently.
				// If we've been idle for a while . . .
				if (m_lAnimTime > m_lNextIdleTime)
					{
					// Toggle anim.
					if (m_panimCur == &m_animStand)
						{
						// Switch to idle.
						m_panimCur			= &m_animIdle;
						// Set next timeout.
						m_lNextIdleTime	= m_animIdle.m_psops->TotalTime() * IDLE_ANIM_LOOPS;
						}
					else
						{
						// Switch to stand.
						m_panimCur			= &m_animStand;
						// Set next timeout.
						m_lNextIdleTime	= IDLE_ANIM_TIMEOUT;
						}

					// Reset anim timer.
					m_lAnimTime					= 0;
					m_lAnimPrevUpdateTime	= lThisTime;
					}
#endif
				break;
				}
			case State_Throw:
				{
#if 1
				U8	u8Event	= *( (U8*)(m_panimCur->m_pevent->GetAtTime(m_lAnimTime) ) );
				// Check for release point in animation . . .
				if (u8Event > 0)
					{
					CWeapon*	pweapon;
					if (m_pRealm->m_idbank.GetThingByID((CThing**)&pweapon, m_u16IdWeapon) == 0)
						{
						if (pweapon->m_eState == CWeapon::State_Hide)
							{
							// Unhide grenade.
							pweapon->m_eState = CWeapon::State_Idle;
							}
						else if (u8Event > 1)
							{
							// Release projectile.
							// Go to State_ThrowRelease which is purely transitional to State_ThrowFinish.
							SetState(State_ThrowRelease);
							}
						}
					else
						{
						if (u8Event > 1)
							{
							// Release projectile.
							// Go to State_ThrowRelease which is purely transitional to State_ThrowFinish.
							SetState(State_ThrowRelease);
							}
						}
					}
#else
				if (WhileHoldingWeapon(
					COLLISION_BITS_INCLUDE, 
					COLLISION_BITS_DONTCARE, 
					COLLISION_BITS_EXCLUDE) == true)
					{
					// Go to State_ThrowFinish.
					SetState(State_ThrowFinish);
					}
#endif
				break;
				}
			case State_ThrowRelease:
				{
				ShootWeapon();
				// Finish animation.
				SetState(State_ThrowFinish);
				break;
				}
			case State_ThrowDone:
			case State_LaunchDone:
				// Move on.
				SetState(State_Persistent);
				break;
			case State_LaunchRelease:
				{
				ShootWeapon();
				// Finish animation.
				SetState(State_LaunchFinish);
				break;
				}
			case State_ThrowFinish:
				// Check for end of throw animation.
				if (m_lAnimTime > m_panimCur->m_psops->TotalTime())
					{
					// This will let us know to be done with throwing on the next Update.
					SetState(State_ThrowDone);
					}
				break;
			case State_Run:
				{
				// Remove adjustment made above.
				m_lAnimTime	-= lDifTime;
				// Tune the time.  Note that this is automathematically negative when
				// velocity is negative.
				long	lDifAnimTime	= lDifTime * (m_dVel / RUN_ANIM_VELOCITY);

				m_lAnimTime	+= lDifAnimTime;

				// Play step noise if event has changed.
				PlayStep();

				break;
				}
			case State_RunAndShoot:
				{
				// Remove adjustment made above.
				m_lAnimTime	-= lDifTime;
				// Tune the time.  Note that this is automathematically negative when
				// velocity is negative.
				long	lDifAnimTime	= lDifTime * (m_dVel / RUN_ANIM_VELOCITY);

				m_lAnimTime	+= lDifAnimTime;

				// Play step noise if event has changed.
				PlayStep();

				if (lThisTime >= m_lNextBulletTime)
					{
					// Shoot.
					ShootWeapon();
					}

				break;
				}
			case State_Shooting:
				// If out of bullets . . .
				if (m_stockpile.m_sNumBullets <= 0)
					{
					// Don't animate.
					m_lAnimTime	-= lDifTime;
					}

				if (lThisTime >= m_lNextBulletTime)
					{
					// Shoot.
					ShootWeapon();
					}
				break;
			case State_StrafeAndShoot:

				// If going left of forwards . . .
				if (m_dRot - sStrafeAngle < 0)
					{
					// Subtract twice the time to make up for fact that it was already added once above
					m_lAnimTime	-= lDifTime * 2;
					}

				// Play step noise if event has changed.
				PlayStep();

				if (lThisTime >= m_lNextBulletTime)
					{
					// Shoot.
					ShootWeapon();
					}
				break;
			case State_Strafe:
				// If going left of forwards . . .
				if (m_dRot - sStrafeAngle < 0)
					{
					// Subtract twice the time to make up for fact that it was already added once above
					m_lAnimTime	-= lDifTime * 2;
					}

				// Play step noise if event has changed.
				PlayStep();

				break;
			case State_Shot:
				// Check for end of damage animation . . .
//				if (m_lAnimTime > m_panimCur->m_psops->TotalTime())
				// Instead check for minimum duration since last shot time . . .
				if (lThisTime > m_lLastShotTime + MIN_SHOT_DURATION)
					{
					// Stand.
					SetState(State_Persistent);
					}
				break;
			case State_BlownUp:
				// Check for end . . .
				if (WhileBlownUp() == false)
					{
					SetState(State_GetUp);
					}
				break;
			case State_Suicide:
				// Check for moment of firing weapon . . .
				if (*((U8*)(m_panimCur->m_pevent->GetAtTime(m_lAnimTime))) > 0
					&& m_bBrainSplatted == false)		// ***FUDGE***
					{
					// Play shot fire.
					PlaySample(g_smidShotgun, SampleMaster::Weapon);
					// Play shot received.
					PlaySample(g_smidDyingYell, SampleMaster::Voices);
					// Start gore.
					StartBrainSplat();
					// Remember.
					m_bBrainSplatted	= true;

					// Register the kill.
					ScoreRegisterKill(m_pRealm, GetInstanceID(), m_u16InstanceId);

					// Note that he's dead
					m_bDead = true;
					}

				// Intentional fall through to State_Die.
			case State_Die:
				// Check for end of die animation . . .
				if (m_lAnimTime > m_panimCur->m_psops->TotalTime())
					{
					// Make sure we're dead for reals.  Do this before the SetState()
					// so we doesn't end up dropping any health.
					m_stockpile.m_sHitPoints	= 0;

					// Go to dead.
					SetState(State_Dead);
					// If he is carying the flag item, then he should drop it.
					DropAllFlags(NULL);
					}
				else if (m_bGenericEvent1 == false)
					{
					U8	u8Event	= *( (U8*)(m_panimCur->m_pevent->GetAtTime(m_lAnimTime) ) );

					if (u8Event > 1)	// ***FUDGE***
						{
						PlaySample(g_smidBodyImpact2, SampleMaster::Unspecified);
						m_bGenericEvent1	= true;
						}
					}
				break;
			case State_Burning:
				{
				// Play step noise if event has changed.
				PlayStep();

				CThing*	pthingFire;
				// If the fire's gone . . .
				if (m_pRealm->m_idbank.GetThingByID(&pthingFire, m_u16IdFire) == 0)
					{
					if (!((CFire*) (pthingFire))->IsBurning())
					// Stand.
					SetState(State_Stand);
					}
					else
					{
					// Stand.
					SetState(State_Stand);
					}
				break;
				}
			case State_Launch:
#if 0
				{
				U8	u8Event	= *( (U8*)(m_panimCur->m_pevent->GetAtTime(m_lAnimTime) ) );
				// Check for launch point in animation . . .
				if (u8Event > 0)
					{
					CWeapon*	pweapon;
					if (m_pRealm->m_idbank.GetThingByID((CThing**)&pweapon, m_u16IdWeapon) == 0)
						{
						if (pweapon->m_eState == CWeapon::State_Hide)
							{
							// Unhide missile.
							pweapon->m_eState = CWeapon::State_Idle;
							}
						else if (u8Event > 1)
							{
							// Release missile.
							// Go to State_LaunchRelase which is purely transitional to State_LaunchFinish.
							SetState(State_LaunchRelease);
							}
						}
					else
						{
						if (u8Event > 1)
							{
							// Release missile.
							// Go to State_LaunchRelase which is purely transitional to State_LaunchFinish.
							SetState(State_LaunchRelease);
							}
						}
					}
				}
#else
				if (WhileHoldingWeapon(
					COLLISION_BITS_INCLUDE, 
					COLLISION_BITS_DONTCARE, 
					COLLISION_BITS_EXCLUDE) == true)
					{
					// Go to State_LaunchFinish.
					SetState(State_LaunchFinish);
					}
#endif
				break;
			case State_LaunchFinish:
				// Check for end of launch animation . . .
				if (m_lAnimTime > m_panimCur->m_psops->TotalTime())
					{
					// Go to State_LaunchDone which is purely transitional to State_Stand.
					SetState(State_LaunchDone);
					}
				break;
			case State_GetUp:
				// check for end of getup anim . . .
				if (m_lAnimTime > m_panimCur->m_psops->TotalTime())
					{
					// Go to last persistent state.  Usually stand.
					SetState(State_Persistent);
					}
				break;
			case State_Rise:
				// check for end of rise anim . . .
				if (m_lAnimTime > m_panimCur->m_psops->TotalTime())
					{
					// Go to last persistent state.  Usually stand.
					SetState(State_Persistent);
					}
				break;
			case State_Jump:
			case State_JumpForward:
				if (m_lAnimTime > 750 && m_bJumpVerticalTrigger == false)
					{
					// Go up.
					m_dExtVertVel	= ms_dVertVelJump;
					m_bJumpVerticalTrigger	= true;
					}

				if (m_lAnimTime > m_panimCur->m_psops->TotalTime())
					{
					// Go to fall state.
					SetState(State_Fall);
					}
				else
					{
					break;
					}

			// Intentional fall through to State_Fall.
			case State_Fall:
				// If we've reached the ground . . .
				if (m_bAboveTerrain == false)
					{
					// Go to land state.
					SetState((m_dVel == 0.0) ? State_Land : State_LandForward);
					}
				break;
			case State_Land:
			case State_LandForward:
				// Return to last persistent state.
				SetState(State_Persistent);
				break;
			case State_Execute:
				// check for end of execute anim . . .
				if (m_lAnimTime > m_panimCur->m_psops->TotalTime())
					{
					// Go to last persistent state.  Usually stand.
					SetState(State_Persistent);
					UnlockAchievement(ACHIEVEMENT_PERFORM_FIRST_EXECUTION);
					if (StatsAreAllowed) Stat_Executions++;
					}
				// If before point of firing . . .
				else if (m_lAnimTime < 1350)	// ***FUDGE***.
					{
					// Track executee.  If we lost 'em . . .
					if (TrackExecutee(lDifTime / 1000.0) == false)
						{
						// Go to last persistent state.  Usually stand.
						SetState(State_Persistent);
						}
					}
				// If in point of firing . . .
				else if (m_lAnimTime < 1950)	// ***FUDGE***.
					{
#if 1
					if (lThisTime >= m_lNextBulletTime)
						{
						OnExecute();
						}
#else
					if (lThisTime >= m_lNextBulletTime)
						{
						ArmWeapon(SemiAutomatic);
						ShootWeapon();
						}
#endif
					}
				break;
			case State_PutDown:
#if 0
				// Check for end of anim . . .
				if (m_lAnimTime > m_panimCur->m_psops->TotalTime())
					{
					// Go to last persistent state.  Usually stand.
					SetState(State_Persistent);
					}
				else 
					{
					U8	u8Event	= *( (U8*)(m_panimCur->m_pevent->GetAtTime(m_lAnimTime) ) );
					// Check for show point in anim . . .
					if (u8Event > 0)
						{
					// Check for drop point in animation . . .
					if (u8Event > 1)
						{
						if (m_u16IdWeapon != CIdBank::IdNil)
							{
							ShootWeapon();
							}
						}
					}
#else
				if (WhileHoldingWeapon(
					COLLISION_BITS_INCLUDE, 
					COLLISION_BITS_DONTCARE, 
					COLLISION_BITS_EXCLUDE) == true)
					{
					// Go to State_ObjectReleased.
					SetState(State_ObjectReleased);
					}
				break;
			case State_ObjectReleased:
				// Check for end of anim . . .
				if (m_lAnimTime > m_panimCur->m_psops->TotalTime())
					{
					// Go to last persistent state.  Usually stand.
					SetState(State_Persistent);
					}
#endif
				break;
			case State_PickUp:
				{
				CPowerUp*	ppowerup;
				// Check for end of anim . . .
				if (m_lAnimTime > m_panimCur->m_psops->TotalTime())
					{
					// If we still have the picked up item . . .
					if (m_pRealm->m_idbank.GetThingByID((CThing**)&ppowerup, m_u16IdChild) == 0)
						{
						TakePowerUp(&ppowerup);

						m_u16IdChild	= CIdBank::IdNil;
						}

					// Go to last persistent state.  Usually stand.
					SetState(State_Persistent);
					}
				else if (m_lAnimTime > 500 && m_bGenericEvent1 == false)
					{
					// This should be QuickCheckClosest, when available . . .
					CSmash*	psmash	= NULL;	// Safety.
					if (m_pRealm->m_smashatorium.QuickCheck(	// Returns true if collision detected, false otherwise
						&m_smash,										// In:  CSmash to check
						CSmash::PowerUp,								// In:  Bits that must be 1 to collide with a given CSmash
						0,													// In:  Bits that you don't care about
						0,													// In:  Bits that must be 0 to collide with a given CSmash
						&psmash) == true)								// Out: Thing being smashed into if any (unless 0)
						{
						ASSERT(psmash->m_pThing != NULL);

						// Make it blit as a child.
						ASSERT(psmash->m_pThing->GetClassID() == CPowerUpID);
						ppowerup	= (CPowerUp*)psmash->m_pThing;
						if (ppowerup->Grab(&m_sprite) == 0)
							{
							// Store/Identify child.
							m_u16IdChild	= ppowerup->GetInstanceID();
							}

						// Note we already did this.
						m_bGenericEvent1	= true;
						}
					}
				else if (m_pRealm->m_idbank.GetThingByID((CThing**)&ppowerup, m_u16IdChild) == 0)
					{
					// Position powerup.
					PositionChild(
						&(ppowerup->m_sprite),	// In:  Child sprite to detach.
						(RTransform*) m_panimCur->m_ptransRigid->GetAtTime(m_lAnimTime),	// In:  Transform specifying position.
						&(ppowerup->m_dX),		// Out: New position of child. 
						&(ppowerup->m_dY),		// Out: New position of child. 
						&(ppowerup->m_dZ) );		// Out: New position of child. 
					}
				break;
				}
			}

		// Update sphere.
		m_smash.m_sphere.sphere.X			= m_dX;
		// Fudge center of sphere as half way up the dude.
		// Doesn't work if dude's feet leave the origin.
		m_smash.m_sphere.sphere.Y			= m_dY + m_sprite.m_sRadius;
		m_smash.m_sphere.sphere.Z			= m_dZ;
		m_smash.m_sphere.sphere.lRadius	= m_sprite.m_sRadius;

		// Update the smash.
		m_pRealm->m_smashatorium.Update(&m_smash);

		// Check for powerups.
		CSmash*	psmash	= NULL;	// Safety.
		if (m_pRealm->m_smashatorium.QuickCheck(	// Returns true if collision detected, false otherwise
			&m_smash,										// In:  CSmash to check
			CSmash::PowerUp,								// In:  Bits that must be 1 to collide with a given CSmash
			0,													// In:  Bits that you don't care about
			0,													// In:  Bits that must be 0 to collide with a given CSmash
			&psmash) == true)								// Out: Thing being smashed into if any (unless 0)
			{
			ASSERT(psmash->m_pThing != NULL);
			ASSERT(psmash->m_pThing->GetClassID() == CPowerUpID);

			CPowerUp* ppowerup	= (CPowerUp*)psmash->m_pThing;

			// If we're not dead or the powerup contains health . . .
			if (m_bDead == false || ppowerup->m_stockpile.m_sHitPoints > 0)
				{
				TakePowerUp(&ppowerup);
				}
			}

		// Save time for next time
		m_lPrevTime = lThisTime;

		// Call the targeting aid function
		ShowTarget();

		// Call base class //////////////////////////////////////////////////////

		CCharacter::Update();

		// If requested to delete self . . .
		if (m_state == State_Delete)
			{
			// Good bye.
			delete this;
			}
		}
	}


////////////////////////////////////////////////////////////////////////////////
// Attempts user motivated state transitions.
////////////////////////////////////////////////////////////////////////////////
void CDude::ProcessInput(		// Returns nothing.
	double*	pdMaxForeVel,		// Out: Maximum forward velocity.
	double*	pdMaxBackVel,		// Out: Maximum backward velocity.
	short*	psStrafeAngle)		// Out: Strafe angle.
	{
	UINPUT input = 0;
	// Get user input
	input	= GetInput(m_sDudeNum);

	bool bCanMove = true;		// Flags for twinstick input so they don't override death, etc. states
	bool bCanFire = true;

	switch (m_state)
		{
		case State_Dead:
			// Let's take jump as 'revive' . . .
			if (input & INPUT_REVIVE)
				{
				// It's alive; alive!
				Revive();
				}

			// Allow only one input (life cheat) . . .
			if ( (input & INPUT_WEAPONS_MASK) == INPUT_CHEAT_15)
				{
				input	= INPUT_CHEAT_15;
				}
			else
				{
				// No other user control during these states.
				input	= 0;
				bCanMove = false;
				bCanFire = false;
				}
			break;
		case State_Die:
		case State_Suicide:
			// No user control during these states.
			input	= 0;
			bCanMove = false;
			bCanFire = false;
			break;
		case State_BlownUp:
		case State_Rise:
		case State_GetUp:
		case State_PutDown:
		case State_ObjectReleased:
		case State_PickUp:
			// No user control during these states other than to change weapon.
			input	&= INPUT_WEAPONS_MASK;
			bCanMove = false;
			bCanFire = false;
			break;
		case State_Throw:
		case State_ThrowRelease:
		case State_ThrowDone:
		case State_ThrowFinish:
		case State_Launch:
		case State_LaunchRelease:
		case State_LaunchDone:
		case State_LaunchFinish:
		//case State_Shot:
			// No influence on velocity during these states.
			input &= ~(INPUT_FORWARD | INPUT_BACKWARD | INPUT_REVIVE | INPUT_DUCK | INPUT_MOVE_UP | INPUT_MOVE_DOWN | INPUT_MOVE_LEFT | INPUT_MOVE_RIGHT);
			bCanMove = false;
			bCanFire = false;
			break;
		case State_Execute:
		case State_Duck:
			// No rotation or jumping.
			input &= ~(INPUT_LEFT | INPUT_RIGHT | INPUT_REVIVE | INPUT_ROT_MASK | INPUT_FORWARD | INPUT_BACKWARD | INPUT_MOVE_UP | INPUT_MOVE_DOWN | INPUT_MOVE_LEFT | INPUT_MOVE_RIGHT);
			bCanMove = false;
			bCanFire = false;
			break;
		}

	// Change weapon . . .
	WeaponType	wtNew					= m_weapontypeCur;
	switch (input & INPUT_WEAPONS_MASK)
		{
		case INPUT_WEAPON_1:
		case INPUT_WEAPON_2:
		case INPUT_WEAPON_3:
		case INPUT_WEAPON_4:
		case INPUT_WEAPON_5:
//#ifdef MOBILE //We want weapon 5 to toggle weapon
if (!demoCompat)
{
			if (((input & INPUT_WEAPONS_MASK) == INPUT_WEAPON_5)
				&& (m_weapontypeCur == Rocket))
			{
				wtNew = Heatseeker;
				break;
			}
}
//#endif
		case INPUT_WEAPON_6:
		case INPUT_WEAPON_7:
		case INPUT_WEAPON_8:
		case INPUT_WEAPON_9:
			wtNew	= (WeaponType)(NoWeapon + ( ( (input & INPUT_WEAPONS_MASK) - INPUT_WEAPON_0) >> INPUT_WEAPONS_BIT) );
			break;
		case INPUT_WEAPON_0:
			switch (m_weapontypeCur)
				{
				case NoWeapon:
					if (m_stockpile.m_sDeathWadLauncher)
						{
						wtNew	= DeathWad;
						break;
						}
				case DeathWad:
					if (m_stockpile.m_sDoubleBarrel)
						{
						wtNew	= DoubleBarrel;
						break;
						}
				case DoubleBarrel:
				default:
					wtNew	= NoWeapon;
					break;
				}
			break;
		case INPUT_WEAPON_10:
			switch (m_weapontypeCur)
				{
				default:
					wtNew	= ProximityMine;
					break;
				case ProximityMine:
					wtNew	= TimedMine;
					break;
				case TimedMine:
#if 0	// Remote mines are not very useful currently.
					wtNew	= RemoteMine;
					break;
				case RemoteMine:
#endif
					wtNew	= BouncingBettyMine;
					break;
				}
			break;
		
		// 'Cheats' ////////////////////////////////////////////////////
		
		case INPUT_CHEAT_11:	// Restore health.
			{
			CStockPile	stockpile	= { 0, };
			stockpile.m_sHitPoints	= MAX(0, m_sOrigHitPoints - m_stockpile.m_sHitPoints);

			CreateCheat(&stockpile);
			break;
			}
		case INPUT_CHEAT_12:	// Full kevlar.
			{
			CStockPile	stockpile		= { 0, };
			stockpile.m_sKevlarLayers	= CStockPile::ms_stockpileBackPackMax.m_sKevlarLayers;

			CreateCheat(&stockpile);
			break;
			}
		case INPUT_CHEAT_13:	// Backpack.
			{
			CStockPile	stockpile	= { 0, };
			stockpile.m_sBackpack	= 1;

			CreateCheat(&stockpile);
			break;
			}
		case INPUT_CHEAT_14:	// Full weaponry, full ammo, backpack, and full kevlar.
			{
			CStockPile	stockpile	= { 0, };
			stockpile.m_sHitPoints	= MAX(0, m_sOrigHitPoints - m_stockpile.m_sHitPoints);

			stockpile.m_sNumGrenades		= CStockPile::ms_stockpileBackPackMax.m_sNumGrenades;
			stockpile.m_sNumFireBombs		= CStockPile::ms_stockpileBackPackMax.m_sNumFireBombs;
			stockpile.m_sNumMissiles		= CStockPile::ms_stockpileBackPackMax.m_sNumMissiles;
			stockpile.m_sNumNapalms			= CStockPile::ms_stockpileBackPackMax.m_sNumNapalms;
			stockpile.m_sNumBullets			= CStockPile::ms_stockpileBackPackMax.m_sNumBullets;
			stockpile.m_sNumShells			= CStockPile::ms_stockpileBackPackMax.m_sNumShells;
			stockpile.m_sNumMines			= CStockPile::ms_stockpileBackPackMax.m_sNumMines;
			stockpile.m_sNumHeatseekers	= CStockPile::ms_stockpileBackPackMax.m_sNumHeatseekers;
			stockpile.m_sNumFuel				= CStockPile::ms_stockpileBackPackMax.m_sNumFuel;

			stockpile.m_sMachineGun			= 1;
			stockpile.m_sMissileLauncher	= 1;
			stockpile.m_sShotGun				= 1;
			stockpile.m_sSprayCannon		= 1;
			stockpile.m_sFlameThrower		= 1;
			stockpile.m_sNapalmLauncher	= 1;

			stockpile.m_sKevlarLayers		= CStockPile::ms_stockpileMax.m_sKevlarLayers;

			stockpile.m_sBackpack			= 1;

			CreateCheat(&stockpile);
			break;
			}
		case INPUT_CHEAT_15:	// Revive.
			{
			UnlockAchievement(ACHIEVEMENT_ENABLE_CHEATS);
			Flag_Achievements |= FLAG_USED_CHEATS;

			Revive(false);
			// Let the demon know they entered a cheat code
			GameMessage msg;
			msg.msg_Cheater.eType = typeCheater;
			msg.msg_Cheater.sPriority = 0;
			CThing* pDemon = m_pRealm->m_aclassHeads[CThing::CDemonID].GetNext();
			if (pDemon)
				SendThingMessage(&msg, pDemon);				
			break;
			}
		case INPUT_CHEAT_16:	// DeathWadLauncher and ammo.
			{
			CStockPile	stockpile			= { 0, };
			stockpile.m_sNumGrenades		= CStockPile::ms_stockpileBackPackMax.m_sNumGrenades;
			stockpile.m_sNumMissiles		= CStockPile::ms_stockpileBackPackMax.m_sNumMissiles;
			stockpile.m_sNumNapalms			= CStockPile::ms_stockpileBackPackMax.m_sNumNapalms;
			stockpile.m_sNumFuel				= CStockPile::ms_stockpileBackPackMax.m_sNumFuel;
			stockpile.m_sDeathWadLauncher	= 1;

			CreateCheat(&stockpile);

			// Enable death wad launcher in editor as well.
			CStockPile::ms_sEnableDeathWad	= TRUE;
			break;
			}
		case INPUT_CHEAT_17:	// DoubleBarrel and shells.
			{
			CStockPile	stockpile			= { 0, };
			stockpile.m_sNumShells			= CStockPile::ms_stockpileBackPackMax.m_sNumShells;
			stockpile.m_sDoubleBarrel		= 1;

			CreateCheat(&stockpile);

			// Enable Double Barrel in editor as well.
			CStockPile::ms_sEnableDoubleBarrel	= TRUE;
			break;
			}
		case INPUT_CHEAT_18:	// Mighty Mouse mode.
			{
			UnlockAchievement(ACHIEVEMENT_ENABLE_CHEATS);
			Flag_Achievements |= FLAG_USED_CHEATS;

			// Modify hood's scale.
			m_pRealm->m_phood->m_dScale3d -= 0.10;

			// Let the hood setup the pipeline. 
			m_pRealm->m_phood->SetupPipeline();

			break;
			}
		case INPUT_CHEAT_19:	// Shell weapons and ammo.
			{
			CStockPile	stockpile			= { 0, };
			stockpile.m_sNumShells			= CStockPile::ms_stockpileBackPackMax.m_sNumShells;
			stockpile.m_sShotGun				= 1;
			stockpile.m_sSprayCannon		= 1;

			CreateCheat(&stockpile);

			break;
			}
		case INPUT_CHEAT_20:	// Explosive weapons and ammo.
			{
			CStockPile	stockpile			= { 0, };
			stockpile.m_sNumGrenades		= CStockPile::ms_stockpileBackPackMax.m_sNumGrenades;
			stockpile.m_sNumMissiles		= CStockPile::ms_stockpileBackPackMax.m_sNumMissiles;
			stockpile.m_sNumHeatseekers	= CStockPile::ms_stockpileBackPackMax.m_sNumHeatseekers;
			stockpile.m_sMissileLauncher	= 1;

			CreateCheat(&stockpile);

			break;
			}
		case INPUT_CHEAT_21:	// Flame weapons and ammo.
			{
			CStockPile	stockpile			= { 0, };
			stockpile.m_sNumFireBombs		= CStockPile::ms_stockpileBackPackMax.m_sNumFireBombs;
			stockpile.m_sNumNapalms			= CStockPile::ms_stockpileBackPackMax.m_sNumNapalms;
			stockpile.m_sNumFuel				= CStockPile::ms_stockpileBackPackMax.m_sNumFuel;
			stockpile.m_sNapalmLauncher	= 1;
			stockpile.m_sFlameThrower		= 1;

			CreateCheat(&stockpile);

			break;
			}
		case INPUT_CHEAT_22:	// Shotgun and shells.
			{
			CStockPile	stockpile			= { 0, };
			stockpile.m_sNumShells			= CStockPile::ms_stockpileBackPackMax.m_sNumShells;
			stockpile.m_sShotGun				= 1;
			
			CreateCheat(&stockpile);

			break;
			}
		case INPUT_CHEAT_23:	// Spray cannon and shells.
			{
			CStockPile	stockpile			= { 0, };
			stockpile.m_sNumShells			= CStockPile::ms_stockpileBackPackMax.m_sNumShells;
			stockpile.m_sSprayCannon		= 1;

			CreateCheat(&stockpile);

			break;
			}
		case INPUT_CHEAT_24:	// Grenades and cocktails.
			{
			CStockPile	stockpile			= { 0, };
			stockpile.m_sNumGrenades		= CStockPile::ms_stockpileBackPackMax.m_sNumGrenades;
			stockpile.m_sNumFireBombs		= CStockPile::ms_stockpileBackPackMax.m_sNumFireBombs;

			CreateCheat(&stockpile);

			break;
			}
		case INPUT_CHEAT_25:	// Missile launcher and missiles (including heatseekers).
			{
			CStockPile	stockpile			= { 0, };
			stockpile.m_sNumMissiles		= CStockPile::ms_stockpileBackPackMax.m_sNumMissiles;
			stockpile.m_sNumHeatseekers	= CStockPile::ms_stockpileBackPackMax.m_sNumHeatseekers;
			stockpile.m_sMissileLauncher	= 1;

			CreateCheat(&stockpile);

			break;
			}
		case INPUT_CHEAT_26:	// Napalm launcher and napalm canisters.
			{
			CStockPile	stockpile			= { 0, };
			stockpile.m_sNumNapalms			= CStockPile::ms_stockpileBackPackMax.m_sNumNapalms;
			stockpile.m_sNapalmLauncher	= 1;

			CreateCheat(&stockpile);

			break;
			}
		case INPUT_CHEAT_27:	// Flame thrower and fuel canisters.
			{
			CStockPile	stockpile			= { 0, };
			stockpile.m_sNumFuel				= CStockPile::ms_stockpileBackPackMax.m_sNumFuel;
			stockpile.m_sFlameThrower		= 1;

			CreateCheat(&stockpile);

			break;
			}
		case INPUT_CHEAT_28:	// Mines.
			{
			CStockPile	stockpile			= { 0, };
			stockpile.m_sNumMines			= CStockPile::ms_stockpileBackPackMax.m_sNumMines;

			CreateCheat(&stockpile);

			break;
			}
		case INPUT_CHEAT_29:	// Unused except for SALES demo.
			{
#ifdef SALES_DEMO
			CStockPile	stockpile	= { 0, };
			stockpile.m_sHitPoints	= MAX(0, m_sOrigHitPoints - m_stockpile.m_sHitPoints);

			stockpile.m_sNumGrenades		= CStockPile::ms_stockpileBackPackMax.m_sNumGrenades;
			stockpile.m_sNumFireBombs		= CStockPile::ms_stockpileBackPackMax.m_sNumFireBombs;
			stockpile.m_sNumMissiles		= CStockPile::ms_stockpileBackPackMax.m_sNumMissiles;
			stockpile.m_sNumNapalms			= CStockPile::ms_stockpileBackPackMax.m_sNumNapalms;
			stockpile.m_sNumBullets			= CStockPile::ms_stockpileBackPackMax.m_sNumBullets;
			stockpile.m_sNumShells			= CStockPile::ms_stockpileBackPackMax.m_sNumShells;
			stockpile.m_sNumMines			= CStockPile::ms_stockpileBackPackMax.m_sNumMines;
			stockpile.m_sNumHeatseekers	= CStockPile::ms_stockpileBackPackMax.m_sNumHeatseekers;
			stockpile.m_sNumFuel				= CStockPile::ms_stockpileBackPackMax.m_sNumFuel;

			stockpile.m_sMachineGun			= 1;
			stockpile.m_sMissileLauncher	= 1;
			stockpile.m_sShotGun				= 1;
			stockpile.m_sSprayCannon		= 1;
			stockpile.m_sFlameThrower		= 1;
			stockpile.m_sNapalmLauncher	= 1;

			stockpile.m_sKevlarLayers		= CStockPile::ms_stockpileMax.m_sKevlarLayers;

			stockpile.m_sBackpack			= 1;

			CreateCheat(&stockpile);

			// Also, make invincible.
			m_bInvincible							= true;

			// Also, allow level advance.
			g_bEnableLevelAdvanceWithoutGoal	= true;
#endif
			break;
			}
		case INPUT_CHEAT_30:	// Toggle invincibility.
			{
			UnlockAchievement(ACHIEVEMENT_ENABLE_CHEATS);
			Flag_Achievements |= FLAG_USED_CHEATS;

			m_bInvincible	= !m_bInvincible;

			break;
			}
		}

	// "Twinstick" style inputs.
if (!demoCompat)
{
	if ((input & INPUT_MOVE_UP) || (input & INPUT_MOVE_DOWN) || (input & INPUT_MOVE_LEFT) || (input & INPUT_MOVE_RIGHT)
			|| (input & INPUT_FIRE_UP) || (input & INPUT_FIRE_DOWN) || (input & INPUT_FIRE_LEFT) || (input & INPUT_FIRE_RIGHT))
	{
		// Say we're using twinstick mode
		m_bUseRotTS = true;

		// Turn off the normal movement inputs if present
		input &= ~(INPUT_FORWARD | INPUT_BACKWARD | INPUT_LEFT | INPUT_RIGHT);

		// Determine movement direction
		if (bCanMove)
		{
			if ((input & INPUT_MOVE_UP) || (input & INPUT_MOVE_DOWN) || (input & INPUT_MOVE_LEFT) || (input & INPUT_MOVE_RIGHT))
			{
				// Set up acceleration
				m_dAcc = ms_dAccUser;
				m_dDrag = 0;

				// Say which direction we want to go
				if (input & INPUT_MOVE_UP)
					m_dRotTS = 90 + (input & INPUT_MOVE_LEFT ? 45 : 0) + (input & INPUT_MOVE_RIGHT ? -45 : 0);
				else if (input & INPUT_MOVE_DOWN)
					m_dRotTS = -90 + (input & INPUT_MOVE_LEFT ? -45 : 0) + (input & INPUT_MOVE_RIGHT ? 45 : 0);
				else if (input & INPUT_MOVE_LEFT)
					m_dRotTS = 180;
				else if (input & INPUT_MOVE_RIGHT)
					m_dRotTS = 0;
			}
			else
			{
				m_dAcc = 0;
				if (m_dVel > 0)
					m_dDrag = ms_dAccDrag;
				else if (m_dVel < 0)
					m_dDrag = -ms_dAccDrag;
			}
		}

		// Determine fire direction
		if (bCanFire)
		{
			if ((input & INPUT_FIRE_UP) || (input & INPUT_FIRE_DOWN) || (input & INPUT_FIRE_LEFT) || (input & INPUT_FIRE_RIGHT))
			{
				// Say we're firing our weapon
				input |= INPUT_FIRE;

				// Then point in the correct direction
				if (input & INPUT_FIRE_UP)
					m_dRot = 90 + (input & INPUT_FIRE_LEFT ? 45 : 0) + (input & INPUT_FIRE_RIGHT ? -45 : 0);
				else if (input & INPUT_FIRE_DOWN)
					m_dRot = -90 + (input & INPUT_FIRE_LEFT ? -45 : 0) + (input & INPUT_FIRE_RIGHT ? 45 : 0);
				else if (input & INPUT_FIRE_LEFT)
					m_dRot = 180;
				else if (input & INPUT_FIRE_RIGHT)
					m_dRot = 0;
			}
			else
				// otherwise, point in the direction we're going
				m_dRot = m_dRotTS;
		}
	}
	//else
		//m_bUseRotTS = false;
}
#ifdef ALLOW_TWINSTICK
if (!demoCompat)
{
	GetDudeVelocity(&m_dJoyMoveVel, &m_dJoyMoveAngle);
	if (!bCanMove)
		m_dJoyMoveVel = 0;

	m_dJoyFireAngle = 0.f;
	m_bJoyFire = (bCanFire && GetDudeFireAngle(&m_dJoyFireAngle));
	if (m_dJoyMoveVel > 0 || m_bJoyFire)
	{
		// Setup movement
		if (m_dJoyMoveVel > 0)
		{
			// Say we're using twinstick mode
			m_bUseRotTS = true;

			// Turn off the normal movement inputs if present
			input &= ~(INPUT_FORWARD | INPUT_BACKWARD | INPUT_LEFT | INPUT_RIGHT);

			// Set up acceleration
			m_dAcc = ms_dAccUser;
			m_dDrag = 0;

			// Say which direction we want to go
			m_dRotTS = m_dJoyMoveAngle;
		}
		else
		{
			// Setup drag
			m_dAcc = 0;
			if (m_dVel > 0)
				m_dDrag = ms_dAccDrag;
			else if (m_dVel < 0)
				m_dDrag = -ms_dAccDrag;
		}

		// Setup firing
		if (m_bJoyFire)
		{
			// Say we're firing our weapon
			//input |= INPUT_FIRE;

			// Then point in the correct direction
			m_dRot = m_dJoyFireAngle;
		}
		else
			// otherwise, point in the direction we're going
			m_dRot = m_dJoyMoveAngle;
	}
}
#endif
	//TRACE("TSD Acc %f MA %f AA %f Fire %i\n", m_dAcc, m_dRotTS, m_dRot, m_bJoyFire);
	//TRACE("JoyVel %f JoyAngle %f Fire %i FireAngle %f\n", m_dJoyMoveVel, m_dJoyMoveAngle, m_bJoyFire, m_dJoyFireAngle);

//#ifdef MOBILE
if (!demoCompat)
{
//#endif // MOBILE
	if (input & INPUT_WEAPON_NEXT)
	{
		input &= ~(INPUT_FIRE);
		NextWeapon();
	}

	if (input & INPUT_WEAPON_PREV)
	{
		input &= ~(INPUT_FIRE);
		PrevWeapon();
	}
//#ifdef MOBILE
}
//#endif // MOBILE

	// If a weapon key was pressed . . .
	if (input & INPUT_WEAPONS_MASK)
		{
		// If weapon was available . . .
		if (SetWeapon(wtNew) == true)
			{
			}
		}

	// See if the guy fires his weapon . . .
	// Check for general weapon fire . . .
	if (input & INPUT_FIRE)
		{
		// If not ducking . . .
		if (m_state != State_Duck)
			{
			ArmWeapon();
			}
		else
			{
			// We should get up first.
			SetState(State_Rise);
			}
		
		// Ducking is not a user option when rising for or firing.
		input	&=	~INPUT_DUCK;
		}
	else
		{
		// If currently shooting . . .
		switch (m_state)
			{
			case State_Shooting:
				SetState(State_Stand);
				break;
			case State_RunAndShoot:
				SetState(State_Run);
				break;
			case State_StrafeAndShoot:
				SetState(State_Strafe);
				break;
			}
		}
#ifdef MOBILE
if (!demoCompat)
{
	if (input & INPUT_STRAFE)
		{
		if (input & INPUT_LEFT)
			{
			*psStrafeAngle	= m_dRot + 90;
			}
		else if (input & INPUT_RIGHT)
			{
			*psStrafeAngle	= m_dRot - 90;
			}
		}

		{
		short sRotDelta	= (short)(input & INPUT_ROT_MASK);
		
		if (sRotDelta != 0)
			sRotDelta	-= 360;

		if (input & INPUT_ROT_IS_ABS)
			m_dRot	= (double) sRotDelta;
		else
			m_dRot	+= (double) sRotDelta;


		// If there was any rotation while standing . . .
		if (sRotDelta != 0 && m_state == State_Stand)
			{
			// Reset stand state (to make sure idle anim doesn't kick (or gets turned
			// off if we're already in it) ).
			SetState(State_Stand);
			}
		}
}
else
{
#endif
	if (input & INPUT_STRAFE)
		{
		if (input & INPUT_LEFT)
			{
			*psStrafeAngle	= m_dRot + 90;
			}
		else if (input & INPUT_RIGHT)
			{
			*psStrafeAngle	= m_dRot - 90;
			}
		}
	else if (input & INPUT_STRAFE_LEFT)
		*psStrafeAngle = m_dRot + 90;
	else if (input & INPUT_STRAFE_RIGHT)
		*psStrafeAngle = m_dRot - 90;
	else
		{
		short sRotDelta	= (short)(input & INPUT_ROT_MASK);
		
		if (sRotDelta != 0)
			sRotDelta	-= 360;

		// Adjust by input delta.
		m_dRot	+= (double) sRotDelta;

//printf("dRotDelta == (%f), m_dRot == (%f)\n", (float) dRotDelta, (float) m_dRot);


		// If there was any rotation while standing . . .
		if (sRotDelta != 0 && m_state == State_Stand)
			{
			// Reset stand state (to make sure idle anim doesn't kick (or gets turned
			// off if we're already in it) ).
			SetState(State_Stand);
			}
		}
#ifdef MOBILE
}
#endif
	// Set acceleration based on user input (forward has precedance over reverse)
	if (input & INPUT_FORWARD)
		{
		m_dAcc	= ms_dAccUser;
		m_dDrag	= 0;
		m_bUseRotTS = false;
		}
	else if (input & INPUT_BACKWARD)
		{
		m_dAcc	= -ms_dAccUser;
		m_dDrag	= 0;
		m_bUseRotTS = false;
		}
	else if (!demoCompat && !((input & INPUT_MOVE_UP) || (input & INPUT_MOVE_DOWN) || (input & INPUT_MOVE_LEFT) || (input & INPUT_MOVE_RIGHT) || m_dJoyMoveVel > 0))
		{
		m_dAcc	= 0;
		// If traveling forward . . .
		if (m_dVel > 0.0)
			{
			m_dDrag	= -ms_dAccDrag;
			}
		else if (m_dVel < 0.0)
			{
			m_dDrag	= ms_dAccDrag;
			}
		}

	// Limit to maximum velocity, which depends on whether "slow" key is being pressed
	if (input & INPUT_RUN)
		{
		*pdMaxForeVel	= ms_dMaxVelForeFast;
		*pdMaxBackVel	= ms_dMaxVelBackFast;
		}
	else
		{
		*pdMaxForeVel	= ms_dMaxVelFore;
		*pdMaxBackVel	= ms_dMaxVelBack;
		}

	// If jump specified . . .
	if ((input & INPUT_REVIVE) && 0) // ***TEMPORARILY DISABLED***
		{
		// If on the ground . . .
		if (m_bAboveTerrain == false)
			{
			// If moving forward of our own power. . .
			if (m_dVel > 0.0)
				{
				SetState(State_JumpForward);
				}
			else
				{
				SetState(State_Jump);
				}
			}
		}

	// If duck specified . . .
	if (input & INPUT_DUCK)
		{
		SetState(State_Duck);
		}
	else
		{
		if (m_state == State_Duck)
			{
			SetState(State_Rise);
			}
		}

	// If suicide specified . . .
	if (input & INPUT_SUICIDE)
		{
		// If we have the appropriate weapon . . .
		if (m_stockpile.m_sMachineGun)
			{
			SetState(State_Suicide);
			}
		else
			{
			// Audible user feedback.
			PlaySample(g_smidGeneralBeep, SampleMaster::UserFeedBack);
			}
		}

	// If execute specified . . .
	if (input & INPUT_EXECUTE)
		{
		// If not already executing . . .
		if (m_state != State_Execute)
			{
			// If we have the appropriate weapon . . .
			if (m_stockpile.m_sMachineGun)
				{
				// If there's someone close enough to execute . . .
				if (FindExecutee() == true)
					{
					// Do it.
					SetState(State_Execute);
					}
				}
			else
				{
				// Audible user feedback.
				PlaySample(g_smidGeneralBeep, SampleMaster::UserFeedBack);
				}
			}
		}
	}


////////////////////////////////////////////////////////////////////////////////
// Applies accelerations, velocities, reacts to terrain obstructions, etc.
////////////////////////////////////////////////////////////////////////////////
void CDude::ProcessForces(	// Returns nothing.
	long		lCurTime,		// In:  Current game time.
	double	dMaxForeVel,	// Out: Maximum forward velocity.
	double	dMaxBackVel,	// Out: Maximum backward velocity.
	short		sStrafeAngle)	// Out: Strafe angle.
	{
	double	dNewX, dNewY, dNewZ;
	
	// Calculate elapsed time in seconds.
	double dSeconds = (double)(lCurTime - m_lPrevTime) / 1000.0;

#ifdef MOBILE
if (!demoCompat)
{
	TwinStickInfo analogInfo = AndroidGetMovment();
	if (analogInfo.velocity)
	{
		UpdateVelocities(dSeconds, dMaxForeVel * analogInfo.velocity, dMaxBackVel * analogInfo.velocity);
		GetNewPositionAngle(&dNewX, &dNewY, &dNewZ, dSeconds,analogInfo.movmentAngle);
	}
	else
	{
		// Update Velocities ////////////////////////////////////////////////////////
		UpdateVelocities(dSeconds, dMaxForeVel, dMaxBackVel);

		// Get New Position /////////////////////////////////////////////////////////
		GetNewPosition(&dNewX, &dNewY, &dNewZ, dSeconds);

		//GetNewPositionAngle(&dNewX, &dNewY, &dNewZ, dSeconds,angle_for_postal);
	}
}
else
{
	// Update Velocities ////////////////////////////////////////////////////////

	UpdateVelocities(dSeconds, dMaxForeVel, dMaxBackVel);
	
	// Get New Position /////////////////////////////////////////////////////////

	GetNewPosition(&dNewX, &dNewY, &dNewZ, dSeconds);
}

#else
	// Update Velocities ////////////////////////////////////////////////////////

#if defined(ALLOW_TWINSTICK)
	if (!demoCompat && m_dJoyMoveVel != 0)
		UpdateVelocities(dSeconds, dMaxForeVel * m_dJoyMoveVel, dMaxBackVel * m_dJoyMoveVel);
	else
#endif // ALLOW_TWINSTICK
	UpdateVelocities(dSeconds, dMaxForeVel, dMaxBackVel);
	
	// Get New Position /////////////////////////////////////////////////////////
	if (!demoCompat && m_bUseRotTS)
		GetNewPositionAngle(&dNewX, &dNewY, &dNewZ, dSeconds, m_dRotTS);
	else
		GetNewPosition(&dNewX, &dNewY, &dNewZ, dSeconds);
#endif // MOBILE



	// If strafing . . .
	if (sStrafeAngle != INVALID_STRAFE_ANGLE)
		{
		// Add strafe.
		sStrafeAngle	= rspMod360(sStrafeAngle);
		double	dStrafeDistance	= (STRAFE_VEL * dSeconds);
		dNewX	+= COSQ[sStrafeAngle] * dStrafeDistance;
		dNewZ	-=	SINQ[sStrafeAngle] * dStrafeDistance;
		}

	// Validate New Position ////////////////////////////////////////////////////

	if (MakeValidPosition(&dNewX, &dNewY, &dNewZ, MaxStepUpThreshold) == true)
		{
		// Update Values /////////////////////////////////////////////////////////

		m_dX	= dNewX;
		m_dY	= dNewY;
		m_dZ	= dNewZ;

		// Map through view angle which is the angle of the trigger plane.
		// (it is created by the user in the editor parallel with the
		// screen).
		double	dTriggerY;
		m_pRealm->MapZ3DtoY2D(m_dZ, &dTriggerY);
		// Spew triggers.
		SpewTriggers(m_pRealm, GetInstanceID(), m_dX, dTriggerY);

		UpdateFirePosition();
		}
	else
		{
		// Restore Values ////////////////////////////////////////////////////////
		
		// Didn't actually move and, therefore, did not actually accelerate.  
		// Restore velocities.
//			m_dVel			-= m_dDeltaVel;
//			m_dExtHorzVel	-= m_dExtHorzDeltaVel;
		m_dExtVertVel	-= m_dExtVertDeltaVel;
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Render object
////////////////////////////////////////////////////////////////////////////////
void CDude::Render(void)
	{
	// Use user's chosen texture.
	m_panimCur->m_ptextures	= m_aptextures[m_sTextureIndex];

	// Call base class.
	CCharacter::Render();

	// Update children, if any . . .
	CFlag*	pflag	= GetNextFlag(NULL);
	while (pflag)
		{
		PositionChild(
			pflag->GetSprite(),
			((CDudeAnim3D*) m_panimCur)->m_ptransLeft->GetAtTime(m_lAnimTime),	// In:  Transform specifying position.
			NULL,
			NULL,
			NULL);

		// Update flag's position so it can correctly collision detect.
		pflag->m_dX	= m_dX;
		pflag->m_dY	= m_dY;
		pflag->m_dZ	= m_dZ;

		// For asthetics, rotate it a bit.
		pflag->m_dRot	= rspMod360(pflag->m_dRot + RAND_SWAY(10) );

		// Get next.
		pflag	= GetNextFlag(pflag);
		}

	// Get anim . . .
	CAnim3D*	panimWeapon	= NULL;	// Safety.
	switch (m_state)
		{
		case State_Suicide:
		case State_Execute:
			panimWeapon	= &(m_aanimWeapons[SemiAutomatic]);
			break;
		default:
			panimWeapon	= &(m_aanimWeapons[m_weapontypeCur]);
			break;
		}

	// If we have a visible weapon . . .
	if (panimWeapon->m_pmeshes)
		{
		// Show weapon sprite.
		m_spriteWeapon.m_sInFlags	&= ~CSprite::InHidden;

		m_spriteWeapon.m_pmesh		= panimWeapon->m_pmeshes->GetAtTime(m_lAnimTime);
		m_spriteWeapon.m_psop		= panimWeapon->m_psops->GetAtTime(m_lAnimTime);
		m_spriteWeapon.m_ptex		= panimWeapon->m_ptextures->GetAtTime(m_lAnimTime);
		m_spriteWeapon.m_psphere	= panimWeapon->m_pbounds->GetAtTime(m_lAnimTime);
		m_spriteWeapon.m_ptrans		= ((CDudeAnim3D*)m_panimCur)->m_ptransRight->GetAtTime(m_lAnimTime);
		}
	else
		{
		// Hide weapon sprite.
		m_spriteWeapon.m_sInFlags	|= CSprite::InHidden;
		}

	// If we have a backpack . . .
	if (m_stockpile.m_sBackpack)
		{
		// Show backpack sprite.
		m_spriteBackpack.m_sInFlags	&= ~CSprite::InHidden;

		ASSERT(m_animBackpack.m_pmeshes);
		m_spriteBackpack.m_pmesh	= m_animBackpack.m_pmeshes->GetAtTime(m_lAnimTime);                   
		m_spriteBackpack.m_psop		= m_animBackpack.m_psops->GetAtTime(m_lAnimTime);                     
		m_spriteBackpack.m_ptex		= m_animBackpack.m_ptextures->GetAtTime(m_lAnimTime);                 
		m_spriteBackpack.m_psphere	= m_animBackpack.m_pbounds->GetAtTime(m_lAnimTime);                   
		m_spriteBackpack.m_ptrans	= ((CDudeAnim3D*)m_panimCur)->m_ptransBack->GetAtTime(m_lAnimTime);
		}
	else
		{
		// Hide bakcpack sprite.
		m_spriteBackpack.m_sInFlags	|= CSprite::InHidden;
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Called by editor to init new object at specified position
////////////////////////////////////////////////////////////////////////////////
short CDude::EditNew(									// Returns 0 if successfull, non-zero otherwise
	short sX,												// In:  New x coord
	short sY,												// In:  New y coord
	short sZ)												// In:  New z coord
	{
	short sResult = CCharacter::EditNew(sX, sY, sZ);

	// Init dude
	sResult = Init();

	return sResult;
	}

////////////////////////////////////////////////////////////////////////////////
// Helper inline to get a GUI, set its text to the value, and recompose it.
////////////////////////////////////////////////////////////////////////////////
inline
void SetText(					// Returns nothing.
	RGuiItem*	pguiRoot,	// In:  Root GUI.
	long			lId,			// In:  ID of GUI to set text.
	long			lVal)			// In:  Value to set text to.
	{
	RGuiItem*	pgui	= pguiRoot->GetItemFromId(lId);
	if (pgui != NULL)
		{
		pgui->SetText("%ld", lVal);
		pgui->Compose(); 
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Called by editor to modify object
////////////////////////////////////////////////////////////////////////////////
short CDude::EditModify(void)				// Returns 0 if successfull, non-zero otherwise.
	{
	short	sResult	= CCharacter::EditModify();

	if (sResult == 0)
		{
		RGuiItem* pgui = RGuiItem::LoadInstantiate(FullPathVD(MODIFY_GUI_FILE) );
		if (pgui)
			{
			sResult = m_stockpile.UserEdit(pgui);
			if (sResult == 0)
				{
				// Get values from dialog:
				
				// Currently none.

				// Success.
				}
			else
				{
				TRACE("EditModify():  m_stockpile.UserEdit() failed.\n");
				}

			delete pgui;
			}
		else
			{
			TRACE("EditModify(): Failed to open GUI \"%s\".\n", MODIFY_GUI_FILE);
			sResult	= -2;
			}
		}
	else
		{
		TRACE("EditModify(): Base class EditModify() failed.\n");
		sResult	= -1;
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Init dude
////////////////////////////////////////////////////////////////////////////////
short CDude::Init(void)									// Returns 0 if successfull, non-zero otherwise
	{
	short sResult = 0;

	m_lPrevTime			= m_pRealm->m_time.GetGameTime();
	m_lNextBulletTime	= m_lPrevTime;
	
	// Get resources
	sResult = GetResources();

	// Prepare shadow (get resources and setup sprite).
	sResult	|= PrepareShadow();

	// Setup stand state.
	SetState(State_Stand);

	m_smash.m_bits		= CSmash::Good | CSmash::Character;
	m_smash.m_pThing	= this;

	// No special flags.
	m_sprite.m_sInFlags = 0;

	// Targeting flag initially starts out as Hidden
	m_TargetSprite.m_sInFlags		= CSprite::InHidden;
	// Set alpha blendage.
	m_TargetSprite.m_sAlphaLevel	= TARGET_ALPHA_LEVEL;
	// This can be changed with the toggle, and should probably be 
	// set initially by the preferences file.
	m_bTargetingHelpEnabled = false;

	// Setup weapon sprite.
	m_spriteWeapon.m_sInFlags	= 0;
	m_spriteWeapon.m_pthing		= this;
	m_sprite.AddChild(&m_spriteWeapon);

	// Setup backpack sprite.
	m_spriteBackpack.m_sInFlags	= 0;
	m_spriteBackpack.m_pthing		= this;
	m_sprite.AddChild(&m_spriteBackpack);

	// Setup our crawler.
	m_crawler.m_prealm			= m_pRealm;
	m_crawler.m_sVertTolerance	= MaxStepUpThreshold;
	m_crawler.Setup(
		sizeof(ms_anubs)/sizeof(ms_anubs[0]), 
		ms_anubs, 
		MAX_CRAWLER_PUSH_X, 
		MAX_CRAWLER_PUSH_Z);

	// Store initial hit points.
	m_sOrigHitPoints	= m_stockpile.m_sHitPoints;

	// Start with a weapon.
	m_weapontypeCur	= NoWeapon;
	NextWeapon();

	// Cache gun sound effects since the gun has no preload
	CacheSample(g_smidRicochet1);
	CacheSample(g_smidRicochet2);
	CacheSample(g_smidBulletFire);
	CacheSample(g_smidShotgun);
	// Cache your own sound effects
	CacheSample(g_smidDyingYell);
	CacheSample(g_smidBodyImpact2);
	CacheSample(g_smidOutOfBullets);
	CacheSample(g_smidBulletIntoVest);
	CacheSample(g_smidBlownupYell);
	CacheSample(g_smidBurningMainGuy);
	CacheSample(g_smidEmptyWeapon);
	CacheSample(g_smidLoadedWeapon);
	CacheSample(g_smidStep);
	CacheSample(g_smidPickedUpWeapon);

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Kill dude
////////////////////////////////////////////////////////////////////////////////
void CDude::Kill(void)
	{
	// Remove the target sprite, if there.
	m_pRealm->m_scene.RemoveSprite(&m_TargetSprite);

	// Free resources
	FreeResources();
	}

////////////////////////////////////////////////////////////////////////////////
// Get all required resources
////////////////////////////////////////////////////////////////////////////////
short CDude::GetResources(void)						// Returns 0 if successfull, non-zero otherwise
	{
	short sResult = 0;

	//											Anim base name					Rigid name		Event name		Loop flags
	//											===================			=============	===========		===========
	sResult	= m_animRun.Get			("3d/main_runnogun",			NULL,				"mainevent",	RChannel_LoopAtStart | RChannel_LoopAtEnd);
	sResult	|= m_animThrow.Get		("3d/main_grenade",			"maingrenade",	"mainevent",	0);
	sResult	|= m_animStand.Get		("3d/main_bobbing",			NULL,				NULL,				RChannel_LoopAtStart | RChannel_LoopAtEnd);
	sResult	|= m_animDie.Get			("3d/main_die",				NULL,				"mainevent",	0);
	sResult	|= m_animShoot.Get		("3d/main_shoot",				"guntip",		NULL,				RChannel_LoopAtStart | RChannel_LoopAtEnd);
	sResult	|= m_animRunShoot.Get	("3d/main_runshoot",			"guntip",		"mainevent",	RChannel_LoopAtStart | RChannel_LoopAtEnd);
	sResult	|= m_animDamage.Get		("3d/main_multi",				NULL,				NULL,				RChannel_LoopAtStart | RChannel_LoopAtEnd);
	sResult	|= m_animBurning.Get		("3d/main_onfire",			NULL,				"mainevent",	RChannel_LoopAtStart | RChannel_LoopAtEnd);
	sResult	|=	m_animStrafe.Get		("3d/main_strafe",			NULL,				"mainevent",	RChannel_LoopAtStart | RChannel_LoopAtEnd);
	sResult	|= m_animStrafeShoot.Get("3d/main_strafe",			"guntip",		"mainevent",	RChannel_LoopAtStart | RChannel_LoopAtEnd);
	sResult	|= m_animSuicide.Get		("3d/main_suicide",			"bhead",			"mainevent",	0);
	sResult	|= m_animLaunch.Get		("3d/main_missile",			"mainmissile",	"mainevent",	0);
	sResult	|= m_animBlownUp.Get		("3d/main_blownup",			NULL,				NULL,				0);
	sResult	|= m_animGetUp.Get		("3d/main_getup",				NULL,				NULL,				0);
	sResult	|= m_animDuck.Get			("3d/main_duck",				NULL,				NULL,				0);
	sResult	|= m_animRise.Get			("3d/main_rise",				NULL,				NULL,				0);
	sResult	|= m_animExecute.Get		("3d/main_execute",			"guntip",		NULL,				0);
	sResult	|= m_animPickPut.Get		("3d/main_pickput",			"lfhand",		"mainevent",	0);
	sResult	|= m_animIdle.Get			("3d/main_idle",				NULL,				NULL,				0);

	// Get the different textures this dude could have.
	short i;
	char	szResName[RSP_MAX_PATH];
	for (i = 0; i < MaxTextures && sResult == 0; i++)
		{
		sprintf(szResName, "3d/main_color%d.tex", i);
		sResult	|= rspGetResource(&g_resmgrGame, szResName, &(m_aptextures[i]) );
		}

	// Get the different weapons this dude could use.
	for (i = NoWeapon; i < NumWeaponTypes; i++)
		{
		if (ms_awdWeapons[i].pszWeaponResName)
			{
			sResult	|= m_aanimWeapons[i].Get(ms_awdWeapons[i].pszWeaponResName, NULL, NULL, NULL, RChannel_LoopAtStart | RChannel_LoopAtEnd);
			}
		}

	// Get the backpack.
	sResult	|= m_animBackpack.Get(BACKPACK_RES_NAME, NULL, NULL, NULL, RChannel_LoopAtStart | RChannel_LoopAtEnd);

	// Get the targeting sprite
	sResult |= rspGetResource(&g_resmgrGame, m_pRealm->Make2dResPath(TARGETING_FILE), &(m_TargetSprite.m_pImage), RFile::LittleEndian);
	
	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Free all resources
////////////////////////////////////////////////////////////////////////////////
void CDude::FreeResources(void)
	{
	// Release resources for animations.
	m_animRun.Release();
	m_animThrow.Release();
	m_animStand.Release();
	m_animDie.Release();
	m_animShoot.Release();
	m_animRunShoot.Release();
	m_animDamage.Release();
	m_animBurning.Release();
	m_animStrafe.Release();
	m_animStrafeShoot.Release();
	m_animSuicide.Release();
	m_animLaunch.Release();
	m_animBlownUp.Release();
	m_animGetUp.Release();
	m_animDuck.Release();
	m_animRise.Release();
	m_animExecute.Release();
	m_animPickPut.Release();
	m_animIdle.Release();

	// Release the different textures.
	short i;
	for (i = 0; i < MaxTextures; i++)
		{
		rspReleaseResource(&g_resmgrGame, &(m_aptextures[i]) );
		}

	// Release the different weapons this dude could use.
	for (i = NoWeapon; i < NumWeaponTypes; i++)
		{
		if (m_aanimWeapons[i].m_pmeshes)
			{
			m_aanimWeapons[i].Release();
			}
		}

	// Release the backpack.
	m_animBackpack.Release();

	// Release the targeting image
	rspReleaseResource(&g_resmgrGame, &(m_TargetSprite.m_pImage));

	}

////////////////////////////////////////////////////////////////////////////////
// Sets a new state based on supplied state enum.  Will set animation ptr
// to proper animation for state and reset animation timer.
////////////////////////////////////////////////////////////////////////////////
bool CDude::SetState(	// Returns true if new state realized, false otherwise.
	State	state)			// New state.
	{
	bool	bRealizeNewState	= true;		// Assume we can enter state.
	State	stateOld				= m_state;	// Remember current state.

	// If persistent state specified . . .
	if (state == State_Persistent)
		{
		// Use last persistent state.
		state	= m_statePersistent;
		}

	// See if the state can be realized.
	switch (m_state)
		{
		case State_Idle:
			// Any new state is okay.
			break;
		case State_Stand:
			// Any new state is okay.
			break;
		case State_Run:
			// Any new state is okay.
			break;
		case State_ThrowDone:
			break;
		case State_Throw:
		case State_ThrowRelease:
		case State_ThrowFinish:
			switch (state)
				{
				case State_ThrowRelease:
				case State_ThrowFinish:
				case State_ThrowDone:
				case State_Die:
				case State_Shot:
				case State_BlownUp:
				case State_Burning:
					break;
				default:
					bRealizeNewState	= false;
					break;
				}
			break;
		case State_Launch:
		case State_LaunchRelease:
		case State_LaunchFinish:
			switch (state)
				{
				case State_Die:
				case State_Shot:
				case State_BlownUp:
				case State_Burning:
				case State_LaunchRelease:
				case State_LaunchFinish:
				case State_LaunchDone:
					break;
				default:
					bRealizeNewState	= false;
					break;
				}
			break;
		case State_LaunchDone:
			break;
		case State_Die:
			switch (state)
				{
				case State_Dead:
				case State_BlownUp:
					break;
				default:
					bRealizeNewState	= false;
					break;
				}
			break;
		case State_Dead:
			switch (state)
				{
				case State_Die:
				case State_Shot:
				case State_Burning:
					bRealizeNewState	= false;
					break;
				case State_BlownUp:
					break;
				default:
					break;
				}
			break;
		case State_Shooting:
			switch (state)
				{
				case State_Duck:
					bRealizeNewState	= false;
					break;
				}
			break;
		case State_RunAndShoot:
			// Can go anywhere from here.
			break;
		case State_Shot:
			switch (state)
				{
				case State_Shot:
					bRealizeNewState	= false;
					break;
				case State_BlownUp:
				case State_Die:
					break;
				default:
					// Check for minimum duration since last shot time . . .
					if (m_pRealm->m_time.GetGameTime() < m_lLastShotTime + MIN_SHOT_DURATION)
						{
						bRealizeNewState = false;
						}
					break;
				}
			break;
		case State_BlownUp:
			switch (state)
				{
				case State_BlownUp:
					break;
				default:
					// Check for end of blown up state . . .
					if (m_lAnimTime < m_panimCur->m_psops->TotalTime())
						{
						bRealizeNewState	= false;
						}
					break;
				}
			break;
		case State_Burning:
			switch (state)
				{
				case State_Shot:
				case State_BlownUp:
					// Ok, but come back to this state.
					m_statePersistent	= m_state;
					break;
				case State_Die:
				case State_Stand:
					// Don't come back.
					m_statePersistent	= State_Stand;
					break;
				default:
					bRealizeNewState	= false;
					break;
				}
			break;
		case State_Suicide:
			switch (state)
				{
				case State_Dead:
					break;
				default:
					// Cannot leave this state except to die.
					bRealizeNewState	= false;
					break;
				}
			break;
		case State_GetUp:
			// Can only switch states due to violence or anim done.
			switch (state)
				{
				case State_Die:
				case State_Shot:
				case State_BlownUp:
				case State_Burning:
					break;
				default:
					// Check for end of get up state . . .
					if (m_lAnimTime < m_panimCur->m_psops->TotalTime())
						{
						bRealizeNewState	= false;
						}
					break;
				}
			break;
		case State_Duck:
			// Can go to any other state.
			break;
		case State_Rise:
			// Can go to any other state.
			break;
		case State_Jump:
		case State_JumpForward:
			// Only landing and falling?...damage? blown up? Not sure yet.
			break;
		case State_Fall:
			break;
		case State_Land:
			break;
		case State_LandForward:
			break;
		case State_Execute:
			break;
		case State_PutDown:
			break;
		case State_ObjectReleased:
			break;
		case State_PickUp:
			break;
		}

	// If new state realized  . . .
	if (bRealizeNewState == true)
		{
		// Clean up old state.
		switch (stateOld)
			{
			case State_Idle:	// No cleaning necessary.
			case State_Stand:
			case State_Die:
			case State_Run:
			case State_Shot:
			case State_Suicide:
			case State_GetUp:
				break;
			case State_Duck:
				// Clear the ducking bit
				m_smash.m_bits &= ~CSmash::Ducking;
				break;
			case State_Dead:
				break;
			case State_Burning:
				// If not coming back to this state . . .
				if (stateOld != m_statePersistent)
					{
					// If the new state is die . . .
					if (state == State_Die)
						{
						// Keep the fire.
						}
					else
						{
						// If there's a fire burning . . .
						CThing*	pthingFire;
						if (m_pRealm->m_idbank.GetThingByID(&pthingFire, m_u16IdFire) == 0)
							{
							// Send it a delete message.
							GameMessage	msg;
							msg.msg_ObjectDelete.eType		= typeObjectDelete;
							msg.msg_ObjectDelete.sPriority	= 0;
							SendThingMessage(&msg, pthingFire);
							}
						}
					}
				break;
			case State_PickUp:
				{
				// If there's a powerup that we haven't let go of . . .
				CPowerUp*	ppowerup;
				if (m_pRealm->m_idbank.GetThingByID((CThing**)&ppowerup, m_u16IdChild) == 0)
					{
					ppowerup->Drop(m_dX, m_dY, m_dZ);
					}
				break;
				}
			case State_Throw:
			case State_ThrowDone:
			case State_ThrowRelease:
			case State_ThrowFinish:
			case State_PutDown:
				{
				// If there's a weapon that we haven't let go of . . .
				CWeapon*	pweapon;
				if (m_pRealm->m_idbank.GetThingByID((CThing**)&pweapon, m_u16IdWeapon) == 0 && state != State_ThrowRelease)
					{
					// It should drop like a rock.
					pweapon->m_dHorizVel	= (GetRand() % (short)CGrenade::ms_dThrowHorizVel);	// NOTE:   ****USING RAND()****
					pweapon->m_dRot	= GetRand() % 360;
					ShootWeapon();
					// Delete it!
					GameMessage msg;
					msg.msg_ObjectDelete.eType = typeObjectDelete;
					msg.msg_ObjectDelete.sPriority = 0;
					SendThingMessage(&msg, pweapon);
					}
				break;
				}
			case State_Launch:
			case State_LaunchDone:
			case State_LaunchRelease:
			case State_LaunchFinish:
			case State_Shooting:
			case State_RunAndShoot:
			case State_StrafeAndShoot:
				{
				// Abort weapon launch.
				// If there's a weapon that we haven't launched . . .
				CWeapon*	pweapon;
				if (m_pRealm->m_idbank.GetThingByID((CThing**)&pweapon, m_u16IdWeapon) == 0 && state != State_LaunchRelease)
					{
					// Done with it.
					ShootWeapon();
					// Delete it!
					GameMessage msg;
					msg.msg_ObjectDelete.eType = typeObjectDelete;
					msg.msg_ObjectDelete.sPriority = 0;
					SendThingMessage(&msg, pweapon);
					}

				break;
				}
			case State_BlownUp:
				// If dead . . .
				if (m_bDead == true)
					{
					// Simon says, "die."
					state	= State_Dead;
					}
				break;
			case State_Execute:
				m_idVictim	= CIdBank::IdNil;
				break;
			}

		// Setup new state.
		m_state	= state;
		switch (state)
			{
			case State_Idle:
				m_panimCur	= NULL;
				// Make sure we're not in the render list.
				m_pRealm->m_scene.RemoveSprite(&m_sprite);
				break;
			case State_Stand:
				m_statePersistent			= State_Stand;
				m_panimCur					= &m_animStand;
				m_lAnimTime					= 0;
				m_lAnimPrevUpdateTime	= m_pRealm->m_time.GetGameTime();
				m_lNextIdleTime			= IDLE_ANIM_TIMEOUT;
				break;
			case State_Run:
				if (stateOld != State_Run)
					{
					m_panimCur			= &m_animRun;
					m_lAnimTime			= 0;
					m_lAnimPrevUpdateTime	= m_pRealm->m_time.GetGameTime();
					
					m_u8LastEvent		= 0;
					}
				break;
			case State_Throw:
				m_panimCur			= &m_animThrow;
				m_lAnimTime			= 0;
				m_lAnimPrevUpdateTime	= m_pRealm->m_time.GetGameTime();
				break;
			case State_ThrowDone:
				// This state is transitional only.
				break;
			case State_ThrowRelease:
				// This state marks the release of the thrown object.
				break;
			case State_ThrowFinish:
				// During this state the throw animation is played to finish.
				// The thrown item has been released already.
				break;
			case State_Die:
				m_panimCur			= &m_animDie;
				m_lAnimTime			= 0;
				m_lAnimPrevUpdateTime	= m_pRealm->m_time.GetGameTime();
				// Ahhhhhhhhhh....
				PlaySample(g_smidDyingYell, SampleMaster::Voices);

				m_bGenericEvent1	= false;

				// Make sure he ends up dead.
				m_bDead	= true;

				break;
			case State_Dead:
				{
				// If not already dead . . .
				if (m_bDead == false)
					{
					// Note deadness.
					m_bDead	= true;
					}

				// Add in our Dead smash bit.
				m_smash.m_bits	|= CSmash::Dead;

				// If in multiplayer . . .
				if (m_pRealm->m_flags.bMultiplayer == true)
					{
					// Drop powerup.
					// Create powerup . . .
					CPowerUp*	ppowerup	= DropPowerUp(&m_stockpile, true);
					if (ppowerup)
						{
						// Clear my stockpile.
						m_stockpile.Zero();
						// Toss it, baby.
						TossPowerUp(ppowerup, 30);
						}
					}

				break;
				}
			case State_Shooting:
				// If not already in this state . . .
				if (stateOld != State_Shooting)
					{
					m_panimCur			= &m_animShoot;
					m_lAnimTime			= 0;
					m_lAnimPrevUpdateTime	= m_pRealm->m_time.GetGameTime();
					}
				break;
			case State_RunAndShoot:
				// If not already in this state . . .
				if (stateOld != State_RunAndShoot)
					{
					m_panimCur			= &m_animRunShoot;
					m_lAnimTime			= 0;
					m_lAnimPrevUpdateTime	= m_pRealm->m_time.GetGameTime();
					m_u8LastEvent		= 0;
					}
				break;
			case State_Shot:
				m_panimCur			= &m_animDamage;
				m_lAnimTime			= 0;
				m_lAnimPrevUpdateTime	= m_pRealm->m_time.GetGameTime();
				break;
			case State_BlownUp:
				m_panimCur					= &m_animBlownUp;
				m_lAnimTime					= 0;
				m_lAnimPrevUpdateTime	= m_pRealm->m_time.GetGameTime();
				
				m_bGenericEvent1			= false;
				break;
			case State_Burning:
				// If not already in this state . . .
				if (stateOld != State_Burning)
					{
					m_panimCur			= &m_animBurning;
					m_lAnimTime			= 0;
					m_lAnimPrevUpdateTime	= m_pRealm->m_time.GetGameTime();
				
					m_u8LastEvent		= 0;
					}
				break;
			case State_Strafe:
				// If not already in this state . . .
				if (stateOld != State_Strafe)
					{
					m_panimCur			= &m_animStrafe;
					m_lAnimTime			= 0;
					m_lAnimPrevUpdateTime	= m_pRealm->m_time.GetGameTime();
				
					m_u8LastEvent		= 0;
					}
				break;
			case State_StrafeAndShoot:
				// If not already in this state . . .
				if (stateOld != State_StrafeAndShoot)
					{
					m_panimCur			= &m_animStrafeShoot;
					m_lAnimTime			= 0;
					m_lAnimPrevUpdateTime	= m_pRealm->m_time.GetGameTime();
				
					m_u8LastEvent		= 0;
					}
				break;
			case State_Suicide:
				{
				if (StatsAreAllowed) Stat_Suicides++;
				UnlockAchievement(ACHIEVEMENT_COMMIT_SUICIDE);

				m_panimCur					= &m_animSuicide;
				m_lAnimTime					= 0;
				m_lAnimPrevUpdateTime	= m_pRealm->m_time.GetGameTime();
				m_bBrainSplatted			= false;
				m_bGenericEvent1			= false;
				m_dVel						= 0.0;

				// Let the demon know.
				GameMessage msg;
				msg.msg_Suicide.eType = typeSuicide;
				msg.msg_Suicide.sPriority = 0;
				CThing* pDemon = m_pRealm->m_aclassHeads[CThing::CDemonID].GetNext();
				if (pDemon)
					SendThingMessage(&msg, pDemon);				

				break;
				}
			case State_Launch:
				m_panimCur					= &m_animLaunch;
				m_lAnimTime					= 0;
				m_lAnimPrevUpdateTime	= m_pRealm->m_time.GetGameTime();
				break;
			case State_LaunchRelease:
				// This state marks the release of the launched object.
				break;
			case State_LaunchFinish:
				// During this state the launch animation is played to finish.
				// The launched item has been released already.
				break;
			case State_LaunchDone:
				// This state is transitional only.
				break;
			case State_GetUp:
				m_panimCur					= &m_animGetUp;
				m_lAnimTime					= 0;
				m_lAnimPrevUpdateTime	= m_pRealm->m_time.GetGameTime();
				break;
			case State_Duck:
				// Set the ducking bits so missiles won't hit him.
				m_smash.m_bits |= CSmash::Ducking;
				if (stateOld != State_Duck)
					{
					m_panimCur					= &m_animDuck;
					m_lAnimTime					= 0;
					m_lAnimPrevUpdateTime	= m_pRealm->m_time.GetGameTime();
					}
				break;
			case State_Rise:
				// If not already in this state . . .
				if (stateOld != State_Rise)
					{
					// If we were just in the duck state . . .
					if (stateOld == State_Duck)
						{
						// We want to only rise based on how ducked we are.
						// If we duck at the same rate we rise, there's a relation
						// between the times of the animations.
						// If not past the end of the duck . . .
						if (m_lAnimTime < m_animDuck.m_psops->TotalTime())
							{
							ASSERT(m_animDuck.m_psops->TotalTime() != 0);

							// Get the ratio between the two animations total times.
							float	fRatio	= (float)m_animRise.m_psops->TotalTime() 
												/ (float)m_animDuck.m_psops->TotalTime();
							// Invert the time in the new animations time base.
							m_lAnimTime				= m_animRise.m_psops->TotalTime() - m_lAnimTime * fRatio;
							}
						else
							{
							m_lAnimTime	= 0;
							}
						}
					else
						{
						m_lAnimTime	= 0;
						}

					m_panimCur					= &m_animRise;
					m_lAnimPrevUpdateTime	= m_pRealm->m_time.GetGameTime();
					}
				break;
			case State_Jump:
				if (stateOld != State_Jump)
					{
//					m_panimCur	= &m_animJump;
					ASSERT(0);	// No longer a valid state.
					m_lAnimTime	= 0;
					m_lAnimPrevUpdateTime	= m_pRealm->m_time.GetGameTime();
					// Clear trigger.
					m_bJumpVerticalTrigger	= false;
					}
				break;
			case State_JumpForward:
				if (stateOld != State_JumpForward)
					{
//					m_panimCur	= &m_animJumpForward;
					ASSERT(0);	// No longer a valid state.
					m_lAnimTime	= 0;
					m_lAnimPrevUpdateTime	= m_pRealm->m_time.GetGameTime();
					// Clear trigger.
					m_bJumpVerticalTrigger	= false;
					}
				break;
			case State_Execute:
				if (stateOld != State_Execute)
					{
					m_panimCur	= &m_animExecute;
					m_lAnimTime	= 0;
					m_lAnimPrevUpdateTime	= m_pRealm->m_time.GetGameTime();
					m_bGenericEvent1	= false;
					PlaySample(g_smidExecution, SampleMaster::UserFeedBack);
					}
				break;
			case State_PutDown:
				if (stateOld != State_PutDown)
					{
					m_panimCur	= &m_animPickPut;
					m_lAnimTime	= 0;
					m_lAnimPrevUpdateTime	= m_pRealm->m_time.GetGameTime();
					}
				break;
			case State_ObjectReleased:
				// Use current anim settings.
				break;
			case State_PickUp:
				if (stateOld != State_PickUp)
					{
					m_panimCur	= &m_animPickPut;
					m_lAnimTime	= 0;
					m_lAnimPrevUpdateTime	= m_pRealm->m_time.GetGameTime();
					m_bGenericEvent1			= false;
					}
				break;
			}
		}

	return bRealizeNewState;
	}

////////////////////////////////////////////////////////////////////////////////
// Gets info on specified weapon.
////////////////////////////////////////////////////////////////////////////////
void CDude::GetWeaponInfo(		// Returns nothing.
	WeaponType		weapon,		// In:  Weapon type to query.
	ClassIDType*	pidWeapon,	// Out: CThing class ID of weapon.
	short**			ppsNum)		// Out: Ptr to the weapon's counter.
	{
	static short sSafetyNum	= 0;
	short	*psNumLeft	= &sSafetyNum;
	// Switch on weapon type . . .
	switch (weapon)
		{
		case NoWeapon:
			*pidWeapon	= 0;
			*ppsNum		= &sSafetyNum;
			break;
		case Grenade:
			*pidWeapon	= CGrenadeID;
			*ppsNum		= &m_stockpile.m_sNumGrenades;
			break;
		case FireBomb:
			*pidWeapon	= CFirebombID;
			*ppsNum		= &m_stockpile.m_sNumFireBombs;
			break;
		case Rocket:
			*pidWeapon	= CRocketID;
			*ppsNum		= &m_stockpile.m_sNumMissiles;
			break;
		case Napalm:
			*pidWeapon	= CNapalmID;
			*ppsNum		= &m_stockpile.m_sNumNapalms;
			break;
		case SemiAutomatic:
			*pidWeapon	= CMachineGunID;
			*ppsNum		= &m_stockpile.m_sNumBullets;
			break;
		case ShotGun:
			*pidWeapon	= CShotGunID;
			*ppsNum		= &m_stockpile.m_sNumShells;
			break;
		case FlameThrower:
			*pidWeapon	= CFirestreamID; //CFireballID;
			*ppsNum		= &m_stockpile.m_sNumFuel;
			break;
		case ProximityMine:
			*pidWeapon	= CProximityMineID;
			*ppsNum		= &m_stockpile.m_sNumMines;
			break;
		case TimedMine:
			*pidWeapon	= CTimedMineID;
			*ppsNum		= &m_stockpile.m_sNumMines;
			break;
		case RemoteMine:
			*pidWeapon	= CRemoteControlMineID;
			*ppsNum		= &m_stockpile.m_sNumMines;
			break;
		case BouncingBettyMine:
			*pidWeapon	= CBouncingBettyMineID;
			*ppsNum		= &m_stockpile.m_sNumMines;
			break;
		case Heatseeker:
			*pidWeapon	= CHeatseekerID;
			*ppsNum		= &m_stockpile.m_sNumHeatseekers;
			break;
		case SprayCannon:
			*pidWeapon	= CAssaultWeaponID;
			*ppsNum		= &m_stockpile.m_sNumShells;
			break;
		case DeathWad:
			*pidWeapon	= CDeathWadID;
			*ppsNum		= &m_stockpile.m_sNumMissiles;
			break;
		case DoubleBarrel:
			*pidWeapon	= CDoubleBarrelID;
			*ppsNum		= &m_stockpile.m_sNumShells;
			break;
		default:
			TRACE("GetWeaponInfo():  Query on invalid weapon (%d).\n", (short)weapon);
			break;
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Fire specified weapon type.
////////////////////////////////////////////////////////////////////////////////
void CDude::ArmWeapon(							// Returns nothing.
	WeaponType weapon /*= CurrentWeapon*/)	// In:  Weapon to fire.
	{
	// If firing not in progress . . .
	if (m_u16IdWeapon == CIdBank::IdNil)
		{
		// If no specific weapon . . .
		if (weapon == CurrentWeapon)
			{
			weapon	= m_weapontypeCur;
			}

		// Get current weapon and stockpile.
		short*	psNumLeft;
		GetWeaponInfo(weapon, &m_eWeaponType, &psNumLeft);

		ULONG weaponFlag = 0;
		switch (weapon)
			{
			case Grenade: weaponFlag = FLAG_USED_GRENADE; break;
			case FireBomb: weaponFlag = FLAG_USED_MOLOTOV; break;
			case Rocket: weaponFlag = FLAG_USED_ROCKET; break;
			case Napalm: weaponFlag = FLAG_USED_NAPALM; break;
			case SemiAutomatic: weaponFlag = FLAG_USED_M16; break;
			case ShotGun: weaponFlag = FLAG_USED_SHOTGUN; break;
			case FlameThrower: weaponFlag = FLAG_USED_FLAMETHROWER; break;
			case ProximityMine: weaponFlag = FLAG_USED_PROXIMITY_MINE; break;
			case TimedMine: weaponFlag = FLAG_USED_TIMED_MINE; break;
			case RemoteMine: weaponFlag = FLAG_USED_REMOTE_MINE; break;
			case BouncingBettyMine: weaponFlag = FLAG_USED_BETTY_MINE; break;
			case Heatseeker: weaponFlag = FLAG_USED_HEATSEEKER; break;
			case SprayCannon: weaponFlag = FLAG_USED_SPRAY_CANNON; break;
			case DeathWad: weaponFlag = FLAG_USED_DEATHWAD; break;
			case DoubleBarrel: weaponFlag = FLAG_USED_DBL_SHOTGUN; break;
			}

		State	stateShoot	= State_Throw;
		switch (weapon)
			{
			case FlameThrower:
			case SemiAutomatic:
			case SprayCannon:
				if ((m_dVel >= MIN_RUN_VEL || m_dVel <= -MIN_RUN_VEL || m_dAcc != 0.0))
					{
					stateShoot	= State_RunAndShoot;
					}
				else
					{
					switch (m_state)
						{
						case State_Strafe:
						case State_StrafeAndShoot:
							stateShoot	= State_StrafeAndShoot;
							break;
						case State_Execute:
							stateShoot	= State_Execute;
							break;
						default:
							stateShoot	= State_Shooting;
							break;
						}
					}
				break;
			case ShotGun:
			case DoubleBarrel:
				stateShoot	= State_Launch;
				break;
			case Grenade:
			case FireBomb:
				stateShoot	= State_Throw;
				break;
			case Napalm:
			case Rocket:
			case Heatseeker:
			case DeathWad:
				stateShoot	= State_Launch;
				break;
			case ProximityMine:
			case TimedMine:
			case RemoteMine:
			case BouncingBettyMine:
				stateShoot	= State_PutDown;
				break;
			}

		// If we have any of this weapon . . .
		if (*psNumLeft >= ms_awdWeapons[weapon].sMinAmmoRequired)
			{
			// Enter our weapon launch state.
			if (SetState(stateShoot) == true)
				{
				if (GetInputMode() != INPUT_MODE_PLAYBACK)  // don't let demo mode set these flags, or the endgame will ruin achievements.
					{
					Flag_Achievements |= weaponFlag;
					if ((Flag_Achievements & FLAG_MASK_WEAPONS) == FLAG_MASK_WEAPONS)
						UnlockAchievement(ACHIEVEMENT_USE_EVERY_WEAPON);
					}

				// Remember the type of ammo we're shooting.
				m_weaponShooting	= weapon;

				CWeapon*	pweapon	= PrepareWeapon();
				if (pweapon != NULL)
					{
					GameMessage msg;
					msg.msg_WeaponFire.eType = typeWeaponFire;
					msg.msg_WeaponFire.sPriority = 0;
					msg.msg_WeaponFire.sWeapon = (short) weapon;
					CThing* pDemon = m_pRealm->m_aclassHeads[CThing::CDemonID].GetNext();
					if (pDemon)
						SendThingMessage(&msg, pDemon);				
					}
				}
			else
				{
				// Note that we were unable to arm a weapon.
//				m_weaponShooting	= NoWeapon;
				}
			}
		else
			{
			// Switch to the old faithful.
			SetWeapon(SemiAutomatic);
			// Note that we were unable to arm a weapon.
			m_weaponShooting	= NoWeapon;
			}
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Shoot current weapon.
// This should be done when the character releases the weapon it's
// shooting.
////////////////////////////////////////////////////////////////////////////////
CWeapon* CDude::ShootWeapon(void)		// Returns the weapoin ptr or NULL.
	{
	return ShootWeapon(
		COLLISION_BITS_INCLUDE,
		COLLISION_BITS_DONTCARE,
		COLLISION_BITS_EXCLUDE);
	}

////////////////////////////////////////////////////////////////////////////////
// Shoot current weapon.
// This should be done when the character releases the weapon it's
// shooting.
// (virtual).
////////////////////////////////////////////////////////////////////////////////
CWeapon* CDude::ShootWeapon(					// Returns the weapon ptr or NULL.
	CSmash::Bits bitsInclude /*= ms_u32CollideBitsInclude*/,
	CSmash::Bits bitsDontcare /*= ms_u32CollideBitsDontcare*/,
	CSmash::Bits bitsExclude /*= ms_u32CollideBitsExclude*/)
	{
	bool	bShootWeapon	= true;	// Assume we should shoot the weapon.
	CWeapon*	pweapon		= NULL;	// Assume nothing.

	// If the weapon is in an invalid position . . .
	if (ValidateWeaponPosition() == false)
		{
		bShootWeapon	= false;
		}

	if (m_weaponShooting != NoWeapon && bShootWeapon == true)
		{
		// Get the weapon info.
		ClassIDType	idWeapon;
		short*	psNumLeft;
		GetWeaponInfo(m_weaponShooting, &idWeapon, &psNumLeft);

//		ASSERT(idWeapon == m_eWeaponType);


		switch (m_eWeaponType)
			{
			case CFirestreamID: //CFireballID:
				if (m_u16IdWeapon != CIdBank::IdNil)
					{
					}
				else
					{
					bShootWeapon	= false;
					}
				break;
			case CShotGunID:
				if (m_stockpile.m_sNumShells > 0)
					{
					}
				else
					{
					// Don't fire.
					bShootWeapon	= false;
					}
				break;
			case CDoubleBarrelID:
				if (m_stockpile.m_sNumShells > 1)
					{
					// Subtract one now (another will be taken soon -- cheezy... I know).
					m_stockpile.m_sNumShells--;
					}
				else
					{
					// Don't fire.
					bShootWeapon	= false;
					}
				break;
			case CAssaultWeaponID:
				if (m_stockpile.m_sNumShells > 0)
					{
					// Note time of next fire.
					m_lNextBulletTime	= m_pRealm->m_time.GetGameTime() + MS_BETWEEN_SPRAYS;
					}
				else
					{
					// Note time of next fire.
					m_lNextBulletTime	= m_pRealm->m_time.GetGameTime() + MS_BETWEEN_SPRAYS * 3;
					// Feedback.
					PlaySample(g_smidOutOfBullets, SampleMaster::Weapon);
					// Don't fire.
					bShootWeapon	= false;
					}
				break;
			case CMachineGunID:
				// Cannot run out of bullets.
				// Note time of next fire.
				m_lNextBulletTime	= m_pRealm->m_time.GetGameTime() + MS_BETWEEN_BULLETS;
				// If we have any of this weapon . . .
				if (m_stockpile.m_sNumBullets < CStockPile::ms_stockpileMax.m_sNumBullets)
					{
					// Success.
					m_stockpile.m_sNumBullets	= CStockPile::ms_stockpileMax.m_sNumBullets;
					}
				break;
			}

		// If we actually want to shoot the weapon . . .
		if (bShootWeapon)
			{
			// Deduct ammo.
			*psNumLeft	= *psNumLeft - 1;

			pweapon	= CCharacter::ShootWeapon(bitsInclude, bitsDontcare, bitsExclude);

			// If a weapon was returned . . .
			if (pweapon)
				{
				// Set the detection bits (not all weapons use these).  The only one
				// I know of currently is the heatseeker.
				pweapon->SetDetectionBits(
					CSmash::Character,
					0,
					m_pRealm->m_flags.bCoopMode ? CSmash::Good : 0);
				}
			}
		}

	return pweapon;
	}

////////////////////////////////////////////////////////////////////////////////
// Receive damage.
////////////////////////////////////////////////////////////////////////////////
void CDude::Damage(			// Returns nothing.
	short	sHitPoints,			// Hit points of damage to do.
	U16	u16ShooterId)		// In:  Thing responsible for damage.
	{
	// Remember if already dead . . .
	bool	bDead	= m_bDead;

	if (m_bInvincible == false)
		{
		if (StatsAreAllowed) Stat_DamageTaken += sHitPoints;

		m_stockpile.m_sHitPoints	-= sHitPoints;
		// If out of life . . .
		if (m_stockpile.m_sHitPoints <= 0)
			{
			m_stockpile.m_sHitPoints	= 0;

			if (m_state != State_Die && m_state != State_Dead)
				{
				// Go to die.
				SetState(State_Die);

				// Even if we fail to enter the die state (like, if we're being
				// blown up).  Be dead.
				m_bDead	= true;
				
				// If he wasn't already dead when he entered here, then register the kill.
				if (bDead == false)
					ScoreRegisterKill(m_pRealm, GetInstanceID(), u16ShooterId);
				}
			}
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Start the brain splat anim on its way.
////////////////////////////////////////////////////////////////////////////////
void CDude::StartBrainSplat(void)	// Returns nothing.
	{
	double	dBrainX, dBrainY, dBrainZ;
	GetLinkPoint(														// Returns nothing.
		m_panimCur->m_ptransRigid->GetAtTime(m_lAnimTime),	// In:  Transform specifying point.
		&dBrainX,														// Out: Point specified.
		&dBrainY,														// Out: Point specified.
		&dBrainZ);														// Out: Point specified.

	// Make absolute by adding dude's position to relative position.
	dBrainX	+= m_dX;
	dBrainY	+= m_dY;
	dBrainZ	+= m_dZ;

	// Create blood chunks.
	short	i;
	for (i = 0; i < BRAIN_SPLAT_NUM_CHUNKS; i++)
		{
		// Create blood particles . . .
		CChunk*	pchunk	= NULL;	// Initialized for safety.
		// Note that this will fail if particles are disabled.
		if (CThing::Construct(CChunkID, m_pRealm, (CThing**)&pchunk) == 0)
			{
			pchunk->Setup(
				dBrainX,				// Source position.
				dBrainY,				// Source position.
				dBrainZ,				// Source position.
				m_dRot - 180,		// Angle of velocity.
				BRAIN_SPLAT_SWAY,	// Angle sway.
				40,					// Velocity (X/Z plane).
				80,					// Velocity (X/Z plane) sway.
				50,					// Velocity (Vertical).
				100,					// Velocity (Vertical) sway.
				CChunk::Blood);	// Type of chunk.
			}														
		}															
	}

////////////////////////////////////////////////////////////////////////////////
// Determines if supplied position is valid tweaking it if necessary.
// (virtual).
////////////////////////////////////////////////////////////////////////////////
bool CDude::MakeValidPosition(		// Returns true, if new position was valid.
												// Returns false, if could not reach new position.
	double*	pdNewX,						// In:  x position to validate.
												// Out: New x position.
	double*	pdNewY,						// In:  y position to validate.
												// Out: New y position.
	double*	pdNewZ,						// In:  z position to validate.
												// Out: New z position.
	short	sVertTolerance /*= 0*/)		// Vertical tolerance.
	{
	bool bValidatedPosition	= false;	// Assume failure.

	double	dCrawlerNewX;
	double	dCrawlerNewY;
	double	dCrawlerNewZ;
	short		sTerrainH;

	// Make sure the position has not changed since our crawlage.
	ASSERT(m_dX == m_dLastCrawledToPosX);
	ASSERT(m_dZ == m_dLastCrawledToPosZ);

	// Restore invalid movements to last good crawl position.
	// The crawler cannot keep us out of things unless it is the only
	// thing responsible for the movement.
	if (m_dX != m_dLastCrawledToPosX)
		m_dX	= m_dLastCrawledToPosX;

	if (m_dZ != m_dLastCrawledToPosZ)
		m_dZ	= m_dLastCrawledToPosZ;

	// Ask crawler for valid position as close as possible to new position
	if (m_crawler.Move(		// Returns 0 if successfull, non-zero otherwise
		m_dX, 					// In:  Position #1 xcoord                     
		m_dY, 					// In:  Position #1 ycoord                     
		m_dZ, 					// In:  Position #1 zcoord                     
		*pdNewX, 				// In:  Position #2 xcoord                     
		*pdNewY, 				// In:  Position #2 ycoord                     
		*pdNewZ, 				// In:  Position #2 zcoord                     
		&dCrawlerNewX, 		// Out: Final position xcoord                  
		&dCrawlerNewY, 		// Out: Final position ycoord                  
		&dCrawlerNewZ,			// Out: Final position zcoord                  
		&sTerrainH)				// Out: Terrain height at new location
		== 0)
		{
		// Success.
		bValidatedPosition	= true;

		*pdNewX	= dCrawlerNewX;
		*pdNewY	= dCrawlerNewY;
		*pdNewZ	= dCrawlerNewZ;

		// Stored crawled to position on X/Z plane.
		m_dLastCrawledToPosX	= dCrawlerNewX;
		m_dLastCrawledToPosZ	= dCrawlerNewZ;

		// If we're gonna be at or below ground level . . .
		if (sTerrainH >= *pdNewY)
			{
			// Get outta there!
			*pdNewY	= sTerrainH;
			// Update vertical delta.
			m_dExtVertDeltaVel	+= -m_dExtVertVel;
			// Reset vertical velocity.
			m_dExtVertVel	= 0.0;
			
			m_bAboveTerrain	= false;
			}
		else
			{
			m_bAboveTerrain	= true;
			}
		}
	else
		{
		// Failure.  Homer, I can't seem to move under my own power.
		m_dVel	= 0.0;
		m_dAcc	= 0.0;
		m_dDrag	= 0.0;
		}

	return bValidatedPosition;
	}

////////////////////////////////////////////////////////////////////////////////
// Message handlers.
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Handles a msg_Shot.
// (virtual).
////////////////////////////////////////////////////////////////////////////////
void CDude::OnShotMsg(			// Returns nothing.
	Shot_Message* pshotmsg)		// In:  Message to handle.
	{
	// Remember if already dead . . .
	bool	bDead	= m_bDead;

	if (StatsAreAllowed)
		{
		Stat_HitsTaken++;
		if (Stat_HitsTaken >= 10000)
			UnlockAchievement(ACHIEVEMENT_TAKE_10000_HITS);
		}

	// Give the msg to CThing3d before we alter it.
	CThing3d::OnShotMsg(pshotmsg);

	short	sInitKevlarLayers	= m_stockpile.m_sKevlarLayers;

	if (m_stockpile.m_sKevlarLayers > 0)
		{
		pshotmsg->sDamage	/= m_stockpile.m_sKevlarLayers * KEVLAR_PROTECTION_MULTIPLIER;

		// Choclate melts vest.
		m_stockpile.m_sKevlarLayers = MAX((short)(m_stockpile.m_sKevlarLayers - (GetRand() & 0x3)), (short)0);
		}

	CCharacter::OnShotMsg(pshotmsg);

	// If we're taking damage . . .
	if (pshotmsg->sDamage > 0)
		{
		Damage(pshotmsg->sDamage, pshotmsg->u16ShooterID);

		// Check for mininum duration since last shot time . . .
		if (	m_pRealm->m_time.GetGameTime() > m_lLastShotTime + MIN_CANNOT_BE_SHOT_DURATION
			||	m_state == State_Stand)
			{
			if (SetState(State_Shot) == true)
				{
				m_lLastShotTime	= m_pRealm->m_time.GetGameTime();
				}
			}
		}

	// If we have protection . . .
	if (sInitKevlarLayers > 0)
		{
		// Audible and visual feedback.
		PlaySample(g_smidBulletIntoVest, SampleMaster::Weapon);
		double	dHitY	= m_dY + m_sprite.m_sRadius + RAND_SWAY(VEST_HIT_SWAY);
		// X/Z position depends on angle of shot (it is opposite).
		short	sDeflectionAngle	= rspMod360(pshotmsg->sAngle + 180);
		double	dHitX	= m_dX + COSQ[sDeflectionAngle] * TORSO_RADIUS;
		double	dHitZ	= m_dZ - SINQ[sDeflectionAngle] * TORSO_RADIUS;

		StartAnim(VEST_HIT_RES_NAME, dHitX, dHitY, dHitZ, false);

		// Create a kevlar peice.
		CChunk*	pchunk	= NULL;	// Initialized for safety.
		// Note that this will fail if particles are disabled.
		if (CThing::Construct(CChunkID, m_pRealm, (CThing**)&pchunk) == 0)
			{
			pchunk->Setup(
				dHitX,						// Source position.
				dHitY,						// Source position.
				dHitZ,						// Source position.
				sDeflectionAngle,			// Angle of velocity.
				0,								// Angle sway.
				40,							// Velocity (X/Z plane).
				80,							// Velocity (X/Z plane) sway.
				50,							// Velocity (Vertical).
				100,							// Velocity (Vertical) sway.
				CChunk::Kevlar);			// Type of chunk.
			}
		}
	
	// If still alive . . .
	if (m_bDead == false)
		{
		if (m_pRealm->m_time.GetGameTime() > m_lLastYellTime + MIN_BETWEEN_YELLS)
			{
			// If we took damage . . .
			if (pshotmsg->sDamage > 0)
				{
				PlaySample(g_smidBlownupYell, SampleMaster::Voices);

				m_lLastYellTime	= m_pRealm->m_time.GetGameTime();
				}
			}
		}
	else
		{
		// If not previously dead or dying . . .
		if (bDead == false && m_state != State_Die)
			{
			// Ahhhhhhhhhh....
			PlaySample(g_smidDyingYell, SampleMaster::Voices);

			m_lLastYellTime	= m_pRealm->m_time.GetGameTime();
			}
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Handles an Explosion_Message.
// (virtual).
////////////////////////////////////////////////////////////////////////////////
void CDude::OnExplosionMsg(				// Returns nothing.
	Explosion_Message* pexplosionmsg)	// In:  Message to handle.
	{
	// Remember if already dead . . .
	bool	bDead	= m_bDead;

	CCharacter::OnExplosionMsg(pexplosionmsg);

	if (SetState(State_BlownUp) == true)
		{
		// Let's increase his vertical just a bit.
		m_dExtVertVel	*= EXPLOSION_VERTICAL_VEL_MULTIPLIER;

		Damage(pexplosionmsg->sDamage, pexplosionmsg->u16ShooterID);

		// If he is carying a flag item, then he should drop it and
		// pass the explosion message on to the flags so they can react.
		DropAllFlags( (GameMessage*)pexplosionmsg);

		// If still alive . . .
		if (m_bDead == false)
			{
			if (m_pRealm->m_time.GetGameTime() > m_lLastYellTime + MIN_BETWEEN_YELLS)
				{
				PlaySample(g_smidBlownupYell, SampleMaster::Voices);

				m_lLastYellTime	= m_pRealm->m_time.GetGameTime();
				}
			}
		else
			{
			// If not previously dead . . .
			if (bDead == false)
				{
				// Ahhhhhhhhhh....
				PlaySample(g_smidDyingYell, SampleMaster::Voices);

				m_lLastYellTime	= m_pRealm->m_time.GetGameTime();
				}
			}

		}

	m_lLastShotTime	= m_pRealm->m_time.GetGameTime();
	}

////////////////////////////////////////////////////////////////////////////////
// Handles a Burn_Message.
// (virtual).
////////////////////////////////////////////////////////////////////////////////
void CDude::OnBurnMsg(			// Returns nothing.
	Burn_Message* pburnmsg)		// In:  Message to handle.
	{
	if (SetState(State_Burning) == true)
		{
		CCharacter::OnBurnMsg(pburnmsg);
		
		PlaySample(g_smidBurningMainGuy, SampleMaster::Voices);
		
		// If he is carying a flag item, then he should drop them.
		DropAllFlags( (GameMessage*)pburnmsg);
		}

	short sDamage = MAX((short) 1, (short) ((double) pburnmsg->sDamage * (((double) m_pRealm->m_flags.sDifficulty) / 10.0)));
	Damage(sDamage, pburnmsg->u16ShooterID);
	}

////////////////////////////////////////////////////////////////////////////////
// Handles a PutMeDown_Message
// (virtual)
////////////////////////////////////////////////////////////////////////////////

void CDude::OnPutMeDownMsg(		// Returns nothing
	PutMeDown_Message* pputmedownmsg)
	{
	// If he is carrying the flag item, then he should put it down
	if (pputmedownmsg->u16FlagInstanceID != CIdBank::IdNil)
		{
		// Detatch child and update its position
		CThing3d* pthing3d = DetachChild(
			&(pputmedownmsg->u16FlagInstanceID),
			((CDudeAnim3D*) m_panimCur)->m_ptransLeft->GetAtTime(m_lAnimTime) );
		if (pthing3d)
			{
			pthing3d->m_dX		= m_dX;
			pthing3d->m_dY		= m_dY;
			pthing3d->m_dZ		= m_dZ;
			pthing3d->m_state = State_Die;
			}
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Handles an ObjectDelete_Message.
// (virtual).
////////////////////////////////////////////////////////////////////////////////
void CDude::OnDeleteMsg(					// Returns nothing.
	ObjectDelete_Message* pdeletemsg)	// In:  Message to handle.
	{
	CCharacter::OnDeleteMsg(pdeletemsg);

	SetState(State_Delete);
	}

////////////////////////////////////////////////////////////////////////////////
// Handles a Suicide_Message.
// (virtual).
////////////////////////////////////////////////////////////////////////////////
void CDude::OnSuicideMsg(				// Returns nothing.
	Suicide_Message* psuicidemsg)		// In:  Message to handle.
	{
	CCharacter::OnSuicideMsg(psuicidemsg);

	SetState(State_Suicide);
	}

////////////////////////////////////////////////////////////////////////////////
// Implements basic functionality while being blown up and returns true
// until the state is completed.
// (virtual).
////////////////////////////////////////////////////////////////////////////////
bool CDude::WhileBlownUp(void)	// Returns true until state is complete.
	{
	bool	bStatePersists	= true;	// Assume not done.
	double	dNewX, dNewY, dNewZ;

	// Get time from last call in seconds.
	long		lCurTime	= m_pRealm->m_time.GetGameTime();
	double	dSeconds	= double(lCurTime - m_lPrevTime) / 1000.0;

	// Update Velocities ////////////////////////////////////////////////////////
	UpdateVelocities(dSeconds, ms_dMaxVelForeFast, ms_dMaxVelBackFast);
	
	// Get New Position /////////////////////////////////////////////////////////
	GetNewPosition(&dNewX, &dNewY, &dNewZ, dSeconds);

	// Validate New Position ////////////////////////////////////////////////////
	if (MakeValidPosition(&dNewX, &dNewY, &dNewZ, MaxStepUpThreshold) == true)
		{
		// Update Values /////////////////////////////////////////////////////////

		m_dX	= dNewX;
		m_dY	= dNewY;
		m_dZ	= dNewZ;

		UpdateFirePosition();
		}
	else
		{
		// Restore Values ////////////////////////////////////////////////////////
		
		// Didn't actually move and, therefore, did not actually accelerate.  
		// Restore velocities.
//			m_dVel			-= m_dDeltaVel;
//			m_dExtHorzVel	-= m_dExtHorzDeltaVel;
		m_dExtVertVel	-= m_dExtVertDeltaVel;
		}

	// If it was above the ground last time and is now below the ground, it must have
	// hit the ground and the blown up state is complete
	if (m_bAboveTerrain == false)
	{

		// If not yet triggered . . .
		if (m_bGenericEvent1 == false)
			{
			PlaySample(g_smidBodyImpact2, SampleMaster::Unspecified);

			m_bGenericEvent1	= true;
			}

		// Make sure its done with current animation also
		if (m_lAnimTime > m_panimCur->m_psops->TotalTime())
			{
			bStatePersists = false;
			}
	}

	return bStatePersists;
	}

////////////////////////////////////////////////////////////////////////////////
// Execute the nearest writhing guy, if any.
////////////////////////////////////////////////////////////////////////////////
void CDude::OnExecute(void)		// Returns nothing.
	{
	// Update execution point via link point.
	double	dMuzzleX, dMuzzleY, dMuzzleZ;
	GetLinkPoint(														// Returns nothing.
		m_panimCur->m_ptransRigid->GetAtTime(m_lAnimTime),	// In:  Transform specifying point.
		&dMuzzleX,														// Out: Point speicfied.
		&dMuzzleY,														// Out: Point speicfied.
		&dMuzzleZ);														// Out: Point speicfied.

	// Get current weapon and stockpile.
	short*		psNumLeft;
	ClassIDType	idWeapon;
	GetWeaponInfo(SemiAutomatic, &idWeapon, &psNumLeft);
	if (*psNumLeft > 0)
		{
		// Muzzle flare and sound feedback.
		m_bullets.Flare(
			m_dRot, 
			m_dX + dMuzzleX,
			m_dY + dMuzzleY,
			m_dZ + dMuzzleZ,
			m_pRealm);

		// Shoot this thing.
		GameMessage	msg;
		msg.msg_Shot.eType			= typeShot;
		msg.msg_Shot.sPriority		= 0;
		msg.msg_Shot.sAngle			= m_dRot;
		msg.msg_Shot.u16ShooterID	= m_u16InstanceId;
		msg.msg_Shot.sDamage			= 10;

		// Send it the message.
		SendThingMessage(&msg, m_idVictim);

		// Note time of next fire.
		m_lNextBulletTime	= m_pRealm->m_time.GetGameTime() + MS_BETWEEN_BULLETS;
		// Deduct a shot.
		*psNumLeft	= *psNumLeft - 1;
		}
	else
		{
		// Feedback.
		PlaySample(g_smidOutOfBullets, SampleMaster::Weapon);
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Implements one-time functionality for when a weapon is destroyed while
// we were moving it (i.e., before we let go or ShootWeapon()'ed it).
// This can occur when a weapon, while traveling along our rigid body,
// enters terrain.
////////////////////////////////////////////////////////////////////////////////
// virtual (overridden here).
void CDude::OnWeaponDestroyed(void)
	{
	// Feedback that we aborted.  Perhaps different by state?
	PlaySample(g_smidGeneralBeep, SampleMaster::UserFeedBack);
	// Finish.
	switch (m_state)
		{
		case State_Throw:
		case State_ThrowRelease:
		case State_ThrowFinish:
			SetState(State_ThrowDone);
			break;

		case State_Launch:
		case State_LaunchRelease:
		case State_LaunchFinish:
			SetState(State_LaunchDone);
			break;

		default:
			// Be done with our state.
			SetState(State_Persistent);
			break;
		}
	
	m_weaponShooting	= NoWeapon;
	}

////////////////////////////////////////////////////////////////////////////////
// Revive a dead dude.  This is a more graceful way than just setting
// his state.  This will restore stockpile and make him animate out of
// a warp point.
////////////////////////////////////////////////////////////////////////////////
void CDude::Revive(				// Returns nothing.
	bool	bWarpIn	/*= true*/)	// In:  true, to warp in, false to just get up.
	{
	// Must be dead, lying there still in multiplayer mode (or we could be cheating) . . .
	if (	m_bDead == true 
		&&	(m_pRealm->m_flags.bMultiplayer == true || bWarpIn == false) )
		{
		// Drop that fire.
		m_u16IdFire	= CIdBank::IdNil;

		// Let's not be responding to old news.
		m_MessageQueue.Empty();

		// Create powerup . . .
		// NOTE that if the dude is going to be warped in (that is, he is going to get
		// reloaded with stuff from the warp), he only drops his current weapon and all
		// his ammo.  If he is not going to be warped in (that is, he will NOT get
		// any stuff from a warp), he drops all his stuff so he can pick it back up.
		DropPowerUp(&m_stockpile, bWarpIn);
		// Clear my stockpile.
		m_stockpile.Zero();

		// Have a life.
		m_stockpile.m_sHitPoints	= m_sOrigHitPoints;
		m_smash.m_bits					&= ~CSmash::Dead;
		m_bDead							= false;
		m_sBrightness					= 0;

		CDude*	pdude	= this;
		if (bWarpIn == true)
			{
			// It would be best if this only occurred when we're in the dead state
			// and, hence, on the proper animation.
			// This should flag us to other situations.
			ASSERT(m_state == State_Dead);

			// Render current dead frame into background to stay.
			m_pRealm->m_scene.DeadRender3D(
				m_pRealm->m_phood->m_pimBackground,	// Destination image.
				&m_sprite,									// Tree of 3D sprites to render.
				m_pRealm->m_phood);						// Dst clip rect.

			// First try to find a warp in point . . .
			if (CWarp::WarpInAnywhere(
				m_pRealm,
				&pdude, 
				CWarp::CopyStockPile) == 0)
				{
				// Force to base weapon
				SetWeapon(SemiAutomatic, true);
				NextWeapon();

				// Run on screen or out of building or whatever.
				SetState(State_Run);
				m_dVel	= ms_dMaxVelForeFast;
				}
			else
				{
				// Just get up, you rogue.
				SetState(State_GetUp);
				}
			}
		else
			{
			// Just get up, you rogue.
			SetState(State_GetUp);
			}
		}
	}

////////////////////////////////////////////////////////////////////////////////
// ShowTarget - If the targeting aid is turned on, then this will put the 
//		          targeting sprite on the target.
////////////////////////////////////////////////////////////////////////////////
void CDude::ShowTarget()
{
	if (m_bTargetingHelpEnabled && m_bDead == false)
	{
		// sAngle must be between 0 and 359.
		short sRotY = rspMod360((short) m_dRot);
		short sRangeXZ = 100;
		short sRadius = 20;

		float	fRateX = COSQ[sRotY] * sRangeXZ;
		float	fRateZ = -SINQ[sRotY] * sRangeXZ;
		float	fRateY = 0.0;	// If we ever want vertical movement . . .

		// Set initial position to first point to check (NEVER checks original position).
		float	fPosX = m_dX + fRateX;
		float	fPosY = m_dY + fRateY;
		float	fPosZ = m_dZ + fRateZ;

		if (m_TargetSprite.m_psprParent)
			m_TargetSprite.m_psprParent->RemoveChild(&m_TargetSprite);
		((CThing3d*)this)->m_sprite.AddChild(&m_TargetSprite);
		// Map from 3d to 2d coords
		Map3Dto2D(
			fRateX - m_sprite.m_sRadius / 2,
			m_sprite.m_sRadius * 2,
			fRateZ,
			&m_TargetSprite.m_sX2,
			&m_TargetSprite.m_sY2);
		m_TargetSprite.m_sInFlags &= ~CSprite::InHidden;
		m_TargetSprite.m_sLayer = CRealm::LayerSprite16;
	}
	else
		m_TargetSprite.m_sInFlags |= CSprite::InHidden;
}
/*
void CDude::ShowTarget(void)
{
	if (m_bTargetingHelpEnabled && m_bDead == false)
	{
		CThing* pTargetThing = NULL;

		if (IlluminateTarget((short) m_dX,
									(short) m_dY,
									(short) m_dZ,
									(short) m_dRot,
									300,
									20,
									CSmash::Character,
									CSmash::Good | CSmash::Bad,
									0,
									&pTargetThing,
									&m_smash))
		{
			if (m_TargetSprite.m_psprParent)
				m_TargetSprite.m_psprParent->RemoveChild(&m_TargetSprite);
			((CThing3d*) pTargetThing)->m_sprite.AddChild(&m_TargetSprite);
			// Map from 3d to 2d coords
			Map3Dto2D(
				(short) 0, 
				(short) 30, 
				(short) 0, 
				&m_TargetSprite.m_sX2, 
				&m_TargetSprite.m_sY2);
			m_TargetSprite.m_sInFlags &= ~CSprite::InHidden;
		}
		else
		{
			m_TargetSprite.m_sInFlags |= CSprite::InHidden;
		}
	}
	else
		m_TargetSprite.m_sInFlags |= CSprite::InHidden;
}
*/

////////////////////////////////////////////////////////////////////////////////
// Next weapon please.
////////////////////////////////////////////////////////////////////////////////
void CDude::NextWeapon(void)
	{
	short	sNumTried	= 0;
	short	sCurWeapon	= m_weapontypeCur;

	while (sNumTried < NumWeaponTypes)
		{
		sNumTried++;
		sCurWeapon++;
		if (sCurWeapon == NumWeaponTypes)
			{
			sCurWeapon	= NoWeapon + 1;
			}

		if (SetWeapon((WeaponType)sCurWeapon, false) == true)
			{
			break;
			}
		}

	if (sNumTried == NumWeaponTypes)
		{
		SetWeapon(NoWeapon, false);
		}
	PlaySample(g_smidLoadedWeapon, SampleMaster::UserFeedBack);
	}

////////////////////////////////////////////////////////////////////////////////
// Previous weapon please.
////////////////////////////////////////////////////////////////////////////////
void CDude::PrevWeapon(void)
	{
	short	sNumTried	= 0;
	short	sCurWeapon	= m_weapontypeCur;

	while (sNumTried < NumWeaponTypes)
		{
		sNumTried++;
		sCurWeapon--;
		if (sCurWeapon == NoWeapon)
			{
			sCurWeapon	= NumWeaponTypes - 1;
			}

		if (SetWeapon((WeaponType)sCurWeapon, false) == true)
			{
			break;
			}
		}

	if (sNumTried == NumWeaponTypes)
		{
		SetWeapon(NoWeapon, false);
		}
	PlaySample(g_smidLoadedWeapon, SampleMaster::UserFeedBack);
	}

////////////////////////////////////////////////////////////////////////////////
// Set the current weapon.
////////////////////////////////////////////////////////////////////////////////
bool CDude::SetWeapon(					// Returns true if weapon could be set as current.
	WeaponType weapon,					// In:  New weapon to attempt to make the current.
	bool	bSetIfNoAmmo /*= true*/)	// In:  true to set weapon (even if no ammo).
	{
	bool	bSetWeapon	= true;	// Assume we could set the weapon.

	switch (weapon)
		{
		case NoWeapon:
			break;
		case SemiAutomatic:
			if (m_stockpile.m_sMachineGun == 0)
				{
				bSetWeapon	= false;
				}
			break;
		case Grenade:
			break;
		case FireBomb:
			break;
		case Rocket:
			if (m_stockpile.m_sMissileLauncher == 0)
				{
				bSetWeapon	= false;
				}
			break;
		case Napalm:
			if (m_stockpile.m_sNapalmLauncher == 0)
				{
				bSetWeapon	= false;
				}
			break;
		case ShotGun:
			if (m_stockpile.m_sShotGun == 0)
				{
				bSetWeapon	= false;
				}
			break;
		case SprayCannon:
			if (m_stockpile.m_sSprayCannon == 0)
				{
				bSetWeapon	= false;
				}
			break;
		case FlameThrower:
			if (m_stockpile.m_sFlameThrower == 0)
				{
				bSetWeapon	= false;
				}
			break;
		case BouncingBettyMine:
		case ProximityMine:
		case TimedMine:
//		case RemoteMine: //Fixed crash when changing weapons and firing at the same time
			break;
		case Heatseeker:
			if (m_stockpile.m_sMissileLauncher == 0)
				{
				bSetWeapon	= false;
				}
			break;
		case DeathWad:
			if (m_stockpile.m_sDeathWadLauncher == 0)
				{
				bSetWeapon	= false;
				}
			break;
		case DoubleBarrel:
			if (m_stockpile.m_sDoubleBarrel == 0)
				{
				bSetWeapon	= false;
				}
			break;

		default:
			bSetWeapon = false;
			break;
		}

	// Get info on this weapon.
	ClassIDType	idDummy;
	short*		psNum;
	GetWeaponInfo(weapon, &idDummy, &psNum);

	// If weapon was available . . .
	if (bSetWeapon == true)
		{
		// If we have no ammo for this one . . .
		if (*psNum <= 0)
			{
			// If set we cannot set when no ammo . . .
			if (bSetIfNoAmmo == false)
				{
				bSetWeapon	= false;
				}
			else
				{
				PlaySample(g_smidEmptyWeapon, SampleMaster::UserFeedBack);
				}
			}
		else
			{
			if (bSetIfNoAmmo == true)
				{
				PlaySample(g_smidLoadedWeapon, SampleMaster::UserFeedBack);
				}
			}


		// If able to change weapons . . .
		if (bSetWeapon == true)
			{
			// Set new weapon.
			m_weapontypeCur	= weapon;

			// If we have ammo . . .
			if (*psNum > 0)
				{
				GameMessage msg;
				msg.msg_WeaponSelect.eType = typeWeaponSelect;
				msg.msg_WeaponSelect.sPriority = 0;
				msg.msg_WeaponSelect.sWeapon = (short) m_weapontypeCur;
				CThing* pDemon = m_pRealm->m_aclassHeads[CThing::CDemonID].GetNext();
				if (pDemon)
					SendThingMessage(&msg, pDemon);				
				}
			}
		}
	else
		{
		if (bSetIfNoAmmo == true)
			{
			PlaySample(g_smidEmptyWeapon, SampleMaster::UserFeedBack);
			}
		}
		
	return bSetWeapon;
	}


////////////////////////////////////////////////////////////////////////////////
// Drop a powerup with the settings described by the specified stockpile.
////////////////////////////////////////////////////////////////////////////////
CPowerUp* CDude::DropPowerUp(		// Returns new powerup on success; NULL on failure.
	CStockPile*	pstockpile,			// In:  Settings for powerup.
	bool			bCurWeaponOnly)	// In:  true, if only the current weapon should be
											// in the powerup; false, if all.
	{
	CPowerUp*	ppowerup	= NULL;

	// If not empty . . .
	if (pstockpile->IsEmpty() == false)
		{
		// Create powerup . . .
		if (ConstructWithID(CPowerUpID, m_pRealm, (CThing**)&ppowerup) == 0)
			{
			// Put stockpile into powerup.
			ppowerup->m_stockpile.Copy(pstockpile);

			if (bCurWeaponOnly)
				{
				// Note whether we had the weapon.  Not sure how it would be possible
				// but just in case we can somehow have a weapon selected that we do
				// hot have.
				short	sHasWeapon	= ppowerup->m_stockpile.GetWeapon(m_weapontypeCur);
				// Remove all but our current weapon.
				short	sIndex;
				for (sIndex = SemiAutomatic; sIndex <= DoubleBarrel; sIndex++)
					{
					// Zero.
					ppowerup->m_stockpile.GetWeapon(sIndex) = 0;
					}

				// Add back our currently selected weapon.
				ppowerup->m_stockpile.GetWeapon(m_weapontypeCur) = sHasWeapon;
				}

			// Place powerup at our feet.
			ppowerup->Setup(m_dX, m_dY, m_dZ);
			}
		}

	return ppowerup;
	}

////////////////////////////////////////////////////////////////////////////////
// Play a step noise if the event is different from the last.
////////////////////////////////////////////////////////////////////////////////
void CDude::PlayStep(void)				// Returns nothing.
	{
#if 1
	// If there is an event channel . . .
	if (m_panimCur->m_pevent != NULL)
		{
		// If the current event is different from the last . . .
		U8	u8Event	= *((U8*)(m_panimCur->m_pevent->GetAtTime(m_lAnimTime)) );
		if (u8Event > 0 && u8Event != m_u8LastEvent)
			{
			PlaySample(g_smidStep, SampleMaster::Unspecified);

			m_u8LastEvent	= u8Event;
			}
		}
#endif
	}


////////////////////////////////////////////////////////////////////////////////
// Find someone to execute.
////////////////////////////////////////////////////////////////////////////////
bool CDude::FindExecutee(void)		// Returns true, if we found one; false, otherwise.
	{
	bool	bFoundOne	= false;	// Assume not found.

	CSmash*	psmashee	= NULL;	// Safety.

	// Find the closest person including writhers.
	if (m_pRealm->m_smashatorium.QuickCheckClosest(
		&m_smash,					// In:  CSmash to check
/*		CSmash::Character		
		| CSmash::Misc 
		| CSmash::Barrel 
		| CSmash::Mine 
		| */CSmash::AlmostDead,	// In:  Bits we can hit.
		0,								// In:  Bits that you don't care about
		0,								// In:  Bits that must be 0 to collide with a given CSmash
		&psmashee) == true)
		{
		// We can handle anyone that wants to reveal their location . . .
		// This can be done via the GetX/Y/Z() functions or their smash . . .
		if ( (psmashee->m_pThing->GetX() != CThing::InvalidPosition
			&&	psmashee->m_pThing->GetY() != CThing::InvalidPosition
			&&	psmashee->m_pThing->GetZ() != CThing::InvalidPosition)
			|| psmashee->m_pThing->GetSprite() )
			{
			// Found one.
			bFoundOne	= true;
			// Remember who.
			m_idVictim	= psmashee->m_pThing->GetInstanceID();
			}
		}

	return bFoundOne;
	}


////////////////////////////////////////////////////////////////////////////////
// Track executee.
////////////////////////////////////////////////////////////////////////////////
bool CDude::TrackExecutee(		// Returns true to persist, false, if we lost the target.
			double dSeconds)		// In:  Seconds since last iteration.
	{
	bool	bPersist	= true;	// Assume we'll persist.
	
	// Get pointer to target.
	CThing*	pthing;
	if (m_pRealm->m_idbank.GetThingByID(&pthing, m_idVictim) == 0)
		{
		double	dVictimX;
		double	dVictimZ;		
		// Try to use smash position first, then resort to thing position.
		CSmash*	psmash	= pthing->GetSmash();
		if (psmash != NULL)
			{
			// This is generally more accurate for an execution point.
			dVictimX	= psmash->m_sphere.sphere.X;
			dVictimZ	= psmash->m_sphere.sphere.Z;
			}
		else
			{
			// This'll do though.
			dVictimX	= pthing->GetX();
			dVictimZ	= pthing->GetZ();
			}

		short	sDistX			= dVictimX - m_dX;
		short	sDistZ			= m_dZ - dVictimZ;
		double	dSqrDistanceXZ	= ABS2(sDistX, sDistZ);

		// Determine angle to target.
		double	dRot		= rspATan(sDistZ, sDistX);
		// Determine which rotation direction to target is smaller.
		double	dDelta	= rspDegDelta(m_dRot, dRot);

		// If turning counter clockwise . . .
		if (dDelta > 0.0)
			{
			m_dRot	+= MIN(dDelta, g_InputSettings.m_dStillFastDegreesPerSec * dSeconds);
			}
		else
			{
			m_dRot	+= MAX(dDelta, -g_InputSettings.m_dStillFastDegreesPerSec * dSeconds);
			}

#if 1
		// If too close . . .
		if (dSqrDistanceXZ < MIN_SQR_DISTANCE_TO_EXECUTEE)
			{
			// Back up.
			m_dVel	= dSqrDistanceXZ - MIN_SQR_DISTANCE_TO_EXECUTEE;
			}
#else
		// Another solution, if we don't like the above, is to take
		// the gun and use a transform we create (rather than the
		// rigid body one) and aim the gun directly at the target
		// point.
#endif
		}
	else
		{
		// Lost our target.  Continuing is futile.
		m_idVictim	= CIdBank::IdNil;
		bPersist	= false;
		}

	return bPersist;
	}

////////////////////////////////////////////////////////////////////////////////
// Take a powerup.
////////////////////////////////////////////////////////////////////////////////
void CDude::TakePowerUp(		// Returns nothing.
	CPowerUp**	pppowerup)		// In:  Power up to take from.
										// Out: Ptr to powerup, if it persisted; NULL otherwise.
	{
	CStockPile*	pspMax;
	// Note if we do or will have the backpack.
	if (m_stockpile.m_sBackpack || (*pppowerup)->m_stockpile.m_sBackpack)
		{
		pspMax	= &CStockPile::ms_stockpileBackPackMax;
		}
	else
		{
		pspMax	= &CStockPile::ms_stockpileMax;
		}

	// Create a stockpile of the amount we can take from the powerup.
	CStockPile	spTake;
	spTake.Copy(pspMax);
	spTake.Sub(&m_stockpile);
	// Intersect the amount we can take with the amount available.
	spTake.Intersect( &( (*pppowerup)->m_stockpile) );

	// If we got anything . . .
	if (spTake.IsEmpty() == false)
		{
		// Cash it in.
		m_stockpile.Add( &spTake );

		// If in MP mode, empty the powerup so it goes away.  Otherwise, just remove what we took
		if (m_pRealm->m_flags.bMultiplayer)
			(*pppowerup)->m_stockpile.Zero();
		else
			(*pppowerup)->m_stockpile.Sub( &spTake );

		// Play feedback.
		(*pppowerup)->PickUpFeedback();

		// Store its ID so we can attempt to get it if it does persist.
		U16	idInstance	= (*pppowerup)->GetInstanceID();

		// Let powerup decide if it should persist.
		(*pppowerup)->RepaginateNow();

		// If it persisted . . .
		if (m_pRealm->m_idbank.GetThingByID((CThing**)pppowerup, idInstance) == 0)
			{
			ASSERT( (*pppowerup)->GetClassID() == CPowerUpID);
			}
		else
			{
			*pppowerup	= NULL;
			}
		}
	else
		{
		// Temp feedback for needed nothing from the weapon.
		PlaySample(g_smidOutOfBullets, SampleMaster::UserFeedBack);

		// If in MP mode, eat the powerup
		if (m_pRealm->m_flags.bMultiplayer)
			{
			(*pppowerup)->m_stockpile.Zero();
			(*pppowerup)->RepaginateNow();
			*pppowerup	= NULL;
			}
		}

	// If it still exists . . .
	if (*pppowerup)
		{
		// Store its ID so we can attempt to get it if it does persist.
		U16	idInstance	= (*pppowerup)->GetInstanceID();

		// Toss it.
		TossPowerUp(*pppowerup, 130);

		// If it persisted . . .
		if (m_pRealm->m_idbank.GetThingByID((CThing**)pppowerup, idInstance) == 0)
			{
			ASSERT( (*pppowerup)->GetClassID() == CPowerUpID);
			}
		else
			{
			*pppowerup	= NULL;
			}
		}
	}


////////////////////////////////////////////////////////////////////////////////
// Create a cheat powerup.
////////////////////////////////////////////////////////////////////////////////
CPowerUp* CDude::CreateCheat(	// Returns new powerup on success; NULL on failure.
	CStockPile*	pstockpile)		// In:  Settings for powerup.
	{
	// First deduct from the cheat powerup what we cannot use.
	CStockPile	spUsable;
	spUsable.Copy( m_stockpile.m_sBackpack ? &CStockPile::ms_stockpileBackPackMax : &CStockPile::ms_stockpileMax );
	spUsable.Sub( &m_stockpile );

	// Intersect with the supplied.
	spUsable.Intersect(pstockpile);

	// Finally, drop it.

	// If it contains anything . . .
	CPowerUp*	ppowerup;
	if (spUsable.IsEmpty() == false)
		{
		ppowerup	= DropPowerUp( &spUsable, false );

		if (ppowerup)
			{
			// Take it right away.
			TakePowerUp(&ppowerup);
			}
		}
	else
		{
		ppowerup	= NULL;
		}

	UnlockAchievement(ACHIEVEMENT_ENABLE_CHEATS);
	Flag_Achievements |= FLAG_USED_CHEATS;

	// Let the demon know.that they are cheating
	GameMessage msg;
	msg.msg_Cheater.eType = typeCheater;
	msg.msg_Cheater.sPriority = 0;
	CThing* pDemon = m_pRealm->m_aclassHeads[CThing::CDemonID].GetNext();
	if (pDemon)
		SendThingMessage(&msg, pDemon);				

	return ppowerup;
	}

////////////////////////////////////////////////////////////////////////////////
// Break a powerup open and toss it.
////////////////////////////////////////////////////////////////////////////////
void CDude::TossPowerUp(		// Returns nothing.
	CPowerUp*	ppowerup,		// In:  Powerup to toss.
	short			sVelocity)		// In:  Velocity of toss.
	{
	// Blow it up.
	GameMessage	msg;
	msg.msg_Explosion.eType				= typeExplosion;
	msg.msg_Explosion.sPriority		= 0;
	msg.msg_Explosion.sDamage			= 0;
	msg.msg_Explosion.sX					= m_dX;
	msg.msg_Explosion.sY					= m_dY;
	msg.msg_Explosion.sZ					= m_dZ;
	msg.msg_Explosion.sVelocity		= sVelocity;
	msg.msg_Explosion.u16ShooterID	= GetInstanceID();

	SendThingMessage(&msg, msg.msg_Explosion.sPriority, ppowerup);
	}

////////////////////////////////////////////////////////////////////////////////
// Get the current weapon the dude has ready to use.
////////////////////////////////////////////////////////////////////////////////
CDude::WeaponType CDude::GetCurrentWeapon(void)
	{
	return m_weapontypeCur;
	}

////////////////////////////////////////////////////////////////////////////////
// Sets the dude's position.  It is very important that the dude is not
// moved by outside things, other than the warp.
////////////////////////////////////////////////////////////////////////////////
void CDude::SetPosition(		// Returns nothing.
	double	dX,					// In:  New position for dude.
	double	dY,					// In:  New position for dude.
	double	dZ)					// In:  New position for dude.
	{
	// Set position and update crawler position.
	m_dLastCrawledToPosX	= m_dX	= dX;
	m_dY	= dY;
	m_dLastCrawledToPosZ	= m_dZ	= dZ;
	}


////////////////////////////////////////////////////////////////////////////////
// Get the next child flag item after the specified flag item.
////////////////////////////////////////////////////////////////////////////////
CFlag* CDude::GetNextFlag(			// Returns the next flag item after pflag.
	CFlag*	pflag)					// In:  The flag to get the follower of.
											// NULL for first child flag.
	{
	CFlag*	pflagNext	= NULL;

	CSprite*	psprite	= pflag ? pflag->m_sprite.m_psprNext : m_sprite.m_psprHeadChild;
	while (psprite && pflagNext == NULL)
		{
		// If this sprite names its owner . . .
		CThing*	pthing	= psprite->m_pthing;
		if (pthing)
			{
			// If it is a flag . . .
			if (pthing->GetClassID() == CFlagID)
				{
				pflagNext	= (CFlag*)pthing;
				}
			}

		// Next Please.
		psprite	= psprite->m_psprNext;
		}

	return pflagNext;
	}

////////////////////////////////////////////////////////////////////////////////
// Drop all child flag items.
////////////////////////////////////////////////////////////////////////////////
void CDude::DropAllFlags(	// Returns nothing.
	GameMessage*	pmsg)		// In:  Message to pass to flags.
	{
	GameMessage	msg;
	
	// If no message specified . . .
	if (pmsg == NULL)
		{
		msg.msg_Explosion.eType = typeExplosion;
		msg.msg_Explosion.sPriority = 0;
		msg.msg_Explosion.sDamage = 10;
		msg.msg_Explosion.sX = (short) m_dX;
		msg.msg_Explosion.sY = (short) m_dY;
		msg.msg_Explosion.sZ = (short) m_dZ;
		msg.msg_Explosion.sVelocity = 30;
		msg.msg_Explosion.u16ShooterID = GetInstanceID();

		pmsg	=	&msg;
		}
	else
		{
		// If it is an explosion message . . .
		if (pmsg->msg_Generic.eType == typeExplosion)
			{
			// Copy it so we can tweak it.
			msg	= *pmsg;
			}
		}

	// Loop through all child flags and send them a message and remove them.
	CFlag*	pflag	= GetNextFlag(NULL);
	CFlag*	pflagNext;
	while (pflag)
		{
		// Get the next now b/c this won't be a sibling after we detach it.
		pflagNext	= GetNextFlag(pflag);

		U16	u16IdFlag	= pflag->GetInstanceID();
		
		// Detach the flag.
		DetachChild(
			&u16IdFlag,
			((CDudeAnim3D*) m_panimCur)->m_ptransLeft->GetAtTime(m_lAnimTime) );

		// Move it to our position rather than the transformed position b/c we
		// don't know if that's a valid position but we do know that our position
		// is.
		pflag->m_dX	= m_dX;
		pflag->m_dY	= m_dY;
		pflag->m_dZ	= m_dZ;

		// If it's an explosion . . .
		if (pmsg->msg_Generic.eType == typeExplosion)
			{
			// Tweak the message a little.
			pmsg->msg_Explosion.sX = (short) m_dX + RAND_SWAY(30);
			pmsg->msg_Explosion.sZ = (short) m_dZ + RAND_SWAY(30);
			}

		// Forward the specified message.
		SendThingMessage(
			pmsg,
			pflag);

		// Next please.
		pflag	= pflagNext;
		}
	}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
