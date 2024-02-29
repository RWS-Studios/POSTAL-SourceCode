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
// play.cpp
// Project: Nostril (aka Postal)
//
// This module deals with the high-level aspects of setting up and running the
// game.
//
// History:
//		11/19/96 MJR	Started.
//
//		A huge number of changes occurred, and then the entire module was
//		reorganized, to the point where the previous history was no longer
//		relevant.  This history was was purged on 8/3/97 -- if you need to
//		refer back to it, simply go back before this date in SourceSafe.
//
//		08/03/97 MJR	A total reorganization occurs.
//
//		08/05/97	JMI	Changed uses of CRealm::m_bMultiplayer to 
//							CRealm::m_flags.bMultiplayer.
//
//		08/06/97 MJR	Fixed bug when going to next level or restarting.
//
//		08/06/97	JMI	Now Play_VerifyQuitMenuChoice() plays the appropriate sound
//							as to whether there was a selection change or an item was
//							chosen.
//							Also, changed uses of InitLocalInput() to ClearLocalInput().
//
//		08/08/97 MJR	Moved background/foreground callbacks to game.cpp.
//
//		08/08/97 MJR	Fixed multiplayer go-to-next-level bug.
//							Got the abort message working properly.
//
//		08/08/97	JMI	CPlayRealm::EndRealm() now only updates players stockpiles
//							if we are not restarting the current level.  This way, in
//							single player mode, when we restart the level, you don't
//							get a combo of the ammo you had when you died and the warp
//							but rather a combo of the ammo you had when you entered the
//							level and the warp.
//
//		08/08/97	JMI	After a realm play when 'Just one realm' was specified,
//							the 'Game Over' flag would get set regardless of whether
//							the player had chosen to restart the realm.  Fixed.
//
//		08/09/97	JMI	CoreLoopRender() and CoreLoopUserInput() were checking
//							m_bCheckForAbortKey without first checking if we're in
//							network mode.  This flag is not used in non-net mode so we
//							must check before using it.
//
//		08/09/97	JRD	Changed play to call the new toolbar render, and modified the
//							score render to include the background bitmap.
//
//		08/11/97	JMI	Changed two occurrences of sRealNum to info.m_sRealNum.
//							sRealmNum is the passed in start realm and info.m_sRealNum
//							is the current realm.
//
//		08/11/97 MJR	Fixed a bug where time wasn't being updated properly (and
//							thereby was at least one reason for sync problems.)
//
//		08/12/97	JMI	Now that cheats require an input event, we only pass it to
//							GetLocalInput() in singel player mode.  Since the two ways
//							of getting input for are so different, it makes it difficult
//							to hack cheats into multiplayer mode.
//
//		08/13/97 MJR	Cleaned up use of info flags to try to simplify and
//							make sure no race conditions exist.
//
//							Fixed bug when trying to resume paused game (wasn't
//							filtering out modifier keys -- now it does).
//
//		08/13/97	JMI	Fixed positioning macros so they are nearly constant (i.e,
//							changes in the g_pimScreenBuf could cause it to be non-
//							constant).
//							Fixed portions of the code that updated the realm status
//							using the INFO_STATUS_* macros.
//							Moved the initial drawing of the toolbar into 
//							CPlayStatus::StartRealm().
//							Now utilizes the return value from ToolbarRender() to de-
//							termine whether to update that area of the display.
//
//		08/14/97	JMI	Took 'again' out of "Hit <pause> key again to resume"
//							paused message.
//							Also, RespondToMenuRequest() now clears all events before
//							starting menu.
//							Made XRay All key a toggle.
//							Changed name of difficulty parameter to Play() from 
//							bDifficulty to sDifficulty.
//							Now uses sDifficulty paramter to Play().
//							Added sDifficulty paramter to 
//							Play_GetRealmSectionAndEntry().
//							Converted ms_bQuitVerified to ms_menuaction and added two
//							actions:  MenuActionQuit and MenuActionSaveGame.
//							Now passes difficulty to Game_SavePlayersGame() which is
//							now called from RespondToMenuRequest().
//
//		08/14/97	JMI	Converted Play_VerifyQuitMenuChoice() to returning true to
//							accept or false to deny.
//
//		08/17/97	JMI	Now disables postal organ option from within the game.
//
//		08/17/97 MJR	Now loads abort gui from g_resmgrShell.
//
//		08/18/97	JMI	Was still clearing KEY_RESTART as a left over from when we
//							would use KEY_RESTART to flag restarting a level in single
//							player (nowadays uses INPUT_REVIVE).
//							Also, was able to get rid of INPUT_JUMP which was left over
//							from when we converted to INPUT_REVIVE but play.cpp was
//							under different construction.
//
//		08/18/97	JMI	Now turns on XRay all when the local dude dies.
//
//		08/18/97	JMI	Added variable that, when true, allows advancing to the next
//							level without meeting the level goal.
//							Also, now in multiplayer mode, the server can advance the
//							level without meeting the level goal.
//
//		08/19/97 MJR	Added supoprt for new MP parameters.
//
//		08/20/97	JMI	Now responds to INPUT_CHEAT_29 by advancing the level if
//							NOT a sales demo.
//
//		08/20/97 BRH	In the Play function, I used the flags passed in to
//							determine and set the scoring mode in the realm.
//
//		08/21/97	JMI	Now keeps the global savable stockpile up to date.
//
//		08/21/97	JMI	Changed call to Update() to UpdateSystem() and occurrences
//							of rspUpdateDisplay() to UpdateDisplay().
//
//		08/22/97	JMI	Changed calls to UpdateDisplay() back to rspUpdateDisplay()
//							since we no longer need UpdateDisplay() now that we are
//							using rspLock/Unlock* functions properly.
//							Also, now locks the composite buffer before accessing it
//							and unlocks it before updating the screen.  This required
//							breaking CoreLoopRender() into CoreLoopRender() and 
//							CoreLoopDraw().
//
//		08/23/97	JMI	Now 'Save' menu option is disabled in multiplayer mode.
//
//		08/24/97	JMI	Added a timeout to the abortion of playing samples just in
//							case there's a bug or a sound driver problem (no need to
//							to taunt infinite loopage).
//
//		08/24/97	JMI	Moved code to stop all samples into a function so we could
//							call it in two places.
//							Now used before starting the load b/c playing samples sound
//							too shitty during loads.
//
//		08/24/97	JMI	Check for INPUT_CHEAT_29 was incorrectly using 
//							INPUT_CHEAT_29 as a mask instead of INPUT_WEAPONS_MASK so
//							other cheats that included all the same mask bits could
//							cause 29 to be activated (there was only one, of course,
//							INPUT_CHEAT_30).
//
//		08/25/97	JMI	Now uses toolbar initialized score font colors for debug
//							display info text.
//
//		08/26/97 BRH	Added special cases for the final ending demo level.  
//							Now when it is determined that the player won, it sets
//							the global g_bLastLevelDemo so that the ending demo will
//							be shown after the final game level which is the air
//							force base.  Also made a few special cases so that the
//							Cutscene shown is the one loaded from the RealmEnd section
//							of the realms.ini file, and that the toolbars are not
//							shown during the final level demo.  
//
//		08/26/97	JMI	Moved m_bXRayAll to CPlayInfo so it could be accessed from
//							anywhere.
//							Fixed problem where, when you come back to life in MP mode
//							or via cheat, the XRay would stay on even if the user
//							setting was off.
//
//		08/27/97	JMI	Changed PAUSED_FONT_HEIGHT to 48 (was 50).  Apparently, we
//							cannot use a size that is larger than the largest cached
//							font size.  So all font sizes for the Smash font must be
//							less than or equal to 48.
//
//		08/27/97 MJR	Updated to use new union name in NetMsg.
//							Now sets dude ID for all players in MP mode.
//							Now sends and receives special peer data.
//
//		08/28/97 MJR	Merged CPlayClient and CPlayServer into CPlayNet.
//
//		xx/xx/97 MJR	HUGE CHANGES to incorporate new network scheme.
//
//							==========================================================
//		09/05/97 MJR	MERGED ALL THE CHANGES FROM THE SEPARATE BRANCH OF PLAY.CPP
//							WHICH IS WHERE THE FOLLOWING CHANGES CAME FROM
//							==========================================================
//
//		08/30/97 BRH	Fixed paths for installer.  The levels were still trying
//							to load from the HD path but they should load from the CD
//							path.
//
//		08/30/97	JMI	If the player hits space to restart, we check if the goal
//							was met and, if so, show the high score dialogs.
//
//		09/02/97	JMI	Now Purges all resources from g_resmgrGame, Samples, and
//							Res on certain systems.
//
//		09/03/97	JMI	I realized that the last change would cause an 
//							unnecessarily long load for restarting a realm so now it
//							only does the purging (on the MAC) if we're not restarting
//							the realm.
//
//		09/03/97	JMI	Changed the check for the end of the demo to use IsDead()
//							instead of State_Dead for determining whether the dude is
//							dead.  Also, now checks InputIsDemoOver().
//
//		09/03/97	JMI	Now checks to make sure we're in SP mode before pausing
//							while in the background.
//
//		09/04/97 BRH	Play no longer sets the full path to the realm file to
//							load.  It is done in Realm::Load instead so that we can
//							try several paths.  This way the realms can be loaded
//							from the HD path, or if not there, loaded from the CD
//							path.  Then if someone wants to insert their level, or
//							we want to provide an updated level, they can copy it
//							to the mirror path on their HD and it will attempt to
//							load that one first.
//
//							==========================================================
//							Finished merging separate branches of PLAY.CPP.
//							==========================================================
//
//		09/06/97 MJR	Fixed bug in SetupDudes() that caused crash in single
//							player mode.
//
//		09/06/97 MJR	Now allows menu to be used in MP mode.
//							Cleaned up how local user quits are handled in MP mode.
//							Properly uses abort gui thing.
//
//		09/07/97	JMI	Now displays the high scores at the end of each MP level.
//							Also, now defaults to 99 (instead of 10) kills when neither
//							a time or a kill limit is specified.
//
//		09/07/97 MJR	Fixed bug that prevented end-of-game sequence from working.
//							Now ignores keyboard input during end-of-game sequence.
//
//		09/08/97 MJR	Centered net prog gui thingy.
//
//		09/11/97	JMI	Added support for ENABLE_PLAY_SPECIFIC_REALMS_ONLY which
//							only allows you to play a realm whose name is jumbled in
//							ms_szSingleRealmPostFix[].
//
//		09/12/97 MJR	In MP game, if a realm can't be loaded, we either abort
//							the game if we're the server or we drop out of the game
//							if we're a client.
//
//							Also removed the ASSERT() from CInfo.GameOver(), which
//							used to not get called in MP mode, but now does due to
//							our sudden use of "just one realm" mode in cases where
//							the server only has one realm available.
//
//		09/16/97 MJR	Removed the JUMBLE stuff, which was made obsolete when we
//							switched to embedding the realm files in the executable.
//
//		09/29/97	JMI	Now updates areas of the display that were blanked by 
//							ScaleFilm() (called from CPlayRealm::CoreLoopRender() ) in 
//							CPlayInfo::UpdateBlankedAreas() (called from 
//							CPlayRealm::CoreLoopDraw() ).  Since, when ScaleFilm() is
//							called, we are inside a rspLock/UnlockBuffer() pair, we
//							cannot call rspUpdateDisplay() there.
//
//		10/30/97	JMI	Used to use a flag to indicate whether CInfo::m_rc* needed
//							to be updated.  Now we simply check whether m_rc*.sW & sH
//							are greater than 0 so we need to make sure they're 
//							initialized to zero.  It didn't show up on the PC b/c Blue
//							does not allow negative widths/heights to be drawn but on
//							the Mac it seems to cause a rather bizarre mess.
//
//		11/19/97	JMI	The m_bDrawFrame flag was not being set to false when 
//							bDoFrame (in CPlayRealm::CoreLoopRender() ) was false.  The
//							result was that while a net game was idle of input, the
//							display was still being updated.  Once this was changed and
//							m_bDrawFrame was moved into CPlayInfo (so all CPlayXxxx's
//							could utilize it), the idle looping increased in speed by
//							approximately 10 times on my machine.  The next logical
//							step would be to use this flag to reduce the number of
//							calls to ToolBarRender() and ScoreUpdateDisplay().  There's
//							a possible order problem with simply checking m_bDrawFrame
//							since it is set to false or true in 
//							CPlayRealm::CoreLoopRender() and ToolBarRender() and
//							ScoreUpdateDisplay() are called in 
//							CPlayStatus::CoreLoopRender().
//
//		11/20/97	JMI	Added net chat and dirty rects.  Now most things don't have to 
//							bother implementing an CoreLoopRender() just for the sake of   
//							updating an area they already processed.  Now, in              
//							CoreLoopRender(), just do a pinfo->m_drl.Add(x, y, w, h) of the
//							area dirtied and it will be combined with everyone else's area 
//							and updated to the screen (usually in one chunk if the film    
//							size has not been altered).
//							More testing needs to be done, though.  Playing against all
//							P200s, the game ran fine.  But with a P120, it ran poorly.
//							We only tried once though...not sure there's really a 
//							problem (also the P120 was the only machine with Win95...).
//
//		11/20/97	JMI	Added bCoopLevels & bCoopMode parameters to 
//							Play_GetRealmInfo() and Play_GetRealmSectionAndEntry() 
//							calls.
//							Also, added sCoopLevels & sCoopMode to Play() call.
//							Also, fixed a bug in Play_GetRealmInfo() where it would
//							write one byte off the end of the pszTitle parameter.
//
//		11/25/97	JMI	Changed the chats' .GUIs to be loaded from the HD
//							instead of from the VD so we can guarantee the new assets
//							get loaded (since they'll use their old Postal disc, we
//							cannot load the .GUIs from the CD).
//
//		06/04/98 BRH	Set the cutscene mode to simple mode if this is a spawn
//							build, since the spawn version only has 1 default cutscene
//							bitmap, it has to use this for all cutscenes.
//
//		10/07/99	JMI	Changed play loop to get the number of single player levels
//							from the INI.  Previously, it was 16.
//
////////////////////////////////////////////////////////////////////////////////
#define PLAY_CPP

#include "RSPiX.h"
#include "main.h"
#include "input.h"
#include "game.h"
#include "update.h"
#include "realm.h"
#include "camera.h"
#include "grip.h"
#include "thing.h"
#include "dude.h"
#include "hood.h"
#include "input.h"
#include "menus.h"
#include "SampleMaster.h"
#include "reality.h"
#include "NetDlg.h"
#include "cutscene.h"
#include "play.h"
#include "warp.h"
#include "scene.h"
#include "score.h"
#include "person.h"
#include "InputSettingsDlg.h"
#include "toolbar.h"
#include "title.h"
#include "credits.h"
#ifdef WIN32
	#include "log.h"
#endif

#if defined(WIN32)
	// For file timestamp.
	#include <windows.h>
	#include <time.h>
	#include <sys/types.h>
	#include <sys/stat.h>
#endif

#if WITH_STEAMWORKS
#include "steam/steam_api.h"
#endif

//#define RSP_PROFILE_ON
//#include "ORANGE/Debug/profile.h"


////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////

#define DEMO_FRAMES_PER_SECOND			15
#define DEMO_TIME_PER_FRAME				(1000 / DEMO_FRAMES_PER_SECOND)
#define DEMO_MAX_SEQUENTIAL_SKIPPED_FRAMES	1
#define DEMO_MAX_LAG							(DEMO_TIME_PER_FRAME / 2)
#define DEMO_MAX_DEAD_TIME					5000

#define DEMO_MULTIALPHA_FILE				"2d/school.mlp"

#define DISP_INFO_INTERVAL					1000	// NEVER EVER MAKE THIS LESS THAN 1!!!!
#define DISP_INFO_FONT_HEIGHT				15

#define VIEW_X									0
#define VIEW_Y									0 
#define VIEW_W									wideScreenWidth
#define VIEW_H									400

#define FILM_X									0
#define FILM_Y									40

// Scaling values
#define FILM_INCDEC_SCALE					0.05
#define FILM_MAX_SCALE						1.00
#define FILM_MIN_SCALE						0.30

#define INFO_STATUS_RECT_X					((VIEW_W - 640)/2)
#define INFO_STATUS_RECT_Y					(FILM_Y - (INFO_STATUS_RECT_H + 3) )
#define INFO_STATUS_RECT_W					(g_pimScreenBuf->m_sWidth - INFO_STATUS_RECT_X)
#define INFO_STATUS_RECT_H					DISP_INFO_FONT_HEIGHT

#define DUDE_STATUS_RECT_X					0
#define DUDE_STATUS_RECT_Y					(FILM_Y + VIEW_H)
#define DUDE_STATUS_RECT_W					(g_pimScreenBuf->m_sWidth - DUDE_STATUS_RECT_X)
#define DUDE_STATUS_RECT_H					(g_pimScreenBuf->m_sHeight - DUDE_STATUS_RECT_Y)

#define REALM_STATUS_RECT_X				0
#define REALM_STATUS_RECT_Y				0
#define REALM_STATUS_RECT_W				(FILM_X + VIEW_W - REALM_STATUS_RECT_X)
#define REALM_STATUS_RECT_H				40

// No less than this even after scaling.
#define MIN_GRIP_ZONE_RADIUS				30

// Grip movement parameters
#define GRIP_MIN_MOVE_X						1
#define GRIP_MIN_MOVE_Y						1
#define GRIP_MAX_MOVE_X						8
#define GRIP_MAX_MOVE_Y						8
#define GRIP_ALIGN_X							1
#define GRIP_ALIGN_Y							1

// Time for black screen between cutscene and game screen
#define BLACK_HOLD_TIME						250

// Default message in case app's time stamp is not available.  MUST be 25 characters or less!!!
#define DEFAULT_APP_TIMESTAMP				"No time stamp available"

#define DEBUG_STR								" Debug"
#define RELEASE_STR							" Release"
#define TRACENASSERT_STR					" Trace & Assert"

// Number of kills limit if they specified no kills limit and no time limit.
#define KILLS_LIMIT_DEFAULT				0

// Default value for "final frame" in network mode (6.8 years at 10fps)
#define DEFAULT_FINAL_FRAME				LONG_MAX

#if WITH_STEAMWORKS
extern bool EnableSteamCloud;
#define SAVEGAME_DIR						(EnableSteamCloud ? "steamcloud" : "savegame")
#else
#define SAVEGAME_DIR						("savegame")
#endif

#define SAVEGAME_EXT							"gme"

#define ABORT_GUI_FILE						"menu/abort.gui"
#define CHAT_GUI								"res/shell/chat.gui"
#define CHAT_IN_GUI							"res/shell/chatin.gui"

#define KEY_MENU								27

#define KEY_PAUSE							RSP_GK_PAUSE
#define KEY_NEXT_LEVEL						RSP_GK_F1
#define KEY_TOGGLE_TARGETING				RSP_GK_F2
// NOTE THAT F3 IS IN USE: DONT USE RSP_GK_F3.
#define KEY_TOGGLE_DISP_INFO				RSP_GK_F4
#define KEY_SHOW_MISSION					RSP_GK_F5
#define KEY_ENLARGE_FILM1					RSP_GK_NUMPAD_PLUS
#define KEY_ENLARGE_FILM2					'+'
#define KEY_ENLARGE_FILM3					'='
#define KEY_REDUCE_FILM1					RSP_GK_NUMPAD_MINUS
#define KEY_REDUCE_FILM2					'-'
#define KEY_TALK1								'T'
#define KEY_TALK2								't'
#define KEY_ACCEPT_CHAT						'\r'
#define KEY_ABORT_CHAT						27

// Note that this uses RSP_SK_* macros for use the rspGetKeyStatusArray() key interface.
#define KEY_XRAY_ALL							RSP_SK_F3
#define KEY_SNAP_PICTURE					RSP_SK_ENTER

#define PAUSED_FONT_HEIGHT					48
#define PAUSED_FONT_SHADOW_X				5								// In pixels.
#define PAUSED_FONT_SHADOW_Y				PAUSED_FONT_SHADOW_X		// In pixels.
#define PAUSED_BASE_PAL_INDEX				64
#define PAUSED_FONT_SHADOW_COLOR_R		0
#define PAUSED_FONT_SHADOW_COLOR_G		0
#define PAUSED_FONT_SHADOW_COLOR_B		0
#define PAUSED_FONT_COLOR_R				0
#define PAUSED_FONT_COLOR_G				15
#define PAUSED_FONT_COLOR_B				255
											
#define PAUSED_MSG_FONT_HEIGHT			29
#define PAUSED_MSG_FONT_SHADOW_X			3									// In pixels.
#define PAUSED_MSG_FONT_SHADOW_Y			PAUSED_MSG_FONT_SHADOW_X	// In pixels.
#define PAUSED_MSG_FONT_SHADOW_COLOR_R	PAUSED_FONT_SHADOW_COLOR_R
#define PAUSED_MSG_FONT_SHADOW_COLOR_G	PAUSED_FONT_SHADOW_COLOR_G
#define PAUSED_MSG_FONT_SHADOW_COLOR_B	PAUSED_FONT_SHADOW_COLOR_B
#define PAUSED_MSG_FONT_COLOR_R			PAUSED_FONT_COLOR_R
#define PAUSED_MSG_FONT_COLOR_G			PAUSED_FONT_COLOR_G
#define PAUSED_MSG_FONT_COLOR_B			PAUSED_FONT_COLOR_B

#define TIME_OUT_FOR_ABORT_SOUNDS		3000	// In ms.

#define MP_HIGH_SCORES_MAX_TIME			7000	// In ms.

#define NUM_CHATS								4
#define CHAT_DELAY							5000	// In ms.

#define CHAT_IN_LENGTH						46

////////////////////////////////////////////////////////////////////////////////
// Types.
////////////////////////////////////////////////////////////////////////////////

// Game states
typedef enum
	{
	// These are defined in a SPECIFIC ORDER!!!  We sometimes check for specific
	// values, but other times we check for less than or greater than a value!!!
	// Note that you can think of the values as a PROGRESSION of states.
	Game_Ok,					// Base state, must be 0
	Game_RedoRealm,		// Redo the current realm
	Game_NextRealm,		// Go to the next realm
	Game_GameOver,			// Game is over
	Game_GameAborted,		// Game is over because user aborted it
	} GameState;

// Menu actions
typedef enum
	{
	MenuActionNone,
	MenuActionQuit,		// Quit game.
	MenuActionSaveGame,	// Save user's game.
	MenuActionEndMenu		// End the menu
	} MenuAction;

////////////////////////////////////////////////////////////////////////////////
// Variables/data
////////////////////////////////////////////////////////////////////////////////

// Quit flag used by menu callbacks
static MenuAction ms_menuaction	= MenuActionNone;

// Number used in filename for snapshots
static long ms_lCurPicture = 0;

#ifdef SALES_DEMO
	// When true, one can advance to the next level without meeting the goal.
	extern bool g_bEnableLevelAdvanceWithoutGoal	= false;
#endif

extern SampleMaster::SoundInstance g_siFinalScene; // should be in game
extern SampleMaster::SoundInstance g_siFinalSceneCredits; // should be in game
SampleMaster::SoundInstance g_siFinalScene; // should be in game
SampleMaster::SoundInstance g_siFinalSceneCredits; // should be in game

//#ifdef MOBILE
extern bool demoCompat; //Try to make demos not go out of sync
//#endif
////////////////////////////////////////////////////////////////////////////////
// Function prototypes
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//
// Info needed by virtually everything in play
//
////////////////////////////////////////////////////////////////////////////////
class CPlayInfo
	{
	friend short Play(										// Returns 0 if successfull, non-zero otherwise
		CNetClient*	pclient,									// In:  Client object or NULL if not network game
		CNetServer*	pserver,									// In:  Server object or NULL if not server or not network game
		INPUT_MODE inputMode,								// In:  Input mode
		const short sRealmNum,								// In:  Realm number to start on or -1 to use specified realm file
		const char*	pszRealmFile,							// In:  Realm file to play (ignored if sRealmNum >= 0)
		const bool bJustOneRealm,							// In:  Play just this one realm (ignored if sRealmNum < 0)
		const bool bGauntlet,								// In:  Play challenge levels gauntlet - as selected on menu
		const bool bAddOn,									// In:  Play new single player Add On levels
		const short sDifficulty,							// In:  Difficulty level
		const bool bRejuvenate,								// In:  Whether to allow players to rejuvenate (MP only)
		const short sTimeLimit,								// In:  Time limit for MP games (0 or negative if none)
		const short sKillLimit,								// In:  Kill limit for MP games (0 or negative if none)
		const	short	sCoopLevels,							// In:  Zero for deathmatch levels, non-zero for cooperative levels.
		const	short	sCoopMode,								// In:  Zero for deathmatch mode, non-zero for cooperative mode.
		const short sFrameTime,								// In:  Milliseconds per frame (MP only)
		RFile* pfileDemoModeDebugMovie);					// In:  File for loading/saving demo mode debug movie

	//------------------------------------------------------------------------------
	// Types, enums, etc.
	//------------------------------------------------------------------------------

	//------------------------------------------------------------------------------
	// Variables
	//------------------------------------------------------------------------------
	private:
		CNetClient*		m_pclient;						// Client object or NULL if not network game
		CNetServer*		m_pserver;						// Server object or NULL if not server or not network game

		short				m_sRealmNum;					// Realm number
		char				m_szRealm[RSP_MAX_PATH+1];	// Realm file
		bool				m_bJustOneRealm;				// Play just this one realm (ignored if sRealmNum < 0)

		CRealm*			m_prealm;
		CCamera*			m_pcamera;
		CGrip*			m_pgrip;

		bool				m_bGauntlet;					// Play challenge levels gauntlet
		bool				m_bAddOn;						// Play new Add On levels
		bool				m_bRejuvenate;					// Whether to allow players to rejuvenate (MP only)
		short				m_sTimeLimit;					// Time limit for MP games (0 or negative if none)
		short				m_sKillLimit;					// Kill limit for MP games (0 or negative if none)
		short				m_sCoopLevels;					// Zero for deathmatch levels, non-zero for cooperative levels.

		short				m_sFrameTime;					// Milliseconds per frame (MP only)

		RFile*			m_pfileDemoModeDebugMovie;	// File for loading/saving demo mode debug movie

		GameState		m_gamestate;

		bool				m_bPurgeSaks;					// Purge the SAKS if true


	public:
		U16				m_idLocalDude;					// Local dude's ID
		U16				m_idGripTarget;				// Grip target's ID
		bool				m_bDoRealmFrame;				// Whether to do a realm frame
		long				m_lSumUpdateDisplayTimes;
		bool				m_bXRayAll;						// X Ray all status.
		bool				m_bInMenu;						// Whether we're in the menu
		bool				m_bUserQuitMP;					// Whether local user wants to quit MP game
		bool				m_bNextRealmMP;				// Whether local user wants next level of MP game
		bool				m_bBadRealmMP;					// Whether MP realm was unable to load
		bool				m_bChatting;					// true, when typing in chat messages.
																// false, otherwise.
		bool				m_bDrawFrame;					// true, if we need to draw a frame.
		RDirtyRects		m_drl;							// Any areas of the composite buffer that is
																// altered should be added to this list so it can
																// be updated on CoreLoopDraw().


	//------------------------------------------------------------------------------
	// Functions
	//------------------------------------------------------------------------------
	public:
		////////////////////////////////////////////////////////////////////////////////
		// Constructor
		////////////////////////////////////////////////////////////////////////////////
		CPlayInfo(void)
			{
			m_pclient = 0;
			m_pserver = 0;
			
			m_sRealmNum = 0;
			m_szRealm[0] = 0;
			m_bJustOneRealm = false;
			
			m_prealm = new CRealm;
			m_pcamera = new CCamera;
			m_pgrip = new CGrip;
			
			m_bGauntlet = false;
			m_bAddOn = false;
			m_bRejuvenate = false;
			m_sTimeLimit = 0;
			m_sKillLimit = 0;
			m_sCoopLevels = 0;

			m_sFrameTime = 0;

			m_pfileDemoModeDebugMovie = 0;

			m_gamestate = Game_Ok;

			m_idLocalDude = CIdBank::IdNil;
			m_idGripTarget = CIdBank::IdNil;
			m_bDoRealmFrame = false;
			m_lSumUpdateDisplayTimes = 0;
			m_bXRayAll	= false;		// Always default to no XRay all.
			m_bPurgeSaks = false;	// Assume no purging
			m_bInMenu = false;
			m_bUserQuitMP = false;
			m_bNextRealmMP = false;
			m_bBadRealmMP = false;

			m_bChatting		= false;
			}


		////////////////////////////////////////////////////////////////////////////////
		// Destructor
		////////////////////////////////////////////////////////////////////////////////
		~CPlayInfo()
			{
			delete m_prealm;
			delete m_pcamera;
			delete m_pgrip;
			}


		////////////////////////////////////////////////////////////////////////////////
		// Simple wrappers that allow "read-only" access to member variables
		////////////////////////////////////////////////////////////////////////////////
		CNetClient*	Client(void)					{ return m_pclient; }
		CNetServer*	Server(void)					{ return m_pserver; }
		short			RealmNum(void)					{ return m_sRealmNum; }
		const char*	RealmName(void)				{ return m_szRealm; }
		bool			JustOneRealm(void)			{ return m_bJustOneRealm; }
		CRealm*		Realm(void)						{ return m_prealm; }
		CCamera*		Camera(void)					{ return m_pcamera; }
		CGrip*		Grip(void)						{ return m_pgrip; }
		bool			Gauntlet(void)					{ return m_bGauntlet; }
		bool			AddOn(void)						{ return m_bAddOn; }
		bool			Rejuvenate(void)				{ return m_bRejuvenate; }
		short			TimeLimit(void)				{ return m_sTimeLimit > 0 ? m_sTimeLimit : 0; }
		short			KillLimit(void)				{ return m_sKillLimit > 0 ? m_sKillLimit : 0; }
		short			CoopLevels(void)				{ return m_sCoopLevels; }
		short			FrameTime(void)				{ return m_sFrameTime; }
		RFile*		DemoModeDebugMovie(void)	{ return m_pfileDemoModeDebugMovie; }


		////////////////////////////////////////////////////////////////////////////////
		// Change the frame time (MP only)
		////////////////////////////////////////////////////////////////////////////////
		void SetFrameTime(
			short sFrameTime)
			{
			m_sFrameTime = sFrameTime;
			}


		////////////////////////////////////////////////////////////////////////////////
		// Set the SAK purge flag.
		////////////////////////////////////////////////////////////////////////////////
		void SetPurgeSaks(void)
			{ m_bPurgeSaks	= true; }

		////////////////////////////////////////////////////////////////////////////////
		// Clear the SAK purge flag.
		////////////////////////////////////////////////////////////////////////////////
		void ClearPurgeSaks(void)
			{ m_bPurgeSaks	= false; }

		////////////////////////////////////////////////////////////////////////////////
		// Query the SAK purge flag status.
		////////////////////////////////////////////////////////////////////////////////
		bool PurgeSaks(void)
			{ return m_bPurgeSaks; }


		////////////////////////////////////////////////////////////////////////////////
		// Get pointer to local dude if one exists, otherwise returns 0.
		////////////////////////////////////////////////////////////////////////////////
		CDude* LocalDudePointer(void)
			{
			CDude* pdudeLocal;
			if (m_prealm->m_idbank.GetThingByID((CThing**)&pdudeLocal, m_idLocalDude) != 0)
				m_idLocalDude = CIdBank::IdNil;
			return pdudeLocal;
			}


		////////////////////////////////////////////////////////////////////////////////
		// Query the game mode
		////////////////////////////////////////////////////////////////////////////////
		bool IsMP(void)
			{ return (m_pclient) ? true : false; }

		bool IsServer(void)
			{ return (m_pserver) ? true : false; }

		////////////////////////////////////////////////////////////////////////////////
		// Set the game state
		////////////////////////////////////////////////////////////////////////////////
		void SetGameState_Ok(void)
			{
			m_gamestate = Game_Ok;
			}

		void SetGameState_RestartRealm(void)
			{
			// This should NEVER occur in MP mode
			ASSERT(!IsMP());
			m_gamestate = Game_RedoRealm;
			}

		void SetGameState_NextRealm(
			bool bServerToldMe = false)
			{
			m_gamestate = Game_NextRealm;
			}

		void SetGameState_GameOver(
			bool bServerToldMe = false)
			{
			// This should NEVER occur in MP mode
			// 09/12/97 MJR -- This USED TO BE TRUE, but now that we re-enabled the
			// use of the "just one realm" mode in MP in the case where the server
			// only has a single realm available, we need this again in MP mode,
			// so I commented it out.
			//	ASSERT(!IsMP());
			m_gamestate = Game_GameOver;
			}

		void SetGameState_GameAborted(
			bool bServerToldMe = false)
			{
			m_gamestate = Game_GameAborted;
			}


		////////////////////////////////////////////////////////////////////////////////
		// Query the game state
		////////////////////////////////////////////////////////////////////////////////
		bool IsRealmDone(void)
			{ return (m_gamestate >= Game_RedoRealm) ? true : false; }

		bool IsRestartingRealm(void)
			{ return (m_gamestate == Game_RedoRealm) ? true : false; }

		bool IsNextRealm(void)
			{ return (m_gamestate == Game_NextRealm) ? true : false; }

		bool IsGameOver(void)
			{ return (m_gamestate >= Game_GameOver) ? true : false; }

		bool IsGameAborted(void)
			{ return (m_gamestate == Game_GameAborted) ? true : false; }

	};


////////////////////////////////////////////////////////////////////////////////
//
// Base class for all "Play Modules"
//
////////////////////////////////////////////////////////////////////////////////
class CPlay
	{
	//------------------------------------------------------------------------------
	// Types, enums, etc.
	//------------------------------------------------------------------------------
	private:

	//------------------------------------------------------------------------------
	// Variables
	//------------------------------------------------------------------------------
	private:

	//------------------------------------------------------------------------------
	// Functions
	//------------------------------------------------------------------------------
	public:
		////////////////////////////////////////////////////////////////////////////////
		// Constructor
		////////////////////////////////////////////////////////////////////////////////
		CPlay(void)
			{
			}


		////////////////////////////////////////////////////////////////////////////////
		// Destructor
		////////////////////////////////////////////////////////////////////////////////
		virtual
		~CPlay()
			{
			}


		////////////////////////////////////////////////////////////////////////////////
		// Prepare game
		////////////////////////////////////////////////////////////////////////////////
		virtual
		short PrepareGame(										// Returns 0 if successfull, non-zero otherwise
			CPlayInfo* pinfo)										// I/O: Play info
			{
			return 0;
			}


		////////////////////////////////////////////////////////////////////////////////
		// Determine if game is ready
		////////////////////////////////////////////////////////////////////////////////
		virtual
		short IsGameReady(										// Returns 0 if successfull, non-zero otherwise
			CPlayInfo* pinfo,										// I/O: Play info
			bool* pbGameReady)									// Out: Whether game is ready
			{
			*pbGameReady = true;
			return 0;
			}


		////////////////////////////////////////////////////////////////////////////////
		// Start game
		////////////////////////////////////////////////////////////////////////////////
		virtual
		short StartGame(											// Returns 0 if successfull, non-zero otherwise
			CPlayInfo* pinfo)										// I/O: Play info
			{
			return 0;
			}


		////////////////////////////////////////////////////////////////////////////////
		// Start cutscene
		////////////////////////////////////////////////////////////////////////////////
		virtual
		void StartCutscene(
			CPlayInfo* pinfo)										// I/O: Play info
			{
			}


		////////////////////////////////////////////////////////////////////////////////
		// Prepare realm
		////////////////////////////////////////////////////////////////////////////////
		virtual
		short PrepareRealm(										// Returns 0 if successfull, non-zero otherwise
			CPlayInfo* pinfo)										// I/O: Play info
			{
			return 0;
			}


		////////////////////////////////////////////////////////////////////////////////
		// Determine if realm is ready
		////////////////////////////////////////////////////////////////////////////////
		virtual
		short IsRealmReady(										// Returns 0 if successfull, non-zero otherwise
			CPlayInfo* pinfo,										// I/O: Play info
			bool* pbRealmReady)									// Out: Whether realm is ready
			{
			*pbRealmReady = true;
			return 0;
			}


		////////////////////////////////////////////////////////////////////////////////
		// Do cutscene
		////////////////////////////////////////////////////////////////////////////////
		virtual
		void DoCutscene(
			CPlayInfo* pinfo)										// I/O: Play info
			{
			}


		////////////////////////////////////////////////////////////////////////////////
		// End cutscene
		////////////////////////////////////////////////////////////////////////////////
		virtual
		void EndCutscene(
			CPlayInfo* pinfo)										// I/O: Play info
			{
			}


		////////////////////////////////////////////////////////////////////////////////
		// Start realm
		////////////////////////////////////////////////////////////////////////////////
		virtual
		short StartRealm(											// Returns 0 if successfull, non-zero otherwise
			CPlayInfo* pinfo)										// I/O: Play info
			{
			return 0;
			}


		////////////////////////////////////////////////////////////////////////////////
		// Core loop user input
		////////////////////////////////////////////////////////////////////////////////
		virtual
		void CoreLoopUserInput(
			CPlayInfo* pinfo,										// I/O: Play info
			RInputEvent* pie)										// I/O: Input event
			{
			}


		////////////////////////////////////////////////////////////////////////////////
		// Core loop update
		////////////////////////////////////////////////////////////////////////////////
		virtual
		void CoreLoopUpdate(
			CPlayInfo* pinfo)										// I/O: Play info
			{
			}


		////////////////////////////////////////////////////////////////////////////////
		// Core loop render -- create and update images to the composite buffer but do
		// NOT update the screen.
		////////////////////////////////////////////////////////////////////////////////
		virtual
		void CoreLoopRender(
			CPlayInfo* pinfo)										// I/O: Play info
			{
			}


		////////////////////////////////////////////////////////////////////////////////
		// Core loop render on top -- create and update images to the composite buffer 
		// on top of things rendered in CoreLoopRender() but do NOT update the screen.
		////////////////////////////////////////////////////////////////////////////////
		virtual
		void CoreLoopRenderOnTop(
			CPlayInfo* pinfo)										// I/O: Play info
			{
			}


		////////////////////////////////////////////////////////////////////////////////
		// Core loop draw -- Draw CoreLoopRender[OnTop]() results to the screen.
		////////////////////////////////////////////////////////////////////////////////
		virtual
		void CoreLoopDraw(
			CPlayInfo* pinfo)										// I/O: Play info
			{
			}


		////////////////////////////////////////////////////////////////////////////////
		// Determine if core loop is done
		////////////////////////////////////////////////////////////////////////////////
		virtual
		bool IsCoreLoopDone(										// Returns true if done, false otherwise
			CPlayInfo* pinfo)										// I/O: Play info
			{
			return pinfo->IsRealmDone();
			}


		////////////////////////////////////////////////////////////////////////////////
		// End realm
		////////////////////////////////////////////////////////////////////////////////
		virtual
		void EndRealm(
			CPlayInfo* pinfo)										// I/O: Play info
			{
			}


		////////////////////////////////////////////////////////////////////////////////
		// Unprepare game
		////////////////////////////////////////////////////////////////////////////////
		virtual
		void UnprepareGame(
			CPlayInfo* pinfo)										// I/O: Play info
			{
			}


		////////////////////////////////////////////////////////////////////////////////
		// Start realm error handler
		////////////////////////////////////////////////////////////////////////////////
		virtual
		void StartRealmErr(
			CPlayInfo* pinfo)										// I/O: Play info
			{
			}


		////////////////////////////////////////////////////////////////////////////////
		// Is realm ready error handler
		////////////////////////////////////////////////////////////////////////////////
		virtual
		void IsRealmReadyErr(
			CPlayInfo* pinfo)										// I/O: Play info
			{
			}


		////////////////////////////////////////////////////////////////////////////////
		// Prepare realm error handler
		////////////////////////////////////////////////////////////////////////////////
		virtual
		void PrepareRealmErr(
			CPlayInfo* pinfo)										// I/O: Play info
			{
			}


		////////////////////////////////////////////////////////////////////////////////
		// Start game error handler
		////////////////////////////////////////////////////////////////////////////////
		virtual
		void StartGameErr(
			CPlayInfo* pinfo)										// I/O: Play info
			{
			}


		////////////////////////////////////////////////////////////////////////////////
		// Is game ready error handler
		////////////////////////////////////////////////////////////////////////////////
		virtual
		void IsGameReadyErr(
			CPlayInfo* pinfo)										// I/O: Play info
			{
			}


		////////////////////////////////////////////////////////////////////////////////
		// Prepare game error handler
		////////////////////////////////////////////////////////////////////////////////
		virtual
		void PrepareGameErr(
			CPlayInfo* pinfo)										// I/O: Play info
			{
			}
	};


////////////////////////////////////////////////////////////////////////////////
//
// Group of "Play Modules"
//
////////////////////////////////////////////////////////////////////////////////
class CPlayGroup
	{
	//------------------------------------------------------------------------------
	// Types, enums, etc.
	//------------------------------------------------------------------------------
	public:
		typedef RFList<CPlay*> Plays;

	//------------------------------------------------------------------------------
	// Variables
	//------------------------------------------------------------------------------
	private:
		Plays		m_Plays;									// List of play modules

	//------------------------------------------------------------------------------
	// Functions
	//------------------------------------------------------------------------------
	public:
		////////////////////////////////////////////////////////////////////////////////
		// Constructor
		////////////////////////////////////////////////////////////////////////////////
		CPlayGroup(void)
			{
			}


		////////////////////////////////////////////////////////////////////////////////
		// Destructor
		////////////////////////////////////////////////////////////////////////////////
		~CPlayGroup()
			{
			m_Plays.Reset();
			}


		////////////////////////////////////////////////////////////////////////////////
		// Add a module
		////////////////////////////////////////////////////////////////////////////////
		void AddModule(
			CPlay* pPlay)
			{
			m_Plays.InsertHead(pPlay);
			}


		////////////////////////////////////////////////////////////////////////////////
		// Remove a module
		////////////////////////////////////////////////////////////////////////////////
		void RemoveModule(
			CPlay* pPlay)
			{
			// Search for the specific pointer and remove it.  This is inefficient, but
			// but it happens so rarely that it doesn't matter.  The alternative was for
			// AddModule() to return a Plays::Pointer, which the caller would pass to
			// this function so it could remove the object without searching for it.
			// This sound good, but then CPlay would have to know what a Plays::Pointer
			// is so it could hold on to it.  Unfortunately, the pointer is defined by
			// this class, and this class must follow the definition of CPlay, so you
			// end up with a classic which-comes-first-the-chicken-or-the-egg problem.
			for (Plays::Pointer p = m_Plays.GetHead(); p != 0; p = m_Plays.GetNext(p))
				{
				if (m_Plays.GetData(p) == pPlay)
					{
					m_Plays.Remove(p);
					break;
					}
				}
			}


		////////////////////////////////////////////////////////////////////////////////
		// Prepare game
		////////////////////////////////////////////////////////////////////////////////
		short PrepareGame(										// Returns 0 if successfull, non-zero otherwise
			CPlayInfo* pinfo)										// I/O: Play info
			{
			short sResult = 0;
			for (Plays::Pointer p = m_Plays.GetHead(); p != 0; p = m_Plays.GetNext(p))
				sResult |= m_Plays.GetData(p)->PrepareGame(pinfo);
			return sResult;
			}


		////////////////////////////////////////////////////////////////////////////////
		// Determine if game is ready
		////////////////////////////////////////////////////////////////////////////////
		short IsGameReady(										// Returns 0 if successfull, non-zero otherwise
			CPlayInfo* pinfo,										// I/O: Play info
			bool* pbGameReady)									// Out: Whether game is ready
			{
			short sResult = 0;
			*pbGameReady = true;
			for (Plays::Pointer p = m_Plays.GetHead(); p != 0; p = m_Plays.GetNext(p))
				{
				bool bGameReady = false;
				sResult |= m_Plays.GetData(p)->IsGameReady(pinfo, &bGameReady);
				*pbGameReady &= bGameReady;
				}
			return sResult;
			}


		////////////////////////////////////////////////////////////////////////////////
		// Start game
		////////////////////////////////////////////////////////////////////////////////
		short StartGame(											// Returns 0 if successfull, non-zero otherwise
			CPlayInfo* pinfo)										// I/O: Play info
			{
			short sResult = 0;
			for (Plays::Pointer p = m_Plays.GetHead(); p != 0; p = m_Plays.GetNext(p))
				sResult |= m_Plays.GetData(p)->StartGame(pinfo);
			return sResult;
			}


		////////////////////////////////////////////////////////////////////////////////
		// Start cutscene
		////////////////////////////////////////////////////////////////////////////////
		void StartCutscene(
			CPlayInfo* pinfo)										// I/O: Play info
			{
			for (Plays::Pointer p = m_Plays.GetHead(); p != 0; p = m_Plays.GetNext(p))
				m_Plays.GetData(p)->StartCutscene(pinfo);
			}


		////////////////////////////////////////////////////////////////////////////////
		// Prepare realm
		////////////////////////////////////////////////////////////////////////////////
		short PrepareRealm(										// Returns 0 if successfull, non-zero otherwise
			CPlayInfo* pinfo)										// I/O: Play info
			{
			short sResult = 0;
			for (Plays::Pointer p = m_Plays.GetHead(); p != 0; p = m_Plays.GetNext(p))
				sResult |= m_Plays.GetData(p)->PrepareRealm(pinfo);
			return sResult;
			}


		////////////////////////////////////////////////////////////////////////////////
		// Determine if realm is ready
		////////////////////////////////////////////////////////////////////////////////
		short IsRealmReady(										// Returns 0 if successfull, non-zero otherwise
			CPlayInfo* pinfo,										// I/O: Play info
			bool* pbRealmReady)									// Out: Whether realm is ready
			{
			short sResult = 0;
			*pbRealmReady = true;
			for (Plays::Pointer p = m_Plays.GetHead(); p != 0; p = m_Plays.GetNext(p))
				{
				bool bRealmReady = false;
				sResult |= m_Plays.GetData(p)->IsRealmReady(pinfo, &bRealmReady);
				*pbRealmReady &= bRealmReady;
				}
			return sResult;
			}


		////////////////////////////////////////////////////////////////////////////////
		// Do cutscene
		////////////////////////////////////////////////////////////////////////////////
		void DoCutscene(
			CPlayInfo* pinfo)										// I/O: Play info
			{
			for (Plays::Pointer p = m_Plays.GetHead(); p != 0; p = m_Plays.GetNext(p))
				m_Plays.GetData(p)->DoCutscene(pinfo);
			}


		////////////////////////////////////////////////////////////////////////////////
		// End cutscene
		////////////////////////////////////////////////////////////////////////////////
		void EndCutscene(
			CPlayInfo* pinfo)										// I/O: Play info
			{
			for (Plays::Pointer p = m_Plays.GetHead(); p != 0; p = m_Plays.GetNext(p))
				m_Plays.GetData(p)->EndCutscene(pinfo);
			}


		////////////////////////////////////////////////////////////////////////////////
		// Start realm
		////////////////////////////////////////////////////////////////////////////////
		short StartRealm(											// Returns 0 if successfull, non-zero otherwise
			CPlayInfo* pinfo)										// I/O: Play info
			{
			short sResult = 0;
			for (Plays::Pointer p = m_Plays.GetHead(); p != 0; p = m_Plays.GetNext(p))
				sResult |= m_Plays.GetData(p)->StartRealm(pinfo);
			return sResult;
			}


		////////////////////////////////////////////////////////////////////////////////
		// Core loop user input
		////////////////////////////////////////////////////////////////////////////////
		void CoreLoopUserInput(
			CPlayInfo* pinfo,										// I/O: Play info
			RInputEvent* pie)										// I/O: Input event
			{
			for (Plays::Pointer p = m_Plays.GetHead(); p != 0; p = m_Plays.GetNext(p))
				m_Plays.GetData(p)->CoreLoopUserInput(pinfo, pie);
			}


		////////////////////////////////////////////////////////////////////////////////
		// Core loop update
		////////////////////////////////////////////////////////////////////////////////
		void CoreLoopUpdate(
			CPlayInfo* pinfo)										// I/O: Play info
			{
			for (Plays::Pointer p = m_Plays.GetHead(); p != 0; p = m_Plays.GetNext(p))
				m_Plays.GetData(p)->CoreLoopUpdate(pinfo);
			}


		////////////////////////////////////////////////////////////////////////////////
		// Core loop render -- create and update images to the composite buffer but do
		// NOT update the screen.
		////////////////////////////////////////////////////////////////////////////////
		void CoreLoopRender(
			CPlayInfo* pinfo)										// I/O: Play info
			{
			for (Plays::Pointer p = m_Plays.GetHead(); p != 0; p = m_Plays.GetNext(p))
				m_Plays.GetData(p)->CoreLoopRender(pinfo);
			}


		////////////////////////////////////////////////////////////////////////////////
		// Core loop render on top -- create and update images to the composite buffer 
		// on top of things rendered in CoreLoopRender() but do NOT update the screen.
		////////////////////////////////////////////////////////////////////////////////
		virtual
		void CoreLoopRenderOnTop(
			CPlayInfo* pinfo)										// I/O: Play info
			{
			for (Plays::Pointer p = m_Plays.GetHead(); p != 0; p = m_Plays.GetNext(p))
				m_Plays.GetData(p)->CoreLoopRenderOnTop(pinfo);
			}


		////////////////////////////////////////////////////////////////////////////////
		// Core loop draw -- Draw CoreLoopRender[OnTop]() results to the screen.
		////////////////////////////////////////////////////////////////////////////////
		virtual
		void CoreLoopDraw(
			CPlayInfo* pinfo)										// I/O: Play info
			{
			for (Plays::Pointer p = m_Plays.GetHead(); p != 0; p = m_Plays.GetNext(p))
				m_Plays.GetData(p)->CoreLoopDraw(pinfo);

			// Update the display in the dirtied areas defined by m_drl.
			RDRect*	pdr	= pinfo->m_drl.GetHead();
			while (pdr)
				{
				long lTime = rspGetMilliseconds();

				// Update the portion of the display.
				rspCacheDirtyRect(pdr->sX, pdr->sY, pdr->sW, pdr->sH);

				pinfo->m_lSumUpdateDisplayTimes += (rspGetMilliseconds() - lTime);

				// Remove the current rectangle from the list and delete it.
				pinfo->m_drl.Remove();

				delete pdr;

				// Get the next one.
				pdr	= pinfo->m_drl.GetNext();
				}
			rspUpdateDisplayRects();
			}


		////////////////////////////////////////////////////////////////////////////////
		// Determine if core loop is done
		////////////////////////////////////////////////////////////////////////////////
		bool IsCoreLoopDone(										// Returns true if done, false otherwise
			CPlayInfo* pinfo)										// I/O: Play info
			{
			bool bDone = true;
			for (Plays::Pointer p = m_Plays.GetHead(); p != 0; p = m_Plays.GetNext(p))
				bDone &= m_Plays.GetData(p)->IsCoreLoopDone(pinfo);
			return bDone;
			}


		////////////////////////////////////////////////////////////////////////////////
		// End realm
		////////////////////////////////////////////////////////////////////////////////
		void EndRealm(
			CPlayInfo* pinfo)										// I/O: Play info
			{
			for (Plays::Pointer p = m_Plays.GetHead(); p != 0; p = m_Plays.GetNext(p))
				m_Plays.GetData(p)->EndRealm(pinfo);

			// Any dirty rects left over, we don't care about.
			pinfo->m_drl.Empty();
			}


		////////////////////////////////////////////////////////////////////////////////
		// Unprepare game
		////////////////////////////////////////////////////////////////////////////////
		void UnprepareGame(
			CPlayInfo* pinfo)										// I/O: Play info
			{
			for (Plays::Pointer p = m_Plays.GetHead(); p != 0; p = m_Plays.GetNext(p))
				m_Plays.GetData(p)->UnprepareGame(pinfo);
			}


		////////////////////////////////////////////////////////////////////////////////
		// Start realm error handler
		////////////////////////////////////////////////////////////////////////////////
		void StartRealmErr(
			CPlayInfo* pinfo)										// I/O: Play info
			{
			for (Plays::Pointer p = m_Plays.GetHead(); p != 0; p = m_Plays.GetNext(p))
				m_Plays.GetData(p)->StartRealmErr(pinfo);
			}


		////////////////////////////////////////////////////////////////////////////////
		// Is realm ready error handler
		////////////////////////////////////////////////////////////////////////////////
		void IsRealmReadyErr(
			CPlayInfo* pinfo)										// I/O: Play info
			{
			for (Plays::Pointer p = m_Plays.GetHead(); p != 0; p = m_Plays.GetNext(p))
				m_Plays.GetData(p)->IsRealmReadyErr(pinfo);
			}


		////////////////////////////////////////////////////////////////////////////////
		// Prepare realm error handler
		////////////////////////////////////////////////////////////////////////////////
		void PrepareRealmErr(
			CPlayInfo* pinfo)										// I/O: Play info
			{
			for (Plays::Pointer p = m_Plays.GetHead(); p != 0; p = m_Plays.GetNext(p))
				m_Plays.GetData(p)->PrepareRealmErr(pinfo);
			}


		////////////////////////////////////////////////////////////////////////////////
		// Start game error handler
		////////////////////////////////////////////////////////////////////////////////
		void StartGameErr(
			CPlayInfo* pinfo)										// I/O: Play info
			{
			for (Plays::Pointer p = m_Plays.GetHead(); p != 0; p = m_Plays.GetNext(p))
				m_Plays.GetData(p)->StartGameErr(pinfo);
			}


		////////////////////////////////////////////////////////////////////////////////
		// Is game ready error handler
		////////////////////////////////////////////////////////////////////////////////
		void IsGameReadyErr(
			CPlayInfo* pinfo)										// I/O: Play info
			{
			for (Plays::Pointer p = m_Plays.GetHead(); p != 0; p = m_Plays.GetNext(p))
				m_Plays.GetData(p)->IsGameReadyErr(pinfo);
			}


		////////////////////////////////////////////////////////////////////////////////
		// Prepare game error handler
		////////////////////////////////////////////////////////////////////////////////
		void PrepareGameErr(
			CPlayInfo* pinfo)										// I/O: Play info
			{
			for (Plays::Pointer p = m_Plays.GetHead(); p != 0; p = m_Plays.GetNext(p))
				m_Plays.GetData(p)->PrepareGameErr(pinfo);
			}
	};


////////////////////////////////////////////////////////////////////////////////
//
// Client Play Module
//
////////////////////////////////////////////////////////////////////////////////
class CPlayNet : public CPlay
	{
	//------------------------------------------------------------------------------
	// Types, enums, etc.
	//------------------------------------------------------------------------------
	private:

	//------------------------------------------------------------------------------
	// Variables
	//------------------------------------------------------------------------------
	private:
		bool				m_bEndServerCleanly;
		bool				m_bServerDone;
		bool				m_bEndClientCleanly;
		bool				m_bClientDone;

		bool				m_bHandledUserQuit;
		bool				m_bHandledNextRealm;

		bool				m_bAbortNow;						// Ultimate abort flag

		bool				m_bCheckForAbortKey;				// Whether to check for user abort
		bool				m_bTimeBombActive;				// Whether time bomb is active
		long				m_lTimeBomb;						// Time when bomb explodes

		bool				m_bShowNetFeedback;				// Whether to show net feedback thingy

		bool				m_bFirstCoreLoopUserInput;
		REdit*			m_apeditChats[NUM_CHATS];		// Received chat edit fields.
		long				m_lLastChatMoveTime;				// Last time chats were adjusted.

	//------------------------------------------------------------------------------
	// Functions
	//------------------------------------------------------------------------------
	public:
		////////////////////////////////////////////////////////////////////////////////
		// Constructor
		////////////////////////////////////////////////////////////////////////////////
		CPlayNet(void)
			{
			// Note that if any of this fails, we don't care (we just won't use them).
			short	sIndex;
			for (sIndex = 0; sIndex < NUM_CHATS; sIndex++)
				{
				m_apeditChats[sIndex]	= (REdit*)RGuiItem::LoadInstantiate(FullPathHD(CHAT_GUI) );
				if (m_apeditChats[sIndex])
					{
					// Recreate in the correct spot and dimensions . . .
					if (m_apeditChats[sIndex]->Create(
						REALM_STATUS_RECT_X,
						REALM_STATUS_RECT_Y + REALM_STATUS_RECT_H + (m_apeditChats[sIndex]->m_im.m_sHeight * sIndex),
						REALM_STATUS_RECT_W,
						m_apeditChats[sIndex]->m_im.m_sHeight,
						g_pimScreenBuf->m_sDepth) == 0)
						{
						m_apeditChats[sIndex]->SetText("");
						m_apeditChats[sIndex]->SetVisible(FALSE);
						}
					else
						{
						delete m_apeditChats[sIndex];
						m_apeditChats[sIndex]	= NULL;
						}
					}
				}
			}


		////////////////////////////////////////////////////////////////////////////////
		// Destructor
		////////////////////////////////////////////////////////////////////////////////
		/* virtual */
		~CPlayNet()
			{
			short	sIndex;
			for (sIndex = 0; sIndex < NUM_CHATS; sIndex++)
				{
				delete m_apeditChats[sIndex];
				m_apeditChats[sIndex]	= NULL;
				}
			}

		
		////////////////////////////////////////////////////////////////////////////////
		// Move all the chat texts up one field.  Hides emptied fields.
		////////////////////////////////////////////////////////////////////////////////
		void MoveChatsUp(
			CPlayInfo*	pinfo)			// In:  Info object.
			{
			short	sIndex	= 0;
			// Goto last chat that is filled moving them up as we go.
			while (sIndex < NUM_CHATS)
				{
				if (m_apeditChats[sIndex]->m_szText[0])
					{
					m_apeditChats[sIndex]->SetText("%s", (sIndex >= NUM_CHATS - 1) ? "" : m_apeditChats[sIndex + 1]->m_szText);
					m_apeditChats[sIndex]->Compose();

					// If this emptied the field . . .
					if (m_apeditChats[sIndex]->m_szText[0] == '\0')
						{
						m_apeditChats[sIndex]->SetVisible(FALSE);	

						// If we're at all off of the view edge . . .
						if (pinfo->Camera()->m_sFilmViewX > 0)
							{
							// Erase now.
							rspGeneralLock(g_pimScreenBuf);

							rspRect(
								RSP_BLACK_INDEX,
								g_pimScreenBuf,
								m_apeditChats[sIndex]->m_sX,
								m_apeditChats[sIndex]->m_sY,
								m_apeditChats[sIndex]->m_im.m_sWidth,
								m_apeditChats[sIndex]->m_im.m_sHeight);

							rspGeneralUnlock(g_pimScreenBuf);

							// Add dirty rectangle to update erased area.
							pinfo->m_drl.Add(
								m_apeditChats[sIndex]->m_sX,
								m_apeditChats[sIndex]->m_sY,
								m_apeditChats[sIndex]->m_im.m_sWidth,
								m_apeditChats[sIndex]->m_im.m_sHeight);
							}
						}
					}
				else
					{
					break;
					}

				sIndex++;
				}

			m_lLastChatMoveTime	= rspGetMilliseconds();
			}

		////////////////////////////////////////////////////////////////////////////////
		// Draw visible chats.
		////////////////////////////////////////////////////////////////////////////////
		void DrawChats(
			CPlayInfo*	pinfo)			// In:  Info object.
			{
			short	sIndex;
			for (sIndex = 0; sIndex < NUM_CHATS; sIndex++)
				{
				if (m_apeditChats[sIndex]->m_sVisible != FALSE)
					{
					m_apeditChats[sIndex]->Draw(g_pimScreenBuf);

					// Make sure this gets to the display.
					pinfo->m_drl.Add(
						m_apeditChats[sIndex]->m_sX,
						m_apeditChats[sIndex]->m_sY,
						m_apeditChats[sIndex]->m_im.m_sWidth,
						m_apeditChats[sIndex]->m_im.m_sHeight);
					}
				}
			}

		////////////////////////////////////////////////////////////////////////////////
		// Prepare realm
		////////////////////////////////////////////////////////////////////////////////
		/* virtual */
		short PrepareRealm(										// Returns 0 if successfull, non-zero otherwise
			CPlayInfo* pinfo)										// I/O: Play info
			{
			CNetClient* pclient = pinfo->Client();
			if (pclient)
				{
				// Clear server flags even if there's no server so we can rely on these flags
				m_bEndServerCleanly = false;
				m_bServerDone = false;
				m_bEndClientCleanly = false;
				m_bClientDone = false;

				m_bHandledUserQuit = false;
				m_bHandledNextRealm = false;

				m_bAbortNow = false;
				m_bCheckForAbortKey = false;
				m_bTimeBombActive = false;
				m_lTimeBomb = 0;

				m_bShowNetFeedback = false;

				m_bFirstCoreLoopUserInput = true;

				m_lLastChatMoveTime = 0;

				pinfo->m_bUserQuitMP = false;
				pinfo->m_bNextRealmMP = false;

				pinfo->m_bDoRealmFrame = false;

				// Call this periodically to let it know we're not locked up
				NetBlockingWatchdog();

				// 09/12/97 MJR - Having this here is a MAJOR MISTAKE, but it happened
				// to work out okay.  The order in which the CPlay-derived objects
				// were added to the CPlayGroup HAPPENS TO WORK OUT so that this
				// function is called AFTER the same function in the CPlayRealm object,
				// which is what actually loads the realm.  Therefore, this message is
				// only sent AFTER the realm was loaded, which is good.  However, the
				// release version also had this hardwired to "true", which means that
				// regardless of whether the realm file loaded or not, it always told
				// the server that it was ready.  That's bad.
				//
				// But it gets worse.  The original intention of this, assuming it had
				// been done correctly, was for the client to either say "yes, this worked,
				// let's play", or "I had a problem, abort the game".  That sounded good
				// at the time, but unfortunately we didn't realize until too late that
				// in the case where some users have added new levels or some users had
				// a limited version of the game, not all the players would have all the
				// same levels available.  The preferred approach, in retrospect, would
				// have been for the server to make sure all the clients had all the
				// necessary levels ahead of time.  Instead, we're now stuck trying to
				// patch it after-the-fact.
				//
				// The new idea is that if we try to load a realm and we fail, then
				// it will most likely be due to the fact that we don't have that
				// realm, as opposed to the very rare file reading error.  And instead
				// of telling the host of the problem, which would cause the host to
				// end the game, we instead lie to the host and say everything is fine,
				// but then follow that up immediately with a request to drop from the
				// game.  This way, everyone else can go on playing.
				//
				// So, after all that, we leave the original bad line of code as is!

				// Tell the server we've got the realm ready to go
				pclient->SendRealmStatus(true);
				}
			return 0;
			}


		////////////////////////////////////////////////////////////////////////////////
		// Start realm
		////////////////////////////////////////////////////////////////////////////////
		virtual
		short StartRealm(											// Returns 0 if successfull, non-zero otherwise
			CPlayInfo* pinfo)										// I/O: Play info
			{
			if (pinfo->IsMP())
				{
				// Most of the players will likely end up waiting for at least one other
				// player to start, which means they are staring at a blank screen.  To
				// alleviate their fears of a lockup, we show this message.
				RTxt* ptxt = GetNetProbGUI();
				if (ptxt)
					{
					ptxt->SetText(
						"Waiting for other\n"
						"players to get ready...");
					ptxt->Compose();
					ptxt->Move(
						(g_pimScreenBuf->m_sWidth / 2) - (ptxt->m_im.m_sWidth / 2),
						(g_pimScreenBuf->m_sHeight / 2) - (ptxt->m_im.m_sHeight / 2));
					ptxt->SetVisible(TRUE);
					ptxt->Draw(g_pimScreenBuf);
					rspUpdateDisplay(ptxt->m_sX, ptxt->m_sY, ptxt->m_im.m_sWidth, ptxt->m_im.m_sHeight);
					m_bShowNetFeedback = true;
					}
				}
			return 0;
			}


		////////////////////////////////////////////////////////////////////////////////
		// Core loop user input
		////////////////////////////////////////////////////////////////////////////////
		void CoreLoopUserInput(
			CPlayInfo* pinfo,										// I/O: Play info
			RInputEvent* pie)										// I/O: Input event
			{
			if (pinfo->IsMP())
				{
				// Get pointers to make this more readable
				CNetServer* pserver = pinfo->Server();
				CNetClient* pclient = pinfo->Client();

				// If this is the first time this function is being called, then turn off
				// the feedback we displayed in StartRealm().
				if (m_bFirstCoreLoopUserInput)
					{
					m_bFirstCoreLoopUserInput = false;
					m_bShowNetFeedback = false;
					}

				// Check if user aborted the net blocking (meaning we were stuck and the
				// user decided he had waited long enough).
				if (NetBlockingWasAborted())
					pinfo->m_bUserQuitMP = true;

				// Check if the realm is bad (meaning it couldn't be loaded) and we've
				// started playing, and if so, abort the game.  If we're the server, this
				// will end the entire game, and if we're a client we'll just drop out
				// and let everyone else continue.  Note that we need to wait until we're
				// playing because we don't currently support dropping players during
				// while waiting for the realm to start.
				if (pinfo->m_bBadRealmMP && pclient->IsPlaying())
					{
					// Quit game
					pinfo->m_bUserQuitMP = true;

					// Tell user what's going on
					RTxt* ptxt = GetNetProbGUI();
					if (ptxt)
						{
						ptxt->SetText(
							"You don't have\n"
							"the selected hood.\n"
							"Dropping out...");
						ptxt->Compose();
						ptxt->Move(
							(g_pimScreenBuf->m_sWidth / 2) - (ptxt->m_im.m_sWidth / 2),
							(g_pimScreenBuf->m_sHeight / 2) - (ptxt->m_im.m_sHeight / 2));
						ptxt->SetVisible(TRUE);
						ptxt->Draw(g_pimScreenBuf);
						m_bShowNetFeedback = true;
						}
					}

				// If user wants to quit the MP game, try to do it cleanly.  It is
				// possible that the network links are down, so we also set a timer
				// that quits the game if it takes too long to do it the "right" way.
				// Note that a second flag keeps us from doing this more than once.
				if (pinfo->m_bUserQuitMP && !m_bHandledUserQuit)
					{
					m_bHandledUserQuit = true;

					if (pserver)
						{
						// Abort game and end server (local client will end when it gets the message)
						pserver->AbortGame(NetMsg::UserAbortedGame);
						m_bEndServerCleanly = true;
						}
					else
						{
						// Tell server we're dropping and then disconnect from it (cleanly, which means
						// it waits until all messages have been sent).  We then set the flag so we
						// do essentially the same thing at this level -- wait for all messages to be sent.
						pclient->Drop();
						pinfo->SetGameState_GameAborted();
						m_bEndClientCleanly = true;
						}

					// Set time bomb in case the "nice" way doesn't work
					m_lTimeBomb = rspGetMilliseconds() + g_GameSettings.m_lNetForceAbortTime;
					m_bTimeBombActive = true;
					}

				// Once the time bomb goes off, we display a message telling the user that something
				// has gone very wrong, and they can either wait a little longer or hit a key to abort.
				// Once that message is displayed, we check if the user has hit the abort key, and if
				// so, we set the ultimate abort flag.
				if (m_bCheckForAbortKey)
					{
					if ((pie->type == RInputEvent::Key) && (pie->lKey == NET_PROB_GUI_ABORT_GK_KEY))
						{
						pinfo->SetGameState_GameAborted();
						m_bAbortNow = true;
						}
					}
				else
					{
					if (m_bTimeBombActive)
						{
						if (rspGetMilliseconds() > m_lTimeBomb)
							{
							RTxt* ptxt = GetNetProbGUI();
							if (ptxt)
								{
								ptxt->SetText("%s", g_pszNetProb_General);
								ptxt->Compose();
								ptxt->SetVisible(TRUE);
								ptxt->Move(
									(g_pimScreenBuf->m_sWidth / 2) - (ptxt->m_im.m_sWidth / 2),
									(g_pimScreenBuf->m_sHeight / 2) - (ptxt->m_im.m_sHeight / 2));

								// Set flag to indicate that we're waiting for user to abort
								m_bCheckForAbortKey = true;
								}
							else
								{
								// If we can't display the message for the user, we don't have much
								// choice but to abort
								TRACE("CPlayNet::CoreLoopUpdate(): Unable to show abort GUI.  Aborting.\n");
								pinfo->SetGameState_GameAborted();
								m_bAbortNow = true;
								}
							}
						}
					}

				// Try doing the next frame, which involves setting all the players' inputs.
				// We must NEVER try to do another frame if we haven't done the previous one,
				// because skipping a frame would screw up network sync!
				if (!pinfo->m_bDoRealmFrame)
					{
					// If we can do a frame, all player's inputs will be returned
					UINPUT aInputs[Net::MaxNumIDs];

					/** SPA **/
					short sFrameTime = 0;
					if (pclient->CanDoFrame(aInputs, &sFrameTime)) // Get frame time as well *SPA
						{
						pinfo->SetFrameTime(sFrameTime);
						/** SPA **/

						// Copy inputs into the input module
						for (Net::ID id = 0; id < Net::MaxNumIDs; id++)
							SetInput(id, aInputs[id]);

						// Set flag so we'll do the frame
						pinfo->m_bDoRealmFrame = true;
						}
					else
						{
						//------------------------------------------------------------------------------
						// Server section *SPA 1/13/98
						//------------------------------------------------------------------------------
						if (pserver && !m_bServerDone)
							{
							// Check to see if we have lost a peer through a timeout
							Net::ID peerID = pclient->CheckForLostPeer();
							if (peerID < Net::MaxNumIDs)
								// Lost one, so drop him so the rest of us can go on
								pserver->DropClient(peerID);
							}
						}


					}
				}
			}


		////////////////////////////////////////////////////////////////////////////////
		// Core loop update
		////////////////////////////////////////////////////////////////////////////////
		/* virtual */
		void CoreLoopUpdate(
			CPlayInfo* pinfo)										// I/O: Play info
			{
			// If we're in MP mode, then  there's always a client and there may be a server
			if (pinfo->IsMP())
				{

				// Get pointers to make this more readable
				CNetServer* pserver = pinfo->Server();
				CNetClient* pclient = pinfo->Client();

				// Call this periodically to let it know we're not locked up
				NetBlockingWatchdog();

				//------------------------------------------------------------------------------
				// Server section
				//------------------------------------------------------------------------------
				if (pserver && !m_bServerDone)
					{

					// Update server (send and receive messages)
					pserver->Update();

					// Check if we're trying to end
					if (m_bEndServerCleanly)
						{
						// If there isn't anything more to send, then we're done.  It's safe to
						// ignore incoming messages because they will get bufferred and we'll
						// handle them in the next realm if there is one, and if the game is
						// over, they don't matter anyway!!!
						if (!pserver->IsMoreToSend())
							m_bServerDone = true;
						}
					else
						{
						// If the realm is done, tell the clients about it.  Only the server ever
						// checks for the end of the realm -- the clients rely on the server to
						// tell them when to end.  In case this flag somehow gets set multiple
						// times, we only want to handle it once per realm, hence the purpose
						// of the second flag.
						if (pinfo->m_bNextRealmMP && !m_bHandledNextRealm)
							{

							//------------------------------------------------------------------------------
							// DIRECT BREACH OF CLIENT/SERVER SEPARATION!
							//
							// In order to get all the clients to stop on the same frame, the server
							// needs to choose a frame that no client has yet reached.  Unfortunately,
							// it has no way of knowing this (having the clients constantly report
							// their frame seq's would not help because the information would always
							// be out of date due to latencies.  Instead, we violate the client/server
							// separation and ask the local client for the most recent input seq that
							// it sent to any of the other clients.  Since no client can possibly be
							// ahead of that (it can't do a frame unless it has all the inputs), we
							// can safely use that as the frame to stop on.
							//
							// But that's not all!!!  If we were to send the "stop on frame" message
							// to all the clients, including the local one, there is still the
							// latency to deal with, and even message to local clients have some
							// latnecy.  During that time, if the local client sends out input beyond
							// the stop frame, all is lost.  So, we again violate the client/server
							// separation and DIRECTLY tell the local client what frame to stop on.
							// This ensures that it will not send out any excess inputs.
							//------------------------------------------------------------------------------
							// Note that we add one to the client's sequence as an extra safey measure.
							Net::SEQ seqHalt = (Net::SEQ)(pclient->GetInputSeqNotYetSent() + (Net::SEQ)1);
							if (pserver->NextRealm(seqHalt))
								{
								// Tell local client (see above explanation)
								pclient->SetHaltFrame(seqHalt);

								// Note that we handled this 
								m_bHandledNextRealm = true;

								// End the server cleanly
								m_bEndServerCleanly = true;
								}
							}
						else
							{
							// Process messages
							NetMsg msg;
							pserver->GetMsg(&msg);
							switch (msg.msg.nothing.ucType)
								{
								case NetMsg::ERR:
									// We'll learn about any really bad errors via the client!
									break;

								case NetMsg::NOTHING:
									break;

								case NetMsg::CHAT_REQ:
									// Handled at lower level.
									break;

								default:
									TRACE("CPlayNet::CoreLoopUpdate(): Unhandled message: %hd\n", (short)msg.msg.nothing.ucType);
									break;
								}
							}
						}
					}

				//------------------------------------------------------------------------------
				// Client section
				//------------------------------------------------------------------------------

				// Check if local input is needed, and if so, hand it over
				if (!pinfo->m_bBadRealmMP)
					{
					if (pclient->IsLocalInputNeeded())
						{
						if (!pinfo->m_bInMenu && !pinfo->m_bChatting)
							pclient->SetLocalInput(GetLocalInput(pinfo->Realm()) );
						else
							pclient->SetLocalInput(INPUT_IDLE);
						}
					}

				// Handle network communiciation stuff
				if (!m_bClientDone)
					{
					pclient->Update();

					// Check if we're trying to end
					if (m_bEndClientCleanly)
						{
						// If there isn't anything more to send, then we're done
						if (!pclient->IsMoreToSend())
							m_bClientDone = true;
						}
					else
						{
						// Process messages from server
						NetMsg msg;
						pclient->GetMsg(&msg);
						switch (msg.msg.nothing.ucType)
							{
							case NetMsg::JOINED:
								break;

							case NetMsg::DROPPED:
								// If I am dropped, abort the game and end immediately
								if (msg.msg.dropped.id == pclient->GetID())
									{
									pinfo->SetGameState_GameAborted();
									m_bClientDone = true;
									}
								break;

								case NetMsg::CHAT:
									{
									short	sIndex	= 0;

									while (m_apeditChats[sIndex]->m_szText[0])
										{
										if (++sIndex >= NUM_CHATS)	// Should never be greater but...
											{
											MoveChatsUp(pinfo);
											sIndex	= NUM_CHATS - 1;
											break;
											}
										}

									ASSERT(sIndex < NUM_CHATS);

									m_apeditChats[sIndex]->SetText("%s", msg.msg.chat.acText);
									m_apeditChats[sIndex]->Compose();
									m_apeditChats[sIndex]->SetVisible(TRUE);

									m_lLastChatMoveTime	= rspGetMilliseconds();

									break;
									}

							case NetMsg::ERR:
								break;

							case NetMsg::NOTHING:
								break;

							case NetMsg::ABORT_GAME:
								// Abort game and end client immediately
								pinfo->SetGameState_GameAborted();
								m_bClientDone = true;
								break;

							case NetMsg::NEXT_REALM:
								// Go on to the next realm (in case there's a server we end it cleanly,
								// too, because it was waiting for it's local client to get this message)
								pinfo->SetGameState_NextRealm();
								m_bEndServerCleanly = true;
								m_bEndClientCleanly = true;
								break;

							case NetMsg::PROGRESS_REALM:
// This was disabled for reasons explained at length in NetServer.cpp -- suffice it to say it's
// far from an easy fix.
#if 0
								m_bShowNetFeedback = false;
								if (msg.msg.progressRealm.sNumNotReady > 0)
									{
									// Show the player how many other players we're waiting for
									RTxt* ptxt = GetNetProbGUI();
									if (ptxt)
										{
										ptxt->SetText("Waiting for %hd\nplayer(s) to get ready...", msg.msg.progressRealm.sNumNotReady);
										ptxt->Compose();
										ptxt->Move(
											(g_pimScreenBuf->m_sWidth / 2) - (ptxt->m_im.m_sWidth / 2),
											(g_pimScreenBuf->m_sHeight / 2) - (ptxt->m_im.m_sHeight / 2));
										ptxt->SetVisible(TRUE);
										m_bShowNetFeedback = true;
										}
									}
#endif
								break;

							case NetMsg::PROCEED:
								break;

							default:
								TRACE("CPlayNet::CoreLoopUpdate(): Unhandled message: %hd\n", (short)msg.msg.nothing.ucType);
								break;
							}
						}
					}

				// Govern loop to a maximum of frames-per-second rate
				//
				// This is definitely not done.  It's a waste to sit here in a dead loop, when we could be checking
				// for more network input/output/whatever.  Perhaps this whole function should be inside a do/while
				// loop that loops as long until we've slowed to the proper frame rate.
				//
				// Actually, I think the real answer might be to put timers on everything in Play.  There's no
				// point in checking for keyboard input 100 times per second.  There's no point in doing many of
				// the things we do at that speed, but we DO want to loop through everything as quickly as possible
				// to give an opportunity to everyone to do whatever DOES need to be done.
				//
				// The frames-per-second governor would be done at the point were we actually render the frame
				// and increment the frame seq.  That portion would say "If we're ready to render the frame AND
				// it's been 1/50th of a second since we rendered the last frame, then do it."
//				while (rspGetMilliseconds() < m_lGovernTime)
//					;
//				m_lGovernTime = rspGetMilliseconds() + Net::MinFrameTime;

				if (m_lLastChatMoveTime	+ CHAT_DELAY < rspGetMilliseconds() )
					{
					// Move the chats up -- this updates m_lLastChatMoveTime.
					MoveChatsUp(pinfo);
					}
				}
			}


		////////////////////////////////////////////////////////////////////////////////
		// Core loop render -- create and update images to the composite buffer but do
		// NOT update the screen.
		////////////////////////////////////////////////////////////////////////////////
		void CoreLoopRender(
			CPlayInfo* pinfo)										// I/O: Play info
			{
			if (pinfo->IsMP())
				{
				// Draw chats, if any.
				DrawChats(pinfo);

				// If we're waiting for user to abort or there was a net problem
				if (m_bCheckForAbortKey || IsNetProb() || m_bShowNetFeedback)
					{
					// What we want to do is put it in the composite buffer.  It will be
					// copied to the screen in CoreLoopDraw().
					RTxt* ptxt = GetNetProbGUI();
					if (ptxt)
						{
						// Draw locks the screen as is necessary for the current buffer type.
						// Even though it's already locked, this should be fine.
						ptxt->Draw(g_pimScreenBuf);
						// Add a rectangle representing this GUI so it gets updated to the screen.
						pinfo->m_drl.Add(ptxt->m_sX, ptxt->m_sY, ptxt->m_im.m_sWidth, ptxt->m_im.m_sHeight);
						}
					}
				}
			}


		////////////////////////////////////////////////////////////////////////////////
		// Determine if core loop is done
		////////////////////////////////////////////////////////////////////////////////
		/* virtual */
		bool IsCoreLoopDone(										// Returns true if done, false otherwise
			CPlayInfo* pinfo)										// I/O: Play info
			{
			bool bDone;
			if (pinfo->IsMP())
				{
				// This is the ultimate abort flag
				if (m_bAbortNow)
					bDone = true;
				else
					{
					// Check server (if there is one)
					bDone = pinfo->Server() ? m_bServerDone : true;

					// There's always a client in MP mode
					bDone &= m_bClientDone;
					}
				}
			else
				{
				// If client doesn't exist, use base-class implimentation
				bDone = CPlay::IsCoreLoopDone(pinfo);
				}
			return bDone;
			}
	};


////////////////////////////////////////////////////////////////////////////////
//
// Display API
//
////////////////////////////////////////////////////////////////////////////////
class CPlayStatus : public CPlay
	{
	//------------------------------------------------------------------------------
	// Types, enums, etc.
	//------------------------------------------------------------------------------
	private:

	//------------------------------------------------------------------------------
	// Variables
	//------------------------------------------------------------------------------
	private:
		long				m_lLastFrameTime;
		long				m_lSumFrameTimes;
		long				m_lNumFrames;
		long				m_lLastIterationTime;
		/* 12/3/97 AJC */
		Net::SEQ			m_seqPrevFrameSeq;
		Net::SEQ			m_seqCurrFrameSeq;
		long				m_lFramePerSecond;
		long				m_lPrevSeqTime;
		/* 12/3/97 AJC */
		long				m_lSumIterationTimes;
		long				m_lNumIterations;
		RRect				m_rectDude;
		RRect				m_rectRealm;
		RRect				m_rectInfo;
		char				m_szFileDescriptor[256];
		RPrint			m_print;
		bool				m_bFirstUpdate;
		bool				m_bUpdateDude;					// true, if dude status area was updated.
		bool				m_bUpdateRealm;				// true, if realm status area was updated.

	//------------------------------------------------------------------------------
	// Functions
	//------------------------------------------------------------------------------
	public:
		////////////////////////////////////////////////////////////////////////////////
		// Constructor
		////////////////////////////////////////////////////////////////////////////////
		CPlayStatus(void)
				: m_bUpdateRealm(true)  // valgrind fix.  --ryan.
			{
			}


		////////////////////////////////////////////////////////////////////////////////
		// Destructor
		////////////////////////////////////////////////////////////////////////////////
		/* virtual */
		~CPlayStatus()
			{
			}


		////////////////////////////////////////////////////////////////////////////////
		// Start realm
		////////////////////////////////////////////////////////////////////////////////
		/* virtual */
		short StartRealm(											// Returns 0 if successfull, non-zero otherwise
			CPlayInfo* pinfo)										// I/O: Play info
			{
			if (!pinfo->m_bBadRealmMP)
				{
				// Initialize times and counts.
				m_lLastFrameTime = rspGetMilliseconds();
				m_lLastIterationTime	= m_lLastFrameTime;
				m_lSumFrameTimes = 0;
				m_lNumFrames = 0;
				pinfo->m_lSumUpdateDisplayTimes = 0;

				/*** 12/3/97 AJC ***/
				m_seqCurrFrameSeq = 0;
				m_lFramePerSecond = 0;
				if (pinfo->IsMP())
					m_seqPrevFrameSeq = pinfo->Client()->GetInputSeqNotYetSent();
				m_lPrevSeqTime	= rspGetMilliseconds();
				/*** 12/3/97 AJC ***/

				m_lSumIterationTimes	= 0;
				m_lNumIterations	= 0;

				// Get app's descriptor.
				Play_GetApplicationDescriptor(m_szFileDescriptor, sizeof(m_szFileDescriptor));

				// Setup rects for various status areas
				m_rectDude.sX = DUDE_STATUS_RECT_X;
				m_rectDude.sY = DUDE_STATUS_RECT_Y;
				m_rectDude.sW = DUDE_STATUS_RECT_W;
				m_rectDude.sH = DUDE_STATUS_RECT_H;

				m_rectRealm.sX = REALM_STATUS_RECT_X;
				m_rectRealm.sY = REALM_STATUS_RECT_Y;
				m_rectRealm.sW = REALM_STATUS_RECT_W;
				m_rectRealm.sH = REALM_STATUS_RECT_H;

				m_rectInfo.sX = INFO_STATUS_RECT_X;
				m_rectInfo.sY = INFO_STATUS_RECT_Y;
				m_rectInfo.sW = INFO_STATUS_RECT_W;
				m_rectInfo.sH = INFO_STATUS_RECT_H;

				// We must lock the buffer before accessing it.
				rspLockBuffer();

				// Init the tool bar
				ToolBarInit(pinfo->Realm()->m_phood);

				// Setup print	utilizing some values initialized by ToolBarInit().
				m_print.SetFont(DISP_INFO_FONT_HEIGHT, &g_fontBig);
				m_print.SetColor(gsStatusFontForeIndex, gsStatusFontBackIndex, gsStatusFontShadowIndex);
				m_print.SetEffectAbs(RPrint::SHADOW_X, 1);
				m_print.SetEffectAbs(RPrint::SHADOW_Y, 1);
				m_print.SetDestination(g_pimScreenBuf);

				// Render it now so it appears in tandem with the top bar: unless 
				// its the last demo level for the game where we don't want the
				// bars to let the player know that they are not under control.
				if (!g_bLastLevelDemo)
					{
					ToolBarRender(
						pinfo->Realm()->m_phood, 
						g_pimScreenBuf, 
						m_rectDude.sX, 
						m_rectDude.sY,
						pinfo->LocalDudePointer(),TRUE);

					// Draw the top bar just once.  I'm assuming all the rest of the updates are
					// partial and, therefore, this only needs to be done once.
					rspBlit(
						pinfo->Realm()->m_phood->m_pimTopBar,
						g_pimScreenBuf,
						0,
						0,
						REALM_STATUS_RECT_X,
						REALM_STATUS_RECT_Y,
						pinfo->Realm()->m_phood->m_pimTopBar->m_sWidth,
						pinfo->Realm()->m_phood->m_pimTopBar->m_sHeight);

					}

				// Unlock once done.
				rspUnlockBuffer();

				// Make sure this these get reflected on the screen intially.
				rspUpdateDisplay(m_rectDude.sX, m_rectDude.sY, m_rectDude.sW, m_rectDude.sH);

				rspUpdateDisplay(
					REALM_STATUS_RECT_X,
					REALM_STATUS_RECT_Y,
					pinfo->Realm()->m_phood->m_pimTopBar->m_sWidth,
					pinfo->Realm()->m_phood->m_pimTopBar->m_sHeight);
				}
			return 0;
			}


		////////////////////////////////////////////////////////////////////////////////
		// Core loop update
		////////////////////////////////////////////////////////////////////////////////
		/* virtual */
		void CoreLoopUpdate(
			CPlayInfo* pinfo)										// I/O: Play info
			{
			if (!pinfo->m_bBadRealmMP)
				{

				/**** 12/3/97  AJC ****/
				if (pinfo->IsMP())
					{

					// Get current frame sequence number
						m_seqCurrFrameSeq = pinfo->Client()->GetInputSeqNotYetSent();

					// Check if it's the next frame, if it is, calculate frame per sec
					if (m_seqCurrFrameSeq != m_seqPrevFrameSeq)
						{
						long lCurrSeqTime = rspGetMilliseconds();
						m_lFramePerSecond = 1000.0 * (m_seqCurrFrameSeq - m_seqPrevFrameSeq) / (lCurrSeqTime - m_lPrevSeqTime);

						m_seqPrevFrameSeq = m_seqCurrFrameSeq;
						m_lPrevSeqTime = lCurrSeqTime;
						}
					}
				/**** 12/3/97  AJC ****/

				//==============================================================================
				// Check for death stuff
				//==============================================================================
				CDude* pdudeLocal = pinfo->LocalDudePointer();
				// If the local dude is dead . . .
				if (pdudeLocal != NULL)
					{
					if (pdudeLocal->IsDead() )
						{
						// Make sure X Ray is on so we can see him.
						pinfo->Realm()->m_scene.SetXRayAll(TRUE);
						}
					else
						{
						// If alive, use user setting.
						pinfo->Realm()->m_scene.SetXRayAll( (pinfo->m_bXRayAll == true) ? TRUE : FALSE);
						}
					}
				}
			}


		////////////////////////////////////////////////////////////////////////////////
		// Core loop render on top -- create and update images to the composite buffer 
		// but do NOT update the screen.
		// NOTE that we use CoreLoopRenderOnTop() not b/c these go on top but instead as
		// a cheat to make sure that the m_bDrawFrame flag has already been set.
		////////////////////////////////////////////////////////////////////////////////
		/* virtual */
		void CoreLoopRenderOnTop(
			CPlayInfo* pinfo)										// I/O: Play info
			{
			if (!pinfo->m_bBadRealmMP)
				{
				// Only if we're not on the menu
				if (!pinfo->m_bInMenu)
					{
					// If display info is enabled . . .
					if (g_GameSettings.m_sDisplayInfo)
						{
						// Add this iteration's elapsed time to total (sum).
						m_lSumIterationTimes += (rspGetMilliseconds() - m_lLastIterationTime);

						// Increment number of iterations.
						m_lNumIterations++;
						}

					m_lLastIterationTime	= rspGetMilliseconds();

					// No need for this unless we're going to draw . . .
					if (pinfo->m_bDrawFrame)
						{
						bool	bUpdateRealm	= false;
						// If its the last level then don't draw the top and bottom, for
						// a letterbox look that lets the player know they don't have
						// control for this demo
						if (!g_bLastLevelDemo)
							{
							// Update the realm (or Score) status
							bUpdateRealm	= ScoreUpdateDisplay(
								g_pimScreenBuf, 
								&m_rectRealm, 
								pinfo->Realm(), 
								pinfo->Client(),
								REALM_STATUS_RECT_X,
								REALM_STATUS_RECT_Y,
								pinfo->Realm()->m_phood);
							if (bUpdateRealm == true)
								{
								pinfo->m_drl.Add(m_rectRealm.sX, m_rectRealm.sY, m_rectRealm.sW, m_rectRealm.sH);
								}

							// Generate TooL Bar
							if (ToolBarRender(
								pinfo->Realm()->m_phood, 
								g_pimScreenBuf, 
								m_rectDude.sX, 
								m_rectDude.sY,
								pinfo->LocalDudePointer()) == true)
								{
								pinfo->m_drl.Add(m_rectDude.sX, m_rectDude.sY, m_rectDude.sW, m_rectDude.sH);
								}
							}

						// If display info is enabled, draw the info
						if (g_GameSettings.m_sDisplayInfo)
							{
							// Add this frame's elapsed time to total (sum)
							m_lSumFrameTimes += (rspGetMilliseconds() - m_lLastFrameTime);

							// Increment number of frames
							m_lNumFrames++;

							// Just update whenever the realm is updating so we can cash in on their
							// update display all.
							// If were not in MP then calculate the FPS. If we are in MP, FPS is calculated elsewhere *SPA
							if (!pinfo->IsMP())
								m_lFramePerSecond = (1000 * m_lNumFrames) / m_lSumFrameTimes;

							if (m_bUpdateRealm)
								{
								m_print.print(
									m_rectInfo.sX, m_rectInfo.sY,
									"FPS: %ld Video H/W Update: %ld%% %s", 
									m_lFramePerSecond,
									(pinfo->m_lSumUpdateDisplayTimes * 100) / m_lSumFrameTimes,
									m_szFileDescriptor);

								// Reset.
								m_lSumFrameTimes = 0;
								m_lNumFrames = 0;
								pinfo->m_lSumUpdateDisplayTimes = 0;
								}
							}

						// By resetting this here, we ignore any time spent in the above code
						m_lLastFrameTime = rspGetMilliseconds();
						}
					}
				}
			}
	
	};


#if 1 //PLATFORM_UNIX
#include <sys/stat.h>
static void EnumSaveGamesSlots(Menu *menu)
{
    char gamename[RSP_MAX_PATH];
    int Max = (sizeof(menu->ami) / sizeof(menu->ami[0])) - 1;
    if (Max > MAX_SAVE_SLOTS)
        Max = MAX_SAVE_SLOTS;

    for (int i = 0; i < Max; i++)
    {
        snprintf(gamename, sizeof (gamename), "%s/%d.gme", SAVEGAME_DIR, i);
        const char *fname = FindCorrectFile(gamename, "w");
        struct stat statbuf;

        char timebuf[32];
        const char *str = "unknown";
        if (stat(fname, &statbuf) == -1)
            str = "available";
        else
        {
            struct tm *tm;
            if ((tm = localtime((const time_t*)&statbuf.st_mtime)) != NULL)
            {
                strftime(timebuf, sizeof (timebuf), "%m/%d/%y %H:%M", tm);
                str = timebuf;
            }
        }

#ifdef MOBILE
        snprintf(gamename, sizeof (gamename), "%d - [%s]", i, str);
        menu->ami[i].pszText = strdup(gamename);
#else
        snprintf(gamename, sizeof (gamename), "%s/%d.gme [%s]", SAVEGAME_DIR, i, str);
        menu->ami[i].pszText = strdup(gamename);
#endif

        menu->ami[i].sEnabled = (menu->ami[i].pszText != NULL);
        if (!menu->ami[i].sEnabled)
            break;
    }
}
#endif

#ifdef MOBILE
static	bool continueIsRestart; //This is to make the continue button actually restart the level
#endif
////////////////////////////////////////////////////////////////////////////////
//
// Input API
//
////////////////////////////////////////////////////////////////////////////////
class CPlayInput : public CPlay
	{
	//------------------------------------------------------------------------------
	// Types, enums, etc.
	//------------------------------------------------------------------------------
	private:

	//------------------------------------------------------------------------------
	// Variables
	//------------------------------------------------------------------------------
	private:
		long				m_lDemoDeadTime;				// Time dude has been dead for
		U8*				m_pau8KeyStatus;				// Key status array
		REdit*			m_peditChatIn;					// Outgoing chat.


	//------------------------------------------------------------------------------
	// Functions
	//------------------------------------------------------------------------------
	public:
		////////////////////////////////////////////////////////////////////////////////
		// Constructor
		////////////////////////////////////////////////////////////////////////////////
		CPlayInput(void)
			{
			// We don't care if this fails.
			m_peditChatIn	= (REdit*)RGuiItem::LoadInstantiate(FullPathHD(CHAT_IN_GUI) );
			if (m_peditChatIn)
				{
				// Limit to chat length minus some room for our name.
				m_peditChatIn->m_sMaxText	= MIN(CHAT_IN_LENGTH, Net::MaxChatSize - 1);

				// Recreate in the correct spot and dimensions . . .
				if (m_peditChatIn->Create(
					DUDE_STATUS_RECT_X,
					DUDE_STATUS_RECT_Y - m_peditChatIn->m_im.m_sHeight,
					DUDE_STATUS_RECT_W,
					m_peditChatIn->m_im.m_sHeight,
					g_pimScreenBuf->m_sDepth) == 0)
					{
					m_peditChatIn->SetVisible(FALSE);
					}
				else
					{
					delete m_peditChatIn;
					m_peditChatIn	= NULL;
					}
				}
#ifdef MOBILE
			continueIsRestart = false;
#endif
			}


		////////////////////////////////////////////////////////////////////////////////
		// Destructor
		////////////////////////////////////////////////////////////////////////////////
		/* virtual */
		~CPlayInput()
			{
			delete m_peditChatIn;
			m_peditChatIn	= NULL;
			}


		////////////////////////////////////////////////////////////////////////////////
		// Start realm
		////////////////////////////////////////////////////////////////////////////////
		virtual
		short StartRealm(											// Returns 0 if successfull, non-zero otherwise
			CPlayInfo* pinfo)										// I/O: Play info
			{
			// Reset time he's been dead ('cause he isn't dead yet)
			m_lDemoDeadTime = -1;
			
			// Get the key status array
			m_pau8KeyStatus		= rspGetKeyStatusArray();

			return 0;
			}


		////////////////////////////////////////////////////////////////////////////////
		// End realm
		////////////////////////////////////////////////////////////////////////////////
		/* virtual */
		void EndRealm(
			CPlayInfo* pinfo)										// I/O: Play info
			{
			}

		////////////////////////////////////////////////////////////////////////////////
		// Core loop user input
		////////////////////////////////////////////////////////////////////////////////
		virtual
		void CoreLoopUserInput(
			CPlayInfo* pinfo,										// I/O: Play info
			RInputEvent* pie)										// I/O: Input event
			{

			// If we're in the menu, then just do that
			if (pinfo->m_bInMenu)
				{
				DoMenu(pinfo, pie);
				}
			else
				{
				// Get usefull pointers
				CDude* pdudeLocal = pinfo->LocalDudePointer();
				CRealm* prealm = pinfo->Realm();

				//==============================================================================
				// A system-specific quit ends the game (MP mode is handled elsewhere)
				//==============================================================================
				if (rspGetQuitStatus())
					{
					if (pinfo->IsMP())
						pinfo->m_bUserQuitMP = true;
					else
						pinfo->SetGameState_GameAborted();
					}

				if (!pinfo->m_bBadRealmMP)
					{
					//==============================================================================
					// Handle all the local input stuff.
					//==============================================================================

					// Always default to "not pressed" for the "end-the-level-now" key
					bool bEndLevelKey = false;

					// If not in special end-of-the-game-demo-mode...
					if (!g_bLastLevelDemo)
						{
						// If playing back a demo and there's any input, then ignore the input and end the demo
						//if ((GetInputMode() == INPUT_MODE_PLAYBACK) && (pie->type != RInputEvent::None))
						if ((GetInputMode() == INPUT_MODE_PLAYBACK) && (pie->type == RInputEvent::Key))
							{
							pinfo->SetGameState_GameAborted();
							}
						else
							{
							// MUST NOT DO THIS IN SALES MODE!  Normally, this cheat just takes
							// you to the next level.  In sales mode, though, this cheat gives
							// you lots o stuff as well.  Going to the next level in Sales Mode
							// is handled separately.
							#if !defined(SALES_DEMO)
								// Cheats are disabled in MP mode by other code, but it still seems better to
								// clarify that this is NOT allowed in MP mode.
								if (!pinfo->IsMP())
									{
									if ((GetInput(0) & INPUT_WEAPONS_MASK) == INPUT_CHEAT_29)
										{
										// End the realm without checking whether goal has been met
										pinfo->SetGameState_NextRealm();
										}
									}
							#endif // !SALES_DEMO

							// Handle pause button on joystick
							if (g_InputSettings.m_sUseJoy)
							{
								XInputState xis;
								GetXInputStateNoUpdate(&xis);
								if (xis.ButtonState[g_InputSettings.m_sJoyStartButton] == XInputState::Press)
								{
									// Start the menu.  I have no idea why the check to make sure the game
									// is not over was necessary, but it doesn't seem like it could hurt,
									// so it was safer to leave it in.
									if (!pinfo->IsGameOver())
										StartMenu(pinfo);
								}
							}

							// Process local key events
							if (pie->type == RInputEvent::Key && pie->sUsed == FALSE)
								{
								// This is the switch for things that don't want specific RSP_GKF_* modifier keys
								switch (pie->lKey & 0x0000FFFF)
									{
									case KEY_NEXT_LEVEL:
										if (pinfo->IsMP())
											{
											// Only the server's local user can advance to the next level, but even
											// then only when the game is playing.  The actual handling of this
											// is done elsewhere -- we just set the flag here.
											if (pinfo->IsServer() && pinfo->Client()->IsPlaying())
												pinfo->m_bNextRealmMP = true;
											}
										else
											{
											// If sales demo cheat is enabled, we can go to the next level
											#if defined(SALES_DEMO)
												if (g_bEnableLevelAdvanceWithoutGoal)
													pinfo->SetGameState_NextRealm();
											#endif // SALES_DEMO

											// Normally, all that happens when you press this key is that we
											// set a flag that gets passed to the function that determines whether
											// the end-of-level-goal has been reached, which may or may not depend
											// on the player actually pressing the key.
											bEndLevelKey = true;

											pie->sUsed	= TRUE;
											}
										break;

									case KEY_PAUSE:
										// Only allow pause if we're NOT in MP mode and we're in live mode
										if (!pinfo->IsMP() && (GetInputMode() == INPUT_MODE_LIVE))
											{
											PauseGame(pinfo->Realm(), "Press <Pause> key to resume", KEY_PAUSE);

											pie->sUsed	= TRUE;
											}
										break;

									case KEY_MENU:
										// If not chatting . . .
										if (pinfo->m_bChatting == false)
											{
											// Start the menu.  I have no idea why the check to make sure the game
											// is not over was necessary, but it doesn't seem like it could hurt,
											// so it was safer to leave it in.
											if (!pinfo->IsGameOver())
												StartMenu(pinfo);

											pie->sUsed	= TRUE;
											}
										break;

									case KEY_TOGGLE_TARGETING:
										// Toggle game settings flag
										g_GameSettings.m_sCrossHair = !g_GameSettings.m_sCrossHair;

										// Toggle local dude's flag (if he exists)
										if (pdudeLocal != NULL)
											pdudeLocal->m_bTargetingHelpEnabled = (g_GameSettings.m_sCrossHair != FALSE) ? true : false;

										pie->sUsed	= TRUE;
										break;

									case KEY_SHOW_MISSION:
										// Show the mission goal line again for about 5 seconds.
										ScoreDisplayStatus(prealm);

										pie->sUsed	= TRUE;
										break;

									case KEY_ENLARGE_FILM1:
									case KEY_ENLARGE_FILM2:
									case KEY_ENLARGE_FILM3:
										// Increase film scale
										g_GameSettings.m_dGameFilmScale += FILM_INCDEC_SCALE;

										pie->sUsed	= TRUE;
										break;
									
									case KEY_REDUCE_FILM1:
									case KEY_REDUCE_FILM2:
										// Decrease film scale
										g_GameSettings.m_dGameFilmScale -= FILM_INCDEC_SCALE;

										pie->sUsed	= TRUE;
										break;

									case KEY_TOGGLE_DISP_INFO:
										// Toggle display info flag.
										if (g_GameSettings.m_sDisplayInfo == FALSE)
											g_GameSettings.m_sDisplayInfo	= TRUE;
										else
											g_GameSettings.m_sDisplayInfo	= FALSE;

										pie->sUsed	= TRUE;
										break;

									case KEY_TALK1:
									case KEY_TALK2:
										if (m_peditChatIn && pinfo->IsMP() && pinfo->m_bChatting == false)
											{
											// Activate talk mode.
											pinfo->m_bChatting	= true;

											m_peditChatIn->SetVisible(TRUE);

											pie->sUsed	= TRUE;
											}
										break;
									}

								// If in talk mode . . .
								if (pinfo->m_bChatting == true && m_peditChatIn && pinfo->IsMP() )
									{
									switch (pie->lKey)
										{
										case KEY_ACCEPT_CHAT:
											// If anything was typed . . .
											if (m_peditChatIn->m_szText[0])
												{
												// Send the chat text.
												pinfo->Client()->SendChat(m_peditChatIn->m_szText);
												}

											// Intentional fall through.

										case  KEY_ABORT_CHAT:	// Cancel jumps in here to clean up but not send anything.

											// If we're at all off of the view edge . . .
											if (pinfo->Camera()->m_sFilmViewX > 0)
												{
												// Erase now.
												rspGeneralLock(g_pimScreenBuf);

												rspRect(
													RSP_BLACK_INDEX,
													g_pimScreenBuf,
													m_peditChatIn->m_sX,
													m_peditChatIn->m_sY,
													m_peditChatIn->m_im.m_sWidth,
													m_peditChatIn->m_im.m_sHeight);

												rspGeneralUnlock(g_pimScreenBuf);

												// Add dirty rectangle to update erased area.
												pinfo->m_drl.Add(
													m_peditChatIn->m_sX,
													m_peditChatIn->m_sY,
													m_peditChatIn->m_im.m_sWidth,
													m_peditChatIn->m_im.m_sHeight);
												}

											// Clear the chat text.
											m_peditChatIn->SetText("");
											m_peditChatIn->Compose();
											m_peditChatIn->SetVisible(FALSE);

											// Reset the input.
											ClearLocalInput();

											pinfo->m_bChatting	= false;
											break;

										default:
											m_peditChatIn->Do(pie);
											break;
										}
									}
								}
							else
								{
								// If we're in chat mode, even if there's no input for the chat box,
								// we need to call the Edit's Do() so it can flash the caret and stuff.
								if (pinfo->m_bChatting == true && m_peditChatIn && pinfo->IsMP() )
									{
									m_peditChatIn->Do(pie);
									}
								}

							// Note that this key's status element in the key status array could be determined once and
							// stored statically, but, if we do this, the key cannot be changed during gameplay.  That
							// is, if the user changed the key to say Caps Lock, it would not work until the next
							// run of the game.

							// If xray all pressed . . .
							if (m_pau8KeyStatus[KEY_XRAY_ALL])
								{
								// Toggle user choice for XRay all.
								pinfo->m_bXRayAll	= !pinfo->m_bXRayAll;
								// Set new value to the scene.
								prealm->m_scene.SetXRayAll( (pinfo->m_bXRayAll == true) ? TRUE : FALSE);
								// Clear key's status.
								m_pau8KeyStatus[KEY_XRAY_ALL]	= 0;
								}

							// If snap picture pressed . . .
							if (m_pau8KeyStatus[KEY_SNAP_PICTURE])
								{
								// If not chatting . . .
								if (pinfo->m_bChatting == false)
									{
									// If snap shots are enabled, take one
									if (g_GameSettings.m_sCanTakeSnapShots != 0)
										Play_SnapPicture();

									// Clear the key.
									m_pau8KeyStatus[KEY_SNAP_PICTURE]	= 0;
									}
								}

							// If app goes to background, we put up a "pause game" message and wait for it to return
							// to the foreground.  In MP mode, we can't do this or all the other players will freeze!
							if (rspIsBackground() && !pinfo->IsMP())
								PauseGame(prealm, "Make Postal the foreground app to resume", 0);

							// If "live" and not MP . . .
							if ((GetInputMode() == INPUT_MODE_LIVE) && !pinfo->IsMP())
								{
								// If revive key pressed . . .
#ifdef MOBILE
								if (continueIsRestart)
#else
								if (GetInput(0) & INPUT_REVIVE)
#endif
									{
#ifdef MOBILE
									continueIsRestart = false; //Reset this flag for next time
#endif

									bool	bRestart	= false;

									// If there's a local dude . . .
									if (pdudeLocal)
										{
										// If he's dead . . .
										if (pdudeLocal->IsDead() == true)
											{
											// Restart the realm
											bRestart	= true;
											}
										}
									else
										{
										// Restart the realm
										bRestart	= true;
										}

									if (bRestart)
										{
										// Restart the realm
										pinfo->SetGameState_RestartRealm();

										// Check the goal (since the user pressed revive, we'll
										// say the 'end the level' key was pressed.
										if (prealm->IsEndOfLevelGoalMet(true))
											{
											// The goal was met, show the dialog(s).
											ScoreDisplayHighScores(prealm);
											}
										}
									}
								}
							}
						}

					//==============================================================================
					// Check if end of level goal has been met.  Depending on the game type, this
					// may require that the user pressed the end-of-level key.
					//==============================================================================

					if (pinfo->IsMP())
						{
						if (pinfo->IsServer() && pinfo->Client()->IsPlaying())
							{
							if (prealm->IsEndOfLevelGoalMet(bEndLevelKey))
								pinfo->m_bNextRealmMP = true;
							}
						}
					else
						{
						if (prealm->IsEndOfLevelGoalMet(bEndLevelKey))
							{
							// Set so we'll go to the next realm
							pinfo->SetGameState_NextRealm();

#ifdef MOBILE //Disble scores for now, don't work
							// Display high scoers
							ScoreDisplayHighScores(prealm);
#endif
							}
						}

					//==============================================================================
					// If NOT multiplayer mode, get player's input here.  Otherwise, player input
					// is handled by the network stuff.
					//==============================================================================

					if (!pinfo->IsMP())
						{
						// Set controls for the one-and-only dude now (allow cheats).
						SetInput(0, GetLocalInput(prealm, pie));
						}

					//==============================================================================
					// Demo mode stuff
					//==============================================================================

					if (GetInputMode() != INPUT_MODE_LIVE)
						{
						// If the local dude dies, we wait a short period of time and then end the game.
						if (pdudeLocal != NULL)
							{
							if (pdudeLocal->IsDead() == true)
								{
								// If this is the first time here, reset the timer
								if (m_lDemoDeadTime < 0)
									m_lDemoDeadTime = prealm->m_time.GetGameTime();

								// If he's been dead long enough, end the game
								if (prealm->m_time.GetGameTime() > m_lDemoDeadTime + DEMO_MAX_DEAD_TIME)
									pinfo->SetGameState_GameOver();
								}
							}
						else
							{
							// If the dude goes away, end the game right away
							pinfo->SetGameState_GameOver();
							}

						// If we're in Playback mode . . . 
						if (GetInputMode() == INPUT_MODE_PLAYBACK)
							{
							// If the input is done . . .
							if (InputIsDemoOver() == true)
								{
								// Game be done now.
								pinfo->SetGameState_GameOver();
								}
							}

						// Govern the speed of the loop
						while (prealm->m_time.GetRealTime() - prealm->m_time.GetGameTime() < 0)
							;
						}
					}
				}
			}


		////////////////////////////////////////////////////////////////////////////////
		// Unprepare game
		////////////////////////////////////////////////////////////////////////////////
		virtual
		void UnprepareGame(
			CPlayInfo* pinfo)										// I/O: Play info
			{
			// If we're still on the menu, end it now
			if (pinfo->m_bInMenu)
				StopMenu(pinfo);
			}


		////////////////////////////////////////////////////////////////////////////////
		// Core loop render -- create and update images to the composite buffer but do
		// NOT update the screen.
		////////////////////////////////////////////////////////////////////////////////
		/* virtual */
		void CoreLoopRenderOnTop(
			CPlayInfo* pinfo)										// I/O: Play info
			{
			if (!pinfo->m_bBadRealmMP)
				{
				// Only if we're not on the menu
				if (!pinfo->m_bInMenu)
					{
					// If in chat input mode . . .
					if (pinfo->m_bChatting && m_peditChatIn)
						{
						m_peditChatIn->Draw(g_pimScreenBuf);

						// Add dirty rectangle.
						pinfo->m_drl.Add(
							m_peditChatIn->m_sX,
							m_peditChatIn->m_sY,
							m_peditChatIn->m_im.m_sWidth,
							m_peditChatIn->m_im.m_sHeight);
						}
					}
				}
			}

		
		////////////////////////////////////////////////////////////////////////////////
		//
		// Pause the specified realm, fade colors to red, display specified message,
		// and wait for specified key or focus.
		// When the key is hit, or foreground status is regained, the color are faded
		// back from red, the realm is resumed, and the function returns.
		//
		////////////////////////////////////////////////////////////////////////////////
		void PauseGame(
			CRealm*	prealm,			// In:  Realm to pause or NULL.
			char*		pszMsg,			// In:  Message to be displayed.
			long		lKey)				// In:  Key to continue or 0 to wait for foreground status
			{
			// Suspend realm.
			if (prealm)
				{
				prealm->Suspend();
				}

			// Fade colors to red.
			PalTranOn();

			// Create colors.

			// 'PAUSED' fore color.
			rspSetPaletteEntry(
				PAUSED_BASE_PAL_INDEX + 0,			// Palette entry (0 to 255)
				PAUSED_FONT_COLOR_R,					// Red component (0 to 255)
				PAUSED_FONT_COLOR_G,					// Green component (0 to 255)
				PAUSED_FONT_COLOR_B);				// Blue component (0 to 255)
															
			// 'PAUSED' shadow color.				
			rspSetPaletteEntry(						
				PAUSED_BASE_PAL_INDEX + 1,			// Palette entry (0 to 255)
				PAUSED_FONT_SHADOW_COLOR_R,		// Red component (0 to 255)
				PAUSED_FONT_SHADOW_COLOR_G,		// Green component (0 to 255)
				PAUSED_FONT_SHADOW_COLOR_B);		// Blue component (0 to 255)
															
			// Message fore color.					
			rspSetPaletteEntry(						
				PAUSED_BASE_PAL_INDEX + 2,			// Palette entry (0 to 255)
				PAUSED_MSG_FONT_COLOR_R,			// Red component (0 to 255)
				PAUSED_MSG_FONT_COLOR_G,			// Green component (0 to 255)
				PAUSED_MSG_FONT_COLOR_B);			// Blue component (0 to 255)

			// Message shadow color.
			rspSetPaletteEntry(
				PAUSED_BASE_PAL_INDEX + 3,			// Palette entry (0 to 255)
				PAUSED_MSG_FONT_SHADOW_COLOR_R,	// Red component (0 to 255)
				PAUSED_MSG_FONT_SHADOW_COLOR_G,	// Green component (0 to 255)
				PAUSED_MSG_FONT_SHADOW_COLOR_B);	// Blue component (0 to 255)

			// Update hardware palette.
			rspUpdatePalette();

			// Lock the buffer before drawing to it.
			rspLockBuffer();

			// Draw text.
			RPrint	print;
			print.SetFont(PAUSED_FONT_HEIGHT, &g_fontPostal);
			print.SetEffectAbs(RPrint::SHADOW_X, PAUSED_FONT_SHADOW_X);
			print.SetEffectAbs(RPrint::SHADOW_Y, PAUSED_FONT_SHADOW_Y);

			print.SetColor(
				PAUSED_BASE_PAL_INDEX + 0, 
				0, 
				PAUSED_BASE_PAL_INDEX + 1);

			print.SetDestination(g_pimScreenBuf);
			print.SetJustifyCenter();

			short	sTotalH	= PAUSED_FONT_HEIGHT + PAUSED_FONT_SHADOW_Y;
			if (pszMsg)
				{
				// Include message height as well.
				sTotalH	+= PAUSED_MSG_FONT_HEIGHT + PAUSED_MSG_FONT_SHADOW_Y;
				}

			short	sPosY		= g_pimScreenBuf->m_sHeight / 2 - sTotalH; // / 2;

			print.print(
				0,
				sPosY,
				"PAUSED");

			if (pszMsg)
				{
				sPosY	+= PAUSED_FONT_HEIGHT + PAUSED_FONT_SHADOW_Y;

				print.SetFont(PAUSED_MSG_FONT_HEIGHT);
				print.SetEffectAbs(RPrint::SHADOW_X, PAUSED_MSG_FONT_SHADOW_X);
				print.SetEffectAbs(RPrint::SHADOW_Y, PAUSED_MSG_FONT_SHADOW_Y);
				
				print.SetColor(
					PAUSED_BASE_PAL_INDEX + 2, 
					0, 
					PAUSED_BASE_PAL_INDEX + 3);

				print.print(
					0,
					sPosY,
					"%s",
					pszMsg);
				}

			// Unlock the buffer now that we're done drawing to it.
			rspUnlockBuffer();

			// Update the screen with the text.
			rspUpdateDisplay();

			// Loop until signaled to continue.
			bool	bResume	= false;
			RInputEvent		ie;
			while (bResume == false)
				{
				UpdateSystem();

				if (lKey)
					{
					ie.type	= RInputEvent::None;
					rspGetNextInputEvent(&ie);
					if (ie.type == RInputEvent::Key)
						{
						if ((ie.lKey & 0x0000FFFF) == lKey)
							{
							bResume	= true;
							}
						}
					}
				else
					{
					if (rspIsBackground() == FALSE)
						{
						bResume	= true;
						}
					}

				if (rspGetQuitStatus() )
					{
					bResume	= true;
					}
				}

			// Fade colors back from red.
			PalTranOff();

			// Resum realm.
			if (prealm)
				{
				prealm->Resume();
				}

			// Clear queues.
			rspClearAllInputEvents();

			// Re-init input.
			ClearLocalInput();
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Start the menu
		//
		////////////////////////////////////////////////////////////////////////////////
		void StartMenu(
			CPlayInfo* pinfo)										// I/O: Play info
			{
			// If not in multiplayer mode, suspend the realm while on the menu
			if (!pinfo->IsMP())
				pinfo->Realm()->Suspend();

			// Fade out colors -- for MP do it fast to avoid hanging the game up
			if (pinfo->IsMP())
				PalTranOn(0);
			else
				PalTranOn();

			// Clear input events.
			rspClearAllInputEvents();

#ifdef MOBILE
			if (pinfo->LocalDudePointer()->IsDead()) //Only enable RETRY if player is dead
				menuClientGame.ami[0].sEnabled = TRUE;
			else
				menuClientGame.ami[0].sEnabled = FALSE;
#endif
			// Disable 'Play Options' on 'Options' menu.
			menuOptions.ami[5].sEnabled	= FALSE;
			// Disable 'Organ' on 'Audio Options' menu.
			menuAudioOptions.ami[1].sEnabled	= FALSE;
			// Disable 'Save' IF in multiplayer.
			menuClientGame.ami[1].sEnabled = (pinfo->IsMP() == true) ? FALSE : TRUE;

			// Start the menu
			if (::StartMenu(&menuClientGame, &g_resmgrShell, g_pimScreenBuf) == 0)
				{
				// Disable autopump.
				RMix::SetAutoPump(FALSE);

				// Disable Camera's screen access by making the view really friggin small.
				pinfo->Camera()->SetViewSize(0, 0);

				// Set flag to indicate we're in the menu
				pinfo->m_bInMenu = true;
#ifdef MOBILE
	AndroidSetScreenMode(TOUCH_SCREEN_MENU);
#endif
				}
			else
				{
				// Clean up
				StopMenu(pinfo);
				}
			}

		////////////////////////////////////////////////////////////////////////////////
		//
		// Do one iteration of the menu (must have called StartMenu() previously!!!)
		//
		////////////////////////////////////////////////////////////////////////////////
		void DoMenu(
			CPlayInfo* pinfo,										// I/O: Play info
			RInputEvent* pie)										// I/O: Input event
			{
			// Make sure StartMenu() was called
			ASSERT(pinfo->m_bInMenu);

			// Run the menu
			ms_menuaction = MenuActionNone;

			DoMenuInput(pie, g_InputSettings.m_sUseJoy);

			switch (ms_menuaction)
				{
				// Nothing in particular.
				case MenuActionNone:
					break;
				// User quit choice.
				case MenuActionQuit:
					StopMenu(pinfo);
					// User hit "Quit" in the menu; end the game.
					if (pinfo->IsMP())
						pinfo->m_bUserQuitMP = true;
					else
						pinfo->SetGameState_GameAborted();
					break;
				// User save choice.
				case MenuActionSaveGame:
					{
					short sResult;
					// Static so dialog will "remember" the previously-used name
					static char	szFile[RSP_MAX_PATH]	= "";

					// If not yet used, start out in appropriate directory
					if (szFile[0] == '\0')
						strcpy(szFile, FullPathHD(SAVEGAME_DIR));

					// Display open dialog to let user choose a file
					#if 1 //PLATFORM_UNIX
					if (PickFile("Choose Game Slot", EnumSaveGamesSlots, szFile, sizeof(szFile)) == 0)
                    {
#ifdef MOBILE
						//Android we have the format "1 - date"
						//Need to create the filename
						char number = szFile[0];
						snprintf(szFile, sizeof (szFile), "%s/%c.gme", SAVEGAME_DIR,number);
#else
						char *ptr = strrchr(szFile, '[');
						if (ptr) *(ptr-1) = '\0';
#endif

						// This function will open the saved game file and set the correct game mode
						// and settings.  Note that this modifies the m_action (that's how we get
						// out this state...this confused me for a while but it seems like a good
						// way to choose the appropriate original action).
						if (Game_SavePlayersGame(szFile, pinfo->Realm()->m_flags.sDifficulty) == SUCCESS)
						{
							#if WITH_STEAMWORKS
							if ((EnableSteamCloud) && (strncmp(szFile, "steamcloud/", 11) == 0))
							{
								char fname[64];
								snprintf(fname, sizeof (fname), "savegame_%s", szFile + 11);
								ISteamRemoteStorage *cloud = SteamRemoteStorage();
								if (cloud)
								{
									FILE *io = fopen(FindCorrectFile(szFile, "rb"), "rb");
									if (io)
									{
										char buf[1024];
										const size_t br = fread(buf, 1, sizeof (buf), io);
										fclose(io);
										if (br > 0)
											cloud->FileWrite(fname, buf, (int32) br);
									}
								}
							}
							#endif
						}
					}
					#else
						#if WITH_STEAMWORKS
						#error You need to switch over from this code to the in-game file UI first.
						#endif
					sResult = rspSaveBox(g_pszSaveGameTitle, szFile, szFile, sizeof(szFile), SAVEGAME_EXT);
					if (sResult == 0)
						{
						if (Game_SavePlayersGame(szFile, pinfo->Realm()->m_flags.sDifficulty) != SUCCESS)
							{
							rspMsgBox(RSP_MB_ICN_EXCLAIM | RSP_MB_BUT_OK, g_pszSaveGameErrorTitle,
							 g_pszSaveGameErrorText);
							}
						}
                    #endif

					break;
					}
				case MenuActionEndMenu:
					StopMenu(pinfo);
					break;
				default:
					TRACE("RespondToMenuRequest(): Unhandled action %d.\n", ms_menuaction);
					break;
				}
	
			DoMenuOutput(pinfo->Camera()->m_pimFilm);

			ms_menuaction	= MenuActionNone;

			// This is CHEEZY AS HELL but the normal menu callback calls
			// game.cpp which sets its action flag telling it to call this
			// function.  Not sure how to do it here.  Will we need to call
			// game.cpp, play.cpp, and gameedit.cpp whenever this menu is
			// activated?
			Menu*	pmenu	= GetCurrentMenu();
			if (pmenu == &menuJoystick || pmenu == &menuMouse || pmenu == &menuKeyboard)
				{
				// Do the input settings.
				EditInputSettings();
				}

			rspUpdateDisplay();
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// End the menu
		//
		////////////////////////////////////////////////////////////////////////////////
		void StopMenu(
			CPlayInfo* pinfo)										// I/O: Play info
			{
			// End the menu
			::StopMenu();

			// Update the display reflecting the erasure of the menu.
			rspUpdateDisplay();

			// Re-enable autopump.
			RMix::SetAutoPump(TRUE);

			// Set the local dude's color in case the user changed it on the menu.  In MP
			// mode we have to ignore this because we currently don't support the messages that
			// would be necessary to tell all the other players about the color change.
			if (!pinfo->IsMP())
				{
				CDude* pdudeLocal = pinfo->LocalDudePointer();
				if (pdudeLocal)
					pdudeLocal->m_sTextureIndex = MAX((short)0, MIN((short)(CDude::MaxTextures - 1), g_GameSettings.m_sPlayerColorIndex));
				}

			// Re-enable 'Play Options' on 'Options' menu.
			menuOptions.ami[5].sEnabled		= TRUE;
			// Re-enable 'Organ' on 'Audio Options' menu.
			menuAudioOptions.ami[1].sEnabled	= TRUE;

			// Fade colors back in
			PalTranOff();

			// Clear queues.
			rspClearAllInputEvents();

			// Clear the local input.
			ClearLocalInput();

			// If not in multiplayer mode, resume the realm
			if (!pinfo->IsMP())
				pinfo->Realm()->Resume();

			// Restore camera's screen access.
			pinfo->Camera()->SetViewSize(
				VIEW_W * g_GameSettings.m_dGameFilmScale,
				VIEW_H * g_GameSettings.m_dGameFilmScale);

			// Clear flag
			pinfo->m_bInMenu = false;
#ifdef MOBILE
	AndroidSetScreenMode(TOUCH_SCREEN_GAME);
#endif
			}
	};


////////////////////////////////////////////////////////////////////////////////
//
// Realm API
//
////////////////////////////////////////////////////////////////////////////////
class CPlayRealm : public CPlay
	{
	//------------------------------------------------------------------------------
	// Types, enums, etc.
	//------------------------------------------------------------------------------
	private:
		typedef struct
			{
			CStockPile				stockpile;
			CDude::WeaponType		weapon;
			} LevelPersist;


	//------------------------------------------------------------------------------
	// Variables
	//------------------------------------------------------------------------------
	private:
		LevelPersist	m_alevelpersist[Net::MaxNumIDs];	// Index by CDude::m_sDudeNum.
		bool				m_bMakeDemoMovie_WaitForClick;	// Flag used when making demo movies
		double			m_dCurrentFilmScale;
		short				m_sCurrentGripZoneRadius;
		long				m_lNumSeqSkippedFrames;


	//------------------------------------------------------------------------------
	// Functions
	//------------------------------------------------------------------------------
	public:
		////////////////////////////////////////////////////////////////////////////////
		// Constructor
		////////////////////////////////////////////////////////////////////////////////
		CPlayRealm(void)
			{
			}


		////////////////////////////////////////////////////////////////////////////////
		// Destructor
		////////////////////////////////////////////////////////////////////////////////
		/* virtual */
		~CPlayRealm()
			{
			}


		////////////////////////////////////////////////////////////////////////////////
		// Prepare game
		////////////////////////////////////////////////////////////////////////////////
		/* virtual */
		short PrepareGame(										// Returns 0 if successfull, non-zero otherwise
			CPlayInfo* pinfo)										// I/O: Play info
			{
			// Note whether multiplayer.
			pinfo->Realm()->m_flags.bMultiplayer = pinfo->IsMP();

			// Array of LevelPersist to carry players' ammo, health, kevlar, current
			// weapon, etc. from level to level.  Using CDudes in this manner was 
			// another idea, but when I tried carrying them from level to level, many, 
			// many possibilities for error were revealed.  For example, various realm 
			// dependent things in the CDude and his base classes are cleaned up by 
			// those respective classes in the destructor (e.g., his smash and his 
			// sprite and, in some cases, a child object).
			// Initialize to appropriate values.
			short sDudeIndex;
			for (sDudeIndex = 0; sDudeIndex < Net::MaxNumIDs; sDudeIndex++)
				{
				// Clear stockpile.
				m_alevelpersist[sDudeIndex].stockpile.Zero();

				// Make machine gun the default weapon.
				m_alevelpersist[sDudeIndex].weapon	= CDude::SemiAutomatic;
				}

			// Debug demo mode stuff (always active -- takes essentially no time unless enabled from game.cpp)
			m_bMakeDemoMovie_WaitForClick = true;
			
			return 0;
			}


		////////////////////////////////////////////////////////////////////////////////
		// Prepare realm
		////////////////////////////////////////////////////////////////////////////////
		/* virtual */
		short PrepareRealm(										// Returns 0 if successfull, non-zero otherwise
			CPlayInfo* pinfo)										// I/O: Play info
			{
			short sResult = 0;

			CRealm* prealm = pinfo->Realm();

			// Clear realm (in case there's any crap left over from last realm)
			prealm->Clear();

			// If we're supposed to purge the SAKs . . .
			if (pinfo->PurgeSaks() == true)
				{
				// We must do this after the Clear() to make sure all the objects
				// have been deleted so that they'll release their resources.
				// Also, we must, of course, do this before the Load().
				g_resmgrGame.Purge();
				g_resmgrSamples.Purge();
				g_resmgrRes.Purge();
				g_resmgrShell.Purge();

				// Clear the flag.
				pinfo->ClearPurgeSaks();
				}

			// Reset time here so that objects can use it when they are loaded
			prealm->m_time.Reset();

			// If there's already a realm error, then don't bother with this
			if (!pinfo->m_bBadRealmMP)
				{
				// Check if specified file exists
				if (prealm->DoesFileExist((char*)pinfo->RealmName()))
					{
					// Load realm (false indicates NOT edit mode)
					if (prealm->Load((char*)pinfo->RealmName(), false) == 0)
						{
						// Startup the realm
						if (prealm->Startup() == 0)
							{

							//==============================================================================
							// Set up dudes.  This can take a while (dude's need lots of resources) so it
							// should be done before the cutscene stuff.
							//==============================================================================

							// If this game was loaded from a saved game file, then copy the
							// global stockpile into the level persistent data array.
							if (g_bTransferStockpile)
								m_alevelpersist[0].stockpile.Copy(&g_stockpile);
							// Otherwise, keep the global savable stockpile up to date.
							else
								g_stockpile.Copy(&m_alevelpersist[0].stockpile);

							g_bTransferStockpile = false;

							// Set up as many dudes as needed and get pointer to local dude
							sResult = SetupDudes(pinfo, m_alevelpersist);
							}
						else
							{
							sResult = -1;
							TRACE("CPlayRealm::PrepareRealm(): Error starting-up realm!\n");
							}
						}
					else
						{
						sResult = -1;
						TRACE("CPlayRealm::PrepareRealm(): Error loading realm!\n");
						}
					}
				else
					{
					sResult = -1;
					TRACE("CPlayRealm::PrepareRealm(): File does not exist: %s\n", (char*)pinfo->RealmName());

					// If we're in the specific realm mode, then display a message telling the user that
					// this version only handles one specific realm.  Otherwise, this shouldn't happen
					// to a user of a normal version, so we don't say anything.
					#if defined(ENABLE_PLAY_SPECIFIC_REALMS_ONLY)
						// MP is a special case that is handled below
						if (!pinfo->IsMP())
							{
							rspMsgBox(
								RSP_MB_ICN_INFO | RSP_MB_BUT_OK,
								g_pszAppName,
								g_pszPlayOneRealmOnlyMessage);
							}
					#endif	// ENABLE_PLAY_SPECIFIC_REALMS_ONLY
					}

				// If there was an error, and this is an MP game, then we ignore the error for now,
				// and instead we set a flag saying the realm is bad.  This is done so we can handle
				// the error as part of the core loop, which is where similar errors are already handled.
				if ((sResult != 0) && pinfo->IsMP())
					{
					sResult = 0;
					pinfo->m_bBadRealmMP = true;
					}
				}

			return sResult;
			}


		////////////////////////////////////////////////////////////////////////////////
		// Start realm
		////////////////////////////////////////////////////////////////////////////////
		/* virtual */
		short StartRealm(											// Returns 0 if successfull, non-zero otherwise
			CPlayInfo* pinfo)										// I/O: Play info
			{
			if (!pinfo->m_bBadRealmMP)
				{
				// Get realm
				CRealm* prealm = pinfo->Realm();

				// Setup camera
				pinfo->Camera()->SetScene(&(prealm->m_scene));
				pinfo->Camera()->SetHood((CHood*)(prealm->m_aclassHeads[CThing::CHoodID].GetNext() ) );
				pinfo->Camera()->SetView(VIEW_X, VIEW_Y, VIEW_W, VIEW_H);

				// Set grip to control camera.
				pinfo->Grip()->SetCamera(pinfo->Camera());

				// Set hood's palette.
				prealm->m_phood->SetPalette();

				// Setup initial film scaling
				ScaleFilm(pinfo, false);

				// Reset time so that the first time update doesn't show (much) elapsed time.
				prealm->m_time.Reset();

				// Reset
				m_lNumSeqSkippedFrames = 0;
				}

			return 0;
			}


		////////////////////////////////////////////////////////////////////////////////
		// Core loop render -- create and update images to the composite buffer but do
		// NOT update the screen.
		////////////////////////////////////////////////////////////////////////////////
		/* virtual */
		void CoreLoopRender(
			CPlayInfo* pinfo)										// I/O: Play info
			{
			if (!pinfo->m_bBadRealmMP)
				{
				// If scale or radius have changed from our current values, then we need
				// to redo everything now.
				if ((m_dCurrentFilmScale != g_GameSettings.m_dGameFilmScale) ||
					 (m_sCurrentGripZoneRadius != g_GameSettings.m_sGripZoneRadius))
					{
					ScaleFilm(pinfo);
					}

				// Figure out whether to do a frame or not
				bool bDoFrame = true;
				if (pinfo->IsMP())
					{
					// In MP mode, we simply follow the special MP flag.  We don't care about
					// the menu, because we must let the game continue to run so we stay in
					// sync with everyone else.
					bDoFrame = pinfo->m_bDoRealmFrame;
					}
				else
					{
					// If we're in the menu, then DON'T do the frame
					if (pinfo->m_bInMenu)
						bDoFrame = false;
					}

				if (bDoFrame)
					{

					// Get realm pointer
					CRealm* prealm = pinfo->Realm();

					// Adjust realm time.  How we do it depends on the mode we're in.
					if (GetInputMode() == INPUT_MODE_LIVE)
						{
						if (pinfo->IsMP())
							{
							// In multiplayer mode, time moves in hardwired increments
							prealm->m_time.Update(pinfo->FrameTime());
							}
						else
							{
							// In non-network mode, time flows freely
							prealm->m_time.Update();
							}
						}
					else
						{
						// In demo mode, time moves in hardwired increments
						prealm->m_time.Update(DEMO_TIME_PER_FRAME);
						}
					

					// Update Realm
					prealm->Update();

					// Prepare Realm for rendering (Snap()).
					prealm->Render();

					// In demo mode (record or playback) we don't draw the results of the frame if
					// we're falling behind.  However, we always draw when doing a demo-mode-movie .
					pinfo->m_bDrawFrame = true;
					if ((pinfo->DemoModeDebugMovie() == 0) && (GetInputMode() != INPUT_MODE_LIVE))
						{
						// If we've fallen behind the demo frame rate by our max lag . . .
						if (prealm->m_time.GetRealTime() - prealm->m_time.GetGameTime() > DEMO_MAX_LAG)
							{
							// If we haven't already skipped too many frames . . .
							if (m_lNumSeqSkippedFrames < DEMO_MAX_SEQUENTIAL_SKIPPED_FRAMES)
								{
								m_lNumSeqSkippedFrames++;
								pinfo->m_bDrawFrame = false;
								}
							else
								m_lNumSeqSkippedFrames	= 0;
							}
						else
							m_lNumSeqSkippedFrames	= 0;
						}

					// Track the local dude with the grip/camera and adjust the sound, too
					CDude* pdudeLocal = pinfo->LocalDudePointer();
					if (pdudeLocal != NULL)
						{
						// Update grip/camera
						short	sX, sY;
						prealm->Map3Dto2D(pdudeLocal->GetX(), pdudeLocal->GetY(), pdudeLocal->GetZ(), &sX, &sY);
						pinfo->Grip()->TrackTarget(sX, sY, 30);

						// Set coordinates for the "ear"
						SetSoundLocation(pdudeLocal->GetX(), pdudeLocal->GetY(), pdudeLocal->GetZ());
						}

					// Snap picture of scene.  Even if we DON'T want to draw this frame, we still
					// have to allow a certain amount of work to get done (we still need things like
					// collision areas to be updated via the 3D scene rendered).  The scene flag tells
					// the scene whether or not to do BLiT's (and anything else that's purely cosmetic.)
					g_bSceneDontBlit = !pinfo->m_bDrawFrame;
					pinfo->Camera()->Snap();
					g_bSceneDontBlit = false;

					// If in MP mode, clear the flag
					if (pinfo->IsMP())
						pinfo->m_bDoRealmFrame = false;
					}
				else
					{
					// 11/18/97	JMI	This didn't seem to get cleared in the case bDoFrame
					//						is false but I didn't see why we'd need to update the
					//						screen in this case (perhaps this is part of our net
					//						slow down?).
					pinfo->m_bDrawFrame	= false;
					}

				// If not in menu . . .
				if (pinfo->m_bInMenu == false)
					{
					// If we should draw the frame . . .
					if (pinfo->m_bDrawFrame)
						{
						CCamera* pcamera = pinfo->Camera();

						pinfo->m_drl.Add(
							pcamera->m_sFilmViewX, 
							pcamera->m_sFilmViewY, 
							pcamera->m_sViewW, 
							pcamera->m_sViewH);
						}
					}
				}
			}


		////////////////////////////////////////////////////////////////////////////////
		// Core loop draw -- Draw CoreLoopRender() results to the screen.
		////////////////////////////////////////////////////////////////////////////////
		virtual
		void CoreLoopDraw(
			CPlayInfo* pinfo)										// I/O: Play info
			{
			// Only if we're not on the menu and not a bad realm
			if (!pinfo->m_bInMenu && !pinfo->m_bBadRealmMP)
				{
				// If we did draw the frame, we need to copy the results to the screen
				if (pinfo->m_bDrawFrame)
					{
					// Special demo-mode debug stuff
					if (pinfo->DemoModeDebugMovie() && (GetInputMode() != INPUT_MODE_LIVE))
						MakeDemoMovie(pinfo->DemoModeDebugMovie());
					}
				}
			}


		////////////////////////////////////////////////////////////////////////////////
		// End realm
		////////////////////////////////////////////////////////////////////////////////
		/* virtual */
		void EndRealm(
			CPlayInfo* pinfo)										// I/O: Play info
			{
			if (!pinfo->m_bBadRealmMP)
				{
				CRealm* prealm = pinfo->Realm();

				// If we're not simply restarting the level . . .
				if (pinfo->IsRestartingRealm() == false)
					{
					// Update players' stockpiles.
					CListNode<CThing>*	plnDude		= prealm->m_aclassHeads[CThing::CDudeID].m_pnNext;
					CListNode<CThing>*	plnDudeTail	= &(prealm->m_aclassTails[CThing::CDudeID]);
					while (plnDude != plnDudeTail)
						{
						CDude* pdude = (CDude*)plnDude->m_powner;
						m_alevelpersist[pdude->m_sDudeNum].stockpile.Copy( &(pdude->m_stockpile) );
						m_alevelpersist[pdude->m_sDudeNum].weapon = pdude->GetCurrentWeapon();
						plnDude	= plnDude->m_pnNext;
						}
					}

				// Shutdown realm
				prealm->Shutdown();
				}
			}


	private:
		////////////////////////////////////////////////////////////////////////////////
		// Setup local dude
		////////////////////////////////////////////////////////////////////////////////
		void SetupLocalDude(
			CPlayInfo*		pinfo,								// I/O: Play info
			CDude* pdude)											// In:  Dude to setup
			{
			// Get local dude's ID
			pinfo->m_idLocalDude = pdude->GetInstanceID();

			// Turn on local dude's xray effect
			pdude->m_sprite.m_sInFlags |= CSprite::InXrayee;

			// Set local dude's targeting status
			pdude->m_bTargetingHelpEnabled = (g_GameSettings.m_sCrossHair != FALSE) ? true : false;
			}


		////////////////////////////////////////////////////////////////////////////////
		// Setup general dude
		////////////////////////////////////////////////////////////////////////////////
		void SetupGeneralDude(
			CDude* pdude,											// In:  Dude to setup
			short sColor,											// In:  Player's color
			LevelPersist*	palevelpersist)					// In:  Players' level persistent data.
			{
			// Union player's pre-existing stockpile with warped-in dude and give him his prior weapon
			ASSERT(pdude != NULL);
			pdude->m_stockpile.Union(&(palevelpersist[pdude->m_sDudeNum].stockpile));
			pdude->SetWeapon(palevelpersist[pdude->m_sDudeNum].weapon, true);

			// Set dude's color
			pdude->m_sTextureIndex = sColor;
			if (pdude->m_sTextureIndex < 0)
				pdude->m_sTextureIndex = 0;
			if (pdude->m_sTextureIndex >= CDude::MaxTextures)
				pdude->m_sTextureIndex = CDude::MaxTextures - 1;
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Setup dudes in this realm based on the specified parameters.
		//
		// This function is designed to work properly with (1) levels that have CDude's
		// but no CWarps, (2) levels that have CWarp's but no CDude's, and (3) levels
		// that have a combination of CWarp's and CDude's.
		//
		// If this function completes successfully, the specified number of CDude's
		// will exist (no more, no less).
		//
		////////////////////////////////////////////////////////////////////////////////
		short SetupDudes(
			CPlayInfo*		pinfo,								// I/O: Play info
			LevelPersist*	palevelpersist)					// In:  Players' level persistent data.
			{
			short sResult = 0;

			CRealm* prealm = pinfo->Realm();

			// Always default to nil for safety!
			pinfo->m_idLocalDude = CIdBank::IdNil;

			//------------------------------------------------------------------------------
			// This is for backwards compatibility with VERY OLD realms that used CDude's
			// to determine where dude's started out and what they got.  This is completely
			// obsolete, since we now use CWarp's instead.
			//
			// If there are no CDude's, then we don't do anything.  If there are, we
			// convert them into CWarp's.  The CWarp's get their settings from the first
			// CDude we find, and all subsequent CWarp's use those same settings.
			//
			// After this point, there will be NO DUDE'S, either because there weren't any
			// to start with or because we converted them into warps.
			//------------------------------------------------------------------------------
			CListNode<CThing>*	pln		= prealm->m_aclassHeads[CThing::CDudeID].m_pnNext;
			CListNode<CThing>*	plnTail	= &(prealm->m_aclassTails[CThing::CDudeID]);
			CDude*	pdude;
			CWarp*	pwarp;
			bool		bFirst = true; 
			while (pln != plnTail)
				{
				CListNode<CThing>*	plnNext	= pln->m_pnNext;

				pdude	= (CDude*)pln->m_powner;
				if (CWarp::CreateWarpFromDude(prealm, pdude, &pwarp, bFirst) == 0)
					bFirst = false;
				else
					TRACE("SetupDudes(): CWarp::CreateWarpFromDude() failed.\n");

				delete pdude;
				pdude = 0;

				pln = plnNext;
				}

			//------------------------------------------------------------------------------
			// Here, we warp-in as many dude's as we need.  If there are no warps, it
			// probably means the realm wasn't designed correctecly, and we bail out.
			//------------------------------------------------------------------------------
			if (prealm->m_asClassNumThings[CThing::CWarpID] > 0)
				{
				// Setup warp pointers
				CListNode<CThing>*	plnWarpHead	= &(prealm->m_aclassHeads[CThing::CWarpID]);
				CListNode<CThing>*	plnWarp		= plnWarpHead->m_pnNext;
				CListNode<CThing>*	plnWarpTail	= &(prealm->m_aclassTails[CThing::CWarpID]);

				// Multiplayer mode is handled separately
				if (pinfo->IsMP())
					{
					// Get convenient pointer
					CNetClient* pclient = pinfo->Client();

					// Find a random starter.  Pick a number from 0 to n - 1 where n is the
					// number of CWarps in the realm.  Next, iterate to that warp so we start
					// creating dudes at a 'random' warp.
					short	sStartWarpNum	= GetRand() % prealm->m_asClassNumThings[CThing::CWarpID];
					short	i;
					for (i = 0; i < sStartWarpNum; i++, plnWarp = plnWarp->m_pnNext)
						;

					// Warp in as many dude's as we need
					for (Net::ID id = 0; (sResult == 0) && (id < Net::MaxNumIDs); id++)
						{
						// If this player needs a dude
						if (pclient->DoesPlayerNeedDude(id))
							{
							// If we've hit the tail of the warps, wrap around
							if (plnWarp == plnWarpTail)
								plnWarp	= plnWarpHead->m_pnNext;

							pwarp	= (CWarp*)plnWarp->m_powner;
							ASSERT(pwarp != NULL);

							// Warp in dude (creates a new dude since the pointer starts out NULL)
							pdude	= NULL;
							if (pwarp->WarpIn(&pdude, CWarp::CopyStockPile) == 0)
								{
								// SPECIAL CASE!!!  In multiplayer mode, we overwrite the dude numbers
								// that are assigned by the CDude constructor, instead using the
								// corresponding network ID.  This isn't a great solution, but it
								// was the best we could do given the little time we have left.
								ASSERT(pdude != NULL);
								pdude->m_sDudeNum = (short)id;

								// Set general dude stuff
								SetupGeneralDude(pdude, (short)pclient->GetPlayerColor(id), palevelpersist);

								// Set dude's instance ID (not to be confused with network ID)
								pclient->SetPlayerDudeID(id, pdude->GetInstanceID());

								// Special stuff for local dude
								if (id == pclient->GetID())
									SetupLocalDude(pinfo, pdude);
								}
							else
								{
								sResult = -1;
								TRACE("SetupDudes(): pwarp->WarpIn() failed.\n");
								}
							plnWarp	= plnWarp->m_pnNext;
							}
						}
					}
				else
					{
					// Use the first warp
					pwarp	= (CWarp*)plnWarp->m_powner;
					ASSERT(pwarp != NULL);

					// Warp in dude (creates a new dude since the pointer starts out NULL)
					pdude	= NULL;
					if (pwarp->WarpIn(&pdude, CWarp::CopyStockPile) == 0)
						{
						// Set general dude stuff
						SetupGeneralDude(pdude, g_GameSettings.m_sPlayerColorIndex, palevelpersist);

						// Special stuff just for local dude
						SetupLocalDude(pinfo, pdude);
						}
					else
						{
						sResult = -1;
						TRACE("SetupDudes(): pwarp->WarpIn() failed.\n");
						}
					}
				}
			else
				{
				TRACE("SetupDudes(): No warps!!!\n");
				rspMsgBox(RSP_MB_ICN_STOP | RSP_MB_BUT_OK, 
					"Realm Error", 
					"There are no warps in this realm!  There must be at least one warp in a realm!\n");
				sResult = -1;
				}

			return sResult;
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Blanks the specified area of the display.
		//
		////////////////////////////////////////////////////////////////////////////////
		void BlankDisplay(				// Returns nothing.
			short			sX,				// In:  X start position.
			short			sY,				// In:  Y start position.
			short			sW,				// In:  Width.
			short			sH,				// In:  Height
			CPlayInfo*	pinfo)			// Out: Dimensions to update to the display later.
			{
			if (sW > 0 && sH > 0)
				{
				rspLockBuffer();

				rspRect(RSP_BLACK_INDEX, g_pimScreenBuf, sX, sY, sW, sH);

				rspUnlockBuffer();

				pinfo->m_drl.Add(sX, sY, sW, sH);
				}
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Scale the film to g_GameSettings.m_dGameFilmScale.
		//
		////////////////////////////////////////////////////////////////////////////////
		void ScaleFilm(
			CPlayInfo* pinfo,										// I/O: Play info
			bool		bRedraw = true)							// In:  true to clear any newly created dirty areas.
			{
			// Get pointers to camera and grip
			CCamera* pcamera = pinfo->Camera();
			CGrip* pgrip = pinfo->Grip();

			// Remember previous values so we know what portion of the screen needs to be cleared
			short	sOldFilmX = pcamera->m_sFilmViewX;
			short	sOldFilmY = pcamera->m_sFilmViewY;
			short	sOldViewW = pcamera->m_sViewW;
			short	sOldViewH = pcamera->m_sViewH;

			// Clamp the scale to fit the valid range
			if (g_GameSettings.m_dGameFilmScale > FILM_MAX_SCALE)
				g_GameSettings.m_dGameFilmScale	= FILM_MAX_SCALE;
			if (g_GameSettings.m_dGameFilmScale < FILM_MIN_SCALE)
				g_GameSettings.m_dGameFilmScale	= FILM_MIN_SCALE;

			// Scale the actual film.
			short	sViewW = VIEW_W * g_GameSettings.m_dGameFilmScale;
			short sViewH = VIEW_H * g_GameSettings.m_dGameFilmScale;
			short	sFilmX = FILM_X + (VIEW_W - sViewW) / 2;
			short	sFilmY = FILM_Y + (VIEW_H - sViewH) / 2;

			// Update the camera to the new film size.
			pcamera->m_sViewW	= sViewW;
			pcamera->m_sViewH	= sViewH;
			pcamera->SetFilm(g_pimScreenBuf, sFilmX, sFilmY);

			// Update the grip to the new film scaling.
			pgrip->SetParms(
				MAX(short(g_GameSettings.m_sGripZoneRadius * g_GameSettings.m_dGameFilmScale), short(MIN_GRIP_ZONE_RADIUS) ),
				GRIP_MIN_MOVE_X,
				GRIP_MIN_MOVE_Y,
				GRIP_MAX_MOVE_X,
				GRIP_MAX_MOVE_Y,
				GRIP_ALIGN_X,
				GRIP_ALIGN_Y,
				true);

			// If local dude exists, reset the grip's targetting
			CDude* pdudeLocal = pinfo->LocalDudePointer();
			if (pdudeLocal)
				pgrip->ResetTarget(pdudeLocal->GetX(), pdudeLocal->GetZ(), 30);

			// Clear any portion of the screen that was revealed by the change in scale
			if (bRedraw == true)
				{
				// Update revealed zones.
				//  ________
				//	|xxxxxxxx|
				//	|xxxxxxxx|
				//	|**|	|++|
				//	|**|__|++|
				//	|--------|
				//	|--------|

				// Top strip.
				//  ________
				//	|xxxxxxxx|
				//	|xxxxxxxx|
				//	|	|	|	|
				//	|	|__|	|
				//	|			|
				//	|________|
				BlankDisplay(sOldFilmX, sOldFilmY, sOldViewW, sFilmY - sOldFilmY, pinfo);

				// Bottom strip.
				//  ________
				//	|			|
				//	|	 __	|
				//	|	|	|	|
				//	|	|__|	|
				//	|xxxxxxxx|
				//	|xxxxxxxx|
				BlankDisplay( sOldFilmX, sFilmY + sViewH, sOldViewW, (sOldFilmY + sOldViewH) - (sFilmY + sViewH), pinfo);

				// Left strip.
				//  ________
				//	|			|
				//	|	 __	|
				//	|xx|	|	|
				//	|xx|__|	|
				//	|			|
				//	|________|
				BlankDisplay(sOldFilmX, sFilmY, sFilmX - sOldFilmX, sViewH, pinfo);

				// Right strip.
				//  ________
				//	|			|
				//	|	 __	|
				//	|	|	|xx|
				//	|	|__|xx|
				//	|			|
				//	|________|
				BlankDisplay(sFilmX + sViewW, sFilmY, (sOldFilmX + sOldViewW) - (sFilmX + sViewW), sViewH, pinfo);
				}

			// Save new settings so we'll know when they change
			m_dCurrentFilmScale = g_GameSettings.m_dGameFilmScale;
			m_sCurrentGripZoneRadius = g_GameSettings.m_sGripZoneRadius;
			}


		////////////////////////////////////////////////////////////////////////////////
		//
		// Make a demo mode movie for debugging purposes.
		//
		////////////////////////////////////////////////////////////////////////////////
		void MakeDemoMovie(
			RFile* pfileDemoModeDebugMovie)					// In:  File for loading/saving demo mode debug movie
			{
			// The basic idea is that in demo record mode, we save every frame of the
			// game being recorded to a file.  Then, in demo playback mode, we compare
			// each frame of the game as it plays back to the recorded frames, and if
			// there's a difference between the frames, we highlight that difference.
			// From that, we hope that the programmer can figure out his stupid mistake
			// that somehow caused such a difference. :)  Naturally, the actual bug may
			// not be directly related to the visual difference, but it should help.
			if (pfileDemoModeDebugMovie && (GetInputMode() != INPUT_MODE_LIVE))
				{
				if (pfileDemoModeDebugMovie->IsOpen())
					{
					bool bDemoErr = false;
					RImage im;

					if (GetInputMode() == INPUT_MODE_RECORD)
						{
						// In record mode, we create an image, copy the screen buffer to it, and save it
						if (im.CreateImage(VIEW_W, VIEW_H, RImage::BMP8) == 0)
							{
							// Must lock the buffer before reading from it.
							rspLockBuffer();

							rspBlit(g_pimScreenBuf, &im, FILM_X, FILM_Y, 0, 0, VIEW_W, VIEW_H);

							// Done with the composite buffer.
							rspUnlockBuffer();

							if (im.Save(pfileDemoModeDebugMovie) != 0)
								{
								TRACE("PlayRealm(): Error writing demo movie!\n");
								rspMsgBox(RSP_MB_ICN_STOP | RSP_MB_BUT_OK, g_pszAppName, "Error writing demo movie!\n");
								bDemoErr = true;
								}
							}
						else
							{
							TRACE("PlayRealm(): Error create demo image!\n");
							rspMsgBox(RSP_MB_ICN_STOP | RSP_MB_BUT_OK, g_pszAppName, "Error creating demo image!\n");
							bDemoErr = true;
							}
						}
					else
						{
						// In playback mode, we load the previously saved image and compare it to the screen bufer
						if (im.Load(pfileDemoModeDebugMovie) == 0)
							{
							// Must lock the buffer before reading from it.
							rspLockBuffer();

							bool bMatch = true;
							int i;
							U8* pSrcLine = im.m_pData;
							U8* pDstLine = g_pimScreenBuf->m_pData + ((long)FILM_Y * g_pimScreenBuf->m_lPitch) + (long)FILM_X;
							short sHeight = im.m_sHeight;
							U8* pSrc;
							U8* pDst;
							while (sHeight--)
								{
								pSrc = pSrcLine;
								pDst = pDstLine;
								i = im.m_sWidth;
								while (i--) 
									{
									if (*pSrc != *pDst) bMatch = false;
									pDst++;pSrc++;
									}
								pSrcLine += im.m_lPitch;
								pDstLine += g_pimScreenBuf->m_lPitch;
								}

							// If there's a mismatch, highlight the differences between the two frames
							if (!bMatch)
								{
								int i;
								U8* pSrcLine = im.m_pData;
								U8* pDstLine = g_pimScreenBuf->m_pData + ((long)FILM_Y * g_pimScreenBuf->m_lPitch) + (long)FILM_X;
								short sHeight = im.m_sHeight;
								U8* pSrc;
								U8* pDst;
								while (sHeight--)
									{
									pSrc = pSrcLine;
									pDst = pDstLine;
									i = im.m_sWidth;
									while (i--) 
										{
										if (*pSrc == *pDst)
											*pSrc = 0;
										pDst++;pSrc++;
										}
									pSrcLine += im.m_lPitch;
									pDstLine += g_pimScreenBuf->m_lPitch;
									}

								// Copy modified image to screen buffer and update the screen
								rspBlit(&im, g_pimScreenBuf, 0, 0, FILM_X, FILM_Y, VIEW_W, VIEW_H);

								// Done with the composite buffer.
								rspUnlockBuffer();

								rspUpdateDisplay();

								// If wait-for-click is enabled, wait for click.  Otherwise, don't.
								// It will always wait for a click on the first different frame, and thereafter
								// the user can disable the waiting by clicking the right mouse button.
								if (m_bMakeDemoMovie_WaitForClick)
									{
									short sButtons;
									do	{
										rspGetMouse(NULL, NULL, &sButtons);
										UpdateSystem();
										} while (sButtons);
									do {
										rspGetMouse(NULL, NULL, &sButtons);
										UpdateSystem();
										} while (!sButtons);
									if (sButtons & 2)
										m_bMakeDemoMovie_WaitForClick = false;
									do	{
										rspGetMouse(NULL, NULL, &sButtons);
										UpdateSystem();
										} while (sButtons);

									rspClearMouseInputEvents();
									}
								}
							else
								{
								// Done with the composite buffer.
								rspUnlockBuffer();
								}
							}
						else
							{
							TRACE("PlayRealm(): Error reading demo movie!\n");
							rspMsgBox(RSP_MB_ICN_STOP | RSP_MB_BUT_OK, g_pszAppName, "Error reading demo movie!\n");
							bDemoErr = true;
							}
						}

					// If there was an error, close file to turn off demo movie mode
					if (bDemoErr)
						pfileDemoModeDebugMovie->Close();
					}
				}
			}
	};


////////////////////////////////////////////////////////////////////////////////
//
// Score play module
//
////////////////////////////////////////////////////////////////////////////////
class CPlayScore : public CPlay
	{
	//------------------------------------------------------------------------------
	// Types, enums, etc.
	//------------------------------------------------------------------------------
	private:

	//------------------------------------------------------------------------------
	// Variables
	//------------------------------------------------------------------------------
	private:

	//------------------------------------------------------------------------------
	// Functions
	//------------------------------------------------------------------------------
	public:
		////////////////////////////////////////////////////////////////////////////////
		// Constructor
		////////////////////////////////////////////////////////////////////////////////
		CPlayScore(void)
			{
			}


		////////////////////////////////////////////////////////////////////////////////
		// Destructor
		////////////////////////////////////////////////////////////////////////////////
		virtual
		~CPlayScore()
			{
			}


		////////////////////////////////////////////////////////////////////////////////
		// Prepare game
		////////////////////////////////////////////////////////////////////////////////
		/* virtual */
		short PrepareGame(										// Returns 0 if successfull, non-zero otherwise
			CPlayInfo* pinfo)										// I/O: Play info
			{
			// Init and reset score module
			ScoreInit();
			ScoreReset();
			return 0;
			}


		////////////////////////////////////////////////////////////////////////////////
		// Start realm
		////////////////////////////////////////////////////////////////////////////////
		/* virtual */
		short StartRealm(											// Returns 0 if successfull, non-zero otherwise
			CPlayInfo* pinfo)										// I/O: Play info
			{
			if (!pinfo->m_bBadRealmMP)
				{
				// Reset the display timer
				ScoreResetDisplay();

				// Set the scoring type
				if (pinfo->IsMP())
					{
					ScoreSetMode(CScoreboard::MultiPlayer);
					if (pinfo->Realm()->m_ScoringMode == 0)
						pinfo->Realm()->m_ScoringMode = CRealm::MPFrag;
					}
				else
					{
					ScoreSetMode(CScoreboard::SinglePlayer);
					}
				}

			return 0;
			}


		////////////////////////////////////////////////////////////////////////////////
		// End realm
		////////////////////////////////////////////////////////////////////////////////
		/* virtual */
		void EndRealm(
			CPlayInfo* pinfo)										// I/O: Play info
			{
			if (!pinfo->m_bBadRealmMP)
				{
				// If MP mode, check if score should be reset after each level
				if (pinfo->IsMP() && g_GameSettings.m_sHostResetScoresEachLevel)
					ScoreReset();
				}
			}
	};


////////////////////////////////////////////////////////////////////////////////
//
// Base class for all "Play Modules"
//
////////////////////////////////////////////////////////////////////////////////
class CPlayCutscene : public CPlay
	{
	//------------------------------------------------------------------------------
	// Types, enums, etc.
	//------------------------------------------------------------------------------
	private:

	//------------------------------------------------------------------------------
	// Variables
	//------------------------------------------------------------------------------
	private:
		bool		m_bSimple;


	//------------------------------------------------------------------------------
	// Functions
	//------------------------------------------------------------------------------
	public:
		////////////////////////////////////////////////////////////////////////////////
		// Constructor
		////////////////////////////////////////////////////////////////////////////////
		CPlayCutscene(void)
			{
			}


		////////////////////////////////////////////////////////////////////////////////
		// Destructor
		////////////////////////////////////////////////////////////////////////////////
		/* virtual */
		~CPlayCutscene()
			{
			}


		////////////////////////////////////////////////////////////////////////////////
		// Start cutscene
		////////////////////////////////////////////////////////////////////////////////
		/* virtual */
		void StartCutscene(
			CPlayInfo* pinfo)										// I/O: Play info
			{
			// Clear input events (don't let user press anything before the cutscene appears)
			rspClearAllInputEvents();

			// For demo and specific file modes, use simple cutscenes.  Otherwise, use real cutscenes.
			m_bSimple = ((GetInputMode() != INPUT_MODE_LIVE) || (pinfo->RealmNum() < 0)) ? true : false;

			// If this is the spawn version, it only has 1 cutscene image, so it should use
			// simple mode to display that one.
#ifdef SPAWN
			m_bSimple = true;
#endif

			// Special case for the last level demo
			if (g_bLastLevelDemo)
				m_bSimple = false;

			// Start cutscene
			RString strSection;
			RString strEntry;
			Play_GetRealmSectionAndEntry(pinfo->IsMP(), pinfo->CoopLevels(), pinfo->Gauntlet(), pinfo->AddOn(), pinfo->RealmNum(), pinfo->Realm()->m_flags.sDifficulty, &strSection, &strEntry);
			CutSceneStart(m_bSimple, &strSection, &strEntry, 24, 24);
			}


		////////////////////////////////////////////////////////////////////////////////
		// Do cutscene
		////////////////////////////////////////////////////////////////////////////////
		/* virtual */
		void DoCutscene(
			CPlayInfo* pinfo)										// I/O: Play info
			{
			// If this is NOT simple and NOT multiplayer, do the effect while waiting for user input
			if (!m_bSimple && !pinfo->IsMP())
				{
				// Configure cutscene effect
				CutSceneConfig(
					3600, 
					-24,24,10000L,
					-24,24,10000L,
					0.6,0.6,4000L,
					0,0,g_pimScreenBuf->m_sWidth,g_pimScreenBuf->m_sHeight);

				// Insert effects into this loop!
				RInputEvent	ie;
				ie.type	= RInputEvent::None;
				rspClearAllInputEvents();
				while (rspGetQuitStatus() == 0)
					{
					CutSceneUpdate();
					UpdateSystem();
					if (((rspGetNextInputEvent(&ie) == 1) && (ie.type == RInputEvent::Key))
						|| IsXInputButtonPressed())
						break;
					}
				}
			}


		////////////////////////////////////////////////////////////////////////////////
		// End cutscene
		////////////////////////////////////////////////////////////////////////////////
		/* virtual */
		void EndCutscene(
			CPlayInfo* pinfo)										// I/O: Play info
			{
			// End cutscene
			CutSceneEnd();

			// Clear any excess inputs
			rspClearAllInputEvents();

			rspLockBuffer();
			
			// Clear screen (to avoid palette flash when the hood sets its palette)
			rspRect(RSP_BLACK_INDEX, g_pimScreenBuf, 0, 0, g_pimScreenBuf->m_sWidth, g_pimScreenBuf->m_sHeight);

			rspUnlockBuffer();

			rspUpdateDisplay();

			// A quick delay while on the black screen looks better than no delay
			long lBlackTime = rspGetMilliseconds();
			while (rspGetMilliseconds() - lBlackTime < BLACK_HOLD_TIME)
				;
			}
	};


////////////////////////////////////////////////////////////////////////////////
//
// Abort all currently playing sounds and do not return until they are gone
// unless timed out for safety.
//
////////////////////////////////////////////////////////////////////////////////
inline void SynchronousSampleAbortion(void)
	{
	// Stop all currently playing samples abruptly.
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
	}


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
	const bool bAddOn,									// In:  Play add on levels
	const short sDifficulty,							// In:  Difficulty level
	const bool bRejuvenate,								// In:  Whether to allow players to rejuvenate (MP only)
	const short sTimeLimit,								// In:  Time limit for MP games (0 or negative if none)
	const short sKillLimit,								// In:  Kill limit for MP games (0 or negative if none)
	const	short	sCoopLevels,							// In:  Zero for deathmatch levels, non-zero for cooperative levels.
	const	short	sCoopMode,								// In:  Zero for deathmatch mode, non-zero for cooperative mode.
	const short sFrameTime,								// In:  Milliseconds per frame (MP only)
	RFile* pfileDemoModeDebugMovie)					// In:  File for loading/saving demo mode debug movie
	{
	short sResult = 0;

//#ifdef MOBILE
	if (inputMode == INPUT_MODE_PLAYBACK)
		demoCompat = true; //DEMO playback
	else
		demoCompat = false;
//#endif


	// If this is the last demo level, then load the mult alpha needed for the ending
	RMultiAlpha* pDemoMultiAlpha = NULL;

	if (g_bLastLevelDemo)
		{
		sResult = rspGetResource(&g_resmgrGame, DEMO_MULTIALPHA_FILE, &pDemoMultiAlpha, RFile::LittleEndian);
		if (sResult != SUCCESS)
			TRACE("Play - Error loading multialpha mask for the ending demo\n");
		}

	// Enable RMix's autopump.
	RMix::SetAutoPump(TRUE);

	// Clear any events that might be in the queue
	rspClearAllInputEvents();

	// Lock the composite buffer before accessing it.
	rspLockBuffer();
	// Clear screen
	rspRect(RSP_BLACK_INDEX, g_pimScreenBuf, 0, 0, g_pimScreenBuf->m_sWidth, g_pimScreenBuf->m_sHeight);
	// Lock the composite buffer now that we're done.
	rspUnlockBuffer();

	rspUpdateDisplay();

	// Set input mode
	SetInputMode(inputMode);

	// Reset AI logging feature to avoid potential multiplayer/demo sync problems
	CPerson::ResetLogAI();

	// Reseed random number generator to keep multiplayer/demo games sync'ed.
	SeedRand(1);

	// Create all the play modules
	CPlayNet			playNet;
	CPlayStatus		playStatus;
	CPlayRealm		playRealm;
	CPlayInput		playInput;
	CPlayCutscene	playCutscene;
	CPlayScore		playScore;

	// Create play group and add all the modules to it
	CPlayGroup playgroup;
	playgroup.AddModule(&playNet);
	playgroup.AddModule(&playStatus);
	playgroup.AddModule(&playRealm);
	playgroup.AddModule(&playInput);
	playgroup.AddModule(&playScore);
	playgroup.AddModule(&playCutscene);

	// Create and fill in play info
	CPlayInfo info;
	info.m_pclient = pclient;
	info.m_pserver = pserver;
	info.m_sRealmNum = sRealmNum;
	info.m_bGauntlet = bGauntlet;
	info.m_bAddOn = bAddOn;
	info.m_sFrameTime = sFrameTime;
	info.m_sCoopLevels = sCoopLevels;
	info.Realm()->m_flags.bCoopMode = sCoopMode ? true : false;
	info.Realm()->m_flags.sDifficulty = sDifficulty;	// MUST be set before Play_GetRealmInfo() calls.
	if (info.m_sRealmNum < 0)
		{
		strncpy(info.m_szRealm, pszRealmFile, sizeof(info.m_szRealm));
		info.m_szRealm[sizeof(info.m_szRealm)-1] = 0;
		info.m_bJustOneRealm = true;
		}
	else
		{
		if (Play_GetRealmInfo(info.IsMP(), info.CoopLevels(), info.Gauntlet(), info.AddOn(), info.m_sRealmNum, info.Realm()->m_flags.sDifficulty, info.m_szRealm, sizeof(info.m_szRealm)) == 0)
			{
			info.m_bJustOneRealm = bJustOneRealm;
			}
		else
			{
			// 09/12/97 MJR - Clear the string.  The CPlayInfo constructor actually does this, but this
			// makes it more obvious.
			info.m_szRealm[0] = 0;
			sResult	= -1;
			TRACE("Play(): Couldn't get info for realm #%hd!\n", (short)sRealmNum);
			}
		}
	info.m_pfileDemoModeDebugMovie = pfileDemoModeDebugMovie;

	// 09/12/97 MJR - Here exists yet another error in the release version, but thankfully,
	// it works out okay.  Note how completely ignore the value in sResult and simply
	// overwrite it with the return value from PrepareGame().  This should, in general,
	// fail further along the way when we try to load this realm.

	// Open the realm prefs file
	RPrefs prefsRealm;

	// Try opening the realms.ini file on the HD path first, if that fails go to the CD
	sResult = prefsRealm.Open(FullPathHD(g_GameSettings.m_pszRealmPrefsFile), "rt");
	if (sResult != 0)
		sResult = prefsRealm.Open(FullPathCD(g_GameSettings.m_pszRealmPrefsFile), "rt");
	if (sResult == 0)
		{
		short	sNumLevels;
		prefsRealm.GetVal("Info", "NumSinglePlayerLevels", 16, &sNumLevels);
		prefsRealm.Close();

		// Prepare game
		sResult = playgroup.PrepareGame(&info);
		if (!sResult)
			{

			// Wait until game is ready
			bool bGameReady = false;
			do	{
				sResult = playgroup.IsGameReady(&info, &bGameReady);
				} while (!sResult && !bGameReady);
			if (!sResult && bGameReady)
				{
				// Start game
				sResult = playgroup.StartGame(&info);
				if (sResult == 0)
					{


					/*** 12/5/97 AJC ***/
	#ifdef WIN32
					if (info.IsMP())
						OpenLogFile();
	#endif

#ifdef MOBILE
					bool doAutoSaveGame = false; //This is set to true when you complete a level, so it's auto saved when the next realm starts
#endif
					/*** 12/5/97 AJC ***/
					// Outer loop keeps playing one realm after another
					do	{
						long startRealmMS = -1;

						// Clear game status
						info.SetGameState_Ok();


						// Update global realm number so the "save game" mechanism knows what realm we're on
						g_sRealmNumToSave = info.m_sRealmNum;

						// Sounds playing during the load suck.
						SynchronousSampleAbortion();

						// Start the cutscene
						playgroup.StartCutscene(&info);

						// Prepare realm
						sResult = playgroup.PrepareRealm(&info);
						if (!sResult)
							{

							// Wait until realm is ready
							bool bRealmReady = false;
							do	{
								sResult = playgroup.IsRealmReady(&info, &bRealmReady);
								} while (!sResult && !bRealmReady);
							if (!sResult && bRealmReady)
								{
								if ((!info.IsMP()) && (info.m_sRealmNum == 1))
									UnlockAchievement(ACHIEVEMENT_START_SECOND_LEVEL);
#ifdef MOBILE//Tap screen to get past
								AndroidSetScreenMode(TOUCH_SCREEN_BLANK_TAP);
#endif
								// do the cutscene
								playgroup.DoCutscene(&info);
#ifdef MOBILE
								if (inputMode == INPUT_MODE_PLAYBACK)
									AndroidSetScreenMode(TOUCH_SCREEN_BLANK_TAP); //DEMO playback
								else
									AndroidSetScreenMode(TOUCH_SCREEN_GAME);
#endif
								// End the cutscene
								playgroup.EndCutscene(&info);
								// If multiplayer mode, set up the scoring mode from
								// the flags passed into play.
								if (pclient)
								{
									info.Realm()->m_sKillsGoal = sKillLimit;
									info.Realm()->m_lScoreInitialTime = info.Realm()->m_lScoreTimeDisplay = (long)sTimeLimit * (long)60000;
									
									// If Rejuvenate is allowed, then its not last man standing
									if (bRejuvenate)
									{
										if (sKillLimit > 0 && sTimeLimit > 0)
											info.Realm()->m_ScoringMode = CRealm::MPTimedFrag;

										if (sKillLimit <= 0 && sTimeLimit > 0)
											info.Realm()->m_ScoringMode = CRealm::MPTimed;

										if (sKillLimit > 0 && sTimeLimit <= 0)
											info.Realm()->m_ScoringMode = CRealm::MPFrag;

										if (sKillLimit <= 0 && sTimeLimit <= 0)
										{
											info.Realm()->m_sKillsGoal = KILLS_LIMIT_DEFAULT;
											info.Realm()->m_ScoringMode = CRealm::MPFrag;
										}
									}
									// Last man standing mode
									else
									{
										if (sKillLimit > 0 && sTimeLimit > 0)
											info.Realm()->m_ScoringMode = CRealm::MPLastManTimedFrag;

										if (sKillLimit > 0 && sTimeLimit <= 0)
											info.Realm()->m_ScoringMode = CRealm::MPLastManFrag;

										if (sKillLimit <= 0 && sTimeLimit > 0)
											info.Realm()->m_ScoringMode = CRealm::MPLastManTimed;

										if (sKillLimit <= 0 && sTimeLimit <= 0)
											info.Realm()->m_ScoringMode = CRealm::MPLastMan;
									}
								}
								// Start realm
								sResult = playgroup.StartRealm(&info);
								if (sResult == 0)
									{

									// Init local input
									ClearLocalInput();

									// Set the resource managers to trace uncached loads
									g_resmgrGame.TraceUncachedLoads(true);
									g_resmgrSamples.TraceUncachedLoads(true);
									g_resmgrRes.TraceUncachedLoads(true);

									// Start the music:
									if (g_bLastLevelDemo)
										{
										// Begin Final Scene Music:
										PlaySample(
											g_smidFinalScene,
											SampleMaster::Unspecified,
											255,
											&g_siFinalScene,
											NULL,
											0,
											0,
											true);
										}

									StatsAreAllowed = !info.IsMP();  // !!! FIXME: we currently only track for single-player (because we don't check that kills belong to the local player, etc).
									startRealmMS = rspGetMilliseconds();

#ifdef MOBILE
									if (doAutoSaveGame)
									{
										TRACE("Doing autosave");
										char  szFile[256];
										snprintf(szFile, sizeof (szFile), "%s/auto.gme", SAVEGAME_DIR);
										if (Game_SavePlayersGame(szFile, info.Realm()->m_flags.sDifficulty) == SUCCESS)
										{
											TRACE("Auto Save success");
										}
										else
											TRACE("Auto Save FAILED");

										doAutoSaveGame= false; //reset
									}
#endif
									// Inner loop plays current realm until it's done
									RInputEvent	ie;
									do	{

										if ((info.Realm()->m_flags.sDifficulty != 11) && (!g_bLastLevelDemo))
											Flag_Achievements &= ~FLAG_HIGHEST_DIFFICULTY;

										// As always...
										UpdateSystem();

										// User input
										ie.type = RInputEvent::None;
										rspGetNextInputEvent(&ie);
										playgroup.CoreLoopUserInput(&info, &ie);

#ifdef MOBILE //Tap screen to show menu
										if (info.LocalDudePointer()->IsDead())
										{
											if (!info.m_bInMenu)
												AndroidSetScreenMode(TOUCH_SCREEN_BLANK_TAP);
										}
#endif
										// Update
										playgroup.CoreLoopUpdate(&info);


										// Render:

										// This requires access to the composite buffer so lock it down.
										rspLockBuffer();

										playgroup.CoreLoopRender(&info);

										playgroup.CoreLoopRenderOnTop(&info);

										// Release the composite buffer now that we're done.
										rspUnlockBuffer();

										// Draw to the screen.
										playgroup.CoreLoopDraw(&info);

										// Check if core loop is done
										} while (!playgroup.IsCoreLoopDone(&info));

									// Set the resource managers to trace uncached loads
									g_resmgrGame.TraceUncachedLoads(false);
									g_resmgrSamples.TraceUncachedLoads(false);
									g_resmgrRes.TraceUncachedLoads(false);

									// If this was the last demo level, then do the martini effect
									if (g_bLastLevelDemo)
										{
										RRect rect(0,40,640,400);
										MartiniDo(g_pimScreenBuf,
													 0,
													 0,
													 pDemoMultiAlpha,
													 15000,
													 24,
													 5000,
													 9000,
													 &rect,
													 5000,
													 g_siFinalScene	// to dim out...
													 );
										
										// End the sound:
										if (g_siFinalScene)
											{
											// Cut it off.
											AbortSample(g_siFinalScene);
											// Clear.
											g_siFinalScene = 0;
											// Play final sample that completes the cut off sound. ***
											}

										TRACE("Stop here before clearing screen");
										}

									// *** MP Score display ///////////////
									if (info.IsMP() && !rspGetQuitStatus() && !info.IsGameAborted())
										{
										// Display the high scores.  Currently, this is MODAL (but has a timeout)!
										ScoreDisplayHighScores(info.Realm(), info.Client(), MP_HIGH_SCORES_MAX_TIME );
										}

									// End realm
									const bool tmpStatsAreAllowed = StatsAreAllowed;
									StatsAreAllowed = false;
									playgroup.EndRealm(&info);
									StatsAreAllowed = tmpStatsAreAllowed;

									}
								else
									playgroup.StartRealmErr(&info);
								}
							else
								playgroup.IsRealmReadyErr(&info);
							}
						else
							playgroup.PrepareRealmErr(&info);

						const long endRealmMS = rspGetMilliseconds();
						const long timePlayedMS = ((startRealmMS > 0) && (endRealmMS > 0) && (endRealmMS > startRealmMS)) ? endRealmMS - startRealmMS : -1;
						const long newPlaythroughMS = playthroughMS + timePlayedMS;
						if (!g_bLastLevelDemo)  // don't charge the last level demo to playthroughMS.
							playthroughMS = ((playthroughMS < 0) || (timePlayedMS < 0) || (newPlaythroughMS < 0)) ? -1 : newPlaythroughMS;

						// End the cutscene.  It normally gets called above, but if an error
						// occurs it doesn't, so this is the backup.  Multiple calls are safe.
						playgroup.EndCutscene(&info);

						if (StatsAreAllowed)
							{
							Stat_LevelsPlayed++;
							if ((!sResult) && (info.LocalDudePointer()->IsDead()))
								Stat_Deaths++;
							}
						StatsAreAllowed = false;

						#if WITH_STEAMWORKS
						RequestSteamStatsStore();  // this is a good time to push any updated stats from the level.
						#endif

						// Figure out what to do next (same realm, next realm, game over, etc.)
						if (!sResult)
							{
							if (info.JustOneRealm() == true && info.IsRestartingRealm() == false)
								{
								info.SetGameState_GameOver();
								}
							else if (info.IsNextRealm())
								{
								CDude *pDude = info.LocalDudePointer();
								// this is how the toolbar display calculates health.
								const double health = (pDude->GetHealth()*100/pDude->m_sOrigHitPoints);
								if (health < 10)
									UnlockAchievement(ACHIEVEMENT_COMPLETE_LEVEL_ON_LOW_HEALTH);

								if (info.Realm()->m_sPopulation != 0)
									Flag_Achievements &= ~FLAG_KILLED_EVERYTHING;

								if (info.m_sRealmNum == 9)
									UnlockAchievement(ACHIEVEMENT_COMPLETE_LEVEL_10);

								info.m_sRealmNum++;
								switch (Play_GetRealmInfo(info.IsMP(), info.CoopLevels(), info.Gauntlet(), info.AddOn(), info.m_sRealmNum, info.Realm()->m_flags.sDifficulty, info.m_szRealm, sizeof(info.m_szRealm)))
									{
									case 0:	// Got info
#ifdef MOBILE
										if (!bGauntlet) //Dont autosave if playing a challenge!
											doAutoSaveGame = true;
#endif
										break;

									case 1:	// No such realm number
										if (info.IsMP())
											{
											// Multiplayer just keeps wrapping around
											info.m_sRealmNum = 0;
											if (Play_GetRealmInfo(info.IsMP(), info.CoopLevels(), info.Gauntlet(), info.AddOn(), info.m_sRealmNum, info.Realm()->m_flags.sDifficulty, info.m_szRealm, sizeof(info.m_szRealm)) != 0)
												{
												// 09/12/97 MJR - We don't want to exit the loop if this happens.  Instead,
												// we set the bad realm flag and let the core loop handle the abort process.
												info.m_bBadRealmMP = true;
												TRACE("Play(): Couldn't get info for realm #%hd!\n", (short)info.m_sRealmNum);
												}
											}
										else
											{
											// This is a bit weird, but it works!  If the player has reached the
											// last level, either the game is over or the player has won the game,
											// depending on what mode we're in.  If the game is over, we just set
											// the appropriate game state.  If the player won the game, we set a
											// special flag and allow the loop we're in to continue, even though
											// there is no such realm (that's how we get to this point).  Other
											// special-case code handles everything that happens after that to do
											// the actual ending scene for the game.
											if (!info.Gauntlet() && !info.JustOneRealm() && info.RealmNum() == sNumLevels)
												g_bLastLevelDemo = true;
											else
												info.SetGameState_GameOver();
											}
										break;

									default:	// Error
										// 09/12/97 MJR - In MP, we don't want to exit the loop if this happens.  Instead,
										// we set the bad realm flag and let the core loop handle the abort process.
										if (info.IsMP())
											info.m_bBadRealmMP = true;
										else
											sResult	= -1;
										TRACE("Play(): Couldn't get info for realm #%hd!\n", (short)info.m_sRealmNum);
										break;
									}
								}
							}

						} while (!sResult && !info.IsGameOver() && !g_bLastLevelDemo);

					/*** 12/5/97 AJC ***/
	#ifdef WIN32
					if (info.IsMP())
						CloseLogFile();
	#endif
					/*** 12/5/97 AJC ***/

					}
				else
					playgroup.StartGameErr(&info);
				}
			else
				playgroup.IsGameReadyErr(&info);
			}
		else
			playgroup.PrepareGameErr(&info);

		// Unprepare game
		playgroup.UnprepareGame(&info);
		}

	rspLockBuffer();

	// Clear screen
	rspRect(RSP_BLACK_INDEX, g_pimScreenBuf, 0, 0, g_pimScreenBuf->m_sWidth, g_pimScreenBuf->m_sHeight);

	rspUnlockBuffer();

	rspUpdateDisplay();

	// Disable autopump.
	RMix::SetAutoPump(FALSE);

	// Abort all playing sounds.
	SynchronousSampleAbortion();

	// Clear any events that might be in the queue
	rspClearAllInputEvents();

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
//
// Snap picture to disk.
//
////////////////////////////////////////////////////////////////////////////////
extern void Play_SnapPicture(void)
	{
	// Feedback is nice.
	PlaySample(
		g_smidClick,
		SampleMaster::UserFeedBack);

	// Set up palette for snap shots once.
	RPal	palPicture;
	if (palPicture.CreatePalette(RPal::PDIB) == 0)
		{
		palPicture.m_sStartIndex	= 0;
		palPicture.m_sNumEntries	= 256;
		rspGetPaletteEntries(
			palPicture.m_sStartIndex,		// Palette entry to start copying to (has no effect on source!)
			palPicture.m_sNumEntries,		// Number of palette entries to do
			palPicture.Red(0),				// Pointer to first red component to copy to
			palPicture.Green(0),				// Pointer to first green component to copy to
			palPicture.Blue(0),				// Pointer to first blue component to copy to
			palPicture.m_sPalEntrySize);	// Number of bytes by which to increment pointers after each copy

		// Store screen buffer's actual type and palette
		RImage::Type	typeOrig		= g_pimScreenBuf->m_type;
		RPal*				ppalOrig		= g_pimScreenBuf->m_pPalette;

		// Temporarily change its type and palette
		g_pimScreenBuf->m_type		= RImage::BMP8;
		g_pimScreenBuf->m_pPalette	= &palPicture;

		// Save picture to file
		char szFileName[RSP_MAX_PATH];
		sprintf(szFileName, "PostalShot%03ld.bmp", ms_lCurPicture++);

		// This will require direct access to the composite buffer.
		rspLockBuffer();

		g_pimScreenBuf->SaveDib(szFileName);

		rspUnlockBuffer();

		// Restore original type and palette
		g_pimScreenBuf->m_type		= typeOrig;
		g_pimScreenBuf->m_pPalette	= ppalOrig;
		}
	}


////////////////////////////////////////////////////////////////////////////////
//
// Callback from g_menuVerifyQuitGame with choice.
//
////////////////////////////////////////////////////////////////////////////////
extern bool Play_VerifyQuitMenuChoice(				// Returns true to accept, false to deny choice.
	Menu*	pmenuCurrent,									// In:  Current menu.
	short	sMenuItem)										// In:  Item chosen or -1 for change of focus.
	{
	bool	bAcceptChoice	= true;	// Assume accepting choice.

	switch (sMenuItem)
		{
		case 0:	// Continue.
#ifdef MOBILE
			continueIsRestart = true; //Now the continue button will restart the realm
#endif
			ms_menuaction	= MenuActionEndMenu;
			break;
		case 1:	// Save game
			ms_menuaction	= MenuActionSaveGame;
			break;
		case 2:	// Options.
			break;
		case 3:	// Quit.
			ms_menuaction	= MenuActionQuit;
			break;
#ifdef MOBILE
		case 10:// Menu cancelled, set in menus_android.cpp
			ms_menuaction	= MenuActionEndMenu;
			break;
#endif
		}

	// Audible Feedback.
	if (sMenuItem == -1)
		PlaySample(g_smidMenuItemChange, SampleMaster::UserFeedBack);
	else
		PlaySample(g_smidMenuItemSelect, SampleMaster::UserFeedBack);

	return bAcceptChoice;
	}


////////////////////////////////////////////////////////////////////////////////
//
// Get info about specified realm
//
////////////////////////////////////////////////////////////////////////////////
extern short Play_GetRealmInfo(						// Returns 0 if successfull, 1 if no such realm, negative on error
	bool	bNetwork,										// In:  true if network game, false otherwise
	bool	bCoop,											// In:  true if coop net game, false otherwise -- no effect if bNetwork is false.
	bool  bGauntlet,										// In:  true if playing challenge mode
	bool  bAddOn,											// In:  true if playing the new add on levels
	short sRealmNum,										// In:  Realm number
	short	sDifficulty,									// In:  Realm difficulty.
	char* pszFile,											// Out: Realm's file name
	short sMaxFileLen,									// In:  Max length of returned file name, including terminating null
	char* pszTitle /*= 0*/,								// Out: Realm's title
	short sMaxTitleLen /*= NULL*/)					// In:  Max length of returned title, including terminating null
	{
	ASSERT(sRealmNum >= 0);
	ASSERT(pszFile != NULL);
	ASSERT(sMaxFileLen > 0);

	short	sResult = 0;

	// Open the realm prefs file
	RPrefs prefsRealm;

	// Try opening the realms.ini file on the HD path first, if that fails go to the CD
	sResult = prefsRealm.Open(FullPathHD(g_GameSettings.m_pszRealmPrefsFile), "rt");
	if (sResult != 0)
		sResult = prefsRealm.Open(FullPathCD(g_GameSettings.m_pszRealmPrefsFile), "rt");
	if (sResult == 0)
		{

		// Get realm's section and entry strings
		RString strSection;
		RString strEntry;
		Play_GetRealmSectionAndEntry(bNetwork, bCoop, bGauntlet, bAddOn, sRealmNum, sDifficulty, &strSection, &strEntry);

		// Get realm file name from prefs file
		char szText[RSP_MAX_PATH * 2];
		prefsRealm.GetVal((char*)strSection, (char*)strEntry, "", szText);
		if (strlen(szText) == 0)
			{
			// Realm not found
			sResult = 1;
			}
		else if ((strlen(szText) + 1) <= sMaxFileLen)
			{
			// Return the file name
			strcpy(pszFile, szText);

			// Check if caller wants the title, too
			if ((sMaxTitleLen > 0) && (pszTitle != NULL))
				{
				// Get title from prefs file
				prefsRealm.GetVal((char*)strSection, "Title", "Untitled", szText);

				// Copy amount that will fit
				strncpy(pszTitle, szText, sMaxTitleLen - 2);
				pszTitle[sMaxTitleLen - 1]	= '\0';
				}
			}
		else
			{
			// File name too long (and can't be truncated)
			sResult = -1;
			TRACE("Play_GetRealmInfo(): Realm file name/path too long!\n");
			rspMsgBox(RSP_MB_ICN_STOP | RSP_MB_BUT_OK, g_pszCriticalErrorTitle, g_pszBadPath_s_s, "Realm", (char*)strSection);
			}

		prefsRealm.Close();
		}
	else
		{
		sResult = -1;
		TRACE("Play_GetRealmInfo(): Error opening realm prefs file: '%s'!\n", FullPathCD(g_GameSettings.m_pszRealmPrefsFile));
		rspMsgBox(RSP_MB_ICN_STOP | RSP_MB_BUT_OK, "", "Can't open realm prefs file '%s'.\n", FullPathCD(g_GameSettings.m_pszRealmPrefsFile));
		}

	return sResult;
	}


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
	bool  bAddOnLevels,									// In:  true if playing new add on levels
	short sRealmNum,										// In:  Realm number
	short	sDifficulty,									// In:  Realm difficulty.
	RString* pstrSection,								// Out: Section is returned here
	RString* pstrEntry)									// Out: Entry is returned here
	{
	if (bNetwork)
		{
		if (bCoop == false)
			{
			// Deathmatch multiplayer sections are named "RealmNet1, "RealmNet2", etc.
			*pstrSection = "RealmNet";
			}
		else
			{
			// Cooperative multiplayer sections are named "RealmCoopNet1, "RealmCoopNet2", etc.
			*pstrSection = "RealmCoopNet";
			}

		*pstrSection += (short)(sRealmNum + 1);
		// Multiplayer realm entry is always "Realm"
		*pstrEntry = "Realm";
		}
	else if (bGauntlet)
		{
		// Challenge sections are named "Challenge1", "Challenge2", etc.
		*pstrSection = "Gauntlet";
		*pstrSection += (short)(sRealmNum + 1);
		// Challen realm entry is always "Realm"
		*pstrEntry = "Realm";
		}
	else
		{
		if (g_bLastLevelDemo)
			{
			*pstrSection = "RealmEnd";
			*pstrEntry = "RealmHard";
			}
			else
			{
			// Single player sections are named "Realm1", "Realm2", etc.
			// AddOn single player sections are named "AddOn1", "AddOn2", etc.
			if (bAddOnLevels)
				*pstrSection = "AddOn";
			else
				*pstrSection = "Realm";
			*pstrSection += (short)(sRealmNum + 1);
			// Single player entry depends on difficulty level
			switch (sDifficulty)
				{
				case 0:
				case 1:
				case 2:
				case 3:
					*pstrEntry = "RealmEasy";
					break;

				case 4:
				case 5:
				case 6:
					*pstrEntry = "RealmMedium";
					break;

				case 7:
				case 8:
				case 9:
				case 10:
				case 11:
				default:
					*pstrEntry = "RealmHard";
					break;
				}
			}
		}
	}


////////////////////////////////////////////////////////////////////////////////
//
// Creates descriptor including app's time stamp, debug status (debug or release)
// and, if defined, TRACENASSERT flag.
//
////////////////////////////////////////////////////////////////////////////////
extern
void Play_GetApplicationDescriptor(			// Returns nothing.
	char* pszText,									// Out: Text descriptor.
	short	sMaxBytes)								// In:  Amount of writable 
														// memory pointed to by pszText.
	{
	// Set default in case there's an error
	ASSERT(strlen(DEFAULT_APP_TIMESTAMP) < sMaxBytes);
	strcpy(pszText, DEFAULT_APP_TIMESTAMP);

	#if defined(WIN32)
		char	szModuleFileName[RSP_MAX_PATH];
		if (GetModuleFileName(NULL, szModuleFileName, sizeof(szModuleFileName)) > 0)
			{
			struct _stat statExe;
			if (_stat(szModuleFileName, &statExe) == 0)
				{
				char*	pszTime	= ctime(&statExe.st_mtime);
				if (pszTime)
					{
					if (strlen(pszText) + strlen(pszTime) < sMaxBytes)
						{
						// ctime() returns a string of exactly 26 characters, including /n and null.
						strcpy(pszText, pszTime);
						}

					// Get rid of trailing '\n'.
					pszText[strlen(pszText) - 1]	= '\0';
					}
				}
			}
	#endif

#ifdef _DEBUG
	if (strlen(pszText) + strlen(DEBUG_STR) < sMaxBytes)
		{
		strcat(pszText, DEBUG_STR);
		}
#else
	if (strlen(pszText) + strlen(RELEASE_STR) < sMaxBytes)
		{
		strcat(pszText, RELEASE_STR);
		}
#endif

#ifdef TRACENASSERT
	if (strlen(pszText) + strlen(TRACENASSERT_STR) < sMaxBytes)
		{
		strcat(pszText, TRACENASSERT_STR);
		}
#endif
	}


////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
