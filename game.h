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
// game.h
// Project: Nostril (aka Postal)
//
// History:
//		12/02/96 MJR	Started.
//
//		01/31/97	JMI	Added g_resmgrGame for specific to actual game data.
//
//		01/31/97	JMI	Added m_lTitleLoadLoops to settings.
//
//		02/03/97	JMI	Added NoSakDir directory for files when there is no SAK
//							to settings.
//
//		02/04/97	JMI	Added functions to get and set the gamma level.
//
//		02/13/97	JMI	Removed game level alpha XRay stuff (now setup in CHood).
//
//		02/21/97	JMI	Now sets gamma level on Save().
//							Also, added m_sUseCurrentDeviceDimensions to settings
//							indicating, if TRUE, not to change the current display
//							device mode.
//
//		03/24/97	JMI	Added m_dDegreesPerSec and m_sUseMouse to CGameSettings.
//
//		03/31/97	JMI	Moved CGameSettings definition to GameSettings.h and the
//							implementation to GameSettings.cpp.
//
//		04/11/97	JMI	Added Game_Menu_Demo() proto.
//
//		05/21/97	JMI	Added a resource manager for resources that are not SAKed.
//
//		06/12/97 MJR	Rewored the callbacks so that the game-specific code now
//							resides in this module rather than the menu module.
//
//		06/16/97	JMI	Added g_fontSmall.
//
//		06/18/97 MJR	Added SeedRandom() and GetRandom().
//
//		06/24/97	JMI	Added SynchLog() and associated macros.
//
//		07/03/97	JMI	Added Game_ControlsMenu().
//
//		07/11/97 BRH	Finished up the expiration date checking for the 
//							Registry.  Still need to add something for the Mac
//							version.  Verified that it works with different times
//							and dates.
//
//		07/13/97	JMI	Added Game_StartChallengeGame() proto.
//
//		07/18/97 BRH	Added game load and save functions so that the player's
//							game can be saved and loaded.  Also added a global
//							stockpile object used to transfer loaded/saved info to/from
//							the CDude's stockpile.
//
//		07/26/97	JMI	Added g_fontPostal to replace g_fontSmall.  Got rid of
//							g_fontSmall.
//
//		08/05/97	JMI	Added Game_AudioOptionsChoice().
//
//		08/12/97	JMI	Added SubPathOpenBox() (see proto for details) and
//							FullPathCustom().
//
//		08/14/97	JMI	Added psDifficulty parameter to Game_Save/LoadPlayersGame()
//							so they can query/modify realm difficulty.
//
//
//		08/15/97	JRD	Added a function to control brightness/contrast in place
//							of gamme.  This was because the gamma function could not
//							recreate a "normal palette."
//
//		08/15/97 JRD	Attempted to hook both brightness and contrast changes to
//							the old gamma slider with a crude algorithm.
//
//		08/20/97	JMI	Moved Game_LoadPlayersGame() proto into game.cpp b/c it
//							now takes an ACTION* but ACTION is defined in game.cpp and
//							since no one currently calls this function externally, what
//							the hell.
//
//		08/21/97 MJR	Added Game_HostMultiGame() and Game_JoinMultiGame().
//
//		08/21/97	JMI	Added rspUpdateDisplay() macros to flag these calls (they
//							should be calls to app level version -- UpdateDisplay() ).
//
//		08/22/97	JMI	Removed rspUpdateDisplay() macro b/c we don't need it now that
//							we're lock/unlocking correctly.
//
//		08/23/97	JMI	Added Game_InitMainMenu() so game.cpp can hook init/kill of
//							main menu.
//
//		10/07/97	JMI	Now GetRandom() uses the logging call in any TRACENASSERT
//							mode (used to be just in _DEBUG mode).
//
//		10/14/97	JMI	LOG() and if() no longer automatically call GetInstanceID()
//							(too many GetRand()s are called from non-CThings).  Now
//							LOG() has an additional argument for specifying the user
//							value which, of course, could be a call to GetInstanceID().
//
//		10/14/97	JMI	Made SynchLog()'s expr parameter a double instead of an int
//							for more accuracy.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef GAME_H
#define GAME_H

#include "RSPiX.h"
#ifdef PATHS_IN_INCLUDES
	#include "WishPiX/ResourceManager/resmgr.h"
#else
	#include "resmgr.h"
#endif

#include "settings.h"
#include "localize.h"
#include "GameSettings.h"
#include "StockPile.h"

////////////////////////////////////////////////////////////////////////////////
// Macros.
////////////////////////////////////////////////////////////////////////////////

// Global game settings
extern CGameSettings g_GameSettings;

// Check for cookie flag
extern long g_lCookieMonster;

// Global screen buffer
extern RImage* g_pimScreenBuf;

// Global big font
extern RFont g_fontBig;

// Global Postal font.
extern RFont g_fontPostal;

// Global flag for end of level 
extern bool g_bLastLevelDemo;

// Resource manager for game resources.  These are resources used by the actual
// game, like things that a CThing loads that is not level specific.  For example,
// CBall loads, of course, foot.bmp, which would be loaded through this ResMgr.
// Note:  This resmgr should not be used for things like info on ordering, menu
// resources, g_fontBig, or anything not specific to the real game.
// Note:  Realm specifc data, such as alpha effects, etc., should be loaded
// through prealm->m_resmgr.
extern RResMgr	g_resmgrGame;

// Resource manager for shell resources.  Do not use this to load things like
// the main dudes' sprites.
extern RResMgr	g_resmgrShell;

// Resource manager for non-SAK resources.
extern RResMgr	g_resmgrRes;

// Time codes for registry values and expiration date
extern long g_lRegTime;
extern long g_lRegValue;
extern long g_lExpTime;
extern long g_lExpValue;
extern long g_lReleaseTime;

// Loaded and saved games use this stockpile to transfer to/from the
// dude's stockpile
extern CStockPile	g_stockpile;
extern bool       g_bTransferStockpile;
extern short	   g_sRealmNumToSave;


////////////////////////////////////////////////////////////////////////////////
//
// Do the high-level startup stuff, run the game, and then cleanup afterwards.
//
// It is assumed that the system/RSPiX environment are setup properly before
// this function is called.
//
////////////////////////////////////////////////////////////////////////////////
extern void TheGame(void);


////////////////////////////////////////////////////////////////////////////////
//
// Callback for the "Start Single Player Game" menu
//
////////////////////////////////////////////////////////////////////////////////
extern void Game_StartSinglePlayerGame(
	short sMenuItem);


////////////////////////////////////////////////////////////////////////////////
//
// Callback for the "Start MultiPlayer Game" menu
//
////////////////////////////////////////////////////////////////////////////////
extern bool Game_StartMultiPlayerGame(
	short sMenuItem);


////////////////////////////////////////////////////////////////////////////////
//
// Callback for the "Join MultiPlayer Game" menu
//
////////////////////////////////////////////////////////////////////////////////
extern void Game_JoinMultiPlayerGame(
	short sMenuItem);


////////////////////////////////////////////////////////////////////////////////
//
// Callback for the "Host MultiPlayer Game" menu
//
////////////////////////////////////////////////////////////////////////////////
extern void Game_HostMultiPlayerGame(
	short sMenuItem);

////////////////////////////////////////////////////////////////////////////////
//
// Callback for the Main Menu init/kill.
//
////////////////////////////////////////////////////////////////////////////////
extern void Game_InitMainMenu(	// Returns nothing.
	short sInit);						// In:  TRUE, if initializing; FALSE, if killing.


////////////////////////////////////////////////////////////////////////////////
//
// Callback for the "Start Demo Game" menu
//
////////////////////////////////////////////////////////////////////////////////
extern void Game_StartDemoGame(
	short sMenuItem);


////////////////////////////////////////////////////////////////////////////////
//
// Callback for the "Editor" option on the Main Menu
//
////////////////////////////////////////////////////////////////////////////////
extern void Game_StartEditor(void);

////////////////////////////////////////////////////////////////////////////////
//
// Callback for the "Buy" option on the Main Menu
//
////////////////////////////////////////////////////////////////////////////////
extern void Game_Buy(void);

////////////////////////////////////////////////////////////////////////////////
//
// Callback for the "Controls" menu.
//
////////////////////////////////////////////////////////////////////////////////
extern void Game_ControlsMenu(	// Returns nothing.
	short sMenuItem);					// In:  Chosen menu item.

////////////////////////////////////////////////////////////////////////////////
//
// Callback for the "Start Challenge" menu.
//
////////////////////////////////////////////////////////////////////////////////
extern void Game_StartChallengeGame(	// Returns nothing.
	short sMenuItem);							// In:  Chosen menu item.

////////////////////////////////////////////////////////////////////////////////
//
// Callback for "Audio Options" menu.
//
////////////////////////////////////////////////////////////////////////////////
extern void Game_AudioOptionsChoice(	// Returns nothing.
	short sMenuItem);							// In:  Chosen item.


////////////////////////////////////////////////////////////////////////////////
//
// Save game settings of the current level so that play can continue on this
//	level.
//
////////////////////////////////////////////////////////////////////////////////

extern short Game_SavePlayersGame(	// Returns SUCCESS if all goes well
				char* pszSaveName,		// In:  Name of the save file
				short sDifficulty);		// In:  Current realm difficulty.

////////////////////////////////////////////////////////////////////////////////
//
// Seed the random number generator
//
////////////////////////////////////////////////////////////////////////////////
#define SeedRand SeedRandom
extern void SeedRandom(
	long lSeed);


////////////////////////////////////////////////////////////////////////////////
//
// Get a random number
//
////////////////////////////////////////////////////////////////////////////////
#define GetRand GetRandom

#if defined(_DEBUG) || defined(TRACENASSERT)

	#define GetRandom()	GetRandomDebug(__FILE__, __LINE__)
	extern long GetRandomDebug(char* FILE_MACRO, long LINE_MACRO);

#else

	extern long GetRandom(void);

#endif	// defined(_DEBUG) || defined(TRACENASSERT)

////////////////////////////////////////////////////////////////////////////////
// Synchronization logger -- Call this function to log an expression and a user
// value in the synch log.  When active (if g_GameSettings.m_szSynchLog is
// a valid path and filename), if recording a demo, these calls are logged to
// a file including the calling file and line number.  When active, if playing
// back a demo, these calls are compared to those stored in the log and, if
// a discrepancy occurs, a modal dialog box will pop up with the pertinent info
// followed by an ASSERT(0) for easy debugging.
////////////////////////////////////////////////////////////////////////////////
extern int SynchLog(		// Result of expr.
	double	expr,			// In:  Expression to evaluate.
	char*		pszFile,		// In:  Calling file.
	long		lLine,		// In:  Calling line.
	char*		pszExpr,		// In:  Original C++ source expression.
	U32		u32User);	// In:  A user value that is intended to be consistent.

////////////////////////////////////////////////////////////////////////////////
// If 'LOG_IFS' macro is defined, this will redefine 'if' such that it will 
// still perform its comparison but will also SynchLog() the expression.
////////////////////////////////////////////////////////////////////////////////
#if defined(LOG_IFS)
	#define if(expr)		if (SynchLog(double(expr != 0), __FILE__, __LINE__, #expr, 0 ) )
#endif

////////////////////////////////////////////////////////////////////////////////
// If 'LOG_LOGS' macro is defined, this will define 'LOG' such that it will 
// SynchLog() the expression.
// If the macro is not defined, this macro expands into absolutely nothing using
// no CPU time or memory.  Note that the expression will __NOT__ be evaluated
// at all.
////////////////////////////////////////////////////////////////////////////////
#if defined(LOG_LOGS)
	#define LOG(expr, user_val)		SynchLog(expr, __FILE__, __LINE__, #expr, user_val)
#else
	#define LOG(expr, user_val)
#endif

////////////////////////////////////////////////////////////////////////////////
//
// Do palette transition so menu can be displayed on top of existing background.
// NOTE: There must be a matching PalTranOff() for each PalTranOn()!!!
//
////////////////////////////////////////////////////////////////////////////////
extern void PalTranOn(
	long lTime = -1);											// In:  How long transition should take (or -1 for default)


////////////////////////////////////////////////////////////////////////////////
//
// Undo the palette transition to restore the original background colors.
//
////////////////////////////////////////////////////////////////////////////////
extern void PalTranOff(void);


////////////////////////////////////////////////////////////////////////////////
//
// Create a full path out of a partial path.
//
// The partial path must be in "RSPiX neutral" format, which is pretty much
// like a partial Windows path, except with slashes instead of backslashes.
//
// BEWARE: The return value is a pointer to a static string, which means its
// contents are changed every time this function is called!  If you're just
// going to use the string and then be done with it, this should work fine, but
// if you need the string to stick around a while, you should probably do a
// strcpy() into your own string buffer.
//
// There are several variations - pick the one you like best.
//
////////////////////////////////////////////////////////////////////////////////

#define GAME_PATH_CD			0
#define GAME_PATH_HD			1
#define GAME_PATH_VD			2
#define GAME_PATH_SOUND		3
#define GAME_PATH_GAME		4
#define GAME_PATH_HOODS		5

extern char* FullPath(									// Returns full path in system format
	short sPathType,										// In:  PATH_CD, PATH_HD, or PATH_VD
	char* pszPartialPath);								// In:  Partial path in RSPiX format

extern char* FullPathCD(								// Returns full path in system format
	char* pszPartialPath);								// In:  Partial path in RSPiX format

extern char* FullPathHD(								// Returns full path in system format
	const char* pszPartialPath);						// In:  Partial path in RSPiX format

extern char* FullPathVD(								// Returns full path in system format
	char* pszPartialPath);								// In:  Partial path in RSPiX format

extern char* FullPathSound(							// Returns full path in system format
	char* pszPartialPath);								// In:  Partial path in RSPiX format

extern char* FullPathGame(								// Returns full path in system format
	char* pszPartialPath);								// In:  Partial path in RSPiX format

extern char* FullPathHoods(							// Returns full path in system format
	char* pszPartialPath);								// In:  Partial path in RSPiX format

extern char* FullPathCustom(							// Returns full path in system format
	char*	pszFullPath,									// In:  Full path in in RSPiX format.
	char* pszPartialPath);								// In:  Partial path in RSPiX format.

////////////////////////////////////////////////////////////////////////////////
//
// Correctify the specified base path.
//
// First, it ensures that the path is absolute (not relative).  Then, it ensures
// the path ends properly, either with or without the system-specific separator
// character, depending on which system we're running on.
//
////////////////////////////////////////////////////////////////////////////////
short CorrectifyBasePath(								// Returns 0 if successfull, non-zero otherwise
	char* pszBasePath,									// I/O: Base path to be corrected
	short sMaxPathLen);									// In:  Maximum length of base path

////////////////////////////////////////////////////////////////////////////////
//
// Get a subpath relative to the specified game path.
//
////////////////////////////////////////////////////////////////////////////////
extern short SubPathOpenBox(		// Returns 0 on success, negative on error, 1 if 
											// not subpathable (i.e., returned path is full path).
	char*	pszFullPath,				// In:  Full path to be relative to.
	char* pszBoxTitle,				// In:  Title of box.
	char*	pszDefFileName,			// In:  Default filename.
	char* pszChosenFileName,		// Out: User's choice.
	short sStrSize,					// In:  Amount of memory pointed to by pszChosenFileName.
	char*	pszFilter = NULL);		// In:  If not NULL, '.' delimited extension based filename
											//	filter specification.  Ex: ".cpp.h.exe.lib" or "cpp.h.exe.lib"
											// Note: Cannot use '.' in filter.  Preceding '.' ignored.

////////////////////////////////////////////////////////////////////////////////
//
// Set gamma/brighten-effect value.
//
////////////////////////////////////////////////////////////////////////////////
extern void SetGammaLevel(
	short sBase);											// In:  New brighten value

////////////////////////////////////////////////////////////////////////////////
//
//	Set Brightness and Contrast.  Zero (neutral) values yield and identity
// curve.  Valid input is from -1 to 1.
//
////////////////////////////////////////////////////////////////////////////////
extern	void	SetBrightnessContrast(
						double dBrightness,	// -1.0 = dim, 0.0 = normal, 1.0 = bright
						double dContrast		// -1.0 = low contrast, 0.0 = normal, 1.0 = high
						);

////////////////////////////////////////////////////////////////////////////////
//
// Get gamma/brighten-effect value from palette map (not from settings).
//
////////////////////////////////////////////////////////////////////////////////
extern short GetGammaLevel(void);					// Returns current brighten value

#ifdef WIN32
#define snprintf _snprintf
#define mkdir _mkdir
#endif

extern bool StatsAreAllowed;

extern int Stat_BulletsFired;
extern int Stat_BulletsHit;
extern int Stat_BulletsMissed;
extern int Stat_Deaths;
extern int Stat_Suicides;
extern int Stat_Executions;
extern int Stat_HitsTaken;
extern int Stat_DamageTaken;
extern int Stat_Burns;
extern int Stat_TimeRunning;
extern int Stat_KilledHostiles;
extern int Stat_KilledCivilians;
extern int Stat_TotalKilled;
extern int Stat_LevelsPlayed;

extern long playthroughMS;

extern ULONG Flag_Achievements;
#define FLAG_USED_M16             (1<<0)
#define FLAG_USED_SHOTGUN         (1<<1)
#define FLAG_USED_DBL_SHOTGUN     (1<<2)
#define FLAG_USED_GRENADE         (1<<3)
#define FLAG_USED_ROCKET          (1<<4)
#define FLAG_USED_MOLOTOV         (1<<5)
#define FLAG_USED_NAPALM          (1<<6)
#define FLAG_USED_FLAMETHROWER    (1<<7)
#define FLAG_USED_PROXIMITY_MINE  (1<<8)
#define FLAG_USED_TIMED_MINE      (1<<9)
#define FLAG_USED_REMOTE_MINE     (1<<10)
#define FLAG_USED_BETTY_MINE      (1<<11)
#define FLAG_USED_HEATSEEKER      (1<<12)
#define FLAG_USED_SPRAY_CANNON    (1<<13)
#define FLAG_USED_DEATHWAD        (1<<14)
#define FLAG_MASK_WEAPONS         0x3bfb // everything but dbl_shotgun, remote_mine and deathwad //((1<<15)-1)

#define FLAG_USED_CHEATS          (1<<15)
#define FLAG_KILLED_EVERYTHING    (1<<16)
#define FLAG_KILLED_ONLY_HOSTILES (1<<17)
#define FLAG_HIGHEST_DIFFICULTY   (1<<18)

// max time that still qualifies for the "Two Pump Chump" achievement in milliseconds.
#define MAX_PLAYTHROUGH_ACHIEVEMENT_MS 60 * (1000 * 60)

enum Achievement
{
    ACHIEVEMENT_KILL_FIRST_VICTIM,            // "And so it begins"
    ACHIEVEMENT_START_SECOND_LEVEL,           // "Oh, you pressed F1"
    ACHIEVEMENT_DUCK_UNDER_ROCKET,            // "Dirty knees"
    ACHIEVEMENT_RUN_5_MINUTES,                // "Forrest Gump"
    ACHIEVEMENT_PERFORM_FIRST_EXECUTION,      // "How Kevorkian!"
    ACHIEVEMENT_KILL_100,                     // "Patrick Bateman"
    ACHIEVEMENT_KILL_1000,                    // "Dexter"
    ACHIEVEMENT_KILL_10000,                   // "Mickey and Mallory"
    ACHIEVEMENT_COMPLETE_LEVEL_10,            // "Half-mast hard-on"
    ACHIEVEMENT_COMPLETE_GAME,                // "Get a girlfriend"
    ACHIEVEMENT_FIRE_1000000_BULLETS,         // "Bulletstorm"
    ACHIEVEMENT_HIT_100000_TARGETS,           // "Peter North would be proud"
    ACHIEVEMENT_TAKE_10000_HITS,              // "Holes is holes"
    ACHIEVEMENT_KILL_EVERYTHING,              // "Z for Zachariah"
    ACHIEVEMENT_KILL_ONLY_HOSTILES,           // "Boondock Saint"
    ACHIEVEMENT_USE_ONLY_M16,                 // "I swear, the AR is for hunting!"
    ACHIEVEMENT_USE_EVERY_WEAPON,             // "Army of One"
    ACHIEVEMENT_COMPLETE_LEVEL_ON_LOW_HEALTH, // "From my cold, dead hands""
    ACHIEVEMENT_FIGHT_AN_OSTRICH,             // "NOPE! Chuck Testa."
    ACHIEVEMENT_WATCH_ALL_CREDITS,            // "You want our autograph too?"
    ACHIEVEMENT_PLAY_ON_NON_WINDOWS_PLATFORM, // "Ballmer Baller"
    ACHIEVEMENT_FIREBOMB_THE_BAND,            // "The Hell's Fargo Wagon"
    ACHIEVEMENT_ROCKET_TO_THE_FACE,           // "Took a banger in the mouth"
    ACHIEVEMENT_KILL_A_NAKED_PERSON,          // "Never nude-ist"
    ACHIEVEMENT_ENABLE_CHEATS,                // "Sissy"
    ACHIEVEMENT_COMMIT_SUICIDE,               // "I'm afraid I just blue myself"
    ACHIEVEMENT_TOUCH_SOMEONE_WHILE_BURNING,  // "Bad touch!"
    ACHIEVEMENT_COMPLETE_GAME_IN_X_MINUTES,   // "Two Pump Chump"
    ACHIEVEMENT_COMPLETE_GAME_ON_HARDEST,     // "Solid as a rock"
    ACHIEVEMENT_MAX  // not an achievement, just the total count.
};

#if WITH_STEAMWORKS
extern void UnlockAchievement(const Achievement ach);
extern void RunSteamworksUpkeep();
extern void RequestSteamStatsStore();
#else
#define UnlockAchievement(x) do {} while (0)
#define RunSteamworksUpkeep() do {} while (0)
#endif

#endif // GAME_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
