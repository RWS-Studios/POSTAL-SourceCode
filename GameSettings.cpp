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
// GameSettings.cpp
// Project: Postal
//
// History:
//		03/31/97 JMI	Started.
//
//		03/31/97	JMI	Moved CGameSettings implementation from game.h to this 
//							file.
//
//		03/31/97	JMI	Now loads game play keys from prefs file.
//
//		04/07/97	JMI	Added m_szServerName and m_usServerPort.
//
//		04/08/97	JMI	Added m_szPlayerName and m_sPlayerColorIndex.
//
//		04/08/97	JMI	Changed defualt for m_usServerPort from 9999 to 61663.
//
//		04/11/97	JMI	No longer sets m_sClient and m_sServer in PreDemo().
//
//		04/14/97	JMI	Added TEMP flag m_bDontBlit to allow us to speed up the
//							Snap() call by not blitting.
//
//		04/21/97 MJR	Added m_pszRealm for the name of the realm prefs file.
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
//							Everything now sets defaults in constructor instead of
//							some things doing it in the Load() portion.
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
//					MJR	Modified to call newer version of CorrectifyBasePath().
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
//		07/08/97 MJR	Added use of separate section ("MacPaths") for mac paths.
//
//		07/16/97	JMI	Changed m_lTitleLoadLoops to m_lTitleDuration.
//
//		07/17/97	JMI	Now saves and loads volume category settings.
//
//		07/20/97	JMI	Added m_sVolumeDistance and m_sGripZoneRadius.
//							Also, now saves m_sDisplayInfo.
//
//		07/23/97 BRH	Changed title durations so that each title screen
//							can have its own duration time rather than splitting
//							it up evenly across all title screens.
//
//		08/03/97 BRH	Added m_usProtocol to be loaded and saved in the ini
//							file.
//
//		08/04/97	JMI	Added m_sPlayAmbientSounds.
//
//		08/05/97	JMI	Added m_eNetConnectionType and 
//							ms_apszNetConnectionTypeNames[].
//
//		08/11/97 MJR	Added m_szHostName and modified connection types and text.
//
//		08/13/97	JMI	PreDemo() now uses difficulty level 10.
//
//		08/18/97 MJR	Lots of changes to network-related stuff.
//
//		08/20/97 BRH	Added Sound, Game and Hoods paths to give more 
//							installation options.
//
//		08/23/97	JMI	Added ms_apszPlayerColorDescriptions[] to provide color
//							descriptions and ms_sNumPlayerColorDescriptions.
//
//		08/24/97	JMI	Editor width and height now default to 640 and 480.
//
//		08/25/97	JMI	Changed some of the color descriptors.
//
//		08/25/97	JMI	Now includes the m_eCurSoundQuality enum representing the
//							game's current sound quality.
//							Also, now sets all the default volumes to one value and
//							stores all values from 0 to the UserMaxVolume instead of
//							in the 0..MaxVolume scale samplemaster uses.
//
//		09/06/97 MJR	Clamped net values to valid ranges.
//
//		09/07/97 MJR	Now defaults to 2 for network lag.
//
//////////////////////////////////////////////////////////////////////////////
//
// Implementation for CGameSettings object.  Each instance contains settings
// for Postal.
//
//////////////////////////////////////////////////////////////////////////////

#include "RSPiX.h"
#include "GameSettings.h"
#include "game.h"
#include "net.h"
#include "SampleMaster.h"
#include "socket.h"
#include "dude.h"	// For MaxTextures.

//////////////////////////////////////////////////////////////////////////////
// Macros.
//////////////////////////////////////////////////////////////////////////////

// Determines the number of elements in the passed array at compile time.
#define NUM_ELEMENTS(a)					(sizeof(a) / sizeof(a[0]) )

// Clamp the specified value so it's between min and max, inclusive
template <class T>
inline T CLAMP(T val, T min,T max)
	{
	if (val < min)
		val = min;
	if (val > max)
		val = max;
	return val;
	}


//////////////////////////////////////////////////////////////////////////////
// Instantiate static members.
//////////////////////////////////////////////////////////////////////////////

// Player color descriptions.
char*	CGameSettings::ms_apszPlayerColorDescriptions[CDude::MaxTextures + 1]	=
	{
	"Black",
	"Green",
	"Blue",
	"Gray",
	"Brown",
	"Red",
	"Tan",
	"Purple",

	// Add new colors above this line.
	"Error",	// Try to catch errors.
	};

// Number of color descriptions.
const	short CGameSettings::ms_sNumPlayerColorDescriptions	= NUM_ELEMENTS(ms_apszPlayerColorDescriptions) - 1;

//////////////////////////////////////////////////////////////////////////////
// Set settings to default values
//////////////////////////////////////////////////////////////////////////////
CGameSettings::CGameSettings(void)
	{
	m_pszCDPath[0]					= 0;
	m_pszHDPath[0]					= 0;
	m_pszVDPath[0]					= 0;
	m_pszSoundPath[0]				= 0;
	m_pszGamePath[0]				= 0;
	m_pszHoodsPath[0]				= 0;
	m_szNoSakDir[0]				= 0;
	m_pszRealmPrefsFile[0]		= 0;
										
	m_sDifficulty					= 5;
	m_sViolence						= 11;
	m_sCrossHair					= TRUE;
										
	m_szServerName[0]				= 0;
	m_usServerPort					= 61663;
	m_usProtocol					= RSocket::FirstProtocol;
	m_szPlayerName[0]				= 0;
	m_sPlayerColorIndex			= 0;
	m_sNetBandwidth				= Net::Analog28_8;
	m_sHostMinBandwidth			= Net::Analog14_4;
	m_sHostMaxPlayers				= Net::MaxNumIDs;
	m_szHostName[0]				= 0;
	m_sHostResetScoresEachLevel = TRUE;
	m_sHostRejuvenate				= TRUE;
	m_sHostTimeLimit				= 0;
	m_sHostKillLimit				= 20;
	m_lNetMaxBlockingTime		= 10000;
	m_lNetForceAbortTime			= 5000;
	m_sNetGetInputInterval		= 100;
	m_sNetSendInputInterval		= 500;
	m_sNetMaxFrameLag				= 2;		// For the new networking stuff, 2 seems to be the "right" number
	m_sNetTimePerFrame			= 200;
/*** 12/5/97 AJC ***/
	m_szNetSyncLogFile[0]		= 0;
	m_lStartRealmTime				= 0L;
	m_bLogNetTime					= false;		// Default is not to log net time
/*** 12/5/97 AJC ***/
/*** 01/14/98 SPA ***/
	m_lPeerDropMaxWaitTime		= 10000;			// Default to 10 seconds
/*** 01/14/98 SPA ***/

	m_lInitialDemoTimeOut		= 10000;
	m_lPersistentDemoTimeOut	= 60000;
	m_sCanRecordDemos				= FALSE;
	m_szDemoDebugMovie[0]		= 0;
	m_sNumAvailableDemos			= 0;

	m_sGammaVal						= 128;
	m_sUseCurrentDeviceDimensions = 0;

	m_sAlphaBlend					= TRUE;
	m_sXRayEffect					= TRUE;					
	m_s3dFog							= TRUE;
	m_sParticleEffects			= TRUE;
	m_sVolumeDistance				= TRUE;
	m_sPlayAmbientSounds			= TRUE;
										
	m_sDisplayInfo					= FALSE;
										
	m_sCanTakeSnapShots			= FALSE;
										
	m_szDontShowTitles[0]		= 0;

	m_sTrickySystemQuit			= FALSE;

	m_szSynchLogFile[0]			= '\0';

	m_dGameFilmScale				= 1.0;	// 100%
	
	m_sEditorViewWidth			= 640;
	m_sEditorViewHeight			= 480;

	m_sGripZoneRadius				= 75;

	m_eCurSoundQuality			= SampleMaster::SQ_22050_8;

	// Initialize all category volumes.
	short i;
	for (i = 0; i < SampleMaster::MAX_NUM_SOUND_CATEGORIES; i++)
		{
		m_asCategoryVolumes[i] = SampleMaster::UserDefaultVolume;
		}
	}


//////////////////////////////////////////////////////////////////////////////
// Destructor
//////////////////////////////////////////////////////////////////////////////
CGameSettings::~CGameSettings()
	{
	}


//////////////////////////////////////////////////////////////////////////////
// Read settings that are stored in preference file
//////////////////////////////////////////////////////////////////////////////
short CGameSettings::LoadPrefs(
	RPrefs* pPrefs)
	{
	short sResult = 0;

	pPrefs->GetVal("Paths", "CD", "", m_pszCDPath);
#if defined(PANDORA) || defined(ODROID)
	strcpy(m_pszCDPath, ".");
#endif

	sResult = (strlen(m_pszCDPath) + 1) <= RSP_MAX_PATH ? 0 : -1;
	if (sResult == 0)
		sResult = CorrectifyBasePath(m_pszCDPath, sizeof(m_pszCDPath));
	if (sResult)
		rspMsgBox(RSP_MB_ICN_STOP | RSP_MB_BUT_OK, g_pszCriticalErrorTitle, g_pszBadCDPath_s_s, "CD", "Paths");

	pPrefs->GetVal("Paths", "HD", "", m_pszHDPath);
#if defined(PANDORA) || defined(ODROID)
	strcpy(m_pszHDPath, ".");
#endif
	sResult = (strlen(m_pszHDPath) + 1) <= RSP_MAX_PATH ? 0 : -1;
	if (sResult == 0)
		sResult = CorrectifyBasePath(m_pszHDPath, sizeof(m_pszHDPath));
	if (sResult)
		rspMsgBox(RSP_MB_ICN_STOP | RSP_MB_BUT_OK, g_pszCriticalErrorTitle, g_pszBadPath_s_s, "HD", "Paths");

	pPrefs->GetVal("Paths", "VD", "", m_pszVDPath);
#if defined(PANDORA) || defined(ODROID)
	strcpy(m_pszVDPath, ".");
#endif
	sResult = (strlen(m_pszVDPath) + 1) <= RSP_MAX_PATH ? 0 : -1;
	if (sResult == 0)
		sResult = CorrectifyBasePath(m_pszVDPath, sizeof(m_pszVDPath));
	if (sResult)
		rspMsgBox(RSP_MB_ICN_STOP | RSP_MB_BUT_OK, g_pszCriticalErrorTitle, g_pszBadPath_s_s, "VD", "Paths");

	pPrefs->GetVal("Paths", "Sound", "", m_pszSoundPath);
#if defined(PANDORA) || defined(ODROID)
	strcpy(m_pszSoundPath, ".");
#endif
	sResult = (strlen(m_pszSoundPath) + 1) <= RSP_MAX_PATH ? 0 : -1;
	if (sResult == 0)
		sResult = CorrectifyBasePath(m_pszSoundPath, sizeof(m_pszSoundPath));
	if (sResult)
		rspMsgBox(RSP_MB_ICN_STOP | RSP_MB_BUT_OK, g_pszCriticalErrorTitle, g_pszBadPath_s_s, "Sound", "Paths");

	pPrefs->GetVal("Paths", "Game", "", m_pszGamePath);
#if defined(PANDORA) || defined(ODROID)
	strcpy(m_pszGamePath, ".");
#endif
	sResult = (strlen(m_pszGamePath) + 1) <= RSP_MAX_PATH ? 0 : -1;
	if (sResult == 0)
		sResult = CorrectifyBasePath(m_pszGamePath, sizeof(m_pszGamePath));
	if (sResult)
		rspMsgBox(RSP_MB_ICN_STOP | RSP_MB_BUT_OK, g_pszCriticalErrorTitle, g_pszBadPath_s_s, "Game", "Paths");

	pPrefs->GetVal("Paths", "Hoods", "", m_pszHoodsPath);
#if defined(PANDORA) || defined(ODROID)
	strcpy(m_pszHoodsPath, ".");
#endif
	sResult = (strlen(m_pszHoodsPath) + 1) <= RSP_MAX_PATH ? 0 : -1;
	if (sResult == 0)
		sResult = CorrectifyBasePath(m_pszHoodsPath, sizeof(m_pszHoodsPath));
	if (sResult)
		rspMsgBox(RSP_MB_ICN_STOP | RSP_MB_BUT_OK, g_pszCriticalErrorTitle, g_pszBadPath_s_s, "Hoods", "Paths");

	pPrefs->GetVal("Paths", "NoSakDir", "", m_szNoSakDir);
	sResult = (strlen(m_szNoSakDir) + 1) <= RSP_MAX_PATH ? 0 : -1;
	if (sResult == 0)
		sResult = CorrectifyBasePath(m_szNoSakDir, sizeof(m_szNoSakDir));
	if (sResult)
		rspMsgBox(RSP_MB_ICN_STOP | RSP_MB_BUT_OK, g_pszCriticalErrorTitle, g_pszBadPath_s_s, "NoSakDir", "Paths");

	pPrefs->GetVal("Realms", "File", "", m_pszRealmPrefsFile);
	if (strlen(m_pszRealmPrefsFile) == 0)
		{
		sResult = -1;
		rspMsgBox(RSP_MB_ICN_STOP | RSP_MB_BUT_OK, g_pszCriticalErrorTitle, g_pszBadPath_s_s, "File", "Realms");
		}
	else if ((strlen(m_pszRealmPrefsFile) + 1) >= RSP_MAX_PATH)
		{
		sResult = -1;
		rspMsgBox(RSP_MB_ICN_STOP | RSP_MB_BUT_OK, g_pszCriticalErrorTitle, g_pszBadPath_s_s, "File", "Realms");
		}

	pPrefs->GetVal("Game", "RecentDifficulty", m_sDifficulty, &m_sDifficulty);
	if (m_sDifficulty < 0)
		m_sDifficulty = 0;
	if (m_sDifficulty > 11)
		m_sDifficulty = 11;
	pPrefs->GetVal("Game", "RecentViolence", m_sViolence, &m_sViolence);
	if (m_sViolence < 0)
		m_sViolence = 0;
	if (m_sViolence > 11)
		m_sViolence = 11;
	pPrefs->GetVal("Game", "UseCrossHair", m_sCrossHair, &m_sCrossHair);

	pPrefs->GetVal("Multiplayer", "Server", m_szServerName, m_szServerName);
	pPrefs->GetVal("Multiplayer", "Port", m_usServerPort, &m_usServerPort);
	pPrefs->GetVal("Multiplayer", "Protocol", m_usProtocol, &m_usProtocol);
	if (m_usProtocol >= RSocket::NumProtocols)
		m_usProtocol = RSocket::FirstProtocol;
	pPrefs->GetVal("Multiplayer", "Name", m_szPlayerName, m_szPlayerName);
	pPrefs->GetVal("Multiplayer", "Color", m_sPlayerColorIndex, &m_sPlayerColorIndex);
	pPrefs->GetVal("Multiplayer", "Bandwidth", m_sNetBandwidth, &m_sNetBandwidth);
	if (m_sNetBandwidth < 0) m_sNetBandwidth = 0;
	if (m_sNetBandwidth >= Net::NumBandwidths) m_sNetBandwidth = Net::NumBandwidths - 1;
	pPrefs->GetVal("Multiplayer", "HostMinBandwidth", m_sHostMinBandwidth, &m_sHostMinBandwidth);
	if (m_sHostMinBandwidth < 0) m_sHostMinBandwidth = 0;
	if (m_sHostMinBandwidth >= Net::NumBandwidths) m_sHostMinBandwidth = Net::NumBandwidths - 1;
	pPrefs->GetVal("Multiplayer", "HostMaxPlayers", m_sHostMaxPlayers, &m_sHostMaxPlayers);
	if (m_sHostMaxPlayers > Net::MaxNumIDs)
		m_sHostMaxPlayers = Net::MaxNumIDs;
	char szHostName[RSP_MAX_PATH];
	pPrefs->GetVal("Multiplayer", "HostName", "", szHostName);
	strncpy(m_szHostName, szHostName, sizeof(m_szHostName));
	m_szHostName[sizeof(m_szHostName)-1] = 0;
	pPrefs->GetVal("Multiplayer", "HostResetScoresEachLevel", m_sHostResetScoresEachLevel, &m_sHostResetScoresEachLevel);
	pPrefs->GetVal("Multiplayer", "HostRejuvenate", m_sHostRejuvenate, &m_sHostRejuvenate);
	pPrefs->GetVal("Multiplayer", "HostTimeLimit", m_sHostTimeLimit, &m_sHostTimeLimit);
	pPrefs->GetVal("Multiplayer", "HostKillLimit", m_sHostKillLimit, &m_sHostKillLimit);
	pPrefs->GetVal("Multiplayer", "GetInputInterval", m_sNetGetInputInterval, &m_sNetGetInputInterval);
	pPrefs->GetVal("Multiplayer", "SendInputInterval", m_sNetSendInputInterval, &m_sNetSendInputInterval);
	pPrefs->GetVal("Multiplayer", "MaxFrameLag", m_sNetMaxFrameLag, &m_sNetMaxFrameLag);
	m_sNetMaxFrameLag = CLAMP(m_sNetMaxFrameLag, (short)0, (short)Net::MaxAheadSeq);
	pPrefs->GetVal("Multiplayer", "TimePerFrame", m_sNetTimePerFrame, &m_sNetTimePerFrame);
	m_sNetTimePerFrame = CLAMP(m_sNetTimePerFrame, (short)Net::MinFrameTime, (short)200);
	pPrefs->GetVal("Multiplayer", "MaxBlockingTime", m_lNetMaxBlockingTime, &m_lNetMaxBlockingTime);
	pPrefs->GetVal("Multiplayer", "ForceAbortTime", m_lNetForceAbortTime, &m_lNetForceAbortTime);
/*** 12/5/97 AJC ***/
	char szLogNetTime[256];
	szLogNetTime[0] = 0;
	pPrefs->GetVal("Multiplayer", "LogNetTime", "", szLogNetTime);
	if (szLogNetTime[0] == 'y' || szLogNetTime[0] == 'Y')
		{
		m_bLogNetTime = true;
		pPrefs->GetVal("Multiplayer", "NetSyncLogFile", "netsync.log", m_szNetSyncLogFile);
		}
	else
		m_bLogNetTime = false;
/*** 12/5/97 AJC ***/
/*** 01/14/98 SPA ***/
	pPrefs->GetVal("Multiplayer", "PeerDropMaxWaitTime", m_lPeerDropMaxWaitTime, &m_lPeerDropMaxWaitTime);
	m_lPeerDropMaxWaitTime *= 1000;			// Change to milliseconds
/*** 01/14/98 SPA ***/

	pPrefs->GetVal("Demo", "InitialTimeOut", m_lInitialDemoTimeOut, &m_lInitialDemoTimeOut);
	pPrefs->GetVal("Demo", "PersistentTimeOut", m_lPersistentDemoTimeOut, &m_lPersistentDemoTimeOut);
	pPrefs->GetVal("Demo", "CanRecordDemos", m_sCanRecordDemos, &m_sCanRecordDemos);
	pPrefs->GetVal("Demo", "DemoDebugMovie", m_szDemoDebugMovie, m_szDemoDebugMovie);
	pPrefs->GetVal("Demo", "NumAvailable", m_sNumAvailableDemos, &m_sNumAvailableDemos);

	short i;
	char szDurationName[100];
	for (i = 0; i < MAX_TITLE_SCREENS; i++)
	{
		sprintf(szDurationName, "Duration%d", i+1);
		pPrefs->GetVal("Title", szDurationName, 3000, &(m_alTitleDurations[i]));
	}

	pPrefs->GetVal("Video", "GammaVal", m_sGammaVal, &m_sGammaVal);
	pPrefs->GetVal("Video", "UseCurrentDeviceDimensions", m_sUseCurrentDeviceDimensions, &m_sUseCurrentDeviceDimensions);
	pPrefs->GetVal("Video", "GameFilmScale", m_dGameFilmScale, &m_dGameFilmScale);
	pPrefs->GetVal("Video", "EditorViewWidth", m_sEditorViewWidth, &m_sEditorViewWidth);
	pPrefs->GetVal("Video", "EditorViewHeight", m_sEditorViewHeight, &m_sEditorViewHeight);

	pPrefs->GetVal("Features", "AlphaBlend", m_sAlphaBlend, &m_sAlphaBlend);
	pPrefs->GetVal("Features", "XRayEffect", m_sXRayEffect, &m_sXRayEffect);
	pPrefs->GetVal("Features", "3DLighting", m_s3dFog, &m_s3dFog);
	pPrefs->GetVal("Features", "ParticleEffects", m_sParticleEffects, &m_sParticleEffects);
	pPrefs->GetVal("Features", "VolumeDistance", m_sVolumeDistance, &m_sVolumeDistance);
	pPrefs->GetVal("Features", "PlayAmbientSounds", m_sPlayAmbientSounds, &m_sPlayAmbientSounds);

	pPrefs->GetVal("Debug", "DisplayInfo", m_sDisplayInfo, &m_sDisplayInfo);
	pPrefs->GetVal("Debug", "IfLog", m_szSynchLogFile, m_szSynchLogFile);

	pPrefs->GetVal("Can", "TakeSnapShots", m_sCanTakeSnapShots, &m_sCanTakeSnapShots);

	pPrefs->GetVal("Title", "DontShow", m_szDontShowTitles, m_szDontShowTitles);

	pPrefs->GetVal("Shell", "TrickySystemQuit", m_sTrickySystemQuit, &m_sTrickySystemQuit);

	for (i = 0; i < SampleMaster::MAX_NUM_SOUND_CATEGORIES; i++)
		{
		pPrefs->GetVal("Volumes", SampleMaster::ms_apszSoundCategories[i], SampleMaster::UserDefaultVolume, &m_asCategoryVolumes[i]);
		}

	// Verify we're in bounds.
	if (m_sPlayerColorIndex >= CDude::MaxTextures || m_sPlayerColorIndex < 0)
		{
		m_sPlayerColorIndex	= 0;
		}

	if (!sResult)
		{
		if (pPrefs->IsError())
			sResult = -1;
		}

	return sResult;
	}


//////////////////////////////////////////////////////////////////////////////
// Write settings that are stored in preference file
//////////////////////////////////////////////////////////////////////////////
short CGameSettings::SavePrefs(
	RPrefs* pPrefs)
	{
	pPrefs->SetVal("Game", "RecentDifficulty", m_sDifficulty);
	pPrefs->SetVal("Game", "RecentViolence", m_sViolence);
	pPrefs->SetVal("Game", "UseCrossHair", m_sCrossHair);

	pPrefs->SetVal("Multiplayer", "Server", m_szServerName);
	pPrefs->SetVal("Multiplayer", "Port", m_usServerPort);
	pPrefs->SetVal("Multiplayer", "Protocol", m_usProtocol);
	pPrefs->SetVal("Multiplayer", "Name", m_szPlayerName);
	pPrefs->SetVal("Multiplayer", "Color", m_sPlayerColorIndex);
	pPrefs->SetVal("Multiplayer", "Bandwidth", (long)m_sNetBandwidth);
	pPrefs->SetVal("Multiplayer", "HostMinBandwidth", (long)m_sHostMinBandwidth);
	pPrefs->SetVal("Multiplayer", "HostMaxPlayers", m_sHostMaxPlayers);
	pPrefs->SetVal("Multiplayer", "HostName", m_szHostName);
	pPrefs->SetVal("Multiplayer", "HostResetScoresEachLevel", m_sHostResetScoresEachLevel);
	pPrefs->SetVal("Multiplayer", "HostRejuvenate", m_sHostRejuvenate);
	pPrefs->SetVal("Multiplayer", "HostTimeLimit", m_sHostTimeLimit);
	pPrefs->SetVal("Multiplayer", "HostKillLimit", m_sHostKillLimit);

	pPrefs->SetVal("Video", "GammaVal", m_sGammaVal);
	pPrefs->SetVal("Video", "GameFilmScale", m_dGameFilmScale);
	pPrefs->SetVal("Video", "EditorViewWidth", m_sEditorViewWidth);
	pPrefs->SetVal("Video", "EditorViewHeight", m_sEditorViewHeight);

	pPrefs->SetVal("Features", "AlphaBlend", m_sAlphaBlend);
	pPrefs->SetVal("Features", "XRayEffect", m_sXRayEffect);
	pPrefs->SetVal("Features", "3DLighting", m_s3dFog);
	pPrefs->SetVal("Features", "ParticleEffects", m_sParticleEffects);
	pPrefs->SetVal("Features", "VolumeDistance", m_sVolumeDistance);
	pPrefs->SetVal("Features", "PlayAmbientSounds", m_sPlayAmbientSounds);

	pPrefs->SetVal("Debug", "DisplayInfo", m_sDisplayInfo);

	short i;
	for (i = 0; i < SampleMaster::MAX_NUM_SOUND_CATEGORIES; i++)
		{
		// Save volume scaled to user mode.
		pPrefs->SetVal("Volumes", SampleMaster::ms_apszSoundCategories[i], m_asCategoryVolumes[i] );
		}

	return pPrefs->IsError();
	}


//////////////////////////////////////////////////////////////////////////////
// Load settings that are stored in game file
//////////////////////////////////////////////////////////////////////////////
short CGameSettings::LoadGame(
	RFile* pFile)
	{
	pFile->Read(&m_sDifficulty);
	pFile->Read(&m_sViolence);
	return 0;
	}


//////////////////////////////////////////////////////////////////////////////
// Save settings that are stored in game file
//////////////////////////////////////////////////////////////////////////////
short CGameSettings::SaveGame(
	RFile* pFile)
	{
	pFile->Write(&m_sDifficulty);
	pFile->Write(&m_sViolence);
	return 0;
	}


//////////////////////////////////////////////////////////////////////////////
// Temporarily set settings for demo mode (file is for saving current settings)
//////////////////////////////////////////////////////////////////////////////
short CGameSettings::PreDemo(
	RFile* pFile)
	{
	pFile->Write(&m_sDifficulty);
	pFile->Write(&m_sViolence);
	m_sDifficulty = 10;
	m_sViolence = 11;
	return 0;
	}


//////////////////////////////////////////////////////////////////////////////
// Restore settings to what they were prior to demo mode
//////////////////////////////////////////////////////////////////////////////
short CGameSettings::PostDemo(
	RFile* pFile)
	{
	pFile->Read(&m_sDifficulty);
	pFile->Read(&m_sViolence);
	return 0;
	}


///////////////////////////////////////////////////////////////////////////////
// EOF
///////////////////////////////////////////////////////////////////////////////
