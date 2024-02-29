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
// Personatorium.H
// Project: Postal
// 
// History:
//		04/28/97 JMI	Started.
//
//		04/29/97 BRH	Removed the verb from the structure since there are
//							many verbs, one for each action that the person 
//							can do, so the verb is best left up to the 
//							GetResources() function in CPerson.
//
//		05/19/97	JMI	Removed Woman from the enum.  She did not have enough
//							animations so she was removed from the g_apersons.
//
//		06/11/97	JMI	Added pszEventName to the Personatorium for specifying
//							event name for person animations.
//
//		07/01/97 BRH	Added Military guy.
//
//		07/03/97 BRH	Added Construction worker.
//
//		07/04/97 BRH	Added man and woman victim.
//
//		07/23/97 BRH	Added three new parameters for timing so that different
//							people can be tuned differently.  This came from a 
//							request to have some people react more/less to bullet
//							shots, and having Rocketmen react quicker in guard mode
//							since it takes them longer to shoot.
//
//		07/25/97 BRH	Added initial hit points so that victims can have 
//							different amount of hit points, swat can have more,
//							etc.
//
//		08/07/97	JMI	Added names for 'hand' transform channels.
//
//		08/09/97 BRH	Added more person types for new voices.
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
//		08/13/97 BRH	Added two more person types for different victim 
//							textures.
//
//		12/17/97	JMI	Added some new persons:
//							Employee 1 & 2, Red Cross 1 & 2, Golfer 1 & 2, Lawyer,
//							Bum 1 & 2, Vet 1 & 2, Nude 1 & 2, Gramps 1 & 2, Granny
//							1, 2, & 3.
//
//		01/19/98	BRH	Added shopper versions of the Golfer, Bum and Vet so
//							that they look different but say the normal victim
//							stuff that is non level specific.
//
//		10/06/99 BRH	Adding japanese characters for the Japan expansion pack.
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

#ifndef PERSONATORIUM_H
#define PERSONATORIUM_H

//////////////////////////////////////////////////////////////////////////////
// C Headers -- Must be included before RSPiX.h b/c RSPiX utilizes SHMalloc.
//////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// RSPiX Headers.
///////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Postal headers.
//////////////////////////////////////////////////////////////////////////////

#include "thing.h"
#include "SampleMaster.h"

//////////////////////////////////////////////////////////////////////////////
// Macros.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Protos.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Typedefs.
//////////////////////////////////////////////////////////////////////////////
typedef struct
	{
	///////////////////////////////////////////////////////////////////////////
	// Typedefs/enums.
	///////////////////////////////////////////////////////////////////////////
	public:

		// This enum has one entry for each index in the g_apersons.
		typedef enum
			{
			Grenader,					// Mimic of classic CGrenader.
			RocketMan,					// Mimic of classic CRocketMan.
			Gunner,						// Mimic of classic CGunner.
			Cop,							// Mimic of classic CCop.
			Miner,						// Miner that acts like the Grenader
			Swat,							// Swat team member
			Kid,							// Kid that can shoot
			Protestor,					// Marching protestor
			HickCop,						// New sound theme for Cop.
			Military,					// Military guy
			Construction,				// Construction worker
			TheMan,						// Man victim
			Woman,						// Woman victim
			ConstructionCarlos,		// Mexican worker
			ConstructionArmando,		// Hispanic worker
			GrenaderPepe,				// Hispanic grenader
			GrenaderLarry,				
			RocketmanDexter,
			RocketmanPaco,
			GunnerHeidi,
			GunnerLinda,
			CopBrian,
			CopHector,
			MinerGary,
			MinerFernando,
			SwatCharlie,
			SwatMatt,
			SheriffDale,
			ManFrank,
			WomanBetty,
			SheriffNorm,
			Employee1,
			Employee2,
			RedCross1,
			RedCross2,
			Golfer1,
			Golfer2,
			Lawyer,
			Bum1,
			Bum2,
			Vet1,
			Vet2,
			Nude1,
			Nude2,
			Gramps1,
			Gramps2,
			Granny1,
			Granny2,
			Granny3,
			ShopperG1,
			ShopperG2,
			ShopperB1,
			ShopperB2,
			ShopperV1,
			ShopperV2,
			JPMaleKensaku,
			JPMaleYutaka,
			JPMaleOsamu,
			JPFemaleSakura,
			JPFemaleTomiko,
			JPFemaleAsami,
			JPCompSales,
			JPCompShopper,
			JPPoliceKazuki,
			JPPoliceNoboru,
			JPOldWomanAyame,
			JPOldWomanShinobu,
			JPOldManRyuichi,
			JPOldManTadao,
			JPOsakaSales,


			// Add new indices above.
			NumPersons	// Number of indices/Personatoriums in the g_apersons.
			} Index;

		// This is the type of writhing motion they use
		typedef enum
			{
			Still,
			Crawl,
			PushBack,
			EndOfWrithingMotions
			} WrithingMotion;

		typedef enum
			{
			Civilian,
			Hostile,
			EndOfLifestyles
			} Lifestyle;

	///////////////////////////////////////////////////////////////////////////
	// Instantiable data.
	///////////////////////////////////////////////////////////////////////////
	public:

		// User level description of person.
		char*	pszDescription;		// Example: "Gunner Gal", as in "Place a GUNNER GAL".
		
		// Animation strings for this person.
		struct
			{
			// Names used to create animation resource filenames.
			char* pszBaseName;		// Example: "Gunner", as in "3d/GUNNER_shoot.sop".
			short sTextureScheme;	// Example: 0 would make "3d/GUNNER0.tex"
			char*	pszObjectName;		// Name of channels of transforms for ammo.
											// Example: "Bullet", as in "3d/gunner_shoot_BULLET.trans".
			char*	pszHandName;		// Name of channels of transforms for hand.
											// Example: "hand", as in "3d/gunner_shoot_HAND.trans"
			char*	pszEventName;		// Example: "main", as in "3d/gunner_shoot_MAIN.event".
			} Anim;

		// Sound effects for this person.
		struct
			{
			SampleMasterID*	psmidShot1;			// ID of 'hit by weapon' sample.
			SampleMasterID*	psmidShot2;			// ID of 'hit by weapon' sample
			SampleMasterID*	psmidShot3;			// ID of 'hit by weapon' sample
			SampleMasterID*	psmidShot4;			// ID of 'hit by weapon' sample
			SampleMasterID*	psmidBlownup1;		// ID of 'blown up' sample.
			SampleMasterID*	psmidBlownup2;		// ID of 'blown up' sample.
			SampleMasterID*	psmidBurning1;		// ID of burning sample.
			SampleMasterID*	psmidBurning2;		// ID of burning sample
			SampleMasterID*	psmidSuffering1;	// ID of suffering sample 1.
			SampleMasterID*	psmidSuffering2;	// ID of suffering sample 2.
			SampleMasterID*	psmidSuffering3;	// ID of suffering sample 3.
			SampleMasterID*	psmidSuffering4;	// ID of suffering sample 4.
			SampleMasterID*	psmidDying1;		// ID of dying sample.
			SampleMasterID*	psmidDying2;		// ID of dying sample.
			SampleMasterID*	psmidShooting1;	// ID of shooting comment
			SampleMasterID*	psmidShooting2;	// ID of shooting comment
			SampleMasterID*	psmidShooting3;	// ID of shooting comment
			SampleMasterID*	psmidShooting4;	// ID of shooting comment
			SampleMasterID*	psmidRandom1;		// ID of random comment
			SampleMasterID*	psmidRandom2;		// ID of random comment
			SampleMasterID*	psmidRandom3;		// ID of random comment
			SampleMasterID*	psmidRandom4;		// ID of random comment
			} Sample;

		// Weapons for this person.  Currently, only one supported.
		struct
			{
			CThing::ClassIDType	idWeapon1;		// CThing ID of preferred weapon.
			} Weapon;

		// Type of writhing motion
		WrithingMotion eWrithingMotion;			// Still, crawling, or push back
		Lifestyle		eLifestyle;					// 

		// Stats.
		long lGuardTimeout;							// Interval between checking for intruders
		long lRunShootInterval;						// Delay between shots while running
		long lShotTimeout;							// Time to wait before reacting to next shot.
		short sInitialHitPoints;					// Initial hit points for person

		/////// NYI ///////

	} Personatorium;

//////////////////////////////////////////////////////////////////////////////
// Declarations.
//////////////////////////////////////////////////////////////////////////////

// This is the master list of person descriptions that can be used to describe
// a CPerson.
extern Personatorium g_apersons[Personatorium::NumPersons];

#endif	// PERSONATORIUM_H
//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
