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
// SampleMaster.H
// 
// History:
//		01/28/97 JMI	Started.
//
//		02/02/97	JMI	Added functions to determine if samples are playing.
//
//		02/18/97	JMI	Made ms_resmgr g_resmgrSamples and made it globally
//							accessible.
//
//		02/19/97	JMI	Added 3 new samples (shoot and 2 ricochets).
//
//		02/19/97	JMI	Added g_smidDyingYell sample.
//
//		02/24/97	JMI	Added g_smidGrenadeBounce.
//
//		02/24/97	JMI	Changed pop.wav to heartbeat.wav.
//
//		02/24/97 BRH	Added firebomb, fire, and napalm sound effects.
//
//		02/24/97	JMI	Added g_smidShotFemaleGrunt, g_smidBlownupFemaleYell,
//							g_smidBurningFemaleYell.
//
//		02/25/97	JMI	Added g_smidBurningMainGuy.
//
//		03/07/97	JMI	Now PlaySample() returns the RSnd* it played the sample
//							on which can be querried and/or passed to AbortSample().
//
//		03/13/97	JMI	Added g_smidOutOfBullets.
//
//		03/21/97 BRH	Added g_smidBounceLaunch, and g_smidBounceExplode
//
//		03/26/97	JMI	Added g_smidClick.
//
//		04/24/97	JMI	Added g_smidShotgun.
//
//		04/25/97	JMI	Added g_smidWrithing1, 2, 3, and 4.
//
//		04/29/97	JMI	Added g_smidNil, a way of specifying to SampleMaster that
//							it should not bother with this sample.  This is useful
//							for things that require a sample ID.
//
//		05/02/97	JMI	Added g_smidExecute1 and g_smidExecute2.
//
//		05/09/97	JMI	Addee g_smidFlameThrower1 - 4.
//							PlaySample() now takes optional looping parameters which
//							are passed directly on to RSnd::Play().
//
//		05/30/97	JMI	Changed g_smidShotGun to "sound/shotgun.wav" (was 
//							"sound/M16 single shot2.wav").
//
//		06/04/97	JMI	Added AbortAllSamples() which aborts all currently 
//							playing samples.
//
//		06/09/97 BRH	Added demonic voice sounds.
//
//		06/09/97	JMI	Added weapon selection sounds.
//
//		06/11/97 BRH	Added PlaySampleThenPurge function which is a convenient
//							way to pass a new purge parameter to PlaySample.  It then
//							uses the new resource manager's ReleaseAndPurge function
//							rather than the regular release.  This will allow you to
//							purge single samples that you don't want to stay in the
//							cache.  
//
//		06/14/97 BRH	Added more sound effects, still need to add shooting
//							and random comments.
//
//		06/15/97 BRH	Added the rest of the comment sounds.
//
//		06/15/97	JMI	Added g_smidBulletIntoVest for when people with bullet
//							proof vests are shot.
//
//		06/16/97	JMI	Added a version of IsSamplePlaying() that allows one to
//							specify the sound channel to check.
//
//		06/16/97	JMI	Added g_smidBodyImpact2.
//
//		06/16/97 BRH	Added more sound effects for enemy events.
//
//		06/16/97	JMI	Added g_smidPickedUpAmmo, g_smidPickedUpWeapon,
//							g_smidPickedUpHealth, g_smidPickedUpArmor.
//
//		06/17/97	JMI	Added g_smidStep.
//
//		06/17/97	JMI	PlaySample() (and PlaySampleThenPurge() ) now always
//							return an RSnd* (even if they fail) and also, optionally,
//							can return the length of the sample to play.
//
//		07/01/97	JMI	Added g_smidMenuItemChange.
//
//		07/09/97	JMI	Added g_smidTitle.
//
//		07/13/97	JMI	Removed 'sound/' from all sample names.  Now that these
//							sounds are stored in folders named for their sample type
//							it seemed rhetorical and annoying. 
//
//		07/15/97	JRD	Added support for local sound volume by channel and
//							category
//
//		07/17/97 JRD	Moved sound category information out of RSND and into
//							sample master for a more appropriate app vs rspix division.
//
//		07/17/97 JRD	Provided a backwards compatible PlaySample stub so old code
//							will simply compile.
//
//
//		07/17/97 JRD	Couldn't resolve overload.  Forced to call new function
//							PlaySampleEx.  (Bleh!)
//
//		07/17/97	JMI	Added g_smidMusicTester and g_smidAmbientTester.
//							Changed VolumeCode to SoundInstance.  Trying to make it a
//							generic playing sample identifier.
//							Also, PlaySample() no longer returns a ptr to the RSnd
//							reducing the chances we rely on sound for synch.
//
//		07/17/97	JRD	Added functionality to calculate volume based on 3d
//							distance.
//
//		07/18/97	JMI	Added StopLoopingSample() to reduce the need for 
//							GetInstanceChannel().
//
//		07/18/97	JMI	Got rid of bogus immitation PlaySample functions.
//							Now there is one PlaySample() function.  Also, you now
//							MUST specify a category and you don't have to specify a
//							SoundInstance ptr to specify a volume.
//
//		07/30/97	JMI	Added g_smidDeathWadLaunch/Explode/Thrust.
//
//		08/05/97	JMI	Added PauseAllSamples() and ResumeAllSamples().
//
//		08/05/97 JRD	Added CSoundCatalogue and automated the listing
//							of sounds for use with organ.
//
//		08/05/97	JMI	Upped MAX_SOUNDS from 200 to 512 (b/c we have 224 
//							samples mastered currently).
//
//		08/08/97	JMI	Added g_smidTitleMusak.
//
//		08/09/97 BRH	Added PaulR and Rubin's voices
//
//		08/13/97 BRH	Added more voices for grunts and police shouts.
//
//		08/16/97 BRH	Added beep sounds for mine to inidate that is is arming.
//
//		08/20/97 BRH	Added the new volume categories for pain and suffering.
//
//		08/25/97	JMI	Added default volumes for each category in each quality
//							via the ms_asQualityCategoryAdjustors.
//							Also, added enums for sound qualities.
//							Also, added macro enums for UserDefaultVolume, 
//							UserMaxVolume, and MaxVolume.
//
//		08/26/97 BRH	Added Verne's voice and the new barrel sounds.
//
//		08/27/97 BRH	Got rid of the unused samples.  Took out writhing1
//							since it was the same as writing3.
//
//		09/03/97 BRH	Added the real ostrich sounds.
//
//		09/06/97 BRH	Added execution sound.
//
//		09/24/97 BRH	Added LOCALE specific compile switch for UK version
//							and removed the female pain and screaming sounds
//							from the UK version.
//
//		09/24/97	JMI	Added bFemalePain member to SampleMasterID.  This field
//							is true if the sample is of a female in pain.  If this
//							field is true, SampleMaster.cpp won't play the sample
//							if the LOCALE is that of a country that does not allow
//							such things in games (currently just the UK).
//
//		10/07/97	JMI	Changed bFemalePain to usDescFlags, a bits field of flags
//							describing the sound so we can know which ones to filter
//							out for various languages.
//							Changed false to SMDF_NO_DESCRIPT and true to 
//							SMDF_FEMALE_PAIN.
//							Also, added SMDF_POLICE_REF in 2 spots.
//
//		01/06/98	BRH	Added sounds for Add-on pack.
//
//		01/07/98	JMI	Added ASSERT in CSoundCatalogue() so it will detect when
//							it overflows.
//							Also, upped MAX_SOUNDS to 650 (there are currently 626).
//
//		11/20/99	BRH	Added new smid for new Japanese characters.
//
//		12/02/99 MJR	Fixed typos in two of the sound file names.
//
//		12/03/99 MJR	Fixed a few more typos in sound file names.
//
//		03/31/00 MJR	Put conditional compilation around Japanese voices.
//
//////////////////////////////////////////////////////////////////////////////
//
// This module caches and plays samples via the RSnd interface.  To identify
// the samples, a simple struct containing a string called a SampleMasterID is
// used.  The string is actually used as a filename.  The idea, though, is to
// use the globally declared IDs to reference the samples so that the 
// filenames can be changed without effecting any other modules and, if a
// needed sample is removed, the code referring to it should generate a
// compile error.
// Search this file for DEFINE_SAMPLE_ID to find the declarations of all the
// sample IDs you can refer to or add to.  For simplicity's sake, let's define
// them all here.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef SAMPLEMASTER_H
#define SAMPLEMASTER_H

//////////////////////////////////////////////////////////////////////////////
// C Headers -- Must be included before RSPiX.h b/c RSPiX utilizes SHMalloc.
//////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// RSPiX Headers.
///////////////////////////////////////////////////////////////////////////////

#include "RSPiX.h"

#ifdef PATHS_IN_INCLUDES
	#include "WishPiX/ResourceManager/resmgr.h"
#else
	#include "resmgr.h"
#endif

//////////////////////////////////////////////////////////////////////////////
// Macros.
//////////////////////////////////////////////////////////////////////////////

// Useful macro that allows us to extern and instantiate/initialize a global
// in one spot based on whether SAMPLEMASTER_CPP is defined.
#ifdef SAMPLEMASTER_CPP
	#define DEFINE_SAMPLE_ID(usDescFlags, smidVar, strId)		\
			SampleMasterID smidVar = { usDescFlags, strId };	\
			CSoundCatalogue CAT##smidVar(&smidVar)
#else
	#define DEFINE_SAMPLE_ID(usDescFlags, smidVar, strId)	extern SampleMasterID smidVar			// Declare.
#endif // SAMPLEMASTER_CPP

// SampleMaster Description Flags for usDescFlags field.
#define SMDF_NO_DESCRIPT	0x0000	// No description flags.
#define SMDF_FEMALE_PAIN	0x0001	// Indicates pain of a female character.
#define SMDF_POLICE_REF		0x0002	// Contains police reference.

//////////////////////////////////////////////////////////////////////////////
// Typedefs.
//////////////////////////////////////////////////////////////////////////////

// Sample identifier type.  See Data section for details.
typedef struct
	{
	USHORT	usDescFlags;	// Use | to combine SMDF_* flags.
	char*		pszId;
	} SampleMasterID;

// This is a trick so we can force the sounds to catelogue themselves for the organ:
#define MAX_SOUNDS 975

class	CSoundCatalogue
	{
public:
	CSoundCatalogue(SampleMasterID* psmID)
		{
		if (ms_ppsmNameList == NULL)
			{
			ms_ppsmNameList = (SampleMasterID**) 
				calloc(sizeof(SampleMasterID*),MAX_SOUNDS);
			ms_sCurPos = 0;
			}

		ASSERT(ms_sCurPos < MAX_SOUNDS);
//TRACE("curpos = %hd\n", ms_sCurPos);

		ms_ppsmNameList[ms_sCurPos] = psmID;
		ms_sCurPos++;
		ms_sRefCount++;
		}

	~CSoundCatalogue()
		{
		ms_sRefCount--;
		if (ms_sRefCount <= 0)
			{
			ms_sRefCount = 0;
			free(ms_ppsmNameList);
			ms_ppsmNameList = NULL;
			}
		}

	static short NumSounds() { return ms_sCurPos; }

	static short	ms_sCurPos;
	static short	ms_sRefCount;
	static SampleMasterID**	ms_ppsmNameList;
	};

// Just in case MCW doesn't support namespaces yet, we'll use something that
// all C++ compilers consider a valid namespace declaration.
class SampleMaster
	{
	public:
		// Used as a unique idea for running sound sample manipulation -> includes channel number
		typedef	U64	SoundInstance; // 0 indicates error condition.

		//////////////////////////////////////////////////////////////
		// Use these tags to specify a sound volume category
		// That the game player might want to alter.
		//
		// Feel free to change these to anything appropriate
		//
		// Note that there is a global volume which can be tuned,
		// and every sound that is played specifies it's own volume
		//
		// IF YOU ADD A NEW CATEGORY, ADD THE CORRESPONDING STRING
		// DESCRIPTION IN ms_apszSoundCategories (SAMPLEMASTER.CPP).
		// ALSO, add a volume adjustor in ms_asQualityCategoryAdjustors 
		// or it will default to 0.
		typedef	enum
			{
			Unspecified = 0,	// Unaffected, generic sound
			BackgroundMusic,	// Actual solid playing music
			Weapon,				// Gunfire, Missile noise, flame thrower
			UserFeedBack,		// Weapon Select, Out of Ammo, Get PowerUp, etc.
			Destruction,		// Explosions, burning fire
			Ambient,				// Birds, Racial Men, Bars
			Demon,				// The demon's volume.
			Voices,				// The peoples' voices.
			Pain,					// Enemies being blown up, burned, or shot
			Suffering,			// Enemies writing on the ground

			MAX_NUM_SOUND_CATEGORIES
			} SoundCategory;

		//////////////////////////////////////////////////////////////
		// These are the possible qualities.
		// 
		// IF YOU ADD A NEW QUALITY, ADD THE CORRESPONDING volume
		// adjustor in ms_asQualityCategoryAdjustors or it will 
		// default to zero.
		typedef enum
			{
			SQ_11025_8	= 0,
			SQ_11025_16,
			SQ_22050_8,
			SQ_22050_16,
			SQ_44100_8,
			SQ_44100_16,

			NumSoundQualities
			} SoundQuality;

		//////////////////////////////////////////////////////////////
		// SampleMaster specific macros.
		typedef enum
			{
			UserDefaultVolume	= 8,		// Default user volume for all categories/qualities.
			UserMaxVolume		= 10,		// User volume ranges from 0 to 10.
			UserVolumeRange	= UserMaxVolume + 1,	// Ranges from 0 to 10.
			MaxVolume			= 255,	// SampleMaster volume ranges from 0 to 255.
			VolumeRange			= MaxVolume + 1	// Ranges from 0 to 255.

			} Macros;

		//////////////////////////////////////////////////////////////
		// These are the names for the corresponding SoundCategory
		// used as an index.
		static char* ms_apszSoundCategories[MAX_NUM_SOUND_CATEGORIES];

		//////////////////////////////////////////////////////////////
		// These are the default volumes for each category in each
		// quality.
		static short ms_asQualityCategoryAdjustors[NumSoundQualities][MAX_NUM_SOUND_CATEGORIES];
	};

//////////////////////////////////////////////////////////////////////////////
// Data.
//////////////////////////////////////////////////////////////////////////////

// These are the identifiers you pass to any of the sample master functions
// to refer to a particular sample.  The idea is to avoid using actual 
// filenames so that the names can be changed without killing the code logic
// and, if a sample identifier is completely removed, it will cause a compile
// error.
// Also, saves memory by having only one instance of the string.

// Shell.
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidGeneralBeep,			"heartbeat.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidMenuItemSelect,		"611.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidMenuItemChange,		"heartbeat.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidTitle,					"heartbeat.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidMusicTester,			"MusicTester.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidAmbientTester,		"AmbientTester.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidTitleMusak,			"music/Intro&Loop.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidEndingAudio,			"music/endingaudio.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidCreditsMusak,		"music/outro.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidFullBand,				"music/StarsAndStripes.wav");

DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRocketExplode,		"rktexpl.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRocketFire,			"rktfire.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDeathWadLaunch,		"DeathWadLaunch.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDeathWadExplode,	"DeathWadExplode.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDeathWadThrust,		"DeathWadThrust.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidGrenadeExplode,		"greexpl.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidBulletFire,			"38 single shot2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRicochet1,			"bullet ricochet1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRicochet2,			"bullet ricochet2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidBurningYell,			"groan_male2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidGrenadeBounce,		"grenade bounce.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidNapalmHit,			"napalm hit1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidNapalmFire,			"napalm fire2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidNapalmShot,			"napalm shot1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidFireLarge,			"fire large.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidFirebomb,				"firebomb2.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN,	g_smidShotFemaleGrunt,	"scream_woman3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidBurningMainGuy,		"fire2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOutOfBullets,		"click.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidBounceLaunch,		"grenade hit2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidBounceExplode,		"grenade1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidClick,					"click_bone break.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidShotgun,				"shotgun.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidSprayCannon,			"spraycannon.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidMineBeep,				"minebeep.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidMineSet,				"mineset.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidNil,					NULL);	// Ignored by SampleMaster functions.
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidFlameThrower3,		"flamethrower3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidParadeSong,			"parade 1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidExecution,			"execution.wav");

// Demon sounds
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonBleed,			"demon/bleed 2-head.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonBlowup,			"demon/blowup 1-head.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonBurn,			"demon/burn 1-head.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonBurnBaby,		"demon/burnbaby 1-head.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonButtSauce,		"demon/butsauce 1-head.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonSmellChicken,	"demon/chicken 1-head.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonDie1,			"demon/die 1-head.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonDie2,			"demon/die 2-head.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonSissy1,			"demon/dontbe 1-head.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonSissy2,			"demon/sissy 1-head.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonEvil,			"demon/evil 1-head.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonFeelHeat,		"demon/feelheat 2-head.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonGetEm1,			"demon/getem 1-head.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonGetEm2,			"demon/getem 2-head.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonGone1,			"demon/gone 1-head.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonGone2,			"demon/gone 2-head.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonGoodOne,		"demon/good 1-head.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonGoPostal4,		"demon/gopostal 6-head.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonHa,				"demon/ha 1-head.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonHesOut,			"demon/hesout 1-head.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonInHell,			"demon/inhell 1-head.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonLikeItHot,		"demon/ithot 1-head.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonKickAss,		"demon/kickass 1-head.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonLaugh1,			"demon/laugh 1-head.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonLaugh2,			"demon/laugh 2-head.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonLikeYou,		"demon/likeyou 1-head.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonNoRegrets,		"demon/noregrets 1-head.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonOhBaby,			"demon/ohbaby 1-head.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonOJ,				"demon/OJ 1-head.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonOnlyWeapons,	"demon/onlyweapons 1-head.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonPostal,			"demon/postal 1-head.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonSlam,			"demon/slam 1-head.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonThatHurt1,		"demon/thathurt 1-head.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonThatHurt2,		"demon/thathurt 2-head.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonTheGipper,		"demon/thegipper 1-head.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonTheGun,			"demon/thegun 1-head.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonWeapon,			"demon/weaponme 1-head.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonYes1,			"demon/yes 2-head.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonYes2,			"demon/yes 3-head.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonBuckwheat4,	"demon/buckweat 6-head.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonTheMan,			"demon/theman 1-head.wav");
// demon sounds - add on
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonProsecutionRests,"demon/d10-k 06.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonCaseDismissed,	"demon/d10-k 09.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonCheckOutEarly,	"demon/d10-k 16.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonKeepTheChange,	"demon/d10-k 24.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonSatisfactGnty,	"demon/d10-k 32.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonIsThereDoctor,	"demon/d10-k 38.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonBegForThis,		"demon/d10-k 42.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonHoleInOne,			"demon/d1-k 04.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonOutHotTowels,		"demon/d2-k 16.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonWontAffectTip,	"demon/d2-k 19.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonYouNeedMasage,	"demon/d2-k 21.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonDizkneeland,		"demon/d2-k 25.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonKillForMasage,	"demon/d2-k 29.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonTennisBalls,		"demon/d2-k 37.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonHeCheckedOut,		"demon/d2-k 40.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonWhoPeedInPool,	"demon/d2-k 43.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonAltLifestyles,	"demon/d2-k 49.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonRippedBday,		"demon/d3-k 05.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonNudityOffensive,	"demon/d3-k 08.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonNoDecency,			"demon/d3-k 14.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonFreezingWarm,		"demon/d3-k 16.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonWatchItWiggle,	"demon/d3-k 24.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonRichBastards,		"demon/d3-k 28.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonCustomerRight,	"demon/d4-k 08.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonBrownBagBody,		"demon/d4-k 15.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonTenItemsOrLess,	"demon/d4-k 19.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonWhatLaneClosed,	"demon/d4-k 31.wav"); // 31 = now it is, or 36 = it is now
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonBastardsWCoupons,"demon/d4-k 38.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonNoRefunds,			"demon/d4-k 43.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonLikeFreeSample,	"demon/d4-k 50.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonCleanupAisle5,	"demon/d4-k 60.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonBlueLightSpecial,"demon/d4-k 62.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonLowPriceQnty,		"demon/d4-k 66.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonMadeInUSABaby,	"demon/d4-k 70.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonDontTakePlastic, "demon/d4-k 78.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonDontSellPostal1, "demon/d5-k 04.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonDontSellPostal2, "demon/d5-k 05.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonSmellsSourMilk,	"demon/d6-k 02.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonBurningGovtCheese,"demon/d6-k 06.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonPropertyValues,	"demon/d6-k 12.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonWelfareReform,	"demon/d6-k 16.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonDoItQuietly,		"demon/d6-k 23.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonYouBlewItUp,		"demon/d7-k 03.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonRemainStillInjured,"demon/d7-k 09.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonCantHaveAnyNice,	"demon/d7-k 18.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonPinnedDown,		"demon/d7-k 24.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonShakeItUpBaby,	"demon/d7-k 26.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonGrenadeWorksGreat,"demon/d7-k 35.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonSeeYouInHellHa,	"demon/d8-k 05.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonAwwBoBo,			"demon/d8-k 06.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonEatLeadSucker,	"demon/d8-k 12.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonTodayGoodToDie,	"demon/d8-k 15.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonExterminatorsBack,"demon/d8-k 24.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonYoullPayForThat,	"demon/d8-k 28.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonAngelOfDeath,		"demon/d8-k 30.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonShowNoMercy,		"demon/d8-k 37.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonDamnImGood,		"demon/d8-k 53.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonDeathMyMaster,	"demon/d8-k 55.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonDeathMyFriend,	"demon/d8-k 63.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonSmellBurning,		"demon/d8-k 67.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonFeelWrathDog,		"demon/d8-k 75.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonDieLikeDogYouAre,"demon/d8-k 76.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonILoveGoodBBQ,		"demon/d8-k 83.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonJudgeJuryExe,		"demon/d8-k 86.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonIsItHotOrJustMe,	"demon/d8-k 89.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonKillingGoodSoal,	"demon/d9-k 06.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonDieWeakling,		"demon/d9-k 11.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonWhusy,				"demon/d9-k 17.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDemonAllMustDie,		"demon/d9-k 21.wav");




DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidLoadedWeapon,	"loadedWeapon.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidEmptyWeapon,		"emptyWeapon.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidBodyImpact2,		"body impact2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidPickedUpWeapon,	"PickedUpWeapon.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidStep,				"Step.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidBarrelCrash1,	"barrelcrash.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidBarrelCrash2,	"barrelcrash2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidShotBarrel1,		"hitbarrel1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidShotBarrel2,		"hitbarrel2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidShotSentry1,		"sentryhit1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidShotSentry2,		"sentryhit2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidShotSentry3,		"sentryhit3.wav");


// Shot sounds
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidBillGrunt,				"BillGrunt.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN,	g_smidCelinaUg,				"rsp celina#12-1.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN,	g_smidAmyMyEyes,				"rsp amy13.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN,	g_smidAndreaMyLeg,			"rsp andrea-17.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidScottGrunt,				"scott riedle#2-1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidScottMyLeg,				"scott riedle#6-1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidBlownupYell,				"groan_male1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDyingYell, 				"weird scream1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidShotGrunt,				"grunt3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidBulletIntoVest,			"napalm shot1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidHickPrick,				"johnbhickck cop1-46.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidHickAhh2,					"johnbhickck cop1-6.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidMikeGrunt,				"rsp mike r.#2-2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidPaulMyLeg,				"rsp paul#4-10.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRandyHuu,					"rsp randy#2-1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRandyUg,					"rsp randy#2-2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRandyUrhh,				"rsp randy#2-9.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidSteveUh,					"rsp steve#7-1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidSteveUrl,					"rsp steve#7-6.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidPaulRBlewHip,			"10Paul r.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidPaulRShotToe,			"11Paul r.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidPaulRBlewShoulder,		"14Paul r.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidPaulRHuht,				"1Paul r.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidPaulRHuh,					"5Paul r.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidPaulRMyEye,				"8Paul r.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRubinImHit,				"16Rubin 1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRubinAh,					"20Rubin 1.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN,	g_smidDebbieAh,				"DebbieAh.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVinceAhuh,				"VinceAhuh.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVinceHu,					"VinceHu.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOstrichShot,				"OstrichShot.wav");


// Blownup Sounds
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN,	g_smidBlownupFemaleYell,	"scream_woman2.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN,	g_smidCarynScream,			"caryn#4-1.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN,	g_smidAndreaYell,				"rsp andrea-11.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN,	g_smidTinaScream1,			"tina naughton#23-2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidScottYell1,				"scott riedle#1-3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidScottYell2,				"scott riedle#2-2.wav");
				  // g_smidScottGrunt
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidHickAhh1,					"johnbhickck cop1-2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidHickAhhPain,				"johnbhickck cop1-20.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidMikeOhh,					"rsp mike r.#2-5.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidPaulAhah,					"rsp paul#4-4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidSteveAhBlowup,			"rsp steve#7-2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidPaulROoh,					"18Paul r.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidPaulROooh,				"3Paul r.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRubinAhHuh,				"15Rubin 1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRubinUhh,					"18Rubin 1.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN,	g_smidDebbieAhah,				"DebbieAhah.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN,	g_smidDebbieOh,				"DebbieOh.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOstrichBlownup,			"OstrichBlowUp.wav");

// Burning Sounds
				 // g_smidCarynScream
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN,	g_smidAmyScream,			"rsp amy4.wav");    
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN,	g_smidAndreaHelp,			"rsp andrea-1.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN,	g_smidTinaScream2,		"tina naughton#10-1.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN,	g_smidTinaScream3,		"tina naughton#23-1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidScottYell3,			"scott riedle#1-2.wav");
					//g_smidScottYell1
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidScottYell4,			"scott riedle#2-3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidScottHelp,			"scott riedle#2-5.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidHickAhhFire,			"johnbhickck cop1-21.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidHickWhaa,				"johnbhickck cop1-3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidMikeAhh,				"rsp mike r.#2-3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidSteveAhFire,			"rsp steve#7-3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidSteveWaFire,			"rsp steve#7-4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidPaulRHauu,			"4Paul r.wav");
//               g_smidPaulRWaaahoh
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRubinHii,				"17Rubin 1.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN,	g_smidDebbieAhh,			"DebbieAhh.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOstrichBurning,		"OstrichFire.wav");


// Writhing Sounds
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidWrithing2,			"writhing2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidWrithing3,			"writhing3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidWrithing4,			"writhing4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidBillKillMe,			"rsp bill#3-2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidBillHelpMe,			"rsp bill#3-7.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN,	g_smidKimHelp,				"kim#9-2.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN,	g_smidKimCantBreathe,	"kim#9-7.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN,	g_smidAmyCantFeelLegs,	"rsp amy12.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN,	g_smidAmyCantBreathe,	"rsp amy14.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN,	g_smidAmyCantSee,			"rsp amy15.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidScottCoughBlood1,	"scott riedle#1-4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidScottCoughBlood2,	"scott riedle#2-4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidHickOhoo,				"johnbhickck cop1-4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidHickHelpCry,			"johnbhickck cop1-8.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidMikeAhuh,				"rsp mike r.#2-1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidPaulRWaaahoh,		"19Paul r.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidPaulRDragMe,			"15Paul r.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidPaulRTooYoung,		"12Paul r.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidPaulRDontThink,		"13Paul r.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidPaulRCoughBlood,	"7Paul r.wav");

// Dying Sounds
					//g_smidBillHelpMe
					//g_smidKimHelp
					//g_smidKimCantBreathe
					//g_smidAmyMyEyes
					//g_smidAmyCantBreathe
					//g_smidAmyCantSee
					//g_smidScottCoughBlood1
					//g_smidScottCoughBlood2
					//g_smidScottHelp
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidJeffCoughBlood1,		"rsp jeff#2-1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidJeffCoughBlood2,		"rsp jeff#2-2.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN,	g_smidAndreaCantFeelLegs,	"rsp andrea-16.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN,	g_smidAndreaCantBreathe,	"rsp andrea-19.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidScottCantBreathe,		"scott riedle#6-2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidPaulCantFeelLegs,		"rsp paul#4-8.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRandyCantFeelLegs,		"rsp randy#2-8.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidSteveMyEyes,				"rsp steve#7-20.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidSteveMyLeg,				"rsp steve#7-21.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidSteveCantBreathe,		"rsp steve#7-22.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidSteveCantSeeAny,		"rsp steve#7-23.wav");
					//g_smidPaulRDragMe
					//g_smidPaulRTooYoung
					//g_smidPaulRWaaahoh
					//g_smidPaulRDontThink
					//g_smidPaulRCoughBlood
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRubinUhhh,				"21Rubin 1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRubinAhhh,				"22Rubin 1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRubinICantMove,			"19Rubin 1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOstrichDie,				"OstrichDie.wav");

// Shooting Comments
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidBillEatThis,				"rsp bill#3-4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidBillJustDie,				"rsp bill#4-3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidAmyEatThis,				"rsp amy9.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidAndreaOneForMom,		"rsp andrea-10.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidAndreaStickThis,		"rsp andrea-27.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidScottBurn,				"scott riedle#8-2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidScottBurnHim,			"scott riedle#8-3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidHickEatThis,				"johnbhickck cop1-18.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidHickOneForMom,			"johnbhickck cop1-19.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidHickScumbag,				"johnbhickck cop1-45.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidHickBastard,				"johnbhickck cop1-48.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidHickRatBastard,			"johnbhickck cop1-49.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidSteveEatThis,			"rsp steve#7-12.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidSteveOneForMom,			"rsp steve#7-13.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRubinMadallo,			"10Rubin 1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRubinDonwemen,			"12Rubin 1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRubinVominosween,		"13Rubin 1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRubinCallo,				"14Rubin 1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRubinGudelet,			"5Rubin 1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRubinGunBandito,		"9Rubin 1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRubinFinishHimOff,		"3Rubin 1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDebbieBringIt,			"DebbieBringIt.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDebbieDontMakeUs,		"DebbieDontMakeUs.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidBillBetterHope,			"BillBetterHope.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidBillBringItOn,			"BillBringItOn.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidBillConstDieWacko,		"BillConstDieWacko.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidBillDieYouNutcase,		"BillDieYouNutcase.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidBillDropWeapons,		"BillDropWeapons.wav");
DEFINE_SAMPLE_ID(SMDF_POLICE_REF, g_smidBillFreezePolice,		"BillFreezePolice.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidBillGetOnGround,		"BillGetOnGround.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidBillTakeYouOut,			"BillTakeYouOut.wav");
DEFINE_SAMPLE_ID(SMDF_POLICE_REF, g_smidBillThisIsPolice,		"BillThisIsThePolice.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidBillYoureDead,			"BillYoureDead.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVernPutHandsUp,			"VernPutHandsUp.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVernStopFreeze,			"VernStopFreeze.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVernDontMove,			"VernDontMove.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVernFreezeYouDirtbag,	"VernFreezeYouDirtbag.wav");

// Random Comments
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidBillLookout,				"rsp bill#3-3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidBillHesManiac,			"rsp bill#3-9.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidBillBleeding,			"rsp bill#3-10.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN,	g_smidCelinaRun,				"rsp celina#12-3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidAmyWhatThe,				"rsp amy16.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidAmyLookout,				"rsp amy8.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidAndreaThereHeIs,		"rsp andrea-12.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidAndreaThereHeGoes,		"rsp andrea-13.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidAndreaNeedBackup,		"rsp andrea-2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidAndreaHesPostal,		"rsp andrea-23.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidAndreaHesManiac,		"rsp andrea-24.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidAndreaWheresBackup,	"rsp andrea-4.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN,	g_smidTinaOhMyGod,			"tina naughton#24-1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidScottBleeding,			"scott riedle#7-3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidScottWhereIsHe,			"scott riedle#7-4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidScottStopViolence,		 "scott riedle#7-5.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidScottDontGetAway,		"scott riedle#8-5.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidScottCantShootAll,		"scott riedle#8-6.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidScottBumRush,			"scott riedle#9-1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidHickLookout,				"johnbhickck cop1-17.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidHickThereHeIs,			"johnbhickck cop1-23.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidHickGetHim,				"johnbhickck cop1-25.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidHickWhatTheHell,		"johnbhickck cop1-56.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidPaulHelpCall,			"rsp paul#4-1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidSteveHeWentThat,		"rsp steve#7-10.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidSteveLookout,			"rsp steve#7-11.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidSteveThereHeIs,			"rsp steve#7-14.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidSteveGetHim,				"rsp steve#7-15.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidSteveScumbag,			"rsp steve#7-24.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidPaulRComingWay,			"17Paul r.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRubinVominos,			"11Rubin 1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRubinMatalo,				"4Rubin 1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRubinIfenVigado,		"6Rubin 1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRubinGamalo,				"7Rubin 1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRubinThatEnough,		"8Rubin 1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidDebbieDropWeapons,		"DebbieDropWeapons.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidBillConstTough,			"BillConstPrettyTough.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidBillConstTakeCare,		"BillConstTakeCare.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidBillGoTime,				"BillGoTime.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidBillWhupAss,				"BillWhupAss.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVernDontYouMove,		"VernDontYouMove.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVernGoKneesStop,		"VernGoKneesStop.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVernHeyGetOverHere,	"VernHeyGetOverHere.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVernWhereYouGoing,		"VernWhereYouGoing.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidFinalScene,				"music/finalScene.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidFinalSceneCredits,		"music/finalSceneCredits.wav");

// Add on sounds
// Vet
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVetSpareDime,			"bernie#01-1 04.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVetShineShotgun,		"bernie#01-1 13.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVetCollectBrassShells,"bernie#01-1 18.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVetDontShootImVet,		"bernie#01-1 21.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVetAhah,				"brandon#01- 16.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVetShineShotgun2,	"brandon#01- 25.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVetUhOh,				"brandon#01- 38.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVetWorkForFood,		"brandon#12- 03.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVetDontShootVet,	"brandon#12- 06.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVetFragThatBastard,"brandon#12- 08.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVetSpareADime,		"tony#01- 14.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVetWorkForFood2,	"tony#01- 19.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVetShingGun,			"tony#01- 21.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVetDontShootVet2,	"tony#01- 30.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVetHereComesCharlie,"tony#01- 36.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVetFragBastard,		"tony#01- 43.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVetSpareDimeYell,	"jerod- 04.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVetWaaoh,				"jerod- 06.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVetWaa,				"jerod- 08.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVetShine,				"jerod- 23.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVetCharlie,			"jerod- 41.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVetFrag,				"jerod- 51.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVetThisGuysNuts,	"jerod#10- 100.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVetKillThatPsyco,	"jerod#11- 26.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVetCollectShells,	"ruben#10- 09.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVetDontShootVet3,	"ruben#10- 12.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVetWhatThatAK47,	"ruben#11- 37.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVetAskHimForHelp,	"ruben#11- 39.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVetSpanish,			"ruben#11- 47.wav");

// Lawyer
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidLawyerYoullNeedLawyer,"bernie#03- 15.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidLawyerSeeAssInCourt,	"bernie#04- 06.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidLawyerGonnaSue,		"brandon#02- 02.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidLawyerSeeAssInCourt2,"brandon#02- 08.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidLawyerGonnaNeedLawyer,"brandon#02- 14.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidLawyerThatsIllegal,	"brandon#02- 21.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidLawyerThatsIllegal2,	"tony#02- 05.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidLawyerSeeAssInCourt3,	"tony#02- 09.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidLawyerNeedLawyerCard,	"tony#02- 13.wav");

// Golfer
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidGolferFore,				"bernie#07- 04.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidGolferHoleInOne,		"bernie#07- 08.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidGolferUglyBogey,		"bernie#07- 11.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidGolferMyBalls,			"bernie#07- 26.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidGolferMyBalls2,			"brandon#03- 16.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidGolferCinderellaStory,"brandon#03- 20.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidGolferFore2,				"brandon#04- 03.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidGolferHoleInOne2,		"brandon#04- 04.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidGolferHoleInOne3,		"tony#03- 08.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidGolferCinderella,		"tony#03- 24.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidGolferUglyBogey2,		"tony#03- 39.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidGolferMyBalls3,			"tony#03- 43.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidGolferFore3,				"jerod#02- 03.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidGolferMyBalls4,			"jerod#02- 21.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidGolferCinderella2,		"jerod#02- 25.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidGolferThatsHolsInOne,	"ruben#10- 33.wav");

// Nudest Man
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidNudeDontDieNaked,		"brandon#05- 04.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidNudeShotOffMyAhhh,		"brandon#05- 05.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidNudeOhMyGod,				"brandon#05- 07.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidNudeNoOneSeeUs,			"brandon#05- 12.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidNudeDontShootImNude,	"brandon#05- 14.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidNudeHowEmbarrassing,	"brandon#05- 25.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidNudeDontDieNaked2,		"tony#04- 01.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidNudeShotOffMyAhhh2,	"tony#04- 07.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidNudeDontFeelFresh,		"tony#04- 27.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidNudeMyGod,				"jerod#03- 10.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidNudeNoOneSeeUs2,		"jerod#03- 12.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidNudeHowEmbarrassing2,	"jerod#03- 28.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidNudeHeShotMy,			"ruben#05- 10.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidNudeDontDieNaked3,		"ruben#10- 50.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidNudeDontShootNude,		"ruben#10- 56.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidNudeDontFeelFresh2,	"ruben#10- 61.wav");

// Nudest Woman
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidWNudeNoOneSeeUs,		"lia#05- 07.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidWNudeDontShootNude,	"lia#05- 14.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidWNudeHowEmbarrassing,	"lia#05- 22.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidWNudeDontDieNaked,		"stephanie#04- 01.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidWNudeHeShotOffMyAh,	"stephanie#04- 05.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidWNudeDontShootNude2,	"stephanie#04- 10.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidWNudeBumLooker,			"stephanie#04- 19.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidWNudeHowEmbarrassing2,"stephanie#04- 21.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidWNudeDontFeelFresh,	"stephanie#04- 26.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidSouthernNeedLawyer,	"stephanie#09- 12.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidSouthernDieNaked,		"stephanie#09- 19.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidSouthernBumLooker,		"stephanie#09- 23.wav");

// Old Man
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOldManSoiledSelf,		"bernie#09- 05.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOldManDizkneeland,		"bernie#09- 23.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOldManColostomyBag,	"bernie#09- 39.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOldManOwSpleen,			"bernie#09- 47.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOldManSoiledSelf2,		"brandon#06- 01.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOldManWhatsThatSunny,	"brandon#06- 04.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOldManDontShoot,		"brandon#06- 11.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOldManDizkneeland2,	"brandon#06- 13.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOldManNoRespect,		"brandon#07- 02.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOldManColostomyBag2,	"brandon#07- 05.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOldManMySpleen,			"brandon#07- 08.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOldManGroan,				"tony#05- 03.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOldManSoiledSelf3,		"tony#05- 05.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOldManColostomyBag3,	"tony#05- 42.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOldManInGreatWar,		"tony#05- 47.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOldManDontShootRetired,"tony#06- 07.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOldManKidsNoRespect,	"tony#06- 10.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOldManInGreatWar2,		"tony#06- 14.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOldManRetired,			"jerod#04- 15.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOldManKidsNoRespect2,	"jerod#05- 05.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOldManSpleen,			"jerod#05- 18.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOldManSoiledMyself,	"ruben#06- 06.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOldManDontShootRetired2,"ruben#06- 11.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOldManKidsNoRespect3,	"ruben#06- 23.wav");

// Old Woman
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidOldWomanSoiledSelf,	"lia#06- 01.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidOldWomanCantHear,		"lia#06- 10.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidOldWomanDizkneeland,	"lia#06- 20.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidOldWomanOh,				"lia#06- 32.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidOldWomanColostomyBag,	"lia#06- 40.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidOldWomanSoiledSelf2,	"stephanie#05- 01.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidOldWomanEhWhatsThat,	"stephanie#05- 13.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidOldWomanDizkneeland2,	"stephanie#05- 19.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidOldWomanColostomy,		"stephanie#05- 25.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidOldWomanMySpleen,		"stephanie#05- 32.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidOldWomanFindColostomy,"stephanie#05- 39.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidOldWomanNoRespect,		"stephanie#11- 35.wav");

// Wal Mart Employee
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidWalMartCleanupAisle6,	"bernie#10- 02.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidWalMartNoWaiting4,		"bernie#10- 13.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidWalMartDontShoot,		"bernie#10- 38.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidWalMartCleanupAisle4,	"brandon#08- 07.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidWalMartCleanupAisle3,	"brandon#08- 08.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidWalMartCleanupAisle2,	"brandon#08- 09.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidWalMartNoWaitingOn4,	"brandon#08- 15.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidWalMartDontShoot2,		"brandon#08- 40.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidWalMartShellsSpecial,	"brandon#12- 38.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidWalMartHelpYa,			"brandon#12- 40.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidWalMartCleanupAisle17,"tony#07- 11.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidWalMartShellsSpecial2,"tony#07- 17.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidWalMartNoWaitLine4,	"tony#07- 18.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidWalMartMyIHelpYou,		"tony#07- 21.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidWalMartGetMyManager,	"tony#07- 38.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidWalMartDontShoot3,		"tony#07- 42.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidWalMartCleanupAisle7,	"jerod#06- 03.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidWalMartCleanupAisle6b,"ruben#07- 01.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidWalMartOnlyPartTime,	"ruben#07- 30.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidWalMartGetManager2,	"ruben#10- 94.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidWalMartOnlyPartTimeComeon,"ruben#10- 97.wav");

// Woman Wal Mart Employee
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidWWalMartCleanupAisle8,"lia#07- 02.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidWWalMartNoWaiting3,	"lia#07- 16.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidWWalMartFindingOK,		"lia#07- 23.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidWWalMartGetManager,	"lia#07- 33.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidWWalMartCleanupAisle6,"stephanie#06- 02.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidWWalMartShellsSpecial,"stephanie#06- 04.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidWWalMartMayIHelpYou,	"stephanie#06- 12.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidWWalMartGetManager2,	"stephanie#06- 18.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidWWalMartDontShoot,		"stephanie#06- 21.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidWWalMartFindOK,			"stephanie#09- 35.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidWWalMartNeedVacation,	"stephanie#11- 17.wav");

// Red Cross Man
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRedCrossMedic,			"bernie#11- 15.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRedCrossWhereCops,		"bernie#11- 32.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRedCrossOtherHalfGuy,	"bernie#11- 42.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRedCrossOverpopulation,"bernie#11- 51.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRedCrossWasntTrained,	"bernie#11- 68.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRedCrossVolunteered,	"bernie#11- 73.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRedCrossNeutralDontShoot,"brandon#09- 02.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRedCrossCallForBackup,"brandon#09- 06.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRedCrossGuysNuts,		"brandon#09- 07.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRedCrossLowOnBlood,	"brandon#09- 20.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRedCrossTriggerHappy,	"brandon#09- 22.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRedCrossWhereCops2,	"brandon#09- 23.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRedCrossEnoughFood,	"brandon#10- 19.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRedCrossTryingToHelp,	"brandon#10- 31.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRedCrossSquashedShot,	"brandon#12- 50.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRedCrossAh,				"brandon#12- 62.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRedCrossNeutral,		"tony#08- 03.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRedCrossCallBackupHelp,"tony#08- 05.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRedCrossMedic2,			"tony#08- 13.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRedCrossLowBlood,		"tony#08- 16.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRedCrossStretcher,		"tony#08- 18.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRedCrossSquachedShot2,"tony#08- 31.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRedCrossOtherHalf,		"tony#08- 37.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRedCrossDoubleBodyBag,"tony#08- 38.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRedCrossJoinedMarines,"tony#08- 52.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRedCrossNotTrained,	"tony#08- 56.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRedCrossNeutralDont,	"jerod#07- 04.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRedCrossMedic3,			"jerod#07- 15.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRedCrossOtherHalfGuy3,"jerod#10=120.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRedCrossNeutral2,		"ruben#08- 02.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRedCrossCallBackup2,	"ruben#08- 08.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRedCrossWhereCops3,	"ruben#08- 34.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRedCrossVolunteered2,	"ruben#08- 68.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRedCrossSquashedShot3,"ruben#10- 115.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRedCrossOtherHalfGuy2,"ruben#10- 120.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRedCrossJoinedMarines2,"ruben#10- 127.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRedCrossVolunteered4,	"ruben#10- 131.wav");

// Red Cross Woman
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidWRedCrossCallForHelp,	"lia#08- 05.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidWRedCrossNeedBlood,	"lia#08- 28.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidWRedCrossOtherHalf,	"lia#08- 53.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidWRedCrossBodyBagOrder,"lia#08- 58.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidWRedCrossMarines,		"lia#08- 73.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidWRedCrossNotTrained,	"lia#08- 77.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidWRedCrossVolunteered,	"lia#08- 80.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidWRedCrossNeutralDont,	"stephanie#07- 03.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidWRedCrossGuysNuts,		"stephanie#07- 11.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidWRedCrossMedic,			"stephanie#07- 14.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidWRedCrossLowBlood,		"stephanie#07- 18.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidWRedCrossSquashedShot,"stephanie#07- 32.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidWRedCrossNotTrainedCry,"stephanie#07- 59.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidWRedCrossVolunteered2,"stephanie#11- 73.wav");

// Victim Man
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVictimAhh,				"bernie#12- 08.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVictimTheEndIsNear,	"bernie#12- 09.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVictimRepent,			"bernie#12- 11.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVictimAhHesGotAGun,	"bernie#12- 17.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVictimStopShootingAlreadyDead,"bernie#12- 22.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVictimINeedFirstAid,	"bernie#12- 32.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVictimRunForLives,		"bernie#12- 41.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVictimOwMyEye,			"bernie#12- 43.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVictimSwallowedBob,	"bernie#12- 60.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVictimAskForHelp,		"bernie#12- 84.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVictimHelpUnderHere,	"bernie#12- 88.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVictumUnderHere,		"bernie#12- 92.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVictumGetThatTV,		"bernie#12- 97.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVictimTheHorror,		"bernie#12- 99.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVictimVeryBadDay,		"brandon#11- 07.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVictimEndNear,			"brandon#11- 10.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVictimRepent2,			"brandon#11- 16.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVictimHesGotGun,		"brandon#11- 20.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVictimStopShooting,	"brandon#11- 24.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVictimFirstAid,			"brandon#11- 32.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVictimWhatsHappening,	"brandon#11- 36.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVictimRunForLives2,	"brandon#11- 38.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVictimAnyoneSeenEar,	"brandon#11- 49.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVictimBadDays,			"brandon#11- 53.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVictimWhatsThat,		"brandon#11- 66.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVictimCough,				"brandon#11- 74.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVictimWhereTV,			"brandon#12- 83.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVictimAhhk,				"tony#01- 07.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVictimAuh,				"tony#01- 12.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVictimAhu,				"tony#01- 15.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVictimRepent3,			"tony#09- 16.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVictimJudgementDay,	"tony#09- 17.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVictimAlreadyDead,		"tony#09- 26.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVictimFirstAid2,		"tony#10- 03.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVictimRunForLives3,	"tony#10- 10.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVictimMyEye,				"tony#10- 15.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVictimSeenMyEar,		"tony#10- 18.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVictimWhereTV2,			"tony#10- 56.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVictimHorrorGroan,		"tony#10- 58.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVictimWhereTV3,			"jerod#09- 100.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVictimTheHorror2,		"jerod#09- 105.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVictimBastardAlreadyDead,"jerod#09- 33.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVictimRunForLives4,	"jerod#09- 48.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVictimWhatAK47,			"jerod#09- 77.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVictimAAhh,				"jerod#09- 86.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVictimNowShooting,		"jerod#10- 33.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVictimAskHim,			"jerod#11- 41.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVictimEndIsNear,		"ruben#09- 14.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVictimAhHesGotAGun2,	"ruben#09- 29.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVictimStopAlreadyDead,"ruben#09- 36.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVictimAlreadyDead2,	"ruben#09- 39.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVictimOwMyEyes,			"ruben#09- 57.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVictimSeenMyEar3,		"ruben#09- 63.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVictimSpanishWhat,		"ruben#11- 20.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVictimAhSheAi,			"ruben#11- 25.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVictimSwallowedRoberto,"ruben#11- 31.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidVictimWhereTVMan,		"ruben#11- 50.wav");

// Victim Woman
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidWVictimGonnaSue,		"lia#03- 02.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidWVictimTheHorror,		"lia#09- 114.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidWVictimScream,			"lia#09- 13.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidWVictimAhGotGun,		"lia#09- 19.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidWVictimKillThePsyco,	"lia#09- 37.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidWVictimNeedFirstAid,	"lia#09- 43.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidWVictimWhatHappening,	"lia#09- 49.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidWVictimSeenEar,			"lia#09- 65.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidWVictimSomeGuyShooting,"lia#09- 80.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidWVictimNeedAid,			"stephanie#08- 23.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidWVictimWhatHappening2,"stephanie#08- 28.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidWVictimMyEyes,			"stephanie#08- 36.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidWVictimSwallowedBob,	"stephanie#08- 49.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidWVictimOhNo,				"stephanie#08- 50.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidWVictimNowShooting,	"stephanie#08- 56.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidWVictimWhatsThatAK47,	"stephanie#08- 64.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidWVictimAh,				"stephanie#08- 70.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidWVictimHelpHelp,		"stephanie#08- 77.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidWVictimGetOffMe,		"stephanie#08- 78.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidWVictimVeryBadDay,		"stephanie#10- 01.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidWVictimHelpUnderHere,	"stephanie#10- 23.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidWVictimThatsIllegal,	"stephanie#11- 10.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidWVictimAlreadyDead,	"stephanie#11- 80.wav");
DEFINE_SAMPLE_ID(SMDF_FEMALE_PAIN, g_smidWVictimRunForLives,	"stephanie#11- 85.wav");

#if (TARGET == SUPER_POSTAL) || (TARGET == JAPAN_ADDON)
// New Japanese sounds
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidAsami_b1,					"asami-b1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidAsami_b2,					"asami-b2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidAsami_d1,					"asami-d1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidAsami_d2,					"asami-d2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidAsami_h1,					"asami-h1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidAsami_h2,					"asami-h2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidAsami_h3,					"asami-h3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidAsami_h4,					"asami-h4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidAsami_o1,					"asami-o1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidAsami_o2,					"asami-o2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidAsami_r1,					"asami-r1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidAsami_r2,					"asami-r2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidAsami_r3,					"asami-r3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidAsami_r4,					"asami-r4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidAsami_sc1,				"asami-sc1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidAsami_sc2,				"asami-sc2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidAsami_sc3,				"asami-sc3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidAsami_sc4,				"asami-sc4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidAsami_ss1,				"asami-ss1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidAsami_ss2,				"asami-ss2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidAsami_ss3,				"asami-ss3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidAsami_ss4,				"asami-ss4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidAyame_b1,					"ayame-b1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidAyame_b2,					"ayame-b2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidAyame_d1,					"ayame-d1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidAyame_d2,					"ayame-d2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidAyame_h1,					"ayame-h1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidAyame_h2,					"ayame-h2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidAyame_h3,					"ayame-h3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidAyame_h4,					"ayame-h4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidAyame_o1,					"ayame-o1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidAyame_o2,					"ayame-o2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidAyame_r1,					"ayame-r1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidAyame_r2,					"ayame-r2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidAyame_r3,					"ayame-r3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidAyame_r4,					"ayame-r4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidAyame_sc1,				"ayame-sc1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidAyame_sc2,				"ayame-sc2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidAyame_sc3,				"ayame-sc3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidAyame_sc4,				"ayame-sc4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidAyame_ss1,				"ayame-ss1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidAyame_ss2,				"ayame-ss2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidAyame_ss3,				"ayame-ss3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidAyame_ss4,				"ayame-ss4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidCompsales_b1,			"compsales-b1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidCompsales_b2,			"compsales-b2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidCompsales_d1,			"compsales-d1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidCompsales_d2,			"compsales-d2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidCompsales_h1,			"compsales-h1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidCompsales_h2,			"compsales-h2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidCompsales_h3,			"compsales-h3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidCompsales_h4,			"compsales-h4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidCompsales_o1,			"compsales-o1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidCompsales_o2,			"compsales-o2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidCompsales_r1,			"compsales-r1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidCompsales_r2,			"compsales-r2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidCompsales_r3,			"compsales-r3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidCompsales_r4,			"compsales-r4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidCompsales_sc1,			"compsales-sc1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidCompsales_sc2,			"compsales-sc2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidCompsales_sc3,			"compsales-sc3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidCompsales_sc4,			"compsales-sc4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidCompsales_ss1,			"compsales-ss1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidCompsales_ss2,			"compsales-ss2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidCompsales_ss3,			"compsales-ss3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidCompsales_ss4,			"compsales-ss4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidCompshop_b1,				"compshop-b1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidCompshop_b2,				"compshop-b2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidCompshop_d1,				"compshop-d1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidCompshop_d2,				"compshop-d2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidCompshop_h1,				"compshop-h1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidCompshop_h2,				"compshop-h2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidCompshop_h3,				"compshop-h3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidCompshop_h4,				"compshop-h4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidCompshop_o1,				"compshop-o1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidCompshop_o2,				"compshop-o2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidCompshop_r1,				"compshop-r1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidCompshop_r2,				"compshop-r2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidCompshop_r3,				"compshop-r3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidCompshop_r4,				"compshop-r4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidCompshop_sc1,			"compshop-sc1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidCompshop_sc2,			"compshop-sc2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidCompshop_sc3,			"compshop-sc3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidCompshop_sc4,			"compshop-sc4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidCompshop_ss1,			"compshop-ss1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidCompshop_ss2,			"compshop-ss2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidCompshop_ss3,			"compshop-ss3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidCompshop_ss4,			"compshop-ss4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidKazuki_b1,				"kazuki-b1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidKazuki_b2,				"kazuki-b2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidKazuki_d1,				"kazuki-d1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidKazuki_d2,				"kazuki-d2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidKazuki_h1,				"kazuki-h1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidKazuki_h2,				"kazuki-h2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidKazuki_h3,				"kazuki-h3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidKazuki_h4,				"kazuki-h4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidKazuki_o1,				"kazuki-o1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidKazuki_o2,				"kazuki-o2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidKazuki_r1,				"kazuki-r1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidKazuki_r2,				"kazuki-r2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidKazuki_r3,				"kazuki-r3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidKazuki_r4,				"kazuki-r4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidKazuki_sc1,				"kazuki-sc1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidKazuki_sc2,				"kazuki-sc2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidKazuki_sc3,				"kazuki-sc3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidKazuki_sc4,				"kazuki-sc4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidKazuki_ss1,				"kazuki-ss1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidKazuki_ss2,				"kazuki-ss2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidKazuki_ss3,				"kazuki-ss3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidKazuki_ss4,				"kazuki-ss4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidKensaku_b1,				"kensaku-b1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidKensaku_b2,				"kensaku-b2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidKensaku_d1,				"kensaku-d1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidKensaku_d2,				"kensaku-d2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidKensaku_h1,				"kensaku-h1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidKensaku_h2,				"kensaku-h2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidKensaku_h3,				"kensaku-h3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidKensaku_h4,				"kensaku-h4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidKensaku_o1,				"kensaku-o1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidKensaku_o2,				"kensaku-o2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidKensaku_r1,				"kensaku-r1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidKensaku_r2,				"kensaku-r2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidKensaku_r3,				"kensaku-r3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidKensaku_r4,				"kensaku-r4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidKensaku_sc1,				"kensaku-sc1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidKensaku_sc2,				"kensaku-sc2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidKensaku_sc3,				"kensaku-sc3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidKensaku_sc4,				"kensaku-sc4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidKensaku_ss1,				"kensaku-ss1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidKensaku_ss2,				"kensaku-ss2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidKensaku_ss3,				"kensaku-ss3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidKensaku_ss4,				"kensaku-ss4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidNoboru_b1,				"noboru-b1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidNoboru_b2,				"noboru-b2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidNoboru_d1,				"noboru-d1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidNoboru_d2,				"noboru-d2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidNoboru_h1,				"noboru-h1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidNoboru_h2,				"noboru-h2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidNoboru_h3,				"noboru-h3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidNoboru_h4,				"noboru-h4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidNoboru_o1,				"noboru-o1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidNoboru_o2,				"noboru-o2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidNoboru_r1,				"noboru-r1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidNoboru_r2,				"noboru-r2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidNoboru_r3,				"noboru-r3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidNoboru_r4,				"noboru-r4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidNoboru_sc1,				"noboru-sc1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidNoboru_sc2,				"noboru-sc2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidNoboru_sc3,				"noboru-sc3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidNoboru_sc4,				"noboru-sc4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidNoboru_ss1,				"noboru-ss1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidNoboru_ss2,				"noboru-ss2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidNoboru_ss3,				"noboru-ss3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidNoboru_ss4,				"noboru-ss4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOsales_b1,				"osales-b1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOsales_b2,				"osales-b2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOsales_d1,				"osales-d1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOsales_d2,				"osales-d2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOsales_h1,				"osales-h1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOsales_h2,				"osales-h2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOsales_h3,				"osales-h3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOsales_h4,				"osales-h4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOsales_o1,				"osales-o1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOsales_o2,				"osales-o2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOsales_r1,				"osales-r1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOsales_r2,				"osales-r2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOsales_r3,				"osales-r3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOsales_r4,				"osales-r4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOsales_sc1,				"osales-sc1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOsales_sc2,				"osales-sc2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOsales_sc3,				"osales-sc3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOsales_sc4,				"osales-sc4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOsales_ss1,				"osales-ss1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOsales_ss2,				"osales-ss2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOsales_ss3,				"osales-ss3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOsales_ss4,				"osales-ss4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOsamu_b1,					"osamu-b1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOsamu_b2,					"osamu-b2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOsamu_d1,					"osamu-d1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOsamu_d2,					"osamu-d2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOsamu_h1,					"osamu-h1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOsamu_h2,					"osamu-h2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOsamu_h3,					"osamu-h3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOsamu_h4,					"osamu-h4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOsamu_o1,					"osamu-o1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOsamu_o2,					"osamu-o2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOsamu_r1,					"osamu-r1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOsamu_r2,					"osamu-r2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOsamu_r3,					"osamu-r3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOsamu_r4,					"osamu-r4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOsamu_sc1,				"osamu-sc1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOsamu_sc2,				"osamu-sc2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOsamu_sc3,				"osamu-sc3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOsamu_sc4,				"osamu-sc4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOsamu_ss1,				"osamu-ss1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOsamu_ss2,				"osamu-ss2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOsamu_ss3,				"osamu-ss3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidOsamu_ss4,				"osamu-ss4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRyuichi_b1,				"ryuichi-b1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRyuichi_b2,				"ryuichi-b2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRyuichi_d1,				"ryuichi-d1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRyuichi_d2,				"ryuichi-d2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRyuichi_h1,				"ryuichi-h1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRyuichi_h2,				"ryuichi-h2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRyuichi_h3,				"ryuichi-h3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRyuichi_h4,				"ryuichi-h4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRyuichi_o1,				"ryuichi-o1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRyuichi_o2,				"ryuichi-o2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRyuichi_r1,				"ryuichi-r1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRyuichi_r2,				"ryuichi-r2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRyuichi_r3,				"ryuichi-r3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRyuichi_r4,				"ryuichi-r4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRyuichi_sc1,				"ryuichi-sc1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRyuichi_sc2,				"ryuichi-sc2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRyuichi_sc3,				"ryuichi-sc3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRyuichi_sc4,				"ryuichi-sc4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRyuichi_ss1,				"ryuichi-ss1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRyuichi_ss2,				"ryuichi-ss2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRyuichi_ss3,				"ryuichi-ss3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidRyuichi_ss4,				"ryuichi-ss4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidSakura_b1,				"sakura-b1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidSakura_b2,				"sakura-b2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidSakura_d1,				"sakura-d1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidSakura_d2,				"sakura-d2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidSakura_h1,				"sakura-h1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidSakura_h2,				"sakura-h2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidSakura_h3,				"sakura-h3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidSakura_h4,				"sakura-h4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidSakura_o1,				"sakura-o1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidSakura_o2,				"sakura-o2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidSakura_r1,				"sakura-r1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidSakura_r2,				"sakura-r2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidSakura_r3,				"sakura-r3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidSakura_r4,				"sakura-r4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidSakura_sc1,				"sakura-sc1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidSakura_sc2,				"sakura-sc2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidSakura_sc3,				"sakura-sc3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidSakura_sc4,				"sakura-sc4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidSakura_ss1,				"sakura-ss1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidSakura_ss2,				"sakura-ss2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidSakura_ss3,				"sakura-ss3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidSakura_ss4,				"sakura-ss4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidShinobu_b1,				"shinobu-b1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidShinobu_b2,				"shinobu-b2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidShinobu_d1,				"shinobu-d1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidShinobu_d2,				"shinobu-d2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidShinobu_h1,				"shinobu-h1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidShinobu_h2,				"shinobu-h2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidShinobu_h3,				"shinobu-h3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidShinobu_h4,				"shinobu-h4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidShinobu_o1,				"shinobu-o1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidShinobu_o2,				"shinobu-o2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidShinobu_r1,				"shinobu-r1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidShinobu_r2,				"shinobu-r2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidShinobu_r3,				"shinobu-r3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidShinobu_r4,				"shinobu-r4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidShinobu_sc1,				"shinobu-sc1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidShinobu_sc2,				"shinobu-sc2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidShinobu_sc3,				"shinobu-sc3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidShinobu_sc4,				"shinobu-sc4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidShinobu_ss1,				"shinobu-ss1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidShinobu_ss2,				"shinobu-ss2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidShinobu_ss3,				"shinobu-ss3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidShinobu_ss4,				"shinobu-ss4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidTadao_b1,					"tadao-b1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidTadao_b2,					"tadao-b2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidTadao_d1,					"tadao-d1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidTadao_d2,					"tadao-d2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidTadao_h1,					"tadao-h1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidTadao_h2,					"tadao-h2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidTadao_h3,					"tadao-h3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidTadao_h4,					"tadao-h4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidTadao_o1,					"tadao-o1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidTadao_o2,					"tadao-o2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidTadao_r1,					"tadao-r1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidTadao_r2,					"tadao-r2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidTadao_r3,					"tadao-r3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidTadao_r4,					"tadao-r4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidTadao_sc1,				"tadao-sc1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidTadao_sc2,				"tadao-sc2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidTadao_sc3,				"tadao-sc3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidTadao_sc4,				"tadao-sc4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidTadao_ss1,				"tadao-ss1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidTadao_ss2,				"tadao-ss2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidTadao_ss3,				"tadao-ss3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidTadao_ss4,				"tadao-ss4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidTomiko_b1,				"tomiko-b1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidTomiko_b2,				"tomiko-b2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidTomiko_d1,				"tomiko-d1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidTomiko_d2,				"tomiko-d2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidTomiko_h1,				"tomiko-d1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidTomiko_h2,				"tomiko-h2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidTomiko_h3,				"tomiko-h3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidTomiko_h4,				"tomiko-h4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidTomiko_o1,				"tomiko-o1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidTomiko_o2,				"tomiko-o2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidTomiko_r1,				"tomiko-r1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidTomiko_r2,				"tomiko-r2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidTomiko_r3,				"tomiko-r3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidTomiko_r4,				"tomiko-r4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidTomiko_sc1,				"tomiko-sc1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidTomiko_sc2,				"tomiko-sc2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidTomiko_sc3,				"tomiko-sc3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidTomiko_sc4,				"tomiko-sc4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidTomiko_ss1,				"tomiko-ss1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidTomiko_ss2,				"tomiko-ss2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidTomiko_ss3,				"tomiko-ss3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidTomiko_ss4,				"tomiko-ss4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidYutaka_b1,				"yutaka-b1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidYutaka_b2,				"yutaka-b2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidYutaka_d1,				"yutaka-d1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidYutaka_d2,				"yutaka-d2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidYutaka_h1,				"yutaka-h1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidYutaka_h2,				"yutaka-h2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidYutaka_h3,				"yutaka-h3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidYutaka_h4,				"yutaka-h4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidYutaka_o1,				"yutaka-o1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidYutaka_o2,				"yutaka-o2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidYutaka_r1,				"yutaka-r1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidYutaka_r2,				"yutaka-r2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidYutaka_r3,				"yutaka-r3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidYutaka_r4,				"yutaka-r4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidYutaka_sc1,				"yutaka-sc1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidYutaka_sc2,				"yutaka-sc2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidYutaka_sc3,				"yutaka-sc3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidYutaka_sc4,				"yutaka-sc4.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidYutaka_ss1,				"yutaka-ss1.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidYutaka_ss2,				"yutaka-ss2.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidYutaka_ss3,				"yutaka-ss3.wav");
DEFINE_SAMPLE_ID(SMDF_NO_DESCRIPT, g_smidYutaka_ss4,				"yutaka-ss4.wav");
#endif


//////////////////////////////////////////////////////////////////////////////
// Exported (extern) variables.
//////////////////////////////////////////////////////////////////////////////

// The SampleMaster's resmgr.
extern RResMgr	g_resmgrSamples;
extern void PlayWithMyOrgan(); // actually in organ.cpp

//////////////////////////////////////////////////////////////////////////////
// Protos.
//////////////////////////////////////////////////////////////////////////////

// Set the volume for a category of sounds (0-SampleMaster::UserMaxVolume)
// returns SUCCESS if input is valid
short	SetCategoryVolume(
	SampleMaster::SoundCategory eType,
	short sVolume = SampleMaster::UserMaxVolume);

//////////////////////////////////////////////////////////////////////////////
// Get the volume for a category of sounds (0-SampleMaster::UserMaxVolume)
// returns volume, or -1 if category is invalid.
//////////////////////////////////////////////////////////////////////////////
short	GetCategoryVolume(
	SampleMaster::SoundCategory eType = SampleMaster::Unspecified);

//////////////////////////////////////////////////////////////////////////////
// Set the current volume for a sound currently playing. (0-255)
// You need the volume ID returned from PlaySample.
// You will NOT get an error if number doesn't match.
// (It is assumed your sound has merely finished.)
// Returns SUCCESS or FAILURE
//////////////////////////////////////////////////////////////////////////////
short	SetInstanceVolume(
	SampleMaster::SoundInstance si,			// make sure it is YOUR sound
	short sVolume = 255);						// 0 - 255

//////////////////////////////////////////////////////////////////////////////
//	
//		Calculate volume based on 3d distance... [ 1 / (R*R) ]
//		Distance is relative to the current sound position, which
//		is set independently by the App.
//
//		The attenuation radius is the distance at which volume is
//		half the original level.  (Very soft, but still audible)
//
//		Returns 0-255 for volume (255 = epicenter), or -1 on error
//
//////////////////////////////////////////////////////////////////////////////
short	DistanceToVolume(float	fX,	// in Postal 3d coordinates
							  float	fY,
							  float	fZ,
							  float	fR		// Sound half life
							  );

//////////////////////////////////////////////////////////////////////////////
//  Set the current 3d center of the sound being played
//////////////////////////////////////////////////////////////////////////////
void	SetSoundLocation(float fX, float fY, float fZ);

// Cache a sample.  Causes sample to be loaded, if it is not.  This keeps it
// available in an instant until the next PurgeSamples() call.
void CacheSample(			// Returns nothing.
	SampleMasterID	id);	// Identifier of sample you want played.

// Plays a sample with volume adjustment.  This may require load from disk.
void PlaySample(										// Returns nothing.
															// Does not fail.
	SampleMasterID	id,								// In:  Identifier of sample you want played.
	SampleMaster::SoundCategory eType,			// In:  Sound Volume Category for user adjustment
	short	sInitialVolume	= 255,					// In:  Initial Sound Volume (0 - 255)
	SampleMaster::SoundInstance*	psi = NULL,	// Out: Handle for adjusting sound volume
	long* plSampleDuration = NULL,				// Out: Sample duration in ms, if not NULL.
	long lLoopStartTime = -1,						// In:  Where to loop back to in milliseconds.
															//	-1 indicates no looping (unless m_sLoop is
															// explicitly set).
	long lLoopEndTime = 0,							// In:  Where to loop back from in milliseconds.
															// In:  If less than 1, the end + lLoopEndTime is used.
	bool bPurgeSample = false);					// In:  Call ReleaseAndPurge rather than Release after playing

#if 0	// Backwards is for sux.  OOooooh grenader guy.  Just kidding.
///////////////////////////////////////////////////////////////////////////////////////////////////
// BACKWARDS COMPATIBLE STUB PROVIDED FOR YOUR COMPILING CONVENIENCE
// Plays a sample.  This may require load from disk.
void PlaySample(							// Returns nothing.
												// Does not fail.
	SampleMasterID	id,					// In:  Identifier of sample you want played.
	long* plSampleDuration = NULL,	// Out: Sample duration in ms, if not NULL.
	long lLoopStartTime = -1,			// In:  Where to loop back to in milliseconds.
												//	-1 indicates no looping (unless m_sLoop is
												// explicitly set).
	long lLoopEndTime = 0,				// In:  Where to loop back from in milliseconds.
												// In:  If less than 1, the end + lLoopEndTime is used.
	bool bPurgeSample = false);		// In:  Call ReleaseAndPurge rather than Release after playing

// Plays a sample and purges the resource after playing (as long as nobody
// else has used the same sample resource)
void PlaySampleThenPurge(					// Returns nothing.
													// Does not fail.
	SampleMasterID	id,						// In:  Identifier of sample you want played.
												
	SampleMaster::SoundInstance*	psi = NULL,				// Out: Handle for adjusting sound volume
	SampleMaster::SoundCategory	eType = SampleMaster::Unspecified,	// In:  Sound Volume Category for user adjustment
	short	sInitialVolume = 255,			// In:  Initial Sound Volume (0 - 255)

	long* plSampleDuration = NULL,		// Out: Sample duration in ms, if not NULL.
	long lLoopStartTime = -1,				// In:  Where to loop back to in milliseconds.
													//	-1 indicates no looping (unless m_sLoop is
													// explicitly set).
	long lLoopEndTime = 0);					// In:  Where to loop back from in milliseconds.
													// If les than 1, the end + lLoopEndTime is used.
#endif

// Checks if a particular sample is playing.
// Note: An ugly side effect is that this will cause the sample to
// load, if it is not already loaded.
bool IsSamplePlaying(	// Returns true, if the sample is playing, 
								// false otherwise.
	SampleMasterID	id);	// Identifier of sample to be checked.

// Checks if the specified play instance is still going.
bool IsSamplePlaying(	// Returns true, if the sample is playing, 
								// false otherwise.
	SampleMaster::SoundInstance	si);	// In:  Identifies play instance.

// Checks if any sample is playing.
bool IsSamplePlaying(void);	// Returns true, if a sample is playing,
										// false otherwise.

// Aborts the specified play instance if it is still going.
short AbortSample(		// Returns 0 if sample aborted, 1 if not.
	SampleMaster::SoundInstance	si);	// In:  Identifies play instance.

// Purges all samples that are not in use.
void PurgeSamples(void);	// Returns nothing.

// Purge sample.  Releases a particular sample.
// Note that this cannot be implemented with the current RResMgr, so it does
// not work.
void PurgeSample(			// Returns nothing.
	SampleMasterID	id);	// Identifier of sample you want played.

// Aborts all currently playing samples.
void AbortAllSamples(void);	// Returns nothing.

// Pauses all active samples.
extern void PauseAllSamples();

// Resumes all paused samples.
extern void ResumeAllSamples();

// Stops looping the specified play instance.  That is, it will continue from
// its current play point to the end.
void StopLoopingSample(						// Returns nothing.
	SampleMaster::SoundInstance	si);	// In:  Identifies play instance.



///////////////////////////////////////////////////////////////////////////////	
////// Danger! Danger! //// Do not cross unless you have  /////////////////////
/////////////////////////// exhausted the rest of the API /////////////////////
/////////////////////////// for your task!!!!!!!!!!!!!!!! /////////////////////
///////////////////////////////////////////////////////////////////////////////

// If you must, you can access the RSnd.  IF YOU DO, READ THIS:
// This function will always return an RSnd, even if your play instance is
// long gone.
// THERE ARE LIMITATIONS TO WHAT YOU CAN DO WITH THIS RSND:
// 1) NEVER READ any value from the RSnd.  You may only WRITE to the RSnd.
RSnd* GetInstanceChannel(					// Returns ptr to an RSnd.  Yours, if
													// it has not finished with your sample.
													// A generic one, otherwise.
	SampleMaster::SoundInstance	si);	// In:  Identifies play instance.


#endif	// SAMPLEMASTER_H
//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
