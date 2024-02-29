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
// SampleMaster.cpp
// 
// History:
//		01/29/97 JMI	Started.
//
//		02/02/97	JMI	Added functions to determine if samples are playing.
//
//		02/04/97	JMI	Updated to new rspGetResource().
//
//		02/10/97	JMI	rspReleaseResource() now takes a ptr to a ptr.
//
//		02/10/97	JMI	Increased NUM_CHANNELS from 32 to 64.
//
//		02/18/97	JMI	Made ms_resmgr g_resmgrSamples and made it globally
//							accessible.
//
//		03/07/97	JMI	Now PlaySample() returns the RSnd* it played the sample
//							on which can be querried and/or passed to AbortSample().
//
//		03/07/97	JMI	Decreased play buffer size to 4096.
//
//		03/24/97	JMI	Now, PlaySample(), if RSnd::Play() fails, does not
//							complain.  RSnd::Play() will complain anyways.
//
//		04/22/97	JMI	AbortSample() was causing the RSnd ptr passed to the done
//							callback to no longer contain a sample.  This means the
//							resource had to released immediately after the call to
//							the abort.
//
//		04/29/97	JMI	Added g_smidNil, a way of specifying to SampleMaster that
//							it should not bother with this sample.  This is useful
//							for things that require a sample ID.
//
//		05/09/97	JMI	PlaySample() now takes optional looping parameters which
//							are passed directly on to RSnd::Play().
//
//		06/04/97	JMI	Added AbortAllSamples() which aborts all currently 
//							playing samples.
//
//		06/11/97 BRH	Added PlaySampleThenPurge function which is a convenient
//							way to pass a new purge parameter to PlaySample.  It then
//							uses the new resource manager's ReleaseAndPurge function
//							rather than the regular release.  This will allow you to
//							purge single samples that you don't want to stay in the
//							cache.  
//
//		06/12/97	JMI	PlaySample() function was only using 
//							rspReleaseAndPurgeResource() if an error occurred
//							during startup.
//							Added an rspReleaseAndPurgeResource() in the SndDoneCall
//							so that when the sample was actually done playing, it
//							would release and purge.
//
//		06/16/97	JMI	Added a version of IsSamplePlaying() that allows one to
//							specify the sound channel to check.
//							Also, removed ASSERTs on psnd in AbortSample().
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
//		07/17/97	JMI	Changed VolumeCode to SampleMaster::SoundInstance.  Trying 
//							to make it a generic playing sample identifier.
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
//		07/20/97	JMI	DistanceToVolume() now always returns 255, if that 
//							feature is off.
//
//		08/05/97	JMI	Added PauseAllSamples() and ResumeAllSamples().
//
//		08/05/97 JRD	Added CSoundCatalogue and automated the listing
//							of sounds for use with organ.
//
//		08/20/97 BRH	Added new pain and suffering volume categories.  Also
//							changed Music to Soundtrack, and Voice to Comments.
//
//		08/25/97	JMI	Added default volumes for each category in each quality
//							via the ms_asQualityCategoryAdjustors.
//							Also, added macro enums for UserDefaultVolume, 
//							UserMaxVolume, and MaxVolume.
//
//		09/05/97	JMI	Lowered adjustor volumes on 22KHz 16bit b/c the source
//							volumes are so loud that they hose when they mix.
//							Unfortunately, 8 bit could use this as well but it sounds
//							bad when we use the volume scaling so either way it sucks.
//
//		09/17/97	JMI	Now gets sound category names from Localize.
//
//		09/17/97	JMI	Even though I'd swear I compiled and tested this before
//							it was not compiling with the new array assignment I 
//							added.  Perhaps I did the conditional compilation
//							wrong...fixed.
//
//		09/24/97	JMI	Added bFemalePain member to SampleMasterID.  This field
//							is true if the sample is of a female in pain.  If this
//							field is true, SampleMaster.cpp won't play the sample
//							if the LOCALE is that of a country that does not allow
//							such things in games (currently all but the US).
//
//		10/07/97	JMI	Changed bFemalePain to usDescFlags, a bits field of flags
//							describing the sound so we can know which ones to filter
//							out for various languages.
//
//		01/05/98	JMI	Changed default volumes for SQ_22050_8, SQ_11025_8, and
//							SQ_11025_16.
//
//		09/27/99	JMI	Changed to allow "violent" sounds in any locale 
//							satisfying the CompilerOptions macro VIOLENT_LOCALE.
//
//////////////////////////////////////////////////////////////////////////////
//
// See .H for explanation of this module.
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// C Headers -- Must be included before RSPiX.h b/c RSPiX utilizes SHMalloc.
//////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// RSPiX Headers.
///////////////////////////////////////////////////////////////////////////////
#include "RSPiX.h"

///////////////////////////////////////////////////////////////////////////////
// WishPiX Headers.
// If PATHS_IN_INCLUDES macro is defined, we can utilize relative
// paths to a header file.  In this case we generally go off of our
// RSPiX root directory.  System.h MUST be included before this macro
// is evaluated.  System.h is the header that, based on the current
// platform (or more so in this case on the compiler), defines 
// PATHS_IN_INCLUDES.  Blue.h includes system.h so you can include that
// instead.
///////////////////////////////////////////////////////////////////////////////
#ifdef PATHS_IN_INCLUDES
	#include "WishPiX/ResourceManager/resmgr.h"
#else
	#include "resmgr.h"
#endif

///////////////////////////////////////////////////////////////////////////////
// Postal Headers.
///////////////////////////////////////////////////////////////////////////////

// Let .H know what CPP is being compiled.
#define SAMPLEMASTER_CPP

#include "SampleMaster.h"
#include "game.h"
#include "CompileOptions.h"

//////////////////////////////////////////////////////////////////////////////
// Module specific macros.
//////////////////////////////////////////////////////////////////////////////

#define NUM_CHANNELS		64	// Max number of sample channels usable at once.
#define CHANNEL_MASK		63 // Used to store channel number as part of sample UID

#define PLAY_BUF_SIZE	4096	// In bytes.

#define SET(ptr, val)	((ptr) ? *(ptr) = val : 0)

// If not a violent locale . . . 
#if !VIOLENT_LOCALE
	// No sounds indicating females are in pain or including police references can be played.
	#define CAN_PLAY_SAMPLE(id)	( (id.usDescFlags & (SMDF_FEMALE_PAIN | SMDF_POLICE_REF) ) == 0)
#else
	// All sounds can be played.
	#define CAN_PLAY_SAMPLE(id)	(1)
#endif

//////////////////////////////////////////////////////////////////////////////
// Module specific typedefs.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Exported (extern) variables.
//////////////////////////////////////////////////////////////////////////////
RResMgr	g_resmgrSamples;

//////////////////////////////////////////////////////////////////////////////
// Module specific (static) variables / Instantiate class statics.
//////////////////////////////////////////////////////////////////////////////
//------------ put in C file:
short CSoundCatalogue::ms_sCurPos = 0;
short CSoundCatalogue::ms_sRefCount = 0;
SampleMasterID** CSoundCatalogue::ms_ppsmNameList = NULL;

// Sound channels for playing samples.
static RSnd		ms_asndChannels[NUM_CHANNELS];

// Unique Sample IDs used in identifying playing sample/channel combos (without channel bits set)
static SampleMaster::SoundInstance		ms_aSoundInstances[NUM_CHANNELS] = {0,};	

// Stores the Sound Levels (0-255) for each sound category
static short	ms_asCategoryVolumes[SampleMaster::MAX_NUM_SOUND_CATEGORIES] = {255,};

// Stores the Sound Category for each sound channel:
static	SampleMaster::SoundCategory	ms_aeSoundTypes[NUM_CHANNELS] = {SampleMaster::Unspecified,};

// Sound channel for failures.
static RSnd		ms_sndFailure;

// Module local storage for current 3d location for the sound
static float	fSoundX = 0.,fSoundY = 0., fSoundZ = 0.;

//////////////////////////////////////////////////////////////
// These are the names for the corresponding SoundCategory
// used as an index.
char* SampleMaster::ms_apszSoundCategories[SampleMaster::MAX_NUM_SOUND_CATEGORIES]	=	
	{
	g_apszSoundCategories[Unspecified],
	g_apszSoundCategories[BackgroundMusic],
	g_apszSoundCategories[Weapon],
	g_apszSoundCategories[UserFeedBack],
	g_apszSoundCategories[Destruction],
	g_apszSoundCategories[Ambient],
	g_apszSoundCategories[Demon],
	g_apszSoundCategories[Voices],
	g_apszSoundCategories[Pain],
	g_apszSoundCategories[Suffering],
	};

//////////////////////////////////////////////////////////////
// These are the default volumes for each category in each
// quality.
short SampleMaster::ms_asQualityCategoryAdjustors[NumSoundQualities][MAX_NUM_SOUND_CATEGORIES]	=
	{
	// SQ_11025_8:
		{
		5,		// General.		
		5,		// Music - Soundtrack.			
		5,		// Weapon.			
		5,		// FeedBack.		
		5,		// Destruction.	
		5,		// Ambient.		
		5,		// Demon.			
		5,		// Voices - Comments.
		5,		// Pain - Shot, Burning, blownup
		5,		// Suffering - writhing sounds			
		},

	// SQ_11025_16:
		{
		9,		// General.		
		9,		// Music - Soundtrack.			
		9,		// Weapon.			
		9,		// FeedBack.		
		9,		// Destruction.	
		9,		// Ambient.		
		9,		// Demon.			
		9,		// Voices - Comments.
		9,		// Pain - Shot, Burning, blownup
		9,		// Suffering - writhing sounds			
		},

	// SQ_22050_8:
		{
		3,	// General.		
		3,	// Music - Soundtrack.			
		3,	// Weapon.			
		3,	// FeedBack.		
		3,	// Destruction.	
		3,	// Ambient.		
		3,	// Demon.			
		3,	// Voices - Comments.
		3,	// Pain - Shot, Burning, blownup
		3,	// Suffering - writhing sounds			
		},

	// SQ_22050_16:
		{
		7,	// General.		
		7,	// Music - Soundtrack.			
		7,	// Weapon.			
		7,	// FeedBack.		
		7,	// Destruction.	
		7,	// Ambient.		
		7,	// Demon.			
		7,	// Voices - Comments.
		7,	// Pain - Shot, Burning, blownup
		7,	// Suffering - writhing sounds			
		},

	// SQ_44100_8:
		{
		10,	// General.		
		10,	// Music - Soundtrack.			
		10,	// Weapon.			
		10,	// FeedBack.		
		10,	// Destruction.	
		10,	// Ambient.		
		10,	// Demon.			
		10,	// Voices - Comments.
		10,	// Pain - Shot, Burning, blownup
		10,	// Suffering - writhing sounds			
		},

	// SQ_44100_16:
		{
		10,	// General.		
		10,	// Music - Soundtrack.			
		10,	// Weapon.			
		10,	// FeedBack.		
		10,	// Destruction.	
		10,	// Ambient.		
		10,	// Demon.			
		10,	// Voices - Comments.
		10,	// Pain - Shot, Burning, blownup
		10,	// Suffering - writhing sounds			
		},

	};

//////////////////////////////////////////////////////////////////////////////
// Global variables used to pass sound IDs in an object oriented environment.
//////////////////////////////////////////////////////////////////////////////
SampleMaster::SoundInstance g_siEndingMusak = 0;

//////////////////////////////////////////////////////////////////////////////
// Module specific (static) protos.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Intern Functions.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Callback when done playing/streaming.
//
//////////////////////////////////////////////////////////////////////////////
void SndDoneCall(		// Returns nothing.
	RSnd*	psnd);		// This RSnd.

void SndDoneCall(		// Returns nothing.
	RSnd*	psnd)			// This RSnd.
	{
	ASSERT(psnd != NULL);
	RSample*	psample	= psnd->GetSample();
	if (psample != NULL)
		{
		// Reduce ResMgr ref count.

		// Either release the sample, or purge and release//////////////////////////////////////////////////////////////////////

		if (psnd->m_ulUser)
			rspReleaseAndPurgeResource(&g_resmgrSamples, &psample);
		else
			rspReleaseResource(&g_resmgrSamples, &psample);
		}
	}

//////////////////////////////////////////////////////////////////////////////
// Extern Functions.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Set the volume for a category of sounds (0-UserMaxVolume)
// This goes through the RSnd array and recalibrates the samples as well
// returns SUCCESS if input is valid
//////////////////////////////////////////////////////////////////////////////
short	SetCategoryVolume(
	SampleMaster::SoundCategory eType,
	short sVolume /* = SampleMaster::UserMaxVolume*/)
	{
	if ((eType < SampleMaster::Unspecified) || (eType >= SampleMaster::MAX_NUM_SOUND_CATEGORIES))
		{
		return FAILURE;
		}

	if ((sVolume < 0) || (sVolume > SampleMaster::UserMaxVolume))
		{
		return FAILURE;
		}

	// Set the new volume for that sound type adjusted through
	// the quality's category volume adjustor.
	// This means we're scaling it TWICE and that is why there are two ratios.
	// Ratio1:				AdjustorVolume to MaxUserVolume			Ratio2: UserVolume to SampleMasterVolume.
	//
	//						ms_asQualityCategoryAdjustors[qual][cat]					SampleMaster::VolumeMax  
	//	sVolume	*	============================================= *	=============================================
	//									(UserMaxVolume)											(UserMaxVolume)              
	//
	// For the sake of using integer math, we don't do these operations in the above order.
	//
	ms_asCategoryVolumes[eType] = sVolume * SampleMaster::ms_asQualityCategoryAdjustors[g_GameSettings.m_eCurSoundQuality][eType] * SampleMaster::MaxVolume / (SampleMaster::UserMaxVolume * SampleMaster::UserMaxVolume);

	// Notify all playing sounds of that type that their volume has changed
	for (short i = 0; i < NUM_CHANNELS; i++)
		{
		if (ms_aeSoundTypes[i] == eType)
			{
			ms_asndChannels[i].m_sTypeVolume = ms_asCategoryVolumes[eType];	// adjust volume
			}
		}

	return SUCCESS;
	}

//////////////////////////////////////////////////////////////////////////////
// Get the volume for a category of sounds (0-UserMaxVolume)
// returns category volume, ( 0 - MaxUserVolume ) or -1 if category is invalid.
//////////////////////////////////////////////////////////////////////////////
short	GetCategoryVolume(
	SampleMaster::SoundCategory eType /*  = SampleMaster::SoundCategory::Unspecified */)
	{
	if ((eType < SampleMaster::Unspecified) || (eType >= SampleMaster::MAX_NUM_SOUND_CATEGORIES))
		{
		return -1;
		}

	// Get the new volume for that sound type dejusted through
	// the quality's category volume adjustor.
	// This means we're scaling it TWICE and that is why there are two ratios.
	// Ratio1:				AdjustorVolume to MaxUserVolume			Ratio2: UserVolume to SampleMasterVolume.
	//
	//									(UserVolumeRange)											(UserVolumeRange)              
	//	sVolume	*	============================================= *	=============================================
	//						ms_asQualityCategoryAdjustors[qual][cat]					SampleMaster::VolumeRange  
	//
	// For the sake of using integer math, we don't do these operations in the above order.
	//
	return ms_asCategoryVolumes[eType] * SampleMaster::UserMaxVolume * SampleMaster::UserMaxVolume / (SampleMaster::ms_asQualityCategoryAdjustors[g_GameSettings.m_eCurSoundQuality][eType] * SampleMaster::MaxVolume);
	}

//////////////////////////////////////////////////////////////////////////////
// Set the current volume for a sound currently playing. (0-255)
// You need the volume ID returned from PlaySample.
// You will NOT get an error if number doesn't match, but the colume won't
// change.
// (It is assumed your sound has merely finished playing.)
//
// Returns SUCCESS or FAILURE
//////////////////////////////////////////////////////////////////////////////
short	SetInstanceVolume(
	SampleMaster::SoundInstance si,				// make sure it is YOUR sound
	short sVolume /* = 255 */)	// 0 - 255
	{
	if ( (si < 0) || (sVolume < 0) || (sVolume > 255) )
		{
		return FAILURE;
		}

	// Get the channel number from the lowest bits:
	short sChannel = si & CHANNEL_MASK;
	if (sChannel >= NUM_CHANNELS)
		{
		return FAILURE;	// MASK error!
		}

	// Make sure the sound is still playing (this is NOT an error)
	// Compare current channel ID with high bits of vid:
	if (ms_aSoundInstances[sChannel] == (si & (~CHANNEL_MASK) ) )
		{
		// Security approved!  You may set the sound volume!
		ms_asndChannels[sChannel].m_sChannelVolume = sVolume;
		}

	return SUCCESS;
	}

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
//		Returns 255 if this feature is off.
//
//////////////////////////////////////////////////////////////////////////////
short	DistanceToVolume(float	fX,	// in Postal 3d coordinates
							  float	fY,
							  float	fZ,
							  float	fR		// Sound half life
							  )
	{
	ASSERT(fR >= 1.0);

	if (g_GameSettings.m_sVolumeDistance != FALSE)
		{
		// -ln 2 = ln (1/2) = 1/2 level (ln 10 = 1/10 volume, etc.)
		const float	 fln2 =  float(0.6931471805599);	

		float fDist2 = ABS2(fX - fSoundX,
								fY - fSoundY,
								fZ - fSoundZ	);

		if (fDist2 < 1.0) return 255;	// Dead epicenter

		return short(255.0 * exp( -fln2 * fDist2 / (fR * fR) ) );
		}
	else
		{
		return 255;
		}
	}

//////////////////////////////////////////////////////////////////////////////
//  Set the current 3d center of the sound being played
//////////////////////////////////////////////////////////////////////////////
void	SetSoundLocation(float fX, float fY, float fZ)
	{
	fSoundX = fX;	// global static
	fSoundY = fY;
	fSoundZ = fZ;
	}



//////////////////////////////////////////////////////////////////////////////
//
// Cache a sample.  Causes sample to be loaded, if it is not.  This keeps it
// available in an instant until the next PurgeSamples() call.
//
//////////////////////////////////////////////////////////////////////////////
void CacheSample(			// Returns nothing.
	SampleMasterID	id)	// Identifier of sample you want played.
	{
	if (id.pszId != NULL && CAN_PLAY_SAMPLE(id) )
		{
		RSample*	psample;
		// Get it into memory. 
		// If successful . . .
		if (rspGetResource(
			&g_resmgrSamples,
			id.pszId, 
			&psample) == 0)
			{
			// Release it, so the next purge will remove this sample from RAM.
			// Sort of a flush.
			rspReleaseResource(&g_resmgrSamples, &psample);
			}
		else
			{
			TRACE("CacheSample(): Could not cache sample \"%s\".\n", id.pszId);
			}
		}
	}

//////////////////////////////////////////////////////////////////////////////
//
// Plays a sample.  This may require load from disk.
// Returns a volume handle if you wish to dynamically adjust the volume
//
//////////////////////////////////////////////////////////////////////////////
void PlaySample(												// Returns nothing.
																	// Does not fail.
	SampleMasterID	id,										// In:  Identifier of sample you want played.
	SampleMaster::SoundCategory eType,					// In:  Sound Volume Category for user adjustment
	short	sInitialVolume /* = 255 */,					// In:  Initial Sound Volume (0 - 255)
	SampleMaster::SoundInstance*	psi/* = NULL */,	// Out: Handle for adjusting sound volume
	long* plSampleDuration /* = NULL */,				// Out: Sample duration in ms, if not NULL.
	long lLoopStartTime /* = -1 */,						// In:  Where to loop back to in milliseconds.
																	//	-1 indicates no looping (unless m_sLoop is
																	// explicitly set).
	long lLoopEndTime /* = 0 */,							// In:  Where to loop back from in milliseconds.
																	// In:  If less than 1, the end + lLoopEndTime is used.
	bool bPurgeSample /* = false */)						// In:  Call ReleaseAndPurge rather than Release after playing
	{
	short	sError	= 0;					// Assume no error.
	RSnd*		psnd	= &ms_sndFailure;	// Default to failure case.
	if (psi) *psi = 0;					// Default to failure case.

	if (id.pszId != NULL && CAN_PLAY_SAMPLE(id) )
		{
		RSample*	psample;
		// Get the sample . . .
		if (rspGetResource(
			&g_resmgrSamples,
			id.pszId, 
			&psample) == 0)
			{
			// Get the duration right away.  We want to return this even if we fail
			// to play the sample.
			SET(plSampleDuration, psample->GetDuration() );

			// Brute force search to find open channel.
			short	i;
			for (i = 0; i < NUM_CHANNELS; i++)
				{
				if (ms_asndChannels[i].GetState() == RSnd::Stopped)
					{
					break;
					}
				}

			// If we got one . . .
			if (i < NUM_CHANNELS)
				{
				// Release sample when done via this callback.
				ms_asndChannels[i].m_dcUser	= SndDoneCall;
				ms_asndChannels[i].m_sLoop		= FALSE;		// Safety.
				ms_asndChannels[i].m_ulUser	= bPurgeSample;
				// Set volume in RSound assuming success:
				ms_aeSoundTypes[i]	= eType;
				ms_asndChannels[i].m_sTypeVolume		= ms_asCategoryVolumes[eType];
				ms_asndChannels[i].m_sChannelVolume = sInitialVolume;
				// Atttempt to play sample . . .
				if (ms_asndChannels[i].Play(psample, PLAY_BUF_SIZE, ms_asndChannels[i].m_sChannelVolume,
					ms_asndChannels[i].m_sTypeVolume, lLoopStartTime, lLoopEndTime) == 0)
					{
					// Success.  Give user access to this channel.
					psnd	= &(ms_asndChannels[i]);
					// Set return ID so user can tweak the volume:
					ms_aSoundInstances[i] += NUM_CHANNELS;	// It is a new sound now!
					// Reserve the lower mask bits for the channel number
					if (psi) *psi = ms_aSoundInstances[i] + i;
					}
				else
					{
	//				TRACE("PlaySample(): RSnd::Play() failed for sample.\n");
					sError	= 3;
					}
				}
			else
				{
				TRACE("PlaySample(): No available sound channels.  Increase NUM_CHANNELS"
					" or like it.\n");
				sError	= 2;
				}

			// If an error occurred . . .
			if (sError != 0)
				{
				// Either release the sample, or purge and release//////////////////////////////////////////////////////////////////////

				if (bPurgeSample)
					rspReleaseAndPurgeResource(&g_resmgrSamples, &psample);
				else
					rspReleaseResource(&g_resmgrSamples, &psample);
				}
			}
		else
			{
			TRACE("PlaySample(): Could not get sample \"%s\".\n", id.pszId);
			sError	= 1;
			}
		}
	}

#if 0
///////////////////////////////////////////////////////////////////////////////////////////////////
// BACKWARDS COMPATIBLE STUB PROVIDED FOR YOUR COMPILING CONVENIENCE
// Plays a sample.  This may require load from disk.
///////////////////////////////////////////////////////////////////////////////////////////////////

void PlaySample(								// Returns nothing.
													// Does not fail.
	SampleMasterID	id,						// In:  Identifier of sample you want played.

	long* plSampleDuration /* = NULL*/,	// Out: Sample duration in ms, if not NULL.
	long lLoopStartTime /* = -1 */,		// In:  Where to loop back to in milliseconds.
													//	-1 indicates no looping (unless m_sLoop is
													// explicitly set).
	long lLoopEndTime /* = 0 */,			// In:  Where to loop back from in milliseconds.
													// In:  If less than 1, the end + lLoopEndTime is used.
	bool bPurgeSample /* = false */)		// In:  Call ReleaseAndPurge rather than Release after playing
	{
	PlaySampleEx(id,NULL,SampleMaster::Unspecified,255,plSampleDuration,lLoopStartTime,lLoopEndTime,bPurgeSample);
	}


//////////////////////////////////////////////////////////////////////////////
// Handier interface for purging a sample on release.
//////////////////////////////////////////////////////////////////////////////
void PlaySampleThenPurge(					// Returns nothing.
													// Does not fail.
	SampleMasterID	id,						// In:  Identifier of sample you want played.

	SampleMaster::SoundInstance*	psi /* = NULL */,		// Out: Handle for adjusting sound volume
	SampleMaster::SoundCategory	eType /* = 
		SampleMaster::SoundCategory::Unspecified */,	// In:  Sound Volume Category for user adjustment
	short	sInitialVolume /* = 255 */,	// In:  Initial Sound Volume (0 - 255)

	long* plSampleDuration /* = NULL */,// Out: Sample duration in ms, if not NULL.
	long lLoopStartTime /* = -1 */,		// In:  Where to loop back to in milliseconds.
													//	-1 indicates no looping (unless m_sLoop is
													// explicitly set).
	long lLoopEndTime /* = 0 */)			// In:  Where to loop back from in milliseconds.
													// In:  If less than 1, the end + lLoopEndTime is used.
	{
	PlaySampleEx(id, psi, eType, sInitialVolume,
		plSampleDuration, lLoopStartTime, lLoopEndTime, true);
	}
#endif

//////////////////////////////////////////////////////////////////////////////
//
// Checks if a particular sample is playing.
// Note: An ugly side effect is that this will cause the sample to
// load, if it is not already loaded.
//
//////////////////////////////////////////////////////////////////////////////
bool IsSamplePlaying(	// Returns true, if the sample is playing, 
								// false otherwise.
	SampleMasterID	id)	// Identifier of sample to be checked.
	{
	bool	bRes	= false;	// Assume not playing.

	if (id.pszId != NULL && CAN_PLAY_SAMPLE(id) )
		{
		RSample*	psample;
		// Get it into memory. 
		// If successful . . .
		if (rspGetResource(
			&g_resmgrSamples,
			id.pszId, 
			&psample) == 0)
			{
			// Check if it is locked.
			if (psample->IsLocked() != FALSE)
				{
				bRes	= true;
				}

			// Release it, so the next purge will remove this sample from RAM.
			// Sort of a flush.
			rspReleaseResource(&g_resmgrSamples, &psample);
			}
		else
			{
			TRACE("IsSamplePlaying(): Could not cache sample \"%s\".\n", id.pszId);
			}
		}

	return bRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Checks if the specified play instance is still going.
//
//////////////////////////////////////////////////////////////////////////////
bool IsSamplePlaying(	// Returns true, if the sample is playing, 
								// false otherwise.
	SampleMaster::SoundInstance	si)	// In:  Identifies play instance.
	{
	bool	bPlaying	= false;	// Assume not playing.

	// Get the channel number from the lowest bits:
	short sChannel = si & CHANNEL_MASK;
	if (sChannel < NUM_CHANNELS)
		{
		// Make sure the sound is still playing (this is NOT an error)
		// Compare current channel ID with high bits of sc:
		if (ms_aSoundInstances[sChannel] == (si & (~CHANNEL_MASK) ) )
			{
			// Security approved!  You may check the RSnd!
			if (ms_asndChannels[sChannel].GetState() != RSnd::Stopped)
				{
				// Yes, it is.
				bPlaying	= true;
				}
			}
		}
	else
		{
		// MASK error!
		}

	return bPlaying;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Checks if any sample is playing.
//
//////////////////////////////////////////////////////////////////////////////
bool IsSamplePlaying(void)		// Returns true, if a sample is playing,
										// false otherwise.
	{
	bool	bRes	= false;	// Assume none playing.

	// Check all channels.
	// Brute force search to find open channel.
	short	i;
	for (i = 0; i < NUM_CHANNELS; i++)
		{
		if (ms_asndChannels[i].GetState() != RSnd::Stopped)
			{
			bRes	= true;
			break;
			}
		}

	return bRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Aborts the specified play instance if it is still going.
//
//////////////////////////////////////////////////////////////////////////////
short AbortSample(		// Returns 0 if sample aborted, 1 if not.
	SampleMaster::SoundInstance	si)	// In:  Identifies play instance.
	{
	short	sRes	= 1;	// Assume failure.

	// Get the channel number from the lowest bits:
	short sChannel = si & CHANNEL_MASK;
	if (sChannel < NUM_CHANNELS)
		{
		// Make sure the sound is still playing (this is NOT an error)
		// Compare current channel ID with high bits of sc:
		if (ms_aSoundInstances[sChannel] == (si & (~CHANNEL_MASK) ) )
			{
			// Okay.  Abort.
			if (ms_asndChannels[sChannel].GetState() != RSnd::Stopped)
				{
				if (ms_asndChannels[sChannel].Abort() == 0)
					{
					// Success.
					sRes	= 0;
					// Do we have to release it?  Should get a callback.
					}
				else
					{
					TRACE("AbortSample(): ms_asndChannels[sChannel].Abort() failed.\n");
					}
				}
			}
		}
	else
		{
		// MASK error!
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Purges all samples that are not in use.
//
//////////////////////////////////////////////////////////////////////////////
void PurgeSamples(void)	// Returns nothing.
	{
	g_resmgrSamples.Purge();
	}

//////////////////////////////////////////////////////////////////////////////
//
// Purge sample.  Releases a particular sample if not in use.
// If implemented with the current state of the RResMgr I would have to do
// an rspGetResource() to get the resource pointer to pass to 
// rspReleaseAndPurgeResource() which might defeat the purpose in some cases
// but would be fine for samples you know are loaded.
//
//////////////////////////////////////////////////////////////////////////////
void PurgeSample(			// Returns nothing.
	SampleMasterID	id)	// Identifier of sample you want played.
	{
	TRACE("PurgeSample(): NYI.\n");
	}


///////////////////////////////////////////////////////////////////////////////
// Aborts all currently playing samples.
///////////////////////////////////////////////////////////////////////////////
void AbortAllSamples(void)	// Returns nothing.
	{
	// Check all channels.
	short	i;
	for (i = 0; i < NUM_CHANNELS; i++)
		{
		if (ms_asndChannels[i].GetState() != RSnd::Stopped)
			{
			ms_asndChannels[i].Abort();
			}
		}
	}

///////////////////////////////////////////////////////////////////////////////
// Pauses all active samples.
///////////////////////////////////////////////////////////////////////////////
extern void PauseAllSamples(void)
	{
	// Pause all active channels.
	short	i;
	for (i = 0; i < NUM_CHANNELS; i++)
		{
		if (ms_asndChannels[i].GetState() != RSnd::Stopped)
			{
			ms_asndChannels[i].Pause();
			}
		}
	}

///////////////////////////////////////////////////////////////////////////////
// Resumes all paused samples.
///////////////////////////////////////////////////////////////////////////////
extern void ResumeAllSamples(void)
	{
	// Resume all active channels.
	short	i;
	for (i = 0; i < NUM_CHANNELS; i++)
		{
		if (ms_asndChannels[i].IsPaused() )
			{
			ms_asndChannels[i].Resume();
			}
		}
	}

///////////////////////////////////////////////////////////////////////////////
// Stops looping the specified play instance.  That is, it will continue from
// its current play point to the end.
///////////////////////////////////////////////////////////////////////////////
void StopLoopingSample(						// Returns nothing.
	SampleMaster::SoundInstance	si)	// In:  Identifies play instance.
	{
	RSnd*	psndInstance		= GetInstanceChannel(si);	// Does not fail.
	psndInstance->m_sLoop	= FALSE;
	}

///////////////////////////////////////////////////////////////////////////////
// If you must, you can access the RSnd.  IF YOU DO, READ THIS:
// This function will always return an RSnd, even if your play instance is
// long gone.
// THERE ARE LIMITATIONS TO WHAT YOU CAN DO WITH THIS RSND:
// 1) NEVER READ any value from the RSnd.  You may only WRITE to the RSnd.
///////////////////////////////////////////////////////////////////////////////
RSnd* GetInstanceChannel(					// Returns ptr to an RSnd.  Yours, if
													// it has not finished with your sample.
													// A generic one, otherwise.
	SampleMaster::SoundInstance	si)	// In:  Identifies play instance.
	{
	RSnd*	psndInstance	= &ms_sndFailure;	// Assume long gone.

	// Get the channel number from the lowest bits:
	short sChannel = si & CHANNEL_MASK;
	if (sChannel < NUM_CHANNELS)
		{
		// Make sure the sound is still playing (this is NOT an error)
		// Compare current channel ID with high bits of sc:
		if (ms_aSoundInstances[sChannel] == (si & (~CHANNEL_MASK) ) )
			{
			if (ms_asndChannels[sChannel].GetState() != RSnd::Stopped)
				{
				psndInstance	= &(ms_asndChannels[sChannel]);
				}
			}
		}

	return psndInstance;
	}
	
///////////////////////////////////////////////////////////////////////////////
// EOF
///////////////////////////////////////////////////////////////////////////////
