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
// play.h
// Project: Postal
//
// History:
//		12/04/96 MJR	Started.
//
//		03/07/97	JMI	Added Play_SetRealmName() so an outside force can choose
//							the realm to load.
//
//		05/27/97	JMI	Added #include of "Menus.h".
//
//		06/11/97	JMI	Added protos for Play_GetRealmName() and Play_SetRealmNum().
//
//		06/12/97 MJR	Reworked the entire interface to this module.
//
//		07/14/97 BRH	Added additional challenge mode parameter to Play()
//							and Play_GetRealmInfo() to indicate if a challenge
//							cycle is being played.
//
//		07/16/97	JMI	Made SnapPicture() extern.
//
//		07/23/97	JMI	Made UpdateDisplays() and GetAppDescriptor() extern.
//							Also, renamed these two and SnapPicture() to be preceded
//							by Play_.
//
//		08/13/97 MJR	Added difficulty as parameter to Play().
//
//		08/14/97	JMI	Added difficulty paramter to Play_GetRealmSectionAndEntry()
//							and Play_GetRealmInfo().
//
//		08/14/97	JMI	Converted Play_VerifyQuitMenuChoice() to returning true to
//							accept or false to deny.
//
//		08/18/97	JMI	Now defines a variable that allows you to go to the next
//							level if SALES_DEMO is defined.
//
//		08/27/97 MJR	Added frames-per-sec to Play().
//
//		11/20/97	JMI	Added bCoopLevels & bCoopMode parameters to 
//							Play_GetRealmInfo() and Play_GetRealmSectionAndEntry() 
//							calls.
//							Also, added sCoopLevels & sCoopMode to Play() call.
//
//		12/01/97 BRH	Added bAddOn parameters to Play_GetRealmInfo() and
//							Play_GetRealmSectionAndEntry() calls.  Added bAddOn
//							flag to Play() call.  
//
////////////////////////////////////////////////////////////////////////////////
#ifndef PLAY_H
#define PLAY_H

#include "RSPiX.h"
#include "menus.h"
#include "netclient.h"
#include "netserver.h"
#include "input.h"
#include "camera.h"
#include "dude.h"

#ifdef MOBILE
#include "android/android.h"
#endif
////////////////////////////////////////////////////////////////////////////////
//
// Play game using specified settings.
//
////////////////////////////////////////////////////////////////////////////////
extern short Play(										// Returns 0 if successfull, non-zero otherwise
	CNetClient*	pclient,									// In:  Client object or NULL if not network game
	CNetServer*	pserver,									// In:  Server object or NULL if not server or not network game
	INPUT_MODE inputMode,								// In:  Input mode
	const short sRealmNum,								// In:  Realm number to start on or -1 to use specified realm file
	const char*	pszRealmFile,							// In:  Realm file to play (ignored if sRealmNum >= 0)
	const bool bJustOneRealm,							// In:  Play just this one realm (ignored if sRealmNum < 0)
	const bool bGauntlet,								// In:  Play challenge levels gauntlet - as selected on menu
	const bool bAddOn,									// In:  Play Add On levels
	const short sDifficulty,							// In:  Difficulty level
	const bool bRejuvenate,								// In:  Whether to allow players to rejuvenate (MP only)
	const short sTimeLimit,								// In:  Time limit for MP games (0 or negative if none)
	const short sKillLimit,								// In:  Kill limit for MP games (0 or negative if none)
	const	short	sCoopLevels,							// In:  Zero for deathmatch levels, non-zero for cooperative levels.
	const	short	sCoopMode,								// In:  Zero for deathmatch mode, non-zero for cooperative mode.
	const short sFrameTime,								// In:  Milliseconds per frame (MP only)
	RFile* pfileDemoModeDebugMovie);					// In:  File for loading/saving demo mode debug movie


////////////////////////////////////////////////////////////////////////////////
//
// Get info about specified realm
//
////////////////////////////////////////////////////////////////////////////////
extern short Play_GetRealmInfo(						// Returns 0 if successfull, 1 if no such realm, negative on error
	bool	bNetwork,										// In:  true if network game, false otherwise
	bool	bCoop,											// In:  true if coop net game, false otherwise -- no effect if bNetwork is false.
	bool  bGauntlet,										// In:  true if playing multiple challenge levels.
	bool  bAddOn,											// In:  true if playing new Add on levels
	short sRealmNum,										// In:  Realm number
	short	sDifficulty,									// In:  Realm difficulty.
	char* pszFile,											// Out: Realm's file name
	short sMaxFileLen,									// In:  Max length of returned file name, including terminating null
	char* pszTitle = 0,									// Out: Realm's title
	short sMaxTitleLen = NULL);						// In:  Max length of returned title, including terminating null


////////////////////////////////////////////////////////////////////////////////
//
// Get the section and entry that should be used when querying the realms prefs
// file for the described realm.
//
////////////////////////////////////////////////////////////////////////////////
extern void Play_GetRealmSectionAndEntry(
	bool	bNetwork,										// In:  true if network game, false otherwise
	bool	bCoop,											// In:  true if coop net game, false otherwise -- no effect if bNetwork is false.
	bool  bGauntlet,										// In:  true if playing challenge mode
	bool  bAddOn,											// In:  true if playing new add on levels
	short sRealmNum,										// In:  Realm number
	short	sDifficulty,									// In:  Realm difficulty.
	RString* pstrSection,								// Out: Section is returned here
	RString* pstrEntry);									// Out: Entry is returned here


////////////////////////////////////////////////////////////////////////////////
//
// Callback from g_menuVerifyQuitGame with choice.
//
////////////////////////////////////////////////////////////////////////////////
extern bool Play_VerifyQuitMenuChoice(				// Returns true to accept, false to deny choice.
	Menu*	pmenuCurrent,									// In:  Current menu.
	short	sMenuItem);										// In:  Item chosen or -1 for change of focus.


////////////////////////////////////////////////////////////////////////////////
//
// Snap picture to disk.
//
////////////////////////////////////////////////////////////////////////////////
extern void Play_SnapPicture(void);


////////////////////////////////////////////////////////////////////////////////
//
// Creates descriptor including app's time stamp, debug status (debug or release)
// and, if defined, TRACENASSERT flag.
//
////////////////////////////////////////////////////////////////////////////////
extern void Play_GetApplicationDescriptor(		// Returns nothing.
	char* pszText,											// Out: Text descriptor.
	short	sMaxBytes);										// In:  Amount of writable 
																// memory pointed to by pszText.

#ifdef SALES_DEMO
	// When true, one can advance to the next level without meeting the goal.
	extern bool g_bEnableLevelAdvanceWithoutGoal;
#endif

#endif //PLAY_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
