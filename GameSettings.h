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
// GameSettings.h
// Project: Nostril (aka Postal)
//
// History:
//		12/02/96 MJR	Started.
//
//		03/31/97	JMI	Moved CGameSettings definition from game.h to this file.
//
//		03/31/97	JMI	Now loads game play keys from prefs file.
//
//		04/07/97	JMI	Added m_szServerName and m_usServerPort.
//
//		04/08/97	JMI	Added m_szPlayerName and m_sPlayerColorIndex.
//
//		04/14/97	JMI	Added TEMP flag m_bDontBlit to allow us to speed up the
//							Snap() call by not blitting.
//
//		04/21/97 MJR	Added m_pszRealm for the name of the world .ini file.
//
//		04/22/97	JMI	Added m_sCanRecordDemos indicating something obscure.
//
//		05/15/97	JMI	Added m_sAlphaBlend and m_sXRayEffect.
//
//		05/19/97	JMI	Added m_sDisplayInfo.
//
//		05/22/97	JMI	Added m_s3dFog.
//
//		05/22/97	JMI	Added m_sParticleEffects.
//
//		06/03/97	JMI	Added m_lInitialDemoTimeOut and m_lPersistentDemoTimeOut.
//
//		06/09/97	JMI	Added m_sCanTakeSnapShots.
//
//		06/09/97	JMI	Added m_sCrossHair.
//
//		06/11/97	JMI	Added m_szDontShowTitles.
//
//		06/12/97 MJR	Renamed m_pszRealm to m_pszRealmPrefsFile.
//							Removed m_sServer and m_sClient and m_sDemo.
//
//		06/13/97 MJR	Removed m_bDontBlit and cleaned up the loads and saves
//							and made sure everything defaulted properly.
//
//		06/16/97 MJR	Added m_pszDemoMovie.
//							Added m_sNetGetInputInterval
//							Added m_sNetSendInputInterval.
//							Added m_sNetMaxFrameLag.
//							Added m_sNetTimePerFrame.
//
//					MJR	Added m_lNetMaxBlockingTime.
//							Added m_sNetUsersMaxPlayers;
//
//					JMI	Added m_sTrickySystemQuit.
//
//		06/17/97 MJR	Added m_lNetForceAbortTime.
//
//		06/19/97	JMI	Added m_sResetMPScoresEachLevel.
//
//		06/23/97	JMI	Added m_szSynchLogFile.
//
//		07/07/97	JMI	Added m_dGameFilmScale and m_dEditorFilmScale.
//
//		07/07/97	JMI	Removed m_dEditorFilmScale and added m_sEditorViewWidth
//							and m_sEditorViewHeight.
//
//		07/16/97	JMI	Changed m_lTitleLoadLoops to m_lTitleDuration.
//
//		07/20/97	JMI	Added m_sVolumeDistance and m_sGripZoneRadius.
//
//		07/23/97 BRH	Added several different values for m_lTitleDuration so
//							that each title screen can have its own setting in
//							the postal.ini file.
//
//		08/03/97 BRH	Added m_usProtocol to be loaded and saved into the ini
//							file.
//
//		08/04/97	JMI	Added m_sPlayAmbientSounds.
//
//		08/05/97	JMI	Added m_eNetConnectionType and 
//							ms_apszNetConnectionTypeNames[].
//
//		08/11/97 MJR	Added m_szHostName and modified connection types and text.
//
//		08/18/97 MJR	Lots of changes to network-related stuff.
//
//		08/20/97 BRH	Added paths for the sound, hoods, and game saks to give
//							more install options.
//
//		08/23/97	JMI	Added ms_apszPlayerColorDescriptions[] to provide color
//							descriptions and ms_sNumPlayerColorDescriptions.
//
//		08/25/97	JMI	Now includes the m_eCurSoundQuality enum representing the
//							game's current sound quality.
//							Also, now sets all the default volumes to one value and
//							stores all values from 0 to the UserMaxVolume instead of
//							in the 0..MaxVolume scale samplemaster uses.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef GAMESETTINGS_H
#define GAMESETTINGS_H

#include "RSPiX.h"
#ifdef PATHS_IN_INCLUDES
	#include "WishPiX/Prefs/prefs.h"
#else
	#include "prefs.h"
#endif

#include "settings.h"
#include "localize.h"
#include "SampleMaster.h"

#define MAX_TITLE_SCREENS	10

// Game settings
class CGameSettings : CSettings
	{
	public:
		char		m_pszCDPath[RSP_MAX_PATH+1];			// CD (Compact Disc) Path.  Where product was installed from.
		char		m_pszHDPath[RSP_MAX_PATH+1];			// HD (Hard Drive) Path.  Where product was installed to.
		char		m_pszVDPath[RSP_MAX_PATH+1];			// VD (Variable Drive) Path.  Either HD or CD path, depending on optional install.
		char		m_pszSoundPath[RSP_MAX_PATH+1];		// Path for sound files
		char		m_pszGamePath[RSP_MAX_PATH+1];		// Path for game sak and other game files
		char		m_pszHoodsPath[RSP_MAX_PATH+1];		// Path for level hoods
		char		m_szNoSakDir[RSP_MAX_PATH];			// NoSakDir path.  The root path to files in the absence of their SAK.
		char		m_pszRealmPrefsFile[RSP_MAX_PATH+1];// Name of realm prefs file

		short		m_sDifficulty;								// Difficulty level (0 to 11)
		short		m_sViolence;								// Violence level (0 to 11)
		short		m_sCrossHair;								// TRUE, to use crosshair.

		char		m_szServerName[RSP_MAX_PATH];			// Name of server for network game.
		USHORT	m_usServerPort;							// Port on server for network game.
		USHORT	m_usProtocol;								// Network protocol enum
		char		m_szPlayerName[256];						// Player's name for multiplayer mode.
		short		m_sPlayerColorIndex;						// Player's color index for multiplayer mode.
		short		m_sNetBandwidth;							// Network bandwidth
		short		m_sHostMinBandwidth;						// Host's minimum network bandwidth
		short		m_sHostMaxPlayers;						// Host's max players
		char		m_szHostName[RSP_MAX_PATH];			// Host name
		short		m_sHostResetScoresEachLevel;			// Host's reset-scores-each-level flag
		short		m_sHostRejuvenate;						// Host's rejuvenation flag
		short		m_sHostTimeLimit;							// Host's time limit
		short		m_sHostKillLimit;							// Host's kill limit
		short		m_sNetGetInputInterval;					// Interval between getting input
		short		m_sNetSendInputInterval;				// Interval between sending input
		short		m_sNetMaxFrameLag;						// Maximum lag between output frame and input seq
		short		m_sNetTimePerFrame;						// Time per frame
		long		m_lNetMaxBlockingTime;					// Maximum network blocking time
		long		m_lNetForceAbortTime;					// Maximum time after which to force abort
/*** 12/5/97 AJC ***/
		char		m_szNetSyncLogFile[RSP_MAX_PATH+1];	// Log file name for recording network syn time
		RFile		m_rfNetSyncLog;							// Log file for network syn time
		long		m_lStartRealmTime;						// Time when a client receives START_REALM from server
		char		m_bLogNetTime;								// True, if user wants to log net time
/*** 12/5/97 AJC ***/
/*** 01/14/98 SPA ***/
		long		m_lPeerDropMaxWaitTime;					// Maximum time to wait for peer data before
															// dropping peer (in seconds!!)
/*** 01/14/98 SPA ***/

		long		m_lInitialDemoTimeOut;					// Initial demo timeout.
		long		m_lPersistentDemoTimeOut;				// Persistent demo timeout.
		short		m_sCanRecordDemos;						// TRUE, if the user can record demos.
		char		m_szDemoDebugMovie[RSP_MAX_PATH];	// If supplied, turns on recording of demo movie to this file name
		short		m_sNumAvailableDemos;					// Number of available demos

		long		m_alTitleDurations[MAX_TITLE_SCREENS];// Time spent on title screen while "loading".
																
		short		m_sGammaVal;								// Gamma brighten value for palette.
		short		m_sUseCurrentDeviceDimensions;		// If TRUE, the current display device dimensions are not changed.
																	
		short		m_sAlphaBlend;								// TRUE, if alpha blending is on.
		short		m_sXRayEffect;								// TRUE, if x-ray effect is on.
		short		m_s3dFog;									// TRUE, if 3D objects are rendered with fog (lighting).
		short		m_sParticleEffects;						// TRUE, if particle effects are to be used.
		short		m_sVolumeDistance;						// TRUE, if volume varied by distance is on.
		short		m_sPlayAmbientSounds;					// TRUE, if we should play ambient sounds.
																
		short		m_sDisplayInfo;							// TRUE, to show display info.
																
		short		m_sCanTakeSnapShots;						// TRUE, to be able to take snap shots.
																
		char		m_szDontShowTitles[512];				// Comma delimited list of title filenames that won't be shown

		short		m_sTrickySystemQuit;						// TRUE, to use 'trickier' system quit.

		char		m_szSynchLogFile[RSP_MAX_PATH];		// If not "", logs if's to specified file.

		double	m_dGameFilmScale;							// Percentage of default film size for Play mode.
																	// For example, if 50, what was normally displayed on 640x480
																	// would be 320x240.

		short		m_sEditorViewWidth;						// Initial display size for editor.
		short		m_sEditorViewHeight;						// Initial display size for editor.

		short		m_sGripZoneRadius;						// Radius of non scroll area to be specified to grip.

		SampleMaster::SoundQuality	m_eCurSoundQuality;	// Current sound quality.

		short		m_asCategoryVolumes[SampleMaster::MAX_NUM_SOUND_CATEGORIES];	// Volumes for each category.

	public:	// Statics.
		
		// Player color descriptions.
		static char*	ms_apszPlayerColorDescriptions[];
		// Number of color descriptions.
		static const short	ms_sNumPlayerColorDescriptions;

	public:
		// Set settings to default values
		CGameSettings(void);

		// Destructor
		~CGameSettings();

		// Read settings that are stored in preference file
		short LoadPrefs(
			RPrefs* pPrefs);

		// Write settings that are stored in preference file
		short SavePrefs(
			RPrefs* pPrefs);

		// Load settings that are stored in game file
		short LoadGame(
			RFile* pFile);

		// Save settings that are stored in game file
		short SaveGame(
			RFile* pFile);

		// Temporarily set settings for demo mode (file is for saving current settings)
		short PreDemo(
			RFile* pFile);

		// Restore settings to what they were prior to demo mode
		short PostDemo(
			RFile* pFile);
	};

#endif // GAMESETTINGS_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
