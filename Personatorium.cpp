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
// Personatorium.cpp
// Project: Postal
// 
// History:
//		04/29/97 JMI	Started.
//
//		04/29/97 BRH	Removed the verb from the structure since each action
//							has a verb, there needed to be more than one for
//							each person type so its better left up to the 
//							GetResources() function.  Also added 3d/ prefix
//							to the base names.
//
//		05/17/97 BRH	Added Kid, Swat, Protestor and Woman to the list.
//
//		06/13/97	JMI	Added event name for grenader, cop, gunner, rocketman,
//							and swat.
//
//		06/13/97	JMI	Changed swat to use 'machinetip' instead of 'hand' for
//							shoot link point.
//
//		06/14/97 BRH	Added more sound events
//
//		06/15/97 BRH	Picked sound effects and comments for the sound
//							events for the different people.
//
//		06/16/97 BRH	Added Hick Cop as a new person type with a differen
//							sound theme.  Also changed some other sound effects
//							now that we have some more variety.
//
//		07/01/97 BRH	Added military guy.
//
//		07/03/97 BRH	Added construction guy, fixed events for miner & 
//							construction worker.
//
//		07/04/97 BRH	Added the man and woman victims.
//
//		07/08/97 BRH	Renamed construction, grenader, gunner, military and
//							rocketman animations since some of the names were too
//							long for the delicate MacOS.
//
//		07/23/97 BRH	Added tunable values for timeouts for guard mode, 
//							time between shot reactions, and time between
//							shooting in run and shoot mode.
//
//		07/25/97 BRH	Added initial hit points for person types.
//
//		07/31/97 BRH	Fixed military and kid rigid body names and event names
//							so that they will load again.
//
//		08/07/97	JMI	Added names for all 'hand' transform channels.
//
//		08/08/97	JMI	Upgraded the rigid body weapon transforms to the correct
//							ones for each type.
//
//		08/09/97 BRH	Added more person types for more voice variety.
//
//		08/10/97 BRH	Evened out the shot talking comments so there is only
//							1 talking comment out of the 4 when possible.  Some
//							sound schemes are short on some sounds like the female
//							voices, there aren't enough reactions, too many comments.
//
//		08/11/97	JMI	Set fallback weapon types for all people.  For those
//							with throwing animations, set the fallback weapon type
//							to TotalIDs which indicates none, of course.
//
//		08/11/97	JMI	Removed fallback weapons for victims.
//
//		08/12/97 BRH	Added writhing motion type so that we know what they
//							do when the are writhing, whether it be crawling, 
//							pushing themselves on their back, or just staying in
//							place.
//
//		08/12/97	JMI	Removed pszShowWeaponEvent that we just added b/c we
//							went ahead and hid the weapon if the current main event
//							is greater than or equal to 10.
//
//		08/12/97 BRH	Added a texture scheme number to the animation struct
//							that indicates which texture file to load.  Now the
//							people only need to load 1 .tex file rather than one
//							for every different animation.
//
//		08/12/97 BRH	Updated color schemes for the people
//
//		08/13/97 BRH	Added two more people for different texture variety.
//							Also moved all shot comments to the top of the list
//							so they won't be repeated.  Added new cop sounds.
//
//		08/14/97 BRH	Made the person description names generic.
//
//		08/16/97 BRH	Fixed problems with the protestor trying to load
//							rigid bodys for the sign.  Since he is not using it
//							I made the names NULL so that all of the animations
//							would load properly, previously some of the animations
//							had the sign and others didn't.
//
//		08/18/97	JMI	Changed hitpoints as follows:
//							Persona					new hitpoints
//							============			==============
//							Swat guys				170
//							Gunner Gals				150
//							Rocket guys				200
//							Military guys			150
//							Cops (except hicks)	125
//
//		08/26/97 BRH	Put back descriptive names, and changed some of the
//							voices.  More moaning for the female characters and
//							the addition of Verne's voice for the cop.
//
//		12/17/97	JMI	Added some new persons:
//							Employee 1 & 2, Red Cross 1 & 2, Golfer 1 & 2, Lawyer,
//							Bum 1 & 2, Vet 1 & 2, Nude 1 & 2, Gramps 1 & 2, Granny
//							1, 2, & 3.
//
//		01/19/98	BRH	Added Shopper versions of Golfer, Bum and Vet so that
///						they look different but say the normal non-level
//							specific victim stuff.
//
//		01/26/98 BRH	Fixed the Lifestyle flags for the new people we added,
//							a previous copy/paste error resulted in all of the new
//							victims being counted as hostiles.
//
//		10/06/99 BRH	Added japanese characters for the Japan Addon Pack.
//							Changed the texture index values for the japanese 
//							people so that we can edit their textures without
//							changing the original textures.
//
//		10/14/99	JMI	Changed Japanese characters' names.
//
//		03/31/00 MJR	Put conditional compilation around Japanese characters.
//
//////////////////////////////////////////////////////////////////////////////
//
// Personatorium is a description of a CPerson.  There, currently, isn't too
// much customizable about each CPerson via the Personatorium, but eventually
// there should be plenty.  Also, eventually, we hope that these might just
// be used to quickly initialize a CPerson allowing it to be customized later
// via an EditModify().
//
// To add to the Personatorium:
// 1) Add an enum value to the Personatorium::Index typedef representing your
// new person.  Note that it MUST be entered just before NumPersons.
//	2) Add an entry to the end of the array of Personatoriums, g_apersons, in 
// Personatorium.cpp copying an existing entry as a template.
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// C Headers -- Must be included before RSPiX.h b/c RSPiX utilizes SHMalloc.
//////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// RSPiX Headers.
///////////////////////////////////////////////////////////////////////////////

#include "RSPiX.h"

//////////////////////////////////////////////////////////////////////////////
// Postal headers.
//////////////////////////////////////////////////////////////////////////////
#include "personatorium.h"

//////////////////////////////////////////////////////////////////////////////
// Module specific macros.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Module specific typedefs.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Instantiate exported (extern) variables.
//////////////////////////////////////////////////////////////////////////////

// This is the master list of person descriptions that can be used to describe
// a CPerson.  See header comment for details of adding a person description.
extern Personatorium g_apersons[Personatorium::NumPersons]	=
	{
	
	///////////////////////////////Grenader////////////////////////////////////
		
		{
		// User level description of person.
		"Grenader Brad",			// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/grnd",					// Example: "Gunner", as in "3d/GUNNER_shoot.sop".         
			0,								// Texture 0
			"grenade",					// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			NULL,							// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"grenader",					// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},
			
		// Sound effects for this person.
			{
			&g_smidScottMyLeg,		// ID of 'hit by weapon' sample,
			&g_smidShotGrunt,			// ID of 'hit by weapon' sample.
			&g_smidVinceHu,			// ID of 'hit by weapon' sample,
			&g_smidScottGrunt,		// ID of 'hit by weapon' sample,
			&g_smidBlownupYell,		// ID of 'blown up' sample.     
			&g_smidScottYell1,		// ID of 'blown up' sample.     
			&g_smidSteveAhFire,		// ID of 'on fire' sample
			&g_smidSteveWaFire,		// ID of 'on fire' sample
			&g_smidScottCoughBlood1,// ID of suffering sample 1.
			&g_smidWrithing2,			// ID of suffering sample 2.
			&g_smidWrithing3,			// ID of suffering sample 3.
			&g_smidWrithing4,			// ID of suffering sample 4.
			&g_smidSteveCantBreathe,// ID of dying sample.          
			&g_smidScottCoughBlood2,// ID of dying sample.          
			&g_smidScottBurn,			// ID of shooting comment.          
			&g_smidScottBurnHim,		// ID of shooting comment.
			&g_smidSteveEatThis,		// ID of shooting comment
			&g_smidSteveOneForMom,	// ID of shooting comment       
			&g_smidSteveLookout,		// ID of random comment        
			&g_smidSteveScumbag,		// ID of random comment         
			&g_smidSteveGetHim,		// ID of random comment         
			&g_smidSteveThereHeIs,	// ID of random comment         
			},
			
		// Weapons for this person.  Currently, only one supported.
			{
			CThing::TotalIDs,			// Weapon to fallback on when none available.
			},

			Personatorium::PushBack,// push along the ground on your back when writhing
			Personatorium::Hostile,	// Used for scoring to determine hostile or victim
			1000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			3000,							// time between reaction to being shot.
			100,							// Initial hit points

		},

	///////////////////////////////RocketMan///////////////////////////////////
		
		{
		// User level description of person.
		"Rocket Man Darrel",		// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/rockt",					// Example: "Gunner", as in "3d/GUNNER_shoot.sop".         
			0,								// Texture to use
			"missile",					// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"gun",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"rocket",					// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidScottMyLeg,		// ID of 'hit by weapon' sample,
			&g_smidShotGrunt,			// ID of 'hit by weapon' sample.
			&g_smidRandyUrhh,			// ID of 'hit by weapon' sample,
			&g_smidVinceHu,			// ID of 'hit by weapon' sample,
			&g_smidScottYell1,		// ID of 'blown up' sample.     
			&g_smidScottYell2,		// ID of 'blown up' sample.     
			&g_smidScottYell1,		// ID of 'on fire' sample
			&g_smidScottYell3,		// ID of 'on fire' sample
			&g_smidScottCoughBlood1,// ID of suffering sample 1.
			&g_smidScottCoughBlood2,// ID of suffering sample 2.
			&g_smidWrithing3,			// ID of suffering sample 3.
			&g_smidWrithing2,			// ID of suffering sample 4.
			&g_smidJeffCoughBlood1,	// ID of dying sample.          
			&g_smidJeffCoughBlood2,	// ID of dying sample.          
			&g_smidBillEatThis,		// ID of shooting comment.          
			&g_smidBillJustDie,		// ID of shooting comment.
			&g_smidBillEatThis,		// ID of shooting comment
			&g_smidBillJustDie,		// ID of shooting comment       
			&g_smidScottBleeding,	// ID of random comment        
			&g_smidScottDontGetAway,// ID of random comment         
			&g_smidScottWhereIsHe,  // ID of random comment         
			&g_smidScottDontGetAway,// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::CRocketID,		// Weapon to fallback on when none available.
			},

			Personatorium::Still,	// Writhing in place
			Personatorium::Hostile,	// Enemy			
			500,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			4000,							// time between reaction to being shot.
			200,							// Initial hit points

		},

	////////////////////////////////Gunner/////////////////////////////////////
		
		{
		// User level description of person.
		"Gunner Lisa",				// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/gnr",					// Example: "Gunner", as in "3d/GUNNER_shoot.sop".         
			0,								// Texture to use
			"machinetip",				// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"gun",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"gunner",					// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidAmyMyEyes,			// ID of 'hit by weapon' sample,
			&g_smidCelinaUg,			// ID of 'hit by weapon' sample.
			&g_smidShotFemaleGrunt,	// ID of 'hit by weapon' sample,
			&g_smidDebbieAh,			// ID of 'hit by weapon' sample,
			&g_smidCarynScream,		// ID of 'blown up' sample.     
			&g_smidAndreaYell,		// ID of 'blown up' sample.     
			&g_smidAndreaHelp,		// ID of 'on fire' sample
			&g_smidTinaScream3,		// ID of 'on fire' sample
			&g_smidKimHelp,			// ID of suffering sample 1.
			&g_smidKimCantBreathe,	// ID of suffering sample 2.
			&g_smidDebbieAhh,			// ID of suffering sample 3.
			&g_smidDebbieAhah,	// ID of suffering sample 4.
			&g_smidAndreaCantFeelLegs,	// ID of dying sample.          
			&g_smidAndreaCantBreathe,  // ID of dying sample.          
			&g_smidDebbieBringIt,		// ID of shooting comment.          
			&g_smidAmyEatThis,		// ID of shooting comment.
			&g_smidAndreaOneForMom,	// ID of shooting comment
			&g_smidAndreaStickThis, // ID of shooting comment       
			&g_smidAndreaThereHeIs,	// ID of random comment        
			&g_smidAndreaHesPostal, // ID of random comment         
			&g_smidAndreaNeedBackup,// ID of random comment         
			&g_smidAndreaThereHeGoes,// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::CMachineGunID,		// Weapon to fallback on when none available.
			},

			Personatorium::PushBack,// Slide on ground on back when writhing
			Personatorium::Hostile,	// Enemy
			2000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			2000,							// time between reaction to being shot.
			150,							// Initial hit points

		},

	/////////////////////////////////Cop///////////////////////////////////////
		
		{
		// User level description of person.
		"Officer Holbert",					// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/cop",					// Example: "Gunner", as in "3d/GUNNER_shoot.sop".         
			0,								// Texture to use
			"pistoltip",				// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"gun",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"cop",						// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidRubinImHit,		// ID of 'hit by weapon' sample.
			&g_smidBillGrunt,			// ID of 'hit by weapon' sample,
			&g_smidShotGrunt,			// ID of 'hit by weapon' sample,
			&g_smidRandyHuu,			// ID of 'hit by weapon' sample,
			&g_smidScottGrunt,		// ID of 'blown up' sample.     
			&g_smidScottYell1,		// ID of 'blown up' sample.     
			&g_smidScottHelp,			// ID of 'on fire' sample
			&g_smidScottYell4,		// ID of 'on fire' sample
			&g_smidBillKillMe,		// ID of suffering sample 1.
			&g_smidBillHelpMe,		// ID of suffering sample 2.
			&g_smidScottCoughBlood2,// ID of suffering sample 3.
			&g_smidWrithing3,			// ID of suffering sample 4.
			&g_smidPaulCantFeelLegs,// ID of dying sample.          
			&g_smidScottCoughBlood2,// ID of dying sample.          
			&g_smidBillTakeYouOut,  // ID of shooting comment.          
			&g_smidBillDieYouNutcase,// ID of shooting comment.
			&g_smidBillFreezePolice,// ID of shooting comment
			&g_smidBillYoureDead,	// ID of shooting comment       
			&g_smidBillDropWeapons,	// ID of random comment        
			&g_smidBillFreezePolice,// ID of random comment         
			&g_smidBillGetOnGround,	// ID of random comment         
			&g_smidBillThisIsPolice,// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::CPistolID,		// Weapon to fallback on when none available.
			},

			Personatorium::Crawl,	// Crawl on knees when writhing
			Personatorium::Hostile,	// Enemy
			3000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			2000,							// time between reaction to being shot.
			125,							// Initial hit points

		},


	///////////////////////////////Miner///////////////////////////////////////
		
		{
		// User level description of person.
		"Miner 1",							// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/miner",					// Example: "Miner", as in "3d/MINER_shoot.sop".         
			0,								// Texture to use
			"grenade",					// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			NULL,							// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"miner",						// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidPaulRBlewShoulder,// ID of 'hit by weapon' sample.
			&g_smidBillGrunt,			// ID of 'hit by weapon' sample,
			&g_smidVinceAhuh,			// ID of 'hit by weapon' sample,
			&g_smidVinceHu,			// ID of 'hit by weapon' sample,
			&g_smidScottYell1,		// ID of 'blown up' sample.     
			&g_smidScottYell2,		// ID of 'blown up' sample.     
			&g_smidScottYell3,		// ID of 'on fire' sample
			&g_smidScottYell4,		// ID of 'on fire' sample
			&g_smidWrithing3,			// ID of suffering sample 1.
			&g_smidWrithing2,			// ID of suffering sample 2.
			&g_smidWrithing3,			// ID of suffering sample 3.
			&g_smidWrithing4,			// ID of suffering sample 4.
			&g_smidJeffCoughBlood1,	// ID of dying sample.          
			&g_smidScottCantBreathe,// ID of dying sample.          
			&g_smidBillEatThis,		// ID of shooting comment.          
			&g_smidBillEatThis,		// ID of shooting comment.
			&g_smidScottBurn,			// ID of shooting comment
			&g_smidScottBurnHim,		// ID of shooting comment       
			&g_smidBillLookout,		// ID of random comment        
			&g_smidBillHesManiac,	// ID of random comment         
			&g_smidBillBleeding, 	// ID of random comment         
			&g_smidScottBumRush,		// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::TotalIDs,			// Weapon to fallback on when none available.
			},

			Personatorium::PushBack,// Slide on back when writhing
			Personatorium::Hostile,	// Enemy
			1000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			3000,							// time between reaction to being shot.
			100,							// Initial hit points

		},

	/////////////////////////////// Swat ////////////////////////////////////////

		{
		// User level description of person.
		"Swat Rick",						// pszDescription.

		// Names used to create animation resource filenames.
			{								
			"3d/swat",					// Example: "Miner", as in "3d/MINER_shoot.sop".         
			0,								// Texture to use
			"machinetip",				// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"gun",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"swat",						// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidBulletIntoVest,	// ID of 'hit by weapon' sample.
			&g_smidBillGrunt,			// ID of 'hit by weapon' sample,
			&g_smidScottGrunt,		// ID of 'hit by weapon' sample,
			&g_smidBulletIntoVest,	// ID of 'hit by weapon' sample,
			&g_smidScottGrunt,		// ID of 'blown up' sample.     
			&g_smidScottYell2,		// ID of 'blown up' sample.     
			&g_smidScottYell3,		// ID of 'on fire' sample
			&g_smidScottHelp,			// ID of 'on fire' sample
			&g_smidMikeAhuh,			// ID of suffering sample 1.
			&g_smidWrithing2,			// ID of suffering sample 2.
			&g_smidWrithing3,			// ID of suffering sample 3.
			&g_smidMikeAhuh,			// ID of suffering sample 4.
			&g_smidScottCoughBlood1,// ID of dying sample.          
			&g_smidScottCoughBlood2,// ID of dying sample.          
			&g_smidBillTakeYouOut,	// ID of shooting comment.          
			&g_smidBillYoureDead,		// ID of shooting comment.
			&g_smidBillDropWeapons,	// ID of shooting comment
			&g_smidBillEatThis,		// ID of shooting comment       
			&g_smidBillFreezePolice,// ID of random comment        
			&g_smidBillThisIsPolice,// ID of random comment         
			&g_smidBillGetOnGround,		// ID of random comment         
			&g_smidBillDropWeapons,	// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::CAutoRifleID,	// Weapon to fallback on when none available.
			},

			Personatorium::PushBack,// Slide on back when writhing
			Personatorium::Hostile,	// Enemy
			1000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			3000,							// time between reaction to being shot.
			170,							// Initial hit points

		},


	///////////////////////////////Kid/////////////////////////////////////////

		{
		// User level description of person.
		"Kid",							// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/kid",					// Example: "Miner", as in "3d/MINER_shoot.sop".         
			0,								// Texture to use
			"boypistol",				// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"gun",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"kid",						// Example: "main" as in "3d/gunner_shoot_MAIN.event"
			},

		// Sound effects for this person.
			{
			&g_smidShotGrunt,			// ID of 'hit by weapon' sample.
			&g_smidShotGrunt,			// ID of 'hit by weapon' sample,
			&g_smidShotGrunt,			// ID of 'hit by weapon' sample,
			&g_smidShotGrunt,			// ID of 'hit by weapon' sample,
			&g_smidBlownupYell,		// ID of 'blown up' sample.     
			&g_smidBlownupYell,		// ID of 'blown up' sample.     
			&g_smidBurningYell,		// ID of 'on fire' sample
			&g_smidBurningYell,		// ID of 'on fire' sample
			&g_smidWrithing3,			// ID of suffering sample 1.
			&g_smidWrithing2,			// ID of suffering sample 2.
			&g_smidWrithing3,			// ID of suffering sample 3.
			&g_smidWrithing4,			// ID of suffering sample 4.
			&g_smidNil,					// ID of dying sample.          
			&g_smidNil,					// ID of dying sample.          
			&g_smidNil,					// ID of shooting comment.          
			&g_smidNil,					// ID of shooting comment.
			&g_smidNil,					// ID of shooting comment
			&g_smidNil,					// ID of shooting comment       
			&g_smidNil,					// ID of random comment        
			&g_smidNil,					// ID of random comment         
			&g_smidNil,					// ID of random comment         
			&g_smidNil,					// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::CSmallPistolID,		// Weapon to fallback on when none available.
			},

			Personatorium::Still,	// Doesn't actually have writhing
			Personatorium::Civilian,// Victim - although never gets killed
			1000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			3000,							// time between reaction to being shot.
			50,							// Initial hit points

		},

		{
		// User level description of person.
		"Protestor Patrick",					// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/protestor",			// Example: "Miner", as in "3d/MINER_shoot.sop".         
			0,								// Texture to use
			NULL,							// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			NULL,							// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"protest",					// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidRandyCantFeelLegs,		// ID of 'hit by weapon' sample.
			&g_smidShotGrunt,			// ID of 'hit by weapon' sample,
			&g_smidRandyUg,			// ID of 'hit by weapon' sample,
			&g_smidMikeGrunt,			// ID of 'hit by weapon' sample,
			&g_smidBlownupYell,		// ID of 'blown up' sample.     
			&g_smidScottYell1,		// ID of 'blown up' sample.     
			&g_smidScottYell3,		// ID of 'on fire' sample
			&g_smidBurningYell,		// ID of 'on fire' sample
			&g_smidWrithing3,			// ID of suffering sample 1.
			&g_smidWrithing2,			// ID of suffering sample 2.
			&g_smidWrithing3,			// ID of suffering sample 3.
			&g_smidWrithing4,			// ID of suffering sample 4.
			&g_smidScottCoughBlood1,// ID of dying sample.          
			&g_smidScottCoughBlood2,// ID of dying sample.          
			&g_smidNil,					// ID of shooting comment.          
			&g_smidNil,					// ID of shooting comment.
			&g_smidNil,					// ID of shooting comment
			&g_smidNil,					// ID of shooting comment       
			&g_smidNil,					// ID of random comment        
			&g_smidNil,					// ID of random comment         
			&g_smidNil,					// ID of random comment         
			&g_smidNil,					// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::TotalIDs,			// Weapon to fallback on when none available.
			},

			Personatorium::Crawl,	// Crawl face down when writhing
			Personatorium::Civilian,// Victim
			1000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			3000,							// time between reaction to being shot.
			50,							// Initial hit points

		},

	////////////////////////////Hick Cop///////////////////////////////////////
		
		{
		// User level description of person.
		"Sheriff Bubba",						// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/cop",					// Example: "Gunner", as in "3d/GUNNER_shoot.sop".         
			1,								// Texture to use
			"pistoltip",				// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"gun",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"cop",						// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidHickPrick,			// ID of 'hit by weapon' sample.
			&g_smidHickAhh2,			// ID of 'hit by weapon' sample,
			&g_smidShotGrunt,			// ID of 'hit by weapon' sample,
			&g_smidMikeGrunt,			// ID of 'hit by weapon' sample,
			&g_smidHickAhh1,			// ID of 'blown up' sample.     
			&g_smidHickAhhPain,		// ID of 'blown up' sample.     
			&g_smidHickAhhFire,		// ID of 'on fire' sample
			&g_smidHickWhaa,			// ID of 'on fire' sample
			&g_smidHickOhoo,			// ID of suffering sample 1.
			&g_smidHickHelpCry,		// ID of suffering sample 2.
			&g_smidScottCoughBlood2,// ID of suffering sample 3.
			&g_smidMikeAhuh,			// ID of suffering sample 4.
			&g_smidScottCoughBlood1,// ID of dying sample.          
			&g_smidScottCoughBlood2,// ID of dying sample.          
			&g_smidHickScumbag,		// ID of shooting comment.          
			&g_smidHickOneForMom,	// ID of shooting comment.
			&g_smidHickRatBastard,	// ID of shooting comment
			&g_smidHickEatThis,		// ID of shooting comment       
			&g_smidHickWhatTheHell, // ID of random comment        
			&g_smidHickGetHim,		// ID of random comment         
			&g_smidHickThereHeIs,	// ID of random comment         
			&g_smidHickLookout,		// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::CPistolID,		// Weapon to fallback on when none available.
			},

			Personatorium::Crawl,	// Crawl on knees when writhing
			Personatorium::Hostile,	// Enemy
			1000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			3000,							// time between reaction to being shot.
			125,							// Initial hit points

		},

	/////////////////////////////// Military ////////////////////////////////////////

		{
		// User level description of person.
		"Military Joe",						// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/mi",						// Example: "Miner", as in "3d/MINER_shoot.sop".         
			0,								// Texture to use
			"machinetip",				// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"gun",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"military",					// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidShotGrunt,			// ID of 'hit by weapon' sample.
			&g_smidVinceHu,			// ID of 'hit by weapon' sample,
			&g_smidScottGrunt,		// ID of 'hit by weapon' sample,
			&g_smidDyingYell,			// ID of 'hit by weapon' sample,
			&g_smidScottGrunt,		// ID of 'blown up' sample.     
			&g_smidScottYell2,		// ID of 'blown up' sample.     
			&g_smidScottYell3,		// ID of 'on fire' sample
			&g_smidScottHelp,			// ID of 'on fire' sample
			&g_smidMikeAhuh,			// ID of suffering sample 1.
			&g_smidWrithing2,			// ID of suffering sample 2.
			&g_smidWrithing3,			// ID of suffering sample 3.
			&g_smidMikeAhuh,			// ID of suffering sample 4.
			&g_smidScottCoughBlood1,// ID of dying sample.          
			&g_smidScottCoughBlood2,// ID of dying sample.          
			&g_smidBillDieYouNutcase,	// ID of shooting comment.          
			&g_smidBillWhupAss,		// ID of shooting comment.
			&g_smidBillBringItOn,	// ID of shooting comment
			&g_smidBillEatThis,		// ID of shooting comment       
			&g_smidBillYoureDead,// ID of random comment        
			&g_smidScottCantShootAll,// ID of random comment         
			&g_smidScottBumRush,		// ID of random comment         
			&g_smidScottDontGetAway,// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::CAutoRifleID,		// Weapon to fallback on when none available.
			},

			Personatorium::PushBack,// Slide on back when writhing
			Personatorium::Hostile,	// Enemy
			1000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			3000,							// time between reaction to being shot.
			150,							// Initial hit points

		},


	/////////////////////////////// Construction worker //////////////////////////////////

		{
		// User level description of person.
		"Construction Mac",				// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/con",					// Example: "Miner", as in "3d/MINER_shoot.sop".         
			0,								// Texture to use
			"grenade",					// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			NULL,							// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"construct",				// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidShotGrunt,			// ID of 'hit by weapon' sample.
			&g_smidBillGrunt,			// ID of 'hit by weapon' sample,
			&g_smidScottGrunt,		// ID of 'hit by weapon' sample,
			&g_smidDyingYell,			// ID of 'hit by weapon' sample,
			&g_smidScottGrunt,		// ID of 'blown up' sample.     
			&g_smidScottYell2,		// ID of 'blown up' sample.     
			&g_smidScottYell3,		// ID of 'on fire' sample
			&g_smidScottHelp,			// ID of 'on fire' sample
			&g_smidMikeAhuh,			// ID of suffering sample 1.
			&g_smidWrithing2,			// ID of suffering sample 2.
			&g_smidWrithing3,			// ID of suffering sample 3.
			&g_smidWrithing3,			// ID of suffering sample 4.
			&g_smidScottCoughBlood1,// ID of dying sample.          
			&g_smidScottCoughBlood2,// ID of dying sample.          
			&g_smidBillConstTakeCare,	// ID of shooting comment.          
			&g_smidBillConstDieWacko,// ID of shooting comment.
			&g_smidBillConstTough,	// ID of shooting comment
			&g_smidBillEatThis,		// ID of shooting comment       
			&g_smidScottStopViolence,// ID of random comment        
			&g_smidScottCantShootAll,// ID of random comment         
			&g_smidScottBumRush,		// ID of random comment         
			&g_smidBillGoTime,		// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::TotalIDs,			// Weapon to fallback on when none available.
			},

			Personatorium::PushBack,// Slide on back when writhing
			Personatorium::Hostile,	// Enemy
			1000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			3000,							// time between reaction to being shot.
			100,							// Initial hit points

		},

	/////////////////////////////// The Man //////////////////////////////////

		{
		// User level description of person.
		"The Man",						// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/man",					// Example: "Miner", as in "3d/MINER_shoot.sop".         
			0,								// Texture to use
			"hand",						// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"hand",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"guy",						// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidVictimStopShootingAlreadyDead,	// ID of 'hit by weapon' sample.
			&g_smidVictimAuh,			// ID of 'hit by weapon' sample,
			&g_smidScottGrunt,		// ID of 'hit by weapon' sample,
			&g_smidDyingYell,			// ID of 'hit by weapon' sample,
			&g_smidVictimAAhh,		// ID of 'blown up' sample.     
			&g_smidVictimAhhk,		// ID of 'blown up' sample.     
			&g_smidScottYell3,		// ID of 'on fire' sample
			&g_smidScottHelp,			// ID of 'on fire' sample
			&g_smidVictimINeedFirstAid,			// ID of suffering sample 1.
			&g_smidVictimAnyoneSeenEar,			// ID of suffering sample 2.
			&g_smidVictimEndIsNear,			// ID of suffering sample 3.
			&g_smidVictimEndNear,			// ID of suffering sample 4.
			&g_smidVictimTheEndIsNear,// ID of dying sample.          
			&g_smidVictimAlreadyDead,// ID of dying sample.          
			&g_smidNil,					// ID of shooting comment.          
			&g_smidNil,					// ID of shooting comment.
			&g_smidNil,					// ID of shooting comment
			&g_smidNil,					// ID of shooting comment       
			&g_smidVictimJudgementDay,		// ID of random comment        
			&g_smidVictimRunForLives3,		// ID of random comment         
			&g_smidVictimWhereTVMan,	// ID of random comment         
			&g_smidVictimWhatsThat,		// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::TotalIDs,			// Weapon to fallback on when none available.
			},

			Personatorium::Crawl,	// Crawl face down when writhing
			Personatorium::Civilian,// Victim
			1000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			3000,							// time between reaction to being shot.
			50,							// Initial hit points

		},


	/////////////////////////////// Woman victim //////////////////////////////////

		{
		// User level description of person.
		"Woman",							// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/woman",					// Example: "Miner", as in "3d/MINER_shoot.sop".         
			0,								// Texture to use
			"hand",						// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"hand",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"woman",						// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidWVictimScream,			// ID of 'hit by weapon' sample.
			&g_smidShotFemaleGrunt,	// ID of 'hit by weapon' sample,
			&g_smidCelinaUg,			// ID of 'hit by weapon' sample,
			&g_smidDebbieAh,			// ID of 'hit by weapon' sample,
			&g_smidWVictimAh,		// ID of 'blown up' sample.     
			&g_smidAndreaYell,		// ID of 'blown up' sample.     
			&g_smidWVictimOhNo,		// ID of 'on fire' sample
			&g_smidTinaScream3,		// ID of 'on fire' sample
			&g_smidWVictimSeenEar,			// ID of suffering sample 1.
			&g_smidWVictimNeedAid,	// ID of suffering sample 2.
			&g_smidWVictimVeryBadDay,		// ID of suffering sample 3.
			&g_smidAmyCantBreathe,	// ID of suffering sample 4.
			&g_smidAndreaCantFeelLegs,	// ID of dying sample.          
			&g_smidAndreaCantBreathe,  // ID of dying sample.          
			&g_smidNil,					// ID of shooting comment.          
			&g_smidNil,					// ID of shooting comment.
			&g_smidNil,					// ID of shooting comment
			&g_smidNil,					// ID of shooting comment       
			&g_smidWVictimThatsIllegal,	// ID of random comment        
			&g_smidWVictimRunForLives, // ID of random comment         
			&g_smidWVictimWhatHappening2,// ID of random comment         
			&g_smidWVictimTheHorror,// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::TotalIDs,			// Weapon to fallback on when none available.
			},

			Personatorium::Crawl,	// Crawl face down when writhing
			Personatorium::Civilian,// Victim
			1000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			3000,							// time between reaction to being shot.
			50,							// Initial hit points

		},

	///////////////////////////////////////////////////////////////////////////

	///////////////////////////////  Mexican Construction worker  /////////////

		{
		// User level description of person.
		"Construction Carlos",				// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/con",					// Example: "Miner", as in "3d/MINER_shoot.sop".         
			1,								// Texture to use
			"grenade",					// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			NULL,							// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"construct",				// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidRubinImHit,			// ID of 'hit by weapon' sample.
			&g_smidBillGrunt,			// ID of 'hit by weapon' sample,
			&g_smidPaulRHuht,			// ID of 'hit by weapon' sample,
			&g_smidPaulRHuh,			// ID of 'hit by weapon' sample,
			&g_smidRubinAhHuh,		// ID of 'blown up' sample.     
			&g_smidRubinUhh,			// ID of 'blown up' sample.     
			&g_smidRubinHii,			// ID of 'on fire' sample
			&g_smidPaulRWaaahoh,		// ID of 'on fire' sample
			&g_smidRubinUhhh,			// ID of suffering sample 1.
			&g_smidRubinAhhh,			// ID of suffering sample 2.
			&g_smidRubinUhhh,			// ID of suffering sample 3.
			&g_smidPaulRWaaahoh,		// ID of suffering sample 4.
			&g_smidRubinICantMove,	// ID of dying sample.          
			&g_smidRubinUhhh,			// ID of dying sample.          
			&g_smidRubinMadallo,		// ID of shooting comment.          
			&g_smidRubinDonwemen,	// ID of shooting comment.
			&g_smidRubinVominosween,// ID of shooting comment
			&g_smidRubinGunBandito,	// ID of shooting comment       
			&g_smidRubinVominos,		// ID of random comment        
			&g_smidRubinIfenVigado, // ID of random comment         
			&g_smidRubinGamalo,		// ID of random comment         
			&g_smidRubinMatalo,		// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::TotalIDs,			// Weapon to fallback on when none available.
			},

			Personatorium::PushBack,// slide on back when writhing
			Personatorium::Hostile,	// Enemy
			1000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			3000,							// time between reaction to being shot.
			100,							// Initial hit points

		},


	///////////////////////////////  Hispanic Construction worker  /////////////

		{
		// User level description of person.
		"Construction Armando",				// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/con",					// Example: "Miner", as in "3d/MINER_shoot.sop".         
			1,								// Texture to use
			"grenade",					// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			NULL,							// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"construct",				// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidPaulRMyEye,		// ID of 'hit by weapon' sample.
			&g_smidPaulRHuh,			// ID of 'hit by weapon' sample,
			&g_smidPaulRHuht,			// ID of 'hit by weapon' sample,
			&g_smidVinceHu,			// ID of 'hit by weapon' sample,
			&g_smidRubinAhHuh,		// ID of 'blown up' sample.     
			&g_smidPaulROoh,			// ID of 'blown up' sample.     
			&g_smidPaulROooh,			// ID of 'on fire' sample
			&g_smidPaulRWaaahoh,		// ID of 'on fire' sample
			&g_smidPaulRHauu,			// ID of suffering sample 1.
			&g_smidPaulRDragMe,		// ID of suffering sample 2.
			&g_smidWrithing3,			// ID of suffering sample 3.
			&g_smidMikeAhuh,			// ID of suffering sample 4.
			&g_smidJeffCoughBlood1,	// ID of dying sample.          
			&g_smidPaulRCoughBlood,	// ID of dying sample.          
			&g_smidRubinFinishHimOff,// ID of shooting comment.          
			&g_smidRubinDonwemen,	// ID of shooting comment.
			&g_smidRubinVominosween,// ID of shooting comment
			&g_smidRubinGunBandito,	// ID of shooting comment       
			&g_smidRubinVominos,		// ID of random comment        
			&g_smidRubinIfenVigado, // ID of random comment         
			&g_smidRubinThatEnough,	// ID of random comment         
			&g_smidPaulRComingWay,	// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::TotalIDs,			// Weapon to fallback on when none available.
			},

			Personatorium::PushBack,// slide on back when writhing
			Personatorium::Hostile,	// Enemy
			1000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			3000,							// time between reaction to being shot.
			100,							// Initial hit points

		},


	///////////////////////////////Grenader////////////////////////////////////
		
		{
		// User level description of person.
		"Grenader Pepe",			// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/grnd",					// Example: "Gunner", as in "3d/GUNNER_shoot.sop".         
			2,								// Texture to use
			"grenade",					// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			NULL,							// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"grenader",					// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},
			
		// Sound effects for this person.
			{
			&g_smidPaulRBlewShoulder,// ID of 'hit by weapon' sample.
			&g_smidVinceHu,			// ID of 'hit by weapon' sample,
			&g_smidPaulRHuh,			// ID of 'hit by weapon' sample,
			&g_smidPaulRHuht,			// ID of 'hit by weapon' sample,
			&g_smidPaulROoh,			// ID of 'blown up' sample.     
			&g_smidPaulROooh,			// ID of 'blown up' sample.     
			&g_smidPaulRHauu,			// ID of 'on fire' sample
			&g_smidPaulRWaaahoh,		// ID of 'on fire' sample
			&g_smidPaulRCoughBlood, // ID of suffering sample 1.
			&g_smidPaulRWaaahoh,		// ID of suffering sample 2.
			&g_smidWrithing3,			// ID of suffering sample 3.
			&g_smidMikeAhuh,			// ID of suffering sample 4.
			&g_smidPaulRTooYoung,   // ID of dying sample.          
			&g_smidRubinUhhh,			// ID of dying sample.          
			&g_smidRubinDonwemen,	// ID of shooting comment.          
			&g_smidRubinCallo,		// ID of shooting comment.
			&g_smidRubinFinishHimOff,// ID of shooting comment
			&g_smidRubinGudelet,		// ID of shooting comment       
			&g_smidPaulRComingWay,	// ID of random comment        
			&g_smidRubinVominos,		// ID of random comment         
			&g_smidRubinThatEnough,	// ID of random comment         
			&g_smidRubinMatalo,		// ID of random comment         
			},
			
		// Weapons for this person.  Currently, only one supported.
			{
			CThing::TotalIDs,			// Weapon to fallback on when none available.
			},

			Personatorium::PushBack,// slide on back when writhing
			Personatorium::Hostile,	// Enemy
			1000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			3000,							// time between reaction to being shot.
			100,							// Initial hit points

		},

	///////////////////////////////Grenader////////////////////////////////////
		
		{
		// User level description of person.
		"Grenader Larry",			// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/grnd",					// Example: "Gunner", as in "3d/GUNNER_shoot.sop".         
			0,								// Texture to use
			"grenade",					// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			NULL,							// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"grenader",					// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},
			
		// Sound effects for this person.
			{
			&g_smidRandyHuu,			// ID of 'hit by weapon' sample.
			&g_smidRubinAh,			// ID of 'hit by weapon' sample,
			&g_smidRandyUg,			// ID of 'hit by weapon' sample,
			&g_smidRandyUrhh,			// ID of 'hit by weapon' sample,
			&g_smidMikeOhh,			// ID of 'blown up' sample.     
			&g_smidPaulAhah,			// ID of 'blown up' sample.     
			&g_smidMikeAhh,			// ID of 'on fire' sample
			&g_smidRubinHii,			// ID of 'on fire' sample
			&g_smidScottCoughBlood1,// ID of suffering sample 1.
			&g_smidScottCoughBlood2,// ID of suffering sample 2.
			&g_smidPaulRWaaahoh,		// ID of suffering sample 3.
			&g_smidWrithing3,			// ID of suffering sample 4.
			&g_smidBillHelpMe,		// ID of dying sample.          
			&g_smidJeffCoughBlood1, // ID of dying sample.          
			&g_smidBillEatThis,		// ID of shooting comment.          
			&g_smidScottBurnHim,		// ID of shooting comment.
			&g_smidSteveEatThis,		// ID of shooting comment
			&g_smidSteveOneForMom,	// ID of shooting comment       
			&g_smidPaulHelpCall,		// ID of random comment        
			&g_smidSteveHeWentThat,	// ID of random comment         
			&g_smidSteveLookout,		// ID of random comment         
			&g_smidSteveThereHeIs,	// ID of random comment         
			},
			
		// Weapons for this person.  Currently, only one supported.
			{
			CThing::TotalIDs,			// Weapon to fallback on when none available.
			},

			Personatorium::PushBack,// slide on back when writhing
			Personatorium::Hostile,	// Enemy
			1000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			3000,							// time between reaction to being shot.
			100,							// Initial hit points

		},

	///////////////////////////////RocketMan///////////////////////////////////
		
		{
		// User level description of person.
		"Rocket Man Dexter",		// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/rockt",					// Example: "Gunner", as in "3d/GUNNER_shoot.sop".         
			0,								// Texture to use
			"missile",					// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"gun",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"rocket",					// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidShotGrunt,			// ID of 'hit by weapon' sample.
			&g_smidSteveUh,			// ID of 'hit by weapon' sample,
			&g_smidMikeGrunt, 		// ID of 'hit by weapon' sample,
			&g_smidVinceHu,			// ID of 'hit by weapon' sample,
			&g_smidSteveAhBlowup,	// ID of 'blown up' sample.     
			&g_smidMikeOhh,			// ID of 'blown up' sample.     
			&g_smidMikeAhh,   		// ID of 'on fire' sample
			&g_smidSteveAhFire,		// ID of 'on fire' sample
			&g_smidScottCoughBlood1,// ID of suffering sample 1.
			&g_smidScottCoughBlood2,// ID of suffering sample 2.
			&g_smidWrithing2,			// ID of suffering sample 3.
			&g_smidWrithing3,			// ID of suffering sample 4.
			&g_smidJeffCoughBlood1,	// ID of dying sample.          
			&g_smidJeffCoughBlood2,	// ID of dying sample.          
			&g_smidSteveEatThis,		// ID of shooting comment.          
			&g_smidSteveOneForMom,  // ID of shooting comment.
			&g_smidSteveOneForMom,	// ID of shooting comment
			&g_smidSteveEatThis,		// ID of shooting comment       
			&g_smidScottBleeding,	// ID of random comment        
			&g_smidSteveGetHim,     // ID of random comment         
			&g_smidBillLookout,		// ID of random comment         
			&g_smidSteveScumbag,		// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::CRocketID,		// Weapon to fallback on when none available.
			},
	
			Personatorium::Still,	// Stay in place when writhing
			Personatorium::Hostile,	// Enemy
			500,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			4000,							// time between reaction to being shot.
			200,							// Initial hit points

		},


	///////////////////////////////RocketMan///////////////////////////////////
		
		{
		// User level description of person.
		"Rocket Man Pedro",				// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/rockt",					// Example: "Gunner", as in "3d/GUNNER_shoot.sop".         
			1,								// Texture to use
			"missile",					// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"gun",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"rocket",					// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidPaulRShotToe,		// ID of 'hit by weapon' sample,
			&g_smidPaulRHuh,			// ID of 'hit by weapon' sample.
			&g_smidPaulRHuht,			// ID of 'hit by weapon' sample,
			&g_smidPaulRHuh,			// ID of 'hit by weapon' sample,
			&g_smidPaulROoh,			// ID of 'blown up' sample.     
			&g_smidPaulROooh,			// ID of 'blown up' sample.     
			&g_smidPaulRWaaahoh,		// ID of 'on fire' sample
			&g_smidRubinHii,			// ID of 'on fire' sample
			&g_smidWrithing3,			// ID of suffering sample 1.
			&g_smidPaulRDragMe,		// ID of suffering sample 2.
			&g_smidPaulRWaaahoh,		// ID of suffering sample 3.
			&g_smidMikeAhuh,			// ID of suffering sample 4.
			&g_smidPaulRTooYoung,	// ID of dying sample.          
			&g_smidPaulRWaaahoh,		// ID of dying sample.          
			&g_smidRubinGudelet,		// ID of shooting comment.          
			&g_smidRubinGunBandito,	// ID of shooting comment.
			&g_smidRubinMadallo,		// ID of shooting comment
			&g_smidRubinDonwemen,	// ID of shooting comment       
			&g_smidRubinIfenVigado,	// ID of random comment        
			&g_smidRubinGamalo,		// ID of random comment         
			&g_smidRubinVominos,		// ID of random comment         
			&g_smidRubinMatalo,		// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::CRocketID,		// Weapon to fallback on when none available.
			},

			Personatorium::Still,	// Stay in place when writhing
			Personatorium::Hostile,	// Enemy
			500,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			4000,							// time between reaction to being shot.
			200,							// Initial hit points

		},

	////////////////////////////////Gunner/////////////////////////////////////
		
		{
		// User level description of person.
		"Gunner Heidi",				// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/gnr",					// Example: "Gunner", as in "3d/GUNNER_shoot.sop".         
			0,								// Texture to use
			"machinetip",				// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"gun",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"gunner",					// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidAndreaMyLeg,		// ID of 'hit by weapon' sample,
			&g_smidCelinaUg,			// ID of 'hit by weapon' sample.
			&g_smidShotFemaleGrunt,	// ID of 'hit by weapon' sample,
			&g_smidDebbieAh,			// ID of 'hit by weapon' sample,
			&g_smidDebbieOh,			// ID of 'blown up' sample.     
			&g_smidTinaScream1,		// ID of 'blown up' sample.     
			&g_smidTinaScream2,		// ID of 'on fire' sample
			&g_smidAmyScream,			// ID of 'on fire' sample
			&g_smidKimHelp,			// ID of suffering sample 1.
			&g_smidDebbieAhh,			// ID of suffering sample 2.
			&g_smidAmyCantSee,		// ID of suffering sample 3.
			&g_smidDebbieAh,			// ID of suffering sample 4.
			&g_smidAndreaCantFeelLegs,	// ID of dying sample.          
			&g_smidAndreaCantBreathe,  // ID of dying sample.          
			&g_smidDebbieBringIt,	// ID of shooting comment.          
			&g_smidDebbieDontMakeUs,// ID of shooting comment.
			&g_smidAndreaOneForMom,	// ID of shooting comment
			&g_smidAndreaStickThis, // ID of shooting comment       
			&g_smidAndreaThereHeIs,	// ID of random comment        
			&g_smidAndreaHesPostal, // ID of random comment         
			&g_smidAndreaNeedBackup,// ID of random comment         
			&g_smidAndreaWheresBackup,// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::CMachineGunID,		// Weapon to fallback on when none available.
			},

			Personatorium::PushBack,// slide on back when writhing
			Personatorium::Hostile,	// Enemy
			2000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			2000,							// time between reaction to being shot.
			150,							// Initial hit points

		},


	////////////////////////////////Gunner/////////////////////////////////////
		
		{
		// User level description of person.
		"Gunner Linda",				// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/gnr",					// Example: "Gunner", as in "3d/GUNNER_shoot.sop".         
			0,								// Texture to use
			"machinetip",				// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"gun",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"gunner",					// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidAndreaMyLeg,		// ID of 'hit by weapon' sample,
			&g_smidCelinaUg,			// ID of 'hit by weapon' sample.
			&g_smidCelinaUg,			// ID of 'hit by weapon' sample,
			&g_smidDebbieAh,			// ID of 'hit by weapon' sample,
			&g_smidCarynScream,		// ID of 'blown up' sample.     
			&g_smidAndreaYell,		// ID of 'blown up' sample.     
			&g_smidAndreaHelp,		// ID of 'on fire' sample
			&g_smidTinaScream3,		// ID of 'on fire' sample
			&g_smidKimHelp,			// ID of suffering sample 1.
			&g_smidKimCantBreathe,	// ID of suffering sample 2.
			&g_smidAmyCantSee,		// ID of suffering sample 3.
			&g_smidAmyCantBreathe,	// ID of suffering sample 4.
			&g_smidAndreaCantFeelLegs,	// ID of dying sample.          
			&g_smidAndreaCantBreathe,  // ID of dying sample.          
			&g_smidDebbieDontMakeUs,	// ID of shooting comment.          
			&g_smidDebbieDropWeapons,	// ID of shooting comment.
			&g_smidAndreaOneForMom,	// ID of shooting comment
			&g_smidAndreaStickThis, // ID of shooting comment       
			&g_smidAndreaThereHeIs,	// ID of random comment        
			&g_smidCelinaRun,			 // ID of random comment         
			&g_smidAmyLookout,		// ID of random comment         
			&g_smidAmyWhatThe,		// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::CMachineGunID,		// Weapon to fallback on when none available.
			},

			Personatorium::PushBack,// slide on back when writhing
			Personatorium::Hostile,	// Enemy
			2000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			2000,							// time between reaction to being shot.
			150,							// Initial hit points

		},


	/////////////////////////////////Cop///////////////////////////////////////
		
		{
		// User level description of person.
		"Officer Smith",					// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/cop",					// Example: "Gunner", as in "3d/GUNNER_shoot.sop".         
			0,								// Texture to use
			"pistoltip",				// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"gun",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"cop",						// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidMikeGrunt,			// ID of 'hit by weapon' sample.
			&g_smidRubinAh,			// ID of 'hit by weapon' sample,
			&g_smidRandyHuu,			// ID of 'hit by weapon' sample,
			&g_smidRandyUg,			// ID of 'hit by weapon' sample,
			&g_smidPaulAhah,			// ID of 'blown up' sample.     
			&g_smidPaulROoh,			// ID of 'blown up' sample.     
			&g_smidPaulRHauu,			// ID of 'on fire' sample
			&g_smidPaulRWaaahoh,		// ID of 'on fire' sample
			&g_smidMikeAhuh,			// ID of suffering sample 1.
			&g_smidJeffCoughBlood1,	// ID of suffering sample 2.
			&g_smidScottCoughBlood2,// ID of suffering sample 3.
			&g_smidWrithing2,			// ID of suffering sample 4.
			&g_smidJeffCoughBlood1, // ID of dying sample.          
			&g_smidJeffCoughBlood2, // ID of dying sample.          
			&g_smidVernPutHandsUp,	// ID of shooting comment.          
			&g_smidVernStopFreeze,	// ID of shooting comment.
			&g_smidVernDontMove,		// ID of shooting comment
			&g_smidVernFreezeYouDirtbag,	// ID of shooting comment       
			&g_smidVernDontYouMove, // ID of random comment        
			&g_smidVernGoKneesStop,	// ID of random comment         
			&g_smidVernHeyGetOverHere,// ID of random comment         
			&g_smidVernWhereYouGoing,// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::CPistolID,		// Weapon to fallback on when none available.
			},

			Personatorium::Crawl,	// Crawl on knees when writhing
			Personatorium::Hostile,	// Enemy
			3000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			2000,							// time between reaction to being shot.
			125,							// Initial hit points

		},


	/////////////////////////////////Cop///////////////////////////////////////
		
		{
		// User level description of person.
		"Officer Ramirez",					// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/cop",					// Example: "Gunner", as in "3d/GUNNER_shoot.sop".         
			2,								// Texture to use
			"pistoltip",				// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"gun",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"cop",						// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidRubinImHit,		// ID of 'hit by weapon' sample,
			&g_smidRandyUrhh,			// ID of 'hit by weapon' sample.
			&g_smidPaulRHuh,			// ID of 'hit by weapon' sample,
			&g_smidPaulRHuht,			// ID of 'hit by weapon' sample,
			&g_smidRubinAhHuh,		// ID of 'blown up' sample.     
			&g_smidRubinUhh,			// ID of 'blown up' sample.     
			&g_smidRubinHii,			// ID of 'on fire' sample
			&g_smidPaulRWaaahoh,		// ID of 'on fire' sample
			&g_smidPaulRDragMe,		// ID of suffering sample 1.
			&g_smidWrithing2,			// ID of suffering sample 2.
			&g_smidMikeAhuh,			// ID of suffering sample 3.
			&g_smidWrithing3,			// ID of suffering sample 4.
			&g_smidRubinICantMove,	// ID of dying sample.          
			&g_smidRubinUhhh,			// ID of dying sample.          
			&g_smidRubinDonwemen,   // ID of shooting comment.          
			&g_smidRubinGunBandito,	// ID of shooting comment.
			&g_smidRubinGudelet,		// ID of shooting comment
			&g_smidRubinCallo,		// ID of shooting comment       
			&g_smidRubinVominos,		// ID of random comment        
			&g_smidRubinThatEnough,	// ID of random comment         
			&g_smidRubinIfenVigado,	// ID of random comment         
			&g_smidRubinMatalo,		// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::CPistolID,		// Weapon to fallback on when none available.
			},

			Personatorium::Crawl,	// Crawl on knees when writhing
			Personatorium::Hostile,	// Enemy
			3000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			2000,							// time between reaction to being shot.
			125,							// Initial hit points

		},


	///////////////////////////////Miner///////////////////////////////////////
		
		{
		// User level description of person.
		"Miner Gary",							// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/miner",					// Example: "Miner", as in "3d/MINER_shoot.sop".         
			0,								// Texture to use
			"grenade",					// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			NULL,							// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"miner",						// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidRandyHuu,			// ID of 'hit by weapon' sample.
			&g_smidBillGrunt,			// ID of 'hit by weapon' sample,
			&g_smidRandyUg,			// ID of 'hit by weapon' sample,
			&g_smidRandyUrhh,			// ID of 'hit by weapon' sample,
			&g_smidMikeOhh,			// ID of 'blown up' sample.     
			&g_smidPaulAhah,			// ID of 'blown up' sample.     
			&g_smidMikeAhh,			// ID of 'on fire' sample
			&g_smidRubinHii,			// ID of 'on fire' sample
			&g_smidScottCoughBlood1,// ID of suffering sample 1.
			&g_smidScottCoughBlood2,// ID of suffering sample 2.
			&g_smidPaulRWaaahoh,		// ID of suffering sample 3.
			&g_smidMikeAhuh,			// ID of suffering sample 4.
			&g_smidBillHelpMe,		// ID of dying sample.          
			&g_smidJeffCoughBlood1, // ID of dying sample.          
			&g_smidBillEatThis,		// ID of shooting comment.          
			&g_smidScottBurnHim,		// ID of shooting comment.
			&g_smidSteveEatThis,		// ID of shooting comment
			&g_smidSteveOneForMom,	// ID of shooting comment       
			&g_smidPaulHelpCall,		// ID of random comment        
			&g_smidSteveHeWentThat,	// ID of random comment         
			&g_smidSteveLookout,		// ID of random comment         
			&g_smidSteveThereHeIs,	// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::TotalIDs,			// Weapon to fallback on when none available.
			},

			Personatorium::PushBack,// slide on back when writhing
			Personatorium::Hostile,	// Enemy
			1000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			3000,							// time between reaction to being shot.
			100,							// Initial hit points

		},


	///////////////////////////////Miner///////////////////////////////////////
		
		{
		// User level description of person.
		"Miner Fernando",							// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/miner",					// Example: "Miner", as in "3d/MINER_shoot.sop".         
			1,								// Texture to use
			"grenade",					// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			NULL,							// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"miner",						// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidPaulRBlewHip,		// ID of 'hit by weapon' sample.
			&g_smidPaulRHuh,			// ID of 'hit by weapon' sample,
			&g_smidPaulRHuht,			// ID of 'hit by weapon' sample,
			&g_smidVinceHu,			// ID of 'hit by weapon' sample,
			&g_smidRubinAhHuh,		// ID of 'blown up' sample.     
			&g_smidPaulROoh,			// ID of 'blown up' sample.     
			&g_smidPaulROooh,			// ID of 'on fire' sample
			&g_smidPaulRWaaahoh,		// ID of 'on fire' sample
			&g_smidPaulRHauu,			// ID of suffering sample 1.
			&g_smidPaulRWaaahoh,		// ID of suffering sample 2.
			&g_smidPaulRDontThink,	// ID of suffering sample 3.
			&g_smidWrithing3,			// ID of suffering sample 4.
			&g_smidPaulRWaaahoh,		// ID of dying sample.          
			&g_smidPaulRCoughBlood,	// ID of dying sample.          
			&g_smidRubinFinishHimOff,// ID of shooting comment.          
			&g_smidRubinDonwemen,	// ID of shooting comment.
			&g_smidRubinVominosween,// ID of shooting comment
			&g_smidRubinGunBandito,	// ID of shooting comment       
			&g_smidRubinVominos,		// ID of random comment        
			&g_smidRubinIfenVigado, // ID of random comment         
			&g_smidRubinThatEnough,	// ID of random comment         
			&g_smidPaulRComingWay,	// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::TotalIDs,		// Weapon to fallback on when none available.
			},

			Personatorium::PushBack,// slide on back when writhing
			Personatorium::Hostile,	// Enemy
			1000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			3000,							// time between reaction to being shot.
			100,							// Initial hit points

		},

	/////////////////////////////// Swat ////////////////////////////////////////

		{
		// User level description of person.
		"Swat Charlie",				// pszDescription.

		// Names used to create animation resource filenames.
			{								
			"3d/swat",					// Example: "Miner", as in "3d/MINER_shoot.sop".         
			0,								// Texture to use
			"machinetip",				// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"gun",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"swat",						// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidRubinImHit,		// ID of 'hit by weapon' sample,
			&g_smidBulletIntoVest,  // ID of 'hit by weapon' sample.
			&g_smidBillGrunt,			// ID of 'hit by weapon' sample,
			&g_smidBulletIntoVest,	// ID of 'hit by weapon' sample,
			&g_smidScottGrunt,		// ID of 'blown up' sample.     
			&g_smidScottYell2,		// ID of 'blown up' sample.     
			&g_smidScottYell3,		// ID of 'on fire' sample
			&g_smidScottHelp,			// ID of 'on fire' sample
			&g_smidMikeAhuh,			// ID of suffering sample 1.
			&g_smidWrithing2,			// ID of suffering sample 2.
			&g_smidWrithing3,			// ID of suffering sample 3.
			&g_smidMikeAhuh,			// ID of suffering sample 4.
			&g_smidScottCoughBlood1,// ID of dying sample.          
			&g_smidScottCoughBlood2,// ID of dying sample.          
			&g_smidBillBringItOn,	// ID of shooting comment.          
			&g_smidBillDieYouNutcase,// ID of shooting comment.
			&g_smidBillBetterHope,	// ID of shooting comment
			&g_smidBillEatThis,		// ID of shooting comment       
			&g_smidBillThisIsPolice,// ID of random comment        
			&g_smidBillGetOnGround,// ID of random comment         
			&g_smidBillWhupAss,		// ID of random comment         
			&g_smidBillGoTime,		// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::CAutoRifleID,		// Weapon to fallback on when none available.
			},

			Personatorium::PushBack,// slide on back when writhing
			Personatorium::Hostile,	// Enemy
			1000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			3000,							// time between reaction to being shot.
			170,							// Initial hit points

		},

	/////////////////////////////// Swat ////////////////////////////////////////

		{
		// User level description of person.
		"Swat Raymond",				// pszDescription.

		// Names used to create animation resource filenames.
			{								
			"3d/swat",					// Example: "Miner", as in "3d/MINER_shoot.sop".         
			0,								// Texture to use
			"machinetip",				// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"gun",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"swat",						// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidPaulRShotToe,		// ID of 'hit by weapon' sample,
			&g_smidBulletIntoVest,	// ID of 'hit by weapon' sample.
			&g_smidVinceHu,			// ID of 'hit by weapon' sample,
			&g_smidBulletIntoVest,	// ID of 'hit by weapon' sample,
			&g_smidScottGrunt,		// ID of 'blown up' sample.     
			&g_smidScottYell2,		// ID of 'blown up' sample.     
			&g_smidScottYell3,		// ID of 'on fire' sample
			&g_smidScottHelp,			// ID of 'on fire' sample
			&g_smidMikeAhuh,			// ID of suffering sample 1.
			&g_smidWrithing2,			// ID of suffering sample 2.
			&g_smidWrithing3,			// ID of suffering sample 3.
			&g_smidWrithing4,			// ID of suffering sample 4.
			&g_smidScottCoughBlood1,// ID of dying sample.          
			&g_smidScottCoughBlood2,// ID of dying sample.          
			&g_smidVernPutHandsUp,	// ID of shooting comment.          
			&g_smidVernStopFreeze,	// ID of shooting comment.
			&g_smidVernDontMove,		// ID of shooting comment
			&g_smidVernFreezeYouDirtbag,	// ID of shooting comment       
			&g_smidVernDontYouMove, // ID of random comment        
			&g_smidVernGoKneesStop,	// ID of random comment         
			&g_smidVernHeyGetOverHere,// ID of random comment         
			&g_smidVernWhereYouGoing,// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::CAutoRifleID,		// Weapon to fallback on when none available.
			},

			Personatorium::PushBack,// slide on back when writhing
			Personatorium::Hostile,	// Enemy
			1000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			3000,							// time between reaction to being shot.
			170,							// Initial hit points

		},

	////////////////////////////Hick Cop///////////////////////////////////////
		
		{
		// User level description of person.
		"Sheriff Dale",						// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/cop",					// Example: "Gunner", as in "3d/GUNNER_shoot.sop".         
			1,								// Texture to use
			"pistoltip",				// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"gun",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"cop",						// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidHickAhh2,			// ID of 'hit by weapon' sample.
			&g_smidHickPrick,			// ID of 'hit by weapon' sample,
			&g_smidShotGrunt,			// ID of 'hit by weapon' sample,
			&g_smidMikeGrunt,			// ID of 'hit by weapon' sample,
			&g_smidHickAhh1,			// ID of 'blown up' sample.     
			&g_smidHickAhhPain,		// ID of 'blown up' sample.     
			&g_smidHickAhhFire,		// ID of 'on fire' sample
			&g_smidHickWhaa,			// ID of 'on fire' sample
			&g_smidHickOhoo,			// ID of suffering sample 1.
			&g_smidHickHelpCry,		// ID of suffering sample 2.
			&g_smidScottCoughBlood2,// ID of suffering sample 3.
			&g_smidMikeAhuh,			// ID of suffering sample 4.
			&g_smidScottCoughBlood1,// ID of dying sample.          
			&g_smidScottCoughBlood2,// ID of dying sample.          
			&g_smidHickBastard,		// ID of shooting comment.          
			&g_smidHickOneForMom,	// ID of shooting comment.
			&g_smidHickScumbag,		// ID of shooting comment
			&g_smidHickEatThis,		// ID of shooting comment       
			&g_smidHickWhatTheHell, // ID of random comment        
			&g_smidHickGetHim,		// ID of random comment         
			&g_smidHickThereHeIs,	// ID of random comment         
			&g_smidHickLookout,		// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::CPistolID,		// Weapon to fallback on when none available.
			},

			Personatorium::Crawl,	// Crawl on knees when writhing
			Personatorium::Hostile,	// Enemy
			1000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			3000,							// time between reaction to being shot.
			100,							// Initial hit points

		},

	/////////////////////////////// The Man //////////////////////////////////

		{
		// User level description of person.
		"Man Frank",						// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/man",					// Example: "Miner", as in "3d/MINER_shoot.sop".         
			1,								// Texture to use
			"hand",						// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"hand",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"guy",						// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidVictimOwMyEye,		// ID of 'hit by weapon' sample.
			&g_smidVinceHu,			// ID of 'hit by weapon' sample,
			&g_smidScottGrunt,		// ID of 'hit by weapon' sample,
			&g_smidVictimAhu,			// ID of 'hit by weapon' sample,
			&g_smidVictimAhhk,		// ID of 'blown up' sample.     
			&g_smidVictimAhh,		// ID of 'blown up' sample.     
			&g_smidVictimAuh,		// ID of 'on fire' sample
			&g_smidScottHelp,			// ID of 'on fire' sample
			&g_smidVictimFirstAid2,			// ID of suffering sample 1.
			&g_smidVictimOwMyEyes,			// ID of suffering sample 2.
			&g_smidVictimAnyoneSeenEar,		// ID of suffering sample 3.
			&g_smidVictimStopAlreadyDead,			// ID of suffering sample 4.
			&g_smidVictimTheHorror,// ID of dying sample.          
			&g_smidVictimAhSheAi,// ID of dying sample.          
			&g_smidNil,					// ID of shooting comment.          
			&g_smidNil,					// ID of shooting comment.
			&g_smidNil,					// ID of shooting comment
			&g_smidNil,					// ID of shooting comment       
			&g_smidVictimRunForLives4,		// ID of random comment        
			&g_smidVictimWhereTV2,		// ID of random comment         
			&g_smidVictimAskHim,	// ID of random comment         
			&g_smidVictimAhHesGotAGun2,		// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::TotalIDs,			// Weapon to fallback on when none available.
			},

			Personatorium::Crawl,	// Crawl face down when writhing
			Personatorium::Civilian,// Victim
			1000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			3000,							// time between reaction to being shot.
			50,							// Initial hit points

		},


	/////////////////////////////// Woman victim //////////////////////////////////

		{
		// User level description of person.
		"Woman Mindy",							// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/woman",					// Example: "Miner", as in "3d/MINER_shoot.sop".         
			1,								// Texture to use
			"hand",						// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"hand",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"woman",						// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidWVictimMyEyes,	// ID of 'hit by weapon' sample.
			&g_smidShotFemaleGrunt,	// ID of 'hit by weapon' sample,
			&g_smidCelinaUg,			// ID of 'hit by weapon' sample,
			&g_smidDebbieAh,			// ID of 'hit by weapon' sample,
			&g_smidCarynScream,		// ID of 'blown up' sample.     
			&g_smidWVictimScream,		// ID of 'blown up' sample.     
			&g_smidWVictimHelpHelp,		// ID of 'on fire' sample
			&g_smidWVictimOhNo,		// ID of 'on fire' sample
			&g_smidKimHelp,			// ID of suffering sample 1.
			&g_smidWVictimSeenEar,	// ID of suffering sample 2.
			&g_smidWVictimNeedFirstAid,		// ID of suffering sample 3.
			&g_smidWVictimGetOffMe,	// ID of suffering sample 4.
			&g_smidWVictimHelpHelp,	// ID of dying sample.          
			&g_smidWVictimGonnaSue,  // ID of dying sample.          
			&g_smidNil,					// ID of shooting comment.          
			&g_smidNil,					// ID of shooting comment.
			&g_smidNil,					// ID of shooting comment
			&g_smidNil,					// ID of shooting comment       
			&g_smidWVictimAhGotGun,	// ID of random comment        
			&g_smidWVictimKillThePsyco, // ID of random comment         
			&g_smidWVictimVeryBadDay,// ID of random comment         
			&g_smidWVictimTheHorror,// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::TotalIDs,			// Weapon to fallback on when none available.
			},

			Personatorium::Crawl,	// Crawl face down when writhing
			Personatorium::Civilian,// Victim
			1000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			3000,							// time between reaction to being shot.
			50,							// Initial hit points

		},


	/////////////////////////////////Cop///////////////////////////////////////
		
		{
		// User level description of person.
		"Sheriff Norm",					// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/cop",					// Example: "Gunner", as in "3d/GUNNER_shoot.sop".         
			1,								// Texture to use
			"pistoltip",				// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"gun",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"cop",						// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidRubinImHit,		// ID of 'hit by weapon' sample.
			&g_smidBillGrunt,			// ID of 'hit by weapon' sample,
			&g_smidShotGrunt,			// ID of 'hit by weapon' sample,
			&g_smidRandyHuu,			// ID of 'hit by weapon' sample,
			&g_smidScottGrunt,		// ID of 'blown up' sample.     
			&g_smidScottYell1,		// ID of 'blown up' sample.     
			&g_smidScottHelp,			// ID of 'on fire' sample
			&g_smidScottYell4,		// ID of 'on fire' sample
			&g_smidBillKillMe,		// ID of suffering sample 1.
			&g_smidBillHelpMe,		// ID of suffering sample 2.
			&g_smidScottCoughBlood2,// ID of suffering sample 3.
			&g_smidWrithing3,			// ID of suffering sample 4.
			&g_smidPaulCantFeelLegs,// ID of dying sample.          
			&g_smidScottCoughBlood2,// ID of dying sample.          
			&g_smidBillTakeYouOut,  // ID of shooting comment.          
			&g_smidBillDieYouNutcase,// ID of shooting comment.
			&g_smidBillFreezePolice,// ID of shooting comment
			&g_smidBillYoureDead,	// ID of shooting comment       
			&g_smidBillDropWeapons,	// ID of random comment        
			&g_smidBillFreezePolice,// ID of random comment         
			&g_smidBillGetOnGround,	// ID of random comment         
			&g_smidBillThisIsPolice,// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::CPistolID,		// Weapon to fallback on when none available.
			},

			Personatorium::Crawl,	// Crawl on knees when writhing
			Personatorium::Hostile,	// Enemy
			3000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			2000,							// time between reaction to being shot.
			100,							// Initial hit points

		},

	////////////////////////////////Employee/////////////////////////////////////
		
		{
		// User level description of person.
		"Employee 1",					// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/emp2",					// Example: "Gunner", as in "3d/GUNNER_shoot.sop".         
			0,							// Texture to use
			"hand",						// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"hand",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"employ2",					// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidWalMartDontShoot,// ID of 'hit by weapon' sample.
			&g_smidBillGrunt,			// ID of 'hit by weapon' sample,
			&g_smidShotGrunt,			// ID of 'hit by weapon' sample,
			&g_smidRandyHuu,			// ID of 'hit by weapon' sample,
			&g_smidScottGrunt,		// ID of 'blown up' sample.     
			&g_smidWalMartCleanupAisle3,// ID of 'blown up' sample.     
			&g_smidScottHelp,			// ID of 'on fire' sample
			&g_smidScottYell4,		// ID of 'on fire' sample
			&g_smidWalMartDontShoot3,// ID of suffering sample 1.
			&g_smidWalMartDontShoot,// ID of suffering sample 2.
			&g_smidWalMartDontShoot2,// ID of suffering sample 3.
			&g_smidWrithing3,			// ID of suffering sample 4.
			&g_smidPaulCantFeelLegs,// ID of dying sample.          
			&g_smidScottCoughBlood2,// ID of dying sample.          
			&g_smidWalMartGetMyManager,  // ID of shooting comment.          
			&g_smidWalMartShellsSpecial,// ID of shooting comment.
			&g_smidWalMartCleanupAisle2,// ID of shooting comment
			&g_smidWalMartHelpYa,	// ID of shooting comment       
			&g_smidWalMartShellsSpecial,	// ID of random comment        
			&g_smidWalMartNoWaiting4,// ID of random comment         
			&g_smidWalMartCleanupAisle4,	// ID of random comment         
			&g_smidWalMartHelpYa,// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::TotalIDs,		// Weapon to fallback on when none available.
			},

			Personatorium::Crawl,	// Crawl on knees when writhing
			Personatorium::Civilian,// Victim
			3000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			2000,							// time between reaction to being shot.
			50,							// Initial hit points

		},

	////////////////////////////////Employee/////////////////////////////////////
		
		{
		// User level description of person.
		"Employee 2",					// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/emp2",					// Example: "Gunner", as in "3d/GUNNER_shoot.sop".         
			1,								// Texture to use
			"hand",						// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"hand",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"employ2",					// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidWalMartDontShoot3,		// ID of 'hit by weapon' sample.
			&g_smidBillGrunt,			// ID of 'hit by weapon' sample,
			&g_smidVinceAhuh,			// ID of 'hit by weapon' sample,
			&g_smidVinceHu,			// ID of 'hit by weapon' sample,
			&g_smidWalMartCleanupAisle17,		// ID of 'blown up' sample.     
			&g_smidWalMartNoWaitLine4,		// ID of 'blown up' sample.     
			&g_smidSteveWaFire,			// ID of 'on fire' sample
			&g_smidScottYell4,		// ID of 'on fire' sample
			&g_smidWalMartGetManager2,		// ID of suffering sample 1.
			&g_smidWalMartGetMyManager,		// ID of suffering sample 2.
			&g_smidWalMartOnlyPartTimeComeon,// ID of suffering sample 3.
			&g_smidWrithing3,			// ID of suffering sample 4.
			&g_smidWalMartMyIHelpYou,// ID of dying sample.          
			&g_smidScottCoughBlood2,// ID of dying sample.          
			&g_smidWalMartDontShoot3,  // ID of shooting comment.          
			&g_smidWalMartCleanupAisle6b,// ID of shooting comment.
			&g_smidWalMartNoWaitLine4,// ID of shooting comment
			&g_smidWalMartGetManager2,	// ID of shooting comment       
			&g_smidWalMartGetManager2,	// ID of random comment        
			&g_smidWalMartShellsSpecial2,// ID of random comment         
			&g_smidWalMartMyIHelpYou,	// ID of random comment         
			&g_smidWalMartDontShoot3,// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::TotalIDs,		// Weapon to fallback on when none available.
			},

			Personatorium::Crawl,	// Crawl on knees when writhing
			Personatorium::Civilian,// Victim
			3000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			2000,							// time between reaction to being shot.
			50,							// Initial hit points

		},

	////////////////////////////////Red Cross///////////////////////////////////
		
		{
		// User level description of person.
		"Red Cross 1",					// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/emp2",					// Example: "Gunner", as in "3d/GUNNER_shoot.sop".         
			2,								// Texture to use
			"hand",						// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"hand",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"employ2",					// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidRedCrossNeutralDontShoot,		// ID of 'hit by weapon' sample.
			&g_smidRedCrossAh,			// ID of 'hit by weapon' sample,
			&g_smidShotGrunt,			// ID of 'hit by weapon' sample,
			&g_smidRedCrossAh,			// ID of 'hit by weapon' sample,
			&g_smidRedCrossWhereCops,		// ID of 'blown up' sample.     
			&g_smidRedCrossMedic,		// ID of 'blown up' sample.     
			&g_smidRedCrossMedic3,			// ID of 'on fire' sample
			&g_smidRedCrossVolunteered,		// ID of 'on fire' sample
			&g_smidRedCrossTryingToHelp,		// ID of suffering sample 1.
			&g_smidRedCrossLowOnBlood,		// ID of suffering sample 2.
			&g_smidScottCoughBlood2,// ID of suffering sample 3.
			&g_smidRedCrossWhereCops,			// ID of suffering sample 4.
			&g_smidPaulCantFeelLegs,// ID of dying sample.          
			&g_smidRedCrossWasntTrained,// ID of dying sample.          
			&g_smidRedCrossOverpopulation,  // ID of shooting comment.          
			&g_smidRedCrossOtherHalfGuy,// ID of shooting comment.
			&g_smidRedCrossEnoughFood,// ID of shooting comment
			&g_smidRedCrossGuysNuts,	// ID of shooting comment       
			&g_smidRedCrossTriggerHappy,	// ID of random comment        
			&g_smidRedCrossGuysNuts,// ID of random comment         
			&g_smidRedCrossSquashedShot3,	// ID of random comment         
			&g_smidRedCrossSquashedShot,// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::TotalIDs,		// Weapon to fallback on when none available.
			},

			Personatorium::Crawl,	// Crawl on knees when writhing
			Personatorium::Civilian,// Victim
			3000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			2000,							// time between reaction to being shot.
			50,							// Initial hit points

		},

	////////////////////////////////Red Cross///////////////////////////////////
		
		{
		// User level description of person.
		"Red Cross 2",					// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/emp2",					// Example: "Gunner", as in "3d/GUNNER_shoot.sop".         
			3,								// Texture to use
			"hand",						// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"hand",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"employ2",					// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidRedCrossNeutral,		// ID of 'hit by weapon' sample.
			&g_smidBillGrunt,			// ID of 'hit by weapon' sample,
			&g_smidShotGrunt,			// ID of 'hit by weapon' sample,
			&g_smidRandyHuu,			// ID of 'hit by weapon' sample,
			&g_smidRedCrossStretcher,		// ID of 'blown up' sample.     
			&g_smidRedCrossDoubleBodyBag,		// ID of 'blown up' sample.     
			&g_smidRedCrossJoinedMarines,			// ID of 'on fire' sample
			&g_smidRedCrossNotTrained,		// ID of 'on fire' sample
			&g_smidRedCrossCallBackupHelp,		// ID of suffering sample 1.
			&g_smidRedCrossMedic2,		// ID of suffering sample 2.
			&g_smidRedCrossLowBlood,// ID of suffering sample 3.
			&g_smidWrithing3,			// ID of suffering sample 4.
			&g_smidRedCrossMedic2,// ID of dying sample.          
			&g_smidRedCrossJoinedMarines2,// ID of dying sample.          
			&g_smidRedCrossSquachedShot2,  // ID of shooting comment.          
			&g_smidRedCrossOtherHalf,// ID of shooting comment.
			&g_smidRedCrossStretcher,// ID of shooting comment
			&g_smidRedCrossVolunteered2,	// ID of shooting comment       
			&g_smidRedCrossVolunteered4,	// ID of random comment        
			&g_smidRedCrossSquashedShot3,// ID of random comment         
			&g_smidRedCrossJoinedMarines2,	// ID of random comment         
			&g_smidRedCrossOtherHalfGuy2,// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::TotalIDs,		// Weapon to fallback on when none available.
			},

			Personatorium::Crawl,	// Crawl on knees when writhing
			Personatorium::Civilian,// Victim
			3000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			2000,							// time between reaction to being shot.
			50,							// Initial hit points

		},

	////////////////////////////////Golfer///////////////////////////////////
		
		{
		// User level description of person.
		"Golfer 1",					// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/emp2",					// Example: "Gunner", as in "3d/GUNNER_shoot.sop".         
			4,								// Texture to use
			"hand",						// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"hand",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"employ2",					// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidGolferMyBalls,		// ID of 'hit by weapon' sample.
			&g_smidPaulRHuh,			// ID of 'hit by weapon' sample,
			&g_smidPaulRHuht,			// ID of 'hit by weapon' sample,
			&g_smidVinceHu,			// ID of 'hit by weapon' sample,
			&g_smidGolferHoleInOne,		// ID of 'blown up' sample.     
			&g_smidGolferMyBalls2,		// ID of 'blown up' sample.     
			&g_smidGolferUglyBogey,			// ID of 'on fire' sample
			&g_smidScottYell4,		// ID of 'on fire' sample
			&g_smidGolferMyBalls3,		// ID of suffering sample 1.
			&g_smidBillHelpMe,		// ID of suffering sample 2.
			&g_smidScottCoughBlood2,// ID of suffering sample 3.
			&g_smidWrithing3,			// ID of suffering sample 4.
			&g_smidPaulCantFeelLegs,// ID of dying sample.          
			&g_smidGolferCinderellaStory,// ID of dying sample.          
			&g_smidGolferCinderella,  // ID of shooting comment.          
			&g_smidGolferFore,// ID of shooting comment.
			&g_smidGolferFore2,// ID of shooting comment
			&g_smidGolferFore3,	// ID of shooting comment       
			&g_smidGolferFore,	// ID of random comment        
			&g_smidGolferFore2,// ID of random comment         
			&g_smidGolferFore3,	// ID of random comment         
			&g_smidGolferUglyBogey2,// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::TotalIDs,		// Weapon to fallback on when none available.
			},

			Personatorium::Crawl,	// Crawl on knees when writhing
			Personatorium::Civilian,// Victim
			3000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			2000,							// time between reaction to being shot.
			50,							// Initial hit points

		},

	////////////////////////////////Golfer///////////////////////////////////
		
		{
		// User level description of person.
		"Golfer 2",					// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/emp2",					// Example: "Gunner", as in "3d/GUNNER_shoot.sop".         
			5,								// Texture to use
			"hand",						// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"hand",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"employ2",					// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidGolferMyBalls4,		// ID of 'hit by weapon' sample.
			&g_smidRubinAh,			// ID of 'hit by weapon' sample,
			&g_smidRandyUg,			// ID of 'hit by weapon' sample,
			&g_smidRandyUrhh,			// ID of 'hit by weapon' sample,
			&g_smidGolferMyBalls3,		// ID of 'blown up' sample.     
			&g_smidGolferFore2,		// ID of 'blown up' sample.     
			&g_smidGolferHoleInOne3,			// ID of 'on fire' sample
			&g_smidGolferUglyBogey2,		// ID of 'on fire' sample
			&g_smidGolferCinderella,		// ID of suffering sample 1.
			&g_smidGolferMyBalls2,		// ID of suffering sample 2.
			&g_smidScottCoughBlood2,// ID of suffering sample 3.
			&g_smidWrithing3,			// ID of suffering sample 4.
			&g_smidPaulCantFeelLegs,// ID of dying sample.          
			&g_smidGolferMyBalls,// ID of dying sample.          
			&g_smidGolferCinderella,  // ID of shooting comment.          
			&g_smidGolferFore,// ID of shooting comment.
			&g_smidGolferFore2,// ID of shooting comment
			&g_smidGolferFore3,	// ID of shooting comment       
			&g_smidGolferFore3,	// ID of random comment        
			&g_smidGolferHoleInOne2,// ID of random comment         
			&g_smidGolferFore2,	// ID of random comment         
			&g_smidGolferHoleInOne,// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::TotalIDs,		// Weapon to fallback on when none available.
			},

			Personatorium::Crawl,	// Crawl on knees when writhing
			Personatorium::Civilian,// Victim
			3000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			2000,							// time between reaction to being shot.
			50,							// Initial hit points

		},

	////////////////////////////////Lawyer///////////////////////////////////
		
		{
		// User level description of person.
		"Lawyer",						// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/emp2",					// Example: "Gunner", as in "3d/GUNNER_shoot.sop".         
			6,								// Texture to use
			"hand",						// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"hand",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"employ2",					// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidLawyerThatsIllegal,		// ID of 'hit by weapon' sample.
			&g_smidBillGrunt,			// ID of 'hit by weapon' sample,
			&g_smidShotGrunt,			// ID of 'hit by weapon' sample,
			&g_smidRandyHuu,			// ID of 'hit by weapon' sample,
			&g_smidLawyerGonnaSue,		// ID of 'blown up' sample.     
			&g_smidLawyerSeeAssInCourt3,		// ID of 'blown up' sample.     
			&g_smidLawyerYoullNeedLawyer,			// ID of 'on fire' sample
			&g_smidLawyerNeedLawyerCard,		// ID of 'on fire' sample
			&g_smidLawyerGonnaSue,		// ID of suffering sample 1.
			&g_smidLawyerSeeAssInCourt2,		// ID of suffering sample 2.
			&g_smidLawyerGonnaSue,// ID of suffering sample 3.
			&g_smidLawyerGonnaSue,			// ID of suffering sample 4.
			&g_smidPaulCantFeelLegs,// ID of dying sample.          
			&g_smidLawyerSeeAssInCourt3,// ID of dying sample.          
			&g_smidLawyerGonnaNeedLawyer,  // ID of shooting comment.          
			&g_smidLawyerThatsIllegal2,// ID of shooting comment.
			&g_smidLawyerYoullNeedLawyer,// ID of shooting comment
			&g_smidLawyerNeedLawyerCard,	// ID of shooting comment       
			&g_smidLawyerGonnaNeedLawyer,	// ID of random comment        
			&g_smidLawyerYoullNeedLawyer,// ID of random comment         
			&g_smidLawyerNeedLawyerCard,	// ID of random comment         
			&g_smidLawyerThatsIllegal,// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::TotalIDs,		// Weapon to fallback on when none available.
			},

			Personatorium::Crawl,	// Crawl on knees when writhing
			Personatorium::Civilian,// Victim
			3000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			2000,							// time between reaction to being shot.
			50,							// Initial hit points

		},

	////////////////////////////////Bum/////////////////////////////////////
		
		{
		// User level description of person.
		"Bum 1",					// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/emp2",					// Example: "Gunner", as in "3d/GUNNER_shoot.sop".         
			7,								// Texture to use
			"hand",						// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"hand",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"employ2",					// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidVetDontShootImVet,		// ID of 'hit by weapon' sample.
			&g_smidVetAhah,			// ID of 'hit by weapon' sample,
			&g_smidVetWaa,			// ID of 'hit by weapon' sample,
			&g_smidVetWaaoh,			// ID of 'hit by weapon' sample,
			&g_smidVetWaaoh,		// ID of 'blown up' sample.     
			&g_smidScottYell1,		// ID of 'blown up' sample.     
			&g_smidVetUhOh,			// ID of 'on fire' sample
			&g_smidVetSpareDimeYell,		// ID of 'on fire' sample
			&g_smidVetSpareDime,		// ID of suffering sample 1.
			&g_smidVetShineShotgun,		// ID of suffering sample 2.
			&g_smidScottCoughBlood2,// ID of suffering sample 3.
			&g_smidWrithing3,			// ID of suffering sample 4.
			&g_smidPaulCantFeelLegs,// ID of dying sample.          
			&g_smidScottCoughBlood2,// ID of dying sample.          
			&g_smidVetFragThatBastard,  // ID of shooting comment.          
			&g_smidVetKillThatPsyco,// ID of shooting comment.
			&g_smidVetSpanish,// ID of shooting comment
			&g_smidVetFrag,	// ID of shooting comment       
			&g_smidVetWorkForFood,	// ID of random comment        
			&g_smidVetAskHimForHelp,// ID of random comment         
			&g_smidVetThisGuysNuts,	// ID of random comment         
			&g_smidVetWorkForFood2,// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::TotalIDs,		// Weapon to fallback on when none available.
			},

			Personatorium::Crawl,	// Crawl on knees when writhing
			Personatorium::Civilian,// Victim
			3000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			2000,							// time between reaction to being shot.
			50,							// Initial hit points

		},

	////////////////////////////////Bum/////////////////////////////////////
		
		{
		// User level description of person.
		"Bum 2",					// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/emp2",					// Example: "Gunner", as in "3d/GUNNER_shoot.sop".         
			8,								// Texture to use
			"hand",						// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"hand",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"employ2",					// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidVetWhatThatAK47,		// ID of 'hit by weapon' sample.
			&g_smidShotGrunt,			// ID of 'hit by weapon' sample.
			&g_smidRandyUrhh,			// ID of 'hit by weapon' sample,
			&g_smidVinceHu,			// ID of 'hit by weapon' sample,
			&g_smidVetCharlie,		// ID of 'blown up' sample.     
			&g_smidScottYell1,		// ID of 'blown up' sample.     
			&g_smidVetUhOh,			// ID of 'on fire' sample
			&g_smidScottYell4,		// ID of 'on fire' sample
			&g_smidVetWaa,		// ID of suffering sample 1.
			&g_smidVetDontShootVet3,		// ID of suffering sample 2.
			&g_smidVetWorkForFood2,// ID of suffering sample 3.
			&g_smidWrithing3,			// ID of suffering sample 4.
			&g_smidVetHereComesCharlie,// ID of dying sample.          
			&g_smidVetAskHimForHelp,// ID of dying sample.          
			&g_smidVetFrag,  // ID of shooting comment.          
			&g_smidVetFragBastard,// ID of shooting comment.
			&g_smidVetKillThatPsyco,// ID of shooting comment
			&g_smidVetSpanish,	// ID of shooting comment       
			&g_smidVetCollectShells,	// ID of random comment        
			&g_smidVetCollectBrassShells,// ID of random comment         
			&g_smidVetShine,	// ID of random comment         
			&g_smidVetShingGun,// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::TotalIDs,		// Weapon to fallback on when none available.
			},

			Personatorium::Crawl,	// Crawl on knees when writhing
			Personatorium::Civilian,// Victim
			3000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			2000,							// time between reaction to being shot.
			50,							// Initial hit points

		},

	////////////////////////////////Vet/////////////////////////////////////
		
		{
		// User level description of person.
		"Vet 1",					// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/emp2",					// Example: "Gunner", as in "3d/GUNNER_shoot.sop".         
			9,								// Texture to use
			"hand",						// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"hand",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"employ2",					// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidVetCharlie,		// ID of 'hit by weapon' sample.
			&g_smidBillGrunt,			// ID of 'hit by weapon' sample,
			&g_smidShotGrunt,			// ID of 'hit by weapon' sample,
			&g_smidRandyHuu,			// ID of 'hit by weapon' sample,
			&g_smidVetWaaoh,		// ID of 'blown up' sample.     
			&g_smidVetSpareDimeYell,		// ID of 'blown up' sample.     
			&g_smidVetWaa,			// ID of 'on fire' sample
			&g_smidVetAhah,		// ID of 'on fire' sample
			&g_smidVetAskHimForHelp,		// ID of suffering sample 1.
			&g_smidBillHelpMe,		// ID of suffering sample 2.
			&g_smidScottCoughBlood2,// ID of suffering sample 3.
			&g_smidVetWaa,			// ID of suffering sample 4.
			&g_smidVetSpareDime,// ID of dying sample.          
			&g_smidVetWorkForFood2,// ID of dying sample.          
			&g_smidVetFrag,  // ID of shooting comment.          
			&g_smidVetFragBastard,// ID of shooting comment.
			&g_smidVetCharlie,// ID of shooting comment
			&g_smidVetKillThatPsyco,	// ID of shooting comment       
			&g_smidVetThisGuysNuts,	// ID of random comment        
			&g_smidVetWorkForFood,// ID of random comment         
			&g_smidVetCollectShells,	// ID of random comment         
			&g_smidVetCollectBrassShells,// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::TotalIDs,		// Weapon to fallback on when none available.
			},

			Personatorium::Crawl,	// Crawl on knees when writhing
			Personatorium::Civilian,// Victim
			3000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			2000,							// time between reaction to being shot.
			50,							// Initial hit points

		},

	////////////////////////////////Vet/////////////////////////////////////
		
		{
		// User level description of person.
		"Vet 2",					// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/emp2",					// Example: "Gunner", as in "3d/GUNNER_shoot.sop".         
			10,							// Texture to use
			"hand",						// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"hand",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"employ2",					// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidVetWhatThatAK47,		// ID of 'hit by weapon' sample.
			&g_smidBillGrunt,			// ID of 'hit by weapon' sample,
			&g_smidShotGrunt,			// ID of 'hit by weapon' sample,
			&g_smidRandyHuu,			// ID of 'hit by weapon' sample,
			&g_smidVetSpareDimeYell,		// ID of 'blown up' sample.     
			&g_smidScottYell1,		// ID of 'blown up' sample.     
			&g_smidScottHelp,			// ID of 'on fire' sample
			&g_smidVetWaaoh,		// ID of 'on fire' sample
			&g_smidBillKillMe,		// ID of suffering sample 1.
			&g_smidVetSpareADime,		// ID of suffering sample 2.
			&g_smidVetAskHimForHelp,// ID of suffering sample 3.
			&g_smidWrithing2,			// ID of suffering sample 4.
			&g_smidVetWorkForFood,// ID of dying sample.          
			&g_smidVetUhOh,// ID of dying sample.          
			&g_smidVetFrag,  // ID of shooting comment.          
			&g_smidVetKillThatPsyco,// ID of shooting comment.
			&g_smidVetFragBastard,// ID of shooting comment
			&g_smidVetCharlie,	// ID of shooting comment       
			&g_smidVetCollectBrassShells,	// ID of random comment        
			&g_smidVetCollectShells,// ID of random comment         
			&g_smidVetWorkForFood,	// ID of random comment         
			&g_smidVetWorkForFood2,// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::TotalIDs,		// Weapon to fallback on when none available.
			},

			Personatorium::Crawl,	// Crawl on knees when writhing
			Personatorium::Civilian,// Victim
			3000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			2000,							// time between reaction to being shot.
			50,							// Initial hit points

		},

	////////////////////////////////Nude/////////////////////////////////////
		
		{
		// User level description of person.
		"Nude 1",					// pszDescription.  - Nude Man

		// Names used to create animation resource filenames.
			{
			"3d/emp2",					// Example: "Gunner", as in "3d/GUNNER_shoot.sop".         
			11,							// Texture to use
			"hand",						// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"hand",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"employ2",					// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidNudeShotOffMyAhhh2,		// ID of 'hit by weapon' sample.
			&g_smidNudeMyGod,			// ID of 'hit by weapon' sample,
			&g_smidShotGrunt,			// ID of 'hit by weapon' sample,
			&g_smidVinceHu,			// ID of 'hit by weapon' sample,
			&g_smidScottGrunt,		// ID of 'blown up' sample.     
			&g_smidScottYell1,		// ID of 'blown up' sample.     
			&g_smidScottHelp,			// ID of 'on fire' sample
			&g_smidScottYell4,		// ID of 'on fire' sample
			&g_smidNudeDontFeelFresh,		// ID of suffering sample 1.
			&g_smidNudeHowEmbarrassing,		// ID of suffering sample 2.
			&g_smidNudeDontDieNaked,// ID of suffering sample 3.
			&g_smidNudeDontFeelFresh2,			// ID of suffering sample 4.
			&g_smidNudeHowEmbarrassing,// ID of dying sample.          
			&g_smidNudeHowEmbarrassing2,// ID of dying sample.          
			&g_smidNudeNoOneSeeUs,  // ID of shooting comment.          
			&g_smidNudeNoOneSeeUs2,// ID of shooting comment.
			&g_smidNudeDontFeelFresh,// ID of shooting comment
			&g_smidNudeDontFeelFresh2,	// ID of shooting comment       
			&g_smidNudeNoOneSeeUs,	// ID of random comment        
			&g_smidNudeNoOneSeeUs2,// ID of random comment         
			&g_smidNudeDontFeelFresh,	// ID of random comment         
			&g_smidNudeDontFeelFresh2,// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::TotalIDs,		// Weapon to fallback on when none available.
			},

			Personatorium::Crawl,	// Crawl on knees when writhing
			Personatorium::Civilian,// Victim
			3000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			2000,							// time between reaction to being shot.
			50,							// Initial hit points

		},

	////////////////////////////////Nude/////////////////////////////////////
		
		{
		// User level description of person.
		"Nude 2",						// pszDescription.  Nude Woman

		// Names used to create animation resource filenames.
			{
			"3d/nude2",					// Example: "Gunner", as in "3d/GUNNER_shoot.sop".         
			-1,							// Texture to use
			"hand",						// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"hand",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"nude2",						// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidWNudeDontShootNude,		// ID of 'hit by weapon' sample.
			&g_smidShotFemaleGrunt,	// ID of 'hit by weapon' sample,
			&g_smidCelinaUg,			// ID of 'hit by weapon' sample,
			&g_smidDebbieAh,			// ID of 'hit by weapon' sample,
			&g_smidWNudeHeShotOffMyAh,		// ID of 'blown up' sample.     
			&g_smidSouthernBumLooker,		// ID of 'blown up' sample.     
			&g_smidSouthernDieNaked,			// ID of 'on fire' sample
			&g_smidSouthernNeedLawyer,		// ID of 'on fire' sample
			&g_smidKimHelp,			// ID of suffering sample 1.
			&g_smidKimCantBreathe,	// ID of suffering sample 2.
			&g_smidWNudeDontFeelFresh,			// ID of suffering sample 3.
			&g_smidWNudeDontFeelFresh,			// ID of suffering sample 4.
			&g_smidWNudeHowEmbarrassing,// ID of dying sample.          
			&g_smidWNudeHowEmbarrassing2,// ID of dying sample.          
			&g_smidNil,  // ID of shooting comment.          
			&g_smidNil,// ID of shooting comment.
			&g_smidNil,// ID of shooting comment
			&g_smidNil,	// ID of shooting comment       
			&g_smidWNudeNoOneSeeUs,	// ID of random comment        
			&g_smidWNudeBumLooker,// ID of random comment         
			&g_smidWNudeHowEmbarrassing,	// ID of random comment         
			&g_smidWNudeDontFeelFresh,// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::TotalIDs,		// Weapon to fallback on when none available.
			},

			Personatorium::Crawl,	// Crawl on knees when writhing
			Personatorium::Civilian,// Victim
			3000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			2000,							// time between reaction to being shot.
			50,							// Initial hit points

		},

	////////////////////////////////Gramps/////////////////////////////////////
		
		{
		// User level description of person.
		"Gramps 1",						// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/gramps",				// Example: "Gunner", as in "3d/GUNNER_shoot.sop".         
			-1,							// Texture to use
			"hand",						// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"hand",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"gramps",						// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidOldManWhatsThatSunny,		// ID of 'hit by weapon' sample.
			&g_smidBillGrunt,			// ID of 'hit by weapon' sample,
			&g_smidShotGrunt,			// ID of 'hit by weapon' sample,
			&g_smidRandyHuu,			// ID of 'hit by weapon' sample,
			&g_smidScottGrunt,		// ID of 'blown up' sample.     
			&g_smidScottYell1,		// ID of 'blown up' sample.     
			&g_smidScottHelp,			// ID of 'on fire' sample
			&g_smidScottYell4,		// ID of 'on fire' sample
			&g_smidOldManSoiledSelf,		// ID of suffering sample 1.
			&g_smidOldManColostomyBag,		// ID of suffering sample 2.
			&g_smidOldManOwSpleen,// ID of suffering sample 3.
			&g_smidOldManGroan,			// ID of suffering sample 4.
			&g_smidOldManSoiledSelf2,// ID of dying sample.          
			&g_smidScottCoughBlood2,// ID of dying sample.          
			&g_smidNil,  // ID of shooting comment.          
			&g_smidNil,// ID of shooting comment.
			&g_smidNil,// ID of shooting comment
			&g_smidNil,	// ID of shooting comment       
			&g_smidOldManDontShoot,	// ID of random comment        
			&g_smidOldManNoRespect,// ID of random comment         
			&g_smidOldManInGreatWar2,	// ID of random comment         
			&g_smidOldManWhatsThatSunny,// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::TotalIDs,		// Weapon to fallback on when none available.
			},

			Personatorium::Crawl,	// Crawl on knees when writhing
			Personatorium::Civilian,// Victim
			3000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			2000,							// time between reaction to being shot.
			50,							// Initial hit points

		},

	////////////////////////////////Gramps/////////////////////////////////////
		
		{
		// User level description of person.
		"Gramps 2",						// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/gramps",				// Example: "Gunner", as in "3d/GUNNER_shoot.sop".         
			2,								// Texture to use
			"hand",						// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"hand",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"gramps",						// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidOldManSpleen,		// ID of 'hit by weapon' sample.
			&g_smidBillGrunt,			// ID of 'hit by weapon' sample,
			&g_smidShotGrunt,			// ID of 'hit by weapon' sample,
			&g_smidRandyHuu,			// ID of 'hit by weapon' sample,
			&g_smidOldManGroan,		// ID of 'blown up' sample.     
			&g_smidOldManGroan,		// ID of 'blown up' sample.     
			&g_smidOldManKidsNoRespect2,			// ID of 'on fire' sample
			&g_smidOldManGroan,		// ID of 'on fire' sample
			&g_smidOldManInGreatWar2,		// ID of suffering sample 1.
			&g_smidOldManInGreatWar,		// ID of suffering sample 2.
			&g_smidOldManSoiledSelf3,// ID of suffering sample 3.
			&g_smidOldManColostomyBag3,			// ID of suffering sample 4.
			&g_smidOldManGroan,// ID of dying sample.          
			&g_smidScottCoughBlood2,// ID of dying sample.          
			&g_smidNil,  // ID of shooting comment.          
			&g_smidNil,// ID of shooting comment.
			&g_smidNil,// ID of shooting comment
			&g_smidNil,	// ID of shooting comment       
			&g_smidOldManKidsNoRespect,	// ID of random comment        
			&g_smidOldManKidsNoRespect3,// ID of random comment         
			&g_smidOldManDontShootRetired2,	// ID of random comment         
			&g_smidOldManRetired,// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::TotalIDs,		// Weapon to fallback on when none available.
			},

			Personatorium::Crawl,	// Crawl on knees when writhing
			Personatorium::Civilian,// Victim
			3000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			2000,							// time between reaction to being shot.
			50,							// Initial hit points

		},

	////////////////////////////////Granny/////////////////////////////////////
		
		{
		// User level description of person.
		"Granny 1",						// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/gramps",				// Example: "Gunner", as in "3d/GUNNER_shoot.sop".         
			3,								// Texture to use
			"hand",						// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"hand",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"gramps",						// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidOldWomanMySpleen,		// ID of 'hit by weapon' sample.
			&g_smidOldWomanOh,			// ID of 'hit by weapon' sample,
			&g_smidOldWomanOh,			// ID of 'hit by weapon' sample,
			&g_smidOldWomanOh,			// ID of 'hit by weapon' sample,
			&g_smidOldWomanFindColostomy,		// ID of 'blown up' sample.     
			&g_smidOldWomanOh,		// ID of 'blown up' sample.     
			&g_smidOldWomanCantHear,			// ID of 'on fire' sample
			&g_smidOldWomanColostomyBag,		// ID of 'on fire' sample
			&g_smidOldWomanMySpleen,		// ID of suffering sample 1.
			&g_smidOldWomanSoiledSelf2,		// ID of suffering sample 2.
			&g_smidOldWomanOh,// ID of suffering sample 3.
			&g_smidOldWomanFindColostomy,			// ID of suffering sample 4.
			&g_smidOldWomanNoRespect,// ID of dying sample.          
			&g_smidOldWomanSoiledSelf,// ID of dying sample.          
			&g_smidNil,  // ID of shooting comment.          
			&g_smidNil,// ID of shooting comment.
			&g_smidNil,// ID of shooting comment
			&g_smidNil,	// ID of shooting comment       
			&g_smidOldWomanNoRespect,	// ID of random comment        
			&g_smidOldWomanEhWhatsThat,// ID of random comment         
			&g_smidOldWomanCantHear,	// ID of random comment         
			&g_smidOldWomanEhWhatsThat,// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::TotalIDs,		// Weapon to fallback on when none available.
			},

			Personatorium::Crawl,	// Crawl on knees when writhing
			Personatorium::Civilian,// Victim
			3000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			2000,							// time between reaction to being shot.
			50,							// Initial hit points

		},

	////////////////////////////////Granny/////////////////////////////////////
		
		{
		// User level description of person.
		"Granny 2",						// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/gramps",				// Example: "Gunner", as in "3d/GUNNER_shoot.sop".         
			4,								// Texture to use
			"hand",						// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"hand",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"gramps",						// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidOldWomanEhWhatsThat,		// ID of 'hit by weapon' sample.
			&g_smidOldWomanOh,			// ID of 'hit by weapon' sample,
			&g_smidOldWomanOh,			// ID of 'hit by weapon' sample,
			&g_smidOldWomanOh,			// ID of 'hit by weapon' sample,
			&g_smidOldWomanColostomy,		// ID of 'blown up' sample.     
			&g_smidOldWomanOh,		// ID of 'blown up' sample.     
			&g_smidOldWomanNoRespect,			// ID of 'on fire' sample
			&g_smidOldWomanOh,		// ID of 'on fire' sample
			&g_smidOldWomanMySpleen,		// ID of suffering sample 1.
			&g_smidOldWomanFindColostomy,		// ID of suffering sample 2.
			&g_smidOldWomanColostomyBag,// ID of suffering sample 3.
			&g_smidOldWomanSoiledSelf,			// ID of suffering sample 4.
			&g_smidOldWomanCantHear,// ID of dying sample.          
			&g_smidOldWomanSoiledSelf2,// ID of dying sample.          
			&g_smidNil,  // ID of shooting comment.          
			&g_smidNil,// ID of shooting comment.
			&g_smidNil,// ID of shooting comment
			&g_smidNil,	// ID of shooting comment       
			&g_smidOldWomanNoRespect,	// ID of random comment        
			&g_smidOldWomanEhWhatsThat,// ID of random comment         
			&g_smidOldWomanCantHear,	// ID of random comment         
			&g_smidOldWomanCantHear,// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::TotalIDs,		// Weapon to fallback on when none available.
			},

			Personatorium::Crawl,	// Crawl on knees when writhing
			Personatorium::Civilian,// Victim
			3000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			2000,							// time between reaction to being shot.
			50,							// Initial hit points

		},

	////////////////////////////////Granny/////////////////////////////////////
		
		{
		// User level description of person.
		"Granny 3",						// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/gramps",				// Example: "Gunner", as in "3d/GUNNER_shoot.sop".         
			5,								// Texture to use
			"hand",						// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"hand",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"gramps",						// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidOldWomanMySpleen,		// ID of 'hit by weapon' sample.
			&g_smidOldWomanOh,			// ID of 'hit by weapon' sample,
			&g_smidOldWomanOh,			// ID of 'hit by weapon' sample,
			&g_smidOldWomanOh,			// ID of 'hit by weapon' sample,
			&g_smidOldWomanEhWhatsThat,		// ID of 'blown up' sample.     
			&g_smidOldWomanOh,		// ID of 'blown up' sample.     
			&g_smidOldWomanEhWhatsThat,			// ID of 'on fire' sample
			&g_smidOldWomanOh,		// ID of 'on fire' sample
			&g_smidOldWomanColostomyBag,		// ID of suffering sample 1.
			&g_smidOldWomanMySpleen,		// ID of suffering sample 2.
			&g_smidOldWomanSoiledSelf,// ID of suffering sample 3.
			&g_smidOldWomanFindColostomy,			// ID of suffering sample 4.
			&g_smidOldWomanSoiledSelf2,// ID of dying sample.          
			&g_smidOldWomanSoiledSelf,// ID of dying sample.          
			&g_smidNil,  // ID of shooting comment.          
			&g_smidNil,// ID of shooting comment.
			&g_smidNil,// ID of shooting comment
			&g_smidNil,	// ID of shooting comment       
			&g_smidOldWomanNoRespect,	// ID of random comment        
			&g_smidOldWomanCantHear,// ID of random comment         
			&g_smidOldWomanEhWhatsThat,	// ID of random comment         
			&g_smidOldWomanSoiledSelf,// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::TotalIDs,		// Weapon to fallback on when none available.
			},

			Personatorium::Crawl,	// Crawl on knees when writhing
			Personatorium::Civilian,// Victim
			3000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			2000,							// time between reaction to being shot.
			50,							// Initial hit points

		},



	////////////////////////////////Shopper Golfer///////////////////////////////////
		
		{
		// User level description of person.
		"Shopper G1",					// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/emp2",					// Example: "Gunner", as in "3d/GUNNER_shoot.sop".         
			4,								// Texture to use
			"hand",						// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"hand",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"employ2",					// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidVictimStopShootingAlreadyDead,	// ID of 'hit by weapon' sample.
			&g_smidVictimAuh,			// ID of 'hit by weapon' sample,
			&g_smidScottGrunt,		// ID of 'hit by weapon' sample,
			&g_smidDyingYell,			// ID of 'hit by weapon' sample,
			&g_smidVictimAAhh,		// ID of 'blown up' sample.     
			&g_smidVictimAhhk,		// ID of 'blown up' sample.     
			&g_smidScottYell3,		// ID of 'on fire' sample
			&g_smidScottHelp,			// ID of 'on fire' sample
			&g_smidVictimINeedFirstAid,			// ID of suffering sample 1.
			&g_smidVictimAnyoneSeenEar,			// ID of suffering sample 2.
			&g_smidVictimVeryBadDay,			// ID of suffering sample 3.
			&g_smidVictimEndNear,			// ID of suffering sample 4.
			&g_smidVictimTheEndIsNear,// ID of dying sample.          
			&g_smidVictimAlreadyDead,// ID of dying sample.          
			&g_smidNil,					// ID of shooting comment.          
			&g_smidNil,					// ID of shooting comment.
			&g_smidNil,					// ID of shooting comment
			&g_smidNil,					// ID of shooting comment       
			&g_smidVictimJudgementDay,		// ID of random comment        
			&g_smidVictimRunForLives3,		// ID of random comment         
			&g_smidVictimWhereTVMan,	// ID of random comment         
			&g_smidVictimWhatsThat,		// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::TotalIDs,		// Weapon to fallback on when none available.
			},

			Personatorium::Crawl,	// Crawl on knees when writhing
			Personatorium::Civilian,// Victim
			3000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			2000,							// time between reaction to being shot.
			50,							// Initial hit points

		},

	////////////////////////////////Shopper Golfer///////////////////////////////////
		
		{
		// User level description of person.
		"Shopper G2",					// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/emp2",					// Example: "Gunner", as in "3d/GUNNER_shoot.sop".         
			5,								// Texture to use
			"hand",						// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"hand",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"employ2",					// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidVictimOwMyEye,		// ID of 'hit by weapon' sample.
			&g_smidVinceHu,			// ID of 'hit by weapon' sample,
			&g_smidScottGrunt,		// ID of 'hit by weapon' sample,
			&g_smidVictimAhu,			// ID of 'hit by weapon' sample,
			&g_smidVictimAhhk,		// ID of 'blown up' sample.     
			&g_smidVictimAhh,		// ID of 'blown up' sample.     
			&g_smidVictimAuh,		// ID of 'on fire' sample
			&g_smidScottHelp,			// ID of 'on fire' sample
			&g_smidVictimFirstAid2,			// ID of suffering sample 1.
			&g_smidVictimOwMyEyes,			// ID of suffering sample 2.
			&g_smidVictimAnyoneSeenEar,		// ID of suffering sample 3.
			&g_smidVictimStopAlreadyDead,			// ID of suffering sample 4.
			&g_smidVictimTheHorror,// ID of dying sample.          
			&g_smidVictimAhSheAi,// ID of dying sample.          
			&g_smidNil,					// ID of shooting comment.          
			&g_smidNil,					// ID of shooting comment.
			&g_smidNil,					// ID of shooting comment
			&g_smidNil,					// ID of shooting comment       
			&g_smidVictimRunForLives4,		// ID of random comment        
			&g_smidVictimWhereTV2,		// ID of random comment         
			&g_smidVictimAskHim,	// ID of random comment         
			&g_smidVictimAhHesGotAGun2,		// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::TotalIDs,		// Weapon to fallback on when none available.
			},

			Personatorium::Crawl,	// Crawl on knees when writhing
			Personatorium::Civilian,// Victim
			3000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			2000,							// time between reaction to being shot.
			50,							// Initial hit points

		},


	////////////////////////////////Shopper Bum/////////////////////////////////////
		
		{
		// User level description of person.
		"Shopper B1",					// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/emp2",					// Example: "Gunner", as in "3d/GUNNER_shoot.sop".         
			7,								// Texture to use
			"hand",						// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"hand",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"employ2",					// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidVictimStopShootingAlreadyDead,	// ID of 'hit by weapon' sample.
			&g_smidVictimAuh,			// ID of 'hit by weapon' sample,
			&g_smidScottGrunt,		// ID of 'hit by weapon' sample,
			&g_smidDyingYell,			// ID of 'hit by weapon' sample,
			&g_smidVictimAAhh,		// ID of 'blown up' sample.     
			&g_smidVictimAhhk,		// ID of 'blown up' sample.     
			&g_smidScottYell3,		// ID of 'on fire' sample
			&g_smidScottHelp,			// ID of 'on fire' sample
			&g_smidVictimINeedFirstAid,			// ID of suffering sample 1.
			&g_smidVictimAnyoneSeenEar,			// ID of suffering sample 2.
			&g_smidVictimFirstAid,			// ID of suffering sample 3.
			&g_smidVictimEndNear,			// ID of suffering sample 4.
			&g_smidVictimTheEndIsNear,// ID of dying sample.          
			&g_smidVictimAlreadyDead,// ID of dying sample.          
			&g_smidNil,					// ID of shooting comment.          
			&g_smidNil,					// ID of shooting comment.
			&g_smidNil,					// ID of shooting comment
			&g_smidNil,					// ID of shooting comment       
			&g_smidVictimJudgementDay,		// ID of random comment        
			&g_smidVictimRunForLives3,		// ID of random comment         
			&g_smidVictimWhereTVMan,	// ID of random comment         
			&g_smidVictimWhatsThat,		// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::TotalIDs,		// Weapon to fallback on when none available.
			},

			Personatorium::Crawl,	// Crawl on knees when writhing
			Personatorium::Civilian,// Victim
			3000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			2000,							// time between reaction to being shot.
			50,							// Initial hit points

		},

	////////////////////////////////Shopper Bum/////////////////////////////////////
		
		{
		// User level description of person.
		"Shopper B2",					// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/emp2",					// Example: "Gunner", as in "3d/GUNNER_shoot.sop".         
			8,								// Texture to use
			"hand",						// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"hand",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"employ2",					// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidVictimOwMyEye,		// ID of 'hit by weapon' sample.
			&g_smidVinceHu,			// ID of 'hit by weapon' sample,
			&g_smidScottGrunt,		// ID of 'hit by weapon' sample,
			&g_smidVictimAhu,			// ID of 'hit by weapon' sample,
			&g_smidVictimAhhk,		// ID of 'blown up' sample.     
			&g_smidVictimAhh,		// ID of 'blown up' sample.     
			&g_smidVictimAuh,		// ID of 'on fire' sample
			&g_smidScottHelp,			// ID of 'on fire' sample
			&g_smidVictimFirstAid2,			// ID of suffering sample 1.
			&g_smidVictimOwMyEyes,			// ID of suffering sample 2.
			&g_smidVictimAnyoneSeenEar,		// ID of suffering sample 3.
			&g_smidVictimStopAlreadyDead,			// ID of suffering sample 4.
			&g_smidVictimTheHorror,// ID of dying sample.          
			&g_smidVictimAhSheAi,// ID of dying sample.          
			&g_smidNil,					// ID of shooting comment.          
			&g_smidNil,					// ID of shooting comment.
			&g_smidNil,					// ID of shooting comment
			&g_smidNil,					// ID of shooting comment       
			&g_smidVictimRunForLives4,		// ID of random comment        
			&g_smidVictimWhereTV2,		// ID of random comment         
			&g_smidVictimAskHim,	// ID of random comment         
			&g_smidVictimAhHesGotAGun2,		// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::TotalIDs,		// Weapon to fallback on when none available.
			},

			Personatorium::Crawl,	// Crawl on knees when writhing
			Personatorium::Civilian,// Victim
			3000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			2000,							// time between reaction to being shot.
			50,							// Initial hit points

		},

	//////////////////////////////// Shopper Vet/////////////////////////////////////
		
		{
		// User level description of person.
		"Shopper V1",					// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/emp2",					// Example: "Gunner", as in "3d/GUNNER_shoot.sop".         
			9,								// Texture to use
			"hand",						// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"hand",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"employ2",					// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidVictimStopShootingAlreadyDead,	// ID of 'hit by weapon' sample.
			&g_smidVictimAuh,			// ID of 'hit by weapon' sample,
			&g_smidScottGrunt,		// ID of 'hit by weapon' sample,
			&g_smidDyingYell,			// ID of 'hit by weapon' sample,
			&g_smidVictimAAhh,		// ID of 'blown up' sample.     
			&g_smidVictimAhhk,		// ID of 'blown up' sample.     
			&g_smidScottYell3,		// ID of 'on fire' sample
			&g_smidScottHelp,			// ID of 'on fire' sample
			&g_smidVictimINeedFirstAid,			// ID of suffering sample 1.
			&g_smidVictimAnyoneSeenEar,			// ID of suffering sample 2.
			&g_smidVictimHorrorGroan,			// ID of suffering sample 3.
			&g_smidVictimEndNear,			// ID of suffering sample 4.
			&g_smidVictimTheEndIsNear,// ID of dying sample.          
			&g_smidVictimAlreadyDead,// ID of dying sample.          
			&g_smidNil,					// ID of shooting comment.          
			&g_smidNil,					// ID of shooting comment.
			&g_smidNil,					// ID of shooting comment
			&g_smidNil,					// ID of shooting comment       
			&g_smidVictimJudgementDay,		// ID of random comment        
			&g_smidVictimRunForLives3,		// ID of random comment         
			&g_smidVictimWhereTVMan,	// ID of random comment         
			&g_smidVictimWhatsThat,		// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::TotalIDs,		// Weapon to fallback on when none available.
			},

			Personatorium::Crawl,	// Crawl on knees when writhing
			Personatorium::Civilian,// Victim
			3000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			2000,							// time between reaction to being shot.
			50,							// Initial hit points

		},

	//////////////////////////////// Shopper Vet/////////////////////////////////////
		
		{
		// User level description of person.
		"Shopper V2",					// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/emp2",					// Example: "Gunner", as in "3d/GUNNER_shoot.sop".         
			10,							// Texture to use
			"hand",						// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"hand",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"employ2",					// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidVictimOwMyEye,		// ID of 'hit by weapon' sample.
			&g_smidVinceHu,			// ID of 'hit by weapon' sample,
			&g_smidScottGrunt,		// ID of 'hit by weapon' sample,
			&g_smidVictimAhu,			// ID of 'hit by weapon' sample,
			&g_smidVictimAhhk,		// ID of 'blown up' sample.     
			&g_smidVictimAhh,		// ID of 'blown up' sample.     
			&g_smidVictimAuh,		// ID of 'on fire' sample
			&g_smidScottHelp,			// ID of 'on fire' sample
			&g_smidVictimFirstAid2,			// ID of suffering sample 1.
			&g_smidVictimOwMyEyes,			// ID of suffering sample 2.
			&g_smidVictimAnyoneSeenEar,		// ID of suffering sample 3.
			&g_smidVictimStopAlreadyDead,			// ID of suffering sample 4.
			&g_smidVictimTheHorror,// ID of dying sample.          
			&g_smidVictimAhSheAi,// ID of dying sample.          
			&g_smidNil,					// ID of shooting comment.          
			&g_smidNil,					// ID of shooting comment.
			&g_smidNil,					// ID of shooting comment
			&g_smidNil,					// ID of shooting comment       
			&g_smidVictimRunForLives4,		// ID of random comment        
			&g_smidVictimWhereTV2,		// ID of random comment         
			&g_smidVictimAskHim,	// ID of random comment         
			&g_smidVictimAhHesGotAGun2,		// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::TotalIDs,		// Weapon to fallback on when none available.
			},

			Personatorium::Crawl,	// Crawl on knees when writhing
			Personatorium::Civilian,// Victim
			3000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			2000,							// time between reaction to being shot.
			50,							// Initial hit points

		},


#if (TARGET == SUPER_POSTAL) || (TARGET == JAPAN_ADDON)
	/////////////////////////////// JPMaleKensaku //////////////////////////////////

		{
		// User level description of person.
		"JP Male Kensaku",						// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/man",					// Example: "Miner", as in "3d/MINER_shoot.sop".         
			2,								// Texture to use
			"hand",						// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"hand",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"guy",						// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidKensaku_h1,		// ID of 'hit by weapon' sample.
			&g_smidKensaku_h2,		// ID of 'hit by weapon' sample,
			&g_smidKensaku_h3,		// ID of 'hit by weapon' sample,
			&g_smidKensaku_h4,		// ID of 'hit by weapon' sample,
			&g_smidKensaku_b1,		// ID of 'blown up' sample.     
			&g_smidKensaku_b2,		// ID of 'blown up' sample.     
			&g_smidKensaku_o1,		// ID of 'on fire' sample
			&g_smidKensaku_o2,		// ID of 'on fire' sample
			&g_smidKensaku_ss1,		// ID of suffering sample 1.
			&g_smidKensaku_ss2,		// ID of suffering sample 2.
			&g_smidKensaku_ss3,		// ID of suffering sample 3.
			&g_smidKensaku_ss4,		// ID of suffering sample 4.
			&g_smidKensaku_d1,		// ID of dying sample.          
			&g_smidKensaku_d2,		// ID of dying sample.          
			&g_smidKensaku_sc1,		// ID of shooting comment. - Nil
			&g_smidKensaku_sc2,		// ID of shooting comment. - Nil
			&g_smidKensaku_sc3,		// ID of shooting comment  - Nil
			&g_smidKensaku_sc4,		// ID of shooting comment  - Nil     
			&g_smidKensaku_r1,		// ID of random comment        
			&g_smidKensaku_r2,		// ID of random comment         
			&g_smidKensaku_r3,		// ID of random comment         
			&g_smidKensaku_r4,		// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::TotalIDs,			// Weapon to fallback on when none available.
			},

			Personatorium::Crawl,	// Crawl face down when writhing
			Personatorium::Civilian,// Victim
			1000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			3000,							// time between reaction to being shot.
			50,							// Initial hit points

		},

	/////////////////////////////// JPMaleYutaka //////////////////////////////////

		{
		// User level description of person.
		"JP Male Yutaka",						// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/man",					// Example: "Miner", as in "3d/MINER_shoot.sop".         
			3,								// Texture to use
			"hand",						// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"hand",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"guy",						// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidYutaka_h1,			// ID of 'hit by weapon' sample.
			&g_smidYutaka_h2,			// ID of 'hit by weapon' sample,
			&g_smidYutaka_h3,			// ID of 'hit by weapon' sample,
			&g_smidYutaka_h4,			// ID of 'hit by weapon' sample,
			&g_smidYutaka_b1,			// ID of 'blown up' sample.     
			&g_smidYutaka_b2,			// ID of 'blown up' sample.     
			&g_smidYutaka_o1,			// ID of 'on fire' sample
			&g_smidYutaka_o2,			// ID of 'on fire' sample
			&g_smidYutaka_ss1,		// ID of suffering sample 1.
			&g_smidYutaka_ss2,		// ID of suffering sample 2.
			&g_smidYutaka_ss3,		// ID of suffering sample 3.
			&g_smidYutaka_ss4,		// ID of suffering sample 4.
			&g_smidYutaka_d1,			// ID of dying sample.          
			&g_smidYutaka_d2,			// ID of dying sample.          
			&g_smidYutaka_sc1,		// ID of shooting comment.- Nil
			&g_smidYutaka_sc2,		// ID of shooting comment.- Nil
			&g_smidYutaka_sc3,		// ID of shooting comment - Nil
			&g_smidYutaka_sc4,		// ID of shooting comment - Nil      
			&g_smidYutaka_r1,			// ID of random comment        
			&g_smidYutaka_r2,			// ID of random comment         
			&g_smidYutaka_r3,			// ID of random comment         
			&g_smidYutaka_r4,			// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::TotalIDs,			// Weapon to fallback on when none available.
			},

			Personatorium::Crawl,	// Crawl face down when writhing
			Personatorium::Civilian,// Victim
			1000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			3000,							// time between reaction to being shot.
			50,							// Initial hit points

		},

	/////////////////////////////// JPMaleOsamu //////////////////////////////////

		{
		// User level description of person.
		"JP Male Osamu",						// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/man",					// Example: "Miner", as in "3d/MINER_shoot.sop".         
			4,								// Texture to use
			"hand",						// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"hand",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"guy",						// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidOsamu_h1,			// ID of 'hit by weapon' sample.
			&g_smidOsamu_h2,			// ID of 'hit by weapon' sample,
			&g_smidOsamu_h3,			// ID of 'hit by weapon' sample,
			&g_smidOsamu_h4,			// ID of 'hit by weapon' sample,
			&g_smidOsamu_b1,			// ID of 'blown up' sample.     
			&g_smidOsamu_b2,			// ID of 'blown up' sample.     
			&g_smidOsamu_o1,			// ID of 'on fire' sample
			&g_smidOsamu_o2,			// ID of 'on fire' sample
			&g_smidOsamu_ss1,			// ID of suffering sample 1.
			&g_smidOsamu_ss2,			// ID of suffering sample 2.
			&g_smidOsamu_ss3,			// ID of suffering sample 3.
			&g_smidOsamu_ss4,			// ID of suffering sample 4.
			&g_smidOsamu_d1,			// ID of dying sample.          
			&g_smidOsamu_d2,			// ID of dying sample.          
			&g_smidOsamu_sc1,			// ID of shooting comment.Nil
			&g_smidOsamu_sc2,			// ID of shooting comment.Nil
			&g_smidOsamu_sc3,			// ID of shooting comment Nil
			&g_smidOsamu_sc4,			// ID of shooting comment Nil
			&g_smidOsamu_r1,			// ID of random comment        
			&g_smidOsamu_r2,			// ID of random comment         
			&g_smidOsamu_r3,			// ID of random comment         
			&g_smidOsamu_r4,			// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::TotalIDs,			// Weapon to fallback on when none available.
			},

			Personatorium::Crawl,	// Crawl face down when writhing
			Personatorium::Civilian,// Victim
			1000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			3000,							// time between reaction to being shot.
			50,							// Initial hit points

		},


	/////////////////////////////// JPFemaleSakura //////////////////////////////////

		{
		// User level description of person.
		"JP Female Sakura",							// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/woman",					// Example: "Miner", as in "3d/MINER_shoot.sop".         
			2,								// Texture to use
			"hand",						// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"hand",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"woman",						// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidSakura_h1,			// ID of 'hit by weapon' sample.
			&g_smidSakura_h2,			// ID of 'hit by weapon' sample,
			&g_smidSakura_h3,			// ID of 'hit by weapon' sample,
			&g_smidSakura_h4,			// ID of 'hit by weapon' sample,
			&g_smidSakura_b1,			// ID of 'blown up' sample.     
			&g_smidSakura_b2,			// ID of 'blown up' sample.     
			&g_smidSakura_o1,			// ID of 'on fire' sample
			&g_smidSakura_o2,			// ID of 'on fire' sample
			&g_smidSakura_ss1,		// ID of suffering sample 1.
			&g_smidSakura_ss2,		// ID of suffering sample 2.
			&g_smidSakura_ss3,		// ID of suffering sample 3.
			&g_smidSakura_ss4,		// ID of suffering sample 4.
			&g_smidSakura_d1,			// ID of dying sample.          
			&g_smidSakura_d2,		   // ID of dying sample.          
			&g_smidSakura_sc1,		// ID of shooting comment.Nil
			&g_smidSakura_sc2,		// ID of shooting comment.Nil
			&g_smidSakura_sc3,		// ID of shooting comment Nil
			&g_smidSakura_sc4,		// ID of shooting comment Nil
			&g_smidSakura_r1,			// ID of random comment        
			&g_smidSakura_r2,			// ID of random comment         
			&g_smidSakura_r3,			// ID of random comment         
			&g_smidSakura_r4,			// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::TotalIDs,			// Weapon to fallback on when none available.
			},

			Personatorium::Crawl,	// Crawl face down when writhing
			Personatorium::Civilian,// Victim
			1000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			3000,							// time between reaction to being shot.
			50,							// Initial hit points

		},



	/////////////////////////////// JPWomanTomiko //////////////////////////////////

		{
		// User level description of person.
		"JP Female Tomiko",							// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/woman",					// Example: "Miner", as in "3d/MINER_shoot.sop".         
			3,								// Texture to use
			"hand",						// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"hand",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"woman",						// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidTomiko_h1,			// ID of 'hit by weapon' sample.
			&g_smidTomiko_h2,			// ID of 'hit by weapon' sample,
			&g_smidTomiko_h3,			// ID of 'hit by weapon' sample,
			&g_smidTomiko_h4,			// ID of 'hit by weapon' sample,
			&g_smidTomiko_b1,			// ID of 'blown up' sample.     
			&g_smidTomiko_b2,			// ID of 'blown up' sample.     
			&g_smidTomiko_o1,			// ID of 'on fire' sample
			&g_smidTomiko_o2,			// ID of 'on fire' sample
			&g_smidTomiko_ss1,		// ID of suffering sample 1.
			&g_smidTomiko_ss2,		// ID of suffering sample 2.
			&g_smidTomiko_ss3,		// ID of suffering sample 3.
			&g_smidTomiko_ss4,		// ID of suffering sample 4.
			&g_smidTomiko_d1,			// ID of dying sample.          
			&g_smidTomiko_d2,			// ID of dying sample.          
			&g_smidTomiko_sc1,		// ID of shooting comment.Nil
			&g_smidTomiko_sc2,		// ID of shooting comment.Nil
			&g_smidTomiko_sc3,		// ID of shooting comment Nil
			&g_smidTomiko_sc4,		// ID of shooting comment Nil      
			&g_smidTomiko_r1,			// ID of random comment        
			&g_smidTomiko_r2,			// ID of random comment         
			&g_smidTomiko_r3,			// ID of random comment         
			&g_smidTomiko_r4,			// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::TotalIDs,			// Weapon to fallback on when none available.
			},

			Personatorium::Crawl,	// Crawl face down when writhing
			Personatorium::Civilian,// Victim
			1000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			3000,							// time between reaction to being shot.
			50,							// Initial hit points

		},


	/////////////////////////////// JPFemaleAsami //////////////////////////////////

		{
		// User level description of person.
		"JP Female Asami",							// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/woman",					// Example: "Miner", as in "3d/MINER_shoot.sop".         
			4,								// Texture to use
			"hand",						// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"hand",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"woman",						// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidAsami_h1,			// ID of 'hit by weapon' sample.
			&g_smidAsami_h2,			// ID of 'hit by weapon' sample,
			&g_smidAsami_h3,			// ID of 'hit by weapon' sample,
			&g_smidAsami_h4,			// ID of 'hit by weapon' sample,
			&g_smidAsami_b1,			// ID of 'blown up' sample.     
			&g_smidAsami_b2,			// ID of 'blown up' sample.     
			&g_smidAsami_o1,			// ID of 'on fire' sample
			&g_smidAsami_o2,			// ID of 'on fire' sample
			&g_smidAsami_ss1,			// ID of suffering sample 1.
			&g_smidAsami_ss2,			// ID of suffering sample 2.
			&g_smidAsami_ss3,			// ID of suffering sample 3.
			&g_smidAsami_ss4,			// ID of suffering sample 4.
			&g_smidAsami_d1,			// ID of dying sample.          
			&g_smidAsami_d2,			// ID of dying sample.          
			&g_smidAsami_sc1,			// ID of shooting comment.Nil
			&g_smidAsami_sc2,			// ID of shooting comment.Nil
			&g_smidAsami_sc3,			// ID of shooting comment Nil
			&g_smidAsami_sc4,			// ID of shooting comment Nil
			&g_smidAsami_r1,			// ID of random comment        
			&g_smidAsami_r2,			// ID of random comment         
			&g_smidAsami_r3,			// ID of random comment         
			&g_smidAsami_r4,			// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::TotalIDs,			// Weapon to fallback on when none available.
			},

			Personatorium::Crawl,	// Crawl face down when writhing
			Personatorium::Civilian,// Victim
			1000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			3000,							// time between reaction to being shot.
			50,							// Initial hit points

		},


	////////////////////////////////JPCompSales/////////////////////////////////////
		
		{
		// User level description of person.
		"JP Computer Sales",					// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/emp2",					// Example: "Gunner", as in "3d/GUNNER_shoot.sop".         
			12,							// Texture to use
			"hand",						// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"hand",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"employ2",					// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidCompsales_h1,		// ID of 'hit by weapon' sample.
			&g_smidCompsales_h2,		// ID of 'hit by weapon' sample,
			&g_smidCompsales_h3,		// ID of 'hit by weapon' sample,
			&g_smidCompsales_h4,		// ID of 'hit by weapon' sample,
			&g_smidCompsales_b1,		// ID of 'blown up' sample.     
			&g_smidCompsales_b2,		// ID of 'blown up' sample.     
			&g_smidCompsales_o1,		// ID of 'on fire' sample
			&g_smidCompsales_o2,		// ID of 'on fire' sample
			&g_smidCompsales_ss1,	// ID of suffering sample 1.
			&g_smidCompsales_ss2,	// ID of suffering sample 2.
			&g_smidCompsales_ss3,	// ID of suffering sample 3.
			&g_smidCompsales_ss4,	// ID of suffering sample 4.
			&g_smidCompsales_d1,		// ID of dying sample.          
			&g_smidCompsales_d2,		// ID of dying sample.          
			&g_smidCompsales_sc1,   // ID of shooting comment.          
			&g_smidCompsales_sc2,	// ID of shooting comment.
			&g_smidCompsales_sc3,	// ID of shooting comment
			&g_smidCompsales_sc4,	// ID of shooting comment       
			&g_smidCompsales_r1,		// ID of random comment        
			&g_smidCompsales_r2,		// ID of random comment         
			&g_smidCompsales_r3,		// ID of random comment         
			&g_smidCompsales_r4,		// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::TotalIDs,		// Weapon to fallback on when none available.
			},

			Personatorium::Crawl,	// Crawl on knees when writhing
			Personatorium::Civilian,// Victim
			3000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			2000,							// time between reaction to being shot.
			50,							// Initial hit points

		},


	////////////////////////////////JPCompShopper///////////////////////////////////
		
		{
		// User level description of person.
		"JP Computer Shopper",					// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/emp2",					// Example: "Gunner", as in "3d/GUNNER_shoot.sop".         
			13,								// Texture to use
			"hand",						// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"hand",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"employ2",					// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidCompshop_h1,		// ID of 'hit by weapon' sample.
			&g_smidCompshop_h2,		// ID of 'hit by weapon' sample,
			&g_smidCompshop_h3,		// ID of 'hit by weapon' sample,
			&g_smidCompshop_h4,		// ID of 'hit by weapon' sample,
			&g_smidCompshop_b1,		// ID of 'blown up' sample.     
			&g_smidCompshop_b2,		// ID of 'blown up' sample.     
			&g_smidCompshop_o1,		// ID of 'on fire' sample
			&g_smidCompshop_o2,		// ID of 'on fire' sample
			&g_smidCompshop_ss1,		// ID of suffering sample 1.
			&g_smidCompshop_ss2,		// ID of suffering sample 2.
			&g_smidCompshop_ss3,		// ID of suffering sample 3.
			&g_smidCompshop_ss4,		// ID of suffering sample 4.
			&g_smidCompshop_d1,		// ID of dying sample.          
			&g_smidCompshop_d2,		// ID of dying sample.          
			&g_smidCompshop_sc1,		// ID of shooting comment.Nil
			&g_smidCompshop_sc2,		// ID of shooting comment.Nil
			&g_smidCompshop_sc3,		// ID of shooting comment Nil
			&g_smidCompshop_sc4,		// ID of shooting comment Nil
			&g_smidCompshop_r1,		// ID of random comment        
			&g_smidCompshop_r2,		// ID of random comment         
			&g_smidCompshop_r3,		// ID of random comment         
			&g_smidCompshop_r4,		// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::TotalIDs,		// Weapon to fallback on when none available.
			},

			Personatorium::Crawl,	// Crawl on knees when writhing
			Personatorium::Civilian,// Victim
			3000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			2000,							// time between reaction to being shot.
			50,							// Initial hit points

		},


	/////////////////////////////////JPPoliceKazuki///////////////////////////////////////
		
		{
		// User level description of person.
		"JP Officer Kazuki",					// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/cop",					// Example: "Gunner", as in "3d/GUNNER_shoot.sop".         
			3,								// Texture to use
			"pistoltip",				// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"gun",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"cop",						// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidKazuki_h1,			// ID of 'hit by weapon' sample.
			&g_smidKazuki_h2,			// ID of 'hit by weapon' sample,
			&g_smidKazuki_h3,			// ID of 'hit by weapon' sample,
			&g_smidKazuki_h4,			// ID of 'hit by weapon' sample,
			&g_smidKazuki_b1,			// ID of 'blown up' sample.     
			&g_smidKazuki_b2,			// ID of 'blown up' sample.     
			&g_smidKazuki_o1,			// ID of 'on fire' sample
			&g_smidKazuki_o2,			// ID of 'on fire' sample
			&g_smidKazuki_ss1,		// ID of suffering sample 1.
			&g_smidKazuki_ss2,		// ID of suffering sample 2.
			&g_smidKazuki_ss3,		// ID of suffering sample 3.
			&g_smidKazuki_ss4,		// ID of suffering sample 4.
			&g_smidKazuki_d1,			// ID of dying sample.          
			&g_smidKazuki_d2,			// ID of dying sample.          
			&g_smidKazuki_sc1,		// ID of shooting comment.          
			&g_smidKazuki_sc2,		// ID of shooting comment.
			&g_smidKazuki_sc3,		// ID of shooting comment
			&g_smidKazuki_sc4,		// ID of shooting comment       
			&g_smidKazuki_r1,			// ID of random comment        
			&g_smidKazuki_r2,			// ID of random comment         
			&g_smidKazuki_r3,			// ID of random comment         
			&g_smidKazuki_r4,			// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::CPistolID,		// Weapon to fallback on when none available.
			},

			Personatorium::Crawl,	// Crawl on knees when writhing
			Personatorium::Hostile,	// Enemy
			3000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			2000,							// time between reaction to being shot.
			125,							// Initial hit points

		},

	/////////////////////////////////JPPoliceNoboru///////////////////////////////////////
		
		{
		// User level description of person.
		"JP Officer Noboru",					// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/cop",					// Example: "Gunner", as in "3d/GUNNER_shoot.sop".         
			3,								// Texture to use
			"pistoltip",				// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"gun",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"cop",						// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidNoboru_h1,			// ID of 'hit by weapon' sample.
			&g_smidNoboru_h2,			// ID of 'hit by weapon' sample,
			&g_smidNoboru_h3,			// ID of 'hit by weapon' sample,
			&g_smidNoboru_h4,			// ID of 'hit by weapon' sample,
			&g_smidNoboru_b1,			// ID of 'blown up' sample.     
			&g_smidNoboru_b2,			// ID of 'blown up' sample.     
			&g_smidNoboru_o1,			// ID of 'on fire' sample
			&g_smidNoboru_o2,			// ID of 'on fire' sample
			&g_smidNoboru_ss1,		// ID of suffering sample 1.
			&g_smidNoboru_ss2,		// ID of suffering sample 2.
			&g_smidNoboru_ss3,		// ID of suffering sample 3.
			&g_smidNoboru_ss4,		// ID of suffering sample 4.
			&g_smidNoboru_d1,			// ID of dying sample.          
			&g_smidNoboru_d2,			// ID of dying sample.          
			&g_smidNoboru_sc1,		// ID of shooting comment.          
			&g_smidNoboru_sc2,		// ID of shooting comment.
			&g_smidNoboru_sc3,		// ID of shooting comment
			&g_smidNoboru_sc4,		// ID of shooting comment       
			&g_smidNoboru_r1,			// ID of random comment        
			&g_smidNoboru_r2,			// ID of random comment         
			&g_smidNoboru_r3,			// ID of random comment         
			&g_smidNoboru_r4,			// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::CPistolID,		// Weapon to fallback on when none available.
			},

			Personatorium::Crawl,	// Crawl on knees when writhing
			Personatorium::Hostile,	// Enemy
			3000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			2000,							// time between reaction to being shot.
			125,							// Initial hit points

		},


	////////////////////////////////JPOldWomanAyame/////////////////////////////////////
		
		{
		// User level description of person.
		"JP Old Woman Ayame",						// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/gramps",				// Example: "Gunner", as in "3d/GUNNER_shoot.sop".         
			6,								// Texture to use
			"hand",						// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"hand",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"gramps",						// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidAyame_h1,				// ID of 'hit by weapon' sample.
			&g_smidAyame_h2,				// ID of 'hit by weapon' sample,
			&g_smidAyame_h3,				// ID of 'hit by weapon' sample,
			&g_smidAyame_h4,				// ID of 'hit by weapon' sample,
			&g_smidAyame_b1,				// ID of 'blown up' sample.     
			&g_smidAyame_b2,				// ID of 'blown up' sample.     
			&g_smidAyame_o1,				// ID of 'on fire' sample
			&g_smidAyame_o2,				// ID of 'on fire' sample
			&g_smidAyame_ss1,				// ID of suffering sample 1.
			&g_smidAyame_ss2,				// ID of suffering sample 2.
			&g_smidAyame_ss3,				// ID of suffering sample 3.
			&g_smidAyame_ss4,				// ID of suffering sample 4.
			&g_smidAyame_d1,				// ID of dying sample.          
			&g_smidAyame_d2,				// ID of dying sample.          
			&g_smidAyame_sc1,				// ID of shooting comment.Nil
			&g_smidAyame_sc2,				// ID of shooting comment.Nil
			&g_smidAyame_sc3,				// ID of shooting comment Nil
			&g_smidAyame_sc4,				// ID of shooting comment Nil       
			&g_smidAyame_r1,				// ID of random comment        
			&g_smidAyame_r2,				// ID of random comment         
			&g_smidAyame_r3,				// ID of random comment         
			&g_smidAyame_r4,				// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::TotalIDs,		// Weapon to fallback on when none available.
			},

			Personatorium::Crawl,	// Crawl on knees when writhing
			Personatorium::Civilian,// Victim
			3000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			2000,							// time between reaction to being shot.
			50,							// Initial hit points

		},

	////////////////////////////////JPOldWomanShinobu/////////////////////////////////////
		
		{
		// User level description of person.
		"JP Old Woman Shinobu",						// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/gramps",				// Example: "Gunner", as in "3d/GUNNER_shoot.sop".         
			7,								// Texture to use
			"hand",						// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"hand",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"gramps",						// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidShinobu_h1,			// ID of 'hit by weapon' sample.
			&g_smidShinobu_h2,			// ID of 'hit by weapon' sample,
			&g_smidShinobu_h3,			// ID of 'hit by weapon' sample,
			&g_smidShinobu_h4,			// ID of 'hit by weapon' sample,
			&g_smidShinobu_b1,			// ID of 'blown up' sample.     
			&g_smidShinobu_b2,			// ID of 'blown up' sample.     
			&g_smidShinobu_o1,			// ID of 'on fire' sample
			&g_smidShinobu_o2,			// ID of 'on fire' sample
			&g_smidShinobu_ss1,			// ID of suffering sample 1.
			&g_smidShinobu_ss2,			// ID of suffering sample 2.
			&g_smidShinobu_ss3,			// ID of suffering sample 3.
			&g_smidShinobu_ss4,			// ID of suffering sample 4.
			&g_smidShinobu_d1,			// ID of dying sample.          
			&g_smidShinobu_d2,			// ID of dying sample.          
			&g_smidShinobu_sc1,		   // ID of shooting comment.Nil
			&g_smidShinobu_sc2,			// ID of shooting comment.Nil
			&g_smidShinobu_sc3,			// ID of shooting comment Nil
			&g_smidShinobu_sc4,			// ID of shooting comment Nil
			&g_smidShinobu_r1,			// ID of random comment        
			&g_smidShinobu_r2,			// ID of random comment         
			&g_smidShinobu_r3,			// ID of random comment         
			&g_smidShinobu_r4,			// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::TotalIDs,		// Weapon to fallback on when none available.
			},

			Personatorium::Crawl,	// Crawl on knees when writhing
			Personatorium::Civilian,// Victim
			3000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			2000,							// time between reaction to being shot.
			50,							// Initial hit points

		},

	////////////////////////////////JPOldManRyuichi/////////////////////////////////////
		
		{
		// User level description of person.
		"JP Old Man Ryuichi",						// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/gramps",				// Example: "Gunner", as in "3d/GUNNER_shoot.sop".         
			8,							// Texture to use
			"hand",						// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"hand",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"gramps",						// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidRyuichi_h1,		// ID of 'hit by weapon' sample.
			&g_smidRyuichi_h2,		// ID of 'hit by weapon' sample,
			&g_smidRyuichi_h3,		// ID of 'hit by weapon' sample,
			&g_smidRyuichi_h4,		// ID of 'hit by weapon' sample,
			&g_smidRyuichi_b1,		// ID of 'blown up' sample.     
			&g_smidRyuichi_b2,		// ID of 'blown up' sample.     
			&g_smidRyuichi_o1,		// ID of 'on fire' sample
			&g_smidRyuichi_o2,		// ID of 'on fire' sample
			&g_smidRyuichi_ss1,		// ID of suffering sample 1.
			&g_smidRyuichi_ss2,		// ID of suffering sample 2.
			&g_smidRyuichi_ss3,		// ID of suffering sample 3.
			&g_smidRyuichi_ss4,		// ID of suffering sample 4.
			&g_smidRyuichi_d1,		// ID of dying sample.          
			&g_smidRyuichi_d2,		// ID of dying sample.          
			&g_smidRyuichi_sc1,	   // ID of shooting comment.Nil
			&g_smidRyuichi_sc2,		// ID of shooting comment.Nil
			&g_smidRyuichi_sc3,		// ID of shooting comment Nil
			&g_smidRyuichi_sc4,		// ID of shooting comment Nil    
			&g_smidRyuichi_r1,		// ID of random comment        
			&g_smidRyuichi_r2,		// ID of random comment         
			&g_smidRyuichi_r3,		// ID of random comment         
			&g_smidRyuichi_r4,		// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::TotalIDs,		// Weapon to fallback on when none available.
			},

			Personatorium::Crawl,	// Crawl on knees when writhing
			Personatorium::Civilian,// Victim
			3000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			2000,							// time between reaction to being shot.
			50,							// Initial hit points

		},

	////////////////////////////////JPOldManTadao/////////////////////////////////////
		
		{
		// User level description of person.
		"JP Old Man Tadao",						// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/gramps",				// Example: "Gunner", as in "3d/GUNNER_shoot.sop".         
			9,								// Texture to use
			"hand",						// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"hand",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"gramps",						// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidTadao_h1,			// ID of 'hit by weapon' sample.
			&g_smidTadao_h2,			// ID of 'hit by weapon' sample,
			&g_smidTadao_h3,			// ID of 'hit by weapon' sample,
			&g_smidTadao_h4,			// ID of 'hit by weapon' sample,
			&g_smidTadao_b1,			// ID of 'blown up' sample.     
			&g_smidTadao_b2,			// ID of 'blown up' sample.     
			&g_smidTadao_o1,			// ID of 'on fire' sample
			&g_smidTadao_o2,			// ID of 'on fire' sample
			&g_smidTadao_ss1,			// ID of suffering sample 1.
			&g_smidTadao_ss2,			// ID of suffering sample 2.
			&g_smidTadao_ss3,			// ID of suffering sample 3.
			&g_smidTadao_ss4,			// ID of suffering sample 4.
			&g_smidTadao_d1,			// ID of dying sample.          
			&g_smidTadao_d2,			// ID of dying sample.          
			&g_smidTadao_sc1,		   // ID of shooting comment.Nil
			&g_smidTadao_sc2,			// ID of shooting comment.Nil
			&g_smidTadao_sc3,			// ID of shooting comment Nil
			&g_smidTadao_sc4,			// ID of shooting comment Nil
			&g_smidTadao_r1,			// ID of random comment        
			&g_smidTadao_r2,			// ID of random comment         
			&g_smidTadao_r3,			// ID of random comment         
			&g_smidTadao_r4,			// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::TotalIDs,		// Weapon to fallback on when none available.
			},

			Personatorium::Crawl,	// Crawl on knees when writhing
			Personatorium::Civilian,// Victim
			3000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			2000,							// time between reaction to being shot.
			50,							// Initial hit points

		},

	////////////////////////////////JPOsakaSales///////////////////////////////////
		
		{
		// User level description of person.
		"JP Osaka Salesman",					// pszDescription.

		// Names used to create animation resource filenames.
			{
			"3d/emp2",					// Example: "Gunner", as in "3d/GUNNER_shoot.sop".         
			15,								// Texture to use
			"hand",						// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			"hand",						// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			"employ2",					// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			},

		// Sound effects for this person.
			{
			&g_smidOsales_h1,			// ID of 'hit by weapon' sample.
			&g_smidOsales_h2,			// ID of 'hit by weapon' sample,
			&g_smidOsales_h3,			// ID of 'hit by weapon' sample,
			&g_smidOsales_h4,			// ID of 'hit by weapon' sample,
			&g_smidOsales_b1,			// ID of 'blown up' sample.     
			&g_smidOsales_b2,			// ID of 'blown up' sample.     
			&g_smidOsales_o1,			// ID of 'on fire' sample
			&g_smidOsales_o2,			// ID of 'on fire' sample
			&g_smidOsales_ss1,		// ID of suffering sample 1.
			&g_smidOsales_ss2,		// ID of suffering sample 2.
			&g_smidOsales_ss3,		// ID of suffering sample 3.
			&g_smidOsales_ss4,		// ID of suffering sample 4.
			&g_smidOsales_d1,			// ID of dying sample.          
			&g_smidOsales_d2,			// ID of dying sample.          
			&g_smidOsales_sc1,		// ID of shooting comment.          
			&g_smidOsales_sc2,		// ID of shooting comment.
			&g_smidOsales_sc3,		// ID of shooting comment
			&g_smidOsales_sc4,		// ID of shooting comment       
			&g_smidOsales_r1,			// ID of random comment        
			&g_smidOsales_r2,			// ID of random comment         
			&g_smidOsales_r3,			// ID of random comment         
			&g_smidOsales_r4,			// ID of random comment         
			},

		// Weapons for this person.  Currently, only one supported.
			{
			CThing::TotalIDs,		// Weapon to fallback on when none available.
			},

			Personatorium::Crawl,	// Crawl on knees when writhing
			Personatorium::Civilian,// Victim
			3000,							// Reaction time for intruders
			2000,							// Time between shots in run & shoot
			2000,							// time between reaction to being shot.
			50,							// Initial hit points

		},
#endif


	};

//////////////////////////////////////////////////////////////////////////////
// Module specific (static) variables / Instantiate class statics.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Module specific (static) protos.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Functions.
//////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// EOF
///////////////////////////////////////////////////////////////////////////////
