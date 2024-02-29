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
// game.cpp
// Project: Postal
//
// This module deals with the high-level aspects of setting up and running the
// game.
//
// History:
//		11/19/96 MJR	Started.
//		01/15/96 MJR	Added GamePath()
//
//		01/28/97	JMI	Added calls to MenuTrans API (a PalTran).
//
//		01/30/97	MJR	Fixed bug in CorrectifyPath() that accidentally truncated
//							the passed-in string.
//
//		01/31/97	JMI	Now uses FullPath() to get full path for filespecs.
//
//		01/31/97	JMI	Added g_resmgrGame for specific to actual game data.
//
//		01/31/97	JMI	Number of title loops done on start up is now gotten from
//							INI file.
//
//		02/02/97	JMI	Now calls StartTitle() and EndTitle() after launching an
//							edit session, a user game, or a demo game.
//
//		02/02/97	JMI	Now, before exiting, makes sure all samples are done 
//							playing.
//
//		02/03/97	JMI	Changed DoMenu() called to new DoMenuInput() and 
//							DoMenuOutput() calls.
//
//		02/03/97	JMI	Now sets the base path for g_resmgrGame, Shell, & Menus.
//
//		02/04/97	JMI	Added functions to get and set the gamma level.
//
//		02/04/97	JMI	Changed LoadDib() call to Load() (which now supports
//							loading of DIBs).
//
//		02/13/97	JMI	Removed game level alpha XRay stuff (now setup in CHood).
//
//		02/18/97	JMI	Now opens SampleMaster's SAK file.
//
//		03/06/97	JMI	Made PalTranOn() and PalTranOff() extern instead of inline.
//
//		03/17/97	JMI	Made PalTranOff() abortable via a keypress.  Put in the
//							shape of the code for aborting via a key in PalTranOn() but
//							there is currently no interface to MenuTrans to allow this
//							type of abort (or jump).
//
//		03/28/97	JMI	Un#if 0'd the SavePrefs() call.
//
//		03/28/97	JMI	Now stores and resets the current dir.
//
//		04/11/97	JMI	Added Game_Menu_Demo() externally callable function to
//							start the demo.
//
//		04/14/97	JMI	Now specifies which image for StartTitle() to start with.
//
//		04/14/97	JMI	Missed a spot in the previous change.
//
//		04/16/97 MJR	Added simple test of file paths.  Also changed it so that
//							preferences are always saved, even if the game encounters
//							an error beforehand.
//
//							Fixed the simple test of file paths to actual use the
//							file path!!!
//
//		04/17/97 MJR	Put the test for the CD path back in.
//
//		04/22/97	JMI	No longer uses chdir().
//
//		04/22/97	JMI	Now sets g_GameSettings.m_sServer and 
//							g_GameSettings.m_sClient to 0 before running the demo.
//
//		04/24/97	JMI	Currently, aborting the PalTranOff() is a bad idea, so
//							we don't allow it (until we support EndMenuTrans(TRUE) ).
//
//		05/06/97	JMI	Now uses a Gamma multiplier to define a line instead of
//							the exponent defining the curve.
//
//		05/07/97	JMI	Now uses a Gamma exponent and uses the 'Gamma Val' as a
//							constant multiplier:
//							MappedColorVal	= (colorVal ^ GAMMA_EXPONENT) * GammaVal
//
//		05/21/97	JMI	Added a resource manager for resources that are not SAKed.
//
//		05/23/97	JMI	Added a CloseSaks() which closes and purges the SAKs.
//							This solves the problem with the ASSERTions about the logic
//							vars (the RResmgr's were declared at file scope as were the
//							CLogTabVars so it was just a matter of order of destruction
//							that dictated whether the ASSERTions occurred).
//
//		06/03/97	JMI	Commented out error message for read-only prefs file.
//
//		06/03/97	JMI	Now uses g_GameSettings members for demo timeouts instead
//							of the DEMO_TIMEOUT macro.
//
//		06/04/97	JMI	Added conditionally compiled message instead of editor
//							when editor is requested from the menu.
//							Also, now StartTitle() calls within GameLoop() request the
//							last title page.
//
//		06/04/97	JMI	Added AbortAllSamples() call when exitting.
//
//		06/04/97	JMI	Added MUST_BE_ON_CD, EDITOR_DISABLED, and CHECK_FOR_COOKIE 
//							conditional compilation macros and added a check for a 
//							specific U32 in the COOKIE file.
//
//		06/12/97 MJR	Reworked the callbacks so that the game-specific code now
//							resides in this module rather than the menu module.
//							Reworked the entire core loop so it passes all the
//							required parameters to Play() instead of using globals.
//							Moved all the demo and network setup stuff out of play.cpp
//							and into here.
//
//		06/15/97 MJR	More changes in terms of how Play() is called and what
//							this module does to get ready for calling it.
//							Also changed calls to rspDoSystem() into calls to Update().
//
//							Added required calls to DeleteInputDemoData().
//
//		06/15/97 MJR	Made use of new-and-improved input demo interface.
//							(But demo mode is still not working right...had to check
//							it in though to go along with changes in other files).
//
//		06/16/97 MJR	Fixed stupid error that caused demo record not to work.
//
//		06/16/97	JMI	Now passes destination buffer in StartMenu() call.
//
//		06/16/97	JMI	Added g_fontSmall.
//
//					MJR	Added make-a-demo-movie stuff for debugging demo mode
//							problems.
//
//					JMI	Now, based on INI setting g_GameSettings.m_sTrickySystemQuit,
//							we en/disable system quit status flags.
//
//					JMI	Changed "res/fonts/SmallSmash.fnt" to "res/fonts/SmSmash.fnt"
//
//		06/17/97 MJR	Modified CorrectifyBasePath() so it will change any
//							relative paths into absolute paths.  This, in turn, fixed
//							a problem whereby the loading of demo files failed because
//							it assumed the name of the demo file would begin with the
//							HD path.  This assumption normally worked, but failed when
//							the HD path was a relative path.
//
//		06/18/97 MJR	Added SeedRandom() and GetRandom().
//
//					MJR	Fixed bug in CorrectifyBasePath().  Now handles empty
//							paths (it ignores them).
//
//		06/24/97	JMI	Added SynchLog() debug mechanism.  See function comment for
//							functionality details.  See macros in game.h for usage via
//							macro(s).
//
//		06/24/97	JMI	Undefined CHECK_FOR_COOKIE.
//
//					JMI	Changed usage of strcmp to rspStricmp in GetRandom() and
//							SynchLog().
//
//		06/30/97 MJR	Changed client and server objects to use new/delete instead
//							of having them live on the stack.  They are very large,
//							and caused local (stack) data to exceed 32k on the mac.
//
//		07/03/97	JMI	Converted calls to rspOpen/SaveBox() to new parm 
//							conventions.
//
//		07/03/97	JMI	Added Game_ControlsMenu() and actions for editting input
//							settings for keys and mouse.
//
//		07/05/97 MJR	Changed to RSP_BLACK_INDEX instead of 0.
//
//		07/06/97	JMI	Changed ACTION_EDIT_MOUSE_SETTINGS and 
//							ACTION_EDIT_KEY_SETTINGS to ACTION_EDIT_INPUT_SETTINGS.
//							Also, now only one function is called no matter which type
//							of input settings are to be editted, EditInputSettings().
//
//		07/11/97 BRH	Finished up the expiration date checking for the 
//							Registry.  Still need to add something for the Mac
//							version.
//
//		07/11/97 BRH	Fixed typos causing the latest date not to be written to
//							the registry.
//
//		07/13/97	JMI	Now sets the appropriate base path or opens the approriate
//							SAK dependent on the current audio mode.
//
//		07/13/97	JMI	Added Game_StartChallengeGame() and an action for the
//							various challenge game types.
//
//		07/14/97 BRH	Changed calls to Play() to pass the challenge mode flag.
//							Fixed value in Get Registry in case of an error on
//							decrypt.
//
//		07/16/97 BRH	Made changes to the Mac expiration date code to normalize
//							the time to 1970 like the rest of the machines so that
//							the dates can be stored in 1 format.
//
//		07/16/97	JMI	Now uses real time when pausing on the titel screens (in-
//							stead of 'Load Loops').
//
//		07/17/97 BRH	Finished debugging the Mac Prefs file expiration date code.
//
//					MJR	Fixed sample rate problem on the mac.
//
//					BRH	Put in the real expiration date and a comment for the
//							real release date, and updated the current release date
//							to today so that we can still test it.
//
//		07/18/97 BRH	Added game load and save functions so that the player's
//							game can be saved and loaded.  Also added a global
//							stockpile object used to transfer loaded/saved info to/from
//							the CDude's stockpile.
//
//		07/19/97 MJR	Fixed bug where demo mode would start if user cancelled out
//							of a dialog after spending a long time there.
//							Also changed so demo mode will not start if app is in BG.
//
//		07/23/97 BRH	Changed calls to Title functions to use the new array of
//							title screen durations.	 
//
//		07/23/97 BRH	Changed expiration date display for PC back to asctime(gmtime())
//							since ctime didn't work correctly.
//
//		07/26/97	JMI	Added g_fontPostal to replace g_fontSmall.  Got rid of
//							g_fontSmall.
//
//		07/28/97	JMI	Now displays a different message for non-marketing 
//							releases.
//
//		08/01/97 BRH	Cycles through 3 demos rather than just 1. Also added
//							open dialog for playing demos so that the user can choose
//							which demo to play.
//
//		08/02/97	JMI	Now initializes the default RGuiItem font to g_fontBig in-
//							stead of relying on everybody who uses it to invidually set
//							it.
//
//		08/05/97	JMI	Added Game_AudioOptionsChoice().
//
//		08/06/97 MJR	Added socket startup/shutdown.
//
//		08/08/97 MJR	Background and foreground callbacks are now installed here
//							so they are in effect for the entire app.
//
//		08/08/97	JMI	Now passes parameter to start musak in title to 
//							StartTitle().
//
//		08/12/97	JMI	m_szDemoFile was FullPath()'ed twice so the demo wouldn't
//							load.  Fixed.
//
//		08/12/97	JMI	Added SubPathOpenBox() (see proto for details) and
//							FullPathCustom().
//
//		08/13/97 MJR	We now pass difficulty level to Play().
//
//		08/13/97	JMI	No longer remembers the previous demo file you recorded as
//							a patch to either improper usage or problem in 
//							SubPathOpenBox().
//
//		08/14/97	JMI	Added prealm parameter to Game_Save/LoadPlayersGame() so
//							they can query/modify realm settings/flags.
//							Also, changed questionable access on m_action in 
//							Game_SavePlayersGame() and Game_LoadPlayersGame() which 
//							were casting &m_action to a (short*) even though ACTION is
//							a 32 bit value.  May not have worked on the Mac although it
//							would for values less than 32767 and greater than 0 on the 
//							little endian PC.
//							Also, added an ACTION_LOAD_GAME so that could be part of
//							the main loop with the intent of making the loaded 
//							difficulty a local variable but this turned out to be error
//							prone with keeping this value up to date with the proper
//							value (either the INI difficulty or the loaded difficulty)
//							and it was annoying to determine which one to use on each
//							Play() call so I made a function that returns the 
//							appropriate value (but only once...ick).  If you feel like
//							it try it the other way, but be aware that on any iteration
//							g_GameSettings.m_sDifficulty may change via a menu.
//							Also, had to add a flag so one action can lead to another.
//							The main loop used to reset the action after every action
//							making it impossible for one action to lead to another.
//
//		08/15/97	JRD	Added a function to control brightness/contrast in place
//							of gamma.  This was because the gamma function could not
//							recreate a "normal palette."
//
//		08/15/97 JRD	Attempted to hook both brightness and contrast changes to
//							the old gamma slider with a crude algorithm.
//
//		08/18/97	JMI	Now allows ALT-F4 to drop you out of the title sequence.
//
//		08/18/97	JMI	In both debug and release modes, we were setting the do
//							system mode to sleep.  But we'd only restore it in release
//							mode which basically left us in sleep mode in debug mode.
//							Now restores it to TOLERATEOS in debug mode.
//
//		08/19/97 BRH	Uses the game settings m_sNumAvailableDemos to control the
//							demo loop rather than the set macro NUM_DEMO_FILES
//
//					MJR	Added support for new MP flags.
//
//		08/20/97	JMI	Modified Game_LoadPlayersGame() to receive the ptr to the
//							action to fill with the Player's saved action so the 
//							function would not have to use the global m_action.
//							Also, Moved Game_LoadPlayersGame() proto into game.cpp b/c
//							it now takes an ACTION* but ACTION is defined in game.cpp
//							and since no one currently calls this function externally,
//							what the hell.         
//
//		08/21/97 BRH	Changed the game sak to be loaded from the GAME_PATH_GAME
//							directory, getting it ready for the installation process.
//							Also changed the demos to HD path to make it easier for
//							people to load their own demos.  Changed the sound res
//							manager path to the GAME_PATH_SOUND for installation.                                       
//
//		08/21/97	JMI	Changed call to Update() to UpdateSystem() and occurrences
//							of rspUpdateDisplay() to UpdateDisplay().
//
//		08/22/97	JMI	Changed occurrences of UpdateDisplay() back to 
//							rspUpdateDisplay().  Now that we are using the lock 
//							functions correctly, we don't need them.
//							Also, removed rspLockBuffer() and rspUnlockBuffer() that
//							were used to encapsulate the entire app in a lock.
//
//		08/22/97 BRH	Changed the cookie position for use with a new smaller
//							res.sak file since we now have Direct X and more sound, 
//							we needed more space on the CD.
//
//		08/23/97	JMI	Now gets callbacks from the initialization and killing of
//							the main menu and stops any playing title/main-menu musak
//							when the main menu is killed.
//
//		08/24/97	JMI	Added a timeout to the abortion of playing samples just in
//							case there's a bug or a sound driver problem (no need to
//							to taunt infinite loopage).
//
//		08/25/97	JMI	Now sets the sound quality based on the current mode.
//							Also, now, if there's no sak dir and no sak for the current
//							audio mode, complains.
//
//		08/26/97 BRH	Merged in changes from a branched game.cpp which has the
//							special cases for the ending demo level of the game.  
//							Added a function GameEndingSequence which is called
//							when the global flag that indicates the player has won is
//							set.  It plays the demo file and then calls the 
//							Title_EndingSequence.
//
//		08/26/97	JMI	Now any menu can be restore in the GameCore() loop instead
//							of just the main menu by setting pmenuStart to point at the
//							menu to be next started.
//
//		08/30/97 BRH	Fixed the base path for the logic table resource manager
//							to load from the CD rather than the HD path.
//
//		09/02/97	JMI	Added a new option to the demo actions which plays one of
//							the factory preset demos.
//							Also, in the case that there were no factory preset demos
//							and the auto play demo kicked in, it would ASSERT that
//							the filename was "".
//
//		09/02/97	JMI	Now more deluxe control for regarding the starting of the
//							next menu after an action in GameCore().
//
//		09/03/97 BRH	Changed the g_resmgrRes logic table manager to load
//							from the HD path rather than the CD path, so that it
//							is easier to modify, add or change logics, especially
//							if we have to patch them in the future.
//
//		09/04/97	JMI	Now Game_ControlsMenu() only sets a new action if one is
//							not already set.  This keeps the controls menu from setting
//							the action when it is launched from Play.cpp instead of 
//							here.
//
//		09/05/97 PPL	In CorrectifyBasePath, made sure we allocate the buffer for the
//							full path instead of letting getcwd allocate it for us.  It's
//							not that we won't let it -- it's just that the Mac implementation
//							will not do it automatically as the PC implementation would.
//
//		09/05/97 PPL	Per Bill's directions, we will use the path from the CD for
//							the challenge level.
//
//		09/05/97 MJR	A few changes to use the new network stuff.
//
//		09/06/97 MJR	Now inits and kills the network problem GUI.
//
//		09/10/97 BRH	Had to change the res.sak in order to fit the MPlayer
//							and Heat setups on the CD, so I also had to change the
//							cookie.
//
//		09/10/97 MJR	Now properly converts paths from system to rspix format
//							on the start single player and start challenge level
//							dialogs.  This bug only showed up on the Mac.
//
//		09/11/97 BRH	Added different cookie and file position for 
//							COMP_USA_VERSION only.
//
//		09/11/97	JMI	Removed old code for PLAY_SPECIFIC_LEVEL_ONLY macro which
//							no longer exists.
//
//		09/12/97 MJR	Now more code doesn't compile when the editor is disabled,
//							in an attempt to make it harder to hack.
//
//		09/18/97	JMI	Added expiration message for
//							EXPIRATION_MSG_POSTAL_LAUNCH_WEEKEND compile condition.
//
//					JMI	Changed 'will expire on' to 'expires' so it sounds better
//							both past/future/etc tense wise and the 'on' didn't work
//							well with the format "Mon Sep 22 23:59:00" which contains
//							no 'at'.
//
//		09/24/97 BRH	For LOCALE != US version, took out the school yard demo
//							from the ending sequence.
//
//		09/29/97	JMI	Now will try to load any samples SAK in the case that there
//							is no audio mode, the specified mode's samples SAK is not
//							present, and there's no 'NoSak' dir specification.
//							If absolutely no samples SAK is found, a message is 
//							displayed that does not mention whether or not your 
//							hardware supports the specified mode.
//
//		10/01/97 BRH	Changed res.sak location so it would fit on the disk.
//
//		10/08/97	JMI	Made the synchronization logs opened and closed via one
//							set of two functions to make it easier to use them in
//							different scenarios.
//							Also, the logs should now only use the filename (excluding
//							the path) to make it possible for Mac vs. PC logs to be
//							compared.
//
//		10/10/97	JMI	Updated Game_ControlsMenu() to handle joystick option
//							when ALLOW_JOYSTICK is defined.
//
//		10/14/97	JMI	Made SynchLog()'s expr parameter a double instead of an int
//							for more accuracy.
//
//		10/15/97	JMI	Changed the %g used to fprintf() SynchLog() expr values
//							to %0.8f b/c the Mac and PC formats differed slightly.  The
//							Mac one often contains as many as four zeroes after the 
//							last significant decimal, sometimes less, and most of the
//							time none.  The PC never seems to put trailing zeroes.
//
//		10/16/97	JMI	Now mac demo basename is mdefault.dmo (instead of 
//							default.dmo) since the game plays slightly differently on
//							the Mac than on the PC.
//
//		10/22/97	JMI	In the call to Play() for recording demos, FullPathHD() was
//							being called on the passed in path but now that 
//							CRealm::Open() tries all the deluxe path stuff we don't
//							need that (and cannot use it b/c Open() does a conversion
//							to system path format as does FullPathHD() ).
//
//		11/20/97	JMI	Added cooperative flag to Play() calls.
//
//		12/01/97 BRH	Added flags for Add On levels to play calls, and
//							added ACTION_PLAY_ADDON as one of the options. 
//
//		12/04/97 BRH	Added Single player menu option to play add on levels
//							or original levels.
//
//		01/05/98	JMI	Now uses RMix::GetMode() to get sound quality.  Also, now
//							uses the source bits per sample (instead of the device
//							setting) to determine which sample resources to use.
//							Note that the quality is still set based on the device bits
//							per sample though so that the volumes could be tuned 
//							accordingly in SampleMaster's ms_asQualityCategoryAdjustors.
//
//		01/21/98	JMI	Now the foreground and background callbacks use a value,
//							INVALID_CURSOR_SHOW_LEVEL, to flag whether or not they're
//							in the background so they know which callbacks to ignore
//							and which one's to use.  I think that before (under DirectX)
//							we were getting more than one Backcall per Forecall and/or
//							vice-versa.
//
//		03/05/98 BRH	Now attempts to identify the PostalSD disc when the game
//							starts to clear up any confusion about which CD must be
//							in the drive to play the Add on Pack.  The Add on Pack
//							requires the original PostalCD in the drive, and it reads
//							the original res.sak file from that disc.  The PostalSD
//							disc also has a res.sak file on it, but its not the one
//							we want, and if someone tried to play the game with the
//							PostalSD disc in the drive rather than the PostalCD, then
//							the copy protection would flag it as invalid.  So an
//							additional check is now made and if the PostalSD disc
//							is in the drive, it will display a message to tell them
//							to put the PostalCD in the drive.  
//
//		06/04/98 BRH	Removed the demo timeout check from the SPAWN build since
//							the spawn version doesn't have any demo files available
//							it shouldn't try to run them when the user is idle.  It
//							was attempting to do so, and when it can't find a demo
//							file, a message box pops up which would cause much 
//							confusion to the user.
//
//		07/08/98	JMI	Removed extra Close() in checking for add on CD.  Also,
//							moved close that was in incorrect spot in check for add on
//							CD.  These were only annoying in Debug mode when it is
//							flagged.
//
//		09/27/99	JMI	Changed to allow ending sequence only in any locale 
//							satisfying the CompilerOptions macro VIOLENT_LOCALE.
//
//		10/07/99	JMI	Conditional remove Add On start menu option when in 
//							SUPER_POSTAL target.
//
//		10/11/99	JMI	Changed the audio sak name to be RATEjBITS instead of 
//							RATE_BITS to differentiate it from Orig & SD Postal sound
//							saks.
//
//		02/04/00 MJR	Added dialog box to prompt for original CD if 
//							PROMPT_FOR_ORIGINAL_CD is defined.
//
//					MJR	Changed so that it now checks the HD first and the VD
//							second when trying to load the shell sak.  This
//							was done to fix problems with the Japan Add On.
//
//		03/30/00 MJR	Changed to new macro for controlling the test for the
//							original Postal CD to differentiate it from the test for
//							a CD-ROM drive.
//
//							Added AUDIO_SAK_SEPARATOR_CHAR to make it easy to control
//							the separater character in audio sak filenames.
//
//		04/01/00 MJR	Fixed bug with AUDIO_SAK_SEPARATOR_CHAR.
//							Changed so in debug mode, it continues even if not on CD.
//
//		04/03/00 MJR	Modified MUST_BE_ON_CD code so that the test is bypassed if
//							the CD path is to our development server.  This allows
//							for easier testing.
//
//		06/24/01 MJR	Got rid of CompUSA cookie variation.  Also cleaned up a
//							few other macros.
//
////////////////////////////////////////////////////////////////////////////////

#define DWORD U32

#include "RSPiX.h"

#include <time.h>
#ifdef WIN32
	#include <direct.h>
#else
	#include <unistd.h>
#endif

#include "WishPiX/Menu/menu.h"
#include "WishPiX/Prefs/prefs.h"
#include "WishPiX/ResourceManager/resmgr.h"

#include "main.h"
#include "menus.h"
#include "game.h"
#include "localize.h"
#include "play.h"
#include "title.h"
#include "credits.h"
#include "scene.h"
#include "update.h"
#include "gameedit.h"
#include "MenuTrans.h"
#include "SampleMaster.h"
#include "net.h"
#include "NetDlg.h"
#include "input.h"
#include "InputSettingsDlg.h"
#include "encrypt.h"
#include "credits.h"

#include "CompileOptions.h"

////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////

// Total units required for loading during title sequence (estimated)
#define TITLE_LOAD_UNITS				5000

// TEMPORARY!  Determines how long the fake title sequence load will take.
// Remove this when there's real stuff being loaded.
#define TEMP_TITLE_LOOPS				500
#define TEMP_TITLE_TIME_PER_LOOP		10		// 100 looks normal, 10 goes extremely quickly

// Font stuff
#define BIG_FONT_FILE					"res/fonts/system1.fnt"
#define POSTAL_FONT_FILE				"res/fonts/smash.fnt"
#define FONT_HEIGHT						12						// "Best" for ComicB.
#define FONT_FORE_INDEX					255					// White or black.
#define FONT_BACK_INDEX					0						// Transparent.

#define NORMAL_PAL_TRAN_TIME			750

#define TEMP_ALPHA_EFFECT				"2dEffects/alpha00.bmp"

#define SHELL_SAK_FILENAME				"res/shell/shell.sak"
#define GAME_SAK_FILENAME				"res/game/game.sak"
#define SAMPLES_SAK_SUBDIR				"res/game/"
#define CUTSCENE_SAK_FILENAME_FRMT	"res/cutscene/cutscene%02d.sak"

// Since the game plays slightly differently on the PC than on the Mac,
// we'll have two different sets of demos.
#define DEFAULT_DEMO_PREFIX			"res/demos/default"
#define DEFAULT_DEMO_SUFFIX			".dmo"
#define DEMO_OPEN_TITLE					"Choose a Demo File to Play Back"
#define DEMO_LEVEL_DIR					"res/demos/."
#define ENDING_DEMO_NAME				"res/levels/single/index.rdx"	// Fake name for school.dmo

#define DEMO_DIR							"res/demos/."
#define DEMO_EXT							".dmo"

#define LEVEL_DIR							"res/levels/."

#if WITH_STEAMWORKS
extern bool EnableSteamCloud;
#define SAVEGAME_DIR						(EnableSteamCloud ? "steamcloud" : "savegame")
#else
#define SAVEGAME_DIR						("savegame")
#endif

#define CHECK_FOR_ASSETS_FILENAME	"res/res.sak"
#define CHECK_FOR_POSTALSD_FILENAME "res/hoods/ezmart.sak"
#define COOKIE_VALUE						0x9504cc39 //0xb5cf76dd
#define COOKIE_FILE_POSITION			289546856 //323290320  
#define COOKIE_XOR_MASK					0x6afb39e5 //0x66666666

// Exponent used to define gamma curve.
#define GAMMA_EXPONENT					1.50

// The directories for each type of challenge levels.
#define TIMED_CHALLENGE_LEVEL_DIR		"res/levels/gauntlet/timed/."
#define CHECKPOINT_CHALLENGE_LEVEL_DIR	"res/levels/gauntlet/checkpt/."
#define GOAL_CHALLENGE_LEVEL_DIR			"res/levels/gauntlet/goal/."
#define FLAG_CHALLENGE_LEVEL_DIR			"res/levels/gauntlet/capflag/."

// The titles for the open dialog for each type of challenge levels.
#ifdef MOBILE
#define TIMED_CHALLENGE_OPEN_TITLE			"Timed Challenge"
#define CHECKPOINT_CHALLENGE_OPEN_TITLE	    "Checkpoint Challenge"
#define GOAL_CHALLENGE_OPEN_TITLE			"Goal Challenge"
#define FLAG_CHALLENGE_OPEN_TITLE			"Capture the Flag Challenge"
#else
#define TIMED_CHALLENGE_OPEN_TITLE			"Choose Timed Challenge"
#define CHECKPOINT_CHALLENGE_OPEN_TITLE	"Choose Checkpoint Challenge"
#define GOAL_CHALLENGE_OPEN_TITLE			"Choose Goal Challenge"
#define FLAG_CHALLENGE_OPEN_TITLE			"Choose Capture the Flag Challenge"
#endif

#define INVALID_DIFFICULTY						-7

#define TIME_OUT_FOR_ABORT_SOUNDS		3000	// In ms.

// Determines the number of elements in the passed array at compile time.
#define NUM_ELEMENTS(a)					(sizeof(a) / sizeof(a[0]) )

// We use this value to flag the fact that we haven't hidden the cursor.
#define INVALID_CURSOR_SHOW_LEVEL	( (short)0x8000)


// Various action types
typedef enum
	{
	ACTION_NOTHING,
	ACTION_PLAY_SINGLE,
	ACTION_PLAY_BROWSE,
	ACTION_PLAY_HOST,
	ACTION_PLAY_CONNECT,
	ACTION_PLAY_CHALLENGE,
	ACTION_EDITOR,
	ACTION_DEMO_PLAYBACK,
	ACTION_DEMO_RECORD,
	ACTION_EDIT_INPUT_SETTINGS,
	ACTION_POSTAL_ORGAN,
	ACTION_LOAD_GAME,
	ACTION_PLAY_ADDON
#ifdef MOBILE
	,ACTION_CONTINUE_GAME
#endif
	} ACTION;


////////////////////////////////////////////////////////////////////////////////
// Variables/data
////////////////////////////////////////////////////////////////////////////////

// Global game settings
CGameSettings g_GameSettings;

// Cookie flag
long g_lCookieMonster;

// Global screen buffer
RImage* g_pimScreenBuf;

// Global big font
RFont g_fontBig;

// Global Postal font.
RFont g_fontPostal;

// Resource manager for game resources.  These are resources used by the actual
// game, like things that a CThing loads that is not level specific.  For example,
// CBall loads, of course, foot.bmp, which would be loaded through this ResMgr.
// Note:  This resmgr should not be used for things like info on ordering, menu
// resources, g_fontBig, or anything not specific to the real game.
// Note:  Realm specifc data, such as alpha effects, etc., should be loaded
// through prealm->m_resmgr.
RResMgr	g_resmgrGame;

// Resource manager for shell resources.  Do not use this to load things like
// the main dudes' sprites.
RResMgr	g_resmgrShell;

// Resource manager for non-SAK resources.
RResMgr	g_resmgrRes;

// Time and Date values
long g_lRegTime;
long g_lRegValue;
long g_lExpTime;
long g_lExpValue;
long g_lReleaseTime;

// Stockpile used to transfer loaded/saved data to/from the CDude's stockpile
CStockPile g_stockpile;
bool		g_bTransferStockpile;
short		g_sRealmNumToSave;

// Flag for special end of game demo sequence which is set in Play() when
// it has been determined that the player has won.
bool		g_bLastLevelDemo = false;

// The secret cookie value used to determine if the humongous file exists
static U32	ms_u32Cookie = COOKIE_VALUE;

// These variables are generally controlled via the menu system
static ACTION m_action;
static long m_lDemoBaseTime;
static long m_lDemoTimeOut;
static char	m_szRealmFile[RSP_MAX_PATH+1];
static char m_szDemoFile[RSP_MAX_PATH+1];
static short m_sRealmNum;
static bool m_bJustOneRealm;

// Cursor show level before we went to the background
static short ms_sForegroundCursorShowLevel	= INVALID_CURSOR_SHOW_LEVEL;

// Used by random number stuff
static long m_lRandom = 1;
static RFile* m_pfileRandom = 0;

// Used by if-logging schtuff.
static long	ms_lSynchLogSeq	= 0;

static RFile	ms_fileSynchLog;

// true to create synchronization logs, false to compare to them.
static bool		m_bWriteLogs	= false;

static short	ms_sLoadedDifficulty	= INVALID_DIFFICULTY;

static SampleMaster::SoundInstance	ms_siMusak	= 0;

////////////////////////////////////////////////////////////////////////////////
// Function prototypes
////////////////////////////////////////////////////////////////////////////////

static short GameCore(void);			// Returns 0 on success.

static void ResetDemoTimer(void);

static short OpenSaks(void);			// Returns 0 on success.

static void CloseSaks(void);

static short LoadAssets(void);

static short UnloadAssets(void);

static void GameSetRegistry(void);

static void GameGetRegistry(void);

static void GameEndingSequence(void);

static short GetRealmToRecord(
	char* pszRealmFile,
	short sMaxFileLen);

static short GetDemoFile(
	char* pszDemoFile,
	short sMaxFileLen);

// Callback gets called when OS is about to switch app into the background
static void BackgroundCall(void);

// Callback gets called when OS is about to switch app into the foreground
static void ForegroundCall(void);

// Returns difficulty for games.
// Note that this function is only valid once after a difficulty adjustment
// and then it goes back to the default (g_GameSettings value).
static short GetGameDifficulty(void);	// Returns cached game difficulty.

// Opens the synchronization log with the specified access flags if in a 
// TRACENASSERT mode and synchronization logging is enabled.
// Also, opens the random log, if it is enabled via 
// g_GameSettings.m_szDebugMovie or something.
static void OpenSynchLogs(			// Returns nothing.
	bool	bWriteLogs);				// In:  true to create log, false to compare.

// Closes the synchronization logs, if open.
static void CloseSynchLogs(void);	// Returns nothing.

////////////////////////////////////////////////////////////////////////////////
//
// Load a previously saved game
//
////////////////////////////////////////////////////////////////////////////////

extern short Game_LoadPlayersGame(	// Returns SUCCESS if loaded saved game file
				char* pszSaveName,		// In:  Name of the saved game file to open
				short* psDifficulty,		// Out: Saved game realm difficulty.
				ACTION* paction);			// Out: Saved game action.

bool StatsAreAllowed = false;

int Stat_BulletsFired = 0;
int Stat_BulletsHit = 0;
int Stat_BulletsMissed = 0;
int Stat_Deaths = 0;
int Stat_Suicides = 0;
int Stat_Executions = 0;
int Stat_HitsTaken = 0;
int Stat_DamageTaken = 0;
int Stat_Burns = 0;
int Stat_TimeRunning = 0;
int Stat_KilledHostiles = 0;
int Stat_KilledCivilians = 0;
int Stat_TotalKilled = 0;
int Stat_LevelsPlayed = 0;

ULONG Flag_Achievements = 0;

#if 1 //PLATFORM_UNIX
#include <sys/stat.h>
static void EnumExistingSaveGames(Menu *menu)
{
    char gamename[RSP_MAX_PATH];
    int i = 0;
    int Max = (sizeof(menu->ami) / sizeof(menu->ami[0])) - 1;
    if (Max > MAX_SAVE_SLOTS)
        Max = MAX_SAVE_SLOTS;

#if MOBILE
    snprintf(gamename, sizeof (gamename), "%s/auto.gme", SAVEGAME_DIR);

    const char *fname = FindCorrectFile(gamename, "w");
    struct stat statbuf;

    const char *str = "unused";
    char timebuf[32];
    menu->ami[0].sEnabled = (stat(fname, &statbuf) != -1);
    if (menu->ami[0].sEnabled)
    {
    	struct tm *tm;
    	if ((tm = localtime((const time_t*)&statbuf.st_mtime)) == NULL)
    		str = "unknown";
    	else
    	{
    		strftime(timebuf, sizeof (timebuf), "%m/%d/%y %H:%M", tm);
    		str = timebuf;
    	}
    }

	snprintf(gamename, sizeof (gamename), "Auto - [%s]", str);
    menu->ami[0].pszText = strdup(gamename);

    for (i = 0; i < Max; i++)
    {
    	snprintf(gamename, sizeof (gamename), "%s/%d.gme", SAVEGAME_DIR, i);

        const char *fname = FindCorrectFile(gamename, "w");
        struct stat statbuf;

        const char *str = "unused";
        char timebuf[32];
        menu->ami[i+1].sEnabled = (stat(fname, &statbuf) != -1);

        if (menu->ami[i+1].sEnabled)
        {
            struct tm *tm;
            if ((tm = localtime((const time_t*)&statbuf.st_mtime)) == NULL)
                str = "unknown";
            else
            {
                strftime(timebuf, sizeof (timebuf), "%m/%d/%y %H:%M", tm);
                str = timebuf;
            }
        }

        snprintf(gamename, sizeof (gamename), "%d - [%s]", i, str);
        menu->ami[i+1].pszText = strdup(gamename);

    }
#else
	for (i = 0; i < Max; i++)
	{
		snprintf(gamename, sizeof (gamename), "%s/%d.gme", SAVEGAME_DIR, i);
		const char *fname = FindCorrectFile(gamename, "w");
		struct stat statbuf;

		const char *str = "unused";
		char timebuf[32];
		menu->ami[i].sEnabled = (stat(fname, &statbuf) != -1);
		if (menu->ami[i].sEnabled)
		{
			struct tm *tm;
			if ((tm = localtime(&statbuf.st_mtime)) == NULL)
				str = "unknown";
			else
			{
				strftime(timebuf, sizeof (timebuf), "%m/%d/%y %H:%M", tm);
				str = timebuf;
			}
		}
		snprintf(gamename, sizeof (gamename), "%s/%d.gme [%s]", SAVEGAME_DIR, i, str);

		menu->ami[i].pszText = strdup(gamename);
	}

#endif
}
#endif

////////////////////////////////////////////////////////////////////////////////
//
// Do the high-level startup stuff, run the game, and then cleanup afterwards.
//
// It is assumed that the system/RSPiX environment are setup properly before
// this function is called.
//
////////////////////////////////////////////////////////////////////////////////
extern void TheGame(void)
	{
	short sResult = 0;

	// Set up callbacks for when OS sends us to foreground or background.
	rspSetBackgroundCallback(BackgroundCall);
	rspSetForegroundCallback(ForegroundCall);

	ms_u32Cookie	= ~ms_u32Cookie;
	g_lRegValue = 0;
	g_lExpValue = 0;
	g_lRegTime = 0;

#ifdef CHECK_EXPIRATION_DATE
	g_lExpTime = EXPIRATION_DATE;
#else
	g_lExpTime = SAFE_DATE;
#endif
	g_lReleaseTime = RELEASE_DATE;

// To make it more confusing for someone debugging through the code, 
// the flag for the cookie check will be this large number SAFE_DATE.
// Once the game determines elsewhere that the cookie is correct, it will
// in some way modify the flag so that it is not SAFE_DATE, then later
// the check will be for anything other than SAFE_DATE
#ifdef CHECK_FOR_COOKIE
	g_lCookieMonster = SAFE_DATE;
#else
	g_lCookieMonster = SAFE_DATE - RELEASE_DATE;
#endif

	g_bTransferStockpile = false;

	// Get pointer to screen buffer and lock it.  Our Update()
	// unlocks and re-locks these each time it is called.
	rspNameBuffers(&g_pimScreenBuf);

	// Lock the RSPiX composite buffer so we can rect it.
	rspLockBuffer();
	// Clear screen buffer
	rspRect(RSP_BLACK_INDEX, g_pimScreenBuf, 
		0, 0, g_pimScreenBuf->m_sWidth, g_pimScreenBuf->m_sHeight);
	// Unlock now that we're done with the composite buffer.
	rspUnlockBuffer();

	// Update display
	rspUpdateDisplay();

	// Read all settings from preference file
	sResult = CSettings::LoadPrefs(g_pszPrefFileName);
	RFile file;

	if (sResult == 0)
		{
#ifdef PROMPT_FOR_ORIGINAL_CD
		rspMsgBox(RSP_MB_ICN_INFO | RSP_MB_BUT_OK, g_pszAppName, g_pszPromptForOriginalCD);
#endif

#ifdef REQUIRE_POSTAL_CD
		// Check to make sure the correct CD is in the drive.  The Postal Add On Requires
		// that the original PostalCD is in the drive.
		short	sCorrectCD = -2;

		while (sCorrectCD == -2)
			{
			// We are looking for a disc that has a res.sak, but doesn't have ezmart.sak
			// to insure that they have the original PostalCD in the drive
			if (file.Open(FullPathCD(CHECK_FOR_ASSETS_FILENAME), "r", RFile::LittleEndian) == 0)
				{
				file.Close();
				sCorrectCD++;
				if (file.Open(FullPathCD(CHECK_FOR_POSTALSD_FILENAME), "r", RFile::LittleEndian) != 0)
					{
					sCorrectCD++;
					}
				else
					{
					file.Close();
					}
				}

			if (sCorrectCD != 0)
				{
				TRACE("Game():  Wrong CD in the drive - this is not the original PostalCD\n");
				if (rspMsgBox(RSP_MB_ICN_INFO | RSP_MB_BUT_RETRYCANCEL, g_pszCriticalErrorTitle, g_pszWrongCD, "Wrong CD") == 3)
					sCorrectCD = -2;
				else
					sCorrectCD = -3;
				}	
			}
		// If we have the correct CD in the drive, then we can proceed
		sResult = sCorrectCD;

#endif // REQUIRE_POSTAL_CD
		
		// Try loading special file that exists solely for the purpose of checking for
		// valid paths in the prefs file.  We do this for each of the paths.
		if (sResult == 0)
			{
			if (file.Open(FullPathHD(CHECK_FOR_ASSETS_FILENAME), "r", RFile::LittleEndian) == 0)
				file.Close();
			else
				{
				sResult = -1;
				TRACE("Game(): Can't find assets based on HD path specified in prefs!\n");
				rspMsgBox(RSP_MB_ICN_STOP | RSP_MB_BUT_OK, g_pszCriticalErrorTitle, g_pszCantFindAssets, "HD");
				}
			}
		if (sResult == 0)
			{
			if (file.Open(FullPathVD(CHECK_FOR_ASSETS_FILENAME), "r", RFile::LittleEndian) == 0)
				file.Close();
			else
				{
				sResult = -1;
				TRACE("Game(): Can't find assets based on VD path specified in prefs!\n");
				rspMsgBox(RSP_MB_ICN_STOP | RSP_MB_BUT_OK, g_pszCriticalErrorTitle, g_pszCantFindAssets, "VD");
				}
			}
		if (sResult == 0)
			{
			if (file.Open(FullPathSound(CHECK_FOR_ASSETS_FILENAME), "r", RFile::LittleEndian) == 0)
				file.Close();
			else
				{
				sResult = -1;
				TRACE("Game(): Can't find assets based on Sound path specified in prefs!\n");
				rspMsgBox(RSP_MB_ICN_STOP | RSP_MB_BUT_OK, g_pszCriticalErrorTitle, g_pszCantFindAssets, "Sound");
				}
			}
		if (sResult == 0)
			{
			if (file.Open(FullPathGame(CHECK_FOR_ASSETS_FILENAME), "r", RFile::LittleEndian) == 0)
				file.Close();
			else
				{
				sResult = -1;
				TRACE("Game(): Can't find assets based on Game path specified in prefs!\n");
				rspMsgBox(RSP_MB_ICN_STOP | RSP_MB_BUT_OK, g_pszCriticalErrorTitle, g_pszCantFindAssets, "Game");
				}
			}
		if (sResult == 0)
			{
			if (file.Open(FullPathHoods(CHECK_FOR_ASSETS_FILENAME), "r", RFile::LittleEndian) == 0)
				file.Close();
			else
				{
				sResult = -1;
				TRACE("Game(): Can't find assets based on Hoods path specified in prefs!\n");
				rspMsgBox(RSP_MB_ICN_STOP | RSP_MB_BUT_OK, g_pszCriticalErrorTitle, g_pszCantFindAssets, "Hoods");
				}
			}


		// Check for special file, COOKIE, and size.
		if (sResult == 0)
			{
			ms_u32Cookie	^= COOKIE_XOR_MASK; 

			if (file.Open(FullPathCD(CHECK_FOR_ASSETS_FILENAME), "rb", RFile::LittleEndian) == 0)
				{
#if defined(CHECK_FOR_COOKIE)
				if (file.Seek(COOKIE_FILE_POSITION, SEEK_SET) == 0)
					{
					U32	u32Cookie	= 0;
					if (file.Read(&u32Cookie) == 1)
						{
						if (u32Cookie == ms_u32Cookie)
							{
							// Okay, you can play.
							// This can be any number, the check will verify that the
							// g_lCookieMonster is not SAFE_DATE.  
							g_lCookieMonster += 15;
							}
						else
							{
							//sResult	= -4;
							TRACE("Game(): Cookie value is incorrect.\n");
							}
						}
					else
						{
						//sResult	= -3;
						TRACE("Game(): Failed to read cookie.\n");
						}
					}
				else
					{
					//sResult	= -2;
					TRACE("Game(): Cookie file is incorrect size!\n");
					}
#endif // defined(CHECK_FOR_COOKIE)

				file.Close();
				}
			else
				{
				sResult = -1;
				TRACE("Game(): Can't find assets based on CD path specified in prefs!\n");
				}

			// If any problems . . .
			if (sResult != 0)
				{
				rspMsgBox(RSP_MB_ICN_STOP | RSP_MB_BUT_OK, g_pszCriticalErrorTitle, g_pszCantFindAssets, "CD");
				}
			}

		// Check for CDROM drive.
		if (sResult == 0)
			{
#if defined(MUST_BE_ON_CD)
			// Check for the special case where the path is the one we use for
			// development.  If someone out there happens to use this as their
			// own path, then they will defeat this test for CD-ROM.  Oh well.
			char* pszDevelopmentPath = "\\\\narnia\\projects\\";
			if (strnicmp(FullPathCD("."), pszDevelopmentPath, strlen(pszDevelopmentPath)) != 0)
				{
#if defined(WIN32)
				if (GetDriveType(FullPathCD(".") ) != DRIVE_CDROM)
#else
				#error MUST_BE_ON_CD feature is currently implemented only for Win32
#endif
					{
					// Only set the error flag if we're in release mode
					#ifndef _DEBUG
						sResult = -1;
					#endif
					TRACE("Game(): CD path is not a CDROM!\n");
					rspMsgBox(RSP_MB_ICN_STOP | RSP_MB_BUT_OK, g_pszCriticalErrorTitle, g_pszNotOnCDROM);
					}
				}
#endif
			}

		if (sResult == 0)
			{

			// Set the gamma level to value indicated by settings.
			SetGammaLevel(g_GameSettings.m_sGammaVal);

			// If trickier quit specified . . .
			if (g_GameSettings.m_sTrickySystemQuit != FALSE)
				{
				// Add shift key as a requirement for the quit status keys.
				rspSetQuitStatusFlags(RSP_GKF_SHIFT);
				}

			// Open SAKs or setup equivalent paths.
			sResult = OpenSaks();
			if (sResult == 0)
				{
				// Start title, passing the "total units" for its progress meter
				sResult = StartTitle(1, true, &ms_siMusak);
				if (sResult == 0)
					{

					// Load assets that we want to keep around at all times
					sResult = LoadAssets();
					
					// End title (regardless of previous result)
					EndTitle();
					if (sResult == 0)
						{

						// Set the font most GUIs will use (the menu system uses its own RPrint).
						// Note that the size does not matter, we just want to set the font ptr.
						RGuiItem::ms_print.SetFont(15, &g_fontBig);

 						// Do the core game stuff
						sResult = GameCore();

						// If there weren't any errors, wrap things up
						if (!sResult)
							{

							// Do ending credits
							if (rspGetQuitStatus() > 1)
								{
								rspSetQuitStatus(0);
								Credits();
								}
							}
						}
					else
						{
						rspMsgBox(RSP_MB_ICN_STOP | RSP_MB_BUT_OK, g_pszCriticalErrorTitle, g_pszGeneralError);
						}

					// Unload assets loaded earlier
					UnloadAssets();

					}
				else
					{
					TRACE("Game(): Error returned by StartTitle()!\n");
					rspMsgBox(RSP_MB_ICN_STOP | RSP_MB_BUT_OK, g_pszCriticalErrorTitle, g_pszTitleError);
					}
				}
			else
				{
				TRACE("Game(): Error returned by OpenSaks().\n");
				}

#if 0
			// Make sure we're not paused . . .
			if (RMix::IsPaused() )
				{
				RMix::Resume();
				}
#endif

			// Abort all samples.
			AbortAllSamples();

			// We should never need a timeout but I don't want to risk a Muppets
			// scenario where a shitty sound driver causes us to think a sound is always
			// playing.
			// Wait for all samples to finish.
			long	lTimeOutTime	= rspGetMilliseconds() + TIME_OUT_FOR_ABORT_SOUNDS;
			// Wait for them to stop.
			while (IsSamplePlaying() == true && rspGetMilliseconds() < lTimeOutTime)
				{
				// Always do periodic updates.
				// Crucial to sound completing.
				UpdateSystem();
				}

			// Close SAKs and/or create SAKs??
			CloseSaks();
			}

		// Save all settings to preference file
		sResult = CSettings::SavePrefs(g_pszPrefFileName);
		if (sResult > 0)
			{
			TRACE("Game(): Read-only prefs file!\n");
//			rspMsgBox(RSP_MB_ICN_INFO | RSP_MB_BUT_OK, g_pszCriticalErrorTitle, g_pszPrefReadOnly);
			}
		else if (sResult < 0)
			{
			TRACE("Game(): Error writing prefs file!\n");
			rspMsgBox(RSP_MB_ICN_STOP | RSP_MB_BUT_OK, g_pszCriticalErrorTitle, g_pszPrefWriteError);
			}

		}
	else
		{
		TRACE("Game(): Error returned by ReadGamePrefs()!\n");
		rspMsgBox(RSP_MB_ICN_STOP | RSP_MB_BUT_OK, g_pszCriticalErrorTitle, g_pszPrefReadError);
		}

	// Remove the callbacks
	rspSetBackgroundCallback(NULL);
	rspSetForegroundCallback(NULL);
	}


////////////////////////////////////////////////////////////////////////////////
//
// Do the core game stuff (display menu, play a game, run the demo, etc.)
//
////////////////////////////////////////////////////////////////////////////////
static short GameCore(void)		// Returns 0 on success.
	{
	short sResult = 0;
	USHORT usDemoCount = 0;
	bool	bMPath = false,
			bMPathServer = false;

#ifdef CHECK_EXPIRATION_DATE
	#ifdef WIN32
		char acTime[100];
		strcpy(acTime, asctime(gmtime(&g_lExpTime)));
		#define NEXT_LINE "\n\n"
	#else
		char acTime[100];
		unsigned long lTime = g_lExpTime + (((365 * 70UL) + 17) * 24 * 60 * 60); // time_fudge 1900->1970
		strcpy(acTime, ctime(&lTime));
		char* pCR = strchr(acTime, '\n');
		if (pCR != NULL)
			*pCR = 0;
		#define NEXT_LINE "\r\r"
	#endif	// WIN32

	rspMsgBox(
		RSP_MB_ICN_INFO | RSP_MB_BUT_OK,
	#if defined(EXPIRATION_MSG_POSTAL_LAUNCH_WEEKEND)
		// Title
		"Postal Launch Weekend Edition",
	#elif defined(EXPIRATION_MSG_MARKETING_RELEASE)
		// Title
		"Postal Beta Contest",
		// Message Body
		"This is a large, single-player, single-level BETA test version -- that means we're "
		"still working on it and it's not final. But we want your comments to help us make it "
		"even cooler, so we're holding a BETA TEST CONTEST!"
		NEXT_LINE
		"We're giving away tons o' free stuff, including FREE GAMES, T-SHIRTS, and even a hot "
		"NEW DELL 266MHZ COMPUTER!"
		NEXT_LINE
		"If you haven't already registered, visit http://www.gopostal.com to find out more about Postal "
		"and how you can register for the BETA TEST CONTEST!"
		NEXT_LINE
	#else
		// Title
		"Limited-Time Version",
	#endif
		// Message Body
		"This limited-time version will expire on %s"
		NEXT_LINE
		"After the expiration date, you will still be able to get into the game, "
		"but you will notice that everyone bursts into flames and dies.  This is "
		"NOT a bug -- it simply indicates that your version has expired.",
		// Time String
		acTime);
#endif	// CHECK_EXPIRATION_DATE

	#ifndef _WIN32
	UnlockAchievement(ACHIEVEMENT_PLAY_ON_NON_WINDOWS_PLATFORM);
	#endif

	// Clear end of game flag - play will set the flag if the player wins the game
	g_bLastLevelDemo = false;

	// Clear any events that might be in the queue
	rspClearAllInputEvents();
	ClearXInputState();

	// Get the registry value
	GameGetRegistry();

	// Init demo mode times
	m_lDemoBaseTime = rspGetMilliseconds();
	m_lDemoTimeOut = g_GameSettings.m_lInitialDemoTimeOut;

	// Clear flags that are updated by the menu system callbacks (in this module)
	m_action = ACTION_NOTHING;
	m_sRealmNum = 0;
	m_szRealmFile[0] = 0;
	m_bJustOneRealm = false;

	// Flag indicates whether or not menu is active
	bool bMenuActive = false;
	ACTION	actionNext	= ACTION_NOTHING;	// Initialized for safety.

	Menu*	pmenuStart	= NULL;				// Next menu to start if not NULL.
	bool	bPalTran		= true;				// true to PalTranOn() before next
													// menu.
	bool	bTitleImage	= true;				// true to display title image before
													// next menu.
	bool	bTitleMusak	= true;				// true to play title musak during
													// next menu.  Requires title image.
//////////////////////////////////////
//////////////////////////////////////

	// Keep looping until user quits or error occurs.  Maybe even play
	// a game or two along the way.  If there's no user input for a
	// certain amount of time, automatically run the self-playing demo.
	RInputEvent ie;
	memset(&ie, '\0', sizeof (ie));  // fix valgrind complaining... --ryan.
	while (sResult == 0)
		{
		// Clear the end of game flag each time just to be safe, it only needs
		// to be set within the last iteration
		g_bLastLevelDemo = false;

		// As always...
		UpdateSystem();

		// Get next input event
		ie.type	= RInputEvent::None;
		rspGetNextInputEvent(&ie);

		// If there's any key or button input, or the app is in BG mode (per the OS),
		// reset the demo timer
		if ((ie.type != RInputEvent::None) || rspIsBackground())
			ResetDemoTimer();

		// Does user want to exit app?  Do this BEFORE checking whether menu is active
		// so we won't bother activating the menu only to find out we're going to exit.
		if (rspGetQuitStatus())
			break;

		// If menu isn't active, get it going
		if (!bMenuActive)
			{
			// If paltran requested . . .
			if (bPalTran)
				PalTranOn();

			sResult = StartMenu(pmenuStart ? pmenuStart : &menuMain, &g_resmgrShell, g_pimScreenBuf);
			if (sResult == 0)
				{
				bMenuActive = true;
				// Restore defaults.
				pmenuStart	= NULL;
				bPalTran		= true;
				bTitleImage	= true;
				bTitleMusak	= true;
				}
			else
				{
				TRACE("GameLoop(): Error returned by StartMenu()!\n");
				rspMsgBox(RSP_MB_ICN_STOP | RSP_MB_BUT_OK, g_pszAppName, "Cannot initialize menu system, most likely due to a memory or drive error.\n");
				break;
				}
			}

		// Let menu system do its thing
		DoMenuInput(&ie, g_InputSettings.m_sUseJoy);
		DoMenuOutput(g_pimScreenBuf);

		// Update the screen
		rspUpdateDisplay();

		// If this is not the spawn version, then check to see if the user has been
		// idle long enough to run a demo.  The spawn version doesn't have any
		// demos available, so we shouldn't try to run them.
#ifndef SPAWN
		// If there's no user action and the demo timer expired, simulate the
		// appropriate action to start the self-running demo.
#if 0 // Just disable demos entirely, we've changed things too much for the old demos to be compatible anymore. - Rick
		if ((m_action == ACTION_NOTHING) && 
		    ((rspGetMilliseconds() - m_lDemoBaseTime) >= m_lDemoTimeOut) &&
			 g_GameSettings.m_sNumAvailableDemos > 0)
			m_action = ACTION_DEMO_PLAYBACK;
#endif
#endif // SPAWN

		// Reset next action.
		actionNext	= ACTION_NOTHING;

		// Any action to handle?
		if (m_action != ACTION_NOTHING)
			{
			// Handle the action
			switch(m_action)
				{
				//------------------------------------------------------------------------------
				// Play single player game
				//------------------------------------------------------------------------------
				case ACTION_PLAY_SINGLE:
// If this is the spawn version, then they are not allowed to play single
// player, so we will take out some of the single player code so its not
// easy to hack back in.
#ifndef SPAWN
					// End menu
					StopMenu();
					PalTranOff();
					bMenuActive = false;

					// Clear the game winning flag - play will set it before it returns
					// if the player has won the game
					g_bLastLevelDemo = false;

					Play(
						NULL,									// No client (not network game)
						NULL,									// No server (not network game)
						INPUT_MODE_LIVE,					// Input mode
						m_sRealmNum,						// Realm number OR -1 to use realm file
						m_szRealmFile,						// Realm file
						m_bJustOneRealm,					// Whether to play just one realm or not
						false,								// Not challenge mode
						false,								// Not new single player Add on levels
						GetGameDifficulty(),				// Difficulty level
						false,								// Rejunenate (MP only)
						0,										// Time limit (MP only)
						0,										// Kill limit (MP only)
						0,										// Use cooperative levels (MP only)
						0,										// Use cooperative mode (MP only)
						0,										// Frame time (MP only)
						NULL);								// Demo mode file

#ifdef MOBILE
	AndroidSetScreenMode(TOUCH_SCREEN_MENU);
#endif

						// If the player won the game, show them the last level demo
						// and then the ending cutscenes.
						if (g_bLastLevelDemo)
							GameEndingSequence();

						// clear the end of game flag just to be safe
						g_bLastLevelDemo = false;
#endif
					break;
				
				//------------------------------------------------------------------------------
				// Join a multiplayer game
				//------------------------------------------------------------------------------
				case ACTION_PLAY_BROWSE:
				case ACTION_PLAY_CONNECT:
// If multiplayer is disabled, leave out this code to make it harder to hack back in
#ifndef MULTIPLAYER_DISABLED
					{
					// Set flag as to whether we're browsing or connecting
					bool bBrowse = (m_action == ACTION_PLAY_BROWSE) ? true : false;

					// Startup sockets with selected protocol
					if (RSocket::Startup((RSocket::ProtoType)g_GameSettings.m_usProtocol, false) == 0)
						{
						InitNetProbGUI();

						// Set next menu to start on to the current menu.
						pmenuStart	= GetCurrentMenu();
						// End the menu but don't PalTranOff unless we actually join a game.
						StopMenu();
						bMenuActive = false;
						bPalTran		= false;
						bTitleImage	= false;
						bTitleMusak	= false;

						// Use the net game dialog to join a multiplayer game
						CNetClient* pnetclient = new CNetClient;
						NetMsg msg;
						if (DoNetGameDialog(pnetclient, bBrowse, NULL, &msg) == 0)
							{
							// If the game was actually started...
							if (msg.msg.nothing.ucType == NetMsg::START_GAME)
								{
								PalTranOff();
								// Go back to the main menu when done and do
								// all the deluxe stuff.
								pmenuStart	= NULL;
								bPalTran		= true;
								bTitleImage	= true;
								bTitleMusak	= true;

								ASSERT(pnetclient->GetNumPlayers() >= 1);

								// Create synchronization logs, if enabled.
								OpenSynchLogs(true);

								Play(
									pnetclient,								// Client
									NULL,										// No server (not hosting game)
									INPUT_MODE_LIVE,						// Input mode
									msg.msg.startGame.sRealmNum,		// Realm number OR -1 to use realm file
									msg.msg.startGame.acRealmFile,	// Realm file
									msg.msg.startGame.sRealmNum >= 0 ? false : true, // Whether to play just one realm or not
									false,									// Not challenge mode
									false,									// Not new single player Add On leves
									msg.msg.startGame.sDifficulty,	// Difficulty
									msg.msg.startGame.sRejuvenate,	// Rejunenate (MP only)
									msg.msg.startGame.sTimeLimit,		// Time limit (MP only)
									msg.msg.startGame.sKillLimit,		// Kill limit (MP only)
									msg.msg.startGame.sCoopLevels,	// Cooperative or Deathmatch levels (MP only)
									msg.msg.startGame.sCoopMode,		// Cooperative or Deathmatch mode (MP only)
									msg.msg.startGame.sFrameTime,		// Frame time (MP only)
									NULL);									// Demo mode file

#ifdef MOBILE
	AndroidSetScreenMode(TOUCH_SCREEN_MENU);
#endif

								// Close synchronization logs, if opened.
								CloseSynchLogs();
								}
							}
						delete pnetclient;
						KillNetProbGUI();
						RSocket::Shutdown();
						}
					else
						{
						TRACE("GameCore(): Couldn't init protocol!\n");
						rspMsgBox(RSP_MB_ICN_STOP | RSP_MB_BUT_OK, g_pszAppName, "The selected network protocol has failed to initialize.  Please contact your system administrator or network vendor.\n");
						}
					}
#endif //MULTIPLAYER_DISABLED
					break;

				//------------------------------------------------------------------------------
				// Host a multiplayer game
				//------------------------------------------------------------------------------
				case ACTION_PLAY_HOST:
// If multiplayer is disabled, leave out this code to make it harder to hack back in
#ifndef MULTIPLAYER_DISABLED
					{
					// Startup sockets with selected protocol
					if (RSocket::Startup((RSocket::ProtoType)g_GameSettings.m_usProtocol, false) == 0)
						{
						InitNetProbGUI();

						// Set next menu to start on to the current menu.
						pmenuStart	= GetCurrentMenu();
						// End the menu but don't PalTranOff unless we actually join a game.
						StopMenu();
						bMenuActive = false;
						bPalTran		= false;
						bTitleImage	= false;
						bTitleMusak	= false;

						// Use the net game dialog to host a multiplayer game
						CNetClient* pnetclient = new CNetClient;
						CNetServer* pnetserver = new CNetServer;
						NetMsg msg;
						if (DoNetGameDialog(pnetclient, false, pnetserver, &msg) == 0)
							{
							// If the game was actually started...
							if (msg.msg.nothing.ucType == NetMsg::START_GAME)
								{
								PalTranOff();
								// Go back to the main menu when done and do
								// all the deluxe stuff.
								pmenuStart	= NULL;
								bPalTran		= true;
								bTitleImage	= true;
								bTitleMusak	= true;

								ASSERT(pnetclient->GetNumPlayers() >= 1);

								// Create synchronization logs, if enabled.
								OpenSynchLogs(true);

								Play(
									pnetclient,								// Client
									pnetserver,								// Server, too
									INPUT_MODE_LIVE,						// Input mode
									msg.msg.startGame.sRealmNum,		// Realm number OR -1 to use realm file
									msg.msg.startGame.acRealmFile,	// Realm file
									msg.msg.startGame.sRealmNum >= 0 ? false : true, // Whether to play just one realm or not
									false,									// Not challenge mode
									false,									// Not new single player Add on levels
									msg.msg.startGame.sDifficulty,	// Difficulty
									msg.msg.startGame.sRejuvenate,	// Rejunenate (MP only)
									msg.msg.startGame.sTimeLimit,		// Time limit (MP only)
									msg.msg.startGame.sKillLimit,		// Kill limit (MP only)
									msg.msg.startGame.sCoopLevels,	// Cooperative or Deathmatch levels (MP only)
									msg.msg.startGame.sCoopMode,		// Cooperative or Deathmatch mode (MP only)
									msg.msg.startGame.sFrameTime,		// Frame time (MP only)
									NULL);									// Demo mode file

#ifdef MOBILE
	AndroidSetScreenMode(TOUCH_SCREEN_MENU);
#endif
								// Close synchronization logs, if opened.
								CloseSynchLogs();
								}
							}
						delete pnetserver;
						delete pnetclient;
						KillNetProbGUI();
						RSocket::Shutdown();
						}
					else
						{
						TRACE("GameCore(): Couldn't init protocol!\n");
						rspMsgBox(RSP_MB_ICN_STOP | RSP_MB_BUT_OK, g_pszAppName, "The selected network protocol has failed to initialize.  Please contact your system administrator or network vendor.\n");
						}
					}
#endif //MULTIPLAYER_DISABLED
					break;

				//------------------------------------------------------------------------------
				// Play challenge game
				//------------------------------------------------------------------------------
				case ACTION_PLAY_CHALLENGE:
					// Remember menu to go back to.
					pmenuStart	= GetCurrentMenu();
					// End the menu.
					StopMenu();
					bMenuActive = false;
					// Turn off paltran but remember to restore.
					PalTranOff();
					bPalTran		= true;
					// Remember to show title, but no musak.
					bTitleImage	= true;
					bTitleMusak	= false;

					// Note that m_sRealmNum, m_szRealmFile, and m_bJustOneRealm are
					// set via the callback, Game_StartChallengeGame().
					// ***ADD FLAG(S) TO THIS CALL INDICATING THIS IS A CHALLENGE GAME***
					Play(
						NULL,									// No client (not network game)
						NULL,									// No server (not network game)
						INPUT_MODE_LIVE,					// Input mode
						m_sRealmNum,						// Realm number OR -1 to use realm file
						m_szRealmFile,						// Realm file
						m_bJustOneRealm,					// Whether to play just one realm or not
						true,									// Play challenge levels
						false,								// Not new single player Add On levels
						GetGameDifficulty(),				// Difficulty level
						false,								// Rejunenate (MP only)
						0,										// Time limit (MP only)
						0,										// Kill limit (MP only)
						0,										// Cooperative (MP only)
						0,										// Use cooperative mode (MP only)
						0,										// Frame time (MP only)
						NULL);								// Demo mode file

#ifdef MOBILE
	AndroidSetScreenMode(TOUCH_SCREEN_MENU);
#endif
					break;

				//------------------------------------------------------------------------------
				// Play Add On levels
				//------------------------------------------------------------------------------
				case ACTION_PLAY_ADDON:
#ifndef SPAWN
					// Remember menu to go back to.
					pmenuStart	= GetCurrentMenu();
					// End the menu.
					StopMenu();
					bMenuActive = false;
					// Turn off paltran but remember to restore.
					PalTranOff();
					bPalTran		= true;
					// Remember to show title, but no musak.
					bTitleImage	= true;
					bTitleMusak	= false;

					// Note that m_sRealmNum, m_szRealmFile, and m_bJustOneRealm are
					// set via the callback, Game_StartChallengeGame().
					// ***ADD FLAG(S) TO THIS CALL INDICATING THIS IS A CHALLENGE GAME***
					Play(
						NULL,									// No client (not network game)
						NULL,									// No server (not network game)
						INPUT_MODE_LIVE,					// Input mode
						m_sRealmNum,						// Realm number OR -1 to use realm file
						m_szRealmFile,						// Realm file
						m_bJustOneRealm,					// Whether to play just one realm or not
						false,								// Don't play challenge levels
						true,									// Play new single player Add on levels
						GetGameDifficulty(),				// Difficulty level
						false,								// Rejunenate (MP only)
						0,										// Time limit (MP only)
						0,										// Kill limit (MP only)
						0,										// Cooperative (MP only)
						0,										// Use cooperative mode (MP only)
						0,										// Frame time (MP only)
						NULL);								// Demo mode file
#ifdef MOBILE
	AndroidSetScreenMode(TOUCH_SCREEN_MENU);
#endif
#endif // SPAWN
					break;

				
				//------------------------------------------------------------------------------
				// Playback demo
				//------------------------------------------------------------------------------
				case ACTION_DEMO_PLAYBACK:
					{
					// Prepare all settings for demo mode
					CSettings::PreDemo();

					// If demo debug movie file is specified in prefs, open it now.  The RFile*
					// is does double-duty as a flag, where non-zero means movie mode is enabled.
					RFile* pfileDemoDebugMovie = 0;
#if 0
					if (strlen(g_GameSettings.m_szDemoDebugMovie) > 0)
						{
						pfileDemoDebugMovie = new RFile;
						if (pfileDemoDebugMovie->Open(g_GameSettings.m_szDemoDebugMovie, "rb", RFile::LittleEndian) != 0)
							{
							delete pfileDemoDebugMovie;
							pfileDemoDebugMovie = 0;
							}
						}
#else
					// Compare to synchronization logs, if enabled.
					OpenSynchLogs(false);
#endif

					if (InputDemoInit() == 0)
						{
						// If no specific filename has been set for the demo, then load one of 
						// the default demos.
						if (m_szDemoFile[0] == '\0')
							{
							// If there are default demos . . .
							if(g_GameSettings.m_sNumAvailableDemos > 0)
								{
								sprintf(m_szDemoFile, "%s%d%s", FullPathHD(DEFAULT_DEMO_PREFIX), usDemoCount % MAX((short) 1, g_GameSettings.m_sNumAvailableDemos), DEFAULT_DEMO_SUFFIX);
								}
							}
					
						// If there is now a demo filename . . .
						if (m_szDemoFile[0] != '\0')
							{
							RFile	fileDemo;
							usDemoCount++;
							if (fileDemo.Open(m_szDemoFile, "rb", RFile::LittleEndian) == 0)
								{
								// Read name of realm file
								char szRealmFile[RSP_MAX_PATH];
								fileDemo.Read(szRealmFile);
								// Read whether it's a full path.
								short	sRealmFileIsFullPath;
								fileDemo.Read(&sRealmFileIsFullPath);
								if (!fileDemo.Error())
									{
									// Load input demo data (must be BEFORE setting playback mode)
									if (InputDemoLoad(&fileDemo) == 0)
										{
										// End menu (now that we know there were no errors)
										StopMenu();
										PalTranOff();
										bMenuActive = false;

										Play(
											NULL,									// No client (not network game)
											NULL,									// No server (not network game)
											INPUT_MODE_PLAYBACK,				// Input mode
											-1,									// Always use specific realm file
											szRealmFile,						// Realm file to be played
											false,								// Don't play just one realm
											false,								// Not challenge mode
											false,								// Not new single player Add On levels
											GetGameDifficulty(),				// Difficulty level
											false,								// Rejunenate (MP only)
											0,										// Time limit (MP only)
											0,										// Kill limit (MP only)
											0,										// Cooperative (MP only)
											0,										// Use cooperative mode (MP only)
											0,										// Frame time (MP only)
											pfileDemoDebugMovie);			// Demo mode file

										}
									else
										{
										TRACE("GameCore(): Couldn't load demo data!\n");
										rspMsgBox(RSP_MB_ICN_STOP | RSP_MB_BUT_OK, g_pszAppName, g_pszFileReadError_s, (char*) m_szDemoFile);
										}
									}
								else
									{
									TRACE("GameCore(): Couldn't load realm name!\n");
									rspMsgBox(RSP_MB_ICN_STOP | RSP_MB_BUT_OK, g_pszAppName, g_pszFileReadError_s, (char*) m_szDemoFile);
									}
								fileDemo.Close();
								}
							else
								{
								TRACE("GameCore(): Couldn't open demo file: '%s'\n", m_szDemoFile);
								rspMsgBox(RSP_MB_ICN_STOP | RSP_MB_BUT_OK, g_pszAppName, g_pszFileOpenError_s, (char*) m_szDemoFile);
								}
							}
						else
							{
							TRACE("GameCore(): No demo filename.\n");
							}

						// Reset demo file name for next time.
						m_szDemoFile[0] = 0;
						InputDemoKill();
						}

#if 0
					if (pfileDemoDebugMovie)
						{
						// File may have been closed by Play() if an error occurred
						if (pfileDemoDebugMovie->IsOpen())
							pfileDemoDebugMovie->Close();
						delete pfileDemoDebugMovie;
						pfileDemoDebugMovie = 0;
						}
#else
					CloseSynchLogs();
#endif

#ifdef MOBILE
	AndroidSetScreenMode(TOUCH_SCREEN_MENU);
#endif
					// Restore settings to what they were before demo mode
					CSettings::PostDemo();
					}
					break;

				//------------------------------------------------------------------------------
				// Record demo
				//------------------------------------------------------------------------------
				case ACTION_DEMO_RECORD:
					{
					// Prepare all settings for demo mode
					CSettings::PreDemo();

					// If demo debug movie file is specified in prefs, open it now.  The RFile*
					// is does double-duty as a flag, where non-zero means movie mode is enabled.
					RFile* pfileDemoDebugMovie = 0;
#if 0
					if (strlen(g_GameSettings.m_szDemoDebugMovie) > 0)
						{
						pfileDemoDebugMovie = new RFile;
						if (pfileDemoDebugMovie->Open(g_GameSettings.m_szDemoDebugMovie, "wb", RFile::LittleEndian) != 0)
							{
							delete pfileDemoDebugMovie;
							pfileDemoDebugMovie = 0;
							}
						}
#else
					// Create synchronization logs, if enabled.
					OpenSynchLogs(true);
#endif

					if (InputDemoInit() == 0)
						{

						// Get name of realm to play
						char szRealmFile[RSP_MAX_PATH];
						short	sGetRealmResult	= GetRealmToRecord(szRealmFile, sizeof(szRealmFile));
						switch (sGetRealmResult)
							{
							case 0:	// Success.
								break;
							case 1:	// Not a relative path.  Got a full path.
								break;
							default:	// Error.
								break;
							}

						if (sGetRealmResult >= 0)
							{
							// Get name of demo file to save to
							char szDemoFile[RSP_MAX_PATH];
							if (GetDemoFile(szDemoFile, sizeof(szDemoFile)) == 0)
								{
								// Open demo file
								RFile	fileDemo;
								if (fileDemo.Open(szDemoFile, "wb", RFile::LittleEndian) == 0)
									{
									// Write name of realm file
									fileDemo.Write(szRealmFile);
									// Write whether it's a full path.
									fileDemo.Write(sGetRealmResult);

									// End menu (now that we know there were no errors)
									StopMenu();
									PalTranOff();
									bMenuActive = false;

									Play(
										NULL,									// No client (not network game)
										NULL,									// No server (not network game)
										INPUT_MODE_RECORD,				// Input mode
										-1,									// Always use specific realm file
										szRealmFile,						// Realm file to be played
										false,								// Don't play just one realm
										false,								// Not challenge mode
										false,								// Not new single player Add on levels
										GetGameDifficulty(),				// Difficulty level
										false,								// Rejunenate (MP only)
										0,										// Time limit (MP only)
										0,										// Kill limit (MP only)
										0,										// Cooperative (MP only)
										0,										// Use cooperative mode (MP only)
										0,										// Frame time (MP only)
										pfileDemoDebugMovie);			// Demo mode file
#ifdef MOBILE
	AndroidSetScreenMode(TOUCH_SCREEN_MENU);
#endif
									// Save input data to file
									if (InputDemoSave(&fileDemo) != 0)
										{
										TRACE("GameCore(): Couldn't save demo data!\n");
										rspMsgBox(RSP_MB_ICN_STOP | RSP_MB_BUT_OK, g_pszAppName, g_pszFileWriteError_s, szDemoFile);
										}
									}
								fileDemo.Close();
								}
							else
								{
								TRACE("GameCore(): Couldn't open demo file: '%s'\n", szDemoFile);
								rspMsgBox(RSP_MB_ICN_STOP | RSP_MB_BUT_OK, g_pszAppName, g_pszFileOpenError_s, szDemoFile);
								}
							}

						InputDemoKill();
						}

#if 0
					if (pfileDemoDebugMovie)
						{
						// File may have been closed by Play() if an error occurred
						if (pfileDemoDebugMovie->IsOpen())
							pfileDemoDebugMovie->Close();
						delete pfileDemoDebugMovie;
						pfileDemoDebugMovie = 0;
						}
#else
					CloseSynchLogs();
#endif

					// Restore settings to what they were before demo mode
					CSettings::PostDemo();
					}
					break;

				//------------------------------------------------------------------------------
				// Go to the editor
				//------------------------------------------------------------------------------
				#if !defined(EDITOR_DISABLED)
					case ACTION_EDITOR:
						// End menu
						StopMenu();
						PalTranOff();
						bMenuActive = false;

						// Run the game editor
						GameEdit();
						break;
				#endif

				//------------------------------------------------------------------------------
				// Edit key, mouse, or joystick settings.
				//------------------------------------------------------------------------------
				case ACTION_EDIT_INPUT_SETTINGS:
					// Leave the menu on.
					// Run the input settings editor.
					EditInputSettings();
					break;

				//------------------------------------------------------------------------------
				// Postal organ.
				//------------------------------------------------------------------------------
				case ACTION_POSTAL_ORGAN:
					// Launch Postal Organ.
					PlayWithMyOrgan();
					break;

				//------------------------------------------------------------------------------
				// Load previously saved user game.
				//------------------------------------------------------------------------------
				case ACTION_LOAD_GAME:
					{
					// Static so dialog will "remember" the previously-used name
					static char szFileSaved[RSP_MAX_PATH] = "";

					// If not yet used, start out in appropirate directory
					if (szFileSaved[0] == '\0')
						strcpy(szFileSaved, FullPathHD(SAVEGAME_DIR));

					// Display option dialog to let user choose a realm file
					#if 1 //PLATFORM_UNIX
                    char tmp[RSP_MAX_PATH];
					if (PickFile("Choose Game Slot", EnumExistingSaveGames, szFileSaved, sizeof(szFileSaved)) == 0)
                    {
#ifdef MOBILE
						//Android we have the format "1 - date"
						//Auot save is "Auto - date"
						//Need to create the filename
						char number = szFileSaved[0];
						if (number == 'A') //Check for auto save now
							snprintf(szFileSaved, sizeof (szFileSaved), "%s/auto.gme", SAVEGAME_DIR);
						else
							snprintf(szFileSaved, sizeof (szFileSaved), "%s/%c.gme", SAVEGAME_DIR,number);
						TRACE("Load file: %s",szFileSaved);
#else
                        char *ptr = strrchr(szFileSaved, '[');
                        if (ptr) *(ptr-1) = '\0';
#endif
						// This function will open the saved game file and set the correct game mode
						// and settings.  Note that this modifies the m_action (that's how we get
						// out this state...this confused me for a while but it seems like a good
						// way to choose the appropriate original action).
						Game_LoadPlayersGame(szFileSaved, &ms_sLoadedDifficulty, &actionNext);
						m_bJustOneRealm = false;
                    }
					#else
					if (rspOpenBox(g_pszLoadGameTitle, szFileSaved, szFileSaved, sizeof(szFileSaved), ".gme") == 0)
						{
						// This function will open the saved game file and set the correct game mode
						// and settings.  Note that this modifies the m_action (that's how we get
						// out this state...this confused me for a while but it seems like a good
						// way to choose the appropriate original action).
						Game_LoadPlayersGame(szFileSaved, &ms_sLoadedDifficulty, &actionNext);
						m_bJustOneRealm = false;
						}
					#endif
					break;
					}
#ifdef MOBILE
				case ACTION_CONTINUE_GAME:
					{
						static char szFileSaved[RSP_MAX_PATH] = "";
						snprintf(szFileSaved, sizeof (szFileSaved), "%s/auto.gme", SAVEGAME_DIR);
						Game_LoadPlayersGame(szFileSaved, &ms_sLoadedDifficulty, &actionNext);
						m_bJustOneRealm = false;
					}
					break;
#endif
				//------------------------------------------------------------------------------
				// Oooops
				//------------------------------------------------------------------------------
				default:
					TRACE("GameCore(): Unrecognized action: %ld!\n", (long)m_action);
					break;
				}

			// If this action lead to another action . . .
			if (actionNext != ACTION_NOTHING)
				{
				// Set current action to the next action
				m_action = actionNext;
				}
			else
				{
				// Reset action
				m_action = ACTION_NOTHING;
				}

			// If menu was taken away, restore the screen behind it
			if (!bMenuActive)
				{
				// If title requested . . .
				if (bTitleImage)
					{
					// Only use musak, if specified.
					StartTitle(0, bTitleMusak, &ms_siMusak);
					EndTitle();
					}
				}

			// Reset demo timer
			ResetDemoTimer();
			}
		}

	// Set the registry value before exiting
	GameSetRegistry();

	// If the menu is active, end it
	if (bMenuActive)
		{
		StopMenu();
		PalTranOff();
		bMenuActive = false;
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
//
// Reset temo timer
//
////////////////////////////////////////////////////////////////////////////////
static void ResetDemoTimer(void)
	{
	// Reset base time
	m_lDemoBaseTime = rspGetMilliseconds();
	
	// Change timeout to use persistant timeout
	m_lDemoTimeOut = g_GameSettings.m_lPersistentDemoTimeOut;
	}


////////////////////////////////////////////////////////////////////////////////
//
// Get realm to be recorded
//
////////////////////////////////////////////////////////////////////////////////
static short GetRealmToRecord(	// Returns 0 on success, negative on error, 1 if      
											// not subpathable (i.e., returned path is full path).
	char* pszRealmFile,				
	short sMaxFileLen)
	{
	short sResult = 0;

	// Static so dialog will "remember" the previously-used name
	static char	szFile[RSP_MAX_PATH]	= "";

	// If not yet used, start out in appropriate directory
//	if (szFile[0] == '\0')
		strcpy(szFile, FullPathHD(LEVEL_DIR));

	// Display open dialog to let user choose a file
	sResult = SubPathOpenBox(FullPathHD(""), "Choose Realm To Record", szFile, szFile, sizeof(szFile), "rlm");
	if (sResult >= 0)
		{
		// Convert path to RSPiX path.
		char*	pszFullPath	= rspPathFromSystem(szFile);
		// Check if result will fit into specified buffer
		if (strlen(pszFullPath) < sMaxFileLen)
			{
			strcpy(pszRealmFile, pszFullPath);
			}
		else
			{
			sResult = -1;
			TRACE("GetRealmToRecord(): File name too long to return in specified buffer!\n");
			}
	
		}
	return sResult;
	}

////////////////////////////////////////////////////////////////////////////////
//
// Get a subpath relative to the specified game path.
//
////////////////////////////////////////////////////////////////////////////////
extern short SubPathOpenBox(		// Returns 0 on success, negative on error, 1 if 
											// not subpathable (i.e., returned path is full path).
	char*	pszFullPath,				// In:  Full path to be relative to (system format).
	char* pszBoxTitle,				// In:  Title of box.
	char*	pszDefFileName,			// In:  Default filename (system format).
	char* pszChosenFileName,		// Out: User's choice (system format).
	short sStrSize,					// In:  Amount of memory pointed to by pszChosenFileName.
	char*	pszFilter /*= NULL*/)	// In:  If not NULL, '.' delimited extension based filename
											//	filter specification.  Ex: ".cpp.h.exe.lib" or "cpp.h.exe.lib"
											// Note: Cannot use '.' in filter.  Preceding '.' ignored.
	{
	short	sResult;

	char	szBasePath[RSP_MAX_PATH];
	long	lBasePathLen	= strlen(pszFullPath);
	if (lBasePathLen < sizeof(szBasePath) )
		{
		strcpy(szBasePath, pszFullPath);

		// Get index to last character
		short sLastIndex = lBasePathLen;
		if (sLastIndex > 0)
			sLastIndex--;

		#ifdef WIN32
			// If base path doesn't end with a slash, add one
			if (szBasePath[sLastIndex] != RSP_SYSTEM_PATH_SEPARATOR)
				{
				if ((sLastIndex + 2) < RSP_MAX_PATH)
					{
					szBasePath[sLastIndex+1] = RSP_SYSTEM_PATH_SEPARATOR;
					szBasePath[sLastIndex+2] = 0;
					}
				else
					{
					sResult = -1;
					TRACE("SubPathOpenBox(): Path would've exceed max length with separator tacked on!\n");
					}
				}
		#else
			// If base path ends with a colon, get rid of it
			if (szBasePath[sLastIndex] == RSP_SYSTEM_PATH_SEPARATOR)
				szBasePath[sLastIndex] = 0;
		#endif

		char szChosenFileName[RSP_MAX_PATH];

		// Display open dialog to let user choose a file
		sResult = rspOpenBox(pszBoxTitle, pszDefFileName, szChosenFileName, sizeof(szChosenFileName), pszFilter);
		if (sResult == 0)
			{
			// Attempt to remove path from the specified name
			long	lFullPathLen	= strlen(szBasePath);
			if (rspStrnicmp(szChosenFileName, szBasePath, lFullPathLen) == 0)
				{
				// Copy sub path to destination.
				strcpy(pszChosenFileName, szChosenFileName + lFullPathLen);
				}
			else
				{
				// Not subpathable.
				sResult	= 1;
				// Return fullpath.
				// Copy full path to destination.
				strcpy(pszChosenFileName, szChosenFileName);
				}
			}
		}
	else
		{
		sResult	= -2;
		TRACE("SubPathOpenBox(): pszFullPath string too long.\n");
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
//
// Get name of demo file
//
////////////////////////////////////////////////////////////////////////////////
static short GetDemoFile(
	char* pszDemoFile,
	short sMaxFileLen)
	{
	short sResult = 0;
	
	// Static so dialog will "remember" the previously-used name
	static char szFile[RSP_MAX_PATH] = "";

	// If not yet used, start out in appropriate directory
	if (szFile[0] == '\0')
		strcpy(szFile, FullPathVD(DEMO_DIR));

	// Display save dialog to let user choose a file
	sResult = rspSaveBox(g_pszSaveDemoTitle, szFile, szFile, sizeof(szFile), DEMO_EXT);
	if (sResult == 0)
		{
		// Check if result will fit into specified buffer
		if (strlen(szFile) < sMaxFileLen)
			{
			strcpy(pszDemoFile, szFile);
			}
		else
			{
			sResult = -1;
			TRACE("GetDemoFile(): File name too long to return in specified buffer!\n");
			}
		}

	return sResult;
	}

////////////////////////////////////////////////////////////////////////////////
// Macro to get the sound SAK and (for the event there is no SAK) dir path.
////////////////////////////////////////////////////////////////////////////////
inline void GetSoundPaths(		// Returns nothing.
	long	lSamplesPerSec,		// In:  The sample rate in samples per second.
	long	lBitsPerSample,		// In:  The number of bits per sample.
	char* pszSakPath,				// Out: The subpath and name of the sound SAK.
										// Should be able to store at least RSP_MAX_PATH
										// characters here.
	char* pszNoSakDir)			// Out: The full path of the sound dir to use when 
										// there is no sak.
										// Should be able to store at least RSP_MAX_PATH
										// characters here.
	{
	// Make the SAK and base path name.
	char	szAudioResDescriptor[256];
	sprintf(
		szAudioResDescriptor,
		"%ld%c%ld",
		lSamplesPerSec,
		AUDIO_SAK_SEPARATOR_CHAR,
		lBitsPerSample);

	// Create the samples SAK sub path.
	strcpy(pszSakPath, SAMPLES_SAK_SUBDIR);
	strcat(pszSakPath, szAudioResDescriptor);
	strcat(pszSakPath, ".sak");

	// Create the samples NO SAK sub path.
	char	szSamplesNoSakSubPath[RSP_MAX_PATH];
	strcpy(szSamplesNoSakSubPath, "sound/");
	strcat(szSamplesNoSakSubPath, szAudioResDescriptor);
	// Note that g_GameSettings.m_szNoSakDir is already system
	// specific and already contains the appropriate ending
	// path delimiter, if any, appropriate to the current
	// platform.
	strcpy(pszNoSakDir, g_GameSettings.m_szNoSakDir);
	strcat(pszNoSakDir, rspPathToSystem(szSamplesNoSakSubPath) );
	}

////////////////////////////////////////////////////////////////////////////////
//
// Open SAKs or set equivalent base paths.
//
////////////////////////////////////////////////////////////////////////////////
static short OpenSaks(void)
	{
	short	sResult	= 0;	// Assume success.

	// Set base paths.
	g_resmgrShell.SetBasePath(g_GameSettings.m_szNoSakDir);
	g_resmgrGame.SetBasePath(g_GameSettings.m_szNoSakDir);
	g_resmgrRes.SetBasePath(FullPath(GAME_PATH_HD, "") );

	// Attempt to load the Game SAK . . .
	if (g_resmgrGame.OpenSak(FullPath(GAME_PATH_GAME, GAME_SAK_FILENAME) ) == 0)
		{
		}

	// Attempt to load the Shell SAK . . .
	if ((g_resmgrShell.OpenSak(FullPath(GAME_PATH_HD, SHELL_SAK_FILENAME) ) == 0) ||
		 (g_resmgrShell.OpenSak(FullPath(GAME_PATH_VD, SHELL_SAK_FILENAME) ) == 0))
		{
		}

	////////////////////////////////////////////////////////////////////////////
	// The Samples res directory and SAK filename are based on the current
	// audio mode.
	////////////////////////////////////////////////////////////////////////////

	// Get the current audio mode, if any.
	short	sInSoundMode;
	long	lSamplesPerSec;
	long	lDevBitsPerSample;
	long	lSrcBitsPerSample;
	long	lMixBitsPerSample;
	if (RMix::GetMode(				// Returns 0 on success;            
											// nonzero if no mode.              
			&lSamplesPerSec,			// Sample rate in samples per second
											// returned here, if not NULL.
			&lDevBitsPerSample,		// Bits per sample of device,
											// returned here, if not NULL.
			NULL,							// Number of channels (1 == mono, 
											// 2 == stereo) returned here, 
											// if not NULL.
			NULL,							// Amount of time in ms to lead the 
											// current play cursor returned here,
											// if not NULL.  This could also be 
											// described as the maximum amount of
											// time in ms that can occur between 
											// calls to rspDoSound.
			NULL,							// Maximum buffer time.  This is the amt
											// that *plBufferTime can be increased to.
											// This is indicative of how much space
											// was/will-be allocated for the sound
											// output device on rspLockSoundOut.
			&lMixBitsPerSample,		// Bits per sample at which samples are
											// mixed, if not NULL.
			&lSrcBitsPerSample)		// Bits per sample at which samples must
											// be to be mixed (0 if no requirement), 
											// if not NULL.
		== 0)
		{
		// Sample quality values set by rspGetSoundOutMode().

		// If no pref on src bits . . .
		if (lSrcBitsPerSample == 0)
			{
			// Set it to the mix quality.  This is a just in case as currently
			// these are always the same as set in main.cpp.
			lSrcBitsPerSample	= lMixBitsPerSample;
			}

		// Note that there is a real sound mode.
		sInSoundMode	= TRUE;
		}
	else
		{
		// In the case that there is no audio mode we still need to access one set of
		// samples so we assume Vanilla settings b/c these use less memory.
		lSamplesPerSec	= MAIN_VANILLA_AUDIO_RATE;
		lDevBitsPerSample	= lMixBitsPerSample = lSrcBitsPerSample = MAIN_VANILLA_AUDIO_BITS;
		
		// Note that there is no real sound mode.
		sInSoundMode	= FALSE;
		}

	// Actual sample rates can (and do) vary from system to system.  We need
	// to "round" the actual rate so it matches one of the specific rates that
	// we're looking for.  The variance we allow for is +/- one percent.
	if ((lSamplesPerSec >= (11025 - 110)) && (lSamplesPerSec <= (11025 + 110)))
		lSamplesPerSec = 11025;
	else if ((lSamplesPerSec >= (22050 - 220)) && (lSamplesPerSec <= (22050 + 220)))
		lSamplesPerSec = 22050;
	else if ((lSamplesPerSec >= (44100 - 441)) && (lSamplesPerSec <= (44100 + 441)))
		lSamplesPerSec = 44100;
	else
		{
		TRACE("OpenSaks(): Unsupported sample rate: %ld!\n", (long)lSamplesPerSec);
		ASSERT(0);
		}

	char	szSamplesSakSubPath[RSP_MAX_PATH];
	char	szSamplesNoSakFullPath[RSP_MAX_PATH];
	GetSoundPaths(lSamplesPerSec, lSrcBitsPerSample, szSamplesSakSubPath, szSamplesNoSakFullPath);

	// Attempt to load the Sample SAK . . .
	if (g_resmgrSamples.OpenSak(FullPath(GAME_PATH_SOUND, szSamplesSakSubPath) ) == 0)
		{
		// Wahoo.  No worries.
		}
	// Otherwise, if there's a dir for files when there's no SAK . . .
	else if (g_GameSettings.m_szNoSakDir[0] != '\0')
		{
		g_resmgrSamples.SetBasePath(szSamplesNoSakFullPath);
		}
	// Otherwise, . . .
	else
		{
		// If there's a sound mode that was successfully set up . . .
		if (sInSoundMode)
			{
			char	szSoundQuality[256];
			sprintf(szSoundQuality, "%.3f kHz, %hd Bit",
				(float)lSamplesPerSec/(float)1000,
				(short)lSrcBitsPerSample,
				(MAIN_AUDIO_CHANNELS == 1) ? "Mono" : "Stereo");

			rspMsgBox(
				RSP_MB_ICN_INFO | RSP_MB_BUT_OK,
				g_pszAppName,
				g_pszCannotOpenSoundFiles_s_s,
				szSoundQuality,
				szSoundQuality);

			// Failure.
			sResult	= 1;
			}
		else
			{
			// We've one last chance.  Try 'em all.  Technically, we've already
			// tried 22050, 8 (the 'Vanilla mode') if we got here, but let's try
			// them all to make sure.
			struct
				{
				long	lSamplesPerSec;
				long	lBitsPerSample;
				}	amodes[]	=
					{
						// Put the smaller ones first b/c they use less memory.
						{ 22050, 8 },
						{ 11025, 16 },
						{ 22050, 16 },
					};

			short	sModeIndex;
			bool	bSakFound	= false;

			for (sModeIndex = 0; sModeIndex < NUM_ELEMENTS(amodes) && bSakFound == false; sModeIndex++)
				{
				// Get the appropriate sample SAK name.
				GetSoundPaths(amodes[sModeIndex].lSamplesPerSec, amodes[sModeIndex].lBitsPerSample, szSamplesSakSubPath, szSamplesNoSakFullPath);
			
				// Attempt to load the Sample SAK . . .
				if (g_resmgrSamples.OpenSak(FullPath(GAME_PATH_SOUND, szSamplesSakSubPath) ) == 0)
					{
					// Set values to determine SampleMaster quality.
					// This is probably not necessary when using no sound but let's be safe.
					lSamplesPerSec	= amodes[sModeIndex].lSamplesPerSec;
					lSrcBitsPerSample	= amodes[sModeIndex].lBitsPerSample;
					// Got one.
					bSakFound	= true;
					}
				}

			// If no SAK found . . .
			if (bSakFound == false)
				{
				rspMsgBox(
					RSP_MB_ICN_INFO | RSP_MB_BUT_OK,
					g_pszAppName,
					g_pszNoSoundFiles);

				// Failure.
				sResult	= 1;
				}
			}
		}

	// Set the appropriate quality.
	g_GameSettings.m_eCurSoundQuality	= (SampleMaster::SoundQuality)( ( (lSamplesPerSec / 11025) - 1) * 2 + ( (lDevBitsPerSample / 8) - 1) );

	// Set volumes based on quality's category adjustor.
	short	i;
	for (i = 0; i < SampleMaster::MAX_NUM_SOUND_CATEGORIES; i++)
		{
		SetCategoryVolume((SampleMaster::SoundCategory)i, g_GameSettings.m_asCategoryVolumes[i] );
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
//
// Close SAKs and/or Purge() them.
//
////////////////////////////////////////////////////////////////////////////////
static void CloseSaks(void)
	{
	g_resmgrShell.Purge();
	g_resmgrShell.CloseSak();
	
	g_resmgrGame.Purge();
	g_resmgrGame.CloseSak();
	
	g_resmgrSamples.Purge();
	g_resmgrSamples.CloseSak();

	g_resmgrRes.Purge();
	g_resmgrRes.CloseSak();
	}


////////////////////////////////////////////////////////////////////////////////
//
// Load game data that we want around the entire game.
//
// I'm not entirely against just keeping all the game data global since that's
// the way it's always been done.  On the other hand, I kind of like the idea
// of using a similar mechanism for loading game data as I did for doing the
// game settings.  It allows various modules to automatically be included in
// the loading process, and keeps the data local to the module.  Back on the
// first hand, we'll probably be grouping data into large, wad-type files, so
// somehow it seems less compelling to try to keep the data separated within
// the app.
//
////////////////////////////////////////////////////////////////////////////////
static short LoadAssets(void)
	{
	// Load font.
	if (g_fontBig.Load(FullPath(GAME_PATH_VD, BIG_FONT_FILE)) != 0)
		{
		TRACE("GameLoadAssets(): Error loading font: %s !\n", FullPath(GAME_PATH_VD, BIG_FONT_FILE));
		return -1;
		}

	// Load font, the smaller.
	if (g_fontPostal.Load(FullPath(GAME_PATH_VD, POSTAL_FONT_FILE) ) != 0)
		{
		TRACE("GameLoadAssets(): Error loading font: %s !\n", FullPath(GAME_PATH_VD, POSTAL_FONT_FILE));
		return -1;
		}

	short i;
	long lTotalTime = 0;
	for (i = 0; i < TitleGetNumTitles(); i++)
		lTotalTime += g_GameSettings.m_alTitleDurations[i];

	// Fake lots of loading with a simple timing loop
	long	lTime;
	long	lLastTime	= rspGetMilliseconds();
	long	lEndTime		= lLastTime + lTotalTime;
	do
		{
		lTime		= rspGetMilliseconds();

		UpdateSystem();

		DoTitle(lTime - lLastTime);

		lLastTime	= lTime;

		} while (lTime < lEndTime && rspGetQuitStatus() == FALSE);

	// If we get this far, return success
	return 0;
	}


////////////////////////////////////////////////////////////////////////////////
//
// Unload game data that was loaded by GameLoadAssets().
//
////////////////////////////////////////////////////////////////////////////////
static short UnloadAssets(void)
	{
	return 0;
	}


////////////////////////////////////////////////////////////////////////////////
//
// Callback for the "Start Single Player Game" menu
//
////////////////////////////////////////////////////////////////////////////////
extern void Game_StartSinglePlayerGame(
	short sMenuItem)
	{

	// we reset these as we go along.
	const bool usedCheats = ((Flag_Achievements & FLAG_USED_CHEATS) != 0);
	Flag_Achievements = FLAG_USED_M16 | FLAG_KILLED_EVERYTHING | FLAG_KILLED_ONLY_HOSTILES | FLAG_HIGHEST_DIFFICULTY;
	if (usedCheats)
		Flag_Achievements |= FLAG_USED_CHEATS;

	playthroughMS = 0;

// If its a spawn version, then don't allow them to play single player
// games, and to make it harder, we will take out some of the code for
// single player games so its harder to hack back in.
#ifndef SPAWN

	switch (sMenuItem)
		{
		// "ORIGINAL LEVELS"
		case 0:
			m_action = ACTION_PLAY_SINGLE;
			m_sRealmNum = 0;
			m_szRealmFile[0] = 0;
			m_bJustOneRealm = false;
			break;
		#if defined(START_MENU_ADDON_ITEM)
			#define START_MENU_ID_OFFSET	0
			// "ADD-ON LEVELS"
			case 1:
				m_action = ACTION_PLAY_ADDON;
				m_sRealmNum = 0;
				m_szRealmFile[0] = 0;
				m_bJustOneRealm = false;
				break;
		#else
			#define START_MENU_ID_OFFSET	-1
		#endif

#ifdef MOBILE
		case 2 + START_MENU_ID_OFFSET:
		m_action	= ACTION_CONTINUE_GAME;
		break;
		case 3 + START_MENU_ID_OFFSET:
#else
		// "LOAD" game
		case 2 + START_MENU_ID_OFFSET:
#endif
        #ifndef LOADLEVEL_REMOVED
			{
			// Static so dialog will "remember" the previously-used name
			static char	szFile[RSP_MAX_PATH]	= "";

			// If not yet used, start out in appropriate directory
			if (szFile[0] == '\0')
				strcpy(szFile, FullPathHD(LEVEL_DIR));

			if (rspOpenBox("Load Realm", szFile, szFile, sizeof(szFile), ".rlm") == 0)
				{
				// Convert path from system format to rspix format so it matches the
				// way we normally call Play(), which is with a rspix path.
				rspPathFromSystem(szFile, m_szRealmFile);

				m_action = ACTION_PLAY_SINGLE;
				m_sRealmNum = -1;
				m_bJustOneRealm = true;
				}
			break;
			}

		// For the final version, the LOAD above will actually be this, but
		// it is still useful for testing the way it is now, so I'll add this
		//	as a separate option - Load Saved Game
		case 3 + START_MENU_ID_OFFSET:
        #endif
			m_action	= ACTION_LOAD_GAME;
			break;
#if (TARGET == POSTAL_2015)
		case 3 + START_MENU_ID_OFFSET:
			Game_StartChallengeGame(0); 
			break;
#endif
		}

	// The main game loop resets the demo timer whenever it notices any user input.
	// However, when the user is in a dialog or message box, the OS handles all the
	// user input, and the main game loop won't know anything about what's going on
	// in there.  If the user spends a long time in there, the demo timer will
	// expire.  We don't want that to happen, so we manually reset the demo timer
	// here in recognition of the fact that some kind of user input obviously occurred.
	ResetDemoTimer();
#endif // SPAWN
	}


////////////////////////////////////////////////////////////////////////////////
//
// Callback for the "Start MultiPlayer Game" menu
//
////////////////////////////////////////////////////////////////////////////////
extern bool Game_StartMultiPlayerGame(
	short sMenuItem)
	{
	bool bAccept = true;

	#if defined(MULTIPLAYER_DISABLED)
		rspMsgBox(RSP_MB_ICN_INFO | RSP_MB_BUT_OK, APP_NAME, g_pszMultiplayerDisabled);
		bAccept = false;
	#else
		// Do nothing if multiplayer is available
	#endif

	// The main game loop resets the demo timer whenever it notices any user input.
	// However, when the user is in a dialog or message box, the OS handles all the
	// user input, and the main game loop won't know anything about what's going on
	// in there.  If the user spends a long time in there, the demo timer will
	// expire.  We don't want that to happen, so we manually reset the demo timer
	// here in recognition of the fact that some kind of user input obviously occurred.
	ResetDemoTimer();

	return bAccept;
	}


////////////////////////////////////////////////////////////////////////////////
//
// Callback for the "Join MultiPlayer Game" menu
//
////////////////////////////////////////////////////////////////////////////////
extern void Game_JoinMultiPlayerGame(
	short sMenuItem)
	{
	switch (sMenuItem)
		{
		// "BROWSE"
		case 0:
			m_action = ACTION_PLAY_BROWSE;
			break;

		// "CONNECT"
		case 1:
			m_action = ACTION_PLAY_CONNECT;
			break;
		}

	// The main game loop resets the demo timer whenever it notices any user input.
	// However, when the user is in a dialog or message box, the OS handles all the
	// user input, and the main game loop won't know anything about what's going on
	// in there.  If the user spends a long time in there, the demo timer will
	// expire.  We don't want that to happen, so we manually reset the demo timer
	// here in recognition of the fact that some kind of user input obviously occurred.
	ResetDemoTimer();
	}


////////////////////////////////////////////////////////////////////////////////
//
// Callback for the "Host MultiPlayer Game" menu
//
////////////////////////////////////////////////////////////////////////////////
extern void Game_HostMultiPlayerGame(
	short sMenuItem)
	{
	switch (sMenuItem)
		{
		// "HOST"
		case 0:
			m_action = ACTION_PLAY_HOST;
			break;
		}

	// The main game loop resets the demo timer whenever it notices any user input.
	// However, when the user is in a dialog or message box, the OS handles all the
	// user input, and the main game loop won't know anything about what's going on
	// in there.  If the user spends a long time in there, the demo timer will
	// expire.  We don't want that to happen, so we manually reset the demo timer
	// here in recognition of the fact that some kind of user input obviously occurred.
	ResetDemoTimer();
	}

////////////////////////////////////////////////////////////////////////////////
//
// Callback for the "Start Demo Game" menu
//
////////////////////////////////////////////////////////////////////////////////
extern void Game_StartDemoGame(
	short sMenuItem)
	{
	char*	pszDemoFile	= NULL;
	char	szLevelDir[RSP_MAX_PATH]	= "";
	char  szTitle[256] = "";

	switch (sMenuItem)
		{
		// Browse for and Playback demo
		case 0:
		{
			// Get the filename of the demo to load.
			static char szFile[RSP_MAX_PATH] = "";
			sprintf(szLevelDir, "%s", DEMO_LEVEL_DIR);
			pszDemoFile = szFile;
			// If not yet used, start out in appropriate directory
			if (pszDemoFile[0] == '\0')
				strcpy(pszDemoFile, FullPathHD(szLevelDir) );

			// Display open dialog to let user choose a realm file
			sprintf(szTitle, "%s", DEMO_OPEN_TITLE);
			if (rspOpenBox(szTitle, pszDemoFile, m_szDemoFile, sizeof(m_szDemoFile), ".dmo") == 0)
			{
				m_action = ACTION_DEMO_PLAYBACK;
			}
		}
		break;

		// Play auto demo.
		case 1:
			// Clear demo filename.  This signifies that we should play one of 
			// the auto demos.
			m_szDemoFile[0]	= '\0';
			m_action = ACTION_DEMO_PLAYBACK;
			break;

		// Record demo
		case 2:
			m_action = ACTION_DEMO_RECORD;
			break;
		}

	// The main game loop resets the demo timer whenever it notices any user input.
	// However, when the user is in a dialog or message box, the OS handles all the
	// user input, and the main game loop won't know anything about what's going on
	// in there.  If the user spends a long time in there, the demo timer will
	// expire.  We don't want that to happen, so we manually reset the demo timer
	// here in recognition of the fact that some kind of user input obviously occurred.
	ResetDemoTimer();
	}


////////////////////////////////////////////////////////////////////////////////
//
// Callback for the "Buy" option on the Main Menu
//
////////////////////////////////////////////////////////////////////////////////
extern void Game_Buy(void)
	{
	rspMsgBox(RSP_MB_ICN_INFO | RSP_MB_BUT_OK, APP_NAME, g_pszBuy);

	// The main game loop resets the demo timer whenever it notices any user input.
	// However, when the user is in a dialog or message box, the OS handles all the
	// user input, and the main game loop won't know anything about what's going on
	// in there.  If the user spends a long time in there, the demo timer will
	// expire.  We don't want that to happen, so we manually reset the demo timer
	// here in recognition of the fact that some kind of user input obviously occurred.
	ResetDemoTimer();
	}

////////////////////////////////////////////////////////////////////////////////
//
// Callback for the "Editor" option on the Main Menu
//
////////////////////////////////////////////////////////////////////////////////
extern void Game_StartEditor(void)
	{
	#if defined(EDITOR_DISABLED)
		rspMsgBox(RSP_MB_ICN_INFO | RSP_MB_BUT_OK, APP_NAME, g_pszEditorDisabled);
	#else
		m_action = ACTION_EDITOR;
	#endif

	// The main game loop resets the demo timer whenever it notices any user input.
	// However, when the user is in a dialog or message box, the OS handles all the
	// user input, and the main game loop won't know anything about what's going on
	// in there.  If the user spends a long time in there, the demo timer will
	// expire.  We don't want that to happen, so we manually reset the demo timer
	// here in recognition of the fact that some kind of user input obviously occurred.
	ResetDemoTimer();
	}

////////////////////////////////////////////////////////////////////////////////
//
// Callback for the "Controls" menu.
//
////////////////////////////////////////////////////////////////////////////////
extern void Game_ControlsMenu(
	short sMenuItem)
	{
	// Only do this if we're not currently in an action . . .
	if (m_action == ACTION_NOTHING)
		{
		switch (sMenuItem)
			{
			// Edit keyboard settings.
			case 0:
				m_action = ACTION_EDIT_INPUT_SETTINGS;
				break;

			// Edit mouse settings.
			case 1:
				m_action = ACTION_EDIT_INPUT_SETTINGS;
				break;

			// Edit joystick settings.
			case 2:
#if defined(ALLOW_JOYSTICK)
				m_action	= ACTION_EDIT_INPUT_SETTINGS;
#endif	// defined(ALLOW_JOYSTICK)
				break;
			}

		// The main game loop resets the demo timer whenever it notices any user input.
		// However, when the user is in a dialog or message box, the OS handles all the
		// user input, and the main game loop won't know anything about what's going on
		// in there.  If the user spends a long time in there, the demo timer will
		// expire.  We don't want that to happen, so we manually reset the demo timer
		// here in recognition of the fact that some kind of user input obviously occurred.
		ResetDemoTimer();
		}
	}

////////////////////////////////////////////////////////////////////////////////
//
// Callback for "Audio Options" menu.
//
////////////////////////////////////////////////////////////////////////////////
extern void Game_AudioOptionsChoice(	// Returns nothing.
	short sMenuItem)							// In:  Chosen item.
	{
	switch (sMenuItem)
		{
		case 1:
			m_action = ACTION_POSTAL_ORGAN;
			break;
		}
	}



////////////////////////////////////////////////////////////////////////////////
//
// Callback for the "Start Challenge" menu.
//
////////////////////////////////////////////////////////////////////////////////
extern void Game_StartChallengeGame(	// Returns nothing.
	short sMenuItem)							// In:  Chosen menu item.
	{
	char*	pszRealmFile	= NULL;
	char	szLevelDir[RSP_MAX_PATH]	= "";
	char	szTitle[256]					= "";

	switch (sMenuItem)
		{
		// Run the Gauntlet.
		case 0:
			m_action = ACTION_PLAY_CHALLENGE;
			m_sRealmNum = 0;
			m_szRealmFile[0] = 0;
			m_bJustOneRealm = false;
			break;

		// Timed Challenge.
		case 1:
			{
			// Static so dialog will "remember" the previously-used name
			static char	szFile[RSP_MAX_PATH]	= "";
			pszRealmFile	= szFile;
			strcpy(szLevelDir, TIMED_CHALLENGE_LEVEL_DIR);
			strcpy(szTitle, TIMED_CHALLENGE_OPEN_TITLE);

			break;
			}

		// Goal Challenge.
		case 2:
			{
			// Static so dialog will "remember" the previously-used name
			static char	szFile[RSP_MAX_PATH]	= "";
			pszRealmFile	= szFile;
			strcpy(szLevelDir, GOAL_CHALLENGE_LEVEL_DIR);
			strcpy(szTitle, GOAL_CHALLENGE_OPEN_TITLE);

			break;
			}

		// Flag Challenge.
		case 3:
			{
			// Static so dialog will "remember" the previously-used name
			static char	szFile[RSP_MAX_PATH]	= "";
			pszRealmFile	= szFile;
			strcpy(szLevelDir, FLAG_CHALLENGE_LEVEL_DIR);
			strcpy(szTitle, FLAG_CHALLENGE_OPEN_TITLE);

			break;
			}

		// Checkpoint Challenge.
		case 4:
			{
			// Static so dialog will "remember" the previously-used name
			static char	szFile[RSP_MAX_PATH]	= "";
			pszRealmFile	= szFile;
			strcpy(szLevelDir, CHECKPOINT_CHALLENGE_LEVEL_DIR);
			strcpy(szTitle, CHECKPOINT_CHALLENGE_OPEN_TITLE);

			break;
			}
		}

	if (pszRealmFile)
		{
		// If not yet used, start out in appropriate directory
		if (pszRealmFile[0] == '\0')
			//strcpy(pszRealmFile, FullPathHD(szLevelDir) );
			strcpy(pszRealmFile, FullPathCD(szLevelDir) );

		// Display open dialog to let user choose a realm file

		if (rspOpenBox(szTitle, pszRealmFile, m_szRealmFile, sizeof(m_szRealmFile), ".rlm") == 0)
			{

			// Convert path from system format to rspix format so it matches the
			// way we normally call Play(), which is with a rspix path.
			// MASSIVE BUG IN rspPathFromSystem() on the Mac -- it doesn't allow
			// the src and dst to be the same, even though the doc says it can!!!!
			// Workaround is to use temporary buffer.
			char szTmp[RSP_MAX_PATH];
			strcpy(szTmp, m_szRealmFile);
			rspPathFromSystem(szTmp, m_szRealmFile);
			m_action				= ACTION_PLAY_CHALLENGE;
			m_sRealmNum			= -1;
			m_bJustOneRealm	= false;
			}
		}

	// The main game loop resets the demo timer whenever it notices any user input.
	// However, when the user is in a dialog or message box, the OS handles all the
	// user input, and the main game loop won't know anything about what's going on
	// in there.  If the user spends a long time in there, the demo timer will
	// expire.  We don't want that to happen, so we manually reset the demo timer
	// here in recognition of the fact that some kind of user input obviously occurred.
	ResetDemoTimer();
	}

////////////////////////////////////////////////////////////////////////////////
//
// Callback for the Main Menu init/kill.
//
////////////////////////////////////////////////////////////////////////////////
extern void Game_InitMainMenu(	// Returns nothing.
	short sInit)						// In:  TRUE, if initializing; FALSE, if killing.
	{
	// If initializing the menu . . .
	if (sInit)
		{
		}
	// Otherwise, killing the menu.
	else
		{
		// Abort the title/main-menu musak.
		AbortSample(ms_siMusak);
		// No need to reset ms_siMusak.
		}
	}

////////////////////////////////////////////////////////////////////////////////
//
// Game_SavePlayersGame - Save the realm, game type, difficulty, and stockpile
//
////////////////////////////////////////////////////////////////////////////////

extern short Game_SavePlayersGame(
				char* pszSaveName,		// In:  Name of the save file
				short sDifficulty)		// In:  Current realm difficulty.
{
	RFile rf;
	short sResult = rf.Open(pszSaveName, "wb", RFile::LittleEndian);
	ULONG ulFileVersion = CRealm::FileVersion;

	if (sResult == SUCCESS)
	{
		rf.Write(ulFileVersion);
		rf.Write(sDifficulty);
		rf.Write((short)m_action);
		rf.Write(g_sRealmNumToSave);
		g_stockpile.Save(&rf);

		// new in version 48.
		rf.Write(Flag_Achievements);

		// new in version 49.
		rf.Write(playthroughMS);

		rf.Close();
	}

	return sResult;
}

////////////////////////////////////////////////////////////////////////////////
//
// Game_LoadPlayersGame - Load a previously saved game, restore the settings
//								  
////////////////////////////////////////////////////////////////////////////////

extern short Game_LoadPlayersGame(
				char* pszSaveName,		// In:  Name of the saved game file to open
				short* psDifficulty,		// Out: Saved game realm difficulty.
				ACTION* paction)			// Out: Saved game action.
{
	RFile rf;
	short sResult = rf.Open(pszSaveName, "rb", RFile::LittleEndian);
	ULONG ulFileVersion;

	if (sResult == SUCCESS)
	{
		rf.Read(&ulFileVersion);
		rf.Read(psDifficulty);
		// Store as 16 bit value (in case Read() fails (we want to keep original
		// functionality which read directly into m_action) ).
		short	sAction	= (short)*paction;
		// Read as 16 bit.
		rf.Read(&sAction);
		// Store as action.
		*paction			= (ACTION)sAction;

		rf.Read(&m_sRealmNum);
		g_stockpile.Load(&rf, ulFileVersion);

		// new in version 48.
		const bool usedCheats = ((Flag_Achievements & FLAG_USED_CHEATS) != 0);
		Flag_Achievements = 0;
		if (ulFileVersion >= 48)
			rf.Read(&Flag_Achievements);
		if (usedCheats)
			Flag_Achievements |= FLAG_USED_CHEATS;

		#if 0
		printf("achievements:\n");
		#define printflag(x) printf("%s: %s\n", #x, (Flag_Achievements & x) ? "true" : "false");
		printflag(FLAG_USED_M16);
		printflag(FLAG_USED_SHOTGUN);
		printflag(FLAG_USED_DBL_SHOTGUN);
		printflag(FLAG_USED_GRENADE);
		printflag(FLAG_USED_ROCKET);
		printflag(FLAG_USED_MOLOTOV);
		printflag(FLAG_USED_NAPALM);
		printflag(FLAG_USED_FLAMETHROWER);
		printflag(FLAG_USED_PROXIMITY_MINE);
		printflag(FLAG_USED_TIMED_MINE);
		printflag(FLAG_USED_REMOTE_MINE);
		printflag(FLAG_USED_BETTY_MINE);
		printflag(FLAG_USED_HEATSEEKER);
		printflag(FLAG_USED_SPRAY_CANNON);
		printflag(FLAG_USED_DEATHWAD);
		printflag(FLAG_USED_CHEATS);
		printflag(FLAG_KILLED_EVERYTHING);
		printflag(FLAG_KILLED_ONLY_HOSTILES);
		#undef printflag
		#endif

		// new in version 49.
		playthroughMS = -1;  // disable the achievement for old save games.
		if (ulFileVersion >= 49)
			rf.Read(&playthroughMS);

		rf.Close();

		g_bTransferStockpile = true;
	}

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
//
// GameEndingSequence - Start a demo of the last level
//
////////////////////////////////////////////////////////////////////////////////

void GameEndingSequence(void)
{
	// Prepare all settings for demo mode
	CSettings::PreDemo();

#if VIOLENT_LOCALE

	// If demo debug movie file is specified in prefs, open it now.  The RFile*
	// is does double-duty as a flag, where non-zero means movie mode is enabled.
	RFile* pfileDemoDebugMovie = 0;
	m_pfileRandom = 0;
	if (strlen(g_GameSettings.m_szDemoDebugMovie) > 0)
		{
		m_pfileRandom = new RFile;
		if (m_pfileRandom->Open(g_GameSettings.m_szDemoDebugMovie, "rb", RFile::LittleEndian) != 0)
			{
			delete m_pfileRandom;
			m_pfileRandom = 0;
			}
		}

	if (InputDemoInit() == 0)
		{
		// This is the special end of game demo, so set up the demo name
		sprintf(m_szDemoFile, "%s", FullPathCD(ENDING_DEMO_NAME));

		RFile	fileDemo;
		if (fileDemo.Open(m_szDemoFile, "rb", RFile::LittleEndian) == 0)
			{
			// Read name of realm file
			char szRealmFile[RSP_MAX_PATH];
			fileDemo.Read(szRealmFile);
			// Read whether it's a full path.
			short	sRealmFileIsFullPath;
			fileDemo.Read(&sRealmFileIsFullPath);
			if (!fileDemo.Error())
				{
				// Load input demo data (must be BEFORE setting playback mode)
				if (InputDemoLoad(&fileDemo) == 0)
					{
					// End menu (now that we know there were no errors)
//					StopMenu();
//					PalTranOff();
//					bMenuActive = false;

					Play(
						NULL,									// No client (not network game)
						NULL,									// No server (not network game)
						INPUT_MODE_PLAYBACK,				// Input mode
						-1,									// Always use specific realm file
						szRealmFile,						// Realm file to be played
						false,								// Don't play just one realm
						false,								// Not challenge mode
						false,								// Not new single player Add On levels
						GetGameDifficulty(),				// Difficulty level
						false,								// Rejunenate (MP only)
						0,										// Time limit (MP only)
						0,										// Kill limit (MP only)
						0,										// Cooperative (MP only)
						0,										// Use cooperative mode (MP only)
						0,										// Frame time (MP only)
						pfileDemoDebugMovie);			// Demo mode file
					}
				else
					{
					TRACE("GameCore(): Couldn't load demo data!\n");
					rspMsgBox(RSP_MB_ICN_STOP | RSP_MB_BUT_OK, g_pszAppName, g_pszFileReadError_s, (char*) m_szDemoFile);
					}
				}
			else
				{
				TRACE("GameCore(): Couldn't load realm name!\n");
				rspMsgBox(RSP_MB_ICN_STOP | RSP_MB_BUT_OK, g_pszAppName, g_pszFileReadError_s, (char*) m_szDemoFile);
				}
			fileDemo.Close();
			}
		else
			{
			TRACE("GameCore(): Couldn't open demo file: '%s'\n", m_szDemoFile);
			rspMsgBox(RSP_MB_ICN_STOP | RSP_MB_BUT_OK, g_pszAppName, g_pszFileOpenError_s, (char*) m_szDemoFile);
			}
		// Reset demo file name for next time.
		m_szDemoFile[0] = 0;
		InputDemoKill();
		}

	if (m_pfileRandom)
		{
		// File may have been closed by Play() if an error occurred
		if (m_pfileRandom->IsOpen())
			m_pfileRandom->Close();
		delete m_pfileRandom;
		m_pfileRandom = 0;
		}

#endif // VIOLENT_LOCALE

	// Restore settings to what they were before demo mode
	CSettings::PostDemo();

	SetInputMode(INPUT_MODE_LIVE);  // take it out of demo mode so achievements can unlock.

	// Do the ending title sequence.
	Title_GameEndSequence();

	// Do special credits:
	Credits(&g_smidFinalSceneCredits);

	UnlockAchievement(ACHIEVEMENT_COMPLETE_GAME);

	if (playthroughMS < MAX_PLAYTHROUGH_ACHIEVEMENT_MS)
		UnlockAchievement(ACHIEVEMENT_COMPLETE_GAME_IN_X_MINUTES);
	if ((Flag_Achievements & FLAG_MASK_WEAPONS) == FLAG_USED_M16)
		UnlockAchievement(ACHIEVEMENT_USE_ONLY_M16);
	if (Flag_Achievements & FLAG_KILLED_EVERYTHING)
		UnlockAchievement(ACHIEVEMENT_KILL_EVERYTHING);
	if (Flag_Achievements & FLAG_KILLED_ONLY_HOSTILES)
		UnlockAchievement(ACHIEVEMENT_KILL_ONLY_HOSTILES);
	if (Flag_Achievements & FLAG_HIGHEST_DIFFICULTY)
		UnlockAchievement(ACHIEVEMENT_COMPLETE_GAME_ON_HARDEST);
}

////////////////////////////////////////////////////////////////////////////////
// Returns a ptr to just the portion of the file path that specifies the file
// name (excluding the path).
////////////////////////////////////////////////////////////////////////////////
static char* GetFileNameFromPath(	// Returns file name.
	char*	pszFullPath)					// In:  File's full path.
	{
	// Scan back for the separator or the beginning.
	char*	pszIndex	= pszFullPath + (strlen(pszFullPath) - 1);

	while (pszIndex >= pszFullPath && *pszIndex != RSP_SYSTEM_PATH_SEPARATOR)
		{
		pszIndex--;
		}

	return (pszIndex + 1);
	}

////////////////////////////////////////////////////////////////////////////////
// Opens the synchronization log with the specified access flags if in a 
// TRACENASSERT mode and synchronization logging is enabled.
// Also, opens the random log, if it is enabled via 
// g_GameSettings.m_szDebugMovie or something.
////////////////////////////////////////////////////////////////////////////////
static void OpenSynchLogs(			// Returns nothing.
	bool	bWriteLogs)					// In:  true to create log, false to compare.
	{
	m_pfileRandom = 0;
	if (strlen(g_GameSettings.m_szDemoDebugMovie) > 0)
		{
		m_pfileRandom = new RFile;
		if (m_pfileRandom->Open(
				g_GameSettings.m_szDemoDebugMovie, 
				bWriteLogs ? "wb" : "rb", 
				RFile::LittleEndian) != 0)
			{
			delete m_pfileRandom;
			m_pfileRandom = 0;
			}
		}

	#if defined(_DEBUG) || defined(TRACENASSERT)
		ms_lSynchLogSeq	= 0;
		if (g_GameSettings.m_szSynchLogFile[0] != '\0')
			{
			if (ms_fileSynchLog.Open(
					g_GameSettings.m_szSynchLogFile, 
					bWriteLogs ? "wb" : "rb", 
					RFile::LittleEndian) == 0)
				{
				// Success.
				}
			else
				{
				TRACE("OpenSynchLogs(): Failed to open 'if' log file \"%s\".\n",
					g_GameSettings.m_szSynchLogFile);
				}
			}
	#endif	// defined(_DEBUG) || defined(TRACENASSERT)

	m_bWriteLogs	= bWriteLogs;
	}

////////////////////////////////////////////////////////////////////////////////
// Closes the synchronization logs, if open.
////////////////////////////////////////////////////////////////////////////////
static void CloseSynchLogs(void)	// Returns nothing.
	{
	if (m_pfileRandom)
		{
		// File may have been closed by Play() if an error occurred
		if (m_pfileRandom->IsOpen())
			m_pfileRandom->Close();
		delete m_pfileRandom;
		m_pfileRandom = 0;
		}

	#if defined(_DEBUG) || defined(TRACENASSERT)
		if (ms_fileSynchLog.IsOpen() )
			{
			ms_fileSynchLog.Close();
			}
	#endif	// defined(_DEBUG) || defined(TRACENASSERT)
	}

////////////////////////////////////////////////////////////////////////////////
// Synchronization logger -- Call this function to log an expression and a user
// value in the synch log.  When active (if g_GameSettings.m_szSynchLog is
// a valid path and filename), if recording a demo, these calls are logged to
// a file including the calling file and line number.  When active, if playing
// back a demo, these calls are compared to those stored in the log and, if
// a discrepancy occurs, a modal dialog box will pop up with the pertinent info
// followed by an ASSERT(0) for easy debugging.
////////////////////////////////////////////////////////////////////////////////
extern int SynchLog(	// Result of expr.
	double	expr,		// In:  Expression to evaluate.
	char*		pszFile,	// In:  Calling file.
	long		lLine,	// In:  Calling line.
	char*		pszExpr,	// In:  Original C++ source expression.
	U32		u32User)	// In:  A user value that is intended to be consistent.
	{
	#if defined(_DEBUG) || defined(TRACENASSERT)
		if (ms_fileSynchLog.IsOpen() )
			{
			if (m_bWriteLogs)
				{
				fprintf(
					ms_fileSynchLog.m_fs, 
					"[Seq: %ld] %s : %ld  <$%s$> == %0.8f; User == %lu\n", 
					ms_lSynchLogSeq++,
					GetFileNameFromPath(pszFile),
					lLine,
					pszExpr,
					expr,
					u32User);
				}
			else
				{
				char		szFileIn[RSP_MAX_PATH];
				char		szExprIn[1024];
				long		lLineIn;
				long		lSeqIn;
				double	exprIn;
				U32		u32UserIn;

				if (fscanf(
					ms_fileSynchLog.m_fs,
					"[Seq: %ld] %s : %ld  <$%1024[^$]$> == %g; User == %lu\n", 
					&lSeqIn,
					szFileIn,
					&lLineIn,
					szExprIn,
					&exprIn,
					&u32UserIn) == 6)
					{
					// Verify . . .
					if (	(rspStricmp(szFileIn, GetFileNameFromPath(pszFile) ) != 0)
						|| (lLineIn != lLine) 
						|| (exprIn != expr) 
						|| (u32UserIn != u32User) )
						{
						char	szOut[2048];
						sprintf(
							szOut,
							"'If' sequence (%ld) mismatch!\n\n"
							"   Was <<%s>> at %s(%ld) which got %g; User == %lu\n\n"
							"   Now <<%s>> at %s(%ld) which got %g; User == %lu",
							ms_lSynchLogSeq,
							szExprIn,
							szFileIn,
							lLineIn,
							exprIn,
							u32UserIn,
							pszExpr,
							GetFileNameFromPath(pszFile),
							lLine,
							expr,
							u32User);

						TRACE("%s\n\n", szOut);

						rspMsgBox(
							RSP_MB_ICN_INFO | RSP_MB_BUT_OK,
							"Postal",
							szOut);

						// Make this easy to debug
						ASSERT(0);
						}

					ms_lSynchLogSeq++;
					}
				else
					{
					TRACE("Synch(): Error reading log file.\n");
					}
				}
			}
#else
		rspMsgBox(
			RSP_MB_ICN_STOP | RSP_MB_BUT_OK,
			"Postal",
			"Synchronization logging is disabled for release mode for safety.\n");
#endif	// defined(_DEBUG) || defined(TRACENASSERT)

	return expr;
	}


////////////////////////////////////////////////////////////////////////////////
// GameGetRegistry
////////////////////////////////////////////////////////////////////////////////

static void GameGetRegistry(void)
{
#ifdef CHECK_EXPIRATION_DATE
	char szTime[40];
	char szTimeEncrypt[40];
	time_t lTime;
	DWORD dwTimeLength;
	DWORD dwSize = 255;
	char szData[256];
	char szName[256];
	short sEncryptedKeyLength = 36;

	unsigned char szKey[40];

	// This is the encoded path name of the registry key where the value is stored
	szKey[0] = 0x07;
	szKey[1] = 0x29;
	szKey[2] = 0xbd;
	szKey[3] = 0x3a;
	szKey[4] = 0xba;
	szKey[5] = 0x22;
	szKey[6] = 0xbe;
	szKey[7] = 0x36;
	szKey[8] = 0xf5;
	szKey[9] = 0x22;
	szKey[10] = 0xfb;
	szKey[11] = 0x24;
	szKey[12] = 0xd0;
	szKey[13] = 0x3a;
	szKey[14] = 0xd4;
	szKey[15] = 0x71;
	szKey[16] = 0xcb;
	szKey[17] = 0x3e;
	szKey[18] = 0xd5;
	szKey[19] = 0x2a;
	szKey[20] = 0xec;
	szKey[21] = 0x29;
	szKey[22] = 0x89;
	szKey[23] = 0x30;
	szKey[24] = 0x8f;
	szKey[25] = 0x78;
	szKey[26] = 0x83;
	szKey[27] = 0x66;
	szKey[28] = 0xb4;
	szKey[29] = 0x40;
	szKey[30] = 0x86;
	szKey[31] = 0x66;
	szKey[32] = 0x8f;
	szKey[33] = 0x71;
	szKey[34] = 0x9b;
	szKey[35] = 0xfe;
	szKey[36] = 0x00;

#ifdef WIN32

	DWORD dwDisposition;
	DWORD dwType;
	DWORD dwNameSize = 255;
	HKEY hkResult;
	long lError;
	short sEncryptedValueLength = 9;


	unsigned char szIn[10];

	// This is the encoded name of the registry value itendifier
	szIn[0] = 0x07;
	szIn[1] = 0x29;
	szIn[2] = 0xa8;
	szIn[3] = 0x0f;
	szIn[4] = 0xa7;
	szIn[5] = 0x0c;
	szIn[6] = 0xa8;
	szIn[7] = 0x0e;
	szIn[8] = 0xfd;
	szIn[9] = 0x00;


	// Get the current time and convert it to a string so it can be encoded
	time( &lTime );
	sprintf(szTime, "%ld", lTime);

	// Decrypte the registry key path so the key can be opened
	Decrypt((char*) szKey, szName, sEncryptedKeyLength);
	szName[sEncryptedKeyLength-2] = 0;
	lError = RegCreateKeyEx(HKEY_LOCAL_MACHINE, szName, 0,
	               "", 	REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
						&hkResult, &dwDisposition);
	// Destroy the source and result.
	memset(szName, 0xeb, sEncryptedKeyLength);

	if (lError == ERROR_SUCCESS && lTime > g_lReleaseTime)
	{
		if (dwDisposition == REG_CREATED_NEW_KEY)
		// Write the initial value
		{
			// Make sure current value is after release value
			dwTimeLength = Encrypt(szTime, szTimeEncrypt, strlen(szTime));
			Decrypt((char*) szIn, szName, sEncryptedValueLength);
			szName[sEncryptedValueLength-2] = 0;
			RegSetValueEx(hkResult, szName, 0, REG_BINARY, (unsigned char *) szTimeEncrypt, dwTimeLength); 
			memset(szName, 0xeb, sEncryptedValueLength);
			g_lRegTime = lTime;
		}
		else
		{
			lError = RegEnumValue(hkResult, 0, szName, &dwNameSize, 0, &dwType, (unsigned char *) szData, &dwSize);
			if (lError != ERROR_SUCCESS)
				g_lRegTime = EXPIRATION_DATE;
			else
			{
				if (Decrypt(szData, szTime, dwSize) == 0)
				{
					szTime[dwSize] = 0;
					g_lRegTime = atol(szTime);
				}
				else
				{
					g_lRegTime = EXPIRATION_DATE;
				}
			}
		}
		RegCloseKey(hkResult);
	}
	else
	// If there was an error opening the registry key, then it will trace the error
	// message here.
	{
		LPVOID lpMsgBuf;

		FormatMessage( 
			 FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			 NULL,
			 GetLastError(),
			 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			 (LPTSTR) &lpMsgBuf,
			 0,
			 NULL 
		);

		// Display the string.
		TRACE((char*) lpMsgBuf);

		// Free the buffer.
		LocalFree( lpMsgBuf );

		// This failed either because it couldn't write, or because
		// the date has been set back before the release date, so 
		// force this to fail
		g_lRegTime = EXPIRATION_DATE;
 	}

#else // WIN32
	// Do mac version here.
	char szPath[512];
	time( &lTime );
	lTime -= ((365 * 70UL) + 17) * 24 * 60 * 60; // time_fudge 1900->1970

	// Open a prefs file and read the value, or write the value
	// if it is the first time.
	RFile rfPref;
	// If the file does not exist, then write the current date to the file 
	// as long as it is after the release date.
	macGetSpecialPath(MacPreferencesFolderType, true, szPath, 254);
	Decrypt((char*) szKey, szName, sEncryptedKeyLength);
	sEncryptedKeyLength = MAX(0, sEncryptedKeyLength - 2);
	szName[sEncryptedKeyLength] = 0;
	strcat(szPath, szName+19);
	if (rfPref.Open(szPath, "rb", RFile::LittleEndian) != SUCCESS)
	{
		if (rfPref.Open(szPath, "wb", RFile::LittleEndian) != SUCCESS)
		{
			// If the file cannot be created, then hose the program
			g_lRegTime = EXPIRATION_DATE;
		}
		else
		{
			// If the date is set back before the release of the beta, then hose the
			// program
			g_lRegTime = lTime;
			if (lTime < g_lReleaseTime)
				g_lRegTime = lTime = EXPIRATION_DATE;
			dwTimeLength = Encrypt((char*) &lTime, szTimeEncrypt, 4);
			rfPref.Write(&dwTimeLength);
			rfPref.Write(szTime, dwTimeLength); 
			rfPref.Close();
		}
	}
	// If the prefs file is here, then open it and read the last date
	else
	{
		rfPref.Read(&dwSize);
		rfPref.Read(szData, dwSize);
		rfPref.Close();	
		
		if (Decrypt(szData, (char*) &lTime, dwSize) == 0)
		{
			g_lRegTime = lTime;
		}
		else
		{
			g_lRegTime = EXPIRATION_DATE;
		}
	}
	

#endif // WIN32
	if (g_lRegTime < RELEASE_DATE)
		g_lRegTime = EXPIRATION_DATE;

#else // CHECK_EXPIRATION_DATE
	g_lRegTime = 0;
#endif // CHECK_EXPIRATION_DATE
}

////////////////////////////////////////////////////////////////////////////////
// GameSetRegistry
////////////////////////////////////////////////////////////////////////////////

static void GameSetRegistry(void)
{
#ifdef CHECK_EXPIRATION_DATE
	char szTimeEncrypt[40];
	time_t lTime;
	DWORD dwTimeLength;
	char szName[256];
	short sEncryptedKeyLength = 36;
	unsigned char szKey[40];

	szKey[0] = 0x00;
	szKey[1] = 0x3e;
	szKey[2] = 0xde;
	szKey[3] = 0x28;
	szKey[4] = 0xda;
	szKey[5] = 0x46;
	szKey[6] = 0xdd;
	szKey[7] = 0x57;
	szKey[8] = 0xc2;
	szKey[9] = 0x34;
	szKey[10] = 0xc4;
	szKey[11] = 0x32;
	szKey[12] = 0xe8;
	szKey[13] = 0x3f;
	szKey[14] = 0xf8;
	szKey[15] = 0x2c;
	szKey[16] = 0xe8;
	szKey[17] = 0x3e;
	szKey[18] = 0xe8;
	szKey[19] = 0x2a;
	szKey[20] = 0xdc;
	szKey[21] = 0x03;
	szKey[22] = 0xff;
	szKey[23] = 0x14;
	szKey[24] = 0xfb;
	szKey[25] = 0x18;
	szKey[26] = 0xfe;
	szKey[27] = 0x1f;
	szKey[28] = 0xc9;
	szKey[29] = 0x35;
	szKey[30] = 0xe0;
	szKey[31] = 0x02;
	szKey[32] = 0xea;
	szKey[33] = 0x09;
	szKey[34] = 0xf1;
	szKey[35] = 0xf5;
	szKey[36] = 0x00;

#ifdef WIN32

	char szTime[40];
	DWORD dwDisposition;
	DWORD dwSize = 255;
	DWORD dwNameSize = 255;
	HKEY hkResult;
	long lError;
	short sEncryptedValueLength = 9;

	unsigned char szIn[10];

	szIn[0] = 0x07;
	szIn[1] = 0x29;
	szIn[2] = 0xa8;
	szIn[3] = 0x0f;
	szIn[4] = 0xa7;
	szIn[5] = 0x0c;
	szIn[6] = 0xa8;
	szIn[7] = 0x0e;
	szIn[8] = 0xfd;
	szIn[9] = 0x00;


	time( &lTime );
	lTime = MAX(lTime, g_lRegTime);
	sprintf(szTime, "%ld", lTime);

	Decrypt((char*) szKey, szName, sEncryptedKeyLength);
	szName[sEncryptedKeyLength-2] = 0;
	lError = RegCreateKeyEx(HKEY_LOCAL_MACHINE, szName, 0,
	               "", 	REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
						&hkResult, &dwDisposition);
	memset(szName, 0xea, sEncryptedKeyLength);

	if (lError == ERROR_SUCCESS)
	{
		// Make sure current value is after release value
		dwTimeLength = Encrypt(szTime, szTimeEncrypt, strlen(szTime));
		Decrypt((char*) szIn, szName, sEncryptedValueLength);
		szName[sEncryptedValueLength-2] = 0;
		RegSetValueEx(hkResult, szName, 0, REG_BINARY, (unsigned char *) szTimeEncrypt, dwTimeLength); 
		memset(szIn, 0xee, sEncryptedValueLength);
		RegCloseKey(hkResult);
	}
	else
	{
		LPVOID lpMsgBuf;

		FormatMessage( 
			 FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			 NULL,
			 GetLastError(),
			 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			 (LPTSTR) &lpMsgBuf,
			 0,
			 NULL 
		);

		// Display the string.
		TRACE((char*) lpMsgBuf);

		// Free the buffer.
		LocalFree( lpMsgBuf );
 	}
#else // WIN32
	// Do the Mac version here
	char szPath[512];
	RFile rfPref;
	time( &lTime );
	lTime -= ((365 * 70UL) + 17) * 24 * 60 * 60; // time_fudge 1900->1970

	// Make sure the time is after the last registered time.
	lTime = MAX(lTime, (time_t) g_lRegTime);
 	dwTimeLength = Encrypt((char*) &lTime, szTimeEncrypt, 4);
	macGetSpecialPath(MacPreferencesFolderType, true, szPath, 254);
	Decrypt((char*) szKey, szName, sEncryptedKeyLength);
	sEncryptedKeyLength = MAX(0, sEncryptedKeyLength - 2);
	szName[sEncryptedKeyLength] = 0;
	strcat(szPath, szName+19);
	
 	if (rfPref.Open(szPath, "wb", RFile::LittleEndian) == SUCCESS)
 	{
 		rfPref.Write(&dwTimeLength);
 		rfPref.Write(szTimeEncrypt, dwTimeLength);
		rfPref.Close(); 	
 	}
	// Open the prefs file and write the value here.

#endif // WIN32

#endif // CHECK_EXPIRATION_DATE
}


////////////////////////////////////////////////////////////////////////////////
//
// Seed the random number generator
//
////////////////////////////////////////////////////////////////////////////////
extern void SeedRand(
	long lSeed)
	{
	m_lRandom = lSeed;
	}


////////////////////////////////////////////////////////////////////////////////
//
// Get a random number
//
////////////////////////////////////////////////////////////////////////////////
#if defined(_DEBUG) || defined(TRACENASSERT)
	extern long GetRandomDebug(char* FILE_MACRO, long LINE_MACRO)
		{
		// Get next random number
		long lNewVal = (((m_lRandom = m_lRandom * 214013L + 2531011L) >> 16) & 0x7fff);

		if (m_pfileRandom)
			{
			if (m_bWriteLogs)
				{
				fprintf(
					m_pfileRandom->m_fs,
					"%s : %ld rand = %ld\n", 
					GetFileNameFromPath(FILE_MACRO),
					LINE_MACRO,
					lNewVal);
//				m_pfileRandom->Write(lNewVal);
//				m_pfileRandom->Write(LINE_MACRO);
//				m_pfileRandom->Write(FILE_MACRO);
				}
			else
				{
				long lSavedVal;
				long lSavedLine;
				char szSavedFile[1024];
				fscanf(
					m_pfileRandom->m_fs,
					"%s : %ld rand = %ld\n", 
					szSavedFile,
					&lSavedLine,
					&lSavedVal);
//				m_pfileRandom->Read(&lSavedVal);
//				m_pfileRandom->Read(&lSavedLine);
//				m_pfileRandom->Read(szSavedFile);

				if ((lSavedVal != lNewVal) || (lSavedLine != LINE_MACRO) || (rspStricmp(szSavedFile, GetFileNameFromPath(FILE_MACRO) ) != 0))
					{
					rspMsgBox(
						RSP_MB_ICN_INFO | RSP_MB_BUT_OK,
						"Postal",
						"Random number sequence mismatch!\n\n"
						"   Was %s(%ld) which got %ld\n\n"
						"   Now %s(%ld) which got %ld",
						szSavedFile,
						(long)lSavedLine,
						(long)lSavedVal,
						GetFileNameFromPath(FILE_MACRO),
						LINE_MACRO,
						(long)lNewVal);

					// Make this easy to debug
					ASSERT(0);
					}
				}
			}
		return lNewVal;
		}
#else
	extern long GetRandom(void)
		{
		// Get next random number
		return (((m_lRandom = m_lRandom * 214013L + 2531011L) >> 16) & 0x7fff);
		}
#endif	// defined(_DEBUG) || defined(TRACENASSERT).


////////////////////////////////////////////////////////////////////////////////
//
// Define a rand() to take the place of the stdlib version.  We don't want
// anyone calling that version!
//
////////////////////////////////////////////////////////////////////////////////
// It seems that this only works in debug mode.  In release, the linker
// generates a fatal "multiple definitions of rand()" error.  For now, only
// do this in debug mode.
#ifdef _DEBUG
#ifndef PLATFORM_UNIX
extern int rand(void)
	{
	rspMsgBox(
		RSP_MB_ICN_INFO | RSP_MB_BUT_OK,
		"Postal",
		"The stdlib version of rand() was called.  It is forbidden within this application.");
	return 0;
	}
#endif
#endif


////////////////////////////////////////////////////////////////////////////////
//
// Do palette transition so menu can be displayed on top of existing background.
// NOTE: There must be a matching PalTranOff() for each PalTranOn()!!!
//
////////////////////////////////////////////////////////////////////////////////
extern void PalTranOn(
	long lTime /* = -1 */)								// In:  How long transition should take (or -1 for default)
	{
	if (lTime == -1)
		lTime = NORMAL_PAL_TRAN_TIME;

	StartMenuTrans(lTime);

	// Do until done or keypress.
	while (DoPreMenuTrans() == false)
		UpdateSystem();
	}


////////////////////////////////////////////////////////////////////////////////
//
// Undo the palette transition to restore the original background colors.
//
////////////////////////////////////////////////////////////////////////////////
extern void PalTranOff(void)
	{
	// Do until done
	while (DoPostMenuTrans() == false)
		UpdateSystem();

	// Let paltran finish/clean up.
	EndMenuTrans(true);
	}


////////////////////////////////////////////////////////////////////////////////
//
// Set gamma/brighten-effect value.  Note that there is no value that results
//	in identity.  See Brightness / contrast below:
// RIGHT NOW THIS FUNCTION IS BEING USED TO CALL BRIGHTNESS CONTRAST:
//
////////////////////////////////////////////////////////////////////////////////
extern void SetGammaLevel(	// Returns nothing.
	short sBase)				// New brighten value.
	{
	// For the time being, use this to control the other function:

	// Pick a contrast based on the chosen brightness.
	double dBrightness = double(sBase)/128.0 - 1.0;

	// Allow a larger range:
	dBrightness *= 2.0;

	// This was no clip
	//double dContrast = MIN(1.0 - dBrightness, dBrightness + 1.0);
	// Let's link contrast with brightness and clip:
	double dContrast = (dBrightness + 1.0); // needs to be a value of 1 to be neutral

	// With this scheme, contrast can never be more than +1.0 or less than 0.0
	SetBrightnessContrast(dBrightness,dContrast);
	// Update settings.
	g_GameSettings.m_sGammaVal	= sBase;

	return; // don't set gamma for now

	U8	au8RedMap[256];
	U8	au8GreenMap[256];
	U8	au8BlueMap[256];

	short i;
	short	sClipVal;
	for (	i = 0;
			i < 256; 
			i++)
		{
		sClipVal	= MAX((short)0, MIN(short(pow((double)i / 100.0, GAMMA_EXPONENT) * sBase), (short)255));
		au8RedMap[i]	= (U8)sClipVal;
		au8GreenMap[i]	= (U8)sClipVal;
		au8BlueMap[i]	= (U8)sClipVal;
		}

	// Update map.
	rspSetPaletteMaps(
		0,
		256,
		au8RedMap,
		au8GreenMap,
		au8BlueMap,
		sizeof(U8));

	// Update hardware through new map.
	rspUpdatePalette();

	// Update settings.
	g_GameSettings.m_sGammaVal	= sBase;
	}

////////////////////////////////////////////////////////////////////////////////
//
//	Set Brightness and Contrast.  Zero (neutral) values yield and identity
// curve.  Valid input is from -1 to 1.
//
////////////////////////////////////////////////////////////////////////////////
extern	void	SetBrightnessContrast(
						double dBrightness,	// -1.0 = dim, 0.0 = normal, 1.0 = bright
						double dContrast		// -1.0 = low contrast, 0.0 = normal, 1.0 = high
						)
	{
	U8	au8RedMap[256];
	U8	au8GreenMap[256];
	U8	au8BlueMap[256];

	// I will scale the ranges to within reasonable limits:
	ASSERT( (dBrightness >= -1.0) || (dBrightness <= 1.0));
	//ASSERT( (dContrast >= -1.0) || (dContrast <= 1.0));

	//dContrast = dContrast + 1.0; // this IS the tangent value (0-2)

	short i;
	double dScale = 1.0/128.0;
	for (i=0;i < 256; i++)
		{
		double dX = dScale * i - 1.0;
		short sLev = short(128.0 + 128.0 * (
			 ( ( ( ((1.0 - dContrast) * dX) - dBrightness) * dX) + dContrast) * dX
			 + dBrightness));

		if (sLev < 0) sLev = 0;
		if (sLev > 255) sLev = 255;

		au8RedMap[i]	= (U8)sLev;
		au8GreenMap[i]	= (U8)sLev;
		au8BlueMap[i]	= (U8)sLev;
		}

	// Update map.
	rspSetPaletteMaps(
		0,
		256,
		au8RedMap,
		au8GreenMap,
		au8BlueMap,
		sizeof(U8));

	// Update hardware through new map.
	rspUpdatePalette();
	//----------------------------------
	// set postal levels?
	}


////////////////////////////////////////////////////////////////////////////////
//
// Get gamma/brighten-effect value from palette map (not from settings).
//
////////////////////////////////////////////////////////////////////////////////
extern short GetGammaLevel(void)	// Returns current brighten value.
	{
	return g_GameSettings.m_sGammaVal;
	}


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

// This string is much larger than it needs to be in case the string
// size changes as a result of the conversion to system format, in which
// case our tests would miss it until it's too late.  A massive overrun
// will still cause a problem, but that would never happen :)
static char m_acFullPath[RSP_MAX_PATH + RSP_MAX_PATH];

extern char* FullPath(									// Returns full path in system format
	short sPathType,										// In:  PATH_CD, PATH_HD, or PATH_VD
	char* pszPartialPath)								// In:  Partial path in RSPiX format
	{
	// Start with the specified base path (copy the string from the game settings)
	if (sPathType == GAME_PATH_CD)
		return FullPathCD(pszPartialPath);
	else if (sPathType == GAME_PATH_HD)
		return FullPathHD(pszPartialPath);
	else if (sPathType == GAME_PATH_VD)
		return FullPathVD(pszPartialPath);
	else if (sPathType == GAME_PATH_SOUND)
		return FullPathSound(pszPartialPath);
	else if (sPathType == GAME_PATH_GAME)
		return FullPathGame(pszPartialPath);
	else if (sPathType == GAME_PATH_HOODS)
		return FullPathHoods(pszPartialPath);
	else 
		{
		TRACE("FullPath(): Unknown path type: %d -- I predict an ASSERT() will occur soon...\n", (short)sPathType);
		ASSERT(1);

		// In case they want to ignore the assert, just return a pointer to an empty string
		m_acFullPath[0] = 0;
		return m_acFullPath;
		}
	}


extern char* FullPathCD(								// Returns full path in system format
	char* pszPartialPath)								// In:  Partial path in RSPiX format
	{
	// Start with proper base path
	ASSERT(strlen(g_GameSettings.m_pszCDPath) < RSP_MAX_PATH);
	strcpy(m_acFullPath, g_GameSettings.m_pszCDPath);
	// Make sure partial path isn't too long.  It is possible that the conversion
	// to the system format will change its length slightly, but it shouldn't be
	// enough to make a real difference to this test.
	ASSERT(strlen(pszPartialPath) < RSP_MAX_PATH);

	// Check if the combination of the partial and base path will be too long.
	ASSERT((strlen(pszPartialPath) + strlen(m_acFullPath)) < RSP_MAX_PATH);

	// Convert specified partial path from rspix to system format, putting the
	// result immediately following the base path
	rspPathToSystem(pszPartialPath, m_acFullPath + strlen(m_acFullPath));

	// Make sure result isn't too long (our buffer can handle it, but it's still a problem)
	ASSERT(strlen(m_acFullPath) < RSP_MAX_PATH);

	// Return pointer to full path
	return m_acFullPath;
	}


extern char* FullPathHD(								// Returns full path in system format
	const char* pszPartialPath)								// In:  Partial path in RSPiX format
	{
	// Start with proper base path
	ASSERT(strlen(g_GameSettings.m_pszHDPath) < RSP_MAX_PATH);
	strcpy(m_acFullPath, g_GameSettings.m_pszHDPath);
	// Make sure partial path isn't too long.  It is possible that the conversion
	// to the system format will change its length slightly, but it shouldn't be
	// enough to make a real difference to this test.
	ASSERT(strlen(pszPartialPath) < RSP_MAX_PATH);

	// Check if the combination of the partial and base path will be too long.
	ASSERT((strlen(pszPartialPath) + strlen(m_acFullPath)) < RSP_MAX_PATH);

	// Convert specified partial path from rspix to system format, putting the
	// result immediately following the base path
	rspPathToSystem(pszPartialPath, m_acFullPath + strlen(m_acFullPath));

	// Make sure result isn't too long (our buffer can handle it, but it's still a problem)
	ASSERT(strlen(m_acFullPath) < RSP_MAX_PATH);

	// Return pointer to full path
	return m_acFullPath;
	}


extern char* FullPathVD(								// Returns full path in system format
	char* pszPartialPath)								// In:  Partial path in RSPiX format
	{
	// Start with proper base path
	ASSERT(strlen(g_GameSettings.m_pszVDPath) < RSP_MAX_PATH);
	strcpy(m_acFullPath, g_GameSettings.m_pszVDPath);
	// Make sure partial path isn't too long.  It is possible that the conversion
	// to the system format will change its length slightly, but it shouldn't be
	// enough to make a real difference to this test.
	ASSERT(strlen(pszPartialPath) < RSP_MAX_PATH);

	// Check if the combination of the partial and base path will be too long.
	ASSERT((strlen(pszPartialPath) + strlen(m_acFullPath)) < RSP_MAX_PATH);

	// Convert specified partial path from rspix to system format, putting the
	// result immediately following the base path
	rspPathToSystem(pszPartialPath, m_acFullPath + strlen(m_acFullPath));

	// Make sure result isn't too long (our buffer can handle it, but it's still a problem)
	ASSERT(strlen(m_acFullPath) < RSP_MAX_PATH);

	// Return pointer to full path
	return m_acFullPath;
	}


extern char* FullPathSound(								// Returns full path in system format
	char* pszPartialPath)								// In:  Partial path in RSPiX format
	{
	// Start with proper base path
	ASSERT(strlen(g_GameSettings.m_pszSoundPath) < RSP_MAX_PATH);
	strcpy(m_acFullPath, g_GameSettings.m_pszSoundPath);
	// Make sure partial path isn't too long.  It is possible that the conversion
	// to the system format will change its length slightly, but it shouldn't be
	// enough to make a real difference to this test.
	ASSERT(strlen(pszPartialPath) < RSP_MAX_PATH);

	// Check if the combination of the partial and base path will be too long.
	ASSERT((strlen(pszPartialPath) + strlen(m_acFullPath)) < RSP_MAX_PATH);

	// Convert specified partial path from rspix to system format, putting the
	// result immediately following the base path
	rspPathToSystem(pszPartialPath, m_acFullPath + strlen(m_acFullPath));

	// Make sure result isn't too long (our buffer can handle it, but it's still a problem)
	ASSERT(strlen(m_acFullPath) < RSP_MAX_PATH);

	// Return pointer to full path
	return m_acFullPath;
	}


extern char* FullPathGame(								// Returns full path in system format
	char* pszPartialPath)								// In:  Partial path in RSPiX format
	{
	// Start with proper base path
	ASSERT(strlen(g_GameSettings.m_pszGamePath) < RSP_MAX_PATH);
	strcpy(m_acFullPath, g_GameSettings.m_pszGamePath);
	// Make sure partial path isn't too long.  It is possible that the conversion
	// to the system format will change its length slightly, but it shouldn't be
	// enough to make a real difference to this test.
	ASSERT(strlen(pszPartialPath) < RSP_MAX_PATH);

	// Check if the combination of the partial and base path will be too long.
	ASSERT((strlen(pszPartialPath) + strlen(m_acFullPath)) < RSP_MAX_PATH);

	// Convert specified partial path from rspix to system format, putting the
	// result immediately following the base path
	rspPathToSystem(pszPartialPath, m_acFullPath + strlen(m_acFullPath));

	// Make sure result isn't too long (our buffer can handle it, but it's still a problem)
	ASSERT(strlen(m_acFullPath) < RSP_MAX_PATH);

	// Return pointer to full path
	return m_acFullPath;
	}

extern char* FullPathHoods(								// Returns full path in system format
	char* pszPartialPath)								// In:  Partial path in RSPiX format
	{
	// Start with proper base path
	ASSERT(strlen(g_GameSettings.m_pszHoodsPath) < RSP_MAX_PATH);
	strcpy(m_acFullPath, g_GameSettings.m_pszHoodsPath);
	// Make sure partial path isn't too long.  It is possible that the conversion
	// to the system format will change its length slightly, but it shouldn't be
	// enough to make a real difference to this test.
	ASSERT(strlen(pszPartialPath) < RSP_MAX_PATH);

	// Check if the combination of the partial and base path will be too long.
	ASSERT((strlen(pszPartialPath) + strlen(m_acFullPath)) < RSP_MAX_PATH);

	// Convert specified partial path from rspix to system format, putting the
	// result immediately following the base path
	rspPathToSystem(pszPartialPath, m_acFullPath + strlen(m_acFullPath));

	// Make sure result isn't too long (our buffer can handle it, but it's still a problem)
	ASSERT(strlen(m_acFullPath) < RSP_MAX_PATH);

	// Return pointer to full path
	return m_acFullPath;
	}


extern char* FullPathCustom(							// Returns full path in system format
	char*	pszFullPath,									// In:  Full path in in RSPiX format.
	char* pszPartialPath)								// In:  Partial path in RSPiX format.
	{
	char*	pszFullSystemPath	= rspPathToSystem(pszFullPath);
	// Start with proper base path
	ASSERT(strlen(pszFullSystemPath) < RSP_MAX_PATH);
	strcpy(m_acFullPath, pszFullSystemPath);

	// Make sure partial path isn't too long.  It is possible that the conversion
	// to the system format will change its length slightly, but it shouldn't be
	// enough to make a real difference to this test.
	ASSERT(strlen(pszPartialPath) < RSP_MAX_PATH);

	// Check if the combination of the partial and base path will be too long.
	ASSERT((strlen(pszPartialPath) + strlen(m_acFullPath)) < RSP_MAX_PATH);

	// Convert specified partial path from rspix to system format, putting the
	// result immediately following the base path
	rspPathToSystem(pszPartialPath, m_acFullPath + strlen(m_acFullPath));

	// Make sure result isn't too long (our buffer can handle it, but it's still a problem)
	ASSERT(strlen(m_acFullPath) < RSP_MAX_PATH);

	// Return pointer to full path
	return m_acFullPath;
	}


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
	short sMaxPathLen)									// In:  Maximum length of base path
	{
	short sResult = 0;

	// Make sure they aren't passing an empty string, which should be be left alone
	if (strlen(pszBasePath) > 0)
		{

		// Make sure they aren't passing a string that's already too large!
		if ((strlen(pszBasePath) + 1) <= sMaxPathLen)
			{

			//------------------------------------------------------------------------------
			// Convert path from relative to absolute.  We do this by setting the current
			// directory to the specified path and then asking for the current directory,
			// which is always returned as an absolute path.  If the path is already
			// absolute to begin with, it *should* be unchanged as a result.
			//------------------------------------------------------------------------------

			// Get original directory (let it allocate buffer of at LEAST specified size, more if needed)
			
			// 09/05/97, PPL
			// Apparently, the getcwd function works completely (well, maybe not completely) different
			// on the MAC.  We need to go ahead and allocate our own buffer and free it later because
			// if a NULL is passed in, a NULL is returned.  This should not have any adverse effects
			// on the PC version.
			//char* pszOrigDir = getcwd(NULL, RSP_MAX_PATH);
			char* pszOrigDir = (char*)malloc(RSP_MAX_PATH);
			if (pszOrigDir != NULL)
				{
				// Let's go ahead and get the current working directory here, once we're sure that
				// the string to store it has been properly allocated.
				pszOrigDir = getcwd(pszOrigDir, RSP_MAX_PATH);

				// Change to specified directory, which may be relative or absolute
				if (chdir(pszBasePath) == 0)
					{
					// Get directory, which is always returned as absolute
					if (getcwd(pszBasePath, sMaxPathLen) == NULL)
						{
						sResult = -1;
						TRACE("CorrectifyBasePath(): Couldn't get current directory (was set to '%s')!\n", pszBasePath);
						}
					}
				else
					{
					sResult = -1;
					TRACE("CorrectifyBasePath(): Couldn't change to specified directory: '%s'!\n", pszBasePath);
					}

				// Restore original directory
				if (chdir(pszOrigDir) != 0)
					{
					sResult = -1;
					TRACE("CorrectifyBasePath(): Couldn't restore original directory: '%s'!\n", pszOrigDir);
					}

				// Free buffer that was allocated by getcwd()
				free(pszOrigDir);
				pszOrigDir = 0;
				}

			//------------------------------------------------------------------------------
			// Ensure that path ends properly, either with or without the system-specific
			// separator character, depending on which system we're on.
			//------------------------------------------------------------------------------
			if (sResult == 0)
				{
				// Get index to last character
				short sLastIndex = strlen(pszBasePath);
				if (sLastIndex > 0)
					sLastIndex--;

				#if 1 //def WIN32
					// If base path doesn't end with a slash, add one
					if (pszBasePath[sLastIndex] != RSP_SYSTEM_PATH_SEPARATOR)
						{
						if ((sLastIndex + 2) < RSP_MAX_PATH)
							{
							pszBasePath[sLastIndex+1] = RSP_SYSTEM_PATH_SEPARATOR;
							pszBasePath[sLastIndex+2] = 0;
							}
						else
							{
							sResult = -1;
							TRACE("CorrectifyBasePath(): Path would've exceed max length with separator tacked on!\n");
							}
						}
				#else
					// If base path ends with a colon, get rid of it
					if (pszBasePath[sLastIndex] == RSP_SYSTEM_PATH_SEPARATOR)
						pszBasePath[sLastIndex] = 0;
				#endif
				}
			}
		else
			{
			sResult = -1;
			TRACE("CorrectifyBasePath(): Specified path is already longer than the specified maximum length!\n");
			}
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
//
// Notification that we are about to be in the background.
//
////////////////////////////////////////////////////////////////////////////////
static void BackgroundCall(void)
	{
	// TRACE("Background\n");

	// Return CPU to OS.
	rspSetDoSystemMode(RSP_DOSYSTEM_SLEEP);

	// If cursor show level not already stored . . .
	if (ms_sForegroundCursorShowLevel == INVALID_CURSOR_SHOW_LEVEL)
		{
		// Store current cursor show level and then make cursor visible
		ms_sForegroundCursorShowLevel	= rspGetMouseCursorShowLevel();
		rspSetMouseCursorShowLevel(1);
		}
	}


////////////////////////////////////////////////////////////////////////////////
//
// Notification that we are about to be in the foreground.
//
////////////////////////////////////////////////////////////////////////////////
static void ForegroundCall(void)
	{
	// TRACE("Foreground\n");

	#if defined(_DEBUG)
		// Wake CPU.
		rspSetDoSystemMode(RSP_DOSYSTEM_TOLERATEOS);
	#else
		// Return CPU to us.
		rspSetDoSystemMode(RSP_DOSYSTEM_HOGCPU);
	#endif


	// If cursor show level stored . . .
	if (ms_sForegroundCursorShowLevel != INVALID_CURSOR_SHOW_LEVEL)
		{
		// Restore previous cursor show level
		rspSetMouseCursorShowLevel(ms_sForegroundCursorShowLevel);
		ms_sForegroundCursorShowLevel	= 0;

		// Reset the input.
		ClearLocalInput();

		// Flag that we're no longer storing the show level.
		ms_sForegroundCursorShowLevel = INVALID_CURSOR_SHOW_LEVEL;
		}
	}


////////////////////////////////////////////////////////////////////////////////
// Returns difficulty for games.
// Note that this function is only valid once after a difficulty adjustment
// and then it goes back to the default (g_GameSettings value).
////////////////////////////////////////////////////////////////////////////////
static short GetGameDifficulty(void)	// Returns cached game difficulty.
	{
	short sDifficulty	= g_GameSettings.m_sDifficulty;

	// If there is a cached difficulty . . .
	if (ms_sLoadedDifficulty != INVALID_DIFFICULTY)
		{
		// Return cached difficulty.
		sDifficulty	= ms_sLoadedDifficulty;

		// Clear cache.
		ms_sLoadedDifficulty = INVALID_DIFFICULTY;
		}

	return sDifficulty;
	}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
